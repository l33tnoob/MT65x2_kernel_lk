/*
 * Copyright (c) 2012, Code Aurora Forum. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  * Neither the name of Google, Inc. nor the names of its contributors
 *    may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
#include <platform/mt_i2c.h>
#include <platform/mt_gpio.h>

/*-----------------------------------------------------------------------
 * Set I2C Speend interface:    Set internal I2C speed,
 *                              Goal is that get sample_cnt_div and step_cnt_div
 *   clock: Depends on the current MCU/AHB/APB clock frequency
 *   mode:  ST_MODE. (fixed setting for stable I2C transaction)
 *   khz:   MAX_ST_MODE_SPEED. (fixed setting for stable I2C transaction)
 *
 *   Returns: ERROR_CODE
 */
unsigned long mt_i2c_set_speed (unsigned char channel,unsigned long clock, I2C_SPD_MODE mode, unsigned long khz)
{
    unsigned long ret_code = I2C_OK;

    CHANNEL_BASE(channel);
    unsigned short sample_cnt_div, step_cnt_div;
    unsigned short max_step_cnt_div = (mode == HS_MODE) ? MAX_HS_STEP_CNT_DIV : MAX_STEP_CNT_DIV;
    unsigned long tmp, sclk;

    {
        unsigned long diff, min_diff = I2C_CLK_RATE;
        unsigned short sample_div = MAX_SAMPLE_CNT_DIV;
        unsigned short step_div = max_step_cnt_div;
        for (sample_cnt_div = 1; sample_cnt_div <= MAX_SAMPLE_CNT_DIV; sample_cnt_div++) {
            for (step_cnt_div = 1; step_cnt_div <= max_step_cnt_div; step_cnt_div++) {
                sclk = (clock >> 1) / (sample_cnt_div * step_cnt_div);
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
    }

    sclk = clock / (2 * sample_cnt_div * step_cnt_div);
    if (sclk > khz) {
	  ret_code = I2C_SET_SPEED_FAIL_OVER_SPEED;
        return ret_code;
    }

    step_cnt_div--;
    sample_cnt_div--;

    if (mode == HS_MODE) {
        tmp  = i2c_read(MT_I2C_HS) & ((0x7 << 12) | (0x7 << 8));
        tmp  = (sample_cnt_div & 0x7) << 12 | (step_cnt_div & 0x7) << 8 | tmp;
        i2c_write(MT_I2C_HS,tmp);
		//I2CLOG("HS_MODE\n");
        I2C_SET_HS_MODE(1);
    }
    else {
        tmp  = i2c_read(MT_I2C_TIMING) & ~((0x7 << 8) | (0x3f << 0));
        tmp  = (sample_cnt_div & 0x7) << 8 | (step_cnt_div & 0x3f) << 0 | tmp;
        i2c_write(MT_I2C_TIMING,tmp);
		//I2CLOG("not HS_MODE\n");
        I2C_SET_HS_MODE(0);
    }

    I2CLOG("[i2c%d set speed] Set sclk to %ld khz (orig: %ld khz)\n",channel, sclk, khz);
    //I2CLOG("[i2c_set_speed] I2C Timing parameter sample_cnt_div(%d),  step_cnt_div(%d)\n", sample_cnt_div, step_cnt_div);

    return ret_code;
}

static unsigned long mt_i2c_channel_init(unsigned char channel)
{
	unsigned long ret_code = I2C_OK;
    CHANNEL_BASE(channel);


    /* Power On I2C Duel */
    //PDN_Power_CONA_DOWN(PDN_PERI_I2C, KAL_FALSE); // wait PLL API release
    I2CLOG("\n[i2c%d_init] Start...................\n",channel);

    /* Reset the HW I2C module */
    I2C_SOFTRESET;

    /* Set I2C control register */
    I2C_SET_TRANS_CTRL(ACK_ERR_DET_EN | CLK_EXT);

    /* Sset I2C speed mode */
    ret_code = mt_i2c_set_speed(channel,I2C_CLK_RATE, ST_MODE, MAX_ST_MODE_SPEED);
    if( ret_code !=  I2C_OK)
    {
        I2CLOG("[i2c%d_init] mt_i2c_set_speed error (%ld)\n", channel,ret_code);
        return ret_code;
    }

    /* Clear Interrupt status */
    I2C_CLR_INTR_STATUS(I2C_TRANSAC_COMP | I2C_ACKERR | I2C_HS_NACKERR);

    /* Double Reset the I2C START bit*/
    i2c_write(MT_I2C_START,0);

    I2CLOG("[i2c%d_init] Done\n",channel);

    return ret_code;

}

/*-----------------------------------------------------------------------
* Initializa the HW I2C module
*    Returns: ERROR_CODE
*/
unsigned long mt_i2c_init(void)
{
	/*if you are using I2C, first init it at here*/
    return I2C_OK;
}

/*-----------------------------------------------------------------------
* De-Initializa the HW I2C module
*    Returns: ERROR_CODE
*/
unsigned long mt_i2c_deinit (unsigned char channel)
{
    unsigned long ret_code = I2C_OK;
 	CHANNEL_BASE(channel);

    /* Reset the HW I2C module */
    I2C_SOFTRESET;

    I2CLOG("[i2c_deinit] Done\n");

    return ret_code;
}

//I2C GPIO debug
struct mt_i2c_gpio_t{
	unsigned short scl;
	unsigned short sda;
};
static struct mt_i2c_gpio_t mt_i2c_gpio_mode[I2C_NR]={
	{GPIO105,GPIO106},
	{GPIO113,GPIO114},
};

static inline void mt_i2c_dump_info(struct mt_i2c_t *i2c)
{
	CHANNEL_BASE(i2c->id);
	I2CERR("I2C register:\nSLAVE_ADDR %x\nINTR_MASK %x\nINTR_STAT %x\nCONTROL %x\nTRANSFER_LEN %x\nTRANSAC_LEN %x\nDELAY_LEN %x\nTIMING %x\nSTART %x\nFIFO_STAT %x\nIO_CONFIG %x\nHS %x\nDEBUGSTAT %x\nEXT_CONF %x\nTRANSFER_LEN_AUX %x\nTIMEOUT %x\n",
			(i2c_read(  MT_I2C_SLAVE_ADDR)),
			(i2c_read(  MT_I2C_INTR_MASK)),
			(i2c_read(  MT_I2C_INTR_STAT)),
			(i2c_read(  MT_I2C_CONTROL)),
			(i2c_read(  MT_I2C_TRANSFER_LEN)),
			(i2c_read(  MT_I2C_TRANSAC_LEN)),
			(i2c_read(  MT_I2C_DELAY_LEN)),
			(i2c_read(  MT_I2C_TIMING)),
			(i2c_read(  MT_I2C_START)),
			(i2c_read(  MT_I2C_FIFO_STAT)),
			(i2c_read(  MT_I2C_IO_CONFIG)),
			(i2c_read(  MT_I2C_HS)),
			(i2c_read(  MT_I2C_DEBUGSTAT)),
			(i2c_read(  MT_I2C_EXT_CONF)),
			(i2c_read(  MT_I2C_TRANSFER_LEN_AUX)),
			(i2c_read(  MT_I2C_TIMEOUT)));
	//I2CERR("DMA register:\nINT_FLAG %x\nCON %x\nTX_MEM_ADDR %x\nRX_MEM_ADDR %x\nTX_LEN %x\nRX_LEN %x\nINT_EN %x\nEN %x\n",(__raw_readl(i2c->pdmabase+OFFSET_INT_FLAG)),(__raw_readl(i2c->pdmabase+OFFSET_CON)),(__raw_readl(i2c->pdmabase+OFFSET_TX_MEM_ADDR)),(__raw_readl(i2c->pdmabase+OFFSET_RX_MEM_ADDR)),(__raw_readl(i2c->pdmabase+OFFSET_TX_LEN)),(__raw_readl(i2c->pdmabase+OFFSET_RX_LEN)),(__raw_readl(i2c->pdmabase+OFFSET_INT_EN)),(__raw_readl(i2c->pdmabase+OFFSET_EN)));

	I2CERR("GPIO%d(SCL):%d,mode%d; GPIO%d(SDA):%d,mode%d\n",
			mt_i2c_gpio_mode[i2c->id].scl,
			mt_get_gpio_in(mt_i2c_gpio_mode[i2c->id].scl),
			mt_get_gpio_mode(mt_i2c_gpio_mode[i2c->id].scl),
			mt_i2c_gpio_mode[i2c->id].sda,
			mt_get_gpio_in(mt_i2c_gpio_mode[i2c->id].sda),
			mt_get_gpio_mode(mt_i2c_gpio_mode[i2c->id].sda));

	I2CERR("base address %x\n",i2c_base);
	return;

}

/*-----------------------------------------------------------------------
 * Read interface: Read bytes
 *   chip:    I2C chip address, range 0..127
 *              e.g. Smart Battery chip number is 0xAA
 *   buffer:  Where to read/write the data (device address is regarded as data)
 *   len:     How many bytes to read/write
 *   dir:  if you use PMIC, please fill it with 1. otherwise dir is 0
 *   Returns: ERROR_CODE
 */
unsigned long mt_i2c_read(unsigned char channel,unsigned char chip, unsigned char *buffer, int len , unsigned char dir)
{
    unsigned long ret_code = I2C_OK;
    unsigned char *ptr = buffer;
    unsigned short status;
    int ret = len;
    //long tmo;
    //unsigned long timeout_ms = I2C_TIMEOUT_TH;
    //unsigned long timeout_ms = 4000; // 4s
    //unsigned long start_tick=0, timeout_tick=0;
	unsigned int time_out_val=0;
	CHANNEL_BASE(channel);

    /* CHECKME. mt65xx doesn't support len = 0. */
    if (!len) {
        I2CLOG("[i2c_read] I2C doesn't support len = 0.\n");
        return I2C_READ_FAIL_ZERO_LENGTH;
    }

#ifndef MACH_FPGA
    ENABLE_CLOCK(channel);
#endif

    I2C_CLR_INTR_STATUS(I2C_TRANSAC_COMP | I2C_ACKERR | I2C_HS_NACKERR);
    /* for read, bit 0 is to indicate read REQ or write REQ */
    chip = (chip | 0x1);

    /* control registers */
    I2C_SET_SLAVE_ADDR(chip);
    I2C_SET_TRANS_LEN(len);
    I2C_SET_TRANSAC_LEN(1);
    I2C_SET_INTR_MASK(I2C_HS_NACKERR | I2C_ACKERR | I2C_TRANSAC_COMP);
    I2C_FIFO_CLR_ADDR;

    I2C_SET_TRANS_CTRL(ACK_ERR_DET_EN | CLK_EXT);

    /* start trnasfer transaction */
    I2C_START_TRANSAC;

    /* set timer to calculate time avoid timeout without any reaction */
    //tmo = get_timer(0);
#if 0
    timeout_tick = gpt4_time2tick_ms(timeout_ms);
    start_tick = gpt4_get_current_tick();
#endif

    /* polling mode : see if transaction complete */
    while (1)
    {
        status = I2C_INTR_STATUS;

        if ( status & I2C_TRANSAC_COMP && (!I2C_FIFO_IS_EMPTY) )
        {
            ret = 0;
            ret_code = I2C_OK; // 0
            break;
        }
        else if ( status & I2C_HS_NACKERR)
        {
            ret = 1;
            ret_code = I2C_READ_FAIL_HS_NACKERR;
            I2CLOG("[i2c_read] transaction NACK error (%x)\n", status);
            break;
        }
        else if ( status & I2C_ACKERR)
        {
            ret = 2;
            ret_code = I2C_READ_FAIL_ACKERR;
            I2CLOG("[i2c_read] transaction ACK error (%x)\n", status);
            break;
        }
#if 1
        else if (time_out_val > 100000) {
            ret = 3;
            ret_code = I2C_READ_FAIL_TIMEOUT;
			I2C_SOFTRESET;
            I2CLOG("[i2c_read] transaction timeout:%d\n", time_out_val);
            break;
        }
		time_out_val++;
#endif
    }

    I2C_CLR_INTR_STATUS(I2C_TRANSAC_COMP | I2C_ACKERR | I2C_HS_NACKERR);

    if (!ret)
    {
        while (len--)
        {
            I2C_READ_BYTE(*ptr);
            I2CLOG("[i2c_read] read byte = 0x%x\n", *ptr);
            ptr++;
        }
    }

    /* clear bit mask */
    I2C_CLR_INTR_MASK(I2C_HS_NACKERR | I2C_ACKERR | I2C_TRANSAC_COMP);
    I2C_SOFTRESET;

#ifndef MACH_FPGA
    DISABLE_CLOCK(channel);
#endif

    I2CLOG("[i2c_read] Done\n");

    return ret_code;
}

/*-----------------------------------------------------------------------
 * Read interface: Write bytes
 *   chip:    I2C chip address, range 0..127
 *              e.g. Smart Battery chip number is 0xAA
 *   buffer:  Where to read/write the data (device address is regarded as data)
 *   len:     How many bytes to read/write
 *
 *   Returns: ERROR_CODE
 */
unsigned long mt_i2c_write (unsigned char channel,unsigned char chip, unsigned char *buffer, int len, unsigned char dir)
{
    unsigned long ret_code = I2C_OK;
    unsigned char *ptr = buffer;
    //long tmo;
    unsigned short status;
    //unsigned long timeout_ms = I2C_TIMEOUT_TH;
    //unsigned long start_tick=0, timeout_tick=0;
	unsigned int time_out_val=0;
    CHANNEL_BASE(channel);
    /* CHECKME. mt65xx doesn't support len = 0. */
    if (!len)
    {
		I2CLOG("[i2c_write] I2C doesn't support len = 0.\n");
        return I2C_WRITE_FAIL_ZERO_LENGTH;
    }

#ifndef MACH_FPGA
    ENABLE_CLOCK(channel);
#endif

    I2C_CLR_INTR_STATUS(I2C_TRANSAC_COMP | I2C_ACKERR | I2C_HS_NACKERR);
    /* bit 0 is to indicate read REQ or write REQ */
    chip = (chip & ~0x1);

    /* control registers */
    I2C_SET_SLAVE_ADDR(chip);
    I2C_SET_TRANS_LEN(len);
    I2C_SET_TRANSAC_LEN(1);
    I2C_SET_INTR_MASK(I2C_HS_NACKERR | I2C_ACKERR | I2C_TRANSAC_COMP);
    I2C_FIFO_CLR_ADDR;

    I2C_SET_TRANS_CTRL(ACK_ERR_DET_EN | CLK_EXT);

    /* start to write data */
    while (len--) {
        I2C_WRITE_BYTE(*ptr);
        //dbg_print("[i2c_write] write byte = 0x%x\n", *ptr);
        ptr++;
    }

    /* start trnasfer transaction */
    I2C_START_TRANSAC;

    /* set timer to calculate time avoid timeout without any reaction */
    //tmo = get_timer(0);
#if 0
    timeout_tick = gpt4_time2tick_ms(timeout_ms);
    start_tick = gpt4_get_current_tick();
#endif

    /* polling mode : see if transaction complete */
    while (1) {
        status = I2C_INTR_STATUS;
		//I2CLOG("new status=%d\n",status);
/*
        if ( status & I2C_TRANSAC_COMP) {
            ret_code = I2C_OK;
			I2CLOG("[i2c_write] i2c transaction complate\n");
            break;
        }
        else */if ( status & I2C_HS_NACKERR) {
            ret_code = I2C_WRITE_FAIL_HS_NACKERR;
            I2CLOG("[i2c%d write] transaction NACK error\n",channel);
            break;
        }
        else if ( status & I2C_ACKERR) {
            ret_code = I2C_WRITE_FAIL_ACKERR;
            I2CLOG( "[i2c%d write] transaction ACK error\n",channel);
            break;
        }
	else if ( status & I2C_TRANSAC_COMP) {
            ret_code = I2C_OK;
			I2CLOG("[i2c%d write] i2c transaction complate\n",channel);
            break;
        }
#if 1
        else if (time_out_val > 100000) {
            ret_code	= I2C_WRITE_FAIL_TIMEOUT;
            I2CLOG("[i2c%d write] transaction timeout:%d\n", channel,time_out_val);
            break;
        }
		time_out_val++;
#endif
    }

    I2C_CLR_INTR_STATUS(I2C_TRANSAC_COMP | I2C_ACKERR | I2C_HS_NACKERR);

    /* clear bit mask */
    I2C_CLR_INTR_MASK(I2C_HS_NACKERR | I2C_ACKERR | I2C_TRANSAC_COMP);
	I2C_SOFTRESET;

#ifndef MACH_FPGA
    DISABLE_CLOCK(channel);
#endif

    return ret_code;
}


/*-----------------------------------------------------------------------
 * new read interface: Read bytes
 *   i2c:    I2C chip config, see struct mt_i2c_t.
 *   buffer:  Where to read/write the data.
 *   len:     How many bytes to read/write
 *   Returns: ERROR_CODE
 */
unsigned long mt_i2c_read_new(struct mt_i2c_t *i2c,unsigned char *buffer, int len)
{
    unsigned long ret_code = I2C_OK;
	unsigned long i2c_clk;
    unsigned char *ptr = buffer;
    unsigned short status;
    int ret = len;
	unsigned int time_out_val=0;
	CHANNEL_BASE(i2c->id);

    /* CHECKME. mt65xx doesn't support len = 0. */
    if ((len == 0) || (len > 255)) {
        I2CLOG("[i2c_read] I2C doesn't support len = %d.\n",len);
        return I2C_READ_FAIL_ZERO_LENGTH;
    }

    /* setting clock */
    i2c_clk = I2C_CLK_RATE;

#ifndef MACH_FPGA
    ENABLE_CLOCK(i2c->id);
#endif

    I2C_CLR_INTR_STATUS(I2C_TRANSAC_COMP | I2C_ACKERR | I2C_HS_NACKERR);
	//setting speed
	mt_i2c_set_speed(i2c->id, i2c_clk, i2c->mode, i2c->speed);
	if(i2c->speed <= 100){
		i2c_write(MT_I2C_EXT_CONF, 0x8001);
	}

    /* control registers */
    I2C_SET_SLAVE_ADDR(((i2c->addr<<1) | 0x1));
    I2C_SET_TRANS_LEN(len);
    I2C_SET_TRANSAC_LEN(1);
    I2C_SET_INTR_MASK(I2C_HS_NACKERR | I2C_ACKERR | I2C_TRANSAC_COMP);
    I2C_FIFO_CLR_ADDR;
    I2C_SET_TRANS_CTRL(ACK_ERR_DET_EN | (i2c->is_clk_ext_disable?0:CLK_EXT) |(i2c->is_rs_enable?REPEATED_START_FLAG:0));
    //I2C_SET_TRANS_CTRL(ACK_ERR_DET_EN | CLK_EXT);

    /* start trnasfer transaction */
    I2C_START_TRANSAC;

    /* polling mode : see if transaction complete */
    while (1)
    {
        status = I2C_INTR_STATUS;

        if ( status & I2C_TRANSAC_COMP && (!I2C_FIFO_IS_EMPTY) )
        {
            ret = 0;
            ret_code = I2C_OK; // 0
            break;
        }else if ( status & I2C_HS_NACKERR)
        {
            ret = 1;
            ret_code = I2C_READ_FAIL_HS_NACKERR;
            I2CERR("[i2c%d read] transaction NACK error (%x)\n",i2c->id, status);
			mt_i2c_dump_info(i2c);
            break;
        }else if ( status & I2C_ACKERR)
        {
            ret = 2;
            ret_code = I2C_READ_FAIL_ACKERR;
            I2CERR("[i2c%d read] transaction ACK error (%x)\n",i2c->id, status);
			mt_i2c_dump_info(i2c);
            break;
        }else if (time_out_val > 100000) {
            ret = 3;
            ret_code = I2C_READ_FAIL_TIMEOUT;
			I2C_SOFTRESET;
            I2CERR("[i2c%d read] transaction timeout:%d\n",i2c->id, time_out_val);
			mt_i2c_dump_info(i2c);
            break;
        }
		time_out_val++;
    }

    I2C_CLR_INTR_STATUS(I2C_TRANSAC_COMP | I2C_ACKERR | I2C_HS_NACKERR);

    if (!ret)
    {
        while (len--)
        {
            I2C_READ_BYTE(*ptr);
            I2CLOG("[i2c_read] read byte = 0x%x\n", *ptr);
            ptr++;
        }
    }

    /* clear bit mask */
    I2C_CLR_INTR_MASK(I2C_HS_NACKERR | I2C_ACKERR | I2C_TRANSAC_COMP);
	I2C_SOFTRESET;

#ifndef MACH_FPGA
    DISABLE_CLOCK(i2c->id);
#endif

    I2CLOG("[i2c_read] Done\n");

    return ret_code;
}

/*-----------------------------------------------------------------------
 * New write interface: Write bytes
 *   i2c:    I2C chip config, see struct mt_i2c_t.
 *   buffer:  Where to read/write the data.
 *   len:     How many bytes to read/write
 *   Returns: ERROR_CODE
 */
unsigned long mt_i2c_write_new(struct mt_i2c_t *i2c,unsigned char *buffer, int len)
{
    unsigned long ret_code = I2C_OK;
    unsigned long i2c_clk;
    unsigned char *ptr = buffer;
    unsigned short status;
    unsigned int time_out_val=0;

    CHANNEL_BASE(i2c->id);

    /* CHECKME. mt65xx doesn't support len = 0. */
    if ((len == 0) || (len > 255)) {
		I2CLOG("[i2c_write] I2C doesn't support len = %d.\n",len);
        return I2C_WRITE_FAIL_ZERO_LENGTH;
    }

    /* setting clock */
    i2c_clk = I2C_CLK_RATE;

#ifndef MACH_FPGA
    ENABLE_CLOCK(i2c->id);
#endif

    I2C_CLR_INTR_STATUS(I2C_TRANSAC_COMP | I2C_ACKERR | I2C_HS_NACKERR);
	//setting speed
	mt_i2c_set_speed(i2c->id, i2c_clk, i2c->mode, i2c->speed);
	if(i2c->speed <= 100){
		i2c_write(MT_I2C_EXT_CONF, 0x8001);
	}
    /* control registers */
    I2C_SET_SLAVE_ADDR(i2c->addr<<1);
    I2C_SET_TRANS_LEN(len);
    I2C_SET_TRANSAC_LEN(1);
    I2C_SET_INTR_MASK(I2C_HS_NACKERR | I2C_ACKERR | I2C_TRANSAC_COMP);
    I2C_FIFO_CLR_ADDR;
    I2C_SET_TRANS_CTRL(ACK_ERR_DET_EN | (i2c->is_clk_ext_disable?0:CLK_EXT) |(i2c->is_rs_enable?REPEATED_START_FLAG:0));
    //I2C_SET_TRANS_CTRL(ACK_ERR_DET_EN | CLK_EXT);

    /* start to write data */
    while (len--) {
        I2C_WRITE_BYTE(*ptr);
        //dbg_print("[i2c_write] write byte = 0x%x\n", *ptr);
        ptr++;
    }

	//mt_i2c_dump_info(i2c);
    /* start trnasfer transaction */
    I2C_START_TRANSAC;

    /* polling mode : see if transaction complete */
    while (1) {
        status = I2C_INTR_STATUS;
        if ( status & I2C_HS_NACKERR) {
            ret_code = I2C_WRITE_FAIL_HS_NACKERR;
            I2CERR("[i2c%d write] transaction NACK error\n",i2c->id);
			mt_i2c_dump_info(i2c);
            break;
        }else if ( status & I2C_ACKERR) {
            ret_code = I2C_WRITE_FAIL_ACKERR;
            I2CERR( "[i2c%d write] transaction ACK error\n",i2c->id);
			mt_i2c_dump_info(i2c);
            break;
        }else if ( status & I2C_TRANSAC_COMP) {
            ret_code = I2C_OK;
			I2CLOG("[i2c%d write] i2c transaction complate\n",i2c->id);
            break;
        }else if (time_out_val > 100000) {
            ret_code	= I2C_WRITE_FAIL_TIMEOUT;
            I2CERR("[i2c%d write] transaction timeout:%d\n", i2c->id,time_out_val);
			mt_i2c_dump_info(i2c);
            break;
        }
		time_out_val++;
    }

    I2C_CLR_INTR_STATUS(I2C_TRANSAC_COMP | I2C_ACKERR | I2C_HS_NACKERR);

    /* clear bit mask */
    I2C_CLR_INTR_MASK(I2C_HS_NACKERR | I2C_ACKERR | I2C_TRANSAC_COMP);
	I2C_SOFTRESET;

#ifndef MACH_FPGA
    DISABLE_CLOCK(i2c->id);
#endif

    return ret_code;
}

/*-----------------------------------------------------------------------
 * New write then read back interface: Write bytes then read bytes
 *   i2c:    I2C chip config, see struct mt_i2c_t.
 *   buffer:  Where to read/write the data.
 *   write_len:     How many bytes to write
 *   read_len:     How many bytes to read
 *   Returns: ERROR_CODE
 */
unsigned long mt_i2c_write_read_new(struct mt_i2c_t *i2c,unsigned char *buffer, int write_len, int read_len)
{
    unsigned long ret_code = I2C_OK;
	unsigned long i2c_clk;
    unsigned char *ptr = buffer;
    unsigned short status;
	unsigned int time_out_val=0;

    CHANNEL_BASE(i2c->id);

    /* CHECKME. mt65xx doesn't support len = 0. */
    if ((write_len==0) || (read_len == 0) || (write_len > 255) || (read_len > 255))
    {
		I2CLOG("[i2c_write_read] I2C doesn't support w,r len = %d,%d.\n",write_len, read_len);
        return I2C_WRITE_FAIL_ZERO_LENGTH;
    }

    /* setting clock */
    i2c_clk = I2C_CLK_RATE;

#ifndef MACH_FPGA
    ENABLE_CLOCK(i2c->id);
#endif

    I2C_CLR_INTR_STATUS(I2C_TRANSAC_COMP | I2C_ACKERR | I2C_HS_NACKERR);
	//setting speed
	mt_i2c_set_speed(i2c->id, i2c_clk, i2c->mode, i2c->speed);
	if (i2c->speed <= 100)
		i2c_write(MT_I2C_EXT_CONF, 0x8001);

    /* control registers */
    I2C_SET_SLAVE_ADDR(i2c->addr << 1);
    I2C_SET_TRANS_LEN(write_len);
    I2C_SET_TRANS_AUX_LEN(read_len);
    I2C_SET_TRANSAC_LEN(2);
    I2C_SET_INTR_MASK(I2C_HS_NACKERR | I2C_ACKERR | I2C_TRANSAC_COMP);
    I2C_FIFO_CLR_ADDR;

    I2C_SET_TRANS_CTRL(ACK_ERR_DET_EN | DIR_CHG | (i2c->is_clk_ext_disable?0:CLK_EXT)
            | REPEATED_START_FLAG);

    /* start to write data */
    while (write_len--) {
        I2C_WRITE_BYTE(*ptr);
        //dbg_print("[i2c_write] write byte = 0x%x\n", *ptr);
        ptr++;
    }

    //mt_i2c_dump_info(i2c);
    /* start trnasfer transaction */
    I2C_START_TRANSAC;
    /* polling mode : see if transaction complete */
    while (1) {
        status = I2C_INTR_STATUS;
        if ( status & I2C_HS_NACKERR) {
            ret_code = I2C_WRITE_FAIL_HS_NACKERR;
            I2CERR("[i2c%d write_read] transaction NACK error\n",i2c->id);
			mt_i2c_dump_info(i2c);
            break;
        }else if ( status & I2C_ACKERR) {
            ret_code = I2C_WRITE_FAIL_ACKERR;
            I2CERR( "[i2c%d write_read] transaction ACK error\n",i2c->id);
			mt_i2c_dump_info(i2c);
            break;
		}else if ( status & I2C_TRANSAC_COMP) {
            ret_code = I2C_OK;
			I2CLOG("[i2c%d write_read] i2c transaction complate\n",i2c->id);
            break;
		} else if (time_out_val > 100000) {
            ret_code	= I2C_WRITE_FAIL_TIMEOUT;
            I2CERR("[i2c%d write_read] transaction timeout:%d\n", i2c->id,time_out_val);
			mt_i2c_dump_info(i2c);
            break;
        }
		time_out_val++;
    }

	if(ret_code == I2C_OK){
		ptr = buffer;
		while (read_len--){
			I2C_READ_BYTE(*ptr);
			I2CLOG("[i2c_write_read] read byte = 0x%x\n", *ptr);
			ptr++;
		}
	}
    I2C_CLR_INTR_STATUS(I2C_TRANSAC_COMP | I2C_ACKERR | I2C_HS_NACKERR);

    /* clear bit mask */
    I2C_CLR_INTR_MASK(I2C_HS_NACKERR | I2C_ACKERR | I2C_TRANSAC_COMP);
	I2C_SOFTRESET;

#ifndef MACH_FPGA
    DISABLE_CLOCK(i2c->id);
#endif

    return ret_code;
}

//Test I2C

int mt_i2c_test_eeprom(int id)
{
	int ret = 0;
	unsigned char byte[2];
	struct mt_i2c_t i2c;

	i2c.id = id;
	i2c.addr = 0x50;
	i2c.mode = ST_MODE;
	i2c.speed = 100;


	byte[0] = 0x20;
	byte[1] = 0x55;

	ret = mt_i2c_write_new(&i2c, byte, 2);
	if(0 != ret){
		I2CERR("Write 2 bytes fails(%x).\n",ret);
		ret = -1;
		return ret;
	}else{
		I2CLOG("Write 2 bytes are %x, %x.\n", byte[0], byte[1]);
	}
	ret = 0xfffff;
	while(ret--);
	byte[0] = 0x20;
	byte[1] = 0x00;
	ret = mt_i2c_write_read_new(&i2c, byte, 1, 1);
	if(0 != ret){
		I2CERR("Write 1 byte fails(%x).\n",ret);
		ret = -1;
		return ret;
	}else{
		I2CLOG("Read 1 byte is %x.\n", byte[0]);
	}
	return ret;
}

int mt_i2c_test(void)
{
	int ret;
	int i;
	for (i = 0; i < I2C_NR; i++) {
		I2CERR("I2C%d,EEPROM test start\n",i);
		ret = mt_i2c_test_eeprom(i);
		I2CERR("I2C%d,EEPROM test ret = %d\n\n",i,ret);
	}
	return 0;
}
