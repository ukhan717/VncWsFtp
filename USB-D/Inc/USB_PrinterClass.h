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
File    : USB_PrinterClass.h
Purpose : Sample implementation of USB printer device class
----------Literature--------------------------------------------------
Universal Serial Bus Device Class Definition for Printing Devices
Version 1.1 January 2000
--------  END-OF-HEADER  ---------------------------------------------
*/
#ifndef USB_PRINTERCLASS_H__
#define USB_PRINTERCLASS_H__

#include "USB.h"

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

/*********************************************************************
*
*       USB_PRINTER_GET_DEVICE_ID_STRING
*
*  Description
*    The library calls this function when the USB host requests
*    the printer's identification string. This string shall confirm to
*    the IEEE 1284 Device ID Syntax.
*
*  Return value
*    Pointer to the ID string.
*
*  Additional information
*   The return string shall confirm to the IEEE 1284 Device ID.
*/
typedef const char * USB_PRINTER_GET_DEVICE_ID_STRING (void);

/*********************************************************************
*
*       USB_PRINTER_ON_DATA_RECEIVED
*
*  Description
*    This function is called when data arrives from USB host.
*
*  Parameters
*    pData     : Pointer to the data.
*    NumBytes  : Data length.
*
*  Return value
*    == 0: More data can be accepted
*    != 0: No more data can be accepted, in this case a stall will be sent back to the host.
*/
typedef int          USB_PRINTER_ON_DATA_RECEIVED     (const U8 * pData, unsigned NumBytes);

/*********************************************************************
*
*       USB_PRINTER_GET_HAS_NO_ERROR
*
*  Description
*    This function should return a non-zero value if the printer has no error.
*
*  Return value
*    == 0: No error.
*    != 0: Error condition present.
*/
typedef U8           USB_PRINTER_GET_HAS_NO_ERROR     (void);

/*********************************************************************
*
*       USB_PRINTER_GET_IS_SELECTED
*
*  Description
*    This function should return a non-zero value if the printer is selected.
*
*  Return value
*    == 0: Not selected.
*    != 0: Selected.
*/
typedef U8           USB_PRINTER_GET_IS_SELECTED      (void);

/*********************************************************************
*
*       USB_PRINTER_GET_IS_PAPER_EMPTY
*
*  Description
*    This function should return a non-zero value if the printer is out of paper.
*
*  Return value
*    == 0: Out of paper.
*    != 0: Has paper.
*/
typedef U8           USB_PRINTER_GET_IS_PAPER_EMPTY   (void);

/*********************************************************************
*
*       USB_PRINTER_ON_RESET
*
*  Description
*    The library calls this function if the USB host sends a soft reset command.
*/
typedef void         USB_PRINTER_ON_RESET             (void);

/*********************************************************************
*
*       USB_PRINTER_API
*
*  Description
*    Initialization structure that is needed when adding a printer
*    interface to emUSB-Device. It holds pointer to callback functions
*    the interface invokes when it processes request from USB host.
*/
typedef struct USB_PRINTER_API {
  USB_PRINTER_GET_DEVICE_ID_STRING  *pfGetDeviceIdString; // The library calls this function when the USB host
                                                          // requests the printer's identification string.
  USB_PRINTER_ON_DATA_RECEIVED      *pfOnDataReceived;    // This function is called when data arrives from USB host.
  USB_PRINTER_GET_HAS_NO_ERROR      *pfGetHasNoError;     // This function should return a non-zero value if the printer has no error.
  USB_PRINTER_GET_IS_SELECTED       *pfGetIsSelected;     // This function should return a non-zero value if the printer is selected
  USB_PRINTER_GET_IS_PAPER_EMPTY    *pfGetIsPaperEmpty;   // This function should return a non-zero value if the printer is out of paper.
  USB_PRINTER_ON_RESET              *pfOnReset;           // The library calls this function if the USB host sends a soft reset command.
} USB_PRINTER_API;

void   USB_PRINTER_Init        (USB_PRINTER_API * pAPI);
void   USB_PRINTER_Task        (void);
void   USB_PRINTER_TaskEx      (void);
int    USB_PRINTER_Read        (      void * pData, unsigned NumBytes);
int    USB_PRINTER_ReadTimed   (      void * pData, unsigned NumBytes, unsigned ms);
int    USB_PRINTER_Receive     (      void * pData, unsigned NumBytes);
int    USB_PRINTER_ReceiveTimed(      void * pData, unsigned NumBytes, unsigned ms);
int    USB_PRINTER_Write       (const void * pData, unsigned NumBytes);
int    USB_PRINTER_WriteTimed  (const void * pData, unsigned NumBytes, int ms);
void   USB_PRINTER_ConfigIRQProcessing(void);

#if defined(__cplusplus)
  }              /* Make sure we have C-declarations in C++ programs */
#endif


#endif

/*************************** End of file ****************************/

