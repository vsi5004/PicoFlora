#ifndef __BSP_CST328_H__
#define __BSP_CST328_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"


#define BSP_CST328_RST_PIN 17
#define BSP_CST328_INT_PIN 18

#define CST328_LCD_TOUCH_MAX_POINTS (5)

#define CST328_DEVICE_ADDR 0x1A

typedef enum
{
    CST328_1ST_TOUCH_ID = 0xd000,
    CST328_1ST_TOUCH_XH,
    CST328_1ST_TOUCH_YH,
    CST328_1ST_TOUCH_XLYL,
    CST328_1ST_TOUCH_PRESSURE,

    CST328_TOUCH_FLAG_AND_NUM,

    CST328_2ND_TOUCH_ID = 0xd007,
    CST328_2ND_TOUCH_XH,
    CST328_2ND_TOUCH_YH,
    CST328_2ND_TOUCH_XLYL,
    CST328_2ND_TOUCH_PRESSURE,

    CST328_3RD_TOUCH_ID,
    CST328_3RD_TOUCH_XH,
    CST328_3RD_TOUCH_YH,
    CST328_3RD_TOUCH_XLYL,
    CST328_3RD_TOUCH_PRESSURE,

    CST328_4TH_TOUCH_ID,
    CST328_4TH_TOUCH_XH,
    CST328_4TH_TOUCH_YH,
    CST328_4TH_TOUCH_XLYL,
    CST328_4TH_TOUCH_PRESSURE,

    CST328_5TH_TOUCH_ID,
    CST328_5TH_TOUCH_XH,
    CST328_5TH_TOUCH_YH,
    CST328_5TH_TOUCH_XLYL,
    CST328_5TH_TOUCH_PRESSURE,

    CST328_ENUM_MODE_DEBUG_INFO = 0xd101,
    CST328_SYSTEM_RESET,
    CST328_REDO_CALIBRATION = 0xd104,
    CST328_DEEP_SLEEP,
    CST328_ENUM_MODE_DEBUG_POINTS = 0xd108,
    CST328_ENUM_MODE_NORMAL,
    CST328_ENUM_MODE_DEBUG_RAWDATA,
    CST328_ENUM_MODE_DEBUG_WRITE,
    CST328_ENUM_MODE_DEBUG_CALIBRATION,
    CST328_ENUM_MODE_DEBUG_DIFF,
    CST328_ENUM_MODE_FACTORY = 0xd119,

    CST328_IC_INFO = 0xd1f4,
    CST328_IC_RES = 0xd1f8,
    CST328_IC_CHECK_CODE_AND_BOOT_TIMER = 0xd1fc,
    CST328_IC_TYPE_AND_PROJECT_ID = 0xd200,
    CST328_IC_VERSION = 0xd204,
} cst328_reg_t;

typedef struct
{
    uint16_t rotation;
    uint16_t width;
    uint16_t height;

} bsp_cst328_info_t;

typedef struct
{
    uint8_t points; // Number of touch points
    bool read_data_done;
    struct
    {
        uint16_t x;        /*!< X coordinate */
        uint16_t y;        /*!< Y coordinate */
        uint16_t pressure; /*!< pressure */
    } coords[CST328_LCD_TOUCH_MAX_POINTS];
}bsp_cst328_data_t;

void bsp_cst328_init(bsp_cst328_info_t *cst328_info);
void bsp_cst328_set_rotation(uint16_t rotation);
bool bsp_cst328_get_touch_data(bsp_cst328_data_t *cst328_data);
void bsp_cst328_read(void);
#endif // __BSP_CST328_H__