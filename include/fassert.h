/*
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */
#ifndef INCLUDE_FASSERT_H
#define INCLUDE_FASSERT_H

#ifdef FIRCD_DEBUG
# include <stdio.h>
# define fassert(cond) \
    do { \
        if (!(cond)) { \
            fprintf(stderr, "Assertion failed: %s:%s:%d: %s\n", \
                    __FILE__, __func__, __LINE__, #cond); \
            *((char *)NULL) = 0; /* Segfault program since we failed an \
                                   assertion. This allowed for easy stack \
                                   traces and etc. to be gathered */ \
        } \
    } while (0)

#else
# define fassert(cond) do { ; } while (0)
#endif

#endif
