/**
 * PIO-based Stepper Motor Driver Implementation
 * 
 * Uses PIO to generate precise step pulses with smooth acceleration/deceleration
 * Hardware-timed pulses eliminate software timing jitter
 */

#include "stepper_driver.h"
#include "../gpio_abstraction/gpio_abstraction.h"
#include "../logging/logging.h"
#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include "hardware/pio.h"
#include "stepper.pio.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

// PIO configuration
#define STEPPER_PIO pio1
#define STEPPER_PIO_SM 0

// Driver state
static struct {
    stepper_state_t state;
    int32_t current_steps;
    int32_t target_steps;
    uint32_t current_frequency;
    uint32_t step_counter;
    bool direction;  // true = forward, false = reverse
    absolute_time_t last_update_time;
    absolute_time_t step_start_time;  // Time when current step sequence started
    uint pio_offset;
    bool pio_initialized;
    bool pio_running;  // Track if PIO is currently running
    bool const_phase_printed;  // Flag to avoid repeated CONST messages
    uint32_t steps_at_last_update;  // For accurate step counting
    gpio_pin_t *enable_pin;  // Optional enable pin (can be NULL)
    bool enable_pin_active;  // Track if enable pin is currently active
} stepper_state = {
    .state = STEPPER_IDLE,
    .current_steps = 0,
    .target_steps = 0,
    .current_frequency = 0,
    .step_counter = 0,
    .direction = true,
    .last_update_time = 0,
    .step_start_time = 0,
    .pio_offset = 0,
    .pio_initialized = false,
    .pio_running = false,
    .const_phase_printed = false,
    .steps_at_last_update = 0,
    .enable_pin = NULL,
    .enable_pin_active = false
};

// Helper function to calculate frequency based on current position
static uint32_t calculate_adaptive_accel_steps(void) {
    // Adaptive acceleration steps: configurable percentage of total movement
    int32_t adaptive_accel_steps = stepper_state.target_steps / STEPPER_ACCEL_DIVISOR;
    if (adaptive_accel_steps < STEPPER_MIN_ACCEL_STEPS) adaptive_accel_steps = STEPPER_MIN_ACCEL_STEPS;
    if (adaptive_accel_steps > STEPPER_MAX_ACCEL_STEPS) adaptive_accel_steps = STEPPER_MAX_ACCEL_STEPS;
    return adaptive_accel_steps;
}

static uint32_t calculate_target_frequency(void) {
    int32_t remaining_steps = stepper_state.target_steps - stepper_state.current_steps;
    
    if (remaining_steps <= 0) {
        return 0;  // Stop
    }
    
    // Get adaptive acceleration steps
    int32_t adaptive_accel_steps = calculate_adaptive_accel_steps();
    
    // Acceleration phase
    if (stepper_state.current_steps < adaptive_accel_steps) {
        // Linear acceleration using integer math for consistency
        uint32_t freq_range = STEPPER_MAX_FREQ_HZ - STEPPER_MIN_FREQ_HZ;
        uint32_t accel_increment = (freq_range * stepper_state.current_steps) / adaptive_accel_steps;
        uint32_t target_freq = STEPPER_MIN_FREQ_HZ + accel_increment;
        return target_freq;
    }
    
    // Deceleration phase
    if (remaining_steps < adaptive_accel_steps) {
        // Linear deceleration using integer math for consistency
        uint32_t freq_range = STEPPER_MAX_FREQ_HZ - STEPPER_MIN_FREQ_HZ;
        uint32_t decel_increment = (freq_range * remaining_steps) / adaptive_accel_steps;
        uint32_t target_freq = STEPPER_MIN_FREQ_HZ + decel_increment;
        return target_freq;
    }
    
    // Constant speed phase - only log once when entering this phase
    if (!stepper_state.const_phase_printed) {
        LOG_STEPPER_DEBUG("Entering constant speed phase: freq=%lu Hz, accel_steps=%ld", 
                         STEPPER_MAX_FREQ_HZ, adaptive_accel_steps);
        stepper_state.const_phase_printed = true;
    }
    return STEPPER_MAX_FREQ_HZ;
}

// Update step frequency using PIO
static void update_step_frequency(uint32_t frequency_hz) {
    if (!stepper_state.pio_initialized) {
        LOG_STEPPER_ERROR("PIO not initialized!");
        return;
    }
    
    stepper_state.current_frequency = frequency_hz;
    
    if (frequency_hz == 0) {
        // Stop the PIO program
        stepper_step_stop(STEPPER_PIO, STEPPER_PIO_SM);
        stepper_state.pio_running = false;
    } else {
        // Set the new frequency
        stepper_step_set_frequency(STEPPER_PIO, STEPPER_PIO_SM, frequency_hz);
        
        // Only start PIO if it's not already running
        if (!stepper_state.pio_running) {
            stepper_step_start(STEPPER_PIO, STEPPER_PIO_SM);
            stepper_state.pio_running = true;
            LOG_STEPPER_DEBUG("Stepper frequency set to %lu Hz", frequency_hz);
        }
        // Don't spam frequency changes - they happen very frequently now
    }
}

// Helper functions for enable pin control
static void stepper_enable_driver(void) {
    if (stepper_state.enable_pin && !stepper_state.enable_pin_active) {
        gpio_pin_set_low(stepper_state.enable_pin);  // Most stepper drivers are active low
        stepper_state.enable_pin_active = true;
        LOG_STEPPER_DEBUG("Stepper driver enabled");
        sleep_ms(10);  // Allow driver to stabilize
    }
}

static void stepper_disable_driver(void) {
    if (stepper_state.enable_pin && stepper_state.enable_pin_active) {
        gpio_pin_set_high(stepper_state.enable_pin);  // Disable (high for most drivers)
        stepper_state.enable_pin_active = false;
        LOG_STEPPER_DEBUG("Stepper driver disabled");
    }
}

void stepper_driver_init(void) {
    stepper_driver_init_with_enable_pin(NULL);
}

void stepper_driver_init_with_enable_pin(gpio_pin_t *enable_pin) {
    LOG_STEPPER_INFO("Initializing PIO-based stepper driver on GPIO %d", STEPPER_STEP_PIN);
    
    // Store enable pin reference
    stepper_state.enable_pin = enable_pin;
    stepper_state.enable_pin_active = false;
    
    // Configure enable pin if provided
    if (enable_pin) {
        LOG_STEPPER_DEBUG("Configuring enable pin");
        if (!gpio_pin_init(enable_pin, true)) {  // true = output
            LOG_STEPPER_ERROR("Failed to configure enable pin as output");
        } else {
            // Start with driver disabled (high for most drivers)
            gpio_pin_set_high(enable_pin);
            LOG_STEPPER_DEBUG("Enable pin configured and driver initially disabled");
        }
    } else {
        LOG_STEPPER_DEBUG("No enable pin specified - driver always enabled");
    }
    
    // Check if PIO program can be added
    if (!pio_can_add_program(STEPPER_PIO, &stepper_step_program)) {
        LOG_STEPPER_ERROR("Cannot add PIO program - insufficient space!");
        stepper_state.pio_initialized = false;
        return;
    }
    
    // Initialize PIO program
    stepper_state.pio_offset = pio_add_program(STEPPER_PIO, &stepper_step_program);
    LOG_STEPPER_DEBUG("PIO program loaded at offset %d", stepper_state.pio_offset);
    
    if (stepper_state.pio_offset < 0) {
        LOG_STEPPER_ERROR("Failed to load PIO program!");
        stepper_state.pio_initialized = false;
        return;
    }
    
    stepper_step_program_init(STEPPER_PIO, STEPPER_PIO_SM, stepper_state.pio_offset, STEPPER_STEP_PIN);
    LOG_STEPPER_DEBUG("PIO state machine initialized");
    
    // Initialize state
    stepper_state.state = STEPPER_IDLE;
    stepper_state.current_steps = 0;
    stepper_state.target_steps = 0;
    stepper_state.current_frequency = 0;
    stepper_state.step_counter = 0;
    stepper_state.last_update_time = get_absolute_time();
    stepper_state.step_start_time = stepper_state.last_update_time;
    stepper_state.steps_at_last_update = 0;
    stepper_state.pio_initialized = true;
    stepper_state.pio_running = false;
    
    LOG_STEPPER_INFO("PIO stepper driver initialized successfully");
    LOG_STEPPER_INFO("Frequency range: %d Hz to %d Hz", STEPPER_MIN_FREQ_HZ, STEPPER_MAX_FREQ_HZ);
}

void stepper_driver_start(int32_t target_steps) {
    if (target_steps <= 0) {
        LOG_STEPPER_ERROR("Invalid target steps: %ld", target_steps);
        return;
    }
    
    LOG_STEPPER_INFO("Starting stepper motor: %ld steps", target_steps);
    
    // Enable the stepper driver
    stepper_enable_driver();
    
    // Reset state
    stepper_state.current_steps = 0;
    stepper_state.target_steps = target_steps;
    stepper_state.state = STEPPER_ACCELERATING;
    stepper_state.step_counter = 0;
    stepper_state.last_update_time = get_absolute_time();
    stepper_state.step_start_time = stepper_state.last_update_time;  // Track movement start time
    stepper_state.steps_at_last_update = 0;
    stepper_state.const_phase_printed = false;  // Reset for new movement
    
    // Start with minimum frequency
    update_step_frequency(STEPPER_MIN_FREQ_HZ);
}

void stepper_driver_stop(void) {
    LOG_STEPPER_INFO("Stopping PIO stepper motor");
    
    // Stop PIO pulses
    update_step_frequency(0);
    
    // Update state
    stepper_state.state = STEPPER_IDLE;
    stepper_state.current_frequency = 0;
    
    // Disable the stepper driver to save power
    stepper_disable_driver();
}

void stepper_driver_update(void) {
    if (stepper_state.state == STEPPER_IDLE || stepper_state.state == STEPPER_COMPLETED) {
        return;
    }
    
    absolute_time_t current_time = get_absolute_time();
    uint64_t update_time_diff = absolute_time_diff_us(stepper_state.last_update_time, current_time);
    
    // Update step count based on time since last update and current frequency
    if (stepper_state.current_frequency > 0 && update_time_diff >= 2000) { // Update every 2ms for balanced smoothness and visibility
        uint32_t steps_since_last = (stepper_state.current_frequency * update_time_diff) / 1000000;
        stepper_state.current_steps += steps_since_last;
        stepper_state.last_update_time = current_time;
        
        // Check if movement is complete
        if (stepper_state.current_steps >= stepper_state.target_steps) {
            stepper_state.current_steps = stepper_state.target_steps;
            stepper_driver_stop();
            stepper_state.state = STEPPER_COMPLETED;
            LOG_STEPPER_INFO("Stepper movement completed: %ld steps", stepper_state.current_steps);
            return;
        }
        
        // Calculate new target frequency based on current position - do this frequently for smooth acceleration!
        uint32_t target_freq = calculate_target_frequency();
        
        // Update state based on position in movement using adaptive acceleration
        int32_t remaining_steps = stepper_state.target_steps - stepper_state.current_steps;
        int32_t adaptive_accel_steps = calculate_adaptive_accel_steps();
        
        if (stepper_state.current_steps < adaptive_accel_steps) {
            stepper_state.state = STEPPER_ACCELERATING;
        } else if (remaining_steps < adaptive_accel_steps) {
            stepper_state.state = STEPPER_DECELERATING;
        } else {
            stepper_state.state = STEPPER_RUNNING;
        }
        
        // Apply new frequency for smoother acceleration - update even for small changes
        if (target_freq != stepper_state.current_frequency) {
            update_step_frequency(target_freq);
            // Print every update to see what's happening
            LOG_STEPPER_DEBUG("Steps: %ld/%ld, State: %d, Freq: %lu Hz", 
                   stepper_state.current_steps, stepper_state.target_steps,
                   stepper_state.state, target_freq);
        }
    }
}

// Status functions
bool stepper_driver_is_running(void) {
    return (stepper_state.state != STEPPER_IDLE && stepper_state.state != STEPPER_COMPLETED);
}

int32_t stepper_driver_get_current_steps(void) {
    return stepper_state.current_steps;
}

int32_t stepper_driver_get_target_steps(void) {
    return stepper_state.target_steps;
}

stepper_state_t stepper_driver_get_state(void) {
    return stepper_state.state;
}

uint32_t stepper_driver_get_current_frequency(void) {
    return stepper_state.current_frequency;
}

// Convenience functions for revolution-based movement
void stepper_driver_start_revolutions(float revolutions) {
    int32_t steps = (int32_t)(revolutions * STEPPER_STEPS_PER_REV);
    stepper_driver_start(steps);
}

float stepper_driver_get_current_revolutions(void) {
    return stepper_state.current_steps / (float)STEPPER_STEPS_PER_REV;
}

float stepper_driver_get_target_revolutions(void) {
    return stepper_state.target_steps / (float)STEPPER_STEPS_PER_REV;
}
