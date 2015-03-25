#include <platform/mt_typedefs.h>
#include <platform/mt_reg_base.h>
#include <platform/mt_i2c.h>     
#include <platform/fan5405.h>
#include <platform/mt_gpio.h>
#include <printf.h>

int g_fan5405_log_en=0;

/**********************************************************
  *
  *   [I2C Slave Setting] 
  *
  *********************************************************/
#define fan5405_SLAVE_ADDR_WRITE   0xD4
#define fan5405_SLAVE_ADDR_Read    0xD5

/**********************************************************
  *
  *   [Global Variable] 
  *
  *********************************************************/
#define fan5405_REG_NUM 7  
kal_uint8 fan5405_reg[fan5405_REG_NUM] = {0};

#define FAN5405_I2C_ID	I2C1
static struct mt_i2c_t fan5405_i2c;

/**********************************************************
  *
  *   [I2C Function For Read/Write fan5405] 
  *
  *********************************************************/
U32 fan5405_i2c_write (U8 chip, U8 *cmdBuffer, int cmdBufferLen, U8 *dataBuffer, int dataBufferLen)
{
    U32 ret_code = I2C_OK;
    U8 write_data[I2C_FIFO_SIZE];
    int transfer_len = cmdBufferLen + dataBufferLen;
    int i=0, cmdIndex=0, dataIndex=0;

    if(I2C_FIFO_SIZE < (cmdBufferLen + dataBufferLen))
    {
        printf("[fan5405_i2c_write] exceed I2C FIFO length!! \n");
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
        //printf("[fan5405_i2c_write] write_data[%d]=%x\n", i, write_data[i]);
    }

    ret_code = mt_i2c_write(I2C1, chip, write_data, transfer_len,1);

    //printf("[fan5405_i2c_write] Done\n");
}

U32 fan5405_i2c_read (U8 chip, U8 *cmdBuffer, int cmdBufferLen, U8 *dataBuffer, int dataBufferLen)
{
    U32 ret_code = I2C_OK;

    ret_code = mt_i2c_write(I2C1, chip, cmdBuffer, cmdBufferLen,1);    // set register command
    if (ret_code != I2C_OK)
        return ret_code;

    ret_code = mt_i2c_read(I2C1, chip, dataBuffer, dataBufferLen,1);

    //printf("[fan5405_i2c_read] Done\n");

    return ret_code;
}

/**********************************************************
  *
  *   [Read / Write Function] 
  *
  *********************************************************/
kal_uint32 fan5405_read_interface (kal_uint8 RegNum, kal_uint8 *val, kal_uint8 MASK, kal_uint8 SHIFT)
{
     U8 chip_slave_address = fan5405_SLAVE_ADDR_WRITE;
    U8 cmd = 0x0;
    int cmd_len = 1;
    U8 data = 0xFF;
    int data_len = 1;
    U32 result_tmp;

    printf("--------------------------------------------------\n");

    cmd = RegNum;
    result_tmp = fan5405_i2c_read(chip_slave_address, &cmd, cmd_len, &data, data_len);

    printf("[fan5405_read_interface] Reg[%x]=0x%x\n", RegNum, data);
    
    data &= (MASK << SHIFT);
    *val = (data >> SHIFT);
    
    printf("[fan5405_read_interface] val=0x%x\n", *val);

    if(g_fan5405_log_en>1)        
        printf("%d\n", result_tmp);

    return 1;
}

kal_uint32 fan5405_config_interface (kal_uint8 RegNum, kal_uint8 val, kal_uint8 MASK, kal_uint8 SHIFT)
{
   U8 chip_slave_address = fan5405_SLAVE_ADDR_WRITE;
    U8 cmd = 0x0;
    int cmd_len = 1;
    U8 data = 0xFF;
    int data_len = 1;
    U32 result_tmp;

    printf("--------------------------------------------------\n");

    cmd = RegNum;
    result_tmp = fan5405_i2c_read(chip_slave_address, &cmd, cmd_len, &data, data_len);
    printf("[fan5405_config_interface] Reg[%x]=0x%x\n", RegNum, data);

    data &= ~(MASK << SHIFT);
    data |= (val << SHIFT);

    result_tmp = fan5405_i2c_write(chip_slave_address, &cmd, cmd_len, &data, data_len);
    printf("[fan5405_config_interface] write Reg[%x]=0x%x\n", RegNum, data);

    // Check
    result_tmp = fan5405_i2c_read(chip_slave_address, &cmd, cmd_len, &data, data_len);
    printf("[fan5405_config_interface] Check Reg[%x]=0x%x\n", RegNum, data);

    if(g_fan5405_log_en>1)        
        printf("%d\n", result_tmp);
    
    return 1;
}

/**********************************************************
  *
  *   [fan5405 Function] 
  *
  *********************************************************/
//CON0----------------------------------------------------

void fan5405_set_tmr_rst(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=fan5405_config_interface(   (kal_uint8)(fan5405_CON0), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON0_TMR_RST_MASK),
                                    (kal_uint8)(CON0_TMR_RST_SHIFT)
                                    );
    if(g_fan5405_log_en>1)        
        printf("%d\n", ret);
}

kal_uint32 fan5405_get_otg_status(void)
{
    kal_uint32 ret=0;
    kal_uint8 val=0;

    ret=fan5405_read_interface(     (kal_uint8)(fan5405_CON0), 
                                    (&val),
                                    (kal_uint8)(CON0_OTG_MASK),
                                    (kal_uint8)(CON0_OTG_SHIFT)
                                    );
    if(g_fan5405_log_en>1)        
        printf("%d\n", ret);
    
    return val;
}

void fan5405_set_en_stat(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=fan5405_config_interface(   (kal_uint8)(fan5405_CON0), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON0_EN_STAT_MASK),
                                    (kal_uint8)(CON0_EN_STAT_SHIFT)
                                    );
    if(g_fan5405_log_en>1)        
            printf("%d\n", ret);    
}

kal_uint32 fan5405_get_chip_status(void)
{
    kal_uint32 ret=0;
    kal_uint8 val=0;

    ret=fan5405_read_interface(     (kal_uint8)(fan5405_CON0), 
                                    (&val),
                                    (kal_uint8)(CON0_STAT_MASK),
                                    (kal_uint8)(CON0_STAT_SHIFT)
                                    );
    if(g_fan5405_log_en>1)        
        printf("%d\n", ret);
    
    return val;
}

kal_uint32 fan5405_get_boost_status(void)
{
    kal_uint32 ret=0;
    kal_uint8 val=0;

    ret=fan5405_read_interface(     (kal_uint8)(fan5405_CON0), 
                                    (&val),
                                    (kal_uint8)(CON0_BOOST_MASK),
                                    (kal_uint8)(CON0_BOOST_SHIFT)
                                    );
    if(g_fan5405_log_en>1)        
        printf("%d\n", ret);
    
    return val;
}

kal_uint32 fan5405_get_fault_status(void)
{
    kal_uint32 ret=0;
    kal_uint8 val=0;

    ret=fan5405_read_interface(     (kal_uint8)(fan5405_CON0), 
                                    (&val),
                                    (kal_uint8)(CON0_FAULT_MASK),
                                    (kal_uint8)(CON0_FAULT_SHIFT)
                                    );
    if(g_fan5405_log_en>1)        
        printf("%d\n", ret);
    
    return val;
}

//CON1----------------------------------------------------

void fan5405_set_input_charging_current(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=fan5405_config_interface(   (kal_uint8)(fan5405_CON1), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON1_LIN_LIMIT_MASK),
                                    (kal_uint8)(CON1_LIN_LIMIT_SHIFT)
                                    );
    if(g_fan5405_log_en>1)        
        printf("%d\n", ret);    
}

void fan5405_set_v_low(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=fan5405_config_interface(   (kal_uint8)(fan5405_CON1), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON1_LOW_V_MASK),
                                    (kal_uint8)(CON1_LOW_V_SHIFT)
                                    );
    if(g_fan5405_log_en>1)        
        printf("%d\n", ret);
}

void fan5405_set_te(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=fan5405_config_interface(   (kal_uint8)(fan5405_CON1), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON1_TE_MASK),
                                    (kal_uint8)(CON1_TE_SHIFT)
                                    );
    if(g_fan5405_log_en>1)        
        printf("%d\n", ret);
}

void fan5405_set_ce(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=fan5405_config_interface(   (kal_uint8)(fan5405_CON1), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON1_CE_MASK),
                                    (kal_uint8)(CON1_CE_SHIFT)
                                    );
    if(g_fan5405_log_en>1)        
        printf("%d\n", ret);
}

void fan5405_set_hz_mode(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=fan5405_config_interface(   (kal_uint8)(fan5405_CON1), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON1_HZ_MODE_MASK),
                                    (kal_uint8)(CON1_HZ_MODE_SHIFT)
                                    );
    if(g_fan5405_log_en>1)        
        printf("%d\n", ret);
}

void fan5405_set_opa_mode(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=fan5405_config_interface(   (kal_uint8)(fan5405_CON1), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON1_OPA_MODE_MASK),
                                    (kal_uint8)(CON1_OPA_MODE_SHIFT)
                                    );
    if(g_fan5405_log_en>1)        
        printf("%d\n", ret);
}

//CON2----------------------------------------------------

void fan5405_set_oreg(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=fan5405_config_interface(   (kal_uint8)(fan5405_CON2), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON2_OREG_MASK),
                                    (kal_uint8)(CON2_OREG_SHIFT)
                                    );
    if(g_fan5405_log_en>1)        
        printf("%d\n", ret);
}

void fan5405_set_otg_pl(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=fan5405_config_interface(   (kal_uint8)(fan5405_CON2), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON2_OTG_PL_MASK),
                                    (kal_uint8)(CON2_OTG_PL_SHIFT)
                                    );
    if(g_fan5405_log_en>1)        
        printf("%d\n", ret);
}

void fan5405_set_otg_en(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=fan5405_config_interface(   (kal_uint8)(fan5405_CON2), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON2_OTG_EN_MASK),
                                    (kal_uint8)(CON2_OTG_EN_SHIFT)
                                    );
    if(g_fan5405_log_en>1)        
        printf("%d\n", ret);
}

//CON3----------------------------------------------------

kal_uint32 fan5405_get_vender_code(void)
{
    kal_uint32 ret=0;
    kal_uint8 val=0;

    ret=fan5405_read_interface(     (kal_uint8)(fan5405_CON3), 
                                    (&val),
                                    (kal_uint8)(CON3_VENDER_CODE_MASK),
                                    (kal_uint8)(CON3_VENDER_CODE_SHIFT)
                                    );
    if(g_fan5405_log_en>1)        
        printf("%d\n", ret);
    
    return val;
}

kal_uint32 fan5405_get_pn(void)
{
    kal_uint32 ret=0;
    kal_uint8 val=0;

    ret=fan5405_read_interface(     (kal_uint8)(fan5405_CON3), 
                                    (&val),
                                    (kal_uint8)(CON3_PIN_MASK),
                                    (kal_uint8)(CON3_PIN_SHIFT)
                                    );
    if(g_fan5405_log_en>1)        
        printf("%d\n", ret);
    
    return val;
}

kal_uint32 fan5405_get_revision(void)
{
    kal_uint32 ret=0;
    kal_uint8 val=0;

    ret=fan5405_read_interface(     (kal_uint8)(fan5405_CON3), 
                                    (&val),
                                    (kal_uint8)(CON3_REVISION_MASK),
                                    (kal_uint8)(CON3_REVISION_SHIFT)
                                    );
    if(g_fan5405_log_en>1)        
        printf("%d\n", ret);
    
    return val;
}

//CON4----------------------------------------------------

void fan5405_set_reset(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=fan5405_config_interface(   (kal_uint8)(fan5405_CON4), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON4_RESET_MASK),
                                    (kal_uint8)(CON4_RESET_SHIFT)
                                    );
    if(g_fan5405_log_en>1)        
        printf("%d\n", ret);    
}

void fan5405_set_iocharge(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=fan5405_config_interface(   (kal_uint8)(fan5405_CON4), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON4_I_CHR_MASK),
                                    (kal_uint8)(CON4_I_CHR_SHIFT)
                                    );
    if(g_fan5405_log_en>1)        
        printf("%d\n", ret);
}

void fan5405_set_iterm(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=fan5405_config_interface(   (kal_uint8)(fan5405_CON4), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON4_I_TERM_MASK),
                                    (kal_uint8)(CON4_I_TERM_SHIFT)
                                    );
    if(g_fan5405_log_en>1)        
        printf("%d\n", ret);
}

//CON5----------------------------------------------------

void fan5405_set_dis_vreg(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=fan5405_config_interface(   (kal_uint8)(fan5405_CON5), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON5_DIS_VREG_MASK),
                                    (kal_uint8)(CON5_DIS_VREG_SHIFT)
                                    );
    if(g_fan5405_log_en>1)        
        printf("%d\n", ret);
}

void fan5405_set_io_level(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=fan5405_config_interface(   (kal_uint8)(fan5405_CON5), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON5_IO_LEVEL_MASK),
                                    (kal_uint8)(CON5_IO_LEVEL_SHIFT)
                                    );
    if(g_fan5405_log_en>1)        
        printf("%d\n", ret);
}

kal_uint32 fan5405_get_sp_status(void)
{
    kal_uint32 ret=0;
    kal_uint8 val=0;

    ret=fan5405_read_interface(     (kal_uint8)(fan5405_CON5), 
                                    (&val),
                                    (kal_uint8)(CON5_SP_STATUS_MASK),
                                    (kal_uint8)(CON5_SP_STATUS_SHIFT)
                                    );
    if(g_fan5405_log_en>1)        
        printf("%d\n", ret);
    
    return val;
}

kal_uint32 fan5405_get_en_level(void)
{
    kal_uint32 ret=0;
    kal_uint8 val=0;

    ret=fan5405_read_interface(     (kal_uint8)(fan5405_CON5), 
                                    (&val),
                                    (kal_uint8)(CON5_EN_LEVEL_MASK),
                                    (kal_uint8)(CON5_EN_LEVEL_SHIFT)
                                    );
    if(g_fan5405_log_en>1)        
        printf("%d\n", ret);
    
    return val;
}

void fan5405_set_vsp(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=fan5405_config_interface(   (kal_uint8)(fan5405_CON5), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON5_VSP_MASK),
                                    (kal_uint8)(CON5_VSP_SHIFT)
                                    );
    if(g_fan5405_log_en>1)        
        printf("%d\n", ret);
}

//CON6----------------------------------------------------

void fan5405_set_i_safe(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=fan5405_config_interface(   (kal_uint8)(fan5405_CON6), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON6_ISAFE_MASK),
                                    (kal_uint8)(CON6_ISAFE_SHIFT)
                                    );
    if(g_fan5405_log_en>1)        
        printf("%d\n", ret);
}

void fan5405_set_v_safe(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=fan5405_config_interface(   (kal_uint8)(fan5405_CON6), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON6_VSAFE_MASK),
                                    (kal_uint8)(CON6_VSAFE_SHIFT)
                                    );
    if(g_fan5405_log_en>1)        
        printf("%d\n", ret);
}

/**********************************************************
  *
  *   [Internal Function] 
  *
  *********************************************************/
//debug liao
unsigned int fan5405_config_interface_liao (unsigned char RegNum, unsigned char val)
{
    kal_uint32 ret=0;
    kal_uint8 ret_val=0;

    printf("--------------------------------------------------\n");
    
    //fan5405_read_byte(RegNum, &fan5405_reg);
    //ret=fan5405_read_interface(RegNum,&val,0xFF,0x0);
    //printk("[liao fan5405_config_interface] Reg[%x]=0x%x\n", RegNum, val);
    
    //fan5405_write_byte(RegNum, val);
    ret=fan5405_config_interface(RegNum,val,0xFF,0x0);
    printf("[liao fan5405_config_interface] write Reg[%x]=0x%x\n", RegNum, val);

    // Check
    //fan5405_read_byte(RegNum, &fan5405_reg);
    ret=fan5405_read_interface(RegNum,&ret_val,0xFF,0x0);
    printf("[liao fan5405_config_interface] Check Reg[%x]=0x%x\n", RegNum, ret_val);

    if(g_fan5405_log_en>1)        
        printf("%d\n", ret);

    return 1;
}

void fan5405_dump_register(void)
{
    U8 chip_slave_address = fan5405_SLAVE_ADDR_WRITE;
    U8 cmd = 0x0;
    int cmd_len = 1;
    U8 data = 0xFF;
    int data_len = 1;
    int i=0;
    for (i=0;i<fan5405_REG_NUM;i++)
    {
        cmd = i;
        fan5405_i2c_read(chip_slave_address, &cmd, cmd_len, &data, data_len);
        fan5405_reg[i] = data;
        printf("[fan5405_dump_register] Reg[0x%X]=0x%X\n", i, fan5405_reg[i]);
    }
}

void fan5405_read_register(int i)
{
    U8 chip_slave_address = fan5405_SLAVE_ADDR_WRITE;
    U8 cmd = 0x0;
    int cmd_len = 1;
    U8 data = 0xFF;
    int data_len = 1;

    cmd = i;
    fan5405_i2c_read(chip_slave_address, &cmd, cmd_len, &data, data_len);
    fan5405_reg[i] = data;
    printf("[fan5405_read_register] Reg[0x%X]=0x%X\n", i, fan5405_reg[i]);
}

#if 1
#include <cust_gpio_usage.h>
int gpio_number   = GPIO_SWCHARGER_EN_PIN; 
int gpio_off_mode = GPIO_SWCHARGER_EN_PIN_M_GPIO;
int gpio_on_mode  = GPIO_SWCHARGER_EN_PIN_M_GPIO;
#else
int gpio_number   = 56; 
int gpio_off_mode = 0;
int gpio_on_mode  = 0;
#endif
int gpio_off_dir  = GPIO_DIR_OUT;
int gpio_off_out  = GPIO_OUT_ONE;
int gpio_on_dir   = GPIO_DIR_OUT;
int gpio_on_out   = GPIO_OUT_ZERO;

void fan5405_turn_on_charging(void)
{
    mt_set_gpio_mode(gpio_number,gpio_on_mode);  
    mt_set_gpio_dir(gpio_number,gpio_on_dir);
    mt_set_gpio_out(gpio_number,gpio_on_out);

    fan5405_config_interface_liao(0x00,0xC0);
    fan5405_config_interface_liao(0x01,0x78);
    //fan5405_config_interface_liao(0x02,0x8e);
    fan5405_config_interface_liao(0x05,0x04);
    fan5405_config_interface_liao(0x04,0x1A); //146mA
}

void fan5405_hw_init(void)
{
#if defined(HIGH_BATTERY_VOLTAGE_SUPPORT)
    printf("[fan5405_hw_init] (0x06,0x77)\n");
    fan5405_config_interface_liao(0x06,0x77); // set ISAFE and HW CV point (4.34)
    fan5405_config_interface_liao(0x02,0xaa); // 4.34    
#else
    printf("[fan5405_hw_init] (0x06,0x70)\n");
    fan5405_config_interface_liao(0x06,0x70); // set ISAFE
    fan5405_config_interface_liao(0x02,0x8e); // 4.2
#endif    
}

