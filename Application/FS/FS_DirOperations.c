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

File    : FS_DirOperations.c
Purpose : Demonstrates the usage of API functions that operate
          on directories.

Additional information:
  Preparations:
    Works out-of-the-box with any storage device.

  Expected behavior:
    This sample creates three directories. In each directory three
    files are created. After creating the directories and files,
    the contents of each directory is shown.

  Sample output:
    Start
    High-level format...OK
    Create directory \Dir00
    Create files ...OK
    Create directory \Dir01
    Create files ...OK
    Create directory \Dir02
    Create files ...OK
    Contents of
    DIR00 (Dir) Attributes: ---- Size: 0
    Contents of \DIR00
      . (Dir) Attributes: ---- Size: 0
      .. (Dir) Attributes: ---- Size: 0
      FILE0000.TXT       Attributes: A--- Size: 19
      FILE0001.TXT       Attributes: A--- Size: 19
      FILE0002.TXT       Attributes: A--- Size: 19

    DIR01 (Dir) Attributes: ---- Size: 0
    Contents of \DIR01
      . (Dir) Attributes: ---- Size: 0
      .. (Dir) Attributes: ---- Size: 0
      FILE0000.TXT       Attributes: A--- Size: 19
      FILE0001.TXT       Attributes: A--- Size: 19
      FILE0002.TXT       Attributes: A--- Size: 19

    DIR02 (Dir) Attributes: ---- Size: 0
    Contents of \DIR02
      . (Dir) Attributes: ---- Size: 0
      .. (Dir) Attributes: ---- Size: 0
      FILE0000.TXT       Attributes: A--- Size: 19
      FILE0001.TXT       Attributes: A--- Size: 19
      FILE0002.TXT       Attributes: A--- Size: 19


    Finished
*/

/*********************************************************************
*
*       #include Section
*
**********************************************************************
*/
#include <string.h>
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
#define NUM_DIRS          3
#define NUM_FILES         3

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
static char _acBuffer[512];

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/

/*********************************************************************
*
*       _CreateFiles
*/
static void _CreateFiles(const char * sPath) {
  int       i;
  FS_FILE * pFile;
  char      acFileName[40];

  FS_X_Log("Create files ");
  for (i = 0; i < NUM_FILES; i++) {
    SEGGER_snprintf(acFileName, sizeof(acFileName), "%s\\file%.4d.txt", sPath, i);
    pFile = FS_FOpen(acFileName, "w");
    FS_Write(pFile, acFileName, strlen(acFileName));
    FS_FClose(pFile);
    FS_X_Log(".");
  }
  FS_X_Log("OK\n");
}

/*********************************************************************
*
*       _ShowDir
*
*/
static void _ShowDir(const char * sDirName, int MaxRecursion) {
  FS_FIND_DATA fd;
  char         acFileName[20];
  char         acDummy[20];
  unsigned     NumBytes;
  int          r;

  NumBytes = (unsigned)(MAX_RECURSION - MaxRecursion);
  memset(acDummy, ' ', NumBytes);
  acDummy[NumBytes] = 0;
  SEGGER_snprintf(_acBuffer, sizeof(_acBuffer), "%sContents of %s \n", acDummy, sDirName);
  FS_X_Log(_acBuffer);
  if (MaxRecursion) {
    r = FS_FindFirstFile(&fd, sDirName, acFileName, sizeof(acFileName));
    if (r == 0) {
      do {
        U8 Attr;

        Attr = fd.Attributes;
        SEGGER_snprintf(_acBuffer, sizeof(_acBuffer), "%s %s %s Attributes: %s%s%s%s Size: %lu\n",
                                   acDummy, fd.sFileName,
                                   (Attr & FS_ATTR_DIRECTORY) ? "(Dir)" : "     ",
                                   (Attr & FS_ATTR_ARCHIVE)   ? "A" : "-",
                                   (Attr & FS_ATTR_READ_ONLY) ? "R" : "-",
                                   (Attr & FS_ATTR_HIDDEN)    ? "H" : "-",
                                   (Attr & FS_ATTR_SYSTEM)    ? "S" : "-",
                                   fd.FileSize);
        FS_X_Log(_acBuffer);
        if (Attr & FS_ATTR_DIRECTORY) {
          char acDirName[256];
          //
          // Show contents of each directory in the root
          //
          if (*fd.sFileName != '.') {
            SEGGER_snprintf(acDirName, sizeof(acDirName), "%s\\%s", sDirName, fd.sFileName);
            _ShowDir(acDirName, MaxRecursion - 1);
          }
        }

      } while (FS_FindNextFile(&fd));
      FS_FindClose(&fd);
    } else if (r == 1) {
      FS_X_Log("Directory is empty");
    } else {
      SEGGER_snprintf(_acBuffer, sizeof(_acBuffer), "Unable to open directory %s\n", sDirName);
      FS_X_Log(_acBuffer);
    }
    FS_X_Log("\n");
  }
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

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
  int i;
  int r;

  FS_X_Log("Start\n");
  FS_Init();
  FS_FormatLLIfRequired(VOLUME_NAME);
  //
  // High level volume format
  //
  FS_X_Log("High-level format...");
#if FS_SUPPORT_FAT
  r = FS_FormatSD(VOLUME_NAME);
#else
  r = FS_Format(VOLUME_NAME, NULL);
#endif
  if (r) {
    FS_X_Log("Could not format storage device\n");
  } else {
    FS_X_Log("OK\n");
    //
    //  Create 3 folders
    //
    for (i = 0; i < NUM_DIRS; i++) {
      char acDirName[20];

      SEGGER_snprintf(acDirName, sizeof(acDirName), "%s\\Dir%.2d", VOLUME_NAME, i);
      SEGGER_snprintf(_acBuffer, sizeof(_acBuffer), "Create directory %s\n", acDirName);
      FS_X_Log(_acBuffer);
      r = FS_MkDir(acDirName);
      //
      // If directory has been successfully created
      // Create the files in that directory.
      //
      if (r == 0) {
        _CreateFiles(acDirName);
      } else {
        SEGGER_snprintf(_acBuffer, sizeof(_acBuffer), "Could not create the directory %s\n", acDirName);
        FS_X_Log(_acBuffer);
      }
    }
    //
    // Show contents of root directory
    //
    _ShowDir(VOLUME_NAME, MAX_RECURSION);
  }
  FS_X_Log("Finished\n");
  while (1) {
    ;
  }
}

/*************************** End of file ****************************/
