#include "qmi8658_tile.h"
#include "bsp_qmi8658.h"

lv_obj_t *label_accel_x;
lv_obj_t *label_accel_y;
lv_obj_t *label_accel_z;
lv_obj_t *label_gyro_x;
lv_obj_t *label_gyro_y;
lv_obj_t *label_gyro_z;

void qmi8658_time_cb(lv_timer_t *timer)
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

void qmi8658_tile_init(lv_obj_t *parent)
{
    /*Create a list*/
    lv_obj_t *list = lv_list_create(parent);
    lv_obj_set_size(list, lv_pct(100), lv_pct(100));
    lv_obj_center(list);

    lv_obj_t *list_item;

    list_item = lv_list_add_btn(list, NULL, "Accel_x");
    label_accel_x = lv_label_create(list_item);
    lv_label_set_text(label_accel_x, "----");

    list_item = lv_list_add_btn(list, NULL, "Accel_y");
    label_accel_y = lv_label_create(list_item);
    lv_label_set_text(label_accel_y, "----");

    list_item = lv_list_add_btn(list, NULL, "Accel_z");
    label_accel_z = lv_label_create(list_item);
    lv_label_set_text(label_accel_z, "----");

    list_item = lv_list_add_btn(list, NULL, "Gyro_x");
    label_gyro_x = lv_label_create(list_item);
    lv_label_set_text(label_gyro_x, "----");

    list_item = lv_list_add_btn(list, NULL, "Gyro_y");
    label_gyro_y = lv_label_create(list_item);
    lv_label_set_text(label_gyro_y, "----");

    list_item = lv_list_add_btn(list, NULL, "Gyro_z");
    label_gyro_z = lv_label_create(list_item);
    lv_label_set_text(label_gyro_z, "----");
    lv_timer_create(qmi8658_time_cb, 100, NULL);
}