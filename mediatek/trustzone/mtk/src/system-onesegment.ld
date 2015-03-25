OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)

ENTRY(_start)
SECTIONS
{
	. = 0x10000000 ;
	
	/* text/read-only data */
	.text.boot : { *(.text.boot) }

	.text :	{ *(.text .text.* .glue_7* .gnu.linkonce.t.*) /*} =0x9090*/
/*	.text.mon : {*/
		. = ALIGN(32);
		_resident_text_start = .;
		KEEP(*(.stext.mon_vector.start))
		KEEP(*(.stext.mon_vector.*))
		_resident_text_end = .;
	}

	.interp : { *(.interp) }
	.hash : { *(.hash) }
	.dynsym : { *(.dynsym) }
	.dynstr : { *(.dynstr) }
	.rel.text : { *(.rel.text) *(.rel.gnu.linkonce.t*) }
	.rela.text : { *(.rela.text) *(.rela.gnu.linkonce.t*) }
	.rel.data : { *(.rel.data) *(.rel.gnu.linkonce.d*) }
	.rela.data : { *(.rela.data) *(.rela.gnu.linkonce.d*) }
	.rel.rodata : { *(.rel.rodata) *(.rel.gnu.linkonce.r*) }
	.rela.rodata : { *(.rela.rodata) *(.rela.gnu.linkonce.r*) }
	.rel.got : { *(.rel.got) }
	.rela.got : { *(.rela.got) }
	.rel.ctors : { *(.rel.ctors) }
	.rela.ctors : { *(.rela.ctors) }
	.rel.dtors : { *(.rel.dtors) }
	.rela.dtors : { *(.rela.dtors) }
	.rel.init : { *(.rel.init) }
	.rela.init : { *(.rela.init) }
	.rel.fini : { *(.rel.fini) }
	.rela.fini : { *(.rela.fini) }
	.rel.bss : { *(.rel.bss) }
	.rela.bss : { *(.rela.bss) }
	.rel.plt : { *(.rel.plt) }
	.rela.plt : { *(.rela.plt) }
	.init : { *(.init) }
	.plt : { *(.plt) }

	.rodata : { 
		*(.rodata .rodata.* .gnu.linkonce.r.*)
		. = ALIGN(4);
		__commands_start = .;
		KEEP (*(.commands))
		__commands_end = .;
		. = ALIGN(4);
		__apps_start = .;
		KEEP (*(.apps))
		__apps_end = .;
		. = ALIGN(4); 
		__rodata_end = . ;		
	}

	.initfunc0 : {
		__initfunc0_start = .;
		*(.initfunc0)
		__initfunc0_end = .;
        }

	.initfunc1 : {
		__initfunc1_start = .;
		*(.initfunc1)
		__initfunc1_end = .;
        }

	.initfunc2 : {
		__initfunc2_start = .;
		*(.initfunc2)
		__initfunc2_end = .;
        }

	.initfunc3 : {
		__initfunc3_start = .;
		*(.initfunc3)
		__initfunc3_end = .;
        }

	.ta_register : {
		__ta_register_start = .;
		*(.ta_register)
		__ta_register_end = .;
	}

	PROVIDE_HIDDEN (__exidx_start = .);
	.ARM.exidx   : { *(.ARM.exidx* .gnu.linkonce.armexidx.*) }
	PROVIDE_HIDDEN (__exidx_end = .);

	/* writable data  */
	__data_start_rom = .;	/* in one segment binaries, the rom data address is on top of the ram data address */
	__data_start = .;
	.data : SUBALIGN(4) { *(.data .data.* .gnu.linkonce.d.*) }

	__ctor_list = .;
	.ctors : { *(.ctors) }
	__ctor_end = .;
	__dtor_list = .;
	.dtors : { *(.dtors) }
	__dtor_end = .;
	.got : { *(.got.plt) *(.got) }
	.dynamic : { *(.dynamic) }

	__data_end = .;

	/* unintialized data (in same segment as writable data) */
	. = ALIGN(4);
	__bss_start = .;
	.bss : { *(.bss .bss.*) }

	. = ALIGN(4); 
	_end = .;	

	/* Strip unnecessary stuff */
	/DISCARD/ : { *(.comment .note .eh_frame) }
}
