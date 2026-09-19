/* cswitch.h
 * Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
 */

#ifndef _CSWITCH_H
#define _CSWITCH_H

/* Context switch interrupt handler */
extern void cswitch_handler(void);

/* Context switch */
extern void cswitch_load(void* context);

/* Save context */
extern void cswitch_save(void* context);

#endif
