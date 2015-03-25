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

#include "mach/mt_typedefs.h"
#include "mach/irqs.h"
#include "mach/mt_ptp.h"
#include "mach/mt_reg_base.h"
#include "mach/mt_cpufreq.h"
#include "mach/mt_thermal.h"
#include "devinfo.h"
#include "mach/mt_clkmgr.h"
#include "mach/mt_pmic_wrap.h"

#ifdef CONFIG_PTP
#define PTP_Get_Real_Val 1

static unsigned int val_0 = 0x14f76907;
static unsigned int val_1 = 0xf6AAAAAA;
static unsigned int val_2 = 0x14AAAAAA;
static unsigned int val_3 = 0x60260000;

static unsigned int PTP_DCVOFFSET = 0;
static unsigned int PTP_AGEVOFFSET = 0;
static unsigned int CPU_Freq[MAX_OPP_NUM];
static unsigned int hw_calc_start = true;
static unsigned int PTP_Enable = false;
static unsigned int PTP_INIT_FLAG = 0;
unsigned int PTP_V[MAX_OPP_NUM];
unsigned int PTP_MON_V[MAX_OPP_NUM];
static PTP_Init_T PTP_Init_value;

static int ptp_init_setup(enum init_type mode, PTP_Init_T* PTP_Init_val);
irqreturn_t PTP_ISR(int irq, void *dev_id);
static void ptp_mon(void);


static struct hrtimer mt_ptp_timer;
struct task_struct *mt_ptp_thread = NULL;
static DECLARE_WAIT_QUEUE_HEAD(mt_ptp_timer_waiter);

static int mt_ptp_timer_flag = 0;
static int mt_ptp_period_s = 2;
static int mt_ptp_period_ns = 0;

enum hrtimer_restart mt_ptp_timer_func(struct hrtimer *timer)
{
    mt_ptp_timer_flag = 1; wake_up_interruptible(&mt_ptp_timer_waiter);
    return HRTIMER_NORESTART;
}

int mt_ptp_thread_handler(void *unused)
{
    unsigned int temperature;
    do
    {
        ktime_t ktime = ktime_set(mt_ptp_period_s, mt_ptp_period_ns);

        wait_event_interruptible(mt_ptp_timer_waiter, mt_ptp_timer_flag != 0);
        mt_ptp_timer_flag = 0;

        temperature = (ptp_read(PTP_TEMP)&0xff);
        if( (temperature > 0x4b) && (temperature < 0xd3) ){
        	
        }
        else{
            ptp_print("PTP_LOG: (%d) - (0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x) - (%d, %d, %d, %d, %d, %d, %d, %d)\n", \
                    (temperature + 25)*1000, \
                    ptp_read(PMIC_WRAP_DVFS_WDATA3), \
                    ptp_read(PMIC_WRAP_DVFS_WDATA4), \
                    ptp_read(PMIC_WRAP_DVFS_WDATA5), \
                    0, \
                    0, \
                    0, \
                    0, \
                    0, \
                    CPU_Freq[0], \
                    CPU_Freq[1], \
                    CPU_Freq[2], \
                    CPU_Freq[3], \
                    CPU_Freq[4], \
                    CPU_Freq[5], \
                    CPU_Freq[6], \
                    CPU_Freq[7]);
        }
        hrtimer_start(&mt_ptp_timer, ktime, HRTIMER_MODE_REL);
    } while (!kthread_should_stop());

    return 0;
}

unsigned int get_ptp_level(void)
{
    unsigned int ver;
    ver = get_devinfo_with_index(PTP_LEVEL_INDEX);
    ver = (ver >> 8) & 0x00000003;
    return ver;
}

static void PTP_set_ptp_volt(unsigned int volt, unsigned int freq)
{
	  if(freq != 0){
	  	  ptp_isr_print("set voltage: 0x%x frequency: %d\n", volt, freq);
        set_spm_tbl(volt, freq);
    }
}


irqreturn_t PTP_ISR(int irq, void *dev_id)
{
    unsigned int PTPINTSTS, temp, ptpen, temperature, i;
    unsigned int dcvalues, agecount;
    bool PTP_Change[MAX_OPP_NUM];           

    PTPINTSTS = ptp_read(PTP_PTPINTSTS);
    dcvalues = ptp_read(PTP_DCVALUES);
    agecount = ptp_read(PTP_AGECOUNT);
    ptpen = ptp_read(PTP_PTPEN);

    ptp_isr_print("[ISR Entry Point]\n");
    ptp_isr_print("PTPINTSTS = 0x%x\n PTP_DCVALUES = 0x%x\n PTP_AGECOUNT = 0x%x\n PTP_PTPEN = 0x%x\n", \
                  PTPINTSTS, dcvalues, agecount, ptpen);

    if( PTPINTSTS == 0x1 ) // PTP init1 or init2
    {
        if((ptpen & 0x7) == 0x1){ // init1

            ptp_isr_print("Enter PTPOD Init 1\n");
            ptp_isr_print("shouldn't happen in linux kernel\n");
            //PTP_DCVOFFSET = ~(ptp_read(PTP_DCVALUES) & 0xffff)+1;  // hw bug, workaround
            //PTP_AGEVOFFSET = ptp_read(PTP_AGEVALUES) & 0xffff;
        
            //PTP_INIT_FLAG = 1;

            // Set PTPEN.PTPINITEN/PTPEN.PTPINIT2EN = 0x0 & Clear PTP INIT interrupt PTPINTSTS = 0x00000001
            //ptp_write(PTP_PTPEN, 0x0);
            //ptp_write(PTP_PTPINTSTS, 0x1);
            // switch block clock source to MAINPLL/12. bclk should be 66.625 (DDR2)
            //clkmux_sel(MT_CLKMUX_AXIBUS_GFMUX_SEL, MT_CG_MPLL_D12, "PTP_BCLK");

            /*for(loop = 0x0; loop <= 0x7c; loop += 0x4){
                ptp_print("Reg[0x%x]: 0x%x\n", PTP_ctr_reg_addr+loop, ptp_read(PTP_ctr_reg_addr+loop));
            }*/
            // trigger init2
            //ptp_init2_standalone(DCVoffset, AGEVoffset);
        }
        else if((ptpen & 0x7) == 0x5){ // init2
            // read PTP_VO_0 ~ PTP_VO_3
            temp = ptp_read( PTP_VOP30 );
            PTP_V[0] = temp & 0xff;
            PTP_V[1] = (temp>>8) & 0xff;
            PTP_V[2] = (temp>>16) & 0xff;
            PTP_V[3] = (temp>>24) & 0xff;

            // read PTP_VO_3 ~ PTP_VO_7
            temp = ptp_read( PTP_VOP74 );
            PTP_V[4] = temp & 0xff;
            PTP_V[5] = (temp>>8) & 0xff;
            PTP_V[6] = (temp>>16) & 0xff;
            PTP_V[7] = (temp>>24) & 0xff;

            for(i = 0; i < MAX_OPP_NUM; i++){
                PTP_set_ptp_volt(PTP_V[i], CPU_Freq[i]);
            }
            // set PTP_VO_0 ~ PTP_VO_7 to PMIC
            ptp_isr_print("Init2 PTP_V_0 = 0x%x\n", PTP_V[0]);
            ptp_isr_print("Init2 PTP_V_1 = 0x%x\n", PTP_V[1]);
            ptp_isr_print("Init2 PTP_V_2 = 0x%x\n", PTP_V[2]);
            ptp_isr_print("Init2 PTP_V_3 = 0x%x\n", PTP_V[3]);
            ptp_isr_print("Init2 PTP_V_4 = 0x%x\n", PTP_V[4]);
            ptp_isr_print("Init2 PTP_V_5 = 0x%x\n", PTP_V[5]);
            ptp_isr_print("Init2 PTP_V_6 = 0x%x\n", PTP_V[6]);
            ptp_isr_print("Init2 PTP_V_7 = 0x%x\n", PTP_V[7]); 
            
            //PTP_set_ptp_volt();

            ptp_write(PTP_PTPEN, 0x0);
            ptp_write(PTP_PTPINTSTS, 0x1);

#ifdef CONFIG_THERMAL
            PTP_INIT_FLAG = 1;
            ptp_mon();
#endif
        }
        else {  // error handler
            // disable PTP
            ptp_isr_print("Error interrupt type: not init1 nor init2 \n");
            ptp_isr_print("====================================================\n");
            ptp_isr_print("PTP error_0 (0x%x) : PTPINTSTS = 0x%x\n", ptpen, PTPINTSTS);
            ptp_isr_print("====================================================\n");
            
            ptp_write(PTP_PTPEN, 0x0);            
            // Clear PTP interrupt PTPINTSTS
            ptp_write(PTP_PTPINTSTS, 0x00ffffff);
         
            restore_default_volt();
            PTP_INIT_FLAG = 0;   
        }
    }
    else if( (PTPINTSTS & 0x00ff0002) != 0x0 )  // PTP Monitor mode
    {   
        temperature = (ptp_read(PTP_TEMP)&0xff); // temp_0
        ptp_isr_print("ptpod temperature: %d\n", temperature);
        
        if( (temperature > 0x4b) && (temperature < 0xd3) ){
            //ptp_print("temperature error\n");
            ptp_isr_print("temperature sensor is not ready: %d\n", temperature);
        }
        else{
            ptp_isr_print("[ISR Entry Begin]\n");
            temp = ptp_read( PTP_VOP30 );
            PTP_MON_V[0] = temp & 0xff;
            PTP_MON_V[1] = (temp>>8) & 0xff;
            PTP_MON_V[2] = (temp>>16) & 0xff;
            PTP_MON_V[3] = (temp>>24) & 0xff;

            // read PTP_VO_3 ~ PTP_VO_7
            temp = ptp_read( PTP_VOP74 );
            PTP_MON_V[4] = temp & 0xff;
            PTP_MON_V[5] = (temp>>8) & 0xff;
            PTP_MON_V[6] = (temp>>16) & 0xff;
            PTP_MON_V[7] = (temp>>24) & 0xff;

            for(i = 0; i < MAX_OPP_NUM; i++){
                PTP_Change[i] = (PTPINTSTS >> (16 + i)) & 0x1;
                if(PTP_Change[i] != 0){
                    ptp_isr_print("MON_Volt Changed. PTP_MON_V[%d] = 0x%x\n", i, PTP_MON_V[i]);
                    PTP_set_ptp_volt(PTP_MON_V[i], CPU_Freq[i]);
                }
            }
            
            ptp_isr_print("TEMPSPARE1 = 0x%x\n", ptp_read(TEMPSPARE1));

            // Clear PTP INIT interrupt PTPINTSTS = 0x00ff0002
            ptp_write(PTP_PTPINTSTS, 0x00ff0000);
        }
    }
    else // PTP error
    {
        if( ((ptpen & 0x7) == 0x1) || ((ptpen & 0x7) == 0x5) )    // init 1  || init 2 error handler ======================
        {
            ptp_isr_print("====================================================\n");
            ptp_isr_print("PTP error_1 error_2 (0x%x) : PTPINTSTS = 0x%x\n", ptpen, PTPINTSTS);
            ptp_isr_print("PTP_SMSTATE0 (0x%x) = 0x%x\n", PTP_SMSTATE0, ptp_read(PTP_SMSTATE0) );
            ptp_isr_print("PTP_SMSTATE1 (0x%x) = 0x%x\n", PTP_SMSTATE1, ptp_read(PTP_SMSTATE1) );
            ptp_isr_print("====================================================\n");
            
        }
        else    // PTP Monitor mode error handler ======================
        {
            ptp_isr_print("====================================================\n");
            ptp_isr_print("PTP error_m (0x%x) : PTPINTSTS = 0x%x\n", ptpen, PTPINTSTS);
            ptp_isr_print("PTP_SMSTATE0 (0x%x) = 0x%x\n", PTP_SMSTATE0, ptp_read(PTP_SMSTATE0) );
            ptp_isr_print("PTP_SMSTATE1 (0x%x) = 0x%x\n", PTP_SMSTATE1, ptp_read(PTP_SMSTATE1) );
            ptp_isr_print("PTP_TEMP (0x%x) = 0x%x\n", PTP_TEMP, ptp_read(PTP_TEMP) );
            
            ptp_isr_print("PTP_TEMPMSR0 (0x%x) = 0x%x\n", TEMPMSR0, ptp_read(TEMPMSR0) );
            ptp_isr_print("PTP_TEMPMSR1 (0x%x) = 0x%x\n", TEMPMSR1, ptp_read(TEMPMSR1) );
            ptp_isr_print("PTP_TEMPMSR2 (0x%x) = 0x%x\n", TEMPMSR2, ptp_read(TEMPMSR2) );
            ptp_isr_print("PTP_TEMPMONCTL0 (0x%x) = 0x%x\n", TEMPMONCTL0, ptp_read(TEMPMONCTL0) );
            ptp_isr_print("PTP_TEMPMSRCTL1 (0x%x) = 0x%x\n", TEMPMSRCTL1, ptp_read(TEMPMSRCTL1) );
            ptp_isr_print("====================================================\n");
            
        }
        // disable PTP
        ptp_write(PTP_PTPEN, 0x0);
        // Clear PTP interrupt PTPINTSTS
        ptp_write(PTP_PTPINTSTS, 0x00ffffff);
        // restore default DVFS table (PMIC)
        restore_default_volt();
        PTP_INIT_FLAG = 0; 
    }
    hw_calc_start = false;
    
    return IRQ_HANDLED;
}

static int ptp_init_setup(enum init_type mode, PTP_Init_T* PTP_Init_val)
{
    // config PTP register
    ptp_write(PTP_DESCHAR, ((((PTP_Init_val->BDES)<<8)&0xff00)|((PTP_Init_val->MDES)&0xff)));
    ptp_write(PTP_TEMPCHAR, ((((PTP_Init_val->VCO)<<16)&0xff0000) | (((PTP_Init_val->MTDES)<<8)&0xff00) | ((PTP_Init_val->DVTFIXED)&0xff)));
    ptp_write(PTP_DETCHAR, ((((PTP_Init_val->DCBDET)<<8)&0xff00) | ((PTP_Init_val->DCMDET)&0xff)));
    ptp_write(PTP_AGECHAR, ((((PTP_Init_val->AGEDELTA)<<8)&0xff00)  | ((PTP_Init_val->AGEM)&0xff)));
    ptp_write(PTP_DCCONFIG, ((PTP_Init_val->DCCONFIG)));
    ptp_write(PTP_AGECONFIG, ((PTP_Init_val->AGECONFIG)));
    if(mode == MONITOR_MODE){
        ptp_write(PTP_TSCALCS, ((((PTP_Init_val->BTS)<<12)&0xfff000) | ((PTP_Init_val->MTS)&0xfff)));
    }

    if( PTP_Init_val->AGEM == 0x0 )
    {
        ptp_write(PTP_RUNCONFIG, 0x80000000);
    }
    else
    {
        unsigned int temp_i, temp_filter, temp_value;
       
        temp_value = 0x0; 
        for (temp_i = 0 ; temp_i < 24 ; temp_i += 2 )
        {
            temp_filter = 0x3 << temp_i;
            	
            if( ((PTP_Init_val->AGECONFIG) & temp_filter) == 0x0 ){
                temp_value |= (0x1 << temp_i);
            }
            else{
                temp_value |= ((PTP_Init_val->AGECONFIG) & temp_filter);
            }
        }
        ptp_write(PTP_RUNCONFIG, temp_value);
    }

    ptp_write(PTP_FREQPCT30, ((((PTP_Init_val->FREQPCT[3])<<24)&0xff000000) | (((PTP_Init_val->FREQPCT[2])<<16)&0xff0000) | (((PTP_Init_val->FREQPCT[1])<<8)&0xff00) | ((PTP_Init_val->FREQPCT[0]) & 0xff)));
    ptp_write(PTP_FREQPCT74, ((((PTP_Init_val->FREQPCT[7])<<24)&0xff000000) | (((PTP_Init_val->FREQPCT[6])<<16)&0xff0000) | (((PTP_Init_val->FREQPCT[5])<<8)&0xff00) | ((PTP_Init_val->FREQPCT[4]) & 0xff)));
    ptp_write(PTP_LIMITVALS, ((((PTP_Init_val->VMAX)<<24)&0xff000000) | (((PTP_Init_val->VMIN)<<16)&0xff0000) | (((PTP_Init_val->DTHI)<<8)&0xff00) | ((PTP_Init_val->DTLO) & 0xff)));
    ptp_write(PTP_VBOOT, (((PTP_Init_val->VBOOT)&0xff)));
    ptp_write(PTP_DETWINDOW, (((PTP_Init_val->DETWINDOW)&0xffff)));
    ptp_write(PTP_PTPCONFIG, (((PTP_Init_val->DETMAX)&0xffff)));

    // clear all pending PTP interrupt & config PTPINTEN
    ptp_write(PTP_PTPINTSTS, 0xffffffff);

    // enable PTP INIT measurement
    hw_calc_start = true;
    if(mode == INIT1_MODE){
        ptp_write(PTP_PTPINTEN, 0x00005f01);
        ptp_write(PTP_PTPEN, 0x00000001);
        //ptp_print("init1 start!!!!!\n");
        return 0;
    }
    else if(mode == INIT2_MODE){
        ptp_write(PTP_PTPINTEN, 0x00005f01);
        ptp_write(PTP_INIT2VALS, ((((PTP_Init_val->AGEVOFFSETIN)<<16)&0xffff0000) | ((PTP_Init_val->DCVOFFSETIN)&0xffff)));
        ptp_write(PTP_PTPEN, 0x00000005); 
        return 0;
    }
    else if(mode == MONITOR_MODE){
        ptp_write(PTP_PTPINTEN, 0x00FF0000);
        ptp_write(PTP_PTPEN, 0x00000002);
        return 0;
    }
    else{
        ptp_print("[ERROR]ptp_init_setup: unknown type\n");
        return -1;
    }
}


void PTP_disable_ptp(void)
{
    unsigned long flags;  
    
    // Mask ARM i bit
    local_irq_save(flags);
    
    // disable PTP
    ptp_write(PTP_PTPEN, 0x0);
            
    // Clear PTP interrupt PTPINTSTS
    ptp_write(PTP_PTPINTSTS, 0x00ffffff);
            
    // restore default DVFS table (PMIC)
    // mt_cpufreq_return_default_DVS_by_ptpod();
    restore_default_volt();

    PTP_Enable = false;
    PTP_INIT_FLAG = 0;
    ptp_print("Disable PTP-OD done.\n");

    // Un-Mask ARM i bit
    local_irq_restore(flags);
}

static void ptp_mon(void)
{
    unsigned int i;
    struct TS_PTPOD ts_info;

    ptp_print("ptp_mon : ptp monitor mode start.\r\n");
    
    // PTP test code ================================
    /*PTP_Init_value.PTPINITEN = (val_0) & 0x1;
    PTP_Init_value.PTPMONEN = (val_0 >> 1) & 0x1;
    PTP_Init_value.ADC_CALI_EN = (val_0 >> 2) & 0x1;
    PTP_Init_value.MDES = (val_0 >> 8) & 0xff;
    PTP_Init_value.BDES = (val_0 >> 16) & 0xff;
    PTP_Init_value.DCMDET = (val_0 >> 24) & 0xff;
    
    PTP_Init_value.DCCONFIG = (val_1) & 0xffffff;
    PTP_Init_value.DCBDET = (val_1 >> 24) & 0xff;
    
    PTP_Init_value.AGECONFIG = (val_2) & 0xffffff;
    PTP_Init_value.AGEM = (val_2 >> 24) & 0xff;
    
    //PTP_Init_value.AGEDELTA = (val_3) & 0xff;
    PTP_Init_value.AGEDELTA = 0x88;    
    PTP_Init_value.DVTFIXED = (val_3 >> 8) & 0xff;
    PTP_Init_value.MTDES = (val_3 >> 16) & 0xff;
    PTP_Init_value.VCO = (val_3 >> 24) & 0xff;*/
    
    // (thermal need to provide get_thermal_slope_intercept)
    get_thermal_slope_intercept(&ts_info);
    PTP_Init_value.MTS = ts_info.ts_MTS; // (2048 * TS_SLOPE) + 2048; 
    PTP_Init_value.BTS = ts_info.ts_BTS; // 4 * TS_INTERCEPT; 

    for(i = 0; i < MAX_OPP_NUM; i++){
        PTP_Init_value.FREQPCT[i] = CPU_Freq[i] * 100 / STANDARD_FREQ; // max freq 1200 x 100%	
    }

    PTP_Init_value.DETWINDOW = 0x514;  //50 us. Detector sampling time as represented in cycles of bclk_ck
    PTP_Init_value.VMAX = 0x58; // 1.3125v (700mv + n * 6.25mv)    
    PTP_Init_value.VMIN = 0x48; // 1.15v (700mv + n * 6.25mv)    
    PTP_Init_value.DTHI = 0x01; // positive
    PTP_Init_value.DTLO = 0xff; // negative (2's compliment) 0xff for monitor, but 0xfe for init1/2
    PTP_Init_value.VBOOT = 0x48; // 115v  (700mv + n * 6.25mv)    
    PTP_Init_value.DETMAX = 0xffff; // This timeout value is in cycles of bclk_ck.

    // start test ============================================
    ptp_print("[Start Monitor mode Test]\n");
    ptp_print("DTHI = 0x%x\n", PTP_Init_value.DTHI);
    ptp_print("DTLO = 0x%x\n", PTP_Init_value.DTLO);
    ptp_print("PTP_TEMP: 0x%x\n", ptp_read(PTP_TEMP));
    ptp_init_setup(MONITOR_MODE, &PTP_Init_value);
}

static void ptp_init2(void)
{
    unsigned int i;

    ptp_print("ptp_init2: ptp init2 start.\r\n");

    //init_PTP_interrupt();
    
    //ptp_write(PTP_PTPEN, 0x0);        
    //PTP_Init_value.MDES = 0x69;
    //PTP_Init_value.BDES = 0xf7;
    //PTP_Init_value.DCMDET = 0x14;    
    //PTP_Init_value.DCBDET = 0xf6;    

    PTP_Init_value.PTPINITEN = (val_0) & 0x1;
    PTP_Init_value.PTPMONEN = (val_0 >> 1) & 0x1;
    PTP_Init_value.MDES = (val_0 >> 8) & 0xff;
    PTP_Init_value.BDES = (val_0 >> 16) & 0xff;
    PTP_Init_value.DCMDET = (val_0 >> 24) & 0xff;
    
    PTP_Init_value.DCCONFIG = (val_1) & 0xffffff;
    PTP_Init_value.DCBDET = (val_1 >> 24) & 0xff;
    
    PTP_Init_value.AGECONFIG = (val_2) & 0xffffff;
    PTP_Init_value.AGEM = (val_2 >> 24) & 0xff;
    
    PTP_Init_value.AGEDELTA = (val_3) & 0xff;
    PTP_Init_value.DVTFIXED = (val_3 >> 8) & 0xff;
    PTP_Init_value.MTDES = (val_3 >> 16) & 0xff;
    PTP_Init_value.VCO = (val_3 >> 24) & 0xff;

    /*PTP_Init_value.PTPINITEN = 0x1;
    PTP_Init_value.PTPMONEN = 0x0;
    PTP_Init_value.AGEM = 0x14;
    PTP_Init_value.DCCONFIG = 0xaaaaaa;
    PTP_Init_value.AGECONFIG = 0xaaaaaa;    
    PTP_Init_value.AGEDELTA = 0x0;
    PTP_Init_value.DVTFIXED = 0x0;
    PTP_Init_value.MTDES = 0x26;
    PTP_Init_value.VCO = 0x60;*/

    for(i = 0; i < MAX_OPP_NUM; i++){
        PTP_Init_value.FREQPCT[i] = CPU_Freq[i] * 100 / STANDARD_FREQ; // max freq 1200 x 100%	
    }
    

    PTP_Init_value.DETWINDOW = 0x514; //40 us. Detector sampling time as represented in cycles of bclk_ck 
    PTP_Init_value.VMAX = 0x58; // 1.25v (700mv + n * 6.25mv)    
    PTP_Init_value.VMIN = 0x48; // 1.15v (700mv + n * 6.25mv)    
    PTP_Init_value.DTHI = 0x01; // positive
    PTP_Init_value.DTLO = 0xfe; // negative (2's compliment)
    PTP_Init_value.VBOOT = 0x48; // 115v  (700mv + n * 6.25mv)    
    PTP_Init_value.DETMAX = 0xffff; // This timeout value is in cycles of bclk_ck.

    PTP_Init_value.DCVOFFSETIN = PTP_DCVOFFSET;
    PTP_Init_value.AGEVOFFSETIN = PTP_AGEVOFFSET;
    ptp_print("[Start Init2]\n");
    ptp_print("PTP_Init_value.DCVOFFSETIN = 0x%x\n", PTP_Init_value.DCVOFFSETIN);
    ptp_print("PTP_Init_value.AGEVOFFSETIN = 0x%x\n", PTP_Init_value.AGEVOFFSETIN);
    // set register for init2
    ptp_init_setup(INIT2_MODE, &PTP_Init_value);

}

void init_PTP_interrupt(void)
{
    int r;
      
    // Set PTP IRQ =========================================
    r = request_irq(PTP_FSM_IRQ_ID, PTP_ISR, IRQF_TRIGGER_LOW, "PTP", NULL);
    if (r)
    {
        ptp_print("PTP IRQ register failed (%d)\n", r);
        WARN_ON(1);
    }
        
    ptp_print("init_PTP_interrupt: Set PTP IRQ OK.\r\n");
}


static int ptp_probe(struct platform_device *pdev)
{
    unsigned int i;
    
    // enable thermal CG
    enable_clock(MT_CG_THEM_SW_CG, "PTPOD");
    // get ptpod value
#if PTP_Get_Real_Val
    val_0 = get_devinfo_with_index(8);
    val_1 = get_devinfo_with_index(9);
    val_2 = get_devinfo_with_index(21);
    val_3 = get_devinfo_with_index(22);
#endif

    if( (val_0 & 0x1) == 0x0 )
    {
        ptp_print("PTPINITEN = 0x%x \n", (val_0 & 0x1));
        ptp_print("PTPOD feature disable \n");
        return 0;
    }
    else if(ptp_read(PTP_PTPINTSTS) != 0x0)
    {
        ptp_print("PTPOD init1 error, PTPINTSTS: 0x%x \n", ptp_read(PTP_PTPINTSTS));
        return 0;
    }
    PTP_Enable = true;
    // Set PTP IRQ =========================================
    init_PTP_interrupt();

    // Get DVFS frequency table ================================
    for(i = 0; i < MAX_OPP_NUM; i++){
        CPU_Freq[i] = (unsigned int)(mt_cpufreq_max_frequency_by_DVS(i));
    }
    
    for(i = 0; i < MAX_OPP_NUM; i++){
        ptp_print("OPP[%d] for PTPOD: %d", i, CPU_Freq[i]);	
    }
    //ptp_level = PTP_get_ptp_level();
    
    //read PTP_DCVOFFSET & PTP_AGEVOFFSET
    PTP_DCVOFFSET = ~(ptp_read(PTP_DCVALUES) & 0xffff) + 1;
    PTP_AGEVOFFSET = ptp_read(PTP_AGEVALUES) & 0xffff;
    //ptp_init2
    ptp_init2();    

    return 0;
}

static int ptp_suspend(struct platform_device *pdev, pm_message_t state)
{
	  disable_clock(MT_CG_THEM_SW_CG, "PTPOD");
    ptp_print("Disable PTPOD clock\n");
    return 0;
}

static int ptp_resume(struct platform_device *pdev)
{
	  enable_clock(MT_CG_THEM_SW_CG, "PTPOD");
	  ptp_print("Enable PTPOD clock\n");
	  if(PTP_Enable == true)
        ptp_init2();
    return 0;
}


static struct platform_driver mtk_ptp_driver = {
    .remove     = NULL,
    .shutdown   = NULL,
    .probe      = ptp_probe,
    .suspend	  = ptp_suspend,
    .resume		  = ptp_resume,
    .driver     = {
        .name = "mtk-ptp",
    },
};

/***************************
* show current PTP stauts
****************************/
static int ptp_debug_read(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
    int len = 0;
    char *p = buf;

    if (PTP_Enable)
        p += sprintf(p, "PTP enabled\n");
    else
        p += sprintf(p, "PTP disabled\n");

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
            // Disable PTP and Restore default DVFS table (PMIC)
            PTP_disable_ptp();
        }
        else
        {
            ptp_print("bad argument_0!! argument should be \"0\"\n");
        }
    }
    else
    {
        ptp_print("bad argument_1!! argument should be \"0\"\n");
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
    unsigned int rdata = 0;

    if (pwrap_read(VPROC_VOSEL_ON, &rdata) == 0)
    {
        p += sprintf(p, "current voltage: (0x%x)\n", rdata);
    }
    else
    {
        p += sprintf(p, "read current voltage fail\n");
    }

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
            ptp_print("ptp log enabled.\n");
            mt_ptp_thread = kthread_run(mt_ptp_thread_handler, 0, "ptp logging");
            if (IS_ERR(mt_ptp_thread))
            {
                ptp_print("[%s]: failed to create ptp logging thread\n", __FUNCTION__);
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
            ptp_print("bad argument!! argument should be \"0\" or \"1\"\n");
        }
    }
    else
    {
        ptp_print("bad argument!! argument should be \"0\" or \"1\"\n");
    }

    return count;
}

static int __init ptp_init(void)
{
    struct proc_dir_entry *mt_entry = NULL;
    struct proc_dir_entry *mt_ptp_dir = NULL;
    int ptp_err = 0;

    hrtimer_init(&mt_ptp_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
    mt_ptp_timer.function = mt_ptp_timer_func;

    mt_ptp_dir = proc_mkdir("ptp", NULL);
    if (!mt_ptp_dir)
    {
        ptp_print("[%s]: mkdir /proc/ptp failed\n", __FUNCTION__);
    }
    else
    {
        mt_entry = create_proc_entry("ptp_debug", S_IRUGO | S_IWUSR | S_IWGRP, mt_ptp_dir);
        if (mt_entry)
        {
            mt_entry->read_proc = ptp_debug_read;
            mt_entry->write_proc = ptp_debug_write;
        }
        mt_entry = create_proc_entry("ptp_dump", S_IRUGO | S_IWUSR | S_IWGRP, mt_ptp_dir);
        if (mt_entry)
        {
            mt_entry->read_proc = ptp_dump_read;
        }
        mt_entry = create_proc_entry("ptp_log_en", S_IRUGO | S_IWUSR | S_IWGRP, mt_ptp_dir);
        if (mt_entry)
        {
            mt_entry->write_proc = ptp_log_en_write;
        }
        
        mt_entry = create_proc_entry("ptp_cur_volt", S_IRUGO | S_IWUSR | S_IWGRP, mt_ptp_dir);
        if (mt_entry)
        {
            mt_entry->read_proc = ptp_cur_volt_read;
        }
    }
    

    ptp_err = platform_driver_register(&mtk_ptp_driver);
    
    if (ptp_err)
    {
        ptp_print("PTP driver callback register failed..\n");
        return ptp_err;
    }
    
    return 0;
}

static void __exit ptp_exit(void)
{
    ptp_print("Exit PTP\n");
}

late_initcall(ptp_init);
#endif //CONFIG_PTP
