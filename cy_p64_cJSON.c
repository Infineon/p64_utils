/***************************************************************************//**
* \file cy_p64_cJSON.c
* \version 1.0
*
* \brief
* This is the source code file for the JSON parsing and processing.
*
*    Changes:
*    - version 1.3.2 is used because of the smallest flash memory consumption
*    - the valueint type in cJSON structure changed from double to uint32_t to
*      reduce flash memory and time consumption; PSoC64 uses only 32-bit
*      unsigned integers in the provisioning policy.
*    - added the cy_p64_ prefix to keep the possibility of integration with another cJSON
*      version in the project
*
********************************************************************************
* \copyright
*  Copyright 2019-2021 Cypress Semiconductor Corporation (an Infineon company)
*******************************************************************************/
/*
  Copyright (c) 2009 Dave Gamble

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/


#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include <ctype.h>
#include "cy_p64_cJSON.h"
#include "cy_p64_malloc.h"

#define DBL_EPSILON (1u)

/* define our own boolean type */
typedef int cjbool;
#define cj_true ((cjbool)1)
#define cj_false ((cjbool)0)

static const unsigned char *global_ep = NULL;

const char *cy_p64_cJSON_GetErrorPtr(void)
{
    return (const char*) global_ep;
}

/* This is a safeguard to prevent copy-pasters from using incompatible C and header files. */
#if (CY_P64_CJSON_VERSION_MAJOR != 1) || (CY_P64_CJSON_VERSION_MINOR != 3) || (CY_P64_CJSON_VERSION_PATCH != 2)
    #error cy_p64_cJSON.h and cy_p64_cJSON.c have different versions. Make sure that both have the same.
#endif


/* Case-insensitive strcmp */
static int cy_p64_cJSON_strcasecmp(const unsigned char *s1, const unsigned char *s2)
{
    if (s1 == NULL)
    {
        return (s1 == s2) ? 0 : 1; /* both NULL? */
    }
    if (s2 == NULL)
    {
        return 1;
    }
    for(; tolower((int)*s1) == tolower((int)*s2); ++s1, ++s2)
    {
        if (*s1 == 0u)
        {
            return 0;
        }
    }

    return (tolower((int)*s1) - tolower((int)*s2));
}

static void *(*cy_p64_cJSON_malloc)(size_t sz) = (void *(*)(size_t sz))cy_p64_malloc;
static void (*cy_p64_cJSON_free)(void *ptr) = (void (*)(void *ptr))cy_p64_free;

static unsigned char* cy_p64_cJSON_strdup(const unsigned char* str)
{
    size_t len = 0;
    unsigned char *copy = NULL;

    if (str == NULL)
    {
        return NULL;
    }

    len = strlen((const char*)str) + 1u;
    if ((copy = (unsigned char*)cy_p64_cJSON_malloc(len)) == NULL)
    {
        return NULL;
    }
    (void)memcpy(copy, str, len);

    return copy;
}

void cy_p64_cJSON_InitHooks(cy_p64_cJSON_Hooks* hooks)
{
    /* Init hooks */
    if(hooks != NULL)
    {
        if(hooks->malloc_fn != NULL)
        {
            cy_p64_cJSON_malloc = hooks->malloc_fn;
        }
        if(hooks->free_fn != NULL)
        {
            cy_p64_cJSON_free = hooks->free_fn;
        }
    }
}

/* Internal constructor. */
static cy_p64_cJSON *cy_p64_cJSON_New_Item(void)
{
    cy_p64_cJSON* node = (cy_p64_cJSON*)cy_p64_cJSON_malloc(sizeof(cy_p64_cJSON));
    if (node != NULL)
    {
        (void)memset(node, 0, sizeof(cy_p64_cJSON));
    }

    return node;
}

/* Delete the cy_p64_cJSON structure. */
void cy_p64_cJSON_Delete(cy_p64_cJSON *c)
{
    cy_p64_cJSON *next = NULL;
    while (c != NULL)
    {
        next = c->next;
        if (!(c->type & CY_P64_cJSON_IsReference) && c->child)
        {
            cy_p64_cJSON_Delete(c->child);
        }
        if (!(c->type & CY_P64_cJSON_IsReference) && c->valuestring)
        {
            cy_p64_cJSON_free(c->valuestring);
        }
        if (!(c->type & CY_P64_cJSON_StringIsConst) && c->string)
        {
            cy_p64_cJSON_free(c->string);
        }
        cy_p64_cJSON_free(c);
        c = next;
    }
}

/* Parse the input text to generate a number, and populate the result into item. */
static const unsigned char *parse_number(cy_p64_cJSON * const item, const unsigned char * const input)
{
    long long number = 0;
    unsigned char *after_end = NULL;

    if (input == NULL)
    {
        return NULL;
    }

    number = strtoll((const char*)input, (char**)&after_end, 10);
    if (input == after_end)
    {
        return NULL; /* parse_error */
    }

    /* Use saturation in case of overflow */
    if (number >= UINT32_MAX)
    {
        item->valueint = UINT32_MAX;
    }
    else if (number <= 0u)
    {
        item->valueint = 0u;
    }
    else
    {
        item->valueint = (uint32_t)number;
    }

    item->type = CY_P64_cJSON_Number;

    return after_end;
}

typedef struct
{
    unsigned char *buffer;
    size_t length;
    size_t offset;
    cjbool noalloc;
} printbuffer;

/* realloc printbuffer if necessary to have at least "needed" bytes more */
static unsigned char* ensure(printbuffer *p, size_t needed)
{
    unsigned char *newbuffer = NULL;
    size_t newsize = 0;

    if (needed > INT_MAX)
    {
        /* Sizes bigger than INT_MAX are not currently supported */
        return NULL;
    }

    if (!p || !p->buffer)
    {
        return NULL;
    }
    needed += p->offset;
    if (needed <= p->length)
    {
        return p->buffer + p->offset;
    }

    if (p->noalloc) {
        return NULL;
    }

    /* Calculate the new buffer size */
    newsize = needed * 2;
    if (newsize > INT_MAX)
    {
        /* Overflow of int, use INT_MAX if possible */
        if (needed <= INT_MAX)
        {
            newsize = INT_MAX;
        }
        else
        {
            return NULL;
        }
    }

    newbuffer = (unsigned char*)cy_p64_cJSON_malloc(newsize);
    if (!newbuffer)
    {
        cy_p64_cJSON_free(p->buffer);
        p->length = 0;
        p->buffer = NULL;

        return NULL;
    }
    else
    {
        (void)memcpy(newbuffer, p->buffer, p->length);
    }
    cy_p64_cJSON_free(p->buffer);
    p->length = newsize;
    p->buffer = newbuffer;

    return newbuffer + p->offset;
}

/* Calculate the new length of the string in the printbuffer */
static size_t update(const printbuffer *p)
{
    const unsigned char *str = NULL;
    if (!p || !p->buffer)
    {
        return 0;
    }
    str = p->buffer + p->offset;

    return p->offset + strlen((const char*)str);
}

/* Render the number nicely from the given item into a string. */
static unsigned char *print_number(const cy_p64_cJSON *item, printbuffer *p)
{
    unsigned char *str = NULL;

    /* Special case for 0. */
    if (item->valueint == 0)
    {
        if (p)
        {
            str = ensure(p, 2);
        }
        else
        {
            str = (unsigned char*)cy_p64_cJSON_malloc(2);
        }
        if (str)
        {
            strcpy((char*)str,"0");
        }
    }
    /* The value is an int */
    else
    {
        if (p)
        {
            str = ensure(p, 21);
        }
        else
        {
            /* 2^64+1 can be represented in 21 chars. */
            str = (unsigned char*)cy_p64_cJSON_malloc(21);
        }
        if (str)
        {
            /* Use of sprintf is safe because str buffer size is calculated beforehand */
            sprintf((char*)str, "%lu", (unsigned long)item->valueint);
        }
    }
    return str;
}

/* Parse the 4-digit hexadecimal number */
static unsigned parse_hex4(const unsigned char * const input)
{
    unsigned int h = 0;
    size_t i = 0;

    for (i = 0; i < 4; i++)
    {
        /* Parse the digit */
        if ((input[i] >= '0') && (input[i] <= '9'))
        {
            h += (unsigned int) input[i] - '0';
        }
        else if ((input[i] >= 'A') && (input[i] <= 'F'))
        {
            h += (unsigned int) 10 + input[i] - 'A';
        }
        else if ((input[i] >= 'a') && (input[i] <= 'f'))
        {
            h += (unsigned int) 10 + input[i] - 'a';
        }
        else /* Invalid */
        {
            return 0;
        }

        if (i < 3)
        {
            /* Shift left to make place for the next nibble */
            h = h << 4;
        }
    }

    return h;
}

/* Converts a UTF-16 literal to UTF-8
 * A literal can be one or two sequences of the form \uXXXX */
static unsigned char utf16_literal_to_utf8(const unsigned char * const input_pointer, const unsigned char * const input_end, unsigned char **output_pointer, const unsigned char **error_pointer)
{
    /* The first bytes of UTF8 encoding for a given length in bytes */
    static const unsigned char firstByteMark[5] =
    {
        0x00, /* should never happen */
        0x00, /* 0xxxxxxx */
        0xC0, /* 110xxxxx */
        0xE0, /* 1110xxxx */
        0xF0 /* 11110xxx */
    };

    long unsigned int codepoint = 0;
    unsigned int first_code = 0;
    const unsigned char *first_sequence = input_pointer;
    unsigned char utf8_length = 0;
    unsigned char sequence_length = 0;

    if ((input_end - first_sequence) < 6)
    {
        /* The input ends unexpectedly */
        *error_pointer = first_sequence;
        goto fail;
    }

    /* Get the first utf16 sequence */
    first_code = parse_hex4(first_sequence + 2);

    /* Check that the code is valid */
    if (((first_code >= 0xDC00) && (first_code <= 0xDFFF)) || (first_code == 0))
    {
        *error_pointer = first_sequence;
        goto fail;
    }

    /* UTF16 surrogate pair */
    if ((first_code >= 0xD800) && (first_code <= 0xDBFF))
    {
        const unsigned char *second_sequence = first_sequence + 6;
        unsigned int second_code = 0;
        sequence_length = 12; /* \uXXXX\uXXXX */

        if ((input_end - second_sequence) < 6)
        {
            /* The input ends unexpectedly */
            *error_pointer = first_sequence;
            goto fail;
        }

        if ((second_sequence[0] != '\\') || (second_sequence[1] != 'u'))
        {
            /* Missing second half of the surrogate pair */
            *error_pointer = first_sequence;
            goto fail;
        }

        /* Get the second utf16 sequence */
        second_code = parse_hex4(second_sequence + 2);
        /* check that the code is valid */
        if ((second_code < 0xDC00) || (second_code > 0xDFFF))
        {
            /* invalid second half of the surrogate pair */
            *error_pointer = first_sequence;
            goto fail;
        }


        /* Calculate the unicode codepoint from the surrogate pair */
        codepoint = 0x10000 + (((first_code & 0x3FF) << 10) | (second_code & 0x3FF));
    }
    else
    {
        sequence_length = 6; /* \uXXXX */
        codepoint = first_code;
    }

    /* Encode as UTF-8
     * Takes at maximum 4 bytes to encode:
     * 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx */
    if (codepoint < 0x80)
    {
        /* Normal ascii, encoding 0xxxxxxx */
        utf8_length = 1;
    }
    else if (codepoint < 0x800)
    {
        /* Two bytes, encoding 110xxxxx 10xxxxxx */
        utf8_length = 2;
    }
    else if (codepoint < 0x10000)
    {
        /* Three bytes, encoding 1110xxxx 10xxxxxx 10xxxxxx */
        utf8_length = 3;
    }
    else if (codepoint <= 0x10FFFF)
    {
        /* Four bytes, encoding 1110xxxx 10xxxxxx 10xxxxxx 10xxxxxx */
        utf8_length = 4;
    }
    else
    {
        /* Invalid unicode codepoint */
        *error_pointer = first_sequence;
        goto fail;
    }

    /* Encode as utf8 */
    switch (utf8_length)
    {
        case 4:
            /* 10xxxxxx */
            (*output_pointer)[3] = (unsigned char)((codepoint | 0x80) & 0xBF);
            codepoint >>= 6;
            break;
        case 3:
            /* 10xxxxxx */
            (*output_pointer)[2] = (unsigned char)((codepoint | 0x80) & 0xBF);
            codepoint >>= 6;
            break;
        case 2:
            (*output_pointer)[1] = (unsigned char)((codepoint | 0x80) & 0xBF);
            codepoint >>= 6;
            break;
        case 1:
            /* Depending on the length in bytes, this determines the
               encoding of the first UTF8 byte */
            (*output_pointer)[0] = (unsigned char)((codepoint | firstByteMark[utf8_length]) & 0xFF);
            break;
        default:
            *error_pointer = first_sequence;
            goto fail;
    }
    *output_pointer += utf8_length;

    return sequence_length;

fail:
    return 0;
}

/* Parse the input text into an unescaped cinput, and populate item. */
static const unsigned char *parse_string(cy_p64_cJSON * const item, const unsigned char * const input, const unsigned char ** const error_pointer)
{
    const unsigned char *input_pointer = input + 1;
    const unsigned char *input_end = input + 1;
    unsigned char *output_pointer = NULL;
    unsigned char *output = NULL;

    /* Not a string */
    if (*input != '\"')
    {
        *error_pointer = input;
        goto fail;
    }

    {
        /* Calculate the approximate size of the output (overestimate) */
        size_t allocation_length = 0;
        size_t skipped_bytes = 0;
        while ((*input_end != '\"') && (*input_end != '\0'))
        {
            /* It is an escape sequence */
            if (input_end[0] == '\\')
            {
                if (input_end[1] == '\0')
                {
                    /* Prevent a buffer overflow when the last input character is a backslash */
                    goto fail;
                }
                skipped_bytes++;
                input_end++;
            }
            input_end++;
        }
        if (*input_end == '\0')
        {
            goto fail; /* The string ended unexpectedly */
        }

        /* This is at most how much we need for the output */
        allocation_length = (size_t) (input_end - input) - skipped_bytes;
        output = (unsigned char*)cy_p64_cJSON_malloc(allocation_length + sizeof('\0'));
        if (output == NULL)
        {
            goto fail; /* Allocation failure */
        }
    }

    output_pointer = output;
    /* Loop through the string literal */
    while (input_pointer < input_end)
    {
        if (*input_pointer != '\\')
        {
            *output_pointer++ = *input_pointer++;
        }
        /* An escape sequence */
        else
        {
            unsigned char sequence_length = 2;
            switch (input_pointer[1])
            {
                case 'b':
                    *output_pointer++ = '\b';
                    break;
                case 'f':
                    *output_pointer++ = '\f';
                    break;
                case 'n':
                    *output_pointer++ = '\n';
                    break;
                case 'r':
                    *output_pointer++ = '\r';
                    break;
                case 't':
                    *output_pointer++ = '\t';
                    break;
                case '\"':
                case '\\':
                case '/':
                    *output_pointer++ = input_pointer[1];
                    break;

                /* UTF-16 literal */
                case 'u':
                    sequence_length = utf16_literal_to_utf8(input_pointer, input_end, &output_pointer, error_pointer);
                    if (sequence_length == 0)
                    {
                        /* Failed to convert UTF16-literal to UTF-8 */
                        goto fail;
                    }
                    break;

                default:
                    *error_pointer = input_pointer;
                    goto fail;
            }
            input_pointer += sequence_length;
        }
    }

    /* A zero terminate the output */
    *output_pointer = '\0';

    item->type = CY_P64_cJSON_String;
    item->valuestring = (char*)output;

    return input_end + 1;

fail:
    if (output != NULL)
    {
        cy_p64_cJSON_free(output);
    }

    return NULL;
}

/* Render the cstring provided to an escaped version that can be printed. */
static unsigned char *print_string_ptr(const unsigned char *str, printbuffer *p)
{
    const unsigned char *ptr = NULL;
    unsigned char *ptr2 = NULL;
    unsigned char *out = NULL;
    size_t len = 0;
    cjbool flag = cj_false;
    unsigned char token = '\0';

    /* An empty string */
    if (!str)
    {
        if (p)
        {
            out = ensure(p, 3);
        }
        else
        {
            out = (unsigned char*)cy_p64_cJSON_malloc(3);
        }
        if (!out)
        {
            return NULL;
        }
        strcpy((char*)out, "\"\"");

        return out;
    }

    /* Set "flag" to 1 if something needs to be escaped */
    for (ptr = str; *ptr; ptr++)
    {
        flag |= (((*ptr > 0) && (*ptr < 32)) /* unprintable characters */
                || (*ptr == '\"') /* double quote */
                || (*ptr == '\\')) /* backslash */
            ? 1
            : 0;
    }
    /* No characters to be escaped */
    if (!flag)
    {
        len = (size_t)(ptr - str);
        if (p)
        {
            out = ensure(p, len + 3);
        }
        else
        {
            out = (unsigned char*)cy_p64_cJSON_malloc(len + 3);
        }
        if (!out)
        {
            return NULL;
        }

        ptr2 = out;
        *ptr2++ = '\"';
        strcpy((char*)ptr2, (const char*)str);
        ptr2[len] = '\"';
        ptr2[len + 1] = '\0';

        return out;
    }

    ptr = str;
    /* Calculate additional space needed for escaping */
    while ((token = *ptr))
    {
        ++len;
        if (strchr("\"\\\b\f\n\r\t", token))
        {
            len++; /* +1 for the backslash */
        }
        else if (token < 32)
        {
            len += 5; /* +5 for \uXXXX */
        }
        ptr++;
    }

    if (p)
    {
        out = ensure(p, len + 3);
    }
    else
    {
        out = (unsigned char*)cy_p64_cJSON_malloc(len + 3);
    }
    if (!out)
    {
        return NULL;
    }

    ptr2 = out;
    ptr = str;
    *ptr2++ = '\"';
    /* copy the string */
    while (*ptr)
    {
        if ((*ptr > 31) && (*ptr != '\"') && (*ptr != '\\'))
        {
            /* normal character, copy */
            *ptr2++ = *ptr++;
        }
        else
        {
            /* character needs to be escaped */
            *ptr2++ = '\\';
            switch (token = *ptr++)
            {
                case '\\':
                    *ptr2++ = '\\';
                    break;
                case '\"':
                    *ptr2++ = '\"';
                    break;
                case '\b':
                    *ptr2++ = 'b';
                    break;
                case '\f':
                    *ptr2++ = 'f';
                    break;
                case '\n':
                    *ptr2++ = 'n';
                    break;
                case '\r':
                    *ptr2++ = 'r';
                    break;
                case '\t':
                    *ptr2++ = 't';
                    break;
                default:
                    /* Escape and print as unicode codepoint */
                    /* Use of sprintf is safe because the ptr2 buffer size is calculated beforehand */
                    sprintf((char*)ptr2, "u%04x", token);
                    ptr2 += 5;
                    break;
            }
        }
    }
    *ptr2++ = '\"';
    *ptr2++ = '\0';

    return out;
}

/* Invoke print_string_ptr (which is useful) on an item. */
static unsigned char *print_string(const cy_p64_cJSON *item, printbuffer *p)
{
    return print_string_ptr((unsigned char*)item->valuestring, p);
}

/* Predeclare these prototypes. */
static const unsigned char *parse_value(cy_p64_cJSON * const item, const unsigned char * const input, const unsigned char ** const ep);
static unsigned char *print_value(const cy_p64_cJSON *item, size_t depth, cjbool fmt, printbuffer *p);
static const unsigned char *parse_array(cy_p64_cJSON * const item, const unsigned char *input, const unsigned char ** const ep);
static unsigned char *print_array(const cy_p64_cJSON *item, size_t depth, cjbool fmt, printbuffer *p);
static const unsigned char *parse_object(cy_p64_cJSON * const item, const unsigned char *input, const unsigned char ** const ep);
static unsigned char *print_object(const cy_p64_cJSON *item, size_t depth, cjbool fmt, printbuffer *p);

/* The utility to jump whitespace and cr/lf */
static const unsigned char *skip(const unsigned char *in)
{
    while (in && *in && (*in <= 32))
    {
        in++;
    }

    return in;
}

/* Parse an object - create a new root, and populate. */
cy_p64_cJSON *cy_p64_cJSON_ParseWithOpts(const char *value, const char **return_parse_end, cjbool require_null_terminated)
{
    const unsigned char *end = NULL;
    /* Use the global error pointer if no specific one was given */
    const unsigned char **ep = return_parse_end ? (const unsigned char**)return_parse_end : &global_ep;
    cy_p64_cJSON *c = cy_p64_cJSON_New_Item();
    *ep = NULL;
    if (!c) /* memory fail */
    {
        return NULL;
    }

    end = parse_value(c, skip((const unsigned char*)value), ep);
    if (!end)
    {
        /* Parse failure. ep is set. */
        cy_p64_cJSON_Delete(c);
        return NULL;
    }

    /* If we require null-terminated JSON without appended garbage, skip and then check for a null terminator */
    if (require_null_terminated)
    {
        end = skip(end);
        if (*end)
        {
            cy_p64_cJSON_Delete(c);
            *ep = end;
            return NULL;
        }
    }
    if (return_parse_end)
    {
        *return_parse_end = (const char*)end;
    }

    return c;
}

/* Default options for cy_p64_cJSON_Parse */
cy_p64_cJSON *cy_p64_cJSON_Parse(const char *value)
{
    return cy_p64_cJSON_ParseWithOpts(value, 0, 0);
}

/* Render a cy_p64_cJSON item/entity/structure to text. */
char *cy_p64_cJSON_Print(const cy_p64_cJSON *item)
{
    return (char*)print_value(item, 0, 1, 0);
}

char *cy_p64_cJSON_PrintUnformatted(const cy_p64_cJSON *item)
{
    return (char*)print_value(item, 0, 0, 0);
}

char *cy_p64_cJSON_PrintBuffered(const cy_p64_cJSON *item, int prebuffer, cjbool fmt)
{
    printbuffer p;

    if (prebuffer < 0)
    {
        return NULL;
    }

    p.buffer = (unsigned char*)cy_p64_cJSON_malloc((size_t)prebuffer);
    if (!p.buffer)
    {
        return NULL;
    }

    p.length = (size_t)prebuffer;
    p.offset = 0;
    p.noalloc = cj_false;

    return (char*)print_value(item, 0, fmt, &p);
}

int cy_p64_cJSON_PrintPreallocated(cy_p64_cJSON *item, char *buf, const int len, const cjbool fmt)
{
    printbuffer p;

    if (len < 0)
    {
        return cj_false;
    }

    p.buffer = (unsigned char*)buf;
    p.length = (size_t)len;
    p.offset = 0;
    p.noalloc = cj_true;
    return print_value(item, 0, fmt, &p) != NULL;
}

/* Parser core - when encountering text, process appropriately. */
static const unsigned  char *parse_value(cy_p64_cJSON * const item, const unsigned char * const input, const unsigned char ** const error_pointer)
{
    if (input == NULL)
    {
        return NULL; /* no input */
    }

    /* Parse the different types of values */
    /* null */
    if (!strncmp((const char*)input, "null", 4))
    {
        item->type = CY_P64_cJSON_NULL;
        return input + 4;
    }
    /* False */
    if (!strncmp((const char*)input, "false", 5))
    {
        item->type = CY_P64_cJSON_False;
        return input + 5;
    }
    /* True */
    if (!strncmp((const char*)input, "true", 4))
    {
        item->type = CY_P64_cJSON_True;
        item->valueint = 1;
        return input + 4;
    }
    /* string */
    if (*input == '\"')
    {
        return parse_string(item, input, error_pointer);
    }
    /* Number */
    if ((*input == '-') || ((*input >= '0') && (*input <= '9')))
    {
        return parse_number(item, input);
    }
    /* Array */
    if (*input == '[')
    {
        return parse_array(item, input, error_pointer);
    }
    /* Object */
    if (*input == '{')
    {
        return parse_object(item, input, error_pointer);
    }

    /* Failure. */
    *error_pointer = input;
    return NULL;
}

/* Render a value to text. */
static unsigned char *print_value(const cy_p64_cJSON *item, size_t depth, cjbool fmt, printbuffer *p)
{
    unsigned char *out = NULL;

    if (!item)
    {
        return NULL;
    }
    if (p)
    {
        switch ((item->type) & 0xFF)
        {
            case CY_P64_cJSON_NULL:
                out = ensure(p, 5);
                if (out)
                {
                    strcpy((char*)out, "null");
                }
                break;
            case CY_P64_cJSON_False:
                out = ensure(p, 6);
                if (out)
                {
                    strcpy((char*)out, "false");
                }
                break;
            case CY_P64_cJSON_True:
                out = ensure(p, 5);
                if (out)
                {
                    strcpy((char*)out, "true");
                }
                break;
            case CY_P64_cJSON_Number:
                out = print_number(item, p);
                break;
            case CY_P64_cJSON_Raw:
            {
                size_t raw_length = 0;
                if (item->valuestring == NULL)
                {
                    if (!p->noalloc)
                    {
                        cy_p64_cJSON_free(p->buffer);
                    }
                    out = NULL;
                    break;
                }

                raw_length = strlen(item->valuestring) + sizeof('\0');
                out = ensure(p, raw_length);
                if (out)
                {
                    (void)memcpy(out, item->valuestring, raw_length);
                }
                break;
            }
            case CY_P64_cJSON_String:
                out = print_string(item, p);
                break;
            case CY_P64_cJSON_Array:
                out = print_array(item, depth, fmt, p);
                break;
            case CY_P64_cJSON_Object:
                out = print_object(item, depth, fmt, p);
                break;
            default:
                out = NULL;
                break;
        }
    }
    else
    {
        switch ((item->type) & 0xFF)
        {
            case CY_P64_cJSON_NULL:
                out = cy_p64_cJSON_strdup((const unsigned char*)"null");
                break;
            case CY_P64_cJSON_False:
                out = cy_p64_cJSON_strdup((const unsigned char*)"false");
                break;
            case CY_P64_cJSON_True:
                out = cy_p64_cJSON_strdup((const unsigned char*)"true");
                break;
            case CY_P64_cJSON_Number:
                out = print_number(item, 0);
                break;
            case CY_P64_cJSON_Raw:
                out = cy_p64_cJSON_strdup((unsigned char*)item->valuestring);
                break;
            case CY_P64_cJSON_String:
                out = print_string(item, 0);
                break;
            case CY_P64_cJSON_Array:
                out = print_array(item, depth, fmt, 0);
                break;
            case CY_P64_cJSON_Object:
                out = print_object(item, depth, fmt, 0);
                break;
            default:
                out = NULL;
                break;
        }
    }

    return out;
}

/* Build an array from input text. */
static const unsigned char *parse_array(cy_p64_cJSON * const item, const unsigned char *input, const unsigned char ** const error_pointer)
{
    cy_p64_cJSON *head = NULL; /* head of the linked list */
    cy_p64_cJSON *current_item = NULL;

    if (*input != '[')
    {
        /* Not an array */
        *error_pointer = input;
        goto fail;
    }

    input = skip(input + 1); /* skip whitespace */
    if (*input == ']')
    {
        /* empty array */
        goto success;
    }

    /* Step back to the character in front of the first element */
    input--;
    /* Loop through the comma separated array elements */
    do
    {
        /* Allocate the next item */
        cy_p64_cJSON *new_item = cy_p64_cJSON_New_Item();
        if (new_item == NULL)
        {
            goto fail; /* Allocation failure */
        }

        /* Attach the next item to the list */
        if (head == NULL)
        {
            /* Start a linked list */
            current_item = head = new_item;
        }
        else
        {
            /* Add to the end and advance */
            current_item->next = new_item;
            new_item->prev = current_item;
            current_item = new_item;
        }

        /* Parse the next value */
        input = skip(input + 1); /* skip whitespace before value */
        input = parse_value(current_item, input, error_pointer);
        input = skip(input); /* skip whitespace after value */
        if (input == NULL)
        {
            goto fail; /* Failed to parse value */
        }
    }
    while (*input == ',');

    if (*input != ']')
    {
        *error_pointer = input;
        goto fail; /* Expected the end of the array */
    }

success:
    item->type = CY_P64_cJSON_Array;
    item->child = head;

    return input + 1;

fail:
    if (head != NULL)
    {
        cy_p64_cJSON_Delete(head);
    }

    return NULL;
}

/* Render an array to text */
static unsigned char *print_array(const cy_p64_cJSON *item, size_t depth, cjbool fmt, printbuffer *p)
{
    unsigned char *out = NULL;
    unsigned char *ptr = NULL;
    unsigned char *ret = NULL;
    size_t len = 5;
    cy_p64_cJSON *child = item->child;
    size_t numentries = 0;
    size_t i = 0;
    cjbool fail = cj_false;

    /* How many entries in the array? */
    while (child)
    {
        numentries++;
        child = child->next;
    }

    /* Explicitly handle numentries == 0 */
    if (!numentries)
    {
        if (p)
        {
            out = ensure(p, 3);
        }
        else
        {
            out = (unsigned char*)cy_p64_cJSON_malloc(3);
        }
        if (out)
        {
            strcpy((char*)out, "[]");
        }

        return out;
    }

    if (p)
    {
        /* Compose the output array. */
        /* Opening square bracket */
        i = p->offset;
        ptr = ensure(p, 1);
        if (!ptr)
        {
            return NULL;
        }
        *ptr = '[';
        p->offset++;

        child = item->child;
        while (child && !fail)
        {
            if (!print_value(child, depth + 1, fmt, p))
            {
                return NULL;
            }
            p->offset = update(p);
            if (child->next)
            {
                len = fmt ? 2 : 1;
                ptr = ensure(p, len + 1);
                if (!ptr)
                {
                    return NULL;
                }
                *ptr++ = ',';
                if(fmt)
                {
                    *ptr++ = ' ';
                }
                *ptr = '\0';
                p->offset += len;
            }
            child = child->next;
        }
        ptr = ensure(p, 2);
        if (!ptr)
        {
            return NULL;
        }
        *ptr++ = ']';
        *ptr = '\0';
        out = (p->buffer) + i;
    }
    else
    {
        unsigned char **entries;

        /* Allocate an array to hold the pointers to all printed values */
        entries = (unsigned char**)cy_p64_cJSON_malloc(numentries * sizeof(unsigned char*));
        if (!entries)
        {
            return NULL;
        }
        (void)memset(entries, '\0', numentries * sizeof(unsigned char*));

        /* Retrieve all the results: */
        child = item->child;
        while (child && !fail)
        {
            ret = print_value(child, depth + 1, fmt, 0);
            entries[i++] = ret;
            if (ret)
            {
                len += strlen((char*)ret) + 2 + (fmt ? 1 : 0);
            }
            else
            {
                fail = cj_true;
            }
            child = child->next;
        }

        /* If no fail, try to malloc the output string */
        if (!fail)
        {
            out = (unsigned char*)cy_p64_cJSON_malloc(len);
        }
        /* If that fails, we fail. */
        if (!out)
        {
            fail = cj_true;
        }

        /* Handle failure. */
        if (fail)
        {
            /* Free all the entries in the array */
            for (i = 0; i < numentries; i++)
            {
                if (entries[i])
                {
                    cy_p64_cJSON_free(entries[i]);
                }
            }
            cy_p64_cJSON_free(entries);
            return NULL;
        }

        /* Compose the output array. */
        *out='[';
        ptr = out + 1;
        *ptr = '\0';
        for (i = 0; i < numentries; i++)
        {
            size_t tmplen = 0;

            tmplen = strlen((char*)entries[i]);
            /* Use of memcpy is safe because the ptr buffer size is calculated beforehand */
            (void)memcpy(ptr, entries[i], tmplen);
            ptr += tmplen;
            if (i != (numentries - 1))
            {
                *ptr++ = ',';
                if(fmt)
                {
                    *ptr++ = ' ';
                }
                *ptr = '\0';
            }
            cy_p64_cJSON_free(entries[i]);
        }
        cy_p64_cJSON_free(entries);
        *ptr++ = ']';
        *ptr++ = '\0';
    }

    return out;
}

/* Build an object from the text. */
static const unsigned char *parse_object(cy_p64_cJSON * const item, const unsigned char *input, const unsigned char ** const error_pointer)
{
    cy_p64_cJSON *head = NULL; /* linked list head */
    cy_p64_cJSON *current_item = NULL;

    if (*input != '{')
    {
        *error_pointer = input;
        goto fail; /* not an object */
    }

    input = skip(input + 1); /* skip whitespace */
    if (*input == '}')
    {
        goto success; /* empty object */
    }

    /* Step back to the character in front of the first element */
    input--;
    /* Loop through the comma-separated array elements */
    do
    {
        /* Allocate the next item */
        cy_p64_cJSON *new_item = cy_p64_cJSON_New_Item();
        if (new_item == NULL)
        {
            goto fail; /* allocation failure */
        }

        /* Attach the next item to the list */
        if (head == NULL)
        {
            /* Start a linked list */
            current_item = head = new_item;
        }
        else
        {
            /* Add to the end and advance */
            current_item->next = new_item;
            new_item->prev = current_item;
            current_item = new_item;
        }

        /* Parse the name of the child */
        input = skip(input + 1); /* Skip whitespaces before the name */
        input = parse_string(current_item, input, error_pointer);
        input = skip(input); /* Skip whitespaces after the name */
        if (input == NULL)
        {
            goto fail; /* Fail to parse the name */
        }

        /* Swap the valuestring and string, because we parsed the name */
        current_item->string = current_item->valuestring;
        current_item->valuestring = NULL;

        if (*input != ':')
        {
            *error_pointer = input;
            goto fail; /* Invalid object */
        }

        /* Parse the value */
        input = skip(input + 1); /* Skip whitespaces before value */
        input = parse_value(current_item, input, error_pointer);
        input = skip(input); /* Skip whitespaces after the value */
        if (input == NULL)
        {
            goto fail; /* Failed to parse the value */
        }
    }
    while (*input == ',');

    if (*input != '}')
    {
        *error_pointer = input;
        goto fail; /* Expected the end of the object */
    }

success:
    item->type = CY_P64_cJSON_Object;
    item->child = head;

    return input + 1;

fail:
    if (head != NULL)
    {
        cy_p64_cJSON_Delete(head);
    }

    return NULL;
}

/* Render an object to text. */
static unsigned char *print_object(const cy_p64_cJSON *item, size_t depth, cjbool fmt, printbuffer *p)
{
    unsigned char **entries = NULL;
    unsigned char **names = NULL;
    unsigned char *out = NULL;
    unsigned char *ptr = NULL;
    unsigned char *ret = NULL;
    unsigned char *str = NULL;
    size_t len = 7;
    size_t i = 0;
    size_t j = 0;
    cy_p64_cJSON *child = item->child;
    size_t numentries = 0;
    cjbool fail = cj_false;

    /* Count the number of entries. */
    while (child)
    {
        numentries++;
        child = child->next;
    }

    /* Explicitly handle an empty object case */
    if (!numentries)
    {
        if (p)
        {
            out = ensure(p, fmt ? depth + 4 : 3);
        }
        else
        {
            out = (unsigned char*)cy_p64_cJSON_malloc(fmt ? depth + 4 : 3);
        }
        if (!out)
        {
            return NULL;
        }
        ptr = out;
        *ptr++ = '{';
        if (fmt) {
            *ptr++ = '\n';
            for (i = 0; i < depth; i++)
            {
                *ptr++ = '\t';
            }
        }
        *ptr++ = '}';
        *ptr++ = '\0';

        return out;
    }

    if (p)
    {
        /* Compose the output: */
        i = p->offset;
        len = fmt ? 2 : 1; /* fmt: {\n */
        ptr = ensure(p, len + 1);
        if (!ptr)
        {
            return NULL;
        }

        *ptr++ = '{';
        if (fmt)
        {
            *ptr++ = '\n';
        }
        *ptr = '\0';
        p->offset += len;

        child = item->child;
        depth++;
        while (child)
        {
            if (fmt)
            {
                ptr = ensure(p, depth);
                if (!ptr)
                {
                    return NULL;
                }
                for (j = 0; j < depth; j++)
                {
                    *ptr++ = '\t';
                }
                p->offset += depth;
            }

            /* Print the key */
            if (!print_string_ptr((unsigned char*)child->string, p))
            {
                return NULL;
            }
            p->offset = update(p);

            len = fmt ? 2 : 1;
            ptr = ensure(p, len);
            if (!ptr)
            {
                return NULL;
            }
            *ptr++ = ':';
            if (fmt)
            {
                *ptr++ = '\t';
            }
            p->offset+=len;

            /* Print the value */
            if (!print_value(child, depth, fmt, p))
            {
                return NULL;
            };
            p->offset = update(p);

            /* Print a comma if not the last */
            len = (size_t) (fmt ? 1 : 0) + (child->next ? 1 : 0);
            ptr = ensure(p, len + 1);
            if (!ptr)
            {
                return NULL;
            }
            if (child->next)
            {
                *ptr++ = ',';
            }

            if (fmt)
            {
                *ptr++ = '\n';
            }
            *ptr = '\0';
            p->offset += len;

            child = child->next;
        }

        ptr = ensure(p, fmt ? (depth + 1) : 2);
        if (!ptr)
        {
            return NULL;
        }
        if (fmt)
        {
            for (i = 0; i < (depth - 1); i++)
            {
                *ptr++ = '\t';
            }
        }
        *ptr++ = '}';
        *ptr = '\0';
        out = (p->buffer) + i;
    }
    else
    {
        /* Allocate space for the names and objects */
        entries = (unsigned char**)cy_p64_cJSON_malloc(numentries * sizeof(unsigned char*));
        if (!entries)
        {
            return NULL;
        }
        names = (unsigned char**)cy_p64_cJSON_malloc(numentries * sizeof(unsigned char*));
        if (!names)
        {
            cy_p64_cJSON_free(entries);
            return NULL;
        }
        (void)memset(entries, '\0', sizeof(unsigned char*) * numentries);
        (void)memset(names, '\0', sizeof(unsigned char*) * numentries);

        /* Collect all the results into our arrays: */
        child = item->child;
        depth++;
        if (fmt)
        {
            len += depth;
        }
        while (child && !fail)
        {
            names[i] = str = print_string_ptr((unsigned char*)child->string, 0); /* print key */
            entries[i++] = ret = print_value(child, depth, fmt, 0);
            if (str && ret)
            {
                len += strlen((char*)ret) + strlen((char*)str) + 2 + (fmt ? 2 + depth : 0);
            }
            else
            {
                fail = cj_true;
            }
            child = child->next;
        }

        /* Try to allocate the output string */
        if (!fail)
        {
            out = (unsigned char*)cy_p64_cJSON_malloc(len);
        }
        if (!out)
        {
            fail = cj_true;
        }

        /* Handle the failure */
        if (fail)
        {
            /* Free all the printed keys and values */
            for (i = 0; i < numentries; i++)
            {
                if (names[i])
                {
                    cy_p64_cJSON_free(names[i]);
                }
                if (entries[i])
                {
                    cy_p64_cJSON_free(entries[i]);
                }
            }
            cy_p64_cJSON_free(names);
            cy_p64_cJSON_free(entries);
            return NULL;
        }

        /* Compose the output: */
        *out = '{';
        ptr = out + 1;
        if (fmt)
        {
            *ptr++ = '\n';
        }
        *ptr = '\0';
        for (i = 0; i < numentries; i++)
        {
            size_t tmplen = 0;

            if (fmt)
            {
                for (j = 0; j < depth; j++)
                {
                    *ptr++='\t';
                }
            }
            tmplen = strlen((char*)names[i]);
            /* Use of memcpy is safe because the ptr buffer size is calculated beforehand */
            (void)memcpy(ptr, names[i], tmplen);
            ptr += tmplen;
            *ptr++ = ':';
            if (fmt)
            {
                *ptr++ = '\t';
            }
            strcpy((char*)ptr, (char*)entries[i]);
            ptr += strlen((char*)entries[i]);
            if (i != (numentries - 1))
            {
                *ptr++ = ',';
            }
            if (fmt)
            {
                *ptr++ = '\n';
            }
            *ptr = '\0';
            cy_p64_cJSON_free(names[i]);
            cy_p64_cJSON_free(entries[i]);
        }

        cy_p64_cJSON_free(names);
        cy_p64_cJSON_free(entries);
        if (fmt)
        {
            for (i = 0; i < (depth - 1); i++)
            {
                *ptr++ = '\t';
            }
        }
        *ptr++ = '}';
        *ptr++ = '\0';
    }

    return out;
}

/* Get the array size/item / object item. */
int cy_p64_cJSON_GetArraySize(const cy_p64_cJSON *array)
{
    cy_p64_cJSON *c = array->child;
    size_t i = 0;
    while(c)
    {
        i++;
        if(i >= INT_MAX)
        {
            i = INT_MAX;
            break;
        }
        c = c->next;
    }

    return (int)i;
}

cy_p64_cJSON *cy_p64_cJSON_GetArrayItem(const cy_p64_cJSON *array, int item)
{
    cy_p64_cJSON *c = array ? array->child : NULL;
    while (c && item > 0)
    {
        item--;
        c = c->next;
    }

    return c;
}

cy_p64_cJSON *cy_p64_cJSON_GetObjectItem(const cy_p64_cJSON *object, const char *string)
{
    cy_p64_cJSON *c = object ? object->child : NULL;
    while (c && cy_p64_cJSON_strcasecmp((unsigned char*)c->string, (const unsigned char*)string))
    {
        c = c->next;
    }
    return c;
}

cjbool cy_p64_cJSON_HasObjectItem(const cy_p64_cJSON *object, const char *string)
{
    return cy_p64_cJSON_GetObjectItem(object, string) ? 1 : 0;
}

/* Utility to hande the list array. */
static void suffix_object(cy_p64_cJSON *prev, cy_p64_cJSON *item)
{
    prev->next = item;
    item->prev = prev;
}

/* Utility to handle references. */
static cy_p64_cJSON *create_reference(const cy_p64_cJSON *item)
{
    cy_p64_cJSON *ref = cy_p64_cJSON_New_Item();
    if (!ref)
    {
        return NULL;
    }
    (void)memcpy(ref, item, sizeof(cy_p64_cJSON));
    ref->string = NULL;
    ref->type |= CY_P64_cJSON_IsReference;
    ref->next = ref->prev = NULL;
    return ref;
}

/* Add an item to the array/object. */
void cy_p64_cJSON_AddItemToArray(cy_p64_cJSON *array, cy_p64_cJSON *item)
{
    cy_p64_cJSON *child = NULL;

    if ((item == NULL) || (array == NULL))
    {
        return;
    }

    child = array->child;

    if (child == NULL)
    {
        /* The list is empty, start a new one */
        array->child = item;
    }
    else
    {
        /* Append to the end */
        while (child->next)
        {
            child = child->next;
        }
        suffix_object(child, item);
    }
}

void   cy_p64_cJSON_AddItemToObject(cy_p64_cJSON *object, const char *string, cy_p64_cJSON *item)
{
    /* Call cy_p64_cJSON_AddItemToObjectCS for code reuse */
    cy_p64_cJSON_AddItemToObjectCS(object, (char*)cy_p64_cJSON_strdup((const unsigned char*)string), item);
    /* Remove the CY_P64_cJSON_StringIsConst flag */
    item->type &= ~CY_P64_cJSON_StringIsConst;
}

/* Add an item to an object with a constant string as the key */
void   cy_p64_cJSON_AddItemToObjectCS(cy_p64_cJSON *object, const char *string, cy_p64_cJSON *item)
{
    if (!item)
    {
        return;
    }
    if (!(item->type & CY_P64_cJSON_StringIsConst) && item->string)
    {
        cy_p64_cJSON_free(item->string);
    }
#if defined ( __GNUC__ )
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
#endif /* ( __GNUC__ ) */
    item->string = (char*)string;
#if defined ( __GNUC__ )
#pragma GCC diagnostic pop
#endif /* ( __GNUC__ ) */
    item->type |= CY_P64_cJSON_StringIsConst;
    cy_p64_cJSON_AddItemToArray(object, item);
}

void cy_p64_cJSON_AddItemReferenceToArray(cy_p64_cJSON *array, cy_p64_cJSON *item)
{
    cy_p64_cJSON_AddItemToArray(array, create_reference(item));
}

void cy_p64_cJSON_AddItemReferenceToObject(cy_p64_cJSON *object, const char *string, cy_p64_cJSON *item)
{
    cy_p64_cJSON_AddItemToObject(object, string, create_reference(item));
}

static cy_p64_cJSON *DetachItemFromArray(cy_p64_cJSON *array, size_t which)
{
    cy_p64_cJSON *c = array->child;
    while (c && (which > 0))
    {
        c = c->next;
        which--;
    }
    if (!c)
    {
        /* The item doesn't exist */
        return NULL;
    }
    if (c->prev)
    {
        /* Not the first element */
        c->prev->next = c->next;
    }
    if (c->next)
    {
        c->next->prev = c->prev;
    }
    if (c==array->child)
    {
        array->child = c->next;
    }
    /* Make sure the detached item doesn't point anywhere anymore */
    c->prev = c->next = NULL;

    return c;
}
cy_p64_cJSON *cy_p64_cJSON_DetachItemFromArray(cy_p64_cJSON *array, int which)
{
    if (which < 0)
    {
        return NULL;
    }

    return DetachItemFromArray(array, (size_t)which);
}

void cy_p64_cJSON_DeleteItemFromArray(cy_p64_cJSON *array, int which)
{
    cy_p64_cJSON_Delete(cy_p64_cJSON_DetachItemFromArray(array, which));
}

cy_p64_cJSON *cy_p64_cJSON_DetachItemFromObject(cy_p64_cJSON *object, const char *string)
{
    size_t i = 0;
    cy_p64_cJSON *c = object->child;
    while (c && cy_p64_cJSON_strcasecmp((unsigned char*)c->string, (const unsigned char*)string))
    {
        i++;
        c = c->next;
    }
    if (c)
    {
        return DetachItemFromArray(object, i);
    }

    return NULL;
}

void cy_p64_cJSON_DeleteItemFromObject(cy_p64_cJSON *object, const char *string)
{
    cy_p64_cJSON_Delete(cy_p64_cJSON_DetachItemFromObject(object, string));
}

/* Replace the array/object items with new ones. */
void cy_p64_cJSON_InsertItemInArray(cy_p64_cJSON *array, int which, cy_p64_cJSON *newitem)
{
    cy_p64_cJSON *c = array->child;
    while (c && (which > 0))
    {
        c = c->next;
        which--;
    }
    if (!c)
    {
        cy_p64_cJSON_AddItemToArray(array, newitem);
        return;
    }
    newitem->next = c;
    newitem->prev = c->prev;
    c->prev = newitem;
    if (c == array->child)
    {
        array->child = newitem;
    }
    else
    {
        newitem->prev->next = newitem;
    }
}

static void ReplaceItemInArray(cy_p64_cJSON *array, size_t which, cy_p64_cJSON *newitem)
{
    cy_p64_cJSON *c = array->child;
    while (c && (which > 0))
    {
        c = c->next;
        which--;
    }
    if (!c)
    {
        return;
    }
    newitem->next = c->next;
    newitem->prev = c->prev;
    if (newitem->next)
    {
        newitem->next->prev = newitem;
    }
    if (c == array->child)
    {
        array->child = newitem;
    }
    else
    {
        newitem->prev->next = newitem;
    }
    c->next = c->prev = NULL;
    cy_p64_cJSON_Delete(c);
}
void cy_p64_cJSON_ReplaceItemInArray(cy_p64_cJSON *array, int which, cy_p64_cJSON *newitem)
{
    if (which < 0)
    {
        return;
    }

    ReplaceItemInArray(array, (size_t)which, newitem);
}

void cy_p64_cJSON_ReplaceItemInObject(cy_p64_cJSON *object, const char *string, cy_p64_cJSON *newitem)
{
    size_t i = 0;
    cy_p64_cJSON *c = object->child;
    while(c && cy_p64_cJSON_strcasecmp((unsigned char*)c->string, (const unsigned char*)string))
    {
        i++;
        c = c->next;
    }
    if(c)
    {
        /* free the old string if not const */
        if (!(newitem->type & CY_P64_cJSON_StringIsConst) && newitem->string)
        {
             cy_p64_cJSON_free(newitem->string);
        }

        newitem->string = (char*)cy_p64_cJSON_strdup((const unsigned char*)string);
        ReplaceItemInArray(object, i, newitem);
    }
}

/* Create basic types: */
cy_p64_cJSON *cy_p64_cJSON_CreateNull(void)
{
    cy_p64_cJSON *item = cy_p64_cJSON_New_Item();
    if(item)
    {
        item->type = CY_P64_cJSON_NULL;
    }

    return item;
}

cy_p64_cJSON *cy_p64_cJSON_CreateTrue(void)
{
    cy_p64_cJSON *item = cy_p64_cJSON_New_Item();
    if(item)
    {
        item->type = CY_P64_cJSON_True;
    }

    return item;
}

cy_p64_cJSON *cy_p64_cJSON_CreateFalse(void)
{
    cy_p64_cJSON *item = cy_p64_cJSON_New_Item();
    if(item)
    {
        item->type = CY_P64_cJSON_False;
    }

    return item;
}

cy_p64_cJSON *cy_p64_cJSON_CreateBool(cjbool b)
{
    cy_p64_cJSON *item = cy_p64_cJSON_New_Item();
    if(item)
    {
        item->type = b ? CY_P64_cJSON_True : CY_P64_cJSON_False;
    }

    return item;
}

cy_p64_cJSON *cy_p64_cJSON_CreateNumber(uint32_t num)
{
    cy_p64_cJSON *item = cy_p64_cJSON_New_Item();
    if(item)
    {
        item->type = CY_P64_cJSON_Number;
        item->valueint = num;
    }

    return item;
}

cy_p64_cJSON *cy_p64_cJSON_CreateString(const char *string)
{
    cy_p64_cJSON *item = cy_p64_cJSON_New_Item();
    if(item)
    {
        item->type = CY_P64_cJSON_String;
        item->valuestring = (char*)cy_p64_cJSON_strdup((const unsigned char*)string);
        if(!item->valuestring)
        {
            cy_p64_cJSON_Delete(item);
            return NULL;
        }
    }

    return item;
}

extern cy_p64_cJSON *cy_p64_cJSON_CreateRaw(const char *raw)
{
    cy_p64_cJSON *item = cy_p64_cJSON_New_Item();
    if(item)
    {
        item->type = CY_P64_cJSON_Raw;
        item->valuestring = (char*)cy_p64_cJSON_strdup((const unsigned char*)raw);
        if(!item->valuestring)
        {
            cy_p64_cJSON_Delete(item);
            return NULL;
        }
    }

    return item;
}

cy_p64_cJSON *cy_p64_cJSON_CreateArray(void)
{
    cy_p64_cJSON *item = cy_p64_cJSON_New_Item();
    if(item)
    {
        item->type=CY_P64_cJSON_Array;
    }

    return item;
}

cy_p64_cJSON *cy_p64_cJSON_CreateObject(void)
{
    cy_p64_cJSON *item = cy_p64_cJSON_New_Item();
    if (item)
    {
        item->type = CY_P64_cJSON_Object;
    }

    return item;
}

/* Create Arrays: */
cy_p64_cJSON *cy_p64_cJSON_CreateIntArray(const int *numbers, int count)
{
    size_t i = 0;
    cy_p64_cJSON *n = NULL;
    cy_p64_cJSON *p = NULL;
    cy_p64_cJSON *a = NULL;

    if (count < 0)
    {
        return NULL;
    }

    a = cy_p64_cJSON_CreateArray();
    for(i = 0; a && (i < (size_t)count); i++)
    {
        n = cy_p64_cJSON_CreateNumber(numbers[i]);
        if (!n)
        {
            cy_p64_cJSON_Delete(a);
            return NULL;
        }
        if(!i)
        {
            a->child = n;
        }
        else
        {
            suffix_object(p, n);
        }
        p = n;
    }

    return a;
}

cy_p64_cJSON *cy_p64_cJSON_CreateStringArray(const char **strings, int count)
{
    size_t i = 0;
    cy_p64_cJSON *n = NULL;
    cy_p64_cJSON *p = NULL;
    cy_p64_cJSON *a = NULL;

    if (count < 0)
    {
        return NULL;
    }

    a = cy_p64_cJSON_CreateArray();

    for (i = 0; a && (i < (size_t)count); i++)
    {
        n = cy_p64_cJSON_CreateString(strings[i]);
        if(!n)
        {
            cy_p64_cJSON_Delete(a);
            return NULL;
        }
        if(!i)
        {
            a->child = n;
        }
        else
        {
            suffix_object(p,n);
        }
        p = n;
    }

    return a;
}

/* Duplication */
cy_p64_cJSON *cy_p64_cJSON_Duplicate(const cy_p64_cJSON *item, cjbool recurse)
{
    cy_p64_cJSON *newitem = NULL;
    cy_p64_cJSON *child = NULL;
    cy_p64_cJSON *next = NULL;
    cy_p64_cJSON *newchild = NULL;

    /* Fail on bad ptr */
    if (!item)
    {
        goto fail;
    }
    /* Create a new item */
    newitem = cy_p64_cJSON_New_Item();
    if (!newitem)
    {
        goto fail;
    }
    /* Copy over all vars */
    newitem->type = item->type & (~CY_P64_cJSON_IsReference);
    newitem->valueint = item->valueint;
    if (item->valuestring)
    {
        newitem->valuestring = (char*)cy_p64_cJSON_strdup((unsigned char*)item->valuestring);
        if (!newitem->valuestring)
        {
            goto fail;
        }
    }
    if (item->string)
    {
        newitem->string = (item->type&CY_P64_cJSON_StringIsConst) ? item->string : (char*)cy_p64_cJSON_strdup((unsigned char*)item->string);
        if (!newitem->string)
        {
            goto fail;
        }
    }
    /* If non-recursive, we're done! */
    if (!recurse)
    {
        return newitem;
    }
    /* Walk the ->next chain for the child. */
    child = item->child;
    while (child != NULL)
    {
        newchild = cy_p64_cJSON_Duplicate(child, cj_true); /* Duplicate (with recurse) each item in the ->next chain */
        if (!newchild)
        {
            goto fail;
        }
        if (next != NULL)
        {
            /* If newitem->child is already set, crosswire ->prev and ->next and move on */
            next->next = newchild;
            newchild->prev = next;
            next = newchild;
        }
        else
        {
            /* Set newitem->child and move to it */
            newitem->child = newchild;
            next = newchild;
        }
        child = child->next;
    }

    return newitem;

fail:
    if (newitem != NULL)
    {
        cy_p64_cJSON_Delete(newitem);
    }

    return NULL;
}

void cy_p64_cJSON_Minify(char *json)
{
    unsigned char *into = (unsigned char*)json;
    while (*json)
    {
        if (*json == ' ')
        {
            json++;
        }
        else if (*json == '\t')
        {
            /* Whitespace characters. */
            json++;
        }
        else if (*json == '\r')
        {
            json++;
        }
        else if (*json=='\n')
        {
            json++;
        }
        else if ((*json == '/') && (json[1] == '/'))
        {
            /* Double-slash comments, to the end of the line. */
            while (*json && (*json != '\n'))
            {
                json++;
            }
        }
        else if ((*json == '/') && (json[1] == '*'))
        {
            /* Multiline comments. */
            while (*json && !((*json == '*') && (json[1] == '/')))
            {
                json++;
            }
            json += 2;
        }
        else if (*json == '\"')
        {
            /* String literals, which are \"-sensitive. */
            *into++ = (unsigned char)*json++;
            while (*json && (*json != '\"'))
            {
                if (*json == '\\')
                {
                    *into++ = (unsigned char)*json++;
                }
                *into++ = (unsigned char)*json++;
            }
            *into++ = (unsigned char)*json++;
        }
        else
        {
            /* All other characters. */
            *into++ = (unsigned char)*json++;
        }
    }

    /* And null-terminate. */
    *into = '\0';
}
