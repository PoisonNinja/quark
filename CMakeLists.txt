# Required for target_sources
cmake_minimum_required(VERSION 3.1)

if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR "In-source builds are not allowed.")
endif("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")

set(CMAKE_SYSTEM_NAME       Generic)
set(CMAKE_SYSTEM_VERSION    1)

find_program(CLANGCXX clang++)
find_program(LD.LLD ld.lld)

set(CMAKE_CXX_COMPILER ${CLANGCXX})
set(CMAKE_CXX_LINKER ${LD.LLD})

enable_language(ASM_NASM)

set(ARCH "x86_64" CACHE STRING "Architecture to build for")
message(STATUS "Architecture: " ${ARCH})

set(ARCH_PATH "${CMAKE_SOURCE_DIR}/arch/${ARCH}")
if(NOT EXISTS ${ARCH_PATH})
    message(FATAL_ERROR "Unable to locate architecture directory. Perhaps your ARCH is wrong?")
endif()
message(STATUS "Architecture folder: " ${ARCH_PATH})

set(ARCH_INCLUDE_PATH "${CMAKE_SOURCE_DIR}/arch/include/${ARCH}")
if(NOT EXISTS ${ARCH_INCLUDE_PATH})
    message(FATAL_ERROR "Unable to locate architecture include directory. Perhaps your ARCH is wrong?")
endif()
message(STATUS "Architecture include folder: " ${ARCH_INCLUDE_PATH})

if(NOT DEFINED ${PLATFORM_FILE})
    set(PLATFORM_FILE ${ARCH_PATH}/platform.cmake)
endif()
if(NOT EXISTS ${PLATFORM_FILE})
    message(FATAL_ERROR "Platform file does not exist: " ${PLATFORM_FILE})
endif()
message(STATUS "Platform file: " ${PLATFORM_FILE})

if(NOT DEFINED SYSROOT)
    set(SYSROOT "${CMAKE_SOURCE_DIR}/../hdd")
endif()
message(STATUS "Sysroot: " ${SYSROOT})

project(Quark CXX)

include(${PLATFORM_FILE})

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -ffreestanding -fno-builtin -fno-rtti -fno-exceptions -fcolor-diagnostics -O3")
message(STATUS "CXXFLAGS: " ${CMAKE_CXX_FLAGS})

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -nostdlib")
message(STATUS "LDFLAGS: " ${CMAKE_EXE_LINKER_FLAGS})

set(INCLUDE_DIRS "${INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/include/")
include_directories(${INCLUDE_DIRS})
message(STATUS "Include directories: " ${INCLUDE_DIRS})

set(CMAKE_CXX_LINK_EXECUTABLE "${CMAKE_CXX_LINKER} <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET>")

set(CREATE_ARCH_SYMLINK ${CMAKE_COMMAND} -E create_symlink ${ARCH_INCLUDE_PATH} ${CMAKE_SOURCE_DIR}/include/arch)
execute_process(
    COMMAND ${CREATE_ARCH_SYMLINK}
)

add_executable(quark.kernel "")
add_subdirectory(${ARCH_PATH})
add_subdirectory(boot)
add_subdirectory(kernel)
add_subdirectory(lib)

install(TARGETS quark.kernel DESTINATION "${SYSROOT}/boot")
