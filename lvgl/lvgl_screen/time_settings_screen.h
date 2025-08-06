/**
 * Time Settings Screen Header
 * 
 * Screen for setting RTC time, date, month, and year
 */

#ifndef TIME_SETTINGS_SCREEN_H
#define TIME_SETTINGS_SCREEN_H

#include "lvgl.h"
#include <time.h>

/**
 * Create the time settings screen
 */
void time_settings_screen_create(void);

/**
 * Get the time settings screen object
 * @return pointer to the time settings screen object
 */
lv_obj_t* time_settings_screen_get_screen(void);

/**
 * Load current RTC time into the screen controls
 */
void time_settings_screen_load_current_time(void);

/**
 * Save the current screen values to RTC and return to main screen
 */
void time_settings_screen_save_and_return(void);

#endif // TIME_SETTINGS_SCREEN_H
