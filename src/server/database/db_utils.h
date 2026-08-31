#ifndef DB_UTILS_H
#define DB_UTILS_H

#include <stdbool.h>
#include <unistd.h>

ssize_t pread_exact(int fd, void *buf, size_t count, off_t offset);
ssize_t pwrite_exact(int fd, const void *buf, size_t count, off_t offset);

#endif
