# head-toto

A GNU `head`-compatible command written in C11. It prints the first 10 lines
(or `-n NUM` lines, or `-c NUM` bytes) of each FILE to standard output, and
reads standard input when no FILE is given or FILE is `-`. Data is copied with
a raw `read(2)` / `write(2)` loop and no heap allocation.

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
| `-n NUM`, `--lines=NUM` | print the first NUM lines (default 10) |
| `-c NUM`, `--bytes=NUM` | print the first NUM bytes |
| `-q`, `--quiet`, `--silent` | never print file name headers |
| `-v`, `--verbose` | always print file name headers |
| `-z`, `--zero-terminated` | line delimiter is NUL, not newline |
| `--help`, `--h` | show help and exit |
| `--version`, `--v` | show version and exit |

- With several FILEs, each one is preceded by `==> NAME <==`, with a blank line
  between files. Standard input is shown as `standard input`.
- `-` (or no FILE at all) means standard input.
- The last `-n` / `-c` wins, and the last `-q` / `-v` wins.
- `-v` means verbose, not version. `--help` anywhere on the command line
  (before `--`) wins over `--version`.
- Options can follow FILE names, and `--` ends option parsing.
- NUM is a non-negative decimal integer; very large values mean "everything".

Examples:

```sh
./build/head-toto notes.txt                 # first 10 lines
./build/head-toto -n 3 a.txt b.txt          # 3 lines of each, with headers
./build/head-toto -q -n 3 a.txt b.txt       # same, without headers
./build/head-toto -c 100 data.bin           # first 100 bytes
seq 100 | ./build/head-toto -n 5            # from standard input
```

### Differences from GNU head

- No negative counts (`-n -K`, `-c -K`): rejected as an invalid number.
- No size suffixes (`K`, `M`, `b`, `KiB`, ...): rejected as an invalid number.
- No obsolete `-NUM` form (`head-toto -5`): rejected as an invalid option.
- Long options must be spelled in full (no abbreviations such as `--lin`).
- NUM must be plain digits: a leading space or `+` sign is rejected.
- Counts larger than the biggest supported integer are treated as
  "everything"; GNU head 9.4 rejects them as too large.

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
head-toto: write: <reason>
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
└── tests/               test_runner.c + test_{count,copy}_*.c → build/tests/test_core
```

The unit tests run on both Linux and Windows builds (there is no `SKIP:` path).
