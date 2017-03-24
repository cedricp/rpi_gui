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

#define _XOPEN_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pty.h>
#include <stdlib.h>
#include <signal.h>
#include "hl_vt100.h"

struct vt100_headless *new_vt100_headless(void)
{
    return calloc(1, sizeof(struct vt100_headless));
}

void delete_vt100_headless(struct vt100_headless *that)
{
    free(that);
}

static void set_non_canonical(struct vt100_headless *that, int fd)
{
    struct termios termios;

    ioctl(fd, TCGETS, &that->backup);
    ioctl(fd, TCGETS, &termios);
    termios.c_lflag |= ICANON | ECHOE;
    termios.c_iflag |= ICANON | ECHOE;
    termios.c_cc[VMIN] = 1;  /* blocking read until 1 character arrives */
    termios.c_cc[VTIME] = 0; /* inter-character timer unused */
    termios.c_cc[VINTR] = 0;
    ioctl(fd, TCSETS, &termios);
}

static void restore_termios(struct vt100_headless *that, int fd)
{
    ioctl(fd, TCSETS, &that->backup);
}

#ifndef NDEBUG
static void strdump(char *str)
{
    while (*str != '\0')
    {
        if (*str >= ' ' && *str <= '~')
            fprintf(stderr, "%c", *str);
        else
            fprintf(stderr, "\\0%o", *str);
        str += 1;
    }
    fprintf(stderr, "\n");
}
#endif

void vt100_headless_stop(struct vt100_headless *that)
{
    that->should_quit = 1;
}

int vt100_headless_main_loop(struct vt100_headless *that)
{
    char buffer[4096];
    fd_set rfds;
    int retval;
    ssize_t read_size;
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 500;
    while (!that->should_quit)
    {
        FD_ZERO(&rfds);
        FD_SET(that->master, &rfds);
        FD_SET(0, &rfds);
        retval = select(that->master + 1, &rfds, NULL, NULL, &tv);
        if (retval == -1)
        {
            perror("select()");
        }
        if (FD_ISSET(0, &rfds))
        {
            read_size = read(0, &buffer, 4096);
            if (read_size == -1)
            {
                perror("read");
                return EXIT_FAILURE;
            }
            buffer[read_size] = '\0';
            write(that->master, buffer, read_size);
        }
        if (FD_ISSET(that->master, &rfds))
        {
            read_size = read(that->master, &buffer, 4096);
            if (read_size == -1)
            {
                perror("read");
                return EXIT_FAILURE;
            }
            buffer[read_size] = '\0';

            lw_terminal_vt100_read_str(that->term, buffer);
            if (that->changed != NULL)
                that->changed(that);
        }
    }

    kill(that->pid, SIGABRT);

    int died = 0, loop;
    for (loop = 0; !died && loop < 5; ++loop)
    {
        int status;
        pid_t id;
        sleep(1);
        if (waitpid(that->pid, &status, WNOHANG) == that->pid) died = 1;
    }

    if (!died) kill(that->pid, SIGKILL);

    return EXIT_SUCCESS;
}

void master_write(void *user_data, void *buffer, size_t len)
{
    struct vt100_headless *that;

    that = (struct vt100_headless*)user_data;
    write(that->master, buffer, len);
}

const char **vt100_headless_getlines(struct vt100_headless *that)
{
    return lw_terminal_vt100_getlines(that->term);
}

void vt100_headless_fork(struct vt100_headless *that,
                         const char *progname,
                         char **argv)
{
    int child;
    struct winsize winsize;

    set_non_canonical(that, 0);
    winsize.ws_row = 24;
    winsize.ws_col = 50;
    child = forkpty(&that->master, NULL, NULL, NULL);
    if (child == CHILD)
    {
        setsid();
        putenv("TERM=vt100");
        execvp(progname, argv);
        return ;
    }
    else
    {
    	that->pid = child;
        that->term = lw_terminal_vt100_init(that, lw_terminal_parser_default_unimplemented);
        that->term->master_write = master_write;
        ioctl(that->master, TIOCSWINSZ, &winsize);
    }
    restore_termios(that, 0);
}
