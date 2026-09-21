# Lab 2 — Console

**Idea:** The kernel can talk.

You write the code. This page is the contract: sinks, port map, call graph, and what `printk` must accept. It is **not** a paste-ready `printk`. If you get stuck, we talk. It does not need to be perfect.

Primer: [os-primer.md](../os-primer.md). Architecture: [architecture.md](../architecture.md). Toolchain: [dev-environment.md](../dev-environment.md). Boot path: [01-boot.md](01-boot.md). Principles: [principles.md](../principles.md) (`console_write`, not a VGA poke in every file).

Lab 1 is done. `kmain` writing two VGA cells was a proof. It is not a console.

## What you write first

The first artifact is **VGA as a console**: a cursor, wrap, scroll, and `\n`. Serial is checkpoint 2. `printk` is checkpoint 3.

```text
Checkpoint 1  VGA console; kmain stops poking 0xB8000
Checkpoint 2  Serial at 0x3F8; same bytes on both sinks
Checkpoint 3  printk and panic; they are the only official shout
```

Lab 2 is done when checkpoint 3 works. Stopping after 1 or 2 is expected. If checkpoint 1 fails, do not add UART.

## Files (create when you code)

Do not create empty directories “for later.” Add these when you start a checkpoint. `boot/` and `kernel/entry.S` stay as they are.

| File | Role |
| --- | --- |
| `kernel/io.h` | `inb` / `outb` (inline `asm`). Serial needs it. Later labs reuse it. |
| `kernel/console.h` | `console_write` (and a putc if you want one). The official byte sink. |
| `kernel/console.c` | Cursor, VGA cells, wrap, scroll. Fans out to serial once checkpoint 2 exists. |
| `kernel/serial.c` | UART init and putc at `0x3F8`. Polling only. |
| `kernel/printk.c` | `printk` and `panic`. |
| `kernel/kmain.c` | Proof calls only. No raw `vga[i] = …`. |

C is C17, freestanding, `-m32`. Format `*.c` / `*.h` with clang-format. Leave `*.S` alone.

Two call sites (VGA and serial) are not a device framework. No ops table.

```text
kmain / later labs
        |
     printk / panic
        |
   console_write
      /        \
   VGA          serial 0x3F8
```

Higher layers call `console_write`. They do not include a VGA pointer.

## Memory and ports

```text
0xB8000   VGA text  (80 columns × 25 rows)
0x3F8     COM1      (16550-compatible UART)
0x90000   stack     (already set in entry.S)
```

Each VGA cell is still a **byte character** plus a **byte attribute**. Index `row * 80 + col`. Attribute `0x0F` is white-on-black unless you have a reason to change it.

Serial is the **debug truth**. VGA is what you see in the QEMU window. After checkpoint 2, a byte that goes to one sink goes to both.

## Checkpoint 1 — VGA is a console

**Done when:** `kmain` prints more than two cells through `console_write` (or `console_putc`). A short line plus a newline is enough. Wrap at column 80. Scroll when you leave row 24. Then hang as you already do.

### Contract

- Keep a **cursor** (`row`, `col`). A write advances it.
- Column 80 wraps to the next row, column 0.
- Past the last row, **scroll**: move rows 1–24 up, clear row 24, put the cursor on row 24.
- Handle **`\n`**. That is the line ending we care about. `\r` may reset the column; tabs are optional (spaces are enough).
- `kmain` no longer assigns `vga[0]`. Lab 1’s **OK** cells may stay on screen until you overwrite them; do not keep that as the official output path.

If checkpoint 1 fails, the window is still your only proof. Fix wrap and scroll before you open a port.

## Checkpoint 2 — Serial works

**Done when:** the same proof string appears in the QEMU window **and** on the host terminal attached to COM1.

### Contract

Still polling. **No UART IRQ.** Interrupts are lab 3.

COM1 base is **`0x3F8`**. Offsets from that base (Intel / 16550, not our invention):

```text
Offset  When DLAB=0              When DLAB=1
0       data (THR / RBR)         divisor latch low
1       interrupt enable         divisor latch high
2       FIFO control
3       line control (bit 7 = DLAB)
4       modem control
5       line status
6       modem status
```

**Line status bit 5** (THR empty) is the wait-before-`out` flag. Do not write the data port until it is set.

Init is **8N1** (8 data bits, no parity, one stop). Pick a baud QEMU accepts (115200 with divisor 1 is enough). Disable the UART’s interrupt-enable register; we are not ready for IRQs. OUT2 in modem control is for the PC IRQ line; leave it off.

`console_write` fans out: VGA then serial, or serial then VGA. Order does not matter. A byte must not appear on only one sink.

### Run (serial on the host)

The window is still VGA. Attach COM1 to stdio:

```bash
qemu-system-i386 -drive format=raw,file=disk.img -serial stdio
```

Close the QEMU window when you are done (the guest is in `hlt`). Ctrl-C on the terminal may kill QEMU; that is acceptable for this lab.

A Makefile `run` target may grow `-serial stdio`. It is not required until you are tired of typing.

## Checkpoint 3 — `printk` and `panic`

**Done when:** `kmain` uses `printk` for the proof and you can force a `panic` (a temporary call is fine). Serial and VGA both show the formatted text. You can explain `printk` → `console_write` → VGA / `0x3F8` without this page.

### Contract

`printk` is a tiny formatter you write. It is **not** libc `printf`.

Required conversions:

```text
%%   a literal %
%s   C string (pointer to 0-terminated bytes)
%c   character
%d   signed 32-bit decimal
%u   unsigned 32-bit decimal
%x   unsigned 32-bit hex (lowercase is fine)
```

No floats. No width, precision, or length modifiers (`%ld`, `%02x`). An unknown specifier may print the `%` and the letter; do not invent a format language.

`panic` prints a message (through `printk` or `console_write`), then **`cli` + `hlt` in a loop**. It does not return. We have no scheduler and no “recover from panic.”

After this checkpoint, **`printk` and `panic` are the only official ways to shout**. Later labs do not grow a third path.

## Build notes

Link the new `.c` files with `entry.o` and `kmain.o`. `entry.o` stays first. `-Ttext 0x10000` does not change.

Format strings live in **`.rodata`**. `objcopy -O binary -j .text` will drop them and `printk` will read garbage. Include `.rodata` (and `.data` if you add initialized globals):

```bash
objcopy -O binary -j .text -j .rodata -j .data kernel.elf kernel.bin
```

If a section is missing, `objcopy` may error; omit only sections you do not have, or keep `-j .text -j .rodata` once strings exist.

The boot sector still reads a **fixed sector count** (16 = 8 KiB to start). When `kernel.bin` grows past that, raise the DAP count. Check `wc -c kernel.bin`. Do not add a second stage for a few extra `printk` bytes.

```bash
# same disk layout as lab 1; add -serial stdio when you hit checkpoint 2
dd if=boot.bin of=disk.img conv=notrunc
dd if=kernel.bin of=disk.img bs=512 seek=1 conv=notrunc
qemu-system-i386 -drive format=raw,file=disk.img -serial stdio
```

## Out of scope

Framebuffer, GUI, ANSI codes, a Unix TTY, libc `printf`, keyboard input, UART IRQs, an ops table of console devices, paging, a second-stage loader.

## If something is wrong

- **Window shows the string, terminal does not:** `-serial stdio` is missing, UART was not inited, or you wrote VGA only. Check line-status wait and base `0x3F8`.
- **Terminal shows the string, window does not:** `console_write` skipped VGA, or the cursor is off the visible page.
- **`printk` prints garbage for `%s`:** `.rodata` was stripped. Check `objcopy -j`.
- **Triple fault after adding C files:** `kernel.bin` grew past the DAP read; bytes at `0x10000` are incomplete. `wc` and raise the sector count.
- **C ran but garbage on VGA:** `DS` is still wrong. That is a lab 1 regression; load `0x10` in `entry.S` before you touch globals or `0xB8000`.

When you can walk checkpoint 3 out loud, lab 2 is done. Lab 3 is interrupts.
