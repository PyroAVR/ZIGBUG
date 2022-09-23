#if !defined(__COMPILER_ABSTRACTION_H__)
#define __COMPILER_ABSTRACTION_H__

#if defined(GCC)
#define WEAK __attribute__((weak))
#else
#define WEAK
#endif
