#ifndef __V7_SYS_TYPES_H
#define __V7_SYS_TYPES_H

typedef signed char		__s8;
typedef unsigned char		__u8;
typedef signed short		__s16;
typedef unsigned short		__u16;
typedef signed int		__s32;
typedef unsigned int		__u32;
typedef signed long long	__s64;
typedef unsigned long long	__u64;

#ifdef KERNEL
typedef __s8			s8;
typedef __u8			u8;
typedef __s16			s16;
typedef __u16			u16;
typedef __s32			s32;
typedef __u32			u32;
typedef __s64			s64;
typedef __u64			u64;
#endif

typedef __s32			daddr_t;
typedef __u8 *			caddr_t;
typedef __u16			ino_t;
typedef __s32			time_t;
typedef __u32			label_t[12];    /* regs r3-r14 */
typedef __s16			dev_t;
typedef __s32			off_t;

	/* selectors and constructor for device code */
/* major part of a device */
#define	major(x)  	(short)(((unsigned)x>>8))
/* minor part of a device */
#define	minor(x)  	(short)(x&0377)
/* make a device number */
#define	makedev(x,y)	(dev_t)((x)<<8|(y))

#endif
