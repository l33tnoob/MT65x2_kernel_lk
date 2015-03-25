/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2008
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <sys/mman.h>

#include "meta_lcdbk.h"
#include <utils/Log.h>

#undef LOG_TAG 
#define LOG_TAG "META"

//#define USE_PMIC_IOCTL

#ifdef USE_PMIC_IOCTL
#define TEST_PMIC_PRINT 0
#define PMIC_READ 1
#define PMIC_WRITE 2
#define SET_PMIC_LCDBK 3

int set_lcdbk_data[1] = {32}; /* brightness*/
int meta_lcdbk_fd = 0;
#endif

int adwBrightness[10] = {255, 0, 30, 65, 100, 135, 170, 205, 240, 255};

LCDLevel_CNF Meta_LCDBK_OP(LCDLevel_REQ dwBrightness)	
{
#ifdef USE_PMIC_IOCTL
	int dwVurrentBrightDuty;
	int ret;
	LCDLevel_CNF lcdbk_cnf;
	lcdbk_cnf.status=true;	

	/* CHecking the range of  lcd_light_level */
	if ( dwBrightness.lcd_light_level < 0 || dwBrightness.lcd_light_level > 9 ) {
		printf("brightness is not correct ! \n");
		lcdbk_cnf.status=false;
		return lcdbk_cnf;
	}
	/* open file */
	meta_lcdbk_fd = open("/dev/MT6326-pmic",O_RDWR, 0);
	if (meta_lcdbk_fd == -1) {
		printf("Open /dev/MT6326-pmic : ERROR \n");
		lcdbk_cnf.status=false;
		return lcdbk_cnf;
	}	

	/* Gemini Phone */
	dwVurrentBrightDuty = adwBrightness[dwBrightness.lcd_light_level];
	if(dwVurrentBrightDuty >=32)
		dwVurrentBrightDuty=0;
	set_lcdbk_data[0]=dwVurrentBrightDuty;
	ret = ioctl(meta_lcdbk_fd, SET_PMIC_LCDBK, set_lcdbk_data);
	if (ret == -1) {
		printf("Meta_LCDBK_OP : ERROR \n");
		lcdbk_cnf.status=false;					
	}
	
	//printf("Meta_LCDBK_OP : Set Brightness %d \n", set_lcdbk_data[0]);

	close(meta_lcdbk_fd);
	
	return lcdbk_cnf;
#else // !USE_PMIC_IOCTL
	LCDLevel_CNF lcdbk_cnf;
	int fd = -1, level;
#define BUF_LEN 16
	char wbuf[BUF_LEN] = {'\0'};
	char rbuf[BUF_LEN] = {'\0'};

	lcdbk_cnf.status = false;

#define BRIGHTNESS_FILE "/sys/class/leds/lcd-backlight/brightness"
	fd = open(BRIGHTNESS_FILE, O_RDWR, 0);
	if (fd == -1) {
		LOGE("Can't open %s\n", BRIGHTNESS_FILE);
		goto EXIT;
	}
	level = adwBrightness[dwBrightness.lcd_light_level];
	sprintf(wbuf, "%d\n", level);
	if (write(fd, wbuf, strlen(wbuf)) == -1) {
		LOGE("Can't write %s\n", BRIGHTNESS_FILE);
		goto EXIT;
	}
	close(fd);
	fd = open(BRIGHTNESS_FILE, O_RDWR, 0);
	if (fd == -1) {
		LOGE("Can't open %s\n", BRIGHTNESS_FILE);
		goto EXIT;
	}
	if (read(fd, rbuf, BUF_LEN) == -1) {
		LOGE("Can't read %s\n", BRIGHTNESS_FILE);
		goto EXIT;
	}
	if (!strncmp(wbuf, rbuf, BUF_LEN))
		lcdbk_cnf.status = true;

EXIT:
		if (fd != -1)
			close(fd);
		return lcdbk_cnf;
#endif
}

BOOL Meta_LCDBK_Init()
{
	return true;
}

BOOL Meta_LCDBK_Deinit()
{
	return true;
}

