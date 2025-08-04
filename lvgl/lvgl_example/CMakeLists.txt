add_executable(lvgl_example
    lvgl_example.c
    ../lv_port/lv_port.c
    )

pico_enable_stdio_usb(lvgl_example 1)
pico_enable_stdio_uart(lvgl_example 0)

# pull in common dependencies
target_link_libraries(lvgl_example 
    pico_stdlib
    bsp
    lvgl
    lvgl::demos)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(lvgl_example)