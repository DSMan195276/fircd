/*
 * rbtree.h - Implementation of generic red-black tree data structure
 *
 * This code is heavily based off of the rb-tree implementation in the Linux
 * kernel.
 *
 * This also includes a example 'rbnode_char' which shows example usage of the
 * rbnode struct (Embedding it into another structure, and then using
 * container_of to get it in the callbacks)
 *
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */
#ifndef INCLUDE_RBTREE_H
#define INCLUDE_RBTREE_H

#include <stdint.h>

#define RB_RED   0
#define RB_BLACK 1

/*
 * This is the main structure for a rb-tree. This is meant to be used in
 * conjunction with container_of to embed it into an eternal structure.
 *
 * The rbnode itself contains this nodes color, as well as pointers to the
 * parent, left, and right nodes.
 *
 * We combine the parent pointer and the color (one bit), to save ourselves
 * some space.
 */
struct rbnode {
    uintptr_t _parent_color;
    struct rbnode *left, *right;
};

/* Macros for accessing the parent and color of a node */
#define rb_get_color(ptr) ((uintptr_t)(ptr) & 1)
#define rb_get_parent(ptr) ((struct rbnode *)((uintptr_t)(ptr) & ~1))
#define rb_color(node) rb_get_color((node)->_parent_color)
#define rb_parent(node) rb_get_parent((node)->_parent_color)

/* Set the color and parent with known values (Fastest option) */
#define rb_set_parent_color(node, par, color) \
    (node)->_parent_color = ((uintptr_t)(par) | (color))

/* Use old value along with new value for the color or parent pointer */
#define rb_set_color(node, col) rb_set_parent_color(node, rb_parent(node), col)
#define rb_set_parent(node, par) rb_set_parent_color(node, par, rb_color(node))

/* Check combined parent_color value */
#define __rb_is_red(pc) (!rb_get_color(pc))
#define __rb_is_black(pc) (rb_get_color(pc))

/* Check pointer itself */
#define rb_is_red(node) (__rb_is_red((node)->_parent_color))
#define rb_is_black(node) (__rb_is_black((node)->_parent_color))

/*
 * enum representing the possible results from a rbtree comparision function.
 * These relate in comparision of the first argument to the second argument.
 *
 * IE. If the first argument is less then the second argument, you would return
 * RB_LT.
 */
enum rbcomp {
    RB_EQ = 0,
    RB_GT = 1,
    RB_LT = 2
};

/*
 * This is a structre for holding a rbtree (By hold, it hold the location of
 * the tree and information nessisary for keeping it updated).
 *
 * The compare function should be implemented so that it can take two rbnode's
 * of the type that will be used in this tree (It's intended that container_of
 * will be used to turn them into pointers of the correct type, so while you
 * could mix/match rbnode structure, it's not recommended as it'll complicate
 * your compare function). The compare function should be capable of taking any
 * two rbnode's from the tree and returning whether the first node is greater
 * then, less then, or equal to the second node.
 *
 * This structure should be zero'd before use, and given an approiate
 * comparision function.
 */
struct rbtree {
    struct rbnode *root;

    enum rbcomp (*compare) (const struct rbnode *, const struct rbnode *);
};

/*
 * rb_insert handles inserting a node into the tree. the node should already be
 * created and filled with data, such that it can be compared and won't be
 * found equal to an already existing node.
 *
 * rb_remove works similarly, it handles removing a node from the tree. The
 * node should already be in the tree, and it should be noted that since this
 * rb-tree implementation does no memory management, the node will need to be
 * free'd if it was allocated memory, as rb_remove won't do that.
 */
extern int  rb_insert (struct rbtree *, struct rbnode *);
extern void rb_remove (struct rbtree *, struct rbnode *);

/*
 * rb_search is intended to be used with rb_remove, though it has many other
 * general purpose uses. It runs an O(lg n) search over the tree for a node
 * that is equal to the provided node. This is useful for rb_remove in that if
 * you have enough information to locate the node, then you can use rb_search
 * on a newly created node to find your actual node and then call rb_remove on
 * it.
 *
 * This works well for rbnode_char, where the compare function simply use
 * strcmp on the strings, and if you know the string you want to remove, you
 * can easily create a new rbnode_char on the stack, put your string in it, and
 * then run rb_search to find the node with that same string in the tree.
 */
extern struct rbnode *rb_search (struct rbtree *, const struct rbnode *);

/*
 * Opaque type for handling traversal state. You shouldn't ever access these
 * members, nor care what they are.
 */
typedef struct {
    struct rbnode *current;
    struct rbnode *parent;
    int from;
} rb_trav_state;

/*
 * These functions implement three different traversals over a rb-tree:
 * preorder  (parent, left, right)
 * inorder   (left, parent, right)
 * postorder (left, right, parent)
 */
extern struct rbnode *rb_trav_first_preorder  (struct rbtree *, rb_trav_state *);
extern struct rbnode *rb_trav_first_inorder   (struct rbtree *, rb_trav_state *);
extern struct rbnode *rb_trav_first_postorder (struct rbtree *, rb_trav_state *);

extern struct rbnode *rb_trav_next_preorder  (rb_trav_state *);
extern struct rbnode *rb_trav_next_inorder   (rb_trav_state *);
extern struct rbnode *rb_trav_next_postorder (rb_trav_state *);

/*
 * This is an example implementation of a rbnode containing simple string data.
 */
struct rbnode_char {
    struct rbnode node;
    char *str;
};

/* This is a quick function for easy allocation of an rbnode_char using an already known string.
 * Node the string is duplicated and the returned node contains it's own copy. */
extern struct rbnode_char *rb_char_alloc (const char *);

/* This is the comparision function that should be used by rbtree's containing rbnode_char's */
extern enum rbcomp rb_char_comp (const struct rbnode *, const struct rbnode *);

/*
 * This is used to free a rbnode_char structure allocated via rb_char_alloc.
 */
extern void rb_char_free (struct rbnode_char *);

#endif
