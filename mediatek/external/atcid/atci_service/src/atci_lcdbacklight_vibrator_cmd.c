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

#include "atci_lcdbacklight_vibrator_cmd.h"
#include "atci_service.h"
#include "atcid_util.h"
#include <stdio.h>
#include <fcntl.h>


#define DEV_VIBR_PATH	"/sys/class/timed_output/vibrator/vibr_on"
#define VIBR_ON		"1"
#define VIBR_OFF	"0"

char vibrator_state = '0';	

int lcdbacklight_power_on_cmd_handler(char *cmdline, ATOP_t at_op, char *response){

	switch(at_op){
	        case AT_ACTION_OP:
        	case AT_READ_OP:
	        case AT_TEST_OP:
		case AT_SET_OP:
			system("echo 254 > /sys/class/leds/lcd-backlight/brightness");
			break;
	default:
			break;
	}
	sprintf(response,"\r\n\r\nLEDON OK\r\n\r\n");
	
	return 0;

}


int vibrator_power_off_cmd_handler(char *cmdline, ATOP_t at_op, char *response){
	int fd = 0;
	char log_info[100] = {'\0'};
	char c;	
	
	ALOGD("handle cmdline:%s", cmdline);
	fd = open(DEV_VIBR_PATH, O_RDWR);
	if(fd < 0){
		sprintf(log_info, "Open FD error");
		goto error;
	}
	switch(at_op){
		case AT_ACTION_OP:
        	case AT_READ_OP:
	        case AT_TEST_OP:
			sprintf(log_info, "\r\n%c\r\n", vibrator_state);
			break;
		case AT_SET_OP:
		{
		if (strcmp(cmdline, VIBR_ON) == 0 || strcmp(cmdline, VIBR_OFF) == 0){
			vibrator_state = cmdline[0];
			if(vibrator_state == '1')
			{	
				sprintf(log_info, "MOTOR ON");
				write(fd, VIBR_ON, 1);
				}else
			{
				sprintf(log_info, "MOTOR OFF");
				write(fd, VIBR_OFF, 1);
			}
		}	
		else
			{	if(vibrator_state == 0 )
					sprintf(log_info, "\r\n%c\r\n", vibrator_state);
				else		
					sprintf(log_info, "MOTOR ERROR");
			}		
		}
		break;
		default:
			sprintf(log_info, "MOTOR ERROR%s", cmdline);
			break;
	}		
		
error:
	sprintf(response,"\r\n%s\n\r\n", log_info);
	close(fd);
	return 0;

}

