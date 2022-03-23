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
File        : USBH_PrinterClass.h
Purpose     : API of the USB host stack
-------------------------- END-OF-HEADER -----------------------------
*/

#ifndef USBH_PRINTERCLASS_H_
#define USBH_PRINTERCLASS_H_

#include "USBH.h"
#include "SEGGER.h"

#if defined(__cplusplus)
  extern "C" {                 // Make sure we have C-declarations in C++ programs
#endif



/*********************************************************************
*
*       USBH_PRINTER
*/
typedef U32 USBH_PRINTER_HANDLE;

typedef struct {
  U16  VendorId;
  U16  ProductId;
  U16  bcdDevice;
  U8   acSerialNo[255];
} USBH_PRINTER_DEVICE_INFO;


U8                  USBH_PRINTER_Init(void);
void                USBH_PRINTER_Exit(void);
USBH_PRINTER_HANDLE USBH_PRINTER_Open(const char * sName);
USBH_PRINTER_HANDLE USBH_PRINTER_OpenByIndex(unsigned Index);
USBH_STATUS         USBH_PRINTER_Write(USBH_PRINTER_HANDLE hDevice, const U8 * pData, unsigned NumBytes);
USBH_STATUS         USBH_PRINTER_Read(USBH_PRINTER_HANDLE hDevice, U8 * pData, unsigned NumBytes);
USBH_STATUS         USBH_PRINTER_GetPortStatus(USBH_PRINTER_HANDLE hDevice, U8 * pStatus);
USBH_STATUS         USBH_PRINTER_ExecSoftReset(USBH_PRINTER_HANDLE hDevice);
USBH_STATUS         USBH_PRINTER_GetDeviceId(USBH_PRINTER_HANDLE hDevice, U8 * pData, unsigned NumBytes);
USBH_STATUS         USBH_PRINTER_Close(USBH_PRINTER_HANDLE hDevice);
int                 USBH_PRINTER_GetNumDevices(void);
void                USBH_PRINTER_RegisterNotification(USBH_NOTIFICATION_FUNC * pfNotification, void * pContext);
void                USBH_PRINTER_ConfigureTimeout(U32 Timeout);
USBH_STATUS         USBH_PRINTER_GetDeviceInfo(USBH_PRINTER_HANDLE hDevice, USBH_PRINTER_DEVICE_INFO * pDevInfo);

#if defined(__cplusplus)
  }
#endif

#endif // USBH_PRINTERCLASS_H_

/*************************** End of file ****************************/
