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

File        : CRYPTO_OS_embOS.c
Purpose     : SEGGER embOS CRYPTO-OS binding.

*/

/*********************************************************************
*
*       #include section
*
**********************************************************************
*/

#include "CRYPTO_Int.h"
#include "RTOS.h"

/*********************************************************************
*
*       Preprocessor definitions, configurable
*
**********************************************************************
*/

#ifndef   CRYPTO_CONFIG_OS_MAX_UNIT
  #define CRYPTO_CONFIG_OS_MAX_UNIT     (CRYPTO_OS_MAX_INTERNAL_UNIT + 3)
#endif

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

static OS_RSEMA _aSema[CRYPTO_CONFIG_OS_MAX_UNIT];

/*********************************************************************
*
*       Public functions
*
**********************************************************************
*/

/*********************************************************************
*
*       CRYPTO_OS_Claim()
*
*  Function description
*    Claim a hardware resource.
*
*  Parameters
*    Unit - Zero-based index to hardware resource.
*/
void CRYPTO_OS_Claim(unsigned Unit) {
  CRYPTO_ASSERT(Unit < CRYPTO_CONFIG_OS_MAX_UNIT);
  //
  OS_Use(&_aSema[Unit]);
}

/*********************************************************************
*
*       CRYPTO_OS_Request()
*
*  Function description
*    Request a hardware resource.
*
*  Parameters
*    Unit - Zero-based index to hardware resource.
*
*  Return value
*    == 0 - Resource is already in use and was not claimed.
*    != 0 - Resource claimed.
*/
int CRYPTO_OS_Request(unsigned Unit) {
  CRYPTO_ASSERT(Unit < CRYPTO_CONFIG_OS_MAX_UNIT);
  //
  return OS_Request(&_aSema[Unit]);
}

/*********************************************************************
*
*       CRYPTO_OS_Unclaim()
*
*  Function description
*    Release claim on a hardware resource.
*
*  Parameters
*    Unit - Zero-based index to hardware resource.
*/
void CRYPTO_OS_Unclaim(unsigned Unit) {
  CRYPTO_ASSERT(Unit < CRYPTO_CONFIG_OS_MAX_UNIT);
  //
  OS_Unuse(&_aSema[Unit]);
}

/*********************************************************************
*
*       CRYPTO_OS_Init()
*
*  Function description
*    Initialize CRYPTO binding to OS.
*/
void CRYPTO_OS_Init(void) {
  unsigned Unit;
  //
  for (Unit = 0; Unit < CRYPTO_CONFIG_OS_MAX_UNIT; ++Unit) {
    OS_CreateRSema(&_aSema[Unit]);
  }
}

/*************************** End of file ****************************/
