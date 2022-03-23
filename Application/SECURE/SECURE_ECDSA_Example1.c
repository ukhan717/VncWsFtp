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

File        : CRYPTO_ECDSA_Example1.c
Purpose     : Sign and verify a message.

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

static const U8 _aMessage[]       = { "This is a message, sign me." };

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
  return SECURE_ECDSA_Sign(&_SECURE_ECDSA_PrivateKey_P256,
                           &_aMessage[0],   sizeof(_aMessage),
                           &_aSignature[0], sizeof(_aSignature));
}

static int _Verify(void) {
  return SECURE_ECDSA_Verify(&_SECURE_ECDSA_PublicKey_P256,
                             &_aMessage[0],   sizeof(_aMessage),
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
