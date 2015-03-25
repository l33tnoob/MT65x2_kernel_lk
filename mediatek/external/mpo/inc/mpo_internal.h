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

#ifndef MPO_INTERNAL_H
#define MPO_INTERNAL_H

// Data types
typedef unsigned char   UINT8;
typedef unsigned int    UINT32;
typedef unsigned short  UINT16;
typedef int IINT32;
typedef void            VOID;

typedef unsigned char   u8;
typedef unsigned short  u16;
typedef unsigned int    u32;


#define TAG_SUPP_LEVEL_NOT_SPECIFIED 0
#define TAG_SUPP_LEVEL_OPTIONAL      1
#define TAG_SUPP_LEVEL_RECOMMENDED   2

#define TAG_MPFVersion   0xB000
#define TAG_ImageNumber  0xB001
#define TAG_MPEntry      0xB002
#define TAG_ImageUIDList 0xB003
#define TAG_TotalFrames  0xB004

#define TAG_MPIndividualNum 0xB101
//for Panoroma
#define TAG_PanOrientation  0xB201
#define TAG_PanOverlap_H    0xB202
#define TAG_PanOverlap_V    0xB203
//for Disparity
#define TAG_BaseViewpointNum 0xB204
#define TAG_ConvergenceAngle 0xB205
#define TAG_BaselineLength   0xB206
#define TAG_VerticalDivergence 0xB207
//for Multi-Angle
#define TAG_AxisDistance_X 0xB208
#define TAG_AxisDistance_Y 0xB209
#define TAG_AxisDistance_Z 0xB20A
#define TAG_YawAngle   0xB20B
#define TAG_PitchAngle 0xB20C
#define TAG_RollAngle  0xB20D

//#define IFD_OFFSET_SIZE 4
#define MP_HEADER_LENGTH 8
#define MP_INDEX_TAG_NUM 5
#define MP_ATTR_TAG_NUM 15
#define MP_IMAGE_LOC 2
#define MP_TYPE_NUM 3
#define MP_FIRST_IMAGE 0
#define MP_OTHER_IMAGE 1

const unsigned char SOI[] = { 0xFF, 0xD8};
const unsigned char EOI[] = { 0xFF, 0xD9};
const unsigned char APP0_MARKER[] = {0XFF, 0XE0};
const unsigned char APP1_MARKER[] = {0XFF, 0XE1};
const unsigned char APP2_MARKER[] = {0XFF, 0XE2};
const unsigned char APP2_MPFormat[] = {0x4D,0x50,0x46,0x00};
const unsigned char LT_Endian[] = {0x49,0x49,0x2A,0x00};
const unsigned char BG_Endian[] = {0x4D,0x4D,0x00,0x2A};
const unsigned char ExifIdCode[] = {'E','x','i','f',0x00,0x00};
const unsigned char DQT_MARKER[] = {0XFF, 0XDB};
const unsigned char MPVersionDefault[] = {'0','1','0','0'};
const unsigned int MPVersionDefaultInt = 0x30303130;

enum MpoType {
    MultiAngle = 0,
    Disparity,
    Panorama
};

enum ifdDataType {
    IFD_DATATYPE_BYTE = 1,
    IFD_DATATYPE_ASCII,
    IFD_DATATYPE_SHORT,
    IFD_DATATYPE_LONG,
    IFD_DATATYPE_RATIONAL,
    IFD_DATATYPE_UNDEFINED = 7,
    IFD_DATATYPE_SLONG = 9,
    IFD_DATATYPE_SRATIONAL = 10
};

const int FIRST_IFD_TYPE = 1;
const int LAST_IFD_TYPE  = 10;

const int TYPE_COUNT[11] = {
    0,//nothing
    1,//Byte
    1,//Ascii
    2,//short
    4,//long
    8,//rational
    0,//reserved
    1,//undefiended
    0,//reserved
    4,//signed long
    8,//signed rational
};

typedef struct ExifIFD {
    ExifIFD(unsigned short Tag, unsigned short Type, 
            unsigned int Count, unsigned int Valoff) {
        tag = Tag;
        type = Type;
        count = Count;
        valoff = Valoff;
    }
    ExifIFD() {
        tag = 0;
        type = 0;
        count = 0;
        valoff = 0;
    };
    unsigned short tag;
    unsigned short type;
    unsigned int count;
    unsigned int valoff;
} ExifIFD_t;

const int MPO_Index_Tag_Level[MP_INDEX_TAG_NUM] = {
    //tags support level for all kinds of MPO
    TAG_SUPP_LEVEL_RECOMMENDED,  //MPFVersion = 0,
    TAG_SUPP_LEVEL_RECOMMENDED,  //ImageNumber,        //1
    TAG_SUPP_LEVEL_RECOMMENDED,  //MPEntry,            //2
    TAG_SUPP_LEVEL_NOT_SPECIFIED,//ImageUIDList,       //3
    TAG_SUPP_LEVEL_RECOMMENDED   //TotalFrames,        //4
};

const int MPO_Attr_Tag_Level[MP_TYPE_NUM][MP_IMAGE_LOC][MP_ATTR_TAG_NUM] = {
    //tags support level for Multi-Angle View MPO
    {
        //tags support level for first individual image
        {
            TAG_SUPP_LEVEL_NOT_SPECIFIED,//MPFVersion = 0,
            TAG_SUPP_LEVEL_RECOMMENDED,  //MPIndividualNum,    //1
            TAG_SUPP_LEVEL_NOT_SPECIFIED,//PanOrientation,     //2
            TAG_SUPP_LEVEL_NOT_SPECIFIED,//PanOverlap_H,       //3
            TAG_SUPP_LEVEL_NOT_SPECIFIED,//PanOverlap_V,       //4
            TAG_SUPP_LEVEL_RECOMMENDED,  //BaseViewpointNum,   //5
            TAG_SUPP_LEVEL_NOT_SPECIFIED,//ConvergenceAngle,   //6
            TAG_SUPP_LEVEL_NOT_SPECIFIED,//BaselineLength,     //7
            TAG_SUPP_LEVEL_NOT_SPECIFIED,//VerticalDivergence, //8
            TAG_SUPP_LEVEL_RECOMMENDED,  //AxisDistance_X,     //9
            TAG_SUPP_LEVEL_RECOMMENDED,  //AxisDistance_Y,     //10
            TAG_SUPP_LEVEL_RECOMMENDED,  //AxisDistance_Z,     //11
            TAG_SUPP_LEVEL_RECOMMENDED,  //YawAngle,           //12
            TAG_SUPP_LEVEL_RECOMMENDED,  //PitchAngle,         //13
            TAG_SUPP_LEVEL_RECOMMENDED,  //RollAngle           //14
        },
        //tags support level for ohter individual images
        {
            TAG_SUPP_LEVEL_RECOMMENDED,  //MPFVersion = 0,
            TAG_SUPP_LEVEL_RECOMMENDED,  //MPIndividualNum,    //1
            TAG_SUPP_LEVEL_NOT_SPECIFIED,//PanOrientation,     //2
            TAG_SUPP_LEVEL_NOT_SPECIFIED,//PanOverlap_H,       //3
            TAG_SUPP_LEVEL_NOT_SPECIFIED,//PanOverlap_V,       //4
            TAG_SUPP_LEVEL_RECOMMENDED,  //BaseViewpointNum,   //5
            TAG_SUPP_LEVEL_NOT_SPECIFIED,//ConvergenceAngle,   //6
            TAG_SUPP_LEVEL_NOT_SPECIFIED,//BaselineLength,     //7
            TAG_SUPP_LEVEL_NOT_SPECIFIED,//VerticalDivergence, //8
            TAG_SUPP_LEVEL_RECOMMENDED,  //AxisDistance_X,     //9
            TAG_SUPP_LEVEL_RECOMMENDED,  //AxisDistance_Y,     //10
            TAG_SUPP_LEVEL_RECOMMENDED,  //AxisDistance_Z,     //11
            TAG_SUPP_LEVEL_RECOMMENDED,  //YawAngle,           //12
            TAG_SUPP_LEVEL_RECOMMENDED,  //PitchAngle,         //13
            TAG_SUPP_LEVEL_RECOMMENDED,  //RollAngle           //14
        }
    },
    //tags support level for Disparity MPO
    {
        //tags support level for first individual image
        {
            TAG_SUPP_LEVEL_NOT_SPECIFIED,//MPFVersion = 0,
            TAG_SUPP_LEVEL_RECOMMENDED,  //MPIndividualNum,    //1
            TAG_SUPP_LEVEL_NOT_SPECIFIED,//PanOrientation,     //2
            TAG_SUPP_LEVEL_NOT_SPECIFIED,//PanOverlap_H,       //3
            TAG_SUPP_LEVEL_NOT_SPECIFIED,//PanOverlap_V,       //4
            TAG_SUPP_LEVEL_RECOMMENDED,  //BaseViewpointNum,   //5
            TAG_SUPP_LEVEL_RECOMMENDED,  //ConvergenceAngle,   //6
            TAG_SUPP_LEVEL_RECOMMENDED,  //BaselineLength,     //7
            TAG_SUPP_LEVEL_OPTIONAL,     //VerticalDivergence, //8
            TAG_SUPP_LEVEL_NOT_SPECIFIED,//AxisDistance_X,     //9
            TAG_SUPP_LEVEL_NOT_SPECIFIED,//AxisDistance_Y,     //10
            TAG_SUPP_LEVEL_NOT_SPECIFIED,//AxisDistance_Z,     //11
            TAG_SUPP_LEVEL_NOT_SPECIFIED,//YawAngle,           //12
            TAG_SUPP_LEVEL_NOT_SPECIFIED,//PitchAngle,         //13
            TAG_SUPP_LEVEL_NOT_SPECIFIED,//RollAngle           //14
        },
        //tags support level for ohter individual images
        {
            TAG_SUPP_LEVEL_RECOMMENDED,  //MPFVersion = 0,
            TAG_SUPP_LEVEL_RECOMMENDED,  //MPIndividualNum,    //1
            TAG_SUPP_LEVEL_NOT_SPECIFIED,//PanOrientation,     //2
            TAG_SUPP_LEVEL_NOT_SPECIFIED,//PanOverlap_H,       //3
            TAG_SUPP_LEVEL_NOT_SPECIFIED,//PanOverlap_V,       //4
            TAG_SUPP_LEVEL_RECOMMENDED,  //BaseViewpointNum,   //5
            TAG_SUPP_LEVEL_RECOMMENDED,  //ConvergenceAngle,   //6
            TAG_SUPP_LEVEL_RECOMMENDED,  //BaselineLength,     //7
            TAG_SUPP_LEVEL_OPTIONAL,     //VerticalDivergence, //8
            TAG_SUPP_LEVEL_NOT_SPECIFIED,//AxisDistance_X,     //9
            TAG_SUPP_LEVEL_NOT_SPECIFIED,//AxisDistance_Y,     //10
            TAG_SUPP_LEVEL_NOT_SPECIFIED,//AxisDistance_Z,     //11
            TAG_SUPP_LEVEL_NOT_SPECIFIED,//YawAngle,           //12
            TAG_SUPP_LEVEL_NOT_SPECIFIED,//PitchAngle,         //13
            TAG_SUPP_LEVEL_NOT_SPECIFIED,//RollAngle           //14
        }
    },
    //tags support level for Panorama MPO
    {
        //tags support level for first individual image
        {
            TAG_SUPP_LEVEL_NOT_SPECIFIED,//MPFVersion = 0,
            TAG_SUPP_LEVEL_RECOMMENDED,  //MPIndividualNum,    //1
            TAG_SUPP_LEVEL_RECOMMENDED,  //PanOrientation,     //2
            TAG_SUPP_LEVEL_OPTIONAL,     //PanOverlap_H,       //3
            TAG_SUPP_LEVEL_OPTIONAL,     //PanOverlap_V,       //4
            TAG_SUPP_LEVEL_NOT_SPECIFIED,//BaseViewpointNum,   //5
            TAG_SUPP_LEVEL_NOT_SPECIFIED,//ConvergenceAngle,   //6
            TAG_SUPP_LEVEL_NOT_SPECIFIED,//BaselineLength,     //7
            TAG_SUPP_LEVEL_NOT_SPECIFIED,//VerticalDivergence, //8
            TAG_SUPP_LEVEL_NOT_SPECIFIED,//AxisDistance_X,     //9
            TAG_SUPP_LEVEL_NOT_SPECIFIED,//AxisDistance_Y,     //10
            TAG_SUPP_LEVEL_NOT_SPECIFIED,//AxisDistance_Z,     //11
            TAG_SUPP_LEVEL_NOT_SPECIFIED,//YawAngle,           //12
            TAG_SUPP_LEVEL_NOT_SPECIFIED,//PitchAngle,         //13
            TAG_SUPP_LEVEL_NOT_SPECIFIED,//RollAngle           //14
        },
        //tags support level for ohter individual images
        {
            TAG_SUPP_LEVEL_RECOMMENDED,  //MPFVersion = 0,
            TAG_SUPP_LEVEL_RECOMMENDED,  //MPIndividualNum,    //1
            TAG_SUPP_LEVEL_RECOMMENDED,  //PanOrientation,     //2
            TAG_SUPP_LEVEL_OPTIONAL,     //PanOverlap_H,       //3
            TAG_SUPP_LEVEL_OPTIONAL,     //PanOverlap_V,       //4
            TAG_SUPP_LEVEL_NOT_SPECIFIED,//BaseViewpointNum,   //5
            TAG_SUPP_LEVEL_NOT_SPECIFIED,//ConvergenceAngle,   //6
            TAG_SUPP_LEVEL_NOT_SPECIFIED,//BaselineLength,     //7
            TAG_SUPP_LEVEL_NOT_SPECIFIED,//VerticalDivergence, //8
            TAG_SUPP_LEVEL_NOT_SPECIFIED,//AxisDistance_X,     //9
            TAG_SUPP_LEVEL_NOT_SPECIFIED,//AxisDistance_Y,     //10
            TAG_SUPP_LEVEL_NOT_SPECIFIED,//AxisDistance_Z,     //11
            TAG_SUPP_LEVEL_NOT_SPECIFIED,//YawAngle,           //12
            TAG_SUPP_LEVEL_NOT_SPECIFIED,//PitchAngle,         //13
            TAG_SUPP_LEVEL_NOT_SPECIFIED,//RollAngle           //14
        }
    }
};

const ExifIFD indexTagIFD[MP_INDEX_TAG_NUM] = {
    ExifIFD_t(0xb000,IFD_DATATYPE_UNDEFINED,4,0), //MPFVersion = 0,
    ExifIFD_t(0xb001,IFD_DATATYPE_LONG,     1,0), //ImageNumber,        //1
    ExifIFD_t(0xb002,IFD_DATATYPE_UNDEFINED,0,0), //MP_Entry,           //2
    ExifIFD_t(),                                  //ImageUIDList,       //3
    ExifIFD_t(0xb004,IFD_DATATYPE_LONG,     1,0), //TotalFrames,        //4
};

const ExifIFD attrTagIFD[MP_ATTR_TAG_NUM] = {
    ExifIFD_t(0xb000,IFD_DATATYPE_UNDEFINED,4,0), //MPFVersion = 0,
    ExifIFD_t(0xb101,IFD_DATATYPE_LONG,     1,0), //MPIndividualNum,    //1
    ExifIFD_t(),                                  //PanOrientation,     //2
    ExifIFD_t(),                                  //PanOverlap_H,       //3
    ExifIFD_t(),                                  //PanOverlap_V,       //4
    ExifIFD_t(0xb204,IFD_DATATYPE_LONG,     1,0), //BaseViewpointNum,   //5
    ExifIFD_t(0xb205,IFD_DATATYPE_SRATIONAL,1,0), //ConvergenceAngle,   //6
    ExifIFD_t(0xb206,IFD_DATATYPE_RATIONAL, 1,0), //BaselineLength,     //7
    ExifIFD_t(),                                  //VerticalDivergence, //8
    ExifIFD_t(),                                  //AxisDistance_X,     //9
    ExifIFD_t(),                                  //AxisDistance_Y,     //10
    ExifIFD_t(),                                  //AxisDistance_Z,     //11
    ExifIFD_t(),                                  //YawAngle,           //12
    ExifIFD_t(),                                  //PitchAngle,         //13
    ExifIFD_t()                                   //RollAngle           //14
};

struct AttrIFDValues {
    unsigned int values[MP_ATTR_TAG_NUM][2];
};

const int VALOFF_COUNT = 4;

typedef struct MP_Entry {
    unsigned int imageAttr;//Individual Image Attribute
    unsigned int imageSize;//Individual ImageSize
    unsigned int imageDataOffset;//Individual Iamge Data Offset
    unsigned short dependEntryNum1;//Dependent image 1 Entry Number
    unsigned short dependEntryNum2;//Dependent image 2 Entry Number
} MPEntry;


#endif
