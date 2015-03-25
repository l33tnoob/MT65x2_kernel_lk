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
	.file	"hacc_lib.c"
	.text
.Ltext0:
	.align	2
	.global	masp_hal_secure_algo_init
	.type	masp_hal_secure_algo_init, %function
masp_hal_secure_algo_init:
.LFB182:
	.file 1 "alps/kernel/mediatek/platform/mt6572/kernel/drivers/masp/mach/hacc_lib.c"
	.loc 1 61 0
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 1, uses_anonymous_args = 0
	mov	ip, sp
.LCFI0:
	stmfd	sp!, {fp, ip, lr, pc}
.LCFI1:
	sub	fp, ip, #4
.LCFI2:
.LVL0:
	.loc 1 64 0
	mov	r0, #58
	bl	clock_is_on
	cmp	r0, #0
	bne	.L2
	.loc 1 66 0
	mov	r0, #58
	ldr	r1, .L4
	bl	enable_clock
.L2:
	.loc 1 70 0
	mov	r0, #1
	ldmfd	sp, {fp, sp, pc}
.L5:
	.align	2
.L4:
	.word	.LC0
.LFE182:
	.size	masp_hal_secure_algo_init, .-masp_hal_secure_algo_init
	.align	2
	.global	masp_hal_secure_algo_deinit
	.type	masp_hal_secure_algo_deinit, %function
masp_hal_secure_algo_deinit:
.LFB183:
	.loc 1 76 0
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 1, uses_anonymous_args = 0
	mov	ip, sp
.LCFI3:
	stmfd	sp!, {fp, ip, lr, pc}
.LCFI4:
	sub	fp, ip, #4
.LCFI5:
.LVL1:
	.loc 1 79 0
	mov	r0, #58
	bl	clock_is_on
	cmp	r0, #0
	beq	.L7
	.loc 1 81 0
	mov	r0, #58
	ldr	r1, .L8
	bl	disable_clock
.L7:
	.loc 1 85 0
	mov	r0, #1
	ldmfd	sp, {fp, sp, pc}
.L9:
	.align	2
.L8:
	.word	.LC0
.LFE183:
	.size	masp_hal_secure_algo_deinit, .-masp_hal_secure_algo_deinit
	.align	2
	.global	masp_hal_secure_algo
	.type	masp_hal_secure_algo, %function
masp_hal_secure_algo:
.LFB184:
	.loc 1 98 0
	@ args = 4, pretend = 0, frame = 0
	@ frame_needed = 1, uses_anonymous_args = 0
.LVL2:
	mov	ip, sp
.LCFI6:
	stmfd	sp!, {r4, r5, r6, r7, r8, fp, ip, lr, pc}
.LCFI7:
	sub	fp, ip, #4
.LCFI8:
	sub	sp, sp, #12
.LCFI9:
	ldr	r5, [fp, #4]
.LVL3:
	mov	r7, r0
	mov	r8, r1
	mov	r4, r2
	mov	r6, r3
.LVL4:
.L11:
	.loc 1 107 0
	bl	osal_hacc_lock
.LVL5:
	.loc 1 108 0 discriminator 1
	cmp	r0, #0
	bne	.L11
.LVL6:
	.loc 1 112 0
	mov	r0, r6
.LVL7:
	mov	r1, #16
	bl	masp_hal_sp_hacc_init
.LVL8:
	.loc 1 114 0
	subs	r6, r0, #0
.LVL9:
	bne	.L12
.LVL10:
	.loc 1 124 0
	cmp	r7, #0
	bne	.L22
	.loc 1 137 0
	mov	r0, r8
.LVL11:
	mov	r1, r4
	mov	r2, #1
	mov	r3, #2
	str	r7, [sp, #0]
	bl	masp_hal_sp_hacc_dec
.LVL12:
.L15:
	.loc 1 147 0 discriminator 1
	cmp	r4, #0
	beq	.L16
	mov	r3, #0
.LVL13:
.L17:
	.loc 1 149 0
	ldrb	r2, [r0, r3]	@ zero_extendqisi2
	strb	r2, [r5, r3]
	.loc 1 147 0 discriminator 2
	add	r3, r3, #1
.LVL14:
	.loc 1 147 0 is_stmt 0 discriminator 1
	cmp	r3, r4
	bne	.L17
.LVL15:
.L16:
	.loc 1 153 0 is_stmt 1
	bl	osal_hacc_unlock
.LVL16:
	.loc 1 164 0
	sub	sp, fp, #32
	ldmfd	sp, {r4, r5, r6, r7, r8, fp, sp, pc}
.LVL17:
.L22:
	.loc 1 124 0
	cmp	r7, #1
	.loc 1 141 0
	movne	r6, #24576
	.loc 1 124 0
	beq	.L23
.LVL18:
.L12:
	.loc 1 159 0
	bl	osal_hacc_unlock
	.loc 1 161 0 discriminator 1
	ldr	r1, .L24
	mov	r2, r6
	ldr	r0, .L24+4
	bl	printk
	.loc 1 162 0 discriminator 1
#APP
@ 162 "alps/kernel/mediatek/platform/mt6572/kernel/drivers/masp/mach/hacc_lib.c" 1
	.word 0xe7f001f2
@ 0 "" 2
.LVL19:
.L23:
	.loc 1 130 0
	mov	r0, r8
.LVL20:
	mov	r1, r4
	mov	r2, r7
	mov	r3, #2
	str	r6, [sp, #0]
	bl	masp_hal_sp_hacc_enc
.LVL21:
	.loc 1 131 0
	b	.L15
.L25:
	.align	2
.L24:
	.word	.LC2
	.word	.LC1
.LFE184:
	.size	masp_hal_secure_algo, .-masp_hal_secure_algo
	.section	.rodata.str1.4,"aMS",%progbits,1
	.align	2
.LC0:
	.ascii	"MASP\000"
	.space	3
.LC1:
	.ascii	"[%s] masp_hal_secure_algo error (0x%x)\012\000"
.LC2:
	.ascii	"CE\000"
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
	.4byte	.LFB182
	.4byte	.LFE182-.LFB182
	.byte	0x4
	.4byte	.LCFI0-.LFB182
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
.LSFDE2:
	.4byte	.LEFDE2-.LASFDE2
.LASFDE2:
	.4byte	.Lframe0
	.4byte	.LFB183
	.4byte	.LFE183-.LFB183
	.byte	0x4
	.4byte	.LCFI3-.LFB183
	.byte	0xd
	.uleb128 0xc
	.byte	0x4
	.4byte	.LCFI4-.LCFI3
	.byte	0x8e
	.uleb128 0x2
	.byte	0x8d
	.uleb128 0x3
	.byte	0x8b
	.uleb128 0x4
	.byte	0x4
	.4byte	.LCFI5-.LCFI4
	.byte	0xc
	.uleb128 0xb
	.uleb128 0x4
	.align	2
.LEFDE2:
.LSFDE4:
	.4byte	.LEFDE4-.LASFDE4
.LASFDE4:
	.4byte	.Lframe0
	.4byte	.LFB184
	.4byte	.LFE184-.LFB184
	.byte	0x4
	.4byte	.LCFI6-.LFB184
	.byte	0xd
	.uleb128 0xc
	.byte	0x4
	.4byte	.LCFI7-.LCFI6
	.byte	0x8e
	.uleb128 0x2
	.byte	0x8d
	.uleb128 0x3
	.byte	0x8b
	.uleb128 0x4
	.byte	0x88
	.uleb128 0x5
	.byte	0x87
	.uleb128 0x6
	.byte	0x86
	.uleb128 0x7
	.byte	0x85
	.uleb128 0x8
	.byte	0x84
	.uleb128 0x9
	.byte	0x4
	.4byte	.LCFI8-.LCFI7
	.byte	0xc
	.uleb128 0xb
	.uleb128 0x4
	.align	2
.LEFDE4:
	.text
.Letext0:
	.file 2 "alps/kernel/../mediatek/kernel/include/mach/mt_sec_hal.h"
	.file 3 "alps/kernel/../mediatek/platform/mt6572/kernel/core/include/mach/mt_clkmgr.h"
	.file 4 "alps/kernel/mediatek/platform/mt6572/kernel/drivers/masp/asf/asf_inc/sec_typedef.h"
	.file 5 "alps/kernel/arch/arm/include/asm/hwcap.h"
	.file 6 "alps/kernel/include/linux/printk.h"
	.file 7 "alps/kernel/include/linux/kernel.h"
	.section	.debug_info,"",%progbits
.Ldebug_info0:
	.4byte	0x51a
	.2byte	0x2
	.4byte	.Ldebug_abbrev0
	.byte	0x4
	.uleb128 0x1
	.4byte	.LASF148
	.byte	0x1
	.4byte	.LASF149
	.4byte	.LASF150
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
	.byte	0x2
	.byte	0x7
	.4byte	.LASF3
	.uleb128 0x2
	.byte	0x1
	.byte	0x8
	.4byte	.LASF4
	.uleb128 0x2
	.byte	0x4
	.byte	0x5
	.4byte	.LASF5
	.uleb128 0x2
	.byte	0x2
	.byte	0x5
	.4byte	.LASF6
	.uleb128 0x2
	.byte	0x1
	.byte	0x6
	.4byte	.LASF7
	.uleb128 0x3
	.byte	0x4
	.byte	0x5
	.ascii	"int\000"
	.uleb128 0x2
	.byte	0x8
	.byte	0x5
	.4byte	.LASF8
	.uleb128 0x2
	.byte	0x8
	.byte	0x7
	.4byte	.LASF9
	.uleb128 0x4
	.byte	0x4
	.4byte	0x49
	.uleb128 0x2
	.byte	0x4
	.byte	0x4
	.4byte	.LASF10
	.uleb128 0x5
	.byte	0x4
	.byte	0x2
	.byte	0x12
	.4byte	0xa2
	.uleb128 0x6
	.4byte	.LASF11
	.sleb128 0
	.uleb128 0x6
	.4byte	.LASF12
	.sleb128 1
	.uleb128 0x6
	.4byte	.LASF13
	.sleb128 2
	.byte	0
	.uleb128 0x2
	.byte	0x1
	.byte	0x2
	.4byte	.LASF14
	.uleb128 0x7
	.byte	0x4
	.byte	0x3
	.2byte	0x1f6
	.4byte	0x3b2
	.uleb128 0x6
	.4byte	.LASF15
	.sleb128 0
	.uleb128 0x6
	.4byte	.LASF16
	.sleb128 0
	.uleb128 0x6
	.4byte	.LASF17
	.sleb128 1
	.uleb128 0x6
	.4byte	.LASF18
	.sleb128 1
	.uleb128 0x6
	.4byte	.LASF19
	.sleb128 2
	.uleb128 0x6
	.4byte	.LASF20
	.sleb128 2
	.uleb128 0x6
	.4byte	.LASF21
	.sleb128 3
	.uleb128 0x6
	.4byte	.LASF22
	.sleb128 4
	.uleb128 0x6
	.4byte	.LASF23
	.sleb128 5
	.uleb128 0x6
	.4byte	.LASF24
	.sleb128 6
	.uleb128 0x6
	.4byte	.LASF25
	.sleb128 7
	.uleb128 0x6
	.4byte	.LASF26
	.sleb128 8
	.uleb128 0x6
	.4byte	.LASF27
	.sleb128 9
	.uleb128 0x6
	.4byte	.LASF28
	.sleb128 10
	.uleb128 0x6
	.4byte	.LASF29
	.sleb128 11
	.uleb128 0x6
	.4byte	.LASF30
	.sleb128 12
	.uleb128 0x6
	.4byte	.LASF31
	.sleb128 12
	.uleb128 0x6
	.4byte	.LASF32
	.sleb128 13
	.uleb128 0x6
	.4byte	.LASF33
	.sleb128 13
	.uleb128 0x6
	.4byte	.LASF34
	.sleb128 14
	.uleb128 0x6
	.4byte	.LASF35
	.sleb128 15
	.uleb128 0x6
	.4byte	.LASF36
	.sleb128 16
	.uleb128 0x6
	.4byte	.LASF37
	.sleb128 17
	.uleb128 0x6
	.4byte	.LASF38
	.sleb128 18
	.uleb128 0x6
	.4byte	.LASF39
	.sleb128 19
	.uleb128 0x6
	.4byte	.LASF40
	.sleb128 20
	.uleb128 0x6
	.4byte	.LASF41
	.sleb128 21
	.uleb128 0x6
	.4byte	.LASF42
	.sleb128 22
	.uleb128 0x6
	.4byte	.LASF43
	.sleb128 23
	.uleb128 0x6
	.4byte	.LASF44
	.sleb128 24
	.uleb128 0x6
	.4byte	.LASF45
	.sleb128 24
	.uleb128 0x6
	.4byte	.LASF46
	.sleb128 25
	.uleb128 0x6
	.4byte	.LASF47
	.sleb128 25
	.uleb128 0x6
	.4byte	.LASF48
	.sleb128 26
	.uleb128 0x6
	.4byte	.LASF49
	.sleb128 27
	.uleb128 0x6
	.4byte	.LASF50
	.sleb128 28
	.uleb128 0x6
	.4byte	.LASF51
	.sleb128 29
	.uleb128 0x6
	.4byte	.LASF52
	.sleb128 30
	.uleb128 0x6
	.4byte	.LASF53
	.sleb128 31
	.uleb128 0x6
	.4byte	.LASF54
	.sleb128 32
	.uleb128 0x6
	.4byte	.LASF55
	.sleb128 33
	.uleb128 0x6
	.4byte	.LASF56
	.sleb128 34
	.uleb128 0x6
	.4byte	.LASF57
	.sleb128 35
	.uleb128 0x6
	.4byte	.LASF58
	.sleb128 36
	.uleb128 0x6
	.4byte	.LASF59
	.sleb128 37
	.uleb128 0x6
	.4byte	.LASF60
	.sleb128 38
	.uleb128 0x6
	.4byte	.LASF61
	.sleb128 39
	.uleb128 0x6
	.4byte	.LASF62
	.sleb128 39
	.uleb128 0x6
	.4byte	.LASF63
	.sleb128 40
	.uleb128 0x6
	.4byte	.LASF64
	.sleb128 40
	.uleb128 0x6
	.4byte	.LASF65
	.sleb128 41
	.uleb128 0x6
	.4byte	.LASF66
	.sleb128 42
	.uleb128 0x6
	.4byte	.LASF67
	.sleb128 43
	.uleb128 0x6
	.4byte	.LASF68
	.sleb128 44
	.uleb128 0x6
	.4byte	.LASF69
	.sleb128 45
	.uleb128 0x6
	.4byte	.LASF70
	.sleb128 46
	.uleb128 0x6
	.4byte	.LASF71
	.sleb128 47
	.uleb128 0x6
	.4byte	.LASF72
	.sleb128 48
	.uleb128 0x6
	.4byte	.LASF73
	.sleb128 49
	.uleb128 0x6
	.4byte	.LASF74
	.sleb128 50
	.uleb128 0x6
	.4byte	.LASF75
	.sleb128 51
	.uleb128 0x6
	.4byte	.LASF76
	.sleb128 52
	.uleb128 0x6
	.4byte	.LASF77
	.sleb128 53
	.uleb128 0x6
	.4byte	.LASF78
	.sleb128 54
	.uleb128 0x6
	.4byte	.LASF79
	.sleb128 55
	.uleb128 0x6
	.4byte	.LASF80
	.sleb128 56
	.uleb128 0x6
	.4byte	.LASF81
	.sleb128 57
	.uleb128 0x6
	.4byte	.LASF82
	.sleb128 58
	.uleb128 0x6
	.4byte	.LASF83
	.sleb128 59
	.uleb128 0x6
	.4byte	.LASF84
	.sleb128 60
	.uleb128 0x6
	.4byte	.LASF85
	.sleb128 61
	.uleb128 0x6
	.4byte	.LASF86
	.sleb128 62
	.uleb128 0x6
	.4byte	.LASF87
	.sleb128 63
	.uleb128 0x6
	.4byte	.LASF88
	.sleb128 64
	.uleb128 0x6
	.4byte	.LASF89
	.sleb128 64
	.uleb128 0x6
	.4byte	.LASF90
	.sleb128 65
	.uleb128 0x6
	.4byte	.LASF91
	.sleb128 65
	.uleb128 0x6
	.4byte	.LASF92
	.sleb128 66
	.uleb128 0x6
	.4byte	.LASF93
	.sleb128 67
	.uleb128 0x6
	.4byte	.LASF94
	.sleb128 68
	.uleb128 0x6
	.4byte	.LASF95
	.sleb128 69
	.uleb128 0x6
	.4byte	.LASF96
	.sleb128 70
	.uleb128 0x6
	.4byte	.LASF97
	.sleb128 71
	.uleb128 0x6
	.4byte	.LASF98
	.sleb128 72
	.uleb128 0x6
	.4byte	.LASF99
	.sleb128 73
	.uleb128 0x6
	.4byte	.LASF100
	.sleb128 74
	.uleb128 0x6
	.4byte	.LASF101
	.sleb128 75
	.uleb128 0x6
	.4byte	.LASF102
	.sleb128 76
	.uleb128 0x6
	.4byte	.LASF103
	.sleb128 77
	.uleb128 0x6
	.4byte	.LASF104
	.sleb128 78
	.uleb128 0x6
	.4byte	.LASF105
	.sleb128 79
	.uleb128 0x6
	.4byte	.LASF106
	.sleb128 80
	.uleb128 0x6
	.4byte	.LASF107
	.sleb128 81
	.uleb128 0x6
	.4byte	.LASF108
	.sleb128 82
	.uleb128 0x6
	.4byte	.LASF109
	.sleb128 82
	.uleb128 0x6
	.4byte	.LASF110
	.sleb128 83
	.uleb128 0x6
	.4byte	.LASF111
	.sleb128 83
	.uleb128 0x6
	.4byte	.LASF112
	.sleb128 84
	.uleb128 0x6
	.4byte	.LASF113
	.sleb128 85
	.uleb128 0x6
	.4byte	.LASF114
	.sleb128 86
	.uleb128 0x6
	.4byte	.LASF115
	.sleb128 87
	.uleb128 0x6
	.4byte	.LASF116
	.sleb128 88
	.uleb128 0x6
	.4byte	.LASF117
	.sleb128 89
	.uleb128 0x6
	.4byte	.LASF118
	.sleb128 89
	.uleb128 0x6
	.4byte	.LASF119
	.sleb128 90
	.uleb128 0x6
	.4byte	.LASF120
	.sleb128 90
	.uleb128 0x6
	.4byte	.LASF121
	.sleb128 90
	.uleb128 0x6
	.4byte	.LASF122
	.sleb128 91
	.uleb128 0x6
	.4byte	.LASF123
	.sleb128 91
	.uleb128 0x6
	.4byte	.LASF124
	.sleb128 92
	.uleb128 0x6
	.4byte	.LASF125
	.sleb128 93
	.uleb128 0x6
	.4byte	.LASF126
	.sleb128 94
	.uleb128 0x6
	.4byte	.LASF127
	.sleb128 95
	.uleb128 0x6
	.4byte	.LASF128
	.sleb128 96
	.uleb128 0x6
	.4byte	.LASF129
	.sleb128 96
	.uleb128 0x6
	.4byte	.LASF130
	.sleb128 97
	.uleb128 0x6
	.4byte	.LASF131
	.sleb128 98
	.uleb128 0x6
	.4byte	.LASF132
	.sleb128 99
	.uleb128 0x6
	.4byte	.LASF133
	.sleb128 100
	.uleb128 0x6
	.4byte	.LASF134
	.sleb128 101
	.byte	0
	.uleb128 0x8
	.4byte	.LASF135
	.byte	0x4
	.byte	0xe
	.4byte	0x34
	.uleb128 0x8
	.4byte	.LASF136
	.byte	0x4
	.byte	0xf
	.4byte	0x49
	.uleb128 0x9
	.byte	0x1
	.4byte	.LASF137
	.byte	0x1
	.byte	0x3c
	.byte	0x1
	.4byte	0x49
	.4byte	.LFB182
	.4byte	.LFE182
	.4byte	.LLST0
	.4byte	0x3f2
	.uleb128 0xa
	.ascii	"ret\000"
	.byte	0x1
	.byte	0x3e
	.4byte	0xa2
	.byte	0x1
	.byte	0
	.uleb128 0x9
	.byte	0x1
	.4byte	.LASF138
	.byte	0x1
	.byte	0x4b
	.byte	0x1
	.4byte	0x49
	.4byte	.LFB183
	.4byte	.LFE183
	.4byte	.LLST1
	.4byte	0x41c
	.uleb128 0xa
	.ascii	"ret\000"
	.byte	0x1
	.byte	0x4d
	.4byte	0xa2
	.byte	0x1
	.byte	0
	.uleb128 0xb
	.byte	0x1
	.4byte	.LASF151
	.byte	0x1
	.byte	0x61
	.byte	0x1
	.4byte	.LFB184
	.4byte	.LFE184
	.4byte	.LLST2
	.4byte	0x4d4
	.uleb128 0xc
	.4byte	.LASF139
	.byte	0x1
	.byte	0x61
	.4byte	0x49
	.4byte	.LLST3
	.uleb128 0xc
	.4byte	.LASF140
	.byte	0x1
	.byte	0x61
	.4byte	0x34
	.4byte	.LLST4
	.uleb128 0xc
	.4byte	.LASF141
	.byte	0x1
	.byte	0x61
	.4byte	0x34
	.4byte	.LLST5
	.uleb128 0xc
	.4byte	.LASF142
	.byte	0x1
	.byte	0x61
	.4byte	0x7a
	.4byte	.LLST6
	.uleb128 0xd
	.4byte	.LASF143
	.byte	0x1
	.byte	0x61
	.4byte	0x7a
	.byte	0x2
	.byte	0x91
	.sleb128 0
	.uleb128 0xe
	.ascii	"err\000"
	.byte	0x1
	.byte	0x63
	.4byte	0x3b2
	.4byte	.LLST7
	.uleb128 0xe
	.ascii	"src\000"
	.byte	0x1
	.byte	0x64
	.4byte	0x4d4
	.4byte	.LLST8
	.uleb128 0xe
	.ascii	"dst\000"
	.byte	0x1
	.byte	0x64
	.4byte	0x4d4
	.4byte	.LLST9
	.uleb128 0xf
	.4byte	.LASF144
	.byte	0x1
	.byte	0x64
	.4byte	0x4d4
	.4byte	.LLST10
	.uleb128 0xe
	.ascii	"i\000"
	.byte	0x1
	.byte	0x65
	.4byte	0x3b2
	.4byte	.LLST11
	.uleb128 0x10
	.4byte	.LASF152
	.byte	0x1
	.byte	0x9d
	.4byte	.L12
	.byte	0
	.uleb128 0x4
	.byte	0x4
	.4byte	0x3bd
	.uleb128 0x11
	.4byte	.LASF145
	.byte	0x5
	.byte	0x23
	.4byte	0x34
	.byte	0x1
	.byte	0x1
	.uleb128 0x12
	.4byte	0x65
	.4byte	0x4f2
	.uleb128 0x13
	.byte	0
	.uleb128 0x11
	.4byte	.LASF146
	.byte	0x6
	.byte	0x1b
	.4byte	0x4e7
	.byte	0x1
	.byte	0x1
	.uleb128 0x12
	.4byte	0x3b
	.4byte	0x50a
	.uleb128 0x13
	.byte	0
	.uleb128 0x14
	.4byte	.LASF147
	.byte	0x7
	.2byte	0x18a
	.4byte	0x518
	.byte	0x1
	.byte	0x1
	.uleb128 0x15
	.4byte	0x4ff
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
	.uleb128 0xf
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x5
	.uleb128 0x4
	.byte	0x1
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x6
	.uleb128 0x28
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x1c
	.uleb128 0xd
	.byte	0
	.byte	0
	.uleb128 0x7
	.uleb128 0x4
	.byte	0x1
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x8
	.uleb128 0x16
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x9
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
	.uleb128 0xa
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x1c
	.uleb128 0xb
	.byte	0
	.byte	0
	.uleb128 0xb
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
	.uleb128 0xc
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
	.uleb128 0xd
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
	.uleb128 0xa
	.byte	0
	.byte	0
	.uleb128 0xe
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0x8
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
	.uleb128 0xf
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
	.uleb128 0x2
	.uleb128 0x6
	.byte	0
	.byte	0
	.uleb128 0x10
	.uleb128 0xa
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x11
	.uleb128 0x1
	.byte	0
	.byte	0
	.uleb128 0x11
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
	.uleb128 0x12
	.uleb128 0x1
	.byte	0x1
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x13
	.uleb128 0x21
	.byte	0
	.byte	0
	.byte	0
	.uleb128 0x14
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
	.uleb128 0x15
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
	.4byte	.LFB182-.Ltext0
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
	.4byte	.LFE182-.Ltext0
	.2byte	0x2
	.byte	0x7b
	.sleb128 4
	.4byte	0
	.4byte	0
.LLST1:
	.4byte	.LFB183-.Ltext0
	.4byte	.LCFI3-.Ltext0
	.2byte	0x2
	.byte	0x7d
	.sleb128 0
	.4byte	.LCFI3-.Ltext0
	.4byte	.LCFI5-.Ltext0
	.2byte	0x2
	.byte	0x7c
	.sleb128 0
	.4byte	.LCFI5-.Ltext0
	.4byte	.LFE183-.Ltext0
	.2byte	0x2
	.byte	0x7b
	.sleb128 4
	.4byte	0
	.4byte	0
.LLST2:
	.4byte	.LFB184-.Ltext0
	.4byte	.LCFI6-.Ltext0
	.2byte	0x2
	.byte	0x7d
	.sleb128 0
	.4byte	.LCFI6-.Ltext0
	.4byte	.LCFI8-.Ltext0
	.2byte	0x2
	.byte	0x7c
	.sleb128 0
	.4byte	.LCFI8-.Ltext0
	.4byte	.LFE184-.Ltext0
	.2byte	0x2
	.byte	0x7b
	.sleb128 4
	.4byte	0
	.4byte	0
.LLST3:
	.4byte	.LVL2-.Ltext0
	.4byte	.LVL4-.Ltext0
	.2byte	0x1
	.byte	0x50
	.4byte	0
	.4byte	0
.LLST4:
	.4byte	.LVL2-.Ltext0
	.4byte	.LVL4-.Ltext0
	.2byte	0x1
	.byte	0x51
	.4byte	.LVL4-.Ltext0
	.4byte	.LFE184-.Ltext0
	.2byte	0x1
	.byte	0x58
	.4byte	0
	.4byte	0
.LLST5:
	.4byte	.LVL2-.Ltext0
	.4byte	.LVL4-.Ltext0
	.2byte	0x1
	.byte	0x52
	.4byte	.LVL4-.Ltext0
	.4byte	.LFE184-.Ltext0
	.2byte	0x1
	.byte	0x54
	.4byte	0
	.4byte	0
.LLST6:
	.4byte	.LVL2-.Ltext0
	.4byte	.LVL4-.Ltext0
	.2byte	0x1
	.byte	0x53
	.4byte	.LVL4-.Ltext0
	.4byte	.LVL9-.Ltext0
	.2byte	0x1
	.byte	0x56
	.4byte	0
	.4byte	0
.LLST7:
	.4byte	.LVL5-.Ltext0
	.4byte	.LVL7-.Ltext0
	.2byte	0x1
	.byte	0x50
	.4byte	.LVL8-.Ltext0
	.4byte	.LVL11-.Ltext0
	.2byte	0x1
	.byte	0x50
	.4byte	.LVL11-.Ltext0
	.4byte	.LVL12-.Ltext0
	.2byte	0x1
	.byte	0x56
	.4byte	.LVL17-.Ltext0
	.4byte	.LVL18-.Ltext0
	.2byte	0x1
	.byte	0x50
	.4byte	.LVL18-.Ltext0
	.4byte	.LVL19-.Ltext0
	.2byte	0x1
	.byte	0x56
	.4byte	.LVL19-.Ltext0
	.4byte	.LVL20-.Ltext0
	.2byte	0x1
	.byte	0x50
	.4byte	0
	.4byte	0
.LLST8:
	.4byte	.LVL10-.Ltext0
	.4byte	.LVL18-.Ltext0
	.2byte	0x1
	.byte	0x58
	.4byte	.LVL19-.Ltext0
	.4byte	.LFE184-.Ltext0
	.2byte	0x1
	.byte	0x58
	.4byte	0
	.4byte	0
.LLST9:
	.4byte	.LVL10-.Ltext0
	.4byte	.LVL12-.Ltext0
	.2byte	0x2
	.byte	0x91
	.sleb128 0
	.4byte	.LVL12-.Ltext0
	.4byte	.LVL16-1-.Ltext0
	.2byte	0x1
	.byte	0x50
	.4byte	.LVL17-.Ltext0
	.4byte	.LVL18-.Ltext0
	.2byte	0x2
	.byte	0x91
	.sleb128 0
	.4byte	.LVL19-.Ltext0
	.4byte	.LVL21-.Ltext0
	.2byte	0x2
	.byte	0x91
	.sleb128 0
	.4byte	.LVL21-.Ltext0
	.4byte	.LFE184-.Ltext0
	.2byte	0x1
	.byte	0x50
	.4byte	0
	.4byte	0
.LLST10:
	.4byte	.LVL6-.Ltext0
	.4byte	.LVL9-.Ltext0
	.2byte	0x1
	.byte	0x56
	.4byte	0
	.4byte	0
.LLST11:
	.4byte	.LVL3-.Ltext0
	.4byte	.LVL13-.Ltext0
	.2byte	0x2
	.byte	0x30
	.byte	0x9f
	.4byte	.LVL14-.Ltext0
	.4byte	.LVL15-.Ltext0
	.2byte	0x1
	.byte	0x53
	.4byte	.LVL17-.Ltext0
	.4byte	.LFE184-.Ltext0
	.2byte	0x2
	.byte	0x30
	.byte	0x9f
	.4byte	0
	.4byte	0
	.section	.debug_pubnames,"",%progbits
.Ldebug_pubnames0:
	.4byte	0x65
	.2byte	0x2
	.4byte	.Ldebug_info0
	.4byte	0x51e
	.4byte	0x3c8
	.ascii	"masp_hal_secure_algo_init\000"
	.4byte	0x3f2
	.ascii	"masp_hal_secure_algo_deinit\000"
	.4byte	0x41c
	.ascii	"masp_hal_secure_algo\000"
	.4byte	0
	.section	.debug_pubtypes,"",%progbits
.Ldebug_pubtypes0:
	.4byte	0xf0
	.2byte	0x2
	.4byte	.Ldebug_info0
	.4byte	0x51e
	.4byte	0x2d
	.ascii	"long unsigned int\000"
	.4byte	0x34
	.ascii	"unsigned int\000"
	.4byte	0x3b
	.ascii	"char\000"
	.4byte	0x42
	.ascii	"short unsigned int\000"
	.4byte	0x49
	.ascii	"unsigned char\000"
	.4byte	0x50
	.ascii	"long int\000"
	.4byte	0x57
	.ascii	"short int\000"
	.4byte	0x5e
	.ascii	"signed char\000"
	.4byte	0x65
	.ascii	"int\000"
	.4byte	0x6c
	.ascii	"long long int\000"
	.4byte	0x73
	.ascii	"long long unsigned int\000"
	.4byte	0x80
	.ascii	"float\000"
	.4byte	0xa2
	.ascii	"_Bool\000"
	.4byte	0x3b2
	.ascii	"uint32\000"
	.4byte	0x3bd
	.ascii	"uchar\000"
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
.LASF107:
	.ascii	"MT_CG_DISP_FAKE_ENG_SW_CG\000"
.LASF69:
	.ascii	"MT_CG_NFI_SW_CG\000"
.LASF31:
	.ascii	"CG_MPLL_TO\000"
.LASF104:
	.ascii	"MT_CG_MM_SENINF_SW_CG\000"
.LASF70:
	.ascii	"MT_CG_NFIECC_SW_CG\000"
.LASF78:
	.ascii	"MT_CG_SPINFI_SW_CG\000"
.LASF146:
	.ascii	"console_printk\000"
.LASF84:
	.ascii	"MT_CG_APXGPT_SW_CG\000"
.LASF131:
	.ascii	"MT_CG_SRC_DBI\000"
.LASF119:
	.ascii	"MT_CG_MFG_PDN_BG3D_SW_CG\000"
.LASF92:
	.ascii	"MT_CG_DISP_BLS_SW_CG\000"
.LASF142:
	.ascii	"CustomSeed\000"
.LASF1:
	.ascii	"unsigned int\000"
.LASF150:
	.ascii	""
	.ascii	"alps/kernel/out\000"
.LASF91:
	.ascii	"CG_MMSYS0_FROM\000"
.LASF123:
	.ascii	"CG_AUDIO_FROM\000"
.LASF63:
	.ascii	"MT_CG_EFUSE_SW_CG\000"
.LASF97:
	.ascii	"MT_CG_MDP_WROT_SW_CG\000"
.LASF114:
	.ascii	"MT_CG_DISP_DPI_IF_SW_CG\000"
.LASF96:
	.ascii	"MT_CG_MDP_TDSHP_SW_CG\000"
.LASF126:
	.ascii	"MT_CG_AUD_PDN_DAC_EN\000"
.LASF152:
	.ascii	"_error\000"
.LASF17:
	.ascii	"MT_CG_USB_48M\000"
.LASF109:
	.ascii	"CG_MMSYS0_TO\000"
.LASF100:
	.ascii	"MT_CG_MDP_RSZ0_SW_CG\000"
.LASF71:
	.ascii	"MT_CG_DEBUGSYS_SW_CG\000"
.LASF10:
	.ascii	"float\000"
.LASF19:
	.ascii	"MT_CG_MPLL_D2\000"
.LASF21:
	.ascii	"MT_CG_MPLL_D3\000"
.LASF24:
	.ascii	"MT_CG_MPLL_D4\000"
.LASF22:
	.ascii	"MT_CG_MPLL_D5\000"
.LASF25:
	.ascii	"MT_CG_MPLL_D6\000"
.LASF23:
	.ascii	"MT_CG_MPLL_D7\000"
.LASF27:
	.ascii	"MT_CG_MPLL_D8\000"
.LASF81:
	.ascii	"MT_CG_PMIC_SW_CG_AP\000"
.LASF89:
	.ascii	"CG_CTRL1_TO\000"
.LASF112:
	.ascii	"MT_CG_DSI_DIGITAL_SW_CG\000"
.LASF9:
	.ascii	"long long unsigned int\000"
.LASF53:
	.ascii	"MT_CG_SC_26M_CK_SEL_EN\000"
.LASF130:
	.ascii	"NR_CLKS\000"
.LASF47:
	.ascii	"CG_CTRL0_FROM\000"
.LASF11:
	.ascii	"HACC_USER1\000"
.LASF12:
	.ascii	"HACC_USER2\000"
.LASF13:
	.ascii	"HACC_USER3\000"
.LASF122:
	.ascii	"MT_CG_AUD_PDN_AFE_EN\000"
.LASF141:
	.ascii	"ContentLen\000"
.LASF55:
	.ascii	"MT_CG_DBI_PAD0_SW_CG\000"
.LASF125:
	.ascii	"MT_CG_AUD_PDN_ADC_EN\000"
.LASF60:
	.ascii	"MT_CG_GPU_500P5M_EN\000"
.LASF86:
	.ascii	"MT_CG_SPM_SW_CG\000"
.LASF14:
	.ascii	"_Bool\000"
.LASF83:
	.ascii	"MT_CG_MEMSLP_DLYER_SW_CG\000"
.LASF120:
	.ascii	"CG_MFG_FROM\000"
.LASF2:
	.ascii	"char\000"
.LASF46:
	.ascii	"MT_CG_PWM_MM_SW_CG\000"
.LASF85:
	.ascii	"MT_CG_AUDIO_SW_CG\000"
.LASF57:
	.ascii	"MT_CG_DBI_PAD2_SW_CG\000"
.LASF79:
	.ascii	"MT_CG_MSDC0_SW_CG\000"
.LASF68:
	.ascii	"MT_CG_I2C1_SW_CG\000"
.LASF94:
	.ascii	"MT_CG_DISP_RDMA_SW_CG\000"
.LASF50:
	.ascii	"MT_CG_SPM_52M_SW_CG\000"
.LASF39:
	.ascii	"MT_CG_UPLL_D10\000"
.LASF41:
	.ascii	"MT_CG_UPLL_D12\000"
.LASF90:
	.ascii	"MT_CG_DISP_COLOR_SW_CG\000"
.LASF43:
	.ascii	"MT_CG_UPLL_D16\000"
.LASF108:
	.ascii	"MT_CG_MUTEX_SLOW_CLOCK_SW_CG\000"
.LASF75:
	.ascii	"MT_CG_BTIF_SW_CG\000"
.LASF95:
	.ascii	"MT_CG_DISP_OVL_SW_CG\000"
.LASF121:
	.ascii	"CG_MFG_TO\000"
.LASF8:
	.ascii	"long long int\000"
.LASF52:
	.ascii	"MT_CG_DBI_BCLK_SW_CG\000"
.LASF54:
	.ascii	"MT_CG_SC_MEM_CK_OFF_EN\000"
.LASF138:
	.ascii	"masp_hal_secure_algo_deinit\000"
.LASF42:
	.ascii	"MT_CG_UPLL_D20\000"
.LASF44:
	.ascii	"MT_CG_UPLL_D24\000"
.LASF45:
	.ascii	"CG_UPLL_TO\000"
.LASF117:
	.ascii	"MT_CG_DISP_DBI_IF_SW_CG\000"
.LASF74:
	.ascii	"MT_CG_UART1_SW_CG\000"
.LASF110:
	.ascii	"MT_CG_DSI_ENGINE_SW_CG\000"
.LASF26:
	.ascii	"MT_CG_MPLL_D10\000"
.LASF28:
	.ascii	"MT_CG_MPLL_D12\000"
.LASF140:
	.ascii	"ContentAddr\000"
.LASF135:
	.ascii	"uint32\000"
.LASF93:
	.ascii	"MT_CG_DISP_WDMA_SW_CG\000"
.LASF77:
	.ascii	"MT_CG_FHCTL_SW_CG\000"
.LASF144:
	.ascii	"seed\000"
.LASF136:
	.ascii	"uchar\000"
.LASF101:
	.ascii	"MT_CG_MDP_RDMA_SW_CG\000"
.LASF124:
	.ascii	"MT_CG_AUD_PDN_I2S_EN\000"
.LASF20:
	.ascii	"CG_MPLL_FROM\000"
.LASF29:
	.ascii	"MT_CG_MPLL_D20\000"
.LASF30:
	.ascii	"MT_CG_MPLL_D24\000"
.LASF105:
	.ascii	"MT_CG_MM_CAMTG_SW_CG\000"
.LASF82:
	.ascii	"MT_CG_SEJ_SW_CG\000"
.LASF32:
	.ascii	"MT_CG_UPLL_D2\000"
.LASF34:
	.ascii	"MT_CG_UPLL_D3\000"
.LASF37:
	.ascii	"MT_CG_UPLL_D4\000"
.LASF35:
	.ascii	"MT_CG_UPLL_D5\000"
.LASF38:
	.ascii	"MT_CG_UPLL_D6\000"
.LASF36:
	.ascii	"MT_CG_UPLL_D7\000"
.LASF40:
	.ascii	"MT_CG_UPLL_D8\000"
.LASF111:
	.ascii	"CG_MMSYS1_FROM\000"
.LASF62:
	.ascii	"CG_CTRL0_TO\000"
.LASF106:
	.ascii	"MT_CG_MM_CODEC_SW_CG\000"
.LASF51:
	.ascii	"MT_CG_MIPI_26M_DBG_EN\000"
.LASF48:
	.ascii	"MT_CG_CAM_MM_SW_CG\000"
.LASF87:
	.ascii	"MT_CG_PMIC_26M_SW_CG\000"
.LASF102:
	.ascii	"MT_CG_MDP_BLS_26M_SW_CG\000"
.LASF6:
	.ascii	"short int\000"
.LASF116:
	.ascii	"MT_CG_DISP_DBI_SMI_SW_CG\000"
.LASF5:
	.ascii	"long int\000"
.LASF65:
	.ascii	"MT_CG_THEM_SW_CG\000"
.LASF133:
	.ascii	"MT_CG_CLOCK_OFF\000"
.LASF145:
	.ascii	"elf_hwcap\000"
.LASF61:
	.ascii	"MT_CG_ARMDCM_CLKOFF_EN\000"
.LASF132:
	.ascii	"MT_CG_SRC_SMI\000"
.LASF147:
	.ascii	"hex_asc\000"
.LASF115:
	.ascii	"MT_CG_DISP_DBI_ENGINE_SW_CG\000"
.LASF143:
	.ascii	"ResText\000"
.LASF99:
	.ascii	"MT_CG_MDP_RSZ1_SW_CG\000"
.LASF118:
	.ascii	"CG_MMSYS1_TO\000"
.LASF148:
	.ascii	"GNU C 4.6.x-google 20120106 (prerelease)\000"
.LASF134:
	.ascii	"MT_CG_INVALID\000"
.LASF127:
	.ascii	"MT_CG_AUD_PDN_DAC_PREDIS_EN\000"
.LASF0:
	.ascii	"long unsigned int\000"
.LASF18:
	.ascii	"CG_MIXED_TO\000"
.LASF15:
	.ascii	"MT_CG_SYS_26M\000"
.LASF139:
	.ascii	"Direction\000"
.LASF59:
	.ascii	"MT_CG_GPU_491P52M_EN\000"
.LASF4:
	.ascii	"unsigned char\000"
.LASF76:
	.ascii	"MT_CG_USB_SW_CG\000"
.LASF98:
	.ascii	"MT_CG_MDP_WDMA_SW_CG\000"
.LASF88:
	.ascii	"MT_CG_AUX_SW_CG_ADC\000"
.LASF56:
	.ascii	"MT_CG_DBI_PAD1_SW_CG\000"
.LASF128:
	.ascii	"MT_CG_AUD_PDN_TML_EN\000"
.LASF67:
	.ascii	"MT_CG_I2C0_SW_CG\000"
.LASF58:
	.ascii	"MT_CG_DBI_PAD3_SW_CG\000"
.LASF66:
	.ascii	"MT_CG_APDMA_SW_CG\000"
.LASF49:
	.ascii	"MT_CG_MFG_MM_SW_CG\000"
.LASF33:
	.ascii	"CG_UPLL_FROM\000"
.LASF7:
	.ascii	"signed char\000"
.LASF64:
	.ascii	"CG_CTRL1_FROM\000"
.LASF3:
	.ascii	"short unsigned int\000"
.LASF103:
	.ascii	"MT_CG_MM_CAM_SW_CG\000"
.LASF137:
	.ascii	"masp_hal_secure_algo_init\000"
.LASF151:
	.ascii	"masp_hal_secure_algo\000"
.LASF16:
	.ascii	"CG_MIXED_FROM\000"
.LASF113:
	.ascii	"MT_CG_DISP_DPI_ENGINE_SW_CG\000"
.LASF149:
	.ascii	""
	.ascii	"alps/kernel/mediatek/p"
	.ascii	"latform/mt6572/kernel/drivers/masp/mach/hacc_lib.c\000"
.LASF73:
	.ascii	"MT_CG_UART0_SW_CG\000"
.LASF129:
	.ascii	"CG_AUDIO_TO\000"
.LASF80:
	.ascii	"MT_CG_MSDC1_SW_CG\000"
.LASF72:
	.ascii	"MT_CG_PWM_SW_CG\000"
	.ident	"GCC: (GNU) 4.6.x-google 20120106 (prerelease)"
	.section	.note.GNU-stack,"",%progbits
