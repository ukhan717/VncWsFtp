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
File    : USB_ECM.h
Purpose : Public header of the USB Ethernet Control Model (ECM)
          The Ethernet Control Model (ECM) is one of the
          Communication Device Class protocols defined by usb.org to
          create a virtual Ethernet connection between a USB
          device and a host PC.
--------  END-OF-HEADER  ---------------------------------------------
*/

#ifndef USB_ECM_H          /* Avoid multiple inclusion */
#define USB_ECM_H

#include "USB.h"
#include "SEGGER.h"
#include "USB_Driver_IP_NI.h"

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

/*********************************************************************
*
*       defines
*
**********************************************************************
*/

/*********************************************************************
*
*       Types
*
**********************************************************************
*/

/*********************************************************************
*
*       USB_ECM_INIT_DATA
*
*   Description
*     Initialization data for ECM interface.
*
*   Additional information
*     This structure holds the endpoints that should be used by
*     the ECM interface (EPin, EPOut and EPInt). Refer to USBD_AddEP()
*     for more information about how to add an endpoint.
*/
typedef struct USB_ECM_INIT_DATA {
  U8   EPIn;      // Endpoint for sending data to the host.
  U8   EPOut;     // Endpoint for receiving data from the host.
                  // The buffer associated to this endpoint must be big enough to hold a complete IP packet.
  U8   EPInt;     // Endpoint for sending status information.
  const USB_IP_NI_DRIVER_API * pDriverAPI;   // Pointer to the Network interface driver API.
                                             // See USB_IP_NI_DRIVER_API.
  USB_IP_NI_DRIVER_DATA        DriverData;   // Configuration data for the network interface driver.
  unsigned                     DataInterfaceNum;
} USB_ECM_INIT_DATA;

/*********************************************************************
*
*       API functions
*
**********************************************************************
*/
void USBD_ECM_Add  (const USB_ECM_INIT_DATA * pInitData);
void USBD_ECM_Task (void);


#if defined(__cplusplus)
  }              /* Make sure we have C-declarations in C++ programs */
#endif

#endif                 /* Avoid multiple inclusion */

/*************************** End of file ****************************/
