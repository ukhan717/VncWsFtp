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
File    : OS_Start2Tasks.c
Purpose : embOS sample program running two simple tasks.
*/

#include "RTOS.h"

static OS_STACKPTR int StackHP[128];  // Task stacks
static OS_TASK         TCBHP;         // Task control blocks

static void HPTask(void) {
  while (1) {
    OS_TASK_Delay(50);
  }
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
  OS_TASK_CREATE(&TCBHP, "HP Task", 150, HPTask, StackHP);
  while (1) {
    OS_TASK_Delay(200);
  }
}

/*************************** End of file ****************************/
