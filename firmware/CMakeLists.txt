cmake_minimum_required(VERSION 3.13)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(PICO_BOARD pico)
set(PICO_PLATFORM rp2040)

include(cmake/pico_sdk_import.cmake)
include(cmake/pico_extras_import_optional.cmake)

project(gekipi C CXX ASM)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.4.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.4.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

pico_sdk_init()

add_executable(gekipi)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/generated)
pico_generate_pio_header(gekipi ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio
                         OUTPUT_DIR ${CMAKE_CURRENT_LIST_DIR}/generated)

target_sources(gekipi PRIVATE
	${CMAKE_CURRENT_LIST_DIR}/gekipi.c
	${CMAKE_CURRENT_LIST_DIR}/usb_descriptors.c
	${CMAKE_CURRENT_LIST_DIR}/pn532-lib/pn532.c
	${CMAKE_CURRENT_LIST_DIR}/pn532-lib/pn532_pico.c
)

target_include_directories(gekipi PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/pn532-lib
)

pico_add_extra_outputs(gekipi)
pico_set_program_url(gekipi "https://github.com/ljsebald/gekipi")

target_link_libraries(gekipi
    pico_stdlib
    pico_multicore
    pico_unique_id
    hardware_adc
    hardware_pio
    hardware_i2c
    tinyusb_device
    tinyusb_board
    tinyusb_additions
)

pico_enable_stdio_usb(gekipi 1)
