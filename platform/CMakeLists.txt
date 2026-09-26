if (LINUX AND NOT TARGET sys_linux)
    if (CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64")
        set(ARCH_SRCS
            linux/crt0_x86_64.s
            linux/syscall_x86_64.s
        )
    elseif (CMAKE_SYSTEM_PROCESSOR MATCHES "i.86|x86")
        set(ARCH_SRCS
            linux/crt0_i386.s
            linux/syscall_i386.s
        )
    elseif (CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64")
        set(ARCH_SRCS
            linux/crt0_aarch64.s
            linux/syscall_aarch64.s
        )
    else()
        message(FATAL_ERROR "++C: unsupported architecture: ${CMAKE_SYSTEM_PROCESSOR}")
    endif()

    add_library(sys_linux OBJECT linux/syscall.c ${ARCH_SRCS} linux/alloc.c)

    target_include_directories(sys_linux PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    )

    target_include_directories(sys_linux PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../libminicrt/include
    )
elseif (WIN32 AND NOT TARGET sys_win32)
    add_library(sys_win32 OBJECT win32/crt0.c)

    target_include_directories(sys_win32 PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    )

    target_compile_definitions(sys_win32 PRIVATE
        _CORECRT_BUILD
        WIN32_LEAN_AND_MEAN
    )
endif()