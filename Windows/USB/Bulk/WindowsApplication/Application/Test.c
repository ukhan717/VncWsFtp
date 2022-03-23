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
File    : Test.c
Purpose : USB BULK Test Application
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
#define APP_TITLE     "USB BULK Sample Test"
#define PRODUCT_NAME  "Bulk test"

#define SIZEOF_BUFFER  0x4000
#define INC_TEST_START  1
#define INC_TEST_END    1024

/*********************************************************************
*
*       static data
*
**********************************************************************
*/
static unsigned char _acTxBuffer[SIZEOF_BUFFER];
static unsigned char _acRxBuffer[SIZEOF_BUFFER];

/*********************************************************************
*
*       static code
*
**********************************************************************
*/

#ifdef _WIN32

#include <windows.h>

#else

#include <poll.h>
#include <time.h>
#include <sys/time.h>

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
*       GetTickCount
*/
static unsigned GetTickCount(void) {
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
*       _SendBuffer
*
*/
static int _SendBuffer(USB_BULK_HANDLE hDevice, unsigned char* pData, int Len) {
  int  r;
  char ac[2];

  if (Len) {
    printf("Writing %d bytes\n", Len);
    //
    //  Send 16bit Len, MSB first to be compatible with SampleApp
    //
    ac[0] = (Len >> 8);
    ac[1] = (Len & 0xFF);
    r = USBBULK_Write(hDevice, ac, 2);
    if (r == 0) {
      printf("Could not write to device\n");
      return 0;
    }
    r = USBBULK_Write(hDevice, pData, Len);
    return r;
  } else {
    return 0;
  }
}

/*********************************************************************
*
*       _ReadBuffer
*
*/
static int _ReadBuffer(USB_BULK_HANDLE hDevice, unsigned char* pDest, int Len) {
  int r;

  printf("Reading %d bytes\n", Len);
  r = USBBULK_Read(hDevice, pDest, Len);
  if (r == 0) {
    printf("Could not read from device (time out)\n");
  }
  return r;
}

/*********************************************************************
*
*       _SendReceive1
*
*/
static int _SendReceive1(USB_BULK_HANDLE hDevice, unsigned char DataTx) {
  unsigned char DataRx;
  int r;

  printf("Writing one byte\n");
  r = USBBULK_Write(hDevice, &DataTx, 1);
  if (r == 0) {
    printf("Could not write to device\n");
  }
  printf("Reading one byte\n");
  r = USBBULK_Read (hDevice, &DataRx, 1);
  if (r == 0) {
    printf("Could not read from device (time out)\n");
    return 1;
  }
  if (DataRx != (DataTx + 1)) {
    printf("Wrong data read\n");
    return 1;
  }
  printf("Operation successful!\n\n");
  return 0;
}

/*********************************************************************
*
*       _Test
*
*/
static int _Test(USB_BULK_HANDLE hDevice) {
  int i;
  int NumBytes;
  int r;
  int t;
  //
  // Do a simple 1 byte test first
  //
  r = _SendReceive1(hDevice, 0x12);
  if (r) {
    return r;
  }
  r = _SendReceive1(hDevice, 0x13);
  if (r) {
    return r;
  }
  //
  // Initially fill buffer
  //
  for (NumBytes = 0; NumBytes < SIZEOF_BUFFER; NumBytes++) {
    _acTxBuffer[NumBytes] = NumBytes % 255;
  }
  //
  // Test different sizes
  //
  for (NumBytes = INC_TEST_START; NumBytes <= INC_TEST_END; NumBytes++) {  // Send and receive various data packets
    r = _SendBuffer(hDevice, _acTxBuffer, NumBytes);
    if (r != NumBytes) {
      printf("Could not write to device (time out)\n");
      return 1;
    }
    r = _ReadBuffer(hDevice, _acRxBuffer, NumBytes);
    if (r != NumBytes) {
      printf("Could not read from device (time out)\n");
      return 1;
    }
    if (memcmp(_acRxBuffer, _acTxBuffer, NumBytes)) {
      printf("Wrong data received\n");
      return 1;
    }
  }
  //
  // Test speed
  //
  printf("Testing speed:");
  t = GetTickCount();
  for (i = 0; i< 500; i++) {
    int NumBytes = 4 * 1024;
    _acTxBuffer[0] = NumBytes >> 8;
    _acTxBuffer[1] = NumBytes & 255;
    if (USBBULK_WriteRead(hDevice, _acTxBuffer, NumBytes + 2, _acTxBuffer, NumBytes) != NumBytes) {
      printf("error reading from USB device\n");
      return 1;
    }
    if (i % 10 == 0) {
      printf(".");
      fflush(stdout);
    }
  }
  t = GetTickCount() - t;
  printf("\nPerformance: %d ms for 4MB\n", t);
  return 0;
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
*       _InitApp
*
*/
static void _InitApp(void) {
  //
  //  Init the USBBULK module
  //
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
*       main
*
* Function description
*/
int main(int argc, char* argv[]) {
  int             r;
  unsigned        DeviceId;
#ifdef _WIN32
  char            ChangeTransferSize;
  U32             TransferSize;
#endif
  USB_BULK_HANDLE hDevice;
  char            ac[256];

  _InitApp();
  DeviceId   = _GetDeviceId();
  hDevice = USBBULK_Open(DeviceId);
  if (hDevice == 0) {
    printf("Unable to connect to USB BULK device\n");
    return 1;
  }
  USBBULK_SetReadTimeout(hDevice, 60 * 1000);
  USBBULK_SetWriteTimeout(hDevice, 60 * 1000);
#ifdef _WIN32
  if (USBBULK_GetDriverVersion()) {
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
      scanf("%d", &TransferSize);
      USBBULK_SetReadMaxTransferSizeDown(hDevice, TransferSize);
      printf("Enter the new Write transfer size down size: ");
      scanf("%d", &TransferSize);
      USBBULK_SetWriteMaxTransferSizeDown(hDevice, TransferSize);
    }
  }
#endif
  r = _Test(hDevice);
  USBBULK_Close(hDevice);
  if (r == 0) {
    printf("Communication with USB BULK device was successful!\n");
  } else {
    printf("Communication with USB BULK device was not successful!\n");
  }
  USBBULK_Exit();
  printf("Press enter to exit.");
  _ConsoleGetLine(ac, sizeof(ac));
  return r;
}

/******************************* End of file ************************/
