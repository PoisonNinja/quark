add_definitions(
    -DX86
    -DX86_64
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mcmodel=kernel -mno-red-zone -zmax-page-size=0x1000 -T${CMAKE_SOURCE_DIR}/arch/x86_64/quark.lds")

# Be careful of what you pass in here. Input will go directly to LLD, so no Clang flags allowed
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ")

set(CMAKE_ASM_NASM_COMPILE_OBJECT "${CMAKE_ASM_NASM_COMPILE_OBJECT} -felf64")
