/***************************************************************************//**
* \file cy_p64_jwt_policy.h
* \version 1.0.1
*
* \brief
* This is the header file for the JWT policy parsing and processing.
*
********************************************************************************
* \copyright
* Copyright 2021-2022, Cypress Semiconductor Corporation (an Infineon company).
* All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#ifndef CY_P64_JWT_POLICY_H
#define CY_P64_JWT_POLICY_H

#include <stdint.h>
#include <stdbool.h>

#include "cy_p64_syscall.h"
#include "cy_p64_cJSON.h"

/** \addtogroup jwt_policy_error
 * @{
 */
/* Error codes for policy processing functions */
/** JSON does not contain requested item */
#define CY_P64_JWT_ERR_JSN_NONOBJ               (0xF8000002U)
/** Invalid type of JSON object */
#define CY_P64_JWT_ERR_JSN_WRONG_TYPE           (0xF8000003U)
/** JSON parse has failed */
#define CY_P64_JWT_ERR_JSN_PARSE_FAIL           (0xF8000006U)
/** Base64 decoding of JWT packet body has failed */
#define CY_P64_JWT_ERR_B64DECODE_FAIL           (0xF8000007U)
/** Invalid JWT packet format
*
* This error is returned if header and/or signature has not been found in JWT packet. */
#define CY_P64_JWT_ERR_JWT_BROKEN_FORMAT        (0xF8000009U)
/** Memory allocation for JSON string has failed */
#define CY_P64_JWT_ERR_MALLOC_FAIL              (0xF800000AU)
/** An error occurred that does not correspond to any defined failure cause */
#define CY_P64_JWT_ERR_OTHER                    (0xF800000BU)
/** The parameters passed to the function are invalid */
#define CY_P64_JWT_ERR_INVALID_PARAMETER        (0xF800000CU)
/**@}*/

/* Public API */
cy_p64_error_codes_t cy_p64_decode_payload_data(const char *jwt_packet, cy_p64_cJSON **json_packet);
const cy_p64_cJSON *cy_p64_find_json_item(const char *path, const cy_p64_cJSON *json);
cy_p64_error_codes_t cy_p64_json_get_boolean(const cy_p64_cJSON *json, bool *value);
cy_p64_error_codes_t cy_p64_json_get_uint32(const cy_p64_cJSON *json, uint32_t *value);
cy_p64_error_codes_t cy_p64_json_get_string(const cy_p64_cJSON *json, const char **value);
cy_p64_error_codes_t cy_p64_json_get_array_uint8(const cy_p64_cJSON *json, uint8_t *buf, uint32_t size, uint32_t *olen);
cy_p64_error_codes_t cy_p64_policy_get_image_record(
    const cy_p64_cJSON *json,
    uint32_t image_id,
    const cy_p64_cJSON **json_image);
cy_p64_error_codes_t cy_p64_policy_get_image_address_and_size(
    const cy_p64_cJSON *json,
    uint32_t image_id,
    const char *image_type,
    uint32_t *address,
    uint32_t *size);
cy_p64_error_codes_t cy_p64_policy_get_image_boot_config(
    const cy_p64_cJSON *json,
    uint32_t image_id,
    bool *wdt_enable,
    uint32_t *wdt_timeout,
    bool *set_img_ok);

#endif /* CY_P64_JWT_POLICY_H */
