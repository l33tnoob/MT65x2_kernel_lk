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
 * Copyright (C) 2007 The Android Open Source Project
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

#ifndef ANDROID_AUDIOPCMXWAY_H
#define ANDROID_AUDIOPCMXWAY_H

#include <stdint.h>
#include <sys/types.h>

#include <media/AudioSystem.h>

#include <utils/RefBase.h>
#include <utils/Errors.h>
#include <binder/IInterface.h>
#include <binder/IMemory.h>
#include <utils/threads.h>


namespace android
{

// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------

class AudioPCMxWay
{
public:

    /* Events used by AudioPCMxWay callback function
     */
    enum event_type
    {
        EVENT_MORE_DATA = 0,        // Request to write more data to PCM buffer.
        EVENT_UNDERRUN = 1,         // PCM buffer underrun occured.
        EVENT_BUFFER_END = 5        // Playback head is at the end of the buffer.
    };

    /* Stream type for Audio PCMxWay
     */
    enum stream_type
    {
        PCMXWAY_OUTPUT,
        PCMXWAY_INPUT
    };

    /* Create Buffer on the stack and pass it to obtainBuffer()
     * and releaseBuffer().
     */

    class Buffer
    {
    public:
        enum
        {
            MUTE    = 0x00000001
        };
        uint32_t    flags;
        int         channelCount;
        int         format;
        size_t      frameCount;
        size_t      size;
        union
        {
            void       *raw;
            short      *i16;
            int8_t     *i8;
        };
    };


    /* As a convenience, if a callback is supplied, a handler thread
     * is automatically created with the appropriate priority. This thread
     * invokes the callback when a new buffer becomes availlable or an underrun condition occurs.
     * Parameters:
     *
     * event:   type of event notified (see enum AudioTrack::event_type).
     * user:    Pointer to context for use by the callback receiver.
     * info:    Pointer to optional parameter according to event type:
     *          - EVENT_MORE_DATA: pointer to AudioTrack::Buffer struct. The callback must not write
     *          more bytes than indicated by 'size' field and update 'size' if less bytes are
     *          written.
     *          - EVENT_UNDERRUN: unused.
     *          - EVENT_LOOP_END: pointer to an int indicating the number of loops remaining.
     *          - EVENT_MARKER: pointer to an uin32_t containing the marker position in frames.
     *          - EVENT_NEW_POS: pointer to an uin32_t containing the new position in frames.
     *          - EVENT_BUFFER_END: unused.
     */

    typedef void (*callback_t)(int event, void *user, void *info);

    AudioPCMxWay();

    /*
     * Parameters:
     *
     * streamType:         Select the type of audio stream this track is attached to
     *                     (e.g. AudioSystem::MUSIC).
     * cbf:                Callback function. If not null, this function is called periodically
     *
     */

    AudioPCMxWay(int streamType,
                 callback_t cbf       = 0,
                 void *user           = 0
                );


    /* Terminates the AudioPCMxWay.
     * Also destroys all resources assotiated with the AudioTrack.
     */
    ~AudioPCMxWay();


    /* Initialize an uninitialized AudioPCMxWay.
     * Returned status (from utils/Errors.h) can be:
     *  - NO_ERROR: successful intialization
     *  - INVALID_OPERATION: AudioPCMxWay is already intitialized
     *  - BAD_VALUE: invalid parameter (channels, format, sampleRate...)
     *  - NO_INIT: audio server or audio hardware not initialized
     * */
    status_t    set(int streamType      = -1,
                    callback_t cbf      = 0,
                    void *user          = 0
                   );


    /* Result of constructing the AudioPCMxWay. This must be checked
     * before using any AudioPCMxWay API (except for set()), using
     * an uninitialized AudioPCMxWay produces undefined results.
     * See set() method above for possible return codes.
     */
    status_t    initCheck() const;


    /* After it's created the track is not active. Call start() to
     * make it active. If set, the callback will start being called.
     */
    void        start();

    /* Stop a track. If set, the callback will cease being called and
     * obtainBuffer returns STOPPED. Note that obtainBuffer() still works
     * and will fill up buffers until the pool is exhausted.
     */
    void        stop();


    /* As a convenience we provide a write() interface to the audio buffer.
     * This is implemented on top of lockBuffer/unlockBuffer. For best
     * performance
     *
     */
    ssize_t     write(const void *buffer, size_t size);

    /* As a convenience we provide a read() interface to the audio buffer.
    * This is implemented on top of lockBuffer/unlockBuffer.
    */
    ssize_t     read(void *buffer, size_t size);


private:
    /* copying audio tracks is not allowed */
    AudioPCMxWay(const AudioPCMxWay &other);
    AudioPCMxWay &operator = (const AudioPCMxWay &other);

    /* a small internal class to handle the callback */
    class AudioPCMxWayThread : public Thread
    {
    public:
        AudioPCMxWayThread(AudioPCMxWay &receiver);
    private:
        friend class AudioPCMxWay;
        virtual bool        threadLoop();
        virtual status_t    readyToRun();
        virtual void        onFirstRef();
        AudioPCMxWay &mReceiver;
        Mutex       mLock;
    };

    bool processAudioBuffer(const sp<AudioPCMxWayThread>& thread);

    status_t                mStatus;

    sp<AudioPCMxWayThread>  mAudioPCMxWayThread;

    bool                    mStarted;

    volatile int32_t        mActive;

    callback_t              mCbf;
    void                   *mUserData;
    uint8_t                 mStreamType;
};


}; // namespace android

#endif // ANDROID_AUDIOTRACK_H
