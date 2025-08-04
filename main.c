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
#include "drivers/stepper/stepper_driver.h"
#include "drivers/stepper/stepper_mcp23017.h"
#include "drivers/gpio_abstraction/gpio_abstraction.h"
#include "drivers/mcp23017/mcp23017_class.h"

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
    
    // Set CPU clock to 220MHz with proper peripheral clock setup
    set_cpu_clock(CPU_FREQ_KHZ);
    bsp_i2c_init();
    lv_port_init();
    bsp_lcd_brightness_init();
    bsp_lcd_brightness_set(80);
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
    
    // Initialize GPIO abstraction layer
    printf("Initializing GPIO abstraction layer...\n");
    gpio_abstraction_init();
    
    // Initialize stepper motor driver with MCP23017 enable pin (object-oriented approach)
    // Address 0x27, Pin 0 (A0) for stepper enable control
    printf("Setting up stepper motor with MCP23017 enable control...\n");
    if (!stepper_init_with_mcp23017_enable(0x27, 0)) {
        printf("Warning: MCP23017 stepper enable failed, falling back to basic stepper init\n");
        stepper_driver_init();  // Fallback to basic initialization
    } else {
        // Demonstrate object-oriented pin access
        mcp23017_class_t *mcp_device = stepper_get_mcp23017_device();
        gpio_pin_t *enable_pin = stepper_get_enable_pin();
        
        if (mcp_device && enable_pin) {
            printf("Successfully created MCP23017 device and pin objects!\n");
            printf("- MCP23017 device at address 0x%02X\n", 0x27);
            printf("- Enable pin object ready for generic use\n");
            
            // Example: You could get additional pins from the same device
            gpio_pin_t *status_pin = mcp23017_class_get_pin_a(mcp_device, 1);  // Pin A1 for status LED
            if (status_pin) {
                printf("- Additional pin objects available (e.g., status LED on A1)\n");
                // Initialize as output for potential status indication
                if (gpio_pin_init(status_pin, true)) {
                    printf("- Status pin initialized as output\n");
                }
            }
        }
    }
    
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
