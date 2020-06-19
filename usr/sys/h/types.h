typedef signed char		i8;
typedef unsigned char		u8;
typedef signed short		i16;
typedef unsigned short		u16;
typedef signed int		i32;
typedef unsigned int		u32;
typedef signed long long	i64;
typedef unsigned long long	u64;

typedef i32             daddr_t;
typedef i8 *            caddr_t;
typedef u16             ino_t;
typedef i32             time_t;
typedef u32             label_t[12];    /* regs r3-r14 */
typedef i16             dev_t;
typedef i32             off_t;

	/* selectors and constructor for device code */
#define	major(x)  	(short)(((unsigned)x>>8))
#define	minor(x)  	(short)(x&0377)
#define	makedev(x,y)	(dev_t)((x)<<8|(y))
