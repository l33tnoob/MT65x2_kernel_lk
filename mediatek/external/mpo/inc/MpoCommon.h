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

#ifndef MPO_COMMON_H
#define MPO_COMMON_H

#include "mpo_internal.h"
#include "mpo_type.h"
#include "MpoStream.h"

//declaration
class MP_Image;
class Exif_Image;

class MP_IFD {
public:
    MP_IFD();
    ~MP_IFD();
    inline int getCount() { return mCount;}
    inline void setCount(int count) { mCount = count;}
    inline ExifIFD_t* getMPIndexFields() { return mIFD_Ptr;}
    inline int getOffsetToNextIFD() { return mIFD_Offset;}
    //extract info from a stream and record them.
    bool readStream(MpoInputStream* stream,bool bigEndian) ;
    bool readStream(MpoInputStream* stream,bool bigEndian,int startOfHeader) ;

private:

protected:
    //count of MP Index/Attribute Fields
    int mCount;
    //MP Index/Attribute Fields pointer
    ExifIFD_t* mIFD_Ptr;
    //Offset to next IFD
    int mIFD_Offset;
    int mVersion;
    bool mBigEndian;
    MPImageInfo* pMPImageInfo;
};

class MP_Index_IFD :public MP_IFD{
public:
    MP_Index_IFD();
    MP_Index_IFD(MPImageInfo* pMPImageInfo,MP_Image** pMP_Images,
                 int mpImageNum,Exif_Image* jpegSrc,bool bigEndian);
    ~MP_Index_IFD();
    //extract info from a stream and record them.
    bool readStream(MpoInputStream* stream,bool bigEndian,int startOfHeader) ;
    //dump recorded info into the stream.
    bool writeStream(MpoOutputStream* stream,int startOfHeader) ;

    int getImageCount();
    int getOffsetInMPExtension(int imageIndex);
    int getImageSize(int imageIndex);
private:
    //record info in pIFD_t into member variable
    bool processIFD_tag(ExifIFD_t* pIFD_t);

private:
    //array of MP_Entry for each image
    MPEntry* mMPEntry;
    int mImageNum;
    int mEntryNum;
    int mImageUIDListNum;
    int mImageIdList;
    int mTotalFramNum;

    //MPO info 
    MPImageInfo* pMPImageInfo;
    //MP Images vector which have be encoded in the temp file
    MP_Image** mEncodedMPImages;
    int mEncodedMPNum;
    Exif_Image* mJpegSrc;
};

class MP_Attr_IFD :public MP_IFD{
public:
    MP_Attr_IFD();
    MP_Attr_IFD(MPImageInfo* pMPImageInfo,bool bigEndian);
    //extract info from a stream and record them.
    bool readStream(MpoInputStream* stream,bool bigEndian,int startOfHeader) ;
    //dump recorded info into the stream.
    bool writeStream(MpoOutputStream* stream, bool encodeMPVersion,
                     int imageNum,int startOfHeader);
private:
    //record info in pIFD_t into member variable
    bool processIFD_tag(ExifIFD_t* pIFD_t);

private:
    //Individual Image Number, different from MP_Index_IFD's
    int mImageNum;
    //for Panoroma
    int mPanOrientation;
    int mPanOverlap_H;
    int mPanOverlap_V;
    //for Disparity
    int mBaseViewpointNum;
    int mConvergenceAngle;
    int mBaselineLength;
    int mVerticalDivergence;
    //for Multi-Angle
    int mAxisDistance_X;
    int mAxisDistance_Y;
    int mAxisDistance_Z;
    int mYawAngle ;
    int mPitchAngle ;
    int mRollAngle ;

};

class MP_Extensions{
public:
    MP_Extensions();
    MP_Extensions(MPImageInfo* pMPImageInfo,MP_Image** pMP_Images=NULL,
                  int mpImageNum=0,Exif_Image* jpegSrc=NULL);
    ~MP_Extensions();
    inline bool getMPEndian() { return mBigEndian;}
    inline void setMPEndian(bool bigEndian) {
        mBigEndian = bigEndian;
    }

    inline int getOffsetTo1stIFD() {return mIFD_Offset;}
    inline void setOffsetTo1stIFD(int ifd_offset) {
        mIFD_Offset = ifd_offset;
    }

    //extract info from a stream and record them.
    bool readStream(MpoInputStream* stream, bool decodeIndexIFD=false) ;
    //dump recorded info into the stream.
    bool writeStream(MpoOutputStream* stream) ;

    int getImageCount();
    //return image offset in file.
    //If this MP_Extension is not with the first image, return -1;
    int getOffsetInMPExtension(int imageIndex);
    int getImageSize(int imageIndex);
private:
    //First image in MPO file
    bool mIsFirstImage;
    //MP Endian
    bool mBigEndian;
    int mMPIndexIFDCount;
    //MP Index IFD
    MP_Index_IFD * mMPIndexIFD;
    //MP Attribute IFD
    MP_Attr_IFD * mMPAttrIFD;
    //Offset to 1st IFD
    int mIFD_Offset;

    //MPO info 
    MPImageInfo* pMPImageInfo;
    //MP Images vector which have be encoded in the temp file
    MP_Image** mEncodedMPImages;
    int mEncodedMPNum;
    Exif_Image* mJpegSrc;
};

class APP {
public:
    inline int getOffsetInFile() { return mOffsetInFile;}
    inline int getFieldLenth() { return mFieldLength;}
protected:
    //offset in file
    int mOffsetInFile;
    //APP field length including Exif Marker
    int mFieldLength;
};

class APP1 :public APP {
public:
    //extract info from a stream and record them.
    bool readStream(MpoInputStream* stream) ;
    //dump recorded info into the stream.
    bool writeStream(MpoOutputStream* stream) ;
};

class APP2 :public APP {
public:
    APP2();
    virtual ~APP2();
    //extract info from a stream and record them.
    virtual bool readStream(MpoInputStream* stream, bool noUse=false) ;
    //dump recorded info into the stream.
    virtual bool writeStream(MpoOutputStream* stream) ;
};

class MP_APP2 :public APP2 {
public:
    MP_APP2();
    MP_APP2(MPImageInfo* pMPImageInfo,MP_Image** pMP_Images=NULL,
            int mpImageNum=0,Exif_Image* jpegSrc=NULL);
    virtual ~MP_APP2();
    //extract info from a stream and record them.
    bool readStream(MpoInputStream* stream, bool decodeIndexIFD=false) ;
    //dump recorded info into the stream.
    bool writeStream(MpoOutputStream* stream) ;

    //return total image count in file
    //If this MP_APP2 is not within the first image, return 0;
    int getImageCount();
    //return image offset in file.
    //If this MP_APP2 is not within the first image, return -1;
    int getOffsetInFile(int imageIndex);
    //return image size
    //If this MP_APP2 is not within the first image, return 0;
    int getImageSize(int imageIndex);
private:
    //location of MP Header int MPO file
    int mMPHeaderPos;
    MP_Extensions* mMP_Extensions;

    //MPO info 
    MPImageInfo* pMPImageInfo;
    //MP Images vector which have be encoded in the temp file
    MP_Image** mEncodedMPImages;
    int mEncodedMPNum;
    Exif_Image* mJpegSrc;
};

class ImageData {
public:
    ImageData();
    inline int getOffsetInFile() { return mOffsetInFile;}
    inline int getFieldLenth() { return mImageDataSize;}
    //extract info from a stream and record them.
    bool readStream(MpoInputStream* stream) ;
    //dump recorded info into the stream.
    bool writeStream(MpoOutputStream* stream) ;
private:
    //offset in file
    int mOffsetInFile;
    //image data size
    int mImageDataSize;
};

class Exif_Image {
public:
    Exif_Image();
    Exif_Image(bool isMpo);
    virtual ~Exif_Image();
    //extract info from a stream and record them.
    virtual bool readStream(MpoInputStream* stream) ;
    bool readApp1(MpoInputStream* stream) ;
    virtual bool onReadApp1(MpoInputStream* stream) ;
    bool readApp2(MpoInputStream* stream) ;
    virtual bool onReadApp2(MpoInputStream* stream) ;
    bool readImageData(MpoInputStream* stream);
    //dump recorded info into the stream.
    bool writeStream(MpoOutputStream* stream) ;

    int getApp1Pos();
    int getApp1Len();
    int getImageDataPos();
    int getImageDataLen();
protected:
    APP1 * mApp1;
    APP2 * mApp2;
    ImageData * mImageData;
    //offset in file
    int mOffsetInFile;
    //image size:between SOI and EOI
    int mImageSize;
    int mImageIndex;//used only for MP image
};

class MP_Image: public Exif_Image{
public:
    MP_Image();
    MP_Image(Exif_Image* exifImage,MPImageInfo* pMPImageInfo,
                       MP_Image** pMP_Images=NULL,int mpImageNum=0,
                       const int mtkMpoType=MTK_TYPE_MAV);
    ~MP_Image();
    //set the image index within MPO file in the MP_Image
    inline void setImageIndex(int imageIndex){ mImageIndex = imageIndex;}
    inline void setOffsetInFile(int offset) { mOffsetInFile = offset;}
    inline void setImageSize(int imageSize) { mImageSize = imageSize;}
    virtual bool onReadApp2(MpoInputStream* stream) ;

    //dump recorded info into the stream.
    bool writeStream(MpoOutputStream* stream) ;
    //fill MPO index Entry attributes
    bool reWriteStream(MpoOutputStream* stream, Exif_Image* exifImage, 
                       MP_Image** MPImages, int mpImageNum);
    inline int getWidth(){ return mWidth;}
    inline int getHeight(){ return mHeight;}
    //this function is devised during MPO encoder development,be careful when used in decoding

    inline int getOffsetInFile() {return mOffsetInFile;}
    inline int getImageSize() {return mImageSize;}

    int getMtkMpoType();
protected:
    //Total field length: sizeof(SOI)+imageSize+sizeof(EOI)
    int mFieldLength;
    //image width
    int mWidth;
    //image height
    int mHeight;
    //MPO info 
    MPImageInfo* pMPImageInfo;
    //JPEG image info
    Exif_Image* mJpegSrc;
    //MP Images vector which have be encoded in the temp file
    MP_Image** mEncodedMPImages;
    int mEncodedMPNum;

};

class First_MP_Image: public MP_Image {
public:
    int getOffsetInFile(int imageIndex);
//    int getFieldLength(int imageIndex);
    int getImageSize(int imageIndex);
    int getImageCount();
    
};

#endif

