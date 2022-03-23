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

File    : IP_FTP_SERVER_Conf.h
Purpose : embOS/IP FTP server add-on configuration file.
*/

#ifndef IP_FTP_SERVER_CONF_H  // Avoid multiple inclusion.
#define IP_FTP_SERVER_CONF_H

#if defined(__cplusplus)
  extern "C" {                // Make sure we have C-declarations in C++ programs.
#endif

/*********************************************************************
*
*       Defines, configurable
*
**********************************************************************
*/

#ifndef   DEBUG
  #define DEBUG  0
#endif

//
// Logging
//
#if (DEBUG || (defined(WIN32) && DEBUG))
  #if   defined(__ICCARM__)  // IAR ARM toolchain
    #include "IP.h"
    #define FTPS_WARN(p)  IP_Warnf_Application p
    #define FTPS_LOG(p)   IP_Logf_Application  p
  #elif defined(__ICCRX__)   // IAR RX toolchain
    #include "IP.h"
    #define FTPS_WARN(p)  IP_Warnf_Application p
    #define FTPS_LOG(p)   IP_Logf_Application  p
  #elif defined(__GNUC__)    // GCC toolchain
    #include "IP.h"
    #define FTPS_WARN(p)  IP_Warnf_Application p
    #define FTPS_LOG(p)   IP_Logf_Application  p
  #elif defined(WIN32)       // Microsoft Visual Studio
    void WIN32_OutputDebugStringf(const char * sFormat, ...);
    #define FTPS_WARN(p)  WIN32_OutputDebugStringf p
    #define FTPS_LOG(p)   WIN32_OutputDebugStringf p
  #else                      // Other toolchains
    #define FTPS_WARN(p)
    #define FTPS_LOG(p)
  #endif
#else                        // Release builds
  #define   FTPS_WARN(p)
  #define   FTPS_LOG(p)
#endif

//
// Old configuration defines for legacy IP_FTPS_Process() API.
// New integrations should use IP_FTPS_ProcessEx() with the
// dynamic buffer configuration via IP_FTPS_ConfigBufSizes() .
//
#define FTPS_BUFFER_SIZE       512  // Two buffers of this size will be used, one for IN and one for OUT.
#define FTPS_MAX_PATH          128  // Max. length of complete path including directory and filename. One buffer.
#define FTPS_MAX_PATH_DIR      128  // Max. length of dirname (path without filename). One buffer.
#define FTPS_MAX_FILE_NAME      13  // The default is 13 characters, because filenames can not be longer than an 8.3 without long file name support.
                                    // 8.3 + 1 character for string termination. One buffer.
//
// Sign on message. Can also be set with new API IP_FTPS_SetSignOnMsg() .
//
#define FTPS_SIGN_ON_MSG  "Welcome to embOS/IP FTP server"


#if defined(__cplusplus)
}                             // Make sure we have C-declarations in C++ programs.
#endif

#endif                        // Avoid multiple inclusion.

/*************************** End of file ****************************/
