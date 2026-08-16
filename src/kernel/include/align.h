/* align.h
 * Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
 */

#ifndef _ALIGN_H
#define _ALIGN_H

/* ALIGN
 t: type
 x: value
 a: alignment. must be power of 2 */
#define ALIGN(t,x,a) (((t)(x) + (t)((a) - 1)) & ~((t)((a) - 1)))

#endif
