#include "platform.h"
#include "i2c.h"
#include "fan5405.h"

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
kal_uint32 fan5405_write_byte(kal_uint8 addr, kal_uint8 value)
{
    kal_uint32 ret_code = I2C_OK;
    kal_uint8 write_data[2];
    kal_uint16 len;

    write_data[0]= addr;
    write_data[1] = value;

    fan5405_i2c.id = FAN5405_I2C_ID;
    /* Since i2c will left shift 1 bit, we need to set FAN5405 I2C address to >>1 */
    fan5405_i2c.addr = (fan5405_SLAVE_ADDR_WRITE >> 1);
    fan5405_i2c.mode = ST_MODE;
    fan5405_i2c.speed = 100;
    len = 2;

    ret_code = i2c_write(&fan5405_i2c, write_data, len);
    printf("%s: i2c_write: ret_code: %d\n", __func__, ret_code);

    return ret_code;
}

kal_uint32 fan5405_read_byte (kal_uint8 addr, kal_uint8 *dataBuffer) 
{
    kal_uint32 ret_code = I2C_OK;
    kal_uint16 len;
    *dataBuffer = addr;

    fan5405_i2c.id = FAN5405_I2C_ID;
    /* Since i2c will left shift 1 bit, we need to set FAN5405 I2C address to >>1 */
    fan5405_i2c.addr = (fan5405_SLAVE_ADDR_WRITE >> 1);
    fan5405_i2c.mode = ST_MODE;
    fan5405_i2c.speed = 100;
    len = 1;

    ret_code = i2c_write_read(&fan5405_i2c, dataBuffer, len, len);
    printf("%s: i2c_read: ret_code: %d\n", __func__, ret_code);

    return ret_code;
}

/**********************************************************
  *
  *   [Read / Write Function] 
  *
  *********************************************************/
kal_uint32 fan5405_read_interface (kal_uint8 RegNum, kal_uint8 *val, kal_uint8 MASK, kal_uint8 SHIFT)
{
    kal_uint8 fan5405_reg = 0;
    int ret = 0;
    
    printf("--------------------------------------------------PL\n");

    ret = fan5405_read_byte(RegNum, &fan5405_reg);
    printf("[fan5405_read_interface] Reg[%x]=0x%x\n", RegNum, fan5405_reg);
    
    fan5405_reg &= (MASK << SHIFT);
    *val = (fan5405_reg >> SHIFT);    
    printf("[fan5405_read_interface] val=0x%x\n", *val);

    return ret;
}

kal_uint32 fan5405_config_interface (kal_uint8 RegNum, kal_uint8 val, kal_uint8 MASK, kal_uint8 SHIFT)
{
    kal_uint8 fan5405_reg = 0;
    int ret = 0;

    printf("--------------------------------------------------PL\n");

    ret = fan5405_read_byte(RegNum, &fan5405_reg);
    printf("[fan5405_config_interface] Reg[%x]=0x%x\n", RegNum, fan5405_reg);
    
    fan5405_reg &= ~(MASK << SHIFT);
    fan5405_reg |= (val << SHIFT);

    ret = fan5405_write_byte(RegNum, fan5405_reg);
    printf("[fan5405_config_interface] write Reg[%x]=0x%x\n", RegNum, fan5405_reg);

    // Check
    //fan5405_read_byte(RegNum, &fan5405_reg);
    //printf("[fan5405_config_interface] Check Reg[%x]=0x%x\n", RegNum, fan5405_reg);

    return ret;
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
    int ret = 0;
    
    ret = fan5405_write_byte(RegNum, val);

    if(g_fan5405_log_en>1)        
        printf("%d\n", ret);

    return ret;
}

void fan5405_dump_register(void)
{
    int i=0;

    for (i=0;i<fan5405_REG_NUM;i++)
    {
        fan5405_read_byte(i, &fan5405_reg[i]);
        printf("[0x%x]=0x%x\n", i, fan5405_reg[i]);        
    }
}

void fan5405_read_register(int i)
{
    fan5405_read_byte(i, &fan5405_reg[i]);
    printf("[0x%x]=0x%x\n", i, fan5405_reg[i]); 
}

void fan5405_hw_init(void)
{
#if defined(HIGH_BATTERY_VOLTAGE_SUPPORT)
    printf("[fan5405_hw_init] (0x06,0x77)\n");
    fan5405_config_interface_liao(0x06,0x77); // set ISAFE and HW CV point (4.34)
#else
    printf("[fan5405_hw_init] (0x06,0x70)\n");
    fan5405_config_interface_liao(0x06,0x70); // set ISAFE
#endif    
}

