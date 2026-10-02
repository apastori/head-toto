#ifndef HEAD_TOTO_IO_H
#define HEAD_TOTO_IO_H

#include <stdint.h>

#define HEAD_TOTO_BUFSIZE 8192

/*
 * Native Windows CRT descriptors default to text mode, which would turn
 * "\r\n" into "\n" and break -c byte counts. The Makefile defines
 * HEAD_TOTO_WIN32_IO only when <io.h> provides _O_BINARY.
 */
#ifdef HEAD_TOTO_WIN32_IO
#include <io.h>
#include <fcntl.h>
#define HEAD_TOTO_O_BINARY _O_BINARY
#else
#define HEAD_TOTO_O_BINARY 0
#endif

/*
 * Copy the first n delim-terminated lines from fd_in to fd_out. A final
 * line without delim is copied as-is.
 *
 * Return: 0 when n lines were copied or EOF was reached, -1 on a read
 *         error with errno preserved.
 * Errors: a write failure exits the process with status 1.
 */
int head_toto_copy_lines(int fd_in, int fd_out, uintmax_t n, char delim);

/*
 * Copy the first n bytes from fd_in to fd_out.
 *
 * Return: 0 when n bytes were copied or EOF was reached, -1 on a read
 *         error with errno preserved.
 * Errors: a write failure exits the process with status 1.
 */
int head_toto_copy_bytes(int fd_in, int fd_out, uintmax_t n);

/*
 * Write "==> display_name <==\n" to stdout, preceded by a blank line
 * unless first is non-zero.
 *
 * Errors: a write failure exits the process with status 1.
 */
void head_toto_write_header(const char *display_name, int first);

#endif
