/*
 * head_toto_count.c — NUM parsing for -n / -c.
 *
 * Chain of thought:
 *   - Responsibility: turn a NUM string into a uintmax_t.
 *   - strtol/strtoumax are avoided: they accept leading whitespace and a
 *     sign, which head-toto must reject. Digits are accumulated manually.
 *   - Values beyond UINTMAX_MAX clamp instead of failing, matching GNU head
 *     treating huge counts as "everything".
 *   - Syscalls: none.
 *   - Heap: none.
 *   - Standard: ISO C11.
 */

#include "head_toto_count.h"

enum head_toto_count_result head_toto_parse_count(const char *s,
                                                  uintmax_t *out)
{
    uintmax_t value = 0;
    const char *p;

    // If the string is empty then return an invalid count result
    if (*s == '\0') {
        return HEAD_TOTO_COUNT_INVALID;
    }

    // Iterate through the string until the string terminator '\0' is reached
    for (p = s; *p != '\0'; p++) {
        unsigned int digit;

        // If the character is not a digit then return an invalid count result
        if (*p < '0' || *p > '9') {
            return HEAD_TOTO_COUNT_INVALID;
        }

        // Convert the character to a digit
        digit = (unsigned int)(*p - '0');

        // If the value is greater than the maximum value then set the value to the maximum value
        if (value > (UINTMAX_MAX - digit) / 10) {
            value = UINTMAX_MAX;
            continue;
        } 
        
        // If the value is not greater than the maximum value then set the value to the value * 10 + digit
        value = value * 10 + digit;
    }

    *out = value;
    return HEAD_TOTO_COUNT_OK;
}
