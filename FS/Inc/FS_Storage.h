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
File        : FS_Storage.h
Purpose     : Define global functions and types to be used by an
              application using the storage API.

              This file needs to be included by any module using the
              storage API.
-------------------------- END-OF-HEADER -----------------------------
*/

#ifndef FS_STORAGE_H               // Avoid recursive and multiple inclusion
#define FS_STORAGE_H

/*********************************************************************
*
*             #include Section
*
**********************************************************************
*/

#include "FS_ConfDefaults.h"        /* FS Configuration */
#include "FS_Types.h"
#include "FS_Dev.h"

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
*       Presence status
*
*  Description
*    Indicates if a removable storage device is present or not.
*/
#define FS_MEDIA_NOT_PRESENT        0     // Storage device is not present.
#define FS_MEDIA_IS_PRESENT         1     // Storage device is present.
#define FS_MEDIA_STATE_UNKNOWN      2     // Presence status is unknown.

#define FS_OPERATION_READ           0
#define FS_OPERATION_WRITE          1

/*********************************************************************
*
*       Public types
*
**********************************************************************
*/

/*********************************************************************
*
*       FS_DEV_INFO
*
*  Description
*    Information about the storage device.
*
*  Additional information
*    NumHeads and SectorsPerTrack are relevant only for mechanical
*    drives. The application can access the information about
*    the storage device by calling FS_STORAGE_GetDeviceInfo().
*/
struct FS_DEV_INFO {
  U16 NumHeads;          // Number of read / write heads.
  U16 SectorsPerTrack;   // Number of sectors stored on a track.
  U32 NumSectors;        // Total number of sectors on the storage device.
  U16 BytesPerSector;    // Size of a logical sector in bytes.
};

/*********************************************************************
*
*       FS_STORAGE_COUNTERS
*
*  Description
*    Statistical counters.
*
*  Additional information
*    ReadSectorCnt can be (and typically is) higher than
*    ReadOperationCnt, since one read operation can request multiple
*    sectors (in a burst). The same applies to write operations:
*    WriteSectorCnt can be (and typically is) higher than
*    WriteOperationCnt, since one read operation can request multiple
*    sectors (in a burst).
*
*    The statistical counters can be read via FS_STORAGE_GetCounters().
*    They are set to 0 when the file system is initialized.
*    Additionally, the application can explicitly set them to 0
*    via FS_STORAGE_ResetCounters().
*/
typedef struct {
  U32 ReadOperationCnt;       // Number of "Read sector operation" calls.
  U32 ReadSectorCnt;          // Number of sectors read (before cache).
  U32 ReadSectorCachedCnt;    // Number of sectors read from cache
  U32 WriteOperationCnt;      // Number of "Write sector operation" calls
  U32 WriteSectorCnt;         // Number of sectors written (before cache).
  U32 WriteSectorCntCleaned;  // Number of sectors written by the cache to storage in order to make room for other data.
  U32 ReadSectorCntMan;       // Number of management sectors read (before cache).
  U32 ReadSectorCntDir;       // Number of directory sectors (which store directory entries) read (before cache).
  U32 WriteSectorCntMan;      // Number of management sectors written (before cache).
  U32 WriteSectorCntDir;      // Number of directory sectors (which store directory entries) written (before cache).
} FS_STORAGE_COUNTERS;

typedef void (FS_ONDEVICEACTIVITYHOOK)(FS_DEVICE * pDevice, unsigned Operation, U32 StartSector, U32 NumSectors, int Sectortype);

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
int      FS_STORAGE_Clean          (const char * sVolumeName);
int      FS_STORAGE_CleanOne       (const char * sVolumeName, int * pMoreToClean);
#if FS_SUPPORT_DEINIT
void     FS_STORAGE_DeInit         (void);
#endif
int      FS_STORAGE_FreeSectors    (const char * sVolumeName, U32 FirstSector, U32 NumSectors);
int      FS_STORAGE_GetCleanCnt    (const char * sVolumeName, U32 * pCnt);
void     FS_STORAGE_GetCounters    (FS_STORAGE_COUNTERS * pStat);
int      FS_STORAGE_GetDeviceInfo  (const char * sVolumeName, FS_DEV_INFO * pDeviceInfo);
int      FS_STORAGE_GetSectorUsage (const char * sVolumeName, U32 SectorIndex);
unsigned FS_STORAGE_Init           (void);
int      FS_STORAGE_ReadSector     (const char * sVolumeName,       void * pData, U32 SectorIndex);
int      FS_STORAGE_ReadSectors    (const char * sVolumeName,       void * pData, U32 FirstSector, U32 NumSectors);
int      FS_STORAGE_RefreshSectors (const char * sVolumeName, U32 FirstSector, U32 NumSectors, void * pBuffer, U32 NumBytes);
void     FS_STORAGE_ResetCounters  (void);
void     FS_STORAGE_Sync           (const char * sVolumeName);
int      FS_STORAGE_SyncSectors    (const char * sVolumeName, U32 FirstSector, U32 NumSectors);
void     FS_STORAGE_Unmount        (const char * sVolumeName);
void     FS_STORAGE_UnmountForced  (const char * sVolumeName);
int      FS_STORAGE_WriteSector    (const char * sVolumeName, const void * pData, U32 SectorIndex);
int      FS_STORAGE_WriteSectors   (const char * sVolumeName, const void * pData, U32 FirstSector, U32 NumSectors);
void     FS_SetOnDeviceActivityHook(const char * sVolumeName, FS_ONDEVICEACTIVITYHOOK * pfOnDeviceActivityHook);

/*********************************************************************
*
*       Compatibility defines
*/
#define FS_InitStorage()                                    FS_STORAGE_Init()
#define FS_ReadSector(sVolumeName, pData, SectorIndex)      FS_STORAGE_ReadSector(sVolumeName,  pData, SectorIndex)
#define FS_WriteSector(sVolumeName, pData, SectorIndex)     FS_STORAGE_WriteSector(sVolumeName, pData, SectorIndex)
#define FS_UnmountLL(sVolumeName)                           FS_STORAGE_Unmount(sVolumeName)
#define FS_CleanVolume(sVolumeName)                         FS_STORAGE_Sync(sVolumeName)
#define FS_GetDeviceInfo(sVolumeName, pDevInfo)             FS_STORAGE_GetDeviceInfo(sVolumeName, pDevInfo)

#if defined(__cplusplus)
  }              /* Make sure we have C-declarations in C++ programs */
#endif

#endif // FS_STORAGE_H

/*************************** End of file ****************************/
