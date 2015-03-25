OUTPUT_ARCH(arm)

ENTRY(_start)

baseaddr = 0x80000300;
pll_init_baseaddr = 0x11141940;
bss_init_emi_baseaddr = 0x01003000;

SECTIONS {

    . = baseaddr;
    .start ALIGN(4) : {
        *(.text.start)
    }

    . = baseaddr + 0x01FC;
    .rom_info ALIGN(4) : {
    	*(.data.rom_info)
    }

    .pll_text pll_init_baseaddr : AT (ADDR(.rom_info) + SIZEOF(.rom_info))
    {
        Image$$PLL_INIT$$Base = . ;
        *(.text.__div0*)
        *eabi.o(.text .data* .rodata*)
        *eabi_idiv.o(.text .data* .rodata*)
        *eabi_uldivmod.o(.text .data* .rodata*)
        *(.text.mtk_pll_init)
        *mt_emi.o(.text.* .data* .rodata*)
        *mtk_timer.o(.text.* .data* .rodata*)
        *custom_emi.o(.text.* .data* .rodata*)
        *print.o(.text.* .data* .rodata*)
        *uart.o(.text.* .data* .rodata*)
        Image$$PLL_INIT$$Limit = . ;
    }

    __load_start_pll_text = LOADADDR (.pll_text);
    __load_stop_pll_text = LOADADDR (.pll_text) + SIZEOF (.pll_text);

    /* sdd 0x200 is larger than .rom_info start address, to avoid overlap (use 0x1FC will be) */
    . = baseaddr + 0x0200 + SIZEOF(.rom_info) + SIZEOF(.pll_text);
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

    __boot_end = .;

    . = bss_init_emi_baseaddr;
    .bss_emi_init ALIGN(16) : {
        *mt_emi.o(.bss*)
        *mtk_timer.o(.bss*)
        *custom_emi.o(.bss*)
        *print.o(.bss*)
        *uart.o(.bss*)
        *eabi.o(.bss*)
        *eabi_idiv.o(.bss*)
        *eabi_uldivmod.o(.bss*)
    }

    /* BMT 8 KB or eMMC PMT 128 KB */
    . = 0x800E0000;
    .uninit ALIGN(16) : {
        *(.bss.uninit)
    }

    /* keep 32KB for BSS section */
    . = 0x800D8000;
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

