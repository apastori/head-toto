/*
 * test_copy_bytes.c — unit tests for head_toto_copy_bytes().
 *
 * Chain of thought:
 *   - Same fixture pattern as test_copy_lines.c: input file under
 *     build/tests/, copy into an output file, read back, compare.
 *   - Covers N below, equal to, above the input size, and N = 0.
 *   - The CRLF case proves bytes pass through untranslated, which on
 *     Windows depends on HEAD_TOTO_O_BINARY.
 *   - Syscalls: open, read, write, close; remove() for cleanup.
 *   - Heap: none.
 *   - Standard: ISO C11 + POSIX.1-2008.
 */

#include "test_copy_bytes.h"

#include <assert.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "head_toto_io.h"

static void fixture_path(char *dst, size_t cap, const char *name,
                         const char *suffix)
{
    int n = snprintf(dst, cap, "build/tests/fixture_%s_%s.tmp", name,
                     suffix);

    assert(n > 0 && (size_t)n < cap);
}

static void write_file(const char *path, const char *data, size_t len)
{
    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC | HEAD_TOTO_O_BINARY,
                  0644);
    size_t off = 0;
    int rc;

    assert(fd >= 0);
    while (off < len) {
        ssize_t n = write(fd, data + off, len - off);

        assert(n > 0);
        off += (size_t)n;
    }
    rc = close(fd);
    assert(rc == 0);
}

static size_t read_file(const char *path, char *dst, size_t cap)
{
    int fd = open(path, O_RDONLY | HEAD_TOTO_O_BINARY);
    size_t off = 0;
    int rc;

    assert(fd >= 0);
    for (;;) {
        ssize_t n = read(fd, dst + off, cap - off);

        assert(n >= 0);
        if (n == 0) {
            break;
        }
        off += (size_t)n;
        assert(off < cap);
    }
    rc = close(fd);
    assert(rc == 0);
    return off;
}

static void expect_copy(const char *name, const char *in, size_t in_len,
                        uintmax_t n, const char *want, size_t want_len)
{
    char in_path[128];
    char out_path[128];
    char out[256];
    int fd_in;
    int fd_out;
    int rc;
    size_t got;

    fixture_path(in_path, sizeof in_path, name, "in");
    fixture_path(out_path, sizeof out_path, name, "out");
    write_file(in_path, in, in_len);

    fd_in = open(in_path, O_RDONLY | HEAD_TOTO_O_BINARY);
    assert(fd_in >= 0);
    fd_out = open(out_path,
                  O_WRONLY | O_CREAT | O_TRUNC | HEAD_TOTO_O_BINARY, 0644);
    assert(fd_out >= 0);

    rc = head_toto_copy_bytes(fd_in, fd_out, n);
    assert(rc == 0);

    rc = close(fd_in);
    assert(rc == 0);
    rc = close(fd_out);
    assert(rc == 0);

    got = read_file(out_path, out, sizeof out);
    assert(got == want_len);
    assert(memcmp(out, want, want_len) == 0);

    rc = remove(in_path);
    assert(rc == 0);
    rc = remove(out_path);
    assert(rc == 0);
}

void test_copy_bytes_run(void)
{
    static const char in[] = "hello world\n";
    static const char crlf[] = "a\r\nb\r\n";

    expect_copy("bytes_short", in, sizeof in - 1, 5, "hello", 5);
    expect_copy("bytes_exact", in, sizeof in - 1, sizeof in - 1,
                in, sizeof in - 1);
    expect_copy("bytes_over", in, sizeof in - 1, 1000, in, sizeof in - 1);
    printf("PASS: copy_bytes N below, equal to, and above input size\n");

    expect_copy("bytes_zero", in, sizeof in - 1, 0, "", 0);
    printf("PASS: copy_bytes N = 0\n");

    expect_copy("bytes_crlf", crlf, sizeof crlf - 1, 3, "a\r\n", 3);
    printf("PASS: copy_bytes preserves CRLF bytes\n");
}
