#include <typedefs.h>
#include <platform.h>
#include <mt_pmic_wrap_init.h>
#include <mtk_pmic_6320.h>

//#define PMIC_DEBUG
#ifdef PMIC_DEBUG
#define PMIC_PRINT   print
#else
#define PMIC_PRINT
#endif

//////////////////////////////////////////////////////////////////////////////////////////
// PMIC access API
//////////////////////////////////////////////////////////////////////////////////////////
U32 pmic_read_interface (U32 RegNum, U32 *val, U32 MASK, U32 SHIFT)
{
    U32 return_value = 0;    
    U32 pmic6320_reg = 0;
    U32 rdata;    

    //mt6320_read_byte(RegNum, &pmic6320_reg);
    return_value= pwrap_wacs2(0, (RegNum), 0, &rdata);
    pmic6320_reg=rdata;
    if(return_value!=0)
    {   
        PMIC_PRINT("[pmic_read_interface] Reg[%x]= pmic_wrap read data fail\n", RegNum);
        return return_value;
    }
    PMIC_PRINT("[pmic_read_interface] Reg[%x]=0x%x\n", RegNum, pmic6320_reg);
    
    pmic6320_reg &= (MASK << SHIFT);
    *val = (pmic6320_reg >> SHIFT);    
    PMIC_PRINT("[pmic_read_interface] val=0x%x\n", *val);

    return return_value;
}

U32 pmic_config_interface (U32 RegNum, U32 val, U32 MASK, U32 SHIFT)
{
    U32 return_value = 0;    
    U32 pmic6320_reg = 0;
    U32 rdata;

    //1. mt6320_read_byte(RegNum, &pmic6320_reg);
    return_value= pwrap_wacs2(0, (RegNum), 0, &rdata);
    pmic6320_reg=rdata;    
    if(return_value!=0)
    {   
        PMIC_PRINT("[pmic_config_interface] Reg[%x]= pmic_wrap read data fail\n", RegNum);
        return return_value;
    }
    PMIC_PRINT("[pmic_config_interface] Reg[%x]=0x%x\n", RegNum, pmic6320_reg);
    
    pmic6320_reg &= ~(MASK << SHIFT);
    pmic6320_reg |= (val << SHIFT);

    //2. mt6320_write_byte(RegNum, pmic6320_reg);
    return_value= pwrap_wacs2(1, (RegNum), pmic6320_reg, &rdata);
    if(return_value!=0)
    {   
        PMIC_PRINT("[pmic_config_interface] Reg[%x]= pmic_wrap read data fail\n", RegNum);
        return return_value;
    }
    PMIC_PRINT("[pmic_config_interface] write Reg[%x]=0x%x\n", RegNum, pmic6320_reg);    

#if 0
    //3. Double Check    
    //mt6320_read_byte(RegNum, &pmic6320_reg);
    return_value= pwrap_wacs2(0, (RegNum), 0, &rdata);
    pmic6320_reg=rdata;    
    if(return_value!=0)
    {   
        print("[pmic_config_interface] Reg[%x]= pmic_wrap write data fail\n", RegNum);
        return return_value;
    }
    print("[pmic_config_interface] Reg[%x]=0x%x\n", RegNum, pmic6320_reg);
#endif    

    return return_value;
}

//////////////////////////////////////////////////////////////////////////////////////////
// PMIC-Charger Type Detection
//////////////////////////////////////////////////////////////////////////////////////////
CHARGER_TYPE g_ret = PMIC_CHARGER_UNKNOWN;
int g_charger_in_flag = 0;
int g_first_check=0;

CHARGER_TYPE hw_charger_type_detection(void)
{
    CHARGER_TYPE ret                 = PMIC_CHARGER_UNKNOWN;

#if 0    
    ret = PMIC_STANDARD_HOST;
#else
    unsigned int USB_U2PHYACR6_2     = 0x1122081A;
    unsigned int USBPHYRegs          = 0x11220800; //U2B20_Base+0x800
    U16 bLineState_B                 = 0;
    U32 wChargerAvail                = 0;
    U32 bLineState_C                 = 0;
    U32 ret_val                      = 0;
    U32 reg_val                      = 0;

    //msleep(400);
    //printf("mt_charger_type_detection : start!\r\n");

/********* Step 0.0 : enable USB memory and clock *********/
    //enable_pll(MT65XX_UPLL,"USB_PLL");
    //hwPowerOn(MT65XX_POWER_LDO_VUSB,VOL_DEFAULT,"VUSB_LDO");
    //printf("[hw_charger_type_detection] enable VUSB and UPLL before connect\n");

/********* Step 1.0 : PMU_BC11_Detect_Init ***************/        
    SETREG16(USB_U2PHYACR6_2,0x80); //bit 7 = 1 : switch to PMIC        
    
    //BC11_RST=1
    ret_val=pmic_config_interface(CHR_CON18,0x1,PMIC_RG_BC11_RST_MASK,PMIC_RG_BC11_RST_SHIFT); 
    //BC11_BB_CTRL=1
    ret_val=pmic_config_interface(CHR_CON18,0x1,PMIC_RG_BC11_BB_CTRL_MASK,PMIC_RG_BC11_BB_CTRL_SHIFT);
    
    //RG_BC11_BIAS_EN=1    
    ret_val=pmic_config_interface(CHR_CON19,0x1,PMIC_RG_BC11_BIAS_EN_MASK,PMIC_RG_BC11_BIAS_EN_SHIFT); 
    //RG_BC11_VSRC_EN[1:0]=00
    ret_val=pmic_config_interface(CHR_CON18,0x0,PMIC_RG_BC11_VSRC_EN_MASK,PMIC_RG_BC11_VSRC_EN_SHIFT); 
    //RG_BC11_VREF_VTH = 0
    ret_val=pmic_config_interface(CHR_CON19,0x0,PMIC_RG_BC11_VREF_VTH_MASK,PMIC_RG_BC11_VREF_VTH_SHIFT); 
    //RG_BC11_CMP_EN[1.0] = 00
    ret_val=pmic_config_interface(CHR_CON19,0x0,PMIC_RG_BC11_CMP_EN_MASK,PMIC_RG_BC11_CMP_EN_SHIFT);
    //RG_BC11_IPU_EN[1.0] = 00
    ret_val=pmic_config_interface(CHR_CON19,0x0,PMIC_RG_BC11_IPU_EN_MASK,PMIC_RG_BC11_IPU_EN_SHIFT);
    //RG_BC11_IPD_EN[1.0] = 00
    ret_val=pmic_config_interface(CHR_CON19,0x0,PMIC_RG_BC11_IPD_EN_MASK,PMIC_RG_BC11_IPD_EN_SHIFT);

    //ret_val=pmic_read_interface(CHR_CON18,&reg_val,0xFFFF,0);        
    //printf("Reg[0x%x]=%x, ", CHR_CON18, reg_val);
    //ret_val=pmic_read_interface(CHR_CON19,&reg_val,0xFFFF,0);        
    //printf("Reg[0x%x]=%x \n", CHR_CON19, reg_val);

/********* Step A *************************************/
    //printf("mt_charger_type_detection : step A\r\n");
    
    //RG_BC11_IPU_EN[1.0] = 00
    ret_val=pmic_config_interface(CHR_CON19,0x0,PMIC_RG_BC11_IPU_EN_MASK,PMIC_RG_BC11_IPU_EN_SHIFT);
    
    SETREG16(USBPHYRegs+0x1C,0x1000);//RG_PUPD_BIST_EN = 1    
    CLRREG16(USBPHYRegs+0x1C,0x0400);//RG_EN_PD_DM=0
    
    //RG_BC11_VSRC_EN[1.0] = 10 
    ret_val=pmic_config_interface(CHR_CON18,0x2,PMIC_RG_BC11_VSRC_EN_MASK,PMIC_RG_BC11_VSRC_EN_SHIFT); 
    //RG_BC11_IPD_EN[1:0] = 01
    ret_val=pmic_config_interface(CHR_CON19,0x1,PMIC_RG_BC11_IPD_EN_MASK,PMIC_RG_BC11_IPD_EN_SHIFT);
    //RG_BC11_VREF_VTH = 0
    ret_val=pmic_config_interface(CHR_CON19,0x0,PMIC_RG_BC11_VREF_VTH_MASK,PMIC_RG_BC11_VREF_VTH_SHIFT);
    //RG_BC11_CMP_EN[1.0] = 01
    ret_val=pmic_config_interface(CHR_CON19,0x1,PMIC_RG_BC11_CMP_EN_MASK,PMIC_RG_BC11_CMP_EN_SHIFT);

    mdelay(100);
        
    ret_val=pmic_read_interface(CHR_CON18,&wChargerAvail,PMIC_RGS_BC11_CMP_OUT_MASK,PMIC_RGS_BC11_CMP_OUT_SHIFT); 
    //printf("mt_charger_type_detection : step A : wChargerAvail=%x\r\n", wChargerAvail);
    
    //RG_BC11_VSRC_EN[1:0]=00
    ret_val=pmic_config_interface(CHR_CON18,0x0,PMIC_RG_BC11_VSRC_EN_MASK,PMIC_RG_BC11_VSRC_EN_SHIFT); 
    //RG_BC11_IPD_EN[1.0] = 00
    ret_val=pmic_config_interface(CHR_CON19,0x0,PMIC_RG_BC11_IPD_EN_MASK,PMIC_RG_BC11_IPD_EN_SHIFT);
    //RG_BC11_CMP_EN[1.0] = 00
    ret_val=pmic_config_interface(CHR_CON19,0x0,PMIC_RG_BC11_CMP_EN_MASK,PMIC_RG_BC11_CMP_EN_SHIFT);
    
    mdelay(50);
    
    if(wChargerAvail==1)
    {
/********* Step B *************************************/
        //printf("mt_charger_type_detection : step B\r\n");

        //RG_BC11_IPU_EN[1:0]=10
        ret_val=pmic_config_interface(CHR_CON19,0x2,PMIC_RG_BC11_IPU_EN_MASK,PMIC_RG_BC11_IPU_EN_SHIFT);        

        mdelay(80);
        
        bLineState_B = INREG16(USBPHYRegs+0x76);
        //printf("mt_charger_type_detection : step B : bLineState_B=%x\r\n", bLineState_B);
        if(bLineState_B & 0x80)
        {
            ret = PMIC_STANDARD_CHARGER;
            printf("pl pmic STANDARD CHARGER\r\n");
        }
        else
        {
            ret = PMIC_CHARGING_HOST;
            printf("pl pmic Charging Host\r\n");
        }
    }
    else
    {
/********* Step C *************************************/
        //printf("mt_charger_type_detection : step C\r\n");

        //RG_BC11_IPU_EN[1:0]=01
        ret_val=pmic_config_interface(CHR_CON19,0x1,PMIC_RG_BC11_IPU_EN_MASK,PMIC_RG_BC11_IPU_EN_SHIFT);
        //RG_BC11_CMP_EN[1.0] = 01
        ret_val=pmic_config_interface(CHR_CON19,0x1,PMIC_RG_BC11_CMP_EN_MASK,PMIC_RG_BC11_CMP_EN_SHIFT);
        
        //ret_val=pmic_read_interface(CHR_CON19,&reg_val,0xFFFF,0);        
        //printf("mt_charger_type_detection : step C : Reg[0x%x]=%x\r\n", CHR_CON19, reg_val);        
        
        mdelay(80);
                
        ret_val=pmic_read_interface(CHR_CON18,&bLineState_C,0xFFFF,0);
        //printf("mt_charger_type_detection : step C : bLineState_C=%x\r\n", bLineState_C);
        if(bLineState_C & 0x0080)
        {
            ret = PMIC_NONSTANDARD_CHARGER;
            printf("pl pmic UNSTANDARD CHARGER\r\n");
            
            //RG_BC11_IPU_EN[1:0]=10
            ret_val=pmic_config_interface(CHR_CON19,0x2,PMIC_RG_BC11_IPU_EN_MASK,PMIC_RG_BC11_IPU_EN_SHIFT);
            
            mdelay(80);
        }
        else
        {
            ret = PMIC_STANDARD_HOST;
            printf("pl pmic Standard USB Host\r\n");
        }
    }
/********* Finally setting *******************************/

    //RG_BC11_VSRC_EN[1:0]=00
    ret_val=pmic_config_interface(CHR_CON18,0x0,PMIC_RG_BC11_VSRC_EN_MASK,PMIC_RG_BC11_VSRC_EN_SHIFT); 
    //RG_BC11_VREF_VTH = 0
    ret_val=pmic_config_interface(CHR_CON19,0x0,PMIC_RG_BC11_VREF_VTH_MASK,PMIC_RG_BC11_VREF_VTH_SHIFT);
    //RG_BC11_CMP_EN[1.0] = 00
    ret_val=pmic_config_interface(CHR_CON19,0x0,PMIC_RG_BC11_CMP_EN_MASK,PMIC_RG_BC11_CMP_EN_SHIFT);
    //RG_BC11_IPU_EN[1.0] = 00
    ret_val=pmic_config_interface(CHR_CON19,0x0,PMIC_RG_BC11_IPU_EN_MASK,PMIC_RG_BC11_IPU_EN_SHIFT);
    //RG_BC11_IPD_EN[1.0] = 00
    ret_val=pmic_config_interface(CHR_CON19,0x0,PMIC_RG_BC11_IPD_EN_MASK,PMIC_RG_BC11_IPD_EN_SHIFT);
    //RG_BC11_BIAS_EN=0
    ret_val=pmic_config_interface(CHR_CON19,0x0,PMIC_RG_BC11_BIAS_EN_MASK,PMIC_RG_BC11_BIAS_EN_SHIFT); 
    
    CLRREG16(USB_U2PHYACR6_2,0x80); //bit 7 = 0 : switch to USB

    //hwPowerDown(MT65XX_POWER_LDO_VUSB,"VUSB_LDO");
    //disable_pll(MT65XX_UPLL,"USB_PLL");
    //printf("[hw_charger_type_detection] disable VUSB and UPLL before disconnect\n");

    if( (ret==PMIC_STANDARD_HOST) || (ret==PMIC_CHARGING_HOST) )
    {
        printf("pl pmic for USB\r\n");
        //RG_BC11_BB_CTRL=1
        ret_val=pmic_config_interface(CHR_CON18,0x1,PMIC_RG_BC11_BB_CTRL_MASK,PMIC_RG_BC11_BB_CTRL_SHIFT);
        //RG_BC11_BIAS_EN=1
        ret_val=pmic_config_interface(CHR_CON19,0x1,PMIC_RG_BC11_BIAS_EN_MASK,PMIC_RG_BC11_BIAS_EN_SHIFT); 
        //RG_BC11_VSRC_EN[1.0] = 11        
        ret_val=pmic_config_interface(CHR_CON18,0x3,PMIC_RG_BC11_VSRC_EN_MASK,PMIC_RG_BC11_VSRC_EN_SHIFT);
        //check
        ret_val=pmic_read_interface(CHR_CON18,&reg_val,0xFFFF,0);
        printf("[0x%x]=0x%x\n", CHR_CON18, reg_val);
        ret_val=pmic_read_interface(CHR_CON19,&reg_val,0xFFFF,0);
        printf("[0x%x]=0x%x\n", CHR_CON19, reg_val);
    }        
#endif

    //step4:done, ret the type    
    return ret;    
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
        printf("Got data, %d, %d\r\n", g_charger_in_flag, g_first_check);
    }

    return g_ret;
}

//==============================================================================
// PMIC6320 Usage APIs
//==============================================================================
U32 get_pmic6320_chip_version (void)
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
        case 400:  ret_val=pmic_config_interface(CHR_CON4, 0x0D, PMIC_RG_CS_VTH_MASK, PMIC_RG_CS_VTH_SHIFT); break;
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
    //upmu_chr_chrwdt_int_en(1);              // CHRWDT_INT_EN
    ret_val=pmic_config_interface(CHR_CON15, 0x01, PMIC_RG_CHRWDT_INT_EN_MASK, PMIC_RG_CHRWDT_INT_EN_SHIFT);
    //upmu_chr_chrwdt_en(1);                  // CHRWDT_EN
    ret_val=pmic_config_interface(CHR_CON13, 0x01, PMIC_RG_CHRWDT_EN_MASK, PMIC_RG_CHRWDT_EN_SHIFT);
    //upmu_chr_chrwdt_flag_wr(1);             // CHRWDT_FLAG
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
    U32 ret_val=0;
    U32 reg_val=0;

    ret_val=pmic_config_interface(CHR_CON7,    0x01, PMIC_RG_BATON_EN_MASK, PMIC_RG_BATON_EN_SHIFT);      //BATON_EN=1
    ret_val=pmic_config_interface(CHR_CON7,    0x00, PMIC_BATON_TDET_EN_MASK, PMIC_BATON_TDET_EN_SHIFT);  //BATON_TDET_EN=0
    ret_val=pmic_config_interface(AUXADC_CON0, 0x00, PMIC_RG_BUF_PWD_B_MASK, PMIC_RG_BUF_PWD_B_SHIFT);    //RG_BUF_PWD_B=0
    //dump to check
    ret_val=pmic_read_interface(CHR_CON7,&reg_val,0xFFFF,0x0);    print("[0x%x]=0x%x\n",CHR_CON7,reg_val);
    ret_val=pmic_read_interface(AUXADC_CON0,&reg_val,0xFFFF,0x0); print("[0x%x]=0x%x\n",AUXADC_CON0,reg_val);

    ret_val=pmic_read_interface(CHR_CON7, &reg_val, PMIC_RGS_BATON_UNDET_MASK, PMIC_RGS_BATON_UNDET_SHIFT);

    if (reg_val == 1)
    {                     
        printf("No Battery\n");

        //ret_val=pmic_config_interface(CHR_CON7,    0x00, PMIC_RG_BATON_EN_MASK, PMIC_RG_BATON_EN_SHIFT);      //BATON_EN=0
        //ret_val=pmic_config_interface(CHR_CON7,    0x00, PMIC_BATON_TDET_EN_MASK, PMIC_BATON_TDET_EN_SHIFT);  //BATON_TDET_EN=0
        //ret_val=pmic_config_interface(AUXADC_CON0, 0x00, PMIC_RG_BUF_PWD_B_MASK, PMIC_RG_BUF_PWD_B_SHIFT);    //RG_BUF_PWD_B=0
        //dump to check
        ret_val=pmic_read_interface(CHR_CON7,&reg_val,0xFFFF,0x0);    print("[0x%x]=0x%x\n",CHR_CON7,reg_val);
        ret_val=pmic_read_interface(AUXADC_CON0,&reg_val,0xFFFF,0x0); print("[0x%x]=0x%x\n",AUXADC_CON0,reg_val);                
        
        return 0;        
    }
    else
    {
        printf("Battery exist\n");

        //ret_val=pmic_config_interface(CHR_CON7,    0x00, PMIC_RG_BATON_EN_MASK, PMIC_RG_BATON_EN_SHIFT);      //BATON_EN=0
        //ret_val=pmic_config_interface(CHR_CON7,    0x00, PMIC_BATON_TDET_EN_MASK, PMIC_BATON_TDET_EN_SHIFT);  //BATON_TDET_EN=0
        //ret_val=pmic_config_interface(AUXADC_CON0, 0x00, PMIC_RG_BUF_PWD_B_MASK, PMIC_RG_BUF_PWD_B_SHIFT);    //RG_BUF_PWD_B=0
        //dump to check
        ret_val=pmic_read_interface(CHR_CON7,&reg_val,0xFF,0x0);    print("[0x%x]=0x%x\n",CHR_CON7,reg_val);
        ret_val=pmic_read_interface(AUXADC_CON0,&reg_val,0xFF,0x0); print("[0x%x]=0x%x\n",AUXADC_CON0,reg_val);
    
        pl_hw_ulc_det();
        
        return 1;
    }
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
    //upmu_chr_chrwdt_int_en(1);             // CHRWDT_INT_EN
    ret_val=pmic_config_interface(CHR_CON15, 0x01, PMIC_RG_CHRWDT_INT_EN_MASK, PMIC_RG_CHRWDT_INT_EN_SHIFT);
    //upmu_chr_chrwdt_en(1);                   // CHRWDT_EN
    ret_val=pmic_config_interface(CHR_CON13, 0x01, PMIC_RG_CHRWDT_EN_MASK, PMIC_RG_CHRWDT_EN_SHIFT);
    //upmu_chr_chrwdt_flag_wr(1);            // CHRWDT_FLAG
    ret_val=pmic_config_interface(CHR_CON15, 0x01, PMIC_RG_CHRWDT_FLAG_WR_MASK, PMIC_RG_CHRWDT_FLAG_WR_SHIFT);

    //printf("[pl_kick_chr_wdt] done\n");
}

void pl_close_pre_chr_led(void)
{
    U32 ret_val=0;    

    ret_val=pmic_config_interface(CHR_CON22, 0x00, PMIC_RG_CHRIND_ON_MASK, PMIC_RG_CHRIND_ON_SHIFT);
    
    printf("pl pmic close pre-chr LED\n");
}

void pmic_efuse_trimming(void)
{
    U32 ret_val=0;
    U32 reg_val=0;

    print("pl pmic efuse start\n");    
    
    //BUCK
    ret_val=pmic_read_interface(0x01CA, &reg_val, 0x1, 6 );
    if(reg_val == 1)
    {
        print("pl pmic efuse BUCK trim\n");
        ret_val=pmic_read_interface(0x01C8, &reg_val, 0x1, 10); ret_val=pmic_config_interface(0x020E, reg_val, 0x1, 4);
        ret_val=pmic_read_interface(0x01C8, &reg_val, 0x1, 11); ret_val=pmic_config_interface(0x020E, reg_val, 0x1, 5);
        ret_val=pmic_read_interface(0x01C8, &reg_val, 0x1, 12); ret_val=pmic_config_interface(0x020E, reg_val, 0x1, 6);
        ret_val=pmic_read_interface(0x01C8, &reg_val, 0x1, 13); ret_val=pmic_config_interface(0x020E, reg_val, 0x1, 7);
        ret_val=pmic_read_interface(0x01C8, &reg_val, 0x1, 14); ret_val=pmic_config_interface(0x0260, reg_val, 0x1, 4);
        ret_val=pmic_read_interface(0x01C8, &reg_val, 0x1, 15); ret_val=pmic_config_interface(0x0260, reg_val, 0x1, 5);
        ret_val=pmic_read_interface(0x01CA, &reg_val, 0x1, 0 ); ret_val=pmic_config_interface(0x0260, reg_val, 0x1, 6);
        ret_val=pmic_read_interface(0x01CA, &reg_val, 0x1, 1 ); ret_val=pmic_config_interface(0x0260, reg_val, 0x1, 7);
        ret_val=pmic_read_interface(0x01CA, &reg_val, 0x1, 2 ); ret_val=pmic_config_interface(0x0286, reg_val, 0x1, 4);
        ret_val=pmic_read_interface(0x01CA, &reg_val, 0x1, 3 ); ret_val=pmic_config_interface(0x0286, reg_val, 0x1, 5);
        ret_val=pmic_read_interface(0x01CA, &reg_val, 0x1, 4 ); ret_val=pmic_config_interface(0x0286, reg_val, 0x1, 6);
        ret_val=pmic_read_interface(0x01CA, &reg_val, 0x1, 5 ); ret_val=pmic_config_interface(0x0286, reg_val, 0x1, 7);
    }
    //log    
    ret_val=pmic_read_interface(0x01C8,&reg_val,0xFFFF,0x0); print("[0x%x]=0x%x\n", 0x01C8, reg_val);
    ret_val=pmic_read_interface(0x01CA,&reg_val,0xFFFF,0x0); print("[0x%x]=0x%x\n", 0x01CA, reg_val);
    ret_val=pmic_read_interface(0x020E,&reg_val,0xFFFF,0x0); print("[0x%x]=0x%x\n", 0x020E, reg_val);
    ret_val=pmic_read_interface(0x0260,&reg_val,0xFFFF,0x0); print("[0x%x]=0x%x\n", 0x0260, reg_val);
    ret_val=pmic_read_interface(0x0286,&reg_val,0xFFFF,0x0); print("[0x%x]=0x%x\n", 0x0286, reg_val);
        
    print("pl pmic efuse end\n");
}

void pmic_vm_tunning(void)
{
    U32 ret_val=0;
    U32 reg_val=0;

    ret_val=pmic_read_interface(VM_CON9,&reg_val,0x007F,0x0);
    print("pl vm read [0x%x]=0x%x\n",VM_CON9,reg_val);

    if(reg_val <= 0x7D)
    {
        reg_val += 0x2;
        ret_val=pmic_config_interface(VM_CON9, reg_val, 0x007F, 0x0);
        print("pl vm set [0x%x]=0x%x\n",VM_CON9,reg_val);
    }
    else
    {
        print("pl vm no need tune\n");
    }

    ret_val=pmic_read_interface(VM_CON9,&reg_val,0x007F,0x0);
    print("pl vm check [0x%x]=0x%x\n",VM_CON9,reg_val);
}
void hqa_pmic_voltage_read(UINT8 temp)
{
    int ret_val = 0;
    unsigned int OldVcore1 = 0;
    unsigned int OldVcore2 = 0;
    unsigned int NewVcore1 = 0;
    unsigned int NewVcore2 = 0;
    unsigned int OldVmem1 = 0;
    unsigned int OldVmem2 = 0;
    unsigned int NewVmem1 = 0;
    unsigned int NewVmem2 = 0;
    print("[PMIC][HQA]pmic_voltage_read %d : ", temp);
    ret_val=pmic_read_interface(0x26A,&OldVcore1,0x7F,0);
    ret_val=pmic_read_interface(0x26C,&OldVcore2,0x7F,0);
    ret_val=pmic_read_interface(0x290,&OldVmem1, 0x7F,0);
    ret_val=pmic_read_interface(0x292,&OldVmem2, 0x7F,0);
    print("0x%x, 0x%x, 0x%x, 0x%x \r\n", OldVcore1,OldVcore2,OldVmem1,OldVmem2);
}

#if 0
#define ETT_HV 1
#define ETT_LV 0
#endif
void hqa_pmic_voltage_adjust()
{

    print("[PMIC][HQA]orig:pmic_voltage_org:\n");
    hqa_pmic_voltage_read(0);

    #if ETT_HV
    print("[PMIC]pmic_voltage_adjust HV\n");
    //Vcore 1.156V
    pmic_config_interface(0x26A, 0x45 + 0x03, 0x7F,0);
    pmic_config_interface(0x26C, 0x45 + 0x03, 0x7F,0);
    //Vmem 1.303V
    pmic_config_interface(0x290, 0x5C + 0x06, 0x7F,0);
    pmic_config_interface(0x292, 0x5C + 0x06, 0x7F,0);
    #elif ETT_LV
    print("[PMIC]pmic_voltage_adjust LV\n");
    //Vcore 0.945V
    pmic_config_interface(0x26A, 0x23 + 0x02, 0x7F,0);
    pmic_config_interface(0x26C, 0x23 + 0x02, 0x7F,0);
    //Vmem 1.137V
    pmic_config_interface(0x290, 0x41 + 0x03, 0x7F,0);
    pmic_config_interface(0x292, 0x41 + 0x03, 0x7F,0);
    #endif

    print("[PMIC][HQA]adjust:pmic_voltage_org:\n");
    hqa_pmic_voltage_read(0);
}

//==============================================================================
// PMIC6320 Init Code
//==============================================================================
U32 pmic6320_init (void)
{
    U32 ret_code = PMIC_TEST_PASS;
    int ret_val=0;
    int reg_val=0;

    print("pl pmic init start\n");

    pmic_efuse_trimming();

    //Enable PMIC RST function (depends on main chip RST function)
    ret_val=pmic_config_interface(TOP_RST_MISC,  0x1, PMIC_RG_SYSRSTB_EN_MASK, PMIC_RG_SYSRSTB_EN_SHIFT);
    //ret_val=pmic_config_interface(TOP_RST_MISC,  0x1, PMIC_RG_STRUP_MAN_RST_EN_MASK, PMIC_RG_STRUP_MAN_RST_EN_SHIFT);
    ret_val=pmic_read_interface(TOP_RST_MISC, &reg_val, 0xFFFF, 0);
    print("pl pmic en rst [0x%x]=0x%x\n", TOP_RST_MISC, reg_val);
    
#ifdef WORKAROUND_GPU_VRF18_2
    /****SMT workaround!!! Leon.chen ***/
    //set VRF18_2=1.05V
    ret_val=pmic_config_interface(VRF18_2_CON9,  0x0, PMIC_VRF18_2_VOSEL_MASK, PMIC_VRF18_2_VOSEL_SHIFT);
    ret_val=pmic_config_interface(VRF18_2_CON10, 0x0, PMIC_VRF18_2_VOSEL_ON_MASK, PMIC_VRF18_2_VOSEL_ON_SHIFT);
    ret_val=pmic_config_interface(VRF18_2_CON11, 0x0, PMIC_VRF18_2_VOSEL_SLEEP_MASK, PMIC_VRF18_2_VOSEL_SLEEP_SHIFT);
    //Turn on VRF18_2
    ret_val=pmic_config_interface(VRF18_2_CON7, 0x1, PMIC_VRF18_2_EN_MASK, PMIC_VRF18_2_EN_SHIFT);
    //debug
    ret_val=pmic_read_interface(VRF18_2_CON7, &reg_val, 0xFFFF, 0); 
    print("[0x%x]=0x%x\n", VRF18_2_CON7, reg_val);
    ret_val=pmic_read_interface(VRF18_2_CON9, &reg_val, 0xFFFF, 0); 
    print("[0x%x]=0x%x\n", VRF18_2_CON9, reg_val);
    ret_val=pmic_read_interface(VRF18_2_CON10, &reg_val, 0xFFFF, 0); 
    print("[0x%x]=0x%x\n", VRF18_2_CON10, reg_val);
    ret_val=pmic_read_interface(VRF18_2_CON11, &reg_val, 0xFFFF, 0); 
    print("[0x%x]=0x%x\n", VRF18_2_CON11, reg_val);
    print("Leon VRF18_2\n");
    /****SMT workaround!!! Leon.chen ***/
#endif
    
    //print("[pmic6320_init] PMIC CHIP Code = %d\n", get_pmic6320_chip_version());    

#ifndef EVB_PLATFORM
    //Enable PMIC HW reset function
    //ret_val=pmic_config_interface(TOP_RST_MISC, 0x01, PMIC_RG_PWRKEY_RST_EN_MASK, PMIC_RG_PWRKEY_RST_EN_SHIFT);
    //ret_val=pmic_config_interface(TOP_RST_MISC, 0x01, PMIC_RG_HOMEKEY_RST_EN_MASK, PMIC_RG_HOMEKEY_RST_EN_SHIFT);
#endif    

    hw_check_battery();
    //printf("[pmic6320_init] hw_check_battery\n");

    pmic_vm_tunning();    
#if 0
    //only for dram HQA stress test use
    hqa_pmic_voltage_adjust();
#endif
    print("pl pmic init done\n");

    return ret_code;
}

