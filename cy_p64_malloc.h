/***************************************************************************//**
* \file cy_p64_malloc.h
* \version 1.0
*
* \brief
* Contains the prototypes and constants used for the memory allocation.
*
********************************************************************************
* \copyright
* Copyright 2019-2021, Cypress Semiconductor Corporation (an Infineon company).
* All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#ifndef CY_P64_MALLOC_H
#define CY_P64_MALLOC_H

#include <stdint.h>


/*******************************************************************************
* Malloc Macros
****************************************************************************//**
*
*  \addtogroup malloc_macros
*
*  \{
*******************************************************************************/

/******************************************************
 *                      Macros
 ******************************************************/

/** The default size in bytes for the data buffer for the local heap. */
#ifndef CY_P64_HEAP_DATA_SIZE
#define CY_P64_HEAP_DATA_SIZE             (0x4000u)
#endif /* CY_P64_HEAP_DATA_SIZE */

/** Round up the value to an alignment of four */
#define CY_P64_ALIGN_TO_4(x)             (((((x) - 1u) >> 2u) << 2u) + 4u)

/** \} */

/******************************************************
 *                      Public API
 ******************************************************/
void *cy_p64_malloc(uint32_t size);
void *cy_p64_calloc(uint32_t nelem, uint32_t elsize);
void cy_p64_free(void *p);

#endif /*CY_P64_MALLOC_H*/

/* [] END OF FILE */
