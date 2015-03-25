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

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include <stdint.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>

#include "atci_mjc_cmd.h"
#ifdef MTK_CLEARMOTION_SUPPORT
#include "mjc_kernel_driver.h"
#endif
#include "atci_service.h"
#include "atcid_util.h"

#define SW_REG_BASE 0x5000
#define SW_REG_CEIL 0x6000
#define SW_REG_FILE_PATH  "/sdcard/SW_REG"

//4 types of operation codes: ACTION / TEST(=?) / READ(?) / SET(=)

int drvHandle = -1;

#ifdef MTK_CLEARMOTION_SUPPORT
int mjc_cmd_handler(char* cmdline, ATOP_t at_op, char* response)
{
	ALOGD("mjc cmd handler handles cmdline:%s", cmdline);

    int actionID=0, RegBase = 0, RegValue = 0, err = 0;
	char fileName[256];
    FILE *fp;
    int num;
	if(drvHandle == -1) //initial
		drvHandle = open("/dev/MJC", O_RDONLY, 0);
	
	switch(at_op){
		case AT_ACTION_OP:
		case AT_TEST_OP:
		case AT_READ_OP:
			sprintf(response,"\r\nPQ OK\r\n");
			break;
			
		case AT_SET_OP:
			
			at_tok_nexthexint(&cmdline, &actionID);
			
			if(actionID == 0) //action: READ
			{
				MJC_READ_REG_T params;
				unsigned int value;
				
				at_tok_nexthexint(&cmdline, &RegBase);

				params.reg = RegBase;// For compatible
				params.val = &value;
				params.mask = 0xFFFFFFFF;
                
				if((RegBase>=SW_REG_BASE)&&(RegBase<=SW_REG_CEIL)&&(!(RegBase%4)))
                {            
                    fp = fopen(SW_REG_FILE_PATH, "rb");
                    if(fp)
                    {
                        fseek(fp, RegBase-SW_REG_BASE, SEEK_SET);
            	        fread(&value, 1, sizeof(int), fp);                     
                        fclose(fp);  
                    }
                    else{
                        ALOGD("open SW_REG fail\n");                        
                    }
                }
                else{
				    ioctl(drvHandle, MJC_READ_REG, &params);                    
                }                    
				sprintf(response,"%x", value);
                          //ALOGD("MJC_DEBUG R: 0x%x 0x%x",params.reg,value);
			}
			else if(actionID == 1)//action: WRITE
			{	
				MJC_WRITE_REG_T params;
				unsigned int value;
					
				at_tok_nexthexint(&cmdline, &RegBase);
				at_tok_nexthexint(&cmdline, &RegValue);

                          if(RegBase == 0xEEEEEEEE)
                          {
                            if(RegValue == 0)
                            {
                                ALOGD("MJC_DEBUG SCE program start...\n");
                                value = 1;
                            }
                            else
                            {
                                ALOGD("MJC_DEBUG SCE program done!\n");
                                value = 2;
                            }
                          }
                          else
                          {
                              params.reg = RegBase;//(RegBase | 0xF0000000); // For compatible
                              params.val = RegValue;
                              params.mask = 0xFFFFFFFF;
                              if((RegBase>=SW_REG_BASE)&&(RegBase<=SW_REG_CEIL)&&(!(RegBase%4)))
                              {
                                fp = fopen(SW_REG_FILE_PATH, "rb+");
                                if (fp)
                                {                                  
                                    fseek(fp, RegBase-SW_REG_BASE, SEEK_SET);
                 	                fwrite((void*)&RegValue, 1, sizeof(int), fp);                 	            
                                    fclose(fp);
                                }else{
                                    ALOGD("open SW_REG fail\n");                                  
                                }
                              }
                              else
                              {
                                ioctl(drvHandle, MJC_WRITE_REG, &params);
                              }                                
                              //ALOGD("MJC_DEBUG W: 0%x = 0%x",params.reg,RegValue);
                          }
                          
				sprintf(response,"%x", value);
			}

			break;
		default:
			break;
	}
		
	return 0;
}
#else
int mjc_cmd_handler(char* cmdline, ATOP_t at_op, char* response)
{
	ALOGD("mjc not support");
    return 0;
}
#endif
