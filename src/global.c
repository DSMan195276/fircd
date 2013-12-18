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

#include "debug.h"
#include "fircd.h"
#include "buf.h"

struct network_cons *current_state = NULL;

static const char default_directory[]   = "/tmp/irc";

static const struct config default_config = {
    .stay_in_forground = 0,
    .no_config = 0
};


static char *valloc_sprintf (const char *format, va_list lst)
{
    size_t size = vsnprintf(NULL, 0, format, lst) + 1;
    char *buf = malloc(size);

    if (!buf)
        return NULL;

    buf[0] = '\0';

    vsnprintf(buf, size, format, lst);
    return buf;
}

char *alloc_sprintf (const char *format, ...)
{
    char *ret;
    va_list lst;

    va_start(lst, format);
    ret = valloc_sprintf(format, lst);
    va_end(lst);

    return ret;
}

static void network_cons_clear(struct network_cons *con)
{
    network_clear_all(con->head);
    config_clear(&(con->conf));

    buf_free(&con->cmdfd);
    
    unlink("cmd");

    free(con->config_file);
    free(con->dir);
}

void current_state_init(void)
{
    current_state = malloc(sizeof(struct network_cons));
    memset(current_state, 0, sizeof(struct network_cons));

    buf_init(&current_state->cmdfd);
    current_state->conf = default_config;
    current_state->dir = strdup(default_directory);

}

void current_state_clear(void)
{

    network_cons_clear(current_state);
    free(current_state);
}

