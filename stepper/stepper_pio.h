#ifndef __STEPPER_PIO_H__
#define __STEPPER_PIO_H__

#include "hardware/pio.h"

// Generated PIO program (will be created by pico_generate_pio_header)
extern const struct pio_program stepper_program;

// PIO initialization function
void stepper_pio_init(PIO pio, uint sm, uint offset, uint pin);

#endif // __STEPPER_PIO_H__
