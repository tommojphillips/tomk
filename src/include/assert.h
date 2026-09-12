/* assert.h
 * Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
 */

#ifndef _ASSERT_H
#define _ASSERT_H

#define ASSERT_ENABLE

#ifdef ASSERT_ENABLE
#include <kpanic.h>
#define STRINGIFY_(x) #x
#define STRINGIFY(x) STRINGIFY_(x)
#define assert(x) do { if (!(x)) { kpanic("ASSERT: \""#x"\" FAILED. in " __FILE__ ":" STRINGIFY(__LINE__) "\n"); } } while(0)
#else
#define assert(x, ...)
#endif

#endif
