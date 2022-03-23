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
File       : FS_MMC_CM_HW_STM32F746_ST_STM32F746G-DISCO.c
Purpose    : Hardware layer for MMC/SD driver in card mode ST STM32F746G-DISCO eval board.
Literature :
  [1] RM0385 Reference manual STM32F75xxx and STM32F74xxx advanced ARM-based 32-bit MCUs
    (\\FILESERVER\Techinfo\Company\ST\MCU\STM32\STM32F7\RM0385_STM32F74xxx_STM32F75xxx_Rev5_DM00124865.pdf)
  [2] STM32F74xxx STM32F75xxx Errata sheet
    (\\FILESERVER\Techinfo\Company\ST\MCU\STM32\STM32F7\STM32F74xxx_STM32F75xxx_Errata_Rev3.pdf)
  [3] UM1907 User manual Discovery kit for STM32F7 Series with STM32F746NG MCU
    (\\FILESERVER\Techinfo\Company\ST\MCU\STM32\STM32F7\EvalBoard\STM32F746_Discovery\DM00190424.pdf)
---------------------------END-OF-HEADER------------------------------
*/

/*********************************************************************
*
*       #include section
*
**********************************************************************
*/
#include <stddef.h>
#include "FS.h"
#include "MMC_SD_CardMode_X_HW.h"
#include "stm32f7xx.h"

/*********************************************************************
*
*       Defines, configurable
*
**********************************************************************
*/
#define SDMMC_CLK_KHZ       48000       // Clock of SDMMC unit
#define MAX_SD_CLK_KHZ      48000       // Maximum transfer speed
#define USE_OS              0           // Selects the operating mode: 1 - event-driven, 0 - polling
#define POWER_GOOD_DELAY    50          // Number of  milliseconds to wait for the power supply of SD card to become ready
#define ALIGNED_DMA_BURSTS  0           // When set to 1 the DMA burst is aligned to the address and the number of bytes to be transferred.
#define HCLK                216000000
#define PCLK2               (HCLK / 2)

/*********************************************************************
*
*       #include section, conditional
*
**********************************************************************
*/
#if USE_OS
  #include "RTOS.h"
#endif

/*********************************************************************
*
*       Defines, non-configurable
*
**********************************************************************
*/

/*********************************************************************
*
*       SDMMC interface registers
*/
#define SDMMC_BASE_ADDR         0x40012C00uL
#define SDMMC_POWER             (*(volatile U32 *)(SDMMC_BASE_ADDR + 0x00))
#define SDMMC_CLKCR             (*(volatile U32 *)(SDMMC_BASE_ADDR + 0x04))
#define SDMMC_ARG               (*(volatile U32 *)(SDMMC_BASE_ADDR + 0x08))
#define SDMMC_CMD               (*(volatile U32 *)(SDMMC_BASE_ADDR + 0x0C))
#define SDMMC_RESPCMD           (*(volatile U32 *)(SDMMC_BASE_ADDR + 0x10))
#define SDMMC_RESP1             (*(volatile U32 *)(SDMMC_BASE_ADDR + 0x14))
#define SDMMC_RESP2             (*(volatile U32 *)(SDMMC_BASE_ADDR + 0x18))
#define SDMMC_RESP3             (*(volatile U32 *)(SDMMC_BASE_ADDR + 0x1C))
#define SDMMC_RESP4             (*(volatile U32 *)(SDMMC_BASE_ADDR + 0x20))
#define SDMMC_DTIMER            (*(volatile U32 *)(SDMMC_BASE_ADDR + 0x24))
#define SDMMC_DLEN              (*(volatile U32 *)(SDMMC_BASE_ADDR + 0x28))
#define SDMMC_DCTRL             (*(volatile U32 *)(SDMMC_BASE_ADDR + 0x2C))
#define SDMMC_DCOUNT            (*(volatile U32 *)(SDMMC_BASE_ADDR + 0x30))
#define SDMMC_STA               (*(volatile U32 *)(SDMMC_BASE_ADDR + 0x34))
#define SDMMC_ICR               (*(volatile U32 *)(SDMMC_BASE_ADDR + 0x38))
#define SDMMC_MASK              (*(volatile U32 *)(SDMMC_BASE_ADDR + 0x3C))
#define SDMMC_FIFOCNT           (*(volatile U32 *)(SDMMC_BASE_ADDR + 0x48))
#define SDMMC_FIFO              (*(volatile U32 *)(SDMMC_BASE_ADDR + 0x80))

/*********************************************************************
*
*       Reset and clock control registers
*/
#define RCC_BASE_ADDR           0x40023800uL
#define RCC_AHB1RSTR            (*(volatile U32 *)(RCC_BASE_ADDR + 0x10))
#define RCC_APB2RSTR            (*(volatile U32 *)(RCC_BASE_ADDR + 0x24))
#define RCC_AHB1ENR             (*(volatile U32 *)(RCC_BASE_ADDR + 0x30))
#define RCC_APB2ENR             (*(volatile U32 *)(RCC_BASE_ADDR + 0x44))

/*********************************************************************
*
*       Port C registers
*/
#define GPIOC_BASE_ADDR         0x40020800uL
#define GPIOC_MODER             (*(volatile U32 *)(GPIOC_BASE_ADDR + 0x00))
#define GPIOC_OSPEEDR           (*(volatile U32 *)(GPIOC_BASE_ADDR + 0x08))
#define GPIOC_PUPDR             (*(volatile U32 *)(GPIOC_BASE_ADDR + 0x0C))
#define GPIOC_IDR               (*(volatile U32 *)(GPIOC_BASE_ADDR + 0x10))
#define GPIOC_AFRL              (*(volatile U32 *)(GPIOC_BASE_ADDR + 0x20))
#define GPIOC_AFRH              (*(volatile U32 *)(GPIOC_BASE_ADDR + 0x24))

/*********************************************************************
*
*       Port D registers
*/
#define GPIOD_BASE_ADDR         0x40020C00uL
#define GPIOD_MODER             (*(volatile U32 *)(GPIOD_BASE_ADDR + 0x00))
#define GPIOD_OSPEEDR           (*(volatile U32 *)(GPIOD_BASE_ADDR + 0x08))
#define GPIOD_PUPDR             (*(volatile U32 *)(GPIOD_BASE_ADDR + 0x0C))
#define GPIOD_AFRL              (*(volatile U32 *)(GPIOD_BASE_ADDR + 0x20))

/*********************************************************************
*
*       DMA 2 registers
*/
#define DMA2_BASE_ADDR          0x40026400uL
#define DMA2_LISR               (*(volatile U32 *)(DMA2_BASE_ADDR + 0))
#define DMA2_HISR               (*(volatile U32 *)(DMA2_BASE_ADDR + 4))
#define DMA2_LIFCR              (*(volatile U32 *)(DMA2_BASE_ADDR + 8))
#define DMA2_HIFCR              (*(volatile U32 *)(DMA2_BASE_ADDR + 12))
#define DMA2_S3CR               (*(volatile U32 *)(DMA2_BASE_ADDR + 24 * 3 + 16))
#define DMA2_S3NDTR             (*(volatile U32 *)(DMA2_BASE_ADDR + 24 * 3 + 20))
#define DMA2_S3PAR              (*(volatile U32 *)(DMA2_BASE_ADDR + 24 * 3 + 24))
#define DMA2_S3M0AR             (*(volatile U32 *)(DMA2_BASE_ADDR + 24 * 3 + 28))
#define DMA2_S3FCR              (*(volatile U32 *)(DMA2_BASE_ADDR + 24 * 3 + 36))

/*********************************************************************
*
*       System Control Block
*/
#define SCB_BASE_ADDR           0xE000E000uL
#define SCB_CCR                 (*(volatile U32 *)(SCB_BASE_ADDR + 0x014))    // Configuration Control Register

/*********************************************************************
*
*       Reset and clock bits for the peripherals used by the driver
*/
#define AHB1ENR_DMA2EN          (1uL << 22)
#define AHB1ENR_PORTCEN         (1uL << 2)
#define AHB1ENR_PORTDEN         (1uL << 3)
#define APB2ENR_SDMMCEN         (1uL << 11)
#define APB2RSTR_SDMMCRST       (1uL << 11)

/*********************************************************************
*
*       SDMMC status register
*/
#define STA_CCRCFAIL            (1uL << 0)
#define STA_DCRCFAIL            (1uL << 1)
#define STA_CTIMEOUT            (1uL << 2)
#define STA_DTIMEOUT            (1uL << 3)
#define STA_TXUNDERR            (1uL << 4)
#define STA_RXOVERR             (1uL << 5)
#define STA_CMDREND             (1uL << 6)
#define STA_CMDSENT             (1uL << 7)
#define STA_DATAEND             (1uL << 8)
#define STA_STBITERR            (1uL << 9)
#define STA_DBCKEND             (1uL << 10)
#define STA_CMDACT              (1uL << 11)
#define STA_TXACT               (1uL << 12)
#define STA_RXACT               (1uL << 13)

/*********************************************************************
*
*       SDMMC data control register
*/
#define DCTRL_DTEN              (1uL << 0)
#define DCTRL_DTDIR             (1uL << 1)
#define DCTRL_DTMODE            (1uL << 2)
#define DCTRL_DMAEN             (1uL << 3)
#define DCTRL_DBLOCKSIZE_SHIFT  4uL

/*********************************************************************
*
*       SDMMC clock control register
*/
#define CLKCR_CLKEN             (1uL   << 8)
#define CLKCR_CLK_PWRSAV        (0x1uL << 9)
#define CLKCR_CLK_BYPASS        (0x1uL << 10)
#define CLKCR_WIDBUS_MASK       (0x3uL << 11)
#define CLKCR_WIDBUS_4BIT       (0x1uL << 11)
#define CLKCR_WIDBUS_8BIT       (0x2uL << 11)
#define CLKCR_HWFC_EN           (1uL   << 14)

/*********************************************************************
*
*       SDMMC command register
*/
#define CMD_CMD_MASK            (0x3FuL)
#define CMD_WAITRESP_SHORT      (1uL << 6)
#define CMD_WAITRESP_LONG       (3uL << 6)
#define CMD_WAITPEND            (1uL << 9)
#define CMD_CPSMEN              (1uL << 10)

/*********************************************************************
*
*       SDMMC interrupt control register
*/
#define ICR_CCRCFAIL            (1uL << 0)
#define ICR_DCRCFAIL            (1uL << 1)
#define ICR_CTIMEOUT            (1uL << 2)
#define ICR_DTIMEOUT            (1uL << 3)
#define ICR_TXUNDERR            (1uL << 4)
#define ICR_RXOVERR             (1uL << 5)
#define ICR_CMDREND             (1uL << 6)
#define ICR_CMDSENT             (1uL << 7)
#define ICR_DATAEND             (1uL << 8)
#define ICR_STBITERR            (1uL << 9)
#define ICR_DBCKEND             (1uL << 10)
#define ICR_SDIOIT              (1uL << 22)
#define ICR_CEATAEND            (1uL << 23)
#define ICR_MASK_STATIC         (ICR_CCRCFAIL | \
                                 ICR_DCRCFAIL | \
                                 ICR_CTIMEOUT | \
                                 ICR_DTIMEOUT | \
                                 ICR_TXUNDERR | \
                                 ICR_RXOVERR  | \
                                 ICR_CMDREND  | \
                                 ICR_CMDSENT  | \
                                 ICR_DATAEND  | \
                                 ICR_STBITERR | \
                                 ICR_DBCKEND  | \
                                 ICR_SDIOIT   | \
                                 ICR_CEATAEND)

/*********************************************************************
*
*       SDMMC interrupt mask register
*/
#define MASK_CCRCFAILIE         (1uL << 0)
#define MASK_DCRCFAILIE         (1uL << 1)
#define MASK_CTIMEOUTIE         (1uL << 2)
#define MASK_DTIMEOUTIE         (1uL << 3)
#define MASK_TXUNDERRIE         (1uL << 4)
#define MASK_RXOVERRIE          (1uL << 5)
#define MASK_CMDRENDIE          (1uL << 6)
#define MASK_CMDSENTIE          (1uL << 7)
#define MASK_DATAENDIE          (1uL << 8)
#define MASK_STBITERRIE         (1uL << 9)
#define MASK_DBCKENDIE          (1uL <<10)
#define MASK_ALL                (MASK_CCRCFAILIE | \
                                 MASK_DCRCFAILIE | \
                                 MASK_CTIMEOUTIE | \
                                 MASK_DTIMEOUTIE | \
                                 MASK_TXUNDERRIE | \
                                 MASK_RXOVERRIE  | \
                                 MASK_CMDRENDIE  | \
                                 MASK_CMDSENTIE  | \
                                 MASK_DATAENDIE  | \
                                 MASK_STBITERRIE | \
                                 MASK_DBCKENDIE)

/*********************************************************************
*
*       SDMMC data length register
*/
#define DLEN_DATALENGTH_MASK    (0x03FFFFFFuL)

/*********************************************************************
*
*       GPIO bit positions of SD card lines
*
*/
#define SD_D0_BIT               8     // Port C
#define SD_D1_BIT               9     // Port C
#define SD_D2_BIT               10    // Port C
#define SD_D3_BIT               11    // Port C
#define SD_CLK_BIT              12    // Port C
#define SD_CD_BIT               13    // Port C
#define SD_CMD_BIT              2     // Port D

/*********************************************************************
*
*       DMA2 related defines
*/
#define LIFCR_CFEIF3            (1uL << 22)
#define LIFCR_CDMEIF3           (1uL << 24)
#define LIFCR_CTCIF3            (1uL << 25)
#define LIFCR_CHTIF3            (1uL << 26)
#define LIFCR_CTEIF3            (1uL << 27)
#define LISR_FEIF3              (1uL << 22)
#define LISR_TEIF3              (1uL << 25)
#define LISR_TCIF3              (1uL << 27)
#define S3CR_EN                 (1uL << 0)
#define S3CR_TEIE               (1uL << 2)
#define S3CR_TCIE               (1uL << 4)
#define S3CR_DIR_MASK           (3uL << 6)
#define S3CR_DIR_M2P            (1uL << 6)
#define S3CR_DIR_M2M            (2uL << 6)
#define S3CR_PINC               (1uL << 9)
#define S3CR_MINC               (1uL << 10)
#define S3CR_PSIZE_32BIT        (2uL << 11)
#define S3CR_MSIZE_32BIT        (2uL << 13)
#define S3CR_PRIO_HIGH          (3uL << 16)
#define S3CR_CHSEL_CH4          (4uL << 25)
#define S3CR_PFCTRL             (1uL << 5)
#define S3CR_PBURST_INCR4       (1uL << 21)
#define S3CR_MBURST_INCR4       (1uL << 23)
#define S3CR_MBURST_NONE        (0uL << 23)
#define S3CR_MBURST_MASK        (3uL << 23)
#define S3FCR_DMDIS             (1uL << 2)
#define S3FCR_FTH_FULL          (3uL << 0)

/*********************************************************************
*
*       Misc. defines
*/
#define PERIPHERAL_TO_MEMORY    0
#define MEMORY_TO_PERIPHERAL    1
#define DMA_MAX_NUM_TRANSFERS   65535
#define MAX_BLOCK_SIZE          512     // Maximum number of bytes in a transferred data block
#define BLOCK_ALIGNMENT         64
#define MAX_NUM_BLOCKS          (((DMA_MAX_NUM_TRANSFERS * 4) / MAX_BLOCK_SIZE) & ~(BLOCK_ALIGNMENT - 1))   // * 4 since the DMA is configured to transfer 4 bytes at once
#define WAIT_TIMEOUT_MAX        1000
#define SDMMC_PRIO              15
#define DMA_PRIO                15
#define CCR_DC_BIT              16
#define CACHE_LINE_SIZE         32     // Number of bytes in a cache line.

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
static U8               _IgnoreCRC;
static U16              _BlockSize;
static U16              _NumBlocks;
static U32              _DataBlockSize;
static void           * _pBuffer;
static U32              _NumLoopsWriteRegDelay;
static U32              _aCacheBuffer[((CACHE_LINE_SIZE * 2) + CACHE_LINE_SIZE) / 4];
static U32            * _pCacheBuffer;
static U8               _IsR1Busy;
#if USE_OS
  static volatile U32   _StatusSDIO;
  static volatile U32   _StatusDMA;
#endif
#if (FS_VERSION > 40405)
  static U8             _RepeatSame;
#endif

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/

/*********************************************************************
*
*       _ld
*/
static U16 _ld(U32 Value) {
  U16 i;

  for (i = 0; i < 16; i++) {
    if ((1uL << i) == Value) {
      break;
    }
  }
  return i;
}

/*********************************************************************
*
*       _RoundUpToCacheLine
*/
static U32 _RoundUpToCacheLine(U32 v) {
  if (v & (CACHE_LINE_SIZE - 1)) {
    v &= ~(CACHE_LINE_SIZE - 1);
    v += CACHE_LINE_SIZE;
  }
  return v;
}

/*********************************************************************
*
*       _RoundDownToCacheLine
*/
static U32 _RoundDownToCacheLine(U32 v) {
  if (v & (CACHE_LINE_SIZE - 1)) {
    v &= ~(CACHE_LINE_SIZE - 1);
  }
  return v;
}

/*********************************************************************
*
*       _Delay1ms
*
*  Function description
*    Blocks the program execution for about 1ms.
*    The number of loops must be adjusted according to the CPU speed.
*/
static void _Delay1ms(void) {
#if USE_OS
  OS_Delay(1);
#else
  volatile unsigned NumLoops;

  NumLoops = 18000;
  do {
    ;
  } while(--NumLoops);
#endif
}

/*********************************************************************
*
*       _StartDMATransfer
*/
static void _StartDMATransfer(U32 * pMemory, U32 * pPripheral, U8 Direction) {
  DMA2_S3CR   &= ~S3CR_EN;          // Stop the data transfer
  while (DMA2_S3CR & S3CR_EN) {     // Wait for the stream to switch off
    ;
  }
  DMA2_S3PAR  = (U32)pPripheral;    // Periphery data register address
  DMA2_S3M0AR = (U32)pMemory;       // Memory buffer address
  DMA2_S3NDTR = 0;
  DMA2_LIFCR  = 0                   // Clear any pending interrupts.
              | LIFCR_CDMEIF3
              | LIFCR_CTEIF3
              | LIFCR_CHTIF3
              | LIFCR_CTCIF3
              | LIFCR_CFEIF3
              ;
  DMA2_S3CR   = 0
              | S3CR_PSIZE_32BIT    // Peripheral bus width
              | S3CR_MSIZE_32BIT    // Memory bus width
              | S3CR_MINC           // Memory increment enable
              | S3CR_PRIO_HIGH      // Set priority to high
              | S3CR_CHSEL_CH4      // Channel connected to SDMMC
              | S3CR_PFCTRL         // Peripheral controls the data transfer
              | S3CR_PBURST_INCR4   // Burst transfer on peripheral side
              | S3CR_MBURST_INCR4   // Burst transfer on memory side
              ;
  if (Direction == MEMORY_TO_PERIPHERAL) {
    DMA2_S3CR |= S3CR_DIR_M2P;
  }
  DMA2_S3FCR  = 0
              | S3FCR_FTH_FULL      // Full FIFO
              | S3FCR_DMDIS         // Disable direct mode (the only way the DMA transfer works together with SDMMC)
              ;
#if USE_OS
  DMA2_S3CR   |= 0
              | S3CR_TEIE
              | S3CR_TCIE
              ;
  _StatusDMA  = 0;
#endif
#if ALIGNED_DMA_BURSTS
  //
  // The memory address must be aligned to burst size.
  // If this is not the case use single transfers.
  //
  DMA2_S3CR   &= ~S3CR_MBURST_MASK;
  if (((U32)pMemory & 0xFuL)  == 0 &&   // Can DMA perform bursts of 16 bytes?
      ((NumBytes    & 0xFFuL) == 0)) {
    DMA2_S3CR |= S3CR_MBURST_INCR4;
  } else {
    DMA2_S3CR |= S3CR_MBURST_NONE;
  }
#endif // ALIGNED_DMA_BURSTS
#if (FS_VERSION > 40405)
  if (_RepeatSame) {
    DMA2_S3CR &= ~S3CR_MINC;      // Do not increment memory pointer when filling with the same 32-bit value.
  } else {
    DMA2_S3CR |=  S3CR_MINC;      // Increment the memory pointer when writing entire blocks.
  }
#endif // FS_VERSION > 40405
  DMA2_S3CR   |= S3CR_EN;               // Start the data transfer
}

/*********************************************************************
*
*       _CopyDataWithDMA
*/
static int _CopyDataWithDMA(U32 * pDest, U32 * pSrc, U32 NumBytes) {
  int r;
  U32 Status;

  r = 0;                              // Set to indicate success.
  DMA2_S3CR &= ~S3CR_EN;              // Stop the data transfer
  while (DMA2_S3CR & S3CR_EN) {       // Wait for the stream to switch off
    ;
  }
  DMA2_S3PAR  = (U32)pSrc;            // Periphery data register address
  DMA2_S3M0AR = (U32)pDest;           // Memory buffer address
  DMA2_S3NDTR = NumBytes >> 2;        // 4 bytes at a time are transferred.
  DMA2_LIFCR  = 0                     // Clear any pending interrupts.
              | LIFCR_CDMEIF3
              | LIFCR_CTEIF3
              | LIFCR_CHTIF3
              | LIFCR_CTCIF3
              | LIFCR_CFEIF3
              ;
  DMA2_S3CR   = 0
              | S3CR_PSIZE_32BIT      // Peripheral bus width
              | S3CR_MSIZE_32BIT      // Memory bus width
              | S3CR_MINC             // Memory increment enable
              | S3CR_PINC             // Memory increment enable
              | S3CR_PBURST_INCR4     // Burst transfer on peripheral side
              | S3CR_MBURST_INCR4     // Burst transfer on memory side
              | S3CR_DIR_M2M          // Perform a memory to memory transfer
              ;
  DMA2_S3FCR  = 0
              | S3FCR_FTH_FULL      // Full FIFO
              | S3FCR_DMDIS         // Disable direct mode
              ;
  DMA2_S3CR   |= S3CR_EN;             // Start the data transfer
  while (1) {                         // Wait for the end of data transfer
    Status = DMA2_LISR;
    if (Status & LISR_TEIF3) {
      r = 1;
      break;                          // Error, could not copy data.
    }
    if (Status & LISR_FEIF3) {
      r = 1;
      break;                          // Error, could not copy data.
    }
    if (Status & LISR_TCIF3) {
      break;                          // OK, transfer completed successfully.
    }
  }
  if (r) {
    DMA2_S3CR &= ~S3CR_EN;            // Stop the data transfer
    while (DMA2_S3CR & S3CR_EN) {     // Wait for the stream to switch off
      ;
    }
  }
  return r;
}

/*********************************************************************
*
*       _IsDCacheEnabled
*
*  Function description
*    Returns if the data cache is enabled.
*/
static int _IsDCacheEnabled(void) {
  int r;

  r = 0;
  if (SCB_CCR & (1uL << CCR_DC_BIT)) {      // Is data cache enabled?
    r = 1;
  }
  return r;
}

/*********************************************************************
*
*       _InvalidateDCache
*
*  Function description
*    Invalidates the lines of the data cache
*    which store a specified memory range.
*/
static void _InvalidateDCache(U32 * p, U32 NumBytes) {
  U32 AddrBegin;
  U32 AddrEnd;

  if (NumBytes) {
    //
    // Align the address and the number of bytes to a cache line.
    //
    AddrBegin = (U32)p;
    AddrEnd   = AddrBegin + NumBytes;
    AddrBegin = _RoundDownToCacheLine(AddrBegin);
    AddrEnd   = _RoundUpToCacheLine(AddrEnd);
    NumBytes  = AddrEnd - AddrBegin;
    SCB_InvalidateDCache_by_Addr((unsigned int *)AddrBegin, (int)NumBytes);
  }
}

/*********************************************************************
*
*       _CleanDCache
*
*  Function description
*    Cleans the lines of the data cache
*    which store a specified memory range.
*/
static void _CleanDCache(U32 * p, U32 NumBytes) {
  U32 AddrBegin;
  U32 AddrEnd;

  if (NumBytes) {
    //
    // Align the address and the number of bytes to a cache line.
    //
    AddrBegin = (U32)p;
    AddrEnd   = AddrBegin + NumBytes;
    AddrBegin = _RoundDownToCacheLine(AddrBegin);
    AddrEnd   = _RoundUpToCacheLine(AddrEnd);
    NumBytes  = AddrEnd - AddrBegin;
    SCB_CleanDCache_by_Addr((unsigned int *)AddrBegin, (int)NumBytes);
  }
}

/*********************************************************************
*
*       _IsDataCached
*/
static int _IsDataCached(U32 * pData) {
  int r;

  //
  // Add addresses above 0x20010000 are accessed via L1 cache of the CPU.
  //
  r = 0;
  if (_IsDCacheEnabled() && ((U32)pData >= 0x20010000)) {
    r = 1;
  }
  return r;
}

/*********************************************************************
*
*       _StartDMARead
*/
static void _StartDMARead(U32 * pData, U32 NumBytes) {
  if (_IsDataCached(pData)) {
    //
    // Make sure that the data is consistent by cleaning the caches.
    //
    _CleanDCache(pData, NumBytes);
  }
  _StartDMATransfer(pData, (U32 *)&SDMMC_FIFO, PERIPHERAL_TO_MEMORY);
}

/*********************************************************************
*
*       _StartDMAWrite
*/
static void _StartDMAWrite(U32 * pData, U32 NumBytes) {
  if (_IsDataCached(pData)) {
    //
    // Make sure that the data is consistent by cleaning the caches.
    //
    _CleanDCache(pData, NumBytes);
  }
  _StartDMATransfer(pData, (U32 *)&SDMMC_FIFO, MEMORY_TO_PERIPHERAL);
}

/*********************************************************************
*
*       _WaitForEndOfDMARead
*
*  Function description
*    Waits until the current DMA read operation finishes.
*/
static int _WaitForEndOfDMARead(void) {
  U32 StatusSDIO;
  U32 StatusDMA;
  int r;

  r = FS_MMC_CARD_READ_TIMEOUT;
  while (1) {
#if USE_OS
    StatusSDIO = _StatusSDIO;
    StatusDMA  = _StatusDMA;
#else
    StatusSDIO = SDMMC_STA;
    StatusDMA  = DMA2_LISR;
#endif
    if (StatusDMA & LISR_TEIF3) {
      r = FS_MMC_CARD_READ_GENERIC_ERROR;
      break;
    }
    if (StatusSDIO & STA_DCRCFAIL) {
      r = FS_MMC_CARD_READ_CRC_ERROR;
      break;
    }
    if (StatusSDIO & STA_DTIMEOUT) {
      r = FS_MMC_CARD_READ_TIMEOUT;
      break;
    }
    if (StatusSDIO & STA_RXOVERR) {
      r = FS_MMC_CARD_READ_GENERIC_ERROR;
      break;
    }
    if (StatusSDIO & STA_STBITERR) {
      r = FS_MMC_CARD_READ_GENERIC_ERROR;
      break;
    }
    if (StatusDMA & LISR_TCIF3) {
      r = FS_MMC_CARD_NO_ERROR;
      break;
    }
#if USE_OS
    FS_X_OS_Wait(WAIT_TIMEOUT_MAX);
#endif
  }
  //
  // In case of an error, disable DMA and the data state machine in SDMMC.
  //
  if (r) {
    DMA2_S3CR &= ~S3CR_EN;            // Stop the data transfer
    while (DMA2_S3CR & S3CR_EN) {     // Wait for the stream to switch off
      ;
    }
    SDMMC_DCTRL &= ~DCTRL_DTEN;        // Cancel SDMMC data transfer.
  }
  return r;
}

/*********************************************************************
*
*       _WaitForEndOfDMARead
*
*  Function description
*    Waits until the current DMA write operation finishes.
*/
static int _WaitForEndOfDMAWrite(void) {
  U32 StatusSDIO;
  U32 StatusDMA;
  int r;

  r = FS_MMC_CARD_WRITE_GENERIC_ERROR;
  //
  // Data transfer starts as soon as the Data Control Register of SDIO is updated.
  //
  SDMMC_DCTRL = 0
             | _DataBlockSize
             | DCTRL_DTEN
             | DCTRL_DMAEN
             ;
  while (1) {
#if USE_OS
    StatusSDIO = _StatusSDIO;
    StatusDMA  = _StatusDMA;
#else
    StatusSDIO = SDMMC_STA;
    StatusDMA  = DMA2_LISR;
#endif
    if (StatusDMA & LISR_TEIF3) {
      r = FS_MMC_CARD_WRITE_GENERIC_ERROR;
      break;
    }
    if (StatusSDIO & STA_DCRCFAIL) {
      r = FS_MMC_CARD_WRITE_CRC_ERROR;
      break;
    }
    if (StatusSDIO & STA_DTIMEOUT) {
      r = FS_MMC_CARD_WRITE_GENERIC_ERROR;
      break;
    }
    if (StatusSDIO & STA_TXUNDERR) {
      r = FS_MMC_CARD_WRITE_GENERIC_ERROR;
      break;
    }
    if (StatusSDIO & STA_STBITERR) {
      r = FS_MMC_CARD_WRITE_GENERIC_ERROR;
      break;
    }
    if (StatusSDIO & STA_DATAEND) {
      r = FS_MMC_CARD_NO_ERROR;
      break;
    }
#if USE_OS
    FS_X_OS_Wait(WAIT_TIMEOUT_MAX);
#endif
  }
  //
  // In case of an error, disable DMA and the data state machine in SDMMC.
  //
  if (r) {
    DMA2_S3CR &= ~S3CR_EN;            // Stop the data transfer
    while (DMA2_S3CR & S3CR_EN) {     // Wait for the stream to switch off
      ;
    }
    SDMMC_DCTRL &= ~DCTRL_DTEN;        // Cancel SDMMC data transfer.
  }
  return r;
}

/*********************************************************************
*
*       _ProcessDMARead
*
*  Function description
*    Invalidates the data cache of the specified buffer by preserving
*    of the bytes located before and after the buffer when the buffer
*    is not cache aligned. The unaligned bytes are copied via DMA
*    to an internal cache-aligned buffer which is then invalidated.
*    At the end the data is copied via CPU from the internal cache-aligned
*    buffer to the original location in the specified buffer.
*/
static void _ProcessDMARead(U32 * pData, U32 NumBytes) {
  U8  * pData8;
  U32   NumBytesAtOnce;

  if (_IsDataCached(pData)) {
    //
    // Invalidate unaligned leading bytes.
    //
    pData8 = (U8 *)pData;
    NumBytesAtOnce = (U32)pData8 & (CACHE_LINE_SIZE - 1);
    if (NumBytesAtOnce) {
      NumBytesAtOnce = CACHE_LINE_SIZE - NumBytesAtOnce;
      NumBytesAtOnce = SEGGER_MIN(NumBytesAtOnce, NumBytes);
      _CopyDataWithDMA(_pCacheBuffer, (U32 *)(void *)pData8, NumBytesAtOnce);
      _InvalidateDCache(_pCacheBuffer, NumBytesAtOnce);
      FS_MEMCPY(pData8, _pCacheBuffer, NumBytesAtOnce);
      NumBytes -= NumBytesAtOnce;
      pData8   += NumBytesAtOnce;
    }
    //
    // Invalidate aligned bytes.
    //
    if (NumBytes) {
      NumBytesAtOnce = NumBytes & ~(CACHE_LINE_SIZE - 1);
      if (NumBytesAtOnce) {
        _InvalidateDCache((U32 *)(void *)pData8, NumBytesAtOnce);
        NumBytes -= NumBytesAtOnce;
        pData8   += NumBytesAtOnce;
      }
    }
    //
    // Invalidate unaligned trailing bytes.
    //
    if (NumBytes) {
      _CopyDataWithDMA(_pCacheBuffer, (U32 *)(void *)pData8, NumBytes);
      _InvalidateDCache(_pCacheBuffer, NumBytes);
      FS_MEMCPY(pData8, _pCacheBuffer, NumBytes);
    }
  }
}

#if USE_OS

/**********************************************************
*
*       SDMMC1_IRQHandler
*
*  Function description
*    Handles the SDMMC interrupt.
*/
#ifdef __cplusplus
extern "C" {                                  // Prevent function name mangling with C++ compilers.
#endif
void SDMMC1_IRQHandler(void);
#ifdef __cplusplus
}
#endif
void SDMMC1_IRQHandler(void) {
  U32 Status;

  OS_EnterInterrupt();                        // Inform embOS that interrupt code is running.
  Status       = SDMMC_STA;
  //
  // Save the status to a static variable and check it in the task.
  //
  SDMMC_ICR    = Status & ICR_MASK_STATIC;    // Clear the static flags to prevent further interrupts.
  _StatusSDIO &= ICR_MASK_STATIC;             // Clear the dynamic flags
  _StatusSDIO |= Status;
  FS_X_OS_Signal();                           // Wake up the task.
  OS_LeaveInterrupt();                        // Inform embOS that interrupt code is left.
}

/**********************************************************
*
*       DMA2_Channel4_IRQHandler
*
*   Function description
*     Handles the DMA interrupt.
*/
#ifdef __cplusplus
extern "C" {                                  // Prevent function name mangling with C++ compilers.
#endif
void DMA2_Stream3_IRQHandler(void);
#ifdef __cplusplus
}
#endif
void DMA2_Stream3_IRQHandler(void) {
  U32 Status;

  OS_EnterInterrupt();          // Inform embOS that interrupt code is running.
  Status      = DMA2_LISR;
  DMA2_LIFCR  = Status;         // Clear pending interrupt flags.
  Status     &= 0               // Make sure that we clear only the flags assigned to the DMA stream we use.
             | LIFCR_CFEIF3
             | LIFCR_CDMEIF3
             | LIFCR_CTCIF3
             | LIFCR_CHTIF3
             | LIFCR_CTEIF3
             ;
  _StatusDMA |= Status;         // Save the status to a static variable and check it in the task.
  FS_X_OS_Signal();             // Wake up the task.
  OS_LeaveInterrupt();          // Inform embOS that interrupt code is left.
}

#endif

/*********************************************************************
*
*       _WriteRegDelayed
*
*  Function description
*    Waits before writing to a register as specified in the datasheet:
*      "After a data write, data cannot be written to this register for three SDIOCLK
*      (48 MHz) clock periods plus two PCLK2 clock periods."
*/
static void _WriteRegDelayed(volatile U32 * pAddr, U32 Value) {
  volatile U32 NumLoops;

  NumLoops  = _NumLoopsWriteRegDelay;
  do {
    ;
  } while (--NumLoops);
  *pAddr = Value;
}

/*********************************************************************
*
*       _WaitForInactive
*/
static void _WaitForInactive(void) {
  U32 v;
  U32 Status;

  while (1) {
    Status = SDMMC_STA;
    if (((Status & STA_CMDACT) == 0) &&
        ((Status & STA_TXACT)  == 0) &&
        ((Status & STA_RXACT)  == 0)) {
      if (_IsR1Busy == 0) {
        break;
      }
      //
      // The SDIO host controller is not able to handle R1 responses
      // with busy signaling therefore we have to check here the
      // level of the DAT0 line. The SD / MMC device drives the DAT0
      // line to LOW if busy.
      // The SD / MMC devices require to be clocked in order to release
      // the DAT0 line but the SDIO host controller is configured clock
      // the SD/ MMC device only when a command or data is transferred
      // (to prevent DMA issues). For this reason we have to temporarily
      // disable this mode so that the SDIO host controller
      // clocks the SD / MMC device while we are checking the status
      // of the DAT0 line.
      //
      v = SDMMC_CLKCR & ~CLKCR_CLK_PWRSAV;
      _WriteRegDelayed(&SDMMC_CLKCR, v);
      if (GPIOC_IDR & (1uL << SD_D0_BIT)) {
        _IsR1Busy = 0;
        v = SDMMC_CLKCR | CLKCR_CLK_PWRSAV;
        _WriteRegDelayed(&SDMMC_CLKCR, v);
        break;
      }
    }
  }
}

/*********************************************************************
*
*       _Reset
*/
static void _Reset(void) {
  U32 RegValueCLKCR;
  U32 RegValueDTIMER;

  //
  // Save register values that have to be restored later.
  //
  RegValueCLKCR  = SDMMC_CLKCR;
  RegValueDTIMER = SDMMC_DTIMER;
  //
  // Reset the SDMMC unit.
  //
  RCC_APB2RSTR |= APB2RSTR_SDMMCRST;
  RCC_APB2RSTR &= ~APB2RSTR_SDMMCRST;
  //
  // SDMMC uses the stream 3, channel 4 of DMA2
  //
  DMA2_S3CR &= ~S3CR_EN;            // Stop the data transfer
  while (DMA2_S3CR & S3CR_EN) {     // Wait for the stream to switch off
    ;
  }
  DMA2_LIFCR |= 0                   // Clear any pending interrupts.
             |  LIFCR_CDMEIF3
             |  LIFCR_CTEIF3
             |  LIFCR_CHTIF3
             |  LIFCR_CTCIF3
             |  LIFCR_CFEIF3
             ;
#if USE_OS
  _StatusSDIO = 0;
  _StatusDMA  = 0;
#endif
  //
  // Initialize the SDMMC unit.
  //
  SDMMC_POWER  = 0;    // Power off
  SDMMC_CLKCR  = 0;    // Disable the clock.
  SDMMC_ARG    = 0;
  SDMMC_CMD    = 0;
  SDMMC_DTIMER = 0;
  SDMMC_DLEN   = 0;
  SDMMC_DCTRL  = 0;
  SDMMC_ICR    = 0x00C007FF;       // Clear interrupts
  SDMMC_MASK   = MASK_ALL;
  _WriteRegDelayed(&SDMMC_POWER, 3);     // Make sure that SDMMC_POWER is written after the specified delay.
  _WriteRegDelayed(&SDMMC_CLKCR, RegValueCLKCR);
  SDMMC_DTIMER = RegValueDTIMER;
}

/*********************************************************************
*
*       Public code (via callback)
*
**********************************************************************
*/

/*********************************************************************
*
*       _HW_Init
*
*  Function description
*    Initialize the SD / MMC host controller.
*
*  Parameters
*    Unit     Index of the SD / MMC host controller (0-based).
*/
static void _HW_Init(U8 Unit) {
  unsigned ms;
  U32      Delay_ns;
  U32      Inst_ns;
  U32      Addr;

  FS_USE_PARA(Unit);
  //
  // Prepare the cache buffer.
  //
  Addr = (U32)_aCacheBuffer;
  Addr = _RoundUpToCacheLine(Addr);
  _pCacheBuffer = (U32 *)Addr;
  //
  // Reset the SDMMC unit.
  //
  RCC_APB2RSTR |= APB2RSTR_SDMMCRST;
  RCC_APB2RSTR &= ~APB2RSTR_SDMMCRST;

  //
  // Enable GPIOs, DMA and SDMMC units.
  //
  RCC_AHB1ENR |= 0
              |  AHB1ENR_DMA2EN
              |  AHB1ENR_PORTCEN
              |  AHB1ENR_PORTDEN
              ;
  RCC_APB2ENR |= 0
              |  APB2ENR_SDMMCEN
              ;
  //
  // D0, D1, D2, D3, CLK signals are controlled by SDMMC
  //
  GPIOC_MODER &= ~((3uL << (SD_D0_BIT  << 1)) |
                   (3uL << (SD_D1_BIT  << 1)) |
                   (3uL << (SD_D2_BIT  << 1)) |
                   (3uL << (SD_D3_BIT  << 1)) |
                   (3uL << (SD_CLK_BIT << 1)));
  GPIOC_MODER |= (2uL << (SD_D0_BIT  << 1))
              |  (2uL << (SD_D1_BIT  << 1))
              |  (2uL << (SD_D2_BIT  << 1))
              |  (2uL << (SD_D3_BIT  << 1))
              |  (2uL << (SD_CLK_BIT << 1))
              ;
  GPIOC_PUPDR &= ~((3uL << (SD_D0_BIT  << 1)) |
                   (3uL << (SD_D1_BIT  << 1)) |
                   (3uL << (SD_D2_BIT  << 1)) |
                   (3uL << (SD_D3_BIT  << 1)) |
                   (3uL << (SD_CLK_BIT << 1)));
  GPIOC_AFRH  &= ~((0xFuL << ((SD_D0_BIT - 8)  << 2)) |
                   (0xFuL << ((SD_D1_BIT - 8) << 2))  |
                   (0xFuL << ((SD_D2_BIT - 8)  << 2)) |
                   (0xFuL << ((SD_D3_BIT - 8)  << 2)) |
                   (0xFuL << ((SD_CLK_BIT - 8) << 2)));
  GPIOC_AFRH  |= (12uL << ((SD_D0_BIT - 8)  << 2))
              |  (12uL << ((SD_D1_BIT - 8)  << 2))
              |  (12uL << ((SD_D2_BIT - 8)  << 2))
              |  (12uL << ((SD_D3_BIT - 8)  << 2))
              |  (12uL << ((SD_CLK_BIT - 8) << 2));
  GPIOC_OSPEEDR &= ~((3uL << (SD_D0_BIT  << 1)) |
                    (3uL << (SD_D1_BIT  << 1))  |
                    (3uL << (SD_D2_BIT  << 1))  |
                    (3uL << (SD_D3_BIT  << 1))  |
                    (3uL << (SD_CLK_BIT << 1)));
  GPIOC_OSPEEDR |= (3uL << (SD_D0_BIT  << 1)) |
                   (3uL << (SD_D1_BIT  << 1)) |
                   (3uL << (SD_D2_BIT  << 1)) |
                   (3uL << (SD_D3_BIT  << 1)) |
                   (3uL << (SD_CLK_BIT << 1));    // High speed ports
  //
  // The CMD signal is also controlled by SDMMC
  //
  GPIOD_MODER   &= ~(3uL   << (SD_CMD_BIT << 1));
  GPIOD_MODER   |=  (2uL   << (SD_CMD_BIT << 1));
  GPIOD_PUPDR   &= ~(3uL   << (SD_CMD_BIT << 1));
  GPIOD_AFRL    &= ~(0xFuL << (SD_CMD_BIT << 2));
  GPIOD_AFRL    |=  (12uL  << (SD_CMD_BIT << 2));
  GPIOD_OSPEEDR &= ~(3uL   << (SD_CMD_BIT << 1));
  GPIOD_OSPEEDR |=  (2uL   << (SD_CMD_BIT << 1));
  //
  // The CD signal is controlled by this HW layer.
  //
  GPIOC_MODER   &= ~(3uL   << (SD_CD_BIT << 1));
  GPIOC_PUPDR   &= ~(3uL   << (SD_CD_BIT << 1));
  GPIOC_AFRH    &= ~(0xFuL << ((SD_CD_BIT - 8) << 2));
  GPIOC_OSPEEDR &= ~(3uL   << (SD_CD_BIT << 1));
  //
  // SDMMC uses the stream 3, channel 4 of DMA2
  //
  DMA2_S3CR   &= ~S3CR_EN;          // Stop the data transfer
  while (DMA2_S3CR & S3CR_EN) {     // Wait for the stream to switch off
    ;
  }
  DMA2_LIFCR  |= LIFCR_CDMEIF3      // Clear any pending interrupts.
              |  LIFCR_CTEIF3
              |  LIFCR_CHTIF3
              |  LIFCR_CTCIF3
              |  LIFCR_CFEIF3
              ;
  //
  // Compute the number of software loops to delay before writing to some SDMMC registers.
  //
  Delay_ns  = (1000000000uL + ((SDMMC_CLK_KHZ * 3) - 1)) / (SDMMC_CLK_KHZ * 3);
  Delay_ns += (1000000000uL + ((PCLK2 * 2) - 1)) / (PCLK2 * 2);
  Inst_ns   = (1000000000uL + (HCLK - 1)) / HCLK;
  _NumLoopsWriteRegDelay = Delay_ns / Inst_ns;
  //
  // Initialize SDMMC
  //
  SDMMC_POWER  = 0;    // Power off
  SDMMC_CLKCR  = 0;    // Disable the clock.
  SDMMC_ARG    = 0;
  SDMMC_CMD    = 0;
  SDMMC_DTIMER = 0;
  SDMMC_DLEN   = 0;
  SDMMC_DCTRL  = 0;
  SDMMC_ICR    = 0x00C007FF;       // Clear interrupts
  SDMMC_MASK   = 0;
  _WriteRegDelayed(&SDMMC_POWER, 3);     // Make sure that SDMMC_POWER is written after the specified delay.
#if USE_OS
  //
  // Unmask the interrupt sources.
  //
  SDMMC_MASK   = MASK_ALL;
  //
  // Set the priority and enable the interrupts.
  //
  NVIC_SetPriority(DMA2_Stream3_IRQn, DMA_PRIO);
  NVIC_SetPriority(SDMMC1_IRQn, SDMMC_PRIO);
  NVIC_EnableIRQ(DMA2_Stream3_IRQn);
  NVIC_EnableIRQ(SDMMC1_IRQn);
#endif
  //
  // Wait for the power supply of the SD card to become ready.
  //
  ms = POWER_GOOD_DELAY;
  while (ms--) {
    _Delay1ms();
  }
}

/*********************************************************************
*
*       _HW_Delay
*
*  Function description
*    Blocks the execution for the specified time.
*
*  Parameters
*    ms   Number of milliseconds to delay.
*/
static void _HW_Delay(int ms) {
  while (ms--) {
    _Delay1ms();
  }
}

/*********************************************************************
*
*       _HW_IsPresent
*
*  Function description
*    Returns the state of the media. If you do not know the state,
*    return FS_MEDIA_STATE_UNKNOWN and the higher layer will try to
*    figure out if a storage device is present.
*
*  Parameters
*    Unit     Index of the SD / MMC host controller (0-based).
*
*  Return value
*    FS_MEDIA_STATE_UNKNOWN     The state of the media is unknown
*    FS_MEDIA_NOT_PRESENT       No card is present
*    FS_MEDIA_IS_PRESENT        A card is present
*/
static int _HW_IsPresent(U8 Unit) {
  int Status;

  FS_USE_PARA(Unit);
  Status = FS_MEDIA_IS_PRESENT;
  if (GPIOC_IDR & (1uL << SD_CD_BIT)) {
    Status = FS_MEDIA_NOT_PRESENT;
  }
  return Status;
}

/*********************************************************************
*
*       _HW_IsWriteProtected
*
*  Function description
*    Returns whether card is write protected or not.
*
*  Parameters
*    Unit     Index of the SD / MMC host controller (0-based).
*/
static int _HW_IsWriteProtected(U8 Unit) {
  FS_USE_PARA(Unit);
  return 0;
}

/*********************************************************************
*
*       _HW_SetMaxSpeed
*
*  Function description
*    Sets the frequency of the MMC/SD card controller.
*    The frequency is given in kHz.
*
*  Parameters
*    Unit     Index of the SD / MMC host controller (0-based).
*
*  Additional information
*    This function is called two times:
*    1. During card initialization
*       Initialize the frequency to not more than 400kHz.
*
*    2. After card initialization
*       The CSD register of card is read and the max frequency
*       the card can operate is determined.
*       [In most cases: MMC cards 20MHz, SD cards 25MHz]
*/
static U16 _HW_SetMaxSpeed(U8 Unit, U16 Freq) {
  U32      Fact;
  unsigned Div;
  U32      ClkBypass;
  U32      v;

  FS_USE_PARA(Unit);
  if (Freq > MAX_SD_CLK_KHZ) {
    Freq = MAX_SD_CLK_KHZ;
  }
  _WaitForInactive();
  SDMMC_CLKCR &= ~(CLKCR_CLKEN | 0xFFuL | CLKCR_CLK_BYPASS);
  Fact        = 2;
  Div         = 0;
  ClkBypass   = CLKCR_CLK_BYPASS;
  if (Freq < SDMMC_CLK_KHZ) {
    ClkBypass = 0;
    while ((Freq * Fact) < SDMMC_CLK_KHZ) {
      ++Fact;
      if (0xFF == ++Div) {
        break;
      }
    }
  }
  v = 0
    | SDMMC_CLKCR
    | Div
    | CLKCR_CLKEN
    | CLKCR_CLK_PWRSAV
    | CLKCR_HWFC_EN         // Enable flow control to prevent over-/underflow errors in the data transfer.
    | ClkBypass
    ;
  _WriteRegDelayed(&SDMMC_CLKCR, v);
  return SDMMC_CLK_KHZ / (Div + 2);
}

/*********************************************************************
*
*       _HW_SetResponseTimeOut
*
*  Function description
*    Sets the maximum time the SD / MMC host controller
*    has to wait for a response.
*
*  Parameters
*    Unit     Index of the SD / MMC host controller (0-based).
*    Value    Number of SD / MMC clock cycles.
*/
static void _HW_SetResponseTimeOut(U8 Unit, U32 Value) {
  FS_USE_PARA(Unit);
  FS_USE_PARA(Value);
  //
  // The response timeout is fixed in hardware
  //
}

/*********************************************************************
*
*       _HW_SetReadDataTimeOut
*
*  Function description
*    Sets the maximum time the SD / MMC host controller has to wait
*    for the arrival of data.
*
*  Parameters
*    Unit     Index of the SD / MMC host controller (0-based).
*    Value    Number of SD / MMC clock cycles.
*/
static void _HW_SetReadDataTimeOut(U8 Unit, U32 Value) {
  FS_USE_PARA(Unit);
  SDMMC_DTIMER = Value;
}

/*********************************************************************
*
*       _HW_SendCmd
*
*  Function description
*    Sends a command to the card.
*
*  Parameters
*    Unit           Index of the SD / MMC host controller (0-based).
*    Cmd            Command number according to [1]
*    CmdFlags       Additional information about the command to execute
*    ResponseType   Type of response as defined in [1]
*    Arg            Command parameter
*/
static void _HW_SendCmd(U8 Unit, unsigned Cmd, unsigned CmdFlags, unsigned ResponseType, U32 Arg) {
  U32 CmdCfg;
  U32 NumBytes;
  U32 Status;
  U32 v;

  FS_USE_PARA(Unit);
  _WaitForInactive();
#if (FS_VERSION > 40405)
  _RepeatSame = 0;
  if (CmdFlags & FS_MMC_CMD_FLAG_WRITE_BURST_FILL) {
    _RepeatSame = 1;
  }
#endif
  if (CmdFlags & FS_MMC_CMD_FLAG_SETBUSY) {
    _IsR1Busy = 1;
  }
  CmdCfg = 0
         | CMD_CPSMEN
         ;
  _IgnoreCRC = 0;
  switch (ResponseType) {
  case FS_MMC_RESPONSE_FORMAT_R3:
    _IgnoreCRC = 1;
    //lint -fallthrough
  default:
    //lint -fallthrough
  case FS_MMC_RESPONSE_FORMAT_R1:
    CmdCfg |= CMD_WAITRESP_SHORT;
    break;
  case FS_MMC_RESPONSE_FORMAT_R2:
    CmdCfg |= CMD_WAITRESP_LONG;
    break;
  }
  v  = SDMMC_CLKCR;
  v &= ~CLKCR_WIDBUS_MASK;
  if (CmdFlags & FS_MMC_CMD_FLAG_USE_SD4MODE) {   // 4 bit mode?
    v |= CLKCR_WIDBUS_4BIT;
  } else {
    v &=~CLKCR_WIDBUS_MASK;
  }
  SDMMC_CLKCR = v;
  v = 0;
  if (CmdFlags & FS_MMC_CMD_FLAG_DATATRANSFER) {
    SDMMC_ICR = 0
              | ICR_DCRCFAIL
              | ICR_DTIMEOUT
              | ICR_TXUNDERR
              | ICR_DATAEND
              | ICR_DBCKEND
              | ICR_STBITERR
              | ICR_RXOVERR
              ;
    NumBytes = _BlockSize * _NumBlocks;
    SDMMC_DLEN = NumBytes;
    if ((CmdFlags & FS_MMC_CMD_FLAG_WRITETRANSFER) == 0) {
      //
      // Start the read operation via DMA.
      //
      _StartDMARead(_pBuffer, NumBytes);
      //
      // Data transfer starts when the data control register is updated.
      //
      v = 0
        | _DataBlockSize
        | DCTRL_DTDIR
        | DCTRL_DMAEN
        | DCTRL_DTEN
        ;
    } else {
      //
      // Start the write operation via DMA.
      //
      _StartDMAWrite(_pBuffer, NumBytes);
    }
  }
  SDMMC_DCTRL = v;
  //
  // Clear pending status flags
  //
  SDMMC_ICR = 0
            | ICR_CCRCFAIL
            | ICR_CTIMEOUT
            | ICR_CMDREND
            | ICR_CMDSENT
            ;
#if USE_OS
  _StatusSDIO = 0;
#endif
  SDMMC_ARG = Arg;
  SDMMC_CMD = CmdCfg | (Cmd & CMD_CMD_MASK);
  if (CmdFlags & FS_MMC_CMD_FLAG_INITIALIZE)  {
    while (1) {
#if USE_OS
      Status = _StatusSDIO;
#else
      Status = SDMMC_STA;
#endif
      if (Status & (STA_CMDSENT | STA_CMDREND | STA_CCRCFAIL |  STA_CTIMEOUT)) {
        break;
      }
#if USE_OS
      FS_X_OS_Wait(WAIT_TIMEOUT_MAX);
#endif
    }
  }
}

/*********************************************************************
*
*       _HW_GetResponse
*
*  Function description
*    Waits for a response to be received.
*
*  Parameters
*    Unit     Index of the SD / MMC host controller (0-based).
*    pBuffer  User allocated buffer where the response is stored.
*    Size     Size of the buffer in bytes.
*
*  Return values
*    FS_MMC_CARD_NO_ERROR                 Success
*    FS_MMC_CARD_RESPONSE_CRC_ERROR       CRC error in response
*    FS_MMC_CARD_RESPONSE_TIMEOUT         No response received
*    FS_MMC_CARD_RESPONSE_GENERIC_ERROR   Any other error
*
*  Notes
*    (1) The response data has to be stored at byte offset 1 when
*        the SD host controller does not provide the first byte
*        of response message, that is the byte that includes the
*        start bit.
*/
static int _HW_GetResponse(U8 Unit, void *pBuffer, U32 Size) {
  U8           * p;
  U32            NumBytes;
  volatile U32 * pReg;
  U32            Data32;
  U32            Status;
  U32            NumWords;

  FS_USE_PARA(Unit);
  p        = (U8 *)pBuffer;
  NumBytes = Size;
  while (1) {
#if USE_OS
    Status = _StatusSDIO;
#else
    Status = SDMMC_STA;
#endif
    if (Status & (STA_CMDSENT | STA_CMDREND | STA_CCRCFAIL | STA_CTIMEOUT)) {
      break;
    }
#if USE_OS
    FS_X_OS_Wait(WAIT_TIMEOUT_MAX);
#endif
  }
  if (STA_CTIMEOUT & Status) {
    _Reset();
    return FS_MMC_CARD_RESPONSE_TIMEOUT;
  }
  if ((STA_CCRCFAIL & Status) && !_IgnoreCRC) {
    _Reset();
    return FS_MMC_CARD_READ_CRC_ERROR;
  }
  if ((STA_CMDREND & Status) || _IgnoreCRC) {
    *p++ = (U8)SDMMC_RESPCMD;
    NumBytes--;
    pReg = (volatile U32 *)(&SDMMC_RESP1);
    NumWords = NumBytes >> 2;
    if (NumWords) {
      do {
        Data32 = *pReg++;
        *p++ = (U8)(Data32 >> 24);
        *p++ = (U8)(Data32 >> 16);
        *p++ = (U8)(Data32 >> 8);
        *p++ = (U8)Data32;
        NumBytes -= 4;
      } while (--NumWords);
    }
    if (NumBytes == 3) {
      Data32 = *pReg;
      *p++ = (U8)(Data32 >> 24);
      *p++ = (U8)(Data32 >> 16);
      *p++ = (U8)(Data32 >> 8);
    }
    if (NumBytes == 2) {
      Data32 = *pReg;
      *p++ = (U8)(Data32 >> 24);
      *p++ = (U8)(Data32 >> 16);
    }
    if (NumBytes == 1) {
      Data32 = *pReg;
      *p++ = (U8)(Data32 >> 24);
    }
  }
  return FS_MMC_CARD_NO_ERROR;
}

/*********************************************************************
*
*       _HW_ReadData
*
*  Function description
*    Reads data from the card using the SD / MMC host controller.
*
*  Return values
*    FS_MMC_CARD_NO_ERROR             Success
*    FS_MMC_CARD_READ_CRC_ERROR       CRC error in received data
*    FS_MMC_CARD_READ_TIMEOUT         No data received
*    FS_MMC_CARD_READ_GENERIC_ERROR   Any other error
*/
static int _HW_ReadData(U8 Unit, void * pBuffer, unsigned NumBytes, unsigned NumBlocks) {
  int r;

  FS_USE_PARA(Unit);
  FS_USE_PARA(pBuffer);
  FS_USE_PARA(NumBytes);
  FS_USE_PARA(NumBlocks);
  r = _WaitForEndOfDMARead();
  if (r) {
    _Reset();
    return r;           // Error, could not read data.
  }
  NumBytes *= NumBlocks;
  _ProcessDMARead(pBuffer, NumBytes);
  return r;
}

/*********************************************************************
*
*       _HW_WriteData
*
*  Function description
*    Writes the data to SD / MMC card using the SD / MMC host controller.
*
*  Return values
*    FS_MMC_CARD_NO_ERROR      Success
*    FS_MMC_CARD_READ_TIMEOUT  No data received
*/
static int _HW_WriteData(U8 Unit, const void * pBuffer, unsigned NumBytes, unsigned NumBlocks) {
  int r;

  FS_USE_PARA(Unit);
  FS_USE_PARA(pBuffer);
  FS_USE_PARA(NumBytes);
  FS_USE_PARA(NumBlocks);
  r = _WaitForEndOfDMAWrite();
  if (r) {
    _Reset();
  }
  return r;
}

/*********************************************************************
*
*       _HW_SetDataPointer
*
*  Function description
*    Tells the hardware layer where to read data from
*    or write data to.
*
*  Parameters
*    Unit     Index of the SD / MMC host controller (0-based).
*    p        Data buffer.
*
*  Additional information
*    This function is optional. It is required to be implemented for
*    some SD / MMC host controllers that need to know the address of
*    the data before sending the command to the card,
*    eg. when transferring the data via DMA.
*/
static void _HW_SetDataPointer(U8 Unit, const void * p) {
  FS_USE_PARA(Unit);
  _pBuffer = (void *)p; // cast const away as this buffer is used also for storing the data from card
}

/*********************************************************************
*
*       _HW_SetBlockLen
*
*  Function description
*    Sets the block size (sector size) that has to be transferred.
*
*  Parameters
*    Unit     Index of the SD / MMC host controller (0-based).
*/
static void _HW_SetHWBlockLen(U8 Unit, U16 BlockSize) {
    FS_USE_PARA(Unit);
  _BlockSize     = BlockSize;
  _DataBlockSize = _ld(BlockSize) << DCTRL_DBLOCKSIZE_SHIFT;
}

/*********************************************************************
*
*       _HW_SetNumBlocks
*
*  Function description
*    Sets the number of blocks (sectors) to be transferred.
*
*  Parameters
*    Unit     Index of the SD / MMC host controller (0-based).
*/
static void _HW_SetHWNumBlocks(U8 Unit, U16 NumBlocks) {
  FS_USE_PARA(Unit);
  _NumBlocks = NumBlocks;
}

/*********************************************************************
*
*       _HW_GetMaxReadBurst
*
*  Function description
*    Returns the number of block (sectors) that can be read at once
*    with a single READ_MULTIPLE_SECTOR command.
*
*  Parameters
*    Unit     Index of the SD / MMC host controller (0-based).
*
*  Return value
*    Number of sectors that can be read at once.
*/
static U16 _HW_GetMaxReadBurst(U8 Unit) {
  FS_USE_PARA(Unit);
  return (U16)MAX_NUM_BLOCKS;
}

/*********************************************************************
*
*       _HW_GetMaxWriteBurst
*
*  Function description
*    Returns the number of block (sectors) that can be written at once
*    with a single WRITE_MULTIPLE_SECTOR command.
*
*  Parameters
*    Unit     Index of the SD / MMC host controller (0-based).
*
*  Return value
*    Number of sectors that can be written at once.
*/
static U16 _HW_GetMaxWriteBurst(U8 Unit) {
  FS_USE_PARA(Unit);
  return (U16)MAX_NUM_BLOCKS;
}

#if (FS_VERSION > 40405)

/*********************************************************************
*
*       _HW_GetMaxWriteBurstFill
*
*  Function description
*    Returns the number of block (sectors) that can be written at once
*    with a single WRITE_MULTIPLE_SECTOR command. The contents of the
*    sectors is filled with the same 32-bit pattern.
*
*  Parameters
*    Unit     Index of the SD / MMC host controller (0-based).
*
*  Return value
*    Number of sectors that can be written at once. The function has
*    to return 0 if the feature is not supported.
*/
static U16 _HW_GetMaxWriteBurstFill(U8 Unit) {
  FS_USE_PARA(Unit);
  return (U16)MAX_NUM_BLOCKS;
}

#endif

/*********************************************************************
*
*       Global data
*
**********************************************************************
*/
const FS_MMC_HW_TYPE_CM FS_MMC_HW_CM_STM32F746_ST_STM32F746G_DISCO = {
  _HW_Init,
  _HW_Delay,
  _HW_IsPresent,
  _HW_IsWriteProtected,
  _HW_SetMaxSpeed,
  _HW_SetResponseTimeOut,
  _HW_SetReadDataTimeOut,
  _HW_SendCmd,
  _HW_GetResponse,
  _HW_ReadData,
  _HW_WriteData,
  _HW_SetDataPointer,
  _HW_SetHWBlockLen,
  _HW_SetHWNumBlocks,
  _HW_GetMaxReadBurst,
  _HW_GetMaxWriteBurst
#if (FS_VERSION > 40405)
  , NULL
  , _HW_GetMaxWriteBurstFill
#endif
};

/*************************** End of file ****************************/
