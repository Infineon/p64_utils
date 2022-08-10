# PSoC64 Secure Boot Utilities Middleware Library

## Overview
The Secure Boot Utilities Middleware Library is a set of utility code for the PSoC64 device to perform some operations, for example, providing an interface to 
Secure FlashBoot SysCalls including the PSA cryptographic functions, generic syscall functions, base64 decode provisioning JWT packet, 
parsing JSON data, policy parsing helpers, dynamic memory manager.

## Features

### PSA Cryptographic library
Wrapper functions to call PSA Crypto API v1.0 are implemented in Secure FlashBoot.
All cryptographic functions are implemented according to the Mbed Crypto library version 2.24 interface. This version corresponds to the version which used in Secure FlashBoot.
Refer to the PSA Crypto API Reference Guide for the specific PSA Crypto functions:  
- https://armmbed.github.io/mbed-crypto/html/index.html
- https://github.com/ARMmbed/mbedtls/tree/v2.24.0#psa-cryptography-api

### Generic Syscall functions
Wrapper functions to call several syscalls which are implemented in Secure FlashBoot:
- Get Provision details
- DAP control
- Rollback counter manipulation
- Acquire response
- Attestation

### base64 library
The base64 library implements the base64 encoding and decoding functions.
The base64 decoding function is used to decode a provisioned JWT packet that contains the policy.
base64 is a third-party library.

### cJSON library
cJSON is a lightweight JSON parser. 
Optimized version 1.3.2 is released in this library. Version 1.3.2 is used because of the smallest flash memory consumption.
The 'valueint' type in the cJSON structure is changed from double to uint32_t to reduce the flash memory usage and time consumption. 
PSoC64 uses only 32-bit unsigned integers in the provisioning policy.
cJSON is a third party library and it is distributed under the MIT license.

### JWT/JSON Policy parsing helper functions.
Includes functions for:
	- Decode and parse the JWT packet 
	- Find items in the JSON object
	- Find the boot and upgrade the image address in the provisioning policy

### Swap upgrade utility functions
This interface allows writing "Image OK" flag to the slot trailer, so CypressBootloader cannot revert the new image. 

### High-level interface for interacting with the Watchdog Timer.
This interface allows start/stop WDT and set new timeout value.
This interface abstracts out the chip specific details. If any chip specific functionality is necessary, 
or performance is critical, the low-level functions can be used directly.

### Dynamic memory allocation functions.
The static buffer is allocated, it is used dynamically for the memory allocation functions.
CY_P64_HEAP_DATA_SIZE defines the size for a local buffer and can be re-defined by the user based on the maximum memory size requirements.

## Supported Kits (make variable 'TARGET')

* [PSoC 64 Secure Boot Wi-Fi BT Pioneer Kit (CY8CKIT-064B0S2-4343W)](http://www.cypress.com/CY8CKIT-064B0S2-4343W)
* [PSoC 64 Standard Secure - AWS Wi-Fi BT Pioneer Kit (CY8CKIT-064S0S2-4343W) ](http://www.cypress.com/CY8CKIT-064S0S2-4343W)
* [PSoC 64 Secure Boot BLE Prototyping Kit (CY8CPROTO-064B0S1-BLE) ](http://www.cypress.com/CY8CPROTO-064B0S1-BLE)
* [PSoC 64 Secure Boot Prototyping Kit (CY8CPROTO-064B0S3) ](http://www.cypress.com/CY8CPROTO-064B0S3)
* [PSoC 64 Secure Boot Prototyping Kit (CY8CPROTO-064S1-SB) ](http://www.cypress.com/CY8CPROTO-064S1-SB)

## Quick Start Guide

This quick start guide assumes that the environment is configured to use the PSoC 6 Peripheral Driver Library(psoc6pdl) for development and the PSoC 6 Peripheral Driver Library(psoc6pdl) is included into the project.

It also assumes the PSoC64 device is provisioned as documented in [PSoC 64 Secure MCU Secure Boot SDK User Guide](www.cypress.com/documentation/software-and-drivers/psoc-64-secure-mcu-secure-boot-sdk-user-guide)

### Adding the library
There are two ways to add library to our project:
- add a dependency file (MTB format) into the 'deps' folder;
- use the Library Manager. It is available under Libraries Tab > PSoC 6 Middleware > p64-utils.

### Using the library

The library automatically allocates the buffer that is enough for default policy parsing.
If the project needs additional memory, change the default buffer size in makefile by defining 'CY_P64_HEAP_DATA_SIZE=XXXX' with the required value. 

#### Code snippet: Get Provisioning policy and parse it to the JSON object
```c
#include "cy_p64_syscalls.h"
#include "cy_p64_jwt_policy.h"

int main(void)
{
    __enable_irq();
    {
        cy_p64_error_codes_t status;
        char *jwt_packet;
        cy_p64_cJSON *json_packet;

        /* Get the provisioned JWT policy */
        status = cy_p64_get_provisioning_details(CY_P64_POLICY_JWT, &jwt_packet, NULL);
        if(status == CY_P64_SUCCESS)
        {
            /* Decode the JWT policy and parse it to the JSON object */
            status = cy_p64_decode_payload_data(jwt_packet, &json_packet);
            if(status == CY_P64_SUCCESS)
            {
                /* json_packet contains the parsed policy, which may be used by the following examples */
				
                /* Delete the JSON object after usage and free memory */
                cy_p64_cJSON_Delete(json_packet);
            }
        }
    }
    for (;;)
    {
    }
}
```

#### Code snippet: Find the upgrade image address and size in the policy
```c
	{
		uint32_t upgrade_address;
		uint32_t upgrade_size;
		status = cy_p64_policy_get_image_address_and_size(json_packet, 1, "UPGRADE", &upgrade_address, &upgrade_size);
		if(status == CY_P64_SUCCESS)
		{
			/* Use upgrade_address and upgrade_size for DFU to program upgrade image */
		}
	}
 ```
#### Code snippet: Confirm that the image is ok
```c
	#include "cy_p64_image.h"
	
	/* Find the boot image address and size in the policy */
	{
		uint32_t boot_address;
		uint32_t boot_size;
		uint32_t image_id = 1;
		status = cy_p64_policy_get_image_address_and_size(json_packet, image_id, "BOOT", &boot_address, &boot_size);
		if(status == CY_P64_SUCCESS)
		{
			/* Confirm that booted image is Ok, so MCU Boot will not swap it */
			status = cy_p64_confirm_image(boot_address, boot_size);
		}
	}
```
Note: To confirm the CM4 boot image when device is provisioned with policy_multi_CM0_CM4.json policy, set image_id = 16.

## Performance
PSA Crypto API benchmark for the PSoC64 2M device with the CM0p core frequency set to 50 Mhz:

Algorithm       | Execution time  
----------------| -------------  
SHA-256         | 2   ms/10kB payload  
SHA-256         | 4   ms/100kB payload  
AES-CBC-128     | 18  ms/10kB payload  
AES-CBC-256     | 19  ms/10kB payload  
AES-CTR-128     | 13  ms/10kB payload  
AES-CTR-256     | 14  ms/10kB payload  
HMAC-SHA256     | 2   ms/10kB payload  
ECDSA-secp256r1 | 110 ms/sign  
ECDSA-secp256r1 | 224 ms/verify  
ECDSA-secp256r1 | 363 ms/verify (including public key calculation)  
ECDH-secp256r1  | 137 ms/generate key pair  
ECDH-secp256r1  | 141 ms/handshake  

## More information
The following resources contain more information:
* [PSoC64 Secure Boot Utilities RELEASE.md](./RELEASE.md)
* [PSoC64 Secure Boot Utilities API Reference Guide](https://cypresssemiconductorco.github.io/p64_utils/p64_utils_api_reference_manual/html/index.html)
* [PSoC 64 Secure MCU Secure Boot SDK User Guide](https://www.cypress.com/documentation/software-and-drivers/psoc-64-secure-mcu-secure-boot-sdk-user-guide)
* [PSoC 64 Microcontrollers](https://www.cypress.com/products/psoc-64-microcontrollers-arm-cortex-m4m0)
* [ModusToolbox Software Environment, Quick Start Guide, Documentation, and Videos](https://www.cypress.com/products/modustoolbox-software-environment)
* [Cypress Semiconductor](http://www.cypress.com)


---
© Cypress Semiconductor Corporation (an Infineon company), 2019-2022.
