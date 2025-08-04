#ifndef __BSP_I2S_H__
#define __BSP_I2S_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"

#define BSP_I2S_BCK_PIN     2
#define BSP_I2S_LRCK_PIN    (BSP_I2S_BCK_PIN + 1)
#define BSP_I2S_DIN_PIN     4


// I2S
#define BSP_I2S_SOUND_PIO           pio1    // 使用的pio模块


#define BSP_I2S_FREQ                16000   // 声道频率


// void bsp_i2c_write(uint8_t device_addr, uint8_t *buffer, size_t len);
// void bsp_i2c_write_reg8(uint8_t device_addr, uint8_t reg_addr, uint8_t *buffer, size_t len);
// void bsp_i2c_read_reg8(uint8_t device_addr, uint8_t reg_addr, uint8_t *buffer, size_t len);

void bsp_i2s_init(void);
void bsp_i2s_output(const uint32_t *sound, size_t len, bool wait);
void i2sSoundOutputDmaBlocking(const uint32_t *sound, size_t len);
#endif

