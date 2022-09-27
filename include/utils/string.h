#if !defined(__UTIL_STRING_H__)
#define __UTIL_STRING_H__

#include <stddef.h>
#include <stdint.h>

char tolower(char c);

char toupper(char c);

char ishex(char c);

char iscrlf(char c);

char iswhitespace(char c);

// FIXME not a great interface here
void int2hex(char *buf, char byte);
uintmax_t hex2int(char *buf, size_t len);
#endif
