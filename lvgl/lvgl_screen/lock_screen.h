/**
 * Lock Screen Header
 * 
 * Black screen with white time/date display that serves as lock screen
 */

#ifndef LOCK_SCREEN_H
#define LOCK_SCREEN_H

#include "lvgl.h"

/**
 * Create the lock screen
 */
void lock_screen_create(void);

/**
 * Get the lock screen object
 * @return pointer to the lock screen object
 */
lv_obj_t* lock_screen_get_screen(void);

/**
 * Update the time/date display
 */
void lock_screen_update_time(void);

#endif // LOCK_SCREEN_H
