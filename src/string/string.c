#include "string.h"

__attribute__((__optimize__("-fno-tree-loop-distribute-patterns"))) void*
memset(void* dest, int c, size_t n)
{
    unsigned char* p = (unsigned char*)dest;

    while (n--)
        *p++ = (unsigned char)c;

    return dest;
}

__attribute__((__optimize__("-fno-tree-loop-distribute-patterns"))) void*
memcpy(void* dest, const void* src, size_t n)
{
    unsigned char* d = (unsigned char*)dest;
    const unsigned char* s = (const unsigned char*)src;

    while (n--)
        *d++ = *s++;

    return dest;
}

__attribute__((__optimize__("-fno-tree-loop-distribute-patterns"))) void*
memmove(void* dest, const void* src, size_t n)
{
    unsigned char* d = (unsigned char*)dest;
    const unsigned char* s = (const unsigned char*)src;

    if (d < s)
        while (n--)
            *d++ = *s++;
    else {
        d += n;
        s += n;
        while (n--)
            *--d = *--s;
    }

    return dest;
}

__attribute__((__optimize__("-fno-tree-loop-distribute-patterns"))) int
memcmp(const void* s1, const void* s2, size_t n)
{
    const unsigned char* a = (const unsigned char*)s1;
    const unsigned char* b = (const unsigned char*)s2;

    while (n--) {
        if (*a != *b)
            return *a - *b;

        a++;
        b++;
    }

    return 0;
}

size_t strlen(const char* str)
{
    size_t len = 0;

    while (str[len])
        len++;

    return len;
}
