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
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "test.h"
#include "confuse.h"

cfg_opt_t ab_sec_opts[] = {
    CFG_INT("a", 1, CFGF_NONE),
    CFG_INT("b", 2, CFGF_NONE),
    CFG_END()
};

cfg_opt_t ab_opts[] = {
    CFG_SEC("sec", ab_sec_opts, CFGF_MULTI | CFGF_TITLE),
    CFG_FUNC("include", &cfg_include),
    CFG_END()
};

int include_test(void)
{
    int ret = 0;
    char *buf = "include (\"./test/a.conf\")\n";
    cfg_t *cfg = cfg_init(ab_opts, CFGF_NONE);

    ret += TEST_ASSERT(cfg);
    ret += TEST_ASSERT(cfg_parse_buf(cfg, buf) == CFG_SUCCESS);

    ret += TEST_ASSERT(cfg_size(cfg, "sec") == 1);
    ret += TEST_ASSERT(cfg_getint(cfg, "sec|a") == 5);
    ret += TEST_ASSERT(cfg_getint(cfg, "sec|b") == 2);

    cfg_free(cfg);

    return ret;
}

int list_and_syntax(void)
{
    int ret = 0;
    cfg_opt_t opts[] = {
        CFG_STR_LIST("stringproperty", 0, CFGF_NONE),
        CFG_END()
    };

    int rc;
    cfg_t *cfg = cfg_init(opts, CFGF_NONE);
    
    ret += TEST_ASSERT(cfg);

    rc = cfg_parse_buf(cfg,
            " stringproperty = {\"this\"}\n"
            " stringproperty += {\"that\"}\n"
            " stringproperty += {\"other\"}\n");

    ret += TEST_ASSERT(rc == CFG_SUCCESS);

    ret += TEST_ASSERT(cfg_size(cfg, "stringproperty") == 3);

    ret += TEST_ASSERT(strcmp(cfg_getnstr(cfg, "stringproperty", 0), "this") == 0);
    ret += TEST_ASSERT(strcmp(cfg_getnstr(cfg, "stringproperty", 1), "that") == 0);
    ret += TEST_ASSERT(strcmp(cfg_getnstr(cfg, "stringproperty", 2), "other") == 0);

    rc = cfg_parse_buf(cfg,
            " stringproperty = \"this\"\n"
            " stringproperty += \"that\"\n"
            " stringproperty += \"other\"\n");

    ret += TEST_ASSERT(rc == CFG_SUCCESS);

    ret += TEST_ASSERT(strcmp(cfg_getnstr(cfg, "stringproperty", 0), "this") == 0);
    ret += TEST_ASSERT(strcmp(cfg_getnstr(cfg, "stringproperty", 1), "that") == 0);
    ret += TEST_ASSERT(strcmp(cfg_getnstr(cfg, "stringproperty", 2), "other") == 0);

    cfg_free(cfg);

    return ret;
}

int quote_before_print(void)
{
    cfg_opt_t opts[] = {
        CFG_STR("parameter", NULL, CFGF_NONE),
        CFG_END()
    };

    int ret = 0;

    FILE *fp;
    char *param;
    cfg_t *cfg = cfg_init(opts, CFGF_NONE);

    ret += TEST_ASSERT(cfg);

    cfg_setstr(cfg, "parameter", "text \" with quotes and \\");

    fp = tmpfile();
    ret += TEST_ASSERT(fp);
    cfg_print(cfg, fp);
    cfg_free(cfg);

    rewind(fp);
    cfg = cfg_init(opts, CFGF_NONE);
    ret += TEST_ASSERT(cfg);
    ret += TEST_ASSERT(cfg_parse_fp(cfg, fp) == CFG_SUCCESS);
    ret += TEST_ASSERT(fclose(fp) == 0);

    param = cfg_getstr(cfg, "parameter");
    ret += TEST_ASSERT(param);

    ret += TEST_ASSERT(strcmp(param, "text \" with quotes and \\") == 0);
    cfg_free(cfg);

    return ret;
}

int searchpath(void)
{
    int ret = 0;
    const char spdir[] = "./test/spdir";
    const char nodir[] = "./test/no-such-directory";

    cfg_t *cfg;
    cfg_t *sec;

    cfg_opt_t sec_opts[] = {
        CFG_FUNC("include", cfg_include),
        CFG_INT("val", 0, CFGF_NONE),
        CFG_END()
    };

    cfg_opt_t opts[] = {
        CFG_FUNC("include", cfg_include),
        CFG_SEC("sec", sec_opts, CFGF_MULTI | CFGF_TITLE),
        CFG_END()
    };

    cfg = cfg_init(opts, 0);

    ret += TEST_ASSERT(cfg_add_searchpath(cfg, nodir) == 0);
    ret += TEST_ASSERT(cfg_add_searchpath(cfg, spdir) == 0);
    ret += TEST_ASSERT(cfg_add_searchpath(cfg, nodir) == 0);

    ret += TEST_ASSERT(cfg_parse(cfg, "spa.conf") == 0);

    ret += TEST_ASSERT(cfg_size(cfg, "sec") == 3);

    sec = cfg_getnsec(cfg, "sec", 0);
    ret += TEST_ASSERT(sec != NULL);
    ret += TEST_ASSERT(strcmp(cfg_title(sec), "acfg") == 0);
    ret += TEST_ASSERT(cfg_getint(sec, "val") == 5);

    sec = cfg_getnsec(cfg, "sec", 1);
    ret += TEST_ASSERT(sec != NULL);
    ret += TEST_ASSERT(strcmp(cfg_title(sec), "bcfg") == 0);
    ret += TEST_ASSERT(cfg_getint(sec, "val") == 6);

    sec = cfg_getnsec(cfg, "sec", 2);
    ret += TEST_ASSERT(sec != NULL);
    ret += TEST_ASSERT(strcmp(cfg_title(sec), "ccfg") == 0);
    ret += TEST_ASSERT(cfg_getint(sec, "val") == 7);

    cfg_free(cfg);

    return ret;
}

int section_title_dupes(void)
{
    int ret = 0;

    cfg_opt_t section_opts[] = {
        CFG_STR("prop", 0, CFGF_NONE),
        CFG_END()
    };

    cfg_opt_t opts[] = {
        CFG_SEC("section", section_opts, CFGF_TITLE | CFGF_MULTI),
        CFG_END()
    };

    cfg_opt_t opts_no_dupes[] = {
        CFG_SEC("section", section_opts, CFGF_TITLE | CFGF_MULTI | CFGF_NO_TITLE_DUPES),
        CFG_END()
    };

    const char *config_data =
        "section title_one { prop = 'value_one' }\n"
        "section title_two { prop = 'value_two' }\n"
        "section title_one { prop = 'value_one' }\n"
    ;

    int rc;
    cfg_t *cfg = cfg_init(opts, CFGF_NONE);
    ret += TEST_ASSERT(cfg);

    rc = cfg_parse_buf(cfg, config_data);

    ret += TEST_ASSERT(rc == CFG_SUCCESS);
    ret += TEST_ASSERT(cfg_size(cfg, "section") == 2);

    ret += TEST_ASSERT(strcmp(cfg_title(cfg_getnsec(cfg, "section", 0)), "title_one") == 0);
    ret += TEST_ASSERT(strcmp(cfg_title(cfg_getnsec(cfg, "section", 1)), "title_two") == 0);

    cfg_free(cfg);

    cfg = cfg_init(opts_no_dupes, CFGF_NONE);
    ret += TEST_ASSERT(cfg);

    rc = cfg_parse_buf(cfg, config_data);
    ret += TEST_ASSERT(rc == CFG_PARSE_ERROR);

    cfg_free(cfg);

    return ret;
}

int main()
{
    int ret;
    struct unit_test tests[] = {
        { include_test, "Include test" },
        { list_and_syntax, "Lists and Syntax" },
        { quote_before_print, "Quote before print" },
        { searchpath, "Search path" },
        { section_title_dupes, "Section title dupes" },
    };

    ret = run_tests("confuse", tests, sizeof(tests) / sizeof(tests[0]));

    return ret;
}

