#ifndef _MT_CLKMGR_H
#define _MT_CLKMGR_H

#include <linux/list.h>
#include "mach/mt_reg_base.h"
#include "mach/mt_typedefs.h"

//#define APMIXED_BASE   	0xF0209000
//#define AUDIO_BASE		0xF1221000
//#define AUDIO_REG_BASE 	(0xF1220000)
//#define G3D_CONFIG_BASE	0xF3000000
//#define DISPSYS_BASE		0xF4000000
//#define IMGSYS_CONFG_BASE 0xF5000000
//#define VDEC_GCON_BASE 	0xF6000000

#define AP_PLL_CON0         (APMIXEDSYS_BASE + 0x0000) //0xF0209000
#define AP_PLL_CON1         (APMIXEDSYS_BASE + 0x0004)
#define AP_PLL_CON2         (APMIXEDSYS_BASE + 0x0008)

#define PLL_HP_CON0         (APMIXEDSYS_BASE + 0x0014)

#define ARMPLL_CON0         (APMIXEDSYS_BASE + 0x0200)
#define ARMPLL_CON1         (APMIXEDSYS_BASE + 0x0204)
#define ARMPLL_PWR_CON0     (APMIXEDSYS_BASE + 0x020C)

#define MAINPLL_CON0        (APMIXEDSYS_BASE + 0x0210)
#define MAINPLL_CON1        (APMIXEDSYS_BASE + 0x0214)
#define MAINPLL_PWR_CON0    (APMIXEDSYS_BASE + 0x021C)

#define UNIVPLL_CON0        (APMIXEDSYS_BASE + 0x0220)
#define UNIVPLL_CON1        (APMIXEDSYS_BASE + 0x0224)
#define UNIVPLL_PWR_CON0    (APMIXEDSYS_BASE + 0x022C)

#define MMPLL_CON0          (APMIXEDSYS_BASE + 0x0230)
#define MMPLL_CON1          (APMIXEDSYS_BASE + 0x0234)
#define MMPLL_PWR_CON0	    (APMIXEDSYS_BASE + 0x023C)

#define MSDCPLL_CON0        (APMIXEDSYS_BASE + 0x0240)
#define MSDCPLL_CON1        (APMIXEDSYS_BASE + 0x0244)
#define MSDCPLL_PWR_CON0    (APMIXEDSYS_BASE + 0x024C)

#define VENCPLL_CON0        (DDRPHY_BASE+0x800)	//0xF000F8000
#define VENCPLL_CON1        (DDRPHY_BASE+0x804)
#define VENCPLL_PWR_CON0    (DDRPHY_BASE+0x80C)

#define CLK_DSI_PLL_CON0    (MIPI_CONFIG_BASE+0x50) //0xF0010050

#define CLK_CFG_0           (INFRA_BASE + 0x0040)	//
#define CLK_CFG_1           (INFRA_BASE + 0x0050)	//
#define CLK_CFG_2           (INFRA_BASE + 0x0060)	//
#define CLK_CFG_3           (INFRA_BASE + 0x0070)	//
#define CLK_CFG_4           (INFRA_BASE + 0x0080)	//
#define CLK_CFG_8           (INFRA_BASE + 0x0100)	//
#define CLK_CFG_9           (INFRA_BASE + 0x0104)
#define CLK_CFG_10          (INFRA_BASE + 0x0108)
#define CLK_CFG_11          (INFRA_BASE + 0x010C)
#define	CLK_SCP_CFG_0       (INFRA_BASE + 0x0200)
#define	CLK_SCP_CFG_1       (INFRA_BASE + 0x0204)

#define INFRA_PDN_SET       (INFRACFG_AO_BASE + 0x0040)	//0xF0001040
#define INFRA_PDN_CLR       (INFRACFG_AO_BASE + 0x0044)
#define INFRA_PDN_STA       (INFRACFG_AO_BASE + 0x0048)

#define TOPAXI_PROT_EN      (INFRACFG_AO_BASE + 0x0220)
#define TOPAXI_PROT_STA1    (INFRACFG_AO_BASE + 0x0228)

#define PERI_PDN0_SET       (PERICFG_BASE + 0x0008)		//0xF0003008
#define PERI_PDN0_CLR       (PERICFG_BASE + 0x0010)
#define PERI_PDN0_STA       (PERICFG_BASE + 0x0018)

#define PERI_PDN0_MD1_SET   (PERICFG_BASE + 0x0020)		//0xF0003020
#define PERI_PDN0_MD2_SET   (PERICFG_BASE + 0x0024)
#define PERI_PDN0_MD1_CLR   (PERICFG_BASE + 0x0028)
#define PERI_PDN0_MD2_CLR   (PERICFG_BASE + 0x002C)
#define PERI_PDN0_MD1_STA   (PERICFG_BASE + 0x0030)
#define PERI_PDN0_MD2_STA   (PERICFG_BASE + 0x0034)
#define PERI_PDN_MD_MASK    (PERICFG_BASE + 0x0038)

#define AUDIO_TOP_CON0      (AUDIO_REG_BASE + 0x0000)	//0xF1220000

#define MFG_CG_CON          (G3D_CONFIG_BASE + 0)	//0xF3000000
#define MFG_CG_SET          (G3D_CONFIG_BASE + 4)	//
#define MFG_CG_CLR          (G3D_CONFIG_BASE + 8)	//

#define DISP_CG_CON0        (DISPSYS_BASE + 0x100)	//0xF4000100
#define DISP_CG_SET0        (DISPSYS_BASE + 0x104)
#define DISP_CG_CLR0        (DISPSYS_BASE + 0x108)
#define DISP_CG_CON1        (DISPSYS_BASE + 0x110)
#define DISP_CG_SET1        (DISPSYS_BASE + 0x114)
#define DISP_CG_CLR1        (DISPSYS_BASE + 0x118)

#define IMG_CG_CON          (IMGSYS_CONFG_BASE + 0x0000)	//0xF5000000
#define IMG_CG_SET          (IMGSYS_CONFG_BASE + 0x0004)	
#define IMG_CG_CLR          (IMGSYS_CONFG_BASE + 0x0008)

#define VDEC_CKEN_SET       (VDEC_GCON_BASE + 0x0000)	//0xF6000000
#define VDEC_CKEN_CLR       (VDEC_GCON_BASE + 0x0004)
#define LARB_CKEN_SET       (VDEC_GCON_BASE + 0x0008)
#define LARB_CKEN_CLR       (VDEC_GCON_BASE + 0x000C)

enum {
    CG_PERI  = 0,
    CG_INFRA = 1,
    CG_TOPCK = 2,
    CG_DISP0 = 3,
    CG_DISP1 = 4,
    CG_IMAGE = 5,
    CG_MFG   = 6,
    CG_AUDIO = 7,
    CG_VDEC0 = 8,
    CG_VDEC1 = 9,
    NR_GRPS  = 10,
};

enum cg_clk_id{
    MT_CG_PERI_NFI					= 0,
    MT_CG_PERI_THERM				= 1,
    MT_CG_PERI_PWM1					= 2,
    MT_CG_PERI_PWM2					= 3,
    MT_CG_PERI_PWM3					= 4,
    MT_CG_PERI_PWM4					= 5,
    MT_CG_PERI_PWM5					= 6, 
    MT_CG_PERI_PWM6					= 7,
    MT_CG_PERI_PWM7					= 8,
    MT_CG_PERI_PWM					= 9,
    MT_CG_PERI_USB0					= 10,
    MT_CG_PERI_AP_DMA				= 11,
    MT_CG_PERI_MSDC30_0				= 12,
    MT_CG_PERI_MSDC30_1				= 13,
    MT_CG_PERI_MSDC30_2				= 14,
    MT_CG_PERI_NLI					= 15,
    MT_CG_PERI_UART0				= 16,
    MT_CG_PERI_UART1				= 17,
    MT_CG_PERI_UART2				= 18,
    MT_CG_PERI_UART3				= 19,
    MT_CG_PERI_BTIF					= 20,
    MT_CG_PERI_I2C0					= 21,
    MT_CG_PERI_I2C1					= 22,
    MT_CG_PERI_I2C2					= 23,
    MT_CG_PERI_AUXADC				= 24,
    MT_CG_PERI_SPI0					= 25,
    
    MT_CG_INFRA_DBGCLK				= 32,
    MT_CG_INFRA_SMI					= 33,
    MT_CG_INFRA_AUDIO				= 37,
    MT_CG_INFRA_EFUSE				= 38,
    MT_CG_INFRA_L2C_SRAM			= 39,
    MT_CG_INFRA_M4U					= 40,
    MT_CG_INFRA_MD1MCUAXI			= 41,
    MT_CG_INFRA_MD1HWMIXAXI			= 42,
    MT_CG_INFRA_MD1AHB				= 43,
    MT_CG_INFRA_CONNMCU				= 44,
    MT_CG_INFRA_TRNG				= 45,
    MT_CG_INFRA_CPUM				= 47,
    MT_CG_INFRA_KP					= 48,
    MT_CG_INFRA_CCIF0				= 52,
    MT_CG_INFRA_PMICWRAP			= 55,
    
    //MT_CG_TOPCK_PMICSPI             = 64,
    MT_CG_TOPCK_PMICSPI             = 69,
    
    MT_CG_DISP0_SMI_COMMON			= 96,
    MT_CG_DISP0_SMI_LARB0			= 97,
    MT_CG_DISP0_MM_CMDQ				= 98,
    MT_CG_DISP0_MUTEX				= 99,
    MT_CG_DISP0_DISP_COLOR			= 100,
    MT_CG_DISP0_DISP_BLS			= 101,
    MT_CG_DISP0_DISP_WDMA			= 102,
    MT_CG_DISP0_DISP_RDMA			= 103,
    MT_CG_DISP0_DISP_OVL			= 104,
    MT_CG_DISP0_MDP_TDSHP			= 105,
    MT_CG_DISP0_MDP_WROT			= 106,
    MT_CG_DISP0_MDP_WDMA			= 107,
    MT_CG_DISP0_MDP_RSZ1			= 108,
    MT_CG_DISP0_MDP_RSZ0			= 109,
    MT_CG_DISP0_MDP_RDMA			= 110,
    MT_CG_DISP0_MDP_BLS_26M			= 111,
    MT_CG_DISP0_CAM_MDP				= 112,
    MT_CG_DISP0_FAKE_ENG			= 113,
    MT_CG_DISP0_MUTEX_32K			= 114,
    
    MT_CG_DISP1_DSI_ENGINE			= 128,
    MT_CG_DISP1_DSI_DIGITAL			= 129,
    MT_CG_DISP1_DPI_DIGITAL_LANE	= 130,
    MT_CG_DISP1_DPI_ENGINE			= 131,
    
    MT_CG_IMAGE_LARB2_SMI			= 160,
    MT_CG_IMAGE_CAM_SMI				= 165,
    MT_CG_IMAGE_CAM_CAM				= 166,
    MT_CG_IMAGE_SEN_TG				= 167,
    MT_CG_IMAGE_SEN_CAM				= 168,
    MT_CG_IMAGE_VENC_JPENC			= 169,
    
    MT_CG_MFG_G3D					= 192,
    
    MT_CG_AUDIO_AFE                 = 226,
    MT_CG_AUDIO_I2S                 = 230,
    
    MT_CG_VDEC0_VDEC				= 256,
    
    MT_CG_VDEC1_LARB				= 288,
    
    CG_PERI_FROM					= MT_CG_PERI_NFI,
    CG_PERI_TO						= MT_CG_PERI_SPI0,
    NR_PERI_CLKS					= 26,
    
    CG_INFRA_FROM                   = MT_CG_INFRA_DBGCLK,
    CG_INFRA_TO                     = MT_CG_INFRA_PMICWRAP,
    NR_INFRA_CLKS                   = 15,
    
    CG_TOPCK_FROM                   = MT_CG_TOPCK_PMICSPI,
    CG_TOPCK_TO                     = MT_CG_TOPCK_PMICSPI,
    NR_TOPCK_CLKS                   = 1,
    
    CG_DISP0_FROM                   = MT_CG_DISP0_SMI_COMMON,
    CG_DISP0_TO                     = MT_CG_DISP0_MUTEX_32K,
    NR_DISP0_CLKS                   = 19,
    
    CG_DISP1_FROM                   = MT_CG_DISP1_DSI_ENGINE,
    CG_DISP1_TO                     = MT_CG_DISP1_DPI_ENGINE,
    NR_DISP1_CLKS                   = 4,
    
    CG_IMAGE_FROM                   = MT_CG_IMAGE_LARB2_SMI,
    CG_IMAGE_TO                     = MT_CG_IMAGE_VENC_JPENC,
    NR_IMAGE_CLKS                   = 6,
    
    CG_MFG_FROM                     = MT_CG_MFG_G3D,
    CG_MFG_TO                       = MT_CG_MFG_G3D,
    NR_MFG_CLKS                     = 1,
    
    CG_AUDIO_FROM                   = MT_CG_AUDIO_AFE,
    CG_AUDIO_TO                     = MT_CG_AUDIO_I2S,
    NR_AUDIO_CLKS                   = 2,
    
    CG_VDEC0_FROM                   = MT_CG_VDEC0_VDEC,
    CG_VDEC0_TO                     = MT_CG_VDEC0_VDEC,
    NR_VDEC0_CLKS                   = 1,
    
    CG_VDEC1_FROM                   = MT_CG_VDEC1_LARB,
    CG_VDEC1_TO                     = MT_CG_VDEC1_LARB,
    NR_VDEC1_CLKS                   = 1,
    
    NR_CLKS                         = 289,
};

#define UNIVPLL2_D4     4

enum {
	//CLK_CFG_0
    MT_MUX_MM           = 0,
//    MT_MUX_DDRPHY     = 1,
//    MT_MUX_MEM		= 2,
//    MT_MUX_AXI		= 3,

    //CLK_CFG_1
    MT_MUX_CAMTG        = 1,
    MT_MUX_MFG          = 2,
    MT_MUX_VDEC         = 3,
    MT_MUX_PWM          = 4,

    //CLK_CFG_2
    MT_MUX_MSDC30_0     = 5,
    MT_MUX_USB20        = 6,
    MT_MUX_SPI          = 7,
    MT_MUX_UART         = 8,

    //CLK_CFG_3
    MT_MUX_AUDINTBUS    = 9,
    MT_MUX_AUDIO        = 10,
    MT_MUX_MSDC30_2     = 11,
    MT_MUX_MSDC30_1     = 12,

    //CLK_CFG_4
//    MT_MUX_SCP			= 13,
//    MT_MUX_PMICSPI		= 14,

    NR_MUXS             = 13,
};

enum {
    ARMPLL  = 0,
    MAINPLL = 1,
    MSDCPLL = 2,
    UNIVPLL = 3,
    MMPLL   = 4,
    VENCPLL = 5,
    NR_PLLS = 6,
};

enum {
    SYS_MD1 = 0,
    SYS_CONN = 1,
    SYS_DPY = 2,
    SYS_DIS = 3,
    SYS_MFG = 4,
    SYS_ISP = 5,
    SYS_IFR = 6,
    SYS_VDE = 7,
    NR_SYSS = 8,
};

enum {
//    MT_LARB_VDEC = 0,
//    MT_LARB_DISP = 1,
//    MT_LARB_IMG = 2,
    MT_LARB_DISP = 0,
    MT_LARB_VDEC = 1,
    MT_LARB_IMG = 2,
};

/* larb monitor mechanism definition*/
enum {
    LARB_MONITOR_LEVEL_HIGH     = 10,
    LARB_MONITOR_LEVEL_MEDIUM   = 20,
    LARB_MONITOR_LEVEL_LOW      = 30,
};

struct larb_monitor {
    struct list_head link;
    int level;
    void (*backup)(struct larb_monitor *h, int larb_idx);       /* called before disable larb clock */
    void (*restore)(struct larb_monitor *h, int larb_idx);      /* called after enable larb clock */
};

enum monitor_clk_sel{
    no_clk               = 0,
    AD_SYS_26M_CK        = 1,
    rtc32k_ck_i          = 2,
    clkph_MCLK_o         = 7,
    AD_DPICLK            = 8,
    AD_MSDCPLL_CK        = 9,
    AD_MMPLL_CK          = 10,
    AD_UNIV_178P3M_CK    = 11,
    AD_MAIN_H156M_CK     = 12,
    AD_VENCPLL_CK        = 13,
};

enum ckmon_sel{
    clk_ckmon1           = 1,
    clk_ckmon2           = 2,
    clk_ckmon3           = 3,
};

extern void register_larb_monitor(struct larb_monitor *handler);
extern void unregister_larb_monitor(struct larb_monitor *handler);

/* clock API */
extern int enable_clock(enum cg_clk_id id, char *mod_name);
extern int disable_clock(enum cg_clk_id id, char *mod_name);
extern int mt_enable_clock(enum cg_clk_id id, char *mod_name);
extern int mt_disable_clock(enum cg_clk_id id, char *mod_name);

extern int enable_clock_ext_locked(int id, char *mod_name);
extern int disable_clock_ext_locked(int id, char *mod_name);

extern int clock_is_on(int id);

extern int clkmux_sel(int id, unsigned int clksrc, char *name);
extern void enable_mux(int id, char *name);
extern void disable_mux(int id, char *name);

extern void clk_set_force_on(int id);
extern void clk_clr_force_on(int id);
extern int clk_is_force_on(int id);

/* pll API */
extern int enable_pll(int id, char *mod_name);
extern int disable_pll(int id, char *mod_name);
extern int enable_pll_spec(int id, char * mod_name);
extern int disable_pll_spec(int id, char * mod_name);


extern int pll_hp_switch_on(int id, int hp_on);
extern int pll_hp_switch_off(int id, int hp_off);

extern int pll_fsel(int id, unsigned int value);
extern int pll_is_on(int id);

/* subsys API */
extern int enable_subsys(int id, char *mod_name);
extern int disable_subsys(int id, char *mod_name);

extern int subsys_is_on(int id);
extern int md_power_on(int id);
extern int md_power_off(int id, unsigned int timeout);
extern int conn_power_on(void);
extern int conn_power_off(void);

/* other API */

extern void enable_clksq1(void);
extern void disable_clksq1(void);

extern void clksq1_sw2hw(void);
extern void clksq1_hw2sw(void);
/*
extern void pmicspi_mempll2clksq(void);
extern void pmicspi_clksq2mempll(void);

extern int get_gpu_power_src(void);
*/
const char* grp_get_name(int id);

extern int clkmgr_is_locked(void);

/* init */
extern void mt_clkmgr_init(void);

extern bool isp_vdec_on_off(void);

extern void CLKM_32K(bool flag);
extern int CLK_Monitor(enum ckmon_sel ckmon, enum monitor_clk_sel sel, int div);

#endif
