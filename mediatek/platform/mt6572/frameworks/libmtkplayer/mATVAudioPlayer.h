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


#ifndef ANDROID_MATV_AUDIOPLAYER_H
#define ANDROID_MATV_AUDIOPLAYER_H


#include <utils/threads.h>

#include <media/MediaPlayerInterface.h>
#include <media/AudioTrack.h>
#include <media/mediaplayer.h>
#include <media/AudioRecord.h>


#include "media/IATVCtrlService.h"

#define ANDROID_LOOP_TAG "ANDROID_LOOP"
#define FOR_MATV_AUDIO_PATH_CHANGE


namespace android
{
class mATVAudioPlayer : public MediaPlayerInterface
{
public:
    mATVAudioPlayer();
    ~mATVAudioPlayer();

    virtual void        onFirstRef();
    virtual status_t    initCheck();
    virtual status_t    setDataSource(const char *path, const KeyedVector<String8, String8> *headers);
    virtual status_t    setDataSource(int fd, int64_t offset, int64_t length);
    virtual status_t    setVideoSurface(const sp<Surface>& surface)
    {
        return UNKNOWN_ERROR;
    }
    virtual status_t    setVideoSurfaceTexture(
        const sp<IGraphicBufferProducer>& bufferProducer)
    {
        return UNKNOWN_ERROR;
    }
    virtual status_t    prepare();
    virtual status_t    prepareAsync();
    virtual status_t    start();
    virtual status_t    stop();
    virtual status_t    seekTo(int msec);
    virtual status_t    pause();
    virtual bool        isPlaying();
    virtual status_t    getCurrentPosition(int *msec);
    virtual status_t    getDuration(int *msec);
    virtual status_t    release();
    virtual status_t    reset();
    virtual status_t    setLooping(int loop);
    virtual player_type playerType()
    {
        return MATV_AUDIO_PLAYER;
    }
    virtual status_t    invoke(const Parcel &request, Parcel *reply)
    {
        return INVALID_OPERATION;
    }
#ifdef FOR_MATV_AUDIO_PATH_CHANGE
	virtual status_t	setParameter(int key, const Parcel &request);
#else
    virtual status_t    setParameter(int key, const Parcel &request)
    {
        return INVALID_OPERATION;
    }
#endif
    virtual status_t    getParameter(int key, Parcel *reply)
    {
        return INVALID_OPERATION;
    }

private:
    status_t    setdatasource(const char *path, int fd, int64_t offset, int64_t length);
    status_t    reset_nosync();
    status_t    createOutputTrack();
    static  int         renderThread(void *);
    int         render();

	bool        createAudioRecord(int frameCount);
	bool        deleteAudioRecord();

    Mutex               mMutex;
	sp<AudioRecord>      mAudioRecord;
    Condition           mCondition;
    FILE               *mFile;
    int64_t             mOffset;
    int64_t             mLength;
    char               *mAudioBuffer;
    char               *mDummyBuffer;
    int                 mPlayTime;
    int                 mDuration;
	uint32_t			mMatvAudioSamplingRate;

    status_t            mState;
    audio_stream_type_t mStreamType;
    bool                mAndroidLoop;
    volatile bool       mExit;
    bool                mPaused;
    volatile bool       mRender;
    pid_t               mRenderTid;

    int mMutePause;

    // Audio driver
    int mFd;

    // Keeping pointer to ATVCtrlService
    sp<IATVCtrlService> spATVCtrlService;

};

}; // namespace android

#endif // ANDROID_MATV_AUDIOPLAYER_H
