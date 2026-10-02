/*
 * head_toto_cli.c — command-line handling for head-toto.
 *
 * Chain of thought:
 *   - Responsibility: detect meta flags, parse -n/-c/-q/-v/-z and their
 *     long forms, collect FILE operands, and print help/version text.
 *   - getopt/getopt_long are not used: getopt_long is a GNU extension and
 *     the exact error wording must be controlled. All option spellings
 *     come from the HEAD_TOTO_ARG_* macros.
 *   - Operands are compacted in place into argv[1 .. nfiles]; the write
 *     index never passes the read index, so no copy of argv is needed.
 *   - Syscalls: none directly (help/version use stdio on stdout).
 *   - Heap: none.
 *   - Standard: ISO C11.
 */

#include "head_toto_cli.h"

#include <stdio.h>
#include <string.h>

#include "head_toto_count.h"
#include "head_toto_emit.h"

/*
 * Return non-zero if flag appears verbatim in argv[1..] before the first
 * "--".
 */
static int argv_has_exact(int argc, char *argv[], const char *flag)
{
    int i;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], HEAD_TOTO_ARG_END_OF_OPTS) == 0) {
            return 0;
        }
        if (strcmp(argv[i], flag) == 0) {
            return 1;
        }
    }
    return 0;
}

enum head_toto_meta scan_meta_flags(int argc, char *argv[])
{
    if (argv_has_exact(argc, argv, HEAD_TOTO_ARG_HELP)
        || argv_has_exact(argc, argv, HEAD_TOTO_ARG_HELP_SHORT)) {
        return HEAD_TOTO_META_HELP;
    }
    if (argv_has_exact(argc, argv, HEAD_TOTO_ARG_VERSION)
        || argv_has_exact(argc, argv, HEAD_TOTO_ARG_VERSION_SHORT)) {
        return HEAD_TOTO_META_VERSION;
    }
    return HEAD_TOTO_META_NONE;
}

/*
 * Parse value as NUM for mode and store it in *opts.
 * Return: 0 on success, -1 after emitting "invalid number of ..." on error.
 */
static int apply_count(struct head_toto_opts *opts, enum head_toto_mode mode,
                       const char *value)
{
    uintmax_t count;

    if (head_toto_parse_count(value, &count) != HEAD_TOTO_COUNT_OK) {
        head_toto_emit_invalid_count(
            mode == HEAD_TOTO_MODE_LINES ? "lines" : "bytes", value);
        return -1;
    }
    opts->mode = mode;
    opts->count = count;
    return 0;
}

/*
 * If arg is the long option name, alone or as "name=VALUE", return a
 * pointer to the character after the name ('\0' or '='); else NULL.
 */
static const char *match_long_with_value(const char *arg, const char *name)
{
    size_t len = strlen(name);

    if (strncmp(arg, name, len) != 0) {
        return NULL;
    }
    if (arg[len] != '\0' && arg[len] != '=') {
        return NULL;
    }
    return arg + len;
}

/*
 * Handle one "--xxx" token at argv[*i]; may consume argv[*i + 1].
 * Return: 0 on success, -1 after emitting a diagnostic.
 */
static int parse_long(int argc, char *argv[], int *i,
                      struct head_toto_opts *opts)
{
    const char *arg = argv[*i];
    const char *rest;
    const char *name;
    enum head_toto_mode mode;

    if (strcmp(arg, HEAD_TOTO_ARG_QUIET_LONG) == 0
        || strcmp(arg, HEAD_TOTO_ARG_SILENT_LONG) == 0) {
        opts->headers = HEAD_TOTO_HEADERS_NEVER;
        return 0;
    }
    if (strcmp(arg, HEAD_TOTO_ARG_VERBOSE_LONG) == 0) {
        opts->headers = HEAD_TOTO_HEADERS_ALWAYS;
        return 0;
    }
    if (strcmp(arg, HEAD_TOTO_ARG_ZERO_LONG) == 0) {
        opts->delim = '\0';
        return 0;
    }

    if ((rest = match_long_with_value(arg, HEAD_TOTO_ARG_LINES_LONG))
        != NULL) {
        name = HEAD_TOTO_ARG_LINES_LONG;
        mode = HEAD_TOTO_MODE_LINES;
    } else if ((rest = match_long_with_value(arg, HEAD_TOTO_ARG_BYTES_LONG))
               != NULL) {
        name = HEAD_TOTO_ARG_BYTES_LONG;
        mode = HEAD_TOTO_MODE_BYTES;
    } else {
        head_toto_emit_unrecognized_option(arg);
        return -1;
    }

    if (*rest == '=') {
        return apply_count(opts, mode, rest + 1);
    }
    if (*i + 1 >= argc) {
        head_toto_emit_missing_arg_long(name);
        return -1;
    }
    *i += 1;
    return apply_count(opts, mode, argv[*i]);
}

/*
 * Handle one clustered short-option token ("-qn5") at argv[*i]; may
 * consume argv[*i + 1] as the value of a trailing -n / -c.
 * Return: 0 on success, -1 after emitting a diagnostic.
 */
static int parse_short(int argc, char *argv[], int *i,
                       struct head_toto_opts *opts)
{
    const char *p = argv[*i] + 1;

    for (; *p != '\0'; p++) {
        char c = *p;
        enum head_toto_mode mode;

        if (c == HEAD_TOTO_ARG_QUIET_SHORT[1]) {
            opts->headers = HEAD_TOTO_HEADERS_NEVER;
            continue;
        }
        if (c == HEAD_TOTO_ARG_VERBOSE_SHORT[1]) {
            opts->headers = HEAD_TOTO_HEADERS_ALWAYS;
            continue;
        }
        if (c == HEAD_TOTO_ARG_ZERO_SHORT[1]) {
            opts->delim = '\0';
            continue;
        }

        if (c == HEAD_TOTO_ARG_LINES_SHORT[1]) {
            mode = HEAD_TOTO_MODE_LINES;
        } else if (c == HEAD_TOTO_ARG_BYTES_SHORT[1]) {
            mode = HEAD_TOTO_MODE_BYTES;
        } else {
            head_toto_emit_invalid_option(c);
            return -1;
        }

        if (p[1] != '\0') {
            return apply_count(opts, mode, p + 1);
        }
        if (*i + 1 >= argc) {
            head_toto_emit_missing_arg_short(c);
            return -1;
        }
        *i += 1;
        return apply_count(opts, mode, argv[*i]);
    }

    return 0;
}

void init_opts(struct head_toto_opts *opts)
{
    opts->mode = HEAD_TOTO_MODE_LINES;
    opts->count = HEAD_TOTO_DEFAULT_COUNT;
    opts->headers = HEAD_TOTO_HEADERS_AUTO;
    opts->delim = '\n';
}

int head_toto_parse_args(int argc, char *argv[], struct head_toto_opts *opts,
                         int *nfiles)
{
    int out = 1;
    int only_operands = 0;
    int i;

    init_opts(opts);

    for (i = 1; i < argc; i++) {
        char *arg = argv[i];
        int rc;

        if (only_operands || arg[0] != '-'
            || strcmp(arg, HEAD_TOTO_ARG_STDIN) == 0) {
            argv[out++] = arg;
            continue;
        }
        if (strcmp(arg, HEAD_TOTO_ARG_END_OF_OPTS) == 0) {
            only_operands = 1;
            continue;
        }

        if (arg[1] == '-') {
            rc = parse_long(argc, argv, &i, opts);
        } else {
            rc = parse_short(argc, argv, &i, opts);
        }
        if (rc != 0) {
            return -1;
        }
    }

    *nfiles = out - 1;
    return 0;
}

void print_help(void)
{
    printf("Usage: %s [OPTION]... [FILE]...\n", HEAD_TOTO_PROGRAM_NAME);
    printf("Print the first %d lines of each FILE to standard output.\n",
           HEAD_TOTO_DEFAULT_COUNT);
    fputs("With more than one FILE, precede each with a header giving the "
          "file name.\n"
          "\n"
          "With no FILE, or when FILE is -, read standard input.\n"
          "\n", stdout);
    fputs("  -c, --bytes=NUM          print the first NUM bytes of each "
          "file\n"
          "  -n, --lines=NUM          print the first NUM lines instead of "
          "the first 10\n"
          "  -q, --quiet, --silent    never print headers giving file "
          "names\n"
          "  -v, --verbose            always print headers giving file "
          "names\n"
          "  -z, --zero-terminated    line delimiter is NUL, not newline\n"
          "      --help, --h          display this help and exit\n"
          "      --version, --v       output version information and exit\n"
          "\n"
          "NUM is a non-negative decimal integer.\n", stdout);
}

void print_version(void)
{
    printf("%s %s\n", HEAD_TOTO_PROGRAM_NAME, HEAD_TOTO_VERSION_STRING);
}
