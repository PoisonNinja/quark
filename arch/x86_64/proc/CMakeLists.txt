target_sources(quark.kernel
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/elf.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/signal.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/thread.cc
)
