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

#include "atci_mmc_cmd.h"
#include "atci_service.h"
#include "atcid_util.h"

#include <unistd.h>
#include <sys/vfs.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

#define  MMC_DIR                "/mnt/sdcard"
#define  MMC_OPR_FAIL          -1
#define  MMC_OPR_SUCCESS        0
#define  MMC_DEV                 "/dev/block/mmcblk1"

static  int  mmc_operation(char *opr_code) {
    int   state;
    pid_t child;

    if (access(MMC_DEV, F_OK) < 0) {
        ALOGE("AT_MMC_CMD " "The SD/MMC device isn't exist.");
        return MMC_OPR_FAIL;
    }

    if (NULL == opr_code) {
        ALOGE("AT_MMC_CMD " "Operation has not defined.");
        return MMC_OPR_FAIL;
    }

    child = fork();
    if (child < 0) {
        ALOGE("AT_MMC_CMD " "Fork fail.");
        return MMC_OPR_FAIL;
    }

    if (child == 0) {
        if (execl("/system/bin/vdc", "vdc", "volume", opr_code, "/mnt/sdcard", NULL) < 0) {
            ALOGE("AT_MMC_CMD " "Child process execl fail.");
            exit(1);
        }
    } else {
        waitpid(child, &state, 0);
        if (WIFEXITED(state) && !WEXITSTATUS(state)) {
            ALOGE("AT_MMC_CMD " "Child process %s SD/MMC successfully.", opr_code);
            return MMC_OPR_SUCCESS;
        } else {
            ALOGE("AT_MMC_CMD " "Child process %s SD/MMC fail.", opr_code);
            ALOGE("AT_MMC_CMD " "WIFEXITED    = %d\n", WIFEXITED(state));
            ALOGE("AT_MMC_CMD " "WEXITSTATUS  = %d\n", WEXITSTATUS(state));
            ALOGE("AT_MMC_CMD " "WIFSIGNALED  = %d\n", WIFSIGNALED(state));
            ALOGE("AT_MMC_CMD " "WTERMSIG     = %d\n", WTERMSIG(state));
            ALOGE("AT_MMC_CMD " "WCOREDUMP    = %d\n", WCOREDUMP(state));
            ALOGE("AT_MMC_CMD " "WIFSTOPPED   = %d\n", WIFSTOPPED(state));
            ALOGE("AT_MMC_CMD " "WSTOPSIG     = %d\n", WSTOPSIG(state));
            return MMC_OPR_FAIL;
        }
    }

    return MMC_OPR_FAIL;
}

int mmc_chk_handler(char* cmdline, ATOP_t at_op, char* response){
    ALOGE("handle cmdline:%s", cmdline);

    if (access(MMC_DEV, F_OK) < 0) {
        ALOGE("AT_MMC_CMD " "The SD/MMC device isn't exist.");
        sprintf(response,"\r\nERROR\r\n\r\n");
        return 0;
    }
    else{
        sprintf(response,"\r\nOK\r\n\r\n");
    }

    return 0;
}

int mmc_format_handler(char* cmdline, ATOP_t at_op, char* response){
    int    ret  = 0;
    int    flag = 0;

    ALOGE("AT_MMC_CMD " "handle cmdline:%s", cmdline);

    if (access(MMC_DEV, F_OK) < 0) {
        ALOGE("AT_MMC_CMD " "The SD/MMC device isn't exist.");
        sprintf(response,"\r\nERROR\r\n\r\n");
        return 0;
    }

    switch(at_op){
    case AT_ACTION_OP:
        if (mmc_operation("unmount") == MMC_OPR_SUCCESS)
            flag = 1;

        ret = mmc_operation("format");
        sprintf(response,"\r\n%s\r\n\r\n", (ret == MMC_OPR_SUCCESS)?"OK":"ERROR");

        if (flag)
            mmc_operation("mount");

        break;
    case AT_READ_OP:
    case AT_TEST_OP:
    case AT_SET_OP:
        sprintf(response,"\r\nNot implemented\r\n\r\n");

    default:
        break;
    }

    return 0;
}

int mmc_totalsize_handler(char* cmdline, ATOP_t at_op, char* response){
    int              ret   = 0;
    unsigned  long   total = 0;
    struct  statfs   fs_info;

    ALOGE("AT_MMC_CMD " "handle cmdline:%s", cmdline);

    if (access(MMC_DEV, F_OK) < 0) {
        ALOGE("AT_MMC_CMD " "The SD/MMC device isn't exist.");
        sprintf(response,"\r\nERROR\r\n\r\n");
        return 0;
    }

    ret = statfs(MMC_DIR, &fs_info);
    if (ret == -1) {
        sprintf(response,"\r\nERROR\r\n\r\n");
        return 0;
    }

    if (fs_info.f_blocks == 0) { // May the card has not been mounted
        if (mmc_operation("mount") == MMC_OPR_SUCCESS) {
            ret = statfs(MMC_DIR, &fs_info);
            if (ret == -1) {
                sprintf(response,"\r\nERROR\r\n\r\n");
                return 0;
            }
            mmc_operation("unmount");
        } else
            ALOGE("AT_MMC_CMD " "May the card has not been inserted.");
    }

    total = fs_info.f_blocks * fs_info.f_frsize;

    switch(at_op){
    case AT_ACTION_OP:
    case AT_READ_OP:
    case AT_TEST_OP:
    case AT_SET_OP:
        sprintf(response,"\r\n%lu Byte\r\nOK\r\n\r\n", total);
        break;
    default:
        break;
    }
    return 0;
}

int mmc_usedsize_handler(char* cmdline, ATOP_t at_op, char* response){
    int            ret   = 0;
    unsigned  long total = 0;
    unsigned  long free  = 0;
    unsigned  long used  = 0;
    struct  statfs fs_info;

    ALOGE("AT_MMC_CMD " "handle cmdline:%s", cmdline);

    if (access(MMC_DEV, F_OK) < 0) {
        ALOGE("AT_MMC_CMD " "The SD/MMC device isn't exist.");
        sprintf(response,"\r\nERROR\r\n\r\n");
        return 0;
    }

    ret = statfs(MMC_DIR, &fs_info);
    if (ret == -1) {
        sprintf(response,"\r\nERROR\r\n\r\n");
        return 0;
    }

    if (fs_info.f_blocks == 0) { // May the card has not been mounted
        if (mmc_operation("mount") == MMC_OPR_SUCCESS) {
            ret = statfs(MMC_DIR, &fs_info);
            if (ret == -1) {
                sprintf(response,"\r\nERROR\r\n\r\n");
                return 0;
            }
            mmc_operation("unmount");
        } else
            ALOGE("AT_MMC_CMD " "May the card has not been inserted.");
    }

    total = fs_info.f_blocks * fs_info.f_frsize;
    free  = fs_info.f_bfree  * fs_info.f_frsize;
    used  = total - free;

    switch(at_op){
    case AT_ACTION_OP:
    case AT_READ_OP:
    case AT_TEST_OP:
    case AT_SET_OP:
        sprintf(response,"\r\n%lu Byte\r\nOK\r\n\r\n", used);
        break;
    default:
        break;
    }

    return 0;
}
