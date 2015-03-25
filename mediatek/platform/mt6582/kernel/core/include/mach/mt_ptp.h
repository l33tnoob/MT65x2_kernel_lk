#ifndef _MT_PTP_
#define _MT_PTP_

#include <linux/kernel.h>
#include <mach/sync_write.h>

/* PTP Register Definition */
#define PTP_BASEADDR        (0xF100B000)
#define PTP_DESCHAR         (PTP_BASEADDR + 0x200)
#define PTP_TEMPCHAR        (PTP_BASEADDR + 0x204)
#define PTP_DETCHAR         (PTP_BASEADDR + 0x208)
#define PTP_AGECHAR         (PTP_BASEADDR + 0x20C)
#define PTP_DCCONFIG        (PTP_BASEADDR + 0x210)
#define PTP_AGECONFIG       (PTP_BASEADDR + 0x214)
#define PTP_FREQPCT30       (PTP_BASEADDR + 0x218)
#define PTP_FREQPCT74       (PTP_BASEADDR + 0x21C)
#define PTP_LIMITVALS       (PTP_BASEADDR + 0x220)
#define PTP_VBOOT           (PTP_BASEADDR + 0x224)
#define PTP_DETWINDOW       (PTP_BASEADDR + 0x228)
#define PTP_PTPCONFIG       (PTP_BASEADDR + 0x22C)
#define PTP_TSCALCS         (PTP_BASEADDR + 0x230)
#define PTP_RUNCONFIG       (PTP_BASEADDR + 0x234)
#define PTP_PTPEN           (PTP_BASEADDR + 0x238)
#define PTP_INIT2VALS       (PTP_BASEADDR + 0x23C)
#define PTP_DCVALUES        (PTP_BASEADDR + 0x240)
#define PTP_AGEVALUES       (PTP_BASEADDR + 0x244)
#define PTP_VOP30           (PTP_BASEADDR + 0x248)
#define PTP_VOP74           (PTP_BASEADDR + 0x24C)
#define PTP_TEMP            (PTP_BASEADDR + 0x250)
#define PTP_PTPINTSTS       (PTP_BASEADDR + 0x254)
#define PTP_PTPINTSTSRAW    (PTP_BASEADDR + 0x258)
#define PTP_PTPINTEN        (PTP_BASEADDR + 0x25C)
#define PTP_PTPCHKSHIFT     (PTP_BASEADDR + 0x264)
#define PTP_SMSTATE0        (PTP_BASEADDR + 0x280)
#define PTP_SMSTATE1        (PTP_BASEADDR + 0x284)

/* Thermal Register Definition */
#define PTP_TEMPMONCTL0         (THERMAL_BASE + 0x000)
#define PTP_TEMPMONCTL1         (THERMAL_BASE + 0x004)
#define PTP_TEMPMONCTL2         (THERMAL_BASE + 0x008)
#define PTP_TEMPMONINT          (THERMAL_BASE + 0x00C)
#define PTP_TEMPMONINTSTS       (THERMAL_BASE + 0x010)
#define PTP_TEMPMONIDET0        (THERMAL_BASE + 0x014)
#define PTP_TEMPMONIDET1        (THERMAL_BASE + 0x018)
#define PTP_TEMPMONIDET2        (THERMAL_BASE + 0x01C)
#define PTP_TEMPH2NTHRE         (THERMAL_BASE + 0x024)
#define PTP_TEMPHTHRE           (THERMAL_BASE + 0x028)
#define PTP_TEMPCTHRE           (THERMAL_BASE + 0x02C)
#define PTP_TEMPOFFSETH         (THERMAL_BASE + 0x030)
#define PTP_TEMPOFFSETL         (THERMAL_BASE + 0x034)
#define PTP_TEMPMSRCTL0         (THERMAL_BASE + 0x038)
#define PTP_TEMPMSRCTL1         (THERMAL_BASE + 0x03C)
#define PTP_TEMPAHBPOLL         (THERMAL_BASE + 0x040)
#define PTP_TEMPAHBTO           (THERMAL_BASE + 0x044)
#define PTP_TEMPADCPNP0         (THERMAL_BASE + 0x048)
#define PTP_TEMPADCPNP1         (THERMAL_BASE + 0x04C)
#define PTP_TEMPADCPNP2         (THERMAL_BASE + 0x050)
#define PTP_TEMPADCMUX          (THERMAL_BASE + 0x054)
#define PTP_TEMPADCEXT          (THERMAL_BASE + 0x058)
#define PTP_TEMPADCEXT1         (THERMAL_BASE + 0x05C)
#define PTP_TEMPADCEN           (THERMAL_BASE + 0x060)
#define PTP_TEMPPNPMUXADDR      (THERMAL_BASE + 0x064)
#define PTP_TEMPADCMUXADDR      (THERMAL_BASE + 0x068)
#define PTP_TEMPADCEXTADDR      (THERMAL_BASE + 0x06C)
#define PTP_TEMPADCEXT1ADDR     (THERMAL_BASE + 0x070)
#define PTP_TEMPADCENADDR       (THERMAL_BASE + 0x074)
#define PTP_TEMPADCVALIDADDR    (THERMAL_BASE + 0x078)
#define PTP_TEMPADCVOLTADDR     (THERMAL_BASE + 0x07C)
#define PTP_TEMPRDCTRL          (THERMAL_BASE + 0x080)
#define PTP_TEMPADCVALIDMASK    (THERMAL_BASE + 0x084)
#define PTP_TEMPADCVOLTAGESHIFT (THERMAL_BASE + 0x088)
#define PTP_TEMPADCWRITECTRL    (THERMAL_BASE + 0x08C)
#define PTP_TEMPMSR0            (THERMAL_BASE + 0x090)
#define PTP_TEMPMSR1            (THERMAL_BASE + 0x094)
#define PTP_TEMPMSR2            (THERMAL_BASE + 0x098)
#define PTP_TEMPIMMD0           (THERMAL_BASE + 0x0A0)
#define PTP_TEMPIMMD1           (THERMAL_BASE + 0x0A4)
#define PTP_TEMPIMMD2           (THERMAL_BASE + 0x0A8)
#define PTP_TEMPPROTCTL         (THERMAL_BASE + 0x0C0)
#define PTP_TEMPPROTTA          (THERMAL_BASE + 0x0C4)
#define PTP_TEMPPROTTB          (THERMAL_BASE + 0x0C8)
#define PTP_TEMPPROTTC          (THERMAL_BASE + 0x0CC)
#define PTP_TEMPSPARE0          (THERMAL_BASE + 0x0F0)
#define PTP_TEMPSPARE1          (THERMAL_BASE + 0x0F4)
#define PTP_TEMPSPARE2          (THERMAL_BASE + 0x0F8)
#define PTP_TEMPSPARE3          (THERMAL_BASE + 0x0FC)

/* PTP Macro Definition */
#define EN_PTP_OD                   (1)
#define EN_ISR_LOG                  (0)

#define PTP_GET_REAL_VAL            (1)
#define SET_PMIC_VOLT               (1)

#define ptp_read(addr)              (*(volatile u32 *)(addr))
#define ptp_write(addr, val)        mt65xx_reg_sync_writel(val, addr)

#define ptp_emerg(fmt, args...)     printk(KERN_EMERG "[PTP] " fmt, ##args)
#define ptp_alert(fmt, args...)     printk(KERN_ALERT "[PTP] " fmt, ##args)
#define ptp_crit(fmt, args...)      printk(KERN_CRIT "[PTP] " fmt, ##args)
#define ptp_error(fmt, args...)     printk(KERN_ERR "[PTP] " fmt, ##args)
#define ptp_warning(fmt, args...)   printk(KERN_WARNING "[PTP] " fmt, ##args)
#define ptp_notice(fmt, args...)    printk(KERN_NOTICE "[PTP] " fmt, ##args)
#define ptp_info(fmt, args...)      printk(KERN_INFO "[PTP] " fmt, ##args)
#define ptp_debug(fmt, args...)     printk(KERN_DEBUG "[PTP] " fmt, ##args)

#if EN_ISR_LOG
#define ptp_isr_info(fmt, args...)  ptp_notice( fmt, ##args)
#else
#define ptp_isr_info(fmt, args...)  ptp_debug( fmt, ##args)
#endif

/* PTP Structure */
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

    unsigned char FREQPCT0;
    unsigned char FREQPCT1;
    unsigned char FREQPCT2;
    unsigned char FREQPCT3;
    unsigned char FREQPCT4;
    unsigned char FREQPCT5;
    unsigned char FREQPCT6;
    unsigned char FREQPCT7;

    unsigned int DETWINDOW;
    unsigned int VMAX;
    unsigned int VMIN;
    unsigned int DTHI;
    unsigned int DTLO;
    unsigned int VBOOT;
    unsigned int DETMAX;

    unsigned int PTPCHKSHIFT;

    unsigned int DCVOFFSETIN;
    unsigned int AGEVOFFSETIN;
} PTP_INIT_T;

/* PTP Extern Function */
extern unsigned int PTP_get_ptp_level(void);

#endif