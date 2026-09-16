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

    movb boot_drive, %dl
    movw $dap, %si                # DS:SI -> DAP
    movb $0x42, %ah
    int  $0x13
    jc   disk_error               # CF=1 => failed

ok:
    # mark success: 'K'
    movw $0xB800, %ax
    movw %ax, %es
    movw $0x0F4B, %ax             # 'K' white-on-black
    movw %ax, %es:0
    jmp  hang

disk_error:
    # mark failure: 'E'
    movw $0xB800, %ax
    movw %ax, %es
    movw $0x0F45, %ax             # 'E' white-on-black
    movw %ax, %es:0

hang:
    hlt
    jmp hang

boot_drive:
    .byte 0

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

