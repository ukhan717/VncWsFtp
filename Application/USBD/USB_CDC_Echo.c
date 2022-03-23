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

File    : USB_CDC_Echo.c
Purpose : The sample shows how to create a simple CDC-ACM echo server.

Additional information:
  Preparations:
    On Windows 8.1 and below the "usbser" driver is not automatically
    assigned to the CDC-ACM device. To install the "usbser" driver
    see \Windows\USB\CDC . The device can be accessed via COM port
    emulation programs e.g. PuTTY.

    On Linux no drivers are needed, the device should show up as
    /dev/ttyACM0 or similar. "sudo screen /dev/ttyACM0 115200"
    can be used to access the device.

    On macOS no drivers are needed, the device should show up as
    /dev/tty.usbmodem13245678 or similar. The "screen" terminal
    program can be used to access the device.

  Expected behavior:
    After connecting the USB cable the PC registers a new COM port
    terminal programs are able to open the COM port.
    Any data sent should be received back from the target.

  Sample output:
    The target side does not produce terminal output.
*/

/*********************************************************************
*
*       #include section
*
**********************************************************************
*/
#include <stdio.h>
#include "BSP.h"
#include "USB_CDC.h"
#include "USB.h"

/*********************************************************************
*
*       Information that are used during enumeration
*/
static const USB_DEVICE_INFO _DeviceInfo = {
  0x8765,         // VendorId
  0x1111,         // ProductId
  "Vendor",       // VendorName
  "CDC device",   // ProductName
  "13245678"      // SerialNumber
};

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _OnLineCoding
*
*  Function description
*    Called whenever a "SetLineCoding" Packet has been received
*
*  Notes
*    (1) Context
*        This function is called directly from an ISR in most cases.
*/
static void _OnLineCoding(USB_CDC_LINE_CODING * pLineCoding) {
#if 0
  USBD_Logf_Application("DTERate=%u, CharFormat=%u, ParityType=%u, DataBits=%u\n",
          pLineCoding->DTERate,
          pLineCoding->CharFormat,
          pLineCoding->ParityType,
          pLineCoding->DataBits);
#else
  BSP_USE_PARA(pLineCoding);
#endif
}


/*********************************************************************
*
*       _AddCDC
*
*  Function description
*    Add communication device class to USB stack
*/
static USB_CDC_HANDLE _AddCDC(void) {
  static U8             _abOutBuffer[USB_HS_BULK_MAX_PACKET_SIZE];
  USB_CDC_INIT_DATA     InitData;
  USB_CDC_HANDLE        hInst;

  InitData.EPOut = USBD_AddEP(USB_DIR_OUT, USB_TRANSFER_TYPE_BULK, 0, _abOutBuffer, sizeof(_abOutBuffer));
  InitData.EPIn  = USBD_AddEP(USB_DIR_IN,  USB_TRANSFER_TYPE_BULK, 0, NULL, 0);
  InitData.EPInt = USBD_AddEP(USB_DIR_IN,  USB_TRANSFER_TYPE_INT, 64, NULL, 0);
  hInst = USBD_CDC_Add(&InitData);
  USBD_CDC_SetOnLineCoding(hInst, _OnLineCoding);
  return hInst;
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
  USB_CDC_HANDLE hInst;
  USBD_Init();
  hInst = _AddCDC();
  USBD_SetDeviceInfo(&_DeviceInfo);
  USBD_Start();
  while (1) {
    static char _ac[USB_HS_BULK_MAX_PACKET_SIZE];
    int         NumBytesReceived;

    //
    // Wait for configuration
    //
    while ((USBD_GetState() & (USB_STAT_CONFIGURED | USB_STAT_SUSPENDED)) != USB_STAT_CONFIGURED) {
      BSP_ToggleLED(0);
      USB_OS_Delay(50);
    }
    BSP_SetLED(0);
    //
    // Receive at maximum of 512 Bytes
    // If less data has been received,
    // this should be OK.
    //
    NumBytesReceived = USBD_CDC_Receive(hInst, &_ac[0], sizeof(_ac), 0);
    if (NumBytesReceived > 0) {
      USBD_CDC_Write(hInst, &_ac[0], NumBytesReceived, 0);
    }
  }
}

/**************************** end of file ***************************/
