/* stackframe.c
 * Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
 */

#include <stdint.h>
#include <stddef.h>

#include <stackframe.h>

stack_frame_t* stackframe_next(stack_frame_t* frame) {
    if (frame == NULL) {
        return NULL;
    }
    return frame->caller;
}
