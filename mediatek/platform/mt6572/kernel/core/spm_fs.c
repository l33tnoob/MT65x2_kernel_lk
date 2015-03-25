#include <linux/proc_fs.h>
#include <mach/mt_spm.h>
#include <mach/mt_idle.h>

extern SPM_PCM_CONFIG pcm_config_suspend;
extern SPM_PCM_CONFIG pcm_config_dpidle;
extern SPM_PCM_CONFIG pcm_config_mcdi;
extern const char *pcm_scenario[SPM_PCM_SCENARIO_NUM];
static SPM_PCM_CONFIG *pcm_fs_curr= &pcm_config_suspend;

bool spm_fs_initialized=0;

u32 pcm_wakesrc_backup[SPM_PCM_SCENARIO_NUM]={0,0,0};

/*************************************************Common Part by Scott***************************************************************/


static bool pcm_sysfs_write(const char *buf,unsigned long count)
{
    char field[30];
    int value;
    
    if(buf[0]=='0' && buf[1]=='x')
        sscanf(buf,"%x%30s",&value,field);
    else
        sscanf(buf,"%d%30s",&value,field);
    
    spm_crit2("field = %s, value=%d\n",field,value);
    
    if(strcmp(field,"cpu_pdn")==0)
    {
        if(pcm_fs_curr->scenario == SPM_PCM_KERNEL_SUSPEND)
        pcm_fs_curr->cpu_pdn=value;
        else if(pcm_fs_curr->scenario == SPM_PCM_DEEP_IDLE)
        {
            if( value == 0 )
                idle_state_en( IDLE_TYPE_DP , value );//dpidle disable
            else
            {
                pcm_fs_curr->cpu_pdn=(value-1);
                idle_state_en( IDLE_TYPE_DP , 1 );//dpidle enable
            }
        }
            
    }
    else if(strcmp(field,"infra_pdn")==0)
        pcm_fs_curr->infra_pdn=value;
    else if(strcmp(field,"timer_val_ms")==0)
        pcm_fs_curr->timer_val_ms=value;
    else if(strcmp(field,"wfi_sel0")==0)
        pcm_fs_curr->wfi_sel[0]=value;
    else if(strcmp(field,"wfi_sel1")==0)
        pcm_fs_curr->wfi_sel[1]=value;
    else if(strcmp(field,"wake_src")==0)
     {
        if(value == 0)
            {
                BUG_ON(pcm_wakesrc_backup[pcm_fs_curr->scenario]!=0);//you need to restore previous mask first
                pcm_wakesrc_backup[pcm_fs_curr->scenario] = spm_wakesrc_mask_all(pcm_fs_curr->scenario);
            }
        else if(value >= 0 && value < 32)
            spm_wakesrc_set(pcm_fs_curr->scenario,value);
        else if(value < 0 && value > -32)
            spm_wakesrc_clear(pcm_fs_curr->scenario,value* -1);
        else if(pcm_wakesrc_backup[pcm_fs_curr->scenario]!=0)
            {
                spm_wakesrc_restore(pcm_fs_curr->scenario,pcm_wakesrc_backup[pcm_fs_curr->scenario]);
                pcm_wakesrc_backup[pcm_fs_curr->scenario]=0;
            }
        
     }
     else if(strcmp(field,"md_mask")==0)
     	{
     		if(value >= 0 && value<=3)
     			pcm_fs_curr->md_mask=value;
     	}
     else if(strcmp(field,"reserved")==0)
        pcm_fs_curr->reserved=value;
     else
     {
        return false;//Can't not find field       
     }
  return true; 
}

static int pcm_sysfs_read(char *buf)
{
    int len = 0;
    char *p = buf;

    if(pcm_fs_curr->scenario == SPM_PCM_KERNEL_SUSPEND)
        p += sprintf(p, "pcm_config_suspend={\n");
    else if(pcm_fs_curr->scenario == SPM_PCM_DEEP_IDLE)
        p += sprintf(p, "pcm_config_dpidle={\n");
    else if(pcm_fs_curr->scenario == SPM_PCM_MCDI)
        p += sprintf(p, "pcm_config_mcdi={\n");
    else
        {
            p += sprintf(p, "SCENARIO ERROR\n");
            goto error;
        }
    
    	p += sprintf(p,"     .scenario = %d\n",pcm_fs_curr->scenario);	
	    p += sprintf(p,"     .spm_turn_off_26m = %d\n",pcm_fs_curr->spm_turn_off_26m);
		p += sprintf(p,"     .pcm_pwrlevel = %d\n",pcm_fs_curr->pcm_pwrlevel);
		p += sprintf(p,"     .pcm_firmware_addr = 0x%X\n",pcm_fs_curr->pcm_firmware_addr);
		p += sprintf(p,"     .pcm_firmware_len = %d\n",pcm_fs_curr->pcm_firmware_len);
		p += sprintf(p,"     .spm_request_uart_sleep = %d \n",pcm_fs_curr->spm_request_uart_sleep);
      //p += sprintf(p,"     .pcm_vsr[SPM_PCM_MAX_VSR_NUM];
  	    p += sprintf(p,"     .md_mask = 0x%X  /*{MDCONN_MASK=0,CONN_MASK,MD_MASK,MDCONN_UNMASK}*/\n",pcm_fs_curr->md_mask);
        p += sprintf(p,"     .mm_mask = 0x%X  /*{MMALL_MASK=0,MFG_MASK,DISP_MASK,MMALL_UNMASK}*/ \n",pcm_fs_curr->mm_mask);
        p += sprintf(p,"     .wfi_scu_mask = %d\n ",pcm_fs_curr->wfi_scu_mask);
		p += sprintf(p,"     .wfi_l2c_mask = %d\n ",pcm_fs_curr->wfi_l2c_mask);
		p += sprintf(p,"     .wfi_op = %d    /*{REDUCE_OR=0,REDUCE_AND}*/\n",pcm_fs_curr->wfi_op);
		p += sprintf(p,"     .wfi_sel[]={%d,%d}\n",pcm_fs_curr->wfi_sel[0],pcm_fs_curr->wfi_sel[1]);
		p += sprintf(p,"     .timer_val_ms=%d\n",(u32)pcm_fs_curr->timer_val_ms);
		p += sprintf(p,"     .wake_src= 0x%X\n",pcm_fs_curr->wake_src);
		p += sprintf(p,"     .cpu_pdn= %d\n",pcm_fs_curr->cpu_pdn);
		p += sprintf(p,"     .infra_pdn = %d\n",pcm_fs_curr->infra_pdn);        
		p += sprintf(p,"     .reserved = 0x%X\n",pcm_fs_curr->reserved);  
        
    p += sprintf(p, "}\n");
error:
    len = p - buf;
    return len;
}

/*************************************************Suspend by Scott***************************************************************/


/* Sysfs Implementation  */
static ssize_t pcm_sysfs_write_suspend(struct file *file, const char *buffer, unsigned long count, void *data)
{
    char field[30];
    int value;
 
    pcm_fs_curr = &pcm_config_suspend;
    
    if(!pcm_sysfs_write(buffer,count))
    {
        /* No common field found in common, Try specific ones!! */
        if(buffer[0]=='0' && buffer[1]=='x')
            sscanf(buffer,"%x%30s",&value,field);
        else
            sscanf(buffer,"%d%30s",&value,field);
        
        if(strcmp(field,"fgauge")==0)
         {
            pcm_fs_curr->reserved &= (~(1U));
            pcm_fs_curr->reserved |= value;
         }
    }
        
    
    return count;
}

static int pcm_sysfs_read_suspend(char *buf, char **start, off_t off, int count, int *eof, void *data)
{   
    pcm_fs_curr = &pcm_config_suspend;
    
    return pcm_sysfs_read(buf);
}

static int pcm_sysfs_read_suspend_mode(char *buf, char **start, off_t off, int count, int *eof, void *data)
{   
    int len = 0;
    char *p = buf;
    pcm_fs_curr = &pcm_config_suspend;
    
    if(pcm_fs_curr->infra_pdn == 0 && pcm_fs_curr->infra_pdn == 0)
       p += sprintf(p, "0");
    else if(pcm_fs_curr->infra_pdn == 1 && pcm_fs_curr->infra_pdn == 1)
       p += sprintf(p, "1");
    else
        {BUG();}
           
    len = p - buf;
    
    return len;
}

static int pcm_sysfs_read_suspend_timer(char *buf, char **start, off_t off, int count, int *eof, void *data)
{   
    int len = 0;
    char *p = buf;
    pcm_fs_curr = &pcm_config_suspend;
    
    if(pcm_fs_curr->reserved & SPM_SUSPEND_GET_FGUAGE)
       p += sprintf(p, "1 ");
    else  
       p += sprintf(p, "0 ");
    
    p += sprintf(p, "%d",(u32)pcm_fs_curr->timer_val_ms);
              
    len = p - buf;
    
    return len;
}

/*************************************************dpidle & mcdi by Mark***************************************************************/

static ssize_t pcm_sysfs_write_dpidle(struct file *file, const char *buffer, unsigned long count, void *data)
{

    char field[30];
    int value;

    pcm_fs_curr = &pcm_config_dpidle;
    

    if(!pcm_sysfs_write(buffer,count))
    {
        /* No common field found in common, Try specific ones!! */
        if(buffer[0]=='0' || buffer[0]=='x')
            sscanf(buffer,"%x%30s",&value,field);
        else
            sscanf(buffer,"%d%30s",&value,field);
        
        if(strcmp(field,"pwrlevel")==0)   
            pcm_fs_curr->pcm_pwrlevel=( 1 << value );

    }
   
    return count;
}

static int pcm_sysfs_read_dpidle(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
    pcm_fs_curr = &pcm_config_dpidle;
    
    return pcm_sysfs_read(buf);   
}

static int pcm_sysfs_read_dpidle_mode(char *buf, char **start, off_t off, int count, int *eof, void *data)
{   
    int len = 0;
    char *p = buf;
    pcm_fs_curr = &pcm_config_dpidle;

    if(idle_state_get(IDLE_TYPE_DP)==0)//disable Dpidle
       p += sprintf(p, "0");
    else if( pcm_fs_curr->cpu_pdn == 0 )//Legacy Mode
       p += sprintf(p, "1");
    else if(pcm_fs_curr->cpu_pdn == 1 )//CPU dormant
       p += sprintf(p, "2");
    else
        {BUG();}
           
    len = p - buf;
    
    return len;
}

static int pcm_sysfs_read_dpidle_level(char *buf, char **start, off_t off, int count, int *eof, void *data)
{   
    int len = 0;
    char *p = buf;
    pcm_fs_curr = &pcm_config_dpidle;

    if(pcm_fs_curr->pcm_pwrlevel == PWR_LV0)
       p += sprintf(p, "0");
    else if(pcm_fs_curr->pcm_pwrlevel == PWR_LV1)
       p += sprintf(p, "1");
    else
        {BUG();}
           
    len = p - buf;
    
    return len;
}

static int pcm_sysfs_read_dpidle_timer(char *buf, char **start, off_t off, int count, int *eof, void *data)
{   
    int len = 0;
    char *p = buf;
    pcm_fs_curr = &pcm_config_dpidle;
    
    p += sprintf(p, "%d",(u32)pcm_fs_curr->timer_val_ms);
       
    len = p - buf;
    
    return len;
}


static ssize_t pcm_sysfs_write_mcdi(struct file *file, const char *buffer, unsigned long count, void *data)
{
    char field[30];
    int value;

    pcm_fs_curr = &pcm_config_mcdi;
    

    if(!pcm_sysfs_write(buffer,count))
    {
        /* No common field found in common, Try specific ones!! */
        if(buffer[0]=='0' || buffer[0]=='x')
            sscanf(buffer,"%x%30s",&value,field);
        else
            sscanf(buffer,"%d%30s",&value,field);
        
        if(strcmp(field,"mcdi_mode")==0)
        {
            if( value == 0 )
                idle_state_en( IDLE_TYPE_MC , value );//Disable NCDI
            else
            {
                if( value == 1 )
                    spm_disable_sodi_user();
                if( value == 2 )
                    spm_enable_sodi_user();            
                idle_state_en( IDLE_TYPE_MC , 1 );//MCDI handler
            }            
        }  


    }    

    
    return count;
}

static int pcm_sysfs_read_mcdi(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
    pcm_fs_curr = &pcm_config_mcdi;
    
    pcm_sysfs_read(buf);

    return pcm_sysfs_read(buf);    
}

static int pcm_sysfs_read_mcdi_mode(char *buf, char **start, off_t off, int count, int *eof, void *data)
{   
    int len = 0;
    char *p = buf;
    pcm_fs_curr = &pcm_config_mcdi;

    if(idle_state_get(IDLE_TYPE_MC)==0)//MCDI/SODI disable
       p += sprintf(p, "0");
    else if( spm_is_sodi_user_en() == false )//MCDI Only
       p += sprintf(p, "1");
    else 
       p += sprintf(p, "2");
           
    len = p - buf;
    
    return len;
}

static int pcm_sysfs_read_mcdi_timer(char *buf, char **start, off_t off, int count, int *eof, void *data)
{   
    int len = 0;
    char *p = buf;
    pcm_fs_curr = &pcm_config_mcdi;
    
    p += sprintf(p, "%d",(u32)pcm_fs_curr->timer_val_ms);
       
    len = p - buf;
    
    return len;
}


void spm_fs_init(void)
{
    struct proc_dir_entry *spm_fs_file = NULL;
    struct proc_dir_entry *spm_fs_dir = NULL;
    
    spm_fs_dir = proc_mkdir("spm_fs", NULL);
    if (!spm_fs_dir)
    {
        //clc_notice("[%s]: mkdir /proc/mcdi failed\n", __FUNCTION__);
    }
    else
    {
 /****************************************Suspend**********************************************************/
        spm_fs_file = create_proc_entry("suspend", S_IRUGO | S_IWUSR | S_IWGRP, spm_fs_dir);
        if (spm_fs_file)
        {
            spm_fs_file->read_proc = pcm_sysfs_read_suspend;
            spm_fs_file->write_proc = pcm_sysfs_write_suspend;
        }
 
        spm_fs_file = create_proc_entry("suspend_mode", S_IRUGO | S_IWUSR | S_IWGRP, spm_fs_dir);
        if (spm_fs_file)
        {
            spm_fs_file->read_proc = pcm_sysfs_read_suspend_mode;
        }
        spm_fs_file = create_proc_entry("suspend_timer", S_IRUGO | S_IWUSR | S_IWGRP, spm_fs_dir);
        if (spm_fs_file)
        {
            spm_fs_file->read_proc = pcm_sysfs_read_suspend_timer;
        } 
  /****************************************Dpidle**********************************************************/       
        spm_fs_file = create_proc_entry("dpidle", S_IRUGO | S_IWUSR | S_IWGRP, spm_fs_dir);
        if (spm_fs_file)
        {
            spm_fs_file->read_proc = pcm_sysfs_read_dpidle;
            spm_fs_file->write_proc = pcm_sysfs_write_dpidle;
        }
        spm_fs_file = create_proc_entry("dpidle_mode", S_IRUGO | S_IWUSR | S_IWGRP, spm_fs_dir);
        if (spm_fs_file)
        {
            spm_fs_file->read_proc = pcm_sysfs_read_dpidle_mode;            
        }
        spm_fs_file = create_proc_entry("dpidle_level", S_IRUGO | S_IWUSR | S_IWGRP, spm_fs_dir);
        if (spm_fs_file)
        {
            spm_fs_file->read_proc = pcm_sysfs_read_dpidle_level;            
        }          
        spm_fs_file = create_proc_entry("dpidle_timer", S_IRUGO | S_IWUSR | S_IWGRP, spm_fs_dir);
        if (spm_fs_file)
        {
            spm_fs_file->read_proc = pcm_sysfs_read_dpidle_timer;            
        }         
  /****************************************MCDI**********************************************************/         
        spm_fs_file = create_proc_entry("mcdi", S_IRUGO | S_IWUSR | S_IWGRP, spm_fs_dir);
        if (spm_fs_file)
        {
            spm_fs_file->read_proc = pcm_sysfs_read_mcdi;
            spm_fs_file->write_proc = pcm_sysfs_write_mcdi;
        }
        spm_fs_file = create_proc_entry("mcdi_mode", S_IRUGO | S_IWUSR | S_IWGRP, spm_fs_dir);
        if (spm_fs_file)
        {
            spm_fs_file->read_proc = pcm_sysfs_read_mcdi_mode;            
        }
        spm_fs_file = create_proc_entry("mcdi_timer", S_IRUGO | S_IWUSR | S_IWGRP, spm_fs_dir);
        if (spm_fs_file)
        {
            spm_fs_file->read_proc = pcm_sysfs_read_mcdi_timer;            
        }  

        
    }

}

