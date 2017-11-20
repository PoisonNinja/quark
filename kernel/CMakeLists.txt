target_sources(quark.kernel
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/console.c
    ${CMAKE_CURRENT_SOURCE_DIR}/kmain.c
    ${CMAKE_CURRENT_SOURCE_DIR}/printk.c
)
