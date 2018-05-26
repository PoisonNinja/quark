target_sources(quark.kernel
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/bootstrap.asm
    ${CMAKE_CURRENT_SOURCE_DIR}/setup.cc
)

add_subdirectory(time)
