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
 * Copyright (C) 2010 The Android Open Source Project
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

#define LOG_NDEBUG 0
#define LOG_TAG "APETAG"

#include "../include/APETag.h"

#include <media/stagefright/DataSource.h>
///#include <media/stagefright/MediaDebug.h>
#include <media/stagefright/Utils.h>
#include <utils/String8.h>
#include <byteswap.h>


#include <cutils/xlog.h>

#undef LOGV
#undef LOGD
#define LOGV(...)  SXLOGV(__VA_ARGS__)
#define LOGD(...)  SXLOGD(__VA_ARGS__)
#define LOGE(...)  SXLOGE(__VA_ARGS__)


namespace android
{


/*  define global variable
*/



static const size_t kMaxMetadataSize = 3 * 1024 * 1024;


int32_t meta_tag_find_terminator(uint8_t *pbuffer,
                                 uint32_t size, uint8_t encoding)
{
    int32_t i = 0, n;

    switch (encoding)
    {
        case META_TAG_TEXT_ENCODING_UTF16:
        case META_TAG_TEXT_ENCODING_UTF16_BE:

            n = (int32_t)(size / 2);

            while (i < n)
            {
                if (pbuffer[i] == 0 && pbuffer[i + 1] == 0)
                {
                    return i;
                }

                i += 2;
            }

            break;

        default:
            while (i < (int32_t)size)
            {
                if (pbuffer[i] == 0)
                {
                    return i;
                }

                i++;
            }

            break;
    }

    return  -1;
}


APETAG::APETAG(const sp<DataSource> &source)
    : mIsValid(false),
      mData(NULL),
      mSize(0),
      mFirstFrameOffset(0)
{

    mIsValid = parsetag(source);

}

APETAG::~APETAG()
{
    if (mData)
    {
        free(mData);
        mData = NULL;
    }
}

bool APETAG::isValid() const
{
    return mIsValid;
}


bool APETAG::parsetag(const sp<DataSource> &source)
{
    uint32_t u32Ver, u32Size, u32Item;
    uint8_t  u8TmpBuf[APE_READ_BUFFER_SIZE + 2];
    off64_t fileoffset = 0;
    ///LOGE("parsetag()");
    source->getSize(&fileoffset);

    if ((fileoffset < APE_TAG_FOOTER_SIZE)
            || (source->readAt(fileoffset - APE_TAG_FOOTER_SIZE, u8TmpBuf, APE_TAG_FOOTER_SIZE) != APE_TAG_FOOTER_SIZE))
    {

        LOGD("tail and read fail %x", fileoffset);
        return false;
    }

    ///LOGD("getSize = %x, %c, %c, %c, %x, %x, %x", fileoffset, u8TmpBuf[0], u8TmpBuf[1], u8TmpBuf[2], u8TmpBuf[0], u8TmpBuf[1], u8TmpBuf[2] );

    if (memcmp(u8TmpBuf, APE_TAG_MAGIC_ID, APE_TAG_MAGIC_SIZE))
    {
        LOGV("APE_TAG_MAGIC_ID tail error");
        return false;
    }

    ///fileoffset -= APE_TAG_FOOTER_SIZE;
    u32Ver = *(uint32_t *)(u8TmpBuf + 8);
    u32Size = *(uint32_t *)(u8TmpBuf + 12);
    u32Item = *(uint32_t *)(u8TmpBuf + 16);

    if ((u32Ver != 2000) && (u32Ver != 1000))
    {
        LOGV("read version error %x", u32Ver);
        return false;
    }

    /*not support item count more then APE_KEY_MAX_NUM*/
    if (u32Item > APE_KEY_MAX_NUM)
    {
        u32Item = APE_KEY_MAX_NUM;
    }

    fileoffset  -= u32Size;


    if ((u32Size - 32) > 0)
    {
        mData = (uint8_t *)malloc(u32Size);
    }
    else
    {
        return false;
    }

    mSize = u32Size;
    mFirstFrameOffset = fileoffset;
    LOGV("read version ok fileoff %x, %d, %x", mFirstFrameOffset, u32Ver, u32Size);

    if ((source->readAt(fileoffset, mData, u32Size) != u32Size))
    {
        if (mData)
        {
            free(mData);
        }

        mData = NULL;
        return false;
    }

    ///LOGE("read parsetag() exit");
    return true;

}


APETAG::Iterator::Iterator(const APETAG &parent, const char *id, uint16_t len)
    : mParent(parent),
      mID(NULL),
      ///mOffset(mParent.mFirstFrameOffset),
      mOffset(0),
      mFrameData(NULL),
      mFrameSize(0)
{
    if (id)
    {
        mID = strdup(id);
    }

    mIDLen = len;
    findFrame();
}

APETAG::Iterator::~Iterator()
{
    if (mID)
    {
        free(mID);
        mID = NULL;
    }
}

bool APETAG::Iterator::done() const
{
    return mFrameData == NULL;
}

void APETAG::Iterator::getString(String8 *id) const
{

    ///LOGE("getString() %x, %x, %x, flag %x", mFrameData, mFrameSize, getHeaderLength(), *(int32_t *)(mFrameData+4));
    id->setTo("");

    if (mFrameData == NULL)
    {
        return;
    }

    size_t n = mFrameSize - getHeaderLength();

    char *temp = (char *) mFrameData + getHeaderLength();

    for (int i = 0; i < n ; i++)
    {
        LOGV("getString() rawdata %x!!!!!",  *(temp + i));
    }

    if (((*(int32_t *)(mFrameData + 4)) & 0xe) == 0x00) // UTF-8
    {
        id->setTo((const char *)(mFrameData + getHeaderLength()), n);
    }
    else  // ISO 8859-1
    {
        LOGE("getString() parse error %x!!!!!",  *(int32_t *)(mFrameData + 4));
        ///convertISO8859ToString8(mFrameData + getHeaderLength(), n, id);
    }
}

size_t APETAG::Iterator::getHeaderLength() const
{
    return (9 + mIDLen);
}

void APETAG::Iterator::findFrame()
{
    ///LOGE("findFrame(), %x, %x", mOffset, mParent.mSize);
    for (;;)
    {
        mFrameData = NULL;
        mFrameSize = 0;

        if (mOffset > mParent.mSize)
        {

            LOGD("findFrame() fail, %x, %x", mOffset, mParent.mSize);
            return;
        }

        mFrameSize =
            +(mParent.mData[mOffset + 3] << 24)
            + (mParent.mData[mOffset + 2] << 16)
            + (mParent.mData[mOffset + 1] << 8)
            + mParent.mData[mOffset + 0];

        ///LOGE("findFrame at offset %x size = %x, mParent.mSize = %x",
        ///  mOffset, mFrameSize, mParent.mSize);

        if (mOffset + mFrameSize > mParent.mSize)
        {
            return;
        }

        ///mFrameData = mParent.mData + mOffset + 8;
        ///first find the key len.
        int32_t  s32Tmp = meta_tag_find_terminator(mParent.mData + mOffset + 8, APE_KEY_MAX_LEN,
                          META_TAG_TEXT_ENCODING_ISO_8859_1);

        if (!mID)
        {
            break;
        }

        char id[mIDLen + 1];
        memcpy(id, mParent.mData + mOffset + 8, mIDLen);
        id[mIDLen] = '\0';

        ///     for(int t=0;t<mIDLen; t++)
        ///         LOGE("meta_tag_find_terminator --%c, %c", id[t], mID[t]);

        ///mFrameData = mParent.mData + mOffset + 9 + s32Tmp;
        mFrameData = (mParent.mData + mOffset);
        mFrameSize += (9 + s32Tmp);

        if ((s32Tmp == strlen(mID))
                && (!strcmp(id, mID)))
        {
            LOGV("meta_tag_find_terminator ok %s,mIDLen %d, strlen %d!!!", mID, strlen(mID), s32Tmp);
            break;
        }

        mOffset += mFrameSize;
    }

}

}  // namespace android
