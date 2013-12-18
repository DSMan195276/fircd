/*
 * ./debug.c -- provides functions for starting and ending debugging. These are
 *              only compiled when debugging is turned-on.
 *
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#include "global.h"

#include <stdio.h>

#include "debug.h"

#ifdef FIRCD_DEBUG

FILE *debug_file = NULL;

void debug_init(void)
{
    debug_file = fopen(DEBUG_FILE, "w");
}

void debug_close(void)
{
    fclose(debug_file);
}

#endif

