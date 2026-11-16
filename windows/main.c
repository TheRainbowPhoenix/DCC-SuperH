#include "dcc.h"
#include "../tests/test_key.h"

#include <stdio.h>
#include <string.h>

int main(int argc, char **argv)
{
    const char *s = argc > 1 ? argv[1] : DCC_TEST_KEY_HEX;
    uint8_t key[16], out[16], buf[4096];
    size_t n = strlen(s), i, count, offset = 0;
    unsigned v;
    FILE *in, *dest;
    int failed;

    if ((argc != 1 && argc != 2 && argc != 4) || n < 2 || n > 32 || n % 2) {
        fprintf(stderr, "usage: dcc HEXKEY [INPUT OUTPUT] (1..16 key bytes)\n");
        return 1;
    }
    for (i = 0; i < n / 2; ++i) {
        char pair[3] = {s[i * 2], s[i * 2 + 1], 0};
        if (strspn(pair, "0123456789abcdefABCDEF") != 2 ||
            sscanf(pair, "%2x", &v) != 1) return 1;
        key[i] = (uint8_t)v;
    }
    if (dcc_keystream(key, n / 2, out)) return 1;
    if (argc < 4) {
        for (i = 0; i < 16; ++i) printf("%02X", out[i]);
        puts("");
        return 0;
    }
    if (!strcmp(argv[2], argv[3])) {
        fprintf(stderr, "input and output must be different files\n");
        return 1;
    }
    in = fopen(argv[2], "rb");
    if (!in) {
        perror(argv[2]);
        return 1;
    }
    dest = fopen(argv[3], "wb");
    if (!dest) {
        perror(argv[3]);
        fclose(in);
        return 1;
    }
    while ((count = fread(buf, 1, sizeof(buf), in)) != 0) {
        for (i = 0; i < count; ++i) buf[i] ^= out[(offset + i) & 15];
        offset = (offset + count) & 15;
        if (fwrite(buf, 1, count, dest) != count) break;
    }
    failed = ferror(in) || ferror(dest);
    failed |= fclose(in) != 0;
    failed |= fclose(dest) != 0;
    if (failed) {
        fprintf(stderr, "file I/O failed\n");
        return 1;
    }
    return 0;
}
