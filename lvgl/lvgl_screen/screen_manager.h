/**
 * Screen Manager Header
 * 
 * Manages navigation between multiple LVGL screens with touch unlock
 * and dynamic CPU frequency scaling for power efficiency
 */

#ifndef SCREEN_MANAGER_H
#define SCREEN_MANAGER_H

#include "lvgl.h"

typedef enum {
    SCREEN_LOCK = 0,
    SCREEN_STEPPER,
    SCREEN_COUNT
} screen_id_t;

/**
 * Initialize the screen manager
 */
void screen_manager_init(void);

/**
 * Add a screen to the manager
 * @param screen_id ID of the screen
 * @param screen_obj LVGL screen object
 */
void screen_manager_add_screen(screen_id_t screen_id, lv_obj_t* screen_obj);

/**
 * Switch to a specific screen
 * @param screen_id ID of the screen to switch to
 */
void screen_manager_switch_to(screen_id_t screen_id);

/**
 * Get the current active screen ID
 * @return current screen ID
 */
screen_id_t screen_manager_get_current(void);

/**
 * Switch to the next screen (with wrap-around)
 */
void screen_manager_next_screen(void);

/**
 * Switch to the previous screen (with wrap-around)
 */
void screen_manager_prev_screen(void);

/**
 * Handle touch event - switches from lock screen to stepper screen
 */
void screen_manager_handle_touch(void);

/**
 * Update screen manager - handles timeout back to lock screen
 */
void screen_manager_update(void);

/**
 * Reset the timeout timer (call when user interacts)
 */
void screen_manager_reset_timeout(void);

/**
 * Helper function for UI elements to handle events and automatically reset timeout
 * Call this from your event handlers instead of manual screen_manager_reset_timeout()
 * @param e LVGL event (can be NULL if you don't need event-specific logic)
 */
void screen_manager_handle_ui_event(lv_event_t * e);

/**
 * Set CPU frequency change callback
 * @param callback Function to call when CPU frequency should change
 */
void screen_manager_set_cpu_callback(void (*callback)(uint32_t freq_khz));

/**
 * Check if CPU frequency should be reduced for power saving
 * @return true if CPU frequency should be reduced
 */
bool screen_manager_should_reduce_cpu(void);

/**
 * Check if lock screen is ready for time updates (after fade animation)
 * @return true if lock screen should update time display
 */
bool screen_manager_lock_screen_ready_for_updates(void);

#endif // SCREEN_MANAGER_H
