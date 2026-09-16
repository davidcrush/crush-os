# 0004 — We write the boot sector

**Status:** accepted

**Context:** Modern hobby OS guides often start with Limine or GRUB/Multiboot. That skips firmware and mode-switch details. David asked to write the pieces from scratch, in the simplest honest way.

**Decision:** Phase 1 boots via our own BIOS boot sector (and a second stage only if kernel size forces it). No Limine, GRUB, or Multiboot as the boot path.

**Why:** The boot path is the first lab. Using someone else’s protocol would teach their protocol, not the CPU.

**Consequences:** We accept 512-byte real-mode constraints and BIOS `int 13h`. We do not write UEFI. Host tools (`as`, `ld`, `dd`, QEMU) are allowed. If a future lab is blocked only by loader size, add a second stage — do not switch bootloaders to save a weekend.
