ENTRY(bootstrap32)
OUTPUT_FORMAT("elf64-x86-64")
SECTIONS
{
	. = 0x100000;

	.bootstrap :
	{
		*(.bootstrap)
		*(.trampoline)
	}

	. += 0xFFFFFFFF80000000;

    __kernel_start = . - 0xFFFFFFFF80000000;

    .text ALIGN (0x1000) : AT(ADDR(.text) - 0xFFFFFFFF80000000)
    {
    	*(.text)
	}

    .rodata ALIGN (0x1000) : AT(ADDR(.rodata) - 0xFFFFFFFF80000000)
    {
        __constructors_start = .;
        *(SORT(.ctors*))
        __constructors_end = .;
        *(.rodata)
    }

    .data ALIGN (0x1000) : AT(ADDR(.data) - 0xFFFFFFFF80000000)
    {
        *(.data)
    }

	.eh_frame ALIGN(0x1000) : AT(ADDR(.eh_frame) - 0xFFFFFFFF80000000)
   	{
   	    *(.eh_frame)
   	}

    .bss ALIGN(0x1000) : AT(ADDR(.bss) - 0xFFFFFFFF80000000)
    {
        *(.bss)
        *(COMMON)
    }

    .initcall ALIGN(0x1000) : AT(ADDR(.initcall) - 0xFFFFFFFF80000000)
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

    __kernel_end = . - 0xFFFFFFFF80000000;
}
