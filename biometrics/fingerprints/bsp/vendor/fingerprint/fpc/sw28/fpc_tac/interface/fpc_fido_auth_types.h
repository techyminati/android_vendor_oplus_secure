/*
* Copyright (c) 2018 Fingerprint Cards AB <tech@fingerprints.com>
*
* All rights are reserved.
* Proprietary and confidential.
* Unauthorized copying of this file, via any medium is strictly prohibited.
* Any use is subject to an appropriate license granted by Fingerprint Cards AB.
*/

#ifndef FPC_FIDO_AUTH_TYPES_H
#define FPC_FIDO_AUTH_TYPES_H

/*
 * . used as 'nonce', for FPC_CONFIG_FIDO_AUTH_VER_GENERIC
 *   . QC_AUTH_NONCE = 32
 * . used as 'challenge', for FPC_CONFIG_FIDO_AUTH_VER_GMRZ
 *   . the length of challenge is 32
 * . the maximum of the above is 32
 */
#define FPC_FIDO_AUTH_INPUT_1_MAX_LEN 32

/*
 * . not used, for FPC_CONFIG_FIDO_AUTH_VER_GENERIC
 * . used as 'fido aaid', for FPC_CONFIG_FIDO_AUTH_VER_GMRZ
 *   . the length of aaid is 32
 * . the maximum of the above is 32
 */
#define FPC_FIDO_AUTH_INPUT_2_MAX_LEN 32

/*
 * . QC_SEC_APP_NAME_LEN = 32
 * . the maximum of the above is 32
 */
#define FPC_FIDO_SEC_APP_NAME_MAX_LEN 32

/*
 * . QSEE4: QC_UVT_MAX_LEN = 5120
 * . QSEE3: QC_USER_VERIFICATION_TOKEN_LEN = 4272
 */
#define FPC_FIDO_UVT_MAX_LEN 5120

#define FPC_FIDO_AUTH_NONCE_MAX_SIZE FPC_FIDO_AUTH_INPUT_1_MAX_LEN

#endif // FPC_FIDO_AUTH_TYPES_H
