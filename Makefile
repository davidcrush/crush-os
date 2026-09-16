AS      = as --32
LD      = ld -m elf_i386
OBJCOPY = objcopy

boot.bin: boot.elf
	$(OBJCOPY) -O binary -j .text $< $@
	@test `wc -c < $@` -eq 512

boot.elf: boot.o
	$(LD) -Ttext 0x7C00 -e start -o $@ $<

boot.o: boot/boot.S
	$(AS) -o $@ $<

disk.img: boot.bin kernel.bin
	dd if=/dev/zero of=$@ bs=512 count=32 status=none
	dd if=boot.bin of=$@ conv=notrunc status=none
	dd if=kernel.bin of=$@ bs=512 seek=1 conv=notrunc status=none

.PHONY: run check clean

run: disk.img
	qemu-system-i386 -drive format=raw,file=disk.img

check: boot.bin
	xxd boot.bin | tail -n 1

clean:
	rm -f boot.o boot.elf boot.bin disk.img