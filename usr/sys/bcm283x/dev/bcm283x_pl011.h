#ifndef BCM283X_PL011_H
#define BCM283X_PL011_H

#include "../kstdint.h"
#include "../kstddef.h"

extern void bcm283x_uart_early_init(void);

/* extern void putc(char c); */
extern void puts(const char * str);

extern void printHexByte(unsigned int n);
extern void printHexShort(unsigned int n);
extern void printHexLong(unsigned int n);
extern void printHexLongLong(unsigned long long n);

#endif
