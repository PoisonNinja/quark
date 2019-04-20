#pragma once

#include <fs/dev.h>
#include <fs/inode.h>
#include <proc/sched.h>

namespace filesystem
{
namespace tty
{
class tty_core;

typedef unsigned char cc_t;
typedef unsigned int speed_t;
typedef unsigned int tcflag_t;

/* c_cc characters */
#define VINTR 0
#define VQUIT 1
#define VERASE 2
#define VKILL 3
#define VEOF 4
#define VTIME 5
#define VMIN 6
#define VSWTC 7
#define VSTART 8
#define VSTOP 9
#define VSUSP 10
#define VEOL 11
#define VREPRINT 12
#define VDISCARD 13
#define VWERASE 14
#define VLNEXT 15
#define VEOL2 16

/* c_iflag bits */
#define IGNBRK 0000001
#define BRKINT 0000002
#define IGNPAR 0000004
#define PARMRK 0000010
#define INPCK 0000020
#define ISTRIP 0000040
#define INLCR 0000100
#define IGNCR 0000200
#define ICRNL 0000400
#define IUCLC 0001000
#define IXON 0002000
#define IXANY 0004000
#define IXOFF 0010000
#define IMAXBEL 0020000
#define IUTF8 0040000

/* c_oflag bits */
#define OPOST 0000001
#define OLCUC 0000002
#define ONLCR 0000004
#define OCRNL 0000010
#define ONOCR 0000020
#define ONLRET 0000040
#define OFILL 0000100
#define OFDEL 0000200
#if defined __USE_MISC || defined __USE_XOPEN
#define NLDLY 0000400
#define NL0 0000000
#define NL1 0000400
#define CRDLY 0003000
#define CR0 0000000
#define CR1 0001000
#define CR2 0002000
#define CR3 0003000
#define TABDLY 0014000
#define TAB0 0000000
#define TAB1 0004000
#define TAB2 0010000
#define TAB3 0014000
#define BSDLY 0020000
#define BS0 0000000
#define BS1 0020000
#define FFDLY 0100000
#define FF0 0000000
#define FF1 0100000
#endif

#define VTDLY 0040000
#define VT0 0000000
#define VT1 0040000

#ifdef __USE_MISC
#define XTABS 0014000
#endif

/* c_cflag bit meaning */
#ifdef __USE_MISC
#define CBAUD 0010017
#endif
#define B0 0000000 /* hang up */
#define B50 0000001
#define B75 0000002
#define B110 0000003
#define B134 0000004
#define B150 0000005
#define B200 0000006
#define B300 0000007
#define B600 0000010
#define B1200 0000011
#define B1800 0000012
#define B2400 0000013
#define B4800 0000014
#define B9600 0000015
#define B19200 0000016
#define B38400 0000017
#ifdef __USE_MISC
#define EXTA B19200
#define EXTB B38400
#endif
#define CSIZE 0000060
#define CS5 0000000
#define CS6 0000020
#define CS7 0000040
#define CS8 0000060
#define CSTOPB 0000100
#define CREAD 0000200
#define PARENB 0000400
#define PARODD 0001000
#define HUPCL 0002000
#define CLOCAL 0004000
#ifdef __USE_MISC
#define CBAUDEX 0010000
#endif
#define B57600 0010001
#define B115200 0010002
#define B230400 0010003
#define B460800 0010004
#define B500000 0010005
#define B576000 0010006
#define B921600 0010007
#define B1000000 0010010
#define B1152000 0010011
#define B1500000 0010012
#define B2000000 0010013
#define B2500000 0010014
#define B3000000 0010015
#define B3500000 0010016
#define B4000000 0010017
#define __MAX_BAUD B4000000
#ifdef __USE_MISC
#define CIBAUD 002003600000  /* input baud rate (not used) */
#define CMSPAR 010000000000  /* mark or space (stick) parity */
#define CRTSCTS 020000000000 /* flow control */
#endif

/* c_lflag bits */
#define ISIG 0000001
#define ICANON 0000002
#if defined __USE_MISC || defined __USE_XOPEN
#define XCASE 0000004
#endif
#define ECHO 0000010
#define ECHOE 0000020
#define ECHOK 0000040
#define ECHONL 0000100
#define NOFLSH 0000200
#define TOSTOP 0000400
#ifdef __USE_MISC
#define ECHOCTL 0001000
#define ECHOPRT 0002000
#define ECHOKE 0004000
#define FLUSHO 0010000
#define PENDIN 0040000
#endif
#define IEXTEN 0100000
#ifdef __USE_BSD
#define EXTPROC 0200000
#endif

/* tcflow() and TCXONC use these */
#define TCOOFF 0
#define TCOON 1
#define TCIOFF 2
#define TCION 3

/* tcflush() and TCFLSH use these */
#define TCIFLUSH 0
#define TCOFLUSH 1
#define TCIOFLUSH 2

/* tcsetattr uses these */
#define TCSANOW 0
#define TCSADRAIN 1
#define TCSAFLUSH 2

#define NCCS 32

extern const char* init_cc;
extern const size_t num_init_cc;

struct ktermios {
    tcflag_t c_iflag; /* input modes */
    tcflag_t c_oflag; /* output modes */
    tcflag_t c_cflag; /* control modes */
    tcflag_t c_lflag; /* local modes */
    cc_t c_cc[NCCS];  /* special characters */
    speed_t c_ispeed; /* input speed */
    speed_t c_ospeed; /* output speed */
};

/* stolen from Linux */
/* ioctl command encoding: 32 bits total, command in lower 16 bits,
 * size of the parameter structure in the lower 14 bits of the
 * upper 16 bits.
 * Encoding the size of the parameter structure in the ioctl request
 * is useful for catching programs compiled with old versions
 * and to avoid overwriting user space outside the user buffer area.
 * The highest 2 bits are reserved for indicating the ``access mode''.
 * NOTE: This limits the max parameter size to 16kB -1 !
 */

/*
 * The following is for compatibility across the various Linux
 * platforms.  The generic ioctl numbering scheme doesn't really enforce
 * a type field.  De facto, however, the top 8 bits of the lower 16
 * bits are indeed used as a type field, so we might just as well make
 * this explicit here.  Please be sure to use the decoding macros
 * below from now on.
 */
#define _IOC_NRBITS 8
#define _IOC_TYPEBITS 8

/*
 * Let any architecture override either of the following before
 * including this file.
 */

#ifndef _IOC_SIZEBITS
#define _IOC_SIZEBITS 14
#endif

#ifndef _IOC_DIRBITS
#define _IOC_DIRBITS 2
#endif

#define _IOC_NRMASK ((1 << _IOC_NRBITS) - 1)
#define _IOC_TYPEMASK ((1 << _IOC_TYPEBITS) - 1)
#define _IOC_SIZEMASK ((1 << _IOC_SIZEBITS) - 1)
#define _IOC_DIRMASK ((1 << _IOC_DIRBITS) - 1)

#define _IOC_NRSHIFT 0
#define _IOC_TYPESHIFT (_IOC_NRSHIFT + _IOC_NRBITS)
#define _IOC_SIZESHIFT (_IOC_TYPESHIFT + _IOC_TYPEBITS)
#define _IOC_DIRSHIFT (_IOC_SIZESHIFT + _IOC_SIZEBITS)

/*
 * Direction bits, which any architecture can choose to override
 * before including this file.
 *
 * NOTE: _IOC_WRITE means userland is writing and kernel is
 * reading. _IOC_READ means userland is reading and kernel is writing.
 */

#ifndef _IOC_NONE
#define _IOC_NONE 0U
#endif

#ifndef _IOC_WRITE
#define _IOC_WRITE 1U
#endif

#ifndef _IOC_READ
#define _IOC_READ 2U
#endif

#define _IOC(dir, type, nr, size)                                              \
    (((dir) << _IOC_DIRSHIFT) | ((type) << _IOC_TYPESHIFT) |                   \
     ((nr) << _IOC_NRSHIFT) | ((size) << _IOC_SIZESHIFT))

#ifndef __KERNEL__
#define _IOC_TYPECHECK(t) (sizeof(t))
#endif

/*
 * Used to create numbers.
 *
 * NOTE: _IOW means userland is writing and kernel is reading. _IOR
 * means userland is reading and kernel is writing.
 */
#define _IO(type, nr) _IOC(_IOC_NONE, (type), (nr), 0)
#define _IOR(type, nr, size)                                                   \
    _IOC(_IOC_READ, (type), (nr), (_IOC_TYPECHECK(size)))
#define _IOW(type, nr, size)                                                   \
    _IOC(_IOC_WRITE, (type), (nr), (_IOC_TYPECHECK(size)))
#define _IOWR(type, nr, size)                                                  \
    _IOC(_IOC_READ | _IOC_WRITE, (type), (nr), (_IOC_TYPECHECK(size)))
#define _IOR_BAD(type, nr, size) _IOC(_IOC_READ, (type), (nr), sizeof(size))
#define _IOW_BAD(type, nr, size) _IOC(_IOC_WRITE, (type), (nr), sizeof(size))
#define _IOWR_BAD(type, nr, size)                                              \
    _IOC(_IOC_READ | _IOC_WRITE, (type), (nr), sizeof(size))

/* used to decode ioctl numbers.. */
#define _IOC_DIR(nr) (((nr) >> _IOC_DIRSHIFT) & _IOC_DIRMASK)
#define _IOC_TYPE(nr) (((nr) >> _IOC_TYPESHIFT) & _IOC_TYPEMASK)
#define _IOC_NR(nr) (((nr) >> _IOC_NRSHIFT) & _IOC_NRMASK)
#define _IOC_SIZE(nr) (((nr) >> _IOC_SIZESHIFT) & _IOC_SIZEMASK)

/* ...and for the drivers/sound files... */

#define IOC_IN (_IOC_WRITE << _IOC_DIRSHIFT)
#define IOC_OUT (_IOC_READ << _IOC_DIRSHIFT)
#define IOC_INOUT ((_IOC_WRITE | _IOC_READ) << _IOC_DIRSHIFT)
#define IOCSIZE_MASK (_IOC_SIZEMASK << _IOC_SIZESHIFT)
#define IOCSIZE_SHIFT (_IOC_SIZESHIFT)

#define TCGETS 0x5401
#define TCSETS 0x5402
#define TCSETSW 0x5403
#define TCSETSF 0x5404
#define TCGETA 0x5405
#define TCSETA 0x5406
#define TCSETAW 0x5407
#define TCSETAF 0x5408
#define TCSBRK 0x5409
#define TCXONC 0x540A
#define TCFLSH 0x540B
#define TIOCEXCL 0x540C
#define TIOCNXCL 0x540D
#define TIOCSCTTY 0x540E
#define TIOCGPGRP 0x540F
#define TIOCSPGRP 0x5410
#define TIOCOUTQ 0x5411
#define TIOCSTI 0x5412
#define TIOCGWINSZ 0x5413
#define TIOCSWINSZ 0x5414
#define TIOCMGET 0x5415
#define TIOCMBIS 0x5416
#define TIOCMBIC 0x5417
#define TIOCMSET 0x5418
#define TIOCGSOFTCAR 0x5419
#define TIOCSSOFTCAR 0x541A
#define FIONREAD 0x541B
#define TIOCINQ FIONREAD
#define TIOCLINUX 0x541C
#define TIOCCONS 0x541D
#define TIOCGSERIAL 0x541E
#define TIOCSSERIAL 0x541F
#define TIOCPKT 0x5420
#define FIONBIO 0x5421
#define TIOCNOTTY 0x5422
#define TIOCSETD 0x5423
#define TIOCGETD 0x5424
#define TCSBRKP 0x5425  /* Needed for POSIX tcsendbreak() */
#define TIOCSBRK 0x5427 /* BSD compatibility */
#define TIOCCBRK 0x5428 /* BSD compatibility */
#define TIOCGSID 0x5429 /* Return the session ID of FD */
#define TIOCGRS485 0x542E
#ifndef TIOCSRS485
#define TIOCSRS485 0x542F
#endif
#define TIOCGPTN                                                               \
    _IOR('T', 0x30, unsigned int)       /* Get Pty Number (of pty-mux device) */
#define TIOCSPTLCK _IOW('T', 0x31, int) /* Lock/unlock Pty */
#define TIOCGDEV                                                               \
    _IOR('T', 0x32, unsigned int) /* Get primary device node of /dev/console   \
                                   */
#define TCGETX 0x5432             /* SYS5 TCGETX compatibility */
#define TCSETX 0x5433
#define TCSETXF 0x5434
#define TCSETXW 0x5435
#define TIOCSIG _IOW('T', 0x36, int) /* pty: generate signal */
#define TIOCVHANGUP 0x5437
#define TIOCGPKT _IOR('T', 0x38, int)   /* Get packet mode state */
#define TIOCGPTLCK _IOR('T', 0x39, int) /* Get Pty lock state */
#define TIOCGEXCL _IOR('T', 0x40, int)  /* Get exclusive mode state */
#define TIOCGPTPEER _IO('T', 0x41)      /* Safely open the slave */
#define TIOCGISO7816 _IOR('T', 0x42, struct serial_iso7816)
#define TIOCSISO7816 _IOWR('T', 0x43, struct serial_iso7816)

#define FIONCLEX 0x5450
#define FIOCLEX 0x5451
#define FIOASYNC 0x5452
#define TIOCSERCONFIG 0x5453
#define TIOCSERGWILD 0x5454
#define TIOCSERSWILD 0x5455
#define TIOCGLCKTRMIOS 0x5456
#define TIOCSLCKTRMIOS 0x5457
#define TIOCSERGSTRUCT 0x5458  /* For debugging only */
#define TIOCSERGETLSR 0x5459   /* Get line status register */
#define TIOCSERGETMULTI 0x545A /* Get multiport config  */
#define TIOCSERSETMULTI 0x545B /* Set multiport config */

#define TIOCMIWAIT 0x545C  /* wait for a change on serial input line(s) */
#define TIOCGICOUNT 0x545D /* read serial port inline interrupt counts */

/*
 * Some arches already define FIOQSIZE due to a historical
 * conflict with a Hayes modem-specific ioctl value.
 */
#ifndef FIOQSIZE
#define FIOQSIZE 0x5460
#endif

/* Used for packet mode */
#define TIOCPKT_DATA 0
#define TIOCPKT_FLUSHREAD 1
#define TIOCPKT_FLUSHWRITE 2
#define TIOCPKT_STOP 4
#define TIOCPKT_START 8
#define TIOCPKT_NOSTOP 16
#define TIOCPKT_DOSTOP 32
#define TIOCPKT_IOCTL 64

#define TIOCSER_TEMT 0x01 /* Transmitter physically empty */

class tty_driver
{
public:
    tty_driver();
    virtual libcxx::pair<int, void*> open(const char* name);
    virtual ssize_t write(const uint8_t* buffer, size_t count);
    virtual void init_termios(struct ktermios& termios);

public:
    void set_core(tty_core* core);

protected:
    tty_core* core;
};

class tty_core : public kdevice
{
public:
    tty_core(tty_driver* driver, struct ktermios& termios);
    int ioctl(unsigned long request, char* argp, void* cookie) override final;
    libcxx::pair<int, void*> open(const char* name) override;
    int poll(filesystem::poll_register_func_t& callback, void* cookie) override;
    ssize_t read(uint8_t* buffer, size_t count, off_t offset,
                 void* cookie) override;
    ssize_t write(const uint8_t* buffer, size_t count, off_t offset,
                  void* cookie) override;

public:
    ssize_t notify(const uint8_t* buffer, size_t count);

private:
    ssize_t dump_input();

    tty_driver* driver;

    struct ktermios termios;

    // TODO: Replace with flip buffer
    char ibuffer[4096];
    size_t itail;

    char buffer[4096];
    size_t head, tail;
    scheduler::wait_queue queue;
};

enum tty_flags {
    tty_no_register = 0x1,
};

tty_core* register_tty(tty_driver* driver, dev_t major, dev_t minor,
                       unsigned flags);

void init();
} // namespace tty
} // namespace filesystem
