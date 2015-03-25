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

#include <sys/reboot.h>
#include <stdint.h>
#include <fcntl.h>

#include "atci_system_cmd.h"
#include "atci_service.h"
#include "atcid_util.h"

#ifndef bool
#define bool unsigned char
#endif

#ifndef true
#define true 1
#endif

#ifndef false
#define false 0
#endif

#define IMEI_LENGTH     15

#define FBOOT_FILE "/data/system/called_pre_boots.dat"
#define RESTART_PASS "RESTARTING SYSTEM"
#define RESTART_FAIL  "ERROR"
#define REBSTART_REASON "AT%RESTART"

#define RESTART_WAITING_TIME 5 //5 sec
//Haman changed for TC3
//extern bool LGE_FacWriteImei(bool isMaster, unsigned char *imei, bool needFlashProgram);
//extern bool LGE_FacReadImei(bool isMaster, unsigned char *imei);
//change end
int system_check_fboot(){
	int fd, ret = 0;
	fd = open(FBOOT_FILE, O_RDONLY);
	if(fd < 0) {
		// we've NOT fbooted
		ret = 0;
	}
	else {
		// we've fbooted
		ret = 1;
		close(fd);
	}
    ALOGD("check fboot :%d", ret);
	return ret;
}

static void* restart_routine(void *arg) {
	ALOGD("Ready to restarting system after %d seconds!", RESTART_WAITING_TIME);
	usleep(RESTART_WAITING_TIME*1000*1000);
	sync();
	ALOGD("byebye");

	//TBD: need check if we have to unmount fs.
	__reboot(LINUX_REBOOT_MAGIC1, LINUX_REBOOT_MAGIC2,
					 LINUX_REBOOT_CMD_RESTART2, REBSTART_REASON);

	return 0;
}

int invoke_restart(){
	int ret = 0;
	pthread_attr_t attr;
	pthread_t restart_thread;

	pthread_attr_init(&attr);

	ret = pthread_create(&restart_thread, &attr, restart_routine, NULL);
	if (ret != 0) 
	{
		ALOGE("fail to create thread for restarting");
	}
	
	return ret;
}

int system_fboot_handler(char* cmdline, ATOP_t at_op, char* response){    
    int retval = 0;
    
        ALOGD("[%s]handle cmdline:%s", __FUNCTION__, cmdline);
            
    switch(at_op){
        case AT_ACTION_OP:
        case AT_READ_OP:
        case AT_TEST_OP:
		case AT_SET_OP:
			retval = system_check_fboot();
            sprintf(response, "\r\n%d\r\nOK\r\n\r\n", retval);
            break;
        default:
		    ALOGD("unknown at_op :%d", at_op);
            break;
    }
    
    return retval;
}

int system_restart_handler(char* cmdline, ATOP_t at_op, char* response){    
    int retval = 0, err = 0;
	char tok[128];
    
    ALOGD("[%s]handle cmdline:%s", __FUNCTION__, cmdline);

	if (cmdline!=NULL)
		err = cmdline[0] == '\0' ? 0 : 1;
	
    switch(at_op){
        case AT_ACTION_OP:
        case AT_READ_OP:
        case AT_TEST_OP:
		case AT_SET_OP:
			if (at_op == AT_SET_OP && err) //has character next to =
	            sprintf(response, "\r\n\r\n%s\r\n\r\n", RESTART_FAIL);
			else {
				invoke_restart();
				sprintf(response, "\r\n\r\n%s\r\n\r\n", RESTART_PASS);	
			}
			break;
        default:
		    ALOGD("unknown at_op :%d", at_op);
            break;
    }
    
    return retval;
}


int
imei_cmd_process(
    bool isMaster,
    char* cmdline,
    ATOP_t at_op,
    char* response
    )
{
    int i, valid_num;
    unsigned char imeiVal[IMEI_LENGTH];
    int ret = -1;

    if(at_op == AT_SET_OP) {
        /* Assign Commands */
        /* zeroize */
        memset(imeiVal, 0, sizeof(char) * IMEI_LENGTH);

        for(i = 0, valid_num = 0 ; valid_num < IMEI_LENGTH && cmdline[i] != '\0' ; i++) {
            switch(cmdline[i]) {
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
            case '0':
                imeiVal[valid_num] |= cmdline[i] - '0';
                valid_num++;
                break;

            case ',':
                continue;

            default:
                break;
            }
        }
//Haman changed for TC3
/*
        if(LGE_FacWriteImei(isMaster, imeiVal, true) == true) {
            sprintf(response, "\r\nIMEI WRITE OK\r\n\r\n");
        }
        else {
            sprintf(response, "\r\nIMEI WRITE FAILURE\r\n\r\n");
        }
*/
//change end
        ret = 0;
    }
    /* query for IMEI */
    else if(at_op == AT_NONE_OP
            || at_op == AT_READ_OP
            || at_op == AT_ACTION_OP) {
        if(
//Haman changed for TC3
//LGE_FacReadImei(isMaster, imeiVal) == true
//||
false
//end change
) {
            sprintf(response,
                    "\r\n%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d\r\nOK\r\n\r\n", 
                    imeiVal[0], 
                    imeiVal[1],
                    imeiVal[2],
                    imeiVal[3],
                    imeiVal[4],
                    imeiVal[5],
                    imeiVal[6],
                    imeiVal[7],
                    imeiVal[8],
                    imeiVal[9],
                    imeiVal[10],
                    imeiVal[11],
                    imeiVal[12],
                    imeiVal[13],
                    imeiVal[14]                                                          
                    );
        }
        else {
            sprintf(response, "\r\nPlease try again (not ready)\r\n\r\n");
        }

        ret = 0;
    }
    else if(at_op == AT_TEST_OP) {
        sprintf(response, "\r\nATMAC=[MAC ADDR : 12 HEX nibble => 6 Bytes]\r\nOK\r\n\r\n");
        ret = 0;
    }

    if(ret != 0) {
        /* generate error message */
        sprintf(response, "\r\n\r\nNOT IMPLEMENTED\r\n\r\n");
    }

    return ret;
}

int
misc_imei_cmd_handler(
    char* cmdline,
    ATOP_t at_op,
    char* response
    )
{
	return imei_cmd_process(true, cmdline, at_op, response);
}

int
misc_imei2_cmd_handler(
    char* cmdline,
    ATOP_t at_op,
    char* response
    )
{
	return imei_cmd_process(false, cmdline, at_op, response);
}
