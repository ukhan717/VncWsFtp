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

File    : IP_MQTT_CLIENT_Subscriber.c
Purpose : Sample program for embOS & embOS/IP demonstrating an MQTT subscriber.
*/

#include "RTOS.h"
#include "BSP.h"
#include "IP.h"
#include "IP_MQTT_CLIENT.h"
#include "SEGGER.h"

/*********************************************************************
*
*       Configuration
*
**********************************************************************
*/

#define USE_RX_TASK                0  // 0: Packets are read in ISR, 1: Packets are read in a task of its own.

#define MQTT_CLIENT_BUFFER_SIZE    256
#define RCV_TIMEOUT                5000

#define MQTT_BROKER_ADDR           "test.mosquitto.org"
#define MQTT_BROKER_PORT           1883

#define MQTT_CLIENT_NAME           "eMQTT_Sub"
#define TOPIC_FILTER_TO_SUBSCRIBE  "eMQTT"

//
// Task priorities.
//
enum {
   TASK_PRIO_IP_TASK = 150  // Priority must be higher as all IP application tasks.
#if USE_RX_TASK
  ,TASK_PRIO_IP_RX_TASK     // Must be the highest priority of all IP related tasks.
#endif
};

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

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

static char            _acBuffer[MQTT_CLIENT_BUFFER_SIZE];                  // Memory block used by the MQTT client.
static char            _acTopic[200];                                       // Memory used to store the received topic for printf().
static char            _aPayload[200];                                      // Memory used to store the received payload for printf().

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
*       Local functions
*
**********************************************************************
*/

/*********************************************************************
*
*       _OnStateChange()
*
*  Function description
*    Callback that will be notified once the state of an interface
*    changes.
*
*  Parameters
*    IFaceId   : Zero-based interface index.
*    AdminState: Is this interface enabled ?
*    HWState   : Is this interface physically ready ?
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
static unsigned _IsIPAddress(const char* sIPAddr) {
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
*    IP as 32-bit number in host endianness.
*/
static long _ParseIPAddr(const char* sIPAddr) {
  long     IPAddr;
  unsigned Value;
  unsigned NumDots;
  unsigned i;
  unsigned j;
  char     acDigits[4];
  char     c;

  IPAddr = 0;
  //
  // Check if string is a valid IP address.
  //
  Value = _IsIPAddress(sIPAddr);
  if (Value) {
    //
    // Parse the IP address.
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
*       _Connect()
*
*  Function description
*    Creates a socket and opens a TCP connection to the MQTT broker.
*/
static IP_MQTT_CLIENT_SOCKET _Connect(const char* sBrokerAddr, unsigned BrokerPort) {
  struct hostent*    pHostEntry;
  struct sockaddr_in sin;
  long               Ip;
  long               hSock;
  int                r;
  U32                Timeout;

  if (_IsIPAddress(sBrokerAddr)) {
    Ip = _ParseIPAddr(sBrokerAddr);
    Ip = htonl(Ip);
  } else {
    //
    // Convert host into IP address.
    //
    pHostEntry = gethostbyname((char*)sBrokerAddr);
    if (pHostEntry == NULL) {
      IP_MQTT_CLIENT_LOG(("APP: gethostbyname failed: %s", sBrokerAddr));
      return NULL;
    }
    Ip = *(unsigned*)(*pHostEntry->h_addr_list);
  }
  //
  // Create socket and connect to the MQTT broker.
  //
  hSock = socket(AF_INET, SOCK_STREAM, 0);
  if(hSock  == -1) {
    IP_MQTT_CLIENT_LOG(("APP: Could not create socket!"));
    return NULL;
  }
  //
  // Set receive timeout.
  //
  Timeout = RCV_TIMEOUT;
  setsockopt(hSock , SOL_SOCKET, SO_RCVTIMEO, (char*)&Timeout, sizeof(Timeout));
  //
  // Connect.
  //
  memset(&sin, 0, sizeof(sin));
  sin.sin_family      = AF_INET;
  sin.sin_port        = htons((U16)BrokerPort);
  sin.sin_addr.s_addr = Ip;
  r = connect(hSock, (struct sockaddr*)&sin, sizeof(sin));
  if(r == SOCKET_ERROR) {
    closesocket(hSock);
    IP_MQTT_CLIENT_LOG(("APP: Connect error."));
    return NULL;
  }
  IP_MQTT_CLIENT_LOG(("APP: Connected to %i, port %d.", Ip, BrokerPort));
  return (IP_MQTT_CLIENT_SOCKET)hSock;
}

/*********************************************************************
*
*       _Disconnect()
*
*  Function description
*    Closes a socket.
*/
static void _Disconnect(void* pSocket) {
  if (pSocket) {
    closesocket((long)pSocket);
  }
}

/*********************************************************************
*
*       _Recv()
*
*  Function description
*    Receives data via socket interface.
*
*  Return value
*    >   0: O.K., number of bytes received.
*    ==  0: Connection has been gracefully closed by the broker.
*    == -1: Error.
*/
static int _Recv(void* pSocket, char* pBuffer, int NumBytes) {
  return recv((long)pSocket, pBuffer, NumBytes, 0);
}

/*********************************************************************
*
*       _Send()
*
*  Function description
*    Sends data via socket interface.
*/
static int _Send(void* pSocket, const char* pBuffer, int NumBytes) {
  return send((long)pSocket, pBuffer, NumBytes, 0);
}

static const IP_MQTT_CLIENT_TRANSPORT_API _IP_Api = {
  _Connect,
  _Disconnect,
  _Recv,
  _Send
};

/*********************************************************************
*
*       _GenRandom()
*
*  Function description
*    Generates a random number.
*/
static U16 _GenRandom(void) {
  U32 TimeStamp;

  TimeStamp = OS_GetTime32();
  return (U16)TimeStamp;  // Return a random value, which can be used for packet Id, ...
}


static const IP_MQTT_CLIENT_APP_API _APP_Api = {
  _GenRandom,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL
};


/*********************************************************************
*
*       _MQTT_Client()
*
*  Function description
*    MQTT client application.
*/
static void _MQTT_Client(void) {
  IP_MQTT_CLIENT_CONTEXT      MQTTClient;
  IP_MQTT_CLIENT_TOPIC_FILTER TopicFilter;
  IP_MQTT_CLIENT_SUBSCRIBE    Subscribe;
  U32                         v;
  U32                         Cnt;
  U32                         Topic;
  U32                         TotalLen;
  U32                         NumBytesBuffer;
  int                         r;
  int                         Error;

  TotalLen       = 0;
  Cnt            = 0;
  Error          = 0;
  NumBytesBuffer = sizeof(_aPayload);
  //
  // Initialize MQTT client context.
  //
  IP_MQTT_CLIENT_Init(&MQTTClient, _acBuffer, MQTT_CLIENT_BUFFER_SIZE, &_IP_Api, &_APP_Api, MQTT_CLIENT_NAME);
  //
  // Main application loop.
  //
  do {
    //
    // Connect to the MQTT broker.
    //
    r = IP_MQTT_CLIENT_Connect(&MQTTClient, MQTT_BROKER_ADDR, MQTT_BROKER_PORT, IP_MQTT_CLIENT_CLEAN_SESSION);
    if (r != 0) {
      IP_MQTT_CLIENT_LOG(("APP: MQTT connect error code: %d.", r));
      goto Disconnect;
    }
    BSP_SetLED(0);
    //
    // Initialize the topic filter and subscribe structures.
    //
    memset(&TopicFilter, 0, sizeof(TopicFilter));
    TopicFilter.sTopicFilter = TOPIC_FILTER_TO_SUBSCRIBE;
    TopicFilter.Length       = strlen(TOPIC_FILTER_TO_SUBSCRIBE);
    memset(&Subscribe, 0, sizeof(Subscribe));
    Subscribe.pTopicFilter   = &TopicFilter;
    Subscribe.TopicCnt       = 1;
    //
    // Subscribe to topic.
    //
    r = IP_MQTT_CLIENT_Subscribe(&MQTTClient, &Subscribe);
    if (r > 0) {
      //
      // Receive a message.
      //
      do {
WaitForMessage:
        Topic    = 0;
        TotalLen = 0;
        r        = IP_MQTT_CLIENT_WaitForNextMessage(&MQTTClient, &Topic, &TotalLen, _acTopic, sizeof(_acTopic));
        if ((r > 0) && (Topic == IP_MQTT_CLIENT_TYPES_PUBLISH)) {
          Cnt++;
          IP_MQTT_CLIENT_LOG(("APP: --------"));
          IP_MQTT_CLIENT_LOG(("APP: Message No. %d:", Cnt));
          if (r < (int)sizeof(_acTopic)) {
            _acTopic[r] = '\0';   // Terminate to visualize topic with printf().
            IP_MQTT_CLIENT_LOG(("APP:   Topic  : %s", _acTopic));
          }
          if (TotalLen > 0) {        // Check since MQTT publish packets without payload are possible.
            //
            // If the payload is larger than the buffer, we have to loop until we have
            // received all payload bytes.
            //
            v = (NumBytesBuffer - 1);  // Buffer size - 1 byte for string termination
            do {
              v = SEGGER_MIN(v, TotalLen);
RetryOnTimeout:
              r = IP_MQTT_CLIENT_Recv(&MQTTClient, _aPayload, v);
              if        (r == 0) {
                break;             // Broker closed the connection.
              } else if (r < 0) {
                //
                // Check if we have an error or just a read timeout.
                //
                Error = 0;
                getsockopt((long)MQTTClient.Connection.Socket, SOL_SOCKET, SO_ERROR, &Error, sizeof(Error));
                if (Error == IP_ERR_TIMEDOUT) {
                  goto RetryOnTimeout;
                }
              }
              TotalLen -= r;
              _aPayload[r] = '\0';  // Terminate to visualize payload with printf. Please note, the payload of an MQTT packet is not necessarily a string.
              IP_MQTT_CLIENT_LOG(("APP:   Payload: %s", _aPayload));
            } while(TotalLen != 0);
          }
        } else {
          //
          // Check if we have an error or just a read timeout.
          //
          if (r < 0) {
            getsockopt((long)MQTTClient.Connection.Socket, SOL_SOCKET, SO_ERROR, &Error, sizeof(Error));
            if (Error == IP_ERR_TIMEDOUT) {
              goto WaitForMessage;
            }
          }
        }
      } while (r != -1);
    }
    //
    // Disconnect.
    //
Disconnect:
    IP_MQTT_CLIENT_Disconnect(&MQTTClient);
    IP_MQTT_CLIENT_LOG(("APP: Done."));
    BSP_ClrLED(0);
    OS_Delay(10000);
  } while (1);
}

/*********************************************************************
*
*       MainTask()
*
*  Function description
*    Main task executed by the RTOS to create further resources and
*    running the main application.
*/
void MainTask(void) {
  IP_Init();
  _IFaceId = IP_INFO_GetNumInterfaces() - 1;                                           // Get the last registered interface ID as this is most likely the interface we want to use in this sample.
  OS_SetPriority(OS_GetTaskID(), TASK_PRIO_IP_TASK - 1);                               // For now, this task has highest prio except IP management tasks.
  OS_CREATETASK(&_IPTCB  , "IP_Task"  , IP_Task  , TASK_PRIO_IP_TASK   , _IPStack);    // Start the IP_Task.
#if USE_RX_TASK
  OS_CREATETASK(&_IPRxTCB, "IP_RxTask", IP_RxTask, TASK_PRIO_IP_RX_TASK, _IPRxStack);  // Start the IP_RxTask, optional.
#endif
  IP_AddStateChangeHook(&_StateChangeHook, _OnStateChange);                            // Register hook to be notified on disconnects.
  IP_Connect(_IFaceId);                                                                // Connect the interface if necessary.
  while (IP_IFaceIsReadyEx(_IFaceId) == 0) {
    BSP_ToggleLED(1);
    OS_Delay(50);
  }
  BSP_ClrLED(0);
  _MQTT_Client();
}

/*************************** End of file ****************************/
