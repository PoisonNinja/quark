target_sources(quark.kernel
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/bootstrap32.asm
    ${CMAKE_CURRENT_SOURCE_DIR}/bootstrap64.asm
    ${CMAKE_CURRENT_SOURCE_DIR}/x86_64_init.c
)
