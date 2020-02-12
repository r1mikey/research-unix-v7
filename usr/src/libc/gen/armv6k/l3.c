/*
 * Convert longs to and from 3-byte disk addresses
 */
ltol3(cp, lp, n)
char	*cp;
long	*lp;
int	n;
{
	register i;
	register char *a;

	a = cp;
	for(i=0;i<n;i++) {
		*a++ = ((*lp) >>  0) & 0xff;
		*a++ = ((*lp) >>  8) & 0xff;
		*a++ = ((*lp) >> 16) & 0xff;
		lp++;
	}
}

l3tol(lp, cp, n)
long	*lp;
char	*cp;
int	n;
{
	register i;
	register char *b;

	b = cp;
	for(i=0;i<n;i++) {
		*lp++ =
			((((unsigned int)b[0]) <<  0) |
			 (((unsigned int)b[1]) <<  8) |
			 (((unsigned int)b[2]) << 16));
	}
}
