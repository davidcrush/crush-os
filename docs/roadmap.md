# Roadmap

Phase 1 is a college lab sequence. Each lab has one idea and a definition of done. Do not start the next lab’s design until the current one is explainable.

Phase 2 is a Unix-like OS. It is not a bonus level inside lab 1. When Phase 1 is done, we plan Phase 2 on purpose.

Principles: [principles.md](principles.md). Design: [architecture.md](architecture.md). Decisions: [decisions/](decisions/).

## Lab 0 — Docs locked

**Idea:** Shared goals, vocabulary, and toolchain before any kernel bytes.

**Done when:**

- This docs set is in the repo and the README is the front door.
- Fedora setup is written ([dev-environment.md](dev-environment.md)).
- Formatting config exists and is the only style tool.

**Status:** docs and formatting are in the repo. Implementation is not part of lab 0.

## Lab 1 — Boot

**Idea:** The CPU starts in real mode. We load our kernel, enter 32-bit protected mode, and run C.

**Design:** [labs/01-boot.md](labs/01-boot.md) (three checkpoints; you write the code).

**Done when:**

- Our boot sector is loaded by BIOS in `qemu-system-i386`.
- The kernel is loaded from disk (second stage only if size forces it).
- `kmain` runs and we can prove it (VGA text is enough).
- A documented QEMU run recipe exists (a Makefile target later).
- The lab page exists under `docs/` (this is the page above).

**Not done if:** we used Limine, GRUB, UEFI, or long mode to skip the lesson.

**Status:** design written. Boot path and `kmain` VGA proof exist.

## Lab 2 — Console

**Idea:** The kernel can talk.

**Design:** [labs/02-console.md](labs/02-console.md) (three checkpoints; you write the code).

**Done when:**

- VGA text (80×25) and serial (`0x3F8`) both work.
- `printk` and `panic` exist and are the only official ways to shout.
- Serial is the debug truth; VGA is what you see in the QEMU window.
- Lab page exists.

**Not done if:** we built a framebuffer console or a GUI.

**Status:** design written. VGA, serial, `printk`, and `panic` exist.

## Lab 3 — Interrupts

**Idea:** Hardware can interrupt us, and we resume.

**Design:** [labs/03-interrupts.md](labs/03-interrupts.md) (three checkpoints; you write the code).

**Done when:**

- IDT and PIC are set up.
- Timer ticks and keyboard input are handled.
- A spurious or unexpected exception panics with enough context to debug.
- Lab page exists.

**Not done if:** we jumped to APIC/IOAPIC or SMP.

**Status:** design written. Code not started.

## Lab 4 — Memory

**Idea:** We own RAM, then we invent addresses.

**Done when:**

- A physical allocator (bitmap or freelist) hands out frames.
- Simple 2-level paging is on.
- A small kernel heap exists (bump or freelist).
- Lab page exists.

**Not done if:** we built a slab/buddy forest or 4-level long-mode paging.

## Lab 5 — Concurrency

**Idea:** More than one kernel thread can share the CPU.

**Done when:**

- Kernel threads have a control block, a stack, and a context switch.
- A round-robin scheduler runs at least two threads.
- The timer (or an explicit yield) is enough to switch.
- Lab page exists.

**Not done if:** we started with user processes or `fork`.

## Lab 6 — Isolation

**Idea:** User code is weaker than the kernel.

**Done when:**

- Ring 3 runs one user program.
- A few syscalls work (`write`, `yield`; `open` / `read` if storage is ready).
- A fault in the user program panics or kills that program without silently corrupting the kernel.
- Lab page exists.

**Not done if:** we required ELF, libc, or POSIX to call the lab finished. The first user program may be a flat binary we load ourselves.

## Lab 7 — Storage

**Idea:** Named bytes exist after a reboot of our thoughts, then maybe after a reboot of the disk.

**Done when:**

- An in-memory toy filesystem can create, read, and write named things.
- Optional follow-on: ATA PIO plus a documented flat on-disk layout.
- Lab page exists.

**Not done if:** we required FAT32, VFS, virtio, or a custom FS before the in-memory tree worked.

## After Phase 1

A separate design pass for a Unix-like Crush OS. New ADRs. New roadmap. Until that pass exists, treat Unix-like work as out of scope.

## How to add a lab later

If we need to split a lab, write the new definition of done here first. Do not grow a lab in place until it is three projects wearing one name.
