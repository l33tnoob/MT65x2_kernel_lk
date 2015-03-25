#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/device.h>
#include <linux/miscdevice.h>
#include <linux/wait.h>
#include <linux/mm.h>
#include <linux/dma-mapping.h>
#include <asm/io.h>

#include <mach/mt_clkmgr.h>
#include <mach/mt_reg_base.h>

#include "i2c_ldvt.h"

//#define I2C_DEBUG

#ifdef I2C_DEBUG
#define I2C_BUG_ON(a) BUG_ON(a)
#else
#define I2C_BUG_ON(a)
#endif

#define I2C_LOG(fmt, args ...) printk(KERN_WARNING "[I2C] %5d: " fmt, __LINE__,##args)

#define I2C_T_DMA 1
#define I2C_T_TRANSFERFLOW 2
#define I2C_T_SPEED 3

#define I2C_LDVT_DEVICE "mt-i2c-ldvt"
#define I2C_DRV_NAME	"mt-i2c"

#define I2C_BUSNUM	0
#define PS_DEV_NAME	"i2c_ps" /* Press sensor */
#define GS_DEV_NAME	"i2c_gs" /* G-sensor */
#define PS_SLAVE_ADDR	0xEE
#define GS_SLAVE_ADDR	0x98
#define SLAVE_ADDR_LEN	1

#define HZ_100K		100 /* kHz */
#define HZ_200K		200 /* kHz */
#define HZ_400K		400 /* kHz */
#define HZ_1P5M		1500 /* kHz */
#define HZ_2M		2000 /* kHz */

#define MAX_RETRY_CNT	5

static struct i2c_board_info __initdata i2c_ps = {I2C_BOARD_INFO(PS_DEV_NAME, (PS_SLAVE_ADDR >> 1))};
static struct i2c_board_info __initdata i2c_gs = {I2C_BOARD_INFO(GS_DEV_NAME, (GS_SLAVE_ADDR >> 1))};
static const struct i2c_device_id i2c_ps_id[] = {{PS_DEV_NAME, 0},{}};
static const struct i2c_device_id i2c_gs_id[] = {{GS_DEV_NAME, 0},{}};
static int i2c_ps_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int i2c_gs_probe(struct i2c_client *client, const struct i2c_device_id *id);

static struct i2c_client *ps_client;
static struct i2c_client *gs_client;

static struct i2c_driver i2c_ps_driver = {
	.driver = {
		.name = PS_DEV_NAME,
		.owner = THIS_MODULE,
	},
	.probe = i2c_ps_probe,
	.id_table = i2c_ps_id,
};

static struct i2c_driver i2c_gs_driver = {
	.driver = {
		.name = GS_DEV_NAME,
		.owner = THIS_MODULE,
	},
	.probe = i2c_gs_probe,
	.id_table = i2c_gs_id,
};

static int i2c_ps_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	I2C_LOG("%s\n", __FUNCTION__);

	ps_client = client;
	if (!ps_client)
		return -EINVAL;

    return 0;
}

static int i2c_gs_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	I2C_LOG("%s\n", __FUNCTION__);

	gs_client = client;
	if(!gs_client)
		return -EINVAL;

	return 0;
}

int i2c_ps_write_bytes(u8 addr, unsigned int ext_flag,
			int speed, u8 *txbuf, int len)
{
	u8 buffer[I2C_FIFO_SIZE];
	u16 left = len;
	u16 offset = 0;
	u8 retry = 0;

	struct i2c_msg msg = {
		.addr = ps_client->addr,
		.ext_flag = ext_flag,
		.flags = 0,
		.buf = txbuf,
		.timing = speed,
	};


	if (txbuf == NULL)
		return -EINVAL;

	I2C_LOG("i2c_ps_write_bytes to device %02X address %04X len %d\n",
			ps_client->addr, addr, len);

	while (left > 0)
	{
		retry = 0;
	/* simple version */
 	       buffer[0] = (addr + offset) & 0xFF;

		if (left > I2C_FIFO_SIZE - SLAVE_ADDR_LEN) {
			memcpy(&buffer[SLAVE_ADDR_LEN], &txbuf[offset], 
				I2C_FIFO_SIZE - SLAVE_ADDR_LEN);
			msg.len = I2C_FIFO_SIZE;
			left -= (I2C_FIFO_SIZE - SLAVE_ADDR_LEN);
			offset += (I2C_FIFO_SIZE - SLAVE_ADDR_LEN);
		} else {
			memcpy(&buffer[SLAVE_ADDR_LEN], &txbuf[offset], left);
			msg.len = left + SLAVE_ADDR_LEN;
			left = 0;
		}

		while (i2c_transfer(ps_client->adapter, &msg, 1) != 1) {
			retry++;

			if (retry == MAX_RETRY_CNT) {
				I2C_LOG("I2C write 0x%X length=%d failed\n", 
					buffer[0], len);
				return -EINVAL;
			}
		}
	}

    return 0;
}

int i2c_ps_read_bytes(u8 addr, unsigned int ext_flag,
			int speed, u8 *rxbuf, int len)
{
	int ret = 0;
	u8 addr_buf = 0;
	u16 left = len;
	u16 offset = 0;
	int retry;

	if (ext_flag & I2C_DMA_FLAG) {
		struct i2c_msg msg_dma = {
			.addr = ps_client->addr,
			.ext_flag = ext_flag,
			.flags = 0,
			.buf = rxbuf,
			.len = (0x1 << 8) | 0x1,
			.timing = speed,
		};

		// rxbuf[0] = addr;
		I2C_LOG("ps DMA to device %02X, address %04X, len %d\n",
			ps_client->addr, addr, len);

		ret = i2c_transfer(ps_client->adapter, &msg_dma, 1);
		if(ret <= 0)
			return -EINVAL;

	} else {
		struct i2c_msg msg[2] = {
			{
				.addr = ps_client->addr,
				.ext_flag = ext_flag,
				.flags = 0,
				.buf = &addr_buf,
				.len = 1,
				.timing = speed,
			},
			{
				.addr = ps_client->addr,
				.ext_flag = ext_flag,
				.flags = I2C_M_RD,
				.timing = speed,
			}
		};

		if (rxbuf == NULL)
			return -EINVAL;

		I2C_LOG("ps to device %02X, address %04X, len %d\n",
					ps_client->addr, addr, len);

		while (left > 0) {
			addr_buf = (addr + offset) & 0xFF;

			msg[1].buf = &rxbuf[offset];

			if (left > I2C_FIFO_SIZE) {
				msg[1].len = I2C_FIFO_SIZE;
				left -= I2C_FIFO_SIZE;
				offset += I2C_FIFO_SIZE;
		        } else {
				msg[1].len = left;
				left = 0;
			}

			retry = 0;

			while (i2c_transfer(ps_client->adapter, &msg[0], 2) != 2) {
				retry++;

				if (retry == MAX_RETRY_CNT) {
					I2C_LOG("ps I2C read 0x%X len %d failed\n", 
						addr + offset, len);
					return -EINVAL;
				}
			}
		}
	}

	return 0;
}

int i2c_gs_read_bytes(u8 addr, unsigned int ext_flag, 
			int speed, u8 *rxbuf, int len)
{
	u8 addr_buf = 0;
	u16 left = len;
	u16 offset = 0;
	int retry;

	if (ext_flag & I2C_DMA_FLAG) {
		// TODO: add DMA function here

	} else {
		struct i2c_msg msg[2] =
		{
			{
				.addr = gs_client->addr,
				.ext_flag = ext_flag,
				.flags = 0,
				.buf = &addr_buf,
				.len = 1,
				.timing = speed,
			},
			{
				.addr = gs_client->addr,
				.ext_flag = ext_flag,
				.flags = I2C_M_RD,
				.timing = speed,
			}
		};

		if (rxbuf == NULL)
			return -EINVAL;

		I2C_LOG("gs to device %02X, address %04X, len %d\n",
			gs_client->addr, addr, len);

		while (left > 0) {
			addr_buf = (addr + offset) & 0xFF;

			msg[1].buf = &rxbuf[offset];

			if (left > I2C_FIFO_SIZE) {
				msg[1].len = I2C_FIFO_SIZE;
				left -= I2C_FIFO_SIZE;
				offset += I2C_FIFO_SIZE;
		        } else {
				msg[1].len = left;
				left = 0;
			}

			retry = 0;

			while (i2c_transfer(gs_client->adapter, &msg[0], 2) != 2) {
				retry++;

				if (retry == MAX_RETRY_CNT) {
					I2C_LOG("gs I2C read 0x%X len %d failed\n", 
						addr + offset, len);
					return -EINVAL;
				}
			}
		}
	}

	return 0;
}


/* I2C LDVT function */
static int mt_i2c_misc_open(struct inode *inode, struct file *file)
{
	return 0;
}

static long mt_i2c_misc_ioctl(struct file *file, unsigned int cmd,
			      unsigned long arg)
{
	void __user *uarg = (void __user *)arg;
	int ret = 0;
	unsigned int ext_flag = 0;
	dma_addr_t data_phy = 0;
	void *vir_addr = NULL;
	u8 *data;
	u8 write_byte = 0xB2;

	switch (arg) {
	case I2C_UVVF_HS_FIFO_1P5MHZ:
		ext_flag |= I2C_HS_FLAG;
		data = kzalloc(I2C_FIFO_SIZE, GFP_KERNEL);
		if (!data) {
			I2C_LOG("kzalloc failed\n");
			return -EBUSY;
		}

		ret = i2c_gs_read_bytes(0x07, ext_flag, HZ_1P5M, 
					data, I2C_FIFO_SIZE);
#if 1
		if (ret)
			I2C_LOG("case 1 failed\n");
		else
			I2C_LOG("case 1 successed\n");
#endif

		kfree(data);
		break;

	case I2C_UVVF_HS_FIFO_2MHZ:
		ext_flag |= I2C_HS_FLAG;
		data = kzalloc(I2C_FIFO_SIZE, GFP_KERNEL);
		if (!data) {
			I2C_LOG("kzalloc failed\n");
			return -EBUSY;
		}

		ret = i2c_gs_read_bytes(0x07, ext_flag, HZ_2M,
					data, I2C_FIFO_SIZE);
#if 1
		if (ret)
			I2C_LOG("case 2 failed\n");
		else
			I2C_LOG("case 2 successed\n");
#endif

		kfree(data);
		break;

	case I2C_UVVF_FS_DMA_RS_DIR_200KHZ:
		ext_flag |= (I2C_DMA_FLAG | I2C_RS_FLAG | I2C_WR_FLAG);
		vir_addr = dma_alloc_coherent(NULL, I2C_FIFO_SIZE,
						&data_phy, GFP_KERNEL);
		if (!vir_addr) {
			I2C_LOG("Alloc DMA buffer failed\n");
			return -EBUSY;
		}

		*(u8 *)vir_addr = 0xB2;
		ret = i2c_ps_read_bytes(0xB2, ext_flag, HZ_200K,
					(void *)data_phy, I2C_FIFO_SIZE);
#if 1
		if (ret)
			I2C_LOG("case 3 failed\n");
		else
			I2C_LOG("case 3 successed\n");
#endif

		dma_free_coherent(NULL, I2C_FIFO_SIZE, vir_addr, data_phy);
		break;

	case I2C_UVVF_FS_DMA_RS_DIR_400KHZ:
		ext_flag |= (I2C_DMA_FLAG | I2C_RS_FLAG | I2C_WR_FLAG);
		vir_addr = dma_alloc_coherent(NULL, I2C_FIFO_SIZE,
						&data_phy, GFP_KERNEL);
		if (!vir_addr)
			I2C_LOG("Alloc DMA buffer failed\n");

		*(u8 *)vir_addr = 0xB2;
		ret = i2c_ps_read_bytes(0xB2, ext_flag, HZ_400K,
					(void *)data_phy, I2C_FIFO_SIZE);
#if 1
		if (ret)
			I2C_LOG("case 4 failed\n");
		else
			I2C_LOG("case 4 successed\n");
#endif

		dma_free_coherent(NULL, I2C_FIFO_SIZE, vir_addr, data_phy);
		break;

	case I2C_UVVF_CLK_EXT_INT:
		ret = i2c_ps_write_bytes(0xB2, ext_flag, HZ_100K,
					&write_byte, 1);
		break;
#if 0
	case I2C_UVVF_REPEAT_RW:
		break;
	case I2C_UVVF_EXT:
		break;
	case I2C_UVVF_REPEAT_EXT:
		break;
#endif
	default:
		printk("test case error!\n");
		break;
	}

	return ret;
}

static struct file_operations mt_i2c_misc_fops =
{
	.owner = THIS_MODULE,
	.unlocked_ioctl = mt_i2c_misc_ioctl,
	.open = mt_i2c_misc_open,
};

static struct miscdevice mt_i2c_miscdevice = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = I2C_LDVT_DEVICE,
	.fops = &mt_i2c_misc_fops,
};

static int __init mt_i2c_ldvt_init(void)
{
	int ret;
	struct miscdevice *misc = &mt_i2c_miscdevice;
	ret = misc_register(misc);
	if (ret < 0)
		I2C_LOG("i2c register char fails.\n");

	/* register slaves boardinfo */
	i2c_register_board_info(I2C_BUSNUM, &i2c_ps, 1);
	i2c_register_board_info(I2C_BUSNUM, &i2c_gs, 1);

	if (i2c_add_driver(&i2c_ps_driver) != 0)
		I2C_LOG("Failed to register ps i2c driver.\n");
	if (i2c_add_driver(&i2c_gs_driver) != 0)
        	I2C_LOG("Failed to register gs i2c driver.\n");

	return ret;
}

static void __exit mt_i2c_ldvt_exit(void)
{
	I2C_LOG("mt_i2c_ldvt_exit\n");
}

module_init(mt_i2c_ldvt_init);
module_exit(mt_i2c_ldvt_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mediatek");
