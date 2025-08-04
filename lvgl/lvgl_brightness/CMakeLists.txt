add_executable(lvgl_brightness
    lvgl_brightness.c
    ../lv_port/lv_port.c
    )

pico_enable_stdio_usb(lvgl_brightness 1)
pico_enable_stdio_uart(lvgl_brightness 0)

# pull in common dependencies
target_link_libraries(lvgl_brightness 
    pico_stdlib
    bsp
    lvgl
    )

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(lvgl_brightness)