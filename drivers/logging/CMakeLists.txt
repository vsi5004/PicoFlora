# Logging library
add_library(logging STATIC
    logging.c
)

target_include_directories(logging PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(logging
    pico_stdlib
)
