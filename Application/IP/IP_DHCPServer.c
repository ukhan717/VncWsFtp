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
File    : IP_DHCPServer.c
Purpose : Sample program for embOS & embOS/IP
          The sample will setup a DHCP server that can assign a network
          configuration to a client requesting one. A DHCP server shall
          be used with a static IP addr. only and shall not request its
          own IP addr. from another DHCP server.

          The following is a sample of the output to the terminal window:

          0:000 MainTask - INIT: Init started. Version 2.13.11
          0:002 MainTask - DRIVER: Found PHY with Id 0x181 at addr 0x1F
          0:003 MainTask - INIT: Link is down
          0:003 MainTask - INIT: Init completed
          0:003 IP_Task - INIT: IP_Task started
          3:000 IP_Task - LINK: Link state changed: Full duplex, 100 MHz
          8:250 IP_Task - DHCPs: DISCOVER from 00:26:9E:E2:C8:44 on IFace 0
          8:250 IP_Task - DHCPs: Lease time [s] requested 0, granted 60
          8:250 IP_Task - DHCPs: OFFER 192.168.12.11
          8:252 IP_Task - DHCPs: REQUEST from 00:26:9E:E2:C8:44 on IFace 0
          8:252 IP_Task - DHCPs: Requested 192.168.12.11
          8:253 IP_Task - DHCPs: Lease time [s] requested 0, granted 60
          8:253 IP_Task - DHCPs: ACK
Notes   : For compatibility with interfaces that need to connect in
          any way this sample calls connect and disconnect routines
          that might not be needed in all cases.

          This sample can be used for Ethernet and dial-up interfaces
          and is configured to use the last registered interface as
          its main interface.
--------  END-OF-HEADER  ---------------------------------------------
*/

#include "RTOS.h"
#include "BSP.h"
#include "IP.h"

/*********************************************************************
*
*       Configuration
*
**********************************************************************
*/

#define USE_RX_TASK          0  // 0: Packets are read in ISR, 1: Packets are read in a task of its own.

//
// DHCP server sample configuration, mandatory.
//
#define IP_POOL_START_ADDR   IP_BYTES2ADDR(192, 168, 12, 11)  // First addr. of IP pool to assign to clients.
#define IP_POOL_SUBNET_MASK  0xFFFF0000                       // Subnet mask assigned to clients.
#define IP_POOL_SIZE         20                               // Number of IP addr. in pool starting from IP_POOL_START_ADDR .

//
// DHCP server sample configuration, optional.
//
#define DNS_ADDR_0      IP_BYTES2ADDR(192, 168, 12, 1)  // First DNS server to offer to client.
#define DNS_ADDR_1      IP_BYTES2ADDR(192, 168, 12, 2)  // Second DNS server to offer to client.
#define GW_ADDR         IP_BYTES2ADDR(192, 168, 12, 1)  // Gateway to offer to client.
#define MAX_LEASE_TIME  60                              // Max lease time override [s]. Default 2h.

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
#ifdef DNS_ADDR_0
  U32 aDNSAddr[] = {
     DNS_ADDR_0
#ifdef DNS_ADDR_1
    ,DNS_ADDR_1
#endif
  };
#endif

  IP_Init();
  _IFaceId = IP_INFO_GetNumInterfaces() - 1;                                             // Get the last registered interface ID as this is most likely the interface we want to use in this sample.
  OS_SetPriority(OS_GetTaskID(), TASK_PRIO_IP_TASK);                                     // For now, this task has highest prio except IP management tasks.
  OS_CREATETASK(&_IPTCB  , "IP_Task"  , IP_Task  , TASK_PRIO_IP_TASK   , _IPStack);      // Start the IP_Task.
#if USE_RX_TASK
  OS_CREATETASK(&_IPRxTCB, "IP_RxTask", IP_RxTask, TASK_PRIO_IP_RX_TASK, _IPRxStack);    // Start the IP_RxTask, optional.
#endif
  IP_AddStateChangeHook(&_StateChangeHook, _OnStateChange);                              // Register hook to be notified on disconnects.
  //
  // Do not wait for interface ready as a DHCP server
  // shall only be used with a static IP configured.
  //
  IP_DHCPS_ConfigPool(_IFaceId, IP_POOL_START_ADDR, IP_POOL_SUBNET_MASK, IP_POOL_SIZE);  // Setup IP pool to distribute.
#ifdef GW_ADDR
  IP_DHCPS_ConfigGWAddr(_IFaceId, GW_ADDR);                                              // Configure default GW addr. if any. Optional.
#endif
#ifdef DNS_ADDR_0
  IP_DHCPS_ConfigDNSAddr(_IFaceId, &aDNSAddr[0], SEGGER_COUNTOF(aDNSAddr));              // Configure DNS addr. if any. Optional.
#endif
#ifdef MAX_LEASE_TIME
  IP_DHCPS_ConfigMaxLeaseTime(_IFaceId, MAX_LEASE_TIME);                                 // Configure maximum lease time to grant. Default is 2h.
#endif
  IP_DHCPS_Init(_IFaceId);                                                               // Initialize server.
  IP_DHCPS_Start(_IFaceId);                                                              // Start server.
  while (1) {
    BSP_ToggleLED(1);
    OS_Delay(200);
  }
}

/****** End Of File *************************************************/
