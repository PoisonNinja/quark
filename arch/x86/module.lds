SECTIONS
{
    .text :
    {
        *(.text)
    }

    .rodata :
    {
        *(.rodata)
        __constructors_start = .;
        *(SORT(.ctors*))
        __constructors_end = .;
    }

    .data :
    {
        *(.data)
    }
    
    .bss :
    {
	    *(.bss)
    }

    .modinfo :
    {
        *(.modinfo)
    }
    
	/DISCARD/ : { *(.comment) }
}