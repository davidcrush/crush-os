#ifndef _PRINTK_H
#define _PRINTK_H

#include <stdarg.h>

void printk(const char *format, ...);
void panic(const char *msg);

#endif
