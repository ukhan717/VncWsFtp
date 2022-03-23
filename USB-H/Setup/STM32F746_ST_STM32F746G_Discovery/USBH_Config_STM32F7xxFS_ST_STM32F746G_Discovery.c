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
File        : USBH_Config_STM32F7xxFS_ST_STM32F746G_Discovery.c
Purpose     : emUSB Host configuration file for the
              ST STM32F746G discovery eval board
---------------------------END-OF-HEADER------------------------------
*/

/*********************************************************************
*
*       #include Section
*
**********************************************************************
*/
#include <stdlib.h>
#include "USBH.h"
#include "BSP_USB.h"
#include "USBH_HW_STM32F7xxFS.h"

/*********************************************************************
*
*       Defines, configurable
*
**********************************************************************
*/
#define USB_ISR_ID    (67)
#define USB_ISR_PRIO  254
#define ALLOC_SIZE             0xf000      // Size of memory dedicated to the stack in bytes
#define STM32_OTG_BASE_ADDRESS 0x50000000UL

//
// RCC
//
#define RCC_BASE_ADDR             ((unsigned int)(0x40023800))
#define RCC_CR                    (*(volatile unsigned int*)(RCC_BASE_ADDR + 0x00))
#define RCC_AHB1RSTR              (*(volatile unsigned int*)(RCC_BASE_ADDR + 0x10))
#define RCC_AHB2RSTR              (*(volatile unsigned int*)(RCC_BASE_ADDR + 0x14))
#define RCC_AHB1ENR               (*(volatile unsigned int*)(RCC_BASE_ADDR + 0x30))
#define RCC_AHB2ENR               (*(volatile unsigned int*)(RCC_BASE_ADDR + 0x34))
#define RCC_PLLSAICFGR            (*(volatile unsigned int*)(RCC_BASE_ADDR + 0x88))
#define RCC_DCKCFGR2              (*(volatile unsigned int*)(RCC_BASE_ADDR + 0x90))
//
// GPIO
//
#define GPIOA_BASE_ADDR           ((unsigned int)0x40020000)
#define GPIOA_MODER               (*(volatile unsigned int*)(GPIOA_BASE_ADDR + 0x00))
#define GPIOA_AFRL                (*(volatile unsigned int*)(GPIOA_BASE_ADDR + 0x20))
#define GPIOA_AFRH                (*(volatile unsigned int*)(GPIOA_BASE_ADDR + 0x24))

#define GPIOD_BASE_ADDR           ((unsigned int)0x40020C00)
#define GPIOD_MODER               (*(volatile unsigned int*)(GPIOD_BASE_ADDR + 0x00))
#define GPIOD_BSRR                (*(volatile unsigned int*)(GPIOD_BASE_ADDR + 0x18))
#define GPIOD_AFRL                (*(volatile unsigned int*)(GPIOD_BASE_ADDR + 0x20))
#define GPIOD_AFRH                (*(volatile unsigned int*)(GPIOD_BASE_ADDR + 0x24))

#define OTG_FS_GOTGCTL            (*(volatile unsigned int*)(0x50000000))

#define OTG_FS_GOTTGCTL_AVALOVAL  (1UL << 5)  // A-peripheral session valid override value
#define OTG_FS_GOTTGCTL_AVALOEN   (1UL << 4)  // A-peripheral session valid override enable
#define OTG_FS_GOTTGCTL_VBVALOVAL (1UL << 3)  // VBUS valid override value.
#define OTG_FS_GOTTGCTL_VBVALOEN  (1UL << 2)  // VBUS valid override enable.

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
static U32 _aPool [((ALLOC_SIZE) / 4)];

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

/*********************************************************************
*
*       _InitUSBHw
*
*/
static void _InitUSBHw(void) {
  U32 v;

  //
  // Configure the 48 Mhz Clock, we assume to have a 25 MHz crystal + PLLM Divider of 25
  // With this we can configure the PLLSAI to 192 MHz in order to get a proper 48 Mhz out of it.
  //
  RCC_CR &= ~(1UL << 28);      // Disable PLLSAI
  while ((RCC_CR & (1UL << 29)) != 0);   // Wait until the PLLSAI is disabled
  //
  // Update the PLLSAI Configuration register:
  // Use N = 192; P = 4, which result in Freq[in] * N / P = 1 MHz * 192 / 4 = 48 MHz
  // which is the clock that we need.
  // Q and R are not touched these value are needed for other peripherals.
  //
  v = RCC_PLLSAICFGR;
  v &= ~((0x1FFUL << 6) | (0x3UL << 16));  // Clear bits
  v |= (192 << 6) | (1 << 16);
  RCC_PLLSAICFGR = v;
  RCC_CR |= (1UL << 28);      // Enable PLLSAI
  while ((RCC_CR & (1UL << 29)) == 0);   // Wait until the PLLSAI is ready
  //
  // Use PLLSAI as CLK48 clock source
  //
  RCC_DCKCFGR2 |= (1U << 27);
  //
  // Set the dedicated port pins
  //
  RCC_AHB1ENR |= 0
               | (1 <<  3)  // GPIOCEN: IO port D clock enable
               | (1 <<  0)  // GPIOAEN: IO port A clock enable
               ;
  RCC_AHB2ENR |= 0
              | (1 <<  7)  // OTGFSEN: Enable USB OTG FS clock enable
              ;
  //
  // Set PA10 (OTG_FS_ID) as alternate function
  //
  v           = GPIOA_MODER;
  v          &= ~(0x3uL << (2 * 10));
  v          |=  (0x2uL << (2 * 10));
  GPIOA_MODER = v;
  v           = GPIOA_AFRH;
  v          &= ~(0xFuL << (4 * 2));
  v          |=  (0xAuL << (4 * 2));
  GPIOA_AFRH  = v;
  //
  // Set PA11 (OTG_FS_DM) as alternate function
  //
  v           = GPIOA_MODER;
  v          &= ~(0x3uL << (2 * 11));
  v          |=  (0x2uL << (2 * 11));
  GPIOA_MODER = v;
  v           = GPIOA_AFRH;
  v          &= ~(0xFuL << (4 * 3));
  v          |=  (0xAuL << (4 * 3));
  GPIOA_AFRH  = v;
  //
  // Set PA12 (OTG_FS_DP) as alternate function
  //
  v           = GPIOA_MODER;
  v          &= ~(0x3uL << (2 * 12));
  v          |=  (0x2uL << (2 * 12));
  GPIOA_MODER = v;
  v           = GPIOA_AFRH;
  v          &= ~(0xFuL << (4 * 4));
  v          |=  (0xAuL << (4 * 4));
  GPIOA_AFRH  = v;
  //
  // Set PD5 (OTG_FS_PowerSwitchOn) as general purpose output mode
  //
  v           = GPIOD_MODER;
  v          &= ~(0x3uL << (2 * 5));
  v          |=  (0x1uL << (2 * 5));
  GPIOD_BSRR  = ((1ul << 5) <<  0); // Set pin high to disable VBUS.
  GPIOD_MODER = v;
  OTG_FS_GOTGCTL |= 0
                 |  OTG_FS_GOTTGCTL_AVALOVAL
                 |  OTG_FS_GOTTGCTL_AVALOEN
                 |  OTG_FS_GOTTGCTL_VBVALOVAL
                 |  OTG_FS_GOTTGCTL_VBVALOEN;
}

/*********************************************************************
*
*       _OnPortPowerControl
*/
static void _OnPortPowerControl(U32 HostControllerIndex, U8 Port, U8 PowerOn) {
  USBH_USE_PARA(HostControllerIndex);
  USBH_USE_PARA(Port);
  if (PowerOn) {
    GPIOD_BSRR  = ((1ul << 5) << 16); // Set pin low to enable VBUS.
  } else {
    GPIOD_BSRR  = ((1ul << 5) <<  0); // Set pin high to disable VBUS.
  }
}

/*********************************************************************
*
*       _ISR
*
*  Function description
*/
static void _ISR(void) {
  USBH_ServiceISR(0);
}

/*********************************************************************
*
*       USBH_X_Config
*
*  Function description
*/
void USBH_X_Config(void) {
  USBH_AssignMemory(&_aPool[0], ALLOC_SIZE);    // Assigning memory should be the first thing
  // USBH_ConfigSupportExternalHubs (1);           // Default values: The hub module is disabled, this is done to save memory.
  // USBH_ConfigPowerOnGoodTime     (300);         // Default values: 300 ms wait time before the host starts communicating with a device.
  //
  // Define log and warn filter
  // Note: The terminal I/O emulation affects the timing
  // of your communication, since the debugger stops the target
  // for every terminal I/O unless you use RTT!
  //
  USBH_SetWarnFilter(0xFFFFFFFF);               // 0xFFFFFFFF: Do not filter: Output all warnings.
  USBH_SetLogFilter(0
                    | USBH_MTYPE_INIT
                    | USBH_MTYPE_APPLICATION
                    );
  _InitUSBHw();
  USBH_STM32F7_FS_Add((void*)STM32_OTG_BASE_ADDRESS);
  //
  //  Please uncomment this function when using OTG functionality.
  //  Otherwise the VBUS power-on will be permanently on and will cause
  //  OTG to detect a session where no session is available.
  //
  // USBH_SetOnSetPortPower(_OnPortPowerControl);                 // This function sets a callback which allows to control VBUS-Power of a USB port.
  _OnPortPowerControl(0, 0, 1);                                // Enable power on USB port
  BSP_USBH_InstallISR_Ex(USB_ISR_ID, _ISR, USB_ISR_PRIO);
}
/*************************** End of file ****************************/
