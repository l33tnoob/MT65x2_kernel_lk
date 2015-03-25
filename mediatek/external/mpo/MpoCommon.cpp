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

#include <utils/Log.h>
#include <cutils/xlog.h>

#include "./inc/MpoCommon.h"
#include "IFDWrapper.h"


#undef LOG_TAG
#define LOG_TAG "MPO_Common"

int testEndian = 1;
const bool SYSTEM_BIG_ENDIAN = *((char*)(&testEndian)) != 1;

//the following varible is define to pass MTK_MPO_type into
//MPO index IFD. it is a temporary and bad solution.
int gMtkMpoType=0;
int gOutMtkMpoType=0;

bool strEqual(const unsigned char *buf1, const unsigned char* buf2,int count)
{
    if (count <=0 || NULL == buf1 || NULL == buf2) {
        XLOGE("bad str compare, please check input");
        return false;
    }
    for (int i=0; i< count; i++) {
        if (buf1[i] != buf2[i]) {
            return false;
        }
    }
    return true;
}

//////////////////MP_IFD//////////////////
MP_IFD::MP_IFD()
{
    mIFD_Ptr = NULL;
}

MP_IFD::~MP_IFD()
{
    if (mIFD_Ptr)
        delete mIFD_Ptr;
    mIFD_Ptr = NULL;
}

bool MP_IFD::readStream(MpoInputStream* stream,bool bigEndian)
{
    XLOGI("MP_IFD::readStream:current position=0x%x",(int)stream->tell());
    int i;
    //read MP Index/Attribute IFD Count
    mCount = stream->readUint16(bigEndian);
    if (mCount < 0) {
        XLOGE("invalid MP Index/Attribute IFD Count");
        return false;
    }
    //read each IFD 
    mIFD_Ptr = new ExifIFD_t[mCount];
    for(i=0; i<mCount; i++) {
        //read MP Index/Attribute IFD Tag
        mIFD_Ptr[i].tag = stream->readUint16(bigEndian);
        //read MP Index/Attribute IFD type
        mIFD_Ptr[i].type = stream->readUint16(bigEndian);
        //read MP Index/Attribute IFD Count
        mIFD_Ptr[i].count = stream->readUint32(bigEndian);
        //read MP Index/Attrbute IFD value/offset
        mIFD_Ptr[i].valoff = stream->readUint32(bigEndian);
    }

    //read offset of Next IFD
    mIFD_Offset = stream->readUint32(bigEndian);
    XLOGD("MP_IFD::readStream:mIFD_Offset=0x%x",mIFD_Offset);
    //till now, the MP Index IFD file is all processed

    return true;
}

bool MP_IFD::readStream(MpoInputStream* stream,bool bigEndian,int startOfHeader)
{
    XLOGI("MP_IFD::readStream:current position=0x%x",(int)stream->tell());
    int i;
    //read MP Index/Attribute IFD Count
    mCount = stream->readUint16(bigEndian);
    if (mCount < 0) {
        XLOGE("invalid MP Index/Attribute IFD Count");
        return false;
    }
    //read each IFD 
    mIFD_Ptr = new ExifIFD_t[mCount];
    for(i=0; i<mCount; i++) {
        IFD_wrapper::readStream(stream, bigEndian, startOfHeader, mIFD_Ptr+i);
    }

    //read offset of Next IFD
    mIFD_Offset = stream->readUint32(bigEndian);
    XLOGD("MP_IFD::readStream:mIFD_Offset=0x%x",mIFD_Offset);
    //till now, the MP Index IFD file is all processed

    return true;
}

//////////////////////////////////////////////////////

//////////////////////MP_Index_IFD//////////////////////////
MP_Index_IFD::MP_Index_IFD()
{
    mMPEntry = NULL;
}

MP_Index_IFD::~MP_Index_IFD()
{
    if (mMPEntry)
        delete [] mMPEntry;
    mMPEntry = NULL;
}

MP_Index_IFD::MP_Index_IFD(MPImageInfo* pMPImageInfo,MP_Image** pMP_Images,
                 int mpImageNum,Exif_Image* jpegSrc,bool bigEndian)
{
    mMPEntry = NULL;

    this->pMPImageInfo = pMPImageInfo;
    this->mEncodedMPImages = pMP_Images;
    this->mEncodedMPNum = mpImageNum;
    mJpegSrc = jpegSrc;
    mBigEndian = bigEndian;
}

bool MP_Index_IFD::processIFD_tag(ExifIFD_t* pIFD_t)
{
    if (NULL == pIFD_t) {
        XLOGE("processIFD_tag:pIFD_t==NULL");
        return false;
    }
    switch (pIFD_t->tag) {
    case TAG_MPFVersion:
        break;
    case TAG_ImageNumber:
        mImageNum = pIFD_t->valoff;
        break;
    case TAG_MPEntry:
        mEntryNum = pIFD_t->count / 16;
        break;
    case TAG_ImageUIDList:
        mImageIdList = pIFD_t->valoff;
        break;
    case TAG_TotalFrames:
        mTotalFramNum = pIFD_t->valoff;
       break;
    default:
        XLOGE("unknown IFD tag type %x!",pIFD_t->tag);
        return false;
    }
    return true;
}

bool MP_Index_IFD::readStream(MpoInputStream* stream,bool bigEndian,int startOfHeader)
{
    int i;
    bool ok = false;
    MP_IFD::readStream(stream, bigEndian, startOfHeader);
    for(i=0; i<mCount; i++) {
        //recode IFD info in MpoDecoder
        ok = processIFD_tag(mIFD_Ptr+i);
        if (!ok)
            return false;
    }

    //the following code process MP Index IFD Value field
    //alloc memory for each MPEntry
    XLOGV("MP_Index_IFD::readStream() before read MP Entry:position=0x%x",(int)stream->tell());
    mMPEntry = new MPEntry[mEntryNum];
    for (i=0; i<mEntryNum; i++) {
        //read MP Entry Individual Image Attribute
        mMPEntry[i].imageAttr = stream->readUint32(bigEndian);
        XLOGD("MP_Index_IFD::readStream:mMPEntry[%d].imageAttr=0x%x",i,mMPEntry[i].imageAttr);
        //read MP Entry Individual Image Size
        mMPEntry[i].imageSize = stream->readUint32(bigEndian);
        XLOGI("MP_Index_IFD::readStream:mMPEntry[%d].imageSize=0x%x",i,mMPEntry[i].imageSize);
        //read MP Entry Individual Image Data Offset
        mMPEntry[i].imageDataOffset = stream->readUint32(bigEndian);
        XLOGV("MP_Index_IFD::readStream:mMPEntry[%d].imageDataOffset=0x%x",i,mMPEntry[i].imageDataOffset);
        //read MP Entry Dependent image 1 Entry Number
        mMPEntry[i].dependEntryNum1 = stream->readUint16(bigEndian);
        XLOGV("MP_Index_IFD::readStream:mMPEntry[%d].dependEntryNum1=0x%x",i,mMPEntry[i].dependEntryNum1);
        //read MP Entry Dependent image 2 Entry Number
        mMPEntry[i].dependEntryNum2 = stream->readUint16(bigEndian);
        XLOGV("MP_Index_IFD::readStream:mMPEntry[%d].dependEntryNum2=0x%x",i,mMPEntry[i].dependEntryNum2);
    }

    //As we've added a hidden tag just behind MPO Entry Values,
    //we should check whether this tag exits. For other kind of
    //MPO file made by other manufactory, we set this type to be
    //none.
    if (stream->tell() + 4 == startOfHeader + mIFD_Offset) {
        //read internal MPO type
        gOutMtkMpoType = stream->readUint32(bigEndian);
        XLOGI("MP_Index_IFD::readStream:read internal MPO type=0x%x",gOutMtkMpoType);
    } else {
        gOutMtkMpoType = MTK_TYPE_NONE;
    }

    //process Individual Image Unique IDs
    if (0 != mImageIdList) {
        //do something
    }
    XLOGV("MP_Index_IFD::readStream() before return:position=0x%x",(int)stream->tell());

    return true;
}

int MP_Index_IFD::getImageCount()
{
    return mEntryNum;
}

int MP_Index_IFD::getOffsetInMPExtension(int imageIndex)
{
    if (imageIndex < 0 || imageIndex >= mEntryNum)
        return -1;
    if (NULL == mMPEntry)
        return -1;
    return mMPEntry[imageIndex].imageDataOffset;
}

int MP_Index_IFD::getImageSize(int imageIndex)
{
    if (imageIndex < 0 || imageIndex >= mEntryNum)
        return -1;
    if (NULL == mMPEntry)
        return -1;
    return mMPEntry[imageIndex].imageSize;
}


bool MP_Index_IFD::writeStream(MpoOutputStream* stream,int startOfHeader)
{
    ExifIFD_t ifd;
    int ifdCount = 0;
    //collect all IFD that need to be write to file
    for (int i=0; i < MP_INDEX_TAG_NUM; i++) {
        if (TAG_SUPP_LEVEL_RECOMMENDED == MPO_Index_Tag_Level[i]) {
            ifdCount ++;
        }
    }
    int ifdLength = 2 /*length*/ + 12 /*size of IFD*/ * ifdCount + 4;
    int curValOff = stream->tell() - startOfHeader + ifdLength;

    //write count field
    if (false == stream->writeUint16(ifdCount,mBigEndian)) {
        XLOGE("MP_Index_IFD:writeSteram:write count=%d failed",ifdCount);
        return false;
    }

    //write all index IFD tags into file
    for (int i=0; i < MP_INDEX_TAG_NUM; i++) {
        if (TAG_SUPP_LEVEL_RECOMMENDED == MPO_Index_Tag_Level[i]) {
            ifd = indexTagIFD[i];
            switch(ifd.tag) {
                case TAG_MPFVersion: ifd.valoff = MPVersionDefaultInt;break;
                case TAG_ImageNumber:ifd.valoff = mEncodedMPNum;      break;
                case TAG_MPEntry:
                        ifd.count  = 16 * mEncodedMPNum;
                        ifd.valoff = curValOff;
                        curValOff += TYPE_COUNT[ifd.type]*ifd.count;
                        //add internal MPO type
                        curValOff += 4;
                    break;
                case TAG_TotalFrames:ifd.valoff = mEncodedMPNum;      break;
                default:XLOGE("encountered unhandled index IFD tag!");
            }
            IFD_wrapper::writeStream(stream,mBigEndian,&ifd);
        }
    }

    //write offset to next ifd: current ValOff
    if (false == stream->writeUint32(curValOff,mBigEndian)) {
        XLOGE("MP_Index_IFD:writeSteram:write offset to next ifd =%d failed",curValOff);
        return false;
    }

    //remember the start of the MP Entry values, wait to be re-written
    if (stream->markSupported()) {
        stream->mark();
    }

    char zeroBuf[16];
    memset(zeroBuf, 0x00, 16);

    //the following code encode MP Index IFD Value field
    for (int i=0; i<mEncodedMPNum; i++) {
        if (false == stream->write(zeroBuf, 0, 16)) {
            XLOGE("MP_Index_IFD:writeStream:can not write 16 bytes of zero for image %d",i);
            return false;
        }
    }

    //temporarily solution to distinguish 3D panorama and stereo image.
    //write 4 bytes to store internal MPO type
    XLOGI("MP_Index_IFD:writeStream:write internal MPO type 0x%x",gMtkMpoType);
    if (false == stream->writeUint32(gMtkMpoType, mBigEndian)) {
        XLOGE("MP_Index_IFD:writeStream:can not write internal MPO type");
        return false;
    }
    

    return true;
}

///////////////////////////////////////////////

/////////////////MP_Attr_IFD/////////////////////

MP_Attr_IFD::MP_Attr_IFD()
{

}

MP_Attr_IFD::MP_Attr_IFD(MPImageInfo* pMPImageInfo,bool bigEndian)
{
    this->pMPImageInfo = pMPImageInfo;
    mBigEndian = bigEndian;
}

bool MP_Attr_IFD::processIFD_tag(ExifIFD_t* pIFD_t)
{
    if (NULL == pIFD_t) {
        XLOGE("processIFD_tag:pIFD_t==NULL");
        return false;
    }
    switch (pIFD_t->tag) {
    case TAG_MPFVersion:
        mVersion = pIFD_t->tag;
        break;
    case TAG_MPIndividualNum:
        mImageNum = pIFD_t->valoff;
        break;
    case TAG_PanOrientation:
        mPanOrientation = pIFD_t->valoff;
        break;
    case TAG_PanOverlap_H:
        mPanOverlap_H = pIFD_t->valoff;
        break;
    case TAG_PanOverlap_V:
        mPanOverlap_V = pIFD_t->valoff;
        break;
    case TAG_BaseViewpointNum:
        mBaseViewpointNum = pIFD_t->valoff;
        break;
    case TAG_ConvergenceAngle:
        mConvergenceAngle = pIFD_t->valoff;
        break;
    case TAG_BaselineLength:
        mBaselineLength = pIFD_t->valoff;
        break;
    case TAG_VerticalDivergence:
        mVerticalDivergence = pIFD_t->valoff;
        break;
    case TAG_AxisDistance_X:
        mAxisDistance_X = pIFD_t->valoff;
        break;
    case TAG_AxisDistance_Y:
       mAxisDistance_Y = pIFD_t->valoff;
        break;
    case TAG_AxisDistance_Z:
        mAxisDistance_Z = pIFD_t->valoff;
        break;
    case TAG_YawAngle:
        mYawAngle = pIFD_t->valoff;
        break;
    case TAG_PitchAngle:
        mPitchAngle = pIFD_t->valoff;
        break;
    case TAG_RollAngle:
        mRollAngle = pIFD_t->valoff;
        break;
    default:
        XLOGE("unknown IFD tag type!");
        return false;
    }
    return true;
}

bool MP_Attr_IFD::readStream(MpoInputStream* stream,bool bigEndian,int startOfHeader)
{
    int i;
    bool ok = false;
    MP_IFD::readStream(stream, bigEndian, startOfHeader);
    for(i=0; i<mCount; i++) {
        //recode IFD info in MpoDecoder
        ok = processIFD_tag(mIFD_Ptr+i);
        if (!ok)
            return false;
    }

    return true;
}

bool MP_Attr_IFD::writeStream(MpoOutputStream* stream, bool encodeMPVersion, 
                              int imageNum,int startOfHeader)
{
    int mpType = 0;
    if (TYPE_Disparity == pMPImageInfo->type) {
        mpType = 1;
    }
    int imageLoc = 0;
    if (1 != pMPImageInfo->MPIndividualNum) {
        imageLoc = 1;
    }
    ExifIFD_t ifd;
    AttrIFDValues attrValues;
    //dump mp image info into array
    attrValues.values[0][0] = MPVersionDefaultInt;
    attrValues.values[1][0] = pMPImageInfo->MPIndividualNum;
    attrValues.values[5][0] = pMPImageInfo->BaseViewpointNum;
    attrValues.values[6][0] = pMPImageInfo->ConvergenceAngel[0];
    attrValues.values[6][1] = pMPImageInfo->ConvergenceAngel[1];
    attrValues.values[7][0] = pMPImageInfo->BaselineLength[0];
    attrValues.values[7][1] = pMPImageInfo->BaselineLength[1];

    int ifdCount = 0;
    for (int i=0; i < MP_ATTR_TAG_NUM; i++) {
        if (TAG_SUPP_LEVEL_RECOMMENDED == MPO_Attr_Tag_Level[mpType][imageLoc][i]) {
            ifdCount ++;
        }
    }
    int ifdLength = 2 /*length*/ + 12 * ifdCount + 4;
    int curValOff = stream->tell() - startOfHeader + ifdLength;
    if (false == stream->writeUint16(ifdCount,mBigEndian)) {
        XLOGE("MP_Attr_IFD:writeSteram:write count=%d failed",ifdCount);
        return false;
    }

    for (int i=0; i < MP_ATTR_TAG_NUM; i++) {
        if (TAG_SUPP_LEVEL_RECOMMENDED == MPO_Attr_Tag_Level[mpType][imageLoc][i]) {
            ifd = attrTagIFD[i];
            if (8 == TYPE_COUNT[ifd.type]) {
                ifd.valoff = curValOff;
                curValOff += 8;
            } else {
                ifd.valoff = attrValues.values[i][0];
            }
            IFD_wrapper::writeStream(stream,mBigEndian,&ifd);
        }
    }

    //write offset to next ifd: 0
    if (false == stream->writeUint32(0,mBigEndian)) {
        XLOGE("MP_Attr_IFD:writeSteram:write offset to next ifd =%d failed",0);
        return false;
    }

    //write ifd values if any
    for (int i=0; i < MP_ATTR_TAG_NUM; i++) {
        if (TAG_SUPP_LEVEL_RECOMMENDED == MPO_Attr_Tag_Level[mpType][imageLoc][i]) {
            ifd = attrTagIFD[i];
            if (8 == TYPE_COUNT[ifd.type]) {
                if (false == stream->writeUint32(attrValues.values[i][0],mBigEndian)) {
                    XLOGE("MP_Attr_IFD:writeStream:value 1 0x%x failed",attrValues.values[i][0]);
                    return false;
                }
                if (false == stream->writeUint32(attrValues.values[i][1],mBigEndian)) {
                    XLOGE("MP_Attr_IFD:writeStream:value 2 0x%x failed",attrValues.values[i][1]);
                    return false;
                }
            }
        }
    }
    return true;
}

//////////////////MP_IFD//////////////////

///////////////////MP_Extensions////////////////////

MP_Extensions::MP_Extensions()
{
    mMPIndexIFD = NULL;
    mMPAttrIFD = NULL;
}

MP_Extensions::MP_Extensions(MPImageInfo* pMPImageInfo,MP_Image** pMP_Images,
                              int mpImageNum,Exif_Image* jpegSrc)
{
    mMPIndexIFD = NULL;
    mMPAttrIFD = NULL;

    this->pMPImageInfo = pMPImageInfo;
    this->mEncodedMPImages = pMP_Images;
    this->mEncodedMPNum = mpImageNum;
    mJpegSrc = jpegSrc;
}

MP_Extensions::~MP_Extensions()
{
    if (NULL != mMPIndexIFD) {
        delete mMPIndexIFD;
    }
    mMPIndexIFD = NULL;
    if (NULL != mMPAttrIFD) {
        delete mMPAttrIFD;
    }
    mMPAttrIFD = NULL;

}

//extract info from a stream and record them.
bool MP_Extensions::readStream(MpoInputStream* stream, bool decodeIndexIFD)
{
    char buffer[4];
    int headerPos = stream->tell();
    //read MP endian property
    if (4 != stream->read(buffer, 4)) {
        XLOGE("read MP endian property failed");
        return false;
    }
    if (true == strEqual((unsigned char*)buffer,LT_Endian,4)) {
        mBigEndian = false;
    } else {
        if (true == strEqual((unsigned char*)buffer,BG_Endian,4)) {
            mBigEndian = true;
        } else {
            XLOGE("wrong endian property");
            return false;
        }
    }

    //read offset to 1st IFD
    int mIFD_Offset = stream->readUint32(mBigEndian);

    //jump to MP Index/Attribute IFD
    if(false == stream->skip(mIFD_Offset - MP_HEADER_LENGTH)) {
        XLOGE("jump to MP Index/Attribute IFD failed");
        return false;
    }

    //read MP Index IFD if needed
    if (decodeIndexIFD) {
        mMPIndexIFD = new MP_Index_IFD();
        mMPIndexIFD->readStream(stream,mBigEndian,headerPos);
    }

    //read MP Attribute IFD
    mMPAttrIFD = new MP_Attr_IFD();
    mMPAttrIFD->readStream(stream,mBigEndian,headerPos);
    return true;
}

int MP_Extensions::getImageCount()
{
    if (NULL == mMPIndexIFD)
        return 0;
    return mMPIndexIFD->getImageCount();
}

int MP_Extensions::getOffsetInMPExtension(int imageIndex)
{
    if (NULL == mMPIndexIFD)
        return -1;
    return mMPIndexIFD->getOffsetInMPExtension(imageIndex);
}

int MP_Extensions::getImageSize(int imageIndex)
{
    if (NULL == mMPIndexIFD)
        return 0;
    return mMPIndexIFD->getImageSize(imageIndex);
}

//dump recorded info into the stream.
bool MP_Extensions::writeStream(MpoOutputStream* stream)
{
    int headerPos = stream->tell();
    //check whether stream is invalid
    if (NULL == stream) {
        XLOGE("MP_Extensions:writeStream: stream is invalid!");
        return false;
    }

    char buffer[4];

    //MP endian should align with APP1 endian.
    //As our camera always capture camera with little endian
    //App1, this is set to be little endian
    bool bigEndian = false;

    //write MP endian property
//    if (4 != stream->write((char*)BG_Endian, 0, 4)) {
    if (4 != stream->write((char*)LT_Endian, 0, 4)) {
        XLOGE("MP_Extensions:write MP endian property failed");
        return false;
    }

    //read offset to 1st IFD
    int IFD_Offset = MP_HEADER_LENGTH;
//XLOGV("MP_Extensions:writeStream:write offset to first IFD 0x%x",IFD_Offset);
    if (false == stream->writeUint32(IFD_Offset, bigEndian)) {
        XLOGE("MP_Extensions:writeStream:write offset to first IFD %d failed",IFD_Offset);
        return false;
    }

    //write MP Index IFD if needed
//    if (NULL != mEncodedMPImages && 0 != mEncodedMPNum) {
    if (0 != mEncodedMPNum) {
        mMPIndexIFD = new MP_Index_IFD(pMPImageInfo,mEncodedMPImages,
                                       mEncodedMPNum,mJpegSrc,bigEndian);
        mMPIndexIFD->writeStream(stream, headerPos);
        delete mMPIndexIFD;
        mMPIndexIFD = NULL;
    }

    int imageNum = 1;
    if (0 == mEncodedMPNum) {
        imageNum = 2;
    }

    //write MP Attribute IFD
    mMPAttrIFD = new MP_Attr_IFD(pMPImageInfo,bigEndian);
//    mMPAttrIFD->writeStream(stream);
    mMPAttrIFD->writeStream(stream, 0 == mEncodedMPNum, imageNum, headerPos);
    delete mMPAttrIFD;
    mMPAttrIFD = NULL;

    return true;
}

///////////////////MP_Extensions////////////////////

//////////////APP1///////////////////////

bool APP1::readStream(MpoInputStream* stream)
{
    char buffer[8];
    //record app1 offset in file
    mOffsetInFile = stream->tell();
    //read App Marker
    if (2 != stream->read(buffer, 2)) {
        XLOGE("read APP Marker failed");
        return false;
    }
    if (buffer[0] == APP0_MARKER[0] &&
        buffer[1] == APP0_MARKER[1]) {
        //read App0 length
         int length = ((MpoInputStream*)stream)->readUint16(true);//it seems length of APPn is big endiant by default
        if (length < 2 ) {
            XLOGE("invalid APP0 length: %d",length);
            return false;
        }
        //skip the rest of APP0
        //does APP0 Marker not included in APP0 Length????????
        if (false == stream->skip(length-2)) {
            XLOGE("skip rest of APP0 failed");
            return false;
        }
    } else {
        //moves back to start of Marker
        if (false == stream->skip(-2)) {
            XLOGE("moves back to start of Marker 0x %x %x failed",buffer[0],buffer[1]);
            return false;
        }
    }

    mOffsetInFile = stream->tell();
    //read App1 Marker
    if (2 != stream->read(buffer, 2)) {
        XLOGE("read APP1 Marker failed");
        return false;
    }
    if (buffer[0] != APP1_MARKER[0] ||
        buffer[1] != APP1_MARKER[1]) {
        mFieldLength = 0;
        if (false == stream->skip(-2)) {
            XLOGE("moves back to start of Marker 0x %x %x failed",buffer[0],buffer[1]);
            return false;
        }
        return true;
    }
    //read App1 length
    int length = ((MpoInputStream*)stream)->readUint16(true);//it seems length of APPn is big endian by default.
    if (length < 2 ) {
        XLOGE("invalid APP1 length: %d",length);
        return false;
    }

    //record APP1 length including APP1 Marker
    mFieldLength = length + 2;

    //skip the rest of APP1
    //does APP1 Marker not included in APP1 Length????????
    if (false == stream->skip(length-2)) {
        XLOGE("skip rest of APP1 failed");
        return false;
    }
    return true;
}

//dump recorded info into the stream.
bool APP1::writeStream(MpoOutputStream* stream)
{
    return false;
}

//////////////////////////////////////////

/////////////APP2/////////////////////////
APP2::APP2()
{
    
}
APP2::~APP2()
{
    
}

bool APP2::readStream(MpoInputStream* stream,bool noUse)
{
    int i,j;
    char buffer[8];
    //record app2 offset in file
    mOffsetInFile = stream->tell();
    //read App2 Marker
    if (2 != stream->read(buffer, 2)) {
        XLOGE("read APP2 Marker failed");
        return false;
    }
    if (buffer[0] != APP2_MARKER[0] ||
        buffer[1] != APP2_MARKER[1]) {
        mFieldLength = 0;
        if (false == stream->skip(-2)) {
            XLOGE("moves back to start of Marker 0x %x %x failed",buffer[0],buffer[1]);
            return false;
        }
        return true;
    }

    //read App2 length
    int length = stream->readUint16(true);//it seems length of APPn is big endiant by default
    if (length < 2 ) {
        XLOGE("invalid APP2 length: %d",length);
        return false;
    }

    //record APP2 length including APP1 Marker
    mFieldLength = length + 2;

    //skip the rest of APP2
    if (false == stream->skip(length-2)) {
        XLOGE("skip rest of APP2 failed");
        return false;
    }

    return true;
}

//dump recorded info into the stream.
bool APP2::writeStream(MpoOutputStream* stream)
{
    return false;
}

///////////////////////////////////////////


///////////////MP_APP2///////////////////////

MP_APP2::MP_APP2()
{
    mMP_Extensions = NULL;
}

MP_APP2::MP_APP2(MPImageInfo* pMPImageInfo,MP_Image** pMP_Images,
                 int mpImageNum,Exif_Image* jpegSrc)
{
    mMP_Extensions = NULL;
    this->pMPImageInfo = pMPImageInfo;//!!!!!!!!!!!!!!!
    mEncodedMPImages = pMP_Images;
    mEncodedMPNum = mpImageNum;
    mJpegSrc = jpegSrc;
}

MP_APP2::~MP_APP2()
{
    if (NULL != mMP_Extensions)
        delete mMP_Extensions;
    mMP_Extensions = NULL;
}

bool MP_APP2::readStream(MpoInputStream* stream,bool decodeIndexIFD)
{
    XLOGV("MP_APP2::readStream():current position=0x%x",(int)stream->tell());
    int i,j;
    char buffer[8];
    mOffsetInFile = stream->tell();
    //read App2 Marker
    if (2 != stream->read(buffer, 2)) {
        XLOGE("read APP2 Marker failed");
        return false;
    }
    if (buffer[0] != APP2_MARKER[0] ||
        buffer[1] != APP2_MARKER[1]) {
        XLOGE("read APP2 failed: wrong App2 Marker");
        return false;
    }
    //read App2 length
    int length = stream->readUint16(true);//it seems length of APPn is big endiant by default
    if (length < 2 ) {
        XLOGE("invalid APP2 length: %d",length);
        return false;
    }
    XLOGV("MP_APP2::readStream:APP2 length = 0x%x",length);
    //read App2 MP format
    if (4 != stream->read(buffer, 4)) {
        XLOGE("read APP2 MP format failed");
        return false;
    }
    if (buffer[0] != APP2_MPFormat[0] ||
        buffer[1] != APP2_MPFormat[1] ||
        buffer[2] != APP2_MPFormat[2] ||
        buffer[3] != APP2_MPFormat[3]) {
        //in case multiple APP2 exits, skip some until we encouter MP format
        do {
            //skip the rest of APPn
            if (false == stream->skip(length-6)) {
                XLOGE("skip rest of APPn failed");
                return false;
            }
            //record appn offset in file
            mOffsetInFile = stream->tell();
            //read Marker
            if (2 != stream->read(buffer, 2)) {
                XLOGE("read Marker failed in MP_APP2.readStream()");
                return false;
            }

            if (buffer[0] != APP2_MARKER[0] || buffer[1] != APP2_MARKER[1]) {
                XLOGE("Error APP2 Marker!");
                stream->rewind();
                stream->skip(mOffsetInFile);
                return false;
            }

            //read APP2 length
            length = stream->readUint16(true);//it seem length of field is big endiant by default
            if (length < 2 ) {
                XLOGE("invalid APPn length: %d",length);
                return false;
            }
            //read App2 MP format
            if (4 != stream->read(buffer, 4)) {
                XLOGE("read APP2 MP format failed");
                return false;
            }


        } while (buffer[0] != APP2_MPFormat[0] ||
                 buffer[1] != APP2_MPFormat[1] ||
                 buffer[2] != APP2_MPFormat[2] ||
                 buffer[3] != APP2_MPFormat[3]);
    }

    //record the position of MP Header (MP Endian)
    mMPHeaderPos = stream->tell();
    mMP_Extensions = new MP_Extensions();
    bool ok = mMP_Extensions->readStream(stream, decodeIndexIFD);
    //skip rest of APP2
    stream->rewind();
    ok = stream->skip(mOffsetInFile + length + 2);
    XLOGV("MP_APP2::readStream:before return, position=0x%x",(int)stream->tell());
    return ok;
}

int MP_APP2::getImageCount()
{
    return mMP_Extensions->getImageCount();
}

//return image offset in file.
//If this MP_APP2 is not with the first image, return -1;
int MP_APP2::getOffsetInFile(int imageIndex)
{
    if (imageIndex < 0)
        return -1;
    if (0 == imageIndex)
        return 0;
    int offsetInMPExtension = mMP_Extensions->getOffsetInMPExtension(imageIndex);
    if (-1 == offsetInMPExtension)
        return -1;
    return mMPHeaderPos + offsetInMPExtension;
}

int MP_APP2::getImageSize(int imageIndex)
{
    return mMP_Extensions->getImageSize(imageIndex);
}

//dump recorded info into the stream.
bool MP_APP2::writeStream(MpoOutputStream* stream)
{
    if (NULL == stream) {
        XLOGE("MP_APP2:writeStream: stream is invalid!");
        return false;
    }

    //1,write App2 Marker into stream
    if (2 != stream->write((char*)APP2_MARKER, 0, 2)) {
        XLOGE("MP_APP2:writeStream: can't write App2 Maker into stream");
        return false;
    }

    //2,write App2 length
    int indexIFDLength = 0;
    int attrIFDLength = 0;

    if (mEncodedMPNum > 0) {
        //sizeof(count) + sizeof(IFD)*count 
        // + sizeof(Offset) +sizeof(MPEntry)*imageNum
        indexIFDLength = 2 + 12*4 + 4 + 16*mEncodedMPNum;
        //add internal MPO type length
        indexIFDLength += 4;
        attrIFDLength = 2 + 12*4 + 4;
    } else {
        attrIFDLength = 2 + 12*5 + 4;
    }
    //sizeof(count) + sizeof(IFD)*count + sizeof(Offset)//count = 4
    int mpExtLength = 4 + 4 + indexIFDLength + attrIFDLength;
    int appLength = 2+4+mpExtLength+16;
    bool bigEndian = true;
    if (false == stream->writeUint16(appLength,bigEndian)) {
        XLOGE("MP_APP2:writeStream: failed to write app length into stream");
        return false;
    }

    //3,write App2 MP format identifier
    if (4 != stream->write((char*)APP2_MPFormat, 0, 4)) {
        XLOGE("MP_APP2:writeStream: App2 MP format failed");
        return false;
    }

    //create MP Extension block
    MP_Extensions* mpExt = new MP_Extensions(pMPImageInfo,
                               mEncodedMPImages,mEncodedMPNum,mJpegSrc);
    if (NULL == mpExt) {
        XLOGE("MP_APP2:writeStream: create MP_Extension failed!");
        return false;
    }

    if (false == mpExt->writeStream(stream) ) {
        XLOGE("MP_APP2:writeStream: call MP_Extension->writeStream failed");
        delete mpExt;
        mpExt = NULL;
        return false;
    }
    delete mpExt;
    mpExt = NULL;

    return true;
}

//////////////////////////////////////////

/////////////////Image Data////////////////
ImageData::ImageData()
{
    mOffsetInFile = 0;
    mImageDataSize = 0;
}

bool ImageData::readStream(MpoInputStream* stream) 
{
    int i,j;
    char buffer[8];
    //because there will be App3,...,Appn behind App2,
    //we have to skip them before read compressed image data
    do {
        //record app1 offset in file
        mOffsetInFile = stream->tell();
        //read Marker
        if (2 != stream->read(buffer, 2)) {
            XLOGE("read Marker failed in ImageData.readStream()");
            return false;
        }

        if (buffer[0] != 0xFF) {
            XLOGE("read image data, invalid tag 0x%x%x, stream->tell()=0x%x",
                  buffer[0],buffer[1],stream->tell());
            return false;
        }

        if (!(buffer[1] >= 0xE1 && buffer[1] <= 0xEF)){
            XLOGD("ImageData::readStream::for exif tag which is not APPn, exit while loop");
            //for exif tag which is not APPn, exit while loop
            break;
        }

        //read APPn length
        int length = stream->readUint16(true);//it seem length of field is big endiant by default
        if (length < 2 ) {
            XLOGE("invalid APPn length: %d",length);
            return false;
        }

        //skip the rest of APPn
        if (false == stream->skip(length-2)) {
            XLOGE("skip rest of APPn failed");
            return false;
        }
    } while (true);

    /// M: add for the 0xFF FF FF E0 marker case when the SW JPEG encoder on 71/72 @{
    if (!(buffer[0] == 0xFF && buffer[1] == 0xDB)) {
        //record app0 offset in file
        mOffsetInFile = stream->tell();
        //read marker
        if (2 != stream->read(buffer, 2)) {
            XLOGE("read Marker failed in ImageData.readStream()");
            return false;
        }

        if (!(buffer[0] == 0xFF && buffer[1] == 0xE0)) {
            XLOGE("read image data, invalid tag 0x%x%x, stream->tell()=0x%x", buffer[0],buffer[1],stream->tell());
            return false;
        }

        //read app0 length
        int length = stream->readUint16(true);
        if (length < 2) {
            XLOGE("invalid app0 length: %d", length);
            return false;
        }

        //skip the rest of app0
        if (false == stream->skip(length - 2)) {
            XLOGE("skip rest of app0 failed");
            return false;
        }

        mOffsetInFile = stream->tell();
        if (2 != stream->read(buffer, 2)) {
            XLOGE("read extra marker failed in ImageData.readStream()");
            return false;
        }        
    }
    //@}

    //now, we expect DQT tag, or error is happened.
    if (buffer[0] != DQT_MARKER[0] ||
        buffer[1] != DQT_MARKER[1]) {
        XLOGE("read DQT failed: wrong DQT Marker");
        return false;
    }
    //goto the end of JPEG file
    if (false == stream->gotoEnd()) {
        XLOGE("goto end of JPEG file failed");
        return false;
    }
    //get the end position of JPEG file
    int fileEndOffset = stream->tell();
    //calculate image data length
    mImageDataSize = fileEndOffset - mOffsetInFile + 1;
    return true;
}

bool ImageData::writeStream(MpoOutputStream* stream)
{
    return true;
}

///////////////////////////////////////////

////////////////Exif Image///////////////
Exif_Image::Exif_Image()
{
    mApp1 = new APP1();
    mApp2 = new APP2();
    mImageData = new ImageData();
}

Exif_Image::Exif_Image(bool isMpo)
{
    mApp1 = new APP1();
    if (isMpo)
        mApp2 = new MP_APP2();
    else
        mApp2 = new APP2();
    mImageData = new ImageData();
}

Exif_Image::~Exif_Image()
{
    //release APP1 marker segment
    delete mApp1;
    mApp1 = NULL;
    //release APP2 marker segment
    delete mApp2;
    mApp2 = NULL;

    delete mImageData;
    mImageData = NULL;
}

bool Exif_Image::readStream(MpoInputStream* stream)
{
    bool ok = false;
    //read SOI
    char buffer[2];
    if (2 != stream->read(buffer,2)) {
        XLOGE("read Individual Image SOI failed");
        return false;
    }
    if (buffer[0] != SOI[0] ||
        buffer[1] != SOI[1]) {
        XLOGE("invalid Individual Image SOI");
        return false;
    }
    ok = readApp1(stream);
    if (ok)
        ok = readApp2(stream);
    //there may be APP3,..., APPn after APP2, we should 
    //take care of that.
    //TEMPORARYLY, we skip them in readImageData
    if (ok)
        ok = readImageData(stream);
    if (ok)
        mImageSize = mImageData->getOffsetInFile()+mImageData->getFieldLenth();
    return ok;
}

bool Exif_Image::readApp1(MpoInputStream* stream)
{
    return onReadApp1(stream);
}

bool Exif_Image::readApp2(MpoInputStream* stream)
{
    return onReadApp2(stream);
}

bool Exif_Image::onReadApp1(MpoInputStream* stream)
{
    if (NULL == mApp1 || NULL == stream)
        return false;
    return mApp1->readStream(stream);
}

bool Exif_Image::onReadApp2(MpoInputStream* stream)
{
    if (NULL == mApp2 || NULL == stream)
        return false;
    return mApp2->readStream(stream);
}

bool Exif_Image::readImageData(MpoInputStream* stream)
{
    if (NULL == mImageData || NULL == stream)
        return false;
    return mImageData->readStream(stream);
}

int Exif_Image::getApp1Pos()
{
    if (NULL == mApp1) {
        XLOGE("Exif_Image:getApp1Pos: mApp1 is NULL");
        return -1;
    }
    return mApp1->getOffsetInFile();
}
int Exif_Image::getApp1Len()
{
    if (NULL == mApp1) {
        XLOGE("Exif_Image:getApp1Len: mApp1 is NULL");
        return -1;
    }
    return mApp1->getFieldLenth();
}

int Exif_Image::getImageDataPos()
{
    if (NULL == mImageData) {
        XLOGE("Exif_Image:getImageDataPos: mImageData is NULL");
        return 0;
    }
    return mImageData->getOffsetInFile();
}
int Exif_Image::getImageDataLen()
{
    if (NULL == mImageData) {
        XLOGE("Exif_Image:getImageDataPos: mImageData is NULL");
        return 0;
    }
    return mImageData->getFieldLenth();
}

/////////////////////////////////////////

////////////////MP Image/////////////////
MP_Image::MP_Image()
:Exif_Image(true)
{
    mWidth = 0;
    mHeight = 0;
}

MP_Image::MP_Image(Exif_Image* exifImage,MPImageInfo* pMPImageInfo,
                   MP_Image** pMP_Images,int mpImageNum,const int mtkMpoType)
{
    if (NULL == exifImage || NULL == pMPImageInfo) {
        XLOGE("Invalid Exif_Image or MPImageInfo to build MP_Image");
        return;
    }

    this->pMPImageInfo = pMPImageInfo;
    mJpegSrc = exifImage;
    mEncodedMPImages = pMP_Images;
    mEncodedMPNum = mpImageNum;

    //change global variable to record MTK MPO type
    gMtkMpoType = mtkMpoType;
}

MP_Image::~MP_Image()
{

}

bool MP_Image::onReadApp2(MpoInputStream* stream)
{
    if (NULL == mApp2)
        return false;
    return mApp2->readStream(stream,0 == mImageIndex);
}

bool MP_Image::writeStream(MpoOutputStream* stream)
{
    //check whether stream is invalid
    if (NULL == stream) {
        XLOGE("MP_Image:writeStream: stream is invalid!");
        return false;
    }
    //open jpeg file as input stream
    MpoInputStream * jpegFile = NULL;//new MpoFileInputStream(pMPImageInfo->filename);
    if (SOURCE_TYPE_BUF == pMPImageInfo->sourceType) {
        if (NULL != pMPImageInfo->imageBuf && pMPImageInfo->imageSize > 0) {
            jpegFile = new MpoMemoryInputStream(
                                           pMPImageInfo->imageBuf,
                                           pMPImageInfo->imageSize);
        } else {
            XLOGE("Image buffer is NULL or image size <= 0");
        }
    } else {
        jpegFile = new MpoFileInputStream(pMPImageInfo->filename,"rb");
    }
    if (NULL == jpegFile) {
        XLOGE("MP_Image:writeStream:open jpeg file %s failed!",pMPImageInfo->filename);
        return false;
    }
    //record offset in stream
    mOffsetInFile = stream->tell();

    //1,write SOI into stream
   if (2 != stream->write((char*)SOI, 0, 2)) {
        XLOGE("MP_Image:writeStream: can't write SOI into stream");
        delete jpegFile;
        return false;
    }

    //2,write APP1 into stream
    //we create a buffer of 64K to copy content from jpeg file and dump to stream
    const int BUFFER_SIZE = 4096 * 16;
    char * buffer = new char[BUFFER_SIZE];
    if (NULL == buffer) {
        XLOGE("MP_Image:writeStream: can't allocate buffer of 4K!");
        delete jpegFile;
        return false;
    }

    //buffer allocated, check APP1 length
    int app1Pos = mJpegSrc->getApp1Pos();
    int app1Len = mJpegSrc->getApp1Len();
    if (app1Pos < 0) {
        XLOGE("MP_Image:writeStream got invalid APP1 position = 0x%x, length=%d",
              app1Pos,app1Len);
        delete buffer;
        delete jpegFile;
        return false;
    }

    //skip the SOI of src JPEG
    if (false == jpegFile->skip(2)) {
        XLOGE("skip SOI failed");
        delete buffer;
        delete jpegFile;
        return false;
    }

    while (app1Len > BUFFER_SIZE) {
        //read content from jpeg source
        if (BUFFER_SIZE != jpegFile->read(buffer,BUFFER_SIZE)) {
            XLOGE("read content of %d Bytes from src jpeg failed",BUFFER_SIZE);
            delete buffer;
            delete jpegFile;
            return false;
        }
        //write content to stream
        if (BUFFER_SIZE != stream->write(buffer, 0, BUFFER_SIZE)) {
            XLOGE("write content of %d Bytes to stream failed",BUFFER_SIZE);
            delete buffer;
            delete jpegFile;
            return false;
        }
        app1Len -= BUFFER_SIZE;
    }
    //read rest content from jpeg source
    if (app1Len != jpegFile->read(buffer,app1Len)) {
        XLOGE("read rest content of %d Bytes from src jpeg failed",app1Len);
        delete buffer;
        delete jpegFile;
        return false;
    }
    //write rest content to stream
    if (app1Len != stream->write(buffer, 0, app1Len)) {
        XLOGE("write rest content of %d Bytes to stream failed",app1Len);
        delete buffer;
        delete jpegFile;
        return false;
    }
        
    //3,create MPO APP2, and write it into the stream
    MP_APP2 * Mp_App2 = new MP_APP2(pMPImageInfo,mEncodedMPImages,mEncodedMPNum,mJpegSrc);
    if (NULL == Mp_App2) {
        XLOGE("MP_Image:writeStream: can not create a MP APP2 block");
        delete buffer;
        delete jpegFile;
        return false;
    }
    //write MP info into MPO file
    if (false == Mp_App2->writeStream(stream)) {
        XLOGE("MP_Image:writeStream: write APP2 to out put stream failed");
        delete buffer;
        delete jpegFile;
        return false;
    }
    delete Mp_App2;
    Mp_App2 = NULL;

    //4,write jpeg primary image data into the stream
    //buffer allocated, check APP1 length
    int imageDataPos = mJpegSrc->getImageDataPos();
    int imageDataLen = mJpegSrc->getImageDataLen();
    if (imageDataPos <= 0 || imageDataLen <= 0) {
        XLOGE("MP_Image:writeStream got invalid Image Data position = 0x%x, length=%d",
              imageDataPos,imageDataLen);
        delete buffer;
        delete jpegFile;
        return false;
    }

    //skip the SOI of src JPEG
    jpegFile->rewind();
    if (false == jpegFile->skip(imageDataPos)) {
        XLOGE("jump to beginning of Image Data failed");
        delete buffer;
        delete jpegFile;
        return false;
    }

    while (imageDataLen > BUFFER_SIZE) {
        //read content from jpeg source
        if (BUFFER_SIZE != jpegFile->read(buffer,BUFFER_SIZE)) {
            XLOGE("read content of %d Bytes from src jpeg failed",BUFFER_SIZE);
            delete buffer;
            delete jpegFile;
            return false;
        }
        //write content to stream
        if (BUFFER_SIZE != stream->write(buffer, 0, BUFFER_SIZE)) {
            XLOGE("write content of %d Bytes to stream failed",BUFFER_SIZE);
            delete buffer;
            delete jpegFile;
            return false;
        }
        imageDataLen -= BUFFER_SIZE;
    }
    //read rest content from jpeg source
    /// M: for the limiting case about imageDataLen = 1(File had arrived the EOF). -- ALPS01077145
    if ((imageDataLen != 1) && (imageDataLen-1 != jpegFile->read(buffer, imageDataLen-1))) {
        XLOGE("read rest content of %d Bytes from src jpeg failed",imageDataLen-1);
        delete buffer;
        delete jpegFile;
        return false;
    }
    //close source file
    delete jpegFile;
    //write rest content to stream
    /// M: for the limiting case about imageDataLen = 1(File had arrived the EOF).  -- ALPS01077145
    if ((imageDataLen !=1) && (imageDataLen-1 != stream->write(buffer, 0, imageDataLen-1))) {
        XLOGE("write rest content of %d Bytes to stream failed",imageDataLen-1);
        delete buffer;
        return false;
    }
        
    //5,write EOI into stream
    if (2 != stream->write((char*)EOI, 0, 2)) {
        XLOGE("MP_Image:writeStream: can't write EOI into stream");
        delete buffer;
        return false;
    }
    
    mImageSize = stream->tell() - mOffsetInFile;
    delete buffer;
    return true;
}

bool MP_Image::reWriteStream(MpoOutputStream* stream, Exif_Image* exifImage,
                             MP_Image** MPImages, int mpImageNum)
{
    //check whether stream is invalid
    if (NULL == stream) {
        XLOGE("MP_Image:reWriteStream: stream is invalid!");
        return false;
    }

    if (!stream->markSupported()) {
        XLOGE("stream->markSupported() returns false, MPO Entry info can not be filled!");
        return false;
    }

    //reset to marked position
    stream->reset();

    //check whether stream has been marked before
    if (stream->tell() <= 0) {
        XLOGE("It seem output stream has never been marked, MPO Entry info can not be filled!");
        return false;
    }

    if (NULL == exifImage || NULL == MPImages || mpImageNum <= 0) {
        XLOGE("Error Param found:exifImage=0x%x, MPImages=0x%x, mpImageNum=%d",
              (unsigned int)exifImage,(unsigned int)MPImages,mpImageNum);
        return false;
    }

    int firstImageApp1Length = exifImage->getApp1Len();
    int firstImageDataLength = exifImage->getImageDataLen();
    int indexIFDLength = 2 + 12*4 + 4 + 16* mpImageNum;
    //add MPO internal type length
    indexIFDLength += 4;
    int attrIFDLength = 2 + 12*4 + 4;
    int mpExtLength = 4 + 4 + indexIFDLength + attrIFDLength;
    int app2Length = 2+4+mpExtLength+16;
//    int nextMPImageOffset = 2 + firstImageApp1Length + 2/*APP2 MAKER*/ + app2Length + firstImageDataLength + 2/*EOF*/;
//    int nextMPImageOffset = 2 + firstImageApp1Length + 2/*APP2 MAKER*/ + app2Length + firstImageDataLength + 2/*EOF*/-1;
    int nextMPImageOffset = 2 + firstImageApp1Length + 2/*APP2 MAKER*/ + app2Length + firstImageDataLength -1;
    XLOGV("nextMPImageOffset=0x%x",nextMPImageOffset);

    bool bigEndian = false;

    //the following code encode MP Index IFD Value field
    for (int i=0; i< mpImageNum; i++) {
        //write MP Entry Individual Image Attribute
        int imageAttr = i == 0 ? 0x20020002 : 0x00020002;
        XLOGD("MP_Image:reWriteStream:write image %d attribute 0x%x",i,imageAttr);
        if (false == stream->writeUint32(imageAttr,bigEndian)) {
            XLOGE("write MP Entry Individual Image Attribute: %x failed",imageAttr);
            return false;
        }
        //write MP Entry Individual Image Size
        int imageSize = 0;
        if (0 == i) {
            imageSize = nextMPImageOffset;
        } else {
            imageSize = MPImages[i]->getImageSize();
        }
        XLOGD("write MP Entry Individual Image Size %d",imageSize);
        if (false == stream->writeUint32(imageSize,bigEndian)) {
            XLOGE("write MP Entry Individual Image Size %d failed",imageSize);
            return false;
        }
        //write MP Entry Individual Image Data Offset
        int imageOffset = 0;
        if (0 != i) {
            //calculate image offset cooresponding to MP header
            imageOffset = MPImages[i]->getOffsetInFile()-(2 + firstImageApp1Length + 2+2+4) ;
        }
        XLOGI("write MP Entry Individual Image Data Offset 0x%x",imageOffset);
        if (false == stream->writeUint32(imageOffset,bigEndian)) {
            XLOGE("write MP Entry Individual Image Data Offset 0x%x",imageOffset);
            return false;
        }
        //write MP Entry Dependent image 1 Entry Number
        XLOGV("write MP Entry Dependent image 1 Entry Number 0");
        if (false == stream->writeUint16(0,bigEndian)) {
            XLOGE("write MP Entry Dependent image 1 Entry Number 0 failed");
            return false;
        }
        //write MP Entry Dependent image 2 Entry Number
        XLOGV("write MP Entry Dependent image 2 Entry Number 0");
        if (false == stream->writeUint16(0,bigEndian)) {
            XLOGE("write MP Entry Dependent image 2 Entry Number 0 failed");
            return false;
        }
    }

    return true;
}

int MP_Image::getMtkMpoType() 
{
    return gOutMtkMpoType;
}

//////////////////MP_Image/////////////////////

//////////////////First_MP_Image///////////////
int First_MP_Image::getOffsetInFile(int imageIndex)
{
    return ((MP_APP2*)mApp2)->getOffsetInFile(imageIndex);
}

int First_MP_Image::getImageSize(int imageIndex)
{
    return ((MP_APP2*)mApp2)->getImageSize(imageIndex);
}

//int MP_Image::getFieldLength(int imageIndex)
//{
//    return mApp2->getFieldLength(imageIndex);
//}

int First_MP_Image::getImageCount()
{
    return ((MP_APP2*)mApp2)->getImageCount();
}


