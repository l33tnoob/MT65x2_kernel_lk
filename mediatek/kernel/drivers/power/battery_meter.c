#include <linux/init.h>        /* For init/exit macros */
#include <linux/module.h>      /* For MODULE_ marcros  */
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/platform_device.h>

#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/kthread.h>
#include <linux/proc_fs.h>
#include <linux/rtc.h>
#include <linux/xlog.h>
#include <linux/time.h>

#include <asm/uaccess.h>
#include <mach/mt_typedefs.h>
#include <mach/hardware.h>
#include <mach/mt_boot.h>

#include <mach/battery_common.h>
#include <mach/battery_meter.h>
#include <mach/battery_meter_hal.h>
#include "cust_battery_meter.h"
#include "cust_battery_meter_table.h"
#include "mach/mtk_rtc.h"

// ============================================================ //
// define
// ============================================================ //
static DEFINE_MUTEX(FGADC_mutex);

int Enable_FGADC_LOG = 1;

// ============================================================ //
// global variable
// ============================================================ //
BATTERY_METER_CONTROL battery_meter_ctrl = NULL;

//static struct proc_dir_entry *proc_entry_fgadc;
static char proc_fgadc_data[32]; 

kal_bool gFG_Is_Charging = KAL_FALSE;
kal_int32 g_auxadc_solution = 0;
U32 g_spm_timer = 600;
bool bat_spm_timeout = false;
U32 _g_bat_sleep_total_time = 0;

///////////////////////////////////////////////////////////////////////////////////////////
//// PMIC AUXADC Related Variable
///////////////////////////////////////////////////////////////////////////////////////////
int g_R_BAT_SENSE = R_BAT_SENSE;
int g_R_I_SENSE = R_I_SENSE;
int g_R_CHARGER_1 = R_CHARGER_1;
int g_R_CHARGER_2 = R_CHARGER_2;

int fg_qmax_update_for_aging_flag = 1;

//HW FG
kal_int32 gFG_DOD0 = 0;
kal_int32 gFG_DOD1 = 0;
kal_int32 gFG_columb = 0;
kal_int32 gFG_voltage = 0;
kal_int32 gFG_current = 0;
kal_int32 gFG_capacity = 0;
kal_int32 gFG_capacity_by_c = 0;
kal_int32 gFG_capacity_by_c_init = 0;
kal_int32 gFG_capacity_by_v = 0;
kal_int32 gFG_capacity_by_v_init = 0;
kal_int32 gFG_temp= 100;
kal_int32 gFG_resistance_bat = 0;
kal_int32 gFG_compensate_value = 0;
kal_int32 gFG_ori_voltage = 0;
kal_int32 gFG_BATT_CAPACITY = 0;
kal_int32 gFG_voltage_init=0;
kal_int32 gFG_current_auto_detect_R_fg_total=0;
kal_int32 gFG_current_auto_detect_R_fg_count=0;
kal_int32 gFG_current_auto_detect_R_fg_result=0;
kal_int32 gFG_15_vlot=3700;
kal_int32 gFG_BATT_CAPACITY_init_high_current = 1200;
kal_int32 gFG_BATT_CAPACITY_aging = 1200;

//voltage mode
kal_int32 gfg_percent_check_point=50;
kal_int32 volt_mode_update_timer=0;
kal_int32 volt_mode_update_time_out=6; //1mins

//EM
kal_int32 g_fg_dbg_bat_volt=0;
kal_int32 g_fg_dbg_bat_current=0;
kal_int32 g_fg_dbg_bat_zcv=0;
kal_int32 g_fg_dbg_bat_temp=0;
kal_int32 g_fg_dbg_bat_r=0;
kal_int32 g_fg_dbg_bat_car=0;
kal_int32 g_fg_dbg_bat_qmax=0;
kal_int32 g_fg_dbg_d0=0;
kal_int32 g_fg_dbg_d1=0;
kal_int32 g_fg_dbg_percentage=0;
kal_int32 g_fg_dbg_percentage_fg=0;
kal_int32 g_fg_dbg_percentage_voltmode=0;

kal_int32 g_update_qmax_flag=1;
kal_int32 FGvbatVoltageBuffer[FG_VBAT_AVERAGE_SIZE];
kal_int32 FGbatteryIndex = 0;
kal_int32 FGbatteryVoltageSum = 0;
kal_int32 gFG_voltage_AVG = 0;
kal_int32 gFG_vbat_offset=0;
kal_int32 g_tracking_point = CUST_TRACKING_POINT;
kal_int32 g_rtc_fg_soc = 0;
kal_int32 g_I_SENSE_offset=0;

//SW FG
kal_int32 oam_v_ocv_init=0;
kal_int32 oam_v_ocv_1=0;
kal_int32 oam_v_ocv_2=0;
kal_int32 oam_r_1=0;
kal_int32 oam_r_2=0;
kal_int32 oam_d0=0;
kal_int32 oam_i_ori=0;
kal_int32 oam_i_1=0;
kal_int32 oam_i_2=0;
kal_int32 oam_car_1=0;
kal_int32 oam_car_2=0;
kal_int32 oam_d_1=1;
kal_int32 oam_d_2=1;
kal_int32 oam_d_3=1;
kal_int32 oam_d_3_pre=0;
kal_int32 oam_d_4=0;
kal_int32 oam_d_4_pre=0;
kal_int32 oam_d_5=0;
kal_int32 oam_init_i=0;
kal_int32 oam_run_i=0;
kal_int32 d5_count=0;
kal_int32 d5_count_time=60;
kal_int32 d5_count_time_rate=1;
kal_int32 g_d_hw_ocv=0;
kal_int32 g_vol_bat_hw_ocv=0;
kal_int32 g_hw_ocv_before_sleep=0;
struct timespec g_rtc_time_before_sleep;
kal_int32 g_sw_vbat_temp=0;
struct timespec last_oam_run_time;

/* Temperature window size */
#define TEMP_AVERAGE_SIZE 	30

#ifdef MTK_MULTI_BAT_PROFILE_SUPPORT
extern int IMM_GetOneChannelValue_Cali(int Channel, int*voltage);
kal_uint32 g_fg_battery_id = 0;

#ifdef MTK_GET_BATTERY_ID_BY_AUXADC
void fgauge_get_profile_id(void)
{
	int id_volt = 0;
    int id = 0;
	int ret = 0;

	ret = IMM_GetOneChannelValue_Cali(BATTERY_ID_CHANNEL_NUM, &id_volt);
	if(ret != 0)
        bm_print(BM_LOG_CRTI, "[fgauge_get_profile_id]id_volt read fail\n");
	else
        bm_print(BM_LOG_CRTI, "[fgauge_get_profile_id]id_volt = %d\n", id_volt);

    if ( (sizeof(g_battery_id_voltage)/sizeof(kal_int32)) != TOTAL_BATTERY_NUMBER)
    {
        bm_print(BM_LOG_CRTI, "[fgauge_get_profile_id]error! voltage range incorrect!\n");
        return;
    }

    for (id = 0; id < TOTAL_BATTERY_NUMBER; id++)
    {
        if(id_volt < g_battery_id_voltage[id])
        {
            g_fg_battery_id = id;
            break;
        }
        else if (g_battery_id_voltage[id] == -1)
        {
            g_fg_battery_id = TOTAL_BATTERY_NUMBER-1;
        }
    }

    bm_print(BM_LOG_CRTI, "[fgauge_get_profile_id]Battery id (%d)\n", g_fg_battery_id);		
}
#elif defined(MTK_GET_BATTERY_ID_BY_GPIO)
void fgauge_get_profile_id(void)
{
    g_fg_battery_id = 0;
}
#else
void fgauge_get_profile_id(void)
{
    g_fg_battery_id = 0;
}
#endif
#endif

// ============================================================ //
// function prototype
// ============================================================ //

// ============================================================ //
// extern variable
// ============================================================ //
 
// ============================================================ //
// extern function
// ============================================================ //
//extern int get_rtc_spare_fg_value(void);
//extern unsigned long rtc_read_hw_time(void);



// ============================================================ //
int get_r_fg_value(void)
{
    return (R_FG_VALUE+CUST_R_FG_OFFSET);
}
#ifdef MTK_MULTI_BAT_PROFILE_SUPPORT
int BattThermistorConverTemp(int Res)
{
    int i=0;
    int RES1=0,RES2=0;
    int TBatt_Value=-200,TMP1=0,TMP2=0;

    BATT_TEMPERATURE *batt_temperature_table = &Batt_Temperature_Table[g_fg_battery_id];
    if(Res>=batt_temperature_table[0].TemperatureR)
    {
        TBatt_Value = -20;
    }
    else if(Res<=batt_temperature_table[16].TemperatureR)
    {
        TBatt_Value = 60;
    }
    else
    {
        RES1=batt_temperature_table[0].TemperatureR;
        TMP1=batt_temperature_table[0].BatteryTemp;

        for(i=0;i<=16;i++)
        {
            if(Res>=batt_temperature_table[i].TemperatureR)
            {
                RES2=batt_temperature_table[i].TemperatureR;
                TMP2=batt_temperature_table[i].BatteryTemp;
                break;
            }
            else
            {
                RES1=batt_temperature_table[i].TemperatureR;
                TMP1=batt_temperature_table[i].BatteryTemp;
            }
        }
        
        TBatt_Value = (((Res-RES2)*TMP1)+((RES1-Res)*TMP2))/(RES1-RES2);
    }

    return TBatt_Value;    
}

kal_int32 fgauge_get_Q_max(kal_int16 temperature)
{
    kal_int32 ret_Q_max=0;
    kal_int32 low_temperature = 0, high_temperature = 0;
    kal_int32 low_Q_max = 0, high_Q_max = 0;

    if (temperature <= TEMPERATURE_T1)
    {
        low_temperature = (-10);
        low_Q_max = g_Q_MAX_NEG_10[g_fg_battery_id];
        high_temperature = TEMPERATURE_T1;
        high_Q_max = g_Q_MAX_POS_0[g_fg_battery_id];
        
        if(temperature < low_temperature)
        {
            temperature = low_temperature;
        }
    }
    else if (temperature <= TEMPERATURE_T2)
    {
        low_temperature = TEMPERATURE_T1;
        low_Q_max = g_Q_MAX_POS_0[g_fg_battery_id];
        high_temperature = TEMPERATURE_T2;
        high_Q_max = g_Q_MAX_POS_25[g_fg_battery_id];
        
        if(temperature < low_temperature)
        {
            temperature = low_temperature;
        }
    }
    else
    {
        low_temperature  = TEMPERATURE_T2;
        low_Q_max = g_Q_MAX_POS_25[g_fg_battery_id];
        high_temperature = TEMPERATURE_T3;
        high_Q_max = g_Q_MAX_POS_50[g_fg_battery_id];
        
        if(temperature > high_temperature)
        {
            temperature = high_temperature;
        }
    }

    ret_Q_max = low_Q_max +
    (
        (
            (temperature - low_temperature) * 
            (high_Q_max - low_Q_max)
        ) / 
        (high_temperature - low_temperature)                
    );

    bm_print(BM_LOG_FULL, "[fgauge_get_Q_max] Q_max = %d\r\n", ret_Q_max);

    return ret_Q_max;
}


kal_int32 fgauge_get_Q_max_high_current(kal_int16 temperature)
{
    kal_int32 ret_Q_max=0;
    kal_int32 low_temperature = 0, high_temperature = 0;
    kal_int32 low_Q_max = 0, high_Q_max = 0;

    if (temperature <= TEMPERATURE_T1)
    {
        low_temperature = (-10);
        low_Q_max = g_Q_MAX_NEG_10_H_CURRENT[g_fg_battery_id];
        high_temperature = TEMPERATURE_T1;
        high_Q_max = g_Q_MAX_POS_0_H_CURRENT[g_fg_battery_id];
        
        if(temperature < low_temperature)
        {
            temperature = low_temperature;
        }
    }
    else if (temperature <= TEMPERATURE_T2)
    {
        low_temperature = TEMPERATURE_T1;
        low_Q_max = g_Q_MAX_POS_0_H_CURRENT[g_fg_battery_id];
        high_temperature = TEMPERATURE_T2;
        high_Q_max = g_Q_MAX_POS_25_H_CURRENT[g_fg_battery_id];
        
        if(temperature < low_temperature)
        {
            temperature = low_temperature;
        }
    }
    else
    {
        low_temperature  = TEMPERATURE_T2;
        low_Q_max = g_Q_MAX_POS_25_H_CURRENT[g_fg_battery_id];
        high_temperature = TEMPERATURE_T3;
        high_Q_max = g_Q_MAX_POS_50_H_CURRENT[g_fg_battery_id];
        
        if(temperature > high_temperature)
        {
            temperature = high_temperature;
        }
    }

    ret_Q_max = low_Q_max +
    (
        (
            (temperature - low_temperature) * 
            (high_Q_max - low_Q_max)
        ) / 
        (high_temperature - low_temperature)                
    );

    bm_print(BM_LOG_FULL, "[fgauge_get_Q_max_high_current] Q_max = %d\r\n", ret_Q_max);

    return ret_Q_max;
}

#else

int BattThermistorConverTemp(int Res)
{
    int i=0;
    int RES1=0,RES2=0;
    int TBatt_Value=-200,TMP1=0,TMP2=0;

    if(Res>=Batt_Temperature_Table[0].TemperatureR)
    {
        TBatt_Value = -20;
    }
    else if(Res<=Batt_Temperature_Table[16].TemperatureR)
    {
        TBatt_Value = 60;
    }
    else
    {
        RES1=Batt_Temperature_Table[0].TemperatureR;
        TMP1=Batt_Temperature_Table[0].BatteryTemp;

        for(i=0;i<=16;i++)
        {
            if(Res>=Batt_Temperature_Table[i].TemperatureR)
            {
                RES2=Batt_Temperature_Table[i].TemperatureR;
                TMP2=Batt_Temperature_Table[i].BatteryTemp;
                break;
            }
            else
            {
                RES1=Batt_Temperature_Table[i].TemperatureR;
                TMP1=Batt_Temperature_Table[i].BatteryTemp;
            }
        }
        
        TBatt_Value = (((Res-RES2)*TMP1)+((RES1-Res)*TMP2))/(RES1-RES2);
    }

    return TBatt_Value;    
}

kal_int32 fgauge_get_Q_max(kal_int16 temperature)
{
    kal_int32 ret_Q_max=0;
    kal_int32 low_temperature = 0, high_temperature = 0;
    kal_int32 low_Q_max = 0, high_Q_max = 0;

    if (temperature <= TEMPERATURE_T1)
    {
        low_temperature = (-10);
        low_Q_max = Q_MAX_NEG_10;
        high_temperature = TEMPERATURE_T1;
        high_Q_max = Q_MAX_POS_0;
        
        if(temperature < low_temperature)
        {
            temperature = low_temperature;
        }
    }
    else if (temperature <= TEMPERATURE_T2)
    {
        low_temperature = TEMPERATURE_T1;
        low_Q_max = Q_MAX_POS_0;
        high_temperature = TEMPERATURE_T2;
        high_Q_max = Q_MAX_POS_25;
        
        if(temperature < low_temperature)
        {
            temperature = low_temperature;
        }
    }
    else
    {
        low_temperature  = TEMPERATURE_T2;
        low_Q_max = Q_MAX_POS_25;
        high_temperature = TEMPERATURE_T3;
        high_Q_max = Q_MAX_POS_50;
        
        if(temperature > high_temperature)
        {
            temperature = high_temperature;
        }
    }

    ret_Q_max = low_Q_max +
    (
        (
            (temperature - low_temperature) * 
            (high_Q_max - low_Q_max)
        ) / 
        (high_temperature - low_temperature)                
    );

    bm_print(BM_LOG_FULL, "[fgauge_get_Q_max] Q_max = %d\r\n", ret_Q_max);

    return ret_Q_max;
}


kal_int32 fgauge_get_Q_max_high_current(kal_int16 temperature)
{
    kal_int32 ret_Q_max=0;
    kal_int32 low_temperature = 0, high_temperature = 0;
    kal_int32 low_Q_max = 0, high_Q_max = 0;

    if (temperature <= TEMPERATURE_T1)
    {
        low_temperature = (-10);
        low_Q_max = Q_MAX_NEG_10_H_CURRENT;
        high_temperature = TEMPERATURE_T1;
        high_Q_max = Q_MAX_POS_0_H_CURRENT;
        
        if(temperature < low_temperature)
        {
            temperature = low_temperature;
        }
    }
    else if (temperature <= TEMPERATURE_T2)
    {
        low_temperature = TEMPERATURE_T1;
        low_Q_max = Q_MAX_POS_0_H_CURRENT;
        high_temperature = TEMPERATURE_T2;
        high_Q_max = Q_MAX_POS_25_H_CURRENT;
        
        if(temperature < low_temperature)
        {
            temperature = low_temperature;
        }
    }
    else
    {
        low_temperature  = TEMPERATURE_T2;
        low_Q_max = Q_MAX_POS_25_H_CURRENT;
        high_temperature = TEMPERATURE_T3;
        high_Q_max = Q_MAX_POS_50_H_CURRENT;
        
        if(temperature > high_temperature)
        {
            temperature = high_temperature;
        }
    }

    ret_Q_max = low_Q_max +
    (
        (
            (temperature - low_temperature) * 
            (high_Q_max - low_Q_max)
        ) / 
        (high_temperature - low_temperature)                
    );

    bm_print(BM_LOG_FULL, "[fgauge_get_Q_max_high_current] Q_max = %d\r\n", ret_Q_max);

    return ret_Q_max;
}

#endif

int BattVoltToTemp(int dwVolt)
{
    kal_int64 TRes_temp;
    kal_int64 TRes;
    int sBaTTMP = -100;

    // TRes_temp = ((kal_int64)RBAT_PULL_UP_R*(kal_int64)dwVolt) / (RBAT_PULL_UP_VOLT-dwVolt); 
    //TRes = (TRes_temp * (kal_int64)RBAT_PULL_DOWN_R)/((kal_int64)RBAT_PULL_DOWN_R - TRes_temp);

    TRes_temp = (RBAT_PULL_UP_R*(kal_int64)dwVolt); 
    do_div(TRes_temp, (RBAT_PULL_UP_VOLT-dwVolt));

    TRes = (TRes_temp * RBAT_PULL_DOWN_R);
    do_div(TRes, abs(RBAT_PULL_DOWN_R - TRes_temp));

    /* convert register to temperature */
    sBaTTMP = BattThermistorConverTemp((int)TRes);
      
    return sBaTTMP;
}

int force_get_tbat(void)
{
#if defined(CONFIG_POWER_EXT) || defined(FIXED_TBAT_25)
    bm_print(BM_LOG_CRTI, "[force_get_tbat] fixed TBAT=25 t\n");
    return 25;
#else
    int bat_temperature_volt=0;
    int bat_temperature_val=0;
    int fg_r_value=0;
    kal_int32 fg_current_temp=0;
    kal_bool fg_current_state=KAL_FALSE;
    int bat_temperature_volt_temp=0;
    int ret=0;
    
    /* Get V_BAT_Temperature */
    bat_temperature_volt = 2; 
    ret = battery_meter_ctrl(BATTERY_METER_CMD_GET_ADC_V_BAT_TEMP, &bat_temperature_volt);
    
    if(bat_temperature_volt != 0)
    {   
       	#if defined(SOC_BY_HW_FG)
        fg_r_value = get_r_fg_value();

        ret = battery_meter_ctrl(BATTERY_METER_CMD_GET_HW_FG_CURRENT, &fg_current_temp);          
        ret = battery_meter_ctrl(BATTERY_METER_CMD_GET_HW_FG_CURRENT_SIGN, &fg_current_state);
        fg_current_temp = fg_current_temp/10;
        
        if(fg_current_state==KAL_TRUE)
        {
            bat_temperature_volt_temp = bat_temperature_volt;
            bat_temperature_volt = bat_temperature_volt - ((fg_current_temp*fg_r_value)/1000);
        }
        else
        {
            bat_temperature_volt_temp = bat_temperature_volt;
            bat_temperature_volt = bat_temperature_volt + ((fg_current_temp*fg_r_value)/1000);
        }
#endif
    
        bat_temperature_val = BattVoltToTemp(bat_temperature_volt);        
    }
    
    bm_print(BM_LOG_CRTI, "[force_get_tbat] %d,%d,%d,%d,%d,%d\n", 
        bat_temperature_volt_temp, bat_temperature_volt, fg_current_state, fg_current_temp, fg_r_value, bat_temperature_val);
    
    return bat_temperature_val;    
#endif    
}
EXPORT_SYMBOL(force_get_tbat);

#ifdef MTK_MULTI_BAT_PROFILE_SUPPORT
int fgauge_get_saddles(void)
{
    return sizeof(battery_profile_temperature) / sizeof(BATTERY_PROFILE_STRUC);
}

int fgauge_get_saddles_r_table(void)
{
    return sizeof(r_profile_temperature) / sizeof(R_PROFILE_STRUC);
}

BATTERY_PROFILE_STRUC_P fgauge_get_profile(kal_uint32 temperature)
{
    switch (temperature)
    {
        case TEMPERATURE_T0:
            return &battery_profile_t0[g_fg_battery_id][0];
            break;    
        case TEMPERATURE_T1:
            return &battery_profile_t1[g_fg_battery_id][0];
            break;
        case TEMPERATURE_T2:
            return &battery_profile_t2[g_fg_battery_id][0];
            break;
        case TEMPERATURE_T3:
            return &battery_profile_t3[g_fg_battery_id][0];
            break;
        case TEMPERATURE_T:
            return &battery_profile_temperature[0];
            break;
        default:
            return NULL;
            break;
    }
}

R_PROFILE_STRUC_P fgauge_get_profile_r_table(kal_uint32 temperature)
{
    switch (temperature)
    {
        case TEMPERATURE_T0:
            return &r_profile_t0[g_fg_battery_id][0];
            break;
        case TEMPERATURE_T1:
            return &r_profile_t1[g_fg_battery_id][0];
            break;
        case TEMPERATURE_T2:
            return &r_profile_t2[g_fg_battery_id][0];
            break;
        case TEMPERATURE_T3:
            return &r_profile_t3[g_fg_battery_id][0];
            break;
        case TEMPERATURE_T:
            return &r_profile_temperature[0];
            break;
        default:
            return NULL;
            break;
    }
}
#else
int fgauge_get_saddles(void)
{
    return sizeof(battery_profile_t2) / sizeof(BATTERY_PROFILE_STRUC);
}

int fgauge_get_saddles_r_table(void)
{
    return sizeof(r_profile_t2) / sizeof(R_PROFILE_STRUC);
}

BATTERY_PROFILE_STRUC_P fgauge_get_profile(kal_uint32 temperature)
{
    switch (temperature)
    {
        case TEMPERATURE_T0:
            return &battery_profile_t0[0];
            break;    
        case TEMPERATURE_T1:
            return &battery_profile_t1[0];
            break;
        case TEMPERATURE_T2:
            return &battery_profile_t2[0];
            break;
        case TEMPERATURE_T3:
            return &battery_profile_t3[0];
            break;
        case TEMPERATURE_T:
            return &battery_profile_temperature[0];
            break;
        default:
            return NULL;
            break;
    }
}

R_PROFILE_STRUC_P fgauge_get_profile_r_table(kal_uint32 temperature)
{
    switch (temperature)
    {
        case TEMPERATURE_T0:
            return &r_profile_t0[0];
            break;
        case TEMPERATURE_T1:
            return &r_profile_t1[0];
            break;
        case TEMPERATURE_T2:
            return &r_profile_t2[0];
            break;
        case TEMPERATURE_T3:
            return &r_profile_t3[0];
            break;
        case TEMPERATURE_T:
            return &r_profile_temperature[0];
            break;
        default:
            return NULL;
            break;
    }
}
#endif

kal_int32 fgauge_read_capacity_by_v(kal_int32 voltage)
{    
    int i = 0, saddles = 0;
    BATTERY_PROFILE_STRUC_P profile_p;
    kal_int32 ret_percent = 0;

    profile_p = fgauge_get_profile(TEMPERATURE_T);
    if (profile_p == NULL)
    {
        bm_print(BM_LOG_CRTI, "[FGADC] fgauge get ZCV profile : fail !\r\n");
        return 100;
    }

    saddles = fgauge_get_saddles();

    if (voltage > (profile_p+0)->voltage)
    {
        return 100; // battery capacity, not dod
    }    
    if (voltage < (profile_p+saddles-1)->voltage)
    {
        return 0; // battery capacity, not dod
    }

    for (i = 0; i < saddles - 1; i++)
    {
        if ((voltage <= (profile_p+i)->voltage) && (voltage >= (profile_p+i+1)->voltage))
        {
            ret_percent = (profile_p+i)->percentage +
                (
                    (
                        ( ((profile_p+i)->voltage) - voltage ) * 
                        ( ((profile_p+i+1)->percentage) - ((profile_p + i)->percentage) ) 
                    ) /
                    ( ((profile_p+i)->voltage) - ((profile_p+i+1)->voltage) )
                );         
            
            break;
        }
        
    }
    ret_percent = 100 - ret_percent;

    return ret_percent;
}

kal_int32 fgauge_read_v_by_capacity(int bat_capacity)
{    
    int i = 0, saddles = 0;
    BATTERY_PROFILE_STRUC_P profile_p;
    kal_int32 ret_volt = 0;

    profile_p = fgauge_get_profile(TEMPERATURE_T);
    if (profile_p == NULL)
    {
        bm_print(BM_LOG_CRTI, "[fgauge_read_v_by_capacity] fgauge get ZCV profile : fail !\r\n");
        return 3700;
    }

    saddles = fgauge_get_saddles();

    if (bat_capacity < (profile_p+0)->percentage)
    {        
        return 3700;         
    }    
    if (bat_capacity > (profile_p+saddles-1)->percentage)
    {        
        return 3700;
    }

    for (i = 0; i < saddles - 1; i++)
    {
        if ((bat_capacity >= (profile_p+i)->percentage) && (bat_capacity <= (profile_p+i+1)->percentage))
        {
            ret_volt = (profile_p+i)->voltage -
                (
                    (
                        ( bat_capacity - ((profile_p+i)->percentage) ) * 
                        ( ((profile_p+i)->voltage) - ((profile_p+i+1)->voltage) ) 
                    ) /
                    ( ((profile_p+i+1)->percentage) - ((profile_p+i)->percentage) )
                );         
            
            break;
        }        
    }    

    return ret_volt;
}

kal_int32 fgauge_read_d_by_v(kal_int32 volt_bat)
{    
    int i = 0, saddles = 0;
    BATTERY_PROFILE_STRUC_P profile_p;
    kal_int32 ret_d = 0;

    profile_p = fgauge_get_profile(TEMPERATURE_T);
    if (profile_p == NULL)
    {
        bm_print(BM_LOG_CRTI, "[FGADC] fgauge get ZCV profile : fail !\r\n");
        return 100;
    }

    saddles = fgauge_get_saddles();

    if (volt_bat > (profile_p+0)->voltage)
    {
        return 0; 
    }    
    if (volt_bat < (profile_p+saddles-1)->voltage)
    {
        return 100; 
    }

    for (i = 0; i < saddles - 1; i++)
    {
        if ((volt_bat <= (profile_p+i)->voltage) && (volt_bat >= (profile_p+i+1)->voltage))
        {
            ret_d = (profile_p+i)->percentage +
                (
                    (
                        ( ((profile_p+i)->voltage) - volt_bat ) * 
                        ( ((profile_p+i+1)->percentage) - ((profile_p + i)->percentage) ) 
                    ) /
                    ( ((profile_p+i)->voltage) - ((profile_p+i+1)->voltage) )
                );         
            
            break;
        }
        
    }

    return ret_d;
}

kal_int32 fgauge_read_v_by_d(int d_val)
{    
    int i = 0, saddles = 0;
    BATTERY_PROFILE_STRUC_P profile_p;
    kal_int32 ret_volt = 0;

    profile_p = fgauge_get_profile(TEMPERATURE_T);
    if (profile_p == NULL)
    {
        bm_print(BM_LOG_CRTI, "[fgauge_read_v_by_capacity] fgauge get ZCV profile : fail !\r\n");
        return 3700;
    }

    saddles = fgauge_get_saddles();

    if (d_val < (profile_p+0)->percentage)
    {        
        return 3700;         
    }    
    if (d_val > (profile_p+saddles-1)->percentage)
    {        
        return 3700;
    }

    for (i = 0; i < saddles - 1; i++)
    {
        if ((d_val >= (profile_p+i)->percentage) && (d_val <= (profile_p+i+1)->percentage))
        {
            ret_volt = (profile_p+i)->voltage -
                (
                    (
                        ( d_val - ((profile_p+i)->percentage) ) * 
                        ( ((profile_p+i)->voltage) - ((profile_p+i+1)->voltage) ) 
                    ) /
                    ( ((profile_p+i+1)->percentage) - ((profile_p+i)->percentage) )
                );         
            
            break;
        }        
    }    

    return ret_volt;
}

kal_int32 fgauge_read_r_bat_by_v(kal_int32 voltage)
{    
    int i = 0, saddles = 0;
    R_PROFILE_STRUC_P profile_p;
    kal_int32 ret_r = 0;

    profile_p = fgauge_get_profile_r_table(TEMPERATURE_T);
    if (profile_p == NULL)
    {
        bm_print(BM_LOG_CRTI, "[FGADC] fgauge get R-Table profile : fail !\r\n");
        return (profile_p+0)->resistance;
    }

    saddles = fgauge_get_saddles_r_table();

    if (voltage > (profile_p+0)->voltage)
    {
        return (profile_p+0)->resistance; 
    }    
    if (voltage < (profile_p+saddles-1)->voltage)
    {
        return (profile_p+saddles-1)->resistance; 
    }

    for (i = 0; i < saddles - 1; i++)
    {
        if ((voltage <= (profile_p+i)->voltage) && (voltage >= (profile_p+i+1)->voltage))
        {
            ret_r = (profile_p+i)->resistance +
                (
                    (
                        ( ((profile_p+i)->voltage) - voltage ) * 
                        ( ((profile_p+i+1)->resistance) - ((profile_p + i)->resistance) ) 
                    ) /
                    ( ((profile_p+i)->voltage) - ((profile_p+i+1)->voltage) )
                );
            break;
        }
    }

    return ret_r;
}


void fgauge_construct_battery_profile(kal_int32 temperature, BATTERY_PROFILE_STRUC_P temp_profile_p)
{
    BATTERY_PROFILE_STRUC_P low_profile_p, high_profile_p;
    kal_int32 low_temperature, high_temperature;
    int i, saddles;
    kal_int32 temp_v_1 = 0, temp_v_2 = 0;

    if (temperature <= TEMPERATURE_T1)
    {
        low_profile_p    = fgauge_get_profile(TEMPERATURE_T0);
        high_profile_p   = fgauge_get_profile(TEMPERATURE_T1);
        low_temperature  = (-10);
        high_temperature = TEMPERATURE_T1;
        
        if(temperature < low_temperature)
        {
            temperature = low_temperature;
        }
    }
    else if (temperature <= TEMPERATURE_T2)
    {
        low_profile_p    = fgauge_get_profile(TEMPERATURE_T1);
        high_profile_p   = fgauge_get_profile(TEMPERATURE_T2);
        low_temperature  = TEMPERATURE_T1;
        high_temperature = TEMPERATURE_T2;
        
        if(temperature < low_temperature)
        {
            temperature = low_temperature;
        }
    }
    else
    {
        low_profile_p    = fgauge_get_profile(TEMPERATURE_T2);
        high_profile_p   = fgauge_get_profile(TEMPERATURE_T3);
        low_temperature  = TEMPERATURE_T2;
        high_temperature = TEMPERATURE_T3;
        
        if(temperature > high_temperature)
        {
            temperature = high_temperature;
        }
    }

    saddles = fgauge_get_saddles();

    for (i = 0; i < saddles; i++)
    {
        if( ((high_profile_p + i)->voltage) > ((low_profile_p + i)->voltage) )
        {
            temp_v_1 = (high_profile_p + i)->voltage;
            temp_v_2 = (low_profile_p + i)->voltage;    

            (temp_profile_p + i)->voltage = temp_v_2 +
            (
                (
                    (temperature - low_temperature) * 
                    (temp_v_1 - temp_v_2)
                ) / 
                (high_temperature - low_temperature)                
            );
        }
        else
        {
            temp_v_1 = (low_profile_p + i)->voltage;
            temp_v_2 = (high_profile_p + i)->voltage;

            (temp_profile_p + i)->voltage = temp_v_2 +
            (
                (
                    (high_temperature - temperature) * 
                    (temp_v_1 - temp_v_2)
                ) / 
                (high_temperature - low_temperature)                
            );
        }
    
        (temp_profile_p + i)->percentage = (high_profile_p + i)->percentage;
#if 0        
        (temp_profile_p + i)->voltage = temp_v_2 +
            (
                (
                    (temperature - low_temperature) * 
                    (temp_v_1 - temp_v_2)
                ) / 
                (high_temperature - low_temperature)                
            );
#endif
    }

    
    // Dumpt new battery profile
    for (i = 0; i < saddles ; i++)
    {
        bm_print(BM_LOG_CRTI, "<DOD,Voltage> at %d = <%d,%d>\r\n",
            temperature, (temp_profile_p+i)->percentage, (temp_profile_p+i)->voltage);
    }
    
}

void fgauge_construct_r_table_profile(kal_int32 temperature, R_PROFILE_STRUC_P temp_profile_p)
{
    R_PROFILE_STRUC_P low_profile_p, high_profile_p;
    kal_int32 low_temperature, high_temperature;
    int i, saddles;
    kal_int32 temp_v_1 = 0, temp_v_2 = 0;
    kal_int32 temp_r_1 = 0, temp_r_2 = 0;

    if (temperature <= TEMPERATURE_T1)
    {
        low_profile_p    = fgauge_get_profile_r_table(TEMPERATURE_T0);
        high_profile_p   = fgauge_get_profile_r_table(TEMPERATURE_T1);
        low_temperature  = (-10);
        high_temperature = TEMPERATURE_T1;
        
        if(temperature < low_temperature)
        {
            temperature = low_temperature;
        }
    }
    else if (temperature <= TEMPERATURE_T2)
    {
        low_profile_p    = fgauge_get_profile_r_table(TEMPERATURE_T1);
        high_profile_p   = fgauge_get_profile_r_table(TEMPERATURE_T2);
        low_temperature  = TEMPERATURE_T1;
        high_temperature = TEMPERATURE_T2;
        
        if(temperature < low_temperature)
        {
            temperature = low_temperature;
        }
    }
    else
    {
        low_profile_p    = fgauge_get_profile_r_table(TEMPERATURE_T2);
        high_profile_p   = fgauge_get_profile_r_table(TEMPERATURE_T3);
        low_temperature  = TEMPERATURE_T2;
        high_temperature = TEMPERATURE_T3;
        
        if(temperature > high_temperature)
        {
            temperature = high_temperature;
        }
    }

    saddles = fgauge_get_saddles_r_table();

    /* Interpolation for V_BAT */
    for (i = 0; i < saddles; i++)
    {
        if( ((high_profile_p + i)->voltage) > ((low_profile_p + i)->voltage) )
        {
            temp_v_1 = (high_profile_p + i)->voltage;
            temp_v_2 = (low_profile_p + i)->voltage;    

            (temp_profile_p + i)->voltage = temp_v_2 +
            (
                (
                    (temperature - low_temperature) * 
                    (temp_v_1 - temp_v_2)
                ) / 
                (high_temperature - low_temperature)                
            );
        }
        else
        {
            temp_v_1 = (low_profile_p + i)->voltage;
            temp_v_2 = (high_profile_p + i)->voltage;

            (temp_profile_p + i)->voltage = temp_v_2 +
            (
                (
                    (high_temperature - temperature) * 
                    (temp_v_1 - temp_v_2)
                ) / 
                (high_temperature - low_temperature)                
            );
        }

#if 0    
        //(temp_profile_p + i)->resistance = (high_profile_p + i)->resistance;
        
        (temp_profile_p + i)->voltage = temp_v_2 +
            (
                (
                    (temperature - low_temperature) * 
                    (temp_v_1 - temp_v_2)
                ) / 
                (high_temperature - low_temperature)                
            );
#endif
    }

    /* Interpolation for R_BAT */
    for (i = 0; i < saddles; i++)
    {
        if( ((high_profile_p + i)->resistance) > ((low_profile_p + i)->resistance) )
        {
            temp_r_1 = (high_profile_p + i)->resistance;
            temp_r_2 = (low_profile_p + i)->resistance;    

            (temp_profile_p + i)->resistance = temp_r_2 +
            (
                (
                    (temperature - low_temperature) * 
                    (temp_r_1 - temp_r_2)
                ) / 
                (high_temperature - low_temperature)                
            );
        }
        else
        {
            temp_r_1 = (low_profile_p + i)->resistance;
            temp_r_2 = (high_profile_p + i)->resistance;

            (temp_profile_p + i)->resistance = temp_r_2 +
            (
                (
                    (high_temperature - temperature) * 
                    (temp_r_1 - temp_r_2)
                ) / 
                (high_temperature - low_temperature)                
            );
        }

#if 0    
        //(temp_profile_p + i)->voltage = (high_profile_p + i)->voltage;
        
        (temp_profile_p + i)->resistance = temp_r_2 +
            (
                (
                    (temperature - low_temperature) * 
                    (temp_r_1 - temp_r_2)
                ) / 
                (high_temperature - low_temperature)                
            );
#endif
    }

    // Dumpt new r-table profile
    for (i = 0; i < saddles ; i++)
    {
        bm_print(BM_LOG_CRTI, "<Rbat,VBAT> at %d = <%d,%d>\r\n",
            temperature, (temp_profile_p+i)->resistance, (temp_profile_p+i)->voltage);
    }
    
}

void fgauge_construct_table_by_temp(void)
{
#if defined(CONFIG_POWER_EXT)
#else
    kal_uint32 i;
    static kal_int32 init_temp = KAL_TRUE;
    static kal_int32 curr_temp, last_temp, avg_temp;
    static kal_int32 battTempBuffer[TEMP_AVERAGE_SIZE];
    static kal_int32 temperature_sum;
    static kal_uint8 tempIndex = 0;

    curr_temp = battery_meter_get_battery_temperature();

    // Temperature window init
    if (init_temp == KAL_TRUE)
    {
        for (i=0; i<TEMP_AVERAGE_SIZE; i++)
		{
            battTempBuffer[i] = curr_temp;            
		}
        last_temp = curr_temp;
        temperature_sum = curr_temp * TEMP_AVERAGE_SIZE;
        init_temp = KAL_FALSE;
    }

    // Temperature sliding window 
	temperature_sum -= battTempBuffer[tempIndex];
	temperature_sum += curr_temp;
	battTempBuffer[tempIndex] = curr_temp;
    avg_temp = (temperature_sum)/TEMP_AVERAGE_SIZE;

    if (avg_temp != last_temp)
    {
        bm_print(BM_LOG_FULL, "[fgauge_construct_table_by_temp] reconstruct table by temperature change from (%d) to (%d)\r\n", last_temp, avg_temp);
        fgauge_construct_r_table_profile(curr_temp, fgauge_get_profile_r_table(TEMPERATURE_T));
        fgauge_construct_battery_profile(curr_temp, fgauge_get_profile(TEMPERATURE_T));
        last_temp = avg_temp;
    }

    tempIndex = (tempIndex+1)%TEMP_AVERAGE_SIZE;

#endif
}

void fg_qmax_update_for_aging(void)
{
#if defined(CONFIG_POWER_EXT)
#else
    kal_bool hw_charging_done = bat_is_charging_full();

    if(hw_charging_done == KAL_TRUE) // charging full, g_HW_Charging_Done == 1
    {
        if(gFG_DOD0 > 85)
        {
        	if(gFG_columb < 0)
				gFG_columb = gFG_columb - gFG_columb*2;		//  absolute value
			
            gFG_BATT_CAPACITY_aging = ( ( (gFG_columb*1000)+(5*gFG_DOD0) ) / gFG_DOD0 ) / 10;

            // tuning
            gFG_BATT_CAPACITY_aging = (gFG_BATT_CAPACITY_aging * 100) / AGING_TUNING_VALUE;

            if(gFG_BATT_CAPACITY_aging == 0)
            {
                gFG_BATT_CAPACITY_aging = fgauge_get_Q_max(battery_meter_get_battery_temperature());
                bm_print(BM_LOG_CRTI, "[fg_qmax_update_for_aging] error, restore gFG_BATT_CAPACITY_aging (%d)\n", gFG_BATT_CAPACITY_aging);
            }
            
            bm_print(BM_LOG_CRTI, "[fg_qmax_update_for_aging] need update : gFG_columb=%d, gFG_DOD0=%d, new_qmax=%d\r\n", 
                gFG_columb, gFG_DOD0, gFG_BATT_CAPACITY_aging);
        }
        else
        {
            bm_print(BM_LOG_CRTI, "[fg_qmax_update_for_aging] no update : gFG_columb=%d, gFG_DOD0=%d, new_qmax=%d\r\n", 
                gFG_columb, gFG_DOD0, gFG_BATT_CAPACITY_aging);
        }
    }
    else
    {
        bm_print(BM_LOG_CRTI, "[fg_qmax_update_for_aging] hw_charging_done=%d\r\n", hw_charging_done);
    }
#endif    
}


void dod_init(void)
{
    #if defined(SOC_BY_HW_FG)
    int ret = 0;    
    //use get_hw_ocv-----------------------------------------------------------------        
    ret=battery_meter_ctrl(BATTERY_METER_CMD_GET_HW_OCV, &gFG_voltage);
    gFG_capacity_by_v = fgauge_read_capacity_by_v(gFG_voltage);
    
    bm_print(BM_LOG_CRTI, "[FGADC] get_hw_ocv=%d, HW_SOC=%d, SW_SOC = %d\n", 
        gFG_voltage, gFG_capacity_by_v, gFG_capacity_by_v_init);
    
    // compare with hw_ocv & sw_ocv, check if less than or equal to 5% tolerance 
    if ((abs(gFG_capacity_by_v_init - gFG_capacity_by_v) > 5) && (bat_is_charger_exist() == KAL_TRUE))
    {
        gFG_capacity_by_v = gFG_capacity_by_v_init;
    }
    //-------------------------------------------------------------------------------
    #endif
    
    #if defined(CONFIG_POWER_EXT)
    g_rtc_fg_soc = gFG_capacity_by_v;    
    #else
    g_rtc_fg_soc = get_rtc_spare_fg_value();
    #endif
    if(g_rtc_fg_soc >= gFG_capacity_by_v)
    {
        if(((g_rtc_fg_soc != 0) && ((g_rtc_fg_soc-gFG_capacity_by_v) < CUST_POWERON_DELTA_CAPACITY_TOLRANCE) &&(( gFG_capacity_by_v > CUST_POWERON_LOW_CAPACITY_TOLRANCE || bat_is_charger_exist() == KAL_TRUE)))
			|| ((g_rtc_fg_soc != 0) &&(g_boot_reason == BR_WDT_BY_PASS_PWK || g_boot_reason == BR_WDT || g_boot_reason == BR_TOOL_BY_PASS_PWK || g_boot_reason == BR_2SEC_REBOOT || g_boot_mode == RECOVERY_BOOT)))
            
        {
            gFG_capacity_by_v = g_rtc_fg_soc;            
        }
    }
    else
    {
    	if(((g_rtc_fg_soc != 0) && ((gFG_capacity_by_v-g_rtc_fg_soc) < CUST_POWERON_DELTA_CAPACITY_TOLRANCE) &&(( gFG_capacity_by_v > CUST_POWERON_LOW_CAPACITY_TOLRANCE || bat_is_charger_exist() == KAL_TRUE)))
			|| ((g_rtc_fg_soc != 0) &&(g_boot_reason == BR_WDT_BY_PASS_PWK || g_boot_reason == BR_WDT || g_boot_reason == BR_TOOL_BY_PASS_PWK || g_boot_reason == BR_2SEC_REBOOT || g_boot_mode == RECOVERY_BOOT)))
        {
            gFG_capacity_by_v = g_rtc_fg_soc;            
        }
    }        

    bm_print(BM_LOG_CRTI, "[FGADC] g_rtc_fg_soc=%d, gFG_capacity_by_v=%d\n", 
            g_rtc_fg_soc, gFG_capacity_by_v);
    
    if (gFG_capacity_by_v == 0 && bat_is_charger_exist() == KAL_TRUE) {
        gFG_capacity_by_v = 1;
        
        bm_print(BM_LOG_CRTI, "[FGADC] gFG_capacity_by_v=%d\n", 
            gFG_capacity_by_v);
    }
    gFG_capacity = gFG_capacity_by_v;   
    gFG_capacity_by_c_init = gFG_capacity;
    gFG_capacity_by_c = gFG_capacity;
    
    gFG_DOD0 = 100 - gFG_capacity;
    gFG_DOD1=gFG_DOD0;

    gfg_percent_check_point = gFG_capacity;        

#if defined(CHANGE_TRACKING_POINT)
    gFG_15_vlot = fgauge_read_v_by_capacity( (100-g_tracking_point) );
    bm_print(BM_LOG_CRTI, "[FGADC] gFG_15_vlot = %dmV\n", gFG_15_vlot);        
#else
    //gFG_15_vlot = fgauge_read_v_by_capacity(86); //14%
    gFG_15_vlot = fgauge_read_v_by_capacity( (100-g_tracking_point) );
    bm_print(BM_LOG_CRTI, "[FGADC] gFG_15_vlot = %dmV\n", gFG_15_vlot);        
    if( (gFG_15_vlot > 3800) || (gFG_15_vlot < 3600) ) 
    {
        bm_print(BM_LOG_CRTI, "[FGADC] gFG_15_vlot(%d) over range, reset to 3700\n", gFG_15_vlot);
        gFG_15_vlot = 3700;
    }
#endif
}

// ============================================================ // SW FG
kal_int32 mtk_imp_tracking(kal_int32 ori_voltage, kal_int32 ori_current, kal_int32 recursion_time)
{
    kal_int32 ret_compensate_value = 0;
    kal_int32 temp_voltage_1 = ori_voltage;
    kal_int32 temp_voltage_2 = temp_voltage_1;
    int i = 0;

    for(i=0 ; i < recursion_time ; i++) 
    {
        gFG_resistance_bat = fgauge_read_r_bat_by_v(temp_voltage_2); 
        ret_compensate_value = ( (ori_current) * (gFG_resistance_bat + R_FG_VALUE)) / 1000;
        ret_compensate_value = (ret_compensate_value+(10/2)) / 10; 
        temp_voltage_2 = temp_voltage_1 + ret_compensate_value;

        bm_print(BM_LOG_FULL, "[mtk_imp_tracking] temp_voltage_2=%d,temp_voltage_1=%d,ret_compensate_value=%d,gFG_resistance_bat=%d\n", 
            temp_voltage_2,temp_voltage_1,ret_compensate_value,gFG_resistance_bat);
    }
    
    gFG_resistance_bat = fgauge_read_r_bat_by_v(temp_voltage_2); 
    ret_compensate_value = ( (ori_current) * (gFG_resistance_bat + R_FG_VALUE + FG_METER_RESISTANCE)) / 1000;    
    ret_compensate_value = (ret_compensate_value+(10/2)) / 10; 

    gFG_compensate_value = ret_compensate_value;

    bm_print(BM_LOG_FULL, "[mtk_imp_tracking] temp_voltage_2=%d,temp_voltage_1=%d,ret_compensate_value=%d,gFG_resistance_bat=%d\n", 
        temp_voltage_2,temp_voltage_1,ret_compensate_value,gFG_resistance_bat);    

    return ret_compensate_value;
}

void oam_init(void)
{
    int ret=0;
	int vol_bat=0;
	kal_int32 vbat_capacity = 0;

	vol_bat = 5; //set avg times
    ret = battery_meter_ctrl(BATTERY_METER_CMD_GET_ADC_V_BAT_SENSE, &vol_bat);
    ret = battery_meter_ctrl(BATTERY_METER_CMD_GET_HW_OCV, &gFG_voltage);

	gFG_capacity_by_v = fgauge_read_capacity_by_v(gFG_voltage);
	vbat_capacity = fgauge_read_capacity_by_v(vol_bat);
	
    if(bat_is_charger_exist() == KAL_TRUE)
    {
    	bm_print(BM_LOG_CRTI, "[oam_init_inf] gFG_capacity_by_v=%d, vbat_capacity=%d, \n",gFG_capacity_by_v,vbat_capacity);

    	// to avoid plug in cable without battery, then plug in battery to make hw soc = 100% 
    	// if the difference bwtween ZCV and vbat is too large, using vbat instead ZCV
    	if(((gFG_capacity_by_v == 100) && (vbat_capacity < CUST_POWERON_MAX_VBAT_TOLRANCE)) ||(abs(gFG_capacity_by_v-vbat_capacity)>CUST_POWERON_DELTA_VBAT_TOLRANCE))	
    	{
			bm_print(BM_LOG_CRTI, "[oam_init] fg_vbat=(%d), vbat=(%d), set fg_vat as vat\n", gFG_voltage,vol_bat);
			
			gFG_voltage = vol_bat;
			gFG_capacity_by_v = vbat_capacity;
    	}   	
    }    
	
    gFG_capacity_by_v_init = gFG_capacity_by_v;

    dod_init();

    gFG_BATT_CAPACITY_aging = fgauge_get_Q_max(battery_meter_get_battery_temperature());

    //oam_v_ocv_1 = gFG_voltage;
    //oam_v_ocv_2 = gFG_voltage;

	
    oam_v_ocv_init = fgauge_read_v_by_d(gFG_DOD0);
    oam_v_ocv_2 = oam_v_ocv_1 = oam_v_ocv_init;
    g_vol_bat_hw_ocv = gFG_voltage;

    //vbat = 5; //set avg times
    //ret = battery_meter_ctrl(BATTERY_METER_CMD_GET_ADC_V_BAT_SENSE, &vbat);    
    //oam_r_1 = fgauge_read_r_bat_by_v(vbat);
    oam_r_1 = fgauge_read_r_bat_by_v(gFG_voltage);
    oam_r_2 = oam_r_1;
        
    oam_d0 = gFG_DOD0;
    oam_d_5 = oam_d0;
    oam_i_ori = gFG_current;
    g_d_hw_ocv = oam_d0;

    if(oam_init_i == 0)
    {
        bm_print(BM_LOG_CRTI, "[oam_init] oam_v_ocv_1,oam_v_ocv_2,oam_r_1,oam_r_2,oam_d0,oam_i_ori\n");
        oam_init_i=1;
    }

    bm_print(BM_LOG_CRTI, "[oam_init] %d,%d,%d,%d,%d,%d\n", 
        oam_v_ocv_1, oam_v_ocv_2, oam_r_1, oam_r_2, oam_d0, oam_i_ori);

	bm_print(BM_LOG_CRTI, "[oam_init_inf] hw_OCV, hw_D0, RTC, D0, oam_OCV_init, tbat\n");
	bm_print(BM_LOG_CRTI, "[oam_run_inf] oam_OCV1, oam_OCV2, vbat, I1, I2, R1, R2, Car1, Car2,qmax, tbat\n");
	bm_print(BM_LOG_CRTI, "[oam_result_inf] D1, D2, D3, D4, D5, UI_SOC\n");
	
	
	bm_print(BM_LOG_CRTI, "[oam_init_inf] %d, %d, %d, %d, %d, %d\n",
		gFG_voltage, (100 - fgauge_read_capacity_by_v(gFG_voltage)), g_rtc_fg_soc, gFG_DOD0 ,oam_v_ocv_init, force_get_tbat());
 
}


void oam_run(void)
{
    int vol_bat=0;
    //int vol_bat_hw_ocv=0;
    //int d_hw_ocv=0;
    int charging_current=0;
    int ret=0;
	//kal_uint32 now_time;
	struct timespec now_time;
	kal_int32 delta_time = 0;

	//now_time = rtc_read_hw_time();
	getrawmonotonic(&now_time);

	//delta_time = now_time - last_oam_run_time;
	delta_time = now_time.tv_sec - last_oam_run_time.tv_sec;

	bm_print(BM_LOG_FULL, "[oam_run_time] last time=%d, now time=%d, delta time=%d\n", 
	last_oam_run_time.tv_sec, now_time.tv_sec, delta_time);
	
	last_oam_run_time = now_time;

    // Reconstruct table if temp changed;
    fgauge_construct_table_by_temp();
		
    vol_bat = 15; //set avg times
    ret = battery_meter_ctrl(BATTERY_METER_CMD_GET_ADC_V_BAT_SENSE, &vol_bat);

    //ret = battery_meter_ctrl(BATTERY_METER_CMD_GET_HW_OCV, &vol_bat_hw_ocv);
    //d_hw_ocv = fgauge_read_d_by_v(vol_bat_hw_ocv);

    oam_i_1 = (((oam_v_ocv_1-vol_bat)*1000)*10) / oam_r_1;    //0.1mA
    oam_i_2 = (((oam_v_ocv_2-vol_bat)*1000)*10) / oam_r_2;    //0.1mA

    oam_car_1 = (oam_i_1*delta_time/3600) + oam_car_1; //0.1mAh
    oam_car_2 = (oam_i_2*delta_time/3600) + oam_car_2; //0.1mAh

    oam_d_1 = oam_d0 + (oam_car_1*100/10)/gFG_BATT_CAPACITY_aging;
    if(oam_d_1 < 0)   oam_d_1 = 0;
    if(oam_d_1 > 100) oam_d_1 = 100;
    
    oam_d_2 = oam_d0 + (oam_car_2*100/10)/gFG_BATT_CAPACITY_aging;
    if(oam_d_2 < 0)   oam_d_2 = 0;
    if(oam_d_2 > 100) oam_d_2 = 100;
    
    oam_v_ocv_1 = vol_bat + mtk_imp_tracking(vol_bat, oam_i_2, 5);
    
    oam_d_3 = fgauge_read_d_by_v(oam_v_ocv_1);        
    if(oam_d_3 < 0)   oam_d_3 = 0;
    if(oam_d_3 > 100) oam_d_3 = 100;

    oam_r_1 = fgauge_read_r_bat_by_v(oam_v_ocv_1);

    oam_v_ocv_2 = fgauge_read_v_by_d(oam_d_2);
    oam_r_2 = fgauge_read_r_bat_by_v(oam_v_ocv_2);    

#if 0
    oam_d_4 = (oam_d_2+oam_d_3)/2;
#else
    oam_d_4 = oam_d_3;
#endif

	gFG_columb = oam_car_2/10;	//mAh

    if( (oam_i_1 < 0) || (oam_i_2 < 0) )
        gFG_Is_Charging = KAL_TRUE;
    else
        gFG_Is_Charging = KAL_FALSE;

#if 0
    if(gFG_Is_Charging == KAL_FALSE)
    {
        d5_count_time = 60;         
    }
    else
    {
        charging_current = get_charging_setting_current();    
        charging_current = charging_current / 100;
        d5_count_time_rate = (((gFG_BATT_CAPACITY_aging*60*60/100/(charging_current-50))*10)+5)/10;
        
        if(d5_count_time_rate < 1)
            d5_count_time_rate = 1;

        d5_count_time = d5_count_time_rate;
    }
#else
    d5_count_time = 60;
#endif    

    if(d5_count >= d5_count_time)
    {
        if(gFG_Is_Charging == KAL_FALSE)
        {
            if( oam_d_3 > oam_d_5 )
            {
                oam_d_5 = oam_d_5 + 1;
            }
            else
            {                
                if(oam_d_4 > oam_d_5)
                {
                    oam_d_5 = oam_d_5 + 1;
                }
            }
        }
        else
        {            
            if( oam_d_5 > oam_d_3 )
            {
                oam_d_5 = oam_d_5 - 1;
            }
            else
            {                
                if(oam_d_4 < oam_d_5)
                {
                    oam_d_5 = oam_d_5 - 1;
                }
            }
        }
        d5_count = 0;
        oam_d_3_pre = oam_d_3;
        oam_d_4_pre = oam_d_4;
    }
    else
    {
        d5_count = d5_count + 10;
    }
    
    bm_print(BM_LOG_CRTI, "[oam_run] %d,%d,%d,%d,%d,%d,%d,%d\n", 
        d5_count, d5_count_time, oam_d_3_pre, oam_d_3, oam_d_4_pre, oam_d_4, oam_d_5, charging_current);    

    if(oam_run_i == 0)
    {
        bm_print(BM_LOG_FULL, "[oam_run] oam_i_1,oam_i_2,oam_car_1,oam_car_2,oam_d_1,oam_d_2,oam_v_ocv_1,oam_d_3,oam_r_1,oam_v_ocv_2,oam_r_2,vol_bat,g_vol_bat_hw_ocv,g_d_hw_ocv\n");
        oam_run_i=1;
    }    

    bm_print(BM_LOG_FULL, "[oam_run] %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n", 
            oam_i_1,oam_i_2,oam_car_1,oam_car_2,oam_d_1,oam_d_2,oam_v_ocv_1,oam_d_3,oam_r_1,oam_v_ocv_2,oam_r_2,vol_bat,g_vol_bat_hw_ocv,g_d_hw_ocv);    

    bm_print(BM_LOG_FULL, "[oam_total] %d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
            gFG_capacity_by_c, gFG_capacity_by_v, gfg_percent_check_point,
            oam_d_1, oam_d_2, oam_d_3, oam_d_4, oam_d_5, gFG_capacity_by_c_init, g_d_hw_ocv);

    bm_print(BM_LOG_CRTI, "[oam_total_s] %d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
        gFG_capacity_by_c,        // 1
        gFG_capacity_by_v,        // 2
        gfg_percent_check_point,  // 3
        (100-oam_d_1),            // 4
        (100-oam_d_2),            // 5
        (100-oam_d_3),            // 6
        (100-oam_d_4),            // 9
        (100-oam_d_5),            // 10
        gFG_capacity_by_c_init,   // 7
        (100-g_d_hw_ocv)          // 8
        );          

    bm_print(BM_LOG_FULL, "[oam_total_s_err] %d,%d,%d,%d,%d,%d,%d\n",
        (gFG_capacity_by_c - gFG_capacity_by_v), 
        (gFG_capacity_by_c - gfg_percent_check_point),
        (gFG_capacity_by_c - (100-oam_d_1)), 
        (gFG_capacity_by_c - (100-oam_d_2)), 
        (gFG_capacity_by_c - (100-oam_d_3)), 
        (gFG_capacity_by_c - (100-oam_d_4)), 
        (gFG_capacity_by_c - (100-oam_d_5))
        );

	bm_print(BM_LOG_CRTI, "[oam_init_inf] %d, %d, %d, %d, %d, %d\n",
		gFG_voltage, (100 - fgauge_read_capacity_by_v(gFG_voltage)), g_rtc_fg_soc, gFG_DOD0 ,oam_v_ocv_init, force_get_tbat());

	 bm_print(BM_LOG_CRTI, "[oam_run_inf] %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d\n", 
           oam_v_ocv_1,oam_v_ocv_2,vol_bat,oam_i_1,oam_i_2,oam_r_1,oam_r_2,oam_car_1,oam_car_2,gFG_BATT_CAPACITY_aging,force_get_tbat(), oam_d0);  

	  bm_print(BM_LOG_CRTI, "[oam_result_inf] %d, %d, %d, %d, %d, %d\n", 
           oam_d_1, oam_d_2, oam_d_3, oam_d_4, oam_d_5, BMT_status.UI_SOC);  
}

// ============================================================ //



void table_init(void)
{
    BATTERY_PROFILE_STRUC_P profile_p;
    R_PROFILE_STRUC_P profile_p_r_table;

    int temperature = force_get_tbat();
    
    // Re-constructure r-table profile according to current temperature
    profile_p_r_table = fgauge_get_profile_r_table(TEMPERATURE_T);
    if (profile_p_r_table == NULL)
    {
        bm_print(BM_LOG_CRTI, "[FGADC] fgauge_get_profile_r_table : create table fail !\r\n");
    }
    fgauge_construct_r_table_profile(temperature, profile_p_r_table);

    // Re-constructure battery profile according to current temperature
    profile_p = fgauge_get_profile(TEMPERATURE_T);
    if (profile_p == NULL)
    {
        bm_print(BM_LOG_CRTI, "[FGADC] fgauge_get_profile : create table fail !\r\n");
    }
    fgauge_construct_battery_profile(temperature, profile_p);
}

kal_int32 auxadc_algo_run(void)
{
    kal_int32 val=0;
    
    gFG_voltage = battery_meter_get_battery_voltage();
    val = fgauge_read_capacity_by_v(gFG_voltage);

    bm_print(BM_LOG_CRTI, "[auxadc_algo_run] %d,%d\n", gFG_voltage, val);
    
    return val;
}

#if defined(SOC_BY_HW_FG)
void update_fg_dbg_tool_value(void)
{
    g_fg_dbg_bat_volt = gFG_voltage_init;

    if (gFG_Is_Charging == KAL_TRUE) 
        g_fg_dbg_bat_current = 1 - gFG_current - 1;
    else
        g_fg_dbg_bat_current = gFG_current;

    g_fg_dbg_bat_zcv = gFG_voltage;

    g_fg_dbg_bat_temp = gFG_temp;

    g_fg_dbg_bat_r = gFG_resistance_bat;

    g_fg_dbg_bat_car = gFG_columb;

    g_fg_dbg_bat_qmax = gFG_BATT_CAPACITY_aging;

    g_fg_dbg_d0 = gFG_DOD0;

    g_fg_dbg_d1 = gFG_DOD1;

    g_fg_dbg_percentage = bat_get_ui_percentage();    

    g_fg_dbg_percentage_fg = gFG_capacity_by_c;

    g_fg_dbg_percentage_voltmode = gfg_percent_check_point;
}

kal_int32 fgauge_compensate_battery_voltage(kal_int32 ori_voltage)
{
    kal_int32 ret_compensate_value = 0;

    gFG_ori_voltage = ori_voltage;
    gFG_resistance_bat = fgauge_read_r_bat_by_v(ori_voltage); // Ohm
    ret_compensate_value = (gFG_current * (gFG_resistance_bat + R_FG_VALUE)) / 1000;
    ret_compensate_value = (ret_compensate_value+(10/2)) / 10;

    if (gFG_Is_Charging == KAL_TRUE) 
    {
        ret_compensate_value = ret_compensate_value - (ret_compensate_value*2);
    }

    gFG_compensate_value = ret_compensate_value;

    bm_print(BM_LOG_FULL, "[CompensateVoltage] Ori_voltage:%d, compensate_value:%d, gFG_resistance_bat:%d, gFG_current:%d\r\n", 
        ori_voltage, ret_compensate_value, gFG_resistance_bat, gFG_current);

    return ret_compensate_value;
}

kal_int32 fgauge_compensate_battery_voltage_recursion(kal_int32 ori_voltage, kal_int32 recursion_time)
{
    kal_int32 ret_compensate_value = 0;
    kal_int32 temp_voltage_1 = ori_voltage;
    kal_int32 temp_voltage_2 = temp_voltage_1;
    int i = 0;

    for(i=0 ; i < recursion_time ; i++) 
    {
        gFG_resistance_bat = fgauge_read_r_bat_by_v(temp_voltage_2); // Ohm
        ret_compensate_value = (gFG_current * (gFG_resistance_bat + R_FG_VALUE)) / 1000;        
        ret_compensate_value = (ret_compensate_value+(10/2)) / 10;
        
        if (gFG_Is_Charging == KAL_TRUE) 
        {
            ret_compensate_value = ret_compensate_value - (ret_compensate_value*2);
        }
        temp_voltage_2 = temp_voltage_1 + ret_compensate_value;

        bm_print(BM_LOG_FULL, "[fgauge_compensate_battery_voltage_recursion] %d,%d,%d,%d\r\n", 
            temp_voltage_1, temp_voltage_2, gFG_resistance_bat, ret_compensate_value);
    }
    
    gFG_resistance_bat = fgauge_read_r_bat_by_v(temp_voltage_2); // Ohm
    ret_compensate_value = (gFG_current * (gFG_resistance_bat + R_FG_VALUE + FG_METER_RESISTANCE)) / 1000;    
    ret_compensate_value = (ret_compensate_value+(10/2)) / 10; 
    
    if(gFG_Is_Charging == KAL_TRUE) 
    {
        ret_compensate_value = ret_compensate_value - (ret_compensate_value*2);
    }

    gFG_compensate_value = ret_compensate_value;

    bm_print(BM_LOG_FULL, "[fgauge_compensate_battery_voltage_recursion] %d,%d,%d,%d\r\n", 
        temp_voltage_1, temp_voltage_2, gFG_resistance_bat, ret_compensate_value);

    return ret_compensate_value;
}


kal_int32 fgauge_get_dod0(kal_int32 voltage, kal_int32 temperature, kal_bool bOcv)
{
    kal_int32 dod0 = 0;
    int i=0, saddles=0, jj=0;
    BATTERY_PROFILE_STRUC_P profile_p;
    R_PROFILE_STRUC_P profile_p_r_table;
    int ret=0;

/* R-Table (First Time) */    
    // Re-constructure r-table profile according to current temperature
    profile_p_r_table = fgauge_get_profile_r_table(TEMPERATURE_T);
    if (profile_p_r_table == NULL)
    {
        bm_print(BM_LOG_CRTI, "[FGADC] fgauge_get_profile_r_table : create table fail !\r\n");
    }
    fgauge_construct_r_table_profile(temperature, profile_p_r_table);

    // Re-constructure battery profile according to current temperature
    profile_p = fgauge_get_profile(TEMPERATURE_T);
    if (profile_p == NULL)
    {
        bm_print(BM_LOG_CRTI, "[FGADC] fgauge_get_profile : create table fail !\r\n");
        return 100;
    }
    fgauge_construct_battery_profile(temperature, profile_p);

    // Get total saddle points from the battery profile
    saddles = fgauge_get_saddles();

    // If the input voltage is not OCV, compensate to ZCV due to battery loading
    // Compasate battery voltage from current battery voltage
    jj=0;
    if (bOcv == KAL_FALSE)
    { 
        while( gFG_current == 0 )
        {            
            ret = battery_meter_ctrl(BATTERY_METER_CMD_GET_HW_FG_CURRENT, &gFG_current);
            if(jj > 10)
                break;
            jj++;
        }
        //voltage = voltage + fgauge_compensate_battery_voltage(voltage); //mV
        voltage = voltage + fgauge_compensate_battery_voltage_recursion(voltage,5); //mV
        bm_print(BM_LOG_CRTI, "[FGADC] compensate_battery_voltage, voltage=%d\r\n", voltage);
    }
    
    // If battery voltage is less then mimimum profile voltage, then return 100
    // If battery voltage is greater then maximum profile voltage, then return 0
    if (voltage > (profile_p+0)->voltage)
    {
        return 0;
    }    
    if (voltage < (profile_p+saddles-1)->voltage)
    {
        return 100;
    }

    // get DOD0 according to current temperature
    for (i = 0; i < saddles - 1; i++)
    {   
        if ((voltage <= (profile_p+i)->voltage) && (voltage >= (profile_p+i+1)->voltage))
        {
            dod0 = (profile_p+i)->percentage +
                (
                    (
                        ( ((profile_p+i)->voltage) - voltage ) * 
                        ( ((profile_p+i+1)->percentage) - ((profile_p + i)->percentage) ) 
                    ) /
                    ( ((profile_p+i)->voltage) - ((profile_p+i+1)->voltage) )
                );
            
            break;
        }
    }

    return dod0;
}


kal_int32 fgauge_update_dod(void)
{
    kal_int32 FG_dod_1 = 0;
    int adjust_coulomb_counter=CAR_TUNE_VALUE;

    if(gFG_DOD0 > 100)
    {
        gFG_DOD0=100;
        bm_print(BM_LOG_FULL, "[fgauge_update_dod] gFG_DOD0 set to 100, gFG_columb=%d\r\n", gFG_columb);
    }
    else if(gFG_DOD0 < 0)
    {
        gFG_DOD0=0;
        bm_print(BM_LOG_FULL, "[fgauge_update_dod] gFG_DOD0 set to 0, gFG_columb=%d\r\n", gFG_columb);
    }
    else
    {
    }    

    gFG_temp = battery_meter_get_battery_temperature();
    
    if(g_update_qmax_flag == 1)
    {
        gFG_BATT_CAPACITY = fgauge_get_Q_max(gFG_temp);
        bm_print(BM_LOG_CRTI, "[fgauge_update_dod] gFG_BATT_CAPACITY=%d, gFG_BATT_CAPACITY_aging=%d, gFG_BATT_CAPACITY_init_high_current=%d\r\n", 
            gFG_BATT_CAPACITY, gFG_BATT_CAPACITY_aging, gFG_BATT_CAPACITY_init_high_current);        
        g_update_qmax_flag = 0;
    }
    
    FG_dod_1 =  gFG_DOD0 - ((gFG_columb*100)/gFG_BATT_CAPACITY_aging);    
    
    bm_print(BM_LOG_FULL, "[fgauge_update_dod] FG_dod_1=%d, adjust_coulomb_counter=%d, gFG_columb=%d, gFG_DOD0=%d, gFG_temp=%d, gFG_BATT_CAPACITY=%d\r\n", 
            FG_dod_1, adjust_coulomb_counter, gFG_columb, gFG_DOD0, gFG_temp, gFG_BATT_CAPACITY);

    if(FG_dod_1 > 100)
    {
        FG_dod_1=100;
        bm_print(BM_LOG_FULL, "[fgauge_update_dod] FG_dod_1 set to 100, gFG_columb=%d\r\n", gFG_columb);
    }
    else if(FG_dod_1 < 0)
    {
        FG_dod_1=0;
        bm_print(BM_LOG_FULL, "[fgauge_update_dod] FG_dod_1 set to 0, gFG_columb=%d\r\n", gFG_columb);
    }
    else
    {
    }

    return FG_dod_1;
}


kal_int32 fgauge_read_capacity(kal_int32 type)
{
    kal_int32 voltage;
    kal_int32 temperature;
    kal_int32 dvalue = 0;
    
#ifndef CUST_DISABLE_CAPACITY_OCV2CV_TRANSFORM    
    kal_int32 C_0mA=0;
    kal_int32 C_400mA=0;
    kal_int32 dvalue_new=0;
#endif    
    
    kal_int32 temp_val = 0;

    if (type == 0) // for initialization
    {
        // Use voltage to calculate capacity
        voltage = battery_meter_get_battery_voltage(); // in unit of mV
        temperature = battery_meter_get_battery_temperature();                               
        dvalue = fgauge_get_dod0(voltage, temperature, KAL_FALSE); // need compensate vbat
    }
    else
    {
        // Use DOD0 and columb counter to calculate capacity
        dvalue = fgauge_update_dod(); // DOD1 = DOD0 + (-CAR)/Qmax
    }

    gFG_DOD1 = dvalue;

#ifndef CUST_DISABLE_CAPACITY_OCV2CV_TRANSFORM
    //User View on HT~LT----------------------------------------------------------
    gFG_temp = battery_meter_get_battery_temperature();
    C_0mA = fgauge_get_Q_max(gFG_temp);
    C_400mA = fgauge_get_Q_max_high_current(gFG_temp);
    if(C_0mA > C_400mA)
    {
        dvalue_new = (100-dvalue) - ( ( (C_0mA-C_400mA) * (dvalue) ) / C_400mA );
        dvalue = 100 - dvalue_new;
    }
    bm_print(BM_LOG_FULL, "[fgauge_read_capacity] %d,%d,%d,%d,%d,D1=%d,D0=%d\r\n", 
            gFG_temp, C_0mA, C_400mA, dvalue, dvalue_new, gFG_DOD1, gFG_DOD0);
    //----------------------------------------------------------------------------
#endif /* CUST_DISABLE_CAPACITY_OCV2CV_TRANSFORM */

    #if 0
    //Battery Aging update ----------------------------------------------------------
    dvalue_new = dvalue;
    dvalue = ( (dvalue_new * gFG_BATT_CAPACITY_init_high_current * 100) / gFG_BATT_CAPACITY_aging ) / 100;
    bm_print(BM_LOG_FULL, "[fgauge_read_capacity] dvalue=%d, dvalue_new=%d, gFG_BATT_CAPACITY_init_high_current=%d, gFG_BATT_CAPACITY_aging=%d\r\n", 
            dvalue, dvalue_new, gFG_BATT_CAPACITY_init_high_current, gFG_BATT_CAPACITY_aging);
    //----------------------------------------------------------------------------
    #endif

    temp_val = dvalue;
    dvalue = 100 - temp_val;

    if(dvalue <= 1)
    {
        dvalue=1;
        bm_print(BM_LOG_FULL, "[fgauge_read_capacity] dvalue<=1 and set dvalue=1 !!\r\n");
    }

    return dvalue;
}


void fg_voltage_mode(void)
{
#if defined(CONFIG_POWER_EXT)
#else
    if(bat_is_charger_exist() == KAL_TRUE)
    {
        /* SOC only UP when charging */
        if ( gFG_capacity_by_v > gfg_percent_check_point ) {                        
            gfg_percent_check_point++;
        }
    }
    else
    {
        /* SOC only Done when dis-charging */
        if ( gFG_capacity_by_v < gfg_percent_check_point ) {            
            gfg_percent_check_point--;
        }
    }
        
    bm_print(BM_LOG_FULL, "[FGADC_VoltageMothod] gFG_capacity_by_v=%ld,gfg_percent_check_point=%ld\r\n", 
        gFG_capacity_by_v, gfg_percent_check_point);
#endif    
}


void fgauge_algo_run(void)
{
    int i=0;
    int ret=0;

    // Reconstruct table if temp changed;
    fgauge_construct_table_by_temp();
    
//1. Get Raw Data  
    ret=battery_meter_ctrl(BATTERY_METER_CMD_GET_HW_FG_CURRENT, &gFG_current);
    ret=battery_meter_ctrl(BATTERY_METER_CMD_GET_HW_FG_CURRENT_SIGN, &gFG_Is_Charging);

    gFG_voltage = battery_meter_get_battery_voltage();
    gFG_voltage_init = gFG_voltage;
    gFG_voltage = gFG_voltage + fgauge_compensate_battery_voltage_recursion(gFG_voltage,5); //mV  
    gFG_voltage = gFG_voltage + OCV_BOARD_COMPESATE;

    ret = battery_meter_ctrl(BATTERY_METER_CMD_GET_HW_FG_CAR, &gFG_columb);

//1.1 Average FG_voltage
    /**************** Averaging : START ****************/
    if(gFG_voltage >= gFG_voltage_AVG)
    {
        gFG_vbat_offset = (gFG_voltage - gFG_voltage_AVG);
    }
    else
    {
        gFG_vbat_offset = (gFG_voltage_AVG - gFG_voltage);
    }

    if(gFG_vbat_offset <= MinErrorOffset)
    {
        FGbatteryVoltageSum -= FGvbatVoltageBuffer[FGbatteryIndex];
        FGbatteryVoltageSum += gFG_voltage;
        FGvbatVoltageBuffer[FGbatteryIndex] = gFG_voltage;
        
        gFG_voltage_AVG = FGbatteryVoltageSum / FG_VBAT_AVERAGE_SIZE;
        gFG_voltage = gFG_voltage_AVG;

        FGbatteryIndex++;
        if (FGbatteryIndex >= FG_VBAT_AVERAGE_SIZE)
            FGbatteryIndex = 0;

        bm_print(BM_LOG_FULL, "[FG_BUFFER] ");
        for (i=0; i<FG_VBAT_AVERAGE_SIZE; i++) {
            bm_print(BM_LOG_FULL, "%d,", FGvbatVoltageBuffer[i]);            
        }
        bm_print(BM_LOG_FULL, "\r\n");
    }
    else
    {
        bm_print(BM_LOG_FULL, "[FG] Over MinErrorOffset:V=%d,Avg_V=%d, ", gFG_voltage, gFG_voltage_AVG);
        
        gFG_voltage = gFG_voltage_AVG;
        
        bm_print(BM_LOG_FULL, "Avg_V need write back to V : V=%d,Avg_V=%d.\r\n", gFG_voltage, gFG_voltage_AVG);
    }

//2. Calculate battery capacity by VBAT    
    gFG_capacity_by_v = fgauge_read_capacity_by_v(gFG_voltage);

//3. Calculate battery capacity by Coulomb Counter
    gFG_capacity_by_c = fgauge_read_capacity(1);

//4. voltage mode
    if(volt_mode_update_timer >= volt_mode_update_time_out)
    {
        volt_mode_update_timer=0;

        fg_voltage_mode();
    }
    else
    {
        volt_mode_update_timer++;
    } 

//5. Logging
    bm_print(BM_LOG_CRTI, "[FGADC] %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n",
            gFG_Is_Charging,gFG_current,
            gFG_columb,gFG_voltage,gFG_capacity_by_v,gFG_capacity_by_c,gFG_capacity_by_c_init, 
            gFG_BATT_CAPACITY,gFG_BATT_CAPACITY_aging,gFG_compensate_value,gFG_ori_voltage,OCV_BOARD_COMPESATE,R_FG_BOARD_SLOPE,
            gFG_voltage_init,MinErrorOffset,gFG_DOD0,gFG_DOD1,
            CAR_TUNE_VALUE,AGING_TUNING_VALUE);            
    update_fg_dbg_tool_value();     
}

void fgauge_algo_run_init(void)
{
    int i=0;
    int ret=0;
    
//1. Get Raw Data  
    ret=battery_meter_ctrl(BATTERY_METER_CMD_GET_HW_FG_CURRENT, &gFG_current);
    ret=battery_meter_ctrl(BATTERY_METER_CMD_GET_HW_FG_CURRENT_SIGN, &gFG_Is_Charging);

    gFG_voltage = battery_meter_get_battery_voltage();
    gFG_voltage_init = gFG_voltage;
    gFG_voltage = gFG_voltage + fgauge_compensate_battery_voltage_recursion(gFG_voltage,5); //mV  
    gFG_voltage = gFG_voltage + OCV_BOARD_COMPESATE;

    ret = battery_meter_ctrl(BATTERY_METER_CMD_GET_HW_FG_CAR, &gFG_columb);

//1.1 Average FG_voltage
    for (i=0; i<FG_VBAT_AVERAGE_SIZE; i++) {
        FGvbatVoltageBuffer[i] = gFG_voltage;            
    }

    FGbatteryVoltageSum = gFG_voltage * FG_VBAT_AVERAGE_SIZE;
    gFG_voltage_AVG = gFG_voltage;
    
//2. Calculate battery capacity by VBAT    
    gFG_capacity_by_v = fgauge_read_capacity_by_v(gFG_voltage);
    gFG_capacity_by_v_init = gFG_capacity_by_v;
    
//3. Calculate battery capacity by Coulomb Counter
    gFG_capacity_by_c = fgauge_read_capacity(1);

//4. update DOD0 

    dod_init();        

    gFG_current_auto_detect_R_fg_count = 0;
        
    for(i=0;i<10;i++)
    {
        ret=battery_meter_ctrl(BATTERY_METER_CMD_GET_HW_FG_CURRENT, &gFG_current);
        
        gFG_current_auto_detect_R_fg_total+= gFG_current;
        gFG_current_auto_detect_R_fg_count++;            
    }

    //double check
    if(gFG_current_auto_detect_R_fg_total <= 0)
    {
        bm_print(BM_LOG_CRTI, "gFG_current_auto_detect_R_fg_total=0, need double check\n");
        
        gFG_current_auto_detect_R_fg_count = 0;
        
        for(i=0;i<10;i++)
        {
            ret=battery_meter_ctrl(BATTERY_METER_CMD_GET_HW_FG_CURRENT, &gFG_current);
            
            gFG_current_auto_detect_R_fg_total+= gFG_current;
            gFG_current_auto_detect_R_fg_count++;            
        }
    }

    gFG_current_auto_detect_R_fg_result = gFG_current_auto_detect_R_fg_total / gFG_current_auto_detect_R_fg_count;
    if(gFG_current_auto_detect_R_fg_result <= CURRENT_DETECT_R_FG)
    {
        g_auxadc_solution=1;  
       
        bm_print(BM_LOG_CRTI, "[FGADC] Detect NO Rfg, use AUXADC report. (%d=%d/%d)(%d)\r\n", 
            gFG_current_auto_detect_R_fg_result, gFG_current_auto_detect_R_fg_total,
            gFG_current_auto_detect_R_fg_count, g_auxadc_solution);            
    }
    else
    {
        if(g_auxadc_solution == 0)
        {
            g_auxadc_solution=0;
    
            bm_print(BM_LOG_CRTI, "[FGADC] Detect Rfg, use FG report. (%d=%d/%d)(%d)\r\n", 
            gFG_current_auto_detect_R_fg_result, gFG_current_auto_detect_R_fg_total,
            gFG_current_auto_detect_R_fg_count, g_auxadc_solution);
        }
        else
        {
            bm_print(BM_LOG_CRTI, "[FGADC] Detect Rfg, but use AUXADC report. due to g_auxadc_solution=%d \r\n", 
                g_auxadc_solution);
        }
    }    
    
//5. Logging
    bm_print(BM_LOG_CRTI, "[FGADC] %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n",
            gFG_Is_Charging,gFG_current,
            gFG_columb,gFG_voltage,gFG_capacity_by_v,gFG_capacity_by_c,gFG_capacity_by_c_init, 
            gFG_BATT_CAPACITY,gFG_BATT_CAPACITY_aging,gFG_compensate_value,gFG_ori_voltage,OCV_BOARD_COMPESATE,R_FG_BOARD_SLOPE,
            gFG_voltage_init,MinErrorOffset,gFG_DOD0,gFG_DOD1,
            CAR_TUNE_VALUE,AGING_TUNING_VALUE);            
    update_fg_dbg_tool_value();    
}

void fgauge_initialization(void)
{
#if defined(CONFIG_POWER_EXT)
#else
    int i = 0;
    kal_uint32 ret=0;

    //gFG_BATT_CAPACITY_init_high_current = fgauge_get_Q_max_high_current(25);    
    //gFG_BATT_CAPACITY_aging = fgauge_get_Q_max(25);

    // 1. HW initialization
    ret = battery_meter_ctrl(BATTERY_METER_CMD_HW_FG_INIT, NULL);
    
    // 2. SW algorithm initialization           
    ret = battery_meter_ctrl(BATTERY_METER_CMD_GET_HW_OCV, &gFG_voltage);

    ret = battery_meter_ctrl(BATTERY_METER_CMD_GET_HW_FG_CURRENT, &gFG_current);
    i=0;
    while( gFG_current == 0 )
    {        
        ret = battery_meter_ctrl(BATTERY_METER_CMD_GET_HW_FG_CURRENT, &gFG_current);        
        if(i > 10)
        {
            bm_print(BM_LOG_CRTI, "[fgauge_initialization] gFG_current == 0\n");
            break;
        }
        i++;
    }

    ret = battery_meter_ctrl(BATTERY_METER_CMD_GET_HW_FG_CAR, &gFG_columb);
    gFG_temp = battery_meter_get_battery_temperature();
    gFG_capacity = fgauge_read_capacity(0);         

    gFG_capacity_by_c_init = gFG_capacity;
    gFG_capacity_by_c = gFG_capacity;
    gFG_capacity_by_v = gFG_capacity;

    gFG_DOD0 = 100 - gFG_capacity;

    gFG_BATT_CAPACITY = fgauge_get_Q_max(gFG_temp);

    gFG_BATT_CAPACITY_init_high_current = fgauge_get_Q_max_high_current(gFG_temp);    
    gFG_BATT_CAPACITY_aging = fgauge_get_Q_max(gFG_temp);

    ret = battery_meter_ctrl(BATTERY_METER_CMD_DUMP_REGISTER, NULL);
    
    bm_print(BM_LOG_CRTI, "[fgauge_initialization] Done\n");
#endif
}
#endif

kal_int32 get_dynamic_period(int first_use, int first_wakeup_time, int battery_capacity_level)
{
#if defined(CONFIG_POWER_EXT)

    return first_wakeup_time;

#elif defined(SOC_BY_AUXADC) ||  defined(SOC_BY_SW_FG)

    kal_int32 vbat_val = 0;

    vbat_val = g_sw_vbat_temp;    
    
    // change wake up period when system suspend.
    if(vbat_val > VBAT_NORMAL_WAKEUP)				//3.6v
        g_spm_timer = NORMAL_WAKEUP_PERIOD; // 90 min
    else if(vbat_val > VBAT_LOW_POWER_WAKEUP)     //3.5v   
        g_spm_timer = LOW_POWER_WAKEUP_PERIOD; // 5 min
    else
        g_spm_timer = CLOSE_POWEROFF_WAKEUP_PERIOD; // 0.5 min



    bm_print(BM_LOG_CRTI, "vbat_val=%d, g_spm_timer=%d\n", vbat_val, g_spm_timer);

    return g_spm_timer;    
#else

    kal_int32 car_instant=0;
    kal_int32 current_instant=0;
    static kal_int32 car_sleep=0;
    kal_int32 car_wakeup=0;
    static kal_int32 last_time=0;

    kal_int32 ret_val=-1;
    int check_fglog=0;
    kal_int32 I_sleep=0;
    kal_int32 new_time=0;

    int ret=0;

    check_fglog=Enable_FGADC_LOG;
    if(check_fglog==0)
    {
        //Enable_FGADC_LOG=1;
    }

    ret = battery_meter_ctrl(BATTERY_METER_CMD_GET_HW_FG_CURRENT, &current_instant);
    
    ret = battery_meter_ctrl(BATTERY_METER_CMD_GET_HW_FG_CAR, &car_instant);
    
    if(check_fglog==0)
    {
        //Enable_FGADC_LOG=0;
    }
    if(car_instant < 0)
    {
        car_instant = car_instant - (car_instant*2);
    }
    
    if(first_use == 1)
    {
        //ret_val = 30*60; /* 30 mins */
        ret_val = first_wakeup_time; 
        last_time = ret_val;        
        car_sleep = car_instant;
    }
    else
    {
        car_wakeup = car_instant;

        if(last_time==0)
            last_time=1;    

        if(car_sleep > car_wakeup)
        {
            car_sleep = car_wakeup;
            bm_print(BM_LOG_FULL, "[get_dynamic_period] reset car_sleep\n");
        }

        I_sleep = ((car_wakeup-car_sleep)*3600)/last_time; // unit: second

        if(I_sleep==0)
        {
            if(check_fglog==0)
            {
                //Enable_FGADC_LOG=1;
            }
            
            ret = battery_meter_ctrl(BATTERY_METER_CMD_GET_HW_FG_CURRENT, &I_sleep);
            
            I_sleep = I_sleep / 10;
            if(check_fglog==0)
            {
                //Enable_FGADC_LOG=0;
            }
        }
        
        if(I_sleep == 0)
        {
            new_time = first_wakeup_time;
        }
        else
        {
            new_time = ((gFG_BATT_CAPACITY*battery_capacity_level*3600)/100)/I_sleep;
        }        
        ret_val = new_time;

        if(ret_val == 0)
            ret_val = first_wakeup_time;

        bm_print(BM_LOG_CRTI, "[get_dynamic_period] car_instant=%d, car_wakeup=%d, car_sleep=%d, I_sleep=%d, gFG_BATT_CAPACITY=%d, last_time=%d, new_time=%d\r\n", 
            car_instant, car_wakeup, car_sleep, I_sleep, gFG_BATT_CAPACITY, last_time, new_time);        
        
        //update parameter
        car_sleep = car_wakeup;
        last_time = ret_val;
    }
    return ret_val;
    
#endif    
}

// ============================================================ //
kal_int32 battery_meter_get_battery_voltage(void)
{
    int ret=0;
    int val=5;
    
    val = 5; //set avg times
    ret = battery_meter_ctrl(BATTERY_METER_CMD_GET_ADC_V_BAT_SENSE, &val);

    g_sw_vbat_temp = val;

    return val;
}

kal_int32 battery_meter_get_charging_current(void)
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
    int ret=0;
    int val=1;

    for(i=0 ; i<repeat ; i++)
    {
        val = 1; //set avg times
        ret = battery_meter_ctrl(BATTERY_METER_CMD_GET_ADC_V_BAT_SENSE, &val);
        ADC_BAT_SENSE_tmp[i] = val;

        val = 1; //set avg times
        ret = battery_meter_ctrl(BATTERY_METER_CMD_GET_ADC_V_I_SENSE, &val);
        ADC_I_SENSE_tmp[i] = val;
    
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

    bm_print(BM_LOG_FULL, "[g_Get_I_Charging:BAT_SENSE]\r\n");    
    for(i=0 ; i<repeat ; i++ )
    {
        bm_print(BM_LOG_FULL, "%d,", ADC_BAT_SENSE_tmp[i]);
    }
    bm_print(BM_LOG_FULL, "\r\n");

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

    bm_print(BM_LOG_FULL, "[g_Get_I_Charging:I_SENSE]\r\n");    
    for(i=0 ; i<repeat ; i++ )
    {
        bm_print(BM_LOG_FULL, "%d,", ADC_I_SENSE_tmp[i]);
    }
    bm_print(BM_LOG_FULL, "\r\n");
        
    ADC_BAT_SENSE_sum -= ADC_BAT_SENSE_tmp[0];
    ADC_BAT_SENSE_sum -= ADC_BAT_SENSE_tmp[1];
    ADC_BAT_SENSE_sum -= ADC_BAT_SENSE_tmp[18];
    ADC_BAT_SENSE_sum -= ADC_BAT_SENSE_tmp[19];        
    ADC_BAT_SENSE = ADC_BAT_SENSE_sum / (repeat-4);

    bm_print(BM_LOG_FULL, "[g_Get_I_Charging] ADC_BAT_SENSE=%d\r\n", ADC_BAT_SENSE);

    ADC_I_SENSE_sum -= ADC_I_SENSE_tmp[0];
    ADC_I_SENSE_sum -= ADC_I_SENSE_tmp[1];
    ADC_I_SENSE_sum -= ADC_I_SENSE_tmp[18];
    ADC_I_SENSE_sum -= ADC_I_SENSE_tmp[19];
    ADC_I_SENSE = ADC_I_SENSE_sum / (repeat-4);

    bm_print(BM_LOG_FULL, "[g_Get_I_Charging] ADC_I_SENSE(Before)=%d\r\n", ADC_I_SENSE);
    

    bm_print(BM_LOG_FULL, "[g_Get_I_Charging] ADC_I_SENSE(After)=%d\r\n", ADC_I_SENSE);
    
    if(ADC_I_SENSE > ADC_BAT_SENSE)
    {
        ICharging = (ADC_I_SENSE - ADC_BAT_SENSE + g_I_SENSE_offset)*1000/CUST_R_SENSE;
    }
    else
    {
        ICharging = 0;
    }

    return ICharging;
}

kal_int32 battery_meter_get_battery_current(void)
{
    int ret=0;
    kal_int32 val=0;

	if(g_auxadc_solution == 1)
	    val = oam_i_2;
	else
    	    ret = battery_meter_ctrl(BATTERY_METER_CMD_GET_HW_FG_CURRENT, &val);

    return val;
}

kal_bool battery_meter_get_battery_current_sign(void)
{
    int ret=0;
    kal_bool val=0;

	if(g_auxadc_solution == 1)
	    val=0;	//discharging
	else
    	    ret = battery_meter_ctrl(BATTERY_METER_CMD_GET_HW_FG_CURRENT_SIGN, &val);

    return val;
}

kal_int32 battery_meter_get_car(void)
{
    int ret=0;
    kal_int32 val=0;

	if(g_auxadc_solution == 1)
    	    val = oam_car_2;
	else
    	    ret = battery_meter_ctrl(BATTERY_METER_CMD_GET_HW_FG_CAR, &val);

    return val;
}

kal_int32 battery_meter_get_battery_temperature(void)
{
    return force_get_tbat();
}

kal_int32 battery_meter_get_charger_voltage(void)
{
    int ret=0;
    int val=0;
    
    val = 5; // set avg times
    ret = battery_meter_ctrl(BATTERY_METER_CMD_GET_ADC_V_CHARGER, &val);

    //val = (((R_CHARGER_1+R_CHARGER_2)*100*val)/R_CHARGER_2)/100;
    return val;
}

kal_int32 battery_meter_get_battery_percentage(void)
{
#if defined(CONFIG_POWER_EXT)
    return 50;
#else

    if(bat_is_charger_exist() == KAL_FALSE)
        fg_qmax_update_for_aging_flag = 1;

    #if defined(SOC_BY_AUXADC)
    return auxadc_algo_run();
    #endif
    
    #if defined(SOC_BY_HW_FG)
    if(g_auxadc_solution == 1)
    {
        return auxadc_algo_run();
    }
    else    
    {
        fgauge_algo_run();    
        return gFG_capacity_by_c; // hw fg, //return gfg_percent_check_point; // voltage mode    
    }
    #endif
    
    #if defined(SOC_BY_SW_FG)
    oam_run();    
        #if (OAM_D5 == 1)
            return (100-oam_d_5);
        #else
            return (100-oam_d_2);
        #endif
    #endif

#endif
}


kal_int32 battery_meter_initial(void)
{
#if defined(CONFIG_POWER_EXT)
    return 0;
#else

#ifdef MTK_MULTI_BAT_PROFILE_SUPPORT
    fgauge_get_profile_id();
#endif

    #if defined(SOC_BY_AUXADC)
    g_auxadc_solution = 1;
    table_init();
    bm_print(BM_LOG_CRTI, "[battery_meter_initial] SOC_BY_AUXADC done\n");
    #endif

    #if defined(SOC_BY_HW_FG)
    fgauge_initialization();
    fgauge_algo_run_init();
    bm_print(BM_LOG_CRTI, "[battery_meter_initial] SOC_BY_HW_FG done\n");
    #endif

    #if defined(SOC_BY_SW_FG)
    g_auxadc_solution = 1;
    table_init();
    oam_init();
    bm_print(BM_LOG_CRTI, "[battery_meter_initial] SOC_BY_SW_FG done\n");
    #endif

    return 0;
#endif
}

void reset_parameter_car(void)
{
    #if defined(SOC_BY_HW_FG)
    int ret = 0;
    ret = battery_meter_ctrl(BATTERY_METER_CMD_HW_RESET, NULL);    
    gFG_columb = 0;
    #endif

    #if defined(SOC_BY_SW_FG)
    oam_car_1 = 0;
    oam_car_2 = 0;
    gFG_columb = 0;
    #endif
}

void reset_parameter_dod_change(void)
{
    #if defined(SOC_BY_HW_FG)
    bm_print(BM_LOG_CRTI, "[FGADC] Update DOD0(%d) by %d \r\n", gFG_DOD0, gFG_DOD1);
  	gFG_DOD0 = gFG_DOD1;
    #endif

    #if defined(SOC_BY_SW_FG)
    bm_print(BM_LOG_CRTI, "[FGADC] Update oam_d0(%d) by %d \r\n", oam_d0, oam_d_5);
	oam_d0 = oam_d_5;
    gFG_DOD0 = oam_d0;
    oam_d_1 = oam_d_5;
    oam_d_2 = oam_d_5;
    oam_d_3 = oam_d_5;
    oam_d_4 = oam_d_5;
    #endif
}

void reset_parameter_dod_full(kal_uint32 ui_percentage)
{
    #if defined(SOC_BY_HW_FG)
    bm_print(BM_LOG_CRTI, "[battery_meter_reset]1 DOD0=%d,DOD1=%d,ui=%d\n", gFG_DOD0, gFG_DOD1, ui_percentage);
    gFG_DOD0 = 100 - ui_percentage;
    gFG_DOD1 = gFG_DOD0;
	bm_print(BM_LOG_CRTI, "[battery_meter_reset]2 DOD0=%d,DOD1=%d,ui=%d\n", gFG_DOD0, gFG_DOD1, ui_percentage);
    #endif

    #if defined(SOC_BY_SW_FG)
    bm_print(BM_LOG_CRTI, "[battery_meter_reset]1 oam_d0=%d,oam_d_5=%d,ui=%d\n", oam_d0, oam_d_5, ui_percentage);
	oam_d0 = 100 - ui_percentage;
    gFG_DOD0 = oam_d0;
    gFG_DOD1 = oam_d0;
    oam_d_1 = oam_d0;
    oam_d_2 = oam_d0;
    oam_d_3 = oam_d0;
    oam_d_4 = oam_d0;
    oam_d_5 = oam_d0;
	bm_print(BM_LOG_CRTI, "[battery_meter_reset]2 oam_d0=%d,oam_d_5=%d,ui=%d\n", oam_d0, oam_d_5, ui_percentage);
    #endif
}

kal_int32 battery_meter_reset(void)
{
#if defined(CONFIG_POWER_EXT)
    return 0;
#else
    kal_uint32 ui_percentage = bat_get_ui_percentage();

    if(bat_is_charging_full() == KAL_TRUE) // charge full
    {
        if(fg_qmax_update_for_aging_flag == 1)
        {
            fg_qmax_update_for_aging();
            fg_qmax_update_for_aging_flag=0;
        }
    }	
		
    reset_parameter_car();
    reset_parameter_dod_full(ui_percentage);
    
    return 0;
#endif
}

kal_int32 battery_meter_sync(kal_int32 bat_i_sense_offset)
{
#if defined(CONFIG_POWER_EXT)
    return 0;
#else
    g_I_SENSE_offset = bat_i_sense_offset;
    return 0;
#endif	
}

kal_int32 battery_meter_get_battery_zcv(void)
{
#if defined(CONFIG_POWER_EXT)
    return 3987;
#else
    return gFG_voltage;
#endif	
}

kal_int32 battery_meter_get_battery_nPercent_zcv(void)    
{
#if defined(CONFIG_POWER_EXT)
    return 3700;
#else
    return gFG_15_vlot; // 15% zcv,  15% can be customized by 100-g_tracking_point
#endif	
}

kal_int32 battery_meter_get_battery_nPercent_UI_SOC(void) 
{
#if defined(CONFIG_POWER_EXT)
    return 15;
#else
    return g_tracking_point; // tracking point
#endif	
}

kal_int32 battery_meter_get_tempR(kal_int32 dwVolt)
{
#if defined(CONFIG_POWER_EXT)
    return 0;
#else
    int TRes;
      
    TRes = (RBAT_PULL_UP_R*dwVolt) / (RBAT_PULL_UP_VOLT-dwVolt);        
    
    return TRes;
#endif
}

kal_int32 battery_meter_get_tempV(void)
{
#if defined(CONFIG_POWER_EXT)
    return 0;
#else
    int ret=0;
		int val=0;
		
		val = 1; // set avg times
    ret = battery_meter_ctrl(BATTERY_METER_CMD_GET_ADC_V_BAT_TEMP, &val);
    return val;
#endif    
}

kal_int32 battery_meter_get_VSense(void)
{
#if defined(CONFIG_POWER_EXT)
    return 0;
#else
		int ret=0;
		int val=0;
		
		val = 1; //set avg times
    ret = battery_meter_ctrl(BATTERY_METER_CMD_GET_ADC_V_I_SENSE, &val);
    return val;
#endif
}

// ============================================================ //
static ssize_t fgadc_log_write( struct file *filp, const char __user *buff,
                        size_t len, loff_t *data )
{
    if (copy_from_user( &proc_fgadc_data, buff, len )) {
        bm_print(BM_LOG_CRTI, "fgadc_log_write error.\n");
        return -EFAULT;
    }

    if (proc_fgadc_data[0] == '1') {
        bm_print(BM_LOG_CRTI, "enable FGADC driver log system\n");
        Enable_FGADC_LOG = 1;
    } else if (proc_fgadc_data[0] == '2') {
        bm_print(BM_LOG_CRTI, "enable FGADC driver log system:2\n");
        Enable_FGADC_LOG = 2;    
    } else {
        bm_print(BM_LOG_CRTI, "Disable FGADC driver log system\n");
        Enable_FGADC_LOG = 0;
    }
    
    return len;
}

static const struct file_operations fgadc_proc_fops = { 
    .write = fgadc_log_write,
};

int init_proc_log_fg(void)
{
    int ret=0;

#if 1
    proc_create("fgadc_log", 0644, NULL, &fgadc_proc_fops);
    bm_print(BM_LOG_CRTI, "proc_create fgadc_proc_fops\n");
#else
    proc_entry_fgadc = create_proc_entry( "fgadc_log", 0644, NULL );
    
    if (proc_entry_fgadc == NULL) {
        ret = -ENOMEM;
        bm_print(BM_LOG_CRTI, "init_proc_log_fg: Couldn't create proc entry\n");
    } else {
        proc_entry_fgadc->write_proc = fgadc_log_write;
        bm_print(BM_LOG_CRTI, "init_proc_log_fg loaded.\n");
    }
#endif
  
    return ret;
}

// ============================================================ //
static ssize_t show_FG_Current(struct device *dev,struct device_attribute *attr, char *buf)
{
    kal_int32 ret=0;
    kal_int32 fg_current_inout_battery = 0;
    kal_int32 val=0;
    kal_bool is_charging=0;
    
    ret = battery_meter_ctrl(BATTERY_METER_CMD_GET_HW_FG_CURRENT, &val);
    ret = battery_meter_ctrl(BATTERY_METER_CMD_GET_HW_FG_CURRENT_SIGN, &is_charging);
    
    if(is_charging == KAL_TRUE)
    {
        fg_current_inout_battery = 0 - val;
    }
    else
    {
        fg_current_inout_battery = val;
    }
    
    bm_print(BM_LOG_CRTI, "[FG] gFG_current_inout_battery : %d\n", fg_current_inout_battery);
    return sprintf(buf, "%d\n", fg_current_inout_battery);
}
static ssize_t store_FG_Current(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
    return size;
}
static DEVICE_ATTR(FG_Current, 0664, show_FG_Current, store_FG_Current);

// ============================================================ //
static ssize_t show_FG_g_fg_dbg_bat_volt(struct device *dev,struct device_attribute *attr, char *buf)
{    
    bm_print(BM_LOG_CRTI, "[FG] g_fg_dbg_bat_volt : %d\n", g_fg_dbg_bat_volt);
    return sprintf(buf, "%d\n", g_fg_dbg_bat_volt);
}
static ssize_t store_FG_g_fg_dbg_bat_volt(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
    return size;
}
static DEVICE_ATTR(FG_g_fg_dbg_bat_volt, 0664, show_FG_g_fg_dbg_bat_volt, store_FG_g_fg_dbg_bat_volt);
//-------------------------------------------------------------------------------------------
static ssize_t show_FG_g_fg_dbg_bat_current(struct device *dev,struct device_attribute *attr, char *buf)
{    
    bm_print(BM_LOG_CRTI, "[FG] g_fg_dbg_bat_current : %d\n", g_fg_dbg_bat_current);
    return sprintf(buf, "%d\n", g_fg_dbg_bat_current);
}
static ssize_t store_FG_g_fg_dbg_bat_current(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
    return size;
}
static DEVICE_ATTR(FG_g_fg_dbg_bat_current, 0664, show_FG_g_fg_dbg_bat_current, store_FG_g_fg_dbg_bat_current);
//-------------------------------------------------------------------------------------------
static ssize_t show_FG_g_fg_dbg_bat_zcv(struct device *dev,struct device_attribute *attr, char *buf)
{    
    bm_print(BM_LOG_CRTI, "[FG] g_fg_dbg_bat_zcv : %d\n", g_fg_dbg_bat_zcv);
    return sprintf(buf, "%d\n", g_fg_dbg_bat_zcv);
}
static ssize_t store_FG_g_fg_dbg_bat_zcv(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
    return size;
}
static DEVICE_ATTR(FG_g_fg_dbg_bat_zcv, 0664, show_FG_g_fg_dbg_bat_zcv, store_FG_g_fg_dbg_bat_zcv);
//-------------------------------------------------------------------------------------------
static ssize_t show_FG_g_fg_dbg_bat_temp(struct device *dev,struct device_attribute *attr, char *buf)
{    
    bm_print(BM_LOG_CRTI, "[FG] g_fg_dbg_bat_temp : %d\n", g_fg_dbg_bat_temp);
    return sprintf(buf, "%d\n", g_fg_dbg_bat_temp);
}
static ssize_t store_FG_g_fg_dbg_bat_temp(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
    return size;
}
static DEVICE_ATTR(FG_g_fg_dbg_bat_temp, 0664, show_FG_g_fg_dbg_bat_temp, store_FG_g_fg_dbg_bat_temp);
//-------------------------------------------------------------------------------------------
static ssize_t show_FG_g_fg_dbg_bat_r(struct device *dev,struct device_attribute *attr, char *buf)
{    
    bm_print(BM_LOG_CRTI, "[FG] g_fg_dbg_bat_r : %d\n", g_fg_dbg_bat_r);
    return sprintf(buf, "%d\n", g_fg_dbg_bat_r);
}
static ssize_t store_FG_g_fg_dbg_bat_r(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
    return size;
}
static DEVICE_ATTR(FG_g_fg_dbg_bat_r, 0664, show_FG_g_fg_dbg_bat_r, store_FG_g_fg_dbg_bat_r);
//-------------------------------------------------------------------------------------------
static ssize_t show_FG_g_fg_dbg_bat_car(struct device *dev,struct device_attribute *attr, char *buf)
{    
    bm_print(BM_LOG_CRTI, "[FG] g_fg_dbg_bat_car : %d\n", g_fg_dbg_bat_car);
    return sprintf(buf, "%d\n", g_fg_dbg_bat_car);
}
static ssize_t store_FG_g_fg_dbg_bat_car(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
    return size;
}
static DEVICE_ATTR(FG_g_fg_dbg_bat_car, 0664, show_FG_g_fg_dbg_bat_car, store_FG_g_fg_dbg_bat_car);
//-------------------------------------------------------------------------------------------
static ssize_t show_FG_g_fg_dbg_bat_qmax(struct device *dev,struct device_attribute *attr, char *buf)
{    
    bm_print(BM_LOG_CRTI, "[FG] g_fg_dbg_bat_qmax : %d\n", g_fg_dbg_bat_qmax);
    return sprintf(buf, "%d\n", g_fg_dbg_bat_qmax);
}
static ssize_t store_FG_g_fg_dbg_bat_qmax(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
    return size;
}
static DEVICE_ATTR(FG_g_fg_dbg_bat_qmax, 0664, show_FG_g_fg_dbg_bat_qmax, store_FG_g_fg_dbg_bat_qmax);
//-------------------------------------------------------------------------------------------
static ssize_t show_FG_g_fg_dbg_d0(struct device *dev,struct device_attribute *attr, char *buf)
{    
    bm_print(BM_LOG_CRTI, "[FG] g_fg_dbg_d0 : %d\n", g_fg_dbg_d0);
    return sprintf(buf, "%d\n", g_fg_dbg_d0);
}
static ssize_t store_FG_g_fg_dbg_d0(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
    return size;
}
static DEVICE_ATTR(FG_g_fg_dbg_d0, 0664, show_FG_g_fg_dbg_d0, store_FG_g_fg_dbg_d0);
//-------------------------------------------------------------------------------------------
static ssize_t show_FG_g_fg_dbg_d1(struct device *dev,struct device_attribute *attr, char *buf)
{    
    bm_print(BM_LOG_CRTI, "[FG] g_fg_dbg_d1 : %d\n", g_fg_dbg_d1);
    return sprintf(buf, "%d\n", g_fg_dbg_d1);
}
static ssize_t store_FG_g_fg_dbg_d1(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
    return size;
}
static DEVICE_ATTR(FG_g_fg_dbg_d1, 0664, show_FG_g_fg_dbg_d1, store_FG_g_fg_dbg_d1);
//-------------------------------------------------------------------------------------------
static ssize_t show_FG_g_fg_dbg_percentage(struct device *dev,struct device_attribute *attr, char *buf)
{    
    bm_print(BM_LOG_CRTI, "[FG] g_fg_dbg_percentage : %d\n", g_fg_dbg_percentage);
    return sprintf(buf, "%d\n", g_fg_dbg_percentage);
}
static ssize_t store_FG_g_fg_dbg_percentage(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
    return size;
}
static DEVICE_ATTR(FG_g_fg_dbg_percentage, 0664, show_FG_g_fg_dbg_percentage, store_FG_g_fg_dbg_percentage);
//-------------------------------------------------------------------------------------------
static ssize_t show_FG_g_fg_dbg_percentage_fg(struct device *dev,struct device_attribute *attr, char *buf)
{    
    bm_print(BM_LOG_CRTI, "[FG] g_fg_dbg_percentage_fg : %d\n", g_fg_dbg_percentage_fg);
    return sprintf(buf, "%d\n", g_fg_dbg_percentage_fg);
}
static ssize_t store_FG_g_fg_dbg_percentage_fg(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
    return size;
}
static DEVICE_ATTR(FG_g_fg_dbg_percentage_fg, 0664, show_FG_g_fg_dbg_percentage_fg, store_FG_g_fg_dbg_percentage_fg);
//-------------------------------------------------------------------------------------------
static ssize_t show_FG_g_fg_dbg_percentage_voltmode(struct device *dev,struct device_attribute *attr, char *buf)
{    
    bm_print(BM_LOG_CRTI, "[FG] g_fg_dbg_percentage_voltmode : %d\n", g_fg_dbg_percentage_voltmode);
    return sprintf(buf, "%d\n", g_fg_dbg_percentage_voltmode);
}
static ssize_t store_FG_g_fg_dbg_percentage_voltmode(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
    return size;
}
static DEVICE_ATTR(FG_g_fg_dbg_percentage_voltmode, 0664, show_FG_g_fg_dbg_percentage_voltmode, store_FG_g_fg_dbg_percentage_voltmode);

// ============================================================ //
static int battery_meter_probe(struct platform_device *dev)    
{
    int ret_device_file = 0;

	battery_meter_ctrl = bm_ctrl_cmd;

    bm_print(BM_LOG_CRTI, "[battery_meter_probe] probe\n");
    //select battery meter control method
     battery_meter_ctrl = bm_ctrl_cmd;
    //LOG System Set
    init_proc_log_fg();

	//last_oam_run_time = rtc_read_hw_time(); 
	getrawmonotonic(&last_oam_run_time);
    //Create File For FG UI DEBUG
    ret_device_file = device_create_file(&(dev->dev), &dev_attr_FG_Current);    
    ret_device_file = device_create_file(&(dev->dev), &dev_attr_FG_g_fg_dbg_bat_volt);
    ret_device_file = device_create_file(&(dev->dev), &dev_attr_FG_g_fg_dbg_bat_current);
    ret_device_file = device_create_file(&(dev->dev), &dev_attr_FG_g_fg_dbg_bat_zcv);
    ret_device_file = device_create_file(&(dev->dev), &dev_attr_FG_g_fg_dbg_bat_temp);
    ret_device_file = device_create_file(&(dev->dev), &dev_attr_FG_g_fg_dbg_bat_r);
    ret_device_file = device_create_file(&(dev->dev), &dev_attr_FG_g_fg_dbg_bat_car);
    ret_device_file = device_create_file(&(dev->dev), &dev_attr_FG_g_fg_dbg_bat_qmax);
    ret_device_file = device_create_file(&(dev->dev), &dev_attr_FG_g_fg_dbg_d0);
    ret_device_file = device_create_file(&(dev->dev), &dev_attr_FG_g_fg_dbg_d1);
    ret_device_file = device_create_file(&(dev->dev), &dev_attr_FG_g_fg_dbg_percentage);
    ret_device_file = device_create_file(&(dev->dev), &dev_attr_FG_g_fg_dbg_percentage_fg);
    ret_device_file = device_create_file(&(dev->dev), &dev_attr_FG_g_fg_dbg_percentage_voltmode);

    return 0;
}

static int battery_meter_remove(struct platform_device *dev)    
{
    bm_print(BM_LOG_CRTI, "[battery_meter_remove]\n");
    return 0;
}

static void battery_meter_shutdown(struct platform_device *dev)    
{
    bm_print(BM_LOG_CRTI, "[battery_meter_shutdown]\n");        
}

static int battery_meter_suspend(struct platform_device *dev, pm_message_t state)    
{
    // -- hibernation path
    if (state.event == PM_EVENT_FREEZE) {
        pr_warn("[%s] %p:%p\n", __func__, battery_meter_ctrl, &bm_ctrl_cmd);
        battery_meter_ctrl = bm_ctrl_cmd;
    }
    // -- end of hibernation path

#if defined(CONFIG_POWER_EXT)

#elif defined(SOC_BY_SW_FG)
    struct timespec xts, tom;

	get_xtime_and_monotonic_and_sleep_offset(&xts, &tom, &g_rtc_time_before_sleep);
	if (_g_bat_sleep_total_time < g_spm_timer) {
		return 0;
	}
	battery_meter_ctrl(BATTERY_METER_CMD_GET_HW_OCV, &g_hw_ocv_before_sleep);
#endif	

    bm_print(BM_LOG_CRTI, "[battery_meter_suspend]\n");
    return 0;
}

static int battery_meter_resume(struct platform_device *dev)
{
#if defined(CONFIG_POWER_EXT)

#elif defined(SOC_BY_SW_FG)
	kal_int32 hw_ocv_after_sleep;
	struct timespec xts, tom, rtc_time_after_sleep;
//    kal_int32 sw_vbat;
//    kal_int32 vbat_diff = 200;
	get_xtime_and_monotonic_and_sleep_offset(&xts, &tom, &rtc_time_after_sleep);
	_g_bat_sleep_total_time += rtc_time_after_sleep.tv_sec - g_rtc_time_before_sleep.tv_sec;
	battery_xlog_printk(BAT_LOG_CRTI, "[battery_meter_resume] sleep time = %d, g_spm_timer = %d\n", _g_bat_sleep_total_time, g_spm_timer);
	if (_g_bat_sleep_total_time < g_spm_timer) {
		return 0;
	}
	bat_spm_timeout = true;
	battery_meter_ctrl(BATTERY_METER_CMD_GET_HW_OCV, &hw_ocv_after_sleep);    
#if 0    
    sw_vbat = battery_meter_get_battery_voltage();
    
    bm_print(BM_LOG_CRTI, "HW_OCV=%d, SW_VBAT=%d\n", hw_ocv_after_sleep, sw_vbat);
    
    if(hw_ocv_after_sleep < sw_vbat)
    {
        bm_print(BM_LOG_CRTI, "Ignore HW_OCV : small than SW_VBAT\n");
    }
    else if( (hw_ocv_after_sleep - sw_vbat) > vbat_diff )
    {
        bm_print(BM_LOG_CRTI, "Ignore HW_OCV : diff > %d\n", vbat_diff);
    }
    else
#endif        
    {    
        
        
    	if(_g_bat_sleep_total_time > 3600)	//1hr
    	{
    		if(hw_ocv_after_sleep != g_hw_ocv_before_sleep)
    		{
    			 gFG_DOD0 = fgauge_read_d_by_v(hw_ocv_after_sleep); 
    			 oam_v_ocv_2 = oam_v_ocv_1 = hw_ocv_after_sleep;
    			 oam_car_1 = 0;
    			 oam_car_2 = 0;
    		}
    		else
    		{
    			 oam_car_1 = oam_car_1 + (40* (rtc_time_after_sleep.tv_sec - g_rtc_time_before_sleep.tv_sec)/3600); //0.1mAh
    		     oam_car_2 = oam_car_2 + (40* (rtc_time_after_sleep.tv_sec - g_rtc_time_before_sleep.tv_sec)/3600); //0.1mAh	
    		}
    	}
    	else
    	{
    			oam_car_1 = oam_car_1 + (20* (rtc_time_after_sleep.tv_sec - g_rtc_time_before_sleep.tv_sec)/3600); //0.1mAh
    			oam_car_2 = oam_car_2 + (20* (rtc_time_after_sleep.tv_sec - g_rtc_time_before_sleep.tv_sec)/3600); //0.1mAh	
    	}	
        bm_print(BM_LOG_CRTI, "sleeptime=(%d)s, be_ocv=(%d), af_ocv=(%d), D0=(%d), car1=(%d), car2=(%d) \n",
    		rtc_time_after_sleep.tv_sec - g_rtc_time_before_sleep.tv_sec, g_hw_ocv_before_sleep, hw_ocv_after_sleep,gFG_DOD0, oam_car_1, oam_car_2);
    }
#endif		
    bm_print(BM_LOG_CRTI, "[battery_meter_resume]\n");
    return 0;
}

struct platform_device battery_meter_device = {
        .name                = "battery_meter",
        .id                  = -1,
};

static struct platform_driver battery_meter_driver = {
    .probe        = battery_meter_probe,
    .remove       = battery_meter_remove,
    .shutdown     = battery_meter_shutdown,
    .suspend      = battery_meter_suspend,
    .resume       = battery_meter_resume,
    .driver       = {
        .name = "battery_meter",
    },
};

static int __init battery_meter_init(void)
{
    int ret;
    
    ret = platform_device_register(&battery_meter_device);
    if (ret) {
        bm_print(BM_LOG_CRTI, "[battery_meter_driver] Unable to device register(%d)\n", ret);
        return ret;
    }
    
    ret = platform_driver_register(&battery_meter_driver);
    if (ret) {
        bm_print(BM_LOG_CRTI, "[battery_meter_driver] Unable to register driver (%d)\n", ret);
        return ret;
    }

    bm_print(BM_LOG_CRTI, "[battery_meter_driver] Initialization : DONE \n");

    return 0;

}

static void __exit battery_meter_exit (void)
{
}

module_init(battery_meter_init);
module_exit(battery_meter_exit);

MODULE_AUTHOR("James Lo");
MODULE_DESCRIPTION("Battery Meter Device Driver");
MODULE_LICENSE("GPL");

