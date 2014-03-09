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
#include "array.h"
#include "net_cons.h"

struct network;

struct network_config {
    unsigned int remove_files_on_close :2;
};

struct config {
    ARRAY(char*, auto_login);
    struct network *first;

    char *config_file;
    char *root_directory;

    struct network_config net_global_conf;

    unsigned int arg_stay_in_forground :1;
    unsigned int arg_dont_auto_load :1;
    unsigned int arg_no_config :1;

    unsigned int stay_in_forground :1;
};

extern struct config prog_config;

extern void config_init(void);

extern int config_read(void);
extern void config_clear(void);

extern void config_add_auto_login(const char *login);

#endif
