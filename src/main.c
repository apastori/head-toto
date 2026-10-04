/*
 * main.c — entry point for head-toto.
 *
 * Chain of thought:
 *   - Responsibility: dispatch only. Meta flags (--help / --version) are
 *     resolved first so they win over any usage error; then options are
 *     parsed and the FILE loop in head_toto_run() does the work.
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
    enum head_toto_meta meta_flags;

    meta_flags = scan_meta_flags(argc, argv);

    if (meta_flags == HEAD_TOTO_META_HELP) {
        print_help();
        return HEAD_TOTO_EXIT_OK;
    }

    if (meta_flags == HEAD_TOTO_META_VERSION) {
        print_version();
        return HEAD_TOTO_EXIT_OK;
    }

    if (head_toto_parse_args(argc, argv, &opts, &nfiles) != 0) {
        return HEAD_TOTO_EXIT_ERR;
    }

    return head_toto_run(&opts, nfiles, argv + 1);
}
