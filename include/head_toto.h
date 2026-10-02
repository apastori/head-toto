#ifndef HEAD_TOTO_H
#define HEAD_TOTO_H

#include <stdint.h>

#define HEAD_TOTO_PROGRAM_NAME   "head-toto"
#define HEAD_TOTO_VERSION_STRING "1.0.0"
#define HEAD_TOTO_DEFAULT_COUNT  10

enum head_toto_exit {
    HEAD_TOTO_EXIT_OK = 0,
    HEAD_TOTO_EXIT_ERR = 1
};

enum head_toto_mode {
    HEAD_TOTO_MODE_LINES,
    HEAD_TOTO_MODE_BYTES
};

enum head_toto_headers {
    HEAD_TOTO_HEADERS_AUTO,
    HEAD_TOTO_HEADERS_ALWAYS,
    HEAD_TOTO_HEADERS_NEVER
};

struct head_toto_opts {
    enum head_toto_mode    mode;     /* default LINES */
    uintmax_t              count;    /* default HEAD_TOTO_DEFAULT_COUNT */
    enum head_toto_headers headers;  /* default AUTO */
    char                   delim;    /* '\n', or '\0' with -z */
};

/*
 * Copy the first opts->count lines or bytes of each FILE to stdout.
 *
 * Preconditions: opts != NULL; files holds nfiles operand strings
 *                (nfiles == 0 means "read standard input once").
 * Return:        HEAD_TOTO_EXIT_OK if every FILE was copied,
 *                HEAD_TOTO_EXIT_ERR if any FILE failed to open or read.
 * Errors:        open/read failures are reported on stderr and processing
 *                continues; a write failure exits the process with status 1.
 */
int head_toto_run(const struct head_toto_opts *opts, int nfiles,
                  char *const files[]);

#endif
