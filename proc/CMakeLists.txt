set(CONFIG_SCHED_PARANOID "0" CACHE STRING "Enable additional (slow) checks to
    ensure scheduler behavior is correct. Generally this is not what you want
    unless you suspect the scheduler is to blame since this slows down overall
    performance" )

target_sources(quark.kernel
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/process.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/ptable.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/sched.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/signal.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/syscall.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/thread.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/work.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/wq.cc
)

add_subdirectory(binfmt)
