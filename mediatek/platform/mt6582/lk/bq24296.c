#include <platform/mt_typedefs.h>
#include <platform/mt_reg_base.h>
#include <platform/mt_i2c.h>   
#include <platform/mt_pmic.h>
#include <platform/bq24296.h>
#include <printf.h>

//#include <target/cust_charging.h>

int g_bq24296_log_en=0;
//static struct mt_i2c_t bq24296_i2c;

/**********************************************************
  *
  *   [I2C Slave Setting] 
  *
  *********************************************************/
#define bq24296_SLAVE_ADDR_WRITE   0xD6
#define bq24296_SLAVE_ADDR_Read    0xD7

/**********************************************************
  *
  *   [Global Variable] 
  *
  *********************************************************/
#define bq24296_REG_NUM 11
kal_uint8 bq24296_reg[bq24296_REG_NUM] = {0};

/**********************************************************
  *
  *   [I2C Function For Read/Write bq24296] 
  *
  *********************************************************/
#ifndef BQ24296_BUSNUM
#define BQ24296_BUSNUM 1
#endif  

U32 bq24296_i2c_read (U8 chip, U8 *cmdBuffer, int cmdBufferLen, U8 *dataBuffer, int dataBufferLen)
{
    U32 ret_code = I2C_OK;
    	
    ret_code = mt_i2c_write(BQ24296_BUSNUM, chip, cmdBuffer, cmdBufferLen, 1);	 // set register command
    if (ret_code != I2C_OK)
    return ret_code;
    
    ret_code = mt_i2c_read(BQ24296_BUSNUM, chip, dataBuffer, dataBufferLen, 1);
    	
    //printf("[bq24296_i2c_read] Done\n");
    	
    return ret_code;
}
	
U32 bq24296_i2c_write (U8 chip, U8 *cmdBuffer, int cmdBufferLen, U8 *dataBuffer, int dataBufferLen)
{
    U32 ret_code = I2C_OK;
    U8 write_data[I2C_FIFO_SIZE];
    int transfer_len = cmdBufferLen + dataBufferLen;
    int i=0, cmdIndex=0, dataIndex=0;
    	
    if(I2C_FIFO_SIZE < (cmdBufferLen + dataBufferLen))
    {
        printf("[bq24296_i2c_write] exceed I2C FIFO length!! \n");
        return 0;
    }
    	
    //write_data[0] = cmd;
    //write_data[1] = writeData;
    
    while(cmdIndex < cmdBufferLen)
    {
        write_data[i] = cmdBuffer[cmdIndex];
        cmdIndex++;
        i++;
    }
    
    while(dataIndex < dataBufferLen)
    {
        write_data[i] = dataBuffer[dataIndex];
        dataIndex++;
        i++;
    }
    	
    /* dump write_data for check */
    for( i=0 ; i < transfer_len ; i++ )
    {
        //printf("[bq24296_i2c_write] write_data[%d]=%x\n", i, write_data[i]);
    }
    	
    ret_code = mt_i2c_write(BQ24296_BUSNUM, chip, write_data, transfer_len, 1);
    	
    //printf("[bq24296_i2c_write] Done\n");
	
    return ret_code;
}

/**********************************************************
  *
  *   [Read / Write Function] 
  *
  *********************************************************/
kal_uint32 bq24296_read_interface (kal_uint8 RegNum, kal_uint8 *val, kal_uint8 MASK, kal_uint8 SHIFT)
{
    U8 chip_slave_address = bq24296_SLAVE_ADDR_WRITE;
    U8 cmd = 0x0;
    int cmd_len = 1;
    U8 data = 0xFF;
    int data_len = 1;
    U32 result_tmp;

    //printf("--------------------------------------------------\n");

    cmd = RegNum;
    result_tmp = bq24296_i2c_read(chip_slave_address, &cmd, cmd_len, &data, data_len);

    //printf("[bq24296_read_interface] Reg[%x]=0x%x\n", RegNum, data);
    
    data &= (MASK << SHIFT);
    *val = (data >> SHIFT);
    
    //printf("[bq24296_read_interface] val=0x%x\n", *val);

    if(g_bq24296_log_en>1)        
        printf("%d\n", result_tmp);

    return 1;

}

kal_uint32 bq24296_config_interface (kal_uint8 RegNum, kal_uint8 val, kal_uint8 MASK, kal_uint8 SHIFT)
{
    U8 chip_slave_address = bq24296_SLAVE_ADDR_WRITE;
    U8 cmd = 0x0;
    int cmd_len = 1;
    U8 data = 0xFF;
    int data_len = 1;
    U32 result_tmp;

    //printf("--------------------------------------------------\n");

    cmd = RegNum;
    result_tmp = bq24296_i2c_read(chip_slave_address, &cmd, cmd_len, &data, data_len);
    //printf("[bq24296_config_interface] Reg[%x]=0x%x\n", RegNum, data);

    data &= ~(MASK << SHIFT);
    data |= (val << SHIFT);

    result_tmp = bq24296_i2c_write(chip_slave_address, &cmd, cmd_len, &data, data_len);
    //printf("[bq24296_config_interface] write Reg[%x]=0x%x\n", RegNum, data);

    // Check
    result_tmp = bq24296_i2c_read(chip_slave_address, &cmd, cmd_len, &data, data_len);
    //printf("[bq24296_config_interface] Check Reg[%x]=0x%x\n", RegNum, data);

    if(g_bq24296_log_en>1)        
        printf("%d\n", result_tmp);
    
    return 1;
}

/**********************************************************
  *
  *   [Internal Function] 
  *
  *********************************************************/
//CON0----------------------------------------------------

void bq24296_set_en_hiz(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=bq24296_config_interface(   (kal_uint8)(bq24296_CON0), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON0_EN_HIZ_MASK),
                                    (kal_uint8)(CON0_EN_HIZ_SHIFT)
                                    );
    if(g_bq24296_log_en>1)        
        printf("%d\n", ret);	
}

void bq24296_set_vindpm(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=bq24296_config_interface(   (kal_uint8)(bq24296_CON0), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON0_VINDPM_MASK),
                                    (kal_uint8)(CON0_VINDPM_SHIFT)
                                    );
    if(g_bq24296_log_en>1)        
        printf("%d\n", ret);		
}

void bq24296_set_iinlim(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=bq24296_config_interface(   (kal_uint8)(bq24296_CON0), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON0_IINLIM_MASK),
                                    (kal_uint8)(CON0_IINLIM_SHIFT)
                                    );
    if(g_bq24296_log_en>1)        
        printf("%d\n", ret);		
}

//CON1----------------------------------------------------

void bq24296_set_reg_rst(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=bq24296_config_interface(   (kal_uint8)(bq24296_CON1), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON1_REG_RST_MASK),
                                    (kal_uint8)(CON1_REG_RST_SHIFT)
                                    );
    if(g_bq24296_log_en>1)        
        printf("%d\n", ret);		
}

void bq24296_set_wdt_rst(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=bq24296_config_interface(   (kal_uint8)(bq24296_CON1), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON1_WDT_RST_MASK),
                                    (kal_uint8)(CON1_WDT_RST_SHIFT)
                                    );
    if(g_bq24296_log_en>1)        
        printf("%d\n", ret);		
}

void bq24296_set_chg_config(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=bq24296_config_interface(   (kal_uint8)(bq24296_CON1), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON1_CHG_CONFIG_MASK),
                                    (kal_uint8)(CON1_CHG_CONFIG_SHIFT)
                                    );
    if(g_bq24296_log_en>1)        
        printf("%d\n", ret);		
}

void bq24296_set_sys_min(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=bq24296_config_interface(   (kal_uint8)(bq24296_CON1), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON1_SYS_MIN_MASK),
                                    (kal_uint8)(CON1_SYS_MIN_SHIFT)
                                    );
    if(g_bq24296_log_en>1)        
        printf("%d\n", ret);		
}

void bq24296_set_boost_lim(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=bq24296_config_interface(   (kal_uint8)(bq24296_CON1), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON1_BOOST_LIM_MASK),
                                    (kal_uint8)(CON1_BOOST_LIM_SHIFT)
                                    );
    if(g_bq24296_log_en>1)        
        printf("%d\n", ret);		
}

//CON2----------------------------------------------------

void bq24296_set_ichg(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=bq24296_config_interface(   (kal_uint8)(bq24296_CON2), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON2_ICHG_MASK),
                                    (kal_uint8)(CON2_ICHG_SHIFT)
                                    );
    if(g_bq24296_log_en>1)        
        printf("%d\n", ret);		
}

//CON3----------------------------------------------------

void bq24296_set_iprechg(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=bq24296_config_interface(   (kal_uint8)(bq24296_CON3), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON3_IPRECHG_MASK),
                                    (kal_uint8)(CON3_IPRECHG_SHIFT)
                                    );
    if(g_bq24296_log_en>1)        
        printf("%d\n", ret);		
}

void bq24296_set_iterm(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=bq24296_config_interface(   (kal_uint8)(bq24296_CON3), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON3_ITERM_MASK),
                                    (kal_uint8)(CON3_ITERM_SHIFT)
                                    );
    if(g_bq24296_log_en>1)        
        printf("%d\n", ret);		
}

//CON4----------------------------------------------------

void bq24296_set_vreg(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=bq24296_config_interface(   (kal_uint8)(bq24296_CON4), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON4_VREG_MASK),
                                    (kal_uint8)(CON4_VREG_SHIFT)
                                    );
    if(g_bq24296_log_en>1)        
        printf("%d\n", ret);		
}

void bq24296_set_batlowv(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=bq24296_config_interface(   (kal_uint8)(bq24296_CON4), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON4_BATLOWV_MASK),
                                    (kal_uint8)(CON4_BATLOWV_SHIFT)
                                    );
    if(g_bq24296_log_en>1)        
        printf("%d\n", ret);		
}

void bq24296_set_vrechg(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=bq24296_config_interface(   (kal_uint8)(bq24296_CON4), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON4_VRECHG_MASK),
                                    (kal_uint8)(CON4_VRECHG_SHIFT)
                                    );
    if(g_bq24296_log_en>1)        
        printf("%d\n", ret);		
}

//CON5----------------------------------------------------

void bq24296_set_en_term(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=bq24296_config_interface(   (kal_uint8)(bq24296_CON5), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON5_EN_TERM_MASK),
                                    (kal_uint8)(CON5_EN_TERM_SHIFT)
                                    );
    if(g_bq24296_log_en>1)        
        printf("%d\n", ret);		
}

void bq24296_set_watchdog(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=bq24296_config_interface(   (kal_uint8)(bq24296_CON5), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON5_WATCHDOG_MASK),
                                    (kal_uint8)(CON5_WATCHDOG_SHIFT)
                                    );
    if(g_bq24296_log_en>1)        
        printf("%d\n", ret);		
}

void bq24296_set_en_timer(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=bq24296_config_interface(   (kal_uint8)(bq24296_CON5), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON5_EN_TIMER_MASK),
                                    (kal_uint8)(CON5_EN_TIMER_SHIFT)
                                    );
    if(g_bq24296_log_en>1)        
        printf("%d\n", ret);		
}

void bq24296_set_chg_timer(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=bq24296_config_interface(   (kal_uint8)(bq24296_CON5), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON5_CHG_TIMER_MASK),
                                    (kal_uint8)(CON5_CHG_TIMER_SHIFT)
                                    );
    if(g_bq24296_log_en>1)        
        printf("%d\n", ret);		
}

//CON6----------------------------------------------------

void bq24296_set_treg(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=bq24296_config_interface(   (kal_uint8)(bq24296_CON6), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON6_TREG_MASK),
                                    (kal_uint8)(CON6_TREG_SHIFT)
                                    );
    if(g_bq24296_log_en>1)        
        printf("%d\n", ret);		
}

//CON7----------------------------------------------------

void bq24296_set_tmr2x_en(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=bq24296_config_interface(   (kal_uint8)(bq24296_CON7), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON7_TMR2X_EN_MASK),
                                    (kal_uint8)(CON7_TMR2X_EN_SHIFT)
                                    );
    if(g_bq24296_log_en>1)        
        printf("%d\n", ret);		
}

void bq24296_set_batfet_disable(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=bq24296_config_interface(   (kal_uint8)(bq24296_CON7), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON7_BATFET_Disable_MASK),
                                    (kal_uint8)(CON7_BATFET_Disable_SHIFT)
                                    );
    if(g_bq24296_log_en>1)        
        printf("%d\n", ret);		
}

void bq24296_set_int_mask(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=bq24296_config_interface(   (kal_uint8)(bq24296_CON7), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON7_INT_MASK_MASK),
                                    (kal_uint8)(CON7_INT_MASK_SHIFT)
                                    );
    if(g_bq24296_log_en>1)        
        printf("%d\n", ret);		
}

//CON8----------------------------------------------------

kal_uint32 bq24296_get_system_status(void)
{
    kal_uint32 ret=0;
    kal_uint8 val=0;

    ret=bq24296_read_interface(     (kal_uint8)(bq24296_CON8), 
                                    (&val),
                                    (kal_uint8)(0xFF),
                                    (kal_uint8)(0x0)
                                    );
    if(g_bq24296_log_en>1)        
        printf("%d\n", ret);	
	
    return val;
}

kal_uint32 bq24296_get_vbus_stat(void)
{
    kal_uint32 ret=0;
    kal_uint8 val=0;

    ret=bq24296_read_interface(     (kal_uint8)(bq24296_CON8), 
                                    (&val),
                                    (kal_uint8)(CON8_VBUS_STAT_MASK),
                                    (kal_uint8)(CON8_VBUS_STAT_SHIFT)
                                    );
    if(g_bq24296_log_en>1)        
        printf("%d\n", ret);	
	
    return val;
}

kal_uint32 bq24296_get_chrg_stat(void)
{
    kal_uint32 ret=0;
    kal_uint8 val=0;

    ret=bq24296_read_interface(     (kal_uint8)(bq24296_CON8), 
                                    (&val),
                                    (kal_uint8)(CON8_CHRG_STAT_MASK),
                                    (kal_uint8)(CON8_CHRG_STAT_SHIFT)
                                    );
    if(g_bq24296_log_en>1)        
        printf("%d\n", ret);	
	
    return val;
}

kal_uint32 bq24296_get_vsys_stat(void)
{
    kal_uint32 ret=0;
    kal_uint8 val=0;

    ret=bq24296_read_interface(     (kal_uint8)(bq24296_CON8), 
                                    (&val),
                                    (kal_uint8)(CON8_VSYS_STAT_MASK),
                                    (kal_uint8)(CON8_VSYS_STAT_SHIFT)
                                    );
    if(g_bq24296_log_en>1)        
        printf("%d\n", ret);	
	
    return val;
}

/**********************************************************
  *
  *   [Internal Function] 
  *
  *********************************************************/
void bq24296_dump_register(void)
{
    U8 chip_slave_address = bq24296_SLAVE_ADDR_WRITE;
    U8 cmd = 0x0;
    int cmd_len = 1;
    U8 data = 0xFF;
    int data_len = 1;
    int i=0;
    for (i=0;i<bq24296_REG_NUM;i++)
    {
        cmd = i;
        bq24296_i2c_read(chip_slave_address, &cmd, cmd_len, &data, data_len);
        bq24296_reg[i] = data;
        printf("[bq24296_dump_register] Reg[0x%X]=0x%X\n", i, bq24296_reg[i]);
    }	
}
void bq24296_hw_init(void)
{
    upmu_set_rg_bc11_bb_ctrl(1);    //BC11_BB_CTRL    
    upmu_set_rg_bc11_rst(1);        //BC11_RST

#if 0 //no use
    //pull PSEL low
    mt_set_gpio_mode(GPIO_CHR_PSEL_PIN,GPIO_MODE_GPIO);  
    mt_set_gpio_dir(GPIO_CHR_PSEL_PIN,GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_CHR_PSEL_PIN,GPIO_OUT_ZERO);
#endif    
    
    //pull CE low
#if 0 //If dws default config is high, Need init GPIO_CHR_CE_PIN high first
    mt_set_gpio_mode(GPIO_CHR_CE_PIN,GPIO_MODE_GPIO);  
    mt_set_gpio_dir(GPIO_CHR_CE_PIN,GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_CHR_CE_PIN,GPIO_OUT_ZERO);    
#endif

    bq24296_set_en_hiz(0x0);
    bq24296_set_vindpm(0xA); //VIN DPM check 4.68V
    bq24296_set_reg_rst(0x0);
	bq24296_set_wdt_rst(0x1); //Kick watchdog

    if(0)
	    bq24296_set_sys_min(0x0); //Minimum system voltage 3.0V (MT6320 E1 workaround, disable powerpath)
    else	    
        bq24296_set_sys_min(0x5); //Minimum system voltage 3.5V		
	bq24296_set_iprechg(0x3); //Precharge current 512mA
	bq24296_set_iterm(0x0); //Termination current 128mA

    bq24296_set_batlowv(0x1); //BATLOWV 3.0V
    bq24296_set_vrechg(0x0); //VRECHG 0.1V (4.108V)
    bq24296_set_en_term(0x1); //Enable termination
    bq24296_set_watchdog(0x1); //WDT 40s
    bq24296_set_en_timer(0x0); //Disable charge timer
    bq24296_set_int_mask(0x0); //Disable fault interrupt
}

void bq24296_turn_on_charging(void)
{
    bq24296_set_en_hiz(0x0);	        	
    bq24296_set_chg_config(0x1); // charger enable
    printf("[BATTERY:bq24296] charger enable !\r\n");
}
