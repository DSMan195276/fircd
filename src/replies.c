/*
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#include "global.h"

#include <string.h>

#include "debug.h"
#include "network.h"
#include "channel.h"
#include "replies.h"

struct reply_handler reply_handler_list[] = {
    { NULL,      0,             reply_handler_default },
    { "PING",    0,             reply_handler_ping },
    { "PRIVMSG", 0,             reply_handler_privmsg },
    { NULL,      RPL_MOTDSTART, reply_handler_motd },
    { NULL,      RPL_MOTD,      reply_handler_motd },
    { NULL,      RPL_TOPIC,     reply_handler_topic },
    { 0 }
};

REPLY_HANDLER(default)
{

}

REPLY_HANDLER(ping)
{
    char *tmp = alloc_sprintf("PONG :%s", rpl->colon);
    irc_send_raw(net, tmp);
    free(tmp);
}

REPLY_HANDLER(privmsg)
{
    struct channel *chan;
    char *user;

    user = rpl->prefix.user;

    DEBUG_PRINT("PRIVMSG: %s %s", user, rpl->lines[0]);

    for (chan = net->head; chan != NULL; chan = chan->next) {
        DEBUG_PRINT("Checking channel: %s", chan->name);
        if (strcmp(rpl->lines[0], chan->name) == 0) {
            channel_write_msg (chan, user, rpl->colon);
            return;
        }
    }

    /* Add this new channel */
    if (strcmp(rpl->lines[0], net->nickname) == 0)
        chan = network_add_channel(net, user);
    else
        chan = network_add_channel(net, rpl->lines[0]);

    channel_setup_files(chan);

    channel_write_msg(chan, user, rpl->colon);
}

REPLY_HANDLER(motd)
{
    if (rpl->code == RPL_MOTDSTART)
        network_write_motd_start(net);
    else if (rpl->code == RPL_MOTD)
        network_write_motd_line(net, rpl->colon);
}

REPLY_HANDLER(topic)
{
    struct channel *chan;
    for (chan = net->head; chan != NULL; chan = chan->next)
        if (strcmp(rpl->lines[0], chan->name) == 0)
            channel_write_topic(chan, rpl->colon);
}

