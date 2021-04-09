/**
 * \file cy_p64_psacrypto_sizes.h
 *
 * \brief PSA cryptography module: Mbed TLS buffer size macros
 *
 * \note This file may not be included directly. Applications must
 * include cy_p64_psacrypto.h.
 *
 * This file contains the definitions of macros that are useful to
 * compute buffer sizes. The signatures and semantics of these macros
 * are standardized, but the definitions are not, because they depend on
 * the available algorithms and, in some cases, on permitted tolerances
 * on buffer sizes.
 *
 * In implementations with isolation between the application and the
 * cryptography module, ensure that the definitions exposed to 
 * applications match what the module implements.
 *
 * Macros that compute sizes whose values do not depend on the
 * implementation are in cy_p64_psacrypto.h.
 *
 * Copyright 2019-2021 Cypress Semiconductor Corporation (an Infineon company)
 *
 */
/*
 *  Copyright (C) 2018, ARM Limited, All Rights Reserved
 *  SPDX-License-Identifier: Apache-2.0
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may
 *  not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  (http)www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  This file is part of mbed TLS ((http)tls.mbed.org)
 */

#ifndef CY_P64_PSA_CRYPTO_SIZES_H
#define CY_P64_PSA_CRYPTO_SIZES_H

#define CY_P64_PSA_BITS_TO_BYTES(bits) (((bits) + 7) / 8)
#define CY_P64_PSA_BYTES_TO_BITS(bytes) ((bytes) * 8)

/** \addtogroup constants_sizes
 * \{
 */
/** The size of the output of psa_hash_finish(), in bytes.
 *
 * This is also the hash size that cy_p64_psa_hash_verify() expects.
 *
 * \param alg   A hash algorithm (\c CY_P64_ALG_XXX value such that
 *              #CY_P64_PSA_ALG_IS_HASH(\p alg) is true), or an HMAC algorithm
 *              (#CY_P64_PSA_ALG_HMAC(\c hash_alg) where \c hash_alg is a
 *              hash algorithm).
 *
 * \return The hash size for the specified hash algorithm.
 *         If the hash algorithm is not recognized, return 0.
 *         An implementation may return either 0 or the correct size
 *         for a hash algorithm that it recognizes, but does not support.
 */
#define CY_P64_PSA_HASH_SIZE(alg)                                      \
    (                                                           \
        CY_P64_ALG_HMAC_GET_HASH(alg) == CY_P64_ALG_SHA_224 ? 28 :        \
        CY_P64_ALG_HMAC_GET_HASH(alg) == CY_P64_ALG_SHA_256 ? 32 :        \
        0)

/** \def CY_P64_PSA_HASH_MAX_SIZE
 *
 * The maximum size of a hash.
 *
 * This macro must expand to a compile-time constant integer. This value
 * should be the maximum size of a hash supported by the implementation,
 * in bytes, and must be no smaller than this maximum.
 */
#define CY_P64_PSA_HASH_MAX_SIZE                 (32)
/** \cond INTERNAL */
#define CY_P64_PSA_HMAC_MAX_HASH_BLOCK_SIZE      (64)
/** \endcond */

/**
 * \brief Maximum size of the export encoding of an ECC public key.
 *
 * The representation of an ECC public key is:
 *      * The byte 0x04;
 *      * `x_P` as a `ceiling(m/8)`-byte string, big-endian;
 *      * `y_P` as a `ceiling(m/8)`-byte string, big-endian;
 *      where m is the bit size associated with the curve.
 *  1 byte + 2 * point size.
 */
#define CY_P64_PSA_KEY_EXPORT_ECC_PUBLIC_KEY_MAX_SIZE(key_bits)        \
    (2 * CY_P64_PSA_BITS_TO_BYTES(key_bits) + 1)

/**
 * \brief The maximum size of the export encoding of an ECC key pair.
 *
 * The ECC key pair is represented by the secret value.
 */
#define CY_P64_PSA_KEY_EXPORT_ECC_KEY_PAIR_MAX_SIZE(key_bits)   \
    (CY_P64_PSA_BITS_TO_BYTES(key_bits))

/**
 * \brief The sufficient output buffer size for psa_export_key() or psa_export_public_key().
 *
 * This macro returns a compile-time constant if its arguments are
 * compile-time constants.
 *
 * \warning This function may call its arguments multiple times or
 *          zero times, so you should not pass arguments that contain
 *          side effects.
 *
 * The following code illustrates how to allocate enough memory to export
 * a key by querying the key type and size at runtime.
 * \code{c}
 * psa_key_attributes_t attributes = CY_P64_PSA_KEY_ATTRIBUTES_INIT;
 * psa_status_t status;
 * status = psa_get_key_attributes(key, &attributes);
 * if (status != CY_P64_PSA_SUCCESS) handle_error(...);
 * psa_key_type_t key_type = psa_get_key_type(&attributes);
 * size_t key_bits = psa_get_key_bits(&attributes);
 * size_t buffer_size = CY_P64_PSA_KEY_EXPORT_MAX_SIZE(key_type, key_bits);
 * psa_reset_key_attributes(&attributes);
 * uint8_t *buffer = malloc(buffer_size);
 * if (buffer == NULL) handle_error(...);
 * size_t buffer_length;
 * status = psa_export_key(key, buffer, buffer_size, &buffer_length);
 * if (status != CY_P64_PSA_SUCCESS) handle_error(...);
 * \endcode
 *
 * For psa_export_public_key(), calculate the buffer size from the
 * public key type. You can use the macro #CY_P64_PSA_KEY_TYPE_PUBLIC_KEY_OF_KEY_PAIR
 * to convert a key pair type to the corresponding public key type.
 * \code{c}
 * psa_key_attributes_t attributes = CY_P64_PSA_KEY_ATTRIBUTES_INIT;
 * psa_status_t status;
 * status = psa_get_key_attributes(key, &attributes);
 * if (status != CY_P64_PSA_SUCCESS) handle_error(...);
 * psa_key_type_t key_type = psa_get_key_type(&attributes);
 * psa_key_type_t public_key_type = CY_P64_PSA_KEY_TYPE_PUBLIC_KEY_OF_KEY_PAIR(key_type);
 * size_t key_bits = psa_get_key_bits(&attributes);
 * size_t buffer_size = CY_P64_PSA_KEY_EXPORT_MAX_SIZE(public_key_type, key_bits);
 * psa_reset_key_attributes(&attributes);
 * uint8_t *buffer = malloc(buffer_size);
 * if (buffer == NULL) handle_error(...);
 * size_t buffer_length;
 * status = psa_export_public_key(key, buffer, buffer_size, &buffer_length);
 * if (status != CY_P64_PSA_SUCCESS) handle_error(...);
 * \endcode
 *
 * \param key_type  A supported key type.
 * \param key_bits  The size of the key in bits.
 *
 * \return If the parameters are valid and supported, return
 *         a buffer size in bytes that guarantees that
 *         psa_sign_hash() will not fail with
 *         #CY_P64_PSA_ERROR_BUFFER_TOO_SMALL.
 *         If the parameters are a valid combination that is not supported
 *         by the implementation, this macro shall return either a
 *         sensible size or 0.
 *         If the parameters are not valid, the
 *         return value is unspecified.
 */
#define CY_P64_PSA_KEY_EXPORT_MAX_SIZE(key_type, key_bits)                     \
    (CY_P64_PSA_KEY_TYPE_IS_UNSTRUCTURED(key_type) ? CY_P64_PSA_BITS_TO_BYTES(key_bits) : \
     CY_P64_PSA_KEY_TYPE_IS_ECC_KEY_PAIR(key_type) ? CY_P64_PSA_KEY_EXPORT_ECC_KEY_PAIR_MAX_SIZE(key_bits) : \
     CY_P64_PSA_KEY_TYPE_IS_ECC_PUBLIC_KEY(key_type) ? CY_P64_PSA_KEY_EXPORT_ECC_PUBLIC_KEY_MAX_SIZE(key_bits) : \
     0)

/** \} */

#endif /* CY_P64_PSA_CRYPTO_SIZES_H */
