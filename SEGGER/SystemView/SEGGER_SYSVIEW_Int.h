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
File    : SEGGER_SYSVIEW_Int.h
Purpose : SEGGER SystemView internal header.
Revision: $Rev: 9599 $
*/

#ifndef SEGGER_SYSVIEW_INT_H
#define SEGGER_SYSVIEW_INT_H

/*********************************************************************
*
*       #include Section
*
**********************************************************************
*/

#include "SEGGER_SYSVIEW.h"
#include "SEGGER_SYSVIEW_Conf.h"
#include "SEGGER_SYSVIEW_ConfDefaults.h"

#ifdef __cplusplus
extern "C" {
#endif


/*********************************************************************
*
*       Private data types
*
**********************************************************************
*/
//
// Commands that Host can send to target
//
typedef enum {
  SEGGER_SYSVIEW_COMMAND_ID_START = 1,
  SEGGER_SYSVIEW_COMMAND_ID_STOP,
  SEGGER_SYSVIEW_COMMAND_ID_GET_SYSTIME,
  SEGGER_SYSVIEW_COMMAND_ID_GET_TASKLIST,
  SEGGER_SYSVIEW_COMMAND_ID_GET_SYSDESC,
  SEGGER_SYSVIEW_COMMAND_ID_GET_NUMMODULES,
  SEGGER_SYSVIEW_COMMAND_ID_GET_MODULEDESC,
  // Extended commands: Commands >= 128 have a second parameter
  SEGGER_SYSVIEW_COMMAND_ID_GET_MODULE = 128
} SEGGER_SYSVIEW_COMMAND_ID;

#ifdef __cplusplus
}
#endif

#endif

/*************************** End of file ****************************/
