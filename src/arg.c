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
#include "arg.h"

/* 
 * help (noarg)
 * server (arg)
 * password (arg)
 * join (arg) 
 * config-file (arg)
 * version (noarg)
 */
static const char shortopts[] = "hs:n:p:j:f:v";

/* These are the getopt return values for various long-options. They match the
 * short-option if there is one, else they are simply a unique number greater
 * then 255 (Greater then any char value) */
#define OPT_HELP     'h'
#define OPT_SERVER   's'
#define OPT_SERVER_NAME 260
#define OPT_PORT     256
#define OPT_NICKNAME 'n'
#define OPT_PASSWORD 'p'
#define OPT_JOIN     'j'
#define OPT_CFG      'f'
#define OPT_VERSION  'v'
#define OPT_NO_CFG   257
#define OPT_FOR      258
#define OPT_REAL     259

/* The definition of the long-options. Every option has a long-option, not all
 * long-options have a short-option. */
static const struct option longopts[] = {
    { "help",        no_argument,       NULL, OPT_HELP },
    { "server",      required_argument, NULL, OPT_SERVER },
    { "server-name", required_argument, NULL, OPT_SERVER_NAME },
    { "port",        required_argument, NULL, OPT_PORT },
    { "nickname",    required_argument, NULL, OPT_NICKNAME },
    { "password",    required_argument, NULL, OPT_PASSWORD },
    { "join",        required_argument, NULL, OPT_JOIN },
    { "config-file", required_argument, NULL, OPT_CFG },
    { "version",     no_argument,       NULL, OPT_VERSION },
    { "no-config",   no_argument,       NULL, OPT_NO_CFG },
    { "forground",   no_argument,       NULL, OPT_FOR },
    { "realname",    required_argument, NULL, OPT_REAL },
    {0}
};

/* help-text to display when using '-h' or '--help' */
static const char help_text[] =
    "Usage: %s [Flags] \n"
    "\n"
    "Flags:\n"
    "  -h, --help\n"
    "  -s, --server <server address>\n"
    "          --port <port number>\n"
    "      -n, --nickname <nick name>\n"
    "          --realname <real name>\n"
    "      -p, --password <password>\n"
    "      -j, --join <channel>\n"
    "  -f, --config-file <filename>\n"
    "  -v, --version\n"
    "      --no-config\n"
    "      --forground\n"
    "See the manpage for more information\n"
;

/* Version output */
static const char version_text[] =
    "fircd-" Q(FIRCD_VERSION_N) "\n"
    "\n"
    "Copyright (C) 2013 Matt Kilgore\n"
    "This is free software; you are free to change and redistribute it.\n"
    "There is NO WARRENTY, to the extent permitted by low\n"
;

/* This code actually implements how to handle each operation. It's a bit bulky
 * but very simple. */
void parse_cmd_args (int argc, char **argv)
{
    struct network *cur_net = NULL;
    int opt;
    int long_index = 0;

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
        case OPT_SERVER:
            cur_net = malloc(sizeof(struct network));
            network_init(cur_net);

            cur_net->name = strdup(optarg);
            cur_net->url  = strdup(optarg);
            cur_net->next = current_state->head;
            current_state->head = cur_net;
            break;
        case OPT_NO_CFG:
            current_state->conf.no_config = 1;
            break;
        case OPT_FOR:
            current_state->conf.stay_in_forground = 1;
            break;
        case OPT_CFG:
            current_state->config_file = optarg;
            break;
        case '?':
            /* Error message already printed by getopt_long() */
            exit(0);
            break;
        default:
            break;
        }

        if (cur_net == NULL)
            goto no_network;

        switch (opt) {
        case OPT_PORT:
            cur_net->portno = atoi(optarg);
            break;
        case OPT_NICKNAME:
            if (cur_net->nickname)
                free(cur_net->nickname);
            cur_net->nickname = strdup(optarg);
            break;
        case OPT_PASSWORD:
            if (cur_net->password)
                free(cur_net->password);
            cur_net->password = strdup(optarg);
            break;
        case OPT_REAL:
            if (cur_net->realname)
                free(cur_net->realname);
            cur_net->realname = strdup(optarg);
            break;
        case OPT_JOIN:
            network_add_channel(cur_net, optarg);
            break;
        case OPT_SERVER_NAME:
            if (cur_net->name)
                free(cur_net->name);
            cur_net->name = strdup(optarg);
            break;
        default:
            break;
        }
    }
    return;

no_network:
    printf("%s: Please specify a network before using ", argv[0]);
    if (long_index)
        printf("'--%s'\n", longopts[long_index].name);
    else
        printf("'%c'\n", opt);
    exit(0);
    
}
