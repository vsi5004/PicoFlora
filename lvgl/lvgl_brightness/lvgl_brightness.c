#include <stdio.h>
#include "pico/stdlib.h"

#include "bsp_i2c.h"
#include "../lv_port/lv_port.h"
#include "demos/lv_demos.h"

#include "hardware/clocks.h"
#include "bsp_battery.h"
#include "bsp_lcd_brightness.h"

lv_obj_t *label_brightness;

lv_timer_t *brightness_timer = NULL;

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

void slider_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_VALUE_CHANGED)
    {
        // Get the current slider value
        lv_obj_t *slider = lv_event_get_target(e);
        int value = lv_slider_get_value(slider);
        // printf("Slider value: %d\n", value);

        lv_label_set_text_fmt(label_brightness, "%d %%", value);
        bsp_lcd_brightness_set(value);
        // Prevent event from bubbling up
        lv_event_stop_bubbling(e);
    }
}

void lvgl_brightness_ui_init(void)
{
    lv_obj_t *obj = lv_obj_create(lv_scr_act());
    lv_obj_set_size(obj, lv_pct(90), lv_pct(50));
    lv_obj_align(obj, LV_ALIGN_CENTER, 0, 0); 
    // Create slider
    lv_obj_t *slider = lv_slider_create(obj);

    // Set slider direction to horizontal
    lv_slider_set_range(slider, 1, 100);          
    lv_slider_set_value(slider, 80, LV_ANIM_OFF); 

    // Adjust slider size and position
    lv_obj_set_size(slider, lv_pct(80), 30);    
    lv_obj_align(slider, LV_ALIGN_CENTER, 0, 0); 

    lv_obj_set_style_pad_top(obj, 20, 0);
    lv_obj_set_style_pad_bottom(obj, 20, 0);
    // lv_obj_set_style_pad_left(parent, 50, 0);
    // lv_obj_set_style_pad_right(parent, 50, 0);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_GESTURE_BUBBLE);
    // Add event callback (optional)
    lv_obj_add_event_cb(slider, slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    label_brightness = lv_label_create(obj);
    lv_label_set_text(label_brightness, "80%");
    lv_obj_align(label_brightness, LV_ALIGN_TOP_MID, 0, 0);

}

int main()
{
    stdio_init_all();
    bsp_battery_init(100);
    set_cpu_clock(220 * 1000);
    bsp_i2c_init();
    lv_port_init();
    bsp_lcd_brightness_init();
    bsp_lcd_brightness_set(80);

    lvgl_brightness_ui_init();


    while (true)
    {
        lv_timer_handler();
        sleep_ms(5);
    }
}