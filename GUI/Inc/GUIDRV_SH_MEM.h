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
File        : GUIDRV_SH_MEM.h
Purpose     : Interface definition for GUIDRV_SH_MEM driver
---------------------------END-OF-HEADER------------------------------
*/

#ifndef GUIDRV_SH_MEM_H
#define GUIDRV_SH_MEM_H

/*********************************************************************
*
*       Display drivers
*/
//
// Addresses
//
extern const GUI_DEVICE_API GUIDRV_Win_API;

extern const GUI_DEVICE_API GUIDRV_SH_MEM_API;
extern const GUI_DEVICE_API GUIDRV_SH_MEM_API_OXY;
extern const GUI_DEVICE_API GUIDRV_SH_MEM_API_OSX;
extern const GUI_DEVICE_API GUIDRV_SH_MEM_API_OSY;

extern const GUI_DEVICE_API GUIDRV_SH_MEM_3_API;
extern const GUI_DEVICE_API GUIDRV_SH_MEM_3_API_OXY;
extern const GUI_DEVICE_API GUIDRV_SH_MEM_3_API_OSX;
extern const GUI_DEVICE_API GUIDRV_SH_MEM_3_API_OSY;

//
// Macros to be used in configuration files
//
#if defined(WIN32) && !defined(LCD_SIMCONTROLLER)

  #define GUIDRV_SH_MEM            &GUIDRV_Win_API
  #define GUIDRV_SH_MEM_OXY        &GUIDRV_Win_API
  #define GUIDRV_SH_MEM_OSX        &GUIDRV_Win_API
  #define GUIDRV_SH_MEM_OSY        &GUIDRV_Win_API

  #define GUIDRV_SH_MEM_3          &GUIDRV_Win_API
  #define GUIDRV_SH_MEM_3_OXY      &GUIDRV_Win_API
  #define GUIDRV_SH_MEM_3_OSX      &GUIDRV_Win_API
  #define GUIDRV_SH_MEM_3_OSY      &GUIDRV_Win_API

#else

  #define GUIDRV_SH_MEM            &GUIDRV_SH_MEM_API
  #define GUIDRV_SH_MEM_OXY        &GUIDRV_SH_MEM_API_OXY
  #define GUIDRV_SH_MEM_OSX        &GUIDRV_SH_MEM_API_OSX
  #define GUIDRV_SH_MEM_OSY        &GUIDRV_SH_MEM_API_OSY

  #define GUIDRV_SH_MEM_3          &GUIDRV_SH_MEM_3_API
  #define GUIDRV_SH_MEM_3_OXY      &GUIDRV_SH_MEM_3_API_OXY
  #define GUIDRV_SH_MEM_3_OSX      &GUIDRV_SH_MEM_3_API_OSX
  #define GUIDRV_SH_MEM_3_OSY      &GUIDRV_SH_MEM_3_API_OSY

#endif

#define GUIDRV_SH_MEM_8BITMODE  0
#define GUIDRV_SH_MEM_10BITMODE 1

typedef struct {
  unsigned Period;           // Period used for toggling VCOM
  unsigned ExtMode;          // Setting of EXTMODE configuration pin
  unsigned BitMode;          // 8- or 10-bit line addressing
  unsigned AddressBitOrder;  // Either LSB first (0) or MSB first (1)
  void (* pfToggleVCOM)(void);
} CONFIG_SH_MEM;

/*********************************************************************
*
*       Prototypes
*
**********************************************************************
*/
void GUIDRV_SH_MEM_SetBus8  (GUI_DEVICE * pDevice, GUI_PORT_API * pHW_API);
void GUIDRV_SH_MEM_Config   (GUI_DEVICE * pDevice, CONFIG_SH_MEM * pConfig);
void GUIDRV_SH_MEM_3_SetBus8(GUI_DEVICE * pDevice, GUI_PORT_API * pHW_API);
void GUIDRV_SH_MEM_3_Config (GUI_DEVICE * pDevice, CONFIG_SH_MEM * pConfig);

#endif

/*************************** End of file ****************************/
