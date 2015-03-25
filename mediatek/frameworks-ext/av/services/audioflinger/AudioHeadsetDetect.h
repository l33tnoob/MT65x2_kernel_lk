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

#ifndef __AUDIO_HEADSET_DETECT_H__
#define __AUDIO_HEADSET_DETECT_H__

#include <utils/threads.h>

namespace android {
/*Bootanimation is played before java stars up which is responsible for sending
** devices to native layer. if headset is pluged in and then phone is boot up, then sound will be
** output from speaker  firstly and then routed to headset when java starts up.
**This class is to detect the headset device before java starts up.
*/
    class HeadsetDetect
    {
    public:
        enum{
            NO_WIRED_LINE='0',
            WIRE_HEADSET='1',
            BIT_HEADSET_NO_MIC='2'
            };
        typedef void (*callback_t)(void* user, int device, bool on);
        HeadsetDetect(void * observer, callback_t cblk);
        ~HeadsetDetect();
		 status_t start();
         void stop();
		 bool isCurrentThread();
    private:
        HeadsetDetect(const HeadsetDetect &);
        HeadsetDetect & operator=(const HeadsetDetect&);
        bool loop();
        int detect();
        int parseState(char *buffer, int len);
        int readStateFromFile();
        bool headsetConnect(int newState);
		int  headsetType(int stateVal);
        int socketInit();
		int socketNextEvent(char* buffer, int buffer_length);
    private:
        volatile bool		mActive;
        bool				mOn;
        Mutex				mLock;
        void *				mObserver;
        callback_t			mCblk;
        int					mFd;
        nsecs_t				mStartTime;
		int					mDevice;
		bool				mFirsttime ;
        struct LooperThread;
        sp<LooperThread>	mThread;
    };
}
#endif
