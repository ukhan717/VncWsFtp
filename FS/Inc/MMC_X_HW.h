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
File        : MMC_X_HW.h
Purpose     : MMC hardware layer
-------------------------- END-OF-HEADER -----------------------------
*/
#ifndef MMC_X_HW_H               // Avoid recursive and multiple inclusion
#define MMC_X_HW_H

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

/*********************************************************************
*
*       Defines, configurable
*
**********************************************************************
*/
#ifndef   FS_MMC_REPORT_HW_ERRORS
  #define FS_MMC_REPORT_HW_ERRORS  0
#endif

#ifndef   FS_MMC_SUPPORT_LOCKING
  #define FS_MMC_SUPPORT_LOCKING   0
#endif

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

/*********************************************************************
*
*       Control line functions
*/
void FS_MMC_HW_X_EnableCS        (U8 Unit);
void FS_MMC_HW_X_DisableCS       (U8 Unit);

/*********************************************************************
*
*       Medium status functions
*/
int  FS_MMC_HW_X_IsPresent       (U8 Unit);
int  FS_MMC_HW_X_IsWriteProtected(U8 Unit);

/*********************************************************************
*
*       Configuration functions
*/
U16  FS_MMC_HW_X_SetMaxSpeed     (U8 Unit, U16 MaxFreq);
int  FS_MMC_HW_X_SetVoltage      (U8 Unit, U16 Vmin,   U16 Vmax);

/*********************************************************************
*
*       Data transfer functions
*/
#if FS_MMC_REPORT_HW_ERRORS
  int  FS_MMC_HW_X_Read          (U8 Unit,       U8 * pData, int NumBytes);
  int  FS_MMC_HW_X_Write         (U8 Unit, const U8 * pData, int NumBytes);
#else
  void FS_MMC_HW_X_Read          (U8 Unit,       U8 * pData, int NumBytes);
  void FS_MMC_HW_X_Write         (U8 Unit, const U8 * pData, int NumBytes);
#endif

/*********************************************************************
*
*       SPI bus locking
*/
#if FS_MMC_SUPPORT_LOCKING
  void FS_MMC_HW_X_Lock          (U8 Unit);
  void FS_MMC_HW_X_Unlock        (U8 Unit);
#endif

#if defined(__cplusplus)
}                /* Make sure we have C-declarations in C++ programs */
#endif

#endif // MMC_X_HW_H

/*************************** End of file ****************************/
