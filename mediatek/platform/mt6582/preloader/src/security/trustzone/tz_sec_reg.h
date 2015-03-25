
#ifndef _SEC_REG_H_
#define _SEC_REG_H_

/* I/O mapping */
#define IO_PHYS            	    0x10000000
#define SRAMROM_BASE        	(IO_PHYS + 0x00202000)
#define INFRACFG_AO_BASE        (IO_PHYS + 0x00001000)
#define SPM_BASE                (IO_PHYS + 0x00006000)

#define BOOTROM_PWR_CTRL        (INFRACFG_AO_BASE + 0x804)

#define SRAMROM_SEC_DOMAIN      (SRAMROM_BASE + 0x00)

#define SRAMROM_SEC_SET0        (0x7 << 0)
#define SRAMROM_SEC_SET1        (0x7 << 8)
#define SRAMROM_SEC_SET2        (0x7 << 16)

#define PERMIT_S_RW_NS_RW       (0x0)
#define PERMIT_S_RW_NS_BLOCK    (0x1)
#define PERMIT_S_RW_NS_RO       (0x2)
#define PERMIT_S_RW_NS_WO       (0x3)
#define PERMIT_S_RO_NS_RO       (0x4)
#define PERMIT_S_BLOCK_NS_BLOCK (0x5)

#define GIC_CPU_CTRL            0x00
#define GIC_CPU_PRIMASK         0x04
#define GIC_CPU_BINPOINT        0x08
#define GIC_CPU_INTACK          0x0c
#define GIC_CPU_EOI             0x10
#define GIC_CPU_RUNNINGPRI      0x14
#define GIC_CPU_HIGHPRI         0x18

#define SPM_POWERON_CONFIG_SET  (SPM_BASE + 0x0000)
#define SPM_SLEEP_TIMER_STA     (SPM_BASE + 0x0720)
#define SPM_FC1_PWR_CON         (SPM_BASE + 0x0218)
#define SPM_FC2_PWR_CON         (SPM_BASE + 0x021c)
#define SPM_FC3_PWR_CON         (SPM_BASE + 0x0220)

#define SPM_CPU_FC0_L1_PDN      (SPM_BASE + 0x025c)
#define SPM_CPU_FC1_L1_PDN      (SPM_BASE + 0x0264)
#define SPM_CPU_FC2_L1_PDN      (SPM_BASE + 0x026c)
#define SPM_CPU_FC3_L1_PDN      (SPM_BASE + 0x0274)

#define SPM_PWR_STATUS          (SPM_BASE + 0x060c)
#define SPM_PWR_STATUS_S        (SPM_BASE + 0x0610)

#define PWR_RST_B               (0x1 << 0)
#define PWR_ISO                 (0x1 << 1)
#define PWR_ON                  (0x1 << 2)
#define PWR_ON_S                (0x1 << 3)
#define PWR_CLK_DIS             (0x1 << 4)

#define FC1                     (1U << 11)
#define FC2                     (1U << 10)
#define FC3                     (1U <<  9)

#define L1_PDN_ACK              (1U << 8)
#define L1_PDN                  (1U << 0)

#define SRAM_ISOINT_B           (1U << 6)
#define SRAM_CKISO              (1U << 5)
#define PWR_CLK_DIS             (1U << 4)
#define PWR_ON_S                (1U << 3)
#define PWR_ON                  (1U << 2)
#define PWR_ISO                 (1U << 1)
#define PWR_RST_B               (1U << 0)

/* SPM_SLEEP_TIMER_STA */
#define APMCU3_SLEEP            (1U << 18)
#define APMCU2_SLEEP            (1U << 17)
#define APMCU1_SLEEP            (1U << 16)

#define SPM_PROJECT_CODE        0xb16

#endif
