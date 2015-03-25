#include <xhci.h>
#include <linux/xhci/xhci-mtk.h>
#include <linux/xhci/xhci-mtk-power.h>
#include <linux/xhci/xhci-mtk-scheduler.h>
#include <linux/mu3phy/mtk-phy.h>
#include <linux/mu3phy/mtk-phy-c60802.h>
#include <linux/kernel.h>       /* printk() */
#include <linux/slab.h>
#include <linux/delay.h>
#include <asm/uaccess.h>
#include <linux/dma-mapping.h>
#include <linux/platform_device.h>
#ifdef CONFIG_USB_MTK_DUALMODE
#include <mach/eint.h>
#include <linux/switch.h>
#include <linux/sched.h>
#include <linux/module.h>
#endif

#define mtk_xhci_mtk_log(fmt, args...) \
    printk("%s(%d): " fmt, __func__, __LINE__, ##args)

#ifdef CONFIG_USB_MTK_DUALMODE

#define IDPIN_IN MT_EINT_POL_NEG
#define IDPIN_OUT MT_EINT_POL_POS

static bool mtk_id_nxt_state = IDPIN_IN;
static struct switch_dev mtk_otg_state;

static struct delayed_work mtk_xhci_delay_work;
int mtk_iddig_debounce = 400;
module_param(mtk_iddig_debounce, int, 0644);


static bool mtk_set_iddig_out_detect(){
    mt_eint_set_polarity(IDDIG_EINT_PIN, MT_EINT_POL_POS);
	mt_eint_unmask(IDDIG_EINT_PIN);
}

static bool mtk_set_iddig_in_detect(){
    mt_eint_set_polarity(IDDIG_EINT_PIN, MT_EINT_POL_NEG);
	mt_eint_unmask(IDDIG_EINT_PIN);
}

static void xhci_mode_switch(){
    bool cur_id_state = mtk_id_nxt_state;

    if(cur_id_state == IDPIN_IN){
        /* open port power and switch resource to host */
        mtk_switch2host();
        /* expect next isr is for id-pin out action */
        mtk_id_nxt_state = IDPIN_OUT;
        switch_set_state(&mtk_otg_state, 1);

        /* make id pin to detect the plug-out */
        mtk_set_iddig_out_detect();

    }
    else{ /* IDPIN_OUT */
        mtk_switch2device(false);
            /* expect next isr is for id-pin in action */
        mtk_id_nxt_state = IDPIN_IN;
        switch_set_state(&mtk_otg_state, 0);

        /* make id pin to detect the plug-in */
        mtk_set_iddig_in_detect();
    }

    mtk_xhci_mtk_log("xhci switch resource to %s\n", (cur_id_state == IDPIN_IN)? "host": "device");
}

static void xhci_eint_iddig_isr(void){
    mtk_xhci_mtk_log("schedule to delayed work\n");
    schedule_delayed_work(&mtk_xhci_delay_work, mtk_iddig_debounce*HZ/1000);
}

void mtk_xhci_eint_iddig_init(void){
	mt_eint_set_sens(IDDIG_EINT_PIN, MT_LEVEL_SENSITIVE);
	mt_eint_set_hw_debounce(IDDIG_EINT_PIN,64);
	mt_eint_registration(IDDIG_EINT_PIN, EINTF_TRIGGER_LOW, xhci_eint_iddig_isr, false);

    mtk_set_iddig_in_detect();
    mtk_xhci_mtk_log("external iddig register done.\n");

    mtk_otg_state.name = "otg_state";
	mtk_otg_state.index = 0;
	mtk_otg_state.state = 0;

	if(switch_dev_register(&mtk_otg_state))
		mtk_xhci_mtk_log("switch_dev_register fail\n");
	else
        mtk_xhci_mtk_log("switch_dev register success\n");

    INIT_DELAYED_WORK(&mtk_xhci_delay_work, xhci_mode_switch);
}

#endif

void mtk_xhci_ck_timer_init(){
	__u32 __iomem *addr;
	u32 temp;
	int num_u3_port;

    num_u3_port = SSUSB_U3_PORT_NUM(readl((void __iomem *)SSUSB_IP_CAP));
    if(num_u3_port ){
    	//set MAC reference clock speed
    	addr = SSUSB_U3_MAC_BASE+U3_UX_EXIT_LFPS_TIMING_PAR;
    	temp = readl(addr);
    	temp &= ~(0xff << U3_RX_UX_EXIT_LFPS_REF_OFFSET);
    	temp |= (U3_RX_UX_EXIT_LFPS_REF << U3_RX_UX_EXIT_LFPS_REF_OFFSET);
    	writel(temp, addr);
    	addr = SSUSB_U3_MAC_BASE+U3_REF_CK_PAR;
    	temp = readl(addr);
    	temp &= ~(0xff);
    	temp |= U3_REF_CK_VAL;
    	writel(temp, addr);

    	//set SYS_CK
    	addr = SSUSB_U3_SYS_BASE+U3_TIMING_PULSE_CTRL;
    	temp = readl(addr);
    	temp &= ~(0xff);
    	temp |= CNT_1US_VALUE;
    	writel(temp, addr);
    }

	addr = SSUSB_U2_SYS_BASE+USB20_TIMING_PARAMETER;
	temp &= ~(0xff);
	temp |= TIME_VALUE_1US;
	writel(temp, addr);

    if(num_u3_port){
    	//set LINK_PM_TIMER=3
    	addr = SSUSB_U3_SYS_BASE+LINK_PM_TIMER;
    	temp = readl(addr);
    	temp &= ~(0xf);
    	temp |= PM_LC_TIMEOUT_VALUE;
    	writel(temp, addr);
    }
}

static void setLatchSel(void){
	__u32 __iomem *latch_sel_addr;
	u32 latch_sel_value;
	int num_u3_port;

    num_u3_port = SSUSB_U3_PORT_NUM(readl((void __iomem *)SSUSB_IP_CAP));
    if(num_u3_port <= 0)
        return;

	latch_sel_addr = U3_PIPE_LATCH_SEL_ADD;
	latch_sel_value = ((U3_PIPE_LATCH_TX)<<2) | (U3_PIPE_LATCH_RX);
	writel(latch_sel_value, latch_sel_addr);
}

#define RET_SUCCESS 0
#define RET_FAIL 1

static int mtk_xhci_phy_init(int argc, char**argv)
{
	int ret;
	ret;

	/* initialize PHY related data structure */
	if (!u3phy_ops)
		ret = u3phy_init();

    /* USB 2.0 slew rate calibration */
	if(u3phy_ops->u2_slew_rate_calibration)
		u3phy_ops->u2_slew_rate_calibration(u3phy);
	else
		printk(KERN_ERR "WARN: PHY doesn't implement u2 slew rate calibration function\n");

    /* phy initialization */
	if(u3phy_ops->init(u3phy) != PHY_TRUE)
	    return RET_FAIL;

    printk(KERN_ERR "phy registers and operations initial done\n");
    return RET_SUCCESS;
}

void mtk_xhci_ip_init(void){
	__u32 __iomem *ip_reset_addr;
	u32 ip_reset_value;

    /* phy initialization is done by device, if target runs on dual mode */
    #if defined(CONFIG_MTK_XHCI) && !defined(CONFIG_USB_MTK_DUALMODE)
    mtk_xhci_phy_init(0, NULL);
    #endif

    /* reset ip, power on host and power on/enable ports */
    #if defined(CONFIG_MTK_XHCI) && !defined(CONFIG_USB_MTK_DUALMODE)
	enableAllClockPower(1); /* host do reset ip */
    #else
    enableAllClockPower(0); /* device do reset ip */
    #endif
	setLatchSel();
    mtk_xhci_ck_timer_init();
	mtk_xhci_scheduler_init();
}

int mtk_xhci_get_port_num(){
    return SSUSB_U3_PORT_NUM(readl((void __iomem *)SSUSB_IP_CAP))
        + SSUSB_U2_PORT_NUM(readl((void __iomem *)SSUSB_IP_CAP));
}

