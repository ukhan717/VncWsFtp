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
File    : GUI_HelloWorld.c
Purpose : A simple 'Hello World'-application.
Literature:
Notes:
Additional information:
  Preperations:
    Works out-of-the-box.
  Expected behavior:
    This sample dispalys 'Hello World' on the display.
  Sample output:
    Hello World!
*/

#include "GUI.h"

/*********************************************************************
*
*       main()
*/
void MainTask(void);
void MainTask(void) {
  GUI_Init();
  GUI_DispString("Hello World!");
  while(1) {
    GUI_Delay(500);
  }
}

/****** End of File *************************************************/
