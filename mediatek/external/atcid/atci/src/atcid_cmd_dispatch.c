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

#include "atcid_serial.h"
#include "atcid_cmd_dispatch.h"
#include "atcid_cust_cmd_process.h"
#include "atcid_bsp_cmd.h"
#include <string.h>
#include "atci_battery_cmd_dispatch.h"
#include "stdio.h"



int handleBspCmd(char* cmdData, int cmdId){
    cmd_data_type cmd_data;
    int fd = -1, result = -1;
    ATOP_t at_op;
    char response[MAX_AT_RESPONSE];
    
    LOGATCI(LOG_DEBUG,"Enter with cmdId:%d", cmdId);
    
    at_op = getAtOp(&cmdData);
    LOGATCI(LOG_DEBUG, "The command op is %d; Support op is %d", at_op, bsp_cmd_table[cmdId].opType);
    memset(response, 0, sizeof(response));

    if((at_op & bsp_cmd_table[cmdId].opType) != 0){
        //Open the FD to handle
        LOGATCI(LOG_DEBUG, "Open FD: %s", bsp_cmd_table[cmdId].devPath);
        fd = open(bsp_cmd_table[cmdId].devPath, O_RDWR);
        if(fd < 0){
            LOGATCI(LOG_ERR, "Fail to open FD with errno:%d", errno);
            writeDataToserialByResponseType(AT_ERROR);
            return 0;
        }
        
        //Initalize the cmd data structure
        memset(cmd_data.cmdDataRequest, 0, sizeof(cmd_data.cmdDataRequest));
        memset(cmd_data.cmdDataResponse, 0, sizeof(cmd_data.cmdDataResponse));
        strcpy(cmd_data.cmdDataRequest, cmdData);
        cmd_data.opType = at_op;
                
        //Send IOControl to device driver
        result = ioctl(fd, bsp_cmd_table[cmdId].cmdId, &cmd_data);
        if(result < 0){
            LOGATCI(LOG_ERR, "Fail to IO Control[%d] with errno:%d", bsp_cmd_table[cmdId].cmdId, errno);
            writeDataToserialByResponseType(AT_ERROR);
            return 0;
        }
        
        if(strlen(cmd_data.cmdDataResponse) > 0){
           sprintf(response, "\r\n%s\r\n", cmd_data.cmdDataResponse);
           writeDataToSerial(response, strlen(response));
        }
    }else{
        writeDataToserialByResponseType(AT_NOT_IMPL);
    }
        
    return 0;    
}

int generic_command_hdlr(char* line){
    int i = 0;
    int table_size = 0;
    char* line_cut = cut_cmd_line(line);
    
    table_size = (sizeof(generic_cmd_table)/sizeof(generic_cmd_type));
    
    LOGATCI(LOG_DEBUG, "The size of generic_cmd_table is %d", table_size);
    for(i = 0; i < table_size;i++){
        if(strcmp(line_cut, generic_cmd_table[i].cmdName) == 0){
            //Write the AT command to generic service by socket interface
            free(line_cut);
            return 1;
        }
    }
    free(line_cut);
    return 0;
}

int custom_command_hdlr(char* line){
    int i = 0, ret = 0;
    int table_size = 0;
    char response[MAX_AT_RESPONSE];
    char* line_cut = cut_cmd_line(line);
    
    memset(response, 0, sizeof(response));
    table_size = (sizeof(custom_cmd_table)/sizeof(customcmd_type));
    
    
    for(i = 0; i < table_size;i++){
		LOGATCI(LOG_DEBUG, "custom_cmd_table[%d].cmdName = %s", i, custom_cmd_table[i].cmdName);
        if(strcmp(line_cut, custom_cmd_table[i].cmdName) == 0){
            ATOP_t at_op = getAtOp(&line);
            LOGATCI(LOG_DEBUG, "The command op is %d; Support op is %d", at_op, custom_cmd_table[i].opType);
            if((at_op & custom_cmd_table[i].opType) == 0){
                writeDataToserialByResponseType(AT_NOT_IMPL);
            }else{
                ret = custom_cmd_table[i].cmd_handle_func(line, at_op, response);
                if(strlen(response) > 0){
                    writeDataToSerial(response, strlen(response));
                }
                
                if(ret != AT_NONE_OP){
                    writeDataToserialByResponseType(ret);
                }
            }
            free(line_cut);
            return 1; //The command is handled here
        }
    }
    free(line_cut);
    return 0;
}

int bsp_command_hdlr(char* line){
    int i = 0, ret = 0;
    int table_size = 0;
    char response[MAX_AT_RESPONSE];
    char* line_cut = cut_cmd_line(line);
    
    memset(response, 0, sizeof(response));
    table_size = (sizeof(bsp_cmd_table)/sizeof(bsp_cmd_type));
        
    for(i = 0; i < table_size;i++){
        if(strcmp(line_cut, bsp_cmd_table[i].cmdName) == 0){
            handleBspCmd(line, i);
            free(line_cut);
            return 1; //The command is handled here
        }
    }
    free(line_cut);
    return 0;
}

int audio_command_hdlr(char* line) {
	if (strncmp(line, "AUD", 3) == 0 || strncmp(line, "aud", 3) == 0 ) {
        return 3;
    }
    if (strncmp(line, "MM", 2) == 0 || strncmp(line, "mm", 2) == 0) {
        return 2;
    }
	return 0;
}

int handleBatCmd(char* line, int cmdId) {
	int result = -1;
	int param = -1;
	char response[MAX_AT_RESPONSE];
	LOGATCI(LOG_DEBUG, "line: %s", line);
	switch(cmdId) {
	case 0:
		result = AT_get_charging_state_flag();
		break;
	case 1:
		result = AT_get_bat_voltage();
		break;
	case 2:
		result = AT_set_Charger_Current(CUT_CHARGER_CURRENT);
		break;
	default:
		LOGATCI(LOG_DEBUG, "atcid not support this at command: %s", line);
	}
	if(result != -1) {
		sprintf(response, "%d\r\nOK\r\n\0", result);
		writeDataToSerial(response, strlen(response));
	}
	else {
		sprintf(response, "ERROR\r\n\0");
		writeDataToSerial(response, strlen(response));
	}
	return result;
}

int bat_command_hdlr(char* line) {
	int i = 0, ret = 0;
    int table_size = 0;
    char* line_cut = cut_cmd_line(line);
	int result = -1;
    
    table_size = (sizeof(bat_cmd_table)/sizeof(generic_cmd_type));
        
    for(i = 0; i < table_size;i++){
        if(strcmp(line_cut, bat_cmd_table[i].cmdName) == 0){
            handleBatCmd(line, i);
            free(line_cut);
            return 1; //The command is handled here
        }
    }
    free(line_cut);
	return 0;
}

int process_cmd_line(char* line){    
    LOGATCI(LOG_DEBUG,"Enter");
    
    //Check the command is belonged to customized command table
    LOGATCI(LOG_DEBUG, "handle in custom_command_hdlr");
    if(custom_command_hdlr(line)){
       return ATCI_TYPE;
    }
    
    LOGATCI(LOG_DEBUG, "handle in generic_command_hdlr");
    if(generic_command_hdlr(line)){
       return GENERIC_TYPE;
    }
    
    LOGATCI(LOG_DEBUG, "handle in bsp_command_hdlr");
    if(bsp_command_hdlr(line)){
       return PLATFORM_TYPE;
    }

	LOGATCI(LOG_DEBUG, "handle in audio_command_hdlr");
	if(audio_command_hdlr(line)) {
		return AUDIO_TYPE;
	}

	LOGATCI(LOG_DEBUG, "handle in bat_command_hdlr");
	if(bat_command_hdlr(line)) {
		return BATTERY_TYPE;
	}
        
    return RIL_TYPE;
}

char* cut_cmd_line(char* line) 
{
    char* result;
    int i = 0;
    int size = strlen(line);
    int len = size;
    for(i = 0; i < size; i++) 
    {
        if(line[i] == '=' || line[i] == '?')
        {
            len = i;
            break;
        }
    }
    result = (char*)malloc((len + 1) * sizeof(char));
    memcpy(result, line, len);
    result[len] = '\0';
    return result;
}
