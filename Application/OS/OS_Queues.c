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
File    : OS_Queue.c
Purpose : embOS sample program demonstrating the usage of queues.
*/

#include "RTOS.h"

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
#define MESSAGE_ALIGNMENT    (4u)  // Depends on core/compiler
#define MESSAGES_SIZE_HELLO  (7u + OS_Q_SIZEOF_HEADER + MESSAGE_ALIGNMENT)
#define MESSAGES_SIZE_WORLD  (9u + OS_Q_SIZEOF_HEADER + MESSAGE_ALIGNMENT)
#define QUEUE_SIZE           (MESSAGES_SIZE_HELLO + MESSAGES_SIZE_WORLD)

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

static OS_STACKPTR int StackHP[128];  // Task stacks
static OS_TASK         TCBHP;         // Task control blocks
static OS_QUEUE        MyQueue;
static char            MyQBuffer[QUEUE_SIZE];

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
  char* pData;
  int   Len;

  while (1) {
    Len = OS_QUEUE_GetPtrBlocked(&MyQueue, (void**)&pData);
    OS_TASK_Delay(10);
    //
    // Evaluate Message
    //
    if (Len) {
      OS_COM_SendString(pData);
      OS_QUEUE_Purge(&MyQueue);
    }
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
  OS_QUEUE_Create(&MyQueue, &MyQBuffer, sizeof(MyQBuffer));
  OS_TASK_CREATE(&TCBHP, "HP Task", 150, HPTask, StackHP);
  OS_COM_SendString("embOS OS_Queue example");
  OS_COM_SendString("\n\nDemonstrating message passing\n");
  while (1) {
    OS_QUEUE_Put(&MyQueue, "\nHello\0", 7);
    OS_QUEUE_Put(&MyQueue, "\nWorld !\0", 9);
    OS_TASK_Delay(500);
  }
}

/*************************** End of file ****************************/
