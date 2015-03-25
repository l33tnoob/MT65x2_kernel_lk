#ifndef _MT_SPM_PCM_
#define _MT_SPM_PCM_

#include <linux/kernel.h>

/**************************************
 * only for internal debug
 **************************************/
//#ifdef CONFIG_MTK_LDVT
#if 1
    #define SPM_AP_ONLY_SLEEP       1
    #define SPM_PWAKE_EN            0
    #define SPM_BYPASS_SYSPWREQ     1
#else
    #define SPM_AP_ONLY_SLEEP       0
    #define SPM_PWAKE_EN            1
    #define SPM_BYPASS_SYSPWREQ     0
#endif

#define CON0_PCM_KICK               (1U << 0)
#define CON0_IM_KICK                (1U << 1)
#define CON0_IM_SLEEP_DVS           (1U << 3)
#define CON0_EVENT_VEC0_EN          (1U << 4)
#define CON0_EVENT_VEC1_EN          (1U << 5)
#define CON0_EVENT_VEC2_EN          (1U << 6)
#define CON0_EVENT_VEC3_EN          (1U << 7)
#define CON0_EVENT_VEC4_EN          (1U << 8)
#define CON0_EVENT_VEC5_EN          (1U << 9)
#define CON0_EVENT_VEC6_EN          (1U << 10)
#define CON0_EVENT_VEC7_EN          (1U << 11)
#define CON0_PCM_SW_RESET           (1U << 15)
#define CON0_CFG_KEY                (SPM_PROJECT_CODE << 16)

#define CON1_IM_SLAVE               (1U << 0)
#define CON1_MIF_APBEN              (1U << 3)
#define CON1_PCM_TIMER_EN           (1U << 5)
#define CON1_IM_NONRP_EN            (1U << 6)
#define CON1_CFG_KEY                (SPM_PROJECT_CODE << 16)
#define CON1_PCM_WDT_EN             (1U << 8)
#define CON1_PCM_WDT_WAKE_MODE      (1U << 9)/*0 Normal Mode*/

#define PWR_STATUS_FC1              (1U << 11)

#define PCM_PWRIO_EN_R1             (1U << 1)
#define PCM_PWRIO_EN_R2             (1U << 2)
#define PCM_PWRIO_EN_R0             (1U << 0)
#define PCM_PWRIO_EN_R7             (1U << 7)
#define PCM_RF_SYNC_R0              (1U << 16)
#define PCM_RF_SYNC_R7              (1U << 23)

#define CC_SYSCLK0_EN_0             (1U << 0)
#define CC_SYSCLK0_EN_1             (1U << 1)
//#define CC_SYSCLK1_EN_0             (1U << 2)
//#define CC_SYSCLK1_EN_1             (1U << 3)
#define CC_SYSSETTLE_SEL            (1U << 4)
#define CC_LOCK_INFRA_DCM           (1U << 5)
#define CC_SRCLKENA_MASK            (1U << 6)
//#define CC_SRCLKEN_MD_MASK        (1U << 7)
#define CC_SRCVOLT_MASK             (1U << 8)
#define CC_CXO32K_REMOVE_MD         (1U << 9)
#define CC_CXO32K_REMOVE_CONN       (1U << 10)
#define CC_CLKSQ0_SEL               (1U << 11)
#define CC_CLKSQ1_SEL               (1U << 12)
#define CC_DISABLE_SODI             (1U << 13)
#define CC_DISABLE_DORM_PWR         (1U << 14)
#define CC_DISABLE_INFRA_PWR        (1U << 15)
#define CC_SRCLKEN0_EN              (1U << 16)
//#define CC_SRCLKEN1_EN              (1U << 17)

#define SR_SRCLKENI_MASK            (1U << 1)

#define PCM_EVENT_NORMAL            (0x100000)
#define PCM_EVENT_ABORT             (0x10000)

/* Wakeup Source ID  */
#define WAKE_ID_PCM_TIMER          0    //Can't be masked up
#define WAKE_ID_TS                 1
#define WAKE_ID_KP                 2
#define WAKE_ID_WDT                3
#define WAKE_ID_GPT                4
#define WAKE_ID_EINT               5
#define WAKE_ID_CONN_WDT           6
#define WAKE_ID_SYSPWREQ           7
#define WAKE_ID_CCIF               8
#define WAKE_ID_LOW_BAT            9
#define WAKE_ID_CONN               10
#define WAKE_ID_26M_WAKE           11
#define WAKE_ID_26M_SLP            12
#define WAKE_ID_PCM_WDT            13
#define WAKE_ID_USB_CD             14
#define WAKE_ID_MD_WDT             15
#define WAKE_ID_USB_PDN            16
#define WAKE_ID_DBGSYS             18
#define WAKE_ID_UART0              19
#define WAKE_ID_AFE                20
#define WAKE_ID_THERM              21
#define WAKE_ID_CIRQ               22
#define WAKE_ID_SYSPWREQ_DUP       24
#define WAKE_ID_MD_DUP             25
#define WAKE_ID_CPU0_IRQ           26
#define WAKE_ID_CPU1_IRQ           27
//from conn sys & modem
#define WAKE_ID_AP_WAKE            30 //apsrc is merged from md_apsrc_req / conn_apsrc_req / mfg_req (3D) / disp_req (Display)
#define WAKE_ID_AP_SLEEP           31


/* PCM R7 */
#define R7_UART_CLK_OFF_REQ         (1U << 0)
#define R7_wdt_kick_p               (1U << 22)

/* PCM R12 */
#define WAKE_SRC_PCM_TIMER          (1U << WAKE_ID_PCM_TIMER)
#define WAKE_SRC_TS                 (1U << WAKE_ID_TS       )
#define WAKE_SRC_KP                 (1U << WAKE_ID_KP       )
#define WAKE_SRC_WDT                (1U << WAKE_ID_WDT      )
#define WAKE_SRC_GPT                (1U << WAKE_ID_GPT      )
#define WAKE_SRC_EINT               (1U << WAKE_ID_EINT     )
#define WAKE_SRC_CONN_WDT           (1U << WAKE_ID_CONN_WDT )
#define WAKE_SRC_SYSPWREQ           (1U << WAKE_ID_SYSPWREQ )
#define WAKE_SRC_CCIF               (1U << WAKE_ID_CCIF     )
#define WAKE_SRC_LOW_BAT            (1U << WAKE_ID_LOW_BAT  )
#define WAKE_SRC_CONN               (1U << WAKE_ID_CONN     )
#define WAKE_SRC_26M_WAKE           (1U << WAKE_ID_26M_WAKE )
#define WAKE_SRC_26M_SLP            (1U << WAKE_ID_26M_SLP  )
#define WAKE_SRC_PCM_WDT0           (1U << WAKE_ID_PCM_WDT0 )
#define WAKE_SRC_USB_CD             (1U << WAKE_ID_USB_CD   )
#define WAKE_SRC_MD_WDT             (1U << WAKE_ID_MD_WDT   )
#define WAKE_SRC_USB_PDN            (1U << WAKE_ID_USB_PDN  )
#define WAKE_SRC_DBGSYS             (1U << WAKE_ID_DBGSYS   )
#define WAKE_SRC_UART0              (1U << WAKE_ID_UART0    )
#define WAKE_SRC_AFE                (1U << WAKE_ID_AFE      )
#define WAKE_SRC_THERM              (1U << WAKE_ID_THERM    )
#define WAKE_SRC_CIRQ               (1U << WAKE_ID_CIRQ     )
//This two bits are dupilicated, always mask it!!
#define WAKE_SRC_SYSPWREQ_DUP       (1U << WAKE_ID_SYSPWREQ_DUP)
#define WAKE_SRC_MD_DUP             (1U << WAKE_ID_MD_DUP)

#define WAKE_SRC_CPU0_IRQ           (1U << WAKE_ID_CPU0_IRQ)
#define WAKE_SRC_CPU1_IRQ           (1U << WAKE_ID_CPU1_IRQ)
//from conn sys & modem
#define WAKE_SRC_AP_WAKE            (1U << WAKE_ID_AP_WAKE ) //apsrc is merged from md_apsrc_req / conn_apsrc_req / mfg_req (3D) / disp_req (Display)
#define WAKE_SRC_AP_SLEEP           (1U << WAKE_ID_AP_SLEEP)

//#define WAKE_SRC_ALWAYS_ON   (WAKE_SRC_PCM_TIMER & WAKE_SRC_26M_WAKE & WAKE_SRC_26M_SLP ))
#define WAKE_SRC_ALWAYS_OFF  ((1U<<17)& (1U<<23) &WAKE_ID_SYSPWREQ_DUP &WAKE_ID_MD_DUP &(1U<<28)&(1U<<29))
#define spm_is_wakesrc_invalid(wakeid) ((wakeid & WAKE_SRC_ALWAYS_OFF) != 0)//Scott, why we need !!??

/* PCM R13 */
#define R13_CONN_APSRC_REQ          (1U << 2)
#define R13_MD_STATE                (1U << 7)
#define R13_CONN_STATE              (1U << 11)
#define R13_UART_CLK_OFF_ACK        (1U << 20)


#define EVENT_VEC(event, resume, imme, pc)  \
    (((event) << 0) | ((resume) << 5) | ((imme) << 6) | ((pc) << 16))

#define SPM_PCM_MAX_VSR_NUM	8
#define SPM_PCM_RECORD_NUM  8
#define SPM_PARSE_BUFF_LEN  200


typedef enum
{
	SPM_PCM_KERNEL_SUSPEND=0,
	SPM_PCM_DEEP_IDLE,
	SPM_PCM_MCDI,
	SPM_PCM_WDT,
	SPM_PCM_SCENARIO_NUM
}SPM_PCM_SCENARIO;

typedef enum {              
    WR_NONE = 0,      
    WR_UART_BUSY,
    WR_PCM_ASSERT,
    WR_PCM_SLEEP_ABORT,
    WR_WAKE_SRC,
    WR_SW_ABORT,
    WR_PCM_TIMER,
    WR_ERROR
} wake_reason_t;      

typedef struct {
    u32 PCM_CON0;
    u32 PCM_CON1;
    u32 PCM_IM_LEN;
    u32 PCM_REG_DATA_INI;
    u32 PCM_EVENT_VECTOR0;
    u32 PCM_TIMER_VAL;
    u32 PCM_TIMER_OUT;
    u32 PCM_EVENT_REG_STA;
    u32 AP_STANBY_CON;
    u32 SLEEP_CPU_WAKEUP_EVENT;
    u32 SLEEP_ISR_STATUS; 
    u32 PCM_REG_DATA[16];
    u32 SPM_CLK_CON_STA;
}PCM_DBG_REG;

typedef struct {
    wake_reason_t wake_reason;
    u32 wakeup_event;
    PCM_DBG_REG  pcm_dbg_reg;
} wake_status_t;

typedef struct
{
		SPM_PCM_SCENARIO scenario;	
	   // u8  cpu_status;                 //Currently not used, paremeter passed tp cpu_power_down()
	    
	   char*    ver;
	    
	/*6589: spm_set_sysclk_settle*/
		bool    spm_turn_off_26m;	    //System 26MHz clock settle time by 32KHz clock cycles. Max : 7.8ms.
		enum    APMCU_PWRCTL{PWR_LV0=0x01,PWR_LV1=0x02,PWR_LV2=0x04,PWR_LVNA=0xFF} pcm_pwrlevel;           //Used only in Deepidle
		//enum 	SYSSETTLE_SEL{MD1=0,MD2} sys_26m_src; //Select settle time source
	
	/*6589: spm_kick_im_to_fetch*/
	    u32 	pcm_firmware_addr;		//Address of the SPM firmware
		u16	pcm_firmware_len;		//Size of the SPM firmware (number of bytes)
		
	/*6589:spm_request_uart_to_sleep*/
		bool 	spm_request_uart_sleep;	//call spm_request_uart_to_sleep or not;
		
    /*6589:spm_init_event_vector*/      //Vector Service Routine
        u32     pcm_vsr[SPM_PCM_MAX_VSR_NUM];
        
    /*spm_init_pcm_register*/
        bool    sync_r0r7;
  	
  /*6589:spm_set_ap_standbywfi*/
        enum    MD_MASK_B{MDCONN_MASK=0,CONN_MASK,MD_MASK,MDCONN_UNMASK} md_mask;
        enum    MM_MASK_B{MMALL_MASK=0,MFG_MASK,DISP_MASK,MMALL_UNMASK} mm_mask;
        
        bool    wfi_scu_mask; //MASK bits of request signals
		bool    wfi_l2c_mask;
		enum 	WFI_OP{REDUCE_OR=0,REDUCE_AND} wfi_op; //wfi operations
		
		bool    wfi_sel[2]; //Currently we turn on/off wfi_sel together

        bool    sodi_en;
		
    /*6589: spm_set_wakeup_event*/
		u64     timer_val_ms;	  //Timer Event Trigger Value (in the unit of 1/1024 seconds, need to convert to 32Khz )
		u64     wdt_val_ms;       //0: Disable,
		u32     wake_src;     //Mask the wake-up event and this event will not wake up the sleep procedure
		//bool spm_unmask_isr;
		
	/*spm_kick_pcm_to_run*/
		bool cpu_pdn;
		bool infra_pdn;
	    bool coclock_en;  //To co-clock with SRCCLEKNI(6605)
	    bool lock_infra_dcm;
		
	/* Reserved Bit for each scenario */
	    u32 reserved;
	
	/* Debug related registers */	
		u32 dbg_wfi_cnt;  //counter of entering wfi
		
		wake_status_t *last_wakesta;
		u32 wakesta_idx;
		wake_status_t wakesta[SPM_PCM_RECORD_NUM];
	    char *result[SPM_PARSE_BUFF_LEN];
		
}SPM_PCM_CONFIG;

/**********************Reserved Bit of Suspend************************/
#define SPM_SUSPEND_GET_FGUAGE         (1U << 0)//0:timer_val_ms, 1:FGUAGE 

/*************SPM Internal APIs****************/
bool spm_init_pcm(SPM_PCM_CONFIG* pcm_config);
void spm_kick_pcm(SPM_PCM_CONFIG* pcm_config);
void spm_trigger_wfi(SPM_PCM_CONFIG* pcm_config);
void spm_clean_after_wakeup(void);
void spm_pcm_dump_regs(void);
wake_status_t* spm_get_wakeup_status(SPM_PCM_CONFIG* pcm_config);

void spm_wfi_sel(bool core_wfi_sel[], u8 core_wfi_sw_mask);

bool spm_cpusys_can_power_down(void);

unsigned int spm_wakesrc_mask_all(SPM_PCM_SCENARIO scenario);
void spm_wakesrc_restore(SPM_PCM_SCENARIO scenario,u32 backup_wakesrc);
void spm_wakesrc_clear(SPM_PCM_SCENARIO scenario,u32 wakeid);
bool spm_wakesrc_set(SPM_PCM_SCENARIO scenario,u32 wakeid);

void spm_timer_disable(SPM_PCM_SCENARIO scenario);
bool spm_infra_pdn_enable(SPM_PCM_SCENARIO scenario,bool enable);
bool spm_cpu_pdn_enable(SPM_PCM_SCENARIO scenario, bool enable);

const char* spm_get_wake_up_result(SPM_PCM_SCENARIO scenario);
wake_status_t* spm_get_last_wakeup_status(void);
void spm_direct_disable_sodi(void);
void spm_direct_enable_sodi(void);
void spm_mcdi_init_core_mux(void);
void spm_mcdi_clean(void);
void spm_mcdi_poll_mask(u32 core_id,bool core_wfi_sel[]);
void spm_dump_pll_regs(void);


#endif



