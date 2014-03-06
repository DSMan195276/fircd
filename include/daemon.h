/*
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */
#ifndef INCLUDE_DAEMON_H
#define INCLUDE_DAEMON_H

#include "global.h"

#include "net_cons.h"

extern void daemon_init(void);
extern void daemon_kill(void);
extern void daemon_main_loop(struct network_cons *con);

#endif
