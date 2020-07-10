#ifndef __V7_SYS_PRF_H
#define __V7_SYS_PRF_H

#include "types.h"

extern void panic(char *s) __attribute__((noreturn));
extern void prdev(char *str, dev_t dev);
extern void printf(const char *fmt, ...);

#endif
