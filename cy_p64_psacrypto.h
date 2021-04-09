/***************************************************************************//**
*
* \brief
*  This is the header file for the secure flashboot psa crypto syscalls.
*
********************************************************************************
* \copyright
* Copyright 2021, Cypress Semiconductor Corporation (an Infineon company).
* All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

/*******************************************************************************
* PSA crypto prototypes
****************************************************************************//**
*
* \defgroup psacrypto     PSA crypto
* \brief
*  This is the adopted version of PSA crypto API based on mbedTLS
*
* \{
*   \defgroup attributes Key attributes
*   \defgroup cipher_operations Cipher operations
*   \defgroup constants_sizes Data size definitions
*   \defgroup crypto_types Key and algorithm types
*   \defgroup error Error codes
*   \defgroup hash_operations Hash operations
*   \defgroup import_export Key import and export
*   \defgroup key_derivation Key derivation
*   \defgroup key_lifetimes Key lifetimes
*   \defgroup mac_operations MAC operations
*   \defgroup policy Key policies
*   \defgroup random Random generation
*   \defgroup mem Memory operation
* \}
*
*******************************************************************************/

#ifndef CY_P64_PASACRYPTO_H
#define CY_P64_PASACRYPTO_H

#include <stddef.h>

/* PSA requires several types which C99 provides in stdint.h. */
#include <stdint.h>

/** \addtogroup import_export
 * \{
 */

/** This type represents open handles to keys.
 * 0 is not a valid key handle. How other handle values are assigned is
 * implementation-dependent.
 * */
typedef uint16_t cy_p64_psa_key_handle_t;

#ifdef __cplusplus
extern "C" {
#endif

/* The file "crypto_types.h" declares types that encode errors,
 * algorithms, key types, policies, etc. */
#include "cy_p64_psacrypto_types.h"

/* The file "crypto_values.h" declares macros to build and analyze values
 * of integral types defined in "crypto_types.h". */
#include "cy_p64_psacrypto_values.h"

/* The file "crypto_struct.h" contains definitions for
 * implementation-specific structs that are declared above. */
#include "cy_p64_psacrypto_struct.h"

/* The file "crypto_sizes.h" contains definitions for size calculation
 * macros whose definitions are implementation-specific. */
#include "cy_p64_psacrypto_sizes.h"

/** \brief Key ID / slot number.
 *
 * This type represents key slot in Secure FlashBoot.
 * Each slot(1 to #CY_P64_KEY_SLOT_STATIC_MAX) stores key handle which is
 * initialized during boot by SFB.
 */
typedef uint32_t cy_p64_key_slot_t;

/** \} */

cy_p64_psa_status_t cy_p64_psa_sign_hash(
    cy_p64_psa_key_handle_t handle,
    cy_p64_psa_algorithm_t alg,
    const uint8_t *hash,
    size_t hash_length,
    const uint8_t *signature,
    size_t signature_size,
    size_t *signature_length);

cy_p64_psa_status_t cy_p64_psa_verify_hash(
    cy_p64_psa_key_handle_t handle,
    cy_p64_psa_algorithm_t alg,
    const uint8_t *hash,
    size_t hash_length,
    const uint8_t *signature,
    size_t signature_length);


/** \addtogroup cipher_operations
 * \{
 */
cy_p64_psa_status_t cy_p64_psa_cipher_decrypt_setup(
    cy_p64_psa_cipher_operation_t *operation,
    cy_p64_psa_key_handle_t handle,
    cy_p64_psa_algorithm_t alg);

cy_p64_psa_status_t cy_p64_psa_cipher_finish(
    cy_p64_psa_cipher_operation_t *operation,
    uint8_t *output,
    size_t output_size,
    size_t *output_length);

cy_p64_psa_status_t cy_p64_psa_cipher_set_iv(
    cy_p64_psa_cipher_operation_t *operation,
    const uint8_t *iv,
    size_t iv_length);

cy_p64_psa_status_t cy_p64_psa_cipher_update(
    cy_p64_psa_cipher_operation_t *operation,
    const uint8_t *input,
    size_t input_length,
    uint8_t *output,
    size_t output_size,
    size_t *output_length);
/** \} */


/**
 * \brief Destroys a key.
 *
 * This function destroys a key from both volatile
 * memory and, if applicable, non-volatile storage. Implementations shall
 * make a best effort to ensure that that the key material cannot be recovered.
 *
 * This function also erases any metadata such as policies and frees
 * resources associated with the key. To free all resources associated with
 * the key, all handles to the key must be closed or destroyed.
 *
 * Destroying the key makes the handle invalid, and the key handle
 * must not be used again by the application. Using other open handles to the
 * destroyed key in a cryptographic operation will result in an error.
 *
 * If a key is currently in use in a multipart operation, then destroying the
 * key will cause the multipart operation to fail.
 *
 * \param handle        The handle to the key to erase.
 *                      If this is \c 0, do nothing and return \c CY_P64_PSA_SUCCESS.
 *
 * \retval #CY_P64_PSA_SUCCESS
 *         \p handle was a valid handle and the key material that it
 *         referred to has been erased.
 *         Alternatively, \p handle is \c 0.
 * \retval #CY_P64_PSA_ERROR_NOT_PERMITTED
 *         The key cannot be erased because it is
 *         read-only, either due to a policy or due to physical restrictions.
 * \retval #CY_P64_PSA_ERROR_INVALID_HANDLE
 *         \p handle is not a valid handle nor \c 0.
 * \retval #CY_P64_PSA_ERROR_COMMUNICATION_FAILURE
 *         There was an failure in communication with the cryptoprocessor.
 *         The key material may still be present in the cryptoprocessor.
 * \retval #CY_P64_PSA_ERROR_STORAGE_FAILURE
 *         The storage is corrupted. Implementations shall make a best effort
 *         to erase key material even in this stage, however applications
 *         should be aware that it may be impossible to guarantee that the
 *         key material is not recoverable in such cases.
 * \retval #CY_P64_PSA_ERROR_CORRUPTION_DETECTED
 *         An unexpected condition which is not a storage corruption or
 *         a communication failure occurred. The cryptoprocessor may have
 *         been compromised.
 * \retval #CY_P64_PSA_ERROR_BAD_STATE
 *         It is implementation-dependent whether a failure to initialize
 *         results in this error code.
 */
 
cy_p64_psa_status_t cy_p64_psa_destroy_key(cy_p64_psa_key_handle_t handle);




cy_p64_psa_status_t cy_p64_psa_generate_key(
    cy_p64_psa_key_handle_t *handle,
    const cy_p64_psa_key_attributes_t *attributes);



/** \addtogroup random
 * \{
 */
cy_p64_psa_status_t cy_p64_psa_generate_random(
    uint8_t *output,
    size_t output_size);
/** \} */


/** \addtogroup hash_operations
 * \{
 */
cy_p64_psa_status_t cy_p64_psa_hash_setup(
    cy_p64_psa_hash_operation_t *operation,
    cy_p64_psa_algorithm_t alg);

cy_p64_psa_status_t cy_p64_psa_hash_update(
    cy_p64_psa_hash_operation_t *operation,
    const uint8_t *input,
    size_t input_length);

cy_p64_psa_status_t cy_p64_psa_hash_finish(
    cy_p64_psa_hash_operation_t *operation,
    uint8_t *hash,
    size_t hash_size,
    size_t *hash_length);
/** \} */


/** \addtogroup import_export
 * \{
 */
cy_p64_psa_status_t cy_p64_psa_import_key(
    const cy_p64_psa_key_attributes_t *attributes,
    const uint8_t *data,
    size_t data_length,
    cy_p64_psa_key_handle_t *handle);

cy_p64_psa_status_t cy_p64_psa_get_key_attributes(
    cy_p64_psa_key_handle_t handle,
    cy_p64_psa_key_attributes_t *attributes );

cy_p64_psa_status_t cy_p64_psa_export_key(cy_p64_psa_key_handle_t handle,
    uint8_t *data,
    size_t data_size,
    size_t *data_length);

cy_p64_psa_status_t cy_p64_psa_export_public_key(cy_p64_psa_key_handle_t handle,
    uint8_t *data,
    size_t data_size,
    size_t *data_length);
/** \} */


/** \addtogroup key_derivation
 * \{
 */
cy_p64_psa_status_t cy_p64_psa_key_derivation_abort(
    cy_p64_psa_key_derivation_operation_t *operation);

cy_p64_psa_status_t cy_p64_psa_key_derivation_setup(
    cy_p64_psa_key_derivation_operation_t *operation,
    cy_p64_psa_algorithm_t alg);

cy_p64_psa_status_t cy_p64_psa_key_derivation_inp_key(
    cy_p64_psa_key_derivation_operation_t *operation,
    cy_p64_psa_key_derivation_step_t step,
    cy_p64_psa_key_handle_t handle );

cy_p64_psa_status_t cy_p64_psa_key_derivation_inp_bytes(
    cy_p64_psa_key_derivation_operation_t *operation,
    cy_p64_psa_key_derivation_step_t step,
    const uint8_t *data,
    size_t data_length );

cy_p64_psa_status_t cy_p64_psa_key_derivation_key_agreement(
    cy_p64_psa_key_derivation_operation_t *operation,
    cy_p64_psa_key_derivation_step_t step,
    cy_p64_psa_key_handle_t private_key,
    const uint8_t *peer_key,
    size_t peer_key_length);

cy_p64_psa_status_t cy_p64_psa_key_derivation_out_bytes(
    cy_p64_psa_key_derivation_operation_t *operation,
    uint8_t *output,
    size_t output_length );

cy_p64_psa_status_t cy_p64_psa_key_derivation_out_key(
    const cy_p64_psa_key_attributes_t *attributes,
    cy_p64_psa_key_derivation_operation_t *operation,
    cy_p64_psa_key_handle_t *handle );
/** \} */

/** \addtogroup import_export
 * \{
 */

cy_p64_psa_status_t cy_p64_keys_load_key_handle(
    cy_p64_key_slot_t key_id,
    cy_p64_psa_key_handle_t *handle);

cy_p64_psa_status_t cy_p64_keys_store_key(
    cy_p64_key_slot_t key_id,
    cy_p64_psa_key_handle_t handle);

cy_p64_psa_status_t cy_p64_keys_close_key(cy_p64_key_slot_t key_id);

uint32_t cy_p64_keys_get_count(void);

/** \} */


/** \addtogroup mac_operations
 * \{
 */
cy_p64_psa_status_t cy_p64_psa_mac_verify_setup(
    cy_p64_psa_mac_operation_t *operation,
    cy_p64_psa_key_handle_t handle,
    cy_p64_psa_algorithm_t alg);

cy_p64_psa_status_t cy_p64_psa_mac_update(
    cy_p64_psa_mac_operation_t *operation,
    const uint8_t *input,
    size_t input_length);

cy_p64_psa_status_t cy_p64_psa_mac_verify_finish(
    cy_p64_psa_mac_operation_t *operation,
    const uint8_t *mac,
    size_t mac_length);
/** \} */

/** \addtogroup mem
 * \{
 */
cy_p64_psa_status_t cy_p64_psa_memset(
    void *dst_addr,
    uint8_t val,
    size_t data_size);

cy_p64_psa_status_t cy_p64_psa_memcpy(
    void *dst_addr,
    void const *src_addr,
    size_t data_size);
/** \} */

#ifdef __cplusplus
}
#endif

#endif /* CY_P64_PASACRYPTO_H */
