/*
 *
 * (C) Copyright 2011
 * MediaTek <www.mediatek.com>
 * Infinity Chen <infinity.chen@mediatek.com>
 *
 * mt8320 I2C Bus Controller
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/wait.h>
#include <linux/mm.h>
#include <linux/dma-mapping.h>
#include <asm/scatterlist.h>
#include <linux/scatterlist.h>
#include <asm/io.h>
//#include <mach/dma.h>
#include <asm/system.h>
#include <mach/mt_reg_base.h>
#include <mach/mt_gpio.h>
#include <mach/sync_write.h>
#include "mach/memory.h"
#include "mt_i2c.h"


#define I2C_DRV_NAME		"mt-i2c"
#define I2C_NR			2
volatile u32 I2C_TIMING_REG_BACKUP[I2C_NR];

static irqreturn_t mt_i2c_dma_irq(int irqno, void *dev_id);

//#define I2C_DEBUG_FS
#ifdef I2C_DEBUG_FS
//#if 1
#define PORT_COUNT		2
#define MESSAGE_COUNT		16
#define I2C_T_DMA		1
#define I2C_T_TRANSFERFLOW	2
#define I2C_T_SPEED		3
/*2 ports, 16 types of message*/
u8 i2c_port[PORT_COUNT][MESSAGE_COUNT];

#define I2C_INFO(i2c, type, format, arg...) do { \
	if (type < MESSAGE_COUNT && type >= 0) { \
		if (i2c_port[i2c->id][0] != 0 && (i2c_port[i2c->id][type] != 0 \
				|| i2c_port[i2c->id][MESSAGE_COUNT - 1] != 0)) { \
			dev_alert(i2c->dev, format,##arg); \
		} \
	} \
} while (0)

static DEVICE_ATTR(debug, S_IRUGO|S_IWUGO, show_config, set_config);

static ssize_t show_config(struct device *dev, struct device_attribute *attr, char *buff)
{
	int i = 0;
	int j = 0;
	char *buf = buff;
	for ( i = 0; i < PORT_COUNT; i++){
		for ( j = 0; j < MESSAGE_COUNT; j++) i2c_port[i][j] += '0';
		strncpy(buf, (char *)i2c_port[i], MESSAGE_COUNT);
		buf += MESSAGE_COUNT;
		*buf = '\n';
		buf++;
		for ( j=0;j < MESSAGE_COUNT; j++) i2c_port[i][j] -= '0';
	}
	return (buf - buff);
}

static ssize_t set_config(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int port,type,status;

	if ( sscanf(buf, "%d %d %d", &port, &type, &status) ) {
		if ( port >= PORT_COUNT || port < 0 || type >= MESSAGE_COUNT || type < 0 ) {
			/*Invalid param*/
			dev_err(dev, "i2c debug system: Parameter overflowed!\n");
		} else {
			if ( status != 0 )
				i2c_port[port][type] = 1;
			else
				i2c_port[port][type] = 0;

			dev_alert(dev, "port:%d type:%d status:%s\ni2c debug system: Parameter accepted!\n", port, type, status?"on":"off");
		}
	} else {
		/*parameter invalid*/
		dev_err(dev, "i2c debug system: Parameter invalid!\n");
	}
	return count;
}

#else
#define I2C_INFO(mt_i2c, type, format, arg...)
#endif

/*this field is only for 3d camera*/
static struct i2c_msg	g_msg[2];
static struct mt_i2c	*g_i2c[2];

//I2C GPIO debug
struct mt_i2c_gpio_t{
	u16 scl;
	u16 sda;
};

static struct mt_i2c_gpio_t mt_i2c_gpio_mode[I2C_NR] = {
	{GPIO105, GPIO106},
	{GPIO113, GPIO114},
};

inline void i2c_writew(struct mt_i2c *i2c, u8 offset, u16 value)
{

	//dev_err(i2c->dev, "before i2c_writew base=%x,offset=%x\n",i2c->base,offset);
	//__raw_writew(value, (i2c->base) + (offset));
	mt65xx_reg_sync_writew(value, (void *)(i2c->base + offset));
}

inline u16 i2c_readw(struct mt_i2c * i2c, u8 offset)
{
	return __raw_readw((void *)(i2c->base + offset));
}



static void mt_i2c_clock_enable(struct mt_i2c *i2c)
{
#if (!defined(CONFIG_MT_I2C_FPGA_ENABLE))
	//I2C_INFO(i2c, I2C_T_TRANSFERFLOW, "Before clock setting .....\n");
	enable_clock(i2c->pdn, i2c->adap.name);
	enable_clock(MT_CG_APDMA_SW_CG, i2c->adap.name);

	I2C_INFO(i2c, I2C_T_TRANSFERFLOW, "clock setting done.....\n");
#endif
	return;
}
static void mt_i2c_clock_disable(struct mt_i2c *i2c)
{
#if (!defined(CONFIG_MT_I2C_FPGA_ENABLE))
	disable_clock(i2c->pdn, i2c->adap.name);
	disable_clock(MT_CG_APDMA_SW_CG, i2c->adap.name);
 
	I2C_INFO(i2c, I2C_T_TRANSFERFLOW, "clock disable done .....\n");
#endif
	return;
}
/*Set i2c port speed*/
static int mt_i2c_set_speed(struct mt_i2c *i2c,struct i2c_msg *msg)
{
	int ret = 0;
	int mode = 0;
	unsigned long khz = 0;
	//u32 base = i2c->base;
	unsigned short step_cnt_div = 0;
	unsigned short sample_cnt_div = 0;
	unsigned long tmp, sclk, hclk = i2c->clk;
	unsigned short max_step_cnt_div = 0;
	unsigned long diff, min_diff = i2c->clk;
	unsigned short sample_div = MAX_SAMPLE_CNT_DIV;
	unsigned short step_div = max_step_cnt_div;
	unsigned short i2c_timing_reg = 0;
	//dev_err(i2c->dev, "mt_i2c_set_speed=================\n");
	if (0 == (msg->timing & 0xFFFF)) {
		mode = ST_MODE;
		khz = MAX_ST_MODE_SPEED;
	} else {
		if ((msg->ext_flag & I2C_HS_FLAG)) {
			mode = HS_MODE;
			khz = msg->timing & 0xFFFF;
		} else {
			mode = FS_MODE;
			khz = msg->timing & 0xFFFF;
		}
	}

	max_step_cnt_div = (mode == HS_MODE) ? MAX_HS_STEP_CNT_DIV : MAX_STEP_CNT_DIV;

	if ((mode == FS_MODE && khz > MAX_FS_MODE_SPEED) 
	    || (mode == HS_MODE && khz > MAX_HS_MODE_SPEED)) {
		dev_err(i2c->dev, "mt-i2c: the speed is too fast for this mode.\n");
		I2C_BUG_ON((mode == FS_MODE && khz > MAX_FS_MODE_SPEED) 
			|| (mode == HS_MODE && khz > MAX_HS_MODE_SPEED));
		ret = -EINVAL;
		goto end;
	}
	i2c_timing_reg = i2c_readw(i2c, OFFSET_TIMING);
	if ((mode == i2c->mode) && (khz == i2c->speed) 
	    && (i2c_timing_reg == I2C_TIMING_REG_BACKUP[i2c->id])) {
		I2C_INFO(i2c, I2C_T_SPEED, "mt-i2c: set sclk to %ldkhz\n", i2c->speed);
		return 0;
	}

	/*Find the best combination*/
	for (sample_cnt_div = 1; sample_cnt_div <= MAX_SAMPLE_CNT_DIV; sample_cnt_div++) {
		for (step_cnt_div = 1; step_cnt_div <= max_step_cnt_div; step_cnt_div++) {
			sclk = (hclk >> 1) / (sample_cnt_div * step_cnt_div);
			if (sclk > khz)
				continue;
			diff = khz - sclk;
			if (diff < min_diff) {
				min_diff = diff;
				sample_div = sample_cnt_div;
				step_div   = step_cnt_div;
			}
		}
	}

	sample_cnt_div = sample_div;
	step_cnt_div   = step_div;

	sclk = hclk / (2 * sample_cnt_div * step_cnt_div);
	if (sclk > khz) {
		dev_err(i2c->dev, "%s mode: unsupported speed (%ldkhz)\n",
			(mode == HS_MODE) ? "HS" : "ST/FT", khz);
		I2C_BUG_ON(sclk > khz);
		ret = -ENOTSUPP;
		goto end;
	}

	step_cnt_div--;
	sample_cnt_div--;

	spin_lock(&i2c->lock);

	if (mode == HS_MODE) {
		/* TODO: define macro instead of hard-code */
		/*Set the timing control register*/
		/* clear TIMING REG */
		tmp	= i2c_readw(i2c, OFFSET_TIMING) & ~((0x7 << 8) | (0x3f << 0));
		tmp	= (0 & 0x7) << 8 | (16 & 0x3f) << 0 | tmp;
		i2c_writew(i2c, OFFSET_TIMING, tmp);
		I2C_TIMING_REG_BACKUP[i2c->id] = tmp;

		/*Set the hign speed mode register*/
		tmp	= i2c_readw(i2c, OFFSET_HS) & ~((0x7 << 12) | (0x7 << 8));
		tmp	= (sample_cnt_div & 0x7) << 12 | (step_cnt_div & 0x7) << 8 | tmp;
		/*Enable the hign speed transaction*/
		tmp	|= 0x0001;
		i2c_writew(i2c, OFFSET_HS, tmp);
	} else {
		/*Set non-highspeed timing*/
		tmp  = i2c_readw(i2c, OFFSET_TIMING) & ~((0x7 << 8) | (0x3f << 0));
		tmp  = (sample_cnt_div & 0x7) << 8 | (step_cnt_div & 0x3f) << 0 | tmp;
		i2c_writew(i2c, OFFSET_TIMING, tmp);
		I2C_TIMING_REG_BACKUP[i2c->id] = tmp;

		/*Disable the high speed transaction*/
		//dev_err(i2c->dev, "NOT HS_MODE============================1\n");
		tmp	= i2c_readw(i2c, OFFSET_HS) & ~(0x0001);
		//dev_err(i2c->dev, "NOT HS_MODE============================2\n");
		i2c_writew(i2c, OFFSET_HS, tmp);
		//dev_err(i2c->dev, "NOT HS_MODE============================3\n");
	}
	i2c->mode = mode;
	i2c->speed = khz;
	spin_unlock(&i2c->lock);
	I2C_INFO(i2c, I2C_T_SPEED, "mt-i2c: set sclk to %ldkhz(orig:%ldkhz), sample=%d,step=%d\n", sclk, khz, sample_cnt_div, step_cnt_div);
end:
	return ret;
}

static void mt_i2c_post_isr(struct mt_i2c *i2c,struct i2c_msg *msg)
{
	//u16 addr = msg->addr;

	if (i2c->irq_stat & I2C_TRANSAC_COMP) {
		atomic_set(&i2c->trans_err, 0);
		atomic_set(&i2c->trans_comp, 1);
	}

	if (i2c->irq_stat & I2C_HS_NACKERR) {
		if (likely(!(msg->ext_flag & I2C_A_FILTER_MSG)))
			dev_err(i2c->dev, "I2C_HS_NACKERR\n");
	}

	if (i2c->irq_stat & I2C_ACKERR) {
		if (likely(!(msg->ext_flag & I2C_A_FILTER_MSG)))
			dev_err(i2c->dev, "I2C_ACKERR\n");
	}

	if (i2c->irq_stat & I2C_TIMEOUT) {
		if (likely(!(msg->ext_flag & I2C_A_FILTER_MSG)))
			dev_err(i2c->dev, "I2C_TIMEOUT\n");
	}

	atomic_set(&i2c->trans_err, i2c->irq_stat & 
				(I2C_TIMEOUT | I2C_HS_NACKERR | I2C_ACKERR));
}

static inline void mt_i2c_dump_info(struct mt_i2c *i2c)
{
	dev_err(i2c->dev, "I2C structure:\nMode %x\nSt_rs %x\nDma_en %x\nOp %x\n"
			"Poll_en %x\nTrans_len %x\nTrans_num %x\nTrans_auxlen %x\nData_size %x\n"
			"Irq_stat %x\nTrans_stop %u\nTrans_comp %u\nTrans_error %u\n",
		   i2c->mode,
		   i2c->st_rs,
		   i2c->dma_en,
		   i2c->op,
		   i2c->poll_en,
		   i2c->trans_data.trans_len,
		   i2c->trans_data.trans_num,
		   i2c->trans_data.trans_auxlen,
		   i2c->trans_data.data_size,
		   i2c->irq_stat,
		   atomic_read(&i2c->trans_stop),
		   atomic_read(&i2c->trans_comp),
		   atomic_read(&i2c->trans_err));
	dev_err(i2c->dev,"base address %x\n",i2c->base);
	dev_err(i2c->dev, "I2C register:\nSLAVE_ADDR %x\nINTR_MASK %x\nINTR_STAT %x\n"
			"CONTROL %x\nTRANSFER_LEN %x\nTRANSAC_LEN %x\nDELAY_LEN %x\n"
			"TIMING %x\n START %x\nFIFO_STAT %x\nIO_CONFIG %x\nHS %x\nDEBUGSTAT %x\n"
			"EXT_CONF %x\nTRANSFER_LEN_AUX %x\nTIMEOUT %x\n",
			(i2c_readw(i2c, OFFSET_SLAVE_ADDR)),
			(i2c_readw(i2c, OFFSET_INTR_MASK)),
			(i2c_readw(i2c, OFFSET_INTR_STAT)),
			(i2c_readw(i2c, OFFSET_CONTROL)),
			(i2c_readw(i2c, OFFSET_TRANSFER_LEN)),
			(i2c_readw(i2c, OFFSET_TRANSAC_LEN)),
			(i2c_readw(i2c, OFFSET_DELAY_LEN)),
			(i2c_readw(i2c, OFFSET_TIMING)),
			(i2c_readw(i2c, OFFSET_START)),
			(i2c_readw(i2c, OFFSET_FIFO_STAT)),
			(i2c_readw(i2c, OFFSET_IO_CONFIG)),
			(i2c_readw(i2c, OFFSET_HS)),
			(i2c_readw(i2c, OFFSET_DEBUGSTAT)),
			(i2c_readw(i2c, OFFSET_EXT_CONF)),
			(i2c_readw(i2c, OFFSET_TRANSFER_LEN_AUX)),
			(i2c_readw(i2c, OFFSET_TIMEOUT)));
	/*
	dev_err(i2c->dev, "DMA register:\nINT_FLAG %x\nCON %x\nTX_MEM_ADDR %x\nRX_MEM_ADDR %x\nTX_LEN %x\nRX_LEN %x\nINT_EN %x\nEN %x\n",
			(__raw_readw(i2c->pdmabase+OFFSET_INT_FLAG)),
			(__raw_readw(i2c->pdmabase+OFFSET_CON)),
			(__raw_readw(i2c->pdmabase+OFFSET_TX_MEM_ADDR)),
			(__raw_readw(i2c->pdmabase+OFFSET_RX_MEM_ADDR)),
			(__raw_readw(i2c->pdmabase+OFFSET_TX_LEN)),
			(__raw_readw(i2c->pdmabase+OFFSET_RX_LEN)),
			(__raw_readw(i2c->pdmabase+OFFSET_INT_EN)),
			(__raw_readw(i2c->pdmabase+OFFSET_EN)));
	*/
#if 0	/* show clock register */
	dev_err(i2c->dev, "Clock %s\n",
			(((__raw_readw(0xF0003018)>>26) | (__raw_readw(0xF000301c)&0x1 << 6)) & (1 << i2c->id))?"disable":"enable");
#endif
	dev_err(i2c->dev,"GPIO%d(SCL):%d,mode%d; GPIO%d(SDA):%d,mode%d\n",
			mt_i2c_gpio_mode[i2c->id].scl,
			mt_get_gpio_in(mt_i2c_gpio_mode[i2c->id].scl),
			mt_get_gpio_mode(mt_i2c_gpio_mode[i2c->id].scl),
			mt_i2c_gpio_mode[i2c->id].sda,
			mt_get_gpio_in(mt_i2c_gpio_mode[i2c->id].sda),
			mt_get_gpio_mode(mt_i2c_gpio_mode[i2c->id].sda));

	return;

}

static int mt_deal_result(struct mt_i2c *i2c, struct i2c_msg *msg)
{
	long tmo = i2c->adap.timeout;
	u16 data_size = 0;
	u8 *ptr = msg->buf;
	u16 addr = msg->addr;
	int ret = msg->len;
	u16 read = (msg->flags & I2C_M_RD);
	long tmo_poll = 0xffff;

	addr = read ? ((addr << 1) | 0x1) : ((addr << 1) & ~0x1);

	if(i2c->poll_en) { /*master read && poll mode*/
		for (;;) { /*check the interrupt status register*/
			i2c->irq_stat = i2c_readw(i2c, OFFSET_INTR_STAT);
			if(i2c->irq_stat & 
				(I2C_TIMEOUT | I2C_HS_NACKERR | I2C_ACKERR | I2C_TRANSAC_COMP)) {
				atomic_set(&i2c->trans_stop, 1);
				spin_lock(&i2c->lock);
				/*Clear interrupt status,write 1 clear*/
				i2c_writew(i2c, OFFSET_INTR_STAT, (I2C_TIMEOUT | I2C_HS_NACKERR | I2C_ACKERR | I2C_TRANSAC_COMP));
				spin_unlock(&i2c->lock);
				break;
			}
			tmo_poll--;
			if(tmo_poll == 0) {
				tmo = 0;
				break;
			}
		}
	} else { /*Interrupt mode,wait for interrupt wake up*/
		tmo = wait_event_timeout(i2c->wait,atomic_read(&i2c->trans_stop), tmo);
		//wait_event(i2c->wait, atomic_read(&i2c->trans_stop));
	}

	/*Save register status to i2c struct*/
	mt_i2c_post_isr(i2c, msg);

	/*Check the transfer status*/
	if (!(tmo == 0 || atomic_read(&i2c->trans_err))) {
		/*Transfer success ,we need to get data from fifo*/
		if(!i2c->dma_en && (i2c->op == I2C_MASTER_RD || i2c->op == I2C_MASTER_WRRD) ) { /*only read mode or write_read mode and fifo mode need to get data*/
			ptr = msg->buf;
			data_size = (i2c_readw(i2c, OFFSET_FIFO_STAT) >> 4) & 0x000F;
			while (data_size--) {
				*ptr = i2c_readw(i2c, OFFSET_DATA_PORT);
				//dev_info(i2c->dev, "addr %.2x read byte = 0x%.2X\n", addr, *ptr);
				ptr++;
			}
		}
	} else { /*Timeout or ACKERR*/
		if (tmo == 0) {
			dev_err(i2c->dev, "addr: %.2x, transfer timeout\n", addr);
			ret = -ETIMEDOUT;
		} else {
			dev_err(i2c->dev, "addr: %.2x, transfer error\n", addr);
			ret = -EREMOTEIO;
		}

		if (likely(!(msg->ext_flag & I2C_A_FILTER_MSG))) { /*Dump i2c_struct & register*/
			mt_i2c_dump_info(i2c);
		}

		spin_lock(&i2c->lock);
		/*Reset i2c port*/
		i2c_writew(i2c, OFFSET_SOFTRESET, 0x0001);
		/*Set slave address*/
		i2c_writew( i2c, OFFSET_SLAVE_ADDR, 0x0000 );
		/*Clear interrupt status*/
		i2c_writew(i2c, OFFSET_INTR_STAT, 
			(I2C_TIMEOUT | I2C_HS_NACKERR | I2C_ACKERR | I2C_TRANSAC_COMP));
		/*Clear fifo address*/
		i2c_writew(i2c, OFFSET_FIFO_ADDR_CLR, 0x0001);

		spin_unlock(&i2c->lock);
	}

	return ret;
}

static int mt_i2c_start_xfer(struct mt_i2c *i2c, struct i2c_msg *msg)
{
	u16 control = 0;
	u16 trans_num = 0;
	u16 data_size = 0;
	u16 trans_len = 0;
	u8 *ptr = msg->buf;
	u16 len = msg->len;
	int ret = len;
	u16 trans_auxlen = 0;
	u16 addr = msg->addr;
	u16 flags = msg->flags;
	u16 read = (flags & I2C_M_RD);


	/*Check param valid*/
	if(addr == 0){
		dev_err(i2c->dev, "mt-i2c: addr is invalid.\n");
		I2C_BUG_ON(addr == NULL);
		ret = -EINVAL;
		goto err;
	}

	if(msg->buf == NULL){
		dev_err(i2c->dev, "mt-i2c: data buffer is NULL.\n");
		I2C_BUG_ON(msg->buf == NULL);
		ret = -EINVAL;
		goto err;
	}
	//compatible with 77/75 driver
	if(msg->addr & 0xFF00){
		//dev_err(i2c->dev, "I2C Slave device driver parameter used warning ! please donot add any external flags at the high 8 bits of 16-bit address\n");
		msg->ext_flag |= msg->addr & 0xFF00;
		//I2C_BUG_ON(msg->addr & 0xFF00);

		/*go on...*/

	//	ret = -EINVAL;
	//	goto err;
	}

	if (g_i2c[0] == i2c || g_i2c[1] == i2c) {
		dev_err(i2c->dev, "mt-i2c%d: Current I2C Adapter is busy.\n", i2c->id);
		ret = -EINVAL;
		goto err;
	}

	I2C_INFO(i2c, I2C_T_TRANSFERFLOW, "Before i2c transfer .....\n");

	if (msg->ext_flag & I2C_DIRECTION_FLAG) {
		if(i2c->id <= 3){
			dev_err(i2c->dev, "the adapter do not support wrapper.\n");
			ret = -EINVAL;
			goto err;
		}
	//	dev_err(i2c->dev, "======enable wrapper=========\n");
		//i2c_writew(i2c, OFFSET_PATH_DIR, I2C_CONTROL_WRAPPER); /*We must set wrapper bit before setting other register*/

		i2c->clk = I2C_CLK_WRAPPER_RATE;
	} else {
#if (defined(CONFIG_MT_I2C_FPGA_ENABLE))
		i2c->clk	= I2C_CLK_RATE;
#else
		i2c->clk = mt_get_bus_freq() / I2C_CLK_DIV;
#endif
	}

	//dev_err(i2c->dev, "======set speed1=========\n");
	/*Set device speed*/
	if ( mt_i2c_set_speed(i2c, msg) < 0 ){
		ret =-EINVAL;
		//dev_err(i2c->dev, "======================\n");
		goto err;
	}
	/*set start condition */
	if((msg->timing & 0xFFFF) <= 100){
		i2c_writew(i2c,OFFSET_EXT_CONF, 0x8001);
	}
	//dev_err(i2c->dev, "======set speed2=========\n");

	/*prepare data*/
	if((msg->ext_flag & I2C_RS_FLAG))
		i2c->st_rs = I2C_TRANS_REPEATED_START;
	else
		i2c->st_rs = I2C_TRANS_STOP;

	if((msg->ext_flag & I2C_DMA_FLAG))
		i2c->dma_en = true;
	else
		i2c->dma_en = false;

	if(i2c->dma_en) {
		I2C_INFO(i2c, I2C_T_DMA, "DMA Transfer mode!\n");
  		if (i2c->pdmabase_tx == 0 || i2c->pdmabase_rx == 0) {
  			dev_err(i2c->dev, "mt-i2c: I2C%d doesnot support DMA mode!\n",i2c->id);
			I2C_BUG_ON(i2c->pdmabase_tx == NULL);
			ret = -EINVAL;
			goto err;
		}
		if((u32)ptr > DMA_ADDRESS_HIGH) {
			dev_err(i2c->dev, "mt-i2c: DMA mode should use physical buffer address!\n");
			I2C_BUG_ON((u32)ptr > DMA_ADDRESS_HIGH);
			ret = -EINVAL;
			goto err;
		}
	}

	i2c->delay_len = (msg->timing & 0xff0000) >> 16;

	if(0 == i2c->delay_len)
		i2c->delay_len = 2;

	if ((msg->ext_flag & I2C_WR_FLAG))
		i2c->op = I2C_MASTER_WRRD;
	else {
		if(msg->flags & I2C_M_RD)
	   		i2c->op = I2C_MASTER_RD;
		else
	   		i2c->op = I2C_MASTER_WR;
	}

	if(msg->ext_flag & I2C_POLLING_FLAG)
		i2c->poll_en = true;
	else
		i2c->poll_en = false;

	atomic_set(&i2c->trans_stop, 0);
	atomic_set(&i2c->trans_comp, 0);
	atomic_set(&i2c->trans_err, 0);
	i2c->irq_stat = 0;
	addr = read ? ((addr << 1) | 0x1) : ((addr << 1) & ~0x1);

	/*Get Transfer len and transaux len*/
	if(false == i2c->dma_en) { /*non-DMA mode*/
		if(I2C_MASTER_WRRD != i2c->op) {
			trans_len = (msg->len) & 0xFF;
			trans_num = (msg->len >> 8) & 0xFF;
			if(0 == trans_num)
				trans_num = 1;
			trans_auxlen = 0;
			data_size = trans_len*trans_num;

			if(!trans_len || !trans_num || trans_len*trans_num > 8) {
				dev_err(i2c->dev, "mt-i2c: non-WRRD transfer length is not right. trans_len=%x, tans_num=%x, trans_auxlen=%x\n", trans_len, trans_num, trans_auxlen);
				I2C_BUG_ON(!trans_len || !trans_num || trans_len*trans_num > 8);
				ret = -EINVAL;
				goto err;
			}
		} else {
			trans_len = (msg->len) & 0xFF;
			trans_auxlen = (msg->len >> 8) & 0xFF;
			trans_num = 2;
			data_size = trans_len;

			if(!trans_len || !trans_auxlen || trans_len > 8 || trans_auxlen > 8) {
				dev_err(i2c->dev, "mt-i2c: WRRD transfer length is not right. trans_len=%x, tans_num=%x, trans_auxlen=%x\n", trans_len, trans_num, trans_auxlen);
				I2C_BUG_ON(!trans_len || !trans_auxlen || trans_len > 8 || trans_auxlen > 8);
				ret = -EINVAL;
				goto err;
			}
		}
	} else { /*DMA mode*/
		if(I2C_MASTER_WRRD != i2c->op) {
			trans_len = (msg->len) & 0xFF;
			trans_num = (msg->len >> 8) & 0xFF;
			if(0 == trans_num)
				trans_num = 1;
			trans_auxlen = 0;
			data_size = trans_len*trans_num;

			if(!trans_len || !trans_num || trans_len > 255 || trans_num > 255) {
				dev_err(i2c->dev, "mt-i2c: DMA non-WRRD transfer length is not right. trans_len=%x, tans_num=%x, trans_auxlen=%x\n", trans_len, trans_num, trans_auxlen);
				I2C_BUG_ON(!trans_len || !trans_num || trans_len > 255 || trans_num > 255);
				ret = -EINVAL;
				goto err;
			}
		I2C_INFO(i2c, I2C_T_DMA, "DMA non-WRRD mode!trans_len=%x, tans_num=%x, trans_auxlen=%x\n",trans_len, trans_num, trans_auxlen);
		} else {
			trans_len = (msg->len) & 0xFF;
			trans_auxlen = (msg->len >> 8) & 0xFF;
			trans_num = 2;
			data_size = trans_len;
			if(!trans_len || !trans_auxlen || trans_len > 255 || trans_auxlen > 31) {
				dev_err(i2c->dev, "mt-i2c: DMA WRRD transfer length is not right. trans_len=%x, tans_num=%x, trans_auxlen=%x\n", trans_len, trans_num, trans_auxlen);
				I2C_BUG_ON(!trans_len || !trans_auxlen || trans_len > 255 || trans_auxlen > 31);
				ret = -EINVAL;
				goto err;
			}
		I2C_INFO(i2c, I2C_T_DMA, "DMA WRRD mode!trans_len=%x, tans_num=%x, trans_auxlen=%x\n",trans_len, trans_num, trans_auxlen);
		}
	}

	spin_lock(&i2c->lock);

	/*Set ioconfig*/
	if ((msg->ext_flag & I2C_PUSHPULL_FLAG)) {
			i2c_writew(i2c, OFFSET_IO_CONFIG, 0x0000);
		} else {
			i2c_writew(i2c, OFFSET_IO_CONFIG, 0x0003);
		}

	/*Set Control Register*/
	control = I2C_CONTROL_TIMEOUT_EN | I2C_CONTROL_ACKERR_DET_EN 
		| I2C_CONTROL_CLK_EXT_EN;

	if(i2c->dma_en)
		control |= I2C_CONTROL_DMA_EN;

	if(I2C_MASTER_WRRD == i2c->op)
		control |= I2C_CONTROL_DIR_CHANGE;

	if(HS_MODE == i2c->mode || 
		(trans_num > 1 && I2C_TRANS_REPEATED_START == i2c->st_rs)) {
		control |= I2C_CONTROL_RS;
	}

#if 0 // 6589 only
	if (msg->ext_flag & I2C_DIRECTION_FLAG) {
		control |= I2C_CONTROL_WRAPPER; /*PMIC WRAPPER */
	}
#endif

	i2c_writew(i2c, OFFSET_CONTROL, control);
	if (~control & I2C_CONTROL_RS) {
		// bit is not set to 1, i.e.,use STOP
		i2c_writew(i2c, OFFSET_DELAY_LEN, i2c->delay_len);
	}

	/*Setup Registers*/

	/*Set slave address*/
	i2c_writew(i2c, OFFSET_SLAVE_ADDR, addr);
	/*Clear interrupt status*/
	i2c_writew(i2c, OFFSET_INTR_STAT, 
		(I2C_TIMEOUT | I2C_HS_NACKERR | I2C_ACKERR | I2C_TRANSAC_COMP));
	/*Clear fifo address*/
	i2c_writew(i2c, OFFSET_FIFO_ADDR_CLR, 0x0001);
	/*Setup the interrupt mask flag*/
	if (i2c->poll_en) {
		/*Disable interrupt*/
		i2c_writew(i2c, OFFSET_INTR_MASK, i2c_readw(i2c, OFFSET_INTR_MASK) & 
		~(I2C_TIMEOUT | I2C_HS_NACKERR | I2C_ACKERR | I2C_TRANSAC_COMP));
	} else {
		/*Enable interrupt*/
		i2c_writew(i2c, OFFSET_INTR_MASK, i2c_readw(i2c, OFFSET_INTR_MASK) | 
		(I2C_TIMEOUT | I2C_HS_NACKERR | I2C_ACKERR | I2C_TRANSAC_COMP));
	}

	/*Set transfer len */
	/* 6572 separates AUX_LEN and TRANSFER_LEN register */
	//i2c_writew(i2c, OFFSET_TRANSFER_LEN, ((trans_auxlen & 0x1F) << 8) | (trans_len & 0xFF));
	i2c_writew(i2c, OFFSET_TRANSFER_LEN, trans_len);
	i2c_writew(i2c, OFFSET_TRANSFER_LEN_AUX, trans_auxlen);

	/*Set transaction len*/
	i2c_writew(i2c, OFFSET_TRANSAC_LEN, trans_num & 0xFF);

	/*Prepare buffer data to start transfer*/
	if(i2c->dma_en) {
		/* reset I2C DMA status */
		mt65xx_reg_sync_writel(0x0001, (void *)(i2c->pdmabase_tx + OFFSET_RST));
		mt65xx_reg_sync_writel(0x0001, (void *)(i2c->pdmabase_rx + OFFSET_RST));
		if (I2C_MASTER_RD == i2c->op) {
			mt65xx_reg_sync_writel(0x0000, (void *)(i2c->pdmabase_rx + OFFSET_INT_FLAG));
			mt65xx_reg_sync_writel(0x0001, (void *)(i2c->pdmabase_rx + OFFSET_CON));
			mt65xx_reg_sync_writel((u32)msg->buf, (void *)(i2c->pdmabase_rx + OFFSET_MEM_ADDR));
			mt65xx_reg_sync_writel(data_size, (void *)(i2c->pdmabase_rx + OFFSET_LEN));
			mb();
			mt65xx_reg_sync_writel(0x0001, (void *)(i2c->pdmabase_rx + OFFSET_EN));
		} else if (I2C_MASTER_WR == i2c->op) {
			mt65xx_reg_sync_writel(0x0000, (void *)(i2c->pdmabase_tx + OFFSET_INT_FLAG));
			mt65xx_reg_sync_writel(0x0000, (void *)(i2c->pdmabase_tx + OFFSET_CON));
			mt65xx_reg_sync_writel((u32)msg->buf, (void *)(i2c->pdmabase_tx + OFFSET_MEM_ADDR));
			mt65xx_reg_sync_writel(data_size, (void *)(i2c->pdmabase_tx + OFFSET_LEN));
			mb();
			mt65xx_reg_sync_writel(0x0001, (void *)(i2c->pdmabase_tx + OFFSET_EN));
		} else {
			mt65xx_reg_sync_writel(0x0000, (void *)(i2c->pdmabase_tx + OFFSET_INT_FLAG));
			mt65xx_reg_sync_writel(0x0001, (void *)(i2c->pdmabase_tx + OFFSET_INT_EN));
			mt65xx_reg_sync_writel(0x0000, (void *)(i2c->pdmabase_rx + OFFSET_INT_FLAG));
			mt65xx_reg_sync_writel(0x0000, (void *)(i2c->pdmabase_tx + OFFSET_CON));
			mt65xx_reg_sync_writel(0x0001, (void *)(i2c->pdmabase_rx + OFFSET_CON));
			mt65xx_reg_sync_writel((u32)msg->buf, (void *)(i2c->pdmabase_tx + OFFSET_MEM_ADDR));
			mt65xx_reg_sync_writel((u32)msg->buf, (void *)(i2c->pdmabase_rx + OFFSET_MEM_ADDR));
			mt65xx_reg_sync_writel(trans_len, (void *)(i2c->pdmabase_tx + OFFSET_LEN));
			mt65xx_reg_sync_writel(trans_auxlen, (void *)(i2c->pdmabase_rx + OFFSET_LEN));
			mb();
			mt65xx_reg_sync_writel(0x0001, (void *)(i2c->pdmabase_tx + OFFSET_EN));
		}

		I2C_INFO(i2c, I2C_T_DMA, "addr %.2x dma %.2X byte\n", addr, data_size);
		I2C_INFO(i2c, I2C_T_DMA, "DMA Register:INT_FLAG:0x%x,INT_EN:0x%x,CON:0x%x,\
				TX_MEM_ADDR:0x%x,TX_LEN:0x%x,EN:0x%x\n",
				  readw(i2c->pdmabase_tx + OFFSET_INT_FLAG),
				  readw(i2c->pdmabase_tx + OFFSET_INT_EN),
				  readw(i2c->pdmabase_tx + OFFSET_CON),
				  readw(i2c->pdmabase_tx + OFFSET_MEM_ADDR),
				  readw(i2c->pdmabase_tx + OFFSET_LEN),
				  readw(i2c->pdmabase_tx + OFFSET_EN));

	} else {/*Set fifo mode data*/
		if (I2C_MASTER_RD == i2c->op) {
			/*do not need set fifo data*/
		} else { /*both write && write_read mode*/
			while (data_size--) {
				i2c_writew(i2c, OFFSET_DATA_PORT, *ptr);
				//dev_info(i2c->dev, "addr %.2x write byte = 0x%.2X\n", addr, *ptr);
				ptr++;
			}
		}
	}

	/*Set trans_data*/
	i2c->trans_data.trans_num = trans_num;
	i2c->trans_data.data_size = data_size;
	i2c->trans_data.trans_len = trans_len;
	i2c->trans_data.trans_auxlen = trans_auxlen;


	/*All register must be prepared before setting the start bit [SMP]*/
	mb();

	/*This is only for 3D CAMERA*/
	if (msg->ext_flag & I2C_3DCAMERA_FLAG) {
		spin_unlock(&i2c->lock);

		if (g_msg[0].buf == NULL)
			memcpy((void *)&g_msg[0], msg, sizeof(struct i2c_msg)); /*Save address infomation for 3d camera*/
		else
			memcpy((void *)&g_msg[1], msg, sizeof(struct i2c_msg)); /*Save address infomation for 3d camera*/

		if (g_i2c[0] == NULL)
			g_i2c[0] = i2c;
		else
			g_i2c[1] = i2c;

		goto end;
	}

	I2C_INFO(i2c, I2C_T_TRANSFERFLOW, "Before start .....\n");

	/*Start the transfer*/
	i2c_writew(i2c, OFFSET_START, 0x0001);
	spin_unlock(&i2c->lock);

	ret = mt_deal_result(i2c, msg);

	I2C_INFO(i2c, I2C_T_TRANSFERFLOW, "After i2c transfer .....\n");
err:

end:
	return ret;
}

/*Interrupt handler function*/
static irqreturn_t mt_i2c_irq(int irqno, void *dev_id)
{
	struct mt_i2c *i2c = (struct mt_i2c *)dev_id;
	//u32 base = i2c->base;

	I2C_INFO(i2c, I2C_T_TRANSFERFLOW, "i2c interrupt coming.....\n");

	/*Clear interrupt mask*/
	i2c_writew(i2c, OFFSET_INTR_MASK, i2c_readw(i2c,OFFSET_INTR_MASK) 
		& ~(I2C_TIMEOUT | I2C_HS_NACKERR | I2C_ACKERR | I2C_TRANSAC_COMP));
	/*Save interrupt status*/
	i2c->irq_stat = i2c_readw(i2c, OFFSET_INTR_STAT);
	/*Clear interrupt status,write 1 clear*/
	i2c_writew(i2c,OFFSET_INTR_STAT, 
		(I2C_TIMEOUT | I2C_HS_NACKERR | I2C_ACKERR | I2C_TRANSAC_COMP));
	//dev_info(i2c->dev, "I2C interrupt status 0x%04X\n", i2c->irq_stat);

	/*Wake up process*/
	atomic_set(&i2c->trans_stop, 1);
	wake_up(&i2c->wait);
	return IRQ_HANDLED;
}

static irqreturn_t mt_i2c_dma_irq(int irqno, void *dev_id)
{
	struct mt_i2c *i2c = (struct mt_i2c *)dev_id;

	mt65xx_reg_sync_writel(0x0000, (void *)(i2c->pdmabase_tx + OFFSET_INT_FLAG));
	mt65xx_reg_sync_writel(0x0001, (void *)(i2c->pdmabase_rx + OFFSET_EN));
	I2C_INFO(i2c, I2C_T_TRANSFERFLOW, "i2c TX DMA interrupt finished\n");

	return IRQ_HANDLED;
}

static int mt_i2c_do_transfer(struct mt_i2c *i2c, struct i2c_msg *msgs, int num)
{
	int ret = 0;
	int left_num = num;

	while (left_num--) {
		ret = mt_i2c_start_xfer(i2c, msgs++);
		if (ret < 0) {
			/*We never try again when the param is invalid*/
			if (ret != -EINVAL)
				return -EAGAIN;
			else
				return -EINVAL;
		}
	}
	/*the return value is number of executed messages*/
	return num;
}

static int mt_i2c_transfer(struct i2c_adapter *adap, struct i2c_msg msgs[], int num)
{
	int	ret = 0;
	int	retry;
	struct mt_i2c *i2c = i2c_get_adapdata(adap);
	mt_i2c_clock_enable(i2c);

	for (retry = 0; retry < adap->retries; retry++) {
		ret = mt_i2c_do_transfer(i2c, msgs, num);
		if (ret != -EAGAIN) {
			break;
		}
		//dev_info(i2c->dev, "Retrying transmission (%d)\n", retry);

		if ( retry < adap->retries - 1 )
			udelay(100);
	}
//	if (!(msg[0]->ext_flag & I2C_3DCAMERA_FLAG))
	mt_i2c_clock_disable(i2c);
	if (ret != -EAGAIN)
		return ret;
	else
		return -EREMOTEIO;
}

/*This function is only for 3d camera*/
int mt_wait4_i2c_complete(void)
{
	struct mt_i2c *i2c0 = g_i2c[0];
	struct mt_i2c *i2c1 = g_i2c[1];
	int result0, result1;
	int ret = 0;

	if ((i2c0 == NULL) ||(i2c1 == NULL)) {
		/*What's wrong?*/
		ret = -EINVAL;
		goto end;
	}

	result0 = mt_deal_result(i2c0, &g_msg[0]);
	result1 = mt_deal_result(i2c1, &g_msg[1]);

  if (result0 < 0 || result1 < 0) {
  	ret = -EINVAL;
  }

end:
	g_i2c[0] = NULL;
	g_i2c[1] = NULL;

	g_msg[0].buf = NULL;
	g_msg[1].buf = NULL;

	return ret;
}
EXPORT_SYMBOL(mt_wait4_i2c_complete);

static u32 mt_i2c_functionality(struct i2c_adapter *adap)
{
	return I2C_FUNC_I2C | I2C_FUNC_10BIT_ADDR | I2C_FUNC_SMBUS_EMUL;
}

static struct i2c_algorithm mt_i2c_algorithm = {
	.master_xfer   = mt_i2c_transfer,
	.smbus_xfer    = NULL,
	.functionality = mt_i2c_functionality,
};

static inline void mt_i2c_init_hw(struct mt_i2c *i2c)
{
	i2c_writew(i2c,OFFSET_SOFTRESET, 0x0001);
}

static void mt_i2c_free(struct mt_i2c *i2c)
{
	if (!i2c)
		return;

	free_irq(i2c->irqnr, i2c);
	switch (i2c->id) {
	case 0:
		free_irq(MT_DMA_I2C0_TX_IRQ_ID, i2c);
		break;
	case 1:
		free_irq(MT_DMA_I2C1_TX_IRQ_ID, i2c);
		break;
	default:
		dev_err(i2c->adap.dev.parent, "Error id %d\n", i2c->id);
	}
	i2c_del_adapter(&i2c->adap);
	kfree(i2c);
}

static int mt_i2c_probe(struct platform_device *pdev)
{
	int ret, irq;
	struct mt_i2c *i2c = NULL;
	struct resource *res;

	/* Request platform_device IO resource*/
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	irq = platform_get_irq(pdev, 0);
	if (res == NULL || irq < 0)
	     return -ENODEV;

	/* Request IO memory */
	if (!request_mem_region(res->start, resource_size(res), pdev->name)) {
		return -ENOMEM;
	}

	if (NULL == (i2c = kzalloc(sizeof(struct mt_i2c), GFP_KERNEL)))
		return -ENOMEM;

	/* initialize mt_i2c structure */
	i2c->id		= pdev->id;
	i2c->base	= IO_PHYS_TO_VIRT(res->start);
	i2c->irqnr	= irq;
#if (defined(CONFIG_MT_I2C_FPGA_ENABLE))
	i2c->clk	= I2C_CLK_RATE;
#else
	i2c->clk	= mt_get_bus_freq() / I2C_CLK_DIV;

	//FIX me if clock manager add this Macro
	switch (i2c->id) {
	case 0:
		i2c->pdn = MT_CG_I2C0_SW_CG;
		ret = request_irq(MT_DMA_I2C0_TX_IRQ_ID, mt_i2c_dma_irq,
				IRQF_TRIGGER_LOW, dev_name(&pdev->dev), i2c);
		break;
	case 1:
		i2c->pdn = MT_CG_I2C1_SW_CG;
		ret = request_irq(MT_DMA_I2C1_TX_IRQ_ID, mt_i2c_dma_irq,
				IRQF_TRIGGER_LOW, dev_name(&pdev->dev), i2c);
		break;
	default:
		dev_err(&pdev->dev, "Error id %d\n", i2c->id);
	}

#endif
	i2c->dev	= &i2c->adap.dev;

	i2c->adap.dev.parent	= &pdev->dev;
	i2c->adap.nr			= i2c->id;
	i2c->adap.owner			= THIS_MODULE;
	i2c->adap.algo			= &mt_i2c_algorithm;
	i2c->adap.algo_data		= NULL;
	i2c->adap.timeout		= 2 * HZ; /*2s*/
	i2c->adap.retries		= 1; /*DO NOT TRY*/

	snprintf(i2c->adap.name, sizeof(i2c->adap.name), dev_name(&pdev->dev));

	i2c->pdmabase_tx = AP_DMA_BASE + 0x180 + (0x100 * (i2c->id));
	i2c->pdmabase_rx = AP_DMA_BASE + 0x200 + (0x100 * (i2c->id));

	spin_lock_init(&i2c->lock);
	init_waitqueue_head(&i2c->wait);

	ret = request_irq(irq, mt_i2c_irq, IRQF_TRIGGER_LOW, dev_name(&pdev->dev), i2c);
	if (ret){
		dev_err(&pdev->dev, "Can Not request I2C IRQ %d\n", irq);
		goto free;
	}

	mt_i2c_init_hw(i2c);
	i2c_set_adapdata(&i2c->adap, i2c);
	ret = i2c_add_numbered_adapter(&i2c->adap);
	if (ret){
		dev_err(&pdev->dev, "failed to add i2c bus to i2c core\n");
		goto free;
	}
	platform_set_drvdata(pdev, i2c);

#ifdef I2C_DEBUG_FS
	ret = device_create_file(i2c->dev, &dev_attr_debug);
	if ( ret ){
		/*Do nothing*/
	}
#endif

	return ret;

free:
	mt_i2c_free(i2c);
	return ret;
}


static int mt_i2c_remove(struct platform_device *pdev)
{
	struct mt_i2c *i2c = platform_get_drvdata(pdev);
	if (i2c) {
		platform_set_drvdata(pdev, NULL);
		mt_i2c_free(i2c);
	}
	return 0;
}

#ifdef CONFIG_PM
static int mt_i2c_suspend(struct platform_device *pdev, pm_message_t state)
{
   // struct mt_i2c *i2c = platform_get_drvdata(pdev);
    //dev_dbg(i2c->dev,"[I2C %d] Suspend!\n", i2c->id);
    return 0;
}

static int mt_i2c_resume(struct platform_device *pdev)
{
   // struct mt_i2c *i2c = platform_get_drvdata(pdev);
   // dev_dbg(i2c->dev,"[I2C %d] Resume!\n", i2c->id);
    return 0;
}
#else
#define mt_i2c_suspend	NULL
#define mt_i2c_resume	NULL
#endif

static struct platform_driver mt_i2c_driver = {
	.probe	 = mt_i2c_probe,
	.remove	 = mt_i2c_remove,
	.suspend = mt_i2c_suspend,
	.resume	 = mt_i2c_resume,
	.driver  = {
        	.name  = I2C_DRV_NAME,
	        .owner = THIS_MODULE,
	},
};

static int __init mt_i2c_init(void)
{
	return platform_driver_register(&mt_i2c_driver);
}

static void __exit mt_i2c_exit(void)
{
	platform_driver_unregister(&mt_i2c_driver);
}

module_init(mt_i2c_init);
module_exit(mt_i2c_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("MediaTek I2C Bus Driver");
MODULE_AUTHOR("Infinity Chen <infinity.chen@mediatek.com>");
