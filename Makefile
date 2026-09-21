AS      = as --32
LD      = ld -m elf_i386
OBJCOPY = objcopy
CC      = gcc
CFLAGS  = -m32 -std=c17 -ffreestanding -fno-pic -fno-stack-protector -Wall -Wextra

.PHONY: all run check clean

all: disk.img

boot.bin: boot.elf
	$(OBJCOPY) -O binary -j .text $< $@
	@test `wc -c < $@` -eq 512

boot.elf: boot.o
	$(LD) -Ttext 0x7C00 -e start -o $@ $<

boot.o: boot/boot.S
	$(AS) -o $@ $<

kernel.bin: kernel.elf
	$(OBJCOPY) -O binary -j .text -j .rodata $< $@

kernel.elf: entry.o kmain.o console.o
	$(LD) -Ttext 0x10000 -e start -o $@ entry.o kmain.o console.o

entry.o: kernel/entry.S
	$(AS) -o $@ $<

kmain.o: kernel/kmain.c kernel/console.h
	$(CC) $(CFLAGS) -c -o $@ $<

console.o: kernel/console.c kernel/console.h
	$(CC) $(CFLAGS) -c -o $@ $<

disk.img: boot.bin kernel.bin
	dd if=/dev/zero of=$@ bs=512 count=32 status=none
	dd if=boot.bin of=$@ conv=notrunc status=none
	dd if=kernel.bin of=$@ bs=512 seek=1 conv=notrunc status=none

run: disk.img
	qemu-system-i386 -drive format=raw,file=disk.img

check: boot.bin
	xxd boot.bin | tail -n 1

clean:
	rm -f boot.o boot.elf boot.bin entry.o kmain.o console.o kernel.elf kernel.bin disk.img