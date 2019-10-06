/* See LICENSE file for copyright and license details. */
#include <unistd.h>


#define t(...)                      do { if   (__VA_ARGS__)  goto fail; } while (0)
#define xpread(fd, buf, len, off)   t  (pread(fd, buf, len, off) < (ssize_t)(len))
#define xwrite(fd, buf, len)        t  (write(fd, buf, len)      < (ssize_t)(len))
