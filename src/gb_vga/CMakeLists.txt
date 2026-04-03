cmake_minimum_required(VERSION 3.16)

# HDMI/DVI-only transition (VGA path disabled by design)
include(../pico-sdk/external/pico_sdk_import.cmake)

project(gb_hdmi_transition C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

include(FetchContent)
FetchContent_Declare(
    picodvi
    GIT_REPOSITORY https://github.com/Wren6991/PicoDVI.git
    GIT_TAG b58f3064377ef1dc484f6bad3337abbb88d32ca6
)
FetchContent_MakeAvailable(picodvi)

add_subdirectory(${picodvi_SOURCE_DIR}/software/libdvi ${picodvi_BINARY_DIR}/libdvi)

# Keep artifact name aligned with HDMI path. VGA target is intentionally disabled.
add_executable(gb_hdmi
    ../gb_hdmi/gb_hdmi.c
)

target_include_directories(gb_hdmi PRIVATE
    ${picodvi_SOURCE_DIR}/software/include
)

target_compile_definitions(gb_hdmi PRIVATE
    DVI_DEFAULT_SERIAL_CONFIG=picodvi_pmod0_cfg
)

target_link_libraries(gb_hdmi PRIVATE
    pico_stdlib
    pico_multicore
    libdvi
    hardware_clocks
    hardware_gpio
    hardware_irq
    hardware_sync
    hardware_vreg
)

pico_set_program_name(gb_hdmi "gb_hdmi")
pico_set_program_version(gb_hdmi "1.0.0")

pico_add_extra_outputs(gb_hdmi)
