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

File    : BSP_Memory.c
Purpose : initialize memory after reset during startup
Additional information:
  <Any additional information for this module>
*/

#include "stm32746g_discovery_sdram.h"


/*********************************************************************
*
*       Defines, configurable
*
**********************************************************************
*/

/*********************************************************************
*
*       Defines, fixed
*
**********************************************************************
*/

/*********************************************************************
*
*       Types, local
*
**********************************************************************
*/

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

/*********************************************************************
*
*       Local functions
*
**********************************************************************
*/
/*********************************************************************
*
*       _MPU_Config
*
*  Function description:
*    This routine is provided by Keil as a workaround for faulty SDRAM
*    access on STM32F7xx. Otherwise a Hardfault will be caused by
*    unaligned access.
*/
static void _MPU_Config(void) {
  MPU_Region_InitTypeDef MPU_InitStruct;

  //
  // Disable the MPU
  //
  HAL_MPU_Disable();
  //
  // Configure the MPU attributes for SDRAM
  //
  MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
  MPU_InitStruct.BaseAddress      = 0xC0000000;
  MPU_InitStruct.Size             = MPU_REGION_SIZE_8MB;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.IsBufferable     = MPU_ACCESS_NOT_BUFFERABLE;
  MPU_InitStruct.IsCacheable      = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsShareable      = MPU_ACCESS_NOT_SHAREABLE;
  MPU_InitStruct.Number           = MPU_REGION_NUMBER0;
  MPU_InitStruct.TypeExtField     = MPU_TEX_LEVEL1;
  MPU_InitStruct.SubRegionDisable = 0x00;
  MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_DISABLE;
  //
  // Tell MPU how it should be configured
  //
  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  //
  // Enable the MPU
  //
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}

/*********************************************************************
*
*       _Init_SDRAM
*/
static void _Init_SDRAM(void) {
  //
  // calling STs SDRAM init
  //
  BSP_SDRAM_Init();
}

/*********************************************************************
*
*       _BSP_SDRAM_Init()
*
*  Function description
*    Does something.
*
*  Additional information
*    Used to explain things that are not necessarily self-explanatory and
*    require more explanation than what can easily be inserted in the code.
*
*  Notes
*    (1) Implementation notes that are for code maintenance and reference.
*/
static void _BSP_SDRAM_Init(void) {
  //
  // Set up MPU
  //
  _MPU_Config();
  //
  // Init SDRAM
  //
  _Init_SDRAM();
}

/*********************************************************************
*
*       Global functions
*
**********************************************************************
*/
void MemoryInit(void);
void MemoryInit(void) {
  _BSP_SDRAM_Init();
}

/*************************** End of file ****************************/
