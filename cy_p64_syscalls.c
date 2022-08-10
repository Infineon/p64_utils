/***************************************************************************//**
* \file cy_p64_syscalls.c
* \version 1.0.1
*
* \brief
* This is the source code file for syscall functions.
*
********************************************************************************
* \copyright
* Copyright 2021-2022, Cypress Semiconductor Corporation (an Infineon company).
* All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

/*******************************************************************************
* Syscalls Prototypes
****************************************************************************//**
*
* \defgroup syscalls     Syscalls
*
* \brief
*  This library implement syscalls functionality
*
* \{
*   \defgroup syscalls_api Functions
*   \defgroup syscalls_macros Macros
*   \defgroup syscalls_t Data Structures
* \}
*******************************************************************************/

#include <string.h>
#include "cy_p64_syscalls.h"
#include "cy_p64_jwt_policy.h"


/** AcquireResponse Syscall opcode */
#define CY_P64_SYSCALL_OPCODE_ACQUIRE_RESP          (0x32UL << 24U)
/** PSA crypto SysCall opcode */
#define CY_P64_SYSCALL_OPCODE_PSA_CRYPTO            (0x35UL << 24U)
/** Roll Back counter SysCall opcode */
#define CY_P64_SYSCALL_OPCODE_ROLL_BACK_COUNTER     (0x36UL << 24U)
/** Get Provision details SysCall opcode */
#define CY_P64_SYSCALL_OPCODE_GET_PROV_DETAILS      (0x37UL << 24U)
/** DAP Control SysCall opcode */
#define CY_P64_SYSCALL_OPCODE_DAP_CONTROL           (0x3AUL << 24U)
/** Attestation SysCall opcode */
#define CY_P64_SYSCALL_OPCODE_ATTESTATION           (0x3CUL << 24U)

#define CY_P64_ACQUIRE_RESP_CLEAR                   (0UL)
#define CY_P64_ACQUIRE_RESP_SET                     (1UL)

#define CY_P64_ROLL_BACK_COUNTER_READ               (0UL)
#define CY_P64_ROLL_BACK_COUNTER_WRITE              (1UL)


/*******************************************************************************
* Function Name: cy_p64_get_certificate
****************************************************************************//**
* Parses chain_of_trust token and returns pointer to particular certificate.
* Note that function allocates buffer for parsed token when cert_buff != NULL and
*  free it after following function call with input parameters equal to NULL.
*
* \param chain_of_trust JSON provisioning packet (0 terminated).
* \param cert_buff      The buffer where certificate will be copied.
* \param cert_len       The length of certBuffLen buffer.
* \param certificate_id ID of the requested certificate.
* \retval #CY_P64_SUCCESS
* \retval #CY_P64_JWT_ERR_JSN_PARSE_FAIL
* \retval #CY_P64_JWT_ERR_JSN_NONOBJ
*
*******************************************************************************/
static cy_p64_error_codes_t cy_p64_get_certificate(const char *chain_of_trust, char **cert_buff, uint32_t *cert_len, uint32_t certificate_id)
{
    cy_p64_error_codes_t rc = CY_P64_JWT_ERR_JSN_PARSE_FAIL;
    static cy_p64_cJSON *json_parent = NULL;
    cy_p64_cJSON *json;
    char *cert_str = NULL;

    /* Free previously allocated buffers */
    if(json_parent != NULL)
    {
        cy_p64_cJSON_Delete(json_parent);
        json_parent = NULL;
    }

    if((cert_buff == NULL) && (cert_len == NULL))
    {
        rc = CY_P64_SUCCESS;    /* Just free static buffer without parsing */
    }
    else
    {
        json_parent = cy_p64_cJSON_Parse(chain_of_trust);
        if(json_parent != NULL)
        {
            json = json_parent;
            if(json->type == CY_P64_cJSON_Array)
            {
                json = json->child;
                while((certificate_id > 0u) && (json != NULL))
                {
                    json = json->next;
                    certificate_id--;
                }
            }
            if((json != NULL) && (certificate_id == 0u))
            {
                if(json->type == CY_P64_cJSON_String)
                {
                    cert_str = json->valuestring;
                }
            }
            if(cert_str != NULL)
            {
                if(cert_len != NULL)
                {
                    *cert_len = strlen(cert_str);
                }
                if(cert_buff != NULL)
                {
                    *cert_buff = cert_str;
                }
                rc = CY_P64_SUCCESS;
            }
            else
            {
                rc = CY_P64_JWT_ERR_JSN_NONOBJ;
            }
            /* Free json_parent only when response string is not requested */
            if(cert_buff == NULL)
            {
                cy_p64_cJSON_Delete(json_parent);
                json_parent = NULL;
            }
        }
    }

    return rc;
}


/** \addtogroup syscalls_api
 * \{
 */

/*******************************************************************************
* Function Name: cy_p64_get_provisioning_details
****************************************************************************//**
*
* Reads the provisioning packet (JWT), policy templates or public keys
*  strings in the JSON format.
*
* \param[in] id: Item id (provisioning packet, templates or public keys):
*             * 0 to 32 - Key slot in SFB Mbed Crypto Key Storage
*             * 0x100 - CY_P64_POLICY_JWT
*             * 0x101 - CY_P64_POLICY_TEMPL_BOOT
*             * 0x102 - CY_P64_POLICY_TEMPL_DEBUG
*             * 0x2xx - CY_P64_POLICY_CERTIFICATE, where xx is a certificate index in
*                       the "chain_of_trust" array of the provisioned packet.
*                       Note that when the function is called with this parameter it allocates buffer for the certificate.
*                       the buffer is free/reused on the following call for read certificate.
*                       To free the buffer explicitly call this function again with ptr=NULL and len=NULL.
*             * 0x300 - CY_P64_POLICY_IMG_CERTIFICATE
* \param[out] ptr: The pointer to the response string. Can be NULL to read 'len' only.
* \param[out] len: The length of the response string.
*
* \return     \ref CY_P64_SUCCESS for success or error code
*
*******************************************************************************/
cy_p64_error_codes_t cy_p64_get_provisioning_details(uint32_t id, char **ptr, uint32_t *len)
{
    cy_p64_error_codes_t status = CY_P64_INVALID;
    uint32_t syscall_cmd[2];
    uint32_t syscall_param[2];
    uint32_t sfb_ver = _FLD2VAL(CY_P64_SFB_VERSION, CY_GET_REG32(CY_P64_SFB_VERSION_ADDR));

    if(((id & ~CY_P64_POLICY_CERT_INDEX_MASK) == CY_P64_POLICY_CERTIFICATE) &&
       (sfb_ver == CY_P64_SFB_VERSION_RELEASE))
    {
        /* Workaround for memory leakage in the released version of Secure FlashBoot which is fixed in the following version */
        status = cy_p64_get_certificate((const char *)CY_P64_CHAIN_OF_TRUST_ADDR, ptr, len, id & CY_P64_POLICY_CERT_INDEX_MASK);
    }
    else
    {
        syscall_cmd[0] = CY_P64_SYSCALL_OPCODE_GET_PROV_DETAILS;
        syscall_cmd[1] = (uint32_t)syscall_param;

        syscall_param[0] = id;
        syscall_param[1] = 0U;

        status = cy_p64_syscall(syscall_cmd);

        if(status == CY_P64_SUCCESS)
        {
            if(ptr != NULL)
            {
                *ptr = (char *)syscall_param[1];
            }
            if(len != NULL)
            {
                *len = syscall_param[0];
            }
        }
    }

    return status;
}


/*******************************************************************************
* Function Name: cy_p64_access_port_control
****************************************************************************//**
*
* Allows the user to control DAP access during run-time.
* It works only when particular Debug Access Port has permission set to "allowed"
*  and control field is set to "open" in Debug policy.
*
* \param[in] ap:         Access port name
* \param[in] control:    Control value.
*
* \return    \ref CY_P64_SUCCESS for success or error code
*
*******************************************************************************/
cy_p64_error_codes_t cy_p64_access_port_control(cy_p64_ap_name_t ap, cy_p64_ap_control_t control)
{
    cy_p64_error_codes_t status = CY_P64_INVALID;
    uint32_t syscall_cmd;

    syscall_cmd = CY_P64_SYSCALL_OPCODE_DAP_CONTROL |
                  ((uint32_t)control << 16U) |
                  ((uint32_t)ap <<  8U) |
                  CY_P64_SYSCALL_DIRECT_PARAMS;

    status = cy_p64_syscall(&syscall_cmd);

    return status;
}


/*******************************************************************************
* Function Name: cy_p64_acquire_response
****************************************************************************//**
*
* Call this function only when TEST_MODE bit is set in the
* SRSS_TST_MODE register, use CY_P64_IS_TEST_MODE_SET() macro to check.
* It sends acquire response, i.e. sets a magic number
* in the protected RAM by calling a syscall. The code after this function should
* wait until TEST_MODE is cleared by the debugger, call
* cy_p64_acquire_test_bit_loop().
* Acquire procedure is described in PSoC 64 Programming Specification
* 002-31353 rev** section 5.3.
*
* \return    \ref CY_P64_SUCCESS for success or error code
*
*******************************************************************************/
cy_p64_error_codes_t cy_p64_acquire_response(void)
{
    cy_p64_error_codes_t status = CY_P64_INVALID;
    uint32_t syscall_cmd;

    syscall_cmd = CY_P64_SYSCALL_OPCODE_ACQUIRE_RESP |
                  (CY_P64_ACQUIRE_RESP_SET << 8U) |
                  CY_P64_SYSCALL_DIRECT_PARAMS;

    status = cy_p64_syscall(&syscall_cmd);

    return status;
}


/*******************************************************************************
* Function Name: cy_p64_acquire_test_bit_loop
****************************************************************************//**
*
* This function should be called after cy_p64_acquire_response(). It is executed
* from SRAM and waits until TEST_MODE bit is cleared in the SRSS_TST_MODE
* register by the debugger.
* The acquire procedure is described in PSoC 64 Programming Specification
* 002-31353 rev** section 5.3.
*
*******************************************************************************/
CY_RAMFUNC_BEGIN
#if !defined (__ICCARM__)
    CY_NOINLINE
#endif
void cy_p64_acquire_test_bit_loop(void)
{
    while(CY_P64_IS_TEST_MODE_SET)
    {
        /* Wait until TEST_MODE bit is cleared */
    }
}
CY_RAMFUNC_END


/*******************************************************************************
* Function Name: cy_p64_get_rollback_counter
****************************************************************************//**
*
* This function reads the rollback counter.
*
* \param[in] number:    Rollback counter number (0-15).
* \param[out] *value:   The pointer to the read value.

* \return    \ref CY_P64_SUCCESS for success or error code
*
*******************************************************************************/
cy_p64_error_codes_t cy_p64_get_rollback_counter(uint32_t number, uint32_t *value)
{
    cy_p64_error_codes_t status = CY_P64_INVALID;
    uint32_t syscall_cmd[2];
    uint32_t syscall_param;

    if(value == NULL)
    {
        status = CY_P64_INVALID_OUT_PAR;
    }
    else
    {
        syscall_cmd[0] = CY_P64_SYSCALL_OPCODE_ROLL_BACK_COUNTER |
                         (number << 16U) |
                         (CY_P64_ROLL_BACK_COUNTER_READ << 8U);
        syscall_cmd[1] = (uint32_t)&syscall_param;

        status = cy_p64_syscall(syscall_cmd);

        if(status == CY_P64_SUCCESS)
        {
            *value = syscall_param;
        }
    }

    return status;
}


/*******************************************************************************
* Function Name: cy_p64_update_rollback_counter
****************************************************************************//**
*
* Updates the rollback counter to a higher value only. This syscall is used by
* Bootloader to prevent firmware reversion during firmware update.
*
* \param[in] number:    Rollback counter number (0-15).
* \param[in] value:     A new value.

* \return    \ref CY_P64_SUCCESS for success or error code
*
*******************************************************************************/
cy_p64_error_codes_t cy_p64_update_rollback_counter(uint32_t number, uint32_t value)
{
    cy_p64_error_codes_t status = CY_P64_INVALID;
    uint32_t syscall_cmd[2];
    uint32_t syscall_param;

    syscall_cmd[0] = CY_P64_SYSCALL_OPCODE_ROLL_BACK_COUNTER |
                     (number << 16U) |
                     (CY_P64_ROLL_BACK_COUNTER_WRITE << 8U);
    syscall_cmd[1] = (uint32_t)&syscall_param;

    syscall_param = value;

    status = cy_p64_syscall(syscall_cmd);

    return status;
}

#ifndef CY_DEVICE_PSOC6A512K


/*******************************************************************************
* Function Name: cy_p64_attestation
****************************************************************************//**
*
* Calculates hashes of memory regions provided in an input array.
* Calculates the signature of a certificate that attests the device state at the
* moment of signing.
* Signature is calculated for the following data structure:
* *   Server random number (uint32_t)
* *   Syscall random number (uint32_t)
* *   Device UID (SFLASH->DIE_LOT array, 11 bytes)
* *   Device Identity (cy_flashDeviceKeyData array, 512 bytes)
* *   OEM Public key and Product ID (cy_flashProvKeyData array, 512 bytes)
* *   Chain of trust (cy_flashChainOfTrust array, 5 kbytes)
* *   Image certificate (cy_flashImgCertJWT array, 1 kbytes)
* *   Policy package (cy_flashProvisionJWT array, 10 kbytes)
* *   Number of memory regions (uint32_t)
* *   for (each memory region):
*   *   Memory region address (uint32_t)
*   *   Memory region size (uint32_t)
*   *   Memory region content (uint8_t array)
*   *   Memory region hash (calculated with the syscall random number at the
*         beginning) (uint8_t array)
*
* The signature is calculated without any additional padding / aligning between
* different certificate fields.
* Memory regions can be from SRAM, Flash, WFlash, SFlash. SMIF and peripheral
* address space is not supported.
*  AttestationSysCall algorithm:
*   Generate syscall random number.
*   Init signature hash calculation.
*   Update signature hash with Server/Syscall random numbers, DevUID, Dev public key, policy package.
*   Update signature hash with Number of regions.
*   Check whether the array with memory regions has at least read access allowed for the caller
*       (to avoid side channel attacks)
*   for (each specified memory region) {
*       Start region hash calculation with the syscall random number.
*       Update signature and region hash with a region content (in parallel using the same data).
*       Update signature hash with the region hash.
*       Verify writing rights of the caller and sufficient memory size.
*       Output the region hash to corresponding memory.
*   }
*   Signs the signature hash with a device private key.
*   Clears Crypto Block internal memories and used stack.
*
* An example of real life usage: There is a server which knows information about all
* connected devices in the field, their IDs, Public keys, available versions of FW.
* The server wants to know exact state of one of the devices - whether it is hacked
* or not, which FW versions it has, which configuration. The server sends a request
* to the device and asks to send back a signed certificate with hashes and content of
* specific memory regions. The application (through SPM service) calls the SysCall which
* calculates hashes of the specified regions and create a signature of a data certificate
* with specific structure. SPM code creates a package with all needed data, includes their
* calculated hashes and signature, and returns it to Application. Application sends it back to server.
*
* This syscall is not available for PSoC64_512K device
*
* \param[in] sign_alg:  PSA signing algorithm (contains hash algorithm type used
*                       both for signature and memory region hashes).  Only
*                       PSA_ALG_ECDSA(PSA_ALG_SHA_256) is allowed at the moment
* \param[in] rnd:  A random number from server
* \param[in] mem_count: The number of memory regions in the array.
* \param[in] mem_start_addr: The pointer to array of memory region start addresses
* \param[in] mem_sizes: The pointer to array of memory region sizes
* \param[in] hash_addr: The pointer to array for hashes of the memory regions
* \param[in] hash_size: The size in bytes of the array for hashes
* \param[out] rnd_out: A random number from syscall.
* \param[out] mem_hash_size: The size in bytes of each memory region Hash.
* \param[out] sign_size: The size in bytes of the signature.
* \param[out] sign_addr: The address where the signature is stored.
*
* \return     \ref CY_P64_SUCCESS for success or error code
*
*******************************************************************************/
cy_p64_error_codes_t cy_p64_attestation(uint32_t sign_alg,
                                        uint32_t rnd,
                                        uint32_t mem_count,
                                        const uint32_t *mem_start_addr,
                                        const uint32_t *mem_sizes,
                                        const uint32_t *hash_addr,
                                        uint32_t hash_size,
                                        uint32_t *rnd_out,
                                        uint32_t *mem_hash_size,
                                        uint32_t *sign_size,
                                        uint32_t *sign_addr)
{
    cy_p64_error_codes_t status = CY_P64_INVALID;
    uint32_t syscall_cmd[2];
    uint32_t syscall_in_param[7];

    if((rnd_out == NULL) || (mem_hash_size == NULL) ||
       (sign_size == NULL) || (sign_addr == NULL))
    {
        status = CY_P64_INVALID_OUT_PAR;
    }
    else
    {
        syscall_cmd[0] = CY_P64_SYSCALL_OPCODE_ATTESTATION;
        syscall_cmd[1] = (uint32_t)syscall_in_param;

        syscall_in_param[0] = sign_alg;
        syscall_in_param[1] = rnd;
        syscall_in_param[2] = mem_count;
        syscall_in_param[3] = (uint32_t)mem_start_addr;
        syscall_in_param[4] = (uint32_t)mem_sizes;
        syscall_in_param[5] = (uint32_t)hash_addr;
        syscall_in_param[6] = hash_size;

        status = cy_p64_syscall(syscall_cmd);

        if(status == CY_P64_SUCCESS)
        {
            const uint32_t *syscall_out_param = (uint32_t *)syscall_cmd[1];

            *rnd_out = syscall_out_param[0];
            *mem_hash_size = syscall_out_param[1];
            *sign_size = syscall_out_param[2];
            *sign_addr = syscall_out_param[3];
        }
    }
    return status;
}

#endif /* !CY_DEVICE_PSOC6A512K */

/** \} */
