/*
 * Structure returned by ftime system call
 */
struct timeb {
	time_t	time;
	u16	millitm;
	s16	timezone;
	s16	dstflag;
};
