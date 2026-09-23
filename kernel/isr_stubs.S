/* kernel/isr_stubs.S */
.code32
.section .text
.extern isr_dispatch_c   /* C function: void isr_dispatch_c(struct isr_frame* f); */

/* Macros to build stubs */
.macro ISR_NOERR num
.global isr_stub_\num
isr_stub_\num:
    pushl $0          /* dummy error code */
    pushl $\num       /* vector number */
    jmp isr_common
.endm

.macro ISR_ERR num
.global isr_stub_\num
isr_stub_\num:
    pushl $\num       /* vector number (CPU already pushed real error code) */
    jmp isr_common
.endm

.global isr_common
isr_common:
    pusha             /* save general-purpose registers */
    pushl %esp        /* pass pointer to frame */
    call isr_dispatch_c
    addl $4, %esp     /* pop arg */
    popa              /* restore registers */
    addl $8, %esp     /* discard vector + errcode (real or dummy) */
    iret

/* CPU exception vectors 0..31 */
ISR_NOERR 0
ISR_NOERR 1
ISR_NOERR 2
ISR_NOERR 3
ISR_NOERR 4
ISR_NOERR 5
ISR_NOERR 6
ISR_NOERR 7
ISR_ERR   8
ISR_NOERR 9
ISR_ERR   10
ISR_ERR   11
ISR_ERR   12
ISR_ERR   13
ISR_ERR   14
ISR_NOERR 15
ISR_NOERR 16
ISR_ERR   17
ISR_NOERR 18
ISR_NOERR 19
ISR_NOERR 20
ISR_NOERR 21
ISR_NOERR 22
ISR_NOERR 23
ISR_NOERR 24
ISR_NOERR 25
ISR_NOERR 26
ISR_NOERR 27
ISR_NOERR 28
ISR_NOERR 29
ISR_NOERR 30
ISR_NOERR 31
ISR_NOERR 32

/* Default stub for vectors you haven't specialized yet */
.global isr_stub_default
isr_stub_default:
    pushl $0
    pushl $255
    jmp isr_common

/* Table C can index: isr_stub_table[i] */
.global isr_stub_table
isr_stub_table:
    .long isr_stub_0,  isr_stub_1,  isr_stub_2,  isr_stub_3
    .long isr_stub_4,  isr_stub_5,  isr_stub_6,  isr_stub_7
    .long isr_stub_8,  isr_stub_9,  isr_stub_10, isr_stub_11
    .long isr_stub_12, isr_stub_13, isr_stub_14, isr_stub_15
    .long isr_stub_16, isr_stub_17, isr_stub_18, isr_stub_19
    .long isr_stub_20, isr_stub_21, isr_stub_22, isr_stub_23
    .long isr_stub_24, isr_stub_25, isr_stub_26, isr_stub_27
    .long isr_stub_28, isr_stub_29, isr_stub_30, isr_stub_31
    .long isr_stub_32
    .rept 223
        .long isr_stub_default
    .endr
