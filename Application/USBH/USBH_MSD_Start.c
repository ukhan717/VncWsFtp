/*********************************************************************
*                     SEGGER Microcontroller GmbH                    *
*                        The Embedded Experts                        *
**********************************************************************
*                                                                    *
*       (c) 2003 - 2019  SEGGER Microcontroller GmbH                 *
*                                                                    *
*       www.segger.com     Support: support@segger.com               *
*                                                                    *
**********************************************************************
-------------------------- END-OF-HEADER -----------------------------

File    : USBH_MSD_Start.c
Purpose : This sample is designed to present emUSBH's capability to
          enumerate Mass Storage Devices and perform file system
          operations on them.

          This sample will try to mount a connected device, check
          if it is formatted and create a "TestFile.txt" in the
          root directory.
          If the enumeration does not succeed an error callback
          is called, it prints information about the particular error.

Notes:
  (1) When testing this sample with multiple USB ports please make sure
      that you have called FS_AddDevice(&USBH_MSD_FS_Driver); in the
      FS_X_AddDevices function in FS_USBH_MSDConfig.c. It should be
      called the same number of times as you have ports.
  (2) A common warning message with this sample is:
      "USBH_Task - *** Warning *** MSD SC6: _TestUnitReady: Command failed"
      This occurs when a USB stick has been enumerated but has a small
      delay before it can reply to MSD commands. This is common for
      low-budget USB sticks and is not an issue if the sample proceeds
      with normal function after this warning.

Additional information:
  Preparations:
    For MSD the correct emFile configuration file has
    to be included in the project: FS_USBH_MSDConfig.c

  Expected behavior:
    After a MSD has been connected information about it is printed
    to the terminal and the file "TestFile.txt" is created in the
    root directory of the MSD.

  Sample output:
    <...>

    **** Device added
    Running sample on "msd:0:"

    Reading volume information...

    **** Volume information for msd:0:
        125105536 KBytes total disk space
        125105536 KBytes available free space

            32768 bytes per cluster
          3909548 clusters available on volume
          3909548 free cluster available on volume

    Creating file msd:0:\TestFile.txt...Ok
    Contents of msd:0:
    TESTFILE.TXT       Attributes: A--- Size: 21

    **** Unmount ****

    Test with the following device was successful:
    VendorId:           0x1234
    ProductId:          0x5678
    VendorName:         XXXXXXX
    ProductName:        XXXXXXXXXXXXXX
    Revision:           1.00
    NumSectors:         250272768
    BytesPerSector:     512
    TotalSize:          122203 MByte
    HighspeedCapable:   No
    ConnectedToRootHub: Yes
    SelfPowered:        No
    Reported Imax:      500 mA
    Connected to Port:  1
    PortSpeed:          FullSpeed

    <...>
*/

/*********************************************************************
*
*       #include section
*
**********************************************************************
*/
#include <stdio.h>
#include "RTOS.h"
#include "BSP.h"
#include "USBH.h"
#include "USBH_MSD.h"
#include "FS.h"
#include "SEGGER.h"

/*********************************************************************
*
*       Local data definitions
*
**********************************************************************
*/
enum {
  TASK_PRIO_APP = 150,
  TASK_PRIO_USBH_MAIN,
  TASK_PRIO_USBH_ISR
};

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
static OS_STACKPTR int     _StackMain[2048/sizeof(int)];
static OS_TASK             _TCBMain;

static OS_STACKPTR int     _StackIsr[1536/sizeof(int)];
static OS_TASK             _TCBIsr;
static U8                  _aMSD_DevIndexes[32];
static OS_MAILBOX          _MSDMailBox;

//
// Memory for the read-ahead-cache.
//
static U8                    _aAheadBuf[512 * 8]; // 8 sectors.
static USBH_MSD_AHEAD_BUFFER _AheadBuf;

/*********************************************************************
*
*       Static Code
*
**********************************************************************
*/

/*********************************************************************
*
*       _GetPortSpeed
*
*  Function description
*    Returns the given USBH speed as a string.
*/
static const char * _GetPortSpeed(USBH_SPEED Speed) {
  switch (Speed) {
  case USBH_LOW_SPEED:
    return "LowSpeed";
  case USBH_FULL_SPEED:
    return "FullSpeed";
  case USBH_HIGH_SPEED:
    return "HighSpeed";
  default:
    break;
  }
  return "Unknown";
}

/*********************************************************************
*
*       _CalcXBytes
*
*  Function description
*    Multiplies 2 values and returns the result divided by 1024.
*    The reason for using a routine is that with a simple multiplication,
*    the 4GB range that a U32 can hold is typically exceeded.
*
*  Parameters
*    NumItems      Number of items such as number of sectors
*    BytesPerItem  Bytes per item, such as number of bytes is a sector or cluster. MUST BE A POWER OF 2!
*    ShiftCnt      Number of bits to shift.
*/
static U32 _CalcXBytes(U32 NumItems, U32 BytesPerItem, unsigned ShiftCnt) {
  int i;
  U32 v;

  v = 0;
  //
  // Compute number of bits to shift. 512: -1, 1024:0, 2048: 1
  //
  i = (int)(ShiftCnt)*(-1);
  while (BytesPerItem) {
    BytesPerItem >>= 1;
    i++;
  }
  //
  // Shift NumItems to compute the result
  //
  if (i > 0) {
    v = NumItems << i;
  } else if (i < 0) {
    v = NumItems >> -i;
  } else {
    v = NumItems;
  }
  return v;
}

/*********************************************************************
*
*       _CalcKBytes
*
*  Function description
*    Multiplies 2 values and returns the result divided by 1024.
*    The reason for using a routine is that with a simple multiplication,
*    the 4GB range that a U32 can hold is typically exceeded.
*
*  Parameters
*    NumItems      Number of items such as number of sectors
*    BytesPerItem  Bytes per item, such as number of bytes is a sector or cluster. MUST BE A POWER OF 2!
*/
static U32 _CalcKBytes(U32 NumItems, U32 BytesPerItem) {
  return _CalcXBytes(NumItems, BytesPerItem, 11);
}

/*********************************************************************
*
*       _CalcMBytes
*/
static U32 _CalcMBytes(U32 NumItems, U32 BytesPerItem) {
  return _CalcXBytes(NumItems, BytesPerItem, 21);
}

/*********************************************************************
*
*       _ShowDiskInfo
*
*  Function description
*    Displays information about a storage volume.
*/
static void _ShowDiskInfo(const char * s) {
  FS_DISK_INFO DiskInfo;

  USBH_Logf_Application("Reading volume information...");
  if (FS_GetVolumeInfo(s, &DiskInfo) != 0) {
    USBH_Logf_Application("**** Failed to retrieve volume information");
  } else {
    U32 DiskSpace;
    U32 FreeSpace;
    U32 BytesPerCluster;

    BytesPerCluster   = DiskInfo.BytesPerSector * DiskInfo.SectorsPerCluster;
    DiskSpace = _CalcKBytes(DiskInfo.NumTotalClusters, BytesPerCluster);
    FreeSpace = _CalcKBytes(DiskInfo.NumFreeClusters, BytesPerCluster);
    USBH_Logf_Application("**** Volume information for %s", s);
    USBH_Logf_Application("   %d KBytes total disk space", DiskSpace);
    USBH_Logf_Application("   %d KBytes available free space", FreeSpace);
    USBH_Logf_Application("   %d bytes per cluster", BytesPerCluster);
    USBH_Logf_Application("   %d clusters available on volume", DiskInfo.NumTotalClusters);
    USBH_Logf_Application("   %d free cluster available on volume", DiskInfo.NumFreeClusters);
  }
}

/*********************************************************************
*
*       _ShowDir
*
*  Function description
*    Displays all files within a given directory.
*/
static void _ShowDir(const char * sDirName) {
  FS_FIND_DATA fd;
  static char acFileName[256];
  char         r;

  USBH_Logf_Application("Contents of %s ", sDirName);
  r = FS_FindFirstFile(&fd, sDirName, acFileName, sizeof(acFileName));
  if (r == 0) {
    do {
      U8 Attr;

      Attr = fd.Attributes;
      USBH_Logf_Application("%s %s Attributes: %s%s%s%s Size: %d", fd.sFileName,
                          (Attr & FS_ATTR_DIRECTORY) ? "(Dir)" : "     ",
                          (Attr & FS_ATTR_ARCHIVE)   ? "A" : "-",
                          (Attr & FS_ATTR_READ_ONLY) ? "R" : "-",
                          (Attr & FS_ATTR_HIDDEN)    ? "H" : "-",
                          (Attr & FS_ATTR_SYSTEM)    ? "S" : "-",
                          fd.FileSize);
    } while (FS_FindNextFile(&fd));
    FS_FindClose(&fd);
  } else if (r == 1) {
    USBH_Logf_Application("Directory is empty");
  } else {
    USBH_Logf_Application("Unable to open directory %s", sDirName);
  }
}

/*********************************************************************
*
*       _CreateFile
*
*  Function description
*    Creates "TestFile.txt" in the root directory (if it does not exist)
*    and adds one line of text.
*/
static void _CreateFile(const char * sPath) {
  FS_FILE * pFile;
  char      ac[40];
  U32       r;

  SEGGER_snprintf(&ac[0], sizeof(ac), "%s\\TestFile.txt", sPath);
  USBH_Logf_Application("Creating file %s...", ac);
  pFile = FS_FOpen(&ac[0], "a+");
  if (pFile) {
    sprintf(ac, "System Time: %d\r\n", (int)OS_GetTime());
    r = FS_Write(pFile, ac, strlen(ac));
    if (r != strlen(ac)) {
      USBH_Logf_Application("FS_Write has an error: %s", FS_ErrorNo2Text(FS_FError(pFile)));
    }
    FS_FClose(pFile);
    USBH_Logf_Application("Ok");
  } else {
    USBH_Logf_Application("Failed. Could not create file");
  }
}

/*********************************************************************
*
*       _OnMSDReady
*
*  Function description
*    Called by the application task (MainTask) when an MSD device is plugged in.
*    It reads and displays capacity, contents of root directory and creates a file.
*/
static void _OnMSDReady(U8 DevIndex) {
  char               ac[20];
  USBH_MSD_UNIT_INFO UnitInfo;
  int                NumVolumes;
  USBH_PORT_INFO     PortInfo;
  int                Found;
  int                i;

  NumVolumes  = FS_GetNumVolumes();
  Found       = 0;
  for (i = 0; i < NumVolumes; i++) {
    FS_GetVolumeName(i, ac, sizeof(ac));
    if ((ac[0] == 'm') && (ac[1] == 's') && (ac[2] == 'd') && (ac[3] == ':') && ((DevIndex + '0')  == ac[4])) {
      Found = 1;
      //
      // Check if volume needs to be high level formatted.
      // If so, we do NOT automatically do this ...
      //
      if (FS_IsHLFormatted(ac) == 0) {
        USBH_Logf_Application("Medium is not formatted.");
      } else {
        USBH_Logf_Application("Running sample on \"%s\"", ac);
        _ShowDiskInfo(ac);
        _CreateFile(ac);
        _ShowDir(ac);
        FS_Unmount(ac);
        if (DevIndex != 0xff) {
          USBH_MSD_GetPortInfo(DevIndex, &PortInfo);
          USBH_MSD_GetUnitInfo(DevIndex, &UnitInfo);
          USBH_Logf_Application("**** Unmount ****\n");
          USBH_Logf_Application("Test with the following device was successful:\n");
          USBH_Logf_Application("VendorId:           0x%0.4X", UnitInfo.VendorId);
          USBH_Logf_Application("ProductId:          0x%0.4X", UnitInfo.ProductId);
          USBH_Logf_Application("VendorName:         %s", UnitInfo.acVendorName);
          USBH_Logf_Application("ProductName:        %s", UnitInfo.acProductName);
          USBH_Logf_Application("Revision:           %s", UnitInfo.acRevision);
          USBH_Logf_Application("NumSectors:         %u", UnitInfo.TotalSectors);
          USBH_Logf_Application("BytesPerSector:     %d", UnitInfo.BytesPerSector);
          USBH_Logf_Application("TotalSize:          %d MByte", _CalcMBytes(UnitInfo.TotalSectors, UnitInfo.BytesPerSector));
          USBH_Logf_Application("HighspeedCapable:   %s", PortInfo.IsHighSpeedCapable ? "Yes" : "No");
          USBH_Logf_Application("ConnectedToRootHub: %s", PortInfo.IsRootHub ? "Yes" : "No");
          USBH_Logf_Application("SelfPowered:        %s", PortInfo.IsSelfPowered ? "Yes" : "No");
          USBH_Logf_Application("Reported Imax:      %d mA", PortInfo.MaxPower);
          USBH_Logf_Application("Connected to Port:  %d", PortInfo.PortNumber);
          USBH_Logf_Application("PortSpeed:          %s", _GetPortSpeed(PortInfo.PortSpeed));
        }
      }
    }
  }
  if (Found == 0) {
    USBH_Logf_Application("MSD volume not found in FS, wrong FS_Config*.c used?");
    USBH_Logf_Application("When using multiple ports: correct number of FS devices added? (See note 1.)");
  }
}

/*********************************************************************
*
*       _cbOnAddRemoveDevice
*
*  Function description
*    Callback, called when a device is added or removed.
*    Called in the context of the USBH_Task.
*    The functionality in this routine should not block!
*/
static void _cbOnAddRemoveDevice(void * pContext, U8 DevIndex, USBH_MSD_EVENT Event) {
  (void)pContext;
  switch (Event) {
  case USBH_MSD_EVENT_ADD:
    USBH_Logf_Application("**** Device added [%d]", DevIndex);
    OS_PutMailCond1(&_MSDMailBox, (char *)&DevIndex);
    break;
  case USBH_MSD_EVENT_REMOVE:
    USBH_Logf_Application("**** Device removed [%d]", DevIndex);
    break;
  default:;   // Should never happen
  }
}

/*********************************************************************
*
*       _OnRegisterEnumErrorHook
*
*  Function description
*    Callback, called when an error occurs during the enumeration of a USB device.
*    Prints the specific error details.
*/
static void _OnRegisterEnumErrorHook( void * pContext, const USBH_ENUM_ERROR * pEnumError) {
  USBH_USE_PARA(pContext);
  USBH_Logf_Application("**** An error during enumeration has occurred");
  //
  // Display error types
  //
  if(pEnumError->Flags & USBH_ENUM_ERROR_EXTHUBPORT_FLAG) {
    USBH_Logf_Application("*** The device is connected to an external hub");
  }
  if(pEnumError->Flags & USBH_ENUM_ERROR_RETRY_FLAG) {
    USBH_Logf_Application("*** The enumeration is being retried");
  }
  if(pEnumError->Flags & USBH_ENUM_ERROR_STOP_ENUM_FLAG) {
    USBH_Logf_Application("*** The enumeration was stopped after retries");
  }
  if(pEnumError->Flags & USBH_ENUM_ERROR_DISCONNECT_FLAG) {
    USBH_Logf_Application("*** The parent port status is disconnected, this means the USB device is not connected or it is connected and has an error");
  }
  //
  // Display error location
  //
  if(pEnumError->Flags & USBH_ENUM_ERROR_ROOT_PORT_RESET) {
    USBH_Logf_Application("*** Error during reset of a USB device on a root hub port");
  }
  if(pEnumError->Flags & USBH_ENUM_ERROR_HUB_PORT_RESET) {
    USBH_Logf_Application("*** Error during reset of a USB device on an external hub port");
  }
  if(pEnumError->Flags & USBH_ENUM_ERROR_INIT_DEVICE) {
    USBH_Logf_Application("*** Error during initialization of an device until it is in the configured state");
  }
  if(pEnumError->Flags & USBH_ENUM_ERROR_INIT_HUB) {
    USBH_Logf_Application("*** Error during initialization of a configured external hub device until the installation of an interrupt IN status pipe");
  }
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       MainTask
*/
#ifdef __cplusplus
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif
void MainTask(void);
#ifdef __cplusplus
}
#endif
void MainTask(void) {
  U8 DevIndex;

  USBH_Init();
  USBH_RegisterEnumErrorNotification(NULL, _OnRegisterEnumErrorHook);                  // Registers a callback function for enumerations errors
  OS_SetPriority(OS_GetTaskID(), TASK_PRIO_APP);                                       // This task has the lowest prio for real-time application.
                                                                                       // Tasks using emUSB-Host API should always have a lower priority than emUSB-Host main and ISR tasks.
  OS_CREATETASK(&_TCBMain, "USBH_Task", USBH_Task, TASK_PRIO_USBH_MAIN, _StackMain);   // Start USBH main task
  OS_CREATETASK(&_TCBIsr, "USBH_isr", USBH_ISRTask, TASK_PRIO_USBH_ISR, _StackIsr);    // Start USBH ISR task
  FS_Init();
  //
  // Uncomment to enable Long File Name support.
  //
//  FS_FAT_SupportLFN();
  OS_CREATEMB(&_MSDMailBox, sizeof(U8), SEGGER_COUNTOF(_aMSD_DevIndexes), &_aMSD_DevIndexes);
  USBH_MSD_Init(_cbOnAddRemoveDevice, NULL);
  //
  //  USBH_MSD_UseAheadCache(1) is used to avoid problems with Transcend/Kingston USB sticks.
  //  These sticks have a problem when reading one sector multiple times. The consequence: They crash and emUSBH recognizes
  //  a detach and says that the current device was removed. Removing the call to this function will result in a small
  //  performance boost.
  //
  _AheadBuf.pBuffer = _aAheadBuf;
  _AheadBuf.Size    = sizeof(_aAheadBuf);
  USBH_MSD_SetAheadBuffer(&_AheadBuf);
  USBH_MSD_UseAheadCache(1);

  while (1) {
    BSP_ToggleLED(1);
    if (OS_GetMailTimed(&_MSDMailBox, (char *)&DevIndex, 100) == 0) {
      _OnMSDReady(DevIndex);
    }
  }
}

/*************************** End of file ****************************/
