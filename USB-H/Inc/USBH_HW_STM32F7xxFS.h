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
File        : USBH_HW_STM32F7xxFS.h
Purpose     : Header for the STM32F7xx/F4xx FullSpeed emUSB Host driver
-------------------------- END-OF-HEADER -----------------------------
*/

#ifndef USBH_HW_STM32F7XX_FS_H_
#define USBH_HW_STM32F7XX_FS_H_


#if defined(__cplusplus)
  extern "C" {                 // Make sure we have C-declarations in C++ programs
#endif

U32            USBH_STM32F7_FS_Add(void * pBase);

#if defined(__cplusplus)
  }
#endif

#endif // USBH_HW_STM32F7XX_FS_H_

/*************************** End of file ****************************/
