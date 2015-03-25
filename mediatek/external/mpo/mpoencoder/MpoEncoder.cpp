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
#include <cutils/xlog.h>

#include "MpoEncoder.h"

using namespace std;

#undef LOG_TAG
#define LOG_TAG "MPO_Encoder"

const char *TEMP_MPO = "/mnt/sdcard/tempMpo";

MpoEncoder::MpoEncoder()
{
    mMPImageInfos = NULL;
    mFileNum = 0;

}

//set jpeg source file names and file number
//return true if parameters seem to be OK
bool MpoEncoder::setJpegSources(int fileType, MPImageInfo* pMPImageInfo, int fileNum)
{
    mMPImageInfos = NULL;
    mFileNum = 0;

    mFileType = fileType;

    if (NULL == pMPImageInfo || fileNum <= 0 ) {
        XLOGE("jpeg sources is null of file number < 0");
        return false;
    }

    for (int i=0; i< fileNum; i++) {
        if (NULL == pMPImageInfo[i].filename && (
            NULL == pMPImageInfo[i].imageBuf ||
            pMPImageInfo[i].imageSize <= 0)) {
            XLOGE("when set jpeg sources, jpeg source is null");
            return false;
        }
    }
    mMPImageInfos = pMPImageInfo;
    mFileNum = fileNum;
    return true;
}

MpoEncoder::MpoEncoder(int fileType, MPImageInfo* pMPImageInfo,int fileNum)
{
    mMPImageInfos = NULL;
    mFileNum = 0;

    mFileType = fileType;

    if (NULL == pMPImageInfo || fileNum <= 0 ) {
        XLOGE("when construct MpoEncoder, jpeg sources is null of file number < 0");
        return;
    }

    for (int i=0; i< fileNum; i++) {
        if (NULL == pMPImageInfo[i].filename && (
            NULL == pMPImageInfo[i].imageBuf ||
            pMPImageInfo[i].imageSize <= 0)) {
            XLOGE("when construct MpoEncoder, jpeg source is null");
            return;
        }
    }

    mMPImageInfos = pMPImageInfo;
    mFileNum = fileNum;
}

int MpoEncoder::getBufferSize()
{
	int size = 0;
	if (NULL == mMPImageInfos || mFileNum <= 0) {
		//XLOGE("jpeg source is null or file number < 0");
		return -1;
	}

	for (int i=0; i<mFileNum; i++) {
		//open mpo input stream
		if (SOURCE_TYPE_BUF == mMPImageInfos[i].sourceType) {
			if (NULL != mMPImageInfos[i].imageBuf && mMPImageInfos[i].imageSize > 0) {
				size += mMPImageInfos[i].imageSize;
				//XLOGV("MpoEncode::getBufferSize: size=%d", size);
			} else {
				//XLOGE("Image buffer is NULL or image size <= 0");
				return -1;
			}
		} else {
			//XLOGE("Image sourceType is not SOURCR_TYPE_BUF");
			return -1;
		}
	}

	return size;
}

void MpoEncoder::releaseImageObjects(Exif_Image*** pExif_Images,MP_Image*** pMPImages, int count)
{
    if (NULL != pExif_Images) {
        if (NULL != *pExif_Images) {
            for(int i=0; i<count; i++) {
                if (NULL != (*pExif_Images)[i])
                    delete (*pExif_Images)[i];
            }
            delete (*pExif_Images);
            *pExif_Images = NULL;
        }
    }
    if (NULL != pMPImages) {
        if (NULL != *pMPImages) {
            for(int i=0; i<count; i++) {
                if (NULL != (*pMPImages)[i])
                    delete (*pMPImages)[i];
            }
            delete (*pMPImages);
            *pMPImages = NULL;
        }
    }
}

bool MpoEncoder::encode(const char * outputMpo, const int mtkMpoType)
{
    return encodeToFile(outputMpo, mtkMpoType);
}

bool MpoEncoder::encodeToMemory(const unsigned char * data, const int mtkMpoType)
{
    return encode_internal(NULL, data, mtkMpoType);
}

bool MpoEncoder::encode_internal(const char * outputMpo, const unsigned char * data, const int mtkMpoType)
{
    if (NULL == mMPImageInfos || mFileNum <= 0 ) {
        XLOGE("jpeg sources is null or file number < 0");
        return false;
    }

    //init MP image info..
    for (int j=0; j< mFileNum; j++) {
        mMPImageInfos[j].MPIndividualNum = j+1;
        mMPImageInfos[j].BaseViewpointNum = 1;
        mMPImageInfos[j].ConvergenceAngel[0] = 0xFFFFFFFF;
        mMPImageInfos[j].ConvergenceAngel[1] = 0xFFFFFFFF;
        mMPImageInfos[j].BaselineLength[0] = 0xFFFFFFFF;
        mMPImageInfos[j].BaselineLength[1] = 0xFFFFFFFF;
        mMPImageInfos[j].type = TYPE_Disparity;
    }

    //allocate and init
    //all the images contained in MPO file;
    Exif_Image** mExif_Images = new Exif_Image*[mFileNum];
    if (NULL == mExif_Images) return false;
    for (int i=0; i<mFileNum; i++) {
        mExif_Images[i] = NULL;
    }
    MP_Image** MPImages = new MP_Image*[mFileNum];
    if (NULL == MPImages) return false;
    for (int i=0; i<mFileNum; i++) {
        MPImages[i] = NULL;
    }

    //decode all JPEG files for preparation
    MpoInputStream * mpoInputStream;
    bool decodeFailed = false;
    /// M: [ALPS01415834] length must be init.
    int length = 0;
    for (int i=0; i<mFileNum; i++) {
        //open mpo input stream
        if (SOURCE_TYPE_BUF == mMPImageInfos[i].sourceType) {
            if (NULL != mMPImageInfos[i].imageBuf && mMPImageInfos[i].imageSize > 0) {
                mpoInputStream = new MpoMemoryInputStream(
                                               mMPImageInfos[i].imageBuf,
                                               mMPImageInfos[i].imageSize);
                length += mMPImageInfos[i].imageSize;
            } else {
                XLOGE("Image buffer is NULL or image size <= 0");
            }
        } else {
            mpoInputStream = new MpoFileInputStream(mMPImageInfos[i].filename,"rb");
            XLOGD("MpoEncoder: encode() mpoInputStream = new MpoFileInputStream()");
        }
        //parse image info
        if (NULL != mpoInputStream) {
            mExif_Images[i] = new Exif_Image();
            if (false == mExif_Images[i]->readStream(mpoInputStream)) {
                XLOGE("Decode Jpeg stream %d failed",i);
                decodeFailed = true;
            }
            delete mpoInputStream;
            mpoInputStream = NULL;
        } else {
            XLOGE("Can not open mpo input stream!");
            decodeFailed = true;
        }
        if (decodeFailed)
            break;
    }
    //return false if Exif Image is not correctly decoded
    if (decodeFailed) {
        releaseImageObjects(&mExif_Images, &MPImages, mFileNum);
        return false;
    }

    bool encodeFailed = false;
    MpoOutputStream *finalOutputStream; 

    //open target mpo file
    MpoOutputStream *finalMpoMemoryOutputStream = new MpoMemoryOutputStream(data, length, false); //yue
    finalOutputStream = finalMpoMemoryOutputStream; //yue

    //The most important task, write other image location 
    //info and MPO attribute info into first image
    MPImages[0] = new MP_Image(mExif_Images[0], mMPImageInfos, NULL, mFileNum, mtkMpoType);
    if (NULL != MPImages[0]) {
        if (false == MPImages[0]->writeStream(finalOutputStream)) { //yue
            XLOGE("MpoEncoder encode jpeg image %d failed!",0);
            encodeFailed = true;
        }
    } else {
        XLOGE("MpoEncoder->encode new MPImages[%d] failed!",0);
        releaseImageObjects(&mExif_Images, &MPImages, mFileNum);
        delete finalOutputStream; 
        return false;
    }

    //write 2~n JPEG file into final mpo file
    for (int i=1; i<mFileNum; i++) {
        MPImages[i] = new MP_Image(mExif_Images[i],mMPImageInfos+i);
        if (NULL != MPImages[i]) {
            if (false == MPImages[i]->writeStream(finalOutputStream)) { 
                XLOGE("MpoEncoder encode jpeg image %d failed!",i);
                encodeFailed = true;
                break;
            }
        } else {
          XLOGE("MpoEncoder->encode new MPImages[%d] failed!",i);
          encodeFailed = true;
          break;
        }

    }

    //rewrite MPO index info
    if (!encodeFailed) {
        if (false == MPImages[0]->reWriteStream(finalOutputStream,  //yue
                                                mExif_Images[0], MPImages, mFileNum)) {
            XLOGE("MpoEncoder encode jpeg image %d failed!",0);
            encodeFailed = true;
        }
    }

    releaseImageObjects(&mExif_Images, &MPImages, mFileNum);

    //flush final stream before dump
    finalOutputStream->flush();           
    delete finalOutputStream; 

    return true;
}

bool MpoEncoder::encodeToFile(const char * outputMpo, const int mtkMpoType)
{
    if (NULL == mMPImageInfos || mFileNum <= 0 ) {
        XLOGE("jpeg sources is null or file number < 0");
        return false;
    }

    //init MP image info..
    for (int j=0; j< mFileNum; j++) {
        mMPImageInfos[j].MPIndividualNum = j+1;
        mMPImageInfos[j].BaseViewpointNum = 1;
        mMPImageInfos[j].ConvergenceAngel[0] = 0xFFFFFFFF;
        mMPImageInfos[j].ConvergenceAngel[1] = 0xFFFFFFFF;
        mMPImageInfos[j].BaselineLength[0] = 0xFFFFFFFF;
        mMPImageInfos[j].BaselineLength[1] = 0xFFFFFFFF;
        mMPImageInfos[j].type = TYPE_Disparity;
    }

    //allocate and init
    //all the images contained in MPO file;
    Exif_Image** mExif_Images = new Exif_Image*[mFileNum];
    if (NULL == mExif_Images) return false;
    for (int i=0; i<mFileNum; i++) {
        mExif_Images[i] = NULL;
    }
    MP_Image** MPImages = new MP_Image*[mFileNum];
    if (NULL == MPImages) return false;
    for (int i=0; i<mFileNum; i++) {
        MPImages[i] = NULL;
    }

    //decode all JPEG files for preparation
    MpoInputStream * mpoInputStream;
    bool decodeFailed = false;
    for (int i=0; i<mFileNum; i++) {
        //open mpo input stream
        if (SOURCE_TYPE_BUF == mMPImageInfos[i].sourceType) {
            if (NULL != mMPImageInfos[i].imageBuf && mMPImageInfos[i].imageSize > 0) {
                mpoInputStream = new MpoMemoryInputStream(
                                               mMPImageInfos[i].imageBuf,
                                               mMPImageInfos[i].imageSize);
            } else {
                XLOGE("Image buffer is NULL or image size <= 0");
            }
        } else {
            mpoInputStream = new MpoFileInputStream(mMPImageInfos[i].filename,"rb");
        }
        //parse image info
        if (NULL != mpoInputStream) {
            mExif_Images[i] = new Exif_Image();
            if (false == mExif_Images[i]->readStream(mpoInputStream)) {
                XLOGE("Decode Jpeg stream %d failed",i);
                decodeFailed = true;
            }
            delete mpoInputStream;
            mpoInputStream = NULL;
        } else {
            XLOGE("Can not open mpo input stream!");
            decodeFailed = true;
        }
        if (decodeFailed)
            break;
    }
    //return false if Exif Image is not correctly decoded
    if (decodeFailed) {
        releaseImageObjects(&mExif_Images, &MPImages, mFileNum);
        return false;
    }

    bool encodeFailed = false;

    //open target mpo file
    MpoOutputStream *finalMpoOutputStream = new MpoFileOutputStream(outputMpo,"wb");
    MpoOutputStream *finalMpoBufferedOutputStream = 
                new MpoBufferedOutputStream(finalMpoOutputStream, 4096 * 16);
    if (NULL == finalMpoOutputStream || NULL == finalMpoBufferedOutputStream) {
        XLOGE("encode:can't open writable file %s !",outputMpo);
        releaseImageObjects(&mExif_Images, &MPImages, mFileNum);
        if (NULL != finalMpoOutputStream) delete finalMpoOutputStream;
        if (NULL != finalMpoBufferedOutputStream) delete finalMpoBufferedOutputStream;
        return false;
    }

    //The most important task, write other image location 
    //info and MPO attribute info into first image
    MPImages[0] = new MP_Image(mExif_Images[0], mMPImageInfos, NULL, mFileNum, mtkMpoType);
    if (NULL != MPImages[0]) {
        if (false == MPImages[0]->writeStream(finalMpoBufferedOutputStream)) {
            XLOGE("MpoEncoder encode jpeg image %d failed!",0);
            encodeFailed = true;
        }
    } else {
        XLOGE("MpoEncoder->encode new MPImages[%d] failed!",0);
        releaseImageObjects(&mExif_Images, &MPImages, mFileNum);
        delete finalMpoBufferedOutputStream;
        delete finalMpoOutputStream;
        return false;
    }

    //write 2~n JPEG file into final mpo file
    for (int i=1; i<mFileNum; i++) {
        MPImages[i] = new MP_Image(mExif_Images[i],mMPImageInfos+i);
        if (NULL != MPImages[i]) {
            if (false == MPImages[i]->writeStream(finalMpoBufferedOutputStream)) {
                XLOGE("MpoEncoder encode jpeg image %d failed!",i);
                encodeFailed = true;
                break;
            }
        } else {
          XLOGE("MpoEncoder->encode new MPImages[%d] failed!",i);
          encodeFailed = true;
          break;
        }
    }

    //rewrite MPO index info
    if (!encodeFailed) {
        if (false == MPImages[0]->reWriteStream(finalMpoBufferedOutputStream, 
                                                mExif_Images[0], MPImages, mFileNum)) {
            XLOGE("MpoEncoder encode jpeg image %d failed!",0);
            encodeFailed = true;
        }
    }

    releaseImageObjects(&mExif_Images, &MPImages, mFileNum);

    //flush final stream before dump
    finalMpoBufferedOutputStream->flush();
    delete finalMpoBufferedOutputStream;
    //close file stream
    delete finalMpoOutputStream;

    // we should clear the final MPO file in case of failure!!!!!!!!
    if (encodeFailed) {
        XLOGE("Error happened when dump 2~n images into output MPO file");
        remove(outputMpo);
        return false;
    }

    return true;
}


MpoEncoder::~MpoEncoder()
{
    
}



