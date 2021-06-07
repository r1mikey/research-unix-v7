.syntax unified
.arm
.section .text

@
@ See http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0301h/index.html
@

.extern __udot_l2pt_address
.extern __udot_start
.extern setup_one_page_mapping

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

.global read_ttbr0
read_ttbr0:
    mrc     p15, 0, r0, c2, c0, 0
    mov     pc, lr

.global read_curcpu
read_curcpu:
    mrc     p15, 0, r0, c0, c0, 5
    and     r0, #3
    mov     pc, lr

.global read_cpuid
read_cpuid:
    mrc     p15, 0, r0, c0, c0, 0                      @ Read the CPUID into r0
    mov     pc, lr

.global read_cpsr
read_cpsr:
    mrs     r0, cpsr
    mov     pc, lr

.global read_spsr
read_spsr:
    mrs     r0, spsr
    mov     pc, lr

.global read_sp
read_sp:
    mov     r0, sp
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

.global read_adfsr
read_adfsr:
    mrc     p15, 0, r2, c5, c1, 0                      @ ADFSR
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

.global do_invalidate_icache
do_invalidate_icache:
    mov     r0, #0
    mcr     p15, 0, r0, c7, c5, 0                      @ Invalidate entire instruction cache (flush branch target cache if applicable)
    mov     pc, lr

.global do_clean_and_invalidate_dcache
do_clean_and_invalidate_dcache:
    mov     r0, #0
    mcr     p15, 0, r0, c7, c14, 0                     @ Clean and invalidate entire data cache
    mov     pc, lr

.global do_clean_dcache
do_clean_dcache:
    mov     r0, #0
    mcr     p15, 0, r0, c7, c10, 0                     @ Clean entire data cache
    mov     pc, lr

.global do_invalidate_dcache
do_invalidate_dcache:
    mov     r0, #0
    mcr     p15, 0, r0, c7, c6, 0                      @ Invalidate entire data cache
    mov     pc, lr

.global coherence
coherence:
  mov     r0, #0
  mcr     p15, 0, r0, c7, c10, 4  @ DSB
  mov     r0, #0
  mcr     p15, 0, r0, c7, c5, 4   @ ISB
  mov     pc, lr

.global read_cpacr
read_cpacr:
  mrc     p15, 0, r0, c1, c0, 2
  mov     pc, lr

.global write_cpacr
write_cpacr:
  mcr     p15, 0, r0, c1, c0, 2
  b       do_arm1176jzfs_isb

@ TST sets the Z bit to 0 if the bit is set - this means that logic is bass ackwards

@
@ all return the previous interrupts-enabled flag
@
.global spl0
spl0:
    mrs     r0, cpsr                                   @ read the CPSR
    tst     r0, #0x00000080                            @ are interrupts disabled?
    bne     1f                                         @ yep, skip to enabling them
    mov     r0, #1                                     @ set the return code to indicate that the previous state was 'enabled'
    mov     pc, lr                                     @ back to the caller
1:  mov     r0, #0                                     @ interrupts are presently disabled, set the return value to reflect this
    cpsie   i                                          @ enable interrupts
    mov     pc, lr                                     @ back to the caller

.global spl1, spl2, spl3, spl4, spl5, spl6, spl7
spl1:
spl2:
spl3:
spl4:
spl5:
spl6:
spl7:
    mrs     r0, cpsr                                   @ read the CPSR
    tst     r0, #0x00000080                            @ are interrupts enabled?
    beq     1f                                         @ yep, skip to disabling them
    mov     r0, #0                                     @ set the return code to indicate that the previous state was 'disabled'
    mov     pc, lr                                     @ back to the caller
1:  mov     r0, #1                                     @ interrupts are presently enabled, set the return value to reflext this
    cpsid   i                                          @ disable interrupts
    mov     pc, lr                                     @ back to the caller

.global splx
splx:
    cmp     r0, #0                                     @ check if the previous state was 'disabled'
    bne     1f                                         @ nope, it was 'enabled' - skip to the 'enable' code
    cpsid   i                                          @ yep, we're restoring a disabled level, make it so...
    mov     pc, lr                                     @ back to the caller
1:  cpsie   i                                          @ we're restoring an enabled level, make it so...
    mov     pc, lr                                     @ back to the caller

.global getsplrestoreval
getsplrestoreval:
    mrs     r0, cpsr                                   @ read the CPSR
    tst     r0, #0x00000080                            @ are interrupts enabled?
    moveq   r0, #1                                     @ yep, enabled - return is 1
    movne   r0, #0                                     @ iff they're disabled we return 0
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


.global pre_page_table_modification
pre_page_table_modification:
    push    {fp, lr}                                   @ set up our stack frame
    add     fp, sp, #0                                 @ adjust the frame pointer
    mov     r0, #0
    mcr     p15, 0, r0, c7, c5, 4                      @ ISB
    mcr     p15, 0, r0, c7, c10, 5                     @ data move barrier
    mcr     p15, 0, r0, c7, c10, 0                     @ clean dcache
    mcr     p15, 0, r0, c7, c10, 4                     @ data sync barrier
    mcr     p15, 0, r0, c7, c10, 5                     @ data move barrier
    mcr     p15, 0, r0, c7, c10, 0                     @ clean dcache
    mcr     p15, 0, r0, c7, c10, 4                     @ data sync barrier
    sub     sp, r11, #0                                @ restore our stack pointer to point to our stack frame
    pop     {fp, pc}                                   @ restore caller frame and return


.global post_page_table_modification
post_page_table_modification:
    push    {fp, lr}                                   @ set up our stack frame
    add     fp, sp, #0                                 @ adjust the frame pointer
    mov     r0, #0
    mcr     p15, 0, r0, c7, c5, 4                      @ ISB
    mcr     p15, 0, r0, c7, c10, 5                     @ data move barrier
    mcr     p15, 0, r0, c7, c10, 0                     @ clean dcache
    mcr     p15, 0, r0, c7, c10, 4                     @ data sync barrier
    mcr     p15, 0, r0, c7, c14, 0                     @ Clean and invalidate entire data cache
    mcr     p15, 0, r0, c8, c7, 0                      @ flush the instruction and data TLBs
    sub     sp, r11, #0                                @ restore our stack pointer to point to our stack frame
    pop     {fp, pc}                                   @ restore caller frame and return


@ void copyseg(int from, int to)
@ addresses are page numbers
.global copyseg
copyseg:
    push    {fp, lr}                                   @ set up our stack frame
    add     fp, sp, #0                                 @ adjust the frame pointer
    push    {r0, r1}

    bl      spl7                                       @ disable interrupts
    push    {r0}                                       @ save the previous interrupt state

    bl      pre_page_table_modification                @ sync

    ldr     r0, [sp, #4]
    ldr     r1, =__copypage_src
    mov     r1, r1, lsr #12
    ldr     r2, =0x0000045f
    bl      setup_one_page_mapping

    ldr     r0, [sp, #8]
    ldr     r1, =__copypage_dst
    mov     r1, r1, lsr #12
    ldr     r2, =0x0000045f
    bl      setup_one_page_mapping

    mov     r3, #0
    mcr     p15, 0, r3, c7, c10, 0                     @ clean dcache
    mcr     p15, 0, r3, c7, c10, 4                     @ data sync barrier
    mcr     p15, 0, r3, c7, c14, 0                     @ Clean and invalidate entire data cache
    mcr     p15, 0, r3, c8, c7, 0                      @ flush the instruction and data TLBs

    push    {r4-r10}                                   @ save the callee-saved register we're going to trash
    ldr     r0, =__copypage_src                        @ r0 contains our source (virtual) address
    ldr     r1, =__copypage_dst                        @ r1 contains our destination (virtual) address
    mov     r10, r1                                    @ stash the dest page address as the comparator for completion
1:  cmp     r0, r10                                    @ are we done yet?
    bge     2f                                         @ why yes, we are! skip past the copy loop
    ldmia   r0!, {r2, r3, r4, r5, r6, r7, r8, r9}      @ load 8 words from the source, incrementing the source
    stmia   r1!, {r2, r3, r4, r5, r6, r7, r8, r9}      @ save those 8 words to the destination, incrementing the destination
    b       1b                                         @ loop back to check the completion condition
2:  pop     {r4-r10}                                   @ restore the callee-saved registers

    mov     r3, #0
    mcr     p15, 0, r3, c7, c10, 5                     @ data move barrier
    mcr     p15, 0, r3, c7, c10, 4                     @ data sync barrier
    mcr     p15, 0, r3, c7, c10, 0                     @ clean dcache

    mov     r0, #0
    ldr     r1, =__copypage_src
    mov     r1, r1, lsr #12
    ldr     r2, =0x0
    bl      setup_one_page_mapping

    mov     r0, #0
    ldr     r1, =__copypage_dst
    mov     r1, r1, lsr #12
    ldr     r2, =0x0
    bl      setup_one_page_mapping

    bl      post_page_table_modification               @ sync

    ldr     r0, [sp, #0]                               @ restore the previous interrupt state variable
    bl      splx                                       @ restore the interrupt state
    sub     sp, r11, #0                                @ restore our stack pointer to point to our stack frame
    pop     {fp, pc}                                   @ restore caller frame and return


@ void clearseg(int a)
@ address is a page number
.global clearseg
clearseg:
    push    {fp, lr}                                   @ set up our stack frame
    add     fp, sp, #0                                 @ adjust the frame pointer
    push    {r0}

    bl      spl7                                       @ disable interrupts
    push    {r0}                                       @ save the previous interrupt state

    bl      pre_page_table_modification                @ sync

    ldr     r0, [sp, #4]
    ldr     r1, =__clearpage_dst
    mov     r1, r1, lsr #12
    ldr     r2, =0x0000045f
    bl      setup_one_page_mapping

    mov     r3, #0
    mcr     p15, 0, r3, c7, c10, 0                     @ clean dcache
    mcr     p15, 0, r3, c7, c10, 4                     @ data sync barrier
    mcr     p15, 0, r3, c7, c14, 0                     @ Clean and invalidate entire data cache
    mcr     p15, 0, r3, c8, c7, 0                      @ flush the instruction and data TLBs

    push    {r4-r10}                                   @ save the callee-saved register we're going to trash
    ldr     r0, =__clearpage_dst                       @ r0 contains our source (virtual) address
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

    mov     r3, #0
    mcr     p15, 0, r3, c7, c10, 5                     @ data move barrier
    mcr     p15, 0, r3, c7, c10, 4                     @ data sync barrier
    mcr     p15, 0, r3, c7, c10, 0                     @ clean dcache

    mov     r0, #0
    ldr     r1, =__clearpage_dst
    mov     r1, r1, lsr #12
    ldr     r2, =0x0
    bl      setup_one_page_mapping

    bl      post_page_table_modification               @ sync

    ldr     r0, [sp, #0]                               @ restore the previous interrupt state variable
    bl      splx                                       @ restore the interrupt state
    sub     sp, r11, #0                                @ restore our stack pointer to point to our stack frame
    pop     {fp, pc}                                   @ restore caller frame and return


@ this is supposed to set a use floating point exception flag
.global stst
stst:
    mov     pc, lr                                     @ back to the caller


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


@ unclear, but I think this adjusts the user mode pc a bit... murky...
.globl backup
backup:
    mov     pc, lr                                     @ back to the caller

@ adds a profiling counter?
.globl addupc
addupc:
    mov     pc, lr                                     @ back to the caller
