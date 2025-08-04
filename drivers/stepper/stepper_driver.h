#ifndef __STEPPER_DRIVER_H__
#define __STEPPER_DRIVER_H__

#include <stdint.h>
#include <stdbool.h>
#include "../gpio_abstraction/gpio_abstraction.h"

/**
 * PIO-based Stepper Motor Driver
 * 
 * Features:
 * - Smooth acceleration and deceleration profiles
 * - Configurable step rates
 * - Non-blocking operation
 * - Real-time position feedback
 * - Optional enable pin control (native GPIO or MCP23017)
 */

// Configuration
#define STEPPER_STEP_PIN 29          // GPIO pin for step signal
#define STEPPER_MIN_FREQ_HZ 2000     // Minimum step frequency (above PIO limit)
#define STEPPER_MAX_FREQ_HZ 8000     // Maximum step frequency (Hz)

// Adaptive acceleration configuration
#define STEPPER_ACCEL_DIVISOR 15     // Acceleration zone = total_steps / divisor (smaller = longer accel)
#define STEPPER_MIN_ACCEL_STEPS 500   // Minimum acceleration steps for short movements
#define STEPPER_MAX_ACCEL_STEPS 3000  // Maximum acceleration steps for long movements
// Note: Acceleration steps are calculated dynamically based on movement distance

// Microstepping configuration
#define STEPPER_MICROSTEPS 8         // 1/8 microstepping
#define STEPPER_FULL_STEPS_PER_REV 200  // Standard 1.8° stepper motor
#define STEPPER_STEPS_PER_REV (STEPPER_FULL_STEPS_PER_REV * STEPPER_MICROSTEPS)  // 1600 steps/rev

// Stepper states
typedef enum {
    STEPPER_IDLE,
    STEPPER_ACCELERATING,
    STEPPER_RUNNING,
    STEPPER_DECELERATING,
    STEPPER_COMPLETED
} stepper_state_t;

// Function prototypes
void stepper_driver_init(void);
void stepper_driver_init_with_enable_pin(gpio_pin_t *enable_pin);
void stepper_driver_start(int32_t target_steps);
void stepper_driver_stop(void);
void stepper_driver_update(void);

// Status functions
bool stepper_driver_is_running(void);
int32_t stepper_driver_get_current_steps(void);
int32_t stepper_driver_get_target_steps(void);
stepper_state_t stepper_driver_get_state(void);
uint32_t stepper_driver_get_current_frequency(void);

// Convenience functions for revolution-based movement
void stepper_driver_start_revolutions(float revolutions);
float stepper_driver_get_current_revolutions(void);
float stepper_driver_get_target_revolutions(void);

#endif // __STEPPER_DRIVER_H__
