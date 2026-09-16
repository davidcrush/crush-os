# Principles

These rules are how Crush OS stays a teaching project instead of a pile of clever exceptions. Humans and agents follow them. If a change fights a principle, stop and say so.

The social contract is [CODE_OF_CONDUCT.md](../CODE_OF_CONDUCT.md). This page is the engineering contract.

## Simplicity

The best solution is the simplest honest one. Extra layers need a reason in the same change.

Honest means we do not hide hardware behind a fake framework, and we do not copy a blog post we cannot explain. Simple means a reader can hold the idea in their head.

If two designs work, pick the smaller one.

## One lab, one idea

Phase 1 is a college sequence. Each lab teaches one mechanism. A change that smuggles Phase 2 work (POSIX, VFS, ELF userspace, long mode, UEFI) into a Phase 1 lab is out of scope. Refuse it and point at [docs/roadmap.md](roadmap.md).

## Standards

We do not cut corners to look fast.

- One C dialect: C17, freestanding in the kernel. See [docs/decisions/0007-c17-and-style.md](decisions/0007-c17-and-style.md).
- One formatter: `clang-format`. No second style engine.
- One naming scheme, one panic policy, one way to add a lab once code exists.
- Makefile for the build. No CMake or Meson until the kernel has earned a more complex build.

Consistency is a gift to future-you and to any agent onboarding later. Same shape, less noise.

## SOLID, as C modules

We do not do class hierarchies in this kernel.

- **Single job:** one file or module does one thing.
- **Small interfaces:** if two implementations must swap (for example a later disk backend), use a small ops table of function pointers, not a kitchen-sink struct.
- **Depend on the interface:** higher layers call `console_write`, not a VGA port poke scattered everywhere.

If a change wants inheritance, templates, or a plugin framework “for SOLID,” reject it.

## Orthogonal modules

Memory does not own the scheduler. The scheduler does not own the filesystem. A new lab should plug in through a narrow call, not a rewrite of a neighbor.

If you cannot name the boundary, the design is not ready.

## Self-documenting code

Prefer names and small functions over narration.

Comments explain **why** and **invariants** (what must stay true). They do not repeat the next line in English.

If a function needs a paragraph, it is probably two functions.

## Docs ship with the work

A lab or subsystem is incomplete until a short page exists under `docs/` that a new developer or agent can read without reverse-engineering the tree.

Code is the first document. The `docs/` page is the map: purpose, entry points, and what we deliberately left out.

## Every change needs a reason

A change must have **learning value** or **system value**. Scaffolding (a linker script, `panic`, serial) counts as system value. Scope theater does not.

If David asks for something that does not help the current lab or the teaching goal, **push back**. Say why, and offer the smaller path.

Assumptions are not sacred. If the maintainer is wrong, say so.

## Formatting contract

Treat formatting like Go’s `gofmt`. Do not debate it in review.

| Role | Tool |
| --- | --- |
| Formatter | `clang-format` via [`.clang-format`](../.clang-format) |
| Editor | Format on save through clangd ([`.vscode/settings.json`](../.vscode/settings.json)) |
| Indent fallback | [`.editorconfig`](../.editorconfig) |
| Linter | `gcc -Wall -Wextra` (not a heavy `clang-tidy` suite) |

Style: LLVM base, 4-space indent, no tabs, 100-column limit. Format `*.c` and `*.h` only. Leave boot assembly (`*.S`) alone.

Do not add Uncrustify, Artistic Style, GNU `indent`, or Linux `checkpatch.pl`.

Do not enable `-Werror` until the first labs compile cleanly. Do not enable a hosted-C `clang-tidy` config; it will lie about a freestanding kernel.

When a Makefile exists, `make fmt` and `make fmt-check` are the CLI. No git hook in this phase.

## Voice

Personality belongs in docs, names, and a later boot banner. Kernel internals stay dry and consistent. Cute allocators are a distraction.

## Where decisions live

Locked choices are short ADRs under [docs/decisions/](decisions/). If you want to reverse one, write a new ADR. Do not silently contradict the last decision in a drive-by patch.
