cmake_minimum_required(VERSION 3.13)

# Create stepper library first
add_library(stepper
    stepper_driver.c
)

# Generate PIO header from .pio file
pico_generate_pio_header(stepper ${CMAKE_CURRENT_LIST_DIR}/stepper.pio)

target_link_libraries(stepper
    pico_stdlib
    hardware_clocks
    hardware_pio
    logging
)

target_include_directories(stepper PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
