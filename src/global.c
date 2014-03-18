/*
 * ./global.c -- Defines program-global constructs
 *
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#include "global.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>

#include "debug.h"
#include "config.h"
#include "buf.h"

int alloc_sprintfv (char **buf, const char *format, va_list lst)
{
    size_t size = vsnprintf(NULL, 0, format, lst) + 1;
    
    *buf = malloc(size);

    if (!*buf)
        return -1;

    (*buf)[0] = '\0';

    size = vsnprintf(*buf, size, format, lst);
    return size;
}

int alloc_sprintf (char **buf, const char *format, ...)
{
    size_t ret;
    va_list lst;

    va_start(lst, format);
    ret = alloc_sprintfv(buf, format, lst);
    va_end(lst);

    return ret;
}

void fdprintfv (const int fd, const char *format, va_list lst)
{
    char *buf = NULL;
    int size = alloc_sprintfv(&buf, format, lst);
    if (size == -1)
        return ;

    write(fd, buf, size);
}

void fdprintf (const int fd, const char *format, ...)
{
    va_list lst;

    va_start(lst, format);
    fdprintfv(fd, format, lst);
    va_end(lst);
}

