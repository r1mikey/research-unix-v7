#ifndef BCM283X_PAGE_TABLES_H
#define BCM283X_PAGE_TABLES_H

#define BCM283X_PT_ALIGN_BYTES (0x00004000)
#define BCM283X_MAX_VIRT_PAGES 1048576

typedef unsigned int l1pte_t;
typedef unsigned int l2pte_t;

typedef struct l1pt_t {
  l1pte_t entries[4096];
} __attribute__((aligned(BCM283X_PT_ALIGN_BYTES))) l1pt_t;

typedef struct l2pt_t {
  l2pte_t entries[1048576];
} __attribute__((aligned(BCM283X_PT_ALIGN_BYTES))) l2pt_t;

extern void setup_one_page_mapping(unsigned int srcpg, unsigned int dstpg, unsigned int a);

#endif
