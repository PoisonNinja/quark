#ifdef X86_64
OUTPUT_FORMAT("elf64-x86-64")
VMA = 0xFFFFFFFF80000000;
#else
OUTPUT_FORMAT("elf32-i386")
VMA = 0xC0000000;
#endif
ENTRY(bootstrap32)
SECTIONS
{
	. = 0x100000;
    __kernel_start = .;

	.bootstrap :
	{
		*(.bootstrap)
		*(.trampoline)
	}

	. += VMA;

    .text ALIGN (0x1000) : AT(ADDR(.text) - VMA)
    {
    	*(.text)
	}

    .rodata ALIGN (0x1000) : AT(ADDR(.rodata) - VMA)
    {
        *(.rodata)
    }

    .data ALIGN (0x1000) : AT(ADDR(.data) - VMA)
    {
        *(.data)
    }

    .ctors ALIGN (0x1000) : AT(ADDR(.ctors) - VMA)
    {
        __constructors_start = .;
        *(SORT(.ctors*))
        __constructors_end = .;
    }

	.eh_frame ALIGN(0x1000) : AT(ADDR(.eh_frame) - VMA)
   	{
   	    *(.eh_frame)
   	}

    .bss ALIGN(0x1000) : AT(ADDR(.bss) - VMA)
    {
        *(.bss)
        *(COMMON)
    }

    .initcall ALIGN(0x1000) : AT(ADDR(.initcall) - VMA)
    {
        __initcall1_start = .;
        *(.initcall1)
        __initcall2_start = .;
        *(.initcall2)
        __initcall3_start = .;
        *(.initcall3)
        __initcall4_start = .;
        *(.initcall4)
        __initcall5_start = .;
        *(.initcall5)
        __initcall6_start = .;
        *(.initcall6)
        __initcall7_start = .;
        *(.initcall7)
        __initcall_end = .;
    }

    /DISCARD/ :
    {
       	*(.comment)
    }

    __kernel_end = . - VMA;
}
