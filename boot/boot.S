.code16

.global start

start:
    # initialize
    cli
    xorw %ax, %ax
    movw %ax, %ds
    movw %ax, %ss
    movw $0x7C00, %sp
    movb %dl, boot_drive


hang:
    hlt
    jmp hang

boot_drive:
    .byte 0

    .fill 510-(.-start), 1, 0
    .byte 0x55, 0xAA

# Disk Address Packet (16 bytes)
dap:
    .byte 0x10                    # packet size
    .byte 0x00                    # reserved
    .word 16                      # sectors to read
    .word 0x0000                  # destination offset
    .word 0x1000                  # destination segment (0x10000 linear)
    .long 1                       # starting LBA (kernel starts at sector 1)
    .long 0                       # high 32 bits of LBA

    .fill 510-(.-start), 1, 0
    .byte 0x55, 0xAA

