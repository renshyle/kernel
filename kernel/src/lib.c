#include <stdlib.h>

void *memcpy(void *restrict s1, const void *restrict s2, size_t n)
{
    char *s1c = s1;
    const char *s2c = s2;

    for (size_t i = 0; i < n; i++) {
        s1c[i] = s2c[i];
    }

    return s1;
}

void *memset(void *s, int c, size_t n)
{
    char *s1 = s;

    for (size_t i = 0; i < n; i++) {
        s1[i] = (unsigned char) c;
    }

    return s;
}
