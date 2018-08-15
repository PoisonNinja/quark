# Version
QUARK_VERSION_MAJOR := 0
QUARK_VERSION_MINOR := 0
QUARK_VERSION_PATCH := 3
QUARK_VERSION_SUFFIX := -dev
QUARK_VERSION := $(QUARK_VERSION_MAJOR).$(QUARK_VERSION_MINOR).$(QUARK_VERSION_PATCH)$(QUARK_VERSION_SUFFIX)

ifeq ("$(origin V)", "command line")
  KBUILD_VERBOSE = $(V)
endif
ifndef KBUILD_VERBOSE
  KBUILD_VERBOSE = 0
endif

ifeq ($(KBUILD_VERBOSE),1)
  quiet =
  Q =
else
  quiet=quiet_
  Q = @
endif

export quiet Q KBUILD_VERBOSE

ARCH ?= x86_64
SRCARCH := $(ARCH)

ifeq ($(ARCH),i686)
	SRCARCH := x86
else ifeq ($(ARCH),x86_64)
	SRCARCH := x86
endif

ARCH_PATH := arch/$(SRCARCH)
ARCH_INCLUDE := arch/include/$(SRCARCH)

# Set compilers/tools
CXX := $(ARCH)-pepper-g++
ASM := nasm
LD := $(ARCH)-pepper-ld
AR := $(ARCH)-pepper-ar

# Set basic flags
CXXFLAGS += -std=gnu++11 -I include/ -Wall -Wextra -Wno-missing-field-initializers -ffreestanding -fno-builtin -fno-rtti -fno-exceptions -fno-stack-protector -fdiagnostics-color=always
LDFLAGS += -nostdlib 
ARFLAGS += D
ASMFLAGS += -F dwarf

# Figure out where we are
srctree := .
SRC := $(srctree)
OBJ := $(srctree)

# Silence make from printing "Entering directory"
MAKEFLAGS += -rR --no-print-directory

# Export some basic stuff
export ARCH SRCARCH ARCH_PATH ARCH_INCLUDE
export CXX ASM LD AR
export CXXFLAGS LDFLAGS ARFLAGS ASMFLAGS
export srctree

# Let's bring in the common includes
include tools/Makefile.include

# Load the architecture files
include arch/$(SRCARCH)/Makefile

# Top level directories
core-y += cpu/ drivers/ fs/ kernel/ lib/ mm/ proc/ 
dirs := $(patsubst %/,%, $(core-y))
core-y := $(patsubst %/, %/built-in.o, $(core-y))

deps := $(core-y)

all: quark.kernel

quark.kernel: $(deps)
	@$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) -lgcc

$(sort $(deps)): $(dirs) ;

PHONY += $(dirs)
$(dirs):
	@$(MAKE) $(build)=$@

.PHONY: $(PHONY)