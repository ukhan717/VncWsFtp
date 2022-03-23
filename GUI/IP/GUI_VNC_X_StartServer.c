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

File    : GUI_VNC_X_StartServer.c
Purpose : Starts the VNC server using plain BSD socket API and
          alternate prepares handling of an incoming WebSocket
          connection handled by the embOS/IP web server.
          By default only the plain BSD socket API will be used.
          The WebSocket handling can be activated in GUIConf.h by
          enabling GUI_VNC_SUPPORT_WEBSOCKET_SERVER .
          The WebSocket based connection is intended to be used
          with the noVNC client written in HTML5 that uses
          WebSockets for communication.
          The noVNC sample per default uses the URI "/websockify"
          and the WebSocket sub protocol "binary".
*/

#include "GUI.h"
#include "GUI_VNC.h"
#include "RTOS.h"
#include "IP.h"
#include "TaskPrio.h"

/*********************************************************************
*
*       Defines, configurable
*
**********************************************************************
*/

//
// Default configuration. Use GUIConf.h for override.
//
#ifndef   GUI_VNC_SUPPORT_PLAIN_SOCKET_SERVER
  #define GUI_VNC_SUPPORT_PLAIN_SOCKET_SERVER  1
#endif
#ifndef   GUI_VNC_SUPPORT_WEBSOCKET_SERVER
  #define GUI_VNC_SUPPORT_WEBSOCKET_SERVER     0
#endif

#if GUI_VNC_SUPPORT_WEBSOCKET_SERVER
#include "IP_WEBSOCKET.h"
#include "IP_Webserver.h"

//
// WebSocket web server configuration.
//
#ifndef   WEBSOCKET_VNC_URI
  #define WEBSOCKET_VNC_URI    "/websockify"
#endif
#ifndef   WEBSOCKET_VNC_PROTO
  #define WEBSOCKET_VNC_PROTO  "binary"
#endif
#endif

//
// Task stack sizes that might not fit for some interfaces (multiples of sizeof(int)).
//
#ifndef   APP_TASK_STACK_OVERHEAD
  #define APP_TASK_STACK_OVERHEAD  0
#endif

/*********************************************************************
*
*       Defines, fixed
*
**********************************************************************
*/

enum {
  SOCKET_TYPE_BSD = 0,
  SOCKET_TYPE_WEBSOCKET
};

/*********************************************************************
*
*       Prototypes
*
**********************************************************************
*/

#if GUI_VNC_SUPPORT_WEBSOCKET_SERVER
static int  _cbWebSocket_GenerateAcceptKey (WEBS_OUTPUT* pOutput, void* pSecWebSocketKey, int SecWebSocketKeyLen, void* pBuffer, int BufferSize);
static void _cbWebSocket_DispatchConnection(WEBS_OUTPUT* pOutput, void* pConnection);
static int  _cbWebSocket_Recv              (IP_WEBSOCKET_CONTEXT* pContext, IP_WEBSOCKET_CONNECTION* pConnection,       void* pData, unsigned NumBytes);
static int  _cbWebSocket_Send              (IP_WEBSOCKET_CONTEXT* pContext, IP_WEBSOCKET_CONNECTION* pConnection, const void* pData, unsigned NumBytes);
#endif

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

static GUI_VNC_CONTEXT _Context;
static void*           _pConnection;      // NULL if disconnected. Set (if NULL) when accepting a connection to tell the VNC
                                          // server task that a new connection has been opened.
#if ((GUI_VNC_SUPPORT_PLAIN_SOCKET_SERVER != 0) || (GUI_VNC_SUPPORT_WEBSOCKET_SERVER != 0))
static U8              _SocketType;       // SOCKET_TYPE_BSD or SOCKET_TYPE_WEBSOCKET .
#endif

#if GUI_VNC_SUPPORT_WEBSOCKET_SERVER
//
// WebSocket resources.
//
static WEBS_WEBSOCKET_HOOK _WebSocketHook;

static const IP_WEBS_WEBSOCKET_API _WebServerWebSocketAPI = {
  _cbWebSocket_GenerateAcceptKey,  // pfGenerateAcceptKey
  _cbWebSocket_DispatchConnection  // pfDispatchConnection
};

static const IP_WEBSOCKET_TRANSPORT_API _WebSocketTransportAPI = {
  _cbWebSocket_Recv,  // pfReceive
  _cbWebSocket_Send   // pfSend
};
#endif

//
// Task stacks and Task-Control-Blocks.
//
static OS_STACKPTR int _VNCServerStack[(4000 + APP_TASK_STACK_OVERHEAD)/sizeof(int)];
static OS_TASK         _VNCServerTCB;

/*********************************************************************
*
*       Local functions
*
**********************************************************************
*/

#if GUI_VNC_SUPPORT_WEBSOCKET_SERVER

/*********************************************************************
*
*       _cbWebSocket_GenerateAcceptKey()
*
*  Function description
*    Generates the value to send back for the Sec-WebSocket-Accept
*    field when accepting the connection.
*
*  Parameters
*    pOutput           : Web server connection context.
*    pSecWebSocketKey  : Pointer to a buffer containing the string of
*                        the Sec-WebSocket-Key from the HTTP request.
*    SecWebSocketKeyLen: Number of characters of the Sec-WebSocket-Key
*                        (without string termination).
*    pBuffer           : Buffer where to store the accept key.
*    BufferSize        : Size of buffer where to store the accept key.
*
*  Return value
*    Length of accept key        : >  0
*    Error, buffer not big enough: == 0
*/
static int _cbWebSocket_GenerateAcceptKey(WEBS_OUTPUT* pOutput, void* pSecWebSocketKey, int SecWebSocketKeyLen, void* pBuffer, int BufferSize) {
  WEBS_USE_PARA(pOutput);

  return IP_WEBSOCKET_GenerateAcceptKey(pSecWebSocketKey, SecWebSocketKeyLen, pBuffer, BufferSize);
}

/*********************************************************************
*
*       _cbWebSocket_DispatchConnection()
*
*  Function description
*    Dispatches the web server connection to the VNC server for
*    further handling and signals the VNC server task.
*
*  Parameters
*    pOutput    : Web server connection context.
*    pConnection: Network connection handle.
*/
static void _cbWebSocket_DispatchConnection(WEBS_OUTPUT* pOutput, void* pConnection) {
  WEBS_USE_PARA(pOutput);

  OS_EnterRegion();  // Make sure that _pConnection does not get set from another task after being read by us.
  //
  // Check if no other VNC client is in process right now.
  //
  if (_pConnection == NULL) {
    //
    // Mark a VNC client in process before re-enabling
    // task switches and signaling the VNC server task.
    //
    _SocketType  = SOCKET_TYPE_WEBSOCKET;
    _pConnection = pConnection;
    OS_LeaveRegion();
  } else {
    OS_LeaveRegion();
    //
    // Another VNC client is handled. Simply
    // close the new socket to close the WebSocket
    // connection as well.
    // Instead of closing the new connection the old
    // connection could be closed for continuation
    // with the new client.
    //
    closesocket((long)pConnection);
  }
}

/*********************************************************************
*
*       _cbWebSocket_Recv()
*
*  Function description
*    WebSocket callback that for sending data using the underlying
*    network communication API (typically BSD socket API).
*
*  Parameters
*    pContext   : WebSocket context.
*    pConnection: Network connection handle.
*    pData      : Where to store the received data.
*    NumBytes   : Maximum amount of data to receive.
*
*  Return value
*    Amount of data received: >  0
*    Connection closed      : == 0
*    Error                  : <  0
*/
static int _cbWebSocket_Recv(IP_WEBSOCKET_CONTEXT* pContext, IP_WEBSOCKET_CONNECTION* pConnection, void* pData, unsigned NumBytes) {
  IP_USE_PARA(pContext);

  return recv((long)pConnection, (char*)pData, NumBytes, 0);
}

/*********************************************************************
*
*       _cbWebSocket_Send()
*
*  Function description
*    WebSocket callback that for sending data using the underlying
*    network communication API (typically BSD socket API).
*
*  Parameters
*    pContext   : WebSocket context.
*    pConnection: Network connection handle.
*    pData      : Data to send.
*    NumBytes   : Amount of data to send.
*
*  Return value
*    Amount of data sent: >  0
*    Connection closed  : == 0
*    Error              : <  0
*/
static int _cbWebSocket_Send(IP_WEBSOCKET_CONTEXT* pContext, IP_WEBSOCKET_CONNECTION* pConnection, const void* pData, unsigned NumBytes) {
  IP_USE_PARA(pContext);

  return send((long)pConnection, (const char*)pData, NumBytes, 0);
}

/*********************************************************************
*
*       _cbVNC_RecvWebSocket()
*
*  Function description
*    This function is called indirectly by the server; it's address
*    is passed to the actual server code as function pointer. It is
*    needed because the server is independent of the TCP/IP stack
*    implementation, so details for the TCP/IP stack can be placed
*    here.
*
*  Parameters
*    pData      : Where to store the received data.
*    NumBytes   : Maximum amount of data to receive.
*    pConnection: Connection handle (WebSocket context).
*
*  Return value
*    Amount of data received: >  0
*    Connection closed      : == 0
*    Error                  : <  0
*/
static int _cbVNC_RecvWebSocket(U8* pData, int NumBytes, void* pConnection) {
  IP_WEBSOCKET_CONTEXT* pContext;
  int r;
  U8  MessageType;

  pContext = (IP_WEBSOCKET_CONTEXT*)pConnection;
  r        = IP_WEBSOCKET_WaitForNextMessage(pContext, &MessageType);
  if (r > 0) {  // O.K. ?
    if        (MessageType == IP_WEBSOCKET_FRAME_TYPE_CLOSE) {
      IP_WEBSOCKET_Close(pContext, NULL, IP_WEBSOCKET_CLOSE_CODE_NORMAL_CLOSURE);
      r = 0;    // Pretend connection closed.
    } else if (MessageType == IP_WEBSOCKET_FRAME_TYPE_BINARY) {  // VNC uses binary messages.
      r = IP_WEBSOCKET_Recv(pContext, pData, NumBytes);
    } else {
      r = -1;   // Error, pretend communication error as we expect binary messages.
    }
  }
  return r;
}

/*********************************************************************
*
*       _cbVNC_SendWebSocket()
*
*  Function description
*    This function is called indirectly by the server; it's address
*    is passed to the actual server code as function pointer. It is
*    needed because the server is independent of the TCP/IP stack
*    implementation, so details for the TCP/IP stack can be placed
*    here.
*
*  Parameters
*    pData      : Data to send.
*    NumBytes   : Amount of data to send.
*    pConnection: Connection handle.
*
*  Return value
*    Amount of data sent: >  0
*    Connection closed  : == 0
*    Error              : <  0
*/
static int _cbVNC_SendWebSocket(const U8* pData, int NumBytes, void* pConnection) {
  IP_WEBSOCKET_CONTEXT* pContext;
  int r;

  pContext = (IP_WEBSOCKET_CONTEXT*)pConnection;
  r        = IP_WEBSOCKET_Send(pContext, pData, NumBytes, IP_WEBSOCKET_FRAME_TYPE_BINARY, 0);  // Send a single frame (FIN bit set of data).
  if (r == 0) {  // Connection closed ? => Send our close.
    IP_WEBSOCKET_Close(pContext, NULL, IP_WEBSOCKET_CLOSE_CODE_NORMAL_CLOSURE);
  }
  return r;
}

#endif

#if GUI_VNC_SUPPORT_PLAIN_SOCKET_SERVER

/*********************************************************************
*
*       _cbVNC_Recv()
*
*  Function description
*    This function is called indirectly by the server; it's address
*    is passed to the actual server code as function pointer. It is
*    needed because the server is independent of the TCP/IP stack
*    implementation, so details for the TCP/IP stack can be placed
*    here.
*
*  Parameters
*    pData      : Where to store the received data.
*    NumBytes   : Maximum amount of data to receive.
*    pConnection: Connection handle (WebSocket context).
*
*  Return value
*    Amount of data received: >  0
*    Connection closed      : == 0
*    Error                  : <  0
*/
static int _cbVNC_Recv(U8* pData, int NumBytes, void* pConnection) {
  return recv((long)pConnection, (char*)pData, NumBytes, 0);
}

/*********************************************************************
*
*       _cbVNC_Send()
*
*  Function description
*    This function is called indirectly by the server; it's address
*    is passed to the actual server code as function pointer. It is
*    needed because the server is independent of the TCP/IP stack
*    implementation, so details for the TCP/IP stack can be placed
*    here.
*
*  Parameters
*    pData      : Data to send.
*    NumBytes   : Amount of data to send.
*    pConnection: Connection handle.
*
*  Return value
*    Amount of data sent: >  0
*    Connection closed  : == 0
*    Error              : <  0
*/
static int _cbVNC_Send(const U8* pData, int NumBytes, void* pConnection) {
  return send((long)pConnection, (const char*)pData, NumBytes, 0);
}

/*********************************************************************
*
*       _ListenAtTCPAddr()
*
*  Function description
*    Creates a non-blocking socket for accepting a VNC connection.
*
*  Parameters
*    Port: Port to use for accepting connections.
*
*  Return value
*    Parent socket handle.
*/
static int _ListenAtTCPAddr(U16 Port) {
         int         hSock;
  struct sockaddr_in InAddr;

  hSock = socket(AF_INET, SOCK_STREAM, 0);
  if (hSock <= 0) {
    while (1);  // Should not happen.
  }
  memset(&InAddr, 0, sizeof(InAddr));
  InAddr.sin_family      = AF_INET;
  InAddr.sin_port        = htons(Port);
  InAddr.sin_addr.s_addr = INADDR_ANY;
  bind(hSock, (struct sockaddr*)&InAddr, sizeof(InAddr));
  listen(hSock, 1);
  setsockopt(hSock, SOL_SOCKET, SO_NBIO, NULL, 0);  // Set socket non-blocking.
  return hSock;
}

#endif

/*********************************************************************
*
*       _ServerTask()
*
*  Function description
*    This routine is the actual server task.
*    It waits in an endless loop for a new WebSocket connection to be
*    signaled and handles the VNC connection.
*/
static void _ServerTask(void) {
#if GUI_VNC_SUPPORT_WEBSOCKET_SERVER
  IP_WEBSOCKET_CONTEXT WebSocketContext;
#endif
#if GUI_VNC_SUPPORT_PLAIN_SOCKET_SERVER
  int                  hSockParent;
  int                  SoError;
  int                  hSock;
  //
  // Start listening on a BSD socket for a connection.
  // WebSocket connections will be dispatched by the Web Server.
  //
  hSockParent = _ListenAtTCPAddr(5900 + _Context.ServerIndex);  // Default port for VNC is is 590x, where x is the 0-based layer index.
#endif
  while (1) {
    //
    // Check that the connection handle is actually valid.
    // A handle of NULL is treated as invalid although some
    // TCP/IP stacks like Windows treat a socket handle of
    // 0 as valid. However this occurs very rare and in this
    // situation the peer should simply close the connection
    // after some time due to a timeout.
    // We will first check if a connection handle is present
    // which means checking for a WebSocket handle. If no
    // WebSocket handle is active we will check non-blocking
    // if there is any BSD socket to accept.
    //
#if (GUI_VNC_SUPPORT_PLAIN_SOCKET_SERVER == 0)
    if (_pConnection == NULL) {
      OS_Delay(50);  // Grant the system some time between retrying.
      continue;
    }
#else
    OS_EnterRegion();  // Make sure that _pConnection does not get set from another task after being read by us.
    if (_pConnection == NULL) {
      //
      // Check if we have a connection to accept on the regular VNC port.
      //
      hSock = accept(hSockParent, NULL, NULL);
      if (hSock == SOCKET_ERROR) {
        OS_LeaveRegion();
        //
        // Check if this is an IP_ERR_WOULD_BLOCK or a real error.
        //
        getsockopt(hSockParent, SOL_SOCKET, SO_ERROR, &SoError, sizeof(SoError));
        if (SoError != IP_ERR_WOULD_BLOCK) {
          while(1);    // Should not happen.
        }
        OS_Delay(50);  // Grant the system some time between retrying.
        continue;
      }
      //
      // We have a connection on the regular VNC port.
      //
      setsockopt(hSock, SOL_SOCKET, SO_BIO, NULL, 0);  // Set child socket blocking.
      _SocketType  = SOCKET_TYPE_BSD;
      _pConnection = (void*)hSock;
    }
    OS_LeaveRegion();
#endif
#if GUI_VNC_SUPPORT_WEBSOCKET_SERVER
    if (_SocketType == SOCKET_TYPE_WEBSOCKET) {
      //
      // Initialize the WebSocket context for the server.
      //
      IP_WEBSOCKET_InitServer(&WebSocketContext, &_WebSocketTransportAPI, (IP_WEBSOCKET_CONNECTION*)_pConnection);
      //
      // Run the actual server using the WebSocket context as connection handle.
      //
      GUI_VNC_Process(&_Context, _cbVNC_SendWebSocket, _cbVNC_RecvWebSocket, (void*)&WebSocketContext);
    }
#endif
#if GUI_VNC_SUPPORT_PLAIN_SOCKET_SERVER
    if (_SocketType == SOCKET_TYPE_BSD) {
      //
      // Run the actual server using the BSD socket handle as connection handle.
      //
      GUI_VNC_Process(&_Context, _cbVNC_Send, _cbVNC_Recv, (void*)_pConnection);
    }
#endif
    //
    // Close the connection and invalidate the connection handle
    // so we can accept a new client.
    //
    closesocket((long)_pConnection);
    _pConnection = NULL;
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
*       GUI_VNC_X_StartServer()
*
*  Function description
*    This routine starts a separate task for the VNC server. It
*    requires that the OS and TCP/IP stack are already initialized.
*
*  Parameters
*    LayerIndex : Index of the GUI layer that is shown via VNC.
*    ServerIndex: Zero-based server index.
*
*  Return value
*    O.K.: 0
*
*  Additional information
*    This sample is intended to be used for starting one VNC server.
*    In case multiple layers shall be accessed by different server
*    instances (ServerIndex) this sample needs to be modified.
*/
int GUI_VNC_X_StartServer(int LayerIndex, int ServerIndex) {
  //
  // Init VNC context and attach to layer (so context is
  // updated if the display-layer-contents change).
  //
  GUI_VNC_AttachToLayer(&_Context, LayerIndex);
  _Context.ServerIndex = ServerIndex;
  //
  // Create task for VNC server. It will wait for the signal
  // of a dispatched WebSocket connection.
  //
  OS_CREATETASK(&_VNCServerTCB, "VNC Server", _ServerTask, TASKPRIO_VNC, _VNCServerStack);
#if GUI_VNC_SUPPORT_WEBSOCKET_SERVER
  //
  // Enable WebSockets for the web server and setup the
  // URI "/websockify" with sub protocol "binary" for the
  // VNC data transfer.
  //
  IP_WEBS_WEBSOCKET_AddHook(&_WebSocketHook, &_WebServerWebSocketAPI, WEBSOCKET_VNC_URI, WEBSOCKET_VNC_PROTO);
#endif
  return 0;
}

/*********************************************************************
*
*       GUI_VNC_X_getpeername()
*
*  Function description
*    Retrieves the IP addr. of the currently connected VNC client.
*
*  Parameters
*    pIPAddr:[OUT] VNC client connected: U32 IP addr. in network endianess.
*                  No client connected : 0
*/
void GUI_VNC_X_getpeername(U32* pIPAddr) {
  struct sockaddr_in Client;
  int Len;

  if (_pConnection != NULL) {
    Len   = sizeof(Client);
    getpeername((long)_pConnection, (struct sockaddr*)&Client, &Len);
    *pIPAddr = Client.sin_addr.s_addr;
  } else {
    *pIPAddr = 0;
  }
}

/*************************** End of file ****************************/

