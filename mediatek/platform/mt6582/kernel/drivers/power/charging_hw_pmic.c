/*****************************************************************************
 *
 * Filename:
 * ---------
 *    charging_pmic.c
 *
 * Project:
 * --------
 *   ALPS_Software
 *
 * Description:
 * ------------
 *   This file implements the interface between BMT and ADC scheduler.
 *
 * Author:
 * -------
 *  Oscar Liu
 *
 *============================================================================
  * $Revision:   1.0  $
 * $Modtime:   11 Aug 2005 10:28:16  $
 * $Log:   //mtkvs01/vmdata/Maui_sw/archives/mcu/hal/peripheral/inc/bmt_chr_setting.h-arc  $
 *             HISTORY
 * Below this line, this part is controlled by PVCS VM. DO NOT MODIFY!!
 *------------------------------------------------------------------------------
 *------------------------------------------------------------------------------
 * Upper this line, this part is controlled by PVCS VM. DO NOT MODIFY!!
 *============================================================================
 ****************************************************************************/
#include <mach/charging.h>
#include <mach/upmu_common.h>
#include <mach/upmu_hw.h>
#include <linux/xlog.h>
#include <linux/delay.h>
#include <mach/mt_sleep.h>
#include <mach/mt_boot.h>
#include <mach/system.h>

#include "cust_battery_meter.h"
#include <cust_charging.h>


#if 0 //defined(TINNO_QUICK_CHARGING)
#include <mach/mt_gpio.h>

#include "cust_adc.h"
//#define DM_MAX_VOLTAGE 100 
//#define DM_MIN_VOLTAGE 0 
extern int IMM_GetOneChannelValue(int dwChannel, int data[4], int* rawdata);

#define GPIO_DM_VOL_DET_PIN GPIO105
#endif
 

 // ============================================================ //
 //define
 // ============================================================ //
#define STATUS_OK	0
#define STATUS_UNSUPPORTED	-1
#define GETARRAYNUM(array) (sizeof(array)/sizeof(array[0]))


 // ============================================================ //
 //global variable
 // ============================================================ //
kal_bool chargin_hw_init_done = KAL_TRUE; 
kal_bool charging_type_det_done = KAL_TRUE;
static CHARGER_TYPE g_charger_type = CHARGER_UNKNOWN;

#if defined(MTK_WIRELESS_CHARGER_SUPPORT)
#define WIRELESS_CHARGER_EXIST_STATE 0
int wireless_charger_gpio_number   = (168 | 0x80000000); 
#endif

const kal_uint32 VBAT_CV_VTH[]=
{
	BATTERY_VOLT_04_200000_V,   BATTERY_VOLT_04_212500_V,	BATTERY_VOLT_04_225000_V,   BATTERY_VOLT_04_237500_V,
	BATTERY_VOLT_04_250000_V,   BATTERY_VOLT_04_262500_V,	BATTERY_VOLT_04_275000_V,   BATTERY_VOLT_04_300000_V,
	BATTERY_VOLT_04_325000_V,	BATTERY_VOLT_04_350000_V,	BATTERY_VOLT_04_375000_V,	BATTERY_VOLT_04_400000_V,
	BATTERY_VOLT_04_425000_V,   BATTERY_VOLT_04_162500_V,	BATTERY_VOLT_04_175000_V,   BATTERY_VOLT_02_200000_V,
	BATTERY_VOLT_04_050000_V,   BATTERY_VOLT_04_100000_V,	BATTERY_VOLT_04_125000_V,   BATTERY_VOLT_03_775000_V,	
	BATTERY_VOLT_03_800000_V,	BATTERY_VOLT_03_850000_V,	BATTERY_VOLT_03_900000_V,	BATTERY_VOLT_04_000000_V,
	BATTERY_VOLT_04_050000_V,	BATTERY_VOLT_04_100000_V,	BATTERY_VOLT_04_125000_V,	BATTERY_VOLT_04_137500_V,
	BATTERY_VOLT_04_150000_V,	BATTERY_VOLT_04_162500_V,	BATTERY_VOLT_04_175000_V,	BATTERY_VOLT_04_187500_V

};

const kal_uint32 CS_VTH[]=
{
	CHARGE_CURRENT_1600_00_MA,   CHARGE_CURRENT_1500_00_MA,	CHARGE_CURRENT_1400_00_MA, CHARGE_CURRENT_1300_00_MA,
	CHARGE_CURRENT_1200_00_MA,   CHARGE_CURRENT_1100_00_MA,	CHARGE_CURRENT_1000_00_MA, CHARGE_CURRENT_900_00_MA,
	CHARGE_CURRENT_800_00_MA,   CHARGE_CURRENT_700_00_MA,	CHARGE_CURRENT_650_00_MA, CHARGE_CURRENT_550_00_MA,
	CHARGE_CURRENT_450_00_MA,   CHARGE_CURRENT_300_00_MA,	CHARGE_CURRENT_200_00_MA, CHARGE_CURRENT_70_00_MA
}; 


const kal_uint32 VCDT_HV_VTH[]=
{
	 BATTERY_VOLT_04_200000_V, BATTERY_VOLT_04_250000_V,	 BATTERY_VOLT_04_300000_V,	 BATTERY_VOLT_04_350000_V,
	 BATTERY_VOLT_04_400000_V, BATTERY_VOLT_04_450000_V,	 BATTERY_VOLT_04_500000_V,	 BATTERY_VOLT_04_550000_V,
	 BATTERY_VOLT_04_600000_V, BATTERY_VOLT_06_000000_V,	 BATTERY_VOLT_06_500000_V,	 BATTERY_VOLT_07_000000_V,
	 BATTERY_VOLT_07_500000_V, BATTERY_VOLT_08_500000_V,	 BATTERY_VOLT_09_500000_V,	 BATTERY_VOLT_10_500000_V		 
};

 // ============================================================ //
 // function prototype
 // ============================================================ //
 
 
 // ============================================================ //
 //extern variable
 // ============================================================ //
 
 // ============================================================ //
 //extern function
 // ============================================================ //
 extern kal_uint32 upmu_get_reg_value(kal_uint32 reg);
 extern void Charger_Detect_Init(void);
 extern void Charger_Detect_Release(void);
 extern int PMIC_IMM_GetOneChannelValue(int dwChannel, int deCount, int trimd);
 extern void mt_power_off(void);
 
 // ============================================================ //
kal_uint32 charging_value_to_parameter(const kal_uint32 *parameter, const kal_uint32 array_size, const kal_uint32 val)
{
	if (val < array_size)
	{
		return parameter[val];
	}
	else
	{
		battery_xlog_printk(BAT_LOG_CRTI, "Can't find the parameter \r\n");	
		return parameter[0];
	}
}

 
 kal_uint32 charging_parameter_to_value(const kal_uint32 *parameter, const kal_uint32 array_size, const kal_uint32 val)
{
	kal_uint32 i;

	for(i=0;i<array_size;i++)
	{
		if (val == *(parameter + i))
		{
				return i;
		}
	}

	 battery_xlog_printk(BAT_LOG_CRTI, "NO register value match \r\n");
	//TODO: ASSERT(0);	// not find the value
	return 0;
}


 static kal_uint32 bmt_find_closest_level(const kal_uint32 *pList,kal_uint32 number,kal_uint32 level)
 {
	 kal_uint32 i;
	 kal_uint32 max_value_in_last_element;
 
	 if(pList[0] < pList[1])
		 max_value_in_last_element = KAL_TRUE;
	 else
		 max_value_in_last_element = KAL_FALSE;
 
	 if(max_value_in_last_element == KAL_TRUE)
	 {
		 for(i = (number-1); i != 0; i--)	 //max value in the last element
		 {
			 if(pList[i] <= level)
			 {
				 return pList[i];
			 }	  
		 }

		 battery_xlog_printk(BAT_LOG_CRTI, "Can't find closest level \r\n");	
		 return pList[0];
		 //return CHARGE_CURRENT_0_00_MA;
	 }
	 else
	 {
		 for(i = 0; i< number; i++)  // max value in the first element
		 {
			 if(pList[i] <= level)
			 {
				 return pList[i];
			 }	  
		 }

 		 battery_xlog_printk(BAT_LOG_CRTI, "Can't find closest level \r\n");
		 return pList[number -1];
		 //return CHARGE_CURRENT_0_00_MA;
	 }
 }

#if defined(CONFIG_POWER_EXT)
#else
static void hw_bc11_dump_register(void)
{
	kal_uint32 reg_val = 0;
	kal_uint32 reg_num = CHR_CON18;
	kal_uint32 i = 0;

	for(i=reg_num ; i<=CHR_CON19 ; i+=2)
	{
		reg_val = upmu_get_reg_value(i);
		battery_xlog_printk(BAT_LOG_FULL, "Chr Reg[0x%x]=0x%x \r\n", i, reg_val);
	}	
}


 static void hw_bc11_init(void)
 {
 	 msleep(300);
	 Charger_Detect_Init();
		 
	 //RG_BC11_BIAS_EN=1	
	 upmu_set_rg_bc11_bias_en(0x1);
	 //RG_BC11_VSRC_EN[1:0]=00
	 upmu_set_rg_bc11_vsrc_en(0x0);
	 //RG_BC11_VREF_VTH = [1:0]=00
	 upmu_set_rg_bc11_vref_vth(0x0);
	 //RG_BC11_CMP_EN[1.0] = 00
	 upmu_set_rg_bc11_cmp_en(0x0);
	 //RG_BC11_IPU_EN[1.0] = 00
	 upmu_set_rg_bc11_ipu_en(0x0);
	 //RG_BC11_IPD_EN[1.0] = 00
	 upmu_set_rg_bc11_ipd_en(0x0);
	 //BC11_RST=1
	 upmu_set_rg_bc11_rst(0x1);
	 //BC11_BB_CTRL=1
	 upmu_set_rg_bc11_bb_ctrl(0x1);
 
 	 //msleep(10);
 	 mdelay(50);
	 
	 if(Enable_BATDRV_LOG == BAT_LOG_FULL)
	 {
    		battery_xlog_printk(BAT_LOG_FULL, "hw_bc11_init() \r\n");
		hw_bc11_dump_register();
	 }	

 }
 
 
 static U32 hw_bc11_DCD(void)
 {
	 U32 wChargerAvail = 0;
 
	 //RG_BC11_IPU_EN[1.0] = 10
	 upmu_set_rg_bc11_ipu_en(0x2);
	 //RG_BC11_IPD_EN[1.0] = 01
	 upmu_set_rg_bc11_ipd_en(0x1);
	 //RG_BC11_VREF_VTH = [1:0]=01
	 upmu_set_rg_bc11_vref_vth(0x1);
	 //RG_BC11_CMP_EN[1.0] = 10
	 upmu_set_rg_bc11_cmp_en(0x2);
 
	 //msleep(20);
	 mdelay(80);

 	 wChargerAvail = upmu_get_rgs_bc11_cmp_out();
	 
	 if(Enable_BATDRV_LOG == BAT_LOG_FULL)
	 {
		battery_xlog_printk(BAT_LOG_FULL, "hw_bc11_DCD() \r\n");
		hw_bc11_dump_register();
	 }
	 
	 //RG_BC11_IPU_EN[1.0] = 00
	 upmu_set_rg_bc11_ipu_en(0x0);
	  //RG_BC11_IPD_EN[1.0] = 00
	 upmu_set_rg_bc11_ipd_en(0x0);
	  //RG_BC11_CMP_EN[1.0] = 00
	 upmu_set_rg_bc11_cmp_en(0x0);
	  //RG_BC11_VREF_VTH = [1:0]=00
	 upmu_set_rg_bc11_vref_vth(0x0);
 
	 return wChargerAvail;
 }
 
 
 static U32 hw_bc11_stepA1(void)
 {
	U32 wChargerAvail = 0;

	//RG_BC11_IPD_EN[1:0] = 01
	upmu_set_rg_bc11_ipd_en(0x1);
	//RG_BC11_VREF_VTH[1:0]=00
	upmu_set_rg_bc11_vref_vth(0x0);
	// RG_BC11_CMP_EN[1:0] = 01
	upmu_set_rg_bc11_cmp_en(0x1);
 
	//msleep(80);
	mdelay(80);
 
	wChargerAvail = upmu_get_rgs_bc11_cmp_out();
 
	if(Enable_BATDRV_LOG == BAT_LOG_FULL)
	{
		battery_xlog_printk(BAT_LOG_FULL, "hw_bc11_stepA1() \r\n");
		hw_bc11_dump_register();
	}
 
	//RG_BC11_IPD_EN[1.0] = 00
	upmu_set_rg_bc11_ipd_en(0x0);
	//RG_BC11_CMP_EN[1.0] = 00
	upmu_set_rg_bc11_cmp_en(0x0);
 
	return  wChargerAvail;
 }
 
 
 static U32 hw_bc11_stepB1(void)
 {
	U32 wChargerAvail = 0;
	  
	//RG_BC11_IPU_EN[1.0] = 01
	//upmu_set_rg_bc11_ipu_en(0x1);
	upmu_set_rg_bc11_ipd_en(0x1);
	//RG_BC11_VREF_VTH = [1:0]=10
	//upmu_set_rg_bc11_vref_vth(0x2);
	upmu_set_rg_bc11_vref_vth(0x0);
	//RG_BC11_CMP_EN[1.0] = 01
	upmu_set_rg_bc11_cmp_en(0x1);
 
	//msleep(80);
	mdelay(80);
 
	wChargerAvail = upmu_get_rgs_bc11_cmp_out();
 
	if(Enable_BATDRV_LOG == BAT_LOG_FULL)
	{
		battery_xlog_printk(BAT_LOG_FULL, "hw_bc11_stepB1() \r\n");
		hw_bc11_dump_register();
	}
 
	//RG_BC11_IPU_EN[1.0] = 00
	upmu_set_rg_bc11_ipu_en(0x0);
	//RG_BC11_CMP_EN[1.0] = 00
	upmu_set_rg_bc11_cmp_en(0x0);
	//RG_BC11_VREF_VTH = [1:0]=00
	upmu_set_rg_bc11_vref_vth(0x0);
 
	return  wChargerAvail;
 }
 
 
 static U32 hw_bc11_stepC1(void)
 {
	U32 wChargerAvail = 0;
	  
	//RG_BC11_IPU_EN[1.0] = 01
	upmu_set_rg_bc11_ipu_en(0x1);
	//RG_BC11_VREF_VTH = [1:0]=10
	upmu_set_rg_bc11_vref_vth(0x2);
	//RG_BC11_CMP_EN[1.0] = 01
	upmu_set_rg_bc11_cmp_en(0x1);
 
	//msleep(80);
	mdelay(80);
 
	wChargerAvail = upmu_get_rgs_bc11_cmp_out();
 
	if(Enable_BATDRV_LOG == BAT_LOG_FULL)
	{
		battery_xlog_printk(BAT_LOG_FULL, "hw_bc11_stepC1() \r\n");
		hw_bc11_dump_register();
	}
 
	//RG_BC11_IPU_EN[1.0] = 00
	upmu_set_rg_bc11_ipu_en(0x0);
	//RG_BC11_CMP_EN[1.0] = 00
	upmu_set_rg_bc11_cmp_en(0x0);
	//RG_BC11_VREF_VTH = [1:0]=00
	upmu_set_rg_bc11_vref_vth(0x0);
 
	return  wChargerAvail;
 }
 
 
 static U32 hw_bc11_stepA2(void)
 {
	U32 wChargerAvail = 0;
	  
	//RG_BC11_VSRC_EN[1.0] = 10 
	upmu_set_rg_bc11_vsrc_en(0x2);
	//RG_BC11_IPD_EN[1:0] = 01
	upmu_set_rg_bc11_ipd_en(0x1);
	//RG_BC11_VREF_VTH = [1:0]=00
	upmu_set_rg_bc11_vref_vth(0x0);
	//RG_BC11_CMP_EN[1.0] = 01
	upmu_set_rg_bc11_cmp_en(0x1);
 
	//msleep(80);
	mdelay(80);
 
	wChargerAvail = upmu_get_rgs_bc11_cmp_out();
 
	if(Enable_BATDRV_LOG == BAT_LOG_FULL)
	{
		battery_xlog_printk(BAT_LOG_FULL, "hw_bc11_stepA2() \r\n");
		hw_bc11_dump_register();
	}
 
	//RG_BC11_VSRC_EN[1:0]=00
	upmu_set_rg_bc11_vsrc_en(0x0);
	//RG_BC11_IPD_EN[1.0] = 00
	upmu_set_rg_bc11_ipd_en(0x0);
	//RG_BC11_CMP_EN[1.0] = 00
	upmu_set_rg_bc11_cmp_en(0x0);
 
	return  wChargerAvail;
 }
 
 
 static U32 hw_bc11_stepB2(void)
 {
	U32 wChargerAvail = 0;
 
	//RG_BC11_IPU_EN[1:0]=10
	upmu_set_rg_bc11_ipu_en(0x2);
	//RG_BC11_VREF_VTH = [1:0]=10
	upmu_set_rg_bc11_vref_vth(0x1);
	//RG_BC11_CMP_EN[1.0] = 01
	upmu_set_rg_bc11_cmp_en(0x1);
 
	//msleep(80);
	mdelay(80);
 
	wChargerAvail = upmu_get_rgs_bc11_cmp_out();
 
	if(Enable_BATDRV_LOG == BAT_LOG_FULL)
	{
		battery_xlog_printk(BAT_LOG_FULL, "hw_bc11_stepB2() \r\n");
		hw_bc11_dump_register();
	}
 
	//RG_BC11_IPU_EN[1.0] = 00
	upmu_set_rg_bc11_ipu_en(0x0);
	//RG_BC11_CMP_EN[1.0] = 00
	upmu_set_rg_bc11_cmp_en(0x0);
	//RG_BC11_VREF_VTH = [1:0]=00
	upmu_set_rg_bc11_vref_vth(0x0);
 
	return  wChargerAvail;
 }
 
 
 static void hw_bc11_done(void)
 {
	//RG_BC11_VSRC_EN[1:0]=00
	upmu_set_rg_bc11_vsrc_en(0x0);
	//RG_BC11_VREF_VTH = [1:0]=0
	upmu_set_rg_bc11_vref_vth(0x0);
	//RG_BC11_CMP_EN[1.0] = 00
	upmu_set_rg_bc11_cmp_en(0x0);
	//RG_BC11_IPU_EN[1.0] = 00
	upmu_set_rg_bc11_ipu_en(0x0);
	//RG_BC11_IPD_EN[1.0] = 00
	upmu_set_rg_bc11_ipd_en(0x0);
	//RG_BC11_BIAS_EN=0
	upmu_set_rg_bc11_bias_en(0x0); 
 
	Charger_Detect_Release();

	if(Enable_BATDRV_LOG == BAT_LOG_FULL)
	{
		battery_xlog_printk(BAT_LOG_FULL, "hw_bc11_done() \r\n");
		hw_bc11_dump_register();
	}
    
 }
#endif

 static kal_uint32 charging_hw_init(void *data)
 {
 	kal_uint32 status = STATUS_OK;
#if defined(MTK_WIRELESS_CHARGER_SUPPORT)
	mt_set_gpio_mode(wireless_charger_gpio_number,0); // 0:GPIO mode
	mt_set_gpio_dir(wireless_charger_gpio_number,0); // 0: input, 1: output
#endif	
   	upmu_set_rg_chrwdt_td(0x0);           // CHRWDT_TD, 4s
	upmu_set_rg_chrwdt_int_en(1);         // CHRWDT_INT_EN
	upmu_set_rg_chrwdt_en(1);             // CHRWDT_EN
	upmu_set_rg_chrwdt_wr(1);             // CHRWDT_WR

	upmu_set_rg_vcdt_mode(0);       //VCDT_MODE
	upmu_set_rg_vcdt_hv_en(1);      //VCDT_HV_EN    

	upmu_set_rg_usbdl_set(0);       //force leave USBDL mode
	upmu_set_rg_usbdl_rst(1);		//force leave USBDL mode

	upmu_set_rg_bc11_bb_ctrl(1);    //BC11_BB_CTRL
	upmu_set_rg_bc11_rst(1);        //BC11_RST
    
	upmu_set_rg_csdac_mode(1);      //CSDAC_MODE
	upmu_set_rg_vbat_ov_en(1);      //VBAT_OV_EN
#ifdef HIGH_BATTERY_VOLTAGE_SUPPORT
	//upmu_set_rg_vbat_ov_vth(0x2);   //VBAT_OV_VTH, 4.4V,
	upmu_set_rg_vbat_ov_vth(0x3);    //LINE <bat> <DATE2014806> <VBAT_OV_VTH, 4.45V> limi.zhang
#else
	upmu_set_rg_vbat_ov_vth(0x1);   //VBAT_OV_VTH, 4.3V,
#endif
	upmu_set_rg_baton_en(1);        //BATON_EN

	//Tim, for TBAT
	//upmu_set_rg_buf_pwd_b(1);       //RG_BUF_PWD_B
	upmu_set_rg_baton_ht_en(0);     //BATON_HT_EN
    
	upmu_set_rg_ulc_det_en(1);      // RG_ULC_DET_EN=1
	upmu_set_rg_low_ich_db(1);      // RG_LOW_ICH_DB=000001'b

	return status;
 }


 static kal_uint32 charging_dump_register(void *data)
 {
 	kal_uint32 status = STATUS_OK;

	kal_uint32 reg_val = 0;
    kal_uint32 reg_num = CHR_CON0;
    kal_uint32 i = 0;

    for(i=reg_num ; i<=CHR_CON29 ; i+=2)
    {
        reg_val = upmu_get_reg_value(i);
        battery_xlog_printk(BAT_LOG_FULL, "Chr Reg[0x%x]=0x%x \r\n", i, reg_val);
    }

	return status;
 }
 	

 static kal_uint32 charging_enable(void *data)
 {
 	kal_uint32 status = STATUS_OK;
	kal_uint32 enable = *(kal_uint32*)(data);

	if(KAL_TRUE == enable)
	{
	    upmu_set_rg_csdac_dly(0x4);             // CSDAC_DLY
	    upmu_set_rg_csdac_stp(0x1);             // CSDAC_STP
	    upmu_set_rg_csdac_stp_inc(0x1);         // CSDAC_STP_INC
	    upmu_set_rg_csdac_stp_dec(0x2);         // CSDAC_STP_DEC
	    upmu_set_rg_cs_en(1);                   // CS_EN, check me

	    upmu_set_rg_hwcv_en(1);
	    
	    upmu_set_rg_vbat_cv_en(1);              // CV_EN
	    upmu_set_rg_csdac_en(1);                // CSDAC_EN

	    upmu_set_rg_chr_en(1); 				// CHR_EN

	    if(Enable_BATDRV_LOG == BAT_LOG_FULL)
	        charging_dump_register(NULL);
	}
	else
	{
	    upmu_set_rg_chrwdt_int_en(0);    // CHRWDT_INT_EN
	    upmu_set_rg_chrwdt_en(0);        // CHRWDT_EN
	    upmu_set_rg_chrwdt_flag_wr(0);   // CHRWDT_FLAG
	    
	    upmu_set_rg_csdac_en(0);         // CSDAC_EN
	    upmu_set_rg_chr_en(0);           // CHR_EN
	    upmu_set_rg_hwcv_en(0);          // RG_HWCV_EN
	}
	return status;
 }


 static kal_uint32 charging_set_cv_voltage(void *data)
 {
 	kal_uint32 status = STATUS_OK;
	kal_uint16 register_value;
	
	register_value = charging_parameter_to_value(VBAT_CV_VTH, GETARRAYNUM(VBAT_CV_VTH) ,*(kal_uint32 *)(data));
	upmu_set_rg_vbat_cv_vth(register_value); 

	return status;
 } 	


 static kal_uint32 charging_get_current(void *data)
 {
	kal_uint32 status = STATUS_OK;
	kal_uint32 array_size;
	kal_uint32 reg_value;
	
	array_size = GETARRAYNUM(CS_VTH);
	reg_value=upmu_get_reg_value(0x8);	//RG_CS_VTH
	*(kal_uint32 *)data = charging_value_to_parameter(CS_VTH,array_size,reg_value);
	
	return status;
 }  
  


 static kal_uint32 charging_set_current(void *data)
 {
 	kal_uint32 status = STATUS_OK;
	kal_uint32 set_chr_current;
	kal_uint32 array_size;
	kal_uint32 register_value;
	
	array_size = GETARRAYNUM(CS_VTH);
	set_chr_current = bmt_find_closest_level(CS_VTH, array_size, *(kal_uint32 *)data);
	register_value = charging_parameter_to_value(CS_VTH, array_size ,set_chr_current);
	upmu_set_rg_cs_vth(register_value);

	return status;
 } 	


static kal_uint32 charging_set_input_current(void *data)
{
 	kal_uint32 status = STATUS_OK;
	return status;
} 	

static kal_uint32 charging_get_charging_status(void *data)
{
	kal_uint32 status = STATUS_OK;
	return status;
}

 
static kal_uint32 charging_reset_watch_dog_timer(void *data)
{
    kal_uint32 status = STATUS_OK;

    upmu_set_rg_chrwdt_td(0x0);           // CHRWDT_TD, 4s
    upmu_set_rg_chrwdt_wr(1);             // CHRWDT_WR
    upmu_set_rg_chrwdt_int_en(1);         // CHRWDT_INT_EN
    upmu_set_rg_chrwdt_en(1);             // CHRWDT_EN
    upmu_set_rg_chrwdt_flag_wr(1);        // CHRWDT_WR
    
    return status;
}


 static kal_uint32 charging_set_hv_threshold(void *data)
 {
	kal_uint32 status = STATUS_OK;

	kal_uint32 set_hv_voltage;
	kal_uint32 array_size;
	kal_uint16 register_value;
	kal_uint32 voltage = *(kal_uint32*)(data);
	
	array_size = GETARRAYNUM(VCDT_HV_VTH);
	set_hv_voltage = bmt_find_closest_level(VCDT_HV_VTH, array_size, voltage);
	register_value = charging_parameter_to_value(VCDT_HV_VTH, array_size ,set_hv_voltage);
	upmu_set_rg_vcdt_hv_vth(register_value);

	return status;
 }


 static kal_uint32 charging_get_hv_status(void *data)
 {
	  kal_uint32 status = STATUS_OK;

	  *(kal_bool*)(data) = upmu_get_rgs_vcdt_hv_det();
	  
	  return status;
 }

		
static kal_uint32 charging_get_battery_status(void *data)
 {
	  kal_uint32 status = STATUS_OK;

	  upmu_set_baton_tdet_en(1);
	  upmu_set_rg_baton_en(1);
	  *(kal_bool*)(data) = upmu_get_rgs_baton_undet();
	  
	  return status;
 }

	
static kal_uint32 charging_get_charger_det_status(void *data)
 {
	   kal_uint32 status = STATUS_OK;
 
#if defined(CHRDET_SW_MODE_EN)
    kal_uint32 vchr_val=0;

    vchr_val = PMIC_IMM_GetOneChannelValue(4,5,1);
    vchr_val = (((330+39)*100*vchr_val)/39)/100;

    if( vchr_val > 4300 )
    {
        battery_xlog_printk(BAT_LOG_FULL, "[CHRDET_SW_WORKAROUND_EN] upmu_is_chr_det=Y (%d)\n", vchr_val);
        *(kal_uint32 *)data = KAL_TRUE;	
    }
    else
    {
        battery_xlog_printk(BAT_LOG_FULL, "[CHRDET_SW_WORKAROUND_EN] upmu_is_chr_det=N (%d)\n", vchr_val);
        *(kal_uint32 *)data = KAL_FALSE;
    }        
#else
	   *(kal_bool*)(data) = upmu_get_rgs_chrdet();
#endif
	   
       if( upmu_get_rgs_chrdet() == 0 )
           g_charger_type = CHARGER_UNKNOWN;
       
	   return status;
 }


kal_bool charging_type_detection_done(void)
 {
	 return charging_type_det_done;
 }

#if 0 //defined(TINNO_QUICK_CHARGING)
#define PAD_AUX_XP				13
static unsigned int dm_adc_read_vol(void)
{
	unsigned int data[4] = {0, 0, 0, 0};
	unsigned int tmp = 0, rc = 0, iVoltage = 0;
	rc = IMM_GetOneChannelValue(PAD_AUX_XP, data, &tmp);
	if(rc < 0) {
		battery_xlog_printk(BAT_LOG_CRTI,"dm_adc_read_vol ==> read dm vol error \r\n");
		return 0;
	}
	else {
		iVoltage = (data[0]*1000) + (data[1]*10) + (data[2]);
		battery_xlog_printk(BAT_LOG_CRTI,"dm_adc_read_vol == >read dm voltage success, data[0]=%d, data[1]=%d, data[2]=%d, data[3]=%d, iVoltage=%d \r\n", 
			data[0], data[1], data[2], data[3], iVoltage);
		
	}
	return iVoltage;
}


void dm_char_turnonoff(unsigned int onoff)
{
    battery_xlog_printk(BAT_LOG_CRTI,"dm_char_turnonoff onoff = %d\n",onoff);
    mt_set_gpio_mode(GPIO_DM_VOL_DET_PIN,GPIO_MODE_00);  // gpio mode
    mt_set_gpio_pull_enable(GPIO_DM_VOL_DET_PIN,GPIO_PULL_ENABLE);
	if(onoff)
	{
        mt_set_gpio_dir(GPIO_DM_VOL_DET_PIN,GPIO_DIR_OUT); // output
        mt_set_gpio_out(GPIO_DM_VOL_DET_PIN,GPIO_OUT_ONE); // high
    }
    else
    {
        mt_set_gpio_dir(GPIO_DM_VOL_DET_PIN,GPIO_DIR_OUT); // output
        mt_set_gpio_out(GPIO_DM_VOL_DET_PIN,GPIO_OUT_ZERO); // high
    }
}

#endif

 static kal_uint32 charging_get_charger_type(void *data)
 {
     
#if 0 //defined(TINNO_QUICK_CHARGING)
     unsigned int dm_vol = 0;
#endif 
	 kal_uint32 status = STATUS_OK;
#if defined(CONFIG_POWER_EXT)
	 *(CHARGER_TYPE*)(data) = STANDARD_HOST;
#else
#if defined(MTK_WIRELESS_CHARGER_SUPPORT)
    int wireless_state = 0;
    wireless_state = mt_get_gpio_in(wireless_charger_gpio_number);
    if(wireless_state == WIRELESS_CHARGER_EXIST_STATE)
    {
        *(CHARGER_TYPE*)(data) = WIRELESS_CHARGER;
        battery_xlog_printk(BAT_LOG_CRTI, "WIRELESS_CHARGER!\r\n");
        return status;
    }
#endif

    if(g_charger_type!=CHARGER_UNKNOWN && g_charger_type!=WIRELESS_CHARGER)
    {
        *(CHARGER_TYPE*)(data) = g_charger_type;
        battery_xlog_printk(BAT_LOG_CRTI, "return %d!\r\n", g_charger_type);
        return status;
    }

	charging_type_det_done = KAL_FALSE;

    #if 0 //defined(TINNO_QUICK_CHARGING)
    dm_char_turnonoff(1);
    dm_vol = dm_adc_read_vol();
    battery_xlog_printk(BAT_LOG_CRTI,"dm_vol start(1) = %d\r\n", dm_vol);
    dm_char_turnonoff(0);
    #endif
     

	

	/********* Step initial  ***************/		 
	hw_bc11_init();

	/********* Step DCD ***************/  
	if(1 == hw_bc11_DCD())
	{
		/********* Step A1 ***************/
		if(1 == hw_bc11_stepA1())
		{
		    #if defined(TINNO_QUICK_CHARGING)
		    *(CHARGER_TYPE*)(data) = TINNO_1_5A_CHARGER;
			battery_xlog_printk(BAT_LOG_CRTI, "step A1 : tinno special CHARGER!\r\n");
		    #else
			*(CHARGER_TYPE*)(data) = APPLE_2_1A_CHARGER;
			battery_xlog_printk(BAT_LOG_CRTI, "step A1 : Apple 2.1A CHARGER!\r\n");
			#endif
		    	
		}	 
		else
		{
			*(CHARGER_TYPE*)(data) = NONSTANDARD_CHARGER;
			battery_xlog_printk(BAT_LOG_CRTI, "step A1 : Non STANDARD CHARGER!\r\n");
		}
	}
	else
	{
		 /********* Step A2 ***************/
		 if(1 == hw_bc11_stepA2())
		 {
			 /********* Step B2 ***************/
			 if(1 == hw_bc11_stepB2())
			 {
				 *(CHARGER_TYPE*)(data) = STANDARD_CHARGER;
				 battery_xlog_printk(BAT_LOG_CRTI, "step B2 : STANDARD CHARGER!\r\n");
			 }
			 else
			 {
				 *(CHARGER_TYPE*)(data) = CHARGING_HOST;
				 battery_xlog_printk(BAT_LOG_CRTI, "step B2 :  Charging Host!\r\n");
			 }
		 }
		 else
		 {
			*(CHARGER_TYPE*)(data) = STANDARD_HOST;
			 battery_xlog_printk(BAT_LOG_CRTI, "step A2 : Standard USB Host!\r\n");
		 }
 
	}
 
	 /********* Finally setting *******************************/
	 hw_bc11_done();

	#if 0 //defined(TINNO_QUICK_CHARGING)
    dm_char_turnonoff(1);
    dm_vol = dm_adc_read_vol();
    battery_xlog_printk(BAT_LOG_CRTI,"dm_vol done(2) = %d\r\n", dm_vol);
    dm_char_turnonoff(0);
    #endif

 	charging_type_det_done = KAL_TRUE;

	g_charger_type = *(CHARGER_TYPE*)(data);
#endif
	 return status;
}

 static kal_uint32 charging_get_is_pcm_timer_trigger(void *data)
 {
     kal_uint32 status = STATUS_OK;
 
#if defined(CONFIG_MTK_FPGA) 
     battery_xlog_printk(BAT_LOG_CRTI, "[Early porting] no slp_get_wake_reason at FPGA\n");
 #else
     if(slp_get_wake_reason() == WR_PCM_TIMER)
         *(kal_bool*)(data) = KAL_TRUE;
     else
         *(kal_bool*)(data) = KAL_FALSE;
 
     battery_xlog_printk(BAT_LOG_CRTI, "slp_get_wake_reason=%d\n", slp_get_wake_reason());
#endif
        
     return status;
 }
 
 static kal_uint32 charging_set_platform_reset(void *data)
 {
     kal_uint32 status = STATUS_OK;
 
     battery_xlog_printk(BAT_LOG_CRTI, "charging_set_platform_reset\n");
  
     arch_reset(0,NULL);
         
     return status;
 }
 
 static kal_uint32 charging_get_platfrom_boot_mode(void *data)
 {
     kal_uint32 status = STATUS_OK;
   
     *(kal_uint32*)(data) = get_boot_mode();
 
     battery_xlog_printk(BAT_LOG_CRTI, "get_boot_mode=%d\n", get_boot_mode());
          
     return status;
 }

 static kal_uint32 charging_set_power_off(void *data)
 {
     kal_uint32 status = STATUS_OK;
  
     battery_xlog_printk(BAT_LOG_CRTI, "charging_set_power_off=%d\n");
     mt_power_off();
         
     return status;
 }

 static kal_uint32 (*charging_func[CHARGING_CMD_NUMBER])(void *data)=
 {
 	 charging_hw_init
	,charging_dump_register  	
	,charging_enable
	,charging_set_cv_voltage
	,charging_get_current
	,charging_set_current
	,charging_set_input_current		// not support, empty function
	,charging_get_charging_status	// not support, empty function
	,charging_reset_watch_dog_timer
	,charging_set_hv_threshold
	,charging_get_hv_status
	,charging_get_battery_status
	,charging_get_charger_det_status
	,charging_get_charger_type
	,charging_get_is_pcm_timer_trigger
	,charging_set_platform_reset
	,charging_get_platfrom_boot_mode
	,charging_set_power_off
 };
 
 
 /*
 * FUNCTION
 *		Internal_chr_control_handler
 *
 * DESCRIPTION															 
 *		 This function is called to set the charger hw
 *
 * CALLS  
 *
 * PARAMETERS
 *		None
 *	 
 * RETURNS
 *		
 *
 * GLOBALS AFFECTED
 *	   None
 */
 kal_int32 chr_control_interface(CHARGING_CTRL_CMD cmd, void *data)
 {
	 kal_int32 status;
	 if(cmd < CHARGING_CMD_NUMBER)
		 status = charging_func[cmd](data);
	 else
		 return STATUS_UNSUPPORTED;
 
	 return status;
 }


