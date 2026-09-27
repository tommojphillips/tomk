/* stackframe.h
 * Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
 */

#ifndef _STACK_FRAME_H
#define _STACK_FRAME_H

#include <stdint.h>

/* Stack Frame */
typedef struct _stack_frame stack_frame_t;

/* Stack Frame */
struct _stack_frame {
    stack_frame_t* caller;   /* saved ebp */
    uint32_t return_address; /* [ebp+4]*/
};

/* Get next stackframe
 Returns NULL if at end of call stack, otherwise returns the next stack frame */
stack_frame_t* stackframe_next(stack_frame_t* frame);

#endif
