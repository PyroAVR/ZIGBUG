#include <interface.h>

#include <posix_private.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

uint8_t simulated_memory[POSIX_SIM_MEM_SIZE];

int memrw(uintptr_t addr, char *buf, size_t len, size_t width, bool write) {
    // not currently simulated
    (void)width;
    // validate first
    if(buf == NULL) {
        return 0;
    }
    if(addr >= POSIX_SIM_MEM_SIZE) {
        return 0;
    }
    if(write) {
        memcpy(simulated_memory + addr, buf, len);
    }
    else {
        memcpy(buf, simulated_memory + addr, len);
    }
    return len;
}

void port_tasks(void) {
    // none for POSIX
}
