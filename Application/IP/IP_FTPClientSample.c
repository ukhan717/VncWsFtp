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
File    : IP_FTPClientSample_Secure.c
Purpose : Sample program for embOS & TCP/IP
          Demonstrates use of the FTP client.
--------- END-OF-HEADER --------------------------------------------*/

#include "RTOS.h"
#include "FS.h"
#include "BSP.h"
#include "IP.h"
#include "IP_FTPC.h"
#include "SEGGER.h"

/*********************************************************************
*
*       Local defines, configurable
*
**********************************************************************
*/

#define USE_SSL           0                             // Use SSL to enable support for secure connections.
#define USE_RX_TASK       0                             // 0: Packets are read in ISR, 1: Packets are read in a task of its own.

#if USE_SSL
#include "SSL.h"

#define FTPC_STACK_SIZE  6144
#else
#define FTPC_STACK_SIZE  4096 // 2560
#endif

#define FTP_HOST  "192.168.11.125"
//
// Choose between FTPS implicit mode server using port 990
// or FTP explicit mode and plain mode server using port 21.
//
#define FTP_PORT  21     // FTP plain / FTP explicit.
//#define FTP_PORT  990    // FTP implicit

#define FTP_USER  "Admin"
#define FTP_PASS  "Secret"

//
// Task priorities
//
enum {
   TASK_PRIO_FTPC   = 150
  ,TASK_PRIO_IP_TASK           // Priority should be higher as all TCP/IP application tasks
#if USE_RX_TASK
  ,TASK_PRIO_IP_RX_TASK        // Must be the highest priority of all TCP/IP related tasks, comment out to read packets in ISR
#endif
};

/*********************************************************************
*
*       Types, local
*
**********************************************************************
*/

typedef struct {
  unsigned                   PlainSocket;
#if USE_SSL
  unsigned                   IsSecure;
  unsigned                   MustResume;
  SSL_SESSION                Session;
  SSL_SESSION_RESUME_PARAS   ResumeParas;
  const char*                sName;
#endif
} FTPS_APP_CONTEXT;


/*********************************************************************
*
*       static data
*
**********************************************************************
*/

static FTPS_APP_CONTEXT  _aContext[2];  // Connecting to one server requires two connections/sessions (command and data).

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

static IP_HOOK_ON_STATE_CHANGE _StateChangeHook;
static int                     _IFaceId;

//
// Task stacks and Task-Control-Blocks.
//
static OS_STACKPTR int _IPStack[TASK_STACK_SIZE_IP_TASK/sizeof(int)];       // Stack of the IP_Task.
static OS_TASK         _IPTCB;                                              // Task-Control-Block of the IP_Task.

#if USE_RX_TASK
static OS_STACKPTR int _IPRxStack[TASK_STACK_SIZE_IP_RX_TASK/sizeof(int)];  // Stack of the IP_RxTask.
static OS_TASK         _IPRxTCB;                                            // Task-Control-Block of the IP_RxTask.
#endif

static OS_STACKPTR int _FTPStack[FTPC_STACK_SIZE/sizeof(int)];              // Stacks of the FTP server child tasks.
static OS_TASK         _FTPTCB;                                             // Task-Control-Blocks of the FTP server child tasks.

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
*       static code
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
*       _IsIPAddress()
*
*  Function description
*    Checks if string is a dot-decimal IP address, for example 192.168.1.1
*/
static unsigned _IsIPAddress(const char * sIPAddr) {
  unsigned NumDots;
  unsigned i;
  char c;

  NumDots = 0;
  i       = 0;
  while (1) {
    c = *(sIPAddr + i);
    if ((c >= '0') && (c <= '9')) {
      goto Loop;
    }
    if (c == '.') {
      NumDots++;
      goto Loop;
    }
    if (c == '\0') {
      if ((NumDots < 3) || (i > 15)) { // Error, every dot-decimal IP address includes 3 '.' and is not longer as 15 characters.
Error:
        return 0;
      }
      return 1;
    } else {
      goto Error;
    }
Loop:
    i++;
  }
}

/*********************************************************************
*
*       _ParseIPAddr()
*
*  Function description
*    Parses a string for a dot-decimal IP address and returns the
*    IP as 32-bit number in host endianess.
*/
static long _ParseIPAddr(const char * sIPAddr) {
  long     IPAddr;
  unsigned Value;
  unsigned NumDots;
  unsigned i;
  unsigned j;
  char     acDigits[4];
  char     c;

  IPAddr = 0;
  //
  // Check if string is a valid IP address
  //
  Value = _IsIPAddress(sIPAddr);
  if (Value) {
    //
    // Parse the IP address
    //
    NumDots = 3;
    i       = 0;
    j       = 0;
    while (1) {
      c = *(sIPAddr + i);
      if (c == '\0') {
        //
        // Add the last byte of the IP address.
        //
        acDigits[j] = '\0';
        Value = SEGGER_atoi(acDigits);
        if (Value < 255) {
          IPAddr |= Value;
        }
        return IPAddr; // O.K., string completely parsed. Returning IP address.
      }
      //
      // Parse the first three byte of the IP address.
      //
      if (c != '.') {
        acDigits[j] = c;
        j++;
      } else {
        acDigits[j] = '\0';
        Value = SEGGER_atoi(acDigits);
        if (Value <= 255) {
          IPAddr |= (Value << (NumDots * 8));
          NumDots--;
          j = 0;
        } else {
          return -1;  // Error, illegal number in IP address.
        }
      }
      i++;
    }
  }
  return -1;
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
  i        = 0;
  OS_DI();
  pRunner = &_aContext[0];
  do {
    if ((pRunner->PlainSocket == 0)
#if USE_SSL
     && (pRunner->Session.Socket == 0)
#endif
       ) {
      memset(pRunner, 0, sizeof(FTPS_APP_CONTEXT));
      pRunner->PlainSocket = hSock;  // Mark the entry to be in use.
      pContext             = pRunner;
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
    if (pContext->MustResume != 0) {
      SSL_SESSION_SetResumeParas(&pContext->Session, &pContext->ResumeParas);  // Bind data connection to control connection
    }
    Status = SSL_SESSION_Connect(&pContext->Session, pContext->sName);
    if (Status < 0) {
      closesocket(pContext->PlainSocket);
      FTPC_LOG(("APP: Could not upgrade to secure."));
    } else {
      if (pContext->MustResume) {
        if (SSL_SESSION_QueryFlags(&pContext->Session) & SSL_SESSION_FLAG_RESUME_GRANTED) {
          SSL_LOG((SSL_LOG_APP, "APP: Session was successfully resumed"));
        } else {
          SSL_LOG((SSL_LOG_APP, "APP: Session was not resumed -- new session ID provided by server"));
        }
      }
      FTPC_LOG(("APP: Secured using %s.", SSL_SUITE_GetIANASuiteName(SSL_SUITE_GetID(SSL_SESSION_GetSuite(&pContext->Session)))));
    }
    pContext->PlainSocket = 0;
  }
  return Status;
}
#endif


/*********************************************************************
*
*       _Connect
*
*  Function description
*    Creates a socket and opens a TCP connection to the FTP host.
*/
static FTPC_SOCKET _Connect(const char * sSrvAddr, unsigned SrvPort) {
  struct sockaddr_in sin;
  struct hostent*    pHostEntry;
  FTPS_APP_CONTEXT*  pContext;
  long               Ip;
  long               Sock;
  int                r;

  if (_IsIPAddress(sSrvAddr)) {
    Ip = _ParseIPAddr(sSrvAddr);
    Ip = htonl(Ip);
  } else {
    //
    // Convert host into IP address
    //
    pHostEntry = gethostbyname((char*)sSrvAddr);
    if (pHostEntry == NULL) {
      FTPC_LOG(("APP: gethostbyname failed: %s\r\n", sSrvAddr));
      return NULL;
    }
    Ip = *(unsigned*)(*pHostEntry->h_addr_list);
  }
  //
  // Create socket and connect to the FTP server
  //
  Sock = socket(AF_INET, SOCK_STREAM, 0);
  if(Sock  == -1) {
    FTPC_LOG(("APP: Could not create socket!" ));
    return NULL;
  }
  memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_port = htons(SrvPort);
  sin.sin_addr.s_addr = Ip;
  r = connect(Sock, (struct sockaddr*)&sin, sizeof(sin));
  if(r == SOCKET_ERROR) {
    closesocket(Sock);
    FTPC_LOG(("APP: \nSocket error :"));
    return NULL;
  }
  //
  // Alloc context.
  //
  pContext = _AllocContext(Sock);
  if (pContext == NULL) {
    closesocket(Sock);
    FTPC_LOG(("APP: Could not allocate context!" ));
    return NULL;
  }
#if USE_SSL
  pContext->sName = sSrvAddr;
#endif
  FTPC_LOG(("APP: Connected to %i, port %d.", Ip, SrvPort));
  return (FTPC_SOCKET)pContext;
}

/*********************************************************************
*
*       _Disconnect
*
*  Function description
*    Closes a socket.
*/
static void _Disconnect(FTPC_SOCKET Socket) {
  FTPS_APP_CONTEXT* pContext;

  pContext = (FTPS_APP_CONTEXT*)Socket;
  if (pContext != NULL) {
#if USE_SSL
    if (pContext->IsSecure != 0) {
      SSL_SESSION_Disconnect(&pContext->Session);
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
*       _Send
*
*  Function description
*    Sends data via socket interface.
*/
static int _Send(const char * buf, int len, void * pConnectionInfo) {
  FTPS_APP_CONTEXT* pContext;
  int               Status;

  pContext = (FTPS_APP_CONTEXT*)pConnectionInfo;
#if USE_SSL
  _UpgradeOnDemand(pContext);
  if (pContext->IsSecure != 0) {
    Status = SSL_SESSION_Send(&pContext->Session, buf, len);
  }
  else
#endif
  {
    Status = send(pContext->PlainSocket, buf, len, 0);
  }
  //
  return Status;
}

/*********************************************************************
*
*       _Recv
*
*  Function description
*    Receives data via socket interface.
*/
static int _Recv(char * buf, int len, void * pConnectionInfo) {
  FTPS_APP_CONTEXT* pContext;
  int               Status;

  pContext = (FTPS_APP_CONTEXT*)pConnectionInfo;
#if USE_SSL
  _UpgradeOnDemand(pContext);
  if (pContext->IsSecure != 0) {
    Status = SSL_SESSION_Receive(&pContext->Session, buf, len);
  }
  else
#endif
  {
    Status = recv(pContext->PlainSocket, buf, len, 0);
  }
  //
  return Status;
}

#if USE_SSL
/*********************************************************************
*
*       _SetSecure
*
*  Function description
*    Receives data via socket interface.
*/
static int _SetSecure(FTPC_SOCKET Socket, FTPC_SOCKET Clone) {
  FTPS_APP_CONTEXT* pSocket;
  FTPS_APP_CONTEXT* pClone;
  //
  pSocket = (FTPS_APP_CONTEXT*)Socket;
  pClone  = (FTPS_APP_CONTEXT*)Clone;
  //
  pSocket->IsSecure     = 1;
  if (pClone != NULL) {
    pSocket->MustResume = 1;
  } else {
    pSocket->MustResume = 0;
  }
  if (pClone != NULL) {
    SSL_SESSION_GetResumeParas(&pClone->Session, &pSocket->ResumeParas);
  }
  //
  return 0;
}
#endif

//
// IP API.
//
static const IP_FTPC_API _IP_Api = {
  _Connect,
  _Disconnect,
  _Send,
  _Recv,
#if USE_SSL
  _SetSecure
#else
  NULL
#endif
};

/*********************************************************************
*
*       _FSTest
*
*  Function description
*    Initializes the file system and creates a test file on storage medium
*/
static void _FSTest(void) {
  FS_FILE    * pFile;
  unsigned     Len;
  const char * sInfo = "SEGGER embOS/IP FTP client.\r\nFor further information please visit: www.segger.com\r\n";

  FS_Init();
  Len = strlen(sInfo);
  if (FS_IsLLFormatted("") == 0) {
    FS_X_Log("Low level formatting");
    FS_FormatLow("");          // Erase & Low-level format the volume
  }
  if (FS_IsHLFormatted("") == 0) {
    FS_X_Log("High level formatting\n");
    FS_Format("", NULL);       // High-level format the volume
  }
  pFile = FS_FOpen("Readme.txt", "w");
  FS_Write(pFile, sInfo, Len);
  FS_FClose(pFile);
  FS_Unmount("");
}

/*********************************************************************
*
*       _FTPClientTask
*
*/
static void _FTPClientTask(void) {
  IP_FTPC_CONTEXT FTPConnection;
  U8              acCtrlIn[FTPC_CTRL_BUFFER_SIZE];
  U8              acDataIn[FTPC_BUFFER_SIZE];
  U8              acDataOut[FTPC_BUFFER_SIZE];
  unsigned        Mode;
  int             r;

  //
  // FTP client task
  //
  BSP_SetLED(0);
  //
  // Initialize FTP client context
  //
  memset(&FTPConnection, 0, sizeof(FTPConnection));
  //
  // Initialize the FTP client
  //
  IP_FTPC_Init(&FTPConnection, &_IP_Api, &IP_FS_FS, acCtrlIn, sizeof(acCtrlIn), acDataIn, sizeof(acDataIn), acDataOut, sizeof(acDataOut));
  //
  // Connect to the FTP server
  //
  Mode  = FTPC_MODE_PASSIVE;
#if USE_SSL
  #if FTP_PORT == 990
    Mode |= FTPC_MODE_IMPLICIT_TLS_REQUIRED;
  #else
    Mode |= FTPC_MODE_EXPLICIT_TLS_REQUIRED;
  #endif
#endif
  r = IP_FTPC_Connect(&FTPConnection, FTP_HOST, FTP_USER, FTP_PASS, FTP_PORT, Mode);
  if (r == FTPC_ERROR) {
    FTPC_LOG(("APP: Could not connect to FTP server.\r\n"));
    goto Disconnect;
  }
  //
  // Create the directory "Test"
  //
  r = IP_FTPC_ExecCmd(&FTPConnection, FTPC_CMD_MKD, "Test");
  if (r == FTPC_ERROR) {
    FTPC_LOG(("APP: Could not change working directory.\r\n"));
    goto Disconnect;
  }
  //
  // Change from root directory into directory "Test"
  //
  r = IP_FTPC_ExecCmd(&FTPConnection, FTPC_CMD_CWD, "/Test/");
  if (r == FTPC_ERROR) {
    FTPC_LOG(("APP: Could not change working directory.\r\n"));
    goto Disconnect;
  }
  //
  // Upload the file "Readme.txt"
  //
  r = IP_FTPC_ExecCmd(&FTPConnection, FTPC_CMD_STOR, "Readme.txt");
  if (r == FTPC_ERROR) {
    FTPC_LOG(("APP: Could not upload data file.\r\n"));
    goto Disconnect;
  }
  //
  // List directory content
  //
  r = IP_FTPC_ExecCmd(&FTPConnection, FTPC_CMD_LIST, NULL);
  if (r == FTPC_ERROR) {
    FTPC_LOG(("APP: Could not list directory.\r\n"));
    goto Disconnect;
  }
  IP_Logf_Application("%s", acDataIn);
  //
  // Delete the file "Readme.txt"
  //
  r = IP_FTPC_ExecCmd(&FTPConnection, FTPC_CMD_DELE, "Readme.txt");
  if (r == FTPC_ERROR) {
    FTPC_LOG(("APP: Could not upload data file.\r\n"));
    goto Disconnect;
  }
  //
  // List directory content
  //
  r = IP_FTPC_ExecCmd(&FTPConnection, FTPC_CMD_LIST, NULL);
  if (r == FTPC_ERROR) {
    FTPC_LOG(("APP: Could not list directory.\r\n"));
    goto Disconnect;
  }
  IP_Logf_Application("%s", acDataIn);
  //
  // Change back to root directory.
  //
  r = IP_FTPC_ExecCmd(&FTPConnection, FTPC_CMD_CDUP, NULL);
  if (r == FTPC_ERROR) {
    FTPC_LOG(("APP: Change to parent directory failed.\r\n"));
    goto Disconnect;
  }
  //
  // Delete the directory "Test"
  //
  r = IP_FTPC_ExecCmd(&FTPConnection, FTPC_CMD_RMD, "Test");
  if (r == FTPC_ERROR) {
    FTPC_LOG(("APP: Could not change working directory.\r\n"));
    goto Disconnect;
  }
  //
  // Disconnect.
  //
Disconnect:
  IP_FTPC_Disconnect(&FTPConnection);
  FTPC_LOG(("APP: Done.\r\n"));
  BSP_ClrLED(0);
  //
  while (1) {
    BSP_ToggleLED(1);
    OS_Delay(500);
  }
}

/*********************************************************************
*
*       MainTask()
*
* Function description
*   Main task executed by the RTOS to create further resources and
*   running the main application.
*/
void MainTask(void) {
  //
  // Create a test file on storage medium
  //
  _FSTest();
  //
  // Initialize the IP stack
  //
  IP_Init();
#if USE_SSL
  //
  // Initialize the SSL stack
  //
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
  //
  // Check if target is configured
  //
  while (IP_IFaceIsReadyEx(_IFaceId) == 0) {
    BSP_ToggleLED(1);
    OS_Delay(50);
  }
  OS_CREATETASK(&_FTPTCB, "FTPClient", _FTPClientTask, TASK_PRIO_FTPC, _FTPStack);
  while (1) {
    OS_Delay(500);
  }
}


/*************************** End of file ****************************/
