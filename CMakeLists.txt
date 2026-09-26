cmake_minimum_required(VERSION 3.25)

project(plusplusc LANGUAGES C ASM)
set(CMAKE_C_STANDARD 23)

add_subdirectory("libminicrt")
add_subdirectory("libxxc")

set_source_files_properties(main.ppc
    PROPERTIES LANGUAGE C
)
add_executable(min main.ppc)
target_link_libraries(min PRIVATE xxc)

if (WIN32)
    target_link_libraries(min PRIVATE kernel32)

    if (CMAKE_C_COMPILER_ID MATCHES "Clang|GNU")
        target_compile_options(min PRIVATE
            -O3
            -ffreestanding
            -fno-builtin
            -fno-stack-protector
            -mno-stack-arg-probe
        )
        target_link_options(min PRIVATE
            -O3
            -nostdlib
            "LINKER:/ENTRY:start"
            "LINKER:/SUBSYSTEM:CONSOLE"
        )
    elseif (MSVC)
        target_compile_options(min PRIVATE
            /O2
            /GS-
            /Gs9999999
            /guard:cf-
            /d2CastGuard-
        )
        target_link_options(min PRIVATE
            /OPT:REF
            /OPT:ICF
            /GS-
            /guard:cf-
            /d2CastGuard-
            /sdl-
            /RTCc- /RTCs- /RTCu- /RTC1-
            /Zl
            /Gs9999999
            "LINKER:/ENTRY:start"
            "LINKER:/SUBSYSTEM:CONSOLE"
        )
    endif()

elseif (LINUX)
    target_compile_options(min PRIVATE
        -O3
        -flto
        -fno-stack-protector
        -ffreestanding
        -fno-builtin
    )
    target_link_options(min PRIVATE
        -O3
        -flto
        -nostdlib
        -fno-builtin
        -static
        -no-pie
    )
endif()