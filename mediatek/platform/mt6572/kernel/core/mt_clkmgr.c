/**
* @file    mt_clkmgr.c
* @brief   Driver for Clock Manager
*
*/

#define __MT_CLKMGR_C__

/*=============================================================*/
// Include files
/*=============================================================*/

// system includes
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/proc_fs.h>
#include <linux/jiffies.h>
#include <linux/debugfs.h>

#include <asm/uaccess.h>

// project includes
#include <mach/memory.h>
#include <mach/mt_typedefs.h>
#include <mach/sync_write.h>
#include <mach/mt_dcm.h>
#include <mach/mt_spm.h>
#include <mach/mt_spm_mtcmos.h>
#include <mach/mt_freqhopping.h>
#include <mach/pmic_mt6320_sw.h>
#include <mach/mt_ptp.h>
#include <mach/mt_gpufreq.h>

// local includes
#include <mach/mt_clkmgr.h>

// forward references


/*=============================================================*/
// Macro definition
/*=============================================================*/

// #define BIT(_bit_)          (unsigned int)(1 << (_bit_))
#define BITS(_bits_, _val_) ((((unsigned) -1 >> (31 - ((1) ? _bits_))) & ~((1U << ((0) ? _bits_)) - 1)) & ((_val_)<<((0) ? _bits_)))
#define BITMASK(_bits_)     (((unsigned) -1 >> (31 - ((1) ? _bits_))) & ~((1U << ((0) ? _bits_)) - 1))


/*=============================================================*/
// Local type definition
/*=============================================================*/


/*=============================================================*/
// Local variable definition
/*=============================================================*/


/*=============================================================*/
// Local function definition
/*=============================================================*/


/*=============================================================*/
// Gobal function definition
/*=============================================================*/

//
// CONFIG
//
// #define CONFIG_CLKMGR_EMULATION
// #define CONFIG_CLKMGR_SHOWLOG
// #define STATE_CHECK_DEBUG // TODO: disable first @ FPGA mode

//
// LOG
//
#define USING_XLOG

#ifdef USING_XLOG
#include <linux/xlog.h>

#define TAG     "Power/clkmgr"

#define HEX_FMT "0x%08x"

#define clk_err(fmt, args...)       \
    xlog_printk(ANDROID_LOG_ERROR, TAG, fmt, ##args)
#define clk_warn(fmt, args...)      \
    xlog_printk(ANDROID_LOG_WARN, TAG, fmt, ##args)
#define clk_info(fmt, args...)      \
    xlog_printk(ANDROID_LOG_INFO, TAG, fmt, ##args)
#define clk_dbg(fmt, args...)       \
    xlog_printk(ANDROID_LOG_DEBUG, TAG, fmt, ##args)
#define clk_ver(fmt, args...)       \
    xlog_printk(ANDROID_LOG_VERBOSE, TAG, fmt, ##args)

#else

#define TAG     "[Power/clkmgr] "

#define clk_err(fmt, args...)       \
    printk(KERN_ERR TAG);           \
    printk(KERN_CONT fmt, ##args)
#define clk_warn(fmt, args...)      \
    printk(KERN_WARNING TAG);       \
    printk(KERN_CONT fmt, ##args)
#define clk_info(fmt, args...)      \
    printk(KERN_NOTICE TAG);        \
    printk(KERN_CONT fmt, ##args)
#define clk_dbg(fmt, args...)       \
    printk(KERN_INFO TAG);          \
    printk(KERN_CONT fmt, ##args)
#define clk_ver(fmt, args...)       \
    printk(KERN_DEBUG TAG);         \
    printk(KERN_CONT fmt, ##args)

#endif

#define FUNC_LV_API         BIT(0)
#define FUNC_LV_LOCKED      BIT(1)
#define FUNC_LV_BODY        BIT(2)
#define FUNC_LV_OP          BIT(3)
#define FUNC_LV_REG_ACCESS  BIT(4)
#define FUNC_LV_DONT_CARE   BIT(5)

#define FUNC_LV_MASK        (FUNC_LV_API | FUNC_LV_LOCKED | FUNC_LV_BODY | FUNC_LV_OP | FUNC_LV_REG_ACCESS | FUNC_LV_DONT_CARE)

#if defined(CONFIG_CLKMGR_SHOWLOG)
#define ENTER_FUNC(lv)      do { if (lv & FUNC_LV_MASK) xlog_printk(ANDROID_LOG_WARN, TAG, ">> %s()\n", __FUNCTION__); } while(0)
#define EXIT_FUNC(lv)       do { if (lv & FUNC_LV_MASK) xlog_printk(ANDROID_LOG_WARN, TAG, "<< %s():%d\n", __FUNCTION__, __LINE__); } while(0)
#else
#define ENTER_FUNC(lv)
#define EXIT_FUNC(lv)
#endif // defined(CONFIG_CLKMGR_SHOWLOG)

//
// Register access function
//

#if defined(CONFIG_CLKMGR_SHOWLOG)

#if defined(CONFIG_CLKMGR_EMULATION)   // XXX: NOT ACCESS REGISTER

#define clk_readl(addr) \
    ((FUNC_LV_REG_ACCESS & FUNC_LV_MASK) ? xlog_printk(ANDROID_LOG_WARN, TAG, "clk_readl("HEX_FMT") @ %s():%d\n", (addr), __FUNCTION__, __LINE__) : 0, 0)

#define clk_writel(addr, val)   \
    do { if (FUNC_LV_REG_ACCESS & FUNC_LV_MASK) xlog_printk(ANDROID_LOG_WARN, TAG, "clk_writel("HEX_FMT", "HEX_FMT") @ %s():%d\n", (addr), (val), __FUNCTION__, __LINE__); } while(0)

#define clk_setl(addr, val) \
    do { if (FUNC_LV_REG_ACCESS & FUNC_LV_MASK) xlog_printk(ANDROID_LOG_WARN, TAG, "clk_setl("HEX_FMT", "HEX_FMT") @ %s():%d\n", (addr), (val), __FUNCTION__, __LINE__); } while(0)

#define clk_clrl(addr, val) \
    do { if (FUNC_LV_REG_ACCESS & FUNC_LV_MASK) xlog_printk(ANDROID_LOG_WARN, TAG, "clk_clrl("HEX_FMT", "HEX_FMT") @ %s():%d\n", (addr), (val), __FUNCTION__, __LINE__); } while(0)

#else                           // XXX: ACCESS REGISTER

#define clk_readl(addr) \
    ((FUNC_LV_REG_ACCESS & FUNC_LV_MASK) ? xlog_printk(ANDROID_LOG_WARN, TAG, "clk_readl("HEX_FMT") @ %s():%d\n", (addr), __FUNCTION__, __LINE__) : 0, DRV_Reg32(addr))

#define clk_writel(addr, val)   \
    do { unsigned int value; if (FUNC_LV_REG_ACCESS & FUNC_LV_MASK) xlog_printk(ANDROID_LOG_WARN, TAG, "clk_writel("HEX_FMT", "HEX_FMT") @ %s():%d\n", (addr), (value = val), __FUNCTION__, __LINE__); mt65xx_reg_sync_writel((value), (addr)); } while(0)

#define clk_setl(addr, val) \
    do { if (FUNC_LV_REG_ACCESS & FUNC_LV_MASK) xlog_printk(ANDROID_LOG_WARN, TAG, "clk_setl("HEX_FMT", "HEX_FMT") @ %s():%d\n", (addr), (val), __FUNCTION__, __LINE__); mt65xx_reg_sync_writel(clk_readl(addr) | (val), (addr)); } while(0)

#define clk_clrl(addr, val) \
    do { if (FUNC_LV_REG_ACCESS & FUNC_LV_MASK) xlog_printk(ANDROID_LOG_WARN, TAG, "clk_clrl("HEX_FMT", "HEX_FMT") @ %s():%d\n", (addr), (val), __FUNCTION__, __LINE__); mt65xx_reg_sync_writel(clk_readl(addr) & ~(val), (addr)); } while(0)

#endif // defined(CONFIG_CLKMGR_EMULATION)

#else

#define clk_readl(addr)         DRV_Reg32(addr)
#define clk_writel(addr, val)   mt65xx_reg_sync_writel(val, addr)
#define clk_setl(addr, val)     mt65xx_reg_sync_writel(clk_readl(addr) | (val), addr)
#define clk_clrl(addr, val)     mt65xx_reg_sync_writel(clk_readl(addr) & ~(val), addr)

#endif // defined(CONFIG_CLKMGR_SHOWLOG)

#ifndef spm_read
#define spm_read  clk_readl
#endif
#ifndef spm_write
#define spm_write clk_writel
#endif

bool is_ddr3(void)
{
    return (clk_readl(CLK_MUX_SEL) & BITMASK(7:7)) ? FALSE : TRUE; // XXX: rg_axibus_gfmux_sel[2]
}


//
// INIT
//
static int initialized = 0;

//
// LOCK
//
static DEFINE_SPINLOCK(clock_lock);

#define clkmgr_lock(flags) \
do { \
    spin_lock_irqsave(&clock_lock, flags); \
} while(0)

#define clkmgr_unlock(flags) \
do { \
    spin_unlock_irqrestore(&clock_lock, flags); \
} while(0)

#define clkmgr_locked()  (spin_is_locked(&clock_lock))

int clkmgr_is_locked(void)
{
    return clkmgr_locked();
}
EXPORT_SYMBOL(clkmgr_is_locked);

//
// PLL data structure
//
struct pll;

struct pll_ops
{
    int (*get_state)(struct pll *pll);
    // void (*change_mode)(int mode);
    void (*enable)(struct pll *pll);
    void (*disable)(struct pll *pll);
    void (*fsel)(struct pll *pll, unsigned int value);
    int (*dump_regs)(struct pll *pll, unsigned int *ptr);
    unsigned int (*vco_calc)(struct pll *pll);
    int (*hp_enable)(struct pll *pll);
    int (*hp_disable)(struct pll *pll);
};

struct pll
{
    const char *name;
    const int type;                 // TODO: NOT USED (only SDM now)
    const int mode;                 // TODO: NOT USED
    const int feat;
    int state;
    unsigned int cnt;
    const unsigned int en_mask;
    const unsigned int base_addr;
    const unsigned int pwr_addr;
    const struct pll_ops *ops;
    const unsigned int hp_id;
    const int hp_switch;
};

//
// SUBSYS data structure
//
struct subsys;

struct subsys_ops
{
    int (*enable)(struct subsys *sys);
    int (*disable)(struct subsys *sys);
    int (*get_state)(struct subsys *sys);
    int (*dump_regs)(struct subsys *sys, unsigned int *ptr);
};

struct subsys
{
    const char *name;
    const int type;
    const int force_on;
    // unsigned int cnt;                    // NOT USED
    unsigned int state;
    unsigned int default_sta;
    const unsigned int sta_mask;            // mask in PWR_STATUS
    const unsigned int ctl_addr;            // SPM_XXX_PWR_CON
    const unsigned int sram_pdn_bits;       // SRAM_PDN @ SPM_XXX_PWR_CON
    const unsigned int sram_pdn_ack_bits;   // SRAM_PDN_ACK @ SPM_XXX_PWR_CON
    unsigned int bus_prot_mask;             // mask in INFRA_TOPAXI_PROTECTEN & INFRA_TOPAXI_PROTECTSTA1
    // int (*pwr_ctrl)(int state);          // NOT USED
    const struct subsys_ops *ops;
    const struct cg_grp *start; // TODO: refine it
    const unsigned int nr_grps; // TODO: refine it
};

//
//  CLKMUX data structure
//
struct clkmux;

struct clkmux_ops
{
    int (*sel)(struct clkmux *mux, cg_clk_id clksrc);
    cg_clk_id (*get)(struct clkmux *mux);
    // void (*enable)(struct clkmux *mux);
    // void (*disable)(struct clkmux *mux);
};

struct clkmux_map
{
    const unsigned int val;
    const cg_clk_id id;
    const unsigned int mask;
};

struct clkmux
{
    const char *name;
    const unsigned int base_addr;
    // const unsigned int sel_mask;
    // unsigned int pdn_mask;
    // unsigned int offset;
    // unsigned int nr_inputs;
    const struct clkmux_ops *ops;

    const struct clkmux_map *map;
    const unsigned int nr_map;
    const cg_clk_id drain;
};

//
// CG_GRP data structure
//
struct cg_grp;

struct cg_grp_ops
{
    // int (*prepare)(struct cg_grp *grp);  // XXX: NOT USED
    // int (*finished)(struct cg_grp *grp); // XXX: NOT USED
    unsigned int (*get_state)(struct cg_grp *grp);
    int (*dump_regs)(struct cg_grp *grp, unsigned int *ptr);
};

struct cg_grp
{
    const char *name;
    const unsigned int set_addr;
    const unsigned int clr_addr;
    const unsigned int sta_addr;
    unsigned int mask;
    unsigned int state;
    const struct cg_grp_ops *ops;
    struct subsys *sys;
};

//
// CG_CLK data structure
//
struct cg_clk;

struct cg_clk_ops
{
    int (*get_state)(struct cg_clk *clk);
    int (*check_validity)(struct cg_clk *clk);// 1: valid, 0: invalid
    int (*enable)(struct cg_clk *clk);
    int (*disable)(struct cg_clk *clk);
};

struct cg_clk
{
    const char *name;
    int cnt;
    unsigned int state;
    const unsigned int mask;
    const struct cg_clk_ops *ops;
    struct cg_grp *grp;
    struct pll *parent;

    cg_clk_id src;
};

//
// CG_CLK variable & function
//
static struct subsys syss[]; // NR_SYSS
static struct cg_grp grps[]; // NR_GRPS
static struct cg_clk clks[]; // NR_CLKS
static struct pll plls[];    // NR_PLLS

static struct cg_clk_ops general_gate_cg_clk_ops; // XXX: set/clr/sta addr are diff
static struct cg_clk_ops general_en_cg_clk_ops;   // XXX: set/clr/sta addr are diff
static struct cg_clk_ops wo_clr_cg_clk_ops;       // XXX: without CLR (i.e. set/clr/sta addr are same)
static struct cg_clk_ops ao_cg_clk_ops;           // XXX: always on
// static struct cg_clk_ops mfg_cg_clk_ops;          // XXX: mfg /* not used */

static struct cg_clk clks[] = // NR_CLKS
{
    // CG_MIXED
    [MT_CG_SYS_26M]                 = { .name = __stringify(MT_CG_SYS_26M),                 .cnt = 0, .mask = BIT(0),                       .ops = &ao_cg_clk_ops,           .grp = &grps[CG_MIXED], .src = MT_CG_INVALID,  }, // pll
    // [MT_CG_UNIV_48M]                = { .name = __stringify(MT_CG_UNIV_48M),                .cnt = 0, .mask = UNIV48M_EN_BIT,               .ops = &wo_clr_cg_clk_ops,       .grp = &grps[CG_MIXED], .src = MT_CG_INVALID,  }, // pll
    [MT_CG_USB_48M]                 = { .name = __stringify(MT_CG_USB_48M),                 .cnt = 0, .mask = USB48M_EN_BIT,                .ops = &wo_clr_cg_clk_ops,       .grp = &grps[CG_MIXED], .src = MT_CG_INVALID,  }, // pll

    // CG_MPLL
    [MT_CG_MPLL_D2]                 = { .name = __stringify(MT_CG_MPLL_D2),                 .cnt = 0, .mask = MPLL_D2_EN_BIT,               .ops = &general_en_cg_clk_ops,   .grp = &grps[CG_MPLL],  .src = MT_CG_INVALID,  }, // pll
    [MT_CG_MPLL_D3]                 = { .name = __stringify(MT_CG_MPLL_D3),                 .cnt = 0, .mask = MPLL_D3_EN_BIT,               .ops = &general_en_cg_clk_ops,   .grp = &grps[CG_MPLL],  .src = MT_CG_INVALID,  }, // pll
    [MT_CG_MPLL_D5]                 = { .name = __stringify(MT_CG_MPLL_D5),                 .cnt = 0, .mask = MPLL_D5_EN_BIT,               .ops = &general_en_cg_clk_ops,   .grp = &grps[CG_MPLL],  .src = MT_CG_INVALID,  }, // pll
    [MT_CG_MPLL_D7]                 = { .name = __stringify(MT_CG_MPLL_D7),                 .cnt = 0, .mask = MPLL_D7_EN_BIT,               .ops = &general_en_cg_clk_ops,   .grp = &grps[CG_MPLL],  .src = MT_CG_INVALID,  }, // pll

    [MT_CG_MPLL_D4]                 = { .name = __stringify(MT_CG_MPLL_D4),                 .cnt = 0, .mask = MPLL_D4_EN_BIT,               .ops = &general_en_cg_clk_ops,   .grp = &grps[CG_MPLL],  .src = MT_CG_MPLL_D2,  },
    [MT_CG_MPLL_D6]                 = { .name = __stringify(MT_CG_MPLL_D6),                 .cnt = 0, .mask = MPLL_D6_EN_BIT,               .ops = &general_en_cg_clk_ops,   .grp = &grps[CG_MPLL],  .src = MT_CG_MPLL_D3,  },
    [MT_CG_MPLL_D10]                = { .name = __stringify(MT_CG_MPLL_D10),                .cnt = 0, .mask = MPLL_D10_EN_BIT,              .ops = &general_en_cg_clk_ops,   .grp = &grps[CG_MPLL],  .src = MT_CG_MPLL_D5,  },

    [MT_CG_MPLL_D8]                 = { .name = __stringify(MT_CG_MPLL_D8),                 .cnt = 0, .mask = MPLL_D8_EN_BIT,               .ops = &general_en_cg_clk_ops,   .grp = &grps[CG_MPLL],  .src = MT_CG_MPLL_D4,  },
    [MT_CG_MPLL_D12]                = { .name = __stringify(MT_CG_MPLL_D12),                .cnt = 0, .mask = MPLL_D12_EN_BIT,              .ops = &general_en_cg_clk_ops,   .grp = &grps[CG_MPLL],  .src = MT_CG_MPLL_D6,  },
    [MT_CG_MPLL_D20]                = { .name = __stringify(MT_CG_MPLL_D20),                .cnt = 0, .mask = MPLL_D20_EN_BIT,              .ops = &general_en_cg_clk_ops,   .grp = &grps[CG_MPLL],  .src = MT_CG_MPLL_D10, },

    [MT_CG_MPLL_D24]                = { .name = __stringify(MT_CG_MPLL_D24),                .cnt = 0, .mask = MPLL_D24_EN_BIT,              .ops = &general_en_cg_clk_ops,   .grp = &grps[CG_MPLL],  .src = MT_CG_MPLL_D12, },

    // CG_UPLL
    [MT_CG_UPLL_D2]                 = { .name = __stringify(MT_CG_UPLL_D2),                 .cnt = 0, .mask = UPLL_D2_EN_BIT,               .ops = &general_en_cg_clk_ops,   .grp = &grps[CG_UPLL],  .src = MT_CG_INVALID,  }, // pll
    [MT_CG_UPLL_D3]                 = { .name = __stringify(MT_CG_UPLL_D3),                 .cnt = 0, .mask = UPLL_D3_EN_BIT,               .ops = &general_en_cg_clk_ops,   .grp = &grps[CG_UPLL],  .src = MT_CG_INVALID,  }, // pll
    [MT_CG_UPLL_D5]                 = { .name = __stringify(MT_CG_UPLL_D5),                 .cnt = 0, .mask = UPLL_D5_EN_BIT,               .ops = &general_en_cg_clk_ops,   .grp = &grps[CG_UPLL],  .src = MT_CG_INVALID,  }, // pll
    [MT_CG_UPLL_D7]                 = { .name = __stringify(MT_CG_UPLL_D7),                 .cnt = 0, .mask = UPLL_D7_EN_BIT,               .ops = &general_en_cg_clk_ops,   .grp = &grps[CG_UPLL],  .src = MT_CG_INVALID,  }, // pll

    [MT_CG_UPLL_D4]                 = { .name = __stringify(MT_CG_UPLL_D4),                 .cnt = 0, .mask = UPLL_D4_EN_BIT,               .ops = &general_en_cg_clk_ops,   .grp = &grps[CG_UPLL],  .src = MT_CG_UPLL_D2,  },
    [MT_CG_UPLL_D6]                 = { .name = __stringify(MT_CG_UPLL_D6),                 .cnt = 0, .mask = UPLL_D6_EN_BIT,               .ops = &general_en_cg_clk_ops,   .grp = &grps[CG_UPLL],  .src = MT_CG_UPLL_D3,  },
    [MT_CG_UPLL_D10]                = { .name = __stringify(MT_CG_UPLL_D10),                .cnt = 0, .mask = UPLL_D10_EN_BIT,              .ops = &general_en_cg_clk_ops,   .grp = &grps[CG_UPLL],  .src = MT_CG_UPLL_D5,  },

    [MT_CG_UPLL_D8]                 = { .name = __stringify(MT_CG_UPLL_D8),                 .cnt = 0, .mask = UPLL_D8_EN_BIT,               .ops = &general_en_cg_clk_ops,   .grp = &grps[CG_UPLL],  .src = MT_CG_UPLL_D4,  },
    [MT_CG_UPLL_D12]                = { .name = __stringify(MT_CG_UPLL_D12),                .cnt = 0, .mask = UPLL_D12_EN_BIT,              .ops = &general_en_cg_clk_ops,   .grp = &grps[CG_UPLL],  .src = MT_CG_UPLL_D6,  },
    [MT_CG_UPLL_D20]                = { .name = __stringify(MT_CG_UPLL_D20),                .cnt = 0, .mask = UPLL_D20_EN_BIT,              .ops = &general_en_cg_clk_ops,   .grp = &grps[CG_UPLL],  .src = MT_CG_UPLL_D10, },

    [MT_CG_UPLL_D16]                = { .name = __stringify(MT_CG_UPLL_D16),                .cnt = 0, .mask = UPLL_D16_EN_BIT,              .ops = &general_en_cg_clk_ops,   .grp = &grps[CG_UPLL],  .src = MT_CG_UPLL_D8,  },
    [MT_CG_UPLL_D24]                = { .name = __stringify(MT_CG_UPLL_D24),                .cnt = 0, .mask = UPLL_D24_EN_BIT,              .ops = &general_en_cg_clk_ops,   .grp = &grps[CG_UPLL],  .src = MT_CG_UPLL_D12, },

    // CG_CTRL0 (PERI/INFRA)
    [MT_CG_PWM_MM_SW_CG]            = { .name = __stringify(MT_CG_PWM_MM_SW_CG),            .cnt = 0, .mask = PWM_MM_SW_CG_BIT,             .ops = &general_gate_cg_clk_ops, .grp = &grps[CG_CTRL0], .src = MT_CG_INVALID,  }, // rg_pwm_mux_sel
    [MT_CG_CAM_MM_SW_CG]            = { .name = __stringify(MT_CG_CAM_MM_SW_CG),            .cnt = 0, .mask = CAM_MM_SW_CG_BIT,             .ops = &general_gate_cg_clk_ops, .grp = &grps[CG_CTRL0], .src = MT_CG_INVALID,  }, // rg_cam_mux_sel
    [MT_CG_MFG_MM_SW_CG]            = { .name = __stringify(MT_CG_MFG_MM_SW_CG),            .cnt = 0, .mask = MFG_MM_SW_CG_BIT,             .ops = &general_gate_cg_clk_ops, .grp = &grps[CG_CTRL0], .src = MT_CG_INVALID,  }, // rg_mfg_gfmux_sel, rg_mfg_mux_sel
    [MT_CG_SPM_52M_SW_CG]           = { .name = __stringify(MT_CG_SPM_52M_SW_CG),           .cnt = 0, .mask = SPM_52M_SW_CG_BIT,            .ops = &general_gate_cg_clk_ops, .grp = &grps[CG_CTRL0], .src = MT_CG_INVALID,  }, // rg_spm_52m_ck_sel
    [MT_CG_MIPI_26M_DBG_EN]         = { .name = __stringify(MT_CG_MIPI_26M_DBG_EN),         .cnt = 0, .mask = MIPI_26M_DBG_EN_BIT,          .ops = &general_en_cg_clk_ops,   .grp = &grps[CG_CTRL0], .src = MT_CG_INVALID,  }, // feature
    [MT_CG_DBI_BCLK_SW_CG]          = { .name = __stringify(MT_CG_DBI_BCLK_SW_CG),          .cnt = 0, .mask = DBI_BCLK_SW_CG_BIT,           .ops = &general_gate_cg_clk_ops, .grp = &grps[CG_CTRL0], .src = MT_CG_SRC_DBI,  }, // auto sel
    [MT_CG_SC_26M_CK_SEL_EN]        = { .name = __stringify(MT_CG_SC_26M_CK_SEL_EN),        .cnt = 0, .mask = SC_26M_CK_SEL_EN_BIT,         .ops = &general_en_cg_clk_ops,   .grp = &grps[CG_CTRL0], .src = MT_CG_INVALID,  }, // feature
    [MT_CG_SC_MEM_CK_OFF_EN]        = { .name = __stringify(MT_CG_SC_MEM_CK_OFF_EN),        .cnt = 0, .mask = SC_MEM_CK_OFF_EN_BIT,         .ops = &general_en_cg_clk_ops,   .grp = &grps[CG_CTRL0], .src = MT_CG_INVALID,  }, // feature

    [MT_CG_DBI_PAD0_SW_CG]          = { .name = __stringify(MT_CG_DBI_PAD0_SW_CG),          .cnt = 0, .mask = DBI_PAD0_SW_CG_BIT,           .ops = &general_gate_cg_clk_ops, .grp = &grps[CG_CTRL0], .src = MT_CG_DBI_BCLK_SW_CG, },
    [MT_CG_DBI_PAD1_SW_CG]          = { .name = __stringify(MT_CG_DBI_PAD1_SW_CG),          .cnt = 0, .mask = DBI_PAD1_SW_CG_BIT,           .ops = &general_gate_cg_clk_ops, .grp = &grps[CG_CTRL0], .src = MT_CG_DBI_BCLK_SW_CG, },
    [MT_CG_DBI_PAD2_SW_CG]          = { .name = __stringify(MT_CG_DBI_PAD2_SW_CG),          .cnt = 0, .mask = DBI_PAD2_SW_CG_BIT,           .ops = &general_gate_cg_clk_ops, .grp = &grps[CG_CTRL0], .src = MT_CG_DBI_BCLK_SW_CG, },
    [MT_CG_DBI_PAD3_SW_CG]          = { .name = __stringify(MT_CG_DBI_PAD3_SW_CG),          .cnt = 0, .mask = DBI_PAD3_SW_CG_BIT,           .ops = &general_gate_cg_clk_ops, .grp = &grps[CG_CTRL0], .src = MT_CG_DBI_BCLK_SW_CG, },
    [MT_CG_GPU_491P52M_EN]          = { .name = __stringify(MT_CG_GPU_491P52M_EN),          .cnt = 0, .mask = GPU_491P52M_EN_BIT,           .ops = &general_en_cg_clk_ops,   .grp = &grps[CG_CTRL0], .src = MT_CG_INVALID,  }, // pll
    [MT_CG_GPU_500P5M_EN]           = { .name = __stringify(MT_CG_GPU_500P5M_EN),           .cnt = 0, .mask = GPU_500P5M_EN_BIT,            .ops = &general_en_cg_clk_ops,   .grp = &grps[CG_CTRL0], .src = MT_CG_INVALID, .parent = &plls[WHPLL] }, // pll

    [MT_CG_ARMDCM_CLKOFF_EN]        = { .name = __stringify(MT_CG_ARMDCM_CLKOFF_EN),        .cnt = 0, .mask = ARMDCM_CLKOFF_EN_BIT,         .ops = &general_en_cg_clk_ops,   .grp = &grps[CG_CTRL0], .src = MT_CG_INVALID,  }, // feature

    // CG_CTRL1 (PERI/INFRA)
    [MT_CG_EFUSE_SW_CG]             = { .name = __stringify(MT_CG_EFUSE_SW_CG),             .cnt = 0, .mask = EFUSE_SW_CG_BIT,              .ops = &general_gate_cg_clk_ops, .grp = &grps[CG_CTRL1], .src = MT_CG_INVALID,  }, // MT_CG_SYS_26M
    [MT_CG_THEM_SW_CG]              = { .name = __stringify(MT_CG_THEM_SW_CG),              .cnt = 0, .mask = THEM_SW_CG_BIT,               .ops = &general_gate_cg_clk_ops, .grp = &grps[CG_CTRL1], .src = MT_CG_INVALID,  }, // MT_CG_SYS_26M
    [MT_CG_APDMA_SW_CG]             = { .name = __stringify(MT_CG_APDMA_SW_CG),             .cnt = 0, .mask = APDMA_SW_CG_BIT,              .ops = &general_gate_cg_clk_ops, .grp = &grps[CG_CTRL1], .src = MT_CG_INVALID,  }, // AXIBUS
    [MT_CG_I2C0_SW_CG]              = { .name = __stringify(MT_CG_I2C0_SW_CG),              .cnt = 0, .mask = I2C0_SW_CG_BIT,               .ops = &general_gate_cg_clk_ops, .grp = &grps[CG_CTRL1], .src = MT_CG_INVALID,  }, // AXIBUS
    [MT_CG_I2C1_SW_CG]              = { .name = __stringify(MT_CG_I2C1_SW_CG),              .cnt = 0, .mask = I2C1_SW_CG_BIT,               .ops = &general_gate_cg_clk_ops, .grp = &grps[CG_CTRL1], .src = MT_CG_INVALID,  }, // AXIBUS
    // [MT_CG_AUX_SW_CG_MD]            = { .name = __stringify(MT_CG_AUX_SW_CG_MD),            .cnt = 0, .mask = AUX_SW_CG_MD_BIT,             .ops = &general_gate_cg_clk_ops, .grp = &grps[CG_CTRL1], .src = MT_CG_INVALID,  }, // MT_CG_SYS_26M

    [MT_CG_NFI_SW_CG]               = { .name = __stringify(MT_CG_NFI_SW_CG),               .cnt = 0, .mask = NFI_SW_CG_BIT,                .ops = &general_gate_cg_clk_ops, .grp = &grps[CG_CTRL1], .src = MT_CG_INVALID,  }, // AXIBUS
    [MT_CG_NFIECC_SW_CG]            = { .name = __stringify(MT_CG_NFIECC_SW_CG),            .cnt = 0, .mask = NFIECC_SW_CG_BIT,             .ops = &general_gate_cg_clk_ops, .grp = &grps[CG_CTRL1], .src = MT_CG_INVALID,  }, // AXIBUS

    [MT_CG_DEBUGSYS_SW_CG]          = { .name = __stringify(MT_CG_DEBUGSYS_SW_CG),          .cnt = 0, .mask = DEBUGSYS_SW_CG_BIT,           .ops = &general_gate_cg_clk_ops, .grp = &grps[CG_CTRL1], .src = MT_CG_INVALID,  }, // AXIBUS
    [MT_CG_PWM_SW_CG]               = { .name = __stringify(MT_CG_PWM_SW_CG),               .cnt = 0, .mask = PWM_SW_CG_BIT,                .ops = &general_gate_cg_clk_ops, .grp = &grps[CG_CTRL1], .src = MT_CG_INVALID,  }, // AXIBUS
    [MT_CG_UART0_SW_CG]             = { .name = __stringify(MT_CG_UART0_SW_CG),             .cnt = 0, .mask = UART0_SW_CG_BIT,              .ops = &general_gate_cg_clk_ops, .grp = &grps[CG_CTRL1], .src = MT_CG_INVALID,  }, // rg_uart0_gfmux_sel
    [MT_CG_UART1_SW_CG]             = { .name = __stringify(MT_CG_UART1_SW_CG),             .cnt = 0, .mask = UART1_SW_CG_BIT,              .ops = &general_gate_cg_clk_ops, .grp = &grps[CG_CTRL1], .src = MT_CG_INVALID,  }, // rg_uart1_gfmux_sel
    [MT_CG_BTIF_SW_CG]              = { .name = __stringify(MT_CG_BTIF_SW_CG),              .cnt = 0, .mask = BTIF_SW_CG_BIT,               .ops = &general_gate_cg_clk_ops, .grp = &grps[CG_CTRL1], .src = MT_CG_INVALID,  }, // AXIBUS
    [MT_CG_USB_SW_CG]               = { .name = __stringify(MT_CG_USB_SW_CG),               .cnt = 0, .mask = USB_SW_CG_BIT,                .ops = &general_gate_cg_clk_ops, .grp = &grps[CG_CTRL1], .src = MT_CG_INVALID,  }, // AXIBUS
    [MT_CG_FHCTL_SW_CG]             = { .name = __stringify(MT_CG_FHCTL_SW_CG),             .cnt = 0, .mask = FHCTL_SW_CG_BIT,              .ops = &general_gate_cg_clk_ops, .grp = &grps[CG_CTRL1], .src = MT_CG_INVALID,  }, // MT_CG_SYS_26M
    // [MT_CG_AUX_SW_CG_THERM]         = { .name = __stringify(MT_CG_AUX_SW_CG_THERM),         .cnt = 0, .mask = AUX_SW_CG_THERM_BIT,          .ops = &general_gate_cg_clk_ops, .grp = &grps[CG_CTRL1], .src = MT_CG_INVALID,  }, // MT_CG_SYS_26M

    [MT_CG_SPINFI_SW_CG]            = { .name = __stringify(MT_CG_SPINFI_SW_CG),            .cnt = 0, .mask = SPINFI_SW_CG_BIT,             .ops = &general_gate_cg_clk_ops, .grp = &grps[CG_CTRL1], .src = MT_CG_INVALID,  }, // rg_spinfi_gfmux_sel, rg_spinfi_mux_sel
    [MT_CG_MSDC0_SW_CG]             = { .name = __stringify(MT_CG_MSDC0_SW_CG),             .cnt = 0, .mask = MSDC0_SW_CG_BIT,              .ops = &general_gate_cg_clk_ops, .grp = &grps[CG_CTRL1], .src = MT_CG_INVALID,  }, // rg_msdc0_mux_sel
    [MT_CG_MSDC1_SW_CG]             = { .name = __stringify(MT_CG_MSDC1_SW_CG),             .cnt = 0, .mask = MSDC1_SW_CG_BIT,              .ops = &general_gate_cg_clk_ops, .grp = &grps[CG_CTRL1], .src = MT_CG_INVALID,  }, // rg_msdc1_mux_sel

    [MT_CG_PMIC_SW_CG_AP]           = { .name = __stringify(MT_CG_PMIC_SW_CG_AP),           .cnt = 0, .mask = PMIC_SW_CG_AP_BIT,            .ops = &general_gate_cg_clk_ops, .grp = &grps[CG_CTRL1], .src = MT_CG_INVALID,  }, // rg_pmicspi_mux_sel (all clk off and then clk off) (rg_pmic_sw_cg_ap (AP) && rg_PMIC_SW_CG_MD (MDSYS) && rg_PMIC_SW_CG_CONN (CONNSYS))
    [MT_CG_SEJ_SW_CG]               = { .name = __stringify(MT_CG_SEJ_SW_CG),               .cnt = 0, .mask = SEJ_SW_CG_BIT,                .ops = &general_gate_cg_clk_ops, .grp = &grps[CG_CTRL1], .src = MT_CG_INVALID,  }, // MT_CG_SYS_26M, AXIBUS
    [MT_CG_MEMSLP_DLYER_SW_CG]      = { .name = __stringify(MT_CG_MEMSLP_DLYER_SW_CG),      .cnt = 0, .mask = MEMSLP_DLYER_SW_CG_BIT,       .ops = &general_gate_cg_clk_ops, .grp = &grps[CG_CTRL1], .src = MT_CG_INVALID   }, // feature

    [MT_CG_APXGPT_SW_CG]            = { .name = __stringify(MT_CG_APXGPT_SW_CG),            .cnt = 0, .mask = APXGPT_SW_CG_BIT,             .ops = &general_gate_cg_clk_ops, .grp = &grps[CG_CTRL1], .src = MT_CG_INVALID,  }, // MT_CG_SYS_26M, AXIBUS
    [MT_CG_AUDIO_SW_CG]             = { .name = __stringify(MT_CG_AUDIO_SW_CG),             .cnt = 0, .mask = AUDIO_SW_CG_BIT,              .ops = &general_gate_cg_clk_ops, .grp = &grps[CG_CTRL1], .src = MT_CG_INVALID,  }, // rg_aud_intbus_sel
    [MT_CG_SPM_SW_CG]               = { .name = __stringify(MT_CG_SPM_SW_CG),               .cnt = 0, .mask = SPM_SW_CG_BIT,                .ops = &general_gate_cg_clk_ops, .grp = &grps[CG_CTRL1], .src = MT_CG_INVALID,  }, // MT_CG_SYS_26M
    // [MT_CG_PMIC_SW_CG_MD]           = { .name = __stringify(MT_CG_PMIC_SW_CG_MD),           .cnt = 0, .mask = PMIC_SW_CG_MD_BIT,            .ops = &general_gate_cg_clk_ops, .grp = &grps[CG_CTRL1], .src = MT_CG_INVALID,  }, // rg_pmicspi_mux_sel (all clk off and then clk off) (rg_pmic_sw_cg_ap (AP) && rg_pmic_sw_cg_md (MDSYS) && rg_pmic_sw_cg_conn (CONNSYS))
    // [MT_CG_PMIC_SW_CG_CONN]         = { .name = __stringify(MT_CG_PMIC_SW_CG_CONN),         .cnt = 0, .mask = PMIC_SW_CG_CONN_BIT,          .ops = &general_gate_cg_clk_ops, .grp = &grps[CG_CTRL1], .src = MT_CG_INVALID,  }, // rg_pmicspi_mux_sel (all clk off and then clk off) (rg_pmic_sw_cg_ap (AP) && rg_pmic_sw_cg_md (MDSYS) && rg_pmic_sw_cg_conn (CONNSYS))
    [MT_CG_PMIC_26M_SW_CG]          = { .name = __stringify(MT_CG_PMIC_26M_SW_CG),          .cnt = 0, .mask = PMIC_26M_SW_CG_BIT,           .ops = &general_gate_cg_clk_ops, .grp = &grps[CG_CTRL1], .src = MT_CG_INVALID,  }, // MT_CG_SYS_26M
    [MT_CG_AUX_SW_CG_ADC]           = { .name = __stringify(MT_CG_AUX_SW_CG_ADC),           .cnt = 0, .mask = AUX_SW_CG_ADC_BIT,            .ops = &general_gate_cg_clk_ops, .grp = &grps[CG_CTRL1], .src = MT_CG_INVALID,  }, // MT_CG_SYS_26M
    // [MT_CG_AUX_SW_CG_TP]            = { .name = __stringify(MT_CG_AUX_SW_CG_TP),            .cnt = 0, .mask = AUX_SW_CG_TP_BIT,             .ops = &general_gate_cg_clk_ops, .grp = &grps[CG_CTRL1], .src = MT_CG_INVALID,  }, // MT_CG_SYS_26M

    // CG_MMSYS0
    // [MT_CG_SMI_COMMON_SW_CG]        = { .name = __stringify(MT_CG_SMI_COMMON_SW_CG),        .cnt = 0, .mask = SMI_COMMON_SW_CG_BIT,         .ops = &general_gate_cg_clk_ops, .grp = &grps[CG_MMSYS0], .src = MT_CG_SRC_SMI, }, // auto sel
    // [MT_CG_SMI_LARB0_SW_CG]         = { .name = __stringify(MT_CG_SMI_LARB0_SW_CG),         .cnt = 0, .mask = SMI_LARB0_SW_CG_BIT,          .ops = &general_gate_cg_clk_ops, .grp = &grps[CG_MMSYS0], .src = MT_CG_SRC_SMI, }, // auto sel
    // [MT_CG_MM_CMDQ_SW_CG]           = { .name = __stringify(MT_CG_MM_CMDQ_SW_CG),           .cnt = 0, .mask = MM_CMDQ_SW_CG_BIT,            .ops = &general_gate_cg_clk_ops, .grp = &grps[CG_MMSYS0], .src = MT_CG_SRC_SMI, }, // auto sel
    // [MT_CG_MM_CMDQ_SMI_IF_SW_CG]    = { .name = __stringify(MT_CG_MM_CMDQ_SMI_IF_SW_CG),    .cnt = 0, .mask = MM_CMDQ_SMI_IF_SW_CG_BIT,     .ops = &general_gate_cg_clk_ops, .grp = &grps[CG_MMSYS0], .src = MT_CG_SRC_SMI, }, // auto sel
    [MT_CG_DISP_COLOR_SW_CG]        = { .name = __stringify(MT_CG_DISP_COLOR_SW_CG),        .cnt = 0, .mask = DISP_COLOR_SW_CG_BIT,         .ops = &general_gate_cg_clk_ops, .grp = &grps[CG_MMSYS0], .src = MT_CG_SRC_SMI, }, // auto sel
    [MT_CG_DISP_BLS_SW_CG]          = { .name = __stringify(MT_CG_DISP_BLS_SW_CG),          .cnt = 0, .mask = DISP_BLS_SW_CG_BIT,           .ops = &general_gate_cg_clk_ops, .grp = &grps[CG_MMSYS0], .src = MT_CG_SRC_SMI, }, // auto sel
    [MT_CG_DISP_WDMA_SW_CG]         = { .name = __stringify(MT_CG_DISP_WDMA_SW_CG),         .cnt = 0, .mask = DISP_WDMA_SW_CG_BIT,          .ops = &general_gate_cg_clk_ops, .grp = &grps[CG_MMSYS0], .src = MT_CG_SRC_SMI, }, // auto sel
    [MT_CG_DISP_RDMA_SW_CG]         = { .name = __stringify(MT_CG_DISP_RDMA_SW_CG),         .cnt = 0, .mask = DISP_RDMA_SW_CG_BIT,          .ops = &general_gate_cg_clk_ops, .grp = &grps[CG_MMSYS0], .src = MT_CG_SRC_SMI, }, // auto sel
    [MT_CG_DISP_OVL_SW_CG]          = { .name = __stringify(MT_CG_DISP_OVL_SW_CG),          .cnt = 0, .mask = DISP_OVL_SW_CG_BIT,           .ops = &general_gate_cg_clk_ops, .grp = &grps[CG_MMSYS0], .src = MT_CG_SRC_SMI, }, // auto sel
    [MT_CG_MDP_TDSHP_SW_CG]         = { .name = __stringify(MT_CG_MDP_TDSHP_SW_CG),         .cnt = 0, .mask = MDP_TDSHP_SW_CG_BIT,          .ops = &general_gate_cg_clk_ops, .grp = &grps[CG_MMSYS0], .src = MT_CG_SRC_SMI, }, // auto sel
    [MT_CG_MDP_WROT_SW_CG]          = { .name = __stringify(MT_CG_MDP_WROT_SW_CG),          .cnt = 0, .mask = MDP_WROT_SW_CG_BIT,           .ops = &general_gate_cg_clk_ops, .grp = &grps[CG_MMSYS0], .src = MT_CG_SRC_SMI, }, // auto sel
    [MT_CG_MDP_WDMA_SW_CG]          = { .name = __stringify(MT_CG_MDP_WDMA_SW_CG),          .cnt = 0, .mask = MDP_WDMA_SW_CG_BIT,           .ops = &general_gate_cg_clk_ops, .grp = &grps[CG_MMSYS0], .src = MT_CG_SRC_SMI, }, // auto sel
    [MT_CG_MDP_RSZ1_SW_CG]          = { .name = __stringify(MT_CG_MDP_RSZ1_SW_CG),          .cnt = 0, .mask = MDP_RSZ1_SW_CG_BIT,           .ops = &general_gate_cg_clk_ops, .grp = &grps[CG_MMSYS0], .src = MT_CG_SRC_SMI, }, // auto sel
    [MT_CG_MDP_RSZ0_SW_CG]          = { .name = __stringify(MT_CG_MDP_RSZ0_SW_CG),          .cnt = 0, .mask = MDP_RSZ0_SW_CG_BIT,           .ops = &general_gate_cg_clk_ops, .grp = &grps[CG_MMSYS0], .src = MT_CG_SRC_SMI, }, // auto sel
    [MT_CG_MDP_RDMA_SW_CG]          = { .name = __stringify(MT_CG_MDP_RDMA_SW_CG),          .cnt = 0, .mask = MDP_RDMA_SW_CG_BIT,           .ops = &general_gate_cg_clk_ops, .grp = &grps[CG_MMSYS0], .src = MT_CG_SRC_SMI, }, // auto sel
    [MT_CG_MDP_BLS_26M_SW_CG]       = { .name = __stringify(MT_CG_MDP_BLS_26M_SW_CG),       .cnt = 0, .mask = MDP_BLS_26M_SW_CG_BIT,        .ops = &general_gate_cg_clk_ops, .grp = &grps[CG_MMSYS0], .src = MT_CG_INVALID, }, // MT_CG_SYS_26M
    [MT_CG_MM_CAM_SW_CG]            = { .name = __stringify(MT_CG_MM_CAM_SW_CG),            .cnt = 0, .mask = MM_CAM_SW_CG_BIT,             .ops = &general_gate_cg_clk_ops, .grp = &grps[CG_MMSYS0], .src = MT_CG_SRC_SMI, }, // auto sel
    [MT_CG_MM_SENINF_SW_CG]         = { .name = __stringify(MT_CG_MM_SENINF_SW_CG),         .cnt = 0, .mask = MM_SENINF_SW_CG_BIT,          .ops = &general_gate_cg_clk_ops, .grp = &grps[CG_MMSYS0], .src = MT_CG_SRC_SMI, }, // auto sel
    [MT_CG_MM_CAMTG_SW_CG]          = { .name = __stringify(MT_CG_MM_CAMTG_SW_CG),          .cnt = 0, .mask = MM_CAMTG_SW_CG_BIT,           .ops = &general_gate_cg_clk_ops, .grp = &grps[CG_MMSYS0], .src = MT_CG_CAM_MM_SW_CG, },
    [MT_CG_MM_CODEC_SW_CG]          = { .name = __stringify(MT_CG_MM_CODEC_SW_CG),          .cnt = 0, .mask = MM_CODEC_SW_CG_BIT,           .ops = &general_gate_cg_clk_ops, .grp = &grps[CG_MMSYS0], .src = MT_CG_SRC_SMI, }, // auto sel
    [MT_CG_DISP_FAKE_ENG_SW_CG]     = { .name = __stringify(MT_CG_DISP_FAKE_ENG_SW_CG),     .cnt = 0, .mask = DISP_FAKE_ENG_SW_CG_BIT,      .ops = &general_gate_cg_clk_ops, .grp = &grps[CG_MMSYS0], .src = MT_CG_SRC_SMI, }, // auto sel
    [MT_CG_MUTEX_SLOW_CLOCK_SW_CG]  = { .name = __stringify(MT_CG_MUTEX_SLOW_CLOCK_SW_CG),  .cnt = 0, .mask = MUTEX_SLOW_CLOCK_SW_CG_BIT,   .ops = &general_gate_cg_clk_ops, .grp = &grps[CG_MMSYS0], .src = MT_CG_INVALID, }, // MT_CG_SYS_32K

    // CG_MMSYS1
    [MT_CG_DSI_ENGINE_SW_CG]        = { .name = __stringify(MT_CG_DSI_ENGINE_SW_CG),        .cnt = 0, .mask = DSI_ENGINE_SW_CG_BIT,         .ops = &general_gate_cg_clk_ops, .grp = &grps[CG_MMSYS1], .src = MT_CG_SRC_SMI,        }, // auto sel
    [MT_CG_DSI_DIGITAL_SW_CG]       = { .name = __stringify(MT_CG_DSI_DIGITAL_SW_CG),       .cnt = 0, .mask = DSI_DIGITAL_SW_CG_BIT,        .ops = &general_gate_cg_clk_ops, .grp = &grps[CG_MMSYS1], .src = MT_CG_INVALID,        }, // from MIPI
    [MT_CG_DISP_DPI_ENGINE_SW_CG]   = { .name = __stringify(MT_CG_DISP_DPI_ENGINE_SW_CG),   .cnt = 0, .mask = DISP_DPI_ENGINE_SW_CG_BIT,    .ops = &general_gate_cg_clk_ops, .grp = &grps[CG_MMSYS1], .src = MT_CG_SRC_SMI,   }, // auto sel
    [MT_CG_DISP_DPI_IF_SW_CG]       = { .name = __stringify(MT_CG_DISP_DPI_IF_SW_CG),       .cnt = 0, .mask = DISP_DPI_IF_SW_CG_BIT,        .ops = &general_gate_cg_clk_ops, .grp = &grps[CG_MMSYS1], .src = MT_CG_INVALID,        }, // from MIPI
    [MT_CG_DISP_DBI_ENGINE_SW_CG]   = { .name = __stringify(MT_CG_DISP_DBI_ENGINE_SW_CG),   .cnt = 0, .mask = DISP_DBI_ENGINE_SW_CG_BIT,    .ops = &general_gate_cg_clk_ops, .grp = &grps[CG_MMSYS1], .src = MT_CG_SRC_SMI,   }, // auto sel
    [MT_CG_DISP_DBI_SMI_SW_CG]      = { .name = __stringify(MT_CG_DISP_DBI_SMI_SW_CG),      .cnt = 0, .mask = DISP_DBI_SMI_SW_CG_BIT,       .ops = &general_gate_cg_clk_ops, .grp = &grps[CG_MMSYS1], .src = MT_CG_SRC_SMI,        }, // auto sel
    [MT_CG_DISP_DBI_IF_SW_CG]       = { .name = __stringify(MT_CG_DISP_DBI_IF_SW_CG),       .cnt = 0, .mask = DISP_DBI_IF_SW_CG_BIT,        .ops = &general_gate_cg_clk_ops, .grp = &grps[CG_MMSYS1], .src = MT_CG_DBI_BCLK_SW_CG, },

    // CG_MFG
    [MT_CG_MFG_PDN_BG3D_SW_CG]      = { .name = __stringify(MT_CG_MFG_PDN_BG3D_SW_CG),      .cnt = 0, .mask = MFG_PDN_BG3D_SW_CG_BIT,       .ops = &general_gate_cg_clk_ops, .grp = &grps[CG_MFG],    .src = MT_CG_MFG_MM_SW_CG, }, // XXX: .ops = &mfg_cg_clk_ops

    // CG_AUDIO
    [MT_CG_AUD_PDN_AFE_EN]          = { .name = __stringify(MT_CG_AUD_PDN_AFE_EN),          .cnt = 0, .mask = AUD_PDN_AFE_EN_BIT,           .ops = &wo_clr_cg_clk_ops,       .grp = &grps[CG_AUDIO], .src = MT_CG_AUDIO_SW_CG, },
    [MT_CG_AUD_PDN_I2S_EN]          = { .name = __stringify(MT_CG_AUD_PDN_I2S_EN),          .cnt = 0, .mask = AUD_PDN_I2S_EN_BIT,           .ops = &wo_clr_cg_clk_ops,       .grp = &grps[CG_AUDIO], .src = MT_CG_INVALID,     }, // from connsys
    [MT_CG_AUD_PDN_ADC_EN]          = { .name = __stringify(MT_CG_AUD_PDN_ADC_EN),          .cnt = 0, .mask = AUD_PDN_ADC_EN_BIT,           .ops = &wo_clr_cg_clk_ops,       .grp = &grps[CG_AUDIO], .src = MT_CG_INVALID,     }, // AXIBUS
    [MT_CG_AUD_PDN_DAC_EN]          = { .name = __stringify(MT_CG_AUD_PDN_DAC_EN),          .cnt = 0, .mask = AUD_PDN_DAC_EN_BIT,           .ops = &wo_clr_cg_clk_ops,       .grp = &grps[CG_AUDIO], .src = MT_CG_INVALID,     }, // AXIBUS
    [MT_CG_AUD_PDN_DAC_PREDIS_EN]   = { .name = __stringify(MT_CG_AUD_PDN_DAC_PREDIS_EN),   .cnt = 0, .mask = AUD_PDN_DAC_PREDIS_EN_BIT,    .ops = &wo_clr_cg_clk_ops,       .grp = &grps[CG_AUDIO], .src = MT_CG_INVALID,     }, // AXIBUS
    [MT_CG_AUD_PDN_TML_EN]          = { .name = __stringify(MT_CG_AUD_PDN_TML_EN),          .cnt = 0, .mask = AUD_PDN_TML_EN_BIT,           .ops = &wo_clr_cg_clk_ops,       .grp = &grps[CG_AUDIO], .src = MT_CG_INVALID,     }, // AXIBUS
};

static struct cg_clk *id_to_clk(cg_clk_id id)
{
    return id < NR_CLKS ? &clks[id] : NULL;
}

// general_cg_clk_ops

static int general_cg_clk_check_validity_op(struct cg_clk *clk)
{
    int valid = 0;

    ENTER_FUNC(FUNC_LV_OP);

    if (clk->mask & clk->grp->mask)
    {
        valid = 1;
    }

    EXIT_FUNC(FUNC_LV_OP);
    return valid;
}

// general_gate_cg_clk_ops

static int general_gate_cg_clk_get_state_op(struct cg_clk *clk)
{
    struct subsys *sys = clk->grp->sys;

    ENTER_FUNC(FUNC_LV_OP);

    EXIT_FUNC(FUNC_LV_OP);

    if (sys && !sys->state)
    {
        return PWR_DOWN;
    }
    else
    {
        return (clk_readl(clk->grp->sta_addr) & (clk->mask)) ? PWR_DOWN : PWR_ON ; // clock gate
    }
}

static int general_gate_cg_clk_enable_op(struct cg_clk *clk)
{
    ENTER_FUNC(FUNC_LV_OP);

    clk_writel(clk->grp->clr_addr, clk->mask); // clock gate

    EXIT_FUNC(FUNC_LV_OP);
    return 0;
}

static int general_gate_cg_clk_disable_op(struct cg_clk *clk)
{
    ENTER_FUNC(FUNC_LV_OP);

    clk_writel(clk->grp->set_addr, clk->mask); // clock gate

    EXIT_FUNC(FUNC_LV_OP);
    return 0;
}

static struct cg_clk_ops general_gate_cg_clk_ops =
{
    .get_state      = general_gate_cg_clk_get_state_op,
    .check_validity = general_cg_clk_check_validity_op,
    .enable         = general_gate_cg_clk_enable_op,
    .disable        = general_gate_cg_clk_disable_op,
};

// general_en_cg_clk_ops

static int general_en_cg_clk_get_state_op(struct cg_clk *clk)
{
    struct subsys *sys = clk->grp->sys;

    ENTER_FUNC(FUNC_LV_OP);

    EXIT_FUNC(FUNC_LV_OP);

    if (sys && !sys->state)
    {
        return PWR_DOWN;
    }
    else
    {
        return (clk_readl(clk->grp->sta_addr) & (clk->mask)) ? PWR_ON : PWR_DOWN ; // clock enable
    }
}

static int general_en_cg_clk_enable_op(struct cg_clk *clk)
{
    ENTER_FUNC(FUNC_LV_OP);

    clk_writel(clk->grp->set_addr, clk->mask); // clock enable

    EXIT_FUNC(FUNC_LV_OP);
    return 0;
}

static int general_en_cg_clk_disable_op(struct cg_clk *clk)
{
    ENTER_FUNC(FUNC_LV_OP);

    clk_writel(clk->grp->clr_addr, clk->mask); // clock enable

    EXIT_FUNC(FUNC_LV_OP);
    return 0;
}

static struct cg_clk_ops general_en_cg_clk_ops =
{
    .get_state      = general_en_cg_clk_get_state_op,
    .check_validity = general_cg_clk_check_validity_op,
    .enable         = general_en_cg_clk_enable_op,
    .disable        = general_en_cg_clk_disable_op,
};

// wo_clr_cg_clk_ops

static int wo_clr_cg_clk_disable_op(struct cg_clk *clk)
{
    volatile unsigned int val = clk_readl(clk->grp->clr_addr) & ~clk->mask; // without clr reg actually

    ENTER_FUNC(FUNC_LV_OP);

    clk_writel(clk->grp->clr_addr, val);

    EXIT_FUNC(FUNC_LV_OP);
    return 0;
}

static struct cg_clk_ops wo_clr_cg_clk_ops =
{
    .get_state      = general_en_cg_clk_get_state_op,
    .check_validity = general_cg_clk_check_validity_op,
    .enable         = general_en_cg_clk_enable_op,
    .disable        = wo_clr_cg_clk_disable_op,
};

// ao_clk_ops

static int ao_cg_clk_get_state_op(struct cg_clk *clk)
{
    ENTER_FUNC(FUNC_LV_OP);
    EXIT_FUNC(FUNC_LV_OP);
    return PWR_ON;
}

static int ao_cg_clk_check_validity_op(struct cg_clk *clk)
{
    ENTER_FUNC(FUNC_LV_OP);
    EXIT_FUNC(FUNC_LV_OP);
    return 1;
}

static int ao_cg_clk_enable_op(struct cg_clk *clk)
{
    ENTER_FUNC(FUNC_LV_OP);
    EXIT_FUNC(FUNC_LV_OP);
    return 0;
}

static int ao_cg_clk_disable_op(struct cg_clk *clk)
{
    ENTER_FUNC(FUNC_LV_OP);
    EXIT_FUNC(FUNC_LV_OP);
    return 0;
}

static struct cg_clk_ops ao_cg_clk_ops =
{
    .get_state      = ao_cg_clk_get_state_op,
    .check_validity = ao_cg_clk_check_validity_op,
    .enable         = ao_cg_clk_enable_op,
    .disable        = ao_cg_clk_disable_op,
};

// mfg_cg_clk_ops
#if 0	/* not used */
static int mfg_cg_clk_get_state_op(struct cg_clk *clk)
{
    ENTER_FUNC(FUNC_LV_OP);
    BUG_ON(NULL == clk || NULL == clk->grp || NULL == clk->grp->sys);
    EXIT_FUNC(FUNC_LV_OP);
    return clk->grp->sys->ops->get_state(clk->grp->sys);
}

static struct cg_clk_ops mfg_cg_clk_ops =
{
    .get_state      = mfg_cg_clk_get_state_op,
    .check_validity = ao_cg_clk_check_validity_op,
    .enable         = ao_cg_clk_enable_op,
    .disable        = ao_cg_clk_disable_op,
};
#endif	/* not used */
//
// CG_GRP variable & function
//
static struct cg_grp_ops general_en_cg_grp_ops;
static struct cg_grp_ops general_gate_cg_grp_ops;
static struct cg_grp_ops ctrl0_cg_grp_ops;
// static struct cg_grp_ops mfg_cg_grp_ops; /* not used */

static struct cg_grp grps[] = // NR_GRPS
{
    [CG_MIXED] =
    {
        .name       = __stringify(CG_MIXED),
        .set_addr   = UNIVPLL_CON0,
        .clr_addr   = UNIVPLL_CON0,
        .sta_addr   = UNIVPLL_CON0,
        .mask       = 0, // CG_MIXED_MASK,    // TODO: set @ init is better
        .ops        = &general_en_cg_grp_ops,
        .sys        = NULL,
    },
    [CG_MPLL] =
    {
        .name       = __stringify(CG_MPLL),
        .set_addr   = SET_MPLL_FREDIV_EN,
        .clr_addr   = CLR_MPLL_FREDIV_EN,
        .sta_addr   = MPLL_FREDIV_EN,
        .mask       = 0, // CG_MPLL_MASK,     // TODO: set @ init is better
        .ops        = &general_en_cg_grp_ops,
        .sys        = NULL,
    },
    [CG_UPLL] =
    {
        .name       = __stringify(CG_UPLL),
        .set_addr   = SET_UPLL_FREDIV_EN,
        .clr_addr   = CLR_UPLL_FREDIV_EN,
        .sta_addr   = UPLL_FREDIV_EN,
        .mask       = 0, // CG_UPLL_MASK,     // TODO: set @ init is better
        .ops        = &general_en_cg_grp_ops,
        .sys        = NULL,
    },
    [CG_CTRL0] =
    {
        .name       = __stringify(CG_CTRL0),
        .set_addr   = SET_CLK_GATING_CTRL0,
        .clr_addr   = CLR_CLK_GATING_CTRL0,
        .sta_addr   = CLK_GATING_CTRL0,
        .mask       = 0, // CG_CTRL0_MASK,    // TODO: set @ init is better
        .ops        = &ctrl0_cg_grp_ops, // XXX: not all are clock gate (e.g. MIPI_26M_DBG_EN_BIT, SC_26M_CK_SEL_EN_BIT, SC_MEM_CK_OFF_EN_BIT, GPU_491P52M_EN_BIT, GPU_500P5M_EN_BIT, ARMDCM_CLKOFF_EN_BIT)
        .sys        = NULL,
    },
    [CG_CTRL1] =
    {
        .name       = __stringify(CG_CTRL1),
        .set_addr   = SET_CLK_GATING_CTRL1,
        .clr_addr   = CLR_CLK_GATING_CTRL1,
        .sta_addr   = CLK_GATING_CTRL1,
        .mask       = 0, // CG_CTRL1_MASK,    // TODO: set @ init is better
        .ops        = &general_gate_cg_grp_ops,
        .sys        = NULL,
    },
    [CG_MMSYS0] =
    {
        .name       = __stringify(CG_MMSYS0),
        .set_addr   = MMSYS_CG_SET0,
        .clr_addr   = MMSYS_CG_CLR0,
        .sta_addr   = MMSYS_CG_CON0,
        .mask       = 0, // CG_MMSYS0_MASK,   // TODO: set @ init is better
        .ops        = &general_gate_cg_grp_ops,
        .sys        = &syss[SYS_DIS],
    },
    [CG_MMSYS1] =
    {
        .name       = __stringify(CG_MMSYS1),
        .set_addr   = MMSYS_CG_SET1,
        .clr_addr   = MMSYS_CG_CLR1,
        .sta_addr   = MMSYS_CG_CON1,
        .mask       = 0, // CG_MMSYS1_MASK,   // TODO: set @ init is better
        .ops        = &general_gate_cg_grp_ops,
        .sys        = &syss[SYS_DIS],
    },
    [CG_MFG] =
    {
        .name       = __stringify(CG_MFG),
        .set_addr   = MFG_CG_SET,
        .clr_addr   = MFG_CG_CLR,
        .sta_addr   = MFG_CG_CON,
        .mask       = 0, // CG_MFG_MASK,      // TODO: set @ init is better
        .ops        = &general_gate_cg_grp_ops,
        .sys        = &syss[SYS_MFG],
    },
    [CG_AUDIO] =
    {
        .name       = __stringify(CG_AUDIO),
        .set_addr   = AUDIO_TOP_CON0,
        .clr_addr   = AUDIO_TOP_CON0,
        .sta_addr   = AUDIO_TOP_CON0,
        .mask       = 0, // CG_AUDIO_MASK,    // TODO: set @ init is better
        .ops        = &general_en_cg_grp_ops,
        .sys        = NULL,
    },
};

static struct cg_grp *id_to_grp(cg_grp_id id)
{
    return id < NR_GRPS ? &grps[id] : NULL;
}

// general_cg_grp

static int general_cg_grp_dump_regs_op(struct cg_grp *grp, unsigned int *ptr)
{
    ENTER_FUNC(FUNC_LV_OP);

    *(ptr) = clk_readl(grp->sta_addr);

    EXIT_FUNC(FUNC_LV_OP);
    return 1; // return size
}

// general_gate_cg_grp

static unsigned int general_gate_cg_grp_get_state_op(struct cg_grp *grp)
{
    volatile unsigned int val;
    struct subsys *sys = grp->sys;

    ENTER_FUNC(FUNC_LV_OP);

    EXIT_FUNC(FUNC_LV_OP);

    if (sys && !sys->state)
    {
        return 0;
    }
    else
    {
        val = clk_readl(grp->sta_addr);
        val = (~val) & (grp->mask); // clock gate

        return val;
    }
}

static struct cg_grp_ops general_gate_cg_grp_ops =
{
    .get_state = general_gate_cg_grp_get_state_op,
    .dump_regs = general_cg_grp_dump_regs_op,
};

// general_en_cg_grp

static unsigned int general_en_cg_grp_get_state_op(struct cg_grp *grp)
{
    volatile unsigned int val;
    struct subsys *sys = grp->sys;

    ENTER_FUNC(FUNC_LV_OP);

    EXIT_FUNC(FUNC_LV_OP);

    if (sys && !sys->state)
    {
        return 0;
    }
    else
    {
        val = clk_readl(grp->sta_addr);
        val &= (grp->mask); // clock enable

        return val;
    }
}

static struct cg_grp_ops general_en_cg_grp_ops =
{
    .get_state = general_en_cg_grp_get_state_op,
    .dump_regs = general_cg_grp_dump_regs_op,
};

// ctrl0_cg_grp

static unsigned int ctrl0_cg_grp_get_state_op(struct cg_grp *grp)
{
    volatile unsigned int val;
    struct subsys *sys = grp->sys;

    ENTER_FUNC(FUNC_LV_OP);

    EXIT_FUNC(FUNC_LV_OP);

    if (sys && !sys->state)
    {
        return 0;
    }
    else
    {
        val = clk_readl(grp->sta_addr);
        val = (val ^ ~(CG_CTRL0_EN_MASK)) & (grp->mask); // XXX some are enable bit and others are gate bit

        return val;
    }
}

static struct cg_grp_ops ctrl0_cg_grp_ops =
{
    .get_state = ctrl0_cg_grp_get_state_op,
    .dump_regs = general_cg_grp_dump_regs_op,
};

// mfg_cg_grp
#if 0	/* not used */
static int get_clk_state_locked(struct cg_clk *clk);

static int mfg_cg_grp_dump_regs_op(struct cg_grp *grp, unsigned int *ptr)
{
    struct cg_clk *clk = id_to_clk(MT_CG_MFG_MM_SW_CG);

    ENTER_FUNC(FUNC_LV_OP);

    if (   NULL != clk
        && PWR_ON == get_clk_state_locked(clk) // XXX: clock_is_on() locked version
        )
    {
        *(ptr) = clk_readl(grp->sta_addr);
    }
    else
    {
        *(ptr) = 0x00000000; // XXX: default value
    }

    EXIT_FUNC(FUNC_LV_OP);
    return 1; // return size
}

static struct cg_grp_ops mfg_cg_grp_ops =
{
    .get_state = general_gate_cg_grp_get_state_op,
    .dump_regs = mfg_cg_grp_dump_regs_op,
};
#endif	/* not used */
//
// enable_clock() / disable_clock()
//
static int enable_pll_locked(struct pll *pll);
static int disable_pll_locked(struct pll *pll);

static int enable_subsys_locked(struct subsys *sys);
static int disable_subsys_locked(struct subsys *sys, int force_off);

static int power_prepare_locked(struct cg_grp *grp)
{
    int err = 0;

    ENTER_FUNC(FUNC_LV_BODY);

    if (grp->sys)
    {
        err = enable_subsys_locked(grp->sys);
    }

    EXIT_FUNC(FUNC_LV_BODY);
    return err;
}

static int power_finish_locked(struct cg_grp *grp)
{
    int err = 0;

    ENTER_FUNC(FUNC_LV_BODY);

    if (grp->sys)
    {
        err = disable_subsys_locked(grp->sys, 0); // NOT force off
    }

    EXIT_FUNC(FUNC_LV_BODY);
    return err;
}

static int enable_clock_locked(struct cg_clk *clk)
{
    struct cg_grp *grp;
    unsigned int local_state;
#ifdef STATE_CHECK_DEBUG
    unsigned int reg_state;
#endif
    int err;

    ENTER_FUNC(FUNC_LV_LOCKED);

    if (NULL == clk)
    {
        EXIT_FUNC(FUNC_LV_LOCKED);
        return -1;
    }

    clk->cnt++;

    // clk_info("%s[%d]\n", __FUNCTION__, clk - &clks[0]); // <-XXX

    if (clk->cnt > 1)
    {
        EXIT_FUNC(FUNC_LV_LOCKED);
        return 0; // XXX: have enabled and return directly
    }

    local_state = clk->state;
    grp = clk->grp;

#ifdef STATE_CHECK_DEBUG
    reg_state = clk->ops->get_state(clk);
    BUG_ON(local_state != reg_state);
#endif

    // step 1: pll check
    if (clk->parent)
    {
        enable_pll_locked(clk->parent);
    }

    // step 3: source clock check
    if (clk->src < NR_CLKS) // TODO: please use macro VALID_CG_CLK() for this (e.g. #define VALID_CG_CLK(cg_clk_id) ((cg_clk_id) < NR_CLKS))
    {
        enable_clock_locked(id_to_clk(clk->src));
    }

    // step 2: subsys check
    err = power_prepare_locked(grp);
    BUG_ON(err);

    if (local_state == PWR_ON)
    {
        EXIT_FUNC(FUNC_LV_LOCKED);
        return 0; // XXX: assume local_state & reg_state are the same
    }

    // step 4: local clock enable
    clk->ops->enable(clk);

    clk->state = PWR_ON;
    grp->state |= clk->mask;

    EXIT_FUNC(FUNC_LV_LOCKED);
    return 0;
}

static int disable_clock_locked(struct cg_clk *clk)
{
    struct cg_grp *grp;
    unsigned int local_state;
#ifdef STATE_CHECK_DEBUG
    unsigned int reg_state;
#endif
    int err;

    ENTER_FUNC(FUNC_LV_LOCKED);

    if (NULL == clk)
    {
        EXIT_FUNC(FUNC_LV_LOCKED);
        return -1;
    }

    if (!clk->cnt) printk(KERN_EMERG "Assert @ %s\n", clk->name);
    BUG_ON(!clk->cnt);
    clk->cnt--;

    // clk_info("%s[%d]\n", __FUNCTION__, clk - &clks[0]); // <-XXX

    if (clk->cnt > 0)
    {
        EXIT_FUNC(FUNC_LV_LOCKED);
        return 0; // XXX: not count down zero and return directly
    }

    local_state = clk->state;
    grp = clk->grp;

#ifdef STATE_CHECK_DEBUG
    reg_state = clk->ops->get_state(clk);
    BUG_ON(local_state != reg_state);
#endif

    if (local_state == PWR_DOWN)
    {
        EXIT_FUNC(FUNC_LV_LOCKED);
        return 0; // XXX: assume local_state & reg_state are the same
    }

    // step 1: local clock disable
    clk->ops->disable(clk);

    clk->state = PWR_DOWN;
    grp->state &= ~(clk->mask);

    // step 3: subsys check
    err = power_finish_locked(grp);
    BUG_ON(err);

    // step 2: source clock check
    if (clk->src < NR_CLKS) // TODO: please use macro VALID_CG_CLK() for this (e.g. #define VALID_CG_CLK(cg_clk_id) ((cg_clk_id) < NR_CLKS))
    {
        disable_clock_locked(id_to_clk(clk->src));
    }

    // step 4: pll check
    if (clk->parent)
    {
        disable_pll_locked(clk->parent);
    }

    EXIT_FUNC(FUNC_LV_LOCKED);
    return 0;
}

static int get_clk_state_locked(struct cg_clk *clk)
{
    if (likely(initialized))
    {
        return clk->state;
    }
    else
    {
        return clk->ops->get_state(clk);
    }
}

#if 1   // XXX: debug for MT_CG_APDMA_SW_CG
static struct
{
    int cnt;
    const char *name;
} clk_cnt_rec[] =
{
    [0] = { .cnt = 0, .name = "WLAN",        },
    [1] = { .cnt = 0, .name = "btif_driver", },
    [2] = { .cnt = 0, .name = "mt-i2c",      },
    [3] = { .cnt = 0, .name = "VFIFO",       },
    [4] = { .cnt = 0, .name = "UNKNOWN",     },
};
#endif  // XXX: debug for MT_CG_APDMA_SW_CG

int mt_enable_clock(enum cg_clk_id id, const char *name)
{
    int err;
    unsigned long flags;
    struct cg_clk *clk = id_to_clk(id);

    ENTER_FUNC(FUNC_LV_API);

#if defined(CONFIG_CLKMGR_EMULATION)

    return 0;

#else   // CONFIG_CLKMGR_EMULATION

    BUG_ON(!initialized);
    BUG_ON(!clk);
    BUG_ON(!clk->grp);
    BUG_ON(!clk->ops->check_validity(clk));

    clkmgr_lock(flags);
#if 1   // XXX: debug for MT_CG_APDMA_SW_CG
    {
        int i;

        if (MT_CG_APDMA_SW_CG == id)
        {
            for (i = 0; i < ARRAY_SIZE(clk_cnt_rec) - 1; i++)
            {
                if (0 == strcmp(clk_cnt_rec[i].name, name))
                {
                    break;
                }
            }

            clk_cnt_rec[i].cnt++;
        }
    }
#endif  // XXX: debug for MT_CG_APDMA_SW_CG
    err = enable_clock_locked(clk);
    clkmgr_unlock(flags);

    EXIT_FUNC(FUNC_LV_API);
    return err;

#endif  // CONFIG_CLKMGR_EMULATION
}
EXPORT_SYMBOL(mt_enable_clock);

int mt_disable_clock(enum cg_clk_id id, const char *name)
{
    int err;
    unsigned long flags;
    struct cg_clk *clk = id_to_clk(id);

    ENTER_FUNC(FUNC_LV_API);

#if defined(CONFIG_CLKMGR_EMULATION)

    return 0;

#else   // CONFIG_CLKMGR_EMULATION

    BUG_ON(!initialized);
    BUG_ON(!clk);
    BUG_ON(!clk->grp);
    BUG_ON(!clk->ops->check_validity(clk));

    clkmgr_lock(flags);
#if 1   // XXX: debug for MT_CG_APDMA_SW_CG
    {
        int i;

        if (MT_CG_APDMA_SW_CG == id)
        {
            for (i = 0; i < ARRAY_SIZE(clk_cnt_rec) - 1; i++)
            {
                if (0 == strcmp(clk_cnt_rec[i].name, name))
                {
                    break;
                }
            }

            if (!clk_cnt_rec[i].cnt) printk(KERN_EMERG "Assert @ %s\n", clk_cnt_rec[i].name);
            BUG_ON(!clk_cnt_rec[i].cnt);

            clk_cnt_rec[i].cnt--;
        }
    }
#endif  // XXX: debug for MT_CG_APDMA_SW_CG
    err = disable_clock_locked(clk);
    clkmgr_unlock(flags);

    EXIT_FUNC(FUNC_LV_API);
    return err;

#endif  // CONFIG_CLKMGR_EMULATION
}
EXPORT_SYMBOL(mt_disable_clock);

int clock_is_on(cg_clk_id id)
{
    int state;
    unsigned long flags;
    struct cg_clk *clk = id_to_clk(id);

    ENTER_FUNC(FUNC_LV_API);

#if defined(CONFIG_CLKMGR_EMULATION)

    return 0;

#else   // CONFIG_CLKMGR_EMULATION

    BUG_ON(!clk);
    BUG_ON(!clk->grp);
    BUG_ON(!clk->ops->check_validity(clk));

    clkmgr_lock(flags);
    state = get_clk_state_locked(clk);
    clkmgr_unlock(flags);

    EXIT_FUNC(FUNC_LV_API);
    return state;

#endif  // CONFIG_CLKMGR_EMULATION
}
EXPORT_SYMBOL(clock_is_on);

int grp_dump_regs(cg_grp_id id, unsigned int *ptr)
{
    struct cg_grp *grp = id_to_grp(id);

    ENTER_FUNC(FUNC_LV_DONT_CARE);

    // BUG_ON(!initialized);
    BUG_ON(!grp);

    EXIT_FUNC(FUNC_LV_DONT_CARE);
    return grp->ops->dump_regs(grp, ptr);
}
EXPORT_SYMBOL(grp_dump_regs);

const char *grp_get_name(cg_grp_id id)
{
    struct cg_grp *grp = id_to_grp(id);

    ENTER_FUNC(FUNC_LV_DONT_CARE);

    // BUG_ON(!initialized);
    BUG_ON(!grp);

    EXIT_FUNC(FUNC_LV_DONT_CARE);
    return grp->name;
}

cg_grp_id clk_id_to_grp_id(cg_clk_id id)
{
    struct cg_clk *clk;

    clk = id_to_clk(id);

    return (NULL == clk) ? NR_GRPS : (clk->grp - &grps[0]);
}

unsigned int clk_id_to_mask(cg_clk_id id)
{
    struct cg_clk *clk;

    clk = id_to_clk(id);

    return (NULL == clk) ? 0 : clk->mask;
}

//
// CLKMUX variable & function
//
static struct clkmux_ops non_gf_clkmux_ops;
static struct clkmux_ops glitch_free_clkmux_ops;
static struct clkmux_ops mfg_glitch_free_clkmux_ops;
static struct clkmux_ops glitch_free_wo_drain_cg_clkmux_ops;
static struct clkmux_ops pmicspi_clkmux_ops;
static struct clkmux_ops aud_intbus_clkmux_ops;

static const struct clkmux_map _mt_clkmux_uart0_gfmux_sel_map[] =
{
    { .val = BITS(0: 0, 0), .id = MT_CG_SYS_26M,  .mask = BITMASK(0:0), }, // default
    { .val = BITS(0: 0, 1), .id = MT_CG_UPLL_D24, .mask = BITMASK(0:0), },
};
static const struct clkmux_map _mt_clkmux_emi2x_gfmux_sel_map[] =
{
    { .val = BITS(4: 4, 0),  .id = MT_CG_SYS_26M, .mask = BITMASK(4: 4), }, // default
    { .val = BITS(4: 1, 9),  .id = MT_CG_MPLL_D3, .mask = BITMASK(4: 1), },
    { .val = BITS(4: 1, 10), .id = MT_CG_MPLL_D4, .mask = BITMASK(4: 1), },
    { .val = BITS(4: 1, 12), .id = MT_CG_MPLL_D2, .mask = BITMASK(4: 1), },
};
static const struct clkmux_map _mt_clkmux_axibus_gfmux_sel_map[] =
{
    { .val = BITS(7: 5, 1), .id = MT_CG_SYS_26M,  .mask = BITMASK(7: 5), }, // default
    { .val = BITS(7: 5, 2), .id = MT_CG_MPLL_D10, .mask = BITMASK(7: 5), },
    { .val = BITS(7: 5, 4), .id = MT_CG_MPLL_D12, .mask = BITMASK(7: 5), },
};
static const struct clkmux_map _mt_clkmux_mfg_mux_sel_map[] =
{
    { .val = BITS(31: 31, 0),                                   .id = MT_CG_UPLL_D3,        .mask = BITMASK(31: 31),                  }, // default
    { .val = BITS(31: 31, 1) | BITS(10: 8, 0),                  .id = MT_CG_GPU_491P52M_EN, .mask = BITMASK(31: 31) | BITMASK(10: 8), },
    { .val = BITS(31: 31, 1) | BITS(10: 8, 1),                  .id = MT_CG_GPU_500P5M_EN,  .mask = BITMASK(31: 31) | BITMASK(10: 8), },
    { .val = BITS(31: 31, 1) | BITS(10: 8, 2),                  .id = MT_CG_MPLL_D3,        .mask = BITMASK(31: 31) | BITMASK(10: 8), },
    { .val = BITS(31: 31, 1) | BITS(10: 8, 3),                  .id = MT_CG_UPLL_D2,        .mask = BITMASK(31: 31) | BITMASK(10: 8), },
    { .val = BITS(31: 31, 1) | BITS(10: 10, 1) | BITS(8: 8, 0), .id = MT_CG_SYS_26M,        .mask = BITMASK(31: 31) | BITMASK(10: 10) | BITMASK(8: 8), },
    { .val = BITS(31: 31, 1) | BITS(10: 10, 1) | BITS(8: 8, 1), .id = MT_CG_MPLL_D2,        .mask = BITMASK(31: 31) | BITMASK(10: 10) | BITMASK(8: 8), },
};
static const struct clkmux_map _mt_clkmux_msdc0_mux_sel_map[] =
{
    { .val = BITS(13: 11, 0), .id = MT_CG_MPLL_D12, .mask = BITMASK(13: 11), },
    { .val = BITS(13: 11, 1), .id = MT_CG_MPLL_D10, .mask = BITMASK(13: 11), },
    { .val = BITS(13: 11, 2), .id = MT_CG_MPLL_D8,  .mask = BITMASK(13: 11), },
    { .val = BITS(13: 11, 3), .id = MT_CG_UPLL_D7,  .mask = BITMASK(13: 11), },
    { .val = BITS(13: 11, 4), .id = MT_CG_MPLL_D7,  .mask = BITMASK(13: 11), },
    { .val = BITS(13: 11, 5), .id = MT_CG_MPLL_D8,  .mask = BITMASK(13: 11), },
    { .val = BITS(13: 11, 6), .id = MT_CG_SYS_26M,  .mask = BITMASK(13: 11), }, // default
    { .val = BITS(13: 11, 7), .id = MT_CG_UPLL_D6,  .mask = BITMASK(13: 11), },
};
static const struct clkmux_map _mt_clkmux_spinfi_mux_sel_map[] =
{
    { .val = BITS(30: 30, 0),                   .id = MT_CG_SYS_26M,  .mask = BITMASK(30: 30),                   }, // default
    { .val = BITS(30: 30, 1) | BITS(16: 14, 0), .id = MT_CG_MPLL_D24, .mask = BITMASK(30: 30) | BITMASK(16: 14), },
    { .val = BITS(30: 30, 1) | BITS(16: 14, 1), .id = MT_CG_MPLL_D20, .mask = BITMASK(30: 30) | BITMASK(16: 14), },
    { .val = BITS(30: 30, 1) | BITS(16: 14, 2), .id = MT_CG_UPLL_D20, .mask = BITMASK(30: 30) | BITMASK(16: 14), },
    { .val = BITS(30: 30, 1) | BITS(16: 14, 3), .id = MT_CG_UPLL_D16, .mask = BITMASK(30: 30) | BITMASK(16: 14), },
    { .val = BITS(30: 30, 1) | BITS(16: 14, 4), .id = MT_CG_UPLL_D12, .mask = BITMASK(30: 30) | BITMASK(16: 14), },
    { .val = BITS(30: 30, 1) | BITS(16: 14, 5), .id = MT_CG_UPLL_D10, .mask = BITMASK(30: 30) | BITMASK(16: 14), },
    { .val = BITS(30: 30, 1) | BITS(16: 14, 6), .id = MT_CG_MPLL_D12, .mask = BITMASK(30: 30) | BITMASK(16: 14), },
    { .val = BITS(30: 30, 1) | BITS(16: 14, 7), .id = MT_CG_MPLL_D10, .mask = BITMASK(30: 30) | BITMASK(16: 14), },
};
static const struct clkmux_map _mt_clkmux_cam_mux_sel_map[] =
{
    { .val = BITS(17: 17, 0), .id = MT_CG_UNIV_48M, .mask = BITMASK(17: 17), }, // default
    { .val = BITS(17: 17, 1), .id = MT_CG_UPLL_D6,  .mask = BITMASK(17: 17), },
};
static const struct clkmux_map _mt_clkmux_pwm_mm_mux_sel_map[] =
{
    { .val = BITS(18: 18, 0), .id = MT_CG_SYS_26M,  .mask = BITMASK(18: 18), }, // default
    { .val = BITS(18: 18, 1), .id = MT_CG_UPLL_D12, .mask = BITMASK(18: 18), },
};
static const struct clkmux_map _mt_clkmux_uart1_gfmux_sel_map[] =
{
    { .val = BITS(19: 19, 0), .id = MT_CG_SYS_26M,  .mask = BITMASK(19: 19), }, // default
    { .val = BITS(19: 19, 1), .id = MT_CG_UPLL_D24, .mask = BITMASK(19: 19), },
};
static const struct clkmux_map _mt_clkmux_msdc1_mux_sel_map[] =
{
    { .val = BITS(22: 20, 0), .id = MT_CG_MPLL_D12, .mask = BITMASK(22: 20), },
    { .val = BITS(22: 20, 1), .id = MT_CG_MPLL_D10, .mask = BITMASK(22: 20), },
    { .val = BITS(22: 20, 2), .id = MT_CG_MPLL_D8,  .mask = BITMASK(22: 20), },
    { .val = BITS(22: 20, 3), .id = MT_CG_UPLL_D7,  .mask = BITMASK(22: 20), },
    { .val = BITS(22: 20, 4), .id = MT_CG_MPLL_D7,  .mask = BITMASK(22: 20), },
    { .val = BITS(22: 20, 5), .id = MT_CG_MPLL_D8,  .mask = BITMASK(22: 20), },
    { .val = BITS(22: 20, 6), .id = MT_CG_SYS_26M,  .mask = BITMASK(22: 20), }, // default
    { .val = BITS(22: 20, 7), .id = MT_CG_UPLL_D6,  .mask = BITMASK(22: 20), },
};
static const struct clkmux_map _mt_clkmux_spm_52m_ck_sel_map[] =
{
    { .val = BITS(23: 23, 0), .id = MT_CG_SYS_26M,  .mask = BITMASK(23: 23), }, // default
    { .val = BITS(23: 23, 1), .id = MT_CG_UPLL_D24, .mask = BITMASK(23: 23), },
};
static const struct clkmux_map _mt_clkmux_pmicspi_mux_sel_map[] =
{
    { .val = BITS(25: 24, 0), .id = MT_CG_MPLL_D24, .mask = BITMASK(25: 24), }, // default // MT_CG_MPLL_D24 (DDR2) or MT_CG_MPLL_D20 (DDR3)
    { .val = BITS(25: 24, 1), .id = MT_CG_UNIV_48M, .mask = BITMASK(25: 24), },
    { .val = BITS(25: 24, 2), .id = MT_CG_UPLL_D16, .mask = BITMASK(25: 24), },
    { .val = BITS(25: 24, 3), .id = MT_CG_SYS_26M,  .mask = BITMASK(25: 24), },
};
#if 0
static const struct clkmux_map _mt_clkmux_aud_hf_26m_sel_map[] =
{
    { .val = BITS(26: 26, 0), .id = MT_CG_SYS_26M, .mask = BITMASK(26: 26), }, // default
};
#endif
// just alias and would replace @ aud_intbus_clkmux_ops
static const struct clkmux_map _mt_clkmux_aud_intbus_sel_map[] =
{
    { .val = BITS(29: 27, 1), .id = MT_CG_SYS_26M,  .mask = BITMASK(29: 27), }, // default
    { .val = BITS(29: 27, 2), .id = MT_CG_MPLL_D24, .mask = BITMASK(29: 27), }, // MT_CG_MPLL_D12/2 (DDR2), MT_CG_MPLL_D10/2 (DDR3)
    { .val = BITS(29: 27, 4), .id = MT_CG_MPLL_D12, .mask = BITMASK(29: 27), }, // MT_CG_MPLL_D12 (DDR2),   MT_CG_MPLL_D10 (DDR3)
};

static struct clkmux muxs[] = // NR_CLKMUXS
{
    [MT_CLKMUX_UART0_GFMUX_SEL] =
    {
        .name       = __stringify(MT_CLKMUX_UART0_GFMUX_SEL),
        .base_addr  = CLK_MUX_SEL,
        // .sel_mask   = BITMASK(0: 0),
        .ops        = &glitch_free_clkmux_ops,
        .map        = _mt_clkmux_uart0_gfmux_sel_map,
        .nr_map     = ARRAY_SIZE(_mt_clkmux_uart0_gfmux_sel_map),
        .drain      = MT_CG_UART0_SW_CG,
    },
    [MT_CLKMUX_EMI2X_GFMUX_SEL] =
    {
        .name       = __stringify(MT_CLKMUX_EMI2X_GFMUX_SEL),
        .base_addr  = CLK_MUX_SEL,
        // .sel_mask   = BITMASK(4: 1),
        .ops        = &glitch_free_wo_drain_cg_clkmux_ops,
        .map        = _mt_clkmux_emi2x_gfmux_sel_map,
        .nr_map     = ARRAY_SIZE(_mt_clkmux_emi2x_gfmux_sel_map),
        .drain      = MT_CG_INVALID,
    },
    [MT_CLKMUX_AXIBUS_GFMUX_SEL] =
    {
        .name       = __stringify(MT_CLKMUX_AXIBUS_GFMUX_SEL),
        .base_addr  = CLK_MUX_SEL,
        // .sel_mask   = BITMASK(7: 5),
        .ops        = &glitch_free_wo_drain_cg_clkmux_ops,
        .map        = _mt_clkmux_axibus_gfmux_sel_map,
        .nr_map     = ARRAY_SIZE(_mt_clkmux_axibus_gfmux_sel_map),
        .drain      = MT_CG_INVALID,
    },
    [MT_CLKMUX_MFG_MUX_SEL] =
    {
        .name       = __stringify(MT_CLKMUX_MFG_MUX_SEL),
        .base_addr  = CLK_MUX_SEL,
        // .sel_mask   = BITMASK(31: 31) | BITMASK(10: 8),
        .ops        = &mfg_glitch_free_clkmux_ops,
        .map        = _mt_clkmux_mfg_mux_sel_map,
        .nr_map     = ARRAY_SIZE(_mt_clkmux_mfg_mux_sel_map),
        .drain      = MT_CG_MFG_MM_SW_CG,
    },
    [MT_CLKMUX_MSDC0_MUX_SEL] =
    {
        .name       = __stringify(MT_CLKMUX_MSDC0_MUX_SEL),
        .base_addr  = CLK_MUX_SEL,
        // .sel_mask   = BITMASK(13: 11),
        .ops        = &non_gf_clkmux_ops,
        .map        = _mt_clkmux_msdc0_mux_sel_map,
        .nr_map     = ARRAY_SIZE(_mt_clkmux_msdc0_mux_sel_map),
        .drain      = MT_CG_MSDC0_SW_CG,
    },
    [MT_CLKMUX_SPINFI_MUX_SEL] =
    {
        .name       = __stringify(MT_CLKMUX_SPINFI_MUX_SEL),
        .base_addr  = CLK_MUX_SEL,
        // .sel_mask   = BITMASK(30: 30) | BITMASK(16: 14),
        .ops        = &glitch_free_clkmux_ops,
        .map        = _mt_clkmux_spinfi_mux_sel_map,
        .nr_map     = ARRAY_SIZE(_mt_clkmux_spinfi_mux_sel_map),
        .drain      = MT_CG_SPINFI_SW_CG,
    },
    [MT_CLKMUX_CAM_MUX_SEL] =
    {
        .name       = __stringify(MT_CLKMUX_CAM_MUX_SEL),
        .base_addr  = CLK_MUX_SEL,
        // .sel_mask   = BITMASK(17: 17),
        .ops        = &non_gf_clkmux_ops,
        .map        = _mt_clkmux_cam_mux_sel_map,
        .nr_map     = ARRAY_SIZE(_mt_clkmux_cam_mux_sel_map),
        .drain      = MT_CG_CAM_MM_SW_CG,
    },
    [MT_CLKMUX_PWM_MM_MUX_SEL] =
    {
        .name       = __stringify(MT_CLKMUX_PWM_MM_MUX_SEL),
        .base_addr  = CLK_MUX_SEL,
        // .sel_mask   = BITMASK(18: 18),
        .ops        = &non_gf_clkmux_ops,
        .map        = _mt_clkmux_pwm_mm_mux_sel_map,
        .nr_map     = ARRAY_SIZE(_mt_clkmux_pwm_mm_mux_sel_map),
        .drain      = MT_CG_PWM_MM_SW_CG,
    },
    [MT_CLKMUX_UART1_GFMUX_SEL] =
    {
        .name       = __stringify(MT_CLKMUX_UART1_GFMUX_SEL),
        .base_addr  = CLK_MUX_SEL,
        // .sel_mask   = BITMASK(19: 19),
        .ops        = &glitch_free_clkmux_ops,
        .map        = _mt_clkmux_uart1_gfmux_sel_map,
        .nr_map     = ARRAY_SIZE(_mt_clkmux_uart1_gfmux_sel_map),
        .drain      = MT_CG_UART1_SW_CG,
    },
    [MT_CLKMUX_MSDC1_MUX_SEL] =
    {
        .name       = __stringify(MT_CLKMUX_MSDC1_MUX_SEL),
        .base_addr  = CLK_MUX_SEL,
        // .sel_mask   = BITMASK(22: 20),
        .ops        = &non_gf_clkmux_ops,
        .map        = _mt_clkmux_msdc1_mux_sel_map,
        .nr_map     = ARRAY_SIZE(_mt_clkmux_msdc1_mux_sel_map),
        .drain      = MT_CG_MSDC1_SW_CG,
    },
    [MT_CLKMUX_SPM_52M_CK_SEL] =
    {
        .name       = __stringify(MT_CLKMUX_SPM_52M_CK_SEL),
        .base_addr  = CLK_MUX_SEL,
        // .sel_mask   = BITMASK(23: 23),
        .ops        = &glitch_free_clkmux_ops,
        .map        = _mt_clkmux_spm_52m_ck_sel_map,
        .nr_map     = ARRAY_SIZE(_mt_clkmux_spm_52m_ck_sel_map),
        .drain      = MT_CG_SPM_52M_SW_CG,
    },
    [MT_CLKMUX_PMICSPI_MUX_SEL] =
    {
        .name       = __stringify(MT_CLKMUX_PMICSPI_MUX_SEL),
        .base_addr  = CLK_MUX_SEL,
        // .sel_mask   = BITMASK(25: 24),
        .ops        = &pmicspi_clkmux_ops,
        .map        = _mt_clkmux_pmicspi_mux_sel_map,
        .nr_map     = ARRAY_SIZE(_mt_clkmux_pmicspi_mux_sel_map),
        .drain      = MT_CG_PMIC_SW_CG_AP,
    },
#if 0
    [MT_CLKMUX_AUD_HF_26M_SEL] =
    {
        .name       = __stringify(MT_CLKMUX_AUD_HF_26M_SEL),
        .base_addr  = CLK_MUX_SEL,
        .sel_mask   = BITMASK(26: 26),
        .ops        = &XXX,
        .map        = _mt_clkmux_aud_hf_26m_sel_map,
        .nr_map     = ARRAY_SIZE(_mt_clkmux_aud_hf_26m_sel_map),
        .drain      = MT_CG_INVALID,
    },
#endif
    [MT_CLKMUX_AUD_INTBUS_SEL] =
    {
        .name       = __stringify(MT_CLKMUX_AUD_INTBUS_SEL),
        .base_addr  = CLK_MUX_SEL,
        // .sel_mask   = BITMASK(29: 27),
        .ops        = &aud_intbus_clkmux_ops,
        .map        = _mt_clkmux_aud_intbus_sel_map,
        .nr_map     = ARRAY_SIZE(_mt_clkmux_aud_intbus_sel_map),
        .drain      = MT_CG_AUDIO_SW_CG,
    },
};

// general clkmux get

static cg_clk_id general_clkmux_get_op(struct clkmux *mux)
{
    volatile unsigned int reg_val;
    int i;

    ENTER_FUNC(FUNC_LV_OP);

    reg_val = clk_readl(mux->base_addr);

    for (i = 0; i < mux->nr_map; i++)
    {
        // clk_dbg(">> "HEX_FMT", "HEX_FMT", "HEX_FMT"\n", mux->map[i].val, mux->map[i].mask, reg_val);
        if ((mux->map[i].val & mux->map[i].mask) == (reg_val & mux->map[i].mask))
        {
            break;
        }
    }

    #if 0 // XXX: just for FPGA
    if (i == mux->nr_map)
    {
        return MT_CG_SYS_26M;
    }
    #else
    BUG_ON(i == mux->nr_map);
    #endif

    EXIT_FUNC(FUNC_LV_OP);

    return mux->map[i].id;
}

// non glitch free clkmux sel (disable drain cg first (i.e. with drain cg))

static int non_gf_clkmux_sel_op(struct clkmux *mux, cg_clk_id clksrc)
{
    struct cg_clk *drain_clk = id_to_clk(mux->drain);
    int err = 0;
    int i;

    ENTER_FUNC(FUNC_LV_OP);

    // 0. all CLKMUX use this op need drain clk (to disable)
    BUG_ON(NULL == drain_clk); // map error @ muxs[]->ops & muxs[]->drain (use non_gf_clkmux_sel_op with "wrong drain cg")

    if (clksrc == drain_clk->src) // switch to the same source (i.e. don't need clk switch actually)
    {
        EXIT_FUNC(FUNC_LV_OP);
        return 0;
    }

    // look up the clk map
    for (i = 0; i < mux->nr_map; i++)
    {
        if (mux->map[i].id == clksrc)
        {
            break;
        }
    }

    // clk sel match
    if (i < mux->nr_map)
    {
        bool orig_drain_cg_is_on_flag = FALSE;

        // 1. turn off first for glitch free if drain CG is on
        if (PWR_ON == drain_clk->ops->get_state(drain_clk))
        {
            drain_clk->ops->disable(drain_clk);
            orig_drain_cg_is_on_flag = TRUE;
        }

        // 2. switch clkmux
        {
            volatile unsigned int reg_val;

            // (just) help to enable/disable src clock if necessary (i.e. drain cg is on original)
            if (TRUE == orig_drain_cg_is_on_flag)
            {
                err = enable_clock_locked(id_to_clk(clksrc)); // XXX: enable first for seemless transition
                BUG_ON(err);
            }

            // set clkmux reg (non glitch free)
            reg_val = (clk_readl(mux->base_addr) & ~mux->map[i].mask) | mux->map[i].val;
            clk_writel(mux->base_addr, reg_val);

            // (just) help to enable/disable src clock if necessary (i.e. drain cg is on original)
            if (TRUE == orig_drain_cg_is_on_flag)
            {
                err = disable_clock_locked(id_to_clk(drain_clk->src));
                BUG_ON(err);
            }

            drain_clk->src = clksrc;
        }

        // 3. turn on drain CG if necessary
        if (TRUE == orig_drain_cg_is_on_flag)
        {
            drain_clk->ops->enable(drain_clk);
        }
    }
    else
    {
        err = -1; // ERROR: clk sel not match
    }
    BUG_ON(err); // clk sel not match

    EXIT_FUNC(FUNC_LV_OP);
    return err;
}

static struct clkmux_ops non_gf_clkmux_ops =
{
    .sel = non_gf_clkmux_sel_op,
    .get = general_clkmux_get_op,
};

// (half) glitch free clkmux sel (with drain cg)

#define GFMUX_BIT_MASK_FOR_HALF_GF_OP BITMASK(31:30) // rg_mfg_gfmux_sel, rg_spinfi_gfmux_sel

static cg_clk_id clkmux_get_locked(struct clkmux *mux);

static int glitch_free_clkmux_sel_op(struct clkmux *mux, cg_clk_id clksrc)
{
    struct cg_clk *drain_clk = id_to_clk(mux->drain);
    struct cg_clk *from_clk;
    int err = 0;
    int i;

    ENTER_FUNC(FUNC_LV_OP);

    BUG_ON(NULL == drain_clk); // map error @ muxs[]->ops & muxs[]->drain (use glitch_free_clkmux_sel_op with "wrong drain cg")

    if (clksrc == drain_clk->src) // switch to the same source (i.e. don't need clk switch actually)
    {
        EXIT_FUNC(FUNC_LV_OP);
        return 0;
    }

    // look up the clk map
    for (i = 0; i < mux->nr_map; i++)
    {
        if (mux->map[i].id == clksrc)
        {
            break;
        }
    }

    // clk sel match
    if (i < mux->nr_map)
    {
        bool disable_src_cg_flag = FALSE;
        volatile unsigned int reg_val;

        // (just) help to enable/disable src clock if necessary (i.e. drain cg is on original)
        if (PWR_ON == drain_clk->ops->get_state(drain_clk))
        {
            err = enable_clock_locked(id_to_clk(clksrc)); // XXX: enable first for seemless transition
            BUG_ON(err);
            disable_src_cg_flag = TRUE;
        }
        else
        {
            err = enable_clock_locked(id_to_clk(clksrc));
            BUG_ON(err);

            from_clk = id_to_clk(clkmux_get_locked(mux));

            err = enable_clock_locked(from_clk);
            BUG_ON(err);
        }

        // set clkmux reg (inc. non glitch free / half glitch free / glitch free case)
        reg_val = (clk_readl(mux->base_addr) & ~mux->map[i].mask) | mux->map[i].val;
        if (mux->map[i].mask & GFMUX_BIT_MASK_FOR_HALF_GF_OP)
        {
            clk_writel(mux->base_addr, clk_readl(mux->base_addr) & ~(mux->map[i].mask & GFMUX_BIT_MASK_FOR_HALF_GF_OP)); // XXX: tricky for half glitch free case (set rg_xxx_gfmux_sel 0 first)
        }
        clk_writel(mux->base_addr, reg_val);

        // (just) help to enable/disable src clock if necessary (i.e. drain cg is on original)
        if (TRUE == disable_src_cg_flag)
        {
            err = disable_clock_locked(id_to_clk(drain_clk->src));
            BUG_ON(err);
        }
        else
        {
            err = disable_clock_locked(id_to_clk(clksrc));
            BUG_ON(err);
            err = disable_clock_locked(from_clk);
            BUG_ON(err);
        }

        drain_clk->src = clksrc;
    }
    else
    {
        err = -1; // ERROR: clk sel not match
    }
    BUG_ON(err); // clk sel not match

    EXIT_FUNC(FUNC_LV_OP);
    return err;
}

static struct clkmux_ops glitch_free_clkmux_ops =
{
    .sel = glitch_free_clkmux_sel_op,
    .get = general_clkmux_get_op,
};

// (half) MFG glitch free clkmux sel (with drain cg)

static cg_clk_id mfg_clkmux_clksrc = MT_CG_UPLL_D3;

static int mfg_glitch_free_clkmux_sel_op(struct clkmux *mux, cg_clk_id clksrc)
{
    int err = 0;

    if (get_gpu_level() == GPU_LEVEL_0)
    {
        mfg_clkmux_clksrc = clksrc;
    }
    else
    {
        err = glitch_free_clkmux_sel_op(mux, clksrc);
    }

    return err;
}

static cg_clk_id mfg_glitch_free_clkmux_get_op(struct clkmux *mux)
{
    cg_clk_id clksrc;

    if (get_gpu_level() == GPU_LEVEL_0)
    {
        clksrc = mfg_clkmux_clksrc;
    }
    else
    {
        clksrc = general_clkmux_get_op(mux);
    }

    return clksrc;
}

static struct clkmux_ops mfg_glitch_free_clkmux_ops =
{
    .sel = mfg_glitch_free_clkmux_sel_op,
    .get = mfg_glitch_free_clkmux_get_op,
};

// glitch free clkmux without drain cg

static int glitch_free_wo_drain_cg_clkmux_sel_op(struct clkmux *mux, cg_clk_id clksrc)
{
    volatile unsigned int reg_val;
    struct cg_clk *src_clk;
    int err;
    int i;

    ENTER_FUNC(FUNC_LV_OP);

    // search current src cg first
    {
        reg_val = clk_readl(mux->base_addr);

        // look up current src cg
        for (i = 0; i < mux->nr_map; i++)
        {
            if ((mux->map[i].val & mux->map[i].mask) == (reg_val & mux->map[i].mask))
            {
                break;
            }
        }

        BUG_ON(i >= mux->nr_map); // map error @ muxs[]->map

        src_clk = id_to_clk(mux->map[i].id);

        BUG_ON(NULL == src_clk); // map error @ muxs[]->map

        if (clksrc == mux->map[i].id) // switch to the same source (i.e. don't need clk switch actually)
        {
            EXIT_FUNC(FUNC_LV_OP);
            return 0;
        }
    }

    // look up the clk map
    for (i = 0; i < mux->nr_map; i++)
    {
        if (mux->map[i].id == clksrc)
        {
            break;
        }
    }

    // clk sel match
    if (i < mux->nr_map)
    {
        err = enable_clock_locked(id_to_clk(clksrc)); // XXX: enable first for seemless transition
        BUG_ON(err);

        // set clkmux reg (inc. non glitch free / half glitch free / glitch free case)
        reg_val = (clk_readl(mux->base_addr) & ~mux->map[i].mask) | mux->map[i].val;
        clk_writel(mux->base_addr, reg_val & ~(mux->map[i].mask & GFMUX_BIT_MASK_FOR_HALF_GF_OP)); // XXX: tricky for half glitch free case (set rg_xxx_gfmux_sel 0 first)
        clk_writel(mux->base_addr, reg_val);

        err = disable_clock_locked(src_clk); // TODO: please check init case (should setup well (i.e. enable) @ init state because of no drain CG)
        BUG_ON(err);
    }
    else
    {
        err = -1; // ERROR: clk sel not match
    }
    BUG_ON(err); // clk sel not match

    EXIT_FUNC(FUNC_LV_OP);
    return err;
}

static struct clkmux_ops glitch_free_wo_drain_cg_clkmux_ops =
{
    .sel = glitch_free_wo_drain_cg_clkmux_sel_op,
    .get = general_clkmux_get_op,
};

// pmicspi clkmux sel (non glitch free & with drain cg)

static int pmicspi_clkmux_sel_op(struct clkmux *mux, cg_clk_id clksrc) // almost the same with non_gf_clkmux_sel_op()
{
    struct cg_clk *drain_clk = id_to_clk(mux->drain);
    int err = 0;
    int i;

    ENTER_FUNC(FUNC_LV_OP);

    // 0. all CLKMUX use this op need drain clk (to disable)
    BUG_ON(NULL == drain_clk); // map error @ muxs[]->ops & muxs[]->drain (use pmicspi_clkmux_sel_op with "wrong drain cg")

    // look up the clk map
    for (i = 0; i < mux->nr_map; i++)
    {
        if (mux->map[i].id == clksrc)
        {
            break;
        }
    }

    if (i < mux->nr_map)
    {
        bool orig_drain_cg_is_on_flag = FALSE;

        if (MT_CG_MPLL_D24 == clksrc)
        {
            clksrc = is_ddr3() ? MT_CG_MPLL_D20 : MT_CG_MPLL_D24;
        }

        if (clksrc == drain_clk->src) // switch to the same source (i.e. don't need clk switch actually)
        {
            EXIT_FUNC(FUNC_LV_OP);
            return 0;
        }

        // 1. turn off first for glitch free if drain CG is on
        if (PWR_ON == drain_clk->ops->get_state(drain_clk))
        {
            drain_clk->ops->disable(drain_clk);
            orig_drain_cg_is_on_flag = TRUE;
        }

        // 2. switch clkmux
        {
            volatile unsigned int reg_val;

            // (just) help to enable/disable src clock if necessary (i.e. drain cg is on original)
            if (TRUE == orig_drain_cg_is_on_flag)
            {
                err = enable_clock_locked(id_to_clk(clksrc)); // XXX: enable first for seemless transition
                BUG_ON(err);
            }

            // set clkmux reg (non glitch free)
            reg_val = (clk_readl(mux->base_addr) & ~mux->map[i].mask) | mux->map[i].val;
            clk_writel(mux->base_addr, reg_val);

            // (just) help to enable/disable src clock if necessary (i.e. drain cg is on original)
            if (TRUE == orig_drain_cg_is_on_flag)
            {
                err = disable_clock_locked(id_to_clk(drain_clk->src));
                BUG_ON(err);
            }

            drain_clk->src = clksrc;
        }

        // 3. turn on drain CG if necessary
        if (TRUE == orig_drain_cg_is_on_flag)
        {
            drain_clk->ops->enable(drain_clk);
        }
    }
    else
    {
        err = -1; // ERROR: clk sel not match
    }
    BUG_ON(err); // clk sel not match

    EXIT_FUNC(FUNC_LV_OP);
    return err;
}

static struct clkmux_ops pmicspi_clkmux_ops =
{
    .sel = pmicspi_clkmux_sel_op,
    .get = general_clkmux_get_op,
};

// aud_intbus clkmux sel (glitch free & with drain cg)

static int aud_intbus_clkmux_sel_op(struct clkmux *mux, cg_clk_id clksrc) //  almost the same with glitch_free_clkmux_sel_op()
{
    struct cg_clk *drain_clk = id_to_clk(mux->drain);
    int err = 0;
    int i;

    ENTER_FUNC(FUNC_LV_OP);

    BUG_ON(NULL == drain_clk); // map error @ muxs[]->ops & muxs[]->drain (use aud_intbus_clkmux_sel_op with "wrong drain cg")

    // look up the clk map
    for (i = 0; i < mux->nr_map; i++)
    {
        if (mux->map[i].id == clksrc)
        {
            break;
        }
    }

    // clk sel match
    if (i < mux->nr_map)
    {
        bool disable_src_cg_flag = FALSE;
        volatile unsigned int reg_val;

        if (MT_CG_SYS_26M != clksrc)
        {
            clksrc = is_ddr3() ? MT_CG_MPLL_D10 : MT_CG_MPLL_D12;
        }

        if (clksrc == drain_clk->src) // switch to the same source (i.e. don't need clk switch actually)
        {
            EXIT_FUNC(FUNC_LV_OP);
            return 0;
        }

        // (just) help to enable/disable src clock if necessary (i.e. drain cg is on original)
        if (PWR_ON == drain_clk->ops->get_state(drain_clk))
        {
            err = enable_clock_locked(id_to_clk(clksrc)); // XXX: enable first for seemless transition
            BUG_ON(err);
            disable_src_cg_flag = TRUE;
        }

        // set clkmux reg (glitch free)
        reg_val = (clk_readl(mux->base_addr) & ~mux->map[i].mask) | mux->map[i].val;
        clk_writel(mux->base_addr, reg_val);

        // (just) help to enable/disable src clock if necessary (i.e. drain cg is on original)
        if (TRUE == disable_src_cg_flag)
        {
            err = disable_clock_locked(id_to_clk(drain_clk->src));
            BUG_ON(err);
        }

        drain_clk->src = clksrc;
    }
    else
    {
        err = -1; // ERROR: clk sel not match
    }
    BUG_ON(err); // clk sel not match

    EXIT_FUNC(FUNC_LV_OP);
    return err;
}

static struct clkmux_ops aud_intbus_clkmux_ops =
{
    .sel = aud_intbus_clkmux_sel_op,
    .get = general_clkmux_get_op,
};

static struct clkmux *id_to_mux(clkmux_id id)
{
    return id < NR_CLKMUXS ? &muxs[id] : NULL;
}

static void clkmux_sel_locked(struct clkmux *mux, cg_clk_id clksrc)
{
    ENTER_FUNC(FUNC_LV_LOCKED);
    mux->ops->sel(mux, clksrc);
    EXIT_FUNC(FUNC_LV_LOCKED);
}

static cg_clk_id clkmux_get_locked(struct clkmux *mux)
{
    ENTER_FUNC(FUNC_LV_LOCKED);

    EXIT_FUNC(FUNC_LV_LOCKED);
    return mux->ops->get(mux);
}

int clkmux_sel(clkmux_id id, cg_clk_id clksrc, const char *name)
{
    unsigned long flags;
    struct clkmux *mux = id_to_mux(id);

    ENTER_FUNC(FUNC_LV_API);

#if defined(CONFIG_CLKMGR_EMULATION)

    return 0;

#else   // CONFIG_CLKMGR_EMULATION

    BUG_ON(!initialized);
    BUG_ON(!mux);
    // BUG_ON(clksrc >= mux->nr_inputs); // XXX: clksrc match is @ clkmux_sel_op()

    clkmgr_lock(flags);
    clkmux_sel_locked(mux, clksrc);
    clkmgr_unlock(flags);

    EXIT_FUNC(FUNC_LV_API);
    return 0;

#endif  // CONFIG_CLKMGR_EMULATION
}
EXPORT_SYMBOL(clkmux_sel);

int clkmux_get(clkmux_id id, const char *name)
{
    cg_clk_id clksrc;
    unsigned long flags;
    struct clkmux *mux = id_to_mux(id);

    ENTER_FUNC(FUNC_LV_API);

#if defined(CONFIG_CLKMGR_EMULATION)

    return 0;

#else   // CONFIG_CLKMGR_EMULATION

    BUG_ON(!initialized);
    BUG_ON(!mux);
    // BUG_ON(clksrc >= mux->nr_inputs); // XXX: clksrc match is @ clkmux_sel_op()

    clkmgr_lock(flags);
    clksrc = clkmux_get_locked(mux);
    clkmgr_unlock(flags);

    EXIT_FUNC(FUNC_LV_API);
    return clksrc;

#endif  // CONFIG_CLKMGR_EMULATION
}
EXPORT_SYMBOL(clkmux_get);

//
// PLL variable & function
//
#define PLL_TYPE_SDM        0
#define PLL_TYPE_LC         1

#define HAVE_RST_BAR        (0x1 << 0)
#define HAVE_PLL_HP         (0x1 << 1)
#define HAVE_FIX_FRQ        (0x1 << 2)

#define PLL_EN              BIT(0)          // @ XPLL_CON0
#define PLL_SDM_FRA_EN      BIT(4)          // @ XPLL_CON0
#define RST_BAR_MASK        BITMASK(27:27)  // @ XPLL_CON0

#define SDM_PLL_N_INFO_MASK BITMASK(20:0)   // @ XPLL_CON1 (XPLL_SDM_PCW)
#define SDM_PLL_POSDIV_MASK BITMASK(26:24)  // @ XPLL_CON1 (XPLL_POSDIV)
#define SDM_PLL_N_INFO_CHG  BITMASK(31:31)  // @ XPLL_CON1 (XPLL_SDM_PCW_CHG)

#define PLL_FBKDIV_MASK     BITMASK(6:0)    // @ UNIVPLL_CON1 (UNIVPLL_SDM_PCW)

#define PLL_PWR_ON          BIT(0)          // @ XPLL_PWR
#define PLL_ISO_EN          BIT(1)          // @ XPLL_PWR

static struct pll_ops sdm_pll_ops;
static struct pll_ops univpll_ops;
static struct pll_ops whpll_ops;

static struct pll plls[NR_PLLS] =
{
    [ARMPLL] =
    {
        .name = __stringify(ARMPLL),
        .type = PLL_TYPE_SDM,
        .feat = HAVE_PLL_HP,
        .en_mask = PLL_EN | PLL_SDM_FRA_EN,
        .base_addr = ARMPLL_CON0,
        .pwr_addr = ARMPLL_PWR_CON0,
        .ops = &sdm_pll_ops,
        .hp_id = MT658X_FH_ARM_PLL,
        .hp_switch = 0,
    },
    [MAINPLL] =
    {
        .name = __stringify(MAINPLL),
        .type = PLL_TYPE_SDM,
        .feat = HAVE_PLL_HP | HAVE_RST_BAR,
        .en_mask = PLL_EN | PLL_SDM_FRA_EN,
        .base_addr = MAINPLL_CON0,
        .pwr_addr = MAINPLL_PWR_CON0,
        .ops = &sdm_pll_ops,
        .hp_id = MT658X_FH_MAIN_PLL,
        .hp_switch = 0,
    },
    [UNIVPLL] =
    {
        .name = __stringify(UNIVPLL),
        .type = PLL_TYPE_SDM,
        .feat = HAVE_RST_BAR | HAVE_FIX_FRQ,
        .en_mask = PLL_EN | PLL_SDM_FRA_EN, // | USB48M_EN_BIT | UNIV48M_EN_BIT, // TODO: check with USB owner
        .base_addr = UNIVPLL_CON0,
        .pwr_addr = UNIVPLL_PWR_CON0,
        .ops = &univpll_ops,
    },
    [WHPLL] =
    {
        .name = __stringify(WHPLL),
        .type = PLL_TYPE_SDM,
        .feat = 0, // HAVE_PLL_HP
        .en_mask = PLL_EN | PLL_SDM_FRA_EN,
        .base_addr = WHPLL_CON0,
        .pwr_addr = WHPLL_PWR_CON0,
        .ops = &whpll_ops,
    },
};

static struct pll *id_to_pll(unsigned int id)
{
    return id < NR_PLLS ? &plls[id] : NULL;
}

static int pll_get_state_op(struct pll *pll)
{
    ENTER_FUNC(FUNC_LV_OP);

    EXIT_FUNC(FUNC_LV_OP);
    return clk_readl(pll->base_addr) & PLL_EN;
}

static void sdm_pll_enable_op(struct pll *pll)
{
    // PWRON:1 -> ISOEN:0 -> EN:1 -> RSTB:1

    ENTER_FUNC(FUNC_LV_OP);

    clk_setl(pll->pwr_addr, PLL_PWR_ON);
    udelay(1); // XXX: 30ns is enough (diff from 89)
    clk_clrl(pll->pwr_addr, PLL_ISO_EN);
    udelay(1); // XXX: 30ns is enough (diff from 89)

    clk_setl(pll->base_addr, pll->en_mask);
    udelay(20);

    if (pll->feat & HAVE_RST_BAR)
    {
        clk_setl(pll->base_addr, RST_BAR_MASK);
    }

    EXIT_FUNC(FUNC_LV_OP);
}

static void sdm_pll_disable_op(struct pll *pll)
{
    // RSTB:0 -> EN:0 -> ISOEN:1 -> PWRON:0

    ENTER_FUNC(FUNC_LV_OP);

    if (pll->feat & HAVE_RST_BAR)
    {
        clk_clrl(pll->base_addr, RST_BAR_MASK);
    }

    clk_clrl(pll->base_addr, PLL_EN);

    clk_setl(pll->pwr_addr, PLL_ISO_EN);
    clk_clrl(pll->pwr_addr, PLL_PWR_ON);

    EXIT_FUNC(FUNC_LV_OP);
}

static void sdm_pll_fsel_op(struct pll *pll, unsigned int value)
{
    unsigned int ctrl_value;
    unsigned int pll_en;

    ENTER_FUNC(FUNC_LV_OP);

    pll_en = clk_readl(pll->base_addr) & PLL_EN;
    ctrl_value = clk_readl(pll->base_addr + 4); // XPLL_CON1
    ctrl_value &= ~(SDM_PLL_N_INFO_MASK | SDM_PLL_POSDIV_MASK);
    ctrl_value |= value & (SDM_PLL_N_INFO_MASK | SDM_PLL_POSDIV_MASK);
    if (pll_en) ctrl_value |= SDM_PLL_N_INFO_CHG;

    clk_writel(pll->base_addr + 4, ctrl_value); // XPLL_CON1
    if (pll_en) udelay(20);

    EXIT_FUNC(FUNC_LV_OP);
}

static int sdm_pll_dump_regs_op(struct pll *pll, unsigned int *ptr)
{
    ENTER_FUNC(FUNC_LV_OP);

    *(ptr) = clk_readl(pll->base_addr);         // XPLL_CON0
    *(++ptr) = clk_readl(pll->base_addr + 4);   // XPLL_CON1
    *(++ptr) = clk_readl(pll->pwr_addr);        // XPLL_PWR

    EXIT_FUNC(FUNC_LV_OP);
    return 3; // return size
}

static const unsigned int pll_vcodivsel_map[2] = {1, 2};
static const unsigned int pll_prediv_map[4] = {1, 2, 4, 4};
static const unsigned int pll_posdiv_map[8] = {1, 2, 4, 8, 16, 16, 16, 16};
static const unsigned int pll_fbksel_map[4] = {1, 2, 4, 4};
static const unsigned int pll_n_info_map[14] = // assume fin = 26MHz
{
    13000000,
    6500000,
    3250000,
    1625000,
    812500,
    406250,
    203125,
    101563,
    50782,
    25391,
    12696,
    6348,
    3174,
    1587,
};

// vco_freq = fin (i.e. 26MHz) x n_info x vco_div_sel / prediv
// freq = vco_freq / posdiv
static unsigned int sdm_pll_vco_calc_op(struct pll *pll)
{
    int i;
    unsigned int mask;
    unsigned int vco_i = 0;
    unsigned int vco_f = 0;
    unsigned int vco = 0;

    // volatile unsigned int con0 = clk_readl(pll->base_addr);     // XPLL_CON0 /* not used */
    volatile unsigned int con1 = clk_readl(pll->base_addr + 4); // XPLL_CON1

    unsigned int vcodivsel  = 0; // (con0 >> 19) & 0x1; // bit[19]  // XXX: always zero
    unsigned int prediv     = 0; // (con0 >> 4) & 0x3;  // bit[5:4] // XXX: always zero
    unsigned int n_info_i = (con1 >> 14) & 0x7F;    // bit[20:14]
    unsigned int n_info_f = (con1 >> 0)  & 0x3FFF;  // bit[13:0]

    ENTER_FUNC(FUNC_LV_OP);

    vcodivsel = pll_vcodivsel_map[vcodivsel];
    prediv = pll_prediv_map[prediv];

    vco_i = 26 * n_info_i;

    for (i = 0; i < 14; i++)
    {
        mask = 1U << (13 - i);

        if (n_info_f & mask)
        {
            vco_f += pll_n_info_map[i];

            if (!(n_info_f & (mask-1))) // could break early if remaining bits are 0
            {
                break;
            }
        }
    }

    vco_f = (vco_f + 1000000 / 2) / 1000000; // round up

    vco = (vco_i + vco_f) * 1000 * vcodivsel / prediv; // KHz

#if 0
    clk_dbg("[%s]%s: ["HEX_FMT", "HEX_FMT"] vco_i=%uMHz, vco_f=%uMHz, vco=%uKHz\n",
             __func__, pll->name, con0, con1, vco_i, vco_f, vco);
#endif

    EXIT_FUNC(FUNC_LV_OP);
    return vco;
}

static int sdm_pll_hp_enable_op(struct pll *pll)
{
    int err;
    unsigned int vco;

    ENTER_FUNC(FUNC_LV_OP);

    if (!pll->hp_switch || (pll->state == PWR_DOWN))
    {
        EXIT_FUNC(FUNC_LV_OP);
        return 0;
    }

    vco = pll->ops->vco_calc(pll);
    err = freqhop_config(pll->hp_id, vco, 1);

    EXIT_FUNC(FUNC_LV_OP);
    return err;
}

static int sdm_pll_hp_disable_op(struct pll *pll)
{
    int err;
    unsigned int vco;

    ENTER_FUNC(FUNC_LV_OP);

    if (!pll->hp_switch || (pll->state == PWR_ON)) // TODO: why PWR_ON
    {
        EXIT_FUNC(FUNC_LV_OP);
        return 0;
    }

    vco = pll->ops->vco_calc(pll);
    err = freqhop_config(pll->hp_id, vco, 0);

    EXIT_FUNC(FUNC_LV_OP);
    return err;
}

static struct pll_ops sdm_pll_ops =
{
    .get_state  = pll_get_state_op,
    .enable     = sdm_pll_enable_op,
    .disable    = sdm_pll_disable_op,
    .fsel       = sdm_pll_fsel_op,
    .dump_regs  = sdm_pll_dump_regs_op,
    .vco_calc   = sdm_pll_vco_calc_op,
    .hp_enable  = sdm_pll_hp_enable_op,
    .hp_disable = sdm_pll_hp_disable_op,
};

static void univ_pll_fsel_op(struct pll *pll, unsigned int value)
{
    unsigned int ctrl_value;
    unsigned int pll_en;

    ENTER_FUNC(FUNC_LV_OP);

    if (pll->feat & HAVE_FIX_FRQ) // XXX: UNIVPLL is HAVE_FIX_FRQ i.e. it would return
    {
        EXIT_FUNC(FUNC_LV_OP);
        return;
    }

    pll_en = clk_readl(pll->base_addr) & PLL_EN;
    ctrl_value = clk_readl(pll->base_addr + 4); // UNIVPLL_CON1
    ctrl_value &= ~PLL_FBKDIV_MASK;
    ctrl_value |= value & PLL_FBKDIV_MASK;
    if (pll_en) ctrl_value |= SDM_PLL_N_INFO_CHG;

    clk_writel(pll->base_addr + 4, ctrl_value); // UNIVPLL_CON1
    if (pll_en) udelay(20);

    EXIT_FUNC(FUNC_LV_OP);
}

static unsigned int univ_pll_vco_calc_op(struct pll *pll)
{
    unsigned int vco = 0;

    // volatile unsigned int con0 = clk_readl(pll->base_addr);     // UNIVPLL_CON0 /* not used */
    volatile unsigned int con1 = clk_readl(pll->base_addr + 4); // UNIVPLL_CON1

    unsigned int vcodivsel  = 0; // (con0 >> 19) & 0x1; // bit[19]      // XXX: always zero
    unsigned int prediv     = 0; // (con0 >> 4) & 0x3;  // bit[5:4]     // XXX: always zero
    unsigned int fbksel     = 0; // (con0 >> 20) & 0x3; // bit[21:20]   // XXX: always zero
    unsigned int fbkdiv     = con1 & PLL_FBKDIV_MASK;   // bit[6:0]

    ENTER_FUNC(FUNC_LV_OP);

    vcodivsel = pll_vcodivsel_map[vcodivsel];
    fbksel = pll_fbksel_map[fbksel];
    prediv = pll_prediv_map[prediv];

    vco = 26000 * fbkdiv * fbksel * vcodivsel / prediv;

#if 0
    clk_dbg("[%s]%s: ["HEX_FMT"] vco=%uKHz\n", __func__, pll->name, con0, vco);
#endif

    EXIT_FUNC(FUNC_LV_OP);
    return vco;
}

static struct pll_ops univpll_ops =
{
    .get_state  = pll_get_state_op,
    .enable     = sdm_pll_enable_op,
    .disable    = sdm_pll_disable_op,
    .fsel       = univ_pll_fsel_op,
    .dump_regs  = sdm_pll_dump_regs_op,
    .vco_calc   = univ_pll_vco_calc_op,
};

static void whpll_enable_op(struct pll *pll)
{
    ENTER_FUNC(FUNC_LV_OP);

    clk_writel(WHPLL_PATHSEL_CON, 1);
    clk_writel(RSV_RW0_CON1, 0xC0000000);

    sdm_pll_enable_op(pll);

    EXIT_FUNC(FUNC_LV_OP);
}

static void whpll_disable_op(struct pll *pll)
{
    ENTER_FUNC(FUNC_LV_OP);

    sdm_pll_disable_op(pll);

    clk_writel(WHPLL_PATHSEL_CON, 0);
    clk_writel(RSV_RW0_CON1, 0);

    EXIT_FUNC(FUNC_LV_OP);
}

static void whpll_fsel_op(struct pll *pll, unsigned int value)
{
    unsigned int ctrl_value;
    unsigned int pll_en;
    struct clkmux *mux = id_to_mux(MT_CLKMUX_MFG_MUX_SEL);

    ENTER_FUNC(FUNC_LV_OP);

    BUG_ON((NULL == mux) || (MT_CG_GPU_500P5M_EN != clkmux_get_locked(mux))); // XXX: please issue clkmux_sel(MT_CLKMUX_MFG_MUX_SEL, MT_CG_GPU_500P5M_EN, "xxx")

    clkmux_sel_locked(mux, MT_CG_UPLL_D3);          // XXX: it may cause WHPLL disabled

    pll_en = clk_readl(pll->base_addr) & PLL_EN;    // XXX: PLL_EN may be 0
    ctrl_value = clk_readl(pll->base_addr + 4);     // XPLL_CON1
    ctrl_value &= ~(SDM_PLL_N_INFO_MASK | SDM_PLL_POSDIV_MASK);
    ctrl_value |= value & (SDM_PLL_N_INFO_MASK | SDM_PLL_POSDIV_MASK);
    if (pll_en) ctrl_value |= SDM_PLL_N_INFO_CHG;

    clk_writel(pll->base_addr + 4, ctrl_value);     // XPLL_CON1
    if (pll_en) udelay(40);                         // XXX: add to 40 for pmic, orig: 20

    clkmux_sel_locked(mux, MT_CG_GPU_500P5M_EN);    // XXX: it may re-enable WHPLL

    EXIT_FUNC(FUNC_LV_OP);
}

static struct pll_ops whpll_ops =
{
    .get_state  = pll_get_state_op,
    .enable     = whpll_enable_op,
    .disable    = whpll_disable_op,
    .fsel       = whpll_fsel_op,
    .dump_regs  = sdm_pll_dump_regs_op,
    .vco_calc   = sdm_pll_vco_calc_op,
};

static unsigned int pll_freq_calc_op(struct pll *pll)
{
    volatile unsigned int con1 = clk_readl(pll->base_addr + 4); // XPLL_CON1
    unsigned int posdiv = (con1 >> 24) & 0x7; // bit[26:24]

    posdiv = pll_posdiv_map[posdiv];

    return pll->ops->vco_calc(pll) / posdiv;
}

static int get_pll_state_locked(struct pll *pll)
{
    ENTER_FUNC(FUNC_LV_LOCKED);

    if (likely(initialized))
    {
        EXIT_FUNC(FUNC_LV_LOCKED);
        return pll->state;                  // after init, get from local_state
    }
    else
    {
        EXIT_FUNC(FUNC_LV_LOCKED);
        return pll->ops->get_state(pll);    // before init, get from reg_val
    }
}

static int enable_pll_locked(struct pll *pll)
{
    ENTER_FUNC(FUNC_LV_LOCKED);

    pll->cnt++;

    if (pll->cnt > 1)
    {
        EXIT_FUNC(FUNC_LV_LOCKED);
        return 0;
    }

    pll->ops->enable(pll);
    pll->state = PWR_ON;

    if (pll->ops->hp_enable && pll->feat & HAVE_PLL_HP) // enable freqnency hopping automatically
    {
        pll->ops->hp_enable(pll);
    }

    EXIT_FUNC(FUNC_LV_LOCKED);
    return 0;
}

static int disable_pll_locked(struct pll *pll)
{
    ENTER_FUNC(FUNC_LV_LOCKED);

    BUG_ON(!pll->cnt);
    pll->cnt--;

    if (pll->cnt > 0)
    {
        EXIT_FUNC(FUNC_LV_LOCKED);
        return 0;
    }

    pll->ops->disable(pll);
    pll->state = PWR_DOWN;

    if (pll->ops->hp_disable && pll->feat & HAVE_PLL_HP) // disable freqnency hopping automatically
    {
        pll->ops->hp_disable(pll);
    }

    EXIT_FUNC(FUNC_LV_LOCKED);
    return 0;
}

static int pll_fsel_locked(struct pll *pll, unsigned int value)
{
    ENTER_FUNC(FUNC_LV_LOCKED);

    pll->ops->fsel(pll, value);

    if (pll->ops->hp_enable && pll->feat & HAVE_PLL_HP)
    {
        pll->ops->hp_enable(pll);
    }

    EXIT_FUNC(FUNC_LV_LOCKED);
    return 0;
}

int pll_is_on(pll_id id)
{
    int state;
    unsigned long flags;
    struct pll *pll = id_to_pll(id);

    ENTER_FUNC(FUNC_LV_API);

#if defined(CONFIG_CLKMGR_EMULATION)

    return 0;

#else   // CONFIG_CLKMGR_EMULATION

    BUG_ON(!pll);

    clkmgr_lock(flags);
    state = get_pll_state_locked(pll);
    clkmgr_unlock(flags);

    EXIT_FUNC(FUNC_LV_API);
    return state;

#endif  // CONFIG_CLKMGR_EMULATION
}
EXPORT_SYMBOL(pll_is_on);

int enable_pll(pll_id id, const char *name)
{
    int err;
    unsigned long flags;
    struct pll *pll = id_to_pll(id);

    ENTER_FUNC(FUNC_LV_API);

#if defined(CONFIG_CLKMGR_EMULATION)

    return 0;

#else   // CONFIG_CLKMGR_EMULATION

    if (WHPLL != id)
    {
        // WARN_ON(WHPLL != id); // XXX: ARMPLL / MAINPLL / UNIVPLL are not controlled by AP side
        return 0;
    }

    BUG_ON(!initialized);
    BUG_ON(!pll);

    clkmgr_lock(flags);
    err = enable_pll_locked(pll);
    clkmgr_unlock(flags);

    EXIT_FUNC(FUNC_LV_API);
    return err;

#endif  // CONFIG_CLKMGR_EMULATION
}
EXPORT_SYMBOL(enable_pll);

int disable_pll(pll_id id, const char *name)
{
    int err;
    unsigned long flags;
    struct pll *pll = id_to_pll(id);

    ENTER_FUNC(FUNC_LV_API);

#if defined(CONFIG_CLKMGR_EMULATION)

    return 0;

#else   // CONFIG_CLKMGR_EMULATION

    if (WHPLL != id)
    {
        // WARN_ON(WHPLL != id); // XXX: ARMPLL / MAINPLL / UNIVPLL are not controlled by AP side
        return 0;
    }

    BUG_ON(!initialized);
    BUG_ON(!pll);

    clkmgr_lock(flags);
    err = disable_pll_locked(pll);
    clkmgr_unlock(flags);

    EXIT_FUNC(FUNC_LV_API);
    return err;

#endif  // CONFIG_CLKMGR_EMULATION
}
EXPORT_SYMBOL(disable_pll);

int pll_fsel(pll_id id, unsigned int value)
{
    int err;
    unsigned long flags;
    struct pll *pll = id_to_pll(id);

    ENTER_FUNC(FUNC_LV_API);

#if defined(CONFIG_CLKMGR_EMULATION)

    return 0;

#else   // CONFIG_CLKMGR_EMULATION

    if (WHPLL != id)
    {
        // WARN_ON(WHPLL != id); // XXX: ARMPLL / MAINPLL / UNIVPLL are not controlled by AP side
        return 0;
    }

    BUG_ON(!initialized);
    BUG_ON(!pll);

    clkmgr_lock(flags);
    err = pll_fsel_locked(pll, value);
    clkmgr_unlock(flags);

    EXIT_FUNC(FUNC_LV_API);
    return err;

#endif  // CONFIG_CLKMGR_EMULATION
}
EXPORT_SYMBOL(pll_fsel);

int pll_dump_regs(pll_id id, unsigned int *ptr)
{
    struct pll *pll = id_to_pll(id);

    ENTER_FUNC(FUNC_LV_DONT_CARE);

    BUG_ON(!initialized);
    BUG_ON(!pll);

    EXIT_FUNC(FUNC_LV_DONT_CARE);
    return pll->ops->dump_regs(pll, ptr);
}
EXPORT_SYMBOL(pll_dump_regs);

const char *pll_get_name(pll_id id)
{
    struct pll *pll = id_to_pll(id);

    ENTER_FUNC(FUNC_LV_DONT_CARE);

    BUG_ON(!initialized);
    BUG_ON(!pll);

    EXIT_FUNC(FUNC_LV_DONT_CARE);
    return pll->name;
}

#if 0   // XXX: NOT USED @ MT6572
#define CLKSQ1_EN           BIT(0)
#define CLKSQ1_LPF_EN       BIT(1)
#define CLKSQ1_EN_SEL       BIT(0)
#define CLKSQ1_LPF_EN_SEL   BIT(1)

void enable_clksq1(void)
{
    unsigned long flags;

    clkmgr_lock(flags);
    clk_setl(AP_PLL_CON0, CLKSQ1_EN);
    udelay(200);
    clk_setl(AP_PLL_CON0, CLKSQ1_LPF_EN);
    clkmgr_unlock(flags);
}
EXPORT_SYMBOL(enable_clksq1);

void disable_clksq1(void)
{
    unsigned long flags;

    clkmgr_lock(flags);
    clk_clrl(AP_PLL_CON0, CLKSQ1_LPF_EN | CLKSQ1_EN);
    clkmgr_unlock(flags);
}
EXPORT_SYMBOL(disable_clksq1);

void clksq1_sw2hw(void)
{
    unsigned long flags;

    clkmgr_lock(flags);
    clk_clrl(AP_PLL_CON1, CLKSQ1_LPF_EN_SEL | CLKSQ1_EN_SEL);
    clkmgr_unlock(flags);
}
EXPORT_SYMBOL(clksq1_sw2hw);

void clksq2_hw2sw(void)
{
    unsigned long flags;

    clkmgr_lock(flags);
    clk_setl(AP_PLL_CON1, CLKSQ1_LPF_EN_SEL | CLKSQ1_EN_SEL);
    clkmgr_unlock(flags);
}
EXPORT_SYMBOL(clksq1_hw2sw);
#endif  // XXX: NOT USED @ MT6572

//
// LARB related
//
static DEFINE_MUTEX(larb_monitor_lock);
static LIST_HEAD(larb_monitor_handlers);

void register_larb_monitor(struct larb_monitor *handler)
{
    struct list_head *pos;

    ENTER_FUNC(FUNC_LV_API);

#if defined(CONFIG_CLKMGR_EMULATION)

#else   // CONFIG_CLKMGR_EMULATION

    mutex_lock(&larb_monitor_lock);
    list_for_each(pos, &larb_monitor_handlers)
    {
        struct larb_monitor *l;
        l = list_entry(pos, struct larb_monitor, link);

        if (l->level > handler->level)
        {
            break;
        }
    }
    list_add_tail(&handler->link, pos);
    mutex_unlock(&larb_monitor_lock);

    EXIT_FUNC(FUNC_LV_API);

#endif  // CONFIG_CLKMGR_EMULATION
}
EXPORT_SYMBOL(register_larb_monitor);


void unregister_larb_monitor(struct larb_monitor *handler)
{
    ENTER_FUNC(FUNC_LV_API);

#if defined(CONFIG_CLKMGR_EMULATION)

#else   // CONFIG_CLKMGR_EMULATION

    mutex_lock(&larb_monitor_lock);
    list_del(&handler->link);
    mutex_unlock(&larb_monitor_lock);

    EXIT_FUNC(FUNC_LV_API);

#endif  // CONFIG_CLKMGR_EMULATION
}
EXPORT_SYMBOL(unregister_larb_monitor);

static void larb_clk_prepare(int larb_idx)
{
    ENTER_FUNC(FUNC_LV_BODY);

    switch (larb_idx)
    {
    case MT_LARB0:
        {
        #if 0
            struct cg_clk *cg_clk = id_to_clk(MT_CG_SMI_LARB0_SW_CG);

            // BUG_ON(PWR_ON == cg_clk->ops->get_state(cg_clk)); // TODO: check it

            cg_clk->ops->enable(cg_clk);
        #else // XXX: if remove MT_CG_SMI_COMMON_SW_CG & MT_CG_SMI_LARB0_SW_CG
            clk_writel(MMSYS_CG_CLR0, SMI_COMMON_SW_CG_BIT | SMI_LARB0_SW_CG_BIT);
        #endif
        }
        break;

    default:
        BUG();
    }

    EXIT_FUNC(FUNC_LV_BODY);
}

static void larb_clk_finish(int larb_idx)
{
    ENTER_FUNC(FUNC_LV_BODY);

    switch (larb_idx)
    {
    case MT_LARB0:
        {
        #if 0
            struct cg_clk *cg_clk = id_to_clk(MT_CG_SMI_LARB0_SW_CG);

            // BUG_ON(PWR_DOWN == cg_clk->ops->get_state(cg_clk)); // TODO: check it

            cg_clk->ops->disable(cg_clk);
        #else // XXX: if remove MT_CG_SMI_COMMON_SW_CG & MT_CG_SMI_LARB0_SW_CG
            // clk_writel(MMSYS_CG_SET0, SMI_COMMON_SW_CG_BIT | SMI_LARB0_SW_CG_BIT);
        #endif
        }
        break;

    default:
        BUG();
    }

    EXIT_FUNC(FUNC_LV_BODY);
}

static void larb_backup(int larb_idx)
{
    struct larb_monitor *pos;

    ENTER_FUNC(FUNC_LV_BODY);

    // clk_dbg("[%s]: start to backup larb%d\n", __func__, larb_idx);
    larb_clk_prepare(larb_idx);

    list_for_each_entry(pos, &larb_monitor_handlers, link)
    {
        if (pos->backup != NULL)
        {
            pos->backup(pos, larb_idx);
        }
    }

    larb_clk_finish(larb_idx);

    EXIT_FUNC(FUNC_LV_BODY);
}

static void larb_restore(int larb_idx)
{
    struct larb_monitor *pos;

    ENTER_FUNC(FUNC_LV_BODY);

    // clk_dbg("[%s]: start to restore larb%d\n", __func__, larb_idx);
    larb_clk_prepare(larb_idx);

    list_for_each_entry(pos, &larb_monitor_handlers, link)
    {
        if (pos->restore != NULL)
        {
            pos->restore(pos, larb_idx);
        }
    }

    larb_clk_finish(larb_idx);

    EXIT_FUNC(FUNC_LV_BODY);
}

//
// SUBSYS variable & function
//
#define SYS_TYPE_MODEM    0
#define SYS_TYPE_MEDIA    1
#define SYS_TYPE_OTHER    2

static struct subsys_ops general_sys_ops;
#if 0
static struct subsys_ops md1_sys_ops;
static struct subsys_ops con_sys_ops;
#endif
static struct subsys_ops dis_sys_ops;
static struct subsys_ops mfg_sys_ops;

#define MD1_PROT_MASK BITMASK(10:7)

static struct subsys syss[] = // NR_SYSS
{
    [SYS_MD1] =
    {
        .name               = __stringify(SYS_MD1),
        .type               = SYS_TYPE_MODEM,
        .default_sta        = PWR_DOWN,
        .sta_mask           = MD1_PWR_STA_MASK,
        .ctl_addr           = SPM_MD_PWR_CON,
        .sram_pdn_bits      = BITMASK(8:8),
        .sram_pdn_ack_bits  = 0,
        .bus_prot_mask      = MD1_PROT_MASK, // TODO: bus protect for abnormal reset?
        // .pwr_ctrl           = spm_mtcmos_ctrl_mdsys1,
        .ops                = &general_sys_ops, // &md1_sys_ops
    },
    [SYS_CON] =
    {
        .name               = __stringify(SYS_CON),
        .type               = SYS_TYPE_MODEM,
        .default_sta        = PWR_DOWN,
        .sta_mask           = CON_PWR_STA_MASK,
        .ctl_addr           = SPM_CONN_PWR_CON,
        .sram_pdn_bits      = 0,
        .sram_pdn_ack_bits  = 0,
        .bus_prot_mask      = BIT(4) | BIT(0),
        // .pwr_ctrl           = spm_mtcmos_ctrl_connsys,
        .ops                = &general_sys_ops, // &con_sys_ops
    },
    [SYS_DIS] =
    {
        .name               = __stringify(SYS_DIS),
        .type               = SYS_TYPE_MEDIA,
        .default_sta        = PWR_ON,
        .sta_mask           = DIS_PWR_STA_MASK,
        .ctl_addr           = SPM_DIS_PWR_CON,
        .sram_pdn_bits      = BITMASK(11:8),
        .sram_pdn_ack_bits  = BITMASK(15:12),
        .bus_prot_mask      = BIT(11),
        // .pwr_ctrl           = spm_mtcmos_ctrl_disp,
        .ops                = &dis_sys_ops,
        .start              = &grps[CG_MMSYS0],
        .nr_grps            = 2,
    },
    [SYS_MFG] =
    {
        .name               = __stringify(SYS_MFG),
        .type               = SYS_TYPE_MEDIA,
        .default_sta        = PWR_DOWN,
        .sta_mask           = MFG_PWR_STA_MASK,
        .ctl_addr           = SPM_MFG_PWR_CON,
        .sram_pdn_bits      = BITMASK(8:8),
        .sram_pdn_ack_bits  = BITMASK(12:12),
        .bus_prot_mask      = 0,
        // .pwr_ctrl           = spm_mtcmos_ctrl_mfg,
        .ops                = &mfg_sys_ops,
        .start              = &grps[CG_MFG],
        .nr_grps            = 1,
    },
};

static struct subsys *id_to_sys(unsigned int id)
{
    return id < NR_SYSS ? &syss[id] : NULL;
}

#define FMT7	",%d,%d,%d,%d,%d,%d,%d\n"
#define VAL7	,value[0],value[1],value[2],value[3],value[4],value[5],value[6]

#define SAMPLE_FMT	"%5lu.%06lu"
#define SAMPLE_VAL	(unsigned long)(timestamp), nano_rem/1000

void ms_mtcmos(unsigned long long timestamp, unsigned char cnt, unsigned int *value)
{
	unsigned long nano_rem = do_div(timestamp, 1000000000);

	switch (cnt) {
	case 7: trace_printk(SAMPLE_FMT FMT7, SAMPLE_VAL VAL7); break;
	}
}

void print_mtcmos_trace_info_for_met(void)
{
    unsigned int mtcmos[7];
    unsigned int reg_val = spm_read(SPM_PWR_STATUS);

    mtcmos[0] = (reg_val & MD1_PWR_STA_MASK) ? 1 : 0; // md
    mtcmos[1] = (reg_val & CON_PWR_STA_MASK) ? 1 : 0; // conn
    mtcmos[2] = (reg_val & DIS_PWR_STA_MASK) ? 1 : 0; // disp
    mtcmos[3] = (reg_val & MFG_PWR_STA_MASK) ? 1 : 0; // mfg
    mtcmos[4] = (reg_val & IFR_PWR_STA_MASK) ? 1 : 0; // infra
    mtcmos[5] = (reg_val & FC1_PWR_STA_MASK) ? 1 : 0; // fc1
    mtcmos[6] = (reg_val & FC0_PWR_STA_MASK) ? 1 : 0; // fc0

    ms_mtcmos(cpu_clock(0), 7, mtcmos);
}

static int spm_mtcmos_ctrl_general_locked(struct subsys *sys,
                                          int state
                                          )
{
    int i;
    int err = 0;
    unsigned long expired = jiffies + HZ/10;

    ENTER_FUNC(FUNC_LV_API);

#if defined(CONFIG_CLKMGR_EMULATION)
#else

    spm_write(SPM_POWERON_CONFIG_SET, (SPM_PROJECT_CODE << 16) | (1U << 0));

    if (state == STA_POWER_DOWN)
    {
        // BUS_PROTECT                                                                              // enable BUS protect
        if (0 != sys->bus_prot_mask)
        {
            spm_write(INFRA_TOPAXI_PROTECTEN, spm_read(INFRA_TOPAXI_PROTECTEN) | sys->bus_prot_mask);
            while ((spm_read(INFRA_TOPAXI_PROTECTSTA1) & sys->bus_prot_mask) != sys->bus_prot_mask)
            {
                if (time_after(jiffies, expired))
                {
                    WARN_ON(1);
                    break;
                }
                /*
                 * FIXME: Workaround for CONNSYS
                 * The clock of bus protect ip of CONNSYS comes from conn2ap_ahb_hclk_ck.
                 * Bus protect ip won't return ACK if no clock.
                 * If CONNSYS bus goes to idle mode, clock will be gated.
                 * Issue a dummy read to enable bus clock temporarily.
                 */
                if (unlikely(sys == &syss[SYS_CON]))
                {
                    /*
                     * Issue a dummy read (read chip id) to CONNSYS.
                     */
                    *(volatile unsigned int *)0xF8070008;
                }
            }
        }

        spm_write(sys->ctl_addr, spm_read(sys->ctl_addr) | PWR_CLK_DIS_BIT);                        // PWR_CLK_DIS = 1
        spm_write(sys->ctl_addr, spm_read(sys->ctl_addr) | PWR_ISO_BIT);                            // PWR_ISO = 1
        spm_write(sys->ctl_addr, spm_read(sys->ctl_addr) & ~PWR_RST_B_BIT);                         // PWR_RST_B = 0

        // SRAM_PDN                                                                                 // MEM power off
        for (i = BIT(8); i <= sys->sram_pdn_bits; i = (i << 1) + BIT(8))                            // set SRAM_PDN 1 one by one
        {
            spm_write(sys->ctl_addr, spm_read(sys->ctl_addr) | i);
        }

        while (   sys->sram_pdn_ack_bits
               && ((spm_read(sys->ctl_addr) & sys->sram_pdn_ack_bits) != sys->sram_pdn_ack_bits)    // wait until SRAM_PDN_ACK all 1
               )
        {
            if (time_after(jiffies, expired))
            {
                WARN_ON(1);
                break;
            }
        }

        spm_write(sys->ctl_addr, spm_read(sys->ctl_addr) & ~(PWR_ON_BIT));                          // PWR_ON = 0
        udelay(1);                                                                                  // delay 1 us
        spm_write(sys->ctl_addr, spm_read(sys->ctl_addr) & ~(PWR_ON_S_BIT));                        // PWR_ON_S = 0
        udelay(1);                                                                                  // delay 1 us

        while (   (spm_read(SPM_PWR_STATUS)   & sys->sta_mask)                                      // wait until PWR_ACK = 0
               || (spm_read(SPM_PWR_STATUS_S) & sys->sta_mask)
               )
        {
            if (time_after(jiffies, expired))
            {
                WARN_ON(1);
                break;
            }
        }
    }
    else /* STA_POWER_ON */
    {
        spm_write(sys->ctl_addr, spm_read(sys->ctl_addr) | PWR_ON_BIT);                             // PWR_ON = 1
        udelay(1);                                                                                  // delay 1 us
        spm_write(sys->ctl_addr, spm_read(sys->ctl_addr) | PWR_ON_S_BIT);                           // PWR_ON_S = 1
        udelay(3);                                                                                  // delay 3 us

        while (   !(spm_read(SPM_PWR_STATUS)   & sys->sta_mask)                                     // wait until PWR_ACK = 1
               || !(spm_read(SPM_PWR_STATUS_S) & sys->sta_mask)
               )
        {
            if (time_after(jiffies, expired))
            {
                WARN_ON(1);
                break;
            }
        }

        // SRAM_PDN                                                                                 // MEM power on
        for (i = BIT(8); i <= sys->sram_pdn_bits; i = (i << 1) + BIT(8))                            // set SRAM_PDN 0 one by one
        {
            spm_write(sys->ctl_addr, spm_read(sys->ctl_addr) & ~i);
        }

        spm_write(sys->ctl_addr, spm_read(sys->ctl_addr) & ~PWR_CLK_DIS_BIT);                       // PWR_CLK_DIS = 0
        spm_write(sys->ctl_addr, spm_read(sys->ctl_addr) & ~PWR_ISO_BIT);                           // PWR_ISO = 0
        spm_write(sys->ctl_addr, spm_read(sys->ctl_addr) | PWR_CLK_DIS_BIT);                        // PWR_CLK_DIS = 1
        spm_write(sys->ctl_addr, spm_read(sys->ctl_addr) | PWR_RST_B_BIT);                          // PWR_RST_B = 1
        spm_write(sys->ctl_addr, spm_read(sys->ctl_addr) & ~PWR_CLK_DIS_BIT);                       // PWR_CLK_DIS = 0

        while (   sys->sram_pdn_ack_bits
               && (spm_read(sys->ctl_addr) & sys->sram_pdn_ack_bits)                                // wait until SRAM_PDN_ACK all 0
               )
        {
            if (time_after(jiffies, expired))
            {
                WARN_ON(1);
                break;
            }
        }

        // BUS_PROTECT                                                                              // disable BUS protect
        if (0 != sys->bus_prot_mask)
        {
            spm_write(INFRA_TOPAXI_PROTECTEN, spm_read(INFRA_TOPAXI_PROTECTEN) & ~sys->bus_prot_mask);
            while (spm_read(INFRA_TOPAXI_PROTECTSTA1) & sys->bus_prot_mask)
            {
                if (time_after(jiffies, expired))
                {
                    WARN_ON(1);
                    break;
                }
            }
        }
    }

    print_mtcmos_trace_info_for_met(); // XXX: for MET

    // spm_write(SPM_POWERON_CONFIG_SET, (SPM_PROJECT_CODE << 16) | (0U << 0));

#endif // defined(CONFIG_CLKMGR_EMULATION)

    EXIT_FUNC(FUNC_LV_API);

    return err;
}


static int general_sys_enable_op(struct subsys *sys)
{
    int err;

    ENTER_FUNC(FUNC_LV_OP);

    err = spm_mtcmos_ctrl_general_locked(sys, STA_POWER_ON);

    EXIT_FUNC(FUNC_LV_OP);
    return err;
}

static int general_sys_disable_op(struct subsys *sys)
{
    int err;

    ENTER_FUNC(FUNC_LV_OP);

    err = spm_mtcmos_ctrl_general_locked(sys, STA_POWER_DOWN);

    EXIT_FUNC(FUNC_LV_OP);
    return err;
}

#if 0
static int md1_sys_enable_op(struct subsys *sys)
{
    int err;

    ENTER_FUNC(FUNC_LV_OP);

    err = spm_mtcmos_ctrl_general_locked(sys, STA_POWER_ON);

    EXIT_FUNC(FUNC_LV_OP);
    return err;
}

static int md1_sys_disable_op(struct subsys *sys)
{
    int err;

    ENTER_FUNC(FUNC_LV_OP);

    err = spm_mtcmos_ctrl_general_locked(sys, STA_POWER_DOWN);

    EXIT_FUNC(FUNC_LV_OP);
    return err;
}

static int con_sys_enable_op(struct subsys *sys)
{
    int err;

    ENTER_FUNC(FUNC_LV_OP);

    err = spm_mtcmos_ctrl_general_locked(sys, STA_POWER_ON);

    EXIT_FUNC(FUNC_LV_OP);
    return err;
}

static int con_sys_disable_op(struct subsys *sys)
{
    int err;

    ENTER_FUNC(FUNC_LV_OP);

    err = spm_mtcmos_ctrl_general_locked(sys, STA_POWER_DOWN);

    EXIT_FUNC(FUNC_LV_OP);
    return err;
}
#endif

static int mfg_sys_enable_op(struct subsys *sys)
{
    int err;
    struct cg_clk *cg_clk_mmsys = id_to_clk(MT_CG_MUTEX_SLOW_CLOCK_SW_CG);
    struct cg_clk *cg_clk = id_to_clk(MT_CG_MFG_MM_SW_CG);
    int is_mfg_cg_off = FALSE;

    ENTER_FUNC(FUNC_LV_OP);

    enable_clock_locked(cg_clk_mmsys);

    if (PWR_DOWN == cg_clk->ops->get_state(cg_clk))
    {
        is_mfg_cg_off = TRUE;
        enable_clock_locked(cg_clk);
    }

    err = spm_mtcmos_ctrl_general_locked(sys, STA_POWER_ON);

    if (TRUE == is_mfg_cg_off)
    {
        disable_clock_locked(cg_clk);
    }

    EXIT_FUNC(FUNC_LV_OP);
    return err;
}

static int mfg_sys_disable_op(struct subsys *sys)
{
    int err;
    struct cg_clk *cg_clk_mmsys = id_to_clk(MT_CG_MUTEX_SLOW_CLOCK_SW_CG);
    struct cg_clk *cg_clk = id_to_clk(MT_CG_MFG_MM_SW_CG);
    int is_mfg_cg_off = FALSE;

    ENTER_FUNC(FUNC_LV_OP);

    if (PWR_DOWN == cg_clk->ops->get_state(cg_clk))
    {
        is_mfg_cg_off = TRUE;
        enable_clock_locked(cg_clk);
    }

    err = spm_mtcmos_ctrl_general_locked(sys, STA_POWER_DOWN);

    if (TRUE == is_mfg_cg_off)
    {
        disable_clock_locked(cg_clk);
    }

    disable_clock_locked(cg_clk_mmsys);

    EXIT_FUNC(FUNC_LV_OP);
    return err;
}

static int dis_sys_enable_op(struct subsys *sys)
{
    int err = 0;

    ENTER_FUNC(FUNC_LV_OP);

    spm_write(SPM_POWERON_CONFIG_SET, (SPM_PROJECT_CODE << 16) | (1U << 0));
    spm_write(SPM_POWER_ON_VAL0, spm_read(SPM_POWER_ON_VAL0) & ~BIT(4));
    // spm_write(SPM_POWERON_CONFIG_SET, (SPM_PROJECT_CODE << 16) | (0U << 0));

    if (   !(spm_read(SPM_PWR_STATUS)   & sys->sta_mask)
        || !(spm_read(SPM_PWR_STATUS_S) & sys->sta_mask)
        )
    {
        err = spm_mtcmos_ctrl_general_locked(sys, STA_POWER_ON);
    }

    larb_restore(MT_LARB0);

    clk_writel(MMSYS_CG_CLR0, SMI_COMMON_SW_CG_BIT | SMI_LARB0_SW_CG_BIT | MM_CMDQ_SW_CG_BIT | MM_CMDQ_SMI_IF_SW_CG_BIT); // XXX: if remove MT_CG_SMI_COMMON_SW_CG & MT_CG_SMI_LARB0_SW_CG

    EXIT_FUNC(FUNC_LV_OP);
    return err;
}

static int dis_sys_disable_op(struct subsys *sys)
{
    int err;

    ENTER_FUNC(FUNC_LV_OP);

    clk_writel(MMSYS_CG_SET0, SMI_COMMON_SW_CG_BIT | SMI_LARB0_SW_CG_BIT | MM_CMDQ_SW_CG_BIT | MM_CMDQ_SMI_IF_SW_CG_BIT); // XXX: if remove MT_CG_SMI_COMMON_SW_CG & MT_CG_SMI_LARB0_SW_CG

    larb_backup(MT_LARB0);
    err = spm_mtcmos_ctrl_general_locked(sys, STA_POWER_DOWN);

    spm_write(SPM_POWERON_CONFIG_SET, (SPM_PROJECT_CODE << 16) | (1U << 0));
    spm_write(SPM_POWER_ON_VAL0, spm_read(SPM_POWER_ON_VAL0) | BIT(4));
    // spm_write(SPM_POWERON_CONFIG_SET, (SPM_PROJECT_CODE << 16) | (0U << 0));

    EXIT_FUNC(FUNC_LV_OP);
    return err;
}

static int sys_get_state_op(struct subsys *sys)
{
    unsigned int sta = clk_readl(SPM_PWR_STATUS);
    unsigned int sta_s = clk_readl(SPM_PWR_STATUS_S);

    ENTER_FUNC(FUNC_LV_OP);
    EXIT_FUNC(FUNC_LV_OP);

    return (sta & sys->sta_mask) && (sta_s & sys->sta_mask);
}

static int sys_dump_regs_op(struct subsys *sys, unsigned int *ptr)
{
    ENTER_FUNC(FUNC_LV_OP);
    *(ptr) = clk_readl(sys->ctl_addr);

    EXIT_FUNC(FUNC_LV_OP);
    return 1; // return size
}

static struct subsys_ops general_sys_ops =
{
    .enable     = general_sys_enable_op,
    .disable    = general_sys_disable_op,
    .get_state  = sys_get_state_op,
    .dump_regs  = sys_dump_regs_op,
};

#if 0
static struct subsys_ops md1_sys_ops =
{
    .enable     = md1_sys_enable_op,
    .disable    = md1_sys_disable_op,
    .get_state  = sys_get_state_op,
    .dump_regs  = sys_dump_regs_op,
};

static struct subsys_ops con_sys_ops =
{
    .enable     = con_sys_enable_op,
    .disable    = con_sys_disable_op,
    .get_state  = sys_get_state_op,
    .dump_regs  = sys_dump_regs_op,
};
#endif

static struct subsys_ops mfg_sys_ops =
{
    .enable     = mfg_sys_enable_op,
    .disable    = mfg_sys_disable_op,
    .get_state  = sys_get_state_op,
    .dump_regs  = sys_dump_regs_op,
};

static struct subsys_ops dis_sys_ops =
{
    .enable     = dis_sys_enable_op,
    .disable    = dis_sys_disable_op,
    .get_state  = sys_get_state_op,
    .dump_regs  = sys_dump_regs_op,
};

static int get_sys_state_locked(struct subsys *sys)
{
    ENTER_FUNC(FUNC_LV_LOCKED);

    if (likely(initialized))
    {
        EXIT_FUNC(FUNC_LV_LOCKED);
        return sys->state;                  // after init, get from local_state
    }
    else
    {
        EXIT_FUNC(FUNC_LV_LOCKED);
        return sys->ops->get_state(sys);    // before init, get from reg_val
    }
}

int subsys_is_on(subsys_id id)
{
    int state;
    unsigned long flags;
    struct subsys *sys = id_to_sys(id);

    ENTER_FUNC(FUNC_LV_API);

#if defined(CONFIG_CLKMGR_EMULATION)

    return 0;

#else   // CONFIG_CLKMGR_EMULATION

    BUG_ON(!sys);

    clkmgr_lock(flags);
    state = get_sys_state_locked(sys);
    clkmgr_unlock(flags);

    EXIT_FUNC(FUNC_LV_API);
    return state;

#endif  // CONFIG_CLKMGR_EMULATION
}
EXPORT_SYMBOL(subsys_is_on);

static int enable_subsys_locked(struct subsys *sys)
{
    int err;
    int local_state = sys->state; //get_subsys_local_state(sys);

    ENTER_FUNC(FUNC_LV_LOCKED);

#ifdef STATE_CHECK_DEBUG
    int reg_state = sys->ops->get_state(sys);//get_subsys_reg_state(sys);
    BUG_ON(local_state != reg_state);
#endif

    if (local_state == PWR_ON)
    {
        EXIT_FUNC(FUNC_LV_LOCKED);
        return 0; // XXX: ??? just check local_state ???
    }

    //err = sys->pwr_ctrl(STA_POWER_ON);
    err = sys->ops->enable(sys);
    WARN_ON(err);

    if (!err)
    {
        sys->state = PWR_ON;
    }

    EXIT_FUNC(FUNC_LV_LOCKED);
    return err;
}

static int disable_subsys_locked(struct subsys *sys, int force_off)
{
    int err;
    int local_state = sys->state;//get_subsys_local_state(sys);
    int i;
    const struct cg_grp *grp;

    ENTER_FUNC(FUNC_LV_LOCKED);

#ifdef STATE_CHECK_DEBUG
    int reg_state = sys->ops->get_state(sys);//get_subsys_reg_state(sys);
    BUG_ON(local_state != reg_state);
#endif

    if (!force_off) // XXX: check all clock gate groups related to this subsys are off
    {
        //could be power off or not
        for (i = 0; i < sys->nr_grps; i++)
        {
            grp = sys->start + i;

            if (grp->state)
            {
                EXIT_FUNC(FUNC_LV_LOCKED);
                return 0;
            }
        }
    }

    if (local_state == PWR_DOWN)
    {
        EXIT_FUNC(FUNC_LV_LOCKED);
        return 0;
    }

    //err = sys->pwr_ctrl(STA_POWER_DOWN);
    err = sys->ops->disable(sys);
    WARN_ON(err);

    if (!err)
    {
        sys->state = PWR_DOWN;
    }

    EXIT_FUNC(FUNC_LV_LOCKED);
    return err;
}

int enable_subsys(subsys_id id, const char *name)
{
    int err;
    unsigned long flags;
    struct subsys *sys = id_to_sys(id);

    ENTER_FUNC(FUNC_LV_API);

#if defined(CONFIG_CLKMGR_EMULATION)

    return 0;

#else   // CONFIG_CLKMGR_EMULATION

    BUG_ON(!initialized);
    BUG_ON(!sys);

    clkmgr_lock(flags);
    err = enable_subsys_locked(sys);
    clkmgr_unlock(flags);

    EXIT_FUNC(FUNC_LV_API);
    return err;

#endif  // CONFIG_CLKMGR_EMULATION
}
EXPORT_SYMBOL(enable_subsys);

int disable_subsys(subsys_id id, const char *name)
{
    int err;
    unsigned long flags;
    struct subsys *sys = id_to_sys(id);

    ENTER_FUNC(FUNC_LV_API);

#if defined(CONFIG_CLKMGR_EMULATION)

    return 0;

#else   // CONFIG_CLKMGR_EMULATION

    BUG_ON(!initialized);
    BUG_ON(!sys);

    clkmgr_lock(flags);
    err = disable_subsys_locked(sys, 0);
    clkmgr_unlock(flags);

    EXIT_FUNC(FUNC_LV_API);
    return err;

#endif  // CONFIG_CLKMGR_EMULATION
}
EXPORT_SYMBOL(disable_subsys);

int disable_subsys_force(subsys_id id, const char *name)
{
    int err;
    unsigned long flags;
    struct subsys *sys = id_to_sys(id);

    ENTER_FUNC(FUNC_LV_API);

#if defined(CONFIG_CLKMGR_EMULATION)

    return 0;

#else   // CONFIG_CLKMGR_EMULATION

    BUG_ON(!initialized);
    BUG_ON(!sys);

    clkmgr_lock(flags);
    err = disable_subsys_locked(sys, 1);
    clkmgr_unlock(flags);

    EXIT_FUNC(FUNC_LV_API);
    return err;

#endif  // CONFIG_CLKMGR_EMULATION
}
EXPORT_SYMBOL(disable_subsys_force);

int subsys_dump_regs(subsys_id id, unsigned int *ptr)
{
    struct subsys *sys = id_to_sys(id);

    ENTER_FUNC(FUNC_LV_DONT_CARE);

    BUG_ON(!initialized);
    BUG_ON(!sys);

    EXIT_FUNC(FUNC_LV_DONT_CARE);
    return sys->ops->dump_regs(sys, ptr);
}
EXPORT_SYMBOL(subsys_dump_regs);

const char *subsys_get_name(subsys_id id)
{
    struct subsys *sys = id_to_sys(id);

    ENTER_FUNC(FUNC_LV_DONT_CARE);

    BUG_ON(!initialized);
    BUG_ON(!sys);

    EXIT_FUNC(FUNC_LV_DONT_CARE);
    return sys->name;
}

#define JIFFIES_PER_LOOP 10

int md_power_on(subsys_id id)
{
    int err;
    unsigned long flags;
    struct subsys *sys = id_to_sys(id);

    ENTER_FUNC(FUNC_LV_API);

#if defined(CONFIG_CLKMGR_EMULATION)

    return 0;

#else   // CONFIG_CLKMGR_EMULATION

    BUG_ON(!initialized);
    BUG_ON(!sys);
    BUG_ON(sys->type != SYS_TYPE_MODEM);
    BUG_ON(id != SYS_MD1);

    clkmgr_lock(flags);
    if (spm_read(INFRA_TOPAXI_PROTECTEN) & MD1_PROT_MASK)
    {
        syss[SYS_MD1].bus_prot_mask = MD1_PROT_MASK;
    }
    else
    {
        syss[SYS_MD1].bus_prot_mask = 0;
    }
    err = enable_subsys_locked(sys);
    clkmgr_unlock(flags);

    WARN_ON(err);

    EXIT_FUNC(FUNC_LV_API);
    return err;

#endif  // CONFIG_CLKMGR_EMULATION
}
EXPORT_SYMBOL(md_power_on);

int md_power_off(subsys_id id, unsigned int timeout)
{
    int err;
    int cnt;
    bool slept;
    unsigned long flags;
    struct subsys *sys = id_to_sys(id);

    ENTER_FUNC(FUNC_LV_API);

#if defined(CONFIG_CLKMGR_EMULATION)

    return 0;

#else   // CONFIG_CLKMGR_EMULATION

    BUG_ON(!initialized);
    BUG_ON(!sys);
    BUG_ON(sys->type != SYS_TYPE_MODEM);
    BUG_ON(id != SYS_MD1);

    // 0: not sleep, 1: sleep
    slept = spm_is_md_sleep();
    cnt = (timeout + JIFFIES_PER_LOOP - 1) / JIFFIES_PER_LOOP;

    while (!slept && cnt--)
    {
        msleep(MSEC_PER_SEC / JIFFIES_PER_LOOP);
        slept = spm_is_md_sleep();

        if (slept)
        {
            break;
        }
    }

    clkmgr_lock(flags);
    // XXX: md (abnormal) reset or flight mode fail
    if (0 == timeout || 0 == slept)
    {
        syss[SYS_MD1].bus_prot_mask = MD1_PROT_MASK;
    }
    // XXX: flight mode (i.e. not to set bus protect)
    else
    {
        syss[SYS_MD1].bus_prot_mask = 0;
    }
    err = disable_subsys_locked(sys, 0);
    clkmgr_unlock(flags);

    WARN_ON(err);

    EXIT_FUNC(FUNC_LV_API);
    return !slept;

#endif  // CONFIG_CLKMGR_EMULATION
}
EXPORT_SYMBOL(md_power_off);

int conn_power_on(subsys_id id)
{
    int err;
    unsigned long flags;
    struct subsys *sys = id_to_sys(id);

    ENTER_FUNC(FUNC_LV_API);

#if defined(CONFIG_CLKMGR_EMULATION)

    return 0;

#else   // CONFIG_CLKMGR_EMULATION

    BUG_ON(!initialized);
    BUG_ON(!sys);
    BUG_ON(sys->type != SYS_TYPE_MODEM);
    BUG_ON(id != SYS_CON);

    clkmgr_lock(flags);
    err = enable_subsys_locked(sys);
    clkmgr_unlock(flags);

    WARN_ON(err);

    EXIT_FUNC(FUNC_LV_API);
    return err;

#endif  // CONFIG_CLKMGR_EMULATION
}
EXPORT_SYMBOL(conn_power_on);

int conn_power_off(subsys_id id, unsigned int timeout)
{
    int err;
    int cnt;
    bool slept;
    unsigned long flags;
    struct subsys *sys = id_to_sys(id);

    ENTER_FUNC(FUNC_LV_API);

#if defined(CONFIG_CLKMGR_EMULATION)

    return 0;

#else   // CONFIG_CLKMGR_EMULATION

    BUG_ON(!initialized);
    BUG_ON(!sys);
    BUG_ON(sys->type != SYS_TYPE_MODEM);
    BUG_ON(id != SYS_CON);

    // 0: not sleep, 1: sleep
    slept = spm_is_conn_sleep();
    cnt = (timeout + JIFFIES_PER_LOOP - 1) / JIFFIES_PER_LOOP;

    while (!slept && cnt--)
    {
        msleep(MSEC_PER_SEC / JIFFIES_PER_LOOP);
        slept = spm_is_conn_sleep();

        if (slept)
        {
            break;
        }
    }

    clkmgr_lock(flags);
    err = disable_subsys_locked(sys, 0);
    clkmgr_unlock(flags);

    WARN_ON(err);

    EXIT_FUNC(FUNC_LV_API);
    return !slept;

#endif  // CONFIG_CLKMGR_EMULATION
}
EXPORT_SYMBOL(conn_power_off);

#if 0   // XXX: NOT USED @ MT6572
#define PMICSPI_CLKMUX_MASK 0x7
#define PMICSPI_MEMPLL_D4   0x5
#define PMICSPI_CLKSQ       0x0
void pmicspi_mempll2clksq(void)
{
    volatile unsigned int val;
    val = clk_readl(CLK_CFG_8);
    //BUG_ON((val & PMICSPI_CLKMUX_MASK) != PMICSPI_MEMPLL_D4);

    val = (val & ~PMICSPI_CLKMUX_MASK) | PMICSPI_CLKSQ;
    clk_writel(CLK_CFG_8, val);
}
EXPORT_SYMBOL(pmicspi_mempll2clksq);

void pmicspi_clksq2mempll(void)
{
    volatile unsigned int val;
    val = clk_readl(CLK_CFG_8);

    val = (val & ~PMICSPI_CLKMUX_MASK) | PMICSPI_MEMPLL_D4;
    clk_writel(CLK_CFG_8, val);
}
EXPORT_SYMBOL(pmicspi_clksq2mempll);
#endif  // XXX: NOT USED @ MT6572

unsigned int mt_get_emi_freq(void)
{
    unsigned int output_freq, clk_mux_sel;

    ENTER_FUNC(FUNC_LV_API);

#if defined(CONFIG_CLKMGR_EMULATION)

    output_freq = 533000;

#else

    output_freq = pll_freq_calc_op(&plls[MAINPLL]);

    clk_mux_sel = (DRV_Reg32(CLK_MUX_SEL) >> 1) & 0xF;

    switch (clk_mux_sel)
    {
    case 9:
        output_freq /= 3; // 533000;
        break;
    case 10:
        output_freq /= 4; // 399750;
        break;
    case 12:
        output_freq /= 2; // 663000;
        break;
    default:
        output_freq = 26000;
        break;
    }

#endif // defined(CONFIG_CLKMGR_EMULATION)

    EXIT_FUNC(FUNC_LV_API);

    return output_freq; // KHz
}
EXPORT_SYMBOL(mt_get_emi_freq);

unsigned int mt_get_bus_freq(void)
{
    unsigned int output_freq, clk_mux_sel;

    ENTER_FUNC(FUNC_LV_API);

#if defined(CONFIG_CLKMGR_EMULATION)

    output_freq = 133250;

#else

    output_freq = pll_freq_calc_op(&plls[MAINPLL]);

    clk_mux_sel = (DRV_Reg32(CLK_MUX_SEL) >> 5) & 0x7;

    switch (clk_mux_sel)
    {
    case 2:
        output_freq /= 10; // 132600;
        break;
    case 4:
        output_freq /= 12; // 133250;
        break;
    default:
        output_freq = 26000;
        break;
    }

#endif // defined(CONFIG_CLKMGR_EMULATION)

    EXIT_FUNC(FUNC_LV_API);

    return output_freq; // KHz
}
EXPORT_SYMBOL(mt_get_bus_freq);

unsigned int mt_get_cpu_freq(void)
{
    unsigned int output_freq, clk_mux_sel;

    ENTER_FUNC(FUNC_LV_API);


#if defined(CONFIG_CLKMGR_EMULATION)

    output_freq = 1000 * 1000;

#else

#if !defined(INFRACFG_AO_BASE)
#define INFRACFG_AO_BASE        INFRA_SYS_CFG_AO_BASE
#endif

#if !defined(TOP_CKMUXSEL)
#define TOP_CKMUXSEL            (INFRA_SYS_CFG_AO_BASE + 0x0000)
#endif

#if !defined(TOP_CKDIV1)
#define TOP_CKDIV1              (INFRA_SYS_CFG_AO_BASE + 0x0008)
#endif

    clk_mux_sel = DRV_Reg32(TOP_CKMUXSEL) & BITMASK(3:2);

    switch (clk_mux_sel)
    {
    default:
    case BITS(3:2, 0):
        output_freq = 26000;
        break;
    case BITS(3:2, 1):
        output_freq = pll_freq_calc_op(&plls[ARMPLL]);
        break;
    case BITS(3:2, 2):
        output_freq = pll_freq_calc_op(&plls[UNIVPLL]);
        break;
    case BITS(3:2, 3):
        output_freq = pll_freq_calc_op(&plls[MAINPLL]) / 2;
        break;
    }

    clk_mux_sel = DRV_Reg32(TOP_CKDIV1) & BITMASK(4:0);

    switch (clk_mux_sel)
    {
    case BITS(4:0, 9):
        output_freq = output_freq * 3 / 4;
        break;
    case BITS(4:0, 10):
        output_freq = output_freq * 2 / 4;
        break;
    case BITS(4:0, 11):
        output_freq = output_freq * 1 / 4;
        break;
    case BITS(4:0, 17):
        output_freq = output_freq * 4 / 5;
        break;
    case BITS(4:0, 18):
        output_freq = output_freq * 3 / 5;
        break;
    case BITS(4:0, 19):
        output_freq = output_freq * 2 / 5;
        break;
    case BITS(4:0, 20):
        output_freq = output_freq * 1 / 5;
        break;
    case BITS(4:0, 25):
        output_freq = output_freq * 5 / 6;
        break;
    case BITS(4:0, 26):
        output_freq = output_freq * 4 / 6;
        break;
    case BITS(4:0, 27):
        output_freq = output_freq * 3 / 6;
        break;
    case BITS(4:0, 28):
        output_freq = output_freq * 2 / 6;
        break;
    case BITS(4:0, 29):
        output_freq = output_freq * 1 / 6;
        break;
    case BITS(4:0, 8):
    case BITS(4:0, 16):
    case BITS(4:0, 24):
    default:
        break;
    }

#endif // defined(CONFIG_CLKMGR_EMULATION)

    EXIT_FUNC(FUNC_LV_API);

    return (get_ptp_level() == PTP_LEVEL_0 && output_freq > 1000 * 1000) ? 1000 * 1000 : output_freq; // KHz
}
EXPORT_SYMBOL(mt_get_cpu_freq);

//
// INIT related
//
static void dump_clk_info(void);

#if 0	/* XXX: for bring up */
static void subsys_all_force_on(void)
{
#if 1
    struct subsys *sys;
    int i;

    for (i = 0; i < NR_SYSS; i++)
    {
        sys = &syss[i];

        if (PWR_DOWN == sys->ops->get_state(sys))
        {
            spm_mtcmos_ctrl_general_locked(sys, STA_POWER_ON);
        }
    }
#else
    if (test_spm_gpu_power_on())
    {
        spm_mtcmos_ctrl_mfg(STA_POWER_ON);
    }
    else
    {
        clk_warn("[%s]: not force to turn on MFG\n", __func__);
    }

    spm_mtcmos_ctrl_vdec(STA_POWER_ON);
    spm_mtcmos_ctrl_venc(STA_POWER_ON);
#endif
}

static void cg_all_force_on(void)
{
    ENTER_FUNC(FUNC_LV_BODY);

    clk_writel(SET_MPLL_FREDIV_EN, CG_MPLL_MASK);
    clk_writel(SET_UPLL_FREDIV_EN, CG_UPLL_MASK);
    clk_writel(CLR_CLK_GATING_CTRL0, (CG_CTRL0_MASK & (~CG_CTRL0_EN_MASK)));
    // clk_writel(SET_CLK_GATING_CTRL0, CG_CTRL0_EN_MASK); // TODO: feature enable?
    clk_writel(CLR_CLK_GATING_CTRL1, CG_CTRL1_MASK);
    clk_writel(MMSYS_CG_CLR0, CG_MMSYS0_MASK);
    clk_writel(MMSYS_CG_CLR1, CG_MMSYS1_MASK);
    clk_writel(MFG_CG_CLR, CG_MFG_MASK);
    clk_writel(AUDIO_TOP_CON0, CG_AUDIO_MASK);

    EXIT_FUNC(FUNC_LV_BODY);
}
#endif	/* XXX: for bring up */

static void mt_subsys_init(void)
{
    int i;
    struct subsys *sys;

    ENTER_FUNC(FUNC_LV_API);

#if 0
    if (test_spm_gpu_power_on())   // XXX: PRESET (mfg force on)
    {
        sys = id_to_sys(SYS_MFG);
        sys->default_sta = PWR_ON;
    }
    else
    {
        clk_warn("[%s]: not force to turn on MFG\n", __func__);
    }
#endif

    for (i = 0; i < NR_SYSS; i++)
    {
        sys = &syss[i];
        sys->state = sys->ops->get_state(sys);

        if (sys->state != sys->default_sta) // XXX: state not match and use default_state (sync with cg_clk[]?)
        {
            clk_info("[%s]%s, change state: (%u->%u)\n", __func__,
                     sys->name, sys->state, sys->default_sta);

            if (sys->default_sta == PWR_DOWN)
            {
                disable_subsys_locked(sys, 1);
            }
            else
            {
                enable_subsys_locked(sys);
            }
        }
    }

    EXIT_FUNC(FUNC_LV_API);
}

static void mt_plls_init(void)
{
    int i;
    struct pll *pll;

    ENTER_FUNC(FUNC_LV_API);

    for (i = 0; i < NR_PLLS; i++)
    {
        pll = &plls[i];
        pll->state = pll->ops->get_state(pll);
    }

    EXIT_FUNC(FUNC_LV_API);
}

#if 0	// TODO: FIXME
static void mt_plls_enable_hp(void)
{
    int i;
    struct pll *pll;

    ENTER_FUNC(FUNC_LV_API);

    for (i = 0; i < NR_PLLS; i++)
    {
        pll = &plls[i];

        if (pll->ops->hp_enable && pll->feat & HAVE_PLL_HP)
        {
            pll->ops->hp_enable(pll);
        }
    }

    EXIT_FUNC(FUNC_LV_API);
}
#endif	// TODO: FIXME

static void mt_clks_init(void)
{
    int i;
    struct cg_grp *grp;
    struct cg_clk *clk;
    struct clkmux *clkmux;

    ENTER_FUNC(FUNC_LV_API);

    // TODO: preset

    clk_writel(CLR_CLK_GATING_CTRL0, MFG_MM_SW_CG_BIT); // XXX: patch for MFG clock access @ init mode

#if 0
    clk_writel(CLR_CLK_GATING_CTRL0, (CG_CTRL0_MASK & (~CG_CTRL0_EN_MASK)));
    // clk_writel(SET_CLK_GATING_CTRL0, CG_CTRL0_EN_MASK); // TODO: feature enable?
    clk_writel(CLR_CLK_GATING_CTRL1, CG_CTRL1_MASK);
    clk_writel(MMSYS_CG_CLR0, CG_MMSYS0_MASK);
    clk_writel(MMSYS_CG_CLR1, CG_MMSYS1_MASK);
    clk_writel(MFG_CG_CLR, CG_MFG_MASK);
    clk_writel(AUDIO_TOP_CON0, CG_AUDIO_MASK);
#else
    clk_writel(SET_CLK_GATING_CTRL1, SEJ_SW_CG_BIT);
#endif

    clk_writel(INFRABUS_DCMCTL1, clk_readl(INFRABUS_DCMCTL1) | BIT(29)); // XXX: for SPM

    // TODO: patch clk_ops if necessary

    // TODO: set parent (i.e. pll)

    // init CG_CLK
    for (i = 0; i < NR_CLKS; i++)
    {
        clk = &clks[i];
        grp = clk->grp;

        if (NULL != grp) // XXX: CG_CLK always map one of CL_GRP
        {
            grp->mask |= clk->mask; // XXX: init cg_grp mask by cg_clk mask
            clk->state = clk->ops->get_state(clk);
            clk->cnt = (PWR_ON == clk->state) ? 1 : 0;

            // update clk src for auto sel
            switch(clk->src)
            {
            case MT_CG_SRC_DBI:
                clk->src = is_ddr3() ? MT_CG_MPLL_D10 : MT_CG_MPLL_D12;
                break;
            case MT_CG_SRC_SMI:
                clk->src = is_ddr3() ? MT_CG_MPLL_D5 : MT_CG_MPLL_D6;
                break;
            default:
                break;
            }
        }
        else
        {
            BUG(); // XXX: CG_CLK always map one of CL_GRP
        }
    }

    // init CG_GRP
    for (i = 0; i < NR_GRPS; i++)
    {
        grp = &grps[i];
        grp->state = grp->ops->get_state(grp);
    }

    // init CLKMUX (link clk src + clkmux + clk drain)
    for (i = 0; i < NR_CLKMUXS; i++)
    {
        cg_clk_id src_id;

        clkmux = &muxs[i];

        src_id = clkmux->ops->get(clkmux);

        // clk_info("clkmux[%d] %d -> %d\n", i, src_id, clkmux->drain); // <-XXX

        clk = id_to_clk(clkmux->drain); // clk (drain)

        if (NULL != clk) // clk (drain)
        {
            clk->src = src_id; // clk (drain)

            if (0) // (PWR_ON == clk->ops->get_state(clk)) // clk (drain) // XXX: would enable @ below (i.e. init CG_CLK again)
            {
                clk = id_to_clk(src_id); // clk (source)

                if (NULL != clk) // clk (source)
                {
                    enable_clock_locked(clk); // clk (source)
                }
            }
        }
        else // wo drain
        {
            clk = id_to_clk(src_id); // clk (source)

            if (NULL != clk) // clk (source)
            {
                enable_clock_locked(clk); // clk (source) // XXX: tricky to keep CG source for CLKMUX wo drain (e.g. EMI & AXI BUS)
            }
        }
    }

    // init CG_CLK again (construct clock tree dependency)
    for (i = 0; i < NR_CLKS; i++)
    {
        clk = &clks[i];

        if (PWR_ON == clk->state)
        {
            BUG_ON((MT_CG_INVALID != clk->src) && (NULL != clk->parent));

            if (MT_CG_INVALID != clk->src)
            {
                clk = id_to_clk(clk->src); // clk (source)

                if (NULL != clk) // clk (source)
                {
                    enable_clock_locked(clk); // clk (source)
                }
            }
            else if (NULL != clk->parent)
            {
                enable_pll_locked(clk->parent);
            }
        }
    }

    for (i = TO_CG_GRP_ID(CG_UPLL); i >= FROM_CG_GRP_ID(CG_MPLL); i--) // XXX: tricky (down to up)
    {
        clk = &clks[i];
        clk->cnt--;

        if (0 == clk->cnt)
        {
            clk->state = PWR_DOWN;
            clk->ops->disable(clk);

            if (MT_CG_INVALID != clk->src)
            {
                clk = id_to_clk(clk->src); // clk (source)

                if (NULL != clk) // clk (source)
                {
                    disable_clock_locked(clk); // clk (source)
                }
            }
            else if (NULL != clk->parent)
            {
                disable_pll_locked(clk->parent);
            }
        }
    }

    disable_clock_locked(&clks[MT_CG_MFG_MM_SW_CG]); // XXX: patch for MFG clock access @ init mode

    EXIT_FUNC(FUNC_LV_API);
}

#if 0
static void mt_md1_cg_init(void)
{
    clk_clrl(PERI_PDN_MD_MASK, 1U << 0);
    clk_writel(PERI_PDN0_MD1_SET, 0xFFFFFFFF);
}
#endif

int mt_clkmgr_init(void)
{
    ENTER_FUNC(FUNC_LV_API);

#if !defined(CONFIG_CLKMGR_EMULATION)

    BUG_ON(initialized);

    // XXX: force MT_CG_SPM_SW_CG on for mt_subsys_init()
    {
        struct cg_clk *cg_clk = id_to_clk(MT_CG_SPM_SW_CG);
        cg_clk->ops->enable(cg_clk);
    }

    mt_subsys_init();
    // cg_all_force_on(); // XXX: for bring up

    mt_clks_init();
    mt_plls_init();

#if defined(CONFIG_CLKMGR_SHOWLOG)
    dump_clk_info();
#endif

#endif // !defined(CONFIG_CLKMGR_EMULATION)

    initialized = 1;

    mt_freqhop_init();
#if 0   // TODO: FIXME
    mt_plls_enable_hp();
#endif  // TODO: FIXME

    EXIT_FUNC(FUNC_LV_API);
    return 0;
}

#ifdef CONFIG_MTK_MMC
extern void msdc_clk_status(int *status);
#else
void msdc_clk_status(int *status)
{
    *status = 0;
}
#endif

//
// IDLE related
//
bool clkmgr_idle_can_enter(unsigned int *condition_mask, unsigned int *block_mask)
{
    int i;
    unsigned int sd_mask = 0;
    unsigned int cg_mask = 0;
    bool cg_fail = 0;

    ENTER_FUNC(FUNC_LV_API);

    msdc_clk_status(&sd_mask);

    if (sd_mask)
    {
        // block_mask[CG_PERI0] |= sd_mask; // TODO: review it latter
        EXIT_FUNC(FUNC_LV_API);
        return false;
    }

    for (i = 0; i < NR_GRPS; i++)
    {
        cg_mask = grps[i].state & condition_mask[i];

        if (cg_mask)
        {
            block_mask[i] |= cg_mask;
            EXIT_FUNC(FUNC_LV_API);
            cg_fail = 1;
        }
    }

    if(cg_fail)
        return false;

    EXIT_FUNC(FUNC_LV_API);
    return true;
}

//
// Golden setting
//
#if defined(CONFIG_MT_ENG_BUILD)

typedef enum
{
    MODE_NORMAL,
    MODE_COMPARE,
    MODE_APPLY,
    MODE_COLOR,
    MODE_DIFF,
} print_mode;

struct golden_setting
{
    unsigned int addr;
    unsigned int mask;
    unsigned int golden_val;
};

struct snapshot
{
    const char *func;
    unsigned int line;
    unsigned int reg_val[1]; // XXX: actually variable length
};

struct golden
{
    unsigned int is_golden_log;

    print_mode mode;

    char func[64]; // TODO: check the size is OK or not
    unsigned int line;

    unsigned int *buf;
    unsigned int buf_size;

    struct golden_setting *buf_golden_setting;
    unsigned int nr_golden_setting;
    unsigned int max_nr_golden_setting;

    struct snapshot *buf_snapshot;
    unsigned int max_nr_snapshot;
    unsigned int snapshot_head;
    unsigned int snapshot_tail;
};

#define SIZEOF_SNAPSHOT(g) (sizeof(struct snapshot) + sizeof(unsigned int) * (g->nr_golden_setting - 1))

static struct golden _golden;

static void _golden_setting_enable(struct golden *g)
{
    if (NULL != g)
    {
        g->buf_snapshot = (struct snapshot *) & (g->buf_golden_setting[g->nr_golden_setting]);
        g->max_nr_snapshot = (g->buf_size - sizeof(struct golden_setting) * g->nr_golden_setting) / SIZEOF_SNAPSHOT(g);
        g->snapshot_head = 0;
        g->snapshot_tail = 0; // TODO: check it

        g->is_golden_log = TRUE;
    }
}

static void _golden_setting_disable(struct golden *g)
{
    if (NULL != g)
    {
        g->is_golden_log = FALSE;

        g->func[0] = '\0';

        g->buf_golden_setting = (struct golden_setting *)g->buf;
        g->nr_golden_setting = 0;
        g->max_nr_golden_setting = g->buf_size / 3 / sizeof(struct golden_setting); // TODO: refine it
    }
}

static void _golden_setting_set_mode(struct golden *g, print_mode mode)
{
    g->mode = mode;
}

static void _golden_setting_init(struct golden *g, unsigned int *buf, unsigned int buf_size)
{
    if (   NULL != g
        && NULL != buf
       )
    {
        g->mode = MODE_NORMAL;

        g->buf = buf;
        g->buf_size = buf_size;

        _golden_setting_disable(g);
    }
}

static void _golden_setting_add(struct golden *g, unsigned int addr, unsigned int mask, unsigned golden_val)
{
    if (   NULL != g
        && FALSE == g->is_golden_log
        && g->nr_golden_setting < g->max_nr_golden_setting
       )
    {
        g->buf_golden_setting[g->nr_golden_setting].addr = addr;
        g->buf_golden_setting[g->nr_golden_setting].mask = mask;
        g->buf_golden_setting[g->nr_golden_setting].golden_val = golden_val;

        g->nr_golden_setting++;
    }
}

static bool _is_pmic_addr(unsigned int addr)
{
    return (addr & 0xF0000000) ? FALSE : TRUE;
}

static void _golden_write_reg(unsigned int addr, unsigned int mask, unsigned int reg_val)
{
    if (_is_pmic_addr(addr))
    {
        pmic_config_interface(addr, reg_val, mask, 0x0);
    }
    else
    {
        *((unsigned int *)IO_PHYS_TO_VIRT(addr)) = (*((unsigned int *)IO_PHYS_TO_VIRT(addr)) & ~mask) | (reg_val & mask);
    }
}

static unsigned int _golden_read_reg(unsigned int addr)
{
    unsigned int reg_val;

    if (_is_pmic_addr(addr))
    {
        pmic_read_interface(addr, &reg_val, 0xFFFFFFFF, 0x0);
    }
    else
    {
        reg_val = *((unsigned int *)IO_PHYS_TO_VIRT(addr));
    }

    return reg_val;
}

static int _is_snapshot_full(struct golden *g)
{
    if (   g->snapshot_head + 1 == g->snapshot_tail
        || g->snapshot_head + 1 == g->snapshot_tail + g->max_nr_snapshot
       )
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

static int _is_snapshot_empty(struct golden *g)
{
    if (g->snapshot_head == g->snapshot_tail)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

static struct snapshot *_snapshot_produce(struct golden *g)
{
    if (   NULL != g
        && !_is_snapshot_full(g)
       )
    {
        int idx = g->snapshot_head++;

        if (g->snapshot_head == g->max_nr_snapshot)
        {
            g->snapshot_head = 0;
        }

        return (struct snapshot *)((int)(g->buf_snapshot) + SIZEOF_SNAPSHOT(g) * idx);
    }
    else
    {
        return NULL;
    }
}

static struct snapshot *_snapshot_consume(struct golden *g)
{
    if (   NULL != g
        && !_is_snapshot_empty(g)
       )
    {
        int idx = g->snapshot_tail++;

        if (g->snapshot_tail == g->max_nr_snapshot)
        {
            g->snapshot_tail = 0;
        }

        return (struct snapshot *)((int)(g->buf_snapshot) + SIZEOF_SNAPSHOT(g) * idx);
    }
    else
    {
        return NULL;
    }
}

static int _snapshot_golden_setting(struct golden *g, const char *func, const unsigned int line)
{
    struct snapshot *snapshot;
    int i;

    if (   NULL != g
        && TRUE == g->is_golden_log
        && (g->func[0] == '\0' || (!strcmp(g->func, func) && ((g->line == line) || (g->line == 0))))
        && NULL != (snapshot = _snapshot_produce(g))
       )
    {
        snapshot->func = func;
        snapshot->line = line;

        for (i = 0; i < g->nr_golden_setting; i++)
        {
            if (MODE_APPLY == _golden.mode)
            {
                _golden_write_reg(g->buf_golden_setting[i].addr,
                                  g->buf_golden_setting[i].mask,
                                  g->buf_golden_setting[i].golden_val
                                  );
            }

            snapshot->reg_val[i] = _golden_read_reg(g->buf_golden_setting[i].addr);
        }

        return 0;
    }
    else
    {
        // printf("[Err]: buffer full or not enabled\n");

        return -1;
    }
}

#endif /* CONFIG_MT_ENG_BUILD */

int snapshot_golden_setting(const char *func, const unsigned int line)
{
#if defined(CONFIG_MT_ENG_BUILD)
    return _snapshot_golden_setting(&_golden, func, line);
#else
    return -1;
#endif
}

#if defined(CONFIG_MT_ENG_BUILD)
static int _parse_mask_val(char *buf, unsigned int *mask, unsigned int *golden_val)
{
    unsigned int i, bit_shift;
    unsigned int mask_result;
    unsigned int golden_val_result;

    for (i = 0,
         bit_shift = 1 << 31,
         mask_result = 0,
         golden_val_result = 0;
         bit_shift > 0;
         )
    {
        switch (buf[i])
        {
        case '1':
            golden_val_result += bit_shift;
        case '0':
            mask_result += bit_shift;
        case 'x':
        case 'X':
            bit_shift >>= 1;
        case '_':
            break;
        default:
            return -1;
        }
        i++;
    }

    *mask = mask_result;
    *golden_val = golden_val_result;

    return 0;
}

static char * _gen_mask_str(const unsigned int mask, const unsigned int reg_val)
{
    static char _mask_str[] = "0bxxxx_xxxx_xxxx_xxxx_xxxx_xxxx_xxxx_xxxx";
    unsigned int i, bit_shift;

    for (i = 2,
         bit_shift = 1 << 31;
         bit_shift > 0;
         )
    {
        switch(_mask_str[i])
        {
        case '_':
            break;
        default:
            if (0 == (mask & bit_shift))
            {
                _mask_str[i] = 'x';
            }
            else if (0 == (reg_val & bit_shift))
            {
                _mask_str[i] = '0';
            }
            else
            {
                _mask_str[i] = '1';
            }
        case '\0':
            bit_shift >>= 1;
            break;
        }

        i++;
    }

    return _mask_str;
}

static char * _gen_diff_str(const unsigned int mask, const unsigned int golden_val, const unsigned int reg_val)
{
    static char _diff_str[] = "0b    _    _    _    _    _    _    _    ";
    unsigned int i, bit_shift;

    for (i = 2,
         bit_shift = 1 << 31;
         bit_shift > 0;
         )
    {
        switch(_diff_str[i])
        {
        case '_':
            break;
        default:
            if (0 != ((golden_val ^ reg_val) & mask & bit_shift))
            {
                _diff_str[i] = '^';
            }
            else
            {
                _diff_str[i] = ' ';
            }
        case '\0':
            bit_shift >>= 1;
            break;
        }

        i++;
    }

    return _diff_str;
}

static char * _gen_color_str(const unsigned int mask, const unsigned int golden_val, const unsigned int reg_val)
{
#define FC "\e[41m"
#define EC "\e[m"
#define XXXX FC "x" EC FC "x" EC FC "x" EC FC "x" EC
    static char _clr_str[] = "0b"XXXX"_"XXXX"_"XXXX"_"XXXX"_"XXXX"_"XXXX"_"XXXX"_"XXXX;
    unsigned int i, bit_shift;

    for (i = 2,
         bit_shift = 1 << 31;
         bit_shift > 0;
         )
    {
        switch(_clr_str[i])
        {
        case '_':
            break;

        default:
            if (0 != ((golden_val ^ reg_val) & mask & bit_shift))
            {
                _clr_str[i+3] = '1';
            }
            else
            {
                _clr_str[i+3] = '0';
            }

            if (0 == (mask & bit_shift))
            {
                _clr_str[i+5] = 'x';
            }
            else if (0 == (reg_val & bit_shift))
            {
                _clr_str[i+5] = '0';
            }
            else
            {
                _clr_str[i+5] = '1';
            }

            i += strlen(EC) + strlen(FC); // XXX: -1 is for '\0' (sizeof)

        case '\0':
            bit_shift >>= 1;
            break;
        }

        i++;
    }

    return _clr_str;

#undef FC
#undef EC
#undef XXXX
}
#endif /* CONFIG_MT_ENG_BUILD */

//
// DEBUG related
//
void dump_clk_info_by_id(cg_clk_id id)
{
    struct cg_clk *clk = id_to_clk(id);

    if (NULL != clk)
    {
        printk(KERN_EMERG "[%d,\t%d,\t%s,\t"HEX_FMT",\t%s,\t%s,\t%s]\n",
               id,
               clk->cnt,
               (PWR_ON == clk->state) ? "ON" : "OFF",
               clk->mask,
               (clk->grp) ? clk->grp->name : "NULL",
               clk->name,
               (clk->src < NR_CLKS) ? clks[clk->src].name : "MT_CG_INVALID"
               );
    }

    return;
}

static void dump_clk_info(void)
{
    int i;

    for (i = 0; i < NR_CLKS; i++)
    {
        dump_clk_info_by_id(i);
    }

#if 0   // XXX: dump for control table
    for (i = 0; i < NR_CLKS; i++)
    {
        struct cg_clk *clk;

        clk = &clks[i];

        clk_info("[%s,\t"HEX_FMT",\t"HEX_FMT",\t"HEX_FMT",\t%d,\t%s]\n",
                 clk->name,
                 (clk->grp) ? clk->grp->set_addr : 0,
                 (clk->grp) ? clk->grp->clr_addr : 0,
                 (clk->grp) ? clk->grp->sta_addr : 0,
                 LSB32(clk->mask),
                 (clk->ops->enable == general_en_cg_clk_enable_op) ? "EN" : "CG"
                 );
    }
#endif  // XXX: dump for control table
}

static int clk_test_read(char *page, char **start, off_t off,
                         int count, int *eof, void *data)
{
    char *p = page;
    int len = 0;

    int i;
    int cnt;
    unsigned int value[2];
    const char *name;

    ENTER_FUNC(FUNC_LV_BODY);

    p += sprintf(p, "********** clk register dump *********\n");

    for (i = 0; i < NR_GRPS; i++)
    {
        name = grp_get_name(i);
        //p += sprintf(p, "[%d][%s] = "HEX_FMT", "HEX_FMT"\n", i, name, grps[i].ops->get_state());
        //p += sprintf(p, "[%d][%s]="HEX_FMT"\n", i, name, grps[i].state);
        cnt = grp_dump_regs(i, value);

        if (cnt == 1)
        {
            p += sprintf(p, "[%02d][%-8s] =\t["HEX_FMT"]\n", i, name, value[0]);
        }
        else
        {
            p += sprintf(p, "[%02d][%-8s] =\t["HEX_FMT"]["HEX_FMT"]\n", i, name, value[0], value[1]);
        }
    }

    #if 0   // TODO: show MD status
    p += sprintf(p, "[PERI_PDN_MD_MASK]="HEX_FMT"\n", clk_readl(PERI_PDN_MD_MASK));
    p += sprintf(p, "[PERI_PDN0_MD1_STA]="HEX_FMT"\n", clk_readl(PERI_PDN0_MD1_STA));
    p += sprintf(p, "[PERI_PDN0_MD2_STA]="HEX_FMT"\n", clk_readl(PERI_PDN0_MD2_STA));
    #endif

    p += sprintf(p, "[CLK_MUX_SEL]="HEX_FMT"\n", clk_readl(CLK_MUX_SEL));

    p += sprintf(p, "\n********** clk_test help *********\n");
    p += sprintf(p, "clkmux     : echo clkmux mux_id cg_id [mod_name] > /proc/clkmgr/clk_test\n");
    p += sprintf(p, "enable  clk: echo enable  cg_id [mod_name] > /proc/clkmgr/clk_test\n");
    p += sprintf(p, "disable clk: echo disable cg_id [mod_name] > /proc/clkmgr/clk_test\n");
    p += sprintf(p, "dump clk   : echo dump cg_id > /proc/clkmgr/clk_test\n");
    p += sprintf(p, "dump clk   : echo dump MT_CG_xxx > /proc/clkmgr/clk_test\n");

    *start = page + off;

    len = p - page;

    if (len > off)
    {
        len -= off;
    }
    else
    {
        len = 0;
    }

    *eof = 1;

    dump_clk_info();

    EXIT_FUNC(FUNC_LV_BODY);
    return len < count ? len  : count;
}

static int clk_test_write(struct file *file, const char *buffer,
                          unsigned long count, void *data)
{
    char desc[32];
    int len = 0;

    char cmd[10];
    char mod_name[10];
    int mux_id;
    int cg_id;

    ENTER_FUNC(FUNC_LV_BODY);

    len = (count < (sizeof(desc) - 1)) ? count : (sizeof(desc) - 1);

    if (copy_from_user(desc, buffer, len))
    {
        EXIT_FUNC(FUNC_LV_BODY);
        return 0;
    }

    desc[len] = '\0';

    if  (sscanf(desc, "%s %d %d %s", cmd, &mux_id, &cg_id, mod_name) == 4)
    {
        if (!strcmp(cmd, "clkmux"))
        {
            clkmux_sel(mux_id, cg_id, mod_name);
        }
    }
    else if (sscanf(desc, "%s %d %s", cmd, &cg_id, mod_name) == 3)
    {
        if (!strcmp(cmd, "enable"))
        {
            enable_clock(cg_id, mod_name);
        }
        else if (!strcmp(cmd, "disable"))
        {
            disable_clock(cg_id, mod_name);
        }
    }
    else if (sscanf(desc, "%s %d", cmd, &cg_id) == 2)
    {
        if (!strcmp(cmd, "enable"))
        {
            enable_clock(cg_id, "pll_test");
        }
        else if (!strcmp(cmd, "disable"))
        {
            disable_clock(cg_id, "pll_test");
        }
        else if (!strcmp(cmd, "dump"))
        {
            dump_clk_info_by_id(cg_id);
        }
    }
    else if (sscanf(desc, "%s %s", cmd, mod_name) == 2)
    {
        if (!strcmp(cmd, "dump"))
        {
            int i;

            for (i = 0; i < ARRAY_SIZE(clks); i++)
            {
                if (0 == strcmp(clks[i].name, mod_name))
                {
                    dump_clk_info_by_id(i);
                    break;
                }
            }
        }
    }

    EXIT_FUNC(FUNC_LV_BODY);
    return count;
}


static int pll_test_read(char *page, char **start, off_t off,
                         int count, int *eof, void *data)
{
    char *p = page;
    int len = 0;

    int i, j;
    int cnt;
    unsigned int value[3];
    const char *name;

    ENTER_FUNC(FUNC_LV_BODY);

    p += sprintf(p, "********** pll register dump *********\n");

    for (i = 0; i < NR_PLLS; i++)
    {
        name = pll_get_name(i);
        cnt = pll_dump_regs(i, value);

        for (j = 0; j < cnt; j++)
        {
            p += sprintf(p, "[%d][%-7s reg%d]=["HEX_FMT"]\n", i, name, j, value[j]);
        }
    }

    p += sprintf(p, "\n********** pll_test help *********\n");
    p += sprintf(p, "enable  pll: echo enable  id [mod_name] > /proc/clkmgr/pll_test\n");
    p += sprintf(p, "disable pll: echo disable id [mod_name] > /proc/clkmgr/pll_test\n");

    *start = page + off;

    len = p - page;

    if (len > off)
    {
        len -= off;
    }
    else
    {
        len = 0;
    }

    *eof = 1;

    EXIT_FUNC(FUNC_LV_BODY);
    return len < count ? len  : count;
}

static int pll_test_write(struct file *file, const char *buffer,
                          unsigned long count, void *data)
{
    char desc[32];
    int len = 0;

    char cmd[10];
    char mod_name[10];
    int id;

    ENTER_FUNC(FUNC_LV_BODY);

    len = (count < (sizeof(desc) - 1)) ? count : (sizeof(desc) - 1);

    if (copy_from_user(desc, buffer, len))
    {
        EXIT_FUNC(FUNC_LV_BODY);
        return 0;
    }

    desc[len] = '\0';

    if (sscanf(desc, "%s %d %s", cmd, &id, mod_name) == 3)
    {
        if (!strcmp(cmd, "enable"))
        {
            enable_pll(id, mod_name);
        }
        else if (!strcmp(cmd, "disable"))
        {
            disable_pll(id, mod_name);
        }
    }
    else if (sscanf(desc, "%s %d", cmd, &id) == 2)
    {
        if (!strcmp(cmd, "enable"))
        {
            enable_pll(id, "pll_test");
        }
        else if (!strcmp(cmd, "disable"))
        {
            disable_pll(id, "pll_test");
        }
    }

    EXIT_FUNC(FUNC_LV_BODY);
    return count;
}

static int pll_fsel_read(char *page, char **start, off_t off,
                         int count, int *eof, void *data)
{
    char *p = page;
    int len = 0;

    int i;
    int cnt;
    unsigned int value[3];
    const char *name;

    ENTER_FUNC(FUNC_LV_BODY);

    for (i = 0; i < NR_PLLS; i++)
    {
        name = pll_get_name(i);

        if (pll_is_on(i))
        {
            cnt = pll_dump_regs(i, value);

            if (cnt >= 2)
            {
                p += sprintf(p, "[%d][%-7s]=["HEX_FMT" "HEX_FMT"]\n", i, name, value[0], value[1]);
            }
            else
            {
                p += sprintf(p, "[%d][%-7s]=["HEX_FMT"]\n", i, name, value[0]);
            }
        }
        else
        {
            p += sprintf(p, "[%d][%-7s]=[-1]\n", i, name);
        }
    }

    p += sprintf(p, "\n********** pll_fsel help *********\n");
    p += sprintf(p, "adjust pll frequency:  echo id freq > /proc/clkmgr/pll_fsel\n");

    *start = page + off;

    len = p - page;

    if (len > off)
    {
        len -= off;
    }
    else
    {
        len = 0;
    }

    *eof = 1;

    EXIT_FUNC(FUNC_LV_BODY);
    return len < count ? len  : count;
}

static int pll_fsel_write(struct file *file, const char *buffer,
                          unsigned long count, void *data)
{
    char desc[32];
    int len = 0;

    int id;
    unsigned int value;

    ENTER_FUNC(FUNC_LV_BODY);

    len = (count < (sizeof(desc) - 1)) ? count : (sizeof(desc) - 1);

    if (copy_from_user(desc, buffer, len))
    {
        EXIT_FUNC(FUNC_LV_BODY);
        return 0;
    }

    desc[len] = '\0';

    if (sscanf(desc, "%d %x", &id, &value) == 2)
    {
        pll_fsel(id, value);
    }

    EXIT_FUNC(FUNC_LV_BODY);
    return count;
}


static int subsys_test_read(char *page, char **start, off_t off,
                            int count, int *eof, void *data)
{
    char *p = page;
    int len = 0;

    int i;
    int state;
    unsigned int value, sta, sta_s;
    const char *name;

    ENTER_FUNC(FUNC_LV_BODY);

    sta = clk_readl(SPM_PWR_STATUS);
    sta_s = clk_readl(SPM_PWR_STATUS_S);

    p += sprintf(p, "********** subsys register dump *********\n");

    for (i = 0; i < NR_SYSS; i++)
    {
        name = subsys_get_name(i);
        state = subsys_is_on(i);
        subsys_dump_regs(i, &value);
        p += sprintf(p, "[%d][%-7s]=["HEX_FMT"], state(%u)\n", i, name, value, state);
    }

    p += sprintf(p, "SPM_PWR_STATUS="HEX_FMT", SPM_PWR_STATUS_S="HEX_FMT"\n", sta, sta_s);

    p += sprintf(p, "\n********** subsys_test help *********\n");
    p += sprintf(p, "enable subsys:  echo enable id > /proc/clkmgr/subsys_test\n");
    p += sprintf(p, "disable subsys: echo disable id [force_off] > /proc/clkmgr/subsys_test\n");

    *start = page + off;

    len = p - page;

    if (len > off)
    {
        len -= off;
    }
    else
    {
        len = 0;
    }

    *eof = 1;

    EXIT_FUNC(FUNC_LV_BODY);
    return len < count ? len  : count;
}

static int subsys_test_write(struct file *file, const char *buffer,
                             unsigned long count, void *data)
{
    char desc[32];
    int len = 0;

    char cmd[10];
    subsys_id id;
    int force_off;
    int err = 0;

    ENTER_FUNC(FUNC_LV_BODY);

    len = (count < (sizeof(desc) - 1)) ? count : (sizeof(desc) - 1);

    if (copy_from_user(desc, buffer, len))
    {
        EXIT_FUNC(FUNC_LV_BODY);
        return 0;
    }

    desc[len] = '\0';

    if (sscanf(desc, "%s %d %d", cmd, (int *)&id, &force_off) == 3)
    {
        if (!strcmp(cmd, "disable"))
        {
            err = disable_subsys_force(id, "test");
        }
    }
    else if (sscanf(desc, "%s %d", cmd, (int *)&id) == 2)
    {
        if (!strcmp(cmd, "enable"))
        {
            err = enable_subsys(id, "test");
        }
        else if (!strcmp(cmd, "disable"))
        {
            err = disable_subsys(id, "test");
        }
    }

    clk_info("[%s]%s subsys %d: result is %d\n", __func__, cmd, id, err);

    EXIT_FUNC(FUNC_LV_BODY);
    return count;
}

static int udelay_test_read(char *page, char **start, off_t off,
                            int count, int *eof, void *data)
{
    char *p = page;
    int len = 0;

    ENTER_FUNC(FUNC_LV_BODY);

    p += sprintf(p, "\n********** udelay_test help *********\n");
    p += sprintf(p, "test udelay:  echo delay > /proc/clkmgr/udelay_test\n");

    *start = page + off;

    len = p - page;

    if (len > off)
    {
        len -= off;
    }
    else
    {
        len = 0;
    }

    *eof = 1;

    EXIT_FUNC(FUNC_LV_BODY);
    return len < count ? len  : count;
}

static int udelay_test_write(struct file *file, const char *buffer,
                             unsigned long count, void *data)
{
    char desc[32];
    int len = 0;

    unsigned int delay;
    unsigned int pre, pos;

    ENTER_FUNC(FUNC_LV_BODY);

    len = (count < (sizeof(desc) - 1)) ? count : (sizeof(desc) - 1);

    if (copy_from_user(desc, buffer, len))
    {
        EXIT_FUNC(FUNC_LV_BODY);
        return 0;
    }

    desc[len] = '\0';

    if (sscanf(desc, "%u", &delay) == 1)
    {
        pre = 0; // clk_readl(0xF0008028); // TODO: FIXME: BAD CODING STYLE, ??? global timer ???
        udelay(delay);
        pos = 0; // clk_readl(0xF0008028); // TODO: FIXME: BAD CODING STYLE, ??? global timer ???
        clk_info("udelay(%u) test: pre="HEX_FMT", pos="HEX_FMT", delta=%u\n",
                 delay, pre, pos, pos - pre);
    }

    EXIT_FUNC(FUNC_LV_BODY);
    return count;
}

#if defined(CONFIG_MT_ENG_BUILD)

static int golden_test_read(char *page, char **start, off_t off,
                            int count, int *eof, void *data)
{
    static int buf_golden_setting_idx = 0;
    static off_t page_len = 0;
    static off_t used_off = 0;

    char *p = page;
    char *start_p;
    int len = 0;
    int i = 0;

    ENTER_FUNC(FUNC_LV_BODY);

    if (0 == off)
    {
        buf_golden_setting_idx = 0;
        page_len = 0;
        used_off = 0;
    }

    if (0 == page_len)
    {
        if (FALSE == _golden.is_golden_log)
        {
            if (0 == off)
            {
                for (i = 0; i < _golden.nr_golden_setting; i++)
                {
                    p += sprintf(p, ""HEX_FMT" "HEX_FMT" "HEX_FMT"\n",
                                 _golden.buf_golden_setting[i].addr,
                                 _golden.buf_golden_setting[i].mask,
                                 _golden.buf_golden_setting[i].golden_val
                                 );
                }
            }
        }

        if (0 == _golden.nr_golden_setting)
        {
            if (0 == off)
            {
                p += sprintf(p, "\n********** golden_test help *********\n");
                p += sprintf(p, "1.   disable snapshot:                  echo disable > /proc/clkmgr/golden_test\n");
                p += sprintf(p, "2.   insert golden setting (tool mode): echo 0x10000000 (addr) 0bxxxx_xxxx_xxxx_xxxx_0001_0100_1001_0100 (mask & golden value) > /proc/clkmgr/golden_test\n");
                p += sprintf(p, "(2.) insert golden setting (hex mode):  echo 0x10000000 (addr) 0xFFFF (mask) 0x1494 (golden value) > /proc/clkmgr/golden_test\n");
                p += sprintf(p, "(2.) insert golden setting (dec mode):  echo 268435456 (addr) 65535 (mask) 5268 (golden value) > /proc/clkmgr/golden_test\n");
                p += sprintf(p, "3.   set filter:                        echo filter func_name [line_num] > /proc/clkmgr/golden_test\n");
                p += sprintf(p, "(3.) disable filter:                    echo filter > /proc/clkmgr/golden_test\n");
                p += sprintf(p, "4.   enable snapshot:                   echo enable > /proc/clkmgr/golden_test\n");
                p += sprintf(p, "5.   set compare mode:                  echo compare > /proc/clkmgr/golden_test\n");
                p += sprintf(p, "(5.) set apply mode:                    echo apply > /proc/clkmgr/golden_test\n");
                p += sprintf(p, "(5.) set color mode:                    echo color > /proc/clkmgr/golden_test\n");
                p += sprintf(p, "(5.) set diff mode:                     echo color > /proc/clkmgr/golden_test\n");
                p += sprintf(p, "(5.) disable compare/apply/color mode:  echo normal > /proc/clkmgr/golden_test\n");
                p += sprintf(p, "6.   set register value (normal mode):  echo set 0x10000000 (addr) 0x13201494 (reg val) > /proc/clkmgr/golden_test\n");
                p += sprintf(p, "(6.) set register value (mask mode):    echo set 0x10000000 (addr) 0xffff (mask) 0x13201494 (reg val) > /proc/clkmgr/golden_test\n");
                p += sprintf(p, "(6.) set register value (bit mode):     echo set 0x10000000 (addr) 0 (bit num) 1 (reg val) > /proc/clkmgr/golden_test\n");
            }
        }
        else
        {
            static struct snapshot *snapshot;

            if (   0 == off // buf_golden_setting_idx
                && !strcmp(_golden.func, __FUNCTION__) && (_golden.line == 0))
            {
                snapshot_golden_setting(__FUNCTION__, 0);
            }

            while ((0 != buf_golden_setting_idx) ||( NULL != (snapshot = _snapshot_consume(&_golden))))
            {
                if (0 == buf_golden_setting_idx)
                {
                    p += sprintf(p, "@ %s():%d\n", snapshot->func, snapshot->line);
                }

		start_p = p;

                for (i = buf_golden_setting_idx, buf_golden_setting_idx = 0; i < _golden.nr_golden_setting; i++)
                {
                    start_p = p;

                    if (   MODE_NORMAL == _golden.mode
                        || (   (_golden.buf_golden_setting[i].mask & _golden.buf_golden_setting[i].golden_val)
                            != (_golden.buf_golden_setting[i].mask & snapshot->reg_val[i])
                            )
                        )
                    {
                        if (MODE_COLOR == _golden.mode)
                        {
                            p += sprintf(p, HEX_FMT"\t"HEX_FMT"\t"HEX_FMT"\t%s\n",
                                         _golden.buf_golden_setting[i].addr,
                                         _golden.buf_golden_setting[i].mask,
                                         snapshot->reg_val[i],
                                         _gen_color_str(_golden.buf_golden_setting[i].mask, _golden.buf_golden_setting[i].golden_val, snapshot->reg_val[i])
                                         );
                        }
                        else if (MODE_DIFF == _golden.mode)
                        {
                            p += sprintf(p, HEX_FMT"\t"HEX_FMT"\t"HEX_FMT"\t%s\n",
                                         _golden.buf_golden_setting[i].addr,
                                         _golden.buf_golden_setting[i].mask,
                                         snapshot->reg_val[i],
                                         _gen_mask_str(_golden.buf_golden_setting[i].mask, snapshot->reg_val[i])
                                         );

                            p += sprintf(p, HEX_FMT"\t"HEX_FMT"\t"HEX_FMT"\t%s\n",
                                         _golden.buf_golden_setting[i].addr,
                                         _golden.buf_golden_setting[i].mask,
                                         _golden.buf_golden_setting[i].golden_val,
                                         _gen_diff_str(_golden.buf_golden_setting[i].mask, _golden.buf_golden_setting[i].golden_val, snapshot->reg_val[i])
                                         );
                        }
                        else
                        {
                            p += sprintf(p, HEX_FMT"\t"HEX_FMT"\n", _golden.buf_golden_setting[i].addr, snapshot->reg_val[i]);
                        }
                    }

                    if ((p - start_p) + (p - page) >= PAGE_SIZE)
                    {
                        buf_golden_setting_idx = i+1;
                        break;
                    }
                }

                if ((p - start_p) + (p - page) >= PAGE_SIZE)
                {
                    break;
                }
            }
        }

        page_len = p - page;
    }
    else
    {
        p = page + page_len;
    }

    *start = page + (off - used_off);

    len = p - page;

    if (len > (off - used_off))
    {
        len -= (off - used_off);
    }
    else
    {
        len = 0;
        used_off += page_len;
        page_len = 0;
    }

    *eof = (0 == buf_golden_setting_idx && 0 == len) ? 1 : 0;

    EXIT_FUNC(FUNC_LV_BODY);

    return len < count ? len : count;
}

static int golden_test_write(struct file *file, const char *buffer,
                             unsigned long count, void *data)
{
    char desc[256];
    int len = 0;

    char cmd[64];
    unsigned int addr;
    unsigned int mask;
    unsigned int golden_val;

    ENTER_FUNC(FUNC_LV_BODY);

    len = (count < (sizeof(desc) - 1)) ? count : (sizeof(desc) - 1);

    if (copy_from_user(desc, buffer, len))
    {
        EXIT_FUNC(FUNC_LV_BODY);
        return 0;
    }

    desc[len] = '\0';

    // set golden setting (hex mode)
    if (sscanf(desc, "0x%x 0x%x 0x%x", &addr, &mask, &golden_val) == 3)
    {
        _golden_setting_add(&_golden, addr, mask, golden_val);
    }
    // set golden setting (dec mode)
    else if (sscanf(desc, "%d %d %d", &addr, &mask, &golden_val) == 3)
    {
        _golden_setting_add(&_golden, addr, mask, golden_val);
    }
    // set filter (func + line)
    else if (sscanf(desc, "filter %s %d", _golden.func, &_golden.line) == 2)
    {
    }
    // set filter (func)
    else if (sscanf(desc, "filter %s", _golden.func) == 1)
    {
        _golden.line = 0;
    }
    // set golden setting (mixed mode)
    else if (sscanf(desc, "0x%x 0b%s", &addr, cmd) == 2)
    {
        if (!_parse_mask_val(cmd, &mask, &golden_val))
        {
            _golden_setting_add(&_golden, addr, mask, golden_val);
        }
    }
    // set reg value (mask mode)
    else if (sscanf(desc, "set 0x%x 0x%x 0x%x", &addr, &mask, &golden_val) == 3)
    {
        _golden_write_reg(addr, mask, golden_val);
    }
    // set reg value (bit mode)
    else if (sscanf(desc, "set 0x%x %d %d", &addr, &mask, &golden_val) == 3)
    {
        if (0 <= mask && mask <= 31) // XXX: mask is bit number (alias)
        {
            golden_val = (golden_val & BIT(0)) << mask;
            mask = BIT(0) << mask;
            _golden_write_reg(addr, mask, golden_val);
        }
    }
    // set reg value (normal mode)
    else if (sscanf(desc, "set 0x%x 0x%x", &addr, &golden_val) == 2)
    {
        _golden_write_reg(addr, 0xFFFFFFFF, golden_val);
    }
    else if (sscanf(desc, "%s", cmd) == 1)
    {
        if (!strcmp(cmd, "enable"))
        {
            _golden_setting_enable(&_golden);
        }
        else if (!strcmp(cmd, "disable"))
        {
            _golden_setting_disable(&_golden);
        }
        else if (!strcmp(cmd, "normal"))
        {
            _golden_setting_set_mode(&_golden, MODE_NORMAL);
        }
        else if (!strcmp(cmd, "compare"))
        {
            _golden_setting_set_mode(&_golden, MODE_COMPARE);
        }
        else if (!strcmp(cmd, "apply"))
        {
            _golden_setting_set_mode(&_golden, MODE_APPLY);
        }
        else if (!strcmp(cmd, "color"))
        {
            _golden_setting_set_mode(&_golden, MODE_COLOR);
        }
        else if (!strcmp(cmd, "diff"))
        {
            _golden_setting_set_mode(&_golden, MODE_DIFF);
        }
        else if (!strcmp(cmd, "filter"))
        {
            _golden.func[0] = '\0';
        }
    }

    EXIT_FUNC(FUNC_LV_BODY);
    return count;
}

#endif /* CONFIG_MT_ENG_BUILD */

#if 0   // XXX: for debugfs
static ssize_t debug_read(struct file *file,
                          char __user *ubuf, size_t count, loff_t *ppos)
{
    char *page;
    char *start;
    int eof;
    int data = 0;
    int len = 0;

    if (page = (char*) __get_free_page(GFP_TEMPORARY))
    {
        len = golden_test_read(page, &start, *ppos, count, &eof, &data);
        len = simple_read_from_buffer(ubuf, count, ppos, page, len);
        free_page((unsigned long) page);
    }

    return len;
}

static ssize_t debug_open(struct inode *inode, struct file *file)
{
    file->private_data = inode->i_private;

    return 0;
}

static struct file_operations debug_fops =
{
    .read   = debug_read,
    .write  = golden_test_write,
    .open   = debug_open,
};
#endif  // XXX: for debugfs

void mt_clkmgr_debug_init(void)
{
    struct proc_dir_entry *entry;
    struct proc_dir_entry *clkmgr_dir;

    ENTER_FUNC(FUNC_LV_API);

    clkmgr_dir = proc_mkdir("clkmgr", NULL);

    if (!clkmgr_dir)
    {
        clk_err("[%s]: fail to mkdir /proc/clkmgr\n", __func__);
        EXIT_FUNC(FUNC_LV_API);
        return;
    }

    entry = create_proc_entry("clk_test", 00640, clkmgr_dir);

    if (entry)
    {
        entry->read_proc = clk_test_read;
        entry->write_proc = clk_test_write;
    }

    entry = create_proc_entry("pll_test", 00640, clkmgr_dir);

    if (entry)
    {
        entry->read_proc = pll_test_read;
        entry->write_proc = pll_test_write;
    }

    entry = create_proc_entry("pll_fsel", 00640, clkmgr_dir);

    if (entry)
    {
        entry->read_proc = pll_fsel_read;
        entry->write_proc = pll_fsel_write;
    }

    entry = create_proc_entry("subsys_test", 00640, clkmgr_dir);

    if (entry)
    {
        entry->read_proc = subsys_test_read;
        entry->write_proc = subsys_test_write;
    }

    entry = create_proc_entry("udelay_test", 00640, clkmgr_dir);

    if (entry)
    {
        entry->read_proc = udelay_test_read;
        entry->write_proc = udelay_test_write;
    }

#if defined(CONFIG_MT_ENG_BUILD)
    {
        #define GOLDEN_SETTING_BUF_SIZE (2 * PAGE_SIZE)

        unsigned int *buf;

        buf = kmalloc(GOLDEN_SETTING_BUF_SIZE, GFP_KERNEL);

        if (NULL != buf)
        {
            _golden_setting_init(&_golden, buf, GOLDEN_SETTING_BUF_SIZE);

            entry = create_proc_entry("golden_test", 00640, clkmgr_dir);

            if (entry)
            {
                entry->read_proc = golden_test_read;
                entry->write_proc = golden_test_write;
            }
        }
    }
#endif /* CONFIG_MT_ENG_BUILD */

#if 0   // XXX: for debugfs
    {
        struct dentry *dir, *file;

        dir = debugfs_create_dir("clkmgr", NULL);
        file = debugfs_create_file("golden_test", 0644, dir, (void *)0, &debug_fops);
    }
#endif  // XXX: for debugfs

    EXIT_FUNC(FUNC_LV_API);
}

static int mt_clkmgr_debug_bringup_init(void)
{
    ENTER_FUNC(FUNC_LV_API);

#if 1   // XXX: temp solution for init function not issued
    if (0 == initialized)
    {
        mt_clkmgr_init();
    }
#endif  // XXX: temp solution for init function not issued

    mt_clkmgr_debug_init();

    EXIT_FUNC(FUNC_LV_API);
    return 0;
}
module_init(mt_clkmgr_debug_bringup_init);
