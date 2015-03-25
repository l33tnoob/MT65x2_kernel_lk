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

#include <mach/system.h>
#include "mach/mtk_thermal_monitor.h"
#include "mach/mt_typedefs.h"
#include "mach/mt_thermal.h"

#include <mach/upmu_common_sw.h>
#include <mach/upmu_hw.h>
#include <mach/mt_pmic_wrap.h>

#define MTK_TZ_COOLER_MAX 10
#define THERMAL_PMIC_REPEAT 1
#define THERMAL_PMIC_CHANNEL 3

/* Zone */
static struct thermal_zone_device *thz_dev;
static unsigned int interval = 0; /* seconds, 0 : no auto polling */
static unsigned int trip_temp[MTK_TZ_COOLER_MAX] = {120000,110000,100000,90000,80000,70000,65000,60000,55000,50000};
static int g_THERMAL_TRIP[MTK_TZ_COOLER_MAX] = {0,0,0,0,0,0,0,0,0,0};
static int num_trip=0;
static char g_bind[MTK_TZ_COOLER_MAX][THERMAL_NAME_LENGTH] = {{0},{0},{0},{0},{0},{0},{0},{0},{0},{0}};
static int kernelmode = 0;
#define mtktspmic_TEMP_CRIT 150000 /* 150.000 degree Celsius */

/* Cooler */
static unsigned int cl_dev_sysrst_state = 0;
static struct thermal_cooling_device *cl_dev_sysrst;

/* Logging */
static int mtktspmic_debug_log = 0;
#define mtktspmic_dprintk(fmt, args...)   \
do {									\
	if (mtktspmic_debug_log) {				\
		xlog_printk(ANDROID_LOG_INFO, "Power/PMIC_Thermal", fmt, ##args); \
	}								   \
} while(0)

/* Cali */
static kal_int32 g_o_vts = 0;
static kal_int32 g_degc_cali = 0;
static kal_int32 g_adc_cali_en = 0;
static kal_int32 g_o_slope = 0;
static kal_int32 g_o_slope_sign = 0;
static kal_int32 g_id = 0;
static kal_int32 g_slope1;
static kal_int32 g_slope2;
static kal_int32 g_intercept;

static int mtktspmic_register_thermal(void);
static void mtktspmic_unregister_thermal(void);
static int mtkts_match(struct thermal_cooling_device *cdev, char bind[MTK_TZ_COOLER_MAX][THERMAL_NAME_LENGTH]);
extern int PMIC_IMM_GetOneChannelValue(int dwChannel, int deCount, int trimd);
kal_uint32 upmu_get_cid(void);

static u16 pmic_read(u16 addr)
{
	u32 rdata=0;
	pwrap_read((u32)addr, &rdata);
	return (u16)rdata;
}

static void pmic_cali_prepare(void)
{
	kal_uint32 temp0, temp1;
	
	temp0 = pmic_read(0x63A);
	temp1 = pmic_read(0x63C);
	
	printk("Power/PMIC_Thermal: Reg(0x63a)=0x%x, Reg(0x63c)=0x%x\n", temp0, temp1);

	g_o_vts = ((temp1&0x001F) << 8) + ((temp0>>8) & 0x00FF);
	g_degc_cali = (temp0>>2) & 0x003f;
	g_adc_cali_en = (temp0>>1) & 0x0001;
	g_o_slope_sign = (temp1>>5) & 0x0001;
	if(upmu_get_cid() == 0x1023)	
	{
		g_id= (temp1>>12) & 0x0001;
    	g_o_slope = (temp1>>6) & 0x003f;
	}
	else
	{
		g_id= (temp1>>14) & 0x0001;
    	g_o_slope = (((temp1>>11) & 0x0007) << 3) + ((temp1>>6) & 0x007);	
	}
	
	if(g_id==0)
	{
	   g_o_slope=0;
	}
	
	if(g_adc_cali_en == 0) //no calibration
	{
		g_o_vts = 3698;
		g_degc_cali = 50;
		g_o_slope = 0;
		g_o_slope_sign = 0;
	}
	printk("Power/PMIC_Thermal: g_ver= 0x%x, g_o_vts = 0x%x, g_degc_cali = 0x%x, g_adc_cali_en = 0x%x, g_o_slope = 0x%x, g_o_slope_sign = 0x%x, g_id = 0x%x\n", 
		 upmu_get_cid(),g_o_vts, g_degc_cali, g_adc_cali_en, g_o_slope, g_o_slope_sign, g_id);

	
}

static void pmic_cali_prepare2(void)
{
	kal_int32 vbe_t;
	g_slope1 = (100 * 1000);	//1000 is for 0.001 degree
	if(g_o_slope_sign==0)
	{		
		g_slope2 = -(171+g_o_slope);     
	}
	else
	{
		g_slope2 = -(171-g_o_slope);
	} 	
	vbe_t= (-1) * (((g_o_vts + 9102)*1800)/32768) * 1000;
	if(g_o_slope_sign==0)
	{		
		g_intercept = (vbe_t * 100) / (-(171+g_o_slope)); 	//0.001 degree
	}
	else
	{
		g_intercept = (vbe_t * 100) / (-(171-g_o_slope));  //0.001 degree
	} 
	g_intercept = g_intercept + (g_degc_cali*(1000/2)); // 1000 is for 0.1 degree
	printk("[Power/PMIC_Thermal] [Thermal calibration] SLOPE1=%d SLOPE2=%d INTERCEPT=%d, Vbe = %d\n",
		g_slope1, g_slope2, g_intercept,vbe_t);	

}

static kal_int32 pmic_raw_to_temp(kal_uint32 ret)
{
	kal_int32 y_curr = ret;
	kal_int32 t_current;
	t_current = g_intercept + ((g_slope1 * y_curr) / (g_slope2));
	return t_current;		
}

extern void pmic_thermal_dump_reg(void);
	
//int ts_pmic_at_boot_time=0;
static DEFINE_MUTEX(TSPMIC_lock);
static int pre_temp1=0, PMIC_counter=0;
static int mtktspmic_get_hw_temp(void)
{
	int temp=0, temp1=0;

	
	mutex_lock(&TSPMIC_lock);
	
	temp = PMIC_IMM_GetOneChannelValue(THERMAL_PMIC_CHANNEL, THERMAL_PMIC_REPEAT , 1);
	temp1 = pmic_raw_to_temp(temp);

	if((temp1>100000) || (temp1<-30000))
	{	
		printk("[Power/PMIC_Thermal] raw=%d, PMIC T=%d", temp, temp1);
//		pmic_thermal_dump_reg();
	}
	
	if((temp1>150000) || (temp1<-50000))
	{
		printk("[Power/PMIC_Thermal] drop this data\n");
		temp1 = pre_temp1;
	}
	else if( (PMIC_counter!=0) && (((pre_temp1-temp1)>30000) || ((temp1-pre_temp1)>30000)) )
	{
		printk("[Power/PMIC_Thermal] drop this data 2\n");
		temp1 = pre_temp1;
	}	
	else
	{
		//update previous temp
		pre_temp1 = temp1;
		mtktspmic_dprintk("[Power/PMIC_Thermal] pre_temp1=%d\n", pre_temp1);
		
		if(PMIC_counter==0)
			PMIC_counter++;	
	}
	
	mutex_unlock(&TSPMIC_lock);
	return temp1;
}
	
static int mtktspmic_get_temp(struct thermal_zone_device *thermal,
				   unsigned long *t)
{
	*t = mtktspmic_get_hw_temp();
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


static int mtktspmic_bind(struct thermal_zone_device *thermal,
			struct thermal_cooling_device *cdev)
{
	int table_val=0;
	table_val = mtkts_match(cdev,g_bind);
	if(table_val >= MTK_TZ_COOLER_MAX ) 
	{
		return 0;
	}
	else
	{
		mtktspmic_dprintk("[mtktspmic_bind] %s\n", cdev->type);	
		if (mtk_thermal_zone_bind_cooling_device(thermal, table_val, cdev)) {
			mtktspmic_dprintk("[mtktspmic_bind] error binding cooling dev\n");
			return -EINVAL;
		} else {
			mtktspmic_dprintk("[mtktspmic_bind] binding OK, %d\n", table_val);
		}	
	}
	return 0;  
}

static int mtktspmic_unbind(struct thermal_zone_device *thermal,
			  struct thermal_cooling_device *cdev)
{
	int table_val=0;
	table_val = mtkts_match(cdev,g_bind);
	if(table_val >= MTK_TZ_COOLER_MAX ) 
	{
		return 0;
	}
	else
	{
		mtktspmic_dprintk("[mtktspmic_unbind] %s\n", cdev->type); 
		if (thermal_zone_unbind_cooling_device(thermal, table_val, cdev)) {
			mtktspmic_dprintk("[mtktspmic_unbind] error unbinding cooling dev\n");
			return -EINVAL;
		} else {
			mtktspmic_dprintk("[mtktspmic_unbind] unbinding OK, %d\n", table_val);
		}	
	}
	return 0;  
}

static int mtktspmic_get_mode(struct thermal_zone_device *thermal,
				enum thermal_device_mode *mode)
{
	*mode = (kernelmode) ? THERMAL_DEVICE_ENABLED
				 : THERMAL_DEVICE_DISABLED;
	return 0;
}

static int mtktspmic_set_mode(struct thermal_zone_device *thermal,
				enum thermal_device_mode mode)
{
	kernelmode = mode;
	return 0;
}

static int mtktspmic_get_trip_type(struct thermal_zone_device *thermal, int trip,
				 enum thermal_trip_type *type)
{
	*type = g_THERMAL_TRIP[trip];
	return 0;
}

static int mtktspmic_get_trip_temp(struct thermal_zone_device *thermal, int trip,
				 unsigned long *temp)
{
	*temp = trip_temp[trip]; 
	return 0;
}

static int mtktspmic_get_crit_temp(struct thermal_zone_device *thermal,
				 unsigned long *temperature)
{
	*temperature = mtktspmic_TEMP_CRIT;
	return 0;
}

/* bind callback functions to thermalzone */
static struct thermal_zone_device_ops mtktspmic_dev_ops = {
	.bind = mtktspmic_bind,
	.unbind = mtktspmic_unbind,
	.get_temp = mtktspmic_get_temp,
	.get_mode = mtktspmic_get_mode,
	.set_mode = mtktspmic_set_mode,
	.get_trip_type = mtktspmic_get_trip_type,
	.get_trip_temp = mtktspmic_get_trip_temp,
	.get_crit_temp = mtktspmic_get_crit_temp,
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
		printk("Power/PMIC_Thermal: reset, reset, reset!!!");
//		printk("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
//		printk("*****************************************");
//		printk("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
		BUG();
		//arch_reset(0,NULL);   
	}	
	return 0;
}

static struct thermal_cooling_device_ops mtktspmic_cooling_sysrst_ops = {
	.get_max_state = sysrst_get_max_state,
	.get_cur_state = sysrst_get_cur_state,
	.set_cur_state = sysrst_set_cur_state,
};


static int mtktspmic_read_cal(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	int len = 0;
	char *p = buf;

	p += sprintf(p, "mtktspmic cal:\nReg(0x63A)=0x%x, Reg(0x63C)=0x%x\n", 
	                pmic_read(0x63A), pmic_read(0x63C));

	*start = buf + off;

	len = p - buf;
	if (len > off)
		len -= off;
	else
		len = 0;
        
	return len < count ? len  : count;
}


static int mtktspmic_read(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	int len = 0;
	char *p = buf;
	
	p+= sprintf(p, "[mtktspmic_read]\n\
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


static ssize_t mtktspmic_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
	int len=0,time_msec=0;
	int trip[10]={0};
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
		mtktspmic_dprintk("[mtktspmic_write] unregister_thermal\n");
		mtktspmic_unregister_thermal();
	
		for(i=0; i<MTK_TZ_COOLER_MAX; i++)
		{
			g_THERMAL_TRIP[i] = t_type[i];	
			memcpy(g_bind[i], bind[i], THERMAL_NAME_LENGTH); 		
			trip_temp[i]=trip[i];			
		}		
		interval=time_msec / 1000;


		mtktspmic_dprintk("[mtktspmic_write] [trip_type]=%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
				g_THERMAL_TRIP[0],g_THERMAL_TRIP[1],g_THERMAL_TRIP[2],g_THERMAL_TRIP[3],g_THERMAL_TRIP[4],
				g_THERMAL_TRIP[5],g_THERMAL_TRIP[6],g_THERMAL_TRIP[7],g_THERMAL_TRIP[8],g_THERMAL_TRIP[9]);
		
		mtktspmic_dprintk("[mtktspmic_write] [cool_bind]=%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n",
				g_bind[0],g_bind[1],g_bind[2],g_bind[3],g_bind[4],g_bind[5],g_bind[6],g_bind[7],g_bind[8],g_bind[9]);

		mtktspmic_dprintk("[mtktspmic_write] [trip_temp]==%d,%d,%d,%d,%d,%d,%d,%d,%d,%d, [time_ms]=%d\n", 
				trip_temp[0],trip_temp[1],trip_temp[2],trip_temp[3],trip_temp[4],
				trip_temp[5],trip_temp[6],trip_temp[7],trip_temp[8],trip_temp[9],interval*1000);
													
		mtktspmic_dprintk("[mtktspmic_write] register_thermal\n");
		mtktspmic_register_thermal();
				
		return count;
	}
	else
	{
		mtktspmic_dprintk("[mtktspmic_write] bad argument\n");
	}
		
	return -EINVAL;
}

static int mtktspmic_register_cooler(void)
{
	cl_dev_sysrst = mtk_thermal_cooling_device_register("mtktspmic-sysrst", NULL,
					   &mtktspmic_cooling_sysrst_ops);		
   	return 0;
}

static int mtktspmic_register_thermal(void)
{
	mtktspmic_dprintk("[mtktspmic_register_thermal] \n");

	/* trips : trip 0~2 */
	thz_dev = mtk_thermal_zone_device_register("mtktspmic", num_trip, NULL,
					  &mtktspmic_dev_ops, 0, 0, 0, interval*1000);

	return 0;
}

static void mtktspmic_unregister_cooler(void)
{
	if (cl_dev_sysrst) {
		mtk_thermal_cooling_device_unregister(cl_dev_sysrst);
		cl_dev_sysrst = NULL;
	}
}

static void mtktspmic_unregister_thermal(void)
{
	mtktspmic_dprintk("[mtktspmic_unregister_thermal] \n");
	
	if (thz_dev) {
		mtk_thermal_zone_device_unregister(thz_dev);
		thz_dev = NULL;
	}
}

static int __init mtktspmic_init(void)
{
	int err = 0;
	struct proc_dir_entry *entry = NULL;
	struct proc_dir_entry *mtktspmic_dir = NULL;

	mtktspmic_dprintk("[mtktspmic_init] \n");
	pmic_cali_prepare();
	pmic_cali_prepare2();
			
	err = mtktspmic_register_cooler();
	if(err)
		return err;
	err = mtktspmic_register_thermal();
	if (err)
		goto err_unreg;

	mtktspmic_dir = proc_mkdir("mtktspmic", NULL);
	if (!mtktspmic_dir)
	{
		mtktspmic_dprintk("[mtktspmic_init]: mkdir /proc/mtktspmic failed\n");
	}
	else
	{
		entry = create_proc_entry("mtktspmic", S_IRUGO | S_IWUSR, mtktspmic_dir);
		if (entry)
		{
			entry->read_proc = mtktspmic_read;
			entry->write_proc = mtktspmic_write;
		}

		entry = create_proc_entry("mtktspmic_cal", S_IRUGO, mtktspmic_dir);
		if (entry)
		{
			entry->read_proc = mtktspmic_read_cal;
			entry->write_proc = NULL;
		}
	}

	return 0;

err_unreg:
		mtktspmic_unregister_cooler();
		return err;
}

static void __exit mtktspmic_exit(void)
{
	mtktspmic_dprintk("[mtktspmic_exit] \n");
	mtktspmic_unregister_thermal();
	mtktspmic_unregister_cooler();
}

module_init(mtktspmic_init);
module_exit(mtktspmic_exit);
