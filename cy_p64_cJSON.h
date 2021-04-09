/***************************************************************************//**
* \file cy_p64_cJSON.h
* \version 1.0
*
* \brief
* This is the header file for the JSON parsing and processing.
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

/*******************************************************************************
* cJSON Prototypes
****************************************************************************//**
*
* \defgroup cjson   cJSON
*
* \brief
*  This library implements a lightweight JSON parser
*
* \{
*   \defgroup cjson_macros Macros
*   \defgroup cjson_t Data Structures
*   \defgroup cjson_api Functions
* \}
*******************************************************************************/


#ifndef CY_P64_CJSON_H
#define CY_P64_CJSON_H

#ifdef __cplusplus
extern "C"
{
#endif

/* project version */
#define CY_P64_CJSON_VERSION_MAJOR 1
#define CY_P64_CJSON_VERSION_MINOR 3
#define CY_P64_CJSON_VERSION_PATCH 2

#include <stddef.h>

/** \addtogroup cjson_macros
 * \{
 */

/** cy_p64_cJSON type: Bool with "false" value */
#define CY_P64_cJSON_False          (0x00)
/** cy_p64_cJSON type: Bool with "true" value */
#define CY_P64_cJSON_True           (0x01)
/** cy_p64_cJSON type: Null, empty object */
#define CY_P64_cJSON_NULL           (0x02)
/** cy_p64_cJSON type: Number, value in valueint */
#define CY_P64_cJSON_Number         (0x04)
/** cy_p64_cJSON type: String, value in valuestring */
#define CY_P64_cJSON_String         (0x08)
/** cy_p64_cJSON type: Array, value in child */
#define CY_P64_cJSON_Array          (0x10)
/** cy_p64_cJSON type: Object */
#define CY_P64_cJSON_Object         (0x20)
/** cy_p64_cJSON type: Raw, value in valuestring */
#define CY_P64_cJSON_Raw            (0x40)
/** cy_p64_cJSON type: Invalid object */
#define CY_P64_cJSON_Invalid        (0x80)
/** cy_p64_cJSON type: Reference object */
#define CY_P64_cJSON_IsReference    (0x100)
/** cy_p64_cJSON type: String is const */
#define CY_P64_cJSON_StringIsConst  (0x200)

/** \} */


/** \addtogroup cjson_t
 * \{
 */

/** The cy_p64_cJSON structure: */
typedef struct cy_p64_cJSON
{
    /** The next/prev allow you to walk array/object chains.
     * Alternatively, use cy_p64_cJSON_GetArraySize(), cy_p64_cJSON_GetArrayItem() or cy_p64_cJSON_GetObjectItem() */
    struct cy_p64_cJSON *next;
    /** next/prev pair */
    struct cy_p64_cJSON *prev;
    /** An array or object item will have a child pointer pointing to a chain of the items in the array/object. */
    struct cy_p64_cJSON *child;

    /** The type of the item, refer to \ref CY_P64_cJSON_False and bellow */
    int type;

    /** The item's string, if type ==\ref CY_P64_cJSON_String  and type == \ref CY_P64_cJSON_Raw */
    char *valuestring;
    /** The item's number, if type ==\ref CY_P64_cJSON_Number */
    uint32_t valueint;

    /** The item's name string, if this item is the child of, or is in the list of sub-items of an object. */
    char *string;
} cy_p64_cJSON;

/** The cy_p64_cJSON_Hooks structure: */
typedef struct cy_p64_cJSON_Hooks
{
    /** The pointer to the malloc() function */
    void *(*malloc_fn)(size_t sz);
    /** The pointer to the free() function */
    void (*free_fn)(void *ptr);
} cy_p64_cJSON_Hooks;

/** \} */


/** \addtogroup cjson_api
 * \{
 */

/*******************************************************************************
* Function Name: cy_p64_cJSON_InitHooks
****************************************************************************//**
* This function supply malloc and free functions to cy_p64_cJSON
*
* \param hooks: The pointer to the structure with alternative malloc and free functions.
*
*******************************************************************************/
extern void cy_p64_cJSON_InitHooks(cy_p64_cJSON_Hooks* hooks);


/*******************************************************************************
* Function Name: cy_p64_cJSON_Parse
****************************************************************************//**
* Supplies a block of JSON, and this returns a cy_p64_cJSON object you can
* interrogate. Call cy_p64_cJSON_Delete() when finished.
*
* \param value: The pointer to a block of JSON.
*
* \return       Parsed a cy_p64_cJSON object.
*******************************************************************************/
extern cy_p64_cJSON *cy_p64_cJSON_Parse(const char *value);


/*******************************************************************************
* Function Name: cy_p64_cJSON_Print
****************************************************************************//**
* Renders a cy_p64_cJSON entity to text for transfer/storage. Free
* the char* when finished.
*
* \param item: The pointer to the cy_p64_cJSON object
*
* \return      The pointer to a string.
*******************************************************************************/
extern char *cy_p64_cJSON_Print(const cy_p64_cJSON *item);


/*******************************************************************************
* Function Name: cy_p64_cJSON_PrintUnformatted
****************************************************************************//**
* Renders a cy_p64_cJSON entity to text for transfer/storage
* without any formatting. Free the char* when finished.
*
* \param item: The pointer to the cy_p64_cJSON object.
*
* \return      The pointer to a string.
*******************************************************************************/
extern char *cy_p64_cJSON_PrintUnformatted(const cy_p64_cJSON *item);


/*******************************************************************************
* Function Name: cy_p64_cJSON_PrintBuffered
****************************************************************************//**
* Renders a cy_p64_cJSON entity to text using a buffered strategy.
*
* \param item:      The pointer to the cy_p64_cJSON object.
* \param prebuffer: Guess at the final size. Guessing well reduces the reallocation.
* \param fmt:       0 gives unformatted, 1 gives formatted.
*
* \return           The pointer to a string.
*******************************************************************************/
extern char *cy_p64_cJSON_PrintBuffered(const cy_p64_cJSON *item, int prebuffer, int fmt);


/*******************************************************************************
* Function Name: cy_p64_cJSON_PrintPreallocated
****************************************************************************//**
* Renders a cy_p64_cJSON entity to text using a buffer already
* allocated in memory with length len. Returns 1 on success and 0 on a failure.
*
* \param item:      The pointer to the cy_p64_cJSON object.
* \param buf:       The pointer to the allocated buffer.
* \param len:       The buffer length.
* \param fmt:       0 gives unformatted, 1 gives formatted.
*
* \return           The pointer to a string.
*******************************************************************************/
extern int cy_p64_cJSON_PrintPreallocated(cy_p64_cJSON *item, char *buf, const int len, const int fmt);


/*******************************************************************************
* Function Name: cy_p64_cJSON_Delete
****************************************************************************//**
* Deletes a cy_p64_cJSON entity and all sub-entities.
*
* \param c:         The pointer to the cy_p64_cJSON object.
*
*******************************************************************************/
extern void cy_p64_cJSON_Delete(cy_p64_cJSON *c);


/*******************************************************************************
* Function Name: cy_p64_cJSON_GetArraySize
****************************************************************************//**
* Returns the number of items in an array (or object).
*
* \param array:     The pointer to the cy_p64_cJSON object
*
* \return           The number of items in an array (or object).
*******************************************************************************/
extern int cy_p64_cJSON_GetArraySize(const cy_p64_cJSON *array);


/*******************************************************************************
* Function Name: cy_p64_cJSON_GetArrayItem
****************************************************************************//**
* This function retrieves item number "item" from array "array". Returns NULL
* if fails.
*
* \param array:     The pointer to the cy_p64_cJSON object.
* \param item:      The item number.
*
* \return           The pointer to the cy_p64_cJSON object in the array or NULL if
*                   fails.
*******************************************************************************/
extern cy_p64_cJSON *cy_p64_cJSON_GetArrayItem(const cy_p64_cJSON *array, int item);


/*******************************************************************************
* Function Name: cy_p64_cJSON_GetObjectItem
****************************************************************************//**
* Gets item "string" from the object. Case-insensitive.
*
* \param object:    The pointer to the cy_p64_cJSON object.
* \param string:    The pointer to the string to find.
*
* \return           The pointer to found the cy_p64_cJSON object or NULL if fails.
*******************************************************************************/
extern cy_p64_cJSON *cy_p64_cJSON_GetObjectItem(const cy_p64_cJSON *object, const char *string);

/*******************************************************************************
* Function Name: cy_p64_cJSON_HasObjectItem
****************************************************************************//**
* Returns "true" if it possible to get an item from the object.
* Case-insensitive.
*
* \param object:    The pointer to the cy_p64_cJSON object.
* \param string:    The pointer to the string to find.
*
* \return           "true" if found or "false" if fails.
*******************************************************************************/
extern int cy_p64_cJSON_HasObjectItem(const cy_p64_cJSON *object, const char *string);


/*******************************************************************************
* Function Name: cy_p64_cJSON_GetErrorPtr
****************************************************************************//**
* Analyses failed parses. This returns a pointer to the parse
* error. Look a few chars back if you need to understand.
* Defined when cy_p64_cJSON_Parse() returns 0.
* 0 when cy_p64_cJSON_Parse() succeeds.
*
* \return The pointer to the parse error.
*******************************************************************************/
extern const char *cy_p64_cJSON_GetErrorPtr(void);


/*******************************************************************************
* Function Name: cy_p64_cJSON_CreateNull
****************************************************************************//**
* Creates a cy_p64_cJSON item of the "null" type.
*
* \return   The pointer to the cy_p64_cJSON item.
*******************************************************************************/
extern cy_p64_cJSON *cy_p64_cJSON_CreateNull(void);

/*******************************************************************************
* Function Name: cy_p64_cJSON_CreateTrue
****************************************************************************//**
* Creates a cy_p64_cJSON item of the "true" type.
*
* \return   The pointer to the cy_p64_cJSON item.
*******************************************************************************/
extern cy_p64_cJSON *cy_p64_cJSON_CreateTrue(void);

/*******************************************************************************
* Function Name: cy_p64_cJSON_CreateFalse
****************************************************************************//**
* Creates a cy_p64_cJSON item of the "false" type.
*
* \return   Pointer to cy_p64_cJSON item
*******************************************************************************/
extern cy_p64_cJSON *cy_p64_cJSON_CreateFalse(void);

/*******************************************************************************
* Function Name: cy_p64_cJSON_CreateBool
****************************************************************************//**
* Creates a cy_p64_cJSON item of the "bool" type.
*
* \param b: The value for This function ccreated cy_p64_cJSON object.
*
* \return    The pointer to This function ccy_p64_cJSON item.
*******************************************************************************/
extern cy_p64_cJSON *cy_p64_cJSON_CreateBool(int b);

/*******************************************************************************
* Function Name: cy_p64_cJSON_CreateNumber
****************************************************************************//**
* This function create a cy_p64_cJSON item of the uint type
*
* \param num:   The value for the created cy_p64_cJSON object.
*
* \return       The pointer to the cy_p64_cJSON item.
*******************************************************************************/
extern cy_p64_cJSON *cy_p64_cJSON_CreateNumber(uint32_t num);

/*******************************************************************************
* Function Name: cy_p64_cJSON_CreateString
****************************************************************************//**
* Creates a cy_p64_cJSON item of the "string" type.
*
* \param string:    The value for the created cy_p64_cJSON object.
*
* \return           The pointer to the cy_p64_cJSON item.
*******************************************************************************/
extern cy_p64_cJSON *cy_p64_cJSON_CreateString(const char *string);

/*******************************************************************************
* Function Name: cy_p64_cJSON_CreateRaw
****************************************************************************//**
* Creates a cy_p64_cJSON item of the "raw" type.
*
* \param raw:   The value for the created cy_p64_cJSON object.
*
* \return       The pointer to the cy_p64_cJSON item.
*******************************************************************************/
extern cy_p64_cJSON *cy_p64_cJSON_CreateRaw(const char *raw);

/*******************************************************************************
* Function Name: cy_p64_cJSON_CreateArray
****************************************************************************//**
* Creates a cy_p64_cJSON item of the raw "Array" type.
*
* \return       The pointer to the cy_p64_cJSON item.
*******************************************************************************/
extern cy_p64_cJSON *cy_p64_cJSON_CreateArray(void);

/*******************************************************************************
* Function Name: cy_p64_cJSON_CreateObject
****************************************************************************//**
* Creates a cy_p64_cJSON item of the "object" type.
*
* \return       The pointer to the cy_p64_cJSON item.
*******************************************************************************/
extern cy_p64_cJSON *cy_p64_cJSON_CreateObject(void);


/*******************************************************************************
* Function Name: cy_p64_cJSON_CreateIntArray
****************************************************************************//**
* Creates an array of count items.
*
* \param numbers:   The pointer to values for the created cy_p64_cJSON object.
* \param count:     The count of the values.
*
* \return       The pointer to the cy_p64_cJSON item.
*******************************************************************************/
extern cy_p64_cJSON *cy_p64_cJSON_CreateIntArray(const int *numbers, int count);

/*******************************************************************************
* Function Name: cy_p64_cJSON_CreateStringArray
****************************************************************************//**
* Creates an array of string items.
*
* \param strings:   The pointer to values for the created cy_p64_cJSON object.
* \param count:     The count of the values.
*
* \return       The pointer to the cy_p64_cJSON item.
*******************************************************************************/
extern cy_p64_cJSON *cy_p64_cJSON_CreateStringArray(const char **strings, int count);

/*******************************************************************************
* Function Name: cy_p64_cJSON_AddItemToArray
****************************************************************************//**
* Appends an item to the specified array.
*
* \param array           : The pointer to the array object.
* \param item            : The pointer to the item object.
*
*******************************************************************************/
extern void cy_p64_cJSON_AddItemToArray(cy_p64_cJSON *array, cy_p64_cJSON *item);

/*******************************************************************************
* Function Name: cy_p64_cJSON_AddItemToObject
****************************************************************************//**
* Appends an item to the specified object.
*
* \param object          : The pointer to the cy_p64_cJSON object.
* \param string          : The pointer to the string with a name of the cy_p64_cJSON object.
* \param item            : The pointer to the item object.
*
*******************************************************************************/
extern void cy_p64_cJSON_AddItemToObject(cy_p64_cJSON *object, const char *string, cy_p64_cJSON *item);

/*******************************************************************************
* Function Name: cy_p64_cJSON_AddItemToObjectCS
****************************************************************************//**
* Use this function when string is definitely const (i.e. a literal, or as good
* as), and will definitely survive the cy_p64_cJSON object.
*
* \note When using this function, make sure to always check that (item->type
* & CY_P64_cJSON_StringIsConst) is zero before writing to `item->string`
*
* \param object          : The pointer to cy_p64_cJSON object
* \param string          : The pointer to string with a name of cy_p64_cJSON object
* \param item            : The pointer to item object
*
*******************************************************************************/
extern void cy_p64_cJSON_AddItemToObjectCS(cy_p64_cJSON *object, const char *string, cy_p64_cJSON *item);

/*******************************************************************************
* Function Name: cy_p64_cJSON_AddItemReferenceToArray
****************************************************************************//**
* Appends reference to an item to the specified array. Use this
* to add an existing cy_p64_cJSON to a new cy_p64_cJSON and not
* corrupt your existing cy_p64_cJSON.
*
* \param array           : The pointer to the cy_p64_cJSON object.
* \param item            : The pointer to the item object.
*
*******************************************************************************/
extern void cy_p64_cJSON_AddItemReferenceToArray(cy_p64_cJSON *array, cy_p64_cJSON *item);

/*******************************************************************************
* Function Name: cy_p64_cJSON_AddItemReferenceToObject
****************************************************************************//**
* This function append reference to item to the specified object. Use this when
* you want to add an existing cy_p64_cJSON to a new cy_p64_cJSON, but don't want
* to corrupt your existing cy_p64_cJSON.
*
* \param object          : The pointer to cy_p64_cJSON object
* \param string          : The pointer to string with a name of cy_p64_cJSON object
* \param item            : The pointer to item object
*
*******************************************************************************/
extern void cy_p64_cJSON_AddItemReferenceToObject(cy_p64_cJSON *object, const char *string, cy_p64_cJSON *item);


/*******************************************************************************
* Function Name: cy_p64_cJSON_DetachItemFromArray
****************************************************************************//**
* Detaches items from an array.
*
* \param array           : The pointer to the cy_p64_cJSON object.
* \param which           : The number of the object to detach.
*
* \return       The pointer to the cy_p64_cJSON item.
*******************************************************************************/
extern cy_p64_cJSON *cy_p64_cJSON_DetachItemFromArray(cy_p64_cJSON *array, int which);


/*******************************************************************************
* Function Name: cy_p64_cJSON_DeleteItemFromArray
****************************************************************************//**
* Removes items from an array.
*
* \param array           : The pointer to the cy_p64_cJSON object
* \param which           : The number of the object to delete.
*
*******************************************************************************/
extern void   cy_p64_cJSON_DeleteItemFromArray(cy_p64_cJSON *array, int which);

/*******************************************************************************
* Function Name: cy_p64_cJSON_DetachItemFromObject
****************************************************************************//**
* Detaches items from an object.
*
* \param object           : The pointer to the cy_p64_cJSON object.
* \param string           : The name of the object to detach.
*
* \return       The pointer to the cy_p64_cJSON item.
*******************************************************************************/
extern cy_p64_cJSON *cy_p64_cJSON_DetachItemFromObject(cy_p64_cJSON *object, const char *string);

/*******************************************************************************
* Function Name: cy_p64_cJSON_DeleteItemFromObject
****************************************************************************//**
* Removes items from an object.
*
* \param object           : The pointer to the cy_p64_cJSON object.
* \param string           : The name of the object to delete.
*
*******************************************************************************/
extern void   cy_p64_cJSON_DeleteItemFromObject(cy_p64_cJSON *object, const char *string);


/*******************************************************************************
* Function Name: cy_p64_cJSON_InsertItemInArray
****************************************************************************//**
* Inserts items in an array. Shifts pre-existing items to the right.
*
* \param array           : The pointer to cy_p64_cJSON object
* \param which           : The number of the item.
* \param newitem         : The pointer to a new object.
*
*******************************************************************************/
/* Update array items. */
extern void cy_p64_cJSON_InsertItemInArray(cy_p64_cJSON *array, int which, cy_p64_cJSON *newitem);

/*******************************************************************************
* Function Name: cy_p64_cJSON_ReplaceItemInArray
****************************************************************************//**
* Replaces items in an array.
*
* \param array           : The pointer to the cy_p64_cJSON object.
* \param which           : The number of an item.
* \param newitem         : The pointer to a new object.
*
*******************************************************************************/
extern void cy_p64_cJSON_ReplaceItemInArray(cy_p64_cJSON *array, int which, cy_p64_cJSON *newitem);

/*******************************************************************************
* Function Name: cy_p64_cJSON_ReplaceItemInObject
****************************************************************************//**
* Replaces items in an object.
*
* \param object          : The pointer to the cy_p64_cJSON object.
* \param string          : The name of an item.
* \param newitem         : The pointer to a new object.
*
*******************************************************************************/
extern void cy_p64_cJSON_ReplaceItemInObject(cy_p64_cJSON *object,const char *string, cy_p64_cJSON *newitem);


/*******************************************************************************
* Function Name: cy_p64_cJSON_Duplicate
****************************************************************************//**
* Duplicates a cy_p64_cJSON item.
*
* \note This function creates a new, identical to the one you
* pass cy_p64_cJSON item , in new memory to be released. With recurse!=0, it will
* duplicate any children connected to the item. The item->next and item->prev
* pointers are always zero on return from this function.
*
* \param item            : Pointer to cy_p64_cJSON object
* \param recurse         : 0-duplicate without children object, other- with children
*
* \return The pointer to the duplicated object
*******************************************************************************/
extern cy_p64_cJSON *cy_p64_cJSON_Duplicate(const cy_p64_cJSON *item, int recurse);

/*******************************************************************************
* Function Name: cy_p64_cJSON_ParseWithOpts
****************************************************************************//**
* Allows you to require (and check) that the JSON is null-
* terminated, and to retrieve the pointer to the final byte parsed.
*
* \note If you supply a ptr in return_parse_end and parsing fails, then
* return_parse_end will contain a pointer to the error. If not, then
* cy_p64_cJSON_GetErrorPtr() does the job.
*
* \param value                   : The pointer to the value for parsing.
* \param return_parse_end        : The pointer to an output error or NUL.L
* \param require_null_terminated : "true" to require (and check) that the JSON is null-
* terminated, "false" otherwise.
*
* \return The pointer to the parsed cy_p64_cJSON object.
*******************************************************************************/
extern cy_p64_cJSON *cy_p64_cJSON_ParseWithOpts(const char *value, const char **return_parse_end, int require_null_terminated);

/*******************************************************************************
* Function Name: cy_p64_cJSON_Minify
****************************************************************************//**
* Removes not required characters from a string like
* whitespace characters.
*
* \param json            : The pointer to a JSON string.
*
*******************************************************************************/
extern void cy_p64_cJSON_Minify(char *json);

/** \} */


/* Macros for creating things quickly. */
#define cy_p64_cJSON_AddNullToObject(object,name) cy_p64_cJSON_AddItemToObject(object, name, cy_p64_cJSON_CreateNull())
#define cy_p64_cJSON_AddTrueToObject(object,name) cy_p64_cJSON_AddItemToObject(object, name, cy_p64_cJSON_CreateTrue())
#define cy_p64_cJSON_AddFalseToObject(object,name) cy_p64_cJSON_AddItemToObject(object, name, cy_p64_cJSON_CreateFalse())
#define cy_p64_cJSON_AddBoolToObject(object,name,b) cy_p64_cJSON_AddItemToObject(object, name, cy_p64_cJSON_CreateBool(b))
#define cy_p64_cJSON_AddNumberToObject(object,name,n) cy_p64_cJSON_AddItemToObject(object, name, cy_p64_cJSON_CreateNumber(n))
#define cy_p64_cJSON_AddStringToObject(object,name,s) cy_p64_cJSON_AddItemToObject(object, name, cy_p64_cJSON_CreateString(s))
#define cy_p64_cJSON_AddRawToObject(object,name,s) cy_p64_cJSON_AddItemToObject(object, name, cy_p64_cJSON_CreateRaw(s))

#define cy_p64_cJSON_SetIntValue(object, number) ((object) ? (object)->valueint = (number) : (number))
#define cy_p64_cJSON_SetNumberValue(object, number) ((object) ? (object)->valueint = (number) : (number))

/* Macro for iterating over an array */
#define CY_P64_cJSON_ArrayForEach(pos, head) for(pos = (head)->child; pos != NULL; pos = pos->next)

#ifdef __cplusplus
}
#endif

#endif /* CY_P64_CJSON_H */
