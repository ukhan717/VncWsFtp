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

File        : FS_ConfigNOR_BM_SPIFI_STM32F746_ST_STM32F746G-DISCO.c
Purpose     : Configuration file for serial NOR flash connected via QSPI.
*/

#include "FS.h"
#include "FS_NOR_HW_SPIFI_STM32F746_ST_STM32F746G-DISCO.h"

/*********************************************************************
*
*       Defines, configurable
*
*       This section is the only section which requires changes
*       for typical embedded systems using the NOR flash driver with a single device.
*
**********************************************************************
*/
#define ALLOC_SIZE          0x2000        // Size of memory dedicated to the file system. This value should be fine tuned according for your system.
#define FLASH_BASE_ADDR     0x90000000    // Base address of the NOR flash device to be used as storage.
#define FLASH_START_ADDR    0x90000000    // Start address of the first sector be used as storage. If the entire chip is used for file system, it is identical to the base address.
#define FLASH_SIZE          0x01000000    // Number of bytes to be used for storage
#define BYTES_PER_SECTOR    512           // Logical sector size

/*********************************************************************
*
*       Static data.
*
*       This section does not require modifications in most systems.
*
**********************************************************************
*/

/*********************************************************************
*
*       Memory pool used for semi-dynamic allocation.
*/
#ifdef __ICCARM__
  #pragma location="FS_RAM"
  static __no_init U32 _aMemBlock[ALLOC_SIZE / 4];
#endif
#ifdef __CC_ARM
  static U32 _aMemBlock[ALLOC_SIZE / 4] __attribute__ ((section ("FS_RAM"), zero_init));
#endif
#ifdef __SES_ARM
  static U32 _aMemBlock[ALLOC_SIZE / 4] __attribute__ ((section ("FS_RAM")));
#endif
#if (!defined(__ICCARM__) && !defined(__CC_ARM) && !defined(__SES_ARM))
  static U32 _aMemBlock[ALLOC_SIZE / 4];
#endif

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/

/*********************************************************************
*
*       Public code
*
*       This section does not require modifications in most systems.
*
**********************************************************************
*/

/*********************************************************************
*
*       FS_X_AddDevices
*
*  Function description
*    This function is called by the FS during FS_Init().
*    It is supposed to add all devices, using primarily FS_AddDevice().
*
*  Note
*    (1) Other API functions may NOT be called, since this function is called
*        during initialization. The devices are not yet ready at this point.
*/
void FS_X_AddDevices(void) {
  //
  // Give file system memory to work with.
  //
  FS_AssignMemory(&_aMemBlock[0], sizeof(_aMemBlock));
  //
  // Configure the size of the logical sector and activate the file buffering.
  //
  FS_SetMaxSectorSize(BYTES_PER_SECTOR);
#if FS_SUPPORT_FILE_BUFFER
  FS_ConfigFileBufferDefault(BYTES_PER_SECTOR, FS_FILE_BUFFER_WRITE);
#endif
  //
  // Add and configure the NOR driver.
  //
  FS_AddDevice(&FS_NOR_BM_Driver);
  FS_NOR_BM_SetPhyType(0, &FS_NOR_PHY_SPIFI);
  FS_NOR_BM_Configure(0, FLASH_BASE_ADDR, FLASH_START_ADDR, FLASH_SIZE);
  FS_NOR_BM_SetSectorSize(0, BYTES_PER_SECTOR);
  //
  // Configure the NOR physical layer.
  //
  FS_NOR_SPIFI_SetHWType(0, &FS_NOR_HW_SPIFI_STM32F746_ST_STM32F746G_DISCO);
  FS_NOR_SPIFI_SetDeviceList(0, &FS_NOR_SPI_DeviceList_Micron);
  FS_NOR_SPIFI_Allow2bitMode(0, 1);
  FS_NOR_SPIFI_Allow4bitMode(0, 1);
}

/*********************************************************************
*
*       FS_X_GetTimeDate
*
*  Description:
*    Current time and date in a format suitable for the file system.
*
*    Bit 0-4:   2-second count (0-29)
*    Bit 5-10:  Minutes (0-59)
*    Bit 11-15: Hours (0-23)
*    Bit 16-20: Day of month (1-31)
*    Bit 21-24: Month of year (1-12)
*    Bit 25-31: Count of years from 1980 (0-127)
*/
U32 FS_X_GetTimeDate(void) {
  U32 r;
  U16 Sec, Min, Hour;
  U16 Day, Month, Year;

  Sec   = 0;        // 0 based.  Valid range: 0..59
  Min   = 0;        // 0 based.  Valid range: 0..59
  Hour  = 0;        // 0 based.  Valid range: 0..23
  Day   = 1;        // 1 based.    Means that 1 is 1. Valid range is 1..31 (depending on month)
  Month = 1;        // 1 based.    Means that January is 1. Valid range is 1..12.
  Year  = 0;        // 1980 based. Means that 2007 would be 27.
  r   = Sec / 2 + (Min << 5) + (Hour  << 11);
  r  |= (U32)(Day + (Month << 5) + (Year  << 9)) << 16;
  return r;
}

/*************************** End of file ****************************/
