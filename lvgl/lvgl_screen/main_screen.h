/**
 * Main Screen Header
 * 
 * Main navigation screen with buttons to access different features
 */

#ifndef MAIN_SCREEN_H
#define MAIN_SCREEN_H

#include "lvgl.h"

/**
 * Create the main screen
 */
void main_screen_create(void);

/**
 * Get the main screen object
 * @return pointer to the main screen object
 */
lv_obj_t* main_screen_get_screen(void);

#endif // MAIN_SCREEN_H
