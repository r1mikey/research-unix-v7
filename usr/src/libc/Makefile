VPATH = stdio/armv6k:gen/armv6k:sys/armv6k:crt/armv6k:csu/armv6k:stdio:gen

STDIO_OBJECTS = getgrgid.o \
                getgrnam.o \
                getgrent.o \
                getpass.o \
                getpwnam.o \
                getpwuid.o \
                getpwent.o \
                fgetc.o \
                fputc.o \
                getchar.o \
                putchar.o \
                popen.o \
                freopen.o \
                fgets.o \
                fputs.o \
                getpw.o \
                fseek.o \
                ftell.o \
                rew.o \
                rdwr.o \
                system.o \
                fopen.o \
                fdopen.o \
                scanf.o \
                doscan.o \
                fprintf.o \
                gets.o \
                getw.o \
                printf.o \
                puts.o \
                putw.o \
                sprintf.o \
                ungetc.o \
                filbuf.o \
                setbuf.o \
                strout.o \
                flsbuf.o \
                endopen.o \
                findiop.o \
                clrerr.o \
                doprnt.o \
                data.o

GEN_OBJECTS   = cuexit.o \
                execvp.o \
                getenv.o \
                getlogin.o \
		getopt.o \
                perror.o \
                sleep.o \
                timezone.o \
                ttyslot.o \
                ttyname.o \
                abort.o \
                abs.o \
                atof.o \
                atoi.o \
                atol.o \
                crypt.o \
                ctime.o \
                calloc.o \
                malloc.o \
                ecvt.o \
                errlst.o \
                isatty.o \
                l3.o \
                mktemp.o \
                mon.o \
                nlist.o \
                qsort.o \
                rand.o \
                setjmp.o \
                stty.o \
                swab.o \
                tell.o \
                ctype_.o \
                index.o \
                rindex.o \
                strcat.o \
                strncat.o \
		strchr.o \
                strcmp.o \
                strncmp.o \
                strcpy.o \
                strncpy.o \
                strlen.o \
                ldexp.o \
                memcpy.o

SYS_OBJECTS   = access.o \
                acct.o \
                alarm.o \
                chdir.o \
                chmod.o \
                chown.o \
                chroot.o \
                close.o \
                creat.o \
                dup.o \
                execl.o \
                execle.o \
                execv.o \
                execve.o \
                exit.o \
                fork.o \
                fstat.o \
                getgid.o \
                getpid.o \
                getuid.o \
                ioctl.o \
                kill.o \
                link.o \
                lock.o \
                lseek.o \
                mknod.o \
                mount.o \
                nice.o \
                open.o \
                pause.o \
                phys.o \
                pipe.o \
                profil.o \
                ptrace.o \
                read.o \
                sbrk.o \
                setgid.o \
                setuid.o \
                signal.o \
                stat.o \
                stime.o \
                sync.o \
                time.o \
                times.o \
                umask.o \
                umount.o \
		uname.o \
                unlink.o \
                utime.o \
                wait.o \
                write.o

CRT_OBJECTS    = __aeabi_idiv0.o \
                cerror.o \
                aeabi_uidivmod.o \
                udivsi3.o \
                divsi3.o \
                divmodsi4.o \
                aeabi_uldivmod.o \
                aeabi_idivmod.o \
                udivmodsi4.o \
                udivmoddi4.o

STARTFILES    = crt0.o


OBJECTS = $(sort $(STDIO_OBJECTS) $(GEN_OBJECTS) $(SYS_OBJECTS) $(CRT_OBJECTS))
TARGET = libc.a

all: $(TARGET) $(STARTFILES)

$(TARGET): $(OBJECTS)
	$(AR) rcu $@ $^
	$(RANLIB) $@

doprnt.o: CPPFLAGS = $(CPP11SPECIALFLAGS)
doprnt.o: CFLAGS = $(C11SPECIALFLAGS)

udivmoddi4.o: CPPFLAGS = $(CPP11SPECIALFLAGS)
udivmoddi4.o: CFLAGS = $(C11SPECIALFLAGS)

#all: libc.a
#        @for __SD in $(SUBDIRS); do $(MAKE) -C $${__SD} all; done

SUBDIR_LEVEL := ../../..
include $(SUBDIR_LEVEL)/tools/tools.mk
include $(SUBDIR_LEVEL)/tools/patterns.mk
