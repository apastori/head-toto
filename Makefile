# head-toto — GNU head replica in C11.
#
# Targets: all (default), debug, test, clean, install.
# All artefacts go under build/ and build/tests/.

CC := $(shell command -v gcc >/dev/null 2>&1 && echo gcc || echo clang)

# On Windows prefer the MSYS2 UCRT64 gcc, so Git Bash does not pick up a
# minimal or unrelated MinGW toolchain earlier on PATH. gcc there appends
# .exe to its output, so targets carry the suffix to stay up to date.
ifeq ($(OS),Windows_NT)
  EXE := .exe
  ifneq ($(wildcard C:/msys64/ucrt64/bin/gcc.exe),)
    CC := C:/msys64/ucrt64/bin/gcc
  endif
endif

# Platform probe: native Windows CRTs provide <io.h> with _O_BINARY, and
# their descriptors default to text mode (CRLF translation). When the probe
# succeeds, HEAD_TOTO_WIN32_IO switches stdin/stdout/FILEs to binary mode.
# Linux has no <io.h>; Cygwin/MSYS runtimes lack _O_BINARY and are already
# binary, so both keep the plain POSIX path. '\043' is '#', which make would
# otherwise treat as a comment.
WIN32_IO_PROBE := $(shell printf '\043include <io.h>\n\043include <fcntl.h>\n\043ifndef _O_BINARY\n\043error no _O_BINARY\n\043endif\n' | $(CC) -E -x c - >/dev/null 2>&1 && echo yes)

# -Werror: the project policy is zero warnings, and treating them as errors
# keeps that enforced on every compiler and platform instead of letting
# warnings accumulate unnoticed.
CFLAGS_COMMON := -std=c11 -Wall -Wextra -Wpedantic -Werror \
                 -D_POSIX_C_SOURCE=200809L -Iinclude

# UCRT64 gcc ships no AddressSanitizer runtime, so Windows debug builds
# drop the sanitizers and keep only debug info.
ifeq ($(WIN32_IO_PROBE),yes)
  CFLAGS_COMMON += -DHEAD_TOTO_WIN32_IO
  SANITIZE :=
else
  SANITIZE := -fsanitize=address,undefined
endif

CFLAGS       := $(CFLAGS_COMMON) -O2
CFLAGS_DEBUG := $(CFLAGS_COMMON) -g -O1 $(SANITIZE) -fno-omit-frame-pointer
LDFLAGS      :=

BUILD_DIR      := build
TEST_BUILD_DIR := $(BUILD_DIR)/tests

SRCS := src/main.c \
        src/head_toto_emit.c \
        src/head_toto_cli.c \
        src/head_toto_count.c \
        src/head_toto_io.c \
        src/head_toto_run.c

OBJS := $(patsubst src/%.c,$(BUILD_DIR)/%.o,$(SRCS))
HDRS := $(wildcard include/*.h)

LIB_OBJS := $(BUILD_DIR)/head_toto_count.o \
            $(BUILD_DIR)/head_toto_io.o \
            $(BUILD_DIR)/head_toto_emit.o

TEST_SRCS := tests/test_runner.c \
             tests/test_count_parse.c \
             tests/test_copy_lines.c \
             tests/test_copy_bytes.c \
             tests/test_elide_lines.c \
             tests/test_elide_bytes.c
             
TEST_OBJS := $(patsubst tests/%.c,$(TEST_BUILD_DIR)/%.o,$(TEST_SRCS))
TEST_HDRS := $(wildcard tests/*.h)

TARGET       := $(BUILD_DIR)/head-toto$(EXE)
TARGET_DEBUG := $(BUILD_DIR)/head-toto-debug$(EXE)
TEST_CORE    := $(TEST_BUILD_DIR)/test_core$(EXE)

PREFIX ?= /usr/local

.PHONY: all debug test clean install

all: $(TARGET)

$(TARGET): $(OBJS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)

$(BUILD_DIR)/%.o: src/%.c $(HDRS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

# Compiled straight from sources so release objects are never reused with
# debug flags.
debug: $(TARGET_DEBUG)

$(TARGET_DEBUG): $(SRCS) $(HDRS) | $(BUILD_DIR)
	$(CC) $(CFLAGS_DEBUG) -o $@ $(SRCS) $(LDFLAGS)

$(TEST_BUILD_DIR)/%.o: tests/%.c $(HDRS) $(TEST_HDRS) | $(TEST_BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(TEST_CORE): $(TEST_OBJS) $(LIB_OBJS) $(HDRS) $(TEST_HDRS) | $(TEST_BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $(TEST_OBJS) $(LIB_OBJS) $(LDFLAGS)

test: $(TEST_CORE)
	./$(TEST_CORE)

$(BUILD_DIR) $(TEST_BUILD_DIR):
	mkdir -p $@

clean:
	rm -f $(BUILD_DIR)/*.o \
	      $(BUILD_DIR)/head-toto $(BUILD_DIR)/head-toto.exe \
	      $(BUILD_DIR)/head-toto-debug $(BUILD_DIR)/head-toto-debug.exe \
	      $(TEST_BUILD_DIR)/*.o \
	      $(TEST_BUILD_DIR)/test_core $(TEST_BUILD_DIR)/test_core.exe \
	      $(TEST_BUILD_DIR)/fixture_*.tmp

install: $(TARGET)
	install -m 755 $(TARGET) $(PREFIX)/bin
