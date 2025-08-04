/**
 * Screen Manager Implementation
 * 
 * Manages navigation between lock screen and stepper screen
 * Lock screen activates on timeout (30 seconds), stepper screen on touch
 * Includes dynamic CPU frequency scaling for power efficiency
 */

#include "screen_manager.h"
#include <stdio.h>
#include "pico/time.h"  // Add this for hardware-independent timing

// Screen management
static lv_obj_t* screens[SCREEN_COUNT] = {NULL};
static screen_id_t current_screen = SCREEN_LOCK;
static uint32_t last_activity_time = 0;
static uint32_t lock_screen_start_time = 0;  // Track when lock screen was activated
static const uint32_t TIMEOUT_MS = 30000;  // 30 seconds timeout
static const uint32_t FADE_ANIMATION_MS = 250;  // Fade animation duration

// CPU frequency management
static void (*cpu_frequency_callback)(uint32_t freq_khz) = NULL;
static const uint32_t CPU_FREQ_HIGH = 220000;  // 220MHz for active use
static const uint32_t CPU_FREQ_LOW = 48000;    // 48MHz for lock screen

// Touch event handler for lock screen
static void lock_screen_touch_event_cb(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    
    if (code == LV_EVENT_CLICKED || code == LV_EVENT_PRESSED) {
        // Boost CPU frequency BEFORE screen transition for smooth animation
        if (cpu_frequency_callback) {
            cpu_frequency_callback(CPU_FREQ_HIGH);
        }
        
        screen_manager_handle_touch();
    }
}

void screen_manager_init(void) {
    // Initialize screens array
    for (int i = 0; i < SCREEN_COUNT; i++) {
        screens[i] = NULL;
    }
    current_screen = SCREEN_LOCK;
    last_activity_time = to_ms_since_boot(get_absolute_time());  // Use hardware timer
    lock_screen_start_time = last_activity_time;  // Initialize lock screen timing
    
    printf("Screen manager initialized with lock screen behavior\n");
}

void screen_manager_add_screen(screen_id_t screen_id, lv_obj_t* screen_obj) {
    if (screen_id < SCREEN_COUNT && screen_obj != NULL) {
        screens[screen_id] = screen_obj;
        
        // Add appropriate event handlers based on screen type
        if (screen_id == SCREEN_LOCK) {
            // Lock screen responds to any touch to unlock
            lv_obj_add_event_cb(screen_obj, lock_screen_touch_event_cb, LV_EVENT_CLICKED, NULL);
            lv_obj_add_event_cb(screen_obj, lock_screen_touch_event_cb, LV_EVENT_PRESSED, NULL);
        }
        // Note: Individual UI elements handle their own timeout resets using screen_manager_handle_ui_event()
        
        printf("Screen %d added to manager\n", screen_id);
    }
}

void screen_manager_switch_to(screen_id_t screen_id) {
    if (screen_id < SCREEN_COUNT && screens[screen_id] != NULL) {
        // Handle CPU frequency changes based on target screen
        if (cpu_frequency_callback) {
            if (screen_id == SCREEN_LOCK && current_screen != SCREEN_LOCK) {
                // Switching to lock screen - reduce CPU after animation
                lock_screen_start_time = to_ms_since_boot(get_absolute_time());  // Track when lock screen starts
            } else if (screen_id == SCREEN_STEPPER && current_screen != SCREEN_STEPPER) {
                // Switching to stepper screen - ensure high CPU (should already be set by touch handler)
                cpu_frequency_callback(CPU_FREQ_HIGH);
            }
        }
        
        // Load the new screen with fade animation (500ms duration)
        lv_scr_load_anim(screens[screen_id], LV_SCR_LOAD_ANIM_FADE_IN, FADE_ANIMATION_MS, 0, false);
        current_screen = screen_id;
        
        // If switching to lock screen, reduce CPU frequency after animation delay
        if (screen_id == SCREEN_LOCK && cpu_frequency_callback) {
            // Schedule CPU reduction after animation completes (500ms + small buffer)
            // Note: In a real implementation, you might want to use an LVGL timer for this
            // For now, this will be handled in the main loop
        }
        
        printf("Switched to screen %d with fade animation\n", screen_id);
    } else {
        printf("ERROR: Invalid screen ID %d or screen not initialized\n", screen_id);
    }
}

screen_id_t screen_manager_get_current(void) {
    return current_screen;
}

void screen_manager_next_screen(void) {
    screen_id_t next_screen = (current_screen + 1) % SCREEN_COUNT;
    screen_manager_switch_to(next_screen);
}

void screen_manager_prev_screen(void) {
    screen_id_t prev_screen = (current_screen + SCREEN_COUNT - 1) % SCREEN_COUNT;
    screen_manager_switch_to(prev_screen);
}

void screen_manager_handle_touch(void) {
    // Switch from lock screen to stepper screen on touch
    if (current_screen == SCREEN_LOCK) {
        screen_manager_switch_to(SCREEN_STEPPER);
        screen_manager_reset_timeout();
    }
}

void screen_manager_update(void) {
    // Check for timeout to return to lock screen
    if (current_screen != SCREEN_LOCK) {
        uint32_t current_time = to_ms_since_boot(get_absolute_time());  // Use hardware timer
        uint32_t elapsed_time = current_time - last_activity_time;
        
        if (elapsed_time > TIMEOUT_MS) {
            printf("Timeout reached after %lu ms - returning to lock screen\n", elapsed_time);
            screen_manager_switch_to(SCREEN_LOCK);
        }
    }
}

void screen_manager_reset_timeout(void) {
    last_activity_time = to_ms_since_boot(get_absolute_time());  // Use hardware timer
}

// Helper function for UI elements to handle events and automatically reset timeout
void screen_manager_handle_ui_event(lv_event_t * e) {
    // Always reset timeout when any UI event occurs (except on lock screen)
    if (current_screen != SCREEN_LOCK) {
        screen_manager_reset_timeout();
    }
}

// Set CPU frequency callback for power management
void screen_manager_set_cpu_callback(void (*callback)(uint32_t freq_khz)) {
    cpu_frequency_callback = callback;
}

// Check if lock screen should update (avoid updates during fade transition)
bool screen_manager_lock_screen_ready_for_updates(void) {
    if (current_screen == SCREEN_LOCK && lock_screen_start_time > 0) {
        uint32_t current_time = to_ms_since_boot(get_absolute_time());
        uint32_t time_since_lock_screen = current_time - lock_screen_start_time;
        // Wait for fade animation to complete plus buffer before allowing time updates
        return (time_since_lock_screen > (FADE_ANIMATION_MS + 100));  // Animation + 100ms buffer
    }
    return (current_screen == SCREEN_LOCK);
}

// Check if we should reduce CPU frequency after lock screen animation
bool screen_manager_should_reduce_cpu(void) {
    // Reduce CPU frequency if we're on lock screen and enough time has passed for animation
    if (current_screen == SCREEN_LOCK && lock_screen_start_time > 0) {
        uint32_t current_time = to_ms_since_boot(get_absolute_time());  // Use hardware timer
        // Allow time for animation to complete before reducing CPU
        uint32_t time_since_lock_screen = current_time - lock_screen_start_time;
        return (time_since_lock_screen > (FADE_ANIMATION_MS + 200));  // Animation duration + 200ms buffer
    }
    return false;
}
