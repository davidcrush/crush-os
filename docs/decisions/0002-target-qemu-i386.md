# 0002 — Target is QEMU i386

**Status:** accepted

**Context:** We need a machine we control. Real hardware and firmware bring-up are a different project.

**Decision:** Phase 1 runs in `qemu-system-i386` with BIOS firmware. One virtual CPU. Development host is Fedora (see [dev-environment.md](../dev-environment.md)).

**Why:** An emulator is rewindable. Serial and GDB work without extra cables. A 32-bit guest is the teaching CPU; the host being 64-bit does not change that.

**Consequences:** We do not target real PCs, USB sticks, or “it boots on my laptop” in Phase 1. KVM is optional speed, not a requirement. `qemu-system-x86_64` is not the Phase 1 machine even if it can run 32-bit code.
