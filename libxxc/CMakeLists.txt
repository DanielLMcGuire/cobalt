if (NOT TARGET xxc)
    set(XXC_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/src/App.ppc
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Argparse.ppc
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Stream.ppc
        ${CMAKE_CURRENT_SOURCE_DIR}/src/FileStream.ppc
        ${CMAKE_CURRENT_SOURCE_DIR}/src/FSPath.ppc
        ${CMAKE_CURRENT_SOURCE_DIR}/src/MemoryStream.ppc
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Socket.ppc
    )
    set_source_files_properties(${XXC_SOURCES} PROPERTIES LANGUAGE C)

    add_library(xxc STATIC
        ${XXC_SOURCES}

        ${CMAKE_CURRENT_SOURCE_DIR}/include/app.pph
        ${CMAKE_CURRENT_SOURCE_DIR}/include/argparse.pph
        ${CMAKE_CURRENT_SOURCE_DIR}/include/stream.pph
        ${CMAKE_CURRENT_SOURCE_DIR}/include/files.pph
        ${CMAKE_CURRENT_SOURCE_DIR}/include/fs.pph
        ${CMAKE_CURRENT_SOURCE_DIR}/include/mems.pph
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