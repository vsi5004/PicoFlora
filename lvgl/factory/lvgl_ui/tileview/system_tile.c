#include "system_tile.h"

#include "bsp_lcd_brightness.h"
#include "bsp_pcf85063.h"
#include "bsp_battery.h"
#include "hardware/clocks.h"
#include "hardware/flash.h"
#include "hardware/sync.h"
#include "hardware/adc.h"


#include "bsp_i2s.h"
#include "../i2stest8bit.h"


#include "f_util.h"
#include "crash.h"
#include "hw_config.h"
#include "my_debug.h"
#include "my_rtc.h"
#include "sd_card.h"
#include "ff.h"
//
#include "diskio.h" /* Declarations of disk functions */
//
// #include "command.h"

lv_obj_t *label_brightness;
lv_obj_t *label_time;
lv_obj_t *label_date;
lv_obj_t *label_adc_raw;
lv_obj_t *label_battery_voltage;
lv_obj_t *label_chip_temp;

uint32_t sample32bit[SOUND_LENGTH];


static void slider_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_VALUE_CHANGED)
    {
        // 获取当前滑块的值
        lv_obj_t *slider = lv_event_get_target(e);
        int value = lv_slider_get_value(slider);
        // printf("Slider value: %d\n", value);

        lv_label_set_text_fmt(label_brightness, "%d %%", value);
        bsp_lcd_brightness_set(value);
        // bsp_display_handle_t display = bsp_display_get_handle();
        // display->set_brightness(display, value);
        // 阻止事件向上传递
        lv_event_stop_bubbling(e);
    }
}

static void button_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_SHORT_CLICKED)
    {
        bsp_i2s_output(sample32bit, SOUND_LENGTH, false);
        printf("button_event_cb\n");
    }
}


float read_chip_temp(void)
{

    /* 12-bit conversion, assume max value == ADC_VREF == 3.3 V */
    const float conversionFactor = 3.3f / (1 << 12);
    adc_select_input(4);
    float adc = (float)adc_read() * conversionFactor;
    float tempC = 27.0f - (adc - 0.706f) / 0.001721f;
    return tempC;
}

static void system_tile_time_cb(lv_timer_t *timer)
{
    char str_buffer[20];
    float voltage;
    float chip_temp;
    uint16_t adc_raw;
    struct tm now_tm;
    bsp_pcf85063_get_time(&now_tm);
    chip_temp = read_chip_temp();
    bsp_battery_read(&voltage, &adc_raw);
    
    lv_label_set_text_fmt(label_time, "%02d:%02d:%02d", now_tm.tm_hour, now_tm.tm_min, now_tm.tm_sec);
    lv_label_set_text_fmt(label_date, "%04d-%02d-%02d", now_tm.tm_year + 1900, now_tm.tm_mon + 1, now_tm.tm_mday);

    
    lv_label_set_text_fmt(label_adc_raw, "%d", adc_raw);
    sprintf(str_buffer, "%.1f V", voltage);
    lv_label_set_text(label_battery_voltage, str_buffer);
    sprintf(str_buffer, "%.1f C", chip_temp);
    lv_label_set_text(label_chip_temp, str_buffer);

}

void initSample()
{
    // 转换成双声道
    for (size_t index = 0; index < SOUND_LENGTH; ++index)
    {
        sample32bit[index] = (i2stest8bit[index] << 24) | (i2stest8bit[index] << 8);

        sample32bit[index] >>= 1;
    }
}


uint32_t get_sd_card_size(void)
{
    const char *arg = "0";
    sd_card_t *sd_card_p = sd_get_by_drive_prefix(arg);
    FATFS *fs_p = &sd_card_p->state.fatfs;

    DWORD fre_clust, fre_sect, tot_sect;
    FRESULT fr = f_getfree(arg, &fre_clust, &fs_p);
    if (FR_OK != fr) {
        printf("f_getfree error: %s (%d)\n", FRESULT_str(fr), fr);
        return 0;
    }
    /* Get total sectors and free sectors */
    tot_sect = (fs_p->n_fatent - 2) * fs_p->csize;
    fre_sect = fre_clust * fs_p->csize;
    /* Print the free space (assuming 512 bytes/sector) */
    printf("\n%10lu KiB (%lu MiB) total drive space.\n%10lu KiB (%lu MiB) available.\n",
           tot_sect / 2, tot_sect / 2 / 1024,
           fre_sect / 2, fre_sect / 2 / 1024);
    
    return tot_sect / 2 / 1024;
}

void system_tile_init(lv_obj_t *parent)
{

    uint8_t txbuf[4] = {0x9F, 0, 0, 0}; 
    uint8_t rxbuf[4] = {0};        
    
    initSample();
    

    uint64_t sd_card_size_MB = get_sd_card_size();


    uint32_t interrupts = save_and_disable_interrupts();
    flash_do_cmd(txbuf, rxbuf, sizeof(txbuf));
    restore_interrupts(interrupts);

    uint32_t flash_size = 1 << rxbuf[3];

    uint32_t sys_clk = clock_get_hz(clk_sys);
    /*Create a list*/
    lv_obj_t *list = lv_list_create(parent);
    lv_obj_set_size(list, lv_pct(100), lv_pct(80));
    // lv_obj_center(list);
    lv_obj_align(list, LV_ALIGN_TOP_MID, 0, 0);

    lv_obj_t *button = lv_btn_create(parent);
    lv_obj_set_size(button, lv_pct(15), lv_pct(10));
    lv_obj_align(button, LV_ALIGN_BOTTOM_MID, -80, -15);
    lv_obj_add_event_cb(button, button_event_cb, LV_EVENT_SHORT_CLICKED, NULL);

    lv_obj_t *slider = lv_slider_create(parent);
    lv_slider_set_range(slider, 1, 100);    
    lv_slider_set_value(slider, 80, LV_ANIM_OFF); 

    lv_obj_set_size(slider, lv_pct(50), lv_pct(5));     
    lv_obj_align(slider, LV_ALIGN_BOTTOM_MID, 30, -20); 
    lv_obj_add_event_cb(slider, slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    lv_obj_t *list_item;
    list_item = lv_list_add_btn(list, NULL, "Chip");
    lv_obj_t *label_chip = lv_label_create(list_item);
    lv_label_set_text(label_chip, "RP2350A");

    list_item = lv_list_add_btn(list, NULL, "ChipTemp");
    label_chip_temp = lv_label_create(list_item);
    lv_label_set_text(label_chip_temp, "--- C");

    list_item = lv_list_add_btn(list, NULL, "ChipFreq");
    lv_obj_t *label_chip_freq = lv_label_create(list_item);
    lv_label_set_text_fmt(label_chip_freq, "%d MHz", sys_clk / 1000 / 1000);

    list_item = lv_list_add_btn(list, NULL, "Brightness");
    label_brightness = lv_label_create(list_item);
    lv_label_set_text(label_brightness, "80 %");

    list_item = lv_list_add_btn(list, NULL, "RAM");
    lv_obj_t *label_ram = lv_label_create(list_item);
    lv_label_set_text(label_ram, "520 KB");

    list_item = lv_list_add_btn(list, NULL, "Flash");
    lv_obj_t *label_flash = lv_label_create(list_item);
    lv_label_set_text_fmt(label_flash, "%d MB", flash_size / 1024 / 1024);

    list_item = lv_list_add_btn(list, NULL, "SDCard");
    lv_obj_t *label_sd_card = lv_label_create(list_item);
    lv_label_set_text_fmt(label_sd_card, "%d MB", sd_card_size_MB);

    list_item = lv_list_add_btn(list, NULL, "Battery");
    label_battery_voltage = lv_label_create(list_item);
    lv_label_set_text(label_battery_voltage, "--- V");

    list_item = lv_list_add_btn(list, NULL, "ADC_Raw");
    label_adc_raw = lv_label_create(list_item);
    lv_label_set_text(label_adc_raw, "----");

    list_item = lv_list_add_btn(list, NULL, "Date");
    label_date = lv_label_create(list_item);
    lv_label_set_text(label_date, "2025-01-01");

    list_item = lv_list_add_btn(list, NULL, "Time");
    label_time = lv_label_create(list_item);
    lv_label_set_text(label_time, "12:00:00");

    lv_timer_create(system_tile_time_cb, 1000, NULL);
}