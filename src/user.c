/*
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */
#include "global.h"

#include <stdlib.h>

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

void irc_user_format_nick(struct irc_user *user)
{
    free(user->formatted);
    user->formatted = strdup(user->nick);
}

struct irc_user *irc_user_new(void)
{
    struct irc_user *user = malloc(sizeof(*user));
    irc_user_init(user);
    return user;
}

void irc_user_free(struct irc_user *user)
{
    irc_user_clear(user);
    free(user);
}


