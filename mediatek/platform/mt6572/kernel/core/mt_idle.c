#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/cpu.h>

#include <linux/types.h>
#include <linux/string.h>
#include <mach/mt_cirq.h>
#include <asm/system_misc.h>
#include <mach/mt_spm.h>

#include <mach/mt_typedefs.h>
#include <mach/sync_write.h>
#include <mach/mt_idle.h>
#include <mach/mt_clkmgr.h>
#include <mach/mt_dcm.h>
#include <mach/mt_gpt.h>
#include <mach/mt_spm_api.h>
#include <mach/mt_spm_pcm.h>
#include <mach/hotplug.h>
#include <mach/mt_clkmgr.h>


#define USING_XLOG

#define IDLE_TEMPERORY_DISABLE 1 //TODO: platform dependent
#define IDLE_DPIDLE_STRESS  0
#define SPM_SUSPEND_GPT_EN

#ifdef USING_XLOG 
#include <linux/xlog.h>

#define TAG     "Power/swap"

#define idle_err(fmt, args...)       \
    xlog_printk(ANDROID_LOG_ERROR, TAG, fmt, ##args)
#define idle_warn(fmt, args...)      \
    xlog_printk(ANDROID_LOG_WARN, TAG, fmt, ##args)
#define idle_info(fmt, args...)      \
    xlog_printk(ANDROID_LOG_INFO, TAG, fmt, ##args)
#define idle_dbg(fmt, args...)       \
    xlog_printk(ANDROID_LOG_DEBUG, TAG, fmt, ##args)
#define idle_ver(fmt, args...)       \
    xlog_printk(ANDROID_LOG_VERBOSE, TAG, fmt, ##args)

#else /* !USING_XLOG */

#define TAG     "[Power/swap] "

#define idle_err(fmt, args...)       \
    printk(KERN_ERR TAG);           \
    printk(KERN_CONT fmt, ##args) 
#define idle_warn(fmt, args...)      \
    printk(KERN_WARNING TAG);       \
    printk(KERN_CONT fmt, ##args)
#define idle_info(fmt, args...)      \
    printk(KERN_NOTICE TAG);        \
    printk(KERN_CONT fmt, ##args)
#define idle_dbg(fmt, args...)       \
    printk(KERN_INFO TAG);          \
    printk(KERN_CONT fmt, ##args)
#define idle_ver(fmt, args...)       \
    printk(KERN_DEBUG TAG);         \
    printk(KERN_CONT fmt, ##args)

#endif


#define INVALID_GRP_ID(grp) (grp < 0 || grp >= NR_GRPS)


extern unsigned long localtimer_get_counter(void);
extern int localtimer_set_next_event(unsigned long evt);


bool __attribute__((weak)) 
clkmgr_idle_can_enter(unsigned int *condition_mask, unsigned int *block_mask)
{
    return false;
}

enum {
    BY_CPU = 0,
    BY_CLK = 1,
    BY_TMR = 2,
    BY_OTH = 3,
    NR_REASONS = 4,
};

static const char *idle_name[NR_TYPES] = {
    "mcidle",
    "dpidle",
    "rgidle",
};

static const char *reason_name[NR_REASONS] = {
    "by_cpu",
    "by_clk",
    "by_tmr",
    "by_oth",
};

static int idle_switch[NR_TYPES] = {
    0,  //mcidle switch 
    1,  //dpidle switch
    1,  //rgidle switch
};

/************************************************
 * multi-core idle part
 ************************************************/
#ifdef SPM_MCDI_FUNC
extern u32 En_SPM_MCDI;

static unsigned int mcidle_gpt_percpu[NR_CPUS] = {
    NR_GPTS,
    GPT4,//Core1

};

//72 TODO, must check CG
static unsigned int mcidle_condition_mask[NR_GRPS] = {
    0xFDC40DC1, //PERI0: i2c5~0, uart2~0, aphif, usb1~0, pwm7~5, nfi
    0x00000009, //PERI1: spi1, i2c6
    0x00000000, //INFRA:
    0x00000000, //TOPCK:
    0x01ff1e8e, //DISP0: g2d,cmdq,gamma,rdma1,wdma1,wdma0,2DSHP,SCL,ROT
    0x00000000, //DISP1:
    0x00003FF5, //IMAGE: all
    0x00000000, //MFG:
    0x00000040, //AUDIO: i2s
    0x00000001, //VDEC0: all
    0x00000001, //VDEC1: all
    0x00000001, //VENC:  all
};

static unsigned int mcidle_block_mask[NR_GRPS] = {0x0};

static unsigned int mcidle_timer_left[NR_CPUS];
static unsigned int mcidle_timer_left2[NR_CPUS];
static unsigned int mcidle_time_critera = 26000;

static unsigned long mcidle_cnt[NR_CPUS] = {0};
static unsigned long mcidle_block_cnt[NR_CPUS][NR_REASONS] = {{0}};

static DEFINE_MUTEX(mcidle_locked);

static void enable_mcidle_by_mask(int grp, unsigned int mask)
{
#if ( IDLE_TEMPERORY_DISABLE == 0 )
    mutex_lock(&mcidle_locked);
    mcidle_condition_mask[grp] &= ~mask;
    mutex_unlock(&mcidle_locked);
#endif    
}

static void disable_mcidle_by_mask(int grp, unsigned int mask)
{
#if ( IDLE_TEMPERORY_DISABLE == 0 )
    mutex_lock(&mcidle_locked);
    mcidle_condition_mask[grp] |= mask;
    mutex_unlock(&mcidle_locked);
#endif    
}

void enable_mcidle_by_bit(int id)
{
#if ( IDLE_TEMPERORY_DISABLE == 0 )
    int grp = id / 32;
    unsigned int mask = 1U << (id % 32);
    BUG_ON(INVALID_GRP_ID(grp));
    enable_mcidle_by_mask(grp, mask);
#endif    
}
EXPORT_SYMBOL(enable_mcidle_by_bit);

void disable_mcidle_by_bit(int id)
{
#if ( IDLE_TEMPERORY_DISABLE == 0 )
    int grp = id / 32;
    unsigned int mask = 1U << (id % 32);
    BUG_ON(INVALID_GRP_ID(grp));
    disable_mcidle_by_mask(grp, mask);
#endif    
}
EXPORT_SYMBOL(disable_mcidle_by_bit);


bool mcidle_can_enter(int cpu)
{
    int reason = NR_REASONS;

    if (En_SPM_MCDI != 1) {
        reason = BY_OTH;
        goto out;
    } 

#if 1
    /* only SODI */
    if (cpu != 0) {
        reason = BY_OTH;
        goto out;
    }
#endif

    if (atomic_read(&is_in_hotplug) == 1) {
        reason = BY_CPU;
        goto out;
    }

#if( IDLE_TEMPERORY_DISABLE == 0 )

    if (cpu == 0) {
        memset(mcidle_block_mask, 0, NR_GRPS * sizeof(unsigned int));
        if (!clkmgr_idle_can_enter(mcidle_condition_mask, mcidle_block_mask)) {
            reason = BY_CLK;
            goto out;
        }
    }
#endif

    mcidle_timer_left[cpu] = localtimer_get_counter();
    if (mcidle_timer_left[cpu] < mcidle_time_critera || 
            ((int)mcidle_timer_left[cpu]) < 0) {
        reason = BY_TMR;
        goto out;
    }

out:
    if (reason < NR_REASONS) {
        mcidle_block_cnt[cpu][reason]++;
        return false;
    } else {
        return true;
    }
}

static void mcidle_before_wfi(int cpu)
{
    unsigned int id = mcidle_gpt_percpu[cpu];
    if (cpu != 0) {
        mcidle_timer_left2[cpu] = localtimer_get_counter(); 
#ifdef SPM_SUSPEND_GPT_EN
        err = request_gpt(id, GPT_ONE_SHOT, GPT_CLK_SRC_SYS, GPT_CLK_DIV_1, 
                    0, NULL, GPT_NOAUTOEN);
        if (err) {
            idle_info("[%s]fail to request GPT4\n", __func__);
        }
#endif        
        gpt_set_cmp(id, mcidle_timer_left2[cpu]);
        start_gpt(id);
    }
}

static void mcidle_after_wfi(int cpu)
{
    unsigned int id = mcidle_gpt_percpu[cpu];
    if (cpu != 0) {
        if (gpt_check_and_ack_irq(id)) {
            localtimer_set_next_event(1);
        } else {
            /* waked up by other wakeup source */
            unsigned int cnt, cmp;
            gpt_get_cnt(id, &cnt);
            gpt_get_cmp(id, &cmp);
            if (unlikely(cmp < cnt)) {
                idle_err("[%s]GPT%d: counter = %10u, compare = %10u\n", __func__, 
                        id + 1, cnt, cmp);
                BUG();
            }
        
            localtimer_set_next_event(cmp-cnt);
            stop_gpt(id);
#ifdef SPM_SUSPEND_GPT_EN
            free_gpt(id);
#endif
        }
    }

    mcidle_cnt[cpu]++;
}

static void go_to_mcidle(int cpu)
{
    mcidle_before_wfi(cpu);

    spm_mcdi_wfi();

    mcidle_after_wfi(cpu);
}
#endif

/************************************************
 * deep idle part
 ************************************************/
static unsigned int dpidle_condition_mask[NR_GRPS]= 
{
    //CG_MIXED
    0x0,
    
    //CG_MPLL
    0x0,
    
    //CG_UPLL
    0x0,
    
    //CG_CTRL0
    PWM_MM_SW_CG_BIT|
    CAM_MM_SW_CG_BIT|
    MFG_MM_SW_CG_BIT|
    SPM_52M_SW_CG_BIT|
    DBI_BCLK_SW_CG_BIT|
    DBI_PAD0_SW_CG_BIT|
    DBI_PAD1_SW_CG_BIT|
    DBI_PAD2_SW_CG_BIT|
    DBI_PAD3_SW_CG_BIT,
    
    //CG_CTRL1
     EFUSE_SW_CG_BIT|
    //THEM_SW_CG_BIT|// //Thermal working from Arvin.Wang
    APDMA_SW_CG_BIT|
    I2C0_SW_CG_BIT|
    I2C1_SW_CG_BIT|
    AUX_SW_CG_MD_BIT|
    NFI_SW_CG_BIT|
    NFIECC_SW_CG_BIT|
    PWM_SW_CG_BIT|
    UART0_SW_CG_BIT|
    UART1_SW_CG_BIT|
    BTIF_SW_CG_BIT|
    USB_SW_CG_BIT|
    FHCTL_SW_CG_BIT|
    AUX_SW_CG_THERM_BIT|
    SPINFI_SW_CG_BIT|
    //MSDC0_SW_CG_BIT|
    //MSDC1_SW_CG_BIT|
    BIT(19) | // reserved_2
    SEJ_SW_CG_BIT|
    //MEMSLP_DLYER_SW_CG_BIT| //dpidle must enable
    BIT(23) | // reserved_3
    //AUX_SW_CG_ADC_BIT| //Thermal working from Arvin.Wang
    AUX_SW_CG_TP_BIT,

    //CG_MMSYS0
    SMI_COMMON_SW_CG_BIT|
    SMI_LARB0_SW_CG_BIT|
    MM_CMDQ_SW_CG_BIT|
    MM_CMDQ_SMI_IF_SW_CG_BIT|
    DISP_COLOR_SW_CG_BIT|
    DISP_BLS_SW_CG_BIT|
    DISP_WDMA_SW_CG_BIT|
    DISP_RDMA_SW_CG_BIT|
    DISP_OVL_SW_CG_BIT|
    MDP_TDSHP_SW_CG_BIT|
    MDP_WROT_SW_CG_BIT|
    MDP_WDMA_SW_CG_BIT|
    MDP_RSZ1_SW_CG_BIT|
    MDP_RSZ0_SW_CG_BIT|
    MDP_RDMA_SW_CG_BIT|
    MDP_BLS_26M_SW_CG_BIT|
    MM_CAM_SW_CG_BIT|
    MM_SENINF_SW_CG_BIT|
    MM_CAMTG_SW_CG_BIT|
    MM_CODEC_SW_CG_BIT|
    DISP_FAKE_ENG_SW_CG_BIT|
    MUTEX_SLOW_CLOCK_SW_CG_BIT,

    //CG_MMSYS1
    DSI_ENGINE_SW_CG_BIT|
    DSI_DIGITAL_SW_CG_BIT|
    DISP_DPI_ENGINE_SW_CG_BIT|
    DISP_DPI_IF_SW_CG_BIT|
    DISP_DBI_ENGINE_SW_CG_BIT|
    DISP_DBI_SMI_SW_CG_BIT|
    DISP_DBI_IF_SW_CG_BIT,

    //CG_MFG
    0x0,

    //CG_AUDIO
    0x0
};



static unsigned long rgidle_cnt[NR_CPUS] = {0};
static unsigned int dpidle_block_mask[NR_GRPS] = {0x0};

static unsigned long dpidle_cnt[NR_CPUS] = {0};
static unsigned long dpidle_block_cnt[NR_REASONS] = {0};

static DEFINE_MUTEX(dpidle_locked);

static void enable_dpidle_by_mask(int grp, unsigned int mask)
{
#if 1//( IDLE_TEMPERORY_DISABLE == 0 )
    mutex_lock(&dpidle_locked);
    dpidle_condition_mask[grp] &= ~mask;
    mutex_unlock(&dpidle_locked);
#endif	
}

static void disable_dpidle_by_mask(int grp, unsigned int mask)
{
#if 1//( IDLE_TEMPERORY_DISABLE == 0 )
    mutex_lock(&dpidle_locked);
    dpidle_condition_mask[grp] |= mask;
    mutex_unlock(&dpidle_locked);
#endif	
}

void enable_dpidle_by_bit(int id)
{
    unsigned int grp = clk_id_to_grp_id(id);
    unsigned int mask = clk_id_to_mask(id);

    if(( grp == NR_GRPS )||( mask == NR_GRPS ))
        idle_info("[%s]wrong clock id\n", __func__);
    else
        dpidle_condition_mask[grp] &= ~mask;   
}
EXPORT_SYMBOL(enable_dpidle_by_bit);

void disable_dpidle_by_bit(int id)
{

    unsigned int grp = clk_id_to_grp_id(id);
    unsigned int mask = clk_id_to_mask(id);

    if(( grp == NR_GRPS )||( mask == NR_GRPS ))
        idle_info("[%s]wrong clock id\n", __func__);
    else
        dpidle_condition_mask[grp] |= mask;
}
EXPORT_SYMBOL(disable_dpidle_by_bit);


signed int dpidle_timer_left;
signed int dpidle_timer_left2;
static signed int dpidle_time_critera = 54500;//26000/13MHz(gpt clock rate)=2ms
unsigned int dpidle_cg_block_counter = 0;
static bool dpidle_can_enter(void)
{

    int reason = NR_REASONS;
    int i;
    

#ifdef SPM_MCDI_FUNC
    if (En_SPM_MCDI != 0) {
        reason = BY_OTH;
        goto out;
    }
#endif

    //if ((smp_processor_id() != 0) || (num_online_cpus() != 1)) {
    if (atomic_read(&hotplug_cpu_count) != 1) {
        reason = BY_CPU;
        goto out;
    }


    memset(dpidle_block_mask, 0, NR_GRPS * sizeof(unsigned int));
    if (!clkmgr_idle_can_enter(dpidle_condition_mask, dpidle_block_mask)) {
        dpidle_cg_block_counter++;
        reason = BY_CLK;
        goto out;
    }

    dpidle_timer_left = localtimer_get_counter();
    if (dpidle_timer_left < dpidle_time_critera || 
            ((int)dpidle_timer_left) < 0) {
        reason = BY_TMR;
        goto out;
    }
  

out:
    if (reason < NR_REASONS) {
        if(dpidle_cg_block_counter>=10000)
        {
            if ((smp_processor_id() == 0))
            {
                for (i = 0; i < nr_cpu_ids; i++) {
                    printk("dpidle_cnt[%d]=%lu, rgidle_cnt[%d]=%lu\n", 
                            i, dpidle_cnt[i], i, rgidle_cnt[i]);
                } 
                
                for (i = 0; i < NR_REASONS; i++) {
                    printk("[%d]dpidle_block_cnt[%s]=%lu\n", i, reason_name[i], 
                            dpidle_block_cnt[i]);
                }

                for (i = 0; i < NR_GRPS; i++) {
                    printk("[%02d]dpidle_condition_mask[%-8s]=0x%08x\t\t"
                            "dpidle_block_mask[%-8s]=0x%08x\n", i, 
                            grp_get_name(i), dpidle_condition_mask[i],
                            grp_get_name(i), dpidle_block_mask[i]);
                }
            }
           
            dpidle_cg_block_counter = 0;
        }
        dpidle_block_cnt[reason]++;

        return false;
    } else {
        return true;
    }

}

static unsigned int g_clk_aud_intbus_sel = 0;
void spm_dpidle_before_wfi(void)
{
    int err = 0;

#if 1//def SPM_CLOCK_INIT    
    g_clk_aud_intbus_sel = clkmux_get(MT_CLKMUX_AUD_INTBUS_SEL,"Deep_Idle");
    clkmux_sel(MT_CLKMUX_AUD_INTBUS_SEL, MT_CG_SYS_26M,"Deep_Idle");

    //if(! clock_is_on(MT_CG_PMIC_SW_CG_AP) )
        enable_clock(MT_CG_PMIC_SW_CG_AP, "DEEP_IDLE");//PMIC CG bit for AP. SPM need PMIC wrapper clock to change Vcore voltage
#endif

#if ( IDLE_DPIDLE_STRESS == 0 )

#ifdef SPM_SUSPEND_GPT_EN
    free_gpt(GPT4);
    err = request_gpt(GPT4, GPT_ONE_SHOT, GPT_CLK_SRC_SYS, GPT_CLK_DIV_1, 
                0, NULL, GPT_NOAUTOEN);

    if (err) {
        idle_info("[%s]fail to request GPT4\n", __func__);
    }
#endif
    
    dpidle_timer_left2 = localtimer_get_counter();

    if( dpidle_timer_left2 <=0 )
        gpt_set_cmp(GPT4, 1);//Trigger GPT4 Timerout imediately
    else
        gpt_set_cmp(GPT4, dpidle_timer_left2);
    //gpt_set_cmp(GPT4, 52000000); // 20ms, 13MHz
    
    start_gpt(GPT4);
#endif



}

void spm_dpidle_after_wfi(void)
{
#if ( IDLE_DPIDLE_STRESS == 0 )

    //if (gpt_check_irq(GPT4)) {
    if (gpt_check_and_ack_irq(GPT4)) {
        /* waked up by WAKEUP_GPT */
        localtimer_set_next_event(1);
    } else {
        /* waked up by other wakeup source */
        unsigned int cnt, cmp;
        gpt_get_cnt(GPT4, &cnt);
        gpt_get_cmp(GPT4, &cmp);
        if (unlikely(cmp < cnt)) {
            idle_err("[%s]GPT%d: counter = %10u, compare = %10u\n", __func__, 
                    GPT4 + 1, cnt, cmp);
            BUG();
        }

        localtimer_set_next_event(cmp-cnt);
        stop_gpt(GPT4);
        //GPT_ClearCount(WAKEUP_GPT);
#ifdef SPM_SUSPEND_GPT_EN
        free_gpt(GPT4);
#endif
    }
#endif
#if 1//def SPM_CLOCK_INIT 
    //if(clock_is_on(MT_CG_PMIC_SW_CG_AP))
        disable_clock(MT_CG_PMIC_SW_CG_AP, "DEEP_IDLE");  
    clkmux_sel(MT_CLKMUX_AUD_INTBUS_SEL,g_clk_aud_intbus_sel,"Deep_Idle");
#endif

    dpidle_cnt[0]++;
}



/************************************************
 * regular idle part
 ************************************************/

static void rgidle_before_wfi(int cpu)
{
}

static void rgidle_after_wfi(int cpu)
{
    rgidle_cnt[cpu]++;
}

static void noinline go_to_rgidle(int cpu)
{
    rgidle_before_wfi(cpu);

    dsb();
    __asm__ __volatile__("wfi" ::: "memory");

    rgidle_after_wfi(cpu);
}

/************************************************
 * idle task flow part
 ************************************************/

/*
 * xxidle_handler return 1 if enter and exit the low power state
 */
#ifdef SPM_MCDI_FUNC
static inline int mcidle_handler(int cpu)
{
    if (idle_switch[IDLE_TYPE_MC]) {
        if (mcidle_can_enter(cpu)) {
            go_to_mcidle(cpu);
            return 1;
        }
    } 

    return 0;
}
#else
static inline int mcidle_handler(int cpu)
{
    return 0;
}
#endif

static int dpidle_cpu_pdn = 1;
static int dpidle_cpu_pwrlevel =0;//level1

#if ( IDLE_DPIDLE_STRESS == 1)
extern u32 cpu_power_down_cnt;
extern S32 pwrap_read( U32  adr, U32 *rdata );
#endif

extern u32 gpt_0xf0008000;
extern u32 gpt_0xf0008004;
extern u32 gpt_0xf0008008;
extern u32 gpt_0xf0008040;
extern u32 gpt_0xf0008044;
extern u32 gpt_0xf0008048;
extern u32 gpt_0xf000804c;
extern u32 gpt_0xf0008010;
extern u32 gpt_0xf0008014;
extern u32 gpt_0xf0008018;
extern u32 gpt_0xf000801c;
extern u32 spm_deepidle_wdt_val;
extern SPM_PCM_CONFIG pcm_config_dpidle;
int dpidle_handler(int cpu)
{
    int ret = 0;
#if ( IDLE_DPIDLE_STRESS == 1)    
    int rdata = 0;
    unsigned long flags;
#endif

    if (idle_switch[IDLE_TYPE_DP]) {
        if (dpidle_can_enter()) {
#if ( IDLE_DPIDLE_STRESS == 0)

            spm_go_to_dpidle(dpidle_cpu_pdn, dpidle_cpu_pwrlevel);
            printk("deepidle %s\n",spm_get_wake_up_result(SPM_PCM_DEEP_IDLE));   
            //printk("cpu_pdn_cnt %d\n",cpu_power_down_cnt); 
#if 1
        printk("timer_left=%d, timer_left2=%d, delta=%d\n", 
            dpidle_timer_left, dpidle_timer_left2, dpidle_timer_left-dpidle_timer_left2);
        printk("0xf0008000=0x%x, 0xf0008004=0x%x, 0xf0008008=0x%x, 0xf0008040=0x%x, 0xf0008044=0x%x, 0xf0008048=0x%x, 0xf000804c=0x%x, \
            0xf0008010=0x%x, 0xf0008014=0x%x, 0xf0008018=0x%x, 0xf000801c=0x%x\n, dpidle_wdt_val : %d", \
            gpt_0xf0008000, gpt_0xf0008004, gpt_0xf0008008,gpt_0xf0008040,gpt_0xf0008044, gpt_0xf0008048, gpt_0xf000804c,gpt_0xf0008010,
            gpt_0xf0008014,gpt_0xf0008018,gpt_0xf000801c,spm_deepidle_wdt_val);


#endif            
            ret = 1;
#else
            preempt_disable();
            local_irq_save(flags);
            mtk_wdt_suspend();

#if 1//dpidle stress

            for(;;)
            {
                spm_go_to_dpidle(dpidle_cpu_pdn, dpidle_cpu_pwrlevel);
                printk("deepidle %s\n",spm_get_wake_up_result(SPM_PCM_DEEP_IDLE));   
                printk("cpu_pdn_cnt %d\n",cpu_power_down_cnt); 
                ret = 1;
            }
#else//pmic wrapper stress

            //axi switch to 26MHz
            spm_write(0xf0000000,(spm_read(0xf0000000)&0xffffff1f)|0x20);
            //ahb:apb[1:1]
            spm_write(0xf000002c,spm_read(0xf000002c)&0x7fffffff);

            for(;;)
            {

                
                spm_write(0xf0006604,(spm_read(0xf0006604)&0xfffffff8)|0x2);//1.05v
                pwrap_read(0x0220, &rdata);
                if(rdata!=0x38)
                {
                    printk("pmic vcore voltage check 1.05v fail\n");
                    break;
                }
                spm_write(0xf0006604,(spm_read(0xf0006604)&0xfffffff8)|0x1);//1.15v
                pwrap_read(0x0220, &rdata);
                if(rdata!=0x48)
                {
                    printk("pmic vcore voltage check 1.15v fail\n");
                    break;
                }
                
            }
            while(1);
#endif

#endif
        }
        
    }

    return ret;
}

static inline int rgidle_handler(int cpu)
{
    int ret = 0;
    if (idle_switch[IDLE_TYPE_RG]) {
        go_to_rgidle(cpu);
        ret = 1;
    }

    return ret;
}

static int (*idle_handlers[NR_TYPES])(int) = {
    mcidle_handler,
    dpidle_handler,
    rgidle_handler,
};


void arch_idle(void)
{
    int cpu = smp_processor_id();
    int i;

    for (i = 0; i < NR_TYPES; i++) {
        if (idle_handlers[i](cpu))
            break;
    }
}

#define idle_attr(_name)                         \
static struct kobj_attribute _name##_attr = {   \
    .attr = {                                   \
        .name = __stringify(_name),             \
        .mode = 0644,                           \
    },                                          \
    .show = _name##_show,                       \
    .store = _name##_store,                     \
}

extern struct kobject *power_kobj;

#ifdef SPM_MCDI_FUNC
static ssize_t mcidle_state_show(struct kobject *kobj, 
                struct kobj_attribute *attr, char *buf)
{
    int len = 0;
    char *p = buf;

    int cpus, reason, i;

    p += sprintf(p, "*********** multi-core idle state ************\n");
    p += sprintf(p, "mcidle_time_critera=%u\n", mcidle_time_critera);

    for (cpus = 0; cpus < nr_cpu_ids; cpus++) {
        p += sprintf(p, "cpu:%d\n", cpus);
        for (reason = 0; reason < NR_REASONS; reason++) {
            p += sprintf(p, "[%d]mcidle_block_cnt[%s]=%lu\n", reason, 
                    reason_name[reason], mcidle_block_cnt[cpus][reason]);
        }
        p += sprintf(p, "\n");
    }

    for (i = 0; i < NR_GRPS; i++) {
        p += sprintf(p, "[%02d]mcidle_condition_mask[%-8s]=0x%08x\t\t"
                "mcidle_block_mask[%08x]=0x%08x\n", i, 
                grp_get_name(i), mcidle_condition_mask[i],
                grp_get_name(i), mcidle_block_mask[i]);
    }

    p += sprintf(p, "\n********** mcidle command help **********\n");
    p += sprintf(p, "mcidle help:   cat /sys/power/mcidle_state\n");
    p += sprintf(p, "switch on/off: echo [mcidle] 1/0 > /sys/power/mcidle_state\n");
    p += sprintf(p, "en_mc_by_bit:  echo enable id > /sys/power/mcidle_state\n");
    p += sprintf(p, "dis_mc_by_bit: echo disable id > /sys/power/mcidle_state\n");
    p += sprintf(p, "modify tm_cri: echo time value(dec) > /sys/power/mcidle_state\n");

    len = p - buf;
    return len;
}

static ssize_t mcidle_state_store(struct kobject *kobj, 
                struct kobj_attribute *attr, const char *buf, size_t n)
{
    char cmd[32];
    int param;

    if (sscanf(buf, "%s %d", cmd, &param) == 2) {
        if (!strcmp(cmd, "mcdle")) {
            idle_switch[IDLE_TYPE_MC] = param;
        } else if (!strcmp(cmd, "enable")) {
            enable_mcidle_by_bit(param);
        } else if (!strcmp(cmd, "disable")) {
            disable_mcidle_by_bit(param);
        } else if (!strcmp(cmd, "time")) {
            mcidle_time_critera = param;
        }
        return n;
    } else if (sscanf(buf, "%d", &param) == 1) {
        idle_switch[IDLE_TYPE_MC] = param;
        return n;
    }

    return -EINVAL;
}
idle_attr(mcidle_state);
#endif

static ssize_t dpidle_state_show(struct kobject *kobj, 
                struct kobj_attribute *attr, char *buf)
{
    int len = 0;
    char *p = buf;

    int i;

    p += sprintf(p, "*********** deep idle state ************\n");
    p += sprintf(p, "dpidle_cpu_pdn = %d\n", dpidle_cpu_pdn);
    p += sprintf(p, "dpidle_time_critera=%u\n", dpidle_time_critera);

    for (i = 0; i < NR_REASONS; i++) {
        p += sprintf(p, "[%d]dpidle_block_cnt[%s]=%lu\n", i, reason_name[i], 
                dpidle_block_cnt[i]);
    }

    p += sprintf(p, "\n");

    for (i = 0; i < NR_GRPS; i++) {
        p += sprintf(p, "[%02d]dpidle_condition_mask[%-8s]=0x%08x\t\t"
                "dpidle_block_mask[%-8s]=0x%08x\n", i, 
                grp_get_name(i), dpidle_condition_mask[i],
                grp_get_name(i), dpidle_block_mask[i]);
    }

    p += sprintf(p, "\n*********** dpidle command help  ************\n");
    p += sprintf(p, "dpidle help:   cat /sys/power/dpidle_state\n");
    p += sprintf(p, "switch on/off: echo [dpidle] 1/0 > /sys/power/dpidle_state\n");
    p += sprintf(p, "cpupdn on/off: echo cpupdn 1/0 > /sys/power/dpidle_state\n");
    p += sprintf(p, "en_dp_by_bit:  echo enable id > /sys/power/dpidle_state\n");
    p += sprintf(p, "dis_dp_by_bit: echo disable id > /sys/power/dpidle_state\n");
    p += sprintf(p, "modify tm_cri: echo time value(dec) > /sys/power/dpidle_state\n");

    len = p - buf;
    return len;
}


static ssize_t dpidle_state_store(struct kobject *kobj, 
                struct kobj_attribute *attr, const char *buf, size_t n)
{
    char cmd[32];
    int param;

    if (sscanf(buf, "%s %d", cmd, &param) == 2) {
        if (!strcmp(cmd, "dpidle")) {
            idle_switch[IDLE_TYPE_DP] = param;
        } else if (!strcmp(cmd, "enable")) {
            enable_dpidle_by_bit(param);
        } else if (!strcmp(cmd, "disable")) {
            disable_dpidle_by_bit(param);
        } else if (!strcmp(cmd, "cpupdn")) {
            dpidle_cpu_pdn = !!param;
        } else if (!strcmp(cmd, "time")) {
            dpidle_time_critera = param;
        } else if (!strcmp(cmd, "bypass")) {
             memset(dpidle_condition_mask, 0, NR_GRPS * sizeof(unsigned int));
        }        
        return n;
    } else if (sscanf(buf, "%d", &param) == 1) {
        idle_switch[IDLE_TYPE_DP] = param;
        return n;
    }

    return -EINVAL;
}
idle_attr(dpidle_state);


static ssize_t rgidle_state_show(struct kobject *kobj, 
                struct kobj_attribute *attr, char *buf)
{
    int len = 0;
    char *p = buf;

    p += sprintf(p, "*********** regular idle state ************\n");
    p += sprintf(p, "\n********** rgidle command help **********\n");
    p += sprintf(p, "rgidle help:   cat /sys/power/rgidle_state\n");
    p += sprintf(p, "switch on/off: echo [rgidle] 1/0 > /sys/power/rgidle_state\n");

    len = p - buf;
    return len;
}

static ssize_t rgidle_state_store(struct kobject *kobj, 
                struct kobj_attribute *attr, const char *buf, size_t n)
{
    char cmd[32];
    int param;

    if (sscanf(buf, "%s %d", cmd, &param) == 2) {
        if (!strcmp(cmd, "rgidle")) {
            idle_switch[IDLE_TYPE_RG] = param;
        }
        return n;
    } else if (sscanf(buf, "%d", &param) == 1) {
        idle_switch[IDLE_TYPE_RG] = param;
        return n;
    }

    return -EINVAL;
}
idle_attr(rgidle_state);

static ssize_t idle_state_show(struct kobject *kobj, 
                struct kobj_attribute *attr, char *buf)
{
    int len = 0;
    char *p = buf;
    
    int i;

    p += sprintf(p, "********** idle state dump **********\n");
#ifdef SPM_MCDI_FUNC
    for (i = 0; i < nr_cpu_ids; i++) {
        p += sprintf(p, "mcidle_cnt[%d]=%lu, dpidle_cnt[%d]=%lu, "
                "rgidle_cnt[%d]=%lu\n", 
                i, mcidle_cnt[i], i, dpidle_cnt[i], 
                i, rgidle_cnt[i]);
    }
#else
    for (i = 0; i < nr_cpu_ids; i++) {
        p += sprintf(p, "dpidle_cnt[%d]=%lu, rgidle_cnt[%d]=%lu\n", 
                i, dpidle_cnt[i], i, rgidle_cnt[i]);
    }
#endif
    
    p += sprintf(p, "\n********** variables dump **********\n");
    for (i = 0; i < NR_TYPES; i++) {
        p += sprintf(p, "%s_switch=%d, ", idle_name[i], idle_switch[i]);
    }
    p += sprintf(p, "\n");

    p += sprintf(p, "\n********** idle command help **********\n");
    p += sprintf(p, "status help:   cat /sys/power/idle_state\n");
    p += sprintf(p, "switch on/off: echo switch mask > /sys/power/idle_state\n");

#ifdef SPM_MCDI_FUNC
    p += sprintf(p, "mcidle help:   cat /sys/power/mcidle_state\n");
#else
    p += sprintf(p, "mcidle help:   mcidle is unavailable\n");
#endif
    p += sprintf(p, "dpidle help:   cat /sys/power/dpidle_state\n");
    p += sprintf(p, "rgidle help:   cat /sys/power/rgidle_state\n");

    len = p - buf;
    return len;
}

static ssize_t idle_state_store(struct kobject *kobj, 
                struct kobj_attribute *attr, const char *buf, size_t n)
{
    char cmd[32];
    int idx;
    int param;

    if (sscanf(buf, "%s %x", cmd, &param) == 2) {
        if (!strcmp(cmd, "switch")) {
            for (idx = 0; idx < NR_TYPES; idx++) {
#ifndef SPM_MCDI_FUNC
                if (idx == IDLE_TYPE_MC) {
                    continue;
                }
#endif
                idle_switch[idx] = (param & (1U << idx)) ? 1 : 0;
            }
        }
        return n;
    }

    return -EINVAL;
}
idle_attr(idle_state);


bool idle_state_get(u8 idx)
{  
    return idle_switch[idx];
}

bool idle_state_en(u8 idx, bool en)
{
    if(idx >= NR_TYPES)
        return FALSE;
    idle_switch[idx] = en;
    return false;
}

void mt_idle_init(void)
{
    int err = 0;

    idle_info("[%s]entry!!\n", __func__);
    arm_pm_idle = arch_idle;

#ifndef SPM_MCDI_FUNC
    idle_switch[IDLE_TYPE_MC] = 0;
#endif

#ifndef SPM_SUSPEND_GPT_EN    
    err = request_gpt(GPT4, GPT_ONE_SHOT, GPT_CLK_SRC_SYS, GPT_CLK_DIV_1, 
                0, NULL, GPT_NOAUTOEN);
    if (err) {
        idle_info("[%s]fail to request GPT4\n", __func__);
    }
#endif

    err = sysfs_create_file(power_kobj, &idle_state_attr.attr);
#ifdef SPM_MCDI_FUNC
    err |= sysfs_create_file(power_kobj, &mcidle_state_attr.attr);
#endif
    err |= sysfs_create_file(power_kobj, &dpidle_state_attr.attr);
    err |= sysfs_create_file(power_kobj, &rgidle_state_attr.attr);

    if (err) {
        idle_err("[%s]: fail to create sysfs\n", __func__);
    }
#if ( IDLE_DPIDLE_STRESS == 1)    
    spm_write(0xf000008c,0x2);//disable pmic wrapper dcm
    spm_write(0xf000002c,spm_read(0xf000002c)|0x20000000);//enable pmic wrapper dcm diable trigger bus clock div /1
    
#endif
}
