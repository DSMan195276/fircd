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

    chdir("..");
    chdir("..");
}

void channel_init_select_desc(struct channel *chan)
{
    ADD_FD_CUR_STATE(chan->infd.fd);
}

void channel_write_msg(struct channel *chan, const char *user, const char *line)
{
    char *msg;

    msg = alloc_sprintf(" <%s> : %s\n", user, line);
    DEBUG_PRINT("Writing msg: %s", msg);
    write(chan->outfd, msg, strlen(msg));
    free(msg);
}

void channel_write_topic(struct channel *chan, const char *topic)
{
    ftruncate(chan->topicfd, 0);
    lseek(chan->topicfd, 0, SEEK_SET);
    write(chan->topicfd, topic, strlen(topic));
}

void channel_handle_input(struct channel *chan)
{
    if (FD_ISSET(chan->infd.fd, &current_state->infd)) {
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
    int i;

    CLOSE_FD_BUF(current->infd);
    CLOSE_FD(current->outfd);
    CLOSE_FD(current->onlinefd);
    CLOSE_FD(current->topicfd);

    if (current->net->remove_files_on_close)
        channel_delete_files(current);

    free(current->name);

    ARRAY_FOREACH(current->nicks, i)
        free(current->nicks.arr[i]);
    ARRAY_FREE(current->nicks);

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

