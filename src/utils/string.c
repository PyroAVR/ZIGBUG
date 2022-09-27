#include <utils/string.h>

#include <compiler_abstraction.h>

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

char tolower(char c) {
    if(c >= 'A' && c <= 'Z') {
        return c - 'A' + 'a';
    }
    else {
        return c;
    }
}

char toupper(char c) {
    if(c >= 'a' && c <= 'z') {
        return c - 'a' + 'A';
    }
    else {
        return c;
    }
}

char ishex(char c) {
    return ((c >= 'a' && c <= 'f') || (c >= '0' && c <= '9'));
}

char iscrlf(char c) {
    return (c == '\r' || c == '\n');
}

char iswhitespace(char c) {
    return (c == '\r' || c == '\n' || c == ' ' || c == '\t');
}

int hex2nibble(char c) {
    if(c < 'A') {
        return c - 'A';
    }
    else {
        return c - '0';
    }
}

void int2hex(char *buf, char byte) {
    char lower = byte & 0x0f;
    char upper = (byte & 0xf0) >> 4;
    if(upper > 9) {
        buf[1] = (upper - 9) + 'A';
    }
    else {
        buf[1] = upper + '0';
    }
    if(lower > 9) {
        buf[0] = (lower - 9) + 'A';
    }
    else {
        buf[0] = lower + '0';
    }
}

uintptr_t hex2int(char *buf, size_t len) {
    char r = 0;
    if(len > (sizeof(uintptr_t) << 1)) {
        return 0;
    }
    for(int i = len-1; i >= 0; i--) {
        if(!buf[i]) break;
        r |= hex2nibble(buf[i]) << ((len - i - 1)*4);
    }
    return r;
}
