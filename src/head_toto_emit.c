/*
 * head_toto_emit.c — stderr diagnostics for head-toto.
 *
 * Chain of thought:
 *   - Responsibility: format every user-visible error message in one place
 *     so wording stays identical to GNU head.
 *   - errno is captured into a local before any other call, because
 *     fprintf() itself may clobber it.
 *   - Syscalls: none directly (stdio on stderr only; stdout is never
 *     touched here).
 *   - Heap: none.
 *   - Standard: ISO C11.
 */

#include "head_toto_emit.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "head_toto.h"

static void emit_try_help(void)
{
    fprintf(stderr, "Try '%s --help' for more information.\n",
            HEAD_TOTO_PROGRAM_NAME);
}

void head_toto_emit_error(const char *context)
{
    int saved = errno;

    fprintf(stderr, "%s: %s: %s\n", HEAD_TOTO_PROGRAM_NAME, context,
            strerror(saved));
}

void head_toto_emit_open_error(const char *name)
{
    int saved = errno;

    fprintf(stderr, "%s: cannot open '%s' for reading: %s\n",
            HEAD_TOTO_PROGRAM_NAME, name, strerror(saved));
}

void head_toto_emit_read_error(const char *name)
{
    int saved = errno;

    fprintf(stderr, "%s: error reading '%s': %s\n",
            HEAD_TOTO_PROGRAM_NAME, name, strerror(saved));
}

void head_toto_emit_invalid_count(const char *what, const char *arg)
{
    fprintf(stderr, "%s: invalid number of %s: '%s'\n",
            HEAD_TOTO_PROGRAM_NAME, what, arg);
}

void head_toto_emit_invalid_option(char c)
{
    fprintf(stderr, "%s: invalid option -- '%c'\n", HEAD_TOTO_PROGRAM_NAME,
            c);
    emit_try_help();
}

void head_toto_emit_unrecognized_option(const char *arg)
{
    fprintf(stderr, "%s: unrecognized option '%s'\n",
            HEAD_TOTO_PROGRAM_NAME, arg);
    emit_try_help();
}

void head_toto_emit_missing_arg_short(char c)
{
    fprintf(stderr, "%s: option requires an argument -- '%c'\n",
            HEAD_TOTO_PROGRAM_NAME, c);
    emit_try_help();
}

void head_toto_emit_missing_arg_long(const char *name)
{
    fprintf(stderr, "%s: option '%s' requires an argument\n",
            HEAD_TOTO_PROGRAM_NAME, name);
    emit_try_help();
}
