#include <linux/i2c.h>
#include <linux/irq.h>
#include <linux/miscdevice.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/kobject.h>
#include <linux/earlysuspend.h>
#include <generated/autoconf.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/kthread.h>
#include <linux/wakelock.h>
#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/aee.h>
#include <linux/xlog.h>
#include <linux/proc_fs.h>
#include <linux/syscalls.h>
#include <linux/sched.h>
#include <linux/writeback.h>
#include <linux/xlog.h>

#include <asm/uaccess.h>
#include <asm/atomic.h>

#include <mach/eint.h>
#include <mach/mt_typedefs.h>
#include <mach/mt_gpio.h>

#include <mach/mt6333.h>
#include <mach/upmu_common.h>

extern void pmu_drv_tool_customization_init(void);
extern void dump_ldo_status_read_debug(void);

/**********************************************************
  *
  *   [I2C Slave Setting] 
  *
  *********************************************************/
#define mt6333_SLAVE_ADDR_WRITE   0xD6
#define mt6333_SLAVE_ADDR_Read    0xD7
#define mt6333_BUSNUM 1

static struct i2c_board_info __initdata i2c_mt6333 = { I2C_BOARD_INFO("mt6333", (mt6333_SLAVE_ADDR_WRITE>>1))};

static struct i2c_client *new_client = NULL;
static const struct i2c_device_id mt6333_i2c_id[] = {{"mt6333",0},{}};   

static int mt6333_driver_probe(struct i2c_client *client, const struct i2c_device_id *id);

static struct i2c_driver mt6333_driver = {
    .driver = {
        .name    = "mt6333",
    },
    .probe       = mt6333_driver_probe,
    .id_table    = mt6333_i2c_id,
};

kal_bool mt6333_hw_init_done = KAL_FALSE;
kal_int32 g_mt6333_cid = 0;

#ifdef MTK_SWCHR_SUPPORT
kal_bool chargin_hw_init_done = KAL_FALSE;
#endif

/**********************************************************
  *
  *   [Global Variable] 
  *
  *********************************************************/
static DEFINE_MUTEX(mt6333_i2c_access);

kal_uint8 g_reg_value_mt6333=0;

/**********************************************************
  *
  *   [I2C Function For Read/Write mt6333] 
  *
  *********************************************************/
int mt6333_read_byte(kal_uint8 cmd, kal_uint8 *returnData)
{
#if 1
    char     cmd_buf[1]={0x00};
    char     readData = 0;
    int      ret=0;

    mutex_lock(&mt6333_i2c_access);
        
    #if 1
    new_client->ext_flag=((new_client->ext_flag ) & I2C_MASK_FLAG ) | I2C_WR_FLAG | I2C_DIRECTION_FLAG;
    #else
    new_client->ext_flag=((new_client->ext_flag ) & I2C_MASK_FLAG ) | I2C_WR_FLAG | I2C_DIRECTION_FLAG | I2C_HS_FLAG;
    new_client->timing=2000;
    #endif

    cmd_buf[0] = cmd;
    ret = i2c_master_send(new_client, &cmd_buf[0], (1<<8 | 1));
    if (ret < 0) 
    {
        new_client->ext_flag=0;

        mutex_unlock(&mt6333_i2c_access);
        return 0;
    }
    
    readData = cmd_buf[0];
    *returnData = readData;
    
    new_client->ext_flag=0;

    mutex_unlock(&mt6333_i2c_access);    
#endif

    return 1;
}

int mt6333_write_byte(kal_uint8 cmd, kal_uint8 writeData)
{
#if 1
    char    write_data[2] = {0};
    int     ret=0;
    
    mutex_lock(&mt6333_i2c_access);
    
    write_data[0] = cmd;
    write_data[1] = writeData;

    #if 1
    new_client->ext_flag=((new_client->ext_flag ) & I2C_MASK_FLAG ) | I2C_DIRECTION_FLAG;
    #else
    new_client->ext_flag=((new_client->ext_flag ) & I2C_MASK_FLAG ) | I2C_DIRECTION_FLAG | I2C_HS_FLAG;
    new_client->timing=2000;
    #endif
    
    ret = i2c_master_send(new_client, write_data, 2);
    if (ret < 0) 
    {
       
        new_client->ext_flag=0;
        mutex_unlock(&mt6333_i2c_access);
        return 0;
    }
    
    new_client->ext_flag=0;
    
    mutex_unlock(&mt6333_i2c_access);
#endif

    return 1;
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

    //printk("--------------------------------------------------\n");

    ret = mt6333_read_byte(RegNum, &mt6333_reg);
    //printk("[mt6333_read_interface] Reg[%x]=0x%x\n", RegNum, mt6333_reg);
    
    mt6333_reg &= (MASK << SHIFT);
    *val = (mt6333_reg >> SHIFT);    
    //printk("[mt6333_read_interface] val=0x%x\n", *val);

    return ret;
}

kal_uint32 mt6333_config_interface (kal_uint8 RegNum, kal_uint8 val, kal_uint8 MASK, kal_uint8 SHIFT)
{
    kal_uint8 mt6333_reg = 0;
    int ret = 0;

    //printk("--------------------------------------------------\n");

    ret = mt6333_read_byte(RegNum, &mt6333_reg);
    //printk("[mt6333_config_interface] Reg[%x]=0x%x\n", RegNum, mt6333_reg);
    
    mt6333_reg &= ~(MASK << SHIFT);
    mt6333_reg |= (val << SHIFT);

    ret = mt6333_write_byte(RegNum, mt6333_reg);
    //printk("[mt6333_config_interface] write Reg[%x]=0x%x\n", RegNum, mt6333_reg);

    // Check
    //mt6333_read_byte(RegNum, &mt6333_reg);
    //printk("[mt6333_config_interface] Check Reg[%x]=0x%x\n", RegNum, mt6333_reg);

    return ret;
}

/**********************************************************
  *
  *   [Internal APIs] 
  *
  *********************************************************/
void mt6333_lock(void)
{
}

void mt6333_unlock(void)
{
} 
  
kal_uint8 mt6333_get_cid0(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_CID0),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_CID0_MASK),
                           (kal_uint8)(MT6333_PMIC_CID0_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

void mt6333_set_rg_bgr_rsel(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_PERP_CON0),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_BGR_RSEL_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_BGR_RSEL_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_bgr_unchop(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_PERP_CON0),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_BGR_UNCHOP_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_BGR_UNCHOP_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_bgr_unchop_ph(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_PERP_CON0),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_BGR_UNCHOP_PH_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_BGR_UNCHOP_PH_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_bgr_trim(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_PERP_CON1),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_BGR_TRIM_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_BGR_TRIM_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_bgr_trim_en(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_PERP_CON1),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_BGR_TRIM_EN_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_BGR_TRIM_EN_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_bgr_test_en(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_PERP_CON2),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_BGR_TEST_EN_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_BGR_TEST_EN_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_bgr_test_rstb(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_PERP_CON2),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_BGR_TEST_RSTB_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_BGR_TEST_RSTB_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_bgr_test_ckin(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_PERP_CON2),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_BGR_TEST_CKIN_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_BGR_TEST_CKIN_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_chr_en(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_PERP_CON3),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_CHR_EN_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_CHR_EN_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_vbout_en(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_PERP_CON4),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_VBOUT_EN_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_VBOUT_EN_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_adcin_vbat_en(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_PERP_CON4),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_ADCIN_VBAT_EN_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_ADCIN_VBAT_EN_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_adcin_chrin_en(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_PERP_CON4),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_ADCIN_CHRIN_EN_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_ADCIN_CHRIN_EN_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_adcin_baton_en(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_PERP_CON4),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_ADCIN_BATON_EN_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_ADCIN_BATON_EN_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_bat_on_open_vth(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_PERP_CON4),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_BAT_ON_OPEN_VTH_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_BAT_ON_OPEN_VTH_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_bat_on_pull_high_en(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_PERP_CON4),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_BAT_ON_PULL_HIGH_EN_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_BAT_ON_PULL_HIGH_EN_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_int_ph_enb(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_PERP_CON5),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_INT_PH_ENB_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_INT_PH_ENB_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_scl_ph_enb(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_PERP_CON5),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_SCL_PH_ENB_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_SCL_PH_ENB_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_sda_ph_enb(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_PERP_CON5),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_SDA_PH_ENB_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_SDA_PH_ENB_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_vdrv_rdivsel(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_PERP_CON6),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_VDRV_RDIVSEL_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_VDRV_RDIVSEL_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_swchr_rv1(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_PERP_CON7),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_SWCHR_RV1_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_SWCHR_RV1_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_chr_otg_lv_th(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_PERP_CON8),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_CHR_OTG_LV_TH_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_CHR_OTG_LV_TH_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_chr_otg_hv_th(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_PERP_CON8),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_CHR_OTG_HV_TH_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_CHR_OTG_HV_TH_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_strup_ther_rg_th(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_PERP_CON9),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_STRUP_THER_RG_TH_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_STRUP_THER_RG_TH_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_strup_rsv(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_PERP_CON9),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_STRUP_RSV_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_STRUP_RSV_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_swchr_ana_test_mode(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_PERP_CON10),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_SWCHR_ANA_TEST_MODE_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_SWCHR_ANA_TEST_MODE_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_swchr_ana_test_mode_sel(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_PERP_CON10),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_SWCHR_ANA_TEST_MODE_SEL_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_SWCHR_ANA_TEST_MODE_SEL_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_csa_otg_sel(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_PERP_CON11),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_CSA_OTG_SEL_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_CSA_OTG_SEL_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_otg_cs_slp_en(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_PERP_CON11),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_OTG_CS_SLP_EN_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_OTG_CS_SLP_EN_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_slp_otg_sel(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_PERP_CON11),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_SLP_OTG_SEL_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_SLP_OTG_SEL_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_iterm_sel(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_CORE_CON0),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_ITERM_SEL_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_ITERM_SEL_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_ics_loop(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_CORE_CON0),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_ICS_LOOP_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_ICS_LOOP_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_zxgm_tune(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_CORE_CON1),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_ZXGM_TUNE_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_ZXGM_TUNE_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_chop_en(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_CORE_CON1),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_CHOP_EN_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_CHOP_EN_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_force_non_oc(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_CORE_CON1),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_FORCE_NON_OC_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_FORCE_NON_OC_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_force_non_ov(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_CORE_CON1),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_FORCE_NON_OV_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_FORCE_NON_OV_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_gdri_minoff_en(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_CORE_CON1),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_GDRI_MINOFF_EN_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_GDRI_MINOFF_EN_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_sys_vreftrim(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_CORE_CON2),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_SYS_VREFTRIM_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_SYS_VREFTRIM_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_cs_vreftrim(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_CORE_CON3),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_CS_VREFTRIM_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_CS_VREFTRIM_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_osc_trim(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_CORE_CON4),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_OSC_TRIM_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_OSC_TRIM_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_vsys_ov_trim(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_CORE_CON5),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_VSYS_OV_TRIM_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_VSYS_OV_TRIM_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_swchr_rv2(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_CORE_CON6),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_SWCHR_RV2_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_SWCHR_RV2_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_input_cc_reg(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_CORE_CON7),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_INPUT_CC_REG_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_INPUT_CC_REG_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_otg_chrin_vol(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_CORE_CON7),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_OTG_CHRIN_VOL_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_OTG_CHRIN_VOL_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_force_otg_non_ov(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_CORE_CON7),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_FORCE_OTG_NON_OV_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_FORCE_OTG_NON_OV_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_inout_csreg_sel(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_CORE_CON8),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_INOUT_CSREG_SEL_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_INOUT_CSREG_SEL_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_chopfreq_sel(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_CORE_CON8),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_CHOPFREQ_SEL_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_CHOPFREQ_SEL_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_chgpreg_sel(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_CORE_CON8),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_CHGPREG_SEL_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_CHGPREG_SEL_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_flash_drv_en(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_CORE_CON9),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_FLASH_DRV_EN_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_FLASH_DRV_EN_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_fpwm_otg(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_CORE_CON10),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_FPWM_OTG_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_FPWM_OTG_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_otg_zx_testmode(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_CORE_CON10),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_OTG_ZX_TESTMODE_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_OTG_ZX_TESTMODE_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_swchr_zx_testmode(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_CORE_CON10),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_SWCHR_ZX_TESTMODE_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_SWCHR_ZX_TESTMODE_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_zx_trim(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_CORE_CON11),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_ZX_TRIM_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_ZX_TRIM_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_zx_trim_otg(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_CORE_CON11),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_ZX_TRIM_OTG_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_ZX_TRIM_OTG_SHIFT)
	                         );
  mt6333_unlock();
}

kal_uint8 mt6333_get_rgs_auto_recharge(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_STA_CON0),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RGS_AUTO_RECHARGE_MASK),
                           (kal_uint8)(MT6333_PMIC_RGS_AUTO_RECHARGE_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rgs_charge_complete_hw(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_STA_CON0),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RGS_CHARGE_COMPLETE_HW_MASK),
                           (kal_uint8)(MT6333_PMIC_RGS_CHARGE_COMPLETE_HW_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rgs_pwm_oc_det(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_STA_CON0),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RGS_PWM_OC_DET_MASK),
                           (kal_uint8)(MT6333_PMIC_RGS_PWM_OC_DET_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rgs_vsys_ov_det(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_STA_CON0),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RGS_VSYS_OV_DET_MASK),
                           (kal_uint8)(MT6333_PMIC_RGS_VSYS_OV_DET_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rgs_power_path(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_STA_CON0),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RGS_POWER_PATH_MASK),
                           (kal_uint8)(MT6333_PMIC_RGS_POWER_PATH_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rgs_force_no_pp_config(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_STA_CON0),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RGS_FORCE_NO_PP_CONFIG_MASK),
                           (kal_uint8)(MT6333_PMIC_RGS_FORCE_NO_PP_CONFIG_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rgs_chrg_status(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_STA_CON0),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RGS_CHRG_STATUS_MASK),
                           (kal_uint8)(MT6333_PMIC_RGS_CHRG_STATUS_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rgs_bat_st_recc(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_STA_CON1),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RGS_BAT_ST_RECC_MASK),
                           (kal_uint8)(MT6333_PMIC_RGS_BAT_ST_RECC_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rgs_sys_gt_cv(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_STA_CON1),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RGS_SYS_GT_CV_MASK),
                           (kal_uint8)(MT6333_PMIC_RGS_SYS_GT_CV_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rgs_bat_gt_cc(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_STA_CON1),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RGS_BAT_GT_CC_MASK),
                           (kal_uint8)(MT6333_PMIC_RGS_BAT_GT_CC_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rgs_bat_gt_30(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_STA_CON1),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RGS_BAT_GT_30_MASK),
                           (kal_uint8)(MT6333_PMIC_RGS_BAT_GT_30_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rgs_bat_gt_22(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_STA_CON1),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RGS_BAT_GT_22_MASK),
                           (kal_uint8)(MT6333_PMIC_RGS_BAT_GT_22_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rgs_buck_mode(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_STA_CON2),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RGS_BUCK_MODE_MASK),
                           (kal_uint8)(MT6333_PMIC_RGS_BUCK_MODE_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rgs_buck_precc_mode(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_STA_CON2),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RGS_BUCK_PRECC_MODE_MASK),
                           (kal_uint8)(MT6333_PMIC_RGS_BUCK_PRECC_MODE_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rgs_chrdet(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_STA_CON2),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RGS_CHRDET_MASK),
                           (kal_uint8)(MT6333_PMIC_RGS_CHRDET_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rgs_chr_hv_det(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_STA_CON2),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RGS_CHR_HV_DET_MASK),
                           (kal_uint8)(MT6333_PMIC_RGS_CHR_HV_DET_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rgs_chr_plug_in(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_STA_CON2),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RGS_CHR_PLUG_IN_MASK),
                           (kal_uint8)(MT6333_PMIC_RGS_CHR_PLUG_IN_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rgs_baton_undet(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_STA_CON2),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RGS_BATON_UNDET_MASK),
                           (kal_uint8)(MT6333_PMIC_RGS_BATON_UNDET_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rgs_chrin_lv_det(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_STA_CON2),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RGS_CHRIN_LV_DET_MASK),
                           (kal_uint8)(MT6333_PMIC_RGS_CHRIN_LV_DET_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rgs_chrin_hv_det(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_STA_CON2),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RGS_CHRIN_HV_DET_MASK),
                           (kal_uint8)(MT6333_PMIC_RGS_CHRIN_HV_DET_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rgs_thermal_sd_mode(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_STA_CON3),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RGS_THERMAL_SD_MODE_MASK),
                           (kal_uint8)(MT6333_PMIC_RGS_THERMAL_SD_MODE_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rgs_chr_hv_mode(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_STA_CON3),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RGS_CHR_HV_MODE_MASK),
                           (kal_uint8)(MT6333_PMIC_RGS_CHR_HV_MODE_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rgs_bat_only_mode(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_STA_CON3),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RGS_BAT_ONLY_MODE_MASK),
                           (kal_uint8)(MT6333_PMIC_RGS_BAT_ONLY_MODE_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rgs_chr_suspend_mode(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_STA_CON3),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RGS_CHR_SUSPEND_MODE_MASK),
                           (kal_uint8)(MT6333_PMIC_RGS_CHR_SUSPEND_MODE_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rgs_precc_mode(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_STA_CON3),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RGS_PRECC_MODE_MASK),
                           (kal_uint8)(MT6333_PMIC_RGS_PRECC_MODE_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rgs_cv_mode(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_STA_CON3),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RGS_CV_MODE_MASK),
                           (kal_uint8)(MT6333_PMIC_RGS_CV_MODE_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rgs_cc_mode(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_STA_CON3),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RGS_CC_MODE_MASK),
                           (kal_uint8)(MT6333_PMIC_RGS_CC_MODE_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rgs_ot_reg(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_STA_CON4),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RGS_OT_REG_MASK),
                           (kal_uint8)(MT6333_PMIC_RGS_OT_REG_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rgs_ot_sd(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_STA_CON4),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RGS_OT_SD_MASK),
                           (kal_uint8)(MT6333_PMIC_RGS_OT_SD_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rgs_pwm_bat_config(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_STA_CON4),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RGS_PWM_BAT_CONFIG_MASK),
                           (kal_uint8)(MT6333_PMIC_RGS_PWM_BAT_CONFIG_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rgs_pwm_current_config(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_STA_CON4),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RGS_PWM_CURRENT_CONFIG_MASK),
                           (kal_uint8)(MT6333_PMIC_RGS_PWM_CURRENT_CONFIG_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rgs_pwm_voltage_config(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_STA_CON4),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RGS_PWM_VOLTAGE_CONFIG_MASK),
                           (kal_uint8)(MT6333_PMIC_RGS_PWM_VOLTAGE_CONFIG_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rgs_buck_overload(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_STA_CON5),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RGS_BUCK_OVERLOAD_MASK),
                           (kal_uint8)(MT6333_PMIC_RGS_BUCK_OVERLOAD_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rgs_bat_dppm_mode(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_STA_CON5),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RGS_BAT_DPPM_MODE_MASK),
                           (kal_uint8)(MT6333_PMIC_RGS_BAT_DPPM_MODE_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rgs_adaptive_cv_mode(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_STA_CON5),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RGS_ADAPTIVE_CV_MODE_MASK),
                           (kal_uint8)(MT6333_PMIC_RGS_ADAPTIVE_CV_MODE_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rgs_vin_dpm_mode(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_STA_CON5),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RGS_VIN_DPM_MODE_MASK),
                           (kal_uint8)(MT6333_PMIC_RGS_VIN_DPM_MODE_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rgs_thermal_reg_mode(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_STA_CON5),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RGS_THERMAL_REG_MODE_MASK),
                           (kal_uint8)(MT6333_PMIC_RGS_THERMAL_REG_MODE_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rgs_ich_setting(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_STA_CON6),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RGS_ICH_SETTING_MASK),
                           (kal_uint8)(MT6333_PMIC_RGS_ICH_SETTING_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rgs_cs_sel(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_STA_CON6),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RGS_CS_SEL_MASK),
                           (kal_uint8)(MT6333_PMIC_RGS_CS_SEL_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rgs_syscv_fine_sel(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_STA_CON7),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RGS_SYSCV_FINE_SEL_MASK),
                           (kal_uint8)(MT6333_PMIC_RGS_SYSCV_FINE_SEL_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rgs_oc_sd_sel(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_STA_CON7),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RGS_OC_SD_SEL_MASK),
                           (kal_uint8)(MT6333_PMIC_RGS_OC_SD_SEL_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rgs_pwm_oc_sel(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_STA_CON7),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RGS_PWM_OC_SEL_MASK),
                           (kal_uint8)(MT6333_PMIC_RGS_PWM_OC_SEL_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rgs_chrwdt_tout(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_STA_CON8),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RGS_CHRWDT_TOUT_MASK),
                           (kal_uint8)(MT6333_PMIC_RGS_CHRWDT_TOUT_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rgs_vsys_ov_vth(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_STA_CON8),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RGS_VSYS_OV_VTH_MASK),
                           (kal_uint8)(MT6333_PMIC_RGS_VSYS_OV_VTH_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rgs_syscv_coarse_sel(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_STA_CON8),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RGS_SYSCV_COARSE_SEL_MASK),
                           (kal_uint8)(MT6333_PMIC_RGS_SYSCV_COARSE_SEL_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rgs_usb_dl_key(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_STA_CON9),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RGS_USB_DL_KEY_MASK),
                           (kal_uint8)(MT6333_PMIC_RGS_USB_DL_KEY_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rgs_force_pp_on(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_STA_CON9),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RGS_FORCE_PP_ON_MASK),
                           (kal_uint8)(MT6333_PMIC_RGS_FORCE_PP_ON_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rgs_ini_sys_on(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_STA_CON9),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RGS_INI_SYS_ON_MASK),
                           (kal_uint8)(MT6333_PMIC_RGS_INI_SYS_ON_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rgs_ich_oc_flag_chr_core(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_STA_CON10),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RGS_ICH_OC_FLAG_CHR_CORE_MASK),
                           (kal_uint8)(MT6333_PMIC_RGS_ICH_OC_FLAG_CHR_CORE_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rgs_pwm_oc_chr_core(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_STA_CON10),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RGS_PWM_OC_CHR_CORE_MASK),
                           (kal_uint8)(MT6333_PMIC_RGS_PWM_OC_CHR_CORE_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rgs_power_on_ready(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_STA_CON11),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RGS_POWER_ON_READY_MASK),
                           (kal_uint8)(MT6333_PMIC_RGS_POWER_ON_READY_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rgs_auto_pwron(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_STA_CON11),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RGS_AUTO_PWRON_MASK),
                           (kal_uint8)(MT6333_PMIC_RGS_AUTO_PWRON_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rgs_auto_pwron_done(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_STA_CON11),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RGS_AUTO_PWRON_DONE_MASK),
                           (kal_uint8)(MT6333_PMIC_RGS_AUTO_PWRON_DONE_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rgs_chr_mode(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_STA_CON11),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RGS_CHR_MODE_MASK),
                           (kal_uint8)(MT6333_PMIC_RGS_CHR_MODE_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rgs_otg_mode(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_STA_CON11),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RGS_OTG_MODE_MASK),
                           (kal_uint8)(MT6333_PMIC_RGS_OTG_MODE_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rgs_poseq_done(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_STA_CON11),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RGS_POSEQ_DONE_MASK),
                           (kal_uint8)(MT6333_PMIC_RGS_POSEQ_DONE_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rgs_otg_precc(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_STA_CON11),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RGS_OTG_PRECC_MASK),
                           (kal_uint8)(MT6333_PMIC_RGS_OTG_PRECC_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rgs_chrin_short(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_STA_CON12),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RGS_CHRIN_SHORT_MASK),
                           (kal_uint8)(MT6333_PMIC_RGS_CHRIN_SHORT_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rgs_drvcdt_short(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_STA_CON12),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RGS_DRVCDT_SHORT_MASK),
                           (kal_uint8)(MT6333_PMIC_RGS_DRVCDT_SHORT_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rgs_otg_m3_oc(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_STA_CON12),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RGS_OTG_M3_OC_MASK),
                           (kal_uint8)(MT6333_PMIC_RGS_OTG_M3_OC_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rgs_otg_thermal(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_STA_CON12),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RGS_OTG_THERMAL_MASK),
                           (kal_uint8)(MT6333_PMIC_RGS_OTG_THERMAL_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rgs_chr_in_flash(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_STA_CON12),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RGS_CHR_IN_FLASH_MASK),
                           (kal_uint8)(MT6333_PMIC_RGS_CHR_IN_FLASH_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rgs_vled_short(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_STA_CON12),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RGS_VLED_SHORT_MASK),
                           (kal_uint8)(MT6333_PMIC_RGS_VLED_SHORT_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rgs_vled_open(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_STA_CON12),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RGS_VLED_OPEN_MASK),
                           (kal_uint8)(MT6333_PMIC_RGS_VLED_OPEN_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rgs_flash_en_timeout(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_STA_CON12),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RGS_FLASH_EN_TIMEOUT_MASK),
                           (kal_uint8)(MT6333_PMIC_RGS_FLASH_EN_TIMEOUT_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rgs_chr_oc(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_STA_CON13),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RGS_CHR_OC_MASK),
                           (kal_uint8)(MT6333_PMIC_RGS_CHR_OC_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rgs_pwm_en(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_STA_CON13),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RGS_PWM_EN_MASK),
                           (kal_uint8)(MT6333_PMIC_RGS_PWM_EN_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rgs_otg_en(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_STA_CON13),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RGS_OTG_EN_MASK),
                           (kal_uint8)(MT6333_PMIC_RGS_OTG_EN_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rgs_otg_en_stb(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_STA_CON13),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RGS_OTG_EN_STB_MASK),
                           (kal_uint8)(MT6333_PMIC_RGS_OTG_EN_STB_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rgs_otg_drv_en(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_STA_CON13),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RGS_OTG_DRV_EN_MASK),
                           (kal_uint8)(MT6333_PMIC_RGS_OTG_DRV_EN_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rgs_flash_en(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_STA_CON13),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RGS_FLASH_EN_MASK),
                           (kal_uint8)(MT6333_PMIC_RGS_FLASH_EN_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rgs_m3_boost_en(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_STA_CON14),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RGS_M3_BOOST_EN_MASK),
                           (kal_uint8)(MT6333_PMIC_RGS_M3_BOOST_EN_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rgs_m3_r_en(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_STA_CON14),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RGS_M3_R_EN_MASK),
                           (kal_uint8)(MT6333_PMIC_RGS_M3_R_EN_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rgs_m3_s_en(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_STA_CON14),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RGS_M3_S_EN_MASK),
                           (kal_uint8)(MT6333_PMIC_RGS_M3_S_EN_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rgs_m3_en(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_STA_CON14),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RGS_M3_EN_MASK),
                           (kal_uint8)(MT6333_PMIC_RGS_M3_EN_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rgs_cpcstsys_en(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_STA_CON14),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RGS_CPCSTSYS_EN_MASK),
                           (kal_uint8)(MT6333_PMIC_RGS_CPCSTSYS_EN_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rgs_sw_gate_ctrl(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_STA_CON14),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RGS_SW_GATE_CTRL_MASK),
                           (kal_uint8)(MT6333_PMIC_RGS_SW_GATE_CTRL_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_qi_otg_chr_gt_lv(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_STA_CON14),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_QI_OTG_CHR_GT_LV_MASK),
                           (kal_uint8)(MT6333_PMIC_QI_OTG_CHR_GT_LV_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rgs_thermal_rg_th(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_STA_CON15),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RGS_THERMAL_RG_TH_MASK),
                           (kal_uint8)(MT6333_PMIC_RGS_THERMAL_RG_TH_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rgs_otg_oc_th(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_STA_CON15),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RGS_OTG_OC_TH_MASK),
                           (kal_uint8)(MT6333_PMIC_RGS_OTG_OC_TH_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

void mt6333_set_rg_chr_suspend(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_DIG_CON0),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_CHR_SUSPEND_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_CHR_SUSPEND_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_sys_on(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_DIG_CON0),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_SYS_ON_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_SYS_ON_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_sys_unstable(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_DIG_CON0),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_SYS_UNSTABLE_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_SYS_UNSTABLE_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_skip_efuse_out(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_DIG_CON1),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_SKIP_EFUSE_OUT_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_SKIP_EFUSE_OUT_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_vsys_sel(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_DIG_CON1),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_VSYS_SEL_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_VSYS_SEL_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_cv_sel(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_DIG_CON1),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_CV_SEL_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_CV_SEL_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_ich_sel(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_DIG_CON2),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_ICH_SEL_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_ICH_SEL_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_ich_pre_sel(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_DIG_CON2),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_ICH_PRE_SEL_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_ICH_PRE_SEL_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_oc_sel(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_DIG_CON3),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_OC_SEL_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_OC_SEL_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_chrin_lv_vth(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_DIG_CON3),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_CHRIN_LV_VTH_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_CHRIN_LV_VTH_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_chrin_hv_vth(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_DIG_CON3),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_CHRIN_HV_VTH_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_CHRIN_HV_VTH_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_usbdl_ext(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_DIG_CON4),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_USBDL_EXT_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_USBDL_EXT_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_usbdl_mode_b(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_DIG_CON4),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_USBDL_MODE_B_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_USBDL_MODE_B_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_usbdl_oc_sel(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_DIG_CON4),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_USBDL_OC_SEL_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_USBDL_OC_SEL_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_buck_overload_prot_en(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_DIG_CON5),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_BUCK_OVERLOAD_PROT_EN_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_BUCK_OVERLOAD_PROT_EN_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_ch_complete_auto_off(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_DIG_CON5),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_CH_COMPLETE_AUTO_OFF_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_CH_COMPLETE_AUTO_OFF_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_term_timer(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_DIG_CON5),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_TERM_TIMER_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_TERM_TIMER_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_chr_oc_auto_off(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_DIG_CON6),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_CHR_OC_AUTO_OFF_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_CHR_OC_AUTO_OFF_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_chr_oc_reset(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_DIG_CON6),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_CHR_OC_RESET_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_CHR_OC_RESET_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_otg_m3_oc_auto_off(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_DIG_CON6),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_OTG_M3_OC_AUTO_OFF_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_OTG_M3_OC_AUTO_OFF_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_otg_en(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_DIG_CON7),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_OTG_EN_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_OTG_EN_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_flash_en(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_DIG_CON8),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_FLASH_EN_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_FLASH_EN_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_flash_pwm_en(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_DIG_CON9),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_FLASH_PWM_EN_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_FLASH_PWM_EN_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_flash_pwm_en_stb(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_DIG_CON9),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_FLASH_PWM_EN_STB_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_FLASH_PWM_EN_STB_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_torch_mode(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_DIG_CON9),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_TORCH_MODE_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_TORCH_MODE_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_torch_chrin_chk(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_DIG_CON9),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_TORCH_CHRIN_CHK_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_TORCH_CHRIN_CHK_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_flash_dim_duty(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_DIG_CON10),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_FLASH_DIM_DUTY_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_FLASH_DIM_DUTY_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_chk_chrin_time_ext(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_DIG_CON10),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_CHK_CHRIN_TIME_EXT_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_CHK_CHRIN_TIME_EXT_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_flash_dim_fsel(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_DIG_CON11),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_FLASH_DIM_FSEL_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_FLASH_DIM_FSEL_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_flash_iset(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_DIG_CON12),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_FLASH_ISET_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_FLASH_ISET_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_flash_iset_step(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_DIG_CON12),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_FLASH_ISET_STEP_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_FLASH_ISET_STEP_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_thermal_rg_th(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_DIG_CON13),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_THERMAL_RG_TH_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_THERMAL_RG_TH_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_thermal_temp_sel(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_DIG_CON14),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_THERMAL_TEMP_SEL_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_THERMAL_TEMP_SEL_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_thermal_checker_sel(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_DIG_CON14),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_THERMAL_CHECKER_SEL_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_THERMAL_CHECKER_SEL_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_flash_en_timeout_sel(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_DIG_CON14),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_FLASH_EN_TIMEOUT_SEL_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_FLASH_EN_TIMEOUT_SEL_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_otg_oc_th(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_DIG_CON15),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_OTG_OC_TH_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_OTG_OC_TH_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_reserve_v0(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_DIG_CON15),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_RESERVE_V0_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_RESERVE_V0_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_cv_sel_usbdl(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_DIG_CON16),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_CV_SEL_USBDL_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_CV_SEL_USBDL_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_ov_sel_usbdl(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_DIG_CON16),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_OV_SEL_USBDL_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_OV_SEL_USBDL_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_sw_gate_ctrl(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_DIG_CON17),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_SW_GATE_CTRL_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_SW_GATE_CTRL_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_reserve_v1(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_DIG_CON17),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_RESERVE_V1_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_RESERVE_V1_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_reserve_v2(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_DIG_CON18),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_RESERVE_V2_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_RESERVE_V2_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_i2c_config(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_GPIO_CON0),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_I2C_CONFIG_MASK),
                             (kal_uint8)(MT6333_PMIC_I2C_CONFIG_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_i2c_deg_en(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_GPIO_CON0),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_I2C_DEG_EN_MASK),
                             (kal_uint8)(MT6333_PMIC_I2C_DEG_EN_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_sda_mode(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_GPIO_CON1),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_SDA_MODE_MASK),
                             (kal_uint8)(MT6333_PMIC_SDA_MODE_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_sda_oe(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_GPIO_CON1),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_SDA_OE_MASK),
                             (kal_uint8)(MT6333_PMIC_SDA_OE_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_sda_out(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_GPIO_CON1),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_SDA_OUT_MASK),
                             (kal_uint8)(MT6333_PMIC_SDA_OUT_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_scl_mode(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_GPIO_CON1),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_SCL_MODE_MASK),
                             (kal_uint8)(MT6333_PMIC_SCL_MODE_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_scl_oe(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_GPIO_CON1),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_SCL_OE_MASK),
                             (kal_uint8)(MT6333_PMIC_SCL_OE_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_scl_out(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_GPIO_CON1),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_SCL_OUT_MASK),
                             (kal_uint8)(MT6333_PMIC_SCL_OUT_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_int_mode(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_GPIO_CON2),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_INT_MODE_MASK),
                             (kal_uint8)(MT6333_PMIC_INT_MODE_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_int_oe(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_GPIO_CON2),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_INT_OE_MASK),
                             (kal_uint8)(MT6333_PMIC_INT_OE_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_int_out(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_GPIO_CON2),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_INT_OUT_MASK),
                             (kal_uint8)(MT6333_PMIC_INT_OUT_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_chr_250k_ck_en(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_CLK_CON0),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_CHR_250K_CK_EN_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_CHR_250K_CK_EN_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_chr_1m_ck_en(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_CLK_CON0),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_CHR_1M_CK_EN_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_CHR_1M_CK_EN_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_chr_pwm_ck_en(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_CLK_CON0),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_CHR_PWM_CK_EN_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_CHR_PWM_CK_EN_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_buck_1m_ck_en(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_CLK_CON0),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_BUCK_1M_CK_EN_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_BUCK_1M_CK_EN_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_buck_2m_ck_en(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_CLK_CON0),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_BUCK_2M_CK_EN_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_BUCK_2M_CK_EN_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_buck_3m_ck_en(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_CLK_CON0),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_BUCK_3M_CK_EN_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_BUCK_3M_CK_EN_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_buck_6m_ck_en(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_CLK_CON0),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_BUCK_6M_CK_EN_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_BUCK_6M_CK_EN_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_buck_osc_en(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_CLK_CON1),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_BUCK_OSC_EN_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_BUCK_OSC_EN_SHIFT)
	                         );
  mt6333_unlock();
}

kal_uint8 mt6333_get_qi_osc_en(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_CLK_CON1),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_QI_OSC_EN_MASK),
                           (kal_uint8)(MT6333_PMIC_QI_OSC_EN_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

void mt6333_set_rg_buck_cali_32k_ck_en(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_CLK_CON2),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_BUCK_CALI_32K_CK_EN_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_BUCK_CALI_32K_CK_EN_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_buck_cali_pwm_ck_en(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_CLK_CON2),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_BUCK_CALI_PWM_CK_EN_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_BUCK_CALI_PWM_CK_EN_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_buck_cali_6m_ck_en(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_CLK_CON2),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_BUCK_CALI_6M_CK_EN_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_BUCK_CALI_6M_CK_EN_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_test_efuse(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_CLK_CON2),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_TEST_EFUSE_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_TEST_EFUSE_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_test_ni_ck(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_CLK_CON2),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_TEST_NI_CK_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_TEST_NI_CK_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_test_smps_ck(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_CLK_CON2),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_TEST_SMPS_CK_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_TEST_SMPS_CK_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_test_pwm_ck(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_CLK_CON2),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_TEST_PWM_CK_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_TEST_PWM_CK_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_int_en_chr_complete(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_INT_CON0),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_INT_EN_CHR_COMPLETE_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_INT_EN_CHR_COMPLETE_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_int_en_thermal_sd(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_INT_CON0),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_INT_EN_THERMAL_SD_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_INT_EN_THERMAL_SD_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_int_en_thermal_reg_in(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_INT_CON0),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_INT_EN_THERMAL_REG_IN_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_INT_EN_THERMAL_REG_IN_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_int_en_thermal_reg_out(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_INT_CON0),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_INT_EN_THERMAL_REG_OUT_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_INT_EN_THERMAL_REG_OUT_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_int_en_otg_oc(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_INT_CON0),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_INT_EN_OTG_OC_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_INT_EN_OTG_OC_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_int_en_otg_thermal(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_INT_CON0),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_INT_EN_OTG_THERMAL_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_INT_EN_OTG_THERMAL_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_int_en_otg_chrin_short(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_INT_CON0),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_INT_EN_OTG_CHRIN_SHORT_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_INT_EN_OTG_CHRIN_SHORT_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_int_en_otg_drvcdt_short(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_INT_CON0),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_INT_EN_OTG_DRVCDT_SHORT_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_INT_EN_OTG_DRVCDT_SHORT_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_int_en_chrwdt_flag(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_INT_CON1),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_INT_EN_CHRWDT_FLAG_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_INT_EN_CHRWDT_FLAG_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_int_en_buck_vcore_oc(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_INT_CON1),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_INT_EN_BUCK_VCORE_OC_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_INT_EN_BUCK_VCORE_OC_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_int_en_buck_vmem_oc(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_INT_CON1),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_INT_EN_BUCK_VMEM_OC_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_INT_EN_BUCK_VMEM_OC_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_int_en_buck_vrf18_oc(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_INT_CON1),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_INT_EN_BUCK_VRF18_OC_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_INT_EN_BUCK_VRF18_OC_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_int_en_buck_thermal(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_INT_CON1),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_INT_EN_BUCK_THERMAL_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_INT_EN_BUCK_THERMAL_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_int_en_flash_en_timeout(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_INT_CON1),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_INT_EN_FLASH_EN_TIMEOUT_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_INT_EN_FLASH_EN_TIMEOUT_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_int_en_flash_vled_short(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_INT_CON1),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_INT_EN_FLASH_VLED_SHORT_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_INT_EN_FLASH_VLED_SHORT_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_int_en_flash_vled_open(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_INT_CON1),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_INT_EN_FLASH_VLED_OPEN_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_INT_EN_FLASH_VLED_OPEN_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_int_en_chr_oc(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_INT_CON2),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_INT_EN_CHR_OC_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_INT_EN_CHR_OC_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_int_en_chr_plug_in_flash(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_INT_CON2),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_INT_EN_CHR_PLUG_IN_FLASH_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_INT_EN_CHR_PLUG_IN_FLASH_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_chrwdt_en(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_CHRWDT_CON0),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_CHRWDT_EN_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_CHRWDT_EN_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_chrwdt_td(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_CHRWDT_CON0),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_CHRWDT_TD_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_CHRWDT_TD_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_chrwdt_wr(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_CHRWDT_CON0),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_CHRWDT_WR_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_CHRWDT_WR_SHIFT)
	                         );
  mt6333_unlock();
}

kal_uint8 mt6333_get_rg_chrwdt_flag(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_CHRWDT_STATUS0),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RG_CHRWDT_FLAG_MASK),
                           (kal_uint8)(MT6333_PMIC_RG_CHRWDT_FLAG_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rg_int_status_chr_complete(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_INT_STATUS0),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RG_INT_STATUS_CHR_COMPLETE_MASK),
                           (kal_uint8)(MT6333_PMIC_RG_INT_STATUS_CHR_COMPLETE_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rg_int_status_thermal_sd(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_INT_STATUS0),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RG_INT_STATUS_THERMAL_SD_MASK),
                           (kal_uint8)(MT6333_PMIC_RG_INT_STATUS_THERMAL_SD_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rg_int_status_thermal_reg_in(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_INT_STATUS0),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RG_INT_STATUS_THERMAL_REG_IN_MASK),
                           (kal_uint8)(MT6333_PMIC_RG_INT_STATUS_THERMAL_REG_IN_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rg_int_status_thermal_reg_out(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_INT_STATUS0),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RG_INT_STATUS_THERMAL_REG_OUT_MASK),
                           (kal_uint8)(MT6333_PMIC_RG_INT_STATUS_THERMAL_REG_OUT_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rg_int_status_otg_oc(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_INT_STATUS0),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RG_INT_STATUS_OTG_OC_MASK),
                           (kal_uint8)(MT6333_PMIC_RG_INT_STATUS_OTG_OC_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rg_int_status_otg_thermal(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_INT_STATUS0),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RG_INT_STATUS_OTG_THERMAL_MASK),
                           (kal_uint8)(MT6333_PMIC_RG_INT_STATUS_OTG_THERMAL_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rg_int_status_otg_chrin_short(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_INT_STATUS0),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RG_INT_STATUS_OTG_CHRIN_SHORT_MASK),
                           (kal_uint8)(MT6333_PMIC_RG_INT_STATUS_OTG_CHRIN_SHORT_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rg_int_status_otg_drvcdt_short(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_INT_STATUS0),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RG_INT_STATUS_OTG_DRVCDT_SHORT_MASK),
                           (kal_uint8)(MT6333_PMIC_RG_INT_STATUS_OTG_DRVCDT_SHORT_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rg_int_status_chrwdt_flag(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_INT_STATUS1),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RG_INT_STATUS_CHRWDT_FLAG_MASK),
                           (kal_uint8)(MT6333_PMIC_RG_INT_STATUS_CHRWDT_FLAG_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rg_int_status_buck_vcore_oc(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_INT_STATUS1),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RG_INT_STATUS_BUCK_VCORE_OC_MASK),
                           (kal_uint8)(MT6333_PMIC_RG_INT_STATUS_BUCK_VCORE_OC_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rg_int_status_buck_vmem_oc(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_INT_STATUS1),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RG_INT_STATUS_BUCK_VMEM_OC_MASK),
                           (kal_uint8)(MT6333_PMIC_RG_INT_STATUS_BUCK_VMEM_OC_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rg_int_status_buck_vrf18_oc(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_INT_STATUS1),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RG_INT_STATUS_BUCK_VRF18_OC_MASK),
                           (kal_uint8)(MT6333_PMIC_RG_INT_STATUS_BUCK_VRF18_OC_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rg_int_status_buck_thermal(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_INT_STATUS1),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RG_INT_STATUS_BUCK_THERMAL_MASK),
                           (kal_uint8)(MT6333_PMIC_RG_INT_STATUS_BUCK_THERMAL_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rg_int_status_flash_en_timeout(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_INT_STATUS1),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RG_INT_STATUS_FLASH_EN_TIMEOUT_MASK),
                           (kal_uint8)(MT6333_PMIC_RG_INT_STATUS_FLASH_EN_TIMEOUT_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rg_int_status_flash_vled_short(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_INT_STATUS1),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RG_INT_STATUS_FLASH_VLED_SHORT_MASK),
                           (kal_uint8)(MT6333_PMIC_RG_INT_STATUS_FLASH_VLED_SHORT_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rg_int_status_flash_vled_open(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_INT_STATUS1),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RG_INT_STATUS_FLASH_VLED_OPEN_MASK),
                           (kal_uint8)(MT6333_PMIC_RG_INT_STATUS_FLASH_VLED_OPEN_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rg_int_status_chr_oc(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_INT_STATUS2),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RG_INT_STATUS_CHR_OC_MASK),
                           (kal_uint8)(MT6333_PMIC_RG_INT_STATUS_CHR_OC_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rg_int_status_chr_plug_in_flash(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_INT_STATUS2),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RG_INT_STATUS_CHR_PLUG_IN_FLASH_MASK),
                           (kal_uint8)(MT6333_PMIC_RG_INT_STATUS_CHR_PLUG_IN_FLASH_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

void mt6333_set_vcore_deg_en(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_OC_CTL_VCORE),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_VCORE_DEG_EN_MASK),
                             (kal_uint8)(MT6333_PMIC_VCORE_DEG_EN_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_vcore_oc_wnd(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_OC_CTL_VCORE),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_VCORE_OC_WND_MASK),
                             (kal_uint8)(MT6333_PMIC_VCORE_OC_WND_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_vcore_oc_thd(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_OC_CTL_VCORE),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_VCORE_OC_THD_MASK),
                             (kal_uint8)(MT6333_PMIC_VCORE_OC_THD_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_vmem_deg_en(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_OC_CTL_VMEM),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_VMEM_DEG_EN_MASK),
                             (kal_uint8)(MT6333_PMIC_VMEM_DEG_EN_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_vmem_oc_wnd(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_OC_CTL_VMEM),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_VMEM_OC_WND_MASK),
                             (kal_uint8)(MT6333_PMIC_VMEM_OC_WND_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_vmem_oc_thd(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_OC_CTL_VMEM),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_VMEM_OC_THD_MASK),
                             (kal_uint8)(MT6333_PMIC_VMEM_OC_THD_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_vrf18_deg_en(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_OC_CTL_VRF18),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_VRF18_DEG_EN_MASK),
                             (kal_uint8)(MT6333_PMIC_VRF18_DEG_EN_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_vrf18_oc_wnd(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_OC_CTL_VRF18),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_VRF18_OC_WND_MASK),
                             (kal_uint8)(MT6333_PMIC_VRF18_OC_WND_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_vrf18_oc_thd(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_OC_CTL_VRF18),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_VRF18_OC_THD_MASK),
                             (kal_uint8)(MT6333_PMIC_VRF18_OC_THD_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_int_polarity(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_INT_MISC_CON),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_INT_POLARITY_MASK),
                             (kal_uint8)(MT6333_PMIC_INT_POLARITY_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_smps_testmode_b(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_BUCK_CON0),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_SMPS_TESTMODE_B_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_SMPS_TESTMODE_B_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_vcore_triml(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VCORE_CON0),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_VCORE_TRIML_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_VCORE_TRIML_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_vcore_trimh(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VCORE_CON0),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_VCORE_TRIMH_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_VCORE_TRIMH_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_vcore_cc(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VCORE_CON1),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_VCORE_CC_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_VCORE_CC_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_vcore_rzsel(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VCORE_CON1),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_VCORE_RZSEL_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_VCORE_RZSEL_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_vcore_slp(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VCORE_CON2),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_VCORE_SLP_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_VCORE_SLP_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_vcore_csl(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VCORE_CON2),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_VCORE_CSL_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_VCORE_CSL_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_vcore_csr(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VCORE_CON2),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_VCORE_CSR_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_VCORE_CSR_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_vcore_avp_os(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VCORE_CON3),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_VCORE_AVP_OS_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_VCORE_AVP_OS_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_vcore_avp_en(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VCORE_CON3),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_VCORE_AVP_EN_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_VCORE_AVP_EN_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_vcore_ndis_en(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VCORE_CON3),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_VCORE_NDIS_EN_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_VCORE_NDIS_EN_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_vcore_modeset(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VCORE_CON3),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_VCORE_MODESET_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_VCORE_MODESET_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_vcore_zx_os(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VCORE_CON3),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_VCORE_ZX_OS_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_VCORE_ZX_OS_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_vcore_csm(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VCORE_CON4),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_VCORE_CSM_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_VCORE_CSM_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_vcore_zxos_trim(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VCORE_CON5),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_VCORE_ZXOS_TRIM_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_VCORE_ZXOS_TRIM_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_vcore_rsv(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VCORE_CON6),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_VCORE_RSV_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_VCORE_RSV_SHIFT)
	                         );
  mt6333_unlock();
}

kal_uint8 mt6333_get_qi_vcore_dig_mon(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_VCORE_CON7),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_QI_VCORE_DIG_MON_MASK),
                           (kal_uint8)(MT6333_PMIC_QI_VCORE_DIG_MON_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_qi_vcore_oc_status(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_VCORE_CON7),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_QI_VCORE_OC_STATUS_MASK),
                           (kal_uint8)(MT6333_PMIC_QI_VCORE_OC_STATUS_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

void mt6333_set_vsleep_src1(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VCORE_CON8),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_VSLEEP_SRC1_MASK),
                             (kal_uint8)(MT6333_PMIC_VSLEEP_SRC1_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_vsleep_src0_7_0(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VCORE_CON9),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_VSLEEP_SRC0_7_0_MASK),
                             (kal_uint8)(MT6333_PMIC_VSLEEP_SRC0_7_0_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_vsleep_src0_8(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VCORE_CON10),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_VSLEEP_SRC0_8_MASK),
                             (kal_uint8)(MT6333_PMIC_VSLEEP_SRC0_8_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_r2r_src0_8(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VCORE_CON11),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_R2R_SRC0_8_MASK),
                             (kal_uint8)(MT6333_PMIC_R2R_SRC0_8_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_r2r_src1(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VCORE_CON11),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_R2R_SRC1_MASK),
                             (kal_uint8)(MT6333_PMIC_R2R_SRC1_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_r2r_src0_7_0(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VCORE_CON12),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_R2R_SRC0_7_0_MASK),
                             (kal_uint8)(MT6333_PMIC_R2R_SRC0_7_0_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_srclken_dly_src1(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VCORE_CON13),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_SRCLKEN_DLY_SRC1_MASK),
                             (kal_uint8)(MT6333_PMIC_SRCLKEN_DLY_SRC1_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_buck_rsv0(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VCORE_CON14),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_BUCK_RSV0_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_BUCK_RSV0_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_srclken_dly_sel(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VCORE_CON15),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_SRCLKEN_DLY_SEL_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_SRCLKEN_DLY_SEL_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_r2r_event_sel(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VCORE_CON15),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_R2R_EVENT_SEL_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_R2R_EVENT_SEL_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_qi_vcore_vsleep(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VCORE_CON16),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_QI_VCORE_VSLEEP_MASK),
                             (kal_uint8)(MT6333_PMIC_QI_VCORE_VSLEEP_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_vcore_en(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VCORE_CON17),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_VCORE_EN_MASK),
                             (kal_uint8)(MT6333_PMIC_VCORE_EN_SHIFT)
	                         );
  mt6333_unlock();
}

kal_uint8 mt6333_get_qi_vcore_stb(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_VCORE_CON17),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_QI_VCORE_STB_MASK),
                           (kal_uint8)(MT6333_PMIC_QI_VCORE_STB_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_qi_vcore_en(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_VCORE_CON17),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_QI_VCORE_EN_MASK),
                           (kal_uint8)(MT6333_PMIC_QI_VCORE_EN_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

void mt6333_set_vcore_en_ctrl(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VCORE_CON17),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_VCORE_EN_CTRL_MASK),
                             (kal_uint8)(MT6333_PMIC_VCORE_EN_CTRL_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_vcore_vosel_ctrl(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VCORE_CON17),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_VCORE_VOSEL_CTRL_MASK),
                             (kal_uint8)(MT6333_PMIC_VCORE_VOSEL_CTRL_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_vcore_dlc_ctrl(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VCORE_CON17),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_VCORE_DLC_CTRL_MASK),
                             (kal_uint8)(MT6333_PMIC_VCORE_DLC_CTRL_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_vcore_burst_ctrl(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VCORE_CON17),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_VCORE_BURST_CTRL_MASK),
                             (kal_uint8)(MT6333_PMIC_VCORE_BURST_CTRL_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_vcore_sfchg_ren(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VCORE_CON18),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_VCORE_SFCHG_REN_MASK),
                             (kal_uint8)(MT6333_PMIC_VCORE_SFCHG_REN_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_vcore_sfchg_rrate(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VCORE_CON18),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_VCORE_SFCHG_RRATE_MASK),
                             (kal_uint8)(MT6333_PMIC_VCORE_SFCHG_RRATE_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_vcore_sfchg_fen(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VCORE_CON19),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_VCORE_SFCHG_FEN_MASK),
                             (kal_uint8)(MT6333_PMIC_VCORE_SFCHG_FEN_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_vcore_sfchg_frate(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VCORE_CON19),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_VCORE_SFCHG_FRATE_MASK),
                             (kal_uint8)(MT6333_PMIC_VCORE_SFCHG_FRATE_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_vcore_vosel(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VCORE_CON20),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_VCORE_VOSEL_MASK),
                             (kal_uint8)(MT6333_PMIC_VCORE_VOSEL_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_vcore_vosel_on(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VCORE_CON21),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_VCORE_VOSEL_ON_MASK),
                             (kal_uint8)(MT6333_PMIC_VCORE_VOSEL_ON_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_vcore_vosel_sleep(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VCORE_CON22),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_VCORE_VOSEL_SLEEP_MASK),
                             (kal_uint8)(MT6333_PMIC_VCORE_VOSEL_SLEEP_SHIFT)
	                         );
  mt6333_unlock();
}

kal_uint8 mt6333_get_ni_vcore_vosel(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_VCORE_CON23),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_NI_VCORE_VOSEL_MASK),
                           (kal_uint8)(MT6333_PMIC_NI_VCORE_VOSEL_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

void mt6333_set_vcore_burst(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VCORE_CON24),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_VCORE_BURST_MASK),
                             (kal_uint8)(MT6333_PMIC_VCORE_BURST_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_vcore_burst_on(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VCORE_CON24),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_VCORE_BURST_ON_MASK),
                             (kal_uint8)(MT6333_PMIC_VCORE_BURST_ON_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_vcore_burst_sleep(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VCORE_CON24),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_VCORE_BURST_SLEEP_MASK),
                             (kal_uint8)(MT6333_PMIC_VCORE_BURST_SLEEP_SHIFT)
	                         );
  mt6333_unlock();
}

kal_uint8 mt6333_get_qi_vcore_burst(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_VCORE_CON24),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_QI_VCORE_BURST_MASK),
                           (kal_uint8)(MT6333_PMIC_QI_VCORE_BURST_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

void mt6333_set_vcore_dlc(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VCORE_CON25),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_VCORE_DLC_MASK),
                             (kal_uint8)(MT6333_PMIC_VCORE_DLC_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_vcore_dlc_on(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VCORE_CON25),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_VCORE_DLC_ON_MASK),
                             (kal_uint8)(MT6333_PMIC_VCORE_DLC_ON_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_vcore_dlc_sleep(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VCORE_CON25),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_VCORE_DLC_SLEEP_MASK),
                             (kal_uint8)(MT6333_PMIC_VCORE_DLC_SLEEP_SHIFT)
	                         );
  mt6333_unlock();
}

kal_uint8 mt6333_get_qi_vcore_dlc(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_VCORE_CON25),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_QI_VCORE_DLC_MASK),
                           (kal_uint8)(MT6333_PMIC_QI_VCORE_DLC_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

void mt6333_set_vcore_dlc_n(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VCORE_CON26),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_VCORE_DLC_N_MASK),
                             (kal_uint8)(MT6333_PMIC_VCORE_DLC_N_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_vcore_dlc_n_on(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VCORE_CON26),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_VCORE_DLC_N_ON_MASK),
                             (kal_uint8)(MT6333_PMIC_VCORE_DLC_N_ON_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_vcore_dlc_n_sleep(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VCORE_CON26),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_VCORE_DLC_N_SLEEP_MASK),
                             (kal_uint8)(MT6333_PMIC_VCORE_DLC_N_SLEEP_SHIFT)
	                         );
  mt6333_unlock();
}

kal_uint8 mt6333_get_qi_vcore_dlc_n(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_VCORE_CON26),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_QI_VCORE_DLC_N_MASK),
                           (kal_uint8)(MT6333_PMIC_QI_VCORE_DLC_N_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

void mt6333_set_vcore_vsleep_en(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VCORE_CON27),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_VCORE_VSLEEP_EN_MASK),
                             (kal_uint8)(MT6333_PMIC_VCORE_VSLEEP_EN_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_vcore_r2r_pdn(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VCORE_CON27),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_VCORE_R2R_PDN_MASK),
                             (kal_uint8)(MT6333_PMIC_VCORE_R2R_PDN_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_vcore_vsleep_sel(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VCORE_CON27),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_VCORE_VSLEEP_SEL_MASK),
                             (kal_uint8)(MT6333_PMIC_VCORE_VSLEEP_SEL_SHIFT)
	                         );
  mt6333_unlock();
}

kal_uint8 mt6333_get_ni_vcore_r2r_pdn(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_VCORE_CON27),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_NI_VCORE_R2R_PDN_MASK),
                           (kal_uint8)(MT6333_PMIC_NI_VCORE_R2R_PDN_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_ni_vcore_vsleep_sel(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_VCORE_CON27),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_NI_VCORE_VSLEEP_SEL_MASK),
                           (kal_uint8)(MT6333_PMIC_NI_VCORE_VSLEEP_SEL_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

void mt6333_set_vcore_transtd(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VCORE_CON28),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_VCORE_TRANSTD_MASK),
                             (kal_uint8)(MT6333_PMIC_VCORE_TRANSTD_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_vcore_vosel_trans_en(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VCORE_CON28),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_VCORE_VOSEL_TRANS_EN_MASK),
                             (kal_uint8)(MT6333_PMIC_VCORE_VOSEL_TRANS_EN_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_vcore_vosel_trans_once(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VCORE_CON28),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_VCORE_VOSEL_TRANS_ONCE_MASK),
                             (kal_uint8)(MT6333_PMIC_VCORE_VOSEL_TRANS_ONCE_SHIFT)
	                         );
  mt6333_unlock();
}

kal_uint8 mt6333_get_ni_vcore_vosel_trans(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_VCORE_CON28),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_NI_VCORE_VOSEL_TRANS_MASK),
                           (kal_uint8)(MT6333_PMIC_NI_VCORE_VOSEL_TRANS_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

void mt6333_set_vcore_sfchg_fen_sleep(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VCORE_CON29),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_VCORE_SFCHG_FEN_SLEEP_MASK),
                             (kal_uint8)(MT6333_PMIC_VCORE_SFCHG_FEN_SLEEP_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_vcore_sfchg_ren_sleep(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VCORE_CON29),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_VCORE_SFCHG_REN_SLEEP_MASK),
                             (kal_uint8)(MT6333_PMIC_VCORE_SFCHG_REN_SLEEP_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_vcore_vosel_on_spm(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VCORE_CON30),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_VCORE_VOSEL_ON_SPM_MASK),
                             (kal_uint8)(MT6333_PMIC_VCORE_VOSEL_ON_SPM_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_vcore_en_oc_sdn_sel(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VMEM_RSV0),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_VCORE_EN_OC_SDN_SEL_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_VCORE_EN_OC_SDN_SEL_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_vmem_en_oc_sdn_sel(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VMEM_RSV0),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_VMEM_EN_OC_SDN_SEL_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_VMEM_EN_OC_SDN_SEL_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_vcore_en_thr_sdn_sel(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VMEM_RSV0),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_VCORE_EN_THR_SDN_SEL_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_VCORE_EN_THR_SDN_SEL_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_vmem_rsv1_5_0_3(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VMEM_RSV0),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_VMEM_RSV1_5_0_3_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_VMEM_RSV1_5_0_3_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_vmem_rsv1_5_4(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VMEM_RSV0),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_VMEM_RSV1_5_4_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_VMEM_RSV1_5_4_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_r2r_event_sync_sel(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VMEM_RSV0),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_R2R_EVENT_SYNC_SEL_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_R2R_EVENT_SYNC_SEL_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_vmem_en_thr_sdn_sel(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VMEM_RSV0),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_VMEM_EN_THR_SDN_SEL_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_VMEM_EN_THR_SDN_SEL_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_vmem_triml(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VMEM_CON0),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_VMEM_TRIML_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_VMEM_TRIML_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_vmem_trimh(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VMEM_CON0),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_VMEM_TRIMH_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_VMEM_TRIMH_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_vmem_cc(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VMEM_CON1),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_VMEM_CC_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_VMEM_CC_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_vmem_rzsel(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VMEM_CON1),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_VMEM_RZSEL_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_VMEM_RZSEL_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_vmem_slp(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VMEM_CON2),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_VMEM_SLP_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_VMEM_SLP_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_vmem_csl(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VMEM_CON2),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_VMEM_CSL_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_VMEM_CSL_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_vmem_csr(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VMEM_CON2),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_VMEM_CSR_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_VMEM_CSR_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_vmem_avp_os(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VMEM_CON3),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_VMEM_AVP_OS_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_VMEM_AVP_OS_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_vmem_avp_en(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VMEM_CON3),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_VMEM_AVP_EN_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_VMEM_AVP_EN_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_vmem_ndis_en(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VMEM_CON3),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_VMEM_NDIS_EN_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_VMEM_NDIS_EN_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_vmem_modeset(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VMEM_CON3),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_VMEM_MODESET_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_VMEM_MODESET_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_vmem_zx_os(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VMEM_CON3),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_VMEM_ZX_OS_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_VMEM_ZX_OS_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_vmem_csm(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VMEM_CON4),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_VMEM_CSM_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_VMEM_CSM_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_vmem_zxos_trim(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VMEM_CON5),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_VMEM_ZXOS_TRIM_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_VMEM_ZXOS_TRIM_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_vmem_rsv(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VMEM_CON6),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_VMEM_RSV_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_VMEM_RSV_SHIFT)
	                         );
  mt6333_unlock();
}

kal_uint8 mt6333_get_qi_vmem_dig_mon(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_VMEM_CON7),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_QI_VMEM_DIG_MON_MASK),
                           (kal_uint8)(MT6333_PMIC_QI_VMEM_DIG_MON_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_qi_vmem_oc_status(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_VMEM_CON7),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_QI_VMEM_OC_STATUS_MASK),
                           (kal_uint8)(MT6333_PMIC_QI_VMEM_OC_STATUS_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

void mt6333_set_qi_vmem_vsleep(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VMEM_CON8),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_QI_VMEM_VSLEEP_MASK),
                             (kal_uint8)(MT6333_PMIC_QI_VMEM_VSLEEP_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_vmem_en(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VMEM_CON9),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_VMEM_EN_MASK),
                             (kal_uint8)(MT6333_PMIC_VMEM_EN_SHIFT)
	                         );
  mt6333_unlock();
}

kal_uint8 mt6333_get_qi_vmem_stb(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_VMEM_CON9),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_QI_VMEM_STB_MASK),
                           (kal_uint8)(MT6333_PMIC_QI_VMEM_STB_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_qi_vmem_en(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_VMEM_CON9),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_QI_VMEM_EN_MASK),
                           (kal_uint8)(MT6333_PMIC_QI_VMEM_EN_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

void mt6333_set_vmem_en_ctrl(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VMEM_CON9),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_VMEM_EN_CTRL_MASK),
                             (kal_uint8)(MT6333_PMIC_VMEM_EN_CTRL_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_vmem_vosel_ctrl(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VMEM_CON9),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_VMEM_VOSEL_CTRL_MASK),
                             (kal_uint8)(MT6333_PMIC_VMEM_VOSEL_CTRL_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_vmem_dlc_ctrl(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VMEM_CON9),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_VMEM_DLC_CTRL_MASK),
                             (kal_uint8)(MT6333_PMIC_VMEM_DLC_CTRL_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_vmem_burst_ctrl(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VMEM_CON9),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_VMEM_BURST_CTRL_MASK),
                             (kal_uint8)(MT6333_PMIC_VMEM_BURST_CTRL_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_vmem_vosel(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VMEM_CON12),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_VMEM_VOSEL_MASK),
                             (kal_uint8)(MT6333_PMIC_VMEM_VOSEL_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_vmem_vosel_on(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VMEM_CON13),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_VMEM_VOSEL_ON_MASK),
                             (kal_uint8)(MT6333_PMIC_VMEM_VOSEL_ON_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_vmem_vosel_sleep(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VMEM_CON14),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_VMEM_VOSEL_SLEEP_MASK),
                             (kal_uint8)(MT6333_PMIC_VMEM_VOSEL_SLEEP_SHIFT)
	                         );
  mt6333_unlock();
}

kal_uint8 mt6333_get_ni_vmem_vosel(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_VMEM_CON15),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_NI_VMEM_VOSEL_MASK),
                           (kal_uint8)(MT6333_PMIC_NI_VMEM_VOSEL_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_qi_vmem_burst(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_VMEM_CON16),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_QI_VMEM_BURST_MASK),
                           (kal_uint8)(MT6333_PMIC_QI_VMEM_BURST_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

void mt6333_set_vmem_burst(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VMEM_CON17),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_VMEM_BURST_MASK),
                             (kal_uint8)(MT6333_PMIC_VMEM_BURST_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_vmem_burst_on(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VMEM_CON17),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_VMEM_BURST_ON_MASK),
                             (kal_uint8)(MT6333_PMIC_VMEM_BURST_ON_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_vmem_burst_sleep(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VMEM_CON17),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_VMEM_BURST_SLEEP_MASK),
                             (kal_uint8)(MT6333_PMIC_VMEM_BURST_SLEEP_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_vmem_dlc(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VMEM_CON18),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_VMEM_DLC_MASK),
                             (kal_uint8)(MT6333_PMIC_VMEM_DLC_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_vmem_dlc_on(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VMEM_CON18),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_VMEM_DLC_ON_MASK),
                             (kal_uint8)(MT6333_PMIC_VMEM_DLC_ON_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_vmem_dlc_sleep(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VMEM_CON18),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_VMEM_DLC_SLEEP_MASK),
                             (kal_uint8)(MT6333_PMIC_VMEM_DLC_SLEEP_SHIFT)
	                         );
  mt6333_unlock();
}

kal_uint8 mt6333_get_qi_vmem_dlc(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_VMEM_CON18),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_QI_VMEM_DLC_MASK),
                           (kal_uint8)(MT6333_PMIC_QI_VMEM_DLC_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_qi_vmem_dlc_n(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_VMEM_CON19),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_QI_VMEM_DLC_N_MASK),
                           (kal_uint8)(MT6333_PMIC_QI_VMEM_DLC_N_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

void mt6333_set_vmem_dlc_n(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VMEM_CON20),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_VMEM_DLC_N_MASK),
                             (kal_uint8)(MT6333_PMIC_VMEM_DLC_N_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_vmem_dlc_n_on(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VMEM_CON20),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_VMEM_DLC_N_ON_MASK),
                             (kal_uint8)(MT6333_PMIC_VMEM_DLC_N_ON_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_vmem_dlc_n_sleep(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VMEM_CON20),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_VMEM_DLC_N_SLEEP_MASK),
                             (kal_uint8)(MT6333_PMIC_VMEM_DLC_N_SLEEP_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_vmem_vsleep_en(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VMEM_CON21),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_VMEM_VSLEEP_EN_MASK),
                             (kal_uint8)(MT6333_PMIC_VMEM_VSLEEP_EN_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_vmem_r2r_pdn(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VMEM_CON21),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_VMEM_R2R_PDN_MASK),
                             (kal_uint8)(MT6333_PMIC_VMEM_R2R_PDN_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_vmem_vsleep_sel(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VMEM_CON21),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_VMEM_VSLEEP_SEL_MASK),
                             (kal_uint8)(MT6333_PMIC_VMEM_VSLEEP_SEL_SHIFT)
	                         );
  mt6333_unlock();
}

kal_uint8 mt6333_get_ni_vmem_r2r_pdn(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_VMEM_CON21),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_NI_VMEM_R2R_PDN_MASK),
                           (kal_uint8)(MT6333_PMIC_NI_VMEM_R2R_PDN_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_ni_vmem_vsleep_sel(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_VMEM_CON21),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_NI_VMEM_VSLEEP_SEL_MASK),
                           (kal_uint8)(MT6333_PMIC_NI_VMEM_VSLEEP_SEL_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

void mt6333_set_vmem_transtd(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VMEM_CON22),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_VMEM_TRANSTD_MASK),
                             (kal_uint8)(MT6333_PMIC_VMEM_TRANSTD_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_vmem_vosel_trans_en(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VMEM_CON22),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_VMEM_VOSEL_TRANS_EN_MASK),
                             (kal_uint8)(MT6333_PMIC_VMEM_VOSEL_TRANS_EN_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_vmem_vosel_trans_once(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VMEM_CON22),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_VMEM_VOSEL_TRANS_ONCE_MASK),
                             (kal_uint8)(MT6333_PMIC_VMEM_VOSEL_TRANS_ONCE_SHIFT)
	                         );
  mt6333_unlock();
}

kal_uint8 mt6333_get_ni_vmem_vosel_trans(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_VMEM_CON22),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_NI_VMEM_VOSEL_TRANS_MASK),
                           (kal_uint8)(MT6333_PMIC_NI_VMEM_VOSEL_TRANS_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

void mt6333_set_rg_vrf18_vocal(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VRF18_RSV0),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_VRF18_VOCAL_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_VRF18_VOCAL_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_vrf18_gmsel(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VRF18_RSV0),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_VRF18_GMSEL_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_VRF18_GMSEL_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_vrf18_bk_ldo(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VRF18_CON0),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_VRF18_BK_LDO_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_VRF18_BK_LDO_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_vrf18_slew_nmos(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VRF18_CON0),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_VRF18_SLEW_NMOS_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_VRF18_SLEW_NMOS_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_vrf18_slew(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VRF18_CON0),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_VRF18_SLEW_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_VRF18_SLEW_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_vrf18_cc(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VRF18_CON1),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_VRF18_CC_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_VRF18_CC_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_vrf18_rzsel(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VRF18_CON1),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_VRF18_RZSEL_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_VRF18_RZSEL_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_vrf18_slp(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VRF18_CON2),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_VRF18_SLP_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_VRF18_SLP_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_vrf18_csl(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VRF18_CON2),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_VRF18_CSL_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_VRF18_CSL_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_vrf18_csr(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VRF18_CON2),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_VRF18_CSR_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_VRF18_CSR_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_vrf18_ndis_en(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VRF18_CON3),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_VRF18_NDIS_EN_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_VRF18_NDIS_EN_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_vrf18_zx_os(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VRF18_CON3),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_VRF18_ZX_OS_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_VRF18_ZX_OS_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_vrf18_burstl(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VRF18_CON4),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_VRF18_BURSTL_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_VRF18_BURSTL_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_vrf18_bursth(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VRF18_CON4),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_VRF18_BURSTH_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_VRF18_BURSTH_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_vrf18_rsv(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VRF18_CON6),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_VRF18_RSV_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_VRF18_RSV_SHIFT)
	                         );
  mt6333_unlock();
}

kal_uint8 mt6333_get_qi_vrf18_oc_status(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_VRF18_CON7),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_QI_VRF18_OC_STATUS_MASK),
                           (kal_uint8)(MT6333_PMIC_QI_VRF18_OC_STATUS_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

void mt6333_set_rg_buck_rsv1(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VRF18_CON8),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_BUCK_RSV1_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_BUCK_RSV1_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_vrf18_vosel_ctrl(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VRF18_CON9),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_VRF18_VOSEL_CTRL_MASK),
                             (kal_uint8)(MT6333_PMIC_VRF18_VOSEL_CTRL_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_vrf18_dlc_ctrl(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VRF18_CON9),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_VRF18_DLC_CTRL_MASK),
                             (kal_uint8)(MT6333_PMIC_VRF18_DLC_CTRL_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_vrf18_burst_ctrl(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VRF18_CON9),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_VRF18_BURST_CTRL_MASK),
                             (kal_uint8)(MT6333_PMIC_VRF18_BURST_CTRL_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_vrf18_vosel(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VRF18_CON12),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_VRF18_VOSEL_MASK),
                             (kal_uint8)(MT6333_PMIC_VRF18_VOSEL_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_vrf18_vosel_on(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VRF18_CON13),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_VRF18_VOSEL_ON_MASK),
                             (kal_uint8)(MT6333_PMIC_VRF18_VOSEL_ON_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_vrf18_vosel_sleep(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VRF18_CON14),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_VRF18_VOSEL_SLEEP_MASK),
                             (kal_uint8)(MT6333_PMIC_VRF18_VOSEL_SLEEP_SHIFT)
	                         );
  mt6333_unlock();
}

kal_uint8 mt6333_get_ni_vrf18_vosel(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_VRF18_CON15),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_NI_VRF18_VOSEL_MASK),
                           (kal_uint8)(MT6333_PMIC_NI_VRF18_VOSEL_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

void mt6333_set_rg_buck_mon_flag_sel(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VRF18_CON16),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_BUCK_MON_FLAG_SEL_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_BUCK_MON_FLAG_SEL_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_buck_rsv2_5_0(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VRF18_CON17),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_BUCK_RSV2_5_0_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_BUCK_RSV2_5_0_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_buck_bypass_vosel_limit(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VRF18_CON17),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_BUCK_BYPASS_VOSEL_LIMIT_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_BUCK_BYPASS_VOSEL_LIMIT_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_buck_mon_flag_en(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VRF18_CON17),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_BUCK_MON_FLAG_EN_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_BUCK_MON_FLAG_EN_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_vrf18_dlc(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VRF18_CON18),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_VRF18_DLC_MASK),
                             (kal_uint8)(MT6333_PMIC_VRF18_DLC_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_vrf18_dlc_on(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VRF18_CON18),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_VRF18_DLC_ON_MASK),
                             (kal_uint8)(MT6333_PMIC_VRF18_DLC_ON_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_vrf18_dlc_sleep(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VRF18_CON18),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_VRF18_DLC_SLEEP_MASK),
                             (kal_uint8)(MT6333_PMIC_VRF18_DLC_SLEEP_SHIFT)
	                         );
  mt6333_unlock();
}

kal_uint8 mt6333_get_qi_vrf18_dlc(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_VRF18_CON18),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_QI_VRF18_DLC_MASK),
                           (kal_uint8)(MT6333_PMIC_QI_VRF18_DLC_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_qi_vrf18_dlc_n(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_VRF18_CON19),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_QI_VRF18_DLC_N_MASK),
                           (kal_uint8)(MT6333_PMIC_QI_VRF18_DLC_N_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

void mt6333_set_vrf18_dlc_n(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VRF18_CON20),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_VRF18_DLC_N_MASK),
                             (kal_uint8)(MT6333_PMIC_VRF18_DLC_N_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_vrf18_dlc_n_on(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VRF18_CON20),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_VRF18_DLC_N_ON_MASK),
                             (kal_uint8)(MT6333_PMIC_VRF18_DLC_N_ON_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_vrf18_dlc_n_sleep(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VRF18_CON20),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_VRF18_DLC_N_SLEEP_MASK),
                             (kal_uint8)(MT6333_PMIC_VRF18_DLC_N_SLEEP_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_vrf18_modeset(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VRF18_CON21),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_VRF18_MODESET_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_VRF18_MODESET_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_vrf18_en(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VRF18_CON21),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_VRF18_EN_MASK),
                             (kal_uint8)(MT6333_PMIC_VRF18_EN_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_vrf18_stb_sel(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VRF18_CON21),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_VRF18_STB_SEL_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_VRF18_STB_SEL_SHIFT)
	                         );
  mt6333_unlock();
}

kal_uint8 mt6333_get_qi_vrf18_stb(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_VRF18_CON21),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_QI_VRF18_STB_MASK),
                           (kal_uint8)(MT6333_PMIC_QI_VRF18_STB_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_qi_vrf18_en(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_VRF18_CON21),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_QI_VRF18_EN_MASK),
                           (kal_uint8)(MT6333_PMIC_QI_VRF18_EN_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

void mt6333_set_vrf18_en_ctrl(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VRF18_CON21),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_VRF18_EN_CTRL_MASK),
                             (kal_uint8)(MT6333_PMIC_VRF18_EN_CTRL_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_vrf18_modeset_spm(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VRF18_CON22),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_VRF18_MODESET_SPM_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_VRF18_MODESET_SPM_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_vrf18_en_spm(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VRF18_CON22),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_VRF18_EN_SPM_MASK),
                             (kal_uint8)(MT6333_PMIC_VRF18_EN_SPM_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_vcore_vosel_set_spm(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_VRF18_CON22),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_VCORE_VOSEL_SET_SPM_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_VCORE_VOSEL_SET_SPM_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_k_rst_done(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_BUCK_K_CON0),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_K_RST_DONE_MASK),
                             (kal_uint8)(MT6333_PMIC_K_RST_DONE_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_k_map_sel(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_BUCK_K_CON0),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_K_MAP_SEL_MASK),
                             (kal_uint8)(MT6333_PMIC_K_MAP_SEL_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_k_once_en(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_BUCK_K_CON0),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_K_ONCE_EN_MASK),
                             (kal_uint8)(MT6333_PMIC_K_ONCE_EN_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_k_once(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_BUCK_K_CON0),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_K_ONCE_MASK),
                             (kal_uint8)(MT6333_PMIC_K_ONCE_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_k_start_manual(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_BUCK_K_CON0),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_K_START_MANUAL_MASK),
                             (kal_uint8)(MT6333_PMIC_K_START_MANUAL_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_k_src_sel(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_BUCK_K_CON0),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_K_SRC_SEL_MASK),
                             (kal_uint8)(MT6333_PMIC_K_SRC_SEL_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_k_auto_en(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_BUCK_K_CON0),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_K_AUTO_EN_MASK),
                             (kal_uint8)(MT6333_PMIC_K_AUTO_EN_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_k_inv(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_BUCK_K_CON0),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_K_INV_MASK),
                             (kal_uint8)(MT6333_PMIC_K_INV_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_k_control_smps(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_BUCK_K_CON1),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_K_CONTROL_SMPS_MASK),
                             (kal_uint8)(MT6333_PMIC_K_CONTROL_SMPS_SHIFT)
	                         );
  mt6333_unlock();
}

kal_uint8 mt6333_get_qi_smps_osc_cal(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_BUCK_K_CON2),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_QI_SMPS_OSC_CAL_MASK),
                           (kal_uint8)(MT6333_PMIC_QI_SMPS_OSC_CAL_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_k_result(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_BUCK_K_CON3),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_K_RESULT_MASK),
                           (kal_uint8)(MT6333_PMIC_K_RESULT_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_k_done(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_BUCK_K_CON3),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_K_DONE_MASK),
                           (kal_uint8)(MT6333_PMIC_K_DONE_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_k_control(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_BUCK_K_CON3),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_K_CONTROL_MASK),
                           (kal_uint8)(MT6333_PMIC_K_CONTROL_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

void mt6333_set_k_buck_ck_cnt(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_BUCK_K_CON4),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_K_BUCK_CK_CNT_MASK),
                             (kal_uint8)(MT6333_PMIC_K_BUCK_CK_CNT_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_k_chr_ck_cnt(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_BUCK_K_CON5),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_K_CHR_CK_CNT_MASK),
                             (kal_uint8)(MT6333_PMIC_K_CHR_CK_CNT_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_strup_rsv1_2_0(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_STRUP_CON0),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_STRUP_RSV1_2_0_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_STRUP_RSV1_2_0_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_strup_sleep_mode_deb_sel(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_STRUP_CON0),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_STRUP_SLEEP_MODE_DEB_SEL_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_STRUP_SLEEP_MODE_DEB_SEL_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_strup_poseq_done_deb_sel(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_STRUP_CON0),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_STRUP_POSEQ_DONE_DEB_SEL_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_STRUP_POSEQ_DONE_DEB_SEL_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_strup_vmem_en_deb_sel(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_STRUP_CON0),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_STRUP_VMEM_EN_DEB_SEL_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_STRUP_VMEM_EN_DEB_SEL_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_strup_vcore_en_deb_sel(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_STRUP_CON0),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_STRUP_VCORE_EN_DEB_SEL_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_STRUP_VCORE_EN_DEB_SEL_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_strup_buck_en_deb_sel(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_STRUP_CON0),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_STRUP_BUCK_EN_DEB_SEL_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_STRUP_BUCK_EN_DEB_SEL_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_strup_osc_en(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_STRUP_CON1),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_STRUP_OSC_EN_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_STRUP_OSC_EN_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_strup_osc_en_sel(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_STRUP_CON1),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_STRUP_OSC_EN_SEL_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_STRUP_OSC_EN_SEL_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_strup_ivgen_enb(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_STRUP_CON1),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_STRUP_IVGEN_ENB_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_STRUP_IVGEN_ENB_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_strup_ivgen_enb_sel(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_STRUP_CON1),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_STRUP_IVGEN_ENB_SEL_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_STRUP_IVGEN_ENB_SEL_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_strup_rsv2_7_4(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_STRUP_CON1),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_STRUP_RSV2_7_4_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_STRUP_RSV2_7_4_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_strup_rsv3_7_0(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_STRUP_CON2),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_STRUP_RSV3_7_0_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_STRUP_RSV3_7_0_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_efuse_addr(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_EFUSE_CON0),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_EFUSE_ADDR_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_EFUSE_ADDR_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_efuse_prog(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_EFUSE_CON1),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_EFUSE_PROG_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_EFUSE_PROG_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_efuse_en(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_EFUSE_CON2),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_EFUSE_EN_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_EFUSE_EN_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_efuse_pkey(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_EFUSE_CON3),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_EFUSE_PKEY_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_EFUSE_PKEY_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_efuse_rd_trig(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_EFUSE_CON4),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_EFUSE_RD_TRIG_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_EFUSE_RD_TRIG_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_efuse_prog_src(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_EFUSE_CON5),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_EFUSE_PROG_SRC_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_EFUSE_PROG_SRC_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_prog_macro_sel(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_EFUSE_CON6),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_PROG_MACRO_SEL_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_PROG_MACRO_SEL_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_rd_rdy_bypass(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_EFUSE_CON7),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_RD_RDY_BYPASS_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_RD_RDY_BYPASS_SHIFT)
	                         );
  mt6333_unlock();
}

kal_uint8 mt6333_get_rg_efuse_rd_ack(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_EFUSE_CON8),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RG_EFUSE_RD_ACK_MASK),
                           (kal_uint8)(MT6333_PMIC_RG_EFUSE_RD_ACK_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rg_efuse_busy(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_EFUSE_CON8),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RG_EFUSE_BUSY_MASK),
                           (kal_uint8)(MT6333_PMIC_RG_EFUSE_BUSY_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

void mt6333_set_rg_otp_pa(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_EFUSE_CON9),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_OTP_PA_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_OTP_PA_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_otp_pdin(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_EFUSE_CON10),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_OTP_PDIN_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_OTP_PDIN_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_otp_ptm(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_EFUSE_CON11),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_OTP_PTM_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_OTP_PTM_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_rg_fsource_en(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_EFUSE_CON12),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_RG_FSOURCE_EN_MASK),
                             (kal_uint8)(MT6333_PMIC_RG_FSOURCE_EN_SHIFT)
	                         );
  mt6333_unlock();
}

kal_uint8 mt6333_get_rg_efuse_dout_0_7(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_EFUSE_DOUT_0_7),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RG_EFUSE_DOUT_0_7_MASK),
                           (kal_uint8)(MT6333_PMIC_RG_EFUSE_DOUT_0_7_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rg_efuse_dout_8_15(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_EFUSE_DOUT_8_15),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RG_EFUSE_DOUT_8_15_MASK),
                           (kal_uint8)(MT6333_PMIC_RG_EFUSE_DOUT_8_15_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rg_efuse_dout_16_23(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_EFUSE_DOUT_16_23),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RG_EFUSE_DOUT_16_23_MASK),
                           (kal_uint8)(MT6333_PMIC_RG_EFUSE_DOUT_16_23_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rg_efuse_dout_24_31(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_EFUSE_DOUT_24_31),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RG_EFUSE_DOUT_24_31_MASK),
                           (kal_uint8)(MT6333_PMIC_RG_EFUSE_DOUT_24_31_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rg_efuse_dout_32_39(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_EFUSE_DOUT_32_39),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RG_EFUSE_DOUT_32_39_MASK),
                           (kal_uint8)(MT6333_PMIC_RG_EFUSE_DOUT_32_39_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rg_efuse_dout_40_47(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_EFUSE_DOUT_40_47),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RG_EFUSE_DOUT_40_47_MASK),
                           (kal_uint8)(MT6333_PMIC_RG_EFUSE_DOUT_40_47_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rg_efuse_dout_48_55(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_EFUSE_DOUT_48_55),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RG_EFUSE_DOUT_48_55_MASK),
                           (kal_uint8)(MT6333_PMIC_RG_EFUSE_DOUT_48_55_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

kal_uint8 mt6333_get_rg_efuse_dout_56_63(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_EFUSE_DOUT_56_63),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_RG_EFUSE_DOUT_56_63_MASK),
                           (kal_uint8)(MT6333_PMIC_RG_EFUSE_DOUT_56_63_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

void mt6333_set_testi0(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_TESTI_CON0),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_TESTI0_MASK),
                             (kal_uint8)(MT6333_PMIC_TESTI0_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_testi1(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_TESTI_CON1),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_TESTI1_MASK),
                             (kal_uint8)(MT6333_PMIC_TESTI1_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_testi2(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_TESTI_CON2),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_TESTI2_MASK),
                             (kal_uint8)(MT6333_PMIC_TESTI2_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_testi3(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_TESTI_CON3),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_TESTI3_MASK),
                             (kal_uint8)(MT6333_PMIC_TESTI3_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_testi4(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_TESTI_CON4),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_TESTI4_MASK),
                             (kal_uint8)(MT6333_PMIC_TESTI4_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_testi5(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_TESTI_CON5),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_TESTI5_MASK),
                             (kal_uint8)(MT6333_PMIC_TESTI5_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_testi6(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_TESTI_CON6),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_TESTI6_MASK),
                             (kal_uint8)(MT6333_PMIC_TESTI6_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_testi7(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_TESTI_CON7),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_TESTI7_MASK),
                             (kal_uint8)(MT6333_PMIC_TESTI7_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_testi8(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_TESTI_CON8),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_TESTI8_MASK),
                             (kal_uint8)(MT6333_PMIC_TESTI8_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_testi0_sel(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_TESTI_MUX_CON0),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_TESTI0_SEL_MASK),
                             (kal_uint8)(MT6333_PMIC_TESTI0_SEL_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_testi1_sel(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_TESTI_MUX_CON1),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_TESTI1_SEL_MASK),
                             (kal_uint8)(MT6333_PMIC_TESTI1_SEL_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_testi2_sel(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_TESTI_MUX_CON2),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_TESTI2_SEL_MASK),
                             (kal_uint8)(MT6333_PMIC_TESTI2_SEL_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_testi3_sel(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_TESTI_MUX_CON3),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_TESTI3_SEL_MASK),
                             (kal_uint8)(MT6333_PMIC_TESTI3_SEL_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_testi4_sel(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_TESTI_MUX_CON4),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_TESTI4_SEL_MASK),
                             (kal_uint8)(MT6333_PMIC_TESTI4_SEL_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_testi5_sel(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_TESTI_MUX_CON5),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_TESTI5_SEL_MASK),
                             (kal_uint8)(MT6333_PMIC_TESTI5_SEL_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_testi6_sel(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_TESTI_MUX_CON6),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_TESTI6_SEL_MASK),
                             (kal_uint8)(MT6333_PMIC_TESTI6_SEL_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_testi7_sel(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_TESTI_MUX_CON7),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_TESTI7_SEL_MASK),
                             (kal_uint8)(MT6333_PMIC_TESTI7_SEL_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_testi8_sel(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_TESTI_MUX_CON8),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_TESTI8_SEL_MASK),
                             (kal_uint8)(MT6333_PMIC_TESTI8_SEL_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_testo0(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_TESTO_CON0),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_TESTO0_MASK),
                             (kal_uint8)(MT6333_PMIC_TESTO0_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_testo1(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_TESTO_CON1),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_TESTO1_MASK),
                             (kal_uint8)(MT6333_PMIC_TESTO1_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_testo2(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_TESTO_CON2),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_TESTO2_MASK),
                             (kal_uint8)(MT6333_PMIC_TESTO2_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_testo3(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_TESTO_CON3),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_TESTO3_MASK),
                             (kal_uint8)(MT6333_PMIC_TESTO3_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_testo0_sel(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_TEST_OMUX_CON0),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_TESTO0_SEL_MASK),
                             (kal_uint8)(MT6333_PMIC_TESTO0_SEL_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_testo1_sel(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_TEST_OMUX_CON1),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_TESTO1_SEL_MASK),
                             (kal_uint8)(MT6333_PMIC_TESTO1_SEL_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_testo2_sel(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_TEST_OMUX_CON2),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_TESTO2_SEL_MASK),
                             (kal_uint8)(MT6333_PMIC_TESTO2_SEL_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_testo3_sel(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_TEST_OMUX_CON3),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_TESTO3_SEL_MASK),
                             (kal_uint8)(MT6333_PMIC_TESTO3_SEL_SHIFT)
	                         );
  mt6333_unlock();
}

void mt6333_set_debug_sel(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_DEBUG_CON0),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_DEBUG_SEL_MASK),
                             (kal_uint8)(MT6333_PMIC_DEBUG_SEL_SHIFT)
	                         );
  mt6333_unlock();
}

kal_uint8 mt6333_get_debug_mon(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_DEBUG_CON1),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_DEBUG_MON_MASK),
                           (kal_uint8)(MT6333_PMIC_DEBUG_MON_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

void mt6333_set_debug_bit_sel(kal_uint8 val)
{
  kal_uint8 ret=0;

  mt6333_lock();
  ret=mt6333_config_interface( (kal_uint8)(MT6333_DEBUG_CON2),
                             (kal_uint8)(val),
                             (kal_uint8)(MT6333_PMIC_DEBUG_BIT_SEL_MASK),
                             (kal_uint8)(MT6333_PMIC_DEBUG_BIT_SEL_SHIFT)
	                         );
  mt6333_unlock();
}

kal_uint8 mt6333_get_cid1(void)
{
  kal_uint8 ret=0;
  kal_uint8 val=0;

  mt6333_lock();
  ret=mt6333_read_interface( (kal_uint8)(MT6333_CID1),
                           (&val),
                           (kal_uint8)(MT6333_PMIC_CID1_MASK),
                           (kal_uint8)(MT6333_PMIC_CID1_SHIFT)
	                       );
  mt6333_unlock();

  return val;
}

/**********************************************************
  *
  *   [Internal Function] 
  *
  *********************************************************/
kal_uint32 mt6333_get_reg_value(kal_uint32 reg)
{
    kal_uint32 ret=0;
    kal_uint8 reg_val=0;

    ret=mt6333_read_interface( (kal_uint8) reg, &reg_val, 0xFF, 0x0);
    
    return reg_val;
}
EXPORT_SYMBOL(mt6333_get_reg_value);

void mt6333_dump_register(void)
{
    kal_uint8 i=0;
    kal_uint8 mt_swchr_60393_reg=0;
    printk("[mt6333] ");
    for (i=0;i<0xFF;i++)
    {
        mt6333_read_interface(i, &mt_swchr_60393_reg, 0xFF, 0);
        printk("[0x%x]=0x%x ", i, mt_swchr_60393_reg);
        
        if( (i%5) == 0)
            printk("\n");        
    }
    printk("\n");
}

void mt6333_hw_init(void)
{
    // disable when DVT
    mt6333_set_rg_usbdl_mode_b(1);

    printk("mt6333_hw_init : Done\n");    
}

/**********************************************************
  *
  *   [MT6333 Interrupt Function] 
  *
  *********************************************************/
#if defined(CONFIG_POWER_EXT)
//----------------------------------------------------------------------
#define CUST_EINT_POLARITY_LOW              0
#define CUST_EINT_POLARITY_HIGH             1
#define CUST_EINT_DEBOUNCE_DISABLE          0
#define CUST_EINT_DEBOUNCE_ENABLE           1
#define CUST_EINT_EDGE_SENSITIVE            0
#define CUST_EINT_LEVEL_SENSITIVE           1
    
#define CUST_EINT_MT6333_PMIC_NUM              0
#define CUST_EINT_MT6333_PMIC_DEBOUNCE_CN      1
//#define CUST_EINT_MT6333_PMIC_POLARITY         CUST_EINT_POLARITY_LOW // deff with 6323
//#define CUST_EINT_MT6333_PMIC_SENSITIVE        CUST_EINT_LEVEL_SENSITIVE
#define CUST_EINT_MT6333_PMIC_TYPE             EINTF_TRIGGER_LOW        
#define CUST_EINT_MT6333_PMIC_DEBOUNCE_EN      CUST_EINT_DEBOUNCE_ENABLE
//----------------------------------------------------------------------
#else
#include <cust_eint.h>
#endif

int mt6333_thread_timeout=0;
static DEFINE_MUTEX(mt6333_mutex);
static DECLARE_WAIT_QUEUE_HEAD(mt6333_thread_wq);

void wake_up_mt6333(void)
{
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[wake_up_mt6333]\n");
    mt6333_thread_timeout = 1;
    wake_up(&mt6333_thread_wq);
}
EXPORT_SYMBOL(wake_up_mt6333);

void cust_mt6333_interrupt_en_setting(void)
{
    mt6333_set_rg_int_en_chr_complete(0);
    mt6333_set_rg_int_en_thermal_sd(0);
    mt6333_set_rg_int_en_thermal_reg_in(0);
    mt6333_set_rg_int_en_thermal_reg_out(0);
    mt6333_set_rg_int_en_otg_oc(0);
    mt6333_set_rg_int_en_otg_thermal(0);
    mt6333_set_rg_int_en_otg_chrin_short(0);
    mt6333_set_rg_int_en_otg_drvcdt_short(0);

    mt6333_set_rg_int_en_chrwdt_flag(0);
    mt6333_set_rg_int_en_buck_vcore_oc(0);
    mt6333_set_rg_int_en_buck_vmem_oc(0);
    mt6333_set_rg_int_en_buck_vrf18_oc(0);
    mt6333_set_rg_int_en_buck_thermal(0);
    mt6333_set_rg_int_en_flash_en_timeout(0);
    mt6333_set_rg_int_en_flash_vled_short(0);
    mt6333_set_rg_int_en_flash_vled_open(0);

    mt6333_set_rg_int_en_chr_oc(0);
    mt6333_set_rg_int_en_chr_plug_in_flash(0);    

    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "Reg[0x%x]=0x%x\n", MT6333_INT_CON0, mt6333_get_reg_value(MT6333_INT_CON0));
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "Reg[0x%x]=0x%x\n", MT6333_INT_CON1, mt6333_get_reg_value(MT6333_INT_CON1));
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "Reg[0x%x]=0x%x\n", MT6333_INT_CON2, mt6333_get_reg_value(MT6333_INT_CON2));
}

void chr_complete_int_handler(void)
{
    kal_uint32 ret=0;
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[chr_complete_int_handler]....\n");
    
    printk("RGS_CHARGE_COMPLETE_HW = %d\n", mt6333_get_rgs_charge_complete_hw());
    
    ret=mt6333_config_interface(MT6333_INT_STATUS0,0x1,0x1,0);
}

void thermal_sd_int_handler(void)
{
    kal_uint32 ret=0;
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[thermal_sd_int_handler]....\n");
    ret=mt6333_config_interface(MT6333_INT_STATUS0,0x1,0x1,1);
}

void thermal_reg_in_int_handler(void)
{
    kal_uint32 ret=0;
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[thermal_reg_in_int_handler]....\n");
    ret=mt6333_config_interface(MT6333_INT_STATUS0,0x1,0x1,2);
}

void thermal_reg_out_int_handler(void)
{
    kal_uint32 ret=0;
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[thermal_reg_out_int_handler]....\n");
    ret=mt6333_config_interface(MT6333_INT_STATUS0,0x1,0x1,3);
}

void otg_oc_int_handler(void)
{
    kal_uint32 ret=0;
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[otg_oc_int_handler]....\n");
    ret=mt6333_config_interface(MT6333_INT_STATUS0,0x1,0x1,4);
}

void otg_thermal_int_handler(void)
{
    kal_uint32 ret=0;
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[otg_thermal_int_handler]....\n");
    ret=mt6333_config_interface(MT6333_INT_STATUS0,0x1,0x1,5);
}

void otg_chrin_short_int_handler(void)
{
    kal_uint32 ret=0;
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[otg_chrin_short_int_handler]....\n");
    ret=mt6333_config_interface(MT6333_INT_STATUS0,0x1,0x1,6);
}

void otg_drvcdt_short_int_handler(void)
{
    kal_uint32 ret=0;
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[otg_drvcdt_short_int_handler]....\n");
    ret=mt6333_config_interface(MT6333_INT_STATUS0,0x1,0x1,7);
}

void chrwdt_flag_int_handler(void)
{
    kal_uint32 ret=0;
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[chrwdt_flag_int_handler]....\n");
    ret=mt6333_config_interface(MT6333_INT_STATUS1,0x1,0x1,0);
}

void buck_vcore_oc_int_handler(void)
{
    kal_uint32 ret=0;
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[buck_vcore_oc_int_handler]....\n");
    ret=mt6333_config_interface(MT6333_INT_STATUS1,0x1,0x1,1);
}

void buck_vmem_oc_int_handler(void)
{
    kal_uint32 ret=0;
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[buck_vmem_oc_int_handler]....\n");
    ret=mt6333_config_interface(MT6333_INT_STATUS1,0x1,0x1,2);
}

void buck_vrf18_oc_int_handler(void)
{
    kal_uint32 ret=0;
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[buck_vrf18_oc_int_handler]....\n");
    ret=mt6333_config_interface(MT6333_INT_STATUS1,0x1,0x1,3);
}

void buck_thermal_int_handler(void)
{
    kal_uint32 ret=0;
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[buck_thermal_int_handler]....\n");
    ret=mt6333_config_interface(MT6333_INT_STATUS1,0x1,0x1,4);
}

void flash_en_timeout_int_handler(void)
{
    kal_uint32 ret=0;
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[flash_en_timeout_int_handler]....\n");
    ret=mt6333_config_interface(MT6333_INT_STATUS1,0x1,0x1,5);
}

void flash_vled_short_int_handler(void)
{
    kal_uint32 ret=0;
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[flash_vled_short_int_handler]....\n");
    ret=mt6333_config_interface(MT6333_INT_STATUS1,0x1,0x1,6);
}

void flash_vled_open_int_handler(void)
{
    kal_uint32 ret=0;
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[flash_vled_open_int_handler]....\n");
    ret=mt6333_config_interface(MT6333_INT_STATUS1,0x1,0x1,7);
}

void chr_plug_in_flash_int_handler(void)
{
    kal_uint32 ret=0;
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[chr_plug_in_flash_int_handler]....\n");
    ret=mt6333_config_interface(MT6333_INT_STATUS2,0x1,0x1,0);
}

void chr_oc_int_handler(void)
{
    kal_uint32 ret=0;
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[chr_oc_int_handler]....\n");
    ret=mt6333_config_interface(MT6333_INT_STATUS2,0x1,0x1,1);
}

static int mt6333_thread_kthread(void *x)
{
    kal_uint32 int_status_val_0=0;
    kal_uint32 int_status_val_1=0;
    kal_uint32 int_status_val_2=0;

    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[mt6333_thread_kthread] enter\n");

    /* Run on a process content */
    while (1) {
        mutex_lock(&mt6333_mutex);

        xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[mt6333_thread_kthread] running\n");

        //--------------------------------------------------------------------------------
        int_status_val_0 = mt6333_get_reg_value(MT6333_INT_STATUS0);
        xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[INT] int_status_val_0=0x%x\n", int_status_val_0);

        if( (((int_status_val_0)&(0x0001))>>0) == 1 )  { chr_complete_int_handler();     }
        if( (((int_status_val_0)&(0x0002))>>1) == 1 )  { thermal_sd_int_handler();       }         
        if( (((int_status_val_0)&(0x0004))>>2) == 1 )  { thermal_reg_in_int_handler();   }
        if( (((int_status_val_0)&(0x0008))>>3) == 1 )  { thermal_reg_out_int_handler();  }
        if( (((int_status_val_0)&(0x0010))>>4) == 1 )  { otg_oc_int_handler();           }
        if( (((int_status_val_0)&(0x0020))>>5) == 1 )  { otg_thermal_int_handler();      }
        if( (((int_status_val_0)&(0x0040))>>6) == 1 )  { otg_chrin_short_int_handler();  }
        if( (((int_status_val_0)&(0x0080))>>7) == 1 )  { otg_drvcdt_short_int_handler(); }                     
        //--------------------------------------------------------------------------------
        int_status_val_1 = mt6333_get_reg_value(MT6333_INT_STATUS1);
        xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[INT] int_status_val_1=0x%x\n", int_status_val_1);

        if( (((int_status_val_0)&(0x0001))>>0) == 1 )  { chrwdt_flag_int_handler();       }
        if( (((int_status_val_0)&(0x0002))>>1) == 1 )  { buck_vcore_oc_int_handler();     }          
        if( (((int_status_val_0)&(0x0004))>>2) == 1 )  { buck_vmem_oc_int_handler();      }
        if( (((int_status_val_0)&(0x0008))>>3) == 1 )  { buck_vrf18_oc_int_handler();     }
        if( (((int_status_val_0)&(0x0010))>>4) == 1 )  { buck_thermal_int_handler();      }
        if( (((int_status_val_0)&(0x0020))>>5) == 1 )  { flash_en_timeout_int_handler();  }
        if( (((int_status_val_0)&(0x0040))>>6) == 1 )  { flash_vled_short_int_handler();  }
        if( (((int_status_val_0)&(0x0080))>>7) == 1 )  { flash_vled_open_int_handler();   }            
        //--------------------------------------------------------------------------------
        int_status_val_2 = mt6333_get_reg_value(MT6333_INT_STATUS2);
        xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[INT] int_status_val_2=0x%x\n", int_status_val_2);

        if( (((int_status_val_0)&(0x0001))>>0) == 1 )  { chr_plug_in_flash_int_handler(); }
        if( (((int_status_val_0)&(0x0002))>>1) == 1 )  { chr_oc_int_handler();            }         
        //--------------------------------------------------------------------------------

        mdelay(1);
        
        mt_eint_unmask(CUST_EINT_MT6333_PMIC_NUM);

        //set INT_EN, in PMIC_EINT_SETTING()
        cust_mt6333_interrupt_en_setting();

        mutex_unlock(&mt6333_mutex);

        wait_event(mt6333_thread_wq, mt6333_thread_timeout);

        mt6333_thread_timeout=0;
    }

    return 0;
}

void mt6333_pmic_eint_irq(void)
{
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[mt6333_pmic_eint_irq] receive interrupt\n");

    wake_up_mt6333();

    return ;
}

void MT6333_EINT_SETTING(void)
{
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[MT6333_EINT_SETTING] start: CUST_EINT_MT6333_PMIC_NUM=%d\n",CUST_EINT_MT6333_PMIC_NUM);

    //ON/OFF interrupt
    cust_mt6333_interrupt_en_setting();

    //GPIO Setting for early porting

    //EINT setting
    //mt_eint_set_sens(           CUST_EINT_MT6333_PMIC_NUM,
    //                            CUST_EINT_MT6333_PMIC_SENSITIVE);
    //mt_eint_set_polarity(       CUST_EINT_MT6333_PMIC_NUM,
    //                            CUST_EINT_MT6333_PMIC_POLARITY);        // set positive polarity
    mt_eint_set_hw_debounce(    CUST_EINT_MT6333_PMIC_NUM,
                                CUST_EINT_MT6333_PMIC_DEBOUNCE_CN);     // set debounce time
    mt_eint_registration(       CUST_EINT_MT6333_PMIC_NUM,                            
                                CUST_EINT_MT6333_PMIC_TYPE,
                                mt6333_pmic_eint_irq,
                                0);

    mt_eint_unmask(CUST_EINT_MT6333_PMIC_NUM);

    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[CUST_EINT] CUST_EINT_MT6333_PMIC_NUM=%d\n",           CUST_EINT_MT6333_PMIC_NUM);
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[CUST_EINT] CUST_EINT_MT6333_PMIC_DEBOUNCE_CN=%d\n",   CUST_EINT_MT6333_PMIC_DEBOUNCE_CN);
    //xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[CUST_EINT] CUST_EINT_MT6333_PMIC_POLARITY=%d\n",      CUST_EINT_MT6333_PMIC_POLARITY);
    //xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[CUST_EINT] CUST_EINT_MT6333_PMIC_SENSITIVE=%d\n",     CUST_EINT_MT6333_PMIC_SENSITIVE);
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[CUST_EINT] CUST_EINT_MT6333_PMIC_TYPE=%d\n",          CUST_EINT_MT6333_PMIC_TYPE);
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[CUST_EINT] CUST_EINT_MT6333_PMIC_DEBOUNCE_EN=%d\n",   CUST_EINT_MT6333_PMIC_DEBOUNCE_EN);
}    

#if 1
/**********************************************************
  *
  *   [DVT API] 
  *
  *********************************************************/
void tc_1101(void)
{   
    int ret = 0;
    kal_uint8 reg_val=0;
    
    printk("[tc_1101] \n");

    ret=mt6333_read_interface(0x00, &reg_val, 0xFF, 0x0);
    if(reg_val != 0x93) printk("Fail : i2c_read(0x00, 0x93), 0x%x\n", reg_val);

    ret=mt6333_read_interface(0xA9, &reg_val, 0xFF, 0x0);
    if(reg_val != 0x00) printk("Fail : i2c_read(0xA9, 0x00), 0x%x\n", reg_val);

    ret=mt6333_read_interface(0xFD, &reg_val, 0xFF, 0x0);
    if(reg_val != 0x10) printk("Fail : i2c_read(0xFD, 0x10), 0x%x\n", reg_val);

    ret=mt6333_config_interface(0x00, 0x00, 0xFF, 0x0);
    ret=mt6333_read_interface(0x00, &reg_val, 0xFF, 0x0);
    if(reg_val != 0x93) printk("Fail : i2c_write(0x00, 0x00), 0x%x\n", reg_val);

    ret=mt6333_config_interface(0xA9, 0xFF, 0xFF, 0x0);
    ret=mt6333_read_interface(0xA9, &reg_val, 0xFF, 0x0);
    if(reg_val != 0xFF) printk("Fail : i2c_write(0xA9, 0xFF), 0x%x\n", reg_val);

    ret=mt6333_config_interface(0xA9, 0x38, 0xFF, 0x0);
    ret=mt6333_read_interface(0xA9, &reg_val, 0xFF, 0x0);
    if(reg_val != 0x38) printk("Fail : i2c_write(0xA9, 0x38), 0x%x\n", reg_val);

    ret=mt6333_config_interface(0xFD, 0xFF, 0xFF, 0x0);
    ret=mt6333_read_interface(0xFD, &reg_val, 0xFF, 0x0);
    if(reg_val != 0x10) printk("Fail : i2c_write(0xFD, 0xFF), 0x%x\n", reg_val); 

    printk("[tc_1101] Done\n");
}

void tc_1102(void)
{
    int ret = 0;
    kal_uint8 reg_val=0;
    
    printk("[tc_1102] \n");

    ret=mt6333_config_interface(0x04, 0x01, 0xFF, 0x0);
    ret=mt6333_read_interface(0x04, &reg_val, 0xFF, 0x0);
    if(reg_val != 0x00) printk("Fail : i2c_write(0x04, 0x01), 0x%x\n", reg_val); 

    ret=mt6333_config_interface(0x04, 0x11, 0xFF, 0x0);
    ret=mt6333_read_interface(0x04, &reg_val, 0xFF, 0x0);
    if(reg_val != 0x00) printk("Fail : i2c_write(0x04, 0x11), 0x%x\n", reg_val); 

    ret=mt6333_config_interface(0x04, 0x95, 0xFF, 0x0);
    ret=mt6333_read_interface(0x04, &reg_val, 0xFF, 0x0);
    if(reg_val != 0x01) printk("Fail : i2c_write(0x04, 0x95), 0x%x\n", reg_val); 

    ret=mt6333_config_interface(0x04, 0x00, 0xFF, 0x0);
    ret=mt6333_read_interface(0x04, &reg_val, 0xFF, 0x0);
    if(reg_val != 0x01) printk("Fail : i2c_write(0x04, 0x00), 0x%x\n", reg_val);

    ret=mt6333_config_interface(0x04, 0x20, 0xFF, 0x0);
    ret=mt6333_read_interface(0x04, &reg_val, 0xFF, 0x0);
    if(reg_val != 0x01) printk("Fail : i2c_write(0x04, 0x20), 0x%x\n", reg_val);

    ret=mt6333_config_interface(0x04, 0x94, 0xFF, 0x0);
    ret=mt6333_read_interface(0x04, &reg_val, 0xFF, 0x0);
    if(reg_val != 0x00) printk("Fail : i2c_write(0x04, 0x94), 0x%x\n", reg_val);

    ret=mt6333_config_interface(0x30, 0x01, 0xFF, 0x0);
    ret=mt6333_read_interface(0x30, &reg_val, 0xFF, 0x0);
    if(reg_val != 0x00) printk("Fail : i2c_write(0x30, 0x01), 0x%x\n", reg_val);

    ret=mt6333_config_interface(0x30, 0x11, 0xFF, 0x0);
    ret=mt6333_read_interface(0x30, &reg_val, 0xFF, 0x0);
    if(reg_val != 0x00) printk("Fail : i2c_write(0x30, 0x11), 0x%x\n", reg_val);

    ret=mt6333_config_interface(0x30, 0x95, 0xFF, 0x0);
    ret=mt6333_read_interface(0x30, &reg_val, 0xFF, 0x0);
    if(reg_val != 0x01) printk("Fail : i2c_write(0x30, 0x95), 0x%x\n", reg_val);

    ret=mt6333_config_interface(0x30, 0x00, 0xFF, 0x0);
    ret=mt6333_read_interface(0x30, &reg_val, 0xFF, 0x0);
    if(reg_val != 0x01) printk("Fail : i2c_write(0x30, 0x00), 0x%x\n", reg_val);

    ret=mt6333_config_interface(0x30, 0x20, 0xFF, 0x0);
    ret=mt6333_read_interface(0x30, &reg_val, 0xFF, 0x0);
    if(reg_val != 0x01) printk("Fail : i2c_write(0x30, 0x20), 0x%x\n", reg_val);

    ret=mt6333_config_interface(0x30, 0x94, 0xFF, 0x0);
    ret=mt6333_read_interface(0x30, &reg_val, 0xFF, 0x0);
    if(reg_val != 0x00) printk("Fail : i2c_write(0x30, 0x94), 0x%x\n", reg_val);

    printk("[tc_1102] Done\n");
}

void tc_1103(void)
{   
    int ret = 0;
    kal_uint8 reg_val=0;
    
    printk("[tc_1103] \n");

    ret=mt6333_read_interface(0xFD, &reg_val, 0xFF, 0x0);
    if(reg_val != 0x20) printk("Fail : i2c_read(0xFD, 0x20), 0x%x\n", reg_val);

    ret=mt6333_read_interface(0x07, &reg_val, 0xFF, 0x0);
    if(reg_val != 0x02) printk("Fail : i2c_read(0x07, 0x02), 0x%x\n", reg_val);

    printk("[tc_1103] Done\n");
}

void tc_1201(void)
{
    printk("[tc_1201] \n");

    tc_1101();
    tc_1102();

    printk("[tc_1201] Done\n");
}

//----------------------------------------------------------------
extern int PMIC_IMM_GetOneChannelValue(int dwChannel, int deCount, int trimd);

void read_adc_value(int index)
{
    int ret=0;

    mdelay(1);
    
    if(index == 0) //vcore
    {   
        printk("Reg[%x]=0x%x,Reg[%x]=0x%x,Reg[%x]=0x%x,Reg[%x]=0x%x,Reg[%x]=0x%x,Reg[%x]=0x%x,Reg[%x]=0x%x,Reg[%x]=0x%x,Reg[%x]=0x%x,Reg[%x]=0x%x\n", 
                0x68, mt6333_get_reg_value(0x68), // NI_xxx_EN
                0x6B, mt6333_get_reg_value(0x6B), 
                0x6C, mt6333_get_reg_value(0x6C),
                0x6D, mt6333_get_reg_value(0x6D),
                0x6E, mt6333_get_reg_value(0x6E), // NI_xxx_VOSEL
                0x75, mt6333_get_reg_value(0x75),
                0x70, mt6333_get_reg_value(0x70), // NI_xxx_DLC
                0x71, mt6333_get_reg_value(0x71), // NI_xxx_DLC_N
                0x6F, mt6333_get_reg_value(0x6F), // NI_xxx_BURST                
                0xA0, mt6333_get_reg_value(0xA0)
                );        
    }
    else if(index == 1) //vmem
    {
        printk("Reg[%x]=0x%x,Reg[%x]=0x%x,Reg[%x]=0x%x,Reg[%x]=0x%x,Reg[%x]=0x%x,Reg[%x]=0x%x,Reg[%x]=0x%x,Reg[%x]=0x%x,Reg[%x]=0x%x\n", 
                0x80, mt6333_get_reg_value(0x80), // NI_xxx_EN
                0x81, mt6333_get_reg_value(0x81),
                0x82, mt6333_get_reg_value(0x82),
                0x83, mt6333_get_reg_value(0x83),
                0x84, mt6333_get_reg_value(0x84), // NI_xxx_VOSEL
                0x87, mt6333_get_reg_value(0x87), // NI_xxx_DLC
                0x88, mt6333_get_reg_value(0x88), // NI_xxx_DLC_N
                0x85, mt6333_get_reg_value(0x85), // NI_xxx_BURST                
                0xA0, mt6333_get_reg_value(0xA0)
                );
    }
    else if(index == 2) //vrf18
    {
        printk("Reg[%x]=0x%x,Reg[%x]=0x%x,Reg[%x]=0x%x,Reg[%x]=0x%x,Reg[%x]=0x%x,Reg[%x]=0x%x,Reg[%x]=0x%x,Reg[%x]=0x%x,Reg[%x]=0x%x\n", 
                0x9F, mt6333_get_reg_value(0x9F), // NI_xxx_EN
                0x95, mt6333_get_reg_value(0x95),
                0x96, mt6333_get_reg_value(0x96),
                0x97, mt6333_get_reg_value(0x97),
                0x98, mt6333_get_reg_value(0x98),
                0x99, mt6333_get_reg_value(0x99), // NI_xxx_VOSEL
                0x9C, mt6333_get_reg_value(0x9C), // NI_xxx_DLC
                0x9D, mt6333_get_reg_value(0x9D), // NI_xxx_DLC_N
                0xA0, mt6333_get_reg_value(0xA0)
                );
    }
    else 
    {
    }

    mdelay(20);
    ret = PMIC_IMM_GetOneChannelValue(5,1,0);

    ret = ret * 4; // with J259 and use J258(UP pin)
    
    printk("[read_auxadc_value] ret = %d\n\n", ret);
}

#define VCORE_INDEX 0
#define VMEM_INDEX  1
#define VRF18_INDEX 2

void set_thr_sdn_en(int thr_sdn_en)
{
    mt6333_config_interface(MT6333_TEST_OMUX_CON0, 1, 0x1, 7); // SW mode
    mt6333_config_interface(MT6333_TESTO_CON0, thr_sdn_en, 0x1, 7);

    printk("[set_thr_sdn_en] Reg[0x%x]=0x%x, Reg[0x%x]=0x%x\n", 
        MT6333_TEST_OMUX_CON0, mt6333_get_reg_value(MT6333_TEST_OMUX_CON0),
        MT6333_TESTO_CON0, mt6333_get_reg_value(MT6333_TESTO_CON0)
        );
}

void exec_scrxxx_map(int buck_index)
{
    mt6333_config_interface(MT6333_TEST_OMUX_CON2, 1, 0x1, 0); // SRCLKEN SW mode
    printk("[SRCLKEN] Reg[0x%x]=0x%x\n", MT6333_TEST_OMUX_CON2, mt6333_get_reg_value(MT6333_TEST_OMUX_CON2));
    
    mt6333_config_interface(MT6333_TESTO_CON2, 1, 0x1, 0);
    printk("[SRCLKEN=1] Reg[0x%x]=0x%x\n", MT6333_TESTO_CON2, mt6333_get_reg_value(MT6333_TESTO_CON2));
    read_adc_value(buck_index);

    mt6333_config_interface(MT6333_TESTO_CON2, 0, 0x1, 0);
    printk("[SRCLKEN=0] Reg[0x%x]=0x%x\n", MT6333_TESTO_CON2, mt6333_get_reg_value(MT6333_TESTO_CON2));
    read_adc_value(buck_index);

    mt6333_config_interface(MT6333_TESTO_CON2, 1, 0x1, 0);
    printk("[SRCLKEN=1] Reg[0x%x]=0x%x\n", MT6333_TESTO_CON2, mt6333_get_reg_value(MT6333_TESTO_CON2));
    read_adc_value(buck_index);
}

void exec_vcore_en_test(void)
{
    int i=0;
   
    for(i=0;i<=1;i++)
    {
        printk("[mt6333_set_vcore_en_ctrl] %d\n", i);
        mt6333_set_vcore_en_ctrl(i);                
        
        switch(i){
            case 0:
                printk("[mt6333_set_vcore_en(0)]\n");
                mt6333_set_vcore_en(0);        
                read_adc_value(VCORE_INDEX);   
                
                printk("[mt6333_set_vcore_en(1)]\n");
                mt6333_set_vcore_en(1);        
                read_adc_value(VCORE_INDEX);
                break;    

            case 1: 
                exec_scrxxx_map(VCORE_INDEX);
                break;

            default:
                printk("Invalid channel value(%d)\n", i);
                break;    
        }            
    } 
}

void exec_vmem_en_test(void)
{
    int i=0;
   
    for(i=0;i<=1;i++)
    {
        printk("[mt6333_set_vmem_en_ctrl] %d\n", i);
        mt6333_set_vmem_en_ctrl(i);                
        
        switch(i){
            case 0:
                printk("[mt6333_set_rg_vmem_en_thr_sdn_sel(0)]\n");
                mt6333_set_rg_vmem_en_thr_sdn_sel(0);
                    //------------------------------------------
                    printk("vmem_en=0, thr_sdn_en=0\n");
                    mt6333_set_vmem_en(0);
                    set_thr_sdn_en(0);
                    read_adc_value(VMEM_INDEX);   
                    //------------------------------------------    
                    printk("vmem_en=0, thr_sdn_en=1\n");
                    mt6333_set_vmem_en(0);
                    set_thr_sdn_en(1);
                    read_adc_value(VMEM_INDEX);
                    //------------------------------------------
                    printk("vmem_en=1, thr_sdn_en=0\n");
                    mt6333_set_vmem_en(1);
                    set_thr_sdn_en(0);
                    read_adc_value(VMEM_INDEX);
                    //------------------------------------------
                    printk("vmem_en=1, thr_sdn_en=1\n");
                    mt6333_set_vmem_en(1);
                    set_thr_sdn_en(1);
                    read_adc_value(VMEM_INDEX);
                    //------------------------------------------
                printk("[mt6333_set_rg_vmem_en_thr_sdn_sel(1)]\n");
                mt6333_set_rg_vmem_en_thr_sdn_sel(1);    
                    //------------------------------------------
                    printk("[mt6333_set_vmem_en(0)]\n");
                    mt6333_set_vmem_en(0);        
                    read_adc_value(VMEM_INDEX);   
                    //------------------------------------------    
                    printk("[mt6333_set_vmem_en(1)]\n");
                    mt6333_set_vmem_en(1);        
                    read_adc_value(VMEM_INDEX);
                    //------------------------------------------                
                break;    

            case 1: 
                exec_scrxxx_map(VMEM_INDEX);
                break;

            default:
                printk("Invalid channel value(%d)\n", i);
                break;    
        }            
    } 
}

void exec_vrf18_en_test(void)
{
    int i=0;
   
    for(i=0;i<=1;i++)
    {
        printk("[mt6333_set_vrf18_en_ctrl] %d\n", i);
        mt6333_set_vrf18_en_ctrl(i);                
        
        switch(i){
            case 0:
                printk("vrf18_en=0, vrf18_en_spm=0]\n");
                mt6333_set_vrf18_en(0);
                mt6333_set_vrf18_en_spm(0);
                read_adc_value(VRF18_INDEX);   
                //------------------------------------------
                printk("vrf18_en=0, vrf18_en_spm=1]\n");
                mt6333_set_vrf18_en(0);
                mt6333_set_vrf18_en_spm(1);
                read_adc_value(VRF18_INDEX);
                //------------------------------------------
                printk("vrf18_en=1, vrf18_en_spm=0]\n");
                mt6333_set_vrf18_en(1);
                mt6333_set_vrf18_en_spm(0);
                read_adc_value(VRF18_INDEX);
                //------------------------------------------
                printk("vrf18_en=1, vrf18_en_spm=1]\n");
                mt6333_set_vrf18_en(1);
                mt6333_set_vrf18_en_spm(1);
                read_adc_value(VRF18_INDEX);
                break;    

            case 1: 
                exec_scrxxx_map(VRF18_INDEX);
                break;

            default:
                printk("Invalid channel value(%d)\n", i);
                break;    
        }            
    } 
}

void PMIC_BUCK_ON_OFF(int index_val)
{   
    printk("[PMIC_BUCK_ON_OFF] start....\n");

    mt6333_config_interface(MT6333_TEST_OMUX_CON2, 1, 0x1, 0); // SRCLKEN SW mode
    printk("[SRCLKEN] Reg[0x%x]=0x%x\n", MT6333_TEST_OMUX_CON2, mt6333_get_reg_value(MT6333_TEST_OMUX_CON2));

    switch(index_val){
      case 0:
        exec_vcore_en_test();
        break;

      case 1:
        exec_vmem_en_test();
        break;

      case 2:
        exec_vrf18_en_test();
        break;
       
	  default:
        printk("[PMIC_BUCK_ON_OFF] Invalid channel value(%d)\n", index_val);
        break;
        
    }
    
    printk("[PMIC_BUCK_ON_OFF] end....\n");
}

void vcore_vosel_sub_1_setting(int vsleep_en, int thr_sdn_en)
{
    //mt6333_set_vcore_vsleep_en(vsleep_en);
    if(vsleep_en==1)
        vsleep_en=0;
    else
        vsleep_en=1;    
    mt6333_config_interface(MT6333_TESTO_CON2, vsleep_en, 0x1, 0);
    printk("[SRCLKEN=%d] Reg[0x%x]=0x%x\n", vsleep_en, MT6333_TESTO_CON2, mt6333_get_reg_value(MT6333_TESTO_CON2));

    set_thr_sdn_en(thr_sdn_en);

    printk("[vcore_vosel_sub_1_setting] Reg[0x%x]=0x%x, Reg[0x%x]=0x%x, Reg[0x%x]=0x%x\n",
        MT6333_VCORE_CON27, mt6333_get_reg_value(MT6333_VCORE_CON27),
        MT6333_TEST_OMUX_CON0, mt6333_get_reg_value(MT6333_TEST_OMUX_CON0),
        MT6333_TESTO_CON0, mt6333_get_reg_value(MT6333_TESTO_CON0));
}

void vcore_vosel_sub_1(void)
{
    int j=0;
    
    for(j=0;j<=MT6333_PMIC_VCORE_VOSEL_SLEEP_MASK;j++)
    {                            
        mt6333_set_vcore_vosel_sleep(j);
        printk("[mt6333_set_vcore_vosel_sleep] j=%d, ",j);
        if(j==0)
            mdelay(500);
        read_adc_value(VCORE_INDEX);
    }
}

void vcore_vosel_sub_2(void)
{
    int j=0; 
    
    mt6333_set_rg_vcore_vosel_set_spm(0);
    printk("[mt6333_set_vcore_vosel] mt6333_set_rg_vcore_vosel_set_spm(0); Reg[%x]=0x%x \n", 
        MT6333_VRF18_CON22, mt6333_get_reg_value(MT6333_VRF18_CON22));
    
    for(j=0;j<=MT6333_PMIC_VCORE_VOSEL_ON_MASK;j++)
    {                            
        mt6333_set_vcore_vosel_on(j);
        printk("[mt6333_set_vcore_vosel_on] j=%d, ",j);
        if(j==0)
            mdelay(500);
        read_adc_value(VCORE_INDEX);
    }
    
    mt6333_set_rg_vcore_vosel_set_spm(1);
    printk("[mt6333_set_vcore_vosel] mt6333_set_rg_vcore_vosel_set_spm(1); Reg[%x]=0x%x \n", 
        MT6333_VRF18_CON22, mt6333_get_reg_value(MT6333_VRF18_CON22));
    
    for(j=0;j<=MT6333_PMIC_VCORE_VOSEL_ON_SPM_MASK;j++)
    {                            
        mt6333_set_vcore_vosel_on_spm(j);
        printk("[mt6333_set_vcore_vosel_on_spm] j=%d, ",j);
        if(j==0)
            mdelay(500);
        read_adc_value(VCORE_INDEX);
    }
}

void exec_vcore_vosel_test(int index_val)
{
    int i,j;
    
    for(i=0;i<=1;i++)
    {
        printk("[mt6333_set_vcore_vosel_ctrl] %d\n", i);
        mt6333_set_vcore_vosel_ctrl(i);        
        
        switch(i){
            case 0:
                for(j=0;j<=MT6333_PMIC_VCORE_VOSEL_MASK;j++)
                {                            
                    mt6333_set_vcore_vosel(j);
                    printk("[mt6333_set_vcore_vosel] j=%d, ",j);
                    if(j==0)
                        mdelay(500);
                    read_adc_value(VCORE_INDEX);
                }
                break;    

            case 1:
                printk("[mt6333_set_vcore_vosel] set VSLEEP_EN=0, THR_SDN=0\n"); //0                
                vcore_vosel_sub_1_setting(0,0);
                vcore_vosel_sub_2();                
                
                mt6333_set_vcore_vosel_sleep(0x37);
                
                printk("[mt6333_set_vcore_vosel] set VSLEEP_EN=0, THR_SDN=1\n"); //1
                vcore_vosel_sub_1_setting(0,1);
                vcore_vosel_sub_1();
                
                printk("[mt6333_set_vcore_vosel] set VSLEEP_EN=1, THR_SDN=0\n"); //1
                vcore_vosel_sub_1_setting(1,0);
                vcore_vosel_sub_1();
                
                printk("[mt6333_set_vcore_vosel] set VSLEEP_EN=1, THR_SDN=1\n"); //1               
                vcore_vosel_sub_1_setting(1,1);
                vcore_vosel_sub_1();
                break;

            default:
                printk("At %d, Invalid channel value(%d)\n", index_val, i);
                break;    
        }            
    }  
}

void exec_vmem_vosel_test(int index_val)
{
    int i,j;
    
    for(i=0;i<=1;i++)
    {
        printk("[mt6333_set_vmem_vosel_ctrl] %d\n", i);
        mt6333_set_vmem_vosel_ctrl(i);        
        
        switch(i){
            case 0:
                for(j=0;j<=MT6333_PMIC_VMEM_VOSEL_MASK;j++)
                {                            
                    mt6333_set_vmem_vosel(j);
                    printk("[mt6333_set_vmem_vosel] j=%d, ",j);
                    if(j==0)
                        mdelay(500);
                    read_adc_value(VMEM_INDEX);
                }
                break;    

            case 1:
                mt6333_config_interface(MT6333_TEST_OMUX_CON2, 1, 0x1, 0); // SRCLKEN SW mode
                printk("[SRCLKEN] Reg[0x%x]=0x%x\n", MT6333_TEST_OMUX_CON2, mt6333_get_reg_value(MT6333_TEST_OMUX_CON2));                    
                //-----------------------------------------

                mt6333_set_vmem_vosel_sleep(0x27);
    
                mt6333_config_interface(MT6333_TESTO_CON2, 0, 0x1, 0);
                printk("[SRCLKEN=0] Reg[0x%x]=0x%x\n", MT6333_TESTO_CON2, mt6333_get_reg_value(MT6333_TESTO_CON2));
                for(j=0;j<=MT6333_PMIC_VMEM_VOSEL_SLEEP_MASK;j++)
                {                            
                    mt6333_set_vmem_vosel_sleep(j);
                    printk("[mt6333_set_vmem_vosel_sleep] j=%d, ",j);
                    if(j==0)
                        mdelay(500);
                    read_adc_value(VMEM_INDEX);
                }
                //-----------------------------------------
                mt6333_config_interface(MT6333_TESTO_CON2, 1, 0x1, 0);
                printk("[SRCLKEN=1] Reg[0x%x]=0x%x\n", MT6333_TESTO_CON2, mt6333_get_reg_value(MT6333_TESTO_CON2));
                for(j=0;j<=MT6333_PMIC_VMEM_VOSEL_ON_MASK;j++)
                {                            
                    mt6333_set_vmem_vosel_on(j);
                    printk("[mt6333_set_vmem_vosel_on] j=%d, ",j);
                    if(j==0)
                        mdelay(500);
                    read_adc_value(VMEM_INDEX);
                }
                break;

            default:
                printk("At %d, Invalid channel value(%d)\n", index_val, i);
                break;    
        }            
    }  
}

void exec_vrf18_vosel_sub_test(void)
{
    int j=0;
    
    mt6333_config_interface(MT6333_TEST_OMUX_CON2, 1, 0x1, 0); // SRCLKEN SW mode
    printk("[SRCLKEN] Reg[0x%x]=0x%x\n", MT6333_TEST_OMUX_CON2, mt6333_get_reg_value(MT6333_TEST_OMUX_CON2));
    
    mt6333_config_interface(MT6333_TESTO_CON2, 0, 0x1, 0);
    printk("[SRCLKEN=0] Reg[0x%x]=0x%x\n", MT6333_TESTO_CON2, mt6333_get_reg_value(MT6333_TESTO_CON2));

    //for(j=0;j<=MT6333_PMIC_VRF18_VOSEL_SLEEP_MASK;j++)
    for(j=MT6333_PMIC_VRF18_VOSEL_SLEEP_MASK;j>=0;j--)
    {                            
        mt6333_set_vrf18_vosel_sleep(j);
        printk("[mt6333_set_vrf18_vosel_sleep] j=%d, ",j);
        if(j==0)
            mdelay(500);
        read_adc_value(VRF18_INDEX);
    }

    mt6333_config_interface(MT6333_TESTO_CON2, 1, 0x1, 0);
    printk("[SRCLKEN=1] Reg[0x%x]=0x%x\n", MT6333_TESTO_CON2, mt6333_get_reg_value(MT6333_TESTO_CON2));
    for(j=0;j<=MT6333_PMIC_VRF18_VOSEL_ON_MASK;j++)
    {                            
        mt6333_set_vrf18_vosel_on(j);
        printk("[mt6333_set_vrf18_vosel_on] j=%d, ",j);
        if(j==0)
            mdelay(500);
        read_adc_value(VRF18_INDEX);
    }    
}

void exec_vrf18_vosel_test(int index_val)
{
    int i,j;
    
    for(i=0;i<=1;i++)
    {
        printk("[mt6333_set_vrf18_vosel_ctrl] %d\n", i);
        mt6333_set_vrf18_vosel_ctrl(i);        
        
        switch(i){
            case 0:
                for(j=0;j<=MT6333_PMIC_VRF18_VOSEL_MASK;j++)
                {                            
                    mt6333_set_vrf18_vosel(j);
                    printk("[mt6333_set_vrf18_vosel] j=%d, ",j);
                    if(j==0)
                        mdelay(500);
                    read_adc_value(VRF18_INDEX);
                }
                break;    

            case 1:
                exec_vrf18_vosel_sub_test();
                break;

            default:
                printk("At %d, Invalid channel value(%d)\n", index_val, i);
                break;    
        }            
    }  
}

void PMIC_BUCK_VOSEL(int index_val)
{    
    printk("[PMIC_BUCK_VOSEL] start....\n");

    mt6333_config_interface(MT6333_TESTO_CON2, 1, 0x1, 0); // SRCLKEN=1
    printk("[SRCLKEN=1] Reg[0x%x]=0x%x\n", MT6333_TESTO_CON2, mt6333_get_reg_value(MT6333_TESTO_CON2));

    mt6333_config_interface(MT6333_TEST_OMUX_CON2, 1, 0x1, 0); // SRCLKEN SW mode
    printk("[SRCLKEN] Reg[0x%x]=0x%x\n", MT6333_TEST_OMUX_CON2, mt6333_get_reg_value(MT6333_TEST_OMUX_CON2));

    mt6333_set_vcore_en_ctrl(0);
    mt6333_set_vmem_en_ctrl(0);
    mt6333_set_vrf18_en_ctrl(0);

    switch(index_val){
      case 0:
        mt6333_set_vcore_en(1);
        exec_vcore_vosel_test(index_val);
        break;

      case 1:
        mt6333_set_vmem_en(1);
        exec_vmem_vosel_test(index_val);
        break;

      case 2:
        mt6333_set_vrf18_en(1);
        exec_vrf18_vosel_test(index_val);
        break;

       
	  default:
        printk("[PMIC_BUCK_VOSEL] Invalid channel value(%d)\n", index_val);
        break;
        
    }
    
    printk("[PMIC_BUCK_VOSEL] end....\n");
}

void exec_vcore_dlc_subtest(void)
{
    int i; 

    printk("[exec_vcore_dlc_subtest]\n");
    
    for(i=0;i<=MT6333_PMIC_VCORE_DLC_SLEEP_MASK;i++)
    {
        mt6333_set_vcore_dlc_sleep(i);
        mt6333_set_vcore_dlc_n_sleep(i);

        printk("[exec_vcore_dlc_subtest] mt6333_set_vcore_dlc_sleep=%d, mt6333_set_vcore_dlc_n_sleep=%d\n", i, i);

        printk("[exec_vcore_dlc_subtest] mt6333_get_qi_vcore_dlc=%d, mt6333_get_qi_vcore_dlc_n=%d\n",
            mt6333_get_qi_vcore_dlc(), mt6333_get_qi_vcore_dlc_n());

    }

    printk("\n");

    for(i=0;i<=MT6333_PMIC_VCORE_DLC_ON_MASK;i++)
    {
        mt6333_set_vcore_dlc_on(i);
        mt6333_set_vcore_dlc_n_on(i);

        printk("[exec_vcore_dlc_subtest] mt6333_set_vcore_dlc_on=%d, mt6333_set_vcore_dlc_n_on=%d\n", i, i);

        printk("[exec_vcore_dlc_subtest] mt6333_get_qi_vcore_dlc=%d, mt6333_get_qi_vcore_dlc_n=%d\n",
            mt6333_get_qi_vcore_dlc(), mt6333_get_qi_vcore_dlc_n());                   
    }

    printk("\n");
}

void exec_vmem_dlc_subtest(void)
{
    int i; 

    printk("[exec_vmem_dlc_subtest]\n");
    
    for(i=0;i<=MT6333_PMIC_VMEM_DLC_SLEEP_MASK;i++)
    {
        mt6333_set_vmem_dlc_sleep(i);
        mt6333_set_vmem_dlc_n_sleep(i);

        printk("[exec_vmem_dlc_subtest] mt6333_set_vmem_dlc_sleep=%d, mt6333_set_vmem_dlc_n_sleep=%d\n", i, i);

        printk("[exec_vmem_dlc_subtest] mt6333_get_qi_vmem_dlc=%d, mt6333_get_qi_vmem_dlc_n=%d\n",
            mt6333_get_qi_vmem_dlc(), mt6333_get_qi_vmem_dlc_n());

    }

    printk("\n");

    for(i=0;i<=MT6333_PMIC_VMEM_DLC_ON_MASK;i++)
    {
        mt6333_set_vmem_dlc_on(i);
        mt6333_set_vmem_dlc_n_on(i);

        printk("[exec_vmem_dlc_subtest] mt6333_set_vmem_dlc_on=%d, mt6333_set_vmem_dlc_n_on=%d\n", i, i);

        printk("[exec_vmem_dlc_subtest] mt6333_get_qi_vmem_dlc=%d, mt6333_get_qi_vmem_dlc_n=%d\n",
            mt6333_get_qi_vmem_dlc(), mt6333_get_qi_vmem_dlc_n());                   
    }

    printk("\n");
}

void exec_vrf18_dlc_subtest(void)
{
    int i; 

    printk("[exec_vrf18_dlc_subtest]\n");
    
    for(i=0;i<=MT6333_PMIC_VRF18_DLC_SLEEP_MASK;i++)
    {
        mt6333_set_vrf18_dlc_sleep(i);
        mt6333_set_vrf18_dlc_n_sleep(i);

        printk("[exec_vrf18_dlc_subtest] mt6333_set_vrf18_dlc_sleep=%d, mt6333_set_vrf18_dlc_n_sleep=%d\n", i, i);

        printk("[exec_vrf18_dlc_subtest] mt6333_get_qi_vrf18_dlc=%d, mt6333_get_qi_vrf18_dlc_n=%d\n",
            mt6333_get_qi_vrf18_dlc(), mt6333_get_qi_vrf18_dlc_n());

    }

    printk("\n");

    for(i=0;i<=MT6333_PMIC_VRF18_DLC_ON_MASK;i++)
    {
        mt6333_set_vrf18_dlc_on(i);
        mt6333_set_vrf18_dlc_n_on(i);

        printk("[exec_vrf18_dlc_subtest] mt6333_set_vrf18_dlc_on=%d, mt6333_set_vrf18_dlc_n_on=%d\n", i, i);

        printk("[exec_vrf18_dlc_subtest] mt6333_get_qi_vrf18_dlc=%d, mt6333_get_qi_vrf18_dlc_n=%d\n",
            mt6333_get_qi_vrf18_dlc(), mt6333_get_qi_vrf18_dlc_n());                   
    }

    printk("\n");
}

void exec_dlc_subtest(int index_val)
{
    switch(index_val){
        case 0:
            exec_vcore_dlc_subtest();
            break;
        case 1:
            exec_vmem_dlc_subtest();
            break;
        case 2:
            exec_vrf18_dlc_subtest();
            break;
        default:
            printk("[exec_dlc_subtest] Invalid channel value(%d)\n", index_val);
            break;     
    }
}

void exec_scrxxx_map_dlc(int index_val)
{
    mt6333_config_interface(MT6333_TEST_OMUX_CON2, 1, 0x1, 0); // SRCLKEN SW mode
    printk("[SRCLKEN] Reg[0x%x]=0x%x\n", MT6333_TEST_OMUX_CON2, mt6333_get_reg_value(MT6333_TEST_OMUX_CON2));
    
    mt6333_config_interface(MT6333_TESTO_CON2, 0, 0x1, 0);
    printk("[SRCLKEN=0] Reg[0x%x]=0x%x\n", MT6333_TESTO_CON2, mt6333_get_reg_value(MT6333_TESTO_CON2));
    exec_dlc_subtest(index_val);

    mt6333_config_interface(MT6333_TESTO_CON2, 1, 0x1, 0);
    printk("[SRCLKEN=1] Reg[0x%x]=0x%x\n", MT6333_TESTO_CON2, mt6333_get_reg_value(MT6333_TESTO_CON2));
    exec_dlc_subtest(index_val);
}

void exec_vcore_dlc_test(int index_val)
{
    int i,j;
    
    for(i=0;i<=1;i++)
    {
        printk("[exec_vcore_dlc_test] %d\n", i);
        mt6333_set_vcore_dlc_ctrl(i);        //0: sw mode , 1: hw mode
        
        switch(i){
            case 0:
                for(j=0;j<=MT6333_PMIC_VCORE_DLC_MASK;j++)
                {
                    mt6333_set_vcore_dlc(j);
                    mt6333_set_vcore_dlc_n(j);

                    if( (mt6333_get_qi_vcore_dlc()!=j) || (mt6333_get_qi_vcore_dlc_n()!=j) )
                    {
                        printk("[exec_vcore_dlc_test] fail at mt6333_get_qi_vcore_dlc=%d, mt6333_get_qi_vcore_dlc_n=%d\n",
                            mt6333_get_qi_vcore_dlc(), mt6333_get_qi_vcore_dlc_n());
                    }

                    printk("[exec_vcore_dlc_test] mt6333_set_vcore_dlc=%d, mt6333_set_vcore_dlc_n=%d, mt6333_get_qi_vcore_dlc=%d, mt6333_get_qi_vcore_dlc_n=%d\n",
                            j, j, mt6333_get_qi_vcore_dlc(), mt6333_get_qi_vcore_dlc_n());
                }
                break;    

            case 1:
                exec_scrxxx_map_dlc(index_val);
                break;

            default:
                printk("BUCK=%d, Invalid channel value(%d)\n", index_val, i);
                break;    
        }            
    }    
}

void exec_vmem_dlc_test(int index_val)
{
    int i,j;
    
    for(i=0;i<=1;i++)
    {
        printk("[exec_vmem_dlc_test] %d\n", i);
        mt6333_set_vmem_dlc_ctrl(i);        //0: sw mode , 1: hw mode
        
        switch(i){
            case 0:
                for(j=0;j<=MT6333_PMIC_VMEM_DLC_MASK;j++)
                {
                    mt6333_set_vmem_dlc(j);
                    mt6333_set_vmem_dlc_n(j);

                    if( (mt6333_get_qi_vmem_dlc()!=j) || (mt6333_get_qi_vmem_dlc_n()!=j) )
                    {
                        printk("[exec_vmem_dlc_test] fail at mt6333_get_qi_vmem_dlc=%d, mt6333_get_qi_vmem_dlc_n=%d\n",
                            mt6333_get_qi_vmem_dlc(), mt6333_get_qi_vmem_dlc_n());
                    }

                    printk("[exec_vmem_dlc_test] mt6333_set_vmem_dlc=%d, mt6333_set_vmem_dlc_n=%d, mt6333_get_qi_vmem_dlc=%d, mt6333_get_qi_vmem_dlc_n=%d\n",
                            j, j, mt6333_get_qi_vmem_dlc(), mt6333_get_qi_vmem_dlc_n());
                }
                break;    

            case 1:
                exec_scrxxx_map_dlc(index_val);
                break;

            default:
                printk("BUCK=%d, Invalid channel value(%d)\n", index_val, i);
                break;    
        }            
    }    
}

void exec_vrf18_dlc_test(int index_val)
{
    int i,j;
    
    for(i=0;i<=1;i++)
    {
        printk("[exec_vrf18_dlc_test] %d\n", i);
        mt6333_set_vrf18_dlc_ctrl(i);        //0: sw mode , 1: hw mode
        
        switch(i){
            case 0:
                for(j=0;j<=MT6333_PMIC_VRF18_DLC_MASK;j++)
                {
                    mt6333_set_vrf18_dlc(j);
                    mt6333_set_vrf18_dlc_n(j);

                    if( (mt6333_get_qi_vrf18_dlc()!=j) || (mt6333_get_qi_vrf18_dlc_n()!=j) )
                    {
                        printk("[exec_vrf18_dlc_test] fail at mt6333_get_qi_vrf18_dlc=%d, mt6333_get_qi_vrf18_dlc_n=%d\n",
                            mt6333_get_qi_vrf18_dlc(), mt6333_get_qi_vrf18_dlc_n());
                    }

                    printk("[exec_vrf18_dlc_test] mt6333_set_vrf18_dlc=%d, mt6333_set_vrf18_dlc_n=%d, mt6333_get_qi_vrf18_dlc=%d, mt6333_get_qi_vrf18_dlc_n=%d\n",
                            j, j, mt6333_get_qi_vrf18_dlc(), mt6333_get_qi_vrf18_dlc_n());
                }
                break;    

            case 1:
                exec_scrxxx_map_dlc(index_val);
                break;

            default:
                printk("BUCK=%d, Invalid channel value(%d)\n", index_val, i);
                break;    
        }            
    }    
}

void PMIC_BUCK_DLC(int index_val)
{
    printk("[PMIC_BUCK_DLC] start....\n");

    mt6333_config_interface(MT6333_TEST_OMUX_CON2, 1, 0x1, 0); // SRCLKEN SW mode
    printk("[SRCLKEN] Reg[0x%x]=0x%x\n", MT6333_TEST_OMUX_CON2, mt6333_get_reg_value(MT6333_TEST_OMUX_CON2));

    mt6333_set_vcore_en_ctrl(0);
    mt6333_set_vmem_en_ctrl(0);
    mt6333_set_vrf18_en_ctrl(0);

    switch(index_val){
      case 0:
        mt6333_set_vcore_en(1);
        exec_vcore_dlc_test(index_val);
        break;

      case 1:
        mt6333_set_vmem_en(1);
        exec_vmem_dlc_test(index_val);
        break;

      case 2:
        mt6333_set_vrf18_en(1);
        exec_vrf18_dlc_test(index_val);
        break;
       
	  default:
        printk("[PMIC_BUCK_DLC] Invalid channel value(%d)\n", index_val);
        break;
        
    }
    
    printk("[PMIC_BUCK_DLC] end....\n");
}

void exec_vcore_burst_subtest(void)
{
    int i; 

    printk("[exec_vcore_burst_subtest]\n");

    for(i=0;i<=MT6333_PMIC_VCORE_BURST_SLEEP_MASK;i++)
    {
        mt6333_set_vcore_burst_sleep(i);

        printk("[exec_vcore_burst_subtest] mt6333_set_vcore_bursth_sleep=%d\n", i);

        printk("[exec_vcore_burst_subtest] mt6333_get_qi_vcore_burst=%d\n",
            mt6333_get_qi_vcore_burst());

    }

    printk("\n");

    for(i=0;i<=MT6333_PMIC_VCORE_BURST_ON_MASK;i++)
    {
        mt6333_set_vcore_burst_on(i);

        printk("[exec_vcore_burst_subtest] mt6333_set_vcore_burst_on=%d\n", i);

        printk("[exec_vcore_burst_subtest] mt6333_get_qi_vcore_burst=%d\n",
            mt6333_get_qi_vcore_burst());                   
    }  

    printk("\n");
}

void exec_vmem_burst_subtest(void)
{
    int i; 

    printk("[exec_vmem_burst_subtest]\n");

    for(i=0;i<=MT6333_PMIC_VMEM_BURST_SLEEP_MASK;i++)
    {
        mt6333_set_vmem_burst_sleep(i);

        printk("[exec_vmem_burst_subtest] mt6333_set_vmem_bursth_sleep=%d\n", i);

        printk("[exec_vmem_burst_subtest] mt6333_get_qi_vmem_burst=%d\n",
            mt6333_get_qi_vmem_burst());

    }

    printk("\n");

    for(i=0;i<=MT6333_PMIC_VMEM_BURST_ON_MASK;i++)
    {
        mt6333_set_vmem_burst_on(i);

        printk("[exec_vmem_burst_subtest] mt6333_set_vmem_burst_on=%d\n", i);

        printk("[exec_vmem_burst_subtest] mt6333_get_qi_vmem_burst=%d\n",
            mt6333_get_qi_vmem_burst());                   
    }  

    printk("\n");
}

void exec_burst_subtest(int index_val)
{
    switch(index_val){
        case 0:
            exec_vcore_burst_subtest();
            break;
        case 1:
            exec_vmem_burst_subtest();
            break;
        case 2:
            //exec_vrf18_burst_subtest(); // no function
            break;
        default:
            printk("[exec_burst_subtest] Invalid channel value(%d)\n", index_val);
            break;     
    }
}

void exec_scrxxx_map_bursth(int index_val)
{
    mt6333_config_interface(MT6333_TEST_OMUX_CON2, 1, 0x1, 0); // SRCLKEN SW mode
    printk("[SRCLKEN] Reg[0x%x]=0x%x\n", MT6333_TEST_OMUX_CON2, mt6333_get_reg_value(MT6333_TEST_OMUX_CON2));
    
    mt6333_config_interface(MT6333_TESTO_CON2, 0, 0x1, 0);
    printk("[SRCLKEN=0] Reg[0x%x]=0x%x\n", MT6333_TESTO_CON2, mt6333_get_reg_value(MT6333_TESTO_CON2));
    exec_burst_subtest(index_val);

    mt6333_config_interface(MT6333_TESTO_CON2, 1, 0x1, 0);
    printk("[SRCLKEN=1] Reg[0x%x]=0x%x\n", MT6333_TESTO_CON2, mt6333_get_reg_value(MT6333_TESTO_CON2));
    exec_burst_subtest(index_val);
}

void exec_vcore_burst_test(int index_val)
{
    int i,j;
    
    for(i=0;i<=1;i++)
    {
        printk("[exec_vcore_burst_test] %d\n", i);
        mt6333_set_vcore_burst_ctrl(i);        
        
        switch(i){
            case 0:
                for(j=0;j<=MT6333_PMIC_VCORE_BURST_MASK;j++)
                {
                    mt6333_set_vcore_burst(j);

                    if( (mt6333_get_qi_vcore_burst()!=j) )
                    {
                        printk("[exec_vcore_burst_test] fail at mt6333_get_qi_vcore_burst=%d\n",
                            mt6333_get_qi_vcore_burst());
                    }

                    printk("[exec_vcore_burst_test] mt6333_set_vcore_burst=%d, mt6333_get_qi_vcore_burst=%d\n",
                            j, mt6333_get_qi_vcore_burst());
                }
                break;    

            case 1:
                exec_scrxxx_map_bursth(index_val);
                break;

            default:
                printk("At %d, Invalid channel value(%d)\n", index_val, i);
                break;    
        }            
    }    
}

void exec_vmem_burst_test(int index_val)
{
    int i,j;
    
    for(i=0;i<=1;i++)
    {
        printk("[exec_vmem_burst_test] %d\n", i);
        mt6333_set_vmem_burst_ctrl(i);        
        
        switch(i){
            case 0:
                for(j=0;j<=MT6333_PMIC_VMEM_BURST_MASK;j++)
                {
                    mt6333_set_vmem_burst(j);

                    if( (mt6333_get_qi_vmem_burst()!=j) )
                    {
                        printk("[exec_vmem_burst_test] fail at mt6333_get_qi_vmem_burst=%d\n",
                            mt6333_get_qi_vmem_burst());
                    }

                    printk("[exec_vmem_burst_test] mt6333_set_vmem_burst=%d, mt6333_get_qi_vmem_burst=%d\n",
                            j, mt6333_get_qi_vmem_burst());
                }
                break;    

            case 1:
                exec_scrxxx_map_bursth(index_val);
                break;

            default:
                printk("At %d, Invalid channel value(%d)\n", index_val, i);
                break;    
        }            
    }    
}

void PMIC_BUCK_BURST(int index_val)
{    
    printk("[PMIC_BUCK_BURST] start....\n");
    
    mt6333_config_interface(MT6333_TEST_OMUX_CON2, 1, 0x1, 0); // SRCLKEN SW mode
    printk("[SRCLKEN] Reg[0x%x]=0x%x\n", MT6333_TEST_OMUX_CON2, mt6333_get_reg_value(MT6333_TEST_OMUX_CON2));

    mt6333_set_vcore_en_ctrl(0);
    mt6333_set_vmem_en_ctrl(0);
    mt6333_set_vrf18_en_ctrl(0);

    switch(index_val){
      case 0:
        mt6333_set_vcore_en(1);
        exec_vcore_burst_test(index_val);
        break;

      case 1:
        mt6333_set_vmem_en(1);
        exec_vmem_burst_test(index_val);
        break;

      case 2:
        //mt6333_set_vrf18_en(1);
        //exec_vrf18_burst_test(index_val);
        break;

	  default:
        printk("[PMIC_BUCK_BURST] Invalid channel value(%d)\n", index_val);
        break;
        
    }
    
    printk("[PMIC_BUCK_BURST] end....\n");
}

void tc_2101(void)
{
    printk("[tc_2101] \n");
    PMIC_BUCK_ON_OFF(VRF18_INDEX);
    printk("[tc_2101] Done\n");
}

void tc_2102(void)
{
    printk("[tc_2102] \n");
    PMIC_BUCK_DLC(VRF18_INDEX);     
    printk("[tc_2102] Done\n");
}

void tc_2103(void)
{
    printk("[tc_2103] \n");
    PMIC_BUCK_VOSEL(VRF18_INDEX);
    printk("[tc_2103] Done\n");
}

void tc_2202(void)
{
    printk("[tc_2202] \n");
    PMIC_BUCK_BURST(VCORE_INDEX); 
    printk("[tc_2202] Done\n");
}

void tc_2203(void)
{
    printk("[tc_2203] \n");
    PMIC_BUCK_DLC(VCORE_INDEX);
    printk("[tc_2203] Done\n");
}

void tc_2207(void)
{
    printk("[tc_2207] \n");
    PMIC_BUCK_ON_OFF(VCORE_INDEX);
    printk("[tc_2207] Done\n");
}

void tc_2208(void)
{
    printk("[tc_2208] integrate into tc_2207\n");
    PMIC_BUCK_VOSEL(VCORE_INDEX);
    printk("[tc_2208] Done\n");
}

void tc_2209(void)
{
    printk("[tc_2209] integrate into tc_2207\n");
    read_adc_value(3);
    printk("[tc_2209] Done\n");
}

void tc_2302(void)
{
    printk("[tc_2302] \n");
    PMIC_BUCK_BURST(VMEM_INDEX); 
    printk("[tc_2302] Done\n");
}

void tc_2303(void)
{
    printk("[tc_2303] \n");
    PMIC_BUCK_DLC(VMEM_INDEX);
    printk("[tc_2303] Done\n");
}

void tc_2306(void)
{
    printk("[tc_2306] \n");
    PMIC_BUCK_ON_OFF(VMEM_INDEX);
    printk("[tc_2306] Done\n");
}

void tc_2307(void)
{
    printk("[tc_2307] \n");
    PMIC_BUCK_VOSEL(VMEM_INDEX);
    printk("[tc_2307] Done\n");
}

//----------------------------------------------------------------

int is_charger_exist(void)
{
    mdelay(500);

    printk("[is_charger_exist] RGS_CHRDET=%d\n", mt6333_get_rgs_chrdet());
    printk("[is_charger_exist] RGS_CHR_PLUG_IN=%d\n", mt6333_get_rgs_chr_plug_in());
    printk("[is_charger_exist] upmu_get_rgs_chrdet=%d\n", upmu_get_rgs_chrdet());

    //return upmu_get_rgs_chrdet();    
    return mt6333_get_rgs_chrdet();
}

int is_swchr_wdt_timeout(void)
{
    printk("[is_swchr_wdt_timeout] Reg[0x%x]=0x%x\n", MT6333_STA_CON8, mt6333_get_reg_value(MT6333_STA_CON8));
    printk("[is_swchr_wdt_timeout] Reg[0x%x]=0x%x\n", MT6333_CHRWDT_CON0, mt6333_get_reg_value(MT6333_CHRWDT_CON0));
    printk("[is_swchr_wdt_timeout] Reg[0x%x]=0x%x\n", MT6333_CHRWDT_STATUS0, mt6333_get_reg_value(MT6333_CHRWDT_STATUS0));
    
    //return mt6333_get_rg_chrwdt_flag();
    return mt6333_get_rgs_chrwdt_tout();
}

void tc_3111(void)
{
    printk("[tc_3111] \n");
    printk("[tc_3111] 1.1.1 Plug-in/out charger check charger plug-in status\n");    

    while(is_charger_exist()==0)
    {
        printk("please plug-in CHARGER!\n");
        mdelay(500);
    }
    
    printk("[tc_3111] compare the following registers after charger plug-in\n");
    printk("[tc_3111] RGS_CHR_PLUG_IN    need 1 => %d\n", mt6333_get_rgs_chr_plug_in());
    printk("[tc_3111] RGS_CHR_MODE       need 1 => %d\n", mt6333_get_rgs_chr_mode());
    printk("[tc_3111] RGS_POWER_ON_READY need 1 => %d\n", mt6333_get_rgs_power_on_ready());    
    printk("[tc_3111] \n");

    while(is_charger_exist()==1)
    {
        printk("please plug-out CHARGER!\n");
        mdelay(500);
    }
    
    printk("[tc_3111] compare the following register after charger plug-out\n");
    printk("[tc_3111] RGS_CHR_PLUG_IN    need 0 => %d\n", mt6333_get_rgs_chr_plug_in());
    printk("[tc_3111] RGS_CHR_MODE       need 0 => %d\n", mt6333_get_rgs_chr_mode());
    printk("[tc_3111] RGS_POWER_ON_READY need 0 => %d\n", mt6333_get_rgs_power_on_ready());
    printk("[tc_3111] \n");
    
    printk("[tc_3111] Done\n");
}

void tc_3112(void)
{
    kal_uint32 ret = 0;
    int polling_timeout_value = 10;
    int polling_time = 0;
    
    printk("[tc_3112] \n");
    printk("[tc_3112] 1.1.2 Check PWM, CP and M3 status after charger plug-in at CC mode (RG_CHR_EN=1 and 3.0V < VBAT < CV_Vth)\n");    

    while(is_charger_exist()==0)
    {
        printk("please plug-in CHARGER!\n");
        mdelay(500);
    }

    while(mt6333_get_rgs_power_on_ready() != 1)
    {
        if(polling_time++ >= polling_timeout_value)
        {
            printk("check rgs_power_on_ready fail\n");
            break;
        }
    }
    printk("polling_time=%d of rgs_power_on_ready\n", polling_time);

    mt6333_set_rg_usbdl_mode_b(1);
    ret=mt6333_config_interface(0x04, 0x95, 0xFF, 0x0);

    printk("[tc_3112] compare the following registers after charger plug-in\n");
    printk("[tc_3112] mt6333_get_rgs_pwm_en                need 1 => %d\n", mt6333_get_rgs_pwm_en());
    printk("[tc_3112] mt6333_get_rgs_cpcstsys_en           need 1 => %d\n", mt6333_get_rgs_cpcstsys_en());
    printk("[tc_3112] mt6333_get_rgs_m3_en                 need 1 => %d\n", mt6333_get_rgs_m3_en());
    printk("[tc_3112] mt6333_get_rgs_m3_s_en               need 1 => %d\n", mt6333_get_rgs_m3_s_en());
    printk("[tc_3112] mt6333_get_rgs_m3_r_en               need 1 => %d\n", mt6333_get_rgs_m3_r_en());
    printk("[tc_3112] mt6333_get_rgs_m3_boost_en           need 0 => %d\n", mt6333_get_rgs_m3_boost_en());
    printk("[tc_3112] mt6333_get_rgs_pwm_voltage_config    need 0 => %d\n", mt6333_get_rgs_pwm_voltage_config());
    printk("[tc_3112] mt6333_get_rgs_pwm_current_config    need 1 => %d\n", mt6333_get_rgs_pwm_current_config());
    printk("[tc_3112] \n");

    while(is_charger_exist()==1)
    {
        printk("please plug-out CHARGER!\n");
        mdelay(500);
    }
    
    printk("[tc_3112] \n");
    
    printk("[tc_3112] Done\n");
}

void tc_3113(void)
{
    kal_uint32 ret = 0;
    int polling_timeout_value = 10;
    int polling_time = 0;
    
    printk("[tc_3113] \n");
    printk("[tc_3113] 1.1.3 Check PWM, CP and M3 status at chr_suspend mode (RG_CHR_EN=0 and CC_FLAG,3.4V < VBAT < CV_Vth) \n");    

    while(is_charger_exist()==0)
    {
        printk("please plug-in CHARGER!\n");
        mdelay(500);
    }

    while(mt6333_get_rgs_power_on_ready() != 1)
    {
        if(polling_time++ >= polling_timeout_value)
        {
            printk("check rgs_power_on_ready fail\n");
            break;
        }
    }
    printk("polling_time=%d of rgs_power_on_ready\n", polling_time);

    mt6333_set_rg_usbdl_mode_b(1);
    ret=mt6333_config_interface(0x04, 0x94, 0xFF, 0x0);

    printk("[tc_3113] compare the following registers after charger plug-in\n");
    printk("[tc_3113] mt6333_get_rgs_pwm_en                need 0 => %d\n", mt6333_get_rgs_pwm_en());
    printk("[tc_3113] mt6333_get_rgs_cpcstsys_en           need 0 => %d\n", mt6333_get_rgs_cpcstsys_en());
    printk("[tc_3113] mt6333_get_rgs_m3_en                 need 0 => %d\n", mt6333_get_rgs_m3_en());
    printk("[tc_3113] mt6333_get_rgs_m3_s_en               need 0 => %d\n", mt6333_get_rgs_m3_s_en());
    printk("[tc_3113] mt6333_get_rgs_m3_r_en               need 0 => %d\n", mt6333_get_rgs_m3_r_en());
    printk("[tc_3113] mt6333_get_rgs_m3_boost_en           need 0 => %d\n", mt6333_get_rgs_m3_boost_en());
    printk("[tc_3113] mt6333_get_rgs_pwm_voltage_config    need 1 => %d\n", mt6333_get_rgs_pwm_voltage_config());
    printk("[tc_3113] mt6333_get_rgs_pwm_current_config    need 0 => %d\n", mt6333_get_rgs_pwm_current_config());
    printk("[tc_3113] \n");

    while(is_charger_exist()==1)
    {
        printk("please plug-out CHARGER!\n");
        mdelay(500);
    }
    
    printk("[tc_3113] \n");
    
    printk("[tc_3113] Done\n");
}


void tc_3121(void)
{
    printk("[tc_3121] \n");
    printk("[tc_3121] // 1.2.1 Set charger to auto-pwron state, and leave auto-pwron state by RG_USBDL_MODE_B\n");

    while(is_charger_exist()==0)
    {
        printk("please plug-in CHARGER!\n");
        mdelay(500);
    }
    
    printk("[tc_3121] after charger plug-in, polling\n");
    printk("[tc_3121] RGS_AUTO_PWRON         need 1 => %d\n", mt6333_get_rgs_auto_pwron());
    printk("[tc_3121] RGS_AUTO_PWRON_DONE    need 0 => %d\n", mt6333_get_rgs_auto_pwron_done());
    printk("[tc_3121] RGS_POWER_ON_READY     need 1 => %d\n", mt6333_get_rgs_power_on_ready());
    printk("[tc_3121] \n");
    
    printk("[tc_3121] set RG_USBDL_MODE_B = 1\n");
    mt6333_set_rg_usbdl_mode_b(1);
    printk("[tc_3121] \n");

    printk("[tc_3121] compare the registers \n");
    printk("[tc_3121] RGS_AUTO_PWRON         need 0 => %d\n", mt6333_get_rgs_auto_pwron());
    printk("[tc_3121] RGS_AUTO_PWRON_DONE    need 1 => %d\n", mt6333_get_rgs_auto_pwron_done());
    printk("[tc_3121] RGS_POWER_ON_READY     need 1 => %d\n", mt6333_get_rgs_power_on_ready());
    printk("[tc_3121] \n");

    while(is_charger_exist()==1)
    {
        printk("please plug-out CHARGER!\n");
        mdelay(500);
    }

    printk("[tc_3121] Done\n");
}

void tc_3122(void)
{        
    printk("[tc_3122] 1.2.2 Set charger to auto-pwron state, and leave auto-pwron state by watchdog timeout\n");

    while(is_charger_exist()==0)
    {
        printk("please plug-in CHARGER!\n");
        mdelay(500);
    }
    
    printk("[tc_3122] after charger plug-in, polling\n");
    printk("[tc_3122] RGS_AUTO_PWRON         need 1 => %d\n", mt6333_get_rgs_auto_pwron());
    printk("[tc_3122] RGS_AUTO_PWRON_DONE    need 0 => %d\n", mt6333_get_rgs_auto_pwron_done());
    printk("[tc_3122] RGS_POWER_ON_READY     need 1 => %d\n", mt6333_get_rgs_power_on_ready());
    printk("[tc_3122] \n");

    mt6333_set_rg_chrwdt_en(0);
    mt6333_set_rg_chrwdt_wr(1); // write 1 to kick chr wdt
    mt6333_set_rg_chrwdt_td(0); //4 sec
    mt6333_set_rg_chrwdt_en(1);

    while(is_swchr_wdt_timeout()==0)
    {
        printk("swchr wdt is not timeout!\n");
        mdelay(500);
    }
    printk("Reg[0x%x]=0x%x\n", MT6333_CHRWDT_STATUS0, mt6333_get_reg_value(MT6333_CHRWDT_STATUS0));

    printk("[tc_3122] compare the registers \n");
    printk("[tc_3122] RGS_AUTO_PWRON         need 0 => %d\n", mt6333_get_rgs_auto_pwron());
    printk("[tc_3122] RGS_AUTO_PWRON_DONE    need 1 => %d\n", mt6333_get_rgs_auto_pwron_done());
    printk("[tc_3122] RGS_POWER_ON_READY     need 0 => %d\n", mt6333_get_rgs_power_on_ready());
    printk("[tc_3122] \n");

    while(is_charger_exist()==1)
    {
        printk("please plug-out CHARGER!\n");
        mdelay(500);
    }
    
    printk("[tc_3122] Done\n");
}

void tc_3123(void)
{   
    int i = 0;
    
    printk("[tc_3123] Check if unable to enter auto-pwron state once leave auto-pwron state without charger plug-out \n");

    while(is_charger_exist()==0)
    {
        printk("please plug-in CHARGER!\n");
        mdelay(500);
    }

    printk("[tc_3123] after charger plug-in, polling\n");
    printk("[tc_3123] RGS_AUTO_PWRON         need 1 => %d\n", mt6333_get_rgs_auto_pwron());
    printk("[tc_3123] RGS_AUTO_PWRON_DONE    need 0 => %d\n", mt6333_get_rgs_auto_pwron_done());
    printk("[tc_3123] RGS_POWER_ON_READY     need 1 => %d\n", mt6333_get_rgs_power_on_ready());
    printk("[tc_3123] \n");

    printk("[tc_3123] set RG_USBDL_MODE_B = 1\n");
    mt6333_set_rg_usbdl_mode_b(1);
    printk("[tc_3123] RGS_AUTO_PWRON         need 0 => %d\n", mt6333_get_rgs_auto_pwron());
    printk("[tc_3123] RGS_AUTO_PWRON_DONE    need 1 => %d\n", mt6333_get_rgs_auto_pwron_done());
    printk("[tc_3123] RGS_POWER_ON_READY     need 1 => %d\n", mt6333_get_rgs_power_on_ready());

    printk("[tc_3123] set RG_USBDL_MODE_B = 0\n");
    mt6333_set_rg_usbdl_mode_b(0);

    for(i=0;i<=60;i++)
    {
        msleep(1000);
        printk(".");
    }    
    printk("[tc_3123] RGS_AUTO_PWRON         need 0 => %d\n", mt6333_get_rgs_auto_pwron());
    printk("[tc_3123] RGS_AUTO_PWRON_DONE    need 1 => %d\n", mt6333_get_rgs_auto_pwron_done());
    printk("[tc_3123] RGS_POWER_ON_READY     need 1 => %d\n", mt6333_get_rgs_power_on_ready());
    
    while(is_charger_exist()==1)
    {
        printk("please plug-out CHARGER!\n");
        mdelay(500);
    }

    while(is_charger_exist()==0)
    {
        printk("please plug-in CHARGER!\n");
        mdelay(500);
    }

    printk("[tc_3123] RGS_AUTO_PWRON         need 1 => %d\n", mt6333_get_rgs_auto_pwron());
    printk("[tc_3123] RGS_AUTO_PWRON_DONE    need 0 => %d\n", mt6333_get_rgs_auto_pwron_done());
    printk("[tc_3123] RGS_POWER_ON_READY     need 1 => %d\n", mt6333_get_rgs_power_on_ready());
    
    printk("[tc_3123] Done\n");
}

void tc_3141(void)
{
    printk("[tc_3141] \n");
    printk("[tc_3141] reach the cut off current and compare the following register\n");
    printk("[tc_3141] RGS_CHARGE_COMPLETE_HW = %d\n", mt6333_get_rgs_charge_complete_hw());

    mt6333_set_rg_int_en_chr_complete(1);

    printk("[tc_3141] Can do test\n");
    //receive Interrupt
    
    printk("[tc_3141] Done\n");
}

void tc_3211(void)
{   
    kal_uint32 ret=0;
    
    printk("[tc_3211] \n");
    
    printk("[tc_3211] // -- 2.1.1 OTG mode on/off by setting RG_OTG_EN, check the power-on sequence d/a flags\n");                
    printk("[tc_3211] NO CHARGER!\n");    

    printk("[tc_3211] Disable HW CHRIN short auto protection\n");
    ret=mt6333_config_interface(0xF3,0x1,0x1,5);
    ret=mt6333_config_interface(0xF7,0x1,0x1,5);    

    printk("[tc_3211] set RG_OTG_EN = 1\n");    
    ret=mt6333_config_interface(0x30,0x95,0xFF,0);
    mdelay(20);
    
    printk("[tc_3211] RGS_OTG_MODE         need 1 => %d\n", mt6333_get_rgs_otg_mode());
    printk("[tc_3211] RGS_CHR_MODE         need 0 => %d\n", mt6333_get_rgs_chr_mode());
    printk("[tc_3211] RGS_OTG_DRV_EN       need 1 => %d\n", mt6333_get_rgs_otg_drv_en());
    printk("[tc_3211] RSG_OTG_PRECC        need 0 => %d\n", mt6333_get_rgs_otg_precc());
    printk("[tc_3211] RGS_OTG_EN           need 1 => %d\n", mt6333_get_rgs_otg_en());
    printk("[tc_3211] RGS_CPCSTSYS_EN      need 1 => %d\n", mt6333_get_rgs_cpcstsys_en());
    printk("[tc_3211] RGS_M3_EN            need 1 => %d\n", mt6333_get_rgs_m3_en());
    printk("[tc_3211] RGS_M3_S_EN          need 1 => %d\n", mt6333_get_rgs_m3_s_en());
    printk("[tc_3211] RGS_M3_R_EN          need 0 => %d\n", mt6333_get_rgs_m3_r_en());
    printk("[tc_3211] RGS_M3_BOOST_EN      need 1 => %d\n", mt6333_get_rgs_m3_boost_en());
    printk("[tc_3211] RGS_OTG_EN_STB       need 1 => %d\n", mt6333_get_rgs_otg_en_stb());
    printk("[tc_3211] \n");

    printk("[tc_3211] set RG_OTG_EN = 0\n");   
    ret=mt6333_config_interface(0x30,0x94,0xFF,0);
    mdelay(20);
    
    printk("[tc_3211] RGS_OTG_MODE         need 0 => %d\n", mt6333_get_rgs_otg_mode());
    printk("[tc_3211] RGS_CHR_MODE         need 0 => %d\n", mt6333_get_rgs_chr_mode());
    printk("[tc_3211] RGS_OTG_DRV_EN       need 0 => %d\n", mt6333_get_rgs_otg_drv_en());
    printk("[tc_3211] RSG_OTG_PRECC        need 0 => %d\n", mt6333_get_rgs_otg_precc());
    printk("[tc_3211] RGS_OTG_EN           need 0 => %d\n", mt6333_get_rgs_otg_en());
    printk("[tc_3211] RGS_CPCSTSYS_EN      need 0 => %d\n", mt6333_get_rgs_cpcstsys_en());
    printk("[tc_3211] RGS_M3_EN            need 0 => %d\n", mt6333_get_rgs_m3_en());
    printk("[tc_3211] RGS_M3_S_EN          need 0 => %d\n", mt6333_get_rgs_m3_s_en());
    printk("[tc_3211] RGS_M3_R_EN          need 0 => %d\n", mt6333_get_rgs_m3_r_en());
    printk("[tc_3211] RGS_M3_BOOST_EN      need 0 => %d\n", mt6333_get_rgs_m3_boost_en());
    printk("[tc_3211] RGS_OTG_EN_STB       need 0 => %d\n", mt6333_get_rgs_otg_en_stb());
    printk("[tc_3211] \n");

    printk("[tc_3211] Done\n");
}

void tc_3212(void)
{   
    kal_uint32 ret=0;
    int i=0;
    
    printk("[tc_3212] // -- 2.1.2 Switching between charger mode and OTG mode\n");

    printk("[tc_3212] Disable HW CHRIN short auto protection\n");
    ret=mt6333_config_interface(0xF3,0x1,0x1,5);
    ret=mt6333_config_interface(0xF7,0x1,0x1,5);   

#if 1
    //--------------------------------------------------------    
    printk("\n[tc_3212] a -------------------------\n");
    printk("[tc_3212] set RG_OTG_EN = 1\n");    
    ret=mt6333_config_interface(0x30,0x95,0xFF,0);    
    mdelay(20);
    
    printk("[tc_3212] RGS_OTG_MODE         need 1 => %d\n", mt6333_get_rgs_otg_mode());
    printk("[tc_3212] RGS_CHR_MODE         need 0 => %d\n", mt6333_get_rgs_chr_mode());        

    printk("[tc_3212] set RG_OTG_EN = 0\n");    
    ret=mt6333_config_interface(0x30,0x94,0xFF,0);    
    mdelay(20);
    
    //--------------------------------------------------------
    printk("\n[tc_3212] b -------------------------\n");
    while(is_charger_exist()==0)
    {
        printk("b please plug-in CHARGER!\n");
        mdelay(500);
    }
    printk("[tc_3212] RGS_OTG_MODE         need 0 => %d\n", mt6333_get_rgs_otg_mode());
    printk("[tc_3212] RGS_CHR_MODE         need 1 => %d\n", mt6333_get_rgs_chr_mode());        
    while(is_charger_exist()==1)
    {
        printk("b please plug-out CHARGER!\n");
        mdelay(500);
    }
    //--------------------------------------------------------
    printk("\n[tc_3212] c -------------------------\n");

    printk("[tc_3212] set RG_OTG_EN = 1\n");    
    ret=mt6333_config_interface(0x30,0x95,0xFF,0);    
    mdelay(20);
    
    mdelay(1000);
    
    for(i=0;i<11;i++)
    //while(is_charger_exist()==0)
    {
        printk("c please plug-in CHARGER!\n");
        mdelay(500);
    }
    printk("[tc_3212] RGS_OTG_MODE         need 1 => %d\n", mt6333_get_rgs_otg_mode());
    printk("[tc_3212] RGS_CHR_MODE         need 0 => %d\n", mt6333_get_rgs_chr_mode());        
    
    printk("[tc_3212] set RG_OTG_EN = 0\n");    
    ret=mt6333_config_interface(0x30,0x94,0xFF,0);    
    mdelay(20);

    for(i=0;i<11;i++)
    //while(is_charger_exist()==1)
    {
        printk("c please plug-out CHARGER!\n");
        mdelay(500);
    }
#endif    
    //--------------------------------------------------------
    mdelay(5000);

    printk("\n[tc_3212] d -------------------------\n");

    for(i=0;i<11;i++)
    //while(is_charger_exist()==0)
    {
        printk("d please plug-in CHARGER!\n");
        mdelay(500);
    }
    mdelay(1000);

    printk("[tc_3212] set RG_OTG_EN = 1\n");    
    ret=mt6333_config_interface(0x30,0x95,0xFF,0);
    mdelay(20);
    
    printk("[tc_3212] RGS_OTG_MODE         need 0 => %d\n", mt6333_get_rgs_otg_mode());
    printk("[tc_3212] RGS_CHR_MODE         need 1 => %d\n", mt6333_get_rgs_chr_mode());

    for(i=0;i<11;i++)
    //while(is_charger_exist()==1)
    {
        printk("d please plug-out CHARGER!\n");
        mdelay(500);
    }
    
    printk("[tc_3212] set RG_OTG_EN = 0\n");    
    ret=mt6333_config_interface(0x30,0x94,0xFF,0);
    mdelay(20);
    
    //--------------------------------------------------------
    
    printk("[tc_3212] Done\n");
}


void tc_3221(void)
{
    kal_uint32 ret = 0;
    int i=0;

    printk("[tc_3221] \n");
    
    printk("[tc_3221] // -- 2.2.1 watdog timeout at OTG mode, check OTG d/a flags\n");
    printk("[tc_3221] NO CHARGER!\n");    

    printk("[tc_3221] Disable HW CHRIN short auto protection\n");
    ret=mt6333_config_interface(0xF3,0x1,0x1,5);
    ret=mt6333_config_interface(0xF7,0x1,0x1,5);
    
    printk("[tc_3221] set RG_OTG_EN = 1\n");
    ret=mt6333_config_interface(0x30,0x95,0xFF,0); 
    mdelay(20);

    for(i=0;i<11;i++)
    //while(is_charger_exist()==0)
    {
        printk("please plug-in CHARGER!\n");
        mdelay(500);
    }

    printk("[tc_3221] RGS_OTG_MODE         need 1 => %d\n", mt6333_get_rgs_otg_mode());
    printk("[tc_3221] RGS_CHR_MODE         need 0 => %d\n", mt6333_get_rgs_chr_mode());
    printk("[tc_3221] RGS_OTG_DRV_EN       need 1 => %d\n", mt6333_get_rgs_otg_drv_en());
    printk("[tc_3221] RSG_OTG_PRECC        need 0 => %d\n", mt6333_get_rgs_otg_precc());
    printk("[tc_3221] RGS_OTG_EN           need 1 => %d\n", mt6333_get_rgs_otg_en());
    printk("[tc_3221] RGS_CPCSTSYS_EN      need 1 => %d\n", mt6333_get_rgs_cpcstsys_en());
    printk("[tc_3221] RGS_M3_EN            need 1 => %d\n", mt6333_get_rgs_m3_en());
    printk("[tc_3221] RGS_M3_S_EN          need 1 => %d\n", mt6333_get_rgs_m3_s_en());
    printk("[tc_3221] RGS_M3_R_EN          need 0 => %d\n", mt6333_get_rgs_m3_r_en());
    printk("[tc_3221] RGS_M3_BOOST_EN      need 1 => %d\n", mt6333_get_rgs_m3_boost_en());
    printk("[tc_3221] RGS_OTG_EN_STB       need 1 => %d\n", mt6333_get_rgs_otg_en_stb());
    printk("[tc_3221] \n");

    mt6333_set_rg_chrwdt_en(0);
    mt6333_set_rg_chrwdt_wr(1); // write 1 to kick chr wdt
    mt6333_set_rg_chrwdt_td(0); //4 sec
    mt6333_set_rg_chrwdt_en(1);

    for(i=0;i<11;i++)
    //while(is_swchr_wdt_timeout()==0)
    {
        printk("swchr wdt is not timeout!\n");
        mdelay(500);
    }
    printk("Reg[0x%x]=0x%x\n", MT6333_CHRWDT_STATUS0, mt6333_get_reg_value(MT6333_CHRWDT_STATUS0));

    printk("[tc_3221] RGS_OTG_MODE         need 0 => %d\n", mt6333_get_rgs_otg_mode());
    printk("[tc_3221] RGS_CHR_MODE         need 0 => %d\n", mt6333_get_rgs_chr_mode());
    printk("[tc_3221] RGS_OTG_DRV_EN       need 0 => %d\n", mt6333_get_rgs_otg_drv_en());
    printk("[tc_3221] RSG_OTG_PRECC        need 0 => %d\n", mt6333_get_rgs_otg_precc());
    printk("[tc_3221] RGS_OTG_EN           need 0 => %d\n", mt6333_get_rgs_otg_en());
    printk("[tc_3221] RGS_CPCSTSYS_EN      need 0 => %d\n", mt6333_get_rgs_cpcstsys_en());
    printk("[tc_3221] RGS_M3_EN            need 0 => %d\n", mt6333_get_rgs_m3_en());
    printk("[tc_3221] RGS_M3_S_EN          need 0 => %d\n", mt6333_get_rgs_m3_s_en());
    printk("[tc_3221] RGS_M3_R_EN          need 0 => %d\n", mt6333_get_rgs_m3_r_en());
    printk("[tc_3221] RGS_M3_BOOST_EN      need 0 => %d\n", mt6333_get_rgs_m3_boost_en());
    printk("[tc_3221] RGS_OTG_EN_STB       need 0 => %d\n", mt6333_get_rgs_otg_en_stb());
    printk("[tc_3221] \n");
    
    printk("[tc_3221] Done\n");
}

void tc_3311(void)
{
    kal_uint32 ret=0;

    printk("[tc_3311] \n");

    printk("[tc_3311] // -- 3.1.1 Flash light mode on/off by setting RG_FLASH_DRV_EN, check the power-on sequence d/a flags\n");
    printk("[tc_3311] NO CHARGER!\n");

    printk("[tc_3311] Disable HW CHRIN short auto protection\n");
    ret=mt6333_config_interface(0xF3,0x1,0x1,5);
    ret=mt6333_config_interface(0xF7,0x1,0x1,5);

    printk("[tc_3311] set RG_FLASH_DRV_EN = 1\n");
    mt6333_set_rg_flash_drv_en(1);
    mdelay(20);
    
    printk("[tc_3311] RGS_OTG_MODE         need 1 => %d\n", mt6333_get_rgs_otg_mode());
    printk("[tc_3311] RGS_CHR_MODE         need 0 => %d\n", mt6333_get_rgs_chr_mode());
    printk("[tc_3311] RGS_OTG_DRV_EN       need 1 => %d\n", mt6333_get_rgs_otg_drv_en());
    printk("[tc_3311] RSG_OTG_PRECC        need 0 => %d\n", mt6333_get_rgs_otg_precc());
    printk("[tc_3311] RGS_OTG_EN           need 1 => %d\n", mt6333_get_rgs_otg_en());
    printk("[tc_3311] RGS_CPCSTSYS_EN      need 1 => %d\n", mt6333_get_rgs_cpcstsys_en());
    printk("[tc_3311] RGS_M3_EN            need 1 => %d\n", mt6333_get_rgs_m3_en());
    printk("[tc_3311] RGS_M3_S_EN          need 1 => %d\n", mt6333_get_rgs_m3_s_en());
    printk("[tc_3311] RGS_M3_R_EN          need 0 => %d\n", mt6333_get_rgs_m3_r_en());
    printk("[tc_3311] RGS_M3_BOOST_EN      need 1 => %d\n", mt6333_get_rgs_m3_boost_en());
    printk("[tc_3311] RGS_OTG_EN_STB       need 1 => %d\n", mt6333_get_rgs_otg_en_stb());
    printk("[tc_3311] \n");

    printk("[tc_3311] set RG_FLASH_DRV_EN = 0\n");
    mt6333_set_rg_flash_drv_en(0);
    mdelay(20);
    
    printk("[tc_3311] RGS_OTG_MODE         need 0 => %d\n", mt6333_get_rgs_otg_mode());
    printk("[tc_3311] RGS_CHR_MODE         need 0 => %d\n", mt6333_get_rgs_chr_mode());
    printk("[tc_3311] RGS_OTG_DRV_EN       need 0 => %d\n", mt6333_get_rgs_otg_drv_en());
    printk("[tc_3311] RSG_OTG_PRECC        need 0 => %d\n", mt6333_get_rgs_otg_precc());
    printk("[tc_3311] RGS_OTG_EN           need 0 => %d\n", mt6333_get_rgs_otg_en());
    printk("[tc_3311] RGS_CPCSTSYS_EN      need 0 => %d\n", mt6333_get_rgs_cpcstsys_en());
    printk("[tc_3311] RGS_M3_EN            need 0 => %d\n", mt6333_get_rgs_m3_en());
    printk("[tc_3311] RGS_M3_S_EN          need 0 => %d\n", mt6333_get_rgs_m3_s_en());
    printk("[tc_3311] RGS_M3_R_EN          need 0 => %d\n", mt6333_get_rgs_m3_r_en());
    printk("[tc_3311] RGS_M3_BOOST_EN      need 0 => %d\n", mt6333_get_rgs_m3_boost_en());
    printk("[tc_3311] RGS_OTG_EN_STB       need 0 => %d\n", mt6333_get_rgs_otg_en_stb());
    printk("[tc_3311] \n");

    printk("[tc_3311] Done\n");
}

void tc_3312(void)
{   
    kal_uint32 ret=0;
    int i=0;
    
    printk("[tc_3312] // -- 3.1.2 Switching between charger mode and flash light mode \n");

    printk("[tc_3312] Disable HW CHRIN short auto protection\n");
    ret=mt6333_config_interface(0xF3,0x1,0x1,5);
    ret=mt6333_config_interface(0xF7,0x1,0x1,5);   

#if 1
    //--------------------------------------------------------    
    printk("\n[tc_3312] a -------------------------\n");

    printk("[tc_3312] set mt6333_set_rg_flash_drv_en = 1\n");    
    mt6333_set_rg_flash_drv_en(1);    
    mdelay(20);
    
    printk("[tc_3312] RGS_OTG_MODE         need 1 => %d\n", mt6333_get_rgs_otg_mode());
    printk("[tc_3312] RGS_CHR_MODE         need 0 => %d\n", mt6333_get_rgs_chr_mode());        

    printk("[tc_3312] set mt6333_set_rg_flash_drv_en = 0\n");    
    mt6333_set_rg_flash_drv_en(0);    
    mdelay(20);
    
    //--------------------------------------------------------
    printk("\n[tc_3312] b -------------------------\n");
    while(is_charger_exist()==0)
    {
        printk("please plug-in CHARGER!\n");
        mdelay(500);
    }
    printk("[tc_3312] RGS_OTG_MODE         need 0 => %d\n", mt6333_get_rgs_otg_mode());
    printk("[tc_3312] RGS_CHR_MODE         need 1 => %d\n", mt6333_get_rgs_chr_mode());        
    while(is_charger_exist()==1)
    {
        printk("please plug-out CHARGER!\n");
        mdelay(500);
    }
    //--------------------------------------------------------
    printk("\n[tc_3312] c -------------------------\n");
    
    printk("[tc_3312] set mt6333_set_rg_flash_drv_en = 1\n");    
    mt6333_set_rg_flash_drv_en(1);    
    mdelay(20);
    
    mdelay(1000);
    for(i=0;i<11;i++)
    //while(is_charger_exist()==0)
    {
        printk("please plug-in CHARGER!\n");
        mdelay(500);
    }
    printk("[tc_3312] RGS_OTG_MODE         need 1 => %d\n", mt6333_get_rgs_otg_mode());
    printk("[tc_3312] RGS_CHR_MODE         need 0 => %d\n", mt6333_get_rgs_chr_mode());        
    
    printk("[tc_3312] set mt6333_set_rg_flash_drv_en = 0\n");    
    mt6333_set_rg_flash_drv_en(0);    
    mdelay(20);
    
    for(i=0;i<11;i++)
    //while(is_charger_exist()==1)
    {
        printk("please plug-out CHARGER!\n");
        mdelay(500);
    }
#endif    
    //--------------------------------------------------------
    mdelay(5000);

    printk("\n[tc_3312] d -------------------------\n");

    for(i=0;i<11;i++)
    //while(is_charger_exist()==0)
    {
        printk("please plug-in CHARGER!\n");
        mdelay(500);
    }
    mdelay(1000);

    printk("[tc_3312] set mt6333_set_rg_flash_drv_en = 1\n");    
    mt6333_set_rg_flash_drv_en(1); 
    mdelay(20);
    
    printk("[tc_3312] RGS_OTG_MODE         need 0 => %d\n", mt6333_get_rgs_otg_mode());
    printk("[tc_3312] RGS_CHR_MODE         need 1 => %d\n", mt6333_get_rgs_chr_mode());

    for(i=0;i<11;i++)
    //while(is_charger_exist()==1)
    {
        printk("please plug-out CHARGER!\n");
        mdelay(500);
    }
    
    printk("[tc_3312] set mt6333_set_rg_flash_drv_en = 0\n");    
    mt6333_set_rg_flash_drv_en(0); 
    mdelay(20);
    
    //--------------------------------------------------------
    
    printk("[tc_3312] Done\n");
}


//----------------------------------------------------------------

static ssize_t show_mt6333_dvt(struct device *dev,struct device_attribute *attr, char *buf)
{
    printk("[show_mt6333_dvt] 0x%x\n", g_reg_value_mt6333);
    return sprintf(buf, "%u\n", g_reg_value_mt6333);
}
static ssize_t store_mt6333_dvt(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
    char *pvalue = NULL;
    unsigned int test_item = 0;
    
    printk("[store_mt6333_dvt] \n");
    
    if(buf != NULL && size != 0)
    {
        printk("[store_mt6333_dvt] buf is %s and size is %d \n",buf,size);
        test_item = simple_strtoul(buf,&pvalue,10);
        printk("[store_mt6333_dvt] test_item=%d\n", test_item);

        switch (test_item)
        {
            // I2C
            case 1101: tc_1101(); break;
            case 1102: tc_1102(); break;
            case 1103: tc_1103(); break;
            case 1201: tc_1201(); break;

            //BUCK
            case 2101: tc_2101(); break;
            case 2102: tc_2102(); break;
            case 2103: tc_2103(); break;
            case 2202: tc_2202(); break;
            case 2203: tc_2203(); break;
            case 2207: tc_2207(); break;
            case 2208: tc_2208(); break;
            case 2209: tc_2209(); break;
            case 2302: tc_2302(); break;
            case 2303: tc_2303(); break;
            case 2306: tc_2306(); break;
            case 2307: tc_2307(); break;

            //SWCHR
            case 3111: tc_3111(); break;
            case 3112: tc_3112(); break;
            case 3113: tc_3113(); break;
            case 3121: tc_3121(); break;
            case 3122: tc_3122(); break;
            case 3123: tc_3123(); break;
            case 3141: tc_3141(); break;
            case 3211: tc_3211(); break;
            case 3212: tc_3212(); break;
            case 3221: tc_3221(); break;                        
            case 3311: tc_3311(); break;
            case 3312: tc_3312(); break;

            default:
                break;
        }
    }    
    return size;
}
static DEVICE_ATTR(mt6333_dvt, 0664, show_mt6333_dvt, store_mt6333_dvt); //664

#endif 

/**********************************************************
  *
  *   [platform_driver API] 
  *
  *********************************************************/
static ssize_t show_mt6333_access(struct device *dev,struct device_attribute *attr, char *buf)
{
    printk("[show_mt6333_access] 0x%x\n", g_reg_value_mt6333);
    return sprintf(buf, "%u\n", g_reg_value_mt6333);
}
static ssize_t store_mt6333_access(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
    int ret=0;
    char *pvalue = NULL;
    unsigned int reg_value = 0;
    unsigned int reg_address = 0;
    
    printk("[store_mt6333_access] \n");
    
    if(buf != NULL && size != 0)
    {
        printk("[store_mt6333_access] buf is %s and size is %d \n",buf,size);
        reg_address = simple_strtoul(buf,&pvalue,16);
        
        if(size > 3)
        {        
            reg_value = simple_strtoul((pvalue+1),NULL,16);        
            printk("[store_mt6333_access] write mt6333 reg 0x%x with value 0x%x !\n",reg_address,reg_value);
            ret=mt6333_config_interface(reg_address, reg_value, 0xFF, 0x0);
        }
        else
        {    
            ret=mt6333_read_interface(reg_address, &g_reg_value_mt6333, 0xFF, 0x0);
            printk("[store_mt6333_access] read mt6333 reg 0x%x with value 0x%x !\n",reg_address,g_reg_value_mt6333);
            printk("[store_mt6333_access] Please use \"cat mt6333_access\" to get value\r\n");
        }        
    }    
    return size;
}
static DEVICE_ATTR(mt6333_access, 0664, show_mt6333_access, store_mt6333_access); //664

void MT6333_PMIC_INIT_SETTING_V1(void)
{
    int ret=0;
    
    if(g_mt6333_cid >= PMIC6333_E1_CID_CODE)
    {
        xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[MT6333_PMIC_INIT_SETTING_V1] PMIC Chip = %x\n",g_mt6333_cid);
        xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[MT6333_PMIC_INIT_SETTING_V1] 20130514\n");

        //put init setting from DE/SA
        
        // write 0x0 to Reg[0x9f]  bit 7
        ret = mt6333_config_interface(0x9f, 0x0,0x1,7);        
        // write 0x3 to Reg[0xA0]  bit 0~2
        ret = mt6333_config_interface(0xA0,0x3,0x7,0);        
        // write 0x2 to Reg[0x69]  bit 1~7
        ret = mt6333_config_interface(0x69,0x2,0x7F,1);
        // write 0x18 to Reg[0x6D]  bit 0~6
        ret = mt6333_config_interface(0x6D,0x18,0x7F,0);        
        // write 0x2 to Reg[0x67]  bit 6~7
        ret = mt6333_config_interface(0x67,0x2,0x3,6);
    }
    else
    {
        xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[MT6333_PMIC_INIT_SETTING_V1] Unknown PMIC Chip (%x)\n",g_mt6333_cid);
    }

    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "after MT6333 init : Reg[%x]=0x%x,Reg[%x]=0x%x,Reg[%x]=0x%x,Reg[%x]=0x%x,Reg[%x]=0x%x\n", 
        0x9f, mt6333_get_reg_value(0x9f),
        0xA0, mt6333_get_reg_value(0xA0),
        0x69, mt6333_get_reg_value(0x69),
        0x6D, mt6333_get_reg_value(0x6D),
        0x67, mt6333_get_reg_value(0x67)
        );

    #ifdef IS_VRF18_USE_6333VRF18
    upmu_set_vrf18_on_ctrl(0); // set VRF18 as SW control
    #endif

    // DCT
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[MT6333_PMIC_INIT_SETTING_V1] pmu_drv_tool_customization_init\n");
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "MT6333 before: Reg[%x]=0x%x,Reg[%x]=0x%x,Reg[%x]=0x%x\n", 
        0x68, mt6333_get_reg_value(0x68),
        0x80, mt6333_get_reg_value(0x80),
        0x9F, mt6333_get_reg_value(0x9F)
        );
    pmu_drv_tool_customization_init(); // #ifdef MTK_MT6333_SUPPORT
    dump_ldo_status_read_debug();
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "MT6333 after: Reg[%x]=0x%x,Reg[%x]=0x%x,Reg[%x]=0x%x\n", 
        0x68, mt6333_get_reg_value(0x68),
        0x80, mt6333_get_reg_value(0x80),
        0x9F, mt6333_get_reg_value(0x9F)
        );
}


static int mt6333_driver_probe(struct i2c_client *client, const struct i2c_device_id *id) 
{             
    int err=0; 

    printk("[mt6333_driver_probe] \n");

    if (!(new_client = kmalloc(sizeof(struct i2c_client), GFP_KERNEL))) {
        err = -ENOMEM;
        goto exit;
    }    
    memset(new_client, 0, sizeof(struct i2c_client));

    new_client = client;    

    //---------------------
    g_mt6333_cid = mt6333_get_cid0();
    g_mt6333_cid |= ((mt6333_get_cid1()) << 8);
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[mt6333_driver_probe] g_mt6333_cid=0x%x\n", g_mt6333_cid);
    
    mt6333_hw_init();    
    //mt6333_dump_register();

    //pmic initial setting
    MT6333_PMIC_INIT_SETTING_V1();
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[MT6333_PMIC_INIT_SETTING_V1] Done\n");

    //MT6333 Interrupt Service
    //MT6333_EINT_SETTING();
    //xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[MT6333_EINT_SETTING] Done\n");
    
    kthread_run(mt6333_thread_kthread, NULL, "mt6333_thread_kthread");
    xlog_printk(ANDROID_LOG_INFO, "Power/PMIC", "[mt6333_thread_kthread] Done\n");

    mt6333_hw_init_done = KAL_TRUE;

    #ifdef MTK_SWCHR_SUPPORT
    chargin_hw_init_done = KAL_TRUE;
    #endif
    
    return 0;                                                                                       

exit:
    return err;

}

static int mt6333_user_space_probe(struct platform_device *dev)    
{    
    int ret_device_file = 0;

    printk("******** mt6333_user_space_probe!! ********\n" );
    
    ret_device_file = device_create_file(&(dev->dev), &dev_attr_mt6333_access);
    
    ret_device_file = device_create_file(&(dev->dev), &dev_attr_mt6333_dvt);
    
    return 0;
}

struct platform_device mt6333_user_space_device = {
    .name   = "mt6333-user",
    .id     = -1,
};

static struct platform_driver mt6333_user_space_driver = {
    .probe      = mt6333_user_space_probe,
    .driver     = {
        .name = "mt6333-user",
    },
};

static int __init mt6333_init(void)
{    
    int ret=0;
    
    printk("[mt6333_init] init start\n");
    
    i2c_register_board_info(mt6333_BUSNUM, &i2c_mt6333, 1);

    if(i2c_add_driver(&mt6333_driver)!=0)
    {
        printk("[mt6333_init] failed to register mt6333 i2c driver.\n");
    }
    else
    {
        printk("[mt6333_init] Success to register mt6333 i2c driver.\n");
    }

    // mt6333 user space access interface
    ret = platform_device_register(&mt6333_user_space_device);
    if (ret) {
        printk("****[mt6333_init] Unable to device register(%d)\n", ret);
        return ret;
    }    
    ret = platform_driver_register(&mt6333_user_space_driver);
    if (ret) {
        printk("****[mt6333_init] Unable to register driver (%d)\n", ret);
        return ret;
    }
    
    return 0;        
}

static void __exit mt6333_exit(void)
{
    i2c_del_driver(&mt6333_driver);
}

module_init(mt6333_init);
module_exit(mt6333_exit);
   
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("I2C mt6333 Driver");
MODULE_AUTHOR("James Lo<james.lo@mediatek.com>");
