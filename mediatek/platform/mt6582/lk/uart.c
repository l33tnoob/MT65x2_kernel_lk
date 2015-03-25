/*
 * Copyright (c) 2008 Travis Geiselbrecht
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include <debug.h>
#include <reg.h>
#include <dev/uart.h>

#include <platform/mt_typedefs.h>
#include <platform/mt_reg_base.h>
#include <platform/mt_uart.h>
#include <platform/boot_mode.h>
#include <platform/mt_gpio.h>
#include <platform/sync_write.h>
#include <cust_gpio_usage.h>

#define CONFIG_BAUDRATE 		921600

#define UART_SET_BITS(BS,REG)       mt65xx_reg_sync_writel(DRV_Reg32(REG) | (u32)(BS), REG)
#define UART_CLR_BITS(BS,REG)       mt65xx_reg_sync_writel(DRV_Reg32(REG) & ~((u32)(BS)), REG)

#define UART_BASE(uart)					  (uart)

#define UART_RBR(uart)                    (UART_BASE(uart)+0x0)  /* Read only */
#define UART_THR(uart)                    (UART_BASE(uart)+0x0)  /* Write only */
#define UART_IER(uart)                    (UART_BASE(uart)+0x4)
#define UART_IIR(uart)                    (UART_BASE(uart)+0x8)  /* Read only */
#define UART_FCR(uart)                    (UART_BASE(uart)+0x8)  /* Write only */
#define UART_LCR(uart)                    (UART_BASE(uart)+0xc)
#define UART_MCR(uart)                    (UART_BASE(uart)+0x10)
#define UART_LSR(uart)                    (UART_BASE(uart)+0x14)
#define UART_MSR(uart)                    (UART_BASE(uart)+0x18)
#define UART_SCR(uart)                    (UART_BASE(uart)+0x1c)
#define UART_DLL(uart)                    (UART_BASE(uart)+0x0)  /* Only when LCR.DLAB = 1 */
#define UART_DLH(uart)                    (UART_BASE(uart)+0x4)  /* Only when LCR.DLAB = 1 */
#define UART_EFR(uart)                    (UART_BASE(uart)+0x8)  /* Only when LCR = 0xbf */
#define UART_XON1(uart)                   (UART_BASE(uart)+0x10) /* Only when LCR = 0xbf */
#define UART_XON2(uart)                   (UART_BASE(uart)+0x14) /* Only when LCR = 0xbf */
#define UART_XOFF1(uart)                  (UART_BASE(uart)+0x18) /* Only when LCR = 0xbf */
#define UART_XOFF2(uart)                  (UART_BASE(uart)+0x1c) /* Only when LCR = 0xbf */
#define UART_AUTOBAUD_EN(uart)            (UART_BASE(uart)+0x20)
#define UART_HIGHSPEED(uart)              (UART_BASE(uart)+0x24)
#define UART_SAMPLE_COUNT(uart)           (UART_BASE(uart)+0x28) 
#define UART_SAMPLE_POINT(uart)           (UART_BASE(uart)+0x2c) 
#define UART_AUTOBAUD_REG(uart)           (UART_BASE(uart)+0x30)
#define UART_RATE_FIX_AD(uart)            (UART_BASE(uart)+0x34)
#define UART_AUTOBAUD_SAMPLE(uart)        (UART_BASE(uart)+0x38)
#define UART_GUARD(uart)                  (UART_BASE(uart)+0x3c)
#define UART_ESCAPE_DAT(uart)             (UART_BASE(uart)+0x40)
#define UART_ESCAPE_EN(uart)              (UART_BASE(uart)+0x44)
#define UART_SLEEP_EN(uart)               (UART_BASE(uart)+0x48)
#define UART_VFIFO_EN(uart)               (UART_BASE(uart)+0x4c)
#define UART_RXTRI_AD(uart)               (UART_BASE(uart)+0x50)

//FIXME Disable for MT6582 LK Porting
#ifndef MACH_FPGA
#define __ENABLE_UART_LOG_SWITCH_FEATURE__
#endif

// output uart port
volatile unsigned int g_uart;
// output uart baudrate
unsigned int g_brg;

//extern unsigned int mtk_get_bus_freq(void);

#if defined(MACH_FPGA)
#define UART_SRC_CLK 12000000
#else
#define UART_SRC_CLK 26000000
#endif

#ifdef __ENABLE_UART_LOG_SWITCH_FEATURE__
extern BOOT_ARGUMENT *g_boot_arg;
extern unsigned int strlen(char *s);
extern unsigned int strncmp(char *str1, char *str2, int maxlen);
int get_uart_port_id(void)
{	
	unsigned int log_port;
	unsigned int log_enable;	

	log_port = g_boot_arg->log_port;
	log_enable = g_boot_arg->log_enable;
	if( (log_port == UART1)&&(log_enable != 0) )
		return 1;
	return 4;
}

static void change_uart_port(char * cmd_line, char new_val)
{
	int i;
	int len;
	char *ptr;
	if(NULL == cmd_line)
		return;

	len = strlen(cmd_line);
	ptr = cmd_line;

	i = strlen("ttyMT");
	if(len < i)
		return;
	len = len-i;

	for(i=0; i<=len; i++)
	{
		if(strncmp(ptr, "ttyMT", 5)==0)
		{
			ptr[5] = new_val; // Find and modify
			break;
		}
		ptr++;
	}
}
void custom_port_in_kernel(BOOTMODE boot_mode, char *command)
{
	if(get_uart_port_id() == 1){
		change_uart_port(command, '0');
	}
}

#else
void custom_port_in_kernel(BOOTMODE boot_mode, char *command)
{
	// Dummy function case
}

int get_uart_port_id(void)
{
	// Dummy function case
}
#endif

int mtk_uart_power_on(MTK_UART uart)
{
    //FIXME Disable for MT6582 LK Porting
    return 0;
    /* UART Powr PDN and Reset*/
    #define AP_PERI_GLOBALCON_RST0 (PERI_CON_BASE+0x0)
    #define AP_PERI_GLOBALCON_PDN0 (PERI_CON_BASE+0x10)
    if (uart == UART1)
        UART_CLR_BITS(1 << 24, AP_PERI_GLOBALCON_PDN0); /* Power on UART1 */
    else if (uart == UART4)
        UART_CLR_BITS(1 << 27, AP_PERI_GLOBALCON_PDN0); /* Power on UART4 */
    return 0;  
}

void uart_setbrg()
{
	unsigned int byte,speed;
	unsigned int highspeed;
	unsigned int quot, divisor, remainder;
	unsigned int uartclk;
	unsigned short data, high_speed_div, sample_count, sample_point;
	unsigned int tmp_div;

	speed = g_brg;
        ////FIXME Disable for MT6582 LK Porting
        uartclk = UART_SRC_CLK;
	//uartclk = (unsigned int)(mtk_get_bus_freq()*1000/4);
	if (speed <= 115200 ) {
		highspeed = 0;
		quot = 16;
	} else {
		highspeed = 3;
		quot = 1;
	}

	if (highspeed < 3) { /*0~2*/
		/* Set divisor DLL and DLH	*/			   
		divisor   =  uartclk / (quot * speed);
		remainder =  uartclk % (quot * speed);
		  
		if (remainder >= (quot / 2) * speed)
			divisor += 1;

		mt65xx_reg_sync_writew(highspeed, UART_HIGHSPEED(g_uart));
		byte = DRV_Reg32(UART_LCR(g_uart));	  /* DLAB start */
		mt65xx_reg_sync_writel((byte | UART_LCR_DLAB), UART_LCR(g_uart));
		mt65xx_reg_sync_writel((divisor & 0x00ff), UART_DLL(g_uart));
		mt65xx_reg_sync_writel(((divisor >> 8)&0x00ff), UART_DLH(g_uart));
		mt65xx_reg_sync_writel(byte, UART_LCR(g_uart));	  /* DLAB end */
	}
	else {
		data=(unsigned short)(uartclk/speed);
		high_speed_div = (data>>8) + 1; // divided by 256

		tmp_div=uartclk/(speed*high_speed_div);
		divisor =  (unsigned short)tmp_div;

		remainder = (uartclk)%(high_speed_div*speed);
		/*get (sample_count+1)*/
		if (remainder >= ((speed)*(high_speed_div))>>1)
			divisor =  (unsigned short)(tmp_div+1);
		else
			divisor =  (unsigned short)tmp_div;
		
		sample_count=divisor-1;
		
		/*get the sample point*/
		sample_point=(sample_count-1)>>1;
		
		/*configure register*/
		mt65xx_reg_sync_writel(highspeed, UART_HIGHSPEED(g_uart));
		
		byte = DRV_Reg32(UART_LCR(g_uart));	   /* DLAB start */
		mt65xx_reg_sync_writel((byte | UART_LCR_DLAB), UART_LCR(g_uart));
		mt65xx_reg_sync_writel((high_speed_div & 0x00ff), UART_DLL(g_uart));
		mt65xx_reg_sync_writel(((high_speed_div >> 8)&0x00ff), UART_DLH(g_uart));
		mt65xx_reg_sync_writel(sample_count, UART_SAMPLE_COUNT(g_uart));
		mt65xx_reg_sync_writel(sample_point, UART_SAMPLE_POINT(g_uart));
		mt65xx_reg_sync_writel(byte, UART_LCR(g_uart));	  /* DLAB end */
	}
}

void mtk_set_current_uart(MTK_UART uart_base)
{
	switch(uart_base)
	{	
    case UART1 :
			g_uart = uart_base;
			break;
		case UART4 :
			g_uart = uart_base;
			break;
		default:
			ASSERT(0);
			break;
	}
}

void uart_init_early(void)
{
	mt_set_gpio_mode(GPIO_UART_UTXD1_PIN, GPIO_MODE_01);
	mt_set_gpio_mode(GPIO_UART_URXD1_PIN, GPIO_MODE_01);
	#ifdef __ENABLE_UART_LOG_SWITCH_FEATURE__
	if(get_uart_port_id() == 1){
		mtk_set_current_uart(UART1);
		mtk_uart_power_on(UART1);
	}else{
		mtk_set_current_uart(UART4);
		mtk_uart_power_on(UART4);
	}
	#else
	mtk_set_current_uart(UART1);
	mtk_uart_power_on(UART1);
	#endif
    
	DRV_SetReg32(UART_FCR(g_uart), UART_FCR_FIFO_INIT); /* clear fifo */ 
	mt65xx_reg_sync_writew(UART_NONE_PARITY | UART_WLS_8 | UART_1_STOP, UART_LCR(g_uart));
	g_brg = CONFIG_BAUDRATE;
	uart_setbrg();
}

void uart_init(void)
{
}

int uart_putc(const char c )
{
	while (!(DRV_Reg32(UART_LSR(g_uart)) & UART_LSR_THRE));

	if (c == '\n')
		mt65xx_reg_sync_writel((unsigned int)'\r', UART_THR(g_uart));

	mt65xx_reg_sync_writel((unsigned int)c, UART_THR(g_uart));

	return 0;
}

int uart_getc(void)  /* returns -1 if no data available */
{
	while (!(DRV_Reg32(UART_LSR(g_uart)) & UART_LSR_DR)); 	
 	return (int)DRV_Reg32(UART_RBR(g_uart));
}

void uart_puts(const char *s)
{
	while (*s)
		uart_putc(*s++);
}
