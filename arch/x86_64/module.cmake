add_definitions(
    -DX86_64
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -zmax-page-size=0x1000 -mcmodel=large -mno-red-zone")

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static -Wl,-r,-T ${ARCH_PATH}/module.lds")

set(CMAKE_ASM_NASM_FLAGS "${CMAKE_ASM_NASM_FLAGS} -F dwarf -d X86_64 -i${CMAKE_SOURCE_DIR}/arch/include/x86/asm/")
set(CMAKE_ASM_NASM_OBJECT_FORMAT elf64)
