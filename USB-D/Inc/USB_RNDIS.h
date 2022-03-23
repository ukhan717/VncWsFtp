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
File    : USB_RNDIS.h
Purpose : Public header of the Remote Network Driver Interface
          Specification (RNDIS)
--------  END-OF-HEADER  ---------------------------------------------
*/

#ifndef USB_RNDIS_H          /* Avoid multiple inclusion */
#define USB_RNDIS_H

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
*       USB_RNDIS_INIT_DATA
*
*   Description
*     Structure which stores the parameters of the RNDIS interface.
*
*   Additional information
*     This structure holds the endpoints that should be used by
*     the RNDIS interface (EPin, EPOut and EPInt). Refer to USBD_AddEP()
*     for more information about how to add an endpoint.
*/
typedef struct USB_RNDIS_INIT_DATA {
  U8   EPIn;      // Endpoint for sending data to the host.
  U8   EPOut;     // Endpoint for receiving data from the host.
  U8   EPInt;     // Endpoint for sending status information.
  const USB_IP_NI_DRIVER_API * pDriverAPI;   // Pointer to the Network interface driver API.
  USB_IP_NI_DRIVER_DATA        DriverData;   // Configuration data for the network interface driver.
  unsigned                     DataInterfaceNum;  // Internal use
} USB_RNDIS_INIT_DATA;


/*********************************************************************
*
*       USB_RNDIS_DEVICE_INFO
*
*   Description
*     Device information that must be provided by the application via
*     the function USBD_RNDIS_SetDeviceInfo() before the USB stack is
*     started using USBD_Start().
*/
typedef struct {
  U32  VendorId;      // A 24-bit Organizationally Unique Identifier (OUI)
                      // of the vendor. This is the same value as the one
                      // stored in the first 3 bytes of a HW (MAC) address.
                      // Only the least significant 24 bits of the retuned
                      // value are used.
  char *sDescription; // 0-terminated ASCII string describing the device.
                      // The string is then sent to the host system.
  U16  DriverVersion; // 16-bit value representing the firmware version.
                      // The high-order byte specifies the major version
                      // and the low-order byte the minor version.
} USB_RNDIS_DEVICE_INFO;

/*********************************************************************
*
*       API functions
*
**********************************************************************
*/
void USBD_RNDIS_Add            (const USB_RNDIS_INIT_DATA * pInitData);
void USBD_RNDIS_Task           (void);
void USBD_RNDIS_SetDeviceInfo  (const USB_RNDIS_DEVICE_INFO *pDeviceInfo);

/*********************************************************************
*
*       Functions defined in the application
*
**********************************************************************
*/
U32          USB_RNDIS_GetVendorId     (void);
const char * USB_RNDIS_GetDescription  (void);
U16          USB_RNDIS_GetDriverVersion(void);

/*********************************************************************
*
*       Internal functions, do not use them!
*
**********************************************************************
*/
void USB__RNDIS_Init             (const USB_RNDIS_INIT_DATA * pInitData);
int  USB__RNDIS_DriverWritePacket(const void * pData, U32 NumBytes);
int  USB__RNDIS_OnClassRequest   (const USB_SETUP_PACKET * pSetupPacket);
void USB__RNDIS_OnRxEP0          (const U8 * pData, unsigned NumBytes);
void USB__RNDIS_OnRxEP           (unsigned Events, void * pContext);
void USB__RNDIS_OnStateChange    (void *pDummy, U8 NewState);

#if defined(__cplusplus)
  }              /* Make sure we have C-declarations in C++ programs */
#endif

#endif                 /* Avoid multiple inclusion */

/*************************** End of file ****************************/
