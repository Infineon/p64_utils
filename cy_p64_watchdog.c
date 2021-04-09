/***************************************************************************//**
* \file cy_p64_watchdog.c
* \version 1.0
*
* \brief
* Provides a high level interface for interacting with the Cypress Watchdog Timer.
* This interface abstracts out the chip specific details. If any chip specific
* functionality is necessary, or performance is critical the low level functions
* can be used directly.
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
* Watchdog Prototypes
****************************************************************************//**
*
* \defgroup watchdog     Watchdog
*
* \brief
*  This library implements the watchdog functionality.
*
* \{
*   \defgroup watchdog_api Functions
* \}
*******************************************************************************/

#include <stdbool.h>
#include "cy_utils.h"
#include "cy_p64_watchdog.h"


/** Maximum number of ignore bits */
#define CY_P64_WDT_MAX_IGNORE_BITS      (12u)

typedef struct {
    uint16_t min_period_ms;         /** The minimum period in milliseconds that can be represented with this many ignored bits */
    uint16_t round_threshold_ms;    /** The timeout threshold in milliseconds, from which to round up to the minimum period */
} cy_p64_wdg_ignore_bits_data_t;

/* ILO Frequency = 32768 Hz
*  ILO Period = 1 / 32768 Hz = .030518 ms
*  WDT Reset Period (timeout_ms) = .030518 ms * (2 * 2^(16 - ignore_bits) + match)
*  ignore_bits range: 0 - 12
*  match range: 0 - (2^(16 - ignore_bits) - 1)
*/
static const cy_p64_wdg_ignore_bits_data_t cy_p64_wdg_ignore_data[CY_P64_WDT_MAX_IGNORE_BITS + 1u] =
{
    {4001u, 3001u}, /* 0 bits:  min period: 4001ms, max period: 6000ms, round up from 3001+ms */
    {2001u, 1500u}, /* 1 bit:   min period: 2001ms, max period: 3000ms, round up from 1500+ms */
    {1001u, 750u},  /* 2 bits:  min period: 1001ms, max period: 1499ms, round up from 750+ms */
    {501u,  375u},  /* 3 bits:  min period: 501ms,  max period: 749ms,  round up from 375+ms */
    {251u,  188u},  /* 4 bits:  min period: 251ms,  max period: 374ms,  round up from 188+ms */
    {126u,  94u},   /* 5 bits:  min period: 126ms,  max period: 187ms,  round up from 94+ms */
    {63u,   47u},   /* 6 bits:  min period: 63ms,   max period: 93ms,   round up from 47+ms */
    {32u,   24u},   /* 7 bits:  min period: 32ms,   max period: 46ms,   round up from 24+ms */
    {16u,   12u},   /* 8 bits:  min period: 16ms,   max period: 23ms,   round up from 12+ms */
    {8u,    6u},    /* 9 bits:  min period: 8ms,    max period: 11ms,   round up from 6+ms */
    {4u,    3u},    /* 10 bits: min period: 4ms,    max period: 5ms,    round up from 3+ms */
    {2u,    2u},    /* 11 bits: min period: 2ms,    max period: 2ms */
    {1u,    1u}     /* 12 bits: min period: 1ms,    max period: 1ms */
};

static bool cy_p64_wdg_initialized = false;
static bool cy_p64_wdg_pdl_initialized = false;

/*******************************************************************************
* Function Prototypes
****************************************************************************//**
*
*  \addtogroup watchdog_api
*
* Provides a high-level interface for interacting with the Cypress Watchdog Timer.
* This interface abstracts out the chip specific details. If any chip specific
* functionality is necessary, or performance is critical the low-level functions
* can be used directly.
*
*  \{
*******************************************************************************/

/*******************************************************************************
* Function Name: cy_p64_wdg_init
****************************************************************************//**
*
* Initializes the WDT. Calls cy_p64_wdg_start() API to start WDT after this
* function.
*
* \note
* The specified timeout must be at least 1ms and at most the WDT's maximum
* timeout (see cy_p64_wdg_max_timeout_ms()).
*
* \param timeout_ms  
* The time in milliseconds before the WDT times out. It is updated by function
* to the rounded value actually set to WDT.
*
* \return     \ref CY_P64_SUCCESS for success of the init request or error code
*
*******************************************************************************/
cy_p64_error_codes_t cy_p64_wdg_init(uint32_t *timeout_ms)
{
    uint8_t ignore_bits;
    cy_p64_error_codes_t ret;

    if ((*timeout_ms == 0u) || (*timeout_ms > cy_p64_wdg_max_timeout_ms()))
    {
        ret = CY_P64_INVALID;
    }
    else if (cy_p64_wdg_initialized)
    {
        ret = CY_P64_INVALID;
    }
    else
    {
        cy_p64_wdg_initialized = true;

        if (!cy_p64_wdg_pdl_initialized)
        {
            Cy_WDT_MaskInterrupt();
            cy_p64_wdg_pdl_initialized = true;
        }

        cy_p64_wdg_stop();

        for (ignore_bits = 0; ignore_bits <= CY_P64_WDT_MAX_IGNORE_BITS; ignore_bits++)
        {
            if (*timeout_ms >= cy_p64_wdg_ignore_data[ignore_bits].round_threshold_ms)
            {
                if (*timeout_ms < cy_p64_wdg_ignore_data[ignore_bits].min_period_ms)
                {
                    *timeout_ms = cy_p64_wdg_ignore_data[ignore_bits].min_period_ms;
                }
                break;
            }
        }

        Cy_WDT_SetIgnoreBits(ignore_bits);

        Cy_WDT_SetMatch(CY_LO16((*timeout_ms * 32768U / 1000U) - (1UL << (17U - ignore_bits)) + Cy_WDT_GetCount()));

        ret = CY_P64_SUCCESS;
    }

    return ret;
}


/*******************************************************************************
* Function Name: cy_p64_wdg_free
****************************************************************************//**
*
* Free the WDT
*
* Make sure cy_p64_wdg_stop() is called to stops the WDT before this function.
* After calling this function no other WDT functions should be called except
* cy_p64_wdg_init().
*
*******************************************************************************/
void cy_p64_wdg_free(void)
{
    cy_p64_wdg_initialized = false;
}


/*******************************************************************************
* Function Name: cy_p64_wdg_start
****************************************************************************//**
*
* Start (enable) the WDT
*
*******************************************************************************/
void cy_p64_wdg_start(void)
{
    Cy_WDT_Unlock();
    Cy_WDT_Enable();
    Cy_WDT_Lock();
}


/*******************************************************************************
* Function Name: cy_p64_wdg_stop
****************************************************************************//**
*
* Stop (disable) the WDT
*
*******************************************************************************/
void cy_p64_wdg_stop(void)
{
    Cy_WDT_Unlock();
    Cy_WDT_Disable();
}


/*******************************************************************************
* Function Name: cy_p64_wdg_max_timeout_ms
****************************************************************************//**
*
* This function returns maximum WDT timeout in milliseconds
*
* \return        maximum WDT timeout in milliseconds
*
*******************************************************************************/
uint32_t cy_p64_wdg_max_timeout_ms(void)
{
    return ((WDT_MAX_MATCH_VALUE + (1UL << 17U)) * 1000U / 32768U);
}

/** \} */
