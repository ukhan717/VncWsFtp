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

File        : SECURE_ECDSA_Example3.c
Purpose     : Sign and verify a digest.

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

static const U8 _aMessage[] = { "This is a message, sign me." };

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

static U8 _aSignature[64];
static U8 _aPrecomputedDigest[CRYPTO_SHA256_DIGEST_BYTE_COUNT];

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/

static int _Sign(void) {
  return SECURE_ECDSA_Sign(&_SECURE_ECDSA_PrivateKey_P256,
                           &_aMessage[0], sizeof(_aMessage),
                           &_aSignature[0], sizeof(_aSignature));
}

static int _Verify(void) {
  //
  // Compute the hash outside of the verification function.
  //
  CRYPTO_SHA256_Calc(_aPrecomputedDigest, sizeof(_aPrecomputedDigest), _aMessage, sizeof(_aMessage));
  //
  // Verify the signature over the precomputed digest.
  //
  return SECURE_ECDSA_VerifyDigest(&_SECURE_ECDSA_PublicKey_P256,
                                   &_aPrecomputedDigest[0],
                                   &_aSignature[0], sizeof(_aSignature));
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

int main(void) {
  CRYPTO_Init();
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
