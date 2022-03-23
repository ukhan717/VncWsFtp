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
File    : USB_Config_ST_STM32F746G-DSCO_OTG-FS.c
Purpose : Config file for STM32F746G-DISCO (MB1191B).
--------  END-OF-HEADER  ---------------------------------------------
*/

#include "USB.h"
#include "BSP_USB.h"

/*********************************************************************
*
*       Defines, configurable
*
**********************************************************************
*/
#define USB_ISR_ID    (67)
#define USB_ISR_PRIO  254

/*********************************************************************
*
*       Defines, sfrs
*
**********************************************************************
*/

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

#define OTG_FS_GOTGCTL            (*(volatile unsigned int*)(0x50000000))

#define OTG_FS_GOTTGCTL_BVALOVAL (1UL << 7)  // B-peripheral session valid override value
#define OTG_FS_GOTTGCTL_BVALOEN  (1UL << 6)  // B-peripheral session valid override enable


/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _EnableISR
*/
static void _EnableISR(USB_ISR_HANDLER * pfISRHandler) {
  BSP_USB_InstallISR_Ex(USB_ISR_ID, pfISRHandler, USB_ISR_PRIO);
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       Setup which target USB driver shall be used
*/

/*********************************************************************
*
*       USBD_X_Config
*/
void USBD_X_Config(void) {
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
  RCC_AHB1ENR |= 0
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
  // Override the B-Session valid operation from the USB PHY
  // and force to be in USB Device mode.
  //
  OTG_FS_GOTGCTL |= 0
                 |  OTG_FS_GOTTGCTL_BVALOVAL
                 |  OTG_FS_GOTTGCTL_BVALOEN
                 ;
  USBD_AddDriver(&USB_Driver_ST_STM32F7xxFS);
  USBD_SetISRMgmFuncs(_EnableISR, USB_OS_IncDI, USB_OS_DecRI);
}

/*************************** End of file ****************************/
