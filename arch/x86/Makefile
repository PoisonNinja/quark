ifeq ($(ARCH),x86_64)
	CXXFLAGS += -DX86_64 -mcmodel=kernel -mno-red-zone
	LDFLAGS += -zmax-page-size=0x1000
	ASMFLAGS += -felf64 -d x86_64 -iarch/include/x86/asm/
	BITS := 64
else
	ASMFLAGS += -felf32
	BITS := 32
endif

export BITS

core-y := arch/$(SRCARCH)/