add_executable(lvgl_qmi8658
    lvgl_qmi8658.c
    ../lv_port/lv_port.c
    )

pico_enable_stdio_usb(lvgl_qmi8658 1)
pico_enable_stdio_uart(lvgl_qmi8658 0)

# pull in common dependencies
target_link_libraries(lvgl_qmi8658 
    pico_stdlib
    bsp
    lvgl
    )

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(lvgl_qmi8658)