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

File        : SSH_SCP_ConfDefaults.h
Purpose     : Defines defaults for most configurable defines used in emSSH's
              SCP server.
              If you want to change a value, please do so in SSH_SCP_Conf.h,
              do NOT modify this file.

*/

#ifndef SSH_SCP_CONFDEFAULTS_H
#define SSH_SCP_CONFDEFAULTS_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************
*
*       #include section
*
**********************************************************************
*/

#include "SSH_Conf.h"
#include "SSH_SCP_Conf.h"

/*********************************************************************
*
*       Defines, configurable
*
**********************************************************************
*/

#ifndef   SSH_SCP_CONFIG_MAX_SESSIONS
  #define SSH_SCP_CONFIG_MAX_SESSIONS  1
#endif

#ifndef   SSH_SCP_CONFIG_PATH_MAX
  #define SSH_SCP_CONFIG_PATH_MAX      260   /*matches emFile*/
#endif

#ifndef   SSH_SCP_INITIAL_WINDOW_SIZE
  #define SSH_SCP_INITIAL_WINDOW_SIZE  8192
#endif

#if SSH_SCP_CONFIG_MAX_SESSIONS > SSH_CONFIG_MAX_SESSIONS
  #error More SCP sessions defined than base SSH sessions to support them
#endif

#ifdef __cplusplus
}
#endif

#endif

/*************************** End of file ****************************/
