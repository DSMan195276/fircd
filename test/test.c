/*
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */
#include <stdio.h>

#include "test.h"

static int cur_test_count;
static int total_test_count = 0;
static const char *current_test;
static const char *current_mod;

int run_tests(const char *mod_name, struct unit_test *tests, int test_count)
{
    int i;
    int error_count = 0;
    int errs;
    current_mod = mod_name;

    printf("==== Starting tests for %s ====\n", mod_name);

    for (i = 0; i < test_count; i++) {
        total_test_count++;

        cur_test_count = 0;
        current_test = tests[i].name;

        printf("== #%d: %s ==\n", total_test_count, tests[i].name);

        errs = (tests + i)->test ();

        printf("== Result: ");
        if (errs != 0)
            printf(COLOR_RED "FAIL -> %d" COLOR_RESET, errs);
        else
            printf(COLOR_GREEN "PASS" COLOR_RESET);

        printf(" ==\n");

        error_count += errs;
    }

    printf("==== Finished tests for %s ====\n", mod_name);
    printf("==== Result: ");
    if (error_count == 0)
        printf(COLOR_GREEN "PASS " COLOR_RESET);
    else
        printf(COLOR_RED "FAIL -> %d " COLOR_RESET, error_count);

    printf("====\n");

    return error_count;
}

int assert_true(const char *arg, int line, int cond)
{
    cur_test_count++;
    printf(" [%d:%d] L:%d \"%s\": ", total_test_count, cur_test_count, line, arg);
    if (cond)
        printf(COLOR_GREEN "PASS" COLOR_RESET);
    else
        printf(COLOR_RED "FAIL" COLOR_RESET);

    printf("\n");

    return !cond;
}






