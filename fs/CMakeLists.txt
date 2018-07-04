target_sources(quark.kernel
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/block.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/descriptor.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/dev.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/dtable.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/fs.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/ftable.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/inode.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/tty.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/vcache.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/vnode.cc
)

add_subdirectory(pty)
add_subdirectory(initrd)
add_subdirectory(tmpfs)

