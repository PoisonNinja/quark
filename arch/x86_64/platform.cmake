add_definitions(
    -DX86_64
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mcmodel=kernel -mno-red-zone -zmax-page-size=0x1000")

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -T${CMAKE_BINARY_DIR}/quark.lds")

set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} -F dwarf -d X86_64 -i${CMAKE_SOURCE_DIR}/arch/include/x86_64/asm/")
set(CMAKE_ASM_NASM_OBJECT_FORMAT elf64)
