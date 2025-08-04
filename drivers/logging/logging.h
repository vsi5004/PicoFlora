/**
 * PicoFlora Logging System
 * 
 * Centralized logging with configurable levels and formatted output
 * Replaces scattered printf statements throughout the codebase
 */

#ifndef LOGGING_H
#define LOGGING_H

#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include "pico/time.h"

// Log levels (ordered by severity)
typedef enum {
    LOG_LEVEL_DEBUG = 0,    // Detailed diagnostic information
    LOG_LEVEL_INFO,         // General information messages
    LOG_LEVEL_WARN,         // Warning conditions
    LOG_LEVEL_ERROR,        // Error conditions
    LOG_LEVEL_FATAL,        // Fatal error conditions
    LOG_LEVEL_NONE          // Disable all logging
} log_level_t;

// Log categories for filtering
typedef enum {
    LOG_CAT_SYSTEM = 0,     // System initialization and core functions
    LOG_CAT_HARDWARE,       // Hardware-related messages (I2C, GPIO, etc.)
    LOG_CAT_STEPPER,        // Stepper motor operations
    LOG_CAT_UI,             // User interface and screen management
    LOG_CAT_POWER,          // Power management and CPU frequency
    LOG_CAT_RTC,            // Real-time clock operations
    LOG_CAT_COUNT           // Number of categories (keep last)
} log_category_t;

// Configuration
#define LOG_MAX_MESSAGE_LENGTH 256
#define LOG_TIMESTAMP_ENABLED true
#define LOG_CATEGORY_ENABLED true

// Global log level - messages below this level are filtered out
extern log_level_t g_current_log_level;

// Category enable/disable flags
extern bool g_log_category_enabled[LOG_CAT_COUNT];

// Initialization
void log_init(void);
void log_set_level(log_level_t level);
void log_enable_category(log_category_t category, bool enabled);

// Core logging function
void log_message(log_level_t level, log_category_t category, const char* format, ...);

// Convenience macros for different log levels
#define LOG_DEBUG(category, ...) log_message(LOG_LEVEL_DEBUG, category, __VA_ARGS__)
#define LOG_INFO(category, ...)  log_message(LOG_LEVEL_INFO, category, __VA_ARGS__)
#define LOG_WARN(category, ...)  log_message(LOG_LEVEL_WARN, category, __VA_ARGS__)
#define LOG_ERROR(category, ...) log_message(LOG_LEVEL_ERROR, category, __VA_ARGS__)
#define LOG_FATAL(category, ...) log_message(LOG_LEVEL_FATAL, category, __VA_ARGS__)

// Convenience macros for specific categories
#define LOG_SYS_INFO(...)     LOG_INFO(LOG_CAT_SYSTEM, __VA_ARGS__)
#define LOG_SYS_ERROR(...)    LOG_ERROR(LOG_CAT_SYSTEM, __VA_ARGS__)
#define LOG_SYS_DEBUG(...)    LOG_DEBUG(LOG_CAT_SYSTEM, __VA_ARGS__)

#define LOG_HW_INFO(...)      LOG_INFO(LOG_CAT_HARDWARE, __VA_ARGS__)
#define LOG_HW_ERROR(...)     LOG_ERROR(LOG_CAT_HARDWARE, __VA_ARGS__)
#define LOG_HW_DEBUG(...)     LOG_DEBUG(LOG_CAT_HARDWARE, __VA_ARGS__)

#define LOG_HARDWARE_INFO(...)  LOG_INFO(LOG_CAT_HARDWARE, __VA_ARGS__)
#define LOG_HARDWARE_ERROR(...) LOG_ERROR(LOG_CAT_HARDWARE, __VA_ARGS__)
#define LOG_HARDWARE_DEBUG(...) LOG_DEBUG(LOG_CAT_HARDWARE, __VA_ARGS__)

#define LOG_STEPPER_INFO(...) LOG_INFO(LOG_CAT_STEPPER, __VA_ARGS__)
#define LOG_STEPPER_WARN(...) LOG_WARN(LOG_CAT_STEPPER, __VA_ARGS__)
#define LOG_STEPPER_ERROR(...) LOG_ERROR(LOG_CAT_STEPPER, __VA_ARGS__)
#define LOG_STEPPER_DEBUG(...) LOG_DEBUG(LOG_CAT_STEPPER, __VA_ARGS__)

#define LOG_UI_INFO(...)      LOG_INFO(LOG_CAT_UI, __VA_ARGS__)
#define LOG_UI_ERROR(...)     LOG_ERROR(LOG_CAT_UI, __VA_ARGS__)
#define LOG_UI_DEBUG(...)     LOG_DEBUG(LOG_CAT_UI, __VA_ARGS__)

#define LOG_POWER_INFO(...)   LOG_INFO(LOG_CAT_POWER, __VA_ARGS__)
#define LOG_POWER_DEBUG(...)  LOG_DEBUG(LOG_CAT_POWER, __VA_ARGS__)

#define LOG_RTC_INFO(...)     LOG_INFO(LOG_CAT_RTC, __VA_ARGS__)
#define LOG_RTC_ERROR(...)    LOG_ERROR(LOG_CAT_RTC, __VA_ARGS__)
#define LOG_RTC_DEBUG(...)    LOG_DEBUG(LOG_CAT_RTC, __VA_ARGS__)

// Utility functions
const char* log_level_to_string(log_level_t level);
const char* log_category_to_string(log_category_t category);

#endif // LOGGING_H
