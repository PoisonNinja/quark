target_sources(quark.kernel
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/intel-8253.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/tsc.cc
)
