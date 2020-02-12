/* kernel types */
typedef signed char	__i8;
typedef unsigned char	__u8;
typedef signed short	__i16;
typedef unsigned short	__u16;
typedef signed int	__i32;
typedef unsigned int	__u32;

typedef	__i32		daddr_t;  	/* disk address */
typedef	__i8 *     	caddr_t;  	/* core address */
typedef	__u16		ino_t;     	/* i-node number */
typedef	__i32		time_t;   	/* a time */
typedef	__u32		label_t[12]; 	/* program status */
typedef	__i16		dev_t;    	/* device code */
typedef	__i32		off_t;    	/* offset in file */
	/* selectors and constructor for device code */
#define	major(x)  	(int)(((unsigned)x>>8))
#define	minor(x)  	(int)(x&0377)
#define	makedev(x,y)	(dev_t)((x)<<8|(y))
