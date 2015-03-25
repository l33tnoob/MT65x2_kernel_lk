OUTPUT_ARCH(arm)

ENTRY(_start)

romBase = 0x00201000;
ramBase = 0x00102180;
dramBase = 0x87B00000;

MEMORY {
    ram : ORIGIN = ramBase, LENGTH = 0xBA80
    rom : ORIGIN = romBase, LENGTH = 0x1F000
    dram : ORIGIN = dramBase, LENGTH = 0x500000
}

SECTIONS {

    . = romBase;
    .start ALIGN(4) : {	
        *(.text.start)
    } >rom
    
    . = romBase + 0x01FC;
    .rom_info ALIGN(4) : {	    	    	    	     
    	*(.data.rom_info)
    } >rom 

    .text ALIGN(4) : {
        *(.text)
        *(.text.*)        
    } >rom

    .rodata ALIGN(4) : {
        *(.rodata)
        *(.rodata.*)        
    } >rom

    .data ALIGN(4) : {
        *(.data)
        *(.data.*)        
    } >rom

    .got ALIGN(4) : {
        *(.got)
        *(.got.*)        
    } >rom

    __boot_end = .;

    . = ramBase;
    .bss ALIGN(16) : {
        _bss_start = .;
        *(.bss)
        *(.bss.*)
        *(COMMON)
        _bss_end = .;
    } >ram
	
	.secbuf ALIGN(4) : {
		_secbuf_start = .;
		*(.secbuf)
		_secbuf_end = .;
	} >ram

    . = dramBase;
    .dram ALIGN(4) : {
	_dram_start = .;
	*(.dram)
	_dram_end = .;
    } >dram
}
