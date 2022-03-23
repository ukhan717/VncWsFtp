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

File    : IP_MQTT_CLIENT.h
Purpose : Header file for the MQTT (MQ Telemetry Transport) client.
*/

#ifndef IP_MQTT_CLIENT_H      // Avoid multiple inclusion.
#define IP_MQTT_CLIENT_H

#if defined(__cplusplus)
  extern "C" {                // Make sure we have C-declarations in C++ programs.
#endif

#include "SEGGER.h"
#include "IP_MQTT_CLIENT_Conf.h"

/*********************************************************************
*
*       Defines, configurable
*
**********************************************************************
*/

//
// For processing QoS 2 messages a subscriber needs to remember
// the packet IDs to be able to process data using only one buffer.
// A small amount of packet IDs to remember should be sufficient
// as QoS 2 messages not completely processed will be retransmitted anyhow.
//
#ifndef   IP_MQTT_CLIENT_RECEIVED_ID_CACHE
  #define IP_MQTT_CLIENT_RECEIVED_ID_CACHE  10
#endif

/*********************************************************************
*
*       Defines, fixed
*
**********************************************************************
*/

#define IP_MQTT_CLIENT_CLEAN_SESSION              1

//
//  MQTT Control Packet types
//
#define IP_MQTT_CLIENT_TYPES_CONNECT              1   // Client request to connect to Server        (Client to Server)
#define IP_MQTT_CLIENT_TYPES_CONNACK              2   // Connect acknowledgment                     (Server to Client)
#define IP_MQTT_CLIENT_TYPES_PUBLISH              3   // Client to Server or Publish message        (Client to Server or Server to Client)
#define IP_MQTT_CLIENT_TYPES_PUBACK               4   // Publish acknowledgment                     (Client to Server or Server to Client)
#define IP_MQTT_CLIENT_TYPES_PUBREC               5   // Publish received (assured delivery part 1) (Client to Server or Server to Client)
#define IP_MQTT_CLIENT_TYPES_PUBREL               6   // Publish release (assured delivery part 2)  (Client to Server or Server to Client)
#define IP_MQTT_CLIENT_TYPES_PUBCOMP              7   // Publish complete (assured delivery part 3) (Client to Server or Server to Client)
#define IP_MQTT_CLIENT_TYPES_SUBSCRIBE            8   // Client subscribe request                   (Client to Server)
#define IP_MQTT_CLIENT_TYPES_SUBACK               9   // Subscribe acknowledgment                   (Server to Client)
#define IP_MQTT_CLIENT_TYPES_UNSUBSCRIBE         10   // Unsubscribe request                        (Client to Server)
#define IP_MQTT_CLIENT_TYPES_UNSUBACK            11   // Unsubscribe acknowledgment                 (Server to Client)
#define IP_MQTT_CLIENT_TYPES_PINGREQ             12   // PING request                               (Client to Server)
#define IP_MQTT_CLIENT_TYPES_PINGRESP            13   // PING response                              (Server to Client)
#define IP_MQTT_CLIENT_TYPES_DISCONNECT          14   // Client is disconnecting                    (Client to Server)

//
//  MQTT Control Packet flags
//
// Control Packet | Fixed header flags | Bit 3 | Bit 2 | Bit 1 | Bit 0
// --------------------------------------------------------------------
// CONNECT        | Reserved           | 0     | 0     | 0     |  0
// CONNACK        | Reserved           | 0     | 0     | 0     |  0
// PUBLISH        | Used in MQTT 3.1.1 | DUP   | QoS   | QoS   | RETAIN
// PUBACK         | Reserved           | 0     | 0     | 0     |  0
// PUBREC         | Reserved           | 0     | 0     | 0     |  0
// PUBREL         | Reserved           | 0     | 0     | 1     |  0
// PUBCOMP        | Reserved           | 0     | 0     | 0     |  0
// SUBSCRIBE      | Reserved           | 0     | 0     | 1     |  0
// SUBACK         | Reserved           | 0     | 0     | 0     |  0
// UNSUBSCRIBE    | Reserved           | 0     | 0     | 1     |  0
// UNSUBACK       | Reserved           | 0     | 0     | 0     |  0
// PINGREQ        | Reserved           | 0     | 0     | 0     |  0
// PINGRESP       | Reserved           | 0     | 0     | 0     |  0
// DISCONNECT     | Reserved           | 0     | 0     | 0     |  0
//
#define IP_MQTT_CLIENT_FLAGS_PUBREL               (1uL << 1)
#define IP_MQTT_CLIENT_FLAGS_SUBSCRIBE            (1uL << 1)
#define IP_MQTT_CLIENT_FLAGS_UNSUBSCRIBE          (1uL << 1)
#define IP_MQTT_CLIENT_FLAGS_PUBLISH_RETAIN       (1uL << 0)
#define IP_MQTT_CLIENT_FLAGS_PUBLISH_QOS0         (0uL << 0)   // At most once delivery
#define IP_MQTT_CLIENT_FLAGS_PUBLISH_QOS1         (1uL << 1)   // At least once delivery
#define IP_MQTT_CLIENT_FLAGS_PUBLISH_QOS2         (1uL << 2)   // Exactly once delivery
#define IP_MQTT_CLIENT_FLAGS_PUBLISH_QOS_DUP      (1uL << 3)


/*********************************************************************
*
*       Types
*
**********************************************************************
*/

typedef void* IP_MQTT_CLIENT_SOCKET;

typedef struct {
  void* (*pfConnect)   (const char* sSrvAddr, unsigned SrvPort);
  void  (*pfDisconnect)(void* pSocket);
  int   (*pfReceive)   (void* pSocket, char* pData, int Len);
  int   (*pfSend)      (void* pSocket, const char* pData, int Len);
} IP_MQTT_CLIENT_TRANSPORT_API;


typedef struct {
  const IP_MQTT_CLIENT_TRANSPORT_API* pAPI;
        IP_MQTT_CLIENT_SOCKET         Socket;  // Socket used for the connection to the MQTT broker
} IP_MQTT_CLIENT_CONNECTION_CONTEXT;


typedef struct {
  U16   (*pfGenRandom)               (void);
  void* (*pfAlloc)                   (U32 NumBytesReq);
  void  (*pfFree)                    (void* p);
  void  (*pfLock)                    (void);
  void  (*pfUnlock)                  (void);
  int   (*pfRecvMessage)             (void* pMQTTClient, void* pPublish, int NumBytesRem);       // Get the subscribed message
  int   (*pfOnMessageConfirm)        (void* pMQTTClient, U8 Type, U16 PacketId);                 // Inform the application that a message has been processed
  int   (*pfHandleError)             (void* pMQTTClient);                                        // Inform the application that the publishing of a message failed.
} IP_MQTT_CLIENT_APP_API;


/*********************************************************************
*
*       MQTT_DLIST
*
*  Doubly linked lists. Allows fast add/remove of items and uses
*  inlining the code to speed up release builds. Typical use case
*  is a packet FIFO that is under constant change.
*/
typedef struct IP_MQTT_DLIST_ITEM_STRUCT IP_MQTT_DLIST_ITEM;
struct IP_MQTT_DLIST_ITEM_STRUCT {
  IP_MQTT_DLIST_ITEM* pNext;
  IP_MQTT_DLIST_ITEM* pPrev;
  unsigned            MessageId;
};


typedef struct {
  IP_MQTT_DLIST_ITEM* pFirst;
  int                 NumItems;
#if DEBUG != 0
  int            MaxItems;
#endif
} IP_MQTT_DLIST_HEAD;



//
// MQTT Message structure
//
typedef struct {
  IP_MQTT_DLIST_ITEM      Link;
  const char*             sTopic;
  const char*             pData;
  U32                     DataLen;
  U16                     PacketId;
  U8                      QoS;
  U8                      Retain;
  U8                      Duplicate;
} IP_MQTT_CLIENT_MESSAGE;


//
// Topic Filter
//
typedef struct {
  char* sTopicFilter;
  U16   Length;
  U8    QoS;
} IP_MQTT_CLIENT_TOPIC_FILTER;


//
// SUBSCRIBE packet
//
typedef struct {
  IP_MQTT_DLIST_ITEM           Link;
  IP_MQTT_CLIENT_TOPIC_FILTER* pTopicFilter;
  int                          TopicCnt;
  U16                          PacketId;
  U8                           ReturnCode;
} IP_MQTT_CLIENT_SUBSCRIBE;


typedef struct IP_MQTT_CLIENT_CONTEXT_STRUCT IP_MQTT_CLIENT_CONTEXT;
struct IP_MQTT_CLIENT_CONTEXT_STRUCT {
        IP_MQTT_CLIENT_CONTEXT*           pNext;             // Link element for internal management.
        IP_MQTT_DLIST_HEAD                Head;
  const IP_MQTT_CLIENT_APP_API*           pAppApi;
  const char*                             sId;               // Client Id.
  const char*                             sUser;
  const char*                             sPass;
        IP_MQTT_CLIENT_MESSAGE*           pLastWill;
        char*                             pBuffer;           // Buffer of the MQTT client.
        IP_MQTT_CLIENT_CONNECTION_CONTEXT Connection;        // API which should be used.
        U32                               BufferSize;        // Buffer size.
        U32                               ReceivedNumBytes;  // Number of bytes of the received payload.
        unsigned                          State;             // State of the MQTT connection.
        U16                               ReceivedID;        // Packet ID of the received packet.
        U16                               KeepAlive;
        U16                               aQOS2Pending[IP_MQTT_CLIENT_RECEIVED_ID_CACHE];
        U8                                ReceivedQOS;       // QoS of the received packet.
};


/*********************************************************************
*
*       API functions / Function prototypes
*
**********************************************************************
*/

int  IP_MQTT_CLIENT_Init               (IP_MQTT_CLIENT_CONTEXT* pClient, char* pBuffer, U32 BufferSize, const IP_MQTT_CLIENT_TRANSPORT_API* pAPI, const IP_MQTT_CLIENT_APP_API* pAppAPI, const char* sId);
int  IP_MQTT_CLIENT_SetLastWill        (IP_MQTT_CLIENT_CONTEXT* pClient, IP_MQTT_CLIENT_MESSAGE* pLastWill);
int  IP_MQTT_CLIENT_SetUserPass        (IP_MQTT_CLIENT_CONTEXT* pClient, const char* sUser, const char* sPass);
int  IP_MQTT_CLIENT_SetKeepAlive       (IP_MQTT_CLIENT_CONTEXT* pClient, U16 KeepAlive);
void IP_MQTT_CLIENT_Timer              (void);

int  IP_MQTT_CLIENT_Connect            (IP_MQTT_CLIENT_CONTEXT* pClient, const char* sAddr, U16 Port, U8 CleanSession);
int  IP_MQTT_CLIENT_Disconnect         (IP_MQTT_CLIENT_CONTEXT* pClient);
int  IP_MQTT_CLIENT_Publish            (IP_MQTT_CLIENT_CONTEXT* pClient, IP_MQTT_CLIENT_MESSAGE* pPublish);
int  IP_MQTT_CLIENT_Subscribe          (IP_MQTT_CLIENT_CONTEXT* pClient, IP_MQTT_CLIENT_SUBSCRIBE* pSubscribe);
int  IP_MQTT_CLIENT_Unsubscribe        (IP_MQTT_CLIENT_CONTEXT* pClient, IP_MQTT_CLIENT_SUBSCRIBE* pUnsubscribe);
int  IP_MQTT_CLIENT_WaitForNextMessage (IP_MQTT_CLIENT_CONTEXT* pClient, U32* pType, U32* pNumBytesRecv, char* pBuffer, U32 BufferSize);
int  IP_MQTT_CLIENT_Recv               (IP_MQTT_CLIENT_CONTEXT* pClient, char* pBuffer, U32 BufferSize);

int  IP_MQTT_CLIENT_Exec               (IP_MQTT_CLIENT_CONTEXT* pClient);
int  IP_MQTT_CLIENT_ParsePublish       (IP_MQTT_CLIENT_MESSAGE* pPublish, char* pBuffer, int NumBytes, char** ppTopic, int* pNumBytesTopic, char** ppPayload, int* pNumBytesPayload);
int  IP_MQTT_CLIENT_IsClientConnected  (const IP_MQTT_CLIENT_CONTEXT* pClient);

#if defined(__cplusplus)
}                             // Make sure we have C-declarations in C++ programs.
#endif

#endif                        // Avoid multiple inclusion.

/*************************** End of file ****************************/
