/*
 * ./fircd.c -- Provides basic initalization code, as well as functions for
 *              gathering the fd's for select(), and handling a select() by handling input
 *              from ./cmd, and calling the handle_input functions on every network
 *
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
#include "daemon.h"
#include "irc.h"
#include "buf.h"
#include "arg.h"
#include "fircd.h"

/*
 * This function initalizes the 'root' directory for fircd -- It creates it if
 * it doesn't exist, chdir's into it, and creates the main 'cmd' pipe.
 */
static void init_directory(void)
{
    struct network *tmp;

    mkdir(current_state->dir, 0755);
    chdir(current_state->dir);

    mkfifo("cmd", 0755);
    current_state->cmdfd.fd = open("cmd", O_RDWR | O_NONBLOCK, 0);

    for (tmp = current_state->head; tmp != NULL; tmp = tmp->next)
        network_setup_files(tmp);
}

static void init_networks(void)
{
    struct network *tmp;

    for (tmp = current_state->head; tmp != NULL; tmp = tmp->next)
        network_connect(tmp);
}

void set_select_desc(void)
{
    struct network *tmp;
    ADD_FD_CUR_STATE(current_state->cmdfd.fd);
    for (tmp = current_state->head; tmp != NULL; tmp = tmp->next)
        network_init_select_desc(tmp);
}

static void handle_networks(void)
{
    struct network **tmp;
    for (tmp = &current_state->head; *tmp != NULL; tmp = &(*tmp)->next) {
        if ((*tmp)->close_network) {
            (*tmp) = (*tmp)->next;
            network_clear(*tmp);
        }
    }
}

void handle_file_check(void)
{
    struct network *tmp;
    if (FD_ISSET(current_state->cmdfd.fd, &current_state->infd)) {
        buf_handle_input(&(current_state->cmdfd));
        while (current_state->cmdfd.has_line > 0) {
            char *line = buf_read_line(&(current_state->cmdfd));
            DEBUG_PRINT("Cmd: %s", line);
            free(line);
        }
    }

    for (tmp = current_state->head; tmp != NULL; tmp = tmp->next)
        network_handle_input(tmp);

    handle_networks();
}

int main(int argc, char **argv)
{
    DEBUG_INIT();
    DEBUG_PRINT("Starting up...");

    current_state_init();

    DEBUG_PRINT("Parsing arguments...");
    parse_cmd_args(argc, argv);

    if (config_file_parse() == 1) {
        printf("Error opening config file...\n");
        return 1;
    }

    if (!current_state->conf.stay_in_forground)
        daemon_init();

    init_directory();
    init_networks();

    daemon_main_loop();

    /* We shouldn't get here. If we do then just die */
    daemon_kill();

    return 0;
}

