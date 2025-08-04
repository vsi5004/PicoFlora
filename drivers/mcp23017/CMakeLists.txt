# MCP23017 I2C I/O Expander Driver

add_library(mcp23017 STATIC
    mcp23017.c
)

target_include_directories(mcp23017 PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(mcp23017
    pico_stdlib
    hardware_i2c
)

# Make the library available to the parent project
target_include_directories(mcp23017 INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
