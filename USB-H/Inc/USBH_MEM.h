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
File        : USBH_MEM.h
Purpose     : USB host, memory management
-------------------------- END-OF-HEADER -----------------------------
*/

#ifndef USBH_MEM_H            // Avoid multiple inclusion.
#define USBH_MEM_H

/*********************************************************************
*
*       #include Section
*
**********************************************************************
*/

#include "SEGGER.h"

#if defined(__cplusplus)
  extern "C" {                // Make sure we have C-declarations in C++ programs.
#endif

/*********************************************************************
*
*       #define constants
*
**********************************************************************
*/
#define MIN_BLOCK_SIZE       64uL        // Needs to be a power of 2
#define MAX_BLOCK_SIZE_INDEX 11u         // Allows blocks up to 128 KB (= MIN_BLOCK_SIZE << MAX_BLOCK_SIZE_INDEX)

#ifndef USBH_MEM_DEBUG
  #define USBH_MEM_DEBUG 0
#endif

/*********************************************************************
*
*       Macros
*
**********************************************************************
*/
//lint --emacro((9087,9079),USBH_MALLOC)             D:100[d]
//lint --emacro((9087,9079),USBH_MALLOC_ZEROED)      D:100[d]
//lint --emacro((9087,9079),USBH_TRY_MALLOC)         D:100[d]
//lint --emacro((9087,9079),USBH_TRY_MALLOC_ZEROED)  D:100[d]
//lint --emacro((9087,9079),USBH_MALLOC_XFERMEM)     D:100[d]
//lint --emacro((9087,9079),USBH_TRY_MALLOC_XFERMEM) D:100[d]
#if USBH_MEM_DEBUG
  #define USBH_FREE(pMemBlock)                 USBH_Free(pMemBlock, __func__, __FILE__, __LINE__)
  #define USBH_MALLOC(Size)                    USBH_Malloc(Size, __func__, __FILE__, __LINE__)
  #define USBH_MALLOC_ZEROED(Size)             USBH_MallocZeroed(Size, __func__, __FILE__, __LINE__)
  #define USBH_TRY_MALLOC(Size)                USBH_TryMalloc(Size, __func__, __FILE__, __LINE__)
  #define USBH_TRY_MALLOC_ZEROED(Size)         USBH_TryMallocZeroed(Size, __func__, __FILE__, __LINE__)
  #define USBH_MALLOC_XFERMEM(Size, Align)     USBH_AllocTransferMemory(Size, Align, __func__, __FILE__, __LINE__)
  #define USBH_TRY_MALLOC_XFERMEM(Size, Align) USBH_TryAllocTransferMemory(Size, Align, __func__, __FILE__, __LINE__)
#else
  #define USBH_FREE(pMemBlock)                 USBH_Free(pMemBlock)
  #define USBH_MALLOC(Size)                    USBH_Malloc(Size)
  #define USBH_MALLOC_ZEROED(Size)             USBH_MallocZeroed(Size)
  #define USBH_TRY_MALLOC(Size)                USBH_TryMalloc(Size)
  #define USBH_TRY_MALLOC_ZEROED(Size)         USBH_TryMallocZeroed(Size)
  #define USBH_MALLOC_XFERMEM(Size, Align)     USBH_AllocTransferMemory(Size, Align)
  #define USBH_TRY_MALLOC_XFERMEM(Size, Align) USBH_TryAllocTransferMemory(Size, Align)
#endif

/*********************************************************************
*
*       Types
*
**********************************************************************
*/
typedef struct _USBH_MEM_FREE_BLCK {
  struct _USBH_MEM_FREE_BLCK * pNext;
#if USBH_DEBUG > 0
  U32 Magic;
#endif
} USBH_MEM_FREE_BLCK;

typedef struct {
  U8            * pBaseAddr;
  U8            * pSizeIdxTab;      // Contains size index (0..MAX_BLOCK_SIZE_INDEX) for each allocated block.
#if USBH_REO_FREE_MEM_LIST > 0
  I8              MemReoScheduled;  // if set, reorganization of free memory area will be performed before next alloc()
#endif
#if USBH_DEBUG > 0
  U32             UsedMem;
  U32             MaxUsedMem;
#endif
  USBH_MEM_FREE_BLCK * apFreeList[MAX_BLOCK_SIZE_INDEX + 1u];
} USBH_MEM_POOL;

/*********************************************************************
*
*       API functions / Function prototypes
*
**********************************************************************
*/
void    USBH_MEM_POOL_Create(USBH_MEM_POOL * pPool, void * pMem, U32 NumBytes);
void  * USBH_MEM_POOL_Alloc (USBH_MEM_POOL * pPool, U32 NumBytesUser, U32 Alignment);
void    USBH_MEM_POOL_Free  (USBH_MEM_POOL * pPool, U8 *p);

#if USBH_MEM_DEBUG
void    USBH_Free                           (void * pMemBlock, const char * sFunc, const char * sFile, int Line);
void  * USBH_Malloc                         (U32    Size,      const char * sFunc, const char * sFile, int Line);
void  * USBH_MallocZeroed                   (U32    Size,      const char * sFunc, const char * sFile, int Line);
void  * USBH_TryMalloc                      (U32    Size,      const char * sFunc, const char * sFile, int Line);
void  * USBH_TryMallocZeroed                (U32    Size,      const char * sFunc, const char * sFile, int Line);
void  * USBH_AllocTransferMemory            (U32    NumBytes, unsigned Alignment, const char * sFunc, const char * sFile, int Line);
void  * USBH_TryAllocTransferMemory         (U32    NumBytes, unsigned Alignment, const char * sFunc, const char * sFile, int Line);
#else
void    USBH_Free                           (void * pMemBlock);
void  * USBH_Malloc                         (U32    Size);
void  * USBH_MallocZeroed                   (U32    Size);
void  * USBH_TryMalloc                      (U32    Size);
void  * USBH_TryMallocZeroed                (U32    Size);
void  * USBH_AllocTransferMemory            (U32    NumBytes, unsigned Alignment);
void  * USBH_TryAllocTransferMemory         (U32    NumBytes, unsigned Alignment);
#endif
void    USBH_MEM_ReoFree                    (int    Idx);
void    USBH_MEM_ScheduleReo                (void);

#if defined(__cplusplus)
}                             // Make sure we have C-declarations in C++ programs.
#endif

#endif                        // Avoid multiple inclusion.

/*************************** End of file ****************************/
