#ifndef HEAD_TOTO_EMIT_H
#define HEAD_TOTO_EMIT_H

/* All functions write a single diagnostic to stderr; none of them exit. */

/* "head-toto: <context>: <strerror(errno)>" */
void head_toto_emit_error(const char *context);

/* "head-toto: cannot open '<name>' for reading: <strerror(errno)>" */
void head_toto_emit_open_error(const char *name);

/* "head-toto: error reading '<name>': <strerror(errno)>" */
void head_toto_emit_read_error(const char *name);

/* "head-toto: invalid number of <what>: '<arg>'" (what = "lines"/"bytes") */
void head_toto_emit_invalid_count(const char *what, const char *arg);

/* "head-toto: invalid option -- '<c>'" + hint line */
void head_toto_emit_invalid_option(char c);

/* "head-toto: unrecognized option '<arg>'" + hint line */
void head_toto_emit_unrecognized_option(const char *arg);

/* "head-toto: option requires an argument -- '<c>'" + hint line */
void head_toto_emit_missing_arg_short(char c);

/* "head-toto: option '<name>' requires an argument" + hint line */
void head_toto_emit_missing_arg_long(const char *name);

#endif
