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

File        : SSH_SCP.h
Purpose     : emSSH SCP User-Level API.

*/

#ifndef SSH_SCP_H
#define SSH_SCP_H

/*********************************************************************
*
*       #include Section
*
**********************************************************************
*/

#include "SSH.h"
#include "SSH_SCP_ConfDefaults.h"
#include "CRYPTO.h"

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************
*
*       Defines, fixed
*
**********************************************************************
*/

/*********************************************************************
*
*       Public types
*
**********************************************************************
*/

/*********************************************************************
*
*       Opaque types
*/
typedef struct SSH_SCP_SINK_CONTEXT_tag SSH_SCP_SINK_CONTEXT;
  
/*********************************************************************
*
*       SSH-SCP interface to file system
*/
typedef struct {
  void         (*pfConfig)       (const char *sRoot);
  void         (*pfInit)         (unsigned Index);
  int          (*pfCreateFile)   (unsigned Index, unsigned Mode, const char *sName);
  int          (*pfWriteData)    (unsigned Index, const U8 *pData, unsigned DataLen);
  int          (*pfCloseFile)    (unsigned Index);
  int          (*pfEnterFolder)  (unsigned Index, unsigned Mode, const char *sName);
  int          (*pfExitFolder)   (unsigned Index);
  const char * (*pfDecodeStatus) (int Status);
} SSH_SCP_FS_API;

/*********************************************************************
*
*       Public const data
*
**********************************************************************
*/

extern const SSH_SCP_FS_API SSH_SCP_FS_Null;
extern const SSH_SCP_FS_API SSH_SCP_FS_FS;

/*********************************************************************
*
*       Public functions
*
**********************************************************************
*/

int  SSH_SCP_SINK_TrySinkReq  (SSH_SESSION *pSession, unsigned Channel, SSH_CHANNEL_REQUEST_PARAS *pParas);
void SSH_SCP_SINK_Init        (const SSH_SCP_FS_API *pAPI, const char *sRoot);
int  SSH_SCP_SINK_Start       (SSH_SESSION *pSelf, unsigned Index);

#ifdef __cplusplus
}
#endif

#endif

/*************************** End of file ****************************/
