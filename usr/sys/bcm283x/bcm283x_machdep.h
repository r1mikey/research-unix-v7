#ifndef BCM283X_MACHDEP_H
#define BCM283X_MACHDEP_H

#include "kstddef.h"

#include "../h/types.h"

extern caddr_t _bcm283x_iobase;              /* peripheral base address */
extern caddr_t _bcm283x_p2v_offset;          /* physical space to kernel space offset */
extern u32 _udot_l2entry_addr;               /* shortcut to the l2 entry for the udot address */
extern u32 _copypage_src_l2entry_addr;       /* shortcut to the l2 entry for the copypage source address */
extern u32 _copypage_dst_l2entry_addr;       /* shortcut to the l2 entry for the copypafe destination address */
extern u32 _clearpage_dst_l2entry_addr;      /* shortcut to the l2 entry for the clearpage destination address */
extern u32 _first_available_phys_addr;       /* first available physical address past the kernel */
extern u32 _bcm283x_has_core_block;          /* 1 iff extra peripheral space for mpcore is available */
extern u32 _bcm283x_l1pt_base;               /* virtual address of the l1 page table */

#define MEM_VIRT_TO_PHYS(__addr) (((u32)(__addr)) - ((u32)_bcm283x_p2v_offset))
#define MEM_PHYS_TO_VIRT(__addr) (((u32)(__addr)) + ((u32)_bcm283x_p2v_offset))

#endif
