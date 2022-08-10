/***************************************************************************//**
* \file cy_p64_jwt_policy.c
* \version 1.0.1
*
* \brief
* This is the source code for the JWT policy parsing and processing.
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
* JWT Policy Prototypes
****************************************************************************//**
*
* \defgroup jwt_policy     JWT/JSON Policy parsing
*
* \brief
*  This library implement functions to:
*       - Decode and parse the JWT packet
*       - Find items in the JSON object
*       - Find the boot and upgrade the image address in the provisioning policy
*
* \{
*   \defgroup jwt_policy_api Functions
    \defgroup jwt_policy_error Error codes
* \}
*******************************************************************************/

#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>
#include <errno.h>

#include "cy_syslib.h"
#include "cy_ipc_drv.h"

#include "cy_p64_base64.h"
#include "cy_p64_malloc.h"
#include "cy_p64_jwt_policy.h"


#define CY_P64_MAX_PATH_LEN     (80)


/*******************************************************************************
* Function Name: cy_p64_get_jwt_data_body
****************************************************************************//**
* Looks over a given JWT packet, finds where the JWT body is located
* and its length.
*
* \param[in] jwt_str  The JWT packet to analyze.
* \param[out] body_ptr The pointer to the JWT body section.
* \param[out] body_len The length of the JWT body section.
*
* \retval #CY_P64_SUCCESS
* \retval #CY_P64_JWT_ERR_JWT_BROKEN_FORMAT
*******************************************************************************/
static cy_p64_error_codes_t cy_p64_get_jwt_data_body(const char *jwt_str, const char **body_ptr, uint32_t *body_len)
{
    cy_p64_error_codes_t ret = CY_P64_SUCCESS;
    const char *ptr_start;
    const char *ptr_end;

    ptr_start = strchr(jwt_str, (int)'.');
    if(ptr_start == NULL)
    {
        ret = CY_P64_JWT_ERR_JWT_BROKEN_FORMAT;
    }
    else
    {
        ptr_end = strchr(ptr_start + 1, (int)'.');
        if(ptr_end == NULL)
        {
            ret = CY_P64_JWT_ERR_JWT_BROKEN_FORMAT;
        }
        else
        {
            *body_len = ((uint32_t)ptr_end - (uint32_t)ptr_start) - 1U;
            *body_ptr = ptr_start + 1;
        }
    }
    return ret;
}


/*******************************************************************************
* Function Name: cy_p64_path_get_next_name_index
****************************************************************************//**
* \warning  CAUTION This function is designed to be used by cy_p64_find_json_item()
*           only, due to the use of internal static buffer - 'orig_path'.
*           So the caller to cy_policy_get_<type> could simply pass in
*           "const char *" as parameter.
* \note
*           - Do NOT remove the "static" attribute
*           - Do NOT call this function
*           - It's not re-entrant and it can only support one caller
*             at a time due to a common static buffer.
*
* \brief Get the next token(name) & index from the path
*        e.g.: "path:2/to/the:1/node" --> "path" & 2
*
* \param[in]   in_path  The path of the policy node.
*                       NULL: find the next token (from previous call)
* \param[out]  name     The name of the next node.
* \param[out]  idx      The index of the next node.
*
* \return Pointer
*         NULL         There is no more token (name) in the path.
*         otherwise    The pointer to the next token (name).
*******************************************************************************/
static char* cy_p64_path_get_next_name_index(const char *in_path, char **name, size_t *idx)
{
    static char orig_path[CY_P64_MAX_PATH_LEN];
    static char *path;
    const char *p_index = NULL;

    if (in_path != NULL)
    {
        if (strlen(in_path) >= sizeof(orig_path))
        {
            return NULL;
        }
        (void)strncpy(orig_path, in_path, sizeof(orig_path) - 1u);
        path = orig_path;
    }
    if (name != NULL)
    {
        *name = path;
    }

    while(*path != '\0')
    {
        if ((*path) == ':') /* Index */
        {
            *path = '\0';
            path++;
            p_index = path;
            continue;
        }
        else if ((*path) == '/') /* Name */
        {
            *path = '\0';
            path++;
            break;
        }
        else
        {
            path++;
        }
    }

    /* Index ? */
    if (idx != NULL)
    {
        if (p_index == NULL)
        {
            *idx = 0;
        }
        else
        {
            errno = 0;
            *idx = strtoul(p_index, NULL, 10);
            if(errno != 0)
            {
                *idx = 0; /* parse_error */
            }
        }
    }

    return ((*path == '\0') ? NULL : path);
}


/*******************************************************************************
* Function Prototypes
****************************************************************************//**
*
*  \addtogroup jwt_policy_api
*
*  \{
*******************************************************************************/

/*******************************************************************************
* Function Name: cy_p64_find_json_item
****************************************************************************//**
* Finds items by name in the whole JSON. The name can be not unique so
* the function returns the first item with this name. For not unique items, it is
* recommended to parse JSON in two steps: the first find upper unique JSON object,
* then parse items in this object.
* Alternatively, provide a full path to the path parameter with the next
* token(name) & index from the path, e.g.: "path:2/to/the:1/node" --> "path" & 2
* For example: boot_upgrade/firmware/resources:1/address:1
*
* \param[in] path   A unique string name to lookup after, or full path.
* \param[in] json   JSON object to check.
* \return           Returns the pointer to the found JSON object or NULL.
*******************************************************************************/
const cy_p64_cJSON *cy_p64_find_json_item(const char *path, const cy_p64_cJSON *json)
{
    char *name = NULL;
    size_t idx = 0;
    const cy_p64_cJSON *item = json;
    const char *path_loc = path;

    while ((path_loc != NULL) && (item != NULL))
    {
        path_loc = cy_p64_path_get_next_name_index(path_loc, &name, &idx);

        /* Special care of Array */
        if (item->type == CY_P64_cJSON_Array)
        {
            item = item->child;
            while ((idx-- != 0u) && (item != NULL))
            {
                item = item->next;
            }
        }
        item = cy_p64_cJSON_GetObjectItem(item, name);
    }

    return item;
}


/*******************************************************************************
* Function Name: cy_p64_decode_payload_data
****************************************************************************//**
* Decodes JWT payload data from the input jwt_packet to JSON object
* and returns pointer to this object: json_packet.
* It allocates the required space in RAM for the JSON object, so after usage the caller
*  needs to free the JSON object by calling the cy_p64_cJSON_Delete() function.
*
* \param[in] jwt_packet     The pointer to the JWT packet.
* \param[out] json_packet   Outputs the JSON object that contains the JWT payload.
*
* \retval #CY_P64_SUCCESS
* \retval #CY_P64_JWT_ERR_INVALID_PARAMETER
*         This error code is returned, if \p json_packet is a null pointer.
* \retval #CY_P64_JWT_ERR_MALLOC_FAIL
* \retval #CY_P64_JWT_ERR_B64DECODE_FAIL
* \retval #CY_P64_JWT_ERR_JSN_PARSE_FAIL
* \retval #CY_P64_JWT_ERR_OTHER
*******************************************************************************/
cy_p64_error_codes_t cy_p64_decode_payload_data(const char *jwt_packet, cy_p64_cJSON **json_packet)
{
    cy_p64_error_codes_t ret = CY_P64_JWT_ERR_OTHER;
    const char *body = NULL;
    uint32_t body_len = 0;
    char *json_str = NULL;
    uint32_t json_len = 0;

    if(json_packet == NULL)
    {
        ret = CY_P64_JWT_ERR_INVALID_PARAMETER;
    }
    else
    {
        ret = cy_p64_get_jwt_data_body(jwt_packet, &body, &body_len);
        if(ret == CY_P64_SUCCESS)
        {
            json_len = CY_P64_GET_B64_DECODE_LEN(body_len);
            json_str = (char *)cy_p64_malloc(json_len);
            if(json_str == NULL)
            {
                ret = CY_P64_JWT_ERR_MALLOC_FAIL;
            }
            else
            {
                if(cy_p64_base64_decode((const uint8_t *)body, (int32_t)body_len,
                        (uint8_t *)json_str, json_len, CY_P64_BASE64_URL_SAFE_CHARSET) <= 0)
                {
                    ret = CY_P64_JWT_ERR_B64DECODE_FAIL;
                }
                else
                {
                    *json_packet = cy_p64_cJSON_Parse(json_str);
                    if(*json_packet == NULL)
                    {
                        ret = CY_P64_JWT_ERR_JSN_PARSE_FAIL;
                    }
                }
                cy_p64_free(json_str);
            }
        }
    }

    return ret;
}


/*******************************************************************************
* Function Name: cy_p64_json_get_boolean
****************************************************************************//**
* Gets the BOOLEAN value of a JSON object. It's caller's responsibility to use
* the right function that matches the data type to retrieve data.
*
* \param[in] json   JSON object.
* \param[out] value The pointer to the BOOLEAN value.
*
* \retval #CY_P64_SUCCESS
* \retval #CY_P64_JWT_ERR_INVALID_PARAMETER
*         This error code is returned, if \p value is a null pointer.
* \retval #CY_P64_JWT_ERR_JSN_WRONG_TYPE
*         This error code is returned, if JSON object type is not boolean.
*******************************************************************************/
cy_p64_error_codes_t cy_p64_json_get_boolean(const cy_p64_cJSON *json, bool *value)
{
    cy_p64_error_codes_t ret = CY_P64_SUCCESS;

    if(value == NULL)
    {
        ret = CY_P64_JWT_ERR_INVALID_PARAMETER;
    }
    else if(json->type == (int)CY_P64_cJSON_True)
    {
        *value = true;
    }
    else if(json->type == (int)CY_P64_cJSON_False)
    {
        *value = false;
    }
    else
    {
        ret = CY_P64_JWT_ERR_JSN_WRONG_TYPE;
    }

    return ret;
}


/*******************************************************************************
* Function Name: cy_p64_json_get_uint32
****************************************************************************//**
* Gets the unsigned INTEGER value of a JSON object. It's caller's responsibility to use
* the right function that matches the data type to retrieve data.
*
* \param[in] json   JSON object.
* \param[out] value The pointer to the unsigned INTEGER value.
*
* \retval #CY_P64_SUCCESS
* \retval #CY_P64_JWT_ERR_INVALID_PARAMETER
*         This error code is returned, if \p value is a null pointer.
* \retval #CY_P64_JWT_ERR_JSN_WRONG_TYPE
*         This error code is returned, if JSON object type is not an integer.
*******************************************************************************/
cy_p64_error_codes_t cy_p64_json_get_uint32(const cy_p64_cJSON *json, uint32_t *value)
{
    cy_p64_error_codes_t ret = CY_P64_SUCCESS;

    if(value == NULL)
    {
        ret = CY_P64_JWT_ERR_INVALID_PARAMETER;
    }
    else if(json->type == CY_P64_cJSON_Number)
    {
        *value = json->valueint;
    }
    else
    {
        ret = CY_P64_JWT_ERR_JSN_WRONG_TYPE;
    }

    return ret;
}


/*******************************************************************************
* Function Name: cy_p64_json_get_string
****************************************************************************//**
* Gets the unsigned STRING value of a JSON object. It's caller's responsibility to use
* the right function that matches the data type to retrieve data.
*
* \param[in] json   The JSON object.
* \param[out] value The pointer to the STRING value.
*
* \retval #CY_P64_SUCCESS
* \retval #CY_P64_JWT_ERR_INVALID_PARAMETER
*          This error code is returned, if \p value is a null pointer.
* \retval #CY_P64_JWT_ERR_JSN_WRONG_TYPE
*         This error ocde is returned, if JSON object type is not a string.
*******************************************************************************/
cy_p64_error_codes_t cy_p64_json_get_string(const cy_p64_cJSON *json, const char **value)
{
    cy_p64_error_codes_t ret = CY_P64_SUCCESS;

    if(value == NULL)
    {
        ret = CY_P64_JWT_ERR_INVALID_PARAMETER;
    }
    else if(json->type == CY_P64_cJSON_String)
    {
        *value = json->valuestring;
    }
    else
    {
        ret = CY_P64_JWT_ERR_JSN_WRONG_TYPE;
    }

    return ret;
}


/*******************************************************************************
* Function Name: cy_p64_json_get_array_uint8
****************************************************************************//**
* Gets the ARRAY value, in uint8, of a JSON object. It's caller's responsibility
* to use the right function that matches the data type to retrieve data.
*
* \param[in]  json  The JSON object.
* \param[out] buf   The pointer to the buffer where to store the content.
* \param[in]  size  The available size of the buffer in bytes.
* \param[out] olen  The actual size used in the buffer in bytes.
*
* \retval #CY_P64_SUCCESS
* \retval #CY_P64_JWT_ERR_INVALID_PARAMETER
*         This error code is returned, if \p buf is a null pointer.
* \retval #CY_P64_JWT_ERR_JSN_WRONG_TYPE
*         This error code is returned, if JSON object is not an array or it contains
*         values other than integer or None.
*******************************************************************************/
cy_p64_error_codes_t cy_p64_json_get_array_uint8(const cy_p64_cJSON *json, uint8_t *buf, uint32_t size, uint32_t *olen)
{
    cy_p64_error_codes_t ret = CY_P64_SUCCESS;

    if(buf == NULL)
    {
        ret = CY_P64_JWT_ERR_INVALID_PARAMETER;
    }
    else if(json->type == CY_P64_cJSON_Array)
    {
        uint32_t arraySize = (uint32_t)cy_p64_cJSON_GetArraySize(json);
        uint32_t i;

        if(arraySize > size)
        {
            arraySize = size;
        }
        for (i = 0; i < arraySize; i++)
        {
            const cy_p64_cJSON *subitem = cy_p64_cJSON_GetArrayItem(json, (int)i);
            if((subitem == NULL) || (subitem->type != CY_P64_cJSON_Number))
            {
                ret = CY_P64_JWT_ERR_JSN_WRONG_TYPE;
                break;
            }
            else
            {
                buf[i] = CY_LO8(subitem->valueint);
            }
        }
        if((olen != NULL) && (ret == CY_P64_SUCCESS))
        {
            *olen = arraySize;
        }
    }
    else
    {
        ret = CY_P64_JWT_ERR_JSN_WRONG_TYPE;
    }

    return ret;
}


/*******************************************************************************
* Function Name: cy_p64_policy_get_image_record
****************************************************************************//**
* Gets the JSON object of the image_id
*
* \param[in]  json          The JSON object with a policy to check.
* \param[in]  image_id      The image ID.
* \param[out] json_image    Output JSON object that contains image configuration.
*
* \retval #CY_P64_SUCCESS
* \retval #CY_P64_JWT_ERR_INVALID_PARAMETER
*         This error code is returned, if \p json_image is a null pointer.
* \retval #CY_P64_JWT_ERR_JSN_NONOBJ
*         This error is returned, if JSON does not contain "boot_upgrade/firmware" array
* \retval #CY_P64_JWT_ERR_JSN_WRONG_TYPE
*         This error is returned, if "boot_upgrade/firmware" object type is not an array
* \retval #CY_P64_INVALID
*******************************************************************************/
cy_p64_error_codes_t cy_p64_policy_get_image_record(
    const cy_p64_cJSON *json,
    uint32_t image_id,
    const cy_p64_cJSON **json_image)
{
    const cy_p64_cJSON *node = NULL;
    cy_p64_error_codes_t ret = CY_P64_INVALID;

    /* First, find the array at the specified path */
    node = cy_p64_find_json_item("boot_upgrade/firmware", json);

    if(json_image == NULL)
    {
        ret = CY_P64_JWT_ERR_INVALID_PARAMETER;
    }
    else if (node == NULL)
    {
        ret = CY_P64_JWT_ERR_JSN_NONOBJ;
    }
    else if(node->type != CY_P64_cJSON_Array)
    {
        ret = CY_P64_JWT_ERR_JSN_WRONG_TYPE;
    }
    else
    {
        const cy_p64_cJSON *subitem;
        uint32_t arraySize = (uint32_t)cy_p64_cJSON_GetArraySize(node);
        uint32_t i;

        /* Now look for the array element with the matching image_id */
        for (i = 0; i < arraySize; i++)
        {
            uint32_t id;

            *json_image = cy_p64_cJSON_GetArrayItem(node, (int)i);
            subitem = cy_p64_cJSON_GetObjectItem(*json_image, "id");
            if(cy_p64_json_get_uint32(subitem, &id) == CY_P64_SUCCESS)
            {
                /* Break if it is the one we're looking for */
                if (image_id == id)
                {
                    ret = CY_P64_SUCCESS;
                    break;
                }
            }
        }
    }
    return ret;
}


/*******************************************************************************
* Function Name: cy_p64_policy_get_image_address_and_size
****************************************************************************//**
* Gets the image address and size from the provisioning policy.
* It calls cy_p64_policy_get_image_record() function to get the image policy.
*
* \param[in]  json          JSON object with policy to check.
* \param[in]  image_id      The image ID.
* \param[in]  image_type    The image type: "BOOT", "UPGRADE" or other.
* \param[out] address       Output image address.
* \param[out] size          Output image size.
*
* \retval #CY_P64_SUCCESS
* \retval #CY_P64_JWT_ERR_INVALID_PARAMETER
*         This error code is returned, if \p address or \p size is a null pointer.
* \retval #CY_P64_JWT_ERR_JSN_NONOBJ
*         This error is returned, if JSON does not contain "resources" array
* \retval #CY_P64_JWT_ERR_JSN_WRONG_TYPE
*         This error is returned, if "resources" object type is not an array
* \retval #CY_P64_JWT_ERR_JSN_PARSE_FAIL
*******************************************************************************/
cy_p64_error_codes_t cy_p64_policy_get_image_address_and_size(
    const cy_p64_cJSON *json,
    uint32_t image_id,
    const char *image_type,
    uint32_t *address,
    uint32_t *size)
{
    const cy_p64_cJSON *json_image;
    cy_p64_error_codes_t ret = CY_P64_JWT_ERR_JSN_PARSE_FAIL;

    if((address == NULL) ||(size == NULL))
    {
        ret = CY_P64_JWT_ERR_INVALID_PARAMETER;
    }
    else
    {
        ret = cy_p64_policy_get_image_record(json, image_id, &json_image);
        if(ret == CY_P64_SUCCESS)
        {
            const cy_p64_cJSON *node = cy_p64_cJSON_GetObjectItem(json_image, "resources");
            if (node == NULL)
            {
                ret = CY_P64_JWT_ERR_JSN_NONOBJ;
            }
            else if(node->type != CY_P64_cJSON_Array)
            {
                ret = CY_P64_JWT_ERR_JSN_WRONG_TYPE;
            }
            else
            {
                uint32_t arraySize = (uint32_t)cy_p64_cJSON_GetArraySize(node);
                uint32_t i;
                ret = CY_P64_JWT_ERR_JSN_PARSE_FAIL;

                /* Now look for the array element with the matching image_type */
                for (i = 0; i < arraySize; i++)
                {
                    const char *str_value;
                    const cy_p64_cJSON *json_res = cy_p64_cJSON_GetArrayItem(node, (int)i);
                    const cy_p64_cJSON *subitem = cy_p64_cJSON_GetObjectItem(json_res, "type");

                    if(cy_p64_json_get_string(subitem, &str_value) == CY_P64_SUCCESS)
                    {
                        if (strcmp(image_type, str_value) == 0)
                        {
                            /* Now parse address and size */
                            subitem = cy_p64_cJSON_GetObjectItem(json_res, "address");
                            ret = cy_p64_json_get_uint32(subitem, address);
                            if(ret == CY_P64_SUCCESS)
                            {
                                subitem = cy_p64_cJSON_GetObjectItem(json_res, "size");
                                ret = cy_p64_json_get_uint32(subitem, size);
                            }
                            break;
                        }
                    }
                }
            }
        }
    }
    return ret;
}


/*******************************************************************************
* Function Name: cy_p64_policy_get_image_boot_config
****************************************************************************//**
* Gets the image wdt configuration and image_ok from provisioning policy.
* It calls cy_p64_policy_get_image_record() function to get image policy.
* The function sets wdt_enable and set_img_ok output parameters to false if
* they are absent in the policy.
*
* \param[in]  json          The JSON object with the policy to check.
* \param[in]  image_id      The image ID.
* \param[out] wdt_enable    An optional output wdt_enable config. If the set image needs
*                            to configure WDT before launching the next image.
* \param[out] wdt_timeout   WDT timeout value config used for WDT
*                            configuration if wdt_enable is set.
* \param[out] set_img_ok    Optional Image Ok config. When it is set to "true"
*                            - CM0 pre-build image confirms image ok status in
*                            image trailer by calling cy_p64_confirm_image() API,
*                            otherwise the user application needs to call this
*                            function after a successful boot.
*
* \retval #CY_P64_SUCCESS
* \retval #CY_P64_JWT_ERR_JSN_PARSE_FAIL
*******************************************************************************/
cy_p64_error_codes_t cy_p64_policy_get_image_boot_config(
    const cy_p64_cJSON *json,
    uint32_t image_id,
    bool *wdt_enable,
    uint32_t *wdt_timeout,
    bool *set_img_ok)
{
    const cy_p64_cJSON *subitem;
    const cy_p64_cJSON *json_image;
    cy_p64_error_codes_t ret = CY_P64_JWT_ERR_JSN_PARSE_FAIL;

    ret = cy_p64_policy_get_image_record(json, image_id, &json_image);
    if(ret == CY_P64_SUCCESS)
    {
        if(wdt_enable != NULL)
        {
            subitem = cy_p64_cJSON_GetObjectItem(json_image, "wdt_enable");
            if(subitem == NULL)
            {
                *wdt_enable = false;
            }
            else
            {
                ret = cy_p64_json_get_boolean(subitem, wdt_enable);
                if((wdt_timeout != NULL) && (ret == CY_P64_SUCCESS))
                {
                    subitem = cy_p64_cJSON_GetObjectItem(json_image, "wdt_timeout");
                    ret = cy_p64_json_get_uint32(subitem, wdt_timeout);
                }
            }
        }

        if((set_img_ok != NULL) && (ret == CY_P64_SUCCESS))
        {
            subitem = cy_p64_cJSON_GetObjectItem(json_image, "set_img_ok");
            if(subitem == NULL)
            {
                *set_img_ok = false;
            }
            else
            {
                ret = cy_p64_json_get_boolean(subitem, set_img_ok);
            }
        }
    }

    return ret;
}

/** \} */

