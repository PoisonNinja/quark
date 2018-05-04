target_sources(quark.kernel
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/process.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/sched.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/syscall.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/thread.cc
)
