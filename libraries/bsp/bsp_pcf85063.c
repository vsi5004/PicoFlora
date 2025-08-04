#include "bsp_pcf85063.h"
#include "bsp_i2c.h"
#include <time.h>

// 读取QMI8658寄存器的值
static void bsp_pcf85063_reg_read(uint8_t reg_addr, uint8_t *data, size_t len)
{
    bsp_i2c_read_reg8(PCF85063_DEVICE_ADDR, reg_addr, data, len);
}

// 给QMI8658的寄存器写值
static void bsp_pcf85063_reg_write_byte(uint8_t reg_addr, uint8_t *data, size_t len)
{
    bsp_i2c_write_reg8(PCF85063_DEVICE_ADDR, reg_addr, data, len);
}

void bsp_pcf85063_init(void)
{
    uint8_t seconds = 0;
    bsp_pcf85063_reg_read(PCF85063_SECONDS, (uint8_t *)&seconds, 1);
    if (seconds & 0x80)
        printf("oscillator_stop detected\n");
    else
        printf("RTC has beeing kept running!\n");
}

static uint8_t dec2bcd(uint8_t value)
{
    return ((value / 10) << 4) + (value % 10);
}

static uint8_t bcd2dec(uint8_t value)
{
    return (((value & 0xF0) >> 4) * 10) + (value & 0xF);
}

void bsp_pcf85063_get_time(struct tm *now_tm)
{
    uint8_t time_data[7];
    bsp_pcf85063_reg_read(PCF85063_SECONDS, time_data, 7);
    now_tm->tm_sec = bcd2dec(time_data[0] & 0x7F);
    now_tm->tm_min = bcd2dec(time_data[1] & 0x7F);
    now_tm->tm_hour = bcd2dec(time_data[2] & 0x3F);
    now_tm->tm_mday = bcd2dec(time_data[3] & 0x3F);
    now_tm->tm_wday = bcd2dec(time_data[4] & 0x7);
    now_tm->tm_mon = bcd2dec(time_data[5] & 0x1F) - 1;
    now_tm->tm_year = bcd2dec(time_data[6]) + 100;
}

void bsp_pcf85063_set_time(struct tm *now_tm)
{
    uint8_t time_data[7];
    time_t now_time = mktime(now_tm);

    time_data[0] = dec2bcd(now_tm->tm_sec) & 0x7F;
    time_data[1] = dec2bcd(now_tm->tm_min) & 0x7F;
    time_data[2] = dec2bcd(now_tm->tm_hour) & 0x3F;
    time_data[3] = dec2bcd(now_tm->tm_mday) & 0x3F;
    time_data[4] = dec2bcd(now_tm->tm_wday) & 0x7;
    time_data[5] = dec2bcd(now_tm->tm_mon + 1) & 0x1F;
    time_data[6] = dec2bcd((now_tm->tm_year - 100) % 100);

    bsp_pcf85063_reg_write_byte(PCF85063_SECONDS, time_data, 7);
}

// static void bsp_pcf85063_task(void *arg)
// {
//     struct tm now_tm;
//     // bsp_pcf85063_get_time(&now_tm);

//     // if (now_tm.tm_year < 124 && now_tm.tm_year > 130 )
//     {
//         now_tm.tm_year = 2024 - 1900; // The year starts from 1900
//         now_tm.tm_mon = 11 - 1;       // Months start from 0 (November = 10)
//         now_tm.tm_mday = 22;          // Day of the month
//         now_tm.tm_hour = 12;          // Hour
//         now_tm.tm_min = 0;            // Minute
//         now_tm.tm_sec = 0;            // Second
//         now_tm.tm_isdst = -1;         // Automatically detect daylight saving time
//         bsp_pcf85063_set_time(&now_tm);
//     }

//     while (1)
//     {
//         bsp_pcf85063_get_time(&now_tm);
//         printf("time: %s\n", asctime(&now_tm));
//         vTaskDelay(pdMS_TO_TICKS(1000));
//     }
// }

// void bsp_pcf85063_test(void)
// {
//     xTaskCreate(bsp_pcf85063_task, "bsp_pcf85063_task", 256, NULL, tskIDLE_PRIORITY + 1, NULL);
// }