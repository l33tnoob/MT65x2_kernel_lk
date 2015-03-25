#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/spinlock.h>
#include <linux/kthread.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/interrupt.h>
#include <linux/syscore_ops.h>
#include <linux/platform_device.h>

#include "mach/mt_reg_base.h"
#include "mach/mt_typedefs.h"

#include "mach/irqs.h"
#include "mach/mt_ptp.h"
#include "mach/mt_cpufreq.h"
#include "mach/mt_thermal.h"
#include "mach/mt_spm_idle.h"
#include "mach/mt_pmic_wrap.h"

#include "mach/mtk_rtc_hal.h"
#include "mach/mt_rtc_hw.h"

/* Global variable */
volatile unsigned int ptp_data[3] = {0, 0, 0};

static unsigned int val_0 = 0x00000000;
static unsigned int val_1 = 0x00000000;
static unsigned int val_2 = 0x00000000;
static unsigned int val_3 = 0x00000000;

/* For PID546 */
#define val_0_1 0x153A4207;
#define val_1_1 0xF6555555;
#define val_2_1 0x00000000;
#define val_3_1 0x307D0788;

/* For PID308 */
#define val_0_2 0x187B1007;
#define val_1_2 0xCD555555;
#define val_2_2 0x00000000;
#define val_3_2 0x30A90788;

static unsigned char freq_0, freq_1, freq_2, freq_3;
static unsigned char freq_4, freq_5, freq_6, freq_7;

static unsigned int ptp_level;
static unsigned int ptp_version;

static unsigned int ptp_volt_0, ptp_volt_1, ptp_volt_2, ptp_volt_3;
static unsigned int ptp_volt_4, ptp_volt_5, ptp_volt_6, ptp_volt_7;

static unsigned int ptp_init2_volt_0, ptp_init2_volt_1, ptp_init2_volt_2, ptp_init2_volt_3;
static unsigned int ptp_init2_volt_4, ptp_init2_volt_5, ptp_init2_volt_6, ptp_init2_volt_7;

static unsigned int ptp_dcvoffset = 0;
static unsigned int ptp_agevoffset = 0;

/* Extern function */
extern int mt_irq_mask_all(struct mtk_irq_mask *mask);
extern int mt_irq_mask_restore(struct mtk_irq_mask *mask);

extern unsigned int get_devinfo_with_index(unsigned int index);

extern void mt_fh_popod_save(void);
extern void mt_fh_popod_restore(void);

extern unsigned int mt_cpufreq_max_frequency_by_DVS(unsigned int num);
extern void mt_cpufreq_return_default_DVS_by_ptpod(void);

static struct hrtimer mt_ptp_timer;
struct task_struct *mt_ptp_thread = NULL;
static DECLARE_WAIT_QUEUE_HEAD(mt_ptp_timer_waiter);

static int mt_ptp_timer_flag = 0;
static int mt_ptp_period_s = 2;
static int mt_ptp_period_ns = 0;

static unsigned int ptp_trasnfer_to_volt(unsigned int value)
{
    return (((value * 625) / 100) + 700); // (700mv + n * 6.25mv)
}

enum hrtimer_restart mt_ptp_timer_func(struct hrtimer *timer)
{
    mt_ptp_timer_flag = 1; wake_up_interruptible(&mt_ptp_timer_waiter);
    return HRTIMER_NORESTART;
}

int mt_ptp_thread_handler(void *unused)
{
    do
    {
        ktime_t ktime = ktime_set(mt_ptp_period_s, mt_ptp_period_ns);

        wait_event_interruptible(mt_ptp_timer_waiter, mt_ptp_timer_flag != 0);
        mt_ptp_timer_flag = 0;

        ptp_notice("PTP_LOG: (%d) - (%d, %d, %d, %d, %d, %d, %d, %d) - (%d, %d, %d, %d, %d, %d, %d, %d)\n", \
                    mtktscpu_get_cpu_temp(), \
                    ptp_trasnfer_to_volt(DRV_Reg32(PMIC_WRAP_DVFS_WDATA0)), \
                    ptp_trasnfer_to_volt(DRV_Reg32(PMIC_WRAP_DVFS_WDATA1)), \
                    ptp_trasnfer_to_volt(DRV_Reg32(PMIC_WRAP_DVFS_WDATA2)), \
                    ptp_trasnfer_to_volt(DRV_Reg32(PMIC_WRAP_DVFS_WDATA3)), \
                    ptp_trasnfer_to_volt(DRV_Reg32(PMIC_WRAP_DVFS_WDATA4)), \
                    ptp_trasnfer_to_volt(DRV_Reg32(PMIC_WRAP_DVFS_WDATA5)), \
                    ptp_trasnfer_to_volt(DRV_Reg32(PMIC_WRAP_DVFS_WDATA6)), \
                    ptp_trasnfer_to_volt(DRV_Reg32(PMIC_WRAP_DVFS_WDATA7)), \
                    mt_cpufreq_max_frequency_by_DVS(0), \
                    mt_cpufreq_max_frequency_by_DVS(1), \
                    mt_cpufreq_max_frequency_by_DVS(2), \
                    mt_cpufreq_max_frequency_by_DVS(3), \
                    mt_cpufreq_max_frequency_by_DVS(4), \
                    mt_cpufreq_max_frequency_by_DVS(5), \
                    mt_cpufreq_max_frequency_by_DVS(6), \
                    mt_cpufreq_max_frequency_by_DVS(7));

        hrtimer_start(&mt_ptp_timer, ktime, HRTIMER_MODE_REL);
    } while (!kthread_should_stop());

    return 0;
}

unsigned int PTP_get_ptp_level(void)
{
    unsigned int hw_resv, sw_resv, ret = 0;

    hw_resv = get_devinfo_with_index(3) & 0x3;
    sw_resv = (get_devinfo_with_index(15) >> 28) & 0x7;

    if (hw_resv == 0x3)
    {
        if (sw_resv == 7)
        {
            ret = 7; // 1.0G
        }
        else if (sw_resv == 6)
        {
            ret = 6; // 1.1G
        }
        else if (sw_resv == 5)
        {
            ret = 6; // 1.1G
        }
        else if (sw_resv == 4)
        {
            ret = 6; // 1.1G
        }
        else if (sw_resv == 3)
        {
            ret = 6; // 1.1G
        }
        else if (sw_resv == 2)
        {
            ret = 6; // 1.1G
        }
        else if (sw_resv == 1)
        {
            ret = 6; // 1.1G
        }
        else
        {
            ret = 6; // 1.1G
        } 
    }
    else if (hw_resv == 0x2)
    {
        if (sw_resv == 7)
        {
            ret = 7; // 1.0G
        }
        else if (sw_resv == 6)
        {
            ret = 6; // 1.1G
        }
        else if (sw_resv == 5)
        {
            ret = 5; // 1.2G
        }
        else if (sw_resv == 4)
        {
            ret = 0; // 1.3G
        }
        else if (sw_resv == 3)
        {
            ret = 0; // 1.3G
        }
        else if (sw_resv == 2)
        {
            ret = 0; // 1.3G
        }
        else if (sw_resv == 1)
        {
            ret = 0; // 1.3G
        }
        else
        {
            ret = 0; // 1.3G
        }
    }
    else if (hw_resv == 0x1)
    {
        if (sw_resv == 7)
        {
            ret = 7; // 1.0G
        }
        else if (sw_resv == 6)
        {
            ret = 6; // 1.1G
        }
        else if (sw_resv == 5)
        {
            ret = 5; // 1.2G
        }
        else if (sw_resv == 4)
        {
            ret = 3; // 1.5G
        }
        else if (sw_resv == 3)
        {
            ret = 4; // 1.4G
        }
        else if (sw_resv == 2)
        {
            ret = 3; // 1.5G
        }
        else if (sw_resv == 1)
        {
            ret = 3; // 1.5G
        }
        else
        {
            ret = 0; // 1.3G
        }
    }
    else
    {
        if (sw_resv == 7)
        {
            ret = 7; // 1.0G
        }
        else if (sw_resv == 6)
        {
            ret = 6; // 1.1G
        }
        else if (sw_resv == 5)
        {
            ret = 5; // 1.2G
        }
        else if (sw_resv == 4)
        {
            ret = 1; // 1.7G
        }
        else if (sw_resv == 3)
        {
            ret = 4; // 1.4G
        }
        else if (sw_resv == 2)
        {
            ret = 3; // 1.5G
        }
        else if (sw_resv == 1)
        {
            ret = 2; // 1.6G
        }
        else
        {
            ret = 0; // 1.3G
        }
    }

    return ret;
}

static void PTP_Initialization_01(PTP_INIT_T* ptp_init_val)
{
    unsigned int temp_i, temp_filter, temp_value;

    // config PTP register
    ptp_write(PTP_DESCHAR,     ((((ptp_init_val->BDES)<<8) & 0xff00)  | ((ptp_init_val->MDES) & 0xff)));
    ptp_write(PTP_TEMPCHAR,    ((((ptp_init_val->VCO)<<16) & 0xff0000) | (((ptp_init_val->MTDES)<<8) & 0xff00)  | ((ptp_init_val->DVTFIXED) & 0xff)));
    ptp_write(PTP_DETCHAR,     ((((ptp_init_val->DCBDET)<<8) & 0xff00)  | ((ptp_init_val->DCMDET) & 0xff)));
    ptp_write(PTP_AGECHAR,     ((((ptp_init_val->AGEDELTA)<<8) & 0xff00)  | ((ptp_init_val->AGEM) & 0xff)));
    ptp_write(PTP_DCCONFIG,    ((ptp_init_val->DCCONFIG)));
    ptp_write(PTP_AGECONFIG,   ((ptp_init_val->AGECONFIG)));

    if (ptp_version == 1)
        ptp_write(PTP_PTPCHKSHIFT, ((ptp_init_val->PTPCHKSHIFT)));

    if (ptp_init_val->AGEM == 0x0)
    {
        ptp_write(PTP_RUNCONFIG, 0x80000000);
    }
    else
    {
        temp_value = 0x0;

        for (temp_i = 0 ; temp_i < 24 ; temp_i += 2)
        {
            temp_filter = 0x3 << temp_i;

            if (((ptp_init_val->AGECONFIG) & temp_filter) == 0x0)
            {
                temp_value |= (0x1 << temp_i);
            }
            else
            {
                temp_value |= ((ptp_init_val->AGECONFIG) & temp_filter);
            }
        }
        ptp_write(PTP_RUNCONFIG, temp_value);
    }

    ptp_write(PTP_FREQPCT30, ((((ptp_init_val->FREQPCT3) << 24)&0xff000000) | (((ptp_init_val->FREQPCT2) << 16) & 0xff0000) | (((ptp_init_val->FREQPCT1) << 8) & 0xff00)  | ((ptp_init_val->FREQPCT0) & 0xff)));
    ptp_write(PTP_FREQPCT74, ((((ptp_init_val->FREQPCT7) << 24)&0xff000000) | (((ptp_init_val->FREQPCT6) << 16) & 0xff0000) | (((ptp_init_val->FREQPCT5) << 8) & 0xff00)  | ((ptp_init_val->FREQPCT4) & 0xff)));

    ptp_write(PTP_LIMITVALS, ((((ptp_init_val->VMAX) << 24) & 0xff000000) | (((ptp_init_val->VMIN) << 16) & 0xff0000) | (((ptp_init_val->DTHI) << 8) & 0xff00)  | ((ptp_init_val->DTLO) & 0xff)));
    ptp_write(PTP_VBOOT,     (((ptp_init_val->VBOOT) & 0xff)));
    ptp_write(PTP_DETWINDOW, (((ptp_init_val->DETWINDOW) & 0xffff)));
    ptp_write(PTP_PTPCONFIG, (((ptp_init_val->DETMAX) & 0xffff)));

    // clear all pending PTP interrupt & config PTPINTEN
    ptp_write(PTP_PTPINTSTS, 0xffffffff);

    ptp_write(PTP_PTPINTEN, 0x00005f01);

    // enable PTP INIT measurement
    ptp_write(PTP_PTPEN, 0x00000001);
}

static void PTP_Initialization_02(PTP_INIT_T* ptp_init_val)
{
    unsigned int temp_i, temp_filter, temp_value;

    // config PTP register
    ptp_write(PTP_DESCHAR,     ((((ptp_init_val->BDES)<<8) & 0xff00)  | ((ptp_init_val->MDES) & 0xff)));
    ptp_write(PTP_TEMPCHAR,    ((((ptp_init_val->VCO)<<16) & 0xff0000) | (((ptp_init_val->MTDES)<<8) & 0xff00)  | ((ptp_init_val->DVTFIXED) & 0xff)));
    ptp_write(PTP_DETCHAR,     ((((ptp_init_val->DCBDET)<<8) & 0xff00)  | ((ptp_init_val->DCMDET) & 0xff)));
    ptp_write(PTP_AGECHAR,     ((((ptp_init_val->AGEDELTA)<<8) & 0xff00)  | ((ptp_init_val->AGEM) & 0xff)));
    ptp_write(PTP_DCCONFIG,    ((ptp_init_val->DCCONFIG)));
    ptp_write(PTP_AGECONFIG,   ((ptp_init_val->AGECONFIG)));

    if (ptp_version == 1)
        ptp_write(PTP_PTPCHKSHIFT, ((ptp_init_val->PTPCHKSHIFT)));

    if (ptp_init_val->AGEM == 0x0)
    {
        ptp_write(PTP_RUNCONFIG, 0x80000000);
    }
    else
    {
        temp_value = 0x0;

        for (temp_i = 0 ; temp_i < 24 ; temp_i += 2 )
        {
            temp_filter = 0x3 << temp_i;

            if( ((ptp_init_val->AGECONFIG) & temp_filter) == 0x0 )
            {
                temp_value |= (0x1 << temp_i);
            }
            else
            {
                temp_value |= ((ptp_init_val->AGECONFIG) & temp_filter);
            }
        }

        ptp_write(PTP_RUNCONFIG, temp_value);
    }

    ptp_write(PTP_FREQPCT30, ((((ptp_init_val->FREQPCT3)<<24)&0xff000000) | (((ptp_init_val->FREQPCT2)<<16)&0xff0000) | (((ptp_init_val->FREQPCT1)<<8)&0xff00)  | ((ptp_init_val->FREQPCT0) & 0xff)));
    ptp_write(PTP_FREQPCT74, ((((ptp_init_val->FREQPCT7)<<24)&0xff000000) | (((ptp_init_val->FREQPCT6)<<16)&0xff0000) | (((ptp_init_val->FREQPCT5)<<8)&0xff00)  | ((ptp_init_val->FREQPCT4) & 0xff)));

    ptp_write(PTP_LIMITVALS, ((((ptp_init_val->VMAX)<<24)&0xff000000) | (((ptp_init_val->VMIN)<<16)&0xff0000) | (((ptp_init_val->DTHI)<<8)&0xff00)  | ((ptp_init_val->DTLO) & 0xff)));
    ptp_write(PTP_VBOOT,     (((ptp_init_val->VBOOT)&0xff)));
    ptp_write(PTP_DETWINDOW, (((ptp_init_val->DETWINDOW)&0xffff)));
    ptp_write(PTP_PTPCONFIG, (((ptp_init_val->DETMAX)&0xffff)));

    // clear all pending PTP interrupt & config PTPINTEN
    ptp_write(PTP_PTPINTSTS, 0xffffffff);

    ptp_write(PTP_PTPINTEN, 0x00005f01);

    ptp_write(PTP_INIT2VALS, ((((ptp_init_val->AGEVOFFSETIN)<<16) & 0xffff0000) | ((ptp_init_val->DCVOFFSETIN) & 0xffff)));

    // enable PTP INIT measurement
    ptp_write(PTP_PTPEN, 0x00000005);
}

static void PTP_Monitor_Mode(PTP_INIT_T* ptp_init_val)
{
    unsigned int temp_i, temp_filter, temp_value;

    // config PTP register
    ptp_write(PTP_DESCHAR,     ((((ptp_init_val->BDES)<<8) & 0xff00)  | ((ptp_init_val->MDES) & 0xff)));
    ptp_write(PTP_TEMPCHAR,    ((((ptp_init_val->VCO)<<16) & 0xff0000) | (((ptp_init_val->MTDES)<<8) & 0xff00)  | ((ptp_init_val->DVTFIXED) & 0xff)));
    ptp_write(PTP_DETCHAR,     ((((ptp_init_val->DCBDET)<<8) & 0xff00)  | ((ptp_init_val->DCMDET) & 0xff)));
    ptp_write(PTP_AGECHAR,     ((((ptp_init_val->AGEDELTA)<<8) & 0xff00)  | ((ptp_init_val->AGEM) & 0xff)));
    ptp_write(PTP_DCCONFIG,    ((ptp_init_val->DCCONFIG)));
    ptp_write(PTP_AGECONFIG,   ((ptp_init_val->AGECONFIG)));

    if (ptp_version == 1)
        ptp_write(PTP_PTPCHKSHIFT, ((ptp_init_val->PTPCHKSHIFT)));

    ptp_write(PTP_TSCALCS,     ((((ptp_init_val->BTS)<<12) & 0xfff000)  | ((ptp_init_val->MTS) & 0xfff)));

    if (ptp_init_val->AGEM == 0x0)
    {
        ptp_write(PTP_RUNCONFIG, 0x80000000);
    }
    else
    {
        temp_value = 0x0;

        for (temp_i = 0 ; temp_i < 24 ; temp_i += 2 )
        {
            temp_filter = 0x3 << temp_i;

            if( ((ptp_init_val->AGECONFIG) & temp_filter) == 0x0 )
            {
                temp_value |= (0x1 << temp_i);
            }
            else
            {
                temp_value |= ((ptp_init_val->AGECONFIG) & temp_filter);
            }
        }

        ptp_write(PTP_RUNCONFIG, temp_value);
    }

    ptp_write(PTP_FREQPCT30, ((((ptp_init_val->FREQPCT3)<<24)&0xff000000) | (((ptp_init_val->FREQPCT2)<<16)&0xff0000) | (((ptp_init_val->FREQPCT1)<<8)&0xff00)  | ((ptp_init_val->FREQPCT0) & 0xff)));
    ptp_write(PTP_FREQPCT74, ((((ptp_init_val->FREQPCT7)<<24)&0xff000000) | (((ptp_init_val->FREQPCT6)<<16)&0xff0000) | (((ptp_init_val->FREQPCT5)<<8)&0xff00)  | ((ptp_init_val->FREQPCT4) & 0xff)));

    ptp_write(PTP_LIMITVALS, ((((ptp_init_val->VMAX)<<24)&0xff000000) | (((ptp_init_val->VMIN)<<16)&0xff0000) | (((ptp_init_val->DTHI)<<8)&0xff00)  | ((ptp_init_val->DTLO) & 0xff)));
    ptp_write(PTP_VBOOT,     (((ptp_init_val->VBOOT)&0xff)));
    ptp_write(PTP_DETWINDOW, (((ptp_init_val->DETWINDOW)&0xffff)));
    ptp_write(PTP_PTPCONFIG, (((ptp_init_val->DETMAX)&0xffff)));

    // clear all pending PTP interrupt & config PTPINTEN
    ptp_write(PTP_PTPINTSTS, 0xffffffff);

    ptp_write(PTP_PTPINTEN, 0x00FF0000);

    // enable PTP monitor mode
    ptp_write(PTP_PTPEN, 0x00000002);
}

unsigned int PTP_INIT_01(void)
{
    PTP_INIT_T ptp_init_value;

    ptp_data[0] = 0xffffffff;

    ptp_notice("PTP_INIT_01() start (ptp_level = 0x%x).\n", ptp_level);

    ptp_init_value.PTPINITEN = (val_0) & 0x1;
    ptp_init_value.PTPMONEN  = (val_0 >> 1) & 0x1;
    ptp_init_value.MDES      = (val_0 >> 8) & 0xff;
    ptp_init_value.BDES      = (val_0 >> 16) & 0xff;
    ptp_init_value.DCMDET    = (val_0 >> 24) & 0xff;

    ptp_init_value.DCCONFIG  = (val_1) & 0xffffff;
    ptp_init_value.DCBDET    = (val_1 >> 24) & 0xff;

    ptp_init_value.AGECONFIG = (val_2) & 0xffffff;
    ptp_init_value.AGEM      = (val_2 >> 24) & 0xff;

    ptp_init_value.AGEDELTA  = (val_3) & 0xff;
    ptp_init_value.DVTFIXED  = (val_3 >> 8) & 0xff;
    ptp_init_value.MTDES     = (val_3 >> 16) & 0xff;
    ptp_init_value.VCO       = (val_3 >> 24) & 0xff;

    ptp_init_value.FREQPCT0 = freq_0;
    ptp_init_value.FREQPCT1 = freq_1;
    ptp_init_value.FREQPCT2 = freq_2;
    ptp_init_value.FREQPCT3 = freq_3;
    ptp_init_value.FREQPCT4 = freq_4;
    ptp_init_value.FREQPCT5 = freq_5;
    ptp_init_value.FREQPCT6 = freq_6;
    ptp_init_value.FREQPCT7 = freq_7;

    ptp_init_value.DETWINDOW   = 0xa28;   // 100 us, This is the PTP Detector sampling time as represented in cycles of bclk_ck during INIT. 52 MHz
    ptp_init_value.VMAX        = 0x5D;    // 1.28125v (700mv + n * 6.25mv)
    #ifdef MTK_SDIOAUTOK_SUPPORT
    ptp_init_value.VMIN        = 0x4E;    // 1.1850v (700mv + n * 6.25mv)
    #else
    ptp_init_value.VMIN        = 0x48;    // 1.1500v (700mv + n * 6.25mv)
    #endif
    ptp_init_value.DTHI        = 0x01;    // positive
    ptp_init_value.DTLO        = 0xfe;    // negative (2s compliment)
    ptp_init_value.VBOOT       = 0x48;    // 115v  (700mv + n * 6.25mv)
    ptp_init_value.DETMAX      = 0xffff;  // This timeout value is in cycles of bclk_ck.

    if (ptp_version == 1)
        ptp_init_value.PTPCHKSHIFT = 0x67; // To avoid overflow.

    if (ptp_init_value.PTPINITEN == 0x0)
    {
        ptp_notice("PTPINITEN = 0x%x \n", ptp_init_value.PTPINITEN);
        return 1;
    }

    ptp_notice("PTPINITEN   = 0x%x\n", ptp_init_value.PTPINITEN);
    ptp_notice("PTPMONEN    = 0x%x\n", ptp_init_value.PTPMONEN);
    ptp_notice("MDES        = 0x%x\n", ptp_init_value.MDES);
    ptp_notice("BDES        = 0x%x\n", ptp_init_value.BDES);
    ptp_notice("DCMDET      = 0x%x\n", ptp_init_value.DCMDET);
    ptp_notice("DCCONFIG    = 0x%x\n", ptp_init_value.DCCONFIG);
    ptp_notice("DCBDET      = 0x%x\n", ptp_init_value.DCBDET);
    ptp_notice("AGECONFIG   = 0x%x\n", ptp_init_value.AGECONFIG);
    ptp_notice("AGEM        = 0x%x\n", ptp_init_value.AGEM);
    ptp_notice("AGEDELTA    = 0x%x\n", ptp_init_value.AGEDELTA);
    ptp_notice("DVTFIXED    = 0x%x\n", ptp_init_value.DVTFIXED);
    ptp_notice("MTDES       = 0x%x\n", ptp_init_value.MTDES);
    ptp_notice("VCO         = 0x%x\n", ptp_init_value.VCO);
    ptp_notice("FREQPCT0    = %d\n", ptp_init_value.FREQPCT0);
    ptp_notice("FREQPCT1    = %d\n", ptp_init_value.FREQPCT1);
    ptp_notice("FREQPCT2    = %d\n", ptp_init_value.FREQPCT2);
    ptp_notice("FREQPCT3    = %d\n", ptp_init_value.FREQPCT3);
    ptp_notice("FREQPCT4    = %d\n", ptp_init_value.FREQPCT4);
    ptp_notice("FREQPCT5    = %d\n", ptp_init_value.FREQPCT5);
    ptp_notice("FREQPCT6    = %d\n", ptp_init_value.FREQPCT6);
    ptp_notice("FREQPCT7    = %d\n", ptp_init_value.FREQPCT7);
    ptp_notice("PTPCHKSHIFT = %d\n", ptp_init_value.PTPCHKSHIFT);

    mt_fh_popod_save(); // disable frequency hopping (main PLL)
    mt_cpufreq_disable_by_ptpod(); // disable DVFS and set vproc = 1.15v (1 GHz)

    PTP_Initialization_01(&ptp_init_value);

    return 0;
}

unsigned int PTP_INIT_02(void)
{
    PTP_INIT_T ptp_init_value;

    ptp_data[0] = 0xffffffff;
    
    ptp_notice("PTP_INIT_02() start (ptp_level = 0x%x).\n", ptp_level);

    ptp_init_value.PTPINITEN = (val_0) & 0x1;
    ptp_init_value.PTPMONEN  = (val_0 >> 1) & 0x1;
    ptp_init_value.MDES      = (val_0 >> 8) & 0xff;
    ptp_init_value.BDES      = (val_0 >> 16) & 0xff;
    ptp_init_value.DCMDET    = (val_0 >> 24) & 0xff;

    ptp_init_value.DCCONFIG  = (val_1) & 0xffffff;
    ptp_init_value.DCBDET    = (val_1 >> 24) & 0xff;

    ptp_init_value.AGECONFIG = (val_2) & 0xffffff;
    ptp_init_value.AGEM      = (val_2 >> 24) & 0xff;

    ptp_init_value.AGEDELTA  = (val_3) & 0xff;
    ptp_init_value.DVTFIXED  = (val_3 >> 8) & 0xff;
    ptp_init_value.MTDES     = (val_3 >> 16) & 0xff;
    ptp_init_value.VCO       = (val_3 >> 24) & 0xff;

    ptp_init_value.FREQPCT0 = freq_0;
    ptp_init_value.FREQPCT1 = freq_1;
    ptp_init_value.FREQPCT2 = freq_2;
    ptp_init_value.FREQPCT3 = freq_3;
    ptp_init_value.FREQPCT4 = freq_4;
    ptp_init_value.FREQPCT5 = freq_5;
    ptp_init_value.FREQPCT6 = freq_6;
    ptp_init_value.FREQPCT7 = freq_7;

    ptp_init_value.DETWINDOW   = 0xa28;   // 100 us, This is the PTP Detector sampling time as represented in cycles of bclk_ck during INIT. 52 MHz
    ptp_init_value.VMAX        = 0x5D;    // 1.28125v (700mv + n * 6.25mv)
    #ifdef MTK_SDIOAUTOK_SUPPORT
    ptp_init_value.VMIN        = 0x4E;    // 1.1850v (700mv + n * 6.25mv)
    #else
    ptp_init_value.VMIN        = 0x48;    // 1.1500v (700mv + n * 6.25mv)
    #endif
    ptp_init_value.DTHI        = 0x01;    // positive
    ptp_init_value.DTLO        = 0xfe;    // negative (2s compliment)
    ptp_init_value.VBOOT       = 0x48;    // 115v  (700mv + n * 6.25mv)
    ptp_init_value.DETMAX      = 0xffff;  // This timeout value is in cycles of bclk_ck.

    if (ptp_version == 1)
        ptp_init_value.PTPCHKSHIFT = 0x67; // To avoid overflow.

    ptp_init_value.DCVOFFSETIN= ptp_dcvoffset;
    ptp_init_value.AGEVOFFSETIN= ptp_agevoffset;

    ptp_notice("DCVOFFSETIN = 0x%x \n", ptp_init_value.DCVOFFSETIN);
    ptp_notice("AGEVOFFSETIN = 0x%x \n", ptp_init_value.AGEVOFFSETIN);

    if (ptp_init_value.PTPINITEN == 0x0)
    {
        ptp_notice("PTPINITEN = 0x%x \n", ptp_init_value.PTPINITEN);
        return 1;
    }

    ptp_isr_info("PTPINITEN    = 0x%x\n", ptp_init_value.PTPINITEN);
    ptp_isr_info("PTPMONEN     = 0x%x\n", ptp_init_value.PTPMONEN);
    ptp_isr_info("MDES         = 0x%x\n", ptp_init_value.MDES);
    ptp_isr_info("BDES         = 0x%x\n", ptp_init_value.BDES);
    ptp_isr_info("DCMDET       = 0x%x\n", ptp_init_value.DCMDET);
    ptp_isr_info("DCCONFIG     = 0x%x\n", ptp_init_value.DCCONFIG);
    ptp_isr_info("DCBDET       = 0x%x\n", ptp_init_value.DCBDET);
    ptp_isr_info("AGECONFIG    = 0x%x\n", ptp_init_value.AGECONFIG);
    ptp_isr_info("AGEM         = 0x%x\n", ptp_init_value.AGEM);
    ptp_isr_info("AGEDELTA     = 0x%x\n", ptp_init_value.AGEDELTA);
    ptp_isr_info("DVTFIXED     = 0x%x\n", ptp_init_value.DVTFIXED);
    ptp_isr_info("MTDES        = 0x%x\n", ptp_init_value.MTDES);
    ptp_isr_info("VCO          = 0x%x\n", ptp_init_value.VCO);
    ptp_isr_info("DCVOFFSETIN  = 0x%x\n", ptp_init_value.DCVOFFSETIN);
    ptp_isr_info("AGEVOFFSETIN = 0x%x\n", ptp_init_value.AGEVOFFSETIN);
    ptp_isr_info("FREQPCT0     = %d\n", ptp_init_value.FREQPCT0);
    ptp_isr_info("FREQPCT1     = %d\n", ptp_init_value.FREQPCT1);
    ptp_isr_info("FREQPCT2     = %d\n", ptp_init_value.FREQPCT2);
    ptp_isr_info("FREQPCT3     = %d\n", ptp_init_value.FREQPCT3);
    ptp_isr_info("FREQPCT4     = %d\n", ptp_init_value.FREQPCT4);
    ptp_isr_info("FREQPCT5     = %d\n", ptp_init_value.FREQPCT5);
    ptp_isr_info("FREQPCT6     = %d\n", ptp_init_value.FREQPCT6);
    ptp_isr_info("FREQPCT7     = %d\n", ptp_init_value.FREQPCT7);
    ptp_isr_info("PTPCHKSHIFT  = %d\n", ptp_init_value.PTPCHKSHIFT);

    PTP_Initialization_02(&ptp_init_value);

    return 0;
}

unsigned int PTP_MON_MODE(void)
{
    PTP_INIT_T ptp_init_value;
    struct TS_PTPOD ts_info;

    ptp_notice("PTP_MON_MODE() start (ptp_level = 0x%x).\n", ptp_level);

    ptp_init_value.PTPINITEN    = (val_0) & 0x1;
    ptp_init_value.PTPMONEN     = (val_0 >> 1) & 0x1;
    ptp_init_value.ADC_CALI_EN  = (val_0 >> 2) & 0x1;
    ptp_init_value.MDES         = (val_0 >> 8) & 0xff;
    ptp_init_value.BDES         = (val_0 >> 16) & 0xff;
    ptp_init_value.DCMDET       = (val_0 >> 24) & 0xff;

    ptp_init_value.DCCONFIG     = (val_1) & 0xffffff;
    ptp_init_value.DCBDET       = (val_1 >> 24) & 0xff;

    ptp_init_value.AGECONFIG    = (val_2) & 0xffffff;
    ptp_init_value.AGEM         = (val_2 >> 24) & 0xff;

    ptp_init_value.AGEDELTA     = (val_3) & 0xff;
    ptp_init_value.DVTFIXED     = (val_3 >> 8) & 0xff;
    ptp_init_value.MTDES        = (val_3 >> 16) & 0xff;
    ptp_init_value.VCO          = (val_3 >> 24) & 0xff;

    get_thermal_slope_intercept(&ts_info);
    ptp_init_value.MTS = ts_info.ts_MTS; 
    ptp_init_value.BTS = ts_info.ts_BTS; 

    ptp_init_value.FREQPCT0 = freq_0;
    ptp_init_value.FREQPCT1 = freq_1;
    ptp_init_value.FREQPCT2 = freq_2;
    ptp_init_value.FREQPCT3 = freq_3;
    ptp_init_value.FREQPCT4 = freq_4;
    ptp_init_value.FREQPCT5 = freq_5;
    ptp_init_value.FREQPCT6 = freq_6;
    ptp_init_value.FREQPCT7 = freq_7;

    ptp_init_value.DETWINDOW   = 0xa28;   // 100 us, This is the PTP Detector sampling time as represented in cycles of bclk_ck during INIT. 52 MHz
    ptp_init_value.VMAX        = 0x5D;    // 1.28125v (700mv + n * 6.25mv)
    #ifdef MTK_SDIOAUTOK_SUPPORT
    ptp_init_value.VMIN        = 0x4E;    // 1.1850v (700mv + n * 6.25mv)
    #else
    ptp_init_value.VMIN        = 0x48;    // 1.1500v (700mv + n * 6.25mv)
    #endif
    ptp_init_value.DTHI        = 0x01;    // positive
    ptp_init_value.DTLO        = 0xfe;    // negative (2s compliment)
    ptp_init_value.VBOOT       = 0x48;    // 115v  (700mv + n * 6.25mv)
    ptp_init_value.DETMAX      = 0xffff;  // This timeout value is in cycles of bclk_ck.

    if (ptp_version == 1)
        ptp_init_value.PTPCHKSHIFT = 0x67; // To avoid overflow.

    if( (ptp_init_value.PTPINITEN == 0x0) || (ptp_init_value.PTPMONEN == 0x0) || (ptp_init_value.ADC_CALI_EN == 0x0) )
    {
        ptp_notice("PTPINITEN = 0x%x, PTPMONEN = 0x%x, ADC_CALI_EN = 0x%x \n", ptp_init_value.PTPINITEN, ptp_init_value.PTPMONEN, ptp_init_value.ADC_CALI_EN);
        return 1;
    }

    ptp_isr_info("PTPINITEN   = 0x%x\n", ptp_init_value.PTPINITEN);
    ptp_isr_info("PTPMONEN    = 0x%x\n", ptp_init_value.PTPMONEN);
    ptp_isr_info("MDES        = 0x%x\n", ptp_init_value.MDES);
    ptp_isr_info("BDES        = 0x%x\n", ptp_init_value.BDES);
    ptp_isr_info("DCMDET      = 0x%x\n", ptp_init_value.DCMDET);
    ptp_isr_info("DCCONFIG    = 0x%x\n", ptp_init_value.DCCONFIG);
    ptp_isr_info("DCBDET      = 0x%x\n", ptp_init_value.DCBDET);
    ptp_isr_info("AGECONFIG   = 0x%x\n", ptp_init_value.AGECONFIG);
    ptp_isr_info("AGEM        = 0x%x\n", ptp_init_value.AGEM);
    ptp_isr_info("AGEDELTA    = 0x%x\n", ptp_init_value.AGEDELTA);
    ptp_isr_info("DVTFIXED    = 0x%x\n", ptp_init_value.DVTFIXED);
    ptp_isr_info("MTDES       = 0x%x\n", ptp_init_value.MTDES);
    ptp_isr_info("VCO         = 0x%x\n", ptp_init_value.VCO);
    ptp_isr_info("MTS         = 0x%x\n", ptp_init_value.MTS);
    ptp_isr_info("BTS         = 0x%x\n", ptp_init_value.BTS);
    ptp_isr_info("FREQPCT0    = %d\n", ptp_init_value.FREQPCT0);
    ptp_isr_info("FREQPCT1    = %d\n", ptp_init_value.FREQPCT1);
    ptp_isr_info("FREQPCT2    = %d\n", ptp_init_value.FREQPCT2);
    ptp_isr_info("FREQPCT3    = %d\n", ptp_init_value.FREQPCT3);
    ptp_isr_info("FREQPCT4    = %d\n", ptp_init_value.FREQPCT4);
    ptp_isr_info("FREQPCT5    = %d\n", ptp_init_value.FREQPCT5);
    ptp_isr_info("FREQPCT6    = %d\n", ptp_init_value.FREQPCT6);
    ptp_isr_info("FREQPCT7    = %d\n", ptp_init_value.FREQPCT7);
    ptp_isr_info("PTPCHKSHIFT = %d\n", ptp_init_value.PTPCHKSHIFT);

    PTP_Monitor_Mode(&ptp_init_value);

    return 0;
}

static void PTP_set_ptp_volt(void)
{
    #if SET_PMIC_VOLT

    if (freq_0 != 0)
    {
        ptp_write(PMIC_WRAP_DVFS_ADR0, 0x0220);
        ptp_write(PMIC_WRAP_DVFS_WDATA0, ptp_volt_0);
    }

    if (freq_1 != 0)
    {
        ptp_write(PMIC_WRAP_DVFS_ADR1, 0x0220);
        ptp_write(PMIC_WRAP_DVFS_WDATA1, ptp_volt_1);
    }

    if (freq_2 != 0)
    {
        ptp_write(PMIC_WRAP_DVFS_ADR2, 0x0220);
        ptp_write(PMIC_WRAP_DVFS_WDATA2, ptp_volt_2);
    }

    if (freq_3 != 0)
    {
        ptp_write(PMIC_WRAP_DVFS_ADR3, 0x0220);
        ptp_write(PMIC_WRAP_DVFS_WDATA3, ptp_volt_3);
    }

    if (freq_4 != 0)
    {
        ptp_write(PMIC_WRAP_DVFS_ADR4, 0x0220);
        ptp_write(PMIC_WRAP_DVFS_WDATA4, ptp_volt_4);
    }

    if (freq_5 != 0) // not support
    {
        //ptp_write(PMIC_WRAP_DVFS_ADR5, 0x0220);
        //ptp_write(PMIC_WRAP_DVFS_WDATA5, ptp_volt_5);
    }

    if (freq_6 != 0) // not support
    {
        //ptp_write(PMIC_WRAP_DVFS_ADR6, 0x0220);
        //ptp_write(PMIC_WRAP_DVFS_WDATA6, ptp_volt_6);
    }

    if (freq_7 != 0) // not support
    {
        //ptp_write(PMIC_WRAP_DVFS_ADR7, 0x0220);
        //ptp_write(PMIC_WRAP_DVFS_WDATA7, ptp_volt_7);
    }

    #endif
}

void mt_ptp_reg_dump(void)
{
    ptp_isr_info("DUMP PTP_DESCHAR      = 0x%x\n", DRV_Reg32(PTP_DESCHAR));
    ptp_isr_info("DUMP PTP_TEMPCHAR     = 0x%x\n", DRV_Reg32(PTP_TEMPCHAR));
    ptp_isr_info("DUMP PTP_DETCHAR      = 0x%x\n", DRV_Reg32(PTP_DETCHAR));
    ptp_isr_info("DUMP PTP_AGECHAR      = 0x%x\n", DRV_Reg32(PTP_AGECHAR));
    ptp_isr_info("DUMP PTP_DCCONFIG     = 0x%x\n", DRV_Reg32(PTP_DCCONFIG));
    ptp_isr_info("DUMP PTP_AGECONFIG    = 0x%x\n", DRV_Reg32(PTP_AGECONFIG));
    ptp_isr_info("DUMP PTP_FREQPCT30    = 0x%x\n", DRV_Reg32(PTP_FREQPCT30));
    ptp_isr_info("DUMP PTP_FREQPCT74    = 0x%x\n", DRV_Reg32(PTP_FREQPCT74));
    ptp_isr_info("DUMP PTP_LIMITVALS    = 0x%x\n", DRV_Reg32(PTP_LIMITVALS));
    ptp_isr_info("DUMP PTP_VBOOT        = 0x%x\n", DRV_Reg32(PTP_VBOOT));
    ptp_isr_info("DUMP PTP_DETWINDOW    = 0x%x\n", DRV_Reg32(PTP_DETWINDOW));
    ptp_isr_info("DUMP PTP_PTPCONFIG    = 0x%x\n", DRV_Reg32(PTP_PTPCONFIG));
    ptp_isr_info("DUMP PTP_TSCALCS      = 0x%x\n", DRV_Reg32(PTP_TSCALCS));
    ptp_isr_info("DUMP PTP_RUNCONFIG    = 0x%x\n", DRV_Reg32(PTP_RUNCONFIG));
    ptp_isr_info("DUMP PTP_PTPEN        = 0x%x\n", DRV_Reg32(PTP_PTPEN));
    ptp_isr_info("DUMP PTP_INIT2VALS    = 0x%x\n", DRV_Reg32(PTP_INIT2VALS));
    ptp_isr_info("DUMP PTP_DCVALUES     = 0x%x\n", DRV_Reg32(PTP_DCVALUES));
    ptp_isr_info("DUMP PTP_AGEVALUES    = 0x%x\n", DRV_Reg32(PTP_AGEVALUES));
    ptp_isr_info("DUMP PTP_VOP30        = 0x%x\n", DRV_Reg32(PTP_VOP30));
    ptp_isr_info("DUMP PTP_VOP74        = 0x%x\n", DRV_Reg32(PTP_VOP74));
    ptp_isr_info("DUMP PTP_TEMP         = 0x%x\n", DRV_Reg32(PTP_TEMP));
    ptp_isr_info("DUMP PTP_PTPINTSTS    = 0x%x\n", DRV_Reg32(PTP_PTPINTSTS));
    ptp_isr_info("DUMP PTP_PTPINTSTSRAW = 0x%x\n", DRV_Reg32(PTP_PTPINTSTSRAW));
    ptp_isr_info("DUMP PTP_PTPINTEN     = 0x%x\n", DRV_Reg32(PTP_PTPINTEN));
    ptp_isr_info("DUMP PTP_PTPCHKSHIFT  = 0x%x\n", DRV_Reg32(PTP_PTPCHKSHIFT));
    ptp_isr_info("DUMP PTP_SMSTATE0     = 0x%x\n", DRV_Reg32(PTP_SMSTATE0));
    ptp_isr_info("DUMP PTP_SMSTATE1     = 0x%x\n", DRV_Reg32(PTP_SMSTATE1));

    ptp_isr_info("DUMP PTP_TEMPMONCTL0          = 0x%x\n", DRV_Reg32(PTP_TEMPMONCTL0));
    ptp_isr_info("DUMP PTP_TEMPMONCTL1          = 0x%x\n", DRV_Reg32(PTP_TEMPMONCTL1));
    ptp_isr_info("DUMP PTP_TEMPMONCTL2          = 0x%x\n", DRV_Reg32(PTP_TEMPMONCTL2));
    ptp_isr_info("DUMP PTP_TEMPMONINT           = 0x%x\n", DRV_Reg32(PTP_TEMPMONINT));
    ptp_isr_info("DUMP PTP_TEMPMONINTSTS        = 0x%x\n", DRV_Reg32(PTP_TEMPMONINTSTS));
    ptp_isr_info("DUMP PTP_TEMPMONIDET0         = 0x%x\n", DRV_Reg32(PTP_TEMPMONIDET0));
    ptp_isr_info("DUMP PTP_TEMPMONIDET1         = 0x%x\n", DRV_Reg32(PTP_TEMPMONIDET1));
    ptp_isr_info("DUMP PTP_TEMPMONIDET2         = 0x%x\n", DRV_Reg32(PTP_TEMPMONIDET2));
    ptp_isr_info("DUMP PTP_TEMPH2NTHRE          = 0x%x\n", DRV_Reg32(PTP_TEMPH2NTHRE));
    ptp_isr_info("DUMP PTP_TEMPHTHRE            = 0x%x\n", DRV_Reg32(PTP_TEMPHTHRE));
    ptp_isr_info("DUMP PTP_TEMPCTHRE            = 0x%x\n", DRV_Reg32(PTP_TEMPCTHRE));
    ptp_isr_info("DUMP PTP_TEMPOFFSETH          = 0x%x\n", DRV_Reg32(PTP_TEMPOFFSETH));
    ptp_isr_info("DUMP PTP_TEMPOFFSETL          = 0x%x\n", DRV_Reg32(PTP_TEMPOFFSETL));
    ptp_isr_info("DUMP PTP_TEMPMSRCTL0          = 0x%x\n", DRV_Reg32(PTP_TEMPMSRCTL0));
    ptp_isr_info("DUMP PTP_TEMPMSRCTL1          = 0x%x\n", DRV_Reg32(PTP_TEMPMSRCTL1));
    ptp_isr_info("DUMP PTP_TEMPAHBPOLL          = 0x%x\n", DRV_Reg32(PTP_TEMPAHBPOLL));
    ptp_isr_info("DUMP PTP_TEMPAHBTO            = 0x%x\n", DRV_Reg32(PTP_TEMPAHBTO));
    ptp_isr_info("DUMP PTP_TEMPADCPNP0          = 0x%x\n", DRV_Reg32(PTP_TEMPADCPNP0));
    ptp_isr_info("DUMP PTP_TEMPADCPNP1          = 0x%x\n", DRV_Reg32(PTP_TEMPADCPNP1));
    ptp_isr_info("DUMP PTP_TEMPADCPNP2          = 0x%x\n", DRV_Reg32(PTP_TEMPADCPNP2));
    ptp_isr_info("DUMP PTP_TEMPADCMUX           = 0x%x\n", DRV_Reg32(PTP_TEMPADCMUX));
    ptp_isr_info("DUMP PTP_TEMPADCEXT           = 0x%x\n", DRV_Reg32(PTP_TEMPADCEXT));
    ptp_isr_info("DUMP PTP_TEMPADCEXT1          = 0x%x\n", DRV_Reg32(PTP_TEMPADCEXT1));
    ptp_isr_info("DUMP PTP_TEMPADCEN            = 0x%x\n", DRV_Reg32(PTP_TEMPADCEN));
    ptp_isr_info("DUMP PTP_TEMPPNPMUXADDR       = 0x%x\n", DRV_Reg32(PTP_TEMPPNPMUXADDR));
    ptp_isr_info("DUMP PTP_TEMPADCMUXADDR       = 0x%x\n", DRV_Reg32(PTP_TEMPADCMUXADDR));
    ptp_isr_info("DUMP PTP_TEMPADCEXTADDR       = 0x%x\n", DRV_Reg32(PTP_TEMPADCEXTADDR));
    ptp_isr_info("DUMP PTP_TEMPADCEXT1ADDR      = 0x%x\n", DRV_Reg32(PTP_TEMPADCEXT1ADDR));
    ptp_isr_info("DUMP PTP_TEMPADCENADDR        = 0x%x\n", DRV_Reg32(PTP_TEMPADCENADDR));
    ptp_isr_info("DUMP PTP_TEMPADCVALIDADDR     = 0x%x\n", DRV_Reg32(PTP_TEMPADCVALIDADDR));
    ptp_isr_info("DUMP PTP_TEMPADCVOLTADDR      = 0x%x\n", DRV_Reg32(PTP_TEMPADCVOLTADDR));
    ptp_isr_info("DUMP PTP_TEMPRDCTRL           = 0x%x\n", DRV_Reg32(PTP_TEMPRDCTRL));
    ptp_isr_info("DUMP PTP_TEMPADCVALIDMASK     = 0x%x\n", DRV_Reg32(PTP_TEMPADCVALIDMASK));
    ptp_isr_info("DUMP PTP_TEMPADCVOLTAGESHIFT  = 0x%x\n", DRV_Reg32(PTP_TEMPADCVOLTAGESHIFT));
    ptp_isr_info("DUMP PTP_TEMPADCWRITECTRL     = 0x%x\n", DRV_Reg32(PTP_TEMPADCWRITECTRL));
    ptp_isr_info("DUMP PTP_TEMPMSR0             = 0x%x\n", DRV_Reg32(PTP_TEMPMSR0));
    ptp_isr_info("DUMP PTP_TEMPMSR1             = 0x%x\n", DRV_Reg32(PTP_TEMPMSR1));
    ptp_isr_info("DUMP PTP_TEMPMSR2             = 0x%x\n", DRV_Reg32(PTP_TEMPMSR2));
    ptp_isr_info("DUMP PTP_TEMPIMMD0            = 0x%x\n", DRV_Reg32(PTP_TEMPIMMD0));
    ptp_isr_info("DUMP PTP_TEMPIMMD1            = 0x%x\n", DRV_Reg32(PTP_TEMPIMMD1));
    ptp_isr_info("DUMP PTP_TEMPIMMD2            = 0x%x\n", DRV_Reg32(PTP_TEMPIMMD2));
    ptp_isr_info("DUMP PTP_TEMPPROTCTL          = 0x%x\n", DRV_Reg32(PTP_TEMPPROTCTL));
    ptp_isr_info("DUMP PTP_TEMPPROTTA           = 0x%x\n", DRV_Reg32(PTP_TEMPPROTTA));
    ptp_isr_info("DUMP PTP_TEMPPROTTB           = 0x%x\n", DRV_Reg32(PTP_TEMPPROTTB));
    ptp_isr_info("DUMP PTP_TEMPPROTTC           = 0x%x\n", DRV_Reg32(PTP_TEMPPROTTC));
    ptp_isr_info("DUMP PTP_TEMPSPARE0           = 0x%x\n", DRV_Reg32(PTP_TEMPSPARE0));
}

irqreturn_t mt_ptp_isr(int irq, void *dev_id)
{
    unsigned int PTPINTSTS, temp, temp_0, temp_ptpen;

    #if 0
    mt_ptp_reg_dump();
    #endif

    PTPINTSTS = ptp_read(PTP_PTPINTSTS);
    temp_ptpen = ptp_read(PTP_PTPEN);

    ptp_isr_info("PTPINTSTS = 0x%x\n", PTPINTSTS);
    ptp_isr_info("PTP_PTPEN = 0x%x\n", temp_ptpen);

    ptp_data[1] = ptp_read(0xf100b240);
    ptp_data[2] = ptp_read(0xf100b27c);

    ptp_isr_info("*(0x1100b240) = 0x%x\n", ptp_data[1]);
    ptp_isr_info("*(0x1100b27c) = 0x%x\n", ptp_data[2]);

    ptp_data[0] = 0;

    if (PTPINTSTS == 0x1) // PTP init1 or init2
    {
        if ((temp_ptpen & 0x7) == 0x1) // PTP init1
        {
            // Read & store 16 bit values DCVALUES.DCVOFFSET and AGEVALUES.AGEVOFFSET for later use in INIT2 procedure
            ptp_dcvoffset = ~(ptp_read(PTP_DCVALUES) & 0xffff) + 1;  // hw bug, workaround
            ptp_agevoffset = ptp_read(PTP_AGEVALUES) & 0xffff;

            // Set PTPEN.PTPINITEN/PTPEN.PTPINIT2EN = 0x0 & Clear PTP INIT interrupt PTPINTSTS = 0x00000001
            ptp_write(PTP_PTPEN, 0x0);
            ptp_write(PTP_PTPINTSTS, 0x1);

            mt_cpufreq_enable_by_ptpod(); // enable DVFS
            mt_fh_popod_restore(); // enable frequency hopping (main PLL)

            PTP_INIT_02();
        }
        else if ((temp_ptpen & 0x7) == 0x5) // PTP init2
        {
            temp = ptp_read(PTP_VOP30); // read ptp_volt_0 ~ ptp_volt_3
            ptp_volt_0 = temp & 0xff;
            ptp_volt_1 = (temp>>8) & 0xff;
            ptp_volt_2 = (temp>>16) & 0xff;
            ptp_volt_3 = (temp>>24) & 0xff;

            temp = ptp_read(PTP_VOP74); // read ptp_volt_4 ~ ptp_volt_7
            ptp_volt_4 = temp & 0xff;
            ptp_volt_5 = (temp>>8) & 0xff;
            ptp_volt_6 = (temp>>16) & 0xff;
            ptp_volt_7 = (temp>>24) & 0xff;

            // save ptp_init2_volt_0 ~ ptp_init2_volt_7
            ptp_init2_volt_0 = ptp_volt_0;
            ptp_init2_volt_1 = ptp_volt_1;
            ptp_init2_volt_2 = ptp_volt_2;
            ptp_init2_volt_3 = ptp_volt_3;
            ptp_init2_volt_4 = ptp_volt_4;
            ptp_init2_volt_5 = ptp_volt_5;
            ptp_init2_volt_6 = ptp_volt_6;
            ptp_init2_volt_7 = ptp_volt_7;

            ptp_isr_info("ptp_volt_0 = 0x%x\n", ptp_volt_0);
            ptp_isr_info("ptp_volt_1 = 0x%x\n", ptp_volt_1);
            ptp_isr_info("ptp_volt_2 = 0x%x\n", ptp_volt_2);
            ptp_isr_info("ptp_volt_3 = 0x%x\n", ptp_volt_3);
            ptp_isr_info("ptp_volt_4 = 0x%x\n", ptp_volt_4);
            ptp_isr_info("ptp_volt_5 = 0x%x\n", ptp_volt_5);
            ptp_isr_info("ptp_volt_6 = 0x%x\n", ptp_volt_6);
            ptp_isr_info("ptp_volt_7 = 0x%x\n", ptp_volt_7);
            ptp_isr_info("ptp_level  = 0x%x\n", ptp_level);

            PTP_set_ptp_volt();

            // Set PTPEN.PTPINITEN/PTPEN.PTPINIT2EN = 0x0 & Clear PTP INIT interrupt PTPINTSTS = 0x00000001
            ptp_write(PTP_PTPEN, 0x0);
            ptp_write(PTP_PTPINTSTS, 0x1);
            PTP_MON_MODE();
        }
        else // error : init1 or init2 , but enable setting is wrong.
        {
            ptp_isr_info("====================================================\n");
            ptp_isr_info("PTP error_0 (0x%x) : PTPINTSTS = 0x%x\n", temp_ptpen, PTPINTSTS);
            ptp_isr_info("PTP_SMSTATE0 (0x%x) = 0x%x\n", PTP_SMSTATE0, ptp_read(PTP_SMSTATE0));
            ptp_isr_info("PTP_SMSTATE1 (0x%x) = 0x%x\n", PTP_SMSTATE1, ptp_read(PTP_SMSTATE1));
            ptp_isr_info("====================================================\n");

            // disable PTP
            ptp_write(PTP_PTPEN, 0x0);

            // Clear PTP interrupt PTPINTSTS
            ptp_write(PTP_PTPINTSTS, 0x00ffffff);

            // restore default DVFS table (PMIC)
            mt_cpufreq_return_default_DVS_by_ptpod();
        }
    }
    else if ((PTPINTSTS & 0x00ff0000) != 0x0)  // PTP Monitor mode
    {
        // check if thermal sensor init completed?
        temp_0 = (ptp_read(PTP_TEMP) & 0xff);

        if ((temp_0 > 0x4b) && (temp_0 < 0xd3))
        {
            ptp_isr_info("thermal sensor init has not been completed. (temp_0 = 0x%x)\n", temp_0);
        }
        else
        {
            temp = ptp_read(PTP_VOP30); // read ptp_volt_0 ~ ptp_volt_3
            ptp_volt_0 = temp & 0xff;
            ptp_volt_1 = (temp>>8) & 0xff;
            ptp_volt_2 = (temp>>16) & 0xff;
            ptp_volt_3 = (temp>>24) & 0xff;

            temp = ptp_read(PTP_VOP74); // read ptp_volt_3 ~ ptp_volt_7
            ptp_volt_4 = temp & 0xff;
            ptp_volt_5 = (temp>>8) & 0xff;
            ptp_volt_6 = (temp>>16) & 0xff;
            ptp_volt_7 = (temp>>24) & 0xff;

            ptp_isr_info("ptp_volt_0 = 0x%x\n", ptp_volt_0);
            ptp_isr_info("ptp_volt_1 = 0x%x\n", ptp_volt_1);
            ptp_isr_info("ptp_volt_2 = 0x%x\n", ptp_volt_2);
            ptp_isr_info("ptp_volt_3 = 0x%x\n", ptp_volt_3);
            ptp_isr_info("ptp_volt_4 = 0x%x\n", ptp_volt_4);
            ptp_isr_info("ptp_volt_5 = 0x%x\n", ptp_volt_5);
            ptp_isr_info("ptp_volt_6 = 0x%x\n", ptp_volt_6);
            ptp_isr_info("ptp_volt_7 = 0x%x\n", ptp_volt_7);
            ptp_isr_info("ptp_level = 0x%x\n", ptp_level);
            ptp_isr_info("ISR : TEMPSPARE1 = 0x%x\n", ptp_read(TEMPSPARE1));

            PTP_set_ptp_volt();
        }

        // Clear PTP INIT interrupt PTPINTSTS = 0x00ff0000
        ptp_write(PTP_PTPINTSTS, 0x00ff0000);
    }
    else // PTP error handler
    {
        if (((temp_ptpen & 0x7) == 0x1) || ((temp_ptpen & 0x7) == 0x5)) // init 1  || init 2 error handler
        {
            ptp_isr_info("====================================================\n");
            ptp_isr_info("PTP error_1 error_2 (0x%x) : PTPINTSTS = 0x%x\n", temp_ptpen, PTPINTSTS);
            ptp_isr_info("PTP_SMSTATE0 (0x%x) = 0x%x\n", PTP_SMSTATE0, ptp_read(PTP_SMSTATE0));
            ptp_isr_info("PTP_SMSTATE1 (0x%x) = 0x%x\n", PTP_SMSTATE1, ptp_read(PTP_SMSTATE1));
            ptp_isr_info("====================================================\n");

            // disable PTP
            ptp_write(PTP_PTPEN, 0x0);

            // Clear PTP interrupt PTPINTSTS
            ptp_write(PTP_PTPINTSTS, 0x00ffffff);

            // restore default DVFS table (PMIC)
            mt_cpufreq_return_default_DVS_by_ptpod();
        }
        else // PTP Monitor mode error handler
        {
            ptp_isr_info("====================================================\n");
            ptp_isr_info("PTP error_m (0x%x) : PTPINTSTS = 0x%x\n", temp_ptpen, PTPINTSTS);
            ptp_isr_info("PTP_SMSTATE0 (0x%x) = 0x%x\n", PTP_SMSTATE0, ptp_read(PTP_SMSTATE0));
            ptp_isr_info("PTP_SMSTATE1 (0x%x) = 0x%x\n", PTP_SMSTATE1, ptp_read(PTP_SMSTATE1));
            ptp_isr_info("PTP_TEMP (0x%x) = 0x%x\n", PTP_TEMP, ptp_read(PTP_TEMP) );
            ptp_isr_info("PTP_TEMPMSR0 (0x%x) = 0x%x\n", PTP_TEMPMSR0, ptp_read(PTP_TEMPMSR0));
            ptp_isr_info("PTP_TEMPMSR1 (0x%x) = 0x%x\n", PTP_TEMPMSR1, ptp_read(PTP_TEMPMSR1));
            ptp_isr_info("PTP_TEMPMSR2 (0x%x) = 0x%x\n", PTP_TEMPMSR2, ptp_read(PTP_TEMPMSR2));
            ptp_isr_info("PTP_TEMPMONCTL0 (0x%x) = 0x%x\n", PTP_TEMPMONCTL0, ptp_read(PTP_TEMPMONCTL0));
            ptp_isr_info("PTP_TEMPMSRCTL1 (0x%x) = 0x%x\n", PTP_TEMPMSRCTL1, ptp_read(PTP_TEMPMSRCTL1));
            ptp_isr_info("====================================================\n");

            // disable PTP
            ptp_write(PTP_PTPEN, 0x0);

            // Clear PTP interrupt PTPINTSTS
            ptp_write(PTP_PTPINTSTS, 0x00ffffff);

            // set init2 value to DVFS table (PMIC)
            ptp_volt_0 = ptp_init2_volt_0;
            ptp_volt_1 = ptp_init2_volt_1;
            ptp_volt_2 = ptp_init2_volt_2;
            ptp_volt_3 = ptp_init2_volt_3;
            ptp_volt_4 = ptp_init2_volt_4;
            ptp_volt_5 = ptp_init2_volt_5;
            ptp_volt_6 = ptp_init2_volt_6;
            ptp_volt_7 = ptp_init2_volt_7;
            PTP_set_ptp_volt();
        }
    }

    return IRQ_HANDLED;
}

unsigned int PTP_INIT_01_API(void)
{
    /* only for CPU stress */

    PTP_INIT_T ptp_init_value;

    unsigned int ptp_counter = 0;

    ptp_notice("PTP_INIT_01_API() start.\n");

    ptp_data[0] = 0xffffffff;
    ptp_data[1] = 0xffffffff;
    ptp_data[2] = 0xffffffff;

    // disable PTP
    ptp_write(PTP_PTPEN, 0x0);

    ptp_init_value.PTPINITEN = (val_0) & 0x1;
    ptp_init_value.PTPMONEN  = (val_0 >> 1) & 0x1;
    ptp_init_value.MDES      = (val_0 >> 8) & 0xff;
    ptp_init_value.BDES      = (val_0 >> 16) & 0xff;
    ptp_init_value.DCMDET    = (val_0 >> 24) & 0xff;

    ptp_init_value.DCCONFIG  = (val_1) & 0xffffff;
    ptp_init_value.DCBDET    = (val_1 >> 24) & 0xff;

    ptp_init_value.AGECONFIG = (val_2) & 0xffffff;
    ptp_init_value.AGEM      = (val_2 >> 24) & 0xff;

    //ptp_init_value.AGEDELTA = (val_3) & 0xff;
    ptp_init_value.AGEDELTA  = 0x88;
    ptp_init_value.DVTFIXED  = (val_3 >> 8) & 0xff;
    ptp_init_value.MTDES     = (val_3 >> 16) & 0xff;
    ptp_init_value.VCO       = (val_3 >> 24) & 0xff;

    // Get DVFS frequency table
    freq_0 = (u8)(mt_cpufreq_max_frequency_by_DVS(0) / 12000); // max freq 1200 x 100%
    freq_1 = (u8)(mt_cpufreq_max_frequency_by_DVS(1) / 12000); // 1000
    freq_2 = (u8)(mt_cpufreq_max_frequency_by_DVS(2) / 12000); // 715
    freq_3 = (u8)(mt_cpufreq_max_frequency_by_DVS(3) / 12000); // 419
    freq_4 = (u8)(mt_cpufreq_max_frequency_by_DVS(4) / 12000);
    freq_5 = (u8)(mt_cpufreq_max_frequency_by_DVS(5) / 12000);
    freq_6 = (u8)(mt_cpufreq_max_frequency_by_DVS(6) / 12000);
    freq_7 = (u8)(mt_cpufreq_max_frequency_by_DVS(7) / 12000);

    ptp_init_value.FREQPCT0 = freq_0;
    ptp_init_value.FREQPCT1 = freq_1;
    ptp_init_value.FREQPCT2 = freq_2;
    ptp_init_value.FREQPCT3 = freq_3;
    ptp_init_value.FREQPCT4 = freq_4;
    ptp_init_value.FREQPCT5 = freq_5;
    ptp_init_value.FREQPCT6 = freq_6;
    ptp_init_value.FREQPCT7 = freq_7;

    ptp_init_value.DETWINDOW   = 0xa28;   // 100 us, This is the PTP Detector sampling time as represented in cycles of bclk_ck during INIT. 52 MHz
    ptp_init_value.VMAX        = 0x5D;    // 1.28125v (700mv + n * 6.25mv)
    #ifdef MTK_SDIOAUTOK_SUPPORT
    ptp_init_value.VMIN        = 0x4E;    // 1.1850v (700mv + n * 6.25mv)
    #else
    ptp_init_value.VMIN        = 0x48;    // 1.1500v (700mv + n * 6.25mv)
    #endif
    ptp_init_value.DTHI        = 0x01;    // positive
    ptp_init_value.DTLO        = 0xfe;    // negative (2s compliment)
    ptp_init_value.VBOOT       = 0x48;    // 115v  (700mv + n * 6.25mv)
    ptp_init_value.DETMAX      = 0xffff;  // This timeout value is in cycles of bclk_ck.

    if (ptp_version == 1)
        ptp_init_value.PTPCHKSHIFT = 0x67; // To avoid overflow.

    if (ptp_init_value.PTPINITEN == 0x0)
    {
        ptp_notice("PTPINITEN = 0x%x \n", ptp_init_value.PTPINITEN);
        return 0;
    }

    #if 0
    // set PTP IRQ
    ret = request_irq(MT_PTP_FSM_IRQ_ID, mt_ptp_isr, IRQF_TRIGGER_LOW, "ptp", NULL);
    if (ret)
    {
        ptp_notice("PTP IRQ register failed (%d)\n", ret);
        WARN_ON(1);
    }
    ptp_notice("Set PTP IRQ OK.\n");
    #endif

    PTP_Initialization_01(&ptp_init_value);

    while(1)
    {
        ptp_counter++;

        if (ptp_counter >= 0xffffff)
        {
            ptp_notice("ptp_counter = 0x%x \n", ptp_counter);
            return 0;
        }

        if (ptp_data[0] == 0)
        {
            break;
        }
    }

    return ((unsigned int)(&ptp_data[1]));
}

#if EN_PTP_OD
static int ptp_probe(struct platform_device *pdev)
{
    int ret;
    u32 test_mode = 0;

    #if 0
    u32 rdata = 0;
    #endif

    #if 0
    if (pwrap_read((u32)RTC_AL_DOM, &rdata) == 0)
    {
        rdata = (rdata >> 8) & 0x7;
        ptp_notice("rdata = 0x%x \n", rdata);

        if (rdata == 1)
        {
            val_0 = val_0_1;
            val_1 = val_1_1;
            val_2 = val_2_1;
            val_3 = val_3_1;

            test_mode = 1;
        }
        else if (rdata == 2)
        {
            val_0 = val_0_2;
            val_1 = val_1_2;
            val_2 = val_2_2;
            val_3 = val_3_2;

            test_mode = 1;
        }
        else
        {
            /* don't care */
            test_mode = 0;
        }
    }
    #endif

    #if PTP_GET_REAL_VAL
    if (!test_mode)
    {
        val_0 = get_devinfo_with_index(16);
        val_1 = get_devinfo_with_index(17);
        val_2 = get_devinfo_with_index(18);
        val_3 = get_devinfo_with_index(19);

        ptp_version = (get_devinfo_with_index(15) >> 24) & 0x1;
    }
    else
    {
        ptp_version = 1;
    }
    #endif

    if ((val_0 & 0x1) == 0x0)
    {
        ptp_notice("PTPINITEN = 0x%x \n", (val_0 & 0x1));
        return 0;
    }

    // set PTP IRQ
    ret = request_irq(MT_PTP_FSM_IRQ_ID, mt_ptp_isr, IRQF_TRIGGER_LOW, "ptp", NULL);
    if (ret) 
    {
        ptp_notice("PTP IRQ register failed (%d)\n", ret);
        WARN_ON(1);
    }

    ptp_notice("Set PTP IRQ OK.\n");

    // get DVFS frequency table
    freq_0 = (unsigned char)(mt_cpufreq_max_frequency_by_DVS(0) / 12000);   // max freq 1200 x 100%
    freq_1 = (unsigned char)(mt_cpufreq_max_frequency_by_DVS(1) / 12000);
    freq_2 = (unsigned char)(mt_cpufreq_max_frequency_by_DVS(2) / 12000);
    freq_3 = (unsigned char)(mt_cpufreq_max_frequency_by_DVS(3) / 12000);
    freq_4 = (unsigned char)(mt_cpufreq_max_frequency_by_DVS(4) / 12000);
    freq_5 = (unsigned char)(mt_cpufreq_max_frequency_by_DVS(5) / 12000);
    freq_6 = (unsigned char)(mt_cpufreq_max_frequency_by_DVS(6) / 12000);
    freq_7 = (unsigned char)(mt_cpufreq_max_frequency_by_DVS(7) / 12000);

    ptp_level = PTP_get_ptp_level();

    PTP_INIT_01();    

    return 0;
}

static int ptp_resume(struct platform_device *pdev)
{
    PTP_INIT_02();
    return 0;
}

static struct platform_driver ptp_driver = {
    .remove     = NULL,
    .shutdown   = NULL,
    .probe      = ptp_probe,
    .suspend	= NULL,
    .resume		= ptp_resume,
    .driver     = {
        .name = "mt-ptp",
    },
};

int ptp_opp_num(void)
{
    int num = 0;

    while (1)
    {
        if (mt_cpufreq_max_frequency_by_DVS(num) == 0) 
            break;
        num++;
    }

    return num;
}
EXPORT_SYMBOL(ptp_opp_num);

void ptp_opp_freq(unsigned int *freq)
{
    int i = 0;

    while (1)
    {
        if (mt_cpufreq_max_frequency_by_DVS(i) == 0) 
            break;
        freq[i] = mt_cpufreq_max_frequency_by_DVS(i);
        i++;
    }
}
EXPORT_SYMBOL(ptp_opp_freq);

void ptp_opp_status(unsigned int *temp, unsigned int *volt)
{
    int i = 0;

    *temp = mtktscpu_get_cpu_temp();

    while (1)
    {
        if (mt_cpufreq_max_frequency_by_DVS(i) == 0) 
            break;
        volt[i] = ptp_trasnfer_to_volt(DRV_Reg32((PMIC_WRAP_DVFS_WDATA0 + (i * 8))));
        i++;
    }
}
EXPORT_SYMBOL(ptp_opp_status);

void ptp_disable(void)
{
    unsigned long flags;

    // Mask ARM i bit
    local_irq_save(flags);
    
    // disable PTP
    ptp_write(PTP_PTPEN, 0x0);

    // Clear PTP interrupt PTPINTSTS
    ptp_write(PTP_PTPINTSTS, 0x00ffffff);

    // restore default DVFS table (PMIC)
    mt_cpufreq_return_default_DVS_by_ptpod();

    ptp_notice("Disable PTP-OD done.\n");

    // Un-Mask ARM i bit
    local_irq_restore(flags);
}

/***************************
* show current PTP stauts
****************************/
static int ptp_debug_read(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
    int len = 0;
    char *p = buf;

    if (ptp_read(PTP_PTPEN) != 0)
        p += sprintf(p, "PTP enabled (ptp_level = 0x%x)\n", ptp_level);
    else
        p += sprintf(p, "PTP disabled (ptp_level = 0x%x)\n", ptp_level);

    len = p - buf;
    return len;
}

/************************************
* set PTP stauts by sysfs interface
*************************************/
static ssize_t ptp_debug_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
    int enabled = 0;

    if (sscanf(buffer, "%d", &enabled) == 1)
    {
        if (enabled == 0)
        {            
            ptp_disable(); // Disable PTP and restore default DVFS table (PMIC)
        }
        else
        {
            ptp_notice("bad argument_0!! argument should be \"0\"\n");
        }
    }
    else
    {
        ptp_notice("bad argument_1!! argument should be \"0\"\n");
    }

    return count;
}

/***************************
* show current PTP data
****************************/
static int ptp_dump_read(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
    int len = 0;
    char *p = buf;

    p += sprintf(p, "(0x%x, 0x%x, 0x%x, 0x%x)\n", val_0, val_1, val_2, val_3);

    len = p - buf;
    return len;
}

/***********************
* show current voltage
************************/
static int ptp_cur_volt_read(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
    int len = 0;
    char *p = buf;
    u32 rdata = 0;

    if (pwrap_read((u32)0x0220, &rdata) == 0)
    {
        p += sprintf(p, "current voltage: (%d)\n", ptp_trasnfer_to_volt(rdata));
    }
    else
    {
        p += sprintf(p, "read current voltage fail\n");
    }

    len = p - buf;
    return len;
}

/**************************
* show current PTP status
***************************/
static int ptp_status_read(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
    int len = 0;
    char *p = buf;

    p += sprintf(p, "PTP_LOG: (%d) - (%d, %d, %d, %d, %d, %d, %d, %d) - (%d, %d, %d, %d, %d, %d, %d, %d)\n", \
                    mtktscpu_get_cpu_temp(), \
                    ptp_trasnfer_to_volt(DRV_Reg32(PMIC_WRAP_DVFS_WDATA0)), \
                    ptp_trasnfer_to_volt(DRV_Reg32(PMIC_WRAP_DVFS_WDATA1)), \
                    ptp_trasnfer_to_volt(DRV_Reg32(PMIC_WRAP_DVFS_WDATA2)), \
                    ptp_trasnfer_to_volt(DRV_Reg32(PMIC_WRAP_DVFS_WDATA3)), \
                    ptp_trasnfer_to_volt(DRV_Reg32(PMIC_WRAP_DVFS_WDATA4)), \
                    ptp_trasnfer_to_volt(DRV_Reg32(PMIC_WRAP_DVFS_WDATA5)), \
                    ptp_trasnfer_to_volt(DRV_Reg32(PMIC_WRAP_DVFS_WDATA6)), \
                    ptp_trasnfer_to_volt(DRV_Reg32(PMIC_WRAP_DVFS_WDATA7)), \
                    mt_cpufreq_max_frequency_by_DVS(0), \
                    mt_cpufreq_max_frequency_by_DVS(1), \
                    mt_cpufreq_max_frequency_by_DVS(2), \
                    mt_cpufreq_max_frequency_by_DVS(3), \
                    mt_cpufreq_max_frequency_by_DVS(4), \
                    mt_cpufreq_max_frequency_by_DVS(5), \
                    mt_cpufreq_max_frequency_by_DVS(6), \
                    mt_cpufreq_max_frequency_by_DVS(7));

    len = p - buf;
    return len;
}

/***************************************
* set PTP log enable by sysfs interface
****************************************/
static ssize_t ptp_log_en_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
    int enabled = 0;
    ktime_t ktime = ktime_set(mt_ptp_period_s, mt_ptp_period_ns);

    if (sscanf(buffer, "%d", &enabled) == 1)
    {
        if (enabled == 1)
        {
            ptp_notice("ptp log enabled.\n");
            mt_ptp_thread = kthread_run(mt_ptp_thread_handler, 0, "ptp logging");
            if (IS_ERR(mt_ptp_thread))
            {
                printk("[%s]: failed to create ptp logging thread\n", __FUNCTION__);
            }
            hrtimer_start(&mt_ptp_timer, ktime, HRTIMER_MODE_REL);
        }
        else if (enabled == 0)
        {
           kthread_stop(mt_ptp_thread);
           hrtimer_cancel(&mt_ptp_timer);
        }
        else
        {
            ptp_notice("ptp log disabled.\n");
            ptp_notice("bad argument!! argument should be \"0\" or \"1\"\n");
        }
    }
    else
    {
        ptp_notice("bad argument!! argument should be \"0\" or \"1\"\n");
    }

    return count;
}

static int __init ptp_init(void)
{
    struct proc_dir_entry *entry = NULL;
    struct proc_dir_entry *dir = NULL;

    int err = 0;

    ptp_data[0] = 0xffffffff;

    hrtimer_init(&mt_ptp_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
    mt_ptp_timer.function = mt_ptp_timer_func;

    dir = proc_mkdir("ptp", NULL);
    if (!dir)
    {
        ptp_notice("[%s]: mkdir /proc/ptp failed\n", __FUNCTION__);
    }
    else
    {
        entry = create_proc_entry("ptp_debug", S_IRUGO | S_IWUSR | S_IWGRP, dir);
        if (entry)
        {
            entry->read_proc = ptp_debug_read;
            entry->write_proc = ptp_debug_write;
        }

        entry = create_proc_entry("ptp_dump", S_IRUGO | S_IWUSR | S_IWGRP, dir);
        if (entry)
        {
            entry->read_proc = ptp_dump_read;
        }

        entry = create_proc_entry("ptp_log_en", S_IRUGO | S_IWUSR | S_IWGRP, dir);
        if (entry)
        {
            entry->write_proc = ptp_log_en_write;
        }

        entry = create_proc_entry("ptp_status", S_IRUGO | S_IWUSR | S_IWGRP, dir);
        if (entry)
        {
            entry->read_proc = ptp_status_read;
        }

        entry = create_proc_entry("ptp_cur_volt", S_IRUGO | S_IWUSR | S_IWGRP, dir);
        if (entry)
        {
            entry->read_proc = ptp_cur_volt_read;
        }
    }

    err = platform_driver_register(&ptp_driver);

    if (err)
    {
        ptp_notice("PTP driver callback register failed..\n");
        return err;
    }

    return 0;
}

static void __exit ptp_exit(void)
{
    ptp_notice("PTP de-initialization\n");
}

late_initcall(ptp_init);
#endif

MODULE_DESCRIPTION("MediaTek PTPOD Driver v0.2");
MODULE_LICENSE("GPL");
