#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/dmi.h>
#include <linux/acpi.h>
#include <linux/thermal.h>
#include <linux/platform_device.h>
#include <linux/aee.h>
#include <linux/xlog.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/proc_fs.h>
#include <linux/syscalls.h>
#include <linux/sched.h>
#include <linux/writeback.h>
#include <asm/uaccess.h>

#include <mach/system.h>
#include "mach/mtk_thermal_monitor.h"
#include "mach/mt_typedefs.h"
#include "mach/mt_thermal.h"

#define MTK_TZ_COOLER_MAX 10
#define AUXADC_CHANNEL 1

/* Zone */
static struct thermal_zone_device *thz_dev;
static unsigned int interval = 1; /* seconds, 0 : no auto polling */
static unsigned int trip_temp[MTK_TZ_COOLER_MAX] = {120000,110000,100000,90000,80000,70000,65000,60000,55000,50000};
static int kernelmode = 0;
static int g_THERMAL_TRIP[MTK_TZ_COOLER_MAX] = {0,0,0,0,0,0,0,0,0,0};
static int num_trip=0;
static char g_bind[MTK_TZ_COOLER_MAX][THERMAL_NAME_LENGTH] = {{0},{0},{0},{0},{0},{0},{0},{0},{0},{0}};
#define MTKTSBATTERY2_TEMP_CRIT 60000 /* 60.000 degree Celsius */

/* Logging */
static int mtktsbattery2_debug_log = 0;
#define mtktsbattery2_dprintk(fmt, args...)   \
do {                                    \
	if (mtktsbattery2_debug_log) {                \
		xlog_printk(ANDROID_LOG_INFO, "Power/Battery_Thermal", fmt, ##args); \
	}                                   \
} while(0)


static int mtktsbattery2_register_thermal(void);
static void mtktsbattery2_unregister_thermal(void);
static int mtkts_match(struct thermal_cooling_device *cdev, char bind[MTK_TZ_COOLER_MAX][THERMAL_NAME_LENGTH]);
extern int IMM_GetOneChannelValue(int dwChannel, int data[4], int* rawdata);
extern int IMM_IsAdcInitReady(void)
	;
typedef struct{
    INT32 BatteryTemp;
    INT32 TemperatureR;
}BATT_TEMPERATURE;

#define RBAT_PULL_UP_R             121000
#define TBAT_OVER_CRITICAL_LOW     68237
#define RBAT_PULL_UP_VOLT          1800

BATT_TEMPERATURE Batt2_Temperature_Table[] = {
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


/* convert register to temperature  */
static int Batt2ThermistorConverTemp(INT32 Res)
{
    int i=0;
    INT32 RES1=0,RES2=0;
    INT32 TBatt_Value=-200,TMP1=0,TMP2=0;



    if(Res>=Batt2_Temperature_Table[0].TemperatureR)
    {
        TBatt_Value = -20;
    }
    else if(Res<=Batt2_Temperature_Table[16].TemperatureR)
    {
        TBatt_Value = 60;
    }
    else
    {
        RES1=Batt2_Temperature_Table[0].TemperatureR;
        TMP1=Batt2_Temperature_Table[0].BatteryTemp;

        for(i=0;i<=16;i++)
        {
            if(Res>=Batt2_Temperature_Table[i].TemperatureR)
            {
                RES2=Batt2_Temperature_Table[i].TemperatureR;
                TMP2=Batt2_Temperature_Table[i].BatteryTemp;
                break;
            }
            else
            {
                RES1=Batt2_Temperature_Table[i].TemperatureR;
                TMP1=Batt2_Temperature_Table[i].BatteryTemp;
            }
        }
        
        TBatt_Value = (((Res-RES2)*TMP1)+((RES1-Res)*TMP2))/(RES1-RES2);
    }

    return TBatt_Value;    
}



/* convert ADC_bat_temp_volt to register */
static int BattVoltageToTemp(UINT32 dwVolt)
{
    INT32 TRes;
    INT32 dwVCriBat = 0; 
    INT32 sBaTTMP = -100;

    dwVCriBat = (TBAT_OVER_CRITICAL_LOW * RBAT_PULL_UP_VOLT) / (TBAT_OVER_CRITICAL_LOW + RBAT_PULL_UP_R);
        
    if(dwVolt > dwVCriBat)
    {
        TRes = TBAT_OVER_CRITICAL_LOW;
    }
    else
    {
        TRes = (RBAT_PULL_UP_R*dwVolt) / (RBAT_PULL_UP_VOLT-dwVolt);    
    }

    /* convert register to temperature */
    sBaTTMP = Batt2ThermistorConverTemp(TRes);
  
    return sBaTTMP;
}


static int get_hw_battery2_temp(void)
{
	int data[4], ret_value = 0, ret_temp = 0, output;
	
	if( IMM_IsAdcInitReady() == 0 )
	{
        mtktsbattery2_dprintk("[thermal_auxadc_get_data]: AUXADC is not ready\n");
		return 0;
	}
	
	ret_value = IMM_GetOneChannelValue(AUXADC_CHANNEL, data, &ret_temp);
	mtktsbattery2_dprintk("[thermal_auxadc_get_data(ADCIN1)]: ret_temp=%d\n",ret_temp);

	ret_temp = ret_temp*1500/4096;
	output = BattVoltageToTemp(ret_temp);
	mtktsbattery2_dprintk("Battery output temperature mV= %d  %d\n",output, ret_temp);
	
	return output;
}


static int mtktsbattery2_get_temp(struct thermal_zone_device *thermal,
			       unsigned long *t)
{
	*t = (unsigned long)get_hw_battery2_temp() * 1000;
	mtktsbattery2_dprintk("[mtktsbattery2_get_hw_temp] T_Battery, %d\n", *t);

	return 0;
}

static int mtkts_match(struct thermal_cooling_device *cdev, char bind[MTK_TZ_COOLER_MAX][THERMAL_NAME_LENGTH])
{
	int i;
	
	for(i=0;i<MTK_TZ_COOLER_MAX;i++)
	{
		if(!strcmp(cdev->type, bind[i]))
		{
			return i;
		}
	}	
	return i;
}


static int mtktsbattery2_bind(struct thermal_zone_device *thermal,
			struct thermal_cooling_device *cdev)
{
	int table_val=0;
	table_val = mtkts_match(cdev,g_bind);
	if(table_val >= MTK_TZ_COOLER_MAX) 
	{
		return 0;
	}
	else
	{
		mtktsbattery2_dprintk("[mtktsbattery2_bind] %s\n", cdev->type);	
		if (mtk_thermal_zone_bind_cooling_device(thermal, table_val, cdev)) {
			mtktsbattery2_dprintk("[mtktsbattery2_bind] error binding cooling dev\n");
			return -EINVAL;
		} else {
			mtktsbattery2_dprintk("[mtktsbattery2_bind] binding OK, %d\n", table_val);
		}	
	}
	return 0;  
}

static int mtktsbattery2_unbind(struct thermal_zone_device *thermal,
			  struct thermal_cooling_device *cdev)
{
    int table_val=0;
	
	table_val = mtkts_match(cdev,g_bind);
	if(table_val >= MTK_TZ_COOLER_MAX) 
	{
		return 0;
	}
	else
	{
		mtktsbattery2_dprintk("[mtktsbattery2_unbind] %s\n", cdev->type); 
		if (thermal_zone_unbind_cooling_device(thermal, table_val, cdev)) {
			mtktsbattery2_dprintk("[mtktsbattery2_unbind] error unbinding cooling dev\n");
			return -EINVAL;
		} else {
			mtktsbattery2_dprintk("[mtktsbattery2_unbind] unbinding OK, %d\n", table_val);
		}	
	}
	return 0;  
}

static int mtktsbattery2_get_mode(struct thermal_zone_device *thermal,
			    enum thermal_device_mode *mode)
{
	*mode = (kernelmode) ? THERMAL_DEVICE_ENABLED
			     : THERMAL_DEVICE_DISABLED;
	return 0;
}

static int mtktsbattery2_set_mode(struct thermal_zone_device *thermal,
			    enum thermal_device_mode mode)
{
	kernelmode = mode;
	return 0;
}

static int mtktsbattery2_get_trip_type(struct thermal_zone_device *thermal, int trip,
				 enum thermal_trip_type *type)
{
	*type = g_THERMAL_TRIP[trip];
	return 0;
}

static int mtktsbattery2_get_trip_temp(struct thermal_zone_device *thermal, int trip,
				 unsigned long *temp)
{
	*temp = trip_temp[trip]; 
	return 0;
}

static int mtktsbattery2_get_crit_temp(struct thermal_zone_device *thermal,
				 unsigned long *temperature)
{
	*temperature = MTKTSBATTERY2_TEMP_CRIT;
	return 0;
}

/* bind callback functions to thermalzone */
static struct thermal_zone_device_ops mtktsbattery2_dev_ops = {
	.bind = mtktsbattery2_bind,
	.unbind = mtktsbattery2_unbind,
	.get_temp = mtktsbattery2_get_temp,
	.get_mode = mtktsbattery2_get_mode,
	.set_mode = mtktsbattery2_set_mode,
	.get_trip_type = mtktsbattery2_get_trip_type,
	.get_trip_temp = mtktsbattery2_get_trip_temp,
	.get_crit_temp = mtktsbattery2_get_crit_temp,
};


static int mtktsbattery2_read(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	int len = 0;
	char *p = buf;
    
	p+= sprintf(p, "[mtktsbattery2_read]\n\
[trip_temp] = %d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n\
[trip_type] = %d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n\
[cool_bind] = %s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n\
time_ms=%d\n",
	trip_temp[0],trip_temp[1],trip_temp[2],trip_temp[3],trip_temp[4],
	trip_temp[5],trip_temp[6],trip_temp[7],trip_temp[8],trip_temp[9],
	g_THERMAL_TRIP[0],g_THERMAL_TRIP[1],g_THERMAL_TRIP[2],g_THERMAL_TRIP[3],g_THERMAL_TRIP[4],
	g_THERMAL_TRIP[5],g_THERMAL_TRIP[6],g_THERMAL_TRIP[7],g_THERMAL_TRIP[8],g_THERMAL_TRIP[9],
	g_bind[0],g_bind[1],g_bind[2],g_bind[3],g_bind[4],g_bind[5],g_bind[6],g_bind[7],g_bind[8],g_bind[9],
	interval*1000);
    
    
    
	*start = buf + off;
    
	len = p - buf;
	if (len > off)
		len -= off;
	else
		len = 0;
    
	return len < count ? len  : count;
}



static ssize_t mtktsbattery2_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
	int len=0,time_msec=0;
	int trip[MTK_TZ_COOLER_MAX]={0};
	int t_type[MTK_TZ_COOLER_MAX]={0};
	int i;
	char bind[MTK_TZ_COOLER_MAX][THERMAL_NAME_LENGTH];	
	char desc[512];


	len = (count < (sizeof(desc) - 1)) ? count : (sizeof(desc) - 1);
	if (copy_from_user(desc, buffer, len))
	{
		return 0;
	}
	desc[len] = '\0';

	if (sscanf(desc, "%d %d %d %19s %d %d %19s %d %d %19s %d %d %19s %d %d %19s %d %d %19s %d %d %19s %d %d %19s %d %d %19s %d %d %19s %d",
				&num_trip, 
				&trip[0],&t_type[0],bind[0], &trip[1],&t_type[1],bind[1],
				&trip[2],&t_type[2],bind[2], &trip[3],&t_type[3],bind[3],
				&trip[4],&t_type[4],bind[4], &trip[5],&t_type[5],bind[5],
				&trip[6],&t_type[6],bind[6], &trip[7],&t_type[7],bind[7],
				&trip[8],&t_type[8],bind[8], &trip[9],&t_type[9],bind[9],
				&time_msec) == 32)
	{
		mtktsbattery2_dprintk("[mtktsbattery2_write] unregister_thermal\n");
		mtktsbattery2_unregister_thermal();
	
		for(i=0; i<MTK_TZ_COOLER_MAX; i++)
		{
			g_THERMAL_TRIP[i] = t_type[i];	
            memcpy(g_bind[i], bind[i], THERMAL_NAME_LENGTH);			
			trip_temp[i]=trip[i];			
		}
		interval=time_msec / 1000;

		mtktsbattery2_dprintk("[mtktsbattery2_write] [trip_type]=%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
				g_THERMAL_TRIP[0],g_THERMAL_TRIP[1],g_THERMAL_TRIP[2],g_THERMAL_TRIP[3],g_THERMAL_TRIP[4],
				g_THERMAL_TRIP[5],g_THERMAL_TRIP[6],g_THERMAL_TRIP[7],g_THERMAL_TRIP[8],g_THERMAL_TRIP[9]);	

		mtktsbattery2_dprintk("[mtktsbattery2_write] [cool_bind]=%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n",
				g_bind[0],g_bind[1],g_bind[2],g_bind[3],g_bind[4],g_bind[5],g_bind[6],g_bind[7],g_bind[8],g_bind[9]);

		mtktsbattery2_dprintk("[mtktsbattery2_write] [trip_temp]==%d,%d,%d,%d,%d,%d,%d,%d,%d,%d, [time_ms]=%d\n", 
				trip_temp[0],trip_temp[1],trip_temp[2],trip_temp[3],trip_temp[4],
				trip_temp[5],trip_temp[6],trip_temp[7],trip_temp[8],trip_temp[9],interval*1000);
													
		mtktsbattery2_dprintk("[mtktsbattery2_write] register_thermal\n");
		mtktsbattery2_register_thermal();
				
		return count;
	}
	else
	{
		mtktsbattery2_dprintk("[mtktsbattery2_write] bad argument\n");
	}
		
	return -EINVAL;
}



static int mtktsbattery2_register_thermal(void)
{
	mtktsbattery2_dprintk("[mtktsbattery2_register_thermal] \n");

	/* trips : trip 0~1 */
	thz_dev = mtk_thermal_zone_device_register("mtktsbattery2", num_trip, NULL,
		&mtktsbattery2_dev_ops, 0, 0, 0, interval*1000);

	return 0;
}


static void mtktsbattery2_unregister_thermal(void)
{
	mtktsbattery2_dprintk("[mtktsbattery2_unregister_thermal] \n");

	if (thz_dev) {
		mtk_thermal_zone_device_unregister(thz_dev);
		thz_dev = NULL;
	}
}

static int __init mtktsbattery2_init(void)
{
	int err = 0;
	struct proc_dir_entry *entry = NULL;
	struct proc_dir_entry *mtktsbattery2_dir = NULL;

	mtktsbattery2_dprintk("[mtktsbattery2_init] \n");
	
	err = mtktsbattery2_register_thermal();
	if (err)
		goto err_unreg;

	mtktsbattery2_dir = proc_mkdir("mtktsbattery2", NULL);
	if (!mtktsbattery2_dir)
	{
		mtktsbattery2_dprintk("[mtktsbattery2_init]: mkdir /proc/mtktsbattery2 failed\n");
	}
	else
	{
		entry = create_proc_entry("mtktsbattery2", S_IRUGO | S_IWUSR, mtktsbattery2_dir);
		if (entry)
		{
			entry->read_proc = mtktsbattery2_read;
			entry->write_proc = mtktsbattery2_write;
		}
	}

	return 0;

err_unreg:
	return err;
}

static void __exit mtktsbattery2_exit(void)
{
	mtktsbattery2_dprintk("[mtktsbattery2_exit] \n");
	mtktsbattery2_unregister_thermal();
}

module_init(mtktsbattery2_init);
module_exit(mtktsbattery2_exit);

