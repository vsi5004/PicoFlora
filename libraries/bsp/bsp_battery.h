#ifndef __BSP_BATTERY_H__
#define __BSP_BATTERY_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"


#define BSP_BAT_ADC_PIN     27
#define BSP_BAT_EN_PIN      26
#define BSP_BAT_KEY_PIN     25

void bsp_battery_init(uint16_t key_wait_ms);
void bsp_battery_read(float *voltage, uint16_t *adc_raw);
bool bsp_battery_get_key_level(void);

#endif // __BSP_BATTERY_H__

