#if !defined(__UTIL_STRING_H__)
#define __UTIL_STRING_H__
#include <stdbool.h>

char tolower(char c);

char toupper(char c);

bool ishex(char c);

// FIXME not a great interface here
static void int2hex(char *buf, char byte);
static uintmax_t hex2int(char *buf, size_t len);
#endif
