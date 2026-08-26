#ifndef SYNC_H
#define SYNC_H

#include <stdbool.h>

bool rwlock_init(int *semaphore);
bool rwlock_get_writer(int *semaphore);
bool rwlock_get_reader(int *semaphore);
bool rwlock_release_writer(int *semaphore);
bool rwlock_release_reader(int *semaphore);
int rwlock_destroy(int *semaphore);

#endif
