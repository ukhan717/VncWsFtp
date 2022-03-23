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

File    : IP_FTP_SERVER_Start.c
Purpose : Sample program for embOS & embOS/IP & FTP server
          Demonstrates use of the embOS/IP FTP server add-on.

          You can test the FTP server by connecting to port 21 . The
          sample is configured for two user accounts:
          User: Anonymous
          Pass: *

          User: Admin
          Pass: Secret
--------- END-OF-HEADER --------------------------------------------*/

#include "RTOS.h"
#include "BSP.h"
#include "IP.h"
#include "IP_FTP_SERVER.h"

/*********************************************************************
*
*       Defines, configurable
*
**********************************************************************
*/

#define USE_SSL            0                                // Use SSL to enable support for secure connections.
#define ALLOW_SECURE_ONLY  USE_SSL                          // Allow only secured connections. No fallback to insecure.
#define USE_RX_TASK        0                                // 0: Packets are read in ISR, 1: Packets are read in a task of its own.

#if USE_SSL
#include "SSL.h"

#define FTPS_STACK_SIZE  4096
#else
#define FTPS_STACK_SIZE  2304
#endif

//
// FTP server sample configuration.
//
#define MAX_CONNECTIONS   2     // Number of connections to handle at the same time
#define BACK_LOG          5     // Number of incoming connections to hold in case one connection gets freed
#define CHILD_ALLOC_SIZE  2560  // NumBytes required from memory pool for one connection. Should be fine tuned according
                                // to your configuration using IP_FTPS_CountRequiredMem() .

enum {
  USER_ID_ANONYMOUS = 1,
  USER_ID_ADMIN
};

//
// Task priorities
//
enum {
   TASK_PRIO_FTPS_CHILD = 150
  ,TASK_PRIO_FTPS_PARENT
  ,TASK_PRIO_IP_TASK           // Priority should be higher as all TCP/IP application tasks
#if USE_RX_TASK
  ,TASK_PRIO_IP_RX_TASK        // Must be the highest priority of all TCP/IP related tasks, comment out to read packets in ISR
#endif
};

//
// Task stack sizes that might not fit for some interfaces (multiples of sizeof(int)).
//
#ifndef   APP_TASK_STACK_OVERHEAD
  #define APP_TASK_STACK_OVERHEAD     0
#endif

/*********************************************************************
*
*       Types, local
*
**********************************************************************
*/

typedef struct {
  FTPS_CONTEXT* pContext;
  unsigned      PlainSocket;
#if USE_SSL
  unsigned      IsSecure;
  SSL_SESSION   Session;
#endif
} FTPS_APP_CONTEXT;

/*********************************************************************
*
*       Prototypes
*
**********************************************************************
*/

#ifdef __cplusplus
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif
void MainTask(void);
#ifdef __cplusplus
}
#endif

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

static FTPS_APP_CONTEXT _aContext[2 * MAX_CONNECTIONS];  // One client requires two connections/sessions.

#if USE_SSL
//
// SSL transport API.
//
static const SSL_TRANSPORT_API _IP_Transport = {
  send,
  recv,
  NULL  // Don't verify the time. Otherwise a function that returns the unix time is needed.
};
#endif

//
// FTP sample data.
//
static U32 _aPool[(CHILD_ALLOC_SIZE * MAX_CONNECTIONS) / sizeof(int)];  // Memory pool for the FTP server child tasks.

static IP_HOOK_ON_STATE_CHANGE _StateChangeHook;
static int                     _IFaceId;

static const IP_FS_API* _pFS_API;     // File system info
static       unsigned   _ConnectCnt;

//
// Task stacks and Task-Control-Blocks.
//
static OS_STACKPTR int _IPStack[TASK_STACK_SIZE_IP_TASK/sizeof(int)];             // Stack of the IP_Task.
static OS_TASK         _IPTCB;                                                    // Task-Control-Block of the IP_Task.

#if USE_RX_TASK
static OS_STACKPTR int _IPRxStack[TASK_STACK_SIZE_IP_RX_TASK/sizeof(int)];        // Stack of the IP_RxTask.
static OS_TASK         _IPRxTCB;                                                  // Task-Control-Block of the IP_RxTask.
#endif

static OS_STACKPTR int _aFTPStack[MAX_CONNECTIONS][FTPS_STACK_SIZE/sizeof(int)];  // Stacks of the FTP server child tasks.
static OS_TASK         _aFTPTCB[MAX_CONNECTIONS];                                 // Task-Control-Blocks of the FTP server child tasks.

/*********************************************************************
*
*       Local functions
*
**********************************************************************
*/

/*********************************************************************
*
*       _OnStateChange()
*
* Function description
*   Callback that will be notified once the state of an interface
*   changes.
*
* Parameters
*   IFaceId   : Zero-based interface index.
*   AdminState: Is this interface enabled ?
*   HWState   : Is this interface physically ready ?
*/
static void _OnStateChange(unsigned IFaceId, U8 AdminState, U8 HWState) {
  //
  // Check if this is a disconnect from the peer or a link down.
  // In this case call IP_Disconnect() to get into a known state.
  //
  if (((AdminState == IP_ADMIN_STATE_DOWN) && (HWState == 1)) ||  // Typical for dial-up connection e.g. PPP when closed from peer. Link up but app. closed.
      ((AdminState == IP_ADMIN_STATE_UP)   && (HWState == 0))) {  // Typical for any Ethernet connection e.g. PPPoE. App. opened but link down.
    IP_Disconnect(IFaceId);                                       // Disconnect the interface to a clean state.
  }
}

/*********************************************************************
*
*       _Alloc()
*
*  Function description
*    Wrapper for memory allocations. (embOS/IP: IP_MEM_AllocEx())
*
*  Parameters
*    NumBytesReq: Number of bytes to allocate from our memory pool.
*
*  Return value
*    != NULL: O.K., pointer to allocated memory block.
*    == NULL: Error.
*/
static void* _Alloc(U32 NumBytesReq) {
  return IP_AllocEx(_aPool, NumBytesReq);
}

/*********************************************************************
*
*       _AllocContext()
*
*  Function description
*    Retrieves the next free context memory block to use.
*
*  Parameters
*    hSock: Socket handle.
*
*  Return value
*    != NULL: Free context found, pointer to context.
*    == NULL: Error, no more free entries.
*/
static FTPS_APP_CONTEXT* _AllocContext(long hSock) {
  FTPS_APP_CONTEXT* pContext;
  FTPS_APP_CONTEXT* pRunner;
  unsigned          i;

  pContext = NULL;
  i        = 0u;
  OS_DI();
  pRunner = &_aContext[0];
  do {
    if ((pRunner->PlainSocket == 0)
#if USE_SSL
     && (pRunner->Session.Socket == 0)
#endif
       ) {
      pRunner->PlainSocket = hSock;  // Mark the entry to be in use.
      pContext             = pRunner;
      pContext->pContext   = NULL;
      break;
    }
    pRunner++;
    i++;
  } while (i < SEGGER_COUNTOF(_aContext));
  OS_EI();
  return pContext;
}

/*********************************************************************
*
*       _FreeContext()
*
*  Function description
*    Frees a context memory block.
*
*  Parameters
*    pContext: Connection context.
*/
static void _FreeContext(FTPS_APP_CONTEXT* pContext) {
  memset(pContext, 0, sizeof(*pContext));
}

#if USE_SSL
/*********************************************************************
*
*       _UpgradeOnDemand()
*
*  Function description
*    Upgrades a connection to a secured one.
*
*  Parameters
*    pContext: Connection context.
*
*  Return value
*    == 0: Success.
*    <  0: Error.
*/
static int _UpgradeOnDemand(FTPS_APP_CONTEXT* pContext) {
  int Status;

  Status = 0;
  if ((pContext->IsSecure != 0) && (pContext->PlainSocket != 0) && (pContext->Session.Socket == 0)) {
    SSL_SESSION_Prepare(&pContext->Session, pContext->PlainSocket, &_IP_Transport);
    Status = SSL_SESSION_Accept(&pContext->Session);
    if (Status < 0) {
      closesocket(pContext->PlainSocket);
    }
    pContext->PlainSocket = 0;
  }
  return Status;
}
#endif

/*********************************************************************
*
*       _Recv()
*/
static int _Recv(unsigned char* buf, int len, void* pConnectionInfo) {
  FTPS_APP_CONTEXT* pContext;
  int               Status;

  pContext = (FTPS_APP_CONTEXT*)pConnectionInfo;
#if USE_SSL
  _UpgradeOnDemand(pContext);
  if (pContext->PlainSocket == 0) {
    Status = SSL_SESSION_Receive(&pContext->Session, buf, len);
  }
  else
#endif
  {
    Status = recv(pContext->PlainSocket, (char*)buf, len, 0);
  }
  return Status;
}

/*********************************************************************
*
*       _Send()
*/
static int _Send(const unsigned char* buf, int len, void* pConnectionInfo) {
  FTPS_APP_CONTEXT* pContext;
  int               Status;

  pContext = (FTPS_APP_CONTEXT*)pConnectionInfo;
#if USE_SSL
  _UpgradeOnDemand(pContext);
  if (pContext->PlainSocket == 0) {
    Status = SSL_SESSION_Send(&pContext->Session, buf, len);
  }
  else
#endif
  {
    Status = send(pContext->PlainSocket, (const char*)buf, len, 0);
  }
  return Status;
}

/*********************************************************************
*
*       _Connect()
*
*  Function description
*    This function is called from the FTP server module if the client
*    uses active FTP to establish the data connection.
*/
static FTPS_SOCKET _Connect(FTPS_SOCKET CtrlSocket, U16 Port) {
  struct sockaddr_in Data;
  struct sockaddr_in PeerAddr;
  FTPS_APP_CONTEXT*  pCtrlContext;
  FTPS_APP_CONTEXT*  pDataContext;
  long               hSock;
  long               DataSock;
  int                ConnectStatus;
  int                AddrSize;
  int                Opt;

  Opt          = 1;
  pCtrlContext = (FTPS_APP_CONTEXT*)CtrlSocket;

  DataSock = socket(AF_INET, SOCK_STREAM, 0);  // Create a new socket for a data connection to the client.
  if (DataSock != SOCKET_ERROR) {              // Socket created?
    setsockopt(DataSock, SOL_SOCKET, SO_REUSEADDR, &Opt, sizeof(Opt));
    Data.sin_family      = AF_INET;
    Data.sin_port        = htons(20);
    Data.sin_addr.s_addr = INADDR_ANY;
    bind(DataSock, (struct sockaddr*)&Data, sizeof(Data));
    //
    // Allocate a context.
    //
    pDataContext = _AllocContext(DataSock);
    if (pDataContext == NULL) {
      closesocket(DataSock);
      return NULL;
    }
#if USE_SSL
    if (pCtrlContext->PlainSocket == 0) {
      hSock = pCtrlContext->Session.Socket;
      //
      // Check if data connection shall be secured too.
      //
      if (IP_FTPS_IsDataSecured(pCtrlContext->pContext) != 0) {
        pDataContext->IsSecure = 1;
      }
    }
    else
#endif
    {
      hSock = pCtrlContext->PlainSocket;
    }
    //
    //  Get IP address of connected client and connect to listening port.
    //
    AddrSize = sizeof(struct sockaddr_in);
    getpeername(hSock, (struct sockaddr*)&PeerAddr, &AddrSize);
    PeerAddr.sin_port = htons(Port);
    ConnectStatus  = connect(DataSock, (struct sockaddr*)&PeerAddr, sizeof(struct sockaddr_in));
    if (ConnectStatus == 0) {
      return (void*)pDataContext;
    } else {
      closesocket(DataSock);
      _FreeContext(pDataContext);
    }
  }
  return NULL;
}

/*********************************************************************
*
*       _Disconnect()
*
*  Function description
*    This function is called from the FTP server module to close the
*    data connection.
*/
static void _Disconnect(FTPS_SOCKET hSock) {
  FTPS_APP_CONTEXT* pContext;
#if USE_SSL
  U32               Timeout;
  char              c;
#endif

  if (hSock != NULL) {
    pContext = (FTPS_APP_CONTEXT*)hSock;
#if USE_SSL
    if (pContext->IsSecure != 0) {
      //
      // Some FTP clients like Filezilla expect a secure connection
      // to be established, whether payload gets sent or not.
      // Upgrading the connection is typically done when calling
      // send/recv for the first time on this connection. If this
      // is not done (no data) we need to do it here, right before
      // disconnecting/downgrading it again.
      //
      _UpgradeOnDemand(pContext);
      SSL_SESSION_Disconnect(&pContext->Session);
      //
      // FTP with SSL/TLS is a strange thing. The FTP protocol
      // expects the server to close the connection once all
      // data has been sent. When SSL/TLS is in use this means
      // the server closes/disconnects SSL/TLS. Once this is done
      // roles switch and the client seems to expect to close the
      // TCP connection first!
      // As a workaround we call recv() with timeout which will
      // either return 0 if the connection gets closed or error
      // on timeout (just used to avoid infinite wait).
      //
      Timeout = 1000;
      setsockopt(pContext->Session.Socket, SOL_SOCKET, SO_RCVTIMEO, &Timeout, sizeof(Timeout));
      recv(pContext->Session.Socket, &c, 1, 0);
      closesocket(pContext->Session.Socket);
    }
    else
#endif
    {
      closesocket(pContext->PlainSocket);
    }
    _FreeContext(pContext);
  }
}

/*********************************************************************
*
*       _Listen()
*
*  Function description
*    This function is called from the FTP server module if the client
*    uses passive FTP. It creates a socket and searches for a free port
*    which can be used for the data connection.
*
*  Return value
*    > 0   Socket descriptor
*    NULL  Error
*/
static FTPS_SOCKET _Listen(FTPS_SOCKET CtrlSocket, U16* pPort, U8* pIPAddr) {
  struct sockaddr_in Addr;
  FTPS_APP_CONTEXT*  pCtrlContext;
  FTPS_APP_CONTEXT*  pDataContext;
  U32  IPAddr;
  long DataSock;
  long hSock;
  int  AddrSize;
  U16  Port;

  pCtrlContext = (FTPS_APP_CONTEXT*)CtrlSocket;

  Addr.sin_family = AF_INET;
  Addr.sin_port   = 0;                         // Let Stack find a free port.
  Addr.sin_addr.s_addr = INADDR_ANY;
  DataSock = socket(AF_INET, SOCK_STREAM, 0);  // Create a new socket for data connection to the client
  if(DataSock == SOCKET_ERROR) {               // Socket created ?
    return NULL;
  }
  bind(DataSock, (struct sockaddr*)&Addr, sizeof(Addr));
  listen(DataSock, 1);
  //
  // Allocate a FTP context.
  //
  pDataContext = _AllocContext(DataSock);
  if (pDataContext == NULL) {
    closesocket(DataSock);
    return NULL;
  }
#if USE_SSL
  if (pCtrlContext->PlainSocket == 0) {
    hSock                 = pCtrlContext->Session.Socket;
    //
    // Check if data connection shall be secured too.
    //
    if (IP_FTPS_IsDataSecured(pCtrlContext->pContext) != 0) {
      pDataContext->IsSecure = 1;
    }
  }
  else
#endif
  {
    hSock                 = pCtrlContext->PlainSocket;
  }
  //
  //  Get port number stack has assigned
  //
  AddrSize = sizeof(struct sockaddr_in);
  getsockname((long)DataSock, (struct sockaddr*)&Addr, &AddrSize);
  Port   = htons(Addr.sin_port);
  *pPort = Port;
  getsockname(hSock, (struct sockaddr*)&Addr, &AddrSize);
  IPAddr = ntohl(Addr.sin_addr.s_addr);  // Load to host endianess.
  SEGGER_WrU32BE(pIPAddr, IPAddr);       // Save from host endianess to network endiness.
  return (FTPS_SOCKET)pDataContext;
}

/*********************************************************************
*
*       _Accept()
*
*  Function description
*    This function is called from the FTP server module if the client
*    uses passive FTP. It sets the command socket to non-blocking before
*    accept() will be called. This guarantees that the FTP server always
*    returns even if the connection to the client gets lost while
*    accept() waits for a connection. The timeout is set to 10 seconds.
*
*  Return value
*    0    O.K.
*   -1    Error
*/
static int _Accept(FTPS_SOCKET CtrlSocket, FTPS_SOCKET* pSock) {
  FTPS_APP_CONTEXT* pDataContext;
  long   hSock;
  long   DataSock;
  int    SoError;
  int    t0;
  int    t;
  struct sockaddr Addr;
  int    AddrSize;
  int    Opt;

  (void)CtrlSocket;

  AddrSize     = sizeof(Addr);
  pDataContext = *(FTPS_APP_CONTEXT**)pSock;
#if USE_SSL
  if (pDataContext->PlainSocket == 0) {
    hSock = pDataContext->Session.Socket;
  }
  else
#endif
  {
    hSock = pDataContext->PlainSocket;
  }
  //
  // Set command socket non-blocking
  //
  Opt = 1;
  setsockopt(hSock, SOL_SOCKET, SO_NONBLOCK, &Opt, sizeof(Opt));
  t0 = IP_OS_GetTime32();
  do {
    DataSock = accept(hSock, &Addr, &AddrSize);
    if ((DataSock != SOCKET_ERROR) && (DataSock != 0)) {
      //
      // Set data socket blocking.
      //
      Opt = 0;
      setsockopt(DataSock, SOL_SOCKET, SO_NONBLOCK, &Opt, sizeof(Opt));
      //
      // Set data socket blocking. The data socket inherits the blocking
      // mode from the socket that was used as parameter for accept().
      // Therefore, we have to set it blocking after creation.
      // SO_KEEPALIVE is required to quarantee that the socket will be
      // closed even if the client has lost the connection to server
      // before he closed the connection.
      //
      Opt=1;
      setsockopt(DataSock, SOL_SOCKET, SO_KEEPALIVE, &Opt, sizeof(Opt));
#if USE_SSL
      if (pDataContext->PlainSocket == 0) {
        pDataContext->Session.Socket = DataSock;
      }
      else
#endif
      {
        pDataContext->PlainSocket    = DataSock;
      }
      closesocket(hSock);
      return 0;                  // Successfully connected
    }
    getsockopt(hSock, SOL_SOCKET, SO_ERROR, &SoError, sizeof(SoError));
    if (SoError != IP_ERR_WOULD_BLOCK) {
      closesocket(hSock);
      _FreeContext(pDataContext);
      return SOCKET_ERROR;       // Not in progress and not successful, error...
    }
    t = IP_OS_GetTime32() - t0;
    if (t >= 10000) {
      closesocket(hSock);
      _FreeContext(pDataContext);
      return SOCKET_ERROR;
    }
    OS_Delay(1);                 // Give lower prior tasks some time
  } while (1);
}

#if USE_SSL
/*********************************************************************
*
*       _SetSecure()
*
*  Function description
*    Update the connection to a secured one.
*/
static int _SetSecure(FTPS_SOCKET hSock, FTPS_SOCKET hSockClone) {
  FTPS_APP_CONTEXT* pSock;
  FTPS_APP_CONTEXT* pSockClone;

  pSock                = (FTPS_APP_CONTEXT*)hSock;
  pSockClone           = (FTPS_APP_CONTEXT*)hSockClone;
  pSock->IsSecure      = 1;
  pSockClone->IsSecure = 1;
  return 0;
}
#endif

/*********************************************************************
*
*       IP_FTPS_API
*
*  Description
*    IP stack function table
*/
static const IP_FTPS_API _IP_API = {
  _Send,
  _Recv,
  _Connect,
  _Disconnect,
  _Listen,
  _Accept,
#if USE_SSL
  _SetSecure
#else
  NULL
#endif
};

/*********************************************************************
*
*       FTPS_SYS_API
*
*  Description
*    Memory handling function table
*/
static const FTPS_SYS_API _Sys_API = {
  _Alloc,
  IP_Free
};

/**************************************************************************************************************************************************************
*
*       User management.
*/

/*********************************************************************
*
*       _FindUser()
*
*  Function description
*    Callback function for user management.
*    Checks if user name is valid.
*
*  Return value
*    0    UserID invalid or unknown
*  > 0    UserID, no password required
*  < 0    - UserID, password required
*/
static int _FindUser (const char * sUser) {
  if (strcmp(sUser, "Admin") == 0) {
    return (0 - USER_ID_ADMIN);
  }
  if (strcmp(sUser, "anonymous") == 0) {
    return USER_ID_ANONYMOUS;
  }
  return 0;
}

/*********************************************************************
*
*       _CheckPass()
*
*  Function description
*    Callback function for user management.
*    Checks user password.
*
*  Return value
*    0    UserID known, password valid
*    1    UserID unknown or password invalid
*/
static int _CheckPass (int UserId, const char * sPass) {
  if ((UserId == USER_ID_ADMIN) && (strcmp(sPass, "Secret") == 0)) {
    return 0;
  } else {
    return 1;
  }
}

/*********************************************************************
*
*       _GetDirInfo()
*
*  Function description
*    Callback function for permission management.
*    Checks directory permissions.
*
*  Return value
*    Returns a combination of the following:
*    IP_FTPS_PERM_VISIBLE    - Directory is visible as a directory entry
*    IP_FTPS_PERM_READ       - Directory can be read/entered
*    IP_FTPS_PERM_WRITE      - Directory can be written to
*
*  Parameters
*    UserId        - User ID returned by _FindUser()
*    sDirIn        - Full directory path and with trailing slash
*    sDirOut       - Reserved for future use
*    DirOutSize    - Reserved for future use
*
*  Notes
*    In this sample configuration anonymous user is allowed to do anything.
*    Samples for folder permissions show how to set permissions for different
*    folders and users. The sample configures permissions for the following
*    directories:
*      - /READONLY/: This directory is read only and can not be written to.
*      - /VISIBLE/ : This directory is visible from the folder it is located
*                    in but can not be accessed.
*      - /ADMIN/   : This directory can only be accessed by the user "Admin".
*/
static int _GetDirInfo(int UserId, const char * sDirIn, char * sDirOut, int DirOutSize) {
  int Perm;

  (void)sDirOut;
  (void)DirOutSize;

  //
  // Generic permissions.
  //  Anonymous : IP_FTPS_PERM_VISIBLE |
  //              IP_FTPS_PERM_READ
  //  Valid user: IP_FTPS_PERM_VISIBLE |
  //              IP_FTPS_PERM_READ    |
  //              IP_FTPS_PERM_WRITE
  //
  if (UserId == USER_ID_ANONYMOUS) {
    Perm = IP_FTPS_PERM_VISIBLE | IP_FTPS_PERM_READ;
  } else {
    Perm = IP_FTPS_PERM_VISIBLE | IP_FTPS_PERM_READ | IP_FTPS_PERM_WRITE;
  }

  if (strcmp(sDirIn, "/READONLY/") == 0) {
    Perm = IP_FTPS_PERM_VISIBLE | IP_FTPS_PERM_READ;
  }
  if (strcmp(sDirIn, "/VISIBLE/") == 0) {
    Perm = IP_FTPS_PERM_VISIBLE;
  }
  if (strcmp(sDirIn, "/ADMIN/") == 0) {
    if (UserId != USER_ID_ADMIN) {
      return 0;  // Only Admin is allowed for this directory
    }
  }
  return Perm;
}

/*********************************************************************
*
*       _GetFileInfo()
*
*  Function description
*    Callback function for permission management.
*    Checks file permissions.
*
*  Return value
*    Returns a combination of the following:
*    IP_FTPS_PERM_VISIBLE    - File is visible as a file entry
*    IP_FTPS_PERM_READ       - File can be read
*    IP_FTPS_PERM_WRITE      - File can be written to
*
*  Parameters
*    UserId        - User ID returned by _FindUser()
*    sFileIn       - Full path to the file
*    sFileOut      - Reserved for future use
*    FileOutSize   - Reserved for future use
*
*  Notes
*    In this sample configuration all file accesses are allowed. File
*    permissions are checked against directory permissions. Therefore it
*    is not necessary to limit permissions on files that reside in a
*    directory that already limits access.
*    Setting permissions works the same as for _GetDirInfo() .
*/
static int _GetFileInfo(int UserId, const char * sFileIn, char * sFileOut, int FileOutSize) {
  int Perm;

  (void)UserId;
  (void)sFileIn;
  (void)sFileOut;
  (void)FileOutSize;

  Perm = IP_FTPS_PERM_VISIBLE | IP_FTPS_PERM_READ | IP_FTPS_PERM_WRITE;

  return Perm;
}

/*********************************************************************
*
*       FTPS_ACCESS_CONTROL
*
*  Description
*   Access control function table
*/
static FTPS_ACCESS_CONTROL _Access_Control = {
  _FindUser,
  _CheckPass,
  _GetDirInfo,
  _GetFileInfo  // Optional, only required if permissions for individual files shall be used
};

/*********************************************************************
*
*       _GetTimeDate()
*
*  Description:
*    Current time and date in a format suitable for the FTP server.
*
*    Bit 0-4:   2-second count (0-29)
*    Bit 5-10:  Minutes (0-59)
*    Bit 11-15: Hours (0-23)
*    Bit 16-20: Day of month (1-31)
*    Bit 21-24: Month of year (1-12)
*    Bit 25-31: Count of years from 1980 (0-127)
*
*  Note:
*    FTP server requires a real time clock for to transmit the
*    correct timestamp of files. Lists transmits either the
*    year or the HH:MM. For example:
*    -rw-r--r--   1 root 1000 Jan  1  1980 DEFAULT.TXT
*    or
*    -rw-r--r--   1 root 1000 Jul 29 11:40 PAKET01.TXT
*    The decision which of both infos the server transmits
*    depends on the system time. If the year of the system time
*    is identical to the year stored in the timestamp of the file,
*    the time will be transmitted, if not the year.
*/
static U32 _GetTimeDate(void) {
  U32 r;
  U16 Sec, Min, Hour;
  U16 Day, Month, Year;

  Sec   = 0;        // 0 based.  Valid range: 0..59
  Min   = 0;        // 0 based.  Valid range: 0..59
  Hour  = 0;        // 0 based.  Valid range: 0..23
  Day   = 1;        // 1 based.    Means that 1 is 1. Valid range is 1..31 (depending on month)
  Month = 1;        // 1 based.    Means that January is 1. Valid range is 1..12.
  Year  = 0;        // 1980 based. Means that 2008 would be 28.
  r   = Sec / 2 + (Min << 5) + (Hour  << 11);
  r  |= (U32)(Day + (Month << 5) + (Year  << 9)) << 16;
  return r;
}

/*********************************************************************
*
*       FTPS_APPLICATION
*
*  Description
*   Application data table, defines all application specifics used by the FTP server
*/
static const FTPS_APPLICATION _Application = {
  &_Access_Control,
  _GetTimeDate
};

/*********************************************************************
*
*       _AddToConnectCnt()
*
*/
static void _AddToConnectCnt(int Delta) {
  OS_EnterRegion();
  _ConnectCnt += Delta;
  OS_LeaveRegion();
}

/*********************************************************************
*
*       _FTPServerChildTask()
*/
static void _FTPServerChildTask(void* pContext) {
  FTPS_APP_CONTEXT* pAppContext;
  FTPS_CONTEXT      FTPSContext;
  long              hSock;
  int               Opt;

  _pFS_API    = &IP_FS_emFile;
  pAppContext = (FTPS_APP_CONTEXT*)pContext;
  hSock       = pAppContext->PlainSocket;
  Opt         = 1;
  //
  pAppContext->pContext = (void*)&FTPSContext;
  setsockopt(hSock, SOL_SOCKET, SO_KEEPALIVE, &Opt, sizeof(Opt));
  IP_FTPS_Init(&FTPSContext, &_IP_API, _pFS_API, &_Application, &_Sys_API);
#if ALLOW_SECURE_ONLY
  //
  // Requires the user to use only secured connection (for data too).
  //
  IP_FTPS_AllowOnlySecured(&FTPSContext, 1);
#endif
  //
  IP_FTPS_ProcessEx(&FTPSContext, pContext);
  closesocket(hSock);
  _FreeContext(pAppContext);
  _AddToConnectCnt(-1);
  OS_Terminate(0);
}

/*********************************************************************
*
*       _FTPServerParentTask()
*/
static void _FTPServerParentTask(void) {
  FTPS_APP_CONTEXT*  pContext;
  struct sockaddr_in Addr;
  struct sockaddr_in InAddr;
  U32  NumBytes;
  long hSockListen;
  long hSock;
  int  i;
  FTPS_BUFFER_SIZES BufferSizes;

  //
  // Configure buffer size.
  //
  memset(&BufferSizes, 0, sizeof(BufferSizes));
  BufferSizes.NumBytesInBuf       = FTPS_BUFFER_SIZE;
  BufferSizes.NumBytesOutBuf      = IP_TCP_GetMTU(_IFaceId) - 72;  // Use max. MTU configured for the last interface added minus worst case IPv4/TCP/VLAN headers.
                                                                   // Calculation for the memory pool is done under assumption of the best case headers with - 40 bytes.;
  BufferSizes.NumBytesCwdNameBuf  = FTPS_MAX_PATH_DIR;
  BufferSizes.NumBytesPathNameBuf = FTPS_MAX_PATH;
  BufferSizes.NumBytesDirNameBuf  = FTPS_MAX_PATH;
  BufferSizes.NumBytesFileNameBuf = FTPS_MAX_FILE_NAME;
  //
  // Configure the size of the buffers used by the FTP server child tasks.
  //
  IP_FTPS_ConfigBufSizes(&BufferSizes);
  //
  // Check memory pool size.
  //
  NumBytes = IP_FTPS_CountRequiredMem(NULL);     // Get NumBytes for internals of one child thread.
  NumBytes = (NumBytes + 64) * MAX_CONNECTIONS;  // Calc. the total amount for x connections (+ some bytes for managing a memory pool).
  IP_Logf_Application("FTPS: Using a memory pool of %lu bytes for %lu connections.", sizeof(_aPool), MAX_CONNECTIONS);
  if (NumBytes > sizeof(_aPool)) {
    IP_Warnf_Application("FTPS: Memory pool should be at least %lu bytes.", NumBytes);
  }
  //
  // Give the stack some more memory to enable the dynamical memory allocation for the FTP server child tasks
  //
  IP_AddMemory(_aPool, sizeof(_aPool));
  //
  // Get a socket into listening state
  //
  hSockListen = socket(AF_INET, SOCK_STREAM, 0);
  if (hSockListen == SOCKET_ERROR) {
    while(1); // This should never happen!
  }
  memset(&InAddr, 0, sizeof(InAddr));
  InAddr.sin_family      = AF_INET;
  InAddr.sin_port        = htons(21);
  InAddr.sin_addr.s_addr = INADDR_ANY;
  bind(hSockListen, (struct sockaddr *)&InAddr, sizeof(InAddr));
  listen(hSockListen, BACK_LOG);
  //
  // Loop once per client and create a thread for the actual server
  //
  while (1) {
    //
    // Wait for an incoming connection
    //
    int AddrLen = sizeof(Addr);
    if ((hSock = accept(hSockListen, (struct sockaddr*)&Addr, &AddrLen)) == SOCKET_ERROR) {
      continue;    // Error
    }
    if (_ConnectCnt < MAX_CONNECTIONS) {
      for (i = 0; i < MAX_CONNECTIONS; i++) {
        U8 r;

        r = OS_IsTask(&_aFTPTCB[i]);
        if (r == 0) {
          //
          // Alloc a FTPS socket.
          //
          pContext = _AllocContext(hSock);
          if (pContext == NULL) {
            break;
          }
          _AddToConnectCnt(1);
          OS_CREATETASK_EX(&_aFTPTCB[i], "FTPServer_Child", _FTPServerChildTask, TASK_PRIO_FTPS_CHILD, _aFTPStack[i], (void*)pContext);
          break;
        }
      }
    } else {
      IP_FTPS_OnConnectionLimit(&_IP_API, (void*)hSock);
      OS_Delay(2000);          // Give connection some time to complete
      closesocket(hSock);
    }
  }
}

/*********************************************************************
*
*       Global functions
*
**********************************************************************
*/

/*********************************************************************
*
*       MainTask()
*
* Function description
*   Main task executed by the RTOS to create further resources and
*   running the main application.
*/
void MainTask(void) {
  IP_Init();
#if USE_SSL
  SSL_Init();
#endif
  _IFaceId = IP_INFO_GetNumInterfaces() - 1;                                           // Get the last registered interface ID as this is most likely the interface we want to use in this sample.
  OS_SetPriority(OS_GetTaskID(), TASK_PRIO_IP_TASK);                                   // For now, this task has highest prio except IP management tasks.
  OS_CREATETASK(&_IPTCB  , "IP_Task"  , IP_Task  , TASK_PRIO_IP_TASK   , _IPStack);    // Start the IP_Task.
#if USE_RX_TASK
  OS_CREATETASK(&_IPRxTCB, "IP_RxTask", IP_RxTask, TASK_PRIO_IP_RX_TASK, _IPRxStack);  // Start the IP_RxTask, optional.
#endif
  IP_AddStateChangeHook(&_StateChangeHook, _OnStateChange);                            // Register hook to be notified on disconnects.
  IP_Connect(_IFaceId);                                                                // Connect the interface if necessary.
  OS_SetPriority(OS_GetTaskID(), TASK_PRIO_FTPS_PARENT);                               // Use this task as FTP server parent task.
  OS_SetTaskName(OS_GetTaskID(), "FTPServer_Parent");                                  // Set task name.
  _FTPServerParentTask();                                                              // Start the FTP server.
}

/****** End Of File *************************************************/
