#include <target/board.h>

//#define CFG_POWER_CHARGING

#ifdef CFG_POWER_CHARGING
#include <platform/mt_typedefs.h>
#include <platform/mt_reg_base.h>
#include <platform/mt_pmic.h>
#include <platform/boot_mode.h>
#include <platform/mt_gpt.h>
#include <platform/mt_sleep.h>
#include <platform/mt_rtc.h>
#include <platform/mt_disp_drv.h>
#include <platform/mtk_wdt.h>
#include <platform/mtk_key.h>
#include <platform/mt_logo.h>
#include <platform/mt_leds.h>
#include <platform/mt_gpio.h>
#include <platform/fan5405.h>
#include <printf.h>
#include <sys/types.h>
#include <target/cust_battery.h>
#include <cust_gpio_usage.h>

#undef printf

//#define CONFIG_DEBUG_MSG
#define GPT_TIMER // when sleep driver ready, open this define

#if defined(HIGH_BATTERY_VOLTAGE_SUPPORT)
int g_enable_high_vbat_spec = 1;
#else
int g_enable_high_vbat_spec = 0;    
#endif

#ifdef MTK_KERNEL_POWER_OFF_CHARGING
bool g_boot_reason_change = false;
#endif
extern int g_pmic_cid;

int g_low_power_ready = 0;

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
    degree    AC Current    USB current    CV threshold    Recharge Vol    hysteresis condition 
    > 60       no charging current,             X                    X                     <54(Down) 
    45~60     600mA         450mA             4.1V               4V                   <39(Down) >60(Up) 
    10~45     600mA         450mA             4.2V               4.1V                <10(Down) >45(Up) 
    0~10       600mA         450mA             4.1V               4V                   <0(Down)  >16(Up) 
    -10~0     200mA         200mA             4V                  3.9V                <-10(Down) >6(Up) 
    <-10      no charging current,              X                    X                    >-10(Up)  
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
    kal_bool       bat_exist;
    kal_bool       bat_full;  
    kal_bool       bat_low;  
    INT32       bat_charging_state;
    INT32       bat_vol;            
    kal_bool    charger_exist;   
    INT32       pre_charging_current;
    INT32       charging_current;
    INT32       charger_vol;        
    INT32       charger_protect_status; 
    INT32       ISENSE;                
    INT32       ICharging;
    INT32       temperature;
    UINT32      total_charging_time;
    UINT32      PRE_charging_time;
    UINT32      CC_charging_time;
    UINT32      TOPOFF_charging_time;
    UINT32      POSTFULL_charging_time;
    INT32       charger_type;
    INT32       PWR_SRC;
    INT32       SOC;
    INT32       ADC_BAT_SENSE;
    INT32       ADC_I_SENSE;
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
static unsigned short batterySOCBuffer[BATTERY_AVERAGE_SIZE];
static int batteryIndex = 0;
static int batteryVoltageSum = 0;
static int batteryCurrentSum = 0;
static int batterySOCSum = 0;
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

int g_bat_temperature_pre=0;

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
int g_temp_CC_value = Cust_CC_0MA;

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
int g_HW_stop_charging = 0;
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

int get_charger_detect_status(void)
{
    return upmu_get_rgs_chrdet();
}
/********************************************** 
 * Battery Temprature Parameters and functions    
 ***********************************************/
typedef struct{
    INT32 BatteryTemp;
    INT32 TemperatureR;
} BATT_TEMPERATURE;

/* convert register to temperature  */
INT16 BattThermistorConverTemp(INT32 Res)
{
    int i = 0;
    INT32 RES1 = 0, RES2 = 0;
    INT32 TBatt_Value = -200, TMP1 = 0, TMP2 = 0;


#if defined(BAT_NTC_CG103JF103F)
BATT_TEMPERATURE Batt_Temperature_Table[] = {
{-20,67790},    
{-15,53460},
{-10,42450},
{ -5,33930},
{  0,27280},
{  5,22070},
{ 10,17960},
{ 15,14700},
{ 20,12090},
{ 25,10000},
{ 30,8312},
{ 35,6942},
{ 40,5826},
{ 45,4911},
{ 50,4158},
{ 55,3536},
{ 60,3019}
};
#endif

#if defined(BAT_NTC_BL197)
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
    INT32 dwVCriBat = 0;
    INT32 sBaTTMP = -100;
    
#if 0
    //Temp workaround
    if(upmu_get_cid() == 0x1020)
    {
        sBaTTMP=21;
        return sBaTTMP;
    }
#endif    

    //SW workaround-----------------------------------------------------
    //dwVCriBat = (TBAT_OVER_CRITICAL_LOW * 1800) / (TBAT_OVER_CRITICAL_LOW + 39000);    
    dwVCriBat = (TBAT_OVER_CRITICAL_LOW * RBAT_PULL_UP_VOLT) / (TBAT_OVER_CRITICAL_LOW + RBAT_PULL_UP_R);
       
    if(dwVolt > dwVCriBat)
    {
        TRes = TBAT_OVER_CRITICAL_LOW;
    }
    else
    {
        //TRes = (39000*dwVolt) / (1800-dwVolt);
        TRes = (RBAT_PULL_UP_R*dwVolt) / (RBAT_PULL_UP_VOLT-dwVolt);        
    }
    //------------------------------------------------------------------
        
    /* convert register to temperature */
    sBaTTMP = BattThermistorConverTemp(TRes);
    
    #ifdef CONFIG_DEBUG_MSG_NO_BQ27500
    printf("BattVoltToTemp() : TBAT_OVER_CRITICAL_LOW = %d\n", TBAT_OVER_CRITICAL_LOW);
    printf("BattVoltToTemp() : RBAT_PULL_UP_VOLT = %d\n", RBAT_PULL_UP_VOLT);
    printf("BattVoltToTemp() : dwVolt = %d\n", dwVolt);
    printf("BattVoltToTemp() : TRes = %d\n", TRes);
    printf("BattVoltToTemp() : sBaTTMP = %d\n", sBaTTMP);
    #endif
       
    return sBaTTMP;
}

//////////////////////////////////////////////////////
//// Pulse Charging Algorithm 
//////////////////////////////////////////////////////
void charger_hv_init(void)
{
    upmu_set_rg_vcdt_hv_vth(0xB);    //VCDT_HV_VTH, 7V
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

void fan5405_set_ac_current(void)
{
    int reg_set_value=0;
    
    if (Enable_BATDRV_LOG == 1) {
        printk("[BATTERY:fan5405] fan5405_set_ac_charging_current \r\n");    
    }    
    //fan5405_config_interface_liao(0x01,0xb8);

    #if 0
    //set the current to 1.25A,
    //1). 0x06h->0x70h  // set safety register first,
    //2). 0x01h->0xF8h
    //3). 0x02h->0x8Eh
    //4). 0x04h->0x79h
    //5). 0x05h->0x04h

    if(g_enable_high_vbat_spec == 1)
    {
        if(g_pmic_cid == 0x1020)
            fan5405_config_interface_liao(0x06,0x70);    
        else    
            fan5405_config_interface_liao(0x06,0x77);
    }
    else
        fan5405_config_interface_liao(0x06,0x70);
    
    fan5405_config_interface_liao(0x01,0xF8);

    if(g_enable_high_vbat_spec == 1)
    {
        if(g_pmic_cid == 0x1020)
            fan5405_config_interface_liao(0x02,0x8E);    
        else
            fan5405_config_interface_liao(0x02,0xaa);
    }
    else
        fan5405_config_interface_liao(0x02,0x8E);
    
    fan5405_config_interface_liao(0x04,0x79);
    fan5405_config_interface_liao(0x05,0x04);
    #endif

    #if 1
    //set the current to 650mA,
    //1). 0x06h->0x10h  // set safety register first,
    //2). 0x01h->0xF8h
    //3). 0x02h->0x8Eh
    //4). 0x04h->0x19h
    //5). 0x05h->0x04h

    if(g_enable_high_vbat_spec == 1)
    {
        if(g_pmic_cid == 0x1020)
            fan5405_config_interface_liao(0x06,0x70); //set ISAFE    
        else    
            fan5405_config_interface_liao(0x06,0x77); //set ISAFE
    }
    else
        fan5405_config_interface_liao(0x06,0x70); //set ISAFE
    
#if defined(MTK_JEITA_STANDARD_SUPPORT)        
    if(g_temp_status == TEMP_NEG_10_TO_POS_0)
    {    
        fan5405_config_interface_liao(0x05,0x24);
        fan5405_config_interface_liao(0x01,0x78);   //for low temp       
        fan5405_config_interface_liao(0x02,0x52);   //for 3.9v CV threshold
    }
    else 
    {
        fan5405_config_interface_liao(0x05,0x04);
        //fan5405_set_iocharge(0x01);                 //FAN5405 CON4 IOCHARGE
        
        if(g_low_power_ready == 1)
            fan5405_config_interface_liao(0x04,0x19);
        else
            fan5405_config_interface_liao(0x04,0x1B); //194mA
        
        fan5405_config_interface_liao(0x01,0xB8);

        if(g_temp_status == TEMP_POS_10_TO_POS_45)
        {
            if(g_enable_high_vbat_spec == 1)
            {
                if(g_pmic_cid == 0x1020)
                    fan5405_config_interface_liao(0x02,0x8E);    
                else
                    fan5405_config_interface_liao(0x02,0xaa);
            }
            else
                fan5405_config_interface_liao(0x02,0x8E);    //for 4.2v CV threshold
        }
        else
        {    
            fan5405_config_interface_liao(0x02,0x7A);    //for 4.1v CV threshold
        }
    }     
#else
    fan5405_config_interface_liao(0x01,0xF8);

    if(g_enable_high_vbat_spec == 1)
    {
        if(g_pmic_cid == 0x1020)
            fan5405_config_interface_liao(0x02,0x8E);    
        else
            fan5405_config_interface_liao(0x02,0xaa);
    }
    else
        fan5405_config_interface_liao(0x02,0x8E);

    #if defined(FAN5405_AC_CHARGING_CURRENT_1250)
        reg_set_value=0x70;
    #else
        #if defined(FAN5405_AC_CHARGING_CURRENT_1150)
            reg_set_value=0x60;
        #else
            #if defined(FAN5405_AC_CHARGING_CURRENT_1050)
                reg_set_value=0x50;
            #else
                #if defined(FAN5405_AC_CHARGING_CURRENT_950)
                    reg_set_value=0x40;
                #else
                    #if defined(FAN5405_AC_CHARGING_CURRENT_850)
                        reg_set_value=0x30;
                    #else
                        #if defined(FAN5405_AC_CHARGING_CURRENT_750)
                            reg_set_value=0x20; 
                        #else
                            reg_set_value=0x10;                                                
                        #endif
                    #endif
                #endif
            #endif
        #endif
    #endif
    if(g_low_power_ready == 1)
        reg_set_value += 0x09;
    else
        reg_set_value += 0x0B;
    fan5405_config_interface_liao(0x04,reg_set_value);
    
    fan5405_config_interface_liao(0x05,0x04);
#endif
    
    #endif
}

void select_charging_curret_fan5405()
{
    if ( BMT_status.charger_type == STANDARD_HOST ) 
    {        
        g_temp_CC_value = USB_CHARGER_CURRENT;    

#if defined(MTK_JEITA_STANDARD_SUPPORT)
        if(g_temp_status == TEMP_NEG_10_TO_POS_0)
        {
            fan5405_config_interface_liao(0x05,0x24);
            fan5405_config_interface_liao(0x01,0x78);   //for low temp  
            fan5405_config_interface_liao(0x02,0x52);   //for 3.9v CV threshold
        }
        else 
        {
            fan5405_config_interface_liao(0x05,0x04);
            //fan5405_set_iocharge(0x02);
            
            if(g_low_power_ready == 1) 
                fan5405_config_interface_liao(0x04,0x29);
            else
                fan5405_config_interface_liao(0x04,0x2B); //194mA
            
            fan5405_config_interface_liao(0x01,0x78);

            if(g_temp_status == TEMP_POS_10_TO_POS_45)
            {
                if(g_enable_high_vbat_spec == 1)
                {
                    if(g_pmic_cid == 0x1020)
                        fan5405_config_interface_liao(0x02,0x8E);
                    else
                        fan5405_config_interface_liao(0x02,0xaa);
                }
                else
                    fan5405_config_interface_liao(0x02,0x8E);  //for 4.2v CV threshold
            }
            else
            {    
                fan5405_config_interface_liao(0x02,0x7A);  //for 4.1v CV threshold
            }
        }                 
#else        
        fan5405_config_interface_liao(0x01,0x78);
#endif  

        if (Enable_BATDRV_LOG == 1) {
            printf("[BATTERY:fan5405] BMT_status.charger_type == STANDARD_HOST \r\n");    
        }
    } 
    else if (BMT_status.charger_type == NONSTANDARD_CHARGER) 
    {   
        g_temp_CC_value = USB_CHARGER_CURRENT;

#if defined(MTK_JEITA_STANDARD_SUPPORT)
        if(g_temp_status == TEMP_NEG_10_TO_POS_0)
        {        
            fan5405_config_interface_liao(0x05,0x24);
            fan5405_config_interface_liao(0x01,0x78);    //for low temp    
            fan5405_config_interface_liao(0x02,0x52);    //for 3.9v CV threshold
        }
        else 
        {
            fan5405_config_interface_liao(0x05,0x04);
            //fan5405_set_iocharge(0x02);
            
            if(g_low_power_ready == 1)
                fan5405_config_interface_liao(0x04,0x29);
            else
                fan5405_config_interface_liao(0x04,0x2B); //194mA
            
            fan5405_config_interface_liao(0x01,0x78);

            if(g_temp_status == TEMP_POS_10_TO_POS_45)
            {
                if(g_enable_high_vbat_spec == 1)
                {
                    if(g_pmic_cid == 0x1020)
                        fan5405_config_interface_liao(0x02,0x8E);    
                    else
                        fan5405_config_interface_liao(0x02,0xaa);
                }
                else
                    fan5405_config_interface_liao(0x02,0x8E);    //for 4.2v CV threshold
            }
            else
            {     
                fan5405_config_interface_liao(0x02,0x7A);    //for 4.1v CV threshold
            }
        }        
#else        
        fan5405_config_interface_liao(0x01,0x78);
#endif
        
        if (Enable_BATDRV_LOG == 1) {
            printf("[BATTERY:fan5405] BMT_status.charger_type == NONSTANDARD_CHARGER \r\n");    
        }
    } 
    else if (BMT_status.charger_type == STANDARD_CHARGER) 
    {
        g_temp_CC_value = AC_CHARGER_CURRENT;
        fan5405_set_ac_current();
        if (Enable_BATDRV_LOG == 1) {
            printf("[BATTERY:fan5405] BMT_status.charger_type == STANDARD_CHARGER \r\n");    
        }
    }
    else if (BMT_status.charger_type == CHARGING_HOST) 
    {
        g_temp_CC_value = AC_CHARGER_CURRENT;
        fan5405_set_ac_current();
        if (Enable_BATDRV_LOG == 1) {
            printf("[BATTERY:fan5405] BMT_status.charger_type == CHARGING_HOST \r\n");    
        }
    }
    else 
    {
        g_temp_CC_value = USB_CHARGER_CURRENT;
        fan5405_config_interface_liao(0x01,0x78);
        if (Enable_BATDRV_LOG == 1) {
            printf("[BATTERY:fan5405] default set g_temp_CC_value = USB_CHARGER_CURRENT \r\n");    
        }
    }
}

static unsigned int temp_init_flag = 0;
void ChargerHwInit_fan5405(void)
{
    if (Enable_BATDRV_LOG == 1) {
        printk("[MT BAT_probe] ChargerHwInit\n" );
    }

    if(temp_init_flag==0)
    {
        if(g_enable_high_vbat_spec == 1)
        {
            if(g_pmic_cid == 0x1020)
                fan5405_config_interface_liao(0x06,0x70);    
            else
                fan5405_config_interface_liao(0x06,0x77);
        }
        else
            fan5405_config_interface_liao(0x06,0x70);
        
        fan5405_config_interface_liao(0x00,0x80);
        fan5405_config_interface_liao(0x01,0xb1);

        if(g_enable_high_vbat_spec == 1)
        {
            if(g_pmic_cid == 0x1020)
                fan5405_config_interface_liao(0x02,0x8e);    
            else
                fan5405_config_interface_liao(0x02,0xaa);
        }
        else
            fan5405_config_interface_liao(0x02,0x8e);
                
        fan5405_config_interface_liao(0x05,0x04);
        
        if(g_low_power_ready == 1)
            fan5405_config_interface_liao(0x04,0x19);
        else
            fan5405_config_interface_liao(0x04,0x1B); //194mA
        
        temp_init_flag =1;    
    }
    else
    {
        fan5405_config_interface_liao(0x00,0x80);
    }
}

//int gpio_number   = GPIOEXT23; // 232+23=255
//int gpio_number   = GPIO_SWCHARGER_EN_PIN;
int gpio_number   = 0;
int gpio_off_mode = GPIO_MODE_GPIO;
int gpio_off_dir  = GPIO_DIR_OUT;
int gpio_off_out  = GPIO_OUT_ONE;
int gpio_on_mode  = GPIO_MODE_GPIO;
int gpio_on_dir   = GPIO_DIR_OUT;
int gpio_on_out   = GPIO_OUT_ZERO;

void pchr_turn_off_charging_fan5405 (void)
{
    if (Enable_BATDRV_LOG == 1) {
        printf("[BATTERY] pchr_turn_off_charging_fan5405 !\r\n");
    }
    
    mt_set_gpio_mode(gpio_number,gpio_off_mode);
    mt_set_gpio_dir(gpio_number,gpio_off_dir);
    mt_set_gpio_out(gpio_number,gpio_off_out);

    fan5405_config_interface_liao(0x01,0xbc);      
}

void pchr_turn_on_charging_fan5405 (void)
{

    mt_set_gpio_mode(gpio_number,gpio_on_mode);
    mt_set_gpio_dir(gpio_number,gpio_on_dir);
    mt_set_gpio_out(gpio_number,gpio_on_out);

    if ( BMT_status.bat_charging_state == CHR_ERROR ) 
    {
        //if (Enable_BATDRV_LOG == 1) {
            printf("[BATTERY] Charger Error, turn OFF charging !\r\n");
        //}
        pchr_turn_off_charging_fan5405();
    }
    else
    {
        ChargerHwInit_fan5405();
        
        if (Enable_BATDRV_LOG == 1) {
            printf("[BATTERY] pchr_turn_on_charging_fan5405 !\r\n");
        }
        
        select_charging_curret_fan5405();
        
        if( g_temp_CC_value == Cust_CC_0MA)
        {
            pchr_turn_off_charging_fan5405();
            printf("[BATTERY] g_temp_CC_value == Cust_CC_0MA !\r\n");
        }
        else
        {        
            if (Enable_BATDRV_LOG == 1) {
                printf("[BATTERY] charger enable !\r\n");
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
        ADC_BAT_SENSE_tmp[i] = get_bat_sense_volt(1);
        ADC_I_SENSE_tmp[i] = get_i_sense_volt(1);
    
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
        ICharging = (ADC_I_SENSE - ADC_BAT_SENSE)*1000/68; //68mohm
    }
    else
    {
        ICharging = 0;
    }

    return ICharging;
}

void BAT_Vbat_Compensate(void)
{
    if( (upmu_is_chr_det() == KAL_TRUE) && (g_bat_full_user_view != KAL_TRUE) )
    {
        if(BMT_status.ADC_BAT_SENSE <= vbat_compensate_cp)
        {
            printf("[vbat compensate before] BMT_status.bat_vol = %d\r\n", BMT_status.ADC_BAT_SENSE);
            BMT_status.ADC_BAT_SENSE = BMT_status.ADC_BAT_SENSE - vbat_compensate_value;
            printf("[vbat compensate after ] BMT_status.bat_vol = %d\r\n", BMT_status.ADC_BAT_SENSE);
        }
    }    
}

void BAT_GetVoltage_fan5405(void)
{ 
    int bat_temperature_volt=0;
    
    /* Get V_BAT_SENSE */
    if (g_chr_event == 0) 
    {        
        BMT_status.ADC_BAT_SENSE = get_bat_sense_volt(1);        
    } 
    else 
    {
        /* Just charger in/out event, same as I_sense */
        g_chr_event = 0;        
        BMT_status.ADC_BAT_SENSE = get_i_sense_volt(1);
    }
    if (batteryBufferFirst)
    {
        BAT_Vbat_Compensate();
    }
    else
    {
        if( (upmu_is_chr_det() == KAL_TRUE) && (g_bat_full_user_view != KAL_TRUE) )
        {
            if(BMT_status.ADC_BAT_SENSE <= vbat_compensate_cp)
            {
                printf("[vbat first compensate before] BMT_status.bat_vol = %d\r\n", BMT_status.ADC_BAT_SENSE);
                BMT_status.ADC_BAT_SENSE = BMT_status.ADC_BAT_SENSE + 20;
                printf("[vbat first compensate after ] BMT_status.bat_vol = %d\r\n", BMT_status.ADC_BAT_SENSE);
            }
        }
    }
    BMT_status.bat_vol = BMT_status.ADC_BAT_SENSE;

    /* Get V_I_SENSE */    
    //BMT_status.ADC_I_SENSE = PMIC_IMM_GetOneChannelValue(AUXADC_REF_CURRENT_CHANNEL, 1);
    //BMT_status.ADC_I_SENSE += gADC_I_SENSE_offset;    
    
    /* Get V_Charger */
    BMT_status.charger_vol = get_charger_volt(5);
    BMT_status.charger_vol = BMT_status.charger_vol / 100;
        
    /* Get V_BAT_Temperature */
    bat_temperature_volt = get_tbat_volt(5);            
    if(bat_temperature_volt == 0)
    {
        BMT_status.temperature = g_bat_temperature_pre;
        if (Enable_BATDRV_LOG == 1) {
            printf("[BATTERY] Warning !! bat_temperature_volt == 0, restore temperature value\n\r");
        }
    }
    else
    {
        BMT_status.temperature = BattVoltToTemp(bat_temperature_volt);
        g_bat_temperature_pre = BMT_status.temperature;     
    }    
    
    /* Calculate the charging current */
    //if(BMT_status.ADC_I_SENSE > BMT_status.ADC_BAT_SENSE)
    //    BMT_status.ICharging = (BMT_status.ADC_I_SENSE - BMT_status.ADC_BAT_SENSE) * 10 / R_CURRENT_SENSE;
    //else
    //    BMT_status.ICharging = 0;    

    /* Calculate the charging current */
    BMT_status.ICharging = g_Get_I_Charging();
    
    //if (Enable_BATDRV_LOG == 1) {
        printf("[BATTERY:ADC] VCHR:%d BAT_SENSE:%d I_SENSE:%d Current:%d\n", BMT_status.charger_vol,
        BMT_status.ADC_BAT_SENSE, BMT_status.ADC_I_SENSE, BMT_status.ICharging);
    //}
    
    g_BatteryAverageCurrent = BMT_status.ICharging;
}

void BAT_GetVoltage_notbat_fan5405(void)
{   
    /* Get V_BAT_SENSE */
    if (g_chr_event == 0) 
    {        
        BMT_status.ADC_BAT_SENSE = get_bat_sense_volt(1);        
    } 
    else 
    {
        /* Just charger in/out event, same as I_sense */
        g_chr_event = 0;        
        BMT_status.ADC_BAT_SENSE = get_i_sense_volt(1);
    }
    BMT_status.bat_vol = BMT_status.ADC_BAT_SENSE;
    
    /* Get V_I_SENSE */    
    BMT_status.ADC_I_SENSE = get_i_sense_volt(1);
    BMT_status.ADC_I_SENSE += gADC_I_SENSE_offset;    
    
    /* Get V_Charger */
    BMT_status.charger_vol = get_charger_volt(5);
    BMT_status.charger_vol = BMT_status.charger_vol / 100;                          
    
    /* Calculate the charging current */
    if(BMT_status.ADC_I_SENSE > BMT_status.ADC_BAT_SENSE)
        BMT_status.ICharging = (BMT_status.ADC_I_SENSE - BMT_status.ADC_BAT_SENSE) * 10 / R_CURRENT_SENSE;
    else
        BMT_status.ICharging = 0;    
    
    //if (Enable_BATDRV_LOG == 1) {
        printf("[BAT_GetVoltage_notbat] VCHR:%d BAT_SENSE:%d I_SENSE:%d Current:%d\n", BMT_status.charger_vol,
        BMT_status.ADC_BAT_SENSE, BMT_status.ADC_I_SENSE, BMT_status.ICharging);
    //}
    
    g_BatteryAverageCurrent = BMT_status.ICharging;
}

UINT32 BattVoltToPercent(UINT16 dwVoltage)
{
    UINT32 m = 0;
    UINT32 VBAT1 = 0, VBAT2 = 0;
    UINT32 bPercntResult = 0, bPercnt1 = 0, bPercnt2 = 0;
    
    //if (Enable_BATDRV_LOG == 1) {
    //    printf("###### 100 <-> voltage : %d ######\r\n", Batt_VoltToPercent_Table[10].BattVolt);
    //}
    
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
        if((g_temp_status == TEMP_NEG_10_TO_POS_0) && (BMT_status.temperature <= TEMP_POS_0_THRES_PLUS_X_DEGREE))
        {
            printf("[BATTERY] Battery Temperature between %d and %d !!\n\r",
                TEMP_POS_0_THRES_PLUS_X_DEGREE,TEMP_POS_10_THRESHOLD); 
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
            g_jeita_recharging_voltage = 3780;
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

int BAT_CheckBatteryStatus_fan5405(void)
{    
    int i = 0; 
    
    /* Get Battery Information */
    BAT_GetVoltage_fan5405();

    #if 0        
    if ((upmu_is_chr_det() == KAL_TRUE) && (g_HW_Charging_Done == 0) &&
        (BMT_status.bat_charging_state != CHR_ERROR) &&
        (BMT_status.bat_charging_state != CHR_TOP_OFF))
    {
        if ((BMT_status.total_charging_time % 10) == 0)
        {        
            g_HW_stop_charging = 1;        
            //printf("Disable charging 1s\n");        
        }
        else
        {
            g_HW_stop_charging = 0;
            //printf("Charging 1s\n");
        }
    }
    else
    {
        g_HW_stop_charging = 0;
        //printf("SW CV mode do not dis-charging 1s\n");
    }
    #endif
    
    /* Re-calculate Battery Percentage (SOC) */    
    BMT_status.SOC = BattVoltToPercent(BMT_status.bat_vol);
    if (Enable_BATDRV_LOG == 1) {
        printf("===> %d , %d (%d)\r\n", BMT_status.SOC, BMT_status.bat_vol, BATTERY_AVERAGE_SIZE);
    }
        
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
            batterySOCBuffer[i] = BMT_status.SOC;
        }
        
        batteryVoltageSum = BMT_status.bat_vol * BATTERY_AVERAGE_SIZE;
        batteryCurrentSum = BMT_status.ICharging * BATTERY_AVERAGE_SIZE;        
        batterySOCSum = BMT_status.SOC * BATTERY_AVERAGE_SIZE;
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
    
    batterySOCSum -= batterySOCBuffer[batteryIndex];
    batterySOCSum += BMT_status.SOC;
    batterySOCBuffer[batteryIndex] = BMT_status.SOC;
    
    BMT_status.bat_vol = batteryVoltageSum / BATTERY_AVERAGE_SIZE;
    BMT_status.ICharging = batteryCurrentSum / BATTERY_AVERAGE_SIZE;    
    BMT_status.SOC = batterySOCSum / BATTERY_AVERAGE_SIZE;
    
    batteryIndex++;
    if (batteryIndex >= BATTERY_AVERAGE_SIZE)
        batteryIndex = 0;
    /**************** Averaging : END ****************/
    
    if( BMT_status.SOC == 100 ) {
        BMT_status.bat_full = KAL_TRUE;   
    }
    
    /**************** For LK : Start ****************/
    if (low_bat_boot_display == 0)
    {
    
        /* SOC only UP when charging */
        if ( BMT_status.SOC > bat_volt_check_point ) {
            bat_volt_check_point = BMT_status.SOC;        
        }

#if 1        
        /* LK charging LED */
        if ( (bat_volt_check_point >= 90)  || (user_view_flag == KAL_TRUE) ) {
            leds_battery_full_charging();
        } else if(bat_volt_check_point <= 10) {
            leds_battery_low_charging();
        } else {
            leds_battery_medium_charging();
        }
#endif

#if 0        
        /* LK charging animation */
        if ( (BMT_status.bat_full) || (user_view_flag == KAL_TRUE) ) 
        {        
            if(g_bl_on == 1)
            {    
                mt_disp_show_battery_full();    
            }
            user_view_flag = KAL_TRUE;
        } 
        else 
        {    
            if ( (bat_volt_check_point>=0) && (bat_volt_check_point<25) )
            {
                prog_temp = 0;
            }
            else if ( (bat_volt_check_point>=25) && (bat_volt_check_point<50) )
            {
                prog_temp = 25;
            }
            else if ( (bat_volt_check_point>=50) && (bat_volt_check_point<75) )
            {
                prog_temp = 50;
            }
            else if ( (bat_volt_check_point>=75) && (bat_volt_check_point<100) )
            {
                prog_temp = 75;
            }
            else
            {
                prog_temp = 100;
            }
            
            if (prog_first == 1)
            {
                prog = prog_temp;
                prog_first = 0;
            }
            if(g_bl_on == 1)
            {    
                mt_disp_show_battery_capacity(prog);
            }
            prog += 25;
            if (prog > 100) prog = prog_temp;
        }
#endif

#if 0
        /* LK charging idle mode */
        if (!bl_switch) {
          //  mtk_sleep(500, KAL_FALSE);
            mt_disp_power(TRUE);
            bl_switch_timer++;
            mt65xx_backlight_on();
            g_bl_on = 1;                
        }    
        if (mtk_detect_key(BACKLIGHT_KEY)) { 
            bl_switch = KAL_FALSE;
            bl_switch_timer = 0;
            g_bl_on = 1;
            printf("[BATTERY] mt65xx_backlight_on\r\n");
        }    
        if (bl_switch_timer > BL_SWITCH_TIMEOUT) {
            bl_switch = KAL_TRUE;
            bl_switch_timer = 0;
            mt65xx_backlight_off();
            mt_disp_power(FALSE);
            g_bl_on = 0;
            
            // fill the screen with a whole black image
           // mt_disp_fill_rect(0, 0, CFG_DISPLAY_WIDTH, CFG_DISPLAY_HEIGHT, 0x0);
           // mt_disp_update(0, 0, CFG_DISPLAY_WIDTH, CFG_DISPLAY_HEIGHT);
            
            printf("[BATTERY] mt65xx_backlight_off\r\n");
        }
#endif

    }

    /**************** For LK : End ****************/    

    //if (Enable_BATDRV_LOG == 1) {
    printf("[BATTERY:AVG(%d,%dmA)] BatTemp:%d Vbat:%d VBatSen:%d SOC:%d ChrDet:%d Vchrin:%d Icharging:%d(%d) ChrType:%d \r\n", 
    BATTERY_AVERAGE_SIZE, CHARGING_FULL_CURRENT, BMT_status.temperature ,BMT_status.bat_vol, BMT_status.ADC_BAT_SENSE, BMT_status.SOC, 
    upmu_is_chr_det(), BMT_status.charger_vol, BMT_status.ICharging, g_BatteryAverageCurrent, CHR_Type_num );  
    //}  
    
    /* Protection Check : start*/
    //BAT_status = BAT_CheckPMUStatusReg();
    //if(BAT_status != PMU_STATUS_OK)
    //    return PMU_STATUS_FAIL;                  
        
#if defined(MTK_JEITA_STANDARD_SUPPORT)
    if (Enable_BATDRV_LOG == 1) {
        printf("[BATTERY] support JEITA, Tbat=%d\n", BMT_status.temperature);            
    }

    if( do_jeita_state_machine() == PMU_STATUS_FAIL)
    {
        return PMU_STATUS_FAIL;
    }
#else
    #if (BAT_TEMP_PROTECT_ENABLE == 1)
    if ((BMT_status.temperature <= MIN_CHARGE_TEMPERATURE) || 
        (BMT_status.temperature == ERR_CHARGE_TEMPERATURE))
    {
        printf(  "[BATTERY] Battery Under Temperature or NTC fail !!\n\r");                
        BMT_status.bat_charging_state = CHR_ERROR;
        return PMU_STATUS_FAIL;       
    }
    #endif                
    if (BMT_status.temperature >= MAX_CHARGE_TEMPERATURE)
    {
        printf(  "[BATTERY] Battery Over Temperature !!\n\r");                
        BMT_status.bat_charging_state = CHR_ERROR;
        return PMU_STATUS_FAIL;       
    }
#endif    
    
    if (upmu_is_chr_det() == KAL_TRUE)
    {
        #if (V_CHARGER_ENABLE == 1)
        if (BMT_status.charger_vol <= V_CHARGER_MIN )
        {
            printf(  "[BATTERY]Charger under voltage!!\r\n");                    
            BMT_status.bat_charging_state = CHR_ERROR;
            return PMU_STATUS_FAIL;        
        }
        #endif        
        if ( BMT_status.charger_vol >= V_CHARGER_MAX )
        {
            printf(  "[BATTERY]Charger over voltage !!\r\n");                    
            BMT_status.charger_protect_status = charger_OVER_VOL;
            BMT_status.bat_charging_state = CHR_ERROR;
            return PMU_STATUS_FAIL;        
        }
    }
    /* Protection Check : end*/
    
    if (upmu_is_chr_det() == KAL_TRUE)
    {

#if defined(MTK_JEITA_STANDARD_SUPPORT)
        if ((BMT_status.bat_vol < g_jeita_recharging_voltage) && (BMT_status.bat_full) && (g_HW_Charging_Done == 1))
#else    
        if ((BMT_status.bat_vol < RECHARGING_VOLTAGE) && (BMT_status.bat_full) && (g_HW_Charging_Done == 1))    
#endif            
        {
            //if (Enable_BATDRV_LOG == 1) {
            printf("[BATTERY] Check Battery Re-charging & turn on charging!!\n\r");                
            //}
            BMT_status.bat_full = KAL_FALSE;    
            g_bat_full_user_view = KAL_TRUE;
            BMT_status.bat_charging_state = CHR_CC;
            
            g_HW_Charging_Done = 0;         
            
            pchr_turn_on_charging_fan5405();
        }        
    }
    
    return PMU_STATUS_OK;
}

PMU_STATUS BAT_BatteryStatusFailAction(void)
{
    if (Enable_BATDRV_LOG == 1) {
        printf(  "[BATTERY] BAD Battery status... Charging Stop !!\n\r");            
    }
    
#if defined(MTK_JEITA_STANDARD_SUPPORT)
    if((g_temp_status == TEMP_ABOVE_POS_60) ||(g_temp_status == TEMP_BELOW_NEG_10))
    {
        temp_error_recovery_chr_flag=KAL_FALSE;
    }

    if((temp_error_recovery_chr_flag==KAL_FALSE) && (g_temp_status != TEMP_ABOVE_POS_60) && (g_temp_status != TEMP_BELOW_NEG_10))
    {
        temp_error_recovery_chr_flag=KAL_TRUE;
        BMT_status.bat_charging_state=CHR_PRE;
    }
#endif
    
    BMT_status.total_charging_time = 0;
    BMT_status.PRE_charging_time = 0;
    BMT_status.CC_charging_time = 0;
    BMT_status.TOPOFF_charging_time = 0;
    BMT_status.POSTFULL_charging_time = 0;

    /*  Disable charger */
    pchr_turn_off_charging_fan5405();
    
    return PMU_STATUS_OK;
}

PMU_STATUS BAT_ChargingOTAction(void)
{    
    printf(  "[BATTERY] Charging over time !!\n\r");            
    
    BMT_status.bat_full = KAL_TRUE;
    BMT_status.total_charging_time = 0;
    BMT_status.PRE_charging_time = 0;
    BMT_status.CC_charging_time = 0;
    BMT_status.TOPOFF_charging_time = 0;
    BMT_status.POSTFULL_charging_time = 0;
    
    g_HW_Charging_Done = 1;    
    g_Charging_Over_Time = 1;
    
    /*  Disable charger*/
    pchr_turn_off_charging_fan5405();
    
    return PMU_STATUS_OK;
}

PMU_STATUS BAT_BatteryFullAction(void)
{    
    printf(  "[BATTERY] Battery full !!\n\r");            
     
    BMT_status.bat_full = KAL_TRUE;
    BMT_status.total_charging_time = 0;
    BMT_status.PRE_charging_time = 0;
    BMT_status.CC_charging_time = 0;
    BMT_status.TOPOFF_charging_time = 0;
    BMT_status.POSTFULL_charging_time = 0;
    
    g_HW_Charging_Done = 1;

    /*  Disable charger */
    //pchr_turn_off_charging_fan5405();
    
    return PMU_STATUS_OK;
}

void check_battery_exist(void)
{
#if 1
    kal_uint32 baton_count = 0;
    int mode = 0;

    baton_count += upmu_get_rgs_baton_undet();
    baton_count += upmu_get_rgs_baton_undet();
    baton_count += upmu_get_rgs_baton_undet();
        
    mode = g_boot_arg->boot_mode &= 0x000000FF;
    if (g_boot_arg->maggic_number == BOOT_ARGUMENT_MAGIC)
    {        
        if( baton_count >= 3)
        {
                if( (mode == META_BOOT) || (mode == ADVMETA_BOOT) || (mode == ATE_FACTORY_BOOT) )
                {
                    printf("[BATTERY] boot mode = %d, bypass battery check\n", mode);
                }
                else
                {
                    printf("[BATTERY] Battery is not exist, power off FAN5405 and system (%d) 1\n", baton_count);
                    pchr_turn_off_charging_fan5405();
                    #ifndef NO_POWER_OFF
                    mt6575_power_off();
                    #endif 
                }
        }        
    }
    else
    {        
        if( baton_count >= 3)
        {
            printf("[BATTERY] Battery is not exist, power off FAN5405 and system (%d) 2\n", baton_count);
            pchr_turn_off_charging_fan5405();
            #ifndef NO_POWER_OFF
            mt6575_power_off();
            #endif        
        }
    }
#else
    printf("[BATTERY] Disable check battery exist for SMT\n");
#endif
}

void bmt_charger_ov_check(void)
{
     if(get_charger_hv_status() == 1)
     {
         pchr_turn_off_charging_fan5405();
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
             #ifdef GPT_TIMER                  
             mtk_sleep(500, KAL_FALSE);          
             #else              
             tmo = get_timer(0);              
             while(get_timer(tmo) <= 500 /* ms */);            
             #endif        
         }
    }    
}

void BAT_thread_fan5405(void)
{
    int BAT_status = 0;
    kal_uint32 fan5405_status=0;
       
    printf("[BATTERY] mtk_wdt_restart()\n");
    mtk_wdt_restart();
       
    if (Enable_BATDRV_LOG == 1) {
        printf("[BATTERY] LOG. %d---------------------------------------------------------------------\n", TBAT_OVER_CRITICAL_LOW);
    }

    if ((upmu_is_chr_det() == KAL_TRUE))
    {
        check_battery_exist();        
    }

    //printf("[BATTERY] GPIO_SWCHARGER_EN_PIN=%d\n", GPIO_SWCHARGER_EN_PIN);
    
    printf("[BATTERY] SET TMR_RST");
    fan5405_config_interface_liao(0x00,0x80);    
    
    /* If charger does not exist */
    if(get_charger_hv_status() == 1 || (upmu_is_chr_det() == KAL_FALSE))
    {
        bmt_charger_ov_check();
        BMT_status.charger_type = CHARGER_UNKNOWN;
        BMT_status.bat_full = KAL_FALSE;
        g_bat_full_user_view = KAL_FALSE;
        g_usb_state = USB_UNCONFIGURED;
        
        g_HW_Charging_Done = 0;
        g_Charging_Over_Time = 0;
        
        printf("[BATTERY] No Charger, Power OFF !?\n");
        pchr_turn_off_charging_fan5405();
        
        printf("[BATTERY] mt_power_off !!\n");
        #ifndef NO_POWER_OFF
        mt6575_power_off();
        #endif
        while(1);
    }
    
    /* Check Battery Status */
    BAT_status = BAT_CheckBatteryStatus_fan5405();
    if( BAT_status == PMU_STATUS_FAIL )
        g_Battery_Fail = KAL_TRUE;
    else
        g_Battery_Fail = KAL_FALSE;
    
    /* No Charger */
    if(BAT_status == PMU_STATUS_FAIL || g_Battery_Fail)    
    {
        BAT_BatteryStatusFailAction();
    }
    
    /* Battery Full *//* HW charging done, real stop charging */
    else if (g_HW_Charging_Done == 1)
    {   
        if (Enable_BATDRV_LOG == 1) {
            printf("[BATTERY] Battery real full. \n");
        }
        BAT_BatteryFullAction();        
    }
    
    /* Charging Overtime, can not charging */
    else if (g_Charging_Over_Time == 1)
    {
        if (Enable_BATDRV_LOG == 1) {
            printf("[BATTERY] Charging Over Time. \n");
        }
        pchr_turn_off_charging_fan5405();
    }
    
    /* Battery Not Full and Charger exist : Do Charging */
    else
    {
        if(BMT_status.total_charging_time >= MAX_CHARGING_TIME)
        {
            BMT_status.bat_charging_state = CHR_BATFULL;
            BAT_ChargingOTAction();
            return;
        }
        
        if ( BMT_status.TOPOFF_charging_time >= MAX_CV_CHARGING_TIME )
        {
            if (Enable_BATDRV_LOG == 1) {
                printf("BMT_status.TOPOFF_charging_time >= %d \r\n", MAX_CV_CHARGING_TIME);
            }
            BMT_status.bat_charging_state = CHR_BATFULL;
            BAT_BatteryFullAction();                                        
            return;
        }

        //printf("[BATTERY] DUMP-start \n");    
        fan5405_dump_register();
        //printf("[BATTERY] DUMP-end \n");
                
        fan5405_status = fan5405_get_chip_status();        
        /* check battery full */
        if( fan5405_status == 0x2 )
        {
            BMT_status.bat_charging_state = CHR_BATFULL;
            BMT_status.bat_full = KAL_TRUE;
            BMT_status.total_charging_time = 0;
            BMT_status.PRE_charging_time = 0;
            BMT_status.CC_charging_time = 0;
            BMT_status.TOPOFF_charging_time = 0;
            BMT_status.POSTFULL_charging_time = 0;    
            g_HW_Charging_Done = 1;            
            //pchr_turn_off_charging_fan5405();
            printk("[BATTERY:fan5405] Battery real full and disable charging (%ld) \n", fan5405_status); 
            return;
        }

        /* Charging flow begin */
        BMT_status.total_charging_time += BAT_TASK_PERIOD;
        pchr_turn_on_charging_fan5405();        
        if (Enable_BATDRV_LOG == 1) {
                printk(  "[BATTERY:fan5405] Total charging timer=%ld, fan5405_status=%ld \n\r", 
                    BMT_status.total_charging_time, fan5405_status);    
        }            
    }
        
}

void lk_charging_display()
{
    #define BATTERY_BAR 25
    
    if ( (BMT_status.bat_full) || (user_view_flag == KAL_TRUE) ) 
    {        
        if(g_bl_on == KAL_TRUE)
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
#ifdef ANIMATION_NEW
            mt_disp_show_battery_capacity(bat_volt_check_point);
#else    
            mt_disp_show_battery_capacity(prog);
#endif
        }
        prog += BATTERY_BAR;
        if (prog > 100) prog = prog_temp;
    }

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
        printf("[BATTERY] mt65xx_backlight_off\r\n");
    }
}

void batdrv_init(void)
{
    int i = 0;    
    
    /* Initialization BMT Struct */
    for (i=0; i<BATTERY_AVERAGE_SIZE; i++) {
        batteryCurrentBuffer[i] = 0;
        batteryVoltageBuffer[i] = 0; 
        batterySOCBuffer[i] = 0;
    }
    batteryVoltageSum = 0;
    batteryCurrentSum = 0;
    batterySOCSum = 0;
    
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
        printf("fan5405 hw init\n");        
        fan5405_hw_init();        
        fan5405_dump_register();
    }
    #if defined(CHARGER_IC_GPIO_RESETTING)
    gpio_number   = cust_gpio_number;
    gpio_off_mode = cust_gpio_off_mode;
    gpio_off_dir  = cust_gpio_off_dir;
    gpio_off_out  = cust_gpio_off_out;
    gpio_on_mode  = cust_gpio_on_mode;
    gpio_on_dir   = cust_gpio_on_dir;
    gpio_on_out   = cust_gpio_on_out;
    #endif
    printf("[LK][Charger IC] GPIO setting : %d,%d,%d,%d,%d,%d,%d\n",
    gpio_number, gpio_off_mode, gpio_off_dir, gpio_off_out, 
    gpio_on_mode, gpio_on_dir, gpio_on_out);
     
    upmu_set_rg_vcdt_hv_vth(0xB);    //VCDT_HV_VTH, 7V
    upmu_set_rg_vcdt_hv_en(1);       //VCDT_HV_EN
           
    printf("[BATTERY] batdrv_init : Done\n");
}

void check_point_sync_leds(void)
{
    int battery_level = BattVoltToPercent(BMT_status.bat_vol);
    printf("[BATTERY] %s  battery_level = %d \n", __func__, battery_level);

    if(battery_level >= 90)    //Full ARGB
    {
        leds_battery_full_charging();
    }
    else                              //Low and Medium ARGB
    {
        leds_battery_medium_charging();
    }
}

extern bool g_boot_menu;
#ifdef MTK_KERNEL_POWER_OFF_CHARGING
void mt65xx_bat_init(void)
{
    // Low Battery Safety Booting
    //pchr_turn_off_charging_fan5405();
    BMT_status.bat_vol = get_bat_sense_volt(1);
    printf("check VBAT=%d mV with %d mV\n", BMT_status.bat_vol, BATTERY_LOWVOL_THRESOLD);

    if ((upmu_is_chr_det() == KAL_TRUE))
    {
        fan5405_hw_init();        
        printf("[mt65xx_bat_init] fan5405 hw init\n");        
        fan5405_dump_register();            
		pchr_turn_on_charging_fan5405();
    }
	pmic_config_interface(INT_STATUS0,0x1,0x1,9);
	if(g_boot_mode == KERNEL_POWER_OFF_CHARGING_BOOT && (upmu_get_pwrkey_deb()==0) ) {
		printf("[mt65xx_bat_init] KPOC+PWRKEY=>change boot mode\n");
		g_boot_reason_change = true;
	}


    rtc_boot_check(false);

    if (BMT_status.bat_vol < BATTERY_LOWVOL_THRESOLD) {
        if(g_boot_mode == KERNEL_POWER_OFF_CHARGING_BOOT && upmu_is_chr_det() == KAL_TRUE) {
            printf("[%s] Kernel Low Battery Power Off Charging Mode\n", __func__);
            g_boot_mode = LOW_POWER_OFF_CHARGING_BOOT;
            return;
        }
        else
        {
            printf("[BATTERY] battery voltage(%dmV) <= CLV ! Can not Boot Linux Kernel !! \n\r",BMT_status.bat_vol);
#ifndef NO_POWER_OFF
            mt6575_power_off();
#endif			
            while(1)
            {
                printf("If you see the log, please check with RTC power off API\n\r");
            }
        }
    }
    return;
}
#else

void mt65xx_bat_init(void)
{    
    #ifndef GPT_TIMER
    long tmo;
    long tmo2;
    #endif
    BOOL print_msg = FALSE;    
    int press_pwrkey_count=0, loop_count = 0;    
    BOOL pwrkey_ready = false;
    BOOL back_to_charging_animation_flag = false;
     
    #if (CHARGING_PICTURE == 1)
    mt_disp_enter_charging_state();
    #else
    mt_disp_show_boot_logo();
    #endif    
    
    if ((upmu_is_chr_det() == KAL_TRUE))
    {
        check_battery_exist();
    }
    
    sc_mod_init();
    batdrv_init();
    
    BMT_status.bat_full = FALSE;
    BAT_GetVoltage_notbat_fan5405();    
   
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
        
        printf("[Battery] turn on charging first for sw workaround! \n");
        pchr_turn_on_charging_fan5405();        
        
        while (1) 
        {
            kick_charger_wdt();               
            upmu_set_rg_vcdt_hv_en(1);      //VCDT_HV_EN
            
            //add charger ov detection
            bmt_charger_ov_check();

            if (rtc_boot_check(true) || meta_mode_check() || (pwrkey_ready == true) 
                || mtk_wdt_boot_check()==WDT_BY_PASS_PWK_REBOOT || g_boot_arg->boot_reason==BR_TOOL_BY_PASS_PWK || g_boot_menu==true)
            {
                // Low Battery Safety Booting
                //pchr_turn_off_charging_fan5405();
                BMT_status.bat_vol = get_bat_sense_volt(1);
                printf("check VBAT=%d mV with %d mV\n", BMT_status.bat_vol, BATTERY_LOWVOL_THRESOLD);
                pchr_turn_on_charging_fan5405();

                while ( BMT_status.bat_vol < BATTERY_LOWVOL_THRESOLD )
                {    
                    if (low_bat_boot_display == 0)
                    {
                        mt_disp_power(TRUE);
                        mt65xx_backlight_off();
                        printf("Before mt6516_disp_show_low_battery\r\n");
                        mt_disp_show_low_battery();
                        printf("After mt6516_disp_show_low_battery\r\n");
                        mt_disp_wait_idle();
                        printf("After mt6516_disp_wait_idle\r\n");
                        
                        low_bat_boot_display = 1;                                                
                        
                        printf("Set low brightness\r\n");
                        mt65xx_leds_brightness_set(6, 20);
                    }
                    
                    rtc_boot_check(false);
                    BAT_thread_fan5405();
                    printf("-");
                    
                    #ifdef GPT_TIMER                        
                        if (g_bl_on == KAL_TRUE)
                            mtk_sleep(1000, KAL_FALSE);
                        else
                            mtk_sleep(1000, KAL_TRUE);
                    #else
                        tmo2 = get_timer(0);            
                        while(get_timer(tmo2) <= 1000 /* ms */);                    
                    #endif            
                    
                    if((pwrkey_ready ==true) & pmic_detect_powerkey()==0 )
                    {
                        back_to_charging_animation_flag = TRUE;
                        break;
                    }
                    else
                    {
                        back_to_charging_animation_flag = false;
                    }

                    //pchr_turn_off_charging_fan5405();
                    BMT_status.bat_vol = get_bat_sense_volt(1);
                    printf("VBAT=%d < %d\n", BMT_status.bat_vol, BATTERY_LOWVOL_THRESOLD);
                    pchr_turn_on_charging_fan5405();
                }
                 
                if(back_to_charging_animation_flag == false)
                {
                    mt_disp_power(KAL_TRUE);
                
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
                        printf("[BATTERY] Power off alarm trigger! Boot Linux Kernel!!\n\r");
                        
                        // fill the screen with a whole black image
                        mt_disp_fill_rect(0, 0, CFG_DISPLAY_WIDTH, CFG_DISPLAY_HEIGHT, 0x0);
                        mt_disp_update(0, 0, CFG_DISPLAY_WIDTH, CFG_DISPLAY_HEIGHT);
                        mt_disp_wait_idle();
                    }
                
                    printf("Restore brightness\r\n");
                    mt65xx_leds_brightness_set(6, 255);
                    check_point_sync_leds();
                    mt65xx_backlight_on();
                
                    //pchr_turn_off_charging_fan5405();
                    
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
                                                
            if (g_thread_count >= 5)  //change for charger ov 
            {
                g_thread_count = 1;
                BAT_thread_fan5405();
                printf(".");        
            }
            else
            {
                g_thread_count++;
            }

            if(print_msg==FALSE)
            {                    
                lk_charging_display();
                printf("[BATTERY] Charging !! Press Power Key to Booting !!! \n\r");                
                print_msg = TRUE;
            }
            
            #ifdef GPT_TIMER        
                if (g_bl_on == KAL_TRUE)
                    mtk_sleep(200, KAL_FALSE);
                else
                    mtk_sleep(200, KAL_TRUE);
            #else
                tmo = get_timer(0);            
                while(get_timer(tmo) <= 200 /* ms */);
            #endif        
            
            if (loop_count++ == 60) loop_count = 0;

            if (mtk_detect_key(BACKLIGHT_KEY) || (!pmic_detect_powerkey() && press_pwrkey_count > 0))
            {
                bl_switch = false;
                bl_switch_timer = 0;
                g_bl_on = true;
                printf("[BATTERY] mt65xx_backlight_on\r\n");
            }

            if (pmic_detect_powerkey())
            { 
                press_pwrkey_count++;
                printf("[BATTERY] press_pwrkey_count = %d, POWER_ON_TIME = %d\n", press_pwrkey_count, POWER_ON_TIME);
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
                    printf("[BATTERY] loop_count = %d\n", loop_count);
                }
                lk_charging_display();
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
            printf("[BATTERY] battery voltage(%dmV) >= CLV ! Boot Linux Kernel !! \n\r",BMT_status.bat_vol);
            sc_mod_exit();
            return;
        }
        else
        {
            printf("[BATTERY] battery voltage(%dmV) <= CLV ! Can not Boot Linux Kernel !! \n\r",BMT_status.bat_vol);
            pchr_turn_off_charging_fan5405();
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
#endif
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
