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

File    : FS_CheckDisk.c
Purpose : Sample program demonstrating disk checking functionality.

Additional information:
  Preparations:
    The file system has to be formatted first otherwise the checking
    operation fails with an error.

  Expected behavior:
    Checks the consistency of the file system structure and displays
    a message indicating the result of operation. In case of an error
    the user interaction is required to decide how the error has to
    be handled. The checking will always fail on a RAMDISK volume since
    the format information is lost when the application is started and
    the RAM is initialized.

  Sample output:
    Start
    File system structure is consistent.
    Finished
*/

/*********************************************************************
*
*       #include Section
*
**********************************************************************
*/
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include "SEGGER.h"
#include "FS.h"

/*********************************************************************
*
*       Defines, configurable
*
**********************************************************************
*/
#define VOLUME_NAME       ""
#define MAX_RECURSION     5

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
static U32 _aBuffer[2000 / 4];   // The more space is used the faster the disk checking can run.
static int _NumErrors;

/*********************************************************************
*
*       _OnError
*/
static int _OnError(int ErrCode, ...) {
  va_list      ParamList;
  const char * sFormat;
  int          c;
  char         ac[1000];

  //lint --e{438} -esym(530, ParamList)
  sFormat = FS_CheckDisk_ErrCode2Text(ErrCode);
  if (sFormat) {
    va_start(ParamList, ErrCode);
    SEGGER_vsnprintf(ac, sizeof(ac), sFormat, ParamList);
    va_end(ParamList);
    FS_X_Log(ac);
    FS_X_Log("\n");
  }
  if (ErrCode != FS_CHECKDISK_ERRCODE_CLUSTER_UNUSED) {
    FS_X_Log("  Do you want to repair this error? (y/n/a) ");
  } else {
    FS_X_Log("  * Convert lost cluster chain into file (y)\n"
             "  * Delete cluster chain                 (d)\n"
             "  * Do not repair                        (n)\n"
             "  * Abort                                (a) ");
    FS_X_Log("\n");
  }
  ++_NumErrors;
  c = getchar();
  FS_X_Log("\n");
  if ((c == 'y') || (c == 'Y')) {
    return FS_CHECKDISK_ACTION_SAVE_CLUSTERS;
  } else if ((c == 'a') || (c == 'A')) {
    return FS_CHECKDISK_ACTION_ABORT;
  } else if ((c == 'd') || (c == 'D')) {
    return FS_CHECKDISK_ACTION_DELETE_CLUSTERS;
  }
  return FS_CHECKDISK_ACTION_DO_NOT_REPAIR;
}

/*********************************************************************
*
*       MainTask
*/
#ifdef __cplusplus
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif
void MainTask(void);
#ifdef __cplusplus
}
#endif
void MainTask(void) {
  FS_X_Log("Start\n");
  FS_Init();
  _NumErrors = 0;
  //
  // Call the function repeatedly until all errors are fixed.
  //
  while (FS_CheckDisk(VOLUME_NAME, _aBuffer, sizeof(_aBuffer), MAX_RECURSION, _OnError) == FS_CHECKDISK_RETVAL_RETRY) {
    ;
  }
  if (_NumErrors == 0) {
    FS_X_Log("File system structure is consistent.\n");
  }
  FS_X_Log("Finished\n");
  while (1) {
    ;
  }
}

/*************************** End of file ****************************/

