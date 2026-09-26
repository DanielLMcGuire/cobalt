add_subdirectory("src/platform")

file(GLOB C_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.ppc")

set_source_files_properties(
    ${C_SOURCES}
    PROPERTIES LANGUAGE C
)

add_library(_crt STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/crt1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/alloc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/atexit.c

    ${CMAKE_CURRENT_SOURCE_DIR}/src/str.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mem.c

    ${CMAKE_CURRENT_SOURCE_DIR}/src/sig.c

    ${CMAKE_CURRENT_SOURCE_DIR}/src/dstr.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/iarr.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/parr.c

    ${CMAKE_CURRENT_SOURCE_DIR}/src/format.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/sio.c

    ${CMAKE_CURRENT_SOURCE_DIR}/src/stream.ppc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/fio.ppc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/fs_path.ppc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mio.ppc

    ${CMAKE_CURRENT_SOURCE_DIR}/src/argparse.ppc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/app.ppc
)

add_library(plusplusc STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/stream.ppc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/fio.ppc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/fs_path.ppc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mio.ppc

    ${CMAKE_CURRENT_SOURCE_DIR}/src/argparse.ppc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/app.ppc
)

target_link_libraries(plusplusc PUBLIC _crt)

if(LINUX)
    target_link_libraries(_crt PRIVATE sys_linux)
elseif(WIN32)
    target_link_libraries(_crt PRIVATE sys_win32)
endif()

target_include_directories(_crt PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_compile_definitions(_crt PUBLIC
    _CORECRT_BUILD
)

if(WIN32)
    target_compile_definitions(_crt PUBLIC
        _CORECRT_BUILD
        WIN32_LEAN_AND_MEAN
    )

    if(MSVC)
        target_compile_options(_crt PUBLIC
            /O2
            /GL
            /GS-
            /guard:cf-
            /d2CastGuard-
            /sdl-
            /RTCc- /RTCs- /RTCu- /RTC1-
            /Zl
            /Gs9999999
        )
    elseif(CMAKE_C_COMPILER_ID MATCHES "Clang|GNU")
        target_compile_options(_crt PUBLIC
            -O3
            -flto
            -ffreestanding
            -fno-builtin
            -fno-stack-protector
            -mno-stack-arg-probe
        )
    endif()

elseif(LINUX)
    if(CMAKE_C_COMPILER_ID MATCHES "Clang|GNU")
        target_compile_options(_crt PUBLIC
            -O3
            -flto
            -ffreestanding
            -fno-builtin
            -fno-stack-protector
        )
    endif()
endif()