/* assert.h
 * Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
 */

#ifndef _ASSERT_H
#define _ASSERT_H

#include <kernel.h>

#define ASSERT_ENABLE

#ifdef ASSERT_ENABLE
#define STRINGIFY_(x) #x
#define STRINGIFY(x) STRINGIFY_(x)
#define assert(x, s) do { if (!(x)) { printf("ASSERT: \""#x"\" FAILED.\n in " __FILE__ " Ln " STRINGIFY(__LINE__) "\n"); kernel_panic(s); } } while(0)

#else
#define assert(x, ...)
#endif

#endif
