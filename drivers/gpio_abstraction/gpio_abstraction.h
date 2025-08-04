/**
 * GPIO Abstraction Layer
 * 
 * Provides a unified interface for controlling both native RP2350 GPIO pins
 * and MCP23017 I/O expander pins using the same function calls.
 * 
 * This allows transparent use of external I/O expanders as if they were
 * native GPIO pins, making the code more portable and easier to maintain.
 */

#ifndef GPIO_ABSTRACTION_H
#define GPIO_ABSTRACTION_H

#include <stdint.h>
#include <stdbool.h>

// Forward declaration
typedef struct gpio_pin gpio_pin_t;

// GPIO pin function pointers for polymorphism
typedef struct {
    bool (*init)(gpio_pin_t *pin, bool is_output);
    bool (*set_high)(gpio_pin_t *pin);
    bool (*set_low)(gpio_pin_t *pin);
    bool (*read)(gpio_pin_t *pin, bool *value);
    bool (*toggle)(gpio_pin_t *pin);
} gpio_pin_ops_t;

// Generic GPIO pin structure
struct gpio_pin {
    const gpio_pin_ops_t *ops;  // Function pointers for pin operations
    void *context;              // Pin-specific context data
    bool is_initialized;        // Track initialization state
};

// Pin creation functions
gpio_pin_t* gpio_create_native_pin(uint8_t pin_number);
gpio_pin_t* gpio_create_mcp23017_pin(uint8_t device_address, uint8_t pin_number);

// Generic pin operations (work with any pin type)
bool gpio_pin_init(gpio_pin_t *pin, bool is_output);
bool gpio_pin_set_high(gpio_pin_t *pin);
bool gpio_pin_set_low(gpio_pin_t *pin);
bool gpio_pin_read(gpio_pin_t *pin, bool *value);
bool gpio_pin_toggle(gpio_pin_t *pin);

// Convenience functions
bool gpio_pin_write(gpio_pin_t *pin, bool value);
void gpio_pin_destroy(gpio_pin_t *pin);

// System initialization
bool gpio_abstraction_init(void);

#endif // GPIO_ABSTRACTION_H
