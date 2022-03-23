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

File        : CRYPTO_RSA_Example1.c
Purpose     : Sign and verify a message.

*/

/*********************************************************************
*
*       #include section
*
**********************************************************************
*/

#include "SECURE_RSA.h"
#include "SECURE_RSA_PrivateKey_Expert.h"
#include "SECURE_RSA_PublicKey_Expert.h"
#include <stdio.h>

/*********************************************************************
*
*       Static const data
*
**********************************************************************
*/

static const U8 _aMessage[] = { "This is a message, sign me." };

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

static U8 _aSignature[SECURE_RSA_MAX_KEY_LENGTH / 8];

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

int main(void) {
  int SigLen;
  int Status;
  //
  SigLen = SECURE_RSA_Sign(&_SECURE_RSA_PrivateKey_Expert,
                           0, 0,
                           &_aMessage[0], sizeof(_aMessage),
                           &_aSignature[0], sizeof(_aSignature));
  if (SigLen > 0) {
    printf("Signed message...SUCCESS!\n");
    Status = SECURE_RSA_Verify(&_SECURE_RSA_PublicKey_Expert,
                               0, 0,
                               &_aMessage[0], sizeof(_aMessage),
                               &_aSignature[0], SigLen);
    if (Status > 0) {
      printf("Verified message is correctly signed...SUCCESS!\n");
    } else {
      printf("Correctly signed message did not verify...ERROR!\n");
    }
  } else {
    printf("Failed to sign message...ERROR!\n");
  }
  return 0;
}

/*************************** End of file ****************************/
