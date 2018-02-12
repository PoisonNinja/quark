target_sources(quark.kernel
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/descriptor.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/fs.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/inode.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/vnode.cc
)
