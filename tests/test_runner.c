/*
 * test_runner.c — entry point for build/tests/test_core.
 *
 * Chain of thought:
 *   - Responsibility: run every suite in a fixed order. Any assert()
 *     failure aborts, so `make test` exits non-zero.
 *   - Syscalls: none directly.
 *   - Heap: none.
 *   - Standard: ISO C11.
 */

#include "test_copy_bytes.h"
#include "test_copy_lines.h"
#include "test_count_parse.h"

int main(void)
{
    test_count_parse_run();
    test_copy_lines_run();
    test_copy_bytes_run();
    return 0;
}
