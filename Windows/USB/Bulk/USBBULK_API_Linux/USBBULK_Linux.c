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
File    : USBBULK_Linux.c
Purpose : USB API functions
---------------------------END-OF-HEADER------------------------------
*/

/*********************************************************************
*
*       #include Section
*
**********************************************************************
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdarg.h>
#include <ctype.h>
#include <poll.h>
#include "USBBULK.h"
#include <sys/ioctl.h>
#include <linux/ioctl.h>
#include <linux/usb/ch9.h>
#include <linux/usbdevice_fs.h>
#include <libudev.h>
#include <pthread.h>

/*********************************************************************
*
*       Defines, configurable
*
**********************************************************************
*/

#ifndef USBBULK_DEBUG
  #define USBBULK_DEBUG           0
#endif

#define DEFAULT_READ_TIMEOUT   5000
#define DEFAULT_WRITE_TIMEOUT  5000


#define MAX_DEVICE_ITEMS          5
#define MAX_PATH                256

#define READ_BUFF_SIZE       0x10000
#define MAX_WRITE_SIZE       0x40000

#define SYSFS_USB_PATH          "/dev/bus/usb"

/*********************************************************************
*
*       Defines, non-configurable
*
**********************************************************************
*/

#define USBBULK_VERSION    10202

#if USBBULK_DEBUG > 0
  #define LOG(Args)  _Log Args
#else
  #define LOG(Args)
#endif


/*********************************************************************
*
*       Types
*
**********************************************************************
*/

typedef struct _DESC_INFO {
  U8              EPin;
  U8              EPout;
  U16             MaxPacketSizeIn;
  U16             MaxPacketSizeOut;
} DESC_INFO;

typedef struct _DEV_INST {
  int             Handle;
  U16             BusNo;
  U16             DeviceNo;
  U16             InterfaceNo;
  int             hDevice;           // handle to device
  int             ReadTimeout;
  int             WriteTimeout;
  U8              ShortMode;
  U8              Speed;
  DESC_INFO       DescInfo;
  char            acName[64];
  char            acSN[64];
  char            acVendor[64];
  U16             VendorId;
  U16             ProductId;
  U32             NumBytesInBuffer;
  U8            * pBuffData;
  U8              acBuffer[READ_BUFF_SIZE];
  int             DescSize;
  U8            * pDesc;
} DEV_INST;

typedef struct _DEVICE_ITEM_LIST {
  U16         VendorId;
  U16         ProductId;
} DEVICE_ITEM_LIST;

typedef struct _USBBULK_GLOBAL {
  int                         NextHandle;
  USBBULK_NOTIFICATION_FUNC * pfOnUserNotification;
  void                      * pfUserNotificationContext;
  pthread_t                   thHandle;
  pthread_mutex_t             Mutex;
  char                        ThreadActive;
  char                        Terminate;
} USBBULK_GLOBAL;

typedef struct {
  USB_BULK_HANDLE hDevice;
  U32             NumBytes;
  const U8        *pData;
} WRITE_JOB;

/*********************************************************************
*
*       Static const
*
**********************************************************************
*/

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
static int              _IsInited;
static USBBULK_GLOBAL   _Global;
static DEVICE_ITEM_LIST _DeviceItemList[MAX_DEVICE_ITEMS];
static DEV_INST        *_Devices[USBBULK_MAX_DEVICES];

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/

/*********************************************************************
*
*       _Log
*/
#if USBBULK_DEBUG > 0
static void _Log(const char * pFormat, ...) {
  va_list ArgList;
  static FILE *LogFd = NULL;

  if (LogFd == NULL) {
    LogFd = fopen("/tmp/USBBULK.log", "a");
  }
  if (LogFd != NULL) {
    va_start(ArgList, pFormat);
    vfprintf(LogFd, pFormat, ArgList);
    va_end(ArgList);
    fflush(LogFd);
  }
}
#endif

/*********************************************************************
*
*       _ParseDescriptors
*
*  Function description
*    Parses a USB descriptor for a specific descriptor and extract
*    endpoint information.
*
*  Return value
*    0                  - Ok
*    < 0                - error
*
*/
static int _ParseDescriptors(U8 *pDesc, int Size, int InterfaceIdx, DESC_INFO *pDescInfo) {
  U8    *p;
  int    IntfFound;
  struct usb_interface_descriptor IntfDesc;
  struct usb_endpoint_descriptor  EndpDesc;

  memset(pDescInfo, 0, sizeof(DESC_INFO));
  memset(&IntfDesc, 0, sizeof(IntfDesc));
  IntfFound   = 0;
  p = pDesc;
  while (Size > 2 && Size >= *p) {
    LOG(("  Parse[%u]: %02x rest %d\n", InterfaceIdx, *p, Size));
    switch (p[1]) {
    case USB_DT_ENDPOINT:
      if (IntfFound == 0) {
        break;
      }
      memcpy(&EndpDesc, p, sizeof(EndpDesc));
      if (EndpDesc.bmAttributes == USB_ENDPOINT_XFER_BULK) {
        if (EndpDesc.bEndpointAddress & USB_ENDPOINT_DIR_MASK) {
          pDescInfo->EPin = EndpDesc.bEndpointAddress;
          pDescInfo->MaxPacketSizeIn = EndpDesc.wMaxPacketSize;

        } else {
          pDescInfo->EPout = EndpDesc.bEndpointAddress;
          pDescInfo->MaxPacketSizeOut = EndpDesc.wMaxPacketSize;
        }
      }
      break;
    case USB_DT_INTERFACE:
      if (IntfFound) {
        Size = 0;
        break;
      }
      memcpy(&IntfDesc, p, sizeof(IntfDesc));
      if (InterfaceIdx > 0) {
        InterfaceIdx--;
        break;
      }
      IntfFound = 1;
      break;
    default:
      break;
    }
    Size -= *p;
    p    += *p;
  }
  if (IntfFound == 0) {
    LOG(("BULK interface not found\n"));
    return -1;
  }
  if (IntfDesc.bInterfaceClass != 0xFF) {
    LOG(("Is not a BULK interface\n"));
    return -1;
  }
  if (pDescInfo->EPin == 0) {
    LOG(("IN endpoint not found\n"));
    return -1;
  }
  if (pDescInfo->EPout == 0) {
    LOG(("OUT endpoint not found\n"));
    return -1;
  }
  LOG(("EPin=%02X (%u), EPout=%02X (%u)\n", pDescInfo->EPin, pDescInfo->MaxPacketSizeIn,
                                            pDescInfo->EPout, pDescInfo->MaxPacketSizeOut));
  return 0;
}


/*********************************************************************
*
*       _ScanForDevices
*
*  Function description
*    Scan for attached USB devices and store into _Devices[]
*
*/
static void _ScanForDevices(void) {
  struct udev *udev;
  struct udev_enumerate *enumerate;
  struct udev_list_entry *devices, *dev_list_entry;
  int    i;
  U16    VendorId;
  U16    ProductId;
  U16    BusNo;
  U16    DeviceNo;
  U8     Speed;
  int    DevIdx;
  int    NumIntf;
  int    Intf;
  int    DescSize;
  U8     aUsedFlag[USBBULK_MAX_DEVICES];
  DEV_INST  * pDev;
  DESC_INFO   DescInfo;
  char   Buff[256];

  if (_Global.ThreadActive) {
    //
    // Notification thread is running, so we have to make sure not to modify
    // the global device list by two threads at the same time.
    //
    pthread_mutex_lock(&_Global.Mutex);
    if (_Global.Terminate) {
      //
      // The API was asked to terminate, so stop here.
      //
      pthread_mutex_unlock(&_Global.Mutex);
      return;
    }
  }
  LOG(("ScanForDevices\n"));
  memset(aUsedFlag, 0, sizeof(aUsedFlag));
  /* Create the udev object */
  udev = udev_new();
  if (!udev) {
    LOG(("HID: Can't create udev\n"));
    return;
  }
  //
  // Create a list of the devices.
  //
  enumerate = udev_enumerate_new(udev);
  udev_enumerate_scan_devices(enumerate);
  devices = udev_enumerate_get_list_entry(enumerate);
  //
  // For each item, see if it matches the vid/pid, and if so
  // create a udev_device record for it
  //
  udev_list_entry_foreach(dev_list_entry, devices) {
    const char *sysfs_path;
    const char *str;
    struct udev_device *usb_dev;        // The device's udev node.
    struct udev_device *dev;            // The actual hardware device.

    //
    // Get the filename of the /sys entry for the device
    // and create a udev_device object (dev) representing it
    //
    sysfs_path = udev_list_entry_get_name(dev_list_entry);
    usb_dev = udev_device_new_from_syspath(udev, sysfs_path);
    //
    // In order to get information about the
    // USB device, get the parent device with the
    // subsystem/devtype pair of "usb"/"usb_device". This will
    // be several levels up the tree, but the function will find
    // it.
    //
    dev = udev_device_get_parent_with_subsystem_devtype(usb_dev, "usb", "usb_device");
    if (dev) {
      LOG(("check device %s\n", sysfs_path));
      //
      // OK, we have found a USB device. Check if it matches VID and PID.
      //
      str = udev_device_get_sysattr_value(dev,"idVendor");
      VendorId = (str)? strtol(str, NULL, 16): 0x0;
      str = udev_device_get_sysattr_value(dev, "idProduct");
      ProductId = (str)? strtol(str, NULL, 16): 0x0;
      LOG(("  VID=%04x, PID=%04X\n", VendorId, ProductId));
      //
      // Search in item list
      //
      for (i = 0; i < MAX_DEVICE_ITEMS; i++) {
        if (_DeviceItemList[i].VendorId == VendorId && _DeviceItemList[i].ProductId == ProductId) {
          break;
        }
      }
      if (i == MAX_DEVICE_ITEMS) {
        LOG(("  ignored.\n"));
        udev_device_unref(usb_dev);
        continue;
      }
      //
      // Get number of interfaces.
      //
      NumIntf = 0;
      str = udev_device_get_sysattr_value(dev,"bNumInterfaces");
      if (str != NULL) {
        NumIntf = atoi(str);
      }
      //
      // Get bus and device no.
      //
      str = udev_device_get_sysattr_value(dev,"busnum");
      if (str == NULL) {
        NumIntf = 0;
        BusNo = 0;
      } else {
        BusNo = atoi(str);
      }
      str = udev_device_get_sysattr_value(dev,"devnum");
      if (str == NULL) {
        NumIntf = 0;
        DeviceNo = 0;
      } else {
        DeviceNo = atoi(str);
      }
      LOG(("  Num interfaces = %d\n", NumIntf));
      str = udev_device_get_sysattr_value(dev,"speed");
      Speed = 0;
      if (str != NULL) {
        unsigned SpeedVal;
        SpeedVal = atoi(str);
        if (SpeedVal >= 1)    Speed = USBBULK_SPEED_LOW;
        if (SpeedVal >= 12)   Speed = USBBULK_SPEED_FULL;
        if (SpeedVal >= 480)  Speed = USBBULK_SPEED_HIGH;
        if (SpeedVal >= 5000) Speed = USBBULK_SPEED_SUPER;
      }
      //
      // Get descriptors
      //
      str = udev_device_get_syspath(dev);
      if (str == NULL) {
        NumIntf = 0;
      } else {
        int fd;
        snprintf(Buff, sizeof(Buff), "%s/descriptors", str);
        if ((fd = open(Buff, 0)) < 0) {
          NumIntf = 0;
        } else {
          DescSize = read(fd, Buff, sizeof(Buff));
          LOG(("DescSize = %d\n", DescSize));
          if (DescSize <= 0) {
            NumIntf = 0;
          }
          close(fd);
        }
      }
      for(Intf = 0; Intf < NumIntf; Intf++) {
        //
        // Check for BULK EPs
        //
        if (_ParseDescriptors((U8 *)Buff, DescSize, Intf, &DescInfo) < 0) {
          LOG((" skipped (no EPs found)\n"));
          continue;
        }
        //
        // Check if device is already known
        //
        pDev = NULL;
        for (DevIdx = 0; DevIdx < USBBULK_MAX_DEVICES; DevIdx++) {
          if (_Devices[DevIdx] != NULL &&
              _Devices[DevIdx]->BusNo == BusNo &&
              _Devices[DevIdx]->DeviceNo == DeviceNo &&
              _Devices[DevIdx]->InterfaceNo == Intf) {
            pDev = _Devices[DevIdx];
            break;
          }
        }
        if (pDev == NULL) {
          //
          // not found, find free slot
          //
          for (DevIdx = 0; DevIdx < USBBULK_MAX_DEVICES; DevIdx++) {
            if (_Devices[DevIdx] == NULL) {
              pDev = (DEV_INST *)malloc(sizeof(DEV_INST));
              memset(pDev, 0, sizeof(DEV_INST));
              pDev->ReadTimeout  = DEFAULT_READ_TIMEOUT;
              pDev->WriteTimeout = DEFAULT_WRITE_TIMEOUT;
              _Devices[DevIdx] = pDev;
              aUsedFlag[DevIdx] = 2;
              break;
            }
          }
        }
        if (pDev == NULL) {
          LOG(("Could not add device. List is full.\n"));
        } else {
          aUsedFlag[DevIdx]   |= 1;
          pDev->hDevice        = -1;
          pDev->VendorId       = VendorId;
          pDev->ProductId      = ProductId;
          pDev->BusNo          = BusNo;
          pDev->DeviceNo       = DeviceNo;
          pDev->InterfaceNo    = (U16)Intf;
          pDev->DescInfo       = DescInfo;
          pDev->Speed          = Speed;
          if (pDev->pDesc != NULL) {
            free(pDev->pDesc);
          }
          pDev->DescSize       = DescSize;
          pDev->pDesc          = malloc(DescSize);
          memcpy(pDev->pDesc, Buff, DescSize);
          LOG(("Add device Index: %d, ProductId: 0x%04x, BusNo: %03d, DeviceNo: %03d.\n", DevIdx, ProductId, BusNo, DeviceNo));
          str = udev_device_get_sysattr_value(dev,"product");
          if (str) {
            strncpy(pDev->acName, str, sizeof(pDev->acName) - 1);
          }
          str = udev_device_get_sysattr_value(dev,"manufacturer");
          if (str) {
            strncpy(pDev->acVendor, str, sizeof(pDev->acVendor) - 1);
          }
          str = udev_device_get_sysattr_value(dev,"serial");
          if (str) {
            strncpy(pDev->acSN, str, sizeof(pDev->acSN) - 1);
          }
          LOG(("   Product=%s, Vendor=%s, SN=%s, Intf=%u\n", pDev->acName, pDev->acVendor, pDev->acSN, pDev->InterfaceNo));
        }
      }
    }
    udev_device_unref(usb_dev);
  }
  for (i = 0; i < USBBULK_MAX_DEVICES; i++) {
    if (aUsedFlag[i] == 0 && _Devices[i] != NULL && _Devices[i]->hDevice < 0) {
      if (_Devices[i]->pDesc != NULL) {
        free(_Devices[i]->pDesc);
      }
      free(_Devices[i]);
      _Devices[i] = NULL;
      aUsedFlag[i] = 4;
    }
  }
  if (_Global.ThreadActive) {
    pthread_mutex_unlock(&_Global.Mutex);
  }
  if (_Global.pfOnUserNotification) {
    for (i = 0; i < USBBULK_MAX_DEVICES; i++) {
      if (aUsedFlag[i] & 2) {
        _Global.pfOnUserNotification(_Global.pfUserNotificationContext, i, USBBULK_DEVICE_EVENT_ADD);
      }
      if (aUsedFlag[i] & 4) {
        _Global.pfOnUserNotification(_Global.pfUserNotificationContext, i, USBBULK_DEVICE_EVENT_REMOVE);
      }
    }
  }
}

/*********************************************************************
*
*       _Handle2Inst
*
*  Function description
*    Find device instance for a given handle.
*
*  Return value
*    Pointer on instance on success.
*    NULL, if handle invalid.
*
*/
static DEV_INST *_Handle2Inst(int Handle) {
  int i;

  if (Handle <= 0) {
    return NULL;
  }
  for (i = 0; i < USBBULK_MAX_DEVICES; i++) {
    if (_Devices[i] != NULL && _Devices[i]->Handle == Handle) {
      return _Devices[i];
    }
  }
  return NULL;
}

/*********************************************************************
*
*       _WriteFunc
*/
static void *_WriteFunc(void *pArg) {
  WRITE_JOB *pJob;
  int       Dumy;

  pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &Dumy);
  pJob = (WRITE_JOB *)pArg;
  USBBULK_Write(pJob->hDevice, pJob->pData, pJob->NumBytes);
  return NULL;
}

/*********************************************************************
*
*       _Init
*/
static void _Init(void) {
  if (_IsInited == 0) {
    memset(&_Global, 0, sizeof(_Global));
    memset(&_DeviceItemList, 0, sizeof(_DeviceItemList));
    memset(&_Devices, 0, sizeof(_Devices));
    _IsInited = 1;
  }
}

/*********************************************************************
*
*       _MonitorFunc
*/
static void *_MonitorFunc(void *dumy) {
  struct udev *udev;
  struct udev_monitor *udev_monitor = NULL;
  int fd;

  udev = udev_new();
  if (udev == NULL) {
    LOG(("udev_new FAILED\n"));
    return NULL;
  }
  udev_monitor = udev_monitor_new_from_netlink(udev, "udev");
  if (udev_monitor == NULL) {
    LOG(("udev_monitor_new_from_netlink FAILED\n"));
    return NULL;
  }
  if (udev_monitor_enable_receiving(udev_monitor) < 0) {
    LOG(("udev_monitor_enable_receiving FAILED\n"));
    return NULL;
  }
  fd = udev_monitor_get_fd(udev_monitor);
  while (_Global.Terminate == 0) {
    int r;
    struct udev_device *device;
    struct pollfd pfd;

    pfd.fd = fd;
    pfd.events = POLLIN;
    r = poll(&pfd, 1, 100);
    if (r < 0 || _Global.Terminate) {
      break;
    }
    if (r == 0) {
      continue;
    }
    device = udev_monitor_receive_device(udev_monitor);
    if (_Global.Terminate) {
      break;
    }
    if (device == NULL) {
      continue;
    }
    LOG(("got udev event\n"));
    udev_device_unref(device);
    _ScanForDevices();
  }
  udev_monitor_unref(udev_monitor);
  return NULL;
}

/*********************************************************************
*
*       _GetDevInfo
*
*  Function description
*    Retrieves information about a USBBULK device
*
*  Parameters
*    pDev        - Pointer to device table.
*    pDevInfo    - Pointer to a device info structure.
*
*/
static void _GetDevInfo(const DEV_INST *pDev, USBBULK_DEV_INFO * pDevInfo) {
  memset(pDevInfo, 0, sizeof(USBBULK_DEV_INFO));
  pDevInfo->VendorId  = pDev->VendorId;
  pDevInfo->ProductId = pDev->ProductId;
  pDevInfo->Speed     = pDev->Speed;
  strncpy(pDevInfo->acSN, pDev->acSN, sizeof(pDevInfo->acSN) - 1);
  strncpy(pDevInfo->acDevName, pDev->acName, sizeof(pDevInfo->acDevName) - 1);
  pDevInfo->InterfaceNo = pDev->InterfaceNo;
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

/*********************************************************************
*
*       USBBULK_Open
*
*  Function description
*    Opens an existing device. The ID of the device can be retrieved by the function
*    USBBULK_GetNumAvailableDevices() via the pDeviceMask parameter. Each bit set in
*    the DeviceMask represents an available device. Currently 32 devices can be managed
*    at once.
*
*  Parameters
*    Id:       Device ID  to be opened (0..31).
*
*  Return value
*    != 0:     Handle to the opened device.
*    == 0:     Error occurred.
*/
USB_BULK_HANDLE USBBULK_Open(unsigned Id) {
  DEV_INST   * pDev;
  int          hDevice;
  char         acDevicePath[MAX_PATH];

  LOG(("USBBULK_Open(%d)\n", Id));
  if (Id >= USBBULK_MAX_DEVICES || _Devices[Id] == NULL) {
    LOG(("bad index: %d\n", Id));
    return 0;
  }
  pDev = _Devices[Id];
  if (pDev->hDevice >= 0) {
    return pDev->Handle;          // already open
  }
  snprintf(acDevicePath, sizeof(acDevicePath), "%s/%03u/%03u", SYSFS_USB_PATH, pDev->BusNo, pDev->DeviceNo);
  hDevice = open(acDevicePath, O_RDWR);
  if (hDevice < 0) {
    LOG(("can't open %s: %s\n", acDevicePath, strerror(errno)));
    return 0;
  }
#if 0
  {
      unsigned int IFNo = 0;
      LOG(("claim=%d\n",ioctl(hDevice, USBDEVFS_CLAIMINTERFACE, &IFNo)));
  }
  if (_ParseDescriptors(hDevice, pDev) < 0) {
    close(hDevice);
    return 0;
  }
#endif
  pDev->hDevice = hDevice;
  pDev->Handle  = ++_Global.NextHandle;
  return pDev->Handle;
}

/*********************************************************************
*
*       USBBULK_Close
*
*  Function description
*    Closes an opened device.
*
*  Parameters
*    hDevice:     Handle to the device that shall be closed.
*/
void USBBULK_Close(USB_BULK_HANDLE hDevice) {
  DEV_INST   * pDev;

  if ((pDev = _Handle2Inst(hDevice)) == NULL) {
    return;
  }
  if (pDev->hDevice >= 0) {
    close(pDev->hDevice);
    pDev->hDevice = -1;
  }
  pDev->Handle = 0;
}

/*********************************************************************
*
*       USBBULK_Read
*
*  Function description
*    Reads data from target device running emUSB-Device-Bulk.
*
*  Parameters
*    hDevice:     Handle to the opened device.
*    pBuffer:     Pointer to a buffer that shall receive the data.
*    NumBytes:    Number of bytes to be read.
*
*  Return value
*    == NumBytes:      All bytes have been successfully read.
*    > 0, < NumBytes:  Number of bytes that have been read.
*                      If short read transfers are not allowed (normal mode)
*                      this indicates a timeout.
*    == 0:             A timeout occurred, no data was read.
*    < 0:              Error occurred.
*
*  Additional information
*    If short read transfers are allowed (see USBBULK_SetMode()) the function returns as
*    soon as data is available, even if just a single byte was read. Otherwise the function
*    blocks until NumBytes were read. In both cases the function returns if a timeout
*    occurs. The default timeout used can be set with USBBULK_SetReadTimeout().
*
*    If NumBytes exceeds the maximum read size the driver can handle (the default value
*    is 64 Kbytes), USBBULK_Read() will read the desired NumBytes in chunks of the maximum
*    read size.
*/
int USBBULK_Read(USB_BULK_HANDLE hDevice, void * pBuffer, int NumBytes) {
  DEV_INST   * pDev;

  if ((pDev = _Handle2Inst(hDevice)) == NULL) {
    return -1;
  }
  return USBBULK_ReadTimed(hDevice, pBuffer, NumBytes, pDev->ReadTimeout);
}

/*********************************************************************
*
*       USBBULK_ReadTimed
*
*  Function description
*    Reads data from target device running emUSB-Device-Bulk within a given timeout.
*
*  Parameters
*    hDevice:     Handle to the opened device.
*    pBuffer:     Pointer to a buffer that shall receive the data.
*    NumBytes:    Maximum number of bytes to be read.
*    ms:          Timeout in milliseconds.
*
*  Return value
*    > 0:              Number of bytes that have been read.
*    == 0:             A timeout occurred during read.
*    < 0:              Error, cannot read from the device.
*
*  Additional information
*    The function returns as soon as data is available, even if just a single byte was read.
*    If no data is available, the functions return after the given timeout was expired.
*
*    If NumBytes exceeds the maximum read size the driver can handle (the default value
*    is 64 Kbytes), USBBULK_ReadTimed() will read the desired NumBytes in chunks of the
*    maximum read size.
*/
int USBBULK_ReadTimed(USB_BULK_HANDLE hDevice, void * pBuffer, int NumBytes, unsigned ms) {
  U8         * pBuff;
  DEV_INST   * pDev;
  U32          BytesAtOnce;
  int          Result;

  if ((pDev = _Handle2Inst(hDevice)) == NULL) {
    return -1;
  }
  Result = 0;
  pBuff = (U8 *)pBuffer;
  while (NumBytes > 0) {
    if (pDev->NumBytesInBuffer == 0) {
      struct usbdevfs_bulktransfer Request;
      int    NumBytesRead;
      //
      // Buffer empty, issue USB read
      //
      Request.ep      = pDev->DescInfo.EPin;
      Request.len     = READ_BUFF_SIZE;
      Request.timeout = ms;
      Request.data    = pDev->acBuffer;
      NumBytesRead = ioctl(pDev->hDevice, USBDEVFS_BULK, &Request);
      LOG(("Read(%u) = %d\n", ms, NumBytesRead));
      if (NumBytesRead < 0) {
        LOG(("Could not read from device (%s).\n", strerror(errno)));
        if (errno == ETIMEDOUT) {
          Result = 0;
        } else {
          Result = -1;
        }
        break;
      }
      pDev->pBuffData        = pDev->acBuffer;
      pDev->NumBytesInBuffer = NumBytesRead;
    }
    BytesAtOnce = pDev->NumBytesInBuffer;
    if (BytesAtOnce > (U32)NumBytes) {
      BytesAtOnce = NumBytes;
    }
    memcpy(pBuff, pDev->pBuffData, BytesAtOnce);
    pBuff                  += BytesAtOnce;
    NumBytes               -= BytesAtOnce;
    pDev->pBuffData        += BytesAtOnce;
    pDev->NumBytesInBuffer -= BytesAtOnce;
    Result                 += BytesAtOnce;
    if (pDev->ShortMode & USBBULK_MODE_BIT_ALLOW_SHORT_READ) {
      break;
    }
  }
  return Result;
}

/*********************************************************************
*
*       USBBULK_Write
*
*  Function description
*    Writes data to the device.
*
*  Parameters
*    hDevice:     Handle to the opened device.
*    pBuffer:     Pointer to a buffer that contains the data.
*    NumBytes:    Number of bytes to be written.
*                 If NumBytes == 0, a zero length packet is written to the device.
*
*  Return value
*    == NumBytes:      All bytes have been successfully  written.
*    > 0, < NumBytes:  Number of bytes that have been written.
*                      If short read transfers are not allowed (normal mode)
*                      this indicates a timeout.
*    == 0:             A timeout occurred, no data was written.
*    < 0:              Error, cannot write to the device.
*
*  Additional information
*    If short write transfers are allowed (see USBBULK_SetMode()) the function returns
*    after writing the minimal amount of data (either NumBytes or the maximal write
*    transfer size). Otherwise the function blocks until NumBytes were written. In both
*    cases the function returns if a timeout occurs. The default timeout used can be set
*    with USBBULK_SetWriteTimeout().
*
*    If NumBytes exceeds the maximum write size the driver can handle (the default value
*    is 64 Kbytes), USBBULK_Write() will write the desired NumBytes in chunks of the
*    maximum write size.
*/
int USBBULK_Write(USB_BULK_HANDLE hDevice, const void * pBuffer, int NumBytes) {
  DEV_INST   * pDev;

  if ((pDev = _Handle2Inst(hDevice)) == NULL) {
    return -1;
  }
  return USBBULK_WriteTimed(hDevice, pBuffer, NumBytes, pDev->WriteTimeout);
}

/*********************************************************************
*
*       USBBULK_WriteEx
*
*  Function description
*    Writes data to the device followed by an optional NULL packet.
*
*  Parameters
*    hDevice:                Handle to the opened device.
*    pBuffer:                Pointer to a buffer that contains the data.
*    NumBytes:               Number of bytes to be written.
*                            If NumBytes == 0, a zero length packet is written to the device.
*    ms:                     Timeout in milliseconds.
*    Send0PacketIfRequired:  1 - if NumBytes is a multiple of the maximum packet size,
*                                then an additional NULL packet is written to the device.
*                            0 - No NULL packet is written.
*
*  Return value
*    == NumBytes:      All bytes have been successfully  written.
*    > 0, < NumBytes:  Number of bytes that have been written.
*                      If short read transfers are not allowed (normal mode)
*                      this indicates a timeout.
*    == 0:             A timeout occurred, no data was written.
*    < 0:              Error, cannot write to the device.
*
*  Additional information
*    If short write transfers are allowed (see USBBULK_SetMode()) the function returns
*    after writing the minimal amount of data (either NumBytes or the maximal write
*    transfer size). Otherwise the function blocks until NumBytes were written. In both
*    cases the function returns if a timeout occurs.
*
*    If NumBytes exceeds the maximum write size the driver can handle (the default value
*    is 64 Kbytes), USBBULK_WriteTimed() will write the desired NumBytes in chunks of the
*    maximum write size.
*/
int USBBULK_WriteEx(USB_BULK_HANDLE hDevice, const void * pBuffer, int NumBytes, unsigned ms, int Send0PacketIfRequired) {
  DEV_INST   * pDev;
  int          Ret;
  int          BytesWritten;

  if ((pDev = _Handle2Inst(hDevice)) == NULL) {
    return -1;
  }
  if (ms == 0) {
    ms = pDev->WriteTimeout;
  }
  BytesWritten = USBBULK_WriteTimed(hDevice, pBuffer, NumBytes, ms);
  if (BytesWritten == NumBytes && Send0PacketIfRequired != 0) {
    if ((BytesWritten & (pDev->DescInfo.MaxPacketSizeOut - 1)) == 0) {
      Ret = USBBULK_WriteTimed(hDevice, NULL, 0, ms);
      if (Ret < 0) {
        BytesWritten = Ret;
      }
    }
  }
  return BytesWritten;
}

/*********************************************************************
*
*       USBBULK_WriteTimed
*
*  Function description
*    Writes data to the device within a given timeout.
*
*  Parameters
*    hDevice:     Handle to the opened device.
*    pBuffer:     Pointer to a buffer that contains the data.
*    NumBytes:    Number of bytes to be written.
*                 If NumBytes == 0, a zero length packet is written to the device.
*    ms:          Timeout in milliseconds.
*
*  Return value
*    == NumBytes:      All bytes have been successfully  written.
*    > 0, < NumBytes:  Number of bytes that have been written.
*                      If short read transfers are not allowed (normal mode)
*                      this indicates a timeout.
*    == 0:             A timeout occurred, no data was written.
*    < 0:              Error, cannot write to the device.
*
*  Additional information
*    If short write transfers are allowed (see USBBULK_SetMode()) the function returns
*    after writing the minimal amount of data (either NumBytes or the maximal write
*    transfer size). Otherwise the function blocks until NumBytes were written. In both
*    cases the function returns if a timeout occurs.
*
*    If NumBytes exceeds the maximum write size the driver can handle (the default value
*    is 64 Kbytes), USBBULK_WriteTimed() will write the desired NumBytes in chunks of the
*    maximum write size.
*/
int USBBULK_WriteTimed(USB_BULK_HANDLE hDevice, const void * pBuffer, int NumBytes, unsigned ms) {
  U8         * pBuff;
  DEV_INST   * pDev;
  U32          BytesAtOnce;
  int          Result;

  if ((pDev = _Handle2Inst(hDevice)) == NULL) {
    return -1;
  }
  Result = 0;
  if (NumBytes == 0) {
    pBuff = (U8 *)"";
  } else {
    pBuff = (U8 *)pBuffer;
  }
  do {
    struct usbdevfs_bulktransfer Request;
    int    NumBytesWritten;

    BytesAtOnce = NumBytes;
    if (BytesAtOnce > MAX_WRITE_SIZE) {
      BytesAtOnce = MAX_WRITE_SIZE;
    }
    //
    // Issue write request.
    //
    Request.ep      = pDev->DescInfo.EPout;
    Request.len     = BytesAtOnce;
    Request.timeout = ms;
    Request.data    = pBuff;
    NumBytesWritten = ioctl(pDev->hDevice, USBDEVFS_BULK, &Request);
    if (NumBytesWritten < 0) {
      LOG(("Could not write to device (%s).\n", strerror(errno)));
      if (errno == ETIMEDOUT) {
        Result = 0;
      } else {
        Result = -1;
      }
      break;
    }
    Result   += NumBytesWritten;
    pBuff    += NumBytesWritten;
    NumBytes -= NumBytesWritten;
  } while (NumBytes > 0);
  return Result;
}

/*********************************************************************
*
*       USBBULK_WriteRead
*
*  Function description
*    Deprecated function. Don't use.
*
*    Writes and reads data to and from target device running emUSB-Device-Bulk.
*
*  Parameters
*    hDevice:     Handle to the opened device.
*    pWrBuffer:   Pointer to a buffer that contains the data to be written.
*    WrNumBytes:  Number of bytes to be written.
*                 If NumBytes == 0, a zero length packet is written to the device.
*    pRdBuffer:   Pointer to a buffer that shall receive the data.
*    RdNumBytes:  Number of bytes to be read.
*
*  Return value
*    == RdNumBytes:      All bytes have been successfully read.
*    > 0, < RdNumBytes:  Number of bytes that have been read.
*                        This indicates a timeout.
*    == 0:               A timeout occurred, no data was read.
*    < 0:                Error occurred.
*
*  Additional information
*    This function can not be used when short read mode is enabled (see USBBULK_SetMode()).
*/
int USBBULK_WriteRead(USB_BULK_HANDLE hDevice, const void * pWrBuffer, int WrNumBytes, void * pRdBuffer, int RdNumBytes) {
  int r;
  pthread_t ThreadId;
  WRITE_JOB WriteJob;
  void      *Dumy;

  WriteJob.hDevice  = hDevice;
  WriteJob.NumBytes = WrNumBytes;
  WriteJob.pData    = pWrBuffer;
  pthread_create(&ThreadId, NULL, _WriteFunc, &WriteJob);
  r = USBBULK_Read(hDevice, pRdBuffer, RdNumBytes);
  pthread_cancel(ThreadId);
  pthread_join(ThreadId, &Dumy);
  return r;
}

/*********************************************************************
*
*       USBBULK_CancelRead
*
*  Function description
*    This cancels an initiated read.
*
*  Parameters
*    hDevice:     Handle to the opened device.
*
*  Additional information
*    Not supported on Linux and MacOSX.
*/
void USBBULK_CancelRead(USB_BULK_HANDLE hDevice) {
}

/*********************************************************************
*
*       USBBULK_GetConfigDescriptor
*
*  Function description
*    Gets the received target USB configuration descriptor of a specified device.
*
*  Parameters
*    hDevice:     Handle to the opened device.
*    pBuffer:     Pointer to the buffer that shall store the descriptor.
*    Size:        Size of the buffer, given in bytes.
*
*  Return value
*    != 0:    Size of the returned USB configuration descriptor (Success).
*    == 0:    Operation failed. Either an invalid handle was used or the buffer that shall
*             store the config descriptor is too small.
*/
int USBBULK_GetConfigDescriptor(USB_BULK_HANDLE hDevice, void* pBuffer, int Size) {
  DEV_INST   * pDev;
  int          DescSize;

  if ((pDev = _Handle2Inst(hDevice)) == NULL) {
    return 0;
  }
  DescSize = pDev->DescSize - 18;
  if (pDev->pDesc != NULL && DescSize > 0 && DescSize <= Size) {
    memcpy(pBuffer, pDev->pDesc + 18, DescSize);
    return DescSize;
  }
  return 0;
}

/*********************************************************************
*
*       USBBULK_ResetPipe
*
*  Function description
*    Resets the pipes that are opened to the device.
*    It also flushes any data the USB bulk driver would cache.
*
*  Parameters
*    hDevice:     Handle to the opened device.
*
*  Return value
*    != 0:    The operation was successful.
*    == 0:    Operation failed. Either an invalid handle was used or
*             the pipes cannot be flushed.
*/
int USBBULK_ResetPipe(USB_BULK_HANDLE hDevice) {
  DEV_INST   * pDev;
  int          Pipe;

  if ((pDev = _Handle2Inst(hDevice)) == NULL) {
    return 0;
  }
  Pipe = pDev->DescInfo.EPin;
  if (ioctl(hDevice, USBDEVFS_CLEAR_HALT, &Pipe)) {
    LOG(("Could not clear IN pipe (%s).\n", strerror(errno)));
    return 0;
  }
  pDev->NumBytesInBuffer = 0;
  Pipe = pDev->DescInfo.EPout;
  if (ioctl(hDevice, USBDEVFS_CLEAR_HALT, &Pipe)) {
    LOG(("Could not clear OUT pipe (%s).\n", strerror(errno)));
    return 0;
  }
  return 1;
}

/*********************************************************************
*
*       USBBULK_FlushRx
*
*  Function description
*    Flush the any received data.
*
*  Parameters
*    hDevice:     Handle to the opened device.
*
*  Return value
*    == 0:    Error, bad handle.
*    != 0:    Success, flushing the RX buffer was successful.
*
*/
int USBBULK_FlushRx(USB_BULK_HANDLE hDevice) {
  DEV_INST   * pDev;

  if ((pDev = _Handle2Inst(hDevice)) == NULL) {
    return 0;
  }
  pDev->NumBytesInBuffer = 0;
  return 1;
}

/*********************************************************************
*
*       USBBULK_ResetDevice
*
*  Function description
*    Resets the device via a USB reset.
*    This can be used when the device does not work properly and may be reactivated via
*    USB reset. This will force a re-enumeration of the device.
*
*  Parameters
*    hDevice:     Handle to the opened device.
*
*  Return value
*    != 0:    The operation was successful.
*    == 0:    Operation failed. Either an invalid handle was used or
*             the device cannot be reset.
*
*  Additional information
*    After the device has been reset it is necessary to re-open the device as the current
*    handle will become invalid.
*/
int USBBULK_ResetDevice(USB_BULK_HANDLE hDevice) {
  DEV_INST   * pDev;

  if ((pDev = _Handle2Inst(hDevice)) == NULL) {
    return 0;
  }
  if (ioctl(hDevice, USBDEVFS_RESET)) {
    LOG(("Could not reset device. Operation failed.\n"));
    return 0;
  }
  return  1;
}

/*********************************************************************
*
*       USBBULK_GetVersion
*
*  Function description
*    Returns the version number of the USBBULK API.
*
*  Return value
*    Version number, format:
*    <Major Version><Minor Version><Subversion> (Mmmrr, decimal).
*
*    Example: 30203 is 3.02c
*/
unsigned USBBULK_GetVersion(void) {
  return USBBULK_VERSION;
}

/*********************************************************************
*
*       USBBULK_GetDriverVersion
*/
unsigned USBBULK_GetDriverVersion(void) {
  return 0;
}

/*********************************************************************
*
*       USBBULK_GetDriverCompileDate
*/
unsigned USBBULK_GetDriverCompileDate(char * s, unsigned Size) {
  *s = 0;
  return 0;
}

/*********************************************************************
*
*       USBBULK_SetMode
*
*  Function description
*    Sets the read and write mode for a specified device running emUSB-Device-Bulk.
*
*  Parameters
*    hDevice:     Handle to the opened device.
*    Mode:        Read and write mode for the USB-Bulk driver.
*                 This is a combination of the following flags, combined by binary or:
*                 * USBBULK_MODE_BIT_ALLOW_SHORT_READ
*                 * USBBULK_MODE_BIT_ALLOW_SHORT_WRITE
*
*  Return value
*    == 0:    Operation failed (invalid handle).
*    != 0:    The operation was successful.
*
*  Additional information
*    USBBULK_MODE_BIT_ALLOW_SHORT_READ allows short read transfers. Short transfers
*    are transfers of less bytes than requested. If this bit is specified, the read function
*    USBBULK_Read() returns as soon as data is available, even if it is just a single byte.
*
*    USBBULK_MODE_BIT_ALLOW_SHORT_WRITE allows short write transfers.
*    USBBULK_Write() and USBBULK_WriteTimed() return after writing the minimal
*    amount of data (either NumBytes or the maximal write transfer size).
*/
unsigned USBBULK_SetMode(USB_BULK_HANDLE hDevice, unsigned Mode) {
  DEV_INST   * pDev;

  if ((pDev = _Handle2Inst(hDevice)) == NULL) {
    return 0;
  }
  pDev->ShortMode = Mode & (USBBULK_MODE_BIT_ALLOW_SHORT_READ | USBBULK_MODE_BIT_ALLOW_SHORT_WRITE);
  return 1;
}

/*********************************************************************
*
*       USBBULK_GetMode
*
*  Function description
*    Returns the current mode of the device.
*
*  Parameters
*    hDevice:     Handle to the opened device.
*
*  Return value
*    A combination of the following flags, combined by binary or:
*    * USBBULK_MODE_BIT_ALLOW_SHORT_READ - Short read mode is enabled.
*    * USBBULK_MODE_BIT_ALLOW_SHORT_WRITE - Short write mode is enabled.
*/
unsigned USBBULK_GetMode(USB_BULK_HANDLE hDevice) {
  DEV_INST   * pDev;

  if ((pDev = _Handle2Inst(hDevice)) == NULL) {
    return 0;
  }
  return pDev->ShortMode;
}

/*********************************************************************
*
*       USBBULK_GetReadMaxTransferSize
*/
unsigned USBBULK_GetReadMaxTransferSize(USB_BULK_HANDLE hDevice) {
  return READ_BUFF_SIZE;
}

/*********************************************************************
*
*       USBBULK_GetWriteMaxTransferSize
*/
unsigned USBBULK_GetWriteMaxTransferSize(USB_BULK_HANDLE hDevice) {
  return MAX_WRITE_SIZE;
}

/*********************************************************************
*
*       USBBULK_GetReadMaxPacketSize
*
*  Function description
*    Returns the maximum packet size for the IN endpoint of the device.
*
*  Parameters
*    hDevice:     Handle to the opened device.
*
*  Return value
*    Maximum packet size in bytes, 0 on error (invalid handle).
*/
unsigned USBBULK_GetReadMaxPacketSize(USB_BULK_HANDLE hDevice) {
  DEV_INST   * pDev;

  if ((pDev = _Handle2Inst(hDevice)) == NULL) {
    return 0;
  }
  return pDev->DescInfo.MaxPacketSizeIn;
}

/*********************************************************************
*
*       USBBULK_GetWriteMaxPacketSize
*
*  Function description
*    Returns the maximum packet size for the OUT endpoint of the device.
*
*  Parameters
*    hDevice:     Handle to the opened device.
*
*  Return value
*    Maximum packet size in bytes, 0 on error (invalid handle).
*/
unsigned USBBULK_GetWriteMaxPacketSize(USB_BULK_HANDLE hDevice) {
  DEV_INST   * pDev;

  if ((pDev = _Handle2Inst(hDevice)) == NULL) {
    return 0;
  }
  return pDev->DescInfo.MaxPacketSizeOut;
}

/*********************************************************************
*
*       USBBULK_GetSN
*
*  Function description
*    Retrieves the USB serial number as a string which was sent by
*    the device during the enumeration.
*
*  Parameters
*    hDevice:     Handle to the opened device.
*    pBuffer:     Pointer to a buffer which shall receive the serial number of the device.
*    BuffSize:    Size of the buffer in bytes.
*
*  Return value
*    == 0:    Operation failed. Either an invalid handle was used or
*             the serial number is not available.
*    != 0:    The operation was successful.
*
*  Additional information
*    If the function succeeds, the buffer pointed by
*    pBuffer contains the serial number of the device as 0-terminated string.
*    If BuffSize is too small, the serial number is truncated.
*/
int USBBULK_GetSN(USB_BULK_HANDLE hDevice, U8 * pBuffer, unsigned BuffSize) {
  DEV_INST   * pDev;

  if ((pDev = _Handle2Inst(hDevice)) == NULL) {
    return 0;
  }
  strncpy((char *)pBuffer, pDev->acSN, BuffSize);
  pBuffer[BuffSize - 1] = 0;
  return 1;
}

/*********************************************************************
*
*       USBBULK_GetUSBId
*
*  Function description
*    Returns the Product and Vendor ID of an opened device.
*
*  Parameters
*    hDevice:     Handle to the opened device.
*    pVendorId:   Pointer to a variable that receives the Vendor ID.
*    pProductId:  Pointer to a variable that receives the Product ID.
*/
void USBBULK_GetUSBId(USB_BULK_HANDLE hDevice, U16 * pVendorId, U16 * pProductId) {
  DEV_INST   * pDev;

  if ((pDev = _Handle2Inst(hDevice)) == NULL) {
    *pProductId = 0;
    *pVendorId  = 0;
  } else {
    *pProductId = pDev->ProductId;
    *pVendorId  = pDev->VendorId;
  }
}

/*********************************************************************
*
*       USBBULK_GetNumAvailableDevices
*
*  Function description
*    Returns the number of connected USB-Bulk devices.
*
*  Parameters
*    pMask:       Pointer to a U32 variable to receive the connected device mask.
*                 This parameter can be  NULL.
*
*  Return value
*    Number of available devices running emUSB-Device-Bulk.
*
*  Additional information
*     For each emUSB-Device device that is connected, a bit in pMask is set.
*     For example if device 0 and device 2 are connected to the host, the value pMask
*     points to will be 0x00000005.
*/
unsigned USBBULK_GetNumAvailableDevices(U32 * pMask) {
  int i;
  unsigned Cnt;
  U32      Mask;

  _ScanForDevices();
  Cnt  = 0;
  Mask = 0;
  for (i = 0; i < USBBULK_MAX_DEVICES; i++) {
    if (_Devices[i] != NULL) {
      Mask |= (1 << i);
      Cnt++;
    }
  }
  if (pMask) {
    *pMask = Mask;
  }
  return Cnt;
}

/*********************************************************************
*
*       USBBULK_GetEnumTickCount
*
*  Function description
*
*
*  Parameters
*    hDevice:     Handle to the opened device.
*/
U32 USBBULK_GetEnumTickCount(USB_BULK_HANDLE hDevice) {
  return 0;
}

/*********************************************************************
*
*       USBBULK_GetReadMaxTransferSizeDown
*/
U32 USBBULK_GetReadMaxTransferSizeDown(USB_BULK_HANDLE hDevice) {
  return 0;
}

/*********************************************************************
*
*       USBBULK_GetWriteMaxTransferSizeDown
*/
U32 USBBULK_GetWriteMaxTransferSizeDown(USB_BULK_HANDLE hDevice) {
  return 0;
}

/*********************************************************************
*
*       USBBULK_SetWriteMaxTransferSizeDown
*/
unsigned USBBULK_SetWriteMaxTransferSizeDown(USB_BULK_HANDLE hDevice, U32 TransferSize) {
  return 0;
}

/*********************************************************************
*
*       USBBULK_SetReadMaxTransferSizeDown
*/
unsigned USBBULK_SetReadMaxTransferSizeDown(USB_BULK_HANDLE hDevice, U32 TransferSize) {
  return 0;
}

/*********************************************************************
*
*       USBBULK_Init
*
*  Function description
*    This function needs to be called first. This makes sure to have
*    all structures and thread have been initialized.
*    It also sets a callback in order to be notified when a device is added or removed.
*
*  Parameters
*    pfNotification:   Pointer to the user callback.
*    pContext:         Context data that shall be called with the callback function.
*/
void USBBULK_Init(USBBULK_NOTIFICATION_FUNC * pfNotification, void * pContext) {
  _Init();
  _Global.pfOnUserNotification      = pfNotification;
  _Global.pfUserNotificationContext = pContext;
  if (pfNotification) {
    //
    // Start notification thread
    //
    _Global.Terminate    = 0;
    _Global.ThreadActive = 1;
    pthread_mutex_init(&_Global.Mutex, NULL);
    pthread_create(&_Global.thHandle, NULL, _MonitorFunc, NULL);
  }
}

/*********************************************************************
*
*       USBBULK_Exit
*
*  Function description
*    This is a cleanup function, it shall be called when exiting the application.
*
*  Additional information
*    We recommend to call this function before exiting the application in order to remove
*    all handles and resources that have been allocated.
*/
void USBBULK_Exit(void) {
  int i;
  DEV_INST * pDev;

  _Global.Terminate            = 1;
  _Global.pfOnUserNotification = NULL;
  if (_Global.ThreadActive) {
    pthread_mutex_lock(&_Global.Mutex);
  }
  for (i = 0; i < USBBULK_MAX_DEVICES; i++) {
    if (_Devices[i] != NULL) {
      pDev = _Devices[i];
      if (pDev->hDevice >= 0) {
        close(pDev->hDevice);
      }
      if (pDev->pDesc != NULL) {
        free(pDev->pDesc);
      }
      free(pDev);
    }
  }
  if (_Global.ThreadActive) {
    pthread_mutex_unlock(&_Global.Mutex);
  }
  _IsInited = 0;
}

/*********************************************************************
*
*       USBBULK_SetReadTimeout
*
*  Function description
*    Sets the default read timeout for an opened device.
*
*  Parameters
*    hDevice:      Handle to the opened device.
*    Timeout:      Timeout in milliseconds.
*
*/
void USBBULK_SetReadTimeout(USB_BULK_HANDLE hDevice, int Timeout) {
  DEV_INST   * pDev;

  if ((pDev = _Handle2Inst(hDevice)) == NULL) {
    return;
  }
  pDev->ReadTimeout = Timeout;
}

/*********************************************************************
*
*       USBBULK_SetWriteTimeout
*
*  Function description
*    Sets a default write timeout for an opened device.
*
*  Parameters
*    hDevice:      Handle to the opened device.
*    Timeout:      Timeout in milliseconds.
*
*/
void USBBULK_SetWriteTimeout(USB_BULK_HANDLE hDevice, int Timeout) {
  DEV_INST   * pDev;

  if ((pDev = _Handle2Inst(hDevice)) == NULL) {
    return;
  }
  pDev->WriteTimeout = Timeout;
}

/*********************************************************************
*
*       USBBULK_GetDevInfo
*
*  Function description
*    Retrieves information about an opened USBBULK device.
*
*  Parameters
*    hDevice:      Handle to the opened device.
*    pDevInfo:     Pointer to a device info structure.
*/
void USBBULK_GetDevInfo(USB_BULK_HANDLE hDevice, USBBULK_DEV_INFO * pDevInfo) {
  DEV_INST   * pDev;

  if ((pDev = _Handle2Inst(hDevice)) == NULL) {
    return;
  }
  _GetDevInfo(pDev, pDevInfo);
}

/*********************************************************************
*
*       USBBULK_GetDevInfoByIdx
*
*  Function description
*    Retrieves information about a USB device.
*
*  Parameters
*    Idx:          Index of the device.
*    pDevInfo:     Pointer to a device info structure.
*
*  Return value
*    == 0;      Error, bad device index.
*    != 0:      Success
*
*/
int USBBULK_GetDevInfoByIdx(unsigned Idx, USBBULK_DEV_INFO * pDevInfo) {
  DEV_INST   * pDev;

  if (Idx >= USBBULK_MAX_DEVICES || _Devices[Idx] == NULL) {
    LOG(("bad index: %d\n", Idx));
    return 0;
  }
  pDev = _Devices[Idx];
  _GetDevInfo(pDev, pDevInfo);
  return 1;
}

/*********************************************************************
*
*       USBBULK_GetVendorName
*
*  Function description
*    Retrieves the vendor name of an opened USBBULK device.
*
*  Parameters
*    hDevice     - Handle to the opened device.
*    sVendorName - Pointer to a buffer that should receive the string.
*    BufferSize  - Size of the buffer, given in bytes.
*
*  Return value
*    == 0:     Error, bad handle.
*    != 0:     Success, vendor name stored in buffer pointed by sVendorName
*              as 0-terminated string.
*
*/
int USBBULK_GetVendorName(USB_BULK_HANDLE hDevice, char * sVendorName, unsigned BufferSize) {
  DEV_INST   * pDev;

  if ((pDev = _Handle2Inst(hDevice)) == NULL) {
    return 0;
  }
  strncpy(sVendorName, pDev->acVendor, BufferSize);
  sVendorName[BufferSize - 1] = 0;
  return 1;
}

/*********************************************************************
*
*       USBBULK_GetProductName
*
*  Function description
*    Retrieves the device/product name if available.
*
*  Parameters
*    hDevice      - Handle to the opened device.
*    sProductName - Pointer to a buffer that should receive the string.
*    BufferSize   - Size of the buffer, given in bytes.
*
*  Return value
*    == 0:     Error, product name not available or buffer to small.
*    != 0:     Success, product name stored in buffer pointed by sProductName
*              as 0-terminated string.
*
*/
int USBBULK_GetProductName(USB_BULK_HANDLE hDevice, char * sProductName, unsigned BufferSize) {
  DEV_INST   * pDev;

  if ((pDev = _Handle2Inst(hDevice)) == NULL) {
    return 0;
  }
  strncpy(sProductName, pDev->acName, BufferSize);
  sProductName[BufferSize - 1] = 0;
  return 1;
}

/*********************************************************************
*
*       USBBULK_AddAllowedDeviceItem
*
*  Function description
*    Adds the Vendor and Product ID to the list of devices the USBBULK
*    API should look for.
*
*  Parameters
*    VendorId:     The desired Vendor ID mask that shall be used with the USBBULK API.
*    ProductId:    The desired Product ID mask that shall be used with the USBBULK API.
*
*  Additional information
*    It is necessary to call this function first before calling
*    USBBULK_GetNumAvailableDevices() or opening any connection to a device.
*
*    The function can be called multiple times to handle more than one pairs
*    of Vendor and Product IDs with the API.
*/
void USBBULK_AddAllowedDeviceItem(U16 VendorId, U16 ProductId) {
  int i;

  for (i = 0; i < MAX_DEVICE_ITEMS; i++) {
    if (_DeviceItemList[i].VendorId == VendorId && _DeviceItemList[i].ProductId == ProductId) {
      return;
    }
  }
  for (i = 0; i < MAX_DEVICE_ITEMS; i++) {
    if (_DeviceItemList[i].VendorId == 0 && _DeviceItemList[i].ProductId == 0) {
      _DeviceItemList[i].VendorId = VendorId;
      _DeviceItemList[i].ProductId = ProductId;
      break;
    }
  }
}

/*********************************************************************
*
*       USBBULK_RemoveAllowedDeviceItem
*
*  Function description
*    Removes the Vendor and Product ID from the list of devices the USBBULK
*    API should look for.
*
*  Parameters
*    VendorId:     The Vendor ID of the entry to be removed.
*    ProductId:    The Product ID of the entry to be removed.
*/
void USBBULK_RemoveAllowedDeviceItem(U16 VendorId, U16 ProductId) {
  int i;

  for (i = 0; i < MAX_DEVICE_ITEMS; i++) {
    if (_DeviceItemList[i].VendorId == VendorId && _DeviceItemList[i].ProductId == ProductId) {
      _DeviceItemList[i].VendorId  = 0;
      _DeviceItemList[i].ProductId = 0;
    }
  }
}

/*********************************************************************
*
*       USBBULK_SetupRequest
*
*  Function description:
*    Performs a setup request on the device within a given timeout.
*
*  Parameters:
*    hDevice:         Handle to the opened device.
*    pSetupRequest:   Pointer to the setup request.
*    pBuffer:         Pointer to a buffer to store the string
*    pNumBytesBuffer: [in]Size of the buffer in bytes.
*                     [out] Number of bytes that have been transferred.
*    Timeout:         Timeout given in ms.
*
*  Return value:
*    == 0   - Success
*    != 0   - Error, setup request failed.
*
*/
int USBBULK_SetupRequest(USB_BULK_HANDLE hDevice, USBBULK_SETUP_REQUEST * pSetupRequest, void * pBuffer, unsigned * pBufferSize, unsigned Timeout) {
  DEV_INST   * pDev;
  int          r;
  struct usbdevfs_ctrltransfer DevRequest;
  int                          NumBytesTransferred;

  if ((pDev = _Handle2Inst(hDevice)) == NULL) {
    return -1;
  }
  r = -1;
  memset(&DevRequest, 0, sizeof(DevRequest));
  DevRequest.bRequestType = pSetupRequest->bRequestType;
  DevRequest.bRequest = pSetupRequest->bRequest;
  DevRequest.wValue = pSetupRequest->wValue;
  DevRequest.wIndex = pSetupRequest->wIndex;
  DevRequest.wLength = pSetupRequest->wLength;
  DevRequest.timeout = Timeout;
 	DevRequest.data = pBuffer;
  NumBytesTransferred = ioctl(pDev->hDevice, USBDEVFS_CONTROL, &DevRequest);
  if (NumBytesTransferred >= 0) {
    if (pBufferSize) {
      *pBufferSize = NumBytesTransferred;
    }
    r = 0;
  }
  return r;
}

/*************************** End of file ****************************/
