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

File    : IP_FTP_SERVER.h
Purpose : embOS/IP FTP server add-on header file.
*/

#ifndef IP_FTP_SERVER_H       // Avoid multiple inclusion.
#define IP_FTP_SERVER_H

#ifdef __ICCARM__  // IAR
  #pragma diag_suppress=Pa029  // No warning for unknown pragmas in earlier verions of EWARM
  #pragma diag_suppress=Pa137  // No warning for C-Style-Casts with C++
#endif

#include "IP_FTP_SERVER_Conf.h"
#include "SEGGER.h"
#include "IP_FS.h"

#if defined(__cplusplus)
  extern "C" {                // Make sure we have C-declarations in C++ programs.
#endif

/*********************************************************************
*
*       Defines, configurable
*
**********************************************************************
*/

#ifndef   FTPS_USE_PARA                             // Some compiler complain about unused parameters.
  #define FTPS_USE_PARA(Para)           (void)Para  // This works for most compilers.
#endif

#ifndef   FTPS_BUFFER_SIZE
  #define FTPS_BUFFER_SIZE              512  // Default size for in and out buffers. The size of the in and out buffers
#endif                                       // has direct impact on the performance. You can increase the buffer size
                                             // depending on the size of the used MTU. 1500 is max. MTU for Ethernet.
                                             // The payload of a TCP packet can be up to 1460 bytes. Your buffer should
                                             // not be larger as the possible payload as otherwise incomplete (not full)
                                             // packets might be sent out.

#ifndef   FTPS_MAX_PATH
  #define FTPS_MAX_PATH                 128  // Max. length of complete path including directory and filename.
#endif

#ifndef   FTPS_MAX_PATH_DIR
  #define FTPS_MAX_PATH_DIR             128  // Max. length of dirname (path without filename).
#endif

#ifndef   FTPS_MAX_FILE_NAME
  #define FTPS_MAX_FILE_NAME            13   // The default is 13 characters because filenames can not be longer than an 8.3 without
#endif                                       // long file name support. 8.3 + 1 character for string termination.

/*********************************************************************
*
*       Defines, fixed
*
**********************************************************************
*/

#define IP_FTPS_PERM_VISIBLE  (1u << 0)
#define IP_FTPS_PERM_READ     (1u << 1)
#define IP_FTPS_PERM_WRITE    (1u << 2)

/*********************************************************************
*
*       Types, global
*
**********************************************************************
*/

typedef void* FTPS_SOCKET;

typedef struct {
  U32 NumBytesInBuf;        // Size of Rx buffer. By default FTPS_BUFFER_SIZE .
  U32 NumBytesOutBuf;       // Size of Tx buffer. By default FTPS_BUFFER_SIZE .
  U32 NumBytesCwdNameBuf;   // Size of buffer used for the Current Working Directory. By default FTPS_MAX_PATH_DIR .
  U32 NumBytesPathNameBuf;  // Size of buffer used for paths (directory + filename). By default FTPS_MAX_PATH .
  U32 NumBytesDirNameBuf;   // Size of buffer used for dir(ectory) names (directory without filename). By default FTPS_MAX_PATH .
  U32 NumBytesFileNameBuf;  // Size of buffer used for filenames. By default FTPS_MAX_FILE_NAME .
} FTPS_BUFFER_SIZES;

typedef struct {
  int (*pfFindUser)   (const char* sUser);
  int (*pfCheckPass)  (int UserId, const char* sPass);
  int (*pfGetDirInfo) (int UserId, const char* sDirIn , char* sDirOut , int SizeOfDirOut);
  int (*pfGetFileInfo)(int UserId, const char* sFileIn, char* sFileOut, int SizeOfFileOut);
} FTPS_ACCESS_CONTROL;

typedef struct {
  int         (*pfSend)      (const unsigned char* pData, int Len, FTPS_SOCKET hSock);
  int         (*pfReceive)   (      unsigned char* pData, int Len, FTPS_SOCKET hSock);
  FTPS_SOCKET (*pfConnect)   (FTPS_SOCKET hCtrlSock, U16 Port);
  void        (*pfDisconnect)(FTPS_SOCKET hDataSock);
  FTPS_SOCKET (*pfListen)    (FTPS_SOCKET hCtrlSock, U16* pPort, U8* pIPAddr);
  int         (*pfAccept)    (FTPS_SOCKET hCtrlSock, FTPS_SOCKET* phDataSocket);
  int         (*pfSetSecure) (FTPS_SOCKET Socket, FTPS_SOCKET Clone);
} IP_FTPS_API;

typedef struct {
  void* (*pfAlloc)(U32 NumBytesReq);
  void  (*pfFree) (void* p);
} FTPS_SYS_API;

typedef struct {
  FTPS_ACCESS_CONTROL* pAccess;
  U32 (*pfGetTimeDate)(void);
} FTPS_APPLICATION;

typedef struct {
  const IP_FTPS_API*      pIP_API;
  const IP_FS_API*        pFS_API;
  const FTPS_APPLICATION* pApplication;
  const FTPS_SYS_API*     pSYS_API;
        void*             pIntContext;
        U8                RequestedSecurity;
        U8                IsImplicitMode;
} FTPS_CONTEXT;

/*********************************************************************
*
*       API functions / Function prototypes
*
**********************************************************************
*/

void IP_FTPS_ConfigBufSizes   (FTPS_BUFFER_SIZES* pBufferSizes);
U32  IP_FTPS_CountRequiredMem (FTPS_CONTEXT* pContext);
void IP_FTPS_Init             (FTPS_CONTEXT* pContext, const IP_FTPS_API* pIP_API, const IP_FS_API* pFS_API, const FTPS_APPLICATION* pApplication, const FTPS_SYS_API* pSYS_API);
int  IP_FTPS_Process          (const IP_FTPS_API* pIP_API, FTPS_SOCKET hCtrlSock, const IP_FS_API* pFS_API, const FTPS_APPLICATION* pApplication);
int  IP_FTPS_ProcessEx        (FTPS_CONTEXT* pContext    , FTPS_SOCKET hCtrlSock);
void IP_FTPS_OnConnectionLimit(const IP_FTPS_API* pIP_API, FTPS_SOCKET hCtrlSock);
void IP_FTPS_SetSignOnMsg     (const char* sSignOnMsg);
int  IP_FTPS_IsDataSecured    (const FTPS_CONTEXT* pContext);
void IP_FTPS_AllowOnlySecured (FTPS_CONTEXT* pContext, unsigned DataOnOff);
void IP_FTPS_SetImplicitMode  (FTPS_CONTEXT* pContext);


#if defined(__cplusplus)
}                             // Make sure we have C-declarations in C++ programs.
#endif

#endif                        // Avoid multiple inclusion.

/*************************** End of file ****************************/
