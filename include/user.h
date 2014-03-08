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
    char *nick;
    char *formatted;

    struct irc_user_flags flags;
};

#define IS_USER_CHAR(ch) ((ch) == '@' || (ch) == '!' || (ch) == '&')

extern void irc_user_init(struct irc_user *);
extern void irc_user_clear(struct irc_user *);

extern void irc_user_cpy(struct irc_user *dest, const struct irc_user *src);

extern void irc_user_format_nick(struct irc_user *);
extern void irc_user_conv(struct irc_user *, char *name);

#endif
