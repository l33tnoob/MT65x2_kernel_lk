/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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

#ifndef __AFM_MODULE_MSDC__
#define __AFM_MODULE_MSDC__


#include <chip_support.h>

struct oldmsdc_ioctl{
	int  						opcode;
	int  						host_num;
	int  						iswrite;
	int  						trans_type;
	unsigned int    total_size;
	unsigned int    address;
	unsigned int*   buffer;
	int  						cmd_driving;
	int  						dat_driving;
	int  						clock_freq;
	int  						result;
};

#define MSDC_ODC_4MA     (0x0)
#define MSDC_ODC_8MA     (0x4)
#define MSDC_ODC_12MA    (0x2)
#define MSDC_ODC_16MA    (0x6)
#define MSDC_ODC_COUNT		4

#define MSDC_HOST_SCLK              (25000000)
/*
#define MSDC_DRIVING_SETTING              (0)
#define MSDC_CLOCK_FREQUENCY              (1)
#define MSDC_SINGLE_READ_WRITE   			    (2)
#define MSDC_MULTIPLE_READ_WRITE   				(3)
*/

#define NEW_MSDC_HOST_SCLK              (25000000)

#define NEW_MSDC_DRIVING_SETTING              (0)
#define NEW_MSDC_CLOCK_FREQUENCY              (1)
#define NEW_MSDC_SINGLE_READ_WRITE 	    (2)
#define NEW_MSDC_MULTIPLE_READ_WRITE   				(3)

class RPCClient;

class ModuleMsdc
{
public:
	ModuleMsdc();
	virtual ~ModuleMsdc();
    
	static int setCurrent(RPCClient* msgSender);
	static int getCurrent(RPCClient* msgSender);
	static int setSd30Mode(RPCClient* msgSender);
	
	static int setCurrentInternal(int hostNum, int clkPU, int clkPD, int cmdPU, 
	    int cmdPD, int datPU, int datPD, int hopBit , int hopTime,int opcode);
	static int setSd30ModeInternal(int hostNum, int sd30Mode, int sd30MaxCurrent, int sd30Drive, int sd30PowerControl);
	static int getCurrentInternal(int hostNum, int opcode);
};

#endif	
