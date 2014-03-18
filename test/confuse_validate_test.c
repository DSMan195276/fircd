/*
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */
#define _BSD_SOURCE
#include "global.h"

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "test.h"
#include "confuse.h"

static int valid_ret = 0;

static cfg_t *cfg = 0;

#define ACTION_NONE 0
#define ACTION_RUN 1
#define ACTION_WALK 2
#define ACTION_CRAWL 3
#define ACTION_JUMP 4

static int parse_action(cfg_t *cfg, cfg_opt_t *opt, const char *value, void *result)
{
    if (strcmp(value, "run") == 0)
        *(int *)result = ACTION_RUN;
    else if (strcmp(value, "walk") == 0)
        *(int *)result = ACTION_WALK;
    else if (strcmp(value, "crawl") == 0)
        *(int *)result = ACTION_CRAWL;
    else if (strcmp(value, "jump") == 0)
        *(int *)result = ACTION_JUMP;
    else
        return -1;

    return 0;
}

int validate_speed(cfg_t *cfg, cfg_opt_t *opt)
{
    unsigned int i;

    for (i = 0; i < cfg_opt_size(opt); i++)
        if (cfg_opt_getnint(opt, i) <= 0)
            return 1;

    return 0;
}

int validate_ip(cfg_t *cfg, cfg_opt_t *opt)
{
    unsigned int i;

    for (i = 0; i < cfg_opt_size(opt); i++) {
        struct in_addr addr;
        char *ip = cfg_opt_getnstr(opt, i);
        if (inet_aton(ip, &addr) == 0)
            return 1;
    }

    return 0;
}

int validate_action(cfg_t *cfg, cfg_opt_t *opt)
{
    cfg_opt_t *name_opt;
    cfg_t *action_sec = cfg_opt_getnsec(opt, 0);

    valid_ret += TEST_ASSERT(action_sec != NULL);

    name_opt = cfg_getopt(action_sec, "name");

    valid_ret += TEST_ASSERT(name_opt != NULL);
    valid_ret += TEST_ASSERT(cfg_opt_size(name_opt) == 1);

    if (cfg_opt_getnstr(name_opt, 0) == NULL)
        return 1;

    return 0;
}

int validate_setup(void)
{
    int ret = 0;
    cfg_opt_t *opt = 0;

    static cfg_opt_t action_opts[] = {
        CFG_INT("speed", 0, CFGF_NONE),
        CFG_STR("name", 0, CFGF_NONE),
        CFG_INT("xspeed", 0, CFGF_NONE),
        CFG_END()
    };

    static cfg_opt_t multi_opts[] = {
        CFG_INT_LIST("speeds", 0, CFGF_NONE),
        CFG_SEC("options", action_opts, CFGF_NONE),
        CFG_END()
    };

    cfg_opt_t opts[] = {
        CFG_STR_LIST("ip-address", 0, CFGF_NONE),
        CFG_INT_CB("action", ACTION_NONE, CFGF_NONE, parse_action),
        CFG_SEC("options", action_opts, CFGF_NONE),
        CFG_SEC("multi_options", multi_opts, CFGF_MULTI),
        CFG_END()
    };

    cfg = cfg_init(opts, 0);

    cfg_set_validate_func(cfg, "ip-address", validate_ip);
    ret += TEST_ASSERT(cfg_set_validate_func(cfg, "ip-address", validate_ip) == validate_ip);
    opt = cfg_getopt(cfg, "ip-address");
    ret += TEST_ASSERT(opt != NULL);
    ret += TEST_ASSERT(opt->validcb == validate_ip);
    
    cfg_set_validate_func(cfg, "options", validate_action);
    ret += TEST_ASSERT(cfg_set_validate_func(cfg, "options", validate_action) == validate_action);
    opt = cfg_getopt(cfg, "options");
    ret += TEST_ASSERT(opt != NULL);
    ret += TEST_ASSERT(opt->validcb == validate_action);

    cfg_set_validate_func(cfg, "options|speed", validate_speed);
    ret += TEST_ASSERT(cfg_set_validate_func(cfg, "options|speed", validate_speed) == validate_speed);
    opt = cfg_getopt(cfg, "options|speed");
    ret += TEST_ASSERT(opt != NULL);
    ret += TEST_ASSERT(opt->validcb == validate_speed);

    cfg_set_validate_func(cfg, "multi_options|speeds", validate_speed);
    ret += TEST_ASSERT(cfg_set_validate_func(cfg, "multi_options|speeds", validate_speed) == validate_speed);
    
    cfg_set_validate_func(cfg, "multi_options|options|xspeed", validate_speed);
    ret += TEST_ASSERT(cfg_set_validate_func(cfg, "multi_options|options|xspeed", validate_speed) == validate_speed);

    return ret; 
}

void validate_teardown(void)
{
    cfg_free(cfg);
}

int validate_test(void)
{
    char *buf;
    unsigned int i;

    buf = "action = wlak";
    valid_ret += TEST_ASSERT(cfg_parse_buf(cfg, buf) == CFG_PARSE_ERROR);

    buf = "action = walk";
    valid_ret += TEST_ASSERT(cfg_parse_buf(cfg, buf) == CFG_SUCCESS);

    buf = "action = run\n"
          "options { speed = 6 }";
    valid_ret += TEST_ASSERT(cfg_parse_buf(cfg, buf) == CFG_PARSE_ERROR);

    buf = "action = jump\n"
          "options { speed = 2, name = 'Joe' }";
    valid_ret += TEST_ASSERT(cfg_parse_buf(cfg, buf) == CFG_PARSE_ERROR);

    buf = "ip-address = { 0.0.0.0 , 1.2.3.4 , 192.168.0.254 , 10.0.0.255 , 20.30.40.256}";
    valid_ret += TEST_ASSERT(cfg_parse_buf(cfg, buf) == CFG_PARSE_ERROR);
    buf = "ip-address = { 0.0.0.0 , 1.2.3.4 , 192.168.0.254 , 10.0.0.255 , 20.30.40.250}";
    valid_ret += TEST_ASSERT(cfg_parse_buf(cfg, buf) == CFG_SUCCESS);
    buf = "ip-address = { 1.2.3. }";
    valid_ret += TEST_ASSERT(cfg_parse_buf(cfg, buf) == CFG_PARSE_ERROR);

    buf = "action = run"
          " multi_options { speeds = {1, 2, 3, 4, 5} }";
    valid_ret += TEST_ASSERT(cfg_parse_buf(cfg, buf) == CFG_SUCCESS);
    for(i = 0; i < cfg_size(cfg, "multi_options"); i++)
    {
        cfg_t *multisec = cfg_getnsec(cfg, "multi_options", i);
        cfg_opt_t *speeds_opt = cfg_getopt(multisec, "speeds");
        valid_ret += TEST_ASSERT(speeds_opt != 0);
        valid_ret += TEST_ASSERT(speeds_opt->validcb == validate_speed);
    }

    buf = "action = run"
          " multi_options { speeds = {1, 2, 3, -4, 5} }";
    valid_ret += TEST_ASSERT(cfg_parse_buf(cfg, buf) == CFG_PARSE_ERROR);

    buf = "action = run"
          " multi_options { speeds = {1, 2, 3, 4, 0} }";
    valid_ret += TEST_ASSERT(cfg_parse_buf(cfg, buf) == CFG_PARSE_ERROR);

    buf = "action = run"
          " multi_options { options { xspeed = 3 } }";
    valid_ret += TEST_ASSERT(cfg_parse_buf(cfg, buf) == CFG_SUCCESS);

    buf = "action = run"
          " multi_options { options { xspeed = -3 } }";
    valid_ret += TEST_ASSERT(cfg_parse_buf(cfg, buf) == CFG_PARSE_ERROR);

    return valid_ret;
}

int main()
{
    int ret;
    struct unit_test tests[] = {
        { validate_setup, "Setup" },
        { validate_test, "Test" },
    };

    ret = run_tests("Confuse Suite Validate", tests, sizeof(tests) / sizeof(tests[0]));

    validate_teardown();

    return ret;
}

