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
File        : IP_Config_USBH_LAN.c
Purpose     : Configuration file for TCP/IP with emUSB-Host LAN driver
---------------------------END-OF-HEADER------------------------------
*/
#include <stdio.h>
#include "BSP.h"
#include "SEGGER.h"
#include "IP.h"
#include "RTOS.h"
#include "USBH.h"
#include "USBH_LAN.h"


/*********************************************************************
*
*       Local defines, configurable
*
**********************************************************************
*/
#define USE_DHCP        1                           // Use DHCP client or static IP configuration.
#define NUM_INSTANCES   1

#if NUM_INSTANCES > 1
  typedef struct _MANUAL_CONFIG_IP {
    U32 IPAddr;
    U32 SubnetMask;
    U32 GatewayAddr;
    U32 DNSAddr;
  } MANUAL_CONFIG_IP;

  //
  // If you are using more than 2 interface and you would like to setup the interfaces manually please add the additional entries here
  //
  static const MANUAL_CONFIG_IP _aIPPool[NUM_INSTANCES] = {
    { IP_BYTES2ADDR(192, 168,   2, 252), IP_BYTES2ADDR(255, 255, 255,   0), IP_BYTES2ADDR(192, 168,   2,   1), IP_BYTES2ADDR(192, 168,   2,   1) }
    { IP_BYTES2ADDR(192, 168,   2, 253), IP_BYTES2ADDR(255, 255, 255,   0), IP_BYTES2ADDR(192, 168,   2,   1), IP_BYTES2ADDR(192, 168,   2,   1) }
  };
  #define IP_ADDR(x)      _aIPPool[x].IPAddr
  #define SUBNET_MASK(x)  _aIPPool[x].SubnetMask
  #define GW_ADDR(x)      _aIPPool[x].GatewayAddr
  #define DNS_ADDR(x)     _aIPPool[x].DNSAddr
#else
  //
  // The following parameters are only used when the DHCP client is not active.
  //
  #define IP_ADDR(x)      IP_BYTES2ADDR(192, 168,   2, 252)
  #define SUBNET_MASK(x)  IP_BYTES2ADDR(255, 255, 255,   0)
  #define GW_ADDR(x)      IP_BYTES2ADDR(192, 168,   2,   1)
  #define DNS_ADDR(x)     IP_BYTES2ADDR(192, 168,   2,   1)
#endif
//
// The serial define can be manipulated for each individual target.
// This allows multiple targets to be connected to the same PC.
//
// In the default configuration of this sample the target would be available
// through the URL usbh.local
//
#define SERVER_NAME                 "usbh"

/*********************************************************************
*
*       Local Types
*
**********************************************************************
*/
enum {
  TASK_PRIO_USBH_MAIN = 200,
  TASK_PRIO_USBH_ISR
};

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
#define USE_MEM_SECTIONS 1
#if USE_MEM_SECTIONS
  #ifdef __ICCARM__          // IAR ARM compiler
    #define PRAGMA(x)         _Pragma(#x)
    #define SECTION(x)                  PRAGMA(location=x)
    #define SECTION_NO_INIT(x)          PRAGMA(location=x)   __no_init
    #define SECTION_NO_INIT_STATIC(x)   PRAGMA(location=x)   static __no_init
  #elif (defined __CC_ARM)   // Keil/ARM compiler
    #define SECTION(x)                  __attribute__ ((section (x)))
    #define SECTION_NO_INIT(x)          __attribute__ ((section (x), zero_init))
    #define SECTION_NO_INIT_STATIC(x)   __attribute__ ((section (x), zero_init)) static
    #define ___WEAK                     __attribute__ ((weak))
  #elif (defined __GNUC__)   // GNU ARM compiler
    #define SECTION(x)                  __attribute__ ((section (x)))
    #define SECTION_NO_INIT(x)          __attribute__ ((section (x)))
    #define SECTION_NO_INIT_STATIC(x)   __attribute__ ((section (x))) static
  #else
    #define SECTION(x)
    #define SECTION_NO_INIT(x)
    #define SECTION_NO_INIT_STATIC(x)   static
  #endif
#else
  #define SECTION(x)
  #define SECTION_NO_INIT(x)
  #define SECTION_NO_INIT_STATIC(x)   static
#endif

OS_STACKPTR int     _StackMain[2048/sizeof(int)];
OS_TASK             _TCBMain;

OS_STACKPTR int     _StackIsr[1536/sizeof(int)];
OS_TASK             _TCBIsr;

#define ALLOC_SIZE                  0x6000                    // Size of memory dedicated to the stack in bytes
SECTION_NO_INIT_STATIC("USB_RAM") U32 _aPool[ALLOC_SIZE / 4];            // This is the memory area used by the IP stack.
static IP_DNS_SERVER_SD_CONFIG _aSDConfig[] = {
  //
  // Secondary entry, main entry is automatically created through _DNSConfig.sHostname
  //
  { .Type = IP_DNS_SERVER_TYPE_A,
    .TTL = 0,
    .Config.A = {
      .sName  = SERVER_NAME,
      .IPAddr = 0, // Set automatically.
    }
  },
  #if IP_SUPPORT_IPV6
  { .Type = IP_DNS_SERVER_TYPE_AAAA,
    .TTL = 0,
    .Config.AAAA = {
      .sName = SERVER_NAME,
      .aIPAddrV6 = {0}, // Set automatically.
    }
  }
  #endif
};
static IP_DNS_SERVER_CONFIG  _DNSConfig;

/*********************************************************************
*
*       IP_X_Config
*
*  Function description
*    This function is called by the IP stack during IP_Init().
*
*  Typical memory/buffer configurations:
*    Microcontroller system, size optimized
*      #define ALLOC_SIZE 0x3000                    // 12KBytes RAM
*      mtu = 576;                                   // 576 is minimum acc. to RFC, 1500 is max. for Ethernet
*      IP_SetMTU(0, mtu);                           // Maximum Transmission Unit is 1500 for ethernet by default
*      IP_AddBuffers(8, 256);                       // Small buffers.
*      IP_AddBuffers(4, mtu + 16);                  // Big buffers. Size should be mtu + 16 byte for ethernet header (2 bytes type, 2*6 bytes MAC, 2 bytes padding)
*      IP_ConfTCPSpace(1 * (mtu-40), 1 * (mtu-40)); // Define the TCP Tx and Rx window size
*
*    Microcontroller system, speed optimized or multiple connections
*      #define ALLOC_SIZE 0x6000                    // 24 KBytes RAM
*      mtu = 1500;                                  // 576 is minimum acc. to RFC, 1500 is max. for Ethernet
*      IP_SetMTU(0, mtu);                           // Maximum Transmission Unit is 1500 for ethernet by default
*      IP_AddBuffers(12, 256);                      // Small buffers.
*      IP_AddBuffers(6, mtu + 16);                  // Big buffers. Size should be mtu + 16 byte for ethernet header (2 bytes type, 2*6 bytes MAC, 2 bytes padding)
*      IP_ConfTCPSpace(3 * (mtu-40), 3 * (mtu-40)); // Define the TCP Tx and Rx window size
*
*    System with lots of RAM
*      #define ALLOC_SIZE 0x20000                   // 128 KBytes RAM
*      mtu = 1500;                                  // 576 is minimum acc. to RFC, 1500 is max. for Ethernet
*      IP_SetMTU(0, mtu);                           // Maximum Transmission Unit is 1500 for ethernet by default
*      IP_AddBuffers(50, 256);                      // Small buffers.
*      IP_AddBuffers(50, mtu + 16);                 // Big buffers. Size should be mtu + 16 byte for ethernet header (2 bytes type, 2*6 bytes MAC, 2 bytes padding)
*      IP_ConfTCPSpace(5 * (mtu-40), 5 * (mtu-40)); // Define the TCP Tx and Rx window size
*/
void IP_X_Config(void) {
  int mtu;
  int aIFaceId[NUM_INSTANCES];
  unsigned i;

  IP_AssignMemory(_aPool, sizeof(_aPool));          // Assigning memory should be the first thing
  IP_ConfigMaxIFaces(NUM_INSTANCES);
  for (i = 0; i < NUM_INSTANCES; i++) {
    aIFaceId[i] = IP_AddEtherInterface(&IP_Driver_USBH);          // Add driver for your hardware.
    //
    // Run-time configure buffers.
    // The default setup will do for most cases.
    //
    mtu = 1500;                                     // 576 is minimum acc. to RFC, 1500 is max. for Ethernet
    IP_SetMTU(aIFaceId[i], mtu);                    // Maximum Transmission Unit is 1500 for ethernet by default
    //
    // Use DHCP client or define IP address, subnet mask,
    // gateway address and DNS server according to the
    // requirements of your application.
    //
#if USE_DHCP
    {
      char aDevName[10] = {"USBH_ETHx"};
      aDevName[8] = '0' + i;
      IP_DHCPC_Activate(aIFaceId[i], aDevName, NULL, NULL);  // Activate DHCP client.
    }
#else
    IP_SetAddrMaskEx(aIFaceId[i], IP_ADDR(i), SUBNET_MASK(i));      // Assign IP addr. and subnet mask.
    IP_SetGWAddr(aIFaceId[i], GW_ADDR(i));                       // Set gateway addr.
    IP_DNS_SetServer(DNS_ADDR(i));                               // Set DNS server addr.
#endif
  }
  USBH_Init();
  OS_CREATETASK(&_TCBMain, "USBH_Task", USBH_Task, TASK_PRIO_USBH_MAIN, _StackMain);   // Start USBH main task
  OS_CREATETASK(&_TCBIsr, "USBH_isr", USBH_ISRTask, TASK_PRIO_USBH_ISR, _StackIsr);    // Start USBH ISR task
  USBH_LAN_Init();
  USBH_LAN_RegisterDriver(&USBH_LAN_DRIVER_ASIX);
  USBH_LAN_RegisterDriver(&USBH_LAN_DRIVER_ECM);
  USBH_LAN_RegisterDriver(&USBH_LAN_DRIVER_RNDIS);
  IP_MEMSET(&_DNSConfig, 0, sizeof(_DNSConfig));
  _DNSConfig.sHostname    = SERVER_NAME;
  _DNSConfig.TTL          = 60;
  _DNSConfig.apSDConfig   = _aSDConfig;
  _DNSConfig.NumConfig    = SEGGER_COUNTOF(_aSDConfig);
  IP_MDNS_SERVER_Start(&_DNSConfig);
  IP_DNS_SERVER_Start(&_DNSConfig);
  IP_AddBuffers(12, 256);                           // Small buffers.
  IP_AddBuffers(6, mtu + 16);                       // Big buffers. Size should be mtu + 16 byte for ethernet header (2 bytes type, 2*6 bytes MAC, 2 bytes padding)
  IP_ConfTCPSpace(3 * (mtu - 40), 3 * (mtu - 40));  // Define the TCP Tx and Rx window size
  IP_SOCKET_SetDefaultOptions(0
//                              | SO_TIMESTAMP   // Send TCP timestamp to optimize the round trip time measurement. Normally not used in LAN.
                              | SO_KEEPALIVE   // Enable keepalives by default for TCP sockets.
                             );
  //
  // Define log and warn filter.
  // Note: The terminal I/O emulation might affect the timing of your
  //       application, since most debuggers need to stop the target
  //       for every terminal I/O output unless you use another
  //       implementation such as DCC or SWO.
  //
  IP_SetWarnFilter(0xFFFFFFFF);              // 0xFFFFFFFF: Do not filter: Output all warnings.
  IP_SetLogFilter(0
                  | IP_MTYPE_APPLICATION     // Output application messages.
                  | IP_MTYPE_INIT            // Output all messages from init.
                  | IP_MTYPE_LINK_CHANGE     // Output a message if link status changes.
                  | IP_MTYPE_PPP             // Output all PPP/PPPoE related messages.
                  | IP_MTYPE_DHCP            // Output general DHCP status messages.
#if IP_SUPPORT_IPV6
                  | IP_MTYPE_IPV6            // Output IPv6 address related messages
#endif
//                  | IP_MTYPE_DHCP_EXT        // Output additional DHCP messages.
//                  | IP_MTYPE_CORE            // Output log messages from core module.
//                  | IP_MTYPE_ALLOC           // Output log messages for memory allocation.
//                  | IP_MTYPE_DRIVER          // Output log messages from driver.
//                  | IP_MTYPE_ARP             // Output log messages from ARP layer.
//                  | IP_MTYPE_IP              // Output log messages from IP layer.
//                  | IP_MTYPE_TCP_CLOSE       // Output a log messages if a TCP connection has been closed.
//                  | IP_MTYPE_TCP_OPEN        // Output a log messages if a TCP connection has been opened.
//                  | IP_MTYPE_TCP_IN          // Output TCP input logs.
//                  | IP_MTYPE_TCP_OUT         // Output TCP output logs.
//                  | IP_MTYPE_TCP_RTT         // Output TCP round trip time (RTT) logs.
//                  | IP_MTYPE_TCP_RXWIN       // Output TCP RX window related log messages.
//                  | IP_MTYPE_TCP             // Output all TCP related log messages.
//                  | IP_MTYPE_UDP_IN          // Output UDP input logs.
//                  | IP_MTYPE_UDP_OUT         // Output UDP output logs.
//                  | IP_MTYPE_UDP             // Output all UDP related messages.
//                  | IP_MTYPE_ICMP            // Output ICMP related log messages.
//                  | IP_MTYPE_NET_IN          // Output network input related messages.
//                  | IP_MTYPE_NET_OUT         // Output network output related messages.
//                  | IP_MTYPE_DNS             // Output all DNS related messages.
//                  | IP_MTYPE_SOCKET_STATE    // Output socket status messages.
//                  | IP_MTYPE_SOCKET_READ     // Output socket read related messages.
//                  | IP_MTYPE_SOCKET_WRITE    // Output socket write related messages.
//                  | IP_MTYPE_SOCKET          // Output all socket related messages.
                 );
  //
  // Add protocols to the stack.
  //
  IP_TCP_Add();
  IP_UDP_Add();
  IP_ICMP_Add();
#if IP_SUPPORT_IPV6
{
  unsigned  OptToDisable;

  OptToDisable = 0
               | IPV6_GENERATE_LINK_LOCAL_ADDR  // By default, at least one link local address will be generated. Can be used to disable link local address generation.
//               | IPV6_ICMPV6_MLD_ADD_DEF_ADDR   // By default, the multicast addresses all-nodes and all-routers will be added automatically. Can be used to disable this.
//               | IPV6_USE_SLAAC                 // By default, SLAAC is enabled. Can be used to disable it.
//               | IPV6_USE_ROUTER_ADVERTISMENTS  // By default, Router Advertisements are processed and supplied options used for configuration. Can be used to disable this behavior.
               ;

  for (i = 0; i < NUM_INSTANCES; i++) {
    IP_IPV6_ChangeDefaultConfig(aIFaceId[i], OptToDisable,  0);
    IP_IPV6_Add(aIFaceId[i]);
  }
}
#endif
}


/*************************** End of file ****************************/
