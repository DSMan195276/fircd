/*
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#include "global.h"

#include "debug.h"
#include "daemon.h"
#include "arg.h"
#include "net_cons.h"
#include "fircd.h"

static struct network_cons state;

int main(int argc, char **argv)
{
    DEBUG_INIT();
    DEBUG_PRINT("Starting up...");

    network_cons_init(&state);

    DEBUG_PRINT("Parsing arguments...");
    parse_cmd_args(argc, argv, &state);

    if (network_cons_config_read(&state) == 1)
        return 1;

    if (!state.dont_auto_load)
        network_cons_auto_login(&state);

    if (!state.conf.stay_in_forground)
        daemon_init();

    network_cons_init_directory(&state);
    network_cons_connect_networks(&state);

    daemon_main_loop(&state);

    /* We shouldn't get here. If we do then just die */
    daemon_kill();

    return 0;
}
