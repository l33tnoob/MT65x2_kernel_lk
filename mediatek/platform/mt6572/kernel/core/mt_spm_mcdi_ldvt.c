#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/proc_fs.h>
#include <linux/platform_device.h>
#include <linux/earlysuspend.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/err.h>
#include <linux/delay.h>  

#include <mach/irqs.h>
#include <mach/mt_spm.h>
#include <mach/mt_dormant.h>
#include <mach/mt_gpt.h>
#include <mach/mt_spm_pcm.h>



static struct task_struct *mcdi_task_0;
static struct task_struct *mcdi_task_1;

//static struct mtk_irq_mask MCDI_cpu_irq_mask;
extern u32 MCDI_Test_Mode;

#define read_cntp_cval(cntp_cval_lo, cntp_cval_hi) \
do {    \
    __asm__ __volatile__(   \
    "MRRC p15, 2, %0, %1, c14\n"    \
    :"=r"(cntp_cval_lo), "=r"(cntp_cval_hi) \
    :   \
    :"memory"); \
} while (0)

#define write_cntp_cval(cntp_cval_lo, cntp_cval_hi) \
do {    \
    __asm__ __volatile__(   \
    "MCRR p15, 2, %0, %1, c14\n"    \
    :   \
    :"r"(cntp_cval_lo), "r"(cntp_cval_hi));    \
} while (0)

#define write_cntp_ctl(cntp_ctl)  \
do {    \
    __asm__ __volatile__(   \
    "MCR p15, 0, %0, c14, c2, 1\n"  \
    :   \
    :"r"(cntp_ctl)); \
} while (0)


#if 0
// =====================================================================
//---------------------------------------------------
// address definition
//---------------------------------------------------
#define disp_base (0xf4000000)
#define DISP_MUTEX_BASE (disp_base+0x11000)
#define DISP_MUTEX0 (DISP_MUTEX_BASE+0x24)

//------------------------------------------------------------------------
// base address definition
//------------------------------------------------------------------------
#define mmsys_base (0xf4000000)
#define i2c_base (0xf4014000)
#define SPM_DSI_BASE ( mmsys_base+0xD000)
#define dbi_base ( mmsys_base+0xC000)
#define LCM_RST_B ( dbi_base+0x0010 )
#define DSI_CMDQ_BASE  (SPM_DSI_BASE+0x180)

//------------------------------------------------------------------------
// MIPI config register map
//------------------------------------------------------------------------
#define mipi_tx_config_base (0xf0012000)
#define mipi_rx_ana_base (0xf0012800)
// =====================================================================
#endif

extern void mtk_wdt_suspend(void);
extern void mtk_wdt_resume(void);


#define clc_notice spm_notice

static void spm_show_lcm_image(void)
{
#if 0
//---------------------------------------------------
// load image
//---------------------------------------------------
u32 pic0_addr=0x95000000;//
//d.load.binary Kara_1080x1920.bmp          &pic0_addr 

u32 v_pic0_addr;

u32 w=1080; //  1080
u32 h=1920; //  1920

u32 pitch=3*w;//

unsigned long flags;


//------------------------------------------------------------------------
// MT6582 DSI Address
//------------------------------------------------------------------------
u32 DSI_START= SPM_DSI_BASE+0x00;
u32 DSI_INTSTA= SPM_DSI_BASE+0x0c;
u32 DSI_COM_CON= SPM_DSI_BASE+0x10;
u32 DSI_MODE_CON= SPM_DSI_BASE+0x14;
u32 DSI_TXRX_CON= SPM_DSI_BASE+0x18;
u32 DSI_PSCON= SPM_DSI_BASE+0x1C;
u32 DSI_VSA_NL= SPM_DSI_BASE+0x20;
u32 DSI_VBP_NL= SPM_DSI_BASE+0x24;
u32 DSI_VFP_NL= SPM_DSI_BASE+0x28;
u32 DSI_VACT_NL= SPM_DSI_BASE+0x2c;
u32 DSI_HSA_WC= SPM_DSI_BASE+0x50;
u32 DSI_HBP_WC= SPM_DSI_BASE+0x54;
u32 DSI_HFP_WC= SPM_DSI_BASE+0x58;
u32 DSI_BLLP_WC= SPM_DSI_BASE+0x5c;
u32 DSI_CMDQ_CON= SPM_DSI_BASE+0x60;
u32 DSI_HSTX_CKLP_WC= SPM_DSI_BASE+0x64;
u32 DSI_RACK= SPM_DSI_BASE+0x84;
u32 DSI_MEM_CONTI= SPM_DSI_BASE+0x90;

u32 DSI_PHY_CON= SPM_DSI_BASE+0x100;
u32 DSI_PHY_LCCON= SPM_DSI_BASE+0x104;
u32 DSI_PHY_LD0CON= SPM_DSI_BASE+0x108;
u32 DSI_PHY_TIMCON0= SPM_DSI_BASE+0x110;
u32 DSI_PHY_TIMCON1= SPM_DSI_BASE+0x114;
u32 DSI_PHY_TIMCON2= SPM_DSI_BASE+0x118;
u32 DSI_PHY_TIMCON3= SPM_DSI_BASE+0x11C;
u32 DSI_PHY_TIMCON4= SPM_DSI_BASE+0x120;

u32 DSI_VM_CMD_CON= SPM_DSI_BASE+0x130;
u32 DSI_VM_CMD_DATA0= SPM_DSI_BASE+0x134;
u32 DSI_VM_CMD_DATA4= SPM_DSI_BASE+0x138;
u32 DSI_VM_CMD_DATA8= SPM_DSI_BASE+0x13c;
u32 DSI_VM_CMD_DATAC= SPM_DSI_BASE+0x140;

u32 DSI_DEBUG_SEL= SPM_DSI_BASE+0x170;

u32 DSI_CMDQ_0= DSI_CMDQ_BASE+0x00;
u32 DSI_CMDQ_1= DSI_CMDQ_BASE+0x04;
u32 DSI_CMDQ_2= DSI_CMDQ_BASE+0x08;
u32 DSI_CMDQ_3= DSI_CMDQ_BASE+0x0c;
u32 DSI_CMDQ_4= DSI_CMDQ_BASE+0x10;
u32 DSI_CMDQ_5= DSI_CMDQ_BASE+0x14;
u32 DSI_CMDQ_6= DSI_CMDQ_BASE+0x18;
u32 DSI_CMDQ_7= DSI_CMDQ_BASE+0x1c;
u32 DSI_CMDQ_8= DSI_CMDQ_BASE+0x20;
u32 DSI_CMDQ_9= DSI_CMDQ_BASE+0x24;

//---------------------------------------------------
// parameter setting
//---------------------------------------------------
u32 size=3; // 0:480x800, 1:720x1280, 2:540x960, 3:1080x1920
u32 out_if=0;// 0:dsi, 1:dbi, 2:dpi
u32 dsi_video_mode=1;// 0:command mode, 1:video mode
u32 reload_img=1;//

u32 sharp_fhd=1;//
u32 temp_address;

//clc_notice("spm_show_lcm_image() start.\n");

/* Mask ARM i bit */
//asm volatile("cpsid i @ arch_local_irq_disable" : : : "memory", "cc"); // set i bit to disable interrupt    
local_irq_save(flags);

// disable all display engine interrupt ===================================
spm_write(0xf400d008 ,  0x0 );
spm_write(0xf4003004 ,  0x0 );
spm_write(0xf4011000 ,  0x0 );
spm_write(0xf4006000 ,  0x0 );
// =========================================================

v_pic0_addr = pic0_addr | 0x40000000;

#if 0
for(temp_address = v_pic0_addr ; temp_address<=(v_pic0_addr+(w*h*pitch)) ; temp_address+=4)
{
    if(((temp_address/(w*200*pitch))%2) == 0)
    {
        spm_write(temp_address, 0xffffffff);
    }
    else
    {
        spm_write(temp_address, 0x0);
    }
}
#endif

//---------------------------------------------------
// set gpio
//---------------------------------------------------
// gpio 130, mode 7 : LPTE
// gpio 131, mode 7 : LRSTB
// gpio 132, mode 1 : LPCE1B
// gpio 133, mode 1 : LPCE0B
// gpio 138~142, mode 7 : dbi-c
// gpio 143~171, mode 1 : dpi, dbi-b

spm_write(0xf0005DA0 ,  0x027f );// gpio 134~130
spm_write(0xf0005DB0 ,  0x7e00 );// gpio 139~135
spm_write(0xf0005DC0 ,  0x13ff );// gpio 144~140
spm_write(0xf0005DD0 ,  0x1249 );// gpio 149~145
spm_write(0xf0005DE0 ,  0x1249 );// gpio 154~150
spm_write(0xf0005DF0 ,  0x1249 );// gpio 159~155
spm_write(0xf0005E00 ,  0x1249 );// gpio 164~160
spm_write(0xf0005E10 ,  0x1249 );// gpio 169~165
spm_write(0xf0005E20 ,  0x0009 );// gpio 174~170

u32 rdma0_out_sel=0;// dsi
//------------------------------------------------------------------------
// PHY Timing Control
//------------------------------------------------------------------------
u32 fbkdiv=0x47;//
u32 txdiv0=0x1;//
u32 txdiv1=0x1;//

//------------------------------------------------------------------------
// Clock source and debug settings
//------------------------------------------------------------------------
spm_write(0xf0000154 ,  0x00000000);    // [12:11] rg_mipi_26m_sel
spm_write(0xf5000000 ,  0x00000e00);    // img_cg_con

spm_write(mmsys_base+0x00108 ,  0xffffffff); // clear cg0
spm_write(mmsys_base+0x00118 ,  0xffffffff); // clear cg1
spm_write(mmsys_base+0x0004c ,  0x00000000); // dsi out 


u32 DSI_CON= mipi_tx_config_base+0x000;
u32 DSI_CLOCK_LANE= mipi_tx_config_base+0x004;
u32 DSI_DATA_LANE_0= mipi_tx_config_base+0x008;
u32 DSI_DATA_LANE_1= mipi_tx_config_base+0x00c;
u32 DSI_DATA_LANE_2= mipi_tx_config_base+0x010;
u32 DSI_DATA_LANE_3= mipi_tx_config_base+0x014;
u32 DSI_TOP_CON= mipi_tx_config_base+0x040;
u32 DSI_BG_CON= mipi_tx_config_base+0x044;
u32 DSI_PLL_CON0= mipi_tx_config_base+0x050;
u32 DSI_PLL_CON1= mipi_tx_config_base+0x054;
u32 DSI_PLL_TOP= mipi_tx_config_base+0x060;
u32 DSI_GPIO_EN= mipi_tx_config_base+0x068;
u32 DSI_GPIO_OUT= mipi_tx_config_base+0x06c;
u32 DSI_SW_CTRL_EN= mipi_tx_config_base+0x080;
u32 DSI_SW_CTRL_CON0= mipi_tx_config_base+0x084;
u32 DSI_SW_CTRL_CON1= mipi_tx_config_base+0x088;

//------------------------------------------------------------------------
// MIPI config
//------------------------------------------------------------------------
// [0] RG_DSI_BG_CORE_EN = 1
// [1] RG_DSI_BG_CKEN = 1
spm_write(DSI_BG_CON   ,  0x88492483);

// [1] RG_DSI_LNT_HS_BIAS_EN = 1
spm_write(DSI_TOP_CON  ,  0x00000082);

// [0] RG_DSI0_LDOCORE_EN = 1
// [1] RG_DSI0_CKG_LDOOUT_EN = 1
spm_write(DSI_CON      ,  0x00000003);

// [10] RG_MPPLL_BIAS_EN = 1
spm_write(DSI_PLL_TOP  ,  0x00000600);

// [7:1] RG_DSI0_MPPLL_FBKDIV = 7¡¦h13
// [13:12] RG_DSI0_MPPLL_TXDIV0 = 0
// [15:14] RG_DSI0_MPPLL_TXDIV1 = 0
u32 pll_con=0xbe510000|( txdiv1<<0xE)|( txdiv0<<0xC)|( fbkdiv<<1);//
spm_write(DSI_PLL_CON0  ,  pll_con);

// [0] RG_DSI0_MPPLL_PLL_EN = 1
pll_con=pll_con|0x1;//
spm_write(DSI_PLL_CON0  ,  pll_con);
udelay(100);

//clc_notice( "PLL setting finish!!\n");

spm_write(DSI_CLOCK_LANE     ,  0x00000821);
spm_write(DSI_DATA_LANE_0    ,  0x00000401);
spm_write(DSI_DATA_LANE_1    ,  0x00000101);
spm_write(DSI_DATA_LANE_2    ,  0x00000101);
spm_write(DSI_DATA_LANE_3    ,  0x00000101);

//------------------------------------------------------------------------
// DSI / LCM Reset
//------------------------------------------------------------------------
//dsi reset - must reset after DSI clock is on
spm_write(DSI_COM_CON ,  0x00000001);
spm_write(DSI_COM_CON ,  0x00000000);

//------------------------------------------------------------------------
// DSI Timing Control
//------------------------------------------------------------------------
u32 txdiv1_real=0;//
if ( txdiv1==0x00)
    txdiv1_real=0x01;

if ( txdiv1==0x01)
    txdiv1_real=0x02;

if ( txdiv1==0x02)
    txdiv1_real=0x04;

if ( txdiv1==0x03)
    txdiv1_real=0x04;

//clc_notice( "txdiv1_real=0x%x\n", txdiv1_real);

u32 txdiv0_real=0;//
if ( txdiv0==0x00)
    txdiv0_real=0x01;

if ( txdiv0==0x01)
    txdiv0_real=0x02;

if ( txdiv0==0x02)
    txdiv0_real=0x04;

if ( txdiv0==0x03)
    txdiv0_real=0x04;

//clc_notice( "txdiv0_real=0x%x\n", txdiv0_real);

////------ cycle time 
u32 cycle_time=0;
////cycle_time=1000xtxdiv1_realxtxdiv0_realx8/(26x( fbkdiv+1)x2)
cycle_time=(0x99*txdiv1_real*txdiv0_real)/(fbkdiv+0x01);

u32 ui=(0x13*txdiv1_real*txdiv0_real)/(fbkdiv+0x01);
//clc_notice( "ui=0x%x\n", ui);

////------ lpx: 50ns ~
u32 lpx=0;//
lpx=(0x50/cycle_time);

if (lpx==0x00)
    lpx=0x01;

//clc_notice( "lpx=0x%x\n", lpx);

////------ hs_prep: 40ns+4*UI  ~  85ns+6*UI
u32 hs_prep=((0x40+0x05*ui)/cycle_time);//
if (hs_prep==0x00)
    hs_prep=0x01;

//clc_notice( "hs_prep=0x%x\n", hs_prep);

////------ hs_prep+hs_zero: 145ns+10*UI
u32 hs_zero=((0xc8+0x0a*ui)/cycle_time)-hs_prep;//
//clc_notice( "hs_zero=0x%x\n", hs_zero);

////------ hs_trail:max(n*8*UI,60ns+n*4*UI) ~
 u32 hs_trail=0;
//// hs_trail_m= lane_num//
u32 hs_trail_m=1;//
//clc_notice( "hs_trail_m=0x%x\n", hs_trail_m);

u32 hs_trail_n=(((hs_trail_m*0x04)+0x60)/cycle_time);//
//clc_notice( "hs_trail_n=0x%x\n", hs_trail_n);

if (hs_trail_m>hs_trail_n)
{
  hs_trail=hs_trail_m+0x0a;
  //clc_notice( "hs_trail=0x%x\n", hs_trail);
}
else
{
  hs_trail=hs_trail_n+0x0a;
  //clc_notice( "hs_trail=0x%x\n", hs_trail);
}

////------ ta_go:4*LPX
u32 ta_go=4*lpx;
//clc_notice( "ta_go=0x%x\n", ta_go);

////------ ta_sure:LPX ~ 2*LPX
 u32 ta_sure= lpx+( lpx/0x02);
//clc_notice( "ta_sure=0x%x\n", ta_sure);

////------ ta_get:5*LPX
 u32 ta_get=5* lpx;
//clc_notice( "ta_get=0x%x\n", ta_get);

u32  ta_sack=0x01;
u32  cont_det=0x00;

////------ hs_exit, clk_exit, clk_post
u32 hs_exit=2* lpx;
u32 clk_exit=2* lpx;
u32 clk_post=0x0e;
//clc_notice( "clk_post=0x%x\n", clk_post);

////------ clk_hs_prep:38~95
u32 clk_hs_prep=(0x40/ cycle_time);
if ( clk_hs_prep==0x00)
{
 clk_hs_prep=0x01;
}
//clc_notice( "clk_hs_prep=0x%x\n", clk_hs_prep);

////------ clk_hs_prep+clk_zero:300~
// clk_zero=(0x190/ cycle_time)- clk_hs_prep
u32 clk_zero=(0x190/ cycle_time);
//clc_notice( "clk_zero=0x%x\n", clk_zero);

////------ clk_trail:60~
u32 clk_trail=(0x64/ cycle_time)+0x0a;
//clc_notice( "clk_trail=0x%x\n", clk_trail);

u32 timcon0=( hs_trail<<0x018)|( hs_zero<<0x10)|( hs_prep<<0x08)| lpx;//
u32 timcon1=( hs_exit<<0x18)|( ta_get<<0x10)|( ta_sure<<0x08)| ta_go;//
u32 timcon2=( clk_trail<<0x018)|( clk_zero<<0x10)| cont_det;//
u32 timcon3=( clk_exit<<0x10)|( clk_post<<0x08)| clk_hs_prep;//

spm_write(DSI_PHY_TIMCON0 ,   timcon0);   // DSI_PHY_TIMCON0
spm_write(DSI_PHY_TIMCON1 ,   timcon1);   // DSI_PHY_TIMCON1
spm_write(DSI_PHY_TIMCON2 ,   timcon2);   // DSI_PHY_TIMCON2
spm_write(DSI_PHY_TIMCON3 ,   timcon3);   // DSI_PHY_TIMCON3

//do dsi/DSI_test_video_mode.cmm
//------------------------------------------------------------------------
// Configuration
//------------------------------------------------------------------------
u32 color_fmt=0x3;            // 0:RGB565, 1:RGB666, 2:RGB666_l, 3:RGB888
u32 lane_num=4;//
u32 dsi_mode=1;               // '0':command mode
                          // '1':sync-pulse video mode
                          // '2':sync-event video mode
                          // '3':burst video mode
u32 cklp_en=1;                // clk low-power enable

//------------------------------------------------------------------------
// DSI setting
//------------------------------------------------------------------------
//DSI mode control
spm_write(DSI_MODE_CON ,  0x0);   // [1:0]:DSI_MODE_CON

u32 lane_enable=0x0f;//
u32 txrx_con=( cklp_en<<0x10)|( lane_enable<<2);//

spm_write(DSI_TXRX_CON ,   txrx_con);    //
                  // [1:0]: virtual channel
                  // [5:2]: lane number
                  // [6]:   disable EoTp
                  // [7]:   BLLP with null packet
                  // [8]:   te_freerun
                  // [15:12]: maximum return packet size                 
                  // [16]: cklp_en

u32 word_cnt= w*3;//
u32 pscon=( color_fmt<<0x10)| word_cnt;//
spm_write(DSI_PSCON ,     pscon   ); // [13:0]:word_cnt, [17:16]:format

spm_write(DSI_BLLP_WC ,  0x00000100   ); // DSI_BLLP_WC
spm_write(DSI_MEM_CONTI ,  0x0000003c  );  // RWMEM_CONTI
spm_write(DSI_HSTX_CKLP_WC ,   pscon   );  // DSI_HSTX_CKLP_WC

spm_write(DSI_DEBUG_SEL ,  0x0000000c);

spm_write(DSI_COM_CON ,  0x00000002 );   //// dsi_en

mdelay(10);

//------------------------------------------------------------------------
// Setup LCM
//------------------------------------------------------------------------
//do dsi/DSI_set_R63311_video.cmm
//------------------------------------------------------------------------
// Setup timing paramaters 
// based on Novatek FHD panel module UserGuide page.27
//------------------------------------------------------------------------
 clk_hs_prep=8;
 clk_zero=30;
 clk_post=22;
 clk_trail=8;
 clk_exit=15;

 hs_prep=9;
 hs_zero=10;
 hs_trail=20;
 hs_exit=15;

 lpx=9;
 ta_sure=3;
 ta_get=4* lpx;
 ta_sure=5* lpx;

 timcon0=( hs_trail<<0x018)|( hs_zero<<0x10)|( hs_prep<<0x08)| lpx;//
 timcon1=( hs_exit<<0x18)|( ta_get<<0x10)|( ta_sure<<0x08)| ta_go;//
 timcon2=( clk_trail<<0x018)|( clk_zero<<0x10)| cont_det;//
 timcon3=( clk_exit<<0x10)|( clk_post<<0x08)| clk_hs_prep;//

spm_write(DSI_PHY_TIMCON0 ,   timcon0);   // DSI_PHY_TIMCON0
spm_write(DSI_PHY_TIMCON1 ,   timcon1);   // DSI_PHY_TIMCON1
spm_write(DSI_PHY_TIMCON2 ,   timcon2);   // DSI_PHY_TIMCON2
spm_write(DSI_PHY_TIMCON3 ,   timcon3);   // DSI_PHY_TIMCON3

//------------------------------------------------------------------------
// Setup video mode
// based on Novatek FHD panel module UserGuide page.27
//------------------------------------------------------------------------
spm_write(DSI_VSA_NL ,  0x00000001);     // DSI_VSA_NL 
spm_write(DSI_VBP_NL ,  0x00000004);     // DSI_VBP_NL 
spm_write(DSI_VFP_NL ,  0x00000002);     // DSI_VFP_NL 
spm_write(DSI_VACT_NL ,  h);            // DSI_VACT_NL(Set Vertical Active line=800)
spm_write(DSI_HSA_WC ,  0x00000010);     // DSI_HSA_WC=1*4
spm_write(DSI_HBP_WC ,  0x00000080);     // DSI_HBP_WC=235/8*4
spm_write(DSI_HFP_WC ,  0x00000118);     // DSI_HFP_WC=235/8*4

//------------------------------------------------------------------------
// Misc
//------------------------------------------------------------------------

spm_write(LCM_RST_B ,  0x00000000);
mdelay(100);
spm_write(LCM_RST_B ,  0x00000001);

mdelay(10);

//------------------------------------------------------------------------
// FullHD LCM setup
//------------------------------------------------------------------------
spm_write(DSI_CMDQ_0 ,  0x04B02300);  // REGW 0xB0
spm_write(DSI_CMDQ_CON ,  0x00000001);  
spm_write(DSI_START ,  0x00000000);  
spm_write(DSI_START ,  0x00000001);  // DSI_START

//clc_notice("spm_show_lcm_image() 00.\n");

while ((spm_read(DSI_INTSTA)&0x2)!=0x2) //
{
  mdelay(500); 
}
spm_write(DSI_INTSTA ,  0xfffd );//write 0 clear

//clc_notice("spm_show_lcm_image() 01.\n");


mdelay(150);     //// mdelay(150ms

spm_write(DSI_CMDQ_0 ,  0x00351500  );// REGW 0x35
spm_write(DSI_CMDQ_CON ,  0x00000001  );
spm_write(DSI_START ,  0x00000000  );
spm_write(DSI_START ,  0x00000001  );// DSI_START

while ((spm_read(DSI_INTSTA)&0x2)!=0x2) //
{
  mdelay(500); 
}
spm_write(DSI_INTSTA ,  0xfffd); //write 0 clear

//clc_notice("spm_show_lcm_image() 02.\n");


mdelay(150);     //// mdelay(150ms

spm_write(DSI_CMDQ_0 ,  0x00290500);  // REGW 0x29
spm_write(DSI_CMDQ_CON ,  0x00000001);  
spm_write(DSI_START ,  0x00000000);  
spm_write(DSI_START ,  0x00000001);  // DSI_START

while ((spm_read(DSI_INTSTA)&0x2)!=0x2) //
{
  mdelay(500); 
}
spm_write(DSI_INTSTA ,  0xfffd); //write 0 clear

//clc_notice("spm_show_lcm_image() 03.\n");

mdelay(150);     //// mdelay(150ms

spm_write(DSI_CMDQ_0 ,  0x00110500);  // REGW 0x11
spm_write(DSI_CMDQ_CON ,  0x00000001);  
spm_write(DSI_START ,  0x00000000);  
spm_write(DSI_START ,  0x00000001);  // DSI_START

while ((spm_read(DSI_INTSTA)&0x2)!=0x2) //
{
  mdelay(500); 
}
spm_write(DSI_INTSTA ,  0xfffd); //write 0 clear

//clc_notice("spm_show_lcm_image() 04.\n");

//------------------------------------------------------------------------
// HS mode transmission, data from lcd
//------------------------------------------------------------------------
spm_write(DSI_PHY_LCCON ,  0x00000001);   // enable clock lane HS mode
mdelay(500);

//// DSI mode control
spm_write(DSI_MODE_CON  ,  dsi_mode);    // [1:0]:DSI_MODE_CON, 0:command mode, 1:sync_pulse, 2:sync_event, 3:burst
spm_write(DSI_START     ,  0x000);   
spm_write(DSI_START     ,  0x001);        // start dsi

//do cmm/ovl_bls.cmm
//---------------------------------------------------
// set disp_mutex
//---------------------------------------------------
u32 mutex_sof_src=1;// dsi
spm_write(disp_base+0x11000 ,  0x00000004); // write 0008
spm_write(disp_base+0x11030 ,  mutex_sof_src); // write 0009
spm_write(disp_base+0x1102c ,  0x00000084); // write 0010
spm_write(disp_base+0x11024 ,  0x00000001); // write 0011
//// polling if we get mutex0 
while ((spm_read(DISP_MUTEX_BASE+0x24)&0x2)!=0x2) // mdelay(get mutex0 
{
  mdelay(500); 
}
//clc_notice("spm_show_lcm_image() 05.\n");

//---------------------------------------------------
// set disp_config
//---------------------------------------------------
spm_write(disp_base+0x00054 ,  0x00000000); // write 0014
spm_write(disp_base+0x00024 ,  0x00000002); // write 0015
spm_write(disp_base+0x00034 ,  rdma0_out_sel); // write 0016
spm_write(disp_base+0x0005c ,  0x00000000); // write 0017
//---------------------------------------------------
// set disp_ovl
//---------------------------------------------------
spm_write(disp_base+0x03020 ,  (h<<0x10)+w); // write 0018
spm_write(disp_base+0x03028 ,  0x881b79c3); // write 0019
spm_write(disp_base+0x0302c ,  0x00000001); // write 0020
spm_write(disp_base+0x03024 ,  0x01e00000); // write 0021
spm_write(disp_base+0x03030 ,  0x00000087); // write 0022
spm_write(disp_base+0x03034 ,  0x5555aaaa); // write 0023
spm_write(disp_base+0x03038 ,  (h<<0x10)+w); // write 0024
spm_write(disp_base+0x0303c ,  0x00000000); // write 0025
//spm_write(disp_base+0x03040 ,  0x81100000 // write 0026
spm_write(disp_base+0x03040 ,  pic0_addr+0x36); //
//spm_write(disp_base+0x03044 ,  0x000005a0 // write 0027
spm_write(disp_base+0x03044 ,  pitch); // write 0027
spm_write(disp_base+0x030c0 ,  0x00000001); // write 0046
spm_write(disp_base+0x030e0 ,  0x00000001); // write 0047
spm_write(disp_base+0x03100 ,  0x00000001); // write 0048
spm_write(disp_base+0x03120 ,  0x00000001); // write 0049
spm_write(disp_base+0x0300c ,  0x00000001); // write 0050
//---------------------------------------------------
// set disp_bls
//---------------------------------------------------
spm_write(disp_base+0x08000 ,  0x00000000); // write 0070
//---------------------------------------------------
// set disp_rdma0
//---------------------------------------------------
spm_write(disp_base+0x06010 ,  0x00000100); // write 0073
spm_write(disp_base+0x06014 ,  w); // write 0074
spm_write(disp_base+0x06018 ,  h); // write 0075
spm_write(disp_base+0x06024 ,  0x00000080); // write 0076
spm_write(disp_base+0x06028 ,  0x00000000); // write 0077
spm_write(disp_base+0x0602c ,  0x00000000); // write 0078
//spm_write(disp_base+0x06030 ,  0x10101010); // write 0079
spm_write(disp_base+0x06030 ,  0x50303040); // write 0079
spm_write(disp_base+0x06038 ,  0x00000020); // write 0080
spm_write(disp_base+0x06040 ,  0x00f00008); // write 0081
spm_write(disp_base+0x06040 ,  0x80f00008); // write 0084
spm_write(disp_base+0x06010 ,  0x00000101); // write 0087
//---------------------------------------------------
// release disp_mutex
//---------------------------------------------------
spm_write(disp_base+0x11024 ,  0x00000000); // write 0088

/* Un-Mask ARM i bit */
//asm volatile("cpsie i @ arch_local_irq_enable" : : : "memory", "cc"); // clear i bit to enable interrupt
local_irq_restore(flags);
        
//clc_notice("spm_show_lcm_image() end.\n");
#endif
}
extern spinlock_t spm_lock;

//u32 g_spm_pc = 0;
//extern u32 cpu_power_down_cnt;
#ifndef SPM_MCDI_FUNC
static unsigned int mcidle_gpt_percpu[NR_CPUS] = {
    NR_GPTS,
    GPT4,//Core1

};
unsigned int mcidle_timer_left[NR_CPUS];
unsigned int mcidle_timer_left2[NR_CPUS];
#endif
extern unsigned long localtimer_get_counter(void);
extern int localtimer_set_next_event(unsigned long evt);

void spm_mcdi_before_wfi(int cpu)
{
    unsigned int id = mcidle_gpt_percpu[cpu];
    if (cpu != 0) {
        mcidle_timer_left2[cpu] = localtimer_get_counter(); 
#ifdef SPM_SUSPEND_GPT_EN
        err = request_gpt(id, GPT_ONE_SHOT, GPT_CLK_SRC_SYS, GPT_CLK_DIV_1, 
                    0, NULL, GPT_NOAUTOEN);
        if (err) {
            idle_info("[%s]fail to request GPT4\n", __func__);
        }
#endif        
        gpt_set_cmp(id, mcidle_timer_left2[cpu]);
        start_gpt(id);
    }
}

void spm_mcdi_after_wfi(int cpu)
{
    unsigned int id = mcidle_gpt_percpu[cpu];
    if (cpu != 0) {
        if (gpt_check_and_ack_irq(id)) {
            localtimer_set_next_event(1);
        } else {
            /* waked up by other wakeup source */
            unsigned int cnt, cmp;
            gpt_get_cnt(id, &cnt);
            gpt_get_cmp(id, &cmp);
            if (unlikely(cmp < cnt)) {
                printk("[%s]GPT%d: counter = %10u, compare = %10u\n", __func__, 
                        id + 1, cnt, cmp);
                BUG();
            }
        
            localtimer_set_next_event(cmp-cnt);
            stop_gpt(id);
#ifdef SPM_SUSPEND_GPT_EN
            free_gpt(id);
#endif
        }
    }

}

int spm_wfi_for_mcdi_test(void *mcdi_data)
{   
    volatile u32 do_not_change_it;
    volatile u32 lo, hi, core_id;
    unsigned long flags;

    preempt_disable();
    do_not_change_it = 1;
    MCDI_Test_Mode = 1;

    while(do_not_change_it)     
    {
        /* Mask ARM i bit */
        local_irq_save(flags);
    
        core_id = (u32)smp_processor_id();

        // set local timer & GPT =========================================
        switch (core_id)
        {
            case 0 : 


                #if 0
                /*trigger pcm timer*/
                spm_write(SPM_POWER_ON_VAL1,(spm_read(SPM_POWER_ON_VAL1)&0xFFFFFFCF)|0x220);
                spm_write(SPM_PCM_PWR_IO_EN,0x00800000);
                spm_write(SPM_PCM_PWR_IO_EN,0x00000000);
                #else
                read_cntp_cval(lo, hi);
                lo+=26000; // 100 ms, 13MHz
                //lo+=5070000; // 390 ms, 13MHz
                write_cntp_cval(lo, hi);
                write_cntp_ctl(0x1);  // CNTP_CTL_ENABLE      
                //printk("mcdi pdn cnt:%d\n",cpu_power_down_cnt);
                #endif


                
            break;       
            
            case 1 : 
                #if 0
                //gpt_set_cmp(GPT4, 2470000); // 190ms, 13MHz
                //printk("mcdi pdn cnt:%d\n",cpu_power_down_cnt);
                gpt_set_cmp(GPT4, 130000); // 10ms, 13MHz
                start_gpt(GPT4);   
                #endif
                read_cntp_cval(lo, hi);
                lo+=26000; // 100 ms, 13MHz
                //lo+=5070000; // 390 ms, 13MHz
                write_cntp_cval(lo, hi);
                write_cntp_ctl(0x1);  // CNTP_CTL_ENABLE   

                spm_mcdi_before_wfi(core_id);          
                

                
            break;          
    
            
            default : break;
        }

        spm_mcdi_wfi();
        
        if(core_id==1)
            spm_mcdi_after_wfi(core_id);
        
        /* Un-Mask ARM i bit */
        local_irq_restore(flags);
    }
    
    preempt_enable();
    return 0;

}     


void spm_mcdi_LDVT_mcdi(void)
{
        clc_notice("spm_mcdi_LDVT_mcdi() start.\n");
        mtk_wdt_suspend();    
    
        // spm_direct_disable_sodi ============================
        spm_direct_disable_sodi();    
    
#if 1
    {
        u32 mcdi_error;
    
        // init GPT ==================================
        free_gpt(GPT4);
        
        mcdi_error = request_gpt(GPT4, GPT_ONE_SHOT, GPT_CLK_SRC_SYS, GPT_CLK_DIV_1, 0, NULL, GPT_NOAUTOEN);
        if(mcdi_error != 0)
        {
            clc_notice("GPT4 init failed.(0x%x)\n", mcdi_error);
        }
        

    }
#else
        // init GPT ==================================
        //spm_write(0xf0008010, 0x2);  //clear GPT1 count
        //spm_write(0xf0008040, 0x2);  //clear GPT4 count
        //spm_write(0xf0008050, 0x2);  //clear GPT5 count
    
        //spm_write(0xf0008000, (spm_read(0xf0008000) | 0x19));  //enable GPT1, 4, 5 IRQ
#endif
    
#if 0
        smp_call_function(spm_mcdi_wfi_for_test, NULL, 0);
        spm_mcdi_wfi_for_test();
#else
        mcdi_task_0 = kthread_create(spm_wfi_for_mcdi_test, NULL, "mcdi_task_0");
        mcdi_task_1 = kthread_create(spm_wfi_for_mcdi_test, NULL, "mcdi_task_1");
    

        if(IS_ERR(mcdi_task_0)||IS_ERR(mcdi_task_1) )
        {  
             clc_notice("Unable to start kernel thread(0x%x, 0x%x)./n", (u32)IS_ERR(mcdi_task_0), (u32)IS_ERR(mcdi_task_1));  
             mcdi_task_0 = NULL;
             mcdi_task_1 = NULL;
        }  
    
        kthread_bind(mcdi_task_0, 0);
        kthread_bind(mcdi_task_1, 1);

    
        wake_up_process(mcdi_task_0);
        wake_up_process(mcdi_task_1);

#endif

}


int spm_wfi_for_sodi_test(void *sodi_data)
{   
    volatile u32 do_not_change_it;
    volatile u32 lo, hi, core_id;
    unsigned long flags;
    //u32 temp_address;

    preempt_disable();
    do_not_change_it = 1;
    MCDI_Test_Mode = 1;
    
    while(do_not_change_it)     
    {
        /* Mask ARM i bit */
        local_irq_save(flags);
    
        core_id = (u32)smp_processor_id();

        // set local timer & GPT =========================================
        switch (core_id)
        {
            case 0 : 
                read_cntp_cval(lo, hi);
                hi+=0xffffffff; // very very long
                write_cntp_cval(lo, hi);
                write_cntp_ctl(0x1);  // CNTP_CTL_ENABLE
            break;       
            
            case 1 : 
                stop_gpt(GPT4); // disable GPT
            break;         
    
            
            default : break;
        }    

        spm_mcdi_wfi();

        /* Un-Mask ARM i bit */
        local_irq_restore(flags);
    }

    preempt_enable();
    return 0;

}



void spm_mcdi_LDVT_sodi(void)
{
    clc_notice("spm_mcdi_LDVT_sodi() start.\n");
    mtk_wdt_suspend();    

    // show image on screen ============================
    spm_show_lcm_image();

    // spm_direct_enable_sodi ============================
    spm_direct_enable_sodi();    

    mcdi_task_0 = kthread_create(spm_wfi_for_sodi_test, NULL, "mcdi_task_0");
    mcdi_task_1 = kthread_create(spm_wfi_for_sodi_test, NULL, "mcdi_task_1");

    if(IS_ERR(mcdi_task_0) ||IS_ERR(mcdi_task_1))
    {  
         clc_notice("Unable to start kernel thread(0x%x, 0x%x)./n", (u32)IS_ERR(mcdi_task_0), (u32)IS_ERR(mcdi_task_1));  
         mcdi_task_0 = NULL;
         mcdi_task_1 = NULL;
    }  

    kthread_bind(mcdi_task_0, 0);
    kthread_bind(mcdi_task_1, 1);

    wake_up_process(mcdi_task_0);
    wake_up_process(mcdi_task_1);
    
    clc_notice("spm_mcdi_LDVT_01() end.\n");

}

