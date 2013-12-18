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
    unsigned int name##_size, name##_alloc; \
    type *name

#define ARRAY_SIZE(str, name) ((str).name##_size)

#define ARRAY_RESIZE(str, name, size) \
    do { \
        unsigned int size_tmp = (size), total_size = 0; \
        void *start = (str).name; \
        const unsigned int block_size = 100; \
        if ((str).name##_alloc <= (size_tmp)) { \
            do { \
                total_size += block_size; \
                (str).name##_alloc += block_size; \
            } while ((str).name##_alloc <= (size_tmp)); \
            (str).name = realloc((str).name, \
                    (sizeof((str).name) / sizeof((str).name[0])) * (str).name##_alloc); \
            start = (str).name + (str).name##_size * (sizeof(str).name / sizeof(str).name[0]); \
            memset(start, 0, total_size * (sizeof(str).name / sizeof(str).name[0])); \
        } \
        (str).name##_size = (size_tmp); \
    } while (0)

#define ARRAY_FOREACH(str, name, index) for (index = 0; index < (str).name##_size; index++)

#define ARRAY_FREE(str, name) \
    do { \
        free((str).name); \
        (str).name = NULL; \
        (str).name##_size = 0; \
        (str).name##_alloc = 0; \
    } while (0)

#endif
