#ifndef __V7_INCLUDE_SETJMP_H_20201218
#define __V7_INCLUDE_SETJMP_H_20201218

typedef struct jmp_buf_t { unsigned int regs[11]; } jmp_buf[1];

extern int setjmp(jmp_buf b);
extern void longjmp(jmp_buf b, int r);

#endif
