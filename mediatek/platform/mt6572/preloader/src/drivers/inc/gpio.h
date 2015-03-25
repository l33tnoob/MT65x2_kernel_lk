#ifndef _GPIO_H_
#define _GPIO_H_

/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

/******************************************************************************
 * mt6516_gpio.h - The file is the gpio header file !
 *
 * Copyright 2008-2009 MediaTek Co.,Ltd.
 *
 * DESCRIPTION: -
 *     This file expose an in-kernel gpio modules to other device drivers
 *
 * modification history
 * ----------------------------------------
 * 01a, 08 oct 2008, Koshi,Chiu written
 * ----------------------------------------
 ******************************************************************************/
#include <typedefs.h>
#include <cust_gpio_usage.h>
#include <gpio_const.h>
//#if (CFG_FPGA_PLATFORM)
//#define MT_GPIO_ENABLED 1
//#endif

/*----------------------------------------------------------------------------*/
//  Error Code No.
#define RSUCCESS	0
#define ERACCESS	1
#define ERINVAL		2
#define ERWRAPPER	3
/*----------------------------------------------------------------------------*/

#define MAX_GPIO_PIN    (MT_GPIO_BASE_MAX)
/******************************************************************************
* Enumeration for GPIO pin
******************************************************************************/
/* GPIO MODE CONTROL VALUE*/
typedef enum {
    GPIO_MODE_UNSUPPORTED = -1,
    GPIO_MODE_GPIO  = 0,
    GPIO_MODE_00    = 0,
    GPIO_MODE_01    = 1,
    GPIO_MODE_02    = 2,
    GPIO_MODE_03    = 3,
    GPIO_MODE_04    = 4,
    GPIO_MODE_05    = 5,
    GPIO_MODE_06    = 6,
    GPIO_MODE_07    = 7,

    GPIO_MODE_MAX,
    GPIO_MODE_DEFAULT = GPIO_MODE_01,
} GPIO_MODE;
/*----------------------------------------------------------------------------*/
/* GPIO DIRECTION */
typedef enum {
    GPIO_DIR_UNSUPPORTED = -1,
    GPIO_DIR_IN     = 0,
    GPIO_DIR_OUT    = 1,

    GPIO_DIR_MAX,
    GPIO_DIR_DEFAULT = GPIO_DIR_IN,
} GPIO_DIR;
/*----------------------------------------------------------------------------*/
/* GPIO PULL ENABLE*/
typedef enum {
    GPIO_PULL_EN_UNSUPPORTED = -1,
    GPIO_PULL_DISABLE = 0,
    GPIO_PULL_ENABLE  = 1,

    GPIO_PULL_EN_MAX,
    GPIO_PULL_EN_DEFAULT = GPIO_PULL_ENABLE,
} GPIO_PULL_EN;
/*----------------------------------------------------------------------------*/
/* GPIO PULL-UP/PULL-DOWN*/
typedef enum {
    GPIO_PULL_UNSUPPORTED = -1,
    GPIO_PULL_DOWN  = 0,
    GPIO_PULL_UP    = 1,

    GPIO_PULL_MAX,
    GPIO_PULL_DEFAULT = GPIO_PULL_DOWN
} GPIO_PULL;
/*----------------------------------------------------------------------------*/
/* GPIO INVERSION */
typedef enum {
    GPIO_DATA_INV_UNSUPPORTED = -1,
    GPIO_DATA_UNINV = 0,
    GPIO_DATA_INV   = 1,

    GPIO_DATA_INV_MAX,
    GPIO_DATA_INV_DEFAULT = GPIO_DATA_UNINV
} GPIO_INVERSION;
/*----------------------------------------------------------------------------*/
/* GPIO OUTPUT */
typedef enum {
    GPIO_OUT_UNSUPPORTED = -1,
    GPIO_OUT_ZERO = 0,
    GPIO_OUT_ONE  = 1,

    GPIO_OUT_MAX,
    GPIO_OUT_DEFAULT = GPIO_OUT_ZERO,
    GPIO_DATA_OUT_DEFAULT = GPIO_OUT_ZERO,  /*compatible with DCT*/
} GPIO_OUT;
/* GPIO INPUT */
typedef enum {
    GPIO_IN_UNSUPPORTED = -1,
    GPIO_IN_ZERO = 0,
    GPIO_IN_ONE  = 1,

    GPIO_IN_MAX,
} GPIO_IN;
/*CLOCK OUT*/
typedef enum {
    CLK_OUT_UNSUPPORTED = -1,
    CLK_OUT0,
    CLK_OUT1,
    CLK_OUT2,
    CLK_OUT3,
    CLK_OUT4,
    CLK_OUT5,
    CLK_MAX
}GPIO_CLKOUT;

typedef enum CLK_SRC
{
    CLK_SRC_UNSUPPORTED = -1,
    CLK_SRC_F32K 	= 0x2,
    CLK_SRC_F26M 	= 0x4,
    CLK_SRC_FAXI0 	= 0x6,
    CLK_SRC_FAXI1 	= 0x8,
    CLK_SRC_FAXI2,
    CLK_SRC_FAXI3,
    CLK_SRC_FAXI4,
    CLK_SRC_DPI_PIXELPLL,
    CLK_SRC_3GPLL_123M,
    CLK_SRC_FMM,

    CLK_SRC_MAX
}GPIO_CLKSRC;

/* GPIO POWER*/
typedef enum {
    GPIO_VIO28 = 0,
    GPIO_VIO18 = 1,
    MSDC_VIO28_MC1 = 2,
    MSDC_VIO18_MC1 = 3,
    MSDC_VMC = 4,

    GPIO_VIO_MAX,
} GPIO_POWER;
/*----------------------------------------------------------------------------*/
typedef struct {
    u32 val;
    u32 set;
    u32 rst;
    u32 _align1;
} VAL_REGS;
/*----------------------------------------------------------------------------*/
typedef struct {
    VAL_REGS    dir[5];             /*0x0000 ~ 0x004F: 80  bytes*/
    u8          rsv00[176];         /*0x0050 ~ 0x00FF: 176 bytes*/
    VAL_REGS    dout[5];            /*0x0100 ~ 0x014F: 80  bytes*/
    u8          rsv01[176];         /*0x0150 ~ 0x01FF: 176 bytes*/
    VAL_REGS    din[5];             /*0x0200 ~ 0x024F: 80  bytes*/
    u8          rsv02[176];         /*0x0250 ~ 0x02FF: 176 bytes*/
    VAL_REGS    mode[20];           /*0x0300 ~ 0x043F: 320 bytes*/
} GPIO_REGS;
/*----------------------------------------------------------------------------*/
typedef struct {
    unsigned int no     : 16;
    unsigned int mode   : 3;
    unsigned int pullsel: 1;
    unsigned int din    : 1;
    unsigned int dout   : 1;
    unsigned int pullen : 1;
    unsigned int dir    : 1;
    unsigned int dinv   : 1;
    unsigned int _align : 7;
} GPIO_CFG;
/******************************************************************************
* GPIO Driver interface
******************************************************************************/
/*direction*/
s32 mt_set_gpio_dir(u32 pin, u32 dir);
s32 mt_get_gpio_dir(u32 pin);

/*pull enable*/
s32 mt_set_gpio_pull_enable(u32 pin, u32 enable);
s32 mt_get_gpio_pull_enable(u32 pin);
/*pull select*/
s32 mt_set_gpio_pull_select(u32 pin, u32 select);
s32 mt_get_gpio_pull_select(u32 pin);

/*data inversion*/
s32 mt_set_gpio_inversion(u32 pin, u32 enable);
s32 mt_get_gpio_inversion(u32 pin);

/*input/output*/
s32 mt_set_gpio_out(u32 pin, u32 output);
s32 mt_get_gpio_out(u32 pin);
s32 mt_get_gpio_in(u32 pin);

/*mode control*/
s32 mt_set_gpio_mode(u32 pin, u32 mode);
s32 mt_get_gpio_mode(u32 pin);

/*clock output setting*/
s32 mt_set_clock_output(u32 num, u32 src, u32 div);
s32 mt_get_clock_output(u32 num, u32 *src, u32 *div);
void mt_gpio_init_post(U32 post_init);

void mt_gpio_set_default(void);
#endif //_GPIO_H_
