#ifndef __MCP23017_H__
#define __MCP23017_H__

#include <stdint.h>
#include <stdbool.h>

/**
 * MCP23017 I2C I/O Expander Driver
 * 
 * Features:
 * - 16 GPIO pins (8 on PORTA, 8 on PORTB)
 * - Individual pin direction control
 * - Individual pin pullup control
 * - Interrupt support
 * - Multiple device support via address pins
 */

// Default I2C address (A2=A1=A0=0)
#define MCP23017_DEFAULT_ADDR 0x20

// Maximum number of MCP23017 devices
#define MCP23017_MAX_DEVICES 8

// GPIO pin definitions
typedef enum {
    MCP23017_PIN_A0 = 0,
    MCP23017_PIN_A1 = 1,
    MCP23017_PIN_A2 = 2,
    MCP23017_PIN_A3 = 3,
    MCP23017_PIN_A4 = 4,
    MCP23017_PIN_A5 = 5,
    MCP23017_PIN_A6 = 6,
    MCP23017_PIN_A7 = 7,
    MCP23017_PIN_B0 = 8,
    MCP23017_PIN_B1 = 9,
    MCP23017_PIN_B2 = 10,
    MCP23017_PIN_B3 = 11,
    MCP23017_PIN_B4 = 12,
    MCP23017_PIN_B5 = 13,
    MCP23017_PIN_B6 = 14,
    MCP23017_PIN_B7 = 15
} mcp23017_pin_t;

// Port definitions
typedef enum {
    MCP23017_PORT_A = 0,
    MCP23017_PORT_B = 1
} mcp23017_port_t;

// Pin direction
typedef enum {
    MCP23017_INPUT = 1,
    MCP23017_OUTPUT = 0
} mcp23017_direction_t;

// Pin state
typedef enum {
    MCP23017_LOW = 0,
    MCP23017_HIGH = 1
} mcp23017_state_t;

// Device handle structure
typedef struct {
    uint8_t i2c_addr;
    bool initialized;
} mcp23017_device_t;

// Function prototypes

/**
 * Initialize MCP23017 device
 * @param device Pointer to device structure
 * @param i2c_addr I2C address of the device (0x20-0x27)
 * @return true if successful, false otherwise
 */
bool mcp23017_init(mcp23017_device_t* device, uint8_t i2c_addr);

/**
 * Set pin direction (input or output)
 * @param device Pointer to device structure
 * @param pin Pin number (0-15)
 * @param direction MCP23017_INPUT or MCP23017_OUTPUT
 * @return true if successful, false otherwise
 */
bool mcp23017_set_pin_direction(mcp23017_device_t* device, mcp23017_pin_t pin, mcp23017_direction_t direction);

/**
 * Set port direction (all 8 pins at once)
 * @param device Pointer to device structure
 * @param port MCP23017_PORTA or MCP23017_PORTB
 * @param direction_mask Bit mask for directions (1=input, 0=output)
 * @return true if successful, false otherwise
 */
bool mcp23017_set_port_direction(mcp23017_device_t* device, mcp23017_port_t port, uint8_t direction_mask);

/**
 * Enable/disable pullup resistor on a pin
 * @param device Pointer to device structure
 * @param pin Pin number (0-15)
 * @param enable true to enable pullup, false to disable
 * @return true if successful, false otherwise
 */
bool mcp23017_set_pin_pullup(mcp23017_device_t* device, mcp23017_pin_t pin, bool enable);

/**
 * Write to a single pin
 * @param device Pointer to device structure
 * @param pin Pin number (0-15)
 * @param state MCP23017_HIGH or MCP23017_LOW
 * @return true if successful, false otherwise
 */
bool mcp23017_write_pin(mcp23017_device_t* device, mcp23017_pin_t pin, mcp23017_state_t state);

/**
 * Read from a single pin
 * @param device Pointer to device structure
 * @param pin Pin number (0-15)
 * @param state Pointer to store the pin state
 * @return true if successful, false otherwise
 */
bool mcp23017_read_pin(mcp23017_device_t* device, mcp23017_pin_t pin, mcp23017_state_t* state);

/**
 * Write to an entire port (8 pins at once)
 * @param device Pointer to device structure
 * @param port MCP23017_PORTA or MCP23017_PORTB
 * @param value 8-bit value to write to the port
 * @return true if successful, false otherwise
 */
bool mcp23017_write_port(mcp23017_device_t* device, mcp23017_port_t port, uint8_t value);

/**
 * Read from an entire port (8 pins at once)
 * @param device Pointer to device structure
 * @param port MCP23017_PORTA or MCP23017_PORTB
 * @param value Pointer to store the 8-bit port value
 * @return true if successful, false otherwise
 */
bool mcp23017_read_port(mcp23017_device_t* device, mcp23017_port_t port, uint8_t* value);

/**
 * Read all 16 GPIO pins at once
 * @param device Pointer to device structure
 * @param value Pointer to store the 16-bit value (A=low byte, B=high byte)
 * @return true if successful, false otherwise
 */
bool mcp23017_read_all(mcp23017_device_t* device, uint16_t* value);

/**
 * Write all 16 GPIO pins at once
 * @param device Pointer to device structure
 * @param value 16-bit value to write (A=low byte, B=high byte)
 * @return true if successful, false otherwise
 */
bool mcp23017_write_all(mcp23017_device_t* device, uint16_t value);

// Convenience macros for stepper driver enable control
#define MCP23017_STEPPER_ENABLE_PIN MCP23017_PIN_A0

// Function prototypes for stepper control
bool mcp23017_stepper_enable(mcp23017_device_t* device);
bool mcp23017_stepper_disable(mcp23017_device_t* device);

#endif // __MCP23017_H__
