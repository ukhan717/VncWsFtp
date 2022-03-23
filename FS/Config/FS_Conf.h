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
File        : FS_Conf.h
Purpose     : File system configuration
---------------------------END-OF-HEADER------------------------------
*/

#ifndef FS_CONF_H
#define FS_CONF_H

#ifdef DEBUG
 #if (DEBUG)
   #define FS_DEBUG_LEVEL     5
 #endif
#endif

#define FS_OS_LOCKING         1

#endif

/*************************** End of file ****************************/