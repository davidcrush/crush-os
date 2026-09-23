#ifndef PRINTK_H
#define PRINTK_H

#include <stdarg.h>

void printk(const char *format, ...);
void panic(const char *msg);

#endif
