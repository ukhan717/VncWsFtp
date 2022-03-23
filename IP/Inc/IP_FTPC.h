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
File        : IP_FTPC.h
Purpose     : Publics for the FTP client
---------------------------END-OF-HEADER------------------------------

Attention : Do not modify this file !
*/

#ifndef  IP_FTPC_H
#define  IP_FTPC_H

#ifdef __ICCARM__  // IAR
  #pragma diag_suppress=Pa029  // No warning for unknown pragmas in earlier verions of EWARM
  #pragma diag_suppress=Pa137  // No warning for C-Style-Casts with C++
#endif

#include "FTPC_Conf.h"
#include "SEGGER.h"
#include "IP_FS.h"

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

/*********************************************************************
*
*       defines
*
**********************************************************************
*/
//
// Connection modes
//
#define FTPC_MODE_ACTIVE                  (0u << 0)
#define FTPC_MODE_PASSIVE                 (1u << 0)
//
#define FTPC_MODE_PLAIN_FTP               (0u << 1)
#define FTPC_MODE_EXPLICIT_TLS_REQUIRED   (1u << 1)
#define FTPC_MODE_IMPLICIT_TLS_REQUIRED   (2u << 1)

//
// FTP commands
//
#define FTPC_CMD_LIST       1
#define FTPC_CMD_CWD        2
#define FTPC_CMD_CDUP       3
#define FTPC_CMD_STOR       4
#define FTPC_CMD_RETR       5
#define FTPC_CMD_USER       6
#define FTPC_CMD_PASS       7
#define FTPC_CMD_SYST       8
#define FTPC_CMD_PWD        9
#define FTPC_CMD_TYPE      10
#define FTPC_CMD_MKD       11
#define FTPC_CMD_RMD       12
#define FTPC_CMD_DELE      13
#define FTPC_CMD_PROT      14
#define FTPC_CMD_PBSZ      15
#define FTPC_CMD_FEAT      16
#define FTPC_CMD_QUIT      17

// These commands are internal and cannot be sent using IP_FTPC_ExecCmd().
#define FTPC_CMD_NULL       0
#define FTPC_CMD_PASV      18
#define FTPC_CMD_AUTH      19


#define FTPC_ERROR         -1

/*********************************************************************
*
*       Types
*
**********************************************************************
*/

typedef void *               FTPC_SOCKET;
typedef struct FTPC_CONTEXT  IP_FTPC_CONTEXT;

typedef struct {
  FTPC_SOCKET  (*pfConnect)    (const char * SrvAddr, unsigned SrvPort);
  void         (*pfDisconnect) (FTPC_SOCKET Socket);
  int          (*pfSend)       (const char * pData, int Len, FTPC_SOCKET Socket);
  int          (*pfReceive)    (char * pData, int Len, FTPC_SOCKET Socket);
  int          (*pfSetSecure)  (FTPC_SOCKET Socket, FTPC_SOCKET Clone);
} IP_FTPC_API;

typedef struct {
  void (*pfReply)(IP_FTPC_CONTEXT *pContext, unsigned Cmd, const char *sResponse, unsigned ResponseLength, unsigned IsLineComplete);
} IP_FTPC_APPLICATION;

typedef struct {
  const IP_FTPC_API * pIP_API;
  void * pSock;
  U8 * pBuffer;                  // Pointer to the data buffer
  int Size;                      // Size of buffer
  int Cnt;                       // Number of bytes in buffer
  int RdOff;
} IN_BUFFER_CONTEXT;

typedef struct {
  const IP_FTPC_API * pIP_API;
  void * pSock;
  U8 * pBuffer;                  // Pointer to the data buffer
  int Size;                      // Size of buffer
  int Cnt;                       // Number of bytes in buffer
} OUT_BUFFER_CONTEXT;

typedef struct {
  const char * sServer;
  const char * sUser;   // User name used for the authentication
  const char * sPass;   // Password used for the authentication
  unsigned PortCmd;     // Port of the command
  unsigned PortData;    // Port of the command
  unsigned Mode;        // Data transfer process type, active: 0, passive: 1
} FTPC_CONN_DATA;

struct FTPC_CONTEXT {
  const IP_FTPC_API *         pIP_API;
  const IP_FS_API *           pFS_API;       // File system info
  const IP_FTPC_APPLICATION * pApplication;
  IN_BUFFER_CONTEXT           CtrlIn;
  IN_BUFFER_CONTEXT           DataIn;
  OUT_BUFFER_CONTEXT          CtrlOut;
  OUT_BUFFER_CONTEXT          DataOut;
  FTPC_CONN_DATA              ConnData;
};


/*********************************************************************
*
*       Functions
*
**********************************************************************
*/

int IP_FTPC_Init      (IP_FTPC_CONTEXT * pContext, const IP_FTPC_API * pIP_API, const IP_FS_API * pFS_API, U8 * pCtrlBuffer, unsigned NumBytesCtrl, U8 * pDataInBuffer, unsigned NumBytesDataIn, U8 * pDataOutBuffer, unsigned NumBytesDataOut);
int IP_FTPC_InitEx    (IP_FTPC_CONTEXT * pContext, const IP_FTPC_API * pIP_API, const IP_FS_API * pFS_API, U8 * pCtrlBuffer, unsigned NumBytesCtrl, U8 * pDataInBuffer, unsigned NumBytesDataIn, U8 * pDataOutBuffer, unsigned NumBytesDataOut, const IP_FTPC_APPLICATION * pApplication);
int IP_FTPC_Connect   (IP_FTPC_CONTEXT * pContext, const char * sServer, const char * sUser, const char * sPass, unsigned PortCmd, unsigned Mode);
int IP_FTPC_Disconnect(IP_FTPC_CONTEXT * pContext);
int IP_FTPC_ExecCmd   (IP_FTPC_CONTEXT * pContext, unsigned Cmd, const char * sPara);

#if defined(__cplusplus)
  }
#endif


#endif   /* Avoid multiple inclusion */

/*************************** End of file ****************************/




