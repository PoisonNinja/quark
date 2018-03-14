target_sources(quark.kernel
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/fork.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/init.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/map.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/update.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/virtual.cc
)
