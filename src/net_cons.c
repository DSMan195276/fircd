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
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#include "debug.h"
#include "config.h"
#include "network.h"
#include "net_cons.h"

void network_cons_init(struct network_cons *con)
{
    memset(con, 0, sizeof(struct network_cons));

    buf_init(&con->cmdfd);
}

void network_cons_clear(struct network_cons *con)
{
    network_clear_all(con->head);

    buf_free(&con->cmdfd);

    unlink("cmd");
}

void network_cons_init_directory(struct network_cons *con)
{
    struct network *tmp;

    mkfifo("cmd", 0755);
    con->cmdfd.fd = open("cmd", O_RDWR | O_NONBLOCK, 0);

    for (tmp = con->head; tmp != NULL; tmp = tmp->next)
        network_setup_files(tmp);
}

void network_cons_connect_networks (struct network_cons *con)
{
    struct network *tmp;

    for (tmp = con->head; tmp != NULL; tmp = tmp->next)
        network_connect(tmp);
}

void network_cons_set_select_desc (struct network_cons *con, fd_set *infd, fd_set *outfd, int *maxfd)
{
    struct network *tmp;
    if (con->cmdfd.fd != -1) {
        FD_SET(con->cmdfd.fd, infd);
        if (con->cmdfd.fd > *maxfd)
            *maxfd = con->cmdfd.fd;
    }
    for (tmp = con->head; tmp != NULL; tmp = tmp->next)
        network_init_select_desc(tmp, infd, outfd, maxfd);
}

static void handle_networks(struct network_cons *con)
{
    struct network *net, *tmp;
    for (net = con->head; net != NULL; net = tmp) {
        tmp = net->next;
        if (net->close_network)
            network_clear(net);
    }
}

void network_cons_handle_file_check(struct network_cons *con, fd_set *infd, fd_set *outfd)
{
    struct network *tmp;
    if (FD_ISSET(con->cmdfd.fd, infd)) {
        buf_handle_input(&(con->cmdfd));
        while (con->cmdfd.has_line > 0) {
            char *line = buf_read_line(&(con->cmdfd));
            DEBUG_PRINT("Cmd: %s", line);
            free(line);
        }
    }

    for (tmp = con->head; tmp != NULL; tmp = tmp->next)
        network_handle_input(tmp, infd, outfd);

    handle_networks(con);
}

void network_cons_load_config(struct network_cons *con)
{
    struct network *tmp, *cur;
    int i;
    ARRAY_FOREACH(prog_config.auto_login, i) {
        for (cur = prog_config.first; cur != NULL; cur = cur->next)
            if (strcmp(cur->name, prog_config.auto_login.arr[i]) == 0)
                break;
        if (cur != NULL) {
            tmp = network_copy(cur);
            tmp->con = con;
            tmp->next = con->head;
            con->head = tmp;
        }
    }
}

