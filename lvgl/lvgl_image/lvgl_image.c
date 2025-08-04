#include <stdio.h>
#include "pico/stdlib.h"

#include "bsp_i2c.h"
#include "../lv_port/lv_port.h"

#include "hardware/clocks.h"
#include "bsp_battery.h"
#include "bsp_lcd_brightness.h"

lv_obj_t *img = NULL;

#define MAX_FILENAME_LENGTH 256
#define MAX_FILE_COUNT 100

char bin_filenames[MAX_FILE_COUNT][MAX_FILENAME_LENGTH];
int bin_file_count = 0;

int is_bin_file(const char *filename)
{
    const char *ext = strrchr(filename, '.');
    return (ext != NULL && strcmp(ext, ".bin") == 0);
}

void read_bin_files(const char *dir_path)
{
    lv_fs_dir_t dir;
    lv_fs_res_t res = lv_fs_dir_open(&dir, dir_path);
    if (res != LV_FS_RES_OK)
    {
        return;
    }
    char filename[MAX_FILENAME_LENGTH];
    while (lv_fs_dir_read(&dir, filename) == LV_FS_RES_OK && filename[0] != '\0')
    {
        if (is_bin_file(filename))
        {
            if (bin_file_count < MAX_FILE_COUNT)
            {
                strncpy(bin_filenames[bin_file_count], filename, MAX_FILENAME_LENGTH - 1);
                bin_filenames[bin_file_count][MAX_FILENAME_LENGTH - 1] = '\0';
                bin_file_count++;
            }
            else
            {
                break;
            }
        }
    }
    lv_fs_dir_close(&dir);
}

void print_bin_filenames()
{
    for (int i = 0; i < bin_file_count; i++)
    {
        printf("Found .bin file: %s \r\n", bin_filenames[i]);
    }
}

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

static void img_callback(lv_timer_t *timer)
{
    char str_buf[200];
    static uint16_t img_index = 0;
    sprintf(str_buf, "0:images/%s", bin_filenames[img_index]);
    lv_img_set_src(img, str_buf);
    if (++img_index >= bin_file_count)
    {
        img_index = 0;
    }
}

static void img_gesture_event_cb(lv_event_t *e)
{
    char str_buf[200];
    static int img_index = 0;

    lv_event_code_t code = lv_event_get_code(e);
    
    if (code == LV_EVENT_GESTURE)
    {
        // printf("img_gesture_event_cb\r\n");
        lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());

        if (dir == LV_DIR_LEFT)
        {
            if (++img_index >= bin_file_count)
            {
                img_index = 0;
            }
            lv_indev_wait_release(lv_indev_get_act());
        }
        else if (dir == LV_DIR_RIGHT)
        {
            if (--img_index < 0)
            {
                img_index = bin_file_count - 1;
            }
            lv_indev_wait_release(lv_indev_get_act());
        }
        sprintf(str_buf, "0:images/%s", bin_filenames[img_index]);
        lv_img_set_src(img, str_buf);
    }
}

int main()
{
    lv_obj_t *obj;
    char str_buf[200];
    stdio_init_all();
    bsp_battery_init(100);
    set_cpu_clock(220 * 1000);
    bsp_i2c_init();
    lv_port_init();
    bsp_lcd_brightness_init();
    bsp_lcd_brightness_set(50);

    read_bin_files("0:images");
    print_bin_filenames();
    // obj = lv_obj_create(lv_scr_act());
    // lv_obj_set_size(obj, 240, 320);
    // lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE); 
    img = lv_img_create(lv_scr_act());
    lv_obj_set_size(img, 240, 320);
    sprintf(str_buf, "0:images/%s", bin_filenames[0]);
    lv_img_set_src(img, str_buf);
    lv_obj_clear_flag(img, LV_OBJ_FLAG_SCROLLABLE); 
    lv_obj_add_flag(img, LV_OBJ_FLAG_GESTURE_BUBBLE);
    // lv_obj_add_event_cb(img, img_gesture_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(lv_scr_act(), img_gesture_event_cb, LV_EVENT_GESTURE, NULL);
    // lv_obj_set_gesture_dir(img, LV_DIR_LEFT | LV_DIR_RIGHT);
    // lv_timer_create(img_callback, 1000, NULL);
    while (true)
    {
        lv_timer_handler();
        sleep_ms(1);
    }
}