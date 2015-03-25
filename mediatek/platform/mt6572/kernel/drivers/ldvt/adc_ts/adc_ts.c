/*
 * Copyright (C) 2010 MediaTek, Inc.
 *
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/earlysuspend.h>
#include <linux/jiffies.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <asm/atomic.h>
#include <asm/uaccess.h>
#include <asm/io.h>
//#include <asm/tcm.h>
#include <mach/irqs.h>
#include <mach/mt_clkmgr.h>


#include "adc_ts.h"

#ifdef CONFIG_MTK_LDVT_ADC_TS

extern u16 tpd_read(int position, int raw_data_offset);
extern int tpd_mode_select(int mode);
extern void tpd_enable_em_log(int enable);
extern void tpd_fav_coord_sel(int coord);

#define TPD_X                  0
#define TPD_Y                  1
#define TPD_Z1                 2
#define TPD_Z2                 3

#define TS_DEBUG   1

#define TS_NAME    "dvt-ts"

#if TS_DEBUG
    #define ts_print(fmt, arg...)  printk("[ts_udvt]: " fmt, ##arg)
#else
    #define ts_print(fmt, arg...)  do {} while (0)
#endif

struct udvt_cmd {
    int cmd;
    int value;
};

static int touch_log_en = 0;
static int touch_interrupt = 0;  // hw status
static int fav_mode_en = 0;
static int tpd_sample_mode_sel = SEL_MODE_SW;

struct timer_list ts_udvt_timer;
struct tasklet_struct ts_udvt_tasklet;
struct tasklet_struct ts_udvt_auto_sample_tasklet;
struct tasklet_struct ts_fav_mode_tasklet;
static inline u32 POS_XYZ(unsigned int v)
{
	*(volatile u32 *)AUXADC_TS_ADDR = v;
	return ((*(volatile u16 *)AUXADC_TS_DATA0) & 0xFFFF);
}



static void dump_AUXADC_regs(void)
{
	int i = 0;

	for(i = 0; i <= 0xa0/4; i++)
	{
		printk("[addr:value] 0x%8x:0x%8x\n", (AUXADC_BASE + 4*i), *(volatile u32 *)(AUXADC_BASE + 4*i));
	}
    printk("[addr:value] 0x%8x:0x%8x\n", AUXADC_TS_RAW_CON, *(volatile u32 *)(AUXADC_TS_RAW_CON));
	printk("[addr:value] 0x%8x:0x%8x\n", AUXADC_TS_AUTO_TIME_INTVL, *(volatile u32 *)(AUXADC_TS_AUTO_TIME_INTVL));
}

static void set_touch_resistance(int value)
{
	if(value == 1)
		*(volatile u32 *)AUXADC_TS_CON3 = (1<<15); //90 OKM
	if(value == 0)
		*(volatile u32 *)AUXADC_TS_CON3 = 0x00; //55 OKM
}

/*
* mode: 0: 12-bit, 1: 10-bit
* sedf: 0: different, 1: single-end
* PD: 0x00: turn on Y-_driver signal and PDN_sh_ref
         0x01: turn on PDN_IRQ and PDN_sh_ref
         0x02: reserved
         0x03: turn on PDN_IRQ
*/
static void set_ts_cmd_reg(int mode, int sedf, int pd)
{
	unsigned int value = *(volatile u32 *)AUXADC_TS_CMD;

	if(mode == 1)//10 bit-mode
		value |= (1<<3);
	else
		value &= (~(1<<3));
	if(sedf == 1)//single-end
		value |= (1<<2);
	else
		value &= (~(1<<2));
	value |= pd;
	*(volatile u32 *)AUXADC_TS_CMD = value;

	printk("AUXADC_TS_CMD: 0x%x\n", *(volatile u32 *)AUXADC_TS_CMD);
}

static void set_ts_raw_reg(int en, int start, int abort)
{
	unsigned int value = *(volatile u32 *)AUXADC_TS_RAW_CON;

	if(en == 1)
		value |= (1<<2);
	else
		value &= (~(1<<2));
	if(start == 1)
		value |= (1<<1);
	else
		value &= (~(1<<1));
	value |= abort;
	*(volatile u32 *)AUXADC_TS_RAW_CON = value;

}

static int get_fav_sw_data(int coordsel)
{
	int x = 0, y = 0, z1 = 0, z2 = 0;

	if(coordsel == 0x00) // x->y->z1->z2
	{
		static int tmp[4] = {0};
		int adr = *(volatile u32 *)AUXADC_TS_ADDR & 0x7;

		if (adr == AUXADC_TS_CMD_ADDR_X)
		{
			//printk("[ts_udvt] coordsel:0 AUXADC_TS_CMD_ADDR_X\n");
			tmp[0] = ((*(volatile u16 *)AUXADC_TS_DATA0) & 0xFFFF);
			*(volatile u32 *)AUXADC_TS_ADDR &= ~0x7;
			*(volatile u32 *)AUXADC_TS_ADDR |= AUXADC_TS_CMD_ADDR_Y;
			*(volatile u32 *)AUXADC_TS_CON1 |= 0x1 << FAV_EN_BIT;
			return 0;
		}
		else if (adr == AUXADC_TS_CMD_ADDR_Y)
		{
			//printk("[ts_udvt] coordsel:0 AUXADC_TS_CMD_ADDR_Y\n");
			tmp[1] = ((*(volatile u16 *)AUXADC_TS_DATA0) & 0xFFFF);
			*(volatile u32 *)AUXADC_TS_ADDR &= ~0x7;
			*(volatile u32 *)AUXADC_TS_ADDR |= AUXADC_TS_CMD_ADDR_Z1;
			*(volatile u32 *)AUXADC_TS_CON1 |= 0x1 << FAV_EN_BIT;
			return 0;
		}
		else if (adr == AUXADC_TS_CMD_ADDR_Z1)
		{
			//printk("[ts_udvt] coordsel:0 AUXADC_TS_CMD_ADDR_Z1\n");
			tmp[2] = ((*(volatile u16 *)AUXADC_TS_DATA0) & 0xFFFF);
			*(volatile u32 *)AUXADC_TS_ADDR &= ~0x7;
			*(volatile u32 *)AUXADC_TS_ADDR |= AUXADC_TS_CMD_ADDR_Z2;
			*(volatile u32 *)AUXADC_TS_CON1 |= 0x1 << FAV_EN_BIT;
			return 0;
		}
		else if (adr == AUXADC_TS_CMD_ADDR_Z2)
		{
			//printk("[ts_udvt] coordsel:0 AUXADC_TS_CMD_ADDR_Z2\n");
			tmp[3] = ((*(volatile u16 *)AUXADC_TS_DATA0) & 0xFFFF);
			*(volatile u32 *)AUXADC_TS_ADDR &= ~0x7;
			*(volatile u32 *)AUXADC_TS_ADDR |= AUXADC_TS_CMD_ADDR_X;
		}
		x = tmp[0];
		y = tmp[1];
		z1 = tmp[2];
		z2 = tmp[3];
		printk("[ts_udvt] coordsel:0x%x, x:%4d, y:%4d, z1:%4d, z2:%4d\n", coordsel, x, y, z1, z2);
		*(volatile u32 *)AUXADC_TS_CON1 |= 0x1 << FAV_EN_BIT;
		return 1;
	}
	else if (coordsel == 1) //x, y, z1, z2
	{
		x = ((*(volatile u16 *)AUXADC_TS_DATA0) & 0xFFFF);
		y = ((*(volatile u16 *)AUXADC_TS_DATA1) & 0xFFFF);
		z1= ((*(volatile u16 *)AUXADC_TS_DATA2) & 0xFFFF);
		z2 =((*(volatile u16 *)AUXADC_TS_DATA3) & 0xFFFF);
		printk("[ts_udvt] coordsel:0x%x, x:%4d, y:%4d, z1:%4d, z2:%4d\n", coordsel, x, y, z1, z2);
		*(volatile u32 *)AUXADC_TS_CON1 |= 0x1 << FAV_EN_BIT;
		return 1;
	}
	else if (coordsel == 2) // x1, y1, x2, y2
	{
		x = ((*(volatile u16 *)AUXADC_TS_DATA0) & 0xFFFF);
		y = ((*(volatile u16 *)AUXADC_TS_DATA1) & 0xFFFF);
		z1= ((*(volatile u16 *)AUXADC_TS_DATA2) & 0xFFFF);
		z2 =((*(volatile u16 *)AUXADC_TS_DATA3) & 0xFFFF);
		printk("[ts_udvt] coordsel:0x%x, x1:%4d, y1:%4d, x2:%4d, y2:%4d\n", coordsel, x, y, z1, z2);
		*(volatile u32 *)AUXADC_TS_CON1 |= 0x1 << FAV_EN_BIT;
		return 1;
	}
	else if (coordsel == 3) // z1, z2
	{
		z1= ((*(volatile u16 *)AUXADC_TS_DATA0) & 0xFFFF);
		z2 =((*(volatile u16 *)AUXADC_TS_DATA1) & 0xFFFF);
		printk("[ts_udvt] coordsel:0x%x, z1:%4d, z1:%4d\n", coordsel, z1, z2);
		*(volatile u32 *)AUXADC_TS_CON1 |= 0x1 << FAV_EN_BIT;
		return 1;
	}
	return 0;
}

unsigned int ts_udvt_read_status()
{
    return (*(volatile u16 *)AUXADC_TS_CON3) & 2;
}

unsigned int ts_udvt_read(int position)
{
    //*(volatile u32 *)AUXADC_TS_CMD = 0x8;
    switch(position) {
        case AUXADC_TS_POSITION_X:
            *(volatile u32 *)AUXADC_TS_ADDR = AUXADC_TS_CMD_ADDR_X ;
            break;
        case AUXADC_TS_POSITION_Y:
            *(volatile u32 *)AUXADC_TS_ADDR = AUXADC_TS_CMD_ADDR_Y;
            break;
        case AUXADC_TS_POSITION_Z1:
            *(volatile u32 *)AUXADC_TS_ADDR = AUXADC_TS_CMD_ADDR_Z1;
            break;
        case AUXADC_TS_POSITION_Z2:
		    *(volatile u32 *)AUXADC_TS_ADDR = AUXADC_TS_CMD_ADDR_Z2;
           // *(volatile u16 *)AUXADC_TS_CMD = AUXADC_TS_CMD_ADDR_Z2 | AUXADC_TS_SAMPLE_SETTING;
            break;
        default:
            return 0;
    }

    *(volatile u16 *)AUXADC_TS_CON0 = AUXADC_TS_CON_SPL_TRIGGER;

   while (AUXADC_TS_CON_SPL_MASK & (*(volatile u16 *)AUXADC_TS_CON0));

   return ((*(volatile u16 *)AUXADC_TS_DATA0) & 0xFFFF);
}

void ts_check_auto_count_interval(bool en)
{
	static unsigned int pre_jiffies;
	#define JIFFIES_TO_MS(j)	(j)*1000/HZ

	if (!en) pre_jiffies = 0;

	if (touch_log_en)
		printk("[ts_udvt]: time interval is %d\n", JIFFIES_TO_MS(jiffies-pre_jiffies));
	pre_jiffies = jiffies;
}

static inline void trig_auxadc_ch(bool autoset, bool en)
{
	if (autoset)
		__raw_writew((u32)en, AUXADC_CON0);

	if (en)
	{
		__raw_writew(0x0, AUXADC_CON1);
		__raw_writew(0xffff, AUXADC_CON1);
	}
	else
	{
		__raw_writew(0x0, AUXADC_CON1);
		__raw_writew(0x0, AUXADC_CON0);
	}
}

#define JIFFIES_TO_MS(j)	(j)*1000/HZ
#define TS_RAW_TRIG_INTVL	1
#define TS_TRIG_BY_POLLING	0
u32 touch_adc_trig_en = 1;
u32 auxadc_trig_en = 1;

static inline int trig_rtouch_ch(bool en)
{
	int cnt = 0;

	if (en)
	{
#if TS_TRIG_BY_POLLING
		__raw_writew(0x5, AUXADC_TS_ADDR);
		__raw_writew(0x1, AUXADC_TS_CON0);
		while(0x1 & __raw_readw(AUXADC_TS_CON0)) cnt++;
		__raw_writew(0x0, AUXADC_TS_RAW_CON);
#else
		/* RAW BATCH */
		disable_irq(MT65XX_TOUCH_BATCH_LINE);
		__raw_writew(TS_RAW_TRIG_INTVL, AUXADC_TS_AUTO_TIME_INTVL);

		/* Trigger */
		__raw_writew(0x6, AUXADC_TS_RAW_CON);
		while((0x1<<4 & __raw_readw(AUXADC_TS_RAW_CON)) == 0);
		while(0x1<<4 & __raw_readw(AUXADC_TS_RAW_CON)) cnt++;
#endif
	}
	else
	{
#if !TS_TRIG_BY_POLLING
		__raw_writew(0x0, AUXADC_TS_RAW_CON);
		enable_irq(MT65XX_TOUCH_BATCH_LINE);
#endif
	}
	return cnt;
}

static int trig_ts_ch(void *unused)
{
	u32 start_time, end_time;
	u32 cnt = 0;
	u32 times = 0;

	do {
		if (touch_adc_trig_en)
		{
			cnt = 0;
			start_time = jiffies;
			cnt = trig_rtouch_ch(TRUE);
			end_time = jiffies;

			if (JIFFIES_TO_MS(end_time-start_time)>10)
			{
				if (touch_log_en) printk("[ts_udvt] RTOUCH sample [%s] (%2d, %2d)\n", "FAILED", cnt, JIFFIES_TO_MS(end_time-start_time));
				break;
			}
			else
			{
				//if (touch_log_en) printk("[ts_udvt] RTOUCH sample [%s] (%2d, %2d)\n", "PASS", cnt, JIFFIES_TO_MS(end_time-start_time));
				if (++times%500 == 0)
					if (touch_log_en) printk("[ts_udvt] RTOUCH times=%d\n",times);
			}
		}
	}while (!kthread_should_stop());

	trig_rtouch_ch(FALSE);
	auxadc_trig_en = 0;

	return 0;
}

void tpd_work_with_adc(void)
{
	static u32 start_time, end_time;
	u32 cnt = 0;
	u32 times = 0;
	struct task_struct *thread;
	int err;

#if 1 //trigger ts adc
    thread = kthread_run(trig_ts_ch, 0, "ts_adc_ts");
    if (IS_ERR(thread))
    {
        err = PTR_ERR(thread);
        printk(" failed to create kernel thread (trig_ts_ch) %d\n", err);
    }
#endif
	//msleep(1);
	do {
		if (auxadc_trig_en)
		{
			cnt = 0;
			start_time = jiffies;
			trig_auxadc_ch(FALSE, TRUE); // trigger ch0 with immediate mode
			while((*(volatile u32 *)(AUXADC_DAT0) & 0x1<<12) == 0) cnt++;
			end_time = jiffies;

			if (JIFFIES_TO_MS(end_time-start_time)>10)
			{
				if (touch_log_en) printk("[ts_udvt] AUXADC sample [%s] (%2d, %2d)\n", "FAILED", cnt, JIFFIES_TO_MS(end_time-start_time));
				trig_auxadc_ch(FALSE, FALSE);
				touch_adc_trig_en = 0;
				if (thread) kthread_stop(thread);
				break;
			}
			else
			{
				//if (touch_log_en) printk("[ts_udvt] AUXADC sample [%s] (%2d, %2d)\n", "PASS", cnt, JIFFIES_TO_MS(end_time-start_time));
				if (++times%500 == 0)
					if (touch_log_en) printk("[ts_udvt] AUXADC times=%d\n",times);
			}
		}
		else
			break;
	}while (1);
}

// Batch_IRQ interrupt
irqreturn_t ts_udvt_FAV_fn(int irq, void *dev_id)
{
	unsigned int tmp_value = 0;
	unsigned int value = *(volatile u32 *)AUXADC_TS_CON1;
	unsigned int x, y, z1, z2;

	if (touch_log_en) printk("[ts_udvt] BATCH_IRQ AUXADC_TS_CON1:0x%x, sample_mode=%d\n" , value, tpd_sample_mode_sel);

	//(*(volatile u32 *)AUXADC_TS_CON1) = (((*(volatile u32 *)AUXADC_TS_CON1))|(1<<FAV_EN_BIT));
	//while(((*(volatile u32 *)AUXADC_TS_CON1)&(1<<FAV_EN_BIT)) == 0)break;


    if (tpd_sample_mode_sel == SEL_MODE_RAW_DATA) {
       disable_irq_nosync(MT65XX_TOUCH_BATCH_LINE);
	   ts_check_auto_count_interval(TRUE);
        if(ts_udvt_auto_sample_tasklet.state != TASKLET_STATE_RUN)
            tasklet_hi_schedule(&ts_udvt_auto_sample_tasklet);
        enable_irq(MT65XX_TOUCH_BATCH_LINE);
        return IRQ_HANDLED;
    }
	else if (tpd_sample_mode_sel == SEL_MODE_FAV_HW)
	{
		/*static int count = 0;
		if (count++ >= 500) {
			*(volatile u32 *)AUXADC_TS_CON1 &= ~(0x1 <<FAV_SEL);
			count = 0;
		}*/
		x = ((*(volatile u16 *)AUXADC_TS_DATA0) & 0xFFFF);
		y = ((*(volatile u16 *)AUXADC_TS_DATA1) & 0xFFFF);
		z1= ((*(volatile u16 *)AUXADC_TS_DATA2) & 0xFFFF);
		z2 =((*(volatile u16 *)AUXADC_TS_DATA3) & 0xFFFF);
		if (touch_log_en) printk("[ts_udvt] FAV mode:x:%4d, y:%4d, z1:%4d, z2:%4d\n", x, y, z1, z2);
	}
	else if (tpd_sample_mode_sel == SEL_MODE_FAV_SW)
	{
		if (*(volatile u32 *)AUXADC_TS_ADDR & 0x1<<FAV_INVALID)
		{
			if (touch_log_en) printk("[ts_udvt] Invalid sample");
			return IRQ_HANDLED;
		}
		if (!touch_interrupt) return IRQ_HANDLED;
		tmp_value = ((value>>5)&0x03);
		if (get_fav_sw_data(tmp_value) == 0)
			return IRQ_HANDLED;
	}

	tmp_value = value&0x03;
	if(tmp_value != 0)
		if (touch_log_en) printk("[ts_udvt] FAV mode: repeat times: %2d \n", 2*(1<<tmp_value));
	else
		if (touch_log_en) printk("[ts_udvt] FAV mode: repeat times: 1 \n");
	return IRQ_HANDLED;
}

/* handle touch panel interrupt */
void ts_udvt_auto_trigger_fn(unsigned long unused)
{
    int i, j;
    int rx, ry, rz1, rz2, x = 0, y = 0, z1 = 0, z2 = 0, p = 0;

    printk("[ts_udvt]: start ts auto trigger\n");

/*    *(volatile u32 *)AUXADC_TS_CON0 = 0x0006;

    for (i = 0; i < 8000; i++);

    while ((*(volatile u16 *)AUXADC_CON2) & 0x01) {
        printk("[ts_udvt]: wait for module idle\n");
        mdelay(300);
	  }

    printk("[ts_udvt]: auto sampling completed, start to read data\n");
 */
 	if ((*(volatile u32 *)AUXADC_TS_RAW_CON) & 0x1)
 	{
		printk("[ts_udvt]: auto sampling abort!!\n");
		return;
	}

	for (i = 0; i < 8; i++) {
		rx  = tpd_read(TPD_X, i );
		ry  = tpd_read(TPD_Y, i );
		rz1 = tpd_read(TPD_Z1, i );
		rz2 = tpd_read(TPD_Z2, i );
		printk("raw[%5d %5d %5d %5d] \n", rx, ry, rz1, rz2);
		if (rx == 0 && ry == 4095) break;

		x += rx; y += ry;
		p = p + ((rx + 1) * (rz2 - rz1) / (rz1 + 1));

		udelay(5);
	 }

	 if (i == 0)
		 x = 0, y = 0, p = 0;
	 else
		 x /= i, y /= i, p /= i;


    printk("[ts_udvt]: x = %4d, y = %4d, z1 = %4d, z2 = %4d\n", x, y, z1, z2);

    if (touch_interrupt && x != 0 && y != 4095) {
        //mod_timer(&ts_udvt_timer, jiffies + 20);
    } else {
        touch_interrupt = 0;
        //mod_timer(&ts_udvt_timer, jiffies + 1000);
    }

}

/* timer keep polling touch panel status */
void ts_udvt_timer_fn(unsigned long arg)
{
    if (touch_log_en) printk("[ts_udvt]: timer trigger\n");

    if ((*(volatile u16 *)AUXADC_TS_RAW_CON) & 0x0004) {
        if(ts_udvt_auto_sample_tasklet.state != TASKLET_STATE_RUN)
            tasklet_hi_schedule(&ts_udvt_auto_sample_tasklet);
    } else{
        if (ts_udvt_tasklet.state != TASKLET_STATE_RUN)
            tasklet_hi_schedule(&ts_udvt_tasklet);
    }
}

/* PEN_IRQ interrupt */
irqreturn_t ts_udvt_handler(int irq, void *dev_id)
{
    if (touch_log_en) printk("[ts_udvt] PEN_IRQ trigger mode=%d\n", tpd_sample_mode_sel);

	if ((*(volatile u32 *)AUXADC_TS_CON0 & AUXADC_TS_CON_STATUS_MASK) == 0)
	{
		touch_interrupt = 0;
	}
	else
		touch_interrupt = 1;

	if (tpd_sample_mode_sel == SEL_MODE_SW)
	{
		// SW_MODE trigger
		if (touch_log_en) printk("[ts_udvt]: TS_STATUS is %d\n", touch_interrupt);
        if(ts_udvt_tasklet.state != TASKLET_STATE_RUN)
            tasklet_hi_schedule(&ts_udvt_tasklet);
    }
	else if (tpd_sample_mode_sel == SEL_MODE_FAV_HW)
	{
		// FAV_HW trigger
		if (touch_log_en) printk("[ts_udvt]: FAV_HW trigger\n");
		if (touch_interrupt)
			*(volatile u32 *)AUXADC_TS_CON1 |= 0x1 <<FAV_SEL;
		else
			*(volatile u32 *)AUXADC_TS_CON1 &= ~(0x1 <<FAV_SEL);
	}
	else if (tpd_sample_mode_sel == SEL_MODE_FAV_SW)
	{
		// FAV_SW trigger
		if (touch_interrupt)
		{
			*(volatile u32 *)AUXADC_TS_ADDR &= ~0x7;
			*(volatile u32 *)AUXADC_TS_ADDR |= AUXADC_TS_CMD_ADDR_X;
			*(volatile u32 *)AUXADC_TS_CON1 |= 0x1 << FAV_EN_BIT;
		}
		else
			*(volatile u32 *)AUXADC_TS_CON1 &= ~(0x1 <<FAV_SEL);
	}
	else if (tpd_sample_mode_sel == SEL_MODE_RAW_DATA)
	{
		// RAW-BATCH mode
		if (touch_interrupt)
			*(volatile u32*)AUXADC_TS_RAW_CON |= 0x1<<2;
		else
			*(volatile u32*)AUXADC_TS_RAW_CON &= ~(0x1<<2);
	}

    return IRQ_HANDLED;
}

/* handle touch panel interrupt (SEL_MODE_SW)*/
void ts_udvt_tasklet_fn(unsigned long unused)
{
    int x, y, z1, z2;

    if (!touch_interrupt) return;

    x  = ts_udvt_read(AUXADC_TS_POSITION_X);
    y  = ts_udvt_read(AUXADC_TS_POSITION_Y);
    z1 = ts_udvt_read(AUXADC_TS_POSITION_Z1);
    z2 = ts_udvt_read(AUXADC_TS_POSITION_Z2);

    if (touch_log_en)
	{
		int p = ((x+1)*(z2-z1)/(z1+1));
		printk("[ts_udvt]: x=%4d, y=%4d, z1=%4d, z2=%4d, p=%4d\n", x, y, z1, z2, p);
    }

    if (touch_interrupt && x != 0 && y != 4095) {
        mod_timer(&ts_udvt_timer, jiffies + 5);
    } else {
        touch_interrupt = 0;
        //mod_timer(&ts_udvt_timer, jiffies + 1000);
    }

    return;
}

static long ts_udvt_dev_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    void __user *uarg = (void __user *)arg;

    struct udvt_cmd *pcmd = (struct udvt_cmd *)arg;

    printk("cmd:%d, value:0x%x\n", pcmd->cmd, pcmd->value);

    switch(pcmd->cmd) {
        case SET_AUXADC_TS_DEBT0:
            *(volatile u32 *)AUXADC_TS_DEBT0 = (u32)(pcmd->value & 0xFFFF);
            break;
		case SET_AUXADC_TS_DEBT1:
			*(volatile u32 *)AUXADC_TS_DEBT1 = (u32)(pcmd->value & 0xFFFF);
			break;
        case SET_AUXADC_TS_CMD:
            *(volatile u32 *)AUXADC_TS_CMD = (u32)(pcmd->value & 0xFFFF);
            break;
        case SET_AUXADC_TS_CON:
            //*(volatile u32 *)AUXADC_CON3 = (u32)(pcmd->value & 0xFFFF);
            break;
        case SET_AUXADC_TS_DAT0:
            *(volatile u32 *)AUXADC_TS_DATA0 = (u32)(pcmd->value & 0xFFFF);
            break;
        case SET_AUXADC_TS_AUTO_CON:
            *(volatile u32 *)AUXADC_TS_RAW_CON = (u32)(pcmd->value & 0xFFFF);
            break;
        case SET_AUXADC_TS_AUTO_COUNT:
            *(volatile u32 *)AUXADC_TS_AUTO_TIME_INTVL = (u16)(pcmd->value & 0xFFFF);
			ts_check_auto_count_interval(FALSE);
            break;
        case SET_AUXADC_TS_AUTO_X_DAT0:
            *(volatile u32 *)AUXADC_TS_RAW_X_DAT0 = (u32)(pcmd->value & 0xFFFF);
            break;
        case SET_AUXADC_TS_AUTO_Y_DAT0:
            *(volatile u32 *)AUXADC_TS_RAW_Y_DAT0 = (u32)(pcmd->value & 0xFFFF);
            break;
        case SET_AUXADC_TS_AUTO_Z1_DAT0:
            *(volatile u32 *)AUXADC_TS_RAW_Z1_DAT0 = (u32)(pcmd->value & 0xFFFF);
            break;
        case SET_AUXADC_TS_AUTO_Z2_DAT0:
            *(volatile u32 *)AUXADC_TS_RAW_Z2_DAT0 = (u32)(pcmd->value & 0xFFFF);
            break;
        case GET_AUXADC_TS_DEBT0:
            pcmd->value = ((*(volatile u32 *)AUXADC_TS_DEBT0) & 0xFFFF);
            break;
        case GET_AUXADC_TS_DEBT1:
            pcmd->value = ((*(volatile u32 *)AUXADC_TS_DEBT1) & 0xFFFF);
            break;
        case GET_AUXADC_TS_CMD:
            pcmd->value = ((*(volatile u32 *)AUXADC_TS_CMD) & 0xFFFF);
            break;
        case GET_AUXADC_TS_CON:
            //pcmd->value = ((*(volatile u32 *)AUXADC_CON3) & 0xFFFF);
            break;
        case GET_AUXADC_TS_DAT0:
            pcmd->value = ((*(volatile u32 *)AUXADC_TS_DATA0) & 0xFFFF);
            break;
        case GET_AUXADC_TS_AUTO_CON:
            pcmd->value = ((*(volatile u32 *)AUXADC_TS_RAW_CON) & 0xFFFF);
            break;
        case GET_AUXADC_TS_AUTO_COUNT:
            pcmd->value = ((*(volatile u32 *)AUXADC_TS_AUTO_TIME_INTVL) & 0xFFFF);
            break;
        case GET_AUXADC_TS_AUTO_X_DAT0:
            pcmd->value = ((*(volatile u32 *)AUXADC_TS_RAW_X_DAT0) & 0xFFFF);
            break;
        case GET_AUXADC_TS_AUTO_Y_DAT0:
            pcmd->value = ((*(volatile u32 *)AUXADC_TS_RAW_Y_DAT0) & 0xFFFF);
            break;
        case GET_AUXADC_TS_AUTO_Z1_DAT0:
            pcmd->value = ((*(volatile u32 *)AUXADC_TS_RAW_Z1_DAT0) & 0xFFFF);
            break;
        case GET_AUXADC_TS_AUTO_Z2_DAT0:
            pcmd->value = ((*(volatile u32 *)AUXADC_TS_RAW_Z2_DAT0) & 0xFFFF);
            break;
		case GET_REG_DEFAULT:
			dump_AUXADC_regs();
			break;
		case SET_RESISTANCE:
			set_touch_resistance(pcmd->value);
			break;
		case SET_S_D_MODE:
			if(pcmd->value == 0)
				set_ts_cmd_reg(0, 0, 0);
			else
				set_ts_cmd_reg(0, 1, 0);
			break;
		case SET_BIT_MODE:
			if(pcmd->value == 0)
				set_ts_cmd_reg(0, 0, 0);
			else
				set_ts_cmd_reg(1, 0, 0);
			break;
		case SET_RAW_ABORT:
			if(pcmd->value == 0) //start
				(*(volatile u32 *)AUXADC_TS_RAW_CON) = (((*(volatile u32 *)AUXADC_TS_RAW_CON))&0xfffe);
			else //abort
				(*(volatile u32 *)AUXADC_TS_RAW_CON) = (((*(volatile u32 *)AUXADC_TS_RAW_CON))|0x1);
			break;
		case SET_FAV_NOISE:
			if(pcmd->value == 1) //enable
				(*(volatile u32 *)AUXADC_TS_CON1) = (((*(volatile u32 *)AUXADC_TS_CON1))|(1<<FAV_ASAMP));
			else //disable
				(*(volatile u32 *)AUXADC_TS_CON1) = (((*(volatile u32 *)AUXADC_TS_CON1))&(~(1<<FAV_ASAMP)));
			break;
		case SET_FAV_ACC_MODE:
			if(pcmd->value == 1)// 1 times
			{
				(*(volatile u32 *)AUXADC_TS_CON1) = ((((*(volatile u32 *)AUXADC_TS_CON1))&0xfffc));
			}
			else if(pcmd->value == 4) // 4 times
			{
				(*(volatile u32 *)AUXADC_TS_CON1) = ((((*(volatile u32 *)AUXADC_TS_CON1))&0xfffc)|0x01);
			}
			else if(pcmd->value == 8) // 8 times
			{
				(*(volatile u32 *)AUXADC_TS_CON1) = ((((*(volatile u32 *)AUXADC_TS_CON1))&0xfffc)|0x02);
			}
			else if(pcmd->value == 16) // 16 times
			{
				(*(volatile u32 *)AUXADC_TS_CON1) = ((((*(volatile u32 *)AUXADC_TS_CON1))&0xfffc)|0x03);
			}
			break;
		case SET_FAV_LATENCY:
			(*(volatile u32 *)AUXADC_TS_CON1) = (((*(volatile u32 *)AUXADC_TS_CON1)&0x00FF)|(pcmd->value<<FAV_ADEL_BIT));
			break;
		case SET_FAV_INTR_MODE:
			if(pcmd->value == 0) {//software mode
				(*(volatile u32 *)AUXADC_TS_CON1) = (((*(volatile u32 *)AUXADC_TS_CON1))&(~(1<<FAV_SEL)));
			}
			if(pcmd->value == 1) {
				(*(volatile u32 *)AUXADC_TS_CON1) = (((*(volatile u32 *)AUXADC_TS_CON1))|((1<<FAV_SEL)));
				(*(volatile u32 *)AUXADC_MISC) = (((*(volatile u32 *)AUXADC_MISC))&(~(1<<8)));	// disale Misc
			}
			break;
		case SET_FAV_MODE:
			if(pcmd->value == 1) // enable
					(*(volatile u32 *)AUXADC_MISC) = (((*(volatile u32 *)AUXADC_MISC))|((1<<8)));
			else
				(*(volatile u32 *)AUXADC_MISC) = (((*(volatile u32 *)AUXADC_MISC))&(~(1<<8)));
			break;
		case SET_FAV_SAMPLING_MODE:
			(*(volatile u32 *)AUXADC_TS_CON1) &= ~(0x7<<5);
			(*(volatile u32 *)AUXADC_TS_CON1) |= pcmd->value<<5;
			//tpd_fav_coord_sel(pcmd->value);
			break;
        case ENABLE_TOUCH_LOG:
            touch_log_en = 1;
            tpd_enable_em_log(1);
            break;
        case DISABLE_TOUCH_LOG:
            touch_log_en = 0;
            tpd_enable_em_log(0);
            break;
		case SET_TS_WAKE_SRC:
#ifdef CONFIG_MTK_LDVT
    		//sc_set_wake_src(WAKE_SRC_TS,MT65XX_TOUCH_IRQ_LINE);
#else
			printk("sc_set_wake_src is only available in LDVT load\n");
#endif
    		break;
    	case SET_SAMPLE_ADJUST:
	        if(pcmd->value == 0)
	    			(*(volatile u32 *)AUXADC_TS_CON2) = 0x01;
	    		else
	    			(*(volatile u32 *)AUXADC_TS_CON2) = (0x100|pcmd->value);
    		printk("AUXADC_TS_CON2: 0x%x\n", *(volatile u32 *)AUXADC_TS_CON2);
    		break;
    	case SEL_MODE_SW:
			tpd_sample_mode_sel = SEL_MODE_SW;
    		tpd_mode_select(0x01);
    		break;
    	case SEL_MODE_FAV_SW:
			tpd_sample_mode_sel = SEL_MODE_FAV_SW;
    		tpd_mode_select(0x02);
    		break;
    	case SEL_MODE_FAV_HW:
			tpd_sample_mode_sel = SEL_MODE_FAV_HW;
    		tpd_mode_select(0x04);
    		break;
    	case SEL_MODE_RAW_DATA:
			tpd_sample_mode_sel = SEL_MODE_RAW_DATA;
    		tpd_mode_select(0x08);
    		break;
		case SET_WORK_WITH_ADC:
			auxadc_trig_en = touch_adc_trig_en = 1;
			tpd_work_with_adc();
        default:
            return -EINVAL;
    }
    return 0;
}

static int ts_udvt_dev_open(struct inode *inode, struct file *file)
{
	//if(hwEnableClock(MT65XX_PDN_PERI_TP,"Touch")==FALSE)
	//     printk("hwEnableClock TP failed.\n");
    return 0;
}

static struct file_operations ts_udvt_dev_fops = {
    .owner          = THIS_MODULE,
    .unlocked_ioctl = ts_udvt_dev_ioctl,
    .open           = ts_udvt_dev_open,
};

static struct miscdevice ts_udvt_dev = {
    .minor  = MISC_DYNAMIC_MINOR,
    .name   = TS_NAME,
    .fops   = &ts_udvt_dev_fops,
};

int ts_udvt_local_init(void)
{
#ifndef MT6585_FPGA
    if(enable_clock(MT_CG_AUX_SW_CG_ADC,"Touch")==FALSE)
        printk("hwEnableClock TP failed.\n");
#endif

    init_timer(&ts_udvt_timer);
    ts_udvt_timer.function = ts_udvt_timer_fn;
    tasklet_init(&ts_udvt_tasklet, ts_udvt_tasklet_fn, 0);
    tasklet_init(&ts_udvt_auto_sample_tasklet, ts_udvt_auto_trigger_fn, 0);
	tasklet_init(&ts_fav_mode_tasklet, ts_udvt_FAV_fn, 0);
    if(request_irq(MT65XX_TOUCH_IRQ_LINE, (irq_handler_t)ts_udvt_handler, IRQF_TRIGGER_FALLING, "mtk_tpd_penirq", NULL))
        printk("[ts_udvt]: request_irq failed.\n");

	if (request_irq(MT65XX_TOUCH_BATCH_LINE, ts_udvt_FAV_fn, IRQF_TRIGGER_FALLING, "mtk_tpd_favirq", NULL))
		printk("[ts_udvt]: request_irq failed.\n");

    return 0;
}

static int __init ts_udvt_mod_init(void)
{
    int ret;
    ret = misc_register(&ts_udvt_dev);
    if (ret) {
        printk("[ts_udvt]: register driver failed (%d)\n", ret);
        return ret;
    }
    /* free Product driver irq */
	free_irq(MT65XX_TOUCH_IRQ_LINE, NULL);
	free_irq(MT65XX_TOUCH_BATCH_LINE);
    ts_udvt_local_init();

    printk("[ts_udvt]: ts_udvt_local_init initialization\n");
    return 0;
}

/* should never be called */
static void __exit ts_udvt_mod_exit(void)
{
    int ret;

    ret = misc_deregister(&ts_udvt_dev);
    if(ret){
        printk("[ts_udvt]: unregister driver failed\n");
    }
}

module_init(ts_udvt_mod_init);
module_exit(ts_udvt_mod_exit);

MODULE_AUTHOR("mediatek");
MODULE_DESCRIPTION("MT6573 TS Driver for UDVT");
MODULE_LICENSE("GPL");

#endif //CONFIG_MTK_LDVT_ADC_TS