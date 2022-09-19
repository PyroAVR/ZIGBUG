#if !defined(__INTERFACE_H__)
#define __INTERFACE_H__
#include <stdint.h>
#include <stddef.h>
/**
 * ROM Monitor main interface
 */

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
void puts(void *ctx, char *s);

/**
 * Read a byte from the terminal
 */
char getc(void *ctx, bool *ok);

/*******************************************************************************
 * Bare minimum features: Memory Read, Memory Write
 ******************************************************************************/

/**
 * Read or write from / to memory
 */
void memrw(uintptr_t addr, char *buf, size_t len, size_t width, bool wr);

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
