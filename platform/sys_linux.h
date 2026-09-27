// ++C CRT | Platform (Linux)
// Copyright 2026 Daniel McGuire
// Licensed under the MIT License

#ifndef SYS_LINUX_H
#define SYS_LINUX_H
#include "../libminicrt/include/def.h"
#include <asm/unistd.h>
#define PROT_READ   0x1
#define PROT_WRITE  0x2
#define MAP_PRIVATE 0x02
#define MAP_ANON    0x20

#ifndef FUTEX_WAIT
#define FUTEX_WAIT 0
#endif
#ifndef FUTEX_WAKE
#define FUTEX_WAKE 1
#endif
#ifndef FUTEX_PRIVATE_FLAG
#define FUTEX_PRIVATE_FLAG 128
#endif
#ifndef FUTEX_WAIT_PRIVATE
#define FUTEX_WAIT_PRIVATE (FUTEX_WAIT | FUTEX_PRIVATE_FLAG)
#endif
#ifndef FUTEX_WAKE_PRIVATE
#define FUTEX_WAKE_PRIVATE (FUTEX_WAKE | FUTEX_PRIVATE_FLAG)
#endif

#define O_RDONLY   00
#define O_WRONLY   01
#define O_RDWR     02
#define O_CREAT    0100
#define O_EXCL     0200
#define O_TRUNC    01000
#define O_APPEND   02000

#define AT_FDCWD (-100)

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

long syscall(long number, ...);

long sys_unlink(const char *path);
long sys_write(int fd, const void *buf, size_t count);
long sys_read(int fd, void *buf, size_t count);
int sys_close(int fd);
long sys_lseek(int fd, long offset, int whence);
long sys_openat(int dirfd, const char *path, int flags, int mode);
void sys_exit(int status);
void* sys_mmap(void *addr, size_t length, int prot, int flags, int fd, size_t offset);
int sys_munmap(void *addr, size_t length);
long sys_futex(int *uaddr, int op, int val, const void *timeout, int *uaddr2, int val3);

long sys_rt_sigaction(int signum, const void *act, void *oldact, size_t sigsetsize);
long sys_getpid(void);
long sys_gettid(void);
long sys_kill(long pid, int sig);

long sys_socket(int domain, int type, int protocol);
long sys_bind(long sockfd, const void *addr, unsigned int addrlen);
long sys_listen(long sockfd, int backlog);
long sys_accept4(long sockfd, void *addr, unsigned int *addrlen, int flags);
long sys_connect(long sockfd, const void *addr, unsigned int addrlen);
long sys_sendto(long sockfd, const void *buf, size_t len, int flags, const void *dest_addr, unsigned int addrlen);
long sys_recvfrom(long sockfd, void *buf, size_t len, int flags, void *src_addr, unsigned int *addrlen);
long sys_shutdown(long sockfd, int how);
long sys_setsockopt(long sockfd, int level, int optname, const void *optval, unsigned int optlen);
long sys_getsockopt(long sockfd, int level, int optname, void *optval, unsigned int *optlen);
long sys_getsockname(long sockfd, void *addr, unsigned int *addrlen);
long sys_getpeername(long sockfd, void *addr, unsigned int *addrlen);
#endif /* SYS_LINUX_H */