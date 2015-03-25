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

/* ***** BEGIN LICENSE BLOCK *****
 *
 * Portions Copyright (c) 1995-2012 RealNetworks, Inc. All Rights Reserved.
 *
 * The contents of this file, and the files included with this file,
 * are subject to the current version of the RealNetworks Public
 * Source License (the "RPSL") available at
 * http://www.helixcommunity.org/content/rpsl unless you have licensed
 * the file under the current version of the RealNetworks Community
 * Source License (the "RCSL") available at
 * http://www.helixcommunity.org/content/rcsl, in which case the RCSL
 * will apply. You may also obtain the license terms directly from
 * RealNetworks.  You may not use this file except in compliance with
 * the RPSL or, if you have a valid RCSL with RealNetworks applicable
 * to this file, the RCSL.  Please see the applicable RPSL or RCSL for
 * the rights, obligations and limitations governing use of the
 * contents of the file.
 *
 * Alternatively, the contents of this file may be used under the
 * terms of the GNU General Public License Version 2 or later (the
 * "GPL") in which case the provisions of the GPL are applicable
 * instead of those above. If you wish to allow use of your version of
 * this file only under the terms of the GPL, and not to allow others
 * to use your version of this file under the terms of either the RPSL
 * or RCSL, indicate your decision by deleting the provisions above
 * and replace them with the notice and other provisions required by
 * the GPL. If you do not delete the provisions above, a recipient may
 * use your version of this file under the terms of any one of the
 * RPSL, the RCSL or the GPL.
 *
 * This file is part of the Helix DNA Technology. RealNetworks is the
 * developer of the Original Code and owns the copyrights in the
 * portions it created.
 *
 * This file, and the files included with this file, is distributed
 * and made available on an 'AS IS' basis, WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESS OR IMPLIED, AND REALNETWORKS HEREBY DISCLAIMS
 * ALL SUCH WARRANTIES, INCLUDING WITHOUT LIMITATION, ANY WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, QUIET
 * ENJOYMENT OR NON-INFRINGEMENT.
 *
 * Technology Compatibility Kit Test Suite(s) Location:
 *    http://www.helixcommunity.org/content/tck
 *
 * Contributor(s):
 *
 * ***** END LICENSE BLOCK ***** */

/*
 * Copyright (C) 2008 The Android Open Source Project
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

#ifndef _HELIXPLAYER_H
#define _HELIXPLAYER_H

#include <media/mediaplayer.h>
#include <media/MediaPlayerInterface.h>

class PlayerDriver;

namespace android {

class HelixPlayer : public MediaPlayerInterface
{
public:
                        HelixPlayer();
    virtual             ~HelixPlayer();

    virtual status_t    initCheck();
    virtual status_t    setSigBusHandlerStructTLSKey(pthread_key_t key);
    virtual status_t    setDataSource(const char *url);
    virtual status_t    setDataSource(int fd, int64_t offset, int64_t length);
    virtual status_t    setDataSource(const char *url, const KeyedVector<String8, String8> *headers);
    virtual status_t    setVideoSurface(const sp<Surface>& surface);
    virtual status_t    setVideoSurfaceTexture(const sp<ISurfaceTexture>& surfaceTexture);
    virtual status_t    prepare();
    virtual status_t    prepareAsync();
    virtual status_t    start();
    virtual status_t    stop();
    virtual status_t    pause();
    virtual bool        isPlaying();
    virtual status_t    getVideoWidth(int *w);
    virtual status_t    getVideoHeight(int *h);
    virtual status_t    seekTo(int msec);
    virtual status_t    getCurrentPosition(int *msec);
    virtual status_t    getDuration(int *msec);
    virtual status_t    reset();
    virtual status_t    setLooping(int loop);
    //virtual player_type playerType() {return HELIX_PLAYER;}
    virtual player_type playerType() {return MTKDC_PLAYER;}
    virtual void        setAudioSink(const sp<AudioSink>& audioSink);
    virtual status_t    invoke(const Parcel& request, Parcel *reply);
    virtual status_t    getMetadata(const SortedVector<media::Metadata::Type>& ids, Parcel *records);
    virtual status_t    setParameters(const String8& params);
    virtual status_t    GetTrackInfoSize (unsigned short iTrkType, unsigned short *pSize);
    virtual status_t    GetAllTracksInfo (unsigned short iTrkType, unsigned short *pTrkNum, char *pTrkInfo);
    virtual status_t    GetCurrentTrackID(unsigned short iTrkType, unsigned short *pTrkID);
    virtual status_t    SetCurrentTrackID(unsigned short iTrkType, unsigned short trkID);
    virtual status_t    SetVelocity(int velocity);
    virtual status_t    GetVelocity(int* pVelocity);
    virtual status_t    setParameter(int key, const Parcel &request);
    virtual status_t    getParameter(int key, Parcel *reply);
#ifndef HELIX_PRODUCT_LEPHONE_DISABLE_SUSPEND 
    virtual status_t    suspend();
    virtual status_t    resume();
#endif
    
    //make available to AndroidClientContext
    void                sendEvent(int msg, int ext1 = 0, int ext2 = 0);
    void                setDuration(int duration) {m_Duration = duration;}
    void                setVideoWidth(int width) {m_VideoWidth = width;}
    void                setVideoHeight(int height) {m_VideoHeight = height;}
    void                setPlayerState(media_player_states state);
    bool                IsTrackLooping();    
    bool                IsPlayerStarted() { return m_bPlayerStarted; }

private:
    static void         do_nothing(status_t s, void*cookie) {}
    void                EventOccurred();

    status_t            m_Init;
    int                 m_Duration;
    int                 m_VideoWidth;
    int                 m_VideoHeight;
    media_player_states	m_eState;
    bool		m_bIsPlaying;
    int                 m_nMyFD;
    bool                m_bPlayerStarted;
#ifndef HELIX_PRODUCT_LEPHONE_DISABLE_SUSPEND 
    int                 m_PositionWhenSuspend;
    char*               m_DataSourcePath;
#endif
    sp<ISurfaceTexture> m_SurfaceTexture;
    PlayerDriver*       m_PlayerDriver;
};

}; // namespace android

#endif // _HELIXPLAYER_H
