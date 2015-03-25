#include <mach/hal_pub_kpd.h>
#include <mach/hal_priv_kpd.h>

#define KPD_DEBUG	KPD_YES

#define KPD_SAY		"kpd: "
#if KPD_DEBUG
#define kpd_print(fmt, arg...)	printk(KPD_SAY fmt, ##arg)
#else
#define kpd_print(fmt, arg...)	do {} while (0)
#endif

extern struct input_dev *kpd_input_dev;

#if KPD_PWRKEY_USE_EINT
static u8 kpd_pwrkey_state = !KPD_PWRKEY_POLARITY;
#endif

static int kpd_show_hw_keycode = 1;
static int kpd_enable_lprst = 1;
static void mtk_kpd_get_gpio_col(unsigned int COL_REG[]);

static u16 kpd_keymap[KPD_NUM_KEYS] = KPD_INIT_KEYMAP();
static u16 kpd_keymap_state[KPD_NUM_MEMS] = {
	0xffff, 0xffff, 0xffff, 0xffff, 0x00ff
};



void kpd_slide_qwerty_init(void){
#if KPD_HAS_SLIDE_QWERTY
	bool evdev_flag=false;
	bool power_op=false;
	struct input_handler *handler;
	struct input_handle *handle;
	handle = rcu_dereference(dev->grab);
	if (handle)
	{
		handler = handle->handler;
		if(strcmp(handler->name, "evdev")==0) 
		{
			return -1;
		}	
	}
	else 
	{
		list_for_each_entry_rcu(handle, &dev->h_list, d_node) {
			handler = handle->handler;
			if(strcmp(handler->name, "evdev")==0) 
			{
				evdev_flag=true;
				break;
			}
		}
		if(evdev_flag==false)
		{
			return -1;	
		}	
	}

	power_op = powerOn_slidePin_interface();
	if(!power_op) {
		printk(KPD_SAY "Qwerty slide pin interface power on fail\n");
	} else {
		kpd_print("Qwerty slide pin interface power on success\n");
	}
		
	mt65xx_eint_set_sens(KPD_SLIDE_EINT, KPD_SLIDE_SENSITIVE);
	mt65xx_eint_set_hw_debounce(KPD_SLIDE_EINT, KPD_SLIDE_DEBOUNCE);
	mt65xx_eint_registration(KPD_SLIDE_EINT, true, KPD_SLIDE_POLARITY,
	                         kpd_slide_eint_handler, false);
	                         
	power_op = powerOff_slidePin_interface();
	if(!power_op) {
		printk(KPD_SAY "Qwerty slide pin interface power off fail\n");
	} else {
		kpd_print("Qwerty slide pin interface power off success\n");
	}
#endif
return;
}
/************************************************************/
/**************************************/
#ifdef CONFIG_MTK_LDVT	
void mtk_kpd_gpios_get(unsigned int ROW_REG[], unsigned int COL_REG[])
{
	int i;
	for(i = 0; i< 8; i++)
	{
		ROW_REG[i] = 0;
		COL_REG[i] = 0;
	}
	#ifdef GPIO_KPD_KROW0_PIN
		ROW_REG[0] = GPIO_KPD_KROW0_PIN;
	#endif

	#ifdef GPIO_KPD_KROW1_PIN
		ROW_REG[1] = GPIO_KPD_KROW1_PIN;
	#endif

	#ifdef GPIO_KPD_KROW2_PIN
		ROW_REG[2] = GPIO_KPD_KROW2_PIN;
	#endif

	#ifdef GPIO_KPD_KROW3_PIN
		ROW_REG[3] = GPIO_KPD_KROW3_PIN;
	#endif

	#ifdef GPIO_KPD_KROW4_PIN
		ROW_REG[4] = GPIO_KPD_KROW4_PIN;
	#endif

	#ifdef GPIO_KPD_KROW5_PIN
		ROW_REG[5] = GPIO_KPD_KROW5_PIN;
	#endif

	#ifdef GPIO_KPD_KROW6_PIN
		ROW_REG[6] = GPIO_KPD_KROW6_PIN;
	#endif

	#ifdef GPIO_KPD_KROW7_PIN
		ROW_REG[7] = GPIO_KPD_KROW7_PIN;
	#endif


	#ifdef GPIO_KPD_KCOL0_PIN
		COL_REG[0] = GPIO_KPD_KCOL0_PIN;
	#endif

	#ifdef GPIO_KPD_KCOL1_PIN
		COL_REG[1] = GPIO_KPD_KCOL1_PIN;
	#endif

	#ifdef GPIO_KPD_KCOL2_PIN
		COL_REG[2] = GPIO_KPD_KCOL2_PIN;
	#endif

	#ifdef GPIO_KPD_KCOL3_PIN
		COL_REG[3] = GPIO_KPD_KCOL3_PIN;
	#endif

	#ifdef GPIO_KPD_KCOL4_PIN
		COL_REG[4] = GPIO_KPD_KCOL4_PIN;
	#endif

	#ifdef GPIO_KPD_KCOL5_PIN
		COL_REG[5] = GPIO_KPD_KCOL5_PIN;
	#endif

	#ifdef GPIO_KPD_KCOL6_PIN
		COL_REG[6] = GPIO_KPD_KCOL6_PIN;
	#endif

	#ifdef GPIO_KPD_KCOL7_PIN
		COL_REG[7] = GPIO_KPD_KCOL7_PIN;
	#endif
}

void mtk_kpd_gpio_set(void)
{
	unsigned int ROW_REG[8];
	unsigned int COL_REG[8];
	int i;

	kpd_print("Enter mtk_kpd_gpio_set! \n");
	mtk_kpd_gpios_get(ROW_REG, COL_REG);
	
	for(i = 0; i < 8; i++)
	{
		if (COL_REG[i] != 0)
		{
			/* KCOL: GPIO INPUT + PULL ENABLE + PULL UP */
			mt_set_gpio_mode(COL_REG[i], 1);
			mt_set_gpio_dir(COL_REG[i], 0);
			mt_set_gpio_pull_enable(COL_REG[i], 1);
			mt_set_gpio_pull_select(COL_REG[i], 1);
		}
		
		if(ROW_REG[i] != 0)
		{
			/* KROW: GPIO output + pull disable + pull down */
			mt_set_gpio_mode(ROW_REG[i], 1);
			mt_set_gpio_dir(ROW_REG[i], 1);
			mt_set_gpio_pull_enable(ROW_REG[i], 0);	
			mt_set_gpio_pull_select(ROW_REG[i], 0);
		}
	}
}
#endif

void kpd_ldvt_test_init(void){
#ifdef CONFIG_MTK_LDVT	
    		//set kpd enable and sel register
		mt65xx_reg_sync_writew(0x0, KP_SEL);
		mt65xx_reg_sync_writew(0x1, KP_EN);
		//set kpd GPIO to kpd mode
		mtk_kpd_gpio_set();
#endif
return;
}

/*******************************kpd factory mode auto test *************************************/
static void mtk_kpd_get_gpio_col(unsigned int COL_REG[])
{
	int i;
	for(i = 0; i< 8; i++)
	{
		COL_REG[i] = 0;
	}
	kpd_print("Enter mtk_kpd_get_gpio_col! \n");
	
	#ifdef GPIO_KPD_KCOL0_PIN
		kpd_print("checking GPIO_KPD_KCOL0_PIN! \n");
		COL_REG[0] = GPIO_KPD_KCOL0_PIN;
	#endif

	#ifdef GPIO_KPD_KCOL1_PIN
		kpd_print("checking GPIO_KPD_KCOL1_PIN! \n");
		COL_REG[1] = GPIO_KPD_KCOL1_PIN;
	#endif

	#ifdef GPIO_KPD_KCOL2_PIN
		kpd_print("checking GPIO_KPD_KCOL2_PIN! \n");
		COL_REG[2] = GPIO_KPD_KCOL2_PIN;
	#endif

	#ifdef GPIO_KPD_KCOL3_PIN
		kpd_print("checking GPIO_KPD_KCOL3_PIN! \n");
		COL_REG[3] = GPIO_KPD_KCOL3_PIN;
	#endif

	#ifdef GPIO_KPD_KCOL4_PIN
		kpd_print("checking GPIO_KPD_KCOL4_PIN! \n");
		COL_REG[4] = GPIO_KPD_KCOL4_PIN;
	#endif

	#ifdef GPIO_KPD_KCOL5_PIN
		kpd_print("checking GPIO_KPD_KCOL5_PIN! \n");
		COL_REG[5] = GPIO_KPD_KCOL5_PIN;
	#endif

	#ifdef GPIO_KPD_KCOL6_PIN
		kpd_print("checking GPIO_KPD_KCOL6_PIN! \n");
		COL_REG[6] = GPIO_KPD_KCOL6_PIN;
	#endif

	#ifdef GPIO_KPD_KCOL7_PIN
		kpd_print("checking GPIO_KPD_KCOL7_PIN! \n");
		COL_REG[7] = GPIO_KPD_KCOL7_PIN;
	#endif
}

void kpd_auto_test_for_factorymode(void)
{
	unsigned int COL_REG[8];
	int i;
	int time = 500;
	
	kpd_pwrkey_pmic_handler(1);
	msleep(time);
	kpd_pwrkey_pmic_handler(0);
	
#ifdef KPD_PMIC_RSTKEY_MAP
	kpd_pmic_rstkey_handler(1);
	msleep(time);
	kpd_pmic_rstkey_handler(0);
#endif

	kpd_print("Enter kpd_auto_test_for_factorymode! \n");
	mtk_kpd_get_gpio_col(COL_REG);
	
	for(i = 0; i < 8; i++)
	{
		if (COL_REG[i] != 0)
		{
			msleep(time);
			kpd_print("kpd kcolumn %d pull down!\n", COL_REG[i]);
			mt_set_gpio_pull_select(COL_REG[i], 0);
			msleep(time);
			kpd_print("kpd kcolumn %d pull up!\n", COL_REG[i]);
			mt_set_gpio_pull_select(COL_REG[i], 1);
		}
	}
	return;
}
/********************************************************************/
void long_press_reboot_function_setting(void)
{
		if(kpd_enable_lprst && get_boot_mode() == NORMAL_BOOT) {
		kpd_print("Normal Boot long press reboot selection\n");
		upmu_set_rg_pwrkey_rst_en(0x00);//pmic package function for long press reboot function setting
		upmu_set_rg_homekey_rst_en(0x00);		
		
	#ifdef CONFIG_ONEKEY_REBOOT_NORMAL_MODE
		kpd_print("Enable ONE KEY normal mode LPRST\n");
		upmu_set_rg_pwrkey_rst_en(0x01);//pmic_config_interface(TOP_RST_MISC, 0x01, PMIC_RG_PWRKEY_RST_EN_MASK, PMIC_RG_PWRKEY_RST_EN_SHIFT);
		upmu_set_rg_pwrkey_rst_td(CONFIG_KPD_PMIC_LPRST_TD);
	#endif
	#ifdef CONFIG_TWOKEY_REBOOT_NORMAL_MODE
		kpd_print("Enable TWO KEY normal mode LPRST\n");
		upmu_set_rg_pwrkey_rst_en(0x01);//pmic package function for long press reboot function setting//pmic_config_interface(TOP_RST_MISC, 0x01, PMIC_RG_PWRKEY_RST_EN_MASK, PMIC_RG_PWRKEY_RST_EN_SHIFT);
		upmu_set_rg_fchr_keydet_en(0);//disable homekey pin FCHR mode of PMIC for 72 project
		upmu_set_rg_homekey_rst_en(0x01);//pmic_config_interface(TOP_RST_MISC, 0x01, PMIC_RG_HOMEKEY_RST_EN_MASK, PMIC_RG_HOMEKEY_RST_EN_SHIFT);
		upmu_set_rg_fchr_pu_en(0x01);//pull up homekey pin of PMIC for 72 project
		upmu_set_rg_pwrkey_rst_td(CONFIG_KPD_PMIC_LPRST_TD);
	#endif
	} 
	else {
		kpd_print("Other Boot Mode long press reboot selection\n");
		upmu_set_rg_pwrkey_rst_en(0x00);//pmic package function for long press reboot function setting
		upmu_set_rg_homekey_rst_en(0x00);		
	
	#ifdef CONFIG_ONEKEY_REBOOT_OTHER_MODE
		kpd_print("Enable ONE KEY other mode LPRST\n");
		upmu_set_rg_pwrkey_rst_en(0x01);//pmic_config_interface(TOP_RST_MISC, 0x01, PMIC_RG_PWRKEY_RST_EN_MASK, PMIC_RG_PWRKEY_RST_EN_SHIFT);
		upmu_set_rg_pwrkey_rst_td(CONFIG_KPD_PMIC_LPRST_TD);
	#endif	
	#ifdef CONFIG_TWOKEY_REBOOT_OTHER_MODE
		kpd_print("Enable TWO KEY other mode LPRST\n");
		upmu_set_rg_pwrkey_rst_en(0x01);//pmic package function for long press reboot function setting//pmic_config_interface(TOP_RST_MISC, 0x01, PMIC_RG_PWRKEY_RST_EN_MASK, PMIC_RG_PWRKEY_RST_EN_SHIFT);
		upmu_set_rg_fchr_keydet_en(0);//disable homekey pin FCHR mode of PMIC for 72 project
		upmu_set_rg_homekey_rst_en(0x01);//pmic_config_interface(TOP_RST_MISC, 0x01, PMIC_RG_HOMEKEY_RST_EN_MASK, PMIC_RG_HOMEKEY_RST_EN_SHIFT);
		upmu_set_rg_fchr_pu_en(0x01);//pull up homekey pin of PMIC for 72 project
		upmu_set_rg_pwrkey_rst_td(CONFIG_KPD_PMIC_LPRST_TD);
	#endif
	}
}
/********************************************************************/
void kpd_wakeup_src_setting(int enable)
{
//#ifndef EVB_PLATFORM
//	int err = 0;
//	if(enable == 1){
//		kpd_print("enable kpd as wake up source operation!\n");
//		err = slp_set_wakesrc(WAKE_SRC_KP|WAKE_SRC_CFG_KEY,true,false);
//		if(err != 0){
//			kpd_print("enable kpd as wake up source fail!\n");
//		}
//	}else{
//		kpd_print("disable kpd as wake up source operation!\n");
//		err = slp_set_wakesrc(WAKE_SRC_KP|WAKE_SRC_CFG_KEY,false,false);
//		if(err != 0){
//			kpd_print("disable kpd as wake up source fail!\n");
//		}
//	}
//#endif
}

/********************************************************************/
void kpd_init_keymap(u16 keymap[])
{
	int i = 0;
	for(i = 0;i < KPD_NUM_KEYS;i++){
	keymap[i] = kpd_keymap[i];
	}
}

void kpd_init_keymap_state(u16 keymap_state[])
{
	int i = 0;
	for(i = 0;i < KPD_NUM_MEMS;i++){
	keymap_state[i] = kpd_keymap_state[i];
	}
	printk(KPD_SAY "init_keymap_state done!\n");
}
/********************************************************************/
void kpd_get_keymap_state(u16 state[])
{
	state[0] = *(volatile u16 *)KP_MEM1;
	state[1] = *(volatile u16 *)KP_MEM2;
	state[2] = *(volatile u16 *)KP_MEM3;
	state[3] = *(volatile u16 *)KP_MEM4;
	state[4] = *(volatile u16 *)KP_MEM5;
	printk(KPD_SAY "register = %x %x %x %x %x\n",state[0], state[1], state[2], state[3], state[4]);
	
}

void kpd_set_debounce(u16 val)
{
	mt65xx_reg_sync_writew((u16)(val & KPD_DEBOUNCE_MASK), KP_DEBOUNCE);
}

/********************************************************************/
void kpd_pmic_rstkey_hal(unsigned long pressed){
#ifdef KPD_PMIC_RSTKEY_MAP
		input_report_key(kpd_input_dev, KPD_PMIC_RSTKEY_MAP, pressed);
		input_sync(kpd_input_dev);
		if (kpd_show_hw_keycode) {
			printk(KPD_SAY "(%s) HW keycode =%d using PMIC\n",
			       pressed ? "pressed" : "released", KPD_PMIC_RSTKEY_MAP);
		}
#endif
}

void kpd_pmic_pwrkey_hal(unsigned long pressed){
#if KPD_PWRKEY_USE_PMIC
		input_report_key(kpd_input_dev, KPD_PWRKEY_MAP, pressed);
		input_sync(kpd_input_dev);
		if (kpd_show_hw_keycode) {
			printk(KPD_SAY "(%s) HW keycode =%d using PMIC\n",
			       pressed ? "pressed" : "released", KPD_PWRKEY_MAP);
		}
#endif
}
/***********************************************************************/
void kpd_pwrkey_handler_hal(unsigned long data){
#if KPD_PWRKEY_USE_EINT
	bool pressed;
	u8 old_state = kpd_pwrkey_state;

	kpd_pwrkey_state = !kpd_pwrkey_state;
	pressed = (kpd_pwrkey_state == !!KPD_PWRKEY_POLARITY);
	if (kpd_show_hw_keycode) {
		printk(KPD_SAY "(%s) HW keycode = using EINT\n",
		       pressed ? "pressed" : "released");
	}
	kpd_backlight_handler(pressed, KPD_PWRKEY_MAP);
	input_report_key(kpd_input_dev, KPD_PWRKEY_MAP, pressed);
	input_sync(kpd_input_dev);
	kpd_print("report Linux keycode = %u\n", KPD_PWRKEY_MAP);

	/* for detecting the return to old_state */
	mt65xx_eint_set_polarity(KPD_PWRKEY_EINT, old_state);
	mt65xx_eint_unmask(KPD_PWRKEY_EINT);
#endif
}
/***********************************************************************/
void mt_eint_register(void){
#if KPD_PWRKEY_USE_EINT
	mt65xx_eint_set_sens(KPD_PWRKEY_EINT, KPD_PWRKEY_SENSITIVE);
	mt65xx_eint_set_hw_debounce(KPD_PWRKEY_EINT, KPD_PWRKEY_DEBOUNCE);
	mt65xx_eint_registration(KPD_PWRKEY_EINT, true, KPD_PWRKEY_POLARITY,
	                         kpd_pwrkey_eint_handler, false);
#endif
}
/************************************************************************/

