/*
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */
#ifndef INCLUDE_BUF_H
#define INCLUDE_BUF_H

#include "global.h"

#include <unistd.h> /* close() */
#include <sys/types.h>
#include <sys/select.h>

#include "array.h"

#define BUF_BLK_UNUSED(blk) ((blk)->allocsize - (blk)->size)

struct buf_blk {
    struct buf_blk *next_blk;

    /* allocsize is total size of block
     * size      is the number (starting from 0) of bytes currently in use
     * offset    is the number of bytes at the beginning not in used
     *
     * (size - offset) is the number of bytes contained in this block
     */
    size_t allocsize, size, offset;
    char buf[];
};

struct buf_fd {
    struct buf_blk *head;
    struct buf_blk *tail;
    struct buf_blk *empty_head;
    int fd;
    unsigned int empty_count;
    unsigned int block_size;
    unsigned int max_empty;
    unsigned short has_line;

    unsigned int errno_ret;
    unsigned int closed_gracefully :1;
};

#define CLOSE_FD_BUF(buf_fd) \
    do { \
        if ((buf_fd).fd != -1) \
            close((buf_fd).fd); \
        buf_free(&(buf_fd)); \
    } while (0)

#define CLOSE_FD(fd) \
    do { \
        if ((fd) != -1) \
            close(fd); \
    } while (0)

#define ADD_FD_CUR_STATE(fd) \
    do { \
        if ((fd) != -1) \
            FD_SET((fd), &current_state->infd); \
        if ((fd) > (current_state->maxfd)) \
            (current_state->maxfd) = (fd); \
    } while (0)

#define CLR_FD_CUR_STATE(fd) \
    do { \
        FD_CLR((fd), &current_state->infd); \
    } while (0)

#define ADD_FD_BUF(buf_fd, in, maxfd) \
    do { \
        if ((buf_fd).fd != -1) \
            FD_SET((buf_fd).fd, in); \
        if ((buf_fd).fd > (maxfd)) \
            (maxfd) = (buf_fd).fd; \
    } while (0)

#define OPEN_FILE(ptr, name) ptr->name##fd = open(#name , O_WRONLY | O_CREAT | O_APPEND | O_NONBLOCK, 0750)

#define OPEN_FIFO(ptr, name) \
    do { \
        mkfifo(#name, 0772); \
        ptr->name ## fd.fd = open(#name , O_RDWR | O_NONBLOCK, 0); \
    } while (0)

extern void buf_init(struct buf_fd *);

extern void buf_free(struct buf_fd *);

extern void buf_handle_input(struct buf_fd *);

extern char *buf_read_line(struct buf_fd *);

#endif
