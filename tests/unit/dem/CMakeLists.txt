# Dem DTC test matrix (P1 Phase 8) — links the real service_dem library
# (Dem.c + Dem_Int.c + Dem_Cfg.c). The test file provides its own
# Det_ReportError recorder, so mock_det.c is deliberately NOT linked.

cmake_minimum_required(VERSION 3.16)

set(DEM_MATRIX_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/test_dem_dtc_matrix.c
)

add_executable(dem_dtc_matrix_test ${DEM_MATRIX_SRC})

target_include_directories(dem_dtc_matrix_test
    PRIVATE
        ${TEST_FRAMEWORK_INCLUDE}
        ${CMAKE_SOURCE_DIR}/src/bsw/services/dem/include
        ${CMAKE_SOURCE_DIR}/src/common/include
        ${CMAKE_SOURCE_DIR}/include/autosar
        ${CMAKE_SOURCE_DIR}/config/input/services
)

target_compile_options(dem_dtc_matrix_test PRIVATE
    -Wall
    -Wextra
    -g
    -O0
)

set_target_properties(dem_dtc_matrix_test PROPERTIES
    C_STANDARD 99
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests
)

if(TARGET service_dem)
    target_link_libraries(dem_dtc_matrix_test PRIVATE service_dem)
endif()

if(TEST_FRAMEWORK_LIB)
    target_link_libraries(dem_dtc_matrix_test PRIVATE ${TEST_FRAMEWORK_LIB})
endif()

add_test(NAME dem_dtc_matrix_test COMMAND dem_dtc_matrix_test)

message(STATUS "  Unit test: dem_dtc_matrix_test (DTC status/clear/freeze-frame/debounce matrix, P1 Phase 8)")
