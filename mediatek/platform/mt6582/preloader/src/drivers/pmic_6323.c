#include <typedefs.h>
#include <platform.h>
#include <pmic_wrap_init.h>
#include <pmic.h>

//////////////////////////////////////////////////////////////////////////////////////////
// PMIC access API
//////////////////////////////////////////////////////////////////////////////////////////
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
        print("[pmic_read_interface] Reg[%x]= pmic_wrap read data fail\n", RegNum);
        return return_value;
    }
    //print("[pmic_read_interface] Reg[%x]=0x%x\n", RegNum, pmic6323_reg);
    
    pmic6323_reg &= (MASK << SHIFT);
    *val = (pmic6323_reg >> SHIFT);    
    //print("[pmic_read_interface] val=0x%x\n", *val);

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
        print("[pmic_config_interface] Reg[%x]= pmic_wrap read data fail\n", RegNum);
        return return_value;
    }
    //print("[pmic_config_interface] Reg[%x]=0x%x\n", RegNum, pmic6323_reg);
    
    pmic6323_reg &= ~(MASK << SHIFT);
    pmic6323_reg |= (val << SHIFT);

    //2. mt6323_write_byte(RegNum, pmic6323_reg);
    return_value= pwrap_wacs2(1, (RegNum), pmic6323_reg, &rdata);
    if(return_value!=0)
    {   
        print("[pmic_config_interface] Reg[%x]= pmic_wrap read data fail\n", RegNum);
        return return_value;
    }
    //print("[pmic_config_interface] write Reg[%x]=0x%x\n", RegNum, pmic6323_reg);    

#if 0
    //3. Double Check    
    //mt6323_read_byte(RegNum, &pmic6323_reg);
    return_value= pwrap_wacs2(0, (RegNum), 0, &rdata);
    pmic6323_reg=rdata;    
    if(return_value!=0)
    {   
        print("[pmic_config_interface] Reg[%x]= pmic_wrap write data fail\n", RegNum);
        return return_value;
    }
    print("[pmic_config_interface] Reg[%x]=0x%x\n", RegNum, pmic6323_reg);
#endif    

    return return_value;
}

//////////////////////////////////////////////////////////////////////////////////////////
// PMIC-Charger Type Detection
//////////////////////////////////////////////////////////////////////////////////////////
CHARGER_TYPE g_ret = CHARGER_UNKNOWN;
int g_charger_in_flag = 0;
int g_first_check=0;

extern void Charger_Detect_Init(void);
extern void Charger_Detect_Release(void);

void pmic_lock(void){    
}

void pmic_unlock(void){    
}


void upmu_set_rg_bc11_bb_ctrl(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(CHR_CON18),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_BC11_BB_CTRL_MASK),
                             (kal_uint32)(PMIC_RG_BC11_BB_CTRL_SHIFT)
	                         );
  pmic_unlock();
}

void upmu_set_rg_bc11_rst(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(CHR_CON18),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_BC11_RST_MASK),
                             (kal_uint32)(PMIC_RG_BC11_RST_SHIFT)
	                         );
  pmic_unlock();
}

void upmu_set_rg_bc11_vsrc_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(CHR_CON18),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_BC11_VSRC_EN_MASK),
                             (kal_uint32)(PMIC_RG_BC11_VSRC_EN_SHIFT)
	                         );
  pmic_unlock();
}

kal_uint32 upmu_get_rgs_bc11_cmp_out(void)
{
  kal_uint32 ret=0;
  kal_uint32 val=0;

  pmic_lock();
  ret=pmic_read_interface( (kal_uint32)(CHR_CON18),
                           (&val),
                           (kal_uint32)(PMIC_RGS_BC11_CMP_OUT_MASK),
                           (kal_uint32)(PMIC_RGS_BC11_CMP_OUT_SHIFT)
	                       );
  pmic_unlock();

  return val;
}

void upmu_set_rg_bc11_vref_vth(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(CHR_CON19),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_BC11_VREF_VTH_MASK),
                             (kal_uint32)(PMIC_RG_BC11_VREF_VTH_SHIFT)
	                         );
  pmic_unlock();
}

void upmu_set_rg_bc11_cmp_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(CHR_CON19),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_BC11_CMP_EN_MASK),
                             (kal_uint32)(PMIC_RG_BC11_CMP_EN_SHIFT)
	                         );
  pmic_unlock();
}

void upmu_set_rg_bc11_ipd_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(CHR_CON19),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_BC11_IPD_EN_MASK),
                             (kal_uint32)(PMIC_RG_BC11_IPD_EN_SHIFT)
	                         );
  pmic_unlock();
}

void upmu_set_rg_bc11_ipu_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(CHR_CON19),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_BC11_IPU_EN_MASK),
                             (kal_uint32)(PMIC_RG_BC11_IPU_EN_SHIFT)
	                         );
  pmic_unlock();
}

void upmu_set_rg_bc11_bias_en(kal_uint32 val)
{
  kal_uint32 ret=0;

  pmic_lock();
  ret=pmic_config_interface( (kal_uint32)(CHR_CON19),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_BC11_BIAS_EN_MASK),
                             (kal_uint32)(PMIC_RG_BC11_BIAS_EN_SHIFT)
	                         );
  pmic_unlock();
}


void hw_bc11_init(void)
 {
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
 
	 mdelay(100);
	 
 }
 
 
 U32 hw_bc11_DCD(void)
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
 
	 mdelay(400);

 	 wChargerAvail = upmu_get_rgs_bc11_cmp_out();
	 
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
 
 
 U32 hw_bc11_stepA1(void)
 {
	 U32 wChargerAvail = 0;
	  
	  //RG_BC11_IPU_EN[1.0] = 10
	 upmu_set_rg_bc11_ipu_en(0x2);
	   //RG_BC11_VREF_VTH = [1:0]=10
	 upmu_set_rg_bc11_vref_vth(0x2);
	  //RG_BC11_CMP_EN[1.0] = 10
	 upmu_set_rg_bc11_cmp_en(0x2);
 
	 mdelay(80);
 
     wChargerAvail = upmu_get_rgs_bc11_cmp_out();
 
	 //RG_BC11_IPU_EN[1.0] = 00
	 upmu_set_rg_bc11_ipu_en(0x0);
	  //RG_BC11_CMP_EN[1.0] = 00
	 upmu_set_rg_bc11_cmp_en(0x0);
 
	 return  wChargerAvail;
 }
 
 
 U32 hw_bc11_stepB1(void)
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
 
	 mdelay(80);
 
     wChargerAvail = upmu_get_rgs_bc11_cmp_out();
 
	 //RG_BC11_IPU_EN[1.0] = 00
	 upmu_set_rg_bc11_ipu_en(0x0);
	  //RG_BC11_CMP_EN[1.0] = 00
	 upmu_set_rg_bc11_cmp_en(0x0);
	   //RG_BC11_VREF_VTH = [1:0]=00
	 upmu_set_rg_bc11_vref_vth(0x0);
 
	 return  wChargerAvail;
 }
 
 
 U32 hw_bc11_stepC1(void)
 {
	 U32 wChargerAvail = 0;
	  
	  //RG_BC11_IPU_EN[1.0] = 01
	 upmu_set_rg_bc11_ipu_en(0x1);
	   //RG_BC11_VREF_VTH = [1:0]=10
	 upmu_set_rg_bc11_vref_vth(0x2);
	  //RG_BC11_CMP_EN[1.0] = 01
	 upmu_set_rg_bc11_cmp_en(0x1);
 
	 mdelay(80);
 
     wChargerAvail = upmu_get_rgs_bc11_cmp_out();
 
	 //RG_BC11_IPU_EN[1.0] = 00
	 upmu_set_rg_bc11_ipu_en(0x0);
	  //RG_BC11_CMP_EN[1.0] = 00
	 upmu_set_rg_bc11_cmp_en(0x0);
	   //RG_BC11_VREF_VTH = [1:0]=00
     upmu_set_rg_bc11_vref_vth(0x0);
 
	 return  wChargerAvail;
 }
 
 
 U32 hw_bc11_stepA2(void)
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
 
	 mdelay(80);

     wChargerAvail = upmu_get_rgs_bc11_cmp_out();
 
	 //RG_BC11_VSRC_EN[1:0]=00
	 upmu_set_rg_bc11_vsrc_en(0x0);
	 //RG_BC11_IPD_EN[1.0] = 00
	 upmu_set_rg_bc11_ipd_en(0x0);
	 //RG_BC11_CMP_EN[1.0] = 00
	 upmu_set_rg_bc11_cmp_en(0x0);
 
	 return  wChargerAvail;
 }
 
 
 U32 hw_bc11_stepB2(void)
 {
	 U32 wChargerAvail = 0;
 
	//RG_BC11_IPU_EN[1:0]=10
	upmu_set_rg_bc11_ipu_en(0x2);
	//RG_BC11_VREF_VTH = [1:0]=01
	upmu_set_rg_bc11_vref_vth(0x1);
	//RG_BC11_CMP_EN[1.0] = 01
	upmu_set_rg_bc11_cmp_en(0x1);
 
	mdelay(80);
 
    wChargerAvail = upmu_get_rgs_bc11_cmp_out();
 
	//RG_BC11_IPU_EN[1.0] = 00
	upmu_set_rg_bc11_ipu_en(0x0);
	//RG_BC11_CMP_EN[1.0] = 00
	upmu_set_rg_bc11_cmp_en(0x0);
	//RG_BC11_VREF_VTH = [1:0]=00
	upmu_set_rg_bc11_vref_vth(0x0);
 
	 return  wChargerAvail;
 }
 
 
 void hw_bc11_done(void)
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
 }

CHARGER_TYPE hw_charger_type_detection(void)
{
    CHARGER_TYPE charger_tye;
#if defined(CONFIG_POWER_EXT)
	 charger_tye = STANDARD_HOST;
#else

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
				  //charger_tye = NONSTANDARD_CHARGER;
				  //printf("step B1 : Non STANDARD CHARGER!\r\n");
				  charger_tye = APPLE_2_1A_CHARGER;
				  printf("step B1 : Apple 2.1A CHARGER!\r\n");
			 }	 
			 else
			 {
				  //charger_tye = APPLE_2_1A_CHARGER;
				  //printf("step B1 : Apple 2.1A CHARGER!\r\n");
				  charger_tye = NONSTANDARD_CHARGER;
				  printf("step B1 : Non STANDARD CHARGER!\r\n");
			 }	 
		 }
		 else
		 {
			 /********* Step C1 ***************/
			 if(1 == hw_bc11_stepC1())
			 {
				 charger_tye = APPLE_1_0A_CHARGER;
				  printf("step C1 : Apple 1A CHARGER!\r\n");
			 }	 
			 else
			 {
				charger_tye = APPLE_0_5A_CHARGER;
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
				 charger_tye = STANDARD_CHARGER;
				  printf("step B2 : STANDARD CHARGER!\r\n");
			 }
			 else
			 {
				 charger_tye = CHARGING_HOST;
				  printf("step B2 :  Charging Host!\r\n");
			 }
		 }
		 else
		 {
			 charger_tye = STANDARD_HOST;
			  printf("step A2 : Standard USB Host!\r\n");
		 }
 
	}
 
	 /********* Finally setting *******************************/
	 hw_bc11_done();
#endif
	 return charger_tye;
}

CHARGER_TYPE mt_charger_type_detection(void)
{
    if( g_first_check == 0 )
    {
        g_first_check = 1;
        g_ret = hw_charger_type_detection();
    }
    else
    {
        printf("[mt_charger_type_detection] Got data !!, %d, %d\r\n", g_charger_in_flag, g_first_check);
    }

    return g_ret;
}

//==============================================================================
// PMIC6323 Usage APIs
//==============================================================================
U32 get_pmic6323_chip_version (void)
{
    U32 ret=0;
    U32 eco_version = 0;
    
    ret=pmic_read_interface( (U32)(CID),
                             (&eco_version),
                             (U32)(PMIC_CID_MASK),
                             (U32)(PMIC_CID_SHIFT)
                             );

    return eco_version;
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
    if (val==1){     
        printf("pl pmic powerkey Release\n");
        return 0;
    }else{
        printf("pl pmic powerkey Press\n");
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
    if (val==1){     
        printf("pl pmic FCHRKEY Release\n");
        return 0;
    }else{
        printf("pl pmic FCHRKEY Press\n");
        return 1;
    }
}

U32 pmic_IsUsbCableIn (void) 
{    
    U32 ret=0;
    U32 val=0;
    
    ret=pmic_read_interface( (U32)(CHR_CON0),
                             (&val),
                             (U32)(PMIC_RGS_CHRDET_MASK),
                             (U32)(PMIC_RGS_CHRDET_SHIFT)
                             );


    if(val)
        return PMIC_CHRDET_EXIST;
    else
        return PMIC_CHRDET_NOT_EXIST;
}    

static int vbat_status = PMIC_VBAT_NOT_DROP;
static void pmic_DetectVbatDrop (void) 
{    
    U32 ret=0;
    U32 val1=0, val2=0;
    
    ret=pmic_read_interface( (U32)(INT_MISC_CON),
                             (&val1),
                             (U32)(PMIC_IVGEN_EXT_EN_MASK),
                             (U32)(PMIC_IVGEN_EXT_EN_SHIFT)
                             );
                             
    ret=pmic_read_interface( (U32)(TOP_RST_MISC),
                             (&val2),
                             (U32)(PMIC_RG_NEWLDO_RSTB_EN_MASK),
                             (U32)(PMIC_RG_NEWLDO_RSTB_EN_SHIFT)
                             );

    ret=pmic_config_interface( (U32)(INT_MISC_CON),
                             0x01,
                             (U32)(PMIC_IVGEN_EXT_EN_MASK),
                             (U32)(PMIC_IVGEN_EXT_EN_SHIFT)
                             );
                      
    printf("INT_MISC_CON: %d  TOP_RST_MISC: %d\r\n", val1, val2);
    if(val1 && !val2)
        vbat_status = PMIC_VBAT_DROP;
    else
        vbat_status = PMIC_VBAT_NOT_DROP;
}

int pmic_IsVbatDrop(void)
{
   return vbat_status;	
}

void hw_set_cc(int cc_val)
{    
	U32 ret_val=0;
    U32 reg_val=0;    
    U32 i=0;
    U32 hw_charger_ov_flag=0;

    printf("hw_set_cc: %d\r\n", cc_val);
    
    //VCDT_HV_VTH, 7V
    ret_val=pmic_config_interface(CHR_CON1, 0x0B, PMIC_RG_VCDT_HV_VTH_MASK, PMIC_RG_VCDT_HV_VTH_SHIFT); 
    //VCDT_HV_EN=1
    ret_val=pmic_config_interface(CHR_CON0, 0x01, PMIC_RG_VCDT_HV_EN_MASK, PMIC_RG_VCDT_HV_EN_SHIFT); 
    //CS_EN=1
    ret_val=pmic_config_interface(CHR_CON2, 0x01, PMIC_RG_CS_EN_MASK, PMIC_RG_CS_EN_SHIFT);
    //CSDAC_MODE=1
    ret_val=pmic_config_interface(CHR_CON23, 0x01, PMIC_RG_CSDAC_MODE_MASK, PMIC_RG_CSDAC_MODE_SHIFT);

    ret_val=pmic_read_interface(CHR_CON0, &hw_charger_ov_flag, PMIC_RGS_VCDT_HV_DET_MASK, PMIC_RGS_VCDT_HV_DET_SHIFT);
    if(hw_charger_ov_flag == 1)
    {
        ret_val=pmic_config_interface(CHR_CON0, 0x00, PMIC_RG_CHR_EN_MASK, PMIC_RG_CHR_EN_SHIFT);
        printf("pl chargerov turn off charging \n"); 
        return;
    }

    // CS_VTH
    switch(cc_val){
        case 1600: ret_val=pmic_config_interface(CHR_CON4, 0x00, PMIC_RG_CS_VTH_MASK, PMIC_RG_CS_VTH_SHIFT); break;
        case 1500: ret_val=pmic_config_interface(CHR_CON4, 0x01, PMIC_RG_CS_VTH_MASK, PMIC_RG_CS_VTH_SHIFT); break;       
        case 1400: ret_val=pmic_config_interface(CHR_CON4, 0x02, PMIC_RG_CS_VTH_MASK, PMIC_RG_CS_VTH_SHIFT); break;
        case 1300: ret_val=pmic_config_interface(CHR_CON4, 0x03, PMIC_RG_CS_VTH_MASK, PMIC_RG_CS_VTH_SHIFT); break;
        case 1200: ret_val=pmic_config_interface(CHR_CON4, 0x04, PMIC_RG_CS_VTH_MASK, PMIC_RG_CS_VTH_SHIFT); break;
        case 1100: ret_val=pmic_config_interface(CHR_CON4, 0x05, PMIC_RG_CS_VTH_MASK, PMIC_RG_CS_VTH_SHIFT); break;
        case 1000: ret_val=pmic_config_interface(CHR_CON4, 0x06, PMIC_RG_CS_VTH_MASK, PMIC_RG_CS_VTH_SHIFT); break;
        case 900:  ret_val=pmic_config_interface(CHR_CON4, 0x07, PMIC_RG_CS_VTH_MASK, PMIC_RG_CS_VTH_SHIFT); break;            
        case 800:  ret_val=pmic_config_interface(CHR_CON4, 0x08, PMIC_RG_CS_VTH_MASK, PMIC_RG_CS_VTH_SHIFT); break;
        case 700:  ret_val=pmic_config_interface(CHR_CON4, 0x09, PMIC_RG_CS_VTH_MASK, PMIC_RG_CS_VTH_SHIFT); break;       
        case 650:  ret_val=pmic_config_interface(CHR_CON4, 0x0A, PMIC_RG_CS_VTH_MASK, PMIC_RG_CS_VTH_SHIFT); break;
        case 550:  ret_val=pmic_config_interface(CHR_CON4, 0x0B, PMIC_RG_CS_VTH_MASK, PMIC_RG_CS_VTH_SHIFT); break;
        case 450:  ret_val=pmic_config_interface(CHR_CON4, 0x0C, PMIC_RG_CS_VTH_MASK, PMIC_RG_CS_VTH_SHIFT); break;
        case 300:  ret_val=pmic_config_interface(CHR_CON4, 0x0D, PMIC_RG_CS_VTH_MASK, PMIC_RG_CS_VTH_SHIFT); break;
        case 200:  ret_val=pmic_config_interface(CHR_CON4, 0x0E, PMIC_RG_CS_VTH_MASK, PMIC_RG_CS_VTH_SHIFT); break;
        case 70:   ret_val=pmic_config_interface(CHR_CON4, 0x0F, PMIC_RG_CS_VTH_MASK, PMIC_RG_CS_VTH_SHIFT); break;            
        default:
            dbg_print("hw_set_cc: argument invalid!!\r\n");
            break;
    }

    //upmu_chr_csdac_dly(0x4);                // CSDAC_DLY
    ret_val=pmic_config_interface(CHR_CON21, 0x04, PMIC_RG_CSDAC_DLY_MASK, PMIC_RG_CSDAC_DLY_SHIFT);
    //upmu_chr_csdac_stp(0x1);                // CSDAC_STP
    ret_val=pmic_config_interface(CHR_CON21, 0x01, PMIC_RG_CSDAC_STP_MASK, PMIC_RG_CSDAC_STP_SHIFT);
    //upmu_chr_csdac_stp_inc(0x1);            // CSDAC_STP_INC
    ret_val=pmic_config_interface(CHR_CON20, 0x01, PMIC_RG_CSDAC_STP_INC_MASK, PMIC_RG_CSDAC_STP_INC_SHIFT);
    //upmu_chr_csdac_stp_dec(0x2);            // CSDAC_STP_DEC
    ret_val=pmic_config_interface(CHR_CON20, 0x02, PMIC_RG_CSDAC_STP_DEC_MASK, PMIC_RG_CSDAC_STP_DEC_SHIFT);
    //upmu_chr_chrwdt_td(0x0);                // CHRWDT_TD, 4s
    ret_val=pmic_config_interface(CHR_CON13, 0x00, PMIC_RG_CHRWDT_TD_MASK, PMIC_RG_CHRWDT_TD_SHIFT);
    //upmu_set_rg_chrwdt_wr(1);             // CHRWDT_FLAG_WR
    ret_val=pmic_config_interface(CHR_CON13, 0x01, PMIC_RG_CHRWDT_WR_MASK, PMIC_RG_CHRWDT_WR_SHIFT);
    //upmu_chr_chrwdt_int_en(1);              // CHRWDT_INT_EN
    ret_val=pmic_config_interface(CHR_CON15, 0x01, PMIC_RG_CHRWDT_INT_EN_MASK, PMIC_RG_CHRWDT_INT_EN_SHIFT);
    //upmu_chr_chrwdt_en(1);                  // CHRWDT_EN
    ret_val=pmic_config_interface(CHR_CON13, 0x01, PMIC_RG_CHRWDT_EN_MASK, PMIC_RG_CHRWDT_EN_SHIFT);
    //upmu_chr_chrwdt_flag_wr(1);             // CHRWDT_FLAG_WR
    ret_val=pmic_config_interface(CHR_CON15, 0x01, PMIC_RG_CHRWDT_FLAG_WR_MASK, PMIC_RG_CHRWDT_FLAG_WR_SHIFT);
    //upmu_chr_csdac_enable(1);               // CSDAC_EN
    ret_val=pmic_config_interface(CHR_CON0, 0x01, PMIC_RG_CSDAC_EN_MASK, PMIC_RG_CSDAC_EN_SHIFT);
    //upmu_set_rg_hwcv_en(1);                 // HWCV_EN
    ret_val=pmic_config_interface(CHR_CON23, 0x01, PMIC_RG_HWCV_EN_MASK, PMIC_RG_HWCV_EN_SHIFT);
    //upmu_chr_enable(1);                     // CHR_EN
    ret_val=pmic_config_interface(CHR_CON0, 0x01, PMIC_RG_CHR_EN_MASK, PMIC_RG_CHR_EN_SHIFT);

    for(i=CHR_CON0 ; i<=CHR_CON29 ; i++)    
    {        
        ret_val=pmic_read_interface(i,&reg_val,0xFFFF,0x0);        
        print("[0x%x]=0x%x\n", i, reg_val);    
    }

    printf("hw_set_cc: done\r\n");    
}

void pl_hw_ulc_det(void)
{
	U32 ret_val=0;
    
    //upmu_chr_ulc_det_en(1);            // RG_ULC_DET_EN=1
    ret_val=pmic_config_interface(CHR_CON23, 0x01, PMIC_RG_ULC_DET_EN_MASK, PMIC_RG_ULC_DET_EN_SHIFT);
    //upmu_chr_low_ich_db(1);            // RG_LOW_ICH_DB=000001'b
    ret_val=pmic_config_interface(CHR_CON22, 0x01, PMIC_RG_LOW_ICH_DB_MASK, PMIC_RG_LOW_ICH_DB_SHIFT);
}

int hw_check_battery(void)
{
#ifndef MTK_DISABLE_POWER_ON_OFF_VOLTAGE_LIMITATION
	U32 ret_val;
    U32 reg_val=0;

 	ret_val=pmic_config_interface(CHR_CON7,0x01, PMIC_RG_BATON_EN_MASK, PMIC_RG_BATON_EN_SHIFT);      //BATON_EN=1
 	ret_val=pmic_config_interface(CHR_CON7,    0x01, PMIC_BATON_TDET_EN_MASK, PMIC_BATON_TDET_EN_SHIFT);  //BATON_TDET_EN=1
	ret_val=pmic_read_interface(CHR_CON7,&reg_val, PMIC_RGS_BATON_UNDET_MASK, PMIC_RGS_BATON_UNDET_SHIFT);

    if (reg_val == 1)
    {                     
        printf("No Battery\n");

        ret_val=pmic_read_interface(CHR_CON7,&reg_val,0xFFFF,0x0);
		print("[0x%x]=0x%x\n",CHR_CON7,reg_val);
           
        return 0;        
    }
    else
    {
        printf("Battery exist\n");

        ret_val=pmic_read_interface(CHR_CON7,&reg_val,0xFF,0x0);
		print("[0x%x]=0x%x\n",CHR_CON7,reg_val);
           
        pl_hw_ulc_det();
        
    return 1;  
    }
#else
	return 1;
#endif
}

void pl_charging(int en_chr)
{
	U32 ret_val=0;
    U32 reg_val=0;
    U32 i=0;
    
    if(en_chr == 1)
    {
        printf("pl charging en\n");
    
        hw_set_cc(450);

        //USBDL set 1
        ret_val=pmic_config_interface(CHR_CON16, 0x01, PMIC_RG_USBDL_SET_MASK, PMIC_RG_USBDL_SET_SHIFT);        
    }
    else
    {
        printf("pl charging dis\n");
    
        //USBDL set 0
        ret_val=pmic_config_interface(CHR_CON16, 0x00, PMIC_RG_USBDL_SET_MASK, PMIC_RG_USBDL_SET_SHIFT);

        //upmu_set_rg_hwcv_en(0); // HWCV_EN
        ret_val=pmic_config_interface(CHR_CON23, 0x00, PMIC_RG_HWCV_EN_MASK, PMIC_RG_HWCV_EN_SHIFT);
        //upmu_chr_enable(0); // CHR_EN
        ret_val=pmic_config_interface(CHR_CON0, 0x00, PMIC_RG_CHR_EN_MASK, PMIC_RG_CHR_EN_SHIFT);        
    }

    for(i=CHR_CON0 ; i<=CHR_CON29 ; i++)    
    {        
        ret_val=pmic_read_interface(i,&reg_val,0xFFFF,0x0);        
        print("[0x%x]=0x%x\n", i, reg_val);    
    }

    printf("pl charging done\n");
}

void pl_kick_chr_wdt(void)
{
	 int ret_val=0;

    //upmu_chr_chrwdt_td(0x0);                // CHRWDT_TD
    ret_val=pmic_config_interface(CHR_CON13, 0x03, PMIC_RG_CHRWDT_TD_MASK, PMIC_RG_CHRWDT_TD_SHIFT);
    //upmu_set_rg_chrwdt_wr(1);            // CHRWDT_FLAG
    ret_val=pmic_config_interface(CHR_CON15, 0x01, PMIC_RG_CHRWDT_WR_MASK, PMIC_RG_CHRWDT_WR_SHIFT);
    //upmu_chr_chrwdt_int_en(1);             // CHRWDT_INT_EN
    ret_val=pmic_config_interface(CHR_CON15, 0x01, PMIC_RG_CHRWDT_INT_EN_MASK, PMIC_RG_CHRWDT_INT_EN_SHIFT);
    //upmu_chr_chrwdt_en(1);                   // CHRWDT_EN
    ret_val=pmic_config_interface(CHR_CON13, 0x01, PMIC_RG_CHRWDT_EN_MASK, PMIC_RG_CHRWDT_EN_SHIFT);
    //upmu_chr_chrwdt_flag_wr(1);            // CHRWDT_FLAG_WR
    ret_val=pmic_config_interface(CHR_CON15, 0x01, PMIC_RG_CHRWDT_FLAG_WR_MASK, PMIC_RG_CHRWDT_FLAG_WR_SHIFT);

    //printf("[pl_kick_chr_wdt] done\n");
}

void pl_close_pre_chr_led(void)
{
	U32 ret_val=0;    

    ret_val=pmic_config_interface(CHR_CON22, 0x00, PMIC_RG_CHRIND_ON_MASK, PMIC_RG_CHRIND_ON_SHIFT);
    
    printf("pl pmic close pre-chr LED\n");
}

void usbdl_wo_battery_forced(int en_chr)
{
		U32 ret_val=0;
		
    if(en_chr == 1)
    {
        printf("chr force en\n");

        //USBDL set 1
        ret_val=pmic_config_interface(CHR_CON16, 0x01, PMIC_RG_USBDL_SET_MASK, PMIC_RG_USBDL_SET_SHIFT);
        // CHRWDT_TD, 4s
        ret_val=pmic_config_interface(CHR_CON13, 0x00, PMIC_RG_CHRWDT_TD_MASK, PMIC_RG_CHRWDT_TD_SHIFT);      
		// CHRWDT_FLAG_WR        
		ret_val=pmic_config_interface(CHR_CON13, 0x01, PMIC_RG_CHRWDT_WR_MASK, PMIC_RG_CHRWDT_WR_SHIFT);
        // CHRWDT_EN
        ret_val=pmic_config_interface(CHR_CON13, 0x01, PMIC_RG_CHRWDT_EN_MASK, PMIC_RG_CHRWDT_EN_SHIFT);
    }
    else
    {
        printf("chr force dis\n");
    
        //USBDL set 0
        ret_val=pmic_config_interface(CHR_CON16, 0x00, PMIC_RG_USBDL_SET_MASK, PMIC_RG_USBDL_SET_SHIFT);
        ret_val=pmic_config_interface(CHR_CON16, 0x01, PMIC_RG_USBDL_RST_MASK, PMIC_RG_USBDL_RST_SHIFT);
		// CHRWDT_FLAG_WR        
		ret_val=pmic_config_interface(CHR_CON13, 0x01, PMIC_RG_CHRWDT_WR_MASK, PMIC_RG_CHRWDT_WR_SHIFT);
        // CHRWDT_EN
        ret_val=pmic_config_interface(CHR_CON13, 0x00, PMIC_RG_CHRWDT_EN_MASK, PMIC_RG_CHRWDT_EN_SHIFT);     
    }	
}


//==============================================================================
// PMIC6323 Init Code
//==============================================================================
U32 pmic_init (void)
{
    U32 ret_code = PMIC_TEST_PASS;
    int ret_val=0;
    int reg_val=0;

    print("[pmic6323_init] Preloader Start..................\n");
    
    print("[pmic6323_init] PMIC CHIP Code = 0x%x\n", get_pmic6323_chip_version());

    //put pmic hw initial setting here

	//detect V battery Drop 
	pmic_DetectVbatDrop();
    //USBDL reset 1
    //ret_val=pmic_config_interface(CHR_CON16, 0x01, PMIC_RG_USBDL_RST_MASK, PMIC_RG_USBDL_RST_SHIFT);

    print("[pmic6323_init] powerKey = %d\n", pmic_detect_powerkey());    
    print("[pmic6323_init] is USB in = 0x%x\n", pmic_IsUsbCableIn());

    //Enable PMIC RST function (depends on main chip RST function)
    ret_val=pmic_config_interface(0x011A, 0x1, 0x1, 1);
    ret_val=pmic_config_interface(0x011A, 0x1, 0x1, 3);
    ret_val=pmic_read_interface( 0x011A, (&reg_val),0xFFFF, 0);
    print("[pmic6323_init] Reg[0x%x]=0x%x\n", 0x011A, reg_val);

    #ifdef DUMMY_AP
    ret_val=pmic_config_interface(0x30E, 0x1, 0x1, 0);
    ret_val=pmic_read_interface( 0x30E, (&reg_val),0xFFFF, 0);
    print("[pmic6323_init for DUMMY_AP] Reg[0x%x]=0x%x\n", 0x30E, reg_val);
    #endif

    #ifdef MTK_MT6333_SUPPORT
    mt6333_init();
    #endif

    print("[pmic6323_init] Done...................\n");

    return ret_code;
}

