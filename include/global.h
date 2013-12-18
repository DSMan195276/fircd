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

#define QQ(x) #x
#define Q(x) QQ(x)

#include <sys/select.h>

#include "network.h"
#include "channel.h"
#include "config.h"

struct network_cons {
    struct network *head;

    char *config_file;
    struct config conf;

    char *dir;
    struct buf_fd cmdfd;

    int maxfd;
    fd_set infd, outfd;
};

extern struct network_cons *current_state;

extern void current_state_init  (void);
extern void current_state_clear (void);
//extern void network_cons_clear(struct network_cons *con);

extern char *alloc_sprintf(const char *format, ...);

#endif
