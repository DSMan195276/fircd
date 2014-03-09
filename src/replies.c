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

    network_foreach_channel(net, chan) {
        DEBUG_PRINT("Checking channel: %s", chan->name);
        if (strcmp(rpl->lines.arr[0], chan->name) == 0) {
            channel_new_message(chan, user, rpl->colon);
            return;
        }
    }

    /* Add this new channel */
    if (strcmp(rpl->lines.arr[0], net->nickname) == 0)
        chan = network_add_channel(net, user);
    else
        chan = network_add_channel(net, rpl->lines.arr[0]);

    channel_create_files(chan);

    channel_new_message(chan, user, rpl->colon);
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
    network_foreach_channel(net, chan) {
        DEBUG_PRINT("Checking channel: %s", chan->name);
        if (strcmp(chan_nam, chan->name) == 0)
            channel_new_topic(chan, rpl->colon, rpl->prefix.user);
    }
}

static void r_join(struct network *net, struct irc_reply *rpl)
{
    struct channel *chan;
    struct irc_user user;

    chan = network_find_channel(net, rpl->lines.arr[0]);

    irc_user_init(&user);

    user.nick = strdup(rpl->prefix.user);
    user.flags = (struct irc_user_flags){ 0 };

    channel_user_join(chan, &user);

    irc_user_clear(&user);
}

static void r_part(struct network *net, struct irc_reply *rpl)
{
    struct channel *chan;

    DEBUG_PRINT("In Part!");

    chan = network_find_channel(net, rpl->lines.arr[0]);
    channel_user_part(chan, rpl->prefix.user);
}

static void r_quit(struct network *net, struct irc_reply *rpl)
{
    struct channel *chan;

    network_foreach_channel(net, chan)
        channel_user_quit(chan, rpl->prefix.user);
}

static void r_names(struct network *net, struct irc_reply *rpl)
{
    struct channel *chan;
    struct irc_user user;
    int name_count = 0, i, flag = 1;
    char **names = NULL;
    char *last, *cur = rpl->colon;

    chan = network_find_channel(net, rpl->lines.arr[2]);

    last = cur;
    for (; flag; cur++) {
        if (*cur == '\0' || *cur == ' ') {
            if (*cur == ' ')
                *cur = '\0';
            else
                flag = 0;

            name_count++;
            names = realloc(names, name_count * (sizeof(*names)));
            names[name_count-1] = last;

            last = cur + 1;
        }
    }

    irc_user_init(&user);
    for (i = 0; i < name_count; i++) {
        irc_user_conv(&user, names[i]);
        channel_user_online(chan, &user);
    }
    irc_user_clear(&user);

    free(names);
}

struct reply_handler reply_handler_list[] = {
    { NULL,      0,             r_default },
    { "PING",    0,             r_ping },
    { "PRIVMSG", 0,             r_privmsg },
    { NULL,      RPL_MOTDSTART, r_motd },
    { NULL,      RPL_MOTD,      r_motd },
    { NULL,      RPL_TOPIC,     r_topic },
    { NULL,      RPL_NAMREPLY,  r_names },
    { "TOPIC",   0,             r_topic },
    { "JOIN",    0,             r_join },
    { "PART",    0,             r_part },
    { "QUIT",    0,             r_quit },
    { 0 }
};

