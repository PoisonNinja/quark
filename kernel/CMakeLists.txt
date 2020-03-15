set(CONFIG_PRINTK_MAX "1024" CACHE STRING "Set kernel printk buffer size" )

target_sources(quark.kernel
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/assert.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/init.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/kmain.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/log.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/module.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/panic.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/stacktrace.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/symbol.cc
)

add_subdirectory(time)
