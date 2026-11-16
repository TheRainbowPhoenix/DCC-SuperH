#include "dcc.h"
#include "selftest.h"
#include "test_key.h"

volatile dcc_test_result dcc_result;

static int equal(const uint8_t *a, const uint8_t *b, unsigned length)
{
    unsigned i;
    for (i = 0; i < length; ++i) {
        if (a[i] != b[i]) return 0;
    }
    return 1;
}

static void record(int passed)
{
    if (!passed) {
        if (!dcc_result.failures) dcc_result.first_failure = dcc_result.completed;
        ++dcc_result.failures;
    }
    ++dcc_result.completed;
}

void dcc_selftest(void)
{
    uint8_t actual[16];
    union aligned_buffer {
        uint32_t alignment;
        uint8_t bytes[4100];
    } src, dst;
    static const unsigned lengths[] = {0, 1, 15, 16, 17, 31, 32, 33, 4097};
    unsigned j, a, inplace, n;

    dcc_result.magic = 0;
    dcc_result.completed = 0;
    dcc_result.failures = 0;
    dcc_result.first_failure = 0xffffffffu;
    record(dcc_keystream(test_key, sizeof(test_key), actual) == 0 &&
           equal(actual, test_stream, sizeof(test_stream)));
    for (j = 0; j < 16; ++j) dcc_result.sample[j] = actual[j];
    for (n = 0; n < sizeof(lengths) / sizeof(lengths[0]); ++n) {
        for (a = 0; a < 2; ++a) {
            for (inplace = 0; inplace < 2; ++inplace) {
                uint8_t *input = src.bytes + a;
                uint8_t *output = inplace ? input : dst.bytes + a;
                unsigned length = lengths[n];
                int passed = 1;
                for (j = 0; j < length; ++j) input[j] = (uint8_t)(j * 73 + 19);
                dcc_xor(actual, input, output, length);
                for (j = 0; j < length; ++j) {
                    if (output[j] != ((uint8_t)(j * 73 + 19) ^ actual[j & 15]))
                        passed = 0;
                }
                record(passed);
            }
        }
    }
    record(dcc_keystream(0, 1, actual) == -1);
    record(dcc_keystream(actual, 0, actual) == -1);
    record(dcc_keystream(actual, 17, actual) == -1);
    record(dcc_keystream(actual, 16, 0) == -1);
    dcc_result.magic = 0x44434331u;
}
