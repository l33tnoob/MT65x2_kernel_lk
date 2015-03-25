#include <platform/mt_typedefs.h>
#include <platform/mt_reg_base.h>
#include <platform/mt_pmic.h>
#include <platform/mt_gpt.h>
#include <platform/mt_pmic_wrap_init.h>
#include <printf.h>

//==============================================================================
// Global variable
//==============================================================================
int Enable_PMIC_LOG = 1;

CHARGER_TYPE g_ret = CHARGER_UNKNOWN;
int g_charger_in_flag = 0;
int g_first_check=0;

extern int g_R_BAT_SENSE;
extern int g_R_I_SENSE;
extern int g_R_CHARGER_1;
extern int g_R_CHARGER_2;

#ifdef MTK_MT6333_SUPPORT
extern void mt6333_init (void);
#endif

//==============================================================================
// PMIC access API
//==============================================================================
U32 pmic_read_interface (U32 RegNum, U32 *val, U32 MASK, U32 SHIFT)
{
    U32 return_value = 0;    
    U32 pmic6323_reg = 0;
    U32 rdata;    

    //mt6323_read_byte(RegNum, &pmic6323_reg);
    return_value= pwrap_wacs2(0, (RegNum), 0, &rdata);
    pmic6323_reg=rdata;
    if(return_value!=0)
    {   
        printf("[pmic_read_interface] Reg[%x]= pmic_wrap read data fail\n", RegNum);
        return return_value;
    }
    //printf("[pmic_read_interface] Reg[%x]=0x%x\n", RegNum, pmic6323_reg);
    
    pmic6323_reg &= (MASK << SHIFT);
    *val = (pmic6323_reg >> SHIFT);    
    //printf("[pmic_read_interface] val=0x%x\n", *val);

    return return_value;
}

U32 pmic_config_interface (U32 RegNum, U32 val, U32 MASK, U32 SHIFT)
{
    U32 return_value = 0;    
    U32 pmic6323_reg = 0;
    U32 rdata;

    //1. mt6323_read_byte(RegNum, &pmic6323_reg);
    return_value= pwrap_wacs2(0, (RegNum), 0, &rdata);
    pmic6323_reg=rdata;    
    if(return_value!=0)
    {   
        printf("[pmic_config_interface] Reg[%x]= pmic_wrap read data fail\n", RegNum);
        return return_value;
    }
    //printf("[pmic_config_interface] Reg[%x]=0x%x\n", RegNum, pmic6323_reg);
    
    pmic6323_reg &= ~(MASK << SHIFT);
    pmic6323_reg |= (val << SHIFT);

    //2. mt6323_write_byte(RegNum, pmic6323_reg);
    return_value= pwrap_wacs2(1, (RegNum), pmic6323_reg, &rdata);
    if(return_value!=0)
    {   
        printf("[pmic_config_interface] Reg[%x]= pmic_wrap read data fail\n", RegNum);
        return return_value;
    }
    //printf("[pmic_config_interface] write Reg[%x]=0x%x\n", RegNum, pmic6323_reg);    

#if 0
    //3. Double Check    
    //mt6323_read_byte(RegNum, &pmic6323_reg);
    return_value= pwrap_wacs2(0, (RegNum), 0, &rdata);
    pmic6323_reg=rdata;    
    if(return_value!=0)
    {   
        printf("[pmic_config_interface] Reg[%x]= pmic_wrap write data fail\n", RegNum);
        return return_value;
    }
    printf("[pmic_config_interface] Reg[%x]=0x%x\n", RegNum, pmic6323_reg);
#endif    

    return return_value;
}

//==============================================================================
// PMIC APIs
//==============================================================================


//==============================================================================
// PMIC6323 Usage APIs
//==============================================================================
kal_bool upmu_is_chr_det(void)
{
	U32 tmp32=0;
	tmp32=upmu_get_rgs_chrdet();
	if(tmp32 == 0)
	{		
		return KAL_FALSE;
	}
	else
	{	
		return KAL_TRUE;
	}
}

kal_bool pmic_chrdet_status(void)
{
    if( upmu_is_chr_det() == KAL_TRUE )    
    {
        #ifndef USER_BUILD
        printf("[pmic_chrdet_status] Charger exist\r\n");
        #endif
        
        return KAL_TRUE;
    }
    else
    {
        #ifndef USER_BUILD
        printf("[pmic_chrdet_status] No charger\r\n");
        #endif
        
        return KAL_FALSE;
    }
}

int pmic_detect_powerkey(void)
{
    U32 ret=0;
    U32 val=0;

    ret=pmic_read_interface( (U32)(CHRSTATUS),
                             (&val),
                             (U32)(PMIC_PWRKEY_DEB_MASK),
                             (U32)(PMIC_PWRKEY_DEB_SHIFT)
                             );

    if(Enable_PMIC_LOG>1) 
      printf("%d", ret);

    if (val==1){
        #ifndef USER_BUILD
        printf("LK pmic powerkey Release\n");
        #endif
        
        return 0;
    }else{
        #ifndef USER_BUILD
        printf("LK pmic powerkey Press\n");
        #endif
        
        return 1;
    }
}

int pmic_detect_homekey(void)
{
    U32 ret=0;
    U32 val=0;

    ret=pmic_read_interface( (U32)(CHRSTATUS),
                             (&val),
                             (U32)(PMIC_FCHRKEY_DEB_MASK),
                             (U32)(PMIC_FCHRKEY_DEB_SHIFT)
                             );

    if(Enable_PMIC_LOG>1) 
      printf("%d", ret);

    if (val==1){     
        #ifndef USER_BUILD
        printf("LK pmic FCHRKEY Release\n");
        #endif
        
        return 0;
    }else{
        #ifndef USER_BUILD
        printf("LK pmic FCHRKEY Press\n");
        #endif
        
        return 1;
    }
}

kal_uint32 upmu_get_reg_value(kal_uint32 reg)
{
	U32 ret=0;
	U32 temp_val=0;

	ret=pmic_read_interface(reg, &temp_val, 0xFFFF, 0x0);

    if(Enable_PMIC_LOG>1) 
      printf("%d", ret);

	return temp_val;
}

void PMIC_DUMP_ALL_Register(void)
{
    U32 i=0;
    U32 ret=0;
    U32 reg_val=0;

    for (i=0;i<0x800;i++)
    {
        ret=pmic_read_interface(i,&reg_val,0xFFFF,0);
        printf("Reg[0x%x]=0x%x, %d\n", i, reg_val, ret);
    }
}

//==============================================================================
// PMIC6323 Init Code
//==============================================================================
void PMIC_INIT_SETTING_V1(void)
{
    U32 chip_version = 0;

    chip_version = upmu_get_cid();

    if(chip_version >= PMIC6323_E1_CID_CODE)
    {
        printf("[LK_PMIC_INIT_SETTING_V1] PMIC Chip = 0x%x\n",chip_version);

        //put init setting from DE/SA        
    }
    else
    {
        printf("[LK_PMIC_INIT_SETTING_V1] Unknown PMIC Chip (%x)\n",chip_version);
    }
}

void PMIC_CUSTOM_SETTING_V1(void)
{
    //printf("[PMIC_CUSTOM_SETTING_V1] \n");
}

U32 pmic_init (void)
{
    U32 ret_code = PMIC_TEST_PASS;

	  //printf("\n[LK_pmic6323_init] LK Start...................\n");

	  upmu_set_rg_chrind_on(0);    
	  //printf("[LK_PMIC_INIT] Turn Off chrind\n");

	  PMIC_INIT_SETTING_V1();
	  //printf("[LK_PMIC_INIT_SETTING_V1] Done\n");	
    
	  PMIC_CUSTOM_SETTING_V1();
	  //printf("[LK_PMIC_CUSTOM_SETTING_V1] Done\n");

	  pmic_detect_powerkey();
	  printf("chr detection : %d \n",upmu_is_chr_det());

	  //PMIC_DUMP_ALL_Register();

      #ifdef MTK_MT6333_SUPPORT
      mt6333_init();
      #endif

	  return ret_code;
}

//==============================================================================
// PMIC6323 API for LK : AUXADC
//==============================================================================
int PMIC_IMM_GetOneChannelValue(int dwChannel, int deCount, int trimd)
{
	kal_int32 ret_data;	
	kal_int32 u4Sample_times = 0;
	kal_int32 u4channel=0;	
	kal_int32 adc_result_temp=0;
       kal_int32 r_val_temp=0;   
	kal_int32 adc_result=0;   
    kal_int32 ret=0;
    kal_uint32 adc_reg_val=0;
	
    /*
        0 : BATON2
        1 : CH6
        2 : THR SENSE2
        3 : THR SENSE1
        4 : VCDT
        5 : BATON1
        6 : ISENSE
        7 : BATSNS
        8 : ACCDET    
    */
    upmu_set_rg_vbuf_en(1);

    //set 0
    ret=pmic_read_interface(AUXADC_CON22,&adc_reg_val,PMIC_RG_AP_RQST_LIST_MASK,PMIC_RG_AP_RQST_LIST_SHIFT);
    adc_reg_val = adc_reg_val & (~(1<<dwChannel));
    ret=pmic_config_interface(AUXADC_CON22,adc_reg_val,PMIC_RG_AP_RQST_LIST_MASK,PMIC_RG_AP_RQST_LIST_SHIFT);

    //set 1
    ret=pmic_read_interface(AUXADC_CON22,&adc_reg_val,PMIC_RG_AP_RQST_LIST_MASK,PMIC_RG_AP_RQST_LIST_SHIFT);
    adc_reg_val = adc_reg_val | (1<<dwChannel);
    ret=pmic_config_interface(AUXADC_CON22,adc_reg_val,PMIC_RG_AP_RQST_LIST_MASK,PMIC_RG_AP_RQST_LIST_SHIFT);

    if(Enable_PMIC_LOG>1) 
      printf("%d", ret);

	do
	{
	    ret_data=0;

	    switch(dwChannel){         
	        case 0:    
	            while( upmu_get_rg_adc_rdy_baton2() != 1 );
	            ret_data = upmu_get_rg_adc_out_baton2();				
	            break;
	        case 1:    
	            while( upmu_get_rg_adc_rdy_ch6() != 1 );
	            ret_data = upmu_get_rg_adc_out_ch6();				
	            break;
	        case 2:    
	            while( upmu_get_rg_adc_rdy_thr_sense2() != 1 );
	            ret_data = upmu_get_rg_adc_out_thr_sense2();				
	            break;				
	        case 3:    
	            while( upmu_get_rg_adc_rdy_thr_sense1() != 1 );
	            ret_data = upmu_get_rg_adc_out_thr_sense1();				
	            break;
	        case 4:    
	            while( upmu_get_rg_adc_rdy_vcdt() != 1 );
	            ret_data = upmu_get_rg_adc_out_vcdt();				
	            break;
	        case 5:    
	            while( upmu_get_rg_adc_rdy_baton1() != 1 );
	            ret_data = upmu_get_rg_adc_out_baton1();				
	            break;
	        case 6:    
	            while( upmu_get_rg_adc_rdy_isense() != 1 );
	            ret_data = upmu_get_rg_adc_out_isense();				
	            break;
	        case 7:    
	            while( upmu_get_rg_adc_rdy_batsns() != 1 );
	            ret_data = upmu_get_rg_adc_out_batsns();				
	            break; 
	        case 8:    
	            while( upmu_get_rg_adc_rdy_ch5() != 1 );
	            ret_data = upmu_get_rg_adc_out_ch5();				
	            break; 				
                
	        default:
	            printf("[AUXADC] Invalid channel value(%d,%d)\n", dwChannel, trimd);
	           
	            return -1;
	            break;
	    }

	    u4channel += ret_data;

	    u4Sample_times++;

	   // if (Enable_BATDRV_LOG == 1)
	    {
	        //debug
	        printf("[AUXADC] u4channel[%d]=%d.\n", 
	            dwChannel, ret_data);
	    }
	    
	}while (u4Sample_times < deCount);

    /* Value averaging  */ 
    adc_result_temp = u4channel / deCount;

    switch(dwChannel){         
        case 0:                
            r_val_temp = 1;           
            adc_result = (adc_result_temp*r_val_temp*VOLTAGE_FULL_RANGE)/ADC_PRECISE;
            break;
        case 1:    
            r_val_temp = 1;
            adc_result = (adc_result_temp*r_val_temp*VOLTAGE_FULL_RANGE)/ADC_PRECISE;
            break;
        case 2:    
            r_val_temp = 1;
            adc_result = (adc_result_temp*r_val_temp*VOLTAGE_FULL_RANGE)/ADC_PRECISE;
            break;
        case 3:    
            r_val_temp = 1;
            adc_result = (adc_result_temp*r_val_temp*VOLTAGE_FULL_RANGE)/ADC_PRECISE;
            break;
        case 4:    
            r_val_temp = 1;
            adc_result = (adc_result_temp*r_val_temp*VOLTAGE_FULL_RANGE)/ADC_PRECISE;
            break;
        case 5:    
            r_val_temp = 1;
            adc_result = (adc_result_temp*r_val_temp*VOLTAGE_FULL_RANGE)/ADC_PRECISE;
            break;
        case 6:    
            r_val_temp = 4;
            adc_result = (adc_result_temp*r_val_temp*VOLTAGE_FULL_RANGE)/ADC_PRECISE;
            break;
        case 7:    
            r_val_temp = 4;
            adc_result = (adc_result_temp*r_val_temp*VOLTAGE_FULL_RANGE)/ADC_PRECISE;
            break;    
        case 8:    
            r_val_temp = 1;
            adc_result = (adc_result_temp*r_val_temp*VOLTAGE_FULL_RANGE)/ADC_PRECISE;
            break;    			
        default:
            printf("[AUXADC] Invalid channel value(%d,%d)\n", dwChannel, trimd);

            return -1;
            break;
    }

        printf("[AUXADC] adc_result_temp=%d, adc_result=%d, r_val_temp=%d.\n", 
                adc_result_temp, adc_result, r_val_temp);

	
    return adc_result;
}

int get_bat_sense_volt(int times)
{
    return PMIC_IMM_GetOneChannelValue(VBAT_CHANNEL_NUMBER,times,1);
}

int get_i_sense_volt(int times)
{
    return PMIC_IMM_GetOneChannelValue(ISENSE_CHANNEL_NUMBER,times,1);
}

int get_charger_volt(int times)
{
    return PMIC_IMM_GetOneChannelValue(VCHARGER_CHANNEL_NUMBER,times,1);
}

int get_tbat_volt(int times)
{
    return PMIC_IMM_GetOneChannelValue(VBATTEMP_CHANNEL_NUMBER,times,1);
}

#if defined(CONFIG_POWER_EXT)
#else

kal_uint32 Enable_Detection_Log = 0;

static void hw_bc11_dump_register(void)
{
	kal_uint32 reg_val = 0;
	kal_uint32 reg_num = CHR_CON18;
	kal_uint32 i = 0;

	for(i=reg_num ; i<=CHR_CON19 ; i+=2)
	{
		reg_val = upmu_get_reg_value(i);
		printf("Chr Reg[0x%x]=0x%x \r\n", i, reg_val);
	}	
}

extern void Charger_Detect_Init(void);
extern void Charger_Detect_Release(void);

static void hw_bc11_init(void)
{
    mdelay(300);
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

    if(Enable_Detection_Log)
    {
        printf("hw_bc11_init() \r\n");
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

    if(Enable_Detection_Log)
    {
        printf("hw_bc11_DCD() \r\n");
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
      
    //RG_BC11_IPU_EN[1.0] = 10
    upmu_set_rg_bc11_ipu_en(0x2);
    //RG_BC11_VREF_VTH = [1:0]=10
    upmu_set_rg_bc11_vref_vth(0x2);
    //RG_BC11_CMP_EN[1.0] = 10
    upmu_set_rg_bc11_cmp_en(0x2);

    //msleep(80);
    mdelay(80);

    wChargerAvail = upmu_get_rgs_bc11_cmp_out();

    if(Enable_Detection_Log)
    {
    	printf("hw_bc11_stepA1() \r\n");
    	hw_bc11_dump_register();
    }

    //RG_BC11_IPU_EN[1.0] = 00
    upmu_set_rg_bc11_ipu_en(0x0);
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

    if(Enable_Detection_Log)
    {
    	printf("hw_bc11_stepB1() \r\n");
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

    if(Enable_Detection_Log)
    {
    	printf("hw_bc11_stepC1() \r\n");
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

    if(Enable_Detection_Log)
    {
    	printf("hw_bc11_stepA2() \r\n");
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

    if(Enable_Detection_Log)
    {
    	printf("hw_bc11_stepB2() \r\n");
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

    if(Enable_Detection_Log)
    {
    	printf("hw_bc11_done() \r\n");
    	hw_bc11_dump_register();
    }

}
#endif

kal_uint32 charging_get_charger_type(void *data)
 {
	 kal_uint32 status = 0;
#if defined(CONFIG_POWER_EXT)
	 *(CHARGER_TYPE*)(data) = STANDARD_HOST;
#else
    
#if 0
    CHARGER_TYPE charger_type = CHARGER_UNKNOWN;
    charger_type = hw_charger_type_detection();
    battery_xlog_printk(BAT_LOG_CRTI, "charging_get_charger_type = %d\r\n", charger_type);
        
    *(CHARGER_TYPE*)(data) = charger_type;
#endif

#if 1
	/********* Step initial  ***************/		 
	hw_bc11_init();
 
	/********* Step DCD ***************/  
	if(1 == hw_bc11_DCD())
	{
		 /********* Step A1 ***************/
		 if(1 == hw_bc11_stepA1())
		 {
			 /********* Step B1 ***************/
			 if(1 == hw_bc11_stepB1())
			 {
				 //*(CHARGER_TYPE*)(data) = NONSTANDARD_CHARGER;
				 //battery_xlog_printk(BAT_LOG_CRTI, "step B1 : Non STANDARD CHARGER!\r\n");				
				 *(CHARGER_TYPE*)(data) = APPLE_2_1A_CHARGER;
				 printf("step B1 : Apple 2.1A CHARGER!\r\n");
			 }	 
			 else
			 {
				 //*(CHARGER_TYPE*)(data) = APPLE_2_1A_CHARGER;
				 //battery_xlog_printk(BAT_LOG_CRTI, "step B1 : Apple 2.1A CHARGER!\r\n");
				 *(CHARGER_TYPE*)(data) = NONSTANDARD_CHARGER;
				 printf("step B1 : Non STANDARD CHARGER!\r\n");
			 }	 
		 }
		 else
		 {
			 /********* Step C1 ***************/
			 if(1 == hw_bc11_stepC1())
			 {
				 *(CHARGER_TYPE*)(data) = APPLE_1_0A_CHARGER;
				 printf("step C1 : Apple 1A CHARGER!\r\n");
			 }	 
			 else
			 {
				 *(CHARGER_TYPE*)(data) = APPLE_0_5A_CHARGER;
				 printf("step C1 : Apple 0.5A CHARGER!\r\n");			 
			 }	 
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
				 printf("step B2 : STANDARD CHARGER!\r\n");
			 }
			 else
			 {
				 *(CHARGER_TYPE*)(data) = CHARGING_HOST;
				 printf("step B2 :  Charging Host!\r\n");
			 }
		 }
		 else
		 {
         *(CHARGER_TYPE*)(data) = STANDARD_HOST;
			 printf("step A2 : Standard USB Host!\r\n");
		 }
 
	}
 
	 /********* Finally setting *******************************/
	 hw_bc11_done();
#endif
#endif
	 return status;
}
