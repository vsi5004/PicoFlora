/**
 * Main Screen Implementation
 * 
 * Main navigation screen with buttons to access stepper control and time settings
 */

#include "main_screen.h"
#include "screen_manager.h"
#include "../../drivers/logging/logging.h"

// Screen object
static lv_obj_t *main_screen = NULL;

// UI elements
static lv_obj_t *stepper_btn = NULL;
static lv_obj_t *time_settings_btn = NULL;
static lv_obj_t *title_label = NULL;

// Event handlers
static void stepper_btn_event_cb(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        screen_manager_handle_ui_event(e);  // Reset timeout
        screen_manager_switch_to(SCREEN_STEPPER);
        LOG_UI_INFO("Navigating to stepper screen");
    }
}

static void time_settings_btn_event_cb(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        screen_manager_handle_ui_event(e);  // Reset timeout
        screen_manager_switch_to(SCREEN_TIME_SETTINGS);
        LOG_UI_INFO("Navigating to time settings screen");
    }
}

void main_screen_create(void) {
    // Create the main screen
    main_screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(main_screen, lv_color_hex(0x1e1e1e), LV_PART_MAIN);  // Dark background
    
    // Create title label
    title_label = lv_label_create(main_screen);
    lv_label_set_text(title_label, "PicoFlora");
    lv_obj_set_style_text_color(title_label, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_font(title_label, &lv_font_montserrat_16, LV_PART_MAIN);
    lv_obj_align(title_label, LV_ALIGN_TOP_MID, 0, 20);
    
    // Create stepper control button
    stepper_btn = lv_btn_create(main_screen);
    lv_obj_set_size(stepper_btn, 200, 60);
    lv_obj_align(stepper_btn, LV_ALIGN_CENTER, 0, -40);
    lv_obj_add_event_cb(stepper_btn, stepper_btn_event_cb, LV_EVENT_CLICKED, NULL);
    
    // Style the stepper button
    lv_obj_set_style_bg_color(stepper_btn, lv_color_hex(0x4CAF50), LV_PART_MAIN);  // Green
    lv_obj_set_style_bg_color(stepper_btn, lv_color_hex(0x45a049), LV_STATE_PRESSED);
    lv_obj_set_style_radius(stepper_btn, 8, LV_PART_MAIN);
    
    // Create stepper button label
    lv_obj_t *stepper_btn_label = lv_label_create(stepper_btn);
    lv_label_set_text(stepper_btn_label, "Stepper Control");
    lv_obj_set_style_text_color(stepper_btn_label, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_font(stepper_btn_label, &lv_font_montserrat_16, LV_PART_MAIN);
    lv_obj_center(stepper_btn_label);
    
    // Create time settings button
    time_settings_btn = lv_btn_create(main_screen);
    lv_obj_set_size(time_settings_btn, 200, 60);
    lv_obj_align(time_settings_btn, LV_ALIGN_CENTER, 0, 40);
    lv_obj_add_event_cb(time_settings_btn, time_settings_btn_event_cb, LV_EVENT_CLICKED, NULL);
    
    // Style the time settings button
    lv_obj_set_style_bg_color(time_settings_btn, lv_color_hex(0x2196F3), LV_PART_MAIN);  // Blue
    lv_obj_set_style_bg_color(time_settings_btn, lv_color_hex(0x1976D2), LV_STATE_PRESSED);
    lv_obj_set_style_radius(time_settings_btn, 8, LV_PART_MAIN);
    
    // Create time settings button label
    lv_obj_t *time_settings_btn_label = lv_label_create(time_settings_btn);
    lv_label_set_text(time_settings_btn_label, "Time Settings");
    lv_obj_set_style_text_color(time_settings_btn_label, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_font(time_settings_btn_label, &lv_font_montserrat_16, LV_PART_MAIN);
    lv_obj_center(time_settings_btn_label);
    
    LOG_UI_INFO("Main screen created with navigation buttons");
}

lv_obj_t* main_screen_get_screen(void) {
    return main_screen;
}
