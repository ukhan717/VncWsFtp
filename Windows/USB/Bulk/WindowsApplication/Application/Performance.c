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
----------------------------------------------------------------------
File    : Performance.c
Purpose : USB BULK performance test application
---------------------------END-OF-HEADER------------------------------
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "USBBULK.h"

/*********************************************************************
*
*       defines, configurable
*
**********************************************************************
*/
#define APP_TITLE     "USB BULK Sample Performance"
#define PRODUCT_NAME  "Bulk performance"

#define CMD_TEST_SPEED         0x01
#define CMD_TEST               0x02
#define SUBCMD_SPEED_READ             100
#define SUBCMD_SPEED_WRITE            101

#define MAX_NUM_BYTES_TEST_NET    (4100)

/*********************************************************************
*
*       static data
*
**********************************************************************
*/

/*********************************************************************
*
*       static code
*
**********************************************************************
*/

#ifdef _WIN32

#include <windows.h>
#include <conio.h>

#else

#include <poll.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>

/*********************************************************************
*
*       Sleep
*
*/
static void Sleep(unsigned ms) {
  poll(NULL,0,ms);
}

/*********************************************************************
*
*       timeGetTime
*/
static unsigned timeGetTime(void) {
#ifdef linux
  struct timespec Time;

  clock_gettime(CLOCK_MONOTONIC, &Time);
  return ((unsigned)Time.tv_sec * 1000) + ((unsigned)Time.tv_nsec / 1000000);
#else
  struct timeval Time;
  gettimeofday(&Time,NULL);
  return ((unsigned)Time.tv_sec * 1000) + ((unsigned)Time.tv_usec / 1000);
#endif
}

/*********************************************************************
*
*       _kbhit
*/
static int _kbhit(void) {
  struct pollfd Fds;

  Fds.fd = 0;
  Fds.events = POLLIN;
  return poll(&Fds,1,0);
}

/*********************************************************************
*
*       _getch
*/
static int _getch(void) {
  char cc;
  read(0,&cc,1);
  return cc & 255;
}

#endif

/*********************************************************************
*
*       _ConsoleGetLine
*
*/
static char * _ConsoleGetLine(char * pBuffer, U32 BufferSize) {
  fgets(pBuffer, BufferSize, stdin);
  pBuffer[strlen(pBuffer) - 1] = 0;
  return pBuffer;
}

/*********************************************************************
*
*       _GetNumberFromConsole
*
*/
static void _GetNumberFromConsole(const char * s, unsigned * pNumber) {
  int    r;
  char   ac[100];

  printf("%s",s);
  _ConsoleGetLine(ac, sizeof(ac));
  r = strtoul(&ac[0],NULL,0);
  if (r > 0) {
    *pNumber = r;
  }
}


/*********************************************************************
*
*       _ShowDriverInfo
*
*/
static void _ShowDriverInfo(void) {
  char ac[200];
  unsigned Ver;
  char Build[2];

  Ver = USBBULK_GetDriverVersion();
  if (Ver) {
    Build[0] = Build[1] = 0;
    if (Ver % 100) {
      Build[0] = (int)(Ver % 100) + 'a' - 1;
    }
    USBBULK_GetDriverCompileDate(ac, sizeof(ac));
    printf("USB BULK driver version: %d.%.2d%s, compiled: %s\n", (int)(Ver / 10000), (int)(Ver / 100) % 100, Build, ac);
  }
}

/*********************************************************************
*
*       USB__StoreU32LE
*/
static void _StoreU32LE(U8 * p, U32 v) {
  *p       = (U8)((v      ) & 255);
  *(p + 1) = (U8)((v >>  8) & 255);
  *(p + 2) = (U8)((v >> 16) & 255);
  *(p + 3) = (U8)((v >> 24) & 255);
}

/*********************************************************************
*
*       _TestSpeed
*/
int _TestSpeed(USB_BULK_HANDLE hDevice, U8 SubCmd, U32 NumReps, U32 NumBytes) {
  U8  abOut[0x100];
  U8* pData;
  U8* p;
  int NumWords;
  int r = -1;
  int i;

  if ((NumReps | NumBytes) == 0) {
    return 0;
  }
  //
  // Alloc memory
  //
  pData = (U8*)malloc(NumBytes);
  if (pData == NULL) {
    return -1;                        // Error, no memory
  }
  memset(pData, 0, NumBytes);
  NumWords = NumBytes >> 2;
  for (i = 0; i < NumWords; i++) {
    *(U32*)(pData + i * 4) = i;
  }
  //
  // Compose command
  //
  p = &abOut[0];
  *p++ = CMD_TEST_SPEED;
  *p++ = SubCmd;
  _StoreU32LE(p, NumReps);
  _StoreU32LE(p + 4, NumBytes);
  //
  // Transfer command
  //
  if (USBBULK_Write(hDevice, &abOut[0], 10) != 10) {
    goto CleanUp;                     // Communication error
  }
  //
  // Transfer the data
  //
  do {
    switch (SubCmd) {
    case SUBCMD_SPEED_WRITE:
      if (USBBULK_Write(hDevice, pData, NumBytes) != (int)NumBytes) {
        goto CleanUp;                 // Communication error
      }
      break;
    case SUBCMD_SPEED_READ:
      if (USBBULK_Read(hDevice, pData, NumBytes) != (int)NumBytes) {
        goto CleanUp;                 // Communication error
      }
      break;
    }
  } while (--NumReps);
  r = 0;
CleanUp:
  free(pData);
  return r;
}


/*********************************************************************
*
*       _PerformanceTest
*
*/
static int _PerformanceTest(unsigned DevIndex) {
  unsigned   NumBytes = 0x2000;
  unsigned   NumReps  = 0x200;
  U32        t, tWrite, tRead;
  int        r = 0;
  USB_BULK_HANDLE hDevice;

  hDevice = USBBULK_Open(DevIndex);
  if (hDevice == 0) {
    printf("Unable to connect to USB BULK device\n");
    return 1;
  }
  USBBULK_SetReadTimeout(hDevice, 3600 * 1000);
  USBBULK_SetWriteTimeout(hDevice, 3600 * 1000);
  _GetNumberFromConsole("Enter the packet size in bytes (default: 0x2000): ", &NumBytes);
  _GetNumberFromConsole("Enter the number of packets    (default: 512): ", &NumReps);
  printf("\n Transferring %d KB (%d * %d KB)\n", (NumReps * NumBytes) >> 10, NumReps, NumBytes >> 10);
  t = timeGetTime();
  _TestSpeed(hDevice, SUBCMD_SPEED_WRITE, NumReps, NumBytes);
  tWrite = timeGetTime() - t;
  if (r < 0) {
    printf("ERROR: Failed to test write speed.\n");
  } else {
    printf("%d KB written (host -> device) in %dms ! (%.1f KB/s)\n", (NumReps * NumBytes) >> 10, (int)tWrite, (float)(NumReps * NumBytes) / tWrite);
    if (tWrite < (int)((NumBytes * NumReps) / 50000)) {
      printf("WARNING: Measured speed is too high (> 50 MB/s). Maybe an error occurred during speed test.\n");
    }
  }
  if (r) {
    goto End;
  }
  //
  // Prepare read command and send to device
  //
  t = timeGetTime();
  r = _TestSpeed(hDevice, SUBCMD_SPEED_READ, NumReps, NumBytes);
  tRead = timeGetTime() - t;
  if (r) {
    printf("ERROR: Failed to test read speed.\n");
  } else {
    printf("%d KB read (device -> host) in %dms ! (%.1f KB/s)\n", (NumReps * NumBytes) >> 10, (int)tRead, (float)(NumReps * NumBytes) / tRead);
    if (tRead < (int)((NumBytes * NumReps) / 50000)) {
      printf("WARNING: Measured speed is too high (> 50 MB/s). Maybe an error occurred during speed test.\n");
    }
  }

End:
  USBBULK_Close(hDevice);
  return r;
}

/*********************************************************************
*
*       _TestNet
*/
static int _TestNet(unsigned DevIndex, int SizeInc, int DelayInc, int DelayMax) {
  U32 AllocSize;
  U8* pWrite;
  U8* pRead;
  U32 NumBytes;
  U32 NumBytesRead;
  int Delay;
  int i;
  int r = -1;
  USB_BULK_HANDLE hDevice;

  hDevice = USBBULK_Open(DevIndex);
  if (hDevice == 0) {
    printf("Unable to connect to USB BULK device\n");
    return 1;
  }
  //
  // Allocate memory
  //
  AllocSize = (MAX_NUM_BYTES_TEST_NET * 2) + 16;
  pWrite = malloc(AllocSize);
  if (pWrite == NULL) {
    printf("ERROR: Could not alloc memory.\n");
    return -1;        // Error
  }
  pRead = pWrite + (AllocSize / 2);
  //
  // Fill memory with test pattern
  //
  for (i = 0; i < MAX_NUM_BYTES_TEST_NET; i++) {
    *(pWrite + i + 4) = i % 255;
  }
  //
  // Perform test in a loop
  //
  for (Delay = 0; Delay <= DelayMax; Delay += DelayInc) {
    for (NumBytes = 1; NumBytes <= MAX_NUM_BYTES_TEST_NET; NumBytes += SizeInc) {
      if (Delay || (NumBytes > 1)) {
        printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
      }
      printf("  %4d bytes with delay %2d", (int)NumBytes, Delay);
      fflush(stdout);
      *(pWrite + 0) = CMD_TEST;
      *(pWrite + 1) = Delay;
      *(pWrite + 2) = (U8)((NumBytes >> 0) & 0xFF);
      *(pWrite + 3) = (U8)((NumBytes >> 8) & 0xFF);
      USBBULK_Write(hDevice, pWrite, (NumBytes + 4));
      NumBytesRead = USBBULK_Read(hDevice, pRead, (NumBytes + 1));
      if (NumBytesRead != (NumBytes + 1)) {
        printf("ERROR: Communication error (Expected %d bytes, received %d)!\n", (int)(NumBytes + 1), (int)NumBytesRead);
        goto Done;
      }
      if (*(pRead + NumBytes) != 0) {
        printf("ERROR: Failed to transfer data!\n");
        goto Done;
      }
      if (memcmp(pWrite + 4, pRead, NumBytes) != 0) {
        printf("ERROR: Verification of data failed!\n");
        goto Done;
      }
      if (_kbhit() != 0) {
        _getch();
        printf("\nTest canceled by user!\n");
        goto Done;    // Canceled by user
      }
    }
  }
  printf("\nTest completed successfully!\n");
  r = 0;              // O.K.
  //
  // Free the allocated memory
  //
Done:
  if (pWrite) {
    free(pWrite);
  }
  return r;
}

/*********************************************************************
*
*       _GetDeviceId
*
*/
static unsigned _GetDeviceId(void) {
  U32      DeviceMask;
  char     Restart;
  char     Msg = 0;
  unsigned i;
  unsigned NumDevices = 0;
  unsigned DeviceId;
  USB_BULK_HANDLE hDevice;
  char            acName[256];
  char            ac[20];
  char          * pEnd = NULL;
  char          * pEndExpected = NULL;
  do {
    Restart = 'N';
    for (;;) {
      NumDevices = USBBULK_GetNumAvailableDevices(&DeviceMask);
      if (NumDevices) {
        break;
      }
      if (Msg == 0) {
        Msg = 1;
        printf("Waiting for USB BULK devices to connect....\n");
      }
      Sleep(100);
    }
    printf("\nFound %d %s\n", NumDevices, NumDevices == 1 ? "device" : "devices");
    printf("Mask = %x\n", DeviceMask);
    for (i = 0; i < NumDevices; i++) {
      if (DeviceMask & (1 << i)) {
        hDevice = USBBULK_Open(i);
        if (hDevice == 0) {
          printf("Can't open device %d:\n", i);
          continue;
        }
        printf("Found the following device %d:\n", i);
        acName[0] = 0;
        USBBULK_GetVendorName(hDevice, acName, sizeof(acName));
        printf("  Vendor Name : %s\n", acName);
        acName[0] = 0;
        USBBULK_GetProductName(hDevice, acName, sizeof(acName));
        printf("  Product Name: %s\n", acName);
        if (strcmp(acName, PRODUCT_NAME) != 0) {
          printf("\n  WARNING: Expected Product Name is: \"%s\". Did you use the correct sample application on the target side? \n\n", PRODUCT_NAME);
        }
        acName[0] = 0;
        USBBULK_GetSN(hDevice, (unsigned char *)acName, sizeof(acName));
        printf("  Serial no.  : %s\n", acName);
        USBBULK_Close(hDevice);
      }
    }
    printf("To which device do you want to connect?\nPlease type in device number (e.g. '0' for the first device, q/a for abort):");
    _ConsoleGetLine(ac, sizeof(ac));
    pEndExpected = &ac[0] +strlen(ac);
    DeviceId = strtol(&ac[0], &pEnd, 0);
    if ((pEnd != pEndExpected)) {
      printf("Invalid device id was entered!!!!\n");
      if ((toupper(ac[0]) == 'Q') || (toupper(ac[0]) == 'A')) {
        DeviceId = -1;
        break;
      } else {
        Restart = 'Y';
        continue;
      }
    }
    if (DeviceId < USBBULK_MAX_DEVICES) {
      break;
    }
  } while (Restart == 'Y');
  return DeviceId;
}


/*********************************************************************
*
*       _InitApp
*
*/
static void _InitApp(void) {
  USBBULK_Init(NULL, NULL);
  //
  //  Add all allowed devices via (VendorId, ProductId)
  //
  USBBULK_AddAllowedDeviceItem(0x8765, 0x1234);
  USBBULK_AddAllowedDeviceItem(0x8765, 0x1240);
  //
  // Retrieve some version information from the USBBULK module
  //
  _ShowDriverInfo();
}


/*********************************************************************
*
*       public code
*
**********************************************************************
*/

/*********************************************************************
*
*       main
*
*/
int main(int argc, char* argv[]) {
  int      r;
  unsigned DeviceId;
  unsigned Choice;
#if _WIN32
  char     ChangeTransferSize;
  U32      TransferSize;
  USB_BULK_HANDLE hDevice;
#endif
  char            ac[256];

  _InitApp();
  DeviceId = _GetDeviceId();
#if _WIN32
  timeBeginPeriod(1);
  hDevice = USBBULK_Open(DeviceId);
  printf("Current Read  transfer size down is %d\n", USBBULK_GetReadMaxTransferSizeDown(hDevice));
  printf("Current Write transfer size down is %d\n", USBBULK_GetWriteMaxTransferSizeDown(hDevice));
  printf("Do you want to change these (y/n)?\n");
  printf("Your choice: ");
  _ConsoleGetLine(ac, sizeof(ac));
  ChangeTransferSize = toupper(ac[0]);
  if ((ChangeTransferSize != 'Y') && (ChangeTransferSize != 'N')) {
    ChangeTransferSize = 'N';
  }
  if (ChangeTransferSize == 'Y') {
    printf("Enter the new Read transfer size down size: ");
    _ConsoleGetLine(ac, sizeof(ac));
    sscanf(ac, "%d", &TransferSize);
    USBBULK_SetReadMaxTransferSizeDown(hDevice, TransferSize);
    printf("Enter the new Write transfer size down size: ");
    _ConsoleGetLine(ac, sizeof(ac));
    sscanf(ac, "%d", &TransferSize);
    USBBULK_SetWriteMaxTransferSizeDown(hDevice, TransferSize);
  }
  USBBULK_Close(hDevice);
#endif
  printf("What kind of test would you like to run?\n");
  printf("(1)Performance test\n");
  printf("(2)Stability test\n");
  printf("Your choice: ");
  _ConsoleGetLine(ac, sizeof(ac));
  sscanf(ac, "%d", &Choice);
  switch (Choice) {
  case 1:
    printf("Starting performance test...\n");
    r = _PerformanceTest(DeviceId);
    if (r) {
      goto End;
    }
    break;
  case 2:
    printf("Performing quick test...\n");
    r = _TestNet(DeviceId, 47, 5, 5);
    if (r == 0) {
      printf("Performing intensive test...\n");
      r = _TestNet(DeviceId, 1, 2, 10);
    }
    if (r) {
      goto End;
    }
    break;
  default:
    r = 1;
    break;
  }
End:
  if (r == 0) {
    printf("Communication with USB BULK device was successful!\n");
  } else {
    printf("Communication with USB BULK device was not successful!\n");
  }
  USBBULK_Exit();
#if _WIN32
  timeEndPeriod(1);
#endif
  printf("Press enter to exit.");
  _ConsoleGetLine(ac, sizeof(ac));
  return r;
}

/******************************* End of file ************************/
