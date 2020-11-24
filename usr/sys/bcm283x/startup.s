.syntax unified
.arm
.section .text

.equ CPSR_MODE_MASK, 0x0000001f                        @ Mode bits mask
.equ CPSR_MODE_USR, 0x00000010                         @ User Mode
.equ CPSR_MODE_FIQ, 0x00000011                         @ Fast Interrupt Request Mode
.equ CPSR_MODE_IRQ, 0x00000012                         @ Interrupt Request Mode
.equ CPSR_MODE_SVC, 0x00000013                         @ Supervisor Mode
.equ CPSR_MODE_MON, 0x00000016                         @ Monitor (secure) Mode
.equ CPSR_MODE_ABT, 0x00000017                         @ Abort Mode
.equ CPSR_MODE_UND, 0x0000001b                         @ Undefined Mode
.equ CPSR_MODE_SYS, 0x0000001f                         @ System Mode

.equ CPSR_FIQ_DISABLED, 0x00000040                     @ IRQ disable bit
.equ CPSR_IRQ_DISABLED, 0x00000080                     @ FIQ disable bit

.equ CPSR_USR_SETUP, (CPSR_MODE_USR|CPSR_FIQ_DISABLED|CPSR_IRQ_DISABLED)
.equ CPSR_FIQ_SETUP, (CPSR_MODE_FIQ|CPSR_FIQ_DISABLED|CPSR_IRQ_DISABLED)
.equ CPSR_IRQ_SETUP, (CPSR_MODE_IRQ|CPSR_FIQ_DISABLED|CPSR_IRQ_DISABLED)
.equ CPSR_SVC_SETUP, (CPSR_MODE_SVC|CPSR_FIQ_DISABLED|CPSR_IRQ_DISABLED)
.equ CPSR_MON_SETUP, (CPSR_MODE_MON|CPSR_FIQ_DISABLED|CPSR_IRQ_DISABLED)
.equ CPSR_ABT_SETUP, (CPSR_MODE_ABT|CPSR_FIQ_DISABLED|CPSR_IRQ_DISABLED)
.equ CPSR_UND_SETUP, (CPSR_MODE_UND|CPSR_FIQ_DISABLED|CPSR_IRQ_DISABLED)
.equ CPSR_SYS_SETUP, (CPSR_MODE_SYS|CPSR_FIQ_DISABLED|CPSR_IRQ_DISABLED)

.extern _bad_exception
.extern _entry_und
.extern _entry_swi
.extern _entry_pabt
.extern _entry_dabt
.extern _entry_irq
.extern _entry_fiq

.global _start
_start:
    mov     r4, #0x100                                 @ DEVELOPMENT HACK: check for a valid ATAG - if not present, probably running qemu
    ldr     r4, [r4, #4]                               @ load up the tag from the potential ATAG
    ldr     r5, =0x54410001                            @ if the tag matches ATAG_CORE we're likely on real hardware
    cmp     r4, r5                                     @ do we match?
    moveq   r0, #0                                     @ yep, we match ATAG_CORE, likely not on qemu
    movne   r0, #1                                     @ nope, we have no ATAG_CORE, likely on qemu
    ldr     r6, =0xfff                                 @ ... for page size rounding...
    bic     r11, pc, r6                                @ r11 has the physical entry point for use in page table setup
    mov     r12, #0x20000000                           @ Pi1 - set the physical peripheral base
    mrc     p15, 0, r4, c0, c0, 0                      @ Read the CPUID into r4
    ldr     r5, =0x410fb767                            @ Pi1
    cmp     r4, r5                                     @ is this a Pi1?
    beq     4f                                         @ yep, skip to uniprocessor startup
    mov     r12, #0x3f000000                           @ Pi2/Pi3 - set the physical peripheral base
    ldr     r5, =0x410fc075                            @ Pi2
    cmp     r4, r5                                     @ is this a Pi2?
    beq     2f                                         @ yep, skip to multiprocesor startup
    ldr     r5, =0x410fd034                            @ Pi3
    cmp     r4, r5                                     @ is this a Pi3?
    beq     2f                                         @ yep, skip to multiprocesor startup
1:  wfi                                                @ not recognised? stall...
    b       1b                                         @ oops, we woke up! stall again...
2:  mrc     p15, 0, r4, c0, c0, 5                      @ multiprocessor startup - load the core identifier into r4
    and     r4, r4, #3                                 @ AND out the core number
    cmp     r4, #0                                     @ compare the current core with core 0
    beq     3f                                         @ if we're core 0, skip to pre-uniprocessor startup
    b       1b                                         @ ... otherwise we stall...
3:
   @mrs     r4, cpsr_all                               @ pre-uniprocessor startup - leave HYP mode if we're there...
   @and     r4, r4, #CPSR_MODE_MASK                    @ mask out the mode bits
   @cmp     r4, #0x1a                                  @ check if we're in HYP mode
   @bne     4f                                         @ nope, skip to uniprocessor startup
   @ldr     r4, 4f                                     @ load up the pc-relative address of uniprocessor startup
   @msr     elr_hyp, r4                                @ move the relative address of uniprocessor startup to the HYP exception link register
   @mov     r4, #CPSR_SVC_SETUP                        @ set up our jump to SVC
   @msr     spsr_hyp, r4                               @ set up SVC as the HYP SPSR
   @eret                                               @ jump out of HYP, continuing at the next line, but in SVC mode
4:  msr     cpsr_c, #0x000000d3                        @ uniprocessor startup - enter SVC mode with IRQ and FIQ disabled
    mov     r4, #0
    mcr     p15, 0, r4, c7, c10, 4                     @ Data Synchronisation Barrier (formerly Drain Write Buffer)
    mcr     p15, 0, r4, c7, c10, 5                     @ Data Memory Barrier (DMB)
    mcr     p15, 0, r4, c7, c5, 0                      @ Invalidate entire instruction cache (flush branch target cache if applicable)
    mcr     p15, 0, r4, c7, c6, 0                      @ Invalidate entire data cache
    mcr     p15, 0, r4, c7, c7, 0                      @ Invalidate both instruction and data caches or unified cache (flush branch target cache, if applicable)
    mcr     p15, 0, r4, c7, c5, 6                      @ Flush entire branch target cache (if applicable)
    mcr     p15, 0, r4, c7, c5, 4                      @ Flush Prefetch Buffer (ISB)
    mcr     p15, 0, r4, c8, c7, 0                      @ flush the instruction and data TLBs
    mov     sp, #0x00003000                            @ give ourselves a temporary stack for use during startuo - this physical storage becomes udot
    push    {r0}                                       @ stash the "probably qemu" flag...
    push    {r12}                                      @ stash the physical I/O address
    push    {r11}                                      @ stash the physical entry point address

    @
    @ Kernel Memory Map
    @
    @ 16MiB @ 4080MiB: User stack mapping         Start: 0xff000000, End: 0xfffffffc - user stack is set to 0xfffffff8 initially - needs a STACKALIGN define, estabur, grow, sureg
    @  ..................................
    @  Unmapped Space
    @  ..................................
    @  ?KiB @ XXXXXXX: Kernel bss mapping         Start: 0x????????, End: 0x????????, Size: 0x????????, Phys: ..., 4K aligned
    @  ?KiB @ XXXXXXX: Kernel data mapping        Start: 0x????????, End: 0x????????, Size: 0x????????, Phys: ..., 4K aligned
    @  ?KiB @ XXXXXXX: Kernel rodata mapping      Start: 0x????????, End: 0x????????, Size: 0x????????, Phys: ..., 4K aligned
    @  ?KiB @ XXXXXXX: Kernel text mapping        Start: 0x61609000, End: 0x????????, Size: 0x????????, Phys: 0x00008000+
    @  4KiB @ XXXXXXX: clearpage dst              Start: 0x61608000, End: 0x61607ffc, Size: 0x00001000, Phys: N/A
    @  4KiB @ XXXXXXX: copypage dst               Start: 0x61607000, End: 0x61607ffc, Size: 0x00001000, Phys: N/A
    @  4KiB @ XXXXXXX: copypage src               Start: 0x61606000, End: 0x61606ffc, Size: 0x00001000, Phys: N/A
    @  4KiB @ XXXXXXX: FIQ, IRQ, ABT, UND stacks  Start: 0x61605000, End: 0x61605ffc, Size: 0x00001000, Phys: 0x00002000
    @  4KiB @ XXXXXXX: Process 0 udot and stack   Start: 0x61604000, End: 0x61604ffc, Size: 0x00001000, Phys: 0x00003000 << nG, ASID 0
    @ 16KiB @ 1558MiB: L1 Page Tables             Start: 0x61600000, End: 0x61603ffc, Size: 0x00004000, Phys: 0x00004000
    @  4MiB @ 1554MiB: L2 Page Tables             Start: 0x61200000, End: 0x615ffffc, Size: 0x00400000, Phys: _end, rounded up to 4KiB
    @  2MiB @ 1552MiB: Extended I/O Window        Start: 0x61000000, End: 0x61200000, Size: 0x00200000, Phys: 0x40000000 or unmapped
    @ 16MiB @ 1536MiB: I/O Window                 Start: 0x60000000, End: 0x611ffffc, Size: 0x01200000, Phys: 0x20000000 or 0x3f000000
    @
    @ ATTRIBUTES:
    @  Device:
    @   APX: 0b0, AP[1:0]: 0b01 - privileged read/write
    @   TEX: 0b000, C: 0b0, B: 0b1 - shared, non-cacheable, bufferable
    @   XN: 0b1
    @   S: 0b1 (should not matter)
    @  Normal Page Tables:
    @   APX: 0b0, AP[1:0]: 0b01 - privileged read/write
    @   TEX: 0b000, C: 0b1, B: 0b0 - outer and inner write through, no write allocate
    @   XN: 0b1
    @   S: 0b1 (should not be needed, but this is read for walks)
    @  Normal Read/Write:
    @   APX: 0b0, AP[1:0]: 0b01 - privileged read/write
    @   TEX: 0b001, C: 0b1, B: 0b1 - outer and inner write-back, write-allocate
    @   XN: 0b1
    @   S: 0b1 (should not be needed, but can't hurt for DMA stuff)
    @  Normal Read-Only:
    @   APX: 0b1, AP[1:0]: 0b01 - privileged read-only
    @   TEX: 0b000, C: 0b1, B: 0b1 - outer and inner write-back, no write allocate
    @   XN: 0b1
    @   S: 0b1 (should not be needed, but can't hurt)
    @  Normal Executable:
    @   APX: 0b1, AP[1:0]: 0b01 - privileged read-only
    @   TEX: 0b000, C: 0b1, B: 0b1 - outer and inner write-back, no write allocate
    @   XN: 0b0
    @   S: 0b1 (should not be needed, but can't hurt)
    @
    @ An L1 supersection is 31:24 of base physical address, 19 is 1, 17 is nG, 16 is S, 15 is APX, 14:12 is TEX, 11:10 is AP, 9 in IMP, 4 is XN, 3 is C, 2 is B, 1:0 is 0b10 (x16)
    @  Dev Mask: 0x00090416
    @ An L1 section entry is 31:20 of base physical address, 18 is 0, 17 is nG, 16 is S, 15 is APX, 14:12 is TEX, 11:10 is AP, 9 in IMP, 8:5 is domain, 4 is XN, 3 is C, 2 is B and 1:0 is 0b10
    @  Dev Mask: 0x00010416
    @  Alt Dev : 0x00002412
    @  R/W Mask: 0x0001141e
    @  R/O Mask: 0x0001841e
    @  R/X Mask: 0x0001840e
    @  R/W/X Mask: 0x0001140e
    @ An L1 entry, pointing to L2, is 31:10 of L2 address, 9 is implementation defined, 8:5 is the domain, 4:2 SBZ, then 1:0 as 0b01
    @  All Mask: 0x00000001
    @ An L2 large page entry is 31:16 of physical base address, 15=XN, 14:12=TEX, 11=nG, 10=S, 9=APX, 8:6=SBZ, 5:4=AP, 3=C, 2=B, 1:0=0b01 - x16 (64KiB)
    @  R/W Mask: 0x0000941d
    @  R/O Mask: 0x0000861d
    @  R/X Mask: 0x0000061d
    @ An L2 extended small page is 31:12=phys, 11=nG, 10=S, 9=APX, 8:6=TEX, 5:4=AP, 3=C, 2=B, 1=1, 0=XN
    @  R/W Mask: 0x0000045f
    @  R/O Mask: 0x0000061f
    @  R/X Mask: 0x0000061e
    @  R/W/X Mask: 0x0000045e
    @  PT Mask: 0x0000041b
    @

    ldr     r4, =_start                                @ load up the start of the executable image
    ldr     r5, =_end                                  @ load up the end of the executable image
    sub     r4, r5, r4                                 @ leave the image size in r4 by subtracting start from end
    add     r4, r4, r6                                 @ add a page less one byte to the image size...
    bic     r4, r4, r6                                 @ ... and mask off the bottom bits to round the size
    bic     r5, pc, r6                                 @ round our pc down to a page boundary to get our entry point physical address
    add     r9, r4, r5                                 @ add the entry point to get a physical memory location from the size
    mov     r10, #0x00004000                           @ the physical location of the L1 page tables is fixed at 16KiB into RAM
    push    {r10}                                      @ stash the L1 page table physical location
    push    {r9}                                       @ stash the L2 page table physical location
    mov     r8, #0                                     @ set up the TTBC for TTBR0-use only
    mcr     p15, 0, r8, c2, c0, 2                      @ write TTBC
    orr     r8, r10, #0x13                             @ set up the TTBR0 entry (tables are in outer cacheable, write-through memory, walk is sharable, walk is cacheable)
    mcr     p15, 0, r8, c2, c0, 0                      @ write TTBR0

    @
    @ Link L1 page tables to L2 entries
    @
    mov     r4, r10                                    @ copy the L1 physical address to a mutable copy
    add     r5, r4, #0x4000                            @ we'll create 4096 L1 entries of 4 bytes each
    mov     r6, r9                                     @ load up the L2 location into r6
    ldr     r7, =0x00000001                            @ load up the basic section template
4:  orr     r8, r6, r7                                 @ OR the attributes template into the address
    str     r8, [r4], #4                               @ write the L1 value to the L1 table pointer, incrementing the pointer
    add     r6, r6, #0x400                             @ point the L2 pointer 1KiB forward
    cmp     r4, r5                                     @ are we done yet?
    blo     4b                                         @ nope, go again

    @
    @ Clear out the L2 entries
    @
    mov     r4, r9                                     @ get a mutable version of the L2 address in r4
    add     r5, r4, #0x400000                          @ we'll clear 4MiB of tables
    mov     r6, #0                                     @ clear down to 0
4:  str     r6, [r4], #4                               @ write a zero to the L2 pointer and increment the pointer
    cmp     r4, r5                                     @ are we done yet?
    blo     4b                                         @ nope, not done yet

    @
    @ Map 16MiB of I/O space to 1536MiB
    @
    mov     r4, r10                                    @ get a mutable copy of the L1 table address
    mov     r5, #0x60000000                            @ the target virtual address is 1536MiB
    mov     r5, r5, lsr #20                            @ r5 now has the table index in words
    mov     r8, #4                                     @ a multiplicand must be in a register...
    mul     r5, r5, r8                                 @ r5 now contains the byte offset of the first entry
    add     r4, r4, r5                                 @ r4 now contains the physical address of the first entry
    add     r5, r4, #64                                @ we're doing 16 1MiB entries at 4 bytes each, so stop when we exceed that
    mov     r8, r12, lsr #20                           @ get a mutable I/O base address in r8
    mov     r8, r8, lsl #20                            @ the address in r8 is now cleaned up for section use
    ldr     r7, =0x00010416                            @ load up the section attributes
10: orr     r6, r8, r7                                 @ OR the attributes in to create the entry
    str     r6, [r4], #4                               @ store the section entry, incrementing the write pointer
    add     r8, r8, #0x100000                          @ increment the mapped address by 1MiB...
    cmp     r4, r5                                     @ are we done yet?
    blt     10b                                        @ no, not done yet

    cmp     r12, #0x20000000                           @ do we have a Pi1 I/O base?
    beq     12f                                        @ we're on a Pi1 - no need to map extended I/O

    @
    @ Map 2MiB of extended I/O space to 1552MiB
    @
    mov     r4, r10                                    @ get a mutable copy of the L1 table address
    mov     r5, #0x61000000                            @ the target virtual address is 1552MiB
    mov     r5, r5, lsr #20                            @ r5 now has the table index in words
    mov     r8, #4                                     @ a multiplicand must be in a register...
    mul     r5, r5, r8                                 @ r5 now contains the byte offset of the first entry
    add     r4, r4, r5                                 @ r4 now contains the physical address of the first entry
    add     r5, r4, #8                                 @ we're doing 2 1MiB entries at 4 bytes each, so stop when we exceed that
    add     r8, r12, #0x01000000                       @ the physical extended address is 16MiB past the normal address
    mov     r8, r8, lsr #20                            @ clean up the physical address for section use
    mov     r8, r8, lsl #20                            @ the address in r8 is now cleaned up for section use
    ldr     r7, =0x00010416                            @ load up the section attributes
10: orr     r6, r8, r7                                 @ OR the attributes in to create the entry
    str     r6, [r4], #4                               @ store the section entry, incrementing the write pointer
    add     r8, r8, #0x100000                          @ increment the mapped address by 1MiB...
    cmp     r4, r5                                     @ are we done yet?
    blt     10b                                        @ no, not done yet

    b       12f
    @ r0: L1 base address
    @ r1: Physical address
    @ r2: Virtual Address
    @ r3: Attributes
    @ r4: Trashed
    @ r5: Trashed
    @ r6: Trashed
__map_one_l2_page:
    mov     r4, r2, lsr #20                            @ r4 contains the L1 entry index
    mov     r5, r2, lsl #12                            @ shift the address left to set up clearing non-address bits...
    mov     r5, r5, lsr #24                            @ r5 contains the L2 entry index
    mov     r6, #4                                     @ we'll need a byte offset for these entries
    mul     r4, r4, r6                                 @ r4 now contains the L1 entry byte offset
    mul     r5, r5, r6                                 @ r5 now contains the L2 entry byte offset
    add     r4, r0, r4                                 @ r4 now contains the L1 entry address
    ldr     r4, [r4]                                   @ ... and now r4 contains the contents of the entry at the derived address
    mov     r4, r4, lsr #10                            @ shift right to clear out non-address bits from the L2 pointer...
    mov     r4, r4, lsl #10                            @ ... and shift back again to just leave the address part in place
    add     r4, r4, r5                                 @ add the L2 offset to the L2 physical address read from the L1 entry to get the L2 entry address in r4
    mov     r5, r1, lsr #12                            @ shift the physical address into the L2 entry template...
    mov     r5, r5, lsl #12                            @ ... and shift the physical address bits back into place, leaving the attributes bits clear
    orr     r5, r5, r3                                 @ OR in the L2 entry attributes
    str     r5, [r4]                                   @ store the L2 entry to the calculated address
    mov     pc, lr                                     @ back to the caller

    @ r0: L1 base address
    @ r1: Physical address, trashed - pointing to the next page after the last one configured
    @ r2: Virtual Address, trashed - pointing to the next page after the last one configured
    @ r3: Attributes
    @ r4: Trashed
    @ r5: Trashed
    @ r6: Trashed
    @ r7: Number of pages, trashed
    @ r8: Trashed
.global __map_many_l2_pages
__map_many_l2_pages:
    cmp     r7, #0
    bxeq    lr
    mov     r8, lr
11: bl      __map_one_l2_page
    add     r1, r1, #0x1000
    add     r2, r2, #0x1000
    sub     r7, r7, #1
    cmp     r7, #0
    bxeq    r8
    b       11b
12:

    mov     r0, r10                                    @ copy the L1 address to r0

    @
    @ Identity map the first 16MiB of RAM during startup
    @
    mov     r1, #0                                     @ map from physical address 0
    mov     r2, #0                                     @ map to virtual address 0
    ldr     r3, =0x0000045e                            @ load up the page attributes - R/W/X
    mov     r7, #0x00001000                            @ 4096 pages...
    bl      __map_many_l2_pages

    @
    @ Map the L2 page tables
    @
    mov     r1, r9                                     @ Physical address is in r9 at the moment
    ldr     r2, =__l2pt_start                          @ The linker tells us the virtual address
    ldr     r3, =0x0000041b                            @ outer and inner write through, no write allocate
    ldr     r7, =__l2pt_end                            @ Load up the end address
    sub     r7, r7, r2                                 @ Subtract start from end to get the number of bytes
    mov     r7, r7, lsr #12                            @ Shift right by 12 to divide by the page size to get pages
    bl      __map_many_l2_pages

    @
    @ Map the L1 page tables
    @
    mov     r1, r10                                    @ Physical address is in r10 at the moment
    ldr     r2, =__l1pt_start                          @ The linker tells us the virtual address
    ldr     r3, =0x0000041b                            @ outer and inner write through, no write allocate
    ldr     r7, =__l1pt_end                            @ Load up the end address
    sub     r7, r7, r2                                 @ Subtract start from end to get the number of bytes
    mov     r7, r7, lsr #12                            @ Shift right by 12 to divide by the page size to get pages
    bl      __map_many_l2_pages

    @
    @ Map the udot page for process 0
    @
    mov     r1, #0x00003000                            @ Fixed physical page
    ldr     r2, =__udot_start                          @ The linker tells us the virtual address
    ldr     r3, =0x00000c5f                            @ read/write, non-global
    mov     r7, #1                                     @ Only one page of udot
    bl      __map_many_l2_pages

    @
    @ Map the mode stacks from the misc area
    @
    mov     r1, #0x00002000                            @ Fixed physical page
    ldr     r2, =__misc_start                          @ The linker tells us the virtual address
    ldr     r3, =0x0000045f                            @ read/write
    mov     r7, #1                                     @ Only one page of mode stacks
    bl      __map_many_l2_pages

    @
    @ map the next program text - need a better way to get segment addresses and virtual offset...
    @
    mov     r1, r11                                    @ the physical entry point is in r11
    ldr     r2, =_stext                                @ the linker tells us the virtual address
    ldr     r3, =0x0000061e                            @ r/x
    ldr     r7, =_etext                                @ end of the text segment
    sub     r7, r7, r2                                 @ r7 now holds the size on bytes of the text segment
    ldr     r6, =0xfff                                 @ 4096 bytes less 1
    add     r7, r7, r6                                 @ add one page less one byte to the size
    bic     r7, r7, r6                                 @ now clear off the bottom bits to leave the text segment size, rounded up to the page size
    mov     r7, r7, lsr #12                            @ finally, shift right by 12 to glean the number of pages
    bl      __map_many_l2_pages

    @
    @ map the rodata
    @
    ldr     r3, =0x0000061f                            @ r/o
    ldr     r9, =_erodata                              @ end of rodata
    ldr     r8, =_srodata                              @ start of rodata
    sub     r9, r9, r8                                 @ r9 now contains the size of the rodata in bytes
    ldr     r7, =0xfff                                 @ we'll round the size up to 4KiB...
    add     r9, r9, r7                                 @ ... by adding 4095...
    bic     r9, r9, r7                                 @ ... and clearing off the last 12 bits...
    mov     r7, r9, lsr #12                            @ move the number of rodata pages into r7
    bl      __map_many_l2_pages

    @
    @ map the initialised data
    @
    ldr     r3, =0x0000045f                            @ r/w
    ldr     r9, =_edata                                @ end of initialised data
    ldr     r8, =_sdata                                @ start of initialised data
    sub     r9, r9, r8                                 @ r9 now contains the size of the initialised data in bytes
    ldr     r7, =0xfff                                 @ we'll round the size up to 4KiB...
    add     r9, r9, r7                                 @ ... by adding 4095...
    bic     r9, r9, r7                                 @ ... and clearing off the last 12 bits...
    mov     r7, r9, lsr #12                            @ move the number of data pages into r7
    bl      __map_many_l2_pages

    @
    @ map the uninitialised data
    @
    ldr     r3, =0x0000045f                            @ r/w
    ldr     r9, =__bss_end                             @ end of uninitialised data
    ldr     r8, =__bss_start                           @ start of uninitialised data
    sub     r9, r9, r8                                 @ r9 now contains the size of the uninitialised data in bytes
    ldr     r7, =0xfff                                 @ we'll round the size up to 4KiB...
    add     r9, r9, r7                                 @ ... by adding 4095...
    bic     r9, r9, r7                                 @ ... and clearing off the last 12 bits...
    mov     r7, r9, lsr #12                            @ move the number of bss pages into r7
    bl      __map_many_l2_pages

    mov     r4, #3                                     @ we will be the manager of domain 0
    mcr     p15, 0, r4, c3, c0, 0                      @ set up the domain access control register

    mov     r4, #0
    mcr     p15, 0, r4, c13, c0, 1                     @ set the address space identifier (ASID) to zero

    pop     {r8, r9, r10, r11, r12}                    @ we'll be moving our stack shortly...

    @
    @ Move to virtual address space
    @
    mov     r4, #0
    mcr     p15, 0, r4, c7, c10, 4                     @ drain write buffer
    mcr     p15, 0, r4, c8, c7, 0                      @ flush the instruction and data TLBs
    mrc     p15, 0, r5, c1, c0, 0                      @ read the SCTLR
    orr     r5, r5, #0x1                               @ set the M bit (MMU enable)
   @orr     r5, r5, #0x2                               @ set the A bit (alignment fault checking enabled)
    orr     r5, r5, #0x4                               @ set the C bit (data cache enable)
    orr     r5, r5, #0x8                               @ set the W bit (write buffer enabled)
    orr     r5, r5, #0x800                             @ set the Z bit (program flow prediction enable)
    orr     r5, r5, #0x1000                            @ set the I bit (L1 instruction cache enable)
    orr     r5, r5, #0x400000                          @ set the U bit (ARMv6 unaligned data support)
    orr     r5, r5, #0x800000                          @ set the XP bit (subpages disabled - VMSAv6 format page tables)
    mcr     p15, 0, r4, c8, c7, 0                      @ flush the instruction and data TLBs
    ldr     lr, =15f
    b       14f
.align 5
14: mcr     p15, 0, r5, c1, c0, 0                      @ write the SCTLR
    mrc     p15, 0, r5, c1, c0, 0                      @ read it back again
    sub     pc, lr, r5, lsr #32                        @ jump to virtual space, flushing the pipeline

15: mov     r4, #0
    mcr     p15, 0, r4, c7, c10, 4                     @ Data Synchronisation Barrier (formerly Drain Write Buffer)
    mcr     p15, 0, r4, c7, c10, 5                     @ Data Memory Barrier (DMB)
    mcr     p15, 0, r4, c7, c5, 0                      @ Invalidate entire instruction cache (flush branch target cache if applicable)
    mcr     p15, 0, r4, c7, c6, 0                      @ Invalidate entire data cache
    mcr     p15, 0, r4, c7, c7, 0                      @ Invalidate both instruction and data caches or unified cache (flush branch target cache, if applicable)
    mcr     p15, 0, r4, c7, c5, 6                      @ Flush entire branch target cache (if applicable)
    mcr     p15, 0, r4, c7, c5, 4                      @ Flush Prefetch Buffer (ISB)

    @
    @ Set up exception vectors
    @
    ldr     r4, =_vectors                              @ load up the virtual address of the vectors...
    mcr     p15, 0, r4, c12, c0, 0                     @ ... now set the vectors

    @
    @ Set up trap modes
    @
    msr     cpsr_c, #CPSR_FIQ_SETUP                    @ enter FIQ mode...
    mov     r8, #0                                     @ clear banked registers in the mode
    mov     r9, #0                                     @ ...
    mov     r10, #0                                    @ ...
    mov     r11, #0                                    @ ...
    mov     r12, #0                                    @ ...
    mov     r14, #0                                    @ ...
    ldr     sp, =__fiq_stack_top                       @ set the stack pointer
    msr     cpsr_c, #CPSR_IRQ_SETUP                    @ enter IRQ mode...
    mov     r14, #0                                    @ clear banked registers in the mode
    ldr     sp, =__irq_stack_top                       @ set the stack pointer
    msr     cpsr_c, #CPSR_ABT_SETUP                    @ enter ABT mode...
    mov     r14, #0                                    @ clear banked registers in the mode
    ldr     sp, =__abt_stack_top                       @ set the stack pointer
    msr     cpsr_c, #CPSR_UND_SETUP                    @ enter UND mode...
    mov     r14, #0                                    @ clear banked registers in the mode
    ldr     sp, =__und_stack_top                       @ set the stack pointer
    msr     cpsr_c, #CPSR_SYS_SETUP                    @ system mode shares user mode registers, but we can't get out of USR mode
    mov     r13, #0                                    @ clear banked registers in the mode
    mov     r14, #0                                    @ ...
    msr     cpsr_c, #CPSR_SVC_SETUP                    @ get back into SVC mode...
    ldr     sp,= __udot_end                            @ load up the end of udot...

    @
    @ Clear the bss
    @
    ldr     r0,= __bss_start                           @ clear our BSS to set up a C environment
    ldr     r1,= __bss_end                             @ we have a start pointer in r0 and end in r1
    cmp     r0, r1                                     @ test if we even have a BSS (r0 and r1 will be equal if not)
    beq     15f                                        @ ... and if there was no BSS we skip past the init code
    mov     r2, #0                                     @ initialise a register to use in clearing the BSS...
14: str     r2, [r0], #4                               @ save the value of r2 to the pointer location stored in r0 and increment the address
    cmp     r0, r1                                     @ test if we've hit the end of the BSS...
    blt     14b                                        @ nope, not the end, clear the next 32 bits...
15:                                                    @ ... either we've cleared the BSS or there was no BSS...

    @
    @ Stash useful globals used in the rest of the kernel
    @
    ldr     r0, =_bcm283x_probably_qemu                @ set to non-zero if it looks like we're on qemu
    str     r12, [r0]

    ldr     r0, =_bcm283x_p2v_offset                   @ applies to the kernel text and data mapping only
    ldr     r1, =_stext
    sub     r1, r1, r10
    str     r1, [r0]
    mov     r2, r1                                     @ copy out the offset for later use

    ldr     r0, =_bcm283x_iobase                       @ used by drivers
    ldr     r1, =__io1_start
    str     r1, [r0]

    ldr     r1, =__udot_start                          @ provided by the linker
    mov     r4, r1, lsl #12                            @ shift out non-address bits...
    mov     r4, r4, lsr #24                            @ ... and shift the address bits into place
    mov     r4, r4, lsl #2                             @ multiply by 4 to get the byte offset
    mov     r1, r1, lsr #20                            @ now r1 contains the l1 index of the udot page
    mov     r1, r1, lsl #2                             @ multiply by 4 to get the byte offset
    ldr     r0, =__l1pt_start                          @ load up the l1 page table virtual address
    add     r0, r0, r1                                 @ now r0 contains the address of the l1 entry
    ldr     r0, [r0]                                   @ and _now_ r0 contains the l1 entry content
    ldr     r3, =0xfffffc00                            @ this is a mask for the non-address bits 
    and     r0, r0, r3                                 @ AND out the non-address bits
    add     r0, r0, r2                                 @ add the p2v offset - r0 now points to the l2 table for this l1 table
    add     r1, r0, r4                                 @ add in the byte index to get the target l2 entry
    ldr     r0, =_udot_l2entry_addr
    str     r1, [r0]

    ldr     r1, =__copypage_src                        @ provided by the linker
    mov     r4, r1, lsl #12                            @ shift out non-address bits...
    mov     r4, r4, lsr #24                            @ ... and shift the address bits into place
    mov     r4, r4, lsl #2                             @ multiply by 4 to get the byte offset
    mov     r1, r1, lsr #20                            @ now r1 contains the l1 index of the udot page
    mov     r1, r1, lsl #2                             @ multiply by 4 to get the byte offset
    ldr     r0, =__l1pt_start                          @ load up the l1 page table virtual address
    add     r0, r0, r1                                 @ now r0 contains the address of the l1 entry
    ldr     r0, [r0]                                   @ and _now_ r0 contains the l1 entry content
    and     r0, r0, r3                                 @ AND out the non-address bits
    add     r0, r0, r2                                 @ add the p2v offset - r0 now points to the l2 table for this l1 table
    add     r1, r0, r4                                 @ add in the byte index to get the target l2 entry
    ldr     r0, =_copypage_src_l2entry_addr
    str     r1, [r0]

    ldr     r1, =__copypage_dst                        @ provided by the linker
    mov     r4, r1, lsl #12                            @ shift out non-address bits...
    mov     r4, r4, lsr #24                            @ ... and shift the address bits into place
    mov     r4, r4, lsl #2                             @ multiply by 4 to get the byte offset
    mov     r1, r1, lsr #20                            @ now r1 contains the l1 index of the udot page
    mov     r1, r1, lsl #2                             @ multiply by 4 to get the byte offset
    ldr     r0, =__l1pt_start                          @ load up the l1 page table virtual address
    add     r0, r0, r1                                 @ now r0 contains the address of the l1 entry
    ldr     r0, [r0]                                   @ and _now_ r0 contains the l1 entry content
    and     r0, r0, r3                                 @ AND out the non-address bits
    add     r0, r0, r2                                 @ add the p2v offset - r0 now points to the l2 table for this l1 table
    add     r1, r0, r4                                 @ add in the byte index to get the target l2 entry
    ldr     r0, =_copypage_dst_l2entry_addr
    str     r1, [r0]

    ldr     r1, =__clearpage_dst                       @ provided by the linker
    mov     r4, r1, lsl #12                            @ shift out non-address bits...
    mov     r4, r4, lsr #24                            @ ... and shift the address bits into place
    mov     r4, r4, lsl #2                             @ multiply by 4 to get the byte offset
    mov     r1, r1, lsr #20                            @ now r1 contains the l1 index of the udot page
    mov     r1, r1, lsl #2                             @ multiply by 4 to get the byte offset
    ldr     r0, =__l1pt_start                          @ load up the l1 page table virtual address
    add     r0, r0, r1                                 @ now r0 contains the address of the l1 entry
    ldr     r0, [r0]                                   @ and _now_ r0 contains the l1 entry content
    and     r0, r0, r3                                 @ AND out the non-address bits
    add     r0, r0, r2                                 @ add the p2v offset - r0 now points to the l2 table for this l1 table
    add     r1, r0, r4                                 @ add in the byte index to get the target l2 entry
    ldr     r0, =_clearpage_dst_l2entry_addr
    str     r1, [r0]

    ldr     r1, =__bss_end                             @ round up the end of bss to find the end of the kernel
    ldr     r3, =0xfff                                 @ we'll round up to a 4KiB boundary...
    add     r1, r1, r3                                 @ ... by adding 4095...
    bic     r1, r1, r3                                 @ and clearing off the last 12 bits... r1 now contains the kernel end virtual address, rounded up to a 4KiB boundary
    sub     r1, r1, r2                                 @ subtract the p2v bits to go from v2p
    ldr     r0, =0x00400000                            @ l2 tables take up 4MiB here...
    add     r1, r1, r0                                 @ r1 now contains the first available physical address past the kernel
    ldr     r0, =_first_available_phys_addr
    str     r1, [r0]

    ldr     r1, =__udot_start
    ldr     r0, =_udot_base
    str     r1, [r0]

    ldr     r0, =0x3f000000
    cmp     r11, r0
    movne   r1, #0
    moveq   r1, #1
    ldr     r0, =_bcm283x_has_core_block
    str     r1, [r0]

    ldr     r9, =__bss_end                             @ end of uninitialised data
    ldr     r8, =__bss_start                           @ start of uninitialised data
    sub     r9, r9, r8                                 @ r9 now contains the size of the uninitialised data in bytes
    ldr     r7, =0xfff                                 @ we'll round the size up to 4KiB...
    add     r9, r9, r7                                 @ ... by adding 4095...
    bic     r9, r9, r7                                 @ ... and clearing off the last 12 bits...

    @
    @ Clear out the process 0 stack (initial udot)
    @
    ldr     r0,= __udot_start                          @ clear our udot to set up a C environment (this is our process 0 stack)
    ldr     r1,= __udot_end                            @ we have a start pointer in r0 and end in r1
    cmp     r0, r1                                     @ test if we even have a udot (r0 and r1 will be equal if not)
    beq     17f                                        @ ... and if there was no udot we skip past the init code
    mov     r2, #0                                     @ initialise a register to use in clearing the BSS...
16: str     r2, [r0], #4                               @ save the value of r2 to the pointer location stored in r0 and increment the address
    cmp     r0, r1                                     @ test if we've hit the end of the udot...
    blt     16b                                        @ nope, not the end, clear the next 32 bits...
17:                                                    @ ... either we've cleared the udot or there was no udot...

    ldr     r2, =__udot_start                          @ get the va of the udot area
    mov     r2, r2, lsr #12                            @ turn the va for udot into a virtual page number
    mov     r2, r2, lsl #2                             @ now turn the vpn into a byte offset
    ldr     r3, =__l2pt_start                          @ load up the start of the L2 page table
    add     r2, r3, r2                                 @ add the L2 page table address to the byte offset to get the address we must write
    ldr     r1, =__udot_l2pt_address                   @ load up the variable that holds the udot l2 page table entry address
    str     r2, [r1]                                   @ write the address

    mov     r0, #0
    mov     r1, #0
    mov     r2, #0
    mov     r3, #0
    mov     r4, #0
    mov     r5, #0
    mov     r6, #0
    mov     r7, #0
    mov     r8, #0
    mov     r9, #0
    mov     r10, #0
    mov     r11, #0
    mov     r12, #0
    mov     r14, #0

    mcr     p15, 0, r3, c7, c10, 4                     @ DSB
    mcr     p15, 0, r3, c7, c10, 5                     @ DMB
    mcr     p15, 0, r3, c7, c5, 4                      @ Flush Prefetch Buffer (ISB)

    bl      main

    mov     r0, #0                                     @ clear out user registers...
    push    {r0}                                       @ ... padding
    ldr     r1, =CPSR_MODE_USR                         @ spsr
    push    {r1}                                       @ cpsr for usr mode
    push    {r0}                                       @ r15 (pc)
    push    {r0}                                       @ r14 (lr)
    push    {r0}                                       @ r13 (sp)
    push    {r0}                                       @ r12 (fp)
    push    {r0}                                       @ r11
    push    {r0}                                       @ r10
    push    {r0}                                       @ r9
    push    {r0}                                       @ r8
    push    {r0}                                       @ r7
    push    {r0}                                       @ r6
    push    {r0}                                       @ r5
    push    {r0}                                       @ r4
    push    {r0}                                       @ r3
    push    {r0}                                       @ r2
    push    {r0}                                       @ r1
    push    {r0}                                       @ r0

    b       __eret_user                                @ use the common exception return code to switch to userspace
1:  wfi                                                @ should never happen...
    b       1b

@ should this panic?
.global __aeabi_idiv0
__aeabi_idiv0:
    mov     pc, lr

.global current_core
current_core:
    mrc     p15, 0, r0, c0, c0, 5                      @ load the core identifier into r0
    and     r0, r0, #3                                 @ AND out the core number
    mov     pc, lr

.global denada
denada:
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    mov     pc, lr

.balign 32
_vectors:
    ldr pc, _rst_ptr                                    @ Each exception vector entry simply loads
    ldr pc, _und_ptr                                    @ the address of a shim function into the
    ldr pc, _swi_ptr                                    @ program counter register.  This has the effect
    ldr pc, _pabt_ptr                                   @ of letting relocate the exception vector and
    ldr pc, _dabt_ptr                                   @ jump table (relatively addressed) and using those
    ldr pc, _unu_ptr                                    @ to target the absolute addresses stored in the
    ldr pc, _irq_ptr                                    @ shim table.  In short, _start loads the value
    ldr pc, _fiq_ptr                                    @ stored at _v0_ptr to the program counter, which
                                                        @ effectively jumps to the _reset function.  This
_rst_ptr:  .word _bad_exception                         @ mechanism is used to get around limits in
_und_ptr:  .word _entry_und                             @ immediates in assembly, and lets us change the
_swi_ptr:  .word _entry_swi                             @ vector targets by changing function pointers
_pabt_ptr: .word _entry_pabt                            @ instead of rewriting instructions at runtime.
_dabt_ptr: .word _entry_dabt
_unu_ptr:  .word _bad_exception
_irq_ptr:  .word _entry_irq
_fiq_ptr:  .word _entry_fiq

.data
.global _bcm283x_probably_qemu
_bcm283x_probably_qemu:
    .word   0

.global _bcm283x_p2v_offset
_bcm283x_p2v_offset:
    .word   0

.global _bcm283x_iobase
_bcm283x_iobase:
    .word   0

.global _bcm283x_l1pt_base
_bcm283x_l1pt_base:
    .word   0

.global _udot_l2entry_addr
_udot_l2entry_addr:
    .word   0

.global _copypage_src_l2entry_addr
_copypage_src_l2entry_addr:
    .word   0

.global _copypage_dst_l2entry_addr
_copypage_dst_l2entry_addr:
    .word   0

.global _clearpage_dst_l2entry_addr
_clearpage_dst_l2entry_addr:
    .word   0

.global _first_available_phys_addr
_first_available_phys_addr:
    .word   0

.global _udot_base
_udot_base:
    .word   0

.global _bcm283x_has_core_block
_bcm283x_has_core_block:
    .word   0

.global __udot_l2pt_address
__udot_l2pt_address:
    .word   0
