/***************************************************************************//**
* \file cy_p64_watchdog.h
* \version 1.0
*
* \brief
* Provides a high-level interface for interacting with the Watchdog Timer.
* This interface abstracts out the chip specific details. If any chip specific
* functionality is necessary, or performance is critical the low-level functions
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

#ifndef CY_P64_WATCHDOG_H
#define CY_P64_WATCHDOG_H

#include <stdint.h>
#include "cy_p64_syscall.h"
#include "cy_wdt.h"


/* Public APIs */
cy_p64_error_codes_t cy_p64_wdg_init(uint32_t *timeout_ms);
void cy_p64_wdg_free(void);
void cy_p64_wdg_start(void);
void cy_p64_wdg_stop(void);
uint32_t cy_p64_wdg_max_timeout_ms(void);


/** \addtogroup watchdog_api
 * \{
 */

/** Call this function periodically to prevent the WDT from timing out and resetting the device. */
#define cy_p64_wdg_kick()           Cy_WDT_ClearWatchdog()

/** Check if WDT is enabled */
#define cy_p64_wdg_is_enabled()     Cy_WDT_IsEnabled()

/** \} */

#endif /* CY_P64_WATCHDOG_H */
