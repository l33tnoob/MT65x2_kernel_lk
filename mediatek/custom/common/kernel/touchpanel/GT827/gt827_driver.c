#include <linux/init.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/bitops.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/byteorder/generic.h>
#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif 
#include <linux/interrupt.h>
#include <linux/time.h>
#include <linux/rtpm_prio.h>
#include <linux/dma-mapping.h>

#include <linux/syscalls.h>
#include <linux/reboot.h>
#include <linux/vmalloc.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/completion.h>
#include <asm/uaccess.h>
#include <linux/miscdevice.h>


#include <mach/mt_pm_ldo.h>
#include <mach/mt_typedefs.h>
#include <mach/mt_boot.h>

#include "tpd_custom_gt827.h"
#include "tpd.h"
#include <cust_eint.h>
//#include <linux/jiffies.h>

#ifndef TPD_NO_GPIO 
#include "cust_gpio_usage.h"
#endif

#define ABS(x)                  ((x<0)?-x:x)


extern struct tpd_device *tpd;

static int tpd_flag = 0;
static int tpd_halt=0;
static struct task_struct *thread = NULL;
static DECLARE_WAIT_QUEUE_HEAD(waiter);

#ifdef TPD_HAVE_BUTTON 
static int tpd_keys_local[TPD_KEY_COUNT] = TPD_KEYS;
static int tpd_keys_dim_local[TPD_KEY_COUNT][4] = TPD_KEYS_DIM;
#endif
#if (defined(TPD_WARP_START) && defined(TPD_WARP_END))
static int tpd_wb_start_local[TPD_WARP_CNT] = TPD_WARP_START;
static int tpd_wb_end_local[TPD_WARP_CNT]   = TPD_WARP_END;
#endif
#if (defined(TPD_HAVE_CALIBRATION) && !defined(TPD_CUSTOM_CALIBRATION))
//static int tpd_calmat_local[8]     				= TPD_CALIBRATION_MATRIX_ROTATION_NORMAL;
static int tpd_calmat_driver[8]                 = {0};
static int tpd_def_calmat_local[8] 				= TPD_CALIBRATION_MATRIX_ROTATION_NORMAL;
static int tpd_def_calmat_local_rotation_0[8]   = TPD_CALIBRATION_MATRIX_ROTATION_FACTORY;
#endif

#define TPD_ESD_PROTECT

#ifdef TPD_ESD_PROTECT
static struct delayed_work tpd_esd_check_work;
static struct workqueue_struct * tpd_esd_check_workqueue = NULL;
static void tpd_esd_check_func(struct work_struct *);
#define ESD_CHECK_CIRCLE 500
//#define ESD_CHECK_DATA_LEN  6
//#define ESD_CHECK_TIME      3
//unsigned char esd_check_data[ESD_CHECK_TIME*ESD_CHECK_DATA_LEN];
//int esd_checked_time = 0;
#endif




static void tpd_eint_interrupt_handler(void);
static int touch_event_handler(void *unused);
static int tpd_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int tpd_i2c_detect(struct i2c_client *client, struct i2c_board_info *info);
static int tpd_i2c_remove(struct i2c_client *client);
static int tpd_i2c_suspend(struct i2c_client *client,  pm_message_t msg);
static int tpd_i2c_resume(struct i2c_client *client);
#if 0
extern void mt65xx_eint_unmask(unsigned int line);
extern void mt65xx_eint_mask(unsigned int line);
extern void mt65xx_eint_set_hw_debounce(kal_uint8 eintno, kal_uint32 ms);
extern kal_uint32 mt65xx_eint_set_sens(kal_uint8 eintno, kal_bool sens);
extern void mt65xx_eint_registration(kal_uint8 eintno, kal_bool Dbounce_En,
                                     kal_bool ACT_Polarity, void (EINT_FUNC_PTR)(void),
                                     kal_bool auto_umask);
#endif
extern void tpd_close_gpio(void);                                                                    
extern void tpd_open_gpio(void);

static struct i2c_client *i2c_client = NULL;
static const struct i2c_device_id tpd_i2c_id[] = {{"mtk-tpd",0},{}};
static unsigned short force[] = {0, 0xBA, I2C_CLIENT_END,I2C_CLIENT_END};
static const unsigned short * const forces[] = { force, NULL };
//static struct i2c_client_address_data addr_data = { .forces = forces,};
static struct i2c_board_info __initdata i2c_tpd = { I2C_BOARD_INFO("mtk-tpd", (0xBA>>1))};


struct i2c_driver tpd_i2c_driver = {                       
    .probe = tpd_i2c_probe,                                   
    .remove = tpd_i2c_remove,                           
    .detect = tpd_i2c_detect,                           
    .driver.name = "mtk-tpd", 
    .id_table = tpd_i2c_id,                             
    .address_list = (const unsigned short*) forces,
    .suspend = tpd_i2c_suspend,
    .resume = tpd_i2c_resume,
};

static int i2c_enable_commands( struct i2c_client *client, u16 addr );
static int i2c_read_bytes( struct i2c_client *client, u16 addr, u8 *rxbuf, int len );
static int i2c_write_bytes( struct i2c_client *client, u16 addr, u8 *txbuf, int len );
static int tpd_init_panel(void);
static void tpd_reset(void);


/*
static u8 cfg_data_dvt[] =
{	
	0x00,0x0F,0x01,0x10,0x02,0x11,0x03,0x12,
	0x04,0x13,0x05,0x14,0x06,0x15,0x1C,0x0D,
	0x1B,0x0C,0x1A,0x0B,0x19,0x0A,0x18,0x09,
	0x17,0x08,0x16,0x07,0xFF,0xFF,0x02,0x0C,
	0x03,0x0D,0x04,0x0E,0x05,0x0F,0x06,0x10,
	0x07,0x11,0x08,0x12,0x09,0x13,0xFF,0x01,
	0x0B,0x00,0x0F,0x03,0x88,0x88,0x88,0x19,
	0x00,0x00,0x05,0x00,0x00,0x0E,0x50,0x3C,
	0x01,0x03,0x00,0x05,0x00,0x02,0x58,0x04,
	0x00,0x5A,0x5A,0x46,0x46,0x08,0x00,0x03,
	0x19,0x05,0x14,0x10,0x00,0x04,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x01,0x00,0x00,0x00,0x00,0x68,0x01			
};

static u8 cfg_data_evt[] =
{
	0x15,0x06,0x14,0x05,0x13,0x04,0x12,0x03,
	0x11,0x02,0x10,0x01,0x0F,0x00,0x07,0x16,
	0x08,0x17,0x09,0x18,0x0A,0x19,0x0B,0x1A,
	0x0C,0x1B,0x0D,0x1C,0x0E,0xFF,0x02,0x0C,
	0x03,0x0D,0x04,0x0E,0x05,0x0F,0x06,0x10,
	0x07,0x11,0x08,0x12,0x09,0x13,0xFF,0x01,
	0x0B,0x00,0x0F,0x03,0x88,0x88,0x88,0x1B,
	0x00,0x00,0x08,0x00,0x00,0x0E,0x50,0x3C,
	0x01,0x03,0x00,0x05,0x00,0x02,0x58,0x04,
	0x00,0x5A,0x5A,0x46,0x46,0x08,0x00,0x03,
	0x19,0x05,0x14,0x10,0x00,0x04,0x00,0x07,
	0x18,0x48,0x70,0x00,0x50,0x3C,0x60,0x20,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x01,0x00,0x00,0x00,0x00,0x68,0x01   
};
*/

static int tpd_i2c_detect(struct i2c_client *client, struct i2c_board_info *info)
{
    strcpy(info->type, "mtk-tpd");
    return 0;
}


static int tpd_i2c_suspend(struct i2c_client *client,  pm_message_t msg)
{
	TPD_DEBUG("real suspend\n");

#if 0	
    //mt_set_gpio_mode(GPIO67, GPIO_MODE_01);
    //mt_set_gpio_dir(GPIO67, GPIO_DIR_OUT);
    //mt_set_gpio_out(GPIO67, GPIO_OUT_ONE);

    mt_set_gpio_mode(GPIO70, GPIO_MODE_00);
    mt_set_gpio_dir(GPIO70, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO70, GPIO_OUT_ZERO);
#endif	

	return 0;
}

static int tpd_i2c_resume(struct i2c_client *client)
{
	TPD_DEBUG("real resume\n");

#if 0	
    tpd_reset();
#endif
	
	return 0;
}

static int i2c_enable_commands( struct i2c_client *client, u16 addr)
{
	u8 retry;
	u8 txbuf[2] = {0};

	if ( txbuf == NULL )
        return -1;

	txbuf[0] = ( addr >> 8 ) & 0xFF;
	txbuf[1] = addr & 0xFF;

	i2c_client->addr = i2c_client->addr & I2C_MASK_FLAG;// | I2C_ENEXT_FLAG;

	retry = 0;
    while ( i2c_master_send(i2c_client, &txbuf[0], I2C_DEVICE_ADDRESS_LEN ) < 0 )
    {
        retry++;

        if ( retry == 5 )
        {
            i2c_client->addr = i2c_client->addr & I2C_MASK_FLAG;
            TPD_DEBUG("I2C read 0x%X length=%d failed\n", addr, I2C_DEVICE_ADDRESS_LEN);
            return -1;
        }
    }

    return 0;
}

static int i2c_read_bytes( struct i2c_client *client, u16 addr, u8 *rxbuf, int len )
{
    u8 retry;
	u8 addrBuffer[2] = {0};
    u16 left = len;
    u16 offset = 0;

    if ( rxbuf == NULL )
        return -1;

    TPD_DEBUG("i2c_read_bytes to device %02X address %04X len %d\n", client->addr, addr, len );

    while ( left > 0 )
    {
        if ( left > MAX_I2C_MAX_TRANSFER_SIZE )
        {
            //addr = addr + offset;

			addrBuffer[0] = ( ( addr + offset ) >> 8 ) & 0x0F;
			addrBuffer[1] = ( addr + offset ) & 0xFF;
			            
            i2c_client->addr = i2c_client->addr & I2C_MASK_FLAG;

            retry = 0;
            while ( i2c_master_send(i2c_client, &addrBuffer[0], I2C_DEVICE_ADDRESS_LEN ) < 0 )
            {
                retry++;

                if ( retry == 5 )
                {
                    i2c_client->addr = i2c_client->addr & I2C_MASK_FLAG;
                    TPD_DEBUG("I2C read 0x%X length=%d failed\n", addr + offset, I2C_DEVICE_ADDRESS_LEN);
                    return -1;
                }
            }

            retry = 0;
            while ( i2c_master_recv(i2c_client, &rxbuf[offset], MAX_I2C_MAX_TRANSFER_SIZE ) < 0 )
            {
                retry++;

                if ( retry == 5 )
                {
                    i2c_client->addr = i2c_client->addr & I2C_MASK_FLAG;
                    TPD_DEBUG("I2C read 0x%X length=%d failed\n", addr + offset, MAX_I2C_MAX_TRANSFER_SIZE);
                    return -1;
                }
            }
            
            left -= MAX_I2C_MAX_TRANSFER_SIZE;
            offset += MAX_I2C_MAX_TRANSFER_SIZE;
        }
        else
        {
            //addr = addr + offset;

			addrBuffer[0] = ( ( addr + offset ) >> 8 ) & 0x0F;
			addrBuffer[1] = ( addr + offset ) & 0xFF;

            i2c_client->addr = i2c_client->addr & I2C_MASK_FLAG;			
            
            retry = 0;
            while ( i2c_master_send(i2c_client, &addrBuffer[0], I2C_DEVICE_ADDRESS_LEN ) < 0 )
            {
                retry++;

                if ( retry == 5 )
                {
                    i2c_client->addr = i2c_client->addr & I2C_MASK_FLAG;
                    TPD_DEBUG("I2C write 0x%X length=%d failed\n", addr + offset, I2C_DEVICE_ADDRESS_LEN);
                    return -1;
                }
            }

            retry = 0;
            while ( i2c_master_recv(i2c_client, &rxbuf[offset], left ) < 0 )
            {
                retry++;

                if ( retry == 5 )
                {
                    i2c_client->addr = i2c_client->addr & I2C_MASK_FLAG;
                    TPD_DEBUG("I2C read 0x%X length=%d failed\n", addr + offset, left);
                    return -1;
                }
            }
            
            offset += left;
            left = 0;
        }
    }
    i2c_client->addr = i2c_client->addr & I2C_MASK_FLAG;
    
    return 0;
}

static int i2c_write_bytes( struct i2c_client *client, u16 addr, u8 *txbuf, int len )
{
    u8 buffer[MAX_TRANSACTION_LENGTH];
    u16 left = len;
    u16 offset = 0;
    u8 retry = 0;

    struct i2c_msg msg = 
    {
        .addr = client->addr & I2C_MASK_FLAG,
        .flags = 0,
        .buf = buffer
    };


    if ( txbuf == NULL )
        return -1;

    TPD_DEBUG("i2c_write_bytes to device %02X address %04X len %d\n", client->addr, addr, len );

    while ( left > 0 )
    {
        retry = 0;

        //addr = addr + offset;

        buffer[0] = ( ( addr + offset ) >> 8 ) & 0x0F;
        buffer[1] = ( addr + offset ) & 0xFF;		

        if ( left > MAX_I2C_TRANSFER_SIZE )
        {
            memcpy( &buffer[I2C_DEVICE_ADDRESS_LEN], &txbuf[offset], MAX_I2C_TRANSFER_SIZE );
            msg.len = MAX_TRANSACTION_LENGTH;
            left -= MAX_I2C_TRANSFER_SIZE;
            offset += MAX_I2C_TRANSFER_SIZE;
        }
        else
        {
            memcpy( &buffer[I2C_DEVICE_ADDRESS_LEN], &txbuf[offset], left );
            msg.len = left + I2C_DEVICE_ADDRESS_LEN;
            left = 0;
        }

        TPD_DEBUG("byte left %d offset %d\n", left, offset );

        while ( i2c_transfer( client->adapter, &msg, 1 ) != 1 )
        {
            retry++;

            if ( retry == 5 )
            {
                TPD_DEBUG("I2C write 0x%X%X length=%d failed\n", buffer[0], buffer[1], len);
                return -1;
            }
            else
                 TPD_DEBUG("I2C write retry %d addr 0x%X%X\n", retry, buffer[0], buffer[1]);
        }
    }
    return 0;
}

static void tpd_reset(void)
{
	//int temp = 0;

	//Power off
#ifdef TPD_POWER_SOURCE_CUSTOM
	hwPowerDown(TPD_POWER_SOURCE_CUSTOM, "TP");
	printk("[mtk-tpd] MediaTek touch panel sets 2.8v hwPowerDown!!!\n");
#else
	hwPowerDown(MT65XX_POWER_LDO_VGP2, "TP");
	printk("[mtk-tpd] MediaTek touch panel sets VGP2 hwPowerDown!!!\n");
#endif
#ifdef TPD_POWER_SOURCE_1800
	hwPowerDown(TPD_POWER_SOURCE_1800, "TP");
	printk("[mtk-tpd] MediaTek touch panel sets 1.8v hwPowerDown!!!\n");
#endif 

	msleep(20);

	//Power on
#ifdef TPD_POWER_SOURCE_CUSTOM
	hwPowerOn(TPD_POWER_SOURCE_CUSTOM, VOL_2800, "TP");
	printk("[mtk-tpd] MediaTek touch panel sets 2.8v hwPowerOn!!!\n");
#else
	hwPowerOn(MT65XX_POWER_LDO_VGP2, VOL_2800, "TP");
	printk("[mtk-tpd] MediaTek touch panel sets VGP2 hwPowerOn!!!\n");
#endif
#ifdef TPD_POWER_SOURCE_1800
	hwPowerOn(TPD_POWER_SOURCE_1800, VOL_1800, "TP");
	printk("[mtk-tpd] MediaTek touch panel sets 1.8v hwPowerOn!!!\n");
#endif

	
	mt_set_gpio_mode(GPIO_CTP_EINT_PIN, GPIO_CTP_EINT_PIN_M_EINT);
    mt_set_gpio_dir(GPIO_CTP_EINT_PIN, GPIO_DIR_IN);
    mt_set_gpio_pull_enable(GPIO_CTP_EINT_PIN, GPIO_PULL_DISABLE);

    mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
    mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);  
    msleep(10);  
    mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);
	msleep(50);

	//temp = i2c_enable_commands(i2c_client, TPD_I2C_ENABLE_REG);
	printk("[mtk-tpd] Start to write config data! \n");
	tpd_init_panel();
    //msleep(10);		
	//temp = i2c_enable_commands(i2c_client, TPD_I2C_DISABLE_REG);
}

static int tpd_init_panel(void)
{
	int err = 0;

	/*******Check DVT1 or EVT*******/
	/*
	u8 bufTest[2] = {0};
	err = i2c_read_bytes( i2c_client, 0xF7E, &bufTest[0], 2);
	TPD_DEBUG("tpd_init_panel VID is 0x%02x, 0x%02x \n", bufTest[0], bufTest[1]);

	if ( ( bufTest[0] >> 1 ) >= 0x02 )
	{
		TPD_DEBUG("DVT1 is detected! \n");
		err = i2c_write_bytes( i2c_client, TPD_CONFIG_REG_BASE, cfg_data_dvt, sizeof( cfg_data_dvt ) );
	}
	else
	{
		TPD_DEBUG("EVT is detected! \n");
		err = i2c_write_bytes( i2c_client, TPD_CONFIG_REG_BASE, cfg_data_evt, sizeof( cfg_data_evt ) );
	}
	*/
	TPD_DEBUG("Start to write config data ... \n");
	err = i2c_write_bytes( i2c_client, TPD_CONFIG_REG_BASE, cfg_data, sizeof( cfg_data ) );
	
	return err;
}

static int tpd_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{             
    int err = 0, ret = -1;
    //struct goodix_ts_data *ts;
    //u8 version[17];
    
    #ifdef TPD_NO_GPIO
    u16 temp;
    temp = *(volatile u16 *) TPD_RESET_PIN_ADDR;
    temp = temp | 0x40;
    *(volatile u16 *) TPD_RESET_PIN_ADDR = temp;
    #endif
    i2c_client = client;
    
    printk("MediaTek touch panel i2c probe - GT827 \n");
    
    //Power on
#ifdef TPD_POWER_SOURCE_CUSTOM
		hwPowerOn(TPD_POWER_SOURCE_CUSTOM, VOL_2800, "TP");
		printk("[mtk-tpd] MediaTek touch panel sets 2.8v hwPowerOn!!!\n");
#else
		hwPowerOn(MT65XX_POWER_LDO_VGP2, VOL_2800, "TP");
		printk("[mtk-tpd] MediaTek touch panel sets VGP2 hwPowerOn!!!\n");
#endif
#ifdef TPD_POWER_SOURCE_1800
		hwPowerOn(TPD_POWER_SOURCE_1800, VOL_1800, "TP");
		printk("[mtk-tpd] MediaTek touch panel sets 1.8v hwPowerOn!!!\n");
#endif

    
    #ifndef TPD_NO_GPIO 	

    mt_set_gpio_mode(GPIO_CTP_EINT_PIN, GPIO_CTP_EINT_PIN_M_EINT);
    mt_set_gpio_dir(GPIO_CTP_EINT_PIN, GPIO_DIR_IN);
    mt_set_gpio_pull_enable(GPIO_CTP_EINT_PIN, GPIO_PULL_DISABLE);

    mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
    mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);  
    msleep(10);  
    mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);
    
    #endif 

    msleep(50);
    
    //Get firmware version
#if 0
    memset( &version[0], 0, 17);
    version[16] = '\0';
    err = i2c_read_bytes( i2c_client, TPD_VERSION_INFO_REG, (u8 *)&version[0], 16);

    if ( err )
    {
        printk(TPD_DEVICE " fail to get tpd info %d\n", err );
        return err;
    }
    else
    {
        printk( "Goodix TouchScreen Version:%s\n", (char *)&version[0]);
    }
#endif    
    //Load the init table
    // setting resolution, RES_X, RES_Y
    //cfg_data[1] = (TPD_RES_X>>8)&0xff;
    //cfg_data[2] = TPD_RES_X&0xff;
    //cfg_data[3] = (TPD_RES_Y>>8)&0xff;
    //cfg_data[4] = TPD_RES_Y&0xff;


	/*******Enable I2c*******/		
	err = i2c_enable_commands(i2c_client, TPD_I2C_ENABLE_REG);
    if (err < 0)
    {
        TPD_DEBUG("[mtk-tpd] write i2c enable commands error\n");
        //continue;
    }		

	/*******Write config data*******/
	//err = i2c_write_bytes( i2c_client, TPD_CONFIG_REG_BASE, cfg_data, sizeof(cfg_data));
	err = tpd_init_panel();
    if (err)
    {
        printk(TPD_DEVICE " fail to write tpd cfg %d\n", err );
        //return err;
    }
    msleep(10);

	/********Test Write-Read match*******/
#if 0
	u8 i;
	u16 addr = TPD_CONFIG_REG_BASE;
	u8 buf2[1] = {0};
	for(i=0; i < 112; i++)
	{
		//err = i2c_write_bytes( i2c_client, addr + i, &cfg_data[i], 1);		
		err = i2c_read_bytes( i2c_client, addr + i, &buf2[0], 1);
		if( i % 8 == 0)
		{
			TPD_DEBUG("\n");
		}
		TPD_DEBUG("Write 0x%02x \n", cfg_data[i]);
		TPD_DEBUG("Read 0x%02x \n", buf2[0]);
		if( cfg_data[i] != buf2[0] )
		{
			TPD_DEBUG("!!!!!!!!!! Write-Read byte[%d] does not match !!!!!!!!!! \n", i);
		}
	}
#endif

	/*******Disable I2c*******/
	err = i2c_enable_commands(i2c_client, TPD_I2C_DISABLE_REG);
    if (err < 0)
    {
        TPD_DEBUG("[mtk-tpd] write i2c disable commands error\n");
        //continue;
    }

#if 0	
	u8 buf[112] = {0};
	err = i2c_read_bytes( i2c_client, 0xF80, &buf[0], 112);

	TPD_DEBUG("Read config: ");
	int i;
	for(i=0; i < 112; i++)
	{
		if( i % 8 == 0)
		{
			TPD_DEBUG("\n");
		}
		TPD_DEBUG("0x%x  ", buf[i]);		
	}
#endif

#if 0
	u8 buf2[1] = {0};
	TPD_DEBUG("Write config data [16]: 0x%x \n", cfg_data[16]);
	err = i2c_write_bytes( i2c_client, 0xF90, &cfg_data[16], 1);
	err = i2c_read_bytes( i2c_client, 0xF90, &buf2[0], 1);
	TPD_DEBUG("Read config data [16]: 0x%x \n", buf2[0]);
#endif

	//i2c_read_bytes(i2c_client, TPD_TOUCH_INFO_REG_BASE, &version[0], 2);
    //printk(TPD_DEVICE " tpd cfg 0x%x 0x%x 0x%x 0x%x\n", version[0], version[1], version[2], version[3]);
	
    thread = kthread_run(touch_event_handler, 0, TPD_DEVICE);
    if ( IS_ERR(thread) ) { 
        err = PTR_ERR(thread);
        printk(TPD_DEVICE " failed to create kernel thread: %d\n", err);
    }
    tpd_load_status = 1;    
   
    //mt65xx_eint_set_sens(CUST_EINT_TOUCH_PANEL_NUM, CUST_EINT_TOUCH_PANEL_SENSITIVE);
    //mt65xx_eint_set_hw_debounce(CUST_EINT_TOUCH_PANEL_NUM, CUST_EINT_TOUCH_PANEL_DEBOUNCE_CN);
	mt_eint_registration(CUST_EINT_TOUCH_PANEL_NUM, CUST_EINT_TOUCH_PANEL_TYPE, tpd_eint_interrupt_handler, 1);
    mt_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);


#ifdef TPD_ESD_PROTECT
	tpd_esd_check_workqueue = create_workqueue("tpd_esd_check");
	INIT_DELAYED_WORK(&tpd_esd_check_work, tpd_esd_check_func);
	ret = queue_delayed_work(tpd_esd_check_workqueue, &tpd_esd_check_work, ESD_CHECK_CIRCLE); //schedule a work for the first detection
	TPD_DEBUG("GT827 - [TSP] ret =%d\n", ret);
#endif	
    
    printk("MediaTek touch panel i2c probe success\n");
    
    return 0;
}

void tpd_eint_interrupt_handler(void) { 
    TPD_DEBUG_PRINT_INT; tpd_flag=1; wake_up_interruptible(&waiter);
} 

static int tpd_i2c_remove(struct i2c_client *client) 
{
	#ifdef TPD_ESD_PROTECT
	destroy_workqueue(tpd_esd_check_workqueue);
	#endif

	return 0;
}

#ifdef TPD_ESD_PROTECT
static void tpd_esd_check_func(struct work_struct *work)
{   
	//int ret = -1;
	//int i;
	int err = 0, temp;
	//u8 tpdInfo[1];
	u8 j;
	//u16 addr = TPD_CONFIG_REG_BASE;
	u8 isAttacked = 0;
	u8 buf112[112] = {0};
	
	TPD_DEBUG( "tpd_esd_check_func++\n");

	if (tpd_halt)
	{
	    TPD_DEBUG( "tpd_esd_check_func return ..\n");
		return;
	}
	
	/***********Check if the calibration matrix is attacked*********/
	TPD_DEBUG( "Cancel Check if the calibration matrix is attacked..\n");
	
	/********Test Write-Read match*******/	
    #if 1	
	
	temp = i2c_enable_commands(i2c_client, TPD_I2C_ENABLE_REG);
	err = i2c_read_bytes( i2c_client, TPD_CONFIG_REG_BASE, &buf112[0], 112);
	temp = i2c_enable_commands(i2c_client, TPD_I2C_DISABLE_REG);
	
	for(j=0; j < 112; j++)
	{
		//err = i2c_write_bytes( i2c_client, addr + i, &cfg_data[i], 1);		
		//err = i2c_read_bytes( i2c_client, addr + j, &buf2[0], 1);
		//if( j % 8 == 0)
		//{
			//TPD_DEBUG("\n");
		//}
		//TPD_DEBUG("Write 0x%02x \n", cfg_data[j]);
		//TPD_DEBUG("Read 0x%02x \n", buf2[0]);
		if ( ( cfg_data[j] != buf112[j] ) && ( j != 111 ) )
		{
			TPD_DEBUG("!!!!!!!!!! Write-Read byte[%d] does not match, so we need to reset the tp !!!!!!!!!! \n", j);
			isAttacked = 1;
			break;
		}
	}
	
	if ( isAttacked )
	{
	    TPD_DEBUG( "Touch panel is attacked!! Start to initialize panel ..\n");
		tpd_reset();
	}
	else
	{
	    TPD_DEBUG("Write-Read config data all match!! ....\n");
	}
    #endif

	if(tpd_halt)
	{
		mt_eint_mask(CUST_EINT_TOUCH_PANEL_NUM);
	}
	else	
	{
		queue_delayed_work(tpd_esd_check_workqueue, &tpd_esd_check_work, ESD_CHECK_CIRCLE);
	}
	
	TPD_DEBUG( "tpd_esd_check_func--\n");
	return;
}
#endif


/**********For communicating with user space in factory mode************/
int gt827_open ( struct inode *inode, struct file *filp )
{
	TPD_DEBUG("gt827 open!\n");
	return 0;
}

int gt827_close ( struct inode *inode, struct file *filp )
{
	TPD_DEBUG("gt827 close!\n");
	return 0;
}

static long gt827_unlocked_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	TPD_DEBUG("gt827 ioctl!\n");

	switch ( cmd )
	{
		case TPD_IOCTL_ENABLE_I2C:
			TPD_DEBUG("gt827 TPD_IOCTL_ENABLE_I2C!\n");
			mt_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);
			break;
			
		case TPD_IOCTL_DISABLE_I2C:
			TPD_DEBUG("gt827 TPD_IOCTL_DISABLE_I2C!\n");
			mt_eint_mask(CUST_EINT_TOUCH_PANEL_NUM);
			break;
			
		default:
			TPD_DEBUG("gt827 command does not exist!\n");
			break;
	}

	return 0;
}

struct file_operations gt827_fops = {
	.owner = THIS_MODULE,
	.open = gt827_open,
	.release = gt827_close,
	.unlocked_ioctl = gt827_unlocked_ioctl,
};

static struct miscdevice tpd_dev = {
	.minor	= MISC_DYNAMIC_MINOR,
	.name	= TPD_NAME,
	.fops	= &gt827_fops,
};


void tpd_down ( int raw_x, int raw_y, int x, int y, int p, int id )
{
    input_report_abs(tpd->dev, ABS_PRESSURE, 128);
    input_report_key(tpd->dev, BTN_TOUCH, 1);
    input_report_abs(tpd->dev, ABS_MT_TOUCH_MAJOR, 128);
    input_report_abs(tpd->dev, ABS_MT_WIDTH_MAJOR, 128);
    input_report_abs(tpd->dev, ABS_MT_POSITION_X, x);
    input_report_abs(tpd->dev, ABS_MT_POSITION_Y, y);
	/* track id Start 0 */
	input_report_abs(tpd->dev, ABS_MT_TRACKING_ID, id);
    input_mt_sync(tpd->dev);
    TPD_DEBUG("D[%4d %4d %4d]\n", x, y, id);
    TPD_EM_PRINT(raw_x, raw_y, x, y, p, 1);
}

void tpd_up ( int raw_x, int raw_y, int x, int y, int p, int id )
{
    //input_report_abs(tpd->dev, ABS_PRESSURE, 0);
    input_report_key(tpd->dev, BTN_TOUCH, 0);
    //input_report_abs(tpd->dev, ABS_MT_TOUCH_MAJOR, 0);
    //input_report_abs(tpd->dev, ABS_MT_WIDTH_MAJOR, 0);
    //input_report_abs(tpd->dev, ABS_MT_POSITION_X, x);
    //input_report_abs(tpd->dev, ABS_MT_POSITION_Y, y);
    /* track id Start 0 */
	//input_report_abs(tpd->dev, ABS_MT_TRACKING_ID, id);
    input_mt_sync(tpd->dev);
    TPD_DEBUG("U[%4d %4d %4d]\n", x, y, id);
    TPD_EM_PRINT(raw_x, raw_y, x, y, p, 0);
}

void tpd_key_down ( u8 keyStatus )
{
	if ( TPD_KEY_MENU == keyStatus )
    {
        input_report_key( tpd->dev, KEY_MENU, 1 );
		TPD_DEBUG("[mtk-tpd] Press key menu (%d)!!\n", KEY_MENU);
    }
	else if ( TPD_KEY_HOME == keyStatus )
    {
        input_report_key( tpd->dev, KEY_HOMEPAGE, 1 );
		TPD_DEBUG("[mtk-tpd] Press key homepage (%d)!!\n", KEY_HOMEPAGE);
    }
	else if ( TPD_KEY_BACK== keyStatus )
    {
        input_report_key( tpd->dev, KEY_BACK, 1 );
		TPD_DEBUG("[mtk-tpd] Press key keyback (%d)!!\n", KEY_BACK);
    }
	//TPD_DEBUG("[mtk-tpd] Call input_mt_sync");
    input_mt_sync( tpd->dev );
}

void tpd_key_up ( u8 keyStatus )
{
	if ( TPD_KEY_MENU == keyStatus )
    {
        input_report_key( tpd->dev, KEY_MENU, 0 );	
		TPD_DEBUG("[mtk-tpd] Release key menu (%d)!!\n", KEY_MENU);
    }
	else if ( TPD_KEY_HOME == keyStatus )
    {
        input_report_key( tpd->dev, KEY_HOMEPAGE, 0 );
		TPD_DEBUG("[mtk-tpd] Release key homepage (%d)!!\n", KEY_HOMEPAGE);
    }
	else if ( TPD_KEY_BACK == keyStatus )
    {
        input_report_key( tpd->dev, KEY_BACK, 0 );
		TPD_DEBUG("[mtk-tpd] Release key keyback (%d)!!\n", KEY_BACK);
    }
	//TPD_DEBUG("[mtk-tpd] Call input_mt_sync");
	input_mt_sync( tpd->dev );
}

/*Coordination mapping*/
void tpd_calibrate_driver(int *x, int *y)
{
    int tx;
    TPD_DEBUG("Call tpd_calibrate of this driver ..\n");
    //if(tpd_calmat[0]==0) for(i=0;i<6;i++) tpd_calmat[i]=tpd_def_calmat[i];    
    tx = ( (tpd_calmat_driver[0] * (*x)) + (tpd_calmat_driver[1] * (*y)) + (tpd_calmat_driver[2]) ) >> 12;
    *y = ( (tpd_calmat_driver[3] * (*x)) + (tpd_calmat_driver[4] * (*y)) + (tpd_calmat_driver[5]) ) >> 12;
    *x = tx;
}

static int touch_event_handler(void *unused) {
    struct sched_param param = { .sched_priority = RTPM_PRIO_TPD };
    static u8 buffer[ TPD_POINT_INFO_LEN*MAX_FINGER_NUM ];
    int x, y, id, size, max_finger_id = 0, finger_num = 0;//temp
    //static u8 id_mask = 0;	
	u8 touchBufferStatus;//keyStatus lastKey = 0x00;
    u16 cur_mask;
    int idx, valid_point, lastIdx = 0;
    int ret = 0;
    static int x_history[MAX_FINGER_NUM+1];
    static int y_history[MAX_FINGER_NUM+1];
    //static int modeSetting = 0;
    
    sched_setscheduler(current, SCHED_RR, &param); 
    do{
        set_current_state(TASK_INTERRUPTIBLE);
        while (tpd_halt) {tpd_flag = 0; msleep(20);}
        wait_event_interruptible(waiter, tpd_flag != 0);
        //msleep(20);
        tpd_flag = 0;
        TPD_DEBUG_SET_TIME;
        set_current_state(TASK_RUNNING); 

		/*Enable I2c prefix*/
		ret = i2c_enable_commands(i2c_client, TPD_I2C_ENABLE_REG);
        if (ret < 0)
        {
            TPD_DEBUG("[mtk-tpd] write i2c enable commands error\n");
            continue;
        }		
        
        ret = i2c_read_bytes( i2c_client, TPD_TOUCH_INFO_REG_BASE, &buffer[0], TPD_TOUCH_INFO_LENGTH );
        if (ret < 0)
        {
            TPD_DEBUG("[mtk-tpd] i2c write communicate error during read status\n");
            continue;
        }


		/*Check if buffer status is ok. (0x80) */
		touchBufferStatus = buffer[0] & 0xC0;
		if ( 0x80 != touchBufferStatus )
		{
			TPD_DEBUG("[mtk-tpd] Data not ready! Touch buffer status : %x \n", touchBufferStatus);
			ret = i2c_enable_commands( i2c_client, TPD_I2C_DISABLE_REG );
			continue;
		}

		cur_mask = buffer[0] & 0x1F;
		TPD_DEBUG("[mtk-tpd] STATUS : %x \n", cur_mask);		

#ifdef TPD_HAVE_TOUCH_KEY
		ret = i2c_read_bytes( i2c_client, TPD_KEY_INFO_REG_BASE, &buffer[0], 1 );
        if (ret < 0)
        {
            TPD_DEBUG("[mtk-tpd] i2c write communicate error during read key status\n");
            //continue;
        }

		keyStatus = buffer[0] & 0x0F;
		TPD_DEBUG("[mtk-tpd] KEY STATUS : 0x%x \n", keyStatus);

		if ( 0x0F == keyStatus )
		{
			ret = tpd_init_panel();
			ret = i2c_enable_commands( i2c_client, TPD_I2C_DISABLE_REG );
			continue;
		}

		if ( 0x00 == keyStatus && 0x00 != lastKey )
		{
			tpd_key_up( lastKey );
			lastKey = 0x00;
		}
		else if ( 0x00 != keyStatus )
		{
			lastKey = keyStatus;
			tpd_key_down( keyStatus );
		}
#endif

		if ( tpd == NULL || tpd->dev == NULL )
            continue;
		
        if ( cur_mask )
        {
            idx = 0;
            do
            {
                idx++;
            }while( ( cur_mask >> idx ) > 0);

			finger_num = idx;
			max_finger_id = idx - 1;
            ret = i2c_read_bytes( i2c_client, TPD_POINT_INFO_REG_BASE, buffer, ( max_finger_id + 1) * TPD_POINT_INFO_LEN );
            if ( ret < 0 )
            {
                TPD_DEBUG("[mtk-tpd] i2c write communcate error during read point\n");
                continue;
            }
        }
        else
        {
            max_finger_id = 0;
			finger_num = 0;
        }

		//Double check mode setting
		/*
		if ( FACTORY_BOOT != get_boot_mode() )
		{
			if ( modeSetting < 5 )
			{
				TPD_DEBUG("Modesetting is less than 5 ! \n");
				memcpy(tpd_calmat, tpd_def_calmat_local, sizeof(tpd_calmat));
		    	memcpy(tpd_def_calmat, tpd_def_calmat_local, sizeof(tpd_def_calmat));
				modeSetting++;

				#if 0
				TPD_DEBUG("tpd_calmat[0] %d    ", tpd_calmat[0]);
				TPD_DEBUG("tpd_calmat[1] %d    ", tpd_calmat[1]);
				TPD_DEBUG("tpd_calmat[2] %d  \n", tpd_calmat[2]);
				TPD_DEBUG("tpd_calmat[3] %d    ", tpd_calmat[3]);
				TPD_DEBUG("tpd_calmat[4] %d    ", tpd_calmat[4]);
				TPD_DEBUG("tpd_calmat[5] %d  \n", tpd_calmat[5]);
				#endif
			}
			else
			{
				TPD_DEBUG("Modesetting is greater or equal than 5 ! \n");			
				#if 0
				TPD_DEBUG("tpd_calmat[0] %d    ", tpd_calmat[0]);
				TPD_DEBUG("tpd_calmat[1] %d    ", tpd_calmat[1]);
				TPD_DEBUG("tpd_calmat[2] %d  \n", tpd_calmat[2]);
				TPD_DEBUG("tpd_calmat[3] %d    ", tpd_calmat[3]);
				TPD_DEBUG("tpd_calmat[4] %d    ", tpd_calmat[4]);
				TPD_DEBUG("tpd_calmat[5] %d  \n", tpd_calmat[5]);
				#endif
			}
		}
		*/

        for ( idx = 0, valid_point = 0 ; idx < ( max_finger_id + 1 ) ; idx++ )
        {
            u8 *ptr = &buffer[valid_point * TPD_POINT_INFO_LEN];
			id = idx;

            if ( cur_mask & ( 1 << idx ) )
            {
                x = ptr[1] + ( ( (int)ptr[0] ) << 8 );
                y = ptr[3] + ( ( (int)ptr[2] ) << 8 );
                size = ptr[4];
                
                TPD_DEBUG("[mtk-tpd] Original position/size: [%4d %4d %4d]\n", x, y, size);

				//temp = x;
				//x = y;
				//y = temp;
			
                //tpd_calibrate(&x, &y);
                tpd_calibrate_driver(&x, &y);
                tpd_down(x, y, x, y, size, id);

                x_history[idx] = x;
                y_history[idx] = y;
                valid_point++;
				lastIdx = idx;
            }
            else
            {
            	TPD_DEBUG("Invalid id %d\n", idx );
            }
        }         

		/****Linux kernel 2.6.35. For GB.****/
#if 0
        if ( cur_mask != id_mask )
        {
            u8 diff = cur_mask^id_mask;
            idx = 0;

            while ( diff )
            {
                if ( ( ( diff & 0x01 ) == 1 ) &&
                     ( ( cur_mask >> idx ) & 0x01 ) == 0 )
                {
                    // check if key release
                    tpd_up( x_history[idx], y_history[idx], x_history[idx], y_history[idx], 0);                    
                }

                diff = ( diff >> 1 );
                idx++;
            }
            id_mask = cur_mask;
        }
		if ( tpd != NULL && tpd->dev != NULL )
        {
        	input_sync(tpd->dev);
    	}
#endif
		/****Linux kernel from 2.6.35 to 3.0. For ICS.****/
		if ( finger_num )
        {
        	if ( tpd != NULL && tpd->dev != NULL )
            {
            	input_sync(tpd->dev);
        	}
		}
		else
		{
			if ( tpd != NULL && tpd->dev != NULL )
            {
            	//TPD_DEBUG("lastIdx = %d \n", lastIdx);
				//TPD_DEBUG("x_history[lastIdx] = %d, y_history[lastIdx] = %d \n", x_history[lastIdx], y_history[lastIdx]);
            	//TPD_DEBUG("Up[%4d %4d %4d]\n", x_history[lastIdx], y_history[lastIdx], 0);
				tpd_up(x_history[lastIdx], y_history[lastIdx], x_history[lastIdx], y_history[lastIdx], 0, lastIdx);
				input_sync(tpd->dev);
        	}
		}

		/*Disable I2c postfix*/
		ret = i2c_enable_commands(i2c_client, TPD_I2C_DISABLE_REG);
        if (ret < 0)
        {
            TPD_DEBUG("[mtk-tpd] write i2c disable commands error\n");
            //continue;
        }
    } while (!kthread_should_stop()); 
    return 0;
}

int tpd_local_init(void) 
{
	int retval;
	
    if(i2c_add_driver(&tpd_i2c_driver)!=0)
	{
      TPD_DMESG("unable to add i2c driver.\n");
      return -1;
    }

    if (0) //if(tpd_load_status == 0)
    {
    	TPD_DMESG("add error touch panel driver.\n");
    	i2c_del_driver(&tpd_i2c_driver);
    	return -1;
    }

	/* register device (/dev/GT827 CTP) */
	retval = misc_register( &tpd_dev );
	TPD_DEBUG("GT827 call misc_register\n");
	if ( 0 != retval )
	{
		TPD_DEBUG("Register gt827 device failed (%d)\n", retval);
		return -1;
	}
    
#ifdef TPD_HAVE_BUTTON     
    tpd_button_setting(TPD_KEY_COUNT, tpd_keys_local, tpd_keys_dim_local);// initialize tpd button data
#endif

#ifdef TPD_HAVE_TOUCH_KEY
	u8 i;
	for ( i = 0; i < TPD_TOUCH_KEY_NUM; i++ )
	{
		input_set_capability(tpd->dev, EV_KEY, touchKeyArray[i]);
	}
#endif

	//For setting track id
	input_set_abs_params(tpd->dev, ABS_MT_TRACKING_ID, 0, 5, 0, 0);
  
#if (defined(TPD_WARP_START) && defined(TPD_WARP_END))    
    TPD_DO_WARP = 1;
    memcpy(tpd_wb_start, tpd_wb_start_local, TPD_WARP_CNT*4);
    memcpy(tpd_wb_end, tpd_wb_start_local, TPD_WARP_CNT*4);
#endif 

#if (defined(TPD_HAVE_CALIBRATION) && !defined(TPD_CUSTOM_CALIBRATION))
	if ( FACTORY_BOOT == get_boot_mode() )
	{
		TPD_DEBUG("Factory mode is detected! \n");		
		memcpy(tpd_calmat_driver, tpd_def_calmat_local_rotation_0, sizeof(tpd_calmat_driver));
	    
		//memcpy(tpd_calmat, tpd_def_calmat_local_rotation_0, sizeof(tpd_calmat));
	    //memcpy(tpd_def_calmat, tpd_def_calmat_local_rotation_0, sizeof(tpd_def_calmat));		
	}
	else
	{
		TPD_DEBUG("Normal mode is detected! \n");
		memcpy(tpd_calmat_driver, tpd_def_calmat_local, sizeof(tpd_calmat_driver));
		
    	//memcpy(tpd_calmat, tpd_def_calmat_local, sizeof(tpd_calmat));
	    //memcpy(tpd_def_calmat, tpd_def_calmat_local, sizeof(tpd_def_calmat));
	}
#endif  
    TPD_DMESG("end %s, %d\n", __FUNCTION__, __LINE__);  
    tpd_type_cap = 1;
    return 0;
}

/* Function to manage low power suspend */
//void tpd_suspend(struct i2c_client *client, pm_message_t message)
static void tpd_suspend( struct early_suspend *h )
{
    int ret = 0;//, retry = 0;
    u8 Wrbuf[1];
    tpd_halt = 1;
    mt_eint_mask(CUST_EINT_TOUCH_PANEL_NUM);
    TPD_DEBUG("GT827 call early suspend\n");

    Wrbuf[0] = 0xC0;
	ret = i2c_write_bytes(i2c_client, TPD_POWER_MODE_REG, &Wrbuf[0], 1);
	
    if(ret < 0)
    {
        TPD_DEBUG("[mtk-tpd] i2c write communcate error during suspend: 0x%x\n", ret);
    }

	//Turn off LCM 3.3v
	//MTKFB takes this
	//tpd_close_gpio();
	/*
	mt_set_gpio_mode(GPIO70, GPIO_MODE_00);
    mt_set_gpio_dir(GPIO70, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO70, GPIO_OUT_ZERO);
    */
}

/* Function to manage power-on resume */
//void tpd_resume(struct i2c_client *client)
static void tpd_resume( struct early_suspend *h )
{
    #ifdef TPD_RESET_ISSUE_WORKAROUND	
    	u8 ret = 0;
    #endif

    TPD_DEBUG("GT827 call early resume\n");

	//Turn on LCM 3.3v
	//MTKFb takes this
	//tpd_open_gpio();
	/*
	mt_set_gpio_mode(GPIO70, GPIO_MODE_00);
    mt_set_gpio_dir(GPIO70, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO70, GPIO_OUT_ONE);
    */


    mt_set_gpio_mode(GPIO_CTP_EINT_PIN, GPIO_CTP_EINT_PIN_M_GPIO);
    mt_set_gpio_dir(GPIO_CTP_EINT_PIN, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_CTP_EINT_PIN, GPIO_OUT_ZERO);
    //msleep(20);
    msleep(1);
	mt_set_gpio_out(GPIO_CTP_EINT_PIN, GPIO_OUT_ONE);
	msleep(1);
	mt_set_gpio_out(GPIO_CTP_EINT_PIN, GPIO_OUT_ZERO);
	msleep(20);
    
    mt_set_gpio_mode(GPIO_CTP_EINT_PIN, GPIO_CTP_EINT_PIN_M_EINT);
    mt_set_gpio_dir(GPIO_CTP_EINT_PIN, GPIO_DIR_IN);
    mt_set_gpio_pull_enable(GPIO_CTP_EINT_PIN, GPIO_PULL_DISABLE);

    mt_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);

	#ifdef TPD_ESD_PROTECT
	queue_delayed_work(tpd_esd_check_workqueue, &tpd_esd_check_work, ESD_CHECK_CIRCLE);
	#endif

#ifdef TPD_RESET_ISSUE_WORKAROUND	
	ret = i2c_enable_commands(i2c_client, TPD_I2C_ENABLE_REG);
	ret = i2c_enable_commands(i2c_client, TPD_I2C_DISABLE_REG);
#endif	
    tpd_halt = 0;
}

static struct tpd_driver_t tpd_device_driver = 
{
	.tpd_device_name = "GT827",
	.tpd_local_init = tpd_local_init,
	.suspend = tpd_suspend,
	.resume = tpd_resume,
#ifdef TPD_HAVE_BUTTON
	.tpd_have_button = 1,
#else
	.tpd_have_button = 0,
#endif
};

/* called when loaded into kernel */
static int __init tpd_driver_init(void)
{
    printk("MediaTek gt827 touch panel driver init\n");
	i2c_register_board_info(0, &i2c_tpd, 1);
    if(tpd_driver_add(&tpd_device_driver) < 0)
		TPD_DMESG("add generic driver failed\n");

	return 0;
}

/* should never be called */
static void __exit tpd_driver_exit(void)
{
    TPD_DMESG("MediaTek gt827 touch panel driver exit\n");
    //input_unregister_device(tpd->dev);
    tpd_driver_remove(&tpd_device_driver);
	misc_deregister( &tpd_dev );
}

module_init(tpd_driver_init);
module_exit(tpd_driver_exit);

