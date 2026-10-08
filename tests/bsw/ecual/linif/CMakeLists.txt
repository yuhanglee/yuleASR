# LinIf unit tests — substantiated against the production LinIf implementation.
# LinIf has no ECUAL library target, so LinIf.c and the link-time config
# LinIf_Lcfg.c are compiled directly into the test executable.
# The real ecual_lintrcv driver library is linked to verify the
# LinIf -> LinTrcv delegation paths (mirrors the lintrcv unit test).
# Det/Dio/EcuM are provided by the test itself, so mock_det.c is not linked.

cmake_minimum_required(VERSION 3.16)

add_executable(linif_test
    test_linif.c
    ${CMAKE_SOURCE_DIR}/src/bsw/ecual/linif/src/LinIf.c
    ${CMAKE_SOURCE_DIR}/src/bsw/ecual/linif/src/LinIf_Lcfg.c
)

target_include_directories(linif_test
    PRIVATE
        ${TEST_FRAMEWORK_INCLUDE}
        ${CMAKE_SOURCE_DIR}/src/bsw/ecual/linif/include
        ${CMAKE_SOURCE_DIR}/src/bsw/mcal/lin/include
        ${CMAKE_SOURCE_DIR}/src/bsw/ecual/lintrcv/include
        ${CMAKE_SOURCE_DIR}/src/bsw/mcal/dio/include
        ${CMAKE_SOURCE_DIR}/include/autosar
)

target_compile_options(linif_test PRIVATE
    -Wall
    -Wextra
    -g
    -O0
)

set_target_properties(linif_test PROPERTIES
    C_STANDARD 99
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests
)

if(TEST_FRAMEWORK_LIB)
    target_link_libraries(linif_test PRIVATE ${TEST_FRAMEWORK_LIB})
endif()

# Link the real LinTrcv driver library to exercise LinIf -> LinTrcv delegation
# (src/bsw/ecual is added before tests/, so the ecual_lintrcv target already exists).
if(TARGET ecual_lintrcv)
    target_link_libraries(linif_test PRIVATE ecual_lintrcv)
endif()

add_test(NAME linif_test COMMAND linif_test)

message(STATUS "  Unit test: linif_test (LIN Interface)")
