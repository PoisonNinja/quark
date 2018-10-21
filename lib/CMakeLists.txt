target_sources(quark.kernel
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/bitset.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/libcxx.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/murmur.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/printf.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/string.cc
)

add_subdirectory(libstdc++)