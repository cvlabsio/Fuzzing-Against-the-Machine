#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/syscall.h>
#include <unistd.h>

#include "drv.h"
#include "sysc.h"

/* to ease compilation when memfd isnt supported on compilation host */
#define MFD_CLOEXEC     0x0001U
#define AF_UNIX         1
#define MFD_ALLOW_SEALING   0x0002U
#define SYS_memfd_create 319 /* x86 */
static int memfd_create(const char *name, unsigned int flags) {
    return real_syscall(SYS_memfd_create, name, flags);
}

int getStdFile(int typ)
{
    int fd, pipes[2];

    fd = -1;
    switch(typ) {
#define F(n, fn, flg) case n: fd = open(fn, flg); break;
    F(0, "/", O_RDONLY);
    F(1, "/proc/uptime", O_RDONLY);
#define S(n, a,b,c) case n: fd = socket(a, b, c); break;
    S(2, AF_INET, SOCK_STREAM, 0);
    S(3, AF_INET, SOCK_DGRAM, 0);
    S(4, AF_UNIX, SOCK_STREAM, 0);
    S(5, AF_UNIX, SOCK_DGRAM, 0);

    case 6: 
        if(pipe(pipes) == -1) return -1;
        fd = pipes[0];
        break;
    case 7:
        if(pipe(pipes) == -1) return -1;
        fd = pipes[1];
        break;

#define EV(n, i) case n: fd = pipe(i); break;
    EV( 8, 0);
    EV( 9, 1); 
    EV(10, 2);
    EV(11, 3);
    EV(12, 4);
    EV(13, 5);
    EV(14, 6);
    EV(15, 7);

#define MEM(n, nm, fl) case n: fd = memfd_create(nm, fl); break;
    MEM(22, "memfd1", 0);
    MEM(23, "memfd2", MFD_CLOEXEC);
    MEM(24, "memfd3", MFD_ALLOW_SEALING);
    MEM(25, "memfd4", MFD_CLOEXEC | MFD_ALLOW_SEALING);

    S(34, AF_UNIX, SOCK_STREAM, 0);
    S(35, AF_UNIX, SOCK_DGRAM, 0);
    S(36, AF_UNIX, SOCK_SEQPACKET, 0);
    S(37, AF_UNIX, SOCK_RAW, 0);
    S(38, AF_UNIX, SOCK_RDM, 0);
    S(40, AF_INET, SOCK_STREAM, 0);
    S(41, AF_INET, SOCK_DGRAM, 0);
    S(42, AF_INET, SOCK_SEQPACKET, 0);
    S(43, AF_INET, SOCK_RAW, IPPROTO_RAW);
    S(44, AF_INET, SOCK_RDM, 0);
    S(46, AF_INET6, SOCK_STREAM, 0);
    S(47, AF_INET6, SOCK_DGRAM, 0);
    S(48, AF_INET6, SOCK_SEQPACKET, 0);
    S(49, AF_INET6, SOCK_RAW, IPPROTO_RAW);
    S(50, AF_INET6, SOCK_RDM, 0);
    S(52, AF_IPX, SOCK_STREAM, 0);
    S(53, AF_IPX, SOCK_DGRAM, 0);
    S(54, AF_IPX, SOCK_SEQPACKET, 0);
    S(55, AF_IPX, SOCK_RAW, 0);
    S(56, AF_IPX, SOCK_RDM, 0);
    S(82, AF_APPLETALK, SOCK_STREAM, 0);
    S(83, AF_APPLETALK, SOCK_DGRAM, 0);
    S(84, AF_APPLETALK, SOCK_SEQPACKET, 0);
    S(85, AF_APPLETALK, SOCK_RAW, 0);
    S(86, AF_APPLETALK, SOCK_RDM, 0);

#define SP(n, f, ty, idx) case n: socketpair(f, ty, 0, pipes); fd = pipes[idx]; break
    SP(100, AF_UNIX, SOCK_STREAM, 0);
    SP(101, AF_UNIX, SOCK_STREAM, 1);
    SP(102, AF_UNIX, SOCK_DGRAM, 0);
    SP(103, AF_UNIX, SOCK_DGRAM, 1);
    SP(104, AF_UNIX, SOCK_SEQPACKET, 0);
    SP(105, AF_UNIX, SOCK_SEQPACKET, 1);

    default:
        // XXX nonblocking sockets?
        // XXX AF_NETLINK x (DGRAM,RAW) x protonr 0..22
        // XXX (INET, INET6) x SOCK_RAW x protonr 0..256
        // XXX PACKET X (DGRAM, RAW) x htons(rawtypes)
        // XXX weird files from /dev, /proc, /sys, etc..
        return -1;
    }
    return fd;
}

