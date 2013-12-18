/*
 * ./config.c -- Handles reading and writing the fircd configuration file
 *
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation. 
 */

#include "global.h"

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "debug.h"
#include "config.h"

static const char default_config_file[] = "~/.fircrc";

/* Returns 0 if file could be opened and parsed, or no file was specified and
 * the default file didn't exist. Returns 1 if the file was specified and it
 * didn't exist. */
int config_file_parse(void)
{
    struct stat st;
    int no_conf = 0;
    const char *filename;

    if (current_state->config_file == NULL) {
        no_conf = 1;
        filename = default_config_file;
    } else {
        filename = current_state->config_file;
    }

    if (stat(filename, &st) == 0)
        return !no_conf;

    config_read(&(current_state->conf), filename);
    return 0;
}

void config_read(struct config *conf, const char *filename)
{

}

void config_write(struct config *conf, const char *filename)
{

}

void config_clear(struct config *conf)
{

}

