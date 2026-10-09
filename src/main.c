/*
 * main.c — entry point for head-toto.
 *
 * Chain of thought:
 *   - Responsibility: dispatch only. head_toto_parse_args() handles the
 *     options left to right and reports --help / --version as soon as it
 *     reaches them (as GNU head does); the FILE loop in head_toto_run()
 *     does the work.
 *   - Syscalls: none directly.
 *   - Heap: none.
 *   - Standard: ISO C11.
 */

#include "head_toto.h"
#include "head_toto_cli.h"

int main(int argc, char *argv[])
{
    struct head_toto_opts opts;
    int nfiles;
    enum head_toto_parse_result result;

    result = head_toto_parse_args(argc, argv, &opts, &nfiles);

    if (result == HEAD_TOTO_PARSE_HELP) {
        print_help();
        return HEAD_TOTO_EXIT_OK;
    }

    if (result == HEAD_TOTO_PARSE_VERSION) {
        print_version();
        return HEAD_TOTO_EXIT_OK;
    }

    if (result == HEAD_TOTO_PARSE_ERROR) {
        return HEAD_TOTO_EXIT_ERR;
    }

    return head_toto_run(&opts, nfiles, argv + 1);
}
