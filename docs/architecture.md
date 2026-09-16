# Architecture (Phase 1)

Crush OS Phase 1 is a **teaching kernel in C**. We write the boot path and every subsystem. We run in **QEMU as a 32-bit i386 guest**.

Success is not “it feels like Linux.” Success is: you can explain and step through boot, output, interrupts, allocation, scheduling, and a toy filesystem because we built the simple version of each.

Phase 2 (a Unix-like OS) is a later design pass. See [os-primer.md](os-primer.md) for what that would add. Do not implement it here by accident.

Locked choices live in [docs/decisions/](decisions/). Principles live in [principles.md](principles.md).

## Picture

```text
+----------------------------------------------------------+
|  QEMU (qemu-system-i386, one CPU, BIOS firmware)         |
+----------------------------------------------------------+
|  Our boot sector (16-bit)                                |
|    load kernel from disk → GDT → protected mode → kmain  |
+----------------------------------------------------------+
|  Crush kernel (32-bit, ring 0, monolithic)               |
|    console | interrupts | mm | threads | toy fs          |
+----------------------------------------------------------+
```

User mode and a `user/` tree appear only when the isolation lab needs them.

## Boot path

We write the boot code. No Limine, no GRUB, no Multiboot as the Phase 1 path.

1. BIOS loads 512 bytes from LBA 0 at `0x7C00`.
2. That sector uses BIOS disk reads (`int 13h`) to load the kernel (typically above 1 MiB).
3. If the kernel outgrows a one-sector loading trick, we add a small **second stage**. We do not add one until load size forces it.
4. We load a GDT, disable interrupts briefly, set `CR0.PE`, far-jump into 32-bit protected mode, set a stack, call `kmain`.

Display: VGA text at `0xB8000`. Debug truth: serial at `0x3F8`, plus QEMU’s GDB stub.

Host tools (assembler, `gcc -m32`, `ld`, `dd`, QEMU) are the course toolchain. They are not the OS.

## Privilege and concurrency

Labs 1–5 run entirely in **ring 0**. The first “programs” are kernel threads or a kernel command loop.

The isolation lab introduces **ring 3**, a handful of syscalls (`write`, `yield`, later `open` / `read`), and one user program. That is enough to feel isolation. It is not a POSIX ABI.

One CPU. No SMP. Scheduling is round-robin once we have threads.

## Modules

Keep these boundaries. A new lab should not rewrite a neighbor.

| Module | Job |
| --- | --- |
| `boot/` | Real-mode load, mode switch, jump to C |
| Console | VGA text, serial, `printk`, `panic` |
| Interrupts | IDT, PIC, timer, keyboard |
| Memory | Physical allocator, then simple 2-level paging, a small kernel heap |
| Threads | Kernel thread control blocks, context switch, scheduler |
| Isolation | Ring 3 entry/exit, a few syscalls |
| Storage | In-memory toy FS first; optional ATA PIO and a flat disk layout later |

Implementations stay lab-sized: bitmap or freelist allocator, not a slab/buddy forest; VGA + serial, not a framebuffer console; homemade FS, not FAT32 or VFS.

## Proposed tree

Documented now. Empty directories wait for the first implementation milestone. This planning pass does not create them.

```text
boot/            boot sector, mode switch, kernel load
kernel/          console, interrupts, mm, threads, toy fs
docs/            primer, architecture, roadmap, decisions, later lab pages
```

No `user/` until the isolation lab needs it.

## Tooling (not architecture, but it shapes the tree)

- Language: C17, freestanding kernel
- Compiler: Fedora `gcc -m32 -ffreestanding` (fallback in [dev-environment.md](dev-environment.md))
- Assembler: GNU `as`
- Build: Makefile, when code exists
- Style: `clang-format` only; see [principles.md](principles.md)

## Non-goals (Phase 1)

Do not sneak these in:

- UEFI, writing firmware, long mode, x86_64
- Limine, GRUB, or Multiboot as a substitute for our boot path
- Microkernel / IPC redesign
- Networking, GUI, USB, audio, GPU, SMP
- POSIX, a Unix shell, ELF userspace as a gate to “done”
- VFS, virtio, FAT32, a custom on-disk FS as a prerequisite to learning files
- Hosted libc inside the kernel
- A second formatter or a heavy `clang-tidy` suite

If a patch’s value is “more like a real Unix,” it belongs in a Phase 2 design, not in the current lab.
