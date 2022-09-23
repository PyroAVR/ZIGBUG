#if !defined(__INTERFACE_H__)
#define __INTERFACE_H__
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
/**
 * ROM Monitor main interface
 */
/*******************************************************************************
 * Pre-terminal system initialization
 ******************************************************************************/

/**
 * Initialize the system aside from the character terminal interface
 */
void port_init(void);

/**
 * Main loop housekeeping tasks required by the port
 */
void port_tasks(void);

/**
 * Any teardown actions that are needed on monitor exit should be here
 */
void port_teardown(void);

/*******************************************************************************
 * Character terminal interface
 ******************************************************************************/

/**
 * Initialize communication primitives (UART, for example)
 */
void term_init(void *ctx);

/**
 * Write a NULL-terminated string to the terminal
 */
void term_puts(void *ctx, char *s, size_t maxlen);

/**
 * Read a byte from the terminal
 */
char term_getc(void *ctx, bool *ok);

/*******************************************************************************
 * Bare minimum features: Memory Read, Memory Write
 ******************************************************************************/

/**
 * Read or write from / to memory
 */
int memrw(uintptr_t addr, char *buf, size_t len, size_t width, bool write);

/*******************************************************************************
 * Extra features are included here for convenience when enabled
 ******************************************************************************/

#if defined(FEATURE_IHEX) && FEATURE_IHEX == 1
#include <intelhex.h>
#endif

#if defined(FEATURE_DUMP) && FEATURE_DUMP == 1
#include <dumpmem.h>
#endif

#endif
