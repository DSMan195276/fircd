/*
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */
#ifndef INCLUDE_NETWORK_H
#define INCLUDE_NETWORK_H

#include "global.h"

#include <sys/types.h>
#include <sys/select.h>

#include "array.h"
#include "channel.h"
#include "config.h"

#define DEFAULT_PORT 6667

enum network_login {
    LOGIN_NONE,
    LOGIN_NICKSERV,
    LOGIN_SASL
};

struct network_cons;

struct network {
    struct network_cons *con;
    struct network *next;

    struct channel *head;

    enum network_login login_type;

    char *name;
    char *url;
    int   portno;
    struct buf_fd sock;

    char *realname;
    char *nickname, *password;

    ARRAY(char*, joined);
    struct buf_fd cmdfd;
    int joinedfd, motdfd, rawfd, realnamefd, nicknamefd;

    struct network_config conf;
    unsigned int close_network :1;
};

extern void network_init             (struct network *, struct network_cons *);
extern void network_init_select_desc (struct network *, fd_set *infd, fd_set *outfd, int *maxfd);
extern void network_setup_files      (struct network *);
extern void network_handle_input     (struct network *, fd_set *, fd_set *);
extern void network_connect          (struct network *);
extern struct network *network_copy  (struct network *);

extern struct channel *network_add_channel (struct network *, const char *channel);
extern struct channel *network_find_channel (struct network *, const char *channel);

extern void network_quit      (struct network *);
extern void network_clear     (struct network *);
extern void network_clear_all (struct network *);

extern void network_write_raw        (struct network *, const char *text);
extern void network_write_nick       (struct network *);
extern void network_write_realname   (struct network *);
extern void network_write_motd_start (struct network *);
extern void network_write_motd_line  (struct network *, const char *motd);
extern void network_write_joined     (struct network *);


#endif
