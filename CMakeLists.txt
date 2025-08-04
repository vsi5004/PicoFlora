# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.1)
set(toolchainVersion 13_3_Rel1)
set(picotoolVersion 2.1.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico2 CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(PicoFlora C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(PicoFlora
        main.c
        lvgl/lv_port/lv_port.c
        drivers/mcp23017/mcp23017_class.c
        drivers/gpio_abstraction/gpio_abstraction.c
        drivers/stepper/stepper_mcp23017.c
        )

# Hardware-specific examples in subdirectories:
add_subdirectory(libraries)
add_subdirectory(drivers/logging)
add_subdirectory(drivers/stepper)
add_subdirectory(drivers/mcp23017)
add_subdirectory(lvgl/lvgl_screen)

# pull in common dependencies
target_link_libraries(PicoFlora 
    pico_stdlib
    bsp
    lvgl
    hardware_clocks
    logging
    stepper
    mcp23017
    lvgl_screen
    )

# Include directories
target_include_directories(PicoFlora PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/lvgl
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/logging
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/stepper
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/mcp23017
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/gpio_abstraction
    ${CMAKE_CURRENT_SOURCE_DIR}/lvgl/lvgl_screen
    )

pico_enable_stdio_usb(PicoFlora 1)
pico_enable_stdio_uart(PicoFlora 0)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(PicoFlora)

