# Required for target_sources
cmake_minimum_required(VERSION 3.1)

set(QUARK_VERSION_MAJOR "0")
set(QUARK_VERSION_MINOR "0")
set(QUARK_VERSION_PATCH "1")
set(QUARK_VERSION_SUFFIX "-rc1")
set(QUARK_VERSION "${QUARK_VERSION_MAJOR}.${QUARK_VERSION_MINOR}.${QUARK_VERSION_PATCH}${QUARK_VERSION_SUFFIX}")

if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR "In-source builds are not allowed.")
endif("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")

set(CMAKE_SYSTEM_NAME       Generic)
set(CMAKE_SYSTEM_VERSION    1)

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

find_program(CXX ${ARCH}-pepper-g++)
if(NOT CXX)
    message(FATAL_ERROR "Compiler not found! Run the \
                         toolchain script in Pepper to build a toolchain. If \
                         you have already built it, make sure that you have run \
                         envsetup.sh before configuring CMake.")
endif()

# Skip compiler checks since we're targeting ELF binaries, and thus those
# would fail on MacOS with the error `file was built for unsupported file format`,
# which makes sense since MacOS uses MachO. Unfortunately, the way that CMake
# does the compiler checks means that it performs the compiler checks without
# letting us inject flags to force Clang to generate ELF binaries
set(CMAKE_CXX_COMPILER_WORKS 1)
set(CMAKE_CXX_COMPILER ${CXX})

message(STATUS "CXX Compiler: " ${CXX})

project(Quark CXX)

include(${PLATFORM_FILE})

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++17 -Wall -Wextra -ffreestanding -fno-builtin -fno-rtti -fno-exceptions -fno-stack-protector -fdiagnostics-color=always")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -DQUARK_DEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DQUARK_RELEASE")
message(STATUS "CXXFLAGS: " ${CMAKE_CXX_FLAGS})

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -nostdlib")
message(STATUS "LDFLAGS: " ${CMAKE_EXE_LINKER_FLAGS})

set(INCLUDE_DIRS "${INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/include/")
include_directories(${INCLUDE_DIRS})
message(STATUS "Include directories: " ${INCLUDE_DIRS})

set(CREATE_ARCH_SYMLINK ${CMAKE_COMMAND} -E create_symlink ${ARCH_INCLUDE_PATH} ${CMAKE_SOURCE_DIR}/include/arch)
execute_process(
    COMMAND ${CREATE_ARCH_SYMLINK}
)

add_custom_command(
    OUTPUT ${CMAKE_SOURCE_DIR}/include/kernel/version.h
    COMMAND ${CMAKE_SOURCE_DIR}/tools/version.sh
        ${QUARK_VERSION}
        ${ARCH}
        ${CMAKE_CXX_COMPILER}
    COMMENT "Generating version header"
)

add_executable(quark.kernel ${CMAKE_SOURCE_DIR}/include/kernel/version.h)
add_subdirectory(${ARCH_PATH})
add_subdirectory(cpu)
add_subdirectory(drivers)
add_subdirectory(fs)
add_subdirectory(kernel)
add_subdirectory(lib)
add_subdirectory(mm)
add_subdirectory(proc)

install(TARGETS quark.kernel DESTINATION "${SYSROOT}/boot")
