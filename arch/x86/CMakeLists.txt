separate_arguments(CMAKE_CXX_FLAGS_LIST NATIVE_COMMAND ${CMAKE_CXX_FLAGS})

if (ARCH STREQUAL "x86_64")
    list(APPEND CMAKE_CXX_FLAGS_LIST -DX86_64)
    list(APPEND CMAKE_ASM_FLAGS_LIST -felf64 -d X86_64)
else()
    list(APPEND CMAKE_ASM_FLAGS_LIST -felf32)
endif()

add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/quark.lds
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/quark.lds
    COMMAND ${CMAKE_CXX_COMPILER}
        ${CMAKE_CXX_FLAGS_LIST}
    -x c++
    -E ${CMAKE_CURRENT_SOURCE_DIR}/quark.lds -P
    -o ${CMAKE_BINARY_DIR}/quark.lds
    COMMENT "Generating preprocessed linker script"
)

# Generate crti and crtn
add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/crti.o
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/crti.asm
    COMMAND nasm
    ${CMAKE_CURRENT_SOURCE_DIR}/crti.asm
    ${CMAKE_ASM_FLAGS_LIST}
    -o ${CMAKE_BINARY_DIR}/crti.o
    COMMENT "Generating crti.o"
)

add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/crtn.o
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/crtn.asm
    COMMAND nasm
    ${CMAKE_CURRENT_SOURCE_DIR}/crtn.asm
    ${CMAKE_ASM_FLAGS_LIST}
    -o ${CMAKE_BINARY_DIR}/crtn.o
    COMMENT "Generating crtn.o"
)

add_custom_target(
    lds
    DEPENDS ${CMAKE_BINARY_DIR}/quark.lds
)

add_custom_target(
    crti
    DEPENDS ${CMAKE_BINARY_DIR}/crti.o
)

add_custom_target(
    crtn
    DEPENDS ${CMAKE_BINARY_DIR}/crtn.o
)

add_dependencies(quark.kernel lds crti crtn)

add_subdirectory(cpu)
add_subdirectory(kernel)
add_subdirectory(lib)
add_subdirectory(mm)
add_subdirectory(proc)
