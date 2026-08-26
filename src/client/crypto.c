#include <openssl/evp.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

int compute_sha256(const void *dati, size_t len, uint8_t output[32])
{
	EVP_MD_CTX *ctx = EVP_MD_CTX_new();
	if (!ctx)
		return 0;

	unsigned int out_len = 0;
	int success = 0;

	if (EVP_DigestInit_ex(ctx, EVP_sha256(), NULL) == 1 && EVP_DigestUpdate(ctx, dati, len) == 1 &&
		EVP_DigestFinal_ex(ctx, output, &out_len) == 1)
	{
		success = (out_len == 32);
	}

	EVP_MD_CTX_free(ctx);
	return success;
}
