#ifndef __STEPPER_IO_INTEGRATION_H__
#define __STEPPER_IO_INTEGRATION_H__

#include "mcp23017.h"
#include <stdint.h>
#include <stdbool.h>

/**
 * Enhanced stepper control with MCP23017 I/O expander
 * 
 * Provides automatic enable/disable control for stepper driver
 */

// Function prototypes

/**
 * Initialize MCP23017 for stepper control
 * @return true if successful, false otherwise
 */
bool stepper_io_init(void);

/**
 * Start stepper with automatic driver enable
 * @param target_steps Number of steps to move
 * @return true if successful, false otherwise
 */
bool stepper_start_with_enable(int32_t target_steps);

/**
 * Stop stepper with automatic driver disable
 */
void stepper_stop_with_disable(void);

/**
 * Get direct access to the I/O expander device
 * @return Pointer to MCP23017 device structure
 */
mcp23017_device_t* get_io_expander(void);

#endif // __STEPPER_IO_INTEGRATION_H__
