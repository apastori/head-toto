/*
 * test_copy_lines.c — unit tests for head_toto_copy_lines().
 *
 * Chain of thought:
 *   - Each case writes its input to build/tests/fixture_<case>_in.tmp,
 *     copies it into fixture_<case>_out.tmp through head_toto_copy_lines(),
 *     reads the output back, and compares it byte for byte.
 *   - Fixtures are opened with HEAD_TOTO_O_BINARY so Windows text mode
 *     never rewrites the bytes under test.
 *   - The buffer-boundary case uses inputs larger than HEAD_TOTO_BUFSIZE
 *     so the delimiter scan must carry its count across read() chunks.
 *   - Syscalls: open, read, write, close; remove() for cleanup.
 *   - Heap: none (large fixtures live on the stack, about 40 KB).
 *   - Standard: ISO C11 + POSIX.1-2008.
 */

#include "test_copy_lines.h"

#include <assert.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "head_toto_io.h"

#define LONG_LINE_LEN 1000
#define LONG_LINES    20
#define BIG_LEN       (LONG_LINES * (LONG_LINE_LEN + 1))

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

/*
 * Run head_toto_copy_lines() over in[0..in_len) and return the number of
 * bytes produced, stored in out (capacity out_cap).
 */
static size_t run_copy(const char *name, const char *in, size_t in_len,
                       uintmax_t n, char delim, char *out, size_t out_cap)
{
    char in_path[128];
    char out_path[128];
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

    rc = head_toto_copy_lines(fd_in, fd_out, n, delim);
    assert(rc == 0);

    rc = close(fd_in);
    assert(rc == 0);
    rc = close(fd_out);
    assert(rc == 0);

    got = read_file(out_path, out, out_cap);
    rc = remove(in_path);
    assert(rc == 0);
    rc = remove(out_path);
    assert(rc == 0);
    return got;
}

static void expect_copy(const char *name, const char *in, size_t in_len,
                        uintmax_t n, char delim, const char *want,
                        size_t want_len)
{
    char out[256];
    size_t got = run_copy(name, in, in_len, n, delim, out, sizeof out);

    assert(got == want_len);
    assert(memcmp(out, want, want_len) == 0);
}

static void test_fewer_than_n(void)
{
    static const char in[] = "one\ntwo\nthree\n";

    expect_copy("lines_fewer", in, sizeof in - 1, 10, '\n',
                in, sizeof in - 1);
    printf("PASS: copy_lines fewer lines than N\n");
}

static void test_more_than_n(void)
{
    static const char in[] =
        "1\n2\n3\n4\n5\n6\n7\n8\n9\n10\n11\n12\n13\n14\n15\n";
    static const char want[] = "1\n2\n3\n4\n5\n6\n7\n8\n9\n10\n";

    expect_copy("lines_more", in, sizeof in - 1, 10, '\n',
                want, sizeof want - 1);
    printf("PASS: copy_lines more lines than N\n");
}

static void test_no_trailing_newline(void)
{
    static const char in[] = "a\nb";

    expect_copy("lines_notrail", in, sizeof in - 1, 2, '\n',
                in, sizeof in - 1);
    printf("PASS: copy_lines final line without newline\n");
}

static void test_zero(void)
{
    static const char in[] = "a\nb\n";

    expect_copy("lines_zero", in, sizeof in - 1, 0, '\n', "", 0);
    printf("PASS: copy_lines N = 0\n");
}

static void test_nul_delimiter(void)
{
    static const char in[] = { 'a', '\0', 'b', '\0', 'c', '\0' };
    static const char want[] = { 'a', '\0', 'b', '\0' };

    expect_copy("lines_nul", in, sizeof in, 2, '\0', want, sizeof want);
    printf("PASS: copy_lines NUL delimiter\n");
}

static void test_buffer_boundary(void)
{
    char big_in[BIG_LEN];
    char big_out[BIG_LEN + 1];
    size_t i;
    size_t got;
    size_t want_len;

    for (i = 0; i < BIG_LEN; i++) {
        big_in[i] = (i % (LONG_LINE_LEN + 1) == LONG_LINE_LEN)
                        ? '\n'
                        : (char)('a' + (i / (LONG_LINE_LEN + 1)) % 26);
    }

    want_len = 12 * (LONG_LINE_LEN + 1);
    assert(want_len > HEAD_TOTO_BUFSIZE);
    got = run_copy("lines_boundary", big_in, BIG_LEN, 12, '\n', big_out,
                   sizeof big_out);
    assert(got == want_len);
    assert(memcmp(big_out, big_in, want_len) == 0);

    memset(big_in, 'x', HEAD_TOTO_BUFSIZE + 100);
    big_in[HEAD_TOTO_BUFSIZE + 100] = '\n';
    big_in[HEAD_TOTO_BUFSIZE + 101] = 'y';
    big_in[HEAD_TOTO_BUFSIZE + 102] = '\n';
    want_len = HEAD_TOTO_BUFSIZE + 101;
    got = run_copy("lines_longline", big_in, HEAD_TOTO_BUFSIZE + 103, 1,
                   '\n', big_out, sizeof big_out);
    assert(got == want_len);
    assert(memcmp(big_out, big_in, want_len) == 0);

    printf("PASS: copy_lines across buffer boundaries\n");
}

void test_copy_lines_run(void)
{
    test_fewer_than_n();
    test_more_than_n();
    test_no_trailing_newline();
    test_zero();
    test_nul_delimiter();
    test_buffer_boundary();
}
