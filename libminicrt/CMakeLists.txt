if (NOT TARGET minicrt)
    add_library(minicrt STATIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src/crt1.c

        ${CMAKE_CURRENT_SOURCE_DIR}/include/def.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/alloc.c
        ${CMAKE_CURRENT_SOURCE_DIR}/include/alloc.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/atexit.c
        ${CMAKE_CURRENT_SOURCE_DIR}/include/atexit.h

        ${CMAKE_CURRENT_SOURCE_DIR}/src/str.c
        ${CMAKE_CURRENT_SOURCE_DIR}/include/str.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/mem.c
        ${CMAKE_CURRENT_SOURCE_DIR}/include/mem.h

        ${CMAKE_CURRENT_SOURCE_DIR}/src/sig.c
        ${CMAKE_CURRENT_SOURCE_DIR}/include/sig.h

        ${CMAKE_CURRENT_SOURCE_DIR}/src/dstr.c
        ${CMAKE_CURRENT_SOURCE_DIR}/include/dstr.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/iarr.c
        ${CMAKE_CURRENT_SOURCE_DIR}/include/iarr.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/parr.c
        ${CMAKE_CURRENT_SOURCE_DIR}/include/parr.h

        ${CMAKE_CURRENT_SOURCE_DIR}/src/format.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/sio.c
        ${CMAKE_CURRENT_SOURCE_DIR}/include/sio.h
    )
    add_library(xxc::minicrt ALIAS minicrt)

    if (LINUX)
        target_link_libraries(minicrt PUBLIC sys_linux)
    elseif (WIN32)
        target_link_libraries(minicrt PUBLIC sys_win32)
    endif()

    target_include_directories(minicrt PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    )

    target_compile_definitions(minicrt PUBLIC
        _CORECRT_BUILD
    )

    if (WIN32)
        target_compile_definitions(minicrt PUBLIC
            WIN32_LEAN_AND_MEAN
        )
    endif()

    xxc_freestanding_compile_options(minicrt PUBLIC)
endif()