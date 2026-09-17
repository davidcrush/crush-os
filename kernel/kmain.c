void kmain(void) {
    volatile unsigned short *vga = (volatile unsigned short *)0xB8000;
    vga[0] = 0x0F4F; // 'O'
    vga[1] = 0x0F4B; // 'K'
    for (;;) { __asm__ __volatile__("hlt"); }
}