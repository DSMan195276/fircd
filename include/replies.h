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

#define REPLY_HANDLER(name) void reply_handler_##name(struct network *net, struct irc_reply *rpl)

struct reply_handler {
    const char *cmd;
    enum irc_reply_code code;
    void (*handler) (struct network *, struct irc_reply *);
};

extern REPLY_HANDLER(default);
extern REPLY_HANDLER(ping);
extern REPLY_HANDLER(privmsg);
extern REPLY_HANDLER(motd);
extern REPLY_HANDLER(topic);

extern int    reply_handler_count;
extern struct reply_handler reply_handler_list[];

#endif
