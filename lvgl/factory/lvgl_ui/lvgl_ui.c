#include "lvgl_ui.h"
#include "tileview/system_tile.h"
#include "tileview/qmi8658_tile.h"
#include "tileview/rgb_tile.h"
#include "tileview/image_tile.h"

void lvgl_ui_init(void)
{
    lv_obj_t *tileview = lv_tileview_create(lv_scr_act());
    lv_obj_set_style_bg_opa(tileview, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(tileview, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_SCROLLED);


    /*Tile1: just a label*/
    lv_obj_t *rgb_tile = lv_tileview_add_tile(tileview, 0, 0, LV_DIR_RIGHT);
    rgb_tile_init(rgb_tile);

    lv_obj_t *system_tile = lv_tileview_add_tile(tileview, 1, 0, LV_DIR_LEFT | LV_DIR_RIGHT);
    system_tile_init(system_tile);

    /*Tile2: a button*/
    lv_obj_t *qmi8658_tile = lv_tileview_add_tile(tileview, 2, 0, LV_DIR_LEFT | LV_DIR_RIGHT);
    qmi8658_tile_init(qmi8658_tile);

    lv_obj_t *image_tile = lv_tileview_add_tile(tileview, 3, 0, LV_DIR_LEFT);
    image_tile_init(image_tile);

    

}