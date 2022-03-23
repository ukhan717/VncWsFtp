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
File    : OS_MeasureCST_HRTimer_embOSView.c
Purpose : embOS sample program that measures the embOS context
          switching time and displays the result in the terminal
          window of embOSView. It is completly generic and runs on
          every target that is configured for embOSView.
*/

#include "RTOS.h"
#include <stdio.h>

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

static OS_STACKPTR int StackHP[128];  // Task stack
static OS_TASK         TCBHP;         // Task-control-block
static OS_U32          Time;

/*********************************************************************
*
*       Local functions
*
**********************************************************************
*/

/*********************************************************************
*
*       HPTask()
*/
static void HPTask(void) {
  while (1) {
    OS_TASK_Suspend(NULL);           // Suspend high priority task
    OS_TIME_StopMeasurement(&Time);  // Stop measurement
  }
}

/*********************************************************************
*
*       Global functions
*
**********************************************************************
*/

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
  char   acBuffer[100];    // Output buffer
  OS_U32 MeasureOverhead;  // Time for Measure Overhead
  OS_U32 v;                // Real context switching time

  OS_TASK_CREATE(&TCBHP, "HP Task", 150, HPTask, StackHP);
  OS_TASK_Delay(1);
  //
  // Measure overhead for time measurement so we can take this into account by subtracting it
  //
  OS_TIME_StartMeasurement(&MeasureOverhead);
  OS_TIME_StopMeasurement(&MeasureOverhead);
  //
  // Perform measurements in endless loop
  //
  while (1) {
    OS_TASK_Delay(100);                        // Synchronize to tick to avoid jitter
    OS_TIME_StartMeasurement(&Time);           // Start measurement
    OS_TASK_Resume(&TCBHP);                    // Resume high priority task to force task switch
    v  = OS_TIME_GetResult(&Time);
    v -= OS_TIME_GetResult(&MeasureOverhead);  // Calculate real context switching time (w/o measurement overhead)
    v  = OS_ConvertCycles2us(1000 * v);        // Convert cycles to nano-seconds, increase time resolution
    sprintf(acBuffer, "Context switch time: %lu. %.3lu usec\r", (v / 1000uL), (v % 1000uL));  // Create result text
    OS_COM_SendString(acBuffer);               // Print out result
  }
}


/*************************** End of file ****************************/
