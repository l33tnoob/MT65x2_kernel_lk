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

#include "utils/Log.h"
#include "./inc/MpoStream.h"
#include "IFDWrapper.h"

#include <cutils/xlog.h>

#undef LOG_TAG
#define LOG_TAG "MPO_IFD_Wrapper"

//extern bool SYSTEM_BIG_ENDIAN;
//int testEndian = 1;
//const bool SYSTEM_BIG_ENDIAN = *((char*)(&testEndian)) != 1;

bool IFD_wrapper::readStream(MpoInputStream* stream, bool bigEndian, int startOfHeader, ExifIFD_t* pIFD)
{
    ExifIFD_t ifd;

    //check passed in Param
    if (NULL == stream || startOfHeader < 0 || NULL == pIFD) {
        XLOGE("IFD_wrapper:readStream:invalid stream or startOfHeader or ExifIFD* !");
        return false;
    }

//XLOGD("IFD_wrapper::readStream:startOfHeader=0x%x",startOfHeader);
//XLOGD("IFD_wrapper::readStream:stream->tell()=0x%x",stream->tell());

    //read IFD Tag
    pIFD->tag = stream->readUint16(bigEndian);
//XLOGV("IFD_wrapper::readStream:ExifIFD_t.tag=0x%x",pIFD->tag);

    //read IFD type
    pIFD->type = stream->readUint16(bigEndian);
//XLOGV("IFD_wrapper::readStream:ExifIFD_t.type=0x%x",pIFD->type);

    //read IFD Count
    pIFD->count = stream->readUint32(bigEndian);
//XLOGV("IFD_wrapper::readStream:ExifIFD_t.count=0x%x",pIFD->count);

    //read IFD value/offset
    pIFD->valoff = stream->readUint32(bigEndian);
//XLOGV("IFD_wrapper::readStream:ExifIFD_t.valoff=0x%x",pIFD->valoff);

//XLOGV("IFD_wrapper::readStream:after read ValOff, stream->tell()=0x%x",stream->tell());
    int endPos = stream->tell();
    //check whether need to seek to read value

    if (pIFD->type < FIRST_IFD_TYPE || pIFD->type > LAST_IFD_TYPE) {
        XLOGE("IFD_wrapper::readStream: invalid IFD type: %d",pIFD->type);
        return false;
    }
    if (pIFD->count <= 0) {
        XLOGE("IFD_wrapper::readStream: invalid IFD count: %d",pIFD->count);
        return false;
    }

    //for rational or signed rational, we read attributes.
//    if (TYPE_COUNT[pIFD->type] * pIFD->count > VALOFF_COUNT) {
    if (TYPE_COUNT[pIFD->type] > 4) {
//XLOGV("IFD_wrapper::readStream: we should seek to values section for Value");
        int valPos = pIFD->valoff + startOfHeader;
//XLOGV("IFD_wrapper::readStream: valuse position in file is 0x%x", valPos);
        stream->rewind();
        stream->skip(valPos);
        int value1 = stream->readUint32(bigEndian);
        int value2 = stream->readUint32(bigEndian);
//XLOGV("IFD_wrapper::readStream: valuse 1 is 0x%x", value1);
//XLOGV("IFD_wrapper::readStream: valuse 2 is 0x%x", value2);
        stream->rewind();
        stream->skip(endPos);
    }

    return true;
}

bool IFD_wrapper::writeStream(MpoOutputStream* stream,bool bigEndian,
                              ExifIFD_t* pIFD) {
//    if (SYSTEM_BIG_ENDIAN == bigEndian) {
//        int count = sizeof(ExifIFD_t);
//        if (count != stream->write((char*)pIFD, 0, count)) {
//            XLOGV("MP_Wrapper:writeStream:write IFD error!");
//            return false;
//        }
//        return true;
//    }

    //we write one by one, without fast
    //write IFD Tag
//XLOGV("MP_wrapper:writeStream:write IFD.tag 0x%x",pIFD->tag);
    if (false == stream->writeUint16(pIFD->tag,bigEndian)) {
        XLOGE("MP_wrapper:writeStream:write IFD.tag 0x%x failed",pIFD->tag);
        return false;
    }
    //write IFD type
//XLOGV("MP_wrapper:writeStream:write IFD.type 0x%x",pIFD->type);
    if (false == stream->writeUint16(pIFD->type,bigEndian)) {
        XLOGE("MP_wrapper:writeStream:write IFD.type 0x%x failed",pIFD->type);
        return false;
    }
    //write IFD Count
//XLOGV("MP_wrapper:writeStream:write IFD.count 0x%x",pIFD->count);
    if (false == stream->writeUint32(pIFD->count,bigEndian)) {
        XLOGE("MP_wrapper:writeStream:write IFD.count 0x%x failed",pIFD->count);
        return false;
    }
    //write IFD value
//XLOGV("MP_wrapper:writeStream:write IFD.valoff 0x%x",pIFD->valoff);
    if (false == stream->writeUint32(pIFD->valoff,bigEndian)) {
        XLOGE("MP_wrapper:writeStream:write IFD.valoff 0x%x failed",pIFD->valoff);
        return false;
    }


    return true;
}


