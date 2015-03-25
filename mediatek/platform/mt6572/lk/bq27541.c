#include <platform/mt_typedefs.h>
#include <platform/mt_reg_base.h>
#include <platform/mt_i2c.h>   
#include <platform/bq27541.h>
#include <printf.h>

int g_bq27541_log_en=1;

/**********************************************************
  *
  *   [I2C Slave Setting] 
  *
  *********************************************************/
#define bq27541_SLAVE_ADDR_WRITE   0xAA
#define bq27541_SLAVE_ADDR_READ    0xAB

/**********************************************************
  *
  *   [Global Variable] 
  *
  *********************************************************/

/**********************************************************
  *
  *   [I2C Function For Read/Write bq27541] 
  *
  *********************************************************/  
U32 bq27541_set_cmd_read(U8 cmd, int *returnData)
{
    char cmd_buf[2]={0x00, 0x00};
    U32 ret_code = I2C_OK;
    int readData = 0;
	
    struct mt_i2c_t i2c;

    cmd_buf[0] = cmd;

    i2c.id = I2C6;
	i2c.dir = 1; //I2C4~6 need set to 1
	i2c.addr = 0x55;
	i2c.mode = ST_MODE;
	i2c.speed = 100;
	i2c.is_rs_enable = 1;
    i2c.is_push_pull_enable = 0;
	i2c.is_clk_ext_disable = 0;
	i2c.delay_len = 0;
	i2c.is_dma_enabled = 0;

    ret_code = mt_i2c_write_new(&i2c, &cmd_buf[0], 1);	 // set register command

	if (ret_code != I2C_OK)
    return ret_code;

//    printf("buffer write = [%X][%X]\n", buffer[0], buffer[1]);

    i2c.id = I2C6;
	i2c.dir = 1; //I2C4~6 need set to 1
	i2c.addr = 0x55;
	i2c.mode = ST_MODE;
	i2c.speed = 100;
	i2c.is_rs_enable = 0;
    i2c.is_push_pull_enable = 0;
	i2c.is_clk_ext_disable = 0;
	i2c.delay_len = 0;
	i2c.is_dma_enabled = 0;

    ret_code = mt_i2c_read_new(&i2c, &cmd_buf[0], 2);	 // set register command

	if (ret_code != I2C_OK)
    return ret_code;

//    printf("buffer read = [%X][%X]\n", buffer[0], buffer[1]);

    readData = (cmd_buf[1] << 8) | cmd_buf[0];
    *returnData = readData;
	
    return ret_code;
}	
