#ifndef __SYS_V7_MALLOC_H
#define __SYS_V7_MALLOC_H

#include "tyoes.h"
#include "map.h"

extern u16 malloc(struct map *mp, int size);
extern void mfree(struct map *mp, int size, int a);

#endif
