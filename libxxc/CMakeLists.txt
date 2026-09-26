file(GLOB C_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.ppc")

set_source_files_properties(
    ${C_SOURCES}
    PROPERTIES LANGUAGE C
)

add_library(xxc STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/app.ppc
    ${CMAKE_CURRENT_SOURCE_DIR}/include/app.pph

    ${CMAKE_CURRENT_SOURCE_DIR}/src/argparse.ppc
    ${CMAKE_CURRENT_SOURCE_DIR}/include/argparse.pph

    ${CMAKE_CURRENT_SOURCE_DIR}/src/stream.ppc
    ${CMAKE_CURRENT_SOURCE_DIR}/include/stream.pph

    ${CMAKE_CURRENT_SOURCE_DIR}/src/fio.ppc
    ${CMAKE_CURRENT_SOURCE_DIR}/include/fio.pph

    ${CMAKE_CURRENT_SOURCE_DIR}/src/fs_path.ppc
    ${CMAKE_CURRENT_SOURCE_DIR}/include/fs_path.pph

    ${CMAKE_CURRENT_SOURCE_DIR}/src/mio.ppc
    ${CMAKE_CURRENT_SOURCE_DIR}/include/mio.pph
)

target_link_libraries(xxc PUBLIC minicrt)

target_include_directories(xxc PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)