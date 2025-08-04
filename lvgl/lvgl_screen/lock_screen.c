/**
 * Lock Screen Implementation
 * 
 * Creates a black screen with white time/date display in center
 * Uses PCF85063 RTC for real time display
 */

#include "lock_screen.h"
#include "bsp_pcf85063.h"
#include "../../drivers/logging/logging.h"
#include <stdio.h>
#include <string.h>

// Screen objects
static lv_obj_t *lock_screen = NULL;
static lv_obj_t *time_label = NULL;
static lv_obj_t *date_label = NULL;
static lv_obj_t *day_label = NULL;

// Cache last displayed values to avoid unnecessary updates
static char last_time_str[16] = {0};
static char last_date_str[32] = {0};
static char last_day_str[16] = {0};

void lock_screen_create(void) {
    // Create the lock screen
    lock_screen = lv_obj_create(NULL);
    
    // Set black background
    lv_obj_set_style_bg_color(lock_screen, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(lock_screen, LV_OPA_COVER, 0);
    
    // Create day of week label
    day_label = lv_label_create(lock_screen);
    
    // Set white text color for day
    lv_obj_set_style_text_color(day_label, lv_color_white(), 0);
    
    // Set font size for day of week
    lv_obj_set_style_text_font(day_label, &lv_font_montserrat_14, 0);
    
    // Position day label above time
    lv_obj_align(day_label, LV_ALIGN_CENTER, 0, -40);
    
    // Create time label
    time_label = lv_label_create(lock_screen);
    
    // Set white text color
    lv_obj_set_style_text_color(time_label, lv_color_white(), 0);
    
    // Set larger font size for better visibility (using available font)
    lv_obj_set_style_text_font(time_label, &lv_font_montserrat_16, 0);
    
    // Position time label above center
    lv_obj_align(time_label, LV_ALIGN_CENTER, 0, -20);
    
    // Create date label
    date_label = lv_label_create(lock_screen);
    
    // Set white text color for date
    lv_obj_set_style_text_color(date_label, lv_color_white(), 0);
    
    // Set font size for date (slightly smaller than time)
    lv_obj_set_style_text_font(date_label, &lv_font_montserrat_14, 0);
    
    // Position date label below the time
    lv_obj_align(date_label, LV_ALIGN_CENTER, 0, 40);
    
    // Initialize with actual RTC time instead of hardcoded values
    lock_screen_update_time();
    
    LOG_UI_INFO("Lock screen created successfully");
}

lv_obj_t* lock_screen_get_screen(void) {
    return lock_screen;
}

void lock_screen_update_time(void) {
    // Get real time from PCF85063 RTC
    if (time_label && date_label && day_label) {
        struct tm now_tm;
        bsp_pcf85063_get_time(&now_tm);
        
        // Format day of week
        const char* days[] = {
            "Sunday", "Monday", "Tuesday", "Wednesday", 
            "Thursday", "Friday", "Saturday"
        };
        const char* current_day = days[now_tm.tm_wday];
        
        // Update day label only if changed
        if (strcmp(last_day_str, current_day) != 0) {
            lv_label_set_text(day_label, current_day);
            strcpy(last_day_str, current_day);
        }
        
        // Format time as HH:MM AM/PM
        int hour_12 = now_tm.tm_hour;
        const char* am_pm = "AM";
        
        if (hour_12 == 0) {
            hour_12 = 12; // Midnight is 12 AM
        } else if (hour_12 > 12) {
            hour_12 -= 12; // Convert to 12-hour format
            am_pm = "PM";
        } else if (hour_12 == 12) {
            am_pm = "PM"; // Noon is 12 PM
        }
        
        // Format time string
        char current_time_str[16];
        snprintf(current_time_str, sizeof(current_time_str), "%02d:%02d %s", hour_12, now_tm.tm_min, am_pm);
        
        // Update time label only if changed
        if (strcmp(last_time_str, current_time_str) != 0) {
            lv_label_set_text(time_label, current_time_str);
            strcpy(last_time_str, current_time_str);
        }
        
        // Format date string
        const char* months[] = {
            "January", "February", "March", "April", "May", "June",
            "July", "August", "September", "October", "November", "December"
        };
        
        char current_date_str[32];
        snprintf(current_date_str, sizeof(current_date_str), "%s %d, %d", 
                 months[now_tm.tm_mon], now_tm.tm_mday, now_tm.tm_year + 1900);
        
        // Update date label only if changed
        if (strcmp(last_date_str, current_date_str) != 0) {
            lv_label_set_text(date_label, current_date_str);
            strcpy(last_date_str, current_date_str);
        }
    }
}
