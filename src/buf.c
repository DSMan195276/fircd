/*
 * ./buf.c -- Implements a resizeable buffer around a file descriptor
 *
 * It is designed to be very memory efficent and fast, reading chunks of data
 * at a time instead of reading one byte at a time. The buffer of read data is
 * stored in a linked-list of allocated blocks, where each block holds a
 * certain number of bytes.
 *
 * The buffer keeps track of which part of the block has yet to be read, which
 * part has yet to be used, etc. It also optionally keeps an extra list of
 * unused but already allocated blocks, to be used in the event more storage is
 * needed. Increasing this count increases the total ammount of memory the
 * buffer will be using even when it's not in use, but it decreases the overall
 * number of malloc() calls. The default is to have a block size of 200 bytes
 * and an to hold a max of 4 empty blocks. Thus, when it's not in use the
 * buffer will be using up around 1kB of memory (Assuming it gets to the point
 * where it needs that many free blocks)
 *
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#include "global.h"

#include <stdlib.h>
#include <errno.h>

#include "debug.h"
#include "buf.h"

void buf_init(struct buf_fd *buf)
{
    memset(buf, 0, sizeof(struct buf_fd));
    buf->block_size = 200;
    buf->max_empty = 4;
    buf->fd = -1;
}

void buf_free(struct buf_fd *buf)
{
    struct buf_blk *current, *tmp;
    for (current = buf->head; current != NULL; current = tmp) {
        tmp = current->next_blk;
        free(current);
    }
    for (current = buf->empty_head; current != NULL; current = tmp) {
        tmp = current->next_blk;
        free(current);
    }
}

static struct buf_blk *new_buf_block(struct buf_fd *buf)
{
    struct buf_blk *new;
    if (buf->empty_head == NULL) {
        new = malloc(sizeof(struct buf_blk) + buf->block_size);
        memset(new, 0, sizeof(struct buf_blk) + buf->block_size);
        new->allocsize = buf->block_size;
    } else {
        new = buf->empty_head;
        buf->empty_head = buf->empty_head->next_blk;
        new->next_blk = NULL;
        new->offset = 0;
        new->size = 0;
        buf->empty_count--;
    }
    return new;
}

void buf_handle_input(struct buf_fd *buf)
{
    size_t size = 200, read_size, tmp, offset;
    struct buf_blk *cur_block, *tmp_blk;
    char tmpbuf[size];

    int i;

    buf->errno_ret = 0;
    buf->closed_gracefully = 0;
    errno = 0;

    while ((read_size = read(buf->fd, tmpbuf, size)) != -1) {
        if (read_size == 0) {
            buf->errno_ret = errno;
            buf->closed_gracefully = 1;
            break;
        }

        for (i = 0; i < read_size; i++)
            if (tmpbuf[i] == '\n')
                buf->has_line += 1;

        cur_block = buf->tail;

        offset = 0;

        while (read_size - offset != 0) {
            tmp = read_size - offset;
            if (cur_block == NULL)
                tmp = 0;
            else if (tmp > BUF_BLK_UNUSED(cur_block))
                tmp = BUF_BLK_UNUSED(cur_block);

            if (tmp == 0) {
                tmp_blk = new_buf_block(buf);
                if (buf->tail == NULL) {
                    buf->head = buf->tail = tmp_blk;
                } else {
                    buf->tail->next_blk = tmp_blk;
                    buf->tail = tmp_blk;
                }
                cur_block = tmp_blk;
                continue; /* Loop again so 'tmp' is recalculated */
            }

            memcpy(cur_block->buf + cur_block->size, tmpbuf + offset, tmp);

            (cur_block)->size += tmp;

            offset += tmp;
        }
    }
}

char *buf_read_line(struct buf_fd *buf)
{
    size_t size = 0;
    char *ret, *curstr;
    int i, found_newline = 0;
    struct buf_blk *cur_block, *tmp;

    if (buf->has_line == 0)
        return NULL;

    for (cur_block = buf->head; cur_block != NULL && found_newline == 0; cur_block = cur_block->next_blk) {
        for (i = cur_block->offset; i < cur_block->size; i++) {
            if (cur_block->buf[i] == '\n') {
                found_newline = 1;
                break;
            }
        }
        size += i - cur_block->offset;
    }

    ret = malloc(size + 1);

    curstr = ret;

    found_newline = 0;

    for (cur_block = buf->head; cur_block != NULL && found_newline == 0; cur_block = tmp) {
        tmp = cur_block->next_blk;
        for (i = cur_block->offset; i < cur_block->size; i++) {
            if (cur_block->buf[i] == '\n') {
                found_newline = 1;
                break;
            }
        }
        memcpy(curstr, cur_block->buf + cur_block->offset, i - cur_block->offset);
        curstr += i - cur_block->offset;
        cur_block->offset = i + 1;
        if (cur_block->size == (cur_block->offset - 1) && cur_block->size == cur_block->allocsize) {
            buf->head = cur_block->next_blk;
            if (buf->head == NULL)
                buf->tail = NULL;

            if (buf->empty_count < buf->max_empty) {
                cur_block->next_blk = buf->empty_head;
                buf->empty_head = cur_block;
                buf->empty_count++;
            } else {
                free(cur_block);
            }
        }
    }

    ret[size] = '\0';

    if (ret[size - 1] == '\r')
        ret[size - 1] = '\0';

    buf->has_line--;

    return ret;
}

