/*
 * Copyright Notice for
 * Proactive Data Containers (PDC) Software Library and Utilities
 * -----------------------------------------------------------------------------

 *** Copyright Notice ***

 * Proactive Data Containers (PDC) Copyright (c) 2017, The Regents of the
 * University of California, through Lawrence Berkeley National Laboratory,
 * UChicago Argonne, LLC, operator of Argonne National Laboratory, and The HDF
 * Group (subject to receipt of any required approvals from the U.S. Dept. of
 * Energy).  All rights reserved.

 * If you have questions about your rights to use or distribute this software,
 * please contact Berkeley Lab's Innovation & Partnerships Office at  IPO@lbl.gov.

 * NOTICE.  This Software was developed under funding from the U.S. Department of
 * Energy and the U.S. Government consequently retains certain rights. As such, the
 * U.S. Government has been granted for itself and others acting on its behalf a
 * paid-up, nonexclusive, irrevocable, worldwide license in the Software to
 * reproduce, distribute copies to the public, prepare derivative works, and
 * perform publicly and display publicly, and to permit other to do so.
 */

#include <stdlib.h>
#include <assert.h>
#include "pdc_malloc.h"
#include "pdc_private.h"

#ifdef HAVE_MALLOC_USABLE_SIZE
#include <malloc.h>
#else
// alternative header files for malloc_usable_size or similar functions.
#endif

size_t PDC_mem_usage_g;

void *
PDC_malloc(size_t size)
{
    void *ret_value;

    FUNC_ENTER(NULL);

    assert(size);

    if (size)
        ret_value = malloc(size);
    else
        ret_value = NULL;

    if (ret_value)
        PDC_mem_usage_g += size;

    FUNC_LEAVE(ret_value);
}

void *
PDC_malloc_addsize(size_t size, size_t *mem_usage_ptr)
{
    void *ret_value = PDC_malloc(size);
    if (ret_value && mem_usage_ptr)
        *mem_usage_ptr += size;
    return ret_value;
}

void *
PDC_calloc(size_t count, size_t size)
{
    void *ret_value;

    FUNC_ENTER(NULL);

    assert(count);
    assert(size);

    if (count && size)
        ret_value = calloc(count, size);
    else
        ret_value = NULL;

    if (ret_value)
        PDC_mem_usage_g += size;

    FUNC_LEAVE(ret_value);
}

void *
PDC_calloc_addsize(size_t count, size_t size, size_t *mem_usage_ptr)
{
    void *ret_value = PDC_calloc(count, size);
    if (ret_value && mem_usage_ptr)
        *mem_usage_ptr += size;
    return ret_value;
}

void *
PDC_realloc_knowing_oldsize(void *ptr, size_t size, size_t old_size)
{
    void *ret_value;

    FUNC_ENTER(NULL);

    assert(size);
    size_t _old_size = old_size;
    if (size)
        ret_value = realloc(ptr, size);
    else
        ret_value = NULL;

    if (ret_value) {
        PDC_mem_usage_g += size;
        if (_old_size) {
            PDC_mem_usage_g -= _old_size;
        }
    }

    FUNC_LEAVE(ret_value);
}

void *
PDC_realloc(void *ptr, size_t size)
{
    size_t _old_size = 0;
#ifdef HAVE_MALLOC_USABLE_SIZE
    _old_size = malloc_usable_size(ptr);
#endif
    return PDC_realloc_knowing_oldsize(ptr, size, _old_size);
}

void *
PDC_realloc_addsize_knowing_oldsize(void *ptr, size_t size, size_t old_size, size_t *mem_usage_ptr)
{
    size_t _old_size = old_size;
    void * ret_value = PDC_realloc_knowing_oldsize(ptr, size, _old_size);
    if (ret_value && mem_usage_ptr) {
        *mem_usage_ptr += size;
        if (_old_size) {
            *mem_usage_ptr -= _old_size;
        }
    }
    return ret_value;
}

void *
PDC_realloc_addsize(void *ptr, size_t size, size_t *mem_usage_ptr)
{
    size_t _old_size = 0;
#ifdef HAVE_MALLOC_USABLE_SIZE
    _old_size = malloc_usable_size(ptr);
#endif
    return PDC_realloc_addsize_knowing_oldsize(ptr, size, _old_size, mem_usage_ptr);
}

void *
PDC_free_knowing_old_size(void *mem, size_t old_size)
{
    size_t _old_size = old_size;

    void *ret_value = NULL;

    FUNC_ENTER(NULL);

    if (mem) {
        free(mem);
        if (_old_size) {
            PDC_mem_usage_g -= _old_size;
        }
    }

    FUNC_LEAVE(ret_value);
}

void *
PDC_free(void *mem)
{
    size_t _old_size = 0;
#ifdef HAVE_MALLOC_USABLE_SIZE
    _old_size = malloc_usable_size(mem);
#endif
    return PDC_free_knowing_old_size(mem, _old_size);
}

void
PDC_free_void(void *mem)
{
    PDC_free(mem);
}

size_t
PDC_get_global_mem_usage()
{
    return PDC_mem_usage_g;
}