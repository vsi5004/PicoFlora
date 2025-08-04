cmake_minimum_required(VERSION 3.13)

# Create LVGL screen library
add_library(lvgl_screen
    stepper_screen.c
    lock_screen.c
    screen_manager.c
)

target_link_libraries(lvgl_screen
    pico_stdlib
    lvgl
    bsp
    mcp23017
)

target_include_directories(lvgl_screen PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../stepper
    ${CMAKE_CURRENT_SOURCE_DIR}/../../libraries/bsp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../mcp23017
)
