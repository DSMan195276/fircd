/*
 * ./config.c -- Handles reading and writing the fircd configuration file
 *
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#include "global.h"

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <confuse.h>

#include "debug.h"
#include "network.h"
#include "config.h"

static const char default_config_file[] = "~/.fircdrc";

static int login_type_callback(cfg_t *cfg, cfg_opt_t *opt, const char *value, void *result);

static cfg_opt_t network_opts[] = {
    CFG_STR      ("server",                NULL,       CFGF_NODEFAULT),
    CFG_INT      ("port",                  6667,       CFGF_NONE),
    CFG_BOOL     ("remove-files-on-close",   -1,  CFGF_NONE),
    CFG_STR      ("nickname",              NULL,       CFGF_NODEFAULT),
    CFG_STR      ("realname",              NULL,       CFGF_NONE),
    CFG_STR      ("password",              NULL,       CFGF_NONE),
    CFG_INT_CB   ("login-type",            LOGIN_NONE, CFGF_NONE, login_type_callback),
    CFG_STR_LIST ("channels",              NULL,       CFGF_NONE),
    CFG_END()
};

static cfg_opt_t main_opts[] = {
    CFG_SEC      ("network",               network_opts, CFGF_MULTI | CFGF_TITLE | CFGF_NO_TITLE_DUPES),
    CFG_BOOL     ("stay-in-forground",     cfg_false,    CFGF_NONE),
    CFG_BOOL     ("remove-files-on-close", cfg_false,    CFGF_NONE),
    CFG_STR_LIST ("auto-login",            NULL,         CFGF_NONE),
    CFG_STR      ("root-directory",        "/tmp/irc",   CFGF_NONE),
    CFG_END()
};

static int strcasecmp(const char *s1, const char *s2)
{
    for (; *s1 && *s2; s1++, s2++)
        if ((*s1 | 32) != (*s2 | 32))
            return 1;

    if (*s1 || *s2)
        return 1;

    return 0;
}

static int login_type_callback(cfg_t *cfg, cfg_opt_t *opt, const char *value, void *result)
{
    if (strcasecmp(value, "nickserv") == 0) {
        *(int *)result = LOGIN_NICKSERV;
    } else if (strcasecmp(value, "sasl") == 0) {
        *(int *)result = LOGIN_SASL;
    } else if (strcasecmp(value, "none") == 0) {
        *(int *)result = LOGIN_NONE;
    } else {
        cfg_error(cfg, "Invalid value for option '%s': %s", cfg_opt_name(opt), value);
        return -1;
    }
    return 0;
}

/* Returns 0 if file could be opened and parsed, or no file was specified and
 * the default file didn't exist. Returns 1 if the file was specified and it
 * didn't exist. */
int config_file_parse(void)
{
    int no_conf = 0, ret;
    const char *filename;

    if (current_state->config_file == NULL) {
        no_conf = 1;
        filename = default_config_file;
    } else {
        filename = current_state->config_file;
    }

    ret = config_read(&(current_state->conf), filename);
    if (ret == 2)
        return !no_conf;
    else
        return  ret;
}

static char *sstrdup(const char *s)
{
    if (s != NULL)
        return strdup(s);
    else
        return NULL;
}

static void add_network(struct config *conf, cfg_t *network)
{
    int i;
    struct network *net = malloc(sizeof(struct network));

    network_init(net);
    net->name = sstrdup(cfg_title(network));
    net->url = sstrdup(cfg_getstr(network, "server"));
    net->portno = cfg_getint(network, "port");
    if ((int)cfg_getbool(network, "remove-files-on-close") != -1)
        net->remove_files_on_close = cfg_getbool(network, "remove-files-on-close");
    else
        net->remove_files_on_close = current_state->conf.remove_files_on_close;

    net->nickname = sstrdup(cfg_getstr(network, "nickname"));
    net->realname = sstrdup(cfg_getstr(network, "realname"));
    net->password = sstrdup(cfg_getstr(network, "password"));
    net->login_type = cfg_getint(network, "login-type");

    for (i = 0; i < cfg_size(network, "channels"); i++)
        network_add_channel(net, cfg_getnstr(network, "channels", i));

    net->next = conf->first;
    conf->first = net;
}

int config_read(struct config *conf, const char *filename)
{
    int i;
    int ret, size;
    cfg_t *cfg;

    cfg = cfg_init(main_opts, CFGF_NONE);

    if (!current_state->no_config) {
        switch(cfg_parse(cfg, filename)) {
        case CFG_FILE_ERROR:
            ret = 2;
            goto cleanup;
        case CFG_PARSE_ERROR:
            printf("Error reading configuration file '%s': %s\n", filename, strerror(errno));
            ret = 1;
            goto cleanup;
        case CFG_SUCCESS:
            break;
        }
    }

    if (cfg) {
        if (current_state->stay_in_forground == 0)
            conf->stay_in_forground = cfg_getbool(cfg, "stay-in-forground");
        else
            conf->stay_in_forground = 1;

        conf->remove_files_on_close = cfg_getbool(cfg, "remove-files-on-close");

        if (!current_state->dir)
            conf->root_directory = strdup(cfg_getstr(cfg, "root-directory"));
        else
            conf->root_directory = strdup(current_state->dir);

        size = cfg_size(cfg, "network");
        for (i = 0; i < size; i++)
            add_network(conf, cfg_getnsec(cfg, "network", i));

        size = cfg_size(cfg, "auto-login");
        if (size > 0) {
            DEBUG_PRINT("Array size: %d", size);
            ARRAY_RESIZE(conf->auto_login, size);
            for (i = 0; i < size; i++)
                conf->auto_login.arr[i] = strdup(cfg_getnstr(cfg, "auto-login", i));
        }

        ret = 0;
    } else {
        ret = 1;
    }

cleanup:
    cfg_free(cfg);
    return ret;
}

void config_clear(struct config *conf)
{
    int i;
    network_clear_all(conf->first);
    free(conf->root_directory);
    ARRAY_FOREACH(conf->auto_login, i)
        free(conf->auto_login.arr[i]);
    ARRAY_FREE(conf->auto_login);
}

