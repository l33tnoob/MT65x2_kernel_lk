/*****************************************************************************
 *
 * Filename:
 * ---------
 *    linear_charging.c
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
#include <linux/xlog.h>
#include <linux/kernel.h>
#include <mach/battery_common.h>
#include <mach/charging.h>
#include "cust_charging.h"
#include <mach/mt_boot.h>
#include <linux/delay.h>
#include <mach/battery_meter.h>

 // ============================================================ //
 //define
 // ============================================================ //
 //cut off to full
#define POST_CHARGING_TIME	 30 * 60 // 30mins



 // ============================================================ //
 //global variable
 // ============================================================ //
 kal_uint32 g_bcct_flag=0;
 CHR_CURRENT_ENUM g_temp_CC_value = CHARGE_CURRENT_0_00_MA;
 kal_uint32 g_usb_state = USB_UNCONFIGURED;
 kal_uint32 charging_full_current = CHARGING_FULL_CURRENT;    // mA 
 kal_uint32 v_cc2topoff_threshold = V_CC2TOPOFF_THRES; 

  ///////////////////////////////////////////////////////////////////////////////////////////
  //// JEITA
  ///////////////////////////////////////////////////////////////////////////////////////////
#if defined(MTK_JEITA_STANDARD_SUPPORT)
  int g_jeita_recharging_voltage = JEITA_RECHARGE_VOLTAGE;
  int g_temp_status=TEMP_POS_10_TO_POS_45;
  kal_bool temp_error_recovery_chr_flag =KAL_TRUE;
#endif

 
 // ============================================================ //
 // function prototype
 // ============================================================ //
 
 
 // ============================================================ //
 //extern variable
 // ============================================================ //
 extern int g_platform_boot_mode;

 // ============================================================ //
 //extern function
 // ============================================================ //
 

 // ============================================================ //
void BATTERY_SetUSBState(int usb_state_value)
{
#if defined(CONFIG_POWER_EXT)
	battery_xlog_printk(BAT_LOG_CRTI, "[BATTERY_SetUSBState] in FPGA/EVB, no service\r\n");
#else
    if ( (usb_state_value < USB_SUSPEND) || ((usb_state_value > USB_CONFIGURED))){
        battery_xlog_printk(BAT_LOG_CRTI, "[BATTERY] BAT_SetUSBState Fail! Restore to default value\r\n");    
        usb_state_value = USB_UNCONFIGURED;
    } else {
        battery_xlog_printk(BAT_LOG_CRTI, "[BATTERY] BAT_SetUSBState Success! Set %d\r\n", usb_state_value);    
        g_usb_state = usb_state_value;    
    }
#endif	
}
//EXPORT_SYMBOL(BATTERY_SetUSBState);


kal_uint32 get_charging_setting_current()
{
	return g_temp_CC_value;
}


#if defined(MTK_JEITA_STANDARD_SUPPORT)

static BATTERY_VOLTAGE_ENUM select_jeita_cv(void)
{
    BATTERY_VOLTAGE_ENUM cv_voltage;

    if(g_temp_status == TEMP_ABOVE_POS_60)
    {
        cv_voltage = JEITA_TEMP_ABOVE_POS_60_CV_VOLTAGE;
    }
    else if(g_temp_status == TEMP_POS_45_TO_POS_60)
    {
        cv_voltage = JEITA_TEMP_POS_45_TO_POS_60_CV_VOLTAGE;
    }
    else if(g_temp_status == TEMP_POS_10_TO_POS_45)
    {
        #ifdef HIGH_BATTERY_VOLTAGE_SUPPORT
        cv_voltage = BATTERY_VOLT_04_350000_V;
        #else
        cv_voltage = JEITA_TEMP_POS_10_TO_POS_45_CV_VOLTAGE;
        #endif
    }
    else if(g_temp_status == TEMP_POS_0_TO_POS_10)
    {
        cv_voltage = JEITA_TEMP_POS_0_TO_POS_10_CV_VOLTAGE;
    }
    else if(g_temp_status == TEMP_NEG_10_TO_POS_0)
    {
        cv_voltage = JEITA_TEMP_NEG_10_TO_POS_0_CV_VOLTAGE;
    }
    else if(g_temp_status == TEMP_BELOW_NEG_10)
    {
        cv_voltage = JEITA_TEMP_BELOW_NEG_10_CV_VOLTAGE;
    }
    else
    {
        cv_voltage = BATTERY_VOLT_04_200000_V;
    }            

    return cv_voltage;
}

PMU_STATUS do_jeita_state_machine(void)
{
	int previous_g_temp_status;
	BATTERY_VOLTAGE_ENUM cv_voltage;
	
	previous_g_temp_status = g_temp_status;
    //JEITA battery temp Standard 
    if (BMT_status.temperature >= TEMP_POS_60_THRESHOLD) 
    {
        battery_xlog_printk(BAT_LOG_CRTI, "[BATTERY] Battery Over high Temperature(%d) !!\n\r", TEMP_POS_60_THRESHOLD);  
        g_temp_status = TEMP_ABOVE_POS_60;
        return PMU_STATUS_FAIL; 
    }
    else if(BMT_status.temperature > TEMP_POS_45_THRESHOLD)  
    {             
        if((g_temp_status == TEMP_ABOVE_POS_60) && (BMT_status.temperature >= TEMP_POS_60_THRES_MINUS_X_DEGREE))
        {
            battery_xlog_printk(BAT_LOG_CRTI, "[BATTERY] Battery Temperature between %d and %d,not allow charging yet!!\n\r", TEMP_POS_60_THRES_MINUS_X_DEGREE,TEMP_POS_60_THRESHOLD); 
            return PMU_STATUS_FAIL; 
        }
        else
        {
            battery_xlog_printk(BAT_LOG_CRTI, "[BATTERY] Battery Temperature between %d and %d !!\n\r", TEMP_POS_45_THRESHOLD,TEMP_POS_60_THRESHOLD); 
            g_temp_status = TEMP_POS_45_TO_POS_60;
            g_jeita_recharging_voltage = JEITA_TEMP_POS_45_TO_POS_60_RECHARGE_VOLTAGE;
            v_cc2topoff_threshold = JEITA_TEMP_POS_45_TO_POS_60_CC2TOPOFF_THRESHOLD;  
	    charging_full_current = CHARGING_FULL_CURRENT;
        }
    }
    else if(BMT_status.temperature >= TEMP_POS_10_THRESHOLD)  
    {
        if( ((g_temp_status == TEMP_POS_45_TO_POS_60) && (BMT_status.temperature >= TEMP_POS_45_THRES_MINUS_X_DEGREE)) ||
            ((g_temp_status == TEMP_POS_0_TO_POS_10 ) && (BMT_status.temperature <= TEMP_POS_10_THRES_PLUS_X_DEGREE ))    ) 
        {
            battery_xlog_printk(BAT_LOG_CRTI, "[BATTERY] Battery Temperature not recovery to normal temperature charging mode yet!!\n\r");     
        }
        else
        {
            battery_xlog_printk(BAT_LOG_CRTI, "[BATTERY] Battery Normal Temperature between %d and %d !!\n\r", TEMP_POS_10_THRESHOLD, TEMP_POS_45_THRESHOLD); 
                            
            g_temp_status = TEMP_POS_10_TO_POS_45;
            #ifdef HIGH_BATTERY_VOLTAGE_SUPPORT
            g_jeita_recharging_voltage = 4200;
            #else
            g_jeita_recharging_voltage = JEITA_TEMP_POS_10_TO_POS_45_RECHARGE_VOLTAGE;
            #endif
            v_cc2topoff_threshold = JEITA_TEMP_POS_10_TO_POS_45_CC2TOPOFF_THRESHOLD;  
	    charging_full_current = CHARGING_FULL_CURRENT;
        }
    }
    else if(BMT_status.temperature >= TEMP_POS_0_THRESHOLD)  
    {
        if((g_temp_status == TEMP_NEG_10_TO_POS_0 || g_temp_status == TEMP_BELOW_NEG_10) && (BMT_status.temperature <= TEMP_POS_0_THRES_PLUS_X_DEGREE))
        {
			if (g_temp_status == TEMP_NEG_10_TO_POS_0)
			{
            	battery_xlog_printk(BAT_LOG_CRTI, "[BATTERY] Battery Temperature between %d and %d !!\n\r", TEMP_POS_0_THRES_PLUS_X_DEGREE, TEMP_POS_10_THRESHOLD); 
			}
			if (g_temp_status == TEMP_BELOW_NEG_10)
			{
				battery_xlog_printk(BAT_LOG_CRTI, "[BATTERY] Battery Temperature between %d and %d,not allow charging yet!!\n\r", TEMP_POS_0_THRESHOLD,TEMP_POS_0_THRES_PLUS_X_DEGREE); 
				return PMU_STATUS_FAIL; 
			}
        }
        else
		{
            battery_xlog_printk(BAT_LOG_CRTI, "[BATTERY] Battery Temperature between %d and %d !!\n\r", TEMP_POS_0_THRESHOLD,TEMP_POS_10_THRESHOLD); 
            g_temp_status = TEMP_POS_0_TO_POS_10;
            g_jeita_recharging_voltage = JEITA_TEMP_POS_0_TO_POS_10_RECHARGE_VOLTAGE;
            v_cc2topoff_threshold = JEITA_TEMP_POS_0_TO_POS_10_CC2TOPOFF_THRESHOLD;
	    charging_full_current = CHARGING_FULL_CURRENT;
        }
    }
    else if(BMT_status.temperature >= TEMP_NEG_10_THRESHOLD)
    {
        if((g_temp_status == TEMP_BELOW_NEG_10) && (BMT_status.temperature <= TEMP_NEG_10_THRES_PLUS_X_DEGREE))
        {
            battery_xlog_printk(BAT_LOG_CRTI, "[BATTERY] Battery Temperature between %d and %d,not allow charging yet!!\n\r", TEMP_NEG_10_THRESHOLD, TEMP_NEG_10_THRES_PLUS_X_DEGREE); 
            return PMU_STATUS_FAIL; 
        }
		else
        {
            battery_xlog_printk(BAT_LOG_CRTI, "[BATTERY] Battery Temperature between %d and %d !!\n\r",TEMP_NEG_10_THRESHOLD,TEMP_POS_0_THRESHOLD); 
            g_temp_status = TEMP_NEG_10_TO_POS_0;
            g_jeita_recharging_voltage = JEITA_TEMP_NEG_10_TO_POS_0_RECHARGE_VOLTAGE;
            v_cc2topoff_threshold = JEITA_TEMP_NEG_10_TO_POS_0_CC2TOPOFF_THRESHOLD;  
	    charging_full_current = JEITA_NEG_10_TO_POS_0_FULL_CURRENT;
        }
    }
    else
    {
        battery_xlog_printk(BAT_LOG_CRTI, "[BATTERY] Battery below low Temperature(%d) !!\n\r", TEMP_NEG_10_THRESHOLD);  
        g_temp_status = TEMP_BELOW_NEG_10;
        return PMU_STATUS_FAIL; 
    }

	//set CV after temperature changed
	if (g_temp_status != previous_g_temp_status) {
		cv_voltage = select_jeita_cv();
		battery_charging_control(CHARGING_CMD_SET_CV_VOLTAGE,&cv_voltage);
	}
	
    return PMU_STATUS_OK;
}


static void set_jeita_charging_current(void)
{
#ifdef CONFIG_USB_IF
	if(BMT_status.charger_type == STANDARD_HOST)
		return;
#endif	

	if(g_temp_status == TEMP_NEG_10_TO_POS_0)
    {
        g_temp_CC_value = CHARGE_CURRENT_200_00_MA;   //for low temp	
        battery_xlog_printk(BAT_LOG_CRTI, "[BATTERY] JEITA set charging current : %d\r\n", g_temp_CC_value);
    }
}

#endif


void select_charging_curret_bcct(void)
{
    // done on set_bat_charging_current_limit
}


kal_uint32 set_bat_charging_current_limit(int current_limit)
{
    battery_xlog_printk(BAT_LOG_CRTI, "[BATTERY] set_bat_charging_current_limit (%d)\r\n", current_limit);

    if(current_limit != -1)
    {
        g_bcct_flag=1;
        
        if(current_limit < 70)         g_temp_CC_value=CHARGE_CURRENT_0_00_MA;
        else if(current_limit < 200)   g_temp_CC_value=CHARGE_CURRENT_70_00_MA;
        else if(current_limit < 300)   g_temp_CC_value=CHARGE_CURRENT_200_00_MA;
        else if(current_limit < 400)   g_temp_CC_value=CHARGE_CURRENT_300_00_MA;
        else if(current_limit < 450)   g_temp_CC_value=CHARGE_CURRENT_400_00_MA;
        else if(current_limit < 550)   g_temp_CC_value=CHARGE_CURRENT_450_00_MA;
        else if(current_limit < 650)   g_temp_CC_value=CHARGE_CURRENT_550_00_MA;
        else if(current_limit < 700)   g_temp_CC_value=CHARGE_CURRENT_650_00_MA;
        else if(current_limit < 800)   g_temp_CC_value=CHARGE_CURRENT_700_00_MA;
        else if(current_limit < 900)   g_temp_CC_value=CHARGE_CURRENT_800_00_MA;
        else if(current_limit < 1000)  g_temp_CC_value=CHARGE_CURRENT_900_00_MA;
        else if(current_limit < 1100)  g_temp_CC_value=CHARGE_CURRENT_1000_00_MA;
        else if(current_limit < 1200)  g_temp_CC_value=CHARGE_CURRENT_1100_00_MA;
        else if(current_limit < 1300)  g_temp_CC_value=CHARGE_CURRENT_1200_00_MA;
        else if(current_limit < 1400)  g_temp_CC_value=CHARGE_CURRENT_1300_00_MA;
        else if(current_limit < 1500)  g_temp_CC_value=CHARGE_CURRENT_1400_00_MA;
        else if(current_limit < 1600)  g_temp_CC_value=CHARGE_CURRENT_1500_00_MA;
        else if(current_limit == 1600) g_temp_CC_value=CHARGE_CURRENT_1600_00_MA;
        else                           g_temp_CC_value=CHARGE_CURRENT_450_00_MA;
    }
    else
    {
        //change to default current setting
        g_bcct_flag=0;
    }
    
    wake_up_bat();

    return g_bcct_flag;
}    


void select_charging_curret(void)
{
    if (g_ftm_battery_flag) 
    {
        battery_xlog_printk(BAT_LOG_CRTI, "[BATTERY] FTM charging : %d\r\n", charging_level_data[0]);    
        g_temp_CC_value = charging_level_data[0];                
    }    
    else 
    {    
        if ( BMT_status.charger_type == STANDARD_HOST ) 
        {
			#ifdef CONFIG_USB_IF
            {
                if (g_usb_state == USB_SUSPEND)
                {
                    g_temp_CC_value = USB_CHARGER_CURRENT_SUSPEND;
                }
                else if (g_usb_state == USB_UNCONFIGURED)
                {
                    g_temp_CC_value = USB_CHARGER_CURRENT_UNCONFIGURED;
                }
                else if (g_usb_state == USB_CONFIGURED)
                {
                    g_temp_CC_value = USB_CHARGER_CURRENT_CONFIGURED;
                }
                else
                {
                    g_temp_CC_value = USB_CHARGER_CURRENT_UNCONFIGURED;
                }
                
                battery_xlog_printk(BAT_LOG_CRTI, "[BATTERY] STANDARD_HOST CC mode charging : %d on %d state\r\n", g_temp_CC_value, g_usb_state);
             }
			#else
            {    
      	        g_temp_CC_value = USB_CHARGER_CURRENT;            
            }
			#endif
        } 
        else if (BMT_status.charger_type == NONSTANDARD_CHARGER) 
        {   
            g_temp_CC_value = NON_STD_AC_CHARGER_CURRENT;
        } 
        else if (BMT_status.charger_type == STANDARD_CHARGER) 
        {
            g_temp_CC_value = AC_CHARGER_CURRENT;
        }
        else if (BMT_status.charger_type == CHARGING_HOST) 
        {
            g_temp_CC_value = CHARGING_HOST_CHARGER_CURRENT;
        }
		else if (BMT_status.charger_type == APPLE_2_1A_CHARGER) 
        {
            g_temp_CC_value = APPLE_2_1A_CHARGER_CURRENT;
        }
		else if (BMT_status.charger_type == APPLE_1_0A_CHARGER) 
        {
            g_temp_CC_value = APPLE_1_0A_CHARGER_CURRENT;
        }
		else if (BMT_status.charger_type == APPLE_0_5A_CHARGER) 
        {
            g_temp_CC_value = APPLE_0_5A_CHARGER_CURRENT;
        }
		else 
        {
            g_temp_CC_value = CHARGE_CURRENT_70_00_MA;
        }

		battery_xlog_printk(BAT_LOG_CRTI, "[BATTERY] Default CC mode charging : %d\r\n", g_temp_CC_value);
		
#if defined(MTK_JEITA_STANDARD_SUPPORT)
	set_jeita_charging_current();
#endif
    }
}




static kal_uint32 charging_full_check(void)
{
	kal_uint32 status = KAL_FALSE;
	
#if defined(POST_TIME_ENABLE)			
	static kal_uint32 post_charging_time=0; 
	
	if (post_charging_time >= POST_CHARGING_TIME)
    {
		status = KAL_TRUE;
        post_charging_time=0;
		
        battery_xlog_printk(BAT_LOG_CRTI, "[BATTERY] Battery real full and disable charging on %ld mA \n", BMT_status.ICharging); 
    }
    else if (post_charging_time > 0)
    {
        post_charging_time+=BAT_TASK_PERIOD;
        battery_xlog_printk(BAT_LOG_CRTI, "[BATTERY] post_charging_time=%d,POST_CHARGING_TIME=%d\n", post_charging_time, POST_CHARGING_TIME); 
    }
    else if ((BMT_status.TOPOFF_charging_time > 60)&& (BMT_status.ICharging <= charging_full_current))
    {
   	    post_charging_time=BAT_TASK_PERIOD;
       	battery_xlog_printk(BAT_LOG_CRTI, "[BATTERY] Enter Post charge, post_charging_time=%d,POST_CHARGING_TIME=%d\n",post_charging_time, POST_CHARGING_TIME); 
    }
    else
    {
        post_charging_time=0;
    }
#else
	static kal_uint8 full_check_count = 0;

	if(BMT_status.ICharging <= charging_full_current)
    {
    	full_check_count++;
		if(6 == full_check_count)
		{
    		status = KAL_TRUE;
			full_check_count = 0;
	   	    battery_xlog_printk(BAT_LOG_CRTI, "[BATTERY] Battery full and disable charging on %ld mA \n", BMT_status.ICharging); 
		}
    }
	else
	{
		full_check_count = 0;
	}
#endif

	return status;
}


static void charging_current_calibration(void)
{
	kal_int32 bat_isense_offset;
#if 0	
	kal_int32 bat_vol = battery_meter_get_battery_voltage();
	kal_int32 Vsense = battery_meter_get_VSense();

	bat_isense_offset = bat_vol - Vsense;

	battery_xlog_printk(BAT_LOG_CRTI, "[BATTERY] bat_vol=%d, Vsense=%d, offset=%d \r\n",bat_vol,Vsense,bat_isense_offset);
#else
	bat_isense_offset = 0;
#endif

	battery_meter_sync(bat_isense_offset);
}

static void pchr_turn_on_charging (void)
{
#if !defined(MTK_JEITA_STANDARD_SUPPORT) 
	BATTERY_VOLTAGE_ENUM cv_voltage;
#endif
	kal_uint32 charging_enable = KAL_TRUE;

	battery_xlog_printk(BAT_LOG_FULL, "[BATTERY] pchr_turn_on_charging()!\r\n");

    if ( BMT_status.bat_charging_state == CHR_ERROR ) 
    {
        battery_xlog_printk(BAT_LOG_CRTI, "[BATTERY] Charger Error, turn OFF charging !\n");
	
		charging_enable = KAL_FALSE;
    }
    else if( (g_platform_boot_mode==META_BOOT) || (g_platform_boot_mode==ADVMETA_BOOT) )
    {   
        battery_xlog_printk(BAT_LOG_CRTI, "[BATTERY] In meta or advanced meta mode, disable charging.\n");    
        charging_enable = KAL_FALSE;
    }
    else
    {
        /*HW initialization*/
  	    battery_xlog_printk(BAT_LOG_FULL, "charging_hw_init\n" );
       	battery_charging_control(CHARGING_CMD_INIT,NULL);
		

		/* Set Charging Current */
        if (g_bcct_flag == 1)
        {
			battery_xlog_printk(BAT_LOG_FULL, "[BATTERY] select_charging_curret_bcct !\n");
            select_charging_curret_bcct();
        }
        else
        {
       	    battery_xlog_printk(BAT_LOG_FULL, "[BATTERY] select_charging_current !\n");
            select_charging_curret();           
        }
        
        if( g_temp_CC_value == CHARGE_CURRENT_0_00_MA)
        {
			charging_enable = KAL_FALSE;
	        battery_xlog_printk(BAT_LOG_CRTI, "[BATTERY] charging current is set 0mA, turn off charging !\r\n");
        }
        else
        {    
           
			battery_charging_control(CHARGING_CMD_SET_CURRENT,&g_temp_CC_value);

			/* Set CV */
			#if !defined(MTK_JEITA_STANDARD_SUPPORT)           
				#ifdef HIGH_BATTERY_VOLTAGE_SUPPORT
					cv_voltage = BATTERY_VOLT_04_350000_V;
				#else
		            cv_voltage = BATTERY_VOLT_04_200000_V;
				#endif
				battery_charging_control(CHARGING_CMD_SET_CV_VOLTAGE,&cv_voltage);
			#endif
        }
    }

	/* enable/disable charging */
	battery_xlog_printk(BAT_LOG_CRTI, "[BATTERY] pchr_turn_on_charging(), enable =%d \r\n", charging_enable);
	battery_charging_control(CHARGING_CMD_ENABLE,&charging_enable);

	
}

 
PMU_STATUS BAT_PreChargeModeAction(void)
{
	kal_bool charging_enable = KAL_FALSE;

    battery_xlog_printk(BAT_LOG_CRTI, "[BATTERY] Pre-CC mode charge, timer=%ld on %ld !!\n\r", BMT_status.PRE_charging_time, BMT_status.total_charging_time);    

    BMT_status.PRE_charging_time += BAT_TASK_PERIOD;
    BMT_status.CC_charging_time = 0;
    BMT_status.TOPOFF_charging_time = 0;
    BMT_status.total_charging_time += BAT_TASK_PERIOD;

	if (BMT_status.UI_SOC == 100)
	{
		BMT_status.bat_charging_state = CHR_BATFULL;
		BMT_status.bat_full = KAL_TRUE;
		g_charging_full_reset_bat_meter = KAL_TRUE;
	}
    else if(BMT_status.bat_vol > V_PRE2CC_THRES)
    {
        BMT_status.bat_charging_state = CHR_CC;
    }

	/*Charging 9s and discharging 1s : start*/  
	battery_charging_control(CHARGING_CMD_ENABLE,&charging_enable);

	msleep(1000);   

	charging_current_calibration();

    pchr_turn_on_charging();   

    return PMU_STATUS_OK;        
} 


PMU_STATUS BAT_ConstantCurrentModeAction(void)
{
	kal_bool charging_enable = KAL_FALSE;

    battery_xlog_printk(BAT_LOG_CRTI, "[BATTERY] CC mode charge, timer=%ld on %ld !!\n\r", BMT_status.CC_charging_time, BMT_status.total_charging_time);    

	BMT_status.PRE_charging_time = 0;
    BMT_status.CC_charging_time += BAT_TASK_PERIOD;
    BMT_status.TOPOFF_charging_time = 0;
    BMT_status.total_charging_time += BAT_TASK_PERIOD;

    if ( BMT_status.bat_vol > v_cc2topoff_threshold )
    {
        BMT_status.bat_charging_state = CHR_TOP_OFF;
    }

	/*Charging 9s and discharging 1s : start*/  
	battery_charging_control(CHARGING_CMD_ENABLE,&charging_enable);

	msleep(1000);	

	charging_current_calibration();

    pchr_turn_on_charging();     

    return PMU_STATUS_OK;        
}    


PMU_STATUS BAT_TopOffModeAction(void)
{
	kal_uint32 charging_enable = KAL_FALSE;
	
    battery_xlog_printk(BAT_LOG_CRTI, "[BATTERY] Top Off mode charge, timer=%ld on %ld !!\n\r", BMT_status.TOPOFF_charging_time, BMT_status.total_charging_time);    

    BMT_status.PRE_charging_time = 0;
    BMT_status.CC_charging_time = 0;
    BMT_status.TOPOFF_charging_time += BAT_TASK_PERIOD;
    BMT_status.total_charging_time += BAT_TASK_PERIOD;                    

    pchr_turn_on_charging();

	if ((BMT_status.TOPOFF_charging_time >= MAX_CV_CHARGING_TIME) || (charging_full_check() == KAL_TRUE) )
    {
        BMT_status.bat_charging_state = CHR_BATFULL;
		BMT_status.bat_full = KAL_TRUE;
		g_charging_full_reset_bat_meter = KAL_TRUE;

		 /*  Disable charging */
		battery_charging_control(CHARGING_CMD_ENABLE,&charging_enable);
    }

    return PMU_STATUS_OK;        
} 


PMU_STATUS BAT_BatteryFullAction(void)
{
	kal_uint32 charging_enable = KAL_FALSE;
	
    battery_xlog_printk(BAT_LOG_CRTI, "[BATTERY] Battery full !!\n\r");            
    
    BMT_status.bat_full = KAL_TRUE;
    BMT_status.total_charging_time = 0;
    BMT_status.PRE_charging_time = 0;
    BMT_status.CC_charging_time = 0;
    BMT_status.TOPOFF_charging_time = 0;
    BMT_status.POSTFULL_charging_time = 0;
	BMT_status.bat_in_recharging_state = KAL_FALSE;

      	       
#if defined(MTK_JEITA_STANDARD_SUPPORT)
    if(BMT_status.bat_vol < g_jeita_recharging_voltage)    
#else         
    if(BMT_status.bat_vol < RECHARGING_VOLTAGE )    
#endif        	
    {
        battery_xlog_printk(BAT_LOG_CRTI, "[BATTERY] Battery Enter Re-charging!! , vbat=(%d)\n\r", BMT_status.bat_vol);                

		BMT_status.bat_in_recharging_state = KAL_TRUE;
        BMT_status.bat_charging_state = CHR_CC;
    }        

    /*  Disable charging */
	battery_charging_control(CHARGING_CMD_ENABLE,&charging_enable);
    
    return PMU_STATUS_OK;
}


PMU_STATUS BAT_BatteryHoldAction(void)
{
	kal_uint32 charging_enable;
	
	battery_xlog_printk(BAT_LOG_CRTI, "[BATTERY] Hold mode !!\n\r");
	 
	if(BMT_status.bat_vol < TALKING_RECHARGE_VOLTAGE || g_call_state == CALL_IDLE)
	{
		BMT_status.bat_charging_state = CHR_CC;
		battery_xlog_printk(BAT_LOG_CRTI, "[BATTERY] Exit Hold mode and Enter CC mode !!\n\r");
	}	
		
	/*  Disable charger */
	charging_enable = KAL_FALSE;
	battery_charging_control(CHARGING_CMD_ENABLE,&charging_enable);	

    return PMU_STATUS_OK;
}


PMU_STATUS BAT_BatteryStatusFailAction(void)
{
	kal_uint32 charging_enable;
	
    battery_xlog_printk(BAT_LOG_CRTI, "[BATTERY] BAD Battery status... Charging Stop !!\n\r");            

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
	charging_enable = KAL_FALSE;
	battery_charging_control(CHARGING_CMD_ENABLE,&charging_enable);

    return PMU_STATUS_OK;
}


void mt_battery_charging_algorithm()
{
	 switch(BMT_status.bat_charging_state)
    {            
        case CHR_PRE :
            BAT_PreChargeModeAction();
            break;    
            
        case CHR_CC :
            BAT_ConstantCurrentModeAction();
            break;    
            
        case CHR_TOP_OFF :
            BAT_TopOffModeAction();
            break;              

        case CHR_BATFULL:
            BAT_BatteryFullAction();
            break;
            
	case CHR_HOLD:
	    BAT_BatteryHoldAction();
            break;
			
        case CHR_ERROR:
            BAT_BatteryStatusFailAction();
            break;                
    }    
   
}
 

