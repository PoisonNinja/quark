# add_definitions(
#     -DX86
# )

if (ARCH STREQUAL "x86_64")
    add_definitions(
        -DX86_64
    )
endif()

if (ARCH STREQUAL "x86_64")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-omit-frame-pointer -mcmodel=large -mno-red-zone")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
endif()

if (ARCH STREQUAL "x86_64")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -zmax-page-size=0x1000")
endif()
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -T${CMAKE_BINARY_DIR}/quark.lds")

if (ARCH STREQUAL "x86_64")
    set(CMAKE_ASM_NASM_COMPILE_OBJECT "${CMAKE_ASM_NASM_COMPILE_OBJECT} -d X86_64 -felf64 -i${CMAKE_SOURCE_DIR}/arch/include/x86/asm/")
else()
    set(CMAKE_ASM_NASM_COMPILE_OBJECT "${CMAKE_ASM_NASM_COMPILE_OBJECT} -felf32")
endif()