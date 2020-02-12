.syntax unified
.arm
.section .text

.extern errno


@ XXX: make off_t 64bit in the future?
@ #define SEEK_SET 0 /* set file offset to offset */
@ #define SEEK_CUR 1 /* set file offset to current plus offset */
@ #define SEEK_END 2 /* set file offset to EOF plus offset */
@ off_t lseek(int fildes, off_t offset, int whence);
.global lseek
.type lseek,%function
lseek:
    push    {fp, lr}                                   @ set up our stack frame
    add     fp, sp, #0                                 @ adjust the frame pointer
    push    {r2}                                       @ whence
    push    {r1}                                       @ offset
    mov     lr, sp                                     @ pointer to indirect args is kept in lr
    swi     #19                                        @ 19 is lseek, r0 is filedes
    bcc     1f                                         @ syscalls return carry-clear for success and carry-set for error
    ldr     r2, =errno                                 @ load up the errno address
    str     r0, [r2]                                   @ stash the returned errno in the error case
    mov     r0, #-1                                    @ syscalls return -1 for error
1:  sub     sp, r11, #0                                @ restore our stack pointer to point to our stack frame
    pop     {fp, pc}                                   @ restore caller frame and return
.size lseek, . - lseek
