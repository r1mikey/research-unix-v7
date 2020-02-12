.syntax unified
.arm
.section .text

.extern errno

.global cerror
.type cerror,%function
cerror:
    ldr     r2, =errno      @ load up the address of errno
    str     r0, [r2]        @ save the errno
    mov     r0, #-1         @ set the return to -1
    bx      lr
.size cerror, . - cerror
