#include <mach/charging.h>
#include <mach/mt6333.h>
#include <mach/upmu_common.h>
#include <mach/mt_gpio.h>
#include <cust_gpio_usage.h>
#include <mach/upmu_hw.h>
#include <linux/xlog.h>
#include <linux/delay.h>
#include <mach/mt_sleep.h>
#include <mach/mt_boot.h>
#include <mach/system.h>
#include <cust_charging.h>

// ============================================================ //
//define
// ============================================================ //
#define STATUS_OK	0
#define STATUS_UNSUPPORTED	-1
#define GETARRAYNUM(array) (sizeof(array)/sizeof(array[0]))

#define MTK_SWCHR_SW_RECHR
#define SWCHR_HW_RECHR_VOLTAGE 4050

// ============================================================ //
//global variable
// ============================================================ //
kal_bool charging_type_det_done = KAL_TRUE;
static CHARGER_TYPE g_charger_type = CHARGER_UNKNOWN;
#if defined(MTK_WIRELESS_CHARGER_SUPPORT)
#define WIRELESS_CHARGER_EXIST_STATE 0
int wireless_charger_gpio_number   = (168 | 0x80000000); 
#endif

const kal_uint32 VBAT_CV_VTH[]=
{
    BATTERY_VOLT_04_450000_V, BATTERY_VOLT_04_350000_V,	BATTERY_VOLT_04_300000_V, BATTERY_VOLT_04_200000_V,
    BATTERY_VOLT_04_100000_V, BATTERY_VOLT_04_000000_V,	BATTERY_VOLT_03_900000_V, BATTERY_VOLT_03_800000_V,        
    BATTERY_VOLT_03_700000_V, BATTERY_VOLT_03_600000_V
};

const kal_uint32 CS_VTH[]=
{
    CHARGE_CURRENT_100_00_MA,   CHARGE_CURRENT_200_00_MA,	CHARGE_CURRENT_300_00_MA,  CHARGE_CURRENT_400_00_MA,
    CHARGE_CURRENT_500_00_MA,   CHARGE_CURRENT_600_00_MA,	CHARGE_CURRENT_700_00_MA,  CHARGE_CURRENT_800_00_MA,
    CHARGE_CURRENT_900_00_MA,   CHARGE_CURRENT_1000_00_MA,	CHARGE_CURRENT_1100_00_MA, CHARGE_CURRENT_1200_00_MA,
    CHARGE_CURRENT_1300_00_MA,  CHARGE_CURRENT_1400_00_MA,	CHARGE_CURRENT_1500_00_MA, CHARGE_CURRENT_1600_00_MA
}; 

const kal_uint32 INPUT_CS_VTH[]=
{
    CHARGE_CURRENT_100_00_MA,   CHARGE_CURRENT_200_00_MA,	CHARGE_CURRENT_300_00_MA,  CHARGE_CURRENT_400_00_MA,
    CHARGE_CURRENT_500_00_MA,   CHARGE_CURRENT_600_00_MA,	CHARGE_CURRENT_700_00_MA,  CHARGE_CURRENT_800_00_MA,
    CHARGE_CURRENT_900_00_MA,   CHARGE_CURRENT_1000_00_MA,	CHARGE_CURRENT_1100_00_MA, CHARGE_CURRENT_1200_00_MA,
    CHARGE_CURRENT_1300_00_MA,  CHARGE_CURRENT_1400_00_MA,	CHARGE_CURRENT_1500_00_MA, CHARGE_CURRENT_1600_00_MA
}; 

const kal_uint32 VCDT_HV_VTH[]=
{
    BATTERY_VOLT_10_000000_V, BATTERY_VOLT_09_000000_V,	  BATTERY_VOLT_08_000000_V,   BATTERY_VOLT_07_000000_V		  
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
extern bool mt_usb_is_device(void);
extern void Charger_Detect_Init(void);
extern void Charger_Detect_Release(void);
extern int PMIC_IMM_GetOneChannelValue(int dwChannel, int deCount, int trimd);
extern kal_uint32 mt6333_get_reg_value(kal_uint32 reg);

// ============================================================ //
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


static kal_uint32 charging_hw_init(void *data)
{
    kal_uint32 status = STATUS_OK;
    int polling_timeout_value = 10;
    int polling_time = 0;

#if defined(MTK_WIRELESS_CHARGER_SUPPORT)
	mt_set_gpio_mode(wireless_charger_gpio_number,0); // 0:GPIO mode
	mt_set_gpio_dir(wireless_charger_gpio_number,0); // 0: input, 1: output
#endif
    mt6333_set_rg_usbdl_mode_b(1);

    while(mt6333_get_rgs_power_on_ready() != 1)
    {
        if(polling_time++ >= polling_timeout_value)
        {
            battery_xlog_printk(BAT_LOG_FULL, "check rgs_power_on_ready fail\n");
            break;
        }
    }
    battery_xlog_printk(BAT_LOG_CRTI, "polling_time=%d of rgs_power_on_ready\n", polling_time);

#if defined(HIGH_BATTERY_VOLTAGE_SUPPORT)
    mt6333_set_rg_cv_sel(0);
    battery_xlog_printk(BAT_LOG_CRTI, "HIGH_BATTERY_VOLTAGE_SUPPORT\n");
#endif
        
    return status;
}

static kal_uint32 charging_dump_register(void *data)
{
    kal_uint32 status = STATUS_OK;
        
    //mt6333_dump_register();
      	
    return status;
}	

static kal_uint32 charging_enable(void *data)
{
    kal_uint32 status = STATUS_OK;
    kal_uint32 enable = *(kal_uint32*)(data);
    kal_uint32 ret = 0;

    if(KAL_TRUE == enable)
    {
        //mt6333_set_rg_chr_en(1);
        ret=mt6333_config_interface(0x04, 0x95, 0xFF, 0x0);
    }
    else
    {

#if defined(CONFIG_USB_MTK_HDRC_HCD)
        if(mt_usb_is_device())
#endif 			
        {
            //mt6333_set_rg_chr_en(0);
            ret=mt6333_config_interface(0x04, 0x94, 0xFF, 0x0);
        }
    }
	
    return status;
}

static kal_uint32 charging_set_cv_voltage(void *data)
{
    kal_uint32 status = STATUS_OK;
    kal_uint16 register_value;    
    
    register_value = charging_parameter_to_value(VBAT_CV_VTH, GETARRAYNUM(VBAT_CV_VTH) ,*(kal_uint32 *)(data));

    mt6333_set_rg_cv_sel(register_value); 
    
    return status;
} 	

static kal_uint32 charging_get_current(void *data)
{
    kal_uint32 status = STATUS_OK;
    kal_uint32 array_size;
    
    array_size = GETARRAYNUM(CS_VTH);
    *(kal_uint32 *)data = bmt_find_closest_level(CS_VTH, array_size, *(kal_uint32 *)data);
    
    return status;
}  

static kal_uint32 charging_set_current(void *data)
{
    kal_uint32 status = STATUS_OK;
    kal_uint32 set_chr_current;
    kal_uint32 array_size;
    kal_uint32 register_value;
    kal_uint32 current_value = *(kal_uint32 *)data;
   
    array_size = GETARRAYNUM(CS_VTH);
    set_chr_current = bmt_find_closest_level(CS_VTH, array_size, current_value);
    register_value = charging_parameter_to_value(CS_VTH, array_size ,set_chr_current);

    mt6333_set_rg_ich_sel(register_value);
   
    return status;
} 	

static kal_uint32 charging_set_input_current(void *data)
{
    kal_uint32 status = STATUS_OK;
    kal_uint32 set_chr_current;
    kal_uint32 array_size;
    kal_uint32 register_value;
    
    //20130604, Quick charging by Tim
    mt6333_set_rg_input_cc_reg(1);
    
    array_size = GETARRAYNUM(INPUT_CS_VTH);
    set_chr_current = bmt_find_closest_level(INPUT_CS_VTH, array_size, *(kal_uint32 *)data);
    register_value = charging_parameter_to_value(INPUT_CS_VTH, array_size ,set_chr_current);  
    
    mt6333_set_rg_ich_sel(register_value);

    return status;
} 	

static kal_uint32 charging_get_charging_status(void *data)
{
    kal_uint32 status = STATUS_OK;
    kal_uint32 ret_val = 0;
    
#ifdef MTK_SWCHR_SW_RECHR

    int vbat_bal = 0;
    kal_uint32 ret = 0;
    ret=mt6333_config_interface(0x2E, 0x00, 0xFF, 0x0);
    
    ret_val = mt6333_get_rgs_charge_complete_hw();
    if(ret_val == 0x1)
    {
        *(kal_uint32 *)data = KAL_TRUE;
        ret=mt6333_config_interface(0x04, 0x94, 0xFF, 0x0);

        vbat_bal = PMIC_IMM_GetOneChannelValue(7,3,1);
        if(vbat_bal <= SWCHR_HW_RECHR_VOLTAGE)
        {
            ret=mt6333_config_interface(0x04, 0x95, 0xFF, 0x0);
        }

        battery_xlog_printk(BAT_LOG_CRTI, "[MTK_SWCHR_SW_RECHR] Reg[0x04]=0x%x, Reg[0x19]=0x%x, Reg[0x2E]=0x%x, vbat_bal=%d, SWCHR_HW_RECHR_VOLTAGE=%d\n", 
            mt6333_get_reg_value(0x04), mt6333_get_reg_value(0x19), mt6333_get_reg_value(0x2E), vbat_bal, SWCHR_HW_RECHR_VOLTAGE);
    }
    else
        *(kal_uint32 *)data = KAL_FALSE;   

#else

    ret_val = mt6333_get_rgs_charge_complete_hw();    
    if(ret_val == 0x1)
        *(kal_uint32 *)data = KAL_TRUE;
    else
        *(kal_uint32 *)data = KAL_FALSE;
    
#endif
    
    return status;
} 	

static kal_uint32 charging_reset_watch_dog_timer(void *data)
{
    kal_uint32 status = STATUS_OK;
    kal_uint32 ret = 0;
    kal_uint8 val=0;
    int vbat_bal = 0;
    int check_vbat = 3950;

#if 1
    // cable plug-out sw workaround
    ret=mt6333_read_interface(0x04, (&val),0x1, 0);
    if(val==1)
    {
        vbat_bal = PMIC_IMM_GetOneChannelValue(7,3,1);
        if(vbat_bal > check_vbat)
        {        
            ret=mt6333_config_interface(0x04, 0x94, 0xFF, 0x0);
            mdelay(50);
            ret=mt6333_config_interface(0x04, 0x95, 0xFF, 0x0);        
        }        
    }
    battery_xlog_printk(BAT_LOG_CRTI, "chr_en=%d, curr_vbat=%d, check_vbat=%d\n", val, vbat_bal, check_vbat);
#endif    
    
    mt6333_set_rg_chrwdt_wr(1); // write 1 to kick chr wdt
    mt6333_set_rg_chrwdt_td(1); // 32s
    mt6333_set_rg_chrwdt_en(1); 
    
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
    
    mt6333_set_rg_chrin_hv_vth(register_value);
    
    return status;
}
 
static kal_uint32 charging_get_hv_status(void *data)
{
    kal_uint32 status = STATUS_OK;

    *(kal_bool*)(data) = mt6333_get_rgs_chr_hv_det();
   
    return status;
}

static kal_uint32 charging_get_battery_status(void *data)
{
    kal_uint32 status = STATUS_OK;

    // use mt6323
    upmu_set_baton_tdet_en(1);
    upmu_set_rg_baton_en(1);
    *(kal_bool*)(data) = upmu_get_rgs_baton_undet();
   
    return status;
}

static kal_uint32 charging_get_charger_det_status(void *data)
{
    kal_uint32 status = STATUS_OK;
    
    //*(kal_bool*)(data) = mt6333_get_rgs_chrdet();
    
    // use mt6323
    *(kal_bool*)(data) = upmu_get_rgs_chrdet();
   if( upmu_get_rgs_chrdet() == 0 )
           g_charger_type = CHARGER_UNKNOWN;

    return status;
}


kal_bool charging_type_detection_done(void)
{
	 return charging_type_det_done;
}


static kal_uint32 charging_get_charger_type(void *data)
{
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

	/********* Step initial  ***************/		 
	hw_bc11_init();
 
	/********* Step DCD ***************/  
	if(1 == hw_bc11_DCD())
	{
		/********* Step A1 ***************/
		if(1 == hw_bc11_stepA1())
		{
			*(CHARGER_TYPE*)(data) = APPLE_2_1A_CHARGER;
			battery_xlog_printk(BAT_LOG_CRTI, "step A1 : Apple 2.1A CHARGER!\r\n");
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

 	charging_type_det_done = KAL_TRUE;

    g_charger_type = *(CHARGER_TYPE*)(data);
#endif
	return status;    
}

static kal_uint32 charging_get_is_pcm_timer_trigger(void *data)
{
    kal_uint32 status = STATUS_OK;

    if(slp_get_wake_reason() == WR_PCM_TIMER)
        *(kal_bool*)(data) = KAL_TRUE;
    else
        *(kal_bool*)(data) = KAL_FALSE;

    battery_xlog_printk(BAT_LOG_CRTI, "slp_get_wake_reason=%d\n", slp_get_wake_reason());
       
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

static kal_uint32 (* const charging_func[CHARGING_CMD_NUMBER])(void *data)=
{
    charging_hw_init
    ,charging_dump_register  	
    ,charging_enable
    ,charging_set_cv_voltage
    ,charging_get_current
    ,charging_set_current
    ,charging_set_input_current
    ,charging_get_charging_status
    ,charging_reset_watch_dog_timer
    ,charging_set_hv_threshold
    ,charging_get_hv_status
    ,charging_get_battery_status
    ,charging_get_charger_det_status
    ,charging_get_charger_type
    ,charging_get_is_pcm_timer_trigger
	,charging_set_platform_reset
	,charging_get_platfrom_boot_mode
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
