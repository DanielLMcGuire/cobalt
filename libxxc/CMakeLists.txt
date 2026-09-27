if (NOT TARGET xxc)
    set(XXC_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/src/app.ppc
        ${CMAKE_CURRENT_SOURCE_DIR}/src/argparse.ppc
        ${CMAKE_CURRENT_SOURCE_DIR}/src/stream.ppc
        ${CMAKE_CURRENT_SOURCE_DIR}/src/fio.ppc
        ${CMAKE_CURRENT_SOURCE_DIR}/src/fs_path.ppc
        ${CMAKE_CURRENT_SOURCE_DIR}/src/mio.ppc
        ${CMAKE_CURRENT_SOURCE_DIR}/src/socket.ppc
    )
    set_source_files_properties(${XXC_SOURCES} PROPERTIES LANGUAGE C)

    add_library(xxc STATIC
        ${XXC_SOURCES}

        ${CMAKE_CURRENT_SOURCE_DIR}/include/app.pph
        ${CMAKE_CURRENT_SOURCE_DIR}/include/argparse.pph
        ${CMAKE_CURRENT_SOURCE_DIR}/include/stream.pph
        ${CMAKE_CURRENT_SOURCE_DIR}/include/fio.pph
        ${CMAKE_CURRENT_SOURCE_DIR}/include/fs_path.pph
        ${CMAKE_CURRENT_SOURCE_DIR}/include/mio.pph
    )
    add_library(xxc::xxc ALIAS xxc)

    target_link_libraries(xxc PUBLIC xxc::minicrt)

    target_include_directories(xxc PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    )

    if (CMAKE_C_COMPILER_ID MATCHES "Clang|GNU")
        if (NOT WIN32 OR MINGW)
            target_link_options(xxc INTERFACE
                "LINKER:--start-group,$<TARGET_FILE:xxc>,$<TARGET_FILE:minicrt>,--end-group"
            )
        endif()
    endif()
endif()