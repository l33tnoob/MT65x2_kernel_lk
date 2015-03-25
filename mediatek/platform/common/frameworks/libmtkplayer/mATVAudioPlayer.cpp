/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

//#define LOG_NDEBUG 0
#define LOG_TAG "mATVPlayer"
#include "utils/Log.h"
#include "cutils/xlog.h"

#include <stdio.h>
#include <assert.h>
#include <limits.h>
#include <unistd.h>
#include <fcntl.h>
#include <sched.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <system/audio.h>

#include <binder/IServiceManager.h>

#include <hardware/audio.h>
#include <AudioSystem.h>

#ifndef CHANGE_AUDIO_PRIORITY
#include <sys/time.h>
#include <sys/resource.h>
#endif

#ifndef FAKE_MATV
// Query output device
//#include "include/AudioIoctl.h"
//#include "AudioResourceManager.h"
//#include "AudioMTKHardware.h"
//#include "AudioType.h"
#endif

#include "mATVAudioPlayer.h"
#include <linux/rtpm_prio.h>

#define  MUTE_PAUSE

#define MATV_AUDIO_SAMPLING_RATE    44100
#define MATV_AUDIO_CHANNEL_NUM      2
#define MATV_SINETABALE_SIZE_FOR_FAKE 320
//#define DUMP_MATV_INPUT_DATA

//#define MATV_AUDIO_LINEIN_PATH
//#define ATV_AUDIO_FILELOG


static pid_t myTid()
{
    return gettid();
}
static long long getTimeMs()
{
    struct timeval t1;
    long long ms;

    gettimeofday(&t1, NULL);
    ms = t1.tv_sec * 1000LL + t1.tv_usec / 1000;

    return ms;
}

#ifdef DUMP_MATV_INPUT_DATA
static FILE *fp = NULL;
#endif



// ----------------------------------------------------------------------------
extern int matv_use_analog_input;//from libaudiosetting.so
extern int using_class_ab_amp;	//from libaudiosetting.so, related to USING_CLASS_AB_AMP

namespace android
{

// ----------------------------------------------------------------------------

//#define FAKE_MATV
//sine table for simulation only
#ifdef FAKE_MATV
static const uint16_t sineTable[320] =
{
    0x0000, 0x0A03, 0x13C7, 0x1D0E, 0x259E, 0x2D41, 0x33C7, 0x3906,
    0x3CDE, 0x3F36, 0x4000, 0x3F36, 0x3CDE, 0x3906, 0x33C7, 0x2D41,
    0x259E, 0x1D0E, 0x13C7, 0x0A03, 0x0000, 0xF5FD, 0xEC39, 0xE2F2,
    0xDA62, 0xD2BF, 0xCC39, 0xC6FA, 0xC322, 0xC0CA, 0xC000, 0xC0CA,
    0xC322, 0xC6FA, 0xCC39, 0xD2BF, 0xDA62, 0xE2F2, 0xEC39, 0xF5FD,
    0xFFFF, 0x0A03, 0x13C7, 0x1D0E, 0x259E, 0x2D41, 0x33C7, 0x3906,
    0x3CDE, 0x3F36, 0x4000, 0x3F36, 0x3CDE, 0x3906, 0x33C7, 0x2D41,
    0x259E, 0x1D0E, 0x13C7, 0x0A03, 0x0000, 0xF5FD, 0xEC39, 0xE2F2,
    0xDA62, 0xD2BF, 0xCC39, 0xC6FA, 0xC322, 0xC0CA, 0xC000, 0xC0CA,
    0xC322, 0xC6FA, 0xCC39, 0xD2BF, 0xDA62, 0xE2F2, 0xEC39, 0xF5FD,
    0xFFFF, 0x0A03, 0x13C7, 0x1D0E, 0x259E, 0x2D41, 0x33C7, 0x3906,
    0x3CDE, 0x3F36, 0x4000, 0x3F36, 0x3CDE, 0x3906, 0x33C7, 0x2D41,
    0x259E, 0x1D0E, 0x13C7, 0x0A03, 0x0000, 0xF5FD, 0xEC39, 0xE2F2,
    0xDA62, 0xD2BF, 0xCC39, 0xC6FA, 0xC322, 0xC0CA, 0xC000, 0xC0CA,
    0xC322, 0xC6FA, 0xCC39, 0xD2BF, 0xDA62, 0xE2F2, 0xEC39, 0xF5FD,
    0xFFFF, 0x0A03, 0x13C7, 0x1D0E, 0x259E, 0x2D41, 0x33C7, 0x3906,
    0x3CDE, 0x3F36, 0x4000, 0x3F36, 0x3CDE, 0x3906, 0x33C7, 0x2D41,
    0x259E, 0x1D0E, 0x13C7, 0x0A03, 0x0000, 0xF5FD, 0xEC39, 0xE2F2,
    0xDA62, 0xD2BF, 0xCC39, 0xC6FA, 0xC322, 0xC0CA, 0xC000, 0xC0CA,
    0xC322, 0xC6FA, 0xCC39, 0xD2BF, 0xDA62, 0xE2F2, 0xEC39, 0xF5FD,
    0xFFFF, 0x0A03, 0x13C7, 0x1D0E, 0x259E, 0x2D41, 0x33C7, 0x3906,
    0x3CDE, 0x3F36, 0x4000, 0x3F36, 0x3CDE, 0x3906, 0x33C7, 0x2D41,
    0x259E, 0x1D0E, 0x13C7, 0x0A03, 0x0000, 0xF5FD, 0xEC39, 0xE2F2,
    0xDA62, 0xD2BF, 0xCC39, 0xC6FA, 0xC322, 0xC0CA, 0xC000, 0xC0CA,
    0xC322, 0xC6FA, 0xCC39, 0xD2BF, 0xDA62, 0xE2F2, 0xEC39, 0xF5FD,
    0xFFFF, 0x0A03, 0x13C7, 0x1D0E, 0x259E, 0x2D41, 0x33C7, 0x3906,
    0x3CDE, 0x3F36, 0x4000, 0x3F36, 0x3CDE, 0x3906, 0x33C7, 0x2D41,
    0x259E, 0x1D0E, 0x13C7, 0x0A03, 0x0000, 0xF5FD, 0xEC39, 0xE2F2,
    0xDA62, 0xD2BF, 0xCC39, 0xC6FA, 0xC322, 0xC0CA, 0xC000, 0xC0CA,
    0xC322, 0xC6FA, 0xCC39, 0xD2BF, 0xDA62, 0xE2F2, 0xEC39, 0xF5FD,
    0xFFFF, 0x0A03, 0x13C7, 0x1D0E, 0x259E, 0x2D41, 0x33C7, 0x3906,
    0x3CDE, 0x3F36, 0x4000, 0x3F36, 0x3CDE, 0x3906, 0x33C7, 0x2D41,
    0x259E, 0x1D0E, 0x13C7, 0x0A03, 0xFFFF, 0xF5FD, 0xEC39, 0xE2F2,
    0xDA62, 0xD2BF, 0xCC39, 0xC6FA, 0xC322, 0xC0CA, 0xC000, 0xC0CA,
    0xC322, 0xC6FA, 0xCC39, 0xD2BF, 0xDA62, 0xE2F2, 0xEC39, 0xF5FD,
    0xFFFF, 0x0A03, 0x13C7, 0x1D0E, 0x259E, 0x2D41, 0x33C7, 0x3906,
    0x3CDE, 0x3F36, 0x4000, 0x3F36, 0x3CDE, 0x3906, 0x33C7, 0x2D41,
    0x259E, 0x1D0E, 0x13C7, 0x0A03, 0x0000, 0xF5FD, 0xEC39, 0xE2F2,
    0xDA62, 0xD2BF, 0xCC39, 0xC6FA, 0xC322, 0xC0CA, 0xC000, 0xC0CA,
    0xC322, 0xC6FA, 0xCC39, 0xD2BF, 0xDA62, 0xE2F2, 0xEC39, 0xF5FD,
};

static int ReadFakeBuffer(void *buffer)
{
    usleep(1000);
    int sineTableSize = 320 * sizeof(uint16_t);
    char *ptr = (char *)buffer;
    memcpy(ptr, sineTable, sineTableSize);
    ptr += sineTableSize;
    memcpy(ptr, sineTable, sineTableSize);
    ptr += sineTableSize;
    return sineTableSize * 2;
}
#endif

static int GetReadBufferSize()
{
    return MATV_SINETABALE_SIZE_FOR_FAKE * sizeof(uint16_t) * 2;        // 2 x sine table size
}


// Here start the code for I2S test for real MT5192
// Pure test use
//#define MATV_AUDIO_TEST_I2S

#ifdef MATV_AUDIO_TEST_I2S

int tvscan_finish = 0;

extern "C" {
    static void atv_autoscan_progress_cb(void *cb_param, kal_uint8 precent, kal_uint8 ch, kal_uint8 chnum)
    {
        matv_chscan_state scan_state;
        SXLOGD("audio scan call back");
        matv_chscan_query(&scan_state);
        SXLOGD("CB.autoscan_progress: %d%% ,update CH-%02d(%c)\n",
               precent, scan_state.ch_latest_updated,
               scan_state.updated_entry.flag ? 'O' : 'X'
              );
        //tvscan_progress = precent;
    }
    static void atv_fullscan_progress_cb(void *cb_param, kal_uint8 precent, kal_uint32 freq, kal_uint32 freq_start, kal_uint32 freq_end)
    {
        SXLOGD("CB.fullscan_progress: %d%%\n", precent);
        //tvscan_progress = precent;
    }


    static void atv_scanfinish_cb(void *cb_param, kal_uint8 chnum)
    {
        SXLOGD("CB.scanfinish: chnum:%d\n", chnum);
        tvscan_finish = 1;
    }

    static void atv_audioformat_cb(void *cb_param, kal_uint32 format)
    {
        SXLOGD("CB.audioformat: %08x\n", format);
    }

}

static int matv_ts_init()
{
    int ret;
    tvscan_finish = 0;
    ret = matv_init();
    matv_register_callback(0,
                           atv_autoscan_progress_cb,
                           atv_fullscan_progress_cb,
                           atv_scanfinish_cb,
                           atv_audioformat_cb);

    return (ret);
}

static void startMATVChip()
{
    int ret;
    matv_ch_entry ch_ent;

    SXLOGD(" startMATVChip begin");

    // init
    ret = matv_ts_init();
    SXLOGD(" mATV init result:%d", ret);

    // force to some specific channel

    matv_set_country(TV_TAIWAN);
    ch_ent.freq = 687250;
    ch_ent.sndsys = 1;
    ch_ent.colsys = 3;
    ch_ent.flag = 1;
    matv_set_chtable(50, &ch_ent);
    matv_get_chtable(50, &ch_ent);
    matv_change_channel(50);
    SXLOGD("channel:%d, freq:%d, sndsys:%d, colsys:%d, flag:%d \n", 50, ch_ent.freq, ch_ent.sndsys, ch_ent.colsys, ch_ent.flag);
    return;


    // scan channel
    SXLOGD(" Start Channel Scan ...");
    matv_set_country(TV_TAIWAN);
    matv_chscan(MATV_AUTOSCAN);

    while (!tvscan_finish)
    {
        sleep(1);
    }

    SXLOGD(" Finiah Channel Scan ...");

    //getting channel number
    int i = 1;
    int ch_candidate = 0;
    int validChannel[128];
    int validIndex = 0;

    while (matv_get_chtable(i++, &ch_ent))
    {
        if (ch_ent.flag & CH_VALID)
        {
            SXLOGD("channel:%d, freq:%d, sndsys:%d, colsys:%d, flag:%d \n", i - 1, ch_ent.freq, ch_ent.sndsys, ch_ent.colsys, ch_ent.flag);
            ch_candidate = i - 1;
            validChannel[validIndex++] = ch_candidate;
        }
    }

    // set to last channel number
    SXLOGD("Last channel number is:%d", ch_candidate);
    matv_change_channel(50);

}

#endif

// TODO: Determine appropriate return codes
static status_t ERROR_NOT_OPEN = -1;
static status_t ERROR_OPEN_FAILED = -2;
static status_t ERROR_ALLOCATE_FAILED = -4;
static status_t ERROR_NOT_SUPPORTED = -8;
static status_t ERROR_NOT_READY = -16;
static status_t ERROR_START_FAILED = -32;
static status_t ERROR_STOP_FAILED = -64;
static status_t STATE_INIT = 0;
static status_t STATE_ERROR = 1;
static status_t STATE_OPEN = 2;
static status_t STATE_PLAY = 3;
static status_t STATE_STOP = 4;

mATVAudioPlayer::mATVAudioPlayer() :
    mAudioBuffer(NULL), mPlayTime(-1), mDuration(-1), mState(STATE_ERROR),
    mStreamType(AUDIO_STREAM_MATV),mAudioRecord(NULL),
    mExit(false), mPaused(false), mRender(false), mRenderTid(-1), mMutePause(false)
{
    SXLOGD("mATVAudioPlayer constructor");

    if (matv_use_analog_input == 1)
    {
        SXLOGD("line in path => constructor");
    }
	else if(matv_use_analog_input == 0)
	{
		SXLOGD("digtal path => constructor");
	}
/*
#ifndef FAKE_MATV
    mFd = ::open(kAudioDeviceName, O_RDWR);
    if (mFd < 0)
    {
        SXLOGE("cannot open audio driver\n");
    }
#endif
*/
}

void mATVAudioPlayer::onFirstRef()
{
    SXLOGD("onFirstRef");
	Mutex::Autolock l(mMutex);

    if (matv_use_analog_input == 1)
    {
        SXLOGD("line in path => onFirstRef");
    }
	else if(matv_use_analog_input == 0)
	{
        SXLOGD("digtal path => onFirstRef");
	}

#ifdef DUMP_MATV_INPUT_DATA
	fp = fopen("/sdcard/mtklog/matv_input.pcm", "wb");
	
	if (NULL == fp)
	{
		SXLOGE("Open file matv pcm to write failed!!!");
	}
#endif
	   
	
    // create playback thread
    createThreadEtc(renderThread, this, "mATV audio player", ANDROID_PRIORITY_AUDIO);
    mCondition.waitRelative(mMutex, seconds(3));
    if (mRenderTid > 0)
    {
        SXLOGD("render thread(%d) started", mRenderTid);
        mState = STATE_INIT;
    }
}

status_t mATVAudioPlayer::initCheck()
{
    if (mState != STATE_ERROR)
    {
        return NO_ERROR;
    }

    return ERROR_NOT_READY;
}

mATVAudioPlayer::~mATVAudioPlayer()
{
    SXLOGD("mATVAudioPlayer destructor");
    release();

    if (matv_use_analog_input == 1)
    {
        SXLOGD("line in path => destructor");
    }
	else
	{
		SXLOGD("digtal path => destructor");
	}
/*
#ifndef FAKE_MATV
    if (mFd >= 0)
    {
        ::close(mFd);
    }
#endif
*/
	deleteAudioRecord();

    SXLOGD("mATVAudioPlayer destructor end");
}

status_t mATVAudioPlayer::setDataSource(
    const char *path, const KeyedVector<String8, String8> *)
{
    SXLOGD("mATVAudioPlayer setDataSource path=%s \n", path);
    return setdatasource(path, -1, 0, 0x7ffffffffffffffLL); // intentionally less than LONG_MAX
}

status_t mATVAudioPlayer::setDataSource(int fd, int64_t offset, int64_t length)
{
    SXLOGD("mATVAudioPlayer setDataSource offset=%d, length=%d \n", ((int)offset), ((int)length));
    return setdatasource(NULL, fd, offset, length);
}


status_t mATVAudioPlayer::setdatasource(const char *path, int fd, int64_t offset, int64_t length)
{
    SXLOGD("setdatasource");

    Mutex::Autolock l(mMutex);

    if (mState == STATE_OPEN)
    {
    	SXLOGD("matv player is still open, so reset it");
        reset_nosync();
    }

    //Check mATV Service for bounding
    {
        int a = 7393;
        int b = 739;
        int c = 73939;
        int in, out;
        //sp<IATVCtrlService> spATVCtrlService;
        sp<IServiceManager> sm = defaultServiceManager();
        sp<IBinder> binder;

        do
        {
            binder = sm->getService(String16("media.ATVCtrlService"));

            if (binder != 0)
            {
                break;
            }

            SXLOGW("ATVCtrlService not published, waiting...");
            usleep(500000); // 0.5 s
        }
        while (true);

        spATVCtrlService = interface_cast<IATVCtrlService>(binder);
        in = (int)&a;
        out = spATVCtrlService->ATVCS_matv_set_parameterb(in);

        if (out != (in + a)*b + c)
        {
            SXLOGD("Set Parameberb failed");
            return ERROR_OPEN_FAILED;
        }
    }

    if (matv_use_analog_input == 1)
    {
        SXLOGD("line in path => setdatasource");
    }
	else if(matv_use_analog_input == 0)
	{
		SXLOGD("digtal path => setdatasource");
	}

    mState = STATE_OPEN;
    return NO_ERROR;
}

status_t mATVAudioPlayer::prepare()
{
    SXLOGD("prepare");

    if (mState != STATE_OPEN )
    {
        SXLOGE("prepare ERROR_NOT_OPEN");
        return ERROR_NOT_OPEN;
    }

    return NO_ERROR;
}

status_t mATVAudioPlayer::prepareAsync()
{
    SXLOGD("prepareAsync");

    // can't hold the lock here because of the callback
    // it's safe because we don't change state
    if(mState != STATE_OPEN && mState != STATE_STOP)
    {
        sendEvent(MEDIA_ERROR);
        SXLOGD("prepareAsync sendEvent(MEDIA_ERROR)");
        return NO_ERROR;
    }

    sendEvent(MEDIA_PREPARED);
    return NO_ERROR;
}

status_t mATVAudioPlayer::start()
{
    SXLOGD("start");
    Mutex::Autolock l(mMutex);
	size_t minFrameCount;

    if (mState != STATE_OPEN && mState != STATE_PLAY && mState != STATE_STOP)
    {
        SXLOGE("start ERROR_NOT_OPEN \n");
        return ERROR_NOT_OPEN;
    }

    // Start MATV chip audio
    SXLOGD("Start matv audio play");
    spATVCtrlService->ATVCS_matv_audio_play();

    //usleep(10*1000*1000);

    if (matv_use_analog_input == 1)
    {
        SXLOGD("line in path => start");
        // tell Audio Driver we are going to stop: for Daul Speaker (ClassD + ClassAB)
/*
#ifndef FAKE_MATV
        if (mFd >= 0)
        {
            int Reg_Data[3] = {INFO_U2K_MATV_AUDIO_START, 0, 0};
            ::ioctl(mFd, YUSU_INFO_FROM_USER, &Reg_Data);
        }
#endif
*/
#ifdef MUTE_PAUSE
        SXLOGD("line in path => startTina!!!");
        if (mMutePause == true)
        {
            mMutePause = false;
            mAudioSink->setVolume(1.0, 1.0);
        }
#endif
        spATVCtrlService->ATVCS_matv_set_chipdep(190, 3);
        AudioSystem::setParameters(0, (String8)"AtvAudioLineInEnable=1");
        AudioSystem::setStreamMute(AUDIO_STREAM_MATV, false);
    }
	else if(matv_use_analog_input == 0)
	{
		SXLOGD("digtal path => start");
#ifdef MUTE_PAUSE
    	if (mMutePause == true)
    	{
      		mMutePause = false;
            mAudioSink->setVolume(1.0, 1.0);
    	}
#endif
    	AudioSystem::setParameters(0, (String8)"AudioSetMatvDigitalEnable=1");
	 }

     // tell Audio Driver we are going to start: for Daul Speaker (ClassD + ClassAB)
/*
#ifndef FAKE_MATV
     if (mFd >= 0)
     {
         int Reg_Data[3] = {INFO_U2K_MATV_AUDIO_START, 0, 0};
         ::ioctl(mFd, YUSU_INFO_FROM_USER, &Reg_Data);
     }
#endif
*/


#ifdef MATV_AUDIO_TEST_I2S
     if (!tvscan_finish)                           //start the chip and scan channel if the scan is not done yet
     {
         SXLOGD("Force start matv chip");
         startMATVChip();
     }
#endif

    mPaused = false;
    mRender = true;


	//create AudioRecord, which must after setParameter
	status_t status = AudioRecord::getMinFrameCount(&minFrameCount,
                                           MATV_AUDIO_SAMPLING_RATE,
                                           AUDIO_FORMAT_PCM_16_BIT,
                                           MATV_AUDIO_CHANNEL_NUM);
	if(NO_ERROR == status)
	{
		SXLOGD("AudioRecord minFrameCount is %d", minFrameCount);
		int frameCount = GetReadBufferSize() / 2 / MATV_AUDIO_CHANNEL_NUM;
		int bufCount = 2;
		while((bufCount * frameCount) < minFrameCount)
		{
			bufCount++;
		}
		SXLOGD("frameCount is %d,bufCount is %d", frameCount, bufCount);
			
		bool ret = createAudioRecord(bufCount*frameCount);
		if(ret == false)
			return ERROR_NOT_READY;
	}
	else
	{
		SXLOGD("AudioRecord getMinFrameCount Failed !!!");
		return ERROR_NOT_READY;
	}
	
    // wake up render thread
    SXLOGD("start wakeup render thread\n");
    mCondition.signal();

	mState = STATE_PLAY;
    return NO_ERROR;
}

status_t mATVAudioPlayer::stop()
{
    SXLOGD("stop");
    Mutex::Autolock l(mMutex);

    if (mState != STATE_OPEN && mState != STATE_PLAY && mState != STATE_STOP)
    {
        SXLOGE("stop ERROR_NOT_OPEN");
        return ERROR_NOT_OPEN;
    }

    // Stop MATV chip audio
    spATVCtrlService->ATVCS_matv_audio_stop();

    if (matv_use_analog_input == 1)
    {
        SXLOGD("line in path => stop");
        // tell Audio Driver we are going to stop: for Daul Speaker (ClassD + ClassAB)
/*
#ifndef FAKE_MATV
        if (mFd >= 0)
        {
            int Reg_Data[3] = {INFO_U2K_MATV_AUDIO_STOP, 0, 0};
            ::ioctl(mFd, YUSU_INFO_FROM_USER, &Reg_Data);
        }
#endif
*/
        AudioSystem::setParameters(0, (String8)"AtvAudioLineInEnable=0");
    }
	else
	{
		SXLOGD("digtal path => stop");
		AudioSystem::setParameters(0, (String8)"AudioSetMatvDigitalEnable=0");
	}

    mPaused = true;
    mRender = false;

	mState = STATE_STOP;
    return NO_ERROR;
}

status_t mATVAudioPlayer::seekTo(int position)
{
    SXLOGD("seekTo %d", position);
    Mutex::Autolock l(mMutex);

    return NO_ERROR;
}

status_t mATVAudioPlayer::pause()
{
    SXLOGD("pause");
    Mutex::Autolock l(mMutex);

    if (mState != STATE_OPEN && mState != STATE_PLAY && mState != STATE_STOP)
    {
        SXLOGE("pause ERROR_NOT_OPEN \n");
        return ERROR_NOT_OPEN;
    }

    if (matv_use_analog_input == 1)
    {
        SXLOGD("line in path => pause means stop");
        // tell Audio Driver we are going to stop: for Daul Speaker (ClassD + ClassAB)
/*
#ifndef FAKE_MATV
        if (mFd >= 0)
        {
            int Reg_Data[3] = {INFO_U2K_MATV_AUDIO_STOP, 0, 0};
            ::ioctl(mFd, YUSU_INFO_FROM_USER, &Reg_Data);
        }
#endif
*/
        ///AudioSystem::setParameters(0, (String8)"AtvAudioLineInEnable=0");
        AudioSystem::setStreamMute(AUDIO_STREAM_MATV, true);
#ifdef MUTE_PAUSE
        SXLOGD("line in path => mute Tina!!!!!");

        if(mMutePause == false)
        {
            mMutePause = true;
        }
#else
        mPaused = true;
#endif

    }
	else if(matv_use_analog_input == 0)
	{
		SXLOGD("digtal path => pause means stop");

		//I2S uses mute to pause
#ifdef MUTE_PAUSE
    	if(mMutePause == false)
   		{
      		mMutePause = true;
   		}
#else
    	mPaused = true;
#endif
	 }

//	mPaused = true;

	mState = STATE_STOP;
    return NO_ERROR;
}

bool mATVAudioPlayer::isPlaying()
{
    SXLOGD("isPlaying");
	Mutex::Autolock l(mMutex);
	
    if (mState == STATE_PLAY)
    {
		return true;
    }
	else
		return false;
}

status_t mATVAudioPlayer::getCurrentPosition(int *position)
{
    SXLOGD("getCurrentPosition always return\n");
    Mutex::Autolock l(mMutex);

    if (mState != STATE_OPEN && mState != STATE_PLAY && mState != STATE_STOP)
    {
        SXLOGD("getCurrentPosition(): file not open");
        return ERROR_NOT_OPEN;
    }

    *position = 0;
    return NO_ERROR;
}

status_t mATVAudioPlayer::getDuration(int *duration)
{
    Mutex::Autolock l(mMutex);

    if (mState != STATE_OPEN && mState != STATE_PLAY && mState != STATE_STOP)
    {
        SXLOGD("getDuration ERROR_NOT_OPEN \n");
        return ERROR_NOT_OPEN;
    }

    *duration = 1000;
    SXLOGD("getDuration duration, always return 0");
    return NO_ERROR;
}

status_t mATVAudioPlayer::release()
{
    SXLOGD("release");

	int ret = 0;
    int count = 100;
    SXLOGD("release mMutex.tryLock ()");

#ifndef CHANGE_AUDIO_PRIORITY
	int priority = getpriority(PRIO_PROCESS, 0);
	SXLOGD("mATV Render Thread priority is %d", priority);
#endif

    do
    {
        ret = mMutex.tryLock();
        if (ret)
        {
            SXLOGW("FMAudioPlayer::release() mMutex return ret = %d", ret);
            usleep(20 * 1000);
            count --;
        }
    }
    while (ret && count);  // only cannot lock
    
    reset_nosync();

    if (matv_use_analog_input == 1)
    {
        SXLOGD("line in path =>Stopping the render thread");
    }
	else if(matv_use_analog_input == 0)
	{
		SXLOGD("digtal path =>Stopping the render thread");
	}
	

    // TODO: timeout when thread won't exit
    // wait for render thread to exit
    if (mRenderTid > 0)
    {
        mExit = true;
        SXLOGD("release signal \n");
        mCondition.signal();
        SXLOGD("release wait \n");
        mCondition.waitRelative(mMutex, seconds(3));
    }

	deleteAudioRecord();

	mMutex.unlock();
    return NO_ERROR;
}

status_t mATVAudioPlayer::reset()
{
    SXLOGD("reset");
    Mutex::Autolock l(mMutex);
    return reset_nosync();
}

// always call with lock held
status_t mATVAudioPlayer::reset_nosync()
{
    SXLOGD("reset_nosync start");

    // Stop MATV chip audio  
    spATVCtrlService->ATVCS_matv_audio_stop();

    // reset flags
    mState = STATE_ERROR;
    mPlayTime = -1;
    mDuration = -1;
    mPaused = false;
    mRender = false;

    if (matv_use_analog_input == 1)
    {
        SXLOGD("line in path => stop");
/*
#ifndef FAKE_MATV
        if (mFd >= 0)
        {
            int Reg_Data[3] = {INFO_U2K_MATV_AUDIO_STOP, 0, 0};
            ::ioctl(mFd, YUSU_INFO_FROM_USER, &Reg_Data);
        }
#endif
*/
        AudioSystem::setParameters(0, (String8)"AtvAudioLineInEnable=0");
    }
	else
	{
        SXLOGD("digtal path => stop");
		AudioSystem::setParameters(0, (String8)"AudioSetMatvDigitalEnable=0");
	}

    SXLOGD("reset_nosync end");
    return NO_ERROR;
}

#ifdef FOR_MATV_AUDIO_PATH_CHANGE
status_t mATVAudioPlayer::setParameter(int key, const Parcel &request)
{
	SXLOGD("setParameter !!!");
	int audioPath = -1;
	
	if(KEY_PARAMETER_MATV_AUDIO_PATH == key)
	{
		SXLOGD("change matv audio path, now matv_use_analog_input = %d", matv_use_analog_input);
		request.readInt32(&audioPath);

		if(0 == audioPath)
		{
			SXLOGD("AUTO MODE,MATV Audio Path according to audiosetting configure");
		}
		else if(1 == audioPath)
		{
			SXLOGD("ANALOG MODE,MATV Audio Path: -> analog");
			matv_use_analog_input = 1;

		}
		else if(2 == audioPath)
		{
			SXLOGD("DIGTAL MODE,MATV Audio Path: -> digtal");
			matv_use_analog_input = 0;
		}
		else
		{
			SXLOGE("setParameter BAD VALUE");
			return BAD_VALUE;
		}
	}
	
	return NO_ERROR;
}
#endif

status_t mATVAudioPlayer::setLooping(int loop)
{
    SXLOGD("setLooping, do nothing");
    return NO_ERROR;
}

status_t mATVAudioPlayer::createOutputTrack()
{
    // open audio track
    SXLOGD("Create AudioTrack object: rate=%d, channels=%d\n", MATV_AUDIO_SAMPLING_RATE, MATV_AUDIO_CHANNEL_NUM);

    if (mAudioSink->open(MATV_AUDIO_SAMPLING_RATE, MATV_AUDIO_CHANNEL_NUM, CHANNEL_MASK_USE_CHANNEL_ORDER , AUDIO_FORMAT_PCM_16_BIT, 3) != NO_ERROR)
    {
        SXLOGE("mAudioSink open failed");
        return ERROR_OPEN_FAILED;
    }

    return NO_ERROR;
}

int mATVAudioPlayer::renderThread(void *p)
{
    return ((mATVAudioPlayer *)p)->render();
}

int mATVAudioPlayer::render()
{
    int result = -1;
    int temp;
    int current_section = 0;
    bool audioStarted = false;
    bool firstOutput = false;
    int t_result = -1;
    int bufSize = 0;
    int lastTime = 0;
    int thisTime = 0;
    int dataCount = 0;
    int frameCount = 0;
    int drop_buffer_number = 0;

    if (using_class_ab_amp == 1)
    {
        SXLOGD("using class AB for mATVAudioPlayer");
        drop_buffer_number = 3;
    }
    else
    {
        SXLOGD("using class D for mATVAudioPlayer");
        drop_buffer_number = 1;
    }

#ifdef ATV_AUDIO_FILELOG
    FILE *fp;
    fp = fopen("sdcard/test.pcm", "wb");
    SXLOGD("fp:%d", fp);
#endif


    // allocate render buffer
    //mAudioBuffer = new char[AUDIOBUFFER_SIZE];
/* 
#ifdef FAKE_MATV
    bufSize = GetReadBufferSize();
#else
    bufSize = GetReadBufferSize(mI2Sdriver);
#endif
*/
	bufSize = GetReadBufferSize();

    SXLOGD("got buffer size = %d", bufSize);
    mAudioBuffer = new char[bufSize * 2];
    mDummyBuffer = new char[bufSize * 2];
    memset(mDummyBuffer, 0, bufSize);


    SXLOGD("mAudioBuffer: 0x%p \n", mAudioBuffer);

    if (!mAudioBuffer)
    {
        SXLOGD("mAudioBuffer allocate failed\n");
        goto threadExit;
    }

#ifdef CHANGE_AUDIO_PRIORITY
    // if set prority false , force to set priority
    if (t_result == -1)
    {
        struct sched_param sched_p;
        sched_getparam(0, &sched_p);
        sched_p.sched_priority = RTPM_PRIO_MATV_AUDIOPLAYER;

        if (0 != sched_setscheduler(0, SCHED_RR, &sched_p))
        {
            SXLOGE("[%s] failed, errno: %d", __func__, errno);
        }
        else
        {
            sched_p.sched_priority = RTPM_PRIO_MATV_AUDIOPLAYER;
            sched_getparam(0, &sched_p);
            SXLOGD("sched_setscheduler ok, priority: %d", sched_p.sched_priority);
        }
    }
#endif

    // let main thread know we're readyfm
    {
		int ret = 0;
        int count = 100;
        SXLOGD("render mMutex.tryLock ()");
        do
        {
            ret = mMutex.tryLock();

            if (ret)
            {
                SXLOGW("mATVAudioPlayer::render() mMutex return ret = %d", ret);
                usleep(20 * 1000);
                count --;
            }
        }
        while (ret && count);  // only cannot lock
        mRenderTid = myTid();
        SXLOGD("render start mRenderTid=%d\n",mRenderTid);
        mCondition.signal();
		mMutex.unlock();
    }

    while (1)
    {
        long numread = 0;
        {
            Mutex::Autolock l(mMutex);

            // pausing?
            if (mPaused)
            {
                SXLOGD("render - pause\n");

                if (mAudioSink->ready())
                {
                    mAudioSink->pause();
                    mAudioSink->flush();
                }

				mAudioRecord->stop();
                usleep(300 * 1000);
                mRender = false;
                audioStarted = false;
            }

            // nothing to render, wait for client thread to wake us up
            if (!mExit && !mRender)
            {
                SXLOGD("render - signal wait\n");
                mCondition.wait(mMutex);
                frameCount = 0;
                SXLOGD("render - signal rx'd\n");
            }

            if (mExit)
            {
                break;
            }

            // We could end up here if start() is called, and before we get a
            // chance to run, the app calls stop() or reset(). Re-check render
            // flag so we don't try to render in stop or reset state.
            if (!mRender)
            {
                continue;
            }

            if (!mAudioSink->ready())
            {
                SXLOGD("render - create output track\n");

                if (createOutputTrack() != NO_ERROR)
                {
                    break;
                }
            }
        }

        // codec returns negative number on error
        if (numread < 0)
        {
            SXLOGE("Error numread=%ld", numread);
            sendEvent(MEDIA_ERROR);
            break;
        }

        // start audio output if necessary
        if (!audioStarted && !mPaused && !mExit)
        {
            SXLOGD("render - starting audio\n");
			mAudioRecord->start();
			mAudioSink->start();
            audioStarted = true;
            firstOutput = true;

            //firstly push some amount of buffer to make the mixer alive
            if ((temp = mAudioSink->write(mDummyBuffer, bufSize)) < 0)
            {
                SXLOGE("Error in writing:%d", temp);
                result = temp;
                break;
            }

            if ((temp = mAudioSink->write(mDummyBuffer, bufSize)) < 0)
            {
                SXLOGE("Error in writing:%d", temp);
                result = temp;
                break;
            }

            if ((temp = mAudioSink->write(mDummyBuffer, bufSize)) < 0)
            {
                SXLOGE("Error in writing:%d", temp);
                result = temp;
                break;
            }

            if ((temp = mAudioSink->write(mDummyBuffer, bufSize)) < 0)
            {
                SXLOGE("Error in writing:%d", temp);
                result = temp;
                break;
            }

        }

        {
            Mutex::Autolock l(mMutex);
            //int brt,art;
#ifdef FAKE_MATV
            numread = ReadFakeBuffer(mAudioBuffer);
#else
			numread = 0;
			memset(mAudioBuffer, 0,bufSize * 2);
			numread = mAudioRecord->read(mAudioBuffer, bufSize);
//			SXLOGD("size of mAudioRecord->read is %d", numread);
			
#ifdef DUMP_MATV_INPUT_DATA
			fwrite((void *)mAudioBuffer, 1, numread, fp);
#endif
#endif
            if (firstOutput && (numread > 0))
            {
                memset(mAudioBuffer, 0, numread);
                firstOutput = false;
            }

            frameCount++;
        }

        lastTime = thisTime;
        thisTime = getTimeMs();

        if (thisTime - lastTime > 160)
        {
            SXLOGW(" !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!time diff = %d", thisTime - lastTime);
        }

        // Write data to the audio hardware
        dataCount += numread;

        // skip 3 buffer here to compensate the wait for class-AB
        // for class-D, we skip 1 buffer only
        if (frameCount > drop_buffer_number)
        {
#ifdef MUTE_PAUSE
            if (mMutePause == true)
            {
            	SXLOGD("Mute Pause !!!");
                memset(mAudioBuffer, 0, numread);
            }

#endif

//			SXLOGD("size of mAudioSink->write is %d", numread);
            if ((temp = mAudioSink->write(mAudioBuffer, numread)) < 0)
            {
                SXLOGE("Error in writing:%d", temp);
                result = temp;
				mAudioRecord->stop();
                break;
            }
        }

        //sleep to allow command to get mutex
        usleep(1000);
    }

threadExit:

    // tell Audio Driver we are going to stop: for Daul Speaker (ClassD + ClassAB)
/*
#ifndef FAKE_MATV
    if (mFd >= 0)
    {
        int Reg_Data[3] = {INFO_U2K_MATV_AUDIO_STOP, 0, 0};
        ::ioctl(mFd, YUSU_INFO_FROM_USER, &Reg_Data);
    }
#endif
*/
    mAudioSink.clear();


    if (mAudioBuffer)
    {
        delete [] mAudioBuffer;
        mAudioBuffer = NULL;
    }

    if (mDummyBuffer)
    {
        delete [] mDummyBuffer;
        mDummyBuffer = NULL;
    }

    SXLOGD("render end mRenderTid=%d\n",mRenderTid);

    // tell main thread goodbye
    Mutex::Autolock l(mMutex);
    mRenderTid = -1;
    mCondition.signal();

#ifdef ATV_AUDIO_FILELOG
    fclose(fp);
#endif

#ifdef DUMP_MATV_INPUT_DATA
	if (fp)
	{
		fclose(fp);
		fp = NULL;
	}
	else
	{
		SXLOGE("fp of pcm file is NULL!!!");
	}
#endif

    return result;
}


bool mATVAudioPlayer::createAudioRecord(int frameCount)
{
	SXLOGD("createAudioRecord !!!");
	
	if(!mAudioRecord.get() || mAudioRecord->initCheck() != NO_ERROR)
	{
		if(mAudioRecord.get())
		{
			mAudioRecord.clear();
		}

		int count = 5;
		do{
			mAudioRecord = new AudioRecord(
			AUDIO_SOURCE_MATV,
			MATV_AUDIO_SAMPLING_RATE,
			AUDIO_FORMAT_DEFAULT,
			AUDIO_CHANNEL_IN_STEREO,
			frameCount,
			(AudioRecord::callback_t)NULL,
			(void *)NULL,
			0,
			0);
			if(mAudioRecord.get() && mAudioRecord->initCheck() == NO_ERROR)
			{
				SXLOGD("Create AudioRecord Success !!!");
				return true;
			}
			else
			{
				SXLOGW("Create AudioRecord Failed,Try again !!!");
			    if(mAudioRecord.get())
				{
					mAudioRecord.clear();
				}
				usleep(500*1000);
				count--;
			}
		}while(count>0);
		return false;
	}
	return true;
}
bool mATVAudioPlayer::deleteAudioRecord()
{
	SXLOGD("deleteAudioRecord !!!");

	if(mAudioRecord.get())
	{
		mAudioRecord.clear();
	}
	return true;
}



} // end namespace android

