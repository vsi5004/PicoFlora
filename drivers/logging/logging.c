/**
 * PicoFlora Logging System Implementation
 */

#include "logging.h"
#include "pico/stdlib.h"
#include <string.h>

// Global configuration
log_level_t g_current_log_level = LOG_LEVEL_INFO;  // Default to INFO level
bool g_log_category_enabled[LOG_CAT_COUNT] = {
    [LOG_CAT_SYSTEM] = true,
    [LOG_CAT_HARDWARE] = true,
    [LOG_CAT_STEPPER] = true,
    [LOG_CAT_UI] = true,
    [LOG_CAT_POWER] = false,    // Disabled by default - can be noisy
    [LOG_CAT_RTC] = true
};

// Level strings for output formatting
static const char* level_strings[] = {
    [LOG_LEVEL_DEBUG] = "DEBUG",
    [LOG_LEVEL_INFO]  = "INFO ",
    [LOG_LEVEL_WARN]  = "WARN ",
    [LOG_LEVEL_ERROR] = "ERROR",
    [LOG_LEVEL_FATAL] = "FATAL",
    [LOG_LEVEL_NONE]  = "NONE "
};

// Category strings for output formatting
static const char* category_strings[] = {
    [LOG_CAT_SYSTEM]   = "SYS",
    [LOG_CAT_HARDWARE] = "HW ",
    [LOG_CAT_STEPPER]  = "STEP",
    [LOG_CAT_UI]       = "UI ",
    [LOG_CAT_POWER]    = "PWR",
    [LOG_CAT_RTC]      = "RTC"
};

// ANSI color codes for different log levels (if terminal supports them)
static const char* level_colors[] = {
    [LOG_LEVEL_DEBUG] = "\033[90m",  // Dark gray
    [LOG_LEVEL_INFO]  = "\033[32m",  // Green
    [LOG_LEVEL_WARN]  = "\033[33m",  // Yellow
    [LOG_LEVEL_ERROR] = "\033[31m",  // Red
    [LOG_LEVEL_FATAL] = "\033[91m",  // Bright red
};
static const char* color_reset = "\033[0m";

void log_init(void) {
    // Ensure stdio is initialized for logging output
    stdio_init_all();
    
    LOG_SYS_INFO("Logging system initialized");
    LOG_SYS_DEBUG("Log level: %s", log_level_to_string(g_current_log_level));
    
    // Show enabled categories
    for (int i = 0; i < LOG_CAT_COUNT; i++) {
        if (g_log_category_enabled[i]) {
            LOG_SYS_DEBUG("Category %s: enabled", log_category_to_string((log_category_t)i));
        }
    }
}

void log_set_level(log_level_t level) {
    if (level <= LOG_LEVEL_NONE) {
        g_current_log_level = level;
        LOG_SYS_INFO("Log level changed to: %s", log_level_to_string(level));
    }
}

void log_enable_category(log_category_t category, bool enabled) {
    if (category < LOG_CAT_COUNT) {
        g_log_category_enabled[category] = enabled;
        LOG_SYS_DEBUG("Category %s: %s", 
                     log_category_to_string(category), 
                     enabled ? "enabled" : "disabled");
    }
}

void log_message(log_level_t level, log_category_t category, const char* format, ...) {
    // Filter by log level
    if (level < g_current_log_level || level == LOG_LEVEL_NONE) {
        return;
    }
    
    // Filter by category
    if (category >= LOG_CAT_COUNT || !g_log_category_enabled[category]) {
        return;
    }
    
    // Format the message
    char message_buffer[LOG_MAX_MESSAGE_LENGTH];
    va_list args;
    va_start(args, format);
    vsnprintf(message_buffer, sizeof(message_buffer), format, args);
    va_end(args);
    
    // Build the log line
    char log_line[LOG_MAX_MESSAGE_LENGTH + 100];  // Extra space for prefix
    
    #if LOG_TIMESTAMP_ENABLED
    // Get timestamp in milliseconds since boot
    uint32_t timestamp_ms = to_ms_since_boot(get_absolute_time());
    uint32_t seconds = timestamp_ms / 1000;
    uint32_t milliseconds = timestamp_ms % 1000;
    
    #if LOG_CATEGORY_ENABLED
    snprintf(log_line, sizeof(log_line), 
             "%s[%7lu.%03lu] [%s] [%s] %s%s\n",
             level_colors[level],
             seconds, milliseconds,
             level_strings[level],
             category_strings[category],
             message_buffer,
             color_reset);
    #else
    snprintf(log_line, sizeof(log_line), 
             "%s[%7lu.%03lu] [%s] %s%s\n",
             level_colors[level],
             seconds, milliseconds,
             level_strings[level],
             message_buffer,
             color_reset);
    #endif
    
    #else  // No timestamp
    
    #if LOG_CATEGORY_ENABLED
    snprintf(log_line, sizeof(log_line), 
             "%s[%s] [%s] %s%s\n",
             level_colors[level],
             level_strings[level],
             category_strings[category],
             message_buffer,
             color_reset);
    #else
    snprintf(log_line, sizeof(log_line), 
             "%s[%s] %s%s\n",
             level_colors[level],
             level_strings[level],
             message_buffer,
             color_reset);
    #endif
    
    #endif
    
    // Output the log line
    printf("%s", log_line);
    
    // For fatal errors, we might want to halt the system
    if (level == LOG_LEVEL_FATAL) {
        printf("FATAL ERROR - System halted\n");
        while (1) {
            tight_loop_contents();
        }
    }
}

const char* log_level_to_string(log_level_t level) {
    if (level < LOG_LEVEL_NONE) {
        return level_strings[level];
    }
    return "UNKNOWN";
}

const char* log_category_to_string(log_category_t category) {
    if (category < LOG_CAT_COUNT) {
        return category_strings[category];
    }
    return "UNKNOWN";
}
