// ++C CRT | Platform (Linux)
// Copyright 2026 Daniel McGuire
// Licensed under the MIT License

#include "../sys_linux.h"

long sys_write(int fd, const void *buf, size_t count) {
    return syscall(__NR_write, (long)fd, (long)buf, (long)count);
}
long sys_read(int fd, void *buf, size_t count) {
    return syscall(__NR_read, (long)fd, (long)buf, (long)count);
}
int sys_close(int fd) {
    return (int)syscall(__NR_close, (long)fd);
}
long sys_unlink(const char *path) {
    return syscall(__NR_unlink, (long)path);
}
long sys_lseek(int fd, long offset, int whence) {
    return syscall(__NR_lseek, (long)fd, offset, (long)whence);
}
long sys_openat(int dirfd, const char *path, int flags, int mode) {
    return syscall(__NR_openat, (long)dirfd, (long)path, (long)flags, (long)mode);
}
long sys_rt_sigaction(int signum, const void *act, void *oldact, size_t sigsetsize) {
    return syscall(__NR_rt_sigaction, (long)signum, (long)act, (long)oldact, (long)sigsetsize);
}
long sys_getpid(void) {
    return syscall(__NR_getpid);
}
long sys_kill(long pid, int sig) {
    return syscall(__NR_kill, pid, (long)sig);
}
void sys_exit(int status) {
#if defined(__NR_exit_group)
    (void)syscall(__NR_exit_group, (long)status);
#elif defined(__NR_exit)
    (void)syscall(__NR_exit, (long)status);
#endif
    __NORETURN__
}
void* sys_mmap(void *addr, size_t length, int prot, int flags, int fd, size_t offset) {
    long ret = -1;
#if defined(__NR_mmap2)
    ret = syscall(__NR_mmap2, (long)addr, (long)length, prot, flags, fd, offset >> 12);
#elif defined(__NR_mmap)
    ret = syscall(__NR_mmap, (long)addr, (long)length, prot, flags, fd, offset);
#endif
    if (ret < 0 && ret >= -4095) return NULL; 
    return (void*)ret;
}
int sys_munmap(void *addr, size_t length) {
    return (int)syscall(__NR_munmap, (long)addr, (long)length, 0);
}
long sys_futex(int *uaddr, int op, int val, const void *timeout, int *uaddr2, int val3) {
    long ret = -1;
#if defined(__NR_futex)
    ret = syscall(__NR_futex, (long)uaddr, (long)op, (long)val, (long)timeout, (long)uaddr2, (long)val3);
#elif defined(__NR_futex_time64)
    ret = syscall(__NR_futex_time64, (long)uaddr, (long)op, (long)val, (long)timeout, (long)uaddr2, (long)val3);
#endif
    if (ret == -38 && (op & FUTEX_PRIVATE_FLAG)) {
#if defined(__NR_futex)
        ret = syscall(__NR_futex, (long)uaddr, (long)(op & ~FUTEX_PRIVATE_FLAG), (long)val, (long)timeout, (long)uaddr2, (long)val3);
#endif
    }
    return ret;
}