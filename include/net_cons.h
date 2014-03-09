/*
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */
#ifndef INCLUDE_NET_CONS_H
#define INCLUDE_NET_CONS_H

#include "global.h"

#include <sys/select.h>

#include "buf.h"
#include "config.h"

struct network;

struct network_cons {
    struct network *head;

    char *config_file;
    struct config conf;

    struct buf_fd cmdfd;

    unsigned short stay_in_forground :1;
    unsigned short no_config :1;
    unsigned short dont_auto_load :1;
};

#define FDADD_FD_TO_CON(con, fd) \
    do { \
        if ((fd) != -1) \
            FD_SET((fd), &con->infd); \
        if ((fd) > (con->maxfd)) \
            (con->maxfd) = (fd); \
    } while (0)

#define FDCLR_FD_FROM_CON(con, fd) \
    do { \
        FD_CLR((fd), &con->infd); \
    } while (0)

extern void network_cons_init  (struct network_cons *);
extern void network_cons_clear (struct network_cons *);

extern int  network_cons_config_read (struct network_cons *);

extern void network_cons_init_directory (struct network_cons *);
extern void network_cons_connect_networks (struct network_cons *);
extern void network_cons_set_select_desc (struct network_cons *, fd_set *, fd_set *, int *);
extern void network_cons_handle_file_check (struct network_cons *, fd_set *, fd_set *);

extern void network_cons_auto_login (struct network_cons *);

#endif
