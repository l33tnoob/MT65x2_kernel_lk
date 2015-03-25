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
#include <mach/upmu_hw.h>
#include <mach/upmu_common.h>
#include <mach/pmic_mt6323_sw.h>

static unsigned int interval = 1; /* seconds, 0 : no auto polling */
static unsigned int trip_temp[10] = {120000,110000,100000,90000,80000,70000,65000,60000,55000,50000};
static struct thermal_zone_device *thz_dev = NULL;

#if 0
static unsigned int cl_dev_sysrst_state = 0;
static struct thermal_cooling_device *cl_dev_sysrst = NULL;
#endif

static int  mtktsxtal_debug_log = 1;
static int  kernelmode = 0;
static int  g_THERMAL_TRIP[10] = {0,0,0,0,0,0,0,0,0,0};
static int  num_trip=0;
static char g_bind0[20]={0};
static char g_bind1[20]={0};
static char g_bind2[20]={0};
static char g_bind3[20]={0};
static char g_bind4[20]={0};
static char g_bind5[20]={0};
static char g_bind6[20]={0};
static char g_bind7[20]={0};
static char g_bind8[20]={0};
static char g_bind9[20]={0};

#define mtktsxtal_TEMP_CRIT 120000 /* 120.000 degree Celsius */



#define mtktsxtal_dprintk(fmt, args...)   \
do {                                    \
	if (mtktsxtal_debug_log) {                \
		xlog_printk(ANDROID_LOG_INFO, "Power/xtal_Thermal", fmt, ##args); \
	}                                   \
} while(0)

int mtktsxtal_register_thermal(void);
void mtktsxtal_unregister_thermal(void);
int mtktsxtal_register_cooler(void);



#define MIN_TSX_TEMP (-40)
#define MAX_TSX_TEMP (+85)

UINT32 u_table[126]={
9777,
9761,
9744,
9726,
9706,
9686,
9664,
9641,
9617,
9591,
9564,
9536,
9505,
9474,
9440,
9405,
9368,
9329,
9288,
9245,
9200,
9153,
9104,
9053,
8999,
8943,
8885,
8824,
8761,
8696,
8628,
8557,
8484,
8409,
8331,
8250,
8167,
8082,
7994,
7904,
7811,
7716,
7619,
7520,
7418,
7315,
7210,
7102,
6993,
6883,
6771,
6657,
6543,
6427,
6310,
6192,
6074,
5955,
5835,
5716,
5596,
5476,
5356,
5237,
5118,
5000,
4882,
4765,
4649,
4534,
4420,
4307,
4196,
4086,
3978,
3871,
3766,
3662,
3561,
3461,
3363,
3267,
3173,
3081,
2990,
2902,
2816,
2732,
2650,
2570,
2491,
2416,
2341,
2270,
2199,
2131,
2065,
2000,
1938,
1877,
1818,
1760,
1705,
1651,
1599,
1548,
1499,
1452,
1405,
1361,
1317,
1276,
1235,
1196,
1158,
1122,
1086,
1052,
1018,
986,
955,
925,
896,
868,
840,
814
};


int binarysearch(int data[], int search, int n)
{
    int top = 0, bottom = n - 1;
    int count=0;
    while ((top <= bottom) && (count < 127))
    {
        int mid = (top + bottom) / 2;
		//printk(" mid = %d\n",mid);

        if (data[mid] > search)
        {
            top = mid + 1;
			//printk(" top = %d\n",top);
            if(data[top] <= search){
				//printk(" top-1 = %d\n",top-1);
                return top-1;
            }
        }
        else if (data[mid] < search)
        {
            bottom = mid - 1;
            //printk(" bottom = %d\n",bottom);
            if(data[bottom] >= search){
				//printk(" data[bottom]> search ,%d\n",data[bottom]);
                return bottom;
            }
        }
        else{
            //printk("mid=%d, %d < %d < %d\n",mid,data[mid+1],search, data[mid]);
            return mid;
        }
        count++;
    }

    return -1;
}

INT32 TSX_U2T(UINT32 u)
{
	int i;
	INT32 ret;
    INT32 u_upper;
    INT32 u_low;
    INT32 t_upper;
    INT32 t_low;

	u = (u *10000)/65536;
	//printk(" u = %d\n",u);


    i = binarysearch(u_table, u, sizeof(u_table)/sizeof(UINT32));
	if(i < 0) return i;

    u_upper =  u_table[i+1];
    u_low   =  u_table[i];

    t_upper =   MIN_TSX_TEMP + (i+1);
    t_low   =   MIN_TSX_TEMP + i;

	//printk(" u_upper = %d\n",u_upper);
    //printk(" u_low   = %d\n",u_low);
    //printk(" t_upper = %d\n",t_upper);
    //printk(" t_low   = %d\n",t_low);


    if((u_upper-u_low) == 0 )
    {
       ret = (INT32)MIN_TSX_TEMP*1000;
    }
    else
    {
    	//Linear extrapolation
    	//(Y-Yk-1)/(Yk-Yk-1) = (X-Xk-1)/(Xk-Xk-1)
    	//Y = Yk-1 + (Yk-Yk-1)*(X-Xk-1)/(Xk-Xk-1)
        //ret = t_low + ((t_upper-t_low)/(u_upper-u_low))*(u-u_low);

        ret = 1000*t_low + (1000*(t_upper-t_low)*(u_low - u))/(u_low - u_upper);
    }

   return ret;
}
static DEFINE_MUTEX(mtktsxtal);



void upmu_set_rg_clksq_en_aux_md_set(kal_uint32 val)
{
  kal_uint32 ret=0;

  ret=pmic_config_interface( (kal_uint32)(TOP_CKPDN1_SET),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_CLKSQ_EN_AUX_MD_MASK),
                             (kal_uint32)(PMIC_RG_CLKSQ_EN_AUX_MD_SHIFT)
	                         );
}
void upmu_set_rg_clksq_en_aux_md_clr(kal_uint32 val)
{
  kal_uint32 ret=0;

  ret=pmic_config_interface( (kal_uint32)(TOP_CKPDN1_CLR),
                             (kal_uint32)(val),
                             (kal_uint32)(PMIC_RG_CLKSQ_EN_AUX_MD_MASK),
                             (kal_uint32)(PMIC_RG_CLKSQ_EN_AUX_MD_SHIFT)
	                         );
}



static int mtktsxtal_get_hw_temp(void)
{
	INT32 t_ret=0;
	INT32 val1 ,val2,count1;



	mutex_lock(&mtktsxtal);


	//printk(" mtktsxtal_get_hw_temp\n");

	upmu_set_rg_clksq_en_aux_md_set(1);
	upmu_set_rg_vref18_enb_md(0);

	upmu_set_rg_md_rqst(0);
    //printk(" 0--->1\n");
	upmu_set_rg_md_rqst(1);
    val1=0;
    count1=0;
    while((val1==0) && (count1 < 100)){
        count1++;
		val1 = upmu_get_rg_adc_rdy_md();
        //printk(" ready bit=%d\n",val1);
    }
	val2 = upmu_get_rg_adc_out_md();
	//printk(" val2=%d\n", val2);

    t_ret = TSX_U2T(val2); //convert
    if(t_ret < 0)
		printk(" XTAL Temp error,T=%d\n",t_ret);

	upmu_set_rg_md_rqst(0);
	upmu_set_rg_clksq_en_aux_md_clr(0);
    upmu_set_rg_vref18_enb_md(1);


	mutex_unlock(&mtktsxtal);


	mtktsxtal_dprintk("[mtktsxtal_get_hw_temp] T_xtal = %d\n", t_ret);
	return t_ret;
}

static int mtktsxtal_get_temp(struct thermal_zone_device *thermal,
			       unsigned long *t)
{
	*t = mtktsxtal_get_hw_temp();
	return 0;
}

static int mtktsxtal_bind(struct thermal_zone_device *thermal,
			struct thermal_cooling_device *cdev)
{
	int table_val=0;

	if(!strcmp(cdev->type, g_bind0))
	{
		table_val = 0;
		mtktsxtal_dprintk("[mtktsxtal_bind] %s\n", cdev->type);
	}
	else if(!strcmp(cdev->type, g_bind1))
	{
		table_val = 1;
		mtktsxtal_dprintk("[mtktsxtal_bind] %s\n", cdev->type);
	}
	else if(!strcmp(cdev->type, g_bind2))
	{
		table_val = 2;
		mtktsxtal_dprintk("[mtktsxtal_bind] %s\n", cdev->type);
	}
	else if(!strcmp(cdev->type, g_bind3))
	{
		table_val = 3;
		mtktsxtal_dprintk("[mtktsxtal_bind] %s\n", cdev->type);
	}
	else if(!strcmp(cdev->type, g_bind4))
	{
		table_val = 4;
		mtktsxtal_dprintk("[mtktsxtal_bind] %s\n", cdev->type);
	}
	else if(!strcmp(cdev->type, g_bind5))
	{
		table_val = 5;
		mtktsxtal_dprintk("[mtktsxtal_bind] %s\n", cdev->type);
	}
	else if(!strcmp(cdev->type, g_bind6))
	{
		table_val = 6;
		mtktsxtal_dprintk("[mtktsxtal_bind] %s\n", cdev->type);
	}
	else if(!strcmp(cdev->type, g_bind7))
	{
		table_val = 7;
		mtktsxtal_dprintk("[mtktsxtal_bind] %s\n", cdev->type);
	}
	else if(!strcmp(cdev->type, g_bind8))
	{
		table_val = 8;
		mtktsxtal_dprintk("[mtktsxtal_bind] %s\n", cdev->type);
	}
	else if(!strcmp(cdev->type, g_bind9))
	{
		table_val = 9;
		mtktsxtal_dprintk("[mtktsxtal_bind] %s\n", cdev->type);
	}
	else
	{
		return 0;
	}

	if (mtk_thermal_zone_bind_cooling_device(thermal, table_val, cdev)) {
		mtktsxtal_dprintk("[mtktsxtal_bind] error binding cooling dev\n");
		return -EINVAL;
	} else {
		mtktsxtal_dprintk("[mtktsxtal_bind] binding OK, %d\n", table_val);
	}

	return 0;
}

static int mtktsxtal_unbind(struct thermal_zone_device *thermal,
			  struct thermal_cooling_device *cdev)
{
    int table_val=0;

	if(!strcmp(cdev->type, g_bind0))
	{
		table_val = 0;
		mtktsxtal_dprintk("[mtktsxtal_unbind] %s\n", cdev->type);
	}
	else if(!strcmp(cdev->type, g_bind1))
	{
		table_val = 1;
		mtktsxtal_dprintk("[mtktsxtal_unbind] %s\n", cdev->type);
	}
	else if(!strcmp(cdev->type, g_bind2))
	{
		table_val = 2;
		mtktsxtal_dprintk("[mtktsxtal_unbind] %s\n", cdev->type);
	}
	else if(!strcmp(cdev->type, g_bind3))
	{
		table_val = 3;
		mtktsxtal_dprintk("[mtktsxtal_unbind] %s\n", cdev->type);
	}
	else if(!strcmp(cdev->type, g_bind4))
	{
		table_val = 4;
		mtktsxtal_dprintk("[mtktsxtal_unbind] %s\n", cdev->type);
	}
	else if(!strcmp(cdev->type, g_bind5))
	{
		table_val = 5;
		mtktsxtal_dprintk("[mtktsxtal_unbind] %s\n", cdev->type);
	}
	else if(!strcmp(cdev->type, g_bind6))
	{
		table_val = 6;
		mtktsxtal_dprintk("[mtktsxtal_unbind] %s\n", cdev->type);
	}
	else if(!strcmp(cdev->type, g_bind7))
	{
		table_val = 7;
		mtktsxtal_dprintk("[mtktsxtal_unbind] %s\n", cdev->type);
	}
	else if(!strcmp(cdev->type, g_bind8))
	{
		table_val = 8;
		mtktsxtal_dprintk("[mtktsxtal_unbind] %s\n", cdev->type);
	}
	else if(!strcmp(cdev->type, g_bind9))
	{
		table_val = 9;
		mtktsxtal_dprintk("[mtktsxtal_unbind] %s\n", cdev->type);
	}
	else
		return 0;

	if (thermal_zone_unbind_cooling_device(thermal, table_val, cdev)) {
		mtktsxtal_dprintk("[mtktsxtal_unbind] error unbinding cooling dev\n");
		return -EINVAL;
	} else {
		mtktsxtal_dprintk("[mtktsxtal_unbind] unbinding OK\n");
	}

	return 0;
}

static int mtktsxtal_get_mode(struct thermal_zone_device *thermal,
			    enum thermal_device_mode *mode)
{
	*mode = (kernelmode) ? THERMAL_DEVICE_ENABLED
			     : THERMAL_DEVICE_DISABLED;
	return 0;
}

static int mtktsxtal_set_mode(struct thermal_zone_device *thermal,
			    enum thermal_device_mode mode)
{
	kernelmode = mode;
	return 0;
}

static int mtktsxtal_get_trip_type(struct thermal_zone_device *thermal, int trip,
				 enum thermal_trip_type *type)
{
	*type = g_THERMAL_TRIP[trip];
	return 0;
}

static int mtktsxtal_get_trip_temp(struct thermal_zone_device *thermal, int trip,
				 unsigned long *temp)
{
	*temp = trip_temp[trip];
	return 0;
}

static int mtktsxtal_get_crit_temp(struct thermal_zone_device *thermal,
				 unsigned long *temperature)
{
	*temperature = mtktsxtal_TEMP_CRIT;
	return 0;
}

/* bind callback functions to thermalzone */
static struct thermal_zone_device_ops mtktsxtal_dev_ops = {
	.bind = 		mtktsxtal_bind,
	.unbind = 		mtktsxtal_unbind,
	.get_temp = 	mtktsxtal_get_temp,
	.get_mode = 	mtktsxtal_get_mode,
	.set_mode = 	mtktsxtal_set_mode,
	.get_trip_type = mtktsxtal_get_trip_type,
	.get_trip_temp = mtktsxtal_get_trip_temp,
	.get_crit_temp = mtktsxtal_get_crit_temp,
};

#if 1
static int mtktsxtal_log_read(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	int len = 0;
	char *p = buf;

	p += sprintf(p, "\n");
    p += sprintf(p, "XTAL log = %d\n", mtktsxtal_debug_log);
	*start = buf + off;

	len = p - buf;
	if (len > off)
		len -= off;
	else
		len = 0;

	return len < count ? len  : count;
}



static ssize_t mtktsxtal_log_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
	char desc[32];
	int len = 0;
    unsigned int enable_xtal=0;

	len = (count < (sizeof(desc) - 1)) ? count : (sizeof(desc) - 1);
	if (copy_from_user(desc, buffer, len))
	{
		return 0;
	}

	desc[len] = '\0';
	if (sscanf(desc, "%d",&enable_xtal) == 1){
        if(enable_xtal==1){
			mtktsxtal_debug_log = 1;
        }else{
			mtktsxtal_debug_log = 0;
        }
		return count;
    }
	else
	{
		printk("[mtktsxtal_write_log] bad argument\n");
		return -EINVAL;
	}

}
#endif


int mtktsxtal_register_thermal(void)
{
	mtktsxtal_dprintk("[mtktsxtal_register_thermal] \n");

	/* trips : trip 0~1 */
	thz_dev = mtk_thermal_zone_device_register("mtktsxtal", num_trip, NULL,
		&mtktsxtal_dev_ops, 0, 0, 0, interval*1000);

	return 0;
}

void mtktsxtal_unregister_thermal(void)
{
	mtktsxtal_dprintk("[mtktsxtal_unregister_thermal] \n");

	if (thz_dev) {
		mtk_thermal_zone_device_unregister(thz_dev);
		thz_dev = NULL;
	}
}

#if 0
static int tsxtal_sysrst_get_max_state(struct thermal_cooling_device *cdev,
				 unsigned long *state)
{
	mtktsxtal_dprintk("tsxtal_sysrst_get_max_state\n");
	*state = 1;
	return 0;
}
static int tsxtal_sysrst_get_cur_state(struct thermal_cooling_device *cdev,
				 unsigned long *state)
{
	mtktsxtal_dprintk("tsxtal_sysrst_get_cur_state\n");

	*state = cl_dev_sysrst_state;
	return 0;
}
static int tsxtal_sysrst_set_cur_state(struct thermal_cooling_device *cdev,
				 unsigned long state)
{
	mtktsxtal_dprintk("tsxtal_sysrst_set_cur_state\n");

	cl_dev_sysrst_state = state;
	if(cl_dev_sysrst_state == 1)
	{
		printk("XTAL_Thermal: ");
		printk("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
		printk("*****************************************");
		printk("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");

	}
	return 0;
}

static struct thermal_cooling_device_ops mtktsxtal_cooling_sysrst_ops = {
	.get_max_state = tsxtal_sysrst_get_max_state,
	.get_cur_state = tsxtal_sysrst_get_cur_state,
	.set_cur_state = tsxtal_sysrst_set_cur_state,
};
#endif

static int mtktsxtal_read(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
    int len = 0;
    char *p = buf;

		p += sprintf(p, "[ mtktsxtal_read] trip_0_temp=%d,trip_1_temp=%d,trip_2_temp=%d,trip_3_temp=%d,trip_4_temp=%d,\n\
trip_5_temp=%d,trip_6_temp=%d,trip_7_temp=%d,trip_8_temp=%d,trip_9_temp=%d,\n\
g_THERMAL_TRIP_0=%d,g_THERMAL_TRIP_1=%d,g_THERMAL_TRIP_2=%d,g_THERMAL_TRIP_3=%d,g_THERMAL_TRIP_4=%d,\n\
g_THERMAL_TRIP_5=%d,g_THERMAL_TRIP_6=%d,g_THERMAL_TRIP_7=%d,g_THERMAL_TRIP_8=%d,g_THERMAL_TRIP_9=%d,\n\
cooldev0=%s,cooldev1=%s,cooldev2=%s,cooldev3=%s,cooldev4=%s,\n\
cooldev5=%s,cooldev6=%s,cooldev7=%s,cooldev8=%s,cooldev9=%s,time_ms=%d\n",
								trip_temp[0],trip_temp[1],trip_temp[2],trip_temp[3],trip_temp[4],
								trip_temp[5],trip_temp[6],trip_temp[7],trip_temp[8],trip_temp[9],
								g_THERMAL_TRIP[0],g_THERMAL_TRIP[1],g_THERMAL_TRIP[2],g_THERMAL_TRIP[3],g_THERMAL_TRIP[4],
								g_THERMAL_TRIP[5],g_THERMAL_TRIP[6],g_THERMAL_TRIP[7],g_THERMAL_TRIP[8],g_THERMAL_TRIP[9],
								g_bind0,g_bind1,g_bind2,g_bind3,g_bind4,g_bind5,g_bind6,g_bind7,g_bind8,g_bind9,
								interval*1000);

    *start = buf + off;

    len = p - buf;
    if (len > off)
        len -= off;
    else
        len = 0;

    return len < count ? len  : count;
}

int mtktsabb_register_thermal(void);
void mtktsabb_unregister_thermal(void);

static ssize_t mtktsxtal_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
		int len=0,time_msec=0;
		int trip[10]={0};
		int t_type[10]={0};
		int i;
		char bind0[20],bind1[20],bind2[20],bind3[20],bind4[20];
		char bind5[20],bind6[20],bind7[20],bind8[20],bind9[20];
		char desc[512];


		len = (count < (sizeof(desc) - 1)) ? count : (sizeof(desc) - 1);
		if (copy_from_user(desc, buffer, len))
		{
				return 0;
		}
		desc[len] = '\0';

		if (sscanf(desc, "%d %d %d %s %d %d %s %d %d %s %d %d %s %d %d %s %d %d %s %d %d %s %d %d %s %d %d %s %d %d %s %d",
							&num_trip, &trip[0],&t_type[0],bind0, &trip[1],&t_type[1],bind1,
												 &trip[2],&t_type[2],bind2, &trip[3],&t_type[3],bind3,
												 &trip[4],&t_type[4],bind4, &trip[5],&t_type[5],bind5,
											   &trip[6],&t_type[6],bind6, &trip[7],&t_type[7],bind7,
												 &trip[8],&t_type[8],bind8, &trip[9],&t_type[9],bind9,
												 &time_msec) == 32)
		{
				mtktsxtal_dprintk("[mtktsxtal_write] mtktsxtal_unregister_thermal\n");
				mtktsxtal_unregister_thermal();

				for(i=0; i<num_trip; i++)
						g_THERMAL_TRIP[i] = t_type[i];

				g_bind0[0]=g_bind1[0]=g_bind2[0]=g_bind3[0]=g_bind4[0]=g_bind5[0]=g_bind6[0]=g_bind7[0]=g_bind8[0]=g_bind9[0]='\0';

				for(i=0; i<20; i++)
				{
					g_bind0[i]=bind0[i];
					g_bind1[i]=bind1[i];
					g_bind2[i]=bind2[i];
					g_bind3[i]=bind3[i];
					g_bind4[i]=bind4[i];
					g_bind5[i]=bind5[i];
					g_bind6[i]=bind6[i];
					g_bind7[i]=bind7[i];
					g_bind8[i]=bind8[i];
					g_bind9[i]=bind9[i];
				}

				mtktsxtal_dprintk("[mtktsxtal_write] g_THERMAL_TRIP_0=%d,g_THERMAL_TRIP_1=%d,g_THERMAL_TRIP_2=%d,g_THERMAL_TRIP_3=%d,g_THERMAL_TRIP_4=%d,\
g_THERMAL_TRIP_5=%d,g_THERMAL_TRIP_6=%d,g_THERMAL_TRIP_7=%d,g_THERMAL_TRIP_8=%d,g_THERMAL_TRIP_9=%d,\n",
													g_THERMAL_TRIP[0],g_THERMAL_TRIP[1],g_THERMAL_TRIP[2],g_THERMAL_TRIP[3],g_THERMAL_TRIP[4],
													g_THERMAL_TRIP[5],g_THERMAL_TRIP[6],g_THERMAL_TRIP[7],g_THERMAL_TRIP[8],g_THERMAL_TRIP[9]);
				mtktsxtal_dprintk("[mtktsxtal_write] cooldev0=%s,cooldev1=%s,cooldev2=%s,cooldev3=%s,cooldev4=%s,\
cooldev5=%s,cooldev6=%s,cooldev7=%s,cooldev8=%s,cooldev9=%s\n",
													g_bind0,g_bind1,g_bind2,g_bind3,g_bind4,g_bind5,g_bind6,g_bind7,g_bind8,g_bind9);

				for(i=0; i<num_trip; i++)
				{
						trip_temp[i]=trip[i];
				}

				interval=time_msec / 1000;

				mtktsxtal_dprintk("[mtktsxtal_write] trip_0_temp=%d,trip_1_temp=%d,trip_2_temp=%d,trip_3_temp=%d,trip_4_temp=%d,\
trip_5_temp=%d,trip_6_temp=%d,trip_7_temp=%d,trip_8_temp=%d,trip_9_temp=%d,time_ms=%d\n",
						trip_temp[0],trip_temp[1],trip_temp[2],trip_temp[3],trip_temp[4],
						trip_temp[5],trip_temp[6],trip_temp[7],trip_temp[8],trip_temp[9],interval*1000);

				mtktsxtal_dprintk("[mtktsxtal_write] mtktsabb_register_thermal\n");
				mtktsxtal_register_thermal();

				return count;
		}
		else
		{
				mtktsxtal_dprintk("[mtktsxtal_write] bad argument\n");
		}

		return -EINVAL;
}


int mtktsxtal_register_cooler(void)
{
#if 0
	mtktsxtal_dprintk("mtktsxtal_register_cooler\n");
	cl_dev_sysrst = mtk_thermal_cooling_device_register("2500", NULL,
					   &mtktsxtal_cooling_sysrst_ops);
#endif
   	return 0;
}


static int __init mtktsxtal_init(void)
{


	struct proc_dir_entry *entry = NULL;
	struct proc_dir_entry *mtktsxtal_dir = NULL;

	mtktsxtal_dprintk("[mtktsxtal_init] \n");

	mtktsxtal_dir = proc_mkdir("mtktsxtal", NULL);
	if (!mtktsxtal_dir)
	{
		mtktsxtal_dprintk("[mtktsxtal_init]: mkdir /proc/mtktsxtal failed\n");
	}
	else
	{
		entry = create_proc_entry("mtktsxtal_log", S_IRUGO | S_IWUSR, mtktsxtal_dir);
		if (entry)
		{
			entry->read_proc = mtktsxtal_log_read;
			entry->write_proc = mtktsxtal_log_write;
		}

        entry = create_proc_entry("mtktsxtal", S_IRUGO | S_IWUSR | S_IWGRP, mtktsxtal_dir);
        if (entry)
        {
            entry->read_proc = mtktsxtal_read;
            entry->write_proc = mtktsxtal_write;
            entry->gid = 1000;
        }
	}

	return 0;

}

static void __exit mtktsxtal_exit(void)
{
	mtktsxtal_dprintk("[mtktsxtal_exit] \n");

	mtktsxtal_unregister_thermal();

}

module_init(mtktsxtal_init);
module_exit(mtktsxtal_exit);


