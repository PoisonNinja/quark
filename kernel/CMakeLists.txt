set(CONFIG_KERNEL_LIB_LOG_PRINTK_MAX "1024" CACHE STRING "Set kernel printk buffer size" )

target_sources(quark.kernel
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/kmain.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/log.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/panic.cc
)

add_subdirectory(time)
