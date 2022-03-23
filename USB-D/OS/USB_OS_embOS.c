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
File    : USB_OS_embOS.c
Purpose : Kernel abstraction for embOS
          Do not modify to allow easy updates !
--------  END-OF-HEADER  ---------------------------------------------
*/


#include "USB_Private.h"
#include "RTOS.h"

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

#if USBD_OS_LAYER_EX > 0
  static OS_MAILBOX _aMailBox[USB_NUM_EPS + USB_EXTRA_EVENTS];
  static U32        _aMBBuffer[USB_NUM_EPS + USB_EXTRA_EVENTS];
  #if USBD_OS_USE_USBD_X_INTERRUPT > 0
    static OS_RSEMA _Sema;
  #endif
#else
  static OS_EVENT   _aEvent[USB_NUM_EPS + USB_EXTRA_EVENTS];
#endif

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       USB_OS_Init
*
*  Function description
*    This function initialize all OS objects that are necessary.
*
*/
void USB_OS_Init(void) {
  unsigned i;

#if USBD_OS_LAYER_EX > 0
  for (i = 0; i < SEGGER_COUNTOF(_aMailBox); i++) {
    OS_CreateMB(_aMailBox + i, sizeof(U32), 1, _aMBBuffer + i);
  }
#if USBD_OS_USE_USBD_X_INTERRUPT > 0
  OS_CREATERSEMA(&_Sema);
#endif
#else
  for (i = 0; i < SEGGER_COUNTOF(_aEvent); i++) {
    OS_EVENT_Create(&_aEvent[i]);
  }
#endif
}

#if USBD_OS_LAYER_EX > 0

/*********************************************************************
*
*       USB_OS_DeInit
*
*  Function description
*    Frees all resources used by the OS layer.
*
*/
void USB_OS_DeInit(void) {
  unsigned i;

  for (i = 0; i < SEGGER_COUNTOF(_aMailBox); i++) {
    OS_DeleteMB(&_aMailBox[i]);
  }
#if USBD_OS_USE_USBD_X_INTERRUPT > 0
  OS_DeleteRSema(&_Sema);
#endif
}

/*********************************************************************
*
*       USB_OS_Signal
*
*  Function description
*    Wakes the task waiting for signal.
*
*  Parameters
*    EPIndex:     Endpoint index. Signaling must be independent for all endpoints.
*    TransactCnt: Transaction counter. Specifies which transaction has been finished.
*
*  Additional information
*    This routine is typically called from within an interrupt
*    service routine.
*/
void USB_OS_Signal(unsigned EPIndex, unsigned TransactCnt) {
  U32 Tmp = TransactCnt;

  while (OS_PutMailCond(&_aMailBox[EPIndex], &Tmp)) {
    OS_ClearMB(&_aMailBox[EPIndex]);
  }
}

/*********************************************************************
*
*        USB_OS_Wait
*
*  Function description
*    Blocks the task until USB_OS_Signal() is called for a given transaction.
*
*  Parameters
*    EPIndex:     Endpoint index. Signaling must be independent for all endpoints.
*    TransactCnt: Transaction counter.  Specifies the transaction to wait for.
*
*  Additional information
*    The function must ignore signaling transactions other than given in  TransactCnt . If
*    this transaction was signaled before this function was called, it must return immediately.
*
*    This routine is called from a task.
*/
void USB_OS_Wait(unsigned EPIndex, unsigned TransactCnt) {
  U32 Tmp;

  do {
    OS_GetMail(&_aMailBox[EPIndex], &Tmp);
  } while (Tmp != TransactCnt);
}

/*********************************************************************
*
*        USB_OS_WaitTimed
*
*  Function description
*    Blocks the task until USB_OS_Signal() is called for a given transaction or a timeout
*    occurs.
*
*  Parameters
*    EPIndex:     Endpoint index. Signaling must be independent for all endpoints.
*    ms:          Timeout time given in ms.
*    TransactCnt: Transaction counter.  Specifies the transaction to wait for.
*
*  Return value
*    == 0:        Task was signaled within the given timeout.
*    == 1:        Timeout occurred.
*
*  Additional information
*    The function must ignore signaling transactions other than given in  TransactCnt . If
*    this transaction was signaled before this function was called, it must return immediately.
*
*    USB_OS_WaitTimed() is called from a task. This function is used by all available timed
*    routines.
*
*    Alternatively this function may take the given timeout in units of system ticks of the
*    underlying  operating  system  instead  of  milliseconds.  In  this  case  all  API  functions
*    that support a timeout parameter use also system ticks for the timeout.
*/
int USB_OS_WaitTimed(unsigned EPIndex, unsigned ms, unsigned TransactCnt) {
  U32 Tmp;
  int r;

  do {
    r = (int)OS_GetMailTimed(&_aMailBox[EPIndex], &Tmp, ms);
  } while (r == 0 && Tmp != TransactCnt);
  return r;
}

#else /* USBD_OS_LAYER_EX == 0 */

/*********************************************************************
*
*       USB_OS_Signal
*
*  Function description
*    Wake the task waiting for reception.
*    This routine is typically called from within an interrupt
*    service routine.
*
*/
void USB_OS_Signal(unsigned EPIndex) {
  OS_EVENT_Pulse(&_aEvent[EPIndex]);
}

/*********************************************************************
*
*        USB_OS_Wait
*
* Function description
*   Block the task until USB_OS_Signal is called.
*   This routine is called from a task.
*
*/
void USB_OS_Wait(unsigned EPIndex) {
  OS_EVENT_Wait(&_aEvent[EPIndex]);
}

/*********************************************************************
*
*        USB_OS_WaitTimed
*
* Function description
*   Block the task until USB_OS_Signal is called
*   or a time out occurs
*   This routine is called from a task.
*
*/
int USB_OS_WaitTimed(unsigned EPIndex, unsigned ms) {
  int r;
  r = (int)OS_EVENT_WaitTimed(&_aEvent[EPIndex], ms + 1);
  return r;
}

#endif /* USBD_OS_LAYER_EX */

/*********************************************************************
*
*       USB_OS_DecRI
*
*  Function description
*    Leave a critical region for the USB stack: Decrements interrupt disable count and
*    enable interrupts if counter reaches 0.
*
*  Additional information
*    The USB stack will perform nested calls to  USB_OS_IncDI()  and  USB_OS_DecRI().
*    This function may be called from a task context or from within an interrupt. If called
*    from an interrupt, it need not do anything.
*
*    An alternate implementation would be to
*      * enable the USB interrupts,
*      * unlock the mutex or semaphore locked in  USB_OS_IncDI()
*    if the disable count reaches 0.
*
*    This may be more efficient, because interrupts of other peripherals can be serviced
*    while inside a critical section of the USB stack.
*/
void USB_OS_DecRI(void) {
#if USBD_OS_LAYER_EX > 0 && USBD_OS_USE_USBD_X_INTERRUPT > 0
  if (OS_InInterrupt() == 0) {
    if (OS_GetSemaValue(&_Sema) == 1) {
      USBD_X_EnableInterrupt();
    }
    OS_Unuse(&_Sema);
  }
#else
  OS_DecRI();
#endif
}

/*********************************************************************
*
*        USB_OS_IncDI
*
*  Function description
*    Enter a critical region for the USB stack: Increments interrupt disable count and
*    disables interrupts.
*
*  Additional information
*    The USB stack will perform nested calls to  USB_OS_IncDI()  and  USB_OS_DecRI().
*    This function may be called from a task context or from within an interrupt. If called
*    from an interrupt, it need not do anything.
*
*    An alternate implementation would be to
*      * perform a lock using a mutex or semaphore and
*      * disable the USB interrupts.
*
*    This may be more efficient, because interrupts of other peripherals can be serviced
*    while inside a critical section of the USB stack.
*/
void   USB_OS_IncDI(void) {
#if USBD_OS_LAYER_EX > 0 && USBD_OS_USE_USBD_X_INTERRUPT > 0
  if (OS_InInterrupt() == 0) {
    if (OS_Use(&_Sema) == 1) {
      USBD_X_DisableInterrupt();
    }
  }
#else
  OS_IncDI();
#endif
}

/*********************************************************************
*
*       USB_OS_Delay
*
*  Function description
*    Delays for a given number of ms.
*
*  Parameters
*    ms:     Number of ms.
*/
void USB_OS_Delay(int ms) {
  OS_Delay(ms);
}

/*********************************************************************
*
*        USB_OS_Panic
*
*  Function description
*    Halts emUSB-Device. Called if a fatal error is detected.
*
*  Parameters
*    pErrMsg:     Pointer to error message string.
*/
void USB_OS_Panic(const char *pErrMsg) {
  USB_OS_IncDI();
  while (pErrMsg);
}

/*********************************************************************
*
*        USB_OS_GetTickCnt
*
*  Function description
*    Returns the current system time in milliseconds or system ticks.
*
*  Return value
*    Current system time.
*/
U32 USB_OS_GetTickCnt(void) {
  return OS_Time;
}

/*************************** End of file ****************************/
