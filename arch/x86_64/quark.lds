ENTRY(bootstrap32)
OUTPUT_FORMAT("elf64-x86-64")
SECTIONS
{
	. = 0x100000;

    __kernel_start = .;

	.bootstrap :
	{
		*(.bootstrap)
		*(.trampoline)
	}

	. += 0xFFFFFFFF80000000;

    .text ALIGN (0x1000) : AT(ADDR(.text) - 0xFFFFFFFF80000000)
    {
    	*(.text)
	}

    .data ALIGN (0x1000) : AT(ADDR(.data) - 0xFFFFFFFF80000000)
    {
        *(.data)
        *(.rodata)
    }

	.eh_frame ALIGN(0x1000) : AT(ADDR(.eh_frame) - 0xFFFFFFFF80000000)
   	{
   	    *(.eh_frame)
   	}

    .bss ALIGN(0x1000) : AT(ADDR(.bss) - 0xFFFFFFFF80000000)
    {
        __bss_start = .;
        *(.bss)
        *(COMMON)
        __bss_end = .;
    }

    __kernel_end = . - 0xFFFFFFFF80000000;
    /DISCARD/ :
    {
       	*(.comment)
    }
}
