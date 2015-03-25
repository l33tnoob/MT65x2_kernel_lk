#include <platform/mt_typedefs.h>
#include <platform/mt_reg_base.h>
#include <platform/mt_i2c.h>
#include <platform/mtk_pmic_mt6333.h>

#define MT6333_I2C_ID	I2C1

#define mt6333_SLAVE_ADDR_WRITE   0xD6
#define mt6333_SLAVE_ADDR_Read    0xD7

static struct mt_i2c_t mt6333_i2c;

/**********************************************************
  *
  *   [I2C Function For Read/Write mt6333] 
  *
  *********************************************************/
kal_uint32 mt6333_write_byte(kal_uint8 addr, kal_uint8 value)
{
        kal_uint32 ret_code = I2C_OK;
        kal_uint8 write_data[2];
        kal_uint16 len;

        write_data[0]= addr;
        write_data[1] = value;

        mt6333_i2c.id = MT6333_I2C_ID;
		/* Since i2c will left shift 1 bit, we need to set MT6333 I2C address to >>1 */
        mt6333_i2c.addr = (mt6333_SLAVE_ADDR_WRITE >> 1);
        mt6333_i2c.mode = ST_MODE;
        mt6333_i2c.speed = 100;
        len = 2;

        ret_code = i2c_write(&mt6333_i2c, write_data, len);
        printf("%s: i2c_write: ret_code: %d\n", __func__, ret_code);

        return ret_code;
}

kal_uint32 mt6333_read_byte (kal_uint8 addr, kal_uint8 *dataBuffer) {
        kal_uint32 ret_code = I2C_OK;
        kal_uint16 len;
		*dataBuffer = addr;

        mt6333_i2c.id = MT6333_I2C_ID;
		/* Since i2c will left shift 1 bit, we need to set MT6333 I2C address to >>1 */
        mt6333_i2c.addr = (mt6333_SLAVE_ADDR_WRITE >> 1);
        mt6333_i2c.mode = ST_MODE;
        mt6333_i2c.speed = 100;
        len = 1;

        ret_code = i2c_write_read(&mt6333_i2c, dataBuffer, len, len);
        printf("%s: i2c_read: ret_code: %d\n", __func__, ret_code);

        return ret_code;
}

/**********************************************************
  *
  *   [Read / Write Function] 
  *
  *********************************************************/
kal_uint32 mt6333_read_interface (kal_uint8 RegNum, kal_uint8 *val, kal_uint8 MASK, kal_uint8 SHIFT)
{
    kal_uint8 mt6333_reg = 0;
    int ret = 0;
    
#if 1
    printf("--------------------------------------------------LK\n");

    ret = mt6333_read_byte(RegNum, &mt6333_reg);
    printf("[mt6333_read_interface] Reg[%x]=0x%x\n", RegNum, mt6333_reg);
    
    mt6333_reg &= (MASK << SHIFT);
    *val = (mt6333_reg >> SHIFT);    
    printf("[mt6333_read_interface] val=0x%x\n", *val);
#endif    

    return ret;
}

kal_uint32 mt6333_config_interface (kal_uint8 RegNum, kal_uint8 val, kal_uint8 MASK, kal_uint8 SHIFT)
{
    kal_uint8 mt6333_reg = 0;
    int ret = 0;

#if 1
    printf("--------------------------------------------------LK\n");

    ret = mt6333_read_byte(RegNum, &mt6333_reg);
    printf("[mt6333_config_interface] Reg[%x]=0x%x\n", RegNum, mt6333_reg);
    
    mt6333_reg &= ~(MASK << SHIFT);
    mt6333_reg |= (val << SHIFT);

    ret = mt6333_write_byte(RegNum, mt6333_reg);
    printf("[mt6333_config_interface] write Reg[%x]=0x%x\n", RegNum, mt6333_reg);

    // Check
    //mt6333_read_byte(RegNum, &mt6333_reg);
    //printf("[mt6333_config_interface] Check Reg[%x]=0x%x\n", RegNum, mt6333_reg);
#endif

    return ret;
}  

kal_uint32 mt6333_get_reg_value(kal_uint32 reg)
{
    kal_uint32 ret=0;
    kal_uint8 reg_val=0;

    ret=mt6333_read_interface( (kal_uint8) reg, &reg_val, 0xFF, 0x0);
    if(ret!=0) printf("%d", ret);
    
    return reg_val;
}

//==============================================================================
// PMIC6323 Init Code
//==============================================================================
void mt6333_init (void)
{
    printf("[mt6333_init] Reg[%x]=0x%x\n", MT6333_CID0, mt6333_get_reg_value(MT6333_CID0));
    printf("[mt6333_init] Reg[%x]=0x%x\n", MT6333_CID1, mt6333_get_reg_value(MT6333_CID1));
}

