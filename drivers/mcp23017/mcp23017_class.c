/**
 * MCP23017 Class Implementation
 * 
 * Object-oriented interface for MCP23017 I/O expander
 */

#include "mcp23017_class.h"
#include "../gpio_abstraction/gpio_abstraction.h"
#include "../logging/logging.h"
#include <stdio.h>
#include <stdlib.h>

mcp23017_class_t* mcp23017_class_create(uint8_t device_address) {
    LOG_HARDWARE_DEBUG("Creating MCP23017 class instance for address 0x%02X", device_address);
    
    // Allocate device structure
    mcp23017_class_t *device = malloc(sizeof(mcp23017_class_t));
    if (!device) {
        LOG_HARDWARE_ERROR("Failed to allocate memory for MCP23017 device");
        return NULL;
    }
    
    device->device_address = device_address;
    device->initialized = false;
    
    // Initialize GPIO abstraction if not already done
    gpio_abstraction_init();
    
    // Create pin objects for all 16 pins
    for (int i = 0; i < MCP23017_PIN_COUNT; i++) {
        device->pins[i] = gpio_create_mcp23017_pin(device_address, i);
        if (!device->pins[i]) {
            LOG_HARDWARE_ERROR("Failed to create pin object for pin %d", i);
            // Clean up previously created pins
            for (int j = 0; j < i; j++) {
                gpio_pin_destroy(device->pins[j]);
            }
            free(device);
            return NULL;
        }
    }
    
    device->initialized = true;
    LOG_HARDWARE_INFO("MCP23017 class instance created successfully with %d pin objects", MCP23017_PIN_COUNT);
    return device;
}

void mcp23017_class_destroy(mcp23017_class_t *device) {
    if (!device) return;
    
    LOG_HARDWARE_DEBUG("Destroying MCP23017 class instance for address 0x%02X", device->device_address);
    
    // Destroy all pin objects
    for (int i = 0; i < MCP23017_PIN_COUNT; i++) {
        if (device->pins[i]) {
            gpio_pin_destroy(device->pins[i]);
        }
    }
    
    free(device);
}

gpio_pin_t* mcp23017_class_get_pin(mcp23017_class_t *device, uint8_t pin_number) {
    if (!device || !device->initialized || pin_number >= MCP23017_PIN_COUNT) {
        return NULL;
    }
    
    return device->pins[pin_number];
}

gpio_pin_t* mcp23017_class_get_pin_a(mcp23017_class_t *device, uint8_t pin_number) {
    if (pin_number > 7) return NULL;
    return mcp23017_class_get_pin(device, pin_number);
}

gpio_pin_t* mcp23017_class_get_pin_b(mcp23017_class_t *device, uint8_t pin_number) {
    if (pin_number > 7) return NULL;
    return mcp23017_class_get_pin(device, pin_number + 8);
}

bool mcp23017_class_init_output_pin(mcp23017_class_t *device, uint8_t pin_number) {
    gpio_pin_t *pin = mcp23017_class_get_pin(device, pin_number);
    if (!pin) return false;
    
    return gpio_pin_init(pin, true);  // true = output
}

bool mcp23017_class_init_input_pin(mcp23017_class_t *device, uint8_t pin_number) {
    gpio_pin_t *pin = mcp23017_class_get_pin(device, pin_number);
    if (!pin) return false;
    
    return gpio_pin_init(pin, false);  // false = input
}

bool mcp23017_class_init_all_outputs(mcp23017_class_t *device) {
    if (!device || !device->initialized) return false;
    
    bool success = true;
    for (int i = 0; i < MCP23017_PIN_COUNT; i++) {
        if (!gpio_pin_init(device->pins[i], true)) {
            success = false;
            LOG_HARDWARE_ERROR("Failed to initialize pin %d as output", i);
        }
    }
    
    return success;
}

bool mcp23017_class_init_all_inputs(mcp23017_class_t *device) {
    if (!device || !device->initialized) return false;
    
    bool success = true;
    for (int i = 0; i < MCP23017_PIN_COUNT; i++) {
        if (!gpio_pin_init(device->pins[i], false)) {
            success = false;
            LOG_HARDWARE_ERROR("Failed to initialize pin %d as input", i);
        }
    }
    
    return success;
}

bool mcp23017_class_set_all_high(mcp23017_class_t *device) {
    if (!device || !device->initialized) return false;
    
    bool success = true;
    for (int i = 0; i < MCP23017_PIN_COUNT; i++) {
        if (!gpio_pin_set_high(device->pins[i])) {
            success = false;
            LOG_HARDWARE_ERROR("Failed to set pin %d high", i);
        }
    }
    
    return success;
}

bool mcp23017_class_set_all_low(mcp23017_class_t *device) {
    if (!device || !device->initialized) return false;
    
    bool success = true;
    for (int i = 0; i < MCP23017_PIN_COUNT; i++) {
        if (!gpio_pin_set_low(device->pins[i])) {
            success = false;
            LOG_HARDWARE_ERROR("Failed to set pin %d low", i);
        }
    }
    
    return success;
}
