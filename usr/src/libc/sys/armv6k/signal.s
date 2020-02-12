.syntax unified
.arm
.section .text

.extern errno


@ typedef void (*sig_t)(void);
@ sig_t signal(int sig, sig_t func);
.global signal
.type signal,%function
signal:
    push    {fp, lr}                                   @ set up our stack frame
    add     fp, sp, #0                                 @ adjust the frame pointer
    sub     sp, sp, #24                                @ make space for the indirect call and some locals
    cmp     r0, #20                                    @ check if the specfied handler is out of range
    bge     2f                                         @ higher or the same, EINVAL
    str     r0, [fp, #-16]                             @ stash the signal number in the indirect args
    mov     r0, r0, lsl #2                             @ turn the signal number into an offset in the tvec and dvec tables
    ldr     r2, =__sig_trap_dest_vector                @ load up the trap dest vector
    add     r2, r2, r0                                 @ now r2 contains the address of the trap dest variable
    str     r2, [fp, #-8]                              @ stash the trap dest address for later use
    ldr     r3, [r2]                                   @ load up the old trap destination
    str     r3, [fp, #-4]                              @ stash the old value for later use
    str     r1, [r2]                                   @ save the new handler to the destination vector
    ldr     r2, =__sig_trap_vector                     @ load up the trap vector
    add     r2, r2, r0                                 @ now r2 contains the address of the trap vector
    str     r2, [fp, #-12]                             @ save the new handler into the indirect args
    mov     r2, #48                                    @ 48 is the syscall number for ssig
    str     r2, [fp, #-20]                             @ save the syscall number into the indirect args
    sub     r2, fp, #20                                @ make r2 point to the indirect args
    str     r2, [fp, #-24]                             @ save the pointer to the indirect args to the first slot
    mov     lr, sp                                     @ the pointer to the indirect args is passed in the link register
    tst     r1, #1                                     @ check of the handler address is odd - sets Z to 1 of the result is 0 (i.e. even)
    beq     42f                                        @ even, skip to the syscall
    str     r1, [fp, #-12]                             @ odd, pass the new handler to the syscall via the indirect structure
42: swi     #0                                         @ call indirect system call
    bcs     1f                                         @ if the syscall returned an error, go to the handler code
    ldr     r0, [fp, #-4]                              @ no error, load the old handler into our return code
    b       4f                                         @ jump to the epilogue, we're done
1:  ldr     r3, [fp, #-8]                              @ load up the trap dest address
    ldr     r2, [fp, #-4]                              @ load the old handler to r3
    str     r2, [r3]                                   @ restore the old handler value
    b       3f                                         @ jump to setting errno and the return value
2:  mov     r0, #22                                    @ EINVAL
3:  ldr     r2, =errno                                 @ load up the address of the errno global
    str     r0, [r2]                                   @ stash the returned errno in the error case
    mov     r0, #-1                                    @ syscalls return -1 for error
4:  sub     sp, fp, #0                                 @ restore our stack pointer to point to our stack frame
    pop     {fp, pc}                                   @ restore caller frame and return
.size signal, . - signal

__sigtramp:
    bic     sp, sp, #7                                 @ align the stack before calling C code (restored via fp, which was set up by the kernel)
    push    {r0, r1, r2, r3}                           @ stash caller-saved registers
    sub     lr, lr, #4                                 @ back up one instruction to get the vector entry that called us
    ldr     r3, =__sig_trap_vector                     @ load up the address of the trap vector
    sub     lr, lr, r3                                 @ get the delta between our return address and the trap vector
    mov     r0, lr, lsr #2                             @ shift right by two to get the index (signal number) in r0
    ldr     r3, =__sig_trap_dest_vector                @ load up the address of the trap handler vector
    add     lr, lr, r3                                 @ lr now points to the trap handler function data address
    ldr     lr, [lr]                                   @ load up the address stored in the trap handler vector
    ldr     r3, [sp, #-4]                              @ restore the r3 value
    blx     lr                                         @ branch off to the signal handler
    pop     {r0, r1, r2, r3}                           @ restore the original caller-saved registers
    sub     sp, fp, #0                                 @ restore our stack pointer to point to our stack frame (set by the kernel!)
    pop     {lr}                                       @ load up the PSR... (was pushed by the kernel)
    msr     cpsr_f, lr                                 @ restore the psr (flags only) - helps when interrupted in a syscall return
    pop     {fp, pc}                                   @ restore caller frame and return


__sig_trap_vector:
    bl      __sigtramp
    bl      __sigtramp
    bl      __sigtramp
    bl      __sigtramp
    bl      __sigtramp
    bl      __sigtramp
    bl      __sigtramp
    bl      __sigtramp
    bl      __sigtramp
    bl      __sigtramp
    bl      __sigtramp
    bl      __sigtramp
    bl      __sigtramp
    bl      __sigtramp
    bl      __sigtramp
    bl      __sigtramp
    bl      __sigtramp
    bl      __sigtramp
    bl      __sigtramp
    bl      __sigtramp


.data
__sig_trap_dest_vector:
    .word   0
    .word   0
    .word   0
    .word   0
    .word   0
    .word   0
    .word   0
    .word   0
    .word   0
    .word   0
    .word   0
    .word   0
    .word   0
    .word   0
    .word   0
    .word   0
    .word   0
    .word   0
    .word   0
    .word   0
