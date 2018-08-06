target_sources(quark.kernel
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/module.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/bootstrap32.asm
    ${CMAKE_CURRENT_SOURCE_DIR}/setup.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/stacktrace.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/symbol.cc
)

if(ARCH STREQUAL "x86_64")
    target_sources(quark.kernel
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/bootstrap64.asm
    )
endif()


add_subdirectory(time)
