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
File        : NOR_HW_SPI_X.h
Purpose     : Serial NOR SPI hardware layer
-------------------------- END-OF-HEADER -----------------------------
*/

#ifndef NOR_HW_SPI_X_H
#define NOR_HW_SPI_X_H

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
int           FS_NOR_SPI_HW_X_Init       (U8 Unit);
void          FS_NOR_SPI_HW_X_EnableCS   (U8 Unit);
void          FS_NOR_SPI_HW_X_DisableCS  (U8 Unit);

/*********************************************************************
*
*       Data transfer functions
*/
void          FS_NOR_SPI_HW_X_Read      (U8 Unit,       U8 * pData, int NumBytes);
void          FS_NOR_SPI_HW_X_Write     (U8 Unit, const U8 * pData, int NumBytes);
void          FS_NOR_SPI_HW_X_Read_x2   (U8 Unit,       U8 * pData, int NumBytes);
void          FS_NOR_SPI_HW_X_Write_x2  (U8 Unit, const U8 * pData, int NumBytes);
void          FS_NOR_SPI_HW_X_Read_x4   (U8 Unit,       U8 * pData, int NumBytes);
void          FS_NOR_SPI_HW_X_Write_x4  (U8 Unit, const U8 * pData, int NumBytes);

#if defined(__cplusplus)
}                /* Make sure we have C-declarations in C++ programs */
#endif

#endif  // NOR_HW_SPI_X_H

/*************************** End of file ****************************/
