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

File    : FS_PerformanceSimple.c
Purpose : Sample program that can be used to measure the performance
          of the file system.

Additional information:
  Preparations:
    Works out-of-the-box with any storage device.

  Expected behavior:
    Measures the speed at witch the file system can write and read blocks
    of data to and from a file. The size of the file as well as the number
    of bytes that have to be written at once are configurable.

    The application always formats the storage device to make sure that
    the measurements are not influenced by the data already stored on the
    file system.

  Sample output:
    Start
    High-level format
    Writing 16 chunks of 524288 bytes...................OK
    Reading 16 chunks of 524288 bytes...................OK

    W Speed: 2115 Kbyte/s
    R Speed: 11130 Kbyte/s
    Finished
*/

/*********************************************************************
*
*       #include Section
*
**********************************************************************
*/
#include <string.h>
#include "FS.h"
#include "FS_OS.h"
#include "SEGGER.h"

/*********************************************************************
*
*       Defines, configurable
*
**********************************************************************
*/
#define FILE_SIZE             (8192L * 1024L)             // Defines the file size that should be used
#define BLOCK_SIZE            (8L    * 1024L)             // Block size for individual read / write operation in bytes
#define NUM_BLOCKS_MEASURE    (64)                        // Number of blocks for individual measurement
#ifndef   VOLUME_NAME                                     // Defines the volume that should be used
  #define VOLUME_NAME         ""
#endif
#define FILE_NAME             VOLUME_NAME"\\default.txt"  // Defines the name of the file to write to

/*********************************************************************
*
*       Types
*
**********************************************************************
*/

typedef struct {
  const char * sName;
  I32          Min;
  I32          Max;
  I32          Av;
  I32          Sum;
  I32          NumSamples;
  U32          NumBytes;
} RESULT;

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
static U32    _aBuffer[BLOCK_SIZE / 4];
static RESULT _aResult[2];
static int    _TestNo;
static char   _ac[512];

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/

/*********************************************************************
*
*       _WriteFile
*
*  Function description
*    Measures the write time.
*/
static I32 _WriteFile(FS_FILE * pFile, const void * pData, U32 NumBytes, U32 NumBlocksMeasure) {
  I32 t;
  U32 i;

  t = (I32)FS_X_OS_GetTime();
  for (i = 0; i < NumBlocksMeasure; i++) {
    FS_Write(pFile, pData, NumBytes);
  }
  return (I32)FS_X_OS_GetTime() - t;
}


/*********************************************************************
*
*       _ReadFile
*
*  Function description
*    Measures the read performance.
*/
static I32 _ReadFile(FS_FILE * pFile, void * pData, U32 NumBytes, U32 NumBlocksMeasure) {
  I32 t;
  U32 i;

  t = (I32)FS_X_OS_GetTime();
  for (i = 0; i < NumBlocksMeasure; i++) {
    FS_Read(pFile, pData, NumBytes);
  }
  return (I32)FS_X_OS_GetTime() - t;
}
/*********************************************************************
*
*       _StartTest
*/
static void _StartTest(const char * sName, U32 NumBytes) {
  RESULT * pResult;

  if ((_TestNo + 1) < (int)SEGGER_COUNTOF(_aResult)) {
    pResult = &_aResult[++_TestNo];
    pResult->sName = sName;
    pResult->Min =  0x7fffffff;
    pResult->Max = -0x7fffffff;
    pResult->NumSamples = 0;
    pResult->Sum = 0;
    pResult->NumBytes = NumBytes;
  }
}

/*********************************************************************
*
*       _StoreResult
*/
static void _StoreResult(I32 t) {
  RESULT * pResult;

  pResult = &_aResult[_TestNo];
  if (t > pResult->Max) {
    pResult->Max = t;
  }
  if (t < pResult->Min) {
    pResult->Min = t;
  }
  pResult->NumSamples++;
  pResult->Sum += (I32)t;
  pResult->Av   = pResult->Sum / pResult->NumSamples;
}

/*********************************************************************
*
*       _GetAverage
*/
static double _GetAverage(int Index) {
  RESULT * pResult;
  double v;

  pResult = &_aResult[Index];
  v = (double)pResult->Av;
  if (v == 0) {
    return 0;
  }
  v = (double)1000.0 / v;
  v = v * (pResult->NumBytes >> 10);
  return v;
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
  int       i;
  U32       Space;
  int       NumLoops;
  U32       NumBytes;
  U32       NumBytesAtOnce;
  FS_FILE * pFile;
  I32       t;
  int       r;
  U32       NumBlocksMeasure;

  FS_X_Log("Start\n");
  FS_Init();
  _TestNo = -1;
  //
  // Check if we need to low-level format the volume
  //
  if (FS_IsLLFormatted(VOLUME_NAME) == 0) {
    FS_X_Log("Low-level format\n");
    FS_FormatLow(VOLUME_NAME);  /* Erase & Low-level  format the flash */
  }
  //
  // Volume is always high level formatted
  // before doing any performance tests.
  //
  FS_X_Log("High-level format\n");
#if FS_SUPPORT_FAT
  r = FS_FormatSD(VOLUME_NAME);
#else
  r = FS_Format(VOLUME_NAME, NULL);
#endif
  if (r == 0) {
    //
    // Configure the file system so that the
    // directory entry and the allocation table
    // are updated when the file is closed.
    //
    FS_SetFileWriteMode(FS_WRITEMODE_FAST);
    //
    // Fill the buffer with data.
    //
    memset((void*)_aBuffer, 'a', sizeof(_aBuffer));
    //
    // Get some general info.
    //
    Space            = FS_GetVolumeFreeSpace(VOLUME_NAME);
    Space            = SEGGER_MIN(Space, FILE_SIZE);
    NumBytesAtOnce   = BLOCK_SIZE;
    NumBlocksMeasure = NUM_BLOCKS_MEASURE;
    while (1) {
      NumBytes = NumBytesAtOnce * NumBlocksMeasure;
      if (NumBytes <= Space) {
        break;
      }
      NumBytesAtOnce   >>= 1;
      NumBlocksMeasure >>= 1;
    }
    NumLoops         = (int)(Space / NumBytes);
    if (NumLoops) {
      //
      // Create file of full size.
      //
      _StartTest("W", NumBytes);
      pFile = FS_FOpen(FILE_NAME, "w");
      //
      // Preallocate the file, setting the file pointer to the highest position
      // and declare it as the end of the file.
      //
      FS_FSeek(pFile, (I32)Space, FS_SEEK_SET);
      FS_SetEndOfFile(pFile);
      //
      // Set file position to the beginning.
      //
      FS_FSeek(pFile, 0, FS_SEEK_SET);
      //
      // Check write performance with clusters/file size preallocated.
      //
      SEGGER_snprintf(_ac, sizeof(_ac), "Writing %d chunks of %lu bytes...", NumLoops, NumBytes);
      FS_X_Log(_ac);
      for (i = 0; i < NumLoops ; i++) {
        t = _WriteFile(pFile, &_aBuffer[0], NumBytesAtOnce, NumBlocksMeasure);
        _StoreResult(t);
        FS_X_Log(".");
      }
      FS_X_Log("OK\n");
      FS_FClose(pFile);
      //
      // Check read performance.
      //
      _StartTest("R", NumBytes);
      SEGGER_snprintf(_ac, sizeof(_ac), "Reading %d chunks of %lu bytes..." , NumLoops, NumBytes);
      FS_X_Log(_ac);
      pFile = FS_FOpen(FILE_NAME, "r");
      for (i = 0; i < NumLoops; i++) {
        t = _ReadFile(pFile, _aBuffer, NumBytesAtOnce, NumBlocksMeasure);
        _StoreResult(t);
        FS_X_Log(".");
      }
      FS_X_Log("OK\n\n");
      FS_FClose(pFile);
      FS_Remove(FILE_NAME);
      //
      // Show results for performance list.
      //
      for (i = 0; i <= _TestNo; i++) {
        SEGGER_snprintf(_ac, sizeof(_ac), "%s Speed: %d Kbyte/s\n", _aResult[i].sName, (int)_GetAverage(i));
        FS_X_Log(_ac);
      }
    } else {
      FS_X_Log("Not enough free space available on the storage.\n");
    }
    FS_Unmount(VOLUME_NAME);
  } else {
    FS_X_Log("Volume could not be formatted!\n");
  }
  FS_X_Log("Finished\n");
  while (1) {
    ;
  }
}

/*************************** End of file ****************************/
