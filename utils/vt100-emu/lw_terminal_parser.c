/*
 * Copyright (c) 2016 Julien Palard.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, that list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, that list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * that SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * that SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdlib.h>
#ifndef NDEBUG
#    include <stdio.h>
#endif

#include "lw_terminal_parser.h"

static void lw_terminal_parser_push(struct lw_terminal *that, char c)
{
    if (that->stack_ptr >= TERM_STACK_SIZE)
        return ;
    that->stack[that->stack_ptr++] = c;
}

static void lw_terminal_parser_parse_params(struct lw_terminal *that)
{
    unsigned int i;
    int got_something;

    got_something = 0;
    that->argc = 0;
    that->argv[0] = 0;
    for (i = 0; i < that->stack_ptr; ++i)
    {
        if (that->stack[i] >= '0' && that->stack[i] <= '9')
        {
            got_something = 1;
            that->argv[that->argc] = that->argv[that->argc] * 10
                + that->stack[i] - '0';
        }
        else if (that->stack[i] == ';')
        {
            got_something = 0;
            that->argc += 1;
            that->argv[that->argc] = 0;
        }
    }
    that->argc += got_something;
}

static void lw_terminal_parser_call_CSI(struct lw_terminal *that, char c)
{
    lw_terminal_parser_parse_params(that);
    if (((term_action *)&that->callbacks.csi)[c - '0'] == NULL)
    {
        if (that->unimplemented != NULL)
            that->unimplemented(that, "CSI", c);
        goto leave;
    }
    ((term_action *)&that->callbacks.csi)[c - '0'](that);
leave:
    that->state = INIT;
    that->flag = '\0';
    that->stack_ptr = 0;
    that->argc = 0;
}

static void lw_terminal_parser_call_ESC(struct lw_terminal *that, char c)
{
    if (((term_action *)&that->callbacks.esc)[c - '0'] == NULL)
    {
        if (that->unimplemented != NULL)
            that->unimplemented(that, "ESC", c);
        goto leave;
    }
    ((term_action *)&that->callbacks.esc)[c - '0'](that);
leave:
    that->state = INIT;
    that->stack_ptr = 0;
    that->argc = 0;
}

static void lw_terminal_parser_call_HASH(struct lw_terminal *that, char c)
{
    if (((term_action *)&that->callbacks.hash)[c - '0'] == NULL)
    {
        if (that->unimplemented != NULL)
            that->unimplemented(that, "HASH", c);
        goto leave;
    }
    ((term_action *)&that->callbacks.hash)[c - '0'](that);
leave:
    that->state = INIT;
    that->stack_ptr = 0;
    that->argc = 0;
}

static void lw_terminal_parser_call_GSET(struct lw_terminal *that, char c)
{
    if (c < '0' || c > 'B'
        || ((term_action *)&that->callbacks.scs)[c - '0'] == NULL)
    {
        if (that->unimplemented != NULL)
            that->unimplemented(that, "GSET", c);
        goto leave;
    }
    ((term_action *)&that->callbacks.scs)[c - '0'](that);
leave:
    that->state = INIT;
    that->stack_ptr = 0;
    that->argc = 0;
}

/*
** INIT
**  \_ ESC "\033"
**  |   \_ CSI   "\033["
**  |   |   \_ c == '?' : term->flag = '?'
**  |   |   \_ c == ';' || (c >= '0' && c <= '9') : term_push
**  |   |   \_ else : term_call_CSI()
**  |   \_ HASH  "\033#"
**  |   |   \_ term_call_hash()
**  |   \_ G0SET "\033("
**  |   |   \_ term_call_GSET()
**  |   \_ G1SET "\033)"
**  |   |   \_ term_call_GSET()
**  \_ term->write()
*/
void lw_terminal_parser_read(struct lw_terminal *that, char c)
{
    if (that->state == INIT)
    {
        if (c == '\033')
            that->state = ESC;
        else
            that->write(that, c);
    }
    else if (that->state == ESC)
    {
        if (c == '[')
            that->state = CSI;
        else if (c == '#')
            that->state = HASH;
        else if (c == '(')
            that->state = G0SET;
        else if (c == ')')
            that->state = G1SET;
        else if (c >= '0' && c <= 'z')
            lw_terminal_parser_call_ESC(that, c);
        else that->write(that, c);
    }
    else if (that->state == HASH)
    {
        if (c >= '0' && c <= '9')
            lw_terminal_parser_call_HASH(that, c);
        else
            that->write(that, c);
    }
    else if (that->state == G0SET || that->state == G1SET)
    {
        lw_terminal_parser_call_GSET(that, c);
    }
    else if (that->state == CSI)
    {
        if (c == '?')
            that->flag = '?';
        else if (c == ';' || (c >= '0' && c <= '9'))
            lw_terminal_parser_push(that, c);
        else if (c >= '?' && c <= 'z')
            lw_terminal_parser_call_CSI(that, c);
        else
            that->write(that, c);
    }
}

void lw_terminal_parser_read_str(struct lw_terminal *that, char *c)
{
    while (*c)
        lw_terminal_parser_read(that, *c++);
}

#ifndef NDEBUG
void lw_terminal_parser_default_unimplemented(struct lw_terminal* that, char *seq, char chr)
{
    unsigned int argc;

    fprintf(stderr, "WARNING: UNIMPLEMENTED %s (", seq);
    for (argc = 0; argc < that->argc; ++argc)
    {
        fprintf(stderr, "%d", that->argv[argc]);
        if (argc != that->argc - 1)
            fprintf(stderr, ", ");
    }
    fprintf(stderr, ")%o\n", chr);
}
#else
void lw_terminal_parser_default_unimplemented(struct lw_terminal* that, char *seq, char chr)
{
    that = that;
    seq = seq;
    chr = chr;
}
#endif

struct lw_terminal *lw_terminal_parser_init(void)
{
    return calloc(1, sizeof(struct lw_terminal));
}

void lw_terminal_parser_destroy(struct lw_terminal* that)
{
    free(that);
}
