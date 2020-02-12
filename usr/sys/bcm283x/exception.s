.syntax unified
.arm
.section .text

.extern c_bad_exception
@.extern c_entry_swi
.extern c_entry_pabt
.extern c_entry_irq
.extern c_entry_fiq

.global _bad_exception
_bad_exception:
    bl      c_bad_exception
1:  cpsid   iaf                                        @ disable interrupts
    wfi                                                @ wait for an interrupt
    b       1b                                         @ and, if we ever get out of that, go again


@.global _entry_swi
@_entry_swi:
@    bl      c_entry_swi
@1:  cpsid   iaf                                        @ disable interrupts
@    wfi                                                @ wait for an interrupt
@    b       1b                                         @ and, if we ever get out of that, go again


@.global _entry_pabt
@_entry_pabt:
@@    mrc     p15, 0, r2, c5, c0, 1                      @ ISFR
@@    mrc     p15, 0, r3, c6, c0, 2                      @ IFAR
@@    mrc     p15, 0, r5, c5, c1, 1                      @ AIFSR
@    bl      c_entry_pabt
@1:  cpsid   iaf                                        @ disable interrupts
@    wfi                                                @ wait for an interrupt
@    b       1b                                         @ and, if we ever get out of that, go again


@.global _entry_dabt
@_entry_dabt:
@    push    {r0,r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,r11,r12,lr}
@    mrc     p15, 0, r0, c5, c0, 0                      @ DFSR
@    mrc     p15, 0, r1, c6, c0, 0                      @ DFAR
@    mrc     p15, 0, r2, c5, c1, 0                      @ ADFSR
@    bl      c_entry_dabt
@    pop     {r0,r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,r11,r12,lr}
@1:  cpsid   iaf                                        @ disable interrupts
@    wfi                                                @ wait for an interrupt
@    b       1b                                         @ and, if we ever get out of that, go again


@.global _entry_irq
@_entry_irq:
@    push    {r0,r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,r11,r12,lr}
@    bl      c_entry_irq
@    pop     {r0,r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,r11,r12,lr}
@    subs    pc,lr,#4


.global _entry_fiq
_entry_fiq:
    bl      c_entry_fiq
1:  cpsid   iaf                                        @ disable interrupts
    wfi                                                @ wait for an interrupt
    b       1b                                         @ and, if we ever get out of that, go again
