/**
 * Stepper Motor Control Screen Implementation nd manages the LVGL UI for stepper motor control
 */

#include "stepper_screen.h"
#include "screen_manager.h"
#include "../../drivers/stepper/stepper_driver.h"
#include <stdio.h>

// Screen objects
lv_obj_t *steps_slider = NULL;
lv_obj_t *start_stop_btn = NULL;
lv_obj_t *progress_bar = NULL;
lv_obj_t *current_steps_label = NULL;
lv_obj_t *target_steps_label = NULL;
lv_obj_t *status_label = NULL;

// Private variables
static lv_obj_t *main_screen = NULL;
static char label_buffer[64];
static bool completion_handled = false;

// Event handlers
static void slider_event_cb(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * slider = lv_event_get_target(e);
    
    if(code == LV_EVENT_VALUE_CHANGED) {
        // Reset timeout automatically using helper function
        screen_manager_handle_ui_event(e);
        
        int32_t value = lv_slider_get_value(slider);
        
        // Update slider label with microstepping-aware units
        static char label_text[64];
        snprintf(label_text, sizeof(label_text), "Steps: %d (%.2f rev)", value, value / (float)STEPPER_STEPS_PER_REV);
        
        lv_label_set_text(target_steps_label, label_text);
    }
}

static void button_event_cb(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    
    if(code == LV_EVENT_CLICKED) {
        // Reset timeout automatically using helper function
        screen_manager_handle_ui_event(e);
        
        if (stepper_driver_is_running()) {
            // Stop the stepper motor (enable pin automatically disabled)
            stepper_driver_stop();
            lv_label_set_text(lv_obj_get_child(start_stop_btn, 0), "START");
            stepper_screen_set_status("Stopped (Driver Disabled)");
            completion_handled = false; // Reset for next movement
        } else {
            // Start the stepper motor (enable pin automatically enabled)
            int32_t target_steps = lv_slider_get_value(steps_slider);
            
            stepper_driver_start(target_steps);
            lv_label_set_text(lv_obj_get_child(start_stop_btn, 0), "STOP");
            stepper_screen_set_status("Running");
            completion_handled = false; // Reset completion flag for new movement
            
            // Reset progress bar
            lv_bar_set_value(progress_bar, 0, LV_ANIM_OFF);
        }
    }
}

void stepper_screen_create(void) {
    // Create standalone screen (not as child of current screen)
    lv_obj_t *screen = lv_obj_create(NULL);
    
    // Create main container on the screen
    lv_obj_t *obj = lv_obj_create(screen);
    lv_obj_set_size(obj, lv_pct(95), lv_pct(90));
    lv_obj_align(obj, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(obj, 10, 0);
    
    main_screen = screen;  // Store reference to screen (not container)
    
    // Steps slider section (removed title to save space)
    lv_obj_t *slider_label = lv_label_create(obj);
    lv_label_set_text(slider_label, "Number of Steps:");
    lv_obj_align(slider_label, LV_ALIGN_TOP_MID, 0, 15);  // Centered
    
    steps_slider = lv_slider_create(obj);
    lv_slider_set_range(steps_slider, MIN_STEPS, MAX_STEPS);
    lv_slider_set_value(steps_slider, DEFAULT_STEPS, LV_ANIM_OFF);
    lv_obj_set_size(steps_slider, lv_pct(80), 30);
    lv_obj_align(steps_slider, LV_ALIGN_TOP_MID, 0, 40);  // Centered
    lv_obj_add_event_cb(steps_slider, slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    
    // Target steps label
    target_steps_label = lv_label_create(obj);
    snprintf(label_buffer, sizeof(label_buffer), "Steps: %d (%.2f rev)", DEFAULT_STEPS, DEFAULT_STEPS / (float)STEPPER_STEPS_PER_REV);
    lv_label_set_text(target_steps_label, label_buffer);
    lv_obj_align(target_steps_label, LV_ALIGN_TOP_MID, 0, 80);  // Centered
    
    // Current steps label
    current_steps_label = lv_label_create(obj);
    lv_label_set_text(current_steps_label, "Current Steps: 0");
    lv_obj_align(current_steps_label, LV_ALIGN_TOP_MID, 0, 105);  // Centered
    
    // Progress bar (removed "Progress:" label to save space)
    progress_bar = lv_bar_create(obj);
    lv_obj_set_size(progress_bar, lv_pct(80), 20);
    lv_obj_align(progress_bar, LV_ALIGN_TOP_MID, 0, 135);  // Centered
    lv_bar_set_range(progress_bar, 0, 100);
    lv_bar_set_value(progress_bar, 0, LV_ANIM_OFF);
    
    // Start/Stop button - moved up due to space savings
    start_stop_btn = lv_btn_create(obj);
    lv_obj_set_size(start_stop_btn, 120, 50);
    lv_obj_align(start_stop_btn, LV_ALIGN_TOP_MID, 0, 170);  // Moved up from 195 to 170
    lv_obj_add_event_cb(start_stop_btn, button_event_cb, LV_EVENT_CLICKED, NULL);
    
    lv_obj_t *btn_label = lv_label_create(start_stop_btn);
    lv_label_set_text(btn_label, "START");
    lv_obj_center(btn_label);
    
    // Status label - moved up below button
    status_label = lv_label_create(obj);
    lv_label_set_text(status_label, "Ready");
    lv_obj_align(status_label, LV_ALIGN_TOP_MID, 0, 230);  // Moved up from 255 to 230
    
    printf("Stepper screen created successfully\n");
}

void stepper_screen_update_progress(void) {
    if (!main_screen) return;
    
    // Get current stepper state
    int32_t current_steps = stepper_driver_get_current_steps();
    int32_t target_steps = stepper_driver_get_target_steps();
    bool is_running = stepper_driver_is_running();
    
    // Update current steps label
    snprintf(label_buffer, sizeof(label_buffer), "Current Steps: %ld", current_steps);
    lv_label_set_text(current_steps_label, label_buffer);
    
    // Update progress bar
    if (target_steps > 0) {
        int32_t progress = (current_steps * 100) / target_steps;
        if (progress > 100) progress = 100;
        lv_bar_set_value(progress_bar, progress, LV_ANIM_OFF);
    }
    
    // Update status when movement completes
    if (!is_running && current_steps > 0 && !completion_handled) {
        if (current_steps >= target_steps) {
            // Enable pin is automatically disabled by the stepper driver
            stepper_screen_set_status("Completed");
            stepper_screen_set_button_text("START");
            completion_handled = true; // Mark completion as handled
        }
    }
}

int32_t stepper_screen_get_target_steps(void) {
    if (!steps_slider) return DEFAULT_STEPS;
    return lv_slider_get_value(steps_slider);
}

void stepper_screen_set_status(const char* status) {
    if (status_label) {
        lv_label_set_text(status_label, status);
    }
}

void stepper_screen_set_button_text(const char* text) {
    if (start_stop_btn) {
        lv_obj_t *btn_label = lv_obj_get_child(start_stop_btn, 0);
        if (btn_label) {
            lv_label_set_text(btn_label, text);
        }
    }
}

lv_obj_t* stepper_screen_get_screen(void) {
    return main_screen;
}
