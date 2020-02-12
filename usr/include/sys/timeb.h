/*
 * Structure returned by ftime system call
 */
struct timeb {
	time_t	time;
	__u16 millitm;
	__i16	timezone;
	__i16	dstflag;
};
