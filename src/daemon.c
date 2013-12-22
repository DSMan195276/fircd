/*
 * ./daemon.c -- Handles basic functions of the program: signals, main loop, and forking.
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
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <fcntl.h>
#include <signal.h>

#include "debug.h"
#include "fircd.h"
#include "network.h"
#include "channel.h"
#include "daemon.h"

static int still_in_parent = 0;

static void sig_int_handler(int sig)
{
    DEBUG_PRINT("Recieved signal: %d", sig);
    daemon_kill();
}

static void sig_segv_handler(int seg)
{
    exit(0);
}


void daemon_init(void)
{
    pid_t pid;

    fflush(NULL);
    umask(0);

    DEBUG_PRINT("Forking...");
    pid = fork();

    if (pid < 0) {
        /* Error forking */
        printf("Error - Unable to fork()\n");
        daemon_kill();
    } else if (pid > 0) {
        /* End of parent */
        printf("Forked successfully\n");
        still_in_parent = 1;
        daemon_kill();
    }

    /* Child process starts here */

    /* Close old in/out tty files */
    freopen("/dev/null", "r", stdin);
    freopen("/dev/null", "w", stdout);
    freopen("/dev/null", "w", stderr);

    if (chdir("/") < 0) {
        printf("Unable to chdir to root\n");
        daemon_kill();
    }

    setsid();
}

void daemon_kill(void)
{
    if (still_in_parent)
        DEBUG_PRINT("Parent process closing...");
    else
        DEBUG_PRINT("Process closing...");

    DEBUG_PRINT("Closing networks...");
    current_state_clear();

    DEBUG_PRINT("Done.");
    DEBUG_CLOSE();
    exit(0);
}

void daemon_main_loop(void)
{
    int ret;

    signal(SIGILL, sig_int_handler);
    signal(SIGINT, sig_int_handler);
    signal(SIGQUIT, sig_int_handler);
    signal(SIGTERM, sig_int_handler);

    signal(SIGSEGV, sig_segv_handler);

    DEBUG_PRINT("Done!");

    while (1) {
        FD_ZERO(&current_state->infd);
        FD_ZERO(&current_state->outfd);
        current_state->maxfd = 0;

        set_select_desc();

        if ((ret = select(current_state->maxfd + 1,
                          &current_state->infd,
                          &current_state->outfd,
                          NULL, NULL))) {
            DEBUG_PRINT("Select Ret: %d", ret);
            handle_file_check();
        }
    }
}

