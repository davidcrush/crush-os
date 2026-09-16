# 0008 — Flat kernel at 64 KiB

**Status:** accepted

**Context:** Lab 1 must load a kernel with BIOS `int 13h` from a 512-byte real-mode sector. Two common upgrades (ELF, load at 1 MiB) both fight that constraint.

**Decision:** The Phase 1 kernel image is a **flat binary** (`objcopy -O binary`). The boot sector loads it to **physical `0x10000`**. Disk layout is LBA 0 = boot sector, LBA 1+ = kernel. A fixed sector count in the sector is enough; we bump it when the blob grows. No second stage and no unreal mode until the kernel no longer fits below 640 KiB.

**Why:** ELF would need a parser in 510 bytes of payload. BIOS reads take `ES:BX` and cannot cleanly target addresses above 1 MiB. `0x10000` is a real-mode-legal destination (`ES=0x1000`, offset 0) with room for an early kernel.

**Consequences:** `entry.S` is linked at `0x10000` (`-Ttext 0x10000`). The far jump after `CR0.PE` is selector `0x08`, offset `0x10000`. When we outgrow low memory, we copy after protected mode (or add a second stage) and write a new ADR. We do not switch to Limine or GRUB to dodge this.
