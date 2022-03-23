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
File        : MMC_SD_CardMode_X_HW.h
Purpose     : MMC hardware layer
-------------------------- END-OF-HEADER -----------------------------
*/
#ifndef MMC_SD_CARDMODE_X_HW_H              // Avoid recursive and multiple inclusion
#define MMC_SD_CARDMODE_X_HW_H

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

/*********************************************************************
*
*       Defines, fixed
*
**********************************************************************
*/

/*********************************************************************
*
*       Card mode error codes
*
*  Description
*    Values that indicate the result of an operation.
*/
#define FS_MMC_CARD_NO_ERROR                      0   // Success.
#define FS_MMC_CARD_RESPONSE_TIMEOUT              1   // No response received.
#define FS_MMC_CARD_RESPONSE_CRC_ERROR            2   // CRC error in response detected.
#define FS_MMC_CARD_READ_TIMEOUT                  3   // No data received.
#define FS_MMC_CARD_READ_CRC_ERROR                4   // CRC error in received data detected.
#define FS_MMC_CARD_WRITE_CRC_ERROR               5   // Card detected an CRC error in the received data.
#define FS_MMC_CARD_RESPONSE_GENERIC_ERROR        6   // Start bit, end bit or command index error.
#define FS_MMC_CARD_READ_GENERIC_ERROR            7   // An error occurred during while receiving data from card.
#define FS_MMC_CARD_WRITE_GENERIC_ERROR           8   // An error occurred during while sending data to card.

/*********************************************************************
*
*       Card mode response formats
*
*  Description
*    Response types returned by different commands.
*/
#define FS_MMC_RESPONSE_FORMAT_NONE               0                           // No response expected.
#define FS_MMC_RESPONSE_FORMAT_R1                 1                           // Card status (48-bit large)
#define FS_MMC_RESPONSE_FORMAT_R2                 2                           // CID or CSD register (128-bit large)
#define FS_MMC_RESPONSE_FORMAT_R3                 3                           // OCR register (48-bit large)
#define FS_MMC_RESPONSE_FORMAT_R6                 FS_MMC_RESPONSE_FORMAT_R1   // Published RCA response (48-bit large)
#define FS_MMC_RESPONSE_FORMAT_R7                 FS_MMC_RESPONSE_FORMAT_R1   // Card interface condition (48-bit large)

/*********************************************************************
*
*       Card mode command flags
*
*  Description
*    Additional options for the executed command.
*/
#define FS_MMC_CMD_FLAG_DATATRANSFER              (1 << 0)    // Command that exchanges data with the card.
#define FS_MMC_CMD_FLAG_WRITETRANSFER             (1 << 1)    // Command that sends data to card. Implies FS_MMC_CMD_FLAG_DATATRANSFER.
#define FS_MMC_CMD_FLAG_SETBUSY                   (1 << 2)    // Command that expects an R1b command.
#define FS_MMC_CMD_FLAG_INITIALIZE                (1 << 3)    // Indicates that the initialization delay has to be performed. According to SD specification this is the maximum of 1 millisecond, 74 clock cycles and supply ramp up time.
#define FS_MMC_CMD_FLAG_USE_SD4MODE               (1 << 4)    // Command that transfers the data via 4 data lines.
#define FS_MMC_CMD_FLAG_STOP_TRANS                (1 << 5)    // Command that stops a data transfer (typically CMD12)
#define FS_MMC_CMD_FLAG_WRITE_BURST_REPEAT        (1 << 6)    // Indicates that the same sector data is written to consecutive sector indexes.
#define FS_MMC_CMD_FLAG_USE_MMC8MODE              (1 << 7)    // Command that transfers the data via 8 data lines (MMC only).
#define FS_MMC_CMD_FLAG_NO_CRC_CHECK              (1 << 8)    // CRC verification has to be disabled for the command. Typically used with the MMC bus test commands.
#define FS_MMC_CMD_FLAG_WRITE_BURST_FILL          (1 << 9)    // Indicates that the 32-bit value is used to fill the contents of consecutive sector indexes.

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
void   FS_MMC_HW_X_InitHW             (U8 Unit);
void   FS_MMC_HW_X_Delay              (int ms);

/*********************************************************************
*
*       Card status functions
*/
int    FS_MMC_HW_X_IsPresent          (U8 Unit);
int    FS_MMC_HW_X_IsWriteProtected   (U8 Unit);

/*********************************************************************
*
*       Configuration functions
*/
U16    FS_MMC_HW_X_SetMaxSpeed        (U8 Unit, U16 Freq);
void   FS_MMC_HW_X_SetResponseTimeOut (U8 Unit, U32 Value);
void   FS_MMC_HW_X_SetReadDataTimeOut (U8 Unit, U32 Value);

/*********************************************************************
*
*       Command execution functions
*/
void   FS_MMC_HW_X_SendCmd            (U8 Unit, unsigned Cmd, unsigned CmdFlags, unsigned ResponseType, U32 Arg);
int    FS_MMC_HW_X_GetResponse        (U8 Unit, void *pBuffer, U32 Size);

/*********************************************************************
*
*       Data transfer functions
*/
int    FS_MMC_HW_X_ReadData           (U8 Unit,       void * pBuffer, unsigned NumBytes, unsigned NumBlocks);
int    FS_MMC_HW_X_WriteData          (U8 Unit, const void * pBuffer, unsigned NumBytes, unsigned NumBlocks);
void   FS_MMC_HW_X_SetDataPointer     (U8 Unit, const void * p);
void   FS_MMC_HW_X_SetHWBlockLen      (U8 Unit, U16 BlockSize);
void   FS_MMC_HW_X_SetHWNumBlocks     (U8 Unit, U16 NumBlocks);

/*********************************************************************
*
*       Query functions
*/
U16    FS_MMC_HW_X_GetMaxReadBurst    (U8 Unit);
U16    FS_MMC_HW_X_GetMaxWriteBurst   (U8 Unit);

#if defined(__cplusplus)
}                /* Make sure we have C-declarations in C++ programs */
#endif

#endif // MMC_SD_CARDMODE_X_HW_H

/*************************** End of file ****************************/
