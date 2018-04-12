target_sources(quark.kernel
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/gdt.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/idt.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/interrupt.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/syscall.asm
    ${CMAKE_CURRENT_SOURCE_DIR}/tables.asm
)
