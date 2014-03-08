/*
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */
#ifndef INCLUDE_CHANNEL_H
#define INCLUDE_CHANNEL_H

#include "global.h"

#include <sys/types.h>
#include <sys/select.h>

#include "buf.h"
#include "net_cons.h"
#include "array.h"
#include "rbtree.h"
#include "user.h"

struct irc_user_node {
    struct irc_user user;
    struct irc_user_node *next;
};

/* 'channel' represents a node on a linked-list of channels */
struct channel {
    struct network *net;
    struct channel *next;

    struct irc_user_node *first_user;

    char *name;
    char *topic, *topic_user;

    int outfd;
    int onlinefd;
    int topicfd;
    int rawfd;
    int msgsfd;

    struct buf_fd in;
};

/* Call on creation and deletion of a channel
 * 'init' clears the channel's memory and inits the buf_fd
 */
extern void channel_init (struct channel *);
extern void channel_clear (struct channel *);
extern void channel_clear_all (struct channel *);

/* These functions create and delete the channels filesystem
 * When calling these functions, you should be sure to chdir into the directory
 * where you want the channel's directory contained. In most cases this amounts
 * to chdir'ing into the network's directory before calling these functions.
 */
extern void channel_create_files (struct channel *);
extern void channel_remove_files (struct channel *);

/* These are used for monitoring and handling input. 'reg_select' is
 * registering fd's to be watched by the main-loop's select() call.
 * 'handle_input' is called after select() returns, and checks to see if any
 * input was recieved for this channel.
 */
extern void channel_reg_select (struct channel *, fd_set *, fd_set *, int *);
extern void channel_handle_input (struct channel *, fd_set *, fd_set *);

/* These are for modifying the state of this channel. 'new_message' write's a
 * new message to this channel, from 'user', with contents 'line'. 'new_topic'
 * sets the current topic for the channel.
 */
extern void channel_new_message (struct channel *, const char *user, const char *line);
extern void channel_new_topic (struct channel *, const char *user, const char *topic);

/* These functions are for modifying the state of users in the channel.
 * 'online' is used to add a user to the online list without showing a 'joined'
 *     message This is used for adding the users who were in the channel when
 *     you joined.
 * 'join' is used when a new user joins.
 * 'part' is used when a user parts from the channel.
 * 'quit' is like part, but used when a user quits the network.
 * 'change' is used when a user changes nicknames.
 */
extern void channel_user_online (struct channel *, const struct irc_user *);
extern void channel_user_join (struct channel *, const struct irc_user *);
extern void channel_user_part (struct channel *, const char *user);
extern void channel_user_quit (struct channel *, const char *user);
extern void channel_user_change (struct channel *, const char *old, const char *new);

#endif
