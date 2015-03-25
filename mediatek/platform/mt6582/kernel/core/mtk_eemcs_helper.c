#include <linux/module.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/uaccess.h>
#include <linux/mm.h>
#include <linux/kfifo.h>
#include <linux/firmware.h>
#include <linux/syscalls.h>
#include <linux/uaccess.h>
#include <linux/platform_device.h>
#include <linux/proc_fs.h>
#include <asm/setup.h>

#include <mach/mtk_eemcs_helper.h>
#include <mach/mt_boot.h>
#include <mach/dfo_boot.h>
#include <asm/memblock.h>
#include <mach/battery_common.h>

#define SHOW_WARNING_NUM (5)

#define DFO_FEATURE_EN
#define MD5_MEM_SIZE    (16*1024*1024)

//-------------feature enable/disable configure (oringinal from mtk_ccci_helper.c)----------------//
#define FEATURE_GET_MD_BAT_VOL		//disable for bring up
//-------------grobal variable define----------------//

unsigned int ext_md_support[MAX_EXT_MD_NUM];
unsigned int ext_md_usage_case = 0;
unsigned int ext_md_size_list[MAX_EXT_MD_NUM] = {0};
phys_addr_t  ext_md_mem_addr[MAX_EXT_MD_NUM] = {0};

eemcs_kern_func_info eemcs_func_table[MAX_EXT_MD_NUM][MAX_KERN_API];
static unsigned char eemcs_kern_func_err_num[MAX_EXT_MD_NUM][MAX_KERN_API];
eemcs_sys_cb_func_info_t eemcs_sys_cb_table_1000[MAX_EXT_MD_NUM][MAX_KERN_API];
eemcs_sys_cb_func_info_t eemcs_sys_cb_table_100[MAX_EXT_MD_NUM][MAX_KERN_API];
int (*eemcs_sys_msg_notify_func[MAX_EXT_MD_NUM])(int, unsigned int, unsigned int);

typedef struct _dfo_item
{
	char name[32];
	int  value;
}dfo_item_t;

static dfo_item_t eemcs_dfo_setting[] =
{
    {"MTK_MD5_SUPPORT",	modem_lwg},
    {"MTK_ENABLE_MD5",	1},
    {"MD5_SIZE",		MD5_MEM_SIZE},
};


/*------------------------external function API------------------------*/
extern unsigned long *get_ext_modem_start_addr_list(void);
/*---------------------------------------------------------------------*/


static int get_eemcs_dfo_setting(char item[], unsigned int *val)
{
	char *eemcs_name;
	int  eemcs_value;
	int  i;

	for (i=0; i<(sizeof(eemcs_dfo_setting)/sizeof(dfo_item_t)); i++) {
		eemcs_name = eemcs_dfo_setting[i].name;
		eemcs_value = eemcs_dfo_setting[i].value;
		if(!strcmp(eemcs_name, item)) {
			printk("[EEMCS/PLAT] Get DFO:%s:0x%08X\n", eemcs_name, eemcs_value);
			*val = (unsigned int)eemcs_value;
			return 0;
		}
	}
	printk("[EEMCS/PLAT] DFO:%s not found\n", i+1, item);
	return -1;
}

void update_ext_md_support(void)
{
    int val;
    ext_md_usage_case = 0;

    if(get_eemcs_dfo_setting("MTK_MD5_SUPPORT", &val) == 0) {
        ext_md_support[MD_SYS5-MD_EXT1] = val;
    }

    if(get_eemcs_dfo_setting("MTK_ENABLE_MD5", &val) == 0) {
    	if(val > 0) {
    		ext_md_usage_case |= MD5_EN;
    	}
    }

    if(get_eemcs_dfo_setting("MD5_SIZE", &val) == 0) {
        val = round_up(val, 0x200000);
        ext_md_size_list[MD_SYS5-MD_EXT1] = val;
    }
  
}

/*API for kernal memory setting*/
/*get the info about how many modem is running currently*/
unsigned int get_nr_ext_modem(void)
{
    return MAX_EXT_MD_NUM;
}
EXPORT_SYMBOL(get_nr_ext_modem);


unsigned int *get_ext_modem_size_list(void)
{
    return ext_md_size_list;
}
EXPORT_SYMBOL(get_ext_modem_size_list);

#ifdef DFO_FEATURE_EN // pase DFO ATAG info
int parse_eemcs_dfo_setting(void *dfo_tbl, int num)
{
	char *eemcs_name;
	int  *eemcs_value;
	char *tag_name;
	int  tag_value;
	int i, j;

	tag_dfo_boot *dfo_data;

	if(dfo_tbl == NULL)
		return -1;

	dfo_data = (tag_dfo_boot *)dfo_tbl;
	for (i=0; i<(sizeof(eemcs_dfo_setting)/sizeof(dfo_item_t)); i++) {
		eemcs_name = eemcs_dfo_setting[i].name;
		eemcs_value = &(eemcs_dfo_setting[i].value);
		for (j=0; j<num; j++) {
			tag_name = dfo_data->name[j];
			tag_value = dfo_data->value[j];
			if(!strcmp(eemcs_name, tag_name)) {
				*eemcs_value = tag_value;
			}
		}
		printk("[EEMCS/PLAT] DFO:%s:0x%08X\n", eemcs_name, *eemcs_value);
	}

  update_ext_md_support();
  return 0;
}
#else // load default seeting
int parse_eemcs_dfo_setting(void *dfo_data, int num)
{
	char *eemcs_name;
	int   eemcs_value;
	int i;

	for (i=0; i<(sizeof(eemcs_dfo_setting)/sizeof(dfo_item_t)); i++) {
		eemcs_name = eemcs_dfo_setting[i].name;
		eemcs_value = eemcs_dfo_setting[i].value;
		printk("[EEMCS/PLAT] DFO:%s:0x%08X\n", eemcs_name, eemcs_value);
	}

  update_ext_md_support();
  return 0;
}
#endif


void eemcs_memory_reserve(void){
    unsigned int	md5_en;
    
    if( (ext_md_usage_case&MD5_EN)== MD5_EN) { //Only MD1 enabled
		md5_en = 1;
		ext_md_mem_addr[MD_SYS5-MD_EXT1] = 
            (unsigned int)arm_memblock_steal(ext_md_size_list[MD_SYS5-MD_EXT1], SZ_32M);
	} else { // No MD is enabled
		md5_en = 0;
		ext_md_mem_addr[MD_SYS5-MD_EXT1] = 0;
	}

	if ( (ext_md_mem_addr[MD_SYS5-MD_EXT1]&(32*1024*1024 - 1)) != 0 )
		printk("[EEMCS/PLAT] md5 memory addr is not 32M align!!!\n");

	printk("[EEMCS/PLAT] EN(%d):MemBase(0x%08X)\n", md5_en, ext_md_mem_addr);

	printk("[EEMCS/PLAT] (0)MemStart(0x%08X):MemSize(0x%08X)\n", \
		ext_md_mem_addr[MD_SYS5-MD_EXT1], ext_md_size_list[MD_SYS5-MD_EXT1]);

}

int parse_ext_meta_md_setting(unsigned char args[])
{
	unsigned char md_active_setting = args[1];
	unsigned char md_setting_flag = args[0];
  int  active_id =  -1;
	int  active_index = -1;

	if(!(md_active_setting & MD5_SETTING_ACTIVE)) {
		printk("[EEMCS/PLAT] META EXT MD setting not found [%d][%d]\n", args[0], args[1]);
  } else {
    active_id = MD_SYS5;
    active_index = MD_SYS5 - MD_EXT1;
  }

  switch(active_id) {
    case MD_SYS5:
    	if(md_setting_flag == MD_LWG_FLAG){
    		ext_md_support[active_index] = modem_lwg;
    	} else if(md_setting_flag == MD_LTG_FLAG){
    		ext_md_support[active_index] = modem_ltg;
    	}
	    printk("[EEMCS/PLAT] META EXT MD type:%d\n", ext_md_support[active_index]);
      break;
  }
	return 0;	
}

void get_ext_md_post_fix(int md_id, char buf[], char buf_ex[])
{
	// name format: modem_X_YYY_Z_Ex.img
	int		X, Ex;
	char		YYY[8];
#if defined(DFO_FEATURE_EN)
	unsigned int	feature_val = 0;
#endif

  if (md_id < MD_SYS5) {
    printk("[EEMCS/PLAT] [Error]get_ext_md_post_fix: invalid md_id=%d\n", md_id);
    return;
  }

	// X
	X = md_id + 1;

#if defined(DFO_FEATURE_EN)
	// DFO start -------------------
	// YYY
	YYY[0] = '\0';
	switch(md_id) {
  	case MD_SYS5:
  		feature_val = ext_md_support[MD_SYS5-MD_EXT1];
  		break;
  	default:
      printk("[EEMCS/PLAT] [Error]get_ext_md_post_fix: invalid md_id=%d\n", md_id);
  		break;
	}
	switch(feature_val) {
  	case modem_lwg:
  		snprintf(YYY, 8, "_lwg_n");
  		break;
  	case modem_ltg:
  		snprintf(YYY, 8, "_ltg_n");
  		break;
  	default:
  		printk("[EEMCS/PLAT] [Error]get_ext_md_post_fix: invalid feature=%d\n", feature_val);
  		break;
	}
	// DFO end ---------------------
#else
	// Static start -------------------
	// YYY
	snprintf(YYY, 8, "_lwg_n");
	// Static end ---------------------
#endif

	// [_Ex] Get chip version
#if 0
	if(get_chip_version() == CHIP_SW_VER_01)
		Ex = 1;
	else if(get_chip_version() == CHIP_SW_VER_02)
		Ex = 2;
#else
	Ex = 1;
#endif

	// Gen post fix
	if(buf) {
		snprintf(buf, 12, "%d%s", X, YYY);
    	printk("[EEMCS/PLAT] MD%d image postfix=%s\n", md_id, buf);
	}

	if(buf_ex) {
		snprintf(buf_ex, 12, "%d%s_E%d", X, YYY, Ex);
    	printk("[EEMCS/PLAT] MD%d image postfix=%s\n", md_id, buf_ex);
	}
}
EXPORT_SYMBOL(get_ext_md_post_fix);

unsigned int get_ext_modem_is_enabled(int md_id)
{
	switch(md_id)
	{
	case MD_SYS5:
		return !!(ext_md_usage_case & MD5_EN);

	default:
		return 0;
	}
}
EXPORT_SYMBOL(get_ext_modem_is_enabled);


unsigned int get_ext_modem_support(int md_id){
    #if defined(DFO_FEATURE_EN)
	unsigned int	feature_val = 0;

    switch(md_id) {
  	case MD_SYS5:
  		feature_val = ext_md_support[MD_SYS5-MD_EXT1];
  		break;
  	default:
        printk("[EEMCS/PLAT] [Error]get_ext_modem_support: invalid md_id=%d\n", md_id);
  		break;
	}
    
    return feature_val;

    #else

    return modem_lwg;
    
    #endif

    
}

EXPORT_SYMBOL(get_ext_modem_support);

unsigned int set_ext_modem_support(int md_id, int md_type){
#if defined(DFO_FEATURE_EN)
    printk("[EEMCS/PLAT] set_ext_modem_support MD %d Type %d", md_id, md_type);
    switch(md_id) {
  	case MD_SYS5:
        if (md_type >= modem_lwg &&  md_type <= modem_ltg){
  		    ext_md_support[MD_SYS5-MD_EXT1] = md_type;
        }
        else{
            printk("[EEMCS/PLAT] [Error]set_modem_support fail(md:%d, md_type:%d)\n", md_id, md_type);
            return -1;
        }
  		break;
  	default:
        printk("[EEMCS/PLAT] [Error]set_modem_support: invalid md_id=%d\n", md_id);
        return -1;
  		break;
	}
    
    return 0;

#else
    
    printk("[EEMCS/PLAT] set_ext_modem_support: DFO not support");
    return -1;
    
#endif

    
}

EXPORT_SYMBOL(set_ext_modem_support);

void get_ap_platform_ver(char * ver)
{
	sprintf(ver, "MT%04x_S%02x", get_chip_code(), (get_chip_sw_ver_code()&0xFF));
}

EXPORT_SYMBOL(get_ap_platform_ver);


unsigned int get_ext_md_mem_start_addr(int md_id){  
    unsigned int	feature_val = 0;

    switch(md_id) {
  	case MD_SYS5:
  		feature_val = ext_md_mem_addr[MD_SYS5-MD_EXT1];
  		break;
  	default:
        printk("[EEMCS/PLAT] [Error]get_ext_md_mem_start_addr: invalid md_id=%d\n", md_id);
  		break;
	}
    
    return feature_val;
}
EXPORT_SYMBOL(get_ext_md_mem_start_addr);

unsigned int get_ext_md_mem_size(int md_id){
    unsigned int	feature_val = 0;

    switch(md_id) {
  	case MD_SYS5:
  		feature_val = ext_md_size_list[MD_SYS5-MD_EXT1];
  		break;
  	default:
        printk("[EEMCS/PLAT] [Error]get_ext_md_mem_size: invalid md_id=%d\n", md_id);
  		break;
	}
    
    return feature_val;
    
}
EXPORT_SYMBOL(get_ext_md_mem_size);


int eemcs_get_bat_info(unsigned int para)
{
    int val = 0;
    #if defined (FEATURE_GET_MD_BAT_VOL)
    val = (int)BAT_Get_Battery_Voltage(0);
    printk("[EEMCS/PLAT] get_bat_info : %d \n", val);
	return val;
	#endif
    
}
EXPORT_SYMBOL(eemcs_get_bat_info);


/*********************************************************************************/
/*  API about Security cipher MD image                                           */
/*                                                                               */
/*********************************************************************************/
#ifdef ENABLE_SECURITY_FEATURE
#include <mach/mt_sec_export.h>
#include <mach/emi_mpu.h>
EXPORT_SYMBOL(masp_boot_init);
EXPORT_SYMBOL(masp_ccci_version_info);
EXPORT_SYMBOL(masp_ccci_is_cipherfmt);
EXPORT_SYMBOL(masp_ccci_signfmt_verify_file);
EXPORT_SYMBOL(masp_ccci_decrypt_cipherfmt);
EXPORT_SYMBOL(masp_secro_md_get_data);
EXPORT_SYMBOL(masp_secro_en);
EXPORT_SYMBOL(masp_secro_md_len);
EXPORT_SYMBOL(masp_secro_blk_sz);
EXPORT_SYMBOL(masp_secure_algo_init);
EXPORT_SYMBOL(masp_secure_algo_deinit);
EXPORT_SYMBOL(masp_secure_algo);
/*********************************************************************************/
/*  API about md ROM/RW/Share memory MPU protection                              */
/*                                                                               */
/*********************************************************************************/
int clear_md_region_protection(int md_id)
{
	unsigned int rom_mem_mpu_id, rw_mem_mpu_id;

	printk("[EEMCS/PLAT] Clear MD%d region protect...\n", md_id+1);
	switch(md_id)
	{
		case MD_SYS5:
			rom_mem_mpu_id = 1; //0;
			rw_mem_mpu_id = 2;  //1;
			break;
			
		default:
			printk("[EEMCS/PLAT] [Error]clear_md_region_protection: invalid md_id=%d\n", md_id+1);
			return -1;
	}
	
	printk("[EEMCS/PLAT] Clear MPU protect MD%d ROM region<%d>\n", md_id+1, rom_mem_mpu_id);
	emi_mpu_set_region_protection(0,	  				/*START_ADDR*/
								  0,      				/*END_ADDR*/
								  rom_mem_mpu_id,       /*region*/
								  SET_ACCESS_PERMISSON(NO_PROTECTION, NO_PROTECTION, NO_PROTECTION, NO_PROTECTION));

	printk("[EEMCS/PLAT] Clear MPU protect MD%d R/W region<%d>\n", md_id+1, rw_mem_mpu_id);
	emi_mpu_set_region_protection(0,		  			/*START_ADDR*/
								  0,       				/*END_ADDR*/
								  rw_mem_mpu_id,        /*region*/
								  SET_ACCESS_PERMISSON(NO_PROTECTION, NO_PROTECTION, NO_PROTECTION, NO_PROTECTION));
	
	return 0;
}
EXPORT_SYMBOL(clear_md_region_protection);
#endif

/***************************************************************************/
/* Register ccci call back function when AP receive system channel message */
/*                                                                         */
/***************************************************************************/
int eemcs_register_sys_msg_notify_func(int md_id, int (*func)(int, unsigned int, unsigned int))
{
	int ret = 0;
	int ex_md_id = md_id - MD_EXT1;
	
	if( ex_md_id >= MAX_EXT_MD_NUM ) {
		printk("[EEMCS/PLAT] [Error]register_sys_msg_notify_func: invalid md id(%d)\n", md_id+1);
		return E_PARAM;
	}

	if(eemcs_sys_msg_notify_func[ex_md_id] == NULL) {
		eemcs_sys_msg_notify_func[ex_md_id] = func;
	} else {
		printk("[EEMCS/PLAT] [Error]eemcs_sys_msg_notify_func: func registered!\n", md_id+1);
	}

	return ret;
}
EXPORT_SYMBOL(eemcs_register_sys_msg_notify_func);


int eemcs_notify_md_by_sys_msg(int md_id, unsigned int msg, unsigned int data)
{
	int (*func)(int, unsigned int, unsigned int);
	int ret = 0;
	int ext_md_id = md_id - MD_EXT1;
	
	if(ext_md_id >= MAX_EXT_MD_NUM) {
		printk("[EEMCS/PLAT] [Error]notify_md_by_sys_msg: invalid md id(%d)\n", md_id+1);
		return E_PARAM;
	}

	func = eemcs_sys_msg_notify_func[ext_md_id];
	if(func != NULL) {
		ret = func(md_id, msg, data);
	} else {
		ret = E_NO_EXIST;
		printk("[EEMCS/PLAT] [Error]notify_md_by_sys_msg: func not register!\n", md_id+1);
	}

	return ret;
}
EXPORT_SYMBOL(eemcs_notify_md_by_sys_msg);


int eemcs_register_ccci_sys_call_back(int md_id, unsigned int id, eemcs_sys_cb_func_t func)
{
	int ret = 0;
	eemcs_sys_cb_func_info_t *info_ptr;
	int ext_md_id = md_id - MD_EXT1;
	
	if( ext_md_id >= MAX_EXT_MD_NUM ) {
		printk("[EEMCS/PLAT] [Error]register_sys_call_back: invalid md id(%d)\n", md_id+1);
		return E_PARAM;
	}

	if((id >= 0x100)&&((id-0x100) < MAX_KERN_API)) {
		info_ptr = &(eemcs_sys_cb_table_100[ext_md_id][id-0x100]);
	} else if((id >= 0x1000)&&((id-0x1000) < MAX_KERN_API)) {
		info_ptr = &(eemcs_sys_cb_table_1000[ext_md_id][id-0x1000]);
	} else {
		printk("[EEMCS/PLAT] [Error]register_sys_call_back: invalid func id(0x%x)\n", id);
		return E_PARAM;
	}
	
	if(info_ptr->func == NULL) {
		info_ptr->id = id;
		info_ptr->func = func;
	}
	else
		printk("[EEMCS/PLAT] [Error]register_sys_call_back: func(0x%x) registered!\n", id);

	return ret;
}
EXPORT_SYMBOL(eemcs_register_ccci_sys_call_back);


void eemcs_exec_ccci_sys_call_back(int md_id, int cb_id, int data)
{
	eemcs_sys_cb_func_t func;
	int	id;
	eemcs_sys_cb_func_info_t	*curr_table;
	int ext_md_id = md_id - MD_EXT1;
	
	if(ext_md_id >= MAX_EXT_MD_NUM) {
		printk("[EEMCS/PLAT] [Error]exec_sys_cb: invalid md id(%d) \n", md_id+1);
		return;
	}

	id = cb_id & 0xFF;
	if(id >= MAX_KERN_API) {
		printk("[EEMCS/PLAT] [Error]exec_sys_cb: invalid func id(0x%x)\n", cb_id);
		return;
	}

	if ((cb_id & (0x1000|0x100))==0x1000) {
		curr_table = eemcs_sys_cb_table_1000[ext_md_id];
	} else if ((cb_id & (0x1000|0x100))==0x100) {
		curr_table = eemcs_sys_cb_table_100[ext_md_id];
	} else {
		printk("[EEMCS/PLAT] [Error]exec_sys_cb: invalid func id(0x%x)\n", cb_id);
		return;
	}
	
	func = curr_table[id].func;
	if(func != NULL) {
		func(md_id, data);
	} else {
		printk("[EEMCS/PLAT] [Error]exec_sys_cb: func id(0x%x) not register!\n", cb_id);
	}
}
EXPORT_SYMBOL(eemcs_exec_ccci_sys_call_back);


/////////////////////////////////////////////////////////////////////////////////////////////

/***************************************************************************/
/* Register kernel API for ccci driver invoking                            */
/*                                                                         */
/***************************************************************************/
int eemcs_register_ccci_kern_func_by_md_id(int md_id, unsigned int id, eemcs_kern_cb_func_t func)
{
	int ret = 0;
	eemcs_kern_func_info *info_ptr;
	
	if((id >= MAX_KERN_API) || (func == NULL) || (md_id >= MAX_EXT_MD_NUM)) {
		printk("[EEMCS/PLAT] [Error]register_kern_func: md_id:%d, func_id:%d!\n", md_id+1, id);
		return E_PARAM;
	}

	info_ptr = &(eemcs_func_table[md_id][id]);
	if(info_ptr->func == NULL) {
		info_ptr->id = id;
		info_ptr->func = func;
	}
	else
		printk("[EEMCS/PLAT] [Error]register_kern_func: func(%d) registered!\n", md_id+1, id);

	return ret;
}
EXPORT_SYMBOL(eemcs_register_ccci_kern_func_by_md_id);


int eemcs_register_ccci_kern_func(unsigned int id, eemcs_kern_cb_func_t func)
{
	return eemcs_register_ccci_kern_func_by_md_id(CURR_EXT_MD_ID, id, func);
}
EXPORT_SYMBOL(eemcs_register_ccci_kern_func);


int eemcs_exec_ccci_kern_func_by_md_id(int md_id, unsigned int id, char *buf, unsigned int len)
{
	eemcs_kern_cb_func_t func;
	int ret = 0;
	
	if(md_id >= MAX_EXT_MD_NUM) {
		printk("[EEMCS/PLAT] [Error]exec kern func: invalid md id(%d)\n", md_id+1);
		return E_PARAM;
	}
	
	if(id >= MAX_KERN_API) {
		printk("[EEMCS/PLAT] [Error]exec kern func: invalid func id(%d)\n", md_id, id);
		return E_PARAM;
	}
	
	func = eemcs_func_table[md_id][id].func;
	if(func != NULL) {
		ret = func(md_id, buf, len);
	}
	else {
		ret = E_NO_EXIST;
		if(eemcs_kern_func_err_num[md_id][id] < SHOW_WARNING_NUM) {
			eemcs_kern_func_err_num[md_id][id]++;
			printk("[EEMCS/PLAT] [Error]exec kern func: func%d not register\n", md_id+1, id);
		}
	}

	return ret;
}
EXPORT_SYMBOL(eemcs_exec_ccci_kern_func_by_md_id);


int eemcs_exec_ccci_kern_func(unsigned int id, char *buf, unsigned int len)
{
	return eemcs_exec_ccci_kern_func_by_md_id(CURR_EXT_MD_ID, id, buf, len);
}
EXPORT_SYMBOL(eemcs_exec_ccci_kern_func);


/***************************************************************************/
/* Register ccci suspend & resume function                                 */
/*                                                                         */
/***************************************************************************/
typedef struct eemcs_pm_cb_item
{
	void (*cb_func)(int);
	int		md_id;
}eemcs_pm_cb_item_t;
	
	
static eemcs_pm_cb_item_t eemcs_suspend_cb_table[MAX_EXT_MD_NUM][MAX_SLEEP_API];
static eemcs_pm_cb_item_t eemcs_resume_cb_table[MAX_EXT_MD_NUM][MAX_SLEEP_API];


void eemcs_register_suspend_notify(int md_id, unsigned int id, void (*func)(int))
{
	if((id >= MAX_SLEEP_API) || (func == NULL) || (md_id >= MAX_EXT_MD_NUM)) {
		printk("[EEMCS/PLAT] [Error]register_suspend_notify: invalid para(md:%d, cmd:%d)\n", md_id, id);
	}
	
	if (eemcs_suspend_cb_table[md_id][id].cb_func == NULL){
		eemcs_suspend_cb_table[md_id][id].cb_func = func;
		eemcs_suspend_cb_table[md_id][id].md_id = md_id;
	}
}
EXPORT_SYMBOL(eemcs_register_suspend_notify);

	
void eemcs_register_resume_notify(int md_id, unsigned int id, void (*func)(int))
{
	if((id >= MAX_SLEEP_API) || (func == NULL) || (md_id >= MAX_EXT_MD_NUM)) {
		printk("[EEMCS/PLAT] [Error]register_resume_notify: invalid para(md:%d, cmd:%d)\n", md_id, id);
	}
	
	if (eemcs_resume_cb_table[md_id][id].cb_func == NULL){
		eemcs_resume_cb_table[md_id][id].cb_func = func;
		eemcs_resume_cb_table[md_id][id].md_id = md_id;
	}
}
EXPORT_SYMBOL(eemcs_register_resume_notify);


static int eemcs_helper_probe(struct platform_device *dev)
{
	
	//printk( "\neemcs_helper_probe\n" );
	return 0;
}

static int eemcs_helper_remove(struct platform_device *dev)
{
	//printk( "\neemcs_helper_remove\n" );
	return 0;
}

static void eemcs_helper_shutdown(struct platform_device *dev)
{
	//printk( "\neemcs_helper_shutdown\n" );
}

static int eemcs_helper_suspend(struct platform_device *dev, pm_message_t state)
{
	int		i, j;
	void	(*func)(int);
	int		md_id;

	printk( "\neemcs_helper_suspend\n" );

	for (i = 0; i < MAX_EXT_MD_NUM; i++) {
		for (j = 0; j < EEMCS_SLP_ID_MAX; j++) {
			func = eemcs_suspend_cb_table[i][j].cb_func;
			md_id = eemcs_suspend_cb_table[i][j].md_id;
			if(func != NULL)
				func(md_id);
		}
	}
	
	return 0;
}

static int eemcs_helper_resume(struct platform_device *dev)
{
	int		i,j;
	void	(*func)(int);
	int		md_id;

	printk( "\neemcs_helper_resume\n" );

	for (i = 0; i < MAX_EXT_MD_NUM; i++) {
		for (j = 0; j < EEMCS_RSM_ID_MAX; j++) {
			func = eemcs_resume_cb_table[i][j].cb_func;
			md_id = eemcs_resume_cb_table[i][j].md_id;
			if(func != NULL)
				func(md_id);
		}
	}
	
	return 0;
}

//*-------------------------------------------------------------*//
#if defined (CONFIG_PM) && defined (FEATURE_PM_IPO_H)
int eemcs_helper_pm_suspend(struct device *device)
{
    //pr_debug("calling %s()\n", __func__);

    struct platform_device *pdev = to_platform_device(device);
    BUG_ON(pdev == NULL);

    return eemcs_helper_suspend(pdev, PMSG_SUSPEND);
}

int eemcs_helper_pm_resume(struct device *device)
{
    //pr_debug("calling %s()\n", __func__);

    struct platform_device *pdev = to_platform_device(device);
    BUG_ON(pdev == NULL);

    return eemcs_helper_resume(pdev);
}

extern void mt_irq_set_sens(unsigned int irq, unsigned int sens);
extern void mt_irq_set_polarity(unsigned int irq, unsigned int polarity);
int eemcs_helper_pm_restore_noirq(struct device *device)
{
    pr_debug("calling %s()\n", __func__);
/*Not ready, need to check more details*/
#if 0
    // CCIF AP0
    mt_irq_set_sens(CCIF0_AP_IRQ_ID, MT_LEVEL_SENSITIVE);
    mt_irq_set_polarity(CCIF0_AP_IRQ_ID, MT_POLARITY_LOW);

    // MD1 WDT
    mt_irq_set_sens(MD_WDT_IRQ_ID, MT_EDGE_SENSITIVE);
    mt_irq_set_polarity(MD_WDT_IRQ_ID, MT_POLARITY_LOW);

    // MD1
    exec_ccci_kern_func_by_md_id(0, ID_IPO_H_RESTORE_CB, NULL, 0);
#endif
    return 0;

}

#else /*CONFIG_PM*/

#define eemcs_helper_pm_suspend NULL
#define eemcs_helper_pm_resume  NULL
#define eemcs_helper_pm_restore_noirq NULL

#endif /*CONFIG_PM*/
//*-------------------------------------------------------------*//

struct dev_pm_ops eemcs_helper_pm_ops = {
    .suspend = eemcs_helper_pm_suspend,
    .resume = eemcs_helper_pm_resume,
    .freeze = eemcs_helper_pm_suspend,
    .thaw = eemcs_helper_pm_resume,
    .poweroff = eemcs_helper_pm_suspend,
    .restore = eemcs_helper_pm_resume,
    .restore_noirq = eemcs_helper_pm_restore_noirq,
};

static struct platform_driver eemcs_helper_driver =
{
	.driver     = {
		.name	= "eemcs-helper",
#ifdef CONFIG_PM
        .pm     = &eemcs_helper_pm_ops,
#endif
	},
	.probe		= eemcs_helper_probe,
	.remove		= eemcs_helper_remove,
	.shutdown	= eemcs_helper_shutdown,
	.suspend	= eemcs_helper_suspend,
	.resume		= eemcs_helper_resume,
};

struct platform_device eemcs_helper_device = {
	.name		= "eemcs-helper",
	.id		= 0,
	.dev		= {}
};

static int __init eemcs_helper_init(void)
{
	int ret;


/* Not ready, need check*/
#if 0
	// Init ccci helper sys fs
	memset( (void*)cmd_map_table, 0, sizeof(cmd_map_table) );
	mtk_ccci_sysfs();
#endif
	// init ccci kernel API register table
	memset((void*)eemcs_func_table, 0, sizeof(eemcs_func_table));
	memset((void*)eemcs_kern_func_err_num, 0, sizeof(eemcs_kern_func_err_num));

	// init ccci system channel call back function register table
	memset((void*)eemcs_sys_cb_table_100, 0, sizeof(eemcs_sys_cb_table_100));
	memset((void*)eemcs_sys_cb_table_1000, 0, sizeof(eemcs_sys_cb_table_1000));

	ret = platform_device_register(&eemcs_helper_device);
	if (ret) {
		printk("[EEMCS/PLAT] [Error]eemcs_helper_device register fail: %d\n", ret);
		return ret;
	}

	ret = platform_driver_register(&eemcs_helper_driver);
	if (ret) {
		printk("[EEMCS/PLAT] [Error]eemcs_helper_driver register fail: %d\n", ret);
		return ret;
	}
	
	return 0;
}

module_init(eemcs_helper_init);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("MTK");
MODULE_DESCRIPTION("The eemcs helper function");
/////////////////////////////////////////////////////////////////////////////////////////////


