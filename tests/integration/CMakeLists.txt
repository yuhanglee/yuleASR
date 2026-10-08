# Integration Test CMakeLists.txt
# 集成测试构建配置
#
# 每个测试套件独立成可执行文件 (runner_single.c + 编译宏选择套件)：
#   integration_comm_tests / integration_diag_tests / integration_safety_tests / integration_mem_tests
# 独立链接避免多个套件的测试替身 (mock) 重复符号冲突
#   (如 test_communication_stack.c 与 test_diagnostic_stack.c 各自定义 PduR_Transmit)。
#
# Unity 框架引导统一收敛: 由上级 tests/CMakeLists.txt 单一来源提供
# (find_package(Unity) 或 bundled tests/unit/framework/unity.c → Unity 库 +
# TEST_FRAMEWORK_LIB/TEST_FRAMEWORK_INCLUDE 变量), 本目录不再自行 add_subdirectory
# (原 ../unity 死路径, 实际框架位于 ../unit/framework)。

cmake_minimum_required(VERSION 3.14)
project(autosar_integration_tests C)

# 共用头文件路径
set(INTEGRATION_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/common/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/bsw/services/com/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/bsw/services/pdur/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/bsw/ecual/canif/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/bsw/mcal/can/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/bsw/services/dcm/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/bsw/services/doip/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/bsw/services/cantsyn/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/bsw/services/stbm/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/bsw/mcal/eth/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/bsw/services/e2e/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/bsw/services/crc/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/bsw/os/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/bsw/services/nvm/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/bsw/ecual/fee/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/bsw/mcal/fls/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/bsw/ecual/ea/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/platform/s32k312/include
    ${TEST_FRAMEWORK_INCLUDE}
)

# 为每个套件创建独立可执行文件
function(add_integration_suite TARGET_NAME RUN_MACRO)
    add_executable(${TARGET_NAME} ${ARGN} runner_single.c)
    target_include_directories(${TARGET_NAME} PRIVATE ${INTEGRATION_INCLUDES})
    target_compile_definitions(${TARGET_NAME} PRIVATE ${RUN_MACRO})
    if(TEST_FRAMEWORK_LIB)
        target_link_libraries(${TARGET_NAME} PRIVATE ${TEST_FRAMEWORK_LIB})
    endif()
    add_test(NAME ${TARGET_NAME} COMMAND ${TARGET_NAME})
endfunction()

# 说明: E2E_P01.c 为生产源码 (src/bsw/services/e2e/src), 自包含、无硬件依赖,
#       仅安全栈测试链接, 对真实生产实现做 E2E 保护/校验验证。
add_integration_suite(integration_comm_tests RUN_COMM
    test_communication_stack.c)
add_integration_suite(integration_diag_tests RUN_DIAG
    test_diagnostic_stack.c)
add_integration_suite(integration_safety_tests RUN_SAFETY
    test_safety_stack.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/bsw/services/e2e/src/E2E_P01.c)
add_integration_suite(integration_mem_tests RUN_MEM
    test_memory_stack.c)

# ── P1 Phase 8: SecOC + Csm + KeyM full-chain integration test ────────────────
# Real production code on both sides of every interface: keys are provisioned
# through the real KeyM, mirrored into the real Csm keystore (mbedTLS
# HMAC-SHA256 backend) and consumed by the real SecOC Tx-auth/Rx-verify
# pipeline. Only PduR/Det/SchM are test doubles provided by the test file
# itself (self-contained, own main() — hence not an add_integration_suite).
add_executable(secoc_chain_test
    ${CMAKE_CURRENT_SOURCE_DIR}/test_secoc_chain.c
)

target_include_directories(secoc_chain_test PRIVATE
    ${TEST_FRAMEWORK_INCLUDE}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/bsw/services/secoc/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/bsw/services/csm/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/bsw/services/keym/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/common/include
    ${CMAKE_SOURCE_DIR}/include/autosar
)

target_compile_options(secoc_chain_test PRIVATE -Wall -Wextra -g -O0)

set_target_properties(secoc_chain_test PROPERTIES
    C_STANDARD 99
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests
)

target_link_libraries(secoc_chain_test PRIVATE
    service_secoc
    service_csm
    service_keym
)

if(TEST_FRAMEWORK_LIB)
    target_link_libraries(secoc_chain_test PRIVATE ${TEST_FRAMEWORK_LIB})
endif()

add_test(NAME secoc_chain_test COMMAND secoc_chain_test)

message(STATUS "  Integration test: secoc_chain_test (SecOC+Csm+KeyM full chain, P1 Phase 8)")

# ── P1-12: DoIP full-chain integration test ────────────────────────────────────
# Self-contained test with mock SoAd/EthIf/Eth layers validating the DoIP
# protocol flow: Vehicle Announcement + Diagnostic Message routing.
add_executable(test_doip_chain
    ${CMAKE_CURRENT_SOURCE_DIR}/test_doip_chain.c
)

target_include_directories(test_doip_chain PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/common/include
)

target_compile_options(test_doip_chain PRIVATE -Wall -Wextra -g -O0)

set_target_properties(test_doip_chain PROPERTIES
    C_STANDARD 99
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

add_test(NAME test_doip_chain COMMAND test_doip_chain)

message(STATUS "  Integration test: test_doip_chain (DoIP->SoAd->EthIf->Eth, P1-12)")
