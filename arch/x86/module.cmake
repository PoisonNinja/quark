add_definitions(
    -DX86
    -DX86_64
)

if (ARCH STREQUAL "x86_64")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-omit-frame-pointer -mcmodel=large -mno-red-zone")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
endif()

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static -Wl,-r,-T ${ARCH_PATH}/module.lds")
if (ARCH STREQUAL "x86_64")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -zmax-page-size=0x1000")
endif()

if (ARCH STREQUAL "x86_64")
    set(CMAKE_ASM_NASM_COMPILE_OBJECT "${CMAKE_ASM_NASM_COMPILE_OBJECT} -d X86_64 -felf64 -i${CMAKE_SOURCE_DIR}/arch/include/x86/asm/")
else()
    set(CMAKE_ASM_NASM_COMPILE_OBJECT "${CMAKE_ASM_NASM_COMPILE_OBJECT} -felf32")
endif()
