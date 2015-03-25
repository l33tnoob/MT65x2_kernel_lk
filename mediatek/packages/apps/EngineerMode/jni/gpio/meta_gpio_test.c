/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "meta_gpio.h"
#define TAG "[GPIOTST] "
#define NULL_FT {0,0}
//#define NULL 0
#define GIO_RETERR(res, fmt, args...)                                       \
    do {                                                                    \
        printf(TAG "%s:%04d: " fmt"\n", __FUNCTION__, __LINE__, ##args);    \
        return res;                                                         \
    } while(0)
/* GPIO MODE CONTROL VALUE*/
typedef enum {
    GPIO_MODE_GPIO  = 0,
    GPIO_MODE_00    = 0,
    GPIO_MODE_01    = 1,
    GPIO_MODE_02    = 2,
    GPIO_MODE_03    = 3,
    
    GPIO_MODE_MAX,    
    GPIO_MODE_DEFAULT = GPIO_MODE_01,
} GPIO_MODE;

/* GPIO DIRECTION */
typedef enum {
    GPIO_DIR_IN     = 0,
    GPIO_DIR_OUT    = 1,

    GPIO_DIR_MAX,
    GPIO_DIR_DEFAULT = GPIO_DIR_IN,
} GPIO_DIR;

/* GPIO PULL ENABLE*/
typedef enum {
    GPIO_PULL_DISABLE = 0,
    GPIO_PULL_ENABLE  = 1,

    GPIO_PULL_EN_MAX,
    GPIO_PULL_EN_DEFAULT = GPIO_PULL_ENABLE,
} GPIO_PULL_EN;

/* GPIO PULL-UP/PULL-DOWN*/
typedef enum {
    GPIO_PULL_DOWN  = 0,
    GPIO_PULL_UP    = 1,

    GPIO_PULL_MAX,
    GPIO_PULL_DEFAULT = GPIO_PULL_DOWN
} GPIO_PULL;

/* GPIO INVERSION */
typedef enum {
    GPIO_DATA_UNINV = 0,
    GPIO_DATA_INV   = 1,

    GPIO_DATA_INV_MAX,
    GPIO_DATA_INV_DEFAULT = GPIO_DATA_UNINV
} GPIO_INVERSION;

/* GPIO OUTPUT */
typedef enum {
    GPIO_OUT_ZERO = 0,
    GPIO_OUT_ONE  = 1,

    GPIO_OUT_MAX,
    GPIO_OUT_DEFAULT = GPIO_OUT_ZERO,
    GPIO_DATA_OUT_DEFAULT = GPIO_OUT_ZERO,  /*compatible with DCT*/
} GPIO_OUT;
/******************************************************************************
 * GET_MODE_STA
 * SET_MODE_0
 * SET_MODE_1
 * SET_MODE_2
 * SET_MODE_3
 *****************************************************************************/
int test_gpio_mode(HW_GPIO pin)
{
    GPIO_REQ req_get = {NULL_FT, pin, GET_MODE_STA};
    GPIO_REQ req_set_mode0 = {NULL_FT, pin, SET_MODE_0};
    GPIO_REQ req_set_mode1 = {NULL_FT, pin, SET_MODE_1};
    GPIO_REQ req_set_mode2 = {NULL_FT, pin, SET_MODE_2};
    GPIO_REQ req_set_mode3 = {NULL_FT, pin, SET_MODE_3};
    GPIO_REQ req_restore = {NULL_FT, pin, SET_DIR_IN};
     
    GPIO_CNF cnf = Meta_GPIO_OP(req_get, NULL, 0);
    unsigned char old = (unsigned char)cnf.data;
    if (old == GPIO_MODE_00)        req_restore.op = SET_MODE_0;
    else if (old == GPIO_MODE_01)   req_restore.op = SET_MODE_1;
    else if (old == GPIO_MODE_02)   req_restore.op = SET_MODE_2;
    else if (old == GPIO_MODE_03)   req_restore.op = SET_MODE_3;

    if (cnf.status != META_SUCCESS)
        GIO_RETERR(cnf.status, "");
    if (old == GPIO_MODE_00 || old == GPIO_MODE_01 ||
        old == GPIO_MODE_02 || old == GPIO_MODE_03)
        printf(TAG "mode old = %d\n", old);
    else
        printf(TAG "test mode fail: %d\n", old);

    /*SET_MODE_0*/
    cnf = Meta_GPIO_OP(req_set_mode0, NULL, 0);
    if (cnf.status != META_SUCCESS)
        GIO_RETERR(cnf.status, "");
    
    cnf = Meta_GPIO_OP(req_get, NULL, 0);
    if (cnf.status != META_SUCCESS)
        GIO_RETERR(cnf.status, "");
    if (cnf.data != GPIO_MODE_00)
        GIO_RETERR(META_FAILED, "");
   
    /*SET_MODE_1*/
    cnf = Meta_GPIO_OP(req_set_mode1, NULL, 0);
    if (cnf.status != META_SUCCESS)
        GIO_RETERR(cnf.status, "");
    
    cnf = Meta_GPIO_OP(req_get, NULL, 0);
    if (cnf.status != META_SUCCESS)
        GIO_RETERR(cnf.status, "");
    if (cnf.data != GPIO_MODE_01)
        GIO_RETERR(META_FAILED, "");
    
    /*SET_MODE_2*/
    cnf = Meta_GPIO_OP(req_set_mode2, NULL, 0);
    if (cnf.status != META_SUCCESS)
        GIO_RETERR(cnf.status, "");
    
    cnf = Meta_GPIO_OP(req_get, NULL, 0);
    if (cnf.status != META_SUCCESS)
        GIO_RETERR(cnf.status, "");
    if (cnf.data != GPIO_MODE_02)
        GIO_RETERR(META_FAILED, "");
    
    /*SET_MODE_3*/
    cnf = Meta_GPIO_OP(req_set_mode3, NULL, 0);
    if (cnf.status != META_SUCCESS)
        GIO_RETERR(cnf.status, "");
    
    cnf = Meta_GPIO_OP(req_get, NULL, 0);
    if (cnf.status != META_SUCCESS)
        GIO_RETERR(cnf.status, "");
    if (cnf.data != GPIO_MODE_03)
        GIO_RETERR(META_FAILED, "");
    
    /*restore*/
    cnf = Meta_GPIO_OP(req_restore, NULL, 0);
    if (cnf.status != META_SUCCESS)
        GIO_RETERR(cnf.status, "");
    
    cnf = Meta_GPIO_OP(req_get, NULL, 0);
    if (cnf.status != META_SUCCESS)
        GIO_RETERR(cnf.status, "");
    if (cnf.data != old)
        GIO_RETERR(META_FAILED, "");
    
    return META_SUCCESS;
}
/******************************************************************************
 * GET_DIR_STA
 * SET_DIR_IN
 * SET_DIR_OUT
 * GET_DATA_IN
 * GET_DATA_OUT
 * SET_DATA_LOW
 * SET_DATA_HIGH
 *****************************************************************************/
int test_gpio_dir(HW_GPIO pin)
{
    GPIO_REQ req_get = {NULL_FT, pin, GET_DIR_STA};
    GPIO_REQ req_set_dir_in  = {NULL_FT, pin, SET_DIR_IN};
    GPIO_REQ req_set_dir_out = {NULL_FT, pin, SET_DIR_OUT};
    GPIO_REQ req_get_dat_in  = {NULL_FT, pin, GET_DATA_IN};
    GPIO_REQ req_get_dat_out = {NULL_FT, pin, GET_DATA_OUT};
    GPIO_REQ req_set_dat_hi  = {NULL_FT, pin, SET_DATA_HIGH};
    GPIO_REQ req_set_dat_lo  = {NULL_FT, pin, SET_DATA_LOW};
    GPIO_REQ req_restore = {NULL_FT, pin, SET_DIR_IN};
     
    GPIO_CNF cnf = Meta_GPIO_OP(req_get, NULL, 0);
    unsigned char old = (unsigned char)cnf.data;
    if (old == GPIO_DIR_IN)         req_restore.op = SET_DIR_IN;
    else if (old == GPIO_DIR_OUT)   req_restore.op = SET_DIR_OUT;

    if (cnf.status != META_SUCCESS)
        GIO_RETERR(cnf.status, "");
    if (old == GPIO_DIR_IN || old == GPIO_DIR_OUT)
        printf(TAG "dir old = %d\n", old);
    else
        printf(TAG "test dir fail: %d\n", old);

    /*SET_DIR_OUT*/
    cnf = Meta_GPIO_OP(req_set_dir_out, NULL, 0);
    if (cnf.status != META_SUCCESS)
        GIO_RETERR(cnf.status, "");
    
    cnf = Meta_GPIO_OP(req_get, NULL, 0);
    if (cnf.status != META_SUCCESS)
        GIO_RETERR(cnf.status, "");
    if (cnf.data != GPIO_DIR_OUT)
        GIO_RETERR(META_FAILED, "");
   
    /*SET_DIR_OUT: 1*/
    cnf = Meta_GPIO_OP(req_set_dat_hi, NULL, 0);
    if (cnf.status != META_SUCCESS)
        GIO_RETERR(cnf.status, "");
    
    cnf = Meta_GPIO_OP(req_get_dat_out, NULL, 0);
    if (cnf.status != META_SUCCESS)
        GIO_RETERR(cnf.status, "");
    if (cnf.data != GPIO_OUT_ONE)
        GIO_RETERR(META_FAILED, "");
        
    /*SET_DIR_OUT: 0*/
    cnf = Meta_GPIO_OP(req_set_dat_lo, NULL, 0);
    if (cnf.status != META_SUCCESS)
        GIO_RETERR(cnf.status, "");
    
    cnf = Meta_GPIO_OP(req_get_dat_out, NULL, 0);
    if (cnf.status != META_SUCCESS)
        GIO_RETERR(cnf.status, "");
    if (cnf.data != GPIO_OUT_ZERO)
        GIO_RETERR(META_FAILED, "");
       
    
    /*SET_DIR_IN*/
    cnf = Meta_GPIO_OP(req_set_dir_in, NULL, 0);
    if (cnf.status != META_SUCCESS)
        GIO_RETERR(cnf.status, "");
    
    cnf = Meta_GPIO_OP(req_get, NULL, 0);
    if (cnf.status != META_SUCCESS)
        GIO_RETERR(cnf.status, "");
    if (cnf.data != GPIO_DIR_IN)
        GIO_RETERR(META_FAILED, "");
    
    cnf = Meta_GPIO_OP(req_get_dat_in, NULL, 0);
    if (cnf.status != META_SUCCESS)
        GIO_RETERR(cnf.status, "");
    if (cnf.data != GPIO_OUT_ZERO && cnf.data != GPIO_OUT_ONE)
        GIO_RETERR(META_FAILED, "");
    
    /*restore*/
    cnf = Meta_GPIO_OP(req_restore, NULL, 0);
    if (cnf.status != META_SUCCESS)
        GIO_RETERR(cnf.status, "");
    
    cnf = Meta_GPIO_OP(req_get, NULL, 0);
    if (cnf.status != META_SUCCESS)
        GIO_RETERR(cnf.status, "");
    if (cnf.data != old)
        GIO_RETERR(META_FAILED, "");
    
    return META_SUCCESS;
}
/******************************************************************************
 * GET_PULLEN_STA
 * SET_PULLEN_DISABLE
 * SET_PULLEN_ENABLE
 *****************************************************************************/
int test_gpio_pullen(HW_GPIO pin)
{
    GPIO_REQ req_get = {NULL_FT, pin, GET_PULLEN_STA};
    GPIO_REQ req_set_pulldi = {NULL_FT, pin, SET_PULLEN_DISABLE};
    GPIO_REQ req_set_pullen = {NULL_FT, pin, SET_PULLEN_ENABLE};
    GPIO_REQ req_restore = {NULL_FT, pin, SET_PULLEN_DISABLE};
     
    GPIO_CNF cnf = Meta_GPIO_OP(req_get, NULL, 0);
    unsigned char old = (unsigned char)cnf.data;
    if (old == GPIO_PULL_DISABLE)   req_restore.op = SET_PULLEN_DISABLE;
    else if (old == GPIO_PULL_ENABLE) req_restore.op = SET_PULLEN_ENABLE;

    if (cnf.status != META_SUCCESS)
        GIO_RETERR(cnf.status, "");
    if (old == GPIO_PULL_DISABLE || old == GPIO_PULL_ENABLE)
        printf(TAG "pull old = %d\n", old);
    else
        printf(TAG "test pull fail: %d\n", old);

    /*SET_PULLEN_DISABLE*/
    cnf = Meta_GPIO_OP(req_set_pulldi, NULL, 0);
    if (cnf.status != META_SUCCESS)
        GIO_RETERR(cnf.status, "");
    
    cnf = Meta_GPIO_OP(req_get, NULL, 0);
    if (cnf.status != META_SUCCESS)
        GIO_RETERR(cnf.status, "");
    if (cnf.data != GPIO_PULL_DISABLE)
        GIO_RETERR(META_FAILED, "");
    
    /*SET_PULLEN_ENABLE*/
    cnf = Meta_GPIO_OP(req_set_pullen, NULL, 0);
    if (cnf.status != META_SUCCESS)
        GIO_RETERR(cnf.status, "");
    
    cnf = Meta_GPIO_OP(req_get, NULL, 0);
    if (cnf.status != META_SUCCESS)
        GIO_RETERR(cnf.status, "");
    if (cnf.data != GPIO_PULL_ENABLE)
        GIO_RETERR(META_FAILED, "");
    
    /*restore*/
    cnf = Meta_GPIO_OP(req_restore, NULL, 0);
    if (cnf.status != META_SUCCESS)
        GIO_RETERR(cnf.status, "");
    
    cnf = Meta_GPIO_OP(req_get, NULL, 0);
    if (cnf.status != META_SUCCESS)
        GIO_RETERR(cnf.status, "");
    if (cnf.data != old)
        GIO_RETERR(META_FAILED, "");
    
    return META_SUCCESS;
}
/******************************************************************************
 * GET_PULL_STA
 * SET_PULL_DOWN
 * SET_PULL_UP
 *****************************************************************************/
int test_gpio_pull(HW_GPIO pin)
{
    GPIO_REQ req_get = {NULL_FT, pin, GET_PULL_STA};
    GPIO_REQ req_set_pulldn = {NULL_FT, pin, SET_PULL_DOWN};
    GPIO_REQ req_set_pullup = {NULL_FT, pin, SET_PULL_UP};
    GPIO_REQ req_restore = {NULL_FT, pin, SET_PULL_DOWN};
     
    GPIO_CNF cnf = Meta_GPIO_OP(req_get, NULL, 0);
    unsigned char old = (unsigned char)cnf.data;
    if (old == GPIO_PULL_DOWN)      req_restore.op = SET_PULL_DOWN;
    else if (old == GPIO_PULL_UP)   req_restore.op = SET_PULL_UP;

    if (cnf.status != META_SUCCESS)
        GIO_RETERR(cnf.status, "");
    if (old == GPIO_PULL_DOWN || old == GPIO_PULL_UP)
        printf(TAG "pull old = %d\n", old);
    else
        printf(TAG "test pull fail: %d\n", old);

    /*SET_PULL_DOWN*/
    cnf = Meta_GPIO_OP(req_set_pulldn, NULL, 0);
    if (cnf.status != META_SUCCESS)
        GIO_RETERR(cnf.status, "");
    
    cnf = Meta_GPIO_OP(req_get, NULL, 0);
    if (cnf.status != META_SUCCESS)
        GIO_RETERR(cnf.status, "");
    if (cnf.data != GPIO_PULL_DOWN)
        GIO_RETERR(META_FAILED, "");
    
    /*SET_PULL_UP*/
    cnf = Meta_GPIO_OP(req_set_pullup, NULL, 0);
    if (cnf.status != META_SUCCESS)
        GIO_RETERR(cnf.status, "");
    
    cnf = Meta_GPIO_OP(req_get, NULL, 0);
    if (cnf.status != META_SUCCESS)
        GIO_RETERR(cnf.status, "");
    if (cnf.data != GPIO_PULL_UP)
        GIO_RETERR(META_FAILED, "");
    
    /*restore*/
    cnf = Meta_GPIO_OP(req_restore, NULL, 0);
    if (cnf.status != META_SUCCESS)
        GIO_RETERR(cnf.status, "");
    
    cnf = Meta_GPIO_OP(req_get, NULL, 0);
    if (cnf.status != META_SUCCESS)
        GIO_RETERR(cnf.status, "");
    if (cnf.data != old)
        GIO_RETERR(META_FAILED, "");
    
    return META_SUCCESS;

}
/******************************************************************************
 * GET_INV_STA
 * SET_INV_ENABLE
 * SET_INV_DISABLE
 *****************************************************************************/
int test_gpio_inv(HW_GPIO pin)
{
    GPIO_REQ req_get = {NULL_FT, pin, GET_INV_STA};
    GPIO_REQ req_set_inven = {NULL_FT, pin, SET_INV_ENABLE};
    GPIO_REQ req_set_invdi = {NULL_FT, pin, SET_INV_DISABLE};
    GPIO_REQ req_restore = {NULL_FT, pin, SET_INV_ENABLE};
     
    GPIO_CNF cnf = Meta_GPIO_OP(req_get, NULL, 0);
    unsigned char old = (unsigned char)cnf.data;
    if (old == GPIO_DATA_UNINV)     req_restore.op = SET_INV_DISABLE;
    else if (old == GPIO_DATA_INV)  req_restore.op = SET_INV_ENABLE;    

    if (cnf.status != META_SUCCESS)
        GIO_RETERR(cnf.status, "");
    if (old == GPIO_DATA_UNINV || old == GPIO_DATA_INV)
        printf(TAG "inv old = %d\n", old);
    else
        printf(TAG "test inv fail: %d\n", old);

    /*SET_INV_ENABLE*/
    cnf = Meta_GPIO_OP(req_set_inven, NULL, 0);
    if (cnf.status != META_SUCCESS)
        GIO_RETERR(cnf.status, "");
    
    cnf = Meta_GPIO_OP(req_get, NULL, 0);
    if (cnf.status != META_SUCCESS)
        GIO_RETERR(cnf.status, "");
    if (cnf.data != GPIO_DATA_INV)
        GIO_RETERR(META_FAILED, "");
    
    /*SET_INV_DISABLE*/
    cnf = Meta_GPIO_OP(req_set_invdi, NULL, 0);
    if (cnf.status != META_SUCCESS)
        GIO_RETERR(cnf.status, "");
    
    cnf = Meta_GPIO_OP(req_get, NULL, 0);
    if (cnf.status != META_SUCCESS)
        GIO_RETERR(cnf.status, "");
    if (cnf.data != GPIO_DATA_UNINV)
        GIO_RETERR(META_FAILED, "");
    
    /*restore*/
    cnf = Meta_GPIO_OP(req_restore, NULL, 0);
    if (cnf.status != META_SUCCESS)
        GIO_RETERR(cnf.status, "");
    
    cnf = Meta_GPIO_OP(req_get, NULL, 0);
    if (cnf.status != META_SUCCESS)
        GIO_RETERR(cnf.status, "");
    if (cnf.data != old)
        GIO_RETERR(META_FAILED, "");
    
    return META_SUCCESS;
}
/******************************************************************************
 * test main entry
 *****************************************************************************/
int main(int argc, const char** args)
{
    int pin;
    int res;
    extern void Meta_GPIO_Debug(bool);

    if (false == Meta_GPIO_Init())
    {
        printf(TAG "Meta_GPIO_Init() fail");
        return -1;
    }
    if (argc == 2 && !strcmp(args[1], "-d")) 
    {
        Meta_GPIO_Debug(true);
        printf(TAG "debug on");
    } 
    else
    {
        Meta_GPIO_Debug(false);
        printf(TAG "debug off");
    }
    for (pin = HW_GPIO0; pin < HW_GPIO_MAX; pin++)        
    {
        printf(TAG "<< GPIO %d START >>\n", pin);
        if (META_SUCCESS != (res = test_gpio_mode(pin)))
        {
            printf(TAG "test_gpio_mode() fail, res = %d\n", res);
            break;
        }
        if (META_SUCCESS != (res = test_gpio_dir(pin)))
        {
            printf(TAG "test_gpio_dir() fail, res = %d\n", res);
            break;
        }
        if (META_SUCCESS != (res = test_gpio_pullen(pin)))
        {
            printf(TAG "test_gpio_pullen() fail, res = %d\n", res);
            break;
        }
        if (META_SUCCESS != (res = test_gpio_pull(pin)))
        {
            printf(TAG "test_gpio_pull() fail, res = %d\n", res);
            break;
        }
        if (META_SUCCESS != (res = test_gpio_inv(pin)))
        {
            printf(TAG "test_gpio_inv() fail, res = %d\n", res);
            break;
        }
        printf(TAG "<< GPIO %d END >>\n", pin);
    }
    if (false == Meta_GPIO_Deinit())
    {
        printf(TAG "Meta_GPIO_DeInit() fail\n");
        return -1;
    }
    return 0;
}
