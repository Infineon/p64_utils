/***************************************************************************//**
* \file cy_p64_image.c
* \version 1.0
*
* \brief
* This is the source code file for secure boot swap upgrade utility functions.
*
********************************************************************************
* \copyright
* Copyright 2021, Cypress Semiconductor Corporation (an Infineon company).
* All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/
#include <string.h>
#include "cy_p64_image.h"
#include "cy_flash.h"

#define CY_P64_USER_SWAP_IMAGE_OK_OFFS      (24u)
#define CY_P64_USER_SWAP_IMAGE_OK           (1u)

/*******************************************************************************
* Image Prototypes
****************************************************************************//**
*
* \defgroup image Image
*
* \brief
*  This library provides functions to work with images_ok field
*
* \{
*   \defgroup image_api Functions
* \}
*******************************************************************************/

/*******************************************************************************
* Function Name: cy_p64_flash_write_byte
****************************************************************************//**
* Writes 1 byte from "data" into flash memory at "address".
* It does a sequence of RD/Modify/WR of data in a Flash Row.
*
* \param[in] address        The address where to write.
* \param[in] data           The data to write.
* \return                   \ref CY_P64_SUCCESS for success or error code.
*******************************************************************************/
static cy_p64_error_codes_t cy_p64_flash_write_byte(uint32_t address, uint8_t data)
{
    cy_p64_error_codes_t ret = CY_P64_INVALID;
    uint32_t row_addr = 0;
    uint32_t row_buff[CY_FLASH_SIZEOF_ROW_LONG_UNITS];
    uint8_t *row_buff_bytes = (uint8_t *)row_buff;

    /* Accepting an arbitrary address */
    row_addr = (address / CY_FLASH_SIZEOF_ROW) * CY_FLASH_SIZEOF_ROW;

    /* Preserving Row */
    (void)memcpy(row_buff, (uint32_t *)row_addr, sizeof(row_buff));

    /* Modifying the target byte */
    row_buff_bytes[address % CY_FLASH_SIZEOF_ROW] = data;

    /* Programming updated row back */
    if(Cy_Flash_WriteRow(row_addr, row_buff) == CY_FLASH_DRV_SUCCESS)
    {
        ret = CY_P64_SUCCESS;
    }

    return ret;
}


/*******************************************************************************
* Function Prototypes
****************************************************************************//**
*
*  \addtogroup image_api
*
*  \{
*******************************************************************************/

/*******************************************************************************
* Function Name: cy_p64_is_image_confirmed
****************************************************************************//**
* Checks the Image OK flag in the slot trailer
*
* \param[in] image_start    The start address of the image.
* \param[in] image_size     The size of the image.
* \return                   "true" if the Image OK flag is set or "false" if not set.
*******************************************************************************/
bool cy_p64_is_image_confirmed(uint32_t image_start, uint32_t image_size)
{
    bool ret;
    uint32_t img_ok_addr;

    img_ok_addr = image_start + image_size - CY_P64_USER_SWAP_IMAGE_OK_OFFS;
    ret = (*((uint8_t *)img_ok_addr) == CY_P64_USER_SWAP_IMAGE_OK);

    return ret;
}


/*******************************************************************************
* Function Name: cy_p64_confirm_image
****************************************************************************//**
* Writes the Image OK flag to the slot trailer, so CypressBootloader
* will not revert the new image. Write to flash operation is skipped if the Image OK
* flag is already set in the trailer.
*
* \param[in] image_start    The start address of the image.
* \param[in] image_size     The size of the image.
* \return     \ref CY_P64_SUCCESS for success or error code.
*******************************************************************************/
cy_p64_error_codes_t cy_p64_confirm_image(uint32_t image_start, uint32_t image_size)
{
    cy_p64_error_codes_t ret = CY_P64_INVALID;
    uint32_t img_ok_addr;

    img_ok_addr = image_start + image_size - CY_P64_USER_SWAP_IMAGE_OK_OFFS;

    /* Write the Image OK flag to the slot trailer, so CypressBootloader
     * will not revert the new image */
    if (*((uint8_t *)img_ok_addr) == CY_P64_USER_SWAP_IMAGE_OK)
    {
        /* Image OK is already set in the trailer */
        ret = CY_P64_SUCCESS;
    }
    else
    {
        ret = cy_p64_flash_write_byte(img_ok_addr, CY_P64_USER_SWAP_IMAGE_OK);
    }

    return ret;
}

/** \} */
