/**
 * PicoFlora - Smart Plant Watering Station
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include "hardware/pio.h"
#include "hardware/i2c.h"
#include "lvgl.h"
#include "config.h"
#include "bsp_i2c.h"
#include "bsp_battery.h"
#include "bsp_lcd_brightness.h"
#include "bsp_pcf85063.h"
#include "lvgl/lv_port/lv_port.h"
#include "lvgl_screen/lock_screen.h"
#include "lvgl_screen/main_screen.h"
#include "lvgl_screen/stepper_screen.h"
#include "lvgl_screen/time_settings_screen.h"
#include "lvgl_screen/screen_manager.h"
#include "drivers/stepper/stepper_driver.h"
#include "drivers/stepper/stepper_mcp23017.h"
#include "drivers/gpio_abstraction/gpio_abstraction.h"
#include "drivers/mcp23017/mcp23017_class.h"
#include "drivers/logging/logging.h"

// Forward declarations
void set_cpu_clock(uint32_t freq_khz);

// Initialization functions
static void initialize_system(void);
static void initialize_hardware_peripherals(void);
static void initialize_rtc_with_defaults(void);
static void initialize_stepper_motor(void);
static void initialize_user_interface(void);
static void run_main_application_loop(void);

// CPU frequency management callback
static bool cpu_reduced = false;

void cpu_frequency_change_callback(uint32_t freq_khz) {
    set_cpu_clock(freq_khz);
    cpu_reduced = (freq_khz < CONFIG_CPU_FREQ_THRESHOLD);
    LOG_POWER_DEBUG("CPU frequency changed to %lu kHz", freq_khz);
}

void set_cpu_clock(uint32_t freq_khz)
{
    set_sys_clock_khz(freq_khz, true);
    clock_configure(
        clk_peri,
        0,
        CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS,
        freq_khz * 1000,
        freq_khz * 1000);
}

int main() {
    // Initialize stdio first
    stdio_init_all();
    
    // Initialize logging system early
    log_init();
    
    LOG_SYS_INFO("PicoFlora - Smart Plant Watering Station Starting...");
    LOG_SYS_INFO("Version: %s", CONFIG_VERSION_STRING);
    
    // Set CPU clock to high performance mode
    set_cpu_clock(CONFIG_CPU_FREQ_HIGH_KHZ);
    LOG_POWER_INFO("CPU clock set to %d kHz", CONFIG_CPU_FREQ_HIGH_KHZ);
    bsp_i2c_init();
    lv_port_init();
    bsp_lcd_brightness_init();
    bsp_lcd_brightness_set(CONFIG_LCD_DEFAULT_BRIGHTNESS);
    bsp_pcf85063_init();
    LOG_SYS_INFO("Hardware peripherals initialized");
    
    // Check if RTC has valid time, set default if needed
    struct tm now_tm;
    bsp_pcf85063_get_time(&now_tm);
    if (now_tm.tm_year < CONFIG_RTC_MIN_YEAR_OFFSET || now_tm.tm_year > CONFIG_RTC_MAX_YEAR_OFFSET) {
        LOG_RTC_INFO("RTC time invalid, setting default time");
        // Set default time from configuration
        now_tm.tm_year = CONFIG_RTC_DEFAULT_YEAR - 1900; // Year since 1900
        now_tm.tm_mon = CONFIG_RTC_DEFAULT_MONTH - 1;    // Month (0-11)
        now_tm.tm_mday = CONFIG_RTC_DEFAULT_DAY;         // Day of month
        now_tm.tm_hour = CONFIG_RTC_DEFAULT_HOUR;        // Hour (24-hour format)
        now_tm.tm_min = CONFIG_RTC_DEFAULT_MINUTE;       // Minute
        now_tm.tm_sec = CONFIG_RTC_DEFAULT_SECOND;       // Second
        now_tm.tm_isdst = -1;                            // Auto-detect DST
        bsp_pcf85063_set_time(&now_tm);
        LOG_RTC_INFO("Default time set: %04d-%02d-%02d %02d:%02d:%02d", 
                    CONFIG_RTC_DEFAULT_YEAR, CONFIG_RTC_DEFAULT_MONTH, CONFIG_RTC_DEFAULT_DAY,
                    CONFIG_RTC_DEFAULT_HOUR, CONFIG_RTC_DEFAULT_MINUTE, CONFIG_RTC_DEFAULT_SECOND);
    } else {
        LOG_RTC_INFO("RTC time valid: %04d-%02d-%02d %02d:%02d:%02d", 
                    now_tm.tm_year + 1900, now_tm.tm_mon + 1, now_tm.tm_mday,
                    now_tm.tm_hour, now_tm.tm_min, now_tm.tm_sec);
    }
    
    // Initialize GPIO abstraction layer
    LOG_HW_INFO("Initializing GPIO abstraction layer...");
    gpio_abstraction_init();
    
    // Initialize stepper motor driver with MCP23017 enable pin (object-oriented approach)
    LOG_STEPPER_INFO("Setting up stepper motor with MCP23017 enable control...");
    if (!stepper_init_with_mcp23017_enable(CONFIG_MCP23017_ADDRESS, CONFIG_MCP23017_ENABLE_PIN)) {
        LOG_STEPPER_ERROR("MCP23017 stepper enable failed, falling back to basic stepper init");
        stepper_driver_init();  // Fallback to basic initialization
    } else {
        // Demonstrate object-oriented pin access
        mcp23017_class_t *mcp_device = stepper_get_mcp23017_device();
        gpio_pin_t *enable_pin = stepper_get_enable_pin();
        
        if (mcp_device && enable_pin) {
            LOG_STEPPER_INFO("Successfully created MCP23017 device and pin objects!");
            LOG_HW_DEBUG("MCP23017 device at address 0x%02X", CONFIG_MCP23017_ADDRESS);
            LOG_STEPPER_DEBUG("Enable pin object ready for generic use");
            
            // Example: You could get additional pins from the same device
            gpio_pin_t *status_pin = mcp23017_class_get_pin_a(mcp_device, CONFIG_MCP23017_STATUS_PIN);
            if (status_pin) {
                LOG_HW_DEBUG("Additional pin objects available (e.g., status LED on A%d)", CONFIG_MCP23017_STATUS_PIN);
                // Initialize as output for potential status indication
                if (gpio_pin_init(status_pin, true)) {
                    LOG_HW_DEBUG("Status pin initialized as output");
                }
            }
        }
    }
    
    // Initialize screen manager
    screen_manager_init();
    screen_manager_set_cpu_callback(cpu_frequency_change_callback);
    
    // Create the UI screens
    lock_screen_create();
    screen_manager_add_screen(SCREEN_LOCK, lock_screen_get_screen());
    
    main_screen_create();
    screen_manager_add_screen(SCREEN_MAIN, main_screen_get_screen());
    
    stepper_screen_create();
    screen_manager_add_screen(SCREEN_STEPPER, stepper_screen_get_screen());
    
    time_settings_screen_create();
    screen_manager_add_screen(SCREEN_TIME_SETTINGS, time_settings_screen_get_screen());
    
    // Start with lock screen
    screen_manager_switch_to(SCREEN_LOCK);
    LOG_UI_INFO("User interface initialized with lock screen");
    
    LOG_SYS_INFO("PicoFlora Started Successfully");
    
    // Main loop
    while (true) {
        // Handle LVGL tasks
        lv_timer_handler();
        
        // Update screen manager (handles timeout)
        screen_manager_update();
        
        // Check if we should reduce CPU frequency for power savings
        if (screen_manager_should_reduce_cpu() && !cpu_reduced) {
            cpu_frequency_change_callback(CONFIG_CPU_FREQ_LOW_KHZ);
        }
        
        // Update stepper motor state
        stepper_driver_update();
        
        // Update UI with stepper progress (only when on stepper screen)
        if (screen_manager_get_current() == SCREEN_STEPPER) {
            stepper_screen_update_progress();
        }
        
        // Update lock screen time display (only when on lock screen and after fade completes)
        if (screen_manager_lock_screen_ready_for_updates()) {
            lock_screen_update_time();
        }
        
        sleep_ms(CONFIG_MAIN_LOOP_DELAY_MS);
    }
    
    return 0;
}
