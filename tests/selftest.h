#ifndef DCC_SELFTEST_H
#define DCC_SELFTEST_H

#include <stdint.h>

typedef struct {
    uint32_t magic;
    uint32_t completed;
    uint32_t failures;
    uint32_t first_failure;
    uint8_t sample[16];
} dcc_test_result;

extern volatile dcc_test_result dcc_result;
void dcc_selftest(void);

#endif
