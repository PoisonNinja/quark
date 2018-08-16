SECTIONS
{
    .text :
    {
        *(.text)
    }

    .rodata :
    {
        *(.rodata)
    }

    .data :
    {
        *(.data)
    }
    
    .bss :
    {
	    *(.bss)
    }

    .ctors :
    {
        __constructors_start = .;
        *(SORT(.ctors*))
        __constructors_end = .;
    }

    .modinfo :
    {
        *(.modinfo)
    }
    
	/DISCARD/ : { *(.comment) }
}