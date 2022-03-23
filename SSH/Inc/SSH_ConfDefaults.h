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

File        : SSH_ConfDefaults.h
Purpose     : Defines defaults for most configurable defines used in emSSH.
              If you want to change a value, please do so in SSH_Conf.h,
              do NOT modify this file.

*/

#ifndef SSH_CONFDEFAULTS_H
#define SSH_CONFDEFAULTS_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************
*
*       #include Section
*
**********************************************************************
*/

#include "SSH_Conf.h"
#include "SEGGER_UTIL.h"

/*********************************************************************
*
*       Defines, configurable
*
**********************************************************************
*/

//
// Define SSH_DEBUG: Debug level for SSH product
//                  0: No checks                      (Smallest and fastest code)
//                  1: Warnings & Panic checks
//                  2: Warnings, logs, & panic checks (Seriously bigger code)
//
#ifndef SSH_DEBUG
  #define SSH_DEBUG                       2
#endif

//
// Log buffer related configuration defaults.
//
#ifndef   SSH_LOG_BUFFER_SIZE
  #define SSH_LOG_BUFFER_SIZE             160
#endif

#ifndef   SSH_OS_DISABLE_INTERRUPT
  #define SSH_OS_DISABLE_INTERRUPT()      SSH_OS_DisableInterrupt()
#endif

#ifndef   SSH_OS_ENABLE_INTERRUPT
  #define SSH_OS_ENABLE_INTERRUPT()       SSH_OS_EnableInterrupt()
#endif

#ifndef   SSH_OS_GET_TIME
  #define SSH_OS_GET_TIME()               SSH_OS_GetTime32()
#endif

#ifndef   SSH_OS_UNLOCK
  #define SSH_OS_UNLOCK()                 SSH_OS_Unlock()
#endif

#ifndef   SSH_OS_LOCK
  #define SSH_OS_LOCK()                   SSH_OS_Lock()
#endif

#ifndef   SSH_MEMCPY
  #define SSH_MEMCPY                      memcpy
#endif

#ifndef   SSH_MEMSET
  #define SSH_MEMSET                      memset
#endif

#ifndef   SSH_MEMMOVE
  #define SSH_MEMMOVE                     memmove
#endif

#ifndef   SSH_MEMCMP
  #define SSH_MEMCMP                      memcmp
#endif

#ifndef   SSH_STRCPY
  #define SSH_STRCPY                      strcpy
#endif

#ifndef   SSH_STRCMP
  #define SSH_STRCMP                      strcmp
#endif

#ifndef   SSH_STRNCMP
  #define SSH_STRNCMP                     strncmp
#endif

#ifndef   SSH_STRCAT
  #define SSH_STRCAT                      strcat
#endif

#ifndef   SSH_STRLEN
  #define SSH_STRLEN                      strlen
#endif

#ifndef   SSH_SNPRINTF
  #define SSH_SNPRINTF                    snprintf
#endif

#ifndef   SSH_RDU32BE
#define   SSH_RDU32BE                     SEGGER_RdU32BE
#endif

#ifndef   SSH_WRU32BE
#define   SSH_WRU32BE                     SEGGER_WrU32BE
#endif

#ifndef   SSH_USE_PARA                                // Some compiler complain about unused parameters.
  #define SSH_USE_PARA(Para)              (void)Para  // This works for most compilers.
#endif

#if SSH_DEBUG >= 2
  #define SSH_LOG(s)                      SSH_Logf s
  #define SSH_LOG_ERR(ERROR)              (SSH_LOG((SSH_LOG_ERROR, "SSH: %s at " __FILE__ ":%d", SSH_ERROR_GetText(ERROR), __LINE__)), ERROR)
#else
  #define SSH_LOG(s)
  #define SSH_LOG_ERR(ERROR)              (ERROR)
#endif

#if SSH_DEBUG >= 1
  #define SSH_WARN(s)                     SSH_Warnf s
#else
  #define SSH_WARN(s)
#endif

#if SSH_DEBUG >= 1
  #define SSH_WARN_IF(C, s)               do { if ((C)) { SSH_Warnf s; } } while (0)
#else
  #define SSH_WARN_IF(C, s)
#endif

#if SSH_DEBUG > 0
  #define SSH_ASSERT(X)                   /*lint -e{717} */ do { if (!(X)) { SSH_PANIC(SSH_ERROR_ASSERT_FAILED); } } while (0)
  #define SSH_PANIC(s)                    SSH_Panic(s)
#else
  #define SSH_ASSERT(X)
  #define SSH_PANIC(s)
#endif

#ifndef   SSH_CONFIG_MAX_SESSIONS
  #define SSH_CONFIG_MAX_SESSIONS         2
#endif

#ifndef   SSH_CONFIG_MAX_CHANNELS
  #define SSH_CONFIG_MAX_CHANNELS         SSH_CONFIG_MAX_SESSIONS
#endif

#ifdef __cplusplus
}
#endif

#endif

/*************************** End of file ****************************/
