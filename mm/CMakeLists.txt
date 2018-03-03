target_sources(quark.kernel
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/heap.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/mm.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/physical.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/valloc.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/virtual.cc
)
