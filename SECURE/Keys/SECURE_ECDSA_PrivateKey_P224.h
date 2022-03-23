static const CRYPTO_MPI_LIMB _SECURE_ECDSA_PrivateKey_P224_X_aLimbs[] = {
  CRYPTO_MPI_LIMB_DATA4(0xBF, 0x58, 0xCC, 0x16),
  CRYPTO_MPI_LIMB_DATA4(0x59, 0x68, 0x32, 0x2A),
  CRYPTO_MPI_LIMB_DATA4(0x1A, 0xD1, 0xFA, 0x82),
  CRYPTO_MPI_LIMB_DATA4(0x30, 0xCD, 0xE0, 0x15),
  CRYPTO_MPI_LIMB_DATA4(0xD6, 0xD3, 0xE3, 0xDA),
  CRYPTO_MPI_LIMB_DATA4(0xC7, 0x4D, 0xD2, 0x09),
  CRYPTO_MPI_LIMB_DATA4(0x96, 0x10, 0xEA, 0xDB)
};

static const CRYPTO_ECDSA_PRIVATE_KEY _SECURE_ECDSA_PrivateKey_P224 = {
  { CRYPTO_MPI_INIT_RO(_SECURE_ECDSA_PrivateKey_P224_X_aLimbs) },
  &CRYPTO_EC_CURVE_secp224r1
};

