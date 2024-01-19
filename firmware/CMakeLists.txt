cmake_minimum_required(VERSION 3.13)
include(cmake/pico_sdk_import.cmake)
include(cmake/pico_extras_import_optional.cmake)
project(gekipi C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(PICO_BOARD pico)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.4.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.4.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

pico_sdk_init()

add_executable(gekipi)

target_sources(gekipi PRIVATE
	${CMAKE_CURRENT_LIST_DIR}/gekipi.c
	${CMAKE_CURRENT_LIST_DIR}/usb_descriptors.c
)

target_include_directories(gekipi PUBLIC ${CMAKE_CURRENT_LIST_DIR})

pico_add_extra_outputs(gekipi)
pico_set_program_url(gekipi "https://github.com/ljsebald/gekipi")

target_link_libraries(gekipi
    pico_stdlib
    pico_unique_id
    hardware_adc
    tinyusb_device
    tinyusb_board
    tinyusb_additions
)

pico_enable_stdio_usb(gekipi 1)
