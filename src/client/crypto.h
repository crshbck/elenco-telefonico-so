#ifndef CRYPTO_H
#define CRYPTO_H

#include <stdint.h>

int compute_sha256(const void *dati, size_t len, uint8_t output[32]);

#endif
