#include <target/board.h>

//#define CFG_POWER_CHARGING

#ifdef CFG_POWER_CHARGING
#include <platform/mt_typedefs.h>
#include <platform/mt_reg_base.h>
#include <platform/mt_pmic.h>
#include <platform/boot_mode.h>
#include <platform/mt_gpt.h>
#include <platform/mt_rtc.h>
#include <platform/mt_disp_drv.h>
#include <platform/mtk_wdt.h>
#include <platform/mtk_key.h>
#include <platform/mt_logo.h>
#include <platform/mt_leds.h>
#include <platform/mt_gpio.h>
#include <platform/bq24196.h>
#include <printf.h>
#include <sys/types.h>
#include <target/cust_battery.h>

#ifdef MTK_BQ27541_SUPPORT
#include <platform/mt_i2c.h>
#include <platform/bq27541.h>
#endif

#undef printf

//#define CONFIG_DEBUG_MSG

/*****************************************************************************
 *  Type define
 ****************************************************************************/
typedef unsigned int       WORD;

typedef enum
{
    USB_SUSPEND = 0,
    USB_UNCONFIGURED,
    USB_CONFIGURED
} usb_state_enum;

/*****************************************************************************
*   JEITA battery temperature standard 
    charging info ,like temperatue, charging current, re-charging voltage, CV threshold would be reconfigurated.
    Temperature hysteresis default 6C.  
    Reference table:
    degree    AC Current   USB current    CV threshold    Recharge Vol    hysteresis condition 
    > 60      no charging current,              X                X              <54(Down) 
    45~60     FULL         500mA           4.096V           3.996V              <39(Down) >60(Up) 
    10~45     FULL         500mA           4.208V           4.108V              <10(Down) >45(Up) 
    0~10      FULL         500mA           4.096V           3.996V              <0(Down)  >16(Up) 
    -10~0     500mA        500mA               4V             3.9V              <-10(Down) >6(Up) 
    <-10      no charging current,              X                X              >-10(Up)  
****************************************************************************/
typedef enum
{
    TEMP_BELOW_NEG_10 = 0,
    TEMP_NEG_10_TO_POS_0,
    TEMP_POS_0_TO_POS_10,
    TEMP_POS_10_TO_POS_45,
    TEMP_POS_45_TO_POS_60,
    TEMP_ABOVE_POS_60
}temp_state_enum;
    
#define TEMP_POS_60_THRESHOLD  60
#define TEMP_POS_60_THRES_MINUS_X_DEGREE 54  

#define TEMP_POS_45_THRESHOLD  45
#define TEMP_POS_45_THRES_MINUS_X_DEGREE 39

#define TEMP_POS_10_THRESHOLD  10
#define TEMP_POS_10_THRES_PLUS_X_DEGREE 16

#define TEMP_POS_0_THRESHOLD  0
#define TEMP_POS_0_THRES_PLUS_X_DEGREE 6

#define TEMP_NEG_10_THRESHOLD  -10
#define TEMP_NEG_10_THRES_PLUS_X_DEGREE  -8  //-10 not threshold

#if defined(MTK_JEITA_STANDARD_SUPPORT)
int g_jeita_recharging_voltage=4110;
int g_temp_status=TEMP_POS_10_TO_POS_45;
kal_bool temp_error_recovery_chr_flag =KAL_TRUE;
#endif

/*****************************************************************************
 *  BATTERY VOLTAGE
 ****************************************************************************/
#define BATTERY_LOWVOL_THRESOLD             3450
#define CHR_OUT_CURRENT                     100

/*****************************************************************************
 *  BATTERY TIMER
 ****************************************************************************/
#define MAX_CHARGING_TIME                   24*60*60    // 24hr
#define MAX_POSTFULL_SAFETY_TIME            1*30*60     // 30mins
#define MAX_PreCC_CHARGING_TIME             1*30*60     // 0.5hr
#define MAX_CV_CHARGING_TIME                3*60*60     // 3hr
#define BAT_TASK_PERIOD                     1           // 1sec
#define BL_SWITCH_TIMEOUT                   1*6         // 6s  
#define POWER_ON_TIME                       4*1         // 0.5s

/*****************************************************************************
 *  BATTERY Protection
 ****************************************************************************/
#define charger_OVER_VOL                    1
#define ADC_SAMPLE_TIMES                    5

/*****************************************************************************
 *  Pulse Charging State
 ****************************************************************************/
#define  CHR_PRE                            0x1000
#define  CHR_CC                             0x1001 
#define  CHR_TOP_OFF                        0x1002 
#define  CHR_POST_FULL                      0x1003
#define  CHR_BATFULL                        0x1004 
#define  CHR_ERROR                          0x1005

///////////////////////////////////////////////////////////////////////////////////////////
//// Smart Battery Structure
///////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
    kal_bool   	bat_exist;
    kal_bool   	bat_full;
    kal_bool   	bat_low;
    UINT32      bat_charging_state;
    INT32      bat_vol;
    kal_bool    charger_exist;
    UINT32      pre_charging_current;
    UINT32      charging_current;
    UINT32      charger_vol;
    UINT32      charger_protect_status;
    UINT32      ISENSE;
    INT32      ICharging;
    INT32      temperature;
    UINT32      total_charging_time;
    UINT32      PRE_charging_time;
    UINT32      CC_charging_time;
    UINT32      TOPOFF_charging_time;
    UINT32      POSTFULL_charging_time;
    UINT32      charger_type;
    UINT32      PWR_SRC;
    INT32      SOC;
    INT32      ADC_BAT_SENSE;
    INT32      ADC_I_SENSE;
} PMU_ChargerStruct;

typedef enum
{
    PMU_STATUS_OK = 0,
    PMU_STATUS_FAIL = 1,
} PMU_STATUS;

/////////////////////////////////////////////////////////////////////
//// Global Variable
/////////////////////////////////////////////////////////////////////
static CHARGER_TYPE CHR_Type_num = CHARGER_UNKNOWN;
static unsigned short batteryVoltageBuffer[BATTERY_AVERAGE_SIZE];
static unsigned short batteryCurrentBuffer[BATTERY_AVERAGE_SIZE];
#ifndef MTK_BQ27541_SUPPORT
static unsigned short batterySOCBuffer[BATTERY_AVERAGE_SIZE];
#endif
static int batteryIndex = 0;
static int batteryVoltageSum = 0;
static int batteryCurrentSum = 0;
#ifndef MTK_BQ27541_SUPPORT
static int batterySOCSum = 0;
#endif
PMU_ChargerStruct BMT_status;
kal_bool g_bat_full_user_view = KAL_FALSE;
kal_bool g_Battery_Fail = KAL_FALSE;
kal_bool batteryBufferFirst = KAL_FALSE;

int V_PRE2CC_THRES = 3400;
int V_CC2TOPOFF_THRES = 4050;

int g_HW_Charging_Done = 0;
int g_Charging_Over_Time = 0;

int g_bl_on = 1;

int g_thread_count = 10;

int CHARGING_FULL_CURRENT = 220;    // mA on phone

int g_bat_temperature_pre=22;

int gADC_BAT_SENSE_temp=0;
int gADC_I_SENSE_temp=0;
int gADC_I_SENSE_offset=0;

int g_R_BAT_SENSE = R_BAT_SENSE;
int g_R_I_SENSE = R_I_SENSE;
int g_R_CHARGER_1 = R_CHARGER_1;
int g_R_CHARGER_2 = R_CHARGER_2;

/*****************************************************************************
 * EM
****************************************************************************/
int g_BatteryAverageCurrent = 0;

/*****************************************************************************
 * USB-IF
****************************************************************************/
int g_usb_state = USB_UNCONFIGURED;
int g_temp_CC_value = 0;

/*****************************************************************************
 * Logging System
****************************************************************************/
int g_chr_event = 0;
int bat_volt_cp_flag = 0;
int Enable_BATDRV_LOG = 1;

/***************************************************
 * LK 
****************************************************/
int prog = 25;
int prog_temp = 0;
int prog_first = 1;
int bl_switch_timer = 0;
int bat_volt_check_point = 0;
int getVoltFlag = 0;
int low_bat_boot_display=0;
int charger_ov_boot_display = 0;

kal_bool bl_switch = KAL_FALSE;
kal_bool user_view_flag = KAL_FALSE;

int vbat_compensate_cp = 4185; //4185mV
int vbat_compensate_value = 80; //80mV

extern BOOT_ARGUMENT *g_boot_arg;

/********************************************** 
 * Battery Temprature Parameters and functions    
 ***********************************************/
typedef struct{
    INT32 BatteryTemp;
    INT32 TemperatureR;
} BATT_TEMPERATURE;

int get_bat_sense_volt_ch0(int times)
{
    kal_uint32 ret;
	//RG_SOURCE_CH0_NORM_SEL set to 0 for bat_sense
    ret=pmic_config_interface(AUXADC_CON14, 0x0, PMIC_RG_SOURCE_CH0_NORM_SEL_MASK, PMIC_RG_SOURCE_CH0_NORM_SEL_SHIFT);	
    return PMIC_IMM_GetOneChannelValue(0,times,1);
}

int get_i_sense_volt_ch0(int times)
{
    kal_uint32 ret;
	//RG_SOURCE_CH0_NORM_SEL set to 1 for i_sense
    ret=pmic_config_interface(AUXADC_CON14, 0x1, PMIC_RG_SOURCE_CH0_NORM_SEL_MASK, PMIC_RG_SOURCE_CH0_NORM_SEL_SHIFT);	
    return PMIC_IMM_GetOneChannelValue(0,times,1); //i sense use channel 0 as well
}

/* convert register to temperature  */
INT16 BattThermistorConverTemp(INT32 Res)
{
    int i = 0;
    INT32 RES1 = 0, RES2 = 0;
    INT32 TBatt_Value = -200, TMP1 = 0, TMP2 = 0;

#if defined(BAT_NTC_10_TDK_1)        
    BATT_TEMPERATURE Batt_Temperature_Table[] = {
     {-20,95327},
     {-15,71746},
     {-10,54564},
     { -5,41813},
     {  0,32330},
     {  5,25194},
     { 10,19785},
     { 15,15651},
     { 20,12468},
     { 25,10000},
     { 30,8072},
     { 35,6556},
     { 40,5356},
     { 45,4401},
     { 50,3635},
     { 55,3019},
     { 60,2521}
    };
#endif

#if defined(BAT_NTC_TSM_1)
    BATT_TEMPERATURE Batt_Temperature_Table[] = {
    {-20,70603},    
    {-15,55183},
    {-10,43499},
    { -5,34569},
    {  0,27680},
    {  5,22316},
    { 10,18104},
    { 15,14773},
    { 20,12122},
    { 25,10000},
    { 30,8294},
    { 35,6915},
    { 40,5795},
    { 45,4882},
    { 50,4133},
    { 55,3516},
    { 60,3004}
    };
#endif

#if defined(BAT_NTC_10_SEN_1)        
    BATT_TEMPERATURE Batt_Temperature_Table[] = {
     {-20,74354},
     {-15,57626},
     {-10,45068},
     { -5,35548},
     {  0,28267},
     {  5,22650},
     { 10,18280},
     { 15,14855},
     { 20,12151},
     { 25,10000},
     { 30,8279},
     { 35,6892},
     { 40,5768},
     { 45,4852},
     { 50,4101},
     { 55,3483},
     { 60,2970}
    };
#endif

#if (BAT_NTC_10 == 1)
    BATT_TEMPERATURE Batt_Temperature_Table[] = {
        {-20,68237},
        {-15,53650},
        {-10,42506},
        { -5,33892},
        {  0,27219},
        {  5,22021},
        { 10,17926},
        { 15,14674},
        { 20,12081},
        { 25,10000},
        { 30,8315},
        { 35,6948},
        { 40,5834},
        { 45,4917},
        { 50,4161},
        { 55,3535},
        { 60,3014}        
    };
#endif

#if (BAT_NTC_47 == 1)
    BATT_TEMPERATURE Batt_Temperature_Table[] = {
        {-20,483954},
        {-15,360850},
        {-10,271697},
        { -5,206463},
        {  0,158214},
        {  5,122259},
        { 10,95227},
        { 15,74730},
        { 20,59065},
        { 25,47000},
        { 30,37643},
        { 35,30334},
        { 40,24591},
        { 45,20048},
        { 50,16433},
        { 55,13539},
        { 60,11210}        
    };
#endif

    if (Enable_BATDRV_LOG == 1) {
        printf("###### %d <-> %d ######\r\n", Batt_Temperature_Table[9].BatteryTemp, 
            Batt_Temperature_Table[9].TemperatureR);
    }

    if(Res >= Batt_Temperature_Table[0].TemperatureR)
    {
        #ifdef CONFIG_DEBUG_MSG_NO_BQ27500
        printf("Res >= %d\n", Batt_Temperature_Table[0].TemperatureR);
        #endif
        TBatt_Value = -20;
    }
    else if(Res <= Batt_Temperature_Table[16].TemperatureR)
    {
        #ifdef CONFIG_DEBUG_MSG_NO_BQ27500
        printf("Res <= %d\n", Batt_Temperature_Table[16].TemperatureR);
        #endif
        TBatt_Value = 60;
    }
    else
    {
        RES1 = Batt_Temperature_Table[0].TemperatureR;
        TMP1 = Batt_Temperature_Table[0].BatteryTemp;
        
        for (i = 0; i <= 16; i++)
        {
            if(Res >= Batt_Temperature_Table[i].TemperatureR)
            {
                RES2 = Batt_Temperature_Table[i].TemperatureR;
                TMP2 = Batt_Temperature_Table[i].BatteryTemp;
                break;
            }
            else
            {
                RES1 = Batt_Temperature_Table[i].TemperatureR;
                TMP1 = Batt_Temperature_Table[i].BatteryTemp;
            }
        }
        
        TBatt_Value = (((Res - RES2) * TMP1) + ((RES1 - Res) * TMP2)) / (RES1-RES2);
    }
    
    #ifdef CONFIG_DEBUG_MSG_NO_BQ27500
    printf("BattThermistorConverTemp() : TBatt_Value = %d\n",TBatt_Value);
    #endif
    
    return TBatt_Value;    
}

/* convert ADC_bat_temp_volt to register */
INT16 BattVoltToTemp(INT32 dwVolt)
{
    INT32 TRes;
    INT32 dwVCriBat = (TBAT_OVER_CRITICAL_LOW * RBAT_PULL_UP_VOLT) / (TBAT_OVER_CRITICAL_LOW + RBAT_PULL_UP_R); //~2000mV
    INT32 sBaTTMP = -100;
    
    if(dwVolt > dwVCriBat)
        TRes = TBAT_OVER_CRITICAL_LOW;
    else
        TRes = (RBAT_PULL_UP_R*dwVolt) / (RBAT_PULL_UP_VOLT-dwVolt);        
        
    /* convert register to temperature */
    sBaTTMP = BattThermistorConverTemp(TRes);
    
    #ifdef CONFIG_DEBUG_MSG_NO_BQ27500
    printf("BattVoltToTemp() : TBAT_OVER_CRITICAL_LOW = %d\n", TBAT_OVER_CRITICAL_LOW);
    printf("BattVoltToTemp() : RBAT_PULL_UP_VOLT = %d\n", RBAT_PULL_UP_VOLT);
    printf("BattVoltToTemp() : dwVolt = %d\n", dwVolt);
    printf("BattVoltToTemp() : TRes = %d\n", TRes);
    printf("BattVoltToTemp() : sBaTTMP = %d\n", sBaTTMP);
    #endif
    
#ifdef BEFORE_TREF_REWORK
    if(upmu_get_cid() == 0x1020)
        sBaTTMP=22; 
#endif
    return sBaTTMP;
}

//////////////////////////////////////////////////////
//// Pulse Charging Algorithm 
//////////////////////////////////////////////////////
void charger_hv_init(void)
{
    upmu_set_rg_vcdt_hv_vth(0xD);    //VCDT_HV_VTH, 8.5V
}

U32 get_charger_hv_status(void)
{
    return upmu_get_rgs_vcdt_hv_det();
}

void kick_charger_wdt(void)
{
    upmu_set_rg_chrwdt_td(0x0);           // CHRWDT_TD, 4s
    upmu_set_rg_chrwdt_int_en(1);         // CHRWDT_INT_EN
    upmu_set_rg_chrwdt_en(1);             // CHRWDT_EN
    upmu_set_rg_chrwdt_wr(1);             // CHRWDT_WR
}

kal_bool pmic_chrdet_status(void)
{
    if( upmu_is_chr_det() == KAL_TRUE )    
    {
        if (Enable_BATDRV_LOG == 1) {
            printf("[pmic_chrdet_status] Charger exist\r\n");
        }
        return KAL_TRUE;
    }
    else
    {
        if (Enable_BATDRV_LOG == 1) {
            printf("[pmic_chrdet_status] No charger\r\n");
        }
        return KAL_FALSE;
    }
}

void bq24196_set_ac_current(void)
{
    int ichg_val;

    if (Enable_BATDRV_LOG == 1) {
        printf("[BATTERY:bq24196] bq24196_set_ac_current \r\n");
    }

    if((g_temp_CC_value > 2484) || (g_temp_CC_value < 500))
    {
        printf("[BATTERY:bq24196] invalid current selected (%d), use 500mA \r\n", g_temp_CC_value);
        bq24196_set_ichg(0); //offset is 500mA
    }
    else
    {
#if defined(MTK_JEITA_STANDARD_SUPPORT)        
        if(g_temp_status == TEMP_NEG_10_TO_POS_0)
        {    
            ichg_val = 0; //lowest possible charge current for bq24196 is 500mA
        }
        else
        {
            ichg_val = (g_temp_CC_value - 500)/64;
        }     
#else
        ichg_val = (g_temp_CC_value - 500)/64;
#endif    
        bq24196_set_ichg(ichg_val);
    }
}

void bq24196_set_low_chg_current(void)
{
    if (Enable_BATDRV_LOG == 1) {
        printf("[BATTERY:bq24196] bq24196_set_low_chg_current \r\n");
    }
    bq24196_set_iinlim(0x0); //IN current limit at 100mA	
    bq24196_set_ichg(0); //charging current limit at 500mA
}

void select_charging_curret_bq24196(void)
{
    if ( BMT_status.charger_type == STANDARD_HOST )
    {
        g_temp_CC_value = USB_CHARGER_CURRENT;
        bq24196_set_iinlim(0x2); //IN current limit at 500mA		  
        bq24196_set_ac_current();
        if (Enable_BATDRV_LOG == 1) {
            printf("Power/Battery", "[BATTERY:bq24196] bq24196_set_ac_current(), CC value(%d) \r\n", g_temp_CC_value);
        }
    }
    else if (BMT_status.charger_type == NONSTANDARD_CHARGER)
    {
        g_temp_CC_value = AC_CHARGER_CURRENT;
        bq24196_set_iinlim(0x6); //IN current limit at 2A			
        bq24196_set_ac_current();
    }
    else if (BMT_status.charger_type == STANDARD_CHARGER)
    {
        g_temp_CC_value = AC_CHARGER_CURRENT;
        bq24196_set_iinlim(0x6); //IN current limit at 2A			
        bq24196_set_ac_current();
    }
    else if (BMT_status.charger_type == CHARGING_HOST)
    {
        g_temp_CC_value = AC_CHARGER_CURRENT;
        bq24196_set_iinlim(0x6); //IN current limit at 2A			
        bq24196_set_ac_current();
    }
    else
    {
        if (Enable_BATDRV_LOG == 1) {
            printf("[BATTERY:bq24196] Unknown charger type\n");
        }
        g_temp_CC_value = 500;
        bq24196_set_iinlim(0x2); //IN current limit at 500mA		  
        bq24196_set_ac_current();     		
//        bq24196_set_low_chg_current();
    }
}

void ChargerHwInit_bq24196(void)
{
	if (Enable_BATDRV_LOG == 1) {
		printf("[BATTERY:bq24196] ChargerHwInit_bq24196\n" );
	}

    upmu_set_rg_bc11_bb_ctrl(1);    //BC11_BB_CTRL    
    upmu_set_rg_bc11_rst(1);        //BC11_RST

    //TODO: pull PSEL low
    //TODO: pull CE low
    bq24196_set_en_hiz(0x0);
    bq24196_set_vindpm(0xA); //VIN DPM check 4.68V
    bq24196_set_reg_rst(0x0);
	bq24196_set_wdt_rst(0x1); //Kick watchdog
//    if(upmu_get_cid() == 0x1020)
    if(0)
	    bq24196_set_sys_min(0x0); //Minimum system voltage 3.0V (MT6320 E1 workaround, disable powerpath)
    else	    
        bq24196_set_sys_min(0x5); //Minimum system voltage 3.5V		
	bq24196_set_iprechg(0x3); //Precharge current 512mA
	bq24196_set_iterm(0x0); //Termination current 128mA

#if defined(MTK_JEITA_STANDARD_SUPPORT)        
    if(g_temp_status == TEMP_NEG_10_TO_POS_0)
    {    
		bq24196_set_vreg(0x1F); //VREG 4.0V
    }
    else
    {
        if(g_temp_status == TEMP_POS_10_TO_POS_45)
		    bq24196_set_vreg(0x2C); //VREG 4.208V			
		else
		    bq24196_set_vreg(0x25); //VREG 4.096V
    }     
#else
    bq24196_set_vreg(0x2C); //VREG 4.208V
#endif    
    bq24196_set_batlowv(0x1); //BATLOWV 3.0V
    bq24196_set_vrechg(0x0); //VRECHG 0.1V (4.108V)
    bq24196_set_en_term(0x1); //Enable termination
    bq24196_set_term_stat(0x0); //Match ITERM
    bq24196_set_watchdog(0x1); //WDT 40s
    bq24196_set_en_timer(0x0); //Disable charge timer
    bq24196_set_int_mask(0x0); //Disable fault interrupt
}

void pchr_turn_off_charging_bq24196 (void)
{
	if (Enable_BATDRV_LOG == 1) {
		printf("[BATTERY:bq24196] pchr_turn_off_charging_bq24196!\r\n");
	}

	bq24196_set_wdt_rst(0x1); //Kick watchdog
	bq24196_set_chg_config(0x0);
	if (BMT_status.charger_protect_status == charger_OVER_VOL)
	{
	    printf("[BATTERY:bq24196] over voltage, enable high impedance mode!\r\n");
        bq24196_set_en_hiz(0x1);
    }
}

void pchr_turn_on_charging_bq24196 (void)
{
	if ( BMT_status.bat_charging_state == CHR_ERROR )
	{
		if (Enable_BATDRV_LOG == 1) {
			printf("[BATTERY:bq24196] Charger Error, turn OFF charging !\r\n");
		}
		BMT_status.total_charging_time = 0;
		pchr_turn_off_charging_bq24196();
	}
	else
	{
 		ChargerHwInit_bq24196();

		if (Enable_BATDRV_LOG == 1) {
			printf("[BATTERY:bq24196] pchr_turn_on_charging_bq24196 !\r\n");
		}

		select_charging_curret_bq24196();

		if( g_temp_CC_value == 0)
		{
		    pchr_turn_off_charging_bq24196();
		}
        else
        {
            bq24196_set_en_hiz(0x0);	        	
            bq24196_set_chg_config(0x1); // charger enable
            if (Enable_BATDRV_LOG == 1) {
               printf("[BATTERY:bq24196] charger enable !\r\n");
            }
        }
	}
}

int BAT_CheckPMUStatusReg(void)
{
    if( upmu_is_chr_det() == KAL_TRUE )
    {
        BMT_status.charger_exist = TRUE;
    }
    else
    {
        BMT_status.charger_exist = FALSE;

        BMT_status.total_charging_time = 0;
        BMT_status.PRE_charging_time = 0;
        BMT_status.CC_charging_time = 0;
        BMT_status.TOPOFF_charging_time = 0;
        BMT_status.POSTFULL_charging_time = 0;

        BMT_status.bat_charging_state = CHR_PRE;

        if (Enable_BATDRV_LOG == 1) {
            printf("[BATTERY] BAT_CheckPMUStatusReg : charger loss \n");
        }

        return PMU_STATUS_FAIL;
    }

    return PMU_STATUS_OK;
}

int g_Get_I_Charging(void)
{
    kal_int32 ADC_BAT_SENSE_tmp[20]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    kal_int32 ADC_BAT_SENSE_sum=0;
    kal_int32 ADC_BAT_SENSE=0;
    kal_int32 ADC_I_SENSE_tmp[20]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    kal_int32 ADC_I_SENSE_sum=0;
    kal_int32 ADC_I_SENSE=0;    
    int repeat=20;
    int i=0;
    int j=0;
    kal_int32 temp=0;
    int ICharging=0;    

    for(i=0 ; i<repeat ; i++)
    {
        ADC_BAT_SENSE_tmp[i] = get_bat_sense_volt_ch0(1);
        ADC_I_SENSE_tmp[i] = get_i_sense_volt_ch0(1);
    
        ADC_BAT_SENSE_sum += ADC_BAT_SENSE_tmp[i];
        ADC_I_SENSE_sum += ADC_I_SENSE_tmp[i];    
    }

    //sorting    BAT_SENSE 
    for(i=0 ; i<repeat ; i++)
    {
        for(j=i; j<repeat ; j++)
        {
            if( ADC_BAT_SENSE_tmp[j] < ADC_BAT_SENSE_tmp[i] )
            {
                temp = ADC_BAT_SENSE_tmp[j];
                ADC_BAT_SENSE_tmp[j] = ADC_BAT_SENSE_tmp[i];
                ADC_BAT_SENSE_tmp[i] = temp;
            }
        }
    }
    if (Enable_BATDRV_LOG == 1) {
        printf("[g_Get_I_Charging:BAT_SENSE]\r\n");    
        for(i=0 ; i<repeat ; i++ )
        {
            printf("%d,", ADC_BAT_SENSE_tmp[i]);
        }
        printf("\n");
    }

    //sorting    I_SENSE 
    for(i=0 ; i<repeat ; i++)
    {
        for(j=i ; j<repeat ; j++)
        {
            if( ADC_I_SENSE_tmp[j] < ADC_I_SENSE_tmp[i] )
            {
                temp = ADC_I_SENSE_tmp[j];
                ADC_I_SENSE_tmp[j] = ADC_I_SENSE_tmp[i];
                ADC_I_SENSE_tmp[i] = temp;
            }
        }
    }
    if (Enable_BATDRV_LOG == 1) {
        printf("[g_Get_I_Charging:I_SENSE]\r\n");    
        for(i=0 ; i<repeat ; i++ )
        {
            printf("%d,", ADC_I_SENSE_tmp[i]);
        }
        printf("\n");
    }
        
    ADC_BAT_SENSE_sum -= ADC_BAT_SENSE_tmp[0];
    ADC_BAT_SENSE_sum -= ADC_BAT_SENSE_tmp[1];
    ADC_BAT_SENSE_sum -= ADC_BAT_SENSE_tmp[18];
    ADC_BAT_SENSE_sum -= ADC_BAT_SENSE_tmp[19];        
    ADC_BAT_SENSE = ADC_BAT_SENSE_sum / (repeat-4);

    if (Enable_BATDRV_LOG == 1) {
        printf("[g_Get_I_Charging] ADC_BAT_SENSE=%d\n", ADC_BAT_SENSE);
    }

    ADC_I_SENSE_sum -= ADC_I_SENSE_tmp[0];
    ADC_I_SENSE_sum -= ADC_I_SENSE_tmp[1];
    ADC_I_SENSE_sum -= ADC_I_SENSE_tmp[18];
    ADC_I_SENSE_sum -= ADC_I_SENSE_tmp[19];
    ADC_I_SENSE = ADC_I_SENSE_sum / (repeat-4);

    if (Enable_BATDRV_LOG == 1) {
        printf("[g_Get_I_Charging] ADC_I_SENSE(Before)=%d\n", ADC_I_SENSE);
    }
    
    ADC_I_SENSE += gADC_I_SENSE_offset;

    if (Enable_BATDRV_LOG == 1) {
        printf("[g_Get_I_Charging] ADC_I_SENSE(After)=%d\n", ADC_I_SENSE);
    }

    BMT_status.ADC_BAT_SENSE = ADC_BAT_SENSE;
    BMT_status.ADC_I_SENSE = ADC_I_SENSE;
    
    if(ADC_I_SENSE > ADC_BAT_SENSE)
    {
        ICharging = (ADC_I_SENSE - ADC_BAT_SENSE)*10/R_CURRENT_SENSE;
    }
    else
    {
        ICharging = 0;
    }

    return ICharging;
}


UINT32 BattVoltToPercent(UINT16 dwVoltage)
{
    UINT32 m = 0;
    UINT32 VBAT1 = 0, VBAT2 = 0;
    UINT32 bPercntResult = 0, bPercnt1 = 0, bPercnt2 = 0;

    if (Enable_BATDRV_LOG == 1) {
        printf("###### 100 <-> voltage : %d ######\r\n", Batt_VoltToPercent_Table[10].BattVolt);
    }

    if(dwVoltage <= Batt_VoltToPercent_Table[0].BattVolt)
    {
        bPercntResult = Batt_VoltToPercent_Table[0].BattPercent;
        return bPercntResult;
    }
    else if (dwVoltage >= Batt_VoltToPercent_Table[10].BattVolt)
    {
        bPercntResult = Batt_VoltToPercent_Table[10].BattPercent;
        return bPercntResult;
    }
    else
    {
        VBAT1 = Batt_VoltToPercent_Table[0].BattVolt;
        bPercnt1 = Batt_VoltToPercent_Table[0].BattPercent;
        for(m = 1; m <= 10; m++)
        {
            if(dwVoltage <= Batt_VoltToPercent_Table[m].BattVolt)
            {
                VBAT2 = Batt_VoltToPercent_Table[m].BattVolt;
                bPercnt2 = Batt_VoltToPercent_Table[m].BattPercent;
                break;
            }
            else
            {
                VBAT1 = Batt_VoltToPercent_Table[m].BattVolt;
                bPercnt1 = Batt_VoltToPercent_Table[m].BattPercent;
            }
        }
    }

    bPercntResult = ( ((dwVoltage - VBAT1) * bPercnt2) + ((VBAT2 - dwVoltage) * bPercnt1) ) / (VBAT2 - VBAT1);

    return bPercntResult;
}

#if defined(MTK_JEITA_STANDARD_SUPPORT)
int do_jeita_state_machine(void)
{
    //JEITA battery temp Standard 
    if (BMT_status.temperature >= TEMP_POS_60_THRESHOLD) 
    {
        printf("[BATTERY] Battery Over high Temperature(%d) !!\n\r", 
            TEMP_POS_60_THRESHOLD);  
        
        g_temp_status = TEMP_ABOVE_POS_60;
        
        return PMU_STATUS_FAIL; 
    }
    else if(BMT_status.temperature > TEMP_POS_45_THRESHOLD)
    {

        if((g_temp_status == TEMP_ABOVE_POS_60) && (BMT_status.temperature >= TEMP_POS_60_THRES_MINUS_X_DEGREE))
        {
            printf("[BATTERY] Battery Temperature between %d and %d,not allow charging yet!!\n\r",
                TEMP_POS_60_THRES_MINUS_X_DEGREE,TEMP_POS_60_THRESHOLD); 
            
            return PMU_STATUS_FAIL; 
        }
        else
        {
            printf("[BATTERY] Battery Temperature between %d and %d !!\n\r",
                TEMP_POS_45_THRESHOLD,TEMP_POS_60_THRESHOLD); 
            
            g_temp_status = TEMP_POS_45_TO_POS_60;
            g_jeita_recharging_voltage = 3980; 
        }
    }
    else if(BMT_status.temperature >= TEMP_POS_10_THRESHOLD)
    {
        if( ((g_temp_status == TEMP_POS_45_TO_POS_60) && (BMT_status.temperature >= TEMP_POS_45_THRES_MINUS_X_DEGREE)) ||
            ((g_temp_status == TEMP_POS_0_TO_POS_10 ) && (BMT_status.temperature <= TEMP_POS_10_THRES_PLUS_X_DEGREE ))    )    
        {
            printf("[BATTERY] Battery Temperature not recovery to normal temperature charging mode yet!!\n\r");     
        }
        else
        {
            if(Enable_BATDRV_LOG >=1)
            {
                printf("[BATTERY] Battery Normal Temperature between %d and %d !!\n\r",TEMP_POS_10_THRESHOLD,TEMP_POS_45_THRESHOLD); 
            }

            g_temp_status = TEMP_POS_10_TO_POS_45;
            g_jeita_recharging_voltage = 4080;
        }
    }
    else if(BMT_status.temperature >= TEMP_POS_0_THRESHOLD)
    {
        if((g_temp_status == TEMP_NEG_10_TO_POS_0 || g_temp_status == TEMP_BELOW_NEG_10) && (BMT_status.temperature <= TEMP_POS_0_THRES_PLUS_X_DEGREE))
        {
            if (g_temp_status == TEMP_NEG_10_TO_POS_0) {
                printf("[BATTERY] Battery Temperature between %d and %d !!\n\r",
                TEMP_POS_0_THRES_PLUS_X_DEGREE,TEMP_POS_10_THRESHOLD); 
            }
            if (g_temp_status == TEMP_BELOW_NEG_10) {
                printf("[BATTERY] Battery Temperature between %d and %d,not allow charging yet!!\n\r",
                TEMP_POS_0_THRESHOLD,TEMP_POS_0_THRES_PLUS_X_DEGREE); 
                return PMU_STATUS_FAIL; 
            }
        }    
        else
        {
            printf("[BATTERY] Battery Temperature between %d and %d !!\n\r",
                TEMP_POS_0_THRESHOLD,TEMP_POS_10_THRESHOLD); 
            
            g_temp_status = TEMP_POS_0_TO_POS_10;
            g_jeita_recharging_voltage = 3980; 
        }
    }
    else if(BMT_status.temperature >= TEMP_NEG_10_THRESHOLD)
    {
        if((g_temp_status == TEMP_BELOW_NEG_10) && (BMT_status.temperature <= TEMP_NEG_10_THRES_PLUS_X_DEGREE))
        {
            printf("[BATTERY] Battery Temperature between %d and %d,not allow charging yet!!\n\r",
                TEMP_NEG_10_THRESHOLD,TEMP_NEG_10_THRES_PLUS_X_DEGREE); 
            
            return PMU_STATUS_FAIL; 
        }
        else
        {
            printf("[BATTERY] Battery Temperature between %d and %d !!\n\r",
                TEMP_NEG_10_THRESHOLD,TEMP_POS_0_THRESHOLD); 
            
            g_temp_status = TEMP_NEG_10_TO_POS_0;
            g_jeita_recharging_voltage = 3880;
        }
    }
    else
    {
        printf("[BATTERY] Battery below low Temperature(%d) !!\n\r", 
            TEMP_NEG_10_THRESHOLD);  
        
        g_temp_status = TEMP_BELOW_NEG_10;
        
        return PMU_STATUS_FAIL; 
    }
}
#endif

int g_lk_anime_on = 0;
void BAT_LkAnimationControl(kal_bool display)
{
    #define BATTERY_BAR 25

    if (low_bat_boot_display == 0)
    {
        /* LK charging animation */
        if(display == KAL_TRUE)
        {
            /* LK charging idle mode */
            if (!bl_switch) {
                mt_disp_power(TRUE);
                bl_switch_timer++;
                mt65xx_backlight_on();
                g_bl_on = 1;
            }
            if (bl_switch_timer > BL_SWITCH_TIMEOUT) {
                bl_switch = KAL_TRUE;
                bl_switch_timer = 0;
                mt65xx_backlight_off();
                mt_disp_power(FALSE);
                g_bl_on = 0;
                printf("[BATTERY:bq24196] mt65xx_backlight_off\r\n");
				return;
            }
			
            if ( (g_HW_Charging_Done == 1) || (user_view_flag == KAL_TRUE) )
            {
                if(g_bl_on == 1)
                {
                    mt_disp_show_battery_full();
                }
                user_view_flag = KAL_TRUE;
            }
            else
            {
                prog_temp = (bat_volt_check_point/BATTERY_BAR) * BATTERY_BAR;

                if (prog_first == 1)
                {
                    prog = prog_temp;
                    prog_first = 0;
                }
                if(g_bl_on == 1)
                {
                    mt_disp_show_battery_capacity(prog);
                }
                prog += BATTERY_BAR;
                if (prog > 100) prog = prog_temp;
            }
        }
        else
        {
            mt65xx_backlight_off();
            mt_disp_power(FALSE);
            printf("[BATTERY:bq24196] mt65xx_backlight_off due to (display == KAL_FALSE)\r\n");
        }
    }
    else
    {
        if(display == KAL_TRUE)
        {
            /* LK charging idle mode */
            if (!bl_switch) {
                mt_disp_power(TRUE);
                bl_switch_timer++;
                mt65xx_backlight_on();
                g_bl_on = 1;
            }
            if (bl_switch_timer > BL_SWITCH_TIMEOUT) {
                bl_switch = KAL_TRUE;
                bl_switch_timer = 0;
                mt65xx_backlight_off();
                mt_disp_power(FALSE);
                printf("[BATTERY:bq24196] mt65xx_backlight_off\r\n");
            }
        }
        else
        {
            mt65xx_backlight_off();
            mt_disp_power(FALSE);
            printf("[BATTERY:bq24196] mt65xx_backlight_off due to (display == KAL_FALSE)\r\n");
        }
    }
}

int BAT_CheckBatteryStatus_bq24196(void)
{
    long tmo;
#ifndef MTK_BQ27541_SUPPORT
    static int cnt = 9;
#else
    U32 returnData = 0;
    int ret = 0;
#endif
	int vol_batfet_disabled = 0;
    int i = 0;
    int bat_temperature_volt=0;
    /* Get Battery Information : start --------------------------------------------------------------------------*/

    /* Get V_BAT_SENSE */
    BMT_status.ADC_I_SENSE = get_i_sense_volt_ch0(5);
    BMT_status.bat_vol = BMT_status.ADC_I_SENSE;

    /* Get V_I_SENSE */
    BMT_status.ADC_BAT_SENSE = get_bat_sense_volt_ch0(5); //system voltage

    /* Get V_Charger */
    BMT_status.charger_vol = get_charger_volt(5);
    BMT_status.charger_vol = BMT_status.charger_vol / 100;

    /* Get V_BAT_Temperature */
    bat_temperature_volt = get_tbat_volt(5);
    if(bat_temperature_volt == 0)
    {
#ifdef BEFORE_TREF_REWORK	
	    if(upmu_get_cid() == 0x1020)
	        g_bat_temperature_pre = 22; // MT6320 E1 workaround
#endif	        
        BMT_status.temperature = g_bat_temperature_pre;
        if (Enable_BATDRV_LOG == 1) {
            printf("[BATTERY:bq24196] Warning !! bat_temperature_volt == 0, restore temperature value\n\r");
        }
    }
    else
    {
        BMT_status.temperature = BattVoltToTemp(bat_temperature_volt);
        g_bat_temperature_pre = BMT_status.temperature;
    }

#ifdef MTK_BQ27541_SUPPORT
    ret = bq27541_set_cmd_read(BQ27541_CMD_Temperature, &returnData);
    if(ret != I2C_OK)
    {
        printf("[BATTERY:ADC:bq27541] bq27541 i2c access error, use MT6320 measured battery temperature\n");
    }	
    else
    {
        printf("[BATTERY:ADC:bq27541] battery temperature read raw data(%d)0.1K\n", returnData); 
        BMT_status.temperature = ((returnData/10) - 273);
    }
#endif

    if (Enable_BATDRV_LOG == 1) {
        printf("[BATTERY:ADC:bq24196] VCHR:%d BAT_SENSE:%d I_SENSE:%d TBAT:%d TBAT_vol:%d\n",
        BMT_status.charger_vol, BMT_status.ADC_BAT_SENSE, BMT_status.ADC_I_SENSE, BMT_status.temperature, bat_temperature_volt);
    }

    BMT_status.ICharging = 0; //cannot measure adaptor charing current when using power path

    g_BatteryAverageCurrent = BMT_status.ICharging;
    /* Get Battery Information : end --------------------------------------------------------------------------*/

    /* Re-calculate Battery Percentage (SOC) */
#ifndef MTK_BQ27541_SUPPORT	
	cnt++;
	if(cnt >= 10) //update SOC every 10 seconds
	{
	    cnt = 0;
//		bq24196_set_batfet_disable(0x1);
        bq24196_set_chg_config(0x0);
        tmo = get_timer(0);
        while(get_timer(tmo) <= 50 /* ms */);
		vol_batfet_disabled = get_i_sense_volt_ch0(5);
        BMT_status.SOC = BattVoltToPercent(vol_batfet_disabled);
//		bq24196_set_batfet_disable(0x0);		
        bq24196_set_chg_config(0x1);
        printf("[BATTERY:bq24196] vol_batfet_disabled = %d\n", vol_batfet_disabled);
	}
#else
    ret = bq27541_set_cmd_read(BQ27541_CMD_StateOfCharge,&returnData);
    if(ret != I2C_OK)
    {
        BMT_status.SOC = BattVoltToPercent(vol_batfet_disabled);    
	    printf("[BATTERY:bq27541] bq27541 i2c access error\n");
    }
	else
	{
        BMT_status.SOC = (INT32)returnData;	
        printf("[BATTERY:bq27541] bq27541 read SOC raw data = %d\n", returnData);		    
	}
#endif

    /* User smooth View when discharging : start */
    if (bat_volt_cp_flag == 0)
    {
        bat_volt_cp_flag = 1;
        bat_volt_check_point = BMT_status.SOC;
    }
    /* User smooth View when discharging : end */

    /**************** Averaging : START ****************/
    if (!batteryBufferFirst)
    {
        batteryBufferFirst = KAL_TRUE;

        for (i=0; i<BATTERY_AVERAGE_SIZE; i++) {
            batteryVoltageBuffer[i] = BMT_status.bat_vol;
            batteryCurrentBuffer[i] = BMT_status.ICharging;
#ifndef MTK_BQ27541_SUPPORT				
            batterySOCBuffer[i] = BMT_status.SOC;
#endif
        }

        batteryVoltageSum = BMT_status.bat_vol * BATTERY_AVERAGE_SIZE;
        batteryCurrentSum = BMT_status.ICharging * BATTERY_AVERAGE_SIZE;
#ifndef MTK_BQ27541_SUPPORT			
        batterySOCSum = BMT_status.SOC * BATTERY_AVERAGE_SIZE;
#endif
    }

    batteryVoltageSum -= batteryVoltageBuffer[batteryIndex];
    batteryVoltageSum += BMT_status.bat_vol;
    batteryVoltageBuffer[batteryIndex] = BMT_status.bat_vol;

    batteryCurrentSum -= batteryCurrentBuffer[batteryIndex];
    batteryCurrentSum += BMT_status.ICharging;
    batteryCurrentBuffer[batteryIndex] = BMT_status.ICharging;

    if (BMT_status.bat_full)
        BMT_status.SOC = 100;
    if (g_bat_full_user_view)
        BMT_status.SOC = 100;

#ifndef MTK_BQ27541_SUPPORT	
    batterySOCSum -= batterySOCBuffer[batteryIndex];
    batterySOCSum += BMT_status.SOC;
    batterySOCBuffer[batteryIndex] = BMT_status.SOC;
#endif

    BMT_status.bat_vol = batteryVoltageSum / BATTERY_AVERAGE_SIZE;
    BMT_status.ICharging = batteryCurrentSum / BATTERY_AVERAGE_SIZE;
#ifndef MTK_BQ27541_SUPPORT		
    BMT_status.SOC = batterySOCSum / BATTERY_AVERAGE_SIZE;
#endif

    batteryIndex++;
    if (batteryIndex >= BATTERY_AVERAGE_SIZE)
        batteryIndex = 0;
    /**************** Averaging : END ****************/

    if( BMT_status.SOC == 100 ) {
        BMT_status.bat_full = KAL_TRUE;
    }

    /**************** For LK : Start ****************/

    if(low_bat_boot_display == 0)
    {
        /* SOC only UP when charging */
        if ( BMT_status.SOC > bat_volt_check_point ) {
            bat_volt_check_point = BMT_status.SOC;
        }
           	
        /* LK charging LED */
        if ( (bat_volt_check_point >= 90)  || (user_view_flag == KAL_TRUE) ) {
            leds_battery_full_charging();
        } else if(bat_volt_check_point <= 10) {
            leds_battery_low_charging();
        } else {
            leds_battery_medium_charging();
        }
    }

    /**************** For LK : End ****************/

    //if (Enable_BATDRV_LOG == 1) {
    printf("[BATTERY:AVG:bq24196(%d,%dmA)] BatTemp:%d Vbat:%d VBatSen:%d SOC:%d ChrDet:%d Vchrin:%d Icharging:%d(%d) ChrType:%d \r\n",
    BATTERY_AVERAGE_SIZE, CHARGING_FULL_CURRENT, BMT_status.temperature ,BMT_status.bat_vol, BMT_status.ADC_BAT_SENSE, BMT_status.SOC,
    upmu_is_chr_det(), BMT_status.charger_vol, BMT_status.ICharging, g_BatteryAverageCurrent, CHR_Type_num );
    //}

	/* Protection Check : start*/
#if defined(MTK_JEITA_STANDARD_SUPPORT)
    if (Enable_BATDRV_LOG == 1) {
        printf("[BATTERY:bq24196] support JEITA, Tbat=%d\n", BMT_status.temperature);			 
    }
	
    if( do_jeita_state_machine() == PMU_STATUS_FAIL)
    {
        printf("[BATTERY] JEITA : fail\n");
        BMT_status.bat_charging_state = CHR_ERROR;  
        return PMU_STATUS_FAIL;
    }
#else
#if (BAT_TEMP_PROTECT_ENABLE == 1)
    if ((BMT_status.temperature < MIN_CHARGE_TEMPERATURE) || 
        (BMT_status.temperature == ERR_CHARGE_TEMPERATURE))
    {
        printf(  "[BATTERY:bq24196] Battery Under Temperature or NTC fail !!\n\r"); 			   
        BMT_status.bat_charging_state = CHR_ERROR;
        return PMU_STATUS_FAIL; 	  
    }
#endif                
    if (BMT_status.temperature >= MAX_CHARGE_TEMPERATURE)
    {
        printf(  "[BATTERY:bq24196] Battery Over Temperature !!\n\r");				  
        BMT_status.bat_charging_state = CHR_ERROR;
        return PMU_STATUS_FAIL; 	  
    }
#endif    

    if( upmu_is_chr_det() == KAL_TRUE)
    {
        #if (V_CHARGER_ENABLE == 1)
        if (BMT_status.charger_vol <= V_CHARGER_MIN )
        {
            printf(  "[BATTERY:bq24196]Charger under voltage!!\r\n");
            BMT_status.bat_charging_state = CHR_ERROR;
            return PMU_STATUS_FAIL;
        }
        #endif
        if ( BMT_status.charger_vol >= V_CHARGER_MAX )
        {
            printf(  "[BATTERY:bq24196]Charger over voltage !!\r\n");
            BMT_status.charger_protect_status = charger_OVER_VOL;
            BMT_status.bat_charging_state = CHR_ERROR;
            return PMU_STATUS_FAIL;
        }
    }
    /* Protection Check : end*/

#if 1
    if( upmu_is_chr_det() == KAL_TRUE)
    {
#if defined(MTK_JEITA_STANDARD_SUPPORT)
        if ((BMT_status.bat_vol < g_jeita_recharging_voltage) && (BMT_status.bat_full) && (g_HW_Charging_Done == 1) && (!g_Battery_Fail))
#else    
        if ((BMT_status.bat_vol < RECHARGING_VOLTAGE) && (BMT_status.bat_full) && (g_HW_Charging_Done == 1) && (!g_Battery_Fail))    
#endif       
        {
            if (Enable_BATDRV_LOG == 1) {
            	printf("[BATTERY:bq24196] Battery Re-charging !!\n\r");
            }
            BMT_status.bat_full = KAL_FALSE;
            g_bat_full_user_view = KAL_TRUE;
            BMT_status.bat_charging_state = CHR_CC;

            g_HW_Charging_Done = 0;
        }
    }
#endif

    //Reset error status if no error condition detected this turn
    BMT_status.charger_protect_status = 0;
    BMT_status.bat_charging_state = CHR_CC;

    return PMU_STATUS_OK;
}

PMU_STATUS BAT_BatteryStatusFailAction(void)
{
    if (Enable_BATDRV_LOG == 1) {
    	printf("[BATTERY:bq24196] BAD Battery status... Charging Stop !!\n\r");
    }

    BMT_status.total_charging_time = 0;
    BMT_status.PRE_charging_time = 0;
    BMT_status.CC_charging_time = 0;
    BMT_status.TOPOFF_charging_time = 0;
    BMT_status.POSTFULL_charging_time = 0;

    /*  Disable charger */
    pchr_turn_off_charging_bq24196();

    return PMU_STATUS_OK;
}

PMU_STATUS BAT_ChargingOTAction(void)
{
    printf("[BATTERY:bq24196] Charging over %d hr stop !!\n\r", MAX_CHARGING_TIME);

    //BMT_status.bat_full = KAL_TRUE;
    BMT_status.total_charging_time = 0;
    BMT_status.PRE_charging_time = 0;
    BMT_status.CC_charging_time = 0;
    BMT_status.TOPOFF_charging_time = 0;
    BMT_status.POSTFULL_charging_time = 0;

	g_HW_Charging_Done = 1;
	g_Charging_Over_Time = 1;

    /*  Disable charger*/
    pchr_turn_off_charging_bq24196();

    return PMU_STATUS_OK;
}

PMU_STATUS BAT_BatteryFullAction(void)
{
    if (Enable_BATDRV_LOG == 1) {    
        printf("[BATTERY:bq24196] check Battery full !!\n\r");            
    }
    
    BMT_status.bat_full = KAL_TRUE;
    BMT_status.total_charging_time = 0;
    BMT_status.PRE_charging_time = 0;
    BMT_status.CC_charging_time = 0;
    BMT_status.TOPOFF_charging_time = 0;
    BMT_status.POSTFULL_charging_time = 0;
    
    g_HW_Charging_Done = 1;
    
    /*  Disable charger */
    pchr_turn_off_charging_bq24196();
        
    return PMU_STATUS_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////
//// bq24196
///////////////////////////////////////////////////////////////////////////////////////////
void pmic_init_for_bq24196(void)
{
	printf("[pmic_init_for_bq24196] Start\n");

    upmu_set_rg_chrwdt_td(0x0);           // CHRWDT_TD, 4s
    upmu_set_rg_chrwdt_int_en(0);         // CHRWDT_INT_EN
    upmu_set_rg_chrwdt_en(0);             // CHRWDT_EN
    upmu_set_rg_chrwdt_wr(0);             // CHRWDT_WR

    upmu_set_rg_csdac_mode(0);      //CSDAC_MODE

    upmu_set_rg_vcdt_mode(0);       //VCDT_MODE
    upmu_set_rg_vcdt_hv_en(1);      //VCDT_HV_EN

    upmu_set_rg_bc11_bb_ctrl(1);    //BC11_BB_CTRL
    upmu_set_rg_bc11_rst(1);        //BC11_RST
    
    upmu_set_rg_csdac_mode(0);      //CSDAC_MODE
    upmu_set_rg_vbat_ov_en(0);      //VBAT_OV_EN
    upmu_set_rg_vbat_ov_vth(0x0);   //VBAT_OV_VTH, 4.3V    
    upmu_set_rg_baton_en(1);        //BATON_EN

    //Tim, for TBAT
    upmu_set_rg_buf_pwd_b(1);       //RG_BUF_PWD_B
    upmu_set_rg_baton_ht_en(0);     //BATON_HT_EN
    
    upmu_set_rg_ulc_det_en(0);      // RG_ULC_DET_EN=1
    upmu_set_rg_low_ich_db(1);      // RG_LOW_ICH_DB=000001'b

    upmu_set_rg_chr_en(0);           // CHR_EN
    upmu_set_rg_hwcv_en(0);          // RG_HWCV_EN

	printf("[pmic_init_for_bq24196] Done\n");
}

void bmt_charger_ov_check(void)
{
    long tmo;

     if(get_charger_hv_status() == 1)
     {
         pchr_turn_off_charging_bq24196();
         printf("[bmt_charger_ov_check]LK charger ov, turn off charging\r\n");
         while(1)             
         {  
             printf("[bmt_charger_ov_check] mtk_wdt_restart()\n");
             mtk_wdt_restart();
             
             if(charger_ov_boot_display == 0)
             {
                mt_disp_power(TRUE);
                mt_disp_show_charger_ov_logo(); 
                mt_disp_wait_idle();
                charger_ov_boot_display = 1;
                printf("LK charger ov, Set low brightness\r\n");
                mt65xx_leds_brightness_set(6, 20);
             }
             BMT_status.charger_vol = get_charger_volt(5);
             BMT_status.charger_vol = BMT_status.charger_vol / 100;
             if (BMT_status.charger_vol < 4000) //charger out detection        
             {             
                 #ifndef NO_POWER_OFF                
                 mt6575_power_off();              
                 #endif             
                 while(1);            
             } 

             tmo = get_timer(0);              
             while(get_timer(tmo) <= 500 /* ms */);            
         }
    }    
}

void BAT_thread_bq24196(void)
{
    long tmo;
    int BAT_status = 0;
    kal_uint32 bq24196_stat=0;
    int i = 0;
    int chr_err_cnt = 0;

    printf("[BAT_thread_bq24196] mtk_wdt_restart()\n");
    mtk_wdt_restart();

    if (Enable_BATDRV_LOG == 1) {
        printf("[BAT_thread_bq24196] LOG. %d,%d----------------------------\n", BATTERY_AVERAGE_SIZE, RECHARGING_VOLTAGE);
    }    

    /* If charger does not exist */
//    if(get_charger_hv_status() == 1 || (upmu_is_chr_det() == KAL_FALSE))
    if(upmu_is_chr_det() == KAL_FALSE)
    {
        bmt_charger_ov_check();
        BMT_status.charger_type = CHARGER_UNKNOWN;
        BMT_status.bat_full = KAL_FALSE;
        g_bat_full_user_view = KAL_FALSE;
        g_usb_state = USB_UNCONFIGURED;

        g_HW_Charging_Done = 0;
        g_Charging_Over_Time = 0;

        printf("[BATTERY:bq24196] No Charger, Power OFF !?\n");
        pchr_turn_off_charging_bq24196();

        printf("[BATTERY:bq24196] mt_power_off !!\n");
	#ifndef NO_POWER_OFF
        mt6575_power_off();
	#endif
        while(1);
    }

    /* Check Battery Status */
    BAT_status = BAT_CheckBatteryStatus_bq24196();

    if(BMT_status.bat_charging_state == CHR_ERROR)
    {
        chr_err_cnt = 600; //10mins debounce
    }
    else if(chr_err_cnt > 0)
    {
        chr_err_cnt --;
    }
	
    if( BAT_status == PMU_STATUS_FAIL )
        g_Battery_Fail = KAL_TRUE;
    else if( chr_err_cnt > 0 )
        g_Battery_Fail = KAL_TRUE;
    else
        g_Battery_Fail = KAL_FALSE;

    /* No Charger */
    if(BAT_status == PMU_STATUS_FAIL || g_Battery_Fail)
    {
        BMT_status.total_charging_time = 0;
        BMT_status.PRE_charging_time = 0;
        BMT_status.CC_charging_time = 0;
        BMT_status.TOPOFF_charging_time = 0;
        BMT_status.POSTFULL_charging_time = 0;

        pchr_turn_off_charging_bq24196();

        if (Enable_BATDRV_LOG == 1)
        {
            printf("[BATTERY:bq24196] PMU_STATUS_FAIL (chr_err_cnt = %d)\n", chr_err_cnt);
        }
    }
    /* Battery Full */
    /* HW charging done, real stop charging */
    else if (g_HW_Charging_Done == 1)
    {
        if (Enable_BATDRV_LOG == 1)
        {
            printf("[BATTERY:bq24196] Battery real full. \n");
        }
        BMT_status.bat_full = KAL_TRUE;
        BMT_status.total_charging_time = 0;
        BMT_status.PRE_charging_time = 0;
        BMT_status.CC_charging_time = 0;
        BMT_status.TOPOFF_charging_time = 0;
        BMT_status.POSTFULL_charging_time = 0;
        g_HW_Charging_Done = 1;
        pchr_turn_off_charging_bq24196();		
    }

    /* Charging Overtime, can not charging (not for total charging timer) */
    else if (g_Charging_Over_Time == 1)
    {
        if (Enable_BATDRV_LOG == 1)
        {
            printf("[BATTERY:bq24196] Charging Over Time. \n");
        }
        pchr_turn_off_charging_bq24196();
    }

    /* Battery Not Full and Charger exist : Do Charging */
    else
    {
        if (Enable_BATDRV_LOG == 1)
        {
            printf("[BATTERY:bq24196] state=%d, \n", BMT_status.bat_charging_state);
        }

        /* Charging OT */
        if(BMT_status.total_charging_time >= MAX_CHARGING_TIME)
        {
            BMT_status.total_charging_time = 0;
            BMT_status.PRE_charging_time = 0;
            BMT_status.CC_charging_time = 0;
            BMT_status.TOPOFF_charging_time = 0;
            BMT_status.POSTFULL_charging_time = 0;
            g_HW_Charging_Done = 1;
	        g_Charging_Over_Time = 1;
            pchr_turn_off_charging_bq24196();
            return;
        }

        bq24196_stat = bq24196_get_chrg_stat();

        /* check battery full */
#if defined(MTK_JEITA_STANDARD_SUPPORT)
        if((BMT_status.bat_vol > g_jeita_recharging_voltage) && (bq24196_stat == 0x3))
#else
        if((BMT_status.bat_vol > RECHARGING_VOLTAGE) && (bq24196_stat == 0x3))
#endif
        {
            BMT_status.bat_charging_state = CHR_BATFULL;
            BMT_status.bat_full = KAL_TRUE;
            BMT_status.total_charging_time = 0;
            BMT_status.PRE_charging_time = 0;
            BMT_status.CC_charging_time = 0;
            BMT_status.TOPOFF_charging_time = 0;
            BMT_status.POSTFULL_charging_time = 0;
            g_HW_Charging_Done = 1;
            pchr_turn_off_charging_bq24196();
            printf("[BATTERY:bq24196] Battery real full and disable charging (%d) \n", bq24196_stat);
            return;
        }
        else if(bq24196_stat == 0x3) //false alarm, reset status
        {
            pchr_turn_off_charging_bq24196();
            tmo = get_timer(0);
            while(get_timer(tmo) <= 100 /* ms */);
            if (Enable_BATDRV_LOG == 1) {
                printf("[BATTERY:bq24196] bq24196_status is (%d), but battery voltage not over recharge voltage\n", bq24196_stat);
            }
        }

        /* Charging flow begin */
        BMT_status.total_charging_time += BAT_TASK_PERIOD;
        BMT_status.bat_charging_state = CHR_CC;
        pchr_turn_on_charging_bq24196();

        if (Enable_BATDRV_LOG == 1)
        {
            printf(  "[BATTERY:bq24196] Total charging timer=%d, bq24196_stat=%d \n\r",
            BMT_status.total_charging_time, bq24196_stat);
        }
    }

    if (Enable_BATDRV_LOG == 1)
    {
        bq24196_dump_register();
    }

    if (Enable_BATDRV_LOG == 1)
    {
        printf("[BAT_thread_bq24196] End ....\n");
    }
}

void batdrv_init(void)
{
    int i = 0;
    /* Initialization BMT Struct */
    for (i=0; i<BATTERY_AVERAGE_SIZE; i++) {
        batteryCurrentBuffer[i] = 0;
        batteryVoltageBuffer[i] = 0;
#ifndef MTK_BQ27541_SUPPORT		
        batterySOCBuffer[i] = 0;
#endif
    }
    batteryVoltageSum = 0;
    batteryCurrentSum = 0;
#ifndef MTK_BQ27541_SUPPORT			
    batterySOCSum = 0;
#endif
    BMT_status.bat_exist = 1;       /* phone must have battery */
    BMT_status.charger_exist = 0;   /* for default, no charger */
    BMT_status.bat_vol = 0;
    BMT_status.ICharging = 0;
    BMT_status.temperature = 0;
    BMT_status.charger_vol = 0;
    //BMT_status.total_charging_time = 0;
    BMT_status.total_charging_time = 1;
    BMT_status.PRE_charging_time = 0;
    BMT_status.CC_charging_time = 0;
    BMT_status.TOPOFF_charging_time = 0;
    BMT_status.POSTFULL_charging_time = 0;

    BMT_status.bat_charging_state = CHR_PRE;

    if ((upmu_is_chr_det() == KAL_TRUE))
    {
        printf("bq24196 hw init\n");
        bq24196_dump_register();
        pmic_init_for_bq24196();
        pchr_turn_on_charging_bq24196();
    }

    upmu_set_rg_vcdt_hv_vth(0xD);    //VCDT_HV_VTH, 8.5V
    upmu_set_rg_vcdt_hv_en(1);       //VCDT_HV_EN

    printf("[BATTERY:bq24196] batdrv_init : Done\n");
}

void check_point_sync_leds(void)
{
    int battery_level = BattVoltToPercent(BMT_status.bat_vol);
    printf("[BATTERY] %s  battery_level = %d \n", __func__, battery_level);
    
    if(battery_level >= 90)   //Full ARGB
    {
        leds_battery_full_charging();
    }
    else  //Low and Medium ARGB
    {
        leds_battery_medium_charging();
    }
}

extern bool g_boot_menu;

void mt65xx_bat_init(void)
{
    long tmo;
    long tmo2;
    BOOL print_msg = FALSE;
    int press_pwrkey_count=0, loop_count = 0;	
    BOOL pwrkey_ready = false;
    BOOL back_to_charging_animation_flag = false;

#if 0
    #if (CHARGING_PICTURE == 1)
    mt_disp_enter_charging_state();
    #else
    mt_disp_show_boot_logo();
    #endif
#endif

    sc_mod_init();
    batdrv_init();

    BMT_status.bat_full = FALSE;
    BMT_status.bat_vol = get_i_sense_volt_ch0(5);
#if defined(MTK_JEITA_STANDARD_SUPPORT)
    if((BMT_status.bat_vol > g_jeita_recharging_voltage) && (upmu_is_chr_det() == KAL_TRUE))
#else		
    if((BMT_status.bat_vol > RECHARGING_VOLTAGE) && (upmu_is_chr_det() == KAL_TRUE))
#endif		
    {
        if(Enable_BATDRV_LOG == 1)
        {
#if defined(MTK_JEITA_STANDARD_SUPPORT)        
            printf("battery voltage during charging (%d) > %d\n", BMT_status.bat_vol, g_jeita_recharging_voltage);
#else
            printf("battery voltage during charging (%d) > %d\n", BMT_status.bat_vol, RECHARGING_VOLTAGE);
#endif
        }
        pchr_turn_off_charging_bq24196(); //turn off charging at first to prevent initial voltage measure reach 4.2 when pushing reset button and adaptor plugin
        BMT_status.bat_vol = get_i_sense_volt_ch0(5);
    }

#if defined(MTK_JEITA_STANDARD_SUPPORT)
    if ( BMT_status.bat_vol > g_jeita_recharging_voltage )
#else		
    if ( BMT_status.bat_vol > RECHARGING_VOLTAGE )
#endif	
    {
        user_view_flag = KAL_TRUE;
    } else {
        user_view_flag = KAL_FALSE;
    }

	if (pmic_detect_powerkey()) 	 
		pwrkey_ready = true;
	else
		pwrkey_ready = false;

    /* Boot with Charger */
    if ((upmu_is_chr_det() == KAL_TRUE))
    {
		CHR_Type_num = mt_charger_type_detection();
        BMT_status.charger_type = CHR_Type_num;
		//BMT_status.charger_type = NONSTANDARD_CHARGER;

        pchr_turn_on_charging_bq24196(); //turn on charging for powerpath

        while (1)
        {
            kick_charger_wdt();               
            upmu_set_rg_vcdt_hv_en(1);      //VCDT_HV_EN

            //add charger ov detection
            bmt_charger_ov_check();
			
            if (rtc_boot_check(true) || meta_mode_check() || (pwrkey_ready == true) 
            	|| mtk_wdt_boot_check()==WDT_BY_PASS_PWK_REBOOT || g_boot_arg->boot_reason==BR_TOOL_BY_PASS_PWK || g_boot_menu==true || g_boot_mode == FASTBOOT)
            {
                // Low Battery Safety Booting
                BMT_status.bat_vol = get_i_sense_volt_ch0(1); //we do not turn off charging because of power path support, the battery voltage may under UVLO
                printf("check VBAT during charging =%d mV with %d mV\n", BMT_status.bat_vol, BATTERY_LOWVOL_THRESOLD);
                while ( BMT_status.bat_vol < BATTERY_LOWVOL_THRESOLD )
                {
                    if(BMT_status.charger_type != STANDARD_HOST)
                        break;
					
                    if (low_bat_boot_display == 0)
                    {
                        mt_disp_power(TRUE);
                        mt65xx_backlight_off();
                        printf("Before mt6516_disp_show_low_battery\r\n");
                        mt_disp_show_low_battery();
                        printf("After mt6516_disp_show_low_battery\r\n");
                        mt_disp_wait_idle();
                        printf("After mt6516_disp_wait_idle\r\n");
                        bl_switch = KAL_FALSE;
                        bl_switch_timer = 0;
                        low_bat_boot_display = 1;

                        printf("Set low brightness\r\n");
                        mt65xx_leds_brightness_set(6, 20);
                    }

                    rtc_boot_check(false);
                    BAT_thread_bq24196();
                    printf("-");

                    tmo2 = get_timer(0);
                    while(get_timer(tmo2) <= 1000 /* ms */);

                    if((pwrkey_ready ==true) & pmic_detect_powerkey()==0 )
                    {
                        back_to_charging_animation_flag = TRUE;
                        break;
                    }
                    else
                    {
                        back_to_charging_animation_flag = false;
                    }

                    BMT_status.bat_vol = get_i_sense_volt_ch0(1); //we do not turn off charging because of power path support, the battery voltage may under UVLO
                    if(BMT_status.bat_vol < BATTERY_LOWVOL_THRESOLD)
                        printf("VBAT=%d < %d\n", BMT_status.bat_vol, BATTERY_LOWVOL_THRESOLD);
                }

                if(back_to_charging_animation_flag == false)
                {
                    mt_disp_power(TRUE);
    
                    if (g_boot_mode != ALARM_BOOT)
                    {
                        mt_disp_show_boot_logo();
                        
                        // update twice here to ensure the LCM is ready to show the
                        // boot logo before turn on backlight, OR user may glimpse
                        // at the previous battery charging screen
                        mt_disp_show_boot_logo();
                        mt_disp_wait_idle();
                    }
                    else
                    {
                        printf("Power off alarm trigger! Boot Linux Kernel!!\n\r");
                        
                        // fill the screen with a whole black image
                        mt_disp_fill_rect(0, 0, CFG_DISPLAY_WIDTH, CFG_DISPLAY_HEIGHT, 0x0);
                        mt_disp_update(0, 0, CFG_DISPLAY_WIDTH, CFG_DISPLAY_HEIGHT);
                        mt_disp_wait_idle();
                    }
    
                    printf("Restore brightness\r\n");
                    mt65xx_leds_brightness_set(6, 255);
                    check_point_sync_leds();					
                    mt65xx_backlight_on();

                    pchr_turn_on_charging_bq24196(); //turn on charging for powerpath
                
                    sc_mod_exit();
                    return;
                }
                back_to_charging_animation_flag = false;
                low_bat_boot_display = 0;                
            }
            else
            {
                //printf("[BATTERY] U-BOOT Charging !! \n\r");
            }

            if (g_thread_count >= 5)
            {
                g_thread_count = 1;
                BAT_thread_bq24196();
                printf(".");
            }
            else
            {
                g_thread_count++;
            }

            if(print_msg==FALSE)
            {
                if((BMT_status.bat_vol < BATTERY_LOWVOL_THRESOLD) && ((BMT_status.total_charging_time < 2) || (BMT_status.charger_type == STANDARD_HOST)) && (g_lk_anime_on != 1))
                {
                    BAT_LkAnimationControl(KAL_FALSE);
                }
                else
                {
                    g_lk_anime_on = 1;
                    BAT_LkAnimationControl(KAL_TRUE);
                }            
                printf("Charging !! Press Power Key to Booting !!! \n\r");
                print_msg = TRUE;
            }

            tmo = get_timer(0);
            while(get_timer(tmo) <= 200 /* ms */);

            if (loop_count++ == 60) loop_count = 0;
        			
            if (mtk_detect_key(BACKLIGHT_KEY) || (!pmic_detect_powerkey() && press_pwrkey_count > 0))
            {
                bl_switch = KAL_FALSE;
                bl_switch_timer = 0;
                g_bl_on = 1;
                printf("mt65xx_backlight_on\r\n");
            }
            			
            if (pmic_detect_powerkey())
            { 
                press_pwrkey_count++;
                printf("press_pwrkey_count = %d, POWER_ON_TIME = %d\n", press_pwrkey_count, POWER_ON_TIME);
            }
            else
            { 
                press_pwrkey_count = 0;
            }
            				 
            if (press_pwrkey_count > POWER_ON_TIME) 
                pwrkey_ready = true;
            else
                pwrkey_ready = false;
            				            				
            if (((loop_count % 5) == 0) && bl_switch == false) // update charging screen every 1s (200ms * 5)
            {
                if (Enable_BATDRV_LOG == 1)
                {
                    printf("loop_count = %d\n", loop_count);
                }
                if((BMT_status.bat_vol < BATTERY_LOWVOL_THRESOLD) && ((BMT_status.total_charging_time < 2) || (BMT_status.charger_type == STANDARD_HOST)) && (g_lk_anime_on != 1))
                {
                    BAT_LkAnimationControl(KAL_FALSE);
                }
                else
                {
                    g_lk_anime_on = 1;
                    BAT_LkAnimationControl(KAL_TRUE);
                }            
            }  
        }
    }
    else
    {
        bmt_charger_ov_check();
        upmu_set_rg_chrind_on(0);  //We must turn off HW Led Power.

        //if (BMT_status.bat_vol >= BATTERY_LOWVOL_THRESOLD)        
        if ( (rtc_boot_check(false)||mtk_wdt_boot_check()==WDT_BY_PASS_PWK_REBOOT) && BMT_status.bat_vol >= BATTERY_LOWVOL_THRESOLD)
        {
            printf("battery voltage(%dmV) >= CLV ! Boot Linux Kernel !! \n\r",BMT_status.bat_vol);
            mt_disp_power(TRUE);
    
            if (g_boot_mode != ALARM_BOOT)
            {
                mt_disp_show_boot_logo();
                        
                // update twice here to ensure the LCM is ready to show the
                // boot logo before turn on backlight, OR user may glimpse
                // at the previous battery charging screen
                mt_disp_show_boot_logo();
                mt_disp_wait_idle();
            }
            else
            {
                printf("Power off alarm trigger! Boot Linux Kernel!!\n\r");
                        
                // fill the screen with a whole black image
                mt_disp_fill_rect(0, 0, CFG_DISPLAY_WIDTH, CFG_DISPLAY_HEIGHT, 0x0);
                mt_disp_update(0, 0, CFG_DISPLAY_WIDTH, CFG_DISPLAY_HEIGHT);
                mt_disp_wait_idle();
            }
    
            printf("Restore brightness\r\n");
            mt65xx_leds_brightness_set(6, 255);
            check_point_sync_leds();					
            mt65xx_backlight_on();            
            sc_mod_exit();
            return;
        }
        else
        {
            printf("battery voltage(%dmV) <= CLV ! Can not Boot Linux Kernel !! \n\r",BMT_status.bat_vol);
            pchr_turn_off_charging_bq24196();
            #ifndef NO_POWER_OFF
            mt6575_power_off();
            #endif
            while(1)
            {
            	printf("If you see the log, please check with RTC power off API\n\r");
            }
        }
    }

    sc_mod_exit();
    return;
}

#else

#include <platform/mt_typedefs.h>
#include <platform/mt_reg_base.h>
#include <printf.h>

void mt65xx_bat_init(void)
{
    printf("[BATTERY] Skip mt65xx_bat_init !!\n\r");
    printf("[BATTERY] If you want to enable power off charging, \n\r");
    printf("[BATTERY] Please #define CFG_POWER_CHARGING!!\n\r");
}

#endif
