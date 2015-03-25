#include <platform/mt_typedefs.h>
#include <platform/mt_reg_base.h>
#include <platform/mt_i2c.h>
#include <platform/bq24158.h>
#include <platform/mt_gpio.h>
#include <printf.h>

int g_bq24158_log_en=0;

/* High battery support */
//#define HIGH_BATTERY_VOLTAGE_SUPPORT

/**********************************************************
  *
  *   [I2C Slave Setting] 
  *
  *********************************************************/
#define bq24158_SLAVE_ADDR_WRITE   0xD4
#define bq24158_SLAVE_ADDR_Read    0xD5

/**********************************************************
  *
  *   [Global Variable] 
  *
  *********************************************************/
#define bq24158_REG_NUM 7  
kal_uint8 bq24158_reg[bq24158_REG_NUM] = {0};

#define BQ24158_I2C_ID	I2C1
static struct mt_i2c_t bq24158_i2c;

/**********************************************************
  *
  *   [I2C Function For Read/Write bq24158] 
  *
  *********************************************************/
kal_uint32 bq24158_write_byte(kal_uint8 addr, kal_uint8 value)
{
    kal_uint32 ret_code = I2C_OK;
    kal_uint8 write_data[2];
    kal_uint16 len;

    write_data[0]= addr;
    write_data[1] = value;

    bq24158_i2c.id = BQ24158_I2C_ID;
    /* Since i2c will left shift 1 bit, we need to set FAN5405 I2C address to >>1 */
    bq24158_i2c.addr = (bq24158_SLAVE_ADDR_WRITE >> 1);
    bq24158_i2c.mode = ST_MODE;
    bq24158_i2c.speed = 100;
    len = 2;

    ret_code = i2c_write(&bq24158_i2c, write_data, len);
    //printf("%s: i2c_write: ret_code: %d\n", __func__, ret_code);

    return ret_code;
}

kal_uint32 bq24158_read_byte (kal_uint8 addr, kal_uint8 *dataBuffer) 
{
    kal_uint32 ret_code = I2C_OK;
    kal_uint16 len;
    *dataBuffer = addr;

    bq24158_i2c.id = BQ24158_I2C_ID;
    /* Since i2c will left shift 1 bit, we need to set FAN5405 I2C address to >>1 */
    bq24158_i2c.addr = (bq24158_SLAVE_ADDR_WRITE >> 1);
    bq24158_i2c.mode = ST_MODE;
    bq24158_i2c.speed = 100;
    len = 1;

    ret_code = i2c_write_read(&bq24158_i2c, dataBuffer, len, len);
    //printf("%s: i2c_read: ret_code: %d\n", __func__, ret_code);

    return ret_code;
}

/**********************************************************
  *
  *   [Read / Write Function] 
  *
  *********************************************************/
kal_uint32 bq24158_read_interface (kal_uint8 RegNum, kal_uint8 *val, kal_uint8 MASK, kal_uint8 SHIFT)
{
    kal_uint8 bq24158_reg = 0;
    int ret = 0;
    
    printf("--------------------------------------------------LK\n");

    ret = bq24158_read_byte(RegNum, &bq24158_reg);
    printf("[bq24158_read_interface] Reg[%x]=0x%x\n", RegNum, bq24158_reg);
    
    bq24158_reg &= (MASK << SHIFT);
    *val = (bq24158_reg >> SHIFT);    
    printf("[bq24158_read_interface] val=0x%x\n", *val);

    return ret;
}

kal_uint32 bq24158_config_interface (kal_uint8 RegNum, kal_uint8 val, kal_uint8 MASK, kal_uint8 SHIFT)
{
    kal_uint8 bq24158_reg = 0;
    int ret = 0;

    printf("--------------------------------------------------LK\n");

    ret = bq24158_read_byte(RegNum, &bq24158_reg);
    printf("[bq24158_config_interface] Reg[%x]=0x%x\n", RegNum, bq24158_reg);
    
    bq24158_reg &= ~(MASK << SHIFT);
    bq24158_reg |= (val << SHIFT);

    ret = bq24158_write_byte(RegNum, bq24158_reg);
    printf("[bq24158_config_interface] write Reg[%x]=0x%x\n", RegNum, bq24158_reg);

    // Check
    //bq24158_read_byte(RegNum, &bq24158_reg);
    //printf("[bq24158_config_interface] Check Reg[%x]=0x%x\n", RegNum, bq24158_reg);

    return ret;
}

/**********************************************************
  *
  *   [bq24158 Function] 
  *
  *********************************************************/
//CON0----------------------------------------------------

void bq24158_set_tmr_rst(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=bq24158_config_interface(   (kal_uint8)(bq24158_CON0), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON0_TMR_RST_MASK),
                                    (kal_uint8)(CON0_TMR_RST_SHIFT)
                                    );
    if(g_bq24158_log_en>1)        
        printf("%d\n", ret);
}

kal_uint32 bq24158_get_otg_status(void)
{
    kal_uint32 ret=0;
    kal_uint8 val=0;

    ret=bq24158_read_interface(     (kal_uint8)(bq24158_CON0), 
                                    (&val),
                                    (kal_uint8)(CON0_OTG_MASK),
                                    (kal_uint8)(CON0_OTG_SHIFT)
                                    );
    if(g_bq24158_log_en>1)        
        printf("%d\n", ret);
    
    return val;
}

void bq24158_set_en_stat(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=bq24158_config_interface(   (kal_uint8)(bq24158_CON0), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON0_EN_STAT_MASK),
                                    (kal_uint8)(CON0_EN_STAT_SHIFT)
                                    );
    if(g_bq24158_log_en>1)        
            printf("%d\n", ret);    
}

kal_uint32 bq24158_get_chip_status(void)
{
    kal_uint32 ret=0;
    kal_uint8 val=0;

    ret=bq24158_read_interface(     (kal_uint8)(bq24158_CON0), 
                                    (&val),
                                    (kal_uint8)(CON0_STAT_MASK),
                                    (kal_uint8)(CON0_STAT_SHIFT)
                                    );
    if(g_bq24158_log_en>1)        
        printf("%d\n", ret);
    
    return val;
}

kal_uint32 bq24158_get_boost_status(void)
{
    kal_uint32 ret=0;
    kal_uint8 val=0;

    ret=bq24158_read_interface(     (kal_uint8)(bq24158_CON0), 
                                    (&val),
                                    (kal_uint8)(CON0_BOOST_MASK),
                                    (kal_uint8)(CON0_BOOST_SHIFT)
                                    );
    if(g_bq24158_log_en>1)        
        printf("%d\n", ret);
    
    return val;
}

kal_uint32 bq24158_get_fault_status(void)
{
    kal_uint32 ret=0;
    kal_uint8 val=0;

    ret=bq24158_read_interface(     (kal_uint8)(bq24158_CON0), 
                                    (&val),
                                    (kal_uint8)(CON0_FAULT_MASK),
                                    (kal_uint8)(CON0_FAULT_SHIFT)
                                    );
    if(g_bq24158_log_en>1)        
        printf("%d\n", ret);
    
    return val;
}

//CON1----------------------------------------------------

void bq24158_set_input_charging_current(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=bq24158_config_interface(   (kal_uint8)(bq24158_CON1), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON1_LIN_LIMIT_MASK),
                                    (kal_uint8)(CON1_LIN_LIMIT_SHIFT)
                                    );
    if(g_bq24158_log_en>1)        
        printf("%d\n", ret);    
}

void bq24158_set_v_low(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=bq24158_config_interface(   (kal_uint8)(bq24158_CON1), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON1_LOW_V_MASK),
                                    (kal_uint8)(CON1_LOW_V_SHIFT)
                                    );
    if(g_bq24158_log_en>1)        
        printf("%d\n", ret);
}

void bq24158_set_te(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=bq24158_config_interface(   (kal_uint8)(bq24158_CON1), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON1_TE_MASK),
                                    (kal_uint8)(CON1_TE_SHIFT)
                                    );
    if(g_bq24158_log_en>1)        
        printf("%d\n", ret);
}

void bq24158_set_ce(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=bq24158_config_interface(   (kal_uint8)(bq24158_CON1), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON1_CE_MASK),
                                    (kal_uint8)(CON1_CE_SHIFT)
                                    );
    if(g_bq24158_log_en>1)        
        printf("%d\n", ret);
}

void bq24158_set_hz_mode(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=bq24158_config_interface(   (kal_uint8)(bq24158_CON1), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON1_HZ_MODE_MASK),
                                    (kal_uint8)(CON1_HZ_MODE_SHIFT)
                                    );
    if(g_bq24158_log_en>1)        
        printf("%d\n", ret);
}

void bq24158_set_opa_mode(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=bq24158_config_interface(   (kal_uint8)(bq24158_CON1), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON1_OPA_MODE_MASK),
                                    (kal_uint8)(CON1_OPA_MODE_SHIFT)
                                    );
    if(g_bq24158_log_en>1)        
        printf("%d\n", ret);
}

//CON2----------------------------------------------------

void bq24158_set_oreg(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=bq24158_config_interface(   (kal_uint8)(bq24158_CON2), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON2_OREG_MASK),
                                    (kal_uint8)(CON2_OREG_SHIFT)
                                    );
    if(g_bq24158_log_en>1)        
        printf("%d\n", ret);
}

void bq24158_set_otg_pl(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=bq24158_config_interface(   (kal_uint8)(bq24158_CON2), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON2_OTG_PL_MASK),
                                    (kal_uint8)(CON2_OTG_PL_SHIFT)
                                    );
    if(g_bq24158_log_en>1)        
        printf("%d\n", ret);
}

void bq24158_set_otg_en(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=bq24158_config_interface(   (kal_uint8)(bq24158_CON2), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON2_OTG_EN_MASK),
                                    (kal_uint8)(CON2_OTG_EN_SHIFT)
                                    );
    if(g_bq24158_log_en>1)        
        printf("%d\n", ret);
}

//CON3----------------------------------------------------

kal_uint32 bq24158_get_vender_code(void)
{
    kal_uint32 ret=0;
    kal_uint8 val=0;

    ret=bq24158_read_interface(     (kal_uint8)(bq24158_CON3), 
                                    (&val),
                                    (kal_uint8)(CON3_VENDER_CODE_MASK),
                                    (kal_uint8)(CON3_VENDER_CODE_SHIFT)
                                    );
    if(g_bq24158_log_en>1)        
        printf("%d\n", ret);
    
    return val;
}

kal_uint32 bq24158_get_pn(void)
{
    kal_uint32 ret=0;
    kal_uint8 val=0;

    ret=bq24158_read_interface(     (kal_uint8)(bq24158_CON3), 
                                    (&val),
                                    (kal_uint8)(CON3_PIN_MASK),
                                    (kal_uint8)(CON3_PIN_SHIFT)
                                    );
    if(g_bq24158_log_en>1)        
        printf("%d\n", ret);
    
    return val;
}

kal_uint32 bq24158_get_revision(void)
{
    kal_uint32 ret=0;
    kal_uint8 val=0;

    ret=bq24158_read_interface(     (kal_uint8)(bq24158_CON3), 
                                    (&val),
                                    (kal_uint8)(CON3_REVISION_MASK),
                                    (kal_uint8)(CON3_REVISION_SHIFT)
                                    );
    if(g_bq24158_log_en>1)        
        printf("%d\n", ret);
    
    return val;
}

//CON4----------------------------------------------------

void bq24158_set_reset(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=bq24158_config_interface(   (kal_uint8)(bq24158_CON4), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON4_RESET_MASK),
                                    (kal_uint8)(CON4_RESET_SHIFT)
                                    );
    if(g_bq24158_log_en>1)        
        printf("%d\n", ret);    
}

void bq24158_set_iocharge(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=bq24158_config_interface(   (kal_uint8)(bq24158_CON4), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON4_I_CHR_MASK),
                                    (kal_uint8)(CON4_I_CHR_SHIFT)
                                    );
    if(g_bq24158_log_en>1)        
        printf("%d\n", ret);
}

void bq24158_set_iterm(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=bq24158_config_interface(   (kal_uint8)(bq24158_CON4), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON4_I_TERM_MASK),
                                    (kal_uint8)(CON4_I_TERM_SHIFT)
                                    );
    if(g_bq24158_log_en>1)        
        printf("%d\n", ret);
}

//CON5----------------------------------------------------

void bq24158_set_dis_vreg(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=bq24158_config_interface(   (kal_uint8)(bq24158_CON5), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON5_DIS_VREG_MASK),
                                    (kal_uint8)(CON5_DIS_VREG_SHIFT)
                                    );
    if(g_bq24158_log_en>1)        
        printf("%d\n", ret);
}

void bq24158_set_io_level(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=bq24158_config_interface(   (kal_uint8)(bq24158_CON5), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON5_IO_LEVEL_MASK),
                                    (kal_uint8)(CON5_IO_LEVEL_SHIFT)
                                    );
    if(g_bq24158_log_en>1)        
        printf("%d\n", ret);
}

kal_uint32 bq24158_get_sp_status(void)
{
    kal_uint32 ret=0;
    kal_uint8 val=0;

    ret=bq24158_read_interface(     (kal_uint8)(bq24158_CON5), 
                                    (&val),
                                    (kal_uint8)(CON5_SP_STATUS_MASK),
                                    (kal_uint8)(CON5_SP_STATUS_SHIFT)
                                    );
    if(g_bq24158_log_en>1)        
        printf("%d\n", ret);
    
    return val;
}

kal_uint32 bq24158_get_en_level(void)
{
    kal_uint32 ret=0;
    kal_uint8 val=0;

    ret=bq24158_read_interface(     (kal_uint8)(bq24158_CON5), 
                                    (&val),
                                    (kal_uint8)(CON5_EN_LEVEL_MASK),
                                    (kal_uint8)(CON5_EN_LEVEL_SHIFT)
                                    );
    if(g_bq24158_log_en>1)        
        printf("%d\n", ret);
    
    return val;
}

void bq24158_set_vsp(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=bq24158_config_interface(   (kal_uint8)(bq24158_CON5), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON5_VSP_MASK),
                                    (kal_uint8)(CON5_VSP_SHIFT)
                                    );
    if(g_bq24158_log_en>1)        
        printf("%d\n", ret);
}

//CON6----------------------------------------------------

void bq24158_set_i_safe(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=bq24158_config_interface(   (kal_uint8)(bq24158_CON6), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON6_ISAFE_MASK),
                                    (kal_uint8)(CON6_ISAFE_SHIFT)
                                    );
    if(g_bq24158_log_en>1)        
        printf("%d\n", ret);
}

void bq24158_set_v_safe(kal_uint32 val)
{
    kal_uint32 ret=0;    

    ret=bq24158_config_interface(   (kal_uint8)(bq24158_CON6), 
                                    (kal_uint8)(val),
                                    (kal_uint8)(CON6_VSAFE_MASK),
                                    (kal_uint8)(CON6_VSAFE_SHIFT)
                                    );
    if(g_bq24158_log_en>1)        
        printf("%d\n", ret);
}

/**********************************************************
  *
  *   [Internal Function] 
  *
  *********************************************************/
//debug liao
unsigned int bq24158_config_interface_liao (unsigned char RegNum, unsigned char val)
{
    int ret = 0;
    
    ret = bq24158_write_byte(RegNum, val);

    if(g_bq24158_log_en>1)        
        printf("%d\n", ret);

    return ret;
}

void bq24158_dump_register(void)
{
    int i=0;

    for (i=0;i<bq24158_REG_NUM;i++)
    {
        bq24158_read_byte(i, &bq24158_reg[i]);
        printf("[0x%x]=0x%x\n", i, bq24158_reg[i]);        
    }
}

void bq24158_read_register(int i)
{
    bq24158_read_byte(i, &bq24158_reg[i]);
    printf("[0x%x]=0x%x\n", i, bq24158_reg[i]); 
}

#if 1
#include <cust_gpio_usage.h>
int gpio_number   = GPIO_SWCHARGER_EN_PIN; 
int gpio_off_mode = GPIO_SWCHARGER_EN_PIN_M_GPIO;
int gpio_on_mode  = GPIO_SWCHARGER_EN_PIN_M_GPIO;
#else
int gpio_number   = (19 | 0x80000000); 
int gpio_off_mode = 0;
int gpio_on_mode  = 0;
#endif
int gpio_off_dir  = GPIO_DIR_OUT;
int gpio_off_out  = GPIO_OUT_ONE;
int gpio_on_dir   = GPIO_DIR_OUT;
int gpio_on_out   = GPIO_OUT_ZERO;

void bq24158_turn_on_charging(void)
{
    mt_set_gpio_mode(gpio_number,gpio_on_mode);  
    mt_set_gpio_dir(gpio_number,gpio_on_dir);
    mt_set_gpio_out(gpio_number,gpio_on_out);

    bq24158_config_interface_liao(0x00,0xC0);
    bq24158_config_interface_liao(0x01,0x78);
    //bq24158_config_interface_liao(0x02,0x8e);
    bq24158_config_interface_liao(0x05,0x04);
    bq24158_config_interface_liao(0x04,0x1A); //146mA
}

void bq24158_hw_init(void)
{
#if defined(HIGH_BATTERY_VOLTAGE_SUPPORT)
    printf("[bq24158_hw_init] (0x06,0x77)\n");
    bq24158_config_interface_liao(0x06,0x77); // set ISAFE and HW CV point (4.34)
    bq24158_config_interface_liao(0x02,0xaa); // 4.34    
#else
    printf("[bq24158_hw_init] (0x06,0x70)\n");
    bq24158_config_interface_liao(0x06,0x70); // set ISAFE
    bq24158_config_interface_liao(0x02,0x8e); // 4.2
#endif    
}

