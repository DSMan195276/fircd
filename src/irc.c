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

    DEBUG_PRINT("Array size: %d", ARRAY_SIZE(*rpl, lines));

    ARRAY_FOREACH(*rpl, lines, index)
        free(rpl->lines[index]);
    ARRAY_FREE(*rpl, lines);
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
            size = ARRAY_SIZE(*rpl, lines);
            ARRAY_RESIZE(*rpl, lines, size + 1);
            rpl->lines[size] = malloc(tmp - cur + 1);
            memcpy(rpl->lines[size], cur, tmp - cur);
            rpl->lines[size][tmp - cur] = '\0';
            cur = tmp + 1;
            break;
        default:
            break;
        }
    }

    if (!exit_flag) {
        len = strlen(cur);
        size = ARRAY_SIZE(*rpl, lines);
        ARRAY_RESIZE(*rpl, lines, size + 1);
        rpl->lines[size] = malloc(len + 1);
        strcpy(rpl->lines[size], cur);
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

void irc_send_raw (struct network *net, const char *text)
{
    DEBUG_PRINT("%s: %s", net->name, text);
    write(net->sock.fd, text, strlen(text));
    write(net->sock.fd, CRLF, 2);
}

void irc_nick (struct network *net)
{
    char *buf;
    buf = alloc_sprintf("NICK %s", net->nickname);
    irc_send_raw(net, buf);
    free(buf);
}

void irc_user (struct network *net)
{
    char *buf;
    if (net->realname)
        buf = alloc_sprintf("USER %s 0 * :%s", net->nickname, net->realname);
    else
        buf = alloc_sprintf("USER %s 0 * :%s", net->nickname, net->nickname);
    irc_send_raw(net, buf);
    free(buf);
}

void irc_pass (struct network *net)
{
    char *buf;
    buf = alloc_sprintf("PASS %s", net->password);
    irc_send_raw(net, buf);
    free(buf);
}

void irc_join (struct network *net, const char *chan)
{
    char *buf;
    buf = alloc_sprintf("JOIN %s", chan);
    irc_send_raw(net, buf);
    free(buf);
}

void irc_part (struct network *net, const char *chan, const char *msg)
{
    char *buf;
    if (msg)
        buf = alloc_sprintf("PART %s :%s", chan, msg);
    else
        buf = alloc_sprintf("PART %s", chan);
    irc_send_raw(net, buf);
    free(buf);
}

void irc_quit (struct network *net, const char *msg)
{
    char *buf;
    if (msg)
        buf = alloc_sprintf("QUIT :%s", msg);
    else
        buf = alloc_sprintf("QUIT");
    irc_send_raw(net, buf);
    free(buf);
}

void irc_privmsg (struct network *net, const char *chan, const char *text)
{
    char *buf;
    buf = alloc_sprintf("PRIVMSG %s :%s", chan, text);
    irc_send_raw(net, buf);
    free(buf);
}

