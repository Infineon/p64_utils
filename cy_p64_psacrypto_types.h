/**
 * \file cy_p64_psacrypto_types.h
 *
 * \brief PSA cryptography module: type aliases.
 *
 * \note This file may not be included directly. Applications must
 * include cy_p64_psacrypto.h.
 *
 * This file contains portable definitions of integral types for properties
 * of cryptographic keys, designations of cryptographic algorithms, and
 * error codes returned by the library.
 *
 * This header file does not declare any function.
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

#ifndef CY_P64_PSA_CRYPTO_TYPES_H
#define CY_P64_PSA_CRYPTO_TYPES_H

#include <stdint.h>

/** \addtogroup psacrypto_error
 * \{
 */

/**
 * \brief PSA Functions return status.
 *
 * This is either #CY_P64_PSA_SUCCESS, indicating success,
 * or other value indicating that an error occurred. Errors are
 * encoded as one of the \c CY_P64_PSA_ERROR_xxx values defined here.
 * If #CY_P64_PSA_SUCCESS is already defined, it means that #cy_p64_psa_status_t
 * is also defined in an external header, so prevent its multiple
 * definition.
 */
#ifndef CY_P64_PSA_SUCCESS
typedef uint32_t cy_p64_psa_status_t;
#endif

/** \} */

/** \addtogroup crypto_types
 * \{
 */

/** \brief Encoding of a key type.
 */
typedef uint16_t cy_p64_psa_key_type_t;

/** The type of PSA Diffie-Hellman group family identifiers.
 *
 * The group identifier is required to create an Diffie-Hellman key using the
 * #CY_P64_PSA_KEY_TYPE_DH_KEY_PAIR() or #CY_P64_PSA_KEY_TYPE_DH_PUBLIC_KEY()
 * macros.
 *
 * Values defined by this standard will never be in the range 0x80-0xff.
 * Vendors who define additional families must use an encoding in this range.
 */
typedef uint8_t cy_p64_psa_dh_family_t;
/** \} */

/** \addtogroup policy
 * \{
 */

/** \brief Encoding of permitted usage on a key. */
typedef uint32_t cy_p64_psa_key_usage_t;
/** \} */

/** \addtogroup attributes
 * \{
 */

/** The type of a structure containing key attributes.
 *
 * This is an opaque structure that can represent the metadata of a key
 * object. Metadata that can be stored in attributes includes:
 * - The location of the key in storage, indicated by its key identifier
 *   and its lifetime.
 * - The key's policy, comprising usage flags and a specification of
 *   the permitted algorithm(s).
 * - Information about the key itself: the key type and its size.
 * - Implementations may define additional attributes.
 *
 * The actual key material is not considered an attribute of a key.
 * Key attributes do not contain information that is generally considered
 * highly confidential.
 *
 * An attribute structure can be a simple data structure where each function
 * `psa_set_key_xxx` sets a field and the corresponding function
 * `psa_get_key_xxx` retrieves the value of the corresponding field.
 * However, implementations may report values that are equivalent to the
 * original one, but have a different encoding. For example, an
 * implementation may use a more compact representation for types where
 * many bit-patterns are invalid or not supported, and store all values
 * that it does not support as a special marker value. In such an
 * implementation, after setting an invalid value, the corresponding
 * get function returns an invalid value which may not be the one that
 * was originally stored.
 *
 * An attribute structure may contain references to auxiliary resources,
 * for example pointers to allocated memory or indirect references to
 * pre-calculated values. In order to free such resources, the application
 * must call cy_p64_psa_reset_key_attributes(). As an exception, calling
 * cy_p64_psa_reset_key_attributes() on an attribute structure is optional if
 * the structure has only been modified by the following functions
 * since it was initialized or last reset with cy_p64_psa_reset_key_attributes():
 * - cy_p64_psa_set_key_id()
 * - cy_p64_psa_set_key_lifetime()
 * - cy_p64_psa_set_key_type()
 * - cy_p64_psa_set_key_bits()
 * - cy_p64_psa_set_key_usage_flags()
 * - cy_p64_psa_set_key_algorithm()
 *
 * Before calling any function on a key attribute structure, the application
 * must initialize it by any of the following means:
 * - Set the structure to all-bits-zero, for example:
 *   \code
 *   cy_p64_psa_key_attributes_t attributes;
 *   memset(&attributes, 0, sizeof(attributes));
 *   \endcode
 * - Initialize the structure to logical zero values, for example:
 *   \code
 *   cy_p64_psa_key_attributes_t attributes = {0};
 *   \endcode
 * - Initialize the structure to the initializer #CY_P64_PSA_KEY_ATTRIBUTES_INIT,
 *   for example:
 *   \code
 *   cy_p64_psa_key_attributes_t attributes = CY_P64_PSA_KEY_ATTRIBUTES_INIT;
 *   \endcode
 * - Assign the result of the function cy_p64_psa_key_attributes_init()
 *   to the structure, for example:
 *   \code
 *   cy_p64_psa_key_attributes_t attributes;
 *   attributes = cy_p64_psa_key_attributes_init();
 *   \endcode
 *
 * A freshly initialized attribute structure contains the following
 * values:
 *
 * - lifetime: #CY_P64_PSA_KEY_LIFETIME_VOLATILE.
 * - key identifier: 0 (which is not a valid key identifier).
 * - type: \c 0 (meaning that the type is unspecified).
 * - key size: \c 0 (meaning that the size is unspecified).
 * - usage flags: \c 0 (which allows no usage except exporting a public key).
 * - algorithm: \c 0 (which allows no cryptographic usage, but allows
 *   exporting).
 *
 * A typical sequence to create a key is as follows:
 * -# Create and initialize an attribute structure.
 * -# If the key is persistent, call cy_p64_psa_set_key_id().
 *    Also call cy_p64_psa_set_key_lifetime() to place the key in a non-default
 *    location.
 * -# Set the key policy with cy_p64_psa_set_key_usage_flags() and
 *    cy_p64_psa_set_key_algorithm().
 * -# Set the key type with cy_p64_psa_set_key_type().
 *    Skip this step if copying an existing key with psa_copy_key().
 * -# When generating a random key with cy_p64_psa_generate_key() or deriving a key
 *    with cy_p64_psa_key_derivation_output_key(), set the desired key size with
 *    cy_p64_psa_set_key_bits().
 * -# Call a key creation function: cy_p64_psa_import_key(), cy_p64_psa_generate_key(),
 *    cy_p64_psa_key_derivation_output_key() or psa_copy_key(). This function reads
 *    the attribute structure, creates a key with these attributes, and
 *    outputs a handle to the newly created key.
 * -# The attribute structure is now no longer necessary.
 *    You may call cy_p64_psa_reset_key_attributes(), although this is optional
 *    with the workflow presented here because the attributes currently
 *    defined in this specification do not require any additional resources
 *    beyond the structure itself.
 *
 * A typical sequence to query a key's attributes is as follows:
 * -# Call cy_p64_psa_get_key_attributes().
 * -# Call `psa_get_key_xxx` functions to retrieve the attribute(s) that
 *    you are interested in.
 * -# Call cy_p64_cy_p64_psa_reset_key_attributes() to free any resources that may be
 *    used by the attribute structure.
 *
 * Once a key is created, it is impossible to change its attributes.
 */
typedef struct cy_p64_psa_key_attributes_s cy_p64_psa_key_attributes_t;

/** The type of the key policy data structure.
 *
 * This is an implementation-defined \c struct. Applications should not
 * make any assumptions about the content of this structure except
 * as directed by the documentation of a specific implementation. */
typedef struct cy_p64_psa_key_policy_s cy_p64_psa_key_policy_t;

/** \} */


/** \addtogroup key_lifetimes
 * \{
 */

/** Encoding of key lifetimes.
 *
 * The lifetime of a key indicates where it is stored and what system actions
 * may create and destroy it.
 *
 * Lifetime values have the following structure:
 * - Bits 0-7 (#CY_P64_PSA_KEY_LIFETIME_GET_PERSISTENCE(\c lifetime)):
 *   persistence level. This value indicates what device management
 *   actions can cause it to be destroyed. In particular, it indicates
 *   whether the key is _volatile_ or _persistent_.
 *   See ::cy_p64_psa_key_persistence_t for more information.
 * - Bits 8-31 (#CY_P64_PSA_KEY_LIFETIME_GET_LOCATION(\c lifetime)):
 *   location indicator. This value indicates where the key is stored
 *   and where operations on the key are performed.
 *   See ::cy_p64_psa_key_location_t for more information.
 *
 * Volatile keys are automatically destroyed when the application instance
 * terminates or on a power reset of the device. Persistent keys are
 * preserved until the application explicitly destroys them or until an
 * implementation-specific device management event occurs (for example,
 * a factory reset).
 *
 * Persistent keys have a key identifier of type #cy_p64_psa_key_id_t.
 * This identifier remains valid throughout the lifetime of the key,
 * even if the application instance that created the key terminates.
 * The application can call cy_p64_keys_load_key_handle() to open a persistent
 * key that it created previously.
 *
 * This specification defines two basic lifetime values:
 * - Keys with the lifetime #CY_P64_PSA_KEY_LIFETIME_VOLATILE are volatile.
 *   All implementations should support this lifetime.
 * - Keys with the lifetime #CY_P64_PSA_KEY_LIFETIME_PERSISTENT are persistent.
 *   All implementations that have access to persistent storage with
 *   appropriate security guarantees should support this lifetime.
 */
typedef uint32_t cy_p64_psa_key_lifetime_t;

/** Encoding of key persistence levels.
 *
 * What distinguishes different persistence levels is what device management
 * events may cause keys to be destroyed. _Volatile_ keys are destroyed
 * by a power reset. Persistent keys may be destroyed by events such as
 * a transfer of ownership or a factory reset. What management events
 * actually affect persistent keys at different levels is outside the
 * scope of the PSA Cryptography specification.
 *
 * This specification defines the following values of persistence levels:
 * - \c 0 = #CY_P64_PSA_KEY_PERSISTENCE_VOLATILE: volatile key.
 *   A volatile key is automatically destroyed by the implementation when
 *   the application instance terminates. In particular, a volatile key
 *   is automatically destroyed on a power reset of the device.
 * - \c 1 = #CY_P64_PSA_KEY_PERSISTENCE_DEFAULT:
 *   persistent key with a default lifetime.
 *   Implementations should support this value if they support persistent
 *   keys at all.
 *   Applications should use this value if they have no specific needs that
 *   are only met by implementation-specific features.
 * - \c 2-127: persistent key with a PSA-specified lifetime.
 *   The PSA Cryptography specification does not define the meaning of these
 *   values, but other PSA specifications may do so.
 * - \c 128-254: persistent key with a vendor-specified lifetime.
 *   No PSA specification will define the meaning of these values, so
 *   implementations may choose the meaning freely.
 *   As a guideline, higher persistence levels should cause a key to survive
 *   more management events than lower levels.
 * - \c 255 = #CY_P64_PSA_KEY_PERSISTENCE_READ_ONLY:
 *   read-only or write-once key.
 *   A key with this persistence level cannot be destroyed.
 *   Implementations that support such keys may either allow their creation
 *   through the PSA Cryptography API, preferably only to applications with
 *   the appropriate privilege, or only expose keys created through
 *   implementation-specific means such as a factory ROM engraving process.
 *   Note that keys that are read-only due to policy restrictions
 *   rather than due to physical limitations should not have this
 *   persistence levels.
 *
 * \note Key persistence levels are 8-bit values. Key management
 *       interfaces operate on lifetimes (type ::cy_p64_psa_key_lifetime_t)
 *       which encode the persistence as the lower 8 bits of a 32-bit value.
 */
typedef uint8_t cy_p64_psa_key_persistence_t;

/** Encoding of key location indicators.
 *
 * If an implementation of this API can make calls to external
 * cryptoprocessors such as secure elements, the location of a key
 * indicates which secure element performs the operations on the key.
 * If an implementation offers multiple physical locations for persistent
 * storage, the location indicator reflects at which physical location
 * the key is stored.
 *
 * This specification defines the following values of location indicators:
 * - \c 0: primary local storage.
 *   All implementations should support this value.
 *   The primary local storage is typically the same storage area that
 *   contains the key metadata.
 * - \c 1: primary secure element.
 *   Implementations should support this value if there is a secure element
 *   attached to the operating environment.
 *   As a guideline, secure elements may provide higher resistance against
 *   side channel and physical attacks than the primary local storage, but may
 *   have restrictions on supported key types, sizes, policies and operations
 *   and may have different performance characteristics.
 * - \c 2-0x7fffff: other locations defined by a PSA specification.
 *   The PSA Cryptography API does not currently assign any meaning to these
 *   locations, but future versions of this specification or other PSA
 *   specifications may do so.
 * - \c 0x800000-0xffffff: vendor-defined locations.
 *   No PSA specification will assign a meaning to locations in this range.
 *
 * \note Key location indicators are 24-bit values. Key management
 *       interfaces operate on lifetimes (type ::cy_p64_psa_key_lifetime_t)
 *       which encode the location as the upper 24 bits of a 32-bit value.
 */
typedef uint32_t cy_p64_psa_key_location_t;

/** Encoding of identifiers of persistent keys.
 *
 * - Applications may freely choose key identifiers in the range
 *   #CY_P64_PSA_KEY_ID_USER_MIN to #CY_P64_PSA_KEY_ID_USER_MAX.
 * - Implementations may define additional key identifiers in the range
 *   #CY_P64_PSA_KEY_ID_VENDOR_MIN to #CY_P64_PSA_KEY_ID_VENDOR_MAX.
 * - 0 is reserved as an invalid key identifier.
 * - Key identifiers outside these ranges are reserved for future use.
 */
/* Implementation-specific quirk: The Mbed Crypto library can be built as
 * part of a multi-client service that exposes the PSA Crypto API in each
 * client and encodes the client identity in the key id argument of functions
 * such as cy_p64_keys_load_key_handle(). In this build configuration, we define
 * cy_p64_psa_key_id_t in crypto_platform.h instead of here. */
typedef uint32_t cy_p64_psa_key_id_t;

/** \} */

/** \addtogroup cipher_operations
 * \{
 */

/** The type of the state data structure for multipart cipher operations.
 *
 * This is an implementation-defined \c struct. Applications should not
 * make any assumptions about the content of this structure except
 * as directed by the documentation of a specific implementation. */
typedef struct cy_p64_psa_cipher_operation_s cy_p64_psa_cipher_operation_t;

/** \} */

/** \addtogroup hash_operations
 * \{
 */

/** The type of the state data structure for multipart hash operations.
 *
 * This is an implementation-defined \c struct. Applications should not
 * make any assumptions about the content of this structure except
 * as directed by the documentation of a specific implementation. */
typedef struct cy_p64_psa_hash_operation_s cy_p64_psa_hash_operation_t;

/** \} */

/** \addtogroup mac_operations
 * \{
 */

/** The type of the state data structure for multipart MAC operations.
 *
 * Before calling any function on a MAC operation object, the application must
 * initialize it by any of the following means:
 * - Set the structure to all-bits-zero, for example:
 *   \code
 *   cy_p64_psa_mac_operation_t operation;
 *   memset(&operation, 0, sizeof(operation));
 *   \endcode
 * - Initialize the structure to logical zero values, for example:
 *   \code
 *   cy_p64_psa_mac_operation_t operation = {0};
 *   \endcode
 *
 * This is an implementation-defined \c struct. Applications should not
 * make any assumptions about the content of this structure except
 * as directed by the documentation of a specific implementation. */
typedef struct cy_p64_psa_mac_operation_s cy_p64_psa_mac_operation_t;

/** \} */

/** \addtogroup key_derivation
 * \{
 */

/** The type of the state data structure for key derivation operations.
 *
 * Before calling any function on a key derivation operation object, the
 * application must initialize it by any of the following means:
 * - Set the structure to all-bits-zero, for example:
 *   \code
 *   cy_p64_psa_key_derivation_operation_t operation;
 *   memset(&operation, 0, sizeof(operation));
 *   \endcode
 * - Initialize the structure to logical zero values, for example:
 *   \code
 *   cy_p64_psa_key_derivation_operation_t operation = {0};
 *   \endcode
 * - Initialize the structure to the initializer #CY_P64_PSA_KEY_DERIVATION_OPERATION_INIT,
 *   for example:
 *   \code
 *   cy_p64_psa_key_derivation_operation_t operation = CY_P64_PSA_KEY_DERIVATION_OPERATION_INIT;
 *   \endcode
 * - Assign the result of the function cy_p64_psa_key_derivation_operation_init()
 *   to the structure, for example:
 *   \code
 *   cy_p64_psa_key_derivation_operation_t operation;
 *   operation = cy_p64_psa_key_derivation_operation_init();
 *   \endcode
 *
 * This is an implementation-defined \c struct. Applications should not
 * make any assumptions about the content of this structure except
 * as directed by the documentation of a specific implementation.
 */
typedef struct cy_p64_psa_key_derivation_s cy_p64_psa_key_derivation_operation_t;

/** \brief Encoding of the step of a key derivation. */
typedef uint16_t cy_p64_psa_key_derivation_step_t;

/** \} */

/** \addtogroup crypto_types
 * \{
 */

/** \brief Encoding of a cryptographic algorithm.
 *
 * For algorithms that can be applied to multiple key types, this type
 * does not encode the key type. For example, for symmetric ciphers
 * based on a block cipher, #cy_p64_psa_algorithm_t encodes the block cipher
 * mode and the padding mode while the block cipher itself is encoded
 * via #cy_p64_psa_key_type_t.
 */
typedef uint32_t cy_p64_psa_algorithm_t;

/** The type of PSA elliptic curve family identifiers.
 *
 * The curve identifier is required to create an ECC key using the
 * CY_P64_PSA_KEY_TYPE_ECC_KEY_PAIR() or CY_P64_PSA_KEY_TYPE_ECC_PUBLIC_KEY()
 * macros.
 *
 * Values defined by this standard will never be in the range 0x80-0xff.
 * Vendors who define additional families must use an encoding in this range.
 */
typedef uint8_t cy_p64_psa_ecc_family_t;

/** \} */

#endif /* CY_P64_PSA_CRYPTO_TYPES_H */
