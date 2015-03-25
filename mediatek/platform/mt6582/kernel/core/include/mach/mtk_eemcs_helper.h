#ifndef __MTK_EEMCS_HELPER_H
#define __MTK_EEMCS_HELPER_H

#define MD_SYS5 (4) // MD SYS counts from 1, but internal index counts from 0
#define MD5_EN (1<<4)
#define MD_EXT1 MD_SYS5 // the first external MD's ID
#define MAX_EXT_MD_NUM (1) // external MD uses ID 5, 6, 7, 8

#define CURR_EXT_MD_ID		  (0)

/* remove to avoid redeclaration
#define MD1_SETTING_ACTIVE	(1<<0)
#define MD2_SETTING_ACTIVE	(1<<1)
*/
#define MD5_SETTING_ACTIVE	(1<<4)

/* remove to avoid redeclaration
#define MD_2G_FLAG		(1<<0)
#define MD_WG_FLAG		(1<<1)
#define MD_TG_FLAG		(1<<2)
#define MD_LTE_FLAG		(1<<3)
*/
#define MD_LWG_FLAG		((1<<3)|(1<<1)|(1<<0))
#define MD_LTG_FLAG		((1<<3)|(1<<2)|(1<<0))


//-------------other configure-------------------------//
#define MAX_KERN_API	  (20)
#define MAX_SLEEP_API	  (20)

//-------------error code define-----------------------//
#define E_NO_EXIST		  (-1)
#define E_PARAM			  (-2)

typedef enum {
    EEMCS_ID_GET_MD_WAKEUP_SRC = 0,
    EEMCS_ID_CCCI_DORMANCY = 1,
    EEMCS_ID_LOCK_MD_SLEEP = 2,
    EEMCS_ID_ACK_MD_SLEEP = 3,
    EEMCS_ID_SSW_SWITCH_MODE = 4,
    EEMCS_ID_SET_MD_TX_LEVEL = 5,
    EEMCS_ID_GET_TXPOWER = 6,			// For thermal
    EEMCS_ID_IPO_H_RESTORE_CB = 7,
}EEMCS_KERN_FUNC_ID;

enum {
    EXT_MD_DORMANT_NOTIFY = 0x100,
    EXT_MD_SLP_REQUEST = 0x101,
    EXT_MD_TX_POWER = 0x102,
    EXT_MD_RF_TEMPERATURE = 0x103,
    EXT_MD_RF_TEMPERATURE_3G = 0x104,
    EXT_MD_GET_BATTERY_INFO = 0x105,
    EXT_MD_SIM_TYPE = 0x107,
};

typedef enum {
    EEMCS_RSM_ID_RESUME_WDT_IRQ = 0,
    EEMCS_RSM_ID_MD_LOCK_DORMANT = 1,
	EEMCS_RSM_ID_WAKE_UP_MD = 2,
	EEMCS_RSM_ID_MAX
}EEMCS_RESUME_ID;
 
typedef enum {
    EEMCS_SLP_ID_MD_FAST_DROMANT = 0,
    EEMCS_SLP_ID_MD_UNLOCK_DORMANT = 1,
    EEMCS_SLP_ID_MAX
}EEMCS_SLEEP_ID;

typedef enum {
    EEMCS_ID_GET_FDD_THERMAL_DATA = 0,
    EEMCS_ID_GET_TDD_THERMAL_DATA,
}EEMCS_SYS_CB_ID;


typedef enum {
    modem_invalid = 0,
/* remove to avoid redeclaration
	modem_2g = 1,
	modem_3g,
	modem_wg,
	modem_tg,
*/
    modem_lwg=5,
    modem_ltg,
}ext_modem_type_t;


//-------------structure define------------------------//
typedef int (*eemcs_kern_cb_func_t)(int, char *, unsigned int);
typedef struct{
    EEMCS_KERN_FUNC_ID id;
    eemcs_kern_cb_func_t func;
}eemcs_kern_func_info;

typedef int (*eemcs_sys_cb_func_t)(int, int);
typedef struct{
    EEMCS_SYS_CB_ID id;
    eemcs_sys_cb_func_t	func;
}eemcs_sys_cb_func_info_t;


//-----------------export function declaration----------------------------//
int parse_eemcs_dfo_setting(void *dfo_data, int num);
int parse_ext_meta_md_setting(unsigned char args[]);
void get_ext_md_post_fix(int md_id, char buf[], char buf_ex[]);
unsigned int get_ext_modem_is_enabled(int md_id);
unsigned int get_ext_modem_support(int md_id);
unsigned int set_ext_modem_support(int md_id, int md_type);
void get_ap_platform_ver(char * ver);
unsigned int get_nr_ext_modem(void);
unsigned int *get_ext_modem_size_list(void);
unsigned int get_ext_md_mem_start_addr(int md_id);
unsigned int get_ext_md_mem_size(int md_id);
int clear_md_region_protection(int md_id);
void eemcs_memory_reserve(void);

int eemcs_register_ccci_kern_func(unsigned int id, eemcs_kern_cb_func_t func);
int eemcs_register_ccci_kern_func_by_md_id(int md_id, unsigned int id, eemcs_kern_cb_func_t func);
int eemcs_exec_ccci_kern_func(unsigned int id, char *buf, unsigned int len);
int eemcs_exec_ccci_kern_func_by_md_id(int md_id, unsigned int id, char *buf, unsigned int len);

void eemcs_register_suspend_notify(int md_id, unsigned int id, void (*func)(int));
void eemcs_register_resume_notify(int md_id, unsigned int id, void (*func)(int));

int eemcs_register_sys_msg_notify_func(int md_id, int (*func)(int, unsigned int, unsigned int));
int eemcs_notify_md_by_sys_msg(int md_id, unsigned int msg, unsigned int data);

int eemcs_register_ccci_sys_call_back(int md_id, unsigned int id, eemcs_sys_cb_func_t func);
void eemcs_exec_ccci_sys_call_back(int md_id, int cb_id, int data);

int eemcs_get_bat_info(unsigned int para);

#endif
