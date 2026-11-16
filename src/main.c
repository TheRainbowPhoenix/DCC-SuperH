#include "dcc.h"
#include "selftest.h"
#include "test_key.h"

#include <gint/display.h>
#include <gint/keyboard.h>

int main(void)
{
    static const char digits[] = "0123456789ABCDEF";
    char stream[33];
    unsigned i;

    dcc_selftest();
    for (i = 0; i < 16; ++i) {
        stream[i * 2] = digits[dcc_result.sample[i] >> 4];
        stream[i * 2 + 1] = digits[dcc_result.sample[i] & 15];
    }
    stream[32] = 0;
    dclear(C_WHITE);
    dtext(12, 12, C_BLACK, "DCC SuperH");
    dprint(12, 40, C_BLACK, "%u checks, %u failures",
           (unsigned)dcc_result.completed, (unsigned)dcc_result.failures);
    dtext(12, 72, C_BLACK, "Key:");
    dtext(12, 92, C_BLACK, DCC_TEST_KEY_HEX);
    dtext(12, 124, C_BLACK, "Stream:");
    dtext(12, 144, C_BLACK, stream);
    dtext(12, 184, C_BLACK, "Press a key to exit.");
    dupdate();
    getkey();
    return 0;
}
