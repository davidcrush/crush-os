# 0005 — 32-bit protected mode

**Status:** accepted

**Context:** x86_64 long mode and UEFI are what current machines use. They also add paging levels, a wider ABI, and a firmware project’s worth of bring-up.

**Decision:** Phase 1 stays in 32-bit protected mode on i386. Paging, when enabled, is 2-level. We revisit 64-bit in Phase 2.

**Why:** Fewer tables, ordinary 32-bit pointers, and a boot path we can finish. The OS ideas do not require 64-bit.

**Consequences:** No long mode, no 4- or 5-level page walks, no UEFI. If Fedora’s `gcc -m32` breaks, we change the **toolchain** (see [dev-environment.md](../dev-environment.md)), not the guest width, unless a new ADR says otherwise.
