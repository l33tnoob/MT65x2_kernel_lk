#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/cpu.h>

#include <linux/types.h>
#include <linux/string.h>
#include <mach/mt_cirq.h>
#include <asm/system_misc.h>

#include <mach/mt_typedefs.h>
#include <mach/sync_write.h>
#include <mach/mt_clkmgr.h>
#include <mach/mt_dcm.h>
#include <mach/mt_gpt.h>
#include <mach/mt_spm_idle.h>
#include <mach/mt_spm_sleep.h>
#include <mach/hotplug.h>
#include <mach/mt_cpufreq.h>
#include <mach/mt_power_gs.h>
#include <mach/mt_ptp.h>

#define USING_XLOG

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


#define idle_readl(addr) \
    DRV_Reg32(addr)

#define idle_writel(addr, val)   \
    mt65xx_reg_sync_writel(val, addr)

#define idle_setl(addr, val) \
    mt65xx_reg_sync_writel(idle_readl(addr) | (val), addr)

#define idle_clrl(addr, val) \
    mt65xx_reg_sync_writel(idle_readl(addr) & ~(val), addr)


extern unsigned long localtimer_get_counter(void);
extern int localtimer_set_next_event(unsigned long evt);


#define INVALID_GRP_ID(grp) (grp < 0 || grp >= NR_GRPS)
bool __attribute__((weak)) 
clkmgr_idle_can_enter(unsigned int *condition_mask, unsigned int *block_mask)
{
    return false;
}

enum {
    IDLE_TYPE_MC = 0,
    IDLE_TYPE_DP = 1,
    IDLE_TYPE_SL = 2,
    IDLE_TYPE_RG = 3,
    NR_TYPES = 4,
};

enum {
    BY_CPU = 0,
    BY_CLK = 1,
    BY_TMR = 2,
    BY_OTH = 3,
    BY_VTG = 4,
    NR_REASONS = 5
};

static const char *idle_name[NR_TYPES] = {
    "mcidle",
    "dpidle",
    "slidle",
    "rgidle",
};

static const char *reason_name[NR_REASONS] = {
    "by_cpu",
    "by_clk",
    "by_tmr",
    "by_oth",
    "by_vtg",
};

static int idle_switch[NR_TYPES] = {
    1,  //mcidle switch 
    1,  //dpidle switch
    1,  //slidle switch
    1,  //rgidle switch
};

/************************************************
 * SODI part
 ************************************************/
#ifdef SPM_SODI_ENABLED
extern u32 gSPM_SODI_EN;
extern bool gSpm_IsLcmVideoMode;

static unsigned int mcidle_gpt_percpu[NR_CPUS] = {
    GPT4,
    GPT1,    
    GPT4,
    GPT5,
};

static unsigned int mcidle_condition_mask[NR_GRPS] = {
    0x00fe05c1, //PERI0:  
    0x00000000, //INFRA:
    0x00000000, //TOPCK:
    0x00024c04, //DISP0: 
    0x00000000, //DISP1:
    0x000003e1, //IMAGE: 
    0x00000001, //MFG:
    0x00000000, //AUDIO: 
    0x00000001, //VDEC0: 
    0x00000001, //VDEC1: 
};

static unsigned int mcidle_block_mask[NR_GRPS] = {0x0};

static unsigned int mcidle_timer_left[NR_CPUS];
static unsigned int mcidle_timer_left2[NR_CPUS];
//static unsigned int mcidle_time_critera = 26000; //2ms
static unsigned int mcidle_time_critera = 13000; //1ms


static unsigned long mcidle_cnt[NR_CPUS] = {0};
static unsigned long mcidle_block_cnt[NR_CPUS][NR_REASONS] = {{0}};

static DEFINE_MUTEX(mcidle_locked);

static void enable_mcidle_by_mask(int grp, unsigned int mask)
{
    mutex_lock(&mcidle_locked);
    mcidle_condition_mask[grp] &= ~mask;
    mutex_unlock(&mcidle_locked);
}

static void disable_mcidle_by_mask(int grp, unsigned int mask)
{
    mutex_lock(&mcidle_locked);
    mcidle_condition_mask[grp] |= mask;
    mutex_unlock(&mcidle_locked);
}

void enable_mcidle_by_bit(int id)
{
    int grp = id / 32;
    unsigned int mask = 1U << (id % 32);
    BUG_ON(INVALID_GRP_ID(grp));
    enable_mcidle_by_mask(grp, mask);
}
EXPORT_SYMBOL(enable_mcidle_by_bit);

void disable_mcidle_by_bit(int id)
{
    int grp = id / 32;
    unsigned int mask = 1U << (id % 32);
    BUG_ON(INVALID_GRP_ID(grp));
    disable_mcidle_by_mask(grp, mask);
}
EXPORT_SYMBOL(disable_mcidle_by_bit);


bool mcidle_can_enter(int cpu)
{
    int reason = NR_REASONS;

    if (TRUE == gSpm_IsLcmVideoMode) {
        reason = BY_OTH;
        goto out;
    }
        
    if (gSPM_SODI_EN != 0) {
        reason = BY_OTH;
        goto out;
    }

    /*if hotplug-ing, can't enter sodi avoid bootslave corrupt*/
    if (atomic_read(&is_in_hotplug) >= 1) {
        reason = BY_CPU;
        goto out;
    }  
    
    if (atomic_read(&hotplug_cpu_count) != 1) {
        reason = BY_CPU;
        goto out;
    }

    if (cpu == 0) {
        memset(mcidle_block_mask, 0, NR_GRPS * sizeof(unsigned int));
        if (!clkmgr_idle_can_enter(mcidle_condition_mask, mcidle_block_mask)) {
            reason = BY_CLK;
            goto out;
        }
    }

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

void mcidle_before_wfi(int cpu)
{
    int err = 0;
    unsigned int id = mcidle_gpt_percpu[cpu];
    
        free_gpt(id);
        err = request_gpt(id, GPT_ONE_SHOT, GPT_CLK_SRC_SYS, GPT_CLK_DIV_1, 
                    0, NULL, GPT_NOAUTOEN);
        if (err) {
            idle_info("[%s]fail to request GPT4\n", __func__);
        }
    
        mcidle_timer_left2[cpu] = localtimer_get_counter(); 


        if( (int)mcidle_timer_left2[cpu] <=0 )
        {
            gpt_set_cmp(id, 1);//Trigger GPT4 Timerout imediately
        }
        else    
            gpt_set_cmp(id, mcidle_timer_left2[cpu]);
        
        start_gpt(id);

}

void mcidle_after_wfi(int cpu)
{
    unsigned int id = mcidle_gpt_percpu[cpu];

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
        }

    mcidle_cnt[cpu]++;
}

#if 0
static void go_to_mcidle(int cpu)
{
    mcidle_before_wfi(cpu);

    //spm_mcdi_wfi();

    mcidle_after_wfi(cpu);
}
#endif

#endif //SPM_SODI_ENABLED

/************************************************
 * deep idle part
 ************************************************/
static unsigned int dpidle_condition_mask[NR_GRPS] = {
    0x02fe87fd, //PERI0: 
    0x0000a080, //INFRA:
    0x00000000, //TOPCK:
    0x0007ffff, //DISP0: 
    0x0000000f, //DISP1: 
    0x000003e1, //IMAGE: 
    0x00000001, //MFG:   
    0x00000000, //AUDIO: 
    0x00000001, //VDEC0: 
    0x00000001, //VDEC1:
};

static unsigned int dpidle_block_mask[NR_GRPS] = {0x0};


static unsigned int dpidle_timer_left;
static unsigned int dpidle_timer_left2;
static unsigned int dpidle_time_critera = 26000;

static unsigned long dpidle_cnt[NR_CPUS] = {0};
static unsigned long dpidle_block_cnt[NR_REASONS] = {0};

static DEFINE_MUTEX(dpidle_locked);

static void enable_dpidle_by_mask(int grp, unsigned int mask)
{
    mutex_lock(&dpidle_locked);
    dpidle_condition_mask[grp] &= ~mask;
    mutex_unlock(&dpidle_locked);
}

static void disable_dpidle_by_mask(int grp, unsigned int mask)
{
    mutex_lock(&dpidle_locked);
    dpidle_condition_mask[grp] |= mask;
    mutex_unlock(&dpidle_locked);
}

void enable_dpidle_by_bit(int id)
{
    int grp = id / 32;
    unsigned int mask = 1U << (id % 32);
    BUG_ON(INVALID_GRP_ID(grp));
    enable_dpidle_by_mask(grp, mask);
}
EXPORT_SYMBOL(enable_dpidle_by_bit);

void disable_dpidle_by_bit(int id)
{
    int grp = id / 32;
    unsigned int mask = 1U << (id % 32);
    BUG_ON(INVALID_GRP_ID(grp));
    disable_dpidle_by_mask(grp, mask);
}
EXPORT_SYMBOL(disable_dpidle_by_bit);


static bool dpidle_can_enter(void)
{
    int reason = NR_REASONS;

#ifdef SPM_SODI_ENABLED
    if (gSPM_SODI_EN != 0) {
        reason = BY_OTH;
        goto out;
    }
#endif

	if (!mt_cpufreq_earlysuspend_status_get()){
		reason = BY_VTG;
        goto out;
	}

    //if ((smp_processor_id() != 0) || (num_online_cpus() != 1)) {
    if (atomic_read(&hotplug_cpu_count) != 1) {
        reason = BY_CPU;
        goto out;
    }

    memset(dpidle_block_mask, 0, NR_GRPS * sizeof(unsigned int));
    if (!clkmgr_idle_can_enter(dpidle_condition_mask, dpidle_block_mask)) {
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
        dpidle_block_cnt[reason]++;
        return false;
    } else {		
        return true;
    }
}

static unsigned int clk_cfg_3 = 0;

#define faudintbus_pll2sq() \
do {    \
    clk_cfg_3 = idle_readl(CLK_CFG_3);\
    idle_writel(CLK_CFG_3, clk_cfg_3 & 0xF8FFFFFF);  \
} while (0);

#define faudintbus_sq2pll() \
do {    \
    idle_writel(CLK_CFG_3, clk_cfg_3);  \
} while (0);


void spm_dpidle_before_wfi(void)
{
    
    mt_power_gs_dump_dpidle();

    bus_dcm_enable();

    faudintbus_pll2sq();

#if 0
        dpidle_timer_left = localtimer_get_counter();
        gpt_set_cmp(GPT4, dpidle_timer_left);
#else
        dpidle_timer_left2 = localtimer_get_counter();
        gpt_set_cmp(GPT4, dpidle_timer_left2);
#endif
        start_gpt(GPT4);

}

void spm_dpidle_after_wfi(void)
{
#if 0
    idle_info("[%s]timer_left=%u, timer_left2=%u, delta=%u\n", 
        dpidle_timer_left, dpidle_timer_left2, dpidle_timer_left-dpidle_timer_left2);
#endif

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
    }

    faudintbus_sq2pll();

    bus_dcm_disable();
    
    dpidle_cnt[0]++;
}


/************************************************
 * slow idle part
 ************************************************/
static unsigned int slidle_condition_mask[NR_GRPS] = {
    0x00f00800, //PERI0:
    0x00000000, //INFRA:
    0x00000000, //TOPCK:
    0x00000000, //DISP0:
    0x00000000, //DISP1:
    0x00000000, //IMAGE:
    0x00000000, //MFG:
    0x00000000, //AUDIO:
    0x00000000, //VDEC0:
    0x00000000, //VDEC1:
};

static unsigned int slidle_block_mask[NR_GRPS] = {0x0};

static unsigned long slidle_cnt[NR_CPUS] = {0};
static unsigned long slidle_block_cnt[NR_REASONS] = {0};

static DEFINE_MUTEX(slidle_locked);


static void enable_slidle_by_mask(int grp, unsigned int mask)
{
    mutex_lock(&slidle_locked);
    slidle_condition_mask[grp] &= ~mask;
    mutex_unlock(&slidle_locked);
}

static void disable_slidle_by_mask(int grp, unsigned int mask)
{
    mutex_lock(&slidle_locked);
    slidle_condition_mask[grp] |= mask;
    mutex_unlock(&slidle_locked);
}

void enable_slidle_by_bit(int id)
{
    int grp = id / 32;
    unsigned int mask = 1U << (id % 32);
    BUG_ON(INVALID_GRP_ID(grp));
    enable_slidle_by_mask(grp, mask);
}
EXPORT_SYMBOL(enable_slidle_by_bit);

void disable_slidle_by_bit(int id)
{
    int grp = id / 32;
    unsigned int mask = 1U << (id % 32);
    BUG_ON(INVALID_GRP_ID(grp));
    disable_slidle_by_mask(grp, mask);
}
EXPORT_SYMBOL(disable_slidle_by_bit);

#if EN_PTP_OD
extern u32 ptp_data[3]; 
#endif
static bool slidle_can_enter(void)
{
    int reason = NR_REASONS;
    //if ((smp_processor_id() != 0) || (num_online_cpus() != 1)) {
    if (atomic_read(&hotplug_cpu_count) != 1) {
        reason = BY_CPU;
        goto out;
    }

    memset(slidle_block_mask, 0, NR_GRPS * sizeof(unsigned int));
    if (!clkmgr_idle_can_enter(slidle_condition_mask, slidle_block_mask)) {
        reason = BY_CLK;
        goto out;
    }

#if EN_PTP_OD    
    if (ptp_data[0]) {
        reason = BY_OTH;
        goto out;
    }
#endif    

out:
    if (reason < NR_REASONS) {
        slidle_block_cnt[reason]++;
        return false;
    } else {
        return true;
    }
}

static void slidle_before_wfi(int cpu)
{
    bus_dcm_enable();
}

static void slidle_after_wfi(int cpu)
{
    bus_dcm_disable();
 
    slidle_cnt[cpu]++;
}

static void go_to_slidle(int cpu)
{
    slidle_before_wfi(cpu);

    dsb();
    __asm__ __volatile__("wfi" ::: "memory");

    slidle_after_wfi(cpu);
}


/************************************************
 * regular idle part
 ************************************************/
static unsigned long rgidle_cnt[NR_CPUS] = {0};

static void rgidle_before_wfi(int cpu)
{
    mt_power_gs_dump_idle();
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
#ifdef SPM_SODI_ENABLED

static int sodi_cpu_pdn = 1;
static inline int mcidle_handler(int cpu)
{
    if (idle_switch[IDLE_TYPE_MC]) {
        if (mcidle_can_enter(cpu)) {
            spm_go_to_sodi(sodi_cpu_pdn);
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

static inline int dpidle_handler(int cpu)
{
    int ret = 0;
    if (idle_switch[IDLE_TYPE_DP]) {
        if (dpidle_can_enter()) {
            spm_go_to_dpidle(dpidle_cpu_pdn, 0);
            ret = 1;
        }
    }

    return ret;
}

static inline int slidle_handler(int cpu)
{
    int ret = 0;
    if (idle_switch[IDLE_TYPE_SL]) {
        if (slidle_can_enter()) {
            go_to_slidle(cpu);
            ret = 1;
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
    dpidle_handler,
    mcidle_handler,
    slidle_handler,
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

#ifdef SPM_SODI_ENABLED
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
                "mcidle_block_mask[%-8s]=0x%08x\n", i, 
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
        if (!strcmp(cmd, "mcidle")) {
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
        }
        return n;
    } else if (sscanf(buf, "%d", &param) == 1) {
        idle_switch[IDLE_TYPE_DP] = param;
        return n;
    }

    return -EINVAL;
}
idle_attr(dpidle_state);

static ssize_t slidle_state_show(struct kobject *kobj, 
                struct kobj_attribute *attr, char *buf)
{
    int len = 0;
    char *p = buf;

    int i;

    p += sprintf(p, "*********** slow idle state ************\n");
    for (i = 0; i < NR_REASONS; i++) {
        p += sprintf(p, "[%d]slidle_block_cnt[%s]=%lu\n", 
                i, reason_name[i], slidle_block_cnt[i]);
    }

    p += sprintf(p, "\n");

    for (i = 0; i < NR_GRPS; i++) {
        p += sprintf(p, "[%02d]slidle_condition_mask[%-8s]=0x%08x\t\t"
                "slidle_block_mask[%-8s]=0x%08x\n", i, 
                grp_get_name(i), slidle_condition_mask[i],
                grp_get_name(i), slidle_block_mask[i]);
    }


    p += sprintf(p, "\n********** slidle command help **********\n");
    p += sprintf(p, "slidle help:   cat /sys/power/slidle_state\n");
    p += sprintf(p, "switch on/off: echo [slidle] 1/0 > /sys/power/slidle_state\n");

    len = p - buf;
    return len;
}

static ssize_t slidle_state_store(struct kobject *kobj, 
                struct kobj_attribute *attr, const char *buf, size_t n)
{
    char cmd[32];
    int param;

    if (sscanf(buf, "%s %d", cmd, &param) == 2) {
        if (!strcmp(cmd, "slidle")) {
            idle_switch[IDLE_TYPE_SL] = param;
        } else if (!strcmp(cmd, "enable")) {
            enable_slidle_by_bit(param);
        } else if (!strcmp(cmd, "disable")) {
            disable_slidle_by_bit(param);
        }
        return n;
    } else if (sscanf(buf, "%d", &param) == 1) {
        idle_switch[IDLE_TYPE_SL] = param;
        return n;
    }

    return -EINVAL;
}
idle_attr(slidle_state);

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
#ifdef SPM_SODI_ENABLED
    for (i = 0; i < nr_cpu_ids; i++) {
        p += sprintf(p, "mcidle_cnt[%d]=%lu, dpidle_cnt[%d]=%lu, "
                "slidle_cnt[%d]=%lu, rgidle_cnt[%d]=%lu\n", 
                i, mcidle_cnt[i], i, dpidle_cnt[i], 
                i, slidle_cnt[i], i, rgidle_cnt[i]);
    }
#else
    for (i = 0; i < nr_cpu_ids; i++) {
        p += sprintf(p, "dpidle_cnt[%d]=%lu, slidle_cnt[%d]=%lu, rgidle_cnt[%d]=%lu\n", 
                i, dpidle_cnt[i], i, slidle_cnt[i], i, rgidle_cnt[i]);
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

#ifdef SPM_SODI_ENABLED
    p += sprintf(p, "mcidle help:   cat /sys/power/mcidle_state\n");
#else
    p += sprintf(p, "mcidle help:   mcidle is unavailable\n");
#endif
    p += sprintf(p, "dpidle help:   cat /sys/power/dpidle_state\n");
    p += sprintf(p, "slidle help:   cat /sys/power/slidle_state\n");
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
#ifndef SPM_SODI_ENABLED
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


void mt_idle_init(void)
{
    int err = 0;
    
    idle_info("[%s]entry!!\n", __func__);
    arm_pm_idle = arch_idle;

#ifndef SPM_SODI_ENABLED
    idle_switch[IDLE_TYPE_MC] = 0;
#endif

    err = free_gpt(GPT1);
    if (err) {
        idle_info("[%s]fail to free GPT1\n", __func__);
    }
    
    err = request_gpt(GPT1, GPT_ONE_SHOT, GPT_CLK_SRC_SYS, GPT_CLK_DIV_1, 
                0, NULL, GPT_NOAUTOEN);
    if (err) {
        idle_info("[%s]fail to request GPT1\n", __func__);
    }

    err = request_gpt(GPT4, GPT_ONE_SHOT, GPT_CLK_SRC_SYS, GPT_CLK_DIV_1, 
                0, NULL, GPT_NOAUTOEN);
    if (err) {
        idle_info("[%s]fail to request GPT4\n", __func__);
    }

    err = request_gpt(GPT5, GPT_ONE_SHOT, GPT_CLK_SRC_SYS, GPT_CLK_DIV_1, 
                0, NULL, GPT_NOAUTOEN);
    if (err) {
        idle_info("[%s]fail to request GPT5\n", __func__);
    }

    err = sysfs_create_file(power_kobj, &idle_state_attr.attr);
#ifdef SPM_SODI_ENABLED
    err |= sysfs_create_file(power_kobj, &mcidle_state_attr.attr);
#endif
    err |= sysfs_create_file(power_kobj, &dpidle_state_attr.attr);
    err |= sysfs_create_file(power_kobj, &slidle_state_attr.attr);
    err |= sysfs_create_file(power_kobj, &rgidle_state_attr.attr);

    if (err) {
        idle_err("[%s]: fail to create sysfs\n", __func__);
    }
}
