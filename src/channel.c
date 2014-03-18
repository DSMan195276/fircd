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
#include "array.h"
#include "buf.h"
#include "irc.h"
#include "net_cons.h"
#include "rbtree.h"
#include "user.h"
#include "fassert.h"
#include "channel.h"

void channel_init (struct channel *chan)
{
    fassert(chan);
    memset(chan, 0, sizeof(struct channel));

    buf_init(&chan->in);
}

void channel_clear (struct channel *current)
{
    fassert(current);
    struct channel_irc_user_node *user, *tmp;

    CLOSE_FD(current->in.fd);
    buf_free(&current->in);

    CLOSE_FD(current->outfd);
    CLOSE_FD(current->onlinefd);
    CLOSE_FD(current->topicfd);
    CLOSE_FD(current->rawfd);
    CLOSE_FD(current->msgsfd);

    free(current->name);
    free(current->topic);
    free(current->topic_user);

    for (user = current->first_user; user != NULL; user = tmp) {
        tmp = user->next;
        irc_user_clear(&user->user);
        free(user);
    }

    free(current);
}

void channel_create_files (struct channel *chan)
{
    fassert(chan);
    mkdir(chan->name, 0775);
    chdir(chan->name);

    mkfifo("in", 0772);
    chan->in.fd = open("in", BUF_FIFO_OPEN_FLAGS, 0);

    chan->outfd    = open("out",    BUF_FILE_OPEN_FLAGS, 0750);
    chan->onlinefd = open("online", BUF_FILE_OPEN_FLAGS, 0750);
    chan->topicfd  = open("topic",  BUF_FILE_OPEN_FLAGS, 0750);
    chan->rawfd    = open("raw",    BUF_FILE_OPEN_FLAGS, 0750);
    chan->msgsfd   = open("msgs",   BUF_FILE_OPEN_FLAGS, 0750);

    chdir("..");
}

void channel_remove_files (struct channel *chan)
{
    fassert(chan);
    chdir(chan->name);

    unlink("in");
    unlink("out");
    unlink("online");
    unlink("topic");
    unlink("raw");
    unlink("msgs");

    chdir("..");
    rmdir(chan->name);
}

static void channel_write_raw_timestamp(struct channel *chan)
{
    time_t cur_time;
    struct tm *tmp;
    char time_buf[100];
    size_t len;

    fassert(chan);

    time(&cur_time);
    tmp = localtime(&cur_time);

    len = strftime(time_buf, sizeof(time_buf), "%F %H-%M-%S:", tmp);
    write(chan->rawfd, time_buf, len);
}

static void channel_write_msg(struct channel *chan, const char *user, const char *line)
{
    const char *format = " <%s> : %s\n";

    fassert(chan);
    fassert(user);
    fassert(line);

    DEBUG_PRINT("Writing msg: %s: %s", user, line);
    fdprintf(chan->msgsfd, format, user, line);
    fdprintf(chan->outfd, format, user, line);

    channel_write_raw_timestamp(chan);
    fdprintf(chan->rawfd, "MSG %s: %s\n", user, line);
}

static void channel_write_topic(struct channel *chan)
{
    fassert(chan);

    ftruncate(chan->topicfd, 0);
    lseek(chan->topicfd, 0, SEEK_SET);

    if (chan->topic_user)
        fdprintf(chan->topicfd, "%s: \"%s\"\n", chan->topic_user, chan->topic);
    else
        fdprintf(chan->topicfd, "\"%s\"\n", chan->topic);
}

static void channel_write_users(struct channel *chan)
{
    struct irc_user *user;

    fassert(chan);

    ftruncate(chan->onlinefd, 0);
    lseek(chan->onlinefd, 0, SEEK_SET);

    channel_foreach_user(chan, user) {
        write(chan->onlinefd, user->formatted, strlen(user->formatted));
        write(chan->onlinefd, "\n", 1);
    }
}

void channel_reg_select (struct channel *chan, fd_set *infd, fd_set *outfd, int *maxfd)
{
    fassert(chan);
    fassert(infd);
    fassert(outfd);
    fassert(maxfd);

    if (chan->in.fd != -1) {
        FD_SET(chan->in.fd, infd);
        if (chan->in.fd > *maxfd)
            *maxfd = chan->in.fd;
    }
}

void channel_handle_input (struct channel *chan, fd_set *infd, fd_set *outfd)
{
    fassert(chan);
    fassert(infd);
    fassert(outfd);

    if (FD_ISSET(chan->in.fd, infd)) {
        buf_handle_input(&(chan->in));
        while (chan->in.has_line > 0) {
            char *line = buf_read_line(&(chan->in));
            if (line[0] != '/') {
                irc_privmsg(chan->net, chan->name, line);
                channel_write_msg(chan, chan->net->nickname, line);
            }
            free(line);
        }
    }
}

void channel_new_topic (struct channel *chan, const char *user, const char *topic)
{
    fassert(chan);
    fassert(user);
    fassert(topic);

    if (chan->topic)
        free(chan->topic);
    if (chan->topic_user)
        free(chan->topic_user);

    if (topic)
        chan->topic = strdup(topic);
    if (user)
        chan->topic_user = strdup(user);

    channel_write_topic(chan);

    channel_write_raw_timestamp(chan);
    if (user)
        fdprintf(chan->rawfd, "TOPIC %s:%s\n", user, topic);
    else
        fdprintf(chan->rawfd, "TOPIC :%s\n", topic);

    if (user)
        fdprintf(chan->outfd, "%s set the topic to %s\n", user, topic);
    else
        fdprintf(chan->outfd, "Topic is %s\n", topic);
}

void channel_new_message (struct channel *chan, const char *user, const char *line)
{
    fassert(chan);
    fassert(user);
    fassert(line);

    channel_write_msg(chan, user, line);
}

void channel_user_online(struct channel *chan, const struct irc_user *user_cpy)
{
    struct channel_irc_user_node *user, **current;

    fassert(chan);
    fassert(user_cpy);

    user = malloc(sizeof(*user));
    user->next = NULL;
    irc_user_init(&user->user);
    irc_user_cpy(&user->user, user_cpy);

    irc_user_format_nick(&user->user);

    for (current = &chan->first_user; *current != NULL; current = &((*current)->next)) {
        int cmp = strcmp((*current)->user.nick, user->user.nick);
        if (cmp < 0) {
            break;
        } else if (cmp == 0) {
            irc_user_clear(&user->user);
            free(user);
            return ;
        }
    }

    user->next = *current;
    *current = user;

    channel_write_users(chan);
}

void channel_user_join(struct channel *chan, const struct irc_user *user_cpy)
{
    fassert(chan);
    fassert(user_cpy);

    channel_user_online(chan, user_cpy);

    fdprintf(chan->outfd, "join > %s\n", user_cpy->nick);

    channel_write_raw_timestamp(chan);
    fdprintf(chan->rawfd, "JOIN %s\n", user_cpy->nick);
}

static int try_remove_user (struct channel *chan, const char *nick)
{
    struct channel_irc_user_node **user, *found;

    fassert(chan);
    fassert(nick);

    for (user = &chan->first_user; *user != NULL; user = &((*user)->next)) {
        int cmp = strcmp((*user)->user.nick, nick);
        if (cmp == 0)
            break;
        else if (cmp < 0)
            return 0;
    }

    found = *user;

    if (!found)
        return 0;

    *user = (*user)->next;

    irc_user_clear(&found->user);
    free(found);
    return 1;
}

void channel_user_part (struct channel *chan, const char *nick)
{
    fassert(chan);
    fassert(nick);

    if (!try_remove_user(chan, nick))
        return;

    channel_write_users(chan);

    fdprintf(chan->outfd, "part > %s\n", nick);

    channel_write_raw_timestamp(chan);
    fdprintf(chan->rawfd, "PART %s\n", nick);

    return ;
}

void channel_user_quit (struct channel *chan, const char *nick)
{
    fassert(chan);
    fassert(nick);

    if (!try_remove_user(chan, nick))
        return;

    channel_write_users(chan);

    fdprintf(chan->outfd, "quit < %s\n", nick);

    channel_write_raw_timestamp(chan);
    fdprintf(chan->rawfd, "QUIT %s\n", nick);

    return ;
}

void channel_change_user(struct channel *chan, const char *old, const char *new)
{
    struct channel_irc_user_node **user, *found;

    fassert(chan);
    fassert(old);
    fassert(new);

    for (user = &chan->first_user; *user != NULL; user = &((*user)->next)) {
        int cmp = strcmp((*user)->user.nick, old);
        if (cmp == 0)
            break;
        else if (cmp < 0)
            return;
    }

    found = *user;
    *user = (*user)->next;

    free(found->user.nick);
    found->user.nick = strdup(new);

    for (user = &chan->first_user; *user != NULL; user = &((*user)->next))
        if (strcmp((*user)->user.nick, new) <= 0)
            break;

    found->next = (*user)->next;
    *user = found;

    irc_user_format_nick(&found->user);
    channel_write_users(chan);
}

