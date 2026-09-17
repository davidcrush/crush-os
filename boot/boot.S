.code16

.global start

start:
    # initialize
    cli
    xorw %ax, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %ss
    movw $0x7C00, %sp
    movb %dl, boot_drive

    movb boot_drive, %dl
    movw $dap, %si                # DS:SI -> DAP
    movb $0x42, %ah
    int  $0x13
    jc   disk_error               # CF=1 => failed

ok:
    inb  $0x92, %al
    orb  $2, %al
    andb $0xFE, %al
    outb %al, $0x92
    
    cli
    lgdt gdt_desc

    movl %cr0, %eax
    orl  $0x1, %eax      # set PE
    movl %eax, %cr0

    # far jump to 32-bit code selector:offset = 0x08:0x10000
    .byte 0x66, 0xEA     # ljmp ptr16:32 encoding
    .long 0x00010000
    .word 0x0008

disk_error:
    # mark failure: 'E'
    movw $0xB800, %ax
    movw %ax, %es
    movw $0x0F45, %ax             # 'E' white-on-black
    movw %ax, %es:0

hang:
    cli
    hlt
    jmp hang

boot_drive:
    .byte 0

gdt:
    .byte 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00   # null
    .byte 0xFF,0xFF,0x00,0x00,0x00,0x9A,0xCF,0x00   # code 0x08
    .byte 0xFF,0xFF,0x00,0x00,0x00,0x92,0xCF,0x00   # data 0x10
gdt_end:

gdt_desc:
    .word gdt_end - gdt - 1
    .long gdt

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

