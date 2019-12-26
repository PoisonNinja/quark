target_sources(quark.kernel
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/cpu.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/feature.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/gdt.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/idt.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/interrupt.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/trampoline.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/signal.asm
    ${CMAKE_CURRENT_SOURCE_DIR}/tables.asm
    ${CMAKE_CURRENT_SOURCE_DIR}/task.asm
)

if (ARCH STREQUAL "x86_64")
    target_sources(quark.kernel
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/syscall.asm
    )
endif()
