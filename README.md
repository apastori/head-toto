# head-toto

A GNU `head`-compatible command written in C11 (it matches GNU coreutils
`head` 9.4). It prints the first 10 lines (or `-n NUM` lines, or `-c NUM`
bytes, or everything but the last NUM with `-n -NUM` / `-c -NUM`) of each FILE
to standard output, and reads standard input when no FILE is given or FILE is
`-`. Data is copied with a raw `read(2)` / `write(2)` loop; the heap is used
only to hold back the end of a pipe for negative counts.

Versions:

- **v1.0.0** (git tag): POSIX-style subset — plain decimal NUM, no negative
  counts, no suffixes, long options spelled in full.
- **v2.0.0** (current): GNU-compatible.

## Build

```sh
make            # build/head-toto
make debug      # build/head-toto-debug
make test       # builds and runs build/tests/test_core
make clean      # removes build artefacts, keeps build/.gitkeep and build/tests/.gitkeep
make install    # optional: installs build/head-toto into /usr/local/bin
```

All artefacts go under `build/` (`build/head-toto`, `build/head-toto-debug`,
`build/tests/test_core`). The empty `build/.gitkeep` and `build/tests/.gitkeep`
files keep those directories in git.

### Linux / WSL

```sh
make clean && make && make test
./build/head-toto README.md
```

### Windows

Use Git Bash or the MSYS2 **UCRT64** shell, with the UCRT64 toolchain installed:

```sh
pacman -S make mingw-w64-ucrt-x86_64-gcc   # once, from an MSYS2 shell
export PATH="/c/msys64/ucrt64/bin:$PATH"   # Git Bash: UCRT64 tools first
make clean && make && make test
./build/head-toto README.md
```

The Makefile prefers `C:/msys64/ucrt64/bin/gcc` when it exists. It then runs a
small preprocessor probe for `<io.h>` with `_O_BINARY`:

- **Probe succeeds** (native Windows CRT): stdin, stdout and every FILE are
  switched to binary mode, so `-c` byte counts and CRLF line endings pass
  through unchanged.
- **Probe fails** (Linux, WSL, Cygwin/MSYS runtimes): plain POSIX I/O.

On Windows, `make debug` builds without `-fsanitize`, because UCRT64 gcc has no
AddressSanitizer runtime.

The result is a native executable (`build/head-toto.exe`) that runs from UCRT64,
Git Bash, cmd, and PowerShell.

## Usage

```
head-toto [OPTION]... [FILE]...
```

| Option | Meaning |
|--------|---------|
| `-n [-]NUM`, `--lines=[-]NUM` | print the first NUM lines (default 10); with a leading `-`, all but the last NUM lines |
| `-c [-]NUM`, `--bytes=[-]NUM` | print the first NUM bytes; with a leading `-`, all but the last NUM bytes |
| `-q`, `--quiet`, `--silent` | never print file name headers |
| `-v`, `--verbose` | always print file name headers |
| `-z`, `--zero-terminated` | line delimiter is NUL, not newline |
| `--help` | show help and exit |
| `--version` | show version and exit |

NUM is a decimal number, optionally preceded by spaces or `+`, and optionally
followed by a multiplier suffix:

| Suffix | Value | Suffix | Value |
|--------|-------|--------|-------|
| `b` | 512 | | |
| `kB` | 1000 | `K`, `k`, `KiB` | 1024 |
| `MB` | 1000² | `M`, `m`, `MiB` | 1024² |
| `GB` | 1000³ | `G`, `GiB` | 1024³ |

and so on for `T`, `P`, `E`, `Z`, `Y`, `R`, `Q`. A suffix on its own means 1 of
it (`-c K` is 1024 bytes). Numbers too large to represent are rejected with
"Value too large for defined data type".

- With several FILEs, each one is preceded by `==> NAME <==`, with a blank line
  between files. Standard input is shown as `standard input`.
- `-` (or no FILE at all) means standard input.
- The last `-n` / `-c` wins, and the last `-q` / `-v` wins.
- `-v` means verbose, not version. Long options may be shortened to any unique
  prefix (`--lin=3`, `--h`); `--v` is ambiguous (`--verbose` or `--version`).
- Options are processed in order: the first `--help` or `--version` wins, and
  an invalid option before it is reported instead.
- The obsolete form `-NUM` (with optional letters `b`, `c`, `k`, `l`, `m`, `q`,
  `v`, `z`, e.g. `-5`, `-20c`) is accepted as the **first** argument only.
- Options can follow FILE names (unless `POSIXLY_CORRECT` is set), and `--`
  ends option parsing.
- After `-n NUM` on a regular file, the read position is left just after the
  last printed line, so `{ head-toto -n 1; cat; } < file` prints the whole file.

Examples:

```sh
./build/head-toto notes.txt                 # first 10 lines
./build/head-toto -n 3 a.txt b.txt          # 3 lines of each, with headers
./build/head-toto -q -n 3 a.txt b.txt       # same, without headers
./build/head-toto -c 100 data.bin           # first 100 bytes
./build/head-toto -c 1K data.bin            # first 1024 bytes
./build/head-toto -n -2 notes.txt           # all but the last 2 lines
./build/head-toto -5 notes.txt              # obsolete form of -n 5
seq 100 | ./build/head-toto -n 5            # from standard input
```

### Differences from GNU head

- The program is called `head-toto`; `--version` prints only
  `head-toto <version>`, and the `--help` text is its own.
- A failed write is always reported as
  `error writing 'standard output': <reason>`. GNU head buffers its output and
  reports failures of small outputs as `write error: <reason>` instead.
- GNU's hidden `---presume-input-pipe` option is not supported.
- The `<reason>` part of messages comes from the system C library, so it reads
  differently on Windows.

## Exit codes

| Code | Meaning |
|------|---------|
| `0` | every FILE was copied (or `--help` / `--version`) |
| `1` | usage error, a FILE could not be opened or read, or a write failed |

A FILE that cannot be read is reported on stderr and the remaining FILEs are
still processed. Typical messages:

```
head-toto: cannot open 'missing.txt' for reading: No such file or directory
head-toto: error reading 'somedir': Is a directory
head-toto: invalid number of lines: 'abc'
head-toto: option '--v' is ambiguous; possibilities: '--verbose' '--version'
head-toto: error writing 'standard output': <reason>
```

On Windows a directory cannot be opened as a file, so it is reported as
`cannot open '<dir>' for reading: Permission denied` instead.

## Layout

```
head-toto/
├── LICENSE.txt          GNU GPL v2
├── c_version.txt        C standard, flags, and toolchains
├── Makefile
├── README.md
├── build/
│   ├── .gitkeep
│   └── tests/
│       └── .gitkeep
├── include/             head_toto.h + head_toto_{emit,cli,count,io}.h
├── src/                 main.c + head_toto_{emit,cli,count,io,run}.c
└── tests/               test_runner.c + test_{count,copy,elide}_*.c → build/tests/test_core
```

The unit tests run on both Linux and Windows builds (there is no `SKIP:` path).
