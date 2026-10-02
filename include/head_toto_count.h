#ifndef HEAD_TOTO_COUNT_H
#define HEAD_TOTO_COUNT_H

#include <stdint.h>

enum head_toto_count_result {
    HEAD_TOTO_COUNT_OK,
    HEAD_TOTO_COUNT_INVALID
};

/*
 * Parse a NUM operand of -n / -c.
 *
 * Preconditions:  s != NULL, out != NULL.
 * Postconditions: on HEAD_TOTO_COUNT_OK, *out holds the value, clamped to
 *                 UINTMAX_MAX on overflow; on HEAD_TOTO_COUNT_INVALID,
 *                 *out is untouched.
 * Accepts one or more ASCII digits only: no sign, whitespace, or suffix.
 */
enum head_toto_count_result head_toto_parse_count(const char *s,
                                                  uintmax_t *out);

#endif
