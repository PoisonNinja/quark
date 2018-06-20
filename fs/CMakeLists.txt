target_sources(quark.kernel
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/descriptor.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/dtable.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/fs.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/inode.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/pty.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/tty.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/vnode.cc
)

add_subdirectory(pty)
add_subdirectory(initfs)
add_subdirectory(initrd)
