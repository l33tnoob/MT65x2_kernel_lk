#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/delay.h>    //udelay

#include <mach/mt_typedefs.h>
#include <mach/mt_spm.h>
#include <mach/mt_spm_mtcmos.h>
#include <mach/mt_clkmgr.h> // TODO: add for BIT()

//
// CONFIG
//
#define CONFIG_CLKMGR_EMULATION
// #define CONFIG_CLKMGR_SHOWLOG

//
// LOG
//
#define TAG     "[Power/mtcmos] "

#define FUNC_LV_API         BIT(0)
#define FUNC_LV_LOCKED      BIT(1)
#define FUNC_LV_BODY        BIT(2)
#define FUNC_LV_OP          BIT(3)
#define FUNC_LV_REG_ACCESS  BIT(4)
#define FUNC_LV_DONT_CARE   BIT(5)

#define FUNC_LV_MASK        (FUNC_LV_API | FUNC_LV_LOCKED | FUNC_LV_BODY | FUNC_LV_OP | FUNC_LV_REG_ACCESS | FUNC_LV_DONT_CARE)


#if defined(CONFIG_CLKMGR_SHOWLOG)
#define ENTER_FUNC(lv)      do { if (lv & FUNC_LV_MASK) xlog_printk(ANDROID_LOG_WARN, TAG, ">> %s()\n", __FUNCTION__); } while(0)
#define EXIT_FUNC(lv)       do { if (lv & FUNC_LV_MASK) xlog_printk(ANDROID_LOG_WARN, TAG, "<< %s():%d\n", __FUNCTION__, __LINE__); } while(0)
#else
#define ENTER_FUNC(lv)
#define EXIT_FUNC(lv)
#endif // defined(CONFIG_CLKMGR_SHOWLOG)

//
// Register access function
//

#if defined(CONFIG_CLKMGR_SHOWLOG)

#if defined(CONFIG_CLKMGR_EMULATION)
#define spm_read(addr) \
    ((FUNC_LV_REG_ACCESS & FUNC_LV_MASK) ? xlog_printk(ANDROID_LOG_WARN, TAG, "spm_read(0x%08x) @ %s():%d\n", (addr), __FUNCTION__, __LINE__) : 0, 0)

#define spm_write(addr, val)   \
    do { if (FUNC_LV_REG_ACCESS & FUNC_LV_MASK) xlog_printk(ANDROID_LOG_WARN, TAG, "spm_write(0x%08x, 0x%08x) @ %s():%d\n", (addr), (val), __FUNCTION__, __LINE__); } while(0)
#else
#define spm_read(addr)          ((FUNC_LV_REG_ACCESS & FUNC_LV_MASK) ? xlog_printk(ANDROID_LOG_WARN, TAG, "spm_read(0x%08x) @ %s():%d\n", (addr), __FUNCTION__, __LINE__) : 0, (*(volatile U32 *)(addr)))
#define spm_write(addr, val)    do { if (FUNC_LV_REG_ACCESS & FUNC_LV_MASK) xlog_printk(ANDROID_LOG_WARN, TAG, "spm_write(0x%08x, 0x%08x) @ %s():%d\n", (addr), (val), __FUNCTION__, __LINE__); mt65xx_reg_sync_writel((val), (addr)); } while(0)
#endif // defined(CONFIG_CLKMGR_EMULATION)

#else

#ifndef spm_read
#define spm_read(addr)          (*(volatile U32 *)(addr))
#endif
#ifndef spm_write
#define spm_write(addr, val)    mt65xx_reg_sync_writel(val, addr)
#endif

#endif

/**************************************
 * for CPU MTCMOS
 **************************************/
/*
 * regiser bit difinition
 */
/* SPM_FC1_PWR_CON */
#define SRAM_ISOINT_B   (1U << 6)
#define SRAM_CKISO      (1U << 5)
#define PWR_CLK_DIS     (1U << 4)
#define PWR_ON_S        (1U << 3)
#define PWR_ON_         (1U << 2)
#define PWR_ISO         (1U << 1)
#define PWR_RST_B       (1U << 0)

/* SPM_PWR_STATUS */
/* SPM_PWR_STATUS_S */
#define FC1             (1U << 11)

/* SPM_SLEEP_TIMER_STA */
#define APMCU1_SLEEP    (1U << 16)

/* SPM_CPU_FC1_L1_PDN */
#define FC1_L1_PDN      (1U << 0)
#define FC1_L1_PDN_ACK  (1U << 8)

static DEFINE_SPINLOCK(spm_cpu_lock);


void spm_mtcmos_cpu_lock(unsigned long *flags)
{
    spin_lock_irqsave(&spm_cpu_lock, *flags);
}

void spm_mtcmos_cpu_unlock(unsigned long *flags)
{
    spin_unlock_irqrestore(&spm_cpu_lock, *flags);
}

int spm_mtcmos_ctrl_cpu0(int state, int chkWfiBeforePdn)
{
    if (state == STA_POWER_DOWN) {

    } else {    /* STA_POWER_ON */

    }

    return 0;
}

int spm_mtcmos_ctrl_cpu1(int state, int chkWfiBeforePdn)
{
    unsigned long flags;

    /* enable register control */
    spm_write(SPM_POWERON_CONFIG_SET, (SPM_PROJECT_CODE << 16) | (1U << 0));

    spm_mtcmos_cpu_lock(&flags);

    if (state == STA_POWER_DOWN)
    {
        if (chkWfiBeforePdn)
        {
            while ((spm_read(SPM_SLEEP_TIMER_STA) & APMCU1_SLEEP) == 0);
        }

        spm_write(SPM_FC1_PWR_CON, spm_read(SPM_FC1_PWR_CON) | PWR_CLK_DIS);
        spm_write(SPM_FC1_PWR_CON, spm_read(SPM_FC1_PWR_CON) | SRAM_CKISO);
        spm_write(SPM_FC1_PWR_CON, spm_read(SPM_FC1_PWR_CON) | PWR_ISO);
        spm_write(SPM_FC1_PWR_CON, spm_read(SPM_FC1_PWR_CON) & ~SRAM_ISOINT_B);
        spm_write(SPM_FC1_PWR_CON, spm_read(SPM_FC1_PWR_CON) & ~PWR_RST_B);

        spm_write(SPM_CPU_FC1_L1_PDN, spm_read(SPM_CPU_FC1_L1_PDN) | FC1_L1_PDN);
        while ((spm_read(SPM_CPU_FC1_L1_PDN) & FC1_L1_PDN_ACK) == 0);

        spm_write(SPM_FC1_PWR_CON, spm_read(SPM_FC1_PWR_CON) & ~PWR_ON_);
        udelay(1);
        spm_write(SPM_FC1_PWR_CON, spm_read(SPM_FC1_PWR_CON) & ~PWR_ON_S);
        udelay(1);
        while (((spm_read(SPM_PWR_STATUS) & FC1) != 0) | ((spm_read(SPM_PWR_STATUS_S) & FC1) != 0));
    }
    else /* STA_POWER_ON */
    {
        spm_write(SPM_FC1_PWR_CON, spm_read(SPM_FC1_PWR_CON) | PWR_ON_);
        udelay(1);
        spm_write(SPM_FC1_PWR_CON, spm_read(SPM_FC1_PWR_CON) | PWR_ON_S);
        udelay(3);
        while (((spm_read(SPM_PWR_STATUS) & FC1) != FC1) | ((spm_read(SPM_PWR_STATUS_S) & FC1) != FC1));

        spm_write(SPM_CPU_FC1_L1_PDN, spm_read(SPM_CPU_FC1_L1_PDN) & ~FC1_L1_PDN);
        udelay(1);
        while ((spm_read(SPM_CPU_FC1_L1_PDN) & FC1_L1_PDN_ACK) != 0);

        spm_write(SPM_FC1_PWR_CON, spm_read(SPM_FC1_PWR_CON) | SRAM_ISOINT_B);
        spm_write(SPM_FC1_PWR_CON, spm_read(SPM_FC1_PWR_CON) & ~PWR_CLK_DIS);
        spm_write(SPM_FC1_PWR_CON, spm_read(SPM_FC1_PWR_CON) & ~PWR_ISO);
        spm_write(SPM_FC1_PWR_CON, spm_read(SPM_FC1_PWR_CON) | PWR_CLK_DIS);
        spm_write(SPM_FC1_PWR_CON, spm_read(SPM_FC1_PWR_CON) & ~SRAM_CKISO);
        spm_write(SPM_FC1_PWR_CON, spm_read(SPM_FC1_PWR_CON) | PWR_RST_B);
        spm_write(SPM_FC1_PWR_CON, spm_read(SPM_FC1_PWR_CON) & ~PWR_CLK_DIS);
    }

    spm_mtcmos_cpu_unlock(&flags);

    //print_mtcmos_trace_info_for_met(); // XXX: for MET

    return 0;
}

int spm_mtcmos_ctrl_dbg(int state)
{
    if (state == STA_POWER_DOWN) {

    } else {    /* STA_POWER_ON */

    }

    return 0;
}

int spm_mtcmos_ctrl_cpusys(int state)
{
    if (state == STA_POWER_DOWN) {

    } else {    /* STA_POWER_ON */

    }

    return 0;
}

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

/**************************************
 * for non-CPU MTCMOS
 **************************************/

/**
* @file    mt_spm_mtcmos.c
* @brief   Driver for SPM MTCMOS
*
*/

#define __MT_SPM_MTCMOS_C__

/*=============================================================*/
// Include files
/*=============================================================*/

// system includes
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/delay.h>

// project includes
#include <mach/mt_typedefs.h>

// local includes
#include <mach/mt_spm_mtcmos.h>
#include <mach/mt_clkmgr.h> // XXX: for BITMASK()
// #include <mach/mt_spm_mtcmos_internal.h>
// #include <mach/mt_clkmgr_internal.h>

// forward references


/*=============================================================*/
// Macro definition
/*=============================================================*/

/*=============================================================*/
// Local type definition
/*=============================================================*/

/*=============================================================*/
// Local variable definition
/*=============================================================*/

/*=============================================================*/
// Local function definition
/*=============================================================*/

/*=============================================================*/
// Gobal function definition
/*=============================================================*/

/**************************************
 * for non-CPU MTCMOS
 **************************************/
unsigned long noncpu_flags;
static DEFINE_SPINLOCK(spm_noncpu_lock);

#if 0
void spm_mtcmos_noncpu_lock(unsigned long *flags)
{
    spin_lock_irqsave(&spm_noncpu_lock, *flags);
}

void spm_mtcmos_noncpu_unlock(unsigned long *flags)
{
    spin_unlock_irqrestore(&spm_noncpu_lock, *flags);
}
#else
static inline void spm_mtcmos_noncpu_lock(void)
{
    spin_lock_irqsave(&spm_noncpu_lock, noncpu_flags);
}

static inline void spm_mtcmos_noncpu_unlock(void)
{
    spin_unlock_irqrestore(&spm_noncpu_lock, noncpu_flags);
}
#endif

#if 0
// TODO: review it

#define DIS_SRAM_ACK        (0xf << 12)
#define MFG_SRAM_ACK        (0x1 << 12)

#define MFG_PROT_MASK       0x0020
#define MD1_PROT_MASK       0x5300
#define MD2_PROT_MASK       0xAC00

#define MFG_SI0_MASK        0x0400


int spm_mtcmos_ctrl_general(subsys_id id,
                            int state,
                            const unsigned int spm_pwr_sta_mask,
                            const unsigned int spm_pwr_con,
                            const unsigned int spm_sram_pdn,
                            const unsigned int spm_sram_pdn_ack,
                            const unsigned int spm_bus_prot_mask
                            )
{
    int i;
    int err = 0;
    volatile unsigned int val;

    ENTER_FUNC(FUNC_LV_API);

#if defined(CONFIG_CLKMGR_EMULATION)
#else

    spm_mtcmos_noncpu_lock();

    if (state == STA_POWER_DOWN)
    {
        // BUS_PROTECT                                                      // enable BUS protect
        if (0 != spm_bus_prot_mask)
        {
            spm_write(INFRA_TOPAXI_PROTECTEN, spm_read(INFRA_TOPAXI_PROTECTEN) | spm_bus_prot_mask);
            while ((spm_read(INFRA_TOPAXI_PROTECTSTA1) & spm_bus_prot_mask) != spm_bus_prot_mask);
        }

        spm_write(spm_pwr_con, spm_read(spm_pwr_con) | PWR_CLK_DIS_BIT);    // PWR_CLK_DIS = 1
        spm_write(spm_pwr_con, spm_read(spm_pwr_con) | PWR_ISO_BIT);        // PWR_ISO = 1
        spm_write(spm_pwr_con, spm_read(spm_pwr_con) & ~PWR_RST_B_BIT);     // PWR_RST_B = 0

        // SRAM_PDN                                                         // MEM power off
        for (i = BIT(8); i <= spm_sram_pdn; i = (i << 1) + BIT(8))          // set SRAM_PDN 1 one by one
        {
            spm_write(spm_pwr_con, spm_read(spm_pwr_con) | i);
        }

        while (   spm_sram_pdn_ack
               && ((spm_read(spm_pwr_con) & spm_sram_pdn_ack) != spm_sram_pdn_ack) // wait until SRAM_PDN_ACK all 1
               );

        spm_write(spm_pwr_con, spm_read(spm_pwr_con) & ~(PWR_ON_BIT|PWR_ON_S_BIT)); // PWR_ON_ = 0 & PWR_ON_S = 0
        udelay(1);                                                          // delay 1 us

        while (   (spm_read(SPM_PWR_STATUS)   & spm_pwr_sta_mask)           // wait until PWR_ACK = 0
               || (spm_read(SPM_PWR_STATUS_S) & spm_pwr_sta_mask)
               );
    }
    else /* STA_POWER_ON */
    {
        spm_write(spm_pwr_con, spm_read(spm_pwr_con) | PWR_ON_BIT);         // PWR_ON_ = 1
        udelay(1);                                                          // delay 1 us
        spm_write(spm_pwr_con, spm_read(spm_pwr_con) | PWR_ON_S_BIT);       // PWR_ON_S = 1
        udelay(1);                                                          // delay 1 us

        while (   !(spm_read(SPM_PWR_STATUS)   & spm_pwr_sta_mask)          // wait until PWR_ACK = 1
               || !(spm_read(SPM_PWR_STATUS_S) & spm_pwr_sta_mask)
               );

        // SRAM_PDN                                                         // MEM power on
        for (i = BIT(8); i <= spm_sram_pdn; i = (i << 1) + BIT(8))          // set SRAM_PDN 0 one by one
        {
            spm_write(spm_pwr_con, spm_read(spm_pwr_con) & ~i);
        }

        while (   spm_sram_pdn_ack
               && (spm_read(spm_pwr_con) & spm_sram_pdn_ack)                // wait until SRAM_PDN_ACK all 0
               );

        spm_write(spm_pwr_con, spm_read(spm_pwr_con) & ~PWR_CLK_DIS_BIT);   // PWR_CLK_DIS = 0
        spm_write(spm_pwr_con, spm_read(spm_pwr_con) & ~PWR_ISO_BIT);       // PWR_ISO = 0
        spm_write(spm_pwr_con, spm_read(spm_pwr_con) | PWR_CLK_DIS_BIT);    // PWR_CLK_DIS = 1
        spm_write(spm_pwr_con, spm_read(spm_pwr_con) | PWR_RST_B_BIT);      // PWR_RST_B = 1
        spm_write(spm_pwr_con, spm_read(spm_pwr_con) & ~PWR_CLK_DIS_BIT);   // PWR_CLK_DIS = 0

        // BUS_PROTECT
        if (0 != spm_bus_prot_mask)
        {
            spm_write(INFRA_TOPAXI_PROTECTEN, spm_read(INFRA_TOPAXI_PROTECTEN) & ~spm_bus_prot_mask);
            while (spm_read(INFRA_TOPAXI_PROTECTSTA1) & spm_bus_prot_mask);
        }
    }

    spm_mtcmos_noncpu_unlock();

#endif // defined(CONFIG_CLKMGR_EMULATION)

    EXIT_FUNC(FUNC_LV_API);

    return err;
}

int spm_mtcmos_ctrl_mfg(int state)
{
    int err = 0;
    volatile unsigned int val;

    ENTER_FUNC(FUNC_LV_API);

#if defined(CONFIG_CLKMGR_EMULATION)
#else

    spm_mtcmos_noncpu_lock();

    if (state == STA_POWER_DOWN)
    {
        #if 0   // TODO: FIXME no bus protect
        spm_write(INFRA_TOPAXI_PROTECTEN, spm_read(INFRA_TOPAXI_PROTECTEN) | MFG_PROT_MASK);

        while ((spm_read(INFRA_TOPAXI_PROTECTSTA1) & MFG_PROT_MASK) != MFG_PROT_MASK)
        {
        }

        spm_write(TOPAXI_SI0_CTL, spm_read(TOPAXI_SI0_CTL) & ~MFG_SI0_MASK); // TODO: review it
        #endif  // TODO: FIXME no bus protect

        spm_write(SPM_MFG_PWR_CON, spm_read(SPM_MFG_PWR_CON) | SRAM_PDN_BITS);

        while ((spm_read(SPM_MFG_PWR_CON) & MFG_SRAM_ACK) != MFG_SRAM_ACK)
        {
        }

        spm_write(SPM_MFG_PWR_CON, spm_read(SPM_MFG_PWR_CON) | PWR_ISO_BIT);

        val = spm_read(SPM_MFG_PWR_CON);
        val = (val & ~PWR_RST_B_BIT) | PWR_CLK_DIS_BIT;
        spm_write(SPM_MFG_PWR_CON, val);

        spm_write(SPM_MFG_PWR_CON, spm_read(SPM_MFG_PWR_CON) & ~(PWR_ON_BIT | PWR_ON_S_BIT));

#if 0
        udelay(1);

        if (spm_read(SPM_PWR_STATUS) & MFG_PWR_STA_MASK)
        {
            err = 1;
        }

#else

        while ((spm_read(SPM_PWR_STATUS) & MFG_PWR_STA_MASK)
               || (spm_read(SPM_PWR_STATUS_S) & MFG_PWR_STA_MASK))
        {
        }

#endif
    }
    else        /* STA_POWER_ON */
    {
        spm_write(SPM_MFG_PWR_CON, spm_read(SPM_MFG_PWR_CON) | PWR_ON_BIT);
        spm_write(SPM_MFG_PWR_CON, spm_read(SPM_MFG_PWR_CON) | PWR_ON_S_BIT);
#if 0
        udelay(1);
#else

        while (!(spm_read(SPM_PWR_STATUS) & MFG_PWR_STA_MASK) ||
               !(spm_read(SPM_PWR_STATUS_S) & MFG_PWR_STA_MASK))
        {
        }

#endif

        spm_write(SPM_MFG_PWR_CON, spm_read(SPM_MFG_PWR_CON) & ~PWR_CLK_DIS_BIT);
        spm_write(SPM_MFG_PWR_CON, spm_read(SPM_MFG_PWR_CON) & ~PWR_ISO_BIT);
        spm_write(SPM_MFG_PWR_CON, spm_read(SPM_MFG_PWR_CON) | PWR_RST_B_BIT);

        spm_write(SPM_MFG_PWR_CON, spm_read(SPM_MFG_PWR_CON) & ~SRAM_PDN_BITS);

        while ((spm_read(SPM_MFG_PWR_CON) & MFG_SRAM_ACK))
        {
        }

#if 0
        udelay(1);

        if (!(spm_read(SPM_PWR_STATUS) & MFG_PWR_STA_MASK))
        {
            err = 1;
        }

#endif
        spm_write(INFRA_TOPAXI_PROTECTEN, spm_read(INFRA_TOPAXI_PROTECTEN) & ~MFG_PROT_MASK);

        while (spm_read(INFRA_TOPAXI_PROTECTSTA1) & MFG_PROT_MASK)
        {
        }

        spm_write(TOPAXI_SI0_CTL, spm_read(TOPAXI_SI0_CTL) | MFG_SI0_MASK); // TODO: review it
    }

    spm_mtcmos_noncpu_unlock();

#endif // defined(CONFIG_CLKMGR_EMULATION)

    EXIT_FUNC(FUNC_LV_API);

    return err;
}

int spm_mtcmos_ctrl_disp(int state)
{
    int err = 0;
    volatile unsigned int val;

    ENTER_FUNC(FUNC_LV_API);

#if defined(CONFIG_CLKMGR_EMULATION)
#else

    spm_mtcmos_noncpu_lock();

    if (state == STA_POWER_DOWN)
    {
        spm_write(SPM_DIS_PWR_CON, spm_read(SPM_DIS_PWR_CON) | SRAM_PDN_BITS);
#if 0

        while ((spm_read(SPM_DIS_PWR_CON) & DIS_SRAM_ACK) != DIS_SRAM_ACK)
        {
        }

#endif
        spm_write(SPM_DIS_PWR_CON, spm_read(SPM_DIS_PWR_CON) | PWR_ISO_BIT);

        val = spm_read(SPM_DIS_PWR_CON);
        val = (val & ~PWR_RST_B_BIT) | PWR_CLK_DIS_BIT;
        spm_write(SPM_DIS_PWR_CON, val);

        spm_write(SPM_DIS_PWR_CON, spm_read(SPM_DIS_PWR_CON) & ~(PWR_ON_BIT | PWR_ON_S_BIT));

#if 0
        udelay(1);

        if (spm_read(SPM_PWR_STATUS) & DIS_PWR_STA_MASK)
        {
            err = 1;
        }

#else

        while ((spm_read(SPM_PWR_STATUS) & DIS_PWR_STA_MASK)
               || (spm_read(SPM_PWR_STATUS_S) & DIS_PWR_STA_MASK))
        {
        }

#endif
    }
    else        /* STA_POWER_ON */
    {
        spm_write(SPM_DIS_PWR_CON, spm_read(SPM_DIS_PWR_CON) | PWR_ON_BIT);
        spm_write(SPM_DIS_PWR_CON, spm_read(SPM_DIS_PWR_CON) | PWR_ON_S_BIT);
#if 0
        udelay(1);
#else

        while (!(spm_read(SPM_PWR_STATUS) & DIS_PWR_STA_MASK)
               || !(spm_read(SPM_PWR_STATUS_S) & DIS_PWR_STA_MASK))
        {
        }

#endif
        spm_write(SPM_DIS_PWR_CON, spm_read(SPM_DIS_PWR_CON) & ~PWR_CLK_DIS_BIT);
        spm_write(SPM_DIS_PWR_CON, spm_read(SPM_DIS_PWR_CON) & ~PWR_ISO_BIT);
        spm_write(SPM_DIS_PWR_CON, spm_read(SPM_DIS_PWR_CON) | PWR_RST_B_BIT);

        spm_write(SPM_DIS_PWR_CON, spm_read(SPM_DIS_PWR_CON) & ~SRAM_PDN_BITS);

#if 0

        while ((spm_read(SPM_DIS_PWR_CON) & DIS_SRAM_ACK))
        {
        }

#endif

#if 0
        udelay(1);

        if (!(spm_read(SPM_PWR_STATUS) & DIS_PWR_STA_MASK))
        {
            err = 1;
        }

#endif
    }

    spm_mtcmos_noncpu_unlock();

#endif // defined(CONFIG_CLKMGR_EMULATION)

    EXIT_FUNC(FUNC_LV_API);

    return err;
}

int spm_mtcmos_ctrl_mdsys1(int state)
{
    int err = 0;
    volatile unsigned int val;

    ENTER_FUNC(FUNC_LV_API);

#if defined(CONFIG_CLKMGR_EMULATION)
#else

    spm_mtcmos_noncpu_lock();

    if (state == STA_POWER_DOWN)
    {
        #if 0   // TODO: FIXME no bus protect
        spm_write(INFRA_TOPAXI_PROTECTEN, spm_read(INFRA_TOPAXI_PROTECTEN) | MD1_PROT_MASK);

        while ((spm_read(INFRA_TOPAXI_PROTECTSTA1) & MD1_PROT_MASK) != MD1_PROT_MASK)
        {
        }
        #endif  // TODO: FIXME no bus protect

        spm_write(SPM_MD_PWR_CON, spm_read(SPM_MD_PWR_CON) | MD_SRAM_PDN_BIT);

        spm_write(SPM_MD_PWR_CON, spm_read(SPM_MD_PWR_CON) | PWR_ISO_BIT);

        val = spm_read(SPM_MD_PWR_CON);
        val = (val & ~PWR_RST_B_BIT) | PWR_CLK_DIS_BIT;
        spm_write(SPM_MD_PWR_CON, val);

        spm_write(SPM_MD_PWR_CON, spm_read(SPM_MD_PWR_CON) & ~(PWR_ON_BIT | PWR_ON_S_BIT));

#if 0
        udelay(1);

        if (spm_read(SPM_PWR_STATUS) & MD1_PWR_STA_MASK)
        {
            err = 1;
        }

#else

        while ((spm_read(SPM_PWR_STATUS) & MD1_PWR_STA_MASK)
               || (spm_read(SPM_PWR_STATUS_S) & MD1_PWR_STA_MASK))
        {
        }

#endif
    }
    else        /* STA_POWER_ON */
    {
        spm_write(SPM_MD_PWR_CON, spm_read(SPM_MD_PWR_CON) | PWR_ON_BIT);
        spm_write(SPM_MD_PWR_CON, spm_read(SPM_MD_PWR_CON) | PWR_ON_S_BIT);
#if 0
        udelay(1);
#else

        while (!(spm_read(SPM_PWR_STATUS) & MD1_PWR_STA_MASK)
               || !(spm_read(SPM_PWR_STATUS_S) & MD1_PWR_STA_MASK))
        {
        }

#endif

        spm_write(SPM_MD_PWR_CON, spm_read(SPM_MD_PWR_CON) & ~PWR_CLK_DIS_BIT);
        spm_write(SPM_MD_PWR_CON, spm_read(SPM_MD_PWR_CON) & ~PWR_ISO_BIT);
        spm_write(SPM_MD_PWR_CON, spm_read(SPM_MD_PWR_CON) | PWR_RST_B_BIT);

        spm_write(SPM_MD_PWR_CON, spm_read(SPM_MD_PWR_CON) & ~MD_SRAM_PDN_BIT);

#if 0
        udelay(1);

        if (!(spm_read(SPM_PWR_STATUS) & MD1_PWR_STA_MASK))
        {
            err = 1;
        }

#endif
        spm_write(INFRA_TOPAXI_PROTECTEN, spm_read(INFRA_TOPAXI_PROTECTEN) & ~MD1_PROT_MASK);

        while (spm_read(INFRA_TOPAXI_PROTECTSTA1) & MD1_PROT_MASK)
        {
        }
    }

    spm_mtcmos_noncpu_unlock();

#endif // defined(CONFIG_CLKMGR_EMULATION)

    EXIT_FUNC(FUNC_LV_API);

    return err;
}

int spm_mtcmos_ctrl_connsys(int state)
{
    ENTER_FUNC(FUNC_LV_API);

    // TODO: ////// UNDER CONSTRUCT //////

    EXIT_FUNC(FUNC_LV_API);

    return 0;
}
#endif

/**
 *test_spm_gpu_power_on - test whether gpu could be powered on
 *
 *Returns 1 if power on operation succeed, 0 otherwise.
 */
int test_spm_gpu_power_on(void)
{
    int i;
    volatile unsigned int sta1, sta2;
    volatile unsigned int val;

    sta1 = spm_read(SPM_PWR_STATUS);
    sta2 = spm_read(SPM_PWR_STATUS_S);

    if (((sta1 & MFG_PWR_STA_MASK) == MFG_PWR_STA_MASK) && ((sta2 & MFG_PWR_STA_MASK) == MFG_PWR_STA_MASK))
    {
        printk("[%s]: test_spm_gpu_power_on already on, return: 1.\n", __func__);
        return 1;
    }

    spm_mtcmos_noncpu_lock();

    val = spm_read(SPM_MFG_PWR_CON);
    BUG_ON(!(val & PWR_ISO_BIT));

    for (i = 0; i < 5; i++)
    {

        spm_write(SPM_MFG_PWR_CON, spm_read(SPM_MFG_PWR_CON) | PWR_ON_BIT);
        spm_write(SPM_MFG_PWR_CON, spm_read(SPM_MFG_PWR_CON) | PWR_ON_S_BIT);

        udelay(5);

        sta1 = spm_read(SPM_PWR_STATUS);
        sta2 = spm_read(SPM_PWR_STATUS_S);

        if (((sta1 & MFG_PWR_STA_MASK) != MFG_PWR_STA_MASK) || ((sta2 & MFG_PWR_STA_MASK) != MFG_PWR_STA_MASK))
        {
            spm_mtcmos_noncpu_unlock();
            printk("[%s]: test_spm_gpu_power_on return: 0.\n", __func__);
            return 0;
        }

        spm_write(SPM_MFG_PWR_CON, spm_read(SPM_MFG_PWR_CON) & ~(PWR_ON_BIT | PWR_ON_S_BIT));

        sta1 = spm_read(SPM_PWR_STATUS);
        sta2 = spm_read(SPM_PWR_STATUS_S);

        if (((sta1 & MFG_PWR_STA_MASK) == MFG_PWR_STA_MASK) || ((sta2 & MFG_PWR_STA_MASK) == MFG_PWR_STA_MASK))
        {
            spm_mtcmos_noncpu_unlock();
            printk("[%s]: test_spm_gpu_power_on return: 0.\n", __func__);
            return 0;
        }

        mdelay(1);
    }

    spm_mtcmos_noncpu_unlock();

    printk("[%s]: test_spm_gpu_power_on return: 1.\n", __func__);
    return 1;
}
