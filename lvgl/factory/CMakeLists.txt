

file(GLOB_RECURSE LVGL_UI_SOURCES "lvgl_ui/*.c")

add_executable(factory
    factory.c
    ../lv_port/lv_port.c
    ${LVGL_UI_SOURCES}
    )

pico_enable_stdio_usb(factory 1)
pico_enable_stdio_uart(factory 0)

# pull in common dependencies
target_link_libraries(factory 
    pico_stdlib
    bsp
    lvgl)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(factory)