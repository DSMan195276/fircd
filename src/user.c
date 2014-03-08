/*
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */
#include "global.h"

#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "user.h"

void irc_user_init(struct irc_user *user)
{
    memset(user, 0, sizeof(struct irc_user));
}

void irc_user_clear(struct irc_user *user)
{
    free(user->nick);
    free(user->formatted);
}

void irc_user_cpy(struct irc_user *dest, const struct irc_user *src)
{
    memcpy(dest, src, sizeof(struct irc_user));

    if (src->nick)
        dest->nick = strdup(src->nick);
    if (src->formatted)
        dest->formatted = strdup(src->formatted);
}

void irc_user_format_nick(struct irc_user *user)
{
    free(user->formatted);
    if (user->nick)
        user->formatted = strdup(user->nick);
}

void irc_user_conv(struct irc_user *user, char *name)
{
    free(user->nick);
    if (IS_USER_CHAR(name[0])) {
        user->nick = strdup(name + 1);
    } else {
        user->nick = strdup(name);
    }
}

