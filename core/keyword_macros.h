#ifndef _MACRO_H_
#define _MACRO_H_

#define LIKELY(x)   __builtin_expect(!!(x), 1)
#define UNLIKELY(x) __builtin_expect(!!(x), 0)

#define FORCE_INLINE __attribute__((always_inline)) 

#endif