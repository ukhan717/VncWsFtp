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
File        : IP_Int.h
Purpose     : Internals used accross different layers of the TCP/IP stack
---------------------------END-OF-HEADER------------------------------
*/

#ifndef IP_INT_H               // Avoid multiple/recursive inclusion
#define IP_INT_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "IP.h"

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

#ifdef IPCORE_C
  #define EXTERN
#else
  #define EXTERN extern
#endif

#define IP_VERSION2STRING_HELPER(x) #x  //lint !e9024 hash in macro.
#define IP_VERSION2STRING(x) IP_VERSION2STRING_HELPER(x)

#define IPV4_HEADER_LEN              (20u)   // The IPv4 header is 20 bytes.
#define IPV6_HEADER_LEN              (40u)   // The IPv6 header (without any option) is 40 bytes.
#define TCP_HEADER_LEN               (20u)
#define UDP_HEADER_LEN                (8u)
#define ETH_MAC_ADDR_LEN              (6u)

#define IP_DISABLE_INT                (0u)
#define IP_ENABLE_INT                 (1u)

/*********************************************************************
*
*       Internal types and macros
*
*  The following types and macros are used internally to reduce
*  warnings. These warnings can be LINT/MISRA based or architecture
*  based. Best example for an architecture based warning are pointers
*  in 64-bit systems not having U32 addresses, causing a compiler
*  warning when being casted.
*/
#define IP_PIFACE(IFaceId)  (/*lint -e(9016)*/(IP_Global.paIFace + IFaceId))  // Allow pointer arithmetic in this case.

/*********************************************************************
*
*       Util macros internal
*/
#define  IP_MIN(x,y)    (((x)  <  (y))   ?  (x)   :  (y))
#define  IP_MAX(x,y)    (((x)  >  (y))   ?  (x)   :  (y))
#define  IP_COUNTOF(a)  (sizeof(a)/sizeof(a[0]))

#define  IP_IS_V6(x)    (((x & 0x60) == 0x60) ? 1 : 0)
#define  IP_NOT_V6(x)   (((x & 0x60) != 0x60) ? 1 : 0)

/*********************************************************************
*
*       IP_FIFO
*/
typedef struct IP_FIFO_ITEM {
   struct IP_FIFO_ITEM * pNext;
} IP_FIFO_ITEM;

typedef struct {
  IP_FIFO_ITEM * volatile pFirst;
  IP_FIFO_ITEM * volatile pLast;
#if ((IP_DEBUG_FIFO != 0) || (IP_SUPPORT_STATS_FIFO != 0) || (IP_SUPPORT_STATS_IFACE != 0))
  unsigned       Cnt;
#if ((IP_DEBUG_FIFO != 0) || (IP_SUPPORT_STATS_FIFO != 0))
  unsigned       Max;
  unsigned       Min;
#endif
#endif
} IP_FIFO;

void           IP_FIFO_Add                (IP_FIFO * pFifo, IP_FIFO_ITEM * pNewItem);
void           IP_FIFO_Add_NoLock         (IP_FIFO * pFifo, IP_FIFO_ITEM * pNewItem);
IP_FIFO_ITEM * IP_FIFO_GetLeave           (IP_FIFO * pFifo);
IP_FIFO_ITEM * IP_FIFO_GetRemove          (IP_FIFO * pFifo);
IP_FIFO_ITEM * IP_FIFO_GetRemove_NoLock   (IP_FIFO * pFifo);
IP_FIFO_ITEM * IP_FIFO_TryGetRemove_NoLock(IP_FIFO * pFifo);
IP_FIFO_ITEM * IP_FIFO_TryGetRemove       (IP_FIFO * pFifo);

//
//      MACROs for higher performance
//
#if (IP_DEBUG != 0) || (IP_PTR_OP_IS_ATOMIC == 0)
  #define IP_FIFO_GET_LEAVE(pFifo) IP_FIFO_GetLeave(pFifo)
#else
  #define IP_FIFO_GET_LEAVE(pFifo) (pFifo)->pFirst
#endif


/*********************************************************************
*
*       IP_DLIST
*
*  Doubly linked lists. Allows fast add/remove of items and uses
*  inlining the code to speed up release builds. Typical use case
*  is a packet FIFO that is under constant change.
*/
typedef struct {
  IP_DLIST_ITEM* pFirst;
  int            NumItems;
#if IP_DEBUG != 0
  int            MaxItems;
#endif
} IP_DLIST_HEAD;

void IP_DLIST_Add   (IP_DLIST_HEAD* pHead, IP_DLIST_ITEM* pNew);
int  IP_DLIST_IsItem(IP_DLIST_HEAD* pHead, IP_DLIST_ITEM* pItem);
void IP_DLIST_Remove(IP_DLIST_HEAD* pHead, IP_DLIST_ITEM* pItem);

//
// NEC CC78K0R has a problem with (IP_DLIST_ITEM*)->pPrev->pNext
// actually the pPrev fails which is stupid as the pPrev already
// is a IP_LIST_ITEM* and pNext is actually the same type where
// pPrev works.
// Using ifndef we allow to override the C-version to be used
// from IP_Conf.h .
//
#if (IP_DEBUG == 0)
#ifndef IP_DLIST_REMOVE
#define IP_DLIST_REMOVE(pHead, pItem)                 \
  {                                                   \
    if ((pItem) == (pHead)->pFirst) {                 \
      (pHead)->pFirst = (pItem)->pNext;               \
    } else {                                          \
      (pItem)->pPrev->pNext = (pItem)->pNext;         \
    }                                                 \
    if ((pItem)->pNext != NULL) {                     \
      (pItem)->pNext->pPrev = (pItem)->pPrev;         \
    }                                                 \
  }
#endif
#ifndef IP_DLIST_ADD
#define IP_DLIST_ADD(pHead, pNew)                     \
  {                                                   \
    (pNew)->pPrev = NULL;                             \
    (pNew)->pNext = (pHead)->pFirst;                  \
    if ((pHead)->pFirst != NULL) {                    \
      (pHead)->pFirst->pPrev = (pNew);                \
    }                                                 \
    (pHead)->pFirst = (pNew);                         \
  }
#endif
#else
#ifndef   IP_DLIST_REMOVE
  #define IP_DLIST_REMOVE(pHead, pItem)  IP_DLIST_Remove(pHead, pItem)
#endif
#ifndef   IP_DLIST_ADD
  #define IP_DLIST_ADD(pHead, pItem)     IP_DLIST_Add   (pHead, pItem)
#endif
#endif

/*********************************************************************
*
*       IP_STAT
*/
typedef struct {
  U32 TxPacketCnt;            // Number of packets sent
  U32 RxPacketCnt;            // Number of packets received
  U32 RxPacketCntIP4;         // Number of IP4 packets received
  U32 RxPacketCntIP4Valid;    // Number of valid IP4 packets received
  U32 ETH_RxCnt;              // Number of packets receive interrupts
} IP_STAT;

/*********************************************************************
*
*       IP_STATS
*/
#if IP_SUPPORT_STATS_IFACE
  #define IP_STATS_IFACE_INC(pStats, Counter)        { if (pStats != NULL) { pStats->Counter ++; } }
  #define IP_STATS_IFACE_INCVAL(pStats, Counter, v)  { if (pStats != NULL) { pStats->Counter += v; } }
  #define IP_STATS_IFACE_SET(pStats, Counter, v)     { if (pStats != NULL) { pStats->Counter  = v; } }
#else
  #define IP_STATS_IFACE_INC(pStats, Counter)
  #define IP_STATS_IFACE_INCVAL(pStats, Counter, v)
  #define IP_STATS_IFACE_SET(pStats, Counter, v)
#endif

/*********************************************************************
*
*       IP_BSP
*/
void IP_BSP_DeInit    (unsigned IFaceId);
void IP_BSP_Init      (unsigned IFaceId);
void IP_BSP_InstallISR(unsigned IFaceId, BSP_IP_INSTALL_ISR_PARA* pPara);

/*********************************************************************
*
*       Interface related structures, functions
*/
typedef struct {
  //
  // IPv4 related functions
  //
  int  (*pfSendBroadcast) (IP_PACKET * pPacket, U16 Type);
  int  (*pfSendUnicast)   (IP_PACKET * pPacket, U32 DestIP);
  int  (*pfSendMulticast) (IP_PACKET * pPacket, U32 DestIP);
  void (*pfOnRx)          (IP_PACKET * pPacket);
#if IP_IFACE_REROUTE
  int  (*pfSendPacket)    (IP_PACKET * pPacket, U16 Type, const U8 * pDestHWAddr);  // Typically only needed for VLAN as we need to exchange the protocol in the ethernet header before sending
#endif
#if IP_SUPPORT_IPV6
  //
  // IPv6 related functions
  //
//  int  (*pfIPv6SendAnycast)   (IP_PACKET * pPacket, U16 Type);
  int  (*pfIPv6SendUnicast)   (IP_PACKET * pPacket, U8 * pDestIP);
  int  (*pfIPv6SendMulticast) (IP_PACKET * pPacket, U8 * pDestIP);
#endif
#if IP_SUPPORT_STATS_IFACE
  int  (*pfIsUnicast)         (IP_PACKET * pPacket);
#endif
#if IP_SUPPORT_MICREL_TAIL_TAGGING
  U8 PreventLinkUpdateFromParent;
#endif
} IFACE_TYPE;

typedef struct IP_HOOK_ON_IP_RX {
  void   (*pf)(IP_PACKET *pPacket);        // Pointer to the function to be called from the hook.
  struct IP_HOOK_ON_IP_RX *pNext;          // Pointer to the next hook function.
} IP_HOOK_ON_IP_RX;

typedef struct IFace {
  const char*         sName;               // Short base name for interface like "ETH" or "WIFI".
  int (*pfConnect)   (unsigned IFaceId);
  int (*pfDisconnect)(unsigned IFaceId);
  struct IFace*       pHWIFace;            // Hardware interface used to send packets. Typically NULL for "real" ethernet interfaces; used for PPPoE
  void*               pContext;            // Optional context for pseudo-interfaces such as PPPoE
  const IFACE_TYPE*   pIFaceType;
  const IP_HW_DRIVER* pDriver;
  IP_HOOK_ON_IP_RX*   pOnIPRxFirstHook;
  const BSP_IP_API*   pBSP_API;
#if IP_SUPPORT_MICREL_TAIL_TAGGING
  int (*pfTailTaggingOnRx)(IP_PACKET* pPacket);
#endif
#if IP_SUPPORT_STATS_IFACE
  IP_STATS_IFACE*     pStats;
#endif
#if IP_SUPPORT_PTP
  const IP_HW_PTP_DRIVER* pPTPDriver;          // Functions array for PTP.
#endif
  IP_ADDR             IpAddr;
  IP_ADDR             SubnetMask;
  IP_ADDR             DefGateway;
  IP_ADDR             aDNSServer[IP_MAX_DNS_SERVERS];
  IP_FIFO             TxPacketFifo;
  //
  // IPv6 related elements
  //
  void*               pIPv6Desc;
  //
  // Other elements
  //
  U32*                paGroupAddr;
  U32                 LinkSpeed;           // Current Link Speed. Ethernet drivers expect a value of either 10, 100 or 1000MHz.
  U32                 LinkSpeedEx;         // "Real" current link speed. For example when using a LAN to WiFi bridge PHY, the
                                           // EMAC might only expect values in the range of LinkSpeed (code knows 10/100MHz),
                                           // the real speed however might be different like 54MHz for WiFi. Directly set by
                                           // whoever means he knows the "real" speed. 0 means not used.
  U32                 LinkDuplex;          // Current duplex state
  I32                 RxIntDisabledUntil;  // Timestamp until which interrupts for an interface shall be disabled. Typically used for anti-flood countermeasures. 0 means allow interrupts.
  U16                 Caps;                // Capabilities of the driver. Tells us if the driver is capable of computing IP, TCP checksums etc.
  U16                 CapsForcedMask;      // Mask for forced capabilities of the driver.
  U16                 CapsForcedValue;     // Value for forced capabilities of the driver. ForcedMask = 3, ForcedValue = 1 means: Bit 0: Forced to 1, Bit 1: Forced to 0, All other unaffected
  U16                 HeaderSize;          // Header of the local network. Typically 16 (=14 + 2 padding) for Ethernet, 20 for Ethernet with VLAN
  U16                 Mtu;                 // Maximum transmission unit
  U8                  abHWAddr[6];         // Hardware address. Typically used to hold the 6-byte Ethernet addr.
  U8                  AdminState;
  U8                  HWState;
  U8                  HasError;
  U8                  Unit;
  U8                  PollFromISREnabled;  // Does this interface need polling due to missing interrupt ?
#if IP_NUM_LINK_UP_PROBES
  U8                  LinkUpProbeCnt;
  U8                  LinkUpProbeCntReload;
#endif
#if IP_NUM_LINK_DOWN_PROBES
  U8                  LinkDownProbeCnt;
  U8                  LinkDownProbeCntReload;
#endif
  U8                  RxIntDisabled;
  U8                  InitDone;
} IFACE;

#define IP_INIT_LATER  110                 // Define to prevent a magic number to be used as return value from a driver init.
                                           // Return IP_LATE_INIT to let the stack know the stack shall late init this driver.

int    IP_AddInterface          (const char* sName, const IP_HW_DRIVER* pDriver, unsigned Mtu, unsigned HeaderSize, const IFACE_TYPE* pIFType);
IFACE* IP_GetIFaceForHost       (U16 ProtoFlags, U32 FHost);
IFACE* IP_GetIFaceForBroadcast  (U16 ProtoFlags);
void   IP_InitFromIFace         (IFACE *pIFaceDest, IFACE *pIFaceSrc);

void IP_NI_ConfigUseTxHwChecksum(unsigned IFaceId, int OnOff);
void IP_NI_EnDisableRxInt       (IFACE* pIFace, const IP_HW_DRIVER* pDriver, unsigned OnOff);
int  IP_NI_GetCaps              (unsigned IFaceId);
int  IP_NI_LoadHWAddr           (unsigned IFaceId);
#if 0  //OO: Could be called from drivers on re-init. Easier to implement if we know we are in ISR for the link change notify.
void IP_NI_OnReInit             (unsigned IFaceId);
#endif
void IP_NI_SetAdminState_NoLock (unsigned IFaceId, int AdminState);
void IP_NI_SetError             (unsigned IFaceId);
void IP_NI_SetLinkChangeCallback(void (*pfOnLinkChange)(unsigned IFaceId, U32 Duplex, U32 Speed));
int  IP_NI_SetLinkState         (unsigned IFaceId, U32 Duplex, U32 Speed);
void IP_NI_SetNoInit            (unsigned IFaceId);
void IP_NI_SignalLateInitReady  (void);

/*********************************************************************
*
*       IP hook
*/
typedef struct IP_HOOK_ON_IP_CHANGE {
  void   (*pf)(unsigned IFace, U32 Addr, U32 Mask);                // Pointer to the function to be called from the hook
  struct IP_HOOK_ON_IP_CHANGE *pNext;                              // Pointer to the next hook function
} IP_HOOK_ON_IP_CHANGE;

typedef struct IP_HOOK_ON_HW_LINK_CHANGE {
  void   (*pf)(IFACE* pIFace, U32 Duplex, U32 Speed, U8 HWState);  // Pointer to the function to be called from the hook
  struct IP_HOOK_ON_HW_LINK_CHANGE* pNext;                            // Pointer to the next hook function
} IP_HOOK_ON_HW_LINK_CHANGE;

/*********************************************************************
*
*       API functions (core/internal)
*/
void IP_AddIPChangeHook          (IP_HOOK_ON_IP_CHANGE *pHook, void (*pf)(unsigned IFace, U32 Addr, U32 Mask));
void IP_AddHwLinkChangeHook      (IP_HOOK_ON_HW_LINK_CHANGE* pHook, void (*pf)(IFACE* pHWIFace, U32 Duplex, U32 Speed, U8 HWState));
void IP_AddEthRxFifoHook         (IP_HOOK_ON_RX_FIFO_ADD* pHook, int (*pf)(IP_PACKET* pPacket, int AllowNoLock, int InInt));
void IP_AddRxFifoHook            (IP_HOOK_ON_RX_FIFO_ADD* pHook, int (*pf)(IP_PACKET* pPacket, int AllowNoLock, int InInt));
void IP_AddStateChangeHook_NoLock(IP_HOOK_ON_STATE_CHANGE* pHook, void (*pf)(unsigned IFaceId, U8 AdminState, U8 HWState));
int  IP_Add2RxFIFO               (IP_PACKET* pPacket, int AllowNoLock, int InInt);
U32  IP_CalcTimeLeft             (I32 Time);
int  IP_FindFirstIFaceByDriver   (const IP_HW_DRIVER* pDriver);
U32  IP_GetCoreClock             (void);
U32  IP_GetFreePacketCnt_NoLock  (U32 NumBytes);
U32  IP_GetRandU32               (void);
int  IP_IsIPv6Enabled            (void);
int  IP_IsAllFF                  (const U8* p, unsigned NumBytes);
int  IP_IsRxTaskRunning          (void);
int  IP_ReceivePacket            (int IFaceId, void *pSrc, U32 NumBytes);
void IP_RemoveEthRxFifoHook      (const IP_HOOK_ON_RX_FIFO_ADD* pHook);
void IP_ResetIFace               (U8 IFace);
void IP_SetRxProt                (unsigned ProtIndex, IP_ON_RX_FUNC* pf, int AllowReplace);
int  IP_StoreRxPacket            (unsigned IFaceId, IP_PACKET* pPacket, U32 NumBytes);
void IP4_AddOnIPRxHook           (IFACE *pIFace, IP_HOOK_ON_IP_RX *pHook, void (*pf)(IP_PACKET *pPacket));

//
// Compatibility macros for old drivers.
// New drivers should call IP_NI_SetLinkState() directly.
//
#define IP_SetCurrentLinkState(Duplex, Speed)  IP_NI_SetLinkState(0, Duplex, Speed)
#define IP_SetCurrentLinkStateEx               IP_NI_SetLinkState

/*********************************************************************
*
*       IP_PACKET_
*/
void       IP_PACKET_AddOnPacketFreeHook(IP_HOOK_ON_PACKET_FREE *pHook, void (*pf)(IP_PACKET* pPacket));
IP_PACKET* IP_PACKET_Alloc                  (unsigned NumBytes, int NumPacketsReserved);
IP_PACKET* IP_PACKET_Alloc_NoLock           (unsigned NumBytes, int NumPacketsReserved);
void       IP_PACKET_Free                   (IP_PACKET* pPacket);
void       IP_PACKET_FreeSignal             (IP_PACKET* pPacket);
void       IP_PACKET_Free_NoLock            (IP_PACKET* pPacket);
void       IP_PACKET_FreeSignal_NoLock      (IP_PACKET* pPacket);
void       IP_PACKET_IncUseCnt              (IP_PACKET* pPacket);
int        IP_PACKET_Init                   (void);
#if IP_SUPPORT_ON_PACKET_FREE_CB
void       IP_PACKET_SetOnPacketFreeCallback(IP_PACKET* pPacket, void (*pfOnFreeCB)(IP_PACKET* pPacketCB, void* pContextCB), void* pContext);
#endif

/*********************************************************************
*
*       Packet types
*/
#define ETH_TYPE_ARP   htons(0x0806u)  // Ethernet type "ARP"
#define ETH_TYPE_IP    htons(0x0800u)  // Ethernet type "IP"
#define ETH_TYPE_IPV6  htons(0x86DDu)  // Ethernet type "IPv6"
#define ETH_TYPE_VLAN  htons(0x8100u)  // Ethernet type "VLAN"

/*********************************************************************
*
*       IP_PROT_
*/
#define IP_PROT_ICMP        1         // ICMP protocol number
#define IP_PROT_IGMP        2         // IGMP protocol number
#define IP_PROT_TCP         6         // TCP protocol number
#define IP_PROT_UDP        17         // UDP protocol number
#define IP_PROT_ICMPV6     58         // ICMPv6 protocol number
#define IP_PROT_MAX        17         // Used as define the maximum array size of IP_Global.apfOnRxProt

/*********************************************************************
*
*       Ethernet related macros
*/
#define  IP_SPEED_UNKNOWN                         0
#define  IP_SPEED_10MHZ                           10000000
#define  IP_SPEED_100MHZ                          100000000
#define  IP_SPEED_1GHZ                            1000000000

#define  IP_DUPLEX_UNKNOWN                        0       // Duplex unknown or auto-neg incomplete
#define  IP_DUPLEX_HALF                           1       // Duplex = Half duplex
#define  IP_DUPLEX_FULL                           2       // Duplex = Full duplex

void    IP_ETH_OnRx(void);      // ETH Receive routine, called by driver
void    IPv4_TCP_OnRx (IP_PACKET * pPacket);
int     IP__SendPacket(IP_PACKET * pPacket);

void    IPvX_TCP_OnRx (IP_PACKET * pPacket);

//
// Interface related
//
int     IP_IsBroadcast   (IFACE *pIFace, U32 IPAddr);
IFACE * IP_FAddr2IFace(U32 FHost);
IFACE * IP_LAddr2IFace(U32 LHost);
int     IP_IsMulticastAddr(U32 IPAddr);


/*********************************************************************
*
*       MBLOCK
*/
typedef struct MBLOCK {
  struct MBLOCK *pNext;      // Used for singly-linked list. Every block is member in either a Free-Fifo, send, receive or sector-buffer
  IP_PACKET     *pPacket;
  U8            *pData;
  U16            NumBytes;
} MBLOCK;


MBLOCK * IP_MBLOCK_Get       (int Len);
MBLOCK * IP_MBLOCK_GetCopy   (void *pData, int NumBytes);
MBLOCK * IP_MBLOCK_GetEmpty  (void);
void     IP_MBLOCK_Init      (int NumBlocks);
void     IP_MBLOCK_FreeChain (MBLOCK *pMBlock);
MBLOCK * IP_MBLOCK_FreeSingle(MBLOCK *pMBlock);


/*********************************************************************
*
*       Memory management related structures
*/
typedef struct FREE_LIST {
  U32 NumBytes;                   // Number of free bytes including this structure
  struct FREE_LIST* pNext;
} FREE_LIST;

typedef struct MEM_POOL_LIST {          // MEM_POOL_LIST shall always be 8 bytes aligned.
  FREE_LIST*            pPosInPool;     // Current position in memory block
  struct MEM_POOL_LIST* pNextPool;      // Base address of the following memory block
  U32*                  pBaseAddr;      // Base address of the memory block
  U32*                  pLastAddr;      // Address of the last byte of the memory block
} MEM_POOL_LIST;

/*********************************************************************
*
*       IP_MEM
*/
#if IP_DEBUG_MEM
void* IP_MEM_Alloc                     (const char *sId, U32 NumBytesReq);
void* IP_MEM_AllocEx                   (const char* sId, void* pBaseAddr, U32 NumBytesReq);
void* IP_MEM_AllocTransferMemory       (const char *sId, U32 NumBytesReq, int ShiftCnt);
void* IP_MEM_AllocTransferMemoryPhys   (const char* sId, U32 NumBytesReq, int ShiftCnt);
void* IP_MEM_AllocTransferMemoryZeroed (const char *sId, U32 NumBytesReq, int ShiftCnt);
void* IP_MEM_AllocZeroed               (const char *sId, U32 NumBytesReq);
void* IP_MEM_AllocZeroedEx             (const char *sId, void* pBaseAddr, U32 NumBytesReq);
#else
void* IP_MEM_Alloc                     (U32 NumBytesReq);
void* IP_MEM_AllocEx                   (void* pBaseAddr, U32 NumBytesReq);
void* IP_MEM_AllocTransferMemory       (U32 NumBytesReq, int ShiftCnt);
void* IP_MEM_AllocTransferMemoryPhys   (U32 NumBytesReq, int ShiftCnt);
void* IP_MEM_AllocTransferMemoryZeroed (U32 NumBytesReq, int ShiftCnt);
void* IP_MEM_AllocZeroed               (U32 NumBytesReq);
void* IP_MEM_AllocZeroedEx             (void* pBaseAddr, U32 NumBytesReq);
#endif
void  IP_MEM_Free                      (void  *p);
void  IP_MEM_FreeEx                    (U32* pBaseAddr, void* p);
void  IP_MEM_FreePtr                   (void **pp);
U32   IP_MEM_GetMaxBlockSize           (void);
#if IP_DEBUG_MEM
U32   IP_MEM_GetTotalNumBytes          (void);
#endif
void  IP_MEM_Add                       (U32  *p, U32 NumBytes, char IsFirstPool);
void  IP_MEM_Shrink                    (void *p, int NewSize);

#if IP_DEBUG_MEM
  #define IP_MEM_ALLOC(Id, NumBytes)                                   IP_MEM_Alloc(Id, NumBytes)
  #define IP_MEM_ALLOC_EX(Id, pBaseAddr, NumBytes)                     IP_MEM_AllocEx(Id, pBaseAddr, NumBytes)
  #define IP_MEM_ALLOC_ZEROED(Id, NumBytes)                            IP_MEM_AllocZeroed(Id, NumBytes)
  #define IP_MEM_ALLOC_ZEROED_EX(Id, pBaseAddr, NumBytes)              IP_MEM_AllocZeroedEx(Id, pBaseAddr, NumBytes)
  #define IP_MEM_ALLOC_TRANSFER_MEMORY(Id, NumBytes, ShiftCnt)         IP_MEM_AllocTransferMemory(Id, NumBytes, ShiftCnt)
  #define IP_MEM_ALLOC_TRANSFER_MEMORY_PHYS(Id, NumBytes, ShiftCnt)    IP_MEM_AllocTransferMemoryPhys(Id, NumBytes, ShiftCnt)
  #define IP_MEM_ALLOC_TRANSFER_MEMORY_ZEROED(Id, NumBytes, ShiftCnt)  IP_MEM_AllocTransferMemoryZeroed(Id, NumBytes, ShiftCnt)
#else
  #define IP_MEM_ALLOC(Id, NumBytes)                                   IP_MEM_Alloc(NumBytes)
  #define IP_MEM_ALLOC_EX(Id, pBaseAddr, NumBytes)                     IP_MEM_AllocEx(pBaseAddr, NumBytes)
  #define IP_MEM_ALLOC_ZEROED(Id, NumBytes)                            IP_MEM_AllocZeroed(NumBytes)
  #define IP_MEM_ALLOC_ZEROED_EX(Id, pBaseAddr, NumBytes)              IP_MEM_AllocZeroedEx(pBaseAddr, NumBytes)
  #define IP_MEM_ALLOC_TRANSFER_MEMORY(Id, NumBytes, ShiftCnt)         IP_MEM_AllocTransferMemory(NumBytes, ShiftCnt)
  #define IP_MEM_ALLOC_TRANSFER_MEMORY_PHYS(Id, NumBytes, ShiftCnt)    IP_MEM_AllocTransferMemoryPhys(NumBytes, ShiftCnt)
  #define IP_MEM_ALLOC_TRANSFER_MEMORY_ZEROED(Id, NumBytes, ShiftCnt)  IP_MEM_AllocTransferMemoryZeroed(NumBytes, ShiftCnt)
#endif

/*********************************************************************
*
*       Socket internals
*/
#define  SOCKET_WAIT_FLAG_TASK        (1 << 1)     // One or more tasks are waiting for this socket buffer
#define  SOCKET_WAIT_FLAG_SELECT      (1 << 2)     // Socket-buffer  is used in a select operation

typedef struct {
  U8 PFamily;
  U8 HopLimit;
} SOCKET_CONTEXT;

typedef int (*PF_SO_CALLBACK)(int hSock, IP_PACKET* pPacket, int MsgCode);

typedef struct SOCKET_STRUCT SOCKET;

/*********************************************************************
*
*       IP_TRANSPORT_PROTO
*/
typedef struct {
  U16      pr_flags;
  U8       Type;            // SOCK_STREAM or SOCK_DGRAM
  U8       ProtoIndex;      // Protocol index as defined by RFC. TCP: 6, UDP: 17
  int      (*pfAbort)       (SOCKET* pSock);
  int      (*pfAccept)      (SOCKET* pSock, struct sockaddr* pSockAddr);
  int      (*pfAttach)      (SOCKET* pSock, int Proto, IFACE *pIFace);
  int      (*pfBind)        (SOCKET* pSock, struct sockaddr* pSockAddr);
  int      (*pfConnect)     (SOCKET* pSock, struct sockaddr* pSockAddr);
  void     (*pfDetach)      (SOCKET* pSock);
  int      (*pfDisconnect)  (SOCKET* pSock);
  int      (*pfGetPeerName) (SOCKET* pSock, struct sockaddr_in* pSockAddr);
  int      (*pfGetSockName) (SOCKET* pSock, struct sockaddr_in* pSockAddr);
  int      (*pfListen)      (SOCKET* pSock);
  int      (*pfReceive)     (SOCKET* pSock, struct sockaddr* pSockAddr, char* pDest, int NumBytes, int Flags);
  int      (*pfSend)        (SOCKET* pSock, const U8* pData, int NumBytes, const struct sockaddr* pSockAddr);
  int      (*pfShutdown)    (SOCKET* pSock);
} IP_TRANSPORT_PROTO;

typedef  const IP_TRANSPORT_PROTO * IP_TRANSPORT_PROTO_CP;

/*********************************************************************
*
*       SOCKET_BUFFER
*/
typedef struct {
   U32      NumBytes;
   U32      Limit;
   U32      Timeout;
   MBLOCK * pFirstMBlock;
} SOCKET_BUFFER;

void  IP_SOCKBUF_AddData (SOCKET_BUFFER * pSockBuf, MBLOCK * pMBlock);
int   IP_SOCKBUF_GetSpace(SOCKET_BUFFER * pSockBuf);
void  IP_SOCKBUF_Flush   (SOCKET_BUFFER * pSockBuf);

struct SOCKET_STRUCT {
  union {
    IP_DLIST_ITEM            Link;
    SOCKET*                  pSocket;
  } Next;
  const IP_TRANSPORT_PROTO*  pProt;
  void*                      pProtData;
  PF_SO_CALLBACK             pfCallback;
  U16                        Handle;
  U16                        Options;
  U16                        Linger;
  U16                        State;
  U32                        Timeout;
  I8                         Error;
  U8                         AppOwned;      // If true, then the socket is owned by the application and can only be freed by app., not the protocol.
  U8                         BackLog;       // Max. number of queued connections. For "parent" sockets only, which are used in listening state
  U8                         WaitFlags;
  SOCKET_BUFFER              SockBufRx;
  SOCKET_BUFFER              SockBufTx;
  SOCKET*                    pOwner;
  SOCKET_CONTEXT*            pSoCon;
  U8                         ToS;           // Type of Service (QoS) byte in IPv4 header.
};

/*********************************************************************
*
*       Socket states
*/
#define  SOCKET_IS_CONNECTED       0x0002
#define  SOCKET_IS_CONNECTING      0x0004
#define  SOCKET_IS_DISCONNECTING   0x0008
#define  SOCKET_CANT_SEND_MORE     0x0010
#define  SOCKET_CANT_RCV_MORE      0x0020
#define  SOCKET_NON_BLOCKING_IO    0x0100
#define  SOCKET_UPCALLED           0x0400
#define  SOCKET_IN_UPCALL          0x0800
#define  SOCKET_WAS_CONNECTING     0x2000

/*********************************************************************
*
*       Ethernet type related
*/
typedef struct {
  unsigned Type;
  void (*pfOnRx)(IP_PACKET* pPacket);
} IP_ETH_TYPE_CALLBACK;

typedef struct {
  void (*pfHandler)(void);
  I32  Period;
  I32  NextTime;
} IP_TIMER;

typedef struct IP_ON_EXIT_CB {
  struct IP_ON_EXIT_CB * pNext;
  void (*pfOnExit)(void);
} IP_ON_EXIT_CB;

#if IP_ALLOW_DEINIT
  void IP_AddOnExitHandler(IP_ON_EXIT_CB * pCB, void (*pfOnExit)(void));
#endif

typedef int IP_FILTER_FUNC  (unsigned IFaceId, U8 *paHWAddr, unsigned NumAddrBufs);

/*********************************************************************
*
*       Internal API
*/
void       IP_AddTimer      (void (*pfHandler)(void), I32 Period);
#if 0  //OO: Can be used together with IP_AddTimer() if IP_AddTimer() would return "IP_TIMER *". Implemented for experimental use.
void       IP_ExecTimer     (IP_TIMER* pTimer);
#endif
void       IP_TCP_DataUpcall(SOCKET* pSocket);

#define     IP_BROADCAST_ADDR     0xFFFFFFFFuL

void       IP_GetNextOutPacket    (void ** ppData, unsigned * pNumBytes);    // Obsolete, but still available (Use ...Fast-Version)
unsigned   IP_GetNextOutPacketFast(void ** ppData);
unsigned   IP_GetNextOutPacketEx  (void ** ppData, unsigned IFaceId);
void       IP_RemoveOutPacket(void);
void       IP_RemoveOutPacket_NoLock(void);
void       IP_RemoveOutPacketEx(unsigned IFaceId);
void       IP_RemoveOutPacketEx_NoLock(unsigned IFaceId);
IP_OPTIMIZE
U32 IP_cksum(void * ptr, unsigned NumHWords, U32 Sum);
U32 IP_CalcChecksum_Byte(const void * pData, unsigned NumBytes, U32 Sum);

/*********************************************************************
*
*       IP_SOCKET
*/
SOCKET* IP_SOCKET_Alloc        (U8 AddContext);
void    IP_SOCKET_Free         (SOCKET* pSock);
U8      IP_SOCKET_GetProtoIndex(SOCKET* pSock);
void    IP_SOCKET_SetRawAPI    (const IP_TRANSPORT_PROTO* pProto);

SOCKET* IP_SOCKET_h2p(int hSock);

/*********************************************************************
*
*       RAW
*/
typedef struct {
  IP_DLIST_HEAD List;                   // List of RAW4 connections
  U32           TxBufferSize;           // Number of bytes which can be buffered on Tx.
  U32           RxBufferSize;           // Number of bytes which can be buffered on Rx.
} IP_RAW_GLOBAL;

typedef void IP_ETH_RAW_CALLBACK (IP_PACKET* pPacket);

void               IP_RAW_Close_NoLock      (IP_RAW_CONNECTION* pCon);
IP_RAW_CONNECTION* IP_RAW_Open_NoLock       (IP_ADDR FAddr, IP_ADDR LAddr, U8 Protocol, int (*handler)(IP_PACKET* pPacket, void* pContext), void * pContext);
int                IP_RAW_SendAndFree_NoLock(IFACE * pIFace, IP_ADDR FHost, U8 Protocol, IP_PACKET * pPacket);

EXTERN IP_RAW_GLOBAL IP_RAW_Global;

/*********************************************************************
*
*       IGMP_
*/
typedef struct {
  int (*pfIsInGroup)(const IFACE *pIFace, IP_ADDR GroupIP);
} IP_IGMP_API;

extern const IP_IGMP_API IP_IGMP_Api;

//
// Filter functions.
//
//   Note: Filter functions are used to collect all required HW addresses
//   (e.g. multicast addresses) from the different modules.
//
typedef struct {
  IP_FILTER_FUNC *pIPv4IGMP;
#if IP_SUPPORT_IPV6
  IP_FILTER_FUNC *pIPv6MLD;
#endif
  IP_FILTER_FUNC *pCustFilter;
} IP_FILTER_FUNCTIONS;

int  IP_IGMP_AddEx_NoLock     (unsigned IFaceId);
void IP_IGMP_JoinGroup_NoLock (unsigned IFaceId, IP_ADDR GroupIP);
void IP_IGMP_LeaveGroup_NoLock(unsigned IFaceId, IP_ADDR GroupIP);

/*********************************************************************
*
*       IP_DNS_
*/
int IP_DNS_ResolveHostEx_NoLock(unsigned IFaceId, const IP_DNSSD_REQUEST* pRequest, int ms);

/*********************************************************************
*
*       IP_PROFILE
*/
typedef struct {
  void (*pfRecordEndCall)       (unsigned int EventId);
  void (*pfRecordEndCallU32)    (unsigned int EventId, U32 Para0);
  void (*pfRecordVoid)          (unsigned int EventId);
  void (*pfRecordU32)           (unsigned int EventId, U32 Para0);
  void (*pfRecordU32x2)         (unsigned int EventId, U32 Para0, U32 Para1);
  void (*pfRecordU32x3)         (unsigned int EventId, U32 Para0, U32 Para1, U32 Para2);
  void (*pfRecordU32x4)         (unsigned int EventId, U32 Para0, U32 Para1, U32 Para2, U32 Para3);
  void (*pfRecordU32x5)         (unsigned int EventId, U32 Para0, U32 Para1, U32 Para2, U32 Para3, U32 Para4);
  void (*pfRecordU32x6)         (unsigned int EventId, U32 Para0, U32 Para1, U32 Para2, U32 Para3, U32 Para4, U32 Para5);
  void (*pfRecordU32x7)         (unsigned int EventId, U32 Para0, U32 Para1, U32 Para2, U32 Para3, U32 Para4, U32 Para5, U32 Para6);
  void (*pfRecordU32x8)         (unsigned int EventId, U32 Para0, U32 Para1, U32 Para2, U32 Para3, U32 Para4, U32 Para5, U32 Para6, U32 Para7);
  void (*pfRecordU32x9)         (unsigned int EventId, U32 Para0, U32 Para1, U32 Para2, U32 Para3, U32 Para4, U32 Para5, U32 Para6, U32 Para7, U32 Para8);
  void (*pfRecordU32x10)        (unsigned int EventId, U32 Para0, U32 Para1, U32 Para2, U32 Para3, U32 Para4, U32 Para5, U32 Para6, U32 Para7, U32 Para8, U32 Para9);
  void (*pfRecordString)        (unsigned int EventId, const char* pPara0);
  void (*pfRecordStringU32)     (unsigned int EventId, const char* pPara0, U32 Para1);
  void (*pfRecordStringx2)      (unsigned int EventId, const char* pPara0, const char* pPara1);
  U32  (*pfPtr2Id)              (U32 Ptr);
  void (*pfRecordEthRxPacket)   (unsigned int EventId, U32 Para0, U32 Para1, U32 Para2, U32 Para3, U32 Para4, U8* pData0, U32 NumBytes0);
  void (*pfRecordEnDisableRxInt)(unsigned int EventId, U32 IFaceId, U32 OnOff);
} IP_PROFILE_API;

typedef struct {
  U32                   IdOffset;
  const IP_PROFILE_API* pAPI;
} IP_PROFILE;

U32  IP_PROFILE_GetAPIDesc(const char** psDesc);
void IP_PROFILE_SetAPI    (const IP_PROFILE_API* pAPI, U32 IdOffset);

/*********************************************************************
*
*       Defines, structs and typedefs required for fragmentation handling
*/

//
// IPv4 related defines, structs and typedefs
//
#if IP_SUPPORT_FRAGMENTATION_IPV4
#define IPV4_MORE_FRAGMENTS_MASK  (1uL << 13)
#define IPV4_DONT_FRAGMENT_MASK   (1uL << 14)

typedef IP_PACKET* (HANDLE_IPV4_FRAGMENT_FUNC) (IP_PACKET* pPacket);

typedef struct {
  IP_DLIST_HEAD              List;            // List of fragmented headers
  HANDLE_IPV4_FRAGMENT_FUNC* pfRxHandleFrag;
  U32                        DiscardedId;
  U32                        TimeOut;
  U8                         MaxFragments;    // By default 0xFF. 0xFF == no limitation of the number of packets.
  U8                         KeepOutOfOrderFragments;
} IP_FRAGMENT_HANDLER_IPV4;
#endif

//
// IPv6 related defines, structs and typedefs
//
#if IP_SUPPORT_FRAGMENTATION_IPV6
typedef IP_PACKET* (HANDLE_IPV6_FRAGMENT_FUNC) (IP_PACKET* pPacket, U32 Offset);

typedef struct {
  IP_DLIST_HEAD              List;             // List of fragmented headers
  HANDLE_IPV6_FRAGMENT_FUNC* pfRxHandleFrag;
  U32                        DiscardedId;
  U32                        TimeOut;
  U8                         MaxFragments;     // By default 0xFF. 0xFF == no limitation of the number of packets.
  U8                         KeepOutOfOrderFragments;
} IP_FRAGMENT_HANDLER_IPV6;
#endif

#if ((IP_SUPPORT_FRAGMENTATION_IPV4 != 0) || (IP_SUPPORT_FRAGMENTATION_IPV6 != 0))
typedef struct {
#if IP_SUPPORT_FRAGMENTATION_IPV4
  IP_FRAGMENT_HANDLER_IPV4* FragmentHandler_IPv4;
#endif
#if IP_SUPPORT_FRAGMENTATION_IPV6
  IP_FRAGMENT_HANDLER_IPV6* FragmentHandler_IPv6;
#endif
} IP_FRAGMENT_HANDLERS;
#endif

/*********************************************************************
*
*       Packet buffer configuration
*/
typedef struct {
  unsigned NumBuffers;  // Number of buffers in this pool.
  unsigned BufferSize;  // Size of each buffer in this pool.
} IP_BUFFER_CONFIG;

/*********************************************************************
*
*       IP_GLOBAL
*/
typedef struct {
  const char*                sCopyright;  // Used to reference and link in the copyright string for libraries.
  //
  // Management pointers
  //
        IFACE*               paIFace;
        IP_ON_RX_FUNC*       apfOnRxProt[IP_PROT_MAX + 1];  // On Rx functions for different protocols, such as UDP, TCP, ICMP .
        IP_ON_EXIT_CB*       pFirstOnExit;
        MEM_POOL_LIST*       pMemPoolList;
        IP_RX_HOOK*          pfOnRx;
        IP_ETH_RAW_CALLBACK* pRawCallback;
  const IP_IGMP_API*         pIGMP_API;
        IP_FILTER_FUNCTIONS  MACFilterFuncs;
  void (*pfOnPacketFree)(void);
  void (*pfGetRand)     (U8* pBuffer, unsigned NumBytes);
#if IP_SUPPORT_PROFILE
        IP_PROFILE           Profile;
#endif
#if ((IP_SUPPORT_FRAGMENTATION_IPV4 != 0) || (IP_SUPPORT_FRAGMENTATION_IPV6 != 0))
        IP_FRAGMENT_HANDLERS FragmentHandlers;
#endif
  //
  // Timer
  //
  unsigned    NumTimers;
  IP_TIMER    aTimer[IP_MAX_TIMERS];
  //
  // Buffers
  //
  IP_BUFFER_CONFIG aBufferConfig[2];
  unsigned         MaxBufferSize;   // Size of the largest type of buffers in the system.
  unsigned         AddBufferIndex;  // Next buffer index that will be used when adding buffers.
  //
  // FIFOs and packet management
  //
  IP_FIFO RxPacketFifo;        // Packets containing Rx data which have not yet been processed.
  IP_FIFO MBufFreeFifo;        // Free mbufs.
  IP_FIFO aFreePacketFifo[2];  // Packets which are not in use (free). Big: FTP etc.. Small: ARPs, TCP acks, pings etc.
  int     aFreePacketCnt[2];
  //
  // Socket management
  //
  struct {
    union {
      IP_DLIST_HEAD Head;
      SOCKET*       pFirstSock;                // For debugging purposes only.
    } List;
    IP_TRANSPORT_PROTO_CP apProto[2];
    U16                   NumRegisteredProtos;
    U16                   DefaultOptions;       // Default Options value for new sockets in the system. Can be set using IP_SOCKET_SetDefaultOptions() .
    U32                   NextHandle;
    U16                   Limit;
    U16                   Cnt;
  #if IP_DEBUG != 0
    U16                   Max;
  #endif
  } Socket;
  //
  // Time callback
  //
#if (defined(U64) && ((IP_SUPPORT_PACKET_TIMESTAMP != 0) || (IP_SUPPORT_PTP != 0)))
  U64 (*pfGetOSTime_us)(void);
#endif
#if (IP_SUPPORT_PTP != 0)
  U32                     CoreClock;            // CPU core clock set by the application and made available to other modules by an internal API.
#endif
  //
  // Others
  //
  I32 Cached2UncachedOff;
  int PrimaryIFaceId;      // ID of primary interface.
  U16 PacketId;
  U8  LocalMcTTL;
  U8  GlobalMcTTL;
  U8  TTL;
  U8  MaxIFaces;
  U8  NumIFaces;
  //
  // TODO (OO): Combine the following into one variable that works with flags.
  //
  U8  ConfigCompleted;
  U8  InitCompleted;
  U8  OnPacketFreeUsed;
  U8  DisableIPv4;         // Disable IPv4 functionality like sending IPv4 packets. The earlier this gets set, the more IPv4 features (like hooks) can be disabled.
  U8  CheckLateInit;       // At least one interface has returned a late init request during its call from IP_Init().
} IP_GLOBAL;

/*********************************************************************
*
*       Debug variables
*/
typedef struct {
  //
  //  Support for an option that allows us to deliberatly loose or slow down packets
  //  Variables are used in this module only, but also only in debug builds.
  //  They could be static, but this would lead to warnings in release builds so we leave them public.
  //
  int TxDropCnt;                 // Packets since last loss
  int TxDropRate;                // Number of packets to punt (3 is "1 in 3")
  int RxDropCnt;                 // Packets since last loss
  int RxDropRate;                // Number of packets to punt (3 is "1 in 3")
  int PacketDelay;
#if IP_DEBUG != 0
  IP_PACKET *pFirstPacketInUse;
  U32        PacketInUseCnt;
  U32        MaxPacketInUseCnt;
#endif
  U32 *        pMem;
  U32          NumBytes;         // Number of bytes assigned to memory area used as heap
} IP_DEBUG_VARS;

EXTERN IP_DEBUG_VARS  IP_Debug;
EXTERN IP_GLOBAL      IP_Global;
EXTERN IP_STAT        IP_Stat;

int  IPv4_Write(IP_PACKET * pPacket, U8 Protocol, U32 FHost);
void IPv4_OnRx (IP_PACKET * pPacket);  // ARP received packet upcall

int  IP_ETH_AddInterface    (const char* sName, const IP_HW_DRIVER* pDriver);
int  IP_ETH_Add2RxFIFO      (IP_PACKET* pPacket, int AllowNoLock, int InInt);
int  IP_ETH_HasHWAddr       (IFACE* pIFace);
int  IP_ETH_IsUnicast       (IP_PACKET* pPacket);
void IP_ETH_OnRxPacket      (IP_PACKET* pPacket);
void IP_ETH_RegisterType    (unsigned Type, void (*pfOnRx)(IP_PACKET *pPacket));
int  IP_ETH_SendPacket      (IP_PACKET* pPacket, U16 Type, const U8* pDestHWAddr);
void IP_ETH_SetCustMACFilter(IP_FILTER_FUNC *pf);
void IP_ETH_UpdateMACFilter (unsigned IFaceId);

/*********************************************************************
*
*       UDP
*/
typedef struct {
  U16 SrcPort;
  U16 DestPort;
  U16 NumBytes;
  U16 CheckSum;
} UDP_HEADER;

typedef struct {
  IP_DLIST_HEAD List;             // List of UDP connections.
  U32           TxBufferSize;     // Number of bytes which can be buffered on Tx.
  U32           RxBufferSize;     // Number of bytes which can be buffered on Rx.
  U16           NextFreePort;
  U8            RxChecksumEnable;
  U8            TxChecksumEnable;
} IP_UDP_GLOBAL;

typedef struct {
  U32  LAddr;
  U32  FAddr;
} IPV4_ADDR_INFO;

typedef struct {
  IPV6_ADDR  aLAddr;
  IPV6_ADDR  aFAddr;
} IPV6_ADDR_INFO;

typedef struct {
  void* pDummy;
} IP_ADDR_INFO_GEN;

//
// Union used as a wrapper for IPV4_ADDR_INFO / IPV6_ADDR_INFO
// The union improves the readability of our code.
//
typedef union {
  IPV4_ADDR_INFO*   pIPv4AddrInfo;
  IPV6_ADDR_INFO*   pIPv6AddrInfo;
  IP_ADDR_INFO_GEN* pIPAddrInfo;
} IP_ADDR_INFO;

//
// UDP generic functions.
//
void IP_UDP_AddGeneric         (const IP_TRANSPORT_PROTO* pProto, IP_ON_RX_FUNC* pf, int AllowReplace);
void IP_UDP_Close_NoLock       (IP_UDP_CONNECTION* pCon);
U16  IP_UDP_FindFreePort_NoLock(void);
void IP_UDP_SwapHeader         (UDP_HEADER* pUDPHeader);

//
// IPv4 related UDP functions.
//
void               IP_UDP_OnRxIP4           (IP_PACKET* pPacket);
IP_UDP_CONNECTION* IP_UDP_OpenEx_NoLock     (IP_ADDR FAddr, U16 FPort, IP_ADDR LAddr, U16 LPort, int (*handler)(IP_PACKET* pPacket, void* p), void* pContext);
int                IP_UDP_SendAndFree_NoLock(IFACE* pIFace, IP_ADDR FHost, U16 fport, U16 lport, IP_PACKET* pPacket);

//
// IPv6 related UDP functions.
//
int  IP_IPV6_UDP_SendAndFree_NoLock(IFACE * pIFace, IPV6_ADDR * pFHost, U16 FPort, U16 LPort, IP_PACKET * pPacket);
void IP_IPvX_UDP_OnRx              (IP_PACKET * pPacket);   // Rx funtions handles IPv4 and IPv6 packets.

EXTERN IP_UDP_GLOBAL IP_UDP_Global;

/*********************************************************************
*
*       IP_STAT
*/
typedef struct {
  U32 TxPacketCnt;                 // Number of packets sent
  U32 RxPacketCnt;                 // Number of packets received
  U32 RxPacketCntValid;            // Number of valid packets
  U32 RxPacketCntDispatched;       // Number of packets dispatched
  U32 RxPacketCntDispatchedToSock; // Number of packets dispatched to socket
} IP_UDP_STAT;

typedef struct {
  U32 TxPacketCnt;                 // Number of packets sent
  U32 RxPacketCnt;                 // Number of packets received
  U32 RxPacketCntDispatched;       // Number of packets dispatched
  U32 RxPacketCntDispatchedToSock; // Number of packets dispatched to socket
} IP_RAW_STAT;

EXTERN IP_UDP_STAT IP_UDP_Stat;
EXTERN IP_RAW_STAT IP_RAW_Stat;

/*********************************************************************
*
*       IP
*/
typedef struct {
  U8   VerLen;
  U8   TOS;
  U16  NumBytes;
  U16  Id;
  U16  FlagsOff;
  U8   TTL;
  U8   Proto;
  U16  HeaderCheckSum;
  U32  SrcAddr;
  U32  DstAddr;
} IP_HEADER;

/*********************************************************************
*
*       Transport protocol handling
*/
#define IP_TRANS_PROTO_ATOMIC               0x01
#define IP_TRANS_PROTO_LOCAL_ADDR_REQUIRED  0x02
#define IP_TRANS_PROTO_CONNREQUIRED         0x04

void IP_SOCK_AddProto(const IP_TRANSPORT_PROTO* pTransProto);
void IP_SOCK_Free    (SOCKET* pSock);

extern const IP_TRANSPORT_PROTO IP_TCP_Api;    // Handles only IPv4 packets.
extern const IP_TRANSPORT_PROTO IP_UDP_Api;    // Handles only IPv4 packets.
extern const IP_TRANSPORT_PROTO IP_RAW_Api;
extern const IP_TRANSPORT_PROTO IPV6_TCP_Api;  // Handles IPv4 and IPv6 packets.
extern const IP_TRANSPORT_PROTO IPV6_UDP_Api;  // Handles IPv4 and IPv6 packets.

/*********************************************************************
*
*       IP_ICMP
*/
void IP_ICMP_SendDestUnreachable(U32 Dest, IP_HEADER * pIP, unsigned Code, IFACE *  pIFace);

/*********************************************************************
*
*       IP_SNTPC_
*/
int IP_SNTPC_GetTimeStampFromServerIP(unsigned IFaceId, U32 ServerAddr, IP_NTP_TIMESTAMP *pTimestamp);

/*********************************************************************
*
*       IP_SOCK_ - socket state/handling functions
*/
void IP_SOCK_Wait               (SOCKET *pSock);
void IP_SOCK_WaitTimed          (SOCKET *pSock, I32 Timeout);
void IP_SOCK_Signal             (SOCKET *pSock);
void IP_SOCK_OnConnEstablished  (SOCKET *pSock);
void IP_SOCK_IsDisconnecting    (SOCKET *pSock);
void IP_SOCK_SetStateConnecting (SOCKET *pSock);
void IP_SOCK_MarkAsDisconnected (SOCKET *pSock);
int  IP_SOCK_Abort              (SOCKET *pSock);


//
// ARP table entry structure
//
typedef struct ARP_ENTRY {
  struct ARP_ENTRY *pNext;
  struct ARP_ENTRY *pPrev;
  U32               IPAddr;
  IFACE            *pIFace;
  IP_PACKET        *pPending;             // Packets waiting for resolution of this ARP entry
  unsigned          tExpire;              // Expiration time stamp, [ms]
  unsigned          RepeatCnt;            // Number of times - 1, the ARP-request has been repeated.
  U8                HWAddr[6];            // Physical address
#if IP_ARP_SNIFF_ON_RX
  U8                IsTemp;               // Is this a temporary ARP entry that has been sniffed from an incoming packet ?
#endif
  U8                UsedAfterRefreshCnt;  // Has this entry been used for sending since the last refresh from an incomimg packet ? Acts as counter for executions per seconds as well.
  U8                NumPending;           // Number of pending packets for this entry.
  U8                IsStatic;             // Is this a static ARP entry ?
} ARP_ENTRY;

typedef struct {
  ARP_ENTRY     *pFirstUsed;      // The list of used ARP entries: The ARP table. Doubly linked, last used entry is first.
  ARP_ENTRY     *pFirstFree;      // The list of free ARP entries. Singly linked.
  unsigned       AgeOut;          // Ageout [ms]: After how much time is a valid entry "kicked out" if not used. Typically around 120 sec
  unsigned       AgeOutNoReply;   // Ageout [ms] for entries which have not received a response.                 Typically around   3 sec
  unsigned       AgeOutSniff;     // Ageout [ms] for entries which have been created by sniffing incoming IP packets in _OnRxIP()
  unsigned       NumEntries;
  unsigned       MaxRetries;      // Number of times an ARP-request will be repeated for a pending packet.
  unsigned       MaxPending;      // Number of maximum pending packets per entry.
  int (*pfHandleAddrConflict)  (int IFace, IP_PACKET * pPacket);  // Function pointer for ACD
  int (*pfCheckAutoIPConflict) (int IFace, IP_PACKET * pPacket);  // Function pointer for AutoIP
  IP_HOOK_ON_RX_FIFO_ADD OnEthRxFifoAdd;
  U8             DiscardGratuitousARP;
} IP_ARP_STATIC;

/*********************************************************************
*
*       ARP
*/
void IP_ARP_AddAutoIPConflictHandler(int (*pf)(int IFaceId, IP_PACKET* pPacket));
void IP_ARP_AddACDHandler           (int (*pf)(int IFace, IP_PACKET* pPacket));
int  IP_ARP_HasEntry                (U32 IPAddr);
void IP_ARP_OnRx                    (IP_PACKET *pPacket);
void IP_ARP_SendProbe               (IFACE *pIFace, U32 IPAddr);
void IP_ARP_SendRequest             (IFACE *pIFace, U32 IPAddr);
void IP_ARP_Timer                   (void);

void        IP_ARP_InitModule      (void);
ARP_ENTRY * IP_ARP_FindARPEntryByIP(U32 IPAddr);
int         IP_ARP_SendViaARPEntry (IP_PACKET *pPacket, ARP_ENTRY *pEntry);
int         IP_ARP_SendARPRequestAndCreateEntry(IFACE * pIFace, IP_PACKET * pPacket, U32 IPAddr);
void        IP_ARP_AddEntryToFreeList(ARP_ENTRY * pEntry);
void        IP_ARP_RemoveEntryFromUseList(ARP_ENTRY *pEntry);
ARP_ENTRY * IP_ARP_GetFreeEntry(void);
void        IP_ARP_AddEntryToUseList (ARP_ENTRY * pEntry);

EXTERN IP_ARP_STATIC IP_ARP_Static;

//lint -e(9021) allow #undef.
#undef EXTERN

#if IP_DEBUG != 0
  #define ASSERT_LOCK() IP_OS_AssertLock()
#else
  #define ASSERT_LOCK()
#endif

/*********************************************************************
*
*       IP_WIFI_...
*
*  WiFi functions for WiFi interfaces
*/
#define IP_WIFI_ESS_MASK      (1 << 0)
#define IP_WIFI_IBSS_MASK     (1 << 1)
#define IP_WIFI_PRIVACY_MASK  (1 << 4)

#define IP_WIFI_IE_RSN        0x30
#define IP_WIFI_IE_VENDOR     0xDD

//
// IE OUIs
//
#define IP_WIFI_IE_OUI_WPA_VERSION  0x0050F201

//
// Unicast cipher OUIs in WPA VERSION IE
//
#define IP_WIFI_IE_OUI_WPA_TKIP  0x0050F202
#define IP_WIFI_IE_OUI_WPA_AES   0x0050F204

int  IP_WIFI_DetectSecurity         (const U8* pParams, U16 Len, U16 FixedParams);
void IP_WIFI_Init                   (void);
void IP_WIFI_OnAssociateStatusChange(unsigned IFaceId, U8 IsAssociated, const IP_WIFI_ASSOCIATE_INFO* pInfo);
void IP_WIFI_OnLinkStatusChange     (unsigned IFaceId, U8 IsConnected);
void IP_WIFI_OnSignalChange         (unsigned IFaceId, IP_WIFI_SIGNAL_INFO* pInfo);
void IP_WIFI_OnClientNotification   (unsigned IFaceId, const IP_WIFI_CLIENT_INFO* pInfo, unsigned DisConnect);

/*********************************************************************
*
*       IP_CACHE_...
*
*  Cache related functions
*/
void     IP_CACHE_Dmb        (void);
void     IP_CACHE_Clean      (void *p, unsigned NumBytes);
void     IP_CACHE_Invalidate (void *p, unsigned NumBytes);
unsigned IP_CACHE_GetLineSize(void);

/*********************************************************************
*
*       DNS (Domain name system) / mDNS
*
*  Name resolution
*/
typedef struct IP_MDNS_DISPATCHER_STRUCT {
  int       (*pReceive)(IP_PACKET* pPacket, void* pContext);  // Pointer to the function to be called from the hook.
  void*     pContext;                                         // Pointer on the context to give to the function.
  unsigned  ExpectReply;                                      // Indication if the function expect queries or replies.
  union {
    IP_ADDR          IPAddr;                                  // Multicast IPv4 Address.
#if IP_SUPPORT_IPV6
    U8*              pIPAddrV6;                               // Multicast IPv6 Address.
#endif
  } Addr;
  struct IP_MDNS_DISPATCHER_STRUCT* pNext;                    // Pointer to the next hook.
} IP_MDNS_DISPATCHER;

IP_UDP_CONNECTION* IP_MDNS_DispatchOpen (IP_MDNS_DISPATCHER* pHook, U16 Port, unsigned IsIPv6);
void               IP_MDNS_DispatchClose(IP_MDNS_DISPATCHER* pHook, unsigned IsIPv6);

/*********************************************************************
*
*       NI driver commands
*/
#define IP_NI_CMD_SET_FILTER                     1   // Set filter. Can handle multiple MAC-addresses.
#define IP_NI_CMD_CLR_BPRESSURE                  2   // Clear back-pressure.
#define IP_NI_CMD_SET_BPRESSURE                  3   // Set back-pressure, to avoid receiving more data until the current data is handled.
#define IP_NI_CMD_GET_CAPS                       4   // Retrieves the capabilites, which are a logical-or combination of the IP_NI_CAPS below.
#define IP_NI_CMD_SET_PHY_ADDR                   5   // Allows settings the PHY address.
#define IP_NI_CMD_SET_PHY_MODE                   6   // Allows settings the PHY in a specific mode (duplex, speed).
#define IP_NI_CMD_POLL                           7   // Poll MAC (typically once per ms) in cases where MAC does not trigger an interrupt.
#define IP_NI_CMD_GET_MAC_ADDR                   8   // Retrieve the MAC address from the MAC. This is used for hardware which stores the MAC addr. in an attached EEPROM.
#define IP_NI_CMD_DISABLE                        9   // Disable the network interface (MAC unit + PHY).
#define IP_NI_CMD_ENABLE                        10   // Enable the network interface (MAC unit + PHY).
#define IP_NI_CMD_SET_TX_BUFFER_SIZE            11   // Allows setting the size of the Tx buffer.
#define IP_NI_CMD_SET_SUPPORTED_DUPLEX_MODES    12   // Allows setting the supported duplex modes.
#define IP_NI_CMD_CFG_POLL                      13   // Configure the target to run in polling mode.
#define IP_NI_CMD_DEINIT                        14   // Deinitilize the driver.
#define IP_NI_CMD_SET_HW_ADDR_PREINIT           15   // Set HW addr. in case it is needed to be set before the driver is initialized.
#define IP_NI_CMD_WIFI_SCAN                     16   // Scan for wireless networks.
#define IP_NI_CMD_WIFI_CONNECT                  17   // Connect to a wireless network.
#define IP_NI_CMD_WIFI_DISCONNECT               18   // Disconnect from a wireless network.
#define IP_NI_CMD_WIFI_CONFIG_REG_DOMAIN        19   // Configure regulatory domain (channels by authority) to be used.
#define IP_NI_CMD_WIFI_CONFIG_ALLOWED_CHANNELS  20   // Configure allowed channels (subset of channels allowed by regulatory domain).
#define IP_NI_CMD_SET_CACHED2UNCACHED_OFF       21   // Configure the offset from uncached memory area that is typically used by default to its uncached equivalent.
#define IP_NI_CMD_GET_CAPS_EX                   22   // Get the extended capabilities of the Ethernet controller.
#define IP_NI_CMD_CONFIG_TX_HW_CHECKSUM         23   // Configures if the Tx checksum computation shall be done by the MAC.
#define IP_NI_CMD_WIFI_POLL_LINK_STATE          24   // Poll the current link state.
#define IP_NI_CMD_CONFIG_LINK_CHECK_MULTIPLIER  25   // Configures the multiplier of the link check period.
#define IP_NI_CMD_ADD_PTP_DRIVER                26   // Adds a PTP driver to the NI driver.

/*********************************************************************
*
*       PHY driver commands
*/
#define IP_PHY_CMD_CHECK_RF                       1  // Check Remote Fault.
#define IP_PHY_CMD_CHECK_REG_SANITY               2  // Check registers for sanity.
#define IP_PHY_CMD_DISABLE_CHECKS                 3  // Disable PHY checks based on a mask.
#define IP_PHY_CMD_SET_ADDR                       4  // Set PHY addr.
#define IP_PHY_CMD_SET_MII_MODE                   5  // Set PHY interface mode: 0 (x)MII, 1 (x)RMII.
#define IP_PHY_CMD_SET_SUPPORTED_MODES            6  // Set supported duplex and speed.
#define IP_PHY_CMD_SET_MAC_GIGABIT_SUPPORT        7  // Tell the PHY driver if the MAC supports Gigabit Ethernet.
#define IP_PHY_CMD_SET_FILTER                     8  // Tell the PHY driver to set filters. Can handle multiple MAC-addresses.
#define IP_PHY_CMD_GET_CAPS                       9  // Get the capabilities of the PHY driver.
#define IP_PHY_CMD_GET_ALT_LINK_STATE            10  // Get the link state for an alternate PHY addr.
#define IP_PHY_CMD_GET_LINK_STATE                11  // Get the current link state.
#define IP_PHY_CMD_POLL                          12  // Poll PHY for periodical tasks that can not be handled automatically.
#define IP_PHY_CMD_WIFI_SCAN                     13  // Scan for wireless networks.
#define IP_PHY_CMD_WIFI_CONNECT                  14  // Connect to a wireless network.
#define IP_PHY_CMD_WIFI_DISCONNECT               15  // Disconnect from a wireless network.
#define IP_PHY_CMD_WIFI_CONFIG_REG_DOMAIN        16  // Configure regulatory domain (channels by authority) to be used.
#define IP_PHY_CMD_WIFI_CONFIG_ALLOWED_CHANNELS  17  // Configure allowed channels (subset of channels allowed by regulatory domain).
#define IP_PHY_CMD_CONFIG_LINK_CHECK_MULTIPLIER  18  // Configures the multiplier of the link check period.

/*********************************************************************
*
*       IP_PHY_...
*
*  PHY related functions
*/
void               IP_PHY_ConfigMiiMode        (unsigned IFaceId, unsigned Mode);
int                IP_PHY_Control              (unsigned IFaceId, unsigned Cmd, void* p);
IP_PHY_CONTEXT_EX* IP_PHY_GetContext           (unsigned IFaceId);
int                IP_PHY_GetLinkState         (unsigned IFaceId, U32* pDuplex, U32* pSpeed);
unsigned           IP_PHY_GetNumStaticFilters  (unsigned IFaceId);
int                IP_PHY_GetUseStaticFilters  (unsigned IFaceId);
void*              IP_PHY_GetUserContext_NoLock(unsigned IFaceId);
void               IP_PHY_Init                 (unsigned IFaceId);
void               IP_PHY_OnReset              (IP_PHY_CONTEXT* pContext);
void               IP_PHY_ReInit_NoLock        (unsigned IFaceId);
unsigned           IP_PHY_Read                 (void* pContext, unsigned RegIndex);
void               IP_PHY_RemoveDriver         (unsigned IFaceId, char CleanDriverContext);
void               IP_PHY_SetUserContext_NoLock(unsigned IFaceId, void* pContext);
void               IP_PHY_UpdateMACFilter      (unsigned IFaceId, void* p);
void               IP_PHY_Write                (void* pContext, unsigned RegIndex, unsigned Data);

int                IP_PHY_GENERIC_AttachContext(unsigned IFaceId, IP_PHY_CONTEXT_EX* pContextEx);
int                IP_PHY_GENERIC_Control      (unsigned IFaceId, IP_PHY_CONTEXT_EX* pContextEx, unsigned Cmd, void* p);
int                IP_PHY_GENERIC_GetLinkState (unsigned IFaceId, IP_PHY_CONTEXT_EX* pContextEx, U32* pDuplex, U32* pSpeed);
int                IP_PHY_GENERIC_Init         (unsigned IFaceId, IP_PHY_CONTEXT_EX* pContextEx);

/*********************************************************************
*
*       IP_PTP_...
*
*  PTP related functions
*/
void  IP_PTP_SetSentTime(U32 Seconds, U32 NSeconds);

/*********************************************************************
*
*       embOS/IP profiling instrumentation
*
**********************************************************************
*/

/*********************************************************************
*
*       Profile event identifiers
*/
enum {
  //
  // Events for IP API functions (IDs 0-249).
  //
  IP_EVTID_INIT = 0,
  IP_EVTID_DEINIT,
  IP_EVTID_SET_ADMIN_STATE_NOLOCK,
  //
  // Events for BSD socket API functions (IDs 250-299).
  //
  IP_EVTID_BSD_CONNECT = 250,
  IP_EVTID_BSD_ACCEPT,
  IP_EVTID_BSD_SEND,
  IP_EVTID_BSD_SENDTO,
  IP_EVTID_BSD_RECV,
  IP_EVTID_BSD_RECVFROM,
  //
  // Events for internal FIFO monitoring (requires IP_SUPPORT_PROFILE_FIFO=1; IDs 500-519).
  //
  IP_EVTID_FIFO_ADD = 500,
  IP_EVTID_FIFO_ADD_NOLOCK,
  IP_EVTID_FIFO_GETLEAVE,
  IP_EVTID_FIFO_GETREMOVE,
  IP_EVTID_FIFO_GETREMOVE_NOLOCK,
  IP_EVTID_FIFO_TRYGETREMOVE,
  IP_EVTID_FIFO_TRYGETREMOVE_NOLOCK,
  //
  // FIFO load monitoring (requires IP_SUPPORT_PROFILE_FIFO=1 && ((IP_DEBUG_FIFO != 0) || (IP_SUPPORT_STATS_FIFO != 0))).
  //
  IP_EVTID_FIFO_UPDATE = 519,
  //
  // Events for internal packet monitoring (IDs 520-539).
  //
  IP_EVTID__SENDPACKET = 520,
  IP_EVTID__SENDPACKET_INFO,
  IP_EVTID_READPACKETS_SINGLEIF,
  IP_EVTID_PACKET_RECEIVED_ETH,
  IP_EVTID_PACKET_ALLOC_NOLOCK,
  IP_EVTID_PACKET_FREE_NOLOCK,
  //
  // Events for other internal events (IDs 560-...).
  //
  IP_EVTID_RX_INT_DISABLE = 560,  // Placeholder, implemented as start/stop user event.
  IP_EVTID_RX_INT_ENABLE,         // Placeholder, to be used when going away from the user event.
  //
  // Make sure this is the last entry.
  //
  IP_EVTID_LAST
};

#define IP_PROFILE_API_DESC  "M=embOSIP"  \
                             ",V=32000"

#define IP_PROFILE_GET_EVENT_ID(EvtId)  ((EvtId) + IP_Global.Profile.IdOffset)

/*********************************************************************
*
*       IP_PROFILE_CALL_VOID()
*/
#if IP_SUPPORT_PROFILE
  #define IP_PROFILE_CALL_VOID(EventId)                                        \
    if (IP_Global.Profile.pAPI) {                                              \
      IP_Global.Profile.pAPI->pfRecordVoid(IP_PROFILE_GET_EVENT_ID(EventId));  \
    }
#else
  #define IP_PROFILE_CALL_VOID(EventId)
#endif

/*********************************************************************
*
*       IP_PROFILE_CALL_U32()
*/
#if IP_SUPPORT_PROFILE
  #define IP_PROFILE_CALL_U32(EventId, Para0)                                               \
    if (IP_Global.Profile.pAPI) {                                                           \
      IP_Global.Profile.pAPI->pfRecordU32(IP_PROFILE_GET_EVENT_ID(EventId), (U32)(Para0));  \
    }
#else
  #define IP_PROFILE_CALL_U32(EventId, Para0)
#endif

/*********************************************************************
*
*       IP_PROFILE_CALL_U32x2()
*/
#if IP_SUPPORT_PROFILE
  #define IP_PROFILE_CALL_U32x2(EventId, Para0, Para1)                                                      \
    if (IP_Global.Profile.pAPI) {                                                                           \
      IP_Global.Profile.pAPI->pfRecordU32x2(IP_PROFILE_GET_EVENT_ID(EventId), (U32)(Para0), (U32)(Para1));  \
    }
#else
  #define IP_PROFILE_CALL_U32x2(Id, Para0, Para1)
#endif

/*********************************************************************
*
*       IP_PROFILE_CALL_U32x3()
*/
#if IP_SUPPORT_PROFILE
  #define IP_PROFILE_CALL_U32x3(EventId, Para0, Para1, Para2)                                                             \
    if (IP_Global.Profile.pAPI) {                                                                                         \
      IP_Global.Profile.pAPI->pfRecordU32x3(IP_PROFILE_GET_EVENT_ID(EventId), (U32)(Para0), (U32)(Para1), (U32)(Para2));  \
    }
#else
  #define IP_PROFILE_CALL_U32x3(Id, Para0, Para1, Para2)
#endif

/*********************************************************************
*
*       IP_PROFILE_CALL_U32x4()
*/
#if IP_SUPPORT_PROFILE
  #define IP_PROFILE_CALL_U32x4(EventId, Para0, Para1, Para2, Para3)                                                                    \
    if (IP_Global.Profile.pAPI) {                                                                                                       \
      IP_Global.Profile.pAPI->pfRecordU32x4(IP_PROFILE_GET_EVENT_ID(EventId), (U32)(Para0), (U32)(Para1), (U32)(Para2), (U32)(Para3));  \
    }
#else
  #define IP_PROFILE_CALL_U32x4(Id, Para0, Para1, Para2, Para3)
#endif

/*********************************************************************
*
*       IP_PROFILE_CALL_U32x5()
*/
#if IP_SUPPORT_PROFILE
  #define IP_PROFILE_CALL_U32x5(EventId, Para0, Para1, Para2, Para3, Para4)                                                                           \
    if (IP_Global.Profile.pAPI) {                                                                                                                     \
      IP_Global.Profile.pAPI->pfRecordU32x5(IP_PROFILE_GET_EVENT_ID(EventId), (U32)(Para0), (U32)(Para1), (U32)(Para2), (U32)(Para3), (U32)(Para4));  \
    }
#else
  #define IP_PROFILE_CALL_U32x5(Id, Para0, Para1, Para2, Para3, Para4)
#endif

/*********************************************************************
*
*       IP_PROFILE_CALL_U32x6()
*/
#if IP_SUPPORT_PROFILE
  #define IP_PROFILE_CALL_U32x6(EventId, Para0, Para1, Para2, Para3, Para4, Para5)                                                                                  \
    if (IP_Global.Profile.pAPI) {                                                                                                                                   \
      IP_Global.Profile.pAPI->pfRecordU32x6(IP_PROFILE_GET_EVENT_ID(EventId), (U32)(Para0), (U32)(Para1), (U32)(Para2), (U32)(Para3), (U32)(Para4), (U32)(Para5));  \
    }
#else
  #define IP_PROFILE_CALL_U32x6(Id, Para0, Para1, Para2, Para3, Para4, Para5)
#endif

/*********************************************************************
*
*       IP_PROFILE_CALL_STRING()
*/
#if IP_SUPPORT_PROFILE
  #define IP_PROFILE_CALL_STRING(EventId, pPara0)                                                       \
    if (IP_Global.Profile.pAPI) {                                                                       \
      IP_Global.Profile.pAPI->pfRecordString(IP_PROFILE_GET_EVENT_ID(EventId), (const char*)(pPara0));  \
    }
#else
  #define IP_PROFILE_CALL_STRING(EventId, pPara0)
#endif

/*********************************************************************
*
*       IP_PROFILE_CALL_STRING_U32()
*/
#if IP_SUPPORT_PROFILE
  #define IP_PROFILE_CALL_STRING_U32(EventId, pPara0, Para1)                                                             \
    if (IP_Global.Profile.pAPI) {                                                                                        \
      IP_Global.Profile.pAPI->pfRecordStringU32(IP_PROFILE_GET_EVENT_ID(EventId), (const char*)(pPara0), (U32)(Para1));  \
    }
#else
  #define IP_PROFILE_CALL_STRING_U32(EventId, pPara0, Para1)
#endif

/*********************************************************************
*
*       IP_PROFILE_CALL_RECEIVE_ETH_PACKET()
*/
#if IP_SUPPORT_PROFILE
  #define IP_PROFILE_CALL_RECEIVE_ETH_PACKET(EventId, Para0, Para1, Para2, Para3, Para4, pData0, NumBytes0)                  \
    if (IP_Global.Profile.pAPI) {                                                                                        \
      IP_Global.Profile.pAPI->pfRecordEthRxPacket(IP_PROFILE_GET_EVENT_ID(EventId), (U32)(Para0), (U32)(Para1), (U32)(Para2), (U32)(Para3), (U32)(Para4), (U8*)(pData0), (U32)(NumBytes0));  \
    }
#else
  #define IP_PROFILE_CALL_RECEIVE_ETH_PACKET(EventId, Para0, Para1, Para2, Para3, Para4, pData0, NumBytes0)
#endif

/*********************************************************************
*
*       IP_PROFILE_CALL_RX_INT_ONOFF()
*/
#if IP_SUPPORT_PROFILE
  #define IP_PROFILE_CALL_RX_INT_ONOFF(EventId, Para0, Para1)                                                        \
    if (IP_Global.Profile.pAPI) {                                                                                    \
      IP_Global.Profile.pAPI->pfRecordEnDisableRxInt(IP_PROFILE_GET_EVENT_ID(EventId), (U32)(Para0), (U32)(Para1));  \
    }
#else
  #define IP_PROFILE_CALL_RX_INT_ONOFF(EventId, Para0, Para1)
#endif

/*********************************************************************
*
*       IP_PROFILE_END_CALL()
*/
#if (IP_SUPPORT_PROFILE != 0) && (IP_SUPPORT_PROFILE_END_CALL != 0)
  #define IP_PROFILE_END_CALL(EventId)                                            \
    if (IP_Global.Profile.pAPI) {                                                 \
      IP_Global.Profile.pAPI->pfRecordEndCall(IP_PROFILE_GET_EVENT_ID(EventId));  \
    }
#else
  #define IP_PROFILE_END_CALL(EventId)
#endif

/*********************************************************************
*
*       IP_PROFILE_END_CALL_U32()
*/
#if (IP_SUPPORT_PROFILE != 0) && (IP_SUPPORT_PROFILE_END_CALL != 0)
  #define IP_PROFILE_END_CALL_U32(EventId, ReturnValue)                                                \
    if (IP_Global.Profile.pAPI) {                                                                      \
      IP_Global.Profile.pAPI->pfRecordEndCallU32(IP_PROFILE_GET_EVENT_ID(EventId), (U32)ReturnValue);  \
    }
#else
  #define IP_PROFILE_END_CALL_U32(EventId, ReturnValue)
#endif

/*********************************************************************
*
*       Other structures
*/
typedef struct {
  U8 NumPreciseFilter;    // Number of supported precise filters.
  U8 HasHashFilter;       // Hash filter supported?       0: Not supported, 1: Supported.
  U8 HasPromiscuousMode;  // Promiscuous mode supported?  0: Not supported, 1: Supported.
  U8 PacketDataShiftCnt;  // ShiftCnt for ideal packet data alignment. Could avoid a memcpy into an extra buffer that is aligned.
} IP_NI_CMD_GET_CAPS_EX_DATA;

//
// OO: Placed prototype here as with other IP_NI_ functions it causes too
//     much trouble as IP_NI_CMD_GET_CAPS_EX_DATA is required.
//
int IP_NI_GetCapsEx(unsigned IFaceId, IP_NI_CMD_GET_CAPS_EX_DATA* pCapsEx);

typedef struct {
  U8 NumStaticFilter;  // Number of supported static MAC filter.
} IP_PHY_CMD_GET_CAPS_DATA;

typedef struct {
  U32 Duplex;
  U32 Speed;
  U8  AltPhyAddr;
} IP_PHY_CMD_GET_ALT_LINK_STATE_DATA;

typedef struct {
        unsigned  NumAddr;
  const U8       *pHWAddr;  // Hardware addresses
} IP_NI_CMD_SET_FILTER_DATA;

typedef struct {
        IP_WIFI_pfScanResult pf;
  const char*                sSSID;
        U32                  Timeout;  // Timeout [ms].
        U8                   Channel;
} IP_NI_CMD_WIFI_SCAN_DATA;

typedef struct {
  const IP_WIFI_CONNECT_PARAMS* pParams;
        U32                     Timeout;
} IP_NI_CMD_WIFI_CONNECT_DATA;

typedef struct {
  const U8* paChannel;
        U8  NumChannels;
} IP_NI_CMD_WIFI_CONFIG_ALLOWED_CHANNELS_DATA;

typedef struct {
  IP_PHY_CONTEXT LegacyContext;
  U16            Bmsr;
} IP_PHY_GENERIC_CONTEXT;

/*********************************************************************
*
*       Compatibility
*
*  Various defines to map obsolete function names to new ones
*/
#define IP_OnRx IP_ETH_OnRx   // Compatibility with older drivers

#if defined(__cplusplus)
  }              // Make sure we have C-declarations in C++ programs
#endif

//
// Include IPV6_Int.h last to make sure that IP.h loads
// IP_ConfDefaults.h and data and structure types that we use.
//
#if IP_SUPPORT_IPV6
#include "IPV6_Int.h"
#endif

#endif                // Avoid multiple/recursive inclusion

/*************************** End of file ****************************/
