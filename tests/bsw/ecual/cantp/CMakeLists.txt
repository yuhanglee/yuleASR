# CanTp unit tests — substantiated against the production CanTp implementation.
# CanTp has no ECUAL library target, so CanTp.c and the link-time config
# CanTp_Lcfg.c are compiled directly into the test executable.
# The test defines its own Det/CanIf/PduR stubs, so mock_det.c is not linked.

cmake_minimum_required(VERSION 3.16)

add_executable(cantp_test
    test_cantp.c
    ${CMAKE_SOURCE_DIR}/src/bsw/ecual/cantp/src/CanTp.c
    ${CMAKE_SOURCE_DIR}/src/bsw/ecual/cantp/src/CanTp_Lcfg.c
)

target_include_directories(cantp_test
    PRIVATE
        ${TEST_FRAMEWORK_INCLUDE}
        ${CMAKE_SOURCE_DIR}/src/bsw/ecual/cantp/include
        ${CMAKE_SOURCE_DIR}/src/bsw/ecual/canif/include
        ${CMAKE_SOURCE_DIR}/src/bsw/mcal/can/include
        ${CMAKE_SOURCE_DIR}/src/bsw/services/pdur/include
        ${CMAKE_SOURCE_DIR}/include/autosar
)

target_compile_options(cantp_test PRIVATE
    -Wall
    -Wextra
    -g
    -O0
)

set_target_properties(cantp_test PROPERTIES
    C_STANDARD 99
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests
)

if(TEST_FRAMEWORK_LIB)
    target_link_libraries(cantp_test PRIVATE ${TEST_FRAMEWORK_LIB})
endif()

add_test(NAME cantp_test COMMAND cantp_test)

# Strict padding-rejection variant: recompiles CanTp.c with
# CANTP_REJECT_INVALID_PADDING=STD_ON to verify the drop path.
add_executable(cantp_test_strict
    test_cantp_padding_strict.c
    ${CMAKE_SOURCE_DIR}/src/bsw/ecual/cantp/src/CanTp.c
    ${CMAKE_SOURCE_DIR}/src/bsw/ecual/cantp/src/CanTp_Lcfg.c
)

target_include_directories(cantp_test_strict
    PRIVATE
        ${TEST_FRAMEWORK_INCLUDE}
        ${CMAKE_SOURCE_DIR}/src/bsw/ecual/cantp/include
        ${CMAKE_SOURCE_DIR}/src/bsw/ecual/canif/include
        ${CMAKE_SOURCE_DIR}/src/bsw/mcal/can/include
        ${CMAKE_SOURCE_DIR}/src/bsw/services/pdur/include
        ${CMAKE_SOURCE_DIR}/include/autosar
)

target_compile_definitions(cantp_test_strict PRIVATE
    CANTP_REJECT_INVALID_PADDING=STD_ON
)

target_compile_options(cantp_test_strict PRIVATE
    -Wall
    -Wextra
    -g
    -O0
)

set_target_properties(cantp_test_strict PROPERTIES
    C_STANDARD 99
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests
)

if(TEST_FRAMEWORK_LIB)
    target_link_libraries(cantp_test_strict PRIVATE ${TEST_FRAMEWORK_LIB})
endif()

add_test(NAME cantp_test_strict COMMAND cantp_test_strict)

message(STATUS "  Unit test: cantp_test (CAN Transport Protocol)")
message(STATUS "  Unit test: cantp_test_strict (CanTp strict padding rejection)")
