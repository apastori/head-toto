#ifndef HEAD_TOTO_CLI_H
#define HEAD_TOTO_CLI_H

#include "head_toto.h"

#define HEAD_TOTO_ARG_HELP            "--help"
#define HEAD_TOTO_ARG_HELP_SHORT      "--h"
#define HEAD_TOTO_ARG_VERSION         "--version"
#define HEAD_TOTO_ARG_VERSION_SHORT   "--v"

#define HEAD_TOTO_ARG_LINES_SHORT     "-n"
#define HEAD_TOTO_ARG_LINES_LONG      "--lines"
#define HEAD_TOTO_ARG_BYTES_SHORT     "-c"
#define HEAD_TOTO_ARG_BYTES_LONG      "--bytes"
#define HEAD_TOTO_ARG_QUIET_SHORT     "-q"
#define HEAD_TOTO_ARG_QUIET_LONG      "--quiet"
#define HEAD_TOTO_ARG_SILENT_LONG     "--silent"
#define HEAD_TOTO_ARG_VERBOSE_SHORT   "-v"
#define HEAD_TOTO_ARG_VERBOSE_LONG    "--verbose"
#define HEAD_TOTO_ARG_ZERO_SHORT      "-z"
#define HEAD_TOTO_ARG_ZERO_LONG       "--zero-terminated"

#define HEAD_TOTO_ARG_END_OF_OPTS     "--"
#define HEAD_TOTO_ARG_STDIN           "-"

enum head_toto_meta {
    HEAD_TOTO_META_NONE,
    HEAD_TOTO_META_HELP,
    HEAD_TOTO_META_VERSION
};

/*
 * Scan argv[1..] up to the first "--" for meta flags.
 * Return: HEAD_TOTO_META_HELP if any help flag is present (wins over
 *         version), else HEAD_TOTO_META_VERSION if any version flag is
 *         present, else HEAD_TOTO_META_NONE.
 */
enum head_toto_meta scan_meta_flags(int argc, char *argv[]);

/*
 * Parse options into *opts and compact FILE operands in place into
 * argv[1 .. *nfiles], preserving their order.
 *
 * Preconditions:  argv is main()'s argument vector; opts, nfiles != NULL.
 * Postconditions: on success *opts and *nfiles are set.
 * Return:         0 on success, -1 on a usage error (diagnostic already
 *                 written to stderr, nothing written to stdout).
 */
int head_toto_parse_args(int argc, char *argv[], struct head_toto_opts *opts,
                         int *nfiles);

void print_help(void);
void print_version(void);

#endif
