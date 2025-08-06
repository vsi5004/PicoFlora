/**
 * Time Settings Screen Implementation
 * 
 * Screen for setting RTC time, date, month, and year
 */

#include "time_settings_screen.h"
#include "screen_manager.h"
#include "../../libraries/bsp/bsp_pcf85063.h"
#include "../../drivers/logging/logging.h"

// Screen object
static lv_obj_t *time_settings_screen = NULL;

// UI elements
static lv_obj_t *calendar = NULL;
static lv_obj_t *hour_roller = NULL;
static lv_obj_t *minute_roller = NULL;
static lv_obj_t *save_btn = NULL;
static lv_obj_t *cancel_btn = NULL;
static lv_obj_t *title_label = NULL;
static lv_obj_t *time_label = NULL;

// Calendar state
static lv_calendar_date_t selected_date = {0};
static bool date_selected = false;

// Event handlers
static void roller_event_cb(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_VALUE_CHANGED) {
        screen_manager_handle_ui_event(e);  // Reset timeout when roller value changes
    }
}

static void calendar_event_cb(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_VALUE_CHANGED) {
        screen_manager_handle_ui_event(e);  // Reset timeout
        
        // Get the pressed date
        lv_res_t res = lv_calendar_get_pressed_date(calendar, &selected_date);
        if (res == LV_RES_OK) {
            date_selected = true;
            
            // Highlight the selected date
            lv_calendar_date_t highlighted_dates[1];
            highlighted_dates[0] = selected_date;
            lv_calendar_set_highlighted_dates(calendar, highlighted_dates, 1);
            
            LOG_UI_INFO("Calendar date selected and highlighted: %d-%02d-%02d", 
                       selected_date.year, selected_date.month, selected_date.day);
        } else {
            LOG_UI_ERROR("Failed to get pressed date from calendar");
        }
    }
}

static void save_btn_event_cb(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        screen_manager_handle_ui_event(e);  // Reset timeout
        time_settings_screen_save_and_return();
    }
}

static void cancel_btn_event_cb(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        screen_manager_handle_ui_event(e);  // Reset timeout
        screen_manager_switch_to(SCREEN_MAIN);
        LOG_UI_INFO("Time settings cancelled, returning to main screen");
    }
}

void time_settings_screen_create(void) {
    // Create the time settings screen
    time_settings_screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(time_settings_screen, lv_color_hex(0x1e1e1e), LV_PART_MAIN);  // Dark background
    
    // Enable scrolling for the screen
    lv_obj_set_scroll_dir(time_settings_screen, LV_DIR_VER);
    lv_obj_set_style_pad_all(time_settings_screen, 10, LV_PART_MAIN);
    
    // Create title label
    title_label = lv_label_create(time_settings_screen);
    lv_label_set_text(title_label, "Set Time & Date");
    lv_obj_set_style_text_color(title_label, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_font(title_label, &lv_font_montserrat_16, LV_PART_MAIN);
    lv_obj_align(title_label, LV_ALIGN_TOP_MID, 0, 5);
    
    // Create time label
    time_label = lv_label_create(time_settings_screen);
    lv_label_set_text(time_label, "Time:");
    lv_obj_set_style_text_color(time_label, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_font(time_label, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_align(time_label, LV_ALIGN_TOP_LEFT, 10, 35);
    
    // Create time container for rollers (positioned at top)
    lv_obj_t *time_container = lv_obj_create(time_settings_screen);
    lv_obj_set_size(time_container, 280, 80);
    lv_obj_align(time_container, LV_ALIGN_TOP_MID, 0, 55);
    lv_obj_set_style_bg_opa(time_container, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_opa(time_container, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_pad_all(time_container, 5, LV_PART_MAIN);
    lv_obj_set_flex_flow(time_container, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(time_container, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    
    // Create hour roller (00-23)
    hour_roller = lv_roller_create(time_container);
    lv_roller_set_options(hour_roller, 
        "00\n01\n02\n03\n04\n05\n06\n07\n08\n09\n"
        "10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n"
        "20\n21\n22\n23", LV_ROLLER_MODE_NORMAL);
    lv_obj_set_width(hour_roller, 100);
    lv_roller_set_visible_row_count(hour_roller, 3);
    lv_obj_set_style_text_color(hour_roller, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_bg_color(hour_roller, lv_color_hex(0x3a3a3a), LV_PART_MAIN);
    lv_obj_add_event_cb(hour_roller, roller_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    
    // Create minute roller (00-59)
    minute_roller = lv_roller_create(time_container);
    lv_roller_set_options(minute_roller, 
        "00\n01\n02\n03\n04\n05\n06\n07\n08\n09\n"
        "10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n"
        "20\n21\n22\n23\n24\n25\n26\n27\n28\n29\n"
        "30\n31\n32\n33\n34\n35\n36\n37\n38\n39\n"
        "40\n41\n42\n43\n44\n45\n46\n47\n48\n49\n"
        "50\n51\n52\n53\n54\n55\n56\n57\n58\n59", LV_ROLLER_MODE_NORMAL);
    lv_obj_set_width(minute_roller, 100);
    lv_roller_set_visible_row_count(minute_roller, 3);
    lv_obj_set_style_text_color(minute_roller, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_bg_color(minute_roller, lv_color_hex(0x3a3a3a), LV_PART_MAIN);
    lv_obj_add_event_cb(minute_roller, roller_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    
    // Create date label
    lv_obj_t *date_label = lv_label_create(time_settings_screen);
    lv_label_set_text(date_label, "Date:");
    lv_obj_set_style_text_color(date_label, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_font(date_label, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_align(date_label, LV_ALIGN_TOP_LEFT, 10, 145);
    
    // Create calendar for date selection (narrower and with built-in navigation)
    calendar = lv_calendar_create(time_settings_screen);
    lv_obj_set_size(calendar, 220, 280);  // Much taller height for very easy day selection
    lv_obj_align(calendar, LV_ALIGN_TOP_MID, 0, 165);
    
    // Style the calendar
    lv_obj_set_style_bg_color(calendar, lv_color_hex(0x2a2a2a), LV_PART_MAIN);
    lv_obj_set_style_text_color(calendar, lv_color_white(), LV_PART_MAIN);
    
    // Style calendar header (arrows and month/year display)
    lv_obj_set_style_text_color(calendar, lv_color_white(), LV_PART_ITEMS);
    lv_obj_set_style_bg_color(calendar, lv_color_hex(0x4a4a4a), LV_PART_ITEMS | LV_STATE_PRESSED);
    
    // Style calendar days to be more selectable
    lv_obj_set_style_text_color(calendar, lv_color_white(), LV_PART_ITEMS);
    lv_obj_set_style_bg_color(calendar, lv_color_hex(0x4CAF50), LV_PART_ITEMS | LV_STATE_PRESSED);  // Green when pressed
    lv_obj_set_style_bg_color(calendar, lv_color_hex(0x2E7D32), LV_PART_ITEMS | LV_STATE_CHECKED);  // Darker green when selected
    lv_obj_set_style_bg_opa(calendar, LV_OPA_COVER, LV_PART_ITEMS | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(calendar, LV_OPA_COVER, LV_PART_ITEMS | LV_STATE_CHECKED);
    
    // Style highlighted dates (selected dates)
    lv_obj_set_style_bg_color(calendar, lv_color_hex(0x4CAF50), LV_PART_ITEMS);  // Green background for highlighted dates
    lv_obj_set_style_text_color(calendar, lv_color_white(), LV_PART_ITEMS);      // White text for highlighted dates
    lv_obj_set_style_bg_opa(calendar, LV_OPA_80, LV_PART_ITEMS);                // Semi-transparent for highlighted dates
    
    // Enable header arrows for month/year navigation
    lv_calendar_header_arrow_create(calendar);
    
    // Add event callback for date selection
    lv_obj_add_event_cb(calendar, calendar_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    
    // Create Save button
    save_btn = lv_btn_create(time_settings_screen);
    lv_obj_set_size(save_btn, 100, 40);
    lv_obj_align(save_btn, LV_ALIGN_TOP_MID, -60, 460);  // Moved down for much taller calendar
    lv_obj_add_event_cb(save_btn, save_btn_event_cb, LV_EVENT_CLICKED, NULL);
    
    // Style the save button
    lv_obj_set_style_bg_color(save_btn, lv_color_hex(0x4CAF50), LV_PART_MAIN);  // Green
    lv_obj_set_style_bg_color(save_btn, lv_color_hex(0x45a049), LV_STATE_PRESSED);
    lv_obj_set_style_radius(save_btn, 6, LV_PART_MAIN);
    
    // Create save button label
    lv_obj_t *save_btn_label = lv_label_create(save_btn);
    lv_label_set_text(save_btn_label, "Save");
    lv_obj_set_style_text_color(save_btn_label, lv_color_white(), LV_PART_MAIN);
    lv_obj_center(save_btn_label);
    
    // Create Cancel button
    cancel_btn = lv_btn_create(time_settings_screen);
    lv_obj_set_size(cancel_btn, 100, 40);
    lv_obj_align(cancel_btn, LV_ALIGN_TOP_MID, 60, 460);  // Moved down for much taller calendar
    lv_obj_add_event_cb(cancel_btn, cancel_btn_event_cb, LV_EVENT_CLICKED, NULL);
    
    // Style the cancel button
    lv_obj_set_style_bg_color(cancel_btn, lv_color_hex(0xF44336), LV_PART_MAIN);  // Red
    lv_obj_set_style_bg_color(cancel_btn, lv_color_hex(0xD32F2F), LV_STATE_PRESSED);
    lv_obj_set_style_radius(cancel_btn, 6, LV_PART_MAIN);
    
    // Create cancel button label
    lv_obj_t *cancel_btn_label = lv_label_create(cancel_btn);
    lv_label_set_text(cancel_btn_label, "Cancel");
    lv_obj_set_style_text_color(cancel_btn_label, lv_color_white(), LV_PART_MAIN);
    lv_obj_center(cancel_btn_label);
    
    LOG_UI_INFO("Time settings screen created with calendar (built-in navigation) and hour/minute rollers");
}

lv_obj_t* time_settings_screen_get_screen(void) {
    return time_settings_screen;
}

void time_settings_screen_load_current_time(void) {
    struct tm current_time;
    bsp_pcf85063_get_time(&current_time);
    
    // Set calendar date using individual parameters
    lv_calendar_set_today_date(calendar, current_time.tm_year + 1900, current_time.tm_mon + 1, current_time.tm_mday);
    lv_calendar_set_showed_date(calendar, current_time.tm_year + 1900, current_time.tm_mon + 1);
    
    // Initialize selected date to current date
    selected_date.year = current_time.tm_year + 1900;
    selected_date.month = current_time.tm_mon + 1;
    selected_date.day = current_time.tm_mday;
    date_selected = true;  // Start with current date selected
    
    // Highlight the current date as initially selected
    lv_calendar_date_t highlighted_dates[1];
    highlighted_dates[0] = selected_date;
    lv_calendar_set_highlighted_dates(calendar, highlighted_dates, 1);
    
    // Set rollers to current time values (seconds will always be set to 0)
    lv_roller_set_selected(hour_roller, current_time.tm_hour, LV_ANIM_OFF);
    lv_roller_set_selected(minute_roller, current_time.tm_min, LV_ANIM_OFF);
    
    LOG_RTC_INFO("Loaded current time into settings: %04d-%02d-%02d %02d:%02d (seconds will be set to 00)",
                current_time.tm_year + 1900, current_time.tm_mon + 1, current_time.tm_mday,
                current_time.tm_hour, current_time.tm_min);
}

void time_settings_screen_save_and_return(void) {
    struct tm new_time = {0};
    
    // Use the tracked selected date
    lv_calendar_date_t date;
    if (date_selected) {
        date = selected_date;
        LOG_UI_INFO("Using tracked selected date: %d-%02d-%02d", date.year, date.month, date.day);
    } else {
        // Fallback to today's date if somehow no date was selected
        const lv_calendar_date_t * today_date = lv_calendar_get_today_date(calendar);
        if (today_date != NULL) {
            date = *today_date;
            LOG_UI_INFO("Using today's date as fallback: %d-%02d-%02d", date.year, date.month, date.day);
        } else {
            // Final fallback
            date.year = 2025;
            date.month = 1;
            date.day = 1;
            LOG_UI_ERROR("Using hardcoded fallback date: %d-%02d-%02d", date.year, date.month, date.day);
        }
    }
    
    // Get values from calendar and rollers
    new_time.tm_year = date.year - 1900;  // tm_year is years since 1900
    new_time.tm_mon = date.month - 1;     // tm_mon is 0-11
    new_time.tm_mday = date.day;
    
    // Get hour and minute from rollers
    uint16_t hour_selection = lv_roller_get_selected(hour_roller);
    uint16_t minute_selection = lv_roller_get_selected(minute_roller);
    
    LOG_UI_INFO("Roller debug - hour_selection: %d, minute_selection: %d", hour_selection, minute_selection);
    
    new_time.tm_hour = hour_selection;
    new_time.tm_min = minute_selection;
    new_time.tm_sec = 0;  // Always set seconds to 0
    
    // Set the RTC time
    bsp_pcf85063_set_time(&new_time);
    
    LOG_RTC_INFO("Time set to: %04d-%02d-%02d %02d:%02d:00 (roller selections: h=%d, m=%d)",
                new_time.tm_year + 1900, new_time.tm_mon + 1, new_time.tm_mday,
                new_time.tm_hour, new_time.tm_min, 
                hour_selection, minute_selection);
    
    // Return to main screen
    screen_manager_switch_to(SCREEN_MAIN);
    LOG_UI_INFO("Time saved, returning to main screen");
}
