#include <interface.h>

#include <termios.h>
#include <stdbool.h>
#include <stdio.h>


struct termios orig_stdin_settings;
struct termios orig_stdout_settings;

/**
 * Make a POSIX terminal... dumber than normal
 * This is to emulate a serial console, where echo is done by the remote,
 * and all interface handling is similarly done by the remote.
 */
void term_init(void *ctx) {
    struct termios term_settings;
    
    tcgetattr(fileno(stdin), &orig_stdin_settings);
    tcgetattr(fileno(stdout), &orig_stdout_settings);

    tcgetattr(fileno(stdin), &term_settings);
    // no fancy handling, nor character echo
    term_settings.c_lflag &= ~(ICANON | ECHO);
    // generate signals for signal-generating "special characters"
    term_settings.c_lflag |= ISIG;
    tcsetattr(fileno(stdin), TCSANOW, &term_settings);

    return;
    tcgetattr(fileno(stdout), &term_settings);
    tcsetattr(fileno(stdout), TCSANOW, &term_settings);
}

char term_getc(void *ctx, bool *ok) {
    (void)ctx;
    char c = fgetc(stdin);
    if(ok != NULL) {
        *ok = (c != EOF);
    }
    return c;
}

void term_puts(void *ctx, char *s, size_t maxlen) {
    (void)ctx;
    char str_max[256];
    memcpy(str_max, s, maxlen);
    str_max[maxlen] = 0;
    fputs(str_max, stdout);
}
