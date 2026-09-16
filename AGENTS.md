# Agent guide

Read this before editing Crush OS. David Crush drives design. You implement, you document, and you **push back**.

## Required reading

- [CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md)
- [docs/principles.md](docs/principles.md)
- [docs/architecture.md](docs/architecture.md)
- [docs/roadmap.md](docs/roadmap.md)
- [docs/decisions/](docs/decisions/)

If a request contradicts those, say so and offer the smaller path. Do not silently “upgrade” the project.

## Phase 1 constraints

Phase 1 is a **32-bit teaching kernel** we boot ourselves in `qemu-system-i386`.

Refuse unless David explicitly opens Phase 2 or writes a new ADR:

- Unix-like scope: POSIX, `fork`/`exec` as the first process model, VFS, libc, a Unix shell, ELF as a gate to “done”
- Firmware and width: UEFI, long mode, x86_64 as the Phase 1 guest
- Other people’s boot: Limine, GRUB, Multiboot as a substitute for our boot sector
- Shape: microkernel rewrite, SMP, networking, GUI, USB
- Style engines: Uncrustify, Artistic Style, GNU `indent`, `checkpatch.pl`, hosted `clang-tidy`

Lab-sized implementations only. One lab, one idea. See the roadmap.

## How to work

- Simplest honest solution. Extra abstraction needs a reason in the same change.
- Orthogonal C modules. SOLID is small files and small ops tables, not classes.
- Names and small functions first. Comments explain why and invariants.
- A feature or lab is incomplete without a short `docs/` page.
- Format `*.c` / `*.h` with `clang-format` (repo [`.clang-format`](.clang-format)). Do not format `*.S`.
- Compiler flags: `-Wall -Wextra`. Do not add `-Werror` until the first labs compile cleanly.
- Do not invent `boot/` or `kernel/` trees until an implementation milestone asks for them.

## Voice

Inclusion is non-negotiable. Be explicit. Plain language. No sarcasm as the default. Personality belongs in docs and names; kernel internals stay dry.
