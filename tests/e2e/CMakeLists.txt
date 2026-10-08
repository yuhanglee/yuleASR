# E2E Tests CMakeLists.txt
cmake_minimum_required(VERSION 3.16)

# E2E test executables
set(E2E_TEST_SOURCES
    test_e2e_can_communication.c
    test_e2e_diagnostic_stack.c
    test_e2e_nvm_stack.c
    test_e2e_dds_communication.c
    test_e2e_watchdog.c
)

foreach(test_src ${E2E_TEST_SOURCES})
    get_filename_component(test_name ${test_src} NAME_WE)
    add_executable(${test_name} ${test_src})
    target_link_libraries(${test_name} m)
    add_test(NAME ${test_name} COMMAND ${test_name})
endforeach()

# P1-4: CI-required E2E test binaries (build/bin/test_e2e_{dds,secoc,e2e,uds,ota})
set(E2E_CI_TEST_SOURCES
    test_e2e_dds.c
    test_e2e_secoc.c
    test_e2e_e2e.c
    test_e2e_uds.c
    test_e2e_ota.c
)

foreach(test_src ${E2E_CI_TEST_SOURCES})
    get_filename_component(test_name ${test_src} NAME_WE)
    add_executable(${test_name} ${test_src})
    set_target_properties(${test_name} PROPERTIES
        C_STANDARD 99
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )
    add_test(NAME ${test_name} COMMAND ${test_name})
endforeach()
