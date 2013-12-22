/*
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation;
 */
#ifndef INCLUDE_ARRAY_H
#define INCLUDE_ARRAY_H

#include "global.h"

#include <stdlib.h> /* realloc(), free() */
#include <string.h> /* memset() */

#define ARRAY(type, name) \
    struct { \
        unsigned int _size, _alloc; \
        type *arr; \
    } name


#define ARRAY_SIZE(array) ((array)._size)

#define ARRAY_RESIZE(array, size) \
    do { \
        unsigned int size_tmp = (size), total_size = 0; \
        void *start = (array).arr; \
        const unsigned int block_size = 100; \
        if ((array)._alloc <= (size_tmp)) { \
            do { \
                total_size += block_size; \
                (array)._alloc += block_size; \
            } while ((array)._alloc <= (size_tmp)); \
            (array).arr = realloc((array).arr, \
                    (sizeof((array).arr) / sizeof((array).arr[0])) * (array)._alloc); \
            start = (array).arr + (array)._size * (sizeof(array).arr / sizeof(array).arr[0]); \
            memset(start, 0, total_size * (sizeof(array).arr / sizeof(array).arr[0])); \
        } \
        (array)._size = (size_tmp); \
    } while (0)

#define ARRAY_FOREACH(array, index) for (index = 0; index < (array)._size; index++)

#define ARRAY_FREE(array) \
    do { \
        free((array).arr); \
        (array).arr = NULL; \
        (array)._size = 0; \
        (array)._alloc = 0; \
    } while (0)

#endif
