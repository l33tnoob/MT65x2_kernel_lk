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

/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <getopt.h> /* optind */
#include <fcntl.h>
#include <ctype.h>
#include <cutils/log.h>
#include "sdiotool.h"

int main(int argc, char **argv)
{ 
	static const char opts[] = "cdegs";
 	DIR*    dir;
	FILE *msdc_FT_file;
	FILE *msdc_debug_file;

	char buffer_file[PATH_MAX];
	int ch,option=0,par=0,par1=0,par2=0;
	char rd_line[256];
	int i_data,i_delay,i_clk, i_driving, i_edge;

	while ((ch = getopt(argc, argv, opts)) != -1)
	{
		switch (ch) {
			case 'g':  //get current settings
	    	option = 1;
	    	break;
			case 's': //set data delay
				option = 2;
				par = 3;
				break;
			case 'c': //set clk
				option = 2;
				par = 1;
				break;
			case 'd': //set driving current
				option = 2;
				par = 2;
				break;
			case 'e': //set edge
				option = 2;
				par = 4;
				break;
			//case 'h':
			//	option = 3;
			//	break;
			default:
	    	usage();
		}
	}
//	printf("argc=%d option=%d \n",argc,option);
	if (!((argc ==2) || (argc == 3) || (argc == 4)))
		usage();
	
	if( option == 1)
	{	
			if (!(dir = opendir("/proc")))
			{
				printf("opendir(proc) failed (%s)", strerror(errno));
				return 0;
			}
		  
			msdc_FT_file = fopen("/proc/msdc_FT", "r");
			if (!msdc_FT_file)
			{
					printf("!msdc_FT_file\n");
					
					msdc_debug_file = fopen("/proc/msdc_debug", "r");
					if (!msdc_debug_file)
					{
						printf("Kernel doesn't support based on your SW version \n");
						return 0;
					}
					else
					{
						while(!feof(msdc_debug_file))
						{
							fgets(buffer_file, sizeof(buffer_file), msdc_debug_file);
							printf("%s",buffer_file);
						}	
						fclose(msdc_debug_file);
					}
			}
			else
			{	
					while(!feof(msdc_FT_file))
					{
						fgets(buffer_file, sizeof(buffer_file), msdc_FT_file);
						printf("%s",buffer_file);
					}
					fclose(msdc_FT_file);
			}
	}
	
	if( option == 2)
	{
		if (argv[2]==NULL)
			{
				usage();
			  return 0;
			}
			
		if (par==3)
		{
			if (argv[3]==NULL)
			{
				usage();
			  return 0;
			}
		}
			
//		printf("argv[2]=%s  %d\n",argv[2],atoi(argv[2]));
//if (par==3)		
//	  printf("argv[3]=%s  %d\n",argv[3],atoi(argv[3]));
		
		if (!(dir = opendir("/proc")))
			{
				printf("opendir(proc) failed (%s)", strerror(errno));
				return 0;
			}
		
		if (par==1)//set clk
		{
				if ((atoi(argv[2])==0) || (atoi(argv[2])==1))
				{
					i_clk = atoi(argv[2]);
					par1 = i_clk;
				}
				else
				{
					usage();
		  		return 0;
				}
		}
		else if (par==2)//set driving current
		{
				if ((atoi(argv[2])>=0) && (atoi(argv[2])<=7))
				{
					i_driving = atoi(argv[2]);
					par1 = i_driving;
				}
				else
				{
					usage();
		  		return 0;
				}
		}
		else if (par==3)//set data delay
		{
				if ((atoi(argv[2])>=0) && (atoi(argv[2])<=5)) //data bit 0~3
				{
						i_data = atoi(argv[2]);
						par1 = i_data;
					
						if ((atoi(argv[3])>=0) && (atoi(argv[3])<=31)) //delay 0~31
						{
								i_delay = atoi(argv[3]);
								par2 = i_delay;
						}
						else
						{
							usage();
		  				return 0;
						}
				}
				else
				{
					usage();
		  		return 0;
				}
		}
		else if (par==4)//set edge
		{
				if ((atoi(argv[2])==0) || (atoi(argv[2])==1))
				{
					i_edge = atoi(argv[2]);
					par1 = i_edge;
				}
				else
				{
					usage();
		  		return 0;
				}
		}
		else
		{
			usage();
		  return 0;
		}

		sprintf(rd_line, "echo %d %d %d >/proc/msdc_FT", par, par1, par2);

// 	printf("cmd: %s \n",rd_line);

		if ((msdc_FT_file = popen(rd_line, "r")) == NULL)
		{
				printf("Can't set the new SDIO settings. \n");
		}
		else
		{
				printf("Set the new SDIO settings. \n");
				printf("Please use [sidotool -g] to get new settings. \n");
		}

	}	
	
	closedir(dir);
	return 0;
}
/*
 * Print usage message.
 */
static void
usage(void)
{
	fprintf(stderr,
	    "usage: sdiotool [ -options ] value\n"
	    "where the options are:\n"
	    "\t-g Get Current Settings [sdiotool -g]\n"
	    "\t-c Set Clock [sdiotool -c 0] 1:Internal clk; 0:Feedback clk\n"
	    "\t-d Set Driving Current [sdiotool -d 1] Range: 0~7\n"
	    "\t-s Set Read Data Delay [sdiotool -s 1 10] 1st:data bit(0~3); 2nd:value(0~31)\n"
	    "\t-s Set Write Data Delay [sdiotool -s 4 X] X:0~31\n"
	    "\t-s Set Cmd Data Delay [sdiotool -s 5 X] X:0~31\n"
	    "\t-e Set Edge [sdiotool -e 1] 1:FALLING; 0:RISING\n");
	exit(1);
}
