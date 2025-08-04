/**
 * PicoFlora - Smart Plant Watering Station
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include "hardware/pio.h"
#include "hardware/i2c.h"
#include "lvgl.h"
#include "bsp_i2c.h"
#include "bsp_battery.h"
#include "bsp_lcd_brightness.h"
#include "bsp_pcf85063.h"
#include "lvgl/lv_port/lv_port.h"
#include "lvgl_screen/stepper_screen.h"
#include "lvgl_screen/lock_screen.h"
#include "lvgl_screen/screen_manager.h"
#include "stepper/stepper_driver.h"
#include "mcp23017/stepper_io_integration.h"

// Main configuration
#define CPU_FREQ_KHZ 220000  // 220MHz

// Forward declarations
void set_cpu_clock(uint32_t freq_khz);

// CPU frequency management callback
static bool cpu_reduced = false;

void cpu_frequency_change_callback(uint32_t freq_khz) {
    set_cpu_clock(freq_khz);
    cpu_reduced = (freq_khz < 100000);  // Consider anything below 100MHz as "reduced"
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
    
    printf("PicoFlora - Smart Plant Watering Station Starting...\n");
    
    // Initialize battery system first (required for display)
    bsp_battery_init(100);
    
    // Set CPU clock to 220MHz with proper peripheral clock setup
    set_cpu_clock(CPU_FREQ_KHZ);
    
    // Initialize I2C (required for touch controller)
    bsp_i2c_init();
    
    // Initialize LVGL port (display, touch, etc.)
    lv_port_init();
    
    // Initialize LCD brightness system
    bsp_lcd_brightness_init();
    bsp_lcd_brightness_set(80);
    
    // Initialize PCF85063 RTC
    bsp_pcf85063_init();
    
    // Check if RTC has valid time, set default if needed
    struct tm now_tm;
    bsp_pcf85063_get_time(&now_tm);
    if (now_tm.tm_year < 124 || now_tm.tm_year > 130) {
        // Set default time (August 4, 2025, 12:00 PM)
        now_tm.tm_year = 2025 - 1900; // Year since 1900
        now_tm.tm_mon = 8 - 1;        // Month (0-11, so August = 7)
        now_tm.tm_mday = 4;           // Day of month
        now_tm.tm_hour = 12;          // Hour (24-hour format)
        now_tm.tm_min = 0;            // Minute
        now_tm.tm_sec = 0;            // Second
        now_tm.tm_isdst = -1;         // Auto-detect DST
        bsp_pcf85063_set_time(&now_tm);
    }
    
    // Initialize stepper motor driver
    stepper_driver_init();
    
    // Initialize MCP23017 I/O expander for stepper control
    stepper_io_init();
    
    // Initialize screen manager
    screen_manager_init();
    screen_manager_set_cpu_callback(cpu_frequency_change_callback);
    
    // Create the UI screens
    stepper_screen_create();
    screen_manager_add_screen(SCREEN_STEPPER, stepper_screen_get_screen());
    
    lock_screen_create();
    screen_manager_add_screen(SCREEN_LOCK, lock_screen_get_screen());
    
    // Start with lock screen
    screen_manager_switch_to(SCREEN_LOCK);
    
    printf("PicoFlora Started Successfully\n");
    
    // Main loop
    while (true) {
        // Handle LVGL tasks
        lv_timer_handler();
        
        // Update screen manager (handles timeout)
        screen_manager_update();
        
        // Check if we should reduce CPU frequency for power savings
        if (screen_manager_should_reduce_cpu() && !cpu_reduced) {
            cpu_frequency_change_callback(48000);  // 48MHz
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
        
        sleep_ms(5);
    }
    
    return 0;
}
