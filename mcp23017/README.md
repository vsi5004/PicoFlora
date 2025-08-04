# MCP23017 I2C I/O Expander Driver

This driver provides control over the MCP23017 16-pin I2C I/O expander for use in the PicoFlora plant watering system.

## Hardware Setup

### MCP23017 Connections
```
MCP23017    →    Pico 2
VDD         →    3.3V
VSS         →    GND
SCL         →    GPIO 1 (I2C0 SCL)
SDA         →    GPIO 0 (I2C0 SDA)
RESET       →    3.3V (or GPIO for reset control)
A0, A1, A2  →    GND (for address 0x20)
```

### I2C Address Configuration
The MCP23017 I2C address is set by pins A0, A1, A2:
- A2=0, A1=0, A0=0: Address 0x20 (default)
- A2=0, A1=0, A0=1: Address 0x21
- A2=0, A1=1, A0=0: Address 0x22
- ...and so on up to 0x27

## Pin Assignments for PicoFlora

### Port A (Pins 0-7)
- **A0**: Stepper Driver Enable (active low)
- **A1**: Water Pump Control
- **A2**: Water Level Sensor Input (with pullup)
- **A3**: Moisture Sensor Power Control
- **A4-A7**: Available for expansion

### Port B (Pins 8-15)  
- **B0-B3**: Plant Zone Selectors (4 zones)
- **B4-B7**: Available for expansion

## Usage Examples

### Basic Initialization
```c
#include "mcp23017.h"

mcp23017_device_t io_expander;

// Initialize device
if (mcp23017_init(&io_expander, MCP23017_DEFAULT_ADDR)) {
    printf("MCP23017 initialized\n");
}

// Configure a pin as output
mcp23017_set_pin_direction(&io_expander, MCP23017_PIN_A0, MCP23017_OUTPUT);

// Write to a pin
mcp23017_write_pin(&io_expander, MCP23017_PIN_A0, MCP23017_HIGH);
```

### Stepper Integration
```c
#include "stepper_io_integration.h"

// Initialize everything
stepper_io_init();

// Start stepper with automatic enable
stepper_start_with_enable(1600); // 1 revolution

// Stop stepper with automatic disable (power saving)
stepper_stop_with_disable();
```

### Plant Watering Control
```c
// Select plant zone
select_plant_zone(0); // Zone 0

// Check water level
if (check_water_level()) {
    // Turn on water pump
    water_pump_on();
    sleep_ms(3000); // Water for 3 seconds
    water_pump_off();
}
```

## API Reference

### Core Functions
- `mcp23017_init()` - Initialize device
- `mcp23017_set_pin_direction()` - Configure pin as input/output
- `mcp23017_write_pin()` - Write to output pin
- `mcp23017_read_pin()` - Read from input pin
- `mcp23017_set_pin_pullup()` - Enable/disable pullup resistor

### Enhanced Stepper Functions
- `stepper_io_init()` - Initialize MCP23017 for stepper control
- `stepper_start_with_enable()` - Start stepper with automatic enable
- `stepper_stop_with_disable()` - Stop stepper with power saving

### Plant Watering Functions
- `water_pump_on()` / `water_pump_off()` - Control water pump
- `check_water_level()` - Read water level sensor
- `select_plant_zone()` - Select watering zone

## Benefits

### Power Management
- Automatic stepper driver enable/disable saves power and reduces heat
- Only enable the driver when actually moving

### Expandability  
- 16 GPIO pins available for sensors, actuators, and indicators
- Support for up to 8 MCP23017 devices (128 total pins)

### Reliability
- I2C communication with error checking
- Safe default pin configurations
- Individual pin control with port-wide operations available

## Troubleshooting

### Communication Issues
1. Check I2C wiring (SDA, SCL, power, ground)
2. Verify I2C address matches hardware configuration
3. Check pullup resistors on I2C lines (usually 4.7kΩ)
4. Ensure MCP23017 is powered (3.3V)

### Pin Issues
1. Verify pin direction is set correctly (input vs output)
2. Check if pullups are needed for input pins
3. Ensure output pins aren't driving into short circuits

## Future Enhancements

Possible additions for the plant watering system:
- Interrupt support for sensor monitoring
- Multiple MCP23017 device management
- Pin state caching for performance
- Configuration save/restore from flash
