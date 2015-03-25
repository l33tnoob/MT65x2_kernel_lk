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

#include "emi.h"

#define NUM_EMI_RECORD (3)

int num_of_emi_records = NUM_EMI_RECORD;

EMI_SETTINGS emi_settings[] =
{
{
        0x0101,            //MCP(NAND+DDR1)
	{0xAD, 0xBC, 0x90, 0x55, 0x54},       //HYNIX
        0x0002202E,        //CONA
        0x88008800,        //DRVCTL0
        0x88008800,        //DRVCTL1
        0x00000005,        //DLE
        0x22824154,        //ACTIM
        0x00000000,        //GDDR3CTL
        0xF0040560,        //CONF1
        0x8283405C,        //DDR2CTL
        0x9F068CA0,        //TEST2_3
        0x00403361,        //CONF2
        0x11642842,        //PD_CTRL
        0x00000000,        //PADCTL3
        0x00000000,        //DQODLY
        0x00000000,        //ADDRODLY
        0x00000000,        //CLKODLY
        .reserved = {[0 ... 9] = 0},
        0x00000032,        //MODE_REG
        0x00000020,        //EXT_MODE_REG
    },
{
        0x0101,            //MCP(NAND+DDR1)
        {0x2C, 0xBC, 0x90, 0x55, 0x56},      //MICRON
        0x0002202E,        //CONA
        0x88008800,        //DRVCTL0
        0x88008800,        //DRVCTL1
        0x00000005,        //DLE
        0x22824154,        //ACTIM
        0x00000000,        //GDDR3CTL
        0xF0040560,        //CONF1
        0x8283405C,        //DDR2CTL
        0x9F008CA0,        //TEST2_3
        0x00403361,        //CONF2
        0x11642842,        //PD_CTRL
        0x00000000,        //PADCTL3
        0x00000000,        //DQODLY
        0x00000000,        //ADDRODLY
        0x00000000,        //CLKODLY
        .reserved = {[0 ... 9] = 0},
        0x00000032,        //MODE_REG
        0x00000020,        //EXT_MODE_REG
    },
{
        0x0101,            //MCP(NAND+DDR1)
        {0xEC, 0xBC, 0x00, 0x66, 0x56},          //SAMSUNG
        0x0002202E,        //CONA
        0x88008800,        //DRVCTL0
        0x88008800,        //DRVCTL1
        0x00000005,        //DLE
        0x22824165,        //ACTIM
        0x00000000,        //GDDR3CTL
        0xF00405A0,        //CONF1
        0x8283405C,        //DDR2CTL
        0x9F048CA0,        //TEST2_3
        0x00403361,        //CONF2
        0x11642842,        //PD_CTRL
        0x00000000,        //PADCTL3
        0x00000000,        //DQODLY
        0x00000000,        //ADDRODLY
        0x00000000,        //CLKODLY
        .reserved = {[0 ... 9] = 0},
        0x00000032,        //MODE_REG
        0x00000020,        //EXT_MODE_REG
    },
};

