.syntax unified
.arm
.section .text

@
@ See http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0301h/index.html
@

.extern __udot_l2pt_address
.extern __udot_start

.global translate_va_to_pa
translate_va_to_pa:
    mov     r1, #0                                     @ we'll need a zero reg for ISB later...
    mov     r0, r0, lsr #10                            @ clear out the address bits by shifting right...
    mov     r0, r0, lsl #10                            @ ... and shifting left again
    mcr     p15, 0, r0, c7, c8, 0                      @ perform the lookup
    mcr     p15, 0, r1, c7, c5, 4                      @ ISB
    mrc     p15, 0, r0, c7, c4, 0                      @ read the result
    and     r1, r0, #1                                 @ check if bit 0 is set
    cmp     r1, #1                                     @ ...
    moveq   r0, #0xffffffff                            @ it is, we have an error
    bxeq    lr                                         @ return the error to the caller
    mov     r0, r0, lsr #12                            @ successful lookup, clear out the non-address bits
    mov     r0, r0, lsl #12                            @ ... and shift the address bits back into place
    mov     pc, lr                                     @ back to the caller

.global __endless_spin
__endless_spin:
    cpsid   iaf
1:  wfi
    b       1b

.global read_cpsr
read_cpsr:
    mrs     r0, cpsr
    mov     pc, lr

.global read_ifsr
read_ifsr:
    mrc     p15, 0, r0, c5, c0, 1                      @ IFSR
    mov     pc, lr

.global read_dfsr
read_dfsr:
    mrc     p15, 0, r0, c5, c0, 0                      @ DFSR
    mov     pc, lr

.global read_dfar
read_dfar:
    mrc     p15, 0, r1, c6, c0, 0                      @ DFAR
    mov     pc, lr

.global read_asid
read_asid:
    mrc     p15, 0, r0, c13, c0, 1                     @ get the context ID register
    and     r0, r0, #0xff                              @ we only want the ASID part
    mov     pc, lr

.global write_asid
write_asid:
    mov     r0, r0, lsl #24                            @ shift up the ASID part
    mov     r0, r0, lsr #24                            @ ... and shift it back, leaving a clean ASID
    mov     r1, #0
    mcr     p15, 0, r1, c7, c10, 4                     @ DSB
    mcr     p15, 0, r0, c13, c0, 1                     @ set the context ID register
    mcr     p15, 0, r1, c7, c5, 4                      @ Flush Prefetch Buffer (IMB)
    mov     pc, lr

@
@ whenever a memory access requires ordering with regards to another memory
@ access
@
.global do_arm1176jzfs_dmb
do_arm1176jzfs_dmb:
    mov     r0, #0                                     @ arg SBZ
    mcr     p15, 0, r0, c7, c10, 5                     @ DMB
    mov     pc, lr

@
@ whenever a memory access needs to have completed before program execution
@ progresses
@
.global do_arm1176jzfs_dsb
do_arm1176jzfs_dsb:
    mov     r0, #0                                     @ arg SBZ
    mcr     p15, 0, r0, c7, c10, 4                     @ DSB
    mov     pc, lr

@
@ whenever instruction fetches need to explicitly take place after a certain
@ point in the program, for example after memory map updates or after writing
@ code to be executed
@
.global do_arm1176jzfs_isb
do_arm1176jzfs_isb:
    mov     r0, #0                                     @ arg SBZ
    mcr     p15, 0, r0, c7, c5, 4                      @ ISB
    mov     pc, lr

.global read_cpacr
read_cpacr:
    mrc     p15, 0, r0, c1, c0, 2
    mov     pc, lr

.global write_cpacr
write_cpacr:
    mcr     p15, 0, r0, c1, c0, 2
    b       do_arm1176jzfs_isb

.global enable_interrupts
enable_interrupts:
    mrs     r0, cpsr                                   @ read the CPSR
    tst     r0, #0x00000080                            @ are interrupts disabled?
    bne     1f                                         @ yep, skip to enabling them
    mov     r0, #1                                     @ set the return code to indicate that the previous state was 'enabled'
    mov     pc, lr                                     @ back to the caller
1:  mov     r0, #0                                     @ interrupts are presently disabled, set the return value to reflect this
    cpsie   i                                          @ enable interrupts
    mov     pc, lr                                     @ back to the caller

.global disable_interrupts
disable_interrupts:
    mrs     r0, cpsr                                   @ read the CPSR
    tst     r0, #0x00000080                            @ are interrupts disabled?
    beq     1f                                         @ nope, skip to disabling them
    mov     r0, #0                                     @ set the return code to indicate that the previous state was 'disabled'
    mov     pc, lr                                     @ back to the caller
1:  mov     r0, #1                                     @ interrupts are presently ensabled, set the return value to reflect this
    cpsid   i                                          @ disable interrupts
    mov     pc, lr                                     @ back to the caller

@
@ BIG FAT TODO
@ ************
@
@ We won't handle SEGV like Research v7 does in here... is it worth it?
@
.global fubyte, fuibyte
fubyte:
fuibyte:
    ldrb    r0, [r0]                                   @ load the byte from the current userspace
    mov     pc, lr                                     @ back to the caller

.global fuword, fuiword
fuword:
fuiword:
    ldr     r0, [r0]                                   @ load the word from the current userspace
    mov     pc, lr                                     @ back to the caller

@ void subyte(caddr_t addr, u8 v)
.global subyte, suibyte
subyte:
suibyte:
    strb    r1, [r0]                                   @ save the byte to the current userspace
    mov     r0, #0
    mov     pc, lr                                     @ back to the caller

@ void suword(caddr_t addr, u32 v)
.global suword, suiword
suword:
suiword:
    str     r1, [r0]                                   @ save the word to the current userspace
    mov     r0, #0
    mov     pc, lr                                     @ back to the caller

@ int copyin(caddr_t from, caddr_t to, size_t sz)
@ returns negative on error
@.global copyin, copyiin
@copyin:
@copyiin:
@    @ TODO: panic
@    mov     pc, lr                                     @ back to the caller

@ int copyout(caddr_t from, caddr_t to, size_t sz)
@ returns negative on error
@.global copyout, copyiout
@copyout:
@copyiout:
@    @ TODO: panic
@    mov     pc, lr                                     @ back to the caller


.global idle, waitloc
idle:
    mrs     r0, cpsr                                   @ read the CPSR
    bic     r1, r0, 0x00000080                         @ clear the I bit (enabling interrupts)
    msr     cpsr, r1                                   @ enable interrupts
    wfi                                                @ wait for an interrupt
_waitloc:                                              @ this location is used for idle tracking
    msr     cpsr, r0                                   @ restore the previous processor state
    mov     pc, lr                                     @ back to the caller
.data
waitloc: .word _waitloc
.text


@ void __copyseg_helper(u32 srcaddr, u32 dstaddr)
.text
.align 2
.global __copyseg_helper
.type __copyseg_helper,#function
.code 32
__copyseg_helper:
@fnstart
    push    {fp, lr}                                   @ set up our stack frame
    add     fp, sp, #0                                 @ adjust the frame pointer
    push    {r4-r10}                                   @ save the callee-saved register we're going to trash
    mov     r10, r1                                    @ stash the dest page address as the comparator for completion
1:  cmp     r0, r10                                    @ are we done yet?
    bge     2f                                         @ why yes, we are! skip past the copy loop
    ldmia   r0!, {r2, r3, r4, r5, r6, r7, r8, r9}      @ load 8 words from the source, incrementing the source
    stmia   r1!, {r2, r3, r4, r5, r6, r7, r8, r9}      @ save those 8 words to the destination, incrementing the destination
    b       1b                                         @ loop back to check the completion condition
2:  pop     {r4-r10}                                   @ restore the callee-saved registers
    sub     sp, r11, #0                                @ restore our stack pointer to point to our stack frame
    pop     {fp, pc}                                   @ restore caller frame and return
.size __copyseg_helper, . - __copyseg_helper
@.fnend


@ void __clearseg_helper(u32 pgaddr);
.text
.align 2
.global __clearseg_helper
.type __clearseg_helper,#function
.code 32
__clearseg_helper:
@fnstart
    push    {fp, lr}                                   @ set up our stack frame
    add     fp, sp, #0                                 @ adjust the frame pointer
    push    {r4-r10}                                   @ save the callee-saved register we're going to trash
    add     r10, r0, #0x1000                           @ r10 contains the comparator for completion
    mov     r2, #0
    mov     r3, #0
    mov     r4, #0
    mov     r5, #0
    mov     r6, #0
    mov     r7, #0
    mov     r8, #0
    mov     r9, #0
1:  cmp     r0, r10                                    @ are we done yet?
    bge     2f                                         @ why yes, we are! skip past the copy loop
    stmia   r0!, {r2, r3, r4, r5, r6, r7, r8, r9}      @ save our 8 words-of-zero to the destination, incrementing the destination
    b       1b                                         @ loop back to check the completion condition
2:  pop     {r4-r10}                                   @ restore the callee-saved registers
    sub     sp, r11, #0                                @ restore our stack pointer to point to our stack frame
    pop     {fp, pc}                                   @ restore caller frame and return
.size __clearseg_helper, . - __clearseg_helper
@.fnend


@ this is supposed to set a use floating point exception flag
.global stst
stst:
    mov     pc, lr                                     @ back to the caller


@ void tlbimva(uint32_t addr, uint32_t asid)
.text
.align 2
.global tlbimva
.type tlbimva,#function
.code 32
tlbimva:
@.fnstart
    mov     r0, r0, lsr #12
    mov     r0, r0, lsl #12
    and     r1, r1, #0xff
    add     r0, r0, r1
    mcr     p15, 0, r0, c8, c7, 1                      @ invalidate unified TLB by MVA (for one page)
    mov     pc, lr
.size tlbimva, . - tlbimva
@.fnend


@ void tlbiasid(uint32_t asid)
.text
.align 2
.global tlbiasid
.type tlbiasid,#function
.code 32
tlbiasid:
@.fnstart
    and     r0, r0, #0xff
    mcr     p15, 0, r0, c8, c7, 2                      @ invalidate unified TLB by ASID
    mov     pc, lr
.size tlbiasid, . - tlbiasid
@.fnend


@ void dcachecva(uint32_t start, uint32_t end)
.text
.align 2
.global dcachecva
.type dcachecva,#function
.code 32
dcachecva:
@.fnstart
    mcrr    p15, 0, r1, r0, c12                        @ clean data cache range
    mov     pc, lr
.size dcachecva, . - dcachecva
@.fnend


@ void dcacheiva(uint32_t start, uint32_t end)
.text
.align 2
.global dcacheiva
.type dcacheiva,#function
.code 32
dcacheiva:
@.fnstart
    mcrr    p15, 0, r1, r0, c6                         @ invalidate data cache range
    mov     pc, lr
.size dcacheiva, . - dcacheiva
@.fnend


@ void dcacheciva(uint32_t start, uint32_t end)
.text
.align 2
.global dcacheciva
.type dcacheciva,#function
.code 32
dcacheciva:
@.fnstart
    mcrr    p15, 0, r1, r0, c14                        @ clean and invalidate data cache range
    mov     pc, lr
.size dcacheciva, . - dcacheciva
@.fnend


@ void icacheiva(uint32_t start, uint32_t end)
.text
.align 2
.global icacheiva
.type icacheiva,#function
.code 32
icacheiva:
@.fnstart
    mcrr    p15, 0, r1, r0, c14                        @ invalidate instruction cache range
    mov     pc, lr
.size icacheiva, . - icacheiva
@.fnend


@ void flush_entire_btc(void)
.text
.align 2
.global flush_entire_btc
.type flush_entire_btc,#function
.code 32
flush_entire_btc:
@.fnstart
    mov     r0, #0
    mcr     p15, 0, r0, c7, c5, 6                      @ Flush Entire Branch Target Cache
    mov     pc, lr
.size flush_entire_btc, . - flush_entire_btc
@.fnend


@ void flush_prefetch_buffer(void)
.text
.align 2
.global flush_prefetch_buffer
.type flush_prefetch_buffer,#function
.code 32
flush_prefetch_buffer:
@.fnstart
    mov     r0, #0
    mcr     p15, 0, r0, c7, c5, 4                      @ Flush Prefetch Buffer
    mov     pc, lr
.size flush_prefetch_buffer, . - flush_prefetch_buffer
@.fnend


@ void resume(int new_stack, label_t label)
@ the new_stack arg is a page number, not an address
.text
.align 2
.global resume
.type resume,#function
.code 32
resume:
@.fnstart
    cpsid   i                                          @ interrupts off, restored by spsr_cxsf
    mov     r0, r0, lsl #12                            @ turn the physical page number into an address
    mov     lr, r0                                     @ back up the start address to lr
    ldr     r2, =0x0000045f                            @ r/w normal memory
    orr     r0, r0, r2                                 @ r0 now contains the entry we're going to write
    ldr     r2, =__udot_l2pt_address                   @ grab the L2 entry address
    ldr     r2, [r2]                                   @ deref to get the actual address
    ldr     r3, [r2]                                   @ deref again to get the current entry
    cmp     r0, r3                                     @ check if we're changing the entry...
    beq     1f                                         @ if no change, just restore registers
    mov     r3, #0                                     @ we're changing the entry
    mcr     p15, 0, r3, c7, c10, 5                     @ data move barrier
    ldr     lr, =__udot_start                          @ VA of the udot
    add     r3, lr, #4096                              @ end of our range...
    sub     r3, r3, #1                                 @ less one, to avoid a page fault
    mcrr    p15, 0, r3, lr, c14                        @ clean and invalidate data cache range
    mcr     p15, 0, lr, c8, c7, 1                      @ invalidate unified TLB by MVA (for one page)
    str     r0, [r2]                                   @ write the entry
    bic     r2, r2, #0x1f                              @ clear the sbz bits for the next instruction
    mcr     p15, 0, r2, c7, c14, 1                     @ clean and invalidate data cache line by mva
    mov     r3, #0                                     @ we need a zero register to do a...
    mcr     p15, 0, r3, c7, c10, 4                     @ ... data sync barrier
1:  ldmia   r1, {r3-lr}                                @ restore psr + callee-saved registers *from the restored udot*
    msr     spsr_cxsf, r3                              @ load up previous processor state
    mov     r0, #1                                     @ resume always returns 1
    clrex                                              @ clear exclusive monitors
    movs    pc, lr                                     @ back to the caller with an exception return
.size resume, . - resume
@.fnend


@ (label_t is an array of registers)
@ int save(label_t label);
@ always returns 0 - the resume returns 1
.text
.align 2
.globl save
.type save,#function
.code 32
save:
@.fnstart
    mrs     r3, cpsr                                   @ load up the processor status
    tst     r3, #0x00000080                            @ are interrupts enabled?
    bne     1f                                         @ presently disabled, no need to disable again
    cpsid   i                                          @ interrupts off
1:  stmia   r0, {r3-lr}                                @ psr + save callee-saved registers
    tst     r3, #0x00000080                            @ were interrupts enabled?
    bne     1f                                         @ nope, we can just exit, otherwise...
    cpsie   i                                          @ reenable interrupts
1:  mov     r0, #0                                     @ save always returns 0
    mov     pc, lr                                     @ back to the caller
.size save, . - save
@.fnend


@ backup unpicks an instruction and moves the passed pc back to start at that instruction
@ this is all already done for us by the trap handlers
.globl backup
backup:
    mov     pc, lr                                     @ back to the caller

@ adds a profiling counter?
.globl addupc
addupc:
    mov     pc, lr                                     @ back to the caller
