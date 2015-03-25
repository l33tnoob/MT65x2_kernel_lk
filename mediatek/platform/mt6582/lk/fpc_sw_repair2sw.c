//#include <platform/project.h>
#include <platform/mt_reg_base.h>
#include <platform/mt_typedefs.h>
#include <platform/mtk_wdt.h>
#include <platform/mt_gpt.h>
#include <printf.h>

/*Integrate project.h*/
#define POWRON_CONFIG_EN        ((UINT32P)(SLEEP_BASE+0x000))
#define SLEEP_PWR_STA   ((UINT32P)(SLEEP_BASE+0x60C))
#define SLEEP_PWR_STAS  ((UINT32P)(SLEEP_BASE+0x610))
#define CKSYS_BASE (0x10000000)
#define CLK_CFG_0       ((UINT32P)(CKSYS_BASE+0x040))
#define CLK_CFG_0_SET   ((UINT32P)(CKSYS_BASE+0x044))
#define CLK_CFG_0_CLR   ((UINT32P)(CKSYS_BASE+0x048))
#define CLK_CFG_1       ((UINT32P)(CKSYS_BASE+0x050))
#define CLK_CFG_1_SET   ((UINT32P)(CKSYS_BASE+0x054))
#define CLK_CFG_1_CLR   ((UINT32P)(CKSYS_BASE+0x058))
#define CLK_CFG_2       ((UINT32P)(CKSYS_BASE+0x060))
#define CLK_CFG_2_SET   ((UINT32P)(CKSYS_BASE+0x064))
#define CLK_CFG_2_CLR   ((UINT32P)(CKSYS_BASE+0x068))
#define CLK_CFG_3       ((UINT32P)(CKSYS_BASE+0x070))
#define CLK_CFG_3_SET   ((UINT32P)(CKSYS_BASE+0x074))
#define CLK_CFG_3_CLR   ((UINT32P)(CKSYS_BASE+0x078))
#define CLK_CFG_4       ((UINT32P)(CKSYS_BASE+0x080))
#define CLK_CFG_4_SET   ((UINT32P)(CKSYS_BASE+0x084))
#define CLK_CFG_4_CLR   ((UINT32P)(CKSYS_BASE+0x088))
#define CKSTA_REG       ((UINT32P)(CKSYS_BASE+0x22C))


//#include <platform/mt_gpt.h>
/* will be placed in mediatek/platform/mt6582/lk/platform.c */
int repair_sram(void)
{
	// --------------------------------------------------
	// common
	// --------------------------------------------------

	unsigned int pwr_ack_status, pwr_acks_status;
	int ret = 0;
	UINT32 rdata_cksw0;
	UINT32 rdata_cksw1;
	UINT32 rdata_cksw2;
	UINT32 rdata_cksw3;
	UINT32 rdata_cksw4;
#if 0
printf("=================================Before Repair SRAM===========================\n");
printf("Repair SRAM 0520 v3-do load_fuse-2000 and Clock switch Enable and add PLL detail INFO\n");
printf("[Repair SRAM Check] Check MTCMOS Before Repair SRAM: START\n");
printf("[Repair SRAM Check] MFG_PWR_CON:0x10006214=0x%x\n",*((UINT32P) (0x10006214)));
printf("[Repair SRAM Check] ISP_PWR_CON:0x10006238=0x%x\n",*((UINT32P) (0x10006238)));
printf("[Repair SRAM Check] DIS_PWR_CON:0x1000623C=0x%x\n",*((UINT32P) (0x1000623C)));
printf("[Repair SRAM Check] MD_PWR_CON:0x10006284=0x%x\n",*((UINT32P) (0x10006284)));
printf("[Repair SRAM Check] PWR_STATUS:0x1000660C=0x%x\n",*((UINT32P) (0x1000660C)));
printf("[Repair SRAM Check] PWR_STATUS_S:0x10006610=0x%x\n",*((UINT32P) (0x10006610)));
printf("[Repair SRAM Check] Check MTCMOS: END\n");

printf("[Repair SRAM Check] Check PLL Before Repair SRAM: START\n");
printf("[Repair SRAM Check] AP_PLL_CON1:0x10209004=0x%x\n",*((UINT32P) (0x10209004)));
printf("[Repair SRAM Check] MAINPLL_CON1:0x10209214=0x%x\n",*((UINT32P) (0x10209214)));
printf("[Repair SRAM Check] MMPLL_CON1:0x0x10209234=0x%x\n",*((UINT32P) (0x10209234)));
printf("[Repair SRAM Check] VENCPLL_CON1:0x1000F804=0x%x\n",*((UINT32P) (0x1000F804)));
printf("[Repair SRAM Check] MSDCPLL_CON1:0x10209244=0x%x\n",*((UINT32P) (0x10209244)));
printf("[Repair SRAM Check] VENCPLL_CON0:0x1000F800=0x%x\n",*((UINT32P) (0x1000F800)));
printf("[Repair SRAM Check] VENCPLL_CON1:0x1000F804=0x%x\n",*((UINT32P) (0x1000F804)));
printf("[Repair SRAM Check] VENCPLL_CON2:0x1000F808=0x%x\n",*((UINT32P) (0x1000F808)));
printf("[Repair SRAM Check] VENCPLL_PWR_CON0:0x1000F80C=0x%x\n",*((UINT32P) (0x1000F80C)));

printf("[Repair SRAM Check] *CLK_CFG_0=0x%x\n",*CLK_CFG_0);
printf("[Repair SRAM Check] *CLK_CFG_1=0x%x\n",*CLK_CFG_1);
printf("[Repair SRAM Check] *CLK_CFG_2=0x%x\n",*CLK_CFG_2);
printf("[Repair SRAM Check] *CLK_CFG_3=0x%x\n",*CLK_CFG_3);
printf("[Repair SRAM Check] *CLK_CFG_4=0x%x\n",*CLK_CFG_4);

printf("[Repair SRAM Check] Check PLL  END\n");
printf("[Repair SRAM Check] VProc Monitor Before Repair SRAM START\n");
ret=pmic_read_interface(0x21E,&reg_val,0xFFFF,0);
printf("Reg[0x21E]=0x%x, %d\n", reg_val, ret);
ret=pmic_read_interface(0x220,&reg_val,0xFFFF,0);
printf("Reg[0x220]=0x%x, %d\n", reg_val, ret);
ret=pmic_read_interface(0x222,&reg_val,0xFFFF,0);
printf("Reg[0x222]=0x%x, %d\n", reg_val, ret);
ret=pmic_read_interface(0x216,&reg_val,0xFFFF,0);
printf("Reg[0x216]=0x%x, %d\n", reg_val, ret);
ret=pmic_read_interface(0x224,&reg_val,0xFFFF,0);
printf("Reg[0x224]=0x%x, %d\n", reg_val, ret);

printf("[Repair SRAM Check] VProc Monitor End\n");
printf("\n");
printf("=======================================END===============================\n");
#endif

	/*TINFO="instruction code " */
	// --------------------------------------------------
	// Turn on MDMCU
	// --------------------------------------------------


//  *ACLKEN_DIV = 0x12; //div2
//  *PCLKEN_DIV = 0x14; //div4
//  *MEM_PWR_CTRL = 0x3; //slpb_dly and mem_off_dly
//  //Enable MCUSYS command queue
//  *MCU_BIU_CON |= 0x1; //enable out-of-order queue 
//  *MCU_BIU_CON |= 0x1000;
//  *CA7_MISC_CONFIG |= 0x200;

///1. set MTCMOS
// --------------------------------------------------
	*POWRON_CONFIG_EN = 0x0B160001;	//spm power code
	*((UINT32P) (0x10006214)) = 0x00000f16;	//MFG MTCOMS 
	*((UINT32P) (0x10006214)) = 0x00000f1e;	//MFG MTCOMS
	udelay(10);
        pwr_ack_status = (*SLEEP_PWR_STA & 0x00000010) >> 4;    //[04]
        pwr_acks_status = (*SLEEP_PWR_STAS & 0x00000010) >> 4;  //[04]
	if ((pwr_ack_status != 0x01) | (pwr_acks_status != 0x01)) {
		/* TINFO="Wait for Power Up MFGSys ..." */
		printf("Power Up MFGSys fail %d\n", __LINE__);
		ret = -1;
	}
       

	*((UINT32P) (0x10006214)) = 0x00000f0e;	//MFG MTCOMS
	*((UINT32P) (0x10006214)) = 0x00000f0c;	//MFG MTCOMS
	*((UINT32P) (0x10006214)) = 0x00000f0d;	//MFG MTCOMS
	*((UINT32P) (0x10006214)) = 0x00000e0d;	//MFG MTCOMS
	*((UINT32P) (0x10006214)) = 0x00000c0d;	//MFG MTCOMS
	*((UINT32P) (0x10006214)) = 0x0000080d;	//MFG MTCOMS
	*((UINT32P) (0x10006214)) = 0x0000000d;	//MFG MTCOMS


        
	*((UINT32P) (0x10006238)) = 0x00000f16;	//ISP MTCOMS 
	*((UINT32P) (0x10006238)) = 0x00000f1e;	//ISP MTCOMS
	udelay(10);
        pwr_ack_status = (*SLEEP_PWR_STA & 0x00000020) >> 5;    //[05]
        pwr_acks_status = (*SLEEP_PWR_STAS & 0x00000020) >> 5;  //[05]
	if ((pwr_ack_status != 0x01) | (pwr_acks_status != 0x01)) {
		/* TINFO="Wait for Power On ISPSys ..." */
		printf("Power Up ISPSys fail %d\n", __LINE__);
		ret = -1;
	}
	*((UINT32P) (0x10006238)) = 0x00000f0e;	//ISP MTCOMS
	*((UINT32P) (0x10006238)) = 0x00000f0c;	//ISP MTCOMS
	*((UINT32P) (0x10006238)) = 0x00000f0d;	//ISP MTCOMS
	*((UINT32P) (0x10006238)) = 0x00000e0d;	//ISP MTCOMS
	*((UINT32P) (0x10006238)) = 0x00000c0d;	//ISP MTCOMS
	*((UINT32P) (0x10006238)) = 0x0000080d;	//ISP MTCOMS
	*((UINT32P) (0x10006238)) = 0x0000000d;	//ISP MTCOMS

#if 0 //DISP already opened in preloader
	*((UINT32P) (0x1000623c)) = 0x00000f16;	//DISP MTCOMS 
	*((UINT32P) (0x1000623c)) = 0x00000f1e;	//DISP MTCOMS
	udelay(10);
        pwr_ack_status = (*SLEEP_PWR_STA & 0x00000008) >> 3;    //[03]
        pwr_acks_status = (*SLEEP_PWR_STAS & 0x00000008) >> 3;  //[03]

	if ((pwr_ack_status != 0x01) | (pwr_acks_status != 0x01)) {
		/* TINFO="Wait for Power Up DisSys ..." */
		printf("Power Up DISPSys fail %d\n", __LINE__);
		ret = -1;
	}

	*((UINT32P) (0x1000623c)) = 0x00000f0e;	//DISP MTCOMS
	*((UINT32P) (0x1000623c)) = 0x00000f0c;	//DISP MTCOMS
	*((UINT32P) (0x1000623c)) = 0x00000f0d;	//DISP MTCOMS
	*((UINT32P) (0x1000623c)) = 0x00000e0d;	//DISP MTCOMS
	*((UINT32P) (0x1000623c)) = 0x00000c0d;	//DISP MTCOMS
	*((UINT32P) (0x1000623c)) = 0x0000080d;	//DISP MTCOMS
	*((UINT32P) (0x1000623c)) = 0x0000000d;	//DISP MTCOMS

#endif

//enable MD MTCOM
if( (*((UINT32P) (0x10206040)) & 0x00400000) == 0x0) {     //MD efuse not disable
    if( (*((UINT32P) (0x10206044)) & 0x00000080) == 0x0) {   //TDD efuse not disable
	*((UINT16P) (0x2400001e)) = 0x000d;	// tddsys mbist_mem_clk_en
    } //TDD efuse not disable
// --------------------------------------------------
/* TINFO=" rdata = %h", *((UINT32P)(0x23010800)) */
	*((UINT32P) (0x23010800)) = 0xa244;	//md2g MODEM2G_TOPSM_RM_PWR_CON0
/* TINFO=" rdata = %h", *((UINT32P)(0x23010804)) */
	*((UINT32P) (0x23010804)) = 0xa244;	//md_hspa1 MODEM2G_TOPSM_RM_PWR_CON1
/* TINFO=" rdata = %h", *((UINT32P)(0x2301080c)) */
	*((UINT32P) (0x2301080c)) = 0xa244;	//md_hspa3 MODEM2G_TOPSM_RM_PWR_CON3
/* TINFO=" rdata = %h", *((UINT32P)(0x23010810)) */
	*((UINT32P) (0x23010810)) = 0xa244;	//md_hspa4 MODEM2G_TOPSM_RM_PWR_CON4
//md_top
/* TINFO=" rdata = %h", *((UINT32P)(0x20030018)) */
	*((UINT32P) (0x20030018)) = 0x0;	//MD_TOPSM_RM_TMR_PWR0
/* TINFO=" rdata = %h", *((UINT32P)(0x23010018)) */
	*((UINT32P) (0x23010018)) = 0x0;	//MODEM2G_TOPSM_RM_TMR_PWR0
} //MD efuse not disable
////2. enable PLL
//// --------------------------------------------------
////2.1 enable PLL power
//*MMPLL_PWR_CON0   |= 0x1;
////wait 5us
///* TINFO="wait 5u"*/
//for(i=0;i<4;i=i+1){
//j=0;
//}
////2.2 release PLL ISO
//*MMPLL_PWR_CON0   &= 0xfffffffd;
////2.3 setting PLL frequency
//ck swithc initial setting recored
	rdata_cksw0 = *CLK_CFG_0;
	rdata_cksw1 = *CLK_CFG_1;
	rdata_cksw2 = *CLK_CFG_2;
	rdata_cksw3 = *CLK_CFG_3;
	rdata_cksw4 = *CLK_CFG_4;
//clock switch of MM --> 26M
//
//*MMPLL_CON1   = 0x80134000; //2002/4=500.5M
//*MMPLL_CON0   = 0x120; //2002/4=500.5M
////2.4 enable PLL
//*MMPLL_CON0   |= 0x1;
////wait 20us 
//for(i=0;i<9;i=i+1){
//j=0;
//}
//

	/*TINFO="Set this register before AXI clock switch to fast clock, APB/AHB" */
//!!!already set in pre-loader, 
//  *INFRA_TOPCKGEN_DCMCTL |= 0x1;                 
//
/*TINFO="cksys clock switch"*/
// ------------------------------------------------------------------
// open ckswitch power// clear power down

	*CLK_CFG_0_CLR = 0x80800080;	//MEMPLL don't change setting
	*CLK_CFG_1_CLR = 0x80808080;
	*CLK_CFG_2_CLR = 0x80808080;
	*CLK_CFG_3_CLR = 0x80808080;
	*CLK_CFG_4_CLR = 0x80808080;
// wait clock switch power
	udelay(1);
	if (((*CKSTA_REG) & 0xffffffff) != 0x0) {
		printf("Clock switch failed %d\n", __LINE__);
		ret = -1;
	}

	*CLK_CFG_0 = 0x01010001 | (rdata_cksw0 & 0x0000FF00);	//for MEMPLL  //mem clock[8] not switch or will let dram hang(w/o set PLL clock source)
	*CLK_CFG_1 = 0x02010103;
	*CLK_CFG_2 = 0x01010101;
	*CLK_CFG_3 = 0x01010101;
	*CLK_CFG_4 = 0x01010101;
 
printf("[CLK_CFG_4] *CLK_CFG_4=0x%x\n",*CLK_CFG_4);
// ------------------------------------------------------------------
//wait md2g_power[0] hspa power [1]
if( (*((UINT32P) (0x10206040)) & 0x00400000) == 0x0) {     //MD efuse not disable
	/*TINFO="check MD MTCMOS ready" */
	udelay(2000);
	if ((*((UINT32P) (0x23010820)) & 0x00000003) != 0x3) {
		printf("MD MTCMOS is not ready. %d\n", __LINE__);
		ret = -1;
	}
//wait tdd power
	udelay(500);
    if( (*((UINT32P) (0x10206044)) & 0x00000080) == 0x0) {   //TDD efuse not disable
	if (!(*((UINT16P) (0x24000002)) & 0x2)) {
		printf("tdd power is not ready %d\n", __LINE__);
		ret = -1;
	}
    }
//enable MD PLL
	/*TINFO="enable MD PLL" */
	*((UINT32P) (0x201200ac)) = 0x3f;	//PLL_DFS_CON7, bit 0~5 set to 1 // [FORCE ON] Bit 5: SYSCLK, Bit 4: MDPLL, Bit 3: WHPLL, Bit 2: WPLL, Bit 1: MCUPLL, Bit 0: No use (HW limit when boot)
	*((UINT32P) (0x2012004c)) = 0x8300;	//PLL_PLL_CON3, bit 12,4 set to 0// [POWER ON] Bit15: MCUPLL, Bit 8: WPLL
	/*TINFO="wait 1us check" */
	udelay(1);
	*((UINT32P) (0x2012004c)) = 0x0;	// bit 15,8,1,0 set to 0
	*((UINT32P) (0x20120048)) = 0x0;	//PLL_PLL_CON2, bit 12,10, 8, 6, 2 set to 0// [TOPSM & SW CTRL] Bit12: MDPLL, Bit 8: MCUPLL, Bit 4: WPLL, Bit 0: WHPLL
	*((UINT32P) (0x20120700)) = 0x10;	//PLL_PLLTD_CON0, bit 0 set to 0// [FHCTL & SW CTRL] Bit 4: CHG_CTRL
	/*TINFO="wait 3us check" */
	udelay(3);
	/* TINFO="Enable PLLs" */
	*((UINT32P) (0x20120100)) = 0x410f | 0x8000;	// Enable PLLs, PLL_MDPLL_CON0
	/*TINFO="wait 100us check" */
	udelay(100);
	*((UINT32P) (0x20120100)) &= 0x7fff;	// Enable PLLs, PLL_MDPLL_CON0
	/*TINFO="wait 30ns check" */
	udelay(1);
	*((UINT32P) (0x20120140)) = 0x0810 | 0x8000;	// Enable PLLs, PLL_MCUPLL_CON0
	*((UINT32P) (0x201201c0)) = 0x0800 | 0x8000;	// Enable PLLs, PLL_WPLL_CON0
	*((UINT32P) (0x20120200)) = 0x0500 | 0x8000;	// Enable PLLs, PLL_WHPLL_CON0
	/*TINFO="wait 100us check" */
	udelay(100);
	/* TINFO="Disable MDPLL and MDPLL2 AUTOK" */
	*((UINT32P) (0x20120110)) &= 0xfffffffe;	// PLL_MDPLL_CON4, bit0 set to 0
	/*TINFO="wait 2us check" */
	udelay(2);
	*((UINT32P) (0x20120100)) = 0x410f | 0x8000;	// Enable PLLs, PLL_MDPLL_CON0
	/* TINFO="wait 20u"*/
	udelay(20);
// ------------------------------------------------------------------
	//SWITCH PLL clock
	//switch CR4 clock to 481M
	/* TINFO="Clock Switch Setting" */
/* TINFO=" rdata = %h", *((UINT32P)(0x2000045c)) */
	*((UINT32P) (0x2000045c)) |= 0x20000000;	//MD_GLOBAL_CON1, BUS_CLK = PLL Freq (not 26MHz)
/* TINFO=" rdata = %h", *((UINT32P)(0x20120060)) */
	*((UINT32P) (0x20120060)) = 0x2020;	//PLL_CLKSW_CKSEL0, Bit 15-12: MDMCU_CLK = MCUPLL 481MHz, Bit  7- 4: DSP_CLK = MCUPLL DIV3 = 481 MHz / 2 = 240.5 MHz  
/* TINFO=" rdata = %h", *((UINT32P)(0x20120064)) */
	*((UINT32P) (0x20120064)) = 0x2000;	//PLL_CLKSW_CKSEL1, Bit 15-12: BUS_CLK   = MCUPLL DIV2 = 481   MHz / 4 = 120.25MHz 
/* TINFO=" rdata = %h", *((UINT32P)(0x20120068)) */
	*((UINT32P) (0x20120068)) = 0x2240;	//PLL_CLKSW_CKSEL2, Bit 15-12: FX64W_CLK = WPLL = 245.76MHz, Bit 11- 8: FX16G_CLK = MDPLL  DIV3 = 416 MHz / 2 = 208 MHz, Bit 7- 4: HW64W_CLK = WHPLL = 250.25MHz
/* TINFO=" rdata = %h", *((UINT32P)(0x22c00040)) */
	*((UINT32P) (0x22c00040)) = 0x41f041f0;	// MD2GSYS_clock switch

}  // MD efuse not disable
//===========MD PLL setting end
//

// ------------------------------------------------------------------
//DCM
	*((UINT32P) (0x13000010)) = 0x0;	//disable mfg DCM [15]
// ------------------------------------------------------------------
//clock enable
	/* TINFO="clock enable" */
/* TINFO=" rdata = %h", *((UINT32P)(0x14000108)) */
	*((UINT32P) (0x14000108)) = 0xffffffff;	//turn mmsys clock
/* TINFO=" rdata = %h", *((UINT32P)(0x14000118)) */
	*((UINT32P) (0x14000118)) = 0xffffffff;	//turn mmsys clock
/* TINFO=" rdata = %h", *((UINT32P)(0x14000118)) */
	*((UINT32P) (0x15000008)) = 0xffffffff;	//turn img clock
/* TINFO=" rdata = %h", *((UINT32P)(0x14000118)) */
	*((UINT32P) (0x15004150)) = 0x0000ffff;	//turn img clock
// ------------------------------------------------------------------
if( (*((UINT32P) (0x10206040)) & 0x00400000) == 0x0) {     //MD efuse not disable
	*((UINT32P) (0x20000458)) = 0xffffffff;	//turn md infrasys clock
	*((UINT32P) (0x23000010)) = 0xffffffff;	//turn md modem clock
	*((UINT32P) (0x23000018)) = 0xffffffff;	//turn md modem clock
	*((UINT32P) (0x23000098)) = 0xffffffff;	//turn md modem clock
    if( (*((UINT32P) (0x10206044)) & 0x00000001) == 0x0) {   //HSPA efuse not disable
	*((UINT32P) (0x2367002c)) = 0xffffffff;	//turn md hspa3 clock
	*((UINT32P) (0x23670010)) = 0x00000000;	//turn md hspa3 clock
    }
    if( (*((UINT32P) (0x10206044)) & 0x00000080) == 0x0) {   //TDD efuse not disable
	*((UINT16P) (0x24000422)) = 0x4911;	//turn on tddsys clock
	*((UINT16P) (0x24000422)) = 0xc911;	//turn on tddsys clock 
    }
}  //MD efuse not disable




  
// ------------------------------------------------------------------
//printf("[Repair SRAM Check ] Start 1nd MBIST\n");
//=============================================
//set mbist_rstb & rp_rstb
	/* TINFO="set mbist_rstb" */
	*((UINT32P) (0x13000100)) = 0x00000000;	// [0] mfg rp_rstb=0
	*((UINT32P) (0x13000060)) = 0x00000000;	// set mfgsys mbist_rstb, [31]:mbist_rstb
	*((UINT32P) (0x14000850)) = 0x00000000;	// set mm_mdp [0] rp_rstb
	*((UINT32P) (0x1400081C)) = 0x00000000;	// set mmsys mbist_rstb, [15]: mbist_rstb [31:16]: mm background 
//*((UINT32P)(0x150001a0)) = 0x00000000; // set imgsys [0]: rp_rstb
	*((UINT32P) (0x15000070)) = 0x00000000;	// set imgsys mbist_rstb, [0]: mbist_rstb
	*((UINT32P) (0x150001A0)) = 0x00000000;	// set imgsys rp_rst, [0] henry add
if( (*((UINT32P) (0x10206040)) & 0x00400000) == 0x0) {     //MD efuse not disable
	*((UINT32P) (0x200e0044)) = 0x00000000;	// set mdsys mbist_rstb, [0]: mbist_rstb, [1]:repair reset [2]:reg_load_fuse
//*((UINT32P)(0x23008040)) = 0x00000000; // set modemsys mbist_rstb, [0]: mbist_rstb
//*((UINT32P)(0x236e008c)) = 0x00000000; // set hspa3sys  [0]: rp_rstb
    if( (*((UINT32P) (0x10206044)) & 0x00000001) == 0x0) {   //HSPA efuse not disable
        *((UINT32P)(0x236e008c)) = 0x00000000;  // set hspa3sys  [0]: rp_rstb henry add
	*((UINT32P)(0x236e007c)) = 0x00000000;	// set hspa3sys mbist_rst, [0]: mbist_rstbb
    }
    if( (*((UINT32P) (0x10206044)) & 0x00000080) == 0x0) {   //HSPA efuse not disabl
	*((UINT32P) (0x24050068)) = 0x00000000;	// set tddsys mbist_rst, [0]: mbist_rstbb
    }
}//MD efuse not disable
// ------------------------------------------------------------------
//release mbist_rstb & rp_rstb
	/* TINFO="release mbist_rstb" */
	*((UINT32P) (0x13000100)) = 0x00000001;	// [0] mfg rp_rstb=1
	*((UINT32P) (0x13000060)) = 0x80000000;	// release mfgsys mbist_rstb, [31]:mbist_rstb
	*((UINT32P) (0x14000850)) = 0x00000001;	// set mm_mdp 0 rp_rstb
	*((UINT32P) (0x1400081C)) = 0x00008000;	// release mmsys mbist_rstb, [15]: mbist_rstb [31:16]: mm background (TBD)
//*((UINT32P)(0x150001a0)) = 0x00000001; // set imgsys [0]: rp_rstb
	*((UINT32P) (0x15000070)) = 0x00000001;	// release imgsys mbist_rstb, [0]: mbist_rstb
	*((UINT32P) (0x150001A0)) = 0x00000001;	// set imgsys rp_rst, [0]  //add by henry
if( (*((UINT32P) (0x10206040)) & 0x00400000) == 0x0) {     //MD efuse not disable
	*((UINT32P) (0x200e0044)) = 0x00000003;	// release mdsys mbist_rstb, [0]: mbist_rstb, [1]:repair reset [2]:reg_load_fuse
//*((UINT32P)(0x23008040)) = 0x00000001; // release modemsys mbist_rstb, [0]: mbist_rstb
//*((UINT32P)(0x236e008c)) = 0x00000001; // set hspa3sys  [0]: rp_rstb
    if( (*((UINT32P) (0x10206044)) & 0x00000001) == 0x0) {   //HSPA efuse not disable
        *((UINT32P)(0x236e008c)) = 0x00000001;  // set hspa3sys  [0]: rp_rstb  //add by henry
	*((UINT32P)(0x236e007c)) = 0x00000001;	// release hspa3sys mbist_rst, [0]: mbist_rstbb
    }
    if( (*((UINT32P) (0x10206044)) & 0x00000080) == 0x0) {   //HSPA efuse not disabl
	*((UINT32P) (0x24050068)) = 0x00000001;	// release tddsys mbist_rst, [0]: mbist_rstbb
    }
}
// ------------------------------------------------------------------
//set mbist_mode=1
	/* TINFO="set mbist_mode" */
	*((UINT32P) (0x13000060)) = 0x80000002;	// [1] mbist_mode, [31]: mbist_rstb  mfg
	*((UINT32P) (0x14000810)) = 0x00000040;	// [6] mbist_mode  mm
	*((UINT32P) (0x15000074)) = 0x00001c00;	// [12:10] mbist_mode  img 
if( (*((UINT32P) (0x10206040)) & 0x00400000) == 0x0) {     //MD efuse not disable
	*((UINT32P) (0x200e0000)) = 0x00000ff6;	// [11:4,2:1] mbist_mode  md
    if( (*((UINT32P) (0x10206044)) & 0x00000001) == 0x0) {   //HSPA efuse not disable
	*((UINT32P) (0x236e0000)) = 0x00000080;	// [7] mbist_mode  md_hspa3
    }
    if( (*((UINT32P) (0x10206044)) & 0x00000080) == 0x0) {   //HSPA efuse not disabl
	*((UINT32P) (0x24050000)) = 0x3b438000;	// [29~27, 25~24,22,17~15] mbist_mode tdd
    }
}
// ------------------------------------------------------------------
//wait mbist_done
	udelay(2000);

/* TINFO="wait mbist_done" */
	if ((*((UINT32P) (0x13000090)) & 0x7fff8) != 0x7fff8) {	//mfg mbist_done
		printf("mfg mbist_done is not ready %d\n", __LINE__);
		ret = -1;
	}
/* TINFO="mfg mbist_done" */
  //      printf("[Repair SRAM Check ] Value check 1nd  MM:0x14000800=0x%x\n",*((UINT32P) (0x14000800)));
	if ((*((UINT32P) (0x14000800)) & 0x20f) != 0x20f) {	//mm mbist_done
		printf("mm mbist_done is not ready %d\n", __LINE__);
                printf("[Repair SRAM Check ] 1nd MM:0x14000800=0x%x\n",*((UINT32P) (0x14000800)));
                printf("[Repair SRAM Check ] 1nd MM:0x14000804=0x%x\n",*((UINT32P) (0x14000804)));
         	ret = -1;
	}
/* TINFO="mm mbist_done" */
    //    printf("[Repair SRAM Check] Value check 1nd IMG:0x15000038=0x%x\n",*((UINT32P) (0x15000038)));
	if ((*((UINT32P) (0x15000038)) & 0x1c40) != 0x1c40) {	//img mbist_done
		printf("img mbist_done is not ready %d\n", __LINE__);
                printf("[Repair SRAM Check] 1nd IMG:0x15000038=0x%x\n",*((UINT32P) (0x15000038)));
                printf("[Repair SRAM Check] 1nd IMG:0x15000048=0x%x\n",*((UINT32P) (0x15000048)));
                printf("[Repair SRAM Check] 1nd IMG:0x1500004c=0x%x\n",*((UINT32P) (0x1500004c)));
                printf("[fuse check] 1nd IMG:0x100011ec=0x%x\n",*((UINT32P) (0x100011ec)));
                printf("[img repair status] IMG:0x150001a4=0x%x\n",*((UINT32P) (0x150001a4)));
		ret = -1;
	}
/* TINFO="img mbist_done" */

if( (*((UINT32P) (0x10206040)) & 0x00400000) == 0x0) {     //MD efuse not disable
	if ((*((UINT32P) (0x200e0024)) & 0x1fec) != 0x1fec) {	//md mbist_done
		printf("md mbist_done is not ready %d\n", __LINE__);
		ret = -1;
	}
/* TINFO="mdmcu mbist_done" */
    if( (*((UINT32P) (0x10206044)) & 0x00000001) == 0x0) {   //HSPA efuse not disable
	if ((*((UINT32P) (0x236e002c)) & 0x4) != 0x4) {	//hspa3 mbist_done
		printf("hspa3 mbist_done is not ready %d\n", __LINE__);
		ret = -1;
	}
    }
/* TINFO="hspa3 mbist_done" */
    if( (*((UINT32P) (0x10206044)) & 0x00000080) == 0x0) {   //TDD efuse not disable
	if ((*((UINT32P) (0x2405001c)) & 0x76870000) != 0x76870000) {	//tdd mbist_done
		printf("tdd mbist_done is not ready %d\n", __LINE__);
		ret = -1;
	}
    }
/* TINFO="tdd mbist_done" */
}
#if 1
printf("==============================================After 1's MBIST===============================\n");
printf("[img repair status] IMG:0x150001a4=0x%x\n",*((UINT32P) (0x150001a4)));
printf("[Repair SRAM Check] 1nd MBIST TEST Check: START\n");
printf("[Repair SRAM Check] IMAGE:0x15000038=0x%x\n",*((UINT32P) (0x15000038)));
printf("[Repair SRAM Check] hspa3:0x236e002c=0x%x\n",*((UINT32P) (0x236e002c)));
printf("[Repair SRAM Check] efuse:0x10206040=0x%x\n",*((UINT32P) (0x10206040)));
printf("[Repair SRAM Check] efuse:0x10206044=0x%x\n",*((UINT32P) (0x10206044)));
printf("[Repair SRAM Check] TDD:0x2405001c=0x%x\n",*((UINT32P) (0x2405001c)));
printf("[Repair SRAM Check] MBIST TEST Check: END\n");
printf("\n");
printf("[Repair SRAM Check] After 1nd MBIST TEST: START\n");
printf("[Repair SRAM Check] MFG:0x130000a8=0x%x\n",*((UINT32P) (0x130000a8)));
printf("[Repair SRAM Check] MM:0x14000804=0x%x\n",*((UINT32P) (0x14000804)));
printf("[Repair SRAM Check] IMG:0x15000048=0x%x\n",*((UINT32P) (0x15000048)));
printf("[Repair SRAM Check] IMG:0x0x1500004c=0x%x\n",*((UINT32P) (0x1500004c)));
printf("[Repair SRAM Check] MD:0x200e0030=0x%x\n",*((UINT32P) (0x200e0030)));
printf("[Repair SRAM Check] MD:0x200e0034=0x%x\n",*((UINT32P) (0x200e0034)));
printf("[Repair SRAM Check] HSPA:0x236e0034=0x%x\n",*((UINT32P) (0x236e0034)));
printf("[Repair SRAM Check] HSPA:0x236e0038=0x%x\n",*((UINT32P) (0x236e0038)));
printf("[Repair SRAM Check] TDD:0x2405002c=0x%x\n",*((UINT32P) (0x2405002c)));
printf("[Repair SRAM Check] TDD:0x24050030=0x%x\n",*((UINT32P) (0x24050030)));
printf("[Repair SRAM Check] TDD:0x24050034=0x%x\n",*((UINT32P) (0x24050034)));
printf("\n");
printf("[Repair SRAM Check] VENCPLL Monitor START\n");
*((UINT32P) (0x10000220)) = 0x80;
*((UINT32P) (0x10000214)) = 0x0;
*((UINT32P) (0x10000100)) = 0x1600;
*((UINT32P) (0x10000220)) = 0x81;
udelay(500);
 printf("[Repair SRAM Check] CLK26CALI_0:0x1000_0220=0x%x\n",*((UINT32P) (0x10000220)));
 printf("[Repair SRAM Check] CLK26CALI_1:0x1000_0224=0x%x\n",*((UINT32P) (0x10000224)));
/*
 printf("[Repair SRAM Check] VENCPLL_CON0:0x1000F800=0x%x\n",*((UINT32P) (0x1000F800)));
 printf("[Repair SRAM Check] VENCPLL_CON1:0x1000F804=0x%x\n",*((UINT32P) (0x1000F804)));
 printf("[Repair SRAM Check] VENCPLL_CON2:0x1000F808=0x%x\n",*((UINT32P) (0x1000F808)));
 printf("[Repair SRAM Check] VENCPLL_PWR_CON0:0x1000F80C=0x%x\n",*((UINT32P) (0x1000F80C)));
*/
 printf("[Repair SRAM Check] VENCPLL Monitor End");
printf("\n");

printf("=================================================END=========================\n");
#endif

	/* TINFO="check mbist_fail" */
	/* TINFO="mfg mbist_fail" */
#if 0
// ------------------------------------------------------------------
//WHILE could change to IF, check correlation result
// ------------------------------------------------------------------
	if ((*((UINT32P) (0x130000a8)) & 0x0007fff8))	// [18:3]: mfgsys mbist_fail
		printf("mfgsys sram fail %d\n", __LINE__);
	/* TINFO="mm mbist_fail" */
	if ((*((UINT32P) (0x14000804)) & 0x00040000))	// [18] mm mbist_fail
		printf("mm sram fail %d\n", __LINE__);
	/* TINFO="img mbist_fail"     
	   [31:26]: img mbist_fail
	   [40, 37, 34:32]: mbist_fail */
	if ((*((UINT32P) (0x15000048)) & 0xfc000000)
	    || (*((UINT32P) (0x1500004c)) & 0x00000127))
		printf("img sram fail %d\n", __LINE__);
	/* TINFO="md mbist_fail"     
	   [28:26, 16:9] mbist_fail, [0]: mbist_fail_all  md
	   [28:0] mbist_fail */
	if (((*((UINT32P) (0x200e0030)) & 0x1c03fc01))
	    || ((*((UINT32P) (0x200e0034)) & 0x1fffffff)))
		printf("md sram fail %d\n", __LINE__);
	/* TINFO="hspa3 mbist_fail"
	   [31:29] mbist_fail hspa3
	   [5:0] mbist_fail */
	if (((*((UINT32P) (0x236e0034)) & 0xc0000000))
	    || ((*((UINT32P) (0x236e0038)) & 0x0000003f)))
		printf("hspa3 sram fail %d\n", __LINE__);
	/* TINFO="tdd mbist_fail"    
	   [16:11] mbist_fail tdd
	   [31:28, 26:23, 20, 19] mbist_fail
	   [1] mbist_fail */
	if (((*((UINT32P) (0x2405002c)) & 0x0001f800))
	    || ((*((UINT32P) (0x24050030)) & 0xf7980000))
	    || ((*((UINT32P) (0x24050034)) & 0x00000001)))
		printf("tdd sram fail %d\n", __LINE__);
	/* TINFO="check repair status" */
	/* TINFO="check mfg status" */
// ------------------------------------------------------------------
//WHILE could change to IF, correlation result
// ------------------------------------------------------------------
	if ((*((UINT32P) (0x13000104)) & 0xff000000))	//[31:24] rp_fail, [23:16] rp_ok, [6:0]: fuse , can't rp_fail, but rp_ok 1 or 0 is ok
		printf("mfg repair failed %d\n", __LINE__);
	if ((*((UINT32P) (0x13000108)) & 0xff000000))	//[31:24] rp_fail, [23:16] rp_ok, [6:0]: fuse ,can't rp_fail, but rp_ok 1 or 0 is ok
		printf("mfg repair failed %d\n", __LINE__);
	/* TINFO="check mm status" */
	if ((*((UINT32P) (0x14000854)) & 0x00000001))	//[0] mm rp_fail
		printf("mm repair failed %d\n", __LINE__);
	/* TINFO="check img status" */
	if ((*((UINT32P) (0x150001a4)) & 0x07ff0000))	// [26:16] imgsys rp_fail, [10:0] rp_ok 
		printf("img repair failed %d\n", __LINE__);
	/* TINFO="check md status" */
	if ((*((UINT32P) (0x200e0050)) & 0xffffffff))	// md rp_fail [31:0]
		printf("md repair failed %d\n", __LINE__);
	if ((*((UINT32P) (0x200e0054)) & 0x000000ff))	// md rp_fail [7:0]
		printf("md repair failed %d\n", __LINE__);
	/* TINFO="check hspa3 status" */
	if ((*((UINT32P) (0x236e0094)) & 0x000000ff))	// hspa3 rp_fail [7:0]
		printf("hspa3 repair failed %d\n", __LINE__);
	/* TINFO="check tdd status" */
	if ((*((UINT32P) (0x24050064)) & 0x0001ffff))	// tdd rp_fail [16:0]
		printf("tdd repair failed %d\n", __LINE__);
#endif

// ------------------------------------------------------------------
	/* TINFO="reg_load_fuse" */
#if 1 
	*((UINT32P) (0x100011a8)) = 0x000f0000;	//INFRA fuse load for mfg/img/mm

if( (*((UINT32P) (0x10206040)) & 0x00400000) == 0x0) {     //MD efuse not disable
	*((UINT32P) (0x200e0044)) = 0x00000002;	//MDMCU 0: mbist_rstb, 1: rp_rstb, 2: load_fuse
	*((UINT32P) (0x200e0044)) = 0x00000007;	// load fuse
    if( (*((UINT32P) (0x10206044)) & 0x00000001) == 0x0) {   //HSPA efuse not disable
	*((UINT32P) (0x236e00a8)) = 0x00000001;	//hspa3 0: load fuse 
    }
    if( (*((UINT32P) (0x10206044)) & 0x00000080) == 0x0) {   //TDD efuse not disable
	*((UINT16P) (0x2400001e)) = 0x004d;	//6: tdd load fuse
    }
}
#endif
// ------------------------------------------------------------------
	/* TINFO="clear mbist_mode" */
	*((UINT32P) (0x13000060)) = 0x00000000;	// [1] mbist_mode, [31]: mbist_rstb  mfg
	*((UINT32P) (0x14000810)) = 0x00000000;	// [6] mbist_mode  mm
	*((UINT32P) (0x15000074)) = 0x00000000;	// [12:10] mbist_mode  img 
if( (*((UINT32P) (0x10206040)) & 0x00400000) == 0x0) {     //MD efuse not disable
	*((UINT32P) (0x200e0000)) = 0x00000000;	// [11:4,2:1] mbist_mode  md
    if( (*((UINT32P) (0x10206044)) & 0x00000001) == 0x0) {   //HSPA efuse not disable
	*((UINT32P) (0x236e0000)) = 0x00000000;	// [7] mbist_mode  md_hspa3
    }
    if( (*((UINT32P) (0x10206044)) & 0x00000080) == 0x0) {   //TDD efuse not disable
	*((UINT32P) (0x24050000)) = 0x00000000;	// [29~27, 25~24,22,17~15] mbist_mode tdd
    }
}
//printf("[Repair SRAM Check ] Start 2nd MBIST\n");
// ------------------------------------------------------------------
	/* TINFO="re-set  mbist_rstb" */
	*((UINT32P) (0x13000060)) = 0x00000000;	// set mfgsys mbist_rstb, [31]:mbist_rstb
	*((UINT32P) (0x1400081C)) = 0x00000000;	// set mmsys mbist_rstb, [15]: mbist_rstb [31:16]: mm background 
	*((UINT32P) (0x15000070)) = 0x00000000;	// set imgsys mbist_rstb, [0]: mbist_rstb
if( (*((UINT32P) (0x10206040)) & 0x00400000) == 0x0) {     //MD efuse not disable
        *((UINT32P) (0x23008040)) = 0x00000000; // set modemsys mbist_rstb, [0]: mbist_rstb
    if( (*((UINT32P) (0x10206044)) & 0x00000001) == 0x0) {   //HSPA efuse not disable
	*((UINT32P) (0x236e007c)) = 0x00000000;	// set hspa3sys mbist_rst, [0]: mbist_rstbb
    }
    if( (*((UINT32P) (0x10206044)) & 0x00000080) == 0x0) {   //TDD efuse not disable
	*((UINT32P) (0x24050068)) = 0x00000000;	// set tddsys mbist_rst, [0]: mbist_rstbb
    }
}
// ------------------------------------------------------------------
//release mbist_rstb
	/* TINFO="release mbist_rstb" */
	*((UINT32P) (0x13000060)) = 0x80000000;	// release mfgsys mbist_rstb, [31]:mbist_rstb
	*((UINT32P) (0x13000100)) = 0x00000001;	// [0] mfg rp_rstb=1
	*((UINT32P) (0x1400081C)) = 0x00008000;	// release mmsys mbist_rstb, [15]: mbist_rstb [31:16]: mm background (TBD)
	*((UINT32P) (0x15000070)) = 0x00000001;	// release vencsys mbist_rstb, [0]: mbist_rstb
if( (*((UINT32P) (0x10206040)) & 0x00400000) == 0x0) {     //MD efuse not disable
        *((UINT32P) (0x23008040)) = 0x00000001; // release modemsys mbist_rstb, [0]: mbist_rstb
    if( (*((UINT32P) (0x10206044)) & 0x00000001) == 0x0) {   //HSPA efuse not disable
	*((UINT32P) (0x236e007c)) = 0x00000001;	// release hspa3sys mbist_rst, [0]: mbist_rstbb
    }
    if( (*((UINT32P) (0x10206044)) & 0x00000080) == 0x0) {   //TDD efuse not disable
	*((UINT32P) (0x24050068)) = 0x00000001;	// release tddsys mbist_rst, [0]: mbist_rstbb
    }
}
// ------------------------------------------------------------------
//set mbist_mode=1
	/* TINFO="set mbist_mode" */
	*((UINT32P) (0x13000060)) = 0x80000002;	// [1] mbist_mode, [31]: mbist_rstb  mfg
	*((UINT32P) (0x14000810)) = 0x00000040;	// [6] mbist_mode  mm
	*((UINT32P) (0x15000074)) = 0x00001c00;	// [12:10] mbist_mode  img 
if( (*((UINT32P) (0x10206040)) & 0x00400000) == 0x0) {     //MD efuse not disable
	*((UINT32P) (0x200e0000)) = 0x00000ff6;	// [11:4,2:1] mbist_mode  md
    if( (*((UINT32P) (0x10206044)) & 0x00000001) == 0x0) {   //HSPA efuse not disable
	*((UINT32P) (0x236e0000)) = 0x00000080;	// [7] mbist_mode  md_hspa3
    }
    if( (*((UINT32P) (0x10206044)) & 0x00000080) == 0x0) {   //TDD efuse not disable
	*((UINT32P) (0x24050000)) = 0x3b438000;	// [29~27, 25~24,22,17~15] mbist_mode tdd
    }
}

// ------------------------------------------------------------------
	udelay(2000);
	/* TINFO="wait mbist_done" */
	if ((*((UINT32P) (0x13000090)) & 0x7fff8) != 0x7fff8) {	//mfg mbist_done
		printf("mfg mbist_done is not ready %d\n", __LINE__);
		ret = -1;
	}
       // printf("[Repair SRAM Check ] Value check 1nd  MM:0x14000800=0x%x\n",*((UINT32P) (0x14000800)));
	/* TINFO="mfg mbist_done" */
	if ((*((UINT32P) (0x14000800)) & 0x20f) != 0x20f) {	//mm mbist_done
		printf("mm mbist_done is not ready %d\n", __LINE__);
                printf("[Repair SRAM Check ] 2nd MM:0x14000800=0x%x\n",*((UINT32P) (0x14000800)));
                printf("[Repair SRAM Check ] 2nd MM:0x14000804=0x%x\n",*((UINT32P) (0x14000804)));

		ret = -1;
	}
//        printf("[Repair SRAM Check] Value check 1nd IMG:0x15000038=0x%x\n",*((UINT32P) (0x15000038)));
	/* TINFO="mm mbist_done" */
	if ((*((UINT32P) (0x15000038)) & 0x1c40) != 0x1c40) {	//img mbist_done
		printf("img mbist_done is not ready %d\n", __LINE__);
                printf("[Repair SRAM Check] 2nd IMG:0x15000038=0x%x\n",*((UINT32P) (0x15000038)));
                printf("[Repair SRAM Check] 2nd IMG:0x15000048=0x%x\n",*((UINT32P) (0x15000048)));
                printf("[Repair SRAM Check] 2nd IMG:0x1500004c=0x%x\n",*((UINT32P) (0x1500004c)));
                ret = -1;
	}
	/* TINFO="img mbist_done" */
if( (*((UINT32P) (0x10206040)) & 0x00400000) == 0x0) {     //MD efuse not disable
	if ((*((UINT32P) (0x200e0024)) & 0x1fec) != 0x1fec) {	//md mbist_done
		printf("md mbist_done is not ready %d\n", __LINE__);
		ret = -1;
	}
	/* TINFO="mdmcu mbist_done" */
    if( (*((UINT32P) (0x10206044)) & 0x00000001) == 0x0) {   //HSPA efuse not disable
	if ((*((UINT32P) (0x236e002c)) & 0x4) != 0x4) {	//hspa3 mbist_done
		printf("hspa3 mbist_done is not ready %d\n", __LINE__);
		ret = -1;
	}
	/* TINFO="hspa3 mbist_done" */
    }
    if( (*((UINT32P) (0x10206044)) & 0x00000080) == 0x0) {   //TDD efuse not disable
	if ((*((UINT32P) (0x2405001c)) & 0x76870000) != 0x76870000) {	//tdd mbist_done
		printf("tdd mbist_done is not ready %d\n", __LINE__);
		ret = -1;
	}
	/* TINFO="tdd mbist_done" */
    }
}

// ------------------------------------------------------------------
	/* TINFO="check mbist_fail" */
	/* TINFO="mfg mbist_fail" */
	if ((*((UINT32P) (0x130000a8)) & 0x0007fff8)) {	// [18:3]: mfgsys mbist_fail
		printf("mfgsys sram fail %d\n", __LINE__);
		ret = -1;
	}
	/* TINFO="mm mbist_fail" */
	if ((*((UINT32P) (0x14000804)) & 0x00040000)) {	// [18] mm mbist_fail
		printf("mm sram fail %d\n", __LINE__);
		ret = -1;
	}
	/* TINFO="img mbist_fail"     
	   [31:26]: img mbist_fail
	   [40, 37, 34:32]: mbist_fail */
	if (((*((UINT32P) (0x15000048)) & 0xfc000000)) 
	    || ((*((UINT32P) (0x1500004c)) & 0x00000127))) {
		printf("img sram fail %d\n", __LINE__);
		ret = -1;
	}


if( (*((UINT32P) (0x10206040)) & 0x00400000) == 0x0) {     //MD efuse not disable
	/* TINFO="md mbist_fail"     
	   [28:26, 16:9] mbist_fail, [0]: mbist_fail_all  md
	   [28:0] mbist_fail */
	if (((*((UINT32P) (0x200e0030)) & 0x1c03fc01))
	    || ((*((UINT32P) (0x200e0034)) & 0x1fffffff))) {
		printf("md sram fail %d\n", __LINE__);
		ret = -1;
	}
    if( (*((UINT32P) (0x10206044)) & 0x00000001) == 0x0) {   //HSPA efuse not disable
	/* TINFO="hspa3 mbist_fail"
	   [31:29] mbist_fail hspa3
	   [5:0] mbist_fail */
	if (((*((UINT32P) (0x236e0034)) & 0xc0000000))
	    || ((*((UINT32P) (0x236e0038)) & 0x0000003f))) {
		printf("hspa3 sram fail %d\n", __LINE__);
		ret = -1;
	}
    }
    if( (*((UINT32P) (0x10206044)) & 0x00000080) == 0x0) {   //TDD efuse not disable
	/* TINFO="tdd mbist_fail"    
	   [16:11] mbist_fail tdd
	   [31:28, 26:23, 20, 19] mbist_fail
	   [1] mbist_fail */
	if (((*((UINT32P) (0x2405002c)) & 0x0001f800))
	    || ((*((UINT32P) (0x24050030)) & 0xf7980000))
	    || ((*((UINT32P) (0x24050034)) & 0x00000001))) {
		printf("hspa3 sram fail %d\n", __LINE__);
		ret = -1;
	}
    }
}
#if 0
printf("===================================After 2nd MBIST TEST============================\n");
printf("[Repair SRAM Check] 2nd MBIST TEST Check: START\n");
printf("[Repair SRAM Check] IMAGE:0x15000038=0x%x\n",*((UINT32P) (0x15000038)));
printf("[Repair SRAM Check] hspa3:0x236e002c=0x%x\n",*((UINT32P) (0x236e002c)));
printf("[Repair SRAM Check] efuse:0x10206040=0x%x\n",*((UINT32P) (0x10206040)));
printf("[Repair SRAM Check] efuse:0x10206044=0x%x\n",*((UINT32P) (0x10206044)));
printf("[Repair SRAM Check] TDD:0x2405001c=0x%x\n",*((UINT32P) (0x2405001c)));
printf("[Repair SRAM Check] MBIST TEST Check: END\n");
printf("\n");
printf("[Repair SRAM Check] After 2nd MBIST TEST: START\n");
printf("[Repair SRAM Check] MFG:0x130000a8=0x%x\n",*((UINT32P) (0x130000a8)));
printf("[Repair SRAM Check] MM:0x14000804=0x%x\n",*((UINT32P) (0x14000804)));
printf("[Repair SRAM Check] IMG:0x15000048=0x%x\n",*((UINT32P) (0x15000048)));
printf("[Repair SRAM Check] IMG:0x0x1500004c=0x%x\n",*((UINT32P) (0x1500004c)));
printf("[Repair SRAM Check] MD:0x200e0030=0x%x\n",*((UINT32P) (0x200e0030)));
printf("[Repair SRAM Check] MD:0x200e0034=0x%x\n",*((UINT32P) (0x200e0034)));
printf("[Repair SRAM Check] HSPA:0x236e0034=0x%x\n",*((UINT32P) (0x236e0034)));
printf("[Repair SRAM Check] HSPA:0x236e0038=0x%x\n",*((UINT32P) (0x236e0038)));
printf("[Repair SRAM Check] TDD:0x2405002c=0x%x\n",*((UINT32P) (0x2405002c)));
printf("[Repair SRAM Check] TDD:0x24050030=0x%x\n",*((UINT32P) (0x24050030)));
printf("[Repair SRAM Check] TDD:0x24050034=0x%x\n",*((UINT32P) (0x24050034)));
printf("[Repair SRAM Check] After 2nd MBIST TEST: END\n");
printf("\n");
#endif
printf("[Repair SRAM Check] VENCPLL Monitor START\n");
*((UINT32P) (0x10000220)) = 0x80;
*((UINT32P) (0x10000214)) = 0x0;
*((UINT32P) (0x10000100)) = 0x1600;

*((UINT32P) (0x10000220)) = 0x81;
udelay(500);
 printf("[Repair SRAM Check] CLK26CALI_0:0x1000_0220=0x%x\n",*((UINT32P) (0x10000220)));
 printf("[Repair SRAM Check] CLK26CALI_1:0x1000_0224=0x%x\n",*((UINT32P) (0x10000224)));
/*
 printf("[Repair SRAM Check] VENCPLL_CON0:0x1000F800=0x%x\n",*((UINT32P) (0x1000F800)));
 printf("[Repair SRAM Check] VENCPLL_CON1:0x1000F804=0x%x\n",*((UINT32P) (0x1000F804)));
 printf("[Repair SRAM Check] VENCPLL_CON2:0x1000F808=0x%x\n",*((UINT32P) (0x1000F808)));
 printf("[Repair SRAM Check] VENCPLL_PWR_CON0:0x1000F80C=0x%x\n",*((UINT32P) (0x1000F80C)));
*/
 printf("[Repair SRAM Check] VENCPLL Monitor End\n");
printf("\n");
printf("=============================================END=================================\n");

/*
printf("[Repair SRAM Check] VProc Monitor START");
ret=pmic_read_interface(0x21E,&reg_val,0xFFFF,0);
printf("Reg[0x21E]=0x%x, %d\n", reg_val, ret);
ret=pmic_read_interface(0x220,&reg_val,0xFFFF,0);
printf("Reg[0x220]=0x%x, %d\n", reg_val, ret);
ret=pmic_read_interface(0x222,&reg_val,0xFFFF,0);
printf("Reg[0x222]=0x%x, %d\n", reg_val, ret);
ret=pmic_read_interface(0x216,&reg_val,0xFFFF,0);
printf("Reg[0x216]=0x%x, %d\n", reg_val, ret);
ret=pmic_read_interface(0x224,&reg_val,0xFFFF,0);
printf("Reg[0x224]=0x%x, %d\n", reg_val, ret);

printf("[Repair SRAM Check] VProc Monitor End");
printf("\n");
*/

// ------------------------------------------------------------------
	/* TINFO="REVERT setting " */
	/* TINFO="clear mbist mode =0 " */
	*((UINT32P) (0x13000060)) = 0x00000000;	// [1] mbist_mode, [31]: mbist_rstb  mfg
	*((UINT32P) (0x14000810)) = 0x00000000;	// [6] mbist_mode  mm
	*((UINT32P) (0x15000074)) = 0x00000000;	// [12:10] mbist_mode  img 
if( (*((UINT32P) (0x10206040)) & 0x00400000) == 0x0) {     //MD efuse not disable
	*((UINT32P) (0x200e0000)) = 0x00000000;	// [11:4,2:1] mbist_mode  md
    if( (*((UINT32P) (0x10206044)) & 0x00000001) == 0x0) {   //HSPA efuse not disable
	*((UINT32P) (0x236e0000)) = 0x00000000;	// [7] mbist_mode  md_hspa3
    }
    if( (*((UINT32P) (0x10206044)) & 0x00000080) == 0x0) {   //TDD efuse not disable
	*((UINT32P) (0x24050000)) = 0x00000000;	// [29~27, 25~24,22,17~15] mbist_mode tdd
    }
}


rgu_swsys_reset(WD_MD_RST);
#if 0
// ------------------------------------------------------------------
if( (*((UINT32P) (0x10206040)) & 0x00400000) == 0x0) {     //MD efuse not disable
	/* TINFO="MD MTCMOS" */
	*((UINT32P) (0x23010810)) = 0xa200;	//md_hspa4 MODEM2G_TOPSM_RM_PWR_CON4
	/* TINFO="MD MTCMOS1" */
	*((UINT32P) (0x2301080c)) = 0xa200;	//md_hspa3 MODEM2G_TOPSM_RM_PWR_CON3
	/* TINFO="MD MTCMOS2" */
	*((UINT32P) (0x23010804)) = 0xa200;	//md_hspa1 MODEM2G_TOPSM_RM_PWR_CON1
	/* TINFO="MD MTCMOS3" */
	*((UINT32P) (0x23010800)) = 0xa200;	//md2g MODEM2G_TOPSM_RM_PWR_CON0
	/* TINFO="MD MTCMOS4" */
	*((UINT32P) (0x23010018)) = 0x0;	//MODEM2G_TOPSM_RM_TMR_PWR0
	/* TINFO="MD MTCMOS5" */

	/*TINFO="polling MD2g power off" */
//	udelay(40);
       udelay(2000);
	if ((*((UINT32P) (0x23010820)) & 0x00030000) != 0x0) {
		printf("MD2g power off is not ready %d\n", __LINE__);
		ret = -1;
	}

// ------------------------------------------------------------------
	/*TINFO="MD clock enable" */
	*((UINT32P) (0x20000458)) = 0x0;	//turn md infrasys clock
	*((UINT32P) (0x23000010)) = 0xffffffff;	//turn md modem clock
	*((UINT32P) (0x23000018)) = 0xffffffff;	//turn md modem clock
	*((UINT32P) (0x23000098)) = 0x0;	//turn md modem clock
    if( (*((UINT32P) (0x10206044)) & 0x00000001) == 0x0) {   //HSPA efuse not disable
	*((UINT32P) (0x2367002c)) = 0xffffffff;	//turn md hspa3 clock
	*((UINT32P) (0x23670010)) = 0xffffffff;	//turn md hspa3 clock
    }
    if( (*((UINT32P) (0x10206044)) & 0x00000080) == 0x0) {   //TDD efuse not disable
	//*((UINT16P) (0x2400001e)) = 0x0002;	// tddsys mbist_mem_clk_en
	//*((UINT16P) (0x24000422)) = 0x4208;	//turn on tddsys clock
	//*((UINT16P) (0x24000422)) = 0xc208;	//turn on tddsys clock
	*((UINT16P) (0x24000018))  = 0x0;   //tdd pmu sft rst 
	udelay(100);                        //wait > 62.5us
	*((UINT16P) (0x24000018))  = 0x1;   //set 1, after reset
    }
//printf("5\n");

// ------------------------------------------------------------------
	/*TINFO="MD clock switch" */
/* TINFO=" rdata = %h", *((UINT32P)(0x22c00040)) */
	*((UINT32P) (0x22c00040)) = 0x0;	// MD2GSYS_clock switch

	//SWITCH PLL clock
	*((UINT32P) (0x2000045c)) = 0x1008510;	//MD_GLOBAL_CON1, BUS_CLK = PLL Freq (not 26MHz)
	*((UINT32P) (0x20120060)) = 0x1010;	//PLL_CLKSW_CKSEL0, Bit 15-12: MDMCU_CLK = MCUPLL 481MHz, Bit  7- 4: DSP_CLK = MCUPLL DIV3 = 481 MHz / 2 = 240.5 MHz  
	*((UINT32P) (0x20120064)) = 0x1000;	//PLL_CLKSW_CKSEL1, Bit 15-12: BUS_CLK   = MCUPLL DIV2 = 481   MHz / 4 = 120.25MHz 
	*((UINT32P) (0x20120068)) = 0x1110;	//PLL_CLKSW_CKSEL2, Bit 15-12: FX64W_CLK = WPLL = 245.76MHz, Bit 11- 8: FX16G_CLK = MDPLL  DIV3 = 416 MHz / 2 = 208 MHz, Bit 7- 4: HW64W_CLK = WHPLL = 250.25MHz
	/*TINFO="MD PLL " */
	*((UINT32P) (0x2012004c)) = 0x1111;	// bit 15,8,1,0 set to 0
	*((UINT32P) (0x2012004c)) = 0x9111;	//PLL_PLL_CON3, bit 12,4 set to 0// [POWER ON] Bit15: MCUPLL, Bit 8: WPLL
	/* TINFO="wait 2u"*/
	udelay(2);
	*((UINT32P) (0x20120048)) = 0x1111;	//PLL_PLL_CON2, bit 12,10, 8, 6, 2 set to 0// [TOPSM & SW CTRL] Bit12: MDPLL, Bit 8: MCUPLL, Bit 4: WPLL, Bit 0: WHPLL
	*((UINT32P) (0x20120700)) = 0x11;	//PLL_PLLTD_CON0, bit 0 set to 0// [FHCTL & SW CTRL] Bit 4: CHG_CTRL
	*((UINT32P) (0x20120100)) = 0x410f;	// Enable PLLs, PLL_MDPLL_CON0
	*((UINT32P) (0x20120140)) = 0x0810;	// Enable PLLs, PLL_MCUPLL_CON0
	*((UINT32P) (0x201201c0)) = 0x0800;	// Enable PLLs, PLL_WPLL_CON0
	*((UINT32P) (0x20120200)) = 0x0500;	// Enable PLLs, PLL_WHPLL_CON0
	*((UINT32P) (0x20120110)) = 0x8003;	// PLL_MDPLL_CON4, bit0 set to 0

	/*TINFO="enable MD PLL" */
	*((UINT32P) (0x2012004c)) = 0x9311;	//PLL_PLL_CON3, bit 12,4 set to 0// [POWER ON] Bit15: MCUPLL, Bit 8: WPLL
	/* TINFO="wait 2u"*/
	udelay(2);
	*((UINT32P) (0x2012004c)) = 0x9331;	//PLL_PLL_CON3, bit 12,4 set to 0// [POWER ON] Bit15: MCUPLL, Bit 8: WPLL
	*((UINT32P) (0x201200ac)) = 0x0;	//PLL_DFS_CON7, bit 0~5 set to 1 // [FORCE ON] Bit 5: SYSCLK, Bit 4: MDPLL, Bit 3: WHPLL, Bit 2: WPLL, Bit 1: MCUPLL, Bit 0: No use (HW limit when boot)

	/* TINFO="MD TOP MTCMOS" */
	*((UINT32P) (0x20030018)) = 0x0;	//MD_TOPSM_RM_TMR_PWR0
	/* TINFO="MD MTCMOS6" */
}
// ------------------------------------------------------------------

// ------------------------------------------------------------------
	/* TINFO="MFG DCM" */
//*((UINT32P)(0x13000010)) = 0xc03f; //disable mfg DCM [15]

// ------------------------------------------------------------------
#endif
	/*TINFO="cksys clock switch" */

	*CLK_CFG_0 = rdata_cksw0 & 0x1F1F1F1F;
	*CLK_CFG_1 = rdata_cksw1 & 0x1F1F1F1F;
	*CLK_CFG_2 = rdata_cksw2 & 0x1F1F1F1F;
	*CLK_CFG_3 = rdata_cksw3 & 0x1F1F1F1F;
	*CLK_CFG_4 = rdata_cksw4 & 0x1F1F1F1F;
	udelay(1);
	if (((*CKSTA_REG) & 0xffffffff) != 0x0) {
		printf("clock switch is not ready %d\n", __LINE__);
		ret = -1;
	}

	*CLK_CFG_0_SET = rdata_cksw0 & 0x80808080;	//mem clock[8] not switch or will let dram hang(w/o set PLL clock source)
        *CLK_CFG_1_SET = rdata_cksw1 & 0x80808080;
	*CLK_CFG_2_SET = rdata_cksw2 & 0x80808080;
	*CLK_CFG_3_SET = rdata_cksw3 & 0x80808080;
	*CLK_CFG_4_SET = rdata_cksw4 & 0x808080E0;


//  *INFRA_TOPCKGEN_DCMCTL = 0x0;                     //Set this register before AXI clock switch to fast clock
////PLL_EN = 0
//*MMPLL_CON0   &= 0xfffffffe;
////*VENCPLL_CON0 &= 0xfffffffe;
////ISO_EN = 1
//*MMPLL_PWR_CON0   = 0x3;
////*VENCPLL_PWR_CON0 = 0x3;
////PWR_ON = 0
//*MMPLL_PWR_CON0   = 0x2;
////*VENCPLL_PWR_CON0 = 0x2;
// ------------------------------------------------------------------
//    /* TINFO="clock enable" */    
//*((UINT32P)(0x14000108)) = rdata_mm0;//turn mmsys clock
//*((UINT32P)(0x14000118)) = rdata_mm1;//turn mmsys clock
//*((UINT32P)(0x15000008)) = 0x0;//turn img clock
//*((UINT32P)(0x15004150)) = 0x0;//turn img clock
	/* TINFO="AP MTCMOS" */

	*((UINT32P) (0x10006214)) = 0x0000010d;	//MFG MTCOMS 
	*((UINT32P) (0x10006214)) = 0x0000030d;	//MFG MTCOMS
	*((UINT32P) (0x10006214)) = 0x0000070d;	//MFG MTCOMS
	*((UINT32P) (0x10006214)) = 0x00000f0d;	//MFG MTCOMS
	*((UINT32P) (0x10006214)) = 0x00000f0f;	//MFG MTCOMS
	*((UINT32P) (0x10006214)) = 0x00000f1e;	//MFG MTCOMS
	*((UINT32P) (0x10006214)) = 0x00000f1a;	//MFG MTCOMS
	*((UINT32P) (0x10006214)) = 0x00000f12;	//MFG MTCOMS

	*((UINT32P) (0x10006238)) = 0x0000010d;	//ISP MTCOMS 
	*((UINT32P) (0x10006238)) = 0x0000030d;	//ISP MTCOMS
	*((UINT32P) (0x10006238)) = 0x0000070d;	//ISP MTCOMS
	*((UINT32P) (0x10006238)) = 0x00000f0d;	//ISP MTCOMS
	*((UINT32P) (0x10006238)) = 0x00000f0f;	//ISP MTCOMS
	*((UINT32P) (0x10006238)) = 0x00000f1e;	//ISP MTCOMS
	*((UINT32P) (0x10006238)) = 0x00000f1a;	//ISP MTCOMS
	*((UINT32P) (0x10006238)) = 0x00000f12;	//ISP MTCOMS


/*  DISP already opened in preloader
	*((UINT32P) (0x1000623c)) = 0x0000010d;	//DISP MTCOMS 
	*((UINT32P) (0x1000623c)) = 0x0000030d;	//DISP MTCOMS
	*((UINT32P) (0x1000623c)) = 0x0000070d;	//DISP MTCOMS
	*((UINT32P) (0x1000623c)) = 0x00000f0d;	//DISP MTCOMS
	*((UINT32P) (0x1000623c)) = 0x00000f0f;	//DISP MTCOMS
	*((UINT32P) (0x1000623c)) = 0x00000f1e;	//DISP MTCOMS
	*((UINT32P) (0x1000623c)) = 0x00000f1a;	//DISP MTCOMS
	*((UINT32P) (0x1000623c)) = 0x00000f12;	//DISP MTCOMS

*/
//    /* TINFO="CPU revert" */    
//  *ACLKEN_DIV = 0x0; //div2
//  *PCLKEN_DIV = 0x10; //div4
//  *MEM_PWR_CTRL = 0x0; //slpb_dly and mem_off_dly
//  *MCU_BIU_CON = 0x7f0; //enable out-of-order queue 
//  *CA7_MISC_CONFIG = 0x9c000000;

	return ret;
}
