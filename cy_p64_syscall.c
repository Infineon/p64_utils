/***************************************************************************//**
* \file cy_p64_syscall.c
* \version 1.0.1
*
* \brief
* This is the source code file for low-level syscall functions.
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
#include "cy_prot.h"
#include "cy_device_headers.h"

/** SysCall timeout values */
#define CY_P64_PSACRYPTO_SYSCALL_TIMEOUT_SHORT      (15000UL)
#define CY_P64_PSACRYPTO_SYSCALL_TIMEOUT_LONG       (2000000000UL)


/*******************************************************************************
* Function Name: IsCryptoPpuDisabled
****************************************************************************//**
*
* This function returns whether the Fixed PPU for Crypto HW is disabled.
*
* \return
* true if Slave structure is disabled.
* false if Slave structure is enabled.
*
*******************************************************************************/
#if (CY_CPU_CORTEX_M4) /* Check if Crypto is enabled for M4 core access by PPU */
static bool IsCryptoPpuDisabled(void)
{
    bool status = false;

    if (CY_PERI_V1 == 0U)
    {
    #if defined(PERI_MS_PPU_FX_CRYPTO_MAIN)
        PERI_MS_PPU_FX_Type* base = PERI_MS_PPU_FX_CRYPTO_MAIN;
        uint32_t mask =  ((_VAL2FLD(CY_PROT_ATT_PERI_USER_PERM, CY_PROT_PERM_W) |
                           _VAL2FLD(CY_PROT_ATT_PERI_PRIV_PERM, CY_PROT_PERM_W)) <<
                               (PERI_MS_PPU_PR_V2_MS_ATT1_PC6_UR_Pos ));
        if((base->SL_ATT1 & mask) == mask)
        {
            status = true;
        }
    #endif /* defined PERI_MS_PPU_FX_CRYPTO_MAIN */
    }
    else /* For mxperi version 1.x */
    {
    #if defined(PERI_GR_PPU_SL_CRYPTO)
        PERI_GR_PPU_SL_Type* base = PERI_GR_PPU_SL_CRYPTO;
        if(_FLD2VAL(PERI_GR_PPU_SL_ATT0_ENABLED, PERI_GR_PPU_SL_ATT0(base)) != CY_PROT_STRUCT_ENABLE)
        {
            status = true;
        }
    #endif /* defined PERI_GR_PPU_SL_CRYPTO */
    }

    return status;
}
#endif /* (CY_CPU_CORTEX_M4) */


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
    cy_en_ipcdrv_status_t ipc_status = CY_IPC_DRV_ERROR;
    cy_p64_error_codes_t status = CY_P64_INVALID_TIMEOUT;
    bool crypto_is_enabled = false;

#if (CY_CPU_CORTEX_M4) /* Check if Crypto is enabled for M4 core access by PPU */
    if(IsCryptoPpuDisabled())
#endif /* (CY_CPU_CORTEX_M4) */
    {
        crypto_is_enabled = Cy_Crypto_Core_IsEnabled(CRYPTO);
    }
    if(crypto_is_enabled)
    {
        /* Syscall will disable Crypto HW,
         * do it explicitly so Crypto driver will properly track Hw state */
        (void)Cy_Crypto_Core_Disable(CRYPTO);
    }

    bool direct_params = (*cmd & CY_P64_SYSCALL_DIRECT_PARAMS) != 0U;

    /* Get IPC base register address */
    IPC_STRUCT_Type *ipc_struct = Cy_IPC_Drv_GetIpcBaseAddress(CY_IPC_CHAN_SYSCALL);

    do
    {
        ipc_status = Cy_IPC_Drv_LockAcquire(ipc_struct);
        ++timeout;
    } while ((timeout < CY_P64_PSACRYPTO_SYSCALL_TIMEOUT_SHORT) && (CY_IPC_DRV_SUCCESS != ipc_status));

    if (CY_IPC_DRV_SUCCESS == ipc_status)
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
