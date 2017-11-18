INCLUDE(CMakeForceCompiler)

set(TOOLCHAIN_BIN_DIR ${TOOLCHAIN_PREFIX}/bin)
set(TOOLCHAIN_INCLUDE_DIR ${TOOLCHAIN_PREFIX}/${TARGET_TRIPLET}/include)
set(TOOLCHAIN_LIBRARY_DIR ${TOOLCHAIN_PREFIX}/${TARGET_TRIPLET}/lib)

set(CMAKE_SYSTEM_NAME       Generic)
set(CMAKE_SYSTEM_VERSION    1)

set(CMAKE_C_COMPILER ${TOOLCHAIN_BIN_DIR}/clang CACHE INTERNAL "C compiler")
set(CMAKE_C_LINKER ${TOOLCHAIN_BIN_DIR}/ld.lld CACHE INTERNAL "C linker")

set(CMAKE_COMPILER_IS_GNUCC     0)
set(CMAKE_C_COMPILER_ID         Clang)
set(CMAKE_C_COMPILER_ID_RUN     TRUE)
set(CMAKE_C_COMPILER_FORCED     TRUE)

# Override the default linking action by replacing <CMAKE_C_COMPILER> with ${CMAKE_C_LINKER}
# because using <CMAKE_C_COMPILER> in conjunction with <arch>-none-elf causes clang to try
# to use the system compiler, which is bad. Instead, we override it with LLD and strip
# out the CFLAGS (which will cause LLD to barf). Everything else is the same.
set(CMAKE_C_LINK_EXECUTABLE "${CMAKE_C_LINKER} <CMAKE_C_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")
