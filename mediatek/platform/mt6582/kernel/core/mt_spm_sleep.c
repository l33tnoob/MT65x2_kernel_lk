#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/delay.h>
#include <linux/string.h>
#include <linux/aee.h>

#include <mach/irqs.h>
#include <mach/mt_cirq.h>
#include <mach/mt_spm.h>
#include <mach/mt_spm_sleep.h>
#include <mach/mt_clkmgr.h>
#include <mach/mt_dormant.h>
#include <mach/wd_api.h>
#include <mach/eint.h>
#include <mach/mtk_ccci_helper.h>

/**************************************
 * only for internal debug
 **************************************/
#ifdef CONFIG_MTK_LDVT
#define SPM_PWAKE_EN            0
#define SPM_PCMWDT_EN           0
#define SPM_BYPASS_SYSPWREQ     1
#else
#define SPM_PWAKE_EN            1
#define SPM_PCMWDT_EN           1
#define SPM_BYPASS_SYSPWREQ     0
#endif

#ifdef IS_VCORE_USE_6333VCORE
#define SPM_CTRL_33VCORE        1
#else
#define SPM_CTRL_33VCORE        0
#endif

#ifdef IS_VRF18_USE_6333VRF18
#define SPM_CTRL_33VRF18        1
#else
#define SPM_CTRL_33VRF18        0
#endif


/**********************************************************
 * PCM code for suspend (v34rc10 @ 2013-07-11)
 **********************************************************/
#if SPM_CTRL_33VRF18
static const u32 __pcm_suspend[] = {
    0x19c0001f, 0x001c4bd7, 0x1800001f, 0x17cf0f3f, 0x1b80001f, 0x20000000,
    0x1800001f, 0x17cf0f16, 0x19c0001f, 0x001c4be7, 0xd80002c6, 0x17c07c1f,
    0x18c0001f, 0x10006234, 0xc0c013a0, 0x1200041f, 0x18c0001f, 0x10006240,
    0xe0e00f16, 0xe0e00f1e, 0xe0e00f0e, 0xe0e00f0f, 0xe8208000, 0x10006354,
    0xfffffbff, 0xc0c01cc0, 0x1280041f, 0x1b00001f, 0x7fffd7ff, 0xf0000000,
    0x17c07c1f, 0x1b00001f, 0x3fffc7ff, 0x1b80001f, 0x20000004, 0xd80007ec,
    0x17c07c1f, 0xe8208000, 0x10006354, 0xfffffbff, 0xc0c01cc0, 0x12807c1f,
    0xd80006a6, 0x17c07c1f, 0x18c0001f, 0x10006240, 0xe0e00f0f, 0xe0e00f1e,
    0xe0e00f12, 0x18c0001f, 0x10006234, 0xc0c01580, 0x17c07c1f, 0x1b00001f,
    0x3fffcfff, 0x19c0001f, 0x001c6bd7, 0x1800001f, 0x17cf0f3f, 0x1800001f,
    0x17ff0f3f, 0x19c0001f, 0x001823d7, 0xf0000000, 0x17c07c1f, 0x18c0001f,
    0x10006294, 0xc0c01620, 0x17c07c1f, 0x1800001f, 0x07cf0f1e, 0x1b80001f,
    0x20000a50, 0x1800001f, 0x07ce0f1e, 0x1b80001f, 0x20000300, 0x1800001f,
    0x078e0f1e, 0x1b80001f, 0x20000300, 0x1800001f, 0x038e0f1e, 0x1b80001f,
    0x20000300, 0x1800001f, 0x038e0e1e, 0x1800001f, 0x038e0e12, 0x1b80001f,
    0x200000ed, 0x18c0001f, 0x10006240, 0xe0e00f0d, 0x1b80001f, 0x2000000e,
    0x19c0001f, 0x000c4ba7, 0x19c0001f, 0x000c4ba5, 0xe8208000, 0x10006354,
    0xfffffa43, 0x19c0001f, 0x000d4ba5, 0x1b00001f, 0xbfffc7ff, 0xf0000000,
    0x17c07c1f, 0x1b80001f, 0x20000fdf, 0x8880000d, 0x00000024, 0x1b00001f,
    0xbfffc7ff, 0xd8001362, 0x17c07c1f, 0x1b00001f, 0x3fffc7ff, 0x1b80001f,
    0x20000004, 0xd800136c, 0x17c07c1f, 0xe8208000, 0x10006354, 0xffffffff,
    0x19c0001f, 0x001c4be5, 0x1880001f, 0x10006320, 0xc0c018e0, 0xe080000f,
    0xd8001363, 0x17c07c1f, 0xe080001f, 0xc0c01a00, 0x17c07c1f, 0x18c0001f,
    0x10006294, 0xe0f07ff0, 0xe0e00ff0, 0xe0e000f0, 0xe8208000, 0x10006294,
    0x000f00f0, 0x1800001f, 0x038e0e16, 0x1800001f, 0x038e0f16, 0x1800001f,
    0x07ce0f16, 0x1800001f, 0x17cf0f16, 0x1b00001f, 0x7fffd7ff, 0xf0000000,
    0x17c07c1f, 0xe0e00f16, 0x1380201f, 0xe0e00f1e, 0x1380201f, 0xe0e00f0e,
    0x1b80001f, 0x20000100, 0xe0e00f0f, 0xe0e00f0d, 0xe0e00e0d, 0xe0e00c0d,
    0xe0e0080d, 0xe0e0000d, 0xf0000000, 0x17c07c1f, 0xe0e00f0d, 0xe0e00f1e,
    0xe0e00f12, 0xf0000000, 0x17c07c1f, 0xe8208000, 0x10006294, 0x000e00f0,
    0xe8208000, 0x10006294, 0x000c00f0, 0xe8208000, 0x10006294, 0x000800f0,
    0xe8208000, 0x10006294, 0x000000f0, 0xe0e008f0, 0xe0e00cf0, 0xe0e00ef0,
    0xe0e00ff0, 0x1b80001f, 0x20000100, 0xe0f07ff0, 0xe0f07f00, 0xf0000000,
    0x17c07c1f, 0xa1d08407, 0x1b80001f, 0x200000ed, 0x80eab401, 0x1a00001f,
    0x10006814, 0xe2000003, 0xf0000000, 0x17c07c1f, 0x18c0001f, 0x80000000,
    0x1a10001f, 0x10002058, 0x1a80001f, 0x10002058, 0xa2000c08, 0xe2800008,
    0x1a10001f, 0x1000206c, 0x1a80001f, 0x1000206c, 0xa2000c08, 0xe2800008,
    0x1a10001f, 0x10002080, 0x1a80001f, 0x10002080, 0xa2000c08, 0xe2800008,
    0xf0000000, 0x17c07c1f, 0x18d0001f, 0x10006600, 0xe8208000, 0x11008014,
    0x00000002, 0xe8208000, 0x11008020, 0x00000101, 0xe8208000, 0x11008004,
    0x000000d6, 0xd8001eaa, 0xe26000a0, 0xd8201eca, 0xe2600000, 0xe2400003,
    0xe8208000, 0x11008024, 0x00000001, 0x1b80001f, 0x20000424, 0xf0000000,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x1840001f, 0x00000001,
    0xa1d48407, 0x1990001f, 0x10006400, 0x1a40001f, 0x11008000, 0x1b00001f,
    0x3fffc7ff, 0x1b80001f, 0xd00f0000, 0x8880000c, 0x3fffc7ff, 0xd8003fc2,
    0x1140041f, 0xe8208000, 0x10006354, 0xfffffa43, 0xc0c03b60, 0x81471801,
    0xd80025c5, 0x17c07c1f, 0x89c00007, 0xffffefff, 0x18c0001f, 0x10006200,
    0xc0c03ca0, 0x12807c1f, 0xe8208000, 0x1000625c, 0x00000001, 0x1b80001f,
    0x20000080, 0xc0c03ca0, 0x1280041f, 0x18c0001f, 0x10006208, 0xc0c03ca0,
    0x12807c1f, 0xe8208000, 0x10006248, 0x00000000, 0x1b80001f, 0x20000080,
    0xc0c03ca0, 0x1280041f, 0xc0c03c00, 0x81879801, 0x1b00001f, 0xffffdfff,
    0x1b80001f, 0x90010000, 0x8880000c, 0x3fffc7ff, 0xd8003a42, 0x17c07c1f,
    0x8880000c, 0x40000800, 0xd8002602, 0x17c07c1f, 0x19c0001f, 0x00044b25,
    0xc0c01cc0, 0x12807c1f, 0x1880001f, 0x10006320, 0xe8208000, 0x10006354,
    0xffffffff, 0xc0c018e0, 0xe080000f, 0xd8002603, 0x17c07c1f, 0xe8208000,
    0x10006310, 0x0b1600f8, 0xe080001f, 0x19c0001f, 0x001c4be7, 0x1b80001f,
    0x20000030, 0xc0c01a00, 0x17c07c1f, 0xd8002b26, 0x17c07c1f, 0x18c0001f,
    0x10006240, 0xc0c01580, 0x17c07c1f, 0x18c0001f, 0x10006294, 0xe0f07ff0,
    0xe0e00ff0, 0xe0e000f0, 0xe8208000, 0x10006294, 0x000f00f0, 0x1800001f,
    0x00000036, 0x1800001f, 0x00000f36, 0x1800001f, 0x07c00f36, 0x1800001f,
    0x17cf0f36, 0xd8002de6, 0x17c07c1f, 0x18c0001f, 0x10006234, 0xc0c01580,
    0x17c07c1f, 0x19c0001f, 0x001c6bd7, 0x1800001f, 0x17cf0f3f, 0x1800001f,
    0x17ff0f3f, 0x19c0001f, 0x001823d7, 0x1b00001f, 0x3fffcfff, 0x1b80001f,
    0x90100000, 0x80c00400, 0xd8003043, 0x80980400, 0xd8003342, 0x17c07c1f,
    0xd8203862, 0x17c07c1f, 0x19c0001f, 0x001c4bd7, 0x1800001f, 0x17cf0f3f,
    0x1b80001f, 0x20000000, 0x1800001f, 0x17cf0f16, 0x19c0001f, 0x001c4be7,
    0xd8003246, 0x17c07c1f, 0x18c0001f, 0x10006234, 0xc0c013a0, 0x1200041f,
    0xd8003346, 0x17c07c1f, 0x18c0001f, 0x10006240, 0xe0e00f16, 0xe0e00f1e,
    0xe0e00f0e, 0xe0e00f0f, 0x18c0001f, 0x10006294, 0xc0c01620, 0x17c07c1f,
    0xe8208000, 0x10006354, 0xfffffbff, 0xc0c01cc0, 0x1280041f, 0x19c0001f,
    0x001c4ba7, 0x1800001f, 0x07cf0f16, 0x1b80001f, 0x20000a50, 0x1800001f,
    0x07c00f16, 0x1b80001f, 0x20000300, 0x1800001f, 0x04000f16, 0x1b80001f,
    0x20000300, 0x1800001f, 0x00000f16, 0x1b80001f, 0x20000300, 0x1800001f,
    0x00000016, 0x10007c1f, 0x1b80001f, 0x2000049c, 0x1b80001f, 0x200000ed,
    0x18c0001f, 0x10006240, 0xe0e00f0d, 0x1b80001f, 0x2000000e, 0xd0003a00,
    0x17c07c1f, 0x1800001f, 0x03800e12, 0x1b80001f, 0x20000300, 0x1800001f,
    0x00000e12, 0x1b80001f, 0x20000300, 0x1800001f, 0x00000012, 0x10007c1f,
    0x1b80001f, 0x2000079e, 0x19c0001f, 0x00054b25, 0xe8208000, 0x10006354,
    0xfffffa43, 0x19c0001f, 0x00014b25, 0x19c0001f, 0x00014a25, 0xd0003fc0,
    0x17c07c1f, 0xa1d10407, 0x1b80001f, 0x20000020, 0xf0000000, 0x17c07c1f,
    0xa1d40407, 0x1391841f, 0xa1d90407, 0xf0000000, 0x17c07c1f, 0xd8003d2a,
    0x17c07c1f, 0xe2e0006d, 0xe2e0002d, 0xd8203dca, 0x17c07c1f, 0xe2e0002f,
    0xe2e0003e, 0xe2e00032, 0xf0000000, 0x17c07c1f, 0xd0003fc0, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0xd8004345, 0x17c07c1f, 0x18c0001f, 0x10006208,
    0x1212841f, 0xc0c04860, 0x12807c1f, 0xe8208000, 0x10006248, 0x00000001,
    0x1b80001f, 0x20000080, 0xc0c04860, 0x1280041f, 0x18c0001f, 0x10006200,
    0x1212841f, 0xc0c04860, 0x12807c1f, 0xe8208000, 0x1000625c, 0x00000000,
    0x1b80001f, 0x20000080, 0xc0c04860, 0x1280041f, 0x19c0001f, 0x00415820,
    0x1ac0001f, 0x55aa55aa, 0x10007c1f, 0x80cab001, 0x808cb401, 0x80800c02,
    0xd82044a2, 0x17c07c1f, 0xa1d78407, 0x1240301f, 0xe8208000, 0x100063e0,
    0x00000001, 0x1b00001f, 0x00202000, 0x1b80001f, 0x80001000, 0x8880000c,
    0x00200000, 0xd80046c2, 0x17c07c1f, 0xe8208000, 0x100063e0, 0x00000002,
    0x1b80001f, 0x00001000, 0x809c840d, 0xd8204522, 0x17c07c1f, 0xa1d78407,
    0x1890001f, 0x10006014, 0x18c0001f, 0x10006014, 0xa0978402, 0xe0c00002,
    0x1b80001f, 0x00001000, 0xf0000000, 0xd800496a, 0x17c07c1f, 0xe2e00036,
    0x1380201f, 0xe2e0003e, 0x1380201f, 0xe2e0002e, 0x1380201f, 0xd8204a6a,
    0x17c07c1f, 0xe2e0006e, 0xe2e0004e, 0xe2e0004c, 0x1b80001f, 0x20000020,
    0xe2e0004d, 0xf0000000, 0x17c07c1f
};
static const pcm_desc_t pcm_suspend = {
    .base   = __pcm_suspend,
    .size   = 597,
    .sess   = 3,
    .vec0   = EVENT_VEC(11, 1, 0, 0),       /* 26M-wake event */
    .vec1   = EVENT_VEC(12, 1, 0, 31),      /* 26M-sleep event */
    .vec2   = EVENT_VEC(30, 1, 0, 65),      /* APSRC-wake event */
    .vec3   = EVENT_VEC(31, 1, 0, 109),     /* APSRC-sleep event */
};
#else
static const u32 __pcm_suspend[] = {
    0x19c0001f, 0x001c4bd7, 0x1800001f, 0x17cf0f3f, 0x1b80001f, 0x20000000,
    0x1800001f, 0x17cf0f16, 0x19c0001f, 0x001c4be7, 0xd80002c6, 0x17c07c1f,
    0x18c0001f, 0x10006234, 0xc0c01260, 0x1200041f, 0x18c0001f, 0x10006240,
    0xe0e00f16, 0xe0e00f1e, 0xe0e00f0e, 0xe0e00f0f, 0x1b00001f, 0x7fffd7ff,
    0xf0000000, 0x17c07c1f, 0x1b00001f, 0x3fffc7ff, 0x1b80001f, 0x20000004,
    0xd80006ac, 0x17c07c1f, 0xd8000566, 0x17c07c1f, 0x18c0001f, 0x10006240,
    0xe0e00f0f, 0xe0e00f1e, 0xe0e00f12, 0x18c0001f, 0x10006234, 0xc0c01440,
    0x17c07c1f, 0x1b00001f, 0x3fffcfff, 0x19c0001f, 0x001c6bd7, 0x1800001f,
    0x17cf0f3f, 0x1800001f, 0x17ff0f3f, 0x19c0001f, 0x001823d7, 0xf0000000,
    0x17c07c1f, 0x18c0001f, 0x10006294, 0xc0c014e0, 0x17c07c1f, 0x1800001f,
    0x07cf0f1e, 0x1b80001f, 0x20000a50, 0x1800001f, 0x07ce0f1e, 0x1b80001f,
    0x20000300, 0x1800001f, 0x078e0f1e, 0x1b80001f, 0x20000300, 0x1800001f,
    0x038e0f1e, 0x1b80001f, 0x20000300, 0x1800001f, 0x038e0e1e, 0x1800001f,
    0x038e0e12, 0x1b80001f, 0x200000ed, 0x18c0001f, 0x10006240, 0xe0e00f0d,
    0x1b80001f, 0x2000000e, 0x19c0001f, 0x000c4ba7, 0x19c0001f, 0x000c4ba5,
    0xe8208000, 0x10006354, 0xfffffa43, 0x19c0001f, 0x000d4ba5, 0x1b00001f,
    0xbfffc7ff, 0xf0000000, 0x17c07c1f, 0x1b80001f, 0x20000fdf, 0x8880000d,
    0x00000024, 0x1b00001f, 0xbfffc7ff, 0xd8001222, 0x17c07c1f, 0x1b00001f,
    0x3fffc7ff, 0x1b80001f, 0x20000004, 0xd800122c, 0x17c07c1f, 0xe8208000,
    0x10006354, 0xffffffff, 0x19c0001f, 0x001c4be5, 0x1880001f, 0x10006320,
    0xc0c017a0, 0xe080000f, 0xd8001223, 0x17c07c1f, 0xe080001f, 0xc0c018c0,
    0x17c07c1f, 0x18c0001f, 0x10006294, 0xe0f07ff0, 0xe0e00ff0, 0xe0e000f0,
    0xe8208000, 0x10006294, 0x000f00f0, 0x1800001f, 0x038e0e16, 0x1800001f,
    0x038e0f16, 0x1800001f, 0x07ce0f16, 0x1800001f, 0x17cf0f16, 0x1b00001f,
    0x7fffd7ff, 0xf0000000, 0x17c07c1f, 0xe0e00f16, 0x1380201f, 0xe0e00f1e,
    0x1380201f, 0xe0e00f0e, 0x1b80001f, 0x20000100, 0xe0e00f0f, 0xe0e00f0d,
    0xe0e00e0d, 0xe0e00c0d, 0xe0e0080d, 0xe0e0000d, 0xf0000000, 0x17c07c1f,
    0xe0e00f0d, 0xe0e00f1e, 0xe0e00f12, 0xf0000000, 0x17c07c1f, 0xe8208000,
    0x10006294, 0x000e00f0, 0xe8208000, 0x10006294, 0x000c00f0, 0xe8208000,
    0x10006294, 0x000800f0, 0xe8208000, 0x10006294, 0x000000f0, 0xe0e008f0,
    0xe0e00cf0, 0xe0e00ef0, 0xe0e00ff0, 0x1b80001f, 0x20000100, 0xe0f07ff0,
    0xe0f07f00, 0xf0000000, 0x17c07c1f, 0xa1d08407, 0x1b80001f, 0x200000ed,
    0x80eab401, 0x1a00001f, 0x10006814, 0xe2000003, 0xf0000000, 0x17c07c1f,
    0x18c0001f, 0x80000000, 0x1a10001f, 0x10002058, 0x1a80001f, 0x10002058,
    0xa2000c08, 0xe2800008, 0x1a10001f, 0x1000206c, 0x1a80001f, 0x1000206c,
    0xa2000c08, 0xe2800008, 0x1a10001f, 0x10002080, 0x1a80001f, 0x10002080,
    0xa2000c08, 0xe2800008, 0xf0000000, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x1840001f, 0x00000001,
    0xa1d48407, 0x1990001f, 0x10006400, 0x1a40001f, 0x11008000, 0x1b00001f,
    0x3fffc7ff, 0x1b80001f, 0xd00f0000, 0x8880000c, 0x3fffc7ff, 0xd8003fc2,
    0x1140041f, 0xe8208000, 0x10006354, 0xfffffa43, 0xc0c03a80, 0x81471801,
    0xd80025c5, 0x17c07c1f, 0x89c00007, 0xffffefff, 0x18c0001f, 0x10006200,
    0xc0c03bc0, 0x12807c1f, 0xe8208000, 0x1000625c, 0x00000001, 0x1b80001f,
    0x20000080, 0xc0c03bc0, 0x1280041f, 0x18c0001f, 0x10006208, 0xc0c03bc0,
    0x12807c1f, 0xe8208000, 0x10006248, 0x00000000, 0x1b80001f, 0x20000080,
    0xc0c03bc0, 0x1280041f, 0xc0c03b20, 0x81879801, 0x1b00001f, 0xffffdfff,
    0x1b80001f, 0x90010000, 0x8880000c, 0x3fffc7ff, 0xd8003962, 0x17c07c1f,
    0x8880000c, 0x40000800, 0xd8002602, 0x17c07c1f, 0x19c0001f, 0x00044b25,
    0x1880001f, 0x10006320, 0xe8208000, 0x10006354, 0xffffffff, 0xc0c017a0,
    0xe080000f, 0xd8002603, 0x17c07c1f, 0xe8208000, 0x10006310, 0x0b1600f8,
    0xe080001f, 0x19c0001f, 0x001c4be7, 0x1b80001f, 0x20000030, 0xc0c018c0,
    0x17c07c1f, 0xd8002ae6, 0x17c07c1f, 0x18c0001f, 0x10006240, 0xc0c01440,
    0x17c07c1f, 0x18c0001f, 0x10006294, 0xe0f07ff0, 0xe0e00ff0, 0xe0e000f0,
    0xe8208000, 0x10006294, 0x000f00f0, 0x1800001f, 0x00000036, 0x1800001f,
    0x00000f36, 0x1800001f, 0x07c00f36, 0x1800001f, 0x17cf0f36, 0xd8002da6,
    0x17c07c1f, 0x18c0001f, 0x10006234, 0xc0c01440, 0x17c07c1f, 0x19c0001f,
    0x001c6bd7, 0x1800001f, 0x17cf0f3f, 0x1800001f, 0x17ff0f3f, 0x19c0001f,
    0x001823d7, 0x1b00001f, 0x3fffcfff, 0x1b80001f, 0x90100000, 0x80c00400,
    0xd8003003, 0x80980400, 0xd8003302, 0x17c07c1f, 0xd8203782, 0x17c07c1f,
    0x19c0001f, 0x001c4bd7, 0x1800001f, 0x17cf0f3f, 0x1b80001f, 0x20000000,
    0x1800001f, 0x17cf0f16, 0x19c0001f, 0x001c4be7, 0xd8003206, 0x17c07c1f,
    0x18c0001f, 0x10006234, 0xc0c01260, 0x1200041f, 0xd8003306, 0x17c07c1f,
    0x18c0001f, 0x10006240, 0xe0e00f16, 0xe0e00f1e, 0xe0e00f0e, 0xe0e00f0f,
    0x18c0001f, 0x10006294, 0xc0c014e0, 0x17c07c1f, 0x19c0001f, 0x001c4ba7,
    0x1800001f, 0x07cf0f16, 0x1b80001f, 0x20000a50, 0x1800001f, 0x07c00f16,
    0x1b80001f, 0x20000300, 0x1800001f, 0x04000f16, 0x1b80001f, 0x20000300,
    0x1800001f, 0x00000f16, 0x1b80001f, 0x20000300, 0x1800001f, 0x00000016,
    0x10007c1f, 0x1b80001f, 0x2000049c, 0x1b80001f, 0x200000ed, 0x18c0001f,
    0x10006240, 0xe0e00f0d, 0x1b80001f, 0x2000000e, 0xd0003920, 0x17c07c1f,
    0x1800001f, 0x03800e12, 0x1b80001f, 0x20000300, 0x1800001f, 0x00000e12,
    0x1b80001f, 0x20000300, 0x1800001f, 0x00000012, 0x10007c1f, 0x1b80001f,
    0x2000079e, 0x19c0001f, 0x00054b25, 0xe8208000, 0x10006354, 0xfffffa43,
    0x19c0001f, 0x00014b25, 0x19c0001f, 0x00014a25, 0xd0003fc0, 0x17c07c1f,
    0xa1d10407, 0x1b80001f, 0x20000020, 0xf0000000, 0x17c07c1f, 0xa1d40407,
    0x1391841f, 0xa1d90407, 0xf0000000, 0x17c07c1f, 0xd8003c4a, 0x17c07c1f,
    0xe2e0006d, 0xe2e0002d, 0xd8203cea, 0x17c07c1f, 0xe2e0002f, 0xe2e0003e,
    0xe2e00032, 0xf0000000, 0x17c07c1f, 0xd0003fc0, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0xd8004345, 0x17c07c1f, 0x18c0001f, 0x10006208,
    0x1212841f, 0xc0c04860, 0x12807c1f, 0xe8208000, 0x10006248, 0x00000001,
    0x1b80001f, 0x20000080, 0xc0c04860, 0x1280041f, 0x18c0001f, 0x10006200,
    0x1212841f, 0xc0c04860, 0x12807c1f, 0xe8208000, 0x1000625c, 0x00000000,
    0x1b80001f, 0x20000080, 0xc0c04860, 0x1280041f, 0x19c0001f, 0x00415820,
    0x1ac0001f, 0x55aa55aa, 0x10007c1f, 0x80cab001, 0x808cb401, 0x80800c02,
    0xd82044a2, 0x17c07c1f, 0xa1d78407, 0x1240301f, 0xe8208000, 0x100063e0,
    0x00000001, 0x1b00001f, 0x00202000, 0x1b80001f, 0x80001000, 0x8880000c,
    0x00200000, 0xd80046c2, 0x17c07c1f, 0xe8208000, 0x100063e0, 0x00000002,
    0x1b80001f, 0x00001000, 0x809c840d, 0xd8204522, 0x17c07c1f, 0xa1d78407,
    0x1890001f, 0x10006014, 0x18c0001f, 0x10006014, 0xa0978402, 0xe0c00002,
    0x1b80001f, 0x00001000, 0xf0000000, 0xd800496a, 0x17c07c1f, 0xe2e00036,
    0x1380201f, 0xe2e0003e, 0x1380201f, 0xe2e0002e, 0x1380201f, 0xd8204a6a,
    0x17c07c1f, 0xe2e0006e, 0xe2e0004e, 0xe2e0004c, 0x1b80001f, 0x20000020,
    0xe2e0004d, 0xf0000000, 0x17c07c1f
};
static const pcm_desc_t pcm_suspend = {
    .base   = __pcm_suspend,
    .size   = 597,
    .sess   = 3,
    .vec0   = EVENT_VEC(11, 1, 0, 0),       /* 26M-wake event */
    .vec1   = EVENT_VEC(12, 1, 0, 26),      /* 26M-sleep event */
    .vec2   = EVENT_VEC(30, 1, 0, 55),      /* APSRC-wake event */
    .vec3   = EVENT_VEC(31, 1, 0, 99),      /* APSRC-sleep event */
};
#endif


/**********************************************************
 * PCM code for deep idle (v17rc3 @ 2013-05-21)
 **********************************************************/
#if SPM_CTRL_33VCORE || SPM_CTRL_33VRF18
static const u32 __pcm_dpidle[] = {
    0x80318400, 0xc0c01540, 0x10c0041f, 0xc0c01a60, 0x1280041f, 0x1800001f,
    0x17cf0f16, 0x1b00001f, 0x7ffff7ff, 0xf0000000, 0x17c07c1f, 0x1b00001f,
    0x3fffe7ff, 0x1b80001f, 0x20000004, 0xd80003cc, 0x17c07c1f, 0xc0c01a60,
    0x12807c1f, 0xc0c01540, 0x10c07c1f, 0x1800001f, 0x17cf0f36, 0x80c31801,
    0xd8200383, 0x17c07c1f, 0x1800001f, 0x17cf0f3e, 0x1b00001f, 0x3fffefff,
    0xf0000000, 0x17c07c1f, 0x19c0001f, 0x001c4ba7, 0x1b80001f, 0x20000030,
    0xe8208000, 0x10006354, 0xfffffbff, 0x1800001f, 0x07cf0f16, 0x1b80001f,
    0x20000a50, 0x1800001f, 0x07ce0f16, 0x1b80001f, 0x20000300, 0x1800001f,
    0x078e0f16, 0x1b80001f, 0x20000300, 0x1800001f, 0x038e0f16, 0x1b80001f,
    0x20000300, 0x1800001f, 0x038e0e16, 0x1800001f, 0x038e0e12, 0x19c0001f,
    0x000c4ba7, 0x19c0001f, 0x000c4ba5, 0xe8208000, 0x10006354, 0xfffffa43,
    0x19c0001f, 0x000d4ba5, 0x1b00001f, 0xbfffe7ff, 0xf0000000, 0x17c07c1f,
    0x1b80001f, 0x20000fdf, 0x8880000d, 0x00000024, 0x1b00001f, 0xbfffe7ff,
    0xd8000dc2, 0x17c07c1f, 0x1b00001f, 0x3fffe7ff, 0x1b80001f, 0x20000004,
    0xd8000dcc, 0x17c07c1f, 0xe8208000, 0x10006354, 0xfffffbff, 0x19c0001f,
    0x001c4be5, 0x1880001f, 0x10006320, 0xc0c01420, 0xe080000f, 0xd8000dc3,
    0x17c07c1f, 0xe080001f, 0xc0c01700, 0x17c07c1f, 0x1800001f, 0x038e0e16,
    0x1800001f, 0x038e0f16, 0x1800001f, 0x07ce0f16, 0x1800001f, 0x17cf0f16,
    0x1b00001f, 0x7ffff7ff, 0xf0000000, 0x17c07c1f, 0xe0e00f16, 0x1380201f,
    0xe0e00f1e, 0x1380201f, 0xe0e00f0e, 0x1380201f, 0xe0e00f0c, 0xe0e00f0d,
    0xe0e00e0d, 0xe0e00c0d, 0xe0e0080d, 0xe0e0000d, 0xf0000000, 0x17c07c1f,
    0xd80010ca, 0x17c07c1f, 0xe2e00036, 0x1380201f, 0xe2e0003e, 0x1380201f,
    0xe2e0002e, 0x1380201f, 0xd82011ca, 0x17c07c1f, 0xe2e0006e, 0xe2e0004e,
    0xe2e0004c, 0x1b80001f, 0x20000020, 0xe2e0004d, 0xf0000000, 0x17c07c1f,
    0xd800128a, 0x17c07c1f, 0xe2e0006d, 0xe2e0002d, 0xd820132a, 0x17c07c1f,
    0xe2e0002f, 0xe2e0003e, 0xe2e00032, 0xf0000000, 0x17c07c1f, 0xa1d10407,
    0x1b80001f, 0x20000080, 0x10c07c1f, 0xf0000000, 0x17c07c1f, 0xa1d08407,
    0x1b80001f, 0x200000ed, 0x80eab401, 0x1a00001f, 0x10006814, 0xe2000003,
    0xf0000000, 0x17c07c1f, 0x1a00001f, 0x10006604, 0xd8001663, 0x17c07c1f,
    0xe2200006, 0x1b80001f, 0x20000020, 0xd82016c3, 0x17c07c1f, 0xe2200005,
    0x1b80001f, 0x20000020, 0xf0000000, 0x17c07c1f, 0x18c0001f, 0x80000000,
    0x1a10001f, 0x10002058, 0x1a80001f, 0x10002058, 0xa2000c08, 0xe2800008,
    0x1a10001f, 0x1000206c, 0x1a80001f, 0x1000206c, 0xa2000c08, 0xe2800008,
    0x1a10001f, 0x10002080, 0x1a80001f, 0x10002080, 0xa2000c08, 0xe2800008,
    0xf0000000, 0x17c07c1f, 0xa1d40407, 0x1391841f, 0xa1d90407, 0xf0000000,
    0x17c07c1f, 0x1910001f, 0x10006600, 0x18c0001f, 0x00000007, 0x80819003,
    0x80c01003, 0xe8208000, 0x11008014, 0x00000002, 0xe8208000, 0x11008020,
    0x00000101, 0xe8208000, 0x11008004, 0x000000d6, 0xd8001cca, 0xe26000a0,
    0xd8201cea, 0xe2400002, 0xe2400003, 0xe8208000, 0x11008024, 0x00000001,
    0x1b80001f, 0x20000424, 0xf0000000, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x1840001f, 0x00000001,
    0xa1d48407, 0x1a40001f, 0x11008000, 0x1b00001f, 0x3fffe7ff, 0x1b80001f,
    0xd00f0000, 0x8880000c, 0x3fffe7ff, 0xd8003762, 0x1140041f, 0xe8208000,
    0x10006354, 0xfffffa43, 0xc0c01360, 0x17c07c1f, 0x1950001f, 0x10006400,
    0x80d70405, 0xd80025e3, 0x17c07c1f, 0x89c00007, 0xffffefff, 0x18c0001f,
    0x10006200, 0xc0c01200, 0x12807c1f, 0xe8208000, 0x1000625c, 0x00000001,
    0x1b80001f, 0x20000080, 0xc0c01200, 0x1280041f, 0x18c0001f, 0x10006208,
    0xc0c01200, 0x12807c1f, 0xe8208000, 0x10006248, 0x00000000, 0x1b80001f,
    0x20000080, 0xc0c01200, 0x1280041f, 0xc0c019c0, 0x17c07c1f, 0x1b00001f,
    0xffffffff, 0x1b80001f, 0xd0010000, 0x8880000c, 0x3fffe7ff, 0xd80033a2,
    0x17c07c1f, 0x8880000c, 0x40000000, 0xd8002622, 0x17c07c1f, 0x8083b401,
    0xd8002622, 0x17c07c1f, 0x1880001f, 0x10006320, 0x1990001f, 0x10006600,
    0xe8208000, 0x10006354, 0xfffffbff, 0xc0c01420, 0xe080000f, 0xd8002623,
    0x17c07c1f, 0xe8208000, 0x10006310, 0x0b1600f8, 0xe080001f, 0x19c0001f,
    0x001c4be7, 0x1b80001f, 0x20000030, 0xc0c01700, 0x17c07c1f, 0x1800001f,
    0x00000016, 0x1800001f, 0x00000f16, 0x1800001f, 0x07c00f16, 0x1800001f,
    0x17cf0f16, 0x8080b401, 0xd8002c82, 0x17c07c1f, 0xc0c01a60, 0x12807c1f,
    0xc0c01540, 0x10c07c1f, 0x80c31801, 0xd8202d23, 0x17c07c1f, 0x1800001f,
    0x17cf0f1e, 0x1b00001f, 0x3fffefff, 0x1b80001f, 0x90100000, 0x80881c01,
    0xd80031c2, 0x17c07c1f, 0x80318400, 0xc0c01540, 0x10c0041f, 0xc0c01a60,
    0x1280041f, 0x1800001f, 0x07cf0f16, 0x1b80001f, 0x20000a50, 0x1800001f,
    0x07c00f16, 0x1b80001f, 0x20000300, 0x1800001f, 0x04000f16, 0x1b80001f,
    0x20000300, 0x1800001f, 0x00000f16, 0x1b80001f, 0x20000300, 0x1800001f,
    0x00000016, 0x10007c1f, 0x1b80001f, 0x2000049c, 0x19c0001f, 0x00044b25,
    0xd82033a2, 0x17c07c1f, 0x1800001f, 0x03800e12, 0x1b80001f, 0x20000300,
    0x1800001f, 0x00000e12, 0x1b80001f, 0x20000300, 0x1800001f, 0x00000012,
    0x10007c1f, 0x1b80001f, 0x2000079e, 0x19c0001f, 0x00054b25, 0x19c0001f,
    0x00014b25, 0x19c0001f, 0x00014a25, 0x80d70405, 0xd8003763, 0x17c07c1f,
    0x18c0001f, 0x10006208, 0x1212841f, 0xc0c00fc0, 0x12807c1f, 0xe8208000,
    0x10006248, 0x00000001, 0x1b80001f, 0x20000080, 0xc0c00fc0, 0x1280041f,
    0x18c0001f, 0x10006200, 0xc0c00fc0, 0x12807c1f, 0xe8208000, 0x1000625c,
    0x00000000, 0x1b80001f, 0x20000080, 0xc0c00fc0, 0x1280041f, 0x19c0001f,
    0x00015820, 0x1ac0001f, 0x55aa55aa, 0x10007c1f, 0x80cab001, 0x808cb401,
    0x80800c02, 0xd82038c2, 0x17c07c1f, 0xa1d78407, 0x1240301f, 0xe8208000,
    0x100063e0, 0x00000001, 0x1b00001f, 0x00202000, 0x1b80001f, 0x80001000,
    0x8880000c, 0x00200000, 0xd8003ae2, 0x17c07c1f, 0xe8208000, 0x100063e0,
    0x00000002, 0x1b80001f, 0x00001000, 0x809c840d, 0xd8203942, 0x17c07c1f,
    0xa1d78407, 0x1890001f, 0x10006014, 0x18c0001f, 0x10006014, 0xa0978402,
    0xe0c00002, 0x1b80001f, 0x00001000, 0xf0000000
};
static const pcm_desc_t pcm_dpidle = {
    .base   = __pcm_dpidle,
    .size   = 484,
    .sess   = 2,
    .vec0   = EVENT_VEC(11, 1, 0, 0),       /* 26M-wake event */
    .vec1   = EVENT_VEC(12, 1, 0, 11),      /* 26M-sleep event */
    .vec2   = EVENT_VEC(30, 1, 0, 32),      /* APSRC-wake event */
    .vec3   = EVENT_VEC(31, 1, 0, 72),      /* APSRC-sleep event */
};
#else
static const u32 __pcm_dpidle[] = {
    0x80318400, 0xc0c014c0, 0x10c0041f, 0x1800001f, 0x17cf0f16, 0x1b00001f,
    0x7ffff7ff, 0xf0000000, 0x17c07c1f, 0x1b00001f, 0x3fffe7ff, 0x1b80001f,
    0x20000004, 0xd800034c, 0x17c07c1f, 0xc0c014c0, 0x10c07c1f, 0x1800001f,
    0x17cf0f36, 0x80c31801, 0xd8200303, 0x17c07c1f, 0x1800001f, 0x17cf0f3e,
    0x1b00001f, 0x3fffefff, 0xf0000000, 0x17c07c1f, 0x19c0001f, 0x001c4ba7,
    0x1b80001f, 0x20000030, 0xe8208000, 0x10006354, 0xfffffbff, 0x1800001f,
    0x07cf0f16, 0x1b80001f, 0x20000a50, 0x1800001f, 0x07ce0f16, 0x1b80001f,
    0x20000300, 0x1800001f, 0x078e0f16, 0x1b80001f, 0x20000300, 0x1800001f,
    0x038e0f16, 0x1b80001f, 0x20000300, 0x1800001f, 0x038e0e16, 0x1800001f,
    0x038e0e12, 0x19c0001f, 0x000c4ba7, 0x19c0001f, 0x000c4ba5, 0xe8208000,
    0x10006354, 0xfffffa43, 0x19c0001f, 0x000d4ba5, 0x1b00001f, 0xbfffe7ff,
    0xf0000000, 0x17c07c1f, 0x1b80001f, 0x20000fdf, 0x8880000d, 0x00000024,
    0x1b00001f, 0xbfffe7ff, 0xd8000d42, 0x17c07c1f, 0x1b00001f, 0x3fffe7ff,
    0x1b80001f, 0x20000004, 0xd8000d4c, 0x17c07c1f, 0xe8208000, 0x10006354,
    0xfffffbff, 0x19c0001f, 0x001c4be5, 0x1880001f, 0x10006320, 0xc0c013a0,
    0xe080000f, 0xd8000d43, 0x17c07c1f, 0xe080001f, 0xc0c01680, 0x17c07c1f,
    0x1800001f, 0x038e0e16, 0x1800001f, 0x038e0f16, 0x1800001f, 0x07ce0f16,
    0x1800001f, 0x17cf0f16, 0x1b00001f, 0x7ffff7ff, 0xf0000000, 0x17c07c1f,
    0xe0e00f16, 0x1380201f, 0xe0e00f1e, 0x1380201f, 0xe0e00f0e, 0x1380201f,
    0xe0e00f0c, 0xe0e00f0d, 0xe0e00e0d, 0xe0e00c0d, 0xe0e0080d, 0xe0e0000d,
    0xf0000000, 0x17c07c1f, 0xd800104a, 0x17c07c1f, 0xe2e00036, 0x1380201f,
    0xe2e0003e, 0x1380201f, 0xe2e0002e, 0x1380201f, 0xd820114a, 0x17c07c1f,
    0xe2e0006e, 0xe2e0004e, 0xe2e0004c, 0x1b80001f, 0x20000020, 0xe2e0004d,
    0xf0000000, 0x17c07c1f, 0xd800120a, 0x17c07c1f, 0xe2e0006d, 0xe2e0002d,
    0xd82012aa, 0x17c07c1f, 0xe2e0002f, 0xe2e0003e, 0xe2e00032, 0xf0000000,
    0x17c07c1f, 0xa1d10407, 0x1b80001f, 0x20000080, 0x10c07c1f, 0xf0000000,
    0x17c07c1f, 0xa1d08407, 0x1b80001f, 0x200000ed, 0x80eab401, 0x1a00001f,
    0x10006814, 0xe2000003, 0xf0000000, 0x17c07c1f, 0x1a00001f, 0x10006604,
    0xd80015e3, 0x17c07c1f, 0xe2200006, 0x1b80001f, 0x20000020, 0xd8201643,
    0x17c07c1f, 0xe2200005, 0x1b80001f, 0x20000020, 0xf0000000, 0x17c07c1f,
    0x18c0001f, 0x80000000, 0x1a10001f, 0x10002058, 0x1a80001f, 0x10002058,
    0xa2000c08, 0xe2800008, 0x1a10001f, 0x1000206c, 0x1a80001f, 0x1000206c,
    0xa2000c08, 0xe2800008, 0x1a10001f, 0x10002080, 0x1a80001f, 0x10002080,
    0xa2000c08, 0xe2800008, 0xf0000000, 0x17c07c1f, 0xa1d40407, 0x1391841f,
    0xa1d90407, 0xf0000000, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f,
    0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x17c07c1f, 0x1840001f, 0x00000001,
    0xa1d48407, 0x1a40001f, 0x11008000, 0x1b00001f, 0x3fffe7ff, 0x1b80001f,
    0xd00f0000, 0x8880000c, 0x3fffe7ff, 0xd80036e2, 0x1140041f, 0xe8208000,
    0x10006354, 0xfffffa43, 0xc0c012e0, 0x17c07c1f, 0x1950001f, 0x10006400,
    0x80d70405, 0xd80025e3, 0x17c07c1f, 0x89c00007, 0xffffefff, 0x18c0001f,
    0x10006200, 0xc0c01180, 0x12807c1f, 0xe8208000, 0x1000625c, 0x00000001,
    0x1b80001f, 0x20000080, 0xc0c01180, 0x1280041f, 0x18c0001f, 0x10006208,
    0xc0c01180, 0x12807c1f, 0xe8208000, 0x10006248, 0x00000000, 0x1b80001f,
    0x20000080, 0xc0c01180, 0x1280041f, 0xc0c01940, 0x17c07c1f, 0x1b00001f,
    0xffffffff, 0x1b80001f, 0xd0010000, 0x8880000c, 0x3fffe7ff, 0xd8003322,
    0x17c07c1f, 0x8880000c, 0x40000000, 0xd8002622, 0x17c07c1f, 0x8083b401,
    0xd8002622, 0x17c07c1f, 0x1880001f, 0x10006320, 0x1990001f, 0x10006600,
    0xe8208000, 0x10006354, 0xfffffbff, 0xc0c013a0, 0xe080000f, 0xd8002623,
    0x17c07c1f, 0xe8208000, 0x10006310, 0x0b1600f8, 0xe080001f, 0x19c0001f,
    0x001c4be7, 0x1b80001f, 0x20000030, 0xc0c01680, 0x17c07c1f, 0x1800001f,
    0x00000016, 0x1800001f, 0x00000f16, 0x1800001f, 0x07c00f16, 0x1800001f,
    0x17cf0f16, 0x8080b401, 0xd8002c42, 0x17c07c1f, 0xc0c014c0, 0x10c07c1f,
    0x80c31801, 0xd8202ce3, 0x17c07c1f, 0x1800001f, 0x17cf0f1e, 0x1b00001f,
    0x3fffefff, 0x1b80001f, 0x90100000, 0x80881c01, 0xd8003142, 0x17c07c1f,
    0x80318400, 0xc0c014c0, 0x10c0041f, 0x1800001f, 0x07cf0f16, 0x1b80001f,
    0x20000a50, 0x1800001f, 0x07c00f16, 0x1b80001f, 0x20000300, 0x1800001f,
    0x04000f16, 0x1b80001f, 0x20000300, 0x1800001f, 0x00000f16, 0x1b80001f,
    0x20000300, 0x1800001f, 0x00000016, 0x10007c1f, 0x1b80001f, 0x2000049c,
    0x19c0001f, 0x00044b25, 0xd8203322, 0x17c07c1f, 0x1800001f, 0x03800e12,
    0x1b80001f, 0x20000300, 0x1800001f, 0x00000e12, 0x1b80001f, 0x20000300,
    0x1800001f, 0x00000012, 0x10007c1f, 0x1b80001f, 0x2000079e, 0x19c0001f,
    0x00054b25, 0x19c0001f, 0x00014b25, 0x19c0001f, 0x00014a25, 0x80d70405,
    0xd80036e3, 0x17c07c1f, 0x18c0001f, 0x10006208, 0x1212841f, 0xc0c00f40,
    0x12807c1f, 0xe8208000, 0x10006248, 0x00000001, 0x1b80001f, 0x20000080,
    0xc0c00f40, 0x1280041f, 0x18c0001f, 0x10006200, 0xc0c00f40, 0x12807c1f,
    0xe8208000, 0x1000625c, 0x00000000, 0x1b80001f, 0x20000080, 0xc0c00f40,
    0x1280041f, 0x19c0001f, 0x00015820, 0x1ac0001f, 0x55aa55aa, 0x10007c1f,
    0x80cab001, 0x808cb401, 0x80800c02, 0xd8203842, 0x17c07c1f, 0xa1d78407,
    0x1240301f, 0xe8208000, 0x100063e0, 0x00000001, 0x1b00001f, 0x00202000,
    0x1b80001f, 0x80001000, 0x8880000c, 0x00200000, 0xd8003a62, 0x17c07c1f,
    0xe8208000, 0x100063e0, 0x00000002, 0x1b80001f, 0x00001000, 0x809c840d,
    0xd82038c2, 0x17c07c1f, 0xa1d78407, 0x1890001f, 0x10006014, 0x18c0001f,
    0x10006014, 0xa0978402, 0xe0c00002, 0x1b80001f, 0x00001000, 0xf0000000
};
static const pcm_desc_t pcm_dpidle = {
    .base   = __pcm_dpidle,
    .size   = 480,
    .sess   = 2,
    .vec0   = EVENT_VEC(11, 1, 0, 0),       /* 26M-wake event */
    .vec1   = EVENT_VEC(12, 1, 0, 9),       /* 26M-sleep event */
    .vec2   = EVENT_VEC(30, 1, 0, 28),      /* APSRC-wake event */
    .vec3   = EVENT_VEC(31, 1, 0, 68),      /* APSRC-sleep event */
};
#endif


/**************************************
 * SW code for suspend and deep idle
 **************************************/
#define SPM_SYSCLK_SETTLE       99      /* 3ms */

#define WAIT_UART_ACK_TIMES     10      /* 10 * 10us */

#define SPM_WAKE_PERIOD         600     /* sec */

#define PCM_WDT_TIMEOUT         (30 * 32768)    /* 30s */
#define PCM_TIMER_MAX_FOR_WDT   (0xffffffff - PCM_WDT_TIMEOUT)

#define WAKE_SRC_FOR_SUSPEND                                                    \
    (WAKE_SRC_KP | WAKE_SRC_EINT | WAKE_SRC_CONN_WDT | WAKE_SRC_CCIF_MD |       \
     WAKE_SRC_CONN | WAKE_SRC_USB_CD | WAKE_SRC_THERM | WAKE_SRC_SYSPWREQ |     \
     WAKE_SRC_MD_WDT)

#define WAKE_SRC_FOR_DPIDLE                                                     \
    (WAKE_SRC_KP | WAKE_SRC_GPT | WAKE_SRC_EINT | WAKE_SRC_CONN_WDT |           \
     WAKE_SRC_CCIF_MD | WAKE_SRC_CONN | WAKE_SRC_USB_CD | WAKE_SRC_USB_PDN |    \
     WAKE_SRC_AFE | WAKE_SRC_THERM | WAKE_SRC_SYSPWREQ | WAKE_SRC_MD_WDT)

#define wfi_with_sync()                         \
do {                                            \
    isb();                                      \
    dsb();                                      \
    __asm__ __volatile__("wfi" : : : "memory"); \
} while (0)

#define spm_crit2(fmt, args...)     \
do {                                \
    aee_sram_printk(fmt, ##args);   \
    spm_crit(fmt, ##args);          \
} while (0)

#define spm_error2(fmt, args...)    \
do {                                \
    aee_sram_printk(fmt, ##args);   \
    spm_error(fmt, ##args);         \
} while (0)

typedef struct {
    u32 debug_reg;      /* PCM_REG_DATA_INI */
    u32 r12;            /* PCM_REG12_DATA */
    u32 raw_sta;        /* SLEEP_ISR_RAW_STA */
    u32 cpu_wake;       /* SLEEP_CPU_WAKEUP_EVENT */
    u32 timer_out;      /* PCM_TIMER_OUT */
    u32 event_reg;      /* PCM_EVENT_REG_STA */
    u32 isr;            /* SLEEP_ISR_STATUS */
    u32 r13;            /* PCM_REG13_DATA */
} wake_status_t;

extern int get_dynamic_period(int first_use, int first_wakeup_time, int battery_capacity_level);

extern int mt_irq_mask_all(struct mtk_irq_mask *mask);
extern int mt_irq_mask_restore(struct mtk_irq_mask *mask);
extern void mt_irq_unmask_for_sleep(unsigned int irq);

extern void mtk_uart_restore(void);
extern void dump_uart_reg(void);

extern spinlock_t spm_lock;

static u32 spm_sleep_wakesrc = WAKE_SRC_FOR_SUSPEND;

static void spm_set_sysclk_settle(void)
{
    u32 md_settle, settle;

    /* get MD SYSCLK settle */
    spm_write(SPM_CLK_CON, spm_read(SPM_CLK_CON) | CC_SYSSETTLE_SEL);
    spm_write(SPM_CLK_SETTLE, 0);
    md_settle = spm_read(SPM_CLK_SETTLE);

    /* SYSCLK settle = MD SYSCLK settle but set it again for MD PDN */
    spm_write(SPM_CLK_SETTLE, SPM_SYSCLK_SETTLE - md_settle);
    settle = spm_read(SPM_CLK_SETTLE);

    spm_crit2("md_settle = %u, settle = %u\n", md_settle, settle);
}

static void spm_reset_and_init_pcm(void)
{
    u32 con1;

    /* reset PCM */
    spm_write(SPM_PCM_CON0, CON0_CFG_KEY | CON0_PCM_SW_RESET);
    spm_write(SPM_PCM_CON0, CON0_CFG_KEY);

    /* init PCM_CON0 (disable event vector) */
    spm_write(SPM_PCM_CON0, CON0_CFG_KEY | CON0_IM_SLEEP_DVS);

    /* init PCM_CON1 (disable PCM timer but keep PCM WDT setting) */
    con1 = spm_read(SPM_PCM_CON1) & (CON1_PCM_WDT_WAKE_MODE | CON1_PCM_WDT_EN);
    spm_write(SPM_PCM_CON1, con1 | CON1_CFG_KEY | CON1_SPM_SRAM_ISO_B |
                            CON1_SPM_SRAM_SLP_B | CON1_IM_NONRP_EN | CON1_MIF_APBEN);
}

/*
 * pcmdesc: pcm_suspend or pcm_dpidle
 */
static void spm_kick_im_to_fetch(const pcm_desc_t *pcmdesc)
{
    u32 ptr, len, con0;

    /* tell IM where is PCM code (use slave mode if code existed and session <= 2) */
    ptr = spm_get_base_phys(pcmdesc->base);
    len = pcmdesc->size - 1;
    if (spm_read(SPM_PCM_IM_PTR) != ptr || spm_read(SPM_PCM_IM_LEN) != len ||
        pcmdesc->sess > 2) {
        spm_write(SPM_PCM_IM_PTR, ptr);
        spm_write(SPM_PCM_IM_LEN, len);
    } else {
        spm_write(SPM_PCM_CON1, spm_read(SPM_PCM_CON1) | CON1_CFG_KEY | CON1_IM_SLAVE);
    }

    /* kick IM to fetch (only toggle IM_KICK) */
    con0 = spm_read(SPM_PCM_CON0) & ~(CON0_IM_KICK | CON0_PCM_KICK);
    spm_write(SPM_PCM_CON0, con0 | CON0_CFG_KEY | CON0_IM_KICK);
    spm_write(SPM_PCM_CON0, con0 | CON0_CFG_KEY);
}

static int spm_request_uart_to_sleep(void)
{
    u32 val1;
    int i = 0;

    /* request UART to sleep */
    val1 = spm_read(SPM_POWER_ON_VAL1);
    spm_write(SPM_POWER_ON_VAL1, val1 | R7_UART_CLK_OFF_REQ);

    /* wait for UART to ACK */
    while (!(spm_read(SPM_PCM_REG13_DATA) & R13_UART_CLK_OFF_ACK)) {
        if (i++ >= WAIT_UART_ACK_TIMES) {
            spm_write(SPM_POWER_ON_VAL1, val1);
            spm_error2("CANNOT GET UART SLEEP ACK (0x%x)\n", spm_read(PERI_PDN0_STA));
            dump_uart_reg();
            return -EBUSY;
        }
        udelay(10);
    }

    return 0;
}

static void spm_init_pcm_register(void)
{
    /* init r0 with POWER_ON_VAL0 */
    spm_write(SPM_PCM_REG_DATA_INI, spm_read(SPM_POWER_ON_VAL0));
    spm_write(SPM_PCM_PWR_IO_EN, PCM_RF_SYNC_R0);
    spm_write(SPM_PCM_PWR_IO_EN, 0);

    /* init r7 with POWER_ON_VAL1 */
    spm_write(SPM_PCM_REG_DATA_INI, spm_read(SPM_POWER_ON_VAL1));
    spm_write(SPM_PCM_PWR_IO_EN, PCM_RF_SYNC_R7);
    spm_write(SPM_PCM_PWR_IO_EN, 0);

    /* clear REG_DATA_INI for PCM after init rX */
    spm_write(SPM_PCM_REG_DATA_INI, 0);
}

/*
 * pcmdesc: pcm_suspend or pcm_dpidle
 */
static void spm_init_event_vector(const pcm_desc_t *pcmdesc)
{
    /* init event vector register */
    spm_write(SPM_PCM_EVENT_VECTOR0, pcmdesc->vec0);
    spm_write(SPM_PCM_EVENT_VECTOR1, pcmdesc->vec1);
    spm_write(SPM_PCM_EVENT_VECTOR2, pcmdesc->vec2);
    spm_write(SPM_PCM_EVENT_VECTOR3, pcmdesc->vec3);
    spm_write(SPM_PCM_EVENT_VECTOR4, pcmdesc->vec4);
    spm_write(SPM_PCM_EVENT_VECTOR5, pcmdesc->vec5);
    spm_write(SPM_PCM_EVENT_VECTOR6, pcmdesc->vec6);
    spm_write(SPM_PCM_EVENT_VECTOR7, pcmdesc->vec7);

    /* event vector will be enabled by PCM itself */
}

static void spm_set_pwrctl_for_sleep(void)
{
    u32 pwrctl = 0;

    /* [5:3]=VRF18_CON22 sleep_val, [2:0]=VRF18_CON22 wakeup_val */
#if SPM_CTRL_33VRF18
    pwrctl |= (0x0 << 3) | (0x3 << 0);
#endif
    spm_write(SPM_APMCU_PWRCTL, pwrctl);

    spm_write(SPM_AP_STANBY_CON, (0x3 << 19) |  /* unmask MD and CONN */
                                 (0 << 16) |    /* mask DISP and MFG */
                                 (0 << 6) |     /* check SCU idle */
                                 (0 << 5) |     /* check L2C idle */
                                 (1U << 4));    /* Reduce AND */
    spm_write(SPM_CORE0_WFI_SEL, 0x1);
    spm_write(SPM_CORE1_WFI_SEL, 0x1);
    spm_write(SPM_CORE2_WFI_SEL, 0x1);
    spm_write(SPM_CORE3_WFI_SEL, 0x1);
}

static void spm_set_pwrctl_for_dpidle(u16 pwrlevel)
{
    u32 pwrctl = 0;

    /* [7:6]=dpidle level, [5:3]=VRF18_CON22 sleep_val, [2:0]=VRF18_CON22 wakeup_val */
    if (pwrlevel == 0)
        pwrctl |= (1U << 6);
    else
        pwrctl |= (1U << 7);

#if SPM_CTRL_33VCORE && SPM_CTRL_33VRF18
    pwrctl |= (0x4 << 3) | (0x3 << 0);
#elif SPM_CTRL_33VCORE && !SPM_CTRL_33VRF18
    pwrctl |= (0x4 << 3) | (0x0 << 0);
#elif !SPM_CTRL_33VCORE && SPM_CTRL_33VRF18
    pwrctl |= (0x0 << 3) | (0x3 << 0);
#endif
    spm_write(SPM_APMCU_PWRCTL, pwrctl);

    spm_write(SPM_AP_STANBY_CON, (0x3 << 19) |  /* unmask MD and CONN */
                                 (0 << 16) |    /* mask DISP and MFG */
                                 (0 << 6) |     /* check SCU idle */
                                 (0 << 5) |     /* check L2C idle */
                                 (1U << 4));    /* Reduce AND */
    spm_write(SPM_CORE0_WFI_SEL, 0x1);
    spm_write(SPM_CORE1_WFI_SEL, 0x1);
    spm_write(SPM_CORE2_WFI_SEL, 0x1);
    spm_write(SPM_CORE3_WFI_SEL, 0x1);
}

/*
 * timer_val: PCM timer value (0 = disable)
 * wake_src : WAKE_SRC_XXX
 */
static void spm_set_wakeup_event(u32 timer_val, u32 wake_src)
{
    u32 isr;

    /* set PCM timer (set to max when disable) */
    spm_write(SPM_PCM_TIMER_VAL, timer_val ? : PCM_TIMER_MAX_FOR_WDT);
    spm_write(SPM_PCM_CON1, spm_read(SPM_PCM_CON1) | CON1_CFG_KEY | CON1_PCM_TIMER_EN);

    /* unmask wakeup source */
#if SPM_BYPASS_SYSPWREQ
    wake_src &= ~WAKE_SRC_SYSPWREQ;     /* make 26M off when attach ICE */
#endif
    spm_write(SPM_SLEEP_WAKEUP_EVENT_MASK, ~wake_src);

    /* unmask SPM ISR (keep TWAM setting) */
    isr = spm_read(SPM_SLEEP_ISR_MASK) & ISR_TWAM;
    spm_write(SPM_SLEEP_ISR_MASK, isr | ISRM_PCM_IRQ_AUX);
}

static void spm_kick_pcm_to_run(bool cpu_pdn, bool infra_pdn, bool pcmwdt_en)
{
    u32 clk, con0;

    /* keep CPU or INFRA/DDRPHY power if needed and lock INFRA DCM */
    clk = spm_read(SPM_CLK_CON) & ~(CC_DISABLE_DORM_PWR | CC_DISABLE_INFRA_PWR);
    if (!cpu_pdn)
        clk |= CC_DISABLE_DORM_PWR;
    if (!infra_pdn)
        clk |= CC_DISABLE_INFRA_PWR;
    spm_write(SPM_CLK_CON, clk | CC_LOCK_INFRA_DCM);

    /* init pause request mask for PCM */
    spm_write(SPM_PCM_MAS_PAUSE_MASK, 0xffffffff);

    /* enable r0 and r7 to control power */
    spm_write(SPM_PCM_PWR_IO_EN, PCM_PWRIO_EN_R0 | PCM_PWRIO_EN_R7);

    /* SRCLKENA: r7 (PWR_IO_EN[7]=1) */
    spm_write(SPM_CLK_CON, spm_read(SPM_CLK_CON) | CC_SRCLKENA_MASK);

    /* enable PCM WDT (normal mode) to start count if needed */
#if SPM_PCMWDT_EN
    if (pcmwdt_en) {
        u32 con1;
        con1 = spm_read(SPM_PCM_CON1) & ~(CON1_PCM_WDT_WAKE_MODE | CON1_PCM_WDT_EN);
        spm_write(SPM_PCM_CON1, CON1_CFG_KEY | con1);

        BUG_ON(spm_read(SPM_PCM_TIMER_VAL) > PCM_TIMER_MAX_FOR_WDT);
        spm_write(SPM_PCM_WDT_TIMER_VAL, spm_read(SPM_PCM_TIMER_VAL) + PCM_WDT_TIMEOUT);
        spm_write(SPM_PCM_CON1, con1 | CON1_CFG_KEY | CON1_PCM_WDT_EN);
    }
#endif

    /* kick PCM to run (only toggle PCM_KICK) */
    con0 = spm_read(SPM_PCM_CON0) & ~(CON0_IM_KICK | CON0_PCM_KICK);
    spm_write(SPM_PCM_CON0, con0 | CON0_CFG_KEY | CON0_PCM_KICK);
    spm_write(SPM_PCM_CON0, con0 | CON0_CFG_KEY);
}

static void spm_trigger_wfi_for_sleep(bool cpu_pdn, bool infra_pdn)
{
#if SPM_CTRL_33VRF18
    u32 pdn0;
    pdn0 = spm_read(PERI_PDN0_STA) & (1U << 22);
    spm_write(PERI_PDN0_CLR, pdn0);     /* power on I2C1 */
    spm_write(0xF1008010, 0);           /* init I2C1_CONTROL */
    spm_write(0xF1008018, 0x1);         /* init I2C1_TRANSAC_LEN */
    spm_write(0xF1008028, 0x1800);      /* init I2C1_EXT_CONF */
    spm_write(0xF1008040, 0x3);         /* init I2C1_IO_CONFIG */
    spm_write(0xF1008048, 0x102);       /* init I2C1_HS */
#endif

    if (cpu_pdn) {
        if (!cpu_power_down(SHUTDOWN_MODE)) {
            switch_to_amp();
            wfi_with_sync();
        }
        switch_to_smp();
        cpu_check_dormant_abort();
    } else {
        wfi_with_sync();
    }

#if SPM_CTRL_33VRF18
    spm_write(PERI_PDN0_SET, pdn0);     /* restore I2C1 power */
#endif

    if (infra_pdn)
        mtk_uart_restore();
}

static void spm_trigger_wfi_for_dpidle(bool cpu_pdn)
{
#if SPM_CTRL_33VCORE || SPM_CTRL_33VRF18
    u32 pdn0;
    pdn0 = spm_read(PERI_PDN0_STA) & (1U << 22);
    spm_write(PERI_PDN0_CLR, pdn0);     /* power on I2C1 */
    spm_write(0xF1008010, 0);           /* init I2C1_CONTROL */
    spm_write(0xF1008018, 0x1);         /* init I2C1_TRANSAC_LEN */
    spm_write(0xF1008028, 0x1800);      /* init I2C1_EXT_CONF */
    spm_write(0xF1008040, 0x3);         /* init I2C1_IO_CONFIG */
    spm_write(0xF1008048, 0x102);       /* init I2C1_HS */
#endif

    if (cpu_pdn) {
        if (!cpu_power_down(DORMANT_MODE)) {
            switch_to_amp();
            wfi_with_sync();
        }
        switch_to_smp();
        cpu_check_dormant_abort();
    } else {
        wfi_with_sync();
    }

#if SPM_CTRL_33VCORE || SPM_CTRL_33VRF18
    spm_write(PERI_PDN0_SET, pdn0);     /* restore I2C1 power */
#endif
}

static void spm_get_wakeup_status(wake_status_t *wakesta)
{
    /* get PC value if PCM assert (pause abort) */
    wakesta->debug_reg = spm_read(SPM_PCM_REG_DATA_INI);

    /* get wakeup event */
    wakesta->r12 = spm_read(SPM_PCM_REG9_DATA);     /* r9 = r12 for pcm_normal */
    wakesta->raw_sta = spm_read(SPM_SLEEP_ISR_RAW_STA);
    wakesta->cpu_wake = spm_read(SPM_SLEEP_CPU_WAKEUP_EVENT);

    /* get sleep time */
    wakesta->timer_out = spm_read(SPM_PCM_TIMER_OUT);

    /* get special pattern (0xf0000 or 0x10000) if sleep abort */
    wakesta->event_reg = spm_read(SPM_PCM_EVENT_REG_STA);

    /* get ISR status */
    wakesta->isr = spm_read(SPM_SLEEP_ISR_STATUS);

    /* get MD/CONN and co-clock status */
    wakesta->r13 = spm_read(SPM_PCM_REG13_DATA);
}

static void spm_clean_after_wakeup(bool pcmwdt_en)
{
    /* disable PCM WDT to stop count if needed */
#if SPM_PCMWDT_EN
    if (pcmwdt_en)
        spm_write(SPM_PCM_CON1, CON1_CFG_KEY | (spm_read(SPM_PCM_CON1) & ~CON1_PCM_WDT_EN));
#endif

    /* PCM has cleared uart_clk_off_req and now clear it in POWER_ON_VAL1 */
    spm_write(SPM_POWER_ON_VAL1, spm_read(SPM_POWER_ON_VAL1) & ~R7_UART_CLK_OFF_REQ);

    /* SRCLKENA: POWER_ON_VAL1|r7 (PWR_IO_EN[7]=1) */
    spm_write(SPM_CLK_CON, spm_read(SPM_CLK_CON) & ~CC_SRCLKENA_MASK);

    /* re-enable POWER_ON_VAL0/1 to control power */
    spm_write(SPM_PCM_PWR_IO_EN, 0);

    /* unlock INFRA DCM */
    spm_write(SPM_CLK_CON, spm_read(SPM_CLK_CON) & ~CC_LOCK_INFRA_DCM);

    /* clean PCM timer event */
    spm_write(SPM_PCM_CON1, CON1_CFG_KEY | (spm_read(SPM_PCM_CON1) & ~CON1_PCM_TIMER_EN));

    /* clean CPU wakeup event (pause abort) */
    spm_write(SPM_SLEEP_CPU_WAKEUP_EVENT, 0);

    /* clean wakeup event raw status (except THERM) */
    spm_write(SPM_SLEEP_WAKEUP_EVENT_MASK, ~WAKE_SRC_THERM);

    /* clean ISR status (except TWAM) */
    spm_write(SPM_SLEEP_ISR_MASK, spm_read(SPM_SLEEP_ISR_MASK) | ISRM_ALL_EXC_TWAM);
    spm_write(SPM_SLEEP_ISR_STATUS, ISRC_ALL_EXC_TWAM);
    spm_write(SPM_PCM_SW_INT_CLEAR, PCM_SW_INT0);
}

static wake_reason_t spm_output_wake_reason(const wake_status_t *wakesta, bool dpidle)
{
    char str[200] = { 0 };
    wake_reason_t wr = WR_NONE;

    if (wakesta->debug_reg != 0) {
        spm_error2("PCM ASSERT AND PC = %u (0x%x)(0x%x)\n",
                   wakesta->debug_reg, wakesta->r13, wakesta->event_reg);
        return WR_PCM_ASSERT;
    }

    if (dpidle)     /* bypass wakeup event check */
        return WR_WAKE_SRC;

    if (wakesta->r12 & (1U << 0)) {
        if (!(wakesta->isr & ISR_TWAM) && !wakesta->cpu_wake) {
            strcat(str, "PCM_TIMER ");
            wr = WR_PCM_TIMER;
        } else {
            if (wakesta->isr & ISR_TWAM) {
                strcat(str, "TWAM ");
                wr = WR_WAKE_SRC;
            }
            if (wakesta->cpu_wake) {
                strcat(str, "CPU ");
                wr = WR_WAKE_SRC;
            }
        }
    }
    if (wakesta->r12 & WAKE_SRC_TS) {
        strcat(str, "TS ");
        wr = WR_WAKE_SRC;
    }
    if (wakesta->r12 & WAKE_SRC_KP) {
        strcat(str, "KP ");
        wr = WR_WAKE_SRC;
    }
    if (wakesta->r12 & WAKE_SRC_WDT) {
        strcat(str, "WDT ");
        wr = WR_WAKE_SRC;
    }
    if (wakesta->r12 & WAKE_SRC_GPT) {
        strcat(str, "GPT ");
        wr = WR_WAKE_SRC;
    }
    if (wakesta->r12 & WAKE_SRC_EINT) {
        strcat(str, "EINT ");
        wr = WR_WAKE_SRC;
    }
    if (wakesta->r12 & WAKE_SRC_CONN_WDT) {
        strcat(str, "CONN_WDT ");
        wr = WR_WAKE_SRC;
    }
    if (wakesta->r12 & WAKE_SRC_CCIF_MD) {
        strcat(str, "CCIF_MD ");
        wr = WR_WAKE_SRC;
    }
    if (wakesta->r12 & WAKE_SRC_LOW_BAT) {
        strcat(str, "LOW_BAT ");
        wr = WR_WAKE_SRC;
    }
    if (wakesta->r12 & WAKE_SRC_CONN) {
        strcat(str, "CONN ");
        wr = WR_WAKE_SRC;
    }
    if (wakesta->r12 & (1U << 13)) {
        strcat(str, "PCM_WDT ");
        wr = WR_PCM_WDT;
    }
    if (wakesta->r12 & WAKE_SRC_USB_CD) {
        strcat(str, "USB_CD ");
        wr = WR_WAKE_SRC;
    }
    if (wakesta->r12 & WAKE_SRC_USB_PDN) {
        strcat(str, "USB_PDN ");
        wr = WR_WAKE_SRC;
    }
    if (wakesta->r12 & WAKE_SRC_DBGSYS) {
        strcat(str, "DBGSYS ");
        wr = WR_WAKE_SRC;
    }
    if (wakesta->r12 & WAKE_SRC_UART0) {
        strcat(str, "UART0 ");
        wr = WR_WAKE_SRC;
    }
    if (wakesta->r12 & WAKE_SRC_AFE) {
        strcat(str, "AFE ");
        wr = WR_WAKE_SRC;
    }
    if (wakesta->r12 & WAKE_SRC_THERM) {
        strcat(str, "THERM ");
        wr = WR_WAKE_SRC;
    }
    if (wakesta->r12 & WAKE_SRC_CIRQ) {
        strcat(str, "CIRQ ");
        wr = WR_WAKE_SRC;
    }
    if (wakesta->r12 & WAKE_SRC_SYSPWREQ) {
        strcat(str, "SYSPWREQ ");
        wr = WR_WAKE_SRC;
    }
    if (wakesta->r12 & WAKE_SRC_MD_WDT) {
        strcat(str, "MD_WDT ");
        wr = WR_WAKE_SRC;
    }
    if (wakesta->r12 & WAKE_SRC_CPU0_IRQ) {
        strcat(str, "CPU0_IRQ ");
        wr = WR_WAKE_SRC;
    }
    if (wakesta->r12 & WAKE_SRC_CPU1_IRQ) {
        strcat(str, "CPU1_IRQ ");
        wr = WR_WAKE_SRC;
    }
    if (wakesta->r12 & WAKE_SRC_CPU2_IRQ) {
        strcat(str, "CPU2_IRQ ");
        wr = WR_WAKE_SRC;
    }
    if (wakesta->r12 & WAKE_SRC_CPU3_IRQ) {
        strcat(str, "CPU3_IRQ ");
        wr = WR_WAKE_SRC;
    }
    if (wr == WR_NONE) {
        strcat(str, "UNKNOWN ");
        wr = WR_UNKNOWN;
    }

    spm_crit2("wake up by %s(0x%x)(0x%x)(%u)\n",
              str, wakesta->r12, wakesta->raw_sta, wakesta->timer_out);
    spm_crit2("event_reg = 0x%x, isr = 0x%x, r13 = 0x%x\n",
              wakesta->event_reg, wakesta->isr, wakesta->r13);

    if (wakesta->r12 & WAKE_SRC_EINT)
        mt_eint_print_status();
    if (wakesta->r12 & WAKE_SRC_CCIF_MD)
        exec_ccci_kern_func_by_md_id(0, ID_GET_MD_WAKEUP_SRC, NULL, 0);

    return wr;
}

#if SPM_PWAKE_EN
static u32 spm_get_wake_period(int pwake_time, wake_reason_t last_wr)
{
    int period = SPM_WAKE_PERIOD;

    if (pwake_time < 0) {
        /* use FG to get the period of 1% battery decrease */
        period = get_dynamic_period(last_wr != WR_PCM_TIMER ? 1 : 0, SPM_WAKE_PERIOD, 1);
        if (period <= 0) {
            spm_warning("CANNOT GET PERIOD FROM FUEL GAUGE\n");
            period = SPM_WAKE_PERIOD;
        }
    } else {
        period = pwake_time;
        spm_crit2("pwake = %d\n", pwake_time);
    }

    if (period > 36 * 3600)     /* max period is 36.4 hours */
        period = 36 * 3600;

    return period;
}
#endif

/*
 * wakesrc: WAKE_SRC_XXX
 * enable : enable or disable @wakesrc
 * replace: if true, will replace the default setting
 */
int spm_set_sleep_wakesrc(u32 wakesrc, bool enable, bool replace)
{
    unsigned long flags;

    if (spm_is_wakesrc_invalid(wakesrc))
        return -EINVAL;

    spin_lock_irqsave(&spm_lock, flags);
    if (enable) {
        if (replace)
            spm_sleep_wakesrc = wakesrc;
        else
            spm_sleep_wakesrc |= wakesrc;
    } else {
        if (replace)
            spm_sleep_wakesrc = 0;
        else
            spm_sleep_wakesrc &= ~wakesrc;
    }
    spin_unlock_irqrestore(&spm_lock, flags);

    return 0;
}

/*
 * cpu_pdn:
 *    true  = CPU shutdown
 *    false = CPU standby
 * infra_pdn:
 *    true  = INFRA/DDRPHY power down
 *    false = keep INFRA/DDRPHY power
 * pwake_time:
 *    >= 0  = specific wakeup period
 */
wake_reason_t spm_go_to_sleep(bool cpu_pdn, bool infra_pdn, int pwake_time)
{
    u32 sec = 0;
    int wd_ret;
    wake_status_t wakesta;
    unsigned long flags;
    struct mtk_irq_mask mask;
    struct wd_api *wd_api;
    static wake_reason_t last_wr = WR_NONE;
    const pcm_desc_t *pcmdesc = &pcm_suspend;
    const bool pcmwdt_en = true;

#if SPM_PWAKE_EN
    sec = spm_get_wake_period(pwake_time, last_wr);
#endif

    wd_ret = get_wd_api(&wd_api);
    if (!wd_ret)
        wd_api->wd_suspend_notify();

    spin_lock_irqsave(&spm_lock, flags);
    mt_irq_mask_all(&mask);
    mt_irq_unmask_for_sleep(MT_SPM_IRQ_ID);
    mt_cirq_clone_gic();
    mt_cirq_enable();

    spm_set_sysclk_settle();

    spm_crit2("sec = %u, wakesrc = 0x%x (%u)(%u)\n",
              sec, spm_sleep_wakesrc, cpu_pdn, infra_pdn);

    spm_reset_and_init_pcm();

    spm_kick_im_to_fetch(pcmdesc);

    if (spm_request_uart_to_sleep()) {
        last_wr = WR_UART_BUSY;
        goto RESTORE_IRQ;
    }

    spm_init_pcm_register();

    spm_init_event_vector(pcmdesc);

    spm_set_pwrctl_for_sleep();

    spm_set_wakeup_event(sec * 32768, spm_sleep_wakesrc);

    spm_kick_pcm_to_run(cpu_pdn, infra_pdn, pcmwdt_en);

    spm_trigger_wfi_for_sleep(cpu_pdn, infra_pdn);

    spm_get_wakeup_status(&wakesta);

    spm_clean_after_wakeup(pcmwdt_en);

    last_wr = spm_output_wake_reason(&wakesta, false);

RESTORE_IRQ:
    mt_cirq_flush();
    mt_cirq_disable();
    mt_irq_mask_restore(&mask);
    spin_unlock_irqrestore(&spm_lock, flags);

    //spm_go_to_normal();   /* included in pcm_suspend */

    if (!wd_ret)
        wd_api->wd_resume_notify();

    return last_wr;
}

/*
 * cpu_pdn:
 *    true  = CPU dormant
 *    false = CPU standby
 * pwrlevel:
 *    0 = AXI is off
 *    1 = AXI is 26M
 * pwake_time:
 *    >= 0  = specific wakeup period
 */
wake_reason_t spm_go_to_sleep_dpidle(bool cpu_pdn, u16 pwrlevel, int pwake_time)
{
    u32 sec = 0;
    int wd_ret;
    wake_status_t wakesta;
    unsigned long flags;
    struct mtk_irq_mask mask;
    struct wd_api *wd_api;
    static wake_reason_t last_wr = WR_NONE;
    const pcm_desc_t *pcmdesc = &pcm_dpidle;
    const bool pcmwdt_en = false;

#if SPM_PWAKE_EN
    sec = spm_get_wake_period(pwake_time, last_wr);
#endif

    wd_ret = get_wd_api(&wd_api);
    if (!wd_ret)
        wd_api->wd_suspend_notify();

    spin_lock_irqsave(&spm_lock, flags);
    mt_irq_mask_all(&mask);
    mt_irq_unmask_for_sleep(MT_SPM_IRQ_ID);
    mt_cirq_clone_gic();
    mt_cirq_enable();

    spm_crit2("sec = %u, wakesrc = 0x%x [%u][%u]\n",
              sec, spm_sleep_wakesrc, cpu_pdn, pwrlevel);

    spm_reset_and_init_pcm();

    spm_kick_im_to_fetch(pcmdesc);

    if (spm_request_uart_to_sleep()) {
        last_wr = WR_UART_BUSY;
        goto RESTORE_IRQ;
    }

    spm_init_pcm_register();

    spm_init_event_vector(pcmdesc);

    spm_set_pwrctl_for_dpidle(pwrlevel);

    spm_set_wakeup_event(sec * 32768, spm_sleep_wakesrc);

    spm_kick_pcm_to_run(cpu_pdn, false, pcmwdt_en);     /* keep INFRA/DDRPHY power */

    spm_trigger_wfi_for_dpidle(cpu_pdn);

    spm_get_wakeup_status(&wakesta);

    spm_clean_after_wakeup(pcmwdt_en);

    last_wr = spm_output_wake_reason(&wakesta, false);

RESTORE_IRQ:
    mt_cirq_flush();
    mt_cirq_disable();
    mt_irq_mask_restore(&mask);
    spin_unlock_irqrestore(&spm_lock, flags);

    if (!wd_ret)
        wd_api->wd_resume_notify();

    return last_wr;
}

void __attribute__((weak)) spm_dpidle_before_wfi(void)
{
}

void __attribute__((weak)) spm_dpidle_after_wfi(void)
{
}

/*
 * cpu_pdn:
 *    true  = CPU dormant
 *    false = CPU standby
 * pwrlevel:
 *    0 = AXI is off
 *    1 = AXI is 26M
 */
wake_reason_t spm_go_to_dpidle(bool cpu_pdn, u16 pwrlevel)
{
    wake_status_t wakesta;
    unsigned long flags;
    struct mtk_irq_mask mask;
    wake_reason_t wr = WR_NONE;
    const pcm_desc_t *pcmdesc = &pcm_dpidle;
    const bool pcmwdt_en = false;

    spin_lock_irqsave(&spm_lock, flags);
    mt_irq_mask_all(&mask);
    mt_irq_unmask_for_sleep(MT_SPM_IRQ_ID);
    mt_cirq_clone_gic();
    mt_cirq_enable();

    spm_reset_and_init_pcm();

    spm_kick_im_to_fetch(pcmdesc);

    if (spm_request_uart_to_sleep()) {
        wr = WR_UART_BUSY;
        goto RESTORE_IRQ;
    }

    spm_init_pcm_register();

    spm_init_event_vector(pcmdesc);

    spm_set_pwrctl_for_dpidle(pwrlevel);

    spm_set_wakeup_event(0, WAKE_SRC_FOR_DPIDLE);

    spm_kick_pcm_to_run(cpu_pdn, false, pcmwdt_en);     /* keep INFRA/DDRPHY power */

    spm_dpidle_before_wfi();

    spm_trigger_wfi_for_dpidle(cpu_pdn);

    spm_dpidle_after_wfi();

    spm_get_wakeup_status(&wakesta);

    spm_clean_after_wakeup(pcmwdt_en);

    wr = spm_output_wake_reason(&wakesta, true);

RESTORE_IRQ:
    mt_cirq_flush();
    mt_cirq_disable();
    mt_irq_mask_restore(&mask);
    spin_unlock_irqrestore(&spm_lock, flags);

    return wr;
}

bool spm_is_md_sleep(void)
{
    return !(spm_read(SPM_PCM_REG13_DATA) & R13_MD_SRCCLKENI);
}

bool spm_is_conn_sleep(void)
{
    return !(spm_read(SPM_PCM_REG13_DATA) & R13_CONN_SRCCLKENI);
}

void spm_output_sleep_option(void)
{
    spm_notice("PWAKE_EN:%d, PCMWDT_EN:%d, BYPASS_SYSPWREQ:%d, "
               "CTRL_33VCORE:%d, CTRL_33VRF18:%d\n",
               SPM_PWAKE_EN, SPM_PCMWDT_EN, SPM_BYPASS_SYSPWREQ,
               SPM_CTRL_33VCORE, SPM_CTRL_33VRF18);
}

MODULE_AUTHOR("Terry Chang <terry.chang@mediatek.com>");
MODULE_DESCRIPTION("SPM-Sleep Driver v1.4");
