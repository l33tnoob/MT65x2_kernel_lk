#ifndef __MTK_CCCI_HELPER_H
#define __MTK_CCCI_HELPER_H



//-------------md sys configure------------------------//
#define MAX_MD_NUM        (1)
#define MD_SYS1           (0)
#define MD_SYS2           (1)
#define CURR_MD_ID		  (0)
#define MD1_EN			  (1<<0)
#define MD2_EN			  (1<<1)


//-------------other configure-------------------------//
#define MAX_KERN_API	  (20)
#define MAX_SLEEP_API	  (20)
#define MAX_FILTER_MEMBER (4)


//-------------error code define-----------------------//
#define E_NO_EXIST		  (-1)
#define E_PARAM			  (-2)


//-------------META MD setting define------------------//
#define MD1_SETTING_ACTIVE	(1<<0)
#define MD2_SETTING_ACTIVE	(1<<1)

#define MD_2G_FLAG		(1<<0)
#define MD_WG_FLAG		(1<<1)
#define MD_TG_FLAG		(1<<2)
#define MD_LTE_FLAG		(1<<3)


//-------------enum define---------------------------//
/*modem image version definitions*/
typedef enum{
	AP_IMG_INVALID = 0,
	AP_IMG_2G,
	AP_IMG_3G
}AP_IMG_TYPE;


typedef enum {
	ID_GET_MD_WAKEUP_SRC = 0,
	ID_CCCI_DORMANCY = 1,
    ID_LOCK_MD_SLEEP = 2,
	ID_ACK_MD_SLEEP = 3,
	ID_SSW_SWITCH_MODE = 4,
	ID_SET_MD_TX_LEVEL = 5,
	ID_GET_TXPOWER = 6,			// For thermal
	ID_IPO_H_RESTORE_CB = 7,
	ID_FORCE_MD_ASSERT = 8,
}KERN_FUNC_ID;

// System channel, AP -->(/ <-->) MD message start from 0x100
enum {
	MD_DORMANT_NOTIFY = 0x100,
	MD_SLP_REQUEST = 0x101,
	MD_TX_POWER = 0x102,
	MD_RF_TEMPERATURE = 0x103,
	MD_RF_TEMPERATURE_3G = 0x104,
	MD_GET_BATTERY_INFO = 0x105,
	MD_SIM_TYPE = 0x107,	//for regional phone boot animation
};

typedef enum {
	 RSM_ID_RESUME_WDT_IRQ = 0,
	 RSM_ID_MD_LOCK_DORMANT = 1,
	 RSM_ID_WAKE_UP_MD = 2,
	 RSM_ID_MAX
}RESUME_ID;
 
typedef enum {
	 SLP_ID_MD_FAST_DROMANT = 0,
	 SLP_ID_MD_UNLOCK_DORMANT = 1,
	 SLP_ID_MAX
}SLEEP_ID;

typedef enum {
	ID_GET_FDD_THERMAL_DATA = 0,
	ID_GET_TDD_THERMAL_DATA,
}SYS_CB_ID;


typedef enum {
	md_type_invalid = 0,
	modem_2g = 1,
	modem_3g,
	modem_wg,
	modem_tg,
}modem_type_t;

//-------------structure define------------------------//
typedef int (*ccci_kern_cb_func_t)(int, char *, unsigned int);
typedef struct{
	KERN_FUNC_ID		id;
	ccci_kern_cb_func_t	func;
}ccci_kern_func_info;


typedef int (*ccci_sys_cb_func_t)(int, int);
typedef struct{
	SYS_CB_ID			id;
	ccci_sys_cb_func_t	func;
}ccci_sys_cb_func_info_t;


typedef size_t (*ccci_filter_cb_func_t)(char *, size_t);
typedef struct _cmd_op_map{
	char cmd[8];
	int  cmd_len;
	ccci_filter_cb_func_t store;
	ccci_filter_cb_func_t show;
}cmd_op_map_t;



//-----------------export function declaration----------------------------//
AP_IMG_TYPE get_ap_img_ver(void);
int get_td_eint_info(int md_id, char * eint_name, unsigned int len);
int get_md_gpio_info(int md_id, char *gpio_name, unsigned int len);
int get_md_gpio_val(int md_id, unsigned int num);
int get_md_adc_info(int md_id, char *adc_name, unsigned int len);
int get_md_adc_val(int md_id, unsigned int num);
int get_dram_type_clk(int *clk, int *type);
int get_eint_attr(char *name, unsigned int name_len, unsigned int type, char * result, unsigned int *len);
int get_bat_info(unsigned int para);


unsigned int get_nr_modem(void);
unsigned int *get_modem_size_list(void);
int parse_ccci_dfo_setting(void *dfo_data, int num);
int parse_meta_md_setting(unsigned char args[]);

unsigned int get_md_mem_start_addr(int md_id);
unsigned int get_md_share_mem_start_addr(int md_id);
unsigned int get_smem_base_addr(int md_id);
unsigned int get_modem_is_enabled(int md_id);
unsigned int get_resv_mem_size_for_md(int md_id);
unsigned int get_resv_share_mem_size_for_md(int md_id);
void get_md_post_fix(int md_id, char buf[], char buf_ex[]);
unsigned int get_modem_support(int md_id);
unsigned int set_modem_support(int md_id, int md_type);


int register_filter_func(char cmd[], ccci_filter_cb_func_t store, ccci_filter_cb_func_t show);

int register_ccci_kern_func(unsigned int id, ccci_kern_cb_func_t func);
int register_ccci_kern_func_by_md_id(int md_id, unsigned int id, ccci_kern_cb_func_t func);
int exec_ccci_kern_func(unsigned int id, char *buf, unsigned int len);
int exec_ccci_kern_func_by_md_id(int md_id, unsigned int id, char *buf, unsigned int len);

void register_suspend_notify(int md_id, unsigned int id, void (*func)(int));
void register_resume_notify(int md_id, unsigned int id, void (*func)(int));

int register_sys_msg_notify_func(int md_id, int (*func)(int, unsigned int, unsigned int));
int notify_md_by_sys_msg(int md_id, unsigned int msg, unsigned int data);

int register_ccci_sys_call_back(int md_id, unsigned int id, ccci_sys_cb_func_t func);
void exec_ccci_sys_call_back(int md_id, int cb_id, int data);

void ccci_helper_exit(void);
void ccci_md_mem_reserve(void);
#endif
