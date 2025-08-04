/**
 * Stepper Driver with MCP23017 Integration Implementation
 * 
 * Simple interface using pin objects and MCP23017 class for clean abstraction
 */

#include "stepper_mcp23017.h"
#include <stdio.h>

// Static storage for the enable pin objects and MCP23017 device
static gpio_pin_t *stepper_enable_pin = NULL;
static mcp23017_class_t *mcp23017_device = NULL;

bool stepper_init_with_mcp23017_enable(uint8_t mcp23017_address, uint8_t enable_pin_number) {
    printf("Initializing stepper with MCP23017 enable pin (address: 0x%02X, pin: %d)\n", 
           mcp23017_address, enable_pin_number);
    
    // Initialize GPIO abstraction layer
    if (!gpio_abstraction_init()) {
        printf("Failed to initialize GPIO abstraction layer\n");
        return false;
    }
    
    // Create MCP23017 device instance
    mcp23017_device = mcp23017_class_create(mcp23017_address);
    if (!mcp23017_device) {
        printf("Failed to create MCP23017 device instance\n");
        return false;
    }
    
    // Get enable pin object from the device
    stepper_enable_pin = mcp23017_class_get_pin(mcp23017_device, enable_pin_number);
    if (!stepper_enable_pin) {
        printf("Failed to get pin object from MCP23017 device\n");
        mcp23017_class_destroy(mcp23017_device);
        mcp23017_device = NULL;
        return false;
    }
    
    // Initialize stepper driver with the enable pin
    stepper_driver_init_with_enable_pin(stepper_enable_pin);
    
    printf("Stepper initialized with MCP23017 enable pin successfully\n");
    return true;
}

bool stepper_init_with_gpio_enable(uint8_t gpio_pin_number) {
    printf("Initializing stepper with native GPIO enable pin (%d)\n", gpio_pin_number);
    
    // Initialize GPIO abstraction layer
    if (!gpio_abstraction_init()) {
        printf("Failed to initialize GPIO abstraction layer\n");
        return false;
    }
    
    // Create native GPIO pin object
    stepper_enable_pin = gpio_create_native_pin(gpio_pin_number);
    if (!stepper_enable_pin) {
        printf("Failed to create native GPIO pin object\n");
        return false;
    }
    
    // Initialize stepper driver with the enable pin
    stepper_driver_init_with_enable_pin(stepper_enable_pin);
    
    printf("Stepper initialized with native GPIO enable pin successfully\n");
    return true;
}

gpio_pin_t* stepper_get_enable_pin(void) {
    return stepper_enable_pin;
}

mcp23017_class_t* stepper_get_mcp23017_device(void) {
    return mcp23017_device;
}
