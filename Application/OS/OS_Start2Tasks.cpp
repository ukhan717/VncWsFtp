/*********************************************************************
*                     SEGGER Microcontroller GmbH                    *
*                        The Embedded Experts                        *
**********************************************************************
*                                                                    *
*       (c) 1995 - 2018 SEGGER Microcontroller GmbH                  *
*                                                                    *
*       Internet: segger.com  Support: support_embos@segger.com      *
*                                                                    *
**********************************************************************
*                                                                    *
*       embOS * Real time operating system for microcontrollers      *
*                                                                    *
*       Please note:                                                 *
*                                                                    *
*       Knowledge of this file may under no circumstances            *
*       be used to write a similar product or a real-time            *
*       operating system for in-house use.                           *
*                                                                    *
*       Thank you for your fairness !                                *
*                                                                    *
**********************************************************************
*                                                                    *
*       OS version: 5.00a                                            *
*                                                                    *
**********************************************************************

-------------------------- END-OF-HEADER -----------------------------
File    : OS_Start2Tasks.cpp
Purpose : embOS C++ sample program running two simple tasks.
*/

#include "RTOS.h"
#include "stdlib.h"

/*********************************************************************
*
*       Class definition
*
**********************************************************************
*/
class CThread:private OS_TASK {
public:
  CThread(char* s, unsigned int p, unsigned int t):sName(s), Priority(p), Timeout(t) {
    void* pTaskStack = malloc(256u);
    if (pTaskStack != NULL) {
      OS_TASK_CreateEx(this, sName, Priority, CThread::run, reinterpret_cast<OS_STACKPTR void*>(pTaskStack), 256u, 2u, reinterpret_cast<void*>(Timeout));
    }
  }

private:
  char*        sName;
  unsigned int Priority;
  unsigned int Timeout;

  static void run(void* t) {
    while (1) {
      OS_TASK_Delay(reinterpret_cast<unsigned long>(t));
    }
  }
};

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
CThread *HPTask;

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
  HPTask = new CThread(const_cast<char*>("HPTask"), 100u, 50u);
  while (1) {
    OS_TASK_Delay(200);
  }  
}

/*************************** End of file ****************************/
