@ C library -- uname
@ uname(unixname);
@ unixname[0], ...unixname[7] contain the unixname

.syntax unified
.arm
.section text

.global uname
.extern errno

.type uname,%function
uname:
	push {fp, lr}					@ setup the stack
	add fp, sp, #0					@ adjust the frame pointer
	push {r0}						@ fname
	mov lr, sp						@ pointer to indirect arguments stored in lr
	swi #57							@ uname syscall number
	bcc 1f							@ syscalls return carry-clear for success and carry-set for error
	ldr r2, =errno                  @ load up the address of errno
    str r0, [r2]                    @ stash the returned errno in the error case
    mov r0, #-1                     @ syscalls return -1 for error
1:  sub sp, fp, #0                  @ restore our stack pointer to point to our stack frame
    pop {fp, pc}                    @ restore caller frame and return
.size uname, . - uname
