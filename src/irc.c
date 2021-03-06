/*
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */
#include "global.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/fcntl.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdarg.h>

#include "debug.h"
#include "irc.h"

void irc_reply_free (struct irc_reply *rpl)
{
    int index = 0;

    free(rpl->raw);
    free(rpl->prefix.raw);
    free(rpl->prefix.user);
    free(rpl->prefix.host);

    DEBUG_PRINT("Array size: %d", ARRAY_SIZE(rpl->lines));

    ARRAY_FOREACH(rpl->lines, index)
        free(rpl->lines.arr[index]);
    ARRAY_FREE(rpl->lines);
    free(rpl->colon);
    free(rpl->cmd);
    free(rpl);
}

static void irc_parse_prefix (struct irc_prefix *prefix)
{
    char *tmp;
    int len;

    tmp = prefix->raw;
    for (; *tmp && *tmp != '!'; tmp++);
    if (*tmp == '!') {
        len = tmp - prefix->raw;
        prefix->user = malloc(len + 1);
        memcpy(prefix->user, prefix->raw, len);
        prefix->user[len] = '\0';
        tmp += 2;
    } else {
        tmp = prefix->raw;
    }

    len = strlen(tmp);
    prefix->host = malloc(len + 1);
    strcpy(prefix->host, tmp);
}

struct irc_reply *irc_parse_line (const char *line)
{
    const char *tmp;
    const char *cur;
    int code, len, exit_flag = 0, size;
    struct irc_reply *rpl;
    if (!line)
        return NULL;

    rpl = malloc(sizeof(struct irc_reply));
    memset(rpl, 0, sizeof(struct irc_reply));

    rpl->raw = strdup(line);

    cur = line;

    if (cur[0] == ':') {
        cur++;
        tmp = cur;
        for (; *tmp && *tmp != ' '; tmp++);
        if (*tmp == ' ') {
            rpl->prefix.raw = malloc(tmp - cur + 1);
            memcpy(rpl->prefix.raw, cur, tmp - cur);
            rpl->prefix.raw[tmp - cur] = '\0';
            cur = tmp + 1;
            irc_parse_prefix(&rpl->prefix);
        }
    }

    code = atoi(cur);

    if (code != 0) {
        rpl->code = code;
        cur += 4;
    } else {
        tmp = cur;
        for (; *tmp && *tmp != ' '; tmp++);
        if (*tmp == ' ') {
            rpl->cmd = malloc(tmp - cur + 1);
            memcpy(rpl->cmd, cur, tmp - cur);
            rpl->cmd[tmp - cur] = '\0';
            cur = tmp + 1;
        }
    }

    tmp = cur;

    for (; *tmp && !exit_flag; tmp++) {
        switch(*tmp) {
        case ':':
            len = strlen(tmp);
            rpl->colon = malloc(len + 1);
            strcpy(rpl->colon, tmp + 1);
            exit_flag = 1;
            break;
        case ' ':
            size = ARRAY_SIZE(rpl->lines);
            ARRAY_RESIZE(rpl->lines, size + 1);
            rpl->lines.arr[size] = malloc(tmp - cur + 1);
            memcpy(rpl->lines.arr[size], cur, tmp - cur);
            rpl->lines.arr[size][tmp - cur] = '\0';
            cur = tmp + 1;
            break;
        default:
            break;
        }
    }

    if (!exit_flag) {
        len = strlen(cur);
        size = ARRAY_SIZE(rpl->lines);
        ARRAY_RESIZE(rpl->lines, size + 1);
        rpl->lines.arr[size] = malloc(len + 1);
        strcpy(rpl->lines.arr[size], cur);
    }

    return rpl;
}

void irc_connect (struct network *net)
{
    struct sockaddr_in sin;
    struct hostent *hp = gethostbyname(net->url);

    memset(&sin, 0, sizeof(struct sockaddr_in));
    if(!hp) {
        net->close_network = 1;
    }
    sin.sin_family = AF_INET;
    memcpy(&sin.sin_addr, hp->h_addr_list[0], hp->h_length);
    sin.sin_port = htons(net->portno);
    if((net->sock.fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        net->close_network = 1;
        return ;
    }

    if(connect(net->sock.fd, (const struct sockaddr *) &sin, sizeof(sin)) < 0) {
        net->close_network = 1;
        return ;
    }

    fcntl(net->sock.fd, F_SETFL, O_NONBLOCK | fcntl(net->sock.fd, F_GETFL));
}

void irc_send_raw (struct network *net, const char *text, ...)
{
    va_list list;
    DEBUG_PRINT("%s: %s", net->name, text);

    va_start(list, text);
    fdprintfv(net->sock.fd, text, list);
    va_end(list);

    write(net->sock.fd, CRLF, 2);
}

void irc_nick (struct network *net)
{
    irc_send_raw(net, "NICK %s", net->nickname);
}

void irc_user (struct network *net)
{
    if (net->realname)
        irc_send_raw(net, "USER %s 0 * :%s", net->nickname, net->realname);
    else
        irc_send_raw(net, "USER %s 0 * :%s", net->nickname, net->nickname);
}

void irc_pass (struct network *net)
{
    irc_send_raw(net, "PASS %s", net->password);
}

void irc_join (struct network *net, const char *chan)
{
    irc_send_raw(net, "JOIN %s", chan);
}

void irc_part (struct network *net, const char *chan, const char *msg)
{
    if (msg)
        irc_send_raw(net, "PART %s :%s", chan, msg);
    else
        irc_send_raw(net, "PART %s", chan);
}

void irc_quit (struct network *net, const char *msg)
{
    if (msg)
        irc_send_raw(net, "QUIT :%s", msg);
    else
        irc_send_raw(net, "QUIT");
}

void irc_privmsg (struct network *net, const char *chan, const char *text)
{
    irc_send_raw(net, "PRIVMSG %s :%s", chan, text);
}

