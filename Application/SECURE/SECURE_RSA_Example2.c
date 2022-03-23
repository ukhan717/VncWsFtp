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

File        : CRYPTO_RSA_Example2.c
Purpose     : Incrementally sign and verify a message.

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

static const U8 _aMessagePart1[] = { "This is a message, " };
static const U8 _aMessagePart2[] = { "sign me." };

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

static U8 _aSignature[SECURE_RSA_MAX_KEY_LENGTH / 8];

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/

static int _Sign(void) {
  SECURE_RSA_HASH_CONTEXT Context;
  //
  // Sign message incrementally.
  //
  SECURE_RSA_HASH_Init(&Context);
  SECURE_RSA_HASH_Add (&Context, &_aMessagePart1[0], sizeof(_aMessagePart1));
  SECURE_RSA_HASH_Add (&Context, &_aMessagePart2[0], sizeof(_aMessagePart2));
  //
  return SECURE_RSA_HASH_Sign(&Context,
                              &_SECURE_RSA_PrivateKey_Expert,
                              NULL, 0,
                              &_aSignature[0], sizeof(_aSignature));
}

static int _Verify(int SigLen) {
  SECURE_RSA_HASH_CONTEXT Context;
  //
  // Verify message incrementally.
  //
  SECURE_RSA_HASH_Init(&Context);
  SECURE_RSA_HASH_Add (&Context, &_aMessagePart1[0], sizeof(_aMessagePart1));
  SECURE_RSA_HASH_Add (&Context, &_aMessagePart2[0], sizeof(_aMessagePart2));
  //
  return SECURE_RSA_HASH_Verify(&Context,
                                &_SECURE_RSA_PublicKey_Expert,
                                NULL, 0,
                                &_aSignature[0], SigLen);
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

int main(void) {
  int SigLen;
  //
  SigLen = _Sign();
  if (SigLen > 0) {
    printf("Signed message...SUCCESS!\n");
    if (_Verify(SigLen) > 0) {
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
