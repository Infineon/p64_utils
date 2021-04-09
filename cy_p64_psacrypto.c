/***************************************************************************//**
*
* \brief
*  This is the source code file for the secure flashboot psa crypto syscalls.
*
********************************************************************************
* \copyright
* Copyright 2021, Cypress Semiconductor Corporation (an Infineon company).
* All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include <stdbool.h>
#include "cy_device.h"
#include "cy_p64_psacrypto.h"
#include "cy_p64_syscall.h"

/** PSA crypto function code */
#define CY_P64_PSA_ASYMMETRIC_VERIFY             (0U)
#define CY_P64_PSA_EXPORT_PUBLIC_KEY             (1U)
#define CY_P64_PSA_GET_KEY_ATTRIBUTES            (2U)
#define CY_P64_PSA_KEY_DERIVATION_INPUT_KEY      (3U)
#define CY_P64_PSA_KEY_DERIVATION_INPUT_BYTES    (4U)
#define CY_P64_PSA_KEY_DERIVATION_ABORT          (5U)
#define CY_P64_PSA_KEY_DERIVATION_KEY_AGREEMENT  (6U)
#define CY_P64_PSA_KEY_DERIVATION_OUTPUT_BYTES   (7U)
#define CY_P64_PSA_IMPORT_KEY                    (9U)
#define CY_P64_PSA_DESTROY_KEY                  (10U)
#define CY_P64_PSA_CIPHER_DECRYPT_SETUP         (11U)
#define CY_P64_PSA_CIPHER_IV                    (12U)
#define CY_P64_PSA_CIPHER_UPDATE                (13U)
#define CY_P64_PSA_CIPHER_FINISH                (14U)
#define CY_P64_PSA_GENERATE_RANDOM              (15U)
#define CY_P64_PSA_HASH_SETUP                   (16U)
#define CY_P64_PSA_HASH_UPDATE                  (17U)
#define CY_P64_PSA_HASH_FINISH                  (18U)
#define CY_P64_PSA_ASYMMETRIC_SIGN              (19U)
#define CY_P64_KS_STORE_KEY_SLOT                (21U)
#define CY_P64_KS_LOAD_KEY_HANDLE               (22U)
#define CY_P64_KS_CLOSE_KEY_HANDLE              (23U)
#define CY_P64_PSA_KEY_DERIVATION_OUTPUT_KEY    (24U)
#define CY_P64_PSA_GENERATE_KEY                 (25U)
#define CY_P64_PSA_EXPORT_KEY                   (26U)
#define CY_P64_PSA_GET_KEYS_COUNT               (27U)
#define CY_P64_PSA_KEY_DERIVATION_SETUP         (29U)
#define CY_P64_PSA_MAC_VER_SETUP                (30U)
#define CY_P64_PSA_MAC_UPDATE                   (31U)
#define CY_P64_PSA_MAC_VER_FINISH               (32U)

#define CY_P64_PSA_MEMCPY                       (33U)
#define CY_P64_PSA_MEMSET                       (34U)

#define CY_P64_PSA_MAX_FUNC_ID                  CY_P64_PSA_MEMSET
#define CY_P64_PSA_FUNC_COUNT                   (CY_P64_PSA_MAX_FUNC_ID + 1)

/** PSA crypto SysCall opcode */
#define CY_P64_SYSCALL_OPCODE_PSA_CRYPTO         (0x35UL << 24UL)

#define CY_P64_SYSCALL_PSA_CRYPTO_CMD(syscallID) (CY_P64_SYSCALL_OPCODE_PSA_CRYPTO + ((uint32_t)(syscallID) << 8U))

/**
 * \brief Signs a hash or short message with a private key.
 *
 * Note that to perform a hash-and-sign signature algorithm, you must
 * first calculate the hash by calling cy_p64_psa_hash_setup(), cy_p64_psa_hash_update()
 * and cy_p64_psa_hash_finish(). Then pass the resulting hash as the \p hash
 * parameter to this function. You can use #CY_P64_PSA_ALG_SIGN_GET_HASH(\p alg)
 * to determine the hash algorithm to use.
 *
 * \param handle                Handle to the key to use for the operation.
 *                              It must be an asymmetric key pair.
 * \param alg                   A signature algorithm that is compatible with
 *                              the type of \p handle.
 * \param[in] hash              The hash or message to sign.
 * \param hash_length           Size of the \p hash buffer in bytes.
 * \param[out] signature        Buffer where the signature is to be written.
 * \param signature_size        Size of the \p signature buffer in bytes.
 * \param[out] signature_length On success, the number of bytes
 *                              that make up the returned signature value.
 *
 * \retval #CY_P64_PSA_SUCCESS
 * \retval #CY_P64_PSA_ERROR_INVALID_HANDLE
 * \retval #CY_P64_PSA_ERROR_NOT_PERMITTED
 * \retval #CY_P64_PSA_ERROR_BUFFER_TOO_SMALL
 * \retval #CY_P64_PSA_ERROR_NOT_SUPPORTED
 * \retval #CY_P64_PSA_ERROR_INVALID_ARGUMENT
 * \retval #CY_P64_PSA_ERROR_INSUFFICIENT_MEMORY
 * \retval #CY_P64_PSA_ERROR_COMMUNICATION_FAILURE
 * \retval #CY_P64_PSA_ERROR_HARDWARE_FAILURE
 * \retval #CY_P64_PSA_ERROR_CORRUPTION_DETECTED
 * \retval #CY_P64_PSA_ERROR_STORAGE_FAILURE
 * \retval #CY_P64_PSA_ERROR_INSUFFICIENT_ENTROPY
 * \retval #CY_P64_PSA_ERROR_BAD_STATE
 *         It is implementation-dependent whether initialize
 *         results fails in this error code
 */
 cy_p64_psa_status_t cy_p64_psa_sign_hash(
                                    cy_p64_psa_key_handle_t handle,
                                    cy_p64_psa_algorithm_t alg,
                                    const uint8_t *hash,
                                    size_t hash_length,
                                    const uint8_t *signature,
                                    size_t signature_size,
                                    size_t *signature_length)
 {
    cy_p64_psa_status_t status = CY_P64_PSA_ERROR_NOT_SUPPORTED;

    uint32_t syscall_cmd[2];
    uint32_t syscall_param[7];

    syscall_cmd[0] = CY_P64_SYSCALL_PSA_CRYPTO_CMD(CY_P64_PSA_ASYMMETRIC_SIGN);
    syscall_cmd[1] = (uint32_t)syscall_param;

    syscall_param[0] = (uint32_t)handle;
    syscall_param[1] = (uint32_t)alg;
    syscall_param[2] = (uint32_t)hash;
    syscall_param[3] = (uint32_t)hash_length;
    syscall_param[4] = (uint32_t)signature;
    syscall_param[5] = (uint32_t)signature_size;
    syscall_param[6] = (uint32_t)signature_length;

    status = cy_p64_syscall(syscall_cmd);

    return status;
 }

/**
 * \brief Verifies the signature of a hash or short message using a public key.
 *
 * Note that to perform a hash-and-sign signature algorithm, you must
 * first calculate the hash by calling cy_p64_psa_hash_setup(), cy_p64_psa_hash_update()
 * and cy_p64_psa_hash_finish(). Then pass the resulting hash as the \p hash
 * parameter to this function. You can use #CY_P64_PSA_ALG_SIGN_GET_HASH(\p alg)
 * to determine the hash algorithm to use.
 *
 * \param handle            Handle to the key to use for the operation.
 *                          It must be a public key or an asymmetric key pair.
 * \param alg               A signature algorithm compatible with
 *                          the type of \p handle.
 * \param[in] hash          The hash or message whose signature is to be
 *                          verified.
 * \param hash_length       The size of the \p hash buffer in bytes.
 * \param[in] signature     The buffer containing the signature to verify.
 * \param signature_length  The size of the \p signature buffer in bytes.
 *
 * \retval #CY_P64_PSA_SUCCESS
 *         The signature is valid.
 * \retval #CY_P64_PSA_ERROR_INVALID_HANDLE
 * \retval #CY_P64_PSA_ERROR_NOT_PERMITTED
 * \retval #CY_P64_PSA_ERROR_INVALID_SIGNATURE
 *         The calculation was perfomed successfully, but the passed
 *         signature is not a valid signature.
 * \retval #CY_P64_PSA_ERROR_NOT_SUPPORTED
 * \retval #CY_P64_PSA_ERROR_INVALID_ARGUMENT
 * \retval #CY_P64_PSA_ERROR_INSUFFICIENT_MEMORY
 * \retval #CY_P64_PSA_ERROR_COMMUNICATION_FAILURE
 * \retval #CY_P64_PSA_ERROR_HARDWARE_FAILURE
 * \retval #CY_P64_PSA_ERROR_CORRUPTION_DETECTED
 * \retval #CY_P64_PSA_ERROR_STORAGE_FAILURE
 * \retval #CY_P64_PSA_ERROR_BAD_STATE
 *         It is implementation-dependent whether initialize
 *         results fails in this error code
 */
cy_p64_psa_status_t cy_p64_psa_verify_hash(
                                    cy_p64_psa_key_handle_t handle,
                                    cy_p64_psa_algorithm_t alg,
                                    const uint8_t *hash,
                                    size_t hash_length,
                                    const uint8_t *signature,
                                    size_t signature_length)
{
    cy_p64_psa_status_t status = CY_P64_PSA_ERROR_NOT_SUPPORTED;

    uint32_t syscall_cmd[2];
    uint32_t syscall_param[6];

    syscall_cmd[0] = CY_P64_SYSCALL_PSA_CRYPTO_CMD(CY_P64_PSA_ASYMMETRIC_VERIFY);
    syscall_cmd[1] = (uint32_t)syscall_param;

    syscall_param[0] = (uint32_t)handle;
    syscall_param[1] = (uint32_t)alg;
    syscall_param[2] = (uint32_t)hash;
    syscall_param[3] = (uint32_t)hash_length;
    syscall_param[4] = (uint32_t)signature;
    syscall_param[5] = (uint32_t)signature_length;

    status = cy_p64_syscall(syscall_cmd);

    return status;
}

/** Sets the key for a multipart symmetric decryption operation.
 *
 * The sequence of operations to decrypt a message with a symmetric cipher
 * is as follows:
 * -# Allocate an operation object which will be passed to all the functions
 *    listed here.
 * -# Initialize the operation object with one of the methods described in the
 *    documentation for #cy_p64_psa_cipher_operation_t, e.g.
 *    #CY_P64_PSA_CIPHER_OPERATION_INIT.
 * -# Call cy_p64_psa_cipher_decrypt_setup() to specify the algorithm and key.
 * -# Call cy_p64_psa_cipher_set_iv() with the IV (initialization vector) for the
 *    decryption. If the IV is prepended to the ciphertext, you can call
 *    cy_p64_psa_cipher_update() on a buffer containing the IV followed by the
 *    beginning of the message.
 * -# Call cy_p64_psa_cipher_update() zero, one or more times, passing a fragment
 *    of the message each time.
 * -# Call cy_p64_psa_cipher_finish().
 *
 * After a successful call to cy_p64_psa_cipher_decrypt_setup(), the application must
 * eventually terminate the operation. The following events terminate an
 * operation:
 * - A successful call to cy_p64_psa_cipher_finish().
 *
 * \param[in,out] operation     The operation object to set up. It must have
 *                              been initialized as per the documentation for
 *                              #cy_p64_psa_cipher_operation_t and not yet in use.
 * \param handle                Handle to the key to use for the operation.
 *                              It must remain valid until the operation
 *                              terminates.
 * \param alg                   The cipher algorithm to compute
 *                              (\c CY_P64_PSA_ALG_XXX value such that
 *                              #CY_P64_PSA_ALG_IS_CIPHER(\p alg) is true).
 *
 * \retval #CY_P64_PSA_SUCCESS
 *         Success.
 * \retval #CY_P64_PSA_ERROR_INVALID_HANDLE
 * \retval #CY_P64_PSA_ERROR_NOT_PERMITTED
 * \retval #CY_P64_PSA_ERROR_INVALID_ARGUMENT
 *         \p handle is not compatible with \p alg.
 * \retval #CY_P64_PSA_ERROR_NOT_SUPPORTED
 *         \p alg is not supported or is not a cipher algorithm.
 * \retval #CY_P64_PSA_ERROR_INSUFFICIENT_MEMORY
 * \retval #CY_P64_PSA_ERROR_COMMUNICATION_FAILURE
 * \retval #CY_P64_PSA_ERROR_HARDWARE_FAILURE
 * \retval #CY_P64_PSA_ERROR_CORRUPTION_DETECTED
 * \retval #CY_P64_PSA_ERROR_STORAGE_FAILURE
 * \retval #CY_P64_PSA_ERROR_BAD_STATE
 *         The operation state is not valid (it must be inactive).
 * \retval #CY_P64_PSA_ERROR_BAD_STATE
 *         It is implementation-dependent whether initialize
 *         results fails in this error code
 */
cy_p64_psa_status_t cy_p64_psa_cipher_decrypt_setup(
                                    cy_p64_psa_cipher_operation_t *operation,
                                    cy_p64_psa_key_handle_t handle,
                                    cy_p64_psa_algorithm_t alg)
{
    cy_p64_psa_status_t status = CY_P64_PSA_ERROR_NOT_SUPPORTED;

    uint32_t syscall_cmd[2];
    uint32_t syscall_param[3];

    syscall_cmd[0] = CY_P64_SYSCALL_PSA_CRYPTO_CMD(CY_P64_PSA_CIPHER_DECRYPT_SETUP);
    syscall_cmd[1] = (uint32_t)syscall_param;

    syscall_param[0] = (uint32_t)operation;
    syscall_param[1] = (uint32_t)handle;
    syscall_param[2] = (uint32_t)alg;

    status = cy_p64_syscall(syscall_cmd);

    return status;
}

/** Finishes encrypting or decrypting a message in a cipher operation.
 *
 * The application must call
 * cy_p64_psa_cipher_decrypt_setup() before calling this function.
 * The choice of setup function determines whether this function encrypts
 * or decrypts its input.
 *
 * This function finishes the encryption or decryption of the message
 * formed by concatenating the inputs passed to preceding calls to
 * cy_p64_psa_cipher_update().
 *
 * When this function returns successfully, the operation becomes inactive.
 *
 * \param[in,out] operation     Active cipher operation.
 * \param[out] output           The buffer to write the output in.
 * \param output_size           The size of the \p output buffer in bytes.
 * \param[out] output_length    On success, the number of bytes
 *                              that make up the returned output.
 *
 * \retval #CY_P64_PSA_SUCCESS
 *         Success.
 * \retval #CY_P64_PSA_ERROR_INVALID_ARGUMENT
 *         The total input size passed to this operation is not valid for
 *         this particular algorithm. For example, the algorithm is a based
 *         on block cipher and requires a whole number of blocks, but the
 *         total input size is not a multiple of the block size.
 * \retval #CY_P64_PSA_ERROR_INVALID_PADDING
 *         This is a decryption operation for an algorithm that includes
 *         padding, and the ciphertext does not contain valid padding.
 * \retval #CY_P64_PSA_ERROR_BAD_STATE
 *         The operation state is not valid (it must be active, with an IV set
 *         if required for the algorithm).
 * \retval #CY_P64_PSA_ERROR_BUFFER_TOO_SMALL
 *         The size of the \p output buffer is too small.
 * \retval #CY_P64_PSA_ERROR_INSUFFICIENT_MEMORY
 * \retval #CY_P64_PSA_ERROR_COMMUNICATION_FAILURE
 * \retval #CY_P64_PSA_ERROR_HARDWARE_FAILURE
 * \retval #CY_P64_PSA_ERROR_CORRUPTION_DETECTED
 * \retval #CY_P64_PSA_ERROR_STORAGE_FAILURE
 * \retval #CY_P64_PSA_ERROR_BAD_STATE
 *         It is implementation-dependent whether initialize
 *         results fails in this error code
 */
cy_p64_psa_status_t cy_p64_psa_cipher_finish(
                                    cy_p64_psa_cipher_operation_t *operation,
                                    uint8_t *output,
                                    size_t output_size,
                                    size_t *output_length)
{
    cy_p64_psa_status_t status = CY_P64_PSA_ERROR_NOT_SUPPORTED;

    uint32_t syscall_cmd[2];
    uint32_t syscall_param[4];

    syscall_cmd[0] = CY_P64_SYSCALL_PSA_CRYPTO_CMD(CY_P64_PSA_CIPHER_FINISH);
    syscall_cmd[1] = (uint32_t)syscall_param;

    syscall_param[0] = (uint32_t)operation;
    syscall_param[1] = (uint32_t)output;
    syscall_param[2] = (uint32_t)output_size;
    syscall_param[3] = (uint32_t)output_length;

    status = cy_p64_syscall(syscall_cmd);

    return status;
}

/** Set the IV for a symmetric decryption operation.
 *
 * This function sets the IV (initialization vector), nonce
 * or initial counter value for the decryption operation.
 *
 * The application must call cy_p64_psa_cipher_decrypt_setup() before
 * calling this function.
 *
 * \param[in,out] operation     Active cipher operation.
 * \param[in] iv                The buffer that contains the IV to use.
 * \param iv_length             The size of the IV in bytes.
 *
 * \retval #CY_P64_PSA_SUCCESS
 *         Success.
 * \retval #CY_P64_PSA_ERROR_BAD_STATE
 *         The operation state is not valid (it must be an active cipher
 *         encrypt operation, with no IV set).
 * \retval #CY_P64_PSA_ERROR_INVALID_ARGUMENT
 *         The size of \p iv is not acceptable for the chosen algorithm,
 *         or the chosen algorithm does not use an IV.
 * \retval #CY_P64_PSA_ERROR_INSUFFICIENT_MEMORY
 * \retval #CY_P64_PSA_ERROR_COMMUNICATION_FAILURE
 * \retval #CY_P64_PSA_ERROR_HARDWARE_FAILURE
 * \retval #CY_P64_PSA_ERROR_CORRUPTION_DETECTED
 * \retval #CY_P64_PSA_ERROR_STORAGE_FAILURE
 * \retval #CY_P64_PSA_ERROR_BAD_STATE
 *         It is implementation-dependent whether initialize
 *         results fails in this error code
 */
cy_p64_psa_status_t cy_p64_psa_cipher_set_iv(
                                    cy_p64_psa_cipher_operation_t *operation,
                                    const uint8_t *iv,
                                    size_t iv_length)
{
    cy_p64_psa_status_t status = CY_P64_PSA_ERROR_NOT_SUPPORTED;

    uint32_t syscall_cmd[2];
    uint32_t syscall_param[3];

    syscall_cmd[0] = CY_P64_SYSCALL_PSA_CRYPTO_CMD(CY_P64_PSA_CIPHER_IV);
    syscall_cmd[1] = (uint32_t)syscall_param;

    syscall_param[0] = (uint32_t)operation;
    syscall_param[1] = (uint32_t)iv;
    syscall_param[2] = (uint32_t)iv_length;

    status = cy_p64_syscall(syscall_cmd);

    return status;
}

/** Decrypt a message fragment in an active cipher operation.
 *
 * Before calling this function, you must:
 * 1. Call cy_p64_psa_cipher_decrypt_setup().
 * 2. If the algorithm requires an IV, call cy_p64_psa_cipher_set_iv().
 *
 * \param[in,out] operation     Active cipher operation.
 * \param[in] input             The buffer that contains the message fragment to
 *                              encrypt or decrypt.
 * \param input_length          The size of the \p input buffer in bytes.
 * \param[out] output           The buffer to write the output in.
 * \param output_size           The size of the \p output buffer in bytes.
 * \param[out] output_length    On success, the number of bytes
 *                              that make up the returned output.
 *
 * \retval #CY_P64_PSA_SUCCESS
 *         Success.
 * \retval #CY_P64_PSA_ERROR_BAD_STATE
 *         The operation state is not valid (it must be active, with an IV set
 *         if required for the algorithm).
 * \retval #CY_P64_PSA_ERROR_BUFFER_TOO_SMALL
 *         The size of the \p output buffer is too small.
 * \retval #CY_P64_PSA_ERROR_INSUFFICIENT_MEMORY
 * \retval #CY_P64_PSA_ERROR_COMMUNICATION_FAILURE
 * \retval #CY_P64_PSA_ERROR_HARDWARE_FAILURE
 * \retval #CY_P64_PSA_ERROR_CORRUPTION_DETECTED
 * \retval #CY_P64_PSA_ERROR_STORAGE_FAILURE
 * \retval #CY_P64_PSA_ERROR_BAD_STATE
 *         It is implementation-dependent whether initialize
 *         results fails in this error code
 */
cy_p64_psa_status_t cy_p64_psa_cipher_update(
                                    cy_p64_psa_cipher_operation_t *operation,
                                    const uint8_t *input,
                                    size_t input_length,
                                    uint8_t *output,
                                    size_t output_size,
                                    size_t *output_length)
{
    cy_p64_psa_status_t status = CY_P64_PSA_ERROR_NOT_SUPPORTED;

    uint32_t syscall_cmd[2];
    uint32_t syscall_param[6];

    syscall_cmd[0] = CY_P64_SYSCALL_PSA_CRYPTO_CMD(CY_P64_PSA_CIPHER_UPDATE);
    syscall_cmd[1] = (uint32_t)syscall_param;

    syscall_param[0] = (uint32_t)operation;
    syscall_param[1] = (uint32_t)input;
    syscall_param[2] = (uint32_t)input_length;
    syscall_param[3] = (uint32_t)output;
    syscall_param[4] = (uint32_t)output_size;
    syscall_param[5] = (uint32_t)output_length;

    status = cy_p64_syscall(syscall_cmd);

    return status;
}

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
 * If a key is currently in use in a multipart operation, destroying the
 * key will cause the multipart operation to fail.
 *
 * \param handle        Handle to the key to erase.
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
 *         There was a failure in communication with the cryptoprocessor.
 *         The key material may still be present in the cryptoprocessor.
 * \retval #CY_P64_PSA_ERROR_STORAGE_FAILURE
 *         The storage is corrupted. Implementations shall make a best effort
 *         to erase the key material even in this stage, however applications
 *         should be aware that it may be impossible to guarantee that the
 *         key material is not recoverable in such cases.
 * \retval #CY_P64_PSA_ERROR_CORRUPTION_DETECTED
 *         An unexpected condition, which is not a storage corruption or
 *         a communication failure. The cryptoprocessor may have
 *         been compromised.
 * \retval #CY_P64_PSA_ERROR_BAD_STATE
 *         It is implementation-dependent whether initialize
 *         results fails in this error code
 */
cy_p64_psa_status_t cy_p64_psa_destroy_key(cy_p64_psa_key_handle_t handle)
{
    cy_p64_psa_status_t status = CY_P64_PSA_ERROR_NOT_SUPPORTED;

    uint32_t syscall_cmd[2];
    uint32_t syscall_param;

    syscall_cmd[0] = CY_P64_SYSCALL_PSA_CRYPTO_CMD(CY_P64_PSA_DESTROY_KEY);
    syscall_cmd[1] = (uint32_t)&syscall_param;

    syscall_param = (uint32_t)handle;

    status = cy_p64_syscall(syscall_cmd);

    return status;
}

/**
 * \brief Generates a key or key pair.
 *
 * The key is generated randomly.
 * Its location, usage policy, type and size are taken from \p attributes.
 *
 * Implementations must reject an attempt to generate a key of size 0.
 *
 * The following type-specific considerations apply:
 * - For RSA keys (#CY_P64_PSA_KEY_TYPE_RSA_KEY_PAIR),
 *   the public exponent is 65537.
 *   The modulus is a product of two probabilistic primes
 *   between 2^{n-1} and 2^n where n is the bit size specified in the
 *   attributes.
 *
 * \param[in] attributes    The attributes for the new key.
 * \param[out] handle       On success, a handle to the newly created key.
 *                          \c 0 on failure.
 *
 * \retval #CY_P64_PSA_SUCCESS
 *         Success.
 *         If the key is persistent, the key material and the key's metadata
 *         have been saved to persistent storage.
 * \retval #CY_P64_PSA_ERROR_ALREADY_EXISTS
 *         This is an attempt to create a persistent key, and there is
 *         already a persistent key with the given identifier.
 * \retval #CY_P64_PSA_ERROR_NOT_SUPPORTED
 * \retval #CY_P64_PSA_ERROR_INVALID_ARGUMENT
 * \retval #CY_P64_PSA_ERROR_INSUFFICIENT_MEMORY
 * \retval #CY_P64_PSA_ERROR_INSUFFICIENT_ENTROPY
 * \retval #CY_P64_PSA_ERROR_COMMUNICATION_FAILURE
 * \retval #CY_P64_PSA_ERROR_HARDWARE_FAILURE
 * \retval #CY_P64_PSA_ERROR_CORRUPTION_DETECTED
 * \retval #CY_P64_PSA_ERROR_INSUFFICIENT_STORAGE
 * \retval #CY_P64_PSA_ERROR_STORAGE_FAILURE
 * \retval #CY_P64_PSA_ERROR_BAD_STATE
 *         It is implementation-dependent whether initialize
 *         results fails in this error code
 */
cy_p64_psa_status_t cy_p64_psa_generate_key(
                                    cy_p64_psa_key_handle_t *handle,
                                    const cy_p64_psa_key_attributes_t *attributes)
{
    cy_p64_psa_status_t status = CY_P64_PSA_ERROR_NOT_SUPPORTED;

    uint32_t syscall_cmd[2];
    uint32_t syscall_param[2];

    syscall_cmd[0] = CY_P64_SYSCALL_PSA_CRYPTO_CMD(CY_P64_PSA_GENERATE_KEY);
    syscall_cmd[1] = (uint32_t)syscall_param;

    syscall_param[0] = (uint32_t)handle;
    syscall_param[1] = (uint32_t)attributes;

    status = cy_p64_syscall(syscall_cmd);

    return status;
}

/**
 * \brief Generates random bytes.
 *
 * \warning This function **can** fail! Callers MUST check the return status
 *          and MUST NOT use the content of the output buffer if the return
 *          status is not #CY_P64_PSA_SUCCESS.
 *
 * \note    To generate a key, use cy_p64_psa_generate_key() instead.
 *
 * \param[out] output       The output buffer for the generated data.
 * \param output_size       The number of bytes to generate and output.
 *
 * \retval #CY_P64_PSA_SUCCESS
 * \retval #CY_P64_PSA_ERROR_NOT_SUPPORTED
 * \retval #CY_P64_PSA_ERROR_INSUFFICIENT_ENTROPY
 * \retval #CY_P64_PSA_ERROR_INSUFFICIENT_MEMORY
 * \retval #CY_P64_PSA_ERROR_COMMUNICATION_FAILURE
 * \retval #CY_P64_PSA_ERROR_HARDWARE_FAILURE
 * \retval #CY_P64_PSA_ERROR_CORRUPTION_DETECTED
 * \retval #CY_P64_PSA_ERROR_BAD_STATE
 *         It is implementation-dependent whether initialize
 *         results fails in this error code.
 */
cy_p64_psa_status_t cy_p64_psa_generate_random(uint8_t *output,
                                    size_t output_size)
{
    cy_p64_psa_status_t status = CY_P64_PSA_ERROR_NOT_SUPPORTED;

    uint32_t syscall_cmd[2];
    uint32_t syscall_param[2];

    syscall_cmd[0] = CY_P64_SYSCALL_PSA_CRYPTO_CMD(CY_P64_PSA_GENERATE_RANDOM);
    syscall_cmd[1] = (uint32_t)syscall_param;

    syscall_param[0] = (uint32_t)output;
    syscall_param[1] = (uint32_t)output_size;

    status = cy_p64_syscall(syscall_cmd);

    return status;
}

/** Aborts a key derivation operation.
 *
 * Aborting an operation frees all associated resources except for the \c
 * operation structure itself. Once aborted, the operation object can be reused
 * for another operation by calling cy_p64_psa_key_derivation_setup() again.
 *
 * This function may be called at any time after the operation
 * object has been initialized as described in #cy_p64_psa_key_derivation_operation_t.
 *
 * In particular, it is valid to call cy_p64_psa_key_derivation_abort() twice, or to
 * call cy_p64_psa_key_derivation_abort() on an operation that has not been set up.
 *
 * \param[in,out] operation    The operation to abort.
 *
 * \retval #CY_P64_PSA_SUCCESS
 * \retval #CY_P64_PSA_ERROR_COMMUNICATION_FAILURE
 * \retval #CY_P64_PSA_ERROR_HARDWARE_FAILURE
 * \retval #CY_P64_PSA_ERROR_CORRUPTION_DETECTED
 * \retval #CY_P64_PSA_ERROR_BAD_STATE
 *         It is implementation-dependent whether initialize
 *         results fails in this error code
 */
cy_p64_psa_status_t cy_p64_psa_key_derivation_abort(
                                    cy_p64_psa_key_derivation_operation_t *operation)
{
    cy_p64_psa_status_t status = CY_P64_PSA_ERROR_NOT_SUPPORTED;

    uint32_t syscall_cmd[2];
    uint32_t syscall_param;

    syscall_cmd[0] = CY_P64_SYSCALL_PSA_CRYPTO_CMD(CY_P64_PSA_KEY_DERIVATION_ABORT);
    syscall_cmd[1] = (uint32_t)&syscall_param;

    syscall_param = (uint32_t)operation;

    status = cy_p64_syscall(syscall_cmd);

    return status;
}

/** Sets up a multipart hash operation.
 *
 * The sequence of operations to calculate a hash (message digest)
 * is as follows:
 * -# Allocate an operation object to be passed to all the functions
 *    listed here.
 * -# Initialize the operation object with one of the methods described in the
 *    documentation for #cy_p64_psa_hash_operation_t, e.g. #CY_P64_PSA_HASH_OPERATION_INIT.
 * -# Call cy_p64_psa_hash_setup() to specify the algorithm.
 * -# Call cy_p64_psa_hash_update() zero, one or more times, passing a fragment
 *    of the message each time. The hash that is calculated is the hash
 *    of the concatenation of these messages in order.
 * -# To calculate the hash, call cy_p64_psa_hash_finish().
 *    To compare the hash with an expected value, call cy_p64_psa_hash_verify().
 *
 * If an error occurs at any step after a call to cy_p64_psa_hash_setup(), 
 * reset the operation by calling to cy_p64_psa_hash_abort(). The
 * application may call cy_p64_psa_hash_abort() at any time after the operation
 * has been initialized.
 *
 * After a successful call to cy_p64_psa_hash_setup(), the application must
 * eventually terminate the operation. The following events terminate an
 * operation:
 * - A successful call to cy_p64_psa_hash_finish() or cy_p64_psa_hash_verify().
 * - A call to cy_p64_psa_hash_abort().
 *
 * \param[in,out] operation The operation object to set up. It must have
 *                          been initialized as per the documentation for
 *                          #cy_p64_psa_hash_operation_t and not yet in use.
 * \param alg               The hash algorithm to compute (\c CY_P64_PSA_ALG_XXX value
 *                          such that #CY_P64_PSA_ALG_IS_HASH(\p alg) is true).
 *
 * \retval #CY_P64_PSA_SUCCESS
 *         Success.
 * \retval #CY_P64_PSA_ERROR_NOT_SUPPORTED
 *         \p alg is not a supported hash algorithm.
 * \retval #CY_P64_PSA_ERROR_INVALID_ARGUMENT
 *         \p alg is not a hash algorithm.
 * \retval #CY_P64_PSA_ERROR_BAD_STATE
 *         The operation state is not valid (it must be inactive).
 * \retval #CY_P64_PSA_ERROR_INSUFFICIENT_MEMORY
 * \retval #CY_P64_PSA_ERROR_COMMUNICATION_FAILURE
 * \retval #CY_P64_PSA_ERROR_HARDWARE_FAILURE
 * \retval #CY_P64_PSA_ERROR_CORRUPTION_DETECTED
 * \retval #CY_P64_PSA_ERROR_BAD_STATE
 *         It is implementation-dependent whether initialize
 *         results fails in this error code
 */
cy_p64_psa_status_t cy_p64_psa_hash_setup(
                                    cy_p64_psa_hash_operation_t *operation,
                                    cy_p64_psa_algorithm_t alg)
{
    cy_p64_psa_status_t status = CY_P64_PSA_ERROR_NOT_SUPPORTED;

    uint32_t syscall_cmd[2];
    uint32_t syscall_param[2];

    syscall_cmd[0] = CY_P64_SYSCALL_PSA_CRYPTO_CMD(CY_P64_PSA_HASH_SETUP);
    syscall_cmd[1] = (uint32_t)syscall_param;

    syscall_param[0] = (uint32_t)operation;
    syscall_param[1] = (uint32_t)alg;

    status = cy_p64_syscall(syscall_cmd);

    return status;
}

/** Adds a message fragment to a multipart hash operation.
 *
 * The application must call cy_p64_psa_hash_setup() before calling this function.
 *
 * If this function returns an error status, the operation enters an error
 * state and must be aborted by calling cy_p64_psa_hash_abort().
 *
 * \param[in,out] operation Active hash operation.
 * \param[in] input         The buffer that contains the message fragment to hash.
 * \param input_length      The size of the \p input buffer in bytes.
 *
 * \retval #CY_P64_PSA_SUCCESS
 *         Success.
 * \retval #CY_P64_PSA_ERROR_BAD_STATE
 *         The operation state is not valid (it must be active).
 * \retval #CY_P64_PSA_ERROR_INSUFFICIENT_MEMORY
 * \retval #CY_P64_PSA_ERROR_COMMUNICATION_FAILURE
 * \retval #CY_P64_PSA_ERROR_HARDWARE_FAILURE
 * \retval #CY_P64_PSA_ERROR_CORRUPTION_DETECTED
 * \retval #CY_P64_PSA_ERROR_BAD_STATE
 *         It is implementation-dependent whether initialize
 *         results fails in this error code
 */
cy_p64_psa_status_t cy_p64_psa_hash_update(
                                    cy_p64_psa_hash_operation_t *operation,
                                    const uint8_t *input,
                                    size_t input_length)
{
    cy_p64_psa_status_t status = CY_P64_PSA_ERROR_NOT_SUPPORTED;

    uint32_t syscall_cmd[2];
    uint32_t syscall_param[3];

    syscall_cmd[0] = CY_P64_SYSCALL_PSA_CRYPTO_CMD(CY_P64_PSA_HASH_UPDATE);
    syscall_cmd[1] = (uint32_t)syscall_param;

    syscall_param[0] = (uint32_t)operation;
    syscall_param[1] = (uint32_t)input;
    syscall_param[2] = (uint32_t)input_length;

    status = cy_p64_syscall(syscall_cmd);

    return status;
}

/** Finishes the calculation of the hash of a message.
 *
 * The application must call cy_p64_psa_hash_setup() before calling this function.
 * This function calculates the hash of the message formed by concatenating
 * the inputs passed to preceding calls to cy_p64_psa_hash_update().
 *
 * When this function returns success, the operation becomes inactive.
 * If this function returns an error status, the operation enters an error
 * state and must be aborted by calling cy_p64_psa_hash_abort().
 *
 * \warning Applications should not call this function if they expect
 *          a specific value for the hash. Call cy_p64_psa_hash_verify() instead.
 *          Beware that comparing integrity or authenticity data such as
 *          hash values with a function such as \c memcmp is risky
 *          because the time taken by the comparison may leak information
 *          about the hashed data which could allow an attacker to guess
 *          a valid hash and thereby bypass security controls.
 *
 * \param[in,out] operation     Active hash operation.
 * \param[out] hash             The buffer to write the hash in.
 * \param hash_size             The size of the \p hash buffer in bytes.
 * \param[out] hash_length      On success, the number of bytes
 *                              that make up the hash value. This is always
 *                              #CY_P64_PSA_HASH_SIZE(\c alg) where \c alg is the
 *                              hash algorithm that is calculated.
 *
 * \retval #CY_P64_PSA_SUCCESS
 *         Success.
 * \retval #CY_P64_PSA_ERROR_BAD_STATE
 *         The operation state is not valid (it must be active).
 * \retval #CY_P64_PSA_ERROR_BUFFER_TOO_SMALL
 *         The size of the \p hash buffer is too small. You can determine a
 *         sufficient buffer size by calling #CY_P64_PSA_HASH_SIZE(\c alg)
 *         where \c alg is the hash algorithm that is calculated.
 * \retval #CY_P64_PSA_ERROR_INSUFFICIENT_MEMORY
 * \retval #CY_P64_PSA_ERROR_COMMUNICATION_FAILURE
 * \retval #CY_P64_PSA_ERROR_HARDWARE_FAILURE
 * \retval #CY_P64_PSA_ERROR_CORRUPTION_DETECTED
 * \retval #CY_P64_PSA_ERROR_BAD_STATE
 *         It is implementation-dependent whether initialize
 *         results fails in this error code
 */
cy_p64_psa_status_t cy_p64_psa_hash_finish(
                                    cy_p64_psa_hash_operation_t *operation,
                                    uint8_t *hash,
                                    size_t hash_size,
                                    size_t *hash_length)
{
    cy_p64_psa_status_t status = CY_P64_PSA_ERROR_NOT_SUPPORTED;

    uint32_t syscall_cmd[2];
    uint32_t syscall_param[4];

    syscall_cmd[0] = CY_P64_SYSCALL_PSA_CRYPTO_CMD(CY_P64_PSA_HASH_FINISH);
    syscall_cmd[1] = (uint32_t)syscall_param;

    syscall_param[0] = (uint32_t)operation;
    syscall_param[1] = (uint32_t)hash;
    syscall_param[2] = (uint32_t)hash_size;
    syscall_param[3] = (uint32_t)hash_length;

    status = cy_p64_syscall(syscall_cmd);

    return status;
}

/**
 * \brief Imports a key in binary format.
 *
 * This function supports any output from cy_p64_psa_export_key(). Refer to the
 * documentation of cy_p64_psa_export_public_key() for the format of public keys
 * and to the documentation of cy_p64_psa_export_key() for the format for
 * other key types.
 *
 * The key data determines the key size. The attributes may optionally
 * specify a key size; it must match the size determined
 * from the key data. A key size of 0 in \p attributes indicates that
 * the key size is solely determined by the key data.
 *
 * Implementations must reject an attempt to import a key of size 0.
 *
 * This specification supports a single format for each key type.
 * Implementations may support other formats as long as the standard
 * format is supported. Implementations that support other formats
 * should ensure that the formats are clearly unambiguous so as to
 * minimize the risk that an invalid input is accidentally interpreted
 * according to a different format.
 *
 * \param[in] attributes    The attributes for the new key.
 *                          The key size is always determined from the
 *                          \p data buffer.
 *                          If the key size in \p attributes is nonzero,
 *                          it must be equal to the size from \p data.
 * \param[out] handle       On success, a handle to the newly created key.
 *                          \c 0 on failure.
 * \param[in] data    The buffer that contains the key data. The content of this
 *                    buffer is interpreted according to the type declared
 *                    in \p attributes.
 *                    All implementations must support at least the format
 *                    described in the documentation
 *                    of cy_p64_psa_export_key() or cy_p64_psa_export_public_key() for
 *                    the chosen type. Implementations may allow other
 *                    formats, but should be conservative: implementations
 *                    should err on the side of rejecting content if it
 *                    may be erroneous (e.g. wrong type or truncated data).
 * \param data_length Size of the \p data buffer in bytes.
 *
 * \retval #CY_P64_PSA_SUCCESS
 *         Success.
 *         If the key is persistent, the key material and the key's metadata
 *         have been saved to persistent storage.
 * \retval #CY_P64_PSA_ERROR_ALREADY_EXISTS
 *         This is an attempt to create a persistent key, and there is
 *         already a persistent key with the given identifier.
 * \retval #CY_P64_PSA_ERROR_NOT_SUPPORTED
 *         The key type or key size is not supported, either by the
 *         implementation in general or in this particular persistent location.
 * \retval #CY_P64_PSA_ERROR_INVALID_ARGUMENT
 *         The key attributes, as a whole, are invalid.
 * \retval #CY_P64_PSA_ERROR_INVALID_ARGUMENT
 *         The key data is not correctly formatted.
 * \retval #CY_P64_PSA_ERROR_INVALID_ARGUMENT
 *         The size in \p attributes is nonzero and does not match the size
 *         of the key data.
 * \retval #CY_P64_PSA_ERROR_INSUFFICIENT_MEMORY
 * \retval #CY_P64_PSA_ERROR_INSUFFICIENT_STORAGE
 * \retval #CY_P64_PSA_ERROR_COMMUNICATION_FAILURE
 * \retval #CY_P64_PSA_ERROR_STORAGE_FAILURE
 * \retval #CY_P64_PSA_ERROR_HARDWARE_FAILURE
 * \retval #CY_P64_PSA_ERROR_CORRUPTION_DETECTED
 * \retval #CY_P64_PSA_ERROR_BAD_STATE
 *         It is implementation-dependent whether initialize
 *         results fails in this error code
 */
cy_p64_psa_status_t cy_p64_psa_import_key(
                                    const cy_p64_psa_key_attributes_t *attributes,
                                    const uint8_t *data,
                                    size_t data_length,
                                    cy_p64_psa_key_handle_t *handle)
{
    cy_p64_psa_status_t status = CY_P64_PSA_ERROR_NOT_SUPPORTED;

    uint32_t syscall_cmd[2];
    uint32_t syscall_param[4];

    syscall_cmd[0] = CY_P64_SYSCALL_PSA_CRYPTO_CMD(CY_P64_PSA_IMPORT_KEY);
    syscall_cmd[1] = (uint32_t)syscall_param;

    syscall_param[0] = (uint32_t)handle;
    syscall_param[1] = (uint32_t)attributes;
    syscall_param[2] = (uint32_t)data;
    syscall_param[3] = (uint32_t)data_length;

    status = cy_p64_syscall(syscall_cmd);

    return status;
}

/** Retrieves the attributes of a key.
 *
 * This function first resets the attribute structure as with
 * cy_p64_psa_reset_key_attributes(). It then copies the attributes of
 * the given key into the given attribute structure.
 *
 * \note This function may allocate memory or other resources.
 *       Once you have called this function on an attribute structure,
 *       you must call cy_p64_psa_reset_key_attributes() to free these resources.
 *
 * \param[in] handle            The handle to the key to query.
 * \param[in,out] attributes    On success, the attributes of the key.
 *                              On failure, equivalent to a
 *                              freshly-initialized structure.
 *
 * \retval #CY_P64_PSA_SUCCESS
 * \retval #CY_P64_PSA_ERROR_INVALID_HANDLE
 * \retval #CY_P64_PSA_ERROR_INSUFFICIENT_MEMORY
 * \retval #CY_P64_PSA_ERROR_COMMUNICATION_FAILURE
 * \retval #CY_P64_PSA_ERROR_CORRUPTION_DETECTED
 * \retval #CY_P64_PSA_ERROR_STORAGE_FAILURE
 * \retval #CY_P64_PSA_ERROR_BAD_STATE
 *         It is implementation-dependent whether initialize
 *         results fails in this error code
 */
cy_p64_psa_status_t cy_p64_psa_get_key_attributes(
                                    cy_p64_psa_key_handle_t handle,
                                    cy_p64_psa_key_attributes_t *attributes )
{
    cy_p64_psa_status_t status = CY_P64_PSA_ERROR_NOT_SUPPORTED;

    uint32_t syscall_cmd[2];
    uint32_t syscall_param[2];

    syscall_cmd[0] = CY_P64_SYSCALL_PSA_CRYPTO_CMD(CY_P64_PSA_GET_KEY_ATTRIBUTES);
    syscall_cmd[1] = (uint32_t)syscall_param;

    syscall_param[0] = (uint32_t)handle;
    syscall_param[1] = (uint32_t)attributes;

    status = cy_p64_syscall(syscall_cmd);

    return status;
}

/**
 * \brief Exports a key in binary format.
 *
 * The output of this function can be passed to cy_p64_psa_import_key() to
 * create an equivalent object.
 *
 * If the implementation of cy_p64_psa_import_key() supports other formats
 * beyond the format specified here, the output from cy_p64_psa_export_key()
 * must use the representation specified here, not the original
 * representation.
 *
 * For standard key types, the output format is as follows:
 *
 * - For symmetric keys (including MAC keys), the format is the
 *   raw bytes of the key.
 * - For DES, the key data consists of 8 bytes. The parity bits must be
 *   correct.
 * - For Triple-DES, the format is the concatenation of the
 *   two or three DES keys.
 * - For RSA key pairs (#CY_P64_PSA_KEY_TYPE_RSA_KEY_PAIR), the format
 *   is the non-encrypted DER encoding of the representation defined by
 *   PKCS\#1 (RFC 8017) as `RSAPrivateKey`, version 0.
 *   ```
 *   RSAPrivateKey ::= SEQUENCE {
 *       version             INTEGER,  -- must be 0
 *       modulus             INTEGER,  -- n
 *       publicExponent      INTEGER,  -- e
 *       privateExponent     INTEGER,  -- d
 *       prime1              INTEGER,  -- p
 *       prime2              INTEGER,  -- q
 *       exponent1           INTEGER,  -- d mod (p-1)
 *       exponent2           INTEGER,  -- d mod (q-1)
 *       coefficient         INTEGER,  -- (inverse of q) mod p
 *   }
 *   ```
 * - For elliptic curve key pairs (key types for which
 *   #CY_P64_PSA_KEY_TYPE_IS_ECC_KEY_PAIR is true), the format is
 *   a representation of the private value as a `ceiling(m/8)`-byte string
 *   where `m` is the bit size associated with the curve, i.e. the bit size
 *   of the order of the curve's coordinate field. This byte string is
 *   in little-endian order for Montgomery curves (curve types
 *   `PSA_ECC_FAMILY_CURVEXXX`), and in big-endian order for Weierstrass
 *   curves (curve types `PSA_ECC_FAMILY_SECTXXX`, `PSA_ECC_FAMILY_SECPXXX`
 *   and `PSA_ECC_FAMILY_BRAINPOOL_PXXX`).
 *   For Weierstrass curves, this is the content of the `privateKey` field of
 *   the `ECPrivateKey` format defined by RFC 5915.  For Montgomery curves,
 *   the format is defined by RFC 7748, and output is masked according to sect.5.
 * - For Diffie-Hellman key exchange key pairs (key types for which
 *   #CY_P64_PSA_KEY_TYPE_IS_DH_KEY_PAIR is true), the
 *   format is the representation of the private key `x` as a big-endian byte
 *   string. The length of the byte string is the private key size in bytes
 *   (leading zeroes are not stripped).
 * - For public keys (key types for which #CY_P64_PSA_KEY_TYPE_IS_PUBLIC_KEY is
 *   true), the format is the same as for cy_p64_psa_export_public_key().
 *
 * The policy on the key must have the usage flag #CY_P64_PSA_KEY_USAGE_EXPORT set.
 *
 * \param handle            The handle to the key to export.
 * \param[out] data         The buffer to write the key data in.
 * \param data_size         The size of the \p data buffer in bytes.
 * \param[out] data_length  On success, the number of bytes
 *                          that make up the key data.
 *
 * \retval #CY_P64_PSA_SUCCESS
 * \retval #CY_P64_PSA_ERROR_INVALID_HANDLE
 * \retval #CY_P64_PSA_ERROR_NOT_PERMITTED
 *         The key does not have the #CY_P64_PSA_KEY_USAGE_EXPORT flag.
 * \retval #CY_P64_PSA_ERROR_NOT_SUPPORTED
 * \retval #CY_P64_PSA_ERROR_BUFFER_TOO_SMALL
 *         The size of the \p data buffer is too small. You can determine a
 *         sufficient buffer size by calling
 *         #CY_P64_PSA_KEY_EXPORT_MAX_SIZE(\c type, \c bits)
 *         where \c type is the key type
 *         and \c bits is the key size in bits.
 * \retval #CY_P64_PSA_ERROR_COMMUNICATION_FAILURE
 * \retval #CY_P64_PSA_ERROR_HARDWARE_FAILURE
 * \retval #CY_P64_PSA_ERROR_CORRUPTION_DETECTED
 * \retval #CY_P64_PSA_ERROR_STORAGE_FAILURE
 * \retval #CY_P64_PSA_ERROR_INSUFFICIENT_MEMORY
 * \retval #CY_P64_PSA_ERROR_BAD_STATE
 *         It is implementation-dependent whether initialize
 *         results fails in this error code
 */
cy_p64_psa_status_t cy_p64_psa_export_key(
                                    cy_p64_psa_key_handle_t handle,
                                    uint8_t *data,
                                    size_t data_size,
                                    size_t *data_length)
{
    cy_p64_psa_status_t status = CY_P64_PSA_ERROR_NOT_SUPPORTED;

    uint32_t syscall_cmd[2];
    uint32_t syscall_param[4];

    syscall_cmd[0] = CY_P64_SYSCALL_PSA_CRYPTO_CMD(CY_P64_PSA_EXPORT_KEY);
    syscall_cmd[1] = (uint32_t)syscall_param;

    syscall_param[0] = (uint32_t)handle;
    syscall_param[1] = (uint32_t)data;
    syscall_param[2] = (uint32_t)data_size;
    syscall_param[3] = (uint32_t)data_length;

    status = cy_p64_syscall(syscall_cmd);

    return status;
}

/**
 * \brief Export a public key or the public part of a key pair in binary format.
 *
 * The output of this function can be passed to cy_p64_psa_import_key() to
 * create an object that is equivalent to the public key.
 *
 * This specification supports a single format for each key type.
 * Implementations may support other formats as long as the standard
 * format is supported. Implementations that support other formats
 * should ensure that the formats are clearly unambiguous so as to
 * minimize the risk that an invalid input is accidentally interpreted
 * according to a different format.
 *
 * For standard key types, the output format is as follows:
 * - For RSA public keys (#CY_P64_PSA_KEY_TYPE_RSA_PUBLIC_KEY), the DER encoding of
 *   the representation defined by RFC 3279 &sect;2.3.1 as `RSAPublicKey`.
 *   ```
 *   RSAPublicKey ::= SEQUENCE {
 *      modulus            INTEGER,    -- n
 *      publicExponent     INTEGER  }  -- e
 *   ```
 * - For elliptic curve public keys (key types for which
 *   #CY_P64_PSA_KEY_TYPE_IS_ECC_PUBLIC_KEY is true), the format is the uncompressed
 *   representation defined by SEC1 &sect;2.3.3 as the content of an ECPoint.
 *   Let `m` be the bit size associated with the curve, i.e. the bit size of
 *   `q` for a curve over `F_q`. The representation consists of:
 *      - The byte 0x04;
 *      - `x_P` as a `ceiling(m/8)`-byte string, big-endian;
 *      - `y_P` as a `ceiling(m/8)`-byte string, big-endian.
 * - For Diffie-Hellman key exchange public keys (key types for which
 *   #CY_P64_PSA_KEY_TYPE_IS_DH_PUBLIC_KEY is true),
 *   the format is the representation of the public key `y = g^x mod p` as a
 *   big-endian byte string. The length of the byte string is the length of the
 *   base prime `p` in bytes.
 *
 * Exporting a public key object or the public part of a key pair is
 * always permitted, regardless of the key's usage flags.
 *
 * \param handle            The handle to the key to export.
 * \param[out] data         The buffer to write the key data in.
 * \param data_size         The size of the \p data buffer in bytes.
 * \param[out] data_length  On success, the number of bytes
 *                          that make up the key data.
 *
 * \retval #CY_P64_PSA_SUCCESS
 * \retval #CY_P64_PSA_ERROR_INVALID_HANDLE
 * \retval #CY_P64_PSA_ERROR_INVALID_ARGUMENT
 *         The key is neither a public key nor a key pair.
 * \retval #CY_P64_PSA_ERROR_NOT_SUPPORTED
 * \retval #CY_P64_PSA_ERROR_BUFFER_TOO_SMALL
 *         The size of the \p data buffer is too small. You can determine a
 *         sufficient buffer size by calling
 *         #CY_P64_PSA_KEY_EXPORT_MAX_SIZE(#CY_P64_PSA_KEY_TYPE_PUBLIC_KEY_OF_KEY_PAIR(\c type), \c bits)
 *         where \c type is the key type
 *         and \c bits is the key size in bits.
 * \retval #CY_P64_PSA_ERROR_COMMUNICATION_FAILURE
 * \retval #CY_P64_PSA_ERROR_HARDWARE_FAILURE
 * \retval #CY_P64_PSA_ERROR_CORRUPTION_DETECTED
 * \retval #CY_P64_PSA_ERROR_STORAGE_FAILURE
 * \retval #CY_P64_PSA_ERROR_INSUFFICIENT_MEMORY
 * \retval #CY_P64_PSA_ERROR_BAD_STATE
 *         It is implementation-dependent whether initialize
 *         results fails in this error code
 */
cy_p64_psa_status_t cy_p64_psa_export_public_key(
                                    cy_p64_psa_key_handle_t handle,
                                    uint8_t *data,
                                    size_t data_size,
                                    size_t *data_length)
{
    cy_p64_psa_status_t status = CY_P64_PSA_ERROR_NOT_SUPPORTED;

    uint32_t syscall_cmd[2];
    uint32_t syscall_param[4];

    syscall_cmd[0] = CY_P64_SYSCALL_PSA_CRYPTO_CMD(CY_P64_PSA_EXPORT_PUBLIC_KEY);
    syscall_cmd[1] = (uint32_t)syscall_param;

    syscall_param[0] = (uint32_t)handle;
    syscall_param[1] = (uint32_t)data;
    syscall_param[2] = (uint32_t)data_size;
    syscall_param[3] = (uint32_t)data_length;

    status = cy_p64_syscall(syscall_cmd);

    return status;
}

/** Derives a key from an ongoing key derivation operation.
 *
 * This function calculates output bytes from a key derivation algorithm
 * and uses those bytes to generate a key deterministically.
 * The key's location, usage policy, type and size are taken from
 * \p attributes.
 *
 * If you view the key derivation's output as a stream of bytes, this
 * function destructively reads as many bytes as required from the
 * stream.
 * The operation's capacity decreases by the number of bytes read.
 *
 * If this function returns an error status other than
 * #CY_P64_PSA_ERROR_INSUFFICIENT_DATA, the operation enters an error
 * state and must be aborted by calling cy_p64_psa_key_derivation_abort().
 *
 * How much output is produced and consumed from the operation, and how
 * the key is derived, depends on the key type:
 *
 * - For key types for which the key is an arbitrary sequence of bytes
 *   of a given size, this function is functionally equivalent to
 *   calling #cy_p64_psa_key_derivation_out_bytes
 *   and passing the resulting output to #cy_p64_psa_import_key.
 *   However, this function has a security benefit:
 *   if the implementation provides an isolation boundary then
 *   the key material is not exposed outside the isolation boundary.
 *   As a consequence, for these key types, this function always consumes
 *   exactly (\p bits / 8) bytes from the operation.
 *   The following key types defined in this specification follow this scheme:
 *
 *     - #CY_P64_PSA_KEY_TYPE_AES;
 *     - #CY_P64_PSA_KEY_TYPE_ARC4;
 *     - #CY_P64_PSA_KEY_TYPE_CAMELLIA;
 *     - #CY_P64_PSA_KEY_TYPE_DERIVE;
 *     - #CY_P64_PSA_KEY_TYPE_HMAC.
 *
 * - For ECC keys on a Montgomery elliptic curve
 *   (#CY_P64_PSA_KEY_TYPE_ECC_KEY_PAIR(\c curve) where \c curve designates a
 *   Montgomery curve), this function always draws a byte string whose
 *   length is determined by the curve, and sets the mandatory bits
 *   accordingly. That is:
 *
 *     - Curve25519 (#CY_P64_PSA_ECC_FAMILY_MONTGOMERY, 255 bits): draw a 32-byte
 *       string and process it as specified in RFC 7748 &sect;5.
 *     - Curve448 (#CY_P64_PSA_ECC_FAMILY_MONTGOMERY, 448 bits): draw a 56-byte
 *       string and process it as specified in RFC 7748 &sect;5.
 *
 * - For key types for which the key is represented by a single sequence of
 *   \p bits bits with constraints as to which bit sequences are acceptable,
 *   this function draws a byte string of length (\p bits / 8) bytes rounded
 *   up to the nearest whole number of bytes. If the resulting byte string
 *   is acceptable, it becomes the key, otherwise the drawn bytes are discarded.
 *   This process is repeated until an acceptable byte string is drawn.
 *   The byte string drawn from the operation is interpreted as specified
 *   for the output produced by cy_p64_psa_export_key().
 *   The following key types defined in this specification follow this scheme:
 *
 *     - #CY_P64_PSA_KEY_TYPE_DES.
 *       Force-set the parity bits, but discard forbidden weak keys.
 *       For 2-key and 3-key triple-DES, the three keys are generated
 *       successively (for example, for 3-key triple-DES,
 *       if the first 8 bytes specify a weak key and the next 8 bytes do not,
 *       discard the first 8 bytes, use the next 8 bytes as the first key,
 *       and continue reading output from the operation to derive the other
 *       two keys).
 *     - Finite-field Diffie-Hellman keys (#CY_P64_PSA_KEY_TYPE_DH_KEY_PAIR(\c group)
 *       where \c group designates any Diffie-Hellman group) and
 *       ECC keys on a Weierstrass elliptic curve
 *       (#CY_P64_PSA_KEY_TYPE_ECC_KEY_PAIR(\c curve) where \c curve designates a
 *       Weierstrass curve).
 *       For these key types, interpret the byte string as integer
 *       in big-endian order. Discard it if it is not in the range
 *       [0, *N* - 2] where *N* is the boundary of the private key domain
 *       (the prime *p* for Diffie-Hellman, the subprime *q* for DSA,
 *       or the order of the curve's base point for ECC).
 *       Add 1 to the resulting integer and use this as the private key *x*.
 *       This method allows compliance to NIST standards, specifically
 *       the methods titled "key-pair generation by testing candidates"
 *       in NIST SP 800-56A &sect;5.6.1.1.4 for Diffie-Hellman,
 *       in FIPS 186-4 &sect;B.1.2 for DSA, and
 *       in NIST SP 800-56A &sect;5.6.1.2.2 or
 *       FIPS 186-4 &sect;B.4.2 for elliptic curve keys.
 *
 * - For other key types, including #CY_P64_PSA_KEY_TYPE_RSA_KEY_PAIR,
 *   the way in which the operation output is consumed is
 *   implementation-defined.
 *
 * In all cases, the data that is read is discarded from the operation.
 * The operation's capacity is decreased by the number of bytes read.
 *
 * For algorithms that take an input step #CY_P64_PSA_KEY_DERIVATION_INPUT_SECRET,
 * the input to that step must be provided with cy_p64_psa_key_derivation_input_key().
 * Future versions of this specification may include additional restrictions
 * on the derived key based on the attributes and strength of the secret key.
 *
 * \param[in] attributes    The attributes for the new key.
 * \param[in,out] operation The key derivation operation object to read from.
 * \param[out] handle       On success, a handle to the newly created key.
 *                          \c 0 on failure.
 *
 * \retval #CY_P64_PSA_SUCCESS
 *         Success.
 *         If the key is persistent, the key material and the key's metadata
 *         have been saved to persistent storage.
 * \retval #CY_P64_PSA_ERROR_ALREADY_EXISTS
 *         This is an attempt to create a persistent key, and there is
 *         already a persistent key with the given identifier.
 * \retval #CY_P64_PSA_ERROR_INSUFFICIENT_DATA
 *         There was not enough data to create the desired key.
 *         Note that in this case, no output is written to the output buffer.
 *         The operation's capacity is set to 0, thus subsequent calls to
 *         this function will not succeed, even with a smaller output buffer.
 * \retval #CY_P64_PSA_ERROR_NOT_SUPPORTED
 *         The key type or key size is not supported, either by the
 *         implementation in general or in this particular location.
 * \retval #CY_P64_PSA_ERROR_INVALID_ARGUMENT
 *         The provided key attributes are not valid for the operation.
 * \retval #CY_P64_PSA_ERROR_NOT_PERMITTED
 *         The #CY_P64_PSA_KEY_DERIVATION_INPUT_SECRET input was not provided through
 *         a key.
 * \retval #CY_P64_PSA_ERROR_BAD_STATE
 *         The operation state is not valid (it must be active and completed
 *         all required input steps).
 * \retval #CY_P64_PSA_ERROR_INSUFFICIENT_MEMORY
 * \retval #CY_P64_PSA_ERROR_INSUFFICIENT_STORAGE
 * \retval #CY_P64_PSA_ERROR_COMMUNICATION_FAILURE
 * \retval #CY_P64_PSA_ERROR_HARDWARE_FAILURE
 * \retval #CY_P64_PSA_ERROR_CORRUPTION_DETECTED
 * \retval #CY_P64_PSA_ERROR_STORAGE_FAILURE
 * \retval #CY_P64_PSA_ERROR_BAD_STATE
 *         It is implementation-dependent whether initialize
 *         results fails in this error code
 */
cy_p64_psa_status_t cy_p64_psa_key_derivation_out_key(
                                    const cy_p64_psa_key_attributes_t *attributes,
                                    cy_p64_psa_key_derivation_operation_t *operation,
                                    cy_p64_psa_key_handle_t *handle )
{
    cy_p64_psa_status_t status = CY_P64_PSA_ERROR_NOT_SUPPORTED;

    uint32_t syscall_cmd[2];
    uint32_t syscall_param[3];

    syscall_cmd[0] = CY_P64_SYSCALL_PSA_CRYPTO_CMD( CY_P64_PSA_KEY_DERIVATION_OUTPUT_KEY);
    syscall_cmd[1] = (uint32_t)syscall_param;

    syscall_param[0] = (uint32_t)operation;
    syscall_param[1] = (uint32_t)handle;
    syscall_param[2] = (uint32_t)attributes;

    status = cy_p64_syscall(syscall_cmd);

    return status;
}

/** Provides an input for key derivation in the form of a key.
 *
 * Which inputs are required and in what order depends on the algorithm.
 * Refer to the documentation of each key derivation or key agreement
 * algorithm for information.
 *
 * This function obtains input from a key object, which is usually correct for
 * secret inputs or for non-secret personalization strings kept in the key
 * store. To pass a non-secret parameter which is not in the key store,
 * call CY_P64_PSA_key_derivation_inp_bytes() instead of this function.
 * Refer to the documentation of individual step types
 * (`PSA_KEY_DERIVATION_INPUT_xxx` values of type ::cy_p64_psa_key_derivation_step_t)
 * for more information.
 *
 * If this function returns an error status, the operation enters an error
 * state and must be aborted by calling cy_p64_psa_key_derivation_abort().
 *
 * \param[in,out] operation       The key derivation operation object to use.
 *                                It must have been set up with
 *                                cy_p64_psa_key_derivation_setup() and must not
 *                                have produced any output yet.
 * \param step                    Which step the input data is for.
 * \param handle                  Handle to the key. It must have an
 *                                appropriate type for \p step and must
 *                                allow the usage #CY_P64_PSA_KEY_USAGE_DERIVE.
 *
 * \retval #CY_P64_PSA_SUCCESS
 *         Success.
 * \retval #CY_P64_PSA_ERROR_INVALID_HANDLE
 * \retval #CY_P64_PSA_ERROR_NOT_PERMITTED
 * \retval #CY_P64_PSA_ERROR_INVALID_ARGUMENT
 *         \c step is not compatible with the operation's algorithm.
 * \retval #CY_P64_PSA_ERROR_INVALID_ARGUMENT
 *         \c step does not allow key inputs of the given type
 *         or does not allow key inputs at all.
 * \retval #CY_P64_PSA_ERROR_INSUFFICIENT_MEMORY
 * \retval #CY_P64_PSA_ERROR_COMMUNICATION_FAILURE
 * \retval #CY_P64_PSA_ERROR_HARDWARE_FAILURE
 * \retval #CY_P64_PSA_ERROR_CORRUPTION_DETECTED
 * \retval #CY_P64_PSA_ERROR_STORAGE_FAILURE
 * \retval #CY_P64_PSA_ERROR_BAD_STATE
 *         The operation state is not valid for this input \p step.
 * \retval #CY_P64_PSA_ERROR_BAD_STATE
 *         It is implementation-dependent whether initialize
 *         results fails in this error code
 */
cy_p64_psa_status_t cy_p64_psa_key_derivation_inp_key(
                                    cy_p64_psa_key_derivation_operation_t *operation,
                                    cy_p64_psa_key_derivation_step_t step,
                                    cy_p64_psa_key_handle_t handle )
{
    cy_p64_psa_status_t status = CY_P64_PSA_ERROR_NOT_SUPPORTED;

    uint32_t syscall_cmd[2];
    uint32_t syscall_param[3];

    syscall_cmd[0] = CY_P64_SYSCALL_PSA_CRYPTO_CMD(CY_P64_PSA_KEY_DERIVATION_INPUT_KEY);
    syscall_cmd[1] = (uint32_t)syscall_param;

    syscall_param[0] = (uint32_t)operation;
    syscall_param[1] = (uint32_t)step;
    syscall_param[2] = (uint32_t)handle;

    status = cy_p64_syscall(syscall_cmd);

    return status;
}

/** Provides an input for key derivation or key agreement.
 *
 * Which inputs are required and in what order depends on the algorithm.
 * Refer to the documentation of each key derivation or key agreement
 * algorithm for information.
 *
 * This function passes direct inputs, which is usually correct for
 * non-secret inputs. To pass a secret input, which should be in a key
 * object, call cy_p64_psa_key_derivation_input_key() instead of this function.
 * Refer to the documentation of individual step types
 * (`CY_P64_PSA_KEY_DERIVATION_INPUT_xxx` values of type ::cy_p64_psa_key_derivation_step_t)
 * for more information.
 *
 * If this function returns an error status, the operation enters an error
 * state and must be aborted by calling cy_p64_psa_key_derivation_abort().
 *
 * \param[in,out] operation       The key derivation operation object to use.
 *                                It must have been set up with
 *                                cy_p64_psa_key_derivation_setup() and must not
 *                                have produced any output yet.
 * \param step                    Which step the input data is for.
 * \param[in] data                Input data to use.
 * \param data_length             Size of the \p data buffer in bytes.
 *
 * \retval #CY_P64_PSA_SUCCESS
 *         Success.
 * \retval #CY_P64_PSA_ERROR_INVALID_ARGUMENT
 *         \c step is not compatible with the operation's algorithm.
 * \retval #CY_P64_PSA_ERROR_INVALID_ARGUMENT
 *         \c step does not allow direct inputs.
 * \retval #CY_P64_PSA_ERROR_INSUFFICIENT_MEMORY
 * \retval #CY_P64_PSA_ERROR_COMMUNICATION_FAILURE
 * \retval #CY_P64_PSA_ERROR_HARDWARE_FAILURE
 * \retval #CY_P64_PSA_ERROR_CORRUPTION_DETECTED
 * \retval #CY_P64_PSA_ERROR_STORAGE_FAILURE
 * \retval #CY_P64_PSA_ERROR_BAD_STATE
 *         The operation state is not valid for this input \p step.
 * \retval #CY_P64_PSA_ERROR_BAD_STATE
 *         It is implementation-dependent whether initialize
 *         results fails in this error code
 */
cy_p64_psa_status_t cy_p64_psa_key_derivation_inp_bytes(
                                    cy_p64_psa_key_derivation_operation_t *operation,
                                    cy_p64_psa_key_derivation_step_t step,
                                    const uint8_t *data,
                                    size_t data_length )
{
    cy_p64_psa_status_t status = CY_P64_PSA_ERROR_NOT_SUPPORTED;

    uint32_t syscall_cmd[2];
    uint32_t syscall_param[4];

    syscall_cmd[0] = CY_P64_SYSCALL_PSA_CRYPTO_CMD(CY_P64_PSA_KEY_DERIVATION_INPUT_BYTES);
    syscall_cmd[1] = (uint32_t)syscall_param;

    syscall_param[0] = (uint32_t)operation;
    syscall_param[1] = (uint32_t)step;
    syscall_param[2] = (uint32_t)data;
    syscall_param[3] = (uint32_t)data_length;

    status = cy_p64_syscall(syscall_cmd);

    return status;
}

/** Reads some data from a key derivation operation.
 *
 * This function calculates output bytes from a key derivation algorithm and
 * return those bytes.
 * If you view the key derivation's output as a stream of bytes, this
 * function destructively reads the requested number of bytes from the
 * stream.
 * The operation's capacity decreases by the number of bytes read.
 *
 * If this function returns an error status other than
 * #CY_P64_PSA_ERROR_INSUFFICIENT_DATA, the operation enters an error
 * state and must be aborted by calling cy_p64_psa_key_derivation_abort().
 *
 * \param[in,out] operation The key derivation operation object to read from.
 * \param[out] output       Buffer where the output will be written.
 * \param output_length     Number of bytes to output.
 *
 * \retval #CY_P64_PSA_SUCCESS
 * \retval #CY_P64_PSA_ERROR_INSUFFICIENT_DATA
 *                          The operation's capacity was less than
 *                          \p output_length bytes. Note that in this case,
 *                          no output is written to the output buffer.
 *                          The operation's capacity is set to 0, thus
 *                          subsequent calls to this function will not
 *                          succeed, even with a smaller output buffer.
 * \retval #CY_P64_PSA_ERROR_BAD_STATE
 *         The operation state is not valid (it must be active and completed
 *         all required input steps).
 * \retval #CY_P64_PSA_ERROR_INSUFFICIENT_MEMORY
 * \retval #CY_P64_PSA_ERROR_COMMUNICATION_FAILURE
 * \retval #CY_P64_PSA_ERROR_HARDWARE_FAILURE
 * \retval #CY_P64_PSA_ERROR_CORRUPTION_DETECTED
 * \retval #CY_P64_PSA_ERROR_STORAGE_FAILURE
 * \retval #CY_P64_PSA_ERROR_BAD_STATE
 *         It is implementation-dependent whether initialize
 *         results fails in this error code
 */
cy_p64_psa_status_t cy_p64_psa_key_derivation_out_bytes(
                                    cy_p64_psa_key_derivation_operation_t *operation,
                                    uint8_t *output,
                                    size_t output_length )
{
    cy_p64_psa_status_t status = CY_P64_PSA_ERROR_NOT_SUPPORTED;

    uint32_t syscall_cmd[2];
    uint32_t syscall_param[3];

    syscall_cmd[0] = CY_P64_SYSCALL_PSA_CRYPTO_CMD(CY_P64_PSA_KEY_DERIVATION_OUTPUT_BYTES);
    syscall_cmd[1] = (uint32_t)syscall_param;

    syscall_param[0] = (uint32_t)operation;
    syscall_param[1] = (uint32_t)output;
    syscall_param[2] = (uint32_t)output_length;

    status = cy_p64_syscall(syscall_cmd);

    return status;
}

/** Performs a key agreement and use the shared secret as input to a key
 * derivation.
 *
 * A key agreement algorithm takes two inputs: a private key \p private_key
 * a public key \p peer_key.
 * The result of this function is passed as input to a key derivation.
 * The output of this key derivation can be extracted by reading from the
 * resulting operation to produce keys and other cryptographic material.
 *
 * If this function returns an error status, the operation enters an error
 * state and must be aborted by calling cy_p64_psa_key_derivation_abort().
 *
 * \param[in,out] operation       The key derivation operation object to use.
 *                                It must have been set up with
 *                                cy_p64_psa_key_derivation_setup() with a
 *                                key agreement and derivation algorithm
 *                                \c alg (\c CY_P64_PSA_ALG_XXX value such that
 *                                #CY_P64_PSA_ALG_IS_KEY_AGREEMENT(\c alg) is true
 *                                and #CY_P64_PSA_ALG_IS_RAW_KEY_AGREEMENT(\c alg)
 *                                is false).
 *                                The operation must be ready for an
 *                                input of the type given by \p step.
 * \param step                    Which step the input data is for.
 * \param private_key             Handle to the private key to use.
 * \param[in] peer_key      The public key of the peer. The peer key must be in the
 *                          same format that cy_p64_psa_import_key() accepts for the
 *                          public key type corresponding to the type of
 *                          private_key. That is, this function performs the
 *                          equivalent of
 *                          #cy_p64_psa_import_key(...,
 *                          `peer_key`, `peer_key_length`) where
 *                          with key attributes indicating the public key
 *                          type corresponding to the type of `private_key`.
 *                          For example, for EC keys, this means that peer_key
 *                          is interpreted as a point on the curve that the
 *                          private key is on. The standard formats for public
 *                          keys are documented in the documentation of
 *                          cy_p64_psa_export_public_key().
 * \param peer_key_length         Size of \p peer_key in bytes.
 *
 * \retval #CY_P64_PSA_SUCCESS
 *         Success.
 * \retval #CY_P64_PSA_ERROR_BAD_STATE
 *         The operation state is not valid for this key agreement \p step.
 * \retval #CY_P64_PSA_ERROR_INVALID_HANDLE
 * \retval #CY_P64_PSA_ERROR_NOT_PERMITTED
 * \retval #CY_P64_PSA_ERROR_INVALID_ARGUMENT
 *         \c private_key is not compatible with \c alg,
 *         or \p peer_key is not valid for \c alg or not compatible with
 *         \c private_key.
 * \retval #CY_P64_PSA_ERROR_NOT_SUPPORTED
 *         \c alg is not supported or is not a key derivation algorithm.
 * \retval #CY_P64_PSA_ERROR_INVALID_ARGUMENT
 *         \c step does not allow an input resulting from a key agreement.
 * \retval #CY_P64_PSA_ERROR_INSUFFICIENT_MEMORY
 * \retval #CY_P64_PSA_ERROR_COMMUNICATION_FAILURE
 * \retval #CY_P64_PSA_ERROR_HARDWARE_FAILURE
 * \retval #CY_P64_PSA_ERROR_CORRUPTION_DETECTED
 * \retval #CY_P64_PSA_ERROR_STORAGE_FAILURE
 * \retval #CY_P64_PSA_ERROR_BAD_STATE
 *         It is implementation-dependent whether initialize
 *         results fails in this error code
 */
cy_p64_psa_status_t cy_p64_psa_key_derivation_key_agreement(
                                    cy_p64_psa_key_derivation_operation_t *operation,
                                    cy_p64_psa_key_derivation_step_t step,
                                    cy_p64_psa_key_handle_t private_key,
                                    const uint8_t *peer_key,
                                    size_t peer_key_length)
{
    cy_p64_psa_status_t status = CY_P64_PSA_ERROR_NOT_SUPPORTED;

    uint32_t syscall_cmd[2];
    uint32_t syscall_param[5];

    syscall_cmd[0] = CY_P64_SYSCALL_PSA_CRYPTO_CMD(CY_P64_PSA_KEY_DERIVATION_KEY_AGREEMENT);
    syscall_cmd[1] = (uint32_t)syscall_param;

    syscall_param[0] = (uint32_t)operation;
    syscall_param[1] = (uint32_t)step;
    syscall_param[2] = (uint32_t)private_key;
    syscall_param[3] = (uint32_t)peer_key;
    syscall_param[4] = (uint32_t)peer_key_length;

    status = cy_p64_syscall(syscall_cmd);

    return status;
}


/** This function stores a new key handle for the key slot.
 * If the slot was not empty the function first destroys existing key.
 * Keys in slot range from 1 to #CY_P64_KEY_SLOT_STATIC_MAX are initialized
 * during boot by SFB. They are protected from modification by SFB.
 *
 * \param[in] key_id    The slot number in SFB key storage.
 *                      Minimum value - #CY_P64_KEY_SLOT_STATIC_MAX + 1.
 *                      Maximum value - cy_p64_keys_get_count()
 * \param[out] handle   Key handle to store.
 *
 * \retval #CY_P64_PSA_SUCCESS
 *         Success.
 * \retval #CY_P64_PSA_ERROR_DOES_NOT_EXIST
 * \retval #CY_P64_PSA_ERROR_INVALID_ARGUMENT
 */
cy_p64_psa_status_t cy_p64_keys_store_key(cy_p64_key_slot_t key_id, cy_p64_psa_key_handle_t handle)
{
    cy_p64_psa_status_t status = CY_P64_PSA_ERROR_NOT_SUPPORTED;

    uint32_t syscall_cmd[2];
    uint32_t syscall_param[2];

    syscall_cmd[0] = CY_P64_SYSCALL_PSA_CRYPTO_CMD(CY_P64_KS_STORE_KEY_SLOT);
    syscall_cmd[1] = (uint32_t)syscall_param;

    syscall_param[0] = (uint32_t)key_id;
    syscall_param[1] = (uint32_t)handle;

    status = cy_p64_syscall(syscall_cmd);

    return status;
}


/** This function loads key handle from the Secure FlashBoot key storage.
 * Keys in slot range from 1 to #CY_P64_KEY_SLOT_STATIC_MAX are initialized
 * during boot by SFB. They are protected from modification by SFB.
 *
 * \param[in] key_id    The slot number in SFB key storage.
 *                      Reserved keys are defined by CY_P64_KEY_SLOT_XXX define.
 *                      Maximum value - cy_p64_keys_get_count()
 * \param[out] handle   Returned key handle.
 *
 * \retval #CY_P64_PSA_SUCCESS
 *         Success.
 * \retval #CY_P64_PSA_ERROR_DOES_NOT_EXIST
 * \retval #CY_P64_PSA_ERROR_INVALID_ARGUMENT
 * \retval #CY_P64_INVALID_ADDR_PROTECTED
 */
cy_p64_psa_status_t cy_p64_keys_load_key_handle(cy_p64_key_slot_t key_id, cy_p64_psa_key_handle_t *handle)
{
    cy_p64_psa_status_t status = CY_P64_PSA_ERROR_NOT_SUPPORTED;

    uint32_t syscall_cmd[2];
    uint32_t syscall_param[2];

    syscall_cmd[0] = CY_P64_SYSCALL_PSA_CRYPTO_CMD(CY_P64_KS_LOAD_KEY_HANDLE);
    syscall_cmd[1] = (uint32_t)syscall_param;

    syscall_param[0] = (uint32_t)key_id;
    syscall_param[1] = (uint32_t)handle;

    status = cy_p64_syscall(syscall_cmd);

    return status;
}


/** This function destroys the key handle stored in key_id slot
 * and clears handle from Secure FlashBoot key storage.
 *
 * \param[in] key_id    The slot number in SFB key storage.
 *                      Minimum value - #CY_P64_KEY_SLOT_STATIC_MAX + 1.
 *                      Maximum value - cy_p64_keys_get_count()
 *
 * \retval #CY_P64_PSA_SUCCESS
 *         Success.
 * \retval #CY_P64_PSA_ERROR_DOES_NOT_EXIST
 * \retval #CY_P64_PSA_ERROR_INVALID_ARGUMENT
  */
cy_p64_psa_status_t cy_p64_keys_close_key(cy_p64_key_slot_t key_id)
{
    cy_p64_psa_status_t status = CY_P64_PSA_ERROR_NOT_SUPPORTED;

    uint32_t syscall_cmd[2];
    uint32_t syscall_param;

    syscall_cmd[0] = CY_P64_SYSCALL_PSA_CRYPTO_CMD(CY_P64_KS_CLOSE_KEY_HANDLE);
    syscall_cmd[1] = (uint32_t)&syscall_param;

    syscall_param = (uint32_t)key_id;

    status = cy_p64_syscall(syscall_cmd);

    return status;
}


/** This function returns the maximum amount of keys supported by Secure FlashBoot
 *
 * \retval Maximum key count.
  */
uint32_t cy_p64_keys_get_count(void)
{
    cy_p64_psa_status_t status = CY_P64_PSA_ERROR_NOT_SUPPORTED;
    uint32_t keyCount = 0;

    uint32_t syscall_cmd[2];
    uint32_t syscall_param;

    syscall_cmd[0] = CY_P64_SYSCALL_PSA_CRYPTO_CMD(CY_P64_PSA_GET_KEYS_COUNT);
    syscall_cmd[1] = (uint32_t)&syscall_param;

    syscall_param = (uint32_t)&keyCount;

    status = cy_p64_syscall(syscall_cmd);
    if (CY_P64_PSA_SUCCESS != status)
    {
        keyCount = 0;
    }

    return keyCount;
}

/** Sets up a key derivation operation.
 *
 * A key derivation algorithm takes some inputs and uses them to generate
 * a byte stream in a deterministic way.
 * This byte stream can be used to produce keys and other
 * cryptographic material.
 *
 * To derive a key:
 * -# Start with an initialized object of type #cy_p64_psa_key_derivation_operation_t.
 * -# Call cy_p64_psa_key_derivation_setup() to select the algorithm.
 * -# Provide the inputs for the key derivation by calling
 *   CY_P64_PSA_key_derivation_inp_bytes() or cy_p64_psa_key_derivation_input_key()
 *    as appropriate. Which inputs are needed, in what order, and whether
 *    they may be keys and if so of what type depends on the algorithm.
 * -# Optionally set the operation's maximum capacity with
 *   cy_p64_psa_key_derivation_set_capacity(). You may do this before, in the middle
 *    of or after providing inputs. For some algorithms, this step is mandatory
 *    because the output depends on the maximum capacity.
 * -# To derive a key, call cy_p64_psa_key_derivation_output_key().
 *    To derive a byte string for a different purpose, call
 *   cy_p64_psa_key_derivation_out_bytes().
 *    Successive calls to these functions use successive output bytes
 *    calculated by the key derivation algorithm.
 * -# Clean up the key derivation operation object with
 *    cy_p64_psa_key_derivation_abort().
 *
 * If this function returns an error, the key derivation operation object is
 * not changed.
 *
 * If an error occurs at any step after a call to cy_p64_psa_key_derivation_setup(),
 * the operation will need to be reset by a call to cy_p64_psa_key_derivation_abort().
 *
 * Implementations must reject an attempt to derive a key of size 0.
 *
 * \param[in,out] operation       The key derivation operation object
 *                                to set up. It must
 *                                have been initialized but not set up yet.
 * \param alg                     The key derivation algorithm to compute
 *                                (\c CY_P64_PSA_ALG_XXX value such that
 *                                #CY_P64_PSA_ALG_IS_KEY_DERIVATION(\p alg) is true).
 *
 * \retval #CY_P64_PSA_SUCCESS
 *         Success.
 * \retval #CY_P64_PSA_ERROR_INVALID_ARGUMENT
 *         \c alg is not a key derivation algorithm.
 * \retval #CY_P64_PSA_ERROR_NOT_SUPPORTED
 *         \c alg is not supported or is not a key derivation algorithm.
 * \retval #CY_P64_PSA_ERROR_INSUFFICIENT_MEMORY
 * \retval #CY_P64_PSA_ERROR_COMMUNICATION_FAILURE
 * \retval #CY_P64_PSA_ERROR_HARDWARE_FAILURE
 * \retval #CY_P64_PSA_ERROR_CORRUPTION_DETECTED
 * \retval #CY_P64_PSA_ERROR_STORAGE_FAILURE
 * \retval #CY_P64_PSA_ERROR_BAD_STATE
 *         The operation state is not valid (it must be inactive).
 * \retval #CY_P64_PSA_ERROR_BAD_STATE
 *         It is implementation-dependent whether initialize
 *         results fails in this error code
 */
cy_p64_psa_status_t cy_p64_psa_key_derivation_setup(
                                    cy_p64_psa_key_derivation_operation_t *operation,
                                    cy_p64_psa_algorithm_t alg)
{
    cy_p64_psa_status_t status = CY_P64_PSA_ERROR_NOT_SUPPORTED;

    uint32_t syscall_cmd[2];
    uint32_t syscall_param[2];

    syscall_cmd[0] = CY_P64_SYSCALL_PSA_CRYPTO_CMD(CY_P64_PSA_KEY_DERIVATION_SETUP);
    syscall_cmd[1] = (uint32_t)syscall_param;

    syscall_param[0] = (uint32_t)operation;
    syscall_param[1] = (uint32_t)alg;

    status = cy_p64_syscall(syscall_cmd);

    return status;
}

/** Sets up a multipart MAC verification operation.
 *
 * This function sets up the verification of the MAC
 * (message authentication code) of a byte string against an expected value.
 *
 * The sequence of operations to verify a MAC is as follows:
 * -# Allocate an operation object which will be passed to all the functions
 *    listed here.
 * -# Initialize the operation object with one of the methods described in the
 *    documentation for #cy_p64_psa_mac_operation_t, e.g. #CY_P64_PSA_MAC_OPERATION_INIT.
 * -# Call cy_p64_psa_mac_verify_setup() to specify the algorithm and key.
 * -# Call cy_p64_psa_mac_update() zero, one or more times, passing a fragment
 *    of the message each time. The MAC that is calculated is the MAC
 *    of the concatenation of these messages in order.
 * -# At the end of the message, call #cy_p64_psa_mac_verify_finish() to finish
 *    calculating the actual MAC of the message and verify it against
 *    the expected value.
 *
 * If an error occurs at any step after a call to cy_p64_psa_mac_verify_setup(), the
 * operation will need to be reset by a call to cy_p64_psa_mac_abort(). The
 * application may call cy_p64_psa_mac_abort() at any time after the operation
 * has been initialized.
 *
 * After a successful call to cy_p64_psa_mac_verify_setup(), the application must
 * eventually terminate the operation through one of the following methods:
 * - A successful call to #cy_p64_psa_mac_verify_finish().
 * - A call to cy_p64_psa_mac_abort().
 *
 * \param[in,out] operation The operation object to set up. It must have
 *                          been initialized as per the documentation for
 *                          #cy_p64_psa_mac_operation_t and not yet in use.
 * \param handle            Handle to the key to use for the operation.
 *                          It must remain valid until the operation
 *                          terminates.
 * \param alg               The MAC algorithm to compute (\c CY_P64_PSA_ALG_XXX value
 *                          such that #CY_P64_PSA_ALG_IS_MAC(\p alg) is true).
 *
 * \retval #CY_P64_PSA_SUCCESS
 *         Success.
 * \retval #CY_P64_PSA_ERROR_INVALID_HANDLE
 * \retval #CY_P64_PSA_ERROR_NOT_PERMITTED
 * \retval #CY_P64_PSA_ERROR_INVALID_ARGUMENT
 *         \c key is not compatible with \c alg.
 * \retval #CY_P64_PSA_ERROR_NOT_SUPPORTED
 *         \c alg is not supported or is not a MAC algorithm.
 * \retval #CY_P64_PSA_ERROR_INSUFFICIENT_MEMORY
 * \retval #CY_P64_PSA_ERROR_COMMUNICATION_FAILURE
 * \retval #CY_P64_PSA_ERROR_HARDWARE_FAILURE
 * \retval #CY_P64_PSA_ERROR_CORRUPTION_DETECTED
 * \retval #CY_P64_PSA_ERROR_STORAGE_FAILURE
 *         The key could not be retrieved from storage
 * \retval #CY_P64_PSA_ERROR_BAD_STATE
 *         The operation state is not valid (it must be inactive).
 * \retval #CY_P64_PSA_ERROR_BAD_STATE
 *         It is implementation-dependent whether initialize
 *         results fails in this error code
 */
cy_p64_psa_status_t cy_p64_psa_mac_verify_setup(
                                    cy_p64_psa_mac_operation_t *operation,
                                    cy_p64_psa_key_handle_t handle,
                                    cy_p64_psa_algorithm_t alg)
{
    cy_p64_psa_status_t status = CY_P64_PSA_ERROR_NOT_SUPPORTED;

    uint32_t syscall_cmd[2];
    uint32_t syscall_param[3];

    syscall_cmd[0] = CY_P64_SYSCALL_PSA_CRYPTO_CMD(CY_P64_PSA_MAC_VER_SETUP);
    syscall_cmd[1] = (uint32_t)syscall_param;

    syscall_param[0] = (uint32_t)operation;
    syscall_param[1] = (uint32_t)handle;
    syscall_param[2] = (uint32_t)alg;

    status = cy_p64_syscall(syscall_cmd);

    return status;
}

/** Adds a message fragment to a multipart MAC operation.
 *
 * The application must call cy_p64_psa_mac_sign_setup() or cy_p64_psa_mac_verify_setup()
 * before calling this function.
 *
 * If this function returns an error status, the operation enters an error
 * state and must be aborted by calling cy_p64_psa_mac_abort().
 *
 * \param[in,out] operation Active MAC operation.
 * \param[in] input         The buffer that contains the message fragment to add to
 *                          the MAC calculation.
 * \param input_length      Size of the \p input buffer in bytes.
 *
 * \retval #CY_P64_PSA_SUCCESS
 *         Success.
 * \retval #CY_P64_PSA_ERROR_BAD_STATE
 *         The operation state is not valid (it must be active).
 * \retval #CY_P64_PSA_ERROR_INSUFFICIENT_MEMORY
 * \retval #CY_P64_PSA_ERROR_COMMUNICATION_FAILURE
 * \retval #CY_P64_PSA_ERROR_HARDWARE_FAILURE
 * \retval #CY_P64_PSA_ERROR_CORRUPTION_DETECTED
 * \retval #CY_P64_PSA_ERROR_STORAGE_FAILURE
 * \retval #CY_P64_PSA_ERROR_BAD_STATE
 *         It is implementation-dependent whether initialize
 *         results fails in this error code
 */
cy_p64_psa_status_t cy_p64_psa_mac_update(
                                    cy_p64_psa_mac_operation_t *operation,
                                    const uint8_t *input,
                                    size_t input_length)
{
    cy_p64_psa_status_t status = CY_P64_PSA_ERROR_NOT_SUPPORTED;

    uint32_t syscall_cmd[2];
    uint32_t syscall_param[3];

    syscall_cmd[0] = CY_P64_SYSCALL_PSA_CRYPTO_CMD(CY_P64_PSA_MAC_UPDATE);
    syscall_cmd[1] = (uint32_t)syscall_param;

    syscall_param[0] = (uint32_t)operation;
    syscall_param[1] = (uint32_t)input;
    syscall_param[2] = (uint32_t)input_length;

    status = cy_p64_syscall(syscall_cmd);

    return status;
}

/** Finishes the calculation of the MAC of a message and compares it with
 * the expected value.
 *
 * The application must call cy_p64_psa_mac_verify_setup() before calling
 * this function. This function calculates the MAC of the message formed by
 * concatenating the inputs passed to preceding calls to cy_p64_psa_mac_update().
 * It then compares the calculated MAC with the expected MAC passed as a
 * parameter to this function.
 *
 * When this function returns success, the operation becomes inactive.
 * If this function returns an error status, the operation enters an error
 * state and must be aborted by calling cy_p64_psa_mac_abort().
 *
 * \note Implementations shall make the best effort to ensure that the
 * comparison between the actual MAC and the expected MAC is performed
 * in constant time.
 *
 * \param[in,out] operation Active MAC operation.
 * \param[in] mac           Buffer containing the expected MAC value.
 * \param mac_length        Size of the \p mac buffer in bytes.
 *
 * \retval #CY_P64_PSA_SUCCESS
 *         The expected MAC is identical to the actual MAC of the message.
 * \retval #CY_P64_PSA_ERROR_INVALID_SIGNATURE
 *         The MAC of the message was calculated successfully, but it
 *         differs from the expected MAC.
 * \retval #CY_P64_PSA_ERROR_BAD_STATE
 *         The operation state is not valid (it must be an active mac verify
 *         operation).
 * \retval #CY_P64_PSA_ERROR_INSUFFICIENT_MEMORY
 * \retval #CY_P64_PSA_ERROR_COMMUNICATION_FAILURE
 * \retval #CY_P64_PSA_ERROR_HARDWARE_FAILURE
 * \retval #CY_P64_PSA_ERROR_CORRUPTION_DETECTED
 * \retval #CY_P64_PSA_ERROR_STORAGE_FAILURE
 * \retval #CY_P64_PSA_ERROR_BAD_STATE
 *         It is implementation-dependent whether initialize
 *         results fails in this error code
 */
cy_p64_psa_status_t cy_p64_psa_mac_verify_finish(
                                    cy_p64_psa_mac_operation_t *operation,
                                    const uint8_t *mac,
                                    size_t mac_length)
{
    cy_p64_psa_status_t status = CY_P64_PSA_ERROR_NOT_SUPPORTED;

    uint32_t syscall_cmd[2];
    uint32_t syscall_param[3];

    syscall_cmd[0] = CY_P64_SYSCALL_PSA_CRYPTO_CMD(CY_P64_PSA_MAC_VER_FINISH);
    syscall_cmd[1] = (uint32_t)syscall_param;

    syscall_param[0] = (uint32_t)operation;
    syscall_param[1] = (uint32_t)mac;
    syscall_param[2] = (uint32_t)mac_length;

    status = cy_p64_syscall(syscall_cmd);

    return status;
}

#ifdef CY_DEVICE_PSOC6ABLE2
/*******************************************************************************
* Function Name: cy_p64_is_aligned
****************************************************************************//**
* Checks if the address is aligned to a given number.
* The number must be a power of 2 value.
* \param address   The address to be verified.
* \param alignment The alignment value
* \returns
* * True, if the 'address' is aligned to the 'alignment' value.
* * False, otherwise.
*******************************************************************************/
static bool cy_p64_is_aligned(uint32_t address, uint32_t alignment)
{
    bool ret = false;

    if (alignment > 0u)
    {
        if ((address & (alignment - 1u)) == 0u)
        {
            ret = true;
        }
    }
    return (ret);
}
#endif /* CY_DEVICE_PSOC6ABLE2 */

/**
 * This function fills the first \p data_size bytes of the array
 * pointed to by \p dst_addr to the value \p val.
 *
 * \param[in]  dst_addr    Destination memory area, must be aligned to 4
 *                         bytes for CY_DEVICE_PSOC6ABLE2 device
 * \param[in]  val         The value
 * \param[in]  data_size   The size of the data buffer in bytes.
 *
 * \return           Pointer to the destination memory block or NULL on error
 */
cy_p64_psa_status_t cy_p64_psa_memset(void *dst_addr, uint8_t val, size_t data_size)
{
    cy_p64_psa_status_t status = CY_P64_PSA_ERROR_NOT_SUPPORTED;

    uint32_t syscall_cmd[2];
    uint32_t syscall_param[3];

#ifdef CY_DEVICE_PSOC6ABLE2
    if(cy_p64_is_aligned((uint32_t)dst_addr, sizeof(uint32_t)))
#endif /* CY_DEVICE_PSOC6ABLE2 */
    {
        syscall_cmd[0] = CY_P64_SYSCALL_PSA_CRYPTO_CMD(CY_P64_PSA_MEMSET);
        syscall_cmd[1] = (uint32_t)syscall_param;

        syscall_param[0] = (uint32_t)dst_addr;
        syscall_param[1] = (uint32_t)val;
        syscall_param[2] = (uint32_t)data_size;

        status = cy_p64_syscall(syscall_cmd);
    }
#ifdef CY_DEVICE_PSOC6ABLE2
    else
    {
        status = CY_P64_PSA_ERROR_INVALID_ARGUMENT;
    }
#endif /* CY_DEVICE_PSOC6ABLE2 */

    return status;
}

/**
 * This function copies the \p data_size bytes from the memory area pointed
 * by \p src_addr in to the memory area pointed by \p dst_addr.
 *
 * Returns a pointer to the first byte of the out area.
 *
 * \param[in]  dst_addr    Destination memory area, must be aligned to 4
 *                         bytes for CY_DEVICE_PSOC6ABLE2 device
 * \param[in]  src_addr    Source memory area, must be aligned to 4
 *                         bytes for CY_DEVICE_PSOC6ABLE2 device
 * \param[in]  data_size   The size of the data buffer in bytes.
 *
 * \return           The pointer to the destination memory block or NULL on error.
 */
cy_p64_psa_status_t cy_p64_psa_memcpy(void *dst_addr, void const *src_addr, size_t data_size)
{
    cy_p64_psa_status_t status = CY_P64_PSA_ERROR_NOT_SUPPORTED;

    uint32_t syscall_cmd[2];
    uint32_t syscall_param[3];

#ifdef CY_DEVICE_PSOC6ABLE2
    if(cy_p64_is_aligned((uint32_t)dst_addr, sizeof(uint32_t)) &&
       cy_p64_is_aligned((uint32_t)src_addr, sizeof(uint32_t)))
#endif /* CY_DEVICE_PSOC6ABLE2 */
    {
        syscall_cmd[0] = CY_P64_SYSCALL_PSA_CRYPTO_CMD(CY_P64_PSA_MEMCPY);
        syscall_cmd[1] = (uint32_t)syscall_param;

        syscall_param[0] = (uint32_t)dst_addr;
        syscall_param[1] = (uint32_t)src_addr;
        syscall_param[2] = (uint32_t)data_size;

        status = cy_p64_syscall(syscall_cmd);
    }
#ifdef CY_DEVICE_PSOC6ABLE2
    else
    {
        status = CY_P64_PSA_ERROR_INVALID_ARGUMENT;
    }
#endif /* CY_DEVICE_PSOC6ABLE2 */

    return status;
}
