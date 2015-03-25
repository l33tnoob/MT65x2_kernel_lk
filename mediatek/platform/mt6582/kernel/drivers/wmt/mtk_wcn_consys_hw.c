/*! \file
    \brief  Declaration of library functions

    Any definitions in this file will be shared among GLUE Layer and internal Driver Stack.
*/




/*******************************************************************************
*                         C O M P I L E R   F L A G S
********************************************************************************
*/

/*******************************************************************************
*                                 M A C R O S
********************************************************************************
*/
#ifdef DFT_TAG
#undef DFT_TAG
#endif
#define DFT_TAG "[WMT-CONSYS-HW]"


/*******************************************************************************
*                    E X T E R N A L   R E F E R E N C E S
********************************************************************************
*/
#include "mtk_wcn_consys_hw.h"

#include <mach/upmu_common.h>
#include <mach/mt_clkmgr.h>
#include <mach/emi_mpu.h>
#if CONSYS_PMIC_CTRL_ENABLE 
#include <mach/mt_pm_ldo.h>
#endif
#include <mach/mtk_hibernate_dpm.h>
#include <asm/memblock.h>



/*******************************************************************************
*                              C O N S T A N T S
********************************************************************************
*/


/*******************************************************************************
*                             D A T A   T Y P E S
********************************************************************************
*/

/*******************************************************************************
*                            P U B L I C   D A T A
********************************************************************************
*/
UINT8 __iomem *pEmibaseaddr = NULL;
phys_addr_t gConEmiPhyBase;
/*******************************************************************************
*                           P R I V A T E   D A T A
********************************************************************************
*/





/*******************************************************************************
*                  F U N C T I O N   D E C L A R A T I O N S
********************************************************************************
*/

extern unsigned int get_CONNSYS_start (void);
extern unsigned int get_CONNSYS_size (void);

/*******************************************************************************
*                              F U N C T I O N S
********************************************************************************
*/
#if CONSYS_ENALBE_SET_JTAG
	UINT32 gJtagCtrl = 0;
	#define JTAG_ADDR1_BASE 0x10208000
	#define JTAG_ADDR2_BASE 0x10005300
	char *jtag_addr1 = (char *)JTAG_ADDR1_BASE;
	char *jtag_addr2 = (char *)JTAG_ADDR2_BASE;

	#define JTAG1_REG_WRITE(addr, value)    \
    writel(value, ((volatile UINT32 *)(jtag_addr1+(addr-JTAG_ADDR1_BASE))))
	#define JTAG1_REG_READ(addr)            \
    readl(((volatile UINT32 *)(jtag_addr1+(addr-JTAG_ADDR1_BASE))))
	#define JTAG2_REG_WRITE(addr, value)    \
    writel(value, ((volatile UINT32 *)(jtag_addr2+(addr-JTAG_ADDR2_BASE))))
	#define JTAG2_REG_READ(addr)            \
    readl(((volatile UINT32 *)(jtag_addr2+(addr-JTAG_ADDR2_BASE))))

static INT32 mtk_wcn_consys_jtag_set_for_mcu(VOID)
{
    int iRet = 0;
    unsigned int tmp = 0;

    jtag_addr1 = ioremap(JTAG_ADDR1_BASE, 0x5000);
    if (jtag_addr1 == 0) {
        printk("remap jtag_addr1 fail!\n");
        return -1;
    }
    printk("jtag_addr1 = 0x%p\n", jtag_addr1);
    jtag_addr2 = ioremap(JTAG_ADDR2_BASE, 0x1000);
    if (jtag_addr2 == 0) {
        printk("remap jtag_addr2 fail!\n");
        return -1;
    }
    printk("jtag_addr2 = 0x%p\n", jtag_addr2);

                /*Enable IES of all pins*/
    JTAG1_REG_WRITE(0x10208004, 0xffffffff);
    JTAG1_REG_WRITE(0x10208014, 0xffffffff);
    JTAG1_REG_WRITE(0x10209004, 0xffffffff);
    JTAG1_REG_WRITE(0x1020A004, 0xffffffff);
    JTAG1_REG_WRITE(0x1020B004, 0xffffffff);

	/*JTAG1 mode setting*/
	/*set GPIO pull mode*/
	/*0x1020A040[6:0]=0x7f*/
	tmp = JTAG1_REG_READ(0x1020A040);
	tmp &= 0xffffff80;
	tmp |= 0x7f;
	JTAG1_REG_WRITE(0x1020A040, tmp);

#if 0
                /*0x1020A060[6:0]=0x31*/
                tmp = JTAG1_REG_READ(0x1020A060);
                tmp &= 0xffffff80;
                tmp |= 0x31;
    JTAG1_REG_WRITE(0x1020A060, tmp);
                /*set CONSYS JTAG mode 0x10005300=0x07777777*/
                //DRV_WriteReg32(0x10005300, 0x07777777); /*remove JTAG mode1*/
                JTAG2_REG_WRITE(0x10005300, 0x00000000);
#endif

#if 1 // Chaozhong modified
    /*0x1020A060[6:0]=0x31*/
    tmp = JTAG1_REG_READ(0x1020A060);
    tmp &= 0xffffff80;
    tmp |= 0x31;
    JTAG1_REG_WRITE(0x1020A060, tmp);
    /*set CONSYS JTAG mode 0x10005300=0x07777777*/
    //DRV_WriteReg32(0x10005300, 0x07777777); /*remove JTAG mode1*/
    JTAG2_REG_WRITE(0x1000530C, 0x08888888);// Chaozhong modified
#endif


/*JTAG1 mode setting*/
    
    /*set GPIO pull mode*/
    /*0x1020A050[6:0]=0x7f*/
    tmp = JTAG1_REG_READ(0x1020A050);
   	tmp &= 0xffffff80;
    tmp |= 0x7f;
    JTAG1_REG_WRITE(0x1020A050, tmp);
    /*0x1020A070[6:0]=0x31*/
    tmp = JTAG1_REG_READ(0x1020A070);
    tmp &= 0xffffff80;
    tmp |= 0x31;
    JTAG1_REG_WRITE(0x1020A070, tmp);
	/*set CONSYS JTAG mode
	0x10005310=0x77111111
	0x10005320=0x00077777*/
	JTAG2_REG_WRITE(0x1000531C, 0xff111111);
	JTAG2_REG_WRITE(0x1000532C, 0x000fffff);

                /*set CONSYS debug flag mode*/
    JTAG2_REG_WRITE(0x10005410, 0x77700000); /* GPIO141 ~ GPIO143 */
    JTAG2_REG_WRITE(0x10005420, 0x00000077); /* GPIO144 ~ GPIO145 */
    JTAG2_REG_WRITE(0x10005370, 0x70000000); /* GPIO63 */
    JTAG2_REG_WRITE(0x10005380, 0x00000777); /* GPIO64 ~ GPIO66 */
    JTAG2_REG_WRITE(0x100053a0, 0x70000000); /* GPIO87 */
    JTAG2_REG_WRITE(0x100053b0, 0x00000777); /* GPIO88 ~ GPIO90 */
    JTAG2_REG_WRITE(0x100053d0, 0x00777000); /* GPIO107 ~ GPIO109 */

    return iRet;
}

UINT32 mtk_wcn_consys_jtag_flag_ctrl(UINT32 en)
{
	WMT_PLAT_INFO_FUNC("%s jtag set for MCU\n",en ? "enable" : "disable");
	gJtagCtrl = en;

	return 0;
}

#endif


INT32 mtk_wcn_consys_hw_reg_ctrl(UINT32 on,UINT32 co_clock_en)
{
	INT32 iRet = -1;
	UINT32 retry = 10;
	UINT32 consysHwChipId = 0;
	
	WMT_PLAT_INFO_FUNC("CONSYS-HW-REG-CTRL(0x%08x),start\n",on);
	if(on)
	{
		
#if CONSYS_PMIC_CTRL_ENABLE 
		/*need PMIC driver provide new API protocol before 1/18/2013*/
	    /*1.Power on MT6323 VCN_1V8 LDO<--<VCN_1V8>-->write 0 to 0x512[1], write 1 to 0x512[14]*/
		upmu_set_vcn_1v8_lp_mode_set(0);
		//upmu_set_rg_vcn_1v8_en(1);
		/*will be replaced by hwpoweron just as below*/
		hwPowerOn(MT6323_POWER_LDO_VCN_1V8,VOL_DEFAULT,"MOD_WMT");
		
		udelay(150);
		
		if(co_clock_en)
		{
			/*2.set VCN_28 to SW control mode<--<VCN28_ON_CTRL>-->write 0 to 0x41C[14]*/
			upmu_set_vcn28_on_ctrl(0);
		}
		else
		{
			/*2.1.switch VCN28 to HW control mode<--<VCN28_ON_CTRL>-->write 1 to 0x41C[14]*/
			upmu_set_vcn28_on_ctrl(1);
			/*2.2.turn on VCN28LDO<--<RG_VCN28_EN>-->write 1 to 0x41C[12]*/
			//upmu_set_rg_vcn28_en(1);
			/*will be replaced by hwpoweron just as below*/
			hwPowerOn(MT6323_POWER_LDO_VCN28,VOL_DEFAULT,"MOD_WMT");
		}
#endif

		/*mask this action and put it into FW patch for resolve ALPS00544691*/
#if 0
		/*1.assert CONSYS CPU SW reset, <CONSYS_CPU_SW_RST_REG>, [12] = 1'b1, [31:24]=8'h88(key)--> CONSYS_CPU_SW_RST_BIT | CONSYS_CPU_SW_RST_CTRL_KEY*/
		CONSYS_SET_BIT(CONSYS_CPU_SW_RST_REG, CONSYS_CPU_SW_RST_BIT | CONSYS_CPU_SW_RST_CTRL_KEY);
		WMT_PLAT_DBG_FUNC("reg uump:CONSYS_CPU_SW_RST_REG(0x%x)\n",CONSYS_REG_READ(CONSYS_CPU_SW_RST_REG));
#endif

#if 0
		/*turn on top clock gating enable*/
		CONSYS_REG_WRITE(CONSYS_TOP_CLKCG_CLR_REG,CONSYS_REG_READ(CONSYS_TOP_CLKCG_CLR_REG) | CONSYS_TOP_CLKCG_BIT);
		WMT_PLAT_DBG_FUNC("reg dump:CONSYS_TOP_CLKCG_CLR_REG(0x%x)\n",CONSYS_REG_READ(CONSYS_TOP_CLKCG_CLR_REG));
		/*turn on SPM clock gating enable*/
		CONSYS_REG_WRITE(CONSYS_PWRON_CONFG_EN_REG, CONSYS_PWRON_CONFG_EN_VALUE);
		WMT_PLAT_DBG_FUNC("reg dump:CONSYS_PWRON_CONFG_EN_REG(0x%x)\n",CONSYS_REG_READ(CONSYS_PWRON_CONFG_EN_REG));
#endif

		/*use colck manger API to control MTCMOS*/
		conn_power_on();

		WMT_PLAT_INFO_FUNC("reg dump:CONSYS_PWR_CONN_ACK_REG(0x%x)\n",CONSYS_REG_READ(CONSYS_PWR_CONN_ACK_REG));
		WMT_PLAT_INFO_FUNC("reg dump:CONSYS_PWR_CONN_ACK_S_REG(0x%x)\n",CONSYS_REG_READ(CONSYS_PWR_CONN_ACK_S_REG));
		WMT_PLAT_INFO_FUNC("reg dump:CONSYS_TOP1_PWR_CTRL_REG(0x%x)\n",CONSYS_REG_READ(CONSYS_TOP1_PWR_CTRL_REG));

		/*11.delay 10us, 26M is ready*/
		udelay(10);

		enable_clock(MT_CG_INFRA_CONNMCU, "WMT_MOD");
		/*12.poll CONSYS CHIP until MT6582/MT6572 is returned, <CONSYS_CHIP_ID_REG>, 32'h6582/32'h6572 */
		/*what does HW do, why we need to polling this register?*/
		while (retry-- > 0)
		{
			WMT_PLAT_DBG_FUNC("CONSYS_CHIP_ID_REG(0x%08x)",CONSYS_REG_READ(CONSYS_CHIP_ID_REG));
			consysHwChipId = CONSYS_REG_READ(CONSYS_CHIP_ID_REG);
			if((consysHwChipId == 0x6582) || (consysHwChipId == 0x6572))
			{
				WMT_PLAT_INFO_FUNC("retry(%d)consys chipId(0x%08x)\n", retry,consysHwChipId);
				break;
			}
			msleep(20);
		}
		/*mask this action and put it into FW patch for resolve ALPS00544691*/
#if 0
		/*13.{default no need}update ROMDEL/PATCH RAM DELSEL if needed, <CONSYS_ROM_RAM_DELSEL_REG>*/
		
		/*14.write 1 to conn_mcu_config ACR[1] if real speed MBIST (default write "1"), <CONSYS_MCU_CFG_ACR_REG>,[18]1'b1-->CONSYS_MCU_CFG_ACR_MBIST_BIT*/
		/*if this bit is 0, HW will do memory auto test under low CPU frequence (26M Hz)*/
		/*if this bit is 0, HW will do memory auto test under high CPU frequence(138M Hz) inclulding low CPU frequence*/
		CONSYS_SET_BIT(CONSYS_MCU_CFG_ACR_REG, CONSYS_MCU_CFG_ACR_MBIST_BIT);
		/*15.{default no need, Analog HW will inform if this need to be update or not 1 week after IC sample back}
		update ANA_WBG(AFE) CR if needed, CONSYS_AFE_REG */
		CONSYS_REG_WRITE(CONSYS_AFE_REG_DIG_RCK_01,CONSYS_AFE_REG_DIG_RCK_01_VALUE);
		CONSYS_REG_WRITE(CONSYS_AFE_REG_WBG_PLL_02,CONSYS_AFE_REG_WBG_PLL_02_VALUE);
		CONSYS_REG_WRITE(CONSYS_AFE_REG_WBG_WB_TX_01,CONSYS_AFE_REG_WBG_WB_TX_01_VALUE);
		/*16.deassert CONSYS CPU SW reset, <CONSYS_CPU_SW_RST_REG>, [12] = 1'b0, [31:24]=8'h88(key)*/
		CONSYS_CLR_BIT_WITH_KEY(CONSYS_CPU_SW_RST_REG, CONSYS_CPU_SW_RST_BIT , CONSYS_CPU_SW_RST_CTRL_KEY);
#endif
		msleep(5);
		iRet = 0;
	}else{

		disable_clock(MT_CG_INFRA_CONNMCU, "WMT_MOD");

		/*New: use colck manger API to control MTCMOS*/
		conn_power_off();

#if CONSYS_PMIC_CTRL_ENABLE
		/*set VCN_28 to SW  control mode*/
		upmu_set_vcn28_on_ctrl(0);
		/*turn off VCN28 LDO*/
		//upmu_set_rg_vcn28_en(0);
		/*will be replaced by hwPowerOff*/
		hwPowerDown(MT6323_POWER_LDO_VCN28,"MOD_WMT");
		/*power off MT6627 VWCN_1V8 LDO*/
		upmu_set_vcn_1v8_lp_mode_set(0);
		//upmu_set_rg_vcn_1v8_en(0);
		/*will be replaced by hwPowerOff*/
		hwPowerDown(MT6323_POWER_LDO_VCN_1V8,"MOD_WMT");
#endif
		
		iRet = 0;

	}
	WMT_PLAT_INFO_FUNC("CONSYS-HW-REG-CTRL(0x%08x),finish\n",on);
	return iRet;
}

INT32
mtk_wcn_consys_hw_gpio_ctrl (UINT32 on)
{
    INT32 iRet = 0;
	WMT_PLAT_INFO_FUNC("CONSYS-HW-GPIO-CTRL(0x%08x), start\n",on);

	if(on)
	{

		/*if external modem used,GPS_SYNC still needed to control*/
	    iRet += wmt_plat_gpio_ctrl(PIN_GPS_SYNC, PIN_STA_INIT);
	    iRet += wmt_plat_gpio_ctrl(PIN_GPS_LNA, PIN_STA_INIT);

	    iRet += wmt_plat_gpio_ctrl(PIN_I2S_GRP,PIN_STA_INIT);

	    /*set EINT< -ommited-> move this to WMT-IC module, where common sdio interface will be identified and do proper operation*/
	    // TODO: [FixMe][GeorgeKuo] double check if BGF_INT is implemented ok
	    //iRet += wmt_plat_gpio_ctrl(PIN_BGF_EINT, PIN_STA_MUX);
	    #if CFG_WMT_DUMP_INT_STATUS
			wmt_plat_BGF_irq_dump_status();
		#endif
	    iRet += wmt_plat_eirq_ctrl(PIN_BGF_EINT, PIN_STA_INIT);
		#if CFG_WMT_DUMP_INT_STATUS
			wmt_plat_BGF_irq_dump_status();
		#endif
		iRet += wmt_plat_eirq_ctrl(PIN_BGF_EINT, PIN_STA_EINT_DIS);
		#if CFG_WMT_DUMP_INT_STATUS
			wmt_plat_BGF_irq_dump_status();
		#endif
	    WMT_PLAT_INFO_FUNC("CONSYS-HW, BGF IRQ registered and disabled \n");

	}else{

	    /* set bgf eint/all eint to deinit state, namely input low state*/
	    iRet += wmt_plat_eirq_ctrl(PIN_BGF_EINT, PIN_STA_EINT_DIS);
		iRet += wmt_plat_eirq_ctrl(PIN_BGF_EINT, PIN_STA_DEINIT);
		#if CFG_WMT_DUMP_INT_STATUS
			wmt_plat_BGF_irq_dump_status();
		#endif
	    WMT_PLAT_INFO_FUNC("CONSYS-HW, BGF IRQ unregistered and disabled\n");
	    //iRet += wmt_plat_gpio_ctrl(PIN_BGF_EINT, PIN_STA_DEINIT);

		/*if external modem used,GPS_SYNC still needed to control*/
	    iRet += wmt_plat_gpio_ctrl(PIN_GPS_SYNC, PIN_STA_DEINIT);
	   	iRet += wmt_plat_gpio_ctrl(PIN_I2S_GRP,PIN_STA_DEINIT);
	    /* deinit gps_lna*/
	    iRet += wmt_plat_gpio_ctrl(PIN_GPS_LNA, PIN_STA_DEINIT);
	    

	}
	WMT_PLAT_INFO_FUNC("CONSYS-HW-GPIO-CTRL(0x%08x), finish\n",on);
    return iRet;

}

INT32 mtk_wcn_consys_hw_pwr_on(UINT32 co_clock_en)
{
	INT32 iRet = 0;

	WMT_PLAT_INFO_FUNC("CONSYS-HW-PWR-ON, start\n");

	iRet += mtk_wcn_consys_hw_reg_ctrl(1,co_clock_en);
	iRet += mtk_wcn_consys_hw_gpio_ctrl(1);
#if CONSYS_ENALBE_SET_JTAG
	if(gJtagCtrl)
	{
		mtk_wcn_consys_jtag_set_for_mcu();
	}
#endif
	WMT_PLAT_INFO_FUNC("CONSYS-HW-PWR-ON, finish(%d)\n",iRet);
	return iRet;
}

INT32 mtk_wcn_consys_hw_pwr_off (VOID)
{
	INT32 iRet = 0;

	WMT_PLAT_INFO_FUNC("CONSYS-HW-PWR-OFF, start\n");

	iRet += mtk_wcn_consys_hw_reg_ctrl(0,0);
	iRet += mtk_wcn_consys_hw_gpio_ctrl(0);

	WMT_PLAT_INFO_FUNC("CONSYS-HW-PWR-OFF, finish(%d)\n",iRet);
	return iRet;
}


INT32
mtk_wcn_consys_hw_rst (UINT32 co_clock_en)
{
    INT32 iRet = 0;
    WMT_PLAT_INFO_FUNC("CONSYS-HW, hw_rst start, eirq should be disabled before this step\n");

    /*1. do whole hw power off flow*/
    iRet += mtk_wcn_consys_hw_reg_ctrl(0,co_clock_en);

    /*2. do whole hw power on flow*/
    iRet += mtk_wcn_consys_hw_reg_ctrl(1,co_clock_en);

    WMT_PLAT_INFO_FUNC("CONSYS-HW, hw_rst finish, eirq should be enabled after this step\n");
    return iRet;
}

INT32 mtk_wcn_consys_hw_bt_paldo_ctrl(UINT32 enable)
{
	if(enable){
		/*do BT PMIC on,depenency PMIC API ready*/
		/*switch BT PALDO control from SW mode to HW mode:0x416[5]-->0x1*/
#if CONSYS_PMIC_CTRL_ENABLE 
		hwPowerOn(MT6323_POWER_LDO_VCN33_BT,VOL_3300,"MOD_WMT");
		upmu_set_vcn33_on_ctrl_bt(1);
		
#endif
		WMT_PLAT_INFO_FUNC("WMT do BT PMIC on\n");
	}else{
		/*do BT PMIC off*/
		/*switch BT PALDO control from HW mode to SW mode:0x416[5]-->0x0*/
#if CONSYS_PMIC_CTRL_ENABLE 
		upmu_set_vcn33_on_ctrl_bt(0);
		hwPowerDown(MT6323_POWER_LDO_VCN33_BT,"MOD_WMT");
#endif
		WMT_PLAT_INFO_FUNC("WMT do BT PMIC off\n");
	}
	return 0;
}

INT32 mtk_wcn_consys_hw_wifi_paldo_ctrl(UINT32 enable)
{
	if(enable){
		/*do WIFI PMIC on,depenency PMIC API ready*/
		/*switch WIFI PALDO control from SW mode to HW mode:0x418[14]-->0x1*/
#if CONSYS_PMIC_CTRL_ENABLE 
		hwPowerOn(MT6323_POWER_LDO_VCN33_WIFI,VOL_3300,"MOD_WMT");
		upmu_set_vcn33_on_ctrl_wifi(1);
#endif
		WMT_PLAT_INFO_FUNC("WMT do WIFI PMIC on\n");
	}else{
		/*do WIFI PMIC off*/
		/*switch WIFI PALDO control from HW mode to SW mode:0x418[14]-->0x0*/
#if CONSYS_PMIC_CTRL_ENABLE 
		upmu_set_vcn33_on_ctrl_wifi(0);
		hwPowerDown(MT6323_POWER_LDO_VCN33_WIFI,"MOD_WMT");
#endif
		WMT_PLAT_INFO_FUNC("WMT do WIFI PMIC off\n");
	}
	return 0;
}

INT32 mtk_wcn_consys_hw_vcn28_ctrl(UINT32 enable)
{
	if(enable){
		/*in co-clock mode,need to turn on vcn28 when fm on*/
#if CONSYS_PMIC_CTRL_ENABLE 
		hwPowerOn(MT6323_POWER_LDO_VCN28,VOL_DEFAULT,"MOD_WMT");
#endif
		WMT_PLAT_INFO_FUNC("turn on vcn28 for fm/gps usage in co-clock mode\n");
	}else{
		/*in co-clock mode,need to turn off vcn28 when fm off*/
#if CONSYS_PMIC_CTRL_ENABLE 
		hwPowerDown(MT6323_POWER_LDO_VCN28,"MOD_WMT");
#endif
		WMT_PLAT_INFO_FUNC("turn off vcn28 for fm/gps usage in co-clock mode\n");
	}
	return 0;
}

INT32 mtk_wcn_consys_hw_state_show(VOID)
{
	return 0;
}

#if CONSYS_WMT_REG_SUSPEND_CB_ENABLE
UINT32 mtk_wcn_consys_hw_osc_en_ctrl(UINT32 en)
{
	if(en)
	{
		WMT_PLAT_INFO_FUNC("enable consys sleep mode(turn off 26M)\n");
		CONSYS_REG_WRITE(CONSYS_AP2CONN_OSC_EN_REG, CONSYS_REG_READ(CONSYS_AP2CONN_OSC_EN_REG) & ~CONSYS_AP2CONN_OSC_EN_BIT);
	}else
	{
		WMT_PLAT_INFO_FUNC("disable consys sleep mode\n");
		CONSYS_REG_WRITE(CONSYS_AP2CONN_OSC_EN_REG, CONSYS_REG_READ(CONSYS_AP2CONN_OSC_EN_REG) | CONSYS_AP2CONN_OSC_EN_BIT);	
	}

	WMT_PLAT_INFO_FUNC("dump CONSYS_AP2CONN_OSC_EN_REG(0x%x)\n",CONSYS_REG_READ(CONSYS_AP2CONN_OSC_EN_REG));

	return 0;
}
#endif

INT32 mtk_wcn_consys_hw_restore(struct device *device)
{
	INT32 iRet = -1;
	UINT32 addrPhy = 0;

	/*set MPU for EMI share Memory*/
	WMT_PLAT_INFO_FUNC("setting MPU for EMI share memory\n");
	emi_mpu_set_region_protection(gConEmiPhyBase + SZ_1M/2,
		gConEmiPhyBase + SZ_1M,
		5,
		SET_ACCESS_PERMISSON(FORBIDDEN,NO_PROTECTION,FORBIDDEN,NO_PROTECTION));
	WMT_PLAT_INFO_FUNC("get consys start phy address(0x%x)\n",gConEmiPhyBase);

	/*consys to ap emi remapping register:10001310, cal remapping address*/
	addrPhy = (gConEmiPhyBase & 0xFFF00000) >> 20;

	/*enable consys to ap emi remapping bit12*/
	addrPhy = addrPhy | 0x1000;

	CONSYS_REG_WRITE(CONSYS_EMI_MAPPING,CONSYS_REG_READ(CONSYS_EMI_MAPPING) | addrPhy);

	WMT_PLAT_INFO_FUNC("CONSYS_EMI_MAPPING dump(0x%08x)\n",CONSYS_REG_READ(CONSYS_EMI_MAPPING));
#if 1
	pEmibaseaddr = ioremap_nocache(gConEmiPhyBase + CONSYS_EMI_AP_PHY_OFFSET,CONSYS_EMI_MEM_SIZE);
#else
	pEmibaseaddr = ioremap_nocache(CONSYS_EMI_AP_PHY_BASE,CONSYS_EMI_MEM_SIZE);
#endif
	//pEmibaseaddr = ioremap_nocache(0x80090400,270*KBYTE);
	if(pEmibaseaddr)
	{
		WMT_PLAT_INFO_FUNC("EMI mapping OK(0x%p)\n",pEmibaseaddr);
		memset(pEmibaseaddr,0,CONSYS_EMI_MEM_SIZE);
		iRet = 0;
	}else{
		WMT_PLAT_ERR_FUNC("EMI mapping fail\n");
	}

	return iRet;
}


VOID __init mtk_wcn_consys_memory_reserve(VOID)
{
	gConEmiPhyBase = arm_memblock_steal(SZ_1M,SZ_1M);

	if(gConEmiPhyBase)
	{
		WMT_PLAT_INFO_FUNC("memblock done: 0x%x\n",gConEmiPhyBase);
	}else
	{
		WMT_PLAT_ERR_FUNC("memblock fail\n");
	}
}

INT32 mtk_wcn_consys_hw_init()
{
	INT32 iRet = -1;
	UINT32 addrPhy = 0;

	/*set MPU for EMI share Memory*/
	WMT_PLAT_INFO_FUNC("setting MPU for EMI share memory\n");
	emi_mpu_set_region_protection(gConEmiPhyBase + SZ_1M/2,
		gConEmiPhyBase + SZ_1M,
		5,
		SET_ACCESS_PERMISSON(FORBIDDEN,NO_PROTECTION,FORBIDDEN,NO_PROTECTION));
	WMT_PLAT_INFO_FUNC("get consys start phy address(0x%x)\n",gConEmiPhyBase);

	/*consys to ap emi remapping register:10001310, cal remapping address*/
	addrPhy = (gConEmiPhyBase & 0xFFF00000) >> 20;

	/*enable consys to ap emi remapping bit12*/
	addrPhy = addrPhy | 0x1000;

	CONSYS_REG_WRITE(CONSYS_EMI_MAPPING,CONSYS_REG_READ(CONSYS_EMI_MAPPING) | addrPhy);

	WMT_PLAT_INFO_FUNC("CONSYS_EMI_MAPPING dump(0x%08x)\n",CONSYS_REG_READ(CONSYS_EMI_MAPPING));
#if 1
	pEmibaseaddr = ioremap_nocache(gConEmiPhyBase + CONSYS_EMI_AP_PHY_OFFSET,CONSYS_EMI_MEM_SIZE);
#else
	pEmibaseaddr = ioremap_nocache(CONSYS_EMI_AP_PHY_BASE,CONSYS_EMI_MEM_SIZE);
#endif
	//pEmibaseaddr = ioremap_nocache(0x80090400,270*KBYTE);
	if(pEmibaseaddr)
	{
		WMT_PLAT_INFO_FUNC("EMI mapping OK(0x%p)\n",pEmibaseaddr);
		memset(pEmibaseaddr,0,CONSYS_EMI_MEM_SIZE);
		iRet = 0;
	}else{
		WMT_PLAT_ERR_FUNC("EMI mapping fail\n");
	}
	WMT_PLAT_INFO_FUNC("register connsys restore cb for complying with IPOH function\n");
	register_swsusp_restore_noirq_func(ID_M_CONNSYS,mtk_wcn_consys_hw_restore,NULL);

	return iRet;
}

INT32 mtk_wcn_consys_hw_deinit()
{
	if(pEmibaseaddr)
	{
		iounmap(pEmibaseaddr);
		pEmibaseaddr = NULL;
	}
	unregister_swsusp_restore_noirq_func(ID_M_CONNSYS);
	return 0;
}


UINT8  *mtk_wcn_consys_emi_virt_addr_get(UINT32 ctrl_state_offset)
{
	UINT8 *p_virtual_addr = NULL;

	if(!pEmibaseaddr)
	{
		WMT_PLAT_ERR_FUNC("EMI base address is NULL\n");
		return NULL;
	}
	WMT_PLAT_DBG_FUNC("ctrl_state_offset(%08x)\n",ctrl_state_offset);
	p_virtual_addr = pEmibaseaddr + ctrl_state_offset;

	return p_virtual_addr;
}

UINT32 mtk_wcn_consys_soc_chipid()
{
	return PLATFORM_SOC_CHIP;
}

