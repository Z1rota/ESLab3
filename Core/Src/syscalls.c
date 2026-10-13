/* No file/console I/O is used. Explicit stubs keep newlib's unused
   standard-stream cleanup independent of semihosting/debugger services. */
#include <errno.h>
#include <sys/types.h>
int _close(int fd) { (void)fd; errno=EBADF; return -1; }
int _read(int fd, char *buf, int count)
{ (void)fd; (void)buf; (void)count; errno=ENOSYS; return -1; }
int _write(int fd, const char *buf, int count)
{ (void)fd; (void)buf; (void)count; errno=ENOSYS; return -1; }
off_t _lseek(int fd, off_t offset, int whence)
{ (void)fd; (void)offset; (void)whence; errno=ESPIPE; return (off_t)-1; }
