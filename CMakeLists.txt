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

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/CompileOptions.cmake")

add_subdirectory(platform)
add_subdirectory(libminicrt)
add_subdirectory(libxxc)

if (XXC_BUILD_DEMO)
    set_source_files_properties(main.ppc PROPERTIES LANGUAGE C)

    add_executable(xxc_demo main.ppc)
    set_target_properties(xxc_demo PROPERTIES OUTPUT_NAME main)
    target_link_libraries(xxc_demo PRIVATE xxc::xxc)

    xxc_freestanding_compile_options(xxc_demo PRIVATE)
    xxc_freestanding_link_options(xxc_demo)
endif()