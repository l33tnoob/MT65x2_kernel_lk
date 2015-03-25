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

#define LOG_TAG "AudioRegSetting"

typedef unsigned int UINT32;
typedef unsigned short  UINT16;

#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <signal.h>
#include <binder/IPCThreadState.h>
#include <binder/MemoryBase.h>

#include "AudioIoctl.h"

namespace android
{

};//namespace android

using namespace android;

static char const *const kAudioDeviceName = "/dev/eac";

static void SetAudSysReg(int input, int mFd)
{
    Register_Control Regdata;
    memset((void *)&Regdata, 0, sizeof(Regdata));
    printf("please enter offset with hex \n");
    scanf("%x", &input);
    Regdata.offset = input;
    printf("offset = %x\n", Regdata.offset);
    printf("please enter value with hex mask = 0xffffffff\n");
    scanf("%x", &input);
    Regdata.value = input;
    Regdata.mask = 0xffffffff;
    printf("value = %x\n", Regdata.value);
    ::ioctl(mFd, SET_AUDSYS_REG, &Regdata);
}

static void SetAnaAfeReg(int input, int mFd)
{
    Register_Control Regdata;
    memset((void *)&Regdata, 0, sizeof(Regdata));
    printf("please enter offset with hex \n");
    scanf("%x", &input);
    Regdata.offset = input;
    printf("offset = %x\n", Regdata.offset);
    printf("please enter value with hex mask = 0xffffffff\n");
    scanf("%x", &input);
    Regdata.value = input;
    Regdata.mask = 0xffffffff;
    printf("value = %x\n", Regdata.value);
    ::ioctl(mFd, SET_ANAAFE_REG, &Regdata);
}

static void GetAudSysReg(int input , int mFd)
{
    Register_Control Regdata;
    memset((void *)&Regdata, 0, sizeof(Regdata));
    printf("please enter offset with hex \n");
    scanf("%x", &input);
    Regdata.offset = input;
    printf("offset = %x\n", Regdata.offset);
    Regdata.value = 0;
    Regdata.mask = 0xffffffff;
    ::ioctl(mFd, GET_AUDSYS_REG, &Regdata);
    printf("Regdata.value = %x \n", Regdata.value);
}

static void GetAnaAfeReg(int input , int mFd)
{
    Register_Control Regdata;
    memset((void *)&Regdata, 0, sizeof(Regdata));
    printf("please enter offset with hex \n");
    scanf("%d", &input);
    Regdata.offset = input;
    printf("offset = %x\n", Regdata.offset);
    Regdata.value = 0;
    Regdata.mask = 0xffffffff;
    ::ioctl(mFd, GET_ANAAFE_REG, &Regdata);
    printf("Regdata.value = %x \n", Regdata.value);
}

static void TurnOnSpeaker(bool bEnable, int mFd)
{
    if (bEnable)
    {
        printf("Turn on Speaker \n");
        ::ioctl(mFd, SET_SPEAKER_ON, 1);
    }
    else
    {
        printf("Turn off Speaker \n");
        ::ioctl(mFd, SET_SPEAKER_OFF, 0);
    }
}

int main()
{
    char str [80];
    int input;
    int exit = 0;
    unsigned int output;
    int mFd = 0;
    mFd = ::open(kAudioDeviceName, O_RDWR);
    printf("open audio drvier mdf = %d\n", mFd);
    if (mFd <= 0)
    {
        printf("open audio driver error , return");
    }

    while (!exit)
    {
        printf("please enter audio ioctl command in hex or ffff to exit \n");
        printf("00:SET_AUDSYS_REG 01:GET_AUDSYS_REG 02:SET_ANAAFE_REG 03:GET_ANAAFE_REG other refernce ioctl code \n");
        scanf("%x", &input);
        switch (input)
        {
            case 0x00:
            {
                printf("SET_AUDSYS_REG \n");
                SetAudSysReg(input, mFd);
                break;
            }
            case 0x01:
            {
                printf("GET_AUDSYS_REG \n");
                GetAudSysReg(input, mFd);
                break;
            }
            case 0x02:
            {
                printf("SET_ANAAFE_REG \n");
                SetAnaAfeReg(input, mFd);
                break;
            }
            case 0x03:
            {
                printf("GET_ANAAFE_REG \n");
                GetAnaAfeReg(input, mFd);
                break;
            }
            case 0x13:
            {
                printf("Set SPEAKER_ON \n");
                TurnOnSpeaker(true, mFd);
                break;
            }
            case 0x14:
            {
                printf("Set SPEAKER_OFF \n");
                TurnOnSpeaker(false, mFd);
                break;
            }
            case 0x20:
            {
                printf("OPEN_DL1_STREAM \n");
                ::ioctl(mFd, OPEN_DL1_STREAM, 0);
                break;
            }
            case 0x21:
            {
                printf("CLOSE_DL1_STREAM \n");
                ::ioctl(mFd, CLOSE_DL1_STREAM, 0);
                break;
            }
            case 0x22:
            {
                printf("INIT_DL1_STREAM \n");
                printf("please enter INIT_DL1_STREAM buffer size with hex \n");
                scanf("%x", &input);
                ::ioctl(mFd, INIT_DL1_STREAM, input);
                break;
            }
            case 0x23:
            {
                printf("START_DL1_STREAM \n");
                ::ioctl(mFd, START_DL1_STREAM, 0);
                break;
            }
            case 0x24:
            {
                printf("STANDBY_DL1_STREAM \n");
                ::ioctl(mFd, STANDBY_DL1_STREAM, 0);
                break;
            }
            case 0x25:
            {
                printf("SET_DL1_AFE_BUFFER \n");
                printf("please enter SET_DL1_AFE_BUFFER size with hex \n");
                scanf("%x", &input);
                ::ioctl(mFd, SET_DL1_AFE_BUFFER, input);
                break;
            }
            case 0x26:
            {
                printf("SET_DL1_SLAVE_MODE \n");
                printf("please enter SET_DL1_SLAVE_MODE with hex \n");
                scanf("%x", &input);
                ::ioctl(mFd, SET_DL1_SLAVE_MODE, input);
                break;
            }
            case 0x27:
            {
                printf("GET_DL1_SLAVE_MODE \n");
                ::ioctl(mFd, GET_DL1_SLAVE_MODE, &output);
                printf("GET_DL1_SLAVE_MODE output = %x\n", output);
                break;
            }
            case 0x30:
            {
                printf("OPEN_I2S_INPUT_STREAM \n");
                printf("please enter OPEN_I2S_INPUT_STREAM buffer size with hex \n");
                scanf("%x", &input);
                ::ioctl(mFd, OPEN_I2S_INPUT_STREAM, input);
                break;
            }
            case 0x31:
            {
                printf("CLOSE_I2S_INPUT_STREAM \n");
                ::ioctl(mFd, CLOSE_I2S_INPUT_STREAM, 0);
                break;
            }
            case 0x33:
            {
                printf("START_I2S_INPUT_STREAM \n");
                ::ioctl(mFd, START_I2S_INPUT_STREAM, 0);
                break;
            }
            case 0x34:
            {
                printf("STANDBY_I2S_INPUT_STREAM \n");
                ::ioctl(mFd, STANDBY_I2S_INPUT_STREAM, 0);
                break;
            }
            case 0x35:
            {
                printf("START_I2S_INPUT_STREAM \n");
                ::ioctl(mFd, START_I2S_INPUT_STREAM, 0);
                break;
            }
            case 0x36:
            {
                printf("SET_I2S_Output_BUFFER \n");
                ::ioctl(mFd, SET_I2S_Output_BUFFER, 0);
                break;
            }
            case 0x51:
            {
                printf("AUD_SET_CLOCK \n");
                printf("please enter AUD_SET_CLOCK enable 0 : disable 1 : enable \n");
                scanf("%x", &input);
                ::ioctl(mFd, AUD_SET_CLOCK, input);
                break;
            }
            case 0x52:
            {
                printf("AUD_SET_26MCLOCK \n");
                printf("please enter AUD_SET_26MCLOCK enable 0 : disable 1 : enable \n");
                scanf("%x", &input);
                ::ioctl(mFd, AUD_SET_26MCLOCK, input);
                break;
            }
            case 0x53:
            {
                printf("AUD_SET_ADC_CLOCK \n");
                printf("please enter AUD_SET_ADC_CLOCK enable 0 : disable 1 : enable \n");
                scanf("%x", &input);
                ::ioctl(mFd, AUD_SET_ADC_CLOCK, input);
                break;
            }
            case 0x54:
            {
                printf("AUD_SET_I2S_CLOCK \n");
                printf("please enter AUD_SET_I2S_CLOCK enable 0 : disable 1 : enable \n");
                scanf("%x", &input);
                ::ioctl(mFd, AUD_SET_I2S_CLOCK, input);
                break;
            }
            case 0x60:
            {
                printf("AUDDRV_RESET_BT_FM_GPIO \n");
                ::ioctl(mFd, AUDDRV_RESET_BT_FM_GPIO, 0);
                break;
            }
            case 0x61:
            {
                printf("AUDDRV_SET_BT_PCM_GPIO \n");
                ::ioctl(mFd, AUDDRV_SET_BT_PCM_GPIO, 0);
                break;
            }
            case 0x62:
            {
                printf("AUDDRV_SET_FM_I2S_GPIO \n");
                ::ioctl(mFd, AUDDRV_SET_FM_I2S_GPIO, 0);
                break;
            }
            case 0x63:
            {
                printf("AUDDRV_MT6573_CHIP_VER \n");
                ::ioctl(mFd, AUDDRV_MT6573_CHIP_VER, 0);
                break;
            }
            case 0x81:
            {
                printf("AUDDRV_START_DAI_OUTPUT \n");
                ::ioctl(mFd, AUDDRV_START_DAI_OUTPUT, 0);
                break;
            }
            case 0x82:
            {
                printf("AUDDRV_STOP_DAI_OUTPUT \n");
                ::ioctl(mFd, AUDDRV_STOP_DAI_OUTPUT, 0);
                break;
            }
            case 0xFD:
            {
                printf("AUDDRV_LOG_PRINT \n");
                ::ioctl(mFd, AUDDRV_LOG_PRINT, 0);
                break;
            }
            case 0xFE:
            {
                printf("AUDDRV_ASSERT_IOCTL \n");
                ::ioctl(mFd, AUDDRV_ASSERT_IOCTL, 0);
                break;
            }
            case 0xFF:
            {
                printf("AUDDRV_BEE_IOCTL \n");
                ::ioctl(mFd, AUDDRV_BEE_IOCTL, 0);
                break;
            }
            case 0xffff:
            {
                printf("0xffff exit!!! \n");
                exit = true;
                break;
            }
            default:
            {
                printf("not support such command please enter again\n");
            }
        }
    }
    return 0;
}


