# Lab 3 — Interrupts

**Idea:** Hardware can interrupt us, and we resume.

You write the code. This page is the contract: IDT layout, PIC remap, PIT and keyboard ports, and when to `panic`. It is **not** a paste-ready `kernel/isr_stubs.S`. If you get stuck, we talk. It does not need to be perfect.

Primer: [os-primer.md](../os-primer.md). Architecture: [architecture.md](../architecture.md). Toolchain: [dev-environment.md](../dev-environment.md). Boot: [01-boot.md](01-boot.md). Console: [02-console.md](02-console.md).

Lab 2 is done. `printk` and `panic` are the only official ways to shout. An interrupt handler uses those. It does not poke `0xB8000`.

Before you `sti`, take the test `panic` out of `kmain`. A panic that never returns will hide the timer.

## What you write first

The first artifact is an **IDT** and a path from a CPU exception to `panic`. The PIC is checkpoint 2. The keyboard is checkpoint 3.

```text
Checkpoint 1  IDT loaded; an unexpected exception panics with a vector
Checkpoint 2  PIC remapped; PIT IRQ0 ticks; then sti
Checkpoint 3  Keyboard IRQ1; a key shows up through printk
```

Lab 3 is done when checkpoint 3 works. Stopping after 1 or 2 is expected. If checkpoint 1 fails, do not unmask the PIC.

## Files (create when you code)

Do not create empty directories “for later.” Add these when you start a checkpoint. `boot/` stays as it is. Console and serial stay as they are.

| File | Role |
| --- | --- |
| `kernel/idt.h`, `kernel/idt.c` | 256-entry IDT, gate builder, `lidt`. |
| `kernel/isr_stubs.S` | `.code32` stubs. Save state, call C, `iret`. C does not `iret`. |
| `kernel/pic.h`, `kernel/pic.c` | Remap the 8259s, mask, EOI. Uses `io.h`. |
| `kernel/timer.c` | PIT init and the IRQ0 C handler. |
| `kernel/kbd.c` | IRQ1 C handler. Read `0x60`. |
| `kernel/kmain.c` | Init in order, drop the test `panic`, then `sti` and a loop `hlt` can wake from. |

C is C17, freestanding, `-m32`. Format `*.c` / `*.h` with clang-format. Leave `*.S` alone.

No ops table of interrupt sources. UART IRQs stay off. Serial remains polling.

```text
CPU exception / PIC IRQ
        |
      IDT gate  (32-bit interrupt gate, selector 0x08)
        |
   stub in isr_stubs.S   (vector, saved regs, call C, iret)
        |
   C handler
        |
   printk / panic
```

## Memory and ports

```text
IDT      256 × 8 bytes, wherever your C array lives (flat kernel)
0x20     master PIC command; also the EOI value you write there
0x21     master PIC data (mask)
0xA0     slave PIC command
0xA1     slave PIC data (mask)
0x40     PIT channel 0 data
0x43     PIT command
0x60     keyboard data (scancode)
```

After the remap, **IRQ0 is vector `0x20`** and **IRQ1 is vector `0x21`**. CPU exceptions stay at vectors 0–31. That split is why we remap. BIOS left the master PIC on vectors 0–7, which are also divide-by-zero, debug, and so on.

## Checkpoint 1 — IDT and exceptions

**Done when:** `lidt` has run and a deliberate fault (divide by zero is enough) reaches `panic` with the vector number. You have not enabled the PIC and you have not executed `sti`.

### Contract

Build an IDT of **256** 8-byte entries. A shorter table that only covers 0–31 is acceptable if the limit you pass to `lidt` matches what you actually allocated. Unused entries still need a stub that panics; an empty slot is a triple fault.

Each entry is a **32-bit interrupt gate**, ring 0, code selector **`0x08`** (the same flat code segment as the far jump in lab 1). Type byte **`0x8E`** (present, DPL 0, 32-bit interrupt gate). Interrupt gates clear IF on entry, so a handler is not re-entered until `iret`.

`lidt` takes a 6-byte record, same shape as `lgdt`:

```text
+--------+----------------------------------+
| limit  | base                             |
| 2 bytes| 4-byte linear address of entry 0 |
+--------+----------------------------------+
```

Limit is `256 * 8 - 1` = **2047** if you built all 256 entries.

The stub is assembly. On entry the CPU has pushed EFLAGS, CS, and EIP (and an error code for some vectors). Your stub pushes the vector, saves the general registers, calls a C function, restores registers, and **`iret`**.

Some vectors push an error code and some do not. If the stub always assumes one or the other, `iret` returns to garbage and you triple-fault. Vectors that push an error code: **8, 10, 11, 12, 13, 14, 17**. The others do not; the stub pushes a dummy 0 so the C frame is the same shape.

The C handler for anything you do not expect calls **`panic`** with the vector number. EIP in the message is useful and optional. Do not resume from a fault you do not understand.

Stay in `cli` until checkpoint 2. A fault does not need `sti`; the CPU takes the exception anyway.

## Checkpoint 2 — PIC and the timer

**Done when:** the master and slave PICs are remapped, only IRQ0 is unmasked, the PIT fires, the handler sends EOI, and you can see ticks (a `printk` every N ticks is enough). Then **`sti`**. The guest keeps running; it does not reset.

### Contract

Program the PICs **before** `sti`. If you enable interrupts first, a BIOS-routed IRQ lands on an exception vector.

Ports: master command `0x20`, master data `0x21`, slave command `0xA0`, slave data `0xA1`.

Init both chips (this is the 8259 sequence, not our invention):

```text
ICW1   0x11          start init, expect ICW4
ICW2   0x20 master   vector base (slave uses 0x28)
       0x28 slave
ICW3   bit 2 master  slave is wired to IRQ2
       2 slave       cascade identity
ICW4   0x01          8086 mode, not special fully nested, not auto-EOI
```

Write ICW1 to the command port and ICW2–ICW4 to the data port, master then slave (or slave then master; finish one chip’s four bytes before you rely on it).

Then set the masks. **Mask every IRQ except IRQ0** (timer). Bit 0 of the master mask is IRQ0; write 0 to unmask it and 1 to mask. Leave the slave fully masked. Keyboard is checkpoint 3.

**EOI** is not optional. After you handle IRQ0, `outb(0x20, 0x20)` (command port, value `0x20`). IRQ0 is on the master, so you do not EOI the slave. If you skip EOI, that IRQ never fires again.

**PIT** channel 0: data `0x40`, command `0x43`. The clock is about **1193182 Hz**. A divisor near `11932` is about 100 Hz. Any honest divisor is fine; pick one and know what it means. Mode 3 (square wave) or mode 2 (rate generator), low byte then high byte, is enough. You do not need three channels.

The IRQ0 C handler increments a counter and occasionally `printk`s it. Do not `printk` every tick unless you enjoy a flooded serial line. Do not touch VGA cells from the handler.

`kmain` order: IDT, PIC remap and masks, PIT program, **then** `sti`, then a loop:

```text
for (;;) { hlt; }
```

`hlt` sleeps until the next interrupt. That is the resume. `cli` around the whole loop puts you back in lab 2.

## Checkpoint 3 — Keyboard

**Done when:** you unmask IRQ1 and a key you press shows up through `printk` (window and serial). A raw scancode is enough.

### Contract

IRQ1 is vector **`0x21`** after the remap. Clear bit 1 of the master mask. Leave every other IRQ masked.

On the interrupt: read one byte from port **`0x60`**, then EOI the master (`outb(0x20, 0x20)`). The byte is a **scancode**, not ASCII. Break codes have the high bit set; you may ignore those. A tiny table for a few make-codes is allowed. A line editor, Ctrl-C, and “the kernel is a shell” are not.

`printk` the scancode or the character. Still no VGA poke in the handler.

There is no syscall and no ring 3. The key only proves the IRQ path.

## Build notes

Assemble `kernel/isr_stubs.S` with `as --32` and link it with the other kernel objects. **`entry.o` stays first** on the `ld` line so the far jump still lands on `kernel/entry.S`. `-Ttext 0x10000` does not change.

```text
ld ... -e start -o kernel.elf entry.o ... isr_stubs.o idt.o pic.o timer.o kbd.o ...
```

`objcopy` still needs `-j .text -j .rodata` (and `-j .data` if you have it). The IDT array is zeros in `.bss` unless you initialize it; `.bss` is not in the flat file, which is fine if you fill the gates at runtime.

The boot sector still reads a **fixed sector count**. When `kernel.bin` grows past that, raise the DAP count. Check `wc -c kernel.bin`. Do not add a second stage for an IDT.

Run the same way as lab 2 (`-serial stdio`). QEMU’s window must be focused for the keyboard IRQ to see your keys.

## Out of scope

APIC, IOAPIC, SMP, MSI, syscall/sysenter, ring 3, a keyboard line editor, UART IRQs, paging (even though vector 14 exists), task gates, a second IDT per CPU.

## If something is wrong

- **Triple fault as soon as you `sti`:** the PIC is still on vectors 0–7, or the IDT limit/base is wrong. Finish the remap before `sti`. A bad `iret` frame (error code present or missing) looks the same.
- **One tick and then silence:** you handled IRQ0 and did not send EOI.
- **Divide-by-zero resets QEMU:** that vector has no gate, or the stub does not match the “no error code” frame.
- **Panic says vector 0 when you pressed a key:** the PIC was not remapped; IRQ1 arrived as an exception.
- **No key in the log:** IRQ1 is still masked, you did not read `0x60`, or the QEMU window does not have focus.
- **`printk` from the handler never appears:** serial or VGA path regressed. Handlers must go through `printk`, and `serial_init` must already have run.

When you can walk checkpoint 3 out loud, lab 3 is done. Lab 4 is [memory](04-memory.md).
