struct map
{
	s16	m_size;
	u16	m_addr;
};

struct map coremap[CMAPSIZ];	/* space for core allocation */
struct map swapmap[SMAPSIZ];	/* space for swap allocation */
