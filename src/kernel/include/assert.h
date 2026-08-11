/* assert.h
 * Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
 */

#ifndef _ASSERT_H
#define _ASSERT_H

#include <kernel.h>

#define ASSERT_ENABLE
#ifdef ASSERT_ENABLE
#define assert(x, ...) do { if (!(x)) { kernel_panic(__VA_ARGS__); } } while(0);
#else
#define assert(x, ...)
#endif

#endif
