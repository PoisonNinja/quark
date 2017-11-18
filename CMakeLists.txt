# Required for target_sources
cmake_minimum_required(VERSION 3.1)

if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR "In-source builds are not allowed.")
endif("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")

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

if(NOT DEFINED ${CMAKE_TOOLCHAIN_FILE})
    set(CMAKE_TOOLCHAIN_FILE ${ARCH_PATH}/toolchain.cmake)
endif()
if(NOT EXISTS ${CMAKE_TOOLCHAIN_FILE})
    message(FATAL_ERROR "Toolchain file does not exist: " ${CMAKE_TOOLCHAIN_FILE})
endif()
message(STATUS "Toolchain file: " ${CMAKE_TOOLCHAIN_FILE})

if(NOT DEFINED TOOLCHAIN_PREFIX)
    if(EXISTS ${CMAKE_SOURCE_DIR}/../toolchain/local/)
        set(TOOLCHAIN_PREFIX ${CMAKE_SOURCE_DIR}/../toolchain/local)
    else()
        message(FATAL_ERROR "TOOLCHAIN_PREFIX is not defined. Unable to locate toolchain")
    endif()
else(DEFINED TOOLCHAIN_PREFIX)
    if(NOT EXISTS ${TOOLCHAIN_PREFIX})
        message(FATAL_ERROR "TOOLCHAIN_PREFIX refers to an invalid path. Unable to locate toolchain")
    endif()
endif()
message(STATUS "Toolchain prefix: " ${TOOLCHAIN_PREFIX})

enable_language(ASM_NASM)

project(Quark C)

if(NOT DEFINED ${PLATFORM_FILE})
    set(PLATFORM_FILE ${ARCH_PATH}/platform.cmake)
endif()
if(NOT EXISTS ${PLATFORM_FILE})
    message(FATAL_ERROR "Platform file does not exist: " ${PLATFORM_FILE})
endif()
message(STATUS "Platform file: " ${PLATFORM_FILE})
include(${PLATFORM_FILE})

add_executable(quark.kernel "")
add_subdirectory(${ARCH_PATH})
add_subdirectory(kernel)
add_subdirectory(lib)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -ffreestanding -fno-builtin -Iinclude -std=gnu11 -fcolor-diagnostics -O3")
message(STATUS "CFLAGS: " ${CMAKE_C_FLAGS})

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -nostdlib")
message(STATUS "LDFLAGS: " ${CMAKE_EXE_LINKER_FLAGS})

set(INCLUDE_DIRS "${INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/include/")
include_directories(${INCLUDE_DIRS})
message(STATUS "Include directories: " ${INCLUDE_DIRS})

if(NOT DEFINED SYSROOT)
    set(SYSROOT "${CMAKE_SOURCE_DIR}/../hdd")
endif()
message(STATUS "Sysroot: " ${SYSROOT})

install(TARGETS quark.kernel RUNTIME DESTINATION "${SYSROOT}/boot")
