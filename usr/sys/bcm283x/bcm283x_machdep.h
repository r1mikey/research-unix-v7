#ifndef BCM283X_MACHDEP_H
#define BCM283X_MACHDEP_H

#include "kstdint.h"
#include "kstddef.h"

extern devaddr_t _bcm283x_iobase;                 /* peripheral base address */
extern devaddr_t _bcm283x_p2v_offset;             /* physical space to kernel space offset */
extern uint32_t _bcm283x_probably_qemu;           /* do we appear to be running on qemu? */
extern uint32_t _udot_l2entry_addr;               /* shortcut to the l2 entry for the udot address */
extern uint32_t _copypage_src_l2entry_addr;       /* shortcut to the l2 entry for the copypage source address */
extern uint32_t _copypage_dst_l2entry_addr;       /* shortcut to the l2 entry for the copypafe destination address */
extern uint32_t _clearpage_dst_l2entry_addr;      /* shortcut to the l2 entry for the clearpage destination address */
extern uint32_t _first_available_phys_addr;       /* first available physical address past the kernel */
extern uint32_t _bcm283x_has_core_block;          /* 1 iff extra peripheral space for mpcore is available */
extern uint32_t _bcm283x_l1pt_base;               /* virtual address of the l1 page table */

#define MEM_VIRT_TO_PHYS(__addr) (((uint32_t)(__addr)) - _bcm283x_p2v_offset)
#define MEM_PHYS_TO_VIRT(__addr) (((uint32_t)(__addr)) + _bcm283x_p2v_offset)

#endif
