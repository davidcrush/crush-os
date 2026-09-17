.code32
.global start
.extern kmain

start:
    movw $0x10, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %ss
    movw %ax, %fs
    movw %ax, %gs

    movl $0x90000, %esp
    call kmain

1:  cli
    hlt
    jmp 1b