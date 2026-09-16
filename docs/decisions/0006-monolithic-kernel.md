# 0006 — Monolithic kernel

**Status:** accepted

**Context:** Microkernels are a common “cleaner architecture” pitch. They move drivers to user space and replace function calls with IPC.

**Decision:** Phase 1 is a monolithic kernel: console, interrupts, memory, threads, and the toy FS run in ring 0 in one address space. Modules stay orthogonal; they are not processes.

**Why:** A teaching kernel should make the call graph obvious. IPC, capability passing, and user-space drivers are extra machinery before the first `printk`.

**Consequences:** We will not start a microkernel rewrite to look more academic. SOLID applies as small C modules and small ops tables, not as user-space servers. A Phase 2 design may revisit this; Phase 1 will not.
