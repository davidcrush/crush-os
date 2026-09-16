# Crush OS

A homegrown teaching kernel in C, built from scratch to relearn how operating systems work.

David Crush is the sole developer. AI agents help write code; David drives design. The social contract is [CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md): everyone is welcome; abusive or discriminatory **behavior** is not.

## What this is

**Phase 1** is a college-style kernel. We write our own BIOS boot sector, enter 32-bit protected mode, and implement simple versions of console, interrupts, memory, kernel threads, isolation, and a toy filesystem. We run in `qemu-system-i386`.

**Phase 2** (later, separate design) is a Unix-like OS. Phase 1 is not a baby Unix. We graduate when the labs have done their job.

There is no kernel code in the tree yet. This pass is the project brain: goals, vocabulary, architecture, and toolchain.

## Read in this order

1. [CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md) — how we treat people
2. [docs/principles.md](docs/principles.md) — how we treat code and scope
3. [docs/os-primer.md](docs/os-primer.md) — OS ideas, mapped to labs
4. [docs/architecture.md](docs/architecture.md) — Phase 1 design
5. [docs/roadmap.md](docs/roadmap.md) — lab sequence and definitions of done
6. [docs/decisions/](docs/decisions/) — locked choices (C, i386, teaching kernel, our boot sector, 32-bit, monolithic, clang-format)
7. [docs/dev-environment.md](docs/dev-environment.md) — Fedora toolchain and format-on-save

Agents start at [AGENTS.md](AGENTS.md).

## Set up the machine

On Fedora, follow [docs/dev-environment.md](docs/dev-environment.md). You will install `gcc` (with 32-bit support), QEMU, GDB, `clang-format`, and `clangd`. Cursor formats C on save.

## License

[MIT](LICENSE). Copyright (c) 2026 David Crush.
