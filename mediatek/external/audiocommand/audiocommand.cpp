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

#define LOG_TAG "audiocommand"

typedef unsigned int UINT32;
typedef unsigned short  UINT16;

#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <signal.h>
#include <binder/IPCThreadState.h>
#include <binder/MemoryBase.h>
#include <media/AudioSystem.h>
#include <media/mediaplayer.h>
#include <media/AudioRecord.h>
#include <media/stagefright/AudioSource.h>
#include <media/AudioSystem.h>
#include <system/audio_policy.h>
#include <hardware/audio_policy.h>
#include <hardware_legacy/AudioPolicyInterface.h>
#include <hardware_legacy/AudioSystemLegacy.h>
#include <utils/List.h>
#include <system/audio.h>
#include <utils/Log.h>
#include <media/AudioTrack.h>

#if 0
#include <AudioAfeReg.h>
#include <AudioAnalogReg.h>
#endif

#define kMaxBufferSize (0x4000)
#define tone_repeat_time (50)

using namespace android;

sp<MediaPlayer>     mSoundPlayer;
unsigned int device;
audio_policy_dev_state_t state;
int AudioMode;

#if 0
static AudioAfeReg *mAudioAfeReg = NULL;
static AudioAnalogReg *mAudioAnalogReg = NULL;
//static char const *const kAudioDeviceName = "/dev/eac";
#endif

static int GetAudioMode()
{
    printf("GetAudioMode enter audio mode\n"
           "  0:normal\n"
           "  1:ringtone\n"
           "  2:incall\n"
           "  3:commuinication mode\n");
    scanf("%x", &AudioMode);
    return AudioMode;
}

static audio_devices_t GetAudioDevice()
{
    printf("GetAudioDevice enter audio device \n"
           "  1: earpiece\n"
           "  2: speaker\n"
           "  4: wired_headset\n"
           "  8: wired_headphone\n");
    scanf("%x", &device);
    return (audio_devices_t)device;
}

unsigned char stone1k_48kHz[192] =
{
    0x00, 0x00, 0xFF, 0xFF, 0xB5, 0x10, 0xB5, 0x10, 0x21, 0x21, 0x21, 0x21,
    0xFC, 0x30, 0xFC, 0x30, 0x00, 0x40, 0x00, 0x40, 0xEB, 0x4D, 0xEB, 0x4D,
    0x81, 0x5A, 0x82, 0x5A, 0x8C, 0x65, 0x8C, 0x65, 0xD9, 0x6E, 0xD9, 0x6E,
    0x40, 0x76, 0x41, 0x76, 0xA3, 0x7B, 0xA2, 0x7B, 0xE7, 0x7E, 0xE6, 0x7E,
    0xFF, 0x7F, 0xFF, 0x7F, 0xE6, 0x7E, 0xE7, 0x7E, 0xA2, 0x7B, 0xA2, 0x7B,
    0x41, 0x76, 0x41, 0x76, 0xD9, 0x6E, 0xD8, 0x6E, 0x8D, 0x65, 0x8C, 0x65,
    0x82, 0x5A, 0x81, 0x5A, 0xEB, 0x4D, 0xEB, 0x4D, 0xFF, 0x3F, 0xFF, 0x3F,
    0xFB, 0x30, 0xFB, 0x30, 0x20, 0x21, 0x21, 0x21, 0xB5, 0x10, 0xB5, 0x10,
    0x00, 0x00, 0xFF, 0xFF, 0x4B, 0xEF, 0x4B, 0xEF, 0xE0, 0xDE, 0xE0, 0xDE,
    0x05, 0xCF, 0x05, 0xCF, 0x00, 0xC0, 0x00, 0xC0, 0x14, 0xB2, 0x14, 0xB2,
    0x7D, 0xA5, 0x7F, 0xA5, 0x74, 0x9A, 0x75, 0x9A, 0x27, 0x91, 0x26, 0x91,
    0xC0, 0x89, 0xBF, 0x89, 0x5E, 0x84, 0x5D, 0x84, 0x19, 0x81, 0x19, 0x81,
    0x02, 0x80, 0x02, 0x80, 0x19, 0x81, 0x19, 0x81, 0x5E, 0x84, 0x5E, 0x84,
    0xBF, 0x89, 0xBF, 0x89, 0x27, 0x91, 0x28, 0x91, 0x75, 0x9A, 0x74, 0x9A,
    0x7E, 0xA5, 0x7E, 0xA5, 0x15, 0xB2, 0x15, 0xB2, 0x00, 0xC0, 0x00, 0xC0,
    0x04, 0xCF, 0x04, 0xCF, 0xDF, 0xDE, 0xDF, 0xDE, 0x4B, 0xEF, 0x4B, 0xEF
};

static void AudioRecordCallbackFunction(int event, void *user, void *info)
{
    AudioSource *source = (AudioSource *) user;
    switch (event)
    {
        case AudioRecord::EVENT_MORE_DATA:
        {
            ALOGD("audiorecordtest::EVENT_MORE_DATA");
            break;
        }
        case AudioRecord::EVENT_OVERRUN:
        {
            ALOGD("audiorecordtest::EVENT_OVERRUN");
            break;
        }
        default:
            // does nothing
            break;
    }
}

void StartPlayBack()
{
    ALOGD("+AudioPlaybackTest \n");
    int count = 20 * 10;
    AudioTrack  *mAudioTrack;
    mAudioTrack = new AudioTrack(
        AUDIO_STREAM_MUSIC, 44100, AUDIO_FORMAT_PCM_16_BIT, (audio_channel_mask_t)AUDIO_CHANNEL_OUT_STEREO,
        1024 * 30, AUDIO_OUTPUT_FLAG_NONE);
    mAudioTrack->start();
    int buffersize = 192 * tone_repeat_time;
    char *temp = (char *)malloc(buffersize);
    char *tempptr = temp;
    ALOGD("buffersize = %d  ", buffersize);
    for (int i = 0; i < tone_repeat_time; i++)
    {
        memcpy((void *)tempptr, (void *)stone1k_48kHz, 192);
        tempptr += 192;
    }
    while (count--)
    {
        // write buffer
        mAudioTrack->write((void *)temp, (size_t)buffersize);
    }
}


void StartPlayBackRing()
{
    ALOGD("+AudioPlaybackTest \n");
    int count = 20 * 10;
    AudioTrack  *mAudioTrack;
    mAudioTrack = new AudioTrack(
        AUDIO_STREAM_RING, 44100, AUDIO_FORMAT_PCM_16_BIT, (audio_channel_mask_t)AUDIO_CHANNEL_OUT_STEREO,
        1024 * 30, AUDIO_OUTPUT_FLAG_NONE);
    mAudioTrack->start();
    int buffersize = 192 * tone_repeat_time;
    char *temp = (char *)malloc(buffersize);
    char *tempptr = temp;
    ALOGD("buffersize = %d  ", buffersize);
    for (int i = 0; i < tone_repeat_time; i++)
    {
        memcpy((void *)tempptr, (void *)stone1k_48kHz, 192);
        tempptr += 192;
    }
    while (count--)
    {
        // write buffer
        mAudioTrack->write((void *)temp, (size_t)buffersize);
    }
}

void StartRecording()
{
    int count = 10;
    AudioRecord *mRecord;
    status_t mInitCheck;
    bool mStarted = false;
    uint32_t sampleRate = 16000;
    int inputSource = AUDIO_SOURCE_MIC;
    int channels = AUDIO_CHANNEL_IN_MONO; //AUDIO_CHANNEL_IN_MONO or AUDIO_CHANNEL_IN_STEREO
/*
    uint32_t flags = AudioRecord::RECORD_AGC_ENABLE |
                     AudioRecord::RECORD_NS_ENABLE  |
                     AudioRecord::RECORD_IIR_ENABLE;
*/
    mRecord = new AudioRecord(
        (audio_source_t)inputSource, sampleRate, AUDIO_FORMAT_PCM_16_BIT, (audio_channel_mask_t)channels,
        4 * kMaxBufferSize / sizeof(int16_t), /* Enable ping-pong buffers */
        AudioRecordCallbackFunction,
        NULL);

    mInitCheck = mRecord->initCheck();
    status_t err = mRecord->start();

    usleep(10 * 1000 * 1000);
    ALOGD("audiorecordtest::sleep for 10 sec");

    mRecord->stop();
}

void AudioHardwareCommand()
{
    int input = 0;
    while (true)
    {
        printf("please enter audio command 0:input parameters \n");
        scanf("%x", &input);
        AudioSystem::SetAudioCommand(0x90, input);
    }
}

void AudioSystemCommand()
{
    int SecondCommand;
    while (true)
    {
        printf("please enter audio command\n"
               "  0:device connnect\n"
               "  1:device disconnect\n"
               "  2:mode set \n");
        scanf("%x", &SecondCommand);
        switch (SecondCommand)
        {
            case 0:
                state = AUDIO_POLICY_DEVICE_STATE_AVAILABLE;
                device = GetAudioDevice();
                AudioSystem::setDeviceConnectionState((audio_devices_t)device, state, NULL);
                break;
            case 1:
                state = AUDIO_POLICY_DEVICE_STATE_UNAVAILABLE;
                device = GetAudioDevice();
                AudioSystem::setDeviceConnectionState((audio_devices_t)device, state, NULL);
                break;
            case 2:
                AudioMode = GetAudioMode();
                AudioSystem::setPhoneState((audio_mode_t)AudioMode);
                break;
            default:
                printf("no this command!!!!!!!!!!!!\n");
                break;
        }
    }
}
#if 0
void AudioSetAfeRegister()
{
    int SecondCommand = 0;
    unsigned int address , Regvalue;

    while (true)
    {
        printf("please enter AudioSetAfeRegister 0:Getregister 1:Set register\n");
        scanf("%x", &SecondCommand);
        switch (SecondCommand)
        {
            case 0:
                printf("please enter afe address \n");
                scanf("%x", &address);
                Regvalue = mAudioAfeReg->GetAfeReg(address);
                printf("GetAfeReg Regvalue=0x%x \n", Regvalue);
                break;
            case 1:
                printf("please enter afe address \n");
                scanf("%x", &address);
                printf("please enter afe value \n");
                scanf("%x", &Regvalue);
                mAudioAfeReg->SetAfeReg(address, Regvalue, 0xffffffff);
                break;
            default:
                break;
        }
    }
}

void AudioSetAnaRegister()
{
    int SecondCommand = 0;
    unsigned int address , Regvalue;

    while (true)
    {
        printf("please enter AudioSetAfeRegister 0:GetAnalogReg 1:Setanalogregister\n");
        scanf("%x", &SecondCommand);
        switch (SecondCommand)
        {
            case 0:
                printf("please enter afe address \n");
                scanf("%x", &address);
                Regvalue = mAudioAnalogReg->GetAnalogReg(address);
                printf("GetAfeReg Regvalue=0x%x \n", Regvalue);
                break;
            case 1:
                printf("please enter afe address \n");
                scanf("%x", &address);
                printf("please enter afe value \n");
                scanf("%x", &Regvalue);
                mAudioAnalogReg->SetAnalogReg(address, Regvalue, 0xffffffff);
                break;
            default:
                break;
        }
    }

}
#endif
int main()
{
    char cmd[100];
    ProcessState::self()->startThreadPool();
    sp<ProcessState> proc(ProcessState::self());
    ALOGD("start audiocommand \n");

#if 0
    mAudioAfeReg = AudioAfeReg::getInstance();
    mAudioAnalogReg = AudioAnalogReg::getInstance();
#endif

    int MainCommand;
    while (true)
    {
        printf("please enter audio command\n"
               "  0: audioplayback(AUDIO_STREAM_MUSIC)\n"
               "  1: audioplayback(AUDIO_STREAM_RING)\n"
               "  2: audiorecord\n"
               "  3: audiosystem command\n"
               "  4: audiohardwarecoomand\n"
#if 0
               "  5: set afe register\n"
               "  6: set pmic register\n"
#endif
               "  q: exit\n");
        fgets(cmd, sizeof(cmd), stdin);
        if (!strcmp(cmd, "q"))
        {
            break;
        }
        if (sscanf(cmd, "%x", &MainCommand) != 1)
        {
            continue;
        }
        switch (MainCommand)
        {
            case 0:
                StartPlayBack();
                break;
            case 1:
                StartPlayBackRing();
                break;
            case 2:
                StartRecording();
                break;
            case 3:
                AudioSystemCommand();
                break;
            case 4:
                AudioHardwareCommand();
                break;
            case 5:
                //AudioSetAfeRegister();//Set/Get AFE Reg
                break;
            case 6:
                //AudioSetAnaRegister();//Set/Get Ana Reg
                break;
            default:
                break;
        }

        if (MainCommand == 7)
        {
            break;
        }
    }

    return 0;
}


