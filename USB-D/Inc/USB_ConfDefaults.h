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
File    : USB_ConfDefaults.h
Purpose : This file contains the default values for emUSB-Device
          configuration parameters. To allow easy updates please
          do not change the parameters here, but add them in the
          USB_Conf.h file, the defines there will replace the default
          values.
--------------------------  END-OF-HEADER  ---------------------------
*/

#ifndef   USB_CONFDEFAULTS_H
#define   USB_CONFDEFAULTS_H

#include "USB_Conf.h"

//
// Set default debug level.
// Debug level: 0: no checks and no logs, 1: Support "Panic" checks, 2: Support warn & log
//
#ifndef  USB_DEBUG_LEVEL
  #if defined(DEBUG) && DEBUG > 0
    #define USB_DEBUG_LEVEL     2
  #else
    #define USB_DEBUG_LEVEL     0
  #endif
#endif

//
// Should log messages be supported?
//
#ifndef     USB_SUPPORT_LOG
  #if       USB_DEBUG_LEVEL > 1
    #define USB_SUPPORT_LOG   1
  #else
    #define USB_SUPPORT_LOG   0
  #endif
#endif

//
// Should warning messages be supported?
//
#ifndef     USB_SUPPORT_WARN
  #if       USB_DEBUG_LEVEL > 1
    #define USB_SUPPORT_WARN  1
  #else
    #define USB_SUPPORT_WARN  0
  #endif
#endif

//
// The maximum current consumption of the device in x*2 mA (e.g. 50 means 100 mA).
//
#ifndef   USB_MAX_POWER
  #define USB_MAX_POWER  50
#endif

//
// Should isochronous transfers be supported?
// ISO is only required for audio and video classes.
//
#ifndef   USB_SUPPORT_TRANSFER_ISO
  #define USB_SUPPORT_TRANSFER_ISO 0
#endif

//
// Maximum number of endpoint descriptors.
//
#ifndef   USB_NUM_EPS
  #define USB_NUM_EPS 8u
#endif

//
// Maximum number of interface descriptors.
//
#ifndef   USB_MAX_NUM_IF
  #define USB_MAX_NUM_IF 4u
#endif

//
// Maximum number of alternative interface descriptors.
//
#ifndef   USB_MAX_NUM_ALT_IF
  #define USB_MAX_NUM_ALT_IF 2u
#endif

//
// Maximum number of interface association descriptors.
//
#ifndef   USB_MAX_NUM_IAD
  #define USB_MAX_NUM_IAD 3u
#endif

//
// Maximum number of Microsoft OS descriptors.
//
#ifndef   USB_MAX_NUM_MS_DESC
  #define USB_MAX_NUM_MS_DESC   3u
#endif

//
// Maximum number of callbacks which can be registered for EP0 via USBD_SetOnRxEP0().
// Normally one class component registers one EP0 receive callback.
//
#ifndef   USB_MAX_NUM_COMPONENTS
  #define USB_MAX_NUM_COMPONENTS 3
#endif

//
// Specifies the number of additional event objects.
// The stack allocates as many events as there are configured endpoints (USB_NUM_EPS),
// the additional events are for the use by class modules through USB__AllocEvent().
//
#ifndef   USB_EXTRA_EVENTS
  #define USB_EXTRA_EVENTS 0u
#endif

//
// Maximum number of string descriptors.
//
#ifndef USB_MAX_STRING_DESC
  #define USB_MAX_STRING_DESC   (USB_MAX_NUM_IF + USB_MAX_NUM_ALT_IF)
#endif

//
// Support other speed descriptor?
// Only necessary if the target supports high-speed.
//
#ifndef   USB_OTHER_SPEED_DESC
  #define USB_OTHER_SPEED_DESC         USB_SUPPORT_HIGH_SPEED
#endif

//
// Support USB test mode?
//
#ifndef   USB_SUPPORT_TEST_MODE
  #define USB_SUPPORT_TEST_MODE        0
#endif

//
// Support profiling via SystemView?
//
#ifndef   USBD_SUPPORT_PROFILE
  #define USBD_SUPPORT_PROFILE  0
#endif

//
// Compatibility define for migration from emUSB-Device V2 DevInfo to V3 DevInfo
//
#ifndef USB_V2_V3_MIGRATION_DEVINFO
  #define USB_V2_V3_MIGRATION_DEVINFO   0
#endif

//
// Compatibility define for migration from emUSB-Device V2 config method to V3 config method.
//
#ifndef USB_V2_V3_MIGRATION_CONFIG
  #define USB_V2_V3_MIGRATION_CONFIG    0
#endif

//
// Chooses which RTOS layer version should be used
// 1 - new API version.
// 0 - old (deprecated) API version
//
#ifndef USBD_OS_LAYER_EX
  #define USBD_OS_LAYER_EX       1
#endif

//
// If set emUSB-Device will use the functions USBD_X_EnableInterrupt/USBD_X_DisableInterrupt
// instead of disabling/enabling the interrupts globally. Those functions only disable/enable the USB interrupt.
// The functions are MCU specific and must be defined in the corresponding USB_Config_*.c file.
//
#ifndef USBD_OS_USE_USBD_X_INTERRUPT
  #define USBD_OS_USE_USBD_X_INTERRUPT  0
#endif

//
// Enables the use of the MSD+MTP combination with which the device
// can be automatically recognized by Windows as MTP and by Linux/Max OS as MSD.
// Disabled by default to save memory.
//
#ifndef USB_SUPPORT_MSD_MTP_COMBINATION
  #define USB_SUPPORT_MSD_MTP_COMBINATION 0
#endif

//
// If set to 1 the deprecated BSP_CACHE_* routines can be used
// instead of the new SEGGER_CACHE_CONFIG set via USBD_SetCacheConfig
//
#ifndef USBD_USE_LEGACY_CACHE_ROUTINES
  #define USBD_USE_LEGACY_CACHE_ROUTINES 0
#endif

//
// Should class requests be supported?
// Note: setting this to 0 is not recommended and is only intended for absolutely minimal configurations.
//
#ifndef   USB_SUPPORT_CLASS_REQUESTS
  #define USB_SUPPORT_CLASS_REQUESTS 1
#endif

//
// Should vendor requests be supported?
// Note: setting this to 0 is not recommended and is only intended for absolutely minimal configurations.
//
#ifndef   USB_SUPPORT_VENDOR_REQUESTS
  #define USB_SUPPORT_VENDOR_REQUESTS 1
#endif

//
// Should status requests be supported?
// Note: setting this to 0 is not recommended and is only intended for absolutely minimal configurations.
//
#ifndef USB_SUPPORT_STATUS
  #define USB_SUPPORT_STATUS 1
#endif

//
// Macro overwrite for memcpy.
//
#ifndef   USB_MEMCPY
  #include <string.h>
  #define USB_MEMCPY(pD, pS, NumBytes)  memcpy((pD), (pS), (NumBytes))
#endif

//
// Macro overwrite for memset.
//
#ifndef   USB_MEMSET
  #include <string.h>
  #define USB_MEMSET(p, c, NumBytes)  memset((p), (c), (NumBytes))
#endif

//
// Macro overwrite for memcmp.
//
#ifndef   USB_MEMCMP
  #include <string.h>
  #define USB_MEMCMP(p1, p2, NumBytes)  memcmp((p1), (p2), (NumBytes))
  //lint -sem(memcmp, pure)          N:100
#endif

//
// Macro used in order to avoid warnings for undefined parameters
//
#ifndef USB_USE_PARA
  #define USB_USE_PARA(para) (void)(para)
#endif

#endif

/*************************** End of file ****************************/
