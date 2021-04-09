/***************************************************************************//**
* \file cy_p64_syscalls.h
* \version 1.0
*
* \brief
* This is the header file for syscall functions.
*
********************************************************************************
* \copyright
* Copyright 2021, Cypress Semiconductor Corporation (an Infineon company).
* All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#ifndef CY_P64_SYSCALLS_H
#define CY_P64_SYSCALLS_H

#include <stdint.h>
#include "cy_device.h"
#include "cy_utils.h"
#include "cy_p64_syscall.h"

#define CY_P64_CM4_ROM_LOOP_ADDR         (0x16004000U)

#define CY_P64_SRSS_TEST_MODE_ADDR       (SRSS_BASE | 0x0100U)
#define CY_P64_TEST_MODE_MASK            (0x80000000U)

/** \addtogroup syscalls_macros
 * \{
 */

/** Verifies if TEST_MODE bit is set in the SRSS_TST_MODE register*/
#define CY_P64_IS_TEST_MODE_SET          ((CY_GET_REG32(CY_P64_SRSS_TEST_MODE_ADDR) & CY_P64_TEST_MODE_MASK) != 0U)

/* IDs of the parts of the provisioning packet for cy_p64_get_provisioning_details() API */
/** Invalid key slot number */
#define CY_P64_KEY_SLOT_NA              (0U)
/** Key slot for: Device Private Key ECDH */
#define CY_P64_KEY_SLOT_DEVICE_ECDH     (1U)
/** Key slot for: Device Private Key ECDSA */
#define CY_P64_KEY_SLOT_DEVICE_ECDSA    (2U)
/** Key slot for: Cypress Public Key */
#define CY_P64_KEY_SLOT_CYPRESS         (3U)
/** Key slot for: HSM Public Key */
#define CY_P64_KEY_SLOT_HSM             (4U)
/** Key slot for: OEM Public Key */
#define CY_P64_KEY_SLOT_OEM             (5U)
/** Key slot for: provisioned Custom Public Key #1 */
#define CY_P64_KEY_SLOT_CUSTOM_1        (6U)
/** Key slot for: provisioned Custom Public Key #2 */
#define CY_P64_KEY_SLOT_CUSTOM_2        (7U)
/** Key slot for: provisioned Custom Public Key #3 */
#define CY_P64_KEY_SLOT_CUSTOM_3        (8U)
/** Key slot for: provisioned Custom Public Key #4 */
#define CY_P64_KEY_SLOT_CUSTOM_4        (9U)
/** Key slot for: provisioned Custom Public Key #5 */
#define CY_P64_KEY_SLOT_CUSTOM_5        (10U)
/** Key slot for: AES 256-bit key derived from 128-bit UDS for Key Derivation by
 * cy_p64_psa_key_derivation_key_agreement() API */
#define CY_P64_KEY_SLOT_DERIVE          (11U)
/** Key slot for: Device Group Key for ECDH Key agreement by
 * cy_p64_psa_key_derivation_key_agreement() API. */
#define CY_P64_KEY_SLOT_DEVICE_GRP_ECDH (12U)
/* Key slot 13 to 16 are reserved for SFB */
/** The number of keys used by SFB PSA Crypto */
#define CY_P64_KEY_SLOT_STATIC_MAX      (16U)
/* Key slot 16 to 32 are available for user application */

/** JWT Policy */
#define CY_P64_POLICY_JWT               (0x100U)
/** Boot policy template */
#define CY_P64_POLICY_TEMPL_BOOT        (0x101U)
/** Debug policy template */
#define CY_P64_POLICY_TEMPL_DEBUG       (0x102U)
/** Get Certificate command, add a certificate index in the "chain_of_trust" array
 * of the provisioned packet to this macro to get specific certificate */
#define CY_P64_POLICY_CERTIFICATE       (0x200U)
/** Image certificate */
#define CY_P64_POLICY_IMG_CERTIFICATE   (0x300U)

/** \} */

/** \addtogroup syscalls_t
 * \{
 */

/** cy_p64_access_port_control() parameter: access port control */
typedef enum {
    /** Access port disable */
    CY_P64_AP_DIS = 0,
    /**  Access port enable */
    CY_P64_AP_EN = 1
} cy_p64_ap_control_t;

/** cy_p64_access_port_control() parameter: access port name */
typedef enum {
    /** CM0 access port */
    CY_P64_CM0_AP = 0,
    /** CM4 access port */
    CY_P64_CM4_AP = 1,
    /** System access port */
    CY_P64_SYS_AP = 2
} cy_p64_ap_name_t;

/** \} */

/* Public APIs */
cy_p64_error_codes_t cy_p64_get_provisioning_details(uint32_t id, char **ptr, uint32_t *len);
cy_p64_error_codes_t cy_p64_access_port_control(cy_p64_ap_name_t ap, cy_p64_ap_control_t control);
cy_p64_error_codes_t cy_p64_acquire_response(void);
void cy_p64_acquire_test_bit_loop(void);
cy_p64_error_codes_t cy_p64_get_rollback_counter(uint32_t number, uint32_t *value);
cy_p64_error_codes_t cy_p64_update_rollback_counter(uint32_t number, uint32_t value);

#ifndef CY_DEVICE_PSOC6A512K
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
                                        uint32_t *sign_addr);
#endif /* !CY_DEVICE_PSOC6A512K */

#endif /* CY_P64_SYSCALLS_H */
