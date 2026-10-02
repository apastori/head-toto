/*
 * test_count_parse.c — unit tests for head_toto_parse_count().
 *
 * Chain of thought:
 *   - Valid inputs are plain decimal digits; leading zeros are fine.
 *   - Every rejected form (empty, sign, whitespace, suffix, letters) must
 *     leave *out untouched, so a sentinel value is checked afterwards.
 *   - Overflow clamps to UINTMAX_MAX rather than failing.
 *   - Syscalls: none.
 *   - Heap: none.
 *   - Standard: ISO C11.
 */

#include "test_count_parse.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "head_toto_count.h"

#define SENTINEL ((uintmax_t)12345)

static void expect_ok(const char *s, uintmax_t want)
{
    uintmax_t got = SENTINEL;

    assert(head_toto_parse_count(s, &got) == HEAD_TOTO_COUNT_OK);
    assert(got == want);
}

static void expect_invalid(const char *s)
{
    uintmax_t got = SENTINEL;

    assert(head_toto_parse_count(s, &got) == HEAD_TOTO_COUNT_INVALID);
    assert(got == SENTINEL);
}

void test_count_parse_run(void)
{
    expect_ok("0", 0);
    expect_ok("10", 10);
    expect_ok("007", 7);
    printf("PASS: count_parse valid decimal\n");

    expect_invalid("");
    expect_invalid("-5");
    expect_invalid("+5");
    expect_invalid(" 5");
    expect_invalid("5K");
    expect_invalid("abc");
    printf("PASS: count_parse rejects invalid input\n");

    expect_ok("9999999999999999999999999999999999999999", UINTMAX_MAX);
    printf("PASS: count_parse clamps overflow to UINTMAX_MAX\n");
}
