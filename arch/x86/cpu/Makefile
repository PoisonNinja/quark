obj-y := \
	cpu.o \
	feature.o \
	gdt.o \
	idt.o \
	interrupt.o \
	trampoline.o

obj-y += \
	signal.o \
	tables.o

ifeq ($(ARCH),x86_64)
	obj-y += syscall.o
endif