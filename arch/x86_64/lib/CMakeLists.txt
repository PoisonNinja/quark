target_sources(quark.kernel
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/memcpy.S
    ${CMAKE_CURRENT_SOURCE_DIR}/memset.S
)
