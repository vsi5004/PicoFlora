#include "bsp_cst328.h"
#include "pico/stdlib.h"
#include "bsp_i2c.h"

bsp_cst328_info_t *g_cst328_info;
bsp_cst328_data_t g_cst328_data;

uint8_t g_rotation;

void bsp_cst328_reg_read_byte(uint16_t reg_addr, uint8_t *data, size_t len)
{
    bsp_i2c_read_reg16(CST328_DEVICE_ADDR, reg_addr, data, len);
}

void bsp_cst328_reg_write_byte(uint16_t reg_addr, uint8_t *data, size_t len)
{
    bsp_i2c_write_reg16(CST328_DEVICE_ADDR, reg_addr, data, len);
}

void bsp_cst328_reset(void)
{
    gpio_put(BSP_CST328_RST_PIN, 0);
    sleep_ms(10);
    gpio_put(BSP_CST328_RST_PIN, 1);
    sleep_ms(50);
}

void bsp_cst328_read(void)
{
    uint8_t buf[28];
    uint8_t points = 0;
    uint8_t num = 0;
    if (!g_cst328_data.read_data_done)
        return;
    g_cst328_data.read_data_done = false;
    bsp_cst328_reg_read_byte(CST328_TOUCH_FLAG_AND_NUM, &buf[0], 1);
    if ((buf[0] & 0x0F) != 0x00)
    {
        points = buf[0] & 0x0F;
        bsp_cst328_reg_read_byte(CST328_1ST_TOUCH_ID, &buf[1], 27);

        if ((buf[1] & 0x0f) != 0x06)
        {
            g_cst328_data.points = 0;
            return;
        }

        if (points > CST328_LCD_TOUCH_MAX_POINTS)
            points = CST328_LCD_TOUCH_MAX_POINTS;
        g_cst328_data.points = (uint8_t)points;

        for (int i = 0; i < points; i++)
        {
            if (i > 0)
                num = 2;
            g_cst328_data.coords[i].x = (uint16_t)(((uint16_t)buf[(i * 5) + 2 + num] << 4) + ((buf[(i * 5) + 4 + num] & 0xF0) >> 4));
            g_cst328_data.coords[i].y = (uint16_t)(((uint16_t)buf[(i * 5) + 3 + num] << 4) + (buf[(i * 5) + 4 + num] & 0x0F));
            g_cst328_data.coords[i].pressure = ((uint16_t)buf[(i * 5) + 5 + num]);
        }
    }
}

bool bsp_cst328_get_touch_data(bsp_cst328_data_t *cst328_data)
{
    memcpy(cst328_data, &g_cst328_data, sizeof(bsp_cst328_data_t));
    cst328_data->points = g_cst328_data.points;
    g_cst328_data.points = 0;

    switch (g_cst328_info->rotation)
    {
    case 1:
        for (int i = 0; i < cst328_data->points; i++)
        {
            cst328_data->coords[i].x = g_cst328_data.coords[i].y;
            cst328_data->coords[i].y = g_cst328_info->height - 1 - g_cst328_data.coords[i].x;
        }
        break;
    case 2:
        for (int i = 0; i < cst328_data->points; i++)
        {
            cst328_data->coords[i].x = g_cst328_info->width - 1 - g_cst328_data.coords[i].x;
            cst328_data->coords[i].y = g_cst328_info->height - 1 - g_cst328_data.coords[i].y;
        }
        break;

    case 3:
        for (int i = 0; i < cst328_data->points; i++)
        {
            cst328_data->coords[i].x = g_cst328_info->width - g_cst328_data.coords[i].y;
            cst328_data->coords[i].y = g_cst328_data.coords[i].x;
        }
        break;
    default:
        break;
    }

    // printf("x:%d, y:%d rotation:%d\r\n", cst328_data->coords[0].x, cst328_data->coords[0].y, g_cst328_info->rotation);
    // printf("g_cst328_info ->width:%d ->height:%d  rotation:%d \r\n", g_cst328_info->width, g_cst328_info->height, g_cst328_info->rotation);
    if (cst328_data->points == 0)
        return false;

    return true;
}

void bsp_cst328_set_rotation(uint16_t rotation)
{
    uint16_t swap;
    g_cst328_info->rotation = rotation;
    if (rotation == 1 || rotation == 3)
    {
        if (g_cst328_info->width < g_cst328_info->height)
        {
            swap = g_cst328_info->width;
            g_cst328_info->width = g_cst328_info->height;
            g_cst328_info->height = swap;
        }
    }
    else
    {
        if (g_cst328_info->width > g_cst328_info->height)
        {
            swap = g_cst328_info->width;
            g_cst328_info->width = g_cst328_info->height;
            g_cst328_info->height = swap;
        }
    }
}

// 外部中断服务例程
void gpio_isr_handler(uint gpio, uint32_t events)
{
    if (gpio == BSP_CST328_INT_PIN)
    {
        // 这里可以处理按键按下事件
        // printf("Button pressed!\n");
        // bsp_cst328_read();
        g_cst328_data.read_data_done = true;
    }
}

void bsp_cst328_init(bsp_cst328_info_t *cst328_info)
{
    uint8_t buf[24] = {0};
    uint16_t check_code;
    gpio_init(BSP_CST328_RST_PIN);
    gpio_set_dir(BSP_CST328_RST_PIN, GPIO_OUT);

    bsp_cst328_reset();
    g_cst328_info = cst328_info;

    bsp_cst328_set_rotation(cst328_info->rotation);
    // g_rotation = rotation;
    // 查看触摸屏信息模式
    bsp_cst328_reg_write_byte(CST328_ENUM_MODE_DEBUG_INFO, NULL, 0);

    bsp_cst328_reg_read_byte(CST328_IC_INFO, buf, 24);
    printf("D1F4:0x%02x,0x%02x,0x%02x,0x%02x\r\n", buf[0], buf[1], buf[2], buf[3]);
    printf("D1F8:0x%02x,0x%02x,0x%02x,0x%02x\r\n", buf[4], buf[5], buf[6], buf[7]);
    printf("D1FC:0x%02x,0x%02x,0x%02x,0x%02x\r\n", buf[8], buf[9], buf[10], buf[11]);
    printf("D200:0x%02x,0x%02x,0x%02x,0x%02x\r\n", buf[12], buf[13], buf[14], buf[15]);
    printf("D204:0x%02x,0x%02x,0x%02x,0x%02x\r\n", buf[16], buf[17], buf[18], buf[19]);
    printf("D208:0x%02x,0x%02x,0x%02x,0x%02x\r\n", buf[20], buf[21], buf[22], buf[23]);
    check_code = (((uint16_t)buf[11] << 8) | buf[10]);

    // 进入正常报点模式
    bsp_cst328_reg_write_byte(CST328_ENUM_MODE_NORMAL, NULL, 0);
    if (check_code == 0xcaca)
        printf("Find CST328!\r\n");
    else
        printf("CST328 not found!\r\n");

    gpio_init(BSP_CST328_INT_PIN);
    gpio_set_dir(BSP_CST328_INT_PIN, GPIO_IN);
    gpio_pull_up(BSP_CST328_INT_PIN);
    gpio_set_irq_enabled_with_callback(BSP_CST328_INT_PIN, GPIO_IRQ_EDGE_FALL, true, gpio_isr_handler);
}
