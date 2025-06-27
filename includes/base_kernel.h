// In your includes/base_kernel.h:

// You'll need these standard string functions.
// If your current kernel doesn't have them, you'll need to add
// simple implementations to utils.c and declare them here.
extern size_t strlen(const char* str);
extern char* strcpy(char* dest, const char* src);
extern int strcmp(const char* s1, const char* s2);
extern char* strstr(const char* haystack, const char* needle);
extern char* strncpy(char* dest, const char* src, size_t n);

// (And ensure int_to_str is declared as it's used for printing file sizes)
void int_to_str(int n, char* str);
