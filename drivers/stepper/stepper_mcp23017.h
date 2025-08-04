/**
 * Stepper Driver with MCP23017 Integration
 * 
 * Simple interface for using MCP23017 pins with the stepper driver
 * This provides clean abstraction using pin objects and MCP23017 class
 */

#ifndef STEPPER_MCP23017_H
#define STEPPER_MCP23017_H

#include "../gpio_abstraction/gpio_abstraction.h"
#include "../mcp23017/mcp23017_class.h"
#include "stepper_driver.h"

// Convenience function to initialize stepper with MCP23017 enable pin using class system
bool stepper_init_with_mcp23017_enable(uint8_t mcp23017_address, uint8_t enable_pin_number);

// Convenience function to initialize stepper with native GPIO enable pin
bool stepper_init_with_gpio_enable(uint8_t gpio_pin_number);

// Get the enable pin object for direct access (if needed)
gpio_pin_t* stepper_get_enable_pin(void);

// Get the MCP23017 device instance (if using MCP23017)
mcp23017_class_t* stepper_get_mcp23017_device(void);

#endif // STEPPER_MCP23017_H
