/*
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#include "global.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "debug.h"
#include "buf.h"
#include "channel.h"
#include "fircd.h"
#include "irc.h"
#include "replies.h"
#include "network.h"

void network_init(struct network *net, struct network_cons *con)
{
    memset(net, 0, sizeof(struct network));
    net->portno = DEFAULT_PORT;
    if (con) {
        net->con = con;
        net->remove_files_on_close = con->conf.remove_files_on_close;
    }

    buf_init(&net->sock);
    buf_init(&net->cmdfd);
    net->joinedfd = -1;
    net->motdfd = -1;
    net->rawfd = -1;
    net->realnamefd = -1;
    net->nicknamefd = -1;
}

void network_setup_files (struct network *net)
{
    struct channel *tmp;

    if (!net->name)
        return ;

    mkdir(net->name, 0775);
    chdir(net->name);

    mkfifo("cmd", 0772);
    net->cmdfd.fd = open("cmd", BUF_FIFO_OPEN_FLAGS, 0);

    net->rawfd      = open("raw",      BUF_FILE_OPEN_FLAGS, 0750);
    net->joinedfd   = open("joined",   BUF_FILE_OPEN_FLAGS, 0750);
    net->motdfd     = open("motd",     BUF_FILE_OPEN_FLAGS, 0750);
    net->realnamefd = open("realname", BUF_FILE_OPEN_FLAGS, 0750);
    net->nicknamefd = open("nickname", BUF_FILE_OPEN_FLAGS, 0750);

    chdir("..");

    for (tmp = net->head; tmp != NULL; tmp = tmp->next)
        channel_setup_files(tmp);
}

void network_delete_files (struct network *net)
{
    DEBUG_PRINT("Removing files...");
    chdir(net->name);

    unlink("cmd");
    unlink("raw");
    unlink("joined");
    unlink("motd");
    unlink("realname");
    unlink("nickname");

    chdir("..");
    rmdir(net->name);
}

void network_init_select_desc(struct network *net, fd_set *infd, fd_set *outfd, int *maxfd)
{
    struct channel *tmp;

    if (net->sock.fd != -1) {
        FD_SET(net->sock.fd, infd);
        if (net->sock.fd > *maxfd)
            *maxfd = net->sock.fd;
    }

    if (net->cmdfd.fd != -1) {
        FD_SET(net->cmdfd.fd, infd);
        if (net->cmdfd.fd > *maxfd)
            *maxfd = net->cmdfd.fd;
    }

    for (tmp = net->head; tmp != NULL; tmp = tmp->next)
        channel_init_select_desc(tmp, infd, outfd, maxfd);
}

static void handle_cmd_line (struct network *net, char *line)
{

}

static void handle_irc_line (struct network *net, char *line)
{
    struct reply_handler *hand;
    struct irc_reply *rpl;
    int index;
    network_write_raw(net, line);

    rpl = irc_parse_line(line);

    if(rpl == NULL)
        DEBUG_PRINT("!!!!ERROR!!!!");

    DEBUG_PRINT("Reply:");
    DEBUG_PRINT("Prefix: %s", rpl->prefix.raw);
    DEBUG_PRINT("Prefix User: %s", rpl->prefix.user);
    DEBUG_PRINT("Prefix Host: %s", rpl->prefix.host);
    DEBUG_PRINT("Code: %d", rpl->code);
    DEBUG_PRINT("Cmd: %s", rpl->cmd);

    ARRAY_FOREACH(rpl->lines, index)
        DEBUG_PRINT("Line %d: %s", index, rpl->lines.arr[index]);

    DEBUG_PRINT("Colon: %s", rpl->colon);

    for (hand = reply_handler_list; hand->handler != NULL; hand++) {
        if (hand->cmd && rpl->cmd) {
            if (strcmp(hand->cmd, rpl->cmd) == 0) {
                DEBUG_PRINT("Handler: %p", hand->handler);
                (hand->handler) (net, rpl);
                goto cleanup;
            }
        } else if (hand->code > 0) {
            if (hand->code == rpl->code) {
                DEBUG_PRINT("Handler: %p", hand->handler);
                (hand->handler) (net, rpl);
                goto cleanup;
            }
        }
    }

    if (hand->handler == NULL)
        (reply_handler_list[0].handler) (net, rpl);

cleanup:
    irc_reply_free(rpl);
}

void network_handle_input (struct network *net, fd_set *infd, fd_set *outfd)
{
    struct channel *tmp;
    if (FD_ISSET(net->cmdfd.fd, infd)) {
        buf_handle_input(&(net->cmdfd));
        while (net->cmdfd.has_line > 0) {
            char *line = buf_read_line(&(net->cmdfd));
            handle_cmd_line(net, line);
            free(line);
        }
    }

    if (FD_ISSET(net->sock.fd, infd)) {
        buf_handle_input(&(net->sock));
        if (net->sock.closed_gracefully) {
            DEBUG_PRINT("Connection to %s was closed", net->name);
            net->close_network = 1;
        }
        while (net->sock.has_line > 0) {
            char *line = buf_read_line(&(net->sock));
            line[strlen(line)] = '\0';
            handle_irc_line(net, line);
            free(line);
        }
    }

    for (tmp = net->head; tmp != NULL; tmp = tmp->next)
        channel_handle_input(tmp, infd, outfd);
}

void network_connect(struct network *net)
{
    struct channel *tmp;
    irc_connect(net);
    if (net->close_network)
        return ;

    irc_nick(net);
    network_write_nick(net);
    irc_user(net);
    network_write_realname(net);
    if (net->password)
        irc_pass(net);

    for (tmp = net->head; tmp != NULL; tmp = tmp->next)
        irc_join(net, tmp->name);

    network_write_joined(net);
}

struct network *network_copy (struct network *net)
{
    struct channel *tmp;
    struct network *newnet = malloc(sizeof(struct network));

    network_init(newnet, NULL);

    if (net->name)
        newnet->name = strdup(net->name);
    if (net->url)
        newnet->url = strdup(net->url);

    newnet->portno = net->portno;

    if (net->nickname)
        newnet->nickname = strdup(net->nickname);
    if (net->realname)
        newnet->realname = strdup(net->realname);
    if (net->password)
        newnet->password = strdup(net->password);

    newnet->remove_files_on_close = net->remove_files_on_close;
    newnet->close_network = net->close_network;

    for (tmp = net->head; tmp != NULL; tmp = tmp->next)
        network_add_channel(newnet, tmp->name);

    return newnet;
}

struct channel *network_add_channel (struct network *net, const char *channel)
{
    struct channel *tmp_chan;
    tmp_chan = malloc(sizeof(struct channel));
    channel_init(tmp_chan);
    tmp_chan->name = strdup(channel);

    tmp_chan->net  = net;
    tmp_chan->next = net->head;
    net->head  = tmp_chan;

    return tmp_chan;
}

struct channel *network_find_channel (struct network *net, const char *channel)
{
    struct channel *chan;

    for (chan = net->head; chan != NULL; chan = chan->next)
        if (strcmp(chan->name, channel) == 0)
            return chan;

    return NULL;
}

void network_write_raw (struct network *net, const char *text)
{
    if (text) {
        write(net->rawfd, text, strlen(text));
        write(net->rawfd, "\n", 1);
    }
}

void network_write_nick (struct network *net)
{
    ftruncate(net->nicknamefd, 0);
    lseek(net->nicknamefd, 0, SEEK_SET);
    if (net->nickname)
        write(net->nicknamefd, net->nickname, strlen(net->nickname));
}

void network_write_realname (struct network *net)
{
    ftruncate(net->realnamefd, 0);
    lseek(net->realnamefd, 0, SEEK_SET);
    if (net->realname)
        write(net->realnamefd, net->realname, strlen(net->realname));
    else
        write(net->realnamefd, net->nickname, strlen(net->nickname));
}

void network_write_motd_start (struct network *net)
{
    const char motd_start[] = "New MOTD:\n";
    write(net->motdfd, motd_start, sizeof(motd_start));
}

void network_write_motd_line (struct network *net, const char *motd)
{
    write(net->motdfd, motd, strlen(motd));
    write(net->motdfd, "\n", 1);
}

void network_write_joined (struct network *net)
{
    struct channel *chan;
    ftruncate(net->joinedfd, 0);
    lseek(net->joinedfd, 0, SEEK_SET);
    for (chan = net->head; chan != NULL; chan = chan->next) {
        write(net->joinedfd, chan->name, strlen(chan->name));
        write(net->joinedfd, "\n", 1);
    }
}

void network_clear (struct network *current)
{
    int i;

    channel_clear_all(current->head);

    CLOSE_FD(current->sock.fd);
    buf_free(&current->sock);

    CLOSE_FD(current->cmdfd.fd);
    buf_free(&current->cmdfd);

    CLOSE_FD(current->joinedfd);
    CLOSE_FD(current->motdfd);
    CLOSE_FD(current->rawfd);
    CLOSE_FD(current->realnamefd);
    CLOSE_FD(current->nicknamefd);

    if (current->remove_files_on_close)
        network_delete_files(current);

    free(current->name);
    free(current->url);
    free(current->realname);
    free(current->nickname);
    free(current->password);

    ARRAY_FOREACH(current->joined, i)
        free(current->joined.arr[i]);
    ARRAY_FREE(current->joined);
}

void network_clear_all(struct network *net)
{
    struct network *tmp, *current = net;
    for (; current != NULL; current = tmp) {
        tmp = current->next;

        network_clear(current);

        free(current);
    }
}

