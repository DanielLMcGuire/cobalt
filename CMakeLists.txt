cmake_minimum_required(VERSION 3.25)

project(plusplusc
    VERSION 0.1.0
    DESCRIPTION "++C"
    LANGUAGES C ASM
)

if (NOT (WIN32 OR LINUX))
    message(FATAL_ERROR "++C only supports Linux and Windows targets (got: ${CMAKE_SYSTEM_NAME})")
endif()

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

option(XXC_BUILD_DEMO "Build the ++C demo executable" ${PROJECT_IS_TOP_LEVEL})
option(XXC_BUILD_TEST "Build the ++C test executable" ${PROJECT_IS_TOP_LEVEL})

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/CompileOptions.cmake")

add_subdirectory(platform)
add_subdirectory(libminicrt)
add_subdirectory(libxxc)

if (XXC_BUILD_DEMO)
    set_source_files_properties(
        "${CMAKE_CURRENT_SOURCE_DIR}/examples/app-demo.ppc"
        "${CMAKE_CURRENT_SOURCE_DIR}/examples/socket-demo.ppc"
        PROPERTIES LANGUAGE C
    )

    add_executable(xxc_demo_app "${CMAKE_CURRENT_SOURCE_DIR}/examples/app-demo.ppc")
    set_target_properties(xxc_demo_app PROPERTIES OUTPUT_NAME app)
    target_link_libraries(xxc_demo_app PRIVATE xxc::xxc)

    xxc_freestanding_compile_options(xxc_demo_app PRIVATE)
    xxc_freestanding_link_options(xxc_demo_app)

    add_executable(xxc_demo_socket "${CMAKE_CURRENT_SOURCE_DIR}/examples/socket-demo.ppc")
    set_target_properties(xxc_demo_socket PROPERTIES OUTPUT_NAME socket)
    target_link_libraries(xxc_demo_socket PRIVATE xxc::xxc)

    xxc_freestanding_compile_options(xxc_demo_socket PRIVATE)
    xxc_freestanding_link_options(xxc_demo_socket)
endif()

if (XXC_BUILD_TEST)
    set_source_files_properties("${CMAKE_CURRENT_SOURCE_DIR}/test/test.ppc" PROPERTIES LANGUAGE C)

    add_executable(xxc_test "${CMAKE_CURRENT_SOURCE_DIR}/test/test.ppc")
    set_target_properties(xxc_test PROPERTIES OUTPUT_NAME test)
    target_link_libraries(xxc_test PRIVATE xxc::xxc)

    xxc_freestanding_compile_options(xxc_test PRIVATE)
    xxc_freestanding_link_options(xxc_test)

    set_source_files_properties("${CMAKE_CURRENT_SOURCE_DIR}/test/leak.ppc" PROPERTIES LANGUAGE C)
    add_executable(xxc_leak_test "${CMAKE_CURRENT_SOURCE_DIR}/test/leak.ppc")
    set_target_properties(xxc_leak_test PROPERTIES OUTPUT_NAME leak_test)
    target_link_libraries(xxc_leak_test PRIVATE xxc::xxc)
    xxc_freestanding_compile_options(xxc_leak_test PRIVATE)
    xxc_freestanding_link_options(xxc_leak_test)
endif()