OUTPUT_ARCH(arm)

ENTRY(_start)

baseaddr = 0x02007500;

/* use in memory preserved mode*/
/* baseaddr = 0x01000300;      */
/* mem_baseaddr = 0x80100300;  */

pll_init_baseaddr = 0x11141940;

SECTIONS {

    . = baseaddr;
    .start ALIGN(4) : {
        *(.text.start)
    }

    . = baseaddr + 0x01FC;
    .rom_info ALIGN(4) : {
    	*(.data.rom_info)
    }

    .text ALIGN(4) : {
        *(.text)
    }


    .rodata ALIGN(4) : {
        *(.rodata)
    }

    .data ALIGN(4) : {
        *(.data)
    }


    .got ALIGN(4) : {
        *(.got)
    }

    .pll_text pll_init_baseaddr : AT (ADDR(.got) + SIZEOF(.got))
    {
        Image$$PLL_INIT$$Base = . ;
        *(.text.mtk_pll_init)
        Image$$PLL_INIT$$Limit = . ;
    }

    __load_start_pll_text = LOADADDR (.pll_text);
    __load_stop_pll_text = LOADADDR (.pll_text) + SIZEOF (.pll_text);

    __boot_end = .;

    /* BMT 8 KB or eMMC PMT 128 KB */
    . = 0x81D00000;
    .uninit ALIGN(16) : {
        *(.bss.uninit)
    }

    . = 0x02000000;
    .bss ALIGN(16) : {
        _bss_start = .;
        *(.bss)
        *(.bss.*)
        *(COMMON)
        _bss_end = .;
    }


    .comment ALIGN(16) : {
        *(.comment)
    }
}

