# Security Tests CMakeLists.txt
cmake_minimum_required(VERSION 3.16)

# P1-4: CI-required security test binary (build/bin/test_security)
add_executable(test_security test_security.c)
set_target_properties(test_security PROPERTIES
    C_STANDARD 99
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
target_compile_options(test_security PRIVATE -Wall -Wextra -O2)
add_test(NAME test_security COMMAND test_security)
message(STATUS "  Security test: test_security (BSW security verification)")
