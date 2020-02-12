#define	ARMAG	0177545
struct	ar_hdr {
	char	ar_name[14];
	long	ar_date;
	char	ar_uid;
	char	ar_gid;
	short	ar_mode;
	long	ar_size;
};
