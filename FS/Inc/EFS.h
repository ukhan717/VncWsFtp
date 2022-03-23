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
File        : EFS.h
Purpose     : EFS File System Layer header
-------------------------- END-OF-HEADER -----------------------------
*/

#ifndef EFS_H               // Avoid recursive and multiple inclusion
#define EFS_H

#include "FS_Int.h"

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

/*********************************************************************
*
*       EFS_Read
*/
U32  FS_EFS_Read(FS_FILE *pFile, void *pData, U32 NumBytesReq);

/*********************************************************************
*
*       EFS_Write
*/
U32  FS_EFS_Write  (FS_FILE   * pFile, const void *pData, U32 NumBytes);
void FS_EFS_Close  (FS_FILE   * pFile);
void FS_EFS_Clean  (FS_VOLUME * pVolume);

/*********************************************************************
*
*       EFS_Open
*/
int  FS_EFS_Open   (const char * pFileName, FS_FILE * pFile, U8 DoDel, U8 DoOpen, U8 DoCreate);

/*********************************************************************
*
*       EFS_Misc
*/
int  FS_EFS_CheckInfoSector     (FS_VOLUME * pVolume);
int  FS_EFS_CreateJournalFile   (FS_VOLUME * pVolume, U32 NumClusters, U32 * pFirstSector, U32 * pNumSectors);
int  FS_EFS_OpenJournalFile     (FS_VOLUME * pVolume);
U32  FS_EFS_GetIndexOfLastSector(FS_VOLUME * pVolume);
int  FS_EFS_FreeSectors         (FS_VOLUME * pVolume);
int  FS_EFS_GetFreeSpace        (FS_VOLUME  * pVolume, void * pBuffer, int SizeOfBuffer, U32 FirstClusterId, U32 * pNumClustersFree, U32 * pNumClustersChecked);

/*********************************************************************
*
*       EFS_Format
*/
int  FS_EFS_Format        (FS_VOLUME * pVolume, const FS_FORMAT_INFO  * pFormatInfo);
int  FS_EFS_GetDiskInfo   (FS_VOLUME * pVolume, FS_DISK_INFO    * pDiskInfo, int Flags);
int  FS_EFS_GetVolumeLabel(FS_VOLUME * pVolume, char * pVolumeLabel, unsigned VolumeLabelSize);
int  FS_EFS_SetVolumeLabel(FS_VOLUME * pVolume, const char * pVolumeLabel);

/*********************************************************************
*
*       EFS_Move
*/
int  FS_EFS_Move(const char * sOldName, const char * sNewName, FS_VOLUME * pVolume);

/*********************************************************************
*
*       FS_EFS_DirEntry
*/
int  FS_EFS_SetDirEntryInfo(FS_VOLUME * pVolume, const char * sName, const void * p, int Mask);
int  FS_EFS_GetDirEntryInfo(FS_VOLUME * pVolume, const char * sName,       void * p, int Mask);

/*********************************************************************
*
*       EFS_Rename
*/
int   FS_EFS_Rename(const char * sOldName, const char * sNewName, FS_VOLUME * pVolume);

/*********************************************************************
*
*       EFS_Dir
*/
int   FS_EFS_OpenDir  (const char * sDirName, FS__DIR * pDir);
int   FS_EFS_CloseDir (FS__DIR * pDir);
int   FS_EFS_ReadDir  (FS__DIR * pDir, FS_DIRENTRY_INFO * pDirEntryInfo);
int   FS_EFS_RemoveDir(FS_VOLUME * pVolume, const char * sDirName);
int   FS_EFS_CreateDir(FS_VOLUME * pVolume, const char * sDirName);
int   FS_EFS_DeleteDir(FS_VOLUME * pVolume, const char * sDirName, int MaxRecusionLevel);

/*********************************************************************
*
*       EFS_SetEndOfFile
*/
int FS_EFS_SetEndOfFile(FS_FILE * pFile);
int FS_EFS_SetFileSize (FS_FILE * pFile, U32 NumBytes);

/*********************************************************************
*
*       FS_EFS_CheckDisk
*/
int FS_EFS__CheckDisk(FS_VOLUME * pVolume, const FS_DISK_INFO * pDiskInfo, void * pBuffer, U32 BufferSize, int MaxRecursionLevel, FS_CHECKDISK_ON_ERROR_CALLBACK * pfOnError);

#if defined(__cplusplus)
}                /* Make sure we have C-declarations in C++ programs */
#endif

#endif // __EFS_H__

/*************************** End of file ****************************/
