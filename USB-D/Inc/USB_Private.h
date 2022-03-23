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
File    : USB_Private.h
Purpose : Private include file.
          Do not modify to allow easy updates !
Literature:
  [1]  Universal Serial Bus Specification Revision 2.0
       \\fileserver\Techinfo\Subject\USB\USB_20\usb_20.pdf
--------  END-OF-HEADER  ---------------------------------------------
*/

#ifndef USB_PRIVATE_H
#define USB_PRIVATE_H

#include "USB.h"

/*********************************************************************
*
*       Defines, fixed
*
**********************************************************************
*/

#define USB_MAX_ALLOWED_EPADDR      0x20u

#define USB_NUM_TX_DATA_PARTS       3u

// Parameters of functions can't be made 'const', if pointer to function is used as callback or in API structure.
#define USBD_API_USE        /*lint -e{818}  N:100 */

/*********************************************************************
*
*       Driver commands
*/
#define USB_DRIVER_CMD_SET_CONFIGURATION        0
#define USB_DRIVER_CMD_GET_TX_BEHAVIOR          1     // obsolete !!!
#define USB_DRIVER_CMD_GET_SETADDRESS_BEHAVIOR  2
#define USB_DRIVER_CMD_REMOTE_WAKEUP            3
#define USB_DRIVER_CMD_TESTMODE                 4
#define USB_DRIVER_CMD_GET_TX_MAX_TRANSFER_SIZE 5     // obsolete !!!
#define USB_DRIVER_CMD_GET_RX_BEHAVIOR          6
#define USB_DRIVER_CMD_ASSIGN_MEMORY            7
#define USB_DRIVER_CMD_GET_MEM_ALIGNMENT        8


#define USB_CMD_TESTMODE_TEST_J                 1
#define USB_CMD_TESTMODE_TEST_K                 2
#define USB_CMD_TESTMODE_TEST_SE0_NAK           3
#define USB_CMD_TESTMODE_TEST_PACKET            4
#define USB_CMD_TESTMODE_TEST_FORCE_ENABLE      5

/*********************************************************************
*
*       Types
*
**********************************************************************
*/
#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif



struct _USB_INFO_BUFFER {
  unsigned Cnt;
  unsigned Sizeof;
  U8 * pBuffer;
};

typedef struct _USB_BUFFER{
  U8       * pData;
  unsigned   Size;
  unsigned   NumBytesIn;
  unsigned   RdPos;
} USB_BUFFER;

struct _EP_STAT {
  U16                       MaxPacketSize;
  U16                       MaxPacketSizeUser;
  U16                       Interval;                   // In frames. Valid only for ISO and interrupt endpoints. Not used by stack except for creating descriptors.
  U8                        EPType;                     // 0: Control, 1: Isochronous, 2: Bulk, 3: Interrupt See [1], chapter "9.6.6  Endpoint", table 9-13.
  U8                        EPAddr;                     // b[4:0]: EPAddr => This is the physical endpoint number.
                                                        // b[6:5]: Unused, SBZ.
                                                        // b[7]:   Direction, 1: Device to Host (IN), 0: Host to device (OUT).
  volatile I8               IsHalted;                   // 1: Endpoint is stalled. Typically set by application via call to USBD_StallEP() in case of fatal error.
  U8                        AsyncCallbackUsed;
  U16                       TransactionCnt;             // Counter to synchronize OS_Wait() + OS_Signal functions.
  union {
    struct _TXINFO {
      I32                   TxNumBytesPending;          // It indicates whether a IN transfer is already queued for the endpoint.
                                                        // 0: No TX transfer is in progress.
                                                        // 1: Transfer of a NULL packet is in progress.
                                                        // >0: Transfer of (TxNumBytesPending - 1) bytes of data is in progress.
                                                        // -1: Intermediate state: TX transfer is just finished, but a new transfer cannot be started yet.
      U32                   NumBytesWithout0Packet;     // No 0-packet is sent after a transfer of a multiple of this number of bytes.
                                                        // Must be a multiple of MaxPacketSize and a power of 2.
      U16                   NumFullPacketsSend;         // Number of full packets (MaxPacketSize) send within a transfer.
      U8                    Send0PacketIfRequired;      // This flag is used with IN endpoints. When set the stack will check at the end of a transaction whether
                                                        // the last transfer was a multiple of MaxPacketSize, if yes it will send a zero packet to complete the transaction.
                                                        // This flag is normally depending on which API function is used. Some API functions even allow the user to set it.
      U8                    SendEventOccured;           // Set if the driver has triggered USB_EVENT_DATA_SEND.
                                                        // If not, the event is later triggered together with USB_EVENT_DATA_ACKED.
      U8                    SignalOnTXReady;            // Send event if TX queue gets a free entry
      U8                    NumDataParts;
      USB_DATA_PART         DataParts[USB_NUM_TX_DATA_PARTS];
    } TxInfo;
    struct _RXINFO {
      USB_ON_RX_FUNC *      pfOnRx;                     // Pointer to a callback function set either by USBD_SetOnRxEP0() or by USBD_SetOnRXHookEP() functions.
      U8             *      pData;
      volatile U32          NumBytesRem;                // Volatile since modified by ISR and checked by task/application.
      I8                    RxInterruptEnabled;         // Keep track of driver RX interrupt enable / disable
      U8                    ReadMode;                   // See USB_READ_MODE... defines.
      U8                    AllowShortPacket;           // Controls the behavior of the internal _OnRxCheckDone() function.
      U8                    RxPacketByPacket;           // Perform read operation packet by packet (only request a single packet from the driver)
      USB_BUFFER            Buffer;
    } RxInfo;
  } Dir;
  USB_EVENT_CALLBACK        *pEventCallbacks;           // List of callback functions for this endpoint.
};

typedef struct _USB_IAD_API {
  void (*pfAdd)(U8 FirstInterFaceNo, U8 NumInterfaces, U8 ClassNo, U8 SubClassNo, U8 ProtocolNo);
  void (*pfAddIadDesc)(unsigned InterFaceNo, USB_INFO_BUFFER * pInfoBuffer);
} USB_IAD_API;

typedef struct _INTERFACE {
  U16                       EPs;
  U8                        IFAlternateSetting;
  U8                        IFClass   ;    // Interface Class
  U8                        IFSubClass;    // Interface Subclass
  U8                        IFProtocol;    // Interface Protocol
  U8                        IFNum;
  U8                        iName;         // Index of String descriptor
  USB_ADD_FUNC_DESC       * pfAddFuncDesc;
  USB_ADD_EP_FUNC_DESC    * pfAddEndpointFuncDesc;
  USB_ON_CLASS_REQUEST    * pfOnClassRequest;
  USB_ON_CLASS_REQUEST    * pfOnVendorRequest;
  USB_ON_SETUP            * pfOnSetup;
} INTERFACE;

typedef struct _ALT_INTERFACE {
  U8        IFNo;   // Index of the real interface
  INTERFACE AltIF;  // Interface structure
} ALT_INTERFACE;


/*********************************************************************
*
*       USBD_PROFILE_API
*/
typedef struct {
  void(*pfRecordEndCall)       (unsigned int EventId);
  void(*pfRecordEndCallU32)    (unsigned int EventId, U32 Para0);
  void(*pfRecordVoid)          (unsigned int EventId);
  void(*pfRecordU32)           (unsigned int EventId, U32 Para0);
  void(*pfRecordU32x2)         (unsigned int EventId, U32 Para0, U32 Para1);
  void(*pfRecordU32x3)         (unsigned int EventId, U32 Para0, U32 Para1, U32 Para2);
  void(*pfRecordU32x4)         (unsigned int EventId, U32 Para0, U32 Para1, U32 Para2, U32 Para3);
  void(*pfRecordU32x5)         (unsigned int EventId, U32 Para0, U32 Para1, U32 Para2, U32 Para3, U32 Para4);
  void(*pfRecordU32x6)         (unsigned int EventId, U32 Para0, U32 Para1, U32 Para2, U32 Para3, U32 Para4, U32 Para5);
  void(*pfRecordU32x7)         (unsigned int EventId, U32 Para0, U32 Para1, U32 Para2, U32 Para3, U32 Para4, U32 Para5, U32 Para6);
  void(*pfRecordU32x8)         (unsigned int EventId, U32 Para0, U32 Para1, U32 Para2, U32 Para3, U32 Para4, U32 Para5, U32 Para6, U32 Para7);
  void(*pfRecordU32x9)         (unsigned int EventId, U32 Para0, U32 Para1, U32 Para2, U32 Para3, U32 Para4, U32 Para5, U32 Para6, U32 Para7, U32 Para8);
  void(*pfRecordU32x10)        (unsigned int EventId, U32 Para0, U32 Para1, U32 Para2, U32 Para3, U32 Para4, U32 Para5, U32 Para6, U32 Para7, U32 Para8, U32 Para9);
  void(*pfRecordString)        (unsigned int EventId, const char* pPara0);
  U32(*pfPtr2Id)              (U32 Ptr);
} USBD_PROFILE_API;

typedef struct {
  U32                   IdOffset;
  const USBD_PROFILE_API* pAPI;
} USBD_PROFILE;

U32  USBD_PROFILE_GetAPIDesc(const char** psDesc);
void USBD_PROFILE_SetAPI(const USBD_PROFILE_API* pAPI, U32 IdOffset);

typedef struct {
  U8                        NumEPs;               // Count of currently used endpoints.
  U8                        NumIFs;               // Count of currently used interfaces.
  U8                        NumAltIFs;            // Count of currently used alternative interfaces.
  U8                        NumEvents;            // Next event, that may be allocated.
  U8                        Class;                // Device Class    (when IAD is not used the class is defined in the device descriptor)
  U8                        SubClass;             // Device Subclass (when IAD is not used the subclass is defined in the device descriptor)
  U8                        Protocol;             // Device Protocol (when IAD is not used the Protocol is defined in the device descriptor)
  I8                        MultiPacketRxBehavior;   // True if driver can receive multiple packets via DMA but does not report each packet
                                                     // received to the stack. Used for correct timeout behavior.
  U8                        SetAddressBehavior;
  U8                        IsInited;             // Flag indicating whether emUSB-Device was initialized.
  volatile U8               State;                // Global USB state, similar to [1]: chapter 9.1.1.
                                                  // Bitwise combination of USB_STAT_ATTACHED, USB_STAT_READY, USB_STAT_ADDRESSED, USB_STAT_CONFIGURED, USB_STAT_SUSPENDED
  volatile U8               AllowRemoteWakeup;    // b[0]: 1: Remote wake-up feature allowed by the application (through USBD_SetAllowRemoteWakeUp()).
                                                  // b[1]: 1: Remote wake-up feature allowed by the host.
  volatile U8               Addr;                 // The USB device address, assigned by the host. Zero when the device is not enumerated.
  volatile U8               IsSelfPowered;        // Flag indicating whether the device is allowed to draw current from the bus or whether it has a differnt power supply.
  U8                        NumStringDesc;        // Number of String descriptors
  const char *              aStringDesc[USB_MAX_STRING_DESC];
  INTERFACE                 aIF[USB_MAX_NUM_IF];  // Array of available interfaces.
#if USB_MAX_NUM_ALT_IF > 0u
  ALT_INTERFACE             aAltIF[USB_MAX_NUM_ALT_IF];  // Array of available alternative interfaces.
#endif
  U16                       TransactionCnt;       // Counter to synchronize OS_Wait() + OS_Signal functions.
  U8                        NumOnRxEP0Callbacks;  // Count of receive callbacks for endpoint zero.
  U8                        MaxPower;             // Used in configuration descriptor.
  USB_ON_RX_FUNC          * apfOnRxEP0[USB_MAX_NUM_COMPONENTS]; // Array of function pointers to receive callbacks for endpoint zero.
  const USB_HW_DRIVER     * pDriver;              // Pointer to the hardware driver structure.
  USB_ENABLE_ISR_FUNC     * pfEnableISR;          // Pointer to function to enable the USB interrupt and set the interrupt handler.
  const USB_IAD_API       * pIadAPI;              // Pointer to the USB Interface Association Descriptor API.
  USB_DEINIT_FUNC         * apfDeInitHandler[5];  // Array of pointers to functions which should be called when USBD_DeInit() is called. These callbacks are normally set by class modules.
  USB_DETACH_FUNC         * pfDetach;             // Pointer to a callback which is called on a detach event. Set from the application (USBD_SetDetachFunc()).
  USB_GET_STRING_FUNC     * pfGetString;          // Pointer to a callback which is called when the host requests a string descriptor from the device. Set from the application (USBD_SetGetStringFunc()).
  USB_ON_BCD_VERSION_FUNC * pfOnBCDVersion;       // Pointer to a callback which is called when the host requests a device descriptor from the device. Set from the application (USBD_SetOnBCDVersionFunc()).
  USB_DEINIT_FUNC         * pfDeInitUserHandler;  // Pointer to a user-set callback which is called at the end of the execution of USBD_DeInit(). Set from the application (USBD_SetDeInitUserFunc()).
  USB_ATTACH_FUNC         * pfAttach;             // Pointer to the attach function (optional).
  USB_ON_SET_IF_FUNC      * pfOnSetInterface;     // Pointer to a callback which is called when a Set Interface command is received. Set from the application (USB_SetOnSetInterfaceFunc()).
  const USB_DEVICE_INFO   * pDeviceInfo;          // Pointer to device information used during enumeration.
  U8                        aPhyAddr2EPIndex[USB_MAX_ALLOWED_EPADDR];  // Mapping table, physical to logical endpoint address.
  USB_HOOK                * pFirstSCHook;         // List of hook function called on status change.
  SEGGER_CACHE_CONFIG       CacheConfig;
#if USBD_SUPPORT_PROFILE
  USBD_PROFILE           Profile;
#endif
  const char              * sCopyright;
} GLOBAL;

enum _STRING_INDEX {
  STRING_INDEX_LANGUAGE = 0,  // Language index. MUST BE 0 acc. to spec.
  STRING_INDEX_MANUFACTURER,  // iManufacturer:      Index of String Desc (Manuf)    (variable, but needs to be unique)
  STRING_INDEX_PRODUCT,       // iProduct:           Index of String Desc (Product)  (variable, but needs to be unique)
  STRING_INDEX_SN,            // iSerialNumber:      Index of String Desc (Serial #) (variable, but needs to be unique)
  STRING_INDEX_CONFIG,        // iConfiguration:     Index of String Desc (Configuration name) (variable, but needs to be unique)
  STRING_INDEX_MAX
};

#define STRING_INDEX_MS_OS  0xEEu   // Special String index that is used by Microsoft to detect "special" interfaces/devices
#define STRING_INDEX_OTHER  ((unsigned)STRING_INDEX_MAX)   // Start index of other string descriptors store in aStringDesc[]

/*********************************************************************
*
*       Global data
*
**********************************************************************
*/

extern EP_STAT USB_aEPStat[USB_NUM_EPS];
extern GLOBAL  USB_Global;
#ifdef USB_MAIN_C
       EP_STAT USB_aEPStat[USB_NUM_EPS];
       GLOBAL  USB_Global;
#endif

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
  USBD_EVTID_INIT = 0,
  USBD_EVTID_DEINIT,
  USBD_EVTID_START,
  USBD_EVTID_STOP,
  //
  // Events for Read/Write (IDs 250-299).
  //
  USBD_EVTID_READ = 250,
  USBD_EVTID_RECEIVE,
  USBD_EVTID_READOVERLAPPED,
  USBD_EVTID_WRITE,
  USBD_EVTID_WRITEOVERLAPPED,
  USBD_EVTID_CANCELIO,
  //
  // Events for other internal events (IDs 560-...).
  //
  USBD_EVTID_INTERNAL1 = 560,  // Placeholder.
  //
  // Make sure this is the last entry.
  //
  USBD_EVTID_LAST
};

#define USBD_PROFILE_API_DESC  "M=emUSBD,V=30214"

#define USBD_PROFILE_GET_EVENT_ID(EvtId)  ((EvtId) + USB_Global.Profile.IdOffset)

/*********************************************************************
*
*       USBD_PROFILE_CALL_VOID()
*/
#if USBD_SUPPORT_PROFILE
#define USBD_PROFILE_CALL_VOID(EventId)                                        \
    if (USB_Global.Profile.pAPI) {                                              \
      USB_Global.Profile.pAPI->pfRecordVoid(USBD_PROFILE_GET_EVENT_ID(EventId));  \
    }
#else
#define USBD_PROFILE_CALL_VOID(EventId)
#endif

/*********************************************************************
*
*       USBD_PROFILE_CALL_U32()
*/
#if USBD_SUPPORT_PROFILE
#define USBD_PROFILE_CALL_U32(EventId, Para0)                                               \
    if (USB_Global.Profile.pAPI) {                                                           \
      USB_Global.Profile.pAPI->pfRecordU32(USBD_PROFILE_GET_EVENT_ID(EventId), (U32)(Para0));  \
    }
#else
#define USBD_PROFILE_CALL_U32(EventId, Para0)
#endif

/*********************************************************************
*
*       USBD_PROFILE_CALL_U32x2()
*/
#if USBD_SUPPORT_PROFILE
#define USBD_PROFILE_CALL_U32x2(EventId, Para0, Para1)                                                      \
    if (USB_Global.Profile.pAPI) {                                                                           \
      USB_Global.Profile.pAPI->pfRecordU32x2(USBD_PROFILE_GET_EVENT_ID(EventId), (U32)(Para0), (U32)(Para1));  \
    }
#else
#define USBD_PROFILE_CALL_U32x2(Id, Para0, Para1)
#endif

/*********************************************************************
*
*       USBD_PROFILE_CALL_U32x3()
*/
#if USBD_SUPPORT_PROFILE
#define USBD_PROFILE_CALL_U32x3(EventId, Para0, Para1, Para2)                                                             \
    if (USB_Global.Profile.pAPI) {                                                                                         \
      USB_Global.Profile.pAPI->pfRecordU32x3(USBD_PROFILE_GET_EVENT_ID(EventId), (U32)(Para0), (U32)(Para1), (U32)(Para2));  \
    }
#else
#define USBD_PROFILE_CALL_U32x3(Id, Para0, Para1, Para2)
#endif

/*********************************************************************
*
*       USBD_PROFILE_CALL_U32x4()
*/
#if USBD_SUPPORT_PROFILE
#define USBD_PROFILE_CALL_U32x4(EventId, Para0, Para1, Para2, Para3)                                                                    \
    if (USB_Global.Profile.pAPI) {                                                                                                       \
      USB_Global.Profile.pAPI->pfRecordU32x4(USBD_PROFILE_GET_EVENT_ID(EventId), (U32)(Para0), (U32)(Para1), (U32)(Para2), (U32)(Para3));  \
    }
#else
#define USBD_PROFILE_CALL_U32x4(Id, Para0, Para1, Para2, Para3)
#endif

/*********************************************************************
*
*       USBD_PROFILE_CALL_U32x5()
*/
#if USBD_SUPPORT_PROFILE
#define USBD_PROFILE_CALL_U32x5(EventId, Para0, Para1, Para2, Para3, Para4)                                                                           \
    if (USB_Global.Profile.pAPI) {                                                                                                                     \
      USB_Global.Profile.pAPI->pfRecordU32x5(USBD_PROFILE_GET_EVENT_ID(EventId), (U32)(Para0), (U32)(Para1), (U32)(Para2), (U32)(Para3), (U32)(Para4));  \
    }
#else
#define USBD_PROFILE_CALL_U32x5(Id, Para0, Para1, Para2, Para3, Para4)
#endif

/*********************************************************************
*
*       USBD_PROFILE_CALL_U32x6()
*/
#if USBD_SUPPORT_PROFILE
#define USBD_PROFILE_CALL_U32x6(EventId, Para0, Para1, Para2, Para3, Para4, Para5)                                                                                  \
    if (USB_Global.Profile.pAPI) {                                                                                                                                   \
      USB_Global.Profile.pAPI->pfRecordU32x6(USBD_PROFILE_GET_EVENT_ID(EventId), (U32)(Para0), (U32)(Para1), (U32)(Para2), (U32)(Para3), (U32)(Para4), (U32)(Para5));  \
    }
#else
#define USBD_PROFILE_CALL_U32x6(Id, Para0, Para1, Para2, Para3, Para4, Para5)
#endif

/*********************************************************************
*
*       USBD_PROFILE_CALL_STRING()
*/
#if USBD_SUPPORT_PROFILE
#define USBD_PROFILE_CALL_STRING(EventId, pPara0)                                                       \
    if (USB_Global.Profile.pAPI) {                                                                       \
      USB_Global.Profile.pAPI->pfRecordString(USBD_PROFILE_GET_EVENT_ID(EventId), (const char*)(pPara0));  \
    }
#else
#define USBD_PROFILE_CALL_STRING(EventId, pPara0)
#endif

/*********************************************************************
*
*       USBD_PROFILE_CALL_STRING_U32()
*/
#if USBD_SUPPORT_PROFILE
#define USBD_PROFILE_CALL_STRING_U32(EventId, pPara0, Para1)                                                             \
    if (USB_Global.Profile.pAPI) {                                                                                        \
      USB_Global.Profile.pAPI->pfRecordStringU32(USBD_PROFILE_GET_EVENT_ID(EventId), (const char*)(pPara0), (U32)(Para1));  \
    }
#else
#define USBD_PROFILE_CALL_STRING_U32(EventId, pPara0, Para1)
#endif

/*********************************************************************
*
*       USBD_PROFILE_END_CALL()
*/
#if USBD_SUPPORT_PROFILE
#define USBD_PROFILE_END_CALL(EventId)                                            \
    if (USB_Global.Profile.pAPI) {                                                 \
      USB_Global.Profile.pAPI->pfRecordEndCall(USBD_PROFILE_GET_EVENT_ID(EventId));  \
    }
#else
#define USBD_PROFILE_END_CALL(EventId)
#endif

/*********************************************************************
*
*       USBD_PROFILE_END_CALL_U32()
*/
#if USBD_SUPPORT_PROFILE
#define USBD_PROFILE_END_CALL_U32(EventId, ReturnValue)                                                \
    if (USB_Global.Profile.pAPI) {                                                                      \
      USB_Global.Profile.pAPI->pfRecordEndCallU32(USBD_PROFILE_GET_EVENT_ID(EventId), (U32)ReturnValue);  \
    }
#else
#define USBD_PROFILE_END_CALL_U32(EventId, ReturnValue)
#endif


/*********************************************************************
*
*       USB descriptor defines.
*       Refer to [1]: chapter 9.6 "Standard USB Descriptor Definitions" for details.
*
*/
#define USB_DESC_TYPE_DEVICE           0x01
#define USB_DESC_TYPE_CONFIG           0x02
#define USB_DESC_TYPE_STRING           0x03
#define USB_DESC_TYPE_INTERFACE        0x04
#define USB_DESC_TYPE_ENDPOINT         0x05
#define USB_DESC_TYPE_QUALIFIER        0x06
#define USB_DESC_TYPE_SPEED_CONFIG     0x07
#define USB_DESC_TYPE_INTERFACE_POWER  0x08
#define USB_DESC_TYPE_IAD              0x0B
#define USB_DESC_TYPE_BOS              0x0F
#define USB_DESC_TYPE_CAPABILITY       0x10

#define USB_BOS_CAP_TYPE_WIRELESS_USB           0x01
#define USB_BOS_CAP_TYPE_USB20_EXTENSION        0x02
#define USB_BOS_CAP_TYPE_SUPER_SPEED            0x03
#define USB_BOS_CAP_TYPE_CONTAINER_ID           0x04
#define USB_BOS_CAP_TYPE_PLATFORM               0x05

#define USB_STORE_U16(u16) ((u16) & 255), ((u16) / 256)

/*********************************************************************
*
*       Internal functions.
*
**********************************************************************
*/
unsigned USB__CalcMaxPacketSize     (unsigned MaxPacketSize, U8 TransferType, U8 IsHighSpeedMode);
U8       USB__EPAddr2Index          (unsigned EPAddr);
U8       USB__EPIndex2Addr          (unsigned EPIndex);
void*    USB__GetpDest              (unsigned EPIndex,    unsigned NumBytes);
U32      USB__GetNextRX             (unsigned EPIndex, U8 **p);
void     USB__HandleSetup           (const USB_SETUP_PACKET * pSetupPacket);
void     USB__OnBusReset            (void);
void     USB__OnResume              (void);
void     USB__OnRx                  (unsigned EPIndex, const U8 * pData, unsigned NumBytes);
void     USB__OnRxZeroCopy          (unsigned EPIndex, unsigned NumBytes);
void     USB__OnRxZeroCopyEx        (unsigned EPIndex, unsigned NumBytes, int Done);
void     USB__OnSetupCancel         (void);
void     USB__OnStatusChange        (U8 NewState);
void     USB__OnSuspend             (void);
void     USB__OnTx                  (unsigned EPIndex);
void     USB__OnTx0Done             (void);
void     USB__Send                  (unsigned EPIndex);
void     USB__UpdateEPHW            (void);
int      USB__IsHighSpeedCapable    (void);
int      USB__IsHighSpeedMode       (void);
U8       USB__AllocIF               (void);
U8       USB__AllocAltIF            (U8 InterFaceNo);
U8       USB__AllocEvent            (void);
void     USB__InvalidateEP          (unsigned EPIndex);
void     USB__StallEP0              (void);
void     USB__ResetDataToggleEP     (unsigned EPIndex);
int      USB__AddDeInitHandler      (USB_DEINIT_FUNC * pfHandler);
void     USB__RemovePendingOperation(unsigned EPIndex, U8 SignalTask);
void     USB__EventDataSend         (unsigned EPIndex);
U8       USB__AllocStringDesc       (const char *sString);
void     USB__EmptyBuffer           (unsigned EPIndex);
U8   *   USB__GetDescBuffer         (unsigned * pDescBufferSize);

const U8 * USB__BuildConfigDesc     (void);
const U8 * USB__BuildDeviceDesc     (void);

//lint -sem(USB__EPIndex2Addr, pure)          N:100
//lint -sem(USB__IsHighSpeedCapable, pure)    N:100
//lint -sem(USB__IsHighSpeedMode, pure)       N:100

/*********************************************************************
*
*       InfoBuffer routines
*
**********************************************************************
*/
void USB_IB_Init                (USB_INFO_BUFFER * pInfoBuffer, U8 * pBuffer, unsigned SizeofBuffer);
void USB_IB_AddU8               (USB_INFO_BUFFER * pInfoBuffer, U8  Data);
void USB_IB_AddU16              (USB_INFO_BUFFER * pInfoBuffer, U16 Data);
void USB_IB_AddU32              (USB_INFO_BUFFER * pInfoBuffer, U32 Data);
void USB_IB_AddBytes            (USB_INFO_BUFFER * pInfoBuffer, const U8 * pData, U32 NumBytes);

/*********************************************************************
*
*       Buffer routines
*
**********************************************************************
*/
unsigned USBD_BUFFER_Read       (USB_BUFFER * pBuffer,       U8 * pData, unsigned NumBytesReq);
unsigned USBD_BUFFER_Write      (USB_BUFFER * pBuffer, const U8 * pData, unsigned NumBytes);

/*********************************************************************
*
*       Legacy
*
**********************************************************************
*/
#define  USB__DecRI()         USB_OS_DecRI()
#define  USB__IncDI()         USB_OS_IncDI()

#if defined(__cplusplus)
  }              /* Make sure we have C-declarations in C++ programs */
#endif

#endif // USB_PRIVATE_H

/*************************** End of file ****************************/
