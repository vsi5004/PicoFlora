#include "bsp_st7789.h"
#include "hardware/spi.h"
#include "pico/stdlib.h"
#include "hardware/dma.h"

#include "bsp_dma_channel_irq.h"

bsp_st7789_info_t *g_st7789_info;

void bsp_st7789_spi_write_cmd8(uint8_t cmd)
{
    // sleep_us(100);
    gpio_put(BSP_ST7789_CS_PIN, 0);
    gpio_put(BSP_ST7789_DC_PIN, 0);
    spi_write_blocking(BSP_ST7789_SPI_NUM, &cmd, 1);
    gpio_put(BSP_ST7789_CS_PIN, 1);
}

void bsp_st7789_spi_write_data8(uint8_t data)
{
    sleep_us(100);
    gpio_put(BSP_ST7789_CS_PIN, 0);
    gpio_put(BSP_ST7789_DC_PIN, 1);
    spi_write_blocking(BSP_ST7789_SPI_NUM, &data, 1);
    gpio_put(BSP_ST7789_CS_PIN, 1);
}

void bsp_st7789_spi_write_data(uint8_t *data, size_t len)
{
    // sleep_us(100);
    gpio_put(BSP_ST7789_CS_PIN, 0);
    gpio_put(BSP_ST7789_DC_PIN, 1);
    spi_write_blocking(BSP_ST7789_SPI_NUM, data, len);
    gpio_put(BSP_ST7789_CS_PIN, 1);
}

void bsp_st7789_spi_write_data_dma(uint8_t *data, size_t len)
{
    // sleep_us(100);
    gpio_put(BSP_ST7789_CS_PIN, 0);
    gpio_put(BSP_ST7789_DC_PIN, 1);
    spi_write_blocking(BSP_ST7789_SPI_NUM, data, len);
    gpio_put(BSP_ST7789_CS_PIN, 1);
}

void bsp_st7789_spi_init(void)
{
    spi_init(BSP_ST7789_SPI_NUM, 80 * 1000 * 1000);
    gpio_set_function(BSP_ST7789_MOSI_PIN, GPIO_FUNC_SPI);
    gpio_set_function(BSP_ST7789_SCLK_PIN, GPIO_FUNC_SPI);

#if BSP_ST7789_MISO_PIN != -1
    gpio_set_function(BSP_ST7789_MISO_PIN, GPIO_FUNC_SPI);
#endif
    // 设置spi的模式
    spi_set_format(BSP_ST7789_SPI_NUM, 8, SPI_CPOL_1, SPI_CPHA_1, SPI_MSB_FIRST);
}

static void __dma_flush_done_callback(void)
{
    sleep_us(50);
    gpio_put(BSP_ST7789_CS_PIN, 1);
    if (g_st7789_info->dma_flush_done_callback != NULL)
        g_st7789_info->dma_flush_done_callback();
    else
        printf("dma_flush_done_callback is not set\r\n");
}

void bsp_st7789_spi_dma_init(void)
{
    g_st7789_info->dma_tx_channel = dma_claim_unused_channel(true);
    dma_channel_config c = dma_channel_get_default_config(g_st7789_info->dma_tx_channel);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
    channel_config_set_dreq(&c, spi_get_dreq(BSP_ST7789_SPI_NUM, true));
    dma_channel_configure(g_st7789_info->dma_tx_channel, &c,
                          &spi_get_hw(BSP_ST7789_SPI_NUM)->dr,              // write address
                          NULL,                    // read address
                          g_st7789_info->width * g_st7789_info->height * 2, // element count (each element is of size transfer_data_size)
                          false);                                           // don't start yet

    bsp_dma_channel_irq_add(1, g_st7789_info->dma_tx_channel, __dma_flush_done_callback);
}

void bsp_st7789_gpio_init(void)
{
    gpio_init(BSP_ST7789_DC_PIN);
    gpio_init(BSP_ST7789_CS_PIN);
    gpio_init(BSP_ST7789_RST_PIN);

    gpio_set_dir(BSP_ST7789_DC_PIN, GPIO_OUT);
    gpio_set_dir(BSP_ST7789_CS_PIN, GPIO_OUT);
    gpio_set_dir(BSP_ST7789_RST_PIN, GPIO_OUT);
}

void bsp_st7789_reset(void)
{
    gpio_put(BSP_ST7789_RST_PIN, 0);
    sleep_ms(50);
    gpio_put(BSP_ST7789_RST_PIN, 1);
    sleep_ms(50);
}

void bsp_st7789_reg_init(void)
{
    bsp_st7789_spi_write_cmd8(0x29); // Display on
    sleep_ms(10);
    bsp_st7789_spi_write_cmd8(0x11);
    sleep_ms(10); // ms
    bsp_st7789_spi_write_cmd8(0x36);
    bsp_st7789_spi_write_data8(0x00);

    bsp_st7789_spi_write_cmd8(0x3A);
    bsp_st7789_spi_write_data8(0x05);

    bsp_st7789_spi_write_cmd8(0xB0);
    bsp_st7789_spi_write_data8(0x00);
    bsp_st7789_spi_write_data8(0xE8); // 5 to 6-bit conversion: r0 = r5, b0 = b5

    bsp_st7789_spi_write_cmd8(0xB2);
    bsp_st7789_spi_write_data8(0x0C);
    bsp_st7789_spi_write_data8(0x0C);
    bsp_st7789_spi_write_data8(0x00);
    bsp_st7789_spi_write_data8(0x33);
    bsp_st7789_spi_write_data8(0x33);

    bsp_st7789_spi_write_cmd8(0xB7);
    bsp_st7789_spi_write_data8(0x75); // VGH=14.97V,VGL=-7.67V

    bsp_st7789_spi_write_cmd8(0xBB);
    bsp_st7789_spi_write_data8(0x1A);

    bsp_st7789_spi_write_cmd8(0xC0);
    bsp_st7789_spi_write_data8(0x2C);

    bsp_st7789_spi_write_cmd8(0xC2);
    bsp_st7789_spi_write_data8(0x01);
    bsp_st7789_spi_write_data8(0xFF);

    bsp_st7789_spi_write_cmd8(0xC3);
    bsp_st7789_spi_write_data8(0x13);

    bsp_st7789_spi_write_cmd8(0xC4);
    bsp_st7789_spi_write_data8(0x20);

    bsp_st7789_spi_write_cmd8(0xC6);
    bsp_st7789_spi_write_data8(0x0F);

    bsp_st7789_spi_write_cmd8(0xD0);
    bsp_st7789_spi_write_data8(0xA4);
    bsp_st7789_spi_write_data8(0xA1);

    bsp_st7789_spi_write_cmd8(0xD6);
    bsp_st7789_spi_write_data8(0xA1);

    bsp_st7789_spi_write_cmd8(0xE0);
    bsp_st7789_spi_write_data8(0xD0);
    bsp_st7789_spi_write_data8(0x0D);
    bsp_st7789_spi_write_data8(0x14);
    bsp_st7789_spi_write_data8(0x0D);
    bsp_st7789_spi_write_data8(0x0D);
    bsp_st7789_spi_write_data8(0x09);
    bsp_st7789_spi_write_data8(0x38);
    bsp_st7789_spi_write_data8(0x44);
    bsp_st7789_spi_write_data8(0x4E);
    bsp_st7789_spi_write_data8(0x3A);
    bsp_st7789_spi_write_data8(0x17);
    bsp_st7789_spi_write_data8(0x18);
    bsp_st7789_spi_write_data8(0x2F);
    bsp_st7789_spi_write_data8(0x30);

    bsp_st7789_spi_write_cmd8(0xE1);
    bsp_st7789_spi_write_data8(0xD0);
    bsp_st7789_spi_write_data8(0x09);
    bsp_st7789_spi_write_data8(0x0F);
    bsp_st7789_spi_write_data8(0x08);
    bsp_st7789_spi_write_data8(0x07);
    bsp_st7789_spi_write_data8(0x14);
    bsp_st7789_spi_write_data8(0x37);
    bsp_st7789_spi_write_data8(0x44);
    bsp_st7789_spi_write_data8(0x4D);
    bsp_st7789_spi_write_data8(0x38);
    bsp_st7789_spi_write_data8(0x15);
    bsp_st7789_spi_write_data8(0x16);
    bsp_st7789_spi_write_data8(0x2C);
    bsp_st7789_spi_write_data8(0x2E);

    bsp_st7789_spi_write_cmd8(0x21);

    bsp_st7789_spi_write_cmd8(0x29);

    bsp_st7789_spi_write_cmd8(0x2C);
}

void bsp_st7789_set_window(uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end)
{
    // if (g_st7789_info->rotation == 1 || g_st7789_info->rotation == 3)
    // {

    //     bsp_st7789_spi_write_cmd8(0x2A);
    //     bsp_st7789_spi_write_data8(y_start >> 8);
    //     bsp_st7789_spi_write_data8(y_start + g_st7789_info->y_offset);
    //     bsp_st7789_spi_write_data8(y_end >> 8);
    //     bsp_st7789_spi_write_data8(y_end + g_st7789_info->y_offset);
    //     bsp_st7789_spi_write_cmd8(0x2B);
    //     bsp_st7789_spi_write_data8(x_start >> 8);
    //     bsp_st7789_spi_write_data8(x_start + g_st7789_info->x_offset);
    //     bsp_st7789_spi_write_data8(x_end >> 8);
    //     bsp_st7789_spi_write_data8(x_end + g_st7789_info->x_offset);
    // }
    // else
    {

        bsp_st7789_spi_write_cmd8(0x2A);
        bsp_st7789_spi_write_data8(x_start >> 8);
        bsp_st7789_spi_write_data8(x_start + g_st7789_info->x_offset);
        bsp_st7789_spi_write_data8(x_end >> 8);
        bsp_st7789_spi_write_data8(x_end + g_st7789_info->x_offset);

        // set the Y coordinates
        bsp_st7789_spi_write_cmd8(0x2B);
        bsp_st7789_spi_write_data8(y_start >> 8);
        bsp_st7789_spi_write_data8(y_start + g_st7789_info->y_offset);
        bsp_st7789_spi_write_data8(y_end >> 8);
        bsp_st7789_spi_write_data8(y_end + g_st7789_info->y_offset);
        // set the Y coordinates
    }

    bsp_st7789_spi_write_cmd8(0x2C);
    // gpio_put(BSP_ST7789_CS_PIN, 0);
    // gpio_put(BSP_ST7789_DC_PIN, 1);
}

void bsp_st7789_set_rotation(uint16_t rotation)
{
    uint8_t data;
    uint16_t swap;

    bsp_st7789_spi_write_cmd8(0x36);
    switch (rotation)
    {
    case 1:
        data = 0x60;
        break;
    case 2:
        data = 0xc0;
        break;
    case 3:
        data = 0xa0;
        break;
    default:
        data = 0x00;
        break;
    }
    bsp_st7789_spi_write_data8(data);
    if (rotation == 1 || rotation == 3)
    {
        if (g_st7789_info->width < g_st7789_info->height)
        {
            swap = g_st7789_info->width;
            g_st7789_info->width = g_st7789_info->height;
            g_st7789_info->height = swap;

            swap = g_st7789_info->y_offset;
            g_st7789_info->y_offset = g_st7789_info->x_offset;
            g_st7789_info->x_offset = swap;
        }
    }
    else
    {
        if (g_st7789_info->width > g_st7789_info->height)
        {
            swap = g_st7789_info->width;
            g_st7789_info->width = g_st7789_info->height;
            g_st7789_info->height = swap;

            swap = g_st7789_info->y_offset;
            g_st7789_info->y_offset = g_st7789_info->x_offset;
            g_st7789_info->x_offset = swap;
        }
    }
    g_st7789_info->rotation = rotation;
}

void bsp_st7789_flush(uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end, uint16_t *color)
{
    uint32_t trans_count = (x_end - x_start + 1) * (y_end - y_start + 1) * 2;
    bsp_st7789_set_window(x_start, y_start, x_end, y_end);
    bsp_st7789_spi_write_data((uint8_t *)color, trans_count);
}

void bsp_st7789_flush_dma(uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end, uint16_t *color)
{
    uint32_t trans_count = (x_end - x_start + 1) * (y_end - y_start + 1) * 2;
    bsp_st7789_set_window(x_start, y_start, x_end, y_end);
    gpio_put(BSP_ST7789_CS_PIN, 0);
    gpio_put(BSP_ST7789_DC_PIN, 1);
    dma_channel_set_trans_count(g_st7789_info->dma_tx_channel,trans_count, true);
    dma_channel_set_read_addr(g_st7789_info->dma_tx_channel, color, true);
}

bsp_st7789_info_t *bsp_st7789_get_info(void)
{
    return g_st7789_info;
}

void bsp_st7789_clear(uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end, uint16_t color)
{
    int16_t width = x_end - x_start  + 1;
    int16_t height = y_end - y_start + 1;

    if (width < 0 || height < 0)
    {
        printf("parameter error\r\n");
        return;
    }
    bsp_st7789_set_window(x_start, y_start, x_end, y_end);
    uint16_t color_arr[width];
    for (int i = 0; i < width; i++)
    {
        color_arr[i] = color;
    }
    for (int i = 0; i < height; i++)
    {
        bsp_st7789_spi_write_data((uint8_t *)color_arr, width * 2);
    }
}

void bsp_st7789_init(bsp_st7789_info_t *st7789_info)
{
    // memcpy(&g_st7789_info, st7789_info, sizeof(bsp_st7789_info_t));
    g_st7789_info = st7789_info;
    bsp_st7789_spi_init();
    bsp_st7789_gpio_init();
    bsp_st7789_reset();
    bsp_st7789_reg_init();
    bsp_st7789_set_rotation(st7789_info->rotation);
    if (g_st7789_info->enabled_dma)
    {
        bsp_st7789_set_window(0, 0, g_st7789_info->width - 1, g_st7789_info->height - 1);
        bsp_st7789_spi_dma_init();
    }
}