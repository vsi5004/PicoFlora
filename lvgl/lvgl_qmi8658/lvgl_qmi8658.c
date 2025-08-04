#include <stdio.h>
#include "pico/stdlib.h"

#include "bsp_i2c.h"
#include "../lv_port/lv_port.h"
#include "demos/lv_demos.h"

#include "hardware/clocks.h"
#include "bsp_battery.h"
#include "bsp_lcd_brightness.h"
#include "bsp_qmi8658.h"

lv_obj_t *label_accel_x;
lv_obj_t *label_accel_y;
lv_obj_t *label_accel_z;
lv_obj_t *label_gyro_x;
lv_obj_t *label_gyro_y;
lv_obj_t *label_gyro_z;

lv_timer_t *qmi8658_timer = NULL;

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


static void qmi8658_callback(lv_timer_t *timer)
{
    qmi8658_data_t data;
    bsp_qmi8658_read_data(&data);
    lv_label_set_text_fmt(label_accel_x, "%d", data.acc_x);
    lv_label_set_text_fmt(label_accel_y, "%d", data.acc_y);
    lv_label_set_text_fmt(label_accel_z, "%d", data.acc_z);

    lv_label_set_text_fmt(label_gyro_x, "%d", data.gyr_x);
    lv_label_set_text_fmt(label_gyro_y, "%d", data.gyr_y);
    lv_label_set_text_fmt(label_gyro_z, "%d", data.gyr_z);
}

void lvgl_qmi8658_ui_init(void)
{
    lv_obj_t *list = lv_list_create(lv_scr_act());
    lv_obj_set_size(list, lv_pct(100), lv_pct(100));

    lv_obj_t *list_item = lv_list_add_btn(list, NULL, "accel_x");
    label_accel_x = lv_label_create(list_item);
    lv_label_set_text(label_accel_x, "0");   

    list_item = lv_list_add_btn(list, NULL, "accel_y");
    label_accel_y = lv_label_create(list_item);
    lv_label_set_text(label_accel_y, "0");   

    list_item = lv_list_add_btn(list, NULL, "accel_z");
    label_accel_z = lv_label_create(list_item);
    lv_label_set_text(label_accel_z, "0");   

    list_item = lv_list_add_btn(list, NULL, "gyro_x");
    label_gyro_x = lv_label_create(list_item);
    lv_label_set_text(label_gyro_x, "0");   

    list_item = lv_list_add_btn(list, NULL, "gyro_y");
    label_gyro_y = lv_label_create(list_item);
    lv_label_set_text(label_gyro_y, "0");   

    list_item = lv_list_add_btn(list, NULL, "gyro_z");
    label_gyro_z = lv_label_create(list_item);
    lv_label_set_text(label_gyro_z, "0");   

    qmi8658_timer = lv_timer_create(qmi8658_callback, 100, NULL);
}



int main()
{
    stdio_init_all();
    stdio_init_all();
    bsp_battery_init(100);
    set_cpu_clock(220 * 1000);
    bsp_i2c_init();
    lv_port_init();
    bsp_lcd_brightness_init();
    bsp_lcd_brightness_set(50);
    bsp_qmi8658_init();

    lvgl_qmi8658_ui_init();

    while (true)
    {
        lv_timer_handler();
        sleep_ms(1);
    }
}