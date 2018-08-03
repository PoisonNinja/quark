add_definitions(
    -DX86
    -DX86_64
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-omit-frame-pointer -mcmodel=large -mno-red-zone")

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static -Wl,-r,-T ${ARCH_PATH}/module.lds -zmax-page-size=0x1000")

set(CMAKE_ASM_NASM_COMPILE_OBJECT "${CMAKE_ASM_NASM_COMPILE_OBJECT} -felf64 -i${CMAKE_SOURCE_DIR}/arch/include/x86_64/asm/")
