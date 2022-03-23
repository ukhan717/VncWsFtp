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
File        : SSH_SCP_Conf.h
Purpose     : Configuration file for configurable defines in SSH SCP module
--------  END-OF-HEADER  ---------------------------------------------
*/

#ifndef SSH_SCP_CONF_H
#define SSH_SCP_CONF_H

#ifndef   SSH_SCP_CONFIG_MAX_SESSIONS
  #define SSH_SCP_CONFIG_MAX_SESSIONS  1
#endif

#ifndef   SSH_SCP_CONFIG_PATH_MAX
  #define SSH_SCP_CONFIG_PATH_MAX      260   /*matches emFile*/
#endif

#ifndef   SSH_SCP_INITIAL_WINDOW_SIZE
  #define SSH_SCP_INITIAL_WINDOW_SIZE  (65536*4)
#endif

#endif

/****** End Of File *************************************************/
