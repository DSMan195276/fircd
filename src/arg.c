/*
 * ./arg.c -- parses command-line arguments for fircd
 *
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#include "global.h"

#include <stdio.h>
#include <getopt.h>

#include "debug.h"
#include "fircd.h"
#include "net_cons.h"
#include "arg.h"

/*
 * help (noarg)
 * config-file (arg)
 * version (noarg)
 * forground (noarg)
 * network (arg)
 * dont-auto-login (noarg)
 */
static const char shortopts[] = "hc:vrfn:d";

/* These are the getopt return values for various long-options. They match the
 * short-option if there is one, else they are simply a unique number greater
 * then 255 (Greater then any char value) */
#define OPT_HELP     'h'
#define OPT_CFG      'c'
#define OPT_VERSION  'v'
#define OPT_NO_CFG   'r'
#define OPT_FOR      'f'
#define OPT_NET      'n'
#define OPT_DONT     'd'

/* The definition of the long-options. Every option has a long-option, not all
 * long-options have a short-option. */
static const struct option longopts[] = {
    { "help",            no_argument,       NULL, OPT_HELP },
    { "config-file",     required_argument, NULL, OPT_CFG },
    { "version",         no_argument,       NULL, OPT_VERSION },
    { "no-config",       no_argument,       NULL, OPT_NO_CFG },
    { "forground",       no_argument,       NULL, OPT_FOR },
    { "network",         required_argument, NULL, OPT_NET },
    { "dont-auto-login", no_argument,       NULL, OPT_DONT },
    {0}
};

/* help-text to display when using '-h' or '--help' */
static const char help_text[] =
    "Usage: %s [Flags] \n"
    "\n"
    "Flags:\n"
    "  -h, --help\n"
    "  -v, --version\n"
    "  -c, --config-file <filename>\n"
    "  -r, --no-config\n"
    "  -f, --forground\n"
    "  -n, --network <network name>\n"
    "  -d, --dont-auto-login\n"
    "See the manpage for more information\n"
;

/* Version output */
static const char version_text[] =
    "fircd-" Q(FIRCD_VERSION_N) "\n"
    "\n"
    "Copyright (C) 2013 Matt Kilgore\n"
    "This is free software; you are free to change and redistribute it.\n"
    "There is NO WARRENTY, to the extent permitted by law\n"
;

/* This code actually implements how to handle each operation. It's a bit bulky
 * but very simple. */
void parse_cmd_args (int argc, char **argv, struct network_cons *con)
{
    int opt;
    int long_index = 0;
    int size;

    while ((opt = getopt_long(argc, argv, shortopts, longopts, &long_index)) != -1) {
        DEBUG_PRINT("Argument: %d", opt);

        switch (opt) {
        case OPT_HELP:
            printf(help_text, argv[0]);
            exit(0);
            break;
        case OPT_VERSION:
            printf(version_text);
            exit(0);
            break;
        case OPT_NO_CFG:
            con->no_config = 1;
            break;
        case OPT_FOR:
            con->stay_in_forground = 1;
            break;
        case OPT_CFG:
            con->config_file = strdup(optarg);
            break;
        case OPT_DONT:
            con->dont_auto_load = 1;
            break;
        case OPT_NET:
            size = ARRAY_SIZE(con->conf.auto_login);
            ARRAY_RESIZE(con->conf.auto_login, size + 1);
            con->conf.auto_login.arr[size] = strdup(optarg);
            DEBUG_PRINT("Auto-starting network %s", optarg);
            break;
        default:
        case '?':
            /* Error message already printed by getopt_long() */
            exit(0);
            break;
        }
    }
    return;
}

