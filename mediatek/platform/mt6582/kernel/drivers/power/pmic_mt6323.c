/*****************************************************************************
 *
 * Filename:
 * ---------
 *    pmic_mt6323.c
 *
 * Project:
 * --------
 *   Android_Software
 *
 * Description:
 * ------------
 *   This Module defines PMIC functions
 *
 * Author:
 * -------
 * James Lo
 *
 ****************************************************************************/
#include <generated/autoconf.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/kthread.h>
#include <linux/wakelock.h>
#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/aee.h>
#include <linux/xlog.h>
#include <linux/proc_fs.h>
#include <linux/syscalls.h>
#include <linux/sched.h>
#include <linux/writeback.h>
#include <linux/earlysuspend.h>

#include <asm/uaccess.h>

#include <mach/pmic_mt6323_sw.h>
#include <mach/upmu_common.h>
#include <mach/upmu_hw.h>
#include <mach/mt_pm_ldo.h>
#include <mach/eint.h>
#include <mach/mt_pmic_wrap.h>
#include <mach/mt_gpio.h>
#include <mach/mtk_rtc.h>
#include <mach/mt_spm_mtcmos.h>

#include <mach/battery_common.h>
#include <linux/time.h> 

#if defined (MTK_KERNEL_POWER_OFF_CHARGING)
#include <mach/mt_boot.h>
#include <mach/system.h>
#include "mach/mt_gpt.h"
#endif

extern int Enable_BATDRV_LOG;

//#include <mach/mt_clock_manager.h>
//----------------------------------------------------------------------test
#define MT65XX_UPLL 3
void pmic_enable_pll(int id, char *mod_name)
{
    printk("enable_pll is not ready.\n");
}
void pmic_disable_pll(int id, char *mod_name)
{
    printk("disable_pll is not ready.\n");
}
//----------------------------------------------------------------------


#if defined(CONFIG_POWER_EXT)
//----------------------------------------------------------------------
#define CUST_EINT_POLARITY_LOW              0
#define CUST_EINT_POLARITY_HIGH             1
#define CUST_EINT_DEBOUNCE_DISABLE          0
#define CUST_EINT_DEBOUNCE_ENABLE           1
#define CUST_EINT_EDGE_SENSITIVE            0
#define CUST_EINT_LEVEL_SENSITIVE           1

#define CUST_EINT_MT6323_PMIC_NUM              25
#define CUST_EINT_MT6323_PMIC_DEBOUNCE_CN      1
//#define CUST_EINT_MT6323_PMIC_POLARITY         CUST_EINT_POLARITY_HIGH
//#define CUST_EINT_MT6323_PMIC_SENSITIVE        CUST_EINT_LEVEL_SENSITIVE
#define CUST_EINT_MT6323_PMIC_TYPE             EINTF_TRIGGER_HIGH
#define CUST_EINT_MT6323_PMIC_DEBOUNCE_EN      CUST_EINT_DEBOUNCE_ENABLE
//----------------------------------------------------------------------
#else
#include <cust_eint.h>
#endif

//==============================================================================
// PMIC related define
//==============================================================================
#define VOLTAGE_FULL_RANGE     1800
#define ADC_PRECISE         32768 // 10 bits

static DEFINE_MUTEX(pmic_lock_mutex);
static DEFINE_MUTEX(pmic_adc_mutex);
static DEFINE_SPINLOCK(pmic_adc_lock);

//==============================================================================
// Extern
//==============================================================================
extern int g_R_BAT_SENSE;
extern int g_R_I_SENSE;
extern int g_R_CHARGER_1;
extern int g_R_CHARGER_2;
extern int g_bat_init_flag;

extern int bat_thread_kthread(void *x);
extern void charger_hv_detect_sw_workaround_init(void);


extern void pmu_drv_tool_customization_init(void);


#if defined (MTK_KERNEL_POWER_OFF_CHARGING)
extern void mt_power_off(void);
static kal_bool long_pwrkey_press = false;
static unsigned long timer_pre = 0; 
static unsigned long timer_pos = 0; 
#define LONG_PWRKEY_PRESS_TIME 		500*1000000    //500ms
#endif
//==============================================================================
// PMIC lock/unlock APIs
//==============================================================================
void pmic_lock(void)
{
    mutex_lock(&pmic_lock_mutex);
}

void pmic_unlock(void)
{
    mutex_unlock(&pmic_lock_mutex);
}

kal_uint32 upmu_get_reg_value(kal_uint32 reg)
{
    U32 ret=0;
    U32 reg_val=0;

    //printk("[upmu_get_reg_value] \n");
    ret=pmic_read_interface(reg, &reg_val, 0xFFFF, 0x0);
    
    return reg_val;
}
EXPORT_SYMBOL(upmu_get_reg_value);

void upmu_set_reg_value(kal_uint32 reg, kal_uint32 reg_val)
{
    U32 ret=0;

    //printk("[upmu_set_reg_value] \n");
    ret=pmic_config_interface(reg, reg_val, 0xFFFF, 0x0);    
}

kal_int32 count_time_out=100;
struct wake_lock pmicAuxadc_irq_lock;

struct wake_lock pmicThread_lock;

kal_uint32  pmic_is_auxadc_busy(void)
{
	kal_uint32 ret=0;
	kal_uint32 int_status_val_0=0;
	ret=pmic_read_interface_nolock(0x73a,(&int_status_val_0),0x7FFF,0x1);
	return int_status_val_0;
}

void PMIC_IMM_PollingAuxadcChannel(void)
{
	kal_uint32 ret=0;
    
	 //xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[PMIC_IMM_PollingAuxadcChannel] before:%d ",upmu_get_rg_adc_deci_gdly());

	if (upmu_get_rg_adc_deci_gdly()==1)
	{
		while(upmu_get_rg_adc_deci_gdly()==1)
		{
			unsigned long flags;
			spin_lock_irqsave(&pmic_adc_lock, flags);
			if (pmic_is_auxadc_busy()==0)
			{
				//upmu_set_rg_adc_deci_gdly(0);
				ret=pmic_config_interface_nolock(AUXADC_CON19,0,PMIC_RG_ADC_DECI_GDLY_MASK,PMIC_RG_ADC_DECI_GDLY_SHIFT);
			}
			spin_unlock_irqrestore(&pmic_adc_lock, flags);
		}
	}
	//xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[PMIC_IMM_PollingAuxadcChannel] after:%d ",upmu_get_rg_adc_deci_gdly());
}

//==============================================================================
// PMIC-AUXADC 
//==============================================================================
int PMIC_IMM_GetOneChannelValue(int dwChannel, int deCount, int trimd)
{

	kal_int32 ret_data;	
	kal_int32 count=0;
	kal_int32 u4Sample_times = 0;
	kal_int32 u4channel=0;	
	kal_int32 adc_result_temp=0;
       kal_int32 r_val_temp=0;   
	kal_int32 adc_result=0;   
    kal_int32 ret=0;
    kal_int32 adc_reg_val=0;
	
    /*
        0 : BATON2 **
        1 : CH6
        2 : THR SENSE2 **
        3 : THR SENSE1
        4 : VCDT
        5 : BATON1
        6 : ISENSE
        7 : BATSNS
        8 : ACCDET    
        9-16 : audio
    */

    //do not suppport BATON2 and THR SENSE2 for sw workaround
    if (dwChannel==0 || dwChannel==2)
        return 0;

    wake_lock(&pmicAuxadc_irq_lock);
	

	do
	{

    mutex_lock(&pmic_adc_mutex);
	
    PMIC_IMM_PollingAuxadcChannel();
	

    if (dwChannel<9)
    {
        upmu_set_rg_vbuf_en(1);

        //set 0
        ret=pmic_read_interface(AUXADC_CON22,&adc_reg_val,PMIC_RG_AP_RQST_LIST_MASK,PMIC_RG_AP_RQST_LIST_SHIFT);
        adc_reg_val = adc_reg_val & (~(1<<dwChannel));
        ret=pmic_config_interface(AUXADC_CON22,adc_reg_val,PMIC_RG_AP_RQST_LIST_MASK,PMIC_RG_AP_RQST_LIST_SHIFT);

        //set 1
        ret=pmic_read_interface(AUXADC_CON22,&adc_reg_val,PMIC_RG_AP_RQST_LIST_MASK,PMIC_RG_AP_RQST_LIST_SHIFT);
        adc_reg_val = adc_reg_val | (1<<dwChannel);
        ret=pmic_config_interface(AUXADC_CON22,adc_reg_val,PMIC_RG_AP_RQST_LIST_MASK,PMIC_RG_AP_RQST_LIST_SHIFT);
    }
    else if(dwChannel>=9 && dwChannel<=16)
    {
        ret=pmic_read_interface(AUXADC_CON23,&adc_reg_val,PMIC_RG_AP_RQST_LIST_RSV_MASK,PMIC_RG_AP_RQST_LIST_RSV_SHIFT);
        adc_reg_val = adc_reg_val & (~(1<<(dwChannel-9)));
        ret=pmic_config_interface(AUXADC_CON23,adc_reg_val,PMIC_RG_AP_RQST_LIST_RSV_MASK,PMIC_RG_AP_RQST_LIST_RSV_SHIFT);

        //set 1
        ret=pmic_read_interface(AUXADC_CON23,&adc_reg_val,PMIC_RG_AP_RQST_LIST_RSV_MASK,PMIC_RG_AP_RQST_LIST_RSV_SHIFT);
        adc_reg_val = adc_reg_val | (1<<(dwChannel-9));
        ret=pmic_config_interface(AUXADC_CON23,adc_reg_val,PMIC_RG_AP_RQST_LIST_RSV_MASK,PMIC_RG_AP_RQST_LIST_RSV_SHIFT);		
    }


	mutex_unlock(&pmic_adc_mutex);

	    //Duo to HW limitation
	    if(dwChannel!=8)
	    msleep(1);

	    count=0;
	    ret_data=0;

	    switch(dwChannel){         
	        case 0:    
	            while( upmu_get_rg_adc_rdy_baton2() != 1 )
	            {
			msleep(1);
			if( (count++) > count_time_out)
			{
                        xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[IMM_GetOneChannelValue_PMIC] (%d) Time out!\n", dwChannel);
			    break;
			}			
	            }
	            ret_data = upmu_get_rg_adc_out_baton2();				
	            break;
		
	        case 1:    
	            while( upmu_get_rg_adc_rdy_ch6() != 1 )
	            {
			msleep(1);
			if( (count++) > count_time_out)
			{
                        xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[IMM_GetOneChannelValue_PMIC] (%d) Time out!\n", dwChannel);
			    break;
			}			
	            }
	            ret_data = upmu_get_rg_adc_out_ch6();				
                xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[upmu_get_rg_adc_out_ch6] 0x%x\n", ret_data);
	            break;
	        case 2:    
	            while( upmu_get_rg_adc_rdy_thr_sense2() != 1 )
	            {
			msleep(1);
			if( (count++) > count_time_out)
			{
                        xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[IMM_GetOneChannelValue_PMIC] (%d) Time out!\n", dwChannel);
			    break;
			}			
	            }
	            ret_data = upmu_get_rg_adc_out_thr_sense2();				
	            break;				
	        case 3:    
	            while( upmu_get_rg_adc_rdy_thr_sense1() != 1 )
	            {
			msleep(1);
			if( (count++) > count_time_out)
			{
                        xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[IMM_GetOneChannelValue_PMIC] (%d) Time out!\n", dwChannel);
			    break;
			}			
	            }
	            ret_data = upmu_get_rg_adc_out_thr_sense1();				
	            break;
	        case 4:    
	            while( upmu_get_rg_adc_rdy_vcdt() != 1 )
	            {
			msleep(1);
			if( (count++) > count_time_out)
			{
                        xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[IMM_GetOneChannelValue_PMIC] (%d) Time out!\n", dwChannel);
			    break;
			}			
	            }
	            ret_data = upmu_get_rg_adc_out_vcdt();				
	            break;
	        case 5:    
	            while( upmu_get_rg_adc_rdy_baton1() != 1 )
	            {
			msleep(1);
			if( (count++) > count_time_out)
			{
                        xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[IMM_GetOneChannelValue_PMIC] (%d) Time out!\n", dwChannel);
			    break;
			}			
	            }
	            ret_data = upmu_get_rg_adc_out_baton1();				
	            break;
	        case 6:    
	            while( upmu_get_rg_adc_rdy_isense() != 1 )
	            {
			msleep(1);
			if( (count++) > count_time_out)
			{
                        xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[IMM_GetOneChannelValue_PMIC] (%d) Time out!\n", dwChannel);
			    break;
			}			
	            }
	            ret_data = upmu_get_rg_adc_out_isense();				
	            break;
	        case 7:    
	            while( upmu_get_rg_adc_rdy_batsns() != 1 )
	            {
			msleep(1);
			if( (count++) > count_time_out)
			{
                        xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[IMM_GetOneChannelValue_PMIC] (%d) Time out!\n", dwChannel);
			    break;
			}			
	            }
	            ret_data = upmu_get_rg_adc_out_batsns();				
	            break; 
                
	        case 8:    
	            while( upmu_get_rg_adc_rdy_ch5() != 1 );
	            ret_data = upmu_get_rg_adc_out_ch5();				
	            break; 				
	        case 9:    
		case 10:  
		case 11:  
		case 12:
	        case 13:    
		case 14:  
		case 15:  
		case 16:	 	
	            while( upmu_get_rg_adc_rdy_int() != 1 )
	            {
			msleep(1);
			if( (count++) > count_time_out)
			{
                        xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[IMM_GetOneChannelValue_PMIC] (%d) Time out!\n", dwChannel);
			    break;
			}			
	            }
	            ret_data = upmu_get_rg_adc_out_int();				
	            break; 				
                
	        default:
	            xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[AUXADC] Invalid channel value(%d,%d)\n", dwChannel, trimd);
	            wake_unlock(&pmicAuxadc_irq_lock);
	            return -1;
	            break;
	    }

	    u4channel += ret_data;

	    u4Sample_times++;

	    if (Enable_BATDRV_LOG == 2)
	    {
	        //debug
	        xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[AUXADC] u4channel[%d]=%d.\n", 
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
	case 9:    
	case 10:  
	case 11:  
	case 12:
	case 13:    
	case 14:  
	case 15:  
	case 16:		
            adc_result = adc_result_temp;
            break;  
        default:
            xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[AUXADC] Invalid channel value(%d,%d)\n", dwChannel, trimd);
            wake_unlock(&pmicAuxadc_irq_lock);
            return -1;
            break;
    }

    if (Enable_BATDRV_LOG == 2)
    {
        //debug
        xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[AUXADC] adc_result_temp=%d, adc_result=%d, r_val_temp=%d.\n", 
                adc_result_temp, adc_result, r_val_temp);
    }

    wake_unlock(&pmicAuxadc_irq_lock);
	
    return adc_result;
   
}

int pmic_get_buck_current(int avg_times)
{
    int raw_data = 0;
    int val = 0;
    int offset = 0; // internal offset voltage = XmV, 80

    upmu_set_rg_smps_testmode_b(0x10);
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[pmic_get_buck_current] before meter, Reg[0x%x]=0x%x, Reg[0x%x]=0x%x\n", 
        0x200, upmu_get_reg_value(0x200), 
        0x758, upmu_get_reg_value(0x758), 
        0x76E, upmu_get_reg_value(0x76E)
        );
    
    raw_data = PMIC_IMM_GetOneChannelValue(1, avg_times, 1); // Vdac = code / 32768 * 1800mV
    val = raw_data - offset;                                            
    if(val > 0)                     
        val = (val*10)/6; // Iload = Vdac / 0.6ohm              
    else
        val = 0;
    
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[pmic_get_buck_current] raw_data=%ld, val=%ld, avg_times=%ld\n", 
        raw_data, val, avg_times);

    upmu_set_rg_smps_testmode_b(0x0);
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[pmic_get_buck_current] after meter, Reg[0x%x]=0x%x, Reg[0x%x]=0x%x\n", 
        0x200, upmu_get_reg_value(0x200), 
        0x758, upmu_get_reg_value(0x758), 
        0x76E, upmu_get_reg_value(0x76E)
        );

    return val;
}
EXPORT_SYMBOL(pmic_get_buck_current);

void upmu_interrupt_chrdet_int_en(kal_uint32 val)
{
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[upmu_interrupt_chrdet_int_en] val=%d.\r\n", val);

    upmu_set_rg_int_en_chrdet(val);
}
EXPORT_SYMBOL(upmu_interrupt_chrdet_int_en);

//==============================================================================
// PMIC Interrupt service
//==============================================================================
int pmic_thread_timeout=0;
static DEFINE_MUTEX(pmic_mutex);
static DECLARE_WAIT_QUEUE_HEAD(pmic_thread_wq);


void wake_up_pmic(void)
{
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[wake_up_pmic]\r\n");
    pmic_thread_timeout = 1;
    wake_up(&pmic_thread_wq);
    wake_lock(&pmicThread_lock);
	
}
EXPORT_SYMBOL(wake_up_pmic);

#define WAKE_LOCK_INITIALIZED            (1U << 8)


void cust_pmic_interrupt_en_setting(void)
{
#if 1
    upmu_set_rg_int_en_spkl_ab(0);
    upmu_set_rg_int_en_spkl(0);
    upmu_set_rg_int_en_bat_l(0);
    upmu_set_rg_int_en_bat_h(0);
    upmu_set_rg_int_en_watchdog(0);
    upmu_set_rg_int_en_pwrkey(1);
    upmu_set_rg_int_en_thr_l(0);
    upmu_set_rg_int_en_thr_h(0);
    upmu_set_rg_int_en_vbaton_undet(0);
    upmu_set_rg_int_en_bvalid_det(0);
    upmu_set_rg_int_en_chrdet(1);
    upmu_set_rg_int_en_ov(0);
    
    upmu_set_rg_int_en_ldo(0);
    upmu_set_rg_int_en_fchrkey(1);
    //upmu_set_rg_int_en_accdet(1);
    upmu_set_rg_int_en_audio(0);
    upmu_set_rg_int_en_rtc(1);
    upmu_set_rg_int_en_vproc(0);
    upmu_set_rg_int_en_vsys(0);
    upmu_set_rg_int_en_vpa(0);     
#endif    
}

void spkl_ab_int_handler(void)
{
    kal_uint32 ret=0;

    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[spkl_ab_int_handler]....\n");
    
    ret=pmic_config_interface(INT_STATUS0,0x1,0x1,0);    
}

void spkl_d_int_handler(void)
{
    kal_uint32 ret=0;

    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[spkl_d_int_handler]....\n");
    
    ret=pmic_config_interface(INT_STATUS0,0x1,0x1,1);    
}

void bat_l_int_handler(void)
{
    kal_uint32 ret=0;

    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[bat_l_int_handler]....\n");


    upmu_set_rg_lbat_irq_en_min(0);	
	
    ret=pmic_config_interface(INT_STATUS0,0x1,0x1,2);    
}

void bat_h_int_handler(void)
{
    kal_uint32 ret=0;

    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[bat_h_int_handler]....\n");

    upmu_set_rg_lbat_irq_en_max(0);
	
    ret=pmic_config_interface(INT_STATUS0,0x1,0x1,3);    
}

void watchdog_int_handler(void)
{
    kal_uint32 ret=0;

    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[watchdog_int_handler]....\n");
    
    ret=pmic_config_interface(INT_STATUS0,0x1,0x1,4);    
}

extern void kpd_pwrkey_pmic_handler(unsigned long pressed);

void pwrkey_int_handler(void)
{
    kal_uint32 ret=0;

    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[pwrkey_int_handler]....\n");
    
		if(upmu_get_pwrkey_deb()==1)    	    	
    {
        xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[pwrkey_int_handler] Release pwrkey\n");
#if defined (MTK_KERNEL_POWER_OFF_CHARGING)
		if(g_boot_mode == KERNEL_POWER_OFF_CHARGING_BOOT && timer_pre != 0)
		{
				timer_pos = sched_clock();
				if(timer_pos - timer_pre >= LONG_PWRKEY_PRESS_TIME)
				{
					long_pwrkey_press = true;
				}
				xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[pmic_thread_kthread] timer_pos = %ld, timer_pre = %ld, timer_pos-timer_pre = %ld, long_pwrkey_press = %d\r\n",timer_pos, timer_pre, timer_pos-timer_pre, long_pwrkey_press);
				if(long_pwrkey_press)   //500ms
				{
					xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[pmic_thread_kthread] Power Key Pressed during kernel power off charging, reboot OS\r\n");
					arch_reset(0, NULL);
				}
		}
#endif
        kpd_pwrkey_pmic_handler(0x0);
        upmu_set_rg_pwrkey_int_sel(0);
    }
    else
    {
        xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[pwrkey_int_handler] Press pwrkey\n");
#if defined (MTK_KERNEL_POWER_OFF_CHARGING)
		if(g_boot_mode == KERNEL_POWER_OFF_CHARGING_BOOT)
		{
			timer_pre = sched_clock();
		}
#endif
        kpd_pwrkey_pmic_handler(0x1);
        upmu_set_rg_pwrkey_int_sel(1);
    }
    
    ret=pmic_config_interface(INT_STATUS0,0x1,0x1,5);    
}

void thr_l_int_handler(void)
{
    kal_uint32 ret=0;

    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[thr_l_int_handler]....\n");
    
    ret=pmic_config_interface(INT_STATUS0,0x1,0x1,6);    
}

void thr_h_int_handler(void)
{
    kal_uint32 ret=0;

    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[thr_h_int_handler]....\n");
    
    ret=pmic_config_interface(INT_STATUS0,0x1,0x1,7);    
}

void vbaton_undet_int_handler(void)
{
    kal_uint32 ret=0;

    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[vbaton_undet_int_handler]....\n");
    
    ret=pmic_config_interface(INT_STATUS0,0x1,0x1,8);    
}

void bvalid_det_int_handler(void)
{
    kal_uint32 ret=0;

    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[bvalid_det_int_handler]....\n");
    
    ret=pmic_config_interface(INT_STATUS0,0x1,0x1,9);    
}

void chrdet_int_handler(void)
{
    kal_uint32 ret=0;
	
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[chrdet_int_handler]....\n");
#ifdef MTK_KERNEL_POWER_OFF_CHARGING
    if (!upmu_get_rgs_chrdet())
    {
        int boot_mode = 0;
        boot_mode = get_boot_mode();
        
        if(boot_mode == KERNEL_POWER_OFF_CHARGING_BOOT || boot_mode == LOW_POWER_OFF_CHARGING_BOOT)
        {
            xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[chrdet_int_handler] Unplug Charger/USB In Kernel Power Off Charging Mode!  Shutdown OS!\r\n");
            mt_power_off();
        }
    }
#endif
    do_chrdet_int_task();

    ret=pmic_config_interface(INT_STATUS0,0x1,0x1,10);  
}

void vbat_ov_int_handler(void)
{
	kal_uint32 ret=0;
	
	xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[vbat_ov_int_handler]....\n"); 

    ret=pmic_config_interface(INT_STATUS0,0x1,0x1,11);
}

void ldo_oc_int_handler(void)
{
    kal_uint32 ret=0;

    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[ldo_oc_int_handler]....\n");
    
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[PMIC] Reg[0x%x]=0x%x\n", OCSTATUS0, upmu_get_reg_value(OCSTATUS0));
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[PMIC] Reg[0x%x]=0x%x\n", OCSTATUS1, upmu_get_reg_value(OCSTATUS1));

    ret=pmic_config_interface(INT_STATUS1,0x1,0x1,0);    
}

extern void kpd_pmic_rstkey_handler(unsigned long pressed);
void fchr_key_int_handler(void)
{
    kal_uint32 ret=0;

    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[fchr_key_int_handler]....\n");
    
    if(upmu_get_fchrkey_deb()==1)
    {
        xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[fchr_key_int_handler] Release fchrkey\n");
        kpd_pmic_rstkey_handler(0x0);
        upmu_set_rg_fchrkey_int_sel(0);
    }
    else
    {
        xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[fchr_key_int_handler] Press fchrkey\n");
        kpd_pmic_rstkey_handler(0x1);
        upmu_set_rg_fchrkey_int_sel(1);
    }
    
    ret=pmic_config_interface(INT_STATUS1,0x1,0x1,1);    
}


extern int accdet_irq_handler(void);
void accdet_int_handler(void)
{
    kal_uint32 ret=0;

    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[accdet_int_handler]....\n");

    ret = accdet_irq_handler();
    if(0 == ret){
        xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[accdet_int_handler] don't finished\n");
    }
    ret=pmic_config_interface(INT_STATUS1,0x1,0x1,2);    
}

void audio_int_handler(void)
{
    kal_uint32 ret=0;

    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[audio_int_handler]....\n");
    
    ret=pmic_config_interface(INT_STATUS1,0x1,0x1,3);    
}

void rtc_int_handler(void)
{
    kal_uint32 ret=0;

    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[rtc_int_handler]....\n");

    rtc_irq_handler();

    ret=pmic_config_interface(INT_STATUS1,0x1,0x1,4);    
}

void vproc_oc_int_handler(void)
{
    kal_uint32 ret=0;

    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[vproc_oc_int_handler]....\n");

    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[PMIC] Reg[0x%x]=0x%x\n", OCSTATUS0, upmu_get_reg_value(OCSTATUS0));
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[PMIC] Reg[0x%x]=0x%x\n", OCSTATUS1, upmu_get_reg_value(OCSTATUS1));

    upmu_set_rg_pwmoc_ck_pdn(1);	
	
    ret=pmic_config_interface(INT_STATUS1,0x1,0x1,5);    

    upmu_set_rg_int_en_vproc(0);	
}

void vsys_oc_int_handler(void)
{
    kal_uint32 ret=0;

    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[vsys_oc_int_handler]....\n");

    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[PMIC] Reg[0x%x]=0x%x\n", OCSTATUS0, upmu_get_reg_value(OCSTATUS0));
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[PMIC] Reg[0x%x]=0x%x\n", OCSTATUS1, upmu_get_reg_value(OCSTATUS1));

    upmu_set_rg_pwmoc_ck_pdn(1);
	
    ret=pmic_config_interface(INT_STATUS1,0x1,0x1,6);    

    upmu_set_rg_int_en_vsys(0);	
}

void vpa_oc_int_handler(void)
{
    kal_uint32 ret=0;

    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[vpa_oc_int_handler]....\n");

    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[PMIC] Reg[0x%x]=0x%x\n", OCSTATUS0, upmu_get_reg_value(OCSTATUS0));
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[PMIC] Reg[0x%x]=0x%x\n", OCSTATUS1, upmu_get_reg_value(OCSTATUS1));

    upmu_set_rg_pwmoc_ck_pdn(1);

    ret=pmic_config_interface(INT_STATUS1,0x1,0x1,7);

    upmu_set_rg_int_en_vpa(0);	
}

static int pmic_thread_kthread(void *x)
{

    kal_uint32 ret=0;
    kal_uint32 int_status_val_0=0;
    kal_uint32 int_status_val_1=0;

    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[pmic_thread_kthread] enter\n");

    /* Run on a process content */
    while (1) {
        mutex_lock(&pmic_mutex);
	
        xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[pmic_thread_kthread] running\n");

        //--------------------------------------------------------------------------------
        ret=pmic_read_interface(INT_STATUS0,(&int_status_val_0),0xFFFF,0x0);
        xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[INT] int_status_val_0=0x%x\n", int_status_val_0);

        if( (((int_status_val_0)&(0x0001))>>0) == 1 )  { spkl_ab_int_handler();      }
        if( (((int_status_val_0)&(0x0002))>>1) == 1 )  { spkl_d_int_handler();      }         
        if( (((int_status_val_0)&(0x0004))>>2) == 1 )  { bat_l_int_handler();         }
        if( (((int_status_val_0)&(0x0008))>>3) == 1 )  { bat_h_int_handler();         }
        if( (((int_status_val_0)&(0x0010))>>4) == 1 )  { watchdog_int_handler();        }
        if( (((int_status_val_0)&(0x0020))>>5) == 1 )  { pwrkey_int_handler();        }
        if( (((int_status_val_0)&(0x0040))>>6) == 1 )  { thr_l_int_handler();     }
        if( (((int_status_val_0)&(0x0080))>>7) == 1 )  { thr_h_int_handler();     }
        if( (((int_status_val_0)&(0x0100))>>8) == 1 )  { vbaton_undet_int_handler();     }
        if( (((int_status_val_0)&(0x0200))>>9) == 1 )  { bvalid_det_int_handler();       }
        if( (((int_status_val_0)&(0x0400))>>10) == 1 ) { chrdet_int_handler();        }
        if( (((int_status_val_0)&(0x0800))>>11) == 1 ) { vbat_ov_int_handler();        }
                     
        //--------------------------------------------------------------------------------
        ret=pmic_read_interface(INT_STATUS1,(&int_status_val_1),0xFFFF,0x0);
        xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[INT] int_status_val_1=0x%x\n", int_status_val_1);

        if( (((int_status_val_1)&(0x0001))>>0) == 1 )  { ldo_oc_int_handler();          }
        if( (((int_status_val_1)&(0x0002))>>1) == 1 )  { fchr_key_int_handler();      }
        if( (((int_status_val_1)&(0x0004))>>2) == 1 )  { accdet_int_handler();       }
        if( (((int_status_val_1)&(0x0008))>>3) == 1 )  { audio_int_handler();        }
        if( (((int_status_val_1)&(0x0010))>>4) == 1 )  { rtc_int_handler();          }
        if( (((int_status_val_1)&(0x0020))>>5) == 1 )  { vproc_oc_int_handler();      }
        if( (((int_status_val_1)&(0x0040))>>6) == 1 )  { vsys_oc_int_handler();       }
        if( (((int_status_val_1)&(0x0080))>>7) == 1 )  { vpa_oc_int_handler();       }             
        //--------------------------------------------------------------------------------

        mdelay(1);
        
        mt_eint_unmask(CUST_EINT_MT6323_PMIC_NUM);

        //set INT_EN, in PMIC_EINT_SETTING()
        cust_pmic_interrupt_en_setting();

        ret=pmic_read_interface(INT_STATUS0,(&int_status_val_0),0xFFFF,0x0);
        xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[INT] after ,int_status_val_0=0x%x\n", int_status_val_0);

        ret=pmic_read_interface(INT_STATUS1,(&int_status_val_1),0xFFFF,0x0);
        xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[INT] after ,int_status_val_1=0x%x\n", int_status_val_1);

	
        mutex_unlock(&pmic_mutex);

        wake_unlock(&pmicThread_lock);

        wait_event(pmic_thread_wq, pmic_thread_timeout);

        pmic_thread_timeout=0;
    }


    return 0;
}

void mt6323_pmic_eint_irq(void)
{
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[mt6323_pmic_eint_irq] receive interrupt\n");

    //pmic internal
    wake_up_pmic();

    return ;
}

void PMIC_EINT_SETTING(void)
{
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[PMIC_EINT_SETTING] start: CUST_EINT_MT6323_PMIC_NUM=%d\n",CUST_EINT_MT6323_PMIC_NUM);

    //ON/OFF interrupt
    cust_pmic_interrupt_en_setting();

    //GPIO Setting for early porting
    //mt_set_gpio_mode(GPIO37,GPIO_MODE_01); //EINT3 mode 1 on GPIO37
    //xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[CUST_EINT] GPIO37=GPIO_MODE_01 for EINT3 usage\n");

    //EINT setting
    //mt_eint_set_sens(           CUST_EINT_MT6323_PMIC_NUM,
    //                            CUST_EINT_MT6323_PMIC_SENSITIVE);
    //mt_eint_set_polarity(       CUST_EINT_MT6323_PMIC_NUM,
    //                            CUST_EINT_MT6323_PMIC_POLARITY);        // set positive polarity
    mt_eint_set_hw_debounce(    CUST_EINT_MT6323_PMIC_NUM,
                                CUST_EINT_MT6323_PMIC_DEBOUNCE_CN);     // set debounce time
    mt_eint_registration(       CUST_EINT_MT6323_PMIC_NUM,                                
                                CUST_EINT_MT6323_PMIC_TYPE,
                                mt6323_pmic_eint_irq,
                                0);

    mt_eint_unmask(CUST_EINT_MT6323_PMIC_NUM);

    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[CUST_EINT] CUST_EINT_MT6323_PMIC_NUM=%d\n", CUST_EINT_MT6323_PMIC_NUM);
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[CUST_EINT] CUST_EINT_MT6323_PMIC_DEBOUNCE_CN=%d\n", CUST_EINT_MT6323_PMIC_DEBOUNCE_CN);
    //xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[CUST_EINT] CUST_EINT_MT6323_PMIC_POLARITY=%d\n", CUST_EINT_MT6323_PMIC_POLARITY);
    //xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[CUST_EINT] CUST_EINT_MT6323_PMIC_SENSITIVE=%d\n", CUST_EINT_MT6323_PMIC_SENSITIVE);
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[CUST_EINT] CUST_EINT_MT6323_PMIC_TYPE=%d\n", CUST_EINT_MT6323_PMIC_TYPE);
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[CUST_EINT] CUST_EINT_MT6323_PMIC_DEBOUNCE_EN=%d\n", CUST_EINT_MT6323_PMIC_DEBOUNCE_EN);

    upmu_set_rg_intrp_ck_pdn(0); //for all interrupt events, turn on interrupt module clock
}

void PMIC_DUMP_ALL_Register(void)
{
    kal_uint32 i=0;
    kal_uint32 ret=0;
    kal_uint32 reg_val=0;

    for (i=0;i<0xFFFF;i++)
    {
        ret=pmic_read_interface(i,&reg_val,0xFFFF,0);
        xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "Reg[0x%x]=0x%x\n", i, reg_val);
    }
}

//==============================================================================
// PMIC read/write APIs
//==============================================================================
#define CONFIG_PMIC_HW_ACCESS_EN

//#define PMIC_REG_NUM 0xFFFF

static DEFINE_MUTEX(pmic_access_mutex);
//U32 pmic6323_reg[PMIC_REG_NUM] = {0};

U32 pmic_read_interface (U32 RegNum, U32 *val, U32 MASK, U32 SHIFT)
{
    U32 return_value = 0;

#if defined(CONFIG_PMIC_HW_ACCESS_EN)
    U32 pmic6323_reg = 0;
    U32 rdata;

    mutex_lock(&pmic_access_mutex);

    //mt6323_read_byte(RegNum, &pmic6323_reg);
    return_value= pwrap_wacs2(0, (RegNum), 0, &rdata);
    pmic6323_reg=rdata;
    if(return_value!=0)
    {
        xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[pmic_read_interface] Reg[%x]= pmic_wrap read data fail\n", RegNum);
        mutex_unlock(&pmic_access_mutex);
        return return_value;
    }
    //xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[pmic_read_interface] Reg[%x]=0x%x\n", RegNum, pmic6323_reg);

    pmic6323_reg &= (MASK << SHIFT);
    *val = (pmic6323_reg >> SHIFT);
    //xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[pmic_read_interface] val=0x%x\n", *val);

    mutex_unlock(&pmic_access_mutex);
#else
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[pmic_read_interface] Can not access HW PMIC\n");
#endif

    return return_value;
}

U32 pmic_config_interface (U32 RegNum, U32 val, U32 MASK, U32 SHIFT)
{
    U32 return_value = 0;

#if defined(CONFIG_PMIC_HW_ACCESS_EN)
    U32 pmic6323_reg = 0;
    U32 rdata;

    mutex_lock(&pmic_access_mutex);

    //1. mt6323_read_byte(RegNum, &pmic6323_reg);
    return_value= pwrap_wacs2(0, (RegNum), 0, &rdata);
    pmic6323_reg=rdata;
    if(return_value!=0)
    {
        xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[pmic_config_interface] Reg[%x]= pmic_wrap read data fail\n", RegNum);
        mutex_unlock(&pmic_access_mutex);
        return return_value;
    }
    //xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[pmic_config_interface] Reg[%x]=0x%x\n", RegNum, pmic6323_reg);

    pmic6323_reg &= ~(MASK << SHIFT);
    pmic6323_reg |= (val << SHIFT);

    //2. mt6323_write_byte(RegNum, pmic6323_reg);
    return_value= pwrap_wacs2(1, (RegNum), pmic6323_reg, &rdata);
    if(return_value!=0)
    {
        xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[pmic_config_interface] Reg[%x]= pmic_wrap read data fail\n", RegNum);
        mutex_unlock(&pmic_access_mutex);
        return return_value;
    }
    //xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[pmic_config_interface] write Reg[%x]=0x%x\n", RegNum, pmic6323_reg);

    #if 0
    //3. Double Check
    //mt6323_read_byte(RegNum, &pmic6323_reg);
    return_value= pwrap_wacs2(0, (RegNum), 0, &rdata);
    pmic6323_reg=rdata;
    if(return_value!=0)
    {
        xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[pmic_config_interface] Reg[%x]= pmic_wrap write data fail\n", RegNum);
        mutex_unlock(&pmic_access_mutex);
        return return_value;
    }
    xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[pmic_config_interface] Reg[%x]=0x%x\n", RegNum, pmic6323_reg);
    #endif

    mutex_unlock(&pmic_access_mutex);
#else
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[pmic_config_interface] Can not access HW PMIC\n");
#endif

    return return_value;
}

//==============================================================================
// PMIC read/write APIs : nolock
//==============================================================================
U32 pmic_read_interface_nolock (U32 RegNum, U32 *val, U32 MASK, U32 SHIFT)
{
    U32 return_value = 0;

#if defined(CONFIG_PMIC_HW_ACCESS_EN)
    U32 pmic6323_reg = 0;
    U32 rdata;

    //mt6323_read_byte(RegNum, &pmic6323_reg);
    return_value= pwrap_wacs2(0, (RegNum), 0, &rdata);
    pmic6323_reg=rdata;
    if(return_value!=0)
    {
        xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[pmic_read_interface_nolock] Reg[%x]= pmic_wrap read data fail\n", RegNum);
        return return_value;
    }
    //xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[pmic_read_interface_nolock] Reg[%x]=0x%x\n", RegNum, pmic6323_reg);

    pmic6323_reg &= (MASK << SHIFT);
    *val = (pmic6323_reg >> SHIFT);
    //xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[pmic_read_interface_nolock] val=0x%x\n", *val);
#else
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[pmic_read_interface_nolock] Can not access HW PMIC\n");
#endif

    return return_value;
}

U32 pmic_config_interface_nolock (U32 RegNum, U32 val, U32 MASK, U32 SHIFT)
{
    U32 return_value = 0;

#if defined(CONFIG_PMIC_HW_ACCESS_EN)
    U32 pmic6323_reg = 0;
    U32 rdata;

    //1. mt6323_read_byte(RegNum, &pmic6323_reg);
    return_value= pwrap_wacs2(0, (RegNum), 0, &rdata);
    pmic6323_reg=rdata;
    if(return_value!=0)
    {
        xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[pmic_config_interface_nolock] Reg[%x]= pmic_wrap read data fail\n", RegNum);
        return return_value;
    }
    //xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[pmic_config_interface_nolock] Reg[%x]=0x%x\n", RegNum, pmic6323_reg);

    pmic6323_reg &= ~(MASK << SHIFT);
    pmic6323_reg |= (val << SHIFT);

    //2. mt6323_write_byte(RegNum, pmic6323_reg);
    return_value= pwrap_wacs2(1, (RegNum), pmic6323_reg, &rdata);
    if(return_value!=0)
    {
        xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[pmic_config_interface_nolock] Reg[%x]= pmic_wrap read data fail\n", RegNum);
        return return_value;
    }
    //xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[pmic_config_interface_nolock] write Reg[%x]=0x%x\n", RegNum, pmic6323_reg);
#else
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[pmic_config_interface_nolock] Can not access HW PMIC\n");
#endif

    return return_value;
}

//==============================================================================
// mt-pmic dev_attr APIs
//==============================================================================
U32 g_reg_value=0;
static ssize_t show_pmic_access(struct device *dev,struct device_attribute *attr, char *buf)
{
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[show_pmic_access] 0x%x\n", g_reg_value);
    return sprintf(buf, "%u\n", g_reg_value);
}
static ssize_t store_pmic_access(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
    int ret=0;
    char *pvalue = NULL;
    U32 reg_value = 0;
    U32 reg_address = 0;
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[store_pmic_access] \n");
    if(buf != NULL && size != 0)
    {
        xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[store_pmic_access] buf is %s and size is %d \n",buf,size);
        reg_address = simple_strtoul(buf,&pvalue,16);

        if(size > 5)
        {
            reg_value = simple_strtoul((pvalue+1),NULL,16);
            xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[store_pmic_access] write PMU reg 0x%x with value 0x%x !\n",reg_address,reg_value);
            ret=pmic_config_interface(reg_address, reg_value, 0xFFFF, 0x0);
        }
        else
        {
            ret=pmic_read_interface(reg_address, &g_reg_value, 0xFFFF, 0x0);
            xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[store_pmic_access] read PMU reg 0x%x with value 0x%x !\n",reg_address,g_reg_value);
            xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[store_pmic_access] Please use \"cat pmic_access\" to get value\r\n");
        }
    }
    return size;
}
static DEVICE_ATTR(pmic_access, 0664, show_pmic_access, store_pmic_access); //664

//==============================================================================
// LDO EN APIs
//==============================================================================

/*
BUCK0:
VPROC
VSYS

BUCK1:
VPA

ANALDO:
VTCXO
VA
VCAMA
VCN33 (wifi/bt })
VCN28

DIGLDO:
VIO28
VUSB
VMC
VMCH
VEMC_3V3
VGP1
VGP2
VGP3
VCN_1V8
VSIM1
VSIM2
VRTC
VCAM_AF
VIBR
VM
VRF18
VIO18
VCAMD
VCAM_IO
*/

void dct_pmic_VPROC_enable(kal_bool dctEnable)
{
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[dct_pmic_VPROC_enable] %d\n", dctEnable);

    if(dctEnable == KAL_TRUE)
    {
        upmu_set_vproc_en(1);
    }
    else
    {
        upmu_set_vproc_en(0);
    }
}

void dct_pmic_VSYS_enable(kal_bool dctEnable)
{
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[dct_pmic_VSYS_enable] %d\n", dctEnable);

    if(dctEnable == KAL_TRUE)
    {
        upmu_set_vsys_en(1);
    }
    else
    {
        upmu_set_vsys_en(0);
    }
}

void dct_pmic_VPA_enable(kal_bool dctEnable)
{
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[dct_pmic_VPA_enable] %d\n", dctEnable);

    if(dctEnable == KAL_TRUE)
    {
        upmu_set_vpa_en(1);
    }
    else
    {
        upmu_set_vpa_en(0);
    }
}

//Digital LDO
void dct_pmic_VIO28_enable(kal_bool dctEnable)
{
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[dct_pmic_VIO28_enable] %d\n", dctEnable);

    if(dctEnable == KAL_TRUE)
    {
        upmu_set_vio28_en(1);
    }
    else
    {
        upmu_set_vio28_en(0);
    }
}

void dct_pmic_VUSB_enable(kal_bool dctEnable)
{
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[dct_pmic_VUSB_enable] %d\n", dctEnable);

    if(dctEnable == KAL_TRUE)
    {
        upmu_set_rg_vusb_en(1);
    }
    else
    {
        upmu_set_rg_vusb_en(0);
    }
}

void dct_pmic_VMC_enable(kal_bool dctEnable)
{
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[dct_pmic_VMC_enable] %d\n", dctEnable);

    if(dctEnable == KAL_TRUE)
    {
        upmu_set_rg_vmc_en(1);
    }
    else
    {
        upmu_set_rg_vmc_en(0);
    }
}

void dct_pmic_VMCH_enable(kal_bool dctEnable)
{
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[dct_pmic_VMCH_enable] %d\n", dctEnable);

    if(dctEnable == KAL_TRUE)
    {
        upmu_set_rg_vmch_en(1);
    }
    else
    {
        upmu_set_rg_vmch_en(0);
    }
}

void dct_pmic_VEMC_3V3_enable(kal_bool dctEnable)
{
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[dct_pmic_VEMC_3V3_enable] %d\n", dctEnable);

    if(dctEnable == KAL_TRUE)
    {
        upmu_set_rg_vemc_3v3_en(1);
    }
    else
    {
        upmu_set_rg_vemc_3v3_en(0);
    }
}

void dct_pmic_VGP1_enable(kal_bool dctEnable)
{
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[dct_pmic_VGP1_enable] %d\n", dctEnable);

    if(dctEnable == KAL_TRUE)
    {
        upmu_set_rg_vgp1_en(1);
    }
    else
    {
        upmu_set_rg_vgp1_en(0);
    }
}

void dct_pmic_VGP2_enable(kal_bool dctEnable)
{
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[dct_pmic_VGP2_enable] %d\n", dctEnable);

    if(dctEnable == KAL_TRUE)
    {
        upmu_set_rg_vgp2_en(1);
    }
    else
    {
        upmu_set_rg_vgp2_en(0);
    }
}

void dct_pmic_VGP3_enable(kal_bool dctEnable)
{
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[dct_pmic_VGP3_enable] %d\n", dctEnable);

    if(dctEnable == KAL_TRUE)
    {
        upmu_set_rg_vgp3_en(1);
    }
    else
    {
        upmu_set_rg_vgp3_en(0);
    }
}

void dct_pmic_VCN_1V8_enable(kal_bool dctEnable)
{
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[dct_pmic_VCN18_enable] %d\n", dctEnable);

    if(dctEnable == KAL_TRUE)
    {
        upmu_set_rg_vcn_1v8_en(1);
    }
    else
    {
        upmu_set_rg_vcn_1v8_en(0);
    }
}

void dct_pmic_VSIM1_enable(kal_bool dctEnable)
{
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[dct_pmic_VSIM1_enable] %d\n", dctEnable);

    if(dctEnable == KAL_TRUE)
    {
        upmu_set_rg_vsim1_en(1);
    }
    else
    {
        upmu_set_rg_vsim1_en(0);
    }
}

void dct_pmic_VSIM2_enable(kal_bool dctEnable)
{
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[dct_pmic_VSIM2_enable] %d\n", dctEnable);

    if(dctEnable == KAL_TRUE)
    {
        upmu_set_rg_vsim2_en(1);
    }
    else
    {
        upmu_set_rg_vsim2_en(0);
    }
}

void dct_pmic_VRTC_enable(kal_bool dctEnable)
{
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[dct_pmic_VRTC_enable] %d\n", dctEnable);

    if(dctEnable == KAL_TRUE)
    {
        upmu_set_vrtc_en(1);
    }
    else
    {
        upmu_set_vrtc_en(0);
    }
}

void dct_pmic_VCAM_AF_enable(kal_bool dctEnable)
{
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[dct_pmic_VCAM_AF_enable] %d\n", dctEnable);

    if(dctEnable == KAL_TRUE)
    {
        upmu_set_rg_vcam_af_en(1);
    }
    else
    {
        upmu_set_rg_vcam_af_en(0);
    }
}

void dct_pmic_VIBR_enable(kal_bool dctEnable)
{
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[dct_pmic_VIBR_enable] %d\n", dctEnable);

    if(dctEnable == KAL_TRUE)
    {
        upmu_set_rg_vibr_en(1);
    }
    else
    {
        upmu_set_rg_vibr_en(0);
    }
}

void dct_pmic_VM_enable(kal_bool dctEnable)
{
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[dct_pmic_VM_enable] %d\n", dctEnable);

    if(dctEnable == KAL_TRUE)
    {
        upmu_set_rg_vm_en(1);
    }
    else
    {
        upmu_set_rg_vm_en(0);
    }
}

void dct_pmic_VRF18_enable(kal_bool dctEnable)
{
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[dct_pmic_VRF18_enable] %d\n", dctEnable);

    if(dctEnable == KAL_TRUE)
    {
        upmu_set_rg_vrf18_en(1);
    }
    else
    {
        upmu_set_rg_vrf18_en(0);
    }
}

void dct_pmic_VIO18_enable(kal_bool dctEnable)
{
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[dct_pmic_VIO18_enable] %d\n", dctEnable);

    if(dctEnable == KAL_TRUE)
    {
        upmu_set_rg_vio18_en(1);
    }
    else
    {
        upmu_set_rg_vio18_en(0);
    }
}

void dct_pmic_VCAMD_enable(kal_bool dctEnable)
{
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[dct_pmic_VCAMD_enable] %d\n", dctEnable);

    if(dctEnable == KAL_TRUE)
    {
        upmu_set_rg_vcamd_en(1);
    }
    else
    {
        upmu_set_rg_vcamd_en(0);
    }
}

void dct_pmic_VCAM_IO_enable(kal_bool dctEnable)
{
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[dct_pmic_VCAM_IO_enable] %d\n", dctEnable);

    if(dctEnable == KAL_TRUE)
    {
        upmu_set_rg_vcam_io_en(1);
    }
    else
    {
        upmu_set_rg_vcam_io_en(0);
    }
}

//ANALOG LDO
void dct_pmic_VTCXO_enable(kal_bool dctEnable)
{
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[dct_pmic_VTCXO_enable] %d\n", dctEnable);

    if(dctEnable == KAL_TRUE)
    {
        upmu_set_rg_vtcxo_en(1);
    }
    else
    {
        upmu_set_rg_vtcxo_en(0);
    }
}

void dct_pmic_VA_enable(kal_bool dctEnable)
{
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[dct_pmic_VA_enable] %d\n", dctEnable);

    if(dctEnable == KAL_TRUE)
    {
        upmu_set_rg_va_en(1);
    }
    else
    {
        upmu_set_rg_va_en(0);
    }
}

void dct_pmic_VCAMA_enable(kal_bool dctEnable)
{
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[dct_pmic_VCAMA_enable] %d\n", dctEnable);

    if(dctEnable == KAL_TRUE)
    {
        upmu_set_rg_vcama_en(1);
    }
    else
    {
        upmu_set_rg_vcama_en(0);
    }
}

void dct_pmic_VCN33_enable_bt(kal_bool dctEnable)
{
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[dct_pmic_VCN28_enable_bt] %d\n", dctEnable);

    if(dctEnable == KAL_TRUE)
    {
        upmu_set_rg_vcn33_en_bt(1);
    }
    else
    {
        upmu_set_rg_vcn33_en_bt(0);
    }
}

void dct_pmic_VCN33_enable_wifi(kal_bool dctEnable)
{
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[dct_pmic_VCN28_enable_wifi] %d\n", dctEnable);

    if(dctEnable == KAL_TRUE)
    {
        upmu_set_rg_vcn33_en_wifi(1);
    }
    else
    {
        upmu_set_rg_vcn33_en_wifi(0);
    }
}



void dct_pmic_VCN28_enable(kal_bool dctEnable)
{
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[dct_pmic_VCN28_enable] %d\n", dctEnable);

    if(dctEnable == KAL_TRUE)
    {
        upmu_set_rg_vcn28_en(1);
    }
    else
    {
        upmu_set_rg_vcn28_en(0);
    }
}


//==============================================================================
// LDO SEL APIs
//==============================================================================

/*
BUCK0:
VPROC
VSYS

BUCK1:
VPA

DIGLDO:
VIO28
VUSB
VMC
VMCH
VEMC_3V3
VGP1
VGP2
VGP3
VCN_1V8
VSIM1
VSIM2
VRTC
VCAM_AF
VIBR
VM
VRF18
VIO18
VCAMD
VCAM_IO

ANALDO:
VTCXO
VA
VCAMA
VCN33
VCN28
*/

//D-LDO
void dct_pmic_VMC_sel(kal_uint32 volt)
{
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "****[dct_pmic_VMC_sel] value=%d \n", volt);

    if(volt == VOL_DEFAULT)     {upmu_set_rg_vmc_vosel(1);}
    else if(volt == VOL_3300){upmu_set_rg_vmc_vosel(1); }
    else if(volt == VOL_1800){upmu_set_rg_vmc_vosel(0); }
    else{
        xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "Error Setting %d. DO nothing.\r\n", volt);
    }
}

void dct_pmic_VMCH_sel(kal_uint32 volt)
{
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "****[dct_pmic_VMCH_sel] value=%d \n", volt);

    if(volt == VOL_DEFAULT)     {upmu_set_rg_vmch_vosel(1);}
    else if(volt == VOL_3000){upmu_set_rg_vmch_vosel(0); }
    else if(volt == VOL_3300){upmu_set_rg_vmch_vosel(1); }
    else{
        xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "Error Setting %d. DO nothing.\r\n", volt);
    }
}

void dct_pmic_VEMC_3V3_sel(kal_uint32 volt)
{
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "****[dct_pmic_VEMC_3V3_sel] value=%d \n", volt);

    if(volt == VOL_DEFAULT)     {upmu_set_rg_vemc_3v3_vosel(1);}
    else if(volt == VOL_3000){upmu_set_rg_vemc_3v3_vosel(0); }
    else if(volt == VOL_3300){upmu_set_rg_vemc_3v3_vosel(1); }
    else{
        xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "Error Setting %d. DO nothing.\r\n", volt);
    }
}

void dct_pmic_VGP1_sel(kal_uint32 volt)
{
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "****[dct_pmic_VGP1_sel] value=%d \n", volt);

    if(volt == VOL_DEFAULT)     { upmu_set_rg_vgp1_vosel(5);}
    else if(volt == VOL_1200){ upmu_set_rg_vgp1_vosel(0);}
    else if(volt == VOL_1300){ upmu_set_rg_vgp1_vosel(1);}
    else if(volt == VOL_1500){ upmu_set_rg_vgp1_vosel(2);}
    else if(volt == VOL_1800){ upmu_set_rg_vgp1_vosel(3);}
    else if(volt == VOL_2000){ upmu_set_rg_vgp1_vosel(4);}
    else if(volt == VOL_2800){ upmu_set_rg_vgp1_vosel(5);}
    else if(volt == VOL_3000){ upmu_set_rg_vgp1_vosel(6);}
    else if(volt == VOL_3300){ upmu_set_rg_vgp1_vosel(7);}
    else{
        xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "Error Setting %d. DO nothing.\r\n", volt);
    }
}

void dct_pmic_VGP2_sel(kal_uint32 volt)
{
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "****[dct_pmic_VGP2_sel] value=%d \n", volt);

    if(volt == VOL_DEFAULT)     { upmu_set_rg_vgp2_vosel(4);}
    else if(volt == VOL_1200){ upmu_set_rg_vgp2_vosel(0);}
    else if(volt == VOL_1300){ upmu_set_rg_vgp2_vosel(1);}
    else if(volt == VOL_1500){ upmu_set_rg_vgp2_vosel(2);}
    else if(volt == VOL_1800){ upmu_set_rg_vgp2_vosel(3);}
    else if(volt == VOL_2500){ upmu_set_rg_vgp2_vosel(4);}
    else if(volt == VOL_2800){ upmu_set_rg_vgp2_vosel(5);}
    else if(volt == VOL_3000){ upmu_set_rg_vgp2_vosel(6);}
    else if(volt == VOL_2000){ upmu_set_rg_vgp2_vosel(7);}
    else{
        xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "Error Setting %d. DO nothing.\r\n", volt);
    }
}

void dct_pmic_VGP3_sel(kal_uint32 volt)
{
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "****[dct_pmic_VGP3_sel] value=%d \n", volt);

    if(volt == VOL_DEFAULT)     { upmu_set_rg_vgp3_vosel(0);}
    else if(volt == VOL_1200){ upmu_set_rg_vgp3_vosel(0);}
    else if(volt == VOL_1300){ upmu_set_rg_vgp3_vosel(1);}
    else if(volt == VOL_1500){ upmu_set_rg_vgp3_vosel(2);}
    else if(volt == VOL_1800){ upmu_set_rg_vgp3_vosel(3);}
    else{
        xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "Error Setting %d. DO nothing.\r\n", volt);
    }
}

void dct_pmic_VCAM_AF_sel(kal_uint32 volt)
{
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "****[dct_pmic_VCAM_AF_sel] value=%d \n", volt);

    if(volt == VOL_DEFAULT)  { upmu_set_rg_vcam_af_vosel(4);}
    else if(volt == VOL_1200){ upmu_set_rg_vcam_af_vosel(0);}
    else if(volt == VOL_1300){ upmu_set_rg_vcam_af_vosel(1);}
    else if(volt == VOL_1500){ upmu_set_rg_vcam_af_vosel(2);}
    else if(volt == VOL_1800){ upmu_set_rg_vcam_af_vosel(3);}
    else if(volt == VOL_2000){ upmu_set_rg_vcam_af_vosel(4);}
    else if(volt == VOL_2800){ upmu_set_rg_vcam_af_vosel(5);}
    else if(volt == VOL_3000){ upmu_set_rg_vcam_af_vosel(6);}
    else if(volt == VOL_3300){ upmu_set_rg_vcam_af_vosel(7);}	
    else{
        xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "Error Setting %d. DO nothing.\r\n", volt);
    }
}

void dct_pmic_VSIM1_sel(kal_uint32 volt)
{
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "****[dct_pmic_VSIM1_sel] value=%d \n", volt);

    if(volt == VOL_DEFAULT)     { upmu_set_rg_vsim1_vosel(0);}
    else if(volt == VOL_1800){ upmu_set_rg_vsim1_vosel(0);}
    else if(volt == VOL_3000){ upmu_set_rg_vsim1_vosel(1);}
    else{
        xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "Error Setting %d. DO nothing.\r\n", volt);
    }
}

void dct_pmic_VSIM2_sel(kal_uint32 volt)
{
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "****[dct_pmic_VSIM2_sel] value=%d \n", volt);

    if(volt == VOL_DEFAULT)     { upmu_set_rg_vsim2_vosel(0);}
    else if(volt == VOL_1800){ upmu_set_rg_vsim2_vosel(0);}
    else if(volt == VOL_3000){ upmu_set_rg_vsim2_vosel(1);}
    else{
        xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "Error Setting %d. DO nothing.\r\n", volt);
    }
}

void dct_pmic_VIBR_sel(kal_uint32 volt)
{
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "****[dct_pmic_VIBR_sel] value=%d \n", volt);

    if(volt == VOL_DEFAULT)     { upmu_set_rg_vibr_vosel(0);}
    else if(volt == VOL_1200){ upmu_set_rg_vibr_vosel(0);}
    else if(volt == VOL_1300){ upmu_set_rg_vibr_vosel(1);}
    else if(volt == VOL_1500){ upmu_set_rg_vibr_vosel(2);}
    else if(volt == VOL_1800){ upmu_set_rg_vibr_vosel(3);}
    else if(volt == VOL_2000){ upmu_set_rg_vibr_vosel(4);}
    else if(volt == VOL_2800){ upmu_set_rg_vibr_vosel(5);}
    else if(volt == VOL_3000){ upmu_set_rg_vibr_vosel(6);}
    else if(volt == VOL_3300){ upmu_set_rg_vibr_vosel(7);}
    else{
        xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "Error Setting %d. DO nothing.\r\n", volt);
    }
}

void dct_pmic_VM_sel(kal_uint32 volt)
{
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "****[dct_pmic_VM_sel] value=%d \n", volt);

    if(volt == VOL_DEFAULT)     { upmu_set_rg_vm_vosel(0);}
    else if(volt == VOL_1200){ upmu_set_rg_vm_vosel(0);}
    else if(volt == VOL_1350){ upmu_set_rg_vm_vosel(1);}
    else if(volt == VOL_1500){ upmu_set_rg_vm_vosel(2);}
    else if(volt == VOL_1800){ upmu_set_rg_vm_vosel(3);}
    else{
        xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "Error Setting %d. DO nothing.\r\n", volt);
    }
}

void dct_pmic_VCAMD_sel(kal_uint32 volt)
{
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "****[dct_pmic_VCAMD_sel] value=%d \n", volt);

    if(volt == VOL_DEFAULT)     { upmu_set_rg_vcamd_vosel(0);}
    else if(volt == VOL_1200){ upmu_set_rg_vcamd_vosel(0);}
    else if(volt == VOL_1300){ upmu_set_rg_vcamd_vosel(1);}  //sync to latest mt6323 datasheet
    else if(volt == VOL_1500){ upmu_set_rg_vcamd_vosel(2);}
    else if(volt == VOL_1800){ upmu_set_rg_vcamd_vosel(3);}
    else{
        xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "Error Setting %d. DO nothing.\r\n", volt);
    }
}

//A-LDO
void dct_pmic_VCAMA_sel(kal_uint32 volt)
{
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "****[dct_pmic_VCAMA_sel] value=%d \n", volt);

    if(volt == VOL_DEFAULT)     { upmu_set_rg_vcama_vosel(3);}
    else if(volt == VOL_1500){ upmu_set_rg_vcama_vosel(0);}
    else if(volt == VOL_1800){ upmu_set_rg_vcama_vosel(1);}
    else if(volt == VOL_2500){ upmu_set_rg_vcama_vosel(2);}
    else if(volt == VOL_2800){ upmu_set_rg_vcama_vosel(3);}
    else{
        xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "Error Setting %d. DO nothing.\r\n", volt);
    }
}

void dct_pmic_VCN33_sel(kal_uint32 volt)
{
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "****[dct_pmic_VCN33_sel] value=%d \n", volt);

    if(volt == VOL_DEFAULT)     { upmu_set_rg_vcn33_vosel(3);}
    else if(volt == VOL_3300){ upmu_set_rg_vcn33_vosel(0);}
    else if(volt == VOL_3400){ upmu_set_rg_vcn33_vosel(1);}
    else if(volt == VOL_3500){ upmu_set_rg_vcn33_vosel(2);}
    else if(volt == VOL_3600){ upmu_set_rg_vcn33_vosel(3);}
    else{
        xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "Error Setting %d. DO nothing.\r\n", volt);
    }
}

/*
	//MT6323 Digital LDO
	MT6323_POWER_LDO_VIO28,
	MT6323_POWER_LDO_VUSB,
	MT6323_POWER_LDO_VMC,
	MT6323_POWER_LDO_VMCH,
	MT6323_POWER_LDO_VEMC_3V3,
	MT6323_POWER_LDO_VGP1,
	MT6323_POWER_LDO_VGP2,
	MT6323_POWER_LDO_VGP3,
	MT6323_POWER_LDO_VCN_1V8,
	MT6323_POWER_LDO_VSIM1,
	MT6323_POWER_LDO_VSIM2,
	MT6323_POWER_LDO_VRTC,
	MT6323_POWER_LDO_VCAM_AF,
	MT6323_POWER_LDO_VIBR,
	MT6323_POWER_LDO_VM,
	MT6323_POWER_LDO_VRF18,
	MT6323_POWER_LDO_VIO18,
	MT6323_POWER_LDO_VCAMD,
	MT6323_POWER_LDO_VCAM_IO,

	//MT6323 Analog LDO
	MT6323_POWER_LDO_VTCXO,
	MT6323_POWER_LDO_VA,
	MT6323_POWER_LDO_VCAMA,
	MT6323_POWER_LDO_VCN33,
	MT6323_POWER_LDO_VCN28,
*/

//==============================================================================
// EM
//==============================================================================
static ssize_t show_BUCK_VPROC_STATUS(struct device *dev,struct device_attribute *attr, char *buf)
{
    kal_uint32 ret_value=0;
    
    kal_uint32 ret=0;
    kal_uint32 reg_address=0x21A;
    kal_uint32 reg_val=0;
    
    ret = pmic_read_interface(reg_address, &reg_val, 0x01, 13);
    ret_value = reg_val;
    
    xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[EM] BUCK_VPROC_STATUS : %d\n", ret_value);
    return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_BUCK_VPROC_STATUS(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
    xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[EM] Not Support Write Function\n");    
    return size;
}
static DEVICE_ATTR(BUCK_VPROC_STATUS, 0664, show_BUCK_VPROC_STATUS, store_BUCK_VPROC_STATUS);


static ssize_t show_BUCK_VSYS_STATUS(struct device *dev,struct device_attribute *attr, char *buf)
{
    kal_uint32 ret_value=0;
    
    kal_uint32 ret=0;
    kal_uint32 reg_address=0x240;
    kal_uint32 reg_val=0;
    
    ret = pmic_read_interface(reg_address, &reg_val, 0x01, 13);
    ret_value = reg_val;
    
    xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[EM] BUCK_VSYS_STATUS : %d\n", ret_value);
    return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_BUCK_VSYS_STATUS(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
    xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[EM] Not Support Write Function\n");    
    return size;
}
static DEVICE_ATTR(BUCK_VSYS_STATUS, 0664, show_BUCK_VSYS_STATUS, store_BUCK_VSYS_STATUS);


static ssize_t show_BUCK_VPA_STATUS(struct device *dev,struct device_attribute *attr, char *buf)
{
    kal_uint32 ret_value=0;
    
    kal_uint32 ret=0;
    kal_uint32 reg_address=0x30e;
    kal_uint32 reg_val=0;
    
    ret = pmic_read_interface(reg_address, &reg_val, 0x01, 13);
    ret_value = reg_val;
    
    xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[EM] BUCK_VPA_STATUS : %d\n", ret_value);
    return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_BUCK_VPA_STATUS(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
    xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[EM] Not Support Write Function\n");    
    return size;
}
static DEVICE_ATTR(BUCK_VPA_STATUS, 0664, show_BUCK_VPA_STATUS, store_BUCK_VPA_STATUS);


static ssize_t show_LDO_VTCXO_STATUS(struct device *dev,struct device_attribute *attr, char *buf)
{
    kal_uint32 ret_value=0;
    
    kal_uint32 ret=0;
    kal_uint32 reg_address=0x402;
    kal_uint32 reg_val=0;
    
    ret = pmic_read_interface(reg_address, &reg_val, 0x01, 15);
    ret_value = reg_val;
    
    xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[EM] LDO_VTCXO_STATUS : %d\n", ret_value);
    return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_LDO_VTCXO_STATUS(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
    xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[EM] Not Support Write Function\n");    
    return size;
}
static DEVICE_ATTR(LDO_VTCXO_STATUS, 0664, show_LDO_VTCXO_STATUS, store_LDO_VTCXO_STATUS);


static ssize_t show_LDO_VA_STATUS(struct device *dev,struct device_attribute *attr, char *buf)
{
    kal_uint32 ret_value=0;
    
    kal_uint32 ret=0;
    kal_uint32 reg_address=0x404;
    kal_uint32 reg_val=0;
    
    ret = pmic_read_interface(reg_address, &reg_val, 0x01, 15);
    ret_value = reg_val;
    
    xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[EM] LDO_VA_STATUS : %d\n", ret_value);
    return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_LDO_VA_STATUS(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
    xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[EM] Not Support Write Function\n");    
    return size;
}
static DEVICE_ATTR(LDO_VA_STATUS, 0664, show_LDO_VA_STATUS, store_LDO_VA_STATUS);


static ssize_t show_LDO_VCAMA_STATUS(struct device *dev,struct device_attribute *attr, char *buf)
{
    kal_uint32 ret_value=0;
    
    kal_uint32 ret=0;
    kal_uint32 reg_address=0x408;
    kal_uint32 reg_val=0;
    
    ret = pmic_read_interface(reg_address, &reg_val, 0x01, 15);
    ret_value = reg_val;
    
    xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[EM] LDO_VCAMA_STATUS : %d\n", ret_value);
    return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_LDO_VCAMA_STATUS(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
    xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[EM] Not Support Write Function\n");    
    return size;
}
static DEVICE_ATTR(LDO_VCAMA_STATUS, 0664, show_LDO_VCAMA_STATUS, store_LDO_VCAMA_STATUS);


static ssize_t show_LDO_VCN28_STATUS(struct device *dev,struct device_attribute *attr, char *buf)
{
    kal_uint32 ret_value=0;
    
    kal_uint32 ret=0;
    kal_uint32 reg_address=0x41C;
    kal_uint32 reg_val=0;
    
    ret = pmic_read_interface(reg_address, &reg_val, 0x01, 15);
    ret_value = reg_val;
    
    xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[EM] LDO_VCN28_STATUS : %d\n", ret_value);
    return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_LDO_VCN28_STATUS(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
    xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[EM] Not Support Write Function\n");    
    return size;
}
static DEVICE_ATTR(LDO_VCN28_STATUS, 0664, show_LDO_VCN28_STATUS, store_LDO_VCN28_STATUS);


static ssize_t show_LDO_VCN33_STATUS(struct device *dev,struct device_attribute *attr, char *buf)
{
    kal_uint32 ret_value=0;
    
    kal_uint32 ret=0;
    kal_uint32 reg_address=0x418;
    kal_uint32 reg_val=0;
    
    ret = pmic_read_interface(reg_address, &reg_val, 0x01, 15);
    ret_value = reg_val;
    
    xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[EM] LDO_VCN33_STATUS : %d\n", ret_value);
    return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_LDO_VCN33_STATUS(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
    xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[EM] Not Support Write Function\n");    
    return size;
}
static DEVICE_ATTR(LDO_VCN33_STATUS, 0664, show_LDO_VCN33_STATUS, store_LDO_VCN33_STATUS);

static ssize_t show_LDO_VIO28_STATUS(struct device *dev,struct device_attribute *attr, char *buf)
{
    kal_uint32 ret_value=0;
    
    kal_uint32 ret=0;
    kal_uint32 reg_address=0x500;
    kal_uint32 reg_val=0;
    
    ret = pmic_read_interface(reg_address, &reg_val, 0x01, 15);
    ret_value = reg_val;
    
    xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[EM] LDO_VIO28_STATUS : %d\n", ret_value);
    return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_LDO_VIO28_STATUS(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
    xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[EM] Not Support Write Function\n");    
    return size;
}
static DEVICE_ATTR(LDO_VIO28_STATUS, 0664, show_LDO_VIO28_STATUS, store_LDO_VIO28_STATUS);


static ssize_t show_LDO_VUSB_STATUS(struct device *dev,struct device_attribute *attr, char *buf)
{
    kal_uint32 ret_value=0;
    
    kal_uint32 ret=0;
    kal_uint32 reg_address=0x502;
    kal_uint32 reg_val=0;
    
    ret = pmic_read_interface(reg_address, &reg_val, 0x01, 15);
    ret_value = reg_val;
    
    xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[EM] LDO_VUSB_STATUS : %d\n", ret_value);
    return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_LDO_VUSB_STATUS(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
    xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[EM] Not Support Write Function\n");    
    return size;
}
static DEVICE_ATTR(LDO_VUSB_STATUS, 0664, show_LDO_VUSB_STATUS, store_LDO_VUSB_STATUS);


static ssize_t show_LDO_VMC_STATUS(struct device *dev,struct device_attribute *attr, char *buf)
{
    kal_uint32 ret_value=0;
    
    kal_uint32 ret=0;
    kal_uint32 reg_address=0x504;
    kal_uint32 reg_val=0;
    
    ret = pmic_read_interface(reg_address, &reg_val, 0x01, 15);
    ret_value = reg_val;
    
    xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[EM] LDO_VMC_STATUS : %d\n", ret_value);
    return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_LDO_VMC_STATUS(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
    xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[EM] Not Support Write Function\n");    
    return size;
}
static DEVICE_ATTR(LDO_VMC_STATUS, 0664, show_LDO_VMC_STATUS, store_LDO_VMC_STATUS);


static ssize_t show_LDO_VMCH_STATUS(struct device *dev,struct device_attribute *attr, char *buf)
{
    kal_uint32 ret_value=0;
    
    kal_uint32 ret=0;
    kal_uint32 reg_address=0x506;
    kal_uint32 reg_val=0;
    
    ret = pmic_read_interface(reg_address, &reg_val, 0x01, 15);
    ret_value = reg_val;
    
    xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[EM] LDO_VMCH_STATUS : %d\n", ret_value);
    return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_LDO_VMCH_STATUS(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
    xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[EM] Not Support Write Function\n");    
    return size;
}
static DEVICE_ATTR(LDO_VMCH_STATUS, 0664, show_LDO_VMCH_STATUS, store_LDO_VMCH_STATUS);


static ssize_t show_LDO_VEMC_3V3_STATUS(struct device *dev,struct device_attribute *attr, char *buf)
{
    kal_uint32 ret_value=0;
    
    kal_uint32 ret=0;
    kal_uint32 reg_address=0x508;
    kal_uint32 reg_val=0;
    
    ret = pmic_read_interface(reg_address, &reg_val, 0x01, 15);
    ret_value = reg_val;
    
    xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[EM] LDO_VEMC_3V3_STATUS : %d\n", ret_value);
    return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_LDO_VEMC_3V3_STATUS(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
    xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[EM] Not Support Write Function\n");    
    return size;
}
static DEVICE_ATTR(LDO_VEMC_3V3_STATUS, 0664, show_LDO_VEMC_3V3_STATUS, store_LDO_VEMC_3V3_STATUS);


static ssize_t show_LDO_VGP1_STATUS(struct device *dev,struct device_attribute *attr, char *buf)
{
    kal_uint32 ret_value=0;
    
    kal_uint32 ret=0;
    kal_uint32 reg_address=0x50a;
    kal_uint32 reg_val=0;
    
    ret = pmic_read_interface(reg_address, &reg_val, 0x01, 15);
    ret_value = reg_val;
    
    xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[EM] LDO_VGP1_STATUS : %d\n", ret_value);
    return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_LDO_VGP1_STATUS(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
    xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[EM] Not Support Write Function\n");    
    return size;
}
static DEVICE_ATTR(LDO_VGP1_STATUS, 0664, show_LDO_VGP1_STATUS, store_LDO_VGP1_STATUS);


static ssize_t show_LDO_VGP2_STATUS(struct device *dev,struct device_attribute *attr, char *buf)
{
    kal_uint32 ret_value=0;
    
    kal_uint32 ret=0;
    kal_uint32 reg_address=0x50c;
    kal_uint32 reg_val=0;
    
    ret = pmic_read_interface(reg_address, &reg_val, 0x01, 15);
    ret_value = reg_val;
    
    xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[EM] LDO_VGP2_STATUS : %d\n", ret_value);
    return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_LDO_VGP2_STATUS(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
    xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[EM] Not Support Write Function\n");    
    return size;
}
static DEVICE_ATTR(LDO_VGP2_STATUS, 0664, show_LDO_VGP2_STATUS, store_LDO_VGP2_STATUS);


static ssize_t show_LDO_VGP3_STATUS(struct device *dev,struct device_attribute *attr, char *buf)
{
    kal_uint32 ret_value=0;
    
    kal_uint32 ret=0;
    kal_uint32 reg_address=0x50e;
    kal_uint32 reg_val=0;
    
    ret = pmic_read_interface(reg_address, &reg_val, 0x01, 15);
    ret_value = reg_val;
    
    xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[EM] LDO_VGP3_STATUS : %d\n", ret_value);
    return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_LDO_VGP3_STATUS(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
    xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[EM] Not Support Write Function\n");    
    return size;
}
static DEVICE_ATTR(LDO_VGP3_STATUS, 0664, show_LDO_VGP3_STATUS, store_LDO_VGP3_STATUS);


static ssize_t show_LDO_VCN_1V8_STATUS(struct device *dev,struct device_attribute *attr, char *buf)
{
    kal_uint32 ret_value=0;
    
    kal_uint32 ret=0;
    kal_uint32 reg_address=0x512;
    kal_uint32 reg_val=0;
    
    ret = pmic_read_interface(reg_address, &reg_val, 0x01, 15);
    ret_value = reg_val;
    
    xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[EM] LDO_VCN_1V8_STATUS : %d\n", ret_value);
    return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_LDO_VCN_1V8_STATUS(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
    xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[EM] Not Support Write Function\n");    
    return size;
}
static DEVICE_ATTR(LDO_VCN_1V8_STATUS, 0664, show_LDO_VCN_1V8_STATUS, store_LDO_VCN_1V8_STATUS);


static ssize_t show_LDO_VSIM1_STATUS(struct device *dev,struct device_attribute *attr, char *buf)
{
    kal_uint32 ret_value=0;
    
    kal_uint32 ret=0;
    kal_uint32 reg_address=0x516;
    kal_uint32 reg_val=0;
    
    ret = pmic_read_interface(reg_address, &reg_val, 0x01, 15);
    ret_value = reg_val;
    
    xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[EM] LDO_VSIM1_STATUS : %d\n", ret_value);
    return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_LDO_VSIM1_STATUS(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
    xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[EM] Not Support Write Function\n");    
    return size;
}
static DEVICE_ATTR(LDO_VSIM1_STATUS, 0664, show_LDO_VSIM1_STATUS, store_LDO_VSIM1_STATUS);


static ssize_t show_LDO_VSIM2_STATUS(struct device *dev,struct device_attribute *attr, char *buf)
{
    kal_uint32 ret_value=0;
    
    kal_uint32 ret=0;
    kal_uint32 reg_address=0x518;
    kal_uint32 reg_val=0;
    
    ret = pmic_read_interface(reg_address, &reg_val, 0x01, 15);
    ret_value = reg_val;
    
    xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[EM] LDO_VSIM2_STATUS : %d\n", ret_value);
    return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_LDO_VSIM2_STATUS(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
    xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[EM] Not Support Write Function\n");    
    return size;
}
static DEVICE_ATTR(LDO_VSIM2_STATUS, 0664, show_LDO_VSIM2_STATUS, store_LDO_VSIM2_STATUS);


static ssize_t show_LDO_VRTC_STATUS(struct device *dev,struct device_attribute *attr, char *buf)
{
    kal_uint32 ret_value=0;
    
    kal_uint32 ret=0;
    kal_uint32 reg_address=0x51a;
    kal_uint32 reg_val=0;
    
    ret = pmic_read_interface(reg_address, &reg_val, 0x01, 15);
    ret_value = reg_val;
    
    xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[EM] LDO_VRTC_STATUS : %d\n", ret_value);
    return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_LDO_VRTC_STATUS(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
    xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[EM] Not Support Write Function\n");    
    return size;
}
static DEVICE_ATTR(LDO_VRTC_STATUS, 0664, show_LDO_VRTC_STATUS, store_LDO_VRTC_STATUS);


static ssize_t show_LDO_VCAM_AF_STATUS(struct device *dev,struct device_attribute *attr, char *buf)
{
    kal_uint32 ret_value=0;
    
    kal_uint32 ret=0;
    kal_uint32 reg_address=0x536;
    kal_uint32 reg_val=0;
    
    ret = pmic_read_interface(reg_address, &reg_val, 0x01, 15);
    ret_value = reg_val;
    
    xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[EM] LDO_VCAM_AF_STATUS : %d\n", ret_value);
    return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_LDO_VCAM_AF_STATUS(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
    xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[EM] Not Support Write Function\n");    
    return size;
}
static DEVICE_ATTR(LDO_VCAM_AF_STATUS, 0664, show_LDO_VCAM_AF_STATUS, store_LDO_VCAM_AF_STATUS);


static ssize_t show_LDO_VIBR_STATUS(struct device *dev,struct device_attribute *attr, char *buf)
{
    kal_uint32 ret_value=0;
    
    kal_uint32 ret=0;
    kal_uint32 reg_address=0x542;
    kal_uint32 reg_val=0;
    
    ret = pmic_read_interface(reg_address, &reg_val, 0x01, 15);
    ret_value = reg_val;
    
    xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[EM] LDO_VIBR_STATUS : %d\n", ret_value);
    return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_LDO_VIBR_STATUS(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
    xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[EM] Not Support Write Function\n");    
    return size;
}
static DEVICE_ATTR(LDO_VIBR_STATUS, 0664, show_LDO_VIBR_STATUS, store_LDO_VIBR_STATUS);


static ssize_t show_LDO_VM_STATUS(struct device *dev,struct device_attribute *attr, char *buf)
{
    kal_uint32 ret_value=0;
    
    kal_uint32 ret=0;
    kal_uint32 reg_address=0x552;
    kal_uint32 reg_val=0;
    
    ret = pmic_read_interface(reg_address, &reg_val, 0x01, 15);
    ret_value = reg_val;
    
    xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[EM] LDO_VM_STATUS : %d\n", ret_value);
    return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_LDO_VM_STATUS(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
    xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[EM] Not Support Write Function\n");    
    return size;
}
static DEVICE_ATTR(LDO_VM_STATUS, 0664, show_LDO_VM_STATUS, store_LDO_VM_STATUS);


static ssize_t show_LDO_VRF18_STATUS(struct device *dev,struct device_attribute *attr, char *buf)
{
    kal_uint32 ret_value=0;
    
    kal_uint32 ret=0;
    kal_uint32 reg_address=0x54e;
    kal_uint32 reg_val=0;
    
    ret = pmic_read_interface(reg_address, &reg_val, 0x01, 15);
    ret_value = reg_val;
    
    xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[EM] LDO_VRF18_STATUS : %d\n", ret_value);
    return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_LDO_VRF18_STATUS(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
    xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[EM] Not Support Write Function\n");    
    return size;
}
static DEVICE_ATTR(LDO_VRF18_STATUS, 0664, show_LDO_VRF18_STATUS, store_LDO_VRF18_STATUS);


static ssize_t show_LDO_VIO18_STATUS(struct device *dev,struct device_attribute *attr, char *buf)
{
    kal_uint32 ret_value=0;
    
    kal_uint32 ret=0;
    kal_uint32 reg_address=0x556;
    kal_uint32 reg_val=0;
    
    ret = pmic_read_interface(reg_address, &reg_val, 0x01, 15);
    ret_value = reg_val;
    
    xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[EM] LDO_VIO18_STATUS : %d\n", ret_value);
    return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_LDO_VIO18_STATUS(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
    xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[EM] Not Support Write Function\n");    
    return size;
}
static DEVICE_ATTR(LDO_VIO18_STATUS, 0664, show_LDO_VIO18_STATUS, store_LDO_VIO18_STATUS);



static ssize_t show_LDO_VCAMD_STATUS(struct device *dev,struct device_attribute *attr, char *buf)
{
    kal_uint32 ret_value=0;
    
    kal_uint32 ret=0;
    kal_uint32 reg_address=0x55a;
    kal_uint32 reg_val=0;
    
    ret = pmic_read_interface(reg_address, &reg_val, 0x01, 15);
    ret_value = reg_val;
    
    xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[EM] LDO_VCAMD_STATUS : %d\n", ret_value);
    return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_LDO_VCAMD_STATUS(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
    xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[EM] Not Support Write Function\n");    
    return size;
}
static DEVICE_ATTR(LDO_VCAMD_STATUS, 0664, show_LDO_VCAMD_STATUS, store_LDO_VCAMD_STATUS);


static ssize_t show_LDO_VCAM_IO_STATUS(struct device *dev,struct device_attribute *attr, char *buf)
{
    kal_uint32 ret_value=0;
    
    kal_uint32 ret=0;
    kal_uint32 reg_address=0x55e;
    kal_uint32 reg_val=0;
    
    ret = pmic_read_interface(reg_address, &reg_val, 0x01, 15);
    ret_value = reg_val;
    
    xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[EM] LDO_VCAM_IO_STATUS : %d\n", ret_value);
    return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_LDO_VCAM_IO_STATUS(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
    xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[EM] Not Support Write Function\n");    
    return size;
}
static DEVICE_ATTR(LDO_VCAM_IO_STATUS, 0664, show_LDO_VCAM_IO_STATUS, store_LDO_VCAM_IO_STATUS);



//voltage
static ssize_t show_BUCK_VPROC_VOLTAGE(struct device *dev,struct device_attribute *attr, char *buf)
{
    kal_uint32 ret_value=0;
    
    kal_uint32 ret=0;
    kal_uint32 reg_address=0x224;
    kal_uint32 reg_val=0;
    
    ret = pmic_read_interface(reg_address, &reg_val, 0x7F, 0);
    ret_value = 70000 + (reg_val*625);
    ret_value = ret_value / 100;
    
    xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[EM] BUCK_VPROC_VOLTAGE : %d\n", ret_value);
    return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_BUCK_VPROC_VOLTAGE(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
    xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[EM] Not Support Write Function\n");    
    return size;
}
static DEVICE_ATTR(BUCK_VPROC_VOLTAGE, 0664, show_BUCK_VPROC_VOLTAGE, store_BUCK_VPROC_VOLTAGE);


static ssize_t show_BUCK_VSYS_VOLTAGE(struct device *dev,struct device_attribute *attr, char *buf)
{
    kal_uint32 ret_value=0;
    
    kal_uint32 ret=0;
    kal_uint32 reg_address=0x24a;
    kal_uint32 reg_val=0;
    
    ret = pmic_read_interface(reg_address, &reg_val, 0x7F, 0);
    ret_value = 140000 + (reg_val*1250);
    ret_value = ret_value / 100;
    
    xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[EM] BUCK_VSYS_VOLTAGE : %d\n", ret_value);
    return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_BUCK_VSYS_VOLTAGE(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
    xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[EM] Not Support Write Function\n");    
    return size;
}
static DEVICE_ATTR(BUCK_VSYS_VOLTAGE, 0664, show_BUCK_VSYS_VOLTAGE, store_BUCK_VSYS_VOLTAGE);


static ssize_t show_BUCK_VPA_VOLTAGE(struct device *dev,struct device_attribute *attr, char *buf)
{
    kal_uint32 ret_value=0;
    
    kal_uint32 ret=0;
    kal_uint32 reg_address=0x318;
    kal_uint32 reg_val=0;
    
    ret = pmic_read_interface(reg_address, &reg_val, 0x3f, 0);
    ret_value = 50000 + (reg_val*5000);
    ret_value = ret_value / 100;
    
    xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[EM] BUCK_VPA_VOLTAGE : %d\n", ret_value);
    return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_BUCK_VPA_VOLTAGE(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
    xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[EM] Not Support Write Function\n");    
    return size;
}
static DEVICE_ATTR(BUCK_VPA_VOLTAGE, 0664, show_BUCK_VPA_VOLTAGE, store_BUCK_VPA_VOLTAGE);





static ssize_t show_LDO_VMC_VOLTAGE(struct device *dev,struct device_attribute *attr, char *buf)
{
    kal_uint32 ret_value=0;
    
    kal_uint32 ret=0;
    kal_uint32 reg_address=0x52A;
    kal_uint32 reg_val=0;
    
    ret = pmic_read_interface(reg_address, &reg_val, 0x01, 4);
    if(reg_val == 0)
        ret_value = 1800;
    else if(reg_val == 1)
        ret_value = 3300;         
    
    xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[EM] LDO_VMC_VOLTAGE : %d\n", ret_value);
    return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_LDO_VMC_VOLTAGE(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
    xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[EM] Not Support Write Function\n");    
    return size;
}
static DEVICE_ATTR(LDO_VMC_VOLTAGE, 0664, show_LDO_VMC_VOLTAGE, store_LDO_VMC_VOLTAGE);



static ssize_t show_LDO_VMCH_VOLTAGE(struct device *dev,struct device_attribute *attr, char *buf)
{
    kal_uint32 ret_value=0;
    
    kal_uint32 ret=0;
    kal_uint32 reg_address=0x52c;
    kal_uint32 reg_val=0;
    
    ret = pmic_read_interface(reg_address, &reg_val, 0x01, 7);
    if(reg_val == 0)
        ret_value = 3000;
    else if(reg_val == 1)
        ret_value = 3300;         
    
    xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[EM] LDO_VMCH_VOLTAGE : %d\n", ret_value);
    return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_LDO_VMCH_VOLTAGE(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
    xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[EM] Not Support Write Function\n");    
    return size;
}
static DEVICE_ATTR(LDO_VMCH_VOLTAGE, 0664, show_LDO_VMCH_VOLTAGE, store_LDO_VMCH_VOLTAGE);


static ssize_t show_LDO_VEMC_3V3_VOLTAGE(struct device *dev,struct device_attribute *attr, char *buf)
{
    kal_uint32 ret_value=0;
    
    kal_uint32 ret=0;
    kal_uint32 reg_address=0x52e;
    kal_uint32 reg_val=0;
    
    ret = pmic_read_interface(reg_address, &reg_val, 0x01, 7);
    if(reg_val == 0)
        ret_value = 3000;
    else if(reg_val == 1)
        ret_value = 3300;         
    
    xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[EM] LDO_VEMC_3V3_VOLTAGE : %d\n", ret_value);
    return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_LDO_VEMC_3V3_VOLTAGE(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
    xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[EM] Not Support Write Function\n");    
    return size;
}
static DEVICE_ATTR(LDO_VEMC_3V3_VOLTAGE, 0664, show_LDO_VEMC_3V3_VOLTAGE, store_LDO_VEMC_3V3_VOLTAGE);


static ssize_t show_LDO_VGP1_VOLTAGE(struct device *dev,struct device_attribute *attr, char *buf)
{
    kal_uint32 ret_value=0;
    
    kal_uint32 ret=0;
    kal_uint32 reg_address=0x530;
    kal_uint32 reg_val=0;
    
    ret = pmic_read_interface(reg_address, &reg_val, 0x07, 5);
    if(reg_val == 0)
        ret_value = 1200;
    else if(reg_val == 1)
        ret_value = 1300;         
    else if(reg_val == 2)
        ret_value = 1500;   
    else if(reg_val == 3)
        ret_value = 1800;   
    else if(reg_val == 4)
        ret_value = 2000;   
    else if(reg_val == 5)
        ret_value = 2800;   
    else if(reg_val == 6)
        ret_value = 3000;   	
    else if(reg_val == 7)
        ret_value = 3300;       
    
    xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[EM] LDO_VGP1_VOLTAGE : %d\n", ret_value);
    return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_LDO_VGP1_VOLTAGE(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
    xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[EM] Not Support Write Function\n");    
    return size;
}
static DEVICE_ATTR(LDO_VGP1_VOLTAGE, 0664, show_LDO_VGP1_VOLTAGE, store_LDO_VGP1_VOLTAGE);


static ssize_t show_LDO_VGP2_VOLTAGE(struct device *dev,struct device_attribute *attr, char *buf)
{
    kal_uint32 ret_value=0;
    
    kal_uint32 ret=0;
    kal_uint32 reg_address=0x532;
    kal_uint32 reg_val=0;
    
    ret = pmic_read_interface(reg_address, &reg_val, 0x07, 5);
    if(reg_val == 0)
        ret_value = 1200;
    else if(reg_val == 1)
        ret_value = 1300;         
    else if(reg_val == 2)
        ret_value = 1500;   
    else if(reg_val == 3)
        ret_value = 1800;   
    else if(reg_val == 4)
        ret_value = 2500;   
    else if(reg_val == 5)
        ret_value = 2800;   
    else if(reg_val == 6)
        ret_value = 3000;   	
    else if(reg_val == 7)
        ret_value = 2000;   


	
    xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[EM] LDO_VGP2_VOLTAGE : %d\n", ret_value);
    return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_LDO_VGP2_VOLTAGE(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
    xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[EM] Not Support Write Function\n");    
    return size;
}
static DEVICE_ATTR(LDO_VGP2_VOLTAGE, 0664, show_LDO_VGP2_VOLTAGE, store_LDO_VGP2_VOLTAGE);


static ssize_t show_LDO_VGP3_VOLTAGE(struct device *dev,struct device_attribute *attr, char *buf)
{
    kal_uint32 ret_value=0;
    
    kal_uint32 ret=0;
    kal_uint32 reg_address=0x534;
    kal_uint32 reg_val=0;
    
    ret = pmic_read_interface(reg_address, &reg_val, 0x07, 5);
    if(reg_val == 0)
        ret_value = 1200;
    else if(reg_val == 1)
        ret_value = 1300;         
    else if(reg_val == 2)
        ret_value = 1500;   
    else if(reg_val == 3)
        ret_value = 1800;   

	
    xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[EM] LDO_VGP3_VOLTAGE : %d\n", ret_value);
    return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_LDO_VGP3_VOLTAGE(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
    xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[EM] Not Support Write Function\n");    
    return size;
}
static DEVICE_ATTR(LDO_VGP3_VOLTAGE, 0664, show_LDO_VGP3_VOLTAGE, store_LDO_VGP3_VOLTAGE);


static ssize_t show_LDO_VSIM1_VOLTAGE(struct device *dev,struct device_attribute *attr, char *buf)
{
    kal_uint32 ret_value=0;
    
    kal_uint32 ret=0;
    kal_uint32 reg_address=0x53C;
    kal_uint32 reg_val=0;
    
    ret = pmic_read_interface(reg_address, &reg_val, 0x01, 5);
    if(reg_val == 0)
        ret_value = 1800;
    else if(reg_val == 1)
        ret_value = 3000;         
    
    xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[EM] LDO_VSIM1_VOLTAGE : %d\n", ret_value);
    return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_LDO_VSIM1_VOLTAGE(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
    xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[EM] Not Support Write Function\n");    
    return size;
}
static DEVICE_ATTR(LDO_VSIM1_VOLTAGE, 0664, show_LDO_VSIM1_VOLTAGE, store_LDO_VSIM1_VOLTAGE);


static ssize_t show_LDO_VSIM2_VOLTAGE(struct device *dev,struct device_attribute *attr, char *buf)
{
    kal_uint32 ret_value=0;
    
    kal_uint32 ret=0;
    kal_uint32 reg_address=0x53e;
    kal_uint32 reg_val=0;
    
    ret = pmic_read_interface(reg_address, &reg_val, 0x01, 5);
    if(reg_val == 0)
        ret_value = 1800;
    else if(reg_val == 1)
        ret_value = 3000;         
    
    xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[EM] LDO_VSIM2_VOLTAGE : %d\n", ret_value);
    return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_LDO_VSIM2_VOLTAGE(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
    xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[EM] Not Support Write Function\n");    
    return size;
}
static DEVICE_ATTR(LDO_VSIM2_VOLTAGE, 0664, show_LDO_VSIM2_VOLTAGE, store_LDO_VSIM2_VOLTAGE);


static ssize_t show_LDO_VCAM_AF_VOLTAGE(struct device *dev,struct device_attribute *attr, char *buf)
{
    kal_uint32 ret_value=0;
    
    kal_uint32 ret=0;
    kal_uint32 reg_address=0x538;
    kal_uint32 reg_val=0;
    
    ret = pmic_read_interface(reg_address, &reg_val, 0x07, 5);
    if(reg_val == 0)
        ret_value = 1200;
    else if(reg_val == 1)
        ret_value = 1300;         
    else if(reg_val == 2)
        ret_value = 1500;   
    else if(reg_val == 3)
        ret_value = 1800;   
    else if(reg_val == 4)
        ret_value = 2000;   
    else if(reg_val == 5)
        ret_value = 2800;   
    else if(reg_val == 6)
        ret_value = 3000;   	
    else if(reg_val == 7)
        ret_value = 3300;          
    
    xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[EM] LDO_VCAM_AF_VOLTAGE : %d\n", ret_value);
    return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_LDO_VCAM_AF_VOLTAGE(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
    xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[EM] Not Support Write Function\n");    
    return size;
}
static DEVICE_ATTR(LDO_VCAM_AF_VOLTAGE, 0664, show_LDO_VCAM_AF_VOLTAGE, store_LDO_VCAM_AF_VOLTAGE);


static ssize_t show_LDO_VIBR_VOLTAGE(struct device *dev,struct device_attribute *attr, char *buf)
{
    kal_uint32 ret_value=0;
    
    kal_uint32 ret=0;
    kal_uint32 reg_address=0x544;
    kal_uint32 reg_val=0;
    
    ret = pmic_read_interface(reg_address, &reg_val, 0x07, 5);
    if(reg_val == 0)
        ret_value = 1200;
    else if(reg_val == 1)
        ret_value = 1300;         
    else if(reg_val == 2)
        ret_value = 1500;   
    else if(reg_val == 3)
        ret_value = 1800;   
    else if(reg_val == 4)
        ret_value = 2000;   
    else if(reg_val == 5)
        ret_value = 2800;   
    else if(reg_val == 6)
        ret_value = 3000;   	
    else if(reg_val == 7)
        ret_value = 3300;          
    
    xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[EM] LDO_VIBR_VOLTAGE : %d\n", ret_value);
    return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_LDO_VIBR_VOLTAGE(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
    xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[EM] Not Support Write Function\n");    
    return size;
}
static DEVICE_ATTR(LDO_VIBR_VOLTAGE, 0664, show_LDO_VIBR_VOLTAGE, store_LDO_VIBR_VOLTAGE);



static ssize_t show_LDO_VM_VOLTAGE(struct device *dev,struct device_attribute *attr, char *buf)
{
    kal_uint32 ret_value=0;
    
    kal_uint32 ret=0;
    kal_uint32 reg_address=0x554;
    kal_uint32 reg_val=0;
    
    ret = pmic_read_interface(reg_address, &reg_val, 0x03, 4);
    if(reg_val == 0)
        ret_value = 1200;
    else if(reg_val == 1)
        ret_value = 1350;         
    else if(reg_val == 2)
        ret_value = 1500;   
    else if(reg_val == 3)
        ret_value = 1800;   
    
    
    xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[EM] LDO_VM_VOLTAGE : %d\n", ret_value);
    return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_LDO_VM_VOLTAGE(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
    xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[EM] Not Support Write Function\n");    
    return size;
}
static DEVICE_ATTR(LDO_VM_VOLTAGE, 0664, show_LDO_VM_VOLTAGE, store_LDO_VM_VOLTAGE);


static ssize_t show_LDO_VCAMD_VOLTAGE(struct device *dev,struct device_attribute *attr, char *buf)
{
    kal_uint32 ret_value=0;
    
    kal_uint32 ret=0;
    kal_uint32 reg_address=0x55c;
    kal_uint32 reg_val=0;
    
    ret = pmic_read_interface(reg_address, &reg_val, 0x03, 5);
    if(reg_val == 0)
        ret_value = 1200;
    else if(reg_val == 1)
        ret_value = 1350;         
    else if(reg_val == 2)
        ret_value = 1500;   
    else if(reg_val == 3)
        ret_value = 1800;   
    
    
    xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[EM] LDO_VCAMD_VOLTAGE : %d\n", ret_value);
    return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_LDO_VCAMD_VOLTAGE(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
    xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[EM] Not Support Write Function\n");    
    return size;
}
static DEVICE_ATTR(LDO_VCAMD_VOLTAGE, 0664, show_LDO_VCAMD_VOLTAGE, store_LDO_VCAMD_VOLTAGE);


static ssize_t show_LDO_VCAMA_VOLTAGE(struct device *dev,struct device_attribute *attr, char *buf)
{
    kal_uint32 ret_value=0;
    
    kal_uint32 ret=0;
    kal_uint32 reg_address=0x412;
    kal_uint32 reg_val=0;
    
    ret = pmic_read_interface(reg_address, &reg_val, 0x03, 5);
    if(reg_val == 0)
        ret_value = 1500;
    else if(reg_val == 1)
        ret_value = 1800;         
    else if(reg_val == 2)
        ret_value = 2500;   
    else if(reg_val == 3)
        ret_value = 2800;   
    
    
    xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[EM] LDO_VCAMA_VOLTAGE : %d\n", ret_value);
    return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_LDO_VCAMA_VOLTAGE(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
    xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[EM] Not Support Write Function\n");    
    return size;
}
static DEVICE_ATTR(LDO_VCAMA_VOLTAGE, 0664, show_LDO_VCAMA_VOLTAGE, store_LDO_VCAMA_VOLTAGE);


static ssize_t show_LDO_VCN33_VOLTAGE(struct device *dev,struct device_attribute *attr, char *buf)
{
    kal_uint32 ret_value=0;
    
    kal_uint32 ret=0;
    kal_uint32 reg_address=0x412;
    kal_uint32 reg_val=0;
    
    ret = pmic_read_interface(reg_address, &reg_val, 0x03, 2);
    if(reg_val == 0)
        ret_value = 3300;
    else if(reg_val == 1)
        ret_value = 3400;         
    else if(reg_val == 2)
        ret_value = 3500;   
    else if(reg_val == 3)
        ret_value = 3600;   
    
    
    xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[EM] LDO_VCN33_VOLTAGE : %d\n", ret_value);
    return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_LDO_VCN33_VOLTAGE(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
    xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[EM] Not Support Write Function\n");    
    return size;
}
static DEVICE_ATTR(LDO_VCN33_VOLTAGE, 0664, show_LDO_VCN33_VOLTAGE, store_LDO_VCN33_VOLTAGE);


static ssize_t show_BUCK_CURRENT_METER(struct device *dev,struct device_attribute *attr, char *buf)
{
    int ret_value=0;

    ret_value = pmic_get_buck_current(10);

    xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[EM] BUCK_CURRENT_METER : %d\n", ret_value);
    return sprintf(buf, "%u\n", ret_value);
}
static ssize_t store_BUCK_CURRENT_METER(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
    xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[EM] Not Support Write Function\n");    
    return size;
}
static DEVICE_ATTR(BUCK_CURRENT_METER, 0664, show_BUCK_CURRENT_METER, store_BUCK_CURRENT_METER);


//==============================================================================
// LDO EN & SEL common API
//==============================================================================
void pmic_ldo_enable(MT65XX_POWER powerId, kal_bool powerEnable)
{
    

    //Need integrate with DCT : using DCT APIs

    if(     powerId == MT6323_POWER_LDO_VIO28)      { dct_pmic_VIO28_enable(powerEnable); }
    else if(powerId == MT6323_POWER_LDO_VUSB)       { dct_pmic_VUSB_enable(powerEnable); }
    else if(powerId == MT6323_POWER_LDO_VMC)        { dct_pmic_VMC_enable(powerEnable); }
    else if(powerId == MT6323_POWER_LDO_VMCH)        { dct_pmic_VMCH_enable(powerEnable); }
    else if(powerId == MT6323_POWER_LDO_VEMC_3V3)    { dct_pmic_VEMC_3V3_enable(powerEnable); }
    else if(powerId == MT6323_POWER_LDO_VGP1)    { dct_pmic_VGP1_enable(powerEnable); }
    else if(powerId == MT6323_POWER_LDO_VGP2)        { dct_pmic_VGP2_enable(powerEnable); }
    else if(powerId == MT6323_POWER_LDO_VGP3)        { dct_pmic_VGP3_enable(powerEnable); }
    else if(powerId == MT6323_POWER_LDO_VCN_1V8)        { dct_pmic_VCN_1V8_enable(powerEnable); }
    else if(powerId == MT6323_POWER_LDO_VSIM1)        { dct_pmic_VSIM1_enable(powerEnable); }
    else if(powerId == MT6323_POWER_LDO_VSIM2)        { dct_pmic_VSIM2_enable(powerEnable); }
    else if(powerId == MT6323_POWER_LDO_VRTC)        { dct_pmic_VRTC_enable(powerEnable); }
    else if(powerId == MT6323_POWER_LDO_VCAM_AF)        { dct_pmic_VCAM_AF_enable(powerEnable); }
    else if(powerId == MT6323_POWER_LDO_VIBR)        { dct_pmic_VIBR_enable(powerEnable); }
    else if(powerId == MT6323_POWER_LDO_VM)        { dct_pmic_VM_enable(powerEnable); }
    else if(powerId == MT6323_POWER_LDO_VRF18)        { dct_pmic_VRF18_enable(powerEnable); }
    else if(powerId == MT6323_POWER_LDO_VIO18)        { dct_pmic_VIO18_enable(powerEnable); }
    else if(powerId == MT6323_POWER_LDO_VCAMD)        { dct_pmic_VCAMD_enable(powerEnable); }
    else if(powerId == MT6323_POWER_LDO_VCAM_IO)    { dct_pmic_VCAM_IO_enable(powerEnable); }
    else if(powerId == MT6323_POWER_LDO_VTCXO)      { dct_pmic_VTCXO_enable(powerEnable); }
    else if(powerId == MT6323_POWER_LDO_VA)         { dct_pmic_VA_enable(powerEnable); }
    else if(powerId == MT6323_POWER_LDO_VCAMA)      { dct_pmic_VCAMA_enable(powerEnable); }
    else if(powerId == MT6323_POWER_LDO_VCN33_BT)        { dct_pmic_VCN33_enable_bt(powerEnable); }
    else if(powerId == MT6323_POWER_LDO_VCN33_WIFI)        { dct_pmic_VCN33_enable_wifi(powerEnable); }	
    else if(powerId == MT6323_POWER_LDO_VCN28)      { dct_pmic_VCN28_enable(powerEnable); }
    else
    {
        xlog_printk(ANDROID_LOG_WARN, "Power/PMIC", "[pmic_ldo_enable] UnKnown powerId (%d)\n", powerId);
    }

    xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[pmic_ldo_enable] Receive powerId %d, action is %d\n", powerId, powerEnable);
	
}

void pmic_ldo_vol_sel(MT65XX_POWER powerId, MT65XX_POWER_VOLTAGE powerVolt)
{
    

    //Need integrate with DCT : using DCT APIs

    if(     powerId == MT6323_POWER_LDO_VIO28)      { xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[dct_pmic_VIO28_enable] No vlotage can setting!\n"); }
    else if(powerId == MT6323_POWER_LDO_VUSB)       { xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[dct_pmic_VUSB_enable] No vlotage can setting!\n"); }
    else if(powerId == MT6323_POWER_LDO_VMC)        { dct_pmic_VMC_sel(powerVolt); }
    else if(powerId == MT6323_POWER_LDO_VMCH)        { dct_pmic_VMCH_sel(powerVolt); }
    else if(powerId == MT6323_POWER_LDO_VEMC_3V3)    { dct_pmic_VEMC_3V3_sel(powerVolt); }
    else if(powerId == MT6323_POWER_LDO_VGP1)    { dct_pmic_VGP1_sel(powerVolt); }
    else if(powerId == MT6323_POWER_LDO_VGP2)        { dct_pmic_VGP2_sel(powerVolt); }
    else if(powerId == MT6323_POWER_LDO_VGP3)        { dct_pmic_VGP3_sel(powerVolt); }
    else if(powerId == MT6323_POWER_LDO_VCN_1V8)        {xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[dct_pmic_VCN_1V8_enable] No vlotage can setting!\n");}
    else if(powerId == MT6323_POWER_LDO_VSIM1)        { dct_pmic_VSIM1_sel(powerVolt); }
    else if(powerId == MT6323_POWER_LDO_VSIM2)        { dct_pmic_VSIM2_sel(powerVolt); }
    else if(powerId == MT6323_POWER_LDO_VRTC)        { xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[dct_pmic_VRTC_enable] No vlotage can setting!\n");}
    else if(powerId == MT6323_POWER_LDO_VCAM_AF)        { dct_pmic_VCAM_AF_sel(powerVolt); }
    else if(powerId == MT6323_POWER_LDO_VIBR)        { dct_pmic_VIBR_sel(powerVolt); }
    else if(powerId == MT6323_POWER_LDO_VM)        { dct_pmic_VM_sel(powerVolt); }
    else if(powerId == MT6323_POWER_LDO_VRF18)        { xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[dct_pmic_VRF18_enable] No vlotage can setting!\n");}
    else if(powerId == MT6323_POWER_LDO_VIO18)        { xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[dct_pmic_VIO18_enable] No vlotage can setting!\n");}
    else if(powerId == MT6323_POWER_LDO_VCAMD)        { dct_pmic_VCAMD_sel(powerVolt); }
    else if(powerId == MT6323_POWER_LDO_VCAM_IO)    { xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[dct_pmic_VCAM_IO_enable] No vlotage can setting!\n");}
    else if(powerId == MT6323_POWER_LDO_VTCXO)        { xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[dct_pmic_VTCXO_enable] No vlotage can setting!\n");}
    else if(powerId == MT6323_POWER_LDO_VA)         { xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[dct_pmic_VA_enable] No vlotage can setting!\n"); }
    else if(powerId == MT6323_POWER_LDO_VCAMA)      { dct_pmic_VCAMA_sel(powerVolt); }
    else if(powerId == MT6323_POWER_LDO_VCN33_BT)        { dct_pmic_VCN33_sel(powerVolt); }
    else if(powerId == MT6323_POWER_LDO_VCN33_WIFI)        { dct_pmic_VCN33_sel(powerVolt); }	
    else if(powerId == MT6323_POWER_LDO_VCN28)      { xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[dct_pmic_VCN28_enable] No vlotage can setting!\n"); }
    else
    {
        xlog_printk(ANDROID_LOG_WARN, "Power/PMIC", "[pmic_ldo_ldo_vol_sel] UnKnown powerId (%d)\n", powerId);
    }

    xlog_printk(ANDROID_LOG_DEBUG, "Power/PMIC", "[pmic_ldo_vol_sel] Receive powerId %d, action is %d\n", powerId, powerVolt);
	
}

//==============================================================================
// PMIC6323 device driver
//==============================================================================
void ldo_service_test(void)
{
/*
    hwPowerOn(MT65XX_POWER_LDO_VIO28,    VOL_DEFAULT, "ldo_test");
    hwPowerOn(MT65XX_POWER_LDO_VUSB,     VOL_DEFAULT, "ldo_test");
    hwPowerOn(MT65XX_POWER_LDO_VMC1,     VOL_DEFAULT, "ldo_test");
    hwPowerOn(MT65XX_POWER_LDO_VMCH1,    VOL_DEFAULT, "ldo_test");
    hwPowerOn(MT65XX_POWER_LDO_VEMC_3V3, VOL_DEFAULT, "ldo_test");
    hwPowerOn(MT65XX_POWER_LDO_VEMC_1V8, VOL_DEFAULT, "ldo_test");
    hwPowerOn(MT65XX_POWER_LDO_VGP1,     VOL_DEFAULT, "ldo_test");
    hwPowerOn(MT65XX_POWER_LDO_VGP2,     VOL_DEFAULT, "ldo_test");
    hwPowerOn(MT65XX_POWER_LDO_VGP3,     VOL_DEFAULT, "ldo_test");
    hwPowerOn(MT65XX_POWER_LDO_VGP4,     VOL_DEFAULT, "ldo_test");
    hwPowerOn(MT65XX_POWER_LDO_VGP5,     VOL_DEFAULT, "ldo_test");
    hwPowerOn(MT65XX_POWER_LDO_VGP6,     VOL_DEFAULT, "ldo_test");
    hwPowerOn(MT65XX_POWER_LDO_VSIM1,    VOL_DEFAULT, "ldo_test");
    hwPowerOn(MT65XX_POWER_LDO_VSIM2,    VOL_DEFAULT, "ldo_test");
    hwPowerOn(MT65XX_POWER_LDO_VIBR,     VOL_DEFAULT, "ldo_test");
    hwPowerOn(MT65XX_POWER_LDO_VRTC,     VOL_DEFAULT, "ldo_test");
    hwPowerOn(MT65XX_POWER_LDO_VAST,     VOL_DEFAULT, "ldo_test");
    hwPowerOn(MT65XX_POWER_LDO_VRF28,    VOL_DEFAULT, "ldo_test");
    hwPowerOn(MT65XX_POWER_LDO_VRF28_2,  VOL_DEFAULT, "ldo_test");
    hwPowerOn(MT65XX_POWER_LDO_VTCXO,    VOL_DEFAULT, "ldo_test");
    hwPowerOn(MT65XX_POWER_LDO_VTCXO_2,  VOL_DEFAULT, "ldo_test");
    hwPowerOn(MT65XX_POWER_LDO_VA,       VOL_DEFAULT, "ldo_test");
    hwPowerOn(MT65XX_POWER_LDO_VA28,     VOL_DEFAULT, "ldo_test");
    hwPowerOn(MT65XX_POWER_LDO_VCAMA,    VOL_DEFAULT, "ldo_test");

    hwPowerDown(MT65XX_POWER_LDO_VIO28,     "ldo_test");
    hwPowerDown(MT65XX_POWER_LDO_VUSB,      "ldo_test");
    hwPowerDown(MT65XX_POWER_LDO_VMC1,      "ldo_test");
    hwPowerDown(MT65XX_POWER_LDO_VMCH1,     "ldo_test");
    hwPowerDown(MT65XX_POWER_LDO_VEMC_3V3,  "ldo_test");
    hwPowerDown(MT65XX_POWER_LDO_VEMC_1V8,  "ldo_test");
    hwPowerDown(MT65XX_POWER_LDO_VGP1,      "ldo_test");
    hwPowerDown(MT65XX_POWER_LDO_VGP2,      "ldo_test");
    hwPowerDown(MT65XX_POWER_LDO_VGP3,      "ldo_test");
    hwPowerDown(MT65XX_POWER_LDO_VGP4,      "ldo_test");
    hwPowerDown(MT65XX_POWER_LDO_VGP5,      "ldo_test");
    hwPowerDown(MT65XX_POWER_LDO_VGP6,      "ldo_test");
    hwPowerDown(MT65XX_POWER_LDO_VSIM1,     "ldo_test");
    hwPowerDown(MT65XX_POWER_LDO_VSIM2,     "ldo_test");
    hwPowerDown(MT65XX_POWER_LDO_VIBR,      "ldo_test");
    hwPowerDown(MT65XX_POWER_LDO_VRTC,      "ldo_test");
    hwPowerDown(MT65XX_POWER_LDO_VAST,      "ldo_test");
    hwPowerDown(MT65XX_POWER_LDO_VRF28,     "ldo_test");
    hwPowerDown(MT65XX_POWER_LDO_VRF28_2,   "ldo_test");
    hwPowerDown(MT65XX_POWER_LDO_VTCXO,     "ldo_test");
    hwPowerDown(MT65XX_POWER_LDO_VTCXO_2,   "ldo_test");
    hwPowerDown(MT65XX_POWER_LDO_VA,        "ldo_test");
    hwPowerDown(MT65XX_POWER_LDO_VA28,      "ldo_test");
    hwPowerDown(MT65XX_POWER_LDO_VCAMA,     "ldo_test");
    */
}

//==============================================================================
// Dump all LDO status 
//==============================================================================
void dump_ldo_status_read_debug(void)
{
    kal_uint32 val_0=0, val_1=1;

    val_0 = upmu_get_reg_value(0x138);
    val_1 = upmu_get_reg_value(0x13A);

    printk("********** ldo status dump [1:ON,0:OFF]**********\n");

    printk("VRTC    =%d, ",  (((val_0)&(0x0008))>>3) );     
    printk("VA      =%d, ",  (((val_0)&(0x0010))>>4) ); 
    printk("VCAMA   =%d, ",  (((val_0)&(0x0020))>>5) ); 
    printk("VCAMD   =%d\n",  (((val_0)&(0x0040))>>6) );
         
    printk("VCAM_AF =%d, ",  (((val_0)&(0x0080))>>7) );        
    printk("VCAM_IO =%d, ",  (((val_0)&(0x0100))>>8) ); 
    printk("VCN28   =%d, ",  (((val_0)&(0x0200))>>9) ); 
    printk("VCN33   =%d\n",  (((val_0)&(0x0400))>>10) );
         
    printk("VCN_1V8 =%d, ",  (((val_0)&(0x0800))>>11) );    
    printk("VEMC_3V3=%d, ",  (((val_0)&(0x1000))>>12) );
    printk("VGP1    =%d, ",  (((val_0)&(0x2000))>>13) );
    printk("VGP2    =%d\n",  (((val_0)&(0x4000))>>14) );
         
    printk("VGP3    =%d, ",  (((val_0)&(0x8000))>>15) );                                                        
    printk("VIBR    =%d, ",  (((val_1)&(0x0001))>>0) ); 
    printk("VIO18   =%d, ",  (((val_1)&(0x0002))>>1) ); 
    printk("VIO28   =%d\n",  (((val_1)&(0x0004))>>2) ); 
         
    printk("VM      =%d, ",  (((val_1)&(0x0008))>>3) );     
    printk("VMC     =%d, ",  (((val_1)&(0x0010))>>4) ); 
    printk("VMCH    =%d, ",  (((val_1)&(0x0020))>>5) ); 
    printk("VRF18   =%d\n",  (((val_1)&(0x0040))>>6) ); 
         
    printk("VSIM1   =%d, ",  (((val_1)&(0x0080))>>7) );     
    printk("VSIM2   =%d, ",  (((val_1)&(0x0100))>>8) ); 
    printk("VTCXO   =%d, ",  (((val_1)&(0x0200))>>9) ); 
    printk("VUSB    =%d\n",  (((val_1)&(0x0400))>>10) );
}

static int dump_ldo_status_read(char *page, char **start, off_t off,
                int count, int *eof, void *data)
{
    char *p = page;
    int len = 0;

    kal_uint32 val_0=0, val_1=1;

    val_0 = upmu_get_reg_value(0x138);
    val_1 = upmu_get_reg_value(0x13A);
    
    p += sprintf(p, "********** ldo status dump [1:ON,0:OFF]**********\n");

    /*
    p += sprintf(p, "VPROC=%d\n",    (((val_1)&(0x0001))>>0) ); 
    p += sprintf(p, "VSRAM=%d\n",    (((val_1)&(0x0002))>>1) );
    p += sprintf(p, "VCORE=%d\n",    (((val_0)&(0x0004))>>2) );
    */
    p += sprintf(p, "VRTC    =%d, ",  (((val_0)&(0x0008))>>3) );     
    p += sprintf(p, "VA      =%d, ",  (((val_0)&(0x0010))>>4) ); 
    p += sprintf(p, "VCAMA   =%d, ",  (((val_0)&(0x0020))>>5) ); 
    p += sprintf(p, "VCAMD   =%d\n",  (((val_0)&(0x0040))>>6) );
    
    p += sprintf(p, "VCAM_AF =%d, ",  (((val_0)&(0x0080))>>7) );        
    p += sprintf(p, "VCAM_IO =%d, ",  (((val_0)&(0x0100))>>8) ); 
    p += sprintf(p, "VCN28   =%d, ",  (((val_0)&(0x0200))>>9) ); 
    p += sprintf(p, "VCN33   =%d\n",  (((val_0)&(0x0400))>>10) );
    
    p += sprintf(p, "VCN_1V8 =%d, ",  (((val_0)&(0x0800))>>11) );    
    p += sprintf(p, "VEMC_3V3=%d, ",  (((val_0)&(0x1000))>>12) );
    p += sprintf(p, "VGP1    =%d, ",  (((val_0)&(0x2000))>>13) );
    p += sprintf(p, "VGP2    =%d\n",  (((val_0)&(0x4000))>>14) );
    
    p += sprintf(p, "VGP3    =%d, ",  (((val_0)&(0x8000))>>15) );                                                        
    p += sprintf(p, "VIBR    =%d, ",  (((val_1)&(0x0001))>>0) ); 
    p += sprintf(p, "VIO18   =%d, ",  (((val_1)&(0x0002))>>1) ); 
    p += sprintf(p, "VIO28   =%d\n",  (((val_1)&(0x0004))>>2) ); 
    
    p += sprintf(p, "VM      =%d, ",  (((val_1)&(0x0008))>>3) );     
    p += sprintf(p, "VMC     =%d, ",  (((val_1)&(0x0010))>>4) ); 
    p += sprintf(p, "VMCH    =%d, ",  (((val_1)&(0x0020))>>5) ); 
    p += sprintf(p, "VRF18   =%d\n",  (((val_1)&(0x0040))>>6) ); 
    
    p += sprintf(p, "VSIM1   =%d, ",  (((val_1)&(0x0080))>>7) );     
    p += sprintf(p, "VSIM2   =%d, ",  (((val_1)&(0x0100))>>8) ); 
    p += sprintf(p, "VTCXO   =%d, ",  (((val_1)&(0x0200))>>9) ); 
    p += sprintf(p, "VUSB    =%d\n",  (((val_1)&(0x0400))>>10) );

    *start = page + off;

    len = p - page;
    if (len > off)
        len -= off;
    else
        len = 0;

    *eof = 1;
    return len < count ? len  : count;
}

void pmic_debug_init(void)
{
    struct proc_dir_entry *entry;
    struct proc_dir_entry *mt_pmic_dir;

    mt_pmic_dir = proc_mkdir("mt_pmic", NULL);
    if (!mt_pmic_dir) {
        xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "fail to mkdir /proc/mt_pmic\n" );
        return;
    }

    entry = create_proc_entry("dump_ldo_status", 00640, mt_pmic_dir);
    if (entry) {
        entry->read_proc = dump_ldo_status_read;
    }
}

void PMIC_INIT_SETTING_V1(void)
{
    U32 chip_version = 0;
    U32 ret=0;

    chip_version = upmu_get_cid();

    if(chip_version >= PMIC6323_E2_CID_CODE)
    {
        xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[Kernel_PMIC_INIT_SETTING_V1] PMIC Chip = %x\n",chip_version);
        xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[Kernel_PMIC_INIT_SETTING_V1] 20130604_0.85v\n");
        
        //put init setting from DE/SA
        
ret = pmic_config_interface(0x2,0xB,0xF,4); // [7:4]: RG_VCDT_HV_VTH; 7V OVP
ret = pmic_config_interface(0xC,0x1,0x7,1); // [3:1]: RG_VBAT_OV_VTH; VBAT_OV=4.3V
ret = pmic_config_interface(0x1A,0x3,0xF,0); // [3:0]: RG_CHRWDT_TD; align to 6250's
ret = pmic_config_interface(0x24,0x1,0x1,1); // [1:1]: RG_BC11_RST; Reset BC11 detection
ret = pmic_config_interface(0x2A,0x0,0x7,4); // [6:4]: RG_CSDAC_STP; align to 6250's setting
ret = pmic_config_interface(0x2E,0x1,0x1,2); // [2:2]: RG_CSDAC_MODE; 
ret = pmic_config_interface(0x2E,0x1,0x1,6); // [6:6]: RG_HWCV_EN; 
ret = pmic_config_interface(0x2E,0x1,0x1,7); // [7:7]: RG_ULC_DET_EN; 
ret = pmic_config_interface(0x3C,0x1,0x1,5); // [5:5]: THR_HWPDN_EN; 
ret = pmic_config_interface(0x40,0x1,0x1,4); // [4:4]: RG_EN_DRVSEL; 
ret = pmic_config_interface(0x40,0x1,0x1,5); // [5:5]: RG_RST_DRVSEL; 
ret = pmic_config_interface(0x46,0x1,0x1,1); // [1:1]: PWRBB_DEB_EN; 
ret = pmic_config_interface(0x48,0x1,0x1,8); // [8:8]: VPROC_PG_H2L_EN; 
ret = pmic_config_interface(0x48,0x1,0x1,9); // [9:9]: VSYS_PG_H2L_EN; 
ret = pmic_config_interface(0x4E,0x1,0x1,5); // [5:5]: STRUP_AUXADC_RSTB_SW; 
ret = pmic_config_interface(0x4E,0x1,0x1,7); // [7:7]: STRUP_AUXADC_RSTB_SEL; 
ret = pmic_config_interface(0x50,0x1,0x1,0); // [0:0]: STRUP_PWROFF_SEQ_EN; 
ret = pmic_config_interface(0x50,0x1,0x1,1); // [1:1]: STRUP_PWROFF_PREOFF_EN; 
ret = pmic_config_interface(0x52,0x1,0x1,9); // [9:9]: SPK_THER_SHDN_L_EN; 
ret = pmic_config_interface(0x56,0x1,0x1,0); // [0:0]: RG_SPK_INTG_RST_L; 
ret = pmic_config_interface(0x64,0x1,0xF,8); // [11:8]: RG_SPKPGA_GAIN; 
ret = pmic_config_interface(0x102,0x1,0x1,6); // [6:6]: RG_RTC_75K_CK_PDN; 
ret = pmic_config_interface(0x102,0x0,0x1,11); // [11:11]: RG_DRV_32K_CK_PDN; 
ret = pmic_config_interface(0x102,0x1,0x1,15); // [15:15]: RG_BUCK32K_PDN; 
ret = pmic_config_interface(0x108,0x1,0x1,12); // [12:12]: RG_EFUSE_CK_PDN; 
ret = pmic_config_interface(0x10E,0x1,0x1,5); // [5:5]: RG_AUXADC_CTL_CK_PDN; 
ret = pmic_config_interface(0x120,0x1,0x1,4); // [4:4]: RG_SRCLKEN_HW_MODE; 
ret = pmic_config_interface(0x120,0x1,0x1,5); // [5:5]: RG_OSC_HW_MODE; 
ret = pmic_config_interface(0x148,0x1,0x1,1); // [1:1]: RG_SMT_INT; 
ret = pmic_config_interface(0x148,0x1,0x1,3); // [3:3]: RG_SMT_RTC_32K1V8; 
ret = pmic_config_interface(0x160,0x1,0x1,2); // [2:2]: RG_INT_EN_BAT_L; 
ret = pmic_config_interface(0x160,0x1,0x1,6); // [6:6]: RG_INT_EN_THR_L; 
ret = pmic_config_interface(0x160,0x1,0x1,7); // [7:7]: RG_INT_EN_THR_H; 
ret = pmic_config_interface(0x166,0x1,0x1,1); // [1:1]: RG_INT_EN_FCHRKEY; 
ret = pmic_config_interface(0x212,0x2,0x3,4); // [5:4]: QI_VPROC_VSLEEP; 
ret = pmic_config_interface(0x216,0x1,0x1,1); // [1:1]: VPROC_VOSEL_CTRL; 
ret = pmic_config_interface(0x21C,0x17,0x7F,0); // [6:0]: VPROC_SFCHG_FRATE; 
ret = pmic_config_interface(0x21C,0x1,0x1,7); // [7:7]: VPROC_SFCHG_FEN; 
ret = pmic_config_interface(0x21C,0x1,0x1,15); // [15:15]: VPROC_SFCHG_REN; 
ret = pmic_config_interface(0x222,0x18,0x7F,0); // [6:0]: VPROC_VOSEL_SLEEP; 
ret = pmic_config_interface(0x230,0x3,0x3,0); // [1:0]: VPROC_TRANSTD; 
ret = pmic_config_interface(0x230,0x1,0x1,8); // [8:8]: VPROC_VSLEEP_EN; 
ret = pmic_config_interface(0x238,0x3,0x3,0); // [1:0]: RG_VSYS_SLP; 
ret = pmic_config_interface(0x238,0x2,0x3,4); // [5:4]: QI_VSYS_VSLEEP; 
ret = pmic_config_interface(0x23C,0x1,0x1,1); // [1:1]: VSYS_VOSEL_CTRL; after 0x0256
ret = pmic_config_interface(0x242,0x23,0x7F,0); // [6:0]: VSYS_SFCHG_FRATE; 
ret = pmic_config_interface(0x242,0x11,0x7F,8); // [14:8]: VSYS_SFCHG_RRATE; 
ret = pmic_config_interface(0x242,0x1,0x1,15); // [15:15]: VSYS_SFCHG_REN; 
ret = pmic_config_interface(0x256,0x3,0x3,0); // [1:0]: VSYS_TRANSTD; 
ret = pmic_config_interface(0x256,0x1,0x3,4); // [5:4]: VSYS_VOSEL_TRANS_EN; 
ret = pmic_config_interface(0x256,0x1,0x1,8); // [8:8]: VSYS_VSLEEP_EN; 
ret = pmic_config_interface(0x302,0x2,0x3,8); // [9:8]: RG_VPA_CSL; OC limit
ret = pmic_config_interface(0x302,0x1,0x3,14); // [15:14]: RG_VPA_ZX_OS; ZX limit
ret = pmic_config_interface(0x310,0x1,0x1,7); // [7:7]: VPA_SFCHG_FEN; 
ret = pmic_config_interface(0x310,0x1,0x1,15); // [15:15]: VPA_SFCHG_REN; 
ret = pmic_config_interface(0x326,0x1,0x1,0); // [0:0]: VPA_DLC_MAP_EN; 
ret = pmic_config_interface(0x402,0x1,0x1,0); // [0:0]: VTCXO_LP_SEL; 
ret = pmic_config_interface(0x402,0x0,0x1,11); // [11:11]: VTCXO_ON_CTRL; 
ret = pmic_config_interface(0x404,0x1,0x1,0); // [0:0]: VA_LP_SEL; 
ret = pmic_config_interface(0x404,0x2,0x3,8); // [9:8]: RG_VA_SENSE_SEL; 
ret = pmic_config_interface(0x500,0x1,0x1,0); // [0:0]: VIO28_LP_SEL; 
ret = pmic_config_interface(0x502,0x1,0x1,0); // [0:0]: VUSB_LP_SEL; 
ret = pmic_config_interface(0x504,0x1,0x1,0); // [0:0]: VMC_LP_SEL; 
ret = pmic_config_interface(0x506,0x1,0x1,0); // [0:0]: VMCH_LP_SEL; 
ret = pmic_config_interface(0x508,0x1,0x1,0); // [0:0]: VEMC_3V3_LP_SEL; 
ret = pmic_config_interface(0x514,0x1,0x1,0); // [0:0]: RG_STB_SIM1_SIO; 
ret = pmic_config_interface(0x516,0x1,0x1,0); // [0:0]: VSIM1_LP_SEL; 
ret = pmic_config_interface(0x518,0x1,0x1,0); // [0:0]: VSIM2_LP_SEL; 
ret = pmic_config_interface(0x524,0x1,0x1,0); // [0:0]: RG_STB_SIM2_SIO; 
ret = pmic_config_interface(0x542,0x1,0x1,2); // [2:2]: VIBR_THER_SHEN_EN; 
ret = pmic_config_interface(0x54E,0x0,0x1,15); // [15:15]: RG_VRF18_EN; 
ret = pmic_config_interface(0x550,0x1,0x1,1); // [1:1]: VRF18_ON_CTRL; 
ret = pmic_config_interface(0x552,0x1,0x1,0); // [0:0]: VM_LP_SEL; 
ret = pmic_config_interface(0x552,0x1,0x1,14); // [14:14]: RG_VM_EN; 
ret = pmic_config_interface(0x556,0x1,0x1,0); // [0:0]: VIO18_LP_SEL; 
ret = pmic_config_interface(0x756,0x1,0x3,2); // [3:2]: RG_ADC_TRIM_CH6_SEL; 
ret = pmic_config_interface(0x756,0x1,0x3,4); // [5:4]: RG_ADC_TRIM_CH5_SEL; 
ret = pmic_config_interface(0x756,0x1,0x3,8); // [9:8]: RG_ADC_TRIM_CH3_SEL; 
ret = pmic_config_interface(0x756,0x1,0x3,10); // [11:10]: RG_ADC_TRIM_CH2_SEL; 
ret = pmic_config_interface(0x778,0x1,0x1,15); // [15:15]: RG_VREF18_ENB_MD; 
        
    }
    else if(chip_version >= PMIC6323_E1_CID_CODE)
    {
        xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[Kernel_PMIC_INIT_SETTING_V1] PMIC Chip = %x\n",chip_version);
        xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[Kernel_PMIC_INIT_SETTING_V1] 20130328_0.85v\n");

        //put init setting from DE/SA

ret = pmic_config_interface(0x2,0xB,0xF,4); // [7:4]: RG_VCDT_HV_VTH; 7V OVP
ret = pmic_config_interface(0xC,0x1,0x7,1); // [3:1]: RG_VBAT_OV_VTH; VBAT_OV=4.3V
ret = pmic_config_interface(0x1A,0x3,0xF,0); // [3:0]: RG_CHRWDT_TD; align to 6250's
ret = pmic_config_interface(0x24,0x1,0x1,1); // [1:1]: RG_BC11_RST; Reset BC11 detection
ret = pmic_config_interface(0x2A,0x0,0x7,4); // [6:4]: RG_CSDAC_STP; align to 6250's setting
ret = pmic_config_interface(0x2E,0x1,0x1,2); // [2:2]: RG_CSDAC_MODE; 
ret = pmic_config_interface(0x2E,0x1,0x1,6); // [6:6]: RG_HWCV_EN; 
ret = pmic_config_interface(0x2E,0x1,0x1,7); // [7:7]: RG_ULC_DET_EN; 
ret = pmic_config_interface(0x3C,0x1,0x1,5); // [5:5]: THR_HWPDN_EN; 
ret = pmic_config_interface(0x40,0x1,0x1,4); // [4:4]: RG_EN_DRVSEL; 
ret = pmic_config_interface(0x40,0x1,0x1,5); // [5:5]: RG_RST_DRVSEL; 
ret = pmic_config_interface(0x46,0x1,0x1,1); // [1:1]: PWRBB_DEB_EN; 
ret = pmic_config_interface(0x48,0x1,0x1,8); // [8:8]: VPROC_PG_H2L_EN; 
ret = pmic_config_interface(0x48,0x1,0x1,9); // [9:9]: VSYS_PG_H2L_EN; 
ret = pmic_config_interface(0x4E,0x1,0x1,5); // [5:5]: STRUP_AUXADC_RSTB_SW; 
ret = pmic_config_interface(0x4E,0x1,0x1,7); // [7:7]: STRUP_AUXADC_RSTB_SEL; 
ret = pmic_config_interface(0x50,0x1,0x1,0); // [0:0]: STRUP_PWROFF_SEQ_EN; 
ret = pmic_config_interface(0x50,0x1,0x1,1); // [1:1]: STRUP_PWROFF_PREOFF_EN; 
ret = pmic_config_interface(0x52,0x1,0x1,9); // [9:9]: SPK_THER_SHDN_L_EN; 
ret = pmic_config_interface(0x56,0x1,0x1,0); // [0:0]: RG_SPK_INTG_RST_L; 
ret = pmic_config_interface(0x64,0x1,0xF,8); // [11:8]: RG_SPKPGA_GAIN; 
ret = pmic_config_interface(0x102,0x1,0x1,6); // [6:6]: RG_RTC_75K_CK_PDN; 
ret = pmic_config_interface(0x102,0x0,0x1,11); // [11:11]: RG_DRV_32K_CK_PDN; 
ret = pmic_config_interface(0x102,0x1,0x1,15); // [15:15]: RG_BUCK32K_PDN; 
ret = pmic_config_interface(0x108,0x1,0x1,12); // [12:12]: RG_EFUSE_CK_PDN; 
ret = pmic_config_interface(0x10E,0x1,0x1,5); // [5:5]: RG_AUXADC_CTL_CK_PDN; 
ret = pmic_config_interface(0x120,0x1,0x1,4); // [4:4]: RG_SRCLKEN_HW_MODE; 
ret = pmic_config_interface(0x120,0x1,0x1,5); // [5:5]: RG_OSC_HW_MODE; 
ret = pmic_config_interface(0x148,0x1,0x1,1); // [1:1]: RG_SMT_INT; 
ret = pmic_config_interface(0x148,0x1,0x1,3); // [3:3]: RG_SMT_RTC_32K1V8; 
ret = pmic_config_interface(0x160,0x1,0x1,2); // [2:2]: RG_INT_EN_BAT_L; 
ret = pmic_config_interface(0x160,0x1,0x1,6); // [6:6]: RG_INT_EN_THR_L; 
ret = pmic_config_interface(0x160,0x1,0x1,7); // [7:7]: RG_INT_EN_THR_H; 
ret = pmic_config_interface(0x166,0x1,0x1,1); // [1:1]: RG_INT_EN_FCHRKEY; 
ret = pmic_config_interface(0x212,0x2,0x3,4); // [5:4]: QI_VPROC_VSLEEP; 
ret = pmic_config_interface(0x216,0x1,0x1,1); // [1:1]: VPROC_VOSEL_CTRL; 
ret = pmic_config_interface(0x21C,0x17,0x7F,0); // [6:0]: VPROC_SFCHG_FRATE; 
ret = pmic_config_interface(0x21C,0x1,0x1,7); // [7:7]: VPROC_SFCHG_FEN;
ret = pmic_config_interface(0x21C,0x1,0x1,15); // [15:15]: VPROC_SFCHG_REN; 
ret = pmic_config_interface(0x222,0x18,0x7F,0); // [6:0]: VPROC_VOSEL_SLEEP; 
ret = pmic_config_interface(0x230,0x3,0x3,0); // [1:0]: VPROC_TRANSTD; 
ret = pmic_config_interface(0x230,0x1,0x1,8); // [8:8]: VPROC_VSLEEP_EN; 
ret = pmic_config_interface(0x238,0x3,0x3,0); // [1:0]: RG_VSYS_SLP; 
ret = pmic_config_interface(0x238,0x2,0x3,4); // [5:4]: QI_VSYS_VSLEEP; 
ret = pmic_config_interface(0x23C,0x1,0x1,1); // [1:1]: VSYS_VOSEL_CTRL; after 0x0256
ret = pmic_config_interface(0x242,0x23,0x7F,0); // [6:0]: VSYS_SFCHG_FRATE; 
ret = pmic_config_interface(0x242,0x11,0x7F,8); // [14:8]: VSYS_SFCHG_RRATE; 
ret = pmic_config_interface(0x242,0x1,0x1,15); // [15:15]: VSYS_SFCHG_REN; 
ret = pmic_config_interface(0x256,0x3,0x3,0); // [1:0]: VSYS_TRANSTD; 
ret = pmic_config_interface(0x256,0x1,0x3,4); // [5:4]: VSYS_VOSEL_TRANS_EN; 
ret = pmic_config_interface(0x256,0x1,0x1,8); // [8:8]: VSYS_VSLEEP_EN; 
ret = pmic_config_interface(0x302,0x2,0x3,8); // [9:8]: RG_VPA_CSL; OC limit
ret = pmic_config_interface(0x302,0x1,0x3,14); // [15:14]: RG_VPA_ZX_OS; ZX limit
ret = pmic_config_interface(0x310,0x1,0x1,7); // [7:7]: VPA_SFCHG_FEN; 
ret = pmic_config_interface(0x310,0x1,0x1,15); // [15:15]: VPA_SFCHG_REN; 
ret = pmic_config_interface(0x326,0x1,0x1,0); // [0:0]: VPA_DLC_MAP_EN; 
ret = pmic_config_interface(0x402,0x1,0x1,0); // [0:0]: VTCXO_LP_SEL; 
ret = pmic_config_interface(0x402,0x0,0x1,11); // [11:11]: VTCXO_ON_CTRL; 
ret = pmic_config_interface(0x404,0x1,0x1,0); // [0:0]: VA_LP_SEL; 
ret = pmic_config_interface(0x404,0x2,0x3,8); // [9:8]: RG_VA_SENSE_SEL; 
ret = pmic_config_interface(0x500,0x1,0x1,0); // [0:0]: VIO28_LP_SEL; 
ret = pmic_config_interface(0x502,0x1,0x1,0); // [0:0]: VUSB_LP_SEL; 
ret = pmic_config_interface(0x504,0x1,0x1,0); // [0:0]: VMC_LP_SEL; 
ret = pmic_config_interface(0x506,0x1,0x1,0); // [0:0]: VMCH_LP_SEL; 
ret = pmic_config_interface(0x508,0x1,0x1,0); // [0:0]: VEMC_3V3_LP_SEL; 
ret = pmic_config_interface(0x514,0x1,0x1,0); // [0:0]: RG_STB_SIM1_SIO; 
ret = pmic_config_interface(0x516,0x1,0x1,0); // [0:0]: VSIM1_LP_SEL; 
ret = pmic_config_interface(0x518,0x1,0x1,0); // [0:0]: VSIM2_LP_SEL; 
ret = pmic_config_interface(0x524,0x1,0x1,0); // [0:0]: RG_STB_SIM2_SIO; 
ret = pmic_config_interface(0x542,0x1,0x1,2); // [2:2]: VIBR_THER_SHEN_EN; 
ret = pmic_config_interface(0x54E,0x0,0x1,15); // [15:15]: RG_VRF18_EN; 
ret = pmic_config_interface(0x550,0x1,0x1,1); // [1:1]: VRF18_ON_CTRL; 
ret = pmic_config_interface(0x552,0x1,0x1,0); // [0:0]: VM_LP_SEL; 
ret = pmic_config_interface(0x552,0x1,0x1,14); // [14:14]: RG_VM_EN; 
ret = pmic_config_interface(0x556,0x1,0x1,0); // [0:0]: VIO18_LP_SEL; 
ret = pmic_config_interface(0x756,0x1,0x3,2); // [3:2]: RG_ADC_TRIM_CH6_SEL; 
ret = pmic_config_interface(0x756,0x1,0x3,4); // [5:4]: RG_ADC_TRIM_CH5_SEL; 
ret = pmic_config_interface(0x756,0x1,0x3,8); // [9:8]: RG_ADC_TRIM_CH3_SEL; 
ret = pmic_config_interface(0x756,0x1,0x3,10); // [11:10]: RG_ADC_TRIM_CH2_SEL; 
ret = pmic_config_interface(0x778,0x1,0x1,15); // [15:15]: RG_VREF18_ENB_MD; 

    }
    else
    {
        xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[Kernel_PMIC_INIT_SETTING_V1] Unknown PMIC Chip (%x)\n",chip_version);
    }


    upmu_set_rg_adc_gps_status(1);
    upmu_set_rg_adc_md_status(1);
    upmu_set_rg_deci_gdly_vref18_selb(1);
    upmu_set_rg_deci_gdly_sel_mode(1);
    upmu_set_rg_osr(3);

    #ifdef MTK_MT6333_SUPPORT
    // move to mt6333 driver
    #else
    pmu_drv_tool_customization_init();
    #endif
    
}

void PMIC_CUSTOM_SETTING_V1(void)
{    
}

void pmic_setting_depends_rtc(void)
{
#ifdef CONFIG_MTK_RTC
    U32 ret=0;
    
    if( crystal_exist_status() )
    {
        // with 32K
        ret = pmic_config_interface(ANALDO_CON1,    1,    0x1,    11); // [11]=1(VTCXO_ON_CTRL), 
        ret = pmic_config_interface(ANALDO_CON1,    0,    0x1,    0);  // [0] =0(VTCXO_LP_SEL),
        
        xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[pmic_setting_depends_rtc] With 32K. Reg[0x%x]=0x%x\n", 
            ANALDO_CON1, upmu_get_reg_value(ANALDO_CON1));
    }
    else
    {
        // without 32K
        ret = pmic_config_interface(ANALDO_CON1,    0,    0x1,    11); // [11]=0(VTCXO_ON_CTRL), 
        ret = pmic_config_interface(ANALDO_CON1,    1,    0x1,    0);  // [0] =1(VTCXO_LP_SEL),

        xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[pmic_setting_depends_rtc] Without 32K. Reg[0x%x]=0x%x\n", 
            ANALDO_CON1, upmu_get_reg_value(ANALDO_CON1));
    }
#else
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[pmic_setting_depends_rtc] no define CONFIG_MTK_RTC\n");
#endif
}

static int pmic_mt6323_probe(struct platform_device *dev)
{
    U32 ret_val=0;

    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "******** MT6323 pmic driver probe!! ********\n" );


    //get PMIC CID
    ret_val=upmu_get_cid();
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "MT6323 PMIC CID=0x%x\n", ret_val );

    //enable rtc 32k to pmic
    //rtc_gpio_enable_32k(RTC_GPIO_USER_PMIC);

    //pmic initial setting
    PMIC_INIT_SETTING_V1();
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[PMIC_INIT_SETTING_V1] Done\n");
    PMIC_CUSTOM_SETTING_V1();
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[PMIC_CUSTOM_SETTING_V1] Done\n");
    
    //No need due to VTCXO always on
    //pmic setting with RTC
    //pmic_setting_depends_rtc();
    
    //PMIC Interrupt Service
    PMIC_EINT_SETTING();
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[PMIC_EINT_SETTING] Done\n");
    
    kthread_run(pmic_thread_kthread, NULL, "pmic_thread_kthread");
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[pmic_thread_kthread] Done\n");

    //Dump register
    //#ifndef USER_BUILD_KERNEL
    //PMIC_DUMP_ALL_Register();
    //#endif

    dump_ldo_status_read_debug();
    pmic_debug_init();
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[PMIC] pmic_debug_init : done.\n" );

    #ifdef MTK_ENABLE_MD5
    pmic_config_interface(0x402,0x0,0x1,0); // [0:0]: VTCXO_LP_SEL; 
    pmic_config_interface(0x402,0x1,0x1,11); // [11:11]: VTCXO_ON_CTRL;
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[PMIC] VTCXO control by SRCLKEM due to MTK_ENABLE_MD5, Reg[0x402]=0x%x\n",
        upmu_get_reg_value(0x402));
    #endif

    return 0;
}

static int pmic_mt6323_remove(struct platform_device *dev)
{
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "******** MT6323 pmic driver remove!! ********\n" );

    return 0;
}

static void pmic_mt6323_shutdown(struct platform_device *dev)
{
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "******** MT6323 pmic driver shutdown!! ********\n" );
}

static int pmic_mt6323_suspend(struct platform_device *dev, pm_message_t state)
{
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "******** MT6323 pmic driver suspend!! ********\n" );
	upmu_set_rg_vref18_enb(1);
	upmu_set_rg_adc_deci_gdly(1);
	upmu_set_rg_clksq_en_aux(1);
	upmu_set_rg_aud26m_div4_ck_pdn(0);
	upmu_set_rg_auxadc_sdm_sel_hw_mode(1);
	upmu_set_rg_auxadc_sdm_ck_hw_mode(1);
	upmu_set_rg_auxadc_sdm_ck_sel(0);
	upmu_set_rg_auxadc_sdm_ck_pdn(0);
	upmu_set_rg_auxadc_sdm_ck_wake_pdn(0);


    return 0;
}

static int pmic_mt6323_resume(struct platform_device *dev)
{
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "******** MT6323 pmic driver resume!! ********\n" );
	upmu_set_rg_vref18_enb(0);
	//upmu_set_rg_adc_deci_gdly(0);
	upmu_set_rg_clksq_en_aux(1);
	upmu_set_rg_aud26m_div4_ck_pdn(0);
	upmu_set_rg_auxadc_sdm_sel_hw_mode(0);
	upmu_set_rg_auxadc_sdm_ck_hw_mode(1);
	upmu_set_rg_auxadc_sdm_ck_sel(0);
	upmu_set_rg_auxadc_sdm_ck_pdn(0);
	upmu_set_rg_auxadc_sdm_ck_wake_pdn(1);
    return 0;
}

struct platform_device pmic_mt6323_device = {
    .name   = "pmic_mt6323",
    .id        = -1,
};

static struct platform_driver pmic_mt6323_driver = {
    .probe        = pmic_mt6323_probe,
    .remove       = pmic_mt6323_remove,
    .shutdown     = pmic_mt6323_shutdown,
    //#ifdef CONFIG_PM
    .suspend      = pmic_mt6323_suspend,
    .resume       = pmic_mt6323_resume,
    //#endif
    .driver       = {
        .name = "pmic_mt6323",
    },
};

//==============================================================================
// PMIC6323 device driver
//==============================================================================
static int mt_pmic_probe(struct platform_device *dev)
{
    int ret_device_file = 0;

    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "******** mt_pmic_probe!! ********\n" );

    ret_device_file = device_create_file(&(dev->dev), &dev_attr_pmic_access);

    ret_device_file = device_create_file(&(dev->dev), &dev_attr_BUCK_VPROC_STATUS);
    ret_device_file = device_create_file(&(dev->dev), &dev_attr_BUCK_VSYS_STATUS);
    ret_device_file = device_create_file(&(dev->dev), &dev_attr_BUCK_VPA_STATUS);

	
    ret_device_file = device_create_file(&(dev->dev), &dev_attr_LDO_VTCXO_STATUS);   
    ret_device_file = device_create_file(&(dev->dev), &dev_attr_LDO_VA_STATUS);    
    ret_device_file = device_create_file(&(dev->dev), &dev_attr_LDO_VCAMA_STATUS);   
    ret_device_file = device_create_file(&(dev->dev), &dev_attr_LDO_VCN28_STATUS);   
    ret_device_file = device_create_file(&(dev->dev), &dev_attr_LDO_VCN33_STATUS);   

	
    ret_device_file = device_create_file(&(dev->dev), &dev_attr_LDO_VIO28_STATUS);   
    ret_device_file = device_create_file(&(dev->dev), &dev_attr_LDO_VUSB_STATUS);   	
    ret_device_file = device_create_file(&(dev->dev), &dev_attr_LDO_VMC_STATUS);   
    ret_device_file = device_create_file(&(dev->dev), &dev_attr_LDO_VMCH_STATUS);   
    ret_device_file = device_create_file(&(dev->dev), &dev_attr_LDO_VEMC_3V3_STATUS);   
    ret_device_file = device_create_file(&(dev->dev), &dev_attr_LDO_VGP1_STATUS);   
    ret_device_file = device_create_file(&(dev->dev), &dev_attr_LDO_VGP2_STATUS);   
    ret_device_file = device_create_file(&(dev->dev), &dev_attr_LDO_VGP3_STATUS);   
    ret_device_file = device_create_file(&(dev->dev), &dev_attr_LDO_VCN_1V8_STATUS);   
    ret_device_file = device_create_file(&(dev->dev), &dev_attr_LDO_VSIM1_STATUS);   
    ret_device_file = device_create_file(&(dev->dev), &dev_attr_LDO_VSIM2_STATUS);   
    ret_device_file = device_create_file(&(dev->dev), &dev_attr_LDO_VRTC_STATUS);   
    ret_device_file = device_create_file(&(dev->dev), &dev_attr_LDO_VCAM_AF_STATUS);   
    ret_device_file = device_create_file(&(dev->dev), &dev_attr_LDO_VIBR_STATUS);   
    ret_device_file = device_create_file(&(dev->dev), &dev_attr_LDO_VM_STATUS);   
    ret_device_file = device_create_file(&(dev->dev), &dev_attr_LDO_VRF18_STATUS);   
    ret_device_file = device_create_file(&(dev->dev), &dev_attr_LDO_VIO18_STATUS);   
    ret_device_file = device_create_file(&(dev->dev), &dev_attr_LDO_VCAMD_STATUS);   
    ret_device_file = device_create_file(&(dev->dev), &dev_attr_LDO_VCAM_IO_STATUS);   	




    ret_device_file = device_create_file(&(dev->dev), &dev_attr_BUCK_VPROC_VOLTAGE);
    ret_device_file = device_create_file(&(dev->dev), &dev_attr_BUCK_VSYS_VOLTAGE);
    ret_device_file = device_create_file(&(dev->dev), &dev_attr_BUCK_VPA_VOLTAGE);

    ret_device_file = device_create_file(&(dev->dev), &dev_attr_LDO_VMC_VOLTAGE);   
    ret_device_file = device_create_file(&(dev->dev), &dev_attr_LDO_VMCH_VOLTAGE);  
    ret_device_file = device_create_file(&(dev->dev), &dev_attr_LDO_VEMC_3V3_VOLTAGE);   
    ret_device_file = device_create_file(&(dev->dev), &dev_attr_LDO_VGP1_VOLTAGE);   
    ret_device_file = device_create_file(&(dev->dev), &dev_attr_LDO_VGP2_VOLTAGE);   
    ret_device_file = device_create_file(&(dev->dev), &dev_attr_LDO_VGP3_VOLTAGE);   

    ret_device_file = device_create_file(&(dev->dev), &dev_attr_LDO_VSIM1_VOLTAGE);   
    ret_device_file = device_create_file(&(dev->dev), &dev_attr_LDO_VSIM2_VOLTAGE);   
    ret_device_file = device_create_file(&(dev->dev), &dev_attr_LDO_VCAM_AF_VOLTAGE);   
	
    ret_device_file = device_create_file(&(dev->dev), &dev_attr_LDO_VIBR_VOLTAGE);   
    ret_device_file = device_create_file(&(dev->dev), &dev_attr_LDO_VM_VOLTAGE);   
    ret_device_file = device_create_file(&(dev->dev), &dev_attr_LDO_VCAMD_VOLTAGE);  
    ret_device_file = device_create_file(&(dev->dev), &dev_attr_LDO_VCAMA_VOLTAGE);   
    ret_device_file = device_create_file(&(dev->dev), &dev_attr_LDO_VCN33_VOLTAGE);  
	
    ret_device_file = device_create_file(&(dev->dev), &dev_attr_BUCK_CURRENT_METER);


    return 0;
}

struct platform_device mt_pmic_device = {
    .name   = "mt-pmic",
    .id        = -1,
};

static struct platform_driver mt_pmic_driver = {
    .probe        = mt_pmic_probe,
    .driver     = {
        .name = "mt-pmic",
    },
};


#ifdef CONFIG_HAS_EARLYSUSPEND
static void pmic_early_suspend(struct early_suspend *h)
{
	xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "******** MT6323 pmic driver early suspend!! ********\n" );
	upmu_set_rg_vref18_enb(0);
	//upmu_set_rg_adc_deci_gdly(0);
	upmu_set_rg_clksq_en_aux(1);
	upmu_set_rg_aud26m_div4_ck_pdn(0);
	upmu_set_rg_auxadc_sdm_sel_hw_mode(0);
	upmu_set_rg_auxadc_sdm_ck_hw_mode(1);
	upmu_set_rg_auxadc_sdm_ck_sel(0);
	upmu_set_rg_auxadc_sdm_ck_pdn(0);
	upmu_set_rg_auxadc_sdm_ck_wake_pdn(1);

}

static void pmic_early_resume(struct early_suspend *h)
{
	xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "******** MT6323 pmic driver early resume!! ********\n" );
	upmu_set_rg_vref18_enb(0);
	//upmu_set_rg_adc_deci_gdly(0);
	upmu_set_rg_clksq_en_aux(1);
	upmu_set_rg_aud26m_div4_ck_pdn(0);
	upmu_set_rg_auxadc_sdm_sel_hw_mode(0);
	upmu_set_rg_auxadc_sdm_ck_hw_mode(0);
	upmu_set_rg_auxadc_sdm_ck_sel(0);
	upmu_set_rg_auxadc_sdm_ck_pdn(0);
	upmu_set_rg_auxadc_sdm_ck_wake_pdn(0);

}

static struct early_suspend pmic_early_suspend_desc = {
	.level		= EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1,
	.suspend	= pmic_early_suspend,
	.resume		= pmic_early_resume,
};
#endif


//==============================================================================
// PMIC6323 mudule init/exit
//==============================================================================
static int __init pmic_mt6323_init(void)
{
    int ret;

    wake_lock_init(&pmicAuxadc_irq_lock, WAKE_LOCK_SUSPEND, "pmicAuxadc irq wakelock");
    wake_lock_init(&pmicThread_lock, WAKE_LOCK_SUSPEND, "pmicThread wakelock");
	
	
    // PMIC device driver register
    ret = platform_device_register(&pmic_mt6323_device);
    if (ret) {
        xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "****[pmic_mt6323_init] Unable to device register(%d)\n", ret);
        return ret;
    }
    ret = platform_driver_register(&pmic_mt6323_driver);
    if (ret) {
        xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "****[pmic_mt6323_init] Unable to register driver (%d)\n", ret);
        return ret;
    }

    // PMIC user space access interface
    ret = platform_device_register(&mt_pmic_device);
    if (ret) {
            xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "****[pmic_mt6323_init] Unable to device register(%d)\n", ret);
            return ret;
    }
    ret = platform_driver_register(&mt_pmic_driver);
    if (ret) {
            xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "****[pmic_mt6323_init] Unable to register driver (%d)\n", ret);
            return ret;
    }

#ifdef CONFIG_HAS_EARLYSUSPEND
	register_early_suspend(&pmic_early_suspend_desc);
#endif

    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "****[pmic_mt6323_init] Initialization : DONE !!\n");

    return 0;
}

static void __exit pmic_mt6323_exit (void)
{
}

fs_initcall(pmic_mt6323_init);

//module_init(pmic_mt6323_init);
module_exit(pmic_mt6323_exit);

MODULE_AUTHOR("James Lo");
MODULE_DESCRIPTION("MT6323 PMIC Device Driver");
MODULE_LICENSE("GPL");

