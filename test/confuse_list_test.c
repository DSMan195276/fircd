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

float absf(float val)
{
    if (val > 0)
        return val;
    else
        return -val;
}

static cfg_t *list_cfg;
static int list_numopts = 0;

static void suppress_errors(cfg_t *cfg, const char *fmt, va_list ap)
{

}

int list_setup(void)
{
    int ret = 0;
    static cfg_opt_t subsec_opts[] = {
        CFG_STR_LIST("subsubstring", 0, CFGF_NONE),
        CFG_INT_LIST("subsubinteger", 0, CFGF_NONE),
        CFG_FLOAT_LIST("subsubfloat", 0, CFGF_NONE),
        CFG_BOOL_LIST("subsubbool", 0, CFGF_NONE),
        CFG_END()
    };

    static cfg_opt_t sec_opts[] = {
        CFG_STR_LIST("substring", "{subdefault1, subdefault2}", CFGF_NONE),
        CFG_INT_LIST("subinteger", "{17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30}", CFGF_NONE),
        CFG_FLOAT_LIST("subfloat", "{8.37}", CFGF_NONE),
        CFG_SEC("subsection", subsec_opts, CFGF_MULTI | CFGF_TITLE),
        CFG_END()
    };

    static cfg_opt_t opts[] = {
        CFG_STR_LIST("string", "{default1, default2, default3}", CFGF_NONE),
        CFG_INT_LIST("integer", "{4711, 123456789}", CFGF_NONE),
        CFG_FLOAT_LIST("float", "{0.42}", CFGF_NONE),
        CFG_BOOL_LIST("bool", "{false, true, no, yes, off, on}", CFGF_NONE),
        CFG_SEC("section", sec_opts, CFGF_MULTI),
        CFG_END()
    };

    list_cfg = cfg_init(opts, 0);
    list_numopts = cfg_numopts(opts);

    ret += TEST_ASSERT(list_numopts == 5);

    memset(opts, 0, sizeof(opts));
    memset(sec_opts, 0, sizeof(sec_opts));
    memset(subsec_opts, 0, sizeof(subsec_opts));

    cfg_set_error_function(list_cfg, suppress_errors);

    return ret;
}

int list_string_test(void)
{
    int ret = 0;
    const char *buf;

    ret += TEST_ASSERT(cfg_size(list_cfg, "string") == 3);
    ret += TEST_ASSERT(cfg_opt_size(cfg_getopt(list_cfg, "string")) == 3);
    ret += TEST_ASSERT(strcmp(cfg_getnstr(list_cfg, "string", 0), "default1") == 0);
    ret += TEST_ASSERT(strcmp(cfg_getnstr(list_cfg, "string", 1), "default2") == 0);
    ret += TEST_ASSERT(strcmp(cfg_getnstr(list_cfg, "string", 2), "default3") == 0);
    buf = "string = {\"manually\", 'set'}";
    ret += TEST_ASSERT(cfg_parse_buf(list_cfg, buf) == CFG_SUCCESS);
    ret += TEST_ASSERT(cfg_size(list_cfg, "string") == 2);
    ret += TEST_ASSERT(strcmp(cfg_getstr(list_cfg, "string"), "manually") == 0);
    ret += TEST_ASSERT(strcmp(cfg_getnstr(list_cfg, "string", 1), "set") == 0);
    cfg_setstr(list_cfg, "string", "manually set");
    ret += TEST_ASSERT(strcmp(cfg_getstr(list_cfg, "string"), "manually set") == 0);
    cfg_setnstr(list_cfg, "string", "foobar", 1);
    ret += TEST_ASSERT(strcmp(cfg_getnstr(list_cfg, "string", 1), "foobar") == 0);

    cfg_addlist(list_cfg, "string", 3, "foo", "bar", "baz");
    ret += TEST_ASSERT(cfg_size(list_cfg, "string") == 5);
    ret += TEST_ASSERT(strcmp(cfg_getnstr(list_cfg, "string", 3), "bar") == 0);

    cfg_setlist(list_cfg, "string", 3, "baz", "foo", "bar");
    ret += TEST_ASSERT(cfg_size(list_cfg, "string") == 3);
    ret += TEST_ASSERT(strcmp(cfg_getnstr(list_cfg, "string", 0), "baz") == 0);

    buf = "string += {gaz, 'onk'}";
    ret += TEST_ASSERT(cfg_parse_buf(list_cfg, buf) == CFG_SUCCESS);
    ret += TEST_ASSERT(cfg_size(list_cfg, "string") == 5);
    ret += TEST_ASSERT(strcmp(cfg_getnstr(list_cfg, "string", 3), "gaz") == 0);
    ret += TEST_ASSERT(strcmp(cfg_getnstr(list_cfg, "string", 4), "onk") == 0);

    return ret;
}

int list_integer_test(void)
{
    int ret = 0;
    const char *buf;

    ret += TEST_ASSERT(cfg_size(list_cfg, "integer") == 2);
    ret += TEST_ASSERT(cfg_opt_size(cfg_getopt(list_cfg, "integer")) == 2);

    ret += TEST_ASSERT(cfg_getint(list_cfg, "integer") == 4711);
    ret += TEST_ASSERT(cfg_getnint(list_cfg, "integer", 1) == 123456789);
    buf = "integer = {-46}";
    ret += TEST_ASSERT(cfg_parse_buf(list_cfg, buf) == CFG_SUCCESS);
    ret += TEST_ASSERT(cfg_size(list_cfg, "integer") == 1);
    ret += TEST_ASSERT(cfg_getnint(list_cfg, "integer", 0) == -46);
    cfg_setnint(list_cfg, "integer", 999999, 1);
    ret += TEST_ASSERT(cfg_size(list_cfg, "integer") == 2);
    ret += TEST_ASSERT(cfg_getnint(list_cfg, "integer", 1) == 999999);

    cfg_addlist(list_cfg, "integer", 3, 11, 22, 33);
    ret += TEST_ASSERT(cfg_size(list_cfg, "integer") == 5);
    ret += TEST_ASSERT(cfg_getnint(list_cfg, "integer", 3) == 22);

    cfg_setlist(list_cfg, "integer", 3, 11, 22, 33);
    ret += TEST_ASSERT(cfg_size(list_cfg, "integer") == 3);
    ret += TEST_ASSERT(cfg_getnint(list_cfg, "integer", 0) == 11);

    buf = "integer += {-1234567890, 1234567890}";
    ret += TEST_ASSERT(cfg_parse_buf(list_cfg, buf) == CFG_SUCCESS);
    ret += TEST_ASSERT(cfg_size(list_cfg, "integer") == 5);
    ret += TEST_ASSERT(cfg_getnint(list_cfg, "integer", 3) == -1234567890);
    ret += TEST_ASSERT(cfg_getnint(list_cfg, "integer", 4) == 1234567890);

    return ret;
}

int list_float_test(void)
{
    int ret = 0;
    const char *buf;

    ret += TEST_ASSERT(cfg_size(list_cfg, "float") == 1);
    ret += TEST_ASSERT(cfg_opt_size(cfg_getopt(list_cfg, "float")) == 1);

    ret += TEST_ASSERT(absf(cfg_getfloat(list_cfg, "float") - 0.42f) < .0001);
    ret += TEST_ASSERT(absf(cfg_getnfloat(list_cfg, "float", 0) - 0.42f) < .0001);

    buf = "float = {-46.777, 0.1, 0.2, 0.17, 17.123}";
    ret += TEST_ASSERT(cfg_parse_buf(list_cfg, buf) == CFG_SUCCESS);
    ret += TEST_ASSERT(absf(cfg_opt_size(cfg_getopt(list_cfg, "float")) - 5) < .0001);
    ret += TEST_ASSERT(absf(cfg_getnfloat(list_cfg, "float", 0) - -46.777f) < .0001);
    ret += TEST_ASSERT(absf(cfg_getnfloat(list_cfg, "float", 1) - 0.1f) < .0001);
    ret += TEST_ASSERT(absf(cfg_getnfloat(list_cfg, "float", 2) - 0.2f) < .0001);
    ret += TEST_ASSERT(absf(cfg_getnfloat(list_cfg, "float", 3) - 0.17f) < .0001);
    ret += TEST_ASSERT(absf(cfg_getnfloat(list_cfg, "float", 4) - 17.123f) < .0001);

    cfg_setnfloat(list_cfg, "float", 5.1234e2, 1);
    ret += TEST_ASSERT(absf(cfg_getnfloat(list_cfg, "float", 1) - 5.1234e2) < .0001);

    cfg_addlist(list_cfg, "float", 1, 11.2233);
    ret += TEST_ASSERT(cfg_size(list_cfg, "float") == 6);
    ret += TEST_ASSERT(absf(cfg_getnfloat(list_cfg, "float", 5) - 11.2233) < .0001);

    cfg_setlist(list_cfg, "float", 2, .3, -18.17e-7);
    ret += TEST_ASSERT(cfg_size(list_cfg, "float") == 2);
    ret += TEST_ASSERT(absf(cfg_getnfloat(list_cfg, "float", 0) - 0.3) < .0001);
    ret += TEST_ASSERT(absf(cfg_getnfloat(list_cfg, "float", 1) - -18.17e-7) < .0001);

    buf = "float += {64.64, 1234.567890}";
    ret += TEST_ASSERT(cfg_parse_buf(list_cfg, buf) == CFG_SUCCESS);
    ret += TEST_ASSERT(cfg_size(list_cfg, "float") == 4);
    ret += TEST_ASSERT(absf(cfg_getnfloat(list_cfg, "float", 2) - 64.64) < .0001);
    ret += TEST_ASSERT(absf(cfg_getnfloat(list_cfg, "float", 3) - 1234.567890) < .0001);

    return ret;
}

int list_bool_test(void)
{
    int ret = 0;
    const char *buf;

    ret += TEST_ASSERT(cfg_size(list_cfg, "bool") == 6);
    ret += TEST_ASSERT(cfg_opt_size(cfg_getopt(list_cfg, "bool")) == 6);
    ret += TEST_ASSERT(cfg_getnbool(list_cfg, "bool", 0) == cfg_false);
    ret += TEST_ASSERT(cfg_getnbool(list_cfg, "bool", 1) == cfg_true);
    ret += TEST_ASSERT(cfg_getnbool(list_cfg, "bool", 2) == cfg_false);
    ret += TEST_ASSERT(cfg_getnbool(list_cfg, "bool", 3) == cfg_true);
    ret += TEST_ASSERT(cfg_getnbool(list_cfg, "bool", 4) == cfg_false);
    ret += TEST_ASSERT(cfg_getnbool(list_cfg, "bool", 5) == cfg_true);

    buf = "bool = {yes, yes, no, false, true}";
    ret += TEST_ASSERT(cfg_parse_buf(list_cfg, buf) == CFG_SUCCESS);
    ret += TEST_ASSERT(cfg_size(list_cfg, "bool") == 5);
    ret += TEST_ASSERT(cfg_getbool(list_cfg, "bool") == cfg_true);
    ret += TEST_ASSERT(cfg_getnbool(list_cfg, "bool", 1) == cfg_true);
    ret += TEST_ASSERT(cfg_getnbool(list_cfg, "bool", 2) == cfg_false);
    ret += TEST_ASSERT(cfg_getnbool(list_cfg, "bool", 3) == cfg_false);
    ret += TEST_ASSERT(cfg_getnbool(list_cfg, "bool", 4) == cfg_true);

    cfg_setbool(list_cfg, "bool", cfg_false);
    ret += TEST_ASSERT(cfg_getbool(list_cfg, "bool") == cfg_false);
    cfg_setnbool(list_cfg, "bool", cfg_false, 1);
    ret += TEST_ASSERT(cfg_getnbool(list_cfg, "bool", 1) == cfg_false);

    cfg_addlist(list_cfg, "bool", 2, cfg_true, cfg_false);
    ret += TEST_ASSERT(cfg_opt_size(cfg_getopt(list_cfg, "bool")) == 7);
    ret += TEST_ASSERT(cfg_getnbool(list_cfg, "bool", 5) == cfg_true);

    cfg_setlist(list_cfg, "bool", 4, cfg_true, cfg_true, cfg_false, cfg_true);
    ret += TEST_ASSERT(cfg_size(list_cfg, "bool") == 4);
    ret += TEST_ASSERT(cfg_getnbool(list_cfg, "bool", 0) == cfg_true);
    ret += TEST_ASSERT(cfg_getnbool(list_cfg, "bool", 1) == cfg_true);
    ret += TEST_ASSERT(cfg_getnbool(list_cfg, "bool", 2) == cfg_false);
    ret += TEST_ASSERT(cfg_getnbool(list_cfg, "bool", 3) == cfg_true);

    buf = "bool += {false, false}";
    ret += TEST_ASSERT(cfg_parse_buf(list_cfg, buf) == CFG_SUCCESS);
    ret += TEST_ASSERT(cfg_size(list_cfg, "bool") == 6);
    ret += TEST_ASSERT(cfg_getnbool(list_cfg, "bool", 4) == cfg_false);
    ret += TEST_ASSERT(cfg_getnbool(list_cfg, "bool", 5) == cfg_false);

    return ret;
}

int list_section_test(void)
{
    int ret = 0;
    const char *buf;
    cfg_t *sec, *subsec;
    cfg_opt_t *opt;

    /* size should be 0 before any section has been parsed. Since the
     * CFGF_MULTI flag is set, there are no default sections.
     */
    ret += TEST_ASSERT(cfg_size(list_cfg, "section") == 0);
    ret += TEST_ASSERT(cfg_opt_size(cfg_getopt(list_cfg, "section")) == 0);
    ret += TEST_ASSERT(cfg_size(list_cfg, "section|subsection") == 0);
    ret += TEST_ASSERT(cfg_opt_size(cfg_getopt(list_cfg, "section|subsection")) == 0);

    buf = "section {}"; /* add a section with default values */
    ret += TEST_ASSERT(cfg_parse_buf(list_cfg, buf) == CFG_SUCCESS);
    ret += TEST_ASSERT(cfg_size(list_cfg, "section") == 1);

    sec = cfg_getsec(list_cfg, "section");
    ret += TEST_ASSERT(sec != 0);
    ret += TEST_ASSERT(strcmp(sec->name, "section") == 0);
    ret += TEST_ASSERT(cfg_title(sec) == 0);

    opt = cfg_getopt(sec, "subsection");
    ret += TEST_ASSERT(opt != 0);
    ret += TEST_ASSERT(cfg_opt_size(opt) == 0);
    ret += TEST_ASSERT(cfg_size(sec, "subsection") == 0);

    ret += TEST_ASSERT(strcmp(cfg_getnstr(sec, "substring", 0), "subdefault1") == 0);
    subsec = cfg_getsec(list_cfg, "section|subsection");
    ret += TEST_ASSERT(subsec == 0);

    buf = "section { subsection 'foo' { subsubfloat = {1.2, 3.4, 5.6} } }";
    ret += TEST_ASSERT(cfg_parse_buf(list_cfg, buf) == CFG_SUCCESS);
    ret += TEST_ASSERT(cfg_size(list_cfg, "section") == 2);

    sec = cfg_getnsec(list_cfg, "section", 1);
    ret += TEST_ASSERT(sec != 0);
    ret += TEST_ASSERT(strcmp(cfg_title(cfg_getnsec(sec, "subsection", 0)), "foo") == 0);
    ret += TEST_ASSERT(cfg_size(sec, "subinteger") == 14);

    subsec = cfg_getsec(sec, "subsection");
    ret += TEST_ASSERT(cfg_size(subsec, "subsubfloat") == 3);
    ret += TEST_ASSERT(cfg_getnfloat(subsec, "subsubfloat", 2) == 5.6);
    ret += TEST_ASSERT(cfg_getnstr(subsec, "subsubstring", 0) == 0);

    sec = cfg_getnsec(list_cfg, "section", 0);
    ret += TEST_ASSERT(sec != 0);
    ret += TEST_ASSERT(cfg_size(sec, "subsection") == 0);
    buf = "subsection 'bar' {}";
    ret += TEST_ASSERT(cfg_parse_buf(sec, buf) == CFG_SUCCESS);
    ret += TEST_ASSERT(cfg_size(sec, "subsection") == 1);
    subsec = cfg_getnsec(sec, "subsection", 0);
    ret += TEST_ASSERT(subsec != 0);
    ret += TEST_ASSERT(strcmp(cfg_title(subsec), "bar") == 0);
    ret += TEST_ASSERT(cfg_getnfloat(subsec, "subsubfloat", 2) == 0);

    buf = "subsection 'baz' {}";
    ret += TEST_ASSERT(cfg_parse_buf(sec, buf) == CFG_SUCCESS);
    ret += TEST_ASSERT(cfg_gettsec(sec, "subsection", "bar") == subsec);
    opt = cfg_getopt(sec, "subsection");
    ret += TEST_ASSERT(opt != 0);
    ret += TEST_ASSERT(cfg_gettsec(sec, "subsection", "baz") == cfg_opt_gettsec(opt, "baz"));
    ret += TEST_ASSERT(cfg_opt_gettsec(opt, "bar") == subsec);
    ret += TEST_ASSERT(cfg_opt_gettsec(opt, "foo") == 0);
    ret += TEST_ASSERT(cfg_gettsec(sec, "subsection", "section") == 0);

    ret += TEST_ASSERT(cfg_gettsec(list_cfg, "section", "baz") == 0);

    return ret;
}

int parse_buf_test(void)
{
    int ret = 0;
    char *buf;

    ret += TEST_ASSERT(cfg_parse_buf(list_cfg, 0) == CFG_SUCCESS);
    ret += TEST_ASSERT(cfg_parse_buf(list_cfg, "") == CFG_SUCCESS);

    buf = "bool = {true, true, false, wrong}";
    ret += TEST_ASSERT(cfg_parse_buf(list_cfg, buf) == CFG_PARSE_ERROR);
    buf = "string = {foo, bar";
    ret += TEST_ASSERT(cfg_parse_buf(list_cfg, buf) == CFG_PARSE_ERROR);

    buf = "/* this is a comment */ bool = {true} /*// another comment */";
    ret += TEST_ASSERT(cfg_parse_buf(list_cfg, buf) == CFG_SUCCESS);

    buf = "/*/*/ bool = {true}//  */";
    ret += TEST_ASSERT(cfg_parse_buf(list_cfg, buf) == CFG_SUCCESS);

    buf = "/////// this is a comment\nbool = {true} // another /* comment */";
    ret += TEST_ASSERT(cfg_parse_buf(list_cfg, buf) == CFG_SUCCESS);

    buf = "# this is a comment\nbool = {true} # another //* comment *//";
    ret += TEST_ASSERT(cfg_parse_buf(list_cfg, buf) == CFG_SUCCESS);

    buf = "string={/usr/local/}";
    ret += TEST_ASSERT(cfg_parse_buf(list_cfg, buf) == CFG_SUCCESS);
    ret += TEST_ASSERT(strcmp(cfg_getnstr(list_cfg, "string", 0), "/usr/local/") == 0);

    return ret;
}

int nonexistent_option_test(void)
{
    int ret = 0;
    char *buf;

    ret = TEST_ASSERT(cfg_numopts(list_cfg->opts) == list_numopts);
    ret = TEST_ASSERT(cfg_getopt(list_cfg, "nonexistent") == 0);

    buf = "section {}";
    ret = TEST_ASSERT(cfg_parse_buf(list_cfg, buf) == CFG_SUCCESS);
    ret = TEST_ASSERT(cfg_getopt(list_cfg, "section|subnonexistent") == 0);

    return ret;
}

int main()
{
    int ret;
    struct unit_test tests[] = {
        { list_setup, "Setup" },
        { list_string_test, "String test" },
        { list_integer_test, "Integer test" },
        { list_float_test, "Float test" },
        { list_bool_test, "Bool test" },
        { list_section_test, "Section test" },
        { parse_buf_test, "Parse buf test" },
        { nonexistent_option_test, "Nonexistent option test" },
    };

    ret = run_tests("confuse suite list", tests, sizeof(tests) / sizeof(tests[0]));

    cfg_free(list_cfg);

    return ret;
}

