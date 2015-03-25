/*! \file
    \brief  Declaration of library functions

    Any definitions in this file will be shared among GLUE Layer and internal Driver Stack.
*/




/*******************************************************************************
*                         C O M P I L E R   F L A G S
********************************************************************************
*/

/*******************************************************************************
*                                 M A C R O S
********************************************************************************
*/


#if CONFIG_HAS_WAKELOCK
#include <linux/wakelock.h>
#define CFG_WMT_WAKELOCK_SUPPORT 1
#endif

#ifdef DFT_TAG
#undef DFT_TAG
#endif
#define DFT_TAG         "[WMT-PLAT]"


/*******************************************************************************
*                    E X T E R N A L   R E F E R E N C E S
********************************************************************************
*/

/* ALPS header files */
#include <mach/mtk_rtc.h>
#include <mach/mt_irq.h>
#include <mach/mtk_rtc.h>
#include <mach/mt_gpio.h>
#include <mach/mtk_wcn_cmb_stub.h>

/* MTK_WCN_COMBO header files */
#include "wmt_plat.h"
#include "mtk_wcn_consys_hw.h"

/*******************************************************************************
*                              C O N S T A N T S
********************************************************************************
*/
EMI_CTRL_STATE_OFFSET mtk_wcn_emi_state_off = {
	.emi_apmem_ctrl_state = EXP_APMEM_CTRL_STATE,
	.emi_apmem_ctrl_host_sync_state = EXP_APMEM_CTRL_HOST_SYNC_STATE,
	.emi_apmem_ctrl_host_sync_num = EXP_APMEM_CTRL_HOST_SYNC_NUM,
	.emi_apmem_ctrl_chip_sync_state = EXP_APMEM_CTRL_CHIP_SYNC_STATE,
	.emi_apmem_ctrl_chip_sync_num = EXP_APMEM_CTRL_CHIP_SYNC_NUM,
	.emi_apmem_ctrl_chip_sync_addr = EXP_APMEM_CTRL_CHIP_SYNC_ADDR,
	.emi_apmem_ctrl_chip_sync_len = EXP_APMEM_CTRL_CHIP_SYNC_LEN,
	.emi_apmem_ctrl_chip_print_buff_start = EXP_APMEM_CTRL_CHIP_PRINT_BUFF_START,
	.emi_apmem_ctrl_chip_print_buff_len = EXP_APMEM_CTRL_CHIP_PRINT_BUFF_LEN,
	.emi_apmem_ctrl_chip_print_buff_idx = EXP_APMEM_CTRL_CHIP_PRINT_BUFF_IDX,
	.emi_apmem_ctrl_chip_int_status = EXP_APMEM_CTRL_CHIP_INT_STATUS,
	.emi_apmem_ctrl_chip_paded_dump_end = EXP_APMEM_CTRL_CHIP_PAGED_DUMP_END,
	.emi_apmem_ctrl_host_outband_assert_w1 = EXP_APMEM_CTRL_HOST_OUTBAND_ASSERT_W1,
};

CONSYS_EMI_ADDR_INFO mtk_wcn_emi_addr_info = {
	.emi_phy_addr = CONSYS_EMI_FW_PHY_BASE,
	.paged_trace_off = CONSYS_EMI_PAGED_TRACE_OFFSET,
	.paged_dump_off = CONSYS_EMI_PAGED_DUMP_OFFSET,
	.full_dump_off = CONSYS_EMI_FULL_DUMP_OFFSET,
	.p_emi_ctrl_state_offset = &mtk_wcn_emi_state_off,
};

/*******************************************************************************
*                             D A T A   T Y P E S
********************************************************************************
*/

/*******************************************************************************
*                  F U N C T I O N   D E C L A R A T I O N S
********************************************************************************
*/

static VOID wmt_plat_bgf_eirq_cb (VOID);

static INT32 wmt_plat_bgf_eint_ctrl (ENUM_PIN_STATE state);
static INT32 wmt_plat_i2s_ctrl (ENUM_PIN_STATE state);
static INT32 wmt_plat_gps_sync_ctrl (ENUM_PIN_STATE state);
static INT32 wmt_plat_gps_lna_ctrl (ENUM_PIN_STATE state);

static INT32 wmt_plat_dump_pin_conf (VOID);

#if CFG_WMT_DUMP_INT_STATUS
extern VOID mt_irq_dump_status(INT32 irq);
#endif

/*******************************************************************************
*                            P U B L I C   D A T A
********************************************************************************
*/
UINT32 gCoClockFlag = 0;
BGF_IRQ_BALANCE gbgfIrqBle;
UINT32 wmtPlatLogLvl = WMT_PLAT_LOG_INFO;
/*******************************************************************************
*                           P R I V A T E   D A T A
********************************************************************************
*/




#if CFG_WMT_WAKELOCK_SUPPORT
static struct mutex gOsSLock;
static struct wake_lock wmtWakeLock;
#endif



irq_cb wmt_plat_bgf_irq_cb = NULL;
device_audio_if_cb wmt_plat_audio_if_cb = NULL;
func_ctrl_cb wmt_plat_func_ctrl_cb = NULL;
thermal_query_ctrl_cb wmt_plat_thermal_query_ctrl_cb = NULL;
deep_idle_ctrl_cb wmt_plat_deep_idle_ctrl_cb = NULL;

const static fp_set_pin gfp_set_pin_table[] =
{
    [PIN_BGF_EINT] = wmt_plat_bgf_eint_ctrl,
    [PIN_I2S_GRP] = wmt_plat_i2s_ctrl,
    [PIN_GPS_SYNC] = wmt_plat_gps_sync_ctrl,
    [PIN_GPS_LNA] = wmt_plat_gps_lna_ctrl,
};

/*******************************************************************************
*                              F U N C T I O N S
********************************************************************************
*/

/*!
 * \brief audio control callback function for CMB_STUB on ALPS
 *
 * A platform function required for dynamic binding with CMB_STUB on ALPS.
 *
 * \param state desired audio interface state to use
 * \param flag audio interface control options
 *
 * \retval 0 operation success
 * \retval -1 invalid parameters
 * \retval < 0 error for operation fail
 */
INT32 wmt_plat_audio_ctrl (CMB_STUB_AIF_X state, CMB_STUB_AIF_CTRL ctrl)
{
    INT32 iRet = 0;
	UINT32 pinShare = 0;

    /* input sanity check */
    if ( (CMB_STUB_AIF_MAX <= state)
        || (CMB_STUB_AIF_CTRL_MAX <= ctrl) ) {
        return -1;
    }
	

    iRet = 0;

    /* set host side first */
    switch (state) {
    case CMB_STUB_AIF_0:
        /* BT_PCM_OFF & FM line in/out */
        iRet += wmt_plat_gpio_ctrl(PIN_I2S_GRP, PIN_STA_DEINIT);
        break;

    case CMB_STUB_AIF_1:
        iRet += wmt_plat_gpio_ctrl(PIN_I2S_GRP, PIN_STA_DEINIT);
        break;

    case CMB_STUB_AIF_2:
        iRet += wmt_plat_gpio_ctrl(PIN_I2S_GRP, PIN_STA_INIT);
        break;

    case CMB_STUB_AIF_3:
        iRet += wmt_plat_gpio_ctrl(PIN_I2S_GRP, PIN_STA_INIT);
        break;

    default:
        /* FIXME: move to cust folder? */
        WMT_PLAT_ERR_FUNC("invalid state [%d]\n", state);
        return -1;
        break;
    }
	

	if (CMB_STUB_AIF_CTRL_EN == ctrl) {
	    WMT_PLAT_INFO_FUNC("call chip aif setting \n");
	    /* need to control chip side GPIO */
		if (NULL != wmt_plat_audio_if_cb)
		{
			iRet += (*wmt_plat_audio_if_cb)(state, (pinShare) ? MTK_WCN_BOOL_TRUE : MTK_WCN_BOOL_FALSE);
		}
		else
		{
			WMT_PLAT_WARN_FUNC("wmt_plat_audio_if_cb is not registered \n");
			iRet -= 1;
		}
	        
			
	}
	else {
	    WMT_PLAT_INFO_FUNC("skip chip aif setting \n");
	}

    return iRet;

}


static VOID wmt_plat_func_ctrl (UINT32 type, UINT32 on)
{
	if(wmt_plat_func_ctrl_cb){
		(*wmt_plat_func_ctrl_cb)(on,type);
	}
    return;
}

static LONG wmt_plat_thermal_ctrl(VOID)
{
	LONG temp = 0;

	if(wmt_plat_thermal_query_ctrl_cb)
	{
		temp = (*wmt_plat_thermal_query_ctrl_cb)();
	}

	return temp;
}

static INT32 wmt_plat_deep_idle_ctrl(UINT32 dpilde_ctrl)
{
	INT32 iRet = -1;
	
	if(wmt_plat_deep_idle_ctrl_cb)
	{
		iRet = (*wmt_plat_deep_idle_ctrl_cb)(dpilde_ctrl);
	}

	return iRet;
}

static VOID
wmt_plat_bgf_eirq_cb (VOID)
{
#if CFG_WMT_PS_SUPPORT
//#error "need to disable EINT here"
    //wmt_lib_ps_irq_cb();
    if (NULL != wmt_plat_bgf_irq_cb)
    {
        (*(wmt_plat_bgf_irq_cb))();
    }
	else
	{
	    WMT_PLAT_WARN_FUNC("WMT-PLAT: wmt_plat_bgf_irq_cb not registered\n");
	}
#else
    return;
#endif

}


irqreturn_t wmt_plat_bgf_irq_isr(INT32 i,VOID *arg)
{
#if CFG_WMT_PS_SUPPORT
	wmt_plat_eirq_ctrl(PIN_BGF_EINT, PIN_STA_EINT_DIS);
	wmt_plat_bgf_eirq_cb();
#else
	WMT_PLAT_INFO_FUNC("skip irq handing becasue psm is disable");
#endif
	return IRQ_HANDLED;
}

VOID wmt_plat_irq_cb_reg (irq_cb bgf_irq_cb)
{
    wmt_plat_bgf_irq_cb = bgf_irq_cb;
}

VOID wmt_plat_aif_cb_reg (device_audio_if_cb aif_ctrl_cb)
{
    wmt_plat_audio_if_cb = aif_ctrl_cb;
}

VOID wmt_plat_func_ctrl_cb_reg(func_ctrl_cb subsys_func_ctrl)
{
	wmt_plat_func_ctrl_cb = subsys_func_ctrl;
}

VOID wmt_plat_thermal_ctrl_cb_reg(thermal_query_ctrl_cb thermal_query_ctrl)
{
	wmt_plat_thermal_query_ctrl_cb = thermal_query_ctrl;
}

VOID wmt_plat_deep_idle_ctrl_cb_reg(deep_idle_ctrl_cb deep_idle_ctrl)
{
	wmt_plat_deep_idle_ctrl_cb = deep_idle_ctrl;
}
UINT32 wmt_plat_soc_co_clock_flag_get(VOID)
{
	return gCoClockFlag;
}
static UINT32 wmt_plat_soc_co_clock_flag_set(UINT32 flag)
{
	gCoClockFlag = flag;
	return 0;
}


INT32
wmt_plat_init (UINT32 co_clock_en)
{
	CMB_STUB_CB stub_cb;
    INT32 iret;
    /*init wmt function ctrl wakelock if wake lock is supported by host platform*/
	if(co_clock_en)
	{
		wmt_plat_soc_co_clock_flag_set(1);
	}else
	{
		wmt_plat_soc_co_clock_flag_set(0);
	}

    stub_cb.aif_ctrl_cb = wmt_plat_audio_ctrl;
    stub_cb.func_ctrl_cb = wmt_plat_func_ctrl;
	stub_cb.thermal_query_cb = wmt_plat_thermal_ctrl;
	stub_cb.deep_idle_ctrl_cb = wmt_plat_deep_idle_ctrl;
    stub_cb.size = sizeof(stub_cb);

    /* register to cmb_stub */
    iret = mtk_wcn_cmb_stub_reg(&stub_cb);
    #ifdef CFG_WMT_WAKELOCK_SUPPORT
    wake_lock_init(&wmtWakeLock, WAKE_LOCK_SUSPEND, "wmtFuncCtrl");
    mutex_init(&gOsSLock);
    #endif

	iret += mtk_wcn_consys_hw_init();

	spin_lock_init(&gbgfIrqBle.lock);
    WMT_PLAT_DBG_FUNC("WMT-PLAT: ALPS platform init (%d)\n", iret);

    return 0;
}


INT32
wmt_plat_deinit (VOID)
{
	INT32 iret = 0;
    /* 2. unreg to cmb_stub */
    iret = mtk_wcn_cmb_stub_unreg();

    /*3. wmt wakelock deinit*/
    #ifdef CFG_WMT_WAKELOCK_SUPPORT
    wake_lock_destroy(&wmtWakeLock);
    mutex_destroy(&gOsSLock);
    WMT_PLAT_DBG_FUNC("destroy wmtWakeLock\n");
    #endif

	iret += mtk_wcn_consys_hw_deinit();
	
    WMT_PLAT_DBG_FUNC("WMT-PLAT: ALPS platform init (%d)\n", iret);

    return 0;
}




static INT32
wmt_plat_dump_pin_conf (VOID)
{
    WMT_PLAT_INFO_FUNC( "[WMT-PLAT]=>dump wmt pin configuration start<=\n");


    #ifdef GPIO_COMBO_BGF_EINT_PIN
        WMT_PLAT_INFO_FUNC( "BGF_EINT(GPIO%d)\n", GPIO_COMBO_BGF_EINT_PIN);
    #else
        WMT_PLAT_INFO_FUNC( "BGF_EINT(not defined)\n");
    #endif

    #ifdef CUST_EINT_COMBO_BGF_NUM
        WMT_PLAT_INFO_FUNC( "BGF_EINT_NUM(%d)\n", CUST_EINT_COMBO_BGF_NUM);
    #else
        WMT_PLAT_INFO_FUNC( "BGF_EINT_NUM(not defined)\n");
    #endif

    #ifdef GPIO_COMBO_URXD_PIN
        WMT_PLAT_INFO_FUNC( "UART_RX(GPIO%d)\n", GPIO_COMBO_URXD_PIN);
    #else
        WMT_PLAT_INFO_FUNC( "UART_RX(not defined)\n");
    #endif
    #if defined(FM_DIGITAL_INPUT) || defined(FM_DIGITAL_OUTPUT)
        #ifdef GPIO_COMBO_I2S_CK_PIN
            WMT_PLAT_INFO_FUNC( "I2S_CK(GPIO%d)\n", GPIO_COMBO_I2S_CK_PIN);
        #else
            WMT_PLAT_INFO_FUNC( "I2S_CK(not defined)\n");
        #endif
        #ifdef GPIO_COMBO_I2S_WS_PIN
            WMT_PLAT_INFO_FUNC( "I2S_WS(GPIO%d)\n", GPIO_COMBO_I2S_WS_PIN);
        #else
            WMT_PLAT_INFO_FUNC( "I2S_WS(not defined)\n");
        #endif
        #ifdef GPIO_COMBO_I2S_DAT_PIN
            WMT_PLAT_INFO_FUNC( "I2S_DAT(GPIO%d)\n", GPIO_COMBO_I2S_DAT_PIN);
        #else
            WMT_PLAT_INFO_FUNC( "I2S_DAT(not defined)\n");
        #endif
    #else    //FM_ANALOG_INPUT || FM_ANALOG_OUTPUT
        WMT_PLAT_INFO_FUNC( "FM digital mode is not set, no need for I2S GPIOs\n");
    #endif     
    #ifdef GPIO_GPS_SYNC_PIN
                WMT_PLAT_INFO_FUNC( "GPS_SYNC(GPIO%d)\n", GPIO_GPS_SYNC_PIN);
    #else
                WMT_PLAT_INFO_FUNC( "GPS_SYNC(not defined)\n");
    #endif

    #ifdef GPIO_GPS_LNA_PIN
                WMT_PLAT_INFO_FUNC( "GPS_LNA(GPIO%d)\n", GPIO_GPS_LNA_PIN);
    #else
                WMT_PLAT_INFO_FUNC( "GPS_LNA(not defined)\n");
    #endif
    WMT_PLAT_INFO_FUNC( "[WMT-PLAT]=>dump wmt pin configuration emds<=\n");
    return 0;
}


INT32 wmt_plat_pwr_ctrl (
    ENUM_FUNC_STATE state
    )
{
    INT32 ret = -1;

    switch (state) {
    case FUNC_ON:
        // TODO:[ChangeFeature][George] always output this or by request throuth /proc or sysfs?
        wmt_plat_dump_pin_conf();
        ret = mtk_wcn_consys_hw_pwr_on(gCoClockFlag);
        break;

    case FUNC_OFF:
        ret = mtk_wcn_consys_hw_pwr_off();
        break;

    case FUNC_RST:
        ret = mtk_wcn_consys_hw_rst(gCoClockFlag);
        break;
    case FUNC_STAT:
        ret = mtk_wcn_consys_hw_state_show();
        break;
    default:
        WMT_PLAT_WARN_FUNC("WMT-PLAT:Warnning, invalid state(%d) in pwr_ctrl\n", state);
        break;
    }

    return ret;
}


INT32
wmt_plat_eirq_ctrl (
    ENUM_PIN_ID id,
    ENUM_PIN_STATE state
    )
{
    INT32 iret;

    // TODO: [ChangeFeature][GeorgeKuo]: use another function to handle this, as done in gpio_ctrls

    if ( (PIN_STA_INIT != state )
        && (PIN_STA_DEINIT != state )
        && (PIN_STA_EINT_EN != state )
        && (PIN_STA_EINT_DIS != state ) ) {
        WMT_PLAT_WARN_FUNC("WMT-PLAT:invalid PIN_STATE(%d) in eirq_ctrl for PIN(%d)\n", state, id);
        return -1;
    }

    iret = -2;
    switch (id) {
    case PIN_BGF_EINT:
#if 1
    if (PIN_STA_INIT == state) {
		iret = request_irq(MT_CONN2AP_BTIF_WAKEUP_IRQ_ID,
			(irq_handler_t)wmt_plat_bgf_irq_isr,
			IRQF_TRIGGER_LOW,
			"BTIF_WAKEUP_IRQ",
			NULL);
		if(iret)
		{
			WMT_PLAT_ERR_FUNC("request_irq fail,irq_no(%d),iret(%d)\n",MT_CONN2AP_BTIF_WAKEUP_IRQ_ID,iret);
			return iret;
		}
		gbgfIrqBle.counter = 1;
    }
    else if (PIN_STA_EINT_EN == state) {

		spin_lock_irqsave(&gbgfIrqBle.lock,gbgfIrqBle.flags);
		if(gbgfIrqBle.counter)
		{
			WMT_PLAT_DBG_FUNC("BGF INT has been enabled,counter(%d)\n",gbgfIrqBle.counter);;
		}
		else
		{
        	enable_irq(MT_CONN2AP_BTIF_WAKEUP_IRQ_ID);
			gbgfIrqBle.counter++;
		}
        WMT_PLAT_DBG_FUNC("WMT-PLAT:BGFInt (en) \n");
		spin_unlock_irqrestore(&gbgfIrqBle.lock,gbgfIrqBle.flags);
    }
    else if (PIN_STA_EINT_DIS == state) {

		spin_lock_irqsave(&gbgfIrqBle.lock,gbgfIrqBle.flags);
		if(!gbgfIrqBle.counter)
		{
			WMT_PLAT_INFO_FUNC("BGF INT has been disabled,counter(%d)\n",gbgfIrqBle.counter);;
		}
		else
		{
        	disable_irq_nosync(MT_CONN2AP_BTIF_WAKEUP_IRQ_ID);
			gbgfIrqBle.counter--;
		}
        WMT_PLAT_DBG_FUNC("WMT-PLAT:BGFInt (dis) \n");
		spin_unlock_irqrestore(&gbgfIrqBle.lock,gbgfIrqBle.flags);
    }
    else {
        free_irq(MT_CONN2AP_BTIF_WAKEUP_IRQ_ID,NULL);
        /* de-init: nothing to do in ALPS, such as un-registration... */
    }
#else
        WMT_PLAT_INFO_FUNC("WMT-PLAT:BGF EINT not defined\n", state);
#endif

        iret = 0;
        break;

    default:
        WMT_PLAT_WARN_FUNC("WMT-PLAT:unsupported EIRQ(PIN_ID:%d) in eirq_ctrl\n", id);
        iret = -1;
        break;
    }

    return iret;
}

INT32 wmt_plat_gpio_ctrl (
    ENUM_PIN_ID id,
    ENUM_PIN_STATE state
    )
{
    if ( (PIN_ID_MAX > id)
        && (PIN_STA_MAX > state) ) {

        // TODO: [FixMe][GeorgeKuo] do sanity check to const function table when init and skip checking here
        if (gfp_set_pin_table[id]) {
            return (*(gfp_set_pin_table[id]))(state); /* .handler */
        }
        else {
            WMT_PLAT_WARN_FUNC("WMT-PLAT: null fp for gpio_ctrl(%d)\n", id);
            return -2;
        }
    }
    return -1;
}


INT32
wmt_plat_bgf_eint_ctrl (
    ENUM_PIN_STATE state
    )
{
#ifdef GPIO_COMBO_BGF_EINT_PIN
    switch(state)
    {
        case PIN_STA_INIT:
            /*set to gpio input low, pull down enable*/
            mt_set_gpio_mode(GPIO_COMBO_BGF_EINT_PIN, GPIO_COMBO_BGF_EINT_PIN_M_GPIO);
            mt_set_gpio_dir(GPIO_COMBO_BGF_EINT_PIN, GPIO_DIR_IN);
            mt_set_gpio_pull_select(GPIO_COMBO_BGF_EINT_PIN, GPIO_PULL_DOWN);
            mt_set_gpio_pull_enable(GPIO_COMBO_BGF_EINT_PIN, GPIO_PULL_ENABLE);
            WMT_PLAT_DBG_FUNC("WMT-PLAT:BGFInt init(in pd) \n");
            break;

        case PIN_STA_MUX:
            mt_set_gpio_mode(GPIO_COMBO_BGF_EINT_PIN, GPIO_COMBO_BGF_EINT_PIN_M_GPIO);
            mt_set_gpio_pull_enable(GPIO_COMBO_BGF_EINT_PIN, GPIO_PULL_ENABLE);
            mt_set_gpio_pull_select(GPIO_COMBO_BGF_EINT_PIN, GPIO_PULL_UP);
            mt_set_gpio_mode(GPIO_COMBO_BGF_EINT_PIN, GPIO_COMBO_BGF_EINT_PIN_M_EINT);
            WMT_PLAT_DBG_FUNC("WMT-PLAT:BGFInt mux (eint) \n");
            break;

        case PIN_STA_IN_L:
        case PIN_STA_DEINIT:
            /*set to gpio input low, pull down enable*/
            mt_set_gpio_mode(GPIO_COMBO_BGF_EINT_PIN, GPIO_COMBO_BGF_EINT_PIN_M_GPIO);
            mt_set_gpio_dir(GPIO_COMBO_BGF_EINT_PIN, GPIO_DIR_IN);
            mt_set_gpio_pull_select(GPIO_COMBO_BGF_EINT_PIN, GPIO_PULL_DOWN);
            mt_set_gpio_pull_enable(GPIO_COMBO_BGF_EINT_PIN, GPIO_PULL_ENABLE);
            WMT_PLAT_DBG_FUNC("WMT-PLAT:BGFInt deinit(in pd) \n");
            break;

        default:
            WMT_PLAT_WARN_FUNC("WMT-PLAT:Warnning, invalid state(%d) on BGF EINT\n", state);
            break;
    }
#else
    WMT_PLAT_INFO_FUNC("WMT-PLAT:BGF EINT not defined\n");
#endif
    return 0;
}

static INT32
wmt_plat_gps_sync_ctrl (
    ENUM_PIN_STATE state
    )
{
#ifdef GPIO_GPS_SYNC_PIN
#ifndef GPIO_GPS_SYNC_PIN_M_GPS_SYNC
	#ifdef GPIO_GPS_SYNC_PIN_M_MD1_GPS_SYNC
		#define GPIO_GPS_SYNC_PIN_M_GPS_SYNC GPIO_GPS_SYNC_PIN_M_MD1_GPS_SYNC
	#else
		#ifdef GPIO_GPS_SYNC_PIN_M_MD2_GPS_SYNC
		    #define GPIO_GPS_SYNC_PIN_M_GPS_SYNC GPIO_GPS_SYNC_PIN_M_MD2_GPS_SYNC
		#endif
	#endif
#endif
    switch (state) {
    case PIN_STA_INIT:
    case PIN_STA_DEINIT:
        mt_set_gpio_mode(GPIO_GPS_SYNC_PIN, GPIO_GPS_SYNC_PIN_M_GPIO);
        mt_set_gpio_dir(GPIO_GPS_SYNC_PIN, GPIO_DIR_OUT);
        mt_set_gpio_out(GPIO_GPS_SYNC_PIN, GPIO_OUT_ZERO);
        break;

    case PIN_STA_MUX:
        mt_set_gpio_mode(GPIO_GPS_SYNC_PIN, GPIO_GPS_SYNC_PIN_M_GPS_SYNC);
        break;

    default:
        break;
    }
#endif
    return 0;
}


static INT32
wmt_plat_gps_lna_ctrl (
        ENUM_PIN_STATE state
        )
{
#ifdef GPIO_GPS_LNA_PIN
    switch (state) {
    case PIN_STA_INIT:
    case PIN_STA_DEINIT:
        mt_set_gpio_pull_enable(GPIO_GPS_LNA_PIN, GPIO_PULL_DISABLE);
        mt_set_gpio_dir(GPIO_GPS_LNA_PIN, GPIO_DIR_OUT);
        mt_set_gpio_mode(GPIO_GPS_LNA_PIN, GPIO_GPS_LNA_PIN_M_GPIO);
        mt_set_gpio_out(GPIO_GPS_LNA_PIN, GPIO_OUT_ZERO);
        break;
    case PIN_STA_OUT_H:
        mt_set_gpio_out(GPIO_GPS_LNA_PIN, GPIO_OUT_ONE);
        break;
    case PIN_STA_OUT_L:
        mt_set_gpio_out(GPIO_GPS_LNA_PIN, GPIO_OUT_ZERO);
        break;

    default:
        WMT_PLAT_WARN_FUNC("%d mode not defined for  gps lna pin !!!\n", state);
        break;
    }
    return 0;
#else
    WMT_PLAT_WARN_FUNC("host gps lna pin not defined!!!\n")
    return 0;
#endif
}


INT32 wmt_plat_i2s_ctrl(ENUM_PIN_STATE state)
{
    // TODO: [NewFeature][GeorgeKuo]: GPIO_I2Sx is changed according to different project.
    // TODO: provide a translation table in board_custom.h for different ALPS project customization.
    
#if defined(FM_DIGITAL_INPUT) || defined(FM_DIGITAL_OUTPUT)
    #if defined (GPIO_COMBO_I2S_CK_PIN)
    switch(state)
    {
    case PIN_STA_INIT:
    case PIN_STA_MUX:
            mt_set_gpio_mode(GPIO_COMBO_I2S_CK_PIN, GPIO_COMBO_I2S_CK_PIN_M_I2S0_CK);
            mt_set_gpio_mode(GPIO_COMBO_I2S_WS_PIN, GPIO_COMBO_I2S_WS_PIN_M_I2S0_WS);
            mt_set_gpio_mode(GPIO_COMBO_I2S_DAT_PIN, GPIO_COMBO_I2S_DAT_PIN_M_I2S0_DAT);
            WMT_PLAT_DBG_FUNC("WMT-PLAT:I2S init (I2S0 system) \n");
        break;
    case PIN_STA_IN_L:
    case PIN_STA_DEINIT:
            mt_set_gpio_mode(GPIO_COMBO_I2S_CK_PIN, GPIO_COMBO_I2S_CK_PIN_M_GPIO);
            mt_set_gpio_dir(GPIO_COMBO_I2S_CK_PIN, GPIO_DIR_OUT);
            mt_set_gpio_out(GPIO_COMBO_I2S_CK_PIN, GPIO_OUT_ZERO);

            mt_set_gpio_mode(GPIO_COMBO_I2S_WS_PIN, GPIO_COMBO_I2S_WS_PIN_M_GPIO);
            mt_set_gpio_dir(GPIO_COMBO_I2S_WS_PIN, GPIO_DIR_OUT);
            mt_set_gpio_out(GPIO_COMBO_I2S_WS_PIN, GPIO_OUT_ZERO);

            mt_set_gpio_mode(GPIO_COMBO_I2S_DAT_PIN, GPIO_COMBO_I2S_DAT_PIN_M_GPIO);
            mt_set_gpio_dir(GPIO_COMBO_I2S_DAT_PIN, GPIO_DIR_OUT);
            mt_set_gpio_out(GPIO_COMBO_I2S_DAT_PIN, GPIO_OUT_ZERO);
            WMT_PLAT_DBG_FUNC("WMT-PLAT:I2S deinit (out 0) \n");
        break;
    default:
        WMT_PLAT_WARN_FUNC("WMT-PLAT:Warnning, invalid state(%d) on I2S Group\n", state);
        break;
    }
    #else
    WMT_PLAT_ERR_FUNC( "[MT6620]Error:FM digital mode set, but no I2S GPIOs defined\n");
    #endif
#else
        WMT_PLAT_INFO_FUNC( "[MT6620]warnning:FM digital mode is not set, no I2S GPIO settings should be modified by combo driver\n");
#endif

    return 0;
}



INT32 wmt_plat_wake_lock_ctrl(ENUM_WL_OP opId)
{
#ifdef CFG_WMT_WAKELOCK_SUPPORT
    static INT32 counter = 0;
    INT32 ret = 0;
    
    
    ret = mutex_lock_killable( &gOsSLock);
    if (ret) {
        WMT_PLAT_ERR_FUNC("--->lock gOsSLock failed, ret=%d\n", ret);
        return ret;
    }
    
    if (WL_OP_GET == opId)
    {
        ++counter;
    }else if (WL_OP_PUT == opId)
    {
        --counter;
    }
    mutex_unlock( &gOsSLock);
    if (WL_OP_GET == opId && counter == 1)
    {
        wake_lock(&wmtWakeLock);
        WMT_PLAT_DBG_FUNC("WMT-PLAT: after wake_lock(%d), counter(%d)\n", wake_lock_active(&wmtWakeLock), counter);    
        
    }
    else if (WL_OP_PUT == opId && counter == 0)
    {
        wake_unlock(&wmtWakeLock);
        WMT_PLAT_DBG_FUNC("WMT-PLAT: after wake_unlock(%d), counter(%d)\n", wake_lock_active(&wmtWakeLock), counter); 
    }
    else
    {
        WMT_PLAT_WARN_FUNC("WMT-PLAT: wakelock status(%d), counter(%d)\n", wake_lock_active(&wmtWakeLock), counter); 
    }
    return 0;
#else
    WMT_PLAT_WARN_FUNC("WMT-PLAT: host awake function is not supported.");
    return 0;

#endif
}



INT32 wmt_plat_soc_paldo_ctrl(ENUM_PALDO_TYPE ePt,ENUM_PALDO_OP ePo)
{
	INT32 iRet = 0;
	switch(ePt){
		
	case BT_PALDO:
        iRet = mtk_wcn_consys_hw_bt_paldo_ctrl(ePo);
        break;
    case WIFI_PALDO:
        iRet = mtk_wcn_consys_hw_wifi_paldo_ctrl(ePo);
        break;
	case FM_PALDO:
	case GPS_PALDO:
		iRet = mtk_wcn_consys_hw_vcn28_ctrl(ePo);
		break;
    default:
        WMT_PLAT_WARN_FUNC("WMT-PLAT:Warnning, invalid type(%d) in palod_ctrl\n", ePt);
        break;
	}
	return iRet;
}

#if CONSYS_WMT_REG_SUSPEND_CB_ENABLE
UINT32 wmt_plat_soc_osc_en_ctrl(UINT32 en)
{
	return mtk_wcn_consys_hw_osc_en_ctrl(en);
}
#endif

UINT8 *wmt_plat_get_emi_virt_add(UINT32 offset)
{
	return mtk_wcn_consys_emi_virt_addr_get(offset);
}

P_CONSYS_EMI_ADDR_INFO wmt_plat_get_emi_phy_add(VOID)
{
	return &mtk_wcn_emi_addr_info;
}

#if CONSYS_ENALBE_SET_JTAG
UINT32 wmt_plat_jtag_flag_ctrl(UINT32 en)
{
	return mtk_wcn_consys_jtag_flag_ctrl(en);
}
#endif
#if CFG_WMT_DUMP_INT_STATUS
VOID wmt_plat_BGF_irq_dump_status(VOID)
{
	mt_irq_dump_status(MT_CONN2AP_BTIF_WAKEUP_IRQ_ID);
}
#endif

UINT32 wmt_plat_read_cpupcr()
{
	return CONSYS_REG_READ(CONSYS_CPUPCR_REG);
}

INT32 wmt_plat_set_host_dump_state(ENUM_HOST_DUMP_STATE state)
{
	UINT8 * p_virtual_addr = NULL;

	p_virtual_addr = wmt_plat_get_emi_virt_add(EXP_APMEM_CTRL_HOST_SYNC_STATE);
	if(!p_virtual_addr)
	{
		WMT_PLAT_ERR_FUNC("get virtual address fail\n");
		return -1;
	}

	CONSYS_REG_WRITE(p_virtual_addr, state);

	return 0;
}

UINT32 wmt_plat_force_trigger_assert(ENUM_FORCE_TRG_ASSERT_T type)
{
	UINT8 * p_virtual_addr = NULL;
	
	switch(type){
		case STP_FORCE_TRG_ASSERT_EMI:

			WMT_PLAT_INFO_FUNC("[Force Assert] stp_trigger_firmware_assert_via_emi -->\n");
    		p_virtual_addr = wmt_plat_get_emi_virt_add(EXP_APMEM_CTRL_HOST_OUTBAND_ASSERT_W1);
    		if(!p_virtual_addr)
    		{
    			WMT_PLAT_ERR_FUNC("get virtual address fail\n");
    			return -1;
    		}

    		CONSYS_REG_WRITE(p_virtual_addr, EXP_APMEM_HOST_OUTBAND_ASSERT_MAGIC_W1);
        	WMT_PLAT_INFO_FUNC("[Force Assert] stp_trigger_firmware_assert_via_emi <--\n");
		break;
		case STP_FORCE_TRG_ASSERT_DEBUG_PIN:

			CONSYS_REG_WRITE(CONSYS_AP2CONN_OSC_EN_REG,CONSYS_REG_READ(CONSYS_AP2CONN_OSC_EN_REG) & ~CONSYS_AP2CONN_WAKEUP_BIT);
			WMT_PLAT_INFO_FUNC("enable:dump CONSYS_AP2CONN_OSC_EN_REG(0x%x)\n",CONSYS_REG_READ(CONSYS_AP2CONN_OSC_EN_REG));
			usleep_range(64, 96);
			CONSYS_REG_WRITE(CONSYS_AP2CONN_OSC_EN_REG,CONSYS_REG_READ(CONSYS_AP2CONN_OSC_EN_REG) | CONSYS_AP2CONN_WAKEUP_BIT);
			WMT_PLAT_INFO_FUNC("disable:dump CONSYS_AP2CONN_OSC_EN_REG(0x%x)\n",CONSYS_REG_READ(CONSYS_AP2CONN_OSC_EN_REG));

		break;
		default:
			WMT_PLAT_ERR_FUNC("unknow force trigger assert type\n");
		break;
	}

	return 0;
}

INT32 wmt_plat_update_host_sync_num(VOID)
{
	UINT8 * p_virtual_addr = NULL;
	UINT32 sync_num = 0;
	
	p_virtual_addr = wmt_plat_get_emi_virt_add(EXP_APMEM_CTRL_HOST_SYNC_NUM);
	if(!p_virtual_addr)
	{
		WMT_PLAT_ERR_FUNC("get virtual address fail\n");
		return -1;
	}

	sync_num = CONSYS_REG_READ(p_virtual_addr);
	CONSYS_REG_WRITE(p_virtual_addr, sync_num + 1);

	return 0;
}

INT32 wmt_plat_get_dump_info(UINT32 offset)
{
	UINT8 * p_virtual_addr = NULL;

	p_virtual_addr = wmt_plat_get_emi_virt_add(offset);
	if(!p_virtual_addr)
	{
		WMT_PLAT_ERR_FUNC("get virtual address fail\n");
		return -1;
	}

	return CONSYS_REG_READ(p_virtual_addr);
}

UINT32 wmt_plat_get_soc_chipid()
{
	UINT32 chipId = mtk_wcn_consys_soc_chipid();
	WMT_PLAT_INFO_FUNC("current SOC chip:0x%x\n",chipId);
	return chipId;
}



EXPORT_SYMBOL(wmt_plat_init);
EXPORT_SYMBOL(wmt_plat_deinit);
EXPORT_SYMBOL(wmt_plat_pwr_ctrl);
EXPORT_SYMBOL(wmt_plat_gpio_ctrl);
EXPORT_SYMBOL(wmt_plat_soc_paldo_ctrl);
EXPORT_SYMBOL(wmt_plat_wake_lock_ctrl);
EXPORT_SYMBOL(wmt_plat_eirq_ctrl);
EXPORT_SYMBOL(wmt_plat_read_cpupcr);
EXPORT_SYMBOL(wmt_plat_update_host_sync_num);
EXPORT_SYMBOL(wmt_plat_set_host_dump_state);
EXPORT_SYMBOL(wmt_plat_get_dump_info);
EXPORT_SYMBOL(wmt_plat_force_trigger_assert);
EXPORT_SYMBOL(wmt_plat_get_emi_virt_add);
EXPORT_SYMBOL(wmt_plat_get_emi_phy_add);
#if CONSYS_ENALBE_SET_JTAG
EXPORT_SYMBOL(wmt_plat_jtag_flag_ctrl);
#endif
#if CFG_WMT_DUMP_INT_STATUS
EXPORT_SYMBOL(wmt_plat_BGF_irq_dump_status);
#endif
#if CONSYS_WMT_REG_SUSPEND_CB_ENABLE
EXPORT_SYMBOL(wmt_plat_soc_osc_en_ctrl);
#endif
EXPORT_SYMBOL(wmt_plat_thermal_ctrl_cb_reg);
EXPORT_SYMBOL(wmt_plat_deep_idle_ctrl_cb_reg);
EXPORT_SYMBOL(wmt_plat_func_ctrl_cb_reg);
EXPORT_SYMBOL(wmt_plat_aif_cb_reg);
EXPORT_SYMBOL(wmt_plat_irq_cb_reg);
EXPORT_SYMBOL(wmt_plat_get_soc_chipid);

