# 0003 — Teaching kernel, not a Unix-like v1

**Status:** accepted

**Context:** An early plan aimed at a small Unix-like OS (processes, VFS-shaped files, a shell) as the first destination. That is a real goal. It is too much for a first kernel when the author wants to relearn OS ideas.

**Decision:** Phase 1 is a college teaching kernel: simple versions of boot, console, interrupts, memory, kernel threads, isolation, and a toy filesystem. A Unix-like OS is Phase 2, planned separately.

**Why:** One idea per lab. Unix’s process and file model are easier to respect after you have built the mechanisms underneath.

**Consequences:** `fork`, POSIX, libc, ELF-as-gate, VFS, and a Unix shell are out of scope until Phase 2 ADRs exist. Agents must refuse to “just add” them to a Phase 1 lab. xv6 is a teaching analog, not a port target.
