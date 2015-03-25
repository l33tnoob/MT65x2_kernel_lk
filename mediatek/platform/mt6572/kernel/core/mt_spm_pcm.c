#include <linux/delay.h>
#include <mach/mt_spm.h>
#include <mach/eint.h>
#include <mach/mtk_ccci_helper.h>
#include <mach/mt_clkmgr.h>
#include <mach/mt_dcm.h>
#include <mach/mt_mci.h>
#include <mach/mt_dormant.h>

#define SPM_SYSCLK_SETTLE       99      /* 3ms */
#define WAIT_UART_ACK_TIMES     10      /* 10 * 10us */

/**********************************************PCM Internal functions**************************************************/
const char *pcm_scenario[SPM_PCM_SCENARIO_NUM]={"Kernel Suspend","Deep Idle","MCDI","WDT"};
static char *pcm_wakeup_reason[]=
{
    "PCM_TIMER",//0
    "TS",//1
    "KP",//2
    "WDT",// 3
    "GPT",//4
    "EINT",//5
    "CONN_WDT",//6
    "SYSPWREQ",//7
    "CCIF",//8
    "LOW_BAT",//9
    "CONN",//10
    "26M_WAKE",//11
    "26M_SLP",//12
    "PCM_WDT0",//13
    "USB_CD",//14
    "MD_WDT",//15
    "USB_PDN",//16
    "ERR 17",
    "DBGSYS",//18
    "UART0",//19
    "AFE",//20
    "THERM",//21
    "CIRQ",//22
    "ERR 23",//23
    "SYSPWREQ_DUP",//24
    "MD_DUP",//25
    "CPU0_IRQ",//26
    "CPU1_IRQ",//27
    "ERR CPU2_IRQ",//28
    "ERR CPU3_IRQ",//29
    "AP_WAKE",//30
    "AP_SLEEP"//31
};

extern SPM_PCM_CONFIG pcm_config_suspend;
extern SPM_PCM_CONFIG pcm_config_dpidle;
extern SPM_PCM_CONFIG pcm_config_mcdi;

#ifdef CONFIG_KICK_SPM_WDT
extern SPM_PCM_CONFIG pcm_config_wdt;
#else
SPM_PCM_CONFIG pcm_config_wdt;
#endif

static SPM_PCM_CONFIG *pcm_config_curr;
static SPM_PCM_CONFIG *pcm_config_arr[SPM_PCM_SCENARIO_NUM]={&pcm_config_suspend,&pcm_config_dpidle,&pcm_config_mcdi,&pcm_config_wdt};

static void spm_set_sysclk_settle(void)
{
    u32 md_settle, settle;
    
    /* MD SYSCLK settle is from MD1 */
    spm_write(SPM_CLK_CON, spm_read(SPM_CLK_CON) | CC_SYSSETTLE_SEL);
    
    /* get MD SYSCLK settle */
    spm_write(SPM_CLK_SETTLE, 0);           
    md_settle = spm_read(SPM_CLK_SETTLE);
    
    /* SYSCLK settle = MD SYSCLK settle but change it if needed */
    #if SPM_AP_ONLY_SLEEP
        spm_write(SPM_CLK_SETTLE, SPM_SYSCLK_SETTLE - md_settle);
    #endif
        settle = spm_read(SPM_CLK_SETTLE);
    
   spm_crit2("md_settle = %u, settle = %u\n", md_settle, settle);
}

static void spm_reset_and_init_pcm(void)
{
    /* reset PCM */
    spm_write(SPM_PCM_CON0, CON0_CFG_KEY | CON0_PCM_SW_RESET);
    spm_write(SPM_PCM_CON0, CON0_CFG_KEY);
    
    /* init PCM control register (disable event vector and PCM timer) */
    spm_write(SPM_PCM_CON0, CON0_CFG_KEY | CON0_IM_SLEEP_DVS);
    
    if(pcm_config_curr->scenario == SPM_PCM_WDT){
       // spm_write(SPM_PCM_CON1, CON1_CFG_KEY & ~(CON1_IM_NONRP_EN | CON1_MIF_APBEN));
        spm_write(SPM_PCM_CON1, CON1_CFG_KEY | CON1_MIF_APBEN);
    }
    else if(pcm_config_curr->scenario == SPM_PCM_DEEP_IDLE)
      spm_write(SPM_PCM_CON1, CON1_CFG_KEY | CON1_IM_NONRP_EN | CON1_MIF_APBEN | (spm_read(SPM_PCM_CON1)&(CON1_PCM_WDT_EN | CON1_PCM_WDT_WAKE_MODE)));
    else    
      spm_write(SPM_PCM_CON1, CON1_CFG_KEY | CON1_IM_NONRP_EN | CON1_MIF_APBEN);
        
}

static void spm_kick_im_to_fetch(u32 code_base, u16 code_len)
{
    u32 con0;

    /* tell IM where is PCM code */
    BUG_ON(code_base & 0x00000003);     /* check 4-byte alignment */
    spm_write(SPM_PCM_IM_PTR, code_base);
    spm_write(SPM_PCM_IM_LEN, code_len);

    /* kick IM to fetch */
    con0 = spm_read(SPM_PCM_CON0);
    spm_write(SPM_PCM_CON0, con0 | CON0_CFG_KEY | CON0_IM_KICK);
    spm_write(SPM_PCM_CON0, con0 | CON0_CFG_KEY);
}

static int spm_request_uart_to_sleep(void)
{
    u32 val1;
    int i = 0;

    /* request UART to sleep */
    val1 = spm_read(SPM_POWER_ON_VAL1);
    spm_write(SPM_POWER_ON_VAL1, val1 | R7_UART_CLK_OFF_REQ);

    /* wait for UART to ACK */
    while (!(spm_read(SPM_PCM_REG13_DATA) & R13_UART_CLK_OFF_ACK)) {
        if (i++ >= WAIT_UART_ACK_TIMES) {
            spm_write(SPM_POWER_ON_VAL1, val1);
            spm_error2("!!! CANNOT GET SLEEP ACK FROM UART !!!\n");
            return -EBUSY;
        }
        udelay(10);
    }

    return 0;
}

static void spm_init_pcm_register(void)
{
     /* init r0 with POWER_ON_VAL0 */
     spm_write(SPM_PCM_REG_DATA_INI, spm_read(SPM_POWER_ON_VAL0));
     spm_write(SPM_PCM_PWR_IO_EN, PCM_RF_SYNC_R0);
     //mdelay(1000);//spm need 2T @26M to sync data
     spm_write(SPM_PCM_PWR_IO_EN, 0);
     
     /* init r7 with POWER_ON_VAL1 */
     spm_write(SPM_PCM_REG_DATA_INI, spm_read(SPM_POWER_ON_VAL1));
     spm_write(SPM_PCM_PWR_IO_EN, PCM_RF_SYNC_R7);
     //mdelay(1000);//spm need 2T @26M to sync data
     spm_write(SPM_PCM_PWR_IO_EN, 0);
     
     /* clear REG_DATA_INI for PCM after init rX */
     spm_write(SPM_PCM_REG_DATA_INI, 0);
}

static void spm_init_event_vector(u32 vec_cfg[])
{
    /* init event vector register */
    spm_write(SPM_PCM_EVENT_VECTOR0, vec_cfg[0]);
    spm_write(SPM_PCM_EVENT_VECTOR1, vec_cfg[1]);
    spm_write(SPM_PCM_EVENT_VECTOR2, vec_cfg[2]);
    spm_write(SPM_PCM_EVENT_VECTOR3, vec_cfg[3]);
    spm_write(SPM_PCM_EVENT_VECTOR4, vec_cfg[4]);
    spm_write(SPM_PCM_EVENT_VECTOR5, vec_cfg[5]);
    spm_write(SPM_PCM_EVENT_VECTOR6, vec_cfg[6]);
    spm_write(SPM_PCM_EVENT_VECTOR7, vec_cfg[7]);
    /* event vector will be enabled by PCM itself */
}

static void spm_set_ap_pwrctl(enum APMCU_PWRCTL pwrlevel)
{
    u32 pwrctl = pwrlevel;
#if 0
    if(pwrlevel==PWR_LVNA) //Kernel Suspend
    {
     pwrctl = 0;

    if (spm_read(INFRA_DCMCTL) & (1U << 8))
        pwrctl |= (1U << 0);    /* input INFRA DCM info for INFRA PDN */

    if (!(spm_read(SPM_MD1_PWR_CON) & (0x3 << 2)))
        pwrctl |= (1U << 1);    /* input MD1 PDN info for WHQA_00013637 */
    if (!(spm_read(SPM_MD2_PWR_CON) & (0x3 << 2)))
        pwrctl |= (1U << 2);    /* input MD2 PDN info for WHQA_00013637 */
    }
#endif    
    spm_write(SPM_APMCU_PWRCTL, pwrctl);
}



void spm_wfi_sel(bool core_wfi_sel[], u8 core_wfi_sw_mask)
{
    if( ~( core_wfi_sw_mask & SPM_CORE0_WFI_SEL_SW_MASK ) )
        spm_write(SPM_CORE0_WFI_SEL, core_wfi_sel[0]);
    if( ~( core_wfi_sw_mask & SPM_CORE1_WFI_SEL_SW_MASK ) )
        spm_write(SPM_CORE1_WFI_SEL, core_wfi_sel[1]);
    spm_write(SPM_CORE2_WFI_SEL, 0);   /*MT6572 only has 2 cores*/
    spm_write(SPM_CORE3_WFI_SEL, 0);   /*MT6572 only has 2 cores*/


}


static void spm_set_ap_standbywfi(u32 md_mask,u32 mm_mask,u32 wfi_scu_mask,u32 wfi_l2c_mask,u32 wfi_op,bool core_wfi_sel[])
{
  //spm_crit2("md_mask:0x%X,mm_mask:0x%X,wfi_scu_mask:0x%X,wfi_l2c_mask:0x%X,wfi_op:0x%X,wfi_sel:0x%X\n",md_mask,mm_mask,wfi_scu_mask,wfi_l2c_mask,wfi_op,wfi_sel);
   
   spm_write(SPM_AP_STANBY_CON, (md_mask << 19) |  /* unmask MD1 and MD2 */
                                 (mm_mask << 16) |  /* unmask DISP and MFG */
                                 (wfi_scu_mask << 6) |     /* check SCU idle */
                                 (wfi_l2c_mask << 5) |     /* check L2C idle */
                                 (wfi_op << 4));    /* Reduce AND */

   spm_wfi_sel(core_wfi_sel, 0 );
}


static void spm_set_wakeup_event(u32 timer_val,u32 wdt_val, u32 wake_src)
{
    /* set PCM timer (set to max when disable) */
    //spm_write(SPM_PCM_TIMER_VAL, timer_val ? : 0xffffffff);
    //spm_write(SPM_PCM_CON1, spm_read(SPM_PCM_CON1) | CON1_CFG_KEY | CON1_PCM_TIMER_EN);
    /*Fix 32K less*/
    spm_write(SPM_PCM_TIMER_VAL, 0xffffffff);
    spm_write(SPM_PCM_CON1, spm_read(SPM_PCM_CON1) | CON1_CFG_KEY | CON1_PCM_TIMER_EN);
    
    /* set PCM WDT */
    if(wdt_val > 0){
    spm_write(SPM_PCM_WDT_TIMER_VAL, wdt_val);
    spm_write(SPM_PCM_CON1, spm_read(SPM_PCM_CON1) | CON1_CFG_KEY | CON1_PCM_WDT_EN | CON1_PCM_WDT_WAKE_MODE);
    }else{
     spm_write(SPM_PCM_WDT_TIMER_VAL, 0);
     spm_write(SPM_PCM_CON1, (spm_read(SPM_PCM_CON1)| CON1_CFG_KEY )& ~(CON1_PCM_WDT_EN | CON1_PCM_WDT_WAKE_MODE));  
    }
    

    /* unmask wakeup source */
#if SPM_BYPASS_SYSPWREQ
    wake_src &= ~WAKE_SRC_SYSPWREQ;     /* make 26M off when attach ICE */
#endif
    spm_write(SPM_SLEEP_WAKEUP_EVENT_MASK, ~wake_src);

    /* unmask SPM ISR */
    spm_write(SPM_SLEEP_ISR_MASK, 0x200);
}

static void spm_trigger_wfi_for_sleep(bool cpu_pdn, bool infra_pdn)
{
    //disable_peri_dcm();     /* workaround for WHQA_00013158 */

/*    if (infra_pdn)
        disable_infra_dcm();     EMI needs clock to sync register back */

    if (cpu_pdn) {
        if (!cpu_power_down(SHUTDOWN_MODE)) {
            /* do not add code here */
            wfi_with_sync();
        }
        cpu_check_dormant_abort();
    } else {
        //mci_snoop_sleep();
        wfi_with_sync();
        //mci_snoop_restore();
    }

    if (infra_pdn) {
        //restore_infra_dcm();
        
        if(console_suspend_enabled==0)
            mtk_uart_restore();
    }

    //restore_peri_dcm();
}

#if 0
static u32 spm_dpidle_axi_bus_clk;
static u32 spm_dpidle_apb_ahb_set;
#endif

void spm_trigger_wfi_for_dpidle(bool cpu_pdn)
{
    //disable_peri_dcm();     /* workaround for WHQA_00013158 */

    /* make PCM to be able to change voltage through PWRAP */
//    pmicspi_mempll2clksq();

    if (cpu_pdn) {
        if (!cpu_power_down(DORMANT_MODE)) {
#if 0
             //bus switch to 26MHz
            spm_dpidle_axi_bus_clk = spm_read(0xf0000000);
            spm_write(0xf0000000,(spm_dpidle_axi_bus_clk&0xffffff1f)|0x20);
            //ahb:apb[1:1]
            spm_dpidle_apb_ahb_set = spm_read(0xf000002c);
            spm_write(0xf000002c,spm_dpidle_apb_ahb_set&0x7fffffff);
#endif            
            /* do not add code here */
            wfi_with_sync();
#if 0
            //ahb:apb restore
            spm_write(0xf000002c,spm_dpidle_apb_ahb_set);
            //bus clk restore
            spm_write(0xf0000000,spm_dpidle_axi_bus_clk);
#endif            
        }
        cpu_check_dormant_abort();
    } else {
       // mci_snoop_sleep();
        wfi_with_sync();
       // mci_snoop_restore();
    }

    /* restore PWRAP clock source */
//    pmicspi_clksq2mempll();

    //restore_peri_dcm();
}



/**********************************************PCM OPEN APIs**************************************************/
bool spm_cpusys_can_power_down(void)
{
    return (!(spm_read(SPM_PWR_STATUS)&PWR_STATUS_FC1) && 
            !(spm_read(SPM_PWR_STATUS_S)&PWR_STATUS_FC1));
    
}

/*
Arguments: scenario : (SPM_PCM_KERNEL_SUSPEND or SPM_PCM_DEEP_IDLE or SPM_PCM_MCDI]
Return   : The wakesrc that has been replaced.
*/
unsigned int spm_wakesrc_mask_all(SPM_PCM_SCENARIO scenario)
{
    unsigned int backup_wakesrc;
    unsigned long flags;
    SPM_PCM_CONFIG* pcm_config = pcm_config_arr[scenario];
    spin_lock_irqsave(&spm_lock, flags);
    backup_wakesrc = pcm_config->wake_src;
    pcm_config->wake_src=0;
    spin_unlock_irqrestore(&spm_lock, flags);
    return backup_wakesrc;
}

/*
Arguments: scenario: (SPM_PCM_KERNEL_SUSPEND or SPM_PCM_DEEP_IDLE or SPM_PCM_MCDI]
           backup_wakesrc : The wakesrc needed to be restore
*/
void spm_wakesrc_restore(SPM_PCM_SCENARIO scenario,unsigned int backup_wakesrc)
{
    unsigned long flags;
    SPM_PCM_CONFIG* pcm_config = pcm_config_arr[scenario];
    spin_lock_irqsave(&spm_lock, flags);
    pcm_config->wake_src = backup_wakesrc;
    spin_unlock_irqrestore(&spm_lock, flags);    
  
}

/*
Arguments: scenario: (SPM_PCM_KERNEL_SUSPEND or SPM_PCM_DEEP_IDLE or SPM_PCM_MCDI]
           backup_wakesrc : The wakesrc needed to be clear
*/
void spm_wakesrc_clear(SPM_PCM_SCENARIO scenario,unsigned int wakeid)
{
    unsigned long flags;
    SPM_PCM_CONFIG* pcm_config = pcm_config_arr[scenario];
    spin_lock_irqsave(&spm_lock, flags);
    pcm_config->wake_src &= ~(1U<<wakeid);
    spin_unlock_irqrestore(&spm_lock, flags);
}

bool spm_wakesrc_set(SPM_PCM_SCENARIO scenario,unsigned int wakeid)
{
    unsigned long flags;
    SPM_PCM_CONFIG* pcm_config = pcm_config_arr[scenario];
    
    if(wakeid>32) 
        return false;
    
    wakeid = 1U << wakeid;
    
    //if(spm_is_wakesrc_invalid(wakeid)){
        spin_lock_irqsave(&spm_lock, flags);
        pcm_config->wake_src |= (wakeid & (~WAKE_SRC_ALWAYS_OFF));
        spin_unlock_irqrestore(&spm_lock, flags);
        return true;   
    //}
  //return false;
}

void spm_timer_disable(SPM_PCM_SCENARIO scenario)
{
    SPM_PCM_CONFIG* pcm_config = pcm_config_arr[scenario];
    pcm_config->timer_val_ms = 0;
}

bool spm_infra_pdn_enable(SPM_PCM_SCENARIO scenario,bool enable)
{
    unsigned long flags;
    SPM_PCM_CONFIG* pcm_config = pcm_config_arr[scenario];
    spin_lock_irqsave(&spm_lock, flags);
    if(!pcm_config->cpu_pdn && enable)
        return false;
    pcm_config->infra_pdn = enable;
    spin_unlock_irqrestore(&spm_lock, flags);   
    return true; 
}

bool spm_cpu_pdn_enable(SPM_PCM_SCENARIO scenario,bool enable)
{
    unsigned long flags;
    SPM_PCM_CONFIG* pcm_config = pcm_config_arr[scenario];
    spin_lock_irqsave(&spm_lock, flags);
    if(!enable && pcm_config->infra_pdn)
        return false;
    pcm_config->cpu_pdn = enable;
    spin_unlock_irqrestore(&spm_lock, flags);
    return true; 
}

bool spm_init_pcm(SPM_PCM_CONFIG* pcm_config)
{
  u32 timer_val,wdt_timer_val;
  bool wfi_sel[2] = {false,false};

  pcm_config_curr =   pcm_config;
    
    if(pcm_config->scenario == SPM_PCM_DEEP_IDLE)
        spm_crit2("%s with cpu_pdn=%d, pwr_level=%d, infra_pdn=%d\n",pcm_scenario[pcm_config->scenario],pcm_config->cpu_pdn,pcm_config->pcm_pwrlevel,pcm_config->infra_pdn);
    else
        spm_crit2("%s with cpu_pdn=%d, infra_pdn=%d\n",pcm_scenario[pcm_config->scenario],pcm_config->cpu_pdn,pcm_config->infra_pdn);
     //spm_crit2("spm_init_pcm():%s, timer_val: %d, wake_src:0x%X \n",pcm_scenario[pcm_config->scenario],pcm_config->timer_val_sec,pcm_config->wake_src);


  /*
  if(pcm_config->scenario == SPM_PCM_KERNEL_SUSPEND)
    pcm_config_curr->cpu_status = SHUTDOWN_MODE;
  else if(pcm_config->scenario == SPM_PCM_DEEP_IDLE)
    pcm_config_curr->cpu_status = DORMANT_MODE;*/
    
      
  pcm_config->wakesta[pcm_config->wakesta_idx].wake_reason = WR_NONE ;
  
  spm_write(SPM_POWERON_CONFIG_SET, (SPM_PROJECT_CODE << 16) | (1U << 0));

 
   //if pcm will turn on and off the 26Mhz, we need to setup the settle time
    if(pcm_config->spm_turn_off_26m)
        spm_set_sysclk_settle();

    spm_reset_and_init_pcm();
    
    /*init Instruction Memory*/
    spm_kick_im_to_fetch(pcm_config->pcm_firmware_addr,pcm_config->pcm_firmware_len);

    if (pcm_config->spm_request_uart_sleep)
        if(spm_request_uart_to_sleep()){
            return false;
    }
    
    /*init R0 ,R7*/
    if(pcm_config->sync_r0r7)
        spm_init_pcm_register();
    
	
	/*init VSRs*/
	spm_init_event_vector(pcm_config->pcm_vsr);

    /*Setup pwr level if necessary*/
    spm_set_ap_pwrctl(pcm_config->pcm_pwrlevel);

    if(pcm_config->scenario == SPM_PCM_MCDI)
    {
    	/* set standby wfi */
         spm_set_ap_standbywfi(pcm_config->md_mask,
    	                      pcm_config->mm_mask,
    	                      pcm_config->wfi_scu_mask,
    	                      pcm_config->wfi_l2c_mask,
    	                      pcm_config->wfi_op,
    	                      wfi_sel);

    }
    else
    {
        
    	/* set standby wfi */
         spm_set_ap_standbywfi(pcm_config->md_mask,
    	                      pcm_config->mm_mask,
    	                      pcm_config->wfi_scu_mask,
    	                      pcm_config->wfi_l2c_mask,
    	                      pcm_config->wfi_op,
    	                      pcm_config->wfi_sel);
    }
	
    timer_val= (unsigned int)((pcm_config->timer_val_ms * 32768)/1024);//In the unit of 32Khz
    wdt_timer_val = (unsigned int)((pcm_config->wdt_val_ms * 32768)/1024);//In the unit of 32Khz
	
    //spm_set_wakeup_event(pcm_config->timer_val_sec * 32768,pcm_config->wake_src);
//if(pcm_config->scenario != SPM_PCM_WDT) 
    spm_set_wakeup_event(timer_val,wdt_timer_val,pcm_config->wake_src);

    return true;
    
}

void spm_kick_pcm(SPM_PCM_CONFIG* pcm_config)
{
    u32 clk,con0;
    
    BUG_ON(pcm_config!=pcm_config_curr);
    
    //spm_crit2("spm_kick_pcm():%s, cpu_pdn:%d, infra_pdn:%d \n",pcm_scenario[pcm_config->scenario],pcm_config_curr->cpu_pdn,pcm_config_curr->infra_pdn);

    /* keep CPU or INFRA/DDRPHY power if needed and lock INFRA DCM */
    clk = spm_read(SPM_CLK_CON) & ~(CC_DISABLE_DORM_PWR | CC_DISABLE_INFRA_PWR);
    if (!pcm_config->cpu_pdn)
        clk |= CC_DISABLE_DORM_PWR;
    if (!pcm_config->infra_pdn)
        clk |= CC_DISABLE_INFRA_PWR;
    
    if(pcm_config->lock_infra_dcm)    
        clk |= CC_LOCK_INFRA_DCM;
        
    spm_write(SPM_CLK_CON, clk);

    /* init pause request mask for PCM */
    spm_write(SPM_PCM_MAS_PAUSE_MASK, 0xffffffff);

    /* enable r0 and r7 to control power */
    if(pcm_config->sync_r0r7)
    {
        spm_write(SPM_PCM_PWR_IO_EN, PCM_PWRIO_EN_R0 | PCM_PWRIO_EN_R7 | PCM_PWRIO_EN_R1 | PCM_PWRIO_EN_R2);
        
        /* In order to prevent glitch from SRCLKENA,we move the co-clock logic to SPM 26M-wake/sleep VSR in MT6572/82 */
        spm_write(SPM_CLK_CON, spm_read(SPM_CLK_CON) |CC_SRCLKENA_MASK );
     }
     else
        spm_write(SPM_PCM_PWR_IO_EN, 0);

    /* for E2 enable LPM to do 32k-less feature*/
    spm_write(SPM_CLK_CON, spm_read(SPM_CLK_CON) |CC_CXO32K_REMOVE_MD);


    
    if(pcm_config->coclock_en){
        spm_write(SPM_PCM_SRC_REQ, spm_read(SPM_PCM_SRC_REQ) |SR_SRCLKENI_MASK );}
    else{
        spm_write(SPM_PCM_SRC_REQ, 0 );} //In the case of no 6605

/**************Disable Switch Bus to 26M from TY******************/
#if 0
spm_write(0xf0000020,spm_read(0xf0000020) & (~0x040));
#endif

    /* kick PCM to run */
    con0 = spm_read(SPM_PCM_CON0);
    con0 &= ~(CON0_PCM_KICK | CON0_IM_KICK);
    spm_write(SPM_PCM_CON0, con0 | CON0_CFG_KEY | CON0_PCM_KICK);
    spm_write(SPM_PCM_CON0, con0 | CON0_CFG_KEY);
}

void spm_trigger_wfi(SPM_PCM_CONFIG* pcm_config)
{
    BUG_ON(pcm_config!=pcm_config_curr);
    
    pcm_config_curr->dbg_wfi_cnt++;
    
    //spm_crit2("spm_trigger_wfi():%s, cpu_pdn:%d, infra_pdn:%d \n",pcm_scenario[pcm_config->scenario],pcm_config_curr->cpu_pdn,pcm_config_curr->infra_pdn);
    
    if(pcm_config->scenario == SPM_PCM_KERNEL_SUSPEND)
        spm_trigger_wfi_for_sleep(pcm_config_curr->cpu_pdn,pcm_config_curr->infra_pdn);
    else if(pcm_config->scenario == SPM_PCM_DEEP_IDLE)
        spm_trigger_wfi_for_dpidle(pcm_config_curr->cpu_pdn);
}

static u32 spm_parse_wake_reason(SPM_PCM_CONFIG* pcm_config, wake_status_t* wake_status_p)
{
    int i,len;
    char *p,*buff= (char*)&pcm_config->result[0];
    PCM_DBG_REG  *pcm_dbg_reg= &wake_status_p->pcm_dbg_reg;
    
    p=buff;
    
    memset(p,0,SPM_PARSE_BUFF_LEN);
    
    p += sprintf(p, "VER = %s\n",pcm_config->ver);
    p += sprintf(p, "PCM_EVENT_REG_STA = 0x%X\n",pcm_dbg_reg->PCM_EVENT_REG_STA);
    p += sprintf(p, "PCM_R9 = 0x%X\n",pcm_dbg_reg->PCM_REG_DATA[9]);
    p += sprintf(p, "PCM_R12 = 0x%X\n",pcm_dbg_reg->PCM_REG_DATA[12]);
    p += sprintf(p, "PCM_R13 = 0x%X\n",pcm_dbg_reg->PCM_REG_DATA[13]);
    
    switch(wake_status_p->wake_reason)
    {
        case WR_PCM_ASSERT:    
            p += sprintf(p, "[PCM ASSERT] at PC= %d\n",pcm_dbg_reg->PCM_REG_DATA_INI);
            goto out;
        case WR_PCM_SLEEP_ABORT:
            p += sprintf(p, "[PCM SLEEP ABORT]");
            break;
        case WR_WAKE_SRC:
            p += sprintf(p, "[PCM WAKEUP NORMAL]");
            break;
        case WR_ERROR:
            p += sprintf(p, "[PCM ERROR]\n");
            goto out;       
        default:
            p += sprintf(p, "[PCM NOT RUNNING]\n");
            goto out;
    }

            p += sprintf(p, "CPU WAKE UP BY: ");

    if (wake_status_p->wakeup_event & (WAKE_SRC_PCM_TIMER)) {
        if (!pcm_dbg_reg->SLEEP_CPU_WAKEUP_EVENT) {
            p += sprintf(p, "PCM_TIMER ");
        } else {
            p += sprintf(p, "CPU ");
        }
    }
    
    for(i=1;i<32;i++)
    {
        if(wake_status_p->wakeup_event & (1<<i)){
            p += sprintf(p, "%s ",pcm_wakeup_reason[i]);
            if(i==WAKE_ID_EINT)
            	p += sprintf(p, ":0x%X ",mt_eint_get_status(0));            
        }
    }

out:
    len = p - buff;
    BUG_ON(len > SPM_PARSE_BUFF_LEN);
 
    return len;
}

wake_status_t* spm_get_last_wakeup_status(void)
{
	return 	pcm_config_curr->last_wakesta ;
}

wake_status_t* spm_get_wakeup_status(SPM_PCM_CONFIG* pcm_config)
{
    wake_status_t *p_wakesta;
    PCM_DBG_REG  *p_pcm_dbg_reg;
    
    BUG_ON(pcm_config!=pcm_config_curr);
    
    p_wakesta = &pcm_config->wakesta[pcm_config->wakesta_idx];
    pcm_config->last_wakesta = p_wakesta;
    p_pcm_dbg_reg = &p_wakesta->pcm_dbg_reg;
    
    p_pcm_dbg_reg->PCM_CON0 = spm_read(SPM_PCM_CON0);
    p_pcm_dbg_reg->PCM_CON1 = spm_read(SPM_PCM_CON1);
    p_pcm_dbg_reg->PCM_IM_LEN = spm_read(SPM_PCM_IM_LEN);
    p_pcm_dbg_reg->PCM_REG_DATA_INI = spm_read(SPM_PCM_REG_DATA_INI);
    p_pcm_dbg_reg->PCM_EVENT_VECTOR0 = spm_read(SPM_PCM_EVENT_VECTOR0);
    p_pcm_dbg_reg->PCM_TIMER_VAL = spm_read(SPM_PCM_TIMER_VAL);
    p_pcm_dbg_reg->PCM_TIMER_OUT = spm_read(SPM_PCM_TIMER_OUT);
    p_pcm_dbg_reg->PCM_EVENT_REG_STA = spm_read(SPM_PCM_EVENT_REG_STA);
    p_pcm_dbg_reg->AP_STANBY_CON = spm_read(SPM_AP_STANBY_CON);
    p_pcm_dbg_reg->SLEEP_CPU_WAKEUP_EVENT = spm_read(SPM_SLEEP_CPU_WAKEUP_EVENT);
    p_pcm_dbg_reg->SLEEP_ISR_STATUS = spm_read(SPM_SLEEP_ISR_STATUS); 
    p_pcm_dbg_reg->PCM_REG_DATA[0]=spm_read(SPM_PCM_REG0_DATA);    
    p_pcm_dbg_reg->PCM_REG_DATA[1]=spm_read(SPM_PCM_REG1_DATA);
    p_pcm_dbg_reg->PCM_REG_DATA[2]=spm_read(SPM_PCM_REG2_DATA);
    p_pcm_dbg_reg->PCM_REG_DATA[3]=spm_read(SPM_PCM_REG3_DATA);
    p_pcm_dbg_reg->PCM_REG_DATA[4]=spm_read(SPM_PCM_REG4_DATA);
    p_pcm_dbg_reg->PCM_REG_DATA[5]=spm_read(SPM_PCM_REG5_DATA);
    p_pcm_dbg_reg->PCM_REG_DATA[6]=spm_read(SPM_PCM_REG6_DATA);
    p_pcm_dbg_reg->PCM_REG_DATA[7]=spm_read(SPM_PCM_REG7_DATA);    
    p_pcm_dbg_reg->PCM_REG_DATA[8]=spm_read(SPM_PCM_REG8_DATA);
    p_pcm_dbg_reg->PCM_REG_DATA[9]=spm_read(SPM_PCM_REG9_DATA);
    p_pcm_dbg_reg->PCM_REG_DATA[10]=spm_read(SPM_PCM_REG10_DATA);
    p_pcm_dbg_reg->PCM_REG_DATA[11]=spm_read(SPM_PCM_REG11_DATA);
    p_pcm_dbg_reg->PCM_REG_DATA[12]=spm_read(SPM_PCM_REG12_DATA);
    p_pcm_dbg_reg->PCM_REG_DATA[13]=spm_read(SPM_PCM_REG13_DATA);
    p_pcm_dbg_reg->PCM_REG_DATA[14]=spm_read(SPM_PCM_REG14_DATA);
    p_pcm_dbg_reg->PCM_REG_DATA[15]=spm_read(SPM_PCM_REG15_DATA);
    p_pcm_dbg_reg->SPM_CLK_CON_STA = spm_read(SPM_CLK_CON);

    if(p_pcm_dbg_reg->PCM_REG_DATA_INI != 0)
        p_wakesta->wake_reason=WR_PCM_ASSERT;
    else if(p_pcm_dbg_reg->PCM_EVENT_REG_STA & PCM_EVENT_ABORT)
        p_wakesta->wake_reason=WR_PCM_SLEEP_ABORT;
    else if(p_pcm_dbg_reg->PCM_EVENT_REG_STA & PCM_EVENT_NORMAL)
        p_wakesta->wake_reason=WR_WAKE_SRC;
    else
        p_wakesta->wake_reason=WR_ERROR;

    p_wakesta->wakeup_event = p_pcm_dbg_reg->PCM_REG_DATA[12];

    pcm_config->wakesta_idx++;		
    if(pcm_config->wakesta_idx >= SPM_PCM_RECORD_NUM) 
 	pcm_config->wakesta_idx -=SPM_PCM_RECORD_NUM;

    spm_parse_wake_reason(pcm_config,p_wakesta);

    return p_wakesta;
}

const char* spm_get_wake_up_result(SPM_PCM_SCENARIO scenario)
{
    return (char*) pcm_config_arr[scenario]->result;
}

void spm_clean_after_wakeup(void)
{
    unsigned tmp;
    
    /* PCM has cleared uart_clk_off_req and now clear it in POWER_ON_VAL1 */
    spm_write(SPM_POWER_ON_VAL1, spm_read(SPM_POWER_ON_VAL1) & ~R7_UART_CLK_OFF_REQ);

    /* SRCLKENA_PERI: POWER_ON_VAL1|r7 (PWR_IO_EN[7]=1) */
    /* SRCLKENA_MD: POWER_ON_VAL1|r7 (PWR_IO_EN[2]=1) */
    //spm_write(SPM_CLK_CON, spm_read(SPM_CLK_CON) &
    //                       ~(CC_SYSCLK0_EN_1 | CC_SYSCLK0_EN_0 | CC_SRCLKEN0_EN));

    spm_write(SPM_CLK_CON, spm_read(SPM_CLK_CON) &
                           ~(CC_SYSCLK0_EN_1 | CC_SYSCLK0_EN_0 | CC_SRCLKEN0_EN | CC_SRCLKENA_MASK));

    /* re-enable POWER_ON_VAL0/1 to control power */
    spm_write(SPM_PCM_PWR_IO_EN, 0);

    /* unlock INFRA DCM */
    spm_write(SPM_CLK_CON, spm_read(SPM_CLK_CON) & ~CC_LOCK_INFRA_DCM);

    /* clean PCM timer event */
    spm_write(SPM_PCM_CON1, CON1_CFG_KEY | (spm_read(SPM_PCM_CON1) & ~ (CON1_PCM_TIMER_EN|CON1_PCM_WDT_EN)));

    /* clean CPU wakeup event (pause abort) */
    spm_write(SPM_SLEEP_CPU_WAKEUP_EVENT, 0);

    /* clean wakeup event raw status */
    spm_write(SPM_SLEEP_WAKEUP_EVENT_MASK, 0xffffffff);

   // spm_write(SPM_PCM_CON0, 0x0b160000);

    /* clean ISR status */
    spm_write(SPM_SLEEP_ISR_MASK, 0x0008);
    spm_write(SPM_SLEEP_ISR_STATUS, 0x0008);

    if(pcm_config_curr->scenario == SPM_PCM_DEEP_IDLE)
        spm_write(SPM_PCM_SW_INT_CLEAR,0xd);//avoid spm1 irq clear for RGU workaround
    else
    spm_write(SPM_PCM_SW_INT_CLEAR,0xf);
    
    /*[WHQA_00014241]Work Around for CCCI needs to sync register values to flipflop after suspend*/
    tmp = spm_read(0xF0200200);
    spm_write(0xF0200200, 0);
    spm_write(0xF0200200, tmp);
    
    tmp = spm_read(0xF0200204);
    spm_write(0xF0200204, 0);
    spm_write(0xF0200204, tmp);
    
}


void spm_dump_pll_regs(void)
{
    /* PLL register */
    spm_notice("ARMPLL_CON0       0x%X = 0x%X\n",ARMPLL_CON0       ,spm_read(ARMPLL_CON0       ));
    spm_notice("ARMPLL_CON1       0x%X = 0x%X\n",ARMPLL_CON1       ,spm_read(ARMPLL_CON1       ));
    spm_notice("ARMPLL_PWR_CON0   0x%X = 0x%X\n",ARMPLL_PWR_CON0   ,spm_read(ARMPLL_PWR_CON0   ));     
    spm_notice("MAINPLL_CON0      0x%X = 0x%X\n",MAINPLL_CON0      ,spm_read(MAINPLL_CON0      ));  
    spm_notice("MAINPLL_CON1      0x%X = 0x%X\n",MAINPLL_CON1      ,spm_read(MAINPLL_CON1      ));  
    spm_notice("MAINPLL_PWR_CON0  0x%X = 0x%X\n",MAINPLL_PWR_CON0  ,spm_read(MAINPLL_PWR_CON0  ));      
    spm_notice("UNIVPLL_CON0      0x%X = 0x%X\n",UNIVPLL_CON0      ,spm_read(UNIVPLL_CON0      ));  
    spm_notice("UNIVPLL_CON1      0x%X = 0x%X\n",(UNIVPLL_CON0+4)  ,spm_read(UNIVPLL_CON0+4    ));  
    spm_notice("UNIVPLL_PWR_CON0  0x%X = 0x%X\n",UNIVPLL_PWR_CON0  ,spm_read(UNIVPLL_PWR_CON0  ));      
    spm_notice("WHPLL_CON0        0x%X = 0x%X\n",WHPLL_CON0        ,spm_read(WHPLL_CON0        ));
    spm_notice("WHPLL_CON1        0x%X = 0x%X\n",WHPLL_CON1        ,spm_read(WHPLL_CON1        ));
    spm_notice("WHPLL_PWR_CON0    0x%X = 0x%X\n",WHPLL_PWR_CON0    ,spm_read(WHPLL_PWR_CON0    ));    
    spm_notice("WHPLL_PATHSEL_CON 0x%X = 0x%X\n",WHPLL_PATHSEL_CON ,spm_read(WHPLL_PATHSEL_CON ));        
}                                     
                                      
                                      
void spm_pcm_dump_regs(void)          
{                                     
    /* SPM register */
    spm_notice("POWER_ON_VAL0   0x%x = 0x%x\n", SPM_POWER_ON_VAL0          , spm_read(SPM_POWER_ON_VAL0));
    spm_notice("POWER_ON_VAL1   0x%x = 0x%x\n", SPM_POWER_ON_VAL1          , spm_read(SPM_POWER_ON_VAL1));
    spm_notice("PCM_PWR_IO_EN   0x%x = 0x%x\n", SPM_PCM_PWR_IO_EN          , spm_read(SPM_PCM_PWR_IO_EN));
    spm_notice("PCM_REG0_DATA   0x%x = 0x%x\n", SPM_PCM_REG0_DATA          , spm_read(SPM_PCM_REG0_DATA));
    spm_notice("PCM_REG7_DATA   0x%x = 0x%x\n", SPM_PCM_REG7_DATA          , spm_read(SPM_PCM_REG7_DATA));
    spm_notice("PCM_REG13_DATA  0x%x = 0x%x\n", SPM_PCM_REG13_DATA         , spm_read(SPM_PCM_REG13_DATA));
    spm_notice("CLK_CON         0x%x = 0x%x\n", SPM_CLK_CON                , spm_read(SPM_CLK_CON));
    spm_notice("AP_DVFS_CON     0x%x = 0x%x\n", SPM_AP_DVFS_CON_SET        , spm_read(SPM_AP_DVFS_CON_SET));
    spm_notice("PWR_STATUS      0x%x = 0x%x\n", SPM_PWR_STATUS             , spm_read(SPM_PWR_STATUS));
    spm_notice("PWR_STATUS_S    0x%x = 0x%x\n", SPM_PWR_STATUS_S           , spm_read(SPM_PWR_STATUS_S));
    spm_notice("SLEEP_TIMER_STA 0x%x = 0x%x\n", SPM_SLEEP_TIMER_STA        , spm_read(SPM_SLEEP_TIMER_STA));
    spm_notice("WAKE_EVENT_MASK 0x%x = 0x%x\n", SPM_SLEEP_WAKEUP_EVENT_MASK, spm_read(SPM_SLEEP_WAKEUP_EVENT_MASK));

    // PCM register
    spm_notice("PCM_REG0_DATA   0x%x = 0x%x\n", SPM_PCM_REG0_DATA          , spm_read(SPM_PCM_REG0_DATA));
    spm_notice("PCM_REG1_DATA   0x%x = 0x%x\n", SPM_PCM_REG1_DATA          , spm_read(SPM_PCM_REG1_DATA));
    spm_notice("PCM_REG2_DATA   0x%x = 0x%x\n", SPM_PCM_REG2_DATA          , spm_read(SPM_PCM_REG2_DATA));
    spm_notice("PCM_REG3_DATA   0x%x = 0x%x\n", SPM_PCM_REG3_DATA          , spm_read(SPM_PCM_REG3_DATA));
    spm_notice("PCM_REG4_DATA   0x%x = 0x%x\n", SPM_PCM_REG4_DATA          , spm_read(SPM_PCM_REG4_DATA));
    spm_notice("PCM_REG5_DATA   0x%x = 0x%x\n", SPM_PCM_REG5_DATA          , spm_read(SPM_PCM_REG5_DATA));
    spm_notice("PCM_REG6_DATA   0x%x = 0x%x\n", SPM_PCM_REG6_DATA          , spm_read(SPM_PCM_REG6_DATA));
    spm_notice("PCM_REG7_DATA   0x%x = 0x%x\n", SPM_PCM_REG7_DATA          , spm_read(SPM_PCM_REG7_DATA));
    spm_notice("PCM_REG8_DATA   0x%x = 0x%x\n", SPM_PCM_REG8_DATA          , spm_read(SPM_PCM_REG8_DATA));
    spm_notice("PCM_REG9_DATA   0x%x = 0x%x\n", SPM_PCM_REG9_DATA          , spm_read(SPM_PCM_REG9_DATA));
    spm_notice("PCM_REG10_DATA   0x%x = 0x%x\n", SPM_PCM_REG10_DATA          , spm_read(SPM_PCM_REG10_DATA));
    spm_notice("PCM_REG11_DATA   0x%x = 0x%x\n", SPM_PCM_REG11_DATA          , spm_read(SPM_PCM_REG11_DATA));
    spm_notice("PCM_REG12_DATA   0x%x = 0x%x\n", SPM_PCM_REG12_DATA          , spm_read(SPM_PCM_REG12_DATA));
    spm_notice("PCM_REG13_DATA   0x%x = 0x%x\n", SPM_PCM_REG13_DATA          , spm_read(SPM_PCM_REG13_DATA));
    spm_notice("PCM_REG14_DATA   0x%x = 0x%x\n", SPM_PCM_REG14_DATA          , spm_read(SPM_PCM_REG14_DATA));
    spm_notice("PCM_REG15_DATA   0x%x = 0x%x\n", SPM_PCM_REG15_DATA          , spm_read(SPM_PCM_REG15_DATA));    
}

