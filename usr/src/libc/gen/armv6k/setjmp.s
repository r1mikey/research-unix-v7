.syntax unified
.arm
.section .text


@ typedef struct jmp_buf_t { unsigned int regs[11]; } jmp_buf[1];


@ int setjmp(jmp_buf b)
.global setjmp
.type setjmp,%function
setjmp:
    stmia   r0!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
    str     sp, [r0]
    mov     r0, #0          @ setjmp returns 0
    bx      lr              @ back to the caller
.size setjmp, . - setjmp


@ void longjmp(jmp_buf b, int r)
@ causes the previous setjmp call to return 1, but has no return value of its own
.global longjmp
.type longjmp,%function
longjmp:
    ldmia   r0!, {r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
    ldr     sp, [r0]
    cmp     r1, #0          @ if the retval arg is 0...
    moveq   r0, #1          @ ... then we'll return 1...
    movne   r0, r1          @ ... otherwise return the retval arg
    bx      lr              @ back to the caller
.size longjmp, . - longjmp
