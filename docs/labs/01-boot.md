# Lab 1 — Boot

**Idea:** The CPU starts in real mode. We load a flat kernel, enter 32-bit protected mode, and run C.

You write the code. This page is the contract: memory map, disk layout, instruction order, and BIOS/Intel structures. It is **not** a paste-ready `boot.S`. If you get stuck, we talk. It does not need to be perfect.

Primer: [os-primer.md](../os-primer.md). Architecture: [architecture.md](../architecture.md). Toolchain: [dev-environment.md](../dev-environment.md). Load address: [0008](../decisions/0008-flat-kernel-at-64kib.md).

## What you write first

The first artifact is a **valid 512-byte boot sector**. The GDT is checkpoint 3.

```text
Checkpoint 1  BIOS loads us; we print in real mode
Checkpoint 2  int 13h reads a flat kernel off disk
Checkpoint 3  GDT, CR0.PE, far jump, kmain writes VGA
```

Lab 1 is done when checkpoint 3 works. Stopping after 1 or 2 is expected. If checkpoint 1 fails, do not add disk reads.

## Files (create when you code)

Do not create empty directories “for later.” Add these when you start a checkpoint.

| File | Role |
| --- | --- |
| `boot/boot.S` | `.code16` only. Prove, load, `lgdt`, switch, far jump to `0x10000`. **Exactly 512 bytes.** |
| `kernel/entry.S` | `.code32`. First bytes of the loaded blob: data selectors `0x10`, stack, `call kmain`. |
| `kernel/kmain.c` | Write a few VGA cells at `0xB8000` so you can see C ran. |

Assembler is GNU `as`, not NASM. C is C17, freestanding, `-m32`. Format `kmain.c` with clang-format. Leave `*.S` alone.

## Memory and disk

```text
Disk                     RAM
LBA 0   boot.bin         0x7C00   boot sector (BIOS put it here)
LBA 1+  kernel.bin       0x10000  flat kernel (we put it here)
                         0x90000  suggested stack (set in entry.S)
                         0xB8000  VGA text (80×25)
```

No second stage. Sector 0 is the boot program. Sectors 1 and up are the kernel blob. Use a **fixed sector count** in the boot sector (16 sectors = 8 KiB is enough to start). Raise the number when the blob grows.

The kernel is a **flat binary**, not ELF. `objcopy -O binary` is a host tool. The boot sector does not parse headers.

We load at **`0x10000` (64 KiB)**, not 1 MiB. BIOS disk reads take a real-mode `ES:BX`. That cannot cleanly target addresses above 1 MiB. When the kernel no longer fits below 640 KiB, we copy up after protected mode. We do not do unreal mode.

## Checkpoint 1 — BIOS ran our sector

**Done when:** `boot.bin` is 512 bytes, ends with `0x55 0xAA`, and QEMU shows a character you wrote. Then `cli` + `hlt` in a loop. Hang is correct. We have no scheduler yet.

### Contract

- BIOS loads LBA 0 to **`0x7C00`** and jumps there in **16-bit real mode**.
- Tell the assembler the origin is `0x7C00` (GNU `as`: `.org` does not relocate like NASM’s `org`; use `ld -Ttext 0x7C00` so symbols match).
- Set segment registers to a known value (usually `0`). You do not inherit a trustworthy `DS`.
- Give yourself a tiny stack (for example `SS=0`, `SP` just below `0x7C00`).
- Prove you ran: write one cell of VGA text. Linear address `0xB8000` is real-mode `ES=0xB800`, offset `0`. Each cell is a **byte character** plus a **byte attribute** (color). `int 10h` (teletype) is an acceptable first try; the better lesson is the memory write.
- Pad the file to 510 bytes of payload, then the signature words **`0x55 0xAA`** at offsets `0x1FE` and `0x1FF`. The CPU starts at byte 0; the signature is last.

```text
Offset  0x000   16-bit instructions and data
        ...
        0x1FE   0x55
        0x1FF   0xAA
```

### Build and run (sector only)

Adjust symbol names to match yours. The shape is what matters.

```bash
as --32 -o boot.o boot/boot.S
ld -m elf_i386 -Ttext 0x7C00 -e start -o boot.elf boot.o
objcopy -O binary boot.elf boot.bin
wc -c boot.bin          # must be 512
xxd boot.bin | tail -n 1
```

`start` is whatever label you put on the first instruction (`-e` must match). If `ld` complains about `_start`, rename the label or the flag.

Make a disk image that is at least one sector, then boot it. **No** `-kernel`, **no** GRUB, **no** OVMF.

```bash
dd if=/dev/zero of=disk.img bs=512 count=32
dd if=boot.bin of=disk.img conv=notrunc
qemu-system-i386 -drive format=raw,file=disk.img
```

Close the QEMU window when you are done (the guest is in `hlt`).

## Checkpoint 2 — Kernel bytes are in RAM

**Done when:** after the BIOS read, known bytes at `0x10000` match what you put on the disk. A dummy pattern (for example sixteen `0xA5` bytes) is enough. QEMU monitor `xp /16bx 0x10000` or a real-mode print of the first byte both count.

### Contract

Still in real mode. BIOS disk services still work. **`DL` is the boot drive** — BIOS leaves it there; do not invent `0x80` unless you have to.

Use **Extended Read**: `int 13h`, `AH=0x42`. The operand is a **disk address packet** (DAP) in memory:

```text
Offset  Size  Field
0       1     packet size (16)
1       1     reserved (0)
2       2     number of sectors to read (16 to start)
4       2     destination offset  (BX part)
6       2     destination segment (ES part)
8       8     starting LBA (1 = first kernel sector)
```

Linear destination `0x10000` is segment `0x1000`, offset `0` (`segment * 16 + offset`).

On success, `CF` is clear. On failure, `CF` is set; print something and halt. Do not continue to the GDT.

Carry is the BIOS error flag. Check it.

You can keep using a dummy `kernel.bin` until checkpoint 3. Concatenate it at LBA 1 the same way you will for the real kernel.

## Checkpoint 3 — Protected mode and `kmain`

**Done when:** VGA text is clearly from C (a short string). You can explain `lgdt` → `CR0.PE` → far jump → stack → `kmain` without this page.

### Order (do not invert)

1. Kernel already at `0x10000` (checkpoint 2).
2. Enable **A20** (port `0x92` is enough on QEMU). At `0x10000` you may not notice if you skip it; enable it anyway so the wrap-at-1-MiB bug is not waiting for you.
3. Build the **GDT** in the boot sector (three 8-byte entries).
4. **`lgdt`** a 6-byte record: 16-bit limit (`3 * 8 - 1` = 23), 32-bit linear address of entry 0.
5. **`cli`**. We have no IDT yet. An interrupt here is a panic you cannot handle.
6. Set **`CR0.PE`**.
7. **Far jump** to selector `0x08`, offset `0x10000`. That loads a 32-bit `CS` and starts `kernel/entry.S`. Until this jump, you are not really in protected mode.

The 32-bit stub does **not** live in the 512-byte sector. That keeps the sector small.

### GDT (Intel layout, not our invention)

```text
Index  Selector  Meaning
0      0x00      null (all zeros)
1      0x08      kernel code: base 0, limit 4 GiB, executable, ring 0, 32-bit
2      0x10      kernel data: base 0, limit 4 GiB, writable, ring 0, 32-bit
```

Usual flat encodings (byte 0 is the first byte in memory):

```text
null  00 00 00 00 00 00 00 00
code  ff ff 00 00 00 9a cf 00
data  ff ff 00 00 00 92 cf 00
```

`lgdt` operand:

```text
+--------+---------------------------+
| limit  | base (linear address)     |
| 23     | address of the null entry |
+--------+---------------------------+
```

After the far jump, `entry.S` loads `0x10` into `DS`, `ES`, `SS`, `FS`, `GS`, sets `ESP` to `0x90000` (or another unused region below 640 KiB), then `call kmain`. If `kmain` returns, `cli` + `hlt`.

`kmain` writes to `(volatile unsigned short *)0xB8000`. Attribute `0x0F` is white-on-black. This is a proof, not `printk`. Serial and a real console are lab 2.

### Build the kernel blob and the full image

```bash
as --32 -o entry.o kernel/entry.S
gcc -m32 -std=c17 -ffreestanding -fno-pic -fno-stack-protector \
    -Wall -Wextra -c -o kmain.o kernel/kmain.c
ld -m elf_i386 -Ttext 0x10000 -e start -o kernel.elf entry.o kmain.o
objcopy -O binary kernel.elf kernel.bin
```

`-e start` must match the first label in `entry.S`. `-Ttext 0x10000` means that label is at the address the far jump uses.

```bash
dd if=/dev/zero of=disk.img bs=512 count=32
dd if=boot.bin of=disk.img conv=notrunc
dd if=kernel.bin of=disk.img bs=512 seek=1 conv=notrunc
qemu-system-i386 -drive format=raw,file=disk.img
```

A tiny Makefile is allowed when you are tired of typing. It is not required.

## Out of scope

Serial, `printk`, paging, interrupts, ELF, Limine, GRUB, Multiboot, UEFI, long mode, a second-stage loader, unreal mode, loading at 1 MiB.

## If something is wrong

- **Black screen, no hang you understand:** the signature is wrong or the file is not 512 bytes. Check `wc` and `xxd` before blaming QEMU.
- **Triple fault (QEMU resets or dies):** protected-mode jump with a bad GDT, wrong far-jump selector, or `CS` not 32-bit. Finish checkpoint 2 first.
- **`int 13h` fails:** packet size not 16, LBA still 0, or `ES:BX` not `0x1000:0`.
- **C ran but garbage:** `DS` still a real-mode value; load `0x10` in `entry.S` before you touch C globals or VGA through a linear pointer.

When you can walk checkpoint 3 out loud, lab 1 is done. Lab 2 is the console.
