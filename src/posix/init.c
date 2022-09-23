#include <interface.h>

#include <posix_private.h>

#include <termios.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static const struct sigaction sigint_action = {
    .sa_handler = port_teardown,
    .sa_mask = SA_NOMASK,
    .sa_flags = ~SA_SIGINFO,
};

void port_init(void) {
    memset(simulated_memory, 0, POSIX_SIM_MEM_SIZE);
    sigaction(SIGINT, &sigint_action, NULL);
}

void port_teardown(void) {
    printf("%s\n", "");
    // restore terminal state to sane and normal status
    tcsetattr(fileno(stdin), TCSANOW, &orig_stdin_settings);
    tcsetattr(fileno(stdout), TCSANOW, &orig_stdout_settings);
    exit(0);
}
