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

/* Zone */
static struct thermal_zone_device *thz_dev;
static unsigned int interval = 0; /* seconds, 0 : no auto polling */
static unsigned int trip_temp[MTK_TZ_COOLER_MAX] = {120000,110000,100000,90000,80000,70000,65000,60000,55000,50000};
static int kernelmode = 0;
static int g_THERMAL_TRIP[MTK_TZ_COOLER_MAX] = {0,0,0,0,0,0,0,0,0,0};
static int num_trip=0;
static char g_bind[MTK_TZ_COOLER_MAX][THERMAL_NAME_LENGTH] = {{0},{0},{0},{0},{0},{0},{0},{0},{0},{0}};
#define MTKTSBATTERY_TEMP_CRIT 60000 /* 60.000 degree Celsius */

/* Cooler */
static unsigned int cl_dev_sysrst_state = 0;
static struct thermal_cooling_device *cl_dev_sysrst;

/* Logging */
static int mtktsbattery_debug_log = 0;
#define mtktsbattery_dprintk(fmt, args...)   \
do {                                    \
	if (mtktsbattery_debug_log) {                \
		xlog_printk(ANDROID_LOG_INFO, "Power/Battery_Thermal", fmt, ##args); \
	}                                   \
} while(0)

static int mtktsbattery_register_thermal(void);
static void mtktsbattery_unregister_thermal(void);
static int mtkts_match(struct thermal_cooling_device *cdev, char bind[MTK_TZ_COOLER_MAX][THERMAL_NAME_LENGTH]);
extern int read_tbat_value(void);

static int get_hw_battery_temp(void)
{
	int ret=0;
#if defined(CONFIG_POWER_EXT)
	//EVB
	ret = -1270;
#else
	//Phone
	ret = read_tbat_value();
#endif	
	return ret;
}

static int mtktsbattery_get_temp(struct thermal_zone_device *thermal,
			       unsigned long *t)
{
	*t = (unsigned long)get_hw_battery_temp() * 1000;
	mtktsbattery_dprintk("[mtktsbattery_get_hw_temp] T_Battery, %d\n", *t);

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


static int mtktsbattery_bind(struct thermal_zone_device *thermal,
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
		mtktsbattery_dprintk("[mtktsbattery_bind] %s\n", cdev->type);	
		if (mtk_thermal_zone_bind_cooling_device(thermal, table_val, cdev)) {
			mtktsbattery_dprintk("[mtktsbattery_bind] error binding cooling dev\n");
			return -EINVAL;
		} else {
			mtktsbattery_dprintk("[mtktsbattery_bind] binding OK, %d\n", table_val);
		}	
	}
	return 0;  
}

static int mtktsbattery_unbind(struct thermal_zone_device *thermal,
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
		mtktsbattery_dprintk("[mtktsbattery_unbind] %s\n", cdev->type); 
		if (thermal_zone_unbind_cooling_device(thermal, table_val, cdev)) {
			mtktsbattery_dprintk("[mtktsbattery_unbind] error unbinding cooling dev\n");
			return -EINVAL;
		} else {
			mtktsbattery_dprintk("[mtktsbattery_unbind] unbinding OK, %d\n", table_val);
		}	
	}
	return 0;  
}

static int mtktsbattery_get_mode(struct thermal_zone_device *thermal,
			    enum thermal_device_mode *mode)
{
	*mode = (kernelmode) ? THERMAL_DEVICE_ENABLED
			     : THERMAL_DEVICE_DISABLED;
	return 0;
}

static int mtktsbattery_set_mode(struct thermal_zone_device *thermal,
			    enum thermal_device_mode mode)
{
	kernelmode = mode;
	return 0;
}

static int mtktsbattery_get_trip_type(struct thermal_zone_device *thermal, int trip,
				 enum thermal_trip_type *type)
{
	*type = g_THERMAL_TRIP[trip];
	return 0;
}

static int mtktsbattery_get_trip_temp(struct thermal_zone_device *thermal, int trip,
				 unsigned long *temp)
{
	*temp = trip_temp[trip]; 
	return 0;
}

static int mtktsbattery_get_crit_temp(struct thermal_zone_device *thermal,
				 unsigned long *temperature)
{
	*temperature = MTKTSBATTERY_TEMP_CRIT;
	return 0;
}

/* bind callback functions to thermalzone */
static struct thermal_zone_device_ops mtktsbattery_dev_ops = {
	.bind = mtktsbattery_bind,
	.unbind = mtktsbattery_unbind,
	.get_temp = mtktsbattery_get_temp,
	.get_mode = mtktsbattery_get_mode,
	.set_mode = mtktsbattery_set_mode,
	.get_trip_type = mtktsbattery_get_trip_type,
	.get_trip_temp = mtktsbattery_get_trip_temp,
	.get_crit_temp = mtktsbattery_get_crit_temp,
};


static int sysrst_get_max_state(struct thermal_cooling_device *cdev,
				 unsigned long *state)
{        
	*state = 1;    
	return 0;
}
static int sysrst_get_cur_state(struct thermal_cooling_device *cdev,
				 unsigned long *state)
{        
	*state = cl_dev_sysrst_state;
	return 0;
}
static int sysrst_set_cur_state(struct thermal_cooling_device *cdev,
				 unsigned long state)
{
	cl_dev_sysrst_state = state;
	if(cl_dev_sysrst_state == 1)
	{
		printk("Power/battery_Thermal: reset, reset, reset!!!");
//		printk("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
//		printk("*****************************************");
//		printk("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");

		BUG();
		//arch_reset(0,NULL);   
	} 
	return 0;
}

static struct thermal_cooling_device_ops mtktsbattery_cooling_sysrst_ops = {
	.get_max_state = sysrst_get_max_state,
	.get_cur_state = sysrst_get_cur_state,
	.set_cur_state = sysrst_set_cur_state,
};


static int mtktsbattery_read(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	int len = 0;
	char *p = buf;
    
	p+= sprintf(p, "[mtktsbattery_read]\n\
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



static ssize_t mtktsbattery_write(struct file *file, const char *buffer, unsigned long count, void *data)
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
		mtktsbattery_dprintk("[mtktsbattery_write] unregister_thermal\n");
		mtktsbattery_unregister_thermal();
	
		for(i=0; i<MTK_TZ_COOLER_MAX; i++)
		{
			g_THERMAL_TRIP[i] = t_type[i];	
            memcpy(g_bind[i], bind[i], THERMAL_NAME_LENGTH);			
			trip_temp[i]=trip[i];			
		}
		interval=time_msec / 1000;

		mtktsbattery_dprintk("[mtktsbattery_write] [trip_type]=%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
				g_THERMAL_TRIP[0],g_THERMAL_TRIP[1],g_THERMAL_TRIP[2],g_THERMAL_TRIP[3],g_THERMAL_TRIP[4],
				g_THERMAL_TRIP[5],g_THERMAL_TRIP[6],g_THERMAL_TRIP[7],g_THERMAL_TRIP[8],g_THERMAL_TRIP[9]);	

		mtktsbattery_dprintk("[mtktsbattery_write] [cool_bind]=%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n",
				g_bind[0],g_bind[1],g_bind[2],g_bind[3],g_bind[4],g_bind[5],g_bind[6],g_bind[7],g_bind[8],g_bind[9]);

		mtktsbattery_dprintk("[mtktsbattery_write] [trip_temp]==%d,%d,%d,%d,%d,%d,%d,%d,%d,%d, [time_ms]=%d\n", 
				trip_temp[0],trip_temp[1],trip_temp[2],trip_temp[3],trip_temp[4],
				trip_temp[5],trip_temp[6],trip_temp[7],trip_temp[8],trip_temp[9],interval*1000);
													
		mtktsbattery_dprintk("[mtktsbattery_write] register_thermal\n");
		mtktsbattery_register_thermal();
				
		return count;
	}
	else
	{
		mtktsbattery_dprintk("[mtktsbattery_write] bad argument\n");
	}
		
	return -EINVAL;
}


static int  mtktsbattery_register_cooler(void)
{
	/* cooling devices */
	cl_dev_sysrst = mtk_thermal_cooling_device_register("mtktsbattery-sysrst", NULL,
		&mtktsbattery_cooling_sysrst_ops);
	return 0;
}

static int mtktsbattery_register_thermal(void)
{
	mtktsbattery_dprintk("[mtktsbattery_register_thermal] \n");

	/* trips : trip 0~1 */
	thz_dev = mtk_thermal_zone_device_register("mtktsbattery", num_trip, NULL,
		&mtktsbattery_dev_ops, 0, 0, 0, interval*1000);

	return 0;
}

static void mtktsbattery_unregister_cooler(void)
{
	if (cl_dev_sysrst) {
		mtk_thermal_cooling_device_unregister(cl_dev_sysrst);
		cl_dev_sysrst = NULL;
	}
}

static void mtktsbattery_unregister_thermal(void)
{
	mtktsbattery_dprintk("[mtktsbattery_unregister_thermal] \n");

	if (thz_dev) {
		mtk_thermal_zone_device_unregister(thz_dev);
		thz_dev = NULL;
	}
}

static int __init mtktsbattery_init(void)
{
	int err = 0;
	struct proc_dir_entry *entry = NULL;
	struct proc_dir_entry *mtktsbattery_dir = NULL;

	mtktsbattery_dprintk("[mtktsbattery_init] \n");

	err = mtktsbattery_register_cooler();
	if(err)
		return err;
	
	err = mtktsbattery_register_thermal();
	if (err)
		goto err_unreg;

	mtktsbattery_dir = proc_mkdir("mtktsbattery", NULL);
	if (!mtktsbattery_dir)
	{
		mtktsbattery_dprintk("[mtktsbattery_init]: mkdir /proc/mtktsbattery failed\n");
	}
	else
	{
		entry = create_proc_entry("mtktsbattery", S_IRUGO | S_IWUSR, mtktsbattery_dir);
		if (entry)
		{
			entry->read_proc = mtktsbattery_read;
			entry->write_proc = mtktsbattery_write;
		}
	}

	return 0;

err_unreg:
	mtktsbattery_unregister_cooler();
	return err;
}

static void __exit mtktsbattery_exit(void)
{
	mtktsbattery_dprintk("[mtktsbattery_exit] \n");
	mtktsbattery_unregister_thermal();
	mtktsbattery_unregister_cooler();
}

module_init(mtktsbattery_init);
module_exit(mtktsbattery_exit);
