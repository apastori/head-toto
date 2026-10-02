/*
 * head_toto_run.c — FILE operand loop for head-toto.
 *
 * Chain of thought:
 *   - Responsibility: decide the header policy, open each FILE (or use
 *     stdin for "-"), hand the descriptor to the copy loop, and track the
 *     exit status. A failing FILE is reported and skipped; the rest are
 *     still processed, as in GNU head.
 *   - stdin is never closed, because "-" may appear more than once.
 *   - Syscalls: open(), close(); _setmode() on native Windows builds.
 *   - Heap: none.
 *   - Standard: ISO C11 + POSIX.1-2008 (open, close, STDIN_FILENO).
 */

#include "head_toto.h"

#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include "head_toto_cli.h"
#include "head_toto_emit.h"
#include "head_toto_io.h"

#define HEAD_TOTO_STDIN_DISPLAY "standard input"

int head_toto_run(const struct head_toto_opts *opts, int nfiles,
                  char *const files[])
{
    static char *const stdin_only[] = { HEAD_TOTO_ARG_STDIN };
    int status = HEAD_TOTO_EXIT_OK;
    int first = 1;
    int print_headers;
    int i;

#ifdef HEAD_TOTO_WIN32_IO
    _setmode(STDIN_FILENO, _O_BINARY);
    _setmode(STDOUT_FILENO, _O_BINARY);
#endif

    if (nfiles == 0) {
        files = stdin_only;
        nfiles = 1;
    }

    print_headers = opts->headers == HEAD_TOTO_HEADERS_ALWAYS
                    || (opts->headers == HEAD_TOTO_HEADERS_AUTO && nfiles > 1);

    for (i = 0; i < nfiles; i++) {
        const char *name = files[i];
        int is_stdin = strcmp(name, HEAD_TOTO_ARG_STDIN) == 0;
        const char *shown = is_stdin ? HEAD_TOTO_STDIN_DISPLAY : name;
        int fd;
        int rc = 0;

        fd = is_stdin ? STDIN_FILENO
                      : open(name, O_RDONLY | HEAD_TOTO_O_BINARY);
        if (fd < 0) {
            head_toto_emit_open_error(shown);
            status = HEAD_TOTO_EXIT_ERR;
            continue;
        }

        if (print_headers) {
            head_toto_write_header(shown, first);
            first = 0;
        }

        if (opts->count > 0) {
            if (opts->mode == HEAD_TOTO_MODE_LINES) {
                rc = head_toto_copy_lines(fd, STDOUT_FILENO, opts->count,
                                          opts->delim);
            } else {
                rc = head_toto_copy_bytes(fd, STDOUT_FILENO, opts->count);
            }
        }
        if (rc != 0) {
            head_toto_emit_read_error(shown);
            status = HEAD_TOTO_EXIT_ERR;
        }

        if (!is_stdin) {
            close(fd);
        }
    }

    return status;
}
