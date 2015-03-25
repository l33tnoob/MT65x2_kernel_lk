#include <platform/mt_pwm.h>
#include <platform/mt_gpt.h>
//#include <platform/mt_utils.h>
#include <debug.h>
#include <platform/sync_write.h>

#define PWM_DEBUG
#ifdef PWM_DEBUG
#define PWMDBG(fmt, args ...) dprintf(INFO,"pwm %5d: " fmt, __LINE__,##args)
#else
#define PWMDBG(fmt, args ...)
#endif

#define PWMMSG(fmt, args ...)  dprintf(INFO, fmt, ##args)

//#define SETREG32(x, y)			((*(volatile u32*)(x)) = (u32)(y))
//#define CLRREG32(x, y)			((*(volatile u32*)(x)) &= ~((u32)(y)))
//#define MASKREG32(x, y, z)		((*(volatile u32*)(x)) &= (~((u32)(y))|(u32)(z)))
#define MASKREG32(x, y, z)  OUTREG32(x, (INREG32(x)&~(y))|(z))
//#define INREG32(x)			(*(volatile u32 *)(x))
typedef int irqreturn_t;

#ifdef OUTREG32
  #undef OUTREG32
  #define OUTREG32(x, y) mt65xx_reg_sync_writel(y, x)
#endif

enum {                                                                                     
	PWM_CON,                                                                              
	PWM_HDURATION,                                                                        
	PWM_LDURATION,                                                                        
	PWM_GDURATION,                                                                        
	PWM_BUF0_BASE_ADDR,                                                                   
	PWM_BUF0_SIZE,                                                                        
	PWM_BUF1_BASE_ADDR,                                                                   
	PWM_BUF1_SIZE,                                                                        
	PWM_SEND_DATA0,                                                                       
	PWM_SEND_DATA1,                                                                       
	PWM_WAVE_NUM,                                                                         
	PWM_DATA_WIDTH,       
	PWM_THRESH, 
	PWM_SEND_WAVENUM,                                                                     
	PWM_VALID                                                                             
}PWM_REG_OFF;

U32 PWM_register[PWM_NUM]={          
	(PWM_BASE+0x0010),     //PWM1 REGISTER BASE,   15 registers  
	(PWM_BASE+0x0050),     //PWM2 register base    15 registers                
	(PWM_BASE+0x0090),     //PWM3 register base    15 registers                
	(PWM_BASE+0x00d0),     //PWM4 register base    13 registers                
	(PWM_BASE+0x0110),     //PWM5 register base    13 registers                         
};

#define PWM_CLOCK_SET   (0x10003008)
#define PWM_CLOCK_CLR   (0x10003010)
#define PWM_SET_BITS(REG, BS)       ((*(volatile u32*)(REG)) |= (u32)(BS))
#define PWM_CLR_BITS(REG, BS)       ((*(volatile u32*)(REG)) &= ~((u32)(BS)))
void mt_pwm_power_on(U32 pwm_no)
{
	PWM_SET_BITS(PWM_CLOCK_CLR, (1<<9));
	PWM_SET_BITS(PWM_CLOCK_CLR, (1<<(0x2+pwm_no)));  //enable clock 

	PWMDBG("enable_clock PWM%d\n", (pwm_no+1));
	mdelay(100);
}

void mt_pwm_power_off (U32 pwm_no)
{
	PWM_SET_BITS(PWM_CLOCK_SET, (1<<(0x2+pwm_no)));  //disable clock 

	PWMDBG("disable_clock PWM%d\n", (pwm_no+1));
}

S32 mt_pwm_sel_pmic(U32 pwm_no)
{
	PWMDBG("mt_pwm_sel_pmic\n");
	return RSUCCESS;
}

S32 mt_pwm_sel_ap(U32 pwm_no)
{
	PWMDBG("mt_pwm_sel_ap\n");
	return RSUCCESS;
}

/*******************************************************
 *   Set PWM_ENABLE register bit to enable pwm1~PWM5
 *
 ********************************************************/
S32 mt_set_pwm_enable(U32 pwm_no)
{
	if ( pwm_no >= PWM_MAX ) {
		PWMDBG ( "pwm number is not between PWM1~PWM5(0~6)\n" );
		return -EEXCESSPWMNO;
	}

	DRV_SetReg32(PWM_ENABLE, 1 << pwm_no);

	return RSUCCESS;
}


/*******************************************************/
S32 mt_set_pwm_disable ( U32 pwm_no )
{
	DRV_ClrReg32 ( PWM_ENABLE, 1 << pwm_no );

	mdelay(1);

	return RSUCCESS;
}

/********************************************************/
S32 mt_get_pwm_enable(U32 pwm_no)
{
	int en;

	if ( pwm_no >= PWM_MAX ) {
		PWMDBG("pwm number is not between PWM1~PWM5.\n");
		return -EEXCESSPWMNO;
	}

	en = INREG32(PWM_ENABLE );
	en &= 1 << pwm_no;
	en >>= pwm_no;

	return en;
}

void mt_pwm_disable(U32 pwm_no, BOOL pmic_pad)
{
	mt_set_pwm_disable(pwm_no);
	mt_pwm_power_off(pwm_no);
}

void mt_set_pwm_enable_seqmode(void)
{
	DRV_SetReg32 ( PWM_ENABLE, 1 << PWM_ENABLE_SEQ_OFFSET );
}

void mt_set_pwm_disable_seqmode(void)
{
	DRV_ClrReg32 ( PWM_ENABLE,1 << PWM_ENABLE_SEQ_OFFSET );
}

S32 mt_set_pwm_test_sel(U32 val)  //val as 0 or 1
{
	if (val == TEST_SEL_TRUE)
		DRV_SetReg32 ( PWM_ENABLE, 1 << PWM_ENABLE_TEST_SEL_OFFSET );
	else if ( val == TEST_SEL_FALSE )
		DRV_ClrReg32 ( PWM_ENABLE, 1 << PWM_ENABLE_TEST_SEL_OFFSET );
	else
		goto err;	
	return RSUCCESS;

err:
	return -EPARMNOSUPPORT;
}

int mt_get_pwm_mode(U32 pwm_no)
{
	U32 reg_val, reg_con;
	int mode = -1;
	u32 con_mode, con_src;

	reg_con = PWM_register[pwm_no] + 4*PWM_CON;

	reg_val = INREG32(reg_con);
	if (reg_val & PWM_CON_OLD_MODE_MASK) {
		mode = 0;
	}else {
		reg_val = INREG32(PWM_ENABLE);
		if (reg_val & (1 << PWM_ENABLE_SEQ_OFFSET)) {
			mode = 4;
		}else {
			reg_val = INREG32(reg_con);
			con_mode = reg_val & PWM_CON_MODE_MASK;
			con_src = reg_val & PWM_CON_SRCSEL_MASK;
			if ((con_mode == PERIOD)&& (con_src == FIFO)) {
				mode = 1;
			}else if ((con_mode == RAND) && (con_src == MEMORY)) {
				mode = 3;
			}else if ((con_mode == PERIOD) && (con_src == MEMORY) )  {
				mode = 2;
			}else {
				PWMDBG("mode is invalid.\n");
				PWMDBG("PWM_CON_MODE is :0x%x, PWM_CON_SRCSEL is: 0x%x\n", con_mode, con_src);
			}
		}
	}

	return mode;

}

S32 mt_set_pwm_clk ( U32 pwm_no, U32 clksrc, U32 div )
{
	U32 reg_con;

	if ( pwm_no >= PWM_MAX ) {
		PWMDBG ( "pwm number excesses PWM_MAX \n" );
		return -EEXCESSPWMNO;
	}

	if ( div >= CLK_DIV_MAX ) {
		PWMDBG ("division excesses CLK_DIV_MAX\n");
		return -EPARMNOSUPPORT;
	}

	if (clksrc > CLK_BLOCK_BY_1625_OR_32K) {
		PWMDBG("clksrc excesses CLK_BLOCK_BY_1625_OR_32K\n");
		return -EPARMNOSUPPORT;
	}

	reg_con = PWM_register [pwm_no] + 4* PWM_CON;

	MASKREG32 ( reg_con, PWM_CON_CLKDIV_MASK, div );
	if (clksrc == CLK_BLOCK)
		DRV_ClrReg32 ( reg_con, 1 << PWM_CON_CLKSEL_OFFSET );
	else if (clksrc == CLK_BLOCK_BY_1625_OR_32K)
		DRV_SetReg32 ( reg_con, 1 << PWM_CON_CLKSEL_OFFSET );

	return RSUCCESS;
}

/****************************************************/
S32 mt_get_pwm_clk ( U32 pwm_no )
{
	S32 clk;
	U32 reg_con, reg_val;

	if ( pwm_no >= PWM_MAX) {
		PWMDBG ( "pwm number excesses PWM_MAX \n" );
		return -EEXCESSPWMNO;
	}

	reg_con = PWM_register[pwm_no] + 4*PWM_CON;

	reg_val = INREG32 (reg_con);

	clk = (reg_val & PWM_CON_CLKSEL_MASK) >> PWM_CON_CLKSEL_OFFSET;
	return clk;
}

/****************************************************/
S32 mt_get_pwm_div ( U32 pwm_no )
{
	S32 div;
	U32 reg_con, reg_val;

	if ( pwm_no >= PWM_MAX) {
		PWMDBG ( "pwm number excesses PWM_MAX \n" );
		return -EEXCESSPWMNO;
	}

	reg_con = PWM_register[pwm_no] + 4*PWM_CON;

	reg_val = INREG32 (reg_con);

	div = (reg_val & PWM_CON_CLKDIV_MASK) >> PWM_CON_CLKDIV_OFFSET;
	return div;
}

/****************************************************/
S32 mt_get_pwm_high ( U32 pwm_no )
{
	S32 high;
	U32 reg_high, reg_val;

	if ( pwm_no >= PWM_MAX) {
		PWMDBG ( "pwm number excesses PWM_MAX \n" );
		return -EEXCESSPWMNO;
	}

	reg_high = PWM_register[pwm_no] + 4*PWM_HDURATION;

	reg_val = INREG32 (reg_high);

	high = (reg_val & PWM_HDURATION) ;
	return high;
}

/****************************************************/
S32 mt_get_pwm_low ( U32 pwm_no )
{
	S32 low;
	U32 reg_low, reg_val;

	if ( pwm_no >= PWM_MAX) {
		PWMDBG ( "pwm number excesses PWM_MAX \n" );
		return -EEXCESSPWMNO;
	}

	reg_low = PWM_register[pwm_no] + 4*PWM_LDURATION;

	reg_val = INREG32 (reg_low);

	low = (reg_val & PWM_LDURATION) ;
	return low;
}

/****************************************************/
S32 mt_get_pwm_grd ( U32 pwm_no )
{
	S32 grd;
	U32 reg_grd, reg_val;

	if ( pwm_no >= PWM_MAX) {
		PWMDBG ( "pwm number excesses PWM_MAX \n" );
		return -EEXCESSPWMNO;
	}

	reg_grd = PWM_register[pwm_no] + 4*PWM_GDURATION;

	reg_val = INREG32 (reg_grd);

	grd = (reg_val & PWM_LDURATION) ;
	return grd;
}

/****************************************************/
S32 mt_get_pwm_grdval ( U32 pwm_no )
{
	S32 grdval;
	U32 reg_con, reg_val;

	if ( pwm_no >= PWM_MAX) {
		PWMDBG ( "pwm number excesses PWM_MAX \n" );
		return -EEXCESSPWMNO;
	}

	reg_con = PWM_register[pwm_no] + 4*PWM_CON;

	reg_val = INREG32 (reg_con);

	grdval = (reg_val & PWM_CON_GUARD_VALUE_MASK) >>PWM_CON_GUARD_VALUE_OFFSET;
	return grdval;
}


/****************************************************/
S32 mt_get_pwm_idlval ( U32 pwm_no )
{
	S32 idlval;
	U32 reg_con, reg_val;

	if ( pwm_no >= PWM_MAX) {
		PWMDBG ( "pwm number excesses PWM_MAX \n" );
		return -EEXCESSPWMNO;
	}

	reg_con = PWM_register[pwm_no] + 4*PWM_CON;

	reg_val = INREG32 (reg_con);

	idlval = (reg_val & PWM_CON_IDLE_VALUE_MASK) >>PWM_CON_IDLE_VALUE_OFFSET;
	return idlval;
}

/******************************************
 * Set PWM_CON register data source
 * pwm_no: pwm1~PWM5(0~6)
 *val: 0 is fifo mode
 *       1 is memory mode
 *******************************************/

S32 mt_set_pwm_con_datasrc ( U32 pwm_no, U32 val )
{
	U32 reg_con;

	if ( pwm_no >= PWM_MAX ) {
		PWMDBG ("pwm number excesses PWM_MAX \n");
		return -EEXCESSPWMNO;
	}

	reg_con = PWM_register[pwm_no] + 4*PWM_CON;

	if ( val == FIFO )
		DRV_ClrReg32 ( reg_con, 1 << PWM_CON_SRCSEL_OFFSET );
	else if ( val == MEMORY )
		DRV_SetReg32 ( reg_con, 1 << PWM_CON_SRCSEL_OFFSET );
	else 
		goto err;
	return RSUCCESS;

err:
	return -EPARMNOSUPPORT;
}


/************************************************
 *  set the PWM_CON register
 * pwm_no : pwm1~PWM5 (0~6)
 * val: 0 is period mode
 *        1 is random mode
 *
 ***************************************************/
S32 mt_set_pwm_con_mode( U32 pwm_no, U32 val )
{
	U32 reg_con;

	if ( pwm_no >= PWM_MAX ) {
		PWMDBG ("pwm number excesses PWM_MAX\n");
		return -EEXCESSPWMNO;
	}

	reg_con = PWM_register[pwm_no] + 4*PWM_CON;

	if ( val == PERIOD )
		DRV_ClrReg32 ( reg_con, 1 << PWM_CON_MODE_OFFSET );
	else if (val == RAND)
		DRV_SetReg32 ( reg_con, 1 << PWM_CON_MODE_OFFSET );
	else
		goto err;
	return RSUCCESS;

err:
	return -EPARMNOSUPPORT;
}

/***********************************************
 *Set PWM_CON register, idle value bit 
 * val: 0 means that  idle state is not put out.
 *       1 means that idle state is put out
 *
 *      IDLE_FALSE: 0
 *      IDLE_TRUE: 1
 ***********************************************/
S32 mt_set_pwm_con_idleval(U32 pwm_no, U16 val)
{
	U32 reg_con;

	if ( pwm_no >= PWM_MAX ) {
		PWMDBG ( "pwm number excesses PWM_MAX \n" );
		return -EEXCESSPWMNO;
	}

	reg_con = PWM_register[pwm_no] + 4*PWM_CON;

	if ( val == IDLE_TRUE )
		DRV_SetReg32 ( reg_con,1 << PWM_CON_IDLE_VALUE_OFFSET );
	else if ( val == IDLE_FALSE )
		DRV_ClrReg32 ( reg_con, 1 << PWM_CON_IDLE_VALUE_OFFSET );
	else 
		goto err;

	return RSUCCESS;

err:
	return -EPARMNOSUPPORT;
}

/*********************************************
 * Set PWM_CON register guardvalue bit
 *  val: 0 means guard state is not put out.
 *        1 mens guard state is put out.
 *
 *    GUARD_FALSE: 0
 *    GUARD_TRUE: 1
 **********************************************/
S32 mt_set_pwm_con_guardval(U32 pwm_no, U16 val)
{
	U32 reg_con;

	if ( pwm_no >= PWM_MAX ) {
		PWMDBG ("pwm number excesses PWM_MAX \n");
		return -EEXCESSPWMNO;
	}

	reg_con = PWM_register[pwm_no] + 4*PWM_CON;

	if ( val == GUARD_TRUE )
		DRV_SetReg32 ( reg_con, 1 << PWM_CON_GUARD_VALUE_OFFSET );
	else if ( val == GUARD_FALSE )
		DRV_ClrReg32 ( reg_con, 1 << PWM_CON_GUARD_VALUE_OFFSET );
	else 
		goto err;

	return RSUCCESS;

err:
	return -EPARMNOSUPPORT;
}

S32 mt_set_pwm_con_stpbit(U32 pwm_no, U32 stpbit, U32 srcsel )
{
	U32 reg_con;

	if ( pwm_no >= PWM_MAX ) {
		PWMDBG ( "pwm number excesses PWM_MAX \n" );
		return -EEXCESSPWMNO;
	}

	reg_con = PWM_register[pwm_no] + 4*PWM_CON;

	if (srcsel == FIFO) {
		if ( stpbit > 0x3f ) {
			PWMDBG ( "stpbit execesses the most of 0x3f in fifo mode\n" );
			return -EPARMNOSUPPORT;
		}
	}else if (srcsel == MEMORY){
		if ( stpbit > 0x1f) {
			PWMDBG ("stpbit excesses the most of 0x1f in memory mode\n");
			return -EPARMNOSUPPORT;
		}
	}

	if ( srcsel == FIFO )
		MASKREG32 ( reg_con, PWM_CON_STOP_BITS_MASK, stpbit << PWM_CON_STOP_BITS_OFFSET);
	if ( srcsel == MEMORY )
		MASKREG32 ( reg_con, PWM_CON_STOP_BITS_MASK & (0x1f << PWM_CON_STOP_BITS_OFFSET), stpbit << PWM_CON_STOP_BITS_OFFSET);

	return RSUCCESS;
}

/*****************************************************
 *Set PWM_CON register oldmode bit
 * val: 0 means disable oldmode
 *        1 means enable oldmode
 *
 *      OLDMODE_DISABLE: 0
 *      OLDMODE_ENABLE: 1
 ******************************************************/

S32 mt_set_pwm_con_oldmode ( U32 pwm_no, U32 val )
{
	U32 reg_con;

	if ( pwm_no >= PWM_MAX ) {
		PWMDBG ("pwm number excesses PWM_MAX \n");
		return -EEXCESSPWMNO;
	}

	reg_con = PWM_register[pwm_no] + 4*PWM_CON;

	if ( val == OLDMODE_DISABLE )
		DRV_ClrReg32 ( reg_con, 1 << PWM_CON_OLD_MODE_OFFSET );
	else if ( val == OLDMODE_ENABLE )
		DRV_SetReg32 ( reg_con, 1 << PWM_CON_OLD_MODE_OFFSET );
	else 
		goto err;

	return RSUCCESS;

err:
	return -EPARMNOSUPPORT;
}

/***********************************************************
 * Set PWM_HIDURATION register
 *
 *************************************************************/

S32 mt_set_pwm_HiDur(U32 pwm_no, U16 DurVal)  //only low 16 bits are valid
{
	U32 reg_HiDur;

	if ( pwm_no >= PWM_MAX ) {
		PWMDBG ( "pwm number excesses PWM_MAX\n" );
		return -EEXCESSPWMNO;
	}

	reg_HiDur = PWM_register[pwm_no]+4*PWM_HDURATION;

	DRV_SetReg32 ( reg_HiDur, DurVal);	

	return RSUCCESS;
}

/************************************************
 * Set PWM Low Duration register
 *************************************************/
S32 mt_set_pwm_LowDur (U32 pwm_no, U16 DurVal)
{
	U32 reg_LowDur;

	if ( pwm_no >= PWM_MAX ) {
		PWMDBG ("pwm number excesses PWM_MAX\n");
		return -EEXCESSPWMNO;
	}

	reg_LowDur = PWM_register[pwm_no] + 4*PWM_LDURATION;

	DRV_SetReg32 ( reg_LowDur, DurVal );

	return RSUCCESS;
}

/***************************************************
 * Set PWM_GUARDDURATION register
 * pwm_no: PWM1~PWM5(0~6)
 * DurVal:   the value of guard duration
 ****************************************************/
S32 mt_set_pwm_GuardDur ( U32 pwm_no, U16 DurVal )
{
	U32 reg_GuardDur;

	if ( pwm_no >= PWM_MAX ) {
		PWMDBG ("pwm number excesses PWM_MAX\n");
		return -EEXCESSPWMNO;
	}

	reg_GuardDur = PWM_register[pwm_no] + 4*PWM_GDURATION;

	DRV_SetReg32 ( reg_GuardDur, DurVal );

	return RSUCCESS;
}

/*****************************************************
 * Set pwm_buf0_addr register
 * pwm_no: pwm1~PWM5 (0~6)
 * addr: data address
 ******************************************************/
S32 mt_set_pwm_buf0_addr (U32 pwm_no, U32 addr )
{
	U32 reg_buff0_addr;

	if ( pwm_no >= PWM_MAX ) {
		PWMDBG ( "pwm number excesses PWM_MAX \n" );
		return -EEXCESSPWMNO;
	}

	reg_buff0_addr = PWM_register[pwm_no] + 4 * PWM_BUF0_BASE_ADDR;

	DRV_SetReg32 ( reg_buff0_addr, addr );

	return RSUCCESS;
}

/*****************************************************
 * Set pwm_buf0_size register
 * pwm_no: pwm1~PWM5 (0~6)
 * size: size of data
 ******************************************************/
S32 mt_set_pwm_buf0_size ( U32 pwm_no, U16 size)
{
	U32 reg_buff0_size;

	if ( pwm_no >= PWM_MAX ) {
		PWMDBG ( "pwm number excesses PWM_MAX \n" );
		return -EEXCESSPWMNO;
	}

	reg_buff0_size = PWM_register[pwm_no] + 4* PWM_BUF0_SIZE;

	DRV_SetReg32 ( reg_buff0_size, size );

	return RSUCCESS;

}

/*****************************************************
 * Set pwm_buf1_addr register
 * pwm_no: pwm1~PWM5 (0~6)
 * addr: data address
 ******************************************************/
S32 mt_set_pwm_buf1_addr (U32 pwm_no, U32 addr )
{
	U32 reg_buff1_addr;

	if ( pwm_no >= PWM_MAX ) {
		PWMDBG ( "pwm number excesses PWM_MAX \n" );
		return -EEXCESSPWMNO;
	}

	reg_buff1_addr = PWM_register[pwm_no] + 4 * PWM_BUF1_BASE_ADDR;

	DRV_SetReg32 ( reg_buff1_addr, addr );

	return RSUCCESS;
}

/*****************************************************
 * Set pwm_buf1_size register
 * pwm_no: pwm1~PWM5 (0~6)
 * size: size of data
 ******************************************************/
S32 mt_set_pwm_buf1_size ( U32 pwm_no, U16 size)
{
	U32 reg_buff1_size;

	if ( pwm_no >= PWM_MAX ) {
		PWMDBG ( "pwm number excesses PWM_MAX \n" );
		return -EEXCESSPWMNO;
	}

	reg_buff1_size = PWM_register[pwm_no] + 4* PWM_BUF1_SIZE;

	DRV_SetReg32 ( reg_buff1_size, size );

	return RSUCCESS;

}

/*****************************************************
 * Set pwm_send_data0 register
 * pwm_no: pwm1~PWM5 (0~6)
 * data: the data in the register
 ******************************************************/
S32 mt_set_pwm_send_data0 ( U32 pwm_no, U32 data )
{
	U32 reg_data0;

	if ( pwm_no >= PWM_MAX ) {
		PWMDBG ( "pwm number excesses PWM_MAX \n" );
		return -EEXCESSPWMNO;
	}

	reg_data0 = PWM_register[pwm_no] + 4 * PWM_SEND_DATA0;

	DRV_SetReg32 ( reg_data0, data );

	return RSUCCESS;
}

/*****************************************************
 * Set pwm_send_data1 register
 * pwm_no: pwm1~PWM5 (0~6)
 * data: the data in the register
 ******************************************************/
S32 mt_set_pwm_send_data1 ( U32 pwm_no, U32 data )
{
	U32 reg_data1;

	if ( pwm_no >= PWM_MAX ) {
		PWMDBG ( "pwm number excesses PWM_MAX \n" );
		return -EEXCESSPWMNO;
	}

	reg_data1 = PWM_register[pwm_no] + 4 * PWM_SEND_DATA1;

	DRV_SetReg32 ( reg_data1, data );

	return RSUCCESS;
}


/*****************************************************
 * Set pwm_wave_num register
 * pwm_no: pwm1~PWM5 (0~6)
 * num:the wave number
 ******************************************************/
S32 mt_set_pwm_wave_num ( U32 pwm_no, U16 num )
{
	U32 reg_wave_num;

	if ( pwm_no >= PWM_MAX ) {
		PWMDBG ( "pwm number excesses PWM_MAX\n" );
		return -EEXCESSPWMNO;
	}

	reg_wave_num = PWM_register[pwm_no] + 4 * PWM_WAVE_NUM;

	DRV_SetReg32 ( reg_wave_num, num );

	return RSUCCESS;
}
/*****************************************************
* Set pwm_data_width register. 
* This is only for old mode
* pwm_no: pwm1~PWM5 (0~6)
* width: set the guard value in the old mode
******************************************************/
S32 mt_set_pwm_data_width ( U32 pwm_no, U16 width )
{
	U32 reg_data_width;

	if ( pwm_no >= PWM_MAX ) {
		PWMDBG ( "pwm number excesses PWM_MAX\n" );
		return -EEXCESSPWMNO;
	}

	if (pwm_no > PWM3) {
		reg_data_width = PWM_register[pwm_no] + 4 * PWM_THRESH;
	} else {
		reg_data_width = PWM_register[pwm_no] + 4 * PWM_DATA_WIDTH;
	}

	DRV_SetReg32 ( reg_data_width, width );

	return RSUCCESS;
}

/*****************************************************
* Set pwm_thresh register
* pwm_no: pwm1~PWM5 (0~6)
* thresh:  the thresh of the wave
******************************************************/
S32 mt_set_pwm_thresh ( U32 pwm_no, U16 thresh )
{
	U32 reg_thresh;

	if ( pwm_no >= PWM_MAX ) {
		PWMDBG ( " pwm number excesses PWM_MAX \n");
		return -EEXCESSPWMNO;
	}

	if (pwm_no > PWM3) {
		reg_thresh = PWM_register[pwm_no] + 4 * PWM_SEND_WAVENUM;
	} else {
		reg_thresh = PWM_register[pwm_no] + 4 * PWM_THRESH;
	}

	DRV_SetReg32 ( reg_thresh, thresh );

	return RSUCCESS;
}
/*****************************************************
 * Set pwm_send_wavenum register
 * pwm_no: pwm1~PWM5 (0~6)
 *
 ******************************************************/
S32 mt_get_pwm_send_wavenum ( U32 pwm_no )
{
	U32 reg_send_wavenum;
	S32 wave_num;

	if ( pwm_no >= PWM_MAX ) {
		PWMDBG ( "pwm number excesses PWM_MAX\n" );
		return -EEXCESSPWMNO;
	}

	if ( pwm_no <=PWM3 )
		reg_send_wavenum = PWM_register[pwm_no] + 4 * PWM_SEND_WAVENUM;
	else
		reg_send_wavenum = PWM_register[pwm_no] + 4 * (PWM_SEND_WAVENUM - 2);  

	wave_num = INREG32 ( reg_send_wavenum );

	return wave_num;
}

/*************************************************
*  set PWM4_delay when using SEQ mode
*
**************************************************/
S32 mt_set_pwm_delay_duration(U32 pwm_delay_reg, U16 val)
{
	MASKREG32 ( pwm_delay_reg, PWM_DELAY_DURATION_MASK, val );

	return RSUCCESS;
}

/*******************************************************
* Set pwm delay clock
* 
*
********************************************************/
S32 mt_set_pwm_delay_clock (U32 pwm_delay_reg, U32 clksrc)
{
	MASKREG32 (pwm_delay_reg, PWM_DELAY_CLK_MASK, clksrc );

	return RSUCCESS;
}

/*****************************************************
 * Set pwm_send_data1 register
 * pwm_no: pwm1~PWM5 (0~6)
 * buf_valid_bit: 
 * for buf0: bit0 and bit1 should be set 1. 
 * for buf1: bit2 and bit3 should be set 1.
 ******************************************************/
S32 mt_set_pwm_valid ( U32 pwm_no, U32 buf_valid_bit )   //set 0  for BUF0 bit or set 1 for BUF1 bit
{
	U32 reg_valid;

	if ( pwm_no >= PWM_MAX ) {
		PWMDBG ( "pwm number excesses PWM_MAX\n" );
		return -EEXCESSPWMNO;
	}

	if ( !buf_valid_bit>= BUF_EN_MAX) {
		PWMDBG ( "inavlid bit \n" );
		return -EPARMNOSUPPORT;
	}

	if (pwm_no <= PWM3)
		reg_valid = PWM_register[pwm_no] + 4 * PWM_VALID;
	else
		reg_valid = PWM_register[pwm_no] + 4* (PWM_VALID -2);

	DRV_SetReg32 ( reg_valid, 0x3 << (buf_valid_bit *2));

	return RSUCCESS;
}
S32 mt_set_pwm_invalid ( U32 pwm_no, U32 buf_valid_bit )   //set 0  for BUF0 bit or set 1 for BUF1 bit
{
	U32 reg_valid;

	if ( pwm_no >= PWM_MAX ) {
		PWMDBG ( "pwm number excesses PWM_MAX\n" );
		return -EEXCESSPWMNO;
	}

	if ( !buf_valid_bit>= BUF_EN_MAX) {
		PWMDBG ( "inavlid bit \n" );
		return -EPARMNOSUPPORT;
	}

	if ( pwm_no <= PWM3)
		reg_valid = PWM_register[pwm_no] + 4 * PWM_VALID;
	else
		reg_valid = PWM_register[pwm_no] + 4* (PWM_VALID -2);

	DRV_ClrReg32 ( reg_valid, 0x1 << (buf_valid_bit * 2));

	return RSUCCESS;
}

S32 mt_get_pwm_valid ( U32 pwm_no, U32 buf_valid_bit )   //set 0  for BUF0 bit or set 1 for BUF1 bit
{
	U32 reg_valid;
	int ret;

	if ( pwm_no >= PWM_MAX ) {
		PWMDBG ( "pwm number excesses PWM_MAX\n" );
		return -EEXCESSPWMNO;
	}

	if ( !buf_valid_bit>= BUF_EN_MAX) {
		PWMDBG ( "inavlid bit \n" );
		return -EPARMNOSUPPORT;
	}

	if ( pwm_no <= PWM3)
		reg_valid = PWM_register[pwm_no] + 4 * PWM_VALID;
	else
		reg_valid = PWM_register[pwm_no] + 4* (PWM_VALID -2);

	ret = INREG32 ( reg_valid );
	ret = ( ret >> (buf_valid_bit * 2)) & 0x01;

	return ret;
}

/*******************************************
 * Set intr enable register
 * pwm_intr_enable_bit: the intr bit, 
 *
 *********************************************/
S32 mt_set_intr_enable(U32 pwm_intr_enable_bit)
{

	if (pwm_intr_enable_bit >= PWM_INT_ENABLE_BITS_MAX) {
		PWMDBG (" pwm inter enable bit is not right.\n"); 
		return -EEXCESSBITS; 
	}

	DRV_SetReg32 ( PWM_INT_ENABLE, 1 << pwm_intr_enable_bit );

	return RSUCCESS;
}

S32 mt_set_intr_disable(U32 pwm_intr_enable_bit)
{

	if (pwm_intr_enable_bit >= PWM_INT_ENABLE_BITS_MAX) {
		PWMDBG (" pwm inter enable bit is not right.\n"); 
		return -EEXCESSBITS; 
	}

	DRV_ClrReg32 ( PWM_INT_ENABLE, 1 << pwm_intr_enable_bit );

	return RSUCCESS;
}

/*****************************************************
 * Set intr status register
 * pwm_no: pwm1~PWM5 (0~6)
 * pwm_intr_status_bit
 ******************************************************/
S32 mt_get_intr_status(U32 pwm_intr_status_bit)
{
	int ret;

	if ( pwm_intr_status_bit >= PWM_INT_STATUS_BITS_MAX ) {
		PWMDBG ( "status bit excesses PWM_INT_STATUS_BITS_MAX\n" );
		return -EEXCESSBITS;
	}

	ret = INREG32 ( PWM_INT_STATUS );
	ret = ( ret >> pwm_intr_status_bit ) & 0x01;

	return ret;
}

/*****************************************************
 * Set intr ack register
 * pwm_no: pwm1~PWM5 (0~6)
 * pwm_intr_ack_bit
 ******************************************************/
S32 mt_set_intr_ack ( U32 pwm_intr_ack_bit )
{

	if ( pwm_intr_ack_bit >= PWM_INT_ACK_BITS_MAX ) {
		PWMDBG ( "ack bit excesses PWM_INT_ACK_BITS_MAX\n" ); 
		return -EEXCESSBITS;
	}

	DRV_SetReg32 ( PWM_INT_ACK, 1 << pwm_intr_ack_bit );

	return RSUCCESS;
}

S32 pwm_set_easy_config ( struct pwm_easy_config *conf)
{

	U32 duty = 0;
	U16 duration = 0;
	U32 data_AllH=0xffffffff;
	U32 data0 = 0;
	U32 data1 = 0;
	
	if ( conf->pwm_no >= PWM_MAX ) {
		PWMDBG("pwm number excess PWM_MAX\n");
		return -EEXCESSPWMNO;
	}

	if (conf->clk_div >= CLK_DIV_MAX) {
		PWMDBG ( "PWM clock division invalid\n" );
		return -EINVALID;
	}
	
	if ( conf ->clk_src >= PWM_CLK_SRC_INVALID ) {
		PWMDBG ("PWM clock source invalid\n");
		return -EINVALID;
	}

	PWMDBG("pwm_set_easy_config\n");

	if (conf->pmic_pad){
		mt_pwm_sel_pmic(conf->pwm_no);
	}

	if ( conf->duty == 0 ) {
		mt_set_pwm_disable (conf->pwm_no);
		mt_pwm_power_off(conf->pwm_no);
		return RSUCCESS;
	}
	
	duty = conf->duty;
	duration = conf->duration;
	
	switch ( conf->clk_src ) {
		case PWM_CLK_OLD_MODE_BLOCK:
		case PWM_CLK_OLD_MODE_32K:
			if ( duration > 8191 ) {
				PWMDBG ( "duration invalid parameter\n" );
				return -EPARMNOSUPPORT;
			}
			/*if ( conf->pwm_no == PWM6 || conf->pwm_no == PWM4 || conf->pwm_no == PWM5 ){
				PWMDBG ( "invalid parameters\n" );
				return -EPARMNOSUPPORT;
			}*/
			if ( duration < 10 ) 
				duration = 10;
			break;
			
		case PWM_CLK_NEW_MODE_BLOCK:
		case PWM_CLK_NEW_MODE_BLOCK_DIV_BY_1625:
			break;
		default:
			PWMDBG("invalid clock source\n");
			return -EPARMNOSUPPORT;
	}
	
	if ( duty > 100 ) 
		duty = 100;

	if ( duty > 50 ){
		data0 = data_AllH;
		data1 = data_AllH >> ((PWM_NEW_MODE_DUTY_TOTAL_BITS * (100 - duty ))/100 );
	}else {
		data0 = data_AllH >> ((PWM_NEW_MODE_DUTY_TOTAL_BITS * (50 - duty))/100);
		PWMDBG("DATA0 :0x%x\n",data0);
		data1 = 0;
	}

	//mt_pwm_power_on(PWM5);
	mt_pwm_power_on(conf->pwm_no);
	mt_set_pwm_con_guardval(conf->pwm_no, GUARD_TRUE);
	mt_set_intr_disable(conf->pwm_no * 2);
	mt_set_intr_disable(conf->pwm_no * 2 + 1);

	switch ( conf->clk_src ) {
		case PWM_CLK_OLD_MODE_32K:
			mt_set_pwm_con_oldmode(conf->pwm_no, OLDMODE_ENABLE);
			mt_set_pwm_clk ( conf->pwm_no, CLK_BLOCK_BY_1625_OR_32K, conf->clk_div);
			break;

		case PWM_CLK_OLD_MODE_BLOCK:
			mt_set_pwm_con_oldmode (conf->pwm_no, OLDMODE_ENABLE );
			mt_set_pwm_clk ( conf->pwm_no, CLK_BLOCK, conf->clk_div );
			break;

		case PWM_CLK_NEW_MODE_BLOCK:
			mt_set_pwm_con_oldmode (conf->pwm_no, OLDMODE_DISABLE );
			mt_set_pwm_clk ( conf->pwm_no, CLK_BLOCK , conf->clk_div );
			mt_set_pwm_con_datasrc( conf->pwm_no, FIFO);
			mt_set_pwm_con_stpbit ( conf->pwm_no, 0x3f, FIFO );
			break;

		case PWM_CLK_NEW_MODE_BLOCK_DIV_BY_1625:
			mt_set_pwm_con_oldmode (conf->pwm_no,  OLDMODE_DISABLE );
			mt_set_pwm_clk ( conf->pwm_no, CLK_BLOCK_BY_1625_OR_32K, conf->clk_div );
			mt_set_pwm_con_datasrc( conf->pwm_no, FIFO);
			mt_set_pwm_con_stpbit ( conf->pwm_no, 0x3f, FIFO );
			break;

		default:
			break;
		}
	mt_set_pwm_HiDur ( conf->pwm_no, duration );
	mt_set_pwm_LowDur (conf->pwm_no, duration );
	mt_set_pwm_GuardDur (conf->pwm_no, 0 );
	mt_set_pwm_buf0_addr (conf->pwm_no, 0 );
	mt_set_pwm_buf0_size( conf->pwm_no, 0 );
	mt_set_pwm_buf1_addr (conf->pwm_no, 0 );
	mt_set_pwm_buf1_size (conf->pwm_no, 0 );
	mt_set_pwm_send_data0 (conf->pwm_no, data0 );
	mt_set_pwm_send_data1 (conf->pwm_no, data1 );
	mt_set_pwm_wave_num (conf->pwm_no, 0 );

	//if ( conf->pwm_no <= PWM2||conf->pwm_no == PWM6)
	//{
		mt_set_pwm_data_width (conf->pwm_no, duration );
		mt_set_pwm_thresh ( conf->pwm_no, (( duration * conf->duty)/100));
//		mt_set_pwm_valid (conf->pwm_no, BUF0_EN_VALID );
//		mt_set_pwm_valid ( conf->pwm_no, BUF1_EN_VALID );
	//}

	mt_set_pwm_enable ( conf->pwm_no );
	PWMDBG("mt_set_pwm_enable\n");

	return RSUCCESS;
}

/*****************************************************
 * Clear intr ack register
 * pwm_no: pwm1~PWM5 (0~6)
 * pwm_intr_ack_bit
 ******************************************************/
S32 mt_clr_intr_ack ( U32 pwm_intr_ack_bit )
{
	if ( pwm_intr_ack_bit >= PWM_INT_ACK_BITS_MAX ) {
		PWMDBG ( "ack bit excesses PWM_INT_ACK_BITS_MAX\n" );
		return -EEXCESSBITS;
	}

	DRV_ClrReg32 ( PWM_INT_ACK, 1 << pwm_intr_ack_bit );

	return RSUCCESS;
}

S32 pwm_set_spec_config(struct pwm_spec_config *conf)
{
	if ( conf->pwm_no >= PWM_MAX ) {
		PWMDBG("pwm number excess PWM_MAX\n");
		return -EEXCESSPWMNO;
	}

	if (conf->mode >= PWM_MODE_INVALID) {
		PWMDBG ( "PWM mode invalid \n" );
		return -EINVALID;
	}

	if (conf ->clk_src >= PWM_CLK_SRC_INVALID) {
		PWMDBG ("PWM clock source invalid\n");
		return -EINVALID;
	}

	if (conf->clk_div >= CLK_DIV_MAX) {
		PWMDBG ( "PWM clock division invalid\n" );
		return -EINVALID;
	}

	/*if ( (( conf->pwm_no == PWM4 || conf->pwm_no == PWM5||conf->pwm_no == PWM6 )
				&& conf->mode == PWM_MODE_OLD)
			|| (conf->mode == PWM_MODE_OLD &&
				(conf->clk_src == PWM_CLK_NEW_MODE_BLOCK|| conf->clk_src == PWM_CLK_NEW_MODE_BLOCK_DIV_BY_1625)) 
			||(conf->mode != PWM_MODE_OLD &&
				(conf->clk_src == PWM_CLK_OLD_MODE_32K || conf->clk_src == PWM_CLK_OLD_MODE_BLOCK)) ) {

		PWMDBG ( "parameters match error\n" );
		return -ERROR;
	}*/

	if (conf->pmic_pad){
		mt_pwm_sel_pmic(conf->pwm_no);
	}

	//mt_pwm_power_on(PWM5);
	mt_pwm_power_on(conf->pwm_no);
	mt_set_intr_disable(conf->pwm_no * 2);
	mt_set_intr_disable(conf->pwm_no * 2 + 1);

	switch (conf->mode ) {
		case PWM_MODE_OLD:
			PWMDBG("PWM_MODE_OLD\n");
			mt_set_pwm_con_oldmode(conf->pwm_no, OLDMODE_ENABLE);
			mt_set_pwm_con_idleval(conf->pwm_no, conf->pwm_mode.PWM_MODE_OLD_REGS.IDLE_VALUE);
			mt_set_pwm_con_guardval (conf->pwm_no, conf->pwm_mode.PWM_MODE_OLD_REGS.GUARD_VALUE);
			mt_set_pwm_GuardDur (conf->pwm_no, conf->pwm_mode.PWM_MODE_OLD_REGS.GDURATION);
			mt_set_pwm_wave_num(conf->pwm_no, conf->pwm_mode.PWM_MODE_OLD_REGS.WAVE_NUM);
			mt_set_pwm_data_width(conf->pwm_no, conf->pwm_mode.PWM_MODE_OLD_REGS.DATA_WIDTH);
			mt_set_pwm_thresh(conf->pwm_no, conf->pwm_mode.PWM_MODE_OLD_REGS.THRESH);
			PWMDBG ("PWM set old mode finish\n");
			break;
		case PWM_MODE_FIFO:
			PWMDBG("PWM_MODE_FIFO\n");
			mt_set_pwm_con_oldmode(conf->pwm_no, OLDMODE_DISABLE);
			mt_set_pwm_con_datasrc(conf->pwm_no, FIFO);
			mt_set_pwm_con_mode (conf->pwm_no, PERIOD);
			mt_set_pwm_con_idleval(conf->pwm_no, conf->pwm_mode.PWM_MODE_FIFO_REGS.IDLE_VALUE);
			mt_set_pwm_con_guardval (conf->pwm_no, conf->pwm_mode.PWM_MODE_FIFO_REGS.GUARD_VALUE);
			mt_set_pwm_HiDur (conf->pwm_no, conf->pwm_mode.PWM_MODE_FIFO_REGS.HDURATION);
			mt_set_pwm_LowDur (conf->pwm_no, conf->pwm_mode.PWM_MODE_FIFO_REGS.LDURATION);
			mt_set_pwm_GuardDur (conf->pwm_no, conf->pwm_mode.PWM_MODE_FIFO_REGS.GDURATION);
			mt_set_pwm_send_data0 (conf->pwm_no, conf->pwm_mode.PWM_MODE_FIFO_REGS.SEND_DATA0);
			mt_set_pwm_send_data1 (conf->pwm_no, conf->pwm_mode.PWM_MODE_FIFO_REGS.SEND_DATA1);
			mt_set_pwm_wave_num(conf->pwm_no, conf->pwm_mode.PWM_MODE_FIFO_REGS.WAVE_NUM);
			mt_set_pwm_con_stpbit(conf->pwm_no, conf->pwm_mode.PWM_MODE_FIFO_REGS.STOP_BITPOS_VALUE,FIFO);
			break;
		/*case PWM_MODE_MEMORY:
			PWMDBG("PWM_MODE_MEMORY\n");
			mt_set_pwm_con_oldmode(conf->pwm_no, OLDMODE_DISABLE);
			mt_set_pwm_con_datasrc(conf->pwm_no, MEMORY);
			mt_set_pwm_con_mode (conf->pwm_no, PERIOD);
			mt_set_pwm_con_idleval(conf->pwm_no, conf->pwm_mode.PWM_MODE_MEMORY_REGS.IDLE_VALUE);
			mt_set_pwm_con_guardval (conf->pwm_no, conf->pwm_mode.PWM_MODE_MEMORY_REGS.GUARD_VALUE);
			mt_set_pwm_HiDur (conf->pwm_no, conf->pwm_mode.PWM_MODE_MEMORY_REGS.HDURATION);
			mt_set_pwm_LowDur (conf->pwm_no, conf->pwm_mode.PWM_MODE_MEMORY_REGS.LDURATION);
			mt_set_pwm_GuardDur (conf->pwm_no, conf->pwm_mode.PWM_MODE_MEMORY_REGS.GDURATION);
			mt_set_pwm_buf0_addr(conf->pwm_no, (U32)conf->pwm_mode.PWM_MODE_MEMORY_REGS.BUF0_BASE_ADDR);
			mt_set_pwm_buf0_size (conf->pwm_no, conf->pwm_mode.PWM_MODE_MEMORY_REGS.BUF0_SIZE);
			mt_set_pwm_wave_num(conf->pwm_no, conf->pwm_mode.PWM_MODE_MEMORY_REGS.WAVE_NUM);
			mt_set_pwm_con_stpbit(conf->pwm_no, conf->pwm_mode.PWM_MODE_MEMORY_REGS.STOP_BITPOS_VALUE,MEMORY);

			break;
		case PWM_MODE_RANDOM:
			PWMDBG("PWM_MODE_RANDOM\n");
			mt_set_pwm_disable(conf->pwm_no);
			mt_set_pwm_con_oldmode(conf->pwm_no, OLDMODE_DISABLE);
			mt_set_pwm_con_datasrc(conf->pwm_no, MEMORY);
			mt_set_pwm_con_mode (conf->pwm_no, RAND);
			mt_set_pwm_con_idleval(conf->pwm_no, conf->pwm_mode.PWM_MODE_RANDOM_REGS.IDLE_VALUE);
			mt_set_pwm_con_guardval (conf->pwm_no, conf->pwm_mode.PWM_MODE_RANDOM_REGS.GUARD_VALUE);
			mt_set_pwm_HiDur (conf->pwm_no, conf->pwm_mode.PWM_MODE_RANDOM_REGS.HDURATION);
			mt_set_pwm_LowDur (conf->pwm_no, conf->pwm_mode.PWM_MODE_RANDOM_REGS.LDURATION);
			mt_set_pwm_GuardDur (conf->pwm_no, conf->pwm_mode.PWM_MODE_RANDOM_REGS.GDURATION);
			mt_set_pwm_buf0_addr(conf->pwm_no, (U32 )conf->pwm_mode.PWM_MODE_RANDOM_REGS.BUF0_BASE_ADDR);
			mt_set_pwm_buf0_size (conf->pwm_no, conf->pwm_mode.PWM_MODE_RANDOM_REGS.BUF0_SIZE);
			mt_set_pwm_buf1_addr(conf->pwm_no, (U32 )conf->pwm_mode.PWM_MODE_RANDOM_REGS.BUF1_BASE_ADDR);
			mt_set_pwm_buf1_size (conf->pwm_no, conf->pwm_mode.PWM_MODE_RANDOM_REGS.BUF1_SIZE);
			mt_set_pwm_wave_num(conf->pwm_no, conf->pwm_mode.PWM_MODE_RANDOM_REGS.WAVE_NUM);
			mt_set_pwm_con_stpbit(conf->pwm_no, conf->pwm_mode.PWM_MODE_RANDOM_REGS.STOP_BITPOS_VALUE, MEMORY);
			mt_set_pwm_valid(conf->pwm_no, BUF0_EN_VALID);
			mt_set_pwm_valid(conf->pwm_no, BUF1_EN_VALID);
			break;

		case PWM_MODE_DELAY:
			PWMDBG("PWM_MODE_DELAY\n");
			mt_set_pwm_con_oldmode(conf->pwm_no, OLDMODE_DISABLE);
			mt_set_pwm_enable_seqmode();
			mt_set_pwm_disable(PWM2);
			mt_set_pwm_disable(PWM3);
			mt_set_pwm_disable(PWM4);
			mt_set_pwm_disable(PWM5);
			if ( conf->pwm_mode.PWM_MODE_DELAY_REGS.PWM4_DELAY_DUR <0 ||conf->pwm_mode.PWM_MODE_DELAY_REGS.PWM4_DELAY_DUR >= (1<<17) ||
					conf->pwm_mode.PWM_MODE_DELAY_REGS.PWM5_DELAY_DUR < 0|| conf->pwm_mode.PWM_MODE_DELAY_REGS.PWM5_DELAY_DUR >= (1<<17) ||
					conf->pwm_mode.PWM_MODE_DELAY_REGS.PWM6_DELAY_DUR <0 || conf->pwm_mode.PWM_MODE_DELAY_REGS.PWM6_DELAY_DUR >=(1<<17) ) {
				PWMDBG("Delay value invalid\n");
				return -EINVALID;
			}
			mt_set_pwm_delay_duration(PWM4_DELAY, conf->pwm_mode.PWM_MODE_DELAY_REGS.PWM4_DELAY_DUR );
			mt_set_pwm_delay_clock(PWM4_DELAY, conf->pwm_mode.PWM_MODE_DELAY_REGS.PWM4_DELAY_CLK);
			mt_set_pwm_delay_duration(PWM5_DELAY, conf->pwm_mode.PWM_MODE_DELAY_REGS.PWM5_DELAY_DUR);
			mt_set_pwm_delay_clock(PWM5_DELAY, conf->pwm_mode.PWM_MODE_DELAY_REGS.PWM5_DELAY_CLK);
			mt_set_pwm_delay_duration(PWM6_DELAY, conf->pwm_mode.PWM_MODE_DELAY_REGS.PWM6_DELAY_DUR);
			mt_set_pwm_delay_clock(PWM6_DELAY, conf->pwm_mode.PWM_MODE_DELAY_REGS.PWM6_DELAY_CLK);

			mt_set_pwm_enable(PWM6);
			mt_set_pwm_enable(PWM3);
			mt_set_pwm_enable(PWM4);
			mt_set_pwm_enable(PWM5);
			break;*/
		default:
			break;
	}

	switch (conf->clk_src) {
		case PWM_CLK_OLD_MODE_BLOCK:
			mt_set_pwm_clk (conf->pwm_no, CLK_BLOCK, conf->clk_div);
			PWMDBG("Enable oldmode and set clock block\n");
			break;
		case PWM_CLK_OLD_MODE_32K:
			mt_set_pwm_clk (conf->pwm_no, CLK_BLOCK_BY_1625_OR_32K, conf->clk_div);
			PWMDBG("Enable oldmode and set clock 32K\n");
			break;
		case PWM_CLK_NEW_MODE_BLOCK:
			mt_set_pwm_clk (conf->pwm_no, CLK_BLOCK, conf->clk_div);
			PWMDBG("Enable newmode and set clock block\n");
			break;
		case PWM_CLK_NEW_MODE_BLOCK_DIV_BY_1625:
			mt_set_pwm_clk (conf->pwm_no, CLK_BLOCK_BY_1625_OR_32K, conf->clk_div);
			PWMDBG("Enable newmode and set clock 32K\n");
			break;
		default:
			break;
	}

	if(conf->intr)
	{
		mt_set_intr_ack(conf->pwm_no * 2);
		mt_set_intr_enable(conf->pwm_no * 2);
	}

	mt_set_pwm_enable(conf->pwm_no);
	PWMDBG("mt_set_pwm_enable\n");

	return RSUCCESS;
}

void pwm_init()
{
	mt_pwm_power_on(PWM4);
	mt_pwm_power_on(PWM1);
	mt_pwm_power_on(PWM2);
	mt_pwm_power_on(PWM3);
	mt_pwm_power_on(PWM5);
	//	mt65xx_irq_set_sens(MT6575_PWM_IRQ_ID, MT65xx_EDGE_SENSITIVE);
	//	mt65xx_irq_set_polarity(MT6575_PWM_IRQ_ID, MT65xx_POLARITY_LOW);
}
