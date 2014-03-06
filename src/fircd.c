/*
 * ./fircd.c -- Provides basic initalization code, as well as functions for
 *              gathering the fd's for select(), and handling a select() by handling input
 *              from ./cmd, and calling the handle_input functions on every network
 *
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */
#include "global.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#include "debug.h"
#include "config.h"
#include "network.h"
#include "irc.h"
#include "buf.h"
#include "fircd.h"

