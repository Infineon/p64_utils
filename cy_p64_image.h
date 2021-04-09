/***************************************************************************//**
* \file cy_p64_image.h
* \version 1.0
*
* \brief
* This is the header file for the secure boot swap upgrade utility functions.
*
********************************************************************************
* \copyright
* Copyright 2021, Cypress Semiconductor Corporation (an Infineon company).
* All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#ifndef CY_P64_IMAGE_H
#define CY_P64_IMAGE_H

#include <stdint.h>
#include <stdbool.h>
#include "cy_p64_syscall.h"

cy_p64_error_codes_t cy_p64_confirm_image(uint32_t image_start, uint32_t image_size);
bool cy_p64_is_image_confirmed(uint32_t image_start, uint32_t image_size);

#endif /* CY_P64_IMAGE_H */
