if (NOT TARGET xxc)
    set(XXC_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/src/App.ppc
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Argparse.ppc
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Stream.ppc
        ${CMAKE_CURRENT_SOURCE_DIR}/src/FileStream.ppc
        ${CMAKE_CURRENT_SOURCE_DIR}/src/FSPath.ppc
        ${CMAKE_CURRENT_SOURCE_DIR}/src/MemoryStream.ppc
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Socket.ppc
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Map.ppc
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Runnable.ppc
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Thread.ppc
    )
    set_source_files_properties(${XXC_SOURCES} PROPERTIES LANGUAGE C)

    add_library(xxc STATIC
        ${XXC_SOURCES}
     ${CMAKE_CURRENT_SOURCE_DIR}/include/class.pph
        ${CMAKE_CURRENT_SOURCE_DIR}/include/app.pph
        ${CMAKE_CURRENT_SOURCE_DIR}/include/argparse.pph
        ${CMAKE_CURRENT_SOURCE_DIR}/include/stream.pph
        ${CMAKE_CURRENT_SOURCE_DIR}/include/files.pph
        ${CMAKE_CURRENT_SOURCE_DIR}/include/fs.pph
        ${CMAKE_CURRENT_SOURCE_DIR}/include/mems.pph
        ${CMAKE_CURRENT_SOURCE_DIR}/include/map.pph
        ${CMAKE_CURRENT_SOURCE_DIR}/include/runnable.pph
        ${CMAKE_CURRENT_SOURCE_DIR}/include/thread.pph
    )
    add_library(xxc::xxc ALIAS xxc)

    target_link_libraries(xxc PUBLIC xxc::minicrt)

    target_include_directories(xxc PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    )
endif()