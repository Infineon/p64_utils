/***************************************************************************//**
* \file cy_p64_malloc.c
* \version 1.0
*
* \brief
* This is the source code for the memory allocation functions.
*
********************************************************************************
* \copyright
* Copyright 2019-2021, Cypress Semiconductor Corporation (an Infineon company).
* All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

/*******************************************************************************
* Malloc Prototypes
****************************************************************************//**
*
* \defgroup malloc     Malloc
*
* \brief
*  This library implement memory allocation functions
*
* \{
*   \defgroup malloc_api Functions
*   \defgroup malloc_macros Macros
* \}
*******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "cy_p64_malloc.h"


/******************************************************
 *                      Macros
 ******************************************************/
#define CY_P64_META_DATA_SIZE             (16u)                /* It doesn't count the dummy pointer to the data block */
#define CY_P64_MIN_BLOCK_SIZE             (4u)


/******************************************************
 *                 Type Definitions
 ******************************************************/

/* The memory block is organized with meta-data in front of the allocated
block. Each chunk of data is composed of a block of meta-data followed by the block of data.
The pointer returned by cy_p64_malloc points on the data block, not on the complete chunk. */
struct cy_p64_meta_data_t
{
    uint32_t size;
    struct cy_p64_meta_data_t *next;
    struct cy_p64_meta_data_t *prev;
    void *free_ptr;             /* A pointer to the allocated block for meta data validation, the NULL block is free */
    uint32_t data[1];           /* Beginning of the the allocated block */
};

/**
 * Holds information needed to create a memory pool
 * addr: The address of the start of a pool.
 * size:  Size in bytes of the pool
 */
typedef struct
{
   void *addr;
   uint32_t size;
   void *base;
   void *shm_break;
} cy_p64_pool_mem_info_t;

typedef struct cy_p64_meta_data_t *cy_p64_meta_data_ptr_t;


/******************************************************
 *                 Global variables
 ******************************************************/
static uint32_t cy_p64_heap_buffer[CY_P64_HEAP_DATA_SIZE / sizeof(uint32_t)];
static cy_p64_pool_mem_info_t cy_p64_heap_pool =
{
    .addr = cy_p64_heap_buffer,
    .size = CY_P64_HEAP_DATA_SIZE,
    .base = NULL,
    .shm_break = cy_p64_heap_buffer
};


/*******************************************************************************
* Function Name: cy_p64_find_block
****************************************************************************//**
*
*  Finds a free sufficiently wide block of the memory. It begins at the
*  base address of the memory, tests the current block, if it fits the
*  needs, the function just returns its address, otherwise continues to the next
*  block until finds a fitting one or the end of the heap.
*
*  \param last: The last visited block is set to allow the cy_p64_malloc function
*  to easily extend the end of the shared memory if function found no fitting block.
*  \param size: The required size of the memory.
*
*  \return
*   A pointer of the cy_p64_meta_data_ptr_t type to the next sufficient free memory
*   block, or NULL if none were found.
*   After the execution, the argument last points to the last visited block.
*
*******************************************************************************/
static cy_p64_meta_data_ptr_t cy_p64_find_block(cy_p64_meta_data_ptr_t *last, uint32_t size)
{
    cy_p64_meta_data_ptr_t b = cy_p64_heap_pool.base;

    if(b != NULL)
    {
        while (!((b->free_ptr == NULL) && (b->size >= size)))
        {
            *last = b;
            b = b->next;
            if(b == NULL)
            {
                break;
            }
        }
    }
    return (b);
}


/*******************************************************************************
* Function Name: cy_p64_sbrk
****************************************************************************//**
*
*  This function increments the break pointer of the memory.
*
*  \param size: The size for which to change the break pointer. Can be positive, negative or null.
*
*  \return
*   The pointer to the memory break, or NULL on failure.
*
*******************************************************************************/
static void *cy_p64_sbrk(uint32_t size)
{
    void *new_break;

    new_break = (uint8_t *)cy_p64_heap_pool.shm_break + size;

    if((new_break >= cy_p64_heap_pool.addr) &&
       (new_break <= (void *)((uint8_t *)cy_p64_heap_pool.addr + cy_p64_heap_pool.size)))
    {
        cy_p64_heap_pool.shm_break = new_break;
    }
    else
    {
        new_break = NULL;
    }

    return (new_break);
}


/*******************************************************************************
* Function Name: cy_p64_extend_heap
****************************************************************************//**
*
*  Extends the memory block by a new block if there is
*  sufficient free space.
*
*  \param last: The last visited block.
*  \param size: The required size of the memory.
*
*  \return
*   The pointer of the cy_p64_meta_data_ptr_t type to the new block of memory,
*   or NULL if not enough space.
*
*******************************************************************************/
static cy_p64_meta_data_ptr_t cy_p64_extend_heap(cy_p64_meta_data_ptr_t last, uint32_t size)
{
    cy_p64_meta_data_ptr_t b;

    b = cy_p64_sbrk(0u);
    if(b != NULL)
    {
        if(cy_p64_sbrk(CY_P64_META_DATA_SIZE + size) != NULL)
        {
            b->size = size;
            b->next = NULL;
            b->prev = last;
            b->free_ptr = b->data;    /* Meta data is allocated */
            if(last != NULL)
            {
                last->next = b;
            }
        }
        else /* If cy_p64_sbrk fails, we return NULL */
        {
            b = NULL;
        }
    }
    return (b);
}


/*******************************************************************************
* Function Name: cy_p64_split_block
****************************************************************************//**
*
*  Splits the block and adds a new block to the list when existing
*  the block is wide enough to held the asked size plus a new block
*  (at least CY_P64_META_DATA_SIZE + 4), it inserts a new block in the list.
*
*  \param b: The pointer to the block to split.
*  \param size: The new size of the block.
*
*******************************************************************************/
static void cy_p64_split_block(cy_p64_meta_data_ptr_t b, uint32_t size)
{
    cy_p64_meta_data_ptr_t new;

    new = (cy_p64_meta_data_ptr_t)((uint8_t *)b->data + size);
    new->size = b->size - size - CY_P64_META_DATA_SIZE;
    new->next = b->next;
    new->prev = b;
    new->free_ptr = NULL;     /* Meta data is free */
    b->size = size;
    b->next = new;
    if(new->next != NULL)
    {
        new->next->prev = new;
    }
}


/*******************************************************************************
* Function Name: cy_p64_fusion
****************************************************************************//**
*
*  Merges two free blocks into one bigger block to eliminate extra
*  fragmentation of the free memory.
*
*  \param b: The pointer to the block to fuse with the next one.
*
*  \return
*   The pointer of the cy_p64_meta_data_ptr_t type to the new block of memory.
*
*******************************************************************************/
static cy_p64_meta_data_ptr_t cy_p64_fusion(cy_p64_meta_data_ptr_t b)
{
    if(b->next != NULL)
    {
        if(b->next->free_ptr == NULL) /* Check if the next block is free */
        {
            /* If the next block is free, sum the sizes of the current block
            *  and the next one, plus the meta-data size.
            */
            b->size += CY_P64_META_DATA_SIZE + b->next->size;
            b->next = b->next->next;
            if (b->next != NULL)
            {
                b->next->prev = b;
            }
        }
    }
    return (b);
}


/*******************************************************************************
* Function Name: cy_p64_get_block
****************************************************************************//**
*
*  This function gets the block address from the allocated memory address.
*
*  \param *p: The memory address.
*
*  \return
*   The cy_p64_meta_data_ptr_t pointer to the block of the allocated memory.
*
*******************************************************************************/
static cy_p64_meta_data_ptr_t cy_p64_get_block(void *p)
{
    return (cy_p64_meta_data_ptr_t)((uint8_t *)p - CY_P64_META_DATA_SIZE);
}


/*******************************************************************************
* Function Name: cy_p64_is_addr_valid
****************************************************************************//**
*
*  Validates the address for free.
*  First, it checks if the pointer is within the memory buffer in the pool, then it
*  verifies that the pointer inside the associated block is pointing to the same data.
*
*  \param *p: The memory address to validate.
*
*  \return
*   true: If the address is in the allocated buffer range.
*   false: The wrong address.
*
*******************************************************************************/
static bool cy_p64_is_addr_valid(void *p)
{
    bool res = false;
    if ((cy_p64_heap_pool.base != NULL) && (p != NULL))
    {
        if(p > cy_p64_heap_pool.base)
        {
            if(p < cy_p64_sbrk(0u))
            {
                res = (p == (cy_p64_get_block(p))->free_ptr);
            }
        }
    }
    return (res);
}


static bool cy_p64_is_u32_multiplication_safe(uint32_t x, uint32_t y)
{
    bool safe = false;
    uint64_t temp = (uint64_t)x * (uint64_t)y;

    if(temp <= (uint64_t)UINT32_MAX)
    {
        safe = true;
    }

    return safe;
}


/*******************************************************************************
* Function Prototypes
****************************************************************************//**
*
*  \addtogroup malloc_api
*
*  \{
*******************************************************************************/

/*******************************************************************************
* Function Name: cy_p64_malloc
****************************************************************************//**
*
*  Allocates the memory from the memory buffer configured statically.
*  \ref CY_P64_HEAP_DATA_SIZE defines the size of default memory buffer.
*
*  \param size: The required size of the memory.
*
*  \return
*   The void pointer to the allocated memory buffer, or NULL if there is no enough space.
*
*******************************************************************************/
void *cy_p64_malloc(uint32_t size)
{
    void *res = NULL;
    uint32_t s;

    /* Align the requested size */
    s = CY_P64_ALIGN_TO_4(size);

    if(s < cy_p64_heap_pool.size)
    {
        cy_p64_meta_data_ptr_t b, last;

        if(cy_p64_heap_pool.base != NULL)
        {
            /* First find a block */
            last = cy_p64_heap_pool.base;
            b = cy_p64_find_block(&last, s);
            if(b != NULL)
            {
                /* Can we split the block? */
                if((b->size - s) >= (CY_P64_META_DATA_SIZE + CY_P64_MIN_BLOCK_SIZE))
                {
                    cy_p64_split_block(b, s);
                }
                /* Mark the block as used */
                b->free_ptr = b->data;
            }
            else    /* There are no fitting block */
            {
                b = cy_p64_extend_heap(last, s);
            }
        }
        else    /* First time */
        {
            b = cy_p64_extend_heap(NULL, s);
            if(b != NULL)
            {
                cy_p64_heap_pool.base = b;
            }
        }

        if(b != NULL)
        {
            res = b->data;
        }
    }

    return (res);
}


/*******************************************************************************
* Function Name: cy_p64_Calloc
****************************************************************************//**
*
*  Allocates the memory from the memory buffer configured statically.
*
*  \param nelem: The number of elements.
*  \param elsize: The required size of the element.
*
*  \return
*   The void pointer to the allocated memory buffer, or NULL if there is no enough space.
*
*******************************************************************************/
void *cy_p64_calloc(uint32_t nelem, uint32_t elsize)
{
    void *res = NULL;
    uint32_t size_in_bytes;

    if(cy_p64_is_u32_multiplication_safe(nelem, elsize))
    {
        size_in_bytes = nelem * elsize;

        if(size_in_bytes != 0u)
        {
            res = cy_p64_malloc(size_in_bytes);

            if(res != NULL)
            {
                (void)memset(res, 0, size_in_bytes);
            }
        }
    }

    return (res);
}


/*******************************************************************************
* Function Name: cy_p64_free
****************************************************************************//**
*
*  Frees the allocated memory.
*  When CY_P64_FREE_WIPED is defined, it also wipes(set to 0) data from memory.
T
*  \param *p: The pointer to the memory.
*
*******************************************************************************/
void cy_p64_free(void *p)
{
    cy_p64_meta_data_ptr_t b;

    /* Verify the pointer */
    if (cy_p64_is_addr_valid(p))
    {
        /* Get the corresponding block */
        b = cy_p64_get_block(p);
    #ifdef CY_P64_FREE_WIPED
        /* Optionally delete data */
        (void)memset(p, 0, b->size);
    #endif /* CY_P64_FREE_WIPED */
        /* Mark the block as free */
        b->free_ptr = NULL;
        /* If the previous block exists and it is free, step backward in the block list and fusion the two blocks. */
        if((b->prev != NULL) && (b->prev->free_ptr == NULL))
        {
            b = cy_p64_fusion(b->prev);
        }
        /* Also fusion with the next block */
        if(b->next != NULL)
        {
            (void)cy_p64_fusion(b);
        }
        else /* If it is the last block - release memory */
        {
            if(b->prev != NULL)
            {
                b->prev->next = NULL;
            }
            else /* If there are no more blocks, set base to NULL */
            {
                cy_p64_heap_pool.base = NULL;
            }
            cy_p64_heap_pool.shm_break = b;
        }
    }

/** \} */
}


/* [] END OF FILE */
