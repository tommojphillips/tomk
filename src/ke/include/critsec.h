/* critsec.h
 * Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
 */

#ifndef _CRITSEC_H
#define _CRITSEC_H

/* Enter critical section 
 Disables interrupts */
void critsec_enter(void);

/* Leave critical section 
 Enables interrupts */
void critsec_leave(void);

#endif
