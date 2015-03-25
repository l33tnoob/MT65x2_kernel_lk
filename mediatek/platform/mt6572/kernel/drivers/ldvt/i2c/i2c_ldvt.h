/* TODO: move these settings (except test case) to mach/include/mt_i2c.h */

/* Register offset definition */
enum I2C_REGS_OFFSET {
	OFFSET_DATA_PORT	= 0x0,
	OFFSET_SLAVE_ADDR	= 0x04,
	OFFSET_INTR_MASK	= 0x08,
	OFFSET_INTR_STAT	= 0x0C,
	OFFSET_CONTROL		= 0x10,
	OFFSET_TRANSFER_LEN	= 0x14,
	OFFSET_TRANSAC_LEN	= 0x18,
	OFFSET_DELAY_LEN	= 0x1C,
	OFFSET_TIMING		= 0x20,
	OFFSET_START		= 0x24,
	OFFSET_EXT_CONF		= 0x28,
	OFFSET_FIFO_STAT	= 0x30,
	OFFSET_FIFO_THRESH	= 0x34,
	OFFSET_FIFO_ADDR_CLR	= 0x38,
	OFFSET_IO_CONFIG	= 0x40,
	OFFSET_MULTIMAS		= 0x44,
	OFFSET_HS		= 0x48,
	OFFSET_SOFTRESET	= 0x50,
	OFFSET_PATH_DIR		= 0x60, /* only for 6589 */
	OFFSET_DEBUGSTAT	= 0x64,
	OFFSET_DEBUGCTRL	= 0x68,
/* For 6572
	OFFSET_TRANSFER_LEN_AUX	= 0x6C,
	OFFSET_TIMEOUT		= 0x74,
*/
};

enum DMA_REGS_OFFSET {
	OFFSET_INT_FLAG		= 0x0,
	OFFSET_INT_EN		= 0x04,
	OFFSET_EN 		= 0x08,
	OFFSET_CON 		= 0x18,
	OFFSET_TX_MEM_ADDR 	= 0x1C,
	OFFSET_RX_MEM_ADDR 	= 0x20,
	OFFSET_TX_LEN 		= 0x24,
	OFFSET_RX_LEN 		= 0x28,
};

enum i2c_trans_st_rs {
	I2C_TRANS_STOP = 0,
	I2C_TRANS_REPEATED_START,
};

enum {
	ST_MODE,
	FS_MODE,
	HS_MODE,
};

enum mt_trans_op {
	I2C_MASTER_NONE = 0,
	I2C_MASTER_WR,
	I2C_MASTER_RD,
	I2C_MASTER_WRRD,
};

/* interrupt offset */
#define I2C_TIMEOUT		(0x1 << 4)
#define I2C_HS_NACKERR		(0x1 << 2)
#define I2C_ACKERR		(0x1 << 1)
#define I2C_TRANSAC_COMP	(0x1 << 0)

/* control offset */
#define I2C_CONTROL_WRAPPER	(0x1 << 0)
#define I2C_CONTROL_RS		(0x1 << 1)
#define I2C_CONTROL_DMA_EN	(0x1 << 2)
#define I2C_CONTROL_CLK_EXT_EN	(0x1 << 3)
#define I2C_CONTROL_DIR_CHANGE	(0x1 << 4)
#define I2C_CONTROL_ACKERR_DET_EN	(0x1 << 5)
#define I2C_CONTROL_TRANSFER_LEN_CHANGE	(0x1 << 6)

/* Parameters */
#define I2C_FIFO_SIZE		8

#define MAX_ST_MODE_SPEED	100  /* khz */
#define MAX_FS_MODE_SPEED	400  /* khz */
#define MAX_HS_MODE_SPEED	3400 /* khz */

#define MAX_DMA_TRANS_SIZE	252 /* Max(255) aligned to 4 bytes = 252 */
#define MAX_DMA_TRANS_NUM	256

#define MAX_SAMPLE_CNT_DIV	8
#define MAX_STEP_CNT_DIV	64
#define MAX_HS_STEP_CNT_DIV	8

#define DMA_ADDRESS_HIGH	(0xC0000000)

/* mt_i2c struct */
struct mt_trans_data {
	u16 trans_num;
	u16 data_size;
	u16 trans_len;
	u16 trans_auxlen;
};

struct mt_i2c {
	struct i2c_adapter	adap;		/* i2c host adapter */
	struct device		*dev;		/* the device object of i2c host adapter */
	u32					base;		/* i2c base addr */
	u16					id;
	u16					irqnr;		/* i2c interrupt number */
	u16					irq_stat;	/* i2c interrupt status */
	spinlock_t			lock;		/* for mt_i2c struct protection */
	wait_queue_head_t	wait;		/* i2c transfer wait queue */

	atomic_t			trans_err;	/* i2c transfer error */
	atomic_t			trans_comp;	/* i2c transfer completion */
	atomic_t			trans_stop;	/* i2c transfer stop */

	unsigned long		clk;		/* host clock speed in khz */
	unsigned long		sclk;		/* khz */
	unsigned long		pdn;		/*clock number*/

	unsigned char		master_code;/* master code in HS mode */
	unsigned char		mode;		/* ST/FS/HS mode */

	enum  i2c_trans_st_rs st_rs;
	bool                dma_en;
	u32                 pdmabase;
	u16                 delay_len;
	enum mt_trans_op op;
	bool                poll_en;
	struct mt_trans_data trans_data;
};

/* TODO: for future use...
static inline void i2c_writew(struct mt_i2c * i2c, u8 offset, u16 value)
{

	//dev_err(i2c->dev, "before i2c_writew base=%x,offset=%x\n",i2c->base,offset);
	__raw_writew(value, (i2c->base) + (offset));
}

static inline u16 i2c_readw(struct mt_i2c * i2c, u8 offset)
{
	return __raw_readw((i2c->base) + (offset));
}
*/

/* test cases */
#define I2C_UVVF_HS_FIFO_1P5MHZ		0x0731
#define I2C_UVVF_HS_FIFO_2MHZ		0x0732
#define I2C_UVVF_FS_DMA_RS_DIR_200KHZ	0x0733
#define I2C_UVVF_FS_DMA_RS_DIR_400KHZ	0x0734
#define I2C_UVVF_CLK_EXT_INT		0x0735
#define I2C_UVVF_CLK_REPEAT_RW		0x0736
#define I2C_UVVF_CLK_EXT		0x0737
#define I2C_UVVF_CLK_REPEAT_EXT		0x0738
