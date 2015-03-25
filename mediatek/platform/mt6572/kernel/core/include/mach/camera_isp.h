#ifndef _MT_ISP_H
#define _MT_ISP_H

#include <linux/ioctl.h>

/*******************************************************************************
*
********************************************************************************/
#define ISP_DEV_MAJOR_NUMBER    251
#define ISP_MAGIC               'k'
/*******************************************************************************
*
********************************************************************************/
#define ISP_INT_IMGO_DROP       ((unsigned int)1 << 19)
#define ISP_INT_IMGO_OVERR     	((unsigned int)1 << 17)
#define ISP_INT_IMGO_ERR        ((unsigned int)1 << 16)
#define ISP_INT_PASS1_TG1_DON   ((unsigned int)1 << 10)
#define ISP_INT_TG1_SOF   		((unsigned int)1 << 7)
#define ISP_INT_TG1_DROP   		((unsigned int)1 << 6)
#define ISP_INT_TG1_GBERR   	((unsigned int)1 << 5)
#define ISP_INT_TG1_ERR   		((unsigned int)1 << 4)
#define ISP_INT_EXPDON1   		((unsigned int)1 << 3)
#define ISP_INT_TG1_INT2   		((unsigned int)1 << 2)
#define ISP_INT_TG1_INT1   		((unsigned int)1 << 1)
#define ISP_INT_VS1   			((unsigned int)1 << 0)
/*******************************************************************************
*
********************************************************************************/
#define ISP_RT_BUF_TBL_NPAGES 	16		// length of the two memory areas
#define ISP_RT_BUF_SIZE 		16		// length of the two memory areas

typedef enum
{
    ISP_RT_BUF_EMPTY, // 0
    ISP_RT_BUF_FILLED,// 1
    ISP_RT_BUF_LOCKED,// 2
}ISP_RT_BUF_STATE_ENUM;

typedef struct {
    unsigned int   memID;
    unsigned int   size;
    unsigned int   base_vAddr;
    unsigned int   base_pAddr;        
    unsigned int   timeStampS;
    unsigned int   timeStampUs;
    unsigned int   bFilled;
}ISP_RT_BUF_STRUCT;

typedef struct	{
	unsigned int		   count;
	ISP_RT_BUF_STRUCT  data[ISP_RT_BUF_SIZE];	
}ISP_RT_DEQUE_BUF_STRUCT;

typedef struct  {
    unsigned int           start;          //current DMA accessing buffer
    unsigned int           read;           //current trying to be deque
    unsigned int           total_count;    //total buffer number.Include Filled and empty
    unsigned int           empty_count;    //total empty buffer number include current DMA accessing buffer
    unsigned int           pre_empty_count;//previous total empty buffer number include current DMA accessing buffer    
    unsigned int           active;
    ISP_RT_BUF_STRUCT  data[ISP_RT_BUF_SIZE];   
}ISP_RT_RING_BUF_STRUCT;
/*******************************************************************************
*
********************************************************************************/
typedef enum
{
	MCLK_USING_UNIV_48M = 1,
	MCLK_USING_UNIV_208M = 2
}ISP_PLL_ENUM;

typedef struct
{
	unsigned long MclkSel;	
	unsigned long backup; 	
}ISP_PLL_SEL_STRUCT;

typedef struct
{
	unsigned long Pin;	
	unsigned long Mode;	 	
}ISP_GPIO_SEL_STRUCT;

typedef struct
{
    unsigned int Addr;   // register's addr
    unsigned int Val;    // register's value
}ISP_REG_STRUCT;

typedef struct
{
    unsigned int Data;   // pointer to ISP_REG_STRUCT
    unsigned int Count;  // count
}ISP_REG_IO_STRUCT;
//
typedef enum
{
    ISP_HOLD_TIME_VD,
    ISP_HOLD_TIME_EXPDONE
}ISP_HOLD_TIME_ENUM;
//
typedef enum
{
	ISP_IRQ_CLEAR_NONE,
	ISP_IRQ_CLEAR_WAIT,
	ISP_IRQ_CLEAR_ALL
}ISP_IRQ_CLEAR_ENUM;

typedef struct
{
    ISP_IRQ_CLEAR_ENUM  Clear;
    unsigned int       Status;
    unsigned int       Timeout;
}ISP_WAIT_IRQ_STRUCT;
//
typedef enum
{
    ISP_RT_BUF_CTRL_ENQUE,          // 0
    ISP_RT_BUF_CTRL_EXCHANGE_ENQUE, // 1
    ISP_RT_BUF_CTRL_DEQUE,          // 2
    ISP_RT_BUF_CTRL_IS_RDY,         // 3
    ISP_RT_BUF_CTRL_GET_SIZE,       // 4
    ISP_RT_BUF_CTRL_CLEAR,          // 5
    ISP_RT_BUF_CTRL_MAX
}ISP_RT_BUF_CTRL_ENUM;

typedef enum
{
    ISP_RT_BUF_IMGI = 0, 
    ISP_RT_BUF_IMGO	= 1,
    ISP_RT_BUF_DMAMAX
}ISP_RT_BUF_DMA_ENUM;

typedef struct  {
    ISP_RT_BUF_CTRL_ENUM    ctrl;
    ISP_RT_BUF_DMA_ENUM     buf_id;
    unsigned int            data_ptr;
    unsigned int            ex_data_ptr; //exchanged buffer
}ISP_RT_BUF_CTRL_STRUCT;
//
#define _use_kernel_ref_cnt_		// reference count
typedef enum
{
    ISP_REF_CNT_GET,    // 0
    ISP_REF_CNT_INC,    // 1
    ISP_REF_CNT_DEC,    // 2    
    ISP_REF_CNT_DEC_AND_RESET_IF_LAST_ONE,	// 3
    ISP_REF_CNT_MAX
}ISP_REF_CNT_CTRL_ENUM;

typedef enum
{
    ISP_REF_CNT_ID_IMEM,    // 0
    ISP_REF_CNT_ID_ISP_FUNC,// 1
    ISP_REF_CNT_ID_MAX,
}ISP_REF_CNT_ID_ENUM;

typedef struct  {
    ISP_REF_CNT_CTRL_ENUM   ctrl;
    ISP_REF_CNT_ID_ENUM     id;
    unsigned long           data_ptr; 
}ISP_REF_CNT_CTRL_STRUCT;
/*******************************************************************************
*
********************************************************************************/
typedef enum
{
    ISP_RT_BUF_INFO_INIT, // 0
    ISP_RT_BUF_INFO_SOF,
    ISP_RT_BUF_INFO_DONE,
    ISP_RT_BUF_INFO_MAX
}ISP_RT_BUF_INFO_STATE_ENUM;

typedef struct  {
    ISP_RT_BUF_INFO_STATE_ENUM 	state;
    unsigned long 				dropCnt;
    ISP_RT_RING_BUF_STRUCT  	ring_buf; 
}ISP_RT_BUF_INFO_STRUCT;
/*******************************************************************************
*
********************************************************************************/
typedef enum
{
    ISP_CMD_RESET,          //Reset
    ISP_CMD_RESET_BUF,
    ISP_CMD_READ_REG,       //Read register from driver
    ISP_CMD_WRITE_REG,      //Write register to driver
    ISP_CMD_HOLD_TIME,
    ISP_CMD_HOLD_REG,       //Hold reg write to hw, on/off
    ISP_CMD_WAIT_IRQ,       //Wait IRQ
    ISP_CMD_READ_IRQ,       //Read IRQ
    ISP_CMD_CLEAR_IRQ,      //Clear IRQ
    ISP_CMD_DUMP_REG,       //Dump ISP registers , for debug usage
//    ISP_CMD_SET_USER_PID,   //for signal
    ISP_CMD_SET_DEVICE_ID, //for atv and sensor 
    ISP_CMD_PLL_SEL,
    ISP_CMD_GPIO_SEL,
    ISP_CMD_RT_BUF_CTRL,    //for pass buffer control    
    ISP_CMD_REF_CNT,        //get imem reference count
    ISP_CMD_DEBUG_FLAG      //Dump message level 
}ISP_CMD_ENUM;

#define ISP_IOC_RESET           _IO  (ISP_MAGIC, ISP_CMD_RESET)
#define ISP_IOC_RESET_BUF       _IO  (ISP_MAGIC, ISP_CMD_RESET_BUF)
#define ISP_IOC_READ_REG   		_IOWR(ISP_MAGIC, ISP_CMD_READ_REG,      ISP_REG_IO_STRUCT)
#define ISP_IOC_WRITE_REG  		_IOWR(ISP_MAGIC, ISP_CMD_WRITE_REG,     ISP_REG_IO_STRUCT)
#define ISP_IOC_HOLD_REG_TIME   _IOW (ISP_MAGIC, ISP_CMD_HOLD_TIME,     ISP_HOLD_TIME_ENUM)
#define ISP_IOC_HOLD_REG        _IOW (ISP_MAGIC, ISP_CMD_HOLD_REG,      bool)
#define ISP_IOC_WAIT_IRQ        _IOW (ISP_MAGIC, ISP_CMD_WAIT_IRQ,      ISP_WAIT_IRQ_STRUCT)
#define ISP_IOC_READ_IRQ        _IOR (ISP_MAGIC, ISP_CMD_READ_IRQ,      unsigned int)
#define ISP_IOC_CLEAR_IRQ       _IOW (ISP_MAGIC, ISP_CMD_CLEAR_IRQ,     unsigned int)
#define ISP_IOC_SET_DEVICE_ID   _IOW (ISP_MAGIC, ISP_CMD_SET_DEVICE_ID, int)
#define ISP_IOC_DUMP_REG        _IO  (ISP_MAGIC, ISP_CMD_DUMP_REG)
//#define ISP_IOC_SET_USER_PID    _IOW (ISP_MAGIC, ISP_CMD_SET_USER_PID, 	unsigned long)
#define ISP_IOC_PLL_SEL_IRQ   	_IOW (ISP_MAGIC, ISP_CMD_PLL_SEL,     	ISP_PLL_SEL_STRUCT)
#define ISP_IOC_GPIO_SEL_IRQ   	_IOW (ISP_MAGIC, ISP_CMD_GPIO_SEL,     	ISP_GPIO_SEL_STRUCT)
#define ISP_IOC_BUFFER_CTRL     _IOWR(ISP_MAGIC, ISP_CMD_RT_BUF_CTRL,  	ISP_RT_BUF_CTRL_STRUCT)
#define ISP_IOC_REF_CNT_CTRL    _IOWR(ISP_MAGIC, ISP_CMD_REF_CNT,       ISP_REF_CNT_CTRL_STRUCT)
#define ISP_IOC_DEBUG_FLAG      _IOW (ISP_MAGIC, ISP_CMD_DEBUG_FLAG,    unsigned long)
/*******************************************************************************
*
********************************************************************************/
typedef void (*pIspCallback)(void);

typedef enum
{
    //Work queue. It is interruptible, so there can be "Sleep" in work queue function.
    ISP_CALLBACK_WORKQUEUE_VD,
    ISP_CALLBACK_WORKQUEUE_EXPDONE,
    //Tasklet. It is uninterrupted, so there can NOT be "Sleep" in tasklet function.
    ISP_CALLBACK_TASKLET_VD,
    ISP_CALLBACK_TASKLET_EXPDONE,
    ISP_CALLBACK_AMOUNT
}ISP_CALLBACK_ENUM;

typedef struct
{
    ISP_CALLBACK_ENUM   Type;
    pIspCallback        Func;
}ISP_CALLBACK_STRUCT;

bool ISP_RegCallback(ISP_CALLBACK_STRUCT* pCallback);
bool ISP_UnregCallback(ISP_CALLBACK_ENUM Type);
/*******************************************************************************
*
********************************************************************************/

#endif
