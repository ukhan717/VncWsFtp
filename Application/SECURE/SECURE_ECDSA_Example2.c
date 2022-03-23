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

#include "SECURE_ECDSA.h"
#include "SECURE_ECDSA_PrivateKey_P256.h"
#include "SECURE_ECDSA_PublicKey_P256.h"
#include <stdio.h>

/*********************************************************************
*
*       Static const data
*
**********************************************************************
*/

static const U8 _aMessagePart1[]       = { "This is a message, " };
static const U8 _aMessagePart2[]       = { "sign me." };

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

static U8 _aSignature[64];

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/

static int _Sign(void) {
  SECURE_ECDSA_HASH_CONTEXT Context;
  //
  // Sign message incrementally.
  //
  SECURE_ECDSA_HASH_Init(&Context);
  SECURE_ECDSA_HASH_Add (&Context, &_aMessagePart1[0], sizeof(_aMessagePart1));
  SECURE_ECDSA_HASH_Add (&Context, &_aMessagePart2[0], sizeof(_aMessagePart2));
  //
  return SECURE_ECDSA_HASH_Sign(&Context,
                                &_SECURE_ECDSA_PrivateKey_P256,
                                &_aSignature[0], sizeof(_aSignature));
}

static int _Verify(void) {
  SECURE_ECDSA_HASH_CONTEXT Context;
  //
  // Verify message incrementally.
  //
  SECURE_ECDSA_HASH_Init(&Context);
  SECURE_ECDSA_HASH_Add (&Context, &_aMessagePart1[0], sizeof(_aMessagePart1));
  SECURE_ECDSA_HASH_Add (&Context, &_aMessagePart2[0], sizeof(_aMessagePart2));
  //
  return SECURE_ECDSA_HASH_Verify(&Context,
                                  &_SECURE_ECDSA_PublicKey_P256,
                                  &_aSignature[0], sizeof(_aSignature));
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

int main(void) {
  if (_Sign() > 0) {
    printf("Signed message...SUCCESS!\n");
    if (_Verify() > 0) {
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
