/**
 * MCP23017 Class - Object-Oriented Pin Management
 * 
 * Provides a convenient class-like interface for managing all pins on an MCP23017
 * Each device instance maintains its own collection of pin objects
 */

#ifndef MCP23017_CLASS_H
#define MCP23017_CLASS_H

#include "../gpio_abstraction/gpio_abstraction.h"
#include <stdint.h>
#include <stdbool.h>

// Number of pins on MCP23017
#define MCP23017_PIN_COUNT 16

// MCP23017 device class
typedef struct {
    uint8_t device_address;
    bool initialized;
    gpio_pin_t *pins[MCP23017_PIN_COUNT];  // Array of pin objects
} mcp23017_class_t;

// Class methods
mcp23017_class_t* mcp23017_class_create(uint8_t device_address);
void mcp23017_class_destroy(mcp23017_class_t *device);

// Pin access methods
gpio_pin_t* mcp23017_class_get_pin(mcp23017_class_t *device, uint8_t pin_number);
gpio_pin_t* mcp23017_class_get_pin_a(mcp23017_class_t *device, uint8_t pin_number); // A0-A7
gpio_pin_t* mcp23017_class_get_pin_b(mcp23017_class_t *device, uint8_t pin_number); // B0-B7

// Convenience methods for common pin configurations
bool mcp23017_class_init_output_pin(mcp23017_class_t *device, uint8_t pin_number);
bool mcp23017_class_init_input_pin(mcp23017_class_t *device, uint8_t pin_number);

// Bulk operations
bool mcp23017_class_init_all_outputs(mcp23017_class_t *device);
bool mcp23017_class_init_all_inputs(mcp23017_class_t *device);
bool mcp23017_class_set_all_high(mcp23017_class_t *device);
bool mcp23017_class_set_all_low(mcp23017_class_t *device);

#endif // MCP23017_CLASS_H
