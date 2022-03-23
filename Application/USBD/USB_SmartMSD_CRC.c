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

File    : USB_SmartMSD_Start.c
Purpose : This sample shows the basic usage of the SmartMSD component.

Additional information:
  Preparations:
    None.

  Expected behavior:
    When the device running this sample is
    connected to a USB host, a file can be copied to the
    volume and the device will calculate a CRC over the file
    and store it into a new file CRC.TXT.

  Sample output:

    <...>

    22:968 MainTask - SMSD CRC: _cbOnWrite(0,Off=2715648,Len=512,--,-1)
    22:969 MainTask - SMSD CRC: _cbOnWrite(0,Off=2716160,Len=512,--,-1)
    22:969 MainTask - SMSD CRC: _cbOnWrite(0,Off=2716672,Len=512,--,-1)
    22:970 MainTask - SMSD CRC: _cbOnWrite(0,Off=2717184,Len=512,--,-1)
    22:971 MainTask - SMSD CRC: _cbOnWrite(0,Off=2717696,Len=512,--,-1)
    22:972 MainTask - SMSD CRC: _cbOnWrite(0,Off=2718208,Len=512,--,-1)
    22:973 MainTask - SMSD CRC: _cbOnWrite(0,Off=2718720,Len=512,--,-1)
    22:974 MainTask - SMSD CRC: _cbOnWrite(0,Off=2719232,Len=512,--,-1)
    22:975 MainTask - SMSD CRC: _cbOnWrite(0,Off=2719744,Len=512,--,-1)
    22:981 MainTask - SMSD CRC: CRC calculated = 2972EDA5
*/

/*********************************************************************
*
*       #include section
*
**********************************************************************
*/
#include <stdio.h>
#include "BSP.h"
#include "USB_Private.h"
#include "USB_MSD.h"
#include "USB_SmartMSD.h"

/*********************************************************************
*
*       Defines, configurable
*
**********************************************************************
*/

#define _SMARTMSD_NUM_SECTORS  12000
#define _SIZE_CRC_FILE            30
#define _WRITE_BUFF_NUM_SECTOTS    8

#ifndef DEBUG
  #define DEBUG 0
#endif

/*********************************************************************
*
*       Defines, non configurable
*
**********************************************************************
*/
#define SECTOR_SIZE                  512

/*********************************************************************
*
*       Static const data
*
**********************************************************************
*/
//
//  Information that is used during enumeration.
//
static const USB_DEVICE_INFO _DeviceInfo = {
  0x8765,         // VendorId
  0x1000,         // ProductId
  "Vendor",       // VendorName
  "SmartMSD device",  // ProductName
  "0123456789AB"  // SerialNumber. Should be 12 character or more for compliance with Mass Storage Device For Bootability spec.
};
//
// String information used when inquiring the volume 0.
//
static const USB_MSD_LUN_INFO _LunInfo = {
  "Vendor",     // MSD VendorName
  "MSD Volume", // MSD ProductName
  "1.00",       // MSD ProductVer
  "134657890"   // MSD SerialNo
};

//
// Remaining bytes of sector are filled with 0s on read, if a file does not occupy complete sectors
//
static const U8 _abFile_ReadmeTxt[] = {
  0x54,  0x68,  0x69,  0x73,  0x20,  0x73,  0x61,  0x6d,  0x70,  0x6c,
  0x65,  0x20,  0x73,  0x68,  0x6f,  0x77,  0x73,  0x20,  0x68,  0x6f,
  0x77,  0x20,  0x73,  0x69,  0x6d,  0x70,  0x6c,  0x65,  0x20,  0x61,
  0x6e,  0x64,  0x20,  0x65,  0x61,  0x73,  0x79,  0x20,  0x69,  0x74,
  0x20,  0x69,  0x73,  0x20,  0x74,  0x6f,  0x20,  0x68,  0x61,  0x76,
  0x65,  0x20,  0x4d,  0x53,  0x44,  0x20,  0x64,  0x72,  0x69,  0x76,
  0x65,  0x0d,  0x0a,  0x63,  0x6f,  0x6d,  0x70,  0x6c,  0x65,  0x74,
  0x65,  0x6c,  0x79,  0x20,  0x76,  0x69,  0x72,  0x74,  0x75,  0x61,
  0x6c,  0x69,  0x7a,  0x65,  0x64,  0x2e,  0x20,  0x43,  0x6f,  0x70,
  0x79,  0x20,  0x61,  0x20,  0x66,  0x69,  0x6c,  0x65,  0x20,  0x74,
  0x6f,  0x20,  0x74,  0x68,  0x69,  0x73,  0x20,  0x64,  0x65,  0x76,
  0x69,  0x63,  0x65,  0x20,  0x74,  0x6f,  0x20,  0x63,  0x61,  0x6c,
  0x63,  0x75,  0x6c,  0x61,  0x74,  0x65,  0x20,  0x69,  0x74,  0x73,
  0x0d,  0x0a,  0x63,  0x68,  0x65,  0x63,  0x6b,  0x73,  0x75,  0x6d,
  0x2e,  0x0a,  0x46,  0x6f,  0x72,  0x20,  0x66,  0x75,  0x72,  0x74,
  0x68,  0x65,  0x72,  0x20,  0x69,  0x6e,  0x66,  0x6f,  0x72,  0x6d,
  0x61,  0x74,  0x69,  0x6f,  0x6e,  0x20,  0x70,  0x6c,  0x65,  0x61,
  0x73,  0x65,  0x20,  0x76,  0x69,  0x73,  0x69,  0x74,  0x20,  0x6f,
  0x75,  0x72,  0x20,  0x77,  0x65,  0x62,  0x73,  0x69,  0x74,  0x65,
  0x3a,  0x0d,  0x0a,  0x77,  0x77,  0x77,  0x2e,  0x73,  0x65,  0x67,
  0x67,  0x65,  0x72,  0x2e,  0x63,  0x6f,  0x6d,  0x20,  0x6f,  0x72,
  0x20,  0x68,  0x61,  0x76,  0x65,  0x20,  0x61,  0x20,  0x63,  0x6c,
  0x6f,  0x73,  0x65,  0x72,  0x20,  0x6c,  0x6f,  0x6f,  0x6b,  0x20,
  0x69,  0x6e,  0x74,  0x6f,  0x20,  0x74,  0x68,  0x65,  0x20,  0x65,
  0x6d,  0x55,  0x53,  0x42,  0x2d,  0x44,  0x65,  0x76,  0x69,  0x63,
  0x65,  0x20,  0x6d,  0x61,  0x6e,  0x75,  0x61,  0x6c,  0x0a
};

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
//
// Constant files which should be displayed on the virtual volume.
//
static USB_SMSD_CONST_FILE _aConstFiles[] = {
//     sName                     pData                       FileSize                      Flags
  { "readme.txt",             _abFile_ReadmeTxt,          sizeof(_abFile_ReadmeTxt),          0 },
  { NULL/*filled in later*/,  NULL,                       _SIZE_CRC_FILE,                     0 }
};

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
static U32                          _aMEMBuffer[2800 / sizeof(U32)];
static int                          _DataRead = -1;
static U32                          _Crc;
static U8                           _DataBuff[_WRITE_BUFF_NUM_SECTOTS * SECTOR_SIZE];
static int                          _DataBuffWrPos;
static U32                          _DataInBuff;
static const USB_SMSD_FILE_INFO*    _pFile;
static U32                          _FileSize;
static U8                           _CrcCalculated;
static U32                          _CrcResult;
static U8                           _ReattchRequest;

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/

/*********************************************************************
*
*       CRC_Calc
*/
static U32 _CRC_Calc(const U8* pData, int NumBytes, U32 crc, U32 Polynom) {
  if (NumBytes) {
    int i;
    do {
      crc ^= *pData++;
      i = 8;
      do {
        if (crc & 1) {
          crc = (crc >> 1) ^ Polynom;
        } else {
          crc >>= 1;
        }
      } while (--i);
    } while (--NumBytes);
  }
  return crc;
}

/*********************************************************************
*
*       _CRC_Calc32
*/
static U32 _CRC_Calc32(const U8* pData, int NumBytes, U32 crc) {
  return _CRC_Calc(pData, NumBytes, crc, 0xEDB88320);    // Normal form is 0x04C11DB7
}

/*********************************************************************
*
*       _cbOnWrite
*
*  Parameters
*    Lun       LUN ID
*    pData     Data to be written
*    Off       Offset into current file to be written
*    NumBytes  Number of bytes to write into the file
*    pFile     *Optional* Additional information about the file being written (RootDir entry etc.)
*/
static int _cbOnWrite(unsigned Lun, const U8* pData, U32 Off, U32 NumBytes, const USB_SMSD_FILE_INFO* pFile) {
  int Ret;
#if DEBUG > 0
  char acBuff[14];
  int FileSize;

  if (pFile) {
    memcpy(acBuff, pFile->pDirEntry->acFilename, 8);
    acBuff[8] = 0;
    FileSize = pFile->pDirEntry->FileSize;
  } else {
    strcpy(acBuff, "--");
    FileSize = -1;
  }
  USBD_Logf_Application("SMSD CRC: _cbOnWrite(%u,Off=%u,Len=%u,%s,%d)",Lun,Off,NumBytes,acBuff,FileSize);
#else
  (void)Lun;
  (void)NumBytes;
#endif
  Ret = 0;
  if (pData != NULL && Off == 0) {
    //
    // New file upload started. Got first sector.
    //
    _CrcCalculated = 0;
    _Crc           = 0xFFFFFFFF;
    _FileSize      = 0;
    _DataRead      = 0;
    _DataInBuff    = 0;
    _DataBuffWrPos = 0;
    _pFile         = pFile;
    if (pFile == NULL) {
      Ret = 1;
    }
  }
  //
  // Check for continuous sectors of file.
  //
  if (_CrcCalculated || (_pFile && _pFile != pFile) || (pData && (int)Off != _DataRead)) {
    //
    // Wrong file or wrong sector sequence. Ignore data.
    //
    return 0;
  }
  if (pFile) {
    if (pFile->pDirEntry->DirAttr &
        (USB_SMSD_ATTR_HIDDEN | USB_SMSD_ATTR_SYSTEM | USB_SMSD_ATTR_VOLUME_ID | USB_SMSD_ATTR_DIRECTORY)) {
      //
      // We have processed the wrong file: Restart.
      //
      _DataRead = -1;
      return -1;
    }
    _FileSize = pFile->pDirEntry->FileSize;
  }
  if (pData) {
    //
    // We got data to process.
    //
    if (_DataInBuff >= sizeof(_DataBuff)) {
      //
      // Calculate CRC over the oldest data to make space for current sector.
      //
      _Crc = _CRC_Calc32(_DataBuff + _DataBuffWrPos, SECTOR_SIZE, _Crc);
      _DataInBuff -= SECTOR_SIZE;
    }
    //
    // Store new sector data into buffer.
    //
    memcpy(_DataBuff + _DataBuffWrPos, pData, SECTOR_SIZE);
    _DataInBuff    += SECTOR_SIZE;
    _DataBuffWrPos += SECTOR_SIZE;
    if (_DataBuffWrPos >= (int)sizeof(_DataBuff)) {
      _DataBuffWrPos = 0;
    }
    _DataRead += SECTOR_SIZE;
  }
  //
  // Check is file was received completely.
  //
  if (_FileSize != 0 && _FileSize <= (U32)_DataRead) {
    int NumBytesToDo;
    int RdPos;
    int NumBytesAtOnce;
    //
    // End of file reached.
    //
    NumBytesToDo = (int)_FileSize - (_DataRead - (int)_DataInBuff);
    if (NumBytesToDo < 0) {
      //
      // We already processed more data than the file contains.
      // --> _DataBuff was too small. Give up.
      //
      USBD_Logf_Application("SMSD CRC: End of file overrun");
      _DataRead = -1;
      return -1;
    }
    //
    // Now process remaining data up to end of file.
    //
    while (NumBytesToDo > 0) {
      RdPos = _DataBuffWrPos - _DataInBuff;
      if (RdPos < 0) {
        RdPos += sizeof(_DataBuff);
      }
      NumBytesAtOnce = sizeof(_DataBuff) - RdPos;
      if (NumBytesAtOnce > NumBytesToDo) {
        NumBytesAtOnce = NumBytesToDo;
      }
      _Crc = _CRC_Calc32(_DataBuff + RdPos, NumBytesAtOnce, _Crc);
      NumBytesToDo -= NumBytesAtOnce;
      _DataInBuff  -= NumBytesAtOnce;
    }
    _CrcResult     = _Crc;
    _CrcCalculated = 1;
    USBD_Logf_Application("SMSD CRC: CRC calculated = %X",_Crc);
    _ReattchRequest = 1;
    BSP_ToggleLED(1);
    USBD_MSD_RequestRefresh(0, USB_MSD_RE_ATTACH | USB_MSD_TRY_DISCONNECT);
    _DataRead = -1;
    Ret = 0;
  }
  return Ret;
}

/*********************************************************************
*
*       _cbOnRead
*
*  Parameters
*    Lun       LUN ID
*    pData     Data which will be sent to the host
*    Off       Offset of the current file requested by the host
*    NumBytes  Number of bytes to read
*    pFile     *Optional* Additional information about the file being written (RootDir entry etc.)
*/
static int _cbOnRead(unsigned Lun, U8* pData, U32 Off, U32 NumBytes, const USB_SMSD_FILE_INFO* pFile) {
  char *x;
  int  i;
  U32  Crc;
  const char *HexTab = "0123456789ABCDEF";

  (void)Lun;
  (void)Off;
  (void)NumBytes;
  (void)pFile;
  memset(pData, ' ', _SIZE_CRC_FILE);
  memcpy(pData, "Calculated CRC =", 16);
  x = (char *)pData + _SIZE_CRC_FILE;
  *--x = '\n';
  *--x = '\n';
  Crc = _CrcResult;
  for (i = 0; i < 8; i++) {
   *--x = HexTab[Crc & 0xF];
   Crc >>= 4;
  }
  return 0;
}


static const USB_SMSD_USER_FUNC_API _UserFuncAPI = {
  _cbOnRead,     // pfOnRead    -> Is called when a sector of a given file is read.
  _cbOnWrite,    // pfOnWrite   -> Is called when a sector of a given file is written.
  NULL,          // pfMemAlloc  -> Optional, can be set in order to allow the SmartMSD to share the mem alloc function of a system.
  NULL           // pfMemFree   -> Optional, can be set in order to allow the SmartMSD to share the mem free function of a system.
};

/*********************************************************************
*
*       USB_SMSD_X_Config
*
*  Function description
*    This function is called by the USB MSD Module during USBD_SMSD_Init() and initializes the SmartMSD volume.
*/
void USB_SMSD_X_Config(void) {
  //
  // Global configuration
  //
  USBD_SMSD_AssignMemory(&_aMEMBuffer[0], sizeof(_aMEMBuffer));
  USBD_SMSD_SetUserAPI(&_UserFuncAPI);
  //
  // Setup LUN0
  //
  USBD_SMSD_SetNumSectors(0, _SMARTMSD_NUM_SECTORS);
  USBD_SMSD_SetSectorsPerCluster(0, 32);    // Anywhere from 1 ... 128, but needs to be a Power of 2
  USBD_SMSD_SetNumRootDirSectors(0, 2);
  USBD_SMSD_SetVolumeInfo(0, "Virt0.MSD", &_LunInfo);   // Add volume ID
  if (_CrcCalculated) {
   _aConstFiles[1].sName = "CRC.txt";
  } else {
   _aConstFiles[1].sName = NULL;
  }
  USBD_SMSD_AddConstFiles(0, &_aConstFiles[0], SEGGER_COUNTOF(_aConstFiles));  // Push const file to the volume
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
*
*/
#if defined(__cplusplus)
extern "C"      /* Make sure we have C-declarations in C++ programs */
#endif
void MainTask(void);

void MainTask(void) {
  USBD_Init();
  //
  // Initialize Smart device (also calls USB_SMSD_X_Config())
  //
  USBD_SMSD_Add();
  USBD_SetDeviceInfo(&_DeviceInfo);
  USBD_Start();
  while (1) {
    while ((USBD_GetState() & (USB_STAT_CONFIGURED | USB_STAT_SUSPENDED)) != USB_STAT_CONFIGURED) {
      BSP_ToggleLED(0);
      USB_OS_Delay(50);
    }
    BSP_SetLED(0);
    USBD_MSD_Task();
    if (_ReattchRequest) {
      USBD_SMSD_ReInit();
      _ReattchRequest = 0;
    }
  }
}

/*************************** End of file ****************************/
