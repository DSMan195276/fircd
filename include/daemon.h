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

extern void daemon_init(struct network_cons *con);
extern void daemon_kill(struct network_cons *con);

#endif
