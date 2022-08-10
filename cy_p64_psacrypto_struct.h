/**
 * \file cy_p64_psacrypto_struct.h
 *
 * \brief PSA cryptography module: Mbed TLS structured type implementations
 *
 * \note This file may not be included directly. Applications must
 * include cy_p64_psacrypto.h.
 *
 * This file contains the definitions of some data structures with
 * implementation-specific definitions.
 *
 * In implementations with isolation between the application and the
 * cryptography module, it is expected that the front-end and the back-end
 * would have different versions of this file.
 *
 * Copyright 2019-2022 Cypress Semiconductor Corporation (an Infineon company)
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

#ifndef CY_P64_PSA_CRYPTO_STRUCT_H
#define CY_P64_PSA_CRYPTO_STRUCT_H

#include <stdint.h>

/** \cond INTERNAL */
struct cy_p64_psa_hash_operation_s
{
    uint32_t operationHandle;   /*!< Frontend context handle associated to a multipart operation */
};
/** \endcond */

/** \addtogroup hash_operations
 * \{
 */

/** the initial value of the Hash operation context */
#define CY_P64_PSA_HASH_OPERATION_INIT {0}

/** Initialize the Hash operation context */
static inline struct cy_p64_psa_hash_operation_s cy_p64_psa_hash_operation_init( void )
{
    const struct cy_p64_psa_hash_operation_s v = CY_P64_PSA_HASH_OPERATION_INIT;
    return( v );
}
/** \} */

/** \cond INTERNAL */
struct cy_p64_psa_key_policy_s
{
    cy_p64_psa_key_usage_t usage;
    cy_p64_psa_algorithm_t alg;
    cy_p64_psa_algorithm_t alg2;
};
/** \endcond */

/** \addtogroup policy
* \{
*/
/** The initial value of the key policy */
#define CY_P64_PSA_KEY_POLICY_INIT {0, 0, 0}

/**
* \brief Sets key policy strcuture with initial value.
* \retval                   /p cy_p64_psa_key_policy_s structure
*/
static inline struct cy_p64_psa_key_policy_s cy_p64_psa_key_policy_init( void )
{
    const struct cy_p64_psa_key_policy_s v = CY_P64_PSA_KEY_POLICY_INIT;
    return( v );
}
/** \} */

/** \cond INTERNAL */
struct cy_p64_psa_cipher_operation_s
{
     uint32_t operationHandle;   /*!< Frontend context handle associated to a multipart operation */
};
/** \endcond */

/** \addtogroup cipher_operations
 * \{
 */

/** The initial value of the Cipher operation context */
#define CY_P64_PSA_CIPHER_OPERATION_INIT {0}

/** Initialize the Cipher operation context */
static inline struct cy_p64_psa_cipher_operation_s cy_p64_psa_cipher_operation_init( void )
{
    const struct cy_p64_psa_cipher_operation_s v = CY_P64_PSA_CIPHER_OPERATION_INIT;
    return( v );
}
/** \} */

/** \cond INTERNAL */
struct cy_p64_psa_mac_operation_s
{
    uint32_t operationHandle;   /*!< Frontend context handle associated to a multipart operation */
};
/** \endcond */

/** \addtogroup mac_operations
 * \{
 */

/** The initial value of the MAC operation context */
#define CY_P64_PSA_MAC_OPERATION_INIT {0}

/** Initialize the MAC operation context */
static inline struct cy_p64_psa_mac_operation_s cy_p64_psa_mac_operation_init( void )
{
    const struct cy_p64_psa_mac_operation_s v = CY_P64_PSA_MAC_OPERATION_INIT;
    return( v );
}
/** \} */


/** \addtogroup key_derivation
 * \{
 */

/** This only zeroes out the first byte in the union, the rest is unspecified. */
#define CY_P64_PSA_KEY_DERIVATION_OPERATION_INIT {0}

/** \cond INTERNAL */
struct cy_p64_psa_key_derivation_s
{
    uint32_t operationHandle;   /*!< Frontend context handle associated to a multipart operation */
};

static inline struct cy_p64_psa_key_derivation_s cy_p64_psa_key_derivation_operation_init( void )
{
    const struct cy_p64_psa_key_derivation_s v = CY_P64_PSA_KEY_DERIVATION_OPERATION_INIT;
    return( v );
}
/** \endcond */
/** \} */

/** \addtogroup attributes
 * \{
 */

/**
 * The type used internally for key sizes.
 * Public interfaces use size_t, but internally we use a smaller type.
 */
typedef uint16_t cy_p64_psa_key_bits_t;

/**
 * The maximum value of the type used to represent bit-sizes.
 * This is used to mark an invalid key size.
 */
#define CY_P64_PSA_KEY_BITS_TOO_LARGE ( (cy_p64_psa_key_bits_t) ( -1 ) )

/**
 * \brief The maximum size of a key in bits.
 * Currently defined as the maximum that can be represented, rounded down
 * to a whole number of bytes.
 * This is an uncast value so that it can be used in preprocessor
 * conditionals.
 */
#define CY_P64_PSA_MAX_KEY_BITS     0xfff8U

/**
 * \brief A mask of flags that can be stored in key attributes.
 *
 * This type is also used internally to store flags in slots. Internal
 * flags are defined in library/psa_crypto_core.h. Internal flags may have
 * the same value as external flags if they are properly handled during
 * key creation and in psa_get_key_attributes.
 */
typedef uint16_t cy_p64_psa_key_attributes_flag_t;

/** \cond INTERNAL */
typedef struct
{
    cy_p64_psa_key_type_t type;
    cy_p64_psa_key_bits_t bits;
    cy_p64_psa_key_lifetime_t lifetime;
    cy_p64_psa_key_id_t id;
    cy_p64_psa_key_policy_t policy;
    cy_p64_psa_key_attributes_flag_t flags;
} cy_p64_psa_core_key_attributes_t;

struct cy_p64_psa_key_attributes_s
{
    cy_p64_psa_core_key_attributes_t core;
    void *domain_parameters;
    size_t domain_parameters_size;
};

#define CY_P64_PSA_KEY_ID_INIT              (0U)
/** \endcond */

/** The initial value of the core key attributes */
#define CY_P64_PSA_CORE_KEY_ATTRIBUTES_INIT { CY_P64_PSA_KEY_TYPE_NONE, 0, \
                                              CY_P64_PSA_KEY_LIFETIME_VOLATILE, \
                                              CY_P64_PSA_KEY_ID_INIT, \
                                              CY_P64_PSA_KEY_POLICY_INIT, 0 }


/** The initial value of the key attributes */
#define CY_P64_PSA_KEY_ATTRIBUTES_INIT { CY_P64_PSA_CORE_KEY_ATTRIBUTES_INIT, NULL, 0 }

/**
* \brief Sets key attributes strcuture with initial value.
* \retval                   /p cy_p64_psa_key_attributes_s structure
*/
static inline struct cy_p64_psa_key_attributes_s cy_p64_psa_key_attributes_init( void )
{
    const struct cy_p64_psa_key_attributes_s v = CY_P64_PSA_KEY_ATTRIBUTES_INIT;
    return( v );
}

/**
* \brief Sets key ID in key attributes strcuture.
* \param[in] attributes     Key attributes structure
* \param[in] id             Key ID
*/
static inline void cy_p64_psa_set_key_id(cy_p64_psa_key_attributes_t *attributes,
                                  cy_p64_psa_key_id_t id)
{
    attributes->core.id = id;
    if( attributes->core.lifetime == CY_P64_PSA_KEY_LIFETIME_VOLATILE )
    {
        attributes->core.lifetime = CY_P64_PSA_KEY_LIFETIME_PERSISTENT;
    }
}

/**
* \brief Gets key ID from key attributes strcuture.
* \param[in] attributes     Key attributes structure
* \retval                   Key ID
*/
static inline cy_p64_psa_key_id_t cy_p64_psa_get_key_id(
    const cy_p64_psa_key_attributes_t *attributes)
{
    return( attributes->core.id );
}

/**
* \brief Sets key lifetime in key attributes strcuture.
* \param[in] attributes     Key attributes structure
* \param[in] lifetime       Key lifetime
*/
static inline void cy_p64_psa_set_key_lifetime(cy_p64_psa_key_attributes_t *attributes,
                                        cy_p64_psa_key_lifetime_t lifetime)
{
    attributes->core.lifetime = lifetime;
    if( lifetime == CY_P64_PSA_KEY_LIFETIME_VOLATILE )
    {
        attributes->core.id = 0;
    }
}

/**
* \brief Gets key lifetime from key attributes strcuture.
* \param[in] attributes     Key attributes structure
* \retval                   Key lifetime
*/
static inline cy_p64_psa_key_lifetime_t cy_p64_psa_get_key_lifetime(
    const cy_p64_psa_key_attributes_t *attributes)
{
    return( attributes->core.lifetime );
}

/**
* \brief Sets key usage flags in key attributes strcuture.
* \param[in] attributes     Key attributes structure
* \param[in] usage_flags    Key usage flags
*/
static inline void cy_p64_psa_set_key_usage_flags(cy_p64_psa_key_attributes_t *attributes,
                                           cy_p64_psa_key_usage_t usage_flags)
{
    attributes->core.policy.usage = usage_flags;
}

/**
* \brief Gets key usage flags from key attributes strcuture.
* \param[in] attributes     Key attributes structure
* \retval                   Key usage flags
*/
static inline cy_p64_psa_key_usage_t cy_p64_psa_get_key_usage_flags(
    const cy_p64_psa_key_attributes_t *attributes)
{
    return( attributes->core.policy.usage );
}

/**
* \brief Sets key algorithm in key attributes strcuture.
* \param[in] attributes     Key attributes structure
* \param[in] alg            Key algorithm
*/
static inline void cy_p64_psa_set_key_algorithm(cy_p64_psa_key_attributes_t *attributes,
                                         cy_p64_psa_algorithm_t alg)
{
    attributes->core.policy.alg = alg;
}

/**
* \brief Gets key algorithm from key attributes strcuture.
* \param[in] attributes     Key attributes structure
* \retval                   Key algorithm
*/
static inline cy_p64_psa_algorithm_t cy_p64_psa_get_key_algorithm(
    const cy_p64_psa_key_attributes_t *attributes)
{
    return( attributes->core.policy.alg );
}

 /**
 * \brief Sets key type in key attributes strcuture.
 * \param[in] attributes     Key attributes structure
 * \param[in] type           Key type
 */
static inline void cy_p64_psa_set_key_type(cy_p64_psa_key_attributes_t *attributes,
                                    cy_p64_psa_key_type_t type)
{
    /* Common case: quick path */
    attributes->core.type = type;
}

/**
* \brief Gets key type from key attributes strcuture
* \param[in] attributes     Key attributes structure
* \retval                   Key type
*/
static inline cy_p64_psa_key_type_t cy_p64_psa_get_key_type(
    const cy_p64_psa_key_attributes_t *attributes)
{
    return( attributes->core.type );
}

/**
* \brief Sets key length in bits in key attributes strcuture
* \param[in] attributes     Key attributes structure
* \param[in] bits           Key length in bits
*/
static inline void cy_p64_psa_set_key_bits(cy_p64_psa_key_attributes_t *attributes,
                                    size_t bits)
{
    if( bits > CY_P64_PSA_MAX_KEY_BITS )
    {
        attributes->core.bits = CY_P64_PSA_KEY_BITS_TOO_LARGE;
    }
    else
    {
        attributes->core.bits = (cy_p64_psa_key_bits_t) bits;
    }
}

/**
* \brief Gets key length in bits from key attributes strcuture
* \param[in] attributes     Key attributes structure
* \retval                   Key length in bits
*/
static inline size_t cy_p64_psa_get_key_bits(
    const cy_p64_psa_key_attributes_t *attributes)
{
    return( attributes->core.bits );
}
/** \} */
#endif /* CY_P64_PSA_CRYPTO_STRUCT_H */
