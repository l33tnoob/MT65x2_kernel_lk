#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/random.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include "met_drv.h"
#include "mt_gpufreq.h"

extern struct metdevice met_gpu;
static struct delayed_work dwork;

//#define FMT7	",%d,%d,%d,%d,%d\n"
//#define VAL7	,value[0],value[1],value[2],value[3],value[4]
#define FMT7	"%d,%d,%d,%d,%d\n"
#define VAL7	value[0],value[1],value[2],value[3],value[4]

#define SAMPLE_FMT	"%5lu.%06lu"
#define SAMPLE_VAL	(unsigned long)(timestamp), nano_rem/1000

static const int metDelay = 20;

void ms_gpu(unsigned long long timestamp, unsigned char cnt, unsigned int *value)
{
    //unsigned long nano_rem = do_div(timestamp, 1000000000);

    switch (cnt) {
    //	case 5: trace_printk(SAMPLE_FMT FMT7, SAMPLE_VAL VAL7); break;
    case 5: trace_printk(FMT7, VAL7); break;
    }
}

static void wq_get_sample(struct work_struct *work)
{
    if (met_gpu.mode) {
        int cpu;
        unsigned int value[7];
        unsigned long long stamp;

        cpu = smp_processor_id();
        stamp = cpu_clock(cpu);

        value[0] = (mt_gpufreq_cur_freq()/1000);  //GPU Clock
        value[1] = mt_gpufreq_cur_load();      //GPU Utilization
        value[2] = 0;                             //GP Frame Rate
        value[3] = 0;                             //PP Frame Rate
        value[4] = 0;                             //Instruction Complete Count

        ms_gpu(stamp, 5, value);
        msleep(metDelay);
        schedule_delayed_work(&dwork, 0);
    }
}

//It will be called back when run "met-cmd --start"
static void sample_start(void)
{
	INIT_DELAYED_WORK(&dwork, wq_get_sample);
	schedule_delayed_work(&dwork, 0);
	return;
}

//It will be called back when run "met-cmd --stop"
static void sample_stop(void)
{
	cancel_delayed_work_sync(&dwork);
	return;
}

//static char header[] = "met-info [000] 0.0: ms_ud_sys_header: ms_gpu,timestamp,clock,util,GPFR,PPFR,ICC,d,d,d,d,d\n";
static char header[] = "met-info [000] 0.0: ms_ud_sys_header: ms_gpu,clock (MHz),util (%),GPFR,PPFR,ICC,d,d,d,d,d\n";
static char help[] = "  --gpu                                 monitor gpu\n";


//It will be called back when run "met-cmd -h"
static int sample_print_help(char *buf, int len)
{
	return snprintf(buf, PAGE_SIZE, help);
}

//It will be called back when run "met-cmd --extract" and mode is 1
static int sample_print_header(char *buf, int len)
{
	return snprintf(buf, PAGE_SIZE, header);
}

struct metdevice met_gpu = {
	.name = "gpu",
	.owner = THIS_MODULE,
	.type = MET_TYPE_BUS,
	.start = sample_start,
	.stop = sample_stop,
	.print_help = sample_print_help,
	.print_header = sample_print_header,
};

EXPORT_SYMBOL(met_gpu);
