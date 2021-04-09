/***************************************************************************//**
* \file cy_p64_syscall.h
* \version 1.0
*
* \brief
* This is the header file for low-level syscall functions.
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
* \mainpage
********************************************************************************
*
*
********************************************************************************
* \section section_p64_misra MISRA-C Compliance
********************************************************************************
*
* This section describes the MISRA-C:2012 compliance and deviations for the
* PSoC64 Secure Boot Utilities.
*
* <table class="doxtable">
*   <tr>
*     <th>MISRA Rule</th>
*     <th>Rule Class (Required/Advisory)</th>
*     <th>Rule Description</th>
*     <th>Description of Deviation(s)</th>
*   </tr>
*   <tr>
*     <td>11.3</td>
*     <td>R</td>
*     <td> A cast shall not be performed between a pointer to object type and
*         a pointer to a different object type.</td>
*     <td>The pointer casting is reviewed and the result pointer is correctly
*         aligned.</td>
*   </tr>
*   <tr>
*     <td>11.6</td>
*     <td>R</td>
*     <td>A cast shall not be performed between pointer to void and an
*         arithmetic type.</td>
*     <td> The pointer casting is reviewed and the result pointer is correctly
*          aligned.</td>
* </table>
*
* Only mandatory and required deviations are documented.
* This middleware has the following open source libraries which are not checked
* for MISRA compliance: cJSON, base64.
*
*******************************************************************************/

#ifndef CY_P64_SYSCALL_H
#define CY_P64_SYSCALL_H

#include <stdint.h>

/** \addtogroup syscall_t
 * \{
 */

/**
 * \brief Functions return status.
 *
 * This is either #CY_P64_SUCCESS, indicating success,
 * or other value indicating that an error occurred. Errors are
 * encoded as one of the \c CY_P64_INVALID_xxx values defined here.
 */
typedef uint32_t cy_p64_error_codes_t;

/** \} */


/** \addtogroup syscall_macros
 * \{
 */

/** SysCall parameters types */
#define CY_P64_SYSCALL_DIRECT_PARAMS     (0x1UL)

/** The status mask of the Secure FlashBoot return value */
#define CY_P64_SYSCALL_MASK              (0xFF000000U)

/** Success operation */
#define CY_P64_SUCCESS                   (0xA0000000U)
/** The fail status of the Secure FlashBoot return value */
#define CY_P64_INVALID                   (0xF7000000U)
/** Reject the system call when CPUSS_PROTECTION is not NORMAL (PSoC6A-BLE2 only) */
#define CY_P64_INVALID_PROTECTION        (0xF0000001U)
/** Returned by all APIs when client doesn't have access to region it is using for passing arguments. */
#define CY_P64_INVALID_ADDR_PROTECTED    (0xF0000008U)
/** The opcode is not a valid API opcode. */
#define CY_P64_INVALID_SYSCALL_OPCODE    (0xF000000BU)
/** Returned when device is in DEAD state */
#define CY_P64_INVALID_STATE_DEAD        (0xF700DEADU)
/** Returned when write to flash operation fails */
#define CY_P64_INVALID_FLASH_OPERATION   (0xF7000002U)
/** Returned if a master with PC > 4 tries to use PSA syscall and protection is enabled in the policy ((protect_flags & 2) != 0) */
#define CY_P64_INVALID_SYSCALL_PROTECTED (0xF700000AU)
/** Returned when Protection Context change of the Crypto is failed */
#define CY_P64_INVALID_PC_CHANGE         (0xF7000010U)
/** Returned by cy_p64_access_port_control() when requested action (enable DP) is not permitted by the provisioned policy */
#define CY_P64_INVALID_PERM_NOT_ALLOWED  (0xF7000012U)
/** Returned when cryptographic operation failed */
#define CY_P64_INVALID_CRYPTO_OPER       (0xF7000013U)
/** Returned if invalid arguments are passed to a API or SysCall */
#define CY_P64_INVALID_ARGUMENT          (0xF7000024U)
/** Returned if a master with PC > 4 tries to use cy_p64_psa_sign_hash() API with an internal key and protection is enabled in the policy ((protect_flag & 1) != 0) */
#define CY_P64_INVALID_KEY_PROTECTED     (0xF7000025U)
/** Returned if API tries to access not permitted regions of the memory according to the policy */
#define CY_P64_INVALID_ADDR_OUT_OF_RANGE (0xF7000026U)
/** Returned when memory allocation failed */
#define CY_P64_INVALID_MEM_ALLOC         (0xF70000FFU)

/** Returned when syscall didn't respond until defined timeout */
#define CY_P64_INVALID_TIMEOUT           (0xF8000100U)
/** Returned when invalid output parameter passed to API */
#define CY_P64_INVALID_OUT_PAR           (0xF8000101U)

/** \} */

/* Public APIs */
cy_p64_error_codes_t cy_p64_syscall(uint32_t *cmd);


#endif /* CY_P64_SYSCALL_H */
