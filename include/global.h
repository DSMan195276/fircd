/*
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */
#ifndef INCLUDE_GLOBAL_H
#define INCLUDE_GLOBAL_H

#define _XOPEN_SOURCE 600

#include <stddef.h>

/* Inspired via the Linux-kernel macro 'container_of' */
#define container_of(ptr, type, member) \
    ((type *) ((char*)(ptr) - offsetof(type, member)))

#define QQ(x) #x
#define Q(x) QQ(x)

extern char *alloc_sprintf(const char *format, ...);

#endif
