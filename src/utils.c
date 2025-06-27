// In src/utils.c (or similar utility file)

#include <stdint.h>
#include <stddef.h>

// Your existing int_to_str and hex_to_str functions...

// Basic strlen
size_t strlen(const char* str) {
    size_t len = 0;
    while (str[len] != '\0') {
        len++;
    }
    return len;
}

// Basic strcpy
char* strcpy(char* dest, const char* src) {
    char* original_dest = dest;
    while ((*dest++ = *src++) != '\0');
    return original_dest;
}

// Basic strcmp
int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

// Basic strstr
char* strstr(const char* haystack, const char* needle) {
    size_t needle_len = strlen(needle);
    if (needle_len == 0) {
        return (char*)haystack;
    }
    for (size_t i = 0; haystack[i] != '\0'; i++) {
        if (strlen(haystack + i) < needle_len) {
            return NULL;
        }
        if (strncmp(haystack + i, needle, needle_len) == 0) {
            return (char*)(haystack + i);
        }
    }
    return NULL;
}

// Basic strncpy
char* strncpy(char* dest, const char* src, size_t n) {
    size_t i;
    for (i = 0; i < n && src[i] != '\0'; i++) {
        dest[i] = src[i];
    }
    for (; i < n; i++) {
        dest[i] = '\0';
    }
    return dest;
}

// Note: strncmp is needed for strstr.
// You might need to add strncmp too if not available from a standard library.
// int strncmp(const char* s1, const char* s2, size_t n);
// int strncmp(const char* s1, const char* s2, size_t n) {
//     for (size_t i = 0; i < n; i++) {
//         if (s1[i] != s2[i]) {
//             return (unsigned char)s1[i] - (unsigned char)s2[i];
//         }
//         if (s1[i] == '\0') {
//             return 0;
//         }
//     }
//     return 0;
// }
