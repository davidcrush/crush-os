# 0007 — C17 and clang-format

**Status:** accepted

**Context:** David comes from Go and Python (format on save) and PHP (linters). C has no language-mandated formatter. Multiple style engines exist and they fight.

**Decision:**

- Language dialect: **C17**, freestanding in the kernel.
- Formatter: **clang-format** only, LLVM base, 4-space indent, no tabs, 100-column limit.
- Editor: format on save via **clangd**.
- Linter: **`gcc -Wall -Wextra`**. No heavy `clang-tidy` in Phase 1. No `-Werror` until the first labs compile cleanly.
- Build, when it exists: Makefile. `make fmt` / `make fmt-check` later. No pre-commit hook in this phase.

**Why:** One dialect and one formatter remove review noise. LLVM + 4 spaces matches PHP/Python muscle memory better than Linux-kernel tabs. clangd is already the language server. The compiler is a better freestanding linter than a hosted tidy config.

**Consequences:** Uncrustify, Artistic Style, GNU `indent`, and `checkpatch.pl` are rejected. Assembly (`*.S`) is not run through clang-format. Style arguments in review are out of bounds; change `.clang-format` only with a new ADR.
