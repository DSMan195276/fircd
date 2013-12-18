/*
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */
#ifndef INCLUDE_CONFIG_H
#define INCLUDE_CONFIG_H

#include "global.h"

struct config {
    unsigned int stay_in_forground :1;
    unsigned int no_config :1;
};

extern int config_file_parse(void);

extern void config_read (struct config *conf, const char *filename);
extern void config_write (struct config *conf, const char *filename);
extern void config_clear (struct config *conf);

#endif
