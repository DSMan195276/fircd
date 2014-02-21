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

static void r_default(struct network *net, struct irc_reply *rpl)
{

}

static void r_ping(struct network *net, struct irc_reply *rpl)
{
    char *tmp = alloc_sprintf("PONG :%s", rpl->colon);
    irc_send_raw(net, tmp);
    free(tmp);
}

static void r_privmsg(struct network *net, struct irc_reply *rpl)
{
    struct channel *chan;
    char *user;

    user = rpl->prefix.user;

    DEBUG_PRINT("PRIVMSG: %s %s", user, rpl->lines.arr[0]);

    for (chan = net->head; chan != NULL; chan = chan->next) {
        DEBUG_PRINT("Checking channel: %s", chan->name);
        if (strcmp(rpl->lines.arr[0], chan->name) == 0) {
            channel_write_msg (chan, user, rpl->colon);
            return;
        }
    }

    /* Add this new channel */
    if (strcmp(rpl->lines.arr[0], net->nickname) == 0)
        chan = network_add_channel(net, user);
    else
        chan = network_add_channel(net, rpl->lines.arr[0]);

    channel_setup_files(chan);

    channel_write_msg(chan, user, rpl->colon);
}

static void r_motd(struct network *net, struct irc_reply *rpl)
{
    if (rpl->code == RPL_MOTDSTART)
        network_write_motd_start(net);
    else if (rpl->code == RPL_MOTD)
        network_write_motd_line(net, rpl->colon);
}

static void r_topic(struct network *net, struct irc_reply *rpl)
{
    const char *chan_nam;
    struct channel *chan;

    if (rpl->code == 332)
        chan_nam = rpl->lines.arr[1];
    else
        chan_nam = rpl->lines.arr[0];

    DEBUG_PRINT("Got a TOPIC");
    for (chan = net->head; chan != NULL; chan = chan->next) {
        DEBUG_PRINT("Checking channel: %s", chan->name);
        if (strcmp(chan_nam, chan->name) == 0)
            channel_write_topic(chan, rpl->colon, rpl->prefix.user);
    }
}

struct reply_handler reply_handler_list[] = {
    { NULL,      0,             r_default },
    { "PING",    0,             r_ping },
    { "PRIVMSG", 0,             r_privmsg },
    { NULL,      RPL_MOTDSTART, r_motd },
    { NULL,      RPL_MOTD,      r_motd },
    { NULL,      RPL_TOPIC,     r_topic },
    { "TOPIC",   0,             r_topic },
    { 0 }
};

