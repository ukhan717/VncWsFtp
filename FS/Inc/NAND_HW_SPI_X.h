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
File        : NAND_HW_SPI_X.h
Purpose     : Hardware layer for SPI NAND devices
-------------------------- END-OF-HEADER -----------------------------
*/

#ifndef NAND_HW_SPI_X_H               // Avoid recursive and multiple inclusion
#define NAND_HW_SPI_X_H

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

/*********************************************************************
*
*       Initialization and control functions
*/
int  FS_NAND_HW_SPI_X_Init     (U8 Unit);
void FS_NAND_HW_SPI_X_DisableCS(U8 Unit);
void FS_NAND_HW_SPI_X_EnableCS (U8 Unit);
void FS_NAND_HW_SPI_X_Delay    (U8 Unit, int ms);

/*********************************************************************
*
*       Data transfer functions
*/
int  FS_NAND_HW_SPI_X_Read     (U8 Unit,       void * pData, unsigned NumBytes);
int  FS_NAND_HW_SPI_X_Write    (U8 Unit, const void * pData, unsigned NumBytes);

#if defined(__cplusplus)
}                /* Make sure we have C-declarations in C++ programs */
#endif

#endif // NAND_HW_SPI_X_H

/*************************** End of file ****************************/
