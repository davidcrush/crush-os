# Operating systems primer

This is a reintroduction, not a textbook. It exists so David (and any agent) can share one vocabulary before we write kernel code.

If a word is new, check the [glossary](#glossary). For what we will actually build, see [architecture](architecture.md) and the [lab roadmap](roadmap.md).

## What an OS is for

An operating system has three jobs, and they pull against each other:

1. **Multiplex.** Many programs share one CPU, one pile of RAM, and a few devices.
2. **Isolate.** A bug in one program should not trash the kernel or another program.
3. **Interact.** Programs still need to cooperate: wait for a child, write to a console, share a file.

Everything else is a mechanism for those three jobs.

You do not need `fork`, POSIX, or a Unix shell to learn this. A teaching kernel can show each mechanism in a simple form.

## How a PC-class machine starts

On our Phase 1 machine the story is:

1. **QEMU** emulates a 32-bit PC (`qemu-system-i386`).
2. **BIOS** (firmware) runs first. It looks at disk LBA 0 and loads 512 bytes into memory at `0x7C00`.
3. Those 512 bytes are **our boot sector**. They are still in 16-bit real mode, where the CPU thinks it is an old 8086.
4. The boot sector loads the kernel from disk, builds a **GDT** (a table that describes memory segments), sets a bit in a control register (`CR0.PE`), and far-jumps into **32-bit protected mode**.
5. It sets a stack and calls **`kmain`** in C. From then on, the kernel is the program that never stops.

Firmware and the boot program are not the OS. They exist to get the kernel running. The kernel then owns interrupts, memory, and devices.

```text
QEMU i386 → BIOS → our boot sector → protected mode → C kernel
```

## Privilege

The CPU can run in a powerful mode or a weaker mode.

- **Kernel / ring 0:** may change page tables, talk to ports, halt the machine.
- **User / ring 3:** may run ordinary instructions. To ask for a service, it performs a **system call** (a controlled trap into the kernel).

Phase 1 stays in the kernel for several labs. User mode is a later lab, when isolation is the lesson. Until then, “programs” can be kernel threads or a kernel command loop.

**Interrupts** are the hardware saying “stop what you are doing.” A timer tick, a key press, and a divide-by-zero all arrive through the **IDT** (interrupt descriptor table). The kernel decides what happens next.

## Memory, in one page

Physical memory is the real RAM chips (as QEMU pretends they exist). The kernel must track which frames are free. That is the **physical allocator**.

**Paging** gives each context its own view of addresses. A virtual address is translated through page tables to a physical frame. Two processes can both think they live at `0x08000000` and not collide.

Phase 1 uses 32-bit paging: a **2-level** page directory and page table. That is simpler than 64-bit’s 4-level walk.

The kernel also needs a **heap** for its own dynamic data: a bump pointer or a freelist is enough at first.

## Threads and processes

A **thread** is a saved CPU context (registers, stack, instruction pointer) the scheduler can switch.

A **process** is usually “a thread plus an address space plus some kernel bookkeeping.” Unix adds `fork`, `exec`, `wait`, and `exit` on top of that.

Phase 1 implements **kernel threads** and a tiny **round-robin** scheduler first. That teaches multiplexing without isolation. User processes come later.

## Files, in the Unix sense (later)

Unix treats many things as files: disks, consoles, pipes. A **file descriptor** is a small integer that names an open thing. **VFS** is a layer that makes different backends look the same.

We do **not** build that in Phase 1. We will write a **toy filesystem** (in memory, then maybe a flat disk image) so the idea of “named bytes on a device” is concrete. The Unix model is [what comes after](#what-unix-like-systems-add-later).

## Kernel shapes

| Shape | Idea | Phase 1 |
| --- | --- | --- |
| **Monolithic** | Drivers and FS run in the kernel. Function calls, not IPC. | Yes. xv6, early Unix, Linux. |
| **Microkernel** | Drivers in user space. Lots of messages. | No. More moving parts. |
| **Hybrid / unikernel** | Extra policy. | No. |

We keep **orthogonal modules** inside one kernel. That is not a microkernel.

## The labs, mapped to ideas

| Lab | Idea you should be able to explain |
| --- | --- |
| Boot | Real mode, disk load, GDT, protected mode, `kmain` |
| Console | VGA text, serial, `printk`, `panic` |
| Interrupts | IDT, PIC, timer, keyboard |
| Memory | Physical frames, then paging |
| Concurrency | Kernel threads, context switch, round-robin |
| Isolation | Ring 3, a few syscalls, one user program |
| Storage | Toy FS, then optional ATA PIO disk |

Exact definitions of done are in [roadmap.md](roadmap.md).

## What Unix-like systems add later

Phase 2 is a separate design pass. This section is orientation, not a backlog for lab 1.

A Unix-like OS typically adds:

- User processes with `fork` / `exec` / `wait` / `exit`
- A stable **syscall ABI** and a small **libc**
- File descriptors, pipes, a directory tree, a **VFS**
- ELF loading and a real shell
- Often 64-bit, UEFI, and a third-party bootloader

Those are good destinations. They are the wrong first mountain. We build the simple version of each idea first.

## Glossary

**ABI**
: Application Binary Interface. How registers, stacks, and syscalls are laid out so two binaries can talk.

**ATA PIO**
: A simple, slow way to read a disk by writing ports and waiting. Honest college disk I/O.

**BIOS**
: Firmware on a classic PC. Loads the first disk sector. Not UEFI.

**Boot sector**
: The first 512 bytes of the disk. BIOS loads it at `0x7C00`. Ours. We write it.

**CR0.PE**
: The CPU bit that turns on protected mode.

**Freestanding**
: C without a hosted libc. No `printf` from the C library unless we write it.

**GDT**
: Global Descriptor Table. Tells the CPU how to interpret segments in protected mode.

**IDT**
: Interrupt Descriptor Table. Where the CPU jumps on interrupts and exceptions.

**i386**
: 32-bit x86. Our Phase 1 guest.

**Initrd**
: An initial ram disk. A Unix-ish way to get files before a real disk driver. Not a Phase 1 requirement.

**LBA**
: Logical Block Address. Disk sector number, starting at 0.

**Long mode**
: 64-bit x86. Phase 2 territory.

**Page / frame**
: A fixed-size chunk of memory, usually 4 KiB. “Page” often means the virtual view; “frame” the physical one. People mix the words.

**PIC**
: Programmable Interrupt Controller. The old PC chip that fans device IRQs into the CPU.

**Protected mode**
: 32-bit x86 mode with privilege rings and (once we enable it) paging.

**QEMU**
: An emulator. Our classroom machine.

**Real mode**
: 16-bit 8086-compatible mode. Where BIOS and the boot sector start.

**Ring 0 / ring 3**
: Kernel privilege vs user privilege.

**Serial (UART)**
: A byte stream, often port `0x3F8`. QEMU can attach it to your terminal. Best debug log.

**Syscall**
: A controlled entry from user code into the kernel.

**UEFI**
: Modern firmware. We do not write it and we do not boot through it in Phase 1.

**VGA text**
: 80×25 characters at physical address `0xB8000`. First display.

**VFS**
: Virtual file system. A Unix layer we are not building yet.
