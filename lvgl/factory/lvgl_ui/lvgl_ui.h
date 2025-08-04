#ifndef __LVGL_UI_H__
#define __LVGL_UI_H__

#include <stdio.h>
#include "lvgl.h"


#ifdef __cplusplus
extern "C" {
#endif
LV_IMG_DECLARE(lv_logo_wx);    // assets/bg1.png

void _ui_screen_change(lv_obj_t ** target, lv_scr_load_anim_t fademode, int spd, int delay, void (*target_init)(void));
void lvgl_ui_init(void);

#ifdef __cplusplus
}
#endif

#endif

