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

File        : SECURE_TrialKey.h
Purpose     : TrialKey for emSecure
              Trial Keys, generated by:
                emKeyGen -l 2048 -pw "SEGGER - The Embedded Experts"
                emPrintKey -m emSecure.pub
                emPrintKey -m emSecure.prv

*/

#ifndef SECURE_TRIAL_KEY_H
#define SECURE_TRIAL_KEY_H

#ifdef SECURE_TRIAL

/*********************************************************************
*
*       #include Section
*
**********************************************************************
*/

#include "CRYPTO_MPI.h"
#include "CRYPTO_RSA.h"

#define _N_LITERAL_DATA \
  __MPI_LITERAL_DATA(0xef, 0x73, 0xa3, 0x82), \
  __MPI_LITERAL_DATA(0x05, 0x3a, 0x25, 0x1b), \
  __MPI_LITERAL_DATA(0xc6, 0x77, 0xfe, 0xae), \
  __MPI_LITERAL_DATA(0x77, 0xfa, 0x56, 0x47), \
  __MPI_LITERAL_DATA(0xdc, 0x0b, 0x00, 0x91), \
  __MPI_LITERAL_DATA(0x08, 0x1d, 0x43, 0xdf), \
  __MPI_LITERAL_DATA(0xa1, 0x7a, 0xf6, 0xd9), \
  __MPI_LITERAL_DATA(0x0d, 0x15, 0x9e, 0x8a), \
  __MPI_LITERAL_DATA(0xd1, 0xe2, 0x20, 0x2e), \
  __MPI_LITERAL_DATA(0x4d, 0x1d, 0x54, 0x7c), \
  __MPI_LITERAL_DATA(0x66, 0xe7, 0x34, 0xfa), \
  __MPI_LITERAL_DATA(0xa1, 0xdf, 0x7f, 0xce), \
  __MPI_LITERAL_DATA(0x82, 0xcf, 0x98, 0x02), \
  __MPI_LITERAL_DATA(0xc3, 0x8b, 0xcc, 0xc7), \
  __MPI_LITERAL_DATA(0x24, 0xa9, 0x08, 0x7e), \
  __MPI_LITERAL_DATA(0x3e, 0x50, 0x2d, 0xe0), \
  __MPI_LITERAL_DATA(0xf6, 0xcc, 0x2e, 0x51), \
  __MPI_LITERAL_DATA(0x82, 0x38, 0xa7, 0x1d), \
  __MPI_LITERAL_DATA(0x2d, 0x17, 0xb1, 0x6c), \
  __MPI_LITERAL_DATA(0x45, 0xea, 0xa6, 0x6c), \
  __MPI_LITERAL_DATA(0x11, 0x4e, 0xff, 0x69), \
  __MPI_LITERAL_DATA(0xa0, 0x98, 0x6b, 0x7d), \
  __MPI_LITERAL_DATA(0x12, 0x02, 0x60, 0x01), \
  __MPI_LITERAL_DATA(0x8a, 0xd0, 0x91, 0x1f), \
  __MPI_LITERAL_DATA(0x5e, 0xc7, 0x6d, 0x6e), \
  __MPI_LITERAL_DATA(0x6f, 0x6a, 0x8b, 0x8a), \
  __MPI_LITERAL_DATA(0xbe, 0xe7, 0x27, 0x4e), \
  __MPI_LITERAL_DATA(0xda, 0x2f, 0x53, 0x1c), \
  __MPI_LITERAL_DATA(0xf1, 0xeb, 0x91, 0x90), \
  __MPI_LITERAL_DATA(0x62, 0xda, 0x95, 0x20), \
  __MPI_LITERAL_DATA(0xc6, 0xf2, 0x4e, 0x67), \
  __MPI_LITERAL_DATA(0x0c, 0x6f, 0x27, 0x05), \
  __MPI_LITERAL_DATA(0x3f, 0xc6, 0x67, 0x2f), \
  __MPI_LITERAL_DATA(0x75, 0x49, 0xfb, 0xb7), \
  __MPI_LITERAL_DATA(0x31, 0x91, 0x5d, 0xc1), \
  __MPI_LITERAL_DATA(0x30, 0x59, 0xec, 0xb0), \
  __MPI_LITERAL_DATA(0xc5, 0x6b, 0xb6, 0x7b), \
  __MPI_LITERAL_DATA(0x33, 0xba, 0xe1, 0x31), \
  __MPI_LITERAL_DATA(0x59, 0x36, 0x5a, 0x1e), \
  __MPI_LITERAL_DATA(0xc0, 0x63, 0xbe, 0xd8), \
  __MPI_LITERAL_DATA(0x2c, 0x86, 0xd0, 0x00), \
  __MPI_LITERAL_DATA(0x9e, 0xa8, 0x70, 0x45), \
  __MPI_LITERAL_DATA(0xc0, 0xa2, 0xba, 0xb1), \
  __MPI_LITERAL_DATA(0xc9, 0xb4, 0xd7, 0x6f), \
  __MPI_LITERAL_DATA(0xab, 0x64, 0x52, 0x37), \
  __MPI_LITERAL_DATA(0xe1, 0xe2, 0xf5, 0xd0), \
  __MPI_LITERAL_DATA(0x5a, 0xa2, 0x9d, 0x46), \
  __MPI_LITERAL_DATA(0xf6, 0xe0, 0x5a, 0x9c), \
  __MPI_LITERAL_DATA(0xcc, 0x58, 0xd6, 0xee), \
  __MPI_LITERAL_DATA(0x11, 0xb0, 0xce, 0xf2), \
  __MPI_LITERAL_DATA(0x63, 0x15, 0xd8, 0x9a), \
  __MPI_LITERAL_DATA(0x59, 0x69, 0x05, 0x1c), \
  __MPI_LITERAL_DATA(0x1d, 0x22, 0x46, 0x7f), \
  __MPI_LITERAL_DATA(0x06, 0xac, 0x10, 0xa6), \
  __MPI_LITERAL_DATA(0x40, 0x31, 0x25, 0xf5), \
  __MPI_LITERAL_DATA(0x3f, 0x09, 0xea, 0x52), \
  __MPI_LITERAL_DATA(0xfe, 0x18, 0x0b, 0xbb), \
  __MPI_LITERAL_DATA(0x56, 0x25, 0x40, 0x3f), \
  __MPI_LITERAL_DATA(0x86, 0x44, 0x6b, 0x8f), \
  __MPI_LITERAL_DATA(0x11, 0xce, 0xf4, 0xf8), \
  __MPI_LITERAL_DATA(0x56, 0x70, 0x7c, 0x78), \
  __MPI_LITERAL_DATA(0x28, 0xfe, 0xeb, 0xea), \
  __MPI_LITERAL_DATA(0xb7, 0x1d, 0x51, 0x92), \
  __MPI_LITERAL_DATA(0x0e, 0x23, 0xcd, 0xb1)

__MPI_LITERAL_BEGIN(static, _N)
  _N_LITERAL_DATA
__MPI_LITERAL_END(static, _N, 2048)

#define _E_LITERAL_DATA \
  __MPI_LITERAL_DATA(0x01, 0x00, 0x01, 0x00)

__MPI_LITERAL_BEGIN(static, _E)
  _E_LITERAL_DATA
__MPI_LITERAL_END(static, _E, 17)

#define _D_LITERAL_DATA \
  __MPI_LITERAL_DATA(0xc1, 0x0e, 0x67, 0x6c), \
  __MPI_LITERAL_DATA(0xde, 0xf0, 0x5e, 0x22), \
  __MPI_LITERAL_DATA(0x90, 0xbc, 0xcc, 0xa8), \
  __MPI_LITERAL_DATA(0x28, 0xa3, 0x0a, 0x05), \
  __MPI_LITERAL_DATA(0x5c, 0x2a, 0xb6, 0x7e), \
  __MPI_LITERAL_DATA(0xc2, 0xd5, 0xa3, 0xec), \
  __MPI_LITERAL_DATA(0x1b, 0x77, 0x80, 0x55), \
  __MPI_LITERAL_DATA(0xab, 0x78, 0xc6, 0x40), \
  __MPI_LITERAL_DATA(0xfa, 0xc3, 0x0b, 0x9a), \
  __MPI_LITERAL_DATA(0x66, 0x8b, 0x8a, 0xb2), \
  __MPI_LITERAL_DATA(0x0d, 0x88, 0x68, 0x4b), \
  __MPI_LITERAL_DATA(0x3a, 0xfe, 0xc0, 0x75), \
  __MPI_LITERAL_DATA(0x4b, 0x80, 0x92, 0xf5), \
  __MPI_LITERAL_DATA(0x4d, 0x14, 0xa2, 0xc9), \
  __MPI_LITERAL_DATA(0x98, 0x50, 0xe0, 0x47), \
  __MPI_LITERAL_DATA(0x33, 0x39, 0x79, 0xe3), \
  __MPI_LITERAL_DATA(0x7e, 0x67, 0x85, 0x4d), \
  __MPI_LITERAL_DATA(0x12, 0xde, 0xe9, 0xcd), \
  __MPI_LITERAL_DATA(0x54, 0xf3, 0x95, 0xb9), \
  __MPI_LITERAL_DATA(0xfa, 0xc0, 0x2a, 0x7a), \
  __MPI_LITERAL_DATA(0xec, 0x5b, 0x9f, 0x7e), \
  __MPI_LITERAL_DATA(0x35, 0xca, 0x31, 0xf1), \
  __MPI_LITERAL_DATA(0x7f, 0x4a, 0x7e, 0xc3), \
  __MPI_LITERAL_DATA(0xd9, 0xff, 0xc9, 0x1e), \
  __MPI_LITERAL_DATA(0xf7, 0x68, 0x5f, 0x06), \
  __MPI_LITERAL_DATA(0xcf, 0x2a, 0x1a, 0x25), \
  __MPI_LITERAL_DATA(0xf7, 0x22, 0x21, 0x6e), \
  __MPI_LITERAL_DATA(0x42, 0xdf, 0xe9, 0xf2), \
  __MPI_LITERAL_DATA(0x70, 0x10, 0xc3, 0x33), \
  __MPI_LITERAL_DATA(0xb1, 0xac, 0x2a, 0x0c), \
  __MPI_LITERAL_DATA(0x8e, 0x9c, 0x9e, 0x5b), \
  __MPI_LITERAL_DATA(0x30, 0xe0, 0x58, 0x17), \
  __MPI_LITERAL_DATA(0x6d, 0x51, 0xb9, 0x00), \
  __MPI_LITERAL_DATA(0xd9, 0x7e, 0xc9, 0xee), \
  __MPI_LITERAL_DATA(0xaa, 0x4b, 0xcc, 0xd6), \
  __MPI_LITERAL_DATA(0x47, 0x6c, 0xdc, 0xba), \
  __MPI_LITERAL_DATA(0x3c, 0x6e, 0x52, 0x88), \
  __MPI_LITERAL_DATA(0xf3, 0xc0, 0x76, 0xa8), \
  __MPI_LITERAL_DATA(0xe7, 0x23, 0x27, 0x61), \
  __MPI_LITERAL_DATA(0xb9, 0x20, 0x33, 0x4b), \
  __MPI_LITERAL_DATA(0x08, 0x8d, 0xef, 0x36), \
  __MPI_LITERAL_DATA(0x1c, 0x63, 0x9f, 0x9e), \
  __MPI_LITERAL_DATA(0xf5, 0xa2, 0xdf, 0x11), \
  __MPI_LITERAL_DATA(0x5d, 0xdc, 0xe1, 0x11), \
  __MPI_LITERAL_DATA(0x24, 0xbe, 0x2e, 0xad), \
  __MPI_LITERAL_DATA(0xd3, 0xb3, 0xc1, 0xda), \
  __MPI_LITERAL_DATA(0xf4, 0xc8, 0x22, 0xc0), \
  __MPI_LITERAL_DATA(0xe6, 0x17, 0x54, 0xf5), \
  __MPI_LITERAL_DATA(0xad, 0xcf, 0x94, 0xb9), \
  __MPI_LITERAL_DATA(0x92, 0xb9, 0xd5, 0x25), \
  __MPI_LITERAL_DATA(0x7a, 0xe7, 0xec, 0x7b), \
  __MPI_LITERAL_DATA(0x64, 0xe6, 0xeb, 0x0c), \
  __MPI_LITERAL_DATA(0x70, 0xbd, 0x63, 0xa4), \
  __MPI_LITERAL_DATA(0x44, 0xa9, 0x49, 0x03), \
  __MPI_LITERAL_DATA(0x74, 0x21, 0x0b, 0x79), \
  __MPI_LITERAL_DATA(0xed, 0xfa, 0x1a, 0xfc), \
  __MPI_LITERAL_DATA(0x38, 0x7f, 0x11, 0x3a), \
  __MPI_LITERAL_DATA(0x81, 0xd2, 0x08, 0xfe), \
  __MPI_LITERAL_DATA(0x8d, 0xcb, 0x5c, 0x5b), \
  __MPI_LITERAL_DATA(0xbf, 0x06, 0x99, 0x0d), \
  __MPI_LITERAL_DATA(0xd8, 0xf0, 0xbd, 0xba), \
  __MPI_LITERAL_DATA(0x4d, 0x93, 0x35, 0xf3), \
  __MPI_LITERAL_DATA(0xaa, 0x2f, 0xaa, 0x48), \
  __MPI_LITERAL_DATA(0xec, 0x64, 0xda, 0x10)

__MPI_LITERAL_BEGIN(static, _D)
  _D_LITERAL_DATA
__MPI_LITERAL_END(static, _D, 2045)

#define _P_LITERAL_DATA \
  __MPI_LITERAL_DATA(0xb7, 0x0e, 0x63, 0x39), \
  __MPI_LITERAL_DATA(0x88, 0x90, 0x46, 0xfe), \
  __MPI_LITERAL_DATA(0xa7, 0xb4, 0x02, 0x91), \
  __MPI_LITERAL_DATA(0xe0, 0x82, 0xd0, 0x45), \
  __MPI_LITERAL_DATA(0x6f, 0x66, 0x49, 0x71), \
  __MPI_LITERAL_DATA(0x76, 0xe6, 0xc2, 0xeb), \
  __MPI_LITERAL_DATA(0x36, 0x00, 0xaf, 0xf8), \
  __MPI_LITERAL_DATA(0x91, 0xc7, 0x02, 0x76), \
  __MPI_LITERAL_DATA(0xa4, 0x72, 0xbb, 0x3c), \
  __MPI_LITERAL_DATA(0xd8, 0xc8, 0x48, 0x9e), \
  __MPI_LITERAL_DATA(0x09, 0xf5, 0xd7, 0x8f), \
  __MPI_LITERAL_DATA(0xae, 0x5e, 0x10, 0xe8), \
  __MPI_LITERAL_DATA(0xdb, 0x6d, 0xd4, 0x90), \
  __MPI_LITERAL_DATA(0xd5, 0x0d, 0x20, 0xab), \
  __MPI_LITERAL_DATA(0x0a, 0xc3, 0x18, 0xb1), \
  __MPI_LITERAL_DATA(0xf6, 0x01, 0x67, 0xa6), \
  __MPI_LITERAL_DATA(0xe9, 0x90, 0x4e, 0x7f), \
  __MPI_LITERAL_DATA(0xfa, 0x2b, 0xa6, 0x87), \
  __MPI_LITERAL_DATA(0x26, 0xef, 0x83, 0xa0), \
  __MPI_LITERAL_DATA(0x6b, 0x73, 0xa8, 0xd4), \
  __MPI_LITERAL_DATA(0x8f, 0xf8, 0x9f, 0x78), \
  __MPI_LITERAL_DATA(0x20, 0xee, 0x7a, 0x5c), \
  __MPI_LITERAL_DATA(0x69, 0x53, 0xa2, 0xde), \
  __MPI_LITERAL_DATA(0x66, 0xae, 0x8c, 0x07), \
  __MPI_LITERAL_DATA(0x1a, 0x15, 0x97, 0x81), \
  __MPI_LITERAL_DATA(0xe2, 0x1d, 0x42, 0x41), \
  __MPI_LITERAL_DATA(0xe9, 0xd2, 0x37, 0x0b), \
  __MPI_LITERAL_DATA(0x8a, 0xd7, 0xc1, 0x5a), \
  __MPI_LITERAL_DATA(0x6c, 0x51, 0xc6, 0xbf), \
  __MPI_LITERAL_DATA(0x87, 0xab, 0x25, 0xeb), \
  __MPI_LITERAL_DATA(0x62, 0x57, 0x56, 0xc4), \
  __MPI_LITERAL_DATA(0x22, 0xa8, 0x49, 0xd2)

__MPI_LITERAL_BEGIN(static, _P)
  _P_LITERAL_DATA
__MPI_LITERAL_END(static, _P, 1024)

#define _Q_LITERAL_DATA \
  __MPI_LITERAL_DATA(0x89, 0x0c, 0x90, 0x27), \
  __MPI_LITERAL_DATA(0x48, 0xe4, 0xea, 0xfd), \
  __MPI_LITERAL_DATA(0x1d, 0xfc, 0x5a, 0x33), \
  __MPI_LITERAL_DATA(0x08, 0x07, 0x44, 0x5a), \
  __MPI_LITERAL_DATA(0xb9, 0xfa, 0x0c, 0x4a), \
  __MPI_LITERAL_DATA(0x63, 0x4d, 0x9c, 0x08), \
  __MPI_LITERAL_DATA(0xfb, 0x27, 0x44, 0xf2), \
  __MPI_LITERAL_DATA(0xa9, 0x72, 0x3d, 0x6f), \
  __MPI_LITERAL_DATA(0x37, 0xb0, 0xbf, 0xe6), \
  __MPI_LITERAL_DATA(0x18, 0xe9, 0x29, 0xa5), \
  __MPI_LITERAL_DATA(0xdf, 0xca, 0x4e, 0xfb), \
  __MPI_LITERAL_DATA(0x9b, 0xc0, 0xfe, 0x84), \
  __MPI_LITERAL_DATA(0xb2, 0xdd, 0xdc, 0xdf), \
  __MPI_LITERAL_DATA(0x19, 0x6e, 0x50, 0x23), \
  __MPI_LITERAL_DATA(0xd3, 0xcd, 0x14, 0x67), \
  __MPI_LITERAL_DATA(0x1c, 0xc5, 0x8a, 0x7b), \
  __MPI_LITERAL_DATA(0x90, 0xfd, 0x79, 0x60), \
  __MPI_LITERAL_DATA(0xb1, 0x75, 0x54, 0x41), \
  __MPI_LITERAL_DATA(0x98, 0x44, 0xc5, 0x66), \
  __MPI_LITERAL_DATA(0x6c, 0x1f, 0xb7, 0x59), \
  __MPI_LITERAL_DATA(0x35, 0x9e, 0xc3, 0xca), \
  __MPI_LITERAL_DATA(0xe4, 0xd4, 0xc5, 0x65), \
  __MPI_LITERAL_DATA(0xfb, 0x4c, 0x7e, 0xcf), \
  __MPI_LITERAL_DATA(0x15, 0x82, 0x06, 0xc8), \
  __MPI_LITERAL_DATA(0xe8, 0xb9, 0x12, 0x65), \
  __MPI_LITERAL_DATA(0xb8, 0x38, 0xe7, 0x57), \
  __MPI_LITERAL_DATA(0x48, 0x09, 0xf0, 0x9e), \
  __MPI_LITERAL_DATA(0x39, 0x6e, 0x81, 0xad), \
  __MPI_LITERAL_DATA(0xf7, 0x80, 0x79, 0xa1), \
  __MPI_LITERAL_DATA(0xbc, 0xfd, 0x32, 0x2a), \
  __MPI_LITERAL_DATA(0x19, 0xc2, 0x62, 0x81), \
  __MPI_LITERAL_DATA(0x41, 0xa3, 0x73, 0xd8)

__MPI_LITERAL_BEGIN(static, _Q)
  _Q_LITERAL_DATA
__MPI_LITERAL_END(static, _Q, 1024)

#define _DP_LITERAL_DATA \
  __MPI_LITERAL_DATA(0x4b, 0xcf, 0xdb, 0xdc), \
  __MPI_LITERAL_DATA(0x52, 0x33, 0x3d, 0x8b), \
  __MPI_LITERAL_DATA(0x66, 0xc6, 0x20, 0x55), \
  __MPI_LITERAL_DATA(0x4e, 0x17, 0x39, 0xb4), \
  __MPI_LITERAL_DATA(0x56, 0xdd, 0x9b, 0x3b), \
  __MPI_LITERAL_DATA(0xe3, 0xeb, 0x1c, 0xc6), \
  __MPI_LITERAL_DATA(0xab, 0x80, 0xdb, 0x79), \
  __MPI_LITERAL_DATA(0x57, 0x01, 0xe1, 0x47), \
  __MPI_LITERAL_DATA(0x1b, 0xce, 0x75, 0x35), \
  __MPI_LITERAL_DATA(0xb7, 0xb5, 0x7d, 0x9a), \
  __MPI_LITERAL_DATA(0xe6, 0xf1, 0x6b, 0xeb), \
  __MPI_LITERAL_DATA(0x0d, 0x3a, 0x74, 0x0b), \
  __MPI_LITERAL_DATA(0x08, 0xdb, 0xb5, 0x84), \
  __MPI_LITERAL_DATA(0x96, 0x7d, 0xf5, 0x84), \
  __MPI_LITERAL_DATA(0x1e, 0x43, 0x8a, 0x84), \
  __MPI_LITERAL_DATA(0x84, 0x46, 0x05, 0xe0), \
  __MPI_LITERAL_DATA(0x84, 0x78, 0x63, 0xad), \
  __MPI_LITERAL_DATA(0x72, 0xb6, 0x5d, 0xfb), \
  __MPI_LITERAL_DATA(0x8a, 0x29, 0x4a, 0x99), \
  __MPI_LITERAL_DATA(0xec, 0x50, 0x7c, 0x54), \
  __MPI_LITERAL_DATA(0x1e, 0xd8, 0xe7, 0xc7), \
  __MPI_LITERAL_DATA(0x2d, 0x61, 0xad, 0xbf), \
  __MPI_LITERAL_DATA(0x4e, 0x0d, 0x7e, 0x80), \
  __MPI_LITERAL_DATA(0x21, 0x7b, 0xad, 0x85), \
  __MPI_LITERAL_DATA(0xae, 0x1e, 0x95, 0xaf), \
  __MPI_LITERAL_DATA(0x9a, 0x0c, 0xac, 0x09), \
  __MPI_LITERAL_DATA(0xe9, 0xa8, 0x4f, 0x77), \
  __MPI_LITERAL_DATA(0x3c, 0x1e, 0x05, 0x19), \
  __MPI_LITERAL_DATA(0x40, 0xab, 0x71, 0x93), \
  __MPI_LITERAL_DATA(0x8d, 0xe9, 0xd1, 0x65), \
  __MPI_LITERAL_DATA(0x2f, 0x05, 0xd7, 0x8a), \
  __MPI_LITERAL_DATA(0x0f, 0x29, 0xd1, 0xcf)

__MPI_LITERAL_BEGIN(static, _DP)
  _DP_LITERAL_DATA
__MPI_LITERAL_END(static, _DP, 1024)

#define _DQ_LITERAL_DATA \
  __MPI_LITERAL_DATA(0x09, 0x75, 0x79, 0x77), \
  __MPI_LITERAL_DATA(0x06, 0x77, 0x27, 0x47), \
  __MPI_LITERAL_DATA(0xda, 0x0c, 0x7d, 0x14), \
  __MPI_LITERAL_DATA(0x01, 0x34, 0x37, 0xa1), \
  __MPI_LITERAL_DATA(0x20, 0xaf, 0xb4, 0x9a), \
  __MPI_LITERAL_DATA(0x77, 0x2b, 0xac, 0x60), \
  __MPI_LITERAL_DATA(0x34, 0x8b, 0x6d, 0x81), \
  __MPI_LITERAL_DATA(0xc4, 0x16, 0x9a, 0xdd), \
  __MPI_LITERAL_DATA(0x1a, 0xac, 0x94, 0x5a), \
  __MPI_LITERAL_DATA(0x01, 0xdc, 0xf2, 0x0d), \
  __MPI_LITERAL_DATA(0x53, 0x94, 0x45, 0x35), \
  __MPI_LITERAL_DATA(0x9d, 0x40, 0xe2, 0x63), \
  __MPI_LITERAL_DATA(0xbb, 0x7b, 0x3d, 0xe5), \
  __MPI_LITERAL_DATA(0xd8, 0x13, 0x6f, 0xed), \
  __MPI_LITERAL_DATA(0x3e, 0xfb, 0x4f, 0xb1), \
  __MPI_LITERAL_DATA(0xd2, 0x77, 0x43, 0x00), \
  __MPI_LITERAL_DATA(0x15, 0x8c, 0xb2, 0x21), \
  __MPI_LITERAL_DATA(0x09, 0xbf, 0xbf, 0x17), \
  __MPI_LITERAL_DATA(0xa1, 0x94, 0xd9, 0x43), \
  __MPI_LITERAL_DATA(0x4d, 0x24, 0xcf, 0x5a), \
  __MPI_LITERAL_DATA(0x3e, 0x00, 0x8a, 0xe9), \
  __MPI_LITERAL_DATA(0xe9, 0x4a, 0x23, 0x06), \
  __MPI_LITERAL_DATA(0x8f, 0xb3, 0x33, 0x75), \
  __MPI_LITERAL_DATA(0x03, 0xf8, 0x67, 0x84), \
  __MPI_LITERAL_DATA(0xb3, 0xb2, 0xbc, 0xdc), \
  __MPI_LITERAL_DATA(0xd7, 0xa7, 0x92, 0x90), \
  __MPI_LITERAL_DATA(0x03, 0x01, 0xc6, 0x55), \
  __MPI_LITERAL_DATA(0x8a, 0xe7, 0x1c, 0xfb), \
  __MPI_LITERAL_DATA(0xb4, 0xa6, 0x48, 0xe1), \
  __MPI_LITERAL_DATA(0x25, 0xc2, 0x14, 0xa5), \
  __MPI_LITERAL_DATA(0xcf, 0x3a, 0xbf, 0x7d), \
  __MPI_LITERAL_DATA(0xec, 0xc8, 0x5b, 0xce)

__MPI_LITERAL_BEGIN(static, _DQ)
  _DQ_LITERAL_DATA
__MPI_LITERAL_END(static, _DQ, 1024)

#define _QINV_LITERAL_DATA \
  __MPI_LITERAL_DATA(0x8e, 0xa6, 0xac, 0x41), \
  __MPI_LITERAL_DATA(0x06, 0xdc, 0xea, 0xba), \
  __MPI_LITERAL_DATA(0x6d, 0xbf, 0xc2, 0x82), \
  __MPI_LITERAL_DATA(0x66, 0x2c, 0xbe, 0xbc), \
  __MPI_LITERAL_DATA(0x74, 0xa1, 0xe3, 0x3b), \
  __MPI_LITERAL_DATA(0x31, 0x10, 0x85, 0xe8), \
  __MPI_LITERAL_DATA(0x87, 0x19, 0x4d, 0xa1), \
  __MPI_LITERAL_DATA(0x47, 0x95, 0xab, 0x2a), \
  __MPI_LITERAL_DATA(0x8b, 0x2f, 0xbd, 0x0e), \
  __MPI_LITERAL_DATA(0xab, 0xf7, 0xcd, 0x2f), \
  __MPI_LITERAL_DATA(0xdb, 0x00, 0x24, 0x1c), \
  __MPI_LITERAL_DATA(0x9f, 0x56, 0xa7, 0xff), \
  __MPI_LITERAL_DATA(0xfc, 0xb3, 0x67, 0xd9), \
  __MPI_LITERAL_DATA(0x24, 0xa5, 0x3c, 0x22), \
  __MPI_LITERAL_DATA(0xc3, 0xc1, 0xe9, 0xee), \
  __MPI_LITERAL_DATA(0xda, 0x45, 0x7b, 0xb6), \
  __MPI_LITERAL_DATA(0xa9, 0x0c, 0x26, 0xeb), \
  __MPI_LITERAL_DATA(0x29, 0xd3, 0xea, 0x41), \
  __MPI_LITERAL_DATA(0x7c, 0x5e, 0x90, 0x45), \
  __MPI_LITERAL_DATA(0x30, 0xd0, 0xdb, 0x28), \
  __MPI_LITERAL_DATA(0x71, 0x69, 0x00, 0xd1), \
  __MPI_LITERAL_DATA(0xab, 0x71, 0x55, 0xcf), \
  __MPI_LITERAL_DATA(0x6a, 0xc6, 0xea, 0x7f), \
  __MPI_LITERAL_DATA(0x3b, 0x00, 0xd8, 0xc0), \
  __MPI_LITERAL_DATA(0xbe, 0x0d, 0x71, 0x4c), \
  __MPI_LITERAL_DATA(0xf4, 0x50, 0x25, 0xfb), \
  __MPI_LITERAL_DATA(0xb7, 0x96, 0x66, 0x13), \
  __MPI_LITERAL_DATA(0x4f, 0x12, 0x15, 0x34), \
  __MPI_LITERAL_DATA(0xd7, 0x18, 0xda, 0x0f), \
  __MPI_LITERAL_DATA(0x4d, 0x2d, 0xdf, 0x47), \
  __MPI_LITERAL_DATA(0x8d, 0x6e, 0x1c, 0xcd), \
  __MPI_LITERAL_DATA(0x86, 0xe1, 0xa5, 0xaf)

__MPI_LITERAL_BEGIN(static, _QINV)
  _QINV_LITERAL_DATA
__MPI_LITERAL_END(static, _QINV, 1024)

#endif  // SECURE_TRIAL

#endif  // SECURE_TRIAL_KEY_H

/*************************** End of file ****************************/
