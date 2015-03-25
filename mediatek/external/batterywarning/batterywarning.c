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

#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, "battery_warning",__VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG  , "battery_warning",__VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO   , "battery_warning",__VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN   , "battery_warning",__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR  , "battery_warning",__VA_ARGS__)
#define MAX_CHAR 100
#define MAX_LENGTH 1024
#define CMD "ps -t | grep ActivityManager"
#define FILE_NAME "/sys/devices/platform/mt-battery/BatteryNotify"
#define INTENT "am broadcast -n com.mediatek.batterywarning/com.mediatek.batterywarning.BatteryWarningReceiver -a android.intent.action.BATTERY_WARNING --ei type "

int systemServerStarted(const char *cmd)   
{
    LOGD("systemServerStarted: %s \n", cmd);  
    int res = 0;
    char buf_ps[MAX_LENGTH];
    char ps[MAX_LENGTH]={0};
    FILE *ptr;   
    strcpy(ps, cmd);   
    if((ptr=popen(ps, "r")) != NULL)   
    {   
        while(fgets(buf_ps, MAX_LENGTH, ptr) != NULL)   
        {
           LOGD("cmd result: %s", buf_ps);    
           if(strlen(buf_ps) > 0)
           {
               res = 1;   
               break;  
           } 
        }   
        pclose(ptr);   
        ptr = NULL;   
    }   
    else  
    {
        LOGE("popen error");
    } 

    return res;  
}

void readType(char* buffer) {
   FILE * pFile;
    pFile = fopen(FILE_NAME, "r");
    if(pFile == NULL) {
        LOGE("error opening file");
        return;
    } else {
        if(fgets(buffer, MAX_CHAR, pFile) == NULL) {
            LOGE("can not get the string from the file");
            return;
        }
    }
    int type = atoi(buffer);
    if (type > 0 && systemServerStarted(CMD))
    {
        LOGD("start activity by send intent to BatteryWarningReceiver, type = %s", buffer);
        char ps[MAX_LENGTH] = INTENT;
        strcat(ps,buffer);
        LOGD("os.system:  %s", ps);
        system(ps);
    }
    fclose(pFile);
}

int main(int argc, char **argv)
{
    char *buffer = (char*) malloc(MAX_CHAR * sizeof(char));
    while(1) {
        readType(buffer);
        sleep(10);
    }
    free(buffer); 
	return 0;
}

