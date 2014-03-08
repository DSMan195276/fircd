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
#include "network.h"
#include "channel.h"
#include "net_cons.h"
#include "daemon.h"

static int still_in_parent = 0;

void daemon_init(struct network_cons *con)
{
    pid_t pid;

    fflush(NULL);
    umask(0);

    DEBUG_PRINT("Forking...");
    pid = fork();

    if (pid < 0) {
        /* Error forking */
        printf("Error - Unable to fork()\n");
        daemon_kill(con);
    } else if (pid > 0) {
        /* End of parent */
        printf("Forked successfully\n");
        still_in_parent = 1;
        daemon_kill(con);
    }

    /* Child process starts here */

    /* Close old in/out tty files */
    freopen("/dev/null", "r", stdin);
    freopen("/dev/null", "w", stdout);
    freopen("/dev/null", "w", stderr);

    if (chdir("/") < 0) {
        printf("Unable to chdir to root\n");
        daemon_kill(con);
    }

    setsid();
}

void daemon_kill(struct network_cons *con)
{
    if (still_in_parent)
        DEBUG_PRINT("Parent process closing...");
    else
        DEBUG_PRINT("Process closing...");

    DEBUG_PRINT("Closing networks...");
    network_cons_clear(con);

    DEBUG_PRINT("Done.");
    DEBUG_CLOSE();
    exit(0);
}

