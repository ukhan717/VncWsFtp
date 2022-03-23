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
File        : COMPRESS_ConfigIO.c
Purpose     : emCompress warning and debug configuration.
--------  END-OF-HEADER  ---------------------------------------------
*/

/*********************************************************************
*
*       #include Section
*
**********************************************************************
*/

#include "COMPRESS.h"
#include <stdio.h>
#include <stdlib.h>

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

/*********************************************************************
*
*       COMPRESS_AssertFail()
*
*  Function description
*    emCompress failure handler.
*
*  Parameters
*    sFilename  - Filename where assertion failed.
*    LineNumber - Line number where assertion failed.
*    sAssertion - Assertion that failed.
*/
void COMPRESS_AssertFail(const char *sFilename, int LineNumber, const char *sAssertion) {
#if defined(_WIN32) && _WIN32 > 0
  printf("COMPRESS: assertion failure: %s at %s:%d\n", sAssertion, sFilename, LineNumber);
  exit(100);
#else
  COMPRESS_USE_PARA(sFilename);
  COMPRESS_USE_PARA(LineNumber);
  COMPRESS_USE_PARA(sAssertion);
  for (;;) {
    /* Hang here */
  }
#endif
}

/*************************** End of file ****************************/
