/**
 * GPIO Abstraction Layer Implementation
 * 
 * Object-oriented approach using function pointers for polymorphic pin operations
 */

#include "gpio_abstraction.h"
#include "../mcp23017/mcp23017.h"
#include "../logging/logging.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include <stdio.h>
#include <stdlib.h>

// =============================================================================
// Native GPIO Pin Implementation
// =============================================================================

typedef struct {
    uint8_t pin_number;
} native_gpio_context_t;

static bool native_gpio_init(gpio_pin_t *pin, bool is_output) {
    native_gpio_context_t *ctx = (native_gpio_context_t*)pin->context;
    gpio_init(ctx->pin_number);
    gpio_set_dir(ctx->pin_number, is_output ? GPIO_OUT : GPIO_IN);
    return true;
}

static bool native_gpio_set_high(gpio_pin_t *pin) {
    native_gpio_context_t *ctx = (native_gpio_context_t*)pin->context;
    gpio_put(ctx->pin_number, true);
    return true;
}

static bool native_gpio_set_low(gpio_pin_t *pin) {
    native_gpio_context_t *ctx = (native_gpio_context_t*)pin->context;
    gpio_put(ctx->pin_number, false);
    return true;
}

static bool native_gpio_read(gpio_pin_t *pin, bool *value) {
    native_gpio_context_t *ctx = (native_gpio_context_t*)pin->context;
    *value = gpio_get(ctx->pin_number);
    return true;
}

static bool native_gpio_toggle(gpio_pin_t *pin) {
    native_gpio_context_t *ctx = (native_gpio_context_t*)pin->context;
    bool current_value = gpio_get(ctx->pin_number);
    gpio_put(ctx->pin_number, !current_value);
    return true;
}

static const gpio_pin_ops_t native_gpio_ops = {
    .init = native_gpio_init,
    .set_high = native_gpio_set_high,
    .set_low = native_gpio_set_low,
    .read = native_gpio_read,
    .toggle = native_gpio_toggle
};

// =============================================================================
// MCP23017 Pin Implementation
// =============================================================================

typedef struct {
    uint8_t device_address;
    uint8_t pin_number;
    mcp23017_device_t *device;
} mcp23017_gpio_context_t;

// Static storage for MCP23017 devices (supports up to 8 devices)
#define MAX_MCP23017_DEVICES 8
static mcp23017_device_t mcp23017_devices[MAX_MCP23017_DEVICES];
static uint8_t mcp23017_device_count = 0;

static mcp23017_device_t* get_or_create_mcp23017_device(uint8_t address) {
    // First, try to find existing device
    for (int i = 0; i < mcp23017_device_count; i++) {
        if (mcp23017_devices[i].i2c_addr == address) {
            return &mcp23017_devices[i];
        }
    }
    
    // If not found and we have space, create a new one
    if (mcp23017_device_count < MAX_MCP23017_DEVICES) {
        mcp23017_device_t *device = &mcp23017_devices[mcp23017_device_count];
        if (mcp23017_init(device, address)) {
            mcp23017_device_count++;
            return device;
        }
    }
    
    return NULL;
}

static bool mcp23017_gpio_init(gpio_pin_t *pin, bool is_output) {
    mcp23017_gpio_context_t *ctx = (mcp23017_gpio_context_t*)pin->context;
    
    // Get or create device instance
    ctx->device = get_or_create_mcp23017_device(ctx->device_address);
    if (!ctx->device) {
        LOG_HARDWARE_ERROR("GPIO: Failed to initialize MCP23017 device at address 0x%02X", ctx->device_address);
        return false;
    }
    
    // Set pin direction
    mcp23017_direction_t direction = is_output ? MCP23017_OUTPUT : MCP23017_INPUT;
    return mcp23017_set_pin_direction(ctx->device, ctx->pin_number, direction);
}

static bool mcp23017_gpio_set_high(gpio_pin_t *pin) {
    mcp23017_gpio_context_t *ctx = (mcp23017_gpio_context_t*)pin->context;
    if (!ctx->device) return false;
    return mcp23017_write_pin(ctx->device, ctx->pin_number, MCP23017_HIGH);
}

static bool mcp23017_gpio_set_low(gpio_pin_t *pin) {
    mcp23017_gpio_context_t *ctx = (mcp23017_gpio_context_t*)pin->context;
    if (!ctx->device) return false;
    return mcp23017_write_pin(ctx->device, ctx->pin_number, MCP23017_LOW);
}

static bool mcp23017_gpio_read(gpio_pin_t *pin, bool *value) {
    mcp23017_gpio_context_t *ctx = (mcp23017_gpio_context_t*)pin->context;
    if (!ctx->device) return false;
    
    mcp23017_state_t state;
    if (!mcp23017_read_pin(ctx->device, ctx->pin_number, &state)) {
        return false;
    }
    *value = (state == MCP23017_HIGH);
    return true;
}

static bool mcp23017_gpio_toggle(gpio_pin_t *pin) {
    bool current_value;
    if (!mcp23017_gpio_read(pin, &current_value)) {
        return false;
    }
    return current_value ? mcp23017_gpio_set_low(pin) : mcp23017_gpio_set_high(pin);
}

static const gpio_pin_ops_t mcp23017_gpio_ops = {
    .init = mcp23017_gpio_init,
    .set_high = mcp23017_gpio_set_high,
    .set_low = mcp23017_gpio_set_low,
    .read = mcp23017_gpio_read,
    .toggle = mcp23017_gpio_toggle
};

// =============================================================================
// Public API Implementation
// =============================================================================

static bool abstraction_initialized = false;

bool gpio_abstraction_init(void) {
    if (abstraction_initialized) {
        return true;
    }
    
    // Initialize stdio for debugging
    stdio_init_all();
    
    // Clear device array
    mcp23017_device_count = 0;
    
    abstraction_initialized = true;
    LOG_HARDWARE_INFO("GPIO Abstraction: Initialized successfully");
    return true;
}

gpio_pin_t* gpio_create_native_pin(uint8_t pin_number) {
    gpio_pin_t *pin = malloc(sizeof(gpio_pin_t));
    if (!pin) return NULL;
    
    native_gpio_context_t *ctx = malloc(sizeof(native_gpio_context_t));
    if (!ctx) {
        free(pin);
        return NULL;
    }
    
    ctx->pin_number = pin_number;
    
    pin->ops = &native_gpio_ops;
    pin->context = ctx;
    pin->is_initialized = false;
    
    return pin;
}

gpio_pin_t* gpio_create_mcp23017_pin(uint8_t device_address, uint8_t pin_number) {
    gpio_pin_t *pin = malloc(sizeof(gpio_pin_t));
    if (!pin) return NULL;
    
    mcp23017_gpio_context_t *ctx = malloc(sizeof(mcp23017_gpio_context_t));
    if (!ctx) {
        free(pin);
        return NULL;
    }
    
    ctx->device_address = device_address;
    ctx->pin_number = pin_number;
    ctx->device = NULL;  // Will be initialized on first use
    
    pin->ops = &mcp23017_gpio_ops;
    pin->context = ctx;
    pin->is_initialized = false;
    
    return pin;
}

bool gpio_pin_init(gpio_pin_t *pin, bool is_output) {
    if (!pin || !pin->ops || !pin->ops->init) return false;
    bool result = pin->ops->init(pin, is_output);
    if (result) {
        pin->is_initialized = true;
    }
    return result;
}

bool gpio_pin_set_high(gpio_pin_t *pin) {
    if (!pin || !pin->ops || !pin->ops->set_high || !pin->is_initialized) return false;
    return pin->ops->set_high(pin);
}

bool gpio_pin_set_low(gpio_pin_t *pin) {
    if (!pin || !pin->ops || !pin->ops->set_low || !pin->is_initialized) return false;
    return pin->ops->set_low(pin);
}

bool gpio_pin_read(gpio_pin_t *pin, bool *value) {
    if (!pin || !pin->ops || !pin->ops->read || !pin->is_initialized || !value) return false;
    return pin->ops->read(pin, value);
}

bool gpio_pin_toggle(gpio_pin_t *pin) {
    if (!pin || !pin->ops || !pin->ops->toggle || !pin->is_initialized) return false;
    return pin->ops->toggle(pin);
}

bool gpio_pin_write(gpio_pin_t *pin, bool value) {
    return value ? gpio_pin_set_high(pin) : gpio_pin_set_low(pin);
}

void gpio_pin_destroy(gpio_pin_t *pin) {
    if (pin) {
        if (pin->context) {
            free(pin->context);
        }
        free(pin);
    }
}
