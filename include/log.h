/*
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */
#ifndef INCLUDE_LOG_H
#define INCLUDE_LOG_H

#include <stdlib.h>
#include <stdarg.h>
#include <syslog.h>

void log_open(const char *name);
void log_close(void);

#define log_debug(...) syslog(LOG_DEBUG, __VA_ARGS__)
#define log_info(...) syslog(LOG_INFO, __VA_ARGS__)
#define log_notice(...) syslog(LOG_NOTICE, __VA_ARGS__)
#define log_warn(...) syslog(LOG_WARNING, __VA_ARGS__)
#define log_err(...) syslog(LOG_ERR, __VA_ARGS__)

#define log_and_die(...) \
    do { \
        log_debug(__VA_ARGS__); \
        _Exit(0); \
    } while (0)

#endif
