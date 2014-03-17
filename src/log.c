/*
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */
#include "global.h"

#include <syslog.h>

#include "log.h"

void log_open(const char *name)
{
    openlog(name, LOG_NDELAY | LOG_PID, LOG_DAEMON);
}

void log_close(void)
{
    closelog();
}

