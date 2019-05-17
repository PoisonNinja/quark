if (ARCH STREQUAL "x86_64")
    add_definitions(
        -DX86_64
    )
endif()

if (ARCH STREQUAL "x86_64")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mcmodel=large -mno-red-zone")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
endif()

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static -Wl,-r,-T ${ARCH_PATH}/module.lds")
if (ARCH STREQUAL "x86_64")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -zmax-page-size=0x1000")
endif()

set(CMAKE_ASM_NASM_FLAGS "${CMAKE_ASM_NASM_FLAGS} -F dwarf")
if (ARCH STREQUAL "x86_64")
    set(CMAKE_ASM_NASM_FLAGS "${CMAKE_ASM_NASM_FLAGS} -d X86_64 -i${CMAKE_SOURCE_DIR}/arch/include/x86/asm/")
    set(CMAKE_ASM_NASM_OBJECT_FORMAT elf64)
else()
    set(CMAKE_ASM_NASM_OBJECT_FORMAT elf)
endif()
