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
File    : SEGGER_SYS_Win32.c
Purpose : Implementation for API functions.
Revision: $Rev: 11360 $
--------  END-OF-HEADER  ---------------------------------------------
*/

/*********************************************************************
*
*       #include Section
*
**********************************************************************
*/

#include "SEGGER_SYS.h"
#include <stdio.h>

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

static char _aCompilerID[40];

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

const char * SEGGER_SYS_GetCompiler(void) {
#if defined(__INTEL_COMPILER)
  #if defined(_W64)
    sprintf(_aCompilerID, "Intel C %d.%d.%d [x64]", __INTEL_COMPILER / 100, __INTEL_COMPILER % 100, __INTEL_COMPILER_UPDATE);
  #else
    sprintf(_aCompilerID, "Intel C %d.%d.%d [x86]", __INTEL_COMPILER / 100, __INTEL_COMPILER % 100, __INTEL_COMPILER_UPDATE);
#endif
#elif defined(__clang__)
  sprintf(_aCompilerID, "clang %s", __clang_version__);
#elif defined(__GNUC__)
  sprintf(_aCompilerID, "gcc %d.%d.%d", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
#elif defined(__IAR_SYSTEMS_ICC__)
  sprintf(_aCompilerID, "IAR icc %d.%d.%d", __VER__ / 1000000, __VER__ / 1000 % 1000, __VER__ % 1000);
#elif defined(_MSC_FULL_VER)
  #if defined(_WIN64)
    sprintf(_aCompilerID, "MSVC %d.%02d.%d [x64]", _MSC_FULL_VER / 10000000, _MSC_FULL_VER / 100000 % 100, _MSC_FULL_VER % 100000);
  #else
    sprintf(_aCompilerID, "MSVC %d.%02d.%d [x86]", _MSC_FULL_VER / 10000000, _MSC_FULL_VER / 100000 % 100, _MSC_FULL_VER % 100000);
  #endif
#else
  strcpy(_aCompilerID, "Cannot be identified");
#endif
  return _aCompilerID;
}
