/*
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */
#ifndef INCLUDE_REPLIES_H
#define INCLUDE_REPLIES_H

#include "network.h"
#include "irc.h"

struct reply_handler {
    const char *cmd;
    enum irc_reply_code code;
    void (*handler) (struct network *, struct irc_reply *);
};

extern struct reply_handler reply_handler_list[];

#endif
