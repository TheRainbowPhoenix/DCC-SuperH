#ifndef DCC_H
#define DCC_H

#include <stddef.h>
#include <stdint.h>

int dcc_keystream(const uint8_t *key, size_t length, uint8_t out[16]);

void dcc_xor(const uint8_t stream[16], const void *input, void *output, size_t length);

#endif
