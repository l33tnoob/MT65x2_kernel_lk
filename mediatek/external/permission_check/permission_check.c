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
/*****************************************************************************
 *
 * Filename:
 * ---------
 *   
 *
 * Project:
 * --------
 *   
 *
 * Description:
 * ------------
 *   
 *
 * Author:
 * -------
 *   
 *
 ****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cutils/properties.h>
#include <android/log.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <stdlib.h>

#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, "permission_check",__VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG  , "permission_check",__VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO   , "permission_check",__VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN   , "permission_check",__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR  , "permission_check",__VA_ARGS__)

/* 
 * Notice
 * For origin design, ccci_fsd may create files and folders whose attribute is "0000, root.root".
 * In order to make cell phone more safe, ccci_fsd change its user from root to ccci.
 * For, MOTA update, data patition will not be ereased; then, ccci_fsd loss the capability to read/write md nvram.
 * EE will occur. So, we modify md nvram files's attribute fisrt here
 */
void change_md_nvram_attr(void)
{
    struct stat statbuf;
    LOGD("change_md_nvram_attr++");
    if(stat("/data/nvram/md/new_ver", &statbuf) == 0){
        LOGD("new_ver file exist!!!");
	LOGD("change_md_nvram_attr--0");
        return;
    }

    LOGD("new_ver file NOT exist, change attr");
    umask(0007);
    // Begin to change file mode and group
    //system("chmod 0770 /data/nvram/md");
    system("chmod 0770 /data/nvram/md/NVRAM");
    system("chmod 0770 /data/nvram/md/NVRAM/NVD_IMEI");
    system("chmod 0770 /data/nvram/md/NVRAM/IMPORTNT");
    system("chmod 0770 /data/nvram/md/NVRAM/CALIBRAT");
    system("chmod 0770 /data/nvram/md/NVRAM/NVD_CORE");
    system("chmod 0770 /data/nvram/md/NVRAM/NVD_DATA");
    system("chmod 0660 /data/nvram/md/NVRAM/NVD_IMEI/*");
    system("chmod 0660 /data/nvram/md/NVRAM/IMPORTNT/*");
    system("chmod 0660 /data/nvram/md/NVRAM/CALIBRAT/*");
    system("chmod 0660 /data/nvram/md/NVRAM/NVD_CORE/*");
    system("chmod 0660 /data/nvram/md/NVRAM/NVD_DATA/*");
    
    system("chmod 0777 /data/mdl");
    system("chmod 0777 /data/mdl/*");
    
    system("chmod 0775 /data/nvram/dm");
    system("chmod 0775 /data/nvram/dm/*");
    
    // Make sure files has correct owner and group
    system("chown root.nvram /data/nvram/md");
    system("chown root.nvram /data/nvram/md/NVRAM");
    system("chown root.nvram /data/nvram/md/NVRAM/NVD_IMEI");
    system("chown root.nvram /data/nvram/md/NVRAM/IMPORTNT");
    system("chown root.nvram /data/nvram/md/NVRAM/CALIBRAT");
    system("chown root.nvram /data/nvram/md/NVRAM/NVD_CORE");
    system("chown root.nvram /data/nvram/md/NVRAM/NVD_DATA");
    system("chown root.nvram /data/nvram/md/NVRAM/NVD_IMEI/*");
    system("chown root.nvram /data/nvram/md/NVRAM/IMPORTNT/*");
    system("chown root.nvram /data/nvram/md/NVRAM/CALIBRAT/*");
    system("chown root.nvram /data/nvram/md/NVRAM/NVD_CORE/*");
    system("chown root.nvram /data/nvram/md/NVRAM/NVD_DATA/*");
    system("chown shell.shell /data/mdl");
    system("chown shell.shell /data/mdl/*");
    
    system("chown system.system /data/nvram/dm");
    system("chown system.system /data/nvram/dm/*");

    system("echo flag > /data/nvram/md/new_ver");
    system("chmod 0660 /data/nvram/md/new_ver");
    LOGD("change_md_nvram_attr--1");
}

int main(int argc, char **argv)
{
	int  need_check = 0;
	char property_val[256] = {0};

	LOGD("permission check ver:0.01");

	// Check whether is at decrypt state
	property_get("vold.decrypt", property_val, NULL);

	if(strcmp(property_val, "") == 0){
		LOGD("Empty, not decrypt!\n");
		need_check = 1;
	} else if(strcmp(property_val, "trigger_restart_framework") == 0){
		LOGD("decrypt done!\n");
		need_check = 1;
	}

	if(need_check==0){
		LOGD("decrypt, do not check!\n");
		return 0;
	}

	change_md_nvram_attr();

	return 0;
}

