/*
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */
#ifndef INCLUDE_USER_H
#define INCLUDE_USER_H

#include "rbtree.h"

struct irc_user_flags {
    unsigned int is_op    :1;
    unsigned int is_voice :1;
};

struct irc_user {
    struct rbnode node;

    char *nick;
    char *formatted;

    struct irc_user_flags flags;
};

void irc_user_init(struct irc_user *);
void irc_user_clear(struct irc_user *);

void irc_user_format_nick(struct irc_user *);

struct irc_user *irc_user_new (void);
void             irc_user_free(struct irc_user*);


#endif
