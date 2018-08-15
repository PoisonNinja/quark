obj-y := \
	module.o \
	setup.o \
	stacktrace.o \
	symbol.o

obj-y += \
	bootstrap32.o

obj-y += time/

ifeq ($(ARCH),x86_64)
	obj-y += bootstrap64.o
endif