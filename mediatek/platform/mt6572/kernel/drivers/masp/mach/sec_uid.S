	.arch armv7-a
	.fpu softvfp
	.eabi_attribute 20, 1
	.eabi_attribute 21, 1
	.eabi_attribute 23, 3
	.eabi_attribute 24, 1
	.eabi_attribute 25, 1
	.eabi_attribute 26, 2
	.eabi_attribute 30, 2
	.eabi_attribute 18, 4
	.file	"sec_uid.c"
	.text
.Ltext0:
	.align	2
	.global	masp_hal_get_uuid
	.type	masp_hal_get_uuid, %function
masp_hal_get_uuid:
.LFB146:
	.file 1 "alps/kernel/mediatek/platform/mt6572/kernel/drivers/masp/mach/sec_uid.c"
	.loc 1 43 0
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 1, uses_anonymous_args = 0
.LVL0:
	mov	ip, sp
.LCFI0:
	stmfd	sp!, {fp, ip, lr, pc}
.LCFI1:
	sub	fp, ip, #4
.LCFI2:
	.loc 1 44 0
	mov	r3, #36864
	movt	r3, 61440
	.loc 1 43 0
	mov	r2, r0
	.loc 1 50 0
	mov	r0, #0
.LVL1:
	.loc 1 44 0
	ldr	r1, [r3, #320]
	str	r1, [r2, #0]
	.loc 1 45 0
	ldr	r1, [r3, #324]
	str	r1, [r2, #4]
	.loc 1 46 0
	ldr	r1, [r3, #320]
	str	r1, [r2, #8]
	.loc 1 47 0
	ldr	r3, [r3, #324]
	str	r3, [r2, #12]
	.loc 1 50 0
	ldmfd	sp, {fp, sp, pc}
.LFE146:
	.size	masp_hal_get_uuid, .-masp_hal_get_uuid
	.section	.debug_frame,"",%progbits
.Lframe0:
	.4byte	.LECIE0-.LSCIE0
.LSCIE0:
	.4byte	0xffffffff
	.byte	0x1
	.ascii	"\000"
	.uleb128 0x1
	.sleb128 -4
	.byte	0xe
	.byte	0xc
	.uleb128 0xd
	.uleb128 0
	.align	2
.LECIE0:
.LSFDE0:
	.4byte	.LEFDE0-.LASFDE0
.LASFDE0:
	.4byte	.Lframe0
	.4byte	.LFB146
	.4byte	.LFE146-.LFB146
	.byte	0x4
	.4byte	.LCFI0-.LFB146
	.byte	0xd
	.uleb128 0xc
	.byte	0x4
	.4byte	.LCFI1-.LCFI0
	.byte	0x8e
	.uleb128 0x2
	.byte	0x8d
	.uleb128 0x3
	.byte	0x8b
	.uleb128 0x4
	.byte	0x4
	.4byte	.LCFI2-.LCFI1
	.byte	0xc
	.uleb128 0xb
	.uleb128 0x4
	.align	2
.LEFDE0:
	.text
.Letext0:
	.file 2 "alps/kernel/arch/arm/include/asm/hwcap.h"
	.file 3 "alps/kernel/include/linux/printk.h"
	.file 4 "alps/kernel/include/linux/kernel.h"
	.section	.debug_info,"",%progbits
.Ldebug_info0:
	.4byte	0xf4
	.2byte	0x2
	.4byte	.Ldebug_abbrev0
	.byte	0x4
	.uleb128 0x1
	.4byte	.LASF14
	.byte	0x1
	.4byte	.LASF15
	.4byte	.LASF16
	.4byte	.Ltext0
	.4byte	.Letext0
	.4byte	.Ldebug_line0
	.4byte	.Ldebug_pubnames0
	.4byte	.Ldebug_pubtypes0
	.uleb128 0x2
	.byte	0x4
	.byte	0x7
	.4byte	.LASF0
	.uleb128 0x2
	.byte	0x4
	.byte	0x7
	.4byte	.LASF1
	.uleb128 0x2
	.byte	0x1
	.byte	0x8
	.4byte	.LASF2
	.uleb128 0x2
	.byte	0x1
	.byte	0x6
	.4byte	.LASF3
	.uleb128 0x2
	.byte	0x1
	.byte	0x8
	.4byte	.LASF4
	.uleb128 0x2
	.byte	0x2
	.byte	0x5
	.4byte	.LASF5
	.uleb128 0x2
	.byte	0x2
	.byte	0x7
	.4byte	.LASF6
	.uleb128 0x3
	.byte	0x4
	.byte	0x5
	.ascii	"int\000"
	.uleb128 0x2
	.byte	0x8
	.byte	0x5
	.4byte	.LASF7
	.uleb128 0x2
	.byte	0x8
	.byte	0x7
	.4byte	.LASF8
	.uleb128 0x2
	.byte	0x4
	.byte	0x5
	.4byte	.LASF9
	.uleb128 0x2
	.byte	0x1
	.byte	0x2
	.4byte	.LASF10
	.uleb128 0x4
	.byte	0x1
	.4byte	.LASF17
	.byte	0x1
	.byte	0x2a
	.byte	0x1
	.4byte	0x5e
	.4byte	.LFB146
	.4byte	.LFE146
	.4byte	.LLST0
	.4byte	0xae
	.uleb128 0x5
	.4byte	.LASF18
	.byte	0x1
	.byte	0x2a
	.4byte	0xae
	.4byte	.LLST1
	.byte	0
	.uleb128 0x6
	.byte	0x4
	.4byte	0x34
	.uleb128 0x7
	.4byte	.LASF11
	.byte	0x2
	.byte	0x23
	.4byte	0x34
	.byte	0x1
	.byte	0x1
	.uleb128 0x8
	.4byte	0x5e
	.4byte	0xcc
	.uleb128 0x9
	.byte	0
	.uleb128 0x7
	.4byte	.LASF12
	.byte	0x3
	.byte	0x1b
	.4byte	0xc1
	.byte	0x1
	.byte	0x1
	.uleb128 0x8
	.4byte	0x3b
	.4byte	0xe4
	.uleb128 0x9
	.byte	0
	.uleb128 0xa
	.4byte	.LASF13
	.byte	0x4
	.2byte	0x18a
	.4byte	0xf2
	.byte	0x1
	.byte	0x1
	.uleb128 0xb
	.4byte	0xd9
	.byte	0
	.section	.debug_abbrev,"",%progbits
.Ldebug_abbrev0:
	.uleb128 0x1
	.uleb128 0x11
	.byte	0x1
	.uleb128 0x25
	.uleb128 0xe
	.uleb128 0x13
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x1b
	.uleb128 0xe
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x1
	.uleb128 0x10
	.uleb128 0x6
	.uleb128 0x2134
	.uleb128 0x6
	.uleb128 0x2135
	.uleb128 0x6
	.byte	0
	.byte	0
	.uleb128 0x2
	.uleb128 0x24
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3e
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0xe
	.byte	0
	.byte	0
	.uleb128 0x3
	.uleb128 0x24
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3e
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0x8
	.byte	0
	.byte	0
	.uleb128 0x4
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3f
	.uleb128 0xc
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x27
	.uleb128 0xc
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x1
	.uleb128 0x40
	.uleb128 0x6
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x5
	.uleb128 0x5
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x6
	.byte	0
	.byte	0
	.uleb128 0x6
	.uleb128 0xf
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x7
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x3f
	.uleb128 0xc
	.uleb128 0x3c
	.uleb128 0xc
	.byte	0
	.byte	0
	.uleb128 0x8
	.uleb128 0x1
	.byte	0x1
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x9
	.uleb128 0x21
	.byte	0
	.byte	0
	.byte	0
	.uleb128 0xa
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x3f
	.uleb128 0xc
	.uleb128 0x3c
	.uleb128 0xc
	.byte	0
	.byte	0
	.uleb128 0xb
	.uleb128 0x26
	.byte	0
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.byte	0
	.section	.debug_loc,"",%progbits
.Ldebug_loc0:
.LLST0:
	.4byte	.LFB146-.Ltext0
	.4byte	.LCFI0-.Ltext0
	.2byte	0x2
	.byte	0x7d
	.sleb128 0
	.4byte	.LCFI0-.Ltext0
	.4byte	.LCFI2-.Ltext0
	.2byte	0x2
	.byte	0x7c
	.sleb128 0
	.4byte	.LCFI2-.Ltext0
	.4byte	.LFE146-.Ltext0
	.2byte	0x2
	.byte	0x7b
	.sleb128 4
	.4byte	0
	.4byte	0
.LLST1:
	.4byte	.LVL0-.Ltext0
	.4byte	.LVL1-.Ltext0
	.2byte	0x1
	.byte	0x50
	.4byte	.LVL1-.Ltext0
	.4byte	.LFE146-.Ltext0
	.2byte	0x1
	.byte	0x52
	.4byte	0
	.4byte	0
	.section	.debug_pubnames,"",%progbits
.Ldebug_pubnames0:
	.4byte	0x24
	.2byte	0x2
	.4byte	.Ldebug_info0
	.4byte	0xf8
	.4byte	0x81
	.ascii	"masp_hal_get_uuid\000"
	.4byte	0
	.section	.debug_pubtypes,"",%progbits
.Ldebug_pubtypes0:
	.4byte	0xd1
	.2byte	0x2
	.4byte	.Ldebug_info0
	.4byte	0xf8
	.4byte	0x2d
	.ascii	"long unsigned int\000"
	.4byte	0x34
	.ascii	"unsigned int\000"
	.4byte	0x3b
	.ascii	"char\000"
	.4byte	0x42
	.ascii	"signed char\000"
	.4byte	0x49
	.ascii	"unsigned char\000"
	.4byte	0x50
	.ascii	"short int\000"
	.4byte	0x57
	.ascii	"short unsigned int\000"
	.4byte	0x5e
	.ascii	"int\000"
	.4byte	0x65
	.ascii	"long long int\000"
	.4byte	0x6c
	.ascii	"long long unsigned int\000"
	.4byte	0x73
	.ascii	"long int\000"
	.4byte	0x7a
	.ascii	"_Bool\000"
	.4byte	0
	.section	.debug_aranges,"",%progbits
	.4byte	0x1c
	.2byte	0x2
	.4byte	.Ldebug_info0
	.byte	0x4
	.byte	0
	.2byte	0
	.2byte	0
	.4byte	.Ltext0
	.4byte	.Letext0-.Ltext0
	.4byte	0
	.4byte	0
	.section	.debug_line,"",%progbits
.Ldebug_line0:
	.section	.debug_str,"MS",%progbits,1
.LASF7:
	.ascii	"long long int\000"
.LASF1:
	.ascii	"unsigned int\000"
.LASF11:
	.ascii	"elf_hwcap\000"
.LASF15:
	.ascii	""
	.ascii	"alps/kernel/mediatek/p"
	.ascii	"latform/mt6572/kernel/drivers/masp/mach/sec_uid.c\000"
.LASF0:
	.ascii	"long unsigned int\000"
.LASF8:
	.ascii	"long long unsigned int\000"
.LASF17:
	.ascii	"masp_hal_get_uuid\000"
.LASF13:
	.ascii	"hex_asc\000"
.LASF4:
	.ascii	"unsigned char\000"
.LASF14:
	.ascii	"GNU C 4.6.x-google 20120106 (prerelease)\000"
.LASF2:
	.ascii	"char\000"
.LASF18:
	.ascii	"uuid\000"
.LASF9:
	.ascii	"long int\000"
.LASF12:
	.ascii	"console_printk\000"
.LASF10:
	.ascii	"_Bool\000"
.LASF6:
	.ascii	"short unsigned int\000"
.LASF3:
	.ascii	"signed char\000"
.LASF16:
	.ascii	""
	.ascii	"alps/kernel/out\000"
.LASF5:
	.ascii	"short int\000"
	.ident	"GCC: (GNU) 4.6.x-google 20120106 (prerelease)"
	.section	.note.GNU-stack,"",%progbits
