/**
 * MCP23017 Integration Example
 * 
 * Shows how to use the MCP23017 I/O expander with the stepper driver
 * for controlling the stepper enable pin and other GPIO functions
 */

#include "mcp23017.h"
#include "../stepper/stepper_driver.h"
#include "pico/stdlib.h"
#include <stdio.h>

// Example device instance
static mcp23017_device_t io_expander;
static bool mcp23017_available = false;

/**
 * Initialize the MCP23017 for stepper control
 * This sets up the I/O expander with common configurations for stepper control
 */
bool stepper_io_init(void) {
    printf("MCP23017: Attempting to initialize at address 0x27\n");
    
    // Initialize the MCP23017 at address 0x27
    if (!mcp23017_init(&io_expander, 0x27)) {
        printf("MCP23017: Failed to initialize - check I2C connections and address\n");
        return false;
    }
    printf("MCP23017: Device initialized successfully\n");
    
    // Configure stepper enable pin as output (active low)
    printf("MCP23017: Configuring pin A0 as output for stepper enable\n");
    if (!mcp23017_set_pin_direction(&io_expander, MCP23017_STEPPER_ENABLE_PIN, MCP23017_OUTPUT)) {
        printf("MCP23017: Failed to configure stepper enable pin direction\n");
        return false;
    }
    printf("MCP23017: Pin A0 configured as output\n");
    
    // Initially disable the stepper (high = disabled for most drivers)
    printf("MCP23017: Setting initial stepper disable state\n");
    if (!mcp23017_stepper_disable(&io_expander)) {
        printf("MCP23017: Failed to set initial stepper disable state\n");
        return false;
    }
    printf("MCP23017: Stepper initially disabled\n");
    
    printf("MCP23017: Stepper I/O initialized successfully\n");
    return true;
}

/**
 * Enhanced stepper start function with automatic enable control
 */
bool stepper_start_with_enable(int32_t target_steps) {
    printf("MCP23017: Attempting to enable stepper driver\n");
    
    // Enable the stepper driver first
    if (!mcp23017_stepper_enable(&io_expander)) {
        printf("MCP23017: Failed to enable stepper driver - I2C communication error\n");
        return false;
    }
    printf("MCP23017: Stepper driver enabled successfully\n");
    
    // Small delay to let the driver stabilize
    sleep_ms(10);
    
    // Start the stepper movement
    printf("Starting stepper movement: %ld steps\n", target_steps);
    stepper_driver_start(target_steps);
    
    printf("Stepper started with %ld steps (driver enabled)\n", target_steps);
    return true;
}

/**
 * Enhanced stepper stop function with automatic disable control
 */
void stepper_stop_with_disable(void) {
    // Stop the stepper movement
    stepper_driver_stop();
    
    // Small delay to let movement stop cleanly
    sleep_ms(50);
    
    // Disable the stepper driver to save power and reduce heat
    mcp23017_stepper_disable(&io_expander);
    
    printf("Stepper stopped and driver disabled\n");
}

/**
 * Get the MCP23017 device pointer for direct access
 */
mcp23017_device_t* get_io_expander(void) {
    return &io_expander;
}
