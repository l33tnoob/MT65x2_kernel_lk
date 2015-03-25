/******************************************************************************
 * pwrap_hal.c - Linux pmic_wrapper Driver,hardware_dependent driver
 *
 *
 * DESCRIPTION:
 *     This file provides other drivers PMIC wrapper relative functions
 *
 ******************************************************************************/

#include <linux/spinlock.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <mach/mt_typedefs.h>
#include <linux/timer.h>
#include <linux/aee.h>
#include <mach/mt_pmic_wrap.h>
#include <linux/io.h>
#include "pwrap_hal.h"

#define PMIC_WRAP_DEVICE "pmic_wrap"

static struct mt_pmic_wrap_driver *mt_wrp;

static spinlock_t	wrp_lock = __SPIN_LOCK_UNLOCKED(lock);
static spinlock_t	wrp_lock_isr = __SPIN_LOCK_UNLOCKED(lock);
static PWRAP_DEBUG_DATA_T pwrap_debug_data[PWRAP_DEBUG_COUNT];
static U32 pwrap_debug_index = 0;
static U64 elapse_time = 0;
#ifdef PWRAP_TIME_LOG
static struct pwrap_access_time pwrap_access_time_log[PWRAP_DBG_MAX][PWRAP_DEBUG_COUNT];
#endif

/* interral API */
static S32 _pwrap_init_dio(U32 dio_en);
static S32 _pwrap_init_cipher(void);
static S32 _pwrap_init_reg_clock(U32 regck_sel);

static S32 _pwrap_wacs2_nochk(U32 write, U32 adr, U32 wdata, U32 *rdata);
static S32 pwrap_write_nochk(U32 adr, U32 wdata);
static S32 pwrap_read_nochk(U32 adr, U32 *rdata);
#ifdef PWRAP_TIME_LOG
static S32 pwrap_wacs2_hal_repeat(U32  write, U32  adr, U32  wdata, U32 *rdata);
static void pwrap_dump_time_log(void);
#endif

/*-pwrap debug--------------------------------------------------------------------------*/
static inline void pwrap_dump_ap_register(void)
{
	U32 i = 0;

	PWRAPREG("dump pwrap register, base=0x%X\n",PMIC_WRAP_BASE);
	PWRAPREG("address     :   3 2 1 0    7 6 5 4    B A 9 8    F E D C \n");
	for (i = 0; i < PMIC_WRAP_REG_RANGE; i += 4)
	{
		PWRAPREG("offset 0x%.3X:0x%.8X 0x%.8X 0x%.8X 0x%.8X \n", i * 4,
			WRAP_RD32(PMIC_WRAP_BASE + (i * 4) + 0),
			WRAP_RD32(PMIC_WRAP_BASE + (i * 4) + 0x4),
			WRAP_RD32(PMIC_WRAP_BASE + (i * 4) + 0x8),
			WRAP_RD32(PMIC_WRAP_BASE + (i * 4) + 0xC));
	}
	PWRAPREG("0x%X = 0x%X\n", PMIC_WRAP_SWRST, WRAP_RD32(PMIC_WRAP_SWRST));
	PWRAPREG("0x%X = 0x%X\n", PMIC_WRAP_DEBUG_SEL, WRAP_RD32(PMIC_WRAP_DEBUG_SEL));
	PWRAPREG("elapse_time=%lld(ns)\n", elapse_time);

	PWRAPREG("dump clock_ctrl register, base=0x%x\n",TOP_CLOCK_CTRL_BASE);
	PWRAPREG("address     :   3 2 1 0    7 6 5 4    B A 9 8    F E D C \n");
	for (i = 0; i < CLOCK_CTRL_REG_RANGE; i += 4)
	{
		PWRAPREG("offset 0x%.3X:0x%.8X 0x%.8X 0x%.8X 0x%.8X \n", i * 4,
			WRAP_RD32(TOP_CLOCK_CTRL_BASE + (i * 4) + 0),
			WRAP_RD32(TOP_CLOCK_CTRL_BASE + (i * 4) + 0x4),
			WRAP_RD32(TOP_CLOCK_CTRL_BASE + (i * 4) + 0x8),
			WRAP_RD32(TOP_CLOCK_CTRL_BASE + (i * 4) + 0xC));
	}

	return;
}
static inline void pwrap_dump_pmic_register(void)
{
	U32 i = 0;
	U32 reg_addr = 0;
	U32 reg_value = 0;

	PWRAPREG("dump dewrap register\n");
	for (i = 0; i < PMIC_SPISLV_REG_RANGE; i++)
	{
		reg_addr = (DEW_BASE + (i * 2));
		pwrap_read_nochk(reg_addr, &reg_value);
		PWRAPREG("0x%X = 0x%X\n", reg_addr, reg_value);
	}
	return;
}

static inline void pwrap_dump_all_register(void)
{
	pwrap_dump_ap_register();
	pwrap_dump_pmic_register();
	return;
}

void pwrap_trace(U64 wacs_time, U64 result, U32 write, U32 addr, U32 wdata, U32 rdata)
{
	U32 index;
	index = pwrap_debug_index;
	if (write == 0)
		pwrap_debug_data[index].operation = _PWRAP_READ;
	else
		pwrap_debug_data[index].operation = _PWRAP_WRITE;
	pwrap_debug_data[index].wacs_time = wacs_time;
	pwrap_debug_data[index].result = result;
	pwrap_debug_data[index].addr = addr;
	pwrap_debug_data[index].wdata = wdata;
	pwrap_debug_data[index].rdata = rdata;

	pwrap_debug_index++;
	pwrap_debug_index %= PWRAP_DEBUG_COUNT;
}

#ifdef PWRAP_TIME_LOG
static void pwrap_dump_time_log(void)
{
	U32 i = 0, j = 0;
	/* print the latest time access log */

	for (i = 0; i < PWRAP_DBG_MAX; i++) {
		PWRAPREG("the latest 20 access time %d\n", i);
		if (pwrap_debug_index >= 20) {
			for (j = pwrap_debug_index - 20 + 1; j <= pwrap_debug_index; j++) {
				PWRAPREG("index=%d,start=%lld,end=%lld,duration=%lld\n", j,
				  pwrap_access_time_log[i][j].start_time,
				  pwrap_access_time_log[i][j].end_time,
				  pwrap_access_time_log[i][j].duration_time);
			}
		} else {
			for (j = PWRAP_DEBUG_COUNT + pwrap_debug_index - 20 + 1; j < PWRAP_DEBUG_COUNT; j++) {
				PWRAPREG("index=%d,start=%lld,end=%lld,duration=%lld\n", j,
				  pwrap_access_time_log[i][j].start_time,
				  pwrap_access_time_log[i][j].end_time,
				  pwrap_access_time_log[i][j].duration_time);
			}
			for (j = 0; j <= pwrap_debug_index; j++) {
				PWRAPREG("index=%d,start=%lld,end=%lld,duration=%lld\n", j,
				  pwrap_access_time_log[i][j].start_time,
				  pwrap_access_time_log[i][j].end_time,
				  pwrap_access_time_log[i][j].duration_time);
			}
		}
	}
}
#endif


/******************************************************************************
  wrapper timeout
 ******************************************************************************/
#define PWRAP_TIMEOUT
#ifdef PWRAP_TIMEOUT
static U64 _pwrap_get_current_time(void)
{
	return sched_clock();   /* TODO: fix me? */
}

static BOOL _pwrap_timeout_ns(U64 start_time_ns, U64 timeout_time_ns)
{
	U64 cur_time = 0;
	/* U64 elapse_time=0; */

	/* get current tick (ns) */
	cur_time = _pwrap_get_current_time();
	elapse_time = cur_time - start_time_ns;

	/* check if timeout */
	if (timeout_time_ns <= elapse_time)
	{
		/* timeout */
		return TRUE;
	}
	return FALSE;
}
static U64 _pwrap_time2ns(U64 time_us)
{
	return time_us * 1000;
}
#else
static U64 _pwrap_get_current_time(void)
{
	return 0;
}
static BOOL _pwrap_timeout_ns(U64 start_time_ns, U64 timeout_time_ns)
{
	return FALSE;
}
static U64 _pwrap_time2ns(U64 time_us)
{
	return 0;
}

#endif
/*
 * define macro and inline function (for do while loop)
 */
typedef U32 (*loop_condition_fp)(U32);/* define a function pointer */

static inline U32 wait_for_fsm_idle(U32 x)
{
	return (GET_WACS0_FSM(x) != WACS_FSM_IDLE);
}
static inline U32 wait_for_fsm_vldclr(U32 x)
{
	return (GET_WACS0_FSM(x) != WACS_FSM_WFVLDCLR);
}
static inline U32 wait_for_sync(U32 x)
{
	return (GET_SYNC_IDLE0(x) != WACS_SYNC_IDLE);
}
static inline U32 wait_for_idle_and_sync(U32 x)
{
	return ((GET_WACS0_FSM(x) != WACS_FSM_IDLE) || (GET_SYNC_IDLE0(x) != WACS_SYNC_IDLE));
}
static inline U32 wait_for_wrap_idle(U32 x)
{
	return ((GET_WRAP_FSM(x) != 0x0) || (GET_WRAP_CH_DLE_RESTCNT(x) != 0x0));
}
static inline U32 wait_for_wrap_state_idle(U32 x)
{
	return (GET_WRAP_AG_DLE_RESTCNT(x) != 0);
}
static inline U32 wait_for_man_idle_and_noreq(U32 x)
{
	return ((GET_MAN_REQ(x) != MAN_FSM_NO_REQ) || (GET_MAN_FSM(x) != MAN_FSM_IDLE));
}
static inline U32 wait_for_man_vldclr(U32 x)
{
	return (GET_MAN_FSM(x) != MAN_FSM_WFVLDCLR);
}
static inline U32 wait_for_cipher_ready(U32 x)
{
	return (x != 1);
}
static inline U32 wait_for_stdupd_idle(U32 x)
{
	return (GET_STAUPD_FSM(x) != 0x0);
}

static inline U32 wait_for_state_ready_init(loop_condition_fp fp, U32 timeout_us, U32 wacs_register, U32 *read_reg)
{

	U64 start_time_ns = 0, timeout_ns = 0;
	U32 reg_rdata = 0, loop_counter = 0;
	start_time_ns = _pwrap_get_current_time();
	timeout_ns = _pwrap_time2ns(timeout_us);
	do
	{
		loop_counter++;
		reg_rdata = WRAP_RD32(wacs_register);

		if (_pwrap_timeout_ns(start_time_ns, timeout_ns))
		{
			PWRAPERR("%s timeout when waiting for idle: %lld ns\n", __FUNCTION__, elapse_time);
			PWRAPERR("reg_data: 0x%x  counter: %d\n", reg_rdata, loop_counter);
			return E_PWR_WAIT_IDLE_TIMEOUT;
		}
	} while (fp(reg_rdata)); /* IDLE State */
	if (read_reg)
		*read_reg = reg_rdata;
	return 0;
}

static inline U32 wait_for_state_idle_init(loop_condition_fp fp, U32 timeout_us, U32 wacs_register, U32 wacs_vldclr_register, U32 *read_reg)
{

	U64 start_time_ns = 0, timeout_ns = 0;
	U32 reg_rdata = 0, loop_counter = 0;
	start_time_ns = _pwrap_get_current_time();
	timeout_ns = _pwrap_time2ns(timeout_us);
	do
	{
		loop_counter++;
		reg_rdata = WRAP_RD32(wacs_register);

		/* if last read command timeout,clear vldclr bit
		 * read command state machine:FSM_REQ-->wfdle-->WFVLDCLR;write:FSM_REQ-->idle
		 */
		switch (GET_WACS0_FSM(reg_rdata))
		{
			case WACS_FSM_WFVLDCLR:
				WRAP_WR32(wacs_vldclr_register, 1);
				PWRAPERR("WACS_FSM = PMIC_WRAP_WACS_VLDCLR\n");
				break;
			case WACS_FSM_WFDLE:
				PWRAPERR("WACS_FSM = WACS_FSM_WFDLE\n");
				break;
			case WACS_FSM_REQ:
				PWRAPERR("WACS_FSM = WACS_FSM_REQ\n");
				break;
			default:
				break;
		}
		if (_pwrap_timeout_ns(start_time_ns, timeout_ns))
		{
			PWRAPERR("%s timeout when waiting for idle: %lld ns\n", __FUNCTION__, elapse_time);
			PWRAPERR("reg_data: 0x%x  counter: %d\n", reg_rdata, loop_counter);
			pwrap_dump_ap_register();
			/* pwrap_trace_wacs2();
			   BUG_ON(1); 
			*/
			return E_PWR_WAIT_IDLE_TIMEOUT;
		}
	} while (fp(reg_rdata)); /* IDLE State */
	if (read_reg)
		*read_reg = reg_rdata;
	return 0;
}

static inline U32 wait_for_state_idle(loop_condition_fp fp, U32 timeout_us, U32 wacs_register, U32 wacs_vldclr_register, U32 *read_reg)
{
	U64 start_time_ns = 0, timeout_ns = 0;
	U32 reg_rdata = 0, loop_counter = 0;

	#ifdef PWRAP_TIME_LOG
	pwrap_access_time_log[PWRAP_DBG][pwrap_debug_index].start_time = sched_clock();
	#endif

	start_time_ns = _pwrap_get_current_time();
	timeout_ns = _pwrap_time2ns(timeout_us);
	do
	{
		loop_counter++;

		#ifdef PWRAP_TIME_LOG
		pwrap_access_time_log[PWRAP_WACS2_READ_DBG][pwrap_debug_index].start_time = sched_clock();
		#endif

		reg_rdata = WRAP_RD32(wacs_register);

		#ifdef PWRAP_TIME_LOG
		pwrap_access_time_log[PWRAP_WACS2_READ_DBG][pwrap_debug_index].end_time = sched_clock();
		pwrap_access_time_log[PWRAP_WACS2_READ_DBG][pwrap_debug_index].duration_time = \
		  pwrap_access_time_log[PWRAP_WACS2_READ_DBG][pwrap_debug_index].end_time - \
		  pwrap_access_time_log[PWRAP_WACS2_READ_DBG][pwrap_debug_index].start_time;
		#endif

		if(GET_INIT_DONE0(reg_rdata) != WACS_INIT_DONE)
		{
			PWRAPERR("initialization isn't finished \n");
			return E_PWR_NOT_INIT_DONE;
		}
		/* if last read command timeout,clear vldclr bit
		 * read command state machine:FSM_REQ-->wfdle-->WFVLDCLR;write:FSM_REQ-->idle
		 */
		switch (GET_WACS0_FSM(reg_rdata))
		{
			case WACS_FSM_WFVLDCLR:
				WRAP_WR32(wacs_vldclr_register, 1);
				PWRAPERR("WACS_FSM = PMIC_WRAP_WACS_VLDCLR\n");
				break;
			case WACS_FSM_WFDLE:
				PWRAPERR("WACS_FSM = WACS_FSM_WFDLE\n");
				break;
			case WACS_FSM_REQ:
				PWRAPERR("WACS_FSM = WACS_FSM_REQ\n");
				break;
			default:
				break;
		}
		if (_pwrap_timeout_ns(start_time_ns, timeout_ns))
		{
			#ifdef PWRAP_TIME_LOG
			pwrap_access_time_log[PWRAP_DBG][pwrap_debug_index].end_time = sched_clock();
			pwrap_access_time_log[PWRAP_DBG][pwrap_debug_index].duration_time = \
			  pwrap_access_time_log[PWRAP_DBG][pwrap_debug_index].end_time - \
			  pwrap_access_time_log[PWRAP_DBG][pwrap_debug_index].start_time;
			PWRAPLOG("read duration: %lld ns\n", pwrap_access_time_log[PWRAP_WACS2_READ_DBG][pwrap_debug_index].duration_time);
			pwrap_dump_time_log();
			#endif
			PWRAPERR("%s timeout when waiting for idle: %lld ns\n", __FUNCTION__, elapse_time);
			PWRAPERR("reg_data: 0x%x  counter: %d\n", reg_rdata, loop_counter);
			pwrap_dump_ap_register();
			/* pwrap_trace_wacs2();
			   BUG_ON(1);
			 */
			return E_PWR_WAIT_IDLE_TIMEOUT;
		}
	} while (fp(reg_rdata)); /* IDLE State */
	if (read_reg)
		*read_reg = reg_rdata;
	return 0;
}

static inline U32 wait_for_state_ready(loop_condition_fp fp, U32 timeout_us, U32 wacs_register, U32 *read_reg)
{
	U64 start_time_ns = 0, timeout_ns = 0;
	U32 reg_rdata = 0, loop_counter = 0;

	#ifdef PWRAP_TIME_LOG
	pwrap_access_time_log[PWRAP_READY_DBG][pwrap_debug_index].start_time = sched_clock();
	#endif

	start_time_ns = _pwrap_get_current_time();
	timeout_ns = _pwrap_time2ns(timeout_us);
	do
	{
		loop_counter++;

		#ifdef PWRAP_TIME_LOG
		pwrap_access_time_log[PWRAP_READY_READ_DBG][pwrap_debug_index].start_time = sched_clock();
		#endif

		reg_rdata = WRAP_RD32(wacs_register);

		#ifdef PWRAP_TIME_LOG
		pwrap_access_time_log[PWRAP_READY_READ_DBG][pwrap_debug_index].end_time = sched_clock();
		pwrap_access_time_log[PWRAP_READY_READ_DBG][pwrap_debug_index].duration_time = \
		  pwrap_access_time_log[PWRAP_READY_READ_DBG][pwrap_debug_index].end_time - \
		  pwrap_access_time_log[PWRAP_READY_READ_DBG][pwrap_debug_index].start_time;
		#endif

		if(GET_INIT_DONE0(reg_rdata) != WACS_INIT_DONE)
		{
			PWRAPERR("initialization isn't finished \n");
			return E_PWR_NOT_INIT_DONE;
		}
		if (_pwrap_timeout_ns(start_time_ns, timeout_ns))
		{
			#ifdef PWRAP_TIME_LOG
			pwrap_access_time_log[PWRAP_READY_DBG][pwrap_debug_index].end_time = sched_clock();
			pwrap_access_time_log[PWRAP_READY_DBG][pwrap_debug_index].duration_time = \
			  pwrap_access_time_log[PWRAP_READY_DBG][pwrap_debug_index].end_time - \
			  pwrap_access_time_log[PWRAP_READY_DBG][pwrap_debug_index].start_time;
			PWRAPLOG("read duration: %lld ns\n", pwrap_access_time_log[PWRAP_READY_READ_DBG][pwrap_debug_index].duration_time);
			pwrap_dump_time_log();
			#endif
			PWRAPERR("timeout when waiting for idle: %lld ns\n", elapse_time);
			PWRAPERR("reg_data: 0x%x  counter: %d\n", reg_rdata, loop_counter);
			pwrap_dump_ap_register();
			/* pwrap_trace_wacs2(); */
			return E_PWR_WAIT_IDLE_TIMEOUT;
		}
	} while (fp(reg_rdata)); /* IDLE State */
	if (read_reg)
		*read_reg = reg_rdata;

	#ifdef PWRAP_TIME_LOG
	pwrap_access_time_log[PWRAP_READY_DBG][pwrap_debug_index].end_time = sched_clock();
	pwrap_access_time_log[PWRAP_READY_DBG][pwrap_debug_index].duration_time = \
	  pwrap_access_time_log[PWRAP_READY_DBG][pwrap_debug_index].end_time - \
	  pwrap_access_time_log[PWRAP_READY_DBG][pwrap_debug_index].start_time;
	#endif

	return 0;
}
/********************************************************************************************/
//extern API for PMIC driver, INT related control, this INT is for PMIC chip to AP (ROME)
/********************************************************************************************/
U32 mt_pmic_wrap_eint_status(void)
{
	return 0;
}

void mt_pmic_wrap_eint_clr(int offset)
{

}

//--------------------------------------------------------
//    Function : pwrap_wacs2_hal()
// Description :
//   Parameter :
//      Return :
//--------------------------------------------------------
static S32 pwrap_wacs2_hal(U32  write, U32  adr, U32  wdata, U32 *rdata)
{
	U64 wrap_access_time=0x0;
	U32 reg_rdata=0;
	U32 wacs_write=0;
	U32 wacs_adr=0;
	U32 wacs_cmd=0;
	U32 return_value=0;
	unsigned long flags=0;
	//PWRAPFUC();
	//#ifndef CONFIG_MTK_LDVT_PMIC_WRAP
	//PWRAPLOG("wrapper access,write=%x,add=%x,wdata=%x,rdata=%x\n",write,adr,wdata,rdata);
	//#endif
	// Check argument validation
	if( (write & ~(0x1))    != 0) {
		return_value = E_PWR_INVALID_RW;
		PWRAPERR("argument error %d: 0x%x\n", return_value, write);
		return return_value;
	}
	if( (adr   & ~(0xffff)) != 0) {
		return_value = E_PWR_INVALID_ADDR;
		PWRAPERR("argument error %d: 0x%x\n", return_value, adr);
		return return_value;
	}
	if( (wdata & ~(0xffff)) != 0) {
		return_value = E_PWR_INVALID_WDAT;
		PWRAPERR("argument error %d: 0x%x\n", return_value, wdata);
		return return_value;
	}

	spin_lock_irqsave(&wrp_lock,flags);
	// Check IDLE & INIT_DONE in advance
	return_value=wait_for_state_idle(wait_for_fsm_idle,TIMEOUT_WAIT_IDLE,PMIC_WRAP_WACS2_RDATA,PMIC_WRAP_WACS2_VLDCLR,0);
	if(return_value!=0)
	{
		PWRAPERR("wait_for_fsm_idle fail,return_value=%d\n",return_value);
		goto FAIL;
	}
	wacs_write  = write << 31;
	wacs_adr    = (adr >> 1) << 16;
	wacs_cmd = wacs_write | wacs_adr | wdata;

	#ifdef PWRAP_TIME_LOG
	pwrap_access_time_log[PWRAP_WACS2_WRITE_DBG][pwrap_debug_index].start_time = sched_clock();
	#endif

	WRAP_WR32(PMIC_WRAP_WACS2_CMD,wacs_cmd);

	#ifdef PWRAP_TIME_LOG
	pwrap_access_time_log[PWRAP_WACS2_WRITE_DBG][pwrap_debug_index].end_time = sched_clock();
	pwrap_access_time_log[PWRAP_WACS2_WRITE_DBG][pwrap_debug_index].duration_time \
	  = pwrap_access_time_log[PWRAP_WACS2_WRITE_DBG][pwrap_debug_index].end_time - \
	    pwrap_access_time_log[PWRAP_WACS2_WRITE_DBG][pwrap_debug_index].start_time;
	#endif

	if(write == 0)
	{
		return_value=wait_for_state_ready(wait_for_fsm_vldclr,TIMEOUT_READ,PMIC_WRAP_WACS2_RDATA,&reg_rdata);
		if(return_value!=0)
		{
			PWRAPERR("wait_for_fsm_vldclr fail,return_value=%d\n",return_value);
			return_value+=1;//E_PWR_NOT_INIT_DONE_READ or E_PWR_WAIT_IDLE_TIMEOUT_READ
			goto FAIL;
		}
		if (NULL == rdata)
		{
			PWRAPERR("rdata is a NULL pointer\n");
			return_value= E_PWR_INVALID_ARG;
			WRAP_WR32(PMIC_WRAP_WACS2_VLDCLR , 1);
			goto FAIL;
		}

		*rdata = GET_WACS0_RDATA( reg_rdata );
		WRAP_WR32(PMIC_WRAP_WACS2_VLDCLR , 1);
	}
FAIL:
	spin_unlock_irqrestore(&wrp_lock,flags);
	if(return_value!=0)
	{
		PWRAPERR("pwrap_wacs2_hal fail,return_value=%d\n",return_value);
		PWRAPERR("timeout:BUG_ON here\n");
		//BUG_ON(1);
		//aee_kernel_warning("PMIC_WRAP", "%s, elapse_time: %d, ret: %d\n", __FUNCTION__, elapse_time, return_value);
	}
	wrap_access_time=sched_clock();
	if (write == 0 && NULL != rdata)
		pwrap_trace(wrap_access_time, return_value, write, adr, 0, *rdata);
	else
		pwrap_trace(wrap_access_time, return_value, write, adr, wdata, 0);

	#ifdef PWRAP_TIME_LOG
	/* send the request again */
	if (return_value != 0) {
		return_value = pwrap_wacs2_hal_repeat(write, adr, wdata, rdata);
		if (return_value != 0)
			PWRAPERR("retried request fail: %d\n", return_value);
	}
	#endif

	return return_value;
}

#ifdef PWRAP_TIME_LOG
static S32 pwrap_wacs2_hal_repeat(U32  write, U32  adr, U32  wdata, U32 *rdata)
{
	U64 wrap_access_time=0x0;
	U32 reg_rdata=0;
	U32 wacs_write=0;
	U32 wacs_adr=0;
	U32 wacs_cmd=0;
	U32 return_value=0;
	unsigned long flags=0;

	// Check argument validation
	if( (write & ~(0x1))    != 0) {
		return_value = E_PWR_INVALID_RW;
		PWRAPERR("argument error %d: 0x%x\n", return_value, write);
		return return_value;
	}
	if( (adr   & ~(0xffff)) != 0) {
		return_value = E_PWR_INVALID_ADDR;
		PWRAPERR("argument error %d: 0x%x\n", return_value, adr);
		return return_value;
	}
	if( (wdata & ~(0xffff)) != 0) {
		return_value = E_PWR_INVALID_WDAT;
		PWRAPERR("argument error %d: 0x%x\n", return_value, wdata);
		return return_value;
	}

	spin_lock_irqsave(&wrp_lock,flags);
	// Check IDLE & INIT_DONE in advance
	return_value=wait_for_state_idle(wait_for_fsm_idle,TIMEOUT_WAIT_IDLE,PMIC_WRAP_WACS2_RDATA,PMIC_WRAP_WACS2_VLDCLR,0);
	if(return_value!=0)
	{
		PWRAPERR("wait_for_fsm_idle fail,return_value=%d\n",return_value);
		goto FAIL;
	}
	wacs_write  = write << 31;
	wacs_adr    = (adr >> 1) << 16;
	wacs_cmd = wacs_write | wacs_adr | wdata;

	pwrap_access_time_log[PWRAP_WACS2_WRITE_DBG][pwrap_debug_index].start_time = sched_clock();

	WRAP_WR32(PMIC_WRAP_WACS2_CMD,wacs_cmd);

	pwrap_access_time_log[PWRAP_WACS2_WRITE_DBG][pwrap_debug_index].end_time = sched_clock();
	pwrap_access_time_log[PWRAP_WACS2_WRITE_DBG][pwrap_debug_index].duration_time \
	  = pwrap_access_time_log[PWRAP_WACS2_WRITE_DBG][pwrap_debug_index].end_time - \
	    pwrap_access_time_log[PWRAP_WACS2_WRITE_DBG][pwrap_debug_index].start_time;

	if (write == 0)
	{
		return_value=wait_for_state_ready(wait_for_fsm_vldclr,TIMEOUT_READ,PMIC_WRAP_WACS2_RDATA,&reg_rdata);
		if(return_value!=0)
		{
			PWRAPERR("wait_for_fsm_vldclr fail,return_value=%d\n",return_value);
			return_value+=1;//E_PWR_NOT_INIT_DONE_READ or E_PWR_WAIT_IDLE_TIMEOUT_READ
			goto FAIL;
		}
		if (NULL == rdata)
		{
			PWRAPERR("rdata is a NULL pointer\n");
			return_value= E_PWR_INVALID_ARG;
			WRAP_WR32(PMIC_WRAP_WACS2_VLDCLR , 1);
			goto FAIL;
		}

		*rdata = GET_WACS0_RDATA(reg_rdata);
		WRAP_WR32(PMIC_WRAP_WACS2_VLDCLR, 1);
	}
FAIL:
	spin_unlock_irqrestore(&wrp_lock,flags);
	if(return_value!=0)
	{
		PWRAPERR("%s fail,return_value=%d\n", __FUNCTION__, return_value);
		PWRAPERR("timeout:BUG_ON here\n");
		//BUG_ON(1);
		aee_kernel_exception("PMIC_WRAP", "%s, elapse_time: %d, ret: %d\n", __FUNCTION__, elapse_time, return_value);
	}
	wrap_access_time=sched_clock();

	if (write == 0 && NULL != rdata)
		pwrap_trace(wrap_access_time, return_value, write, adr, 0, *rdata);
	else
		pwrap_trace(wrap_access_time, return_value, write, adr, wdata, 0);

	return return_value;
}
#endif

//S32 pwrap_wacs2( U32  write, U32  adr, U32  wdata, U32 *rdata )
//{
//	return pwrap_wacs2_hal(write, adr,wdata,rdata );
//}
//EXPORT_SYMBOL(pwrap_wacs2);
//S32 pwrap_read( U32  adr, U32 *rdata )
//{
//	return pwrap_wacs2( PWRAP_READ, adr,0,rdata );
//}
//EXPORT_SYMBOL(pwrap_read);
//
//S32 pwrap_write( U32  adr, U32  wdata )
//{
//	return pwrap_wacs2( PWRAP_WRITE, adr,wdata,0 );
//}
//EXPORT_SYMBOL(pwrap_write);
//******************************************************************************
//--internal API for pwrap_init-------------------------------------------------
//******************************************************************************
//--------------------------------------------------------
//    Function : _pwrap_wacs2_nochk()
// Description :
//   Parameter :
//      Return :
//--------------------------------------------------------
static S32 pwrap_read_nochk( U32  adr, U32 *rdata )
{
	return _pwrap_wacs2_nochk( 0, adr,  0, rdata );
}

static S32 pwrap_write_nochk( U32  adr, U32  wdata )
{
	return _pwrap_wacs2_nochk( 1, adr,wdata,0 );
}

static S32 _pwrap_wacs2_nochk( U32 write, U32 adr, U32 wdata, U32 *rdata )
{
	U32 reg_rdata=0x0;
	U32 wacs_write=0x0;
	U32 wacs_adr=0x0;
	U32 wacs_cmd=0x0;
	U32 return_value=0x0;
	//PWRAPFUC();
	// Check argument validation
	if( (write & ~(0x1))    != 0)  return E_PWR_INVALID_RW;
	if( (adr   & ~(0xffff)) != 0)  return E_PWR_INVALID_ADDR;
	if( (wdata & ~(0xffff)) != 0)  return E_PWR_INVALID_WDAT;

	// Check IDLE
	return_value=wait_for_state_idle_init(wait_for_fsm_idle, TIMEOUT_WAIT_IDLE, PMIC_WRAP_WACS2_RDATA, PMIC_WRAP_WACS2_VLDCLR, 0);
	if(return_value!=0)
	{
		PWRAPERR("_pwrap_wacs2_nochk write command fail,return_value=%x\n", return_value);
		return return_value;
	}

	wacs_write  = write << 31;
	wacs_adr    = (adr >> 1) << 16;
	wacs_cmd = wacs_write | wacs_adr | wdata;
	WRAP_WR32(PMIC_WRAP_WACS2_CMD,wacs_cmd);

	if( write == 0 )
	{
		// wait for read data ready
		return_value=wait_for_state_ready_init(wait_for_fsm_vldclr,TIMEOUT_WAIT_IDLE,PMIC_WRAP_WACS2_RDATA,&reg_rdata);
		if(return_value!=0)
		{
			PWRAPERR("_pwrap_wacs2_nochk read fail,return_value=%x\n", return_value);
			return return_value;
		}
		if (NULL == rdata)
		{
			PWRAPERR("rdata is a NULL pointer\n");
			WRAP_WR32(PMIC_WRAP_WACS2_VLDCLR , 1);
			return E_PWR_INVALID_ARG;
		}

		*rdata = GET_WACS0_RDATA( reg_rdata );
		WRAP_WR32(PMIC_WRAP_WACS2_VLDCLR , 1);
	}
	return 0;
}

//--------------------------------------------------------
//    Function : _pwrap_init_dio()
// Description :call it in pwrap_init,mustn't check init done
//   Parameter :
//      Return :
//--------------------------------------------------------
static S32 _pwrap_init_dio( U32 dio_en )
{
	U32 arb_en_backup=0x0;
	U32 rdata=0x0;
	U32 return_value=0;

	//PWRAPFUC();
	arb_en_backup = WRAP_RD32(PMIC_WRAP_HIPRIO_ARB_EN);
	WRAP_WR32(PMIC_WRAP_HIPRIO_ARB_EN ,WACS2 ); // only WACS2
	pwrap_write_nochk(DEW_DIO_EN, dio_en);

	// Check IDLE & INIT_DONE in advance
	return_value=wait_for_state_ready_init(wait_for_idle_and_sync,TIMEOUT_WAIT_IDLE,PMIC_WRAP_WACS2_RDATA,0);
	if(return_value!=0)
	{
		PWRAPERR("_pwrap_init_dio fail,return_value=%x\n", return_value);
		return return_value;
	}
	WRAP_WR32(PMIC_WRAP_DIO_EN , dio_en);
	// Read Test
	pwrap_read_nochk(DEW_READ_TEST,&rdata);
	if( rdata != DEFAULT_VALUE_READ_TEST )
	{
		PWRAPERR("[Dio_mode][Read Test] fail,dio_en = %x, READ_TEST rdata=%x, exp=0x5aa5\n", dio_en, rdata);
		return E_PWR_READ_TEST_FAIL;
	}
	WRAP_WR32(PMIC_WRAP_HIPRIO_ARB_EN , arb_en_backup);
	return 0;
}

//--------------------------------------------------------
//    Function : _pwrap_init_cipher()
// Description :
//   Parameter :
//      Return :
//--------------------------------------------------------
static S32 _pwrap_init_cipher( void )
{
	U32 arb_en_backup=0;
	U32 rdata=0;
	U32 return_value=0;
	U64 start_time_ns=0, timeout_ns=0;
	//PWRAPFUC();
	arb_en_backup = WRAP_RD32(PMIC_WRAP_HIPRIO_ARB_EN);

	WRAP_WR32(PMIC_WRAP_HIPRIO_ARB_EN ,WACS2); // only WACS2

	WRAP_WR32(PMIC_WRAP_CIPHER_SWRST ,  1);
	WRAP_WR32(PMIC_WRAP_CIPHER_SWRST ,  0);
	WRAP_WR32(PMIC_WRAP_CIPHER_KEY_SEL , 1);
	WRAP_WR32(PMIC_WRAP_CIPHER_IV_SEL  , 2);
#ifdef SLV_6320
	WRAP_WR32(PMIC_WRAP_CIPHER_LOAD  , 1);
#endif
	WRAP_WR32(PMIC_WRAP_CIPHER_EN   , 1);

	//Config CIPHER @ PMIC
	pwrap_write_nochk(DEW_CIPHER_SWRST, 0x1);
	pwrap_write_nochk(DEW_CIPHER_SWRST, 0x0);
	pwrap_write_nochk(DEW_CIPHER_KEY_SEL, 0x1);
	pwrap_write_nochk(DEW_CIPHER_IV_SEL,  0x2);
#ifdef SLV_6320
	pwrap_write_nochk(DEW_CIPHER_LOAD,  0x1);
	pwrap_write_nochk(DEW_CIPHER_START, 0x1);
#elif defined SLV_6323
	pwrap_write_nochk(DEW_CIPHER_EN,  0x1);
#endif

	//wait for cipher data ready@AP
	return_value=wait_for_state_ready_init(wait_for_cipher_ready,TIMEOUT_WAIT_IDLE,PMIC_WRAP_CIPHER_RDY,0);
	if(return_value!=0)
	{
		PWRAPERR("wait for cipher data ready@AP fail,return_value=%x\n", return_value);
		return return_value;
	}

	//wait for cipher data ready@PMIC
	start_time_ns = _pwrap_get_current_time();
	timeout_ns = _pwrap_time2ns(0xFFFF);
	do
	{
		pwrap_read_nochk(DEW_CIPHER_RDY,&rdata);
		if (_pwrap_timeout_ns(start_time_ns, timeout_ns))
		{
			PWRAPERR("wait for cipher data ready@PMIC: %lld ns\n", elapse_time);
			return E_PWR_WAIT_IDLE_TIMEOUT;
		}
	} while( rdata != 0x1 ); //cipher_ready

	pwrap_write_nochk(DEW_CIPHER_MODE, 0x1);
	//wait for cipher mode idle
	return_value=wait_for_state_ready_init(wait_for_idle_and_sync,TIMEOUT_WAIT_IDLE,PMIC_WRAP_WACS2_RDATA,0);
	if(return_value!=0)
	{
		PWRAPERR("wait for cipher mode idle fail,return_value=%x\n", return_value);
		return return_value;
	}
	WRAP_WR32(PMIC_WRAP_CIPHER_MODE , 1);

	// Read Test
	pwrap_read_nochk(DEW_READ_TEST,&rdata);
	if( rdata != DEFAULT_VALUE_READ_TEST )
	{
		PWRAPERR("_pwrap_init_cipher,read test error,error code=%x, rdata=%x\n", 1, rdata);
		return E_PWR_READ_TEST_FAIL;
	}
	WRAP_WR32(PMIC_WRAP_HIPRIO_ARB_EN , arb_en_backup);
	return 0;
}

//--------------------------------------------------------
//    Function : _pwrap_init_sistrobe()
// Description : Initialize SI_CK_CON and SIDLY
//   Parameter :
//      Return :
//--------------------------------------------------------
static S32 _pwrap_init_sistrobe( void )
{
	U32 arb_en_backup;
	U32 rdata;
	S32 ind, tmp1, tmp2;
	U32 result;
	U32 result_faulty;
	U32 leading_one, tailing_one;

	arb_en_backup = WRAP_RD32(PMIC_WRAP_HIPRIO_ARB_EN);

	WRAP_WR32(PMIC_WRAP_HIPRIO_ARB_EN ,WACS2); // only WACS2

	//---------------------------------------------------------------------
	// Scan all possible input strobe by READ_TEST
	//---------------------------------------------------------------------
	result = 0;
	result_faulty = 0;
	for( ind=0; ind<24; ind++)  // 24 sampling clock edge
	{
		WRAP_WR32(PMIC_WRAP_SI_CK_CON , (ind >> 2) & 0x7);
		WRAP_WR32(PMIC_WRAP_SIDLY ,0x3 - (ind & 0x3));
		_pwrap_wacs2_nochk(0, DEW_READ_TEST, 0, &rdata);
		if( rdata == DEFAULT_VALUE_READ_TEST )
		{
			PWRAPLOG("_pwrap_init_sistrobe [Read Test] pass,index=%d rdata=%x\n", ind,rdata);
			result |= (0x1 << ind);
		}
		else
			PWRAPLOG("_pwrap_init_sistrobe [Read Test] fail,index=%d,rdata=%x\n", ind,rdata);
	}

	//---------------------------------------------------------------------
	// Locate the leading one and trailing one
	//---------------------------------------------------------------------
	for( ind=23 ; ind>=0 ; ind-- )
	{
		if( result & (0x1 << ind) ) break;
	}
	leading_one = ind;

	for( ind=0 ; ind<24 ; ind++ )
	{
		if( result & (0x1 << ind) ) break;
	}
	tailing_one = ind;

	//---------------------------------------------------------------------
	// Check the continuity of pass range
	//---------------------------------------------------------------------
	tmp1 = (0x1 << (leading_one+1)) - 1;
	tmp2 = (0x1 << tailing_one) - 1;
	if( (tmp1 - tmp2) != result )
	{
		/*TERR = "[DrvPWRAP_InitSiStrobe] Fail, tmp1:%d, tmp2:%d", tmp1, tmp2*/
		PWRAPERR("_pwrap_init_sistrobe Fail,tmp1=%x,tmp2=%x\n", tmp1,tmp2);
		result_faulty = 0x1;
	}
	//---------------------------------------------------------------------
	// Config SICK and SIDLY to the middle point of pass range
	//---------------------------------------------------------------------
	ind = (leading_one + tailing_one)/2;
	WRAP_WR32(PMIC_WRAP_SI_CK_CON , (ind >> 2) & 0x7);
	WRAP_WR32(PMIC_WRAP_SIDLY , 0x3 - (ind & 0x3));

	//---------------------------------------------------------------------
	// Restore
	//---------------------------------------------------------------------
	WRAP_WR32(PMIC_WRAP_HIPRIO_ARB_EN , arb_en_backup);
	if( result_faulty == 0 )
		return 0;
	else
	{
		/*TERR = "[DrvPWRAP_InitSiStrobe] Fail, result = %x", result*/
		PWRAPERR("_pwrap_init_sistrobe Fail,result=%x\n", result);
		return result_faulty;
	}
}

//--------------------------------------------------------
//    Function : _pwrap_reset_spislv()
// Description :
//   Parameter :
//      Return :
//--------------------------------------------------------
static S32 _pwrap_reset_spislv( void )
{
	U32 ret=0;
	U32 return_value=0;
	//PWRAPFUC();
	// This driver does not using _pwrap_switch_mux
	// because the remaining requests are expected to fail anyway

	WRAP_WR32(PMIC_WRAP_HIPRIO_ARB_EN , DISABLE_ALL);
	WRAP_WR32(PMIC_WRAP_WRAP_EN , DISABLE);
	WRAP_WR32(PMIC_WRAP_MUX_SEL , MANUAL_MODE);
	WRAP_WR32(PMIC_WRAP_MAN_EN ,ENABLE);
	WRAP_WR32(PMIC_WRAP_DIO_EN ,DISABLE);

	WRAP_WR32(PMIC_WRAP_MAN_CMD , (OP_WR << 13) | (OP_CSL  << 8));//0x2100
	WRAP_WR32(PMIC_WRAP_MAN_CMD , (OP_WR << 13) | (OP_OUTS << 8)); //0x2800//to reset counter
	WRAP_WR32(PMIC_WRAP_MAN_CMD , (OP_WR << 13) | (OP_CSH  << 8));//0x2000
	WRAP_WR32(PMIC_WRAP_MAN_CMD , (OP_WR << 13) | (OP_OUTS << 8));
	WRAP_WR32(PMIC_WRAP_MAN_CMD , (OP_WR << 13) | (OP_OUTS << 8));
	WRAP_WR32(PMIC_WRAP_MAN_CMD , (OP_WR << 13) | (OP_OUTS << 8));
	WRAP_WR32(PMIC_WRAP_MAN_CMD , (OP_WR << 13) | (OP_OUTS << 8));

	return_value=wait_for_state_ready_init(wait_for_sync,TIMEOUT_WAIT_IDLE,PMIC_WRAP_WACS2_RDATA,0);
	if(return_value!=0)
	{
		PWRAPERR("_pwrap_reset_spislv fail,return_value=%x\n", return_value);
		ret=E_PWR_TIMEOUT;
		goto timeout;
	}

	WRAP_WR32(PMIC_WRAP_MAN_EN ,DISABLE);
	WRAP_WR32(PMIC_WRAP_MUX_SEL ,WRAPPER_MODE);

timeout:
	WRAP_WR32(PMIC_WRAP_MAN_EN ,DISABLE);
	WRAP_WR32(PMIC_WRAP_MUX_SEL ,WRAPPER_MODE);
	return ret;
}

static S32 _pwrap_init_reg_clock( U32 regck_sel )
{
	U32 wdata=0;
	U32 rdata=0;
	PWRAPFUC();

	// Set reg clk freq
#ifdef SLV_6320
	pwrap_read_nochk(PMIC_TOP_CKCON2,&rdata);
#endif

#ifdef SLV_6320
	if( regck_sel == 1 )
		wdata = (rdata & (~(0x3<<10))) | (0x1<<10);
	else
		wdata = rdata & ~(0x3 << 10);
#elif defined SLV_6323
	if(regck_sel == 1) // not supported in 6323!!
		return E_PWR_INIT_REG_CLOCK;
	else
		wdata = 0x3;
#endif

#ifdef SLV_6320
	pwrap_write_nochk(PMIC_TOP_CKCON2, wdata);
	pwrap_read_nochk(PMIC_TOP_CKCON2, &rdata);
#elif defined SLV_6323
	pwrap_write_nochk(TOP_CKCON1_CLR, wdata);
	pwrap_read_nochk(TOP_CKCON1,  &rdata);
#endif

#ifdef SLV_6320
	if( rdata != wdata )
	{
		PWRAPERR("_pwrap_init_reg_clock,PMIC_TOP_CKCON2 Write [15]=1 Fail, rdata=%x\n",rdata);
		return E_PWR_WRITE_TEST_FAIL;
	}
#elif defined SLV_6323
	if( (rdata & 0x3) != 0)
	{
		PWRAPERR("_pwrap_init_reg_clock,PMIC_TOP_CKCON2 Write [15]=1 Fail, rdata=%x\n",rdata);
		return E_PWR_WRITE_TEST_FAIL;
	}
#endif

	// Set Dummy cycle for both 6320(assume 18MHz)/6323 (assume 12MHz)
#ifdef SLV_6320
	WRAP_WR32(PMIC_WRAP_RDDMY ,0x5);
#elif defined SLV_6323
	pwrap_write_nochk(DEW_RDDMY_NO, 0x8);
	WRAP_WR32(PMIC_WRAP_RDDMY ,0x8);
#endif

	// Config SPI Waveform according to reg clk
	if( regck_sel == 1 )
	{ // 6MHz in 6323  => no support ; 18MHz in 6320
		WRAP_WR32(PMIC_WRAP_CSHEXT_WRITE   , 0x4);  // wait data written into register => 3T_PMIC
		WRAP_WR32(PMIC_WRAP_CSHEXT_READ    , 0x5);  // for 6320, slave need enough time (4T of PMIC reg_ck) to back idle state
		WRAP_WR32(PMIC_WRAP_CSLEXT_START   , 0x0);
		WRAP_WR32(PMIC_WRAP_CSLEXT_END   , 0x0);
	} else if( regck_sel == 2 )
	{ // 12MHz in 6323; 36MHz in 6320
#ifdef SLV_6320
		WRAP_WR32(PMIC_WRAP_CSHEXT_READ  , 0x2);  // for 6320, slave need enough time (4T of PMIC reg_ck) to back idle state
		WRAP_WR32(PMIC_WRAP_CSHEXT_WRITE   , 0x2);
		WRAP_WR32(PMIC_WRAP_RDDMY    , 0x2);
#elif defined SLV_6323
		WRAP_WR32(PMIC_WRAP_CSHEXT_READ  , 0x0);
		WRAP_WR32(PMIC_WRAP_CSHEXT_WRITE   , 0x5);  // wait data written into register => 3T_PMIC: consists of CSLEXT_END(1T) + CSHEXT(6T)
#endif
		WRAP_WR32(PMIC_WRAP_CSLEXT_START   , 0x0);
		WRAP_WR32(PMIC_WRAP_CSLEXT_END   ,0x0);
	} else
	{ //Safe mode
		WRAP_WR32(PMIC_WRAP_CSHEXT_WRITE   , 0xf);
		WRAP_WR32(PMIC_WRAP_CSHEXT_READ    , 0xf);
		WRAP_WR32(PMIC_WRAP_CSLEXT_START   , 0xf);
		WRAP_WR32(PMIC_WRAP_CSLEXT_END   , 0xf);
	}

	return 0;
}

//--------------------------------------------------------
//    Function : DrvPWRAP_Switch_Strobe_Setting()
// Description : used to switch input data calibration setting before system sleep or after system wakeup
//               no use since SPI_CK (26MHz) is always kept unchanged
//   Parameter :
//      Return :
//--------------------------------------------------------
//void DrvPWRAP_Switch_Strobe_Setting (int si_ck_con, int sidly)
//{
//  int reg_rdata;
//  // turn off spi_wrap
//  *PMIC_WRAP_WRAP_EN = 0;
//  // wait for WRAP to be in idle state
//  // and no remaining rdata to be received
//  do
//  {
//    reg_rdata = *PMIC_WRAP_WRAP_STA;
//  } while ( (GET_WRAP_FSM(reg_rdata) != 0) ||
//            (GET_WRAP_CH_DLE_RESTCNT(reg_rdata)) != 0 );
//
//  *PMIC_WRAP_SI_CK_CON = si_ck_con;
//  *PMIC_WRAP_SIDLY = sidly;
//
//  // turn on spi_wrap
//  *PMIC_WRAP_WRAP_EN = 1;
//}


/*
 *pmic_wrap init,init wrap interface
 *
 */
S32 pwrap_init(void)
{
	S32 sub_return=0;
	S32 sub_return1=0;
	//S32 ret=0;
	U32 rdata=0x0;
	U32 clk_sel = 0;
	U32 cg_mask = 0;
	U32 backup = 0;
	//U32 timeout=0;
	PWRAPFUC();
	//###############################
	//toggle PMIC_WRAP and pwrap_spictl reset
	//###############################
	// Turn off module clock
	cg_mask = ((1 << 20) | (1 << 27) | (1 << 28) | (1 << 29));
	backup = (~WRAP_RD32(CLK_SWCG_1)) & cg_mask; // backup for later turn on after reset
	WRAP_WR32(CLK_SETCG_1, cg_mask);
	// dummy read to add latency (to wait clock turning off)
	rdata = WRAP_RD32(PMIC_WRAP_SWRST);

	// Toggle module reset
	WRAP_WR32(PMIC_WRAP_SWRST, 1);
	rdata = WRAP_RD32(WDT_SWSYSRST);
	WRAP_WR32(WDT_SWSYSRST, (rdata | (0x1 << 11)) | (0x88 << 24));
	WRAP_WR32(WDT_SWSYSRST, (rdata & (~(0x1 << 11))) | (0x88 << 24));
	WRAP_WR32(PMIC_WRAP_SWRST, 0);

	// Turn on module clock
	WRAP_WR32(CLK_CLRCG_1, backup | (1 << 20)); // ensure cg for AP is off;

	// Turn on module clock dcm (in global_con)
	// WHQA_00014186: set PMIC bclk DCM default off due to HW issue
	// WRAP_WR32(CLK_SETCG_3, (1 << 2) | (1 << 1));
	WRAP_WR32(CLK_SETCG_3, (1 << 2));

	//###############################
	// Set SPI_CK_freq = 26MHz for both 6320/6323
	//###############################
	clk_sel = WRAP_RD32(CLK_SEL_0);
	WRAP_WR32(CLK_SEL_0, clk_sel | (0x3 << 24));

	//###############################
	//toggle PERI_PWRAP_BRIDGE reset
	//###############################
	//WRAP_SET_BIT(0x04,PERI_GLOBALCON_RST1);
	//WRAP_CLR_BIT(0x04,PERI_GLOBALCON_RST1);

	//###############################
	//Enable DCM
	//###############################
	WRAP_WR32(PMIC_WRAP_DCM_EN , ENABLE);
	WRAP_WR32(PMIC_WRAP_DCM_DBC_PRD ,DISABLE);
	//###############################
	//Enable 6320 option
	//###############################
#ifdef SLV_6320
	WRAP_WR32(PMIC_WRAP_OP_TYPE ,OP_TYPE_CSL);
	WRAP_WR32(PMIC_WRAP_MSB_FIRST , LSB);
#endif

	//###############################
	//Reset SPISLV
	//###############################
	sub_return=_pwrap_reset_spislv();
	if( sub_return != 0 )
	{
		PWRAPERR("error,_pwrap_reset_spislv fail,sub_return=%x\n",sub_return);
		return E_PWR_INIT_RESET_SPI;
	}
	//###############################
	// Enable WACS2
	//###############################
	WRAP_WR32(PMIC_WRAP_WRAP_EN,ENABLE);//enable wrap
	WRAP_WR32(PMIC_WRAP_HIPRIO_ARB_EN,WACS2); //Only WACS2
	WRAP_WR32(PMIC_WRAP_WACS2_EN,ENABLE);
	//###############################
	// Set Dummy cycle to make it the same at both AP side and PMIC side
	//###############################
	// (default value of 6320 dummy cycle is already 0x8)
#ifdef SLV_6323
	WRAP_WR32(PMIC_WRAP_RDDMY , 0xF);
#endif

	//###############################
	// Input data calibration flow
	//###############################
	sub_return = _pwrap_init_sistrobe();
	if( sub_return != 0 )
	{
		PWRAPERR("error,DrvPWRAP_InitSiStrobe fail,sub_return=%x\n",sub_return);
		return E_PWR_INIT_SIDLY;
	}

	//###############################
	// SPI Waveform Configuration
	//###############################
	//0:safe mode, 1:6MHz, 2:12MHz => no support 6MHz since the clock is too slow to transmit data (due to RDDMY's limit -> only 4'hf)
	sub_return = _pwrap_init_reg_clock(2);
	if( sub_return != 0)
	{
		PWRAPERR("error,_pwrap_init_reg_clock fail,sub_return=%x\n",sub_return);
		return E_PWR_INIT_REG_CLOCK;
	}

	//###############################
	// Enable PMIC dewrapper (only for 6320)
	// (May not be necessary, depending on S/W partition)
	//###############################
#ifdef SLV_6320
	sub_return= pwrap_write_nochk(PMIC_WRP_CKPDN,   0);//set dewrap clock bit
	sub_return1=pwrap_write_nochk(PMIC_WRP_RST_CON, 0);//clear dewrap reset bit
	if(( sub_return != 0 )||( sub_return1 != 0 ))
	{
		PWRAPERR("Enable PMIC fail, sub_return=%x sub_return1=%x\n", sub_return,sub_return1);
		return E_PWR_INIT_ENABLE_PMIC;
	}
#endif

	//###############################
	// Enable DIO mode
	//###############################
	sub_return = _pwrap_init_dio(ENABLE);
	if( sub_return != 0 )
	{
		PWRAPERR("_pwrap_init_dio test error,error code=%x, sub_return=%x\n", 0x11, sub_return);
		return E_PWR_INIT_DIO;
	}

	//###############################
	// Enable Encryption
	//###############################
	sub_return = _pwrap_init_cipher();
	if( sub_return != 0 )
	{
		PWRAPERR("Enable Encryption fail, return=%x\n", sub_return);
		return E_PWR_INIT_CIPHER;
	}

	//###############################
	// Write test using WACS2
	//###############################
	sub_return = pwrap_write_nochk(DEW_WRITE_TEST, WRITE_TEST_VALUE);
	sub_return1 = pwrap_read_nochk(DEW_WRITE_TEST,&rdata);
	if(( rdata != WRITE_TEST_VALUE )||( sub_return != 0 )||( sub_return1 != 0 ))
	{
		PWRAPERR("write test error,rdata=0x%x,exp=0xa55a,sub_return=0x%x,sub_return1=0x%x\n", rdata,sub_return,sub_return1);
		return E_PWR_INIT_WRITE_TEST;
	}

	//###############################
	// Signature Checking - Using Write Test Register
	// should be the last to modify WRITE_TEST
	//###############################
	//_pwrap_wacs2_nochk(1, DEW_WRITE_TEST, 0x5678, &rdata);
	//WRAP_WR32(PMIC_WRAP_SIG_ADR,DEW_WRITE_TEST);
	//WRAP_WR32(PMIC_WRAP_SIG_VALUE,0x5678);
	//WRAP_WR32(PMIC_WRAP_SIG_MODE, 0x1);

	//###############################
	// Signature Checking - Using CRC
	// should be the last to modify WRITE_TEST
	//###############################
	sub_return=pwrap_write_nochk(DEW_CRC_EN,ENABLE);
	if( sub_return != 0 )
	{
		PWRAPERR("enable CRC fail,sub_return=%x\n", sub_return);
		return E_PWR_INIT_ENABLE_CRC;
	}
	WRAP_WR32(PMIC_WRAP_CRC_EN ,ENABLE);
	WRAP_WR32(PMIC_WRAP_SIG_MODE, CHECK_CRC);
	WRAP_WR32(PMIC_WRAP_SIG_ADR , DEW_CRC_VAL);


	//###############################
	// PMIC_WRAP enables
	//###############################
	WRAP_WR32(PMIC_WRAP_HIPRIO_ARB_EN,0x1ff);
	//WRAP_WR32(PMIC_WRAP_RRARB_EN ,0x7);
	WRAP_WR32(PMIC_WRAP_WACS0_EN,ENABLE);
	WRAP_WR32(PMIC_WRAP_WACS1_EN,ENABLE);
	//WRAP_WR32(PMIC_WRAP_WACS2_EN,0x1);//already enabled
	// WRAP_WR32(PMIC_WRAP_EVENT_IN_EN,0x1);
	//WRAP_WR32(PMIC_WRAP_EVENT_DST_EN,0xffff);
	WRAP_WR32(PMIC_WRAP_STAUPD_PRD, 0x5);  //0x1:20us,for concurrence test,MP:0x5;  //100us
	WRAP_WR32(PMIC_WRAP_STAUPD_GRPEN,0xff);
	WRAP_WR32(PMIC_WRAP_WDT_UNIT,0xf);
	WRAP_WR32(PMIC_WRAP_WDT_SRC_EN,0xffffffff);
	WRAP_WR32(PMIC_WRAP_TIMER_EN,0x1);
	WRAP_WR32(PMIC_WRAP_INT_EN,0x7ffffffd); //except for [31] debug_int

	//###############################
	// GPS_INTF initialization
	//###############################
#ifdef SLV_6323
	WRAP_WR32(PMIC_WRAP_ADC_CMD_ADDR    , AUXADC_CON21);
	WRAP_WR32(PMIC_WRAP_PWRAP_ADC_CMD   , 0x8000);
	WRAP_WR32(PMIC_WRAP_ADC_RDY_ADDR    , AUXADC_ADC12);
	WRAP_WR32(PMIC_WRAP_ADC_RDATA_ADDR1 , AUXADC_ADC13);
	WRAP_WR32(PMIC_WRAP_ADC_RDATA_ADDR2 , AUXADC_ADC14);
#endif

	//###############################
	// Initialization Done
	//###############################
	WRAP_WR32(PMIC_WRAP_INIT_DONE2 , 0x1);

	//###############################
	//TBD: Should be configured by MD MCU
	//###############################
#if 1 //CONFIG_MTK_LDVT_PMIC_WRAP
	WRAP_WR32(PMIC_WRAP_INIT_DONE0 ,1);
	WRAP_WR32(PMIC_WRAP_INIT_DONE1 , 1);
#endif
	return 0;
}
//EXPORT_SYMBOL(pwrap_init);

/*Interrupt handler function*/
static irqreturn_t mt_pmic_wrap_irq(int irqno, void *dev_id)
{
	unsigned long flags = 0;
	unsigned int int_flag = 0;

	PWRAPFUC();
	PWRAPREG("dump pwrap register\n");
	spin_lock_irqsave(&wrp_lock_isr, flags);
	//*-----------------------------------------------------------------------
	pwrap_dump_ap_register();
	//raise the priority of WACS2 for AP
	WRAP_WR32(PMIC_WRAP_HARB_HPRIO, 1 << 3);

	//*-----------------------------------------------------------------------
	int_flag = WRAP_RD32(PMIC_WRAP_INT_FLG);

	//clear interrupt flag
	WRAP_WR32(PMIC_WRAP_INT_CLR, 0xffffffff);
	spin_unlock_irqrestore(&wrp_lock_isr, flags);

	PWRAPERR("Interrupt has been raised!! 0x%x\n", int_flag);
	BUG_ON(1);

	return IRQ_HANDLED;
}

#ifndef USER_BUILD_KERNEL
static void pwrap_read_reg_on_ap(U32 reg_addr)
{
	U32 reg_value=0;
	reg_value=WRAP_RD32(reg_addr);
	PWRAPREG("0x%x=0x%x\n",reg_addr,reg_value);
}

static void pwrap_write_reg_on_ap(U32 reg_addr,U32 reg_value)
{
	PWRAPREG("write 0x%x to register 0x%x\n",reg_value,reg_addr);
	WRAP_WR32(reg_addr,reg_value);
	reg_value=WRAP_RD32(reg_addr);
	PWRAPREG("the result:0x%x=0x%x\n",reg_addr,reg_value);
}

static void pwrap_read_reg_on_pmic(U32 reg_addr)
{
	U32 reg_value=0;
	U32 return_value=0;
	//PWRAPFUC();
	return_value=pwrap_read(reg_addr, &reg_value);
	PWRAPREG("0x%x=0x%x,return_value=%x\n",reg_addr,reg_value,return_value);
}

static void pwrap_write_reg_on_pmic(U32 reg_addr,U32 reg_value)
{
	U32 return_value=0;
	PWRAPREG("write 0x%x to register 0x%x\n",reg_value,reg_addr);
	return_value=pwrap_write(reg_addr, reg_value);
	return_value=pwrap_read(reg_addr, &reg_value);
	//PWRAPFUC();
	PWRAPREG("the result:0x%x=0x%x,return_value=%x\n",reg_addr,reg_value,return_value);
}
#endif

static U32 pwrap_read_test(void)
{
	U32 rdata=0;
	U32 return_value=0;
	// Read Test
	return_value=pwrap_read(DEW_READ_TEST,&rdata);
	if( rdata != DEFAULT_VALUE_READ_TEST )
	{
		PWRAPREG("Read Test fail,rdata=0x%x, exp=0x5aa5,return_value=0x%x\n", rdata,return_value);
		return E_PWR_READ_TEST_FAIL;
	}
	else
	{
		PWRAPREG("Read Test pass,return_value=%d\n",return_value);
		return 0;
	}
}
static U32 pwrap_write_test(void)
{
	U32 rdata=0;
	U32 sub_return=0;
	U32 sub_return1=0;
	//###############################
	// Write test using WACS2
	//###############################
	sub_return = pwrap_write(DEW_WRITE_TEST, WRITE_TEST_VALUE);
	PWRAPREG("after pwrap_write\n");
	sub_return1 = pwrap_read(DEW_WRITE_TEST,&rdata);
	if(( rdata != WRITE_TEST_VALUE )||( sub_return != 0 )||( sub_return1 != 0 ))
	{
		PWRAPREG("write test error,rdata=0x%x,exp=0xa55a,sub_return=0x%x,sub_return1=0x%x\n", rdata,sub_return,sub_return1);
		return E_PWR_INIT_WRITE_TEST;
	}
	else
	{
		PWRAPREG("write Test pass\n");
		return 0;
	}
}

#define WRAP_ACCESS_TEST_REG DEW_WRITE_TEST
static void pwrap_wacs2_para_test(void)
{
	U32 return_value=0;
	U32 result=0;
	U32 rdata=0;
	//test 1st parameter--------------------------------------------
	return_value=pwrap_wacs2(3, WRAP_ACCESS_TEST_REG, 0x1234, &rdata);
	if( return_value != 0 )
	{
		PWRAPREG("pwrap_wacs2_para_test 1st para,return_value=%x\n", return_value);
		result+=1;
	}
	//test 2nd parameter--------------------------------------------
	return_value=pwrap_wacs2(0, 0xffff+0x10, 0x1234, &rdata);
	if( return_value != 0 )
	{
		PWRAPREG("pwrap_wacs2_para_test 2nd para,return_value=%x\n", return_value);
		result+=1;
	}
	//test 3rd parameter--------------------------------------------
	return_value=pwrap_wacs2(0, WRAP_ACCESS_TEST_REG, 0xffff+0x10, &rdata);
	if( return_value != 0 )
	{
		PWRAPREG("pwrap_wacs2_para_test 3rd para,return_value=%x\n", return_value);
		result+=1;
	}
	//test 4th parameter--------------------------------------------
	return_value=pwrap_wacs2(0, WRAP_ACCESS_TEST_REG, 0x1234, 0);
	if( return_value != 0 )
	{
		PWRAPREG("pwrap_wacs2_para_test 4th para,return_value=%x\n", return_value);
		result+=1;
	}
	if(result==4)
		PWRAPREG("pwrap_wacs2_para_test pass\n");
	else
		PWRAPREG("pwrap_wacs2_para_test fail\n");
}

static void pwrap_ut(U32 ut_test)
{
	switch(ut_test)
	{
	case 1:
		pwrap_wacs2_para_test();
		break;
	case 2:
		//pwrap_wacs2_para_test();
		break;

	default:
		PWRAPREG ( "default test.\n" );
		break;
	}
	return;
}
/*---------------------------------------------------------------------------*/
static S32 mt_pwrap_show_hal(char *buf)
{
	PWRAPFUC();
	return snprintf(buf, PAGE_SIZE, "%s\n","no implement" );
}
/*---------------------------------------------------------------------------*/
static S32 mt_pwrap_store_hal(const char *buf,size_t count)
{
#ifndef USER_BUILD_KERNEL
	U32 reg_value=0;
	U32 reg_addr=0;
#endif
	U32 return_value=0;
	U32 ut_test=0;
	if(!strncmp(buf, "-h", 2))
	{
		PWRAPREG("PWRAP debug: [-dump_reg][-trace_wacs2][-init][-rdap][-wrap][-rdpmic][-wrpmic][-readtest][-writetest]\n");
		PWRAPREG("PWRAP UT: [1][2]\n");
	}
	//--------------------------------------pwrap debug-------------------------------------------------------------
	else if(!strncmp(buf, "-dump_reg", 9))
	{
		pwrap_dump_all_register();
	}
	else if(!strncmp(buf, "-trace_wacs2", 12))
	{
		//pwrap_trace_wacs2();
	}
	else if(!strncmp(buf, "-init", 5))
	{
		return_value=pwrap_init();
		if(return_value==0)
			PWRAPREG("pwrap_init pass,return_value=%d\n",return_value);
		else
			PWRAPREG("pwrap_init fail,return_value=%d\n",return_value);
	}
#ifndef USER_BUILD_KERNEL
	else if (!strncmp(buf, "-rdap", 5) && (1 == sscanf(buf+5, "%x", &reg_addr)))
	{
		pwrap_read_reg_on_ap(reg_addr);
	}
	else if (!strncmp(buf, "-wrap", 5) && (2 == sscanf(buf+5, "%x %x", &reg_addr,&reg_value)))
	{
		pwrap_write_reg_on_ap(reg_addr,reg_value);
	}
	else if (!strncmp(buf, "-rdpmic", 7) && (1 == sscanf(buf+7, "%x", &reg_addr)))
	{
		pwrap_read_reg_on_pmic(reg_addr);
	}
	else if (!strncmp(buf, "-wrpmic", 7) && (2 == sscanf(buf+7, "%x %x", &reg_addr,&reg_value)))
	{
		pwrap_write_reg_on_pmic(reg_addr,reg_value);
	}
	#ifdef PWRAP_TIME_LOG
	else if (!strncmp(buf, "-dump_log", 9))
	{
		pwrap_dump_time_log();
	}
	#endif
#endif
	else if(!strncmp(buf, "-readtest", 9))
	{
		pwrap_read_test();
	}
	else if(!strncmp(buf, "-writetest", 10))
	{
		pwrap_write_test();
	}
	//--------------------------------------pwrap UT-------------------------------------------------------------
	else if (!strncmp(buf, "-ut", 3) && (1 == sscanf(buf+3, "%d", &ut_test)))
	{
		pwrap_ut(ut_test);
	}else{
		PWRAPREG("wrong parameter\n");
	}
	return count;
}

#define PMIC_WRAP_SUSPEND_DEBUG
#ifdef PMIC_WRAP_SUSPEND_DEBUG
static int mt_pwrap_suspend_hal(void)
{
	u32 init_done0 = 0;
	u32 init_done1 = 0;
	u32 init_done2 = 0;
	u32 pattern = 0;

	PWRAPLOG("Suspend!\n");
	init_done0 = WRAP_RD32(PMIC_WRAP_INIT_DONE0);
	init_done1 = WRAP_RD32(PMIC_WRAP_INIT_DONE1);
	init_done2 = WRAP_RD32(PMIC_WRAP_INIT_DONE2);
	pattern = WRAP_RD32(PMIC_WRAP_SIG_VALUE);
	PWRAPLOG("init_done0: %d init_done1: %d init_done2: %d pattern: 0x%X\n",
		init_done0, init_done1, init_done2, pattern);

	WRAP_WR32(PMIC_WRAP_SIG_VALUE, 0x1234);

	return 0;
}

static void mt_pwrap_resume_hal(void)
{
	u32 init_done0 = 0;
	u32 init_done1 = 0;
	u32 init_done2 = 0;
	u32 pattern = 0;

	PWRAPLOG("Resume!\n");
	init_done0 = WRAP_RD32(PMIC_WRAP_INIT_DONE0);
	init_done1 = WRAP_RD32(PMIC_WRAP_INIT_DONE1);
	init_done2 = WRAP_RD32(PMIC_WRAP_INIT_DONE2);
	pattern = WRAP_RD32(PMIC_WRAP_SIG_VALUE);
	PWRAPLOG("init_done0: %d init_done1: %d init_done2: %d pattern: 0x%X\n",
		init_done0, init_done1, init_done2, pattern);

	WRAP_WR32(PMIC_WRAP_SIG_VALUE, 0xABCD);

	return;
}
#endif

#ifdef CONFIG_MT6572_FPGA
// only for early porting
static S32 _pwrap_status_update_test_porting(void)
{
	U32 ret = 0;
	U32 rdata = 0;
	volatile U32 delay = 1000*1000*1;
	PWRAPFUC();

	//disable signature interrupt
	WRAP_WR32(PMIC_WRAP_INT_EN, 0x0);
	pwrap_write(DEW_WRITE_TEST, WRITE_TEST_VALUE);
	WRAP_WR32(PMIC_WRAP_SIG_ADR, DEW_WRITE_TEST);
	WRAP_WR32(PMIC_WRAP_SIG_VALUE, 0xAA55);
	WRAP_WR32(PMIC_WRAP_SIG_MODE, CHECK_SIG);

	//pwrap_delay_us(5000);//delay 5 seconds
	while(delay--);

	rdata = WRAP_RD32(PMIC_WRAP_SIG_ERRVAL) & 0xFFFF;
	if (rdata != WRITE_TEST_VALUE)
	{
		PWRAPERR("_pwrap_status_update_test error,error code=%x, rdata=%x\n", 1, rdata);
		ret = E_PWR_NOT_INIT_DONE;
        }
	WRAP_WR32(PMIC_WRAP_SIG_VALUE, WRITE_TEST_VALUE);//the same as write test
	//clear sig_error interrupt flag bit
	WRAP_WR32(PMIC_WRAP_INT_CLR,1<<1);

	//enable signature interrupt
	WRAP_WR32(PMIC_WRAP_INT_EN, 0x7ffffffd);
	WRAP_WR32(PMIC_WRAP_SIG_MODE, CHECK_CRC);
	WRAP_WR32(PMIC_WRAP_SIG_ADR , DEW_CRC_VAL);
	return ret;
}

static void pwrap_init_kernel(void) // only for early porting
{
	int ret = 0;
	U32 res = 0;
	PWRAPFUC();

	ret = pwrap_init();
	if (ret == 0)
		PWRAPLOG("wrap_init test pass.\n");
	else {
		PWRAPLOG("error:wrap_init test fail.\n");
		return;
	}

	ret = _pwrap_status_update_test_porting();
	if (ret == 0)
		PWRAPLOG("wrapper_StatusUpdateTest pass.\n");
	else
		PWRAPLOG("error:wrapper_StatusUpdateTest fail.\n");

	return;
}
#endif

/*---------------------------------------------------------------------------*/
#define VERSION     "$Revision$"
static int __init mt_pwrap_init(void)
{
	S32 ret = 0;
	PWRAPLOG("HAL init: version %s\n", VERSION);
	mt_wrp = get_mt_pmic_wrap_drv();

	mt_wrp->wacs2_hal = pwrap_wacs2_hal;
	mt_wrp->store_hal = mt_pwrap_store_hal;
	mt_wrp->show_hal = mt_pwrap_show_hal;
	mt_wrp->suspend = mt_pwrap_suspend_hal;
	mt_wrp->resume = mt_pwrap_resume_hal;

	ret = request_irq(MT_PMIC_WRAP_IRQ_ID, mt_pmic_wrap_irq, IRQF_TRIGGER_HIGH, PMIC_WRAP_DEVICE,0);
	if (ret) {
		PWRAPERR("register IRQ failed (%d)\n", ret);
		return ret;
	}
#ifdef CONFIG_MT6572_FPGA
	pwrap_init_kernel();  //add for FPGA early porting PMIC_WRAP user in kernel only
#endif

	return ret;
}
postcore_initcall(mt_pwrap_init);
