# yuleASR Example Applications
#
# Examples are standalone demonstrations of BSW module usage.
# They are NOT built as part of the main test suite.
#
# To build examples:
#   cmake -B build -DBUILD_EXAMPLES=ON
#   cmake --build build --target <example_name>
#
# Available examples:
#   seat_control    - Seat ECU with BswM/ComM state management
#
# Standalone examples (build with provided source + BSW headers):
#   led_blink       - Minimal Mcu+Port+Dio blinky
#   can_demo        - CAN transmission/reception demo
#   can_loopback    - CAN loopback test (requires 2 CAN controllers)
#   commercial_vehicle_gateway - J1939 gateway with OBD-II reader
#   safety_critical_system     - E2E protection demo
#   ccc_digital_key            - CCC digital key reference

cmake_minimum_required(VERSION 3.14)
project(yuleasr_examples C)

if(NOT DEFINED BUILD_EXAMPLES OR NOT BUILD_EXAMPLES)
    message(STATUS "Examples disabled. Use -DBUILD_EXAMPLES=ON to enable.")
    return()
endif()

message(STATUS "Building yuleASR examples")

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/seat_control/CMakeLists.txt)
    add_subdirectory(seat_control)
    message(STATUS "  Example: seat_control")
endif()
