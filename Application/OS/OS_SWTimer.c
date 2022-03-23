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
File    : OS_SWTimer.c
Purpose : embOS sample program running two simple software timer.
*/

#include "RTOS.h"
#include "BSP.h"

static OS_TIMER TIMER50, TIMER200;

static void Timer50(void) {
  BSP_ToggleLED(0);
  OS_TIMER_Restart(&TIMER50);
}

static void Timer200(void) {
  BSP_ToggleLED(1);
  OS_TIMER_Restart(&TIMER200);
}

/*********************************************************************
*
*       MainTask()
*/
#ifdef __cplusplus
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif
void MainTask(void);
#ifdef __cplusplus
}
#endif
void MainTask(void) {
  OS_TIMER_CREATE(&TIMER50,  Timer50,   50);
  OS_TIMER_CREATE(&TIMER200, Timer200, 200);
  OS_TASK_Terminate(NULL);
}

/*************************** End of file ****************************/
