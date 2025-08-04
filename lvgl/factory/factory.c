#include <stdio.h>
#include "pico/stdlib.h"

#include "bsp_i2c.h"
#include "bsp_i2s.h"

#include "../lv_port/lv_port.h"
#include "demos/lv_demos.h"

#include "hardware/clocks.h"
#include "bsp_battery.h"
#include "bsp_lcd_brightness.h"

#include "lvgl_ui/lvgl_ui.h"
#include "bsp_pcf85063.h"
#include "bsp_qmi8658.h"
#include "hardware/adc.h"
#include "bsp_st7789.h"
#include "bsp_cst328.h"
#include "hardware/adc.h"

uint16_t color_arr[6] = {0xf800, 0x07e0, 0x001f, 0xffe0, 0xf81f, 0xfd20};

void set_cpu_clock(uint32_t freq_khz)
{
    set_sys_clock_khz(freq_khz, true);
    clock_configure(
        clk_peri,
        0,
        CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS,
        freq_khz * 1000,
        freq_khz * 1000);
}

int main()
{
    struct tm now_tm;
    stdio_init_all();
    bsp_battery_init(100);
    adc_set_temp_sensor_enabled(true);
    set_cpu_clock(250 * 1000);
    bsp_i2c_init();
    bsp_qmi8658_init();
    bsp_pcf85063_init();
    bsp_pcf85063_get_time(&now_tm);
    if (now_tm.tm_year < 125 || now_tm.tm_year > 130)
    {
        now_tm.tm_year = 2025 - 1900; // The year starts from 1900
        now_tm.tm_mon = 1 - 1;        // Months start from 0 (November = 10)
        now_tm.tm_mday = 1;           // Day of the month
        now_tm.tm_hour = 12;          // Hour
        now_tm.tm_min = 0;            // Minute
        now_tm.tm_sec = 0;            // Second
        now_tm.tm_isdst = -1;         // Automatically detect daylight saving time
        bsp_pcf85063_set_time(&now_tm);
    }
    lv_port_init();
    bsp_cst328_data_t cst328_data;
    bsp_st7789_info_t *st7789_info = bsp_st7789_get_info();

    bsp_st7789_clear(0, 0, st7789_info->width - 1, st7789_info->height - 1, 0xffff);
    bsp_lcd_brightness_init();
    bsp_lcd_brightness_set(80);
    while (!bsp_battery_get_key_level())
    {
        sleep_ms(1);
    }
    bool key_state = true;
    bool key_state_last = true;
    uint16_t rect_size = 3;
    while (true)
    {
        bsp_cst328_read();
        if (bsp_cst328_get_touch_data(&cst328_data))
        {
            if (cst328_data.points > 1)
                rect_size = 30;
            else
                rect_size = 3;
            
            for (int i = 0; i < cst328_data.points; i++)
            {
                if (cst328_data.coords[i].x > st7789_info->width - rect_size - 1)
                    cst328_data.coords[i].x = st7789_info->width - rect_size - 1;
                if (cst328_data.coords[i].y > st7789_info->height - rect_size - 1)
                    cst328_data.coords[i].y = st7789_info->height - rect_size - 1;
                bsp_st7789_clear(cst328_data.coords[i].x, cst328_data.coords[i].y, cst328_data.coords[i].x + rect_size, cst328_data.coords[i].y + rect_size, color_arr[i]);
            }
        }
        key_state = bsp_battery_get_key_level();
        if (key_state != key_state_last)
        {
            if (key_state == true)
            {
                break;
            }
            key_state_last = key_state;
        }
        sleep_ms(1);
    }
    bsp_st7789_clear(0, 0, st7789_info->width - 1, st7789_info->height - 1, 0xffff);
    lvgl_ui_init();
    while (true)
    {
        lv_timer_handler();
        sleep_ms(1);
    }
}