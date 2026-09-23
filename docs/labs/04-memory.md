# Lab 4 — Memory

**Idea:** We own RAM, then we invent addresses.

You write the code. This page is the contract: which frames exist, how a 32-bit page walk is laid out, and what `kmalloc` is allowed to do. It is **not** a paste-ready `pmm.c`. If you get stuck, we talk. It does not need to be perfect.

Primer: [os-primer.md](../os-primer.md). Architecture: [architecture.md](../architecture.md). Toolchain: [dev-environment.md](../dev-environment.md). Boot map: [01-boot.md](01-boot.md). Interrupts: [03-interrupts.md](03-interrupts.md). Width: [0005](../decisions/0005-32-bit-protected-mode.md). Load address: [0008](../decisions/0008-flat-kernel-at-64kib.md).

Lab 3 is done. `printk` and `panic` are still the only official ways to shout. A page fault uses those. It does not poke `0xB8000`.

## What you write first

The first artifact is a **physical frame allocator**. Paging is checkpoint 2. The heap is checkpoint 3.

```text
Checkpoint 1  Bitmap of 4 KiB frames; alloc and free show up in printk
Checkpoint 2  One page directory, one page table, identity map, then CR0.PG
Checkpoint 3  Bump kmalloc, backed by frames from checkpoint 1
```

Lab 4 is done when checkpoint 3 works. Stopping after 1 or 2 is expected. If checkpoint 1 fails, do not set `CR0.PG`.

## Files (create when you code)

Do not create empty directories “for later.” Add these when you start a checkpoint. `boot/` stays as it is. Console, serial, and the interrupt path stay as they are.

| File | Role |
| --- | --- |
| `kernel/pmm.h`, `kernel/pmm.c` | Bitmap. `pmm_init`, `alloc_frame`, `free_frame`. |
| `kernel/paging.h`, `kernel/paging.c` | Static directory and table. Load `CR3`, set `CR0.PG`. |
| `kernel/kmalloc.h`, `kernel/kmalloc.c` | Bump `kmalloc`. No `kfree`. |
| `kernel/kmain.c` | Call the new inits after Lab 3’s init and **before** `sti`. Proof `printk`s only. |
| `kernel/idt.c` | Vector 14 prints `CR2`, then `panic`. |

C is C17, freestanding, `-m32`. Format `*.c` / `*.h` with clang-format. Leave `*.S` alone.

No buddy allocator, no slab, no ops table of heaps. The bitmap is the physical allocator. The bump pointer is the heap. They are not the same module.

```text
kmalloc
   |
alloc_frame  -->  bitmap over 0x100000..0x400000
   |
identity map (virtual == physical) so the pointer is usable
   |
printk / panic
```

## Memory map

```text
0x00000000  first 1 MiB: kernel, stack, VGA, BIOS holes
0x00010000  flat kernel (lab 1 load address)
0x00090000  stack, set in entry.S, grows down
0x000B8000  VGA text
0x00100000  first frame you may hand out
0x00400000  end of the pool, and the first unmapped byte
```

The pool is **`0x100000` through `0x400000`**, exclusive of the end. That is 3 MiB, **768 frames** of 4 KiB. The bitmap is 768 bits (**96 bytes**). It does not cover the kernel, the stack, or VGA. Those stay where lab 1 put them.

QEMU’s default RAM is 128 MiB, so this window exists. Do not pass a tiny `-m`. Do not parse the BIOS E820 map. A linker script is the next step only if the kernel image grows into the pool. Until then, `-Ttext 0x10000` stays.

Nothing zeroes `.bss`. `pmm_init` clears the bitmap. `paging_init` fills every directory and table entry it relies on. A static array you did not write is garbage.

## Checkpoint 1 — Physical frames

**Done when:** `alloc_frame` and `free_frame` hand out 4 KiB frames inside the pool, and a short `printk` shows the addresses. Paging is still off.

### Contract

A frame is 4096 bytes. The first free frame is **`0x100000`**. Search from the **lowest index**, so the order is deterministic.

```text
index = (addr - 0x100000) >> 12
addr  = 0x100000 + (index << 12)
```

One bit per frame. **0 means free. 1 means allocated.** `pmm_init` sets every bit to 0.

`alloc_frame` returns the physical address as a `uint32_t`. It sets the bit. If the pool is empty, `panic`. Do not return 0 and hope the caller checks.

`free_frame` clears the bit. `panic` if the address is outside the pool, is not 4 KiB aligned, or was already free.

Proof, in this order, before you enable paging:

- Three allocs return `0x100000`, `0x101000`, `0x102000`.
- Free the middle one.
- The next alloc returns `0x101000` again.

That is the whole checkpoint. Do not allocate page tables from this bitmap.

## Checkpoint 2 — Identity paging

**Done when:** `CR0.PG` is set, `printk` still works, and one deliberate load of an unmapped address panics with vector **14** and `CR2` equal to that address. Then take the test load out. A panic that never returns will hide the heap and the timer.

### Contract

32-bit paging is two levels. No 4 MiB pages. The page-size bit in a directory entry stays **0**.

```text
linear address (32 bits)
+----------+----------+----------+
| dir 10   | table 10 | off 12   |
+----------+----------+----------+
     |          |          |
     |          |          +-- byte inside the 4 KiB frame
     |          +-- which of 1024 page-table entries
     +-- which of 1024 directory entries
```

Each entry is 4 bytes. A directory and a table are each **1024 entries, 4 KiB, aligned to 4 KiB**. Put them in static storage in the kernel. Do not `alloc_frame` them. They already sit inside the first 4 MiB, which you are about to map.

```text
31                          12 11        0
+-----------------------------+----------+
| frame number (physical)     | flags    |
+-----------------------------+----------+
```

Flags you set: **bit 0 present**, **bit 1 read/write**. Leave user (bit 2) clear. Supervisor is enough while we stay in ring 0. A useful value is `address | 0x3`.

**Identity-map the first 4 MiB** and nothing else. One page table holds 1024 pages, which is exactly 4 MiB. Point directory entry 0 at that table. Leave the other 1023 directory entries not present.

That window already contains the kernel, the stack, VGA, and the frame pool. Virtual address equals physical address, so pointers you already have keep working. `0x400000` is the first byte with no page.

Enable paging only after the table is filled:

1. Write `CR3` with the physical address of the directory. The low 12 bits of `CR3` are not the address. The directory is aligned, so the pointer itself is the value.
2. Set **`CR0.PG` (bit 31, `0x80000000`)** without clearing **`CR0.PE` (bit 0)**. Protected mode stays on.
3. A near jump right after the write. Old CPUs fetch the next instruction before the new translation is live.

Do this **before** `sti`. The timer and the keyboard keep using the same addresses once the map is up.

Vector **14** is a page fault. The CPU pushes an error code. Lab 3’s stub already does that for vector 14. Read **`CR2`** in the C handler. `printk` the vector, the error code, and `CR2`, then `panic`. Do not `iret` back to the faulting instruction. There is no demand paging.

The test load can be a byte at `0x400000`. Expect the panic to name that address. Comment the load out once you have seen it.

## Checkpoint 3 — Bump heap

**Done when:** `kmalloc` returns two non-overlapping buffers, you write different bytes into each, and `printk` shows both. Frames come from `alloc_frame`. The guest keeps running.

### Contract

`void *kmalloc(size_t n)`. No `kfree`. Freeing a **frame** is checkpoint 1. Freeing an arbitrary heap object is a second idea.

- `n == 0` panics.
- Round the bump up to **8 bytes** before handing out the address.
- When the next object does not fit in the current frame, `alloc_frame` and keep going.
- If `alloc_frame` panics, the heap is out of memory. That is the policy.
- Call `kmalloc` only after `paging_init`. The identity map makes the physical address a pointer you can store through.

`kmain` order: Lab 3 init (serial, IDT, PIC, PIT, keyboard), then `pmm_init`, `paging_init`, the proof allocs, then **`sti`**, then the `hlt` loop. The test page fault from checkpoint 2 is not part of this loop.

## Build notes

Link the new objects with the kernel. **`entry.o` stays first** on the `ld` line. `-Ttext 0x10000` does not change.

```text
ld ... -e start -o kernel.elf entry.o ... pmm.o paging.o kmalloc.o ...
```

`objcopy` still needs `-j .text -j .rodata` (and `-j .data` if you have it). The bitmap, the page directory, and the page table live in `.bss`. `.bss` is not in the flat file. Filling them at runtime is the point of `pmm_init` and `paging_init`.

The boot sector still reads a **fixed sector count**. When `kernel.bin` grows past that, raise the DAP count. Check `wc -c kernel.bin`. Do not add a second stage for a bitmap.

Run the same way as lab 3 (`-serial stdio`).

## Out of scope

BIOS E820, a linker script, moving the kernel to 1 MiB, a freelist `malloc`, `kfree`, coalescing, buddy, slab, 4 MiB pages, PAE, long mode, a higher-half kernel, user page tables, ring 3, guard pages, demand paging, swap, the recursive page-directory trick.

## If something is wrong

- **QEMU resets as soon as you set `CR0.PG`:** the directory is not 4 KiB aligned, directory entry 0 is not present, or the instruction after the write is not inside the identity map. The near jump has to land on a mapped page. You already live in the first 4 MiB if the map is the one above.
- **`printk` dies after paging, but you did not fault on purpose:** VGA `0xB8000` is missing from the map, or `CR3` has junk in the low 12 bits.
- **First alloc is not `0x100000`:** the pool base is wrong, or the search does not start at index 0.
- **Two allocs return the same address:** the bit was not set.
- **Free then alloc does not reuse that frame:** the free cleared the wrong bit, or the search skips clear bits.
- **An unmapped load resets QEMU instead of panicking:** vector 14’s gate is empty, or the stub does not match the “CPU pushed an error code” frame. Lab 3 already lists 14 as an error-code vector.
- **Panic says vector 14 but the address is missing or wrong:** you did not read `CR2`, or you read it after something else faulted.
- **The two `kmalloc` buffers overlap:** the bump did not advance, or the 8-byte round went backwards.
- **`-m` too small and the first alloc panics:** the pool is not in the guest. Use QEMU’s default memory.

When you can walk checkpoint 3 out loud, lab 4 is done. Lab 5 is kernel threads.
