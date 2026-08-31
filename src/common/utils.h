#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>
#include <unistd.h>

void error(const char *m);
ssize_t recv_exact(int fd, void *buf, size_t count, int flags);
ssize_t send_exact(int fd, const void *buf, size_t count, int flags);
bool is_printable(const char *string, int len);

#endif
