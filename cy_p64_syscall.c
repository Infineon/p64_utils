/***************************************************************************//**
* \file cy_p64_syscall.c
* \version 1.0
*
* \brief
* This is the source code file for low-level syscall functions.
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
* Low-level Prototypes
****************************************************************************//**
*
* \defgroup syscall     Low-level syscall
*
* \brief
*  This library implement low-level syscall functionality
*
* \{
*   \defgroup syscall_api Functions
*   \defgroup syscall_macros Macros
*   \defgroup syscall_t Data Structures
* \}
*******************************************************************************/

#include "cy_ipc_drv.h"
#include "cy_crypto_common.h"
#include "cy_crypto_core.h"
#include "cy_p64_syscall.h"

/** SysCall timeout values */
#define CY_P64_PSACRYPTO_SYSCALL_TIMEOUT_SHORT      (15000UL)
#define CY_P64_PSACRYPTO_SYSCALL_TIMEOUT_LONG       (2000000000UL)


/** \addtogroup syscall_api
 * \{
 */

/*******************************************************************************
* Function Name: cy_p64_syscall
****************************************************************************//**
*
* Used to call syscall from Secure FlashBoot.
* If SFB uses Crypto HW for particular syscall processing, it cleans and disables
* Crypto HW before return from syscall. Therefore this API checks Cypto HW status
* before syscall and enables it automatically before exit.
*
* \param[in,out] *cmd:    The pointer to the buffer with syscall parameters.
*                         The buffer is updated by syscall with response data
*
* \return        \ref CY_P64_SUCCESS for success or error code
*
*******************************************************************************/
cy_p64_error_codes_t cy_p64_syscall(uint32_t *cmd)
{
    uint32_t timeout = 0U;
    cy_p64_error_codes_t status = CY_P64_INVALID_TIMEOUT;
    bool crypto_is_enabled = Cy_Crypto_Core_IsEnabled(CRYPTO);
    if(crypto_is_enabled)
    {
        /* Syscall will disable Crypto HW,
         * do it explicitly so Crypto driver will properly track Hw state */
        (void)Cy_Crypto_Core_Disable(CRYPTO);
    }

    bool direct_params = (*cmd & CY_P64_SYSCALL_DIRECT_PARAMS) != 0U;

    /* Get IPC base register address */
    IPC_STRUCT_Type *ipc_struct = Cy_IPC_Drv_GetIpcBaseAddress(CY_IPC_CHAN_SYSCALL);

    while((CY_IPC_DRV_SUCCESS != Cy_IPC_Drv_LockAcquire(ipc_struct)) &&
          (timeout < CY_P64_PSACRYPTO_SYSCALL_TIMEOUT_SHORT))
    {
        ++timeout;
    }

    if(timeout < CY_P64_PSACRYPTO_SYSCALL_TIMEOUT_SHORT)
    {
        timeout = 0U;
        /* Write command value directly to the IPC DATA register if CY_P64_SYSCALL_DIRECT_PARAMS
         * bit is set, else write the address of RAM scratch with command and parameters
         * */
        Cy_IPC_Drv_WriteDataValue(ipc_struct, direct_params ? *cmd : (uint32_t)cmd);

        Cy_IPC_Drv_AcquireNotify(ipc_struct, ((uint32_t)1U << CY_IPC_INTR_SYSCALL1));

        while(Cy_IPC_Drv_IsLockAcquired(ipc_struct) &&
              (timeout < CY_P64_PSACRYPTO_SYSCALL_TIMEOUT_LONG))
        {
            ++timeout;
        #if (defined(CY_DEVICE_PSOC6A2M) || defined(CY_DEVICE_PSOC6A512K))
            /* Read PPU#4 registers as a workaround for ID# 338574 */
            (void)CY_GET_REG32(PERI_MS_PPU_PR4);
        #endif /* CY_DEVICE_PSOC6A2M */
        }

        if(timeout < CY_P64_PSACRYPTO_SYSCALL_TIMEOUT_LONG)
        {
            /* Syscall returns the status of the transaction in place where command was placed:
             * IPC DATA register or RAM scratch area depend on CY_P64_SYSCALL_DIRECT_PARAMS bit */
            if(direct_params)
            {
                status = Cy_IPC_Drv_ReadDataValue(ipc_struct);
            }
            else
            {
                status = CY_GET_REG32(cmd);
            }
        }
        /* SFB syscall returns with HW Crypto module disabled
         * Turn it on if its previous state in enabled */
        if (crypto_is_enabled)
        {
            (void)Cy_Crypto_Core_Enable(CRYPTO);
        }
    }

    return status;
}


/** \} */
