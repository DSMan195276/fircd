/*
 * ./channel.c -- Handles managing a channel on an IRC network (Both, normal channels as well
 * as private-message channels)
 *
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation;
 */

#include "global.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/stat.h>

#include "debug.h"
#include "fircd.h"
#include "array.h"
#include "buf.h"
#include "irc.h"
#include "net_cons.h"
#include "channel.h"

void channel_init(struct channel *chan)
{
    memset(chan, 0, sizeof(struct channel));

    buf_init(&chan->infd);
}

void channel_setup_files(struct channel *chan)
{
    chdir(chan->net->name);

    mkdir(chan->name, 0775);
    chdir(chan->name);

    OPEN_FIFO(chan, in);

    OPEN_FILE(chan, out);
    OPEN_FILE(chan, online);
    OPEN_FILE(chan, topic);
    OPEN_FILE(chan, raw);
    OPEN_FILE(chan, msgs);

    chdir("..");
    chdir("..");
}

void channel_init_select_desc(struct channel *chan, fd_set *infd, fd_set *outfd, int *maxfd)
{
    if (chan->infd.fd != -1) {
        FD_SET(chan->infd.fd, infd);
        if (chan->infd.fd > *maxfd)
            *maxfd = chan->infd.fd;
    }
}

void channel_write_raw(struct channel *chan, const char *msg)
{
    time_t cur_time;
    struct tm *tmp;
    char time_buf[100];
    size_t len;
    time(&cur_time);
    tmp = localtime(&cur_time);

    len = strftime(time_buf, sizeof(time_buf), "%F %H-%M-%S:", tmp);
    write(chan->rawfd, time_buf, len);
    write(chan->rawfd, msg, strlen(msg));
}

void channel_write_out(struct channel *chan, const char *msg)
{
    write(chan->outfd, msg, strlen(msg));
}

void channel_write_msg(struct channel *chan, const char *user, const char *line)
{
    char *msg;

    msg = alloc_sprintf(" <%s> : %s\n", user, line);
    DEBUG_PRINT("Writing msg: %s", msg);
    write(chan->msgsfd, msg, strlen(msg));
    channel_write_out(chan, msg);
    free(msg);

    msg = alloc_sprintf("MSG %s: %s\n", user, line);
    channel_write_raw(chan, msg);
    free(msg);
}

void channel_write_topic(struct channel *chan, const char *topic, const char *user)
{
    char *msg;
    ftruncate(chan->topicfd, 0);
    lseek(chan->topicfd, 0, SEEK_SET);
    write(chan->topicfd, topic, strlen(topic));

    if (user)
        msg = alloc_sprintf("TOPIC %s:%s\n", user, topic);
    else
        msg = alloc_sprintf("TOPIC :%s\n", topic);
    channel_write_raw(chan, msg);
    free(msg);

    if (user)
        msg = alloc_sprintf("%s set the topic to %s\n", user, topic);
    else
        msg = alloc_sprintf("Topic is %s\n", topic);
    channel_write_out(chan, msg);
    free(msg);
}

static void channel_write_users(struct channel *chan)
{
    struct rbnode *current;
    struct irc_user *user;

    ftruncate(chan->onlinefd, 0);
    lseek(chan->onlinefd, 0, SEEK_SET);

    rb_foreach_inorder(&chan->nicks, current) {
        user = container_of(current, struct irc_user, node);
        write(chan->onlinefd, user->formatted, strlen(user->formatted));
        write(chan->onlinefd, "\n", 1);
    }
}

void channel_update_users (struct channel *chan)
{
    channel_write_users(chan);
}

void channel_add_user(struct channel *chan, const char *nick, struct irc_user_flags flags)
{
    struct irc_user *user;

    user = irc_user_new();
    user->nick = strdup(nick);
    user->flags = flags;

    irc_user_format_nick(user);

    rb_insert(&chan->nicks, &user->node);
    channel_write_users(chan);
}

struct irc_user *channel_get_user(struct channel *chan, const char *nick)
{
    struct irc_user user, *found;

    user.nick = (char *)nick;

    found = container_of(rb_search(&chan->nicks, &user.node), struct irc_user, node);

    return found;
}

int channel_del_user (struct channel *chan, const char *nick)
{
    struct irc_user *user = channel_get_user(chan, nick);

    if (!user)
        return 0;

    rb_remove(&chan->nicks, &user->node);
    irc_user_free(user);
    channel_write_users(chan);
    return 1;
}

void channel_change_user(struct channel *chan, const char *old, const char *new)
{
    struct irc_user *user = channel_get_user(chan, old);

    rb_remove(&chan->nicks, &user->node);

    free(user->nick);
    user->nick = strdup(new);

    rb_insert(&chan->nicks, &user->node);

    irc_user_format_nick(user);
    channel_write_users(chan);
}

void channel_handle_input(struct channel *chan, fd_set *infd, fd_set *outfd)
{
    if (FD_ISSET(chan->infd.fd, infd)) {
        buf_handle_input(&(chan->infd));
        while (chan->infd.has_line > 0) {
            char *line = buf_read_line(&(chan->infd));
            if (line[0] != '/') {
                irc_privmsg(chan->net, chan->name, line);
                channel_write_msg (chan, chan->net->nickname, line);
            }
            free(line);
        }
    }
}

void channel_delete_files(struct channel *chan)
{
    chdir(chan->net->name);
    chdir(chan->name);

    unlink("in");
    unlink("out");
    unlink("online");
    unlink("topic");

    chdir("..");
    rmdir(chan->name);
    chdir("..");
}

void channel_clear(struct channel *current)
{
    struct rbnode *current_node;

    CLOSE_FD_BUF(current->infd);
    CLOSE_FD(current->outfd);
    CLOSE_FD(current->onlinefd);
    CLOSE_FD(current->topicfd);
    CLOSE_FD(current->rawfd);
    CLOSE_FD(current->msgsfd);

    if (current->net->remove_files_on_close)
        channel_delete_files(current);

    free(current->name);

    rb_foreach_postorder(&current->nicks, current_node)
        irc_user_free(container_of(current_node, struct irc_user, node));

    free(current);
}

void channel_clear_all(struct channel *chan)
{
    struct channel *current = chan, *tmp;

    for (; current != NULL; current = tmp) {
        tmp = current->next;

        channel_clear(current);
    }
}

