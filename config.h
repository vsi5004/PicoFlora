/**
 * PicoFlora Configuration
 * 
 * Centralized configuration for the entire application
 * This replaces scattered magic numbers and hard-coded values
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

// ============================================================================
// System Configuration
// ============================================================================

// CPU Frequencies
#define CONFIG_CPU_FREQ_HIGH_KHZ    220000  // 220MHz for active operation
#define CONFIG_CPU_FREQ_LOW_KHZ     48000   // 48MHz for power saving
#define CONFIG_CPU_FREQ_THRESHOLD   100000  // Threshold for "reduced" CPU

// ============================================================================
// Hardware Pin Assignments
// ============================================================================

// Stepper Motor
#define CONFIG_STEPPER_STEP_PIN     29      // GPIO pin for step signal

// MCP23017 I/O Expander
#define CONFIG_MCP23017_ADDRESS     0x27    // I2C address
#define CONFIG_MCP23017_ENABLE_PIN  0       // Pin A0 for stepper enable
#define CONFIG_MCP23017_STATUS_PIN  1       // Pin A1 for status LED

// ============================================================================
// Stepper Motor Configuration
// ============================================================================

// Frequency limits
#define CONFIG_STEPPER_MIN_FREQ_HZ      2000    // Minimum step frequency
#define CONFIG_STEPPER_MAX_FREQ_HZ      8000    // Maximum step frequency

// Acceleration profile
#define CONFIG_STEPPER_ACCEL_DIVISOR    15      // Acceleration zone divisor
#define CONFIG_STEPPER_MIN_ACCEL_STEPS  500     // Minimum acceleration steps
#define CONFIG_STEPPER_MAX_ACCEL_STEPS  3000    // Maximum acceleration steps

// Microstepping
#define CONFIG_STEPPER_MICROSTEPS       8       // 1/8 microstepping
#define CONFIG_STEPPER_FULL_STEPS_REV   200     // Full steps per revolution
#define CONFIG_STEPPER_STEPS_PER_REV    (CONFIG_STEPPER_FULL_STEPS_REV * CONFIG_STEPPER_MICROSTEPS)

// ============================================================================
// User Interface Configuration
// ============================================================================

// Screen timeout
#define CONFIG_UI_TIMEOUT_MS            30000   // 30 seconds to lock screen
#define CONFIG_UI_FADE_ANIMATION_MS     250     // Fade animation duration

// LCD Settings
#define CONFIG_LCD_DEFAULT_BRIGHTNESS   80      // Default brightness (0-100)

// ============================================================================
// Real-Time Clock Configuration
// ============================================================================

// Default time if RTC is invalid (August 4, 2025, 12:00 PM)
#define CONFIG_RTC_DEFAULT_YEAR         2025
#define CONFIG_RTC_DEFAULT_MONTH        8       // August
#define CONFIG_RTC_DEFAULT_DAY          4
#define CONFIG_RTC_DEFAULT_HOUR         12      // 12:00 PM
#define CONFIG_RTC_DEFAULT_MINUTE       0
#define CONFIG_RTC_DEFAULT_SECOND       0

// Valid year range for RTC validation
#define CONFIG_RTC_MIN_YEAR_OFFSET      124     // 2024 (years since 1900)
#define CONFIG_RTC_MAX_YEAR_OFFSET      130     // 2030 (years since 1900)

// ============================================================================
// Logging Configuration
// ============================================================================

// Default log level (can be changed at runtime)
#define CONFIG_LOG_DEFAULT_LEVEL        LOG_LEVEL_INFO

// Enable/disable categories by default
#define CONFIG_LOG_SYSTEM_ENABLED       true
#define CONFIG_LOG_HARDWARE_ENABLED     true
#define CONFIG_LOG_STEPPER_ENABLED      true
#define CONFIG_LOG_UI_ENABLED           true
#define CONFIG_LOG_POWER_ENABLED        false   // Disabled by default - can be noisy
#define CONFIG_LOG_RTC_ENABLED          true

// ============================================================================
// Timing Configuration
// ============================================================================

// Main loop timing
#define CONFIG_MAIN_LOOP_DELAY_MS       5       // Main loop delay

// Update intervals
#define CONFIG_LVGL_UPDATE_INTERVAL_MS  5       // LVGL timer handler interval

// ============================================================================
// Version Information
// ============================================================================

#define CONFIG_VERSION_MAJOR    1
#define CONFIG_VERSION_MINOR    0
#define CONFIG_VERSION_PATCH    0
#define CONFIG_VERSION_STRING   "1.0.0"

#endif // CONFIG_H
