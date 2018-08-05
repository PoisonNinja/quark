separate_arguments(CMAKE_CXX_FLAGS_LIST NATIVE_COMMAND ${CMAKE_CXX_FLAGS})

if (ARCH STREQUAL "x86_64")
    list(APPEND CMAKE_CXX_FLAGS_LIST -DX86_64)
endif()

add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/quark.lds
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/quark.lds
    COMMAND ${CMAKE_CXX_COMPILER}
        ${CMAKE_CXX_FLAGS_LIST}
    -x c++
    -E ${CMAKE_CURRENT_SOURCE_DIR}/quark.lds -P
    -o ${CMAKE_BINARY_DIR}/quark.lds
)

add_custom_target(
    lds
    DEPENDS ${CMAKE_BINARY_DIR}/quark.lds
)

add_dependencies(quark.kernel lds)
