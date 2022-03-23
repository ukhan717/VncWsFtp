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

File    : FS_Performance.c
Purpose : Sample program that can be used to measure the performance
          of the file system.

Additional information:
  Preparations:
    Works out-of-the-box with any storage device.

  Expected behavior:
    The sample application performs the following tests:

    1. Checking the write performance by writing into a file
    which has been created by pre-allocating clusters.
    This is done in order to make sure that when writing to the
    file, no further write operations to the allocation table
    or to the directory entry are necessary. These additional
    operations can negatively affect the performance.
    The directory entry is updated when the file is closed.

    2. Checking the write performance by writing into a file
    which has been created without cluster pre-allocation.
    The directory entry is updated when the file is closed.

    3. Same as test 1. but with the directory entry being updated
    on each write operation.

    4. Same as test 2. but with the directory entry being updated
    on each write operation.

    5. Checking the read performance by reading the contents of
    the file that has been previously created during the write
    tests.

    By default, in all test cases the file is written or read
    in chunks as follows:
      16 chunks (512 KB per chunk)
      1 chunk = 64 blocks = 64 calls of FS_FWrite/FS_FRead
      1 block = 8 KB = 8 KB per write/read transaction.
    with a total file size of 8 MB.

    The application always formats the storage device to make
    sure that the measurements are not influenced by the data
    already stored on the file system.

  Sample output:
    Start
    High-level format
    T0: Write 16 chunks of 524288 bytes................OK
    T1: Write 16 chunks of 524288 bytes................OK
    T2: Write 16 chunks of 524288 bytes................OK
    T3: Write 16 chunks of 524288 bytes................OK
    T4: Read  16 chunks of 524288 bytes................OK
    Test 0: Write, pre-allocated clusters
      Chunk (512 Kbytes)
        Time (Min/Max/Av):   164/175/169 ms
        Speed:               3029.59 Kbytes/s
      Block (8 Kbytes)
        Time (Min/Max/Av):   0/13/2 ms
        Speed:               4000.00 Kbytes/s
      Counters
        ReadOperationCnt:    2
        ReadSectorCnt:       2
        WriteOperationCnt:   1024
        WriteSectorCnt:      16384

    Test 1: Write, clusters dynamically allocated, fast write mode
      Chunk (512 Kbytes)
        Time (Min/Max/Av):   163/199/171 ms
        Speed:               2994.15 Kbytes/s
      Block (8 Kbytes)
        Time (Min/Max/Av):   0/16/2 ms
        Speed:               4000.00 Kbytes/s
      Counters
        ReadOperationCnt:    7
        ReadSectorCnt:       7
        WriteOperationCnt:   1028
        WriteSectorCnt:      16388

    Test 2: Write, clusters dynamically allocated, medium write mode
      Chunk (512 Kbytes)
        Time (Min/Max/Av):   258/271/264 ms
        Speed:               1939.39 Kbytes/s
      Block (8 Kbytes)
        Time (Min/Max/Av):   1/12/4 ms
        Speed:               2000.00 Kbytes/s
      Counters
        ReadOperationCnt:    263
        ReadSectorCnt:       263
        WriteOperationCnt:   1283
        WriteSectorCnt:      16643

    Test 3: Write, clusters dynamically allocated, safe write mode
      Chunk (512 Kbytes)
        Time (Min/Max/Av):   425/685/504 ms
        Speed:               1015.87 Kbytes/s
      Block (8 Kbytes)
        Time (Min/Max/Av):   3/115/7 ms
        Speed:               1142.86 Kbytes/s
      Counters
        ReadOperationCnt:    1286
        ReadSectorCnt:       1286
        WriteOperationCnt:   2306
        WriteSectorCnt:      17666

    Test 4: Read
      Chunk (512 Kbytes)
        Time (Min/Max/Av):   45/49/46 ms
        Speed:               11130 Kbytes/s
      Block (8 Kbytes)
        Time (Min/Max/Av):   0/2/0 ms
        Speed:               0 Kbytes/s
      Counters
        ReadOperationCnt:    1027
        ReadSectorCnt:       16387
        WriteOperationCnt:   0
        WriteSectorCnt:      0

    Test 0 Speed (chunk/block): 3029/4000 Kbytes/s
    Test 1 Speed (chunk/block): 2994/4000 Kbytes/s
    Test 2 Speed (chunk/block): 1939/2000 Kbytes/s
    Test 3 Speed (chunk/block): 1015/1142 Kbytes/s
    Test 4 Speed (chunk/block): 11130/0 Kbytes/s

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
#define NUM_BLOCKS_MEASURE    64                          // Number of blocks for individual measurement
#define VOLUME_NAME           ""                          // Defines the volume that should be used
#define FILE_NAME             VOLUME_NAME"\\default.txt"  // Defines the name of the file to write to

/*********************************************************************
*
*       Types
*
**********************************************************************
*/
typedef struct {
  const char * sName;
  I32 MinChunk;
  I32 MaxChunk;
  I32 AvChunk;
  I32 SumChunk;
  I32 NumSamplesChunk;
  U32 NumBytesChunk;
  I32 MinBlock;
  I32 MaxBlock;
  I32 AvBlock;
  I32 SumBlock;
  I32 NumSamplesBlock;
  U32 NumBytesBlock;
  FS_STORAGE_COUNTERS StorageCounter;
} RESULT;

/*********************************************************************
*
*       static data
*
**********************************************************************
*/
static FS_FILE * _pFile;
static U32       _aBuffer[BLOCK_SIZE / 4];
static RESULT    _aResult[5];
static int       _TestNo;
static char      _ac[512];
static U32       _Space;
static U32       _NumLoops;
static U32       _NumBytes;
static U32       _NumBytesAtOnce;
static U32       _NumBlocksMeasure;

/*********************************************************************
*
*             Local functions
*
**********************************************************************
*/

/*********************************************************************
*
*       _StartTest
*/
static void _StartTest(const char * sName, U32 NumBytesChunk, U32 NumBytesBlock) {
  RESULT * pResult;

  if ((_TestNo + 1) < (int)SEGGER_COUNTOF(_aResult)) {
    pResult = &_aResult[++_TestNo];
    pResult->sName           = sName;
    pResult->MinChunk        =  0x7fffffff;
    pResult->MaxChunk        = -0x7fffffff;
    pResult->NumSamplesChunk = 0;
    pResult->SumChunk        = 0;
    pResult->NumBytesChunk   = NumBytesChunk;
    pResult->MinBlock        =  0x7fffffff;
    pResult->MaxBlock        = -0x7fffffff;
    pResult->NumSamplesBlock = 0;
    pResult->SumBlock        = 0;
    pResult->NumBytesBlock   = NumBytesBlock;
  }
}

/*********************************************************************
*
*       _StoreResultChunk
*/
static void _StoreResultChunk(I32 t) {
  RESULT * pResult;

  pResult = &_aResult[_TestNo];
  if (t < 0) {
    FS_X_Panic(110);
  }
  if (t > pResult->MaxChunk) {
    pResult->MaxChunk = t;
  }
  if (t < pResult->MinChunk) {
    pResult->MinChunk = t;
  }
  pResult->NumSamplesChunk++;
  pResult->SumChunk += (I32)t;
  pResult->AvChunk   = pResult->SumChunk / pResult->NumSamplesChunk;
}

/*********************************************************************
*
*       _GetAverageChunk
*/
static float _GetAverageChunk(int Index) {
  RESULT * pResult;
  float    v;

  pResult = &_aResult[Index];
  v = (float)pResult->AvChunk;
  if (v == 0) {
    return 0;
  }
  v = (float)1000.0 / v;
  v = v * (pResult->NumBytesChunk >> 10);
  return v;
}

/*********************************************************************
*
*       _StoreResultBlock
*/
static void _StoreResultBlock(I32 t) {
  RESULT * pResult;

  pResult = &_aResult[_TestNo];
  if (t > pResult->MaxBlock) {
    pResult->MaxBlock = t;
  }
  if (t < pResult->MinBlock) {
    pResult->MinBlock = t;
  }
  pResult->NumSamplesBlock++;
  pResult->SumBlock += (I32)t;
  pResult->AvBlock   = pResult->SumBlock / pResult->NumSamplesBlock;
}

/*********************************************************************
*
*       _GetAverageBlock
*/
static float _GetAverageBlock(int Index) {
  RESULT * pResult;
  float v;

  pResult = &_aResult[Index];
  v = (float)pResult->AvBlock;
  if (v == 0) {
    return 0;
  }
  v = (float)1000.0 / v;
  v = v * (pResult->NumBytesBlock >> 10);
  return v;
}

/*********************************************************************
*
*       _WriteFile
*
*   Function description
*     Write to file and measure time it takes to complete the operation
*/
static I32 _WriteFile(const void * pData, U32 NumBytes) {
  I32 t0;
  I32 t1;
  U32 i;

  t0 = (I32)FS_X_OS_GetTime();
  for (i = 0; i < _NumBlocksMeasure; i++) {
    t1 = (I32)FS_X_OS_GetTime();
    FS_Write(_pFile, pData, NumBytes);
    _StoreResultBlock((I32)FS_X_OS_GetTime() - t1);
  }
  return (I32)FS_X_OS_GetTime() - t0;
}

/*********************************************************************
*
*       _ReadFile
*
*   Function description
*     Read from file and measure time it takes to complete the operation
*/
static I32 _ReadFile(void * pData, U32 NumBytes) {
  I32 t0;
  I32 t1;
  U32 i;

  t0 = (I32)FS_X_OS_GetTime();
  for (i = 0; i < _NumBlocksMeasure; i++) {
    t1 = (I32)FS_X_OS_GetTime();
    FS_Read(_pFile, pData, NumBytes);
    _StoreResultBlock((I32)FS_X_OS_GetTime() - t1);
  }
  return (I32)FS_X_OS_GetTime() - t0;
}

/*********************************************************************
*
*       _TestWriteWithPreAllocation
*/
static void _TestWriteWithPreAllocation(const char * sName) {
  unsigned i;
  I32      t;

  _StartTest(sName, _NumBytes, _NumBytesAtOnce);
  //
  // Create file of full size
  //
  _pFile = FS_FOpen(FILE_NAME, "w");
  FS_FSeek(_pFile, (I32)_Space, FS_SEEK_SET);
  FS_SetEndOfFile(_pFile);
  FS_FSeek(_pFile, 0, FS_SEEK_SET);
  SEGGER_snprintf(_ac, sizeof(_ac), "T%d: Write %lu chunks of %lu bytes", _TestNo, _NumLoops, _NumBytes);
  FS_X_Log(_ac);
  FS_STORAGE_ResetCounters();
  for (i = 0; i < _NumLoops ; i++) {
    t = _WriteFile(&_aBuffer[0], _NumBytesAtOnce);
    _StoreResultChunk(t);
    FS_X_Log(".");
  }
  FS_STORAGE_GetCounters(&_aResult[_TestNo].StorageCounter);
  FS_X_Log("OK\n");
  FS_FClose(_pFile);
}

/*********************************************************************
*
*       _TestWriteWithDynamicAllocation
*/
static void _TestWriteWithDynamicAllocation(const char * sName) {
  unsigned i;
  I32      t;

  _StartTest(sName, _NumBytes, _NumBytesAtOnce);
  _pFile = FS_FOpen(FILE_NAME, "w");
  SEGGER_snprintf(_ac, sizeof(_ac), "T%d: Write %lu chunks of %lu bytes", _TestNo, _NumLoops, _NumBytes);
  FS_X_Log(_ac);
  FS_STORAGE_ResetCounters();
  for (i = 0; i < _NumLoops ; i++) {
    t = _WriteFile(&_aBuffer[0], _NumBytesAtOnce);
    _StoreResultChunk(t);
    FS_X_Log(".");
  }
  FS_STORAGE_GetCounters(&_aResult[_TestNo].StorageCounter);
  FS_X_Log("OK\n");
  FS_FClose(_pFile);
}

/*********************************************************************
*
*       _TestRead
*/
static void _TestRead(const char * sName) {
  unsigned i;
  I32      t;

  _StartTest(sName, _NumBytes, _NumBytesAtOnce);
  SEGGER_snprintf(_ac, sizeof(_ac), "T%d: Read  %lu chunks of %lu bytes", _TestNo, _NumLoops, _NumBytes);
  FS_X_Log(_ac);
  _pFile = FS_FOpen(FILE_NAME, "r");
  FS_STORAGE_ResetCounters();
  for (i = 0; i < _NumLoops; i++) {
    t = _ReadFile(_aBuffer, _NumBytesAtOnce);
    _StoreResultChunk(t);
    FS_X_Log(".");
  }
  FS_STORAGE_GetCounters(&_aResult[_TestNo].StorageCounter);
  FS_X_Log("OK\n");
  FS_FClose(_pFile);
}

/*********************************************************************
*
*       _ShowResults
*/
static void _ShowResults(void) {
  int i;

  //
  // Show measurement results.
  //
  for (i = 0; i <= _TestNo; i++) {
    SEGGER_snprintf(_ac, sizeof(_ac),
                    "Test %d: %s\n"
                    "  Chunk (%lu Kbytes)\n"
                    "    Time (Min/Max/Av):   %ld/%ld/%ld ms\n"
                    "    Speed:               %d Kbytes/s\n"
                    "  Block (%lu Kbytes)\n"
                    "    Time (Min/Max/Av):   %ld/%ld/%ld ms\n"
                    "    Speed:               %d Kbytes/s\n"
                    "  Counters\n"
                    "    ReadOperationCnt:    %lu\n"
                    "    ReadSectorCnt:       %lu\n"
                    "    WriteOperationCnt:   %lu\n"
                    "    WriteSectorCnt:      %lu\n", i, _aResult[i].sName,
                                                      ((_Space / _NumLoops) >> 10),
                                                      _aResult[i].MinChunk,
                                                      _aResult[i].MaxChunk,
                                                      _aResult[i].AvChunk,
                                                      (int)_GetAverageChunk(i),
                                                      _NumBytesAtOnce >> 10,
                                                      _aResult[i].MinBlock,
                                                      _aResult[i].MaxBlock,
                                                      _aResult[i].AvBlock,
                                                      (int)_GetAverageBlock(i),
                                                      _aResult[i].StorageCounter.ReadOperationCnt,
                                                      _aResult[i].StorageCounter.ReadSectorCnt,
                                                      _aResult[i].StorageCounter.WriteOperationCnt,
                                                      _aResult[i].StorageCounter.WriteSectorCnt);
    FS_X_Log(_ac);
    FS_X_Log("\n");
  }
  //
  // Show summary.
  //
  for (i = 0; i <= _TestNo; i++) {
    SEGGER_snprintf(_ac, sizeof(_ac), "Test %d Speed (chunk/block): %d/%d Kbytes/s\n",
                    i,
                    (int)_GetAverageChunk(i),
                    (int)_GetAverageBlock(i));
    FS_X_Log(_ac);
  }
  FS_X_Log("\n");
}

/*********************************************************************
*
*             Global functions
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
  int r;

  FS_X_Log("Start\n");
  FS_Init();
  _TestNo = -1;
  if (FS_IsLLFormatted(VOLUME_NAME) == 0) {
    FS_X_Log("Low-level format\n");
    FS_FormatLow(VOLUME_NAME);
  }
  FS_X_Log("High-level format\n");
#if FS_SUPPORT_FAT
  r = FS_FormatSD(VOLUME_NAME);
#else
  r = FS_Format(VOLUME_NAME, NULL);
#endif
  if (r == 0) {
    memset((void*)_aBuffer, 'a', sizeof(_aBuffer));
    //
    // Get information about the storage capacity.
    //
    _Space            = FS_GetVolumeFreeSpace(VOLUME_NAME);
    _Space            = SEGGER_MIN(_Space, FILE_SIZE);
    _NumBytesAtOnce   = BLOCK_SIZE;
    _NumBlocksMeasure = NUM_BLOCKS_MEASURE;
    while (1) {
      _NumBytes = _NumBytesAtOnce * _NumBlocksMeasure;
      if (_NumBytes <= _Space) {
        break;
      }
      _NumBytesAtOnce   >>= 1;
      _NumBlocksMeasure >>= 1;
    }
    _NumLoops         = _Space / _NumBytes;
    if (_NumLoops) {
      //
      // Write to a pre-allocated file.
      //
      _TestWriteWithPreAllocation("Write, pre-allocated clusters");
      //
      // Update the directory entry and FAT only when the file is closed to speed up the write process.
      //
      FS_SetFileWriteMode(FS_WRITEMODE_FAST);
      _TestWriteWithDynamicAllocation("Write, clusters dynamically allocated, fast write mode");
      //
      // Update the directory entry only when the file is closed to speed up the write process.
      //
      FS_SetFileWriteMode(FS_WRITEMODE_MEDIUM);
      _TestWriteWithDynamicAllocation("Write, clusters dynamically allocated, medium write mode");
      //
      // Update the directory entry on every write operation.
      //
      FS_SetFileWriteMode(FS_WRITEMODE_SAFE);
      _TestWriteWithDynamicAllocation("Write, clusters dynamically allocated, safe write mode");
      //
      // Measure the read speed.
      //
      _TestRead("Read");
      //
      // Perform cleanup.
      //
      FS_Remove(FILE_NAME);
      //
      // Display the results on console.
      //
      _ShowResults();
    } else {
      FS_X_Log("Not enough free space available on the storage.\n");
    }
  }
  FS_Unmount("");
  FS_X_Log("Finished\n");
  while (1) {
    ;
  }
}

/*************************** End of file ****************************/
