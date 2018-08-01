target_sources(quark.kernel
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/bootstrap32.asm
    ${CMAKE_CURRENT_SOURCE_DIR}/bootstrap64.asm
    ${CMAKE_CURRENT_SOURCE_DIR}/setup.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/stacktrace.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/symbol.cc
)

add_subdirectory(time)