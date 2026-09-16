# Development environment (Fedora)

This page is the on-ramp. You do not need a C background to follow it. Run the commands on your machine. A one-off agent install is not a substitute for this document.

Written for **Fedora 44** (`fc44`) on x86_64, which is David’s host. Later Fedora releases should work with the same package names until a note here says otherwise.

Phase 1 target is a **32-bit guest** in QEMU, not a 32-bit Fedora install. See [0002](decisions/0002-target-qemu-i386.md) and [0005](decisions/0005-32-bit-protected-mode.md).

## What you will have

| Tool | Why |
| --- | --- |
| `gcc`, `as`, `ld` | Compile and link a freestanding 32-bit kernel |
| `glibc-devel.i686`, `libgcc.i686` | Make `gcc -m32` work on a 64-bit host |
| `make` | Build, when a Makefile exists |
| `qemu-system-i386` | The classroom machine |
| `qemu-ui-gtk` | A window for VGA text |
| `gdb` | Attach to QEMU’s gdbstub |
| `clang-format`, `clangd` | Format on save and editor intelligence |

GNU `as` is the assembler. `nasm` is optional and not the project default.

## Install

```bash
sudo dnf install \
  gcc make binutils gdb \
  glibc-devel.i686 libgcc.i686 \
  qemu-system-x86 qemu-ui-gtk \
  clang-tools-extra clangd
```

`qemu-system-x86` is the Fedora package that provides `qemu-system-i386`.

Optional:

- `qemu-kvm` — faster emulation if your user is in the `kvm` group. TCG is enough for a tiny kernel.
- `nasm` — only if you personally prefer it. The repo standard is GNU `as`.

### Do not install for Phase 1

These are useful in other OS projects and wrong here:

- Limine, GRUB as a Crush boot path
- `edk2-ovmf` (UEFI firmware)
- A prebuilt `i686-elf` toolchain (see [fallback](#if-gcc--m32-fails))
- An MCP server that drives QEMU or GDB. Learn `make run` and GDB by hand first.

Your host’s own GRUB (the one that boots Fedora) is unrelated. Leave it alone.

## Verify

Run these. You want a 32-bit object file, a linker that speaks `elf_i386`, and QEMU that starts.

```bash
gcc -m32 -ffreestanding -nostdlib -c -x c /dev/null -o /tmp/crush-m32.o
file /tmp/crush-m32.o
ld -m elf_i386 --version
qemu-system-i386 --version
gdb --version
clang-format --version
clangd --version
```

`file` should report a 32-bit ELF object. You can delete `/tmp/crush-m32.o` afterward.

Fedora’s `ld` uses `--version`. If that flag errors on your build, `ld -m elf_i386 -v` is the same idea.

A windowed QEMU smoke check (Ctrl-C or close the window to quit):

```bash
qemu-system-i386 -nographic -version
```

## Format on save in Cursor

1. Install the packages above so `clangd` and `clang-format` are on `PATH`.
2. Open this repo in Cursor. Workspace settings in [`.vscode/settings.json`](../.vscode/settings.json) turn on format-on-save for C and point the formatter at clangd.
3. Install the **clangd** extension if Cursor asks. Do **not** also install a separate Clang-Format extension. Two formatters will fight.
4. Open any future `*.c` file, save, and watch clang-format apply [`.clang-format`](../.clang-format).

Style rules: [principles.md](principles.md) and [0007](decisions/0007-c17-and-style.md).

From a terminal, once C files exist:

```bash
clang-format -i path/to/file.c
```

A `make fmt` target will wrap that when a Makefile exists. There is no git hook in this phase.

Assembly (`*.S`) is not formatted by clang-format. EditorConfig still enforces UTF-8, LF, and a final newline.

## Compiler flags we will use

When code exists, the kernel build is freestanding, for example:

```text
-std=c17 -m32 -ffreestanding -nostdlib -fno-pic -fno-stack-protector -Wall -Wextra
```

Exact flags belong in the Makefile, not copied from memory. **Do not** add `-Werror` until the first labs compile cleanly.

The compiler is the linter. Do not add a hosted `clang-tidy` config in Phase 1.

## If `gcc -m32` fails

Fedora 44 still ships i686/multilib packages. A later release may drop them. `gcc -m32` can also fail if the host compiler assumes a hosted 32-bit Linux userspace we do not want.

Do **not** switch the kernel to 64-bit to dodge a package problem. Change the toolchain:

1. Try `clang --target=i686-unknown-none-elf -ffreestanding -c` and link with `ld -m elf_i386`.
2. Only then build an `i686-elf` cross compiler, following [GCC Cross-Compiler (OSDev)](https://wiki.osdev.org/GCC_Cross-Compiler).

Building a cross compiler is not the default on-ramp. Record the working command in a new note or ADR if we have to go there.

Note: Fedora dropping **32-bit QEMU host builds** is unrelated. We run 64-bit QEMU that emulates a 32-bit guest. That is the supported shape.

## Editor

Cursor plus `clangd` is enough. No extra C/C++ pack is required if clangd is on `PATH` and the clangd extension is enabled.

## What this page is not

It is not a kernel build guide. Lab 1 will add the run recipe when there is something to run.
