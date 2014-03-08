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

static const char default_config_file[] = "~/.fircdrc";

void network_cons_init(struct network_cons *con)
{
    memset(con, 0, sizeof(struct network_cons));

    buf_init(&con->cmdfd);
}

void network_cons_clear(struct network_cons *con)
{
    network_clear_all(con->head);
    config_clear(&(con->conf));

    buf_free(&con->cmdfd);

    unlink("cmd");

    free(con->config_file);
    free(con->dir);
}

static void apply_net_override_settings(struct network_cons *con)
{
    struct network *net;
    for (net = con->head; net != NULL; net = net->next) {
        if (net->remove_files_on_close == -1)
            net->remove_files_on_close = con->conf.remove_files_on_close;
    }

    if (con->stay_in_forground)
        con->conf.stay_in_forground = 1;

    if (con->dir) {
        free(con->conf.root_directory);
        con->conf.root_directory = strdup(con->dir);
    }
}

/* Returns 0 if file could be opened and parsed, or no file was specified and
 * the default file didn't exist. Returns 1 if the file was specified and it
 * didn't exist. */
int network_cons_config_read(struct network_cons *con)
{
    int no_conf = 0, ret;
    const char *filename;

    if (con->no_config)
        return 0;

    if (con->config_file == NULL) {
        no_conf = 1;
        filename = default_config_file;
    } else {
        filename = con->config_file;
    }

    ret = config_read(&(con->conf), filename);
    if (ret == 0)
        apply_net_override_settings(con);

    if (ret == 2)
        return !no_conf;
    else
        return  ret;
}

void network_cons_init_directory(struct network_cons *con)
{
    struct network *tmp;

    mkdir(con->conf.root_directory, 0755);
    chdir(con->conf.root_directory);

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

void network_cons_auto_login (struct network_cons *con)
{
    struct network *tmp, *cur;
    int i;
    ARRAY_FOREACH(con->conf.auto_login, i) {
        for (cur = con->conf.first; cur != NULL; cur = cur->next)
            if (strcmp(cur->name, con->conf.auto_login.arr[i]) == 0)
                break;
        if (cur != NULL) {
            tmp = network_copy(cur);
            tmp->con = con;
            tmp->next = con->head;
            con->head = tmp;
        }
    }
}

