/***************************************************************************//**
* \file cy_p64_jwt_policy.h
* \version 1.0
*
* \brief
* This is the header file for the JWT policy parsing and processing.
*
********************************************************************************
* \copyright
* Copyright 2021, Cypress Semiconductor Corporation (an Infineon company).
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


/** Error codes for policy processing functions */
#define CY_P64_JWT_ERR_JKEY_NOT_FOUND           (0xF8000001U)
#define CY_P64_JWT_ERR_JSN_NONOBJ               (0xF8000002U)
#define CY_P64_JWT_ERR_JSN_WRONG_TYPE           (0xF8000003U)
#define CY_P64_JWT_ERR_JWT_PACKET_PARSE         (0xF8000004U)
#define CY_P64_JWT_ERR_JSN_BIG_NESTING          (0xF8000005U)
#define CY_P64_JWT_ERR_JSN_PARSE_FAIL           (0xF8000006U)
#define CY_P64_JWT_ERR_B64DECODE_FAIL           (0xF8000007U)
#define CY_P64_JWT_ERR_JWT_TOO_BIG              (0xF8000008U)
#define CY_P64_JWT_ERR_JWT_BROKEN_FORMAT        (0xF8000009U)
#define CY_P64_JWT_ERR_MALLOC_FAIL              (0xF800000AU)
#define CY_P64_JWT_ERR_OTHER                    (0xF800000BU)
#define CY_P64_JWT_ERR_INVALID_PARAMETER        (0xF800000CU)


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
