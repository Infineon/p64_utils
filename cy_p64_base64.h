/***************************************************************************//**
* \file cy_p64_base64.h
* \version 1.0
*
* \brief
* Provides header file for base64 APIs.
*
*******************************************************************************/

/*
* Copyright 2019-2021 Cypress Semiconductor Corporation
* (an Infineon company).
*/

/*    $OpenBSD: base64.c,v 1.3 1997/11/08 20:46:55 deraadt Exp $    */

/*
* Copyright (c) 1996 by Internet Software Consortium.
*
* Permission to use, copy, modify, and distribute this software for any
* purpose with or without fee is hereby granted, provided that the above
* copyright notice and this permission notice appear in all copies.
*
* THE SOFTWARE IS PROVIDED "AS IS" AND INTERNET SOFTWARE CONSORTIUM DISCLAIMS
* ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES
* OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL INTERNET SOFTWARE
* CONSORTIUM BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
* DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
* PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
* ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
* SOFTWARE.
*/

/*
* Portions Copyright (c) 1995 by International Business Machines, Inc.
*
* International Business Machines, Inc. (hereinafter called IBM) grants
* permission under its copyrights to use, copy, modify, and distribute this
* Software with or without fee, provided that the above copyright notice and
* all paragraphs of this notice appear in all copies, and that the name of IBM
* not be used in connection with the marketing of any product incorporating
* the Software or modifications thereof, without specific, written prior
* permission.
*
* To the extent it has a right to do so, IBM grants an immunity from suit
* under its patents, if any, for the use, sale or manufacture of products to
* the extent that such products are used for performing Domain Name System
* dynamic updates in TCP/IP networks by means of the Software.  No immunity is
* granted for any product per se or for any other function of any product.
*
* THE SOFTWARE IS PROVIDED "AS IS", AND IBM DISCLAIMS ALL WARRANTIES,
* INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
* PARTICULAR PURPOSE.  IN NO EVENT SHALL IBM BE LIABLE FOR ANY SPECIAL,
* DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER ARISING
* OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE, EVEN
* IF IBM IS APPRISED OF THE POSSIBILITY OF SUCH DAMAGES.
*/

/*******************************************************************************
* Base64 Prototypes
****************************************************************************//**
*
* \defgroup base64_api Base64
*
* \brief
*  This library provides Base64 API
*
* \{
*   \defgroup base64_macros Macros
*   \defgroup base64_enum Enumerations
*   \defgroup base64_func Functions
* \}
*******************************************************************************/

#ifndef CY_P64_BASE64_H
#define CY_P64_BASE64_H

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Base64 Macros
****************************************************************************//**
*
* \addtogroup base64_macros
*
* \{
*******************************************************************************/

/** Calculates the buffer size required for decoded base64 string storage + 1 for end of string. */
#define CY_P64_GET_B64_DECODE_LEN(length)       (((((length) + 3u) / 4u) * 3u) + 1u)

/** Calculates the buffer size required for string encoded into base64.
    This includes terminating null character. */
#define CY_P64_GET_B64_ENCODE_LEN(length)       (((((length) + 2u) / 3u) * 4u) + 1u)

/** \} */


/*******************************************************************************
* Base64 Enumerations
****************************************************************************//**
*
*  \addtogroup base64_enum
*
*  \{
*******************************************************************************/

/** List of Base64 conversion standards */
typedef enum
{
    CY_P64_BASE64_STANDARD                         = ( ( '+' << 16 ) | ( '/' << 8 ) | '=' ),  /* RFC 1421, 2045, 3548, 4648, 4880 */
    CY_P64_BASE64_NO_PADDING                       = ( ( '+' << 16 ) | ( '/' << 8 )       ),  /* RFC 1642, 3548, 4648 */
    CY_P64_BASE64_URL_SAFE_CHARSET                 = ( ( '-' << 16 ) | ( '_' << 8 )       ),  /* RFC 4648 */
    CY_P64_BASE64_URL_SAFE_CHARSET_WITH_PADDING    = ( ( '-' << 16 ) | ( '_' << 8 ) | '=' ),  /* RFC 4648 */
    CY_P64_BASE64_Y64                              = ( ( '.' << 16 ) | ( '_' << 8 ) | '-' ),
    CY_P64_BASE64_XML_TOKEN                        = ( ( '.' << 16 ) | ( '-' << 8 )       ),
    CY_P64_BASE64_XML_IDENTIFIER                   = ( ( '_' << 16 ) | ( ':' << 8 )       ),
    CY_P64_BASE64_PROG_IDENTIFIER1                 = ( ( '_' << 16 ) | ( '-' << 8 )       ),
    CY_P64_BASE64_PROG_IDENTIFIER2                 = ( ( '.' << 16 ) | ( '_' << 8 )       ),
    CY_P64_BASE64_REGEX                            = ( ( '!' << 16 ) | ( '-' << 8 )       ),
} cy_p64_base64_options_t;

/** \} */


/*******************************************************************************
* Base64 Functions
****************************************************************************//**
*
* \addtogroup base64_func
*
* This library implements Base64 encoding and decoding routines and their associated
* helper functions. For more information on Base64 encoding, see RFC4648.
*
*  \{
*******************************************************************************/

/*******************************************************************************
* Function Name: cy_p64_base64_encode
****************************************************************************//**
* This function encodes data into Base-64 coding which can be sent safely as text.
* Terminating null is appended.
*
* \param[in] src         : The pointer to the source data to be converted
* \param[in] src_length  : The length of data to be converted, or -1 if the data is a null terminated string
* \param[out] target     : The buffer that will receive the encoded data. NOTE: src and target can't be pointing to the same buffer.
* \param[in] target_size : The size of the output buffer in bytes. Use \ref CY_P64_GET_B64_ENCODE_LEN(src_length) macro for the calculation of this size.
* \param[in] options     : Specifies which Base64 encoding standard to use - see \ref cy_p64_base64_options_t
*
* \return number of Base64 characters output (not including terminating null),  otherwise negative indicates an error
*******************************************************************************/
int cy_p64_base64_encode( unsigned char const* src, int32_t src_length, unsigned char* target, uint32_t target_size, cy_p64_base64_options_t options );

/*******************************************************************************
* Function Name: cy_p64_base64_decode
****************************************************************************//**
* This function decodes data from Base-64 coding which can be sent safely as text.
* Terminating null is appended.
*
* \param[in] src         : The pointer to the source Base64 coded data to be decoded
* \param[in] src_length  : The length of data to be converted, or -1 if the data is a null terminated string
* \param[out] target     : The buffer that will receive the decoded data.
* \param[in] target_size : The size of the output buffer in bytes. Use \ref CY_P64_GET_B64_DECODE_LEN(src_length) macro for the calculation of this size.
* \param[in] options     : Specifies which Base64 encoding standard to use - see \ref cy_p64_base64_options_t
*
* \return number of decoded characters output (not including terminating null),  otherwise negative indicates an error
*******************************************************************************/
int cy_p64_base64_decode( unsigned char const* src, int32_t src_length, unsigned char* target, uint32_t target_size, cy_p64_base64_options_t options );

/** \} */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CY_P64_BASE64_H */
