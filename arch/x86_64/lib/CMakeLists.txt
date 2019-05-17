if(ARCH STREQUAL "x86_64")
    target_sources(quark.kernel
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/memcpy_64.asm
        ${CMAKE_CURRENT_SOURCE_DIR}/memset_64.asm
    )
else()
    target_sources(quark.kernel
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/memcpy_32.asm
        ${CMAKE_CURRENT_SOURCE_DIR}/memset_32.asm
    )
endif()
