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
File        : FS.h
Purpose     : Define global functions and types to be used by an
              application using the file system.

              This file needs to be included by any module using the
              file system.
-------------------------- END-OF-HEADER -----------------------------
*/

#ifndef FS_H               // Avoid recursive and multiple inclusion
#define FS_H

/*********************************************************************
*
*             #include Section
*
**********************************************************************
*/

#include "FS_ConfDefaults.h"        /* FS Configuration */
#include "FS_Types.h"
#include "FS_Storage.h"
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
*       File system version
*/
#define FS_VERSION      40601UL

/*********************************************************************
*
*       Error codes
*
*  Description
*    Values returned by the API functions to indicate the reason of an error.
*
*  Additional information
*    The last error code of an file handle can be checked using FS_FError().
*/
#define FS_ERRCODE_OK                           0         // No error
#define FS_ERRCODE_EOF                          (-1)      // End of file reached
#define FS_ERRCODE_PATH_TOO_LONG                (-2)      // Path to file or directory is too long
#define FS_ERRCODE_INVALID_PARA                 (-3)      // Invalid parameter passed
#define FS_ERRCODE_WRITE_ONLY_FILE              (-5)      // File can only be written
#define FS_ERRCODE_READ_ONLY_FILE               (-6)      // File can not be written
#define FS_ERRCODE_READ_FAILURE                 (-7)      // Error while reading from storage medium
#define FS_ERRCODE_WRITE_FAILURE                (-8)      // Error while writing to storage medium
#define FS_ERRCODE_FILE_IS_OPEN                 (-9)      // Trying to delete a file which is open
#define FS_ERRCODE_PATH_NOT_FOUND               (-10)     // Path to file or directory not found
#define FS_ERRCODE_FILE_DIR_EXISTS              (-11)     // File or directory already exists
#define FS_ERRCODE_NOT_A_FILE                   (-12)     // Trying to open a directory instead of a file
#define FS_ERRCODE_TOO_MANY_FILES_OPEN          (-13)     // Exceeded number of files opened at once (trial version)
#define FS_ERRCODE_INVALID_FILE_HANDLE          (-14)     // The file handle has been invalidated
#define FS_ERRCODE_VOLUME_NOT_FOUND             (-15)     // The volume name specified in a path is does not exist
#define FS_ERRCODE_READ_ONLY_VOLUME             (-16)     // Trying to write to a volume mounted in read-only mode
#define FS_ERRCODE_VOLUME_NOT_MOUNTED           (-17)     // Trying access a volume which is not mounted
#define FS_ERRCODE_NOT_A_DIR                    (-18)     // Trying to open a file instead of a directory
#define FS_ERRCODE_FILE_DIR_NOT_FOUND           (-19)     // File or directory not found
#define FS_ERRCODE_NOT_SUPPORTED                (-20)     // Functionality not supported
#define FS_ERRCODE_CLUSTER_NOT_FREE             (-21)     // Trying to allocate a cluster which is not free
#define FS_ERRCODE_INVALID_CLUSTER_CHAIN        (-22)     // Detected a shorter than expected cluster chain
#define FS_ERRCODE_STORAGE_NOT_PRESENT          (-23)     // Trying to access a removable storage which is not inserted
#define FS_ERRCODE_BUFFER_NOT_AVAILABLE         (-24)     // No more sector buffers available
#define FS_ERRCODE_STORAGE_TOO_SMALL            (-25)     // Not enough sectors on the storage medium
#define FS_ERRCODE_STORAGE_NOT_READY            (-26)     // Storage device can not be accessed
#define FS_ERRCODE_BUFFER_TOO_SMALL             (-27)     // Sector buffer smaller than sector size of storage medium
#define FS_ERRCODE_INVALID_FS_FORMAT            (-28)     // Storage medium is not formatted or the format is not valid
#define FS_ERRCODE_INVALID_FS_TYPE              (-29)     // Type of file system is invalid or not configured
#define FS_ERRCODE_FILENAME_TOO_LONG            (-30)     // The name of the file is too long
#define FS_ERRCODE_VERIFY_FAILURE               (-31)     // Data verification failure
#define FS_ERRCODE_VOLUME_FULL                  (-32)     // No more space on storage medium
#define FS_ERRCODE_DIR_NOT_EMPTY                (-33)     // Trying to delete a directory which is not empty
#define FS_ERRCODE_IOCTL_FAILURE                (-34)     // Error while executing a driver control command
#define FS_ERRCODE_INVALID_MBR                  (-35)     // Invalid information in the Master Boot Record
#define FS_ERRCODE_OUT_OF_MEMORY                (-36)     // Could not allocate memory
#define FS_ERRCODE_UNKNOWN_DEVICE               (-37)     // Storage device is not configured
#define FS_ERRCODE_ASSERT_FAILURE               (-38)     // FS_DEBUG_ASSERT() macro failed
#define FS_ERRCODE_TOO_MANY_TRANSACTIONS_OPEN   (-39)     // Maximum number of opened journal transactions exceeded
#define FS_ERRCODE_NO_OPEN_TRANSACTION          (-40)     // Trying to close a journal transaction which is not opened
#define FS_ERRCODE_INIT_FAILURE                 (-41)     // Error while initializing the storage medium
#define FS_ERRCODE_FILE_TOO_LARGE               (-42)     // Trying to write to a file which is larger than 4 Gbytes
#define FS_ERRCODE_HW_LAYER_NOT_SET             (-43)     // The HW layer is not configured
#define FS_ERRCODE_INVALID_USAGE                (-44)     // Trying to call a function in an invalid state
#define FS_ERRCODE_TOO_MANY_INSTANCES           (-45)     // Trying to create one instance more than maximum configured

/*********************************************************************
*
*       File positioning reference
*
*  Description
*    Reference point when changing the file position.
*/
#define FS_SEEK_SET                     0     // The reference is the beginning of the file.
#define FS_SEEK_CUR                     1     // The reference is the current position of the file pointer.
#define FS_SEEK_END                     2     // The reference is the end-of-file position.

/*********************************************************************
*
*       I/O commands for driver (internal)
*/
#define FS_CMD_REQUIRES_FORMAT          1003
#define FS_CMD_GET_DEVINFO              1004
#define FS_CMD_FORMAT_LOW_LEVEL         1005            // Used internally by FS_FormatLow() to command the driver to perform low-level format
#define FS_CMD_FREE_SECTORS             1006            // Used internally: Allows the FS layer to inform driver about free sectors
#define FS_CMD_SET_DELAY                1007            // Used in the simulation to simulate a slow device with RAM driver
#define FS_CMD_UNMOUNT                  1008            // Used internally by FS_STORAGE_Unmount() to inform the driver. Driver invalidates caches and all other information about medium.
#define FS_CMD_UNMOUNT_FORCED           1009            // Used internally by FS_STORAGE_UnmountForced() to inform the driver about an unforced remove of the device.
                                                        // Driver invalidates caches and all other information about medium.
#define FS_CMD_SYNC                     1010            // Tells the driver to clean caches. Typically, all dirty sectors are written.
#define FS_CMD_UNMOUNT_VOLUME           FS_CMD_UNMOUNT  // Obsolete: FS_CMD_UNMOUNT shall be used instead of FS_CMD_UNMOUNT_VOLUME.
#define FS_CMD_DEINIT                   1011            // Frees the resources allocated by the file system.
#define FS_CMD_CLEAN_ONE                1012            // Tells the driver to perform one clean operation. Usually this operation erases some data block on the storage medium which stores invalid information.
                                                        // Aux                    Not used
                                                        // pBuffer  [*int, OUT]   ==0 A second command will do nothing
                                                        //                        ==1 A second command will perform a clean operation
#define FS_CMD_CLEAN                    1013            // Executes the clean operation until all the invalid data on the storage medium has been erased
#define FS_CMD_GET_SECTOR_USAGE         1014            // Queries the driver about the usage of a logical sector.
                                                        // Aux      [int,  IN]    Index of the sector to be queried
                                                        // pBuffer  [*int, OUT]   ==0 Sector in use
                                                        //                        ==1 Sector not used
                                                        //                        ==2 Unknown
#define FS_CMD_ENABLE_READ_AHEAD        1015            // Requests the driver to start reading sectors in advance
#define FS_CMD_DISABLE_READ_AHEAD       1016            // Requests the driver to stop reading sectors in advance
#define FS_CMD_GET_CLEAN_CNT            1017            // Returns the number of clean operations which should be performed before all the invalid data on the storage medium has been erased
                                                        // pBuffer  [*int, OUT]   Number of clean operations
#define FS_CMD_SET_READ_ERROR_CALLBACK  1018            // Registers a callback function which should be invoked by a driver when a read error occurs.
                                                        // Aux      not used
                                                        // pBuffer  [*FS_ON_READ_ERROR_CALLBACK, IN]  Pointer to callback function
#define FS_CMD_SYNC_SECTORS             1019            // Informs the driver about the sectors which must be synchronized. Typically, the sectors are written to storage if cached.
                                                        // Aux      [int,  IN]    Index of the fist sector to be synchronized
                                                        // pBuffer  [*int, IN]    Number of sectors to synchronize

/*********************************************************************
*
*       CACHE Commands (internal)
*/
#define FS_CMD_CACHE_SET_MODE           6000L
#define FS_CMD_CACHE_CLEAN              6001L   // Write out all dirty sectors
#define FS_CMD_CACHE_SET_QUOTA          6002L
#define FS_CMD_CACHE_FREE_SECTORS       6003L
#define FS_CMD_CACHE_INVALIDATE         6004L   // Invalidate all sectors in cache
#define FS_CMD_CACHE_SET_ASSOC_LEVEL    6005L   // Sets the associativity level for the multi-way cache
#define FS_CMD_CACHE_GET_NUM_SECTORS    6006L   // Returns the number of sectors the cache is able to store
#define FS_CMD_CACHE_GET_TYPE           6007L   // Returns the type of the cache configured

/*********************************************************************
*
*       File attributes
*
*  Description
*    Attributes of files and directories.
*/
#define FS_ATTR_READ_ONLY                             0x01      // The file is read-only. Applications can read the file
                                                                // but cannot write to it.
#define FS_ATTR_HIDDEN                                0x02      // The file or directory is marked as hidden. Most of operating systems
                                                                // do not include these files in an ordinary directory listing.
                                                                // This flag is not evaluated by emFile.
#define FS_ATTR_SYSTEM                                0x04      // The file or directory is part of, or is used exclusively by, the
                                                                // operating system. This flag is not evaluated by emFile.
#define FS_ATTR_ARCHIVE                               0x20      // The file or directory is an archive file or directory.
                                                                // Applications can use this attribute to mark files for backup or removal.
                                                                // This flag is not evaluated by emFile.
#define FS_ATTR_DIRECTORY                             0x10      // The file is actually a directory.

/*********************************************************************
*
*       File time types
*
*  Description
*    Types of timestamps available for a file or directory.
*/
#define FS_FILETIME_CREATE                            0         // Date and time when the file or directory was created.
#define FS_FILETIME_ACCESS                            1         // Date and time of the last read access to file or directory.
#define FS_FILETIME_MODIFY                            2         // Date and time of the last write access to file or directory.

/*********************************************************************
*
*       Volume mounting flags (internal)
*/
#define FS_MOUNT_R                                    (1 << 0)
#define FS_MOUNT_W                                    (1 << 1)

/*********************************************************************
*
*       Volume mounting modes
*
*  Description
*    Modes for mounting a volume.
*/
#define FS_MOUNT_RO                                   FS_MOUNT_R                  // Read-only. Data can only be read from storage device.
#define FS_MOUNT_RW                                   (FS_MOUNT_R | FS_MOUNT_W)   // Read / Write. Data can be read form and written to storage device.

/*********************************************************************
*
*       File system types
*
*  Description
*    Types of file systems supported by emFile.
*/
#define FS_FAT                                        0         // File Allocation Table (FAT).
#define FS_EFS                                        1         // SEGGER's Embedded File System (EFS).

/*********************************************************************
*
*       Format types
*
*  Description
*    Types of format supported by the emFile.
*/
#define FS_TYPE_FAT12                                 0x000C    // FAT with 12-bit allocation table entries.
#define FS_TYPE_FAT16                                 0x0010    // FAT with 16-bit allocation table entries.
#define FS_TYPE_FAT32                                 0x0020    // FAT with 32-bit allocation table entries.
#define FS_TYPE_EFS                                   0x0120    // SEGGER's Embedded File System (EFS).

/*********************************************************************
*
*       Disk checking error codes
*
*  Description
*    Error codes reported by FS_CheckDisk() during operation.
*
*  Additional information
*    The error codes are reported via the ErrCode parameter of
*    the callback function.
*/
#define FS_CHECKDISK_ERRCODE_0FILE                    0x10      // A cluster chain is assigned to a file that do not contain data (file size is 0)
#define FS_CHECKDISK_ERRCODE_SHORTEN_CLUSTER          0x11      // The cluster chain assigned to a file is longer than the size of the file.
#define FS_CHECKDISK_ERRCODE_CROSSLINKED_CLUSTER      0x12      // A cluster is assigned to more than one file or directory.
#define FS_CHECKDISK_ERRCODE_FEW_CLUSTER              0x13      // The size of the file is larger than the cluster chain assigned to file.
#define FS_CHECKDISK_ERRCODE_CLUSTER_UNUSED           0x14      // A cluster is marked as in use, but not assigned to any file or directory.
#define FS_CHECKDISK_ERRCODE_CLUSTER_NOT_EOC          0x15      // A cluster is does not have end-of-chain marker.
#define FS_CHECKDISK_ERRCODE_INVALID_CLUSTER          0x16      // Invalid cluster id.
#define FS_CHECKDISK_ERRCODE_INVALID_DIRECTORY_ENTRY  0x17      // Invalid directory entry.
#define FS_CHECKDISK_ERRCODE_SECTOR_NOT_IN_USE        0x18      // A logical sector that stores data is not marked as in use in the device driver.

/*********************************************************************
*
*       Disk checking return values
*
*  Description
*    Error codes returned by FS_CheckDisk().
*/
#define FS_CHECKDISK_RETVAL_OK                        0         // OK, file system not in corrupted state
#define FS_CHECKDISK_RETVAL_RETRY                     1         // An error has be found and repaired, retry is required.
#define FS_CHECKDISK_RETVAL_ABORT                     2         // User aborted operation via callback or API call
#define FS_CHECKDISK_RETVAL_MAX_RECURSE               3         // Max recursion level reached, operation aborted
#define FS_CHECKDISK_RETVAL_ERROR                     (-1)      // Error while accessing the file system

/*********************************************************************
*
*       Disk checking action codes
*
*  Description
*    Values returned by the FS_CheckDisk() callback function.
*
*  Additional information
*    These values indicate FS_CheckDisk() how to handle a file
*    system error.
*/
#define FS_CHECKDISK_ACTION_DO_NOT_REPAIR             0         // The error does not have to be repaired.
#define FS_CHECKDISK_ACTION_SAVE_CLUSTERS             1         // The data stored in the clusters of a faulty cluster chain has to be saved to a file.
#define FS_CHECKDISK_ACTION_ABORT                     2         // The disk checking operation has to be aborted.
#define FS_CHECKDISK_ACTION_DELETE_CLUSTERS           3         // The data stored in the clusters of a faulty cluster chain has to be deleted.

/*********************************************************************
*
*       Volume information flags
*
*  Description
*    Flags that control the information returned by FS_GetVolumeInfoEx().
*/
#define FS_DISKINFO_FLAG_FREE_SPACE                   (1 << 0)    // Returns the available free space on the storage medium

/*********************************************************************
*
*       Logging flags
*
*  Description
*    Flags that control the logging of different emFile layers.
*/
#define FS_MTYPE_INIT                                 (1uL << 0)  // Initialization log messages.
#define FS_MTYPE_API                                  (1uL << 1)  // Log messages from API functions.
#define FS_MTYPE_FS                                   (1uL << 2)  // Log messages from file system.
#define FS_MTYPE_STORAGE                              (1uL << 3)  // Storage layer log messages.
#define FS_MTYPE_JOURNAL                              (1uL << 4)  // Journal log messages
#define FS_MTYPE_CACHE                                (1uL << 5)  // Cache log messages
#define FS_MTYPE_DRIVER                               (1uL << 6)  // Log messages from device and logical drivers.
#define FS_MTYPE_OS                                   (1uL << 7)  // Log messages from OS integration layer.
#define FS_MTYPE_MEM                                  (1uL << 8)  // Log messages from internal memory allocator.

/*********************************************************************
*
*       Sector usage
*
*  Description
*    Usage status of logical sectors.
*/
#define FS_SECTOR_IN_USE                              0           // Logical sector is used to store data.
#define FS_SECTOR_NOT_USED                            1           // Logical sector is available.
#define FS_SECTOR_USAGE_UNKNOWN                       2           // The usage status is unknown to file system.

/*********************************************************************
*
*       OS locking
*
*  Description
*    Types of locking for multitasking access.
*
*  Additional information
*    These values can be assigned to FS_OS_LOCKING compile-time
*    configuration define.
*/
#define FS_OS_LOCKING_NONE                            0           // No locking against concurrent access.
#define FS_OS_LOCKING_API                             1           // Locking is performed at API function level (coarse locking).
#define FS_OS_LOCKING_DRIVER                          2           // Locking is performed at device driver level (fine locking).

/*********************************************************************
*
*       Public types
*
**********************************************************************
*/

/*********************************************************************
*
*       FS_CHECKDISK_ON_ERROR_CALLBACK
*
*  Function description
*    Type of function called by FS_CheckDisk() to report an error.
*
*  Parameters
*    ErrCode    Code of the reported error.
*
*  Return value
*    Value that indicates FS_CheckDisk() how the error should be
*    handled.
*
*  Additional information
*    ErrCode is one of the \tt{FS_CHECKDISK_ERRCODE_...} defines.
*    The value returned by FS_CHECKDISK_ON_ERROR_CALLBACK()
*    can be one of the \tt{FS_CHECKDISK_ACTION_...} defines.
*
*    In addition to ErrCode FS_CheckDisk() can pass other parameters
*    FS_CHECKDISK_ON_ERROR_CALLBACK() providing information about
*    the reported error. These parameters can be directly passed
*    to a printf()-style function along with the format returned
*    by FS_CheckDisk_ErrCode2Text() to create a text description
*    of the error in human-readable format.
*/
typedef int FS_CHECKDISK_ON_ERROR_CALLBACK(int ErrCode, ...);

/*********************************************************************
*
*       FS_BUSY_LED_CALLBACK
*
*  Function description
*    Type of function called by the file system to report a
*    change in the busy status of a volume.
*
*  Parameters
*    OnOff      Indicates if the volume is busy or not.
*               * 1   The volume is busy.
*               * 0   The volume is ready.
*
*  Additional information
*    A function of this type can be registered with the file system
*    via FS_SetBusyLEDCallback(). FS_BUSY_LED_CALLBACK() is called
*    by the file system each time the volume changes the busy status
*    from busy to ready and reverse. Therefore, an application
*    can use FS_BUSY_LED_CALLBACK()to indicate the busy / ready
*    status of a volume via LED or by other means.
*
*    The file system calls FS_BUSY_LED_CALLBACK() with OnOff set to 1
*    when the volume goes busy. When the volume becomes ready
*    FS_BUSY_LED_CALLBACK() is called again with OnOff set to 0.
*/
typedef void FS_BUSY_LED_CALLBACK(U8 OnOff);

/*********************************************************************
*
*       FS_MEMORY_IS_ACCESSIBLE_CALLBACK
*
*  Function description
*    Type of function called by the file system to check if a memory
*    region can be used in a 0-copy operation.
*
*  Parameters
*    pMem       Points to the first byte in the memory area
*               to be checked.
*    NumBytes   Size of the memory area in bytes.
*
*  Return value
*    !=0    The memory region can be used in a 0-copy operation.
*    ==0    The memory region cannot be used in a 0-copy operation.
*
*  Additional information
*    A function of this type is called by the file system before
*    any read or write operation performed by the application.
*    The function has to check if the memory region defined
*    by pMem and NumBytes can be passed directly to the device
*    driver during a 0-copy operation.
*
*    The callback function is typically required on a system where
*    the device driver uses DMA to transfer data to and from the
*    storage device and where not all the memory regions can be
*    accessed by the DMA. If the memory region cannot be accessed
*    by DMA the callback function has to return 0. The file system
*    copies then the data to an internal buffer that is accessible
*    to DMA and preforms the data transfer. The callback has to
*    return a value different than 0 if the DMA can access the
*    specified memory region. In this case the memory region is
*    passed directly to device driver to perform the data transfer
*    and the internal copy operation of the file system is skipped.
*/
typedef int FS_MEMORY_IS_ACCESSIBLE_CALLBACK(void * pMem, U32 NumBytes);

/*********************************************************************
*
*       FS_PF_FREE
*/
typedef void FS_PF_FREE(void * p);

/*********************************************************************
*
*       FS_PF_ALLOC
*/
typedef void * FS_PF_ALLOC(U32 NumBytes);

/*********************************************************************
*
*       FS_ON_READ_ERROR_CALLBACK
*
*  Function description
*    The function is called when a driver encounters an error while
*    reading sector data. Typically called by the NAND driver to get
*    corrected sector data when a bit error occurs.
*
*  Parameters
*    pDeviceType    Type of storage device which encountered the read error
*    DeviceUnit     Unit number of the storage device where the read error occurred
*    SectorIndex    Index of the sector where the read error occurred
*    pBuffer        [OUT] Corrected sector data
*    NumSectors     Number of sectors on which the read error occurred
*
*  Return value
*    ==0    OK, sector data returned
*    !=0    An error occurred
*/
typedef struct FS_ON_READ_ERROR_CALLBACK {
  int (*pfCallback)(const FS_DEVICE_TYPE * pDeviceType, U32 DeviceUnit, U32 SectorIndex, void * pBuffer, U32 NumSectors);
} FS_ON_READ_ERROR_CALLBACK;

/*********************************************************************
*
*       FS_DIRENT
*/
struct FS_DIRENT {
  char DirName[FS_MAX_PATH];
  U8   Attributes;
  U32  Size;
  U32  TimeStamp;
};

/*********************************************************************
*
*       FS_DIR_POS
*/
typedef struct FS_DIR_POS {
  U32 ClusterId;                  // Id of the current cluster.
  U32 FirstClusterId;             // Id of the first cluster assigned to directory.
  U32 DirEntryIndex;              // Index of the current directory entry (first directory entry has index 0)
  U32 ClusterIndex;               // Index of the current cluster relative to beginning of directory (0-based).
} FS_DIR_POS;

/*********************************************************************
*
*       FS__DIR
*/
typedef struct FS__DIR {
  FS_DIR_POS  DirPos;             // Stores the current position in directory.
  U32         DirEntryIndex;      // Index of the directory entry in the parent directory.
  U32         FirstClusterId;     // Id of the first cluster assigned to directory.
  FS_VOLUME * pVolume;            // Volume on which the directory is stored.
} FS__DIR;

/*********************************************************************
*
*       FS_FIND_DATA
*
*  Description
*    Information about a file or directory.
*
*  Additional information
*    This structure contains also the context for the file listing
*    operation. These members are considered internal and should
*    not be used by the application. FS_FIND_DATA is used as context
*    by the FS_FindFirstFile() and FS_FindNextFile() pair of functions.
*/
typedef struct FS_FIND_DATA {
  U8     Attributes;          // Attributes of the file or directory.
  U32    CreationTime;        // Date and time when the file or directory was created.
  U32    LastAccessTime;      // Date and time when the file or directory was accessed last.
  U32    LastWriteTime;       // Date and time when the file or directory was modified last.
  U32    FileSize;            // Size of the file. Set to 0 for a directory.
  char * sFileName;           // Name of the file or directory as 0-terminated string.
                              // It points to the buffer passed as argument to
                              // FS_FindFirstFile().
  //
  // Private elements. Not to be used by the application.
  //
  int     SizeofFileName;     // Internal. Do not use.
  FS__DIR Dir;                // Internal. Do not use.
} FS_FIND_DATA;

/*********************************************************************
*
*       FS_DISK_INFO
*
*  Description
*    Information about a volume.
*
*  Additional information
*    IsDirty can be used to check if the volume formatted as FAT
*    has been correctly unmounted before a system reset. IsDirty
*    is set to 1 at file system initialization if the file system
*    was not properly unmounted.
*/
typedef struct FS_DISK_INFO {
  U32 NumTotalClusters;     // Total number of clusters on the storage device.
  U32 NumFreeClusters;      // Number of clusters that are not in use.
  U16 SectorsPerCluster;    // Number of sectors in a cluster.
  U16 BytesPerSector;       // Size of the logical sector in bytes.
  U16 NumRootDirEntries;    // Number of directory entries in the root directory.
                            // This member is valid only for volumes formatted
                            // as FS_TYPE_FAT12 or FS_TYPE_FAT16.
  U16 FSType;               // Type of file system. One of FS_TYPE_... defines.
  U8  IsSDFormatted;        // Set to 1 if the volume has been formatted according
                            // to SD specification. This member is valid only for
                            // volumes formatted as FAT.
  U8  IsDirty;              // Set to 1 if the volume was not unmounted correctly
                            // or the file system modified the storage. This member
                            // is valid only for volumes formatted as FAT.
} FS_DISK_INFO;

/*********************************************************************
*
*       FS_FORMAT_INFO
*
*  Description
*    Parameters for the high-level format.
*
*  Additional information
*    This structure is passed as parameter to FS_Format() to specify
*    additional information about how the storage device has to be
*    formatted.
*
*    A cluster is the minimal unit size a file system can handle.
*    Sectors are combined together to form a cluster.
*    SectorsPerCluster has to be a power of 2 value, for example 1,
*    2, 4, 8, 16, 32, 64. Larger values lead to a higher read / write
*    performance when working with large files while low values (1)
*    make more efficient use of disk space.
*    * Allowed values for FAT: 1--128
*    * Allowed values for EFS: 1--32768
*
*    NumRootDirEntries represents the number of directory entries in
*    the root directory should have. This is only a proposed value.
*    The actual value  depends on the FAT type. This value is typically
*    used for FAT12 or FAT16 formatted volume that have a fixed number
*    of entries in the root directory. On FAT32 formatted volume the
*    root directory can grow dynamically. The file system uses a
*    default value of 256 if NumRootDirEntries is set to 0.
*
*    pDevInfo should be typically set to NULL unless some specific
*    information about the storage device has to be passed to
*    format function. The file system requests internally the
*    information from storage device if pDevInfo is set to NULL.
*/
typedef struct FS_FORMAT_INFO {
  U16           SectorsPerCluster;    // Number of sectors in a cluster.
  U16           NumRootDirEntries;    // Number of directory entries in the root directory.
  FS_DEV_INFO * pDevInfo;             // Information about the storage device.
} FS_FORMAT_INFO;

/*********************************************************************
*
*       FS_FILETIME
*
*  Description
*    Time and date representation.
*
*  Additional information
*    FS_FILETIME represents a timestamp using individual members for
*    the month, day, year, weekday, hour, minute, and second values.
*    This can be useful for getting or setting a timestamp of a file
*    or directory. The conversion between timestamp and FS_FILETIME
*    can be done using FS_FileTimeToTimeStamp() and
*    FS_TimeStampToFileTime()
*/
typedef struct FS_FILETIME {
  U16 Year;         // Year (The value hast to be greater than 1980.)
  U16 Month;        // Month (1--12, 1: January, 2: February, etc.)
  U16 Day;          // Day of the month (1--31)
  U16 Hour;         // Hour (0--23)
  U16 Minute;       // Minute (0--59)
  U16 Second;       // Second (0--59)
} FS_FILETIME;

/*********************************************************************
*
*       FS_WRITEMODE
*
*  Description
*    Modes of writing to file.
*/
typedef enum FS_WRITEMODE {
  FS_WRITEMODE_SAFE,        // Allows maximum fail-safe behavior. The allocation table and
                            // the directory entry are updated after each write access to file.
                            // This write mode provides the slowest performance.
  FS_WRITEMODE_MEDIUM,      // Medium fail-safe. The allocation table is updated after each
                            // write access to file. The directory entry is updated only
                            // if file is synchronized that is when FS_Sync(), FS_FClose(),
                            // or FS_SyncFile() is called.
  FS_WRITEMODE_FAST,        // This write mode provided the maximum performance. The directory entry
                            // is updated only if the file is synchronized that is when FS_Sync(),
                            // FS_FClose(), or FS_SyncFile() is called. The allocation table is
                            // modified only if necessary.
  FS_WRITEMODE_UNKNOWN      // End of enumeration. For internal use only.
} FS_WRITEMODE;

/*********************************************************************
*
*       FS_CHS_ADDR
*
*  Description
*    Address of physical block on a mechanical drive.
*/
typedef struct FS_CHS_ADDR {
  U16 Cylinder;           // Cylinder number (0-based)
  U8  Head;               // Read / write head (0-based)
  U8  Sector;             // Index of the sector on a cylinder.
} FS_CHS_ADDR;

/*********************************************************************
*
*       FS_PARTITION_INFO
*
*  Description
*    Information about a MBR partition.
*/
typedef struct FS_PARTITION_INFO {
  U32         NumSectors;       // Total number of sectors in the partition.
  U32         StartSector;      // Index of the first sector in the partition
                                // relative to the beginning of the storage device.
  FS_CHS_ADDR StartAddr;        // Address of the first sector in the partition in CHS format.
  FS_CHS_ADDR EndAddr;          // Address of the last sector in the partition in CHS format.
  U8          Type;             // Type of the partition.
  U8          IsActive;         // Set to 1 if the partition is bootable.
} FS_PARTITION_INFO;

/*********************************************************************
*
*       FS_FILE_INFO
*
*  Description
*    Information about a file or directory.
*
*  Additional information
*    The Attributes member is an or-combination of the following
*    values: FS_ATTR_READ_ONLY, FS_ATTR_HIDDEN, FS_ATTR_SYSTEM,
*    FS_ATTR_ARCHIVE, or FS_ATTR_DIRECTORY.
*
*    For directories the FileSize member is always 0.
*/
typedef struct FS_FILE_INFO {
  U8  Attributes;       // File or directory attributes.
  U32 CreationTime;     // Date and time when the file was created.
  U32 LastAccessTime;   // Date and time when the file was accessed last.
  U32 LastWriteTime;    // Date and time when the file was written to last.
  U32 FileSize;         // Size of the file in bytes.
} FS_FILE_INFO;

#if FS_SUPPORT_DEINIT
  struct FS_ON_EXIT_CB;

  typedef struct FS_ON_EXIT_CB {
    struct FS_ON_EXIT_CB * pNext;
    void (*pfOnExit)(void);
  } FS_ON_EXIT_CB;
#endif

/*********************************************************************
*
*       FS_FREE_SPACE_DATA
*
*  Description
*    Information the number of free space available on a volume.
*
*  Additional information
*    This structure stores the result and context of the operation
*    that calculates the amount of free space available on a volume.
*    The amount of free space is returned as a number of clusters
*    via the NumClustersFree member. The members of the search
*    context are considered internal and should not be used by
*    the application.
*    FS_FREE_SPACE_DATA is used by the FS_GetVolumeFreeSpaceFirst()
*    FS_GetVolumeFreeSpaceNext() pair of API functions.
*/
typedef struct FS_FREE_SPACE_DATA {
  U32         NumClustersFree;      // Number of unallocated clusters found.
  //
  // Context information. Not to be used by the application.
  //
  int         SizeOfBuffer;         // Internal. Do not use. Number of bytes in the work buffer.
  void      * pBuffer;              // Internal. Do not use. Work buffer.
  FS_VOLUME * pVolume;              // Internal. Do not use. Volume information.
  U32         FirstClusterId;       // Internal. Do not use. Id of the first cluster to be checked.
} FS_FREE_SPACE_DATA;

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

/*********************************************************************
*
*       "Standard" file I/O functions
*/
FS_FILE * FS_FOpen  (const char * pFileName, const char * pMode);
int       FS_FOpenEx(const char * pFileName, const char * pMode, FS_FILE ** ppFile);
int       FS_FClose (FS_FILE    * pFile);
U32       FS_FRead  (      void * pData, U32 ItemSize, U32 NumItems, FS_FILE * pFile);
U32       FS_FWrite (const void * pData, U32 ItemSize, U32 NumItems, FS_FILE * pFile);

/*********************************************************************
*
*       Non-standard file I/O functions
*/
U32 FS_Read (FS_FILE * pFile,       void * pData, U32 NumBytes);
U32 FS_Write(FS_FILE * pFile, const void * pData, U32 NumBytes);

/*********************************************************************
*
*       File pointer handling
*/
int FS_FSeek       (FS_FILE * pFile, I32 Offset, int Origin);
int FS_SetEndOfFile(FS_FILE * pFile);
int FS_SetFileSize (FS_FILE * pFile, U32 NumBytes);
I32 FS_FTell       (FS_FILE * pFile);

#define FS_GetFilePos(pFile)                                FS_FTell(pFile)
#define FS_SetFilePos(pFile, DistanceToMove, MoveMethod)    FS_FSeek(pFile, DistanceToMove, MoveMethod)

/*********************************************************************
*
*       I/O error handling
*/
int          FS_FEof                     (FS_FILE * pFile);
I16          FS_FError                   (FS_FILE * pFile);
void         FS_ClearErr                 (FS_FILE * pFile);
const char * FS_ErrorNo2Text             (int       ErrCode);
#if FS_SUPPRESS_EOF_ERROR
  void       FS_ConfigEOFErrorSuppression(int OnOff);
#endif

/*********************************************************************
*
*       File functions
*/
int FS_CopyFile   (const char * sSource,       const char * sDest);
int FS_CopyFileEx (const char * sSource,       const char * sDest, void * pBuffer, U32 NumBytes);
U32 FS_GetFileSize(const FS_FILE    * pFile);
int FS_Move       (const char * sExistingName, const char * sNewName);
int FS_Remove     (const char * pFileName);
int FS_Rename     (const char * sOldName,      const char * sNewName);
int FS_Truncate   (FS_FILE    * pFile,         U32          NewSize);
int FS_Verify     (FS_FILE    * pFile,         const void * pData, U32 NumBytes);
int FS_SyncFile   (FS_FILE    * pFile);
int FS_WipeFile   (const char * pFileName);

/*********************************************************************
*
*       IOCTL
*/
int         FS_IoCtl(const char *pVolumeName, I32 Cmd, I32 Aux, void * pBuffer);

/*********************************************************************
*
*       Volume related functions
*/
int         FS_GetVolumeName          (int VolumeIndex, char * sVolumeName, int VolumeNameSize);
U32         FS_GetVolumeSize          (const char * sVolumeName);
U32         FS_GetVolumeSizeKB        (const char * sVolumeName);
U32         FS_GetVolumeFreeSpace     (const char * sVolumeName);
U32         FS_GetVolumeFreeSpaceKB   (const char * sVolumeName);
int         FS_GetNumVolumes          (void);
FS_VOLUME * FS_AddDevice              (const FS_DEVICE_TYPE * pDevType);
void        FS_Unmount                (const char * sVolumeName);
int         FS_Mount                  (const char * sVolumeName);
int         FS_MountEx                (const char * sVolumeName, U8 MountType);
int         FS_GetVolumeInfo          (const char * sVolumeName, FS_DISK_INFO * pInfo);
int         FS_GetVolumeInfoEx        (const char * sVolumeName, FS_DISK_INFO * pInfo, int Flags);
int         FS_IsVolumeMounted        (const char * sVolumeName);
int         FS_GetVolumeLabel         (const char * sVolumeName, char * sVolumeLabel, unsigned VolumeLabelSize);
int         FS_SetVolumeLabel         (const char * sVolumeName, const char * sVolumeLabel);
void        FS_UnmountForced          (const char * sVolumeName);
void        FS_SetAutoMount           (const char * sVolumeName, U8 MountType);
int         FS_GetVolumeStatus        (const char * sVolumeName);
FS_VOLUME * FS_FindVolume             (const char * sVolumeName);
void        FS_RemoveDevice           (const char * sVolumeName);
int         FS_Sync                   (const char * sVolumeName);
int         FS_FreeSectors            (const char * sVolumeName);
int         FS_GetVolumeFreeSpaceFirst(FS_FREE_SPACE_DATA * pFSD, const char * sVolumeName, void * pBuffer, int SizeOfBuffer);
int         FS_GetVolumeFreeSpaceNext (FS_FREE_SPACE_DATA * pFSD);

/*********************************************************************
*
*       File write mode
*/
void        FS_SetFileWriteMode    (FS_WRITEMODE WriteMode);
void        FS_SetFileWriteModeEx  (FS_WRITEMODE WriteMode, const char * sVolumeName);

/*********************************************************************
*
*       Journaling / transaction safety
*/
typedef struct FS_JOURNAL_STAT_COUNTERS {
  U32 WriteSectorCnt;     // Number of sectors written by the file system to journal
  U32 NumTransactions;    // Number of journal transactions performed
  U32 FreeSectorCnt;      // Number of sectors freed
  U32 OverflowCnt;        // Number of times the journal has been cleaned before the end of a transaction
} FS_JOURNAL_STAT_COUNTERS;

typedef void (FS_JOURNAL_ON_OVERFLOW_CALLBACK)(const char * sVolumeName);

int  FS_JOURNAL_Begin                (const char * sVolumeName);
int  FS_JOURNAL_Create               (const char * sVolumeName, U32 NumBytes);
int  FS_JOURNAL_CreateEx             (const char * sVolumeName, U32 NumBytes, U8 SupportFreeSector);
int  FS_JOURNAL_Disable              (const char * sVolumeName);
int  FS_JOURNAL_Enable               (const char * sVolumeName);
int  FS_JOURNAL_End                  (const char * sVolumeName);
int  FS_JOURNAL_GetOpenCnt           (const char * sVolumeName);
int  FS_JOURNAL_GetStatCounters      (const char * sVolumeName, FS_JOURNAL_STAT_COUNTERS * pStat);
int  FS_JOURNAL_Invalidate           (const char * sVolumeName);
int  FS_JOURNAL_ResetStatCounters    (const char * sVolumeName);
void FS_JOURNAL_SetOnOverflowCallback(FS_JOURNAL_ON_OVERFLOW_CALLBACK * pfOnOverflow);

/*********************************************************************
*
*       File/directory attribute functions
*/
int  FS_SetFileAttributes   (const char * pName, U8 AttrMask);
U8   FS_GetFileAttributes   (const char * pName);
U8   FS_ModifyFileAttributes(const char * pName, U8 SetMask, U8 ClrMask);

/*********************************************************************
*
*       File/directory time stamp functions
*/
void FS_FileTimeToTimeStamp(const FS_FILETIME * pFileTime, U32         * pTimeStamp);
int  FS_GetFileTime        (const char        * pName,     U32         * pTimeStamp);
int  FS_GetFileTimeEx      (const char        * pName,     U32         * pTimeStamp, int TimeType);
int  FS_SetFileTime        (const char        * pName,     U32           TimeStamp);
int  FS_SetFileTimeEx      (const char        * pName,     U32           TimeStamp,  int TimeType);
void FS_TimeStampToFileTime(U32                 TimeStamp, FS_FILETIME * pFileTime);

/*********************************************************************
*
*       File/directory information
*/
int  FS_GetFileInfo(const char * pName, FS_FILE_INFO * pInfo);

/*********************************************************************
*
*       File system misc. functions
*/
int  FS_GetNumFilesOpen(void);
U32  FS_GetMaxSectorSize(void);

/*********************************************************************
*
*       File system directory functions
*/
int  FS_CreateDir      (const char * sDirName);
int  FS_MkDir          (const char * sDirName);
int  FS_RmDir          (const char * sDirName);
int  FS_DeleteDir      (const char * sDirName, int MaxRecusionLevel);
int  FS_FindFirstFile  (FS_FIND_DATA * pfd, const char * sPath, char * sFileName, int SizeOfFileName);
int  FS_FindNextFile   (FS_FIND_DATA * pfd);
void FS_FindClose      (FS_FIND_DATA * pfd);

/*********************************************************************
*
*       Obsolete directory functions
*/
void        FS_DirEnt2Attr   (FS_DIRENT  * pDirEnt, U8   * pAttr);
void        FS_DirEnt2Name   (FS_DIRENT  * pDirEnt, char * pBuffer);
U32         FS_DirEnt2Size   (FS_DIRENT  * pDirEnt);
U32         FS_DirEnt2Time   (FS_DIRENT  * pDirEnt);
U32         FS_GetNumFiles   (FS_DIR     * pDir);

FS_DIR    * FS_OpenDir    (const char * pDirName);
int         FS_CloseDir   (FS_DIR     * pDir);
FS_DIRENT * FS_ReadDir    (FS_DIR     * pDir);
void        FS_RewindDir  (FS_DIR     * pDir);

/*********************************************************************
*
*       File system control functions
*/
void     FS_Init            (void);
#if FS_SUPPORT_DEINIT
  void   FS_DeInit          (void);
  void   FS_AddOnExitHandler(FS_ON_EXIT_CB * pCB, void (*pfOnExit)(void));
#endif

/*********************************************************************
*
*       Formatting
*/
int          FS_FormatLLIfRequired(const char * sVolumeName);
int          FS_FormatLow         (const char * sVolumeName);
int          FS_Format            (const char * sVolumeName, const FS_FORMAT_INFO * pFormatInfo);
int          FS_IsLLFormatted     (const char * sVolumeName);
int          FS_IsHLFormatted     (const char * sVolumeName);

/*********************************************************************
*
*       Partitioning
*/
#define FS_NUM_PARTITIONS  4
int          FS_CreateMBR       (const char * sVolumeName, FS_PARTITION_INFO * pPartInfo, int NumPartitions);
int          FS_GetPartitionInfo(const char * sVolumeName, FS_PARTITION_INFO * pPartInfo, U8 PartIndex);

/*********************************************************************
*
*       CheckDisk functionality
*/
const char * FS_CheckDisk_ErrCode2Text(int ErrCode);
int          FS_CheckDisk             (const char * sVolumeName, void * pBuffer, U32 BufferSize, int MaxRecursionLevel, FS_CHECKDISK_ON_ERROR_CALLBACK * pfOnError);
void         FS_FAT_AbortCheckDisk    (void);

/*********************************************************************
*
*       Configuration functions.
*/

/*********************************************************************
*
*       General runtime configuration functions
*/
int  FS_SetMaxSectorSize         (unsigned MaxSectorSize);
void FS_ConfigOnWriteDirUpdate   (char OnOff);
void FS_AssignMemory             (U32 * pMem, U32 NumBytes);
void FS_SetMemHandler            (FS_PF_ALLOC * pfAlloc, FS_PF_FREE * pfFree);
#if FS_SUPPORT_POSIX
  void FS_ConfigPOSIXSupport     (int OnOff);
#endif
#if FS_VERIFY_WRITE
  void FS_ConfigWriteVerification(int OnOff);
#endif

/*********************************************************************
*
*       FAT specific functions.
*/
int    FS_FAT_FormatSD                  (const char * sVolumeName);
U32    FS_FAT_GrowRootDir               (const char * sVolumeName, U32 NumAddEntries);
void   FS_FAT_SupportLFN                (void);
void   FS_FAT_DisableLFN                (void);
#if FS_FAT_USE_FSINFO_SECTOR
  void FS_FAT_ConfigFSInfoSectorUse     (int OnOff);
#endif
#if FS_MAINTAIN_FAT_COPY
  void FS_FAT_ConfigFATCopyMaintenance  (int OnOff);
#endif
#if FS_FAT_PERMIT_RO_FILE_MOVE
  void FS_FAT_ConfigROFileMovePermission(int OnOff);
#endif
#if FS_FAT_UPDATE_DIRTY_FLAG
  void FS_FAT_ConfigDirtyFlagUpdate     (int OnOff);
#endif

/*********************************************************************
*
*       EFS runtime configuration functions.
*/
#if FS_EFS_SUPPORT_STATUS_SECTOR
  void FS_EFS_ConfigStatusSectorSupport (int OnOff);
#endif

/*********************************************************************
*
*       File buffer related functions.
*/
#define FS_FILE_BUFFER_WRITE              (1 << 0)
#define FS_SIZEOF_FILE_BUFFER_STRUCT      24        // sizeof(FS_FILE_BUFFER)
#define FS_SIZEOF_FILE_BUFFER(NumBytes)   (FS_SIZEOF_FILE_BUFFER_STRUCT + (NumBytes))
#if (FS_SUPPORT_FILE_BUFFER)
  int FS_ConfigFileBufferDefault(int          BufferSize,  int Flags);
  int FS_SetFileBufferFlags     (FS_FILE *    pFile,       int Flags);
  int FS_SetFileBufferFlagsEx   (const char * sVolumeName, int Flags);
  int FS_SetFileBuffer           (FS_FILE * pFile, const void * pData, I32 NumBytes, int Flags);
#endif

/*********************************************************************
*
*       BusyLED support
*/
void FS_SetBusyLEDCallback(const char * sVolumeName, FS_BUSY_LED_CALLBACK * pfBusyLEDCallback);

/*********************************************************************
*
*       Memory accessible support
*/
void FS_SetMemAccessCallback(const char * sVolumeName, FS_MEMORY_IS_ACCESSIBLE_CALLBACK * pfIsAccessibleCallback);

/*********************************************************************
*
*       Misc. functions
*/
int FS_GetFileId (const char * pFileName, U8 * pId);
int FS_GetVersion(void);

/*********************************************************************
*
*       Configuration checking functions
*/
int  FS_CONF_GetMaxPath(void);
int  FS_CONF_IsFATSupported(void);
int  FS_CONF_IsEFSSupported(void);
int  FS_CONF_IsFreeSectorSupported(void);
int  FS_CONF_IsCacheSupported(void);
int  FS_CONF_IsEncryptionSupported(void);
int  FS_CONF_IsJournalSupported(void);
char FS_CONF_GetDirectoryDelimiter(void);
int  FS_CONF_IsDeInitSupported(void);
int  FS_CONF_GetOSLocking(void);
int  FS_CONF_GetNumVolumes(void);
int  FS_CONF_IsTrialVersion(void);
int  FS_CONF_GetDebugLevel(void);

/*********************************************************************
*
*       SPI bus width handling
*/
#define FS_BUSWIDTH_CMD_SHIFT               8
#define FS_BUSWIDTH_ADDR_SHIFT              4
#define FS_BUSWIDTH_DATA_SHIFT              0
#define FS_BUSWIDTH_MASK                    0x0F
#define FS_BUSWIDTH_CMD_1BIT                (1u << FS_BUSWIDTH_CMD_SHIFT)
#define FS_BUSWIDTH_CMD_2BIT                (2u << FS_BUSWIDTH_CMD_SHIFT)
#define FS_BUSWIDTH_CMD_4BIT                (4u << FS_BUSWIDTH_CMD_SHIFT)
#define FS_BUSWIDTH_ADDR_1BIT               (1u << FS_BUSWIDTH_ADDR_SHIFT)
#define FS_BUSWIDTH_ADDR_2BIT               (2u << FS_BUSWIDTH_ADDR_SHIFT)
#define FS_BUSWIDTH_ADDR_4BIT               (4u << FS_BUSWIDTH_ADDR_SHIFT)
#define FS_BUSWIDTH_DATA_1BIT               (1u << FS_BUSWIDTH_DATA_SHIFT)
#define FS_BUSWIDTH_DATA_2BIT               (2u << FS_BUSWIDTH_DATA_SHIFT)
#define FS_BUSWIDTH_DATA_4BIT               (4u << FS_BUSWIDTH_DATA_SHIFT)
#define FS_BUSWIDTH_MAKE(Cmd, Addr, Data)   (((Cmd) << FS_BUSWIDTH_CMD_SHIFT) | ((Addr) << FS_BUSWIDTH_ADDR_SHIFT) | ((Data) << FS_BUSWIDTH_DATA_SHIFT))
#define FS_BUSWIDTH_GET_CMD(BusWidth)       (((BusWidth) >> FS_BUSWIDTH_CMD_SHIFT)  & FS_BUSWIDTH_MASK)
#define FS_BUSWIDTH_GET_ADDR(BusWidth)      (((BusWidth) >> FS_BUSWIDTH_ADDR_SHIFT) & FS_BUSWIDTH_MASK)
#define FS_BUSWIDTH_GET_DATA(BusWidth)      (((BusWidth) >> FS_BUSWIDTH_DATA_SHIFT) & FS_BUSWIDTH_MASK)

/*********************************************************************
*
*       Device and logical drivers
*/

/*********************************************************************
*
*       FS_DEVICE_TYPE
*/
struct FS_DEVICE_TYPE {      // Note: This definition is really intern and should be in "FS_Int.h". In order to avoid problems with old compilers, keep it here.
  const char * (*pfGetName)    (U8 Unit);
  int          (*pfAddDevice)  (void);                                                       // Called from AddDevice. Usually the first call to the driver
  int          (*pfRead)       (U8 Unit, U32 SectorIndex,       void * pBuffer, U32 NumSectors);
  int          (*pfWrite)      (U8 Unit, U32 SectorIndex, const void * pBuffer, U32 NumSectors, U8 RepeatSame);
  int          (*pfIoCtl)      (U8 Unit, I32 Cmd, I32 Aux, void * pBuffer);
  int          (*pfInitMedium) (U8 Unit);                                                       // Called when formatting or mounting a device
  int          (*pfGetStatus)  (U8 Unit);
  int          (*pfGetNumUnits)(void);
};

/*********************************************************************
*
*       Driver types
*/
extern const FS_DEVICE_TYPE FS_RAMDISK_Driver;         // Driver that uses RAM as storage
extern const FS_DEVICE_TYPE FS_WINDRIVE_Driver;        // Driver for Windows drives and file images
extern const FS_DEVICE_TYPE FS_MMC_CardMode_Driver;    // Driver for SD/MMC using card controller
extern const FS_DEVICE_TYPE FS_MMC_SPI_Driver;         // Driver for SD/MMC using SPI
extern const FS_DEVICE_TYPE FS_IDE_Driver;             // Driver for IDE and Compact Flash
extern const FS_DEVICE_TYPE FS_NOR_Driver;             // Driver for NOR flashes (fast write)
extern const FS_DEVICE_TYPE FS_NAND_Driver;            // Driver for SLC NAND flashes
extern const FS_DEVICE_TYPE FS_NOR_BM_Driver;          // Driver for NOR flashes (reduced RAM usage)
extern const FS_DEVICE_TYPE FS_NAND_UNI_Driver;        // Driver for SLC NAND flashes with ECC engine
extern const FS_DEVICE_TYPE FS_DISKPART_Driver;        // Logical driver for disk partitions
extern const FS_DEVICE_TYPE FS_CRYPT_Driver;           // Logical driver for encryption
extern const FS_DEVICE_TYPE FS_READAHEAD_Driver;       // Logical driver that reads sectors in advance
extern const FS_DEVICE_TYPE FS_RAID1_Driver;           // Logical driver that performs disk mirroring
extern const FS_DEVICE_TYPE FS_RAID5_Driver;           // Logical driver that performs disk stripping with parity checking
extern const FS_DEVICE_TYPE FS_SECSIZE_Driver;         // Logical driver that converts between different sector sizes
extern const FS_DEVICE_TYPE FS_WRBUF_Driver;           // Logical driver that buffers sector write operations
extern const FS_DEVICE_TYPE FS_LOGVOL_Driver;          // Logical driver for combining / splitting physical volumes

/*********************************************************************
*
*       NOR driver
*/

/*********************************************************************
*
*       Block types assigned to a physical sector (FS_NOR_BM_Driver)
*/
#define FS_NOR_BLOCK_TYPE_UNKNOWN               0
#define FS_NOR_BLOCK_TYPE_DATA                  1
#define FS_NOR_BLOCK_TYPE_WORK                  2
#define FS_NOR_BLOCK_TYPE_EMPTY_ERASED          3
#define FS_NOR_BLOCK_TYPE_EMPTY_NOT_ERASED      4

/*********************************************************************
*
*       Flags to be used in the Flags member of FS_NOR_SPI_DEVICE_PARA
*/
#define FS_NOR_SPI_DEVICE_FLAG_ERROR_STATUS     (1 << 0)      // The device reports errors via Flag Status Register.
#define FS_NOR_SPI_DEVICE_FLAG_WEL_ADDR_MODE    (1 << 1)      // The write enable latch has to be set before changing the address mode.

/*********************************************************************
*
*       Types assigned to physical sectors (FS_NOR_Driver)
*/
#define FS_NOR_SECTOR_TYPE_UNKNOWN              0
#define FS_NOR_SECTOR_TYPE_DATA                 1
#define FS_NOR_SECTOR_TYPE_WORK                 2
#define FS_NOR_SECTOR_TYPE_INVALID              3

/*********************************************************************
*
*       FS_NOR_SECTOR_INFO
*
*  Information about a physical sector (FS_NOR_Driver)
*/
typedef struct FS_NOR_SECTOR_INFO {
  U32 Off;                    // Position of the physical sector relative to the first byte of NOR flash device
  U32 Size;                   // Size of physical sector in bytes
  U32 EraseCnt;               // Number of times the physical sector has been erased
  U16 NumUsedSectors;         // Number of logical sectors that contain valid data
  U16 NumFreeSectors;         // Number of logical sectors that are empty (i.e. blank)
  U16 NumEraseableSectors;    // Number of logical sectors that contain old (i.e. invalid) data.
  U8  Type;                   // Type of data stored in the physical sector (see FS_NOR_SECTOR_TYPE_...)
} FS_NOR_SECTOR_INFO;

/*********************************************************************
*
*       FS_NOR_DISK_INFO
*
*  Information about the NOR flash organization (FS_NOR_Driver)
*/
typedef struct FS_NOR_DISK_INFO {
  U32 NumPhysSectors;
  U32 NumLogSectors;
  U32 NumUsedSectors;   // Number of used logical sectors
} FS_NOR_DISK_INFO;

/*********************************************************************
*
*       FS_NOR_STAT_COUNTERS
*
*  Statistical counters (FS_NOR_Driver)
*/
typedef struct FS_NOR_STAT_COUNTERS {
  U32 EraseCnt;
  U32 ReadSectorCnt;
  U32 WriteSectorCnt;
  U32 CopySectorCnt;
} FS_NOR_STAT_COUNTERS;

/*********************************************************************
*
*       FS_NOR_BM_SECTOR_INFO
*
*  Information about a physical sector (FS_NOR_BM_Driver)
*/
typedef struct FS_NOR_BM_SECTOR_INFO {
  U32 Off;          // Position of the physical sector relative to the first byte of NOR flash device
  U32 Size;         // Size of physical sector in bytes
  U32 EraseCnt;     // Number of times the physical sector has been erased
  U16 lbi;          // Index of the logical block stored in the physical sector
  U8  Type;         // Type of data stored in the physical sector (see FS_NOR_BLOCK_TYPE_...)
} FS_NOR_BM_SECTOR_INFO;

/*********************************************************************
*
*       FS_NOR_BM_DISK_INFO
*
*  Information about the NOR flash organization (FS_NOR_BM_Driver)
*/
typedef struct FS_NOR_BM_DISK_INFO {
  U16 NumPhySectors;
  U16 NumLogBlocks;
  U16 NumUsedPhySectors;
  U16 LSectorsPerPSector;
  U16 BytesPerSector;
  U32 EraseCntMax;
  U32 EraseCntMin;
  U32 EraseCntAvg;
  U8  HasFatalError;
  U8  ErrorType;
  U32 ErrorPSI;
  U8  IsWriteProtected;
} FS_NOR_BM_DISK_INFO;

/*********************************************************************
*
*       FS_NOR_BM_STAT_COUNTERS
*
*  Statistical counters (FS_NOR_BM_Driver)
*/
typedef struct FS_NOR_BM_STAT_COUNTERS {
  U32 NumFreeBlocks;
  U32 EraseCnt;
  U32 ReadSectorCnt;
  U32 WriteSectorCnt;
  U32 ConvertViaCopyCnt;
  U32 ConvertInPlaceCnt;
  U32 NumValidSectors;
  U32 CopySectorCnt;
  U32 NumReadRetries;
  U32 ReadPSHCnt;
  U32 WritePSHCnt;
  U32 ReadLSHCnt;
  U32 WriteLSHCnt;
  U32 ReadCnt;
  U32 ReadByteCnt;
  U32 WriteCnt;
  U32 WriteByteCnt;
} FS_NOR_BM_STAT_COUNTERS;

/*********************************************************************
*
*       FS_NOR_PHY_TYPE
*
*  Physical layer API
*/
typedef struct FS_NOR_PHY_TYPE {
  int  (*pfWriteOff)     (U8 Unit, U32 Off, const void * pSrc, U32 Len);
  int  (*pfReadOff)      (U8 Unit, void * pDest, U32 Off, U32 Len);
  int  (*pfEraseSector)  (U8 Unit, unsigned int SectorIndex);
  void (*pfGetSectorInfo)(U8 Unit, unsigned int SectorIndex, U32 * pOff, U32 * pLen);
  int  (*pfGetNumSectors)(U8 Unit);
  void (*pfConfigure)    (U8 Unit, U32 BaseAddr, U32 StartAddr, U32 NumBytes);
  void (*pfOnSelectPhy)  (U8 Unit);
  void (*pfDeInit)       (U8 Unit);
  int  (*pfIsSectorBlank)(U8 Unit, unsigned int SectorIndex);
  int  (*pfInit)         (U8 Unit);
} FS_NOR_PHY_TYPE;

typedef void (FS_NOR_READ_CFI_FUNC)(U8 Unit, U32 BaseAddr, U32 Off, U8 * pData, unsigned NumItems);

/*********************************************************************
*
*       FS_NOR_HW_TYPE_SPI
*
*  Hardware layer for NOR flash devices connected via SPI
*/
typedef struct FS_NOR_HW_TYPE_SPI {
  int     (*pfInit)     (U8 Unit);
  void    (*pfEnableCS) (U8 Unit);
  void    (*pfDisableCS)(U8 Unit);
  void    (*pfRead)     (U8 Unit,       U8 * pData, int NumBytes);
  void    (*pfWrite)    (U8 Unit, const U8 * pData, int NumBytes);
  void    (*pfRead_x2)  (U8 Unit,       U8 * pData, int NumBytes);
  void    (*pfWrite_x2) (U8 Unit, const U8 * pData, int NumBytes);
  void    (*pfRead_x4)  (U8 Unit,       U8 * pData, int NumBytes);
  void    (*pfWrite_x4) (U8 Unit, const U8 * pData, int NumBytes);
  int     (*pfDelay)    (U8 Unit, U32 ms);
  void    (*pfLock)     (U8 Unit);
  void    (*pfUnlock)   (U8 Unit);
} FS_NOR_HW_TYPE_SPI;

/*********************************************************************
*
*       FS_NOR_HW_TYPE_SPIFI
*
*  Hardware layer for NOR flash devices connected via memory-mapped SPI
*/
typedef struct FS_NOR_HW_TYPE_SPIFI {
  int     (*pfInit)      (U8 Unit);
  void    (*pfSetCmdMode)(U8 Unit);
  void    (*pfSetMemMode)(U8 Unit, U8 ReadCmd, unsigned NumBytesAddr, unsigned NumBytesDummy, U16 BusWidth);
  void    (*pfExecCmd)   (U8 Unit, U8 Cmd, U8 BusWidth);
  void    (*pfReadData)  (U8 Unit, U8 Cmd, const U8 * pPara, unsigned NumBytesPara, unsigned NumBytesAddr,       U8 * pData, unsigned NumBytesData, U16 BusWidth);
  void    (*pfWriteData) (U8 Unit, U8 Cmd, const U8 * pPara, unsigned NumBytesPara, unsigned NumBytesAddr, const U8 * pData, unsigned NumBytesData, U16 BusWidth);
  int     (*pfPoll)      (U8 Unit, U8 Cmd, U8 BitPos, U8 BitValue, U32 Delay, U32 TimeOut_ms, U16 BusWidth);
  int     (*pfDelay)     (U8 Unit, U32 ms);
  void    (*pfLock)      (U8 Unit);
  void    (*pfUnlock)    (U8 Unit);
} FS_NOR_HW_TYPE_SPIFI;

/*********************************************************************
*
*       FS_NOR_SPI_TYPE
*
*  Description
*    Operations on serial NOR devices
*/
typedef struct FS_NOR_SPI_TYPE FS_NOR_SPI_TYPE;

/*********************************************************************
*
*       FS_NOR_SPI_DEVICE_LIST
*
*  Description
*    Defines a list of supported devices.
*/
typedef struct FS_NOR_SPI_DEVICE_LIST {
  U8                       NumDevices;
  const FS_NOR_SPI_TYPE ** ppDevice;
} FS_NOR_SPI_DEVICE_LIST;

/*********************************************************************
*
*       FS_NOR_SPI_DEVICE_PARA
*
*  Defines the parameters of a serial NOR flash device.
*/
typedef struct FS_NOR_SPI_DEVICE_PARA {
  U8  Id;                     // Value to identify the serial NOR flash device. This is the 3rd byte in the response to READ ID (0x9F) command.
  U8  ldBytesPerSector;       // Number of bytes in a physical sector as power of 2.
  U8  ldBytesPerPage;         // Number of bytes in a page as a power of 2.
  U8  NumBytesAddr;           // Number of address bytes. 4 for NOR flash devices with a capacity larger than 128 Mbit (16 Mbyte).
  U16 NumSectors;             // Total number of physical sectors in the device.
  U8  Flags;                  // Additional functionality supported by device that requires special processing. It is an OR combination of FS_NOR_SPI_DEVICE_FLAG_... defines.
  U8  MfgId;                  // Id of device manufacturer. This is the 1st byte in the response to READ ID (0x9F) command.
  U8  CmdWrite112;            // Code of the command used to write the data to NOR flash via 2 data lines. The command itself and the address are transferred via data 1 line.
  U8  CmdWrite122;            // Code of the command used to write the data to NOR flash via 2 data lines. The command itself is transferred via data 1 line while the address via 2 data lines.
  U8  CmdWrite114;            // Code of the command used to write the data to NOR flash via 4 data lines. The command itself and the address are transferred via data 1 line.
  U8  CmdWrite144;            // Code of the command used to write the data to NOR flash via 4 data lines. The command itself is transferred via data 1 line while the address via 4 data lines.
} FS_NOR_SPI_DEVICE_PARA;

/*********************************************************************
*
*       FS_NOR_SPI_DEVICE_PARA_LIST
*
*  Description
*    Defines a list of device parameters.
*/
typedef struct FS_NOR_SPI_DEVICE_PARA_LIST {
  U8                             NumParas;
  const FS_NOR_SPI_DEVICE_PARA * pPara;
} FS_NOR_SPI_DEVICE_PARA_LIST;

/*********************************************************************
*
*       FS_NOR_FATAL_ERROR_INFO
*
*  Description
*    Information passed to callback function when a fatal error occurs.
*/
typedef struct FS_NOR_FATAL_ERROR_INFO {
  U8  Unit;                   // Index of the driver that triggered the fatal error.
  U8  ErrorType;              // Type of the error that occurred.
  U32 ErrorPSI;               // Index of the physical sector in which the error occurred.
} FS_NOR_FATAL_ERROR_INFO;

/*********************************************************************
*
*       FS_NOR_ON_FATAL_ERROR_CALLBACK
*
*  Function description
*    The type of the callback function invoked by the NOR driver
*    when a fatal error occurs.
*
*  Parameters
*    pFatalErrorInfo    Information about the fatal error.
*
*  Return value
*    ==0  The NOR driver has to mark the NOR flash device as read only.
*    !=0  The NOR flash device has to remain writable.
*
*  Additional information
*    If the callback function returns a 0 the NOR driver marks the NOR flash
*    device as read-only and it remains in this state until the NOR flash device
*    is low-level formated. In this state, all further write operations
*    are rejected with an error by the NOR driver.
*
*    The application is responsible to handle the fatal error by for example
*    checking the consistency of the file system via FS_CheckDisk(). The callback
*    function is not allowed to invoke any other FS API functions therefore the
*    handling of the error has to be done after the FS API function that triggered
*    the error returns.
*/
typedef int FS_NOR_ON_FATAL_ERROR_CALLBACK(FS_NOR_FATAL_ERROR_INFO * pFatalErrorInfo);

/*********************************************************************
*
*       Available physical layers for FS_NOR_Driver and FS_NOR_BM_Driver
*/
extern const FS_NOR_PHY_TYPE FS_NOR_PHY_CFI_1x16;            // 1 x 16-bit CFI compliant NOR flash
extern const FS_NOR_PHY_TYPE FS_NOR_PHY_CFI_2x16;            // 2 x 16-bit CFI compliant NOR flash
extern const FS_NOR_PHY_TYPE FS_NOR_PHY_ST_M25;              // ST M25P compliant Serial NOR flash
extern const FS_NOR_PHY_TYPE FS_NOR_PHY_SFDP;                // Serial NOR flash that supports Serial Flash Discoverable Parameters JDEDEC standard
extern const FS_NOR_PHY_TYPE FS_NOR_PHY_DSPI;                // Serial NOR flash that uses SPI in direct mode to access the NOR flash device (FS_NOR_PHY_ST_M25 and FS_NOR_PHY_SFDP)
extern const FS_NOR_PHY_TYPE FS_NOR_PHY_SPIFI;               // Serial NOR flash connected via a memory-mapped SPI interface

/*********************************************************************
*
*       Default HW layers for the NOR flash drivers.
*/
extern const FS_NOR_HW_TYPE_SPI FS_NOR_HW_ST_M25_Default;
extern const FS_NOR_HW_TYPE_SPI FS_NOR_HW_SFDP_Default;

/*********************************************************************
*
*       Lists of supported serial NOR devices
*/
extern const FS_NOR_SPI_DEVICE_LIST FS_NOR_SPI_DeviceList_All;
extern const FS_NOR_SPI_DEVICE_LIST FS_NOR_SPI_DeviceList_Default;
extern const FS_NOR_SPI_DEVICE_LIST FS_NOR_SPI_DeviceList_Micron;
extern const FS_NOR_SPI_DEVICE_LIST FS_NOR_SPI_DeviceList_Spansion;
extern const FS_NOR_SPI_DEVICE_LIST FS_NOR_SPI_DeviceList_Microchip;
extern const FS_NOR_SPI_DEVICE_LIST FS_NOR_SPI_DeviceList_Winbond;
extern const FS_NOR_SPI_DEVICE_LIST FS_NOR_SPI_DeviceList_ISSI;
extern const FS_NOR_SPI_DEVICE_LIST FS_NOR_SPI_DeviceList_Macronix;
extern const FS_NOR_SPI_DEVICE_LIST FS_NOR_SPI_DeviceList_GigaDevice;

/*********************************************************************
*
*       FS_NOR_Driver API functions
*/
void         FS_NOR_Configure                 (U8 Unit, U32 BaseAddr, U32 StartAddr, U32 NumBytes);
void         FS_NOR_ConfigureReserve          (U8 Unit, U8 Percentage2Reserve);
int          FS_NOR_EraseDevice               (U8 Unit);
int          FS_NOR_FormatLow                 (U8 Unit);
int          FS_NOR_GetDiskInfo               (U8 Unit, FS_NOR_DISK_INFO * pDiskInfo);
void         FS_NOR_GetSectorInfo             (U8 Unit, U32 PhysSectorIndex, FS_NOR_SECTOR_INFO * pSectorInfo);
void         FS_NOR_GetStatCounters           (U8 Unit, FS_NOR_STAT_COUNTERS * pStat);
int          FS_NOR_IsLLFormatted             (U8 Unit);
const void * FS_NOR_LogSector2PhySectorAddr   (U8 Unit, U32 LogSectorNo);
int          FS_NOR_ReadOff                   (U8 Unit, U32 Off, void * pData, U32 NumBytes);
void         FS_NOR_ResetStatCounters         (U8 Unit);
void         FS_NOR_SetBlankSectorSkip        (U8 Unit, U8 OnOff);
void         FS_NOR_SetDeviceLineSize         (U8 Unit, U8 ldBytesPerLine);
void         FS_NOR_SetDeviceRewriteSupport   (U8 Unit, U8 OnOff);
void         FS_NOR_SetDirtyCheckOptimization (U8 Unit, U8 OnOff);
void         FS_NOR_SetEraseVerification      (U8 Unit, U8 OnOff);
void         FS_NOR_SetPhyType                (U8 Unit, const FS_NOR_PHY_TYPE * pPhyType);
void         FS_NOR_SetSectorSize             (U8 Unit, U16 SectorSize);
void         FS_NOR_SetWriteVerification      (U8 Unit, U8 OnOff);

/*********************************************************************
*
*       FS_NOR_BM_Driver API functions
*/
void         FS_NOR_BM_Configure              (U8 Unit, U32 BaseAddr, U32 StartAddr, U32 NumBytes);
int          FS_NOR_BM_SetByteOrderBE         (void);
int          FS_NOR_BM_SetByteOrderLE         (void);
int          FS_NOR_BM_DisableCRC             (void);
int          FS_NOR_BM_EnableCRC              (void);
int          FS_NOR_BM_EraseDevice            (U8 Unit);
int          FS_NOR_BM_ErasePhySector         (U8 Unit, U32 PhySectorIndex);
int          FS_NOR_BM_FormatLow              (U8 Unit);
int          FS_NOR_BM_GetDiskInfo            (U8 Unit, FS_NOR_BM_DISK_INFO * pDiskInfo);
int          FS_NOR_BM_GetSectorInfo          (U8 Unit, U32 PhysSectorIndex, FS_NOR_BM_SECTOR_INFO * pSectorInfo);
void         FS_NOR_BM_GetStatCounters        (U8 Unit, FS_NOR_BM_STAT_COUNTERS * pStat);
int          FS_NOR_BM_IsCRCEnabled           (void);
int          FS_NOR_BM_IsLLFormatted          (U8 Unit);
int          FS_NOR_BM_ReadOff                (U8 Unit, void * pData, U32 Off, U32 NumBytes);
void         FS_NOR_BM_ResetStatCounters      (U8 Unit);
void         FS_NOR_BM_SetBlankSectorSkip     (U8 Unit, U8 OnOff);
void         FS_NOR_BM_SetEraseVerification   (U8 Unit, U8 OnOff);
void         FS_NOR_BM_SetMaxEraseCntDiff     (U8 Unit, U32 EraseCntDiff);
void         FS_NOR_BM_SetNumWorkBlocks       (U8 Unit, unsigned NumWorkBlocks);
void         FS_NOR_BM_SetOnFatalErrorCallback(FS_NOR_ON_FATAL_ERROR_CALLBACK * pfOnFatalError);
void         FS_NOR_BM_SetPhyType             (U8 Unit, const FS_NOR_PHY_TYPE * pPhyType);
void         FS_NOR_BM_SetSectorSize          (U8 Unit, unsigned SectorSize);
void         FS_NOR_BM_SetWriteVerification   (U8 Unit, U8 OnOff);
void         FS_NOR_BM_SetUsedSectorsErasure  (U8 Unit, U8 OnOff);
int          FS_NOR_BM_WriteOff               (U8 Unit, const void * pData, U32 Off, U32 NumBytes);

/*********************************************************************
*
*       CFI physical layer API functions
*/
void         FS_NOR_CFI_SetAddrGap         (U8 Unit, U32 StartAddr, U32 NumBytes);
void         FS_NOR_CFI_SetReadCFIFunc     (U8 Unit, FS_NOR_READ_CFI_FUNC * pReadCFI);

/*********************************************************************
*
*       ST M25 physical layer API functions
*/
void         FS_NOR_SPI_Configure          (U8 Unit, U32 SectorSize, U16 NumSectors);
void         FS_NOR_SPI_SetPageSize        (U8 Unit, U16 BytesPerPage);
void         FS_NOR_SPI_SetHWType          (U8 Unit, const FS_NOR_HW_TYPE_SPI * pHWType);
void         FS_NOR_SPI_ReadDeviceId       (U8 Unit, U8 * pId, unsigned NumBytes);
void         FS_NOR_SPI_AddDevice          (const FS_NOR_SPI_DEVICE_PARA * pDevicePara);

/*********************************************************************
*
*       SFDP physical layer API functions
*/
void         FS_NOR_SFDP_Allow2bitMode     (U8 Unit, U8 OnOff);
void         FS_NOR_SFDP_Allow4bitMode     (U8 Unit, U8 OnOff);
void         FS_NOR_SFDP_SetHWType         (U8 Unit, const FS_NOR_HW_TYPE_SPI     * pHWType);
void         FS_NOR_SFDP_SetDeviceList     (U8 Unit, const FS_NOR_SPI_DEVICE_LIST * pDeviceList);
void         FS_NOR_SFDP_SetSectorSize     (U8 Unit, U32 BytesPerSector);

/*********************************************************************
*
*       SPIFI physical layer API functions
*/
void         FS_NOR_SPIFI_Allow2bitMode    (U8 Unit, U8 OnOff);
void         FS_NOR_SPIFI_Allow4bitMode    (U8 Unit, U8 OnOff);
void         FS_NOR_SPIFI_SetHWType        (U8 Unit, const FS_NOR_HW_TYPE_SPIFI        * pHWType);
void         FS_NOR_SPIFI_SetDeviceList    (U8 Unit, const FS_NOR_SPI_DEVICE_LIST      * pDeviceList);
void         FS_NOR_SPIFI_SetDeviceParaList(U8 Unit, const FS_NOR_SPI_DEVICE_PARA_LIST * pDeviceParaList);

/*********************************************************************
*
*       DSPI physical layer API functions
*/
void         FS_NOR_DSPI_SetHWType         (U8 Unit, const FS_NOR_HW_TYPE_SPI * pHWType);

/*********************************************************************
*
*       RAMDISK driver
*/
void FS_RAMDISK_Configure(U8 Unit, void * pData, U16 BytesPerSector, U32 NumSectors);

/*********************************************************************
*
*       MMC/SD driver
*/

/*********************************************************************
*
*       Type of storage cards
*/
#define FS_MMC_CARD_TYPE_UNKNOWN      0     // The driver was not able to identify the device.
#define FS_MMC_CARD_TYPE_MMC          1     // The storage device conforms to MMC specification.
#define FS_MMC_CARD_TYPE_SD           2     // The storage device conforms to SD specification.

/*********************************************************************
*
*       MMC_CARD_ID
*/
typedef struct MMC_CARD_ID {
  U8 aData[16];
} MMC_CARD_ID;

/*********************************************************************
*
*       FS_MMC_STAT_COUNTERS
*/
typedef struct FS_MMC_STAT_COUNTERS {
  U32 WriteSectorCnt;
  U32 WriteErrorCnt;
  U32 ReadSectorCnt;
  U32 ReadErrorCnt;
  U32 CmdExecCnt;
} FS_MMC_STAT_COUNTERS;

/*********************************************************************
*
*       FS_MMC_CARD_INFO
*/
typedef struct FS_MMC_CARD_INFO {
  U8  CardType;           // Type of the storage card (FS_MMC_CARD_TYPE_...)
  U8  BusWidth;           // Number of data lines used for the data transfer: 1,4 or 8.
  U8  IsWriteProtected;   // Set to 1 if the card is write protected
  U8  IsHighSpeedMode;    // Set to 1 if the card operates in the high-speed mode
  U16 BytesPerSector;     // Number of bytes in a sector
  U32 NumSectors;         // Total number of sectors on the card
} FS_MMC_CARD_INFO;

/*********************************************************************
*
*       FS_MMC_HW_TYPE_SPI
*
*  Hardware layer API for FS_MMC_Driver
*/
typedef struct FS_MMC_HW_TYPE_SPI {
  void  (*pfEnableCS)        (U8 Unit);
  void  (*pfDisableCS)       (U8 Unit);
  int   (*pfIsPresent)       (U8 Unit);
  int   (*pfIsWriteProtected)(U8 Unit);
  U16   (*pfSetMaxSpeed)     (U8 Unit, U16 MaxFreq);
  int   (*pfSetVoltage)      (U8 Unit, U16 Vmin, U16 Vmax);
  void  (*pfRead)            (U8 Unit,       U8 * pData, int NumBytes);
  void  (*pfWrite)           (U8 Unit, const U8 * pData, int NumBytes);
  int   (*pfReadEx)          (U8 Unit,       U8 * pData, int NumBytes);
  int   (*pfWriteEx)         (U8 Unit, const U8 * pData, int NumBytes);
  void  (*pfLock)            (U8 Unit);
  void  (*pfUnlock)          (U8 Unit);
} FS_MMC_HW_TYPE_SPI;

/*********************************************************************
*
*       FS_MMC_HW_TYPE_CM
*
*  Hardware layer API for FS_MMC_CardMode_Driver
*/
typedef struct FS_MMC_HW_TYPE_CM {
  void  (*pfInitHW)                (U8 Unit);
  void  (*pfDelay)                 (int ms);
  int   (*pfIsPresent)             (U8 Unit);
  int   (*pfIsWriteProtected)      (U8 Unit);
  U16   (*pfSetMaxSpeed)           (U8 Unit, U16 Freq);
  void  (*pfSetResponseTimeOut)    (U8 Unit, U32 Value);
  void  (*pfSetReadDataTimeOut)    (U8 Unit, U32 Value);
  void  (*pfSendCmd)               (U8 Unit, unsigned Cmd, unsigned CmdFlags, unsigned ResponseType, U32 Arg);
  int   (*pfGetResponse)           (U8 Unit, void * pBuffer, U32 Size);
  int   (*pfReadData)              (U8 Unit,       void * pBuffer, unsigned NumBytes, unsigned NumBlocks);
  int   (*pfWriteData)             (U8 Unit, const void * pBuffer, unsigned NumBytes, unsigned NumBlocks);
  void  (*pfSetDataPointer)        (U8 Unit, const void * p);
  void  (*pfSetHWBlockLen)         (U8 Unit, U16 BlockSize);
  void  (*pfSetHWNumBlocks)        (U8 Unit, U16 NumBlocks);
  U16   (*pfGetMaxReadBurst)       (U8 Unit);
  U16   (*pfGetMaxWriteBurst)      (U8 Unit);
  U16   (*pfGetMaxWriteBurstRepeat)(U8 Unit);
  U16   (*pfGetMaxWriteBurstFill)  (U8 Unit);
} FS_MMC_HW_TYPE_CM;

/*********************************************************************
*
*       Compatibility HW layers
*/
extern const FS_MMC_HW_TYPE_SPI     FS_MMC_HW_SPI_Default;
extern const FS_MMC_HW_TYPE_CM      FS_MMC_HW_CM_Default;

/*********************************************************************
*
*       FS_MMC_Driver API functions
*/
void FS_MMC_ActivateCRC          (void);
void FS_MMC_DeactivateCRC        (void);
int  FS_MMC_GetCardId            (U8 Unit, MMC_CARD_ID * pCardId);
void FS_MMC_GetStatCounters      (U8 Unit, FS_MMC_STAT_COUNTERS * pStat);
void FS_MMC_ResetStatCounters    (U8 Unit);
void FS_MMC_SetHWType            (U8 Unit, const FS_MMC_HW_TYPE_SPI * pHWType);

/*********************************************************************
*
*       FS_MMC_CardMode_Driver API functions
*/
void FS_MMC_CM_Allow4bitMode     (U8 Unit, U8 OnOff);
void FS_MMC_CM_Allow8bitMode     (U8 Unit, U8 OnOff);
void FS_MMC_CM_AllowBufferedWrite(U8 Unit, U8 OnOff);
void FS_MMC_CM_AllowHighSpeedMode(U8 Unit, U8 OnOff);
void FS_MMC_CM_AllowReliableWrite(U8 Unit, U8 OnOff);
void FS_MMC_CM_AllowPowerSaveMode(U8 Unit, U8 OnOff);
int  FS_MMC_CM_EnterPowerSaveMode(U8 Unit);
int  FS_MMC_CM_Erase             (U8 Unit, U32 StartSector, U32 NumSectors);
int  FS_MMC_CM_GetCardId         (U8 Unit, MMC_CARD_ID * pCardId);
int  FS_MMC_CM_GetCardInfo       (U8 Unit, FS_MMC_CARD_INFO * pCardInfo);
void FS_MMC_CM_GetStatCounters   (U8 Unit, FS_MMC_STAT_COUNTERS * pStat);
int  FS_MMC_CM_ReadExtCSD        (U8 Unit, U32 * pBuffer);
void FS_MMC_CM_ResetStatCounters (U8 Unit);
void FS_MMC_CM_SetSectorRange    (U8 Unit, U32 StartSector, U32 MaxNumSectors);
void FS_MMC_CM_SetHWType         (U8 Unit, const FS_MMC_HW_TYPE_CM * pHWType);
int  FS_MMC_CM_UnlockCardForced  (U8 Unit);
int  FS_MMC_CM_WriteExtCSD       (U8 Unit, unsigned Off, const U8 * pData, unsigned NumBytes);

/*********************************************************************
*
*       IDE/CF driver
*/

/*********************************************************************
*
*       FS_IDE_HW_TYPE
*
*  Hardware layer API
*/
typedef struct FS_IDE_HW_TYPE {
  void (*pfReset)       (U8 Unit);
  int  (*pfIsPresent)   (U8 Unit);
  void (*pfDelay400ns)  (U8 Unit);
  U16  (*pfReadReg)     (U8 Unit, unsigned AddrOff);
  void (*pfWriteReg)    (U8 Unit, unsigned AddrOff, U16 Data);
  void (*pfReadData)    (U8 Unit,       U8 * pData, unsigned NumBytes);
  void (*pfWriteData)   (U8 Unit, const U8 * pData, unsigned NumBytes);
} FS_IDE_HW_TYPE;

/*********************************************************************
*
*       Compatibility HW layer
*/
extern const FS_IDE_HW_TYPE FS_IDE_HW_Default;

/*********************************************************************
*
*       API functions
*/
void FS_IDE_Configure(U8 Unit, U8 IsSlave);
void FS_IDE_SetHWType(U8 Unit, const FS_IDE_HW_TYPE * pHWType);

/*********************************************************************
*
*       NAND driver
*/

/*********************************************************************
*
*       NAND block types
*/
#define FS_NAND_BLOCK_TYPE_UNKNOWN              0
#define FS_NAND_BLOCK_TYPE_BAD                  1
#define FS_NAND_BLOCK_TYPE_EMPTY                2
#define FS_NAND_BLOCK_TYPE_WORK                 3
#define FS_NAND_BLOCK_TYPE_DATA                 4

/*********************************************************************
*
*       Return values for FS_NAND_TestBlock()/FS_NAND_UNI_TestBlock()
*/
#define FS_NAND_TEST_RETVAL_OK                  0
#define FS_NAND_TEST_RETVAL_CORRECTABLE_ERROR   1
#define FS_NAND_TEST_RETVAL_FATAL_ERROR         2
#define FS_NAND_TEST_RETVAL_BAD_BLOCK           3
#define FS_NAND_TEST_RETVAL_ERASE_FAILURE       4
#define FS_NAND_TEST_RETVAL_WRITE_FAILURE       5
#define FS_NAND_TEST_RETVAL_READ_FAILURE        6
#define FS_NAND_TEST_RETVAL_INTERNAL_ERROR      (-1)

/*********************************************************************
*
*       ECC correction status
*/
#define FS_NAND_CORR_NOT_APPLIED                0     // No bit errors detected
#define FS_NAND_CORR_APPLIED                    1     // Bit errors were detected and corrected
#define FS_NAND_CORR_FAILURE                    2     // Bit errors were detected but not corrected

/*********************************************************************
*
*       Bad block types
*
*  Description
*    Defines how blocks are marked as defective.
*/
#define FS_NAND_BAD_BLOCK_MARKING_TYPE_UNKNOWN  0     // Not known.
#define FS_NAND_BAD_BLOCK_MARKING_TYPE_ONFI     1     // The block is marked as bad in the first page of the block.
#define FS_NAND_BAD_BLOCK_MARKING_TYPE_LEGACY   2     // The block is marked as bad in the first and second page of the block.

/*********************************************************************
*
*       FS_NAND_SECTOR_INFO
*
*   Information about a logical sector stored on the NAND flash.
*/
typedef struct FS_NAND_SECTOR_INFO {
  U16 brsi;              // Block relative sector index.
  U32 ECC;               // ECC stored for this sector.
  U8  IsValid;           // Sector contains valid data.
} FS_NAND_SECTOR_INFO;

/*********************************************************************
*
*       FS_NAND_BLOCK_INFO
*
*   Information about a NAND flash block.
*/
typedef struct FS_NAND_BLOCK_INFO {
  U32          EraseCnt;                  // Number of times the block has been erased
  U32          lbi;                       // Logical block index assigned to queried physical block.
  U16          NumSectorsBlank;           // Sectors are not used yet.
  U16          NumSectorsValid;           // Sectors contain correct data.
  U16          NumSectorsInvalid;         // Sectors have been invalidated.
  U16          NumSectorsECCError;        // Sectors have incorrect ECC.
  U16          NumSectorsECCCorrectable;  // Sectors have correctable ECC error.
  U16          NumSectorsErrorInECC;      // Sectors have error in ECC.
  U8           IsDriverBadBlock;          // Set to 1 if the block has been marked as bad by the driver. This value is valid only when Type is set to FS_NAND_BLOCK_TYPE_BAD.
  U8           BadBlockErrorType;         // Type of the error that caused the block to be marked as defective.
  U16          BadBlockErrorBRSI;         // Block-relative sector index on which the fatal error occurred that caused the block to be marked as defective.
  U8           Type;                      // Type of data stored in the block (see the FS_NAND_BLOCK_TYPE_... defines).
  const char * sType;                     // Zero-terminated string holding the block type.
} FS_NAND_BLOCK_INFO;

/*********************************************************************
*
*       FS_NAND_DISK_INFO
*
*   Information about the NAND flash.
*/
typedef struct FS_NAND_DISK_INFO {
  U32 NumPhyBlocks;
  U32 NumLogBlocks;
  U32 NumUsedPhyBlocks;
  U32 NumBadPhyBlocks;
  U32 NumPagesPerBlock;
  U32 NumSectorsPerBlock;
  U32 BytesPerPage;
  U32 BytesPerSpareArea;
  U32 BytesPerSector;
  U32 EraseCntMin;
  U32 EraseCntMax;
  U32 EraseCntAvg;
  U8  BadBlockMarkingType;
  U8  IsWriteProtected;
  U8  HasFatalError;
  U8  ErrorType;
  U32 ErrorSectorIndex;
  U16 BlocksPerGroup;
  U32 NumWorkBlocks;
} FS_NAND_DISK_INFO;

/*********************************************************************
*
*       FS_NAND_ECC_INFO
*
*   Information about the ECC used to protect data stored on the NAND flash.
*/
typedef struct FS_NAND_ECC_INFO {
  U8 NumBitsCorrectable;                // Number of bits the ECC should be able to correct.
  U8 ldBytesPerBlock;                   // Number of bytes the ECC should protect.
  U8 HasHW_ECC;                         // Set to 1 if the NAND flash has HW internal ECC.
} FS_NAND_ECC_INFO;

/*********************************************************************
*
*       FS_NAND_ECC_RESULT
*
*   Information about the ECC number of bits corrected in an ECC block.
*/
typedef struct FS_NAND_ECC_RESULT {
  U8 CorrectionStatus;                  // Indicates if the correction succeeded or failed (FS_NAND_CORR_STATUS_...).
  U8 MaxNumBitsCorrected;               // Maximum number of bit errors detected and corrected in any ECC block of a page.
} FS_NAND_ECC_RESULT;

/*********************************************************************
*
*       FS_NAND_DEVICE_INFO
*
*   Information about the NAND flash device.
*/
typedef struct FS_NAND_DEVICE_INFO {
  U8               BPP_Shift;           // Bytes per page shift: 9 for 512 bytes/page or 11 for 2048 bytes/page
  U8               PPB_Shift;           // Pages per block shift: 32 -> 5, 64 -> 6
  U16              NumBlocks;           // Number of blocks in device
  U16              BytesPerSpareArea;   // Number of bytes in the spare area. Usually this is (1 << BPP_Shift)/32. Micron MT29F8G08ABABA has a spare area of 224 bytes for 4096 bytes/page.
  FS_NAND_ECC_INFO ECC_Info;            // Required ECC capability
  U8               DataBusWidth;        // Number of lines used for exchanging the data with the NAND flash (0 - unknown, 1 - SPI, 8 - parallel 8-bit or 16 - parallel 16-bit)
  U8               BadBlockMarkingType; // Specifies how the blocks are marked as defective. Can be one of the FS_NAND_BAD_BLOCK_MARKING_TYPE_... defines.
} FS_NAND_DEVICE_INFO;

/*********************************************************************
*
*       FS_NAND_STAT_COUNTERS
*
*   Statistic counters of NAND flash driver.
*/
typedef struct FS_NAND_STAT_COUNTERS {
  U32 NumFreeBlocks;
  U32 NumBadBlocks;
  U32 EraseCnt;
  U32 ReadDataCnt;
  U32 ReadSpareCnt;
  U32 ReadSectorCnt;
  U32 NumReadRetries;
  U32 WriteDataCnt;
  U32 WriteSpareCnt;
  U32 WriteSectorCnt;
  U32 ConvertViaCopyCnt;
  U32 ConvertInPlaceCnt;
  U32 NumValidSectors;
  U32 CopySectorCnt;
  U32 BlockRelocationCnt;
  U32 ReadByteCnt;
  U32 WriteByteCnt;
  U32 BitErrorCnt;
  U32 aBitErrorCnt[FS_NAND_STAT_MAX_BIT_ERRORS];
} FS_NAND_STAT_COUNTERS;

/*********************************************************************
*
*       FS_NAND_FATAL_ERROR_INFO
*
*  Description
*    Information passed to callback function when a fatal error occurs.
*/
typedef struct FS_NAND_FATAL_ERROR_INFO {
  U8  Unit;                     // Instance of the NAND driver that generated the fatal error.
  U8  ErrorType;                // Type of fatal error.
  U32 ErrorSectorIndex;         // Index of the physical sector where the error occurred. Not applicable for all type of errors.
} FS_NAND_FATAL_ERROR_INFO;

/*********************************************************************
*
*       FS_NAND_ON_FATAL_ERROR_CALLBACK
*
*  Function description
*    The type of the callback function invoked by the NAND driver
*    when a fatal error occurs.
*
*  Parameters
*    pFatalErrorInfo    Information about the fatal error.
*
*  Return value
*    ==0  The NAND driver has to mark the NAND flash device as read only.
*    !=0  The NAND flash device has to remain writable.
*
*  Additional information
*    If the callback function returns a 0 the NAND driver marks the NAND flash
*    device as read-only and it remains in this state until the NAND flash device
*    is low-level formated. In this state all further write operations are rejected
*    with an error by the NAND driver.
*
*    The application is responsible to handle the fatal error by for example
*    checking the consistency of the file system via FS_CheckDisk(). The callback
*    function is not allowed to invoke any other FS API functions therefore the
*    handling of the error has to be done after the FS API function that triggered
*    the error returns.
*/
typedef int FS_NAND_ON_FATAL_ERROR_CALLBACK(FS_NAND_FATAL_ERROR_INFO * pFatalErrorInfo);

/*********************************************************************
*
*       FS_NAND_TEST_INFO
*
*  Description
*    Additional information passed to test routine.
*    The test routine returns information about what went wrong during the test.
*/
typedef struct FS_NAND_TEST_INFO {
  //
  // Input (required only for FS_NAND_UNI_TestBlock)
  //
  U8  NumBitsCorrectable;     // Number of bits the ECC can correct in the data and spare area (typ. 4)
  U8  OffSpareECCProt;        // Offset in the spare area of the first byte protected by ECC (typ. 4).
  U8  NumBytesSpareECCProt;   // Number of bytes in the spare area protected by ECC (typ. 4 bytes)
  U16 BytesPerSpare;          // Total number of bytes in the spare area. When set to 0 the default value of 1/32 of page size is used.
  //
  // Output
  //
  U32 BitErrorCnt;            // Number of bit errors detected and corrected.
  U32 PageIndex;              // Index of the physical page where the error happened.
} FS_NAND_TEST_INFO;

/*********************************************************************
*
*       FS_NAND_ECC_HOOK
*
*   External ECC computation module.
*/
typedef struct FS_NAND_ECC_HOOK {
  void (*pfCalc) (const U32 * pData, U8 * pSpare);
  int  (*pfApply)(      U32 * pData, U8 * pSpare);
  U8   NumBitsCorrectable;    // Number of bits the ECC algorithm is able to correct in the data block and 4 bytes of spare area.
  U8   ldBytesPerBlock;
} FS_NAND_ECC_HOOK;

/*********************************************************************
*
*       FS_NAND_PHY_TYPE
*
*   API structure of a NAND physical layer.
*/
typedef struct FS_NAND_PHY_TYPE {
  int  (*pfEraseBlock)       (U8 Unit, U32 Block);
  int  (*pfInitGetDeviceInfo)(U8 Unit, FS_NAND_DEVICE_INFO * pDevInfo);
  int  (*pfIsWP)             (U8 Unit);
  int  (*pfRead)             (U8 Unit, U32 PageIndex,       void * pData, unsigned Off, unsigned NumBytes);
  int  (*pfReadEx)           (U8 Unit, U32 PageIndex,       void * pData, unsigned Off, unsigned NumBytes,       void * pSpare, unsigned OffSpare, unsigned NumBytesSpare);
  int  (*pfWrite)            (U8 Unit, U32 PageIndex, const void * pData, unsigned Off, unsigned NumBytes);
  int  (*pfWriteEx)          (U8 Unit, U32 PageIndex, const void * pData, unsigned Off, unsigned NumBytes, const void * pSpare, unsigned OffSpare, unsigned NumBytesSpare);
  int  (*pfEnableECC)        (U8 Unit);
  int  (*pfDisableECC)       (U8 Unit);
  int  (*pfConfigureECC)     (U8 Unit, U8 NumBitsCorrectable, U16 BytesPerECCBlock);
  int  (*pfCopyPage)         (U8 Unit, U32 PageIndexSrc, U32 PageIndexDest);
  int  (*pfGetECCResult)     (U8 Unit, FS_NAND_ECC_RESULT * pResult);
  void (*pfDeInit)           (U8 Unit);
} FS_NAND_PHY_TYPE;

/*********************************************************************
*
*       FS_NAND_HW_TYPE
*
*   API structure of the HW layer for NAND flash devices connected via parallel I/O.
*/
typedef struct FS_NAND_HW_TYPE {
  void    (*pfInit_x8)        (U8 Unit);
  void    (*pfInit_x16)       (U8 Unit);
  void    (*pfDisableCE)      (U8 Unit);
  void    (*pfEnableCE)       (U8 Unit);
  void    (*pfSetAddrMode)    (U8 Unit);
  void    (*pfSetCmdMode)     (U8 Unit);
  void    (*pfSetDataMode)    (U8 Unit);
  int     (*pfWaitWhileBusy)  (U8 Unit, unsigned us);
  void    (*pfRead_x8)        (U8 Unit,       void * pBuffer, unsigned NumBytes);
  void    (*pfWrite_x8)       (U8 Unit, const void * pBuffer, unsigned NumBytes);
  void    (*pfRead_x16)       (U8 Unit,       void * pBuffer, unsigned NumBytes);
  void    (*pfWrite_x16)      (U8 Unit, const void * pBuffer, unsigned NumBytes);
} FS_NAND_HW_TYPE;

/*********************************************************************
*
*       FS_NAND_HW_TYPE_SPI
*
*   API structure of the HW layer for NAND flash devices connected via SPI.
*/
typedef struct FS_NAND_HW_TYPE_SPI {
  int     (*pfInit)           (U8 Unit);
  void    (*pfDisableCS)      (U8 Unit);
  void    (*pfEnableCS)       (U8 Unit);
  void    (*pfDelay)          (U8 Unit, int ms);
  int     (*pfRead)           (U8 Unit,       void * pData, unsigned NumBytes);
  int     (*pfWrite)          (U8 Unit, const void * pData, unsigned NumBytes);
  void    (*pfLock)           (U8 Unit);
  void    (*pfUnlock)         (U8 Unit);
} FS_NAND_HW_TYPE_SPI;

/*********************************************************************
*
*       FS_NAND_HW_TYPE_DF
*
*   API structure of the HW layer for DataFlash devices.
*/
typedef struct FS_NAND_HW_TYPE_DF {
  int     (*pfInit)           (U8 Unit);
  void    (*pfEnableCS)       (U8 Unit);
  void    (*pfDisableCS)      (U8 Unit);
  void    (*pfRead)           (U8 Unit,       U8 * pData, int NumBytes);
  void    (*pfWrite)          (U8 Unit, const U8 * pData, int NumBytes);
} FS_NAND_HW_TYPE_DF;

/*********************************************************************
*
*       FS_NAND_HW_TYPE_QSPI
*
*   API structure of the HW layer for NAND flash devices connected via quad and dual SPI.
*/
typedef struct FS_NAND_HW_TYPE_QSPI {
  int     (*pfInit)           (U8 Unit);
  int     (*pfExecCmd)        (U8 Unit, U8 Cmd, U8 BusWidth);
  int     (*pfReadData)       (U8 Unit, U8 Cmd, const U8 * pPara, unsigned NumBytesPara, unsigned NumBytesAddr,       U8 * pData, unsigned NumBytesData, U16 BusWidth);
  int     (*pfWriteData)      (U8 Unit, U8 Cmd, const U8 * pPara, unsigned NumBytesPara, unsigned NumBytesAddr, const U8 * pData, unsigned NumBytesData, U16 BusWidth);
  int     (*pfPoll)           (U8 Unit, U8 Cmd, const U8 * pPara, unsigned NumBytesPara, U8 BitPos, U8 BitValue, U32 Delay, U32 TimeOut_ms, U16 BusWidth);
  void    (*pfDelay)          (U8 Unit, int ms);
  void    (*pfLock)           (U8 Unit);
  void    (*pfUnlock)         (U8 Unit);
} FS_NAND_HW_TYPE_QSPI;

/*********************************************************************
*
*       Available physical layers
*/
extern const FS_NAND_PHY_TYPE FS_NAND_PHY_x;          // 512 or 2048 byte pages,  8-bit or 16-bit interface
extern const FS_NAND_PHY_TYPE FS_NAND_PHY_x8;         // 512 or 2048 byte pages,  8-bit interface
extern const FS_NAND_PHY_TYPE FS_NAND_PHY_512x8;      // 512 byte pages, 8-bit interface
extern const FS_NAND_PHY_TYPE FS_NAND_PHY_2048x8;     // 2048 byte pages, 8-bit interface
extern const FS_NAND_PHY_TYPE FS_NAND_PHY_2048x16;    // 2048 byte pages, 16-bit interface
extern const FS_NAND_PHY_TYPE FS_NAND_PHY_4096x8;     // 4096 byte pages, 8-bit interface
extern const FS_NAND_PHY_TYPE FS_NAND_PHY_DataFlash;  // Physical layer for ATMEL serial DataFlash
extern const FS_NAND_PHY_TYPE FS_NAND_PHY_ONFI;       // Physical layer for NAND flashes which support ONFI
extern const FS_NAND_PHY_TYPE FS_NAND_PHY_SPI;        // Physical layer for NAND flashes with SPI interface
extern const FS_NAND_PHY_TYPE FS_NAND_PHY_QSPI;       // Physical layer for NAND flashes with quad and dual SPI interface

/*********************************************************************
*
*       Default HW layers for the NAND flash drivers.
*/
extern const FS_NAND_HW_TYPE      FS_NAND_HW_Default;
extern const FS_NAND_HW_TYPE_SPI  FS_NAND_HW_SPI_Default;
extern const FS_NAND_HW_TYPE_DF   FS_NAND_HW_DF_Default;

/*********************************************************************
*
*       ECC computation modules
*/
extern const FS_NAND_ECC_HOOK FS_NAND_ECC_HW_NULL;    // Uses the HW ECC of the NAND flash device
extern const FS_NAND_ECC_HOOK FS_NAND_ECC_HW_4BIT;    // Uses the HW ECC of the NAND flash controller
extern const FS_NAND_ECC_HOOK FS_NAND_ECC_HW_8BIT;    // Uses the HW ECC of the NAND flash controller
extern const FS_NAND_ECC_HOOK FS_NAND_ECC_SW_1BIT;    // 1-bit SW ECC over 512 bytes of page and 4 bytes of spare area

/*********************************************************************
*
*       SLC1 NAND driver functions
*/
int  FS_NAND_EraseBlock             (U8 Unit, unsigned BlockIndex);
int  FS_NAND_Clean                  (U8 Unit, unsigned NumBlocksFree, unsigned NumSectorsFree);
void FS_NAND_EraseFlash             (U8 Unit);
int  FS_NAND_FormatLow              (U8 Unit);
void FS_NAND_GetBlockInfo           (U8 Unit, U32 PhyBlockIndex, FS_NAND_BLOCK_INFO * pBlockInfo);
void FS_NAND_GetDiskInfo            (U8 Unit, FS_NAND_DISK_INFO * pDiskInfo);
void FS_NAND_GetSectorInfo          (U8 Unit, U32 PhySectorIndex, FS_NAND_SECTOR_INFO * pBlockInfo);
void FS_NAND_GetStatCounters        (U8 Unit, FS_NAND_STAT_COUNTERS * pStat);
int  FS_NAND_IsBlockBad             (U8 Unit, unsigned BlockIndex);
int  FS_NAND_IsLLFormatted          (U8 Unit);
int  FS_NAND_ReadPageRaw            (U8 Unit, U32 PageIndex, void * pData, unsigned NumBytes);
int  FS_NAND_ReadPhySector          (U8 Unit, U32 PhySectorIndex, void * pData, unsigned * pNumBytesData, void * pSpare, unsigned * pNumBytesSpare);
void FS_NAND_ResetStatCounters      (U8 Unit);
void FS_NAND_SetBlockRange          (U8 Unit, U16 FirstBlock, U16 MaxNumBlocks);
int  FS_NAND_SetCleanThreshold      (U8 Unit, unsigned NumBlocksFree, unsigned NumSectorsFree);
void FS_NAND_SetEraseVerification   (U8 Unit, U8 OnOff);
void FS_NAND_SetPhyType             (U8 Unit, const FS_NAND_PHY_TYPE * pPhyType);
void FS_NAND_SetMaxEraseCntDiff     (U8 Unit, U32 EraseCntDiff);
void FS_NAND_SetNumWorkBlocks       (U8 Unit, U32 NumWorkBlocks);
void FS_NAND_SetOnFatalErrorCallback(FS_NAND_ON_FATAL_ERROR_CALLBACK * pfOnFatalError);
void FS_NAND_SetWriteVerification   (U8 Unit, U8 OnOff);
int  FS_NAND_TestBlock              (U8 Unit, unsigned BlockIndex, U32 Pattern, FS_NAND_TEST_INFO * pInfo);
int  FS_NAND_WritePage              (U8 Unit, U32 PageIndex, const void * pData, unsigned NumBytes);
int  FS_NAND_WritePageRaw           (U8 Unit, U32 PageIndex, const void * pData, unsigned NumBytes);

/*********************************************************************
*
*       Universal NAND driver functions
*/
void FS_NAND_UNI_AllowBlankUnusedSectors     (U8 Unit, U8 OnOff);
void FS_NAND_UNI_AllowReadErrorBadBlocks     (U8 Unit, U8 OnOff);
int  FS_NAND_UNI_Clean                       (U8 Unit, unsigned NumBlocksFree, unsigned NumSectorsFree);
int  FS_NAND_UNI_EraseBlock                  (U8 Unit, unsigned BlockIndex);
int  FS_NAND_UNI_EraseFlash                  (U8 Unit);
int  FS_NAND_UNI_GetBlockInfo                (U8 Unit, U32 PhysBlockIndex, FS_NAND_BLOCK_INFO * pBlockInfo);
int  FS_NAND_UNI_GetDiskInfo                 (U8 Unit, FS_NAND_DISK_INFO * pDiskInfo);
void FS_NAND_UNI_GetStatCounters             (U8 Unit, FS_NAND_STAT_COUNTERS * pStat);
int  FS_NAND_UNI_IsBlockBad                  (U8 Unit, unsigned BlockIndex);
int  FS_NAND_UNI_ReadLogSectorPartial        (U8 Unit, U32 LogSectorIndex, void * pData, unsigned Off, unsigned NumBytes);
int  FS_NAND_UNI_ReadPageRaw                 (U8 Unit, U32 PageIndex, void * pData, unsigned NumBytes);
int  FS_NAND_UNI_ReadPhySector               (U8 Unit, U32 PhySectorIndex, void * pData, unsigned * pNumBytesData, void * pSpare, unsigned * pNumBytesSpare);
void FS_NAND_UNI_ResetStatCounters           (U8 Unit);
void FS_NAND_UNI_SetBlockRange               (U8 Unit, U16 FirstBlock, U16 MaxNumBlocks);
void FS_NAND_UNI_SetBlockReserve             (U8 Unit, unsigned Percentage);
int  FS_NAND_UNI_SetCleanThreshold           (U8 Unit, unsigned NumBlocksFree, unsigned NumSectorsFree);
void FS_NAND_UNI_SetECCHook                  (U8 Unit, const FS_NAND_ECC_HOOK * pECCHook);
void FS_NAND_UNI_SetEraseVerification        (U8 Unit, U8 OnOff);
void FS_NAND_UNI_SetMaxBitErrorCnt           (U8 Unit, unsigned BitErrorCnt);
void FS_NAND_UNI_SetMaxEraseCntDiff          (U8 Unit, U32 EraseCntDiff);
void FS_NAND_UNI_SetNumWorkBlocks            (U8 Unit, U32 NumWorkBlocks);
void FS_NAND_UNI_SetNumBlocksPerGroup        (U8 Unit, unsigned BlocksPerGroup);
void FS_NAND_UNI_SetOnFatalErrorCallback     (FS_NAND_ON_FATAL_ERROR_CALLBACK * pfOnFatalError);
void FS_NAND_UNI_SetPhyType                  (U8 Unit, const FS_NAND_PHY_TYPE * pPhyType);
void FS_NAND_UNI_SetWriteVerification        (U8 Unit, U8 OnOff);
void FS_NAND_UNI_SetDriverBadBlockReclamation(U8 Unit, U8 OnOff);
int  FS_NAND_UNI_TestBlock                   (U8 Unit, unsigned BlockIndex, U32 Pattern, FS_NAND_TEST_INFO * pInfo);
int  FS_NAND_UNI_WritePage                   (U8 Unit, U32 PageIndex, const void * pData, unsigned NumBytes);
int  FS_NAND_UNI_WritePageRaw                (U8 Unit, U32 PageIndex, const void * pData, unsigned NumBytes);

/*********************************************************************
*
*       NAND physical layer specific functions
*/
void FS_NAND_PHY_ReadDeviceId           (U8 Unit, U8 * pId, U32 NumBytes);
int  FS_NAND_PHY_ReadONFIPara           (U8 Unit, void * pPara);
void FS_NAND_PHY_SetHWType              (U8 Unit, const FS_NAND_HW_TYPE * pHWType);

/*********************************************************************
*
*       2048x16 physical layer
*/
void FS_NAND_2048x16_SetHWType          (U8 Unit, const FS_NAND_HW_TYPE * pHWType);

/*********************************************************************
*
*       2048x8 physical layer
*/
void FS_NAND_2048x8_EnableReadCache     (U8 Unit);
void FS_NAND_2048x8_DisableReadCache    (U8 Unit);
void FS_NAND_2048x8_SetHWType           (U8 Unit, const FS_NAND_HW_TYPE * pHWType);

/*********************************************************************
*
*       4096x8 physical layer
*/
void FS_NAND_4096x8_SetHWType           (U8 Unit, const FS_NAND_HW_TYPE * pHWType);

/*********************************************************************
*
*       512x8 physical layer
*/
void FS_NAND_512x8_SetHWType            (U8 Unit, const FS_NAND_HW_TYPE * pHWType);

/*********************************************************************
*
*       x8 physical layer
*/
void FS_NAND_x8_SetHWType               (U8 Unit, const FS_NAND_HW_TYPE * pHWType);
void FS_NAND_x8_Configure               (U8 Unit, unsigned NumBlocks, unsigned PagesPerBlock, unsigned BytesPerPage, unsigned BytesPerSpareArea);

/*********************************************************************
*
*       x physical layer
*/
void FS_NAND_x_SetHWType                (U8 Unit, const FS_NAND_HW_TYPE * pHWType);
void FS_NAND_x_Configure                (U8 Unit, unsigned NumBlocks, unsigned PagesPerBlock, unsigned BytesPerPage, unsigned BytesPerSpareArea, unsigned DataBusWidth);

/*********************************************************************
*
*       ONFI physical layer
*/
void FS_NAND_ONFI_SetHWType             (U8 Unit, const FS_NAND_HW_TYPE * pHWType);

/*********************************************************************
*
*       SPI physical layer
*/
void FS_NAND_SPI_EnableReadCache        (U8 Unit);
void FS_NAND_SPI_DisableReadCache       (U8 Unit);
void FS_NAND_SPI_SetHWType              (U8 Unit, const FS_NAND_HW_TYPE_SPI * pHWType);

/*********************************************************************
*
*       DataFlash physical layer
*/
void FS_NAND_DF_EraseChip               (U8 Unit);
void FS_NAND_DF_SetMinPageSize          (U8 Unit, U32 NumBytes);
void FS_NAND_DF_SetHWType               (U8 Unit, const FS_NAND_HW_TYPE_DF * pHWType);

/*********************************************************************
*
*       QSPI physical layer
*/
void FS_NAND_QSPI_EnableReadCache       (U8 Unit);
void FS_NAND_QSPI_DisableReadCache      (U8 Unit);
void FS_NAND_QSPI_SetHWType             (U8 Unit, const FS_NAND_HW_TYPE_QSPI * pHWType);
void FS_NAND_QSPI_Allow2bitMode         (U8 Unit, U8 OnOff);
void FS_NAND_QSPI_Allow4bitMode         (U8 Unit, U8 OnOff);

/*********************************************************************
*
*       WinDrive driver
*/
void FS_WINDRIVE_Configure(U8 Unit, const char * sWindowsDriveName);

/*********************************************************************
*
*       Logical drivers
*/
int  FS_AddPhysDevice     (const FS_DEVICE_TYPE * pDevType);

/*********************************************************************
*
*       Logical volume
*/
int  FS_LOGVOL_Create     (const char * sVolName);
int  FS_LOGVOL_AddDevice  (const char * sLogVolName, const FS_DEVICE_TYPE * pDevice, U8 Unit, U32 StartSector, U32 NumSectors);
int  FS_LOGVOL_AddDeviceEx(U8 Unit, const FS_DEVICE_TYPE * pDeviceType, U8 DeviceUnit, U32 StartSector, U32 NumSectors);

/*********************************************************************
*
*       MBR disk partition
*/
void FS_DISKPART_Configure(U8 Unit, const FS_DEVICE_TYPE * pDevice, U8 DeviceUnit, U8 PartIndex);

/*********************************************************************
*
*       Encryption
*/
typedef struct FS_CRYPT_ALGO_TYPE {
  void (*pfPrepare)(void * pContext, const U8 * pKey);
  void (*pfEncrypt)(void * pContext, U8 * pDest, const U8 * pSrc, U32 NumBytes, U32 BlockIndex);
  void (*pfDecrypt)(void * pContext, U8 * pDest, const U8 * pSrc, U32 NumBytes, U32 BlockIndex);
  U16    BitsPerBlock;     // Number of bits encrypted at once by the algorithm: AES -> 128, DES -> 64.
} FS_CRYPT_ALGO_TYPE;

/*********************************************************************
*
*       FS_CRYPT_OBJ
*
*   FS internal structure. Describes an encryption object.
*/
typedef struct FS_CRYPT_OBJ {
  void                     * pContext;
  const FS_CRYPT_ALGO_TYPE * pAlgoType;
  U16                        ldBytesPerBlock;
} FS_CRYPT_OBJ;

/*********************************************************************
*
*       FS_DES_CONTEXT
*
*   FS internal structure. Describes the context of the DES encryption algorithm.
*/
typedef struct FS_DES_CONTEXT {
  U32 _aRoundKey[32];
} FS_DES_CONTEXT;

/*********************************************************************
*
*       FS_AES_CONTEXT
*
*   FS internal structure. Describes the context of the AES encryption algorithm.
*/
typedef struct FS_AES_CONTEXT {
  U32 aRoundKeyEnc[60];
  U32 aRoundKeyDec[60];
} FS_AES_CONTEXT;

extern const FS_CRYPT_ALGO_TYPE   FS_CRYPT_ALGO_DES;      // 56-bit DES encryption
extern const FS_CRYPT_ALGO_TYPE   FS_CRYPT_ALGO_AES128;   // 128-bit AES encryption
extern const FS_CRYPT_ALGO_TYPE   FS_CRYPT_ALGO_AES256;   // 256-bit AES encryption

void FS_CRYPT_Configure(U8 Unit, const FS_DEVICE_TYPE * pDeviceType, U8 DeviceUnit, const FS_CRYPT_ALGO_TYPE * pAlgoType, void * pContext, const U8 * pKey);
void FS_CRYPT_Prepare  (FS_CRYPT_OBJ * pCryptObj, const FS_CRYPT_ALGO_TYPE * pAlgoType, void * pContext, U32 BytesPerBlock, const U8 * pKey);
void FS_CRYPT_Encrypt  (const FS_CRYPT_OBJ * pCryptObj, U8 * pDest, const U8 * pSrc, U32 NumBytes, U32 * pBlockIndex);
void FS_CRYPT_Decrypt  (const FS_CRYPT_OBJ * pCryptObj, U8 * pDest, const U8 * pSrc, U32 NumBytes, U32 * pBlockIndex);

#if FS_SUPPORT_ENCRYPTION
  int FS_SetEncryptionObject(FS_FILE * pFile, FS_CRYPT_OBJ * pCryptObj);
#endif

/*********************************************************************
*
*       Encryption performance test routines
*/
void FS_CRYPT_DES_Encrypt1MB   (void);
void FS_CRYPT_DES_Decrypt1MB   (void);
void FS_CRYPT_AES256_Encrypt1MB(void);
void FS_CRYPT_AES256_Decrypt1MB(void);
void FS_CRYPT_AES128_Encrypt1MB(void);
void FS_CRYPT_AES128_Decrypt1MB(void);

/*********************************************************************
*
*       Sector read ahead driver
*/
typedef struct FS_READAHEAD_STAT_COUNTERS {
  U32 ReadSectorCnt;
  U32 ReadSectorCachedCnt;
} FS_READAHEAD_STAT_COUNTERS;

void FS_READAHEAD_Configure        (U8 Unit, const FS_DEVICE_TYPE * pDevice, U8 DeviceUnit, U32 * pData, U32 NumBytes);
void FS_READAHEAD_GetStatCounters  (U8 Unit, FS_READAHEAD_STAT_COUNTERS * pStat);
void FS_READAHEAD_ResetStatCounters(U8 Unit);

/*********************************************************************
*
*       RAID operating modes
*
*  Description
*    Operating modes of a RAID logical driver.
*
*  Additional information
*    The operating mode of RAID5 logical driver can be queried via
*    the FS_RAID5_GetOperatingMode() API function.
*/ 
#define FS_RAID_OPERATING_MODE_NORMAL        0     // All storage devices present and operating normally.
#define FS_RAID_OPERATING_MODE_DEGRADED      1     // One storage device is not operating properly.
#define FS_RAID_OPERATING_MODE_FAILURE       2     // Two or more storage devices are not operating properly.

/*********************************************************************
*
*       Disk mirroring driver
*/
void FS_RAID1_Configure      (U8 Unit, const FS_DEVICE_TYPE * pDeviceType0, U8 DeviceUnit0, const FS_DEVICE_TYPE * pDeviceType1, U8 DeviceUnit1);
void FS_RAID1_SetSectorRanges(U8 Unit, U32 NumSectors, U32 StartSector0, U32 StartSector1);
void FS_RAID1_SetSyncBuffer  (U8 Unit, void * pBuffer, U32 NumBytes);
void FS_RAID1_SetSyncSource  (U8 Unit, unsigned StorageIndex);

/*********************************************************************
*
*       Driver for disk stripping with parity checking
*/
void FS_RAID5_AddDevice       (U8 Unit, const FS_DEVICE_TYPE * pDeviceType, U8 DeviceUnit, U32 StartSector);
void FS_RAID5_SetNumSectors   (U8 Unit, U32 NumSectors);
int  FS_RAID5_GetOperatingMode(U8 Unit);

/*********************************************************************
*
*       Sector size conversion driver
*/
void FS_SECSIZE_Configure(U8 Unit, const FS_DEVICE_TYPE * pDeviceType, U8 DeviceUnit, U16 BytesPerSector);

/*********************************************************************
*
*       Sector write buffer
*/

/*********************************************************************
*
*       FS_WRBUF_SECTOR_INFO
*
*  FS internal structure. One instance per sector.
*/
typedef struct FS_WRBUF_SECTOR_INFO {
  U32      SectorIndex;
  unsigned IsValid;
} FS_WRBUF_SECTOR_INFO;

/*********************************************************************
*
*       FS_WRBUF_SIZEOF
*
*  Computes the number of bytes required to store the specified number of sectors.
*/
#define FS_SIZEOF_WRBUF(NumSectors, BytesPerSector)     ((sizeof(FS_WRBUF_SECTOR_INFO) + (BytesPerSector)) * NumSectors)

void FS_WRBUF_Configure(U8 Unit, const FS_DEVICE_TYPE * pDeviceType, U8 DeviceUnit, void * pBuffer, U32 NumBytes);

/*********************************************************************
*
*       Cache handling
*/
#if FS_SUPPORT_CACHE

/*********************************************************************
*
*       General Cache mode defines, do not use in application
*/
#define FS_CACHE_MODE_W         0x02
#define FS_CACHE_MODE_D         0x04

/*********************************************************************
*
*       Sector cache modes
*
*  Description
*    Operating modes of sector caches.
*
*  Additional information
*    The operating mode of a cache module can be configured via
*    the FS_CACHE_SetMode() function separately for each sector type.
*/
#define FS_CACHE_MODE_R         0x01                                                    // Pure read cache.
#define FS_CACHE_MODE_WT        (FS_CACHE_MODE_R | FS_CACHE_MODE_W)                     // Write-through cache.
#define FS_CACHE_MODE_WB        (FS_CACHE_MODE_R | FS_CACHE_MODE_W | FS_CACHE_MODE_D)   // Write-back cache.

/*********************************************************************
*
*       Sector cache types
*
*  Description
*    Types of sector caches.
*
*  Additional information
*    The type of a cache module can be configured via FS_AssignCache()
*    function when the sector cache is enabled for a specified volume.
*/
#define FS_CACHE_NONE           NULL                    // Pseudo-type that can be used to disable the sector cache.
#define FS_CACHE_ALL            FS_CacheAll_Init        // A pure read cache.
#define FS_CACHE_MAN            FS_CacheMan_Init        // A pure read cache that caches only the management sectors.
#define FS_CACHE_RW             FS_CacheRW_Init         // A read / write cache module.
#define FS_CACHE_RW_QUOTA       FS_CacheRWQuota_Init    // A read / write cache module with configurable capacity per sector type.
#define FS_CACHE_MULTI_WAY      FS_CacheMultiWay_Init   // A read / write cache module with configurable associativity level.

/*********************************************************************
*
*       Sizes of internal data structures used by the cache modules.
*/
#define FS_SIZEOF_CACHE_ALL_DATA                12    // sizeof(CACHE_ALL_DATA)
#define FS_SIZEOF_CACHE_ALL_BLOCK_INFO          4     // sizeof(CACHE_ALL_BLOCK_INFO)
#define FS_SIZEOF_CACHE_MAN_DATA                12    // sizeof(CACHE_MAN_DATA)
#define FS_SIZEOF_CACHE_MAN_BLOCK_INFO          4     // sizeof(CACHE_MAN_BLOCK_INFO)
#define FS_SIZEOF_CACHE_RW_DATA                 16    // sizeof(CACHE_RW_DATA)
#define FS_SIZEOF_CACHE_RW_BLOCK_INFO           8     // sizeof(CACHE_RW_BLOCK_INFO)
#define FS_SIZEOF_CACHE_RW_QUOTA_DATA           52    // sizeof(CACHE_RW_QUOTA_DATA)
#define FS_SIZEOF_CACHE_RW_QUOTA_BLOCK_INFO     8     // sizeof(CACHE_RW_QUOTA_BLOCK_INFO)
#define FS_SIZEOF_CACHE_MULTI_WAY_DATA          20    // sizeof(CACHE_MULTI_WAY_DATA)
#define FS_SIZEOF_CACHE_MULTI_WAY_BLOCK_INFO    8     // sizeof(CACHE_MULTI_WAY_BLOCK_INFO)

/*********************************************************************
*
*       Sector cache size
*
*  Description
*    Calculates the cache size.
*
*  Additional information
*    These defines can be used to calculate the size of the memory
*    area to be assigned to a cache module based on the size of a
*    sector (\tt SectorSize) and the number of sectors to be cached
*    (\tt NumSectors). The sector size of a specific volume can be
*    queried via FS_GetVolumeInfo().
*/
#define FS_SIZEOF_CACHE_ALL(NumSectors, SectorSize)         (FS_SIZEOF_CACHE_ALL_DATA +               \
                                                              (FS_SIZEOF_CACHE_ALL_BLOCK_INFO +       \
                                                              (SectorSize)) * (NumSectors))                     // Calculates the cache size of a FS_CACHE_ALL cache module.
#define FS_SIZEOF_CACHE_MAN(NumSectors, SectorSize)         (FS_SIZEOF_CACHE_MAN_DATA +               \
                                                              (FS_SIZEOF_CACHE_MAN_BLOCK_INFO +       \
                                                              (SectorSize)) * (NumSectors))                     // Calculates the cache size of a FS_CACHE_MAN cache module.
#define FS_SIZEOF_CACHE_RW(NumSectors, SectorSize)          (FS_SIZEOF_CACHE_RW_DATA +                \
                                                              (FS_SIZEOF_CACHE_RW_BLOCK_INFO +        \
                                                              (SectorSize)) * (NumSectors))                     // Calculates the cache size of a FS_CACHE_RW cache module.
#define FS_SIZEOF_CACHE_RW_QUOTA(NumSectors, SectorSize)    (FS_SIZEOF_CACHE_RW_QUOTA_DATA +          \
                                                              (FS_SIZEOF_CACHE_RW_QUOTA_BLOCK_INFO +  \
                                                              (SectorSize)) * (NumSectors))                     // Calculates the cache size of a FS_CACHE_RW_QUOTA cache module.
#define FS_SIZEOF_CACHE_MULTI_WAY(NumSectors, SectorSize)   (FS_SIZEOF_CACHE_MULTI_WAY_DATA +         \
                                                              (FS_SIZEOF_CACHE_MULTI_WAY_BLOCK_INFO + \
                                                              (SectorSize)) * (NumSectors))                     // Calculates the cache size of a FS_CACHE_MULTI_WAY cache module.
#define FS_SIZEOF_CACHE_ANY(NumSectors, SectorSize)         FS_SIZEOF_CACHE_RW_QUOTA(NumSectors, SectorSize)    // Calculates the size of cache that works with any cache module.

/*********************************************************************
*
*       Cache specific types
*/
typedef U32    FS_INIT_CACHE (FS_DEVICE * pDevice, void * pData, I32 NumBytes);
typedef U32 (* FS_CACHE_TYPE)(FS_DEVICE * pDevice, void * pData, I32 NumBytes);

/*********************************************************************
*
*       Cache specific prototypes
*/
U32    FS_AssignCache        (const char * sVolumeName, void * pData, I32 NumBytes, FS_INIT_CACHE * pfInit);
void   FS_CACHE_Clean        (const char * sVolumeName);
int    FS_CACHE_Command      (const char * sVolumeName, int   Cmd,      void * pData);
int    FS_CACHE_SetMode      (const char * sVolumeName, int   TypeMask, int    ModeMask);
int    FS_CACHE_SetQuota     (const char * sVolumeName, int   TypeMask, U32    NumSectors);
int    FS_CACHE_SetAssocLevel(const char * sVolumeName, int   AssocLevel);
int    FS_CACHE_GetNumSectors(const char * sVolumeName, U32 * pNumSectors);
int    FS_CACHE_Invalidate   (const char * sVolumeName);

/*********************************************************************
*
*       Cache initialization prototypes
*/
U32 FS_CacheAll_Init     (FS_DEVICE * pDevice, void * pData, I32 NumBytes);
U32 FS_CacheMan_Init     (FS_DEVICE * pDevice, void * pData, I32 NumBytes);
U32 FS_CacheRW_Init      (FS_DEVICE * pDevice, void * pData, I32 NumBytes);
U32 FS_CacheRWQuota_Init (FS_DEVICE * pDevice, void * pData, I32 NumBytes);
U32 FS_CacheMultiWay_Init(FS_DEVICE * pDevice, void * pData, I32 NumBytes);

#endif // FS_SUPPORT_CACHE

/*********************************************************************
*
*       File system selection prototype
*/
#if FS_SUPPORT_MULTIPLE_FS
  int FS_SetFSType(const char * sVolumeName, int FSType);
  int FS_GetFSType(const char * sVolumeName);
#endif // FS_SUPPORT_MULTIPLE_FS

/*********************************************************************
*
*       Memory allocation functions
*/
void * FS_Alloc         (I32     NumBytes);
void * FS_AllocZeroed   (I32     NumBytes);
void   FS_AllocZeroedPtr(void ** pp, I32 NumBytes);
void * FS_TryAlloc      (I32     NumBytesReq);
void * FS_GetFreeMem    (I32  *  pNumBytes);
#if FS_SUPPORT_DEINIT
  void FS_Free          (void *  p);
#endif

/*********************************************************************
*
*       File system locking
*/
#if FS_OS_LOCKING
  void FS_Lock        (void);
  void FS_Unlock      (void);
  void FS_LockVolume  (const char * sVolumeName);
  void FS_UnlockVolume(const char * sVolumeName);
#endif // FS_OS_LOCKING

/*********************************************************************
*
*       Instrumentation
*/
typedef struct FS_PROFILE_API {
  void (*pfRecordEndCall)   (unsigned EventId);
  void (*pfRecordEndCallU32)(unsigned EventId, U32 Para0);
  void (*pfRecordVoid)      (unsigned EventId);
  void (*pfRecordU32)       (unsigned EventId, U32 Para0);
  void (*pfRecordU32x2)     (unsigned EventId, U32 Para0, U32 Para1);
  void (*pfRecordU32x3)     (unsigned EventId, U32 Para0, U32 Para1, U32 Para2);
  void (*pfRecordU32x4)     (unsigned EventId, U32 Para0, U32 Para1, U32 Para2, U32 Para3);
  void (*pfRecordU32x5)     (unsigned EventId, U32 Para0, U32 Para1, U32 Para2, U32 Para3, U32 Para4);
  void (*pfRecordU32x6)     (unsigned EventId, U32 Para0, U32 Para1, U32 Para2, U32 Para3, U32 Para4, U32 Para5);
  void (*pfRecordU32x7)     (unsigned EventId, U32 Para0, U32 Para1, U32 Para2, U32 Para3, U32 Para4, U32 Para5, U32 Para6);
  void (*pfRecordString)    (unsigned EventId, const char * pPara0);
  void (*pfRecordStringx2)  (unsigned EventId, const char * pPara0, const char * pPara1);
} FS_PROFILE_API;

U32  FS_PROFILE_GetAPIDesc(const char ** psDesc);
void FS_PROFILE_SetAPI    (const FS_PROFILE_API * pAPI, U32 IdOffset);

#if FS_SUPPORT_PROFILE
  void FS_SYSVIEW_Init(void);
#endif

/*********************************************************************
*
*       Application supplied functions FS_X_...
*/

/*********************************************************************
*
*       Debug support
*/
void FS_X_Log     (const char *s);
void FS_X_Warn    (const char *s);
void FS_X_ErrorOut(const char *s);

/*********************************************************************
*
*       Misc. functions
*/
void FS_X_Panic      (int ErrorCode);
void FS_X_AddDevices (void);
U32  FS_X_GetTimeDate(void);

/*********************************************************************
*
*       Compatibility defines
*/

/*********************************************************************
*
*       Deprecated error codes
*/
#define FS_ERR_OK                       FS_ERRCODE_OK
#define FS_ERR_EOF                      FS_ERRCODE_EOF
#define FS_ERR_CMDNOTSUPPORTED          (-4)    // Deprecated
#define FS_ERR_DISKFULL                 FS_ERRCODE_VOLUME_FULL
#define FS_ERR_INVALIDPAR               FS_ERRCODE_INVALID_PARA
#define FS_ERR_WRITEONLY                FS_ERRCODE_WRITE_ONLY_FILE
#define FS_ERR_READONLY                 FS_ERRCODE_READ_ONLY_FILE
#define FS_ERR_READERROR                FS_ERRCODE_READ_FAILURE
#define FS_ERR_WRITEERROR               FS_ERRCODE_WRITE_FAILURE
#define FS_ERROR_ALLOC                  FS_ERRCODE_OUT_OF_MEMORY
#define FS_ERROR_INVALID_PARA           FS_ERRCODE_INVALID_PARA
#define FS_ERROR_UNKNOWN_DEVICE         FS_ERRCODE_UNKNOWN_DEVICE

/*********************************************************************
*
*       Cache API
*/
#define FS_CACHEALL_API             FS_CacheAll_Init
#define FS_CACHEMAN_API             FS_CacheMan_Init
#define FS_CACHE_MODE_FULL          (FS_CACHE_MODE_R | FS_CACHE_MODE_W | FS_CACHE_MODE_D)

/*********************************************************************
*
*       Configuration API
*/
#define FS_WriteUpdateDir(OnOff)                    FS_ConfigUpdateDirOnWrite(OnOff)
#define FS_ConfigFileEncryption(pFile, pCryptObj)   FS_SetEncryptionObject(pFile, pCryptObj)
#define FS_ConfigFileBufferFlags(pFile, Flags)      FS_SetFileBufferFlags(pFile, Flags)
#define WINDRIVE_Configure(Unit, sWindowsDriveName) FS_WINDRIVE_Configure(Unit, sWindowsDriveName)
#define FS_ConfigUpdateDirOnWrite(OnOff)            FS_ConfigOnWriteDirUpdate(OnOff)

/*********************************************************************
*
*       File API
*/
#define FS_FFlush(pFile)            FS_SyncFile(pFile)
#define FS_FILE_CURRENT             FS_SEEK_CUR
#define FS_FILE_END                 FS_SEEK_END
#define FS_FILE_BEGIN               FS_SEEK_SET

/*********************************************************************
*
*       NAND flash driver
*/
#define FS_NAND_ON_FATAL_ERROR_CB                       FS_NAND_ON_FATAL_ERROR_CALLBACK
#define FS_NAND_SetOnFatalErrorCB(pfOnFatalError)       FS_NAND_SetOnFatalErrorCallback(pfOnFatalError)
#define FS_NAND_ReadDeviceId(Unit, pId, NumBytes)       FS_NAND_PHY_ReadDeviceId(Unit, pId, NumBytes)
#define FS_NAND_ECC_NULL                                FS_NAND_ECC_HW_NULL
#define FS_NAND_ECC_1BIT                                FS_NAND_ECC_SW_1BIT
#define FS_NAND_UNI_SetOnFatalErrorCB(pfOnFatalError)   FS_NAND_UNI_SetOnFatalErrorCallback(pfOnFatalError)
#define NAND_BLOCK_TYPE_UNKNOWN                         FS_NAND_BLOCK_TYPE_UNKNOWN
#define NAND_BLOCK_TYPE_BAD                             FS_NAND_BLOCK_TYPE_BAD
#define NAND_BLOCK_TYPE_EMPTY                           FS_NAND_BLOCK_TYPE_EMPTY
#define NAND_BLOCK_TYPE_WORK                            FS_NAND_BLOCK_TYPE_WORK
#define NAND_BLOCK_TYPE_DATA                            FS_NAND_BLOCK_TYPE_DATA
#define FS_DF_ChipErase(Unit)                           FS_NAND_DF_EraseChip(Unit)

/*********************************************************************
*
*       Journaling add-on
*/
#define FS_CreateJournal(sVolumeName, NumBytes)         FS_JOURNAL_Create(sVolumeName, NumBytes)
#define FS_BeginTransaction(sVolumeName)                FS_JOURNAL_Begin(sVolumeName)
#define FS_EndTransaction(sVolumeName)                  FS_JOURNAL_End(sVolumeName)

/*********************************************************************
*
*       Disk checking
*/
#define FS_ERRCODE_0FILE                      FS_CHECKDISK_ERRCODE_0FILE
#define FS_ERRCODE_SHORTEN_CLUSTER            FS_CHECKDISK_ERRCODE_SHORTEN_CLUSTER
#define FS_ERRCODE_CROSSLINKED_CLUSTER        FS_CHECKDISK_ERRCODE_CROSSLINKED_CLUSTER
#define FS_ERRCODE_FEW_CLUSTER                FS_CHECKDISK_ERRCODE_FEW_CLUSTER
#define FS_ERRCODE_CLUSTER_UNUSED             FS_CHECKDISK_ERRCODE_CLUSTER_UNUSED
#define FS_ERRCODE_CLUSTER_NOT_EOC            FS_CHECKDISK_ERRCODE_CLUSTER_NOT_EOC
#define FS_ERRCODE_INVALID_CLUSTER            FS_CHECKDISK_ERRCODE_INVALID_CLUSTER
#define FS_ERRCODE_INVALID_DIRECTORY_ENTRY    FS_CHECKDISK_ERRCODE_INVALID_DIRECTORY_ENTRY

#define FS_QUERY_F_TYPE                  FS_ON_CHECK_DISK_ERROR_CALLBACK
#define FS_ON_CHECK_DISK_ERROR_CALLBACK  FS_CHECKDISK_ON_ERROR_CALLBACK
#define FS_EFS_CheckDisk(sVolumeName, pBuffer, BufferSize, MaxRecursionLevel, pfOnError)  FS_CheckDisk(sVolumeName, pBuffer, BufferSize, MaxRecursionLevel, pfOnError)
#define FS_FAT_CheckDisk(sVolumeName, pBuffer, BufferSize, MaxRecursionLevel, pfOnError)  FS_CheckDisk(sVolumeName, pBuffer, BufferSize, MaxRecursionLevel, pfOnError)
#define FS_EFS_CheckDisk_ErrCode2Text(ErrCode)                                            FS_CheckDisk_ErrCode2Text(ErrCode)
#define FS_FAT_CheckDisk_ErrCode2Text(ErrCode)                                            FS_CheckDisk_ErrCode2Text(ErrCode)

/*********************************************************************
*
*       Space information functions
*/
#define FS_GetFreeSpace(sVolume)  FS_GetVolumeFreeSpace(sVolume)
#define FS_GetTotalSpace(sVolume) FS_GetVolumeSize(sVolume)

/*********************************************************************
*
*       FAT
*/
#define FS_FormatSD(sVolumeName)              FS_FAT_FormatSD(sVolumeName)
#define FS_FAT_ConfigMaintainFATCopy(OnOff)   FS_FAT_ConfigFATCopyMaintenance(OnOff)
#define FS_FAT_ConfigUseFSInfoSector(OnOff)   FS_FAT_ConfigFSInfoSectorUse(OnOff)

/*********************************************************************
*
*       SPI bus width handling
*/
#define FS_NOR_BUSWIDTH_CMD_SHIFT                 FS_BUSWIDTH_CMD_SHIFT
#define FS_NOR_BUSWIDTH_ADDR_SHIFT                FS_BUSWIDTH_ADDR_SHIFT
#define FS_NOR_BUSWIDTH_DATA_SHIFT                FS_BUSWIDTH_DATA_SHIFT
#define FS_NOR_BUSWIDTH_MASK                      FS_BUSWIDTH_MASK
#define FS_NOR_BUSWIDTH_CMD_1BIT                  FS_BUSWIDTH_CMD_1BIT
#define FS_NOR_BUSWIDTH_CMD_2BIT                  FS_BUSWIDTH_CMD_2BIT
#define FS_NOR_BUSWIDTH_CMD_4BIT                  FS_BUSWIDTH_CMD_4BIT
#define FS_NOR_BUSWIDTH_ADDR_1BIT                 FS_BUSWIDTH_ADDR_1BIT
#define FS_NOR_BUSWIDTH_ADDR_2BIT                 FS_BUSWIDTH_ADDR_2BIT
#define FS_NOR_BUSWIDTH_ADDR_4BIT                 FS_BUSWIDTH_ADDR_4BIT
#define FS_NOR_BUSWIDTH_DATA_1BIT                 FS_BUSWIDTH_DATA_1BIT
#define FS_NOR_BUSWIDTH_DATA_2BIT                 FS_BUSWIDTH_DATA_2BIT
#define FS_NOR_BUSWIDTH_DATA_4BIT                 FS_BUSWIDTH_DATA_4BIT
#define FS_NOR_MAKE_BUSWIDTH(Cmd, Addr, Data)     FS_BUSWIDTH_MAKE(Cmd, Addr, Data)
#define FS_NOR_GET_BUSWIDTH_CMD(BusWidth)         FS_BUSWIDTH_GET_CMD(BusWidth)
#define FS_NOR_GET_BUSWIDTH_ADDR(BusWidth)        FS_BUSWIDTH_GET_ADDR(BusWidth)
#define FS_NOR_GET_BUSWIDTH_DATA(BusWidth)        FS_BUSWIDTH_GET_DATA(BusWidth)

#if defined(__cplusplus)
  }              /* Make sure we have C-declarations in C++ programs */
#endif

#endif // __FS_H__

/*************************** End of file ****************************/
