#ifndef __BSP_ST7789_H__
#define __BSP_ST7789_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

#include "bsp_dma_channel_irq.h"

#define BSP_ST7789_SPI_NUM      spi1    

#define BSP_ST7789_MOSI_PIN     11
#define BSP_ST7789_MISO_PIN     -1
#define BSP_ST7789_SCLK_PIN     10

#define BSP_ST7789_DC_PIN       14
#define BSP_ST7789_CS_PIN       13
#define BSP_ST7789_RST_PIN      15

#define BSP_ST7789_ROTATION     0

#define BSP_ST7789_OFFSET_X     0
#define BSP_ST7789_OFFSET_Y     0

// #define ROTATION                0
// #define ROTATION                0


typedef struct
{
    uint16_t width;
    uint16_t height;
    uint16_t rotation;
    uint16_t x_offset;
    uint16_t y_offset;
    uint dma_tx_channel;
    bool enabled_dma;
    channel_irq_callback_t dma_flush_done_callback;
}bsp_st7789_info_t;


bsp_st7789_info_t *bsp_st7789_get_info(void);
void bsp_st7789_flush_dma(uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end, uint16_t *color);
void bsp_st7789_init(bsp_st7789_info_t *st7789_info);
void bsp_st7789_set_window(uint16_t Xstart, uint16_t Ystart, uint16_t Xend, uint16_t Yend);
void bsp_st7789_flush(uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end, uint16_t *color);
void bsp_st7789_clear(uint16_t Xstart, uint16_t Ystart, uint16_t Xend, uint16_t Yend, uint16_t color);
#endif // __BSP_ST7789_H__