#include <platform/mt_typedefs.h>
#include <platform/mt_reg_base.h>
#include <platform/mt_i2c.h>  
#include <platform/mt_gpio.h>
#include <platform/ncp1851.h>
#include <printf.h>
/**********************************************************
  *
  *   [I2C Slave Setting]
  *
  *********************************************************/
#define NCP1851_SLAVE_ADDR_WRITE	0x6C
#define NCP1851_SLAVE_ADDR_Read	0x6D

/**********************************************************
  *
  *   [Global Variable]
  *
  *********************************************************/
#define NCP1851_REG_NUM 19

#ifndef NCP1851_BUSNUM
#define NCP1851_BUSNUM 1
#endif  

#ifndef GPIO_CHR_SPM_PIN
#define GPIO_CHR_SPM_PIN 65
#endif
kal_uint8 ncp1851_reg[NCP1851_REG_NUM] = {0};

/**********************************************************
  *
  *   [I2C Function For Read/Write ncp1851]
  *
  *********************************************************/
U32 ncp1851_i2c_read (U8 chip, U8 *cmdBuffer, int cmdBufferLen, U8 *dataBuffer, int dataBufferLen)
{
    U32 ret_code = I2C_OK;

    ret_code = mt_i2c_write(NCP1851_BUSNUM, chip, cmdBuffer, cmdBufferLen, 1);    // set register command
    if (ret_code != I2C_OK)
        return ret_code;

    ret_code = mt_i2c_read(NCP1851_BUSNUM, chip, dataBuffer, dataBufferLen, 1);

    //dbg_print("[pmic6329_i2c_read] Done\n");

    return ret_code;
}

U32 ncp1851_i2c_write (U8 chip, U8 *cmdBuffer, int cmdBufferLen, U8 *dataBuffer, int dataBufferLen)
{
    U32 ret_code = I2C_OK;
    U8 write_data[I2C_FIFO_SIZE];
    int transfer_len = cmdBufferLen + dataBufferLen;
    int i=0, cmdIndex=0, dataIndex=0;

    if(I2C_FIFO_SIZE < (cmdBufferLen + dataBufferLen))
    {
        printf("[ncp1851_i2c_write] exceed I2C FIFO length!! \n");
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
        //dbg_print("[pmic6329_i2c_write] write_data[%d]=%x\n", i, write_data[i]);
    }

    ret_code = mt_i2c_write(NCP1851_BUSNUM, chip, write_data, transfer_len, 1);

    //dbg_print("[pmic6329_i2c_write] Done\n");

    return ret_code;
}

/**********************************************************
  *
  *   [Read / Write Function]
  *
  *********************************************************/
kal_uint32 ncp1851_read_interface (kal_uint8 RegNum, kal_uint8 *val, kal_uint8 MASK, kal_uint8 SHIFT)
{
    U8 chip_slave_address = NCP1851_SLAVE_ADDR_WRITE;
    U8 cmd = 0x0;
    int cmd_len = 1;
    U8 data = 0xFF;
    int data_len = 1;
    U32 result_tmp;

    cmd = RegNum;
    result_tmp = ncp1851_i2c_read(chip_slave_address, &cmd, cmd_len, &data, data_len);

    //printk("[ncp1851_read_interface] Reg[%x]=0x%x\n", RegNum, data);

    data &= (MASK << SHIFT);
    *val = (data >> SHIFT);

    //printk("[ncp1851_read_interface] val=0x%x\n", *val);
    return 1;
}

kal_uint32 ncp1851_config_interface (kal_uint8 RegNum, kal_uint8 val, kal_uint8 MASK, kal_uint8 SHIFT)
{
    U8 chip_slave_address = NCP1851_SLAVE_ADDR_WRITE;
    U8 cmd = 0x0;
    int cmd_len = 1;
    U8 data = 0xFF;
    int data_len = 1;
    U32 result_tmp;

    cmd = RegNum;
    result_tmp = ncp1851_i2c_read(chip_slave_address, &cmd, cmd_len, &data, data_len);
    //printk("[ncp1851_config_interface] Reg[%x]=0x%x\n", RegNum, data);

    data &= ~(MASK << SHIFT);
    data |= (val << SHIFT);

    result_tmp = ncp1851_i2c_write(chip_slave_address, &cmd, cmd_len, &data, data_len);
    //printk("[ncp1851_config_interface] write Reg[%x]=0x%x\n", RegNum, ncp1851_reg);

    // Check
    result_tmp = ncp1851_i2c_read(chip_slave_address, &cmd, cmd_len, &data, data_len);
    //printk("[ncp1851_config_interface] Check Reg[%x]=0x%x\n", RegNum, ncp1851_reg);

    return 1;
}

/**********************************************************
  *
  *   [ncp1851 Function]
  *
  *********************************************************/
//CON0
kal_uint32 ncp1851_get_chip_status(void)
{
    kal_uint32 ret=0;
    kal_uint8 val=0;

    ret=ncp1851_read_interface((kal_uint8)(NCP1851_CON0),
							    (&val),
							    (kal_uint8)(CON0_STATE_MASK),
							    (kal_uint8)(CON0_STATE_SHIFT)
							    );
    return val;
}

kal_uint32 ncp1851_get_batfet(void)
{
    kal_uint32 ret=0;
    kal_uint8 val=0;

    ret=ncp1851_read_interface((kal_uint8)(NCP1851_CON0),
	        					      (&val),
							      (kal_uint8)(CON0_BATFET_MASK),
							      (kal_uint8)(CON0_BATFET_SHIFT)
							      );
    return val;
}

kal_uint32 ncp1851_get_ntc(void)
{
    kal_uint32 ret=0;
    kal_uint8 val=0;

    ret=ncp1851_read_interface((kal_uint8)(NCP1851_CON0),
	        					      (&val),
							      (kal_uint8)(CON0_NTC_MASK),
							      (kal_uint8)(CON0_NTC_SHIFT)
							      );
    return val;
}

//CON1
void ncp1851_set_reset(kal_uint32 val)
{
    kal_uint32 ret=0;

    ret=ncp1851_config_interface((kal_uint8)(NCP1851_CON1),
								(kal_uint8)(val),
								(kal_uint8)(CON1_REG_RST_MASK),
								(kal_uint8)(CON1_REG_RST_SHIFT)
								);
}

void ncp1851_set_chg_en(kal_uint32 val)
{
    kal_uint32 ret=0;

    ret=ncp1851_config_interface((kal_uint8)(NCP1851_CON1),
    								(kal_uint8)(val),
    								(kal_uint8)(CON1_CHG_EN_MASK),
    								(kal_uint8)(CON1_CHG_EN_SHIFT)
    								);
}

void ncp1851_set_otg_en(kal_uint32 val)
{
    kal_uint32 ret=0;

    ret=ncp1851_config_interface((kal_uint8)(NCP1851_CON1),
                                                   (kal_uint8)(val),
                                                   (kal_uint8)(CON1_OTG_EN_MASK),
                                                   (kal_uint8)(CON1_OTG_EN_SHIFT)
                                                   );
    return val;
}

void ncp1851_set_ntc_en(kal_uint32 val)
{
    kal_uint32 ret=0;

    ret=ncp1851_config_interface((kal_uint8)(NCP1851_CON1),
    								(kal_uint8)(val),
    								(kal_uint8)(CON1_NTC_EN_MASK),
    								(kal_uint8)(CON1_NTC_EN_SHIFT)
    								);
}

void ncp1851_set_tj_warn_opt(kal_uint32 val)
{
    kal_uint32 ret=0;

    ret=ncp1851_config_interface((kal_uint8)(NCP1851_CON1),
                                                   (kal_uint8)(val),
                                                   (kal_uint8)(CON1_TJ_WARN_OPT_MASK),
                                                   (kal_uint8)(CON1_TJ_WARN_OPT_SHIFT)
                                                   );
    return val;
}

void ncp1851_set_jeita_opt(kal_uint32 val)
{
    kal_uint32 ret=0;

    ret=ncp1851_config_interface((kal_uint8)(NCP1851_CON1),
                                                   (kal_uint8)(val),
                                                   (kal_uint8)(CON1_JEITA_OPT_MASK),
                                                   (kal_uint8)(CON1_JEITA_OPT_SHIFT)
                                                   );
    return val;
}

void ncp1851_set_tchg_rst(kal_uint32 val)
{
    kal_uint32 ret=0;

    ret=ncp1851_config_interface(	(kal_uint8)(NCP1851_CON1),
								(kal_uint8)(val),
								(kal_uint8)(CON1_TCHG_RST_MASK),
								(kal_uint8)(CON1_TCHG_RST_SHIFT)
								);
}

void ncp1851_set_int_mask(kal_uint32 val)
{
    kal_uint32 ret=0;

    ret=ncp1851_config_interface((kal_uint8)(NCP1851_CON1),
                                                   (kal_uint8)(val),
                                                   (kal_uint8)(CON1_INT_MASK_MASK),
                                                   (kal_uint8)(CON1_INT_MASK_SHIFT)
                                                   );
    return val;
}

//CON2
void ncp1851_set_wdto_dis(kal_uint32 val)
{
    kal_uint32 ret=0;

    ret=ncp1851_config_interface((kal_uint8)(NCP1851_CON2),
								(kal_uint8)(val),
								(kal_uint8)(CON2_WDTO_DIS_MASK),
								(kal_uint8)(CON2_WDTO_DIS_SHIFT)
								);
}

void ncp1851_set_chgto_dis(kal_uint32 val)
{
    kal_uint32 ret=0;

    ret=ncp1851_config_interface((kal_uint8)(NCP1851_CON2),
								(kal_uint8)(val),
								(kal_uint8)(CON2_CHGTO_DIS_MASK),
								(kal_uint8)(CON2_CHGTO_DIS_SHIFT)
								);
}

void ncp1851_set_pwr_path(kal_uint32 val)
{
    kal_uint32 ret=0;

    ret=ncp1851_config_interface((kal_uint8)(NCP1851_CON2),
								(kal_uint8)(val),
								(kal_uint8)(CON2_PWR_PATH_MASK),
								(kal_uint8)(CON2_PWR_PATH_SHIFT)
								);
}

void ncp1851_set_trans_en(kal_uint32 val)
{
    kal_uint32 ret=0;

    ret=ncp1851_config_interface((kal_uint8)(NCP1851_CON2),
								(kal_uint8)(val),
								(kal_uint8)(CON2_TRANS_EN_MASK),
								(kal_uint8)(CON2_TRANS_EN_SHIFT)
								);
}

void ncp1851_set_factory_mode(kal_uint32 val)
{
    kal_uint32 ret=0;

    ret=ncp1851_config_interface((kal_uint8)(NCP1851_CON2),
								(kal_uint8)(val),
								(kal_uint8)(CON2_FCTRY_MOD_MASK),
								(kal_uint8)(CON2_FCTRY_MOD_SHIFT)
								);
}

void ncp1851_set_iinset_pin_en(kal_uint32 val)
{
    kal_uint32 ret=0;

    ret=ncp1851_config_interface((kal_uint8)(NCP1851_CON2),
								(kal_uint8)(val),
								(kal_uint8)(CON2_IINSET_PIN_EN_MASK),
								(kal_uint8)(CON2_IINSET_PIN_EN_SHIFT)
								);
}

void ncp1851_set_iinlim_en(kal_uint32 val)
{
    kal_uint32 ret=0;

    ret=ncp1851_config_interface((kal_uint8)(NCP1851_CON2),
								(kal_uint8)(val),
								(kal_uint8)(CON2_IINLIM_EN_MASK),
								(kal_uint8)(CON2_IINLIM_EN_SHIFT)
								);
}

void ncp1851_set_aicl_en(kal_uint32 val)
{
    kal_uint32 ret=0;

    ret=ncp1851_config_interface((kal_uint8)(NCP1851_CON2),
								(kal_uint8)(val),
								(kal_uint8)(CON2_AICL_EN_MASK),
								(kal_uint8)(CON2_AICL_EN_SHIFT)
								);
}

//CON8
kal_uint32 ncp1851_get_vfet_ok(void)
{
    kal_uint32 ret=0;
    kal_uint8 val=0;

    ret=ncp1851_read_interface((kal_uint8)(NCP1851_CON8),
	        					      (&val),
							      (kal_uint8)(CON8_VFET_OK_MASK),
							      (kal_uint8)(CON8_VFET_OK_SHIFT)
							      );
    return val;
}


//CON14
void ncp1851_set_ctrl_vbat(kal_uint32 val)
{
    kal_uint32 ret=0;

    ret=ncp1851_config_interface((kal_uint8)(NCP1851_CON14),
								(kal_uint8)(val),
								(kal_uint8)(CON14_CTRL_VBAT_MASK),
								(kal_uint8)(CON14_CTRL_VBAT_SHIFT)
								);
}

//CON15
void ncp1851_set_ieoc(kal_uint32 val)
{
    kal_uint32 ret=0;

    ret=ncp1851_config_interface((kal_uint8)(NCP1851_CON15),
								(kal_uint8)(val),
								(kal_uint8)(CON15_IEOC_MASK),
								(kal_uint8)(CON15_IEOC_SHIFT)
								);
}

void ncp1851_set_ichg(kal_uint32 val)
{
    kal_uint32 ret=0;

    ret=ncp1851_config_interface((kal_uint8)(NCP1851_CON15),
								(kal_uint8)(val),
								(kal_uint8)(CON15_ICHG_MASK),
								(kal_uint8)(CON15_ICHG_SHIFT)
								);
}

//CON16
void ncp1851_set_iweak(kal_uint32 val)
{
    kal_uint32 ret=0;

    ret=ncp1851_config_interface((kal_uint8)(NCP1851_CON16),
								(kal_uint8)(val),
								(kal_uint8)(CON16_IWEAK_MASK),
								(kal_uint8)(CON16_IWEAK_SHIFT)
								);
}

void ncp1851_set_ctrl_vfet(kal_uint32 val)
{
    kal_uint32 ret=0;

    ret=ncp1851_config_interface((kal_uint8)(NCP1851_CON16),
								(kal_uint8)(val),
								(kal_uint8)(CON16_CTRL_VFET_MASK),
								(kal_uint8)(CON16_CTRL_VFET_SHIFT)
								);
}

void ncp1851_set_iinlim(kal_uint32 val)
{
    kal_uint32 ret=0;

    ret=ncp1851_config_interface((kal_uint8)(NCP1851_CON16),
								(kal_uint8)(val),
								(kal_uint8)(CON16_IINLIM_MASK),
								(kal_uint8)(CON16_IINLIM_SHIFT)
								);
}

//CON17
void ncp1851_set_vchred(kal_uint32 val)
{
    kal_uint32 ret=0;

    ret=ncp1851_config_interface((kal_uint8)(NCP1851_CON17),
								(kal_uint8)(val),
								(kal_uint8)(CON17_VCHRED_MASK),
								(kal_uint8)(CON17_VCHRED_SHIFT)
								);
}

void ncp1851_set_ichred(kal_uint32 val)
{
    kal_uint32 ret=0;

    ret=ncp1851_config_interface((kal_uint8)(NCP1851_CON17),
								(kal_uint8)(val),
								(kal_uint8)(CON17_ICHRED_MASK),
								(kal_uint8)(CON17_ICHRED_SHIFT)
								);
}

//CON18
void ncp1851_set_batcold(kal_uint32 val)
{
    kal_uint32 ret=0;

    ret=ncp1851_config_interface((kal_uint8)(NCP1851_CON18),
								(kal_uint8)(val),
								(kal_uint8)(CON18_BATCOLD_MASK),
								(kal_uint8)(CON18_BATCOLD_SHIFT)
								);
}

void ncp1851_set_bathot(kal_uint32 val)
{
    kal_uint32 ret=0;

    ret=ncp1851_config_interface((kal_uint8)(NCP1851_CON18),
								(kal_uint8)(val),
								(kal_uint8)(CON18_BATHOT_MASK),
								(kal_uint8)(CON18_BATHOT_SHIFT)
								);
}

void ncp1851_set_batchilly(kal_uint32 val)
{
    kal_uint32 ret=0;

    ret=ncp1851_config_interface((kal_uint8)(NCP1851_CON18),
								(kal_uint8)(val),
								(kal_uint8)(CON18_BATCHIL_MASK),
								(kal_uint8)(CON18_BATCHIL_SHIFT)
								);
}

void ncp1851_set_batwarm(kal_uint32 val)
{
    kal_uint32 ret=0;

    ret=ncp1851_config_interface((kal_uint8)(NCP1851_CON18),
								(kal_uint8)(val),
								(kal_uint8)(CON18_BATWARM_MASK),
								(kal_uint8)(CON18_BATWARM_SHIFT)
								);
}

/**********************************************************
  *
  *   [Internal Function]
  *
  *********************************************************/
void ncp1851_dump_register(void)
{
    U8 chip_slave_address = NCP1851_SLAVE_ADDR_WRITE;
    U8 cmd = 0x0;
    int cmd_len = 1;
    U8 data = 0xFF;
    int data_len = 1;
    int i=0;
    for (i=0;i<NCP1851_REG_NUM;i++)
    {
        cmd = i;
        ncp1851_i2c_read(chip_slave_address, &cmd, cmd_len, &data, data_len);
        ncp1851_reg[i] = data;
        printf("[ncp1851_dump_register] Reg[0x%X]=0x%X\n", i, ncp1851_reg[i]);
    }
}

void ncp1851_read_register(int i)
{
    U8 chip_slave_address = NCP1851_SLAVE_ADDR_WRITE;
    U8 cmd = 0x0;
    int cmd_len = 1;
    U8 data = 0xFF;
    int data_len = 1;

    cmd = i;
    ncp1851_i2c_read(chip_slave_address, &cmd, cmd_len, &data, data_len);
    ncp1851_reg[i] = data;
    printf("[ncp1851_read_register] Reg[0x%x]=0x%X\n", i, ncp1851_reg[i]);
}
void ncp1851_hw_init()
{
    printf("[ncp1851] ChargerHwInit_ncp1851\n" );

    kal_uint32 ncp1851_status;
    ncp1851_status = ncp1851_get_chip_status();

    upmu_set_rg_bc11_bb_ctrl(1);    //BC11_BB_CTRL    
    upmu_set_rg_bc11_rst(1);        //BC11_RST

    ncp1851_set_otg_en(0x0);
    ncp1851_set_ntc_en(0x0);
    ncp1851_set_trans_en(0);
    ncp1851_set_tj_warn_opt(0x1);
    ncp1851_set_jeita_opt(0x1);
//  ncp1851_set_int_mask(0x0); //disable all interrupt
    ncp1851_set_int_mask(0x1); //enable all interrupt for boost mode status monitor
    ncp1851_set_tchg_rst(0x1); //reset charge timer
#ifdef NCP1851_PWR_PATH
    ncp1851_set_pwr_path(0x1);
#else
    ncp1851_set_pwr_path(0x0);
#endif

    if((ncp1851_status == 0x8) || (ncp1851_status == 0x9) || (ncp1851_status == 0xA)) //WEAK WAIT, WEAK SAFE, WEAK CHARGE
        ncp1851_set_ctrl_vbat(0x1C); //VCHG = 4.0V
    else if(ncp1851_status == 0x4)
        ncp1851_set_ctrl_vbat(0x28); //VCHG = 4.3V to decrease charge time
    else
        ncp1851_set_ctrl_vbat(0x24); //VCHG = 4.2V

    ncp1851_set_ieoc(0x4); // terminate current = 200mA for ICS optimized suspend power

    ncp1851_set_iweak(0x3); //weak charge current = 300mA

    ncp1851_set_ctrl_vfet(0x3); // VFET = 3.4V
    ncp1851_set_vchred(0x2); //reduce 200mV (JEITA)
    ncp1851_set_ichred(0x0); //reduce 400mA (JEITA)
    ncp1851_set_batcold(0x5);
    ncp1851_set_bathot(0x3);
    ncp1851_set_batchilly(0x0);
    ncp1851_set_batwarm(0x0);
}

void ncp1851_turn_on_charging()
{
	ncp1851_set_chg_en(0x1); // charger enable
	//Set SPM = 1, GPIO_CHR_SPM_PIN
	mt_set_gpio_mode(GPIO_CHR_SPM_PIN, GPIO_MODE_00);
	mt_set_gpio_dir(GPIO_CHR_SPM_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_CHR_SPM_PIN, GPIO_OUT_ONE);
 }
