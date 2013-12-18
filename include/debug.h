/*
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */
#ifndef INCLUDE_DEBUG_H
#define INCLUDE_DEBUG_H

#include "global.h"

#define DEBUG_FILE "/tmp/fircd.log"

#ifdef FIRCD_DEBUG

# include <stdio.h>
  extern FILE *debug_file;
  extern void debug_init(void);
  extern void debug_close(void);
# define DEBUG_PRINT(...) \
    do { \
        fprintf(debug_file, "%s: %d: ", __FILE__, __LINE__); \
        fprintf(debug_file, __VA_ARGS__); \
        fputc('\n', debug_file); \
        fflush(debug_file); \
    } while (0)
# define DEBUG_INIT() debug_init()
# define DEBUG_CLOSE() debug_close()

#else

# define DEBUG_PRINT(...) do { ; } while (0)
# define DEBUG_INIT() do { ; } while (0)
# define DEBUG_CLOSE() do { ; } while (0)
#endif


#endif
