/*
 * Structure returned by ftime system call
 */
struct timeb {
	time_t	time;
	u16	millitm;
	i16	timezone;
	i16	dstflag;
};
