/*
 * head_toto_io.c — raw read/write copy loops and header output.
 *
 * Chain of thought:
 *   - Responsibility: move bytes from an input descriptor to an output
 *     descriptor, stopping after N lines or N bytes, and write file headers.
 *   - Syscalls: read(), write(). No stdio, so file data is never buffered
 *     twice or reordered against headers.
 *   - No lseek(): stdin may be a pipe, so at most one buffer is read past
 *     the needed data and never pushed back.
 *   - Heap: none. One stack buffer of HEAD_TOTO_BUFSIZE bytes per copy call.
 *   - Standard: ISO C11 + POSIX.1-2008 (read, write, ssize_t).
 */

#include "head_toto_io.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "head_toto.h"
#include "head_toto_emit.h"

/*
 * Write all len bytes of buf to fd.
 *
 * Preconditions:  buf != NULL, len > 0.
 * Postconditions: all len bytes written, or the process has exited.
 * Errors:         EINTR is retried; any other failure reports
 *                 "head-toto: write: <reason>" and exits with status 1.
 */
static void try_write_all(int fd, const char *buf, size_t len)
{
    while (len > 0) {
        ssize_t n = write(fd, buf, len);

        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            head_toto_emit_error("write");
            exit(HEAD_TOTO_EXIT_ERR);
        }
        buf += n;
        len -= (size_t)n;
    }
}

/*
 * Read up to len bytes from fd into buf.
 *
 * Preconditions: buf != NULL, len > 0.
 * Return:        bytes read (> 0), 0 on EOF, -1 on error with errno set.
 * Errors:        EINTR is retried.
 */
static ssize_t try_read(int fd, char *buf, size_t len)
{
    for (;;) {
        ssize_t n = read(fd, buf, len);

        if (n < 0 && errno == EINTR) {
            continue;
        }
        return n;
    }
}

int head_toto_copy_lines(int fd_in, int fd_out, uintmax_t n, char delim)
{
    char buf[HEAD_TOTO_BUFSIZE];

    while (n > 0) {
        ssize_t got = try_read(fd_in, buf, sizeof buf);
        size_t len;
        const char *p = buf;
        size_t left;

        if (got < 0) {
            return -1;
        }
        if (got == 0) {
            break;
        }

        len = (size_t)got;
        left = len;
        while (left > 0) {
            const char *hit = memchr(p, delim, left);

            if (hit == NULL) {
                break;
            }
            left -= (size_t)(hit - p) + 1;
            p = hit + 1;
            if (--n == 0) {
                len = (size_t)(p - buf);
                break;
            }
        }

        try_write_all(fd_out, buf, len);
    }

    return 0;
}

int head_toto_copy_bytes(int fd_in, int fd_out, uintmax_t n)
{
    char buf[HEAD_TOTO_BUFSIZE];

    while (n > 0) {
        size_t want = n < sizeof buf ? (size_t)n : sizeof buf;
        ssize_t got = try_read(fd_in, buf, want);

        if (got < 0) {
            return -1;
        }
        if (got == 0) {
            break;
        }

        try_write_all(fd_out, buf, (size_t)got);
        n -= (uintmax_t)got;
    }

    return 0;
}

void head_toto_write_header(const char *display_name, int first)
{
    static const char sep[] = "\n";
    static const char prefix[] = "==> ";
    static const char suffix[] = " <==\n";
    size_t name_len = strlen(display_name);

    if (!first) {
        try_write_all(STDOUT_FILENO, sep, sizeof sep - 1);
    }
    try_write_all(STDOUT_FILENO, prefix, sizeof prefix - 1);
    if (name_len > 0) {
        try_write_all(STDOUT_FILENO, display_name, name_len);
    }
    try_write_all(STDOUT_FILENO, suffix, sizeof suffix - 1);
}
