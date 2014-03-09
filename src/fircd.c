/*
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#include "global.h"

#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>

#include "debug.h"
#include "daemon.h"
#include "arg.h"
#include "net_cons.h"

static struct network_cons state;

static void sig_int_handler(int sig)
{
    DEBUG_PRINT("Recieved signal: %d", sig);
    daemon_kill(&state);
}

static void sig_segv_handler(int seg)
{
    exit(0);
}

static void init_directory(void)
{
    mkdir(prog_config.root_directory, 0755);
    chdir(prog_config.root_directory);

    network_cons_init_directory(&state);
}

int main(int argc, char **argv)
{
    int ret;
    int maxfd = 0;
    fd_set infd, outfd;

    DEBUG_INIT();
    DEBUG_PRINT("Starting up...");

    config_init();

    network_cons_init(&state);

    DEBUG_PRINT("Parsing arguments...");
    parse_cmd_args(argc, argv);

    if (config_read() == 1)
        return 1;

    if (!prog_config.arg_dont_auto_load)
        network_cons_load_config(&state);

    if (!prog_config.stay_in_forground)
        daemon_init(&state);

    init_directory();
    network_cons_connect_networks(&state);

    signal(SIGILL, sig_int_handler);
    signal(SIGINT, sig_int_handler);
    signal(SIGQUIT, sig_int_handler);
    signal(SIGTERM, sig_int_handler);

    signal(SIGSEGV, sig_segv_handler);

    while (1) {
        FD_ZERO(&infd);
        FD_ZERO(&outfd);
        maxfd = 0;

        network_cons_set_select_desc(&state, &infd, &outfd, &maxfd);

        if ((ret = select(maxfd + 1,
                          &infd,
                          &outfd,
                          NULL, NULL))) {
            DEBUG_PRINT("Select Ret: %d", ret);
            network_cons_handle_file_check(&state, &infd, &outfd);
        }
    }

    return 0;
}

