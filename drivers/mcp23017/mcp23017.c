/**
 * MCP23017 I2C I/O Expander Driver Implementation
 * 
 * Provides control over 16 GPIO pins via I2C interface
 */

#include "mcp23017.h"
#include "../logging/logging.h"
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include <stdio.h>

// MCP23017 Register addresses (IOCON.BANK = 0)
#define MCP23017_REG_IODIRA   0x00  // I/O Direction Register Port A
#define MCP23017_REG_IODIRB   0x01  // I/O Direction Register Port B
#define MCP23017_REG_IPOLA    0x02  // Input Polarity Register Port A
#define MCP23017_REG_IPOLB    0x03  // Input Polarity Register Port B
#define MCP23017_REG_GPINTENA 0x04  // Interrupt-on-change Enable Register Port A
#define MCP23017_REG_GPINTENB 0x05  // Interrupt-on-change Enable Register Port B
#define MCP23017_REG_DEFVALA  0x06  // Default Value Register Port A
#define MCP23017_REG_DEFVALB  0x07  // Default Value Register Port B
#define MCP23017_REG_INTCONA  0x08  // Interrupt-on-change Control Register Port A
#define MCP23017_REG_INTCONB  0x09  // Interrupt-on-change Control Register Port B
#define MCP23017_REG_IOCONA   0x0A  // I/O Expander Configuration Register
#define MCP23017_REG_IOCONB   0x0B  // I/O Expander Configuration Register
#define MCP23017_REG_GPPUA    0x0C  // GPIO Pull-up Resistor Register Port A
#define MCP23017_REG_GPPUB    0x0D  // GPIO Pull-up Resistor Register Port B
#define MCP23017_REG_INTFA    0x0E  // Interrupt Flag Register Port A
#define MCP23017_REG_INTFB    0x0F  // Interrupt Flag Register Port B
#define MCP23017_REG_INTCAPA  0x10  // Interrupt Captured Value Register Port A
#define MCP23017_REG_INTCAPB  0x11  // Interrupt Captured Value Register Port B
#define MCP23017_REG_GPIOA    0x12  // General Purpose I/O Port Register Port A
#define MCP23017_REG_GPIOB    0x13  // General Purpose I/O Port Register Port B
#define MCP23017_REG_OLATA    0x14  // Output Latch Register Port A
#define MCP23017_REG_OLATB    0x15  // Output Latch Register Port B

// Configuration constants
#define MCP23017_I2C_INSTANCE i2c1
#define MCP23017_I2C_TIMEOUT_US 10000

// Private functions
static bool mcp23017_write_register(mcp23017_device_t* device, uint8_t reg, uint8_t value) {
    uint8_t data[2] = {reg, value};
    int result = i2c_write_timeout_us(MCP23017_I2C_INSTANCE, device->i2c_addr, data, 2, false, MCP23017_I2C_TIMEOUT_US);
    
    if (result < 0) {
        LOG_HARDWARE_ERROR("MCP23017: Failed to write register 0x%02X to device 0x%02X", reg, device->i2c_addr);
        return false;
    }
    return true;
}

static bool mcp23017_read_register(mcp23017_device_t* device, uint8_t reg, uint8_t* value) {
    if (!value) return false;
    
    // Write register address
    int result = i2c_write_timeout_us(MCP23017_I2C_INSTANCE, device->i2c_addr, &reg, 1, true, MCP23017_I2C_TIMEOUT_US);
    if (result < 0) {
        LOG_HARDWARE_ERROR("MCP23017: Failed to write register address 0x%02X to device 0x%02X", reg, device->i2c_addr);
        return false;
    }
    
    // Read the value
    result = i2c_read_timeout_us(MCP23017_I2C_INSTANCE, device->i2c_addr, value, 1, false, MCP23017_I2C_TIMEOUT_US);
    if (result < 0) {
        LOG_HARDWARE_ERROR("MCP23017: Failed to read register 0x%02X from device 0x%02X", reg, device->i2c_addr);
        return false;
    }
    
    return true;
}

bool mcp23017_init(mcp23017_device_t* device, uint8_t i2c_addr) {
    if (!device) return false;
    
    LOG_HARDWARE_INFO("Initializing MCP23017 at I2C address 0x%02X", i2c_addr);
    
    // Validate I2C address range
    if (i2c_addr < 0x20 || i2c_addr > 0x27) {
        LOG_HARDWARE_ERROR("MCP23017: Invalid I2C address 0x%02X (must be 0x20-0x27)", i2c_addr);
        return false;
    }
    
    device->i2c_addr = i2c_addr;
    
    // Configure IOCON register (both IOCONA and IOCONB should be the same)
    // BANK=0, MIRROR=0, SEQOP=0, DISSLW=0, HAEN=0, ODR=0, INTPOL=0
    uint8_t iocon_config = 0x00;
    
    if (!mcp23017_write_register(device, MCP23017_REG_IOCONA, iocon_config)) {
        LOG_HARDWARE_ERROR("MCP23017: Failed to configure IOCON register");
        return false;
    }
    
    // Set all pins as inputs initially (0xFF = all inputs)
    if (!mcp23017_write_register(device, MCP23017_REG_IODIRA, 0xFF) ||
        !mcp23017_write_register(device, MCP23017_REG_IODIRB, 0xFF)) {
        LOG_HARDWARE_ERROR("MCP23017: Failed to configure direction registers");
        return false;
    }
    
    // Clear output latches
    if (!mcp23017_write_register(device, MCP23017_REG_OLATA, 0x00) ||
        !mcp23017_write_register(device, MCP23017_REG_OLATB, 0x00)) {
        LOG_HARDWARE_ERROR("MCP23017: Failed to clear output latches");
        return false;
    }
    
    LOG_HARDWARE_INFO("MCP23017 initialized successfully");
    return true;
}

bool mcp23017_set_pin_direction(mcp23017_device_t* device, mcp23017_pin_t pin, mcp23017_direction_t direction) {
    if (!device || pin > MCP23017_PIN_B7) return false;
    
    uint8_t reg = (pin < 8) ? MCP23017_REG_IODIRA : MCP23017_REG_IODIRB;
    uint8_t bit_pos = pin % 8;
    
    // Read current direction register
    uint8_t current_value;
    if (!mcp23017_read_register(device, reg, &current_value)) {
        return false;
    }
    
    // Modify the bit
    if (direction == MCP23017_INPUT) {
        current_value |= (1 << bit_pos);   // Set bit for input
    } else {
        current_value &= ~(1 << bit_pos);  // Clear bit for output
    }
    
    return mcp23017_write_register(device, reg, current_value);
}

bool mcp23017_write_pin(mcp23017_device_t* device, mcp23017_pin_t pin, mcp23017_state_t state) {
    if (!device || pin > MCP23017_PIN_B7) return false;
    
    uint8_t reg = (pin < 8) ? MCP23017_REG_OLATA : MCP23017_REG_OLATB;
    uint8_t bit_pos = pin % 8;
    
    // Read current output latch
    uint8_t current_value;
    if (!mcp23017_read_register(device, reg, &current_value)) {
        return false;
    }
    
    // Modify the bit
    if (state == MCP23017_HIGH) {
        current_value |= (1 << bit_pos);   // Set bit
    } else {
        current_value &= ~(1 << bit_pos);  // Clear bit
    }
    
    return mcp23017_write_register(device, reg, current_value);
}

bool mcp23017_read_pin(mcp23017_device_t* device, mcp23017_pin_t pin, mcp23017_state_t* state) {
    if (!device || !state || pin > MCP23017_PIN_B7) return false;
    
    uint8_t reg = (pin < 8) ? MCP23017_REG_GPIOA : MCP23017_REG_GPIOB;
    uint8_t bit_pos = pin % 8;
    
    // Read GPIO register
    uint8_t gpio_value;
    if (!mcp23017_read_register(device, reg, &gpio_value)) {
        return false;
    }
    
    // Extract the bit
    *state = (gpio_value & (1 << bit_pos)) ? MCP23017_HIGH : MCP23017_LOW;
    
    return true;
}

bool mcp23017_write_port(mcp23017_device_t* device, mcp23017_port_t port, uint8_t value) {
    if (!device || port > MCP23017_PORT_B) return false;
    
    uint8_t reg = (port == MCP23017_PORT_A) ? MCP23017_REG_OLATA : MCP23017_REG_OLATB;
    
    return mcp23017_write_register(device, reg, value);
}

bool mcp23017_read_port(mcp23017_device_t* device, mcp23017_port_t port, uint8_t* value) {
    if (!device || !value || port > MCP23017_PORT_B) return false;
    
    uint8_t reg = (port == MCP23017_PORT_A) ? MCP23017_REG_GPIOA : MCP23017_REG_GPIOB;
    
    return mcp23017_read_register(device, reg, value);
}

bool mcp23017_set_pin_pullup(mcp23017_device_t* device, mcp23017_pin_t pin, bool enable) {
    if (!device || pin > MCP23017_PIN_B7) return false;
    
    uint8_t reg = (pin < 8) ? MCP23017_REG_GPPUA : MCP23017_REG_GPPUB;
    uint8_t bit_pos = pin % 8;
    
    // Read current pullup register
    uint8_t current_value;
    if (!mcp23017_read_register(device, reg, &current_value)) {
        return false;
    }
    
    // Modify the bit
    if (enable) {
        current_value |= (1 << bit_pos);   // Enable pullup
    } else {
        current_value &= ~(1 << bit_pos);  // Disable pullup
    }
    
    return mcp23017_write_register(device, reg, current_value);
}

bool mcp23017_stepper_enable(mcp23017_device_t* device) {
    // Enable stepper (active low for most drivers)
    return mcp23017_write_pin(device, MCP23017_STEPPER_ENABLE_PIN, MCP23017_LOW);
}

bool mcp23017_stepper_disable(mcp23017_device_t* device) {
    // Disable stepper (active low for most drivers)
    return mcp23017_write_pin(device, MCP23017_STEPPER_ENABLE_PIN, MCP23017_HIGH);
}
