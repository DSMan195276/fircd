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
#include "fircd.h"

int main(int argc, char **argv)
{
    DEBUG_INIT();
    DEBUG_PRINT("Starting up...");

    current_state_init();

    DEBUG_PRINT("Parsing arguments...");
    parse_cmd_args(argc, argv);

    if (config_file_parse() == 1)
        return 1;

    if (!current_state->dont_auto_load)
        setup_auto_load();

    if (!current_state->conf.stay_in_forground)
        daemon_init();

    init_directory();
    init_networks();

    daemon_main_loop();

    /* We shouldn't get here. If we do then just die */
    daemon_kill();

    return 0;
}
