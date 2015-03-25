/* system header files */
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/interrupt.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/pm.h>
#include <linux/mutex.h>
#include <linux/uaccess.h>
#include <linux/syscalls.h>
#include <linux/mtd/nand.h>

#include <asm/irq.h>
#include <asm/io.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/irq.h>
#include <asm/mach/map.h>
#include <asm/mach/time.h>
#include <asm/setup.h>

#include <mach/system.h>
#include <mach/board.h>
#include <mach/hardware.h>
#include <mach/mt_gpio.h>
#include <mach/mt_bt.h>
#include <mach/eint.h>
#include <mach/mtk_rtc.h>
#include <mach/mt_typedefs.h>
// Fix-me: marked for early porting
#include <cust_gpio_usage.h>
#include <cust_eint.h>
#if defined(CONFIG_MTK_COMBO) || defined(CONFIG_MTK_COMBO_MODULE)
#include <mach/mtk_wcn_cmb_stub.h>

#if 0
static void combo_bt_pcm_pin_on(void);
static void combo_bt_pcm_pin_off(void);
static void combo_fm_i2s_pin_on(void);
static void combo_fm_i2s_pin_off(void);
#endif

#endif
#if defined(CONFIG_MTK_COMBO) || defined(CONFIG_MTK_COMBO_MODULE)
    /* combo chip */
    #if defined(MTK_MT6620)  
void mt6620_power_on(void);
void mt6620_power_off(void);
void mt6620_print_pin_configure(void);
    #endif
#endif
#if defined(CONFIG_MTK_COMBO_SDIO_SLOT)

#if 0
static sdio_irq_handler_t combo_sdio_eirq_handler = NULL;
static void *combo_sdio_eirq_data = NULL;
#endif
static void *combo_sdio_pm_data = NULL;
static pm_callback_t combo_sdio_pm_cb = NULL;
//static pm_message_t mt_wifi_pm_state = { .event = PM_EVENT_HIBERNATE };
//static int mt_wifi_pm_late_cb = 0;

/* temp solution to avoid compile error */
#define CUST_EINT_WIFI_NUM          0
#define CUST_EINT_WIFI_SENSITIVE    0
#define CUST_EINT_WIFI_DEBOUNCE_CN  0
#define CUST_EINT_WIFI_DEBOUNCE_EN  0
#define CUST_EINT_WIFI_POLARITY     0
#define GPIO_WIFI_EINT_PIN          0
#define GPIO_WIFI_EINT_PIN_M_EINT   0
#define GPIO_WIFI_EINT_PIN_M_GPIO   0

	#if (CONFIG_MTK_COMBO_SDIO_SLOT == 0)
		const static u32 combo_sdio_eint_pin = GPIO_WIFI_EINT_PIN;
		const static u32 combo_sdio_eint_num = CUST_EINT_WIFI_NUM;
		const static u32 combo_sdio_eint_m_eint = GPIO_WIFI_EINT_PIN_M_EINT;
		const static u32 combo_sdio_eint_m_gpio = GPIO_WIFI_EINT_PIN_M_GPIO;
		static unsigned char combo_port_pwr_map[4] = {0x0, 0xFF, 0xFF, 0xFF};
		/*
		index: port number of combo chip (1:SDIO1, 2:SDIO2, no SDIO0)
		value: slot power status of  (0:off, 1:on, 0xFF:invalid)
		*/
	#elif (CONFIG_MTK_COMBO_SDIO_SLOT == 1)
		const static u32 combo_sdio_eint_pin = GPIO_WIFI_EINT_PIN;
		const static u32 combo_sdio_eint_num = CUST_EINT_WIFI_NUM;
		const static u32 combo_sdio_eint_m_eint = GPIO_WIFI_EINT_PIN_M_EINT;
		const static u32 combo_sdio_eint_m_gpio = GPIO_WIFI_EINT_PIN_M_GPIO;
		static unsigned char combo_port_pwr_map[4] = {0xFF, 0x0, 0xFF, 0xFF};
		/*
		index: port number of combo chip (1:SDIO1, 2:SDIO2, no SDIO0)
		value: slot power status of  (0:off, 1:on, 0xFF:invalid)
		*/
    #elif (CONFIG_MTK_COMBO_SDIO_SLOT == 2)
		const static u32 combo_sdio_eint_pin = GPIO_WIFI_EINT_PIN;
		const static u32 combo_sdio_eint_num = CUST_EINT_WIFI_NUM;
		const static u32 combo_sdio_eint_m_eint = GPIO_WIFI_EINT_PIN_M_EINT;
		const static u32 combo_sdio_eint_m_gpio = GPIO_WIFI_EINT_PIN_M_GPIO;
#if 0
		static unsigned char combo_port_pwr_map[4] = {0xFF, 0xFF, 0x0, 0xFF};
#endif
		/*
		index: port number of combo chip (1:SDIO1, 2:SDIO2, no SDIO0)
		value: slot power status of  (0:off, 1:on, 0xFF:invalid)
		*/

    #elif (CONFIG_MTK_COMBO_SDIO_SLOT == 3)
		/* jump ALL_INT_B to GPIO_WIFI_EINT_PIN(EINT4) instead of using GPIO_COMBO_ALL_EINT_PIN */
		const static u32 combo_sdio_eint_pin = GPIO_WIFI_EINT_PIN;
		const static u32 combo_sdio_eint_num = CUST_EINT_WIFI_NUM; /* CUST_EINT_COMBO_ALL_NUM */
		const static u32 combo_sdio_eint_m_eint = GPIO_WIFI_EINT_PIN_M_EINT;
		const static u32 combo_sdio_eint_m_gpio = GPIO_WIFI_EINT_PIN_M_GPIO;
		static unsigned char combo_port_pwr_map[4] = {0xFF, 0xFF, 0xFF, 0x0};
		/*
		index: port number of combo chip (1:SDIO1, 2:SDIO2, no SDIO0)
		value: slot power status of  (0:off, 1:on, 0xFF:invalid)
		*/

    #else
    #error "unsupported CONFIG_MTK_COMBO_SDIO_SLOT" CONFIG_MTK_COMBO_SDIO_SLOT
    #endif

#else
#endif
/*=======================================================================*/
/* Board Specific Devices Power Management                               */
/*=======================================================================*/
extern kal_bool pmic_chrdet_status(void);

void mt_power_off(void)
{
	printk("mt_power_off\n");

	/* pull PWRBB low */
	rtc_bbpu_power_down();

	while (1) {
#if defined(CONFIG_POWER_EXT)
		//EVB
		printk("EVB without charger\n");
#else	
		//Phone	
		printk("Phone with charger\n");
		if (pmic_chrdet_status() == KAL_TRUE)
			arch_reset(0, "power_off_with_charger");
#endif
    }
}

/*=======================================================================*/
/* Board Specific Devices                                                */
/*=======================================================================*/
/*GPS driver*/
/*FIXME: remove mt3326 notation */
struct mt3326_gps_hardware mt3326_gps_hw = {
    .ext_power_on =  NULL,
    .ext_power_off = NULL,
};

/*=======================================================================*/
/* Board Specific Devices Init                                           */
/*=======================================================================*/
#if !defined(CONFIG_MTK_COMBO) && !defined(CONFIG_MTK_COMBO_MODULE)
#endif
#ifdef MTK_BT_SUPPORT
void mt_bt_power_on(void)
{
    printk(KERN_INFO "+mt_bt_power_on\n");

#if defined(CONFIG_MTK_COMBO) || defined(CONFIG_MTK_COMBO_MODULE)
    /* combo chip product */
    /*
     * Ignore rfkill0/state call. Controll BT power on/off through device /dev/stpbt.
     */
#else 
    /* standalone product */
#endif

    printk(KERN_INFO "-mt_bt_power_on\n");
}
EXPORT_SYMBOL(mt_bt_power_on);

void mt_bt_power_off(void)
{
    printk(KERN_INFO "+mt_bt_power_off\n");

#if defined(CONFIG_MTK_COMBO) || defined(CONFIG_MTK_COMBO_MODULE)
    /* combo chip product */
    /*
     * Ignore rfkill0/state call. Controll BT power on/off through device /dev/stpbt.
     */
#else
    /* standalone product */
#endif

    printk(KERN_INFO "-mt_bt_power_off\n");
}
EXPORT_SYMBOL(mt_bt_power_off);

int mt_bt_suspend(pm_message_t state)
{
    printk(KERN_INFO "+mt_bt_suspend\n");
    printk(KERN_INFO "-mt_bt_suspend\n");
    return MT_BT_OK;
}

int mt_bt_resume(pm_message_t state)
{
    printk(KERN_INFO "+mt_bt_resume\n");
    printk(KERN_INFO "-mt_bt_resume\n");
    return MT_BT_OK;
}
#endif

#if 0
#if defined(CONFIG_MTK_COMBO) || defined(CONFIG_MTK_COMBO_MODULE)

static void combo_bgf_eirq_handler(void)
{
    mt_combo_bgf_eirq_handler(NULL);
}

static void mt_combo_bgf_request_irq(void *data)
{
    mt65xx_eint_set_sens(CUST_EINT_COMBO_BGF_NUM, CUST_EINT_COMBO_BGF_SENSITIVE);
    mt65xx_eint_set_hw_debounce(CUST_EINT_COMBO_BGF_NUM, CUST_EINT_COMBO_BGF_DEBOUNCE_CN);
    mt65xx_eint_registration(CUST_EINT_COMBO_BGF_NUM,
        CUST_EINT_COMBO_BGF_DEBOUNCE_EN,
        CUST_EINT_COMBO_BGF_POLARITY,
        combo_bgf_eirq_handler,
        0);
    mt65xx_eint_mask(CUST_EINT_COMBO_BGF_NUM); /*2*/
    return;
}

/* Combo chip shared interrupt (BGF_INT_B) */
void mt_combo_bgf_enable_irq(void)
{
    mt65xx_eint_unmask(CUST_EINT_COMBO_BGF_NUM);
    return;
}
EXPORT_SYMBOL(mt_combo_bgf_enable_irq);

void mt_combo_bgf_disable_irq(void)
{
    mt65xx_eint_mask(CUST_EINT_COMBO_BGF_NUM);
    return;
}
EXPORT_SYMBOL(mt_combo_bgf_disable_irq);

void mt6620_print_pin_configure(void)
{
	printk(KERN_INFO "[MT6620_PIN]=>GPIO pin configuration start<=\n");
#ifdef GPIO_COMBO_6620_LDO_EN_PIN
	printk(KERN_INFO "LDO_EN(GPIO%d)\n", GPIO_COMBO_6620_LDO_EN_PIN);
#else
	printk(KERN_INFO "LDO_EN(not defined)\n");	
#endif

#ifdef GPIO_COMBO_PMU_EN_PIN
	printk(KERN_INFO "PMU_EN(GPIO%d)\n", GPIO_COMBO_PMU_EN_PIN);
#else
	printk(KERN_INFO "PMU_EN(not defined)\n");	
#endif


#ifdef GPIO_COMBO_RST_PIN
	printk(KERN_INFO "RST(GPIO%d)\n", GPIO_COMBO_RST_PIN);
#else
	printk(KERN_INFO "RST(not defined)\n");	
#endif
#ifdef GPIO_COMBO_ALL_EINT_PIN
	printk(KERN_INFO "ALL_EINT(GPIO%d)\n", GPIO_COMBO_ALL_EINT_PIN);
#else
	printk(KERN_INFO "ALL_EINT(not defined)\n");
#endif
#ifdef GPIO_COMBO_BGF_EINT_PIN
	printk(KERN_INFO "BGF_EINT(GPIO%d)\n", GPIO_COMBO_BGF_EINT_PIN);
#else
	printk(KERN_INFO "BGF_EINT(not defined)\n");	
#endif

#ifdef CUST_EINT_COMBO_BGF_NUM
	printk(KERN_INFO "BGF_EINT_NUM(%d)\n", CUST_EINT_COMBO_BGF_NUM);
#else
	printk(KERN_INFO "BGF_EINT_NUM(not defined)\n");	
#endif

#ifdef GPIO_WIFI_EINT_PIN
	printk(KERN_INFO "WIFI_EINT(GPIO%d)\n", GPIO_WIFI_EINT_PIN);
#else
	printk(KERN_INFO "WIFI_EINT(not defined)\n");	
#endif

#ifdef CUST_EINT_WIFI_NUM
	printk(KERN_INFO "WIFI_EINT_NUM(%d)\n", CUST_EINT_WIFI_NUM);
#else
	printk(KERN_INFO "WIFI_EINT_NUM(not defined)\n");	
#endif

#ifdef GPIO_UART_URXD3_PIN
	printk(KERN_INFO "UART_RX(GPIO%d)\n", GPIO_UART_URXD3_PIN);
#else
	printk(KERN_INFO "UART_RX(not defined)\n");	
#endif
#ifdef GPIO_UART_UTXD3_PIN
	printk(KERN_INFO "UART_TX(GPIO%d)\n", GPIO_UART_UTXD3_PIN);
#else
	printk(KERN_INFO "UART_TX(not defined)\n");	
#endif
#ifdef GPIO_PCM_DAICLK_PIN
	printk(KERN_INFO "DAICLK(GPIO%d)\n", GPIO_PCM_DAICLK_PIN);
#else
	printk(KERN_INFO "DAICLK(not defined)\n");	
#endif
#ifdef GPIO_PCM_DAIPCMOUT_PIN
	printk(KERN_INFO "PCMOUT(GPIO%d)\n", GPIO_PCM_DAIPCMOUT_PIN);
#else
	printk(KERN_INFO "PCMOUT(not defined)\n");	
#endif
#ifdef GPIO_PCM_DAIPCMIN_PIN
	printk(KERN_INFO "PCMIN(GPIO%d)\n", GPIO_PCM_DAIPCMIN_PIN);
#else
	printk(KERN_INFO "PCMIN(not defined)\n");	
#endif
#ifdef GPIO_PCM_DAISYNC_PIN
	printk(KERN_INFO "PCMSYNC(GPIO%d)\n", GPIO_PCM_DAISYNC_PIN);
#else
	printk(KERN_INFO "PCMSYNC(not defined)\n");	
#endif
#ifndef FM_ANALOG_INPUT
	#ifdef GPIO_I2S0_CK_PIN
		printk(KERN_INFO "I2S_CK(GPIO%d)\n", GPIO_I2S0_CK_PIN);
	#else
		printk(KERN_INFO "I2S_CK(GPIO%d)\n", GPIO_I2S0_CK_PIN);
	#endif
	#ifdef GPIO_I2S0_WS_PIN
		printk(KERN_INFO "I2S_WS(GPIO%d)\n", GPIO_I2S0_WS_PIN);
	#else
		printk(KERN_INFO "I2S_WS(GPIO%d)\n", GPIO_I2S0_WS_PIN);
	#endif
	#ifdef GPIO_I2S0_DAT_PIN
		printk(KERN_INFO "I2S_DAT(GPIO%d)\n", GPIO_I2S0_DAT_PIN);
	#else
		printk(KERN_INFO "I2S_DAT(GPIO%d)\n", GPIO_I2S0_DAT_PIN);
	#endif
#else
	printk(KERN_INFO "fm analog input mode is set, no need to set I2S pins\n");
#endif
	printk(KERN_INFO "[MT6620_PIN]=>GPIO pin configuration end<=\n");
	
}

void mt6620_power_on(void)
{
    int result = 0;
    static int _32k_set = 0;
	/*log MT6620 GPIO Settings*/
	mt6620_print_pin_configure();
    /* disable interrupt firstly */
    mt_combo_bgf_disable_irq();

#define MT6620_OFF_TIME (10) /* in ms, workable value */
#define MT6620_RST_TIME (30) /* in ms, workable value */
#define MT6620_STABLE_TIME (30) /* in ms, workable value */
#define MT6620_EXT_INT_TIME (5) /* in ms, workable value */
#define MT6620_32K_STABLE_TIME (100) /* in ms, test value */

    
    
#if defined(CONFIG_MTK_COMBO_SDIO_SLOT) && (CONFIG_MTK_COMBO_SDIO_SLOT == 0)
	printk(KERN_INFO "[mt6620] pull up sd0 bus(gpio169~gpio175(exclude gpio174))\n");
    mt_set_gpio_pull_enable(GPIO172, GPIO_PULL_ENABLE);	//->CLK
    mt_set_gpio_pull_select(GPIO172, GPIO_PULL_UP);		
    mt_set_gpio_pull_enable(GPIO171, GPIO_PULL_ENABLE);	//->CMD
    mt_set_gpio_pull_select(GPIO171, GPIO_PULL_UP);
    mt_set_gpio_pull_enable(GPIO175, GPIO_PULL_ENABLE);	//->DAT0
    mt_set_gpio_pull_select(GPIO175, GPIO_PULL_UP);
    mt_set_gpio_pull_enable(GPIO173, GPIO_PULL_ENABLE);	//->DAT1
    mt_set_gpio_pull_select(GPIO173, GPIO_PULL_UP);
    mt_set_gpio_pull_enable(GPIO169, GPIO_PULL_ENABLE);	//->DAT2
    mt_set_gpio_pull_select(GPIO169, GPIO_PULL_UP);
    mt_set_gpio_pull_enable(GPIO170, GPIO_PULL_ENABLE);	//->DAT3
    mt_set_gpio_pull_select(GPIO170, GPIO_PULL_UP);
#elif (CONFIG_MTK_COMBO_SDIO_SLOT == 1)
	#error "error:MSDC1 is not reserved for MT6620 on MT6575EVB"
#elif (CONFIG_MTK_COMBO_SDIO_SLOT == 2)
    printk(KERN_INFO "[mt6620] pull up sd2 bus(gpio182~187)\n");
    mt_set_gpio_pull_enable(GPIO182, GPIO_PULL_ENABLE);	//->CLK
    mt_set_gpio_pull_select(GPIO182, GPIO_PULL_UP);		
    mt_set_gpio_pull_enable(GPIO184, GPIO_PULL_ENABLE);	//->CMD
    mt_set_gpio_pull_select(GPIO184, GPIO_PULL_UP);
    mt_set_gpio_pull_enable(GPIO186, GPIO_PULL_ENABLE);	//->DAT0
    mt_set_gpio_pull_select(GPIO186, GPIO_PULL_UP);
    mt_set_gpio_pull_enable(GPIO187, GPIO_PULL_ENABLE);	//->DAT1
    mt_set_gpio_pull_select(GPIO187, GPIO_PULL_UP);
    mt_set_gpio_pull_enable(GPIO185, GPIO_PULL_ENABLE);	//->DAT2
    mt_set_gpio_pull_select(GPIO185, GPIO_PULL_UP);
    mt_set_gpio_pull_enable(GPIO183, GPIO_PULL_ENABLE);	//->DAT3
    mt_set_gpio_pull_select(GPIO183, GPIO_PULL_UP);
#elif (CONFIG_MTK_COMBO_SDIO_SLOT == 3)
	printk(KERN_INFO "[mt6620] pull up sd3 bus (GPIO89~GPIO94)\n");
    mt_set_gpio_pull_enable(GPIO92, GPIO_PULL_ENABLE);	//->CLK
    mt_set_gpio_pull_select(GPIO92, GPIO_PULL_UP);		
    mt_set_gpio_pull_enable(GPIO91, GPIO_PULL_ENABLE);	//->CMD
    mt_set_gpio_pull_select(GPIO91, GPIO_PULL_UP);
    mt_set_gpio_pull_enable(GPIO94, GPIO_PULL_ENABLE);	//->DAT0
    mt_set_gpio_pull_select(GPIO94, GPIO_PULL_UP);
    mt_set_gpio_pull_enable(GPIO90, GPIO_PULL_ENABLE);	//->DAT1
    mt_set_gpio_pull_select(GPIO90, GPIO_PULL_UP);
    mt_set_gpio_pull_enable(GPIO89, GPIO_PULL_ENABLE);	//->DAT2
    mt_set_gpio_pull_select(GPIO89, GPIO_PULL_UP);
    mt_set_gpio_pull_enable(GPIO93, GPIO_PULL_ENABLE);	//->DAT3
    mt_set_gpio_pull_select(GPIO93, GPIO_PULL_UP);
#else
	#error "error:unsupported CONFIG_MTK_COMBO_SDIO_SLOT" CONFIG_MTK_COMBO_SDIO_SLOT
#endif

    //printk(KERN_INFO "[mt6620] enable RTC CLK\n");
	if(_32k_set == 0){
		//rtc_gpio_export_32k(true); //old 32k export API
		/*
		* To fix RTC32k clocks stops after system reboot
		*/
		rtc_gpio_enable_32k(RTC_GPIO_USER_GPS);
		_32k_set = 1;
		printk("[mt6620]rtc_gpio_enable_32k(RTC_GPIO_USER_GPS) \n");
	} else {
		printk("[mt6620]not to rtc_gpio_enable_32k(RTC_GPIO_USER_GPS)\n");
	}
    msleep(MT6620_32K_STABLE_TIME);


    /* UART Mode */
    result += mt_set_gpio_mode(GPIO_UART_URXD3_PIN, GPIO_UART_URXD3_PIN_M_URXD);//GPIO_MODE_01->GPIO_UART_URXD3_PIN_M_URXD
    result += mt_set_gpio_mode(GPIO_UART_UTXD3_PIN, GPIO_UART_UTXD3_PIN_M_UTXD);//GPIO_MODE_01->GPIO_UART_UTXD3_PIN_M_UTXD
    //printk(KERN_INFO "[mt6620] set UART GPIO Mode [%d]\n", result);

    /* FIXME! GeorgeKuo: added for MT6620 GPIO initialization */
    /* disable pull */

	mt_set_gpio_pull_enable(GPIO_COMBO_PMU_EN_PIN, GPIO_PULL_DISABLE);

    mt_set_gpio_pull_enable(GPIO_COMBO_RST_PIN, GPIO_PULL_DISABLE);
    /* set output */

	mt_set_gpio_dir(GPIO_COMBO_PMU_EN_PIN, GPIO_DIR_OUT);

    mt_set_gpio_dir(GPIO_COMBO_RST_PIN, GPIO_DIR_OUT);
    /* set gpio mode */

	mt_set_gpio_mode(GPIO_COMBO_PMU_EN_PIN, GPIO_MODE_GPIO);

    mt_set_gpio_mode(GPIO_COMBO_RST_PIN, GPIO_MODE_GPIO);

    /* SYSRST_B low */
    mt_set_gpio_out(GPIO_COMBO_RST_PIN, GPIO_OUT_ZERO);
    /* PMU_EN low */
	mt_set_gpio_out(GPIO_COMBO_PMU_EN_PIN, GPIO_OUT_ZERO);

    msleep(MT6620_OFF_TIME);

    /* PMU_EN high, SYSRST_B low */
	mt_set_gpio_out(GPIO_COMBO_PMU_EN_PIN, GPIO_OUT_ONE);
    msleep(MT6620_RST_TIME);

    /* SYSRST_B high */
    mt_set_gpio_out(GPIO_COMBO_RST_PIN, GPIO_OUT_ONE);
    msleep(MT6620_STABLE_TIME);

    /* BT PCM bus default mode. Real control is done by audio and mt_combo.c */
    mt_combo_audio_ctrl_ex(COMBO_AUDIO_STATE_1, 0);

    /* EINT1 for BGF_INT_B */
    mt_set_gpio_mode(GPIO_COMBO_BGF_EINT_PIN, GPIO_COMBO_BGF_EINT_PIN_M_GPIO);
    mt_set_gpio_pull_enable(GPIO_COMBO_BGF_EINT_PIN, GPIO_PULL_ENABLE);
    mt_set_gpio_pull_select(GPIO_COMBO_BGF_EINT_PIN, GPIO_PULL_UP);
    mt_set_gpio_mode(GPIO_COMBO_BGF_EINT_PIN, GPIO_COMBO_BGF_EINT_PIN_M_EINT);

    /* request IRQ (EINT1) */
    mt_combo_bgf_request_irq(NULL);

    printk(KERN_INFO "[mt6620] power on \n");

    return;
}
EXPORT_SYMBOL(mt6620_power_on);

void mt6620_power_off(void)
{
    printk(KERN_INFO "[mt6620] power off\n");

    //printk(KERN_INFO "[mt6620] mt_combo_bgf_disable_irq\n");
    mt_combo_bgf_disable_irq();

    //printk(KERN_INFO "[mt6620] set BGF_EINT input pull down\n");
    mt_set_gpio_mode(GPIO_COMBO_BGF_EINT_PIN, GPIO_COMBO_BGF_EINT_PIN_M_GPIO);
    mt_set_gpio_dir(GPIO_COMBO_BGF_EINT_PIN, GPIO_DIR_IN);
    mt_set_gpio_pull_select(GPIO_COMBO_BGF_EINT_PIN, GPIO_PULL_DOWN);
    mt_set_gpio_pull_enable(GPIO_COMBO_BGF_EINT_PIN, GPIO_PULL_ENABLE);

    //printk(KERN_INFO "[mt6620] ALL_EINT NC\n");
#ifdef GPIO_COMBO_ALL_EINT_PIN
    //printk(KERN_INFO "[mt6620] set ALL_EINT input pull down\n");
    mt_set_gpio_mode(GPIO_COMBO_ALL_EINT_PIN, GPIO_MODE_00);
    mt_set_gpio_dir(GPIO_COMBO_ALL_EINT_PIN, GPIO_DIR_IN);
    mt_set_gpio_pull_select(GPIO_COMBO_ALL_EINT_PIN, GPIO_PULL_DOWN);
    mt_set_gpio_pull_enable(GPIO_COMBO_ALL_EINT_PIN, GPIO_PULL_ENABLE);
#endif
    //printk(KERN_INFO "[mt6620] set COMBO_AUDIO_STATE_0\n");
    mt_combo_audio_ctrl_ex(COMBO_AUDIO_STATE_0, 0);

//MT6620 I2S0 related pin defination has been added to the .dws file
#ifndef FM_ANALOG_INPUT
	mt_set_gpio_mode(GPIO_I2S0_CK_PIN, GPIO_I2S0_CK_PIN_M_GPIO);
    mt_set_gpio_dir(GPIO_I2S0_CK_PIN, GPIO_DIR_IN);
    mt_set_gpio_pull_select(GPIO_I2S0_CK_PIN, GPIO_PULL_DOWN);
    mt_set_gpio_pull_enable(GPIO_I2S0_CK_PIN, GPIO_PULL_ENABLE);
    mt_set_gpio_mode(GPIO_I2S0_WS_PIN, GPIO_I2S0_WS_PIN_M_GPIO);
    mt_set_gpio_dir(GPIO_I2S0_WS_PIN, GPIO_DIR_IN);
    mt_set_gpio_pull_select(GPIO_I2S0_WS_PIN, GPIO_PULL_DOWN);
    mt_set_gpio_pull_enable(GPIO_I2S0_WS_PIN, GPIO_PULL_ENABLE);
    mt_set_gpio_mode(GPIO_I2S0_DAT_PIN, GPIO_I2S0_DAT_PIN_M_GPIO);
    mt_set_gpio_dir(GPIO_I2S0_DAT_PIN, GPIO_DIR_IN);
    mt_set_gpio_pull_select(GPIO_I2S0_DAT_PIN, GPIO_PULL_DOWN);
    mt_set_gpio_pull_enable(GPIO_I2S0_DAT_PIN, GPIO_PULL_ENABLE);
#endif    
    //printk(KERN_INFO "[mt6620] set SYSRST_B 0 and PMU_EN 0 \n");
    /* SYSRST_B low */
    mt_set_gpio_out(GPIO_COMBO_RST_PIN, GPIO_OUT_ZERO);
    /* PMU_EN low */
	mt_set_gpio_out(GPIO_COMBO_PMU_EN_PIN, GPIO_OUT_ZERO);

#if defined(CONFIG_MTK_COMBO_SDIO_SLOT) && (CONFIG_MTK_COMBO_SDIO_SLOT == 0)
	printk(KERN_INFO "[mt6620] pull down sd0 bus(gpio169~gpio175(exclude gpio174))\n");
    mt_set_gpio_pull_enable(GPIO172, GPIO_PULL_DOWN);	//->CLK
    mt_set_gpio_pull_select(GPIO172, GPIO_PULL_ENABLE);		
    mt_set_gpio_pull_enable(GPIO171, GPIO_PULL_DOWN);	//->CMD
    mt_set_gpio_pull_select(GPIO171, GPIO_PULL_ENABLE);
    mt_set_gpio_pull_enable(GPIO175, GPIO_PULL_DOWN);	//->DAT0
    mt_set_gpio_pull_select(GPIO175, GPIO_PULL_ENABLE);
    mt_set_gpio_pull_enable(GPIO173, GPIO_PULL_DOWN);	//->DAT1
    mt_set_gpio_pull_select(GPIO173, GPIO_PULL_ENABLE);
    mt_set_gpio_pull_enable(GPIO169, GPIO_PULL_DOWN);	//->DAT2
    mt_set_gpio_pull_select(GPIO169, GPIO_PULL_ENABLE);
    mt_set_gpio_pull_enable(GPIO170, GPIO_PULL_DOWN);	//->DAT3
    mt_set_gpio_pull_select(GPIO170, GPIO_PULL_ENABLE);
#elif (CONFIG_MTK_COMBO_SDIO_SLOT == 1)
	#error "error:MSDC1 is not reserved for MT6620 on MT6575EVB"
#elif (CONFIG_MTK_COMBO_SDIO_SLOT == 2)
    printk(KERN_INFO "[mt6620] pull down sd2 bus(gpio182~187)\n");
    mt_set_gpio_pull_enable(GPIO182, GPIO_PULL_DOWN);	//->CLK
    mt_set_gpio_pull_select(GPIO182, GPIO_PULL_ENABLE);		
    mt_set_gpio_pull_enable(GPIO184, GPIO_PULL_DOWN);	//->CMD
    mt_set_gpio_pull_select(GPIO184, GPIO_PULL_ENABLE);
    mt_set_gpio_pull_enable(GPIO186, GPIO_PULL_DOWN);	//->DAT0
    mt_set_gpio_pull_select(GPIO186, GPIO_PULL_ENABLE);
    mt_set_gpio_pull_enable(GPIO187, GPIO_PULL_DOWN);	//->DAT1
    mt_set_gpio_pull_select(GPIO187, GPIO_PULL_ENABLE);
    mt_set_gpio_pull_enable(GPIO185, GPIO_PULL_DOWN);	//->DAT2
    mt_set_gpio_pull_select(GPIO185, GPIO_PULL_ENABLE);
    mt_set_gpio_pull_enable(GPIO183, GPIO_PULL_DOWN);	//->DAT3
    mt_set_gpio_pull_select(GPIO183, GPIO_PULL_ENABLE);
#elif (CONFIG_MTK_COMBO_SDIO_SLOT == 3)
	printk(KERN_INFO "[mt6620] pull down sd3 bus (GPIO89~GPIO94)\n");
    mt_set_gpio_pull_enable(GPIO92, GPIO_PULL_DOWN);	//->CLK
    mt_set_gpio_pull_select(GPIO92, GPIO_PULL_ENABLE);		
    mt_set_gpio_pull_enable(GPIO91, GPIO_PULL_DOWN);	//->CMD
    mt_set_gpio_pull_select(GPIO91, GPIO_PULL_ENABLE);
    mt_set_gpio_pull_enable(GPIO94, GPIO_PULL_DOWN);	//->DAT0
    mt_set_gpio_pull_select(GPIO94, GPIO_PULL_ENABLE);
    mt_set_gpio_pull_enable(GPIO90, GPIO_PULL_DOWN);	//->DAT1
    mt_set_gpio_pull_select(GPIO90, GPIO_PULL_ENABLE);
    mt_set_gpio_pull_enable(GPIO89, GPIO_PULL_DOWN);	//->DAT2
    mt_set_gpio_pull_select(GPIO89, GPIO_PULL_ENABLE);
    mt_set_gpio_pull_enable(GPIO93, GPIO_PULL_DOWN);	//->DAT3
    mt_set_gpio_pull_select(GPIO93, GPIO_PULL_ENABLE);
#else
	#error "error:unsupported CONFIG_MTK_COMBO_SDIO_SLOT" CONFIG_MTK_COMBO_SDIO_SLOT
#endif
    //printk(KERN_INFO "[mt6620] set UART GPIO Mode output 0\n");
    mt_set_gpio_mode(GPIO_UART_URXD3_PIN, GPIO_UART_URXD3_PIN_M_GPIO);
    mt_set_gpio_dir(GPIO_UART_URXD3_PIN, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_UART_URXD3_PIN, GPIO_OUT_ZERO);

    mt_set_gpio_mode(GPIO_UART_UTXD3_PIN, GPIO_UART_UTXD3_PIN_M_GPIO);
    mt_set_gpio_dir(GPIO_UART_UTXD3_PIN, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_UART_UTXD3_PIN, GPIO_OUT_ZERO);

    //printk(KERN_INFO "[mt6620] disable RTC CLK \n");
	printk("[mt6620]not to rtc_gpio_disable_32k(RTC_GPIO_USER_GPS)  \n");
    return;
}
EXPORT_SYMBOL(mt6620_power_off);

int mt_combo_gps_sync_pin_ctrl(int on)
{
    printk("[mt6620]ignore gps_sync function on mt6575_evb project.\n");
}
EXPORT_SYMBOL(mt_combo_gps_sync_pin_ctrl);

static void combo_bt_pcm_pin_on(void)
{
    mt_set_gpio_mode(GPIO_PCM_DAICLK_PIN, GPIO_PCM_DAICLK_PIN_M_CLK);
    mt_set_gpio_mode(GPIO_PCM_DAIPCMOUT_PIN, GPIO_PCM_DAIPCMOUT_PIN_M_DAIPCMOUT);
    mt_set_gpio_mode(GPIO_PCM_DAIPCMIN_PIN, GPIO_PCM_DAIPCMIN_PIN_M_DAIPCMIN);
    mt_set_gpio_mode(GPIO_PCM_DAISYNC_PIN, GPIO_PCM_DAISYNC_PIN_M_BTSYNC);	
}

static void combo_bt_pcm_pin_off(void)
{
    mt_set_gpio_mode(GPIO_PCM_DAICLK_PIN, GPIO_MODE_00);
    mt_set_gpio_dir(GPIO_PCM_DAICLK_PIN, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_PCM_DAICLK_PIN, GPIO_OUT_ZERO);

    mt_set_gpio_mode(GPIO_PCM_DAIPCMOUT_PIN, GPIO_MODE_00);
    mt_set_gpio_dir(GPIO_PCM_DAIPCMOUT_PIN, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_PCM_DAIPCMOUT_PIN, GPIO_OUT_ZERO);

    mt_set_gpio_mode(GPIO_PCM_DAIPCMIN_PIN, GPIO_MODE_00);
    mt_set_gpio_dir(GPIO_PCM_DAIPCMIN_PIN, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_PCM_DAIPCMIN_PIN, GPIO_OUT_ZERO);

    mt_set_gpio_mode(GPIO_PCM_DAISYNC_PIN, GPIO_MODE_00);
    mt_set_gpio_dir(GPIO_PCM_DAISYNC_PIN, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_PCM_DAISYNC_PIN, GPIO_OUT_ZERO);
}

static void combo_fm_i2s_pin_on(void)
{
#ifndef FM_ANALOG_INPUT
    mt_set_gpio_mode(GPIO_I2S0_CK_PIN, GPIO_MODE_01);	//GPIO_MODE_01->I2S0 mode
    mt_set_gpio_mode(GPIO_I2S0_WS_PIN, GPIO_MODE_01);	//GPIO_MODE_01->I2S0 mode
    mt_set_gpio_mode(GPIO_I2S0_DAT_PIN, GPIO_MODE_01);	//GPIO_MODE_01->I2S0 mode
#else
	printk("[mt6620]warnning:fm analog input mode is set, combo_fm_i2s_pin_on should not be called. \n");
#endif
}

static void combo_fm_i2s_pin_off(void)
{
#ifndef FM_ANALOG_INPUT
    mt_set_gpio_mode(GPIO_I2S0_CK_PIN, GPIO_MODE_00);
    mt_set_gpio_dir(GPIO_I2S0_CK_PIN, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_I2S0_CK_PIN, GPIO_OUT_ZERO);

    mt_set_gpio_mode(GPIO_I2S0_WS_PIN, GPIO_MODE_00);
    mt_set_gpio_dir(GPIO_I2S0_WS_PIN, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_I2S0_WS_PIN, GPIO_OUT_ZERO);

    mt_set_gpio_mode(GPIO_I2S0_DAT_PIN, GPIO_MODE_00);
    mt_set_gpio_dir(GPIO_I2S0_DAT_PIN, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_I2S0_DAT_PIN, GPIO_OUT_ZERO);
#else
	printk("[mt6620]warnning:fm analog input mode is set, combo_fm_i2s_pin_off should not be called.\n");
#endif    
}



void combo_audio_pin_conf(COMBO_AUDIO_STATE state)
{
    printk(KERN_INFO "combo_audio_pin_conf, state = [%d]\n", state);
    switch(state)
    {
		case COMBO_AUDIO_STATE_0:
			/*BT_PCM_OFF*/ 
			combo_bt_pcm_pin_off();
			/*FM_I2S0_OFF*/
			combo_fm_i2s_pin_off();
	    	break;
		case COMBO_AUDIO_STATE_1:
			/* BT_PCM_ON */
			combo_bt_pcm_pin_on();
			/*FM_I2S0_OFF*/
			combo_fm_i2s_pin_off();
	    	break;
		case COMBO_AUDIO_STATE_2:
	    	/*FM_I2S_ON*/
			combo_fm_i2s_pin_on();
	    	/*BT_PCM_OFF*/ 
			combo_bt_pcm_pin_off();
	    	break;
		case COMBO_AUDIO_STATE_3:
	    	/*FM_I2S_ON*/
			combo_fm_i2s_pin_on();
	    	/* BT_PCM_ON */
			combo_bt_pcm_pin_on();
		break;
			printk(KERN_INFO "combo_audio_pin_conf,warnning: invalid state(%d)\n", state);
		default:
		break;
    }
}

#if defined(CONFIG_MTK_COMBO_SDIO_SLOT)
	static void combo_sdio_enable_eirq(void)
	{
	    mt65xx_eint_unmask(combo_sdio_eint_num);/* CUST_EINT_WIFI_NUM */
	}
	
	static void combo_sdio_disable_eirq(void)
	{
	    mt65xx_eint_mask(combo_sdio_eint_num); /* CUST_EINT_WIFI_NUM */
	}
	
	static void combo_sdio_eirq_handler_stub(void)
	{
	    if (combo_sdio_eirq_handler) {
	        combo_sdio_eirq_handler(combo_sdio_eirq_data);
	    }
	}
	
	static void combo_sdio_request_eirq(sdio_irq_handler_t irq_handler, void *data)
	{
	    mt65xx_eint_set_sens(combo_sdio_eint_num, CUST_EINT_WIFI_SENSITIVE); /*CUST_EINT_WIFI_NUM */
	    mt65xx_eint_set_hw_debounce(combo_sdio_eint_num, CUST_EINT_WIFI_DEBOUNCE_CN); /*CUST_EINT_WIFI_NUM */
	    mt65xx_eint_registration(combo_sdio_eint_num/*CUST_EINT_WIFI_NUM */,
	        CUST_EINT_WIFI_DEBOUNCE_EN,
	        CUST_EINT_WIFI_POLARITY,
	        combo_sdio_eirq_handler_stub,
	        0);
	    mt65xx_eint_mask(combo_sdio_eint_num);/*CUST_EINT_WIFI_NUM */
	
	    combo_sdio_eirq_handler = irq_handler;
	    combo_sdio_eirq_data    = data;
	}
	
	static void combo_sdio_register_pm(pm_callback_t pm_cb, void *data)
	{
	    /*printk( KERN_INFO "combo_sdio_register_pm (0x%p, 0x%p)\n", pm_cb, data);*/
	    /* register pm change callback */
	    combo_sdio_pm_cb = pm_cb;
	    combo_sdio_pm_data = data;
	}
	
	static void combo_sdio_on (int sdio_port_num) {
	    pm_message_t state = { .event = PM_EVENT_USER_RESUME };
	
	    printk(KERN_INFO "combo_sdio_on (%d) \n", sdio_port_num);
	
	    /* 1. disable sdio eirq */
	    combo_sdio_disable_eirq();
	    mt_set_gpio_pull_enable(combo_sdio_eint_pin, GPIO_PULL_DISABLE); /* GPIO_WIFI_EINT_PIN */
	    mt_set_gpio_mode(combo_sdio_eint_pin, combo_sdio_eint_m_eint); /* EINT mode */
	
	    /* 2. call sd callback */
	    if (combo_sdio_pm_cb) {
	        printk(KERN_INFO "combo_sdio_pm_cb(PM_EVENT_USER_RESUME, 0x%p, 0x%p) \n", combo_sdio_pm_cb, combo_sdio_pm_data);
	        combo_sdio_pm_cb(state, combo_sdio_pm_data);
	    }
	    else {
	        printk(KERN_WARNING "combo_sdio_on no sd callback!!\n");
	    }
	}
	
	static void combo_sdio_off (int sdio_port_num) {
	    pm_message_t state = { .event = PM_EVENT_USER_SUSPEND };
	
	    printk(KERN_INFO "combo_sdio_off (%d) \n", sdio_port_num);
	
	    /* 1. call sd callback */
	    if (combo_sdio_pm_cb) {
	        printk(KERN_INFO "combo_sdio_off(PM_EVENT_USER_SUSPEND, 0x%p, 0x%p) \n", combo_sdio_pm_cb, combo_sdio_pm_data);
	        combo_sdio_pm_cb(state, combo_sdio_pm_data);
	    }
	    else {
	        printk(KERN_WARNING "combo_sdio_off no sd callback!!\n");
	    }
	
	    /* 2. disable sdio eirq */
	    combo_sdio_disable_eirq();
	    /*printk(KERN_INFO "[mt6620] set WIFI_EINT input pull down\n");*/
	    mt_set_gpio_mode(combo_sdio_eint_pin, combo_sdio_eint_m_gpio); /* GPIO mode */
	    mt_set_gpio_dir(combo_sdio_eint_pin, GPIO_DIR_IN);
	    mt_set_gpio_pull_select(combo_sdio_eint_pin, GPIO_PULL_DOWN);
	    mt_set_gpio_pull_enable(combo_sdio_eint_pin, GPIO_PULL_ENABLE);
	}
	
	int mt_combo_sdio_ctrl (unsigned int sdio_port_num, unsigned int on) {
	/*FIXME: set sdio_port_num to CONFIG_MTK_COMBO_SDIO_SLOT*/
		#if defined(CONFIG_MTK_COMBO_SDIO_SLOT)
			sdio_port_num = CONFIG_MTK_COMBO_SDIO_SLOT;
			printk(KERN_WARNING "mt_combo_sdio_ctrl: force set sdio port to (%d)\n", sdio_port_num);
		#endif
	    if ((sdio_port_num >= 4) || (combo_port_pwr_map[sdio_port_num] == 0xFF) ) {
	        /* invalid sdio port number or slot mapping */
	        printk(KERN_WARNING "mt_combo_sdio_ctrl invalid port(%d, %d)\n", sdio_port_num, combo_port_pwr_map[sdio_port_num]);
	        return -1;
	    }
	    /*printk(KERN_INFO "mt_combo_sdio_ctrl (%d, %d)\n", sdio_port_num, on);*/
	
	    if (!combo_port_pwr_map[sdio_port_num] && on) {
			/* on -> off */
			printk(KERN_INFO "mt_combo_sdio_ctrl:force [sdio%d] off before on\n", sdio_port_num);
	        combo_sdio_off(sdio_port_num);
	        combo_port_pwr_map[sdio_port_num] = 0;
	        /* off -> on */
	        combo_sdio_on(sdio_port_num);
	        combo_port_pwr_map[sdio_port_num] = 1;
	    }
	    else if (combo_port_pwr_map[sdio_port_num] && !on) {
	        /* on -> off */
	        combo_sdio_off(sdio_port_num);
	        combo_port_pwr_map[sdio_port_num] = 0;
	    }
	    else {
	        return -2;
	    }
	    return 0;
	}
#else
	int mt_combo_sdio_ctrl (unsigned int sdio_port_num, unsigned int on) {
	    printk(KERN_WARNING "mt_combo_sdio_ctrl but CONFIG_MTK_COMBO_SDIO_SLOT undefined!\n");
	    return -1;
	}

#endif /* end of defined(CONFIG_MTK_COMBO_SDIO_SLOT) */
EXPORT_SYMBOL(mt_combo_sdio_ctrl);

int mt_combo_gps_lna_pin_ctrl(unsigned int on)
{
	printk(KERN_INFO "%s:(MT6575 platform, this operation is omitted @EVB project)\n", __FUNCTION__);
	return 0;
}
EXPORT_SYMBOL(mt_combo_gps_lna_pin_ctrl);
#endif /* end of  defined(CONFIG_MTK_COMBO) || defined(CONFIG_MTK_COMBO_MODULE) */
#endif
#if defined(CONFIG_WLAN)
    #if !defined(CONFIG_MTK_COMBO_SDIO_SLOT)
    #endif /* end of !defined(CONFIG_MTK_COMBO_SDIO_SLOT) */

int mt_wifi_resume(pm_message_t state)
{
    int evt = state.event;

    if (evt != PM_EVENT_USER_RESUME && evt != PM_EVENT_RESUME) {
        return -1;
    }

    /*printk(KERN_INFO "[WIFI] %s Resume\n", evt == PM_EVENT_RESUME ? "PM":"USR");*/

#if defined(CONFIG_MTK_COMBO) || defined(CONFIG_MTK_COMBO_MODULE)
    /* combo chip product: notify combo driver to turn on Wi-Fi */
    mtk_wcn_cmb_stub_func_ctrl(COMBO_FUNC_TYPE_WIFI, 1);

#else /* standalone product */
#endif

    return 0;
}

int mt_wifi_suspend(pm_message_t state)
{
    int evt = state.event;
#if defined(CONFIG_MTK_COMBO_SDIO_SLOT)
    static int is_1st_suspend_from_boot = 1;
#endif

    if (evt != PM_EVENT_USER_SUSPEND && evt != PM_EVENT_SUSPEND) {
        return -1;
    }

#if defined(CONFIG_MTK_COMBO) || defined(CONFIG_MTK_COMBO_MODULE)
    #if defined(CONFIG_MTK_COMBO_SDIO_SLOT)
    /* combo chip product: notify combo driver to turn on Wi-Fi */
    if (is_1st_suspend_from_boot) {
        pm_message_t state = { .event = PM_EVENT_USER_SUSPEND };

        if (combo_sdio_pm_cb) {
            is_1st_suspend_from_boot = 0;
            /*              *** IMPORTANT DEPENDENDY***
            RFKILL: set wifi and bt suspend by default in probe()
            MT6573-SD: sd host is added to MMC stack and suspend is ZERO by default
            (which means NOT suspended).

            When boot up, RFKILL will set wifi off and this function gets
            called. In order to successfully resume wifi at 1st time, pm_cb here
            shall be called once to let MT6573-SD do sd host suspend and remove
            sd host from MMC. Then wifi can be turned on successfully.

            Boot->SD host added to MMC (suspend=0)->RFKILL set wifi off
            ->SD host removed from MMC (suspend=1)->RFKILL set wifi on
            */
            printk(KERN_INFO "1st mt_wifi_suspend (PM_EVENT_USER_SUSPEND) \n");
            combo_sdio_pm_cb(state, combo_sdio_pm_data);
        }
        else {
            printk(KERN_WARNING "1st mt_wifi_suspend but no sd callback!!\n");
        }
    }
    else {
        /* combo chip product, notify combo driver */
        mtk_wcn_cmb_stub_func_ctrl(COMBO_FUNC_TYPE_WIFI, 0);
    }
    #endif

#else
#endif
    return 0;
}

void mt_wifi_power_on(void)
{
    pm_message_t state = { .event = PM_EVENT_USER_RESUME };

    (void)mt_wifi_resume(state);
}
EXPORT_SYMBOL(mt_wifi_power_on);

void mt_wifi_power_off(void)
{
    pm_message_t state = { .event = PM_EVENT_USER_SUSPEND };

    (void)mt_wifi_suspend(state);
}
EXPORT_SYMBOL(mt_wifi_power_off);

#endif /* end of defined(CONFIG_WLAN) */

/* Board Specific Devices                                                */
/*=======================================================================*/

/*=======================================================================*/
/* Board Specific Devices Init                                           */
/*=======================================================================*/

/*=======================================================================*/
/* Board Devices Capability                                              */
/*=======================================================================*/
#define MSDC_SDCARD_FLAG  (MSDC_SYS_SUSPEND | MSDC_WP_PIN_EN | MSDC_CD_PIN_EN | MSDC_REMOVABLE | MSDC_HIGHSPEED)
#define MSDC_SDIO_FLAG    (MSDC_EXT_SDIO_IRQ | MSDC_HIGHSPEED)

#if defined(CFG_DEV_MSDC0)
    #if defined(CONFIG_MTK_COMBO_SDIO_SLOT) && (CONFIG_MTK_COMBO_SDIO_SLOT == 0)
    struct msdc_hw msdc0_hw = {	    
		.clk_src        = MSDC_CLKSRC_200MHZ,
	    .cmd_edge       = MSDC_SMPL_FALLING,
	    .rdata_edge     = MSDC_SMPL_FALLING,
	    .wdata_edge     = MSDC_SMPL_FALLING,
	    .clk_drv        = 0,
	    .cmd_drv        = 0,
	    .dat_drv        = 0,
	    .data_pins      = 4,
	    .data_offset    = 0,
	    //MT6620 use External IRQ, wifi uses high speed. here wifi manage his own suspend and resume, does not support hot plug
	    .flags          = MSDC_SDIO_FLAG,//MSDC_SYS_SUSPEND | MSDC_WP_PIN_EN | MSDC_CD_PIN_EN | MSDC_REMOVABLE,(this flag is for SD card)
	    .dat0rddly		= 0,
			.dat1rddly		= 0,
			.dat2rddly		= 0,
			.dat3rddly		= 0,
			.dat4rddly		= 0,
			.dat5rddly		= 0,
			.dat6rddly		= 0,
			.dat7rddly		= 0,
			.datwrddly		= 0,
			.cmdrrddly		= 0,
			.cmdrddly		= 0,
	    .host_function	= MSDC_SDIO,
	    .boot			= 0,
	    .request_sdio_eirq = combo_sdio_request_eirq,
	    .enable_sdio_eirq  = combo_sdio_enable_eirq,
	    .disable_sdio_eirq = combo_sdio_disable_eirq,
	    .register_pm       = combo_sdio_register_pm,
	};
	#else
	struct msdc_hw msdc0_hw = {
	    .clk_src        = MSDC_CLKSRC_200MHZ,
	    .cmd_edge       = MSDC_SMPL_FALLING,
		.rdata_edge 	= MSDC_SMPL_FALLING,
		.wdata_edge 	= MSDC_SMPL_FALLING,
	    .clk_drv        = 4,
	    .cmd_drv        = 2,
	    .dat_drv        = 2,
	    .data_pins      = 8,
	    .data_offset    = 0,
	    .flags          = MSDC_SYS_SUSPEND | MSDC_HIGHSPEED | MSDC_UHS1 | MSDC_DDR,
	    .dat0rddly		= 0xa,
			.dat1rddly		= 0,
			.dat2rddly		= 0,
			.dat3rddly		= 0,
			.dat4rddly		= 0,
			.dat5rddly		= 0,
			.dat6rddly		= 0,
			.dat7rddly		= 0,
			.datwrddly		= 0,
			.cmdrrddly		= 0,
			.cmdrddly		= 0,
	    .host_function	= MSDC_EMMC,
	    .boot			= MSDC_BOOT_EN,
	};
	#endif
#endif
#if defined(CFG_DEV_MSDC1)
struct msdc_hw msdc1_hw = {
    .clk_src        = MSDC_CLKSRC_200MHZ,
    .cmd_edge       = MSDC_SMPL_FALLING,
    .rdata_edge     = MSDC_SMPL_FALLING,
    .wdata_edge     = MSDC_SMPL_FALLING,
    .clk_drv        = 4,
    .cmd_drv        = 3,
    .dat_drv        = 3,
    .clk_drv_sd_18	= 4,               /* sdr104 mode */
    .cmd_drv_sd_18	= 4,
    .dat_drv_sd_18	= 4,
    .clk_drv_sd_18_sdr50	= 4,       /* sdr50 mode */
    .cmd_drv_sd_18_sdr50	= 4,
    .dat_drv_sd_18_sdr50	= 4,
    .clk_drv_sd_18_ddr50	= 4,       /* ddr50 mode */
    .cmd_drv_sd_18_ddr50	= 4,
    .dat_drv_sd_18_ddr50	= 4,
    .data_pins      = 4,
    .data_offset    = 0,
#ifdef CUST_EINT_MSDC1_INS_NUM
    .flags          = MSDC_SYS_SUSPEND | MSDC_WP_PIN_EN | MSDC_CD_PIN_EN | MSDC_REMOVABLE | MSDC_HIGHSPEED | MSDC_UHS1 |MSDC_DDR,
    
#else
    .flags          = MSDC_SYS_SUSPEND | MSDC_WP_PIN_EN | MSDC_HIGHSPEED | MSDC_UHS1 |MSDC_DDR,   
#endif
  	  .dat0rddly		= 0,
			.dat1rddly		= 0,
			.dat2rddly		= 0,
			.dat3rddly		= 0,
			.dat4rddly		= 0,
			.dat5rddly		= 0,
			.dat6rddly		= 0,
			.dat7rddly		= 0,
			.datwrddly		= 0,
			.cmdrrddly		= 0,
			.cmdrddly		= 0,
    .host_function	= MSDC_SD,
    .boot			= 0,
    //LINE <Tcard> <DATE20130724> <T card level high> zhangxiaofei
    .cd_level		= MSDC_CD_HIGH,
};

#endif
#if defined(CFG_DEV_MSDC2)
    #if defined(CONFIG_MTK_COMBO_SDIO_SLOT) && (CONFIG_MTK_COMBO_SDIO_SLOT == 2)
    /* MSDC2 settings for MT66xx combo connectivity chip */
	struct msdc_hw msdc2_hw = {	    
		.clk_src        = MSDC_CLKSRC_200MHZ,
	    .cmd_edge       = MSDC_SMPL_FALLING,
	    .rdata_edge     = MSDC_SMPL_FALLING,
    	.wdata_edge     = MSDC_SMPL_FALLING,
	    .clk_drv        = 0,
    	.cmd_drv        = 0,
    	.dat_drv        = 0,
	    .data_pins      = 4,
	    .data_offset    = 0,
	    //MT6620 use External IRQ, wifi uses high speed. here wifi manage his own suspend and resume, does not support hot plug
	    .flags          = MSDC_SDIO_FLAG,//MSDC_SYS_SUSPEND | MSDC_WP_PIN_EN | MSDC_CD_PIN_EN | MSDC_REMOVABLE,
	    .dat0rddly		= 0,
			.dat1rddly		= 0,
			.dat2rddly		= 0,
			.dat3rddly		= 0,
			.dat4rddly		= 0,
			.dat5rddly		= 0,
			.dat6rddly		= 0,
			.dat7rddly		= 0,
			.datwrddly		= 0,
			.cmdrrddly		= 0,
			.cmdrddly		= 0,
	    .host_function	= MSDC_SDIO,
	    .boot			= 0,
	    .request_sdio_eirq = combo_sdio_request_eirq,
	    .enable_sdio_eirq  = combo_sdio_enable_eirq,
	    .disable_sdio_eirq = combo_sdio_disable_eirq,
	    .register_pm       = combo_sdio_register_pm,
	};
    #else

#if 0
struct msdc_hw msdc2_hw = {
    .clk_src        = MSDC_CLKSRC_200MHZ,
    .cmd_edge       = MSDC_SMPL_FALLING,
    .rdata_edge     = MSDC_SMPL_FALLING,
    .wdata_edge     = MSDC_SMPL_FALLING,
    .clk_drv        = 3,
    .cmd_drv        = 2,
    .dat_drv        = 2,
    .clk_drv_sd_18	= 4,
    .cmd_drv_sd_18	= 3,
    .dat_drv_sd_18	= 3,
    .data_pins      = 4,
    .data_offset    = 0,
    .flags          = MSDC_SYS_SUSPEND | MSDC_WP_PIN_EN | MSDC_CD_PIN_EN | MSDC_REMOVABLE | MSDC_HIGHSPEED,
  	  .dat0rddly		= 0,
			.dat1rddly		= 0,
			.dat2rddly		= 0,
			.dat3rddly		= 0,
			.dat4rddly		= 0,
			.dat5rddly		= 0,
			.dat6rddly		= 0,
			.dat7rddly		= 0,
			.datwrddly		= 0,
			.cmdrrddly		= 0,
			.cmdrddly		= 0,
    .host_function	= MSDC_SD,
    .boot			= 0,
     //LINE <Tcard> <DATE20130724> <T card level high> zhangxiaofei
    .cd_level		= MSDC_CD_HIGH,
};
#endif
    #endif
#endif
#if defined(CFG_DEV_MSDC3)
    #if defined(CONFIG_MTK_COMBO_SDIO_SLOT) && (CONFIG_MTK_COMBO_SDIO_SLOT == 3)
    /* MSDC3 settings for MT66xx combo connectivity chip */
    struct msdc_hw msdc3_hw = {
	    .clk_src        = MSDC_CLKSRC_200MHZ,
	    .cmd_edge       = MSDC_SMPL_FALLING,
	    .rdata_edge     = MSDC_SMPL_FALLING,
    	.wdata_edge     = MSDC_SMPL_FALLING,
	    .clk_drv        = 0,
	    .cmd_drv        = 0,
	    .dat_drv        = 0,
	    .data_pins      = 4,
	    .data_offset    = 0,
	    //MT6620 use External IRQ, wifi uses high speed. here wifi manage his own suspend and resume, does not support hot plug
	    .flags          = MSDC_SDIO_FLAG,//MSDC_SYS_SUSPEND | MSDC_WP_PIN_EN | MSDC_CD_PIN_EN | MSDC_REMOVABLE,
	    .dat0rddly		= 0,
			.dat1rddly		= 0,
			.dat2rddly		= 0,
			.dat3rddly		= 0,
			.dat4rddly		= 0,
			.dat5rddly		= 0,
			.dat6rddly		= 0,
			.dat7rddly		= 0,
			.datwrddly		= 0,
			.cmdrrddly		= 0,
			.cmdrddly		= 0,
	    .host_function	= MSDC_SDIO,
	    .boot			= 0,
	    .request_sdio_eirq = combo_sdio_request_eirq,
	    .enable_sdio_eirq  = combo_sdio_enable_eirq,
	    .disable_sdio_eirq = combo_sdio_disable_eirq,
	    .register_pm       = combo_sdio_register_pm,
	};
	#if 0
    
	struct msdc_hw msdc3_hw = {
	    .clk_src        = 1,
	    .cmd_edge       = MSDC_SMPL_FALLING,
	    .data_edge      = MSDC_SMPL_FALLING,
	    .clk_drv        = 4,
	    .cmd_drv        = 4,
	    .dat_drv        = 4,
	    .data_pins      = 4,
	    .data_offset    = 0,
	    .flags          = MSDC_SDCARD_FLAG,
	    .dat0rddly		= 0,
			.dat1rddly		= 0,
			.dat2rddly		= 0,
			.dat3rddly		= 0,
			.dat4rddly		= 0,
			.dat5rddly		= 0,
			.dat6rddly		= 0,
			.dat7rddly		= 0,
			.datwrddly		= 0,
			.cmdrrddly		= 0,
			.cmdrddly		= 0,
	};
	#endif
	#endif
#endif
#if defined(CFG_DEV_MSDC4)
    #if defined(CONFIG_MTK_COMBO_SDIO_SLOT) && (CONFIG_MTK_COMBO_SDIO_SLOT == 0)
    struct msdc_hw msdc4_hw = {	    
		.clk_src        = MSDC_CLKSRC_200MHZ,
	    .cmd_edge       = MSDC_SMPL_FALLING,
	    .rdata_edge     = MSDC_SMPL_FALLING,
    	.wdata_edge     = MSDC_SMPL_FALLING,
	    .clk_drv        = 0,
	    .cmd_drv        = 0,
	    .dat_drv        = 0,
	    .data_pins      = 4,
	    .data_offset    = 0,
	    //MT6620 use External IRQ, wifi uses high speed. here wifi manage his own suspend and resume, does not support hot plug
	    .flags          = MSDC_SDIO_FLAG,//MSDC_SYS_SUSPEND | MSDC_WP_PIN_EN | MSDC_CD_PIN_EN | MSDC_REMOVABLE,(this flag is for SD card)
	    .dat0rddly		= 0,
			.dat1rddly		= 0,
			.dat2rddly		= 0,
			.dat3rddly		= 0,
			.dat4rddly		= 0,
			.dat5rddly		= 0,
			.dat6rddly		= 0,
			.dat7rddly		= 0,
			.datwrddly		= 0,
			.cmdrrddly		= 0,
			.cmdrddly		= 0,
	    .host_function	= MSDC_SDIO,
	    .boot			= 0,
	    .request_sdio_eirq = combo_sdio_request_eirq,
	    .enable_sdio_eirq  = combo_sdio_enable_eirq,
	    .disable_sdio_eirq = combo_sdio_disable_eirq,
	    .register_pm       = combo_sdio_register_pm,
	};
	#else
	struct msdc_hw msdc4_hw = {
	    .clk_src        = MSDC_CLKSRC_200MHZ,
	    .cmd_edge       = MSDC_SMPL_FALLING,
	    .rdata_edge     = MSDC_SMPL_FALLING,
    	.wdata_edge     = MSDC_SMPL_FALLING,
	    .clk_drv        = 1,
	    .cmd_drv        = 0,
	    .dat_drv        = 0,
	    .data_pins      = 8,
	    .data_offset    = 0,
	    .flags          = MSDC_SYS_SUSPEND | MSDC_HIGHSPEED | MSDC_UHS1 |MSDC_DDR,
	    .dat0rddly		= 0,
			.dat1rddly		= 0,
			.dat2rddly		= 0,
			.dat3rddly		= 0,
			.dat4rddly		= 0,
			.dat5rddly		= 0,
			.dat6rddly		= 0,
			.dat7rddly		= 0,
			.datwrddly		= 0,
			.cmdrrddly		= 0,
			.cmdrddly		= 0,
	    .host_function	= MSDC_EMMC,
	    .boot			= MSDC_BOOT_EN,
	};
	#endif
#endif

/* MT6575 NAND Driver */
#if defined(CONFIG_MTK_MTD_NAND)
struct mt6575_nand_host_hw mt6575_nand_hw = {
    .nfi_bus_width          = 8,
	.nfi_access_timing		= NFI_DEFAULT_ACCESS_TIMING,
	.nfi_cs_num				= NFI_CS_NUM,
	.nand_sec_size			= 512,
	.nand_sec_shift			= 9,
	.nand_ecc_size			= 2048,
	.nand_ecc_bytes			= 32,
	.nand_ecc_mode			= NAND_ECC_HW,
};
#endif
