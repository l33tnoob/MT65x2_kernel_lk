#ifndef __MTK_PTP_H__
#define __MTK_PTP_H__

#include <mach/sync_write.h>
#include <linux/xlog.h>

#define PTP_GIC_SIGNALS         (32)
#define PTP_FSM_IRQ_ID          (PTP_GIC_SIGNALS + 30)
#define PTP_THERM_IRQ_ID        (PTP_GIC_SIGNALS + 25)
#define MAX_OPP_NUM             (8)
#define STANDARD_FREQ           (1000000)  // khz
#define PTP_LEVEL_INDEX         (18)
#define PTP_LEVEL_0             (0x3)
#define PTP_LEVEL_1             (0x2)
#define PTP_LEVEL_2             (0x1)

/* ====== 6572 PTP register ====== */
#define PTP_base_addr           (0xf100D000)
#define PTP_ctr_reg_addr        (PTP_base_addr+0x200)

#define PTP_DESCHAR             (PTP_ctr_reg_addr)
#define PTP_TEMPCHAR            (PTP_ctr_reg_addr+0x04)
#define PTP_DETCHAR             (PTP_ctr_reg_addr+0x08)
#define PTP_AGECHAR             (PTP_ctr_reg_addr+0x0c)

#define PTP_DCCONFIG            (PTP_ctr_reg_addr+0x10)
#define PTP_AGECONFIG           (PTP_ctr_reg_addr+0x14)
#define PTP_FREQPCT30           (PTP_ctr_reg_addr+0x18)
#define PTP_FREQPCT74           (PTP_ctr_reg_addr+0x1c)

#define PTP_LIMITVALS           (PTP_ctr_reg_addr+0x20)
#define PTP_VBOOT               (PTP_ctr_reg_addr+0x24)
#define PTP_DETWINDOW           (PTP_ctr_reg_addr+0x28)
#define PTP_PTPCONFIG           (PTP_ctr_reg_addr+0x2c)

#define PTP_TSCALCS             (PTP_ctr_reg_addr+0x30)
#define PTP_RUNCONFIG           (PTP_ctr_reg_addr+0x34)
#define PTP_PTPEN               (PTP_ctr_reg_addr+0x38)
#define PTP_INIT2VALS           (PTP_ctr_reg_addr+0x3c)

#define PTP_DCVALUES            (PTP_ctr_reg_addr+0x40)
#define PTP_AGEVALUES           (PTP_ctr_reg_addr+0x44)
#define PTP_VOP30               (PTP_ctr_reg_addr+0x48)
#define PTP_VOP74               (PTP_ctr_reg_addr+0x4c)

#define PTP_TEMP                (PTP_ctr_reg_addr+0x50)
#define PTP_PTPINTSTS           (PTP_ctr_reg_addr+0x54)
#define PTP_PTPINTSTSRAW        (PTP_ctr_reg_addr+0x58)
#define PTP_PTPINTEN            (PTP_ctr_reg_addr+0x5c)
#define PTP_AGECOUNT            (PTP_ctr_reg_addr+0x7c)
#define PTP_SMSTATE0            (PTP_ctr_reg_addr+0x80)
#define PTP_SMSTATE1            (PTP_ctr_reg_addr+0x84)

/* ====== Thermal Controller register ======= */
#define PTP_Thermal_ctr_reg_addr (PTP_base_addr)

#define ptp_print(fmt, args...) xlog_printk(ANDROID_LOG_INFO, "PTPOD", fmt, ##args)
#define ptp_isr_print(fmt, args...) printk(KERN_DEBUG "[PTPOD] " fmt, ##args)

#define ptp_read(addr)		    (*(volatile u32 *)(addr))
#define ptp_write(addr, val)	mt65xx_reg_sync_writel(val, addr)

typedef struct {
    unsigned int ADC_CALI_EN;
    unsigned int PTPINITEN;
    unsigned int PTPMONEN;
    
    unsigned int MDES;
    unsigned int BDES;
    unsigned int DCCONFIG;
    unsigned int DCMDET;
    unsigned int DCBDET;
    unsigned int AGECONFIG;
    unsigned int AGEM;
    unsigned int AGEDELTA;
    unsigned int DVTFIXED;
    unsigned int VCO;
    unsigned int MTDES;
    unsigned int MTS;
    unsigned int BTS;

    char FREQPCT[MAX_OPP_NUM];

    unsigned int DETWINDOW;
    unsigned int VMAX;
    unsigned int VMIN;
    unsigned int DTHI;
    unsigned int DTLO;
    unsigned int VBOOT;
    unsigned int DETMAX;

    unsigned int DCVOFFSETIN;
    unsigned int AGEVOFFSETIN;
} PTP_Init_T;

enum init_type{
    INIT1_MODE = 0,
    INIT2_MODE,
    MONITOR_MODE,
};

void init_PTP_interrupt(void);
void init_PTP_Therm_interrupt(void);
void ptp_init1(void);

#if defined (CONFIG_PTP)
unsigned int get_ptp_level(void) ;
#else  //#if defined (CONFIG_PTP)
#include "devinfo.h"
#define get_ptp_level()                                         \
        ({                                                      \
                unsigned int ver;                               \
                ver = get_devinfo_with_index(PTP_LEVEL_INDEX);  \
                ver = (ver >> 8) & 0x00000003;                  \
                ver;                                            \
        }) 
#endif //#if defined (CONFIG_PTP)

#endif /* __MTK_PTP_H__  */
