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

#ifndef __MT_I2C_H__
#define __MT_I2C_H__

#include "platform.h"
//==============================================================================
// I2C Configuration
//==============================================================================

#define I2C_NR		2

#if (CFG_FPGA_PLATFORM)
#define FPGA_CLOCK                          12000 /* FPGA crystal frequency(KHz) */
#define I2C_CLK_DIV                         5 /* frequency divider */
#define I2C_CLK_RATE                        (FPGA_CLOCK / I2C_CLK_DIV) /* I2C base clock(KHz) */
#else
#define I2C_CLK_DIV                         10 /* frequency divider */
#define I2C_CLK_RATE                        (133250 / I2C_CLK_DIV) /* I2C base clock(KHz) */
#endif

#define I2C_FIFO_SIZE                       8

#define MAX_ST_MODE_SPEED                   100     /* khz */
#define MAX_FS_MODE_SPEED                   400     /* khz */
#define MAX_HS_MODE_SPEED                   3400    /* khz */

#define MAX_DMA_TRANS_SIZE                  252     /* Max(255) aligned to 4 bytes = 252 */
#define MAX_DMA_TRANS_NUM                   256

#define MAX_SAMPLE_CNT_DIV                  8
#define MAX_STEP_CNT_DIV                    64
#define MAX_HS_STEP_CNT_DIV                 8

#define I2C_TIMEOUT_TH                      200     // i2c wait for response timeout value, 200ms


#define I2CTAG                "[I2C][PL] "
#define I2CLOG(fmt, arg...)   print(I2CTAG fmt, ##arg)
#define I2CMSG(fmt, arg...)   print(fmt, ##arg)
#define I2CERR(fmt, arg...)   print(I2CTAG fmt, ##arg)

#define i2c_write(addr,val) __raw_writel(val,addr)
#define i2c_read(addr) __raw_readl(addr)
enum{
	I2C0 = 0,
	I2C1 = 1,
};

#define CHANNEL_BASE(channel) \
	unsigned int i2c_base;\
	if(channel == I2C0) \
		i2c_base = I2C0_BASE;\
	else \
		i2c_base = I2C1_BASE;

typedef enum {
    ST_MODE,
    FS_MODE,
    HS_MODE,
} I2C_SPD_MODE;

struct mt_i2c_t {
    unsigned char id;          // select which one i2c controller
    unsigned char dir;         // Transaction direction 1:PMIC or 0:AP
    unsigned char addr;    	   //The address of the slave device, 7bit
    unsigned char mode;        //i2c mode, stand mode or High speed mode
    unsigned long speed;	   //The speed (Kb)
    unsigned char is_rs_enable;   //repeat start enable or stop condition

	//reserved funtion
    unsigned char is_push_pull_enable;   //IO push-pull or open-drain
    unsigned char is_clk_ext_disable;   //clk entend default enable
    unsigned char delay_len;        //number of half pulse between transfers in a trasaction
    unsigned char is_dma_enabled;   //Transaction via DMA instead of 8-byte FIFO
};
/*
typedef struct
{
	//volatile I2C_STATE  state;
	unsigned char	 owner;
	unsigned char	number_of_read;
	unsigned char*	read_buffer;
	unsigned char	is_DMA_enabled;
}i2c_status_struct;
*/
//==============================================================================
// I2C Register
//==============================================================================
#define MT_I2C_DATA_PORT                       ((i2c_base) + 0x0000)
#define MT_I2C_SLAVE_ADDR                      ((i2c_base) + 0x0004)
#define MT_I2C_INTR_MASK                       ((i2c_base) + 0x0008)
#define MT_I2C_INTR_STAT                       ((i2c_base) + 0x000C)
#define MT_I2C_CONTROL                         ((i2c_base) + 0x0010)
#define MT_I2C_TRANSFER_LEN                    ((i2c_base) + 0x0014)
#define MT_I2C_TRANSAC_LEN                     ((i2c_base) + 0x0018)
#define MT_I2C_DELAY_LEN                       ((i2c_base) + 0x001C)
#define MT_I2C_TIMING                          ((i2c_base) + 0x0020)
#define MT_I2C_START                           ((i2c_base) + 0x0024)
#define MT_I2C_EXT_CONF                        ((i2c_base) + 0x0028)
#define MT_I2C_FIFO_STAT                       ((i2c_base) + 0x0030)
#define MT_I2C_FIFO_THRESH                     ((i2c_base) + 0x0034)
#define MT_I2C_FIFO_ADDR_CLR                   ((i2c_base) + 0x0038)
#define MT_I2C_IO_CONFIG                       ((i2c_base) + 0x0040)
#define MT_I2C_MULTIMAS                        ((i2c_base) + 0x0044)
#define MT_I2C_HS                              ((i2c_base) + 0x0048)
#define MT_I2C_SOFTRESET                       ((i2c_base) + 0x0050)
#define MT_I2C_DEBUGSTAT                       ((i2c_base) + 0x0064)
#define MT_I2C_DEBUGCTRL                       ((i2c_base) + 0x0068)
#define MT_I2C_TRANSFER_LEN_AUX                ((i2c_base) + 0x006C)
#define MT_I2C_TIMEOUT                         ((i2c_base) + 0x0074)

#define I2C_TRANS_LEN_MASK                  (0xffff)
#define I2C_TRANS_AUX_LEN_MASK              (0xffff)
#define I2C_CONTROL_MASK                    (0xff << 1)

//----------- Register mask -------------------//
#define I2C_3_BIT_MASK                      0x07
#define I2C_4_BIT_MASK                      0x0f
#define I2C_8_BIT_MASK                      0xff
#define I2C_6_BIT_MASK                      0x3f
#define I2C_MASTER_READ                     0x01
#define I2C_MASTER_WRITE                    0x00
#define I2C_FIFO_THRESH_MASK                0x07
#define I2C_CTL_RS_STOP_BIT                 0x02
#define I2C_CTL_DMA_EN_BIT                  0x04
#define I2C_CTL_ACK_ERR_DET_BIT             0x20 
#define I2C_CTL_CLK_EXT_EN_BIT              0x08
#define I2C_CTL_DIR_CHANGE_BIT              0x10
#define I2C_CTL_TRANSFER_LEN_CHG_BIT        0x40
#define I2C_DATA_READ_ADJ_BIT               0x8000
#define I2C_SDA_MODE_BIT                    0x02
#define I2C_SCL_MODE_BIT                    0x01
#define I2C_ARBITRATION_BIT                 0x01
#define I2C_CLOCK_SYNC_BIT                  0x02
#define I2C_BUS_DETECT_EN_BIT               0x04
#define I2C_HS_EN_BIT                       0x01
#define I2C_HS_NACK_ERR_DET_EN_BIT          0x02
#define I2C_BUS_BUSY_DET_BIT                0x04
#define I2C_HS_MASTER_CODE_MASK             0x70
#define I2C_HS_STEP_CNT_DIV_MASK            0x700
#define I2C_HS_SAMPLE_CNT_DIV_MASK          0x7000
#define I2C_FIFO_FULL_STATUS                0x01
#define I2C_FIFO_EMPTY_STATUS               0x02

/* I2C interrupt bit */
#define I2C_TIMEOUT                         (1 << 4)
#define I2C_ARB_LOST                        (1 << 3)
#define I2C_HS_NACKERR                      (1 << 2)
#define I2C_ACKERR                          (1 << 1)
#define I2C_TRANSAC_COMP                    (1 << 0)

#define I2C_TX_THR_OFFSET                   8
#define I2C_RX_THR_OFFSET                   0

/* I2C control bits */
#define TIMEOUT_EN                          (1 << 8)
#define RESET_BUS_BUSY_EN                   (1 << 7)
#define TRANS_LEN_CHG                       (1 << 6)
#define ACK_ERR_DET_EN                      (1 << 5)
#define DIR_CHG                             (1 << 4)
#define CLK_EXT                             (1 << 3)
#define DMA_EN                              (1 << 2)
#define REPEATED_START_FLAG                 (1 << 1)

//------------------------------------- Register Settings ---------------------------------------//
#define I2C_START_TRANSAC                   i2c_write(MT_I2C_START,0x1)
#define I2C_FIFO_CLR_ADDR                   i2c_write(MT_I2C_FIFO_ADDR_CLR,0x1)
#define I2C_FIFO_OFFSET                     (i2c_read(MT_I2C_FIFO_STAT)>>4&0xf)
#define I2C_FIFO_IS_EMPTY                   (i2c_read(MT_I2C_FIFO_STAT)>>0&0x1)

#define I2C_SOFTRESET                       i2c_write(MT_I2C_SOFTRESET,0x1)

#define I2C_INTR_STATUS                     i2c_read(MT_I2C_INTR_STAT)

#define I2C_SET_BITS(BS,REG)                ((*(volatile unsigned long*)(REG)) |= (unsigned long)(BS))
#define I2C_CLR_BITS(BS,REG)                ((*(volatile unsigned long*)(REG)) &= ~((unsigned long)(BS)))

#define I2C_SET_FIFO_THRESH(tx,rx) \
    do { unsigned long tmp = (((tx) & 0x7) << I2C_TX_THR_OFFSET) | \
                   (((rx) & 0x7) << I2C_RX_THR_OFFSET); \
         i2c_write(MT_I2C_FIFO_THRESH,tmp); \
    } while(0)

#define I2C_SET_INTR_MASK(mask)             i2c_write(MT_I2C_INTR_MASK,mask)

#define I2C_CLR_INTR_MASK(mask)\
    do { unsigned long tmp = i2c_read(MT_I2C_INTR_MASK); \
         tmp &= ~(mask); \
         i2c_write(MT_I2C_INTR_MASK,tmp); \
    } while(0)

#define I2C_SET_SLAVE_ADDR(addr)            i2c_write(MT_I2C_SLAVE_ADDR,addr)

#define I2C_SET_TRANS_LEN(len)				i2c_write(MT_I2C_TRANSFER_LEN, len)
#define I2C_SET_TRANS_AUX_LEN(len)			i2c_write(MT_I2C_TRANSFER_LEN_AUX, len)

#define I2C_SET_TRANSAC_LEN(len)            i2c_write(MT_I2C_TRANSAC_LEN,len)
#define I2C_SET_TRANS_DELAY(delay)          i2c_write(MT_I2C_DELAY_LEN,delay)

#define I2C_SET_TRANS_CTRL(ctrl)\
    do { unsigned long tmp = i2c_read(MT_I2C_CONTROL) & ~I2C_CONTROL_MASK; \
        tmp |= ((ctrl) & I2C_CONTROL_MASK); \
        i2c_write(MT_I2C_CONTROL,tmp); \
    } while(0)

#define I2C_SET_HS_MODE(on_off) \
    do { unsigned long tmp = i2c_read(MT_I2C_HS) & ~0x1; \
    tmp |= (on_off & 0x1); \
    i2c_write(MT_I2C_HS,tmp); \
    } while(0)

#define I2C_READ_BYTE(byte)     \
    do { byte = i2c_read(MT_I2C_DATA_PORT); } while(0)

#define I2C_WRITE_BYTE(byte) \
    do { i2c_write(MT_I2C_DATA_PORT,byte); } while(0)

#define I2C_CLR_INTR_STATUS(status) \
    do { i2c_write(MT_I2C_INTR_STAT,status); } while(0)



//==============================================================================
// I2C Status Code
//==============================================================================
#define I2C_OK                              0x0000
#define I2C_SET_SPEED_FAIL_OVER_SPEED       0xA001
#define I2C_READ_FAIL_ZERO_LENGTH           0xA002
#define I2C_READ_FAIL_HS_NACKERR            0xA003
#define I2C_READ_FAIL_ACKERR                0xA004
#define I2C_READ_FAIL_TIMEOUT               0xA005
#define I2C_WRITE_FAIL_ZERO_LENGTH          0xA012
#define I2C_WRITE_FAIL_HS_NACKERR           0xA013
#define I2C_WRITE_FAIL_ACKERR               0xA014
#define I2C_WRITE_FAIL_TIMEOUT              0xA015

//==============================================================================
// I2C Exported Function
//==============================================================================
extern unsigned long mt_i2c_init(void);
extern unsigned long mt_i2c_deinit (unsigned char);
extern unsigned long mt_i2c_set_speed (unsigned char,unsigned long clock, I2C_SPD_MODE mode, unsigned long khz);
extern unsigned long mt_i2c_read(unsigned char,unsigned char chip, unsigned char *buffer, int len, unsigned char dir);
extern unsigned long mt_i2c_write (unsigned char,unsigned char chip, unsigned char *buffer, int len, unsigned char dir);

extern unsigned long i2c_v1_init (void);
extern unsigned long i2c_v1_deinit (void);
extern unsigned long i2c_v1_set_speed (unsigned long clock, I2C_SPD_MODE mode, unsigned long khz);
extern unsigned long i2c_v1_read(unsigned char chip, unsigned char *buffer, int len);
extern unsigned long i2c_v1_write (unsigned char chip, unsigned char *buffer, int len);

/*-----------------------------------------------------------------------
 * new read interface: Read bytes
 *   i2c:    I2C chip config, see struct mt_i2c_t.
 *   buffer:  Where to read/write the data.
 *   len:     How many bytes to read/write
 *   Returns: ERROR_CODE
 */
extern unsigned long mt_i2c_read_new(struct mt_i2c_t *i2c,unsigned char *buffer, int len);

/*-----------------------------------------------------------------------
 * New write interface: Write bytes
 *   i2c:    I2C chip config, see struct mt_i2c_t.
 *   buffer:  Where to read/write the data.
 *   len:     How many bytes to read/write
 *   Returns: ERROR_CODE
 */
extern unsigned long mt_i2c_write_new(struct mt_i2c_t *i2c,unsigned char *buffer, int len);

/*-----------------------------------------------------------------------
 * New write then read back interface: Write bytes then read bytes
 *   i2c:    I2C chip config, see struct mt_i2c_t.
 *   buffer:  Where to read/write the data.
 *   write_len:     How many bytes to write
 *   read_len:     How many bytes to read
 *   Returns: ERROR_CODE
 */
extern unsigned long mt_i2c_write_read_new(struct mt_i2c_t *i2c,unsigned char *buffer, int write_len, int read_len);
int mt_i2c_test(void);
#endif /* __MT_I2C_H__ */
