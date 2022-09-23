#pragma once

#include <stdint.h>

#if !defined(POSIX_SIM_MEM_SIZE)
#error no definition provided for POSIX_SIM_MEM_SIZE
#endif

extern uint8_t simulated_memory[POSIX_SIM_MEM_SIZE];
extern struct termios orig_stdin_settings;
extern struct termios orig_stdout_settings;
