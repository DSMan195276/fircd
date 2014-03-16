/*
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */
#include "global.h"

#include <stdio.h>
#include <string.h>

#include "test.h"
#include "confuse.h"

static cfg_opt_t ab_sec_opts[] = {
    CFG_INT("a", 1, CFGF_NONE),
    CFG_INT("b", 2, CFGF_NONE),
    CFG_END()
};

static cfg_opt_t ab_opts[] = {
    CFG_SEC("sec", ab_sec_opts, CFGF_MULTI | CFGF_TITLE),
    CFG_FUNC("include", &cfg_include),
    CFG_END()
};

int suite_dup(void)
{
    int ret = 0;

    cfg_t *acfg, *bcfg, *sec;

    acfg = cfg_init(ab_opts, 0);
    ret += TEST_ASSERT(cfg_parse(acfg, "test/a.conf") == 0);

    bcfg = cfg_init(ab_opts, 0);
    ret += TEST_ASSERT(cfg_parse(bcfg, "test/b.conf") == 0);

    sec = cfg_getnsec(acfg, "sec", 0);
    ret += TEST_ASSERT(sec != NULL);
    ret += TEST_ASSERT(cfg_size(acfg, "sec") == 1);
    ret += TEST_ASSERT(strcmp(cfg_title(sec), "acfg") == 0);
    ret += TEST_ASSERT(cfg_getint(sec, "a") == 5);
    ret += TEST_ASSERT(cfg_getint(sec, "b") == 2);

    sec = cfg_getnsec(bcfg, "sec", 0);
    ret += TEST_ASSERT(sec != NULL);
    ret += TEST_ASSERT(cfg_size(bcfg, "sec") == 1);
    ret += TEST_ASSERT(strcmp(cfg_title(sec), "bcfg") == 0);
    ret += TEST_ASSERT(cfg_getint(sec, "a") == 1);
    ret += TEST_ASSERT(cfg_getint(sec, "b") == 9);

    cfg_free(acfg);
    cfg_free(bcfg);

    return ret;
}

int main()
{
    int ret;
    struct unit_test tests[] = {
        { suite_dup, "Suite dup" },
    };

    ret = run_tests("confuse suite dup", tests, sizeof(tests) / sizeof(tests[0]));

    return ret;
}


