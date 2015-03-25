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

#ifndef I_HW_SCENARIO_H
#define I_HW_SCENARIO_H
//
#include <vector>
using namespace std;
//
class IhwScenario {

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    struct Rect_t {
        MUINT32 w;
        MUINT32 h;
        MUINT32 x;
        MUINT32 y;
        MUINT32 floatX;
        MUINT32 floatY;
        Rect_t (MUINT32 _w = 0, MUINT32 _h = 0, MUINT32 _x = 0, MUINT32 _y = 0, MUINT32 _floatX = 0, MUINT32 _floatY = 0)
               :w(_w), h(_h), x(_x), y(_y), floatX(_floatX), floatY(_floatY){}
    };

    struct PortImgInfo
    {
        EHwBufIdx          ePortIdx;  
        const char*        sFormat;
        MUINT32            u4Width;
        MUINT32            u4Height;
        MUINT32            u4Stride[3];
        Rect_t             crop;
        EImageRotation     eRotate;
        EImageFlip         eFlip;
     
        //
        PortImgInfo(
            EHwBufIdx        _idx = eID_Unknown,
            const char*      _Format = "UNKNOWN",
            MUINT32          _Width = 0,
            MUINT32          _Height = 0,
            EImageRotation   _eRotate = eImgRot_0,
            EImageFlip       _eFlip = eImgFlip_OFF
        )
        : ePortIdx(_idx)
        , sFormat(_Format)
        , u4Width(_Width)
        , u4Height(_Height)
        , crop(0, 0, _Width, _Height)        
        , eRotate(_eRotate)
        , eFlip(_eFlip)
        {
            memset(u4Stride, 0 , sizeof(u4Stride));
        }
    };   


    struct PortBufInfo
    {
        EHwBufIdx      ePortIndex;
        MUINT32        virtAddr; 
        MUINT32        phyAddr; 
        MUINT32        bufSize;
        MINT32         memID;
        MINT32         bufSecu;
        MINT32         bufCohe;
        //
        PortBufInfo(
                  EHwBufIdx _ePortIndex = eID_Unknown, 
                  MUINT32 _virtAddr = 0,           
                  MUINT32 _phyAddr = 0, 
                  MUINT32 _bufSize = 0,
                  MINT32  _memID = -1,
                  MINT32  _bufSecu = 0,
                  MINT32  _bufCohe = 0
        )
        : ePortIndex(_ePortIndex)
        , virtAddr(_virtAddr)
        , phyAddr(_phyAddr)
        , memID(_memID)
        , bufSize(_bufSize)
        , bufSecu(_bufSecu)
        , bufCohe(_bufCohe)
        {}
    };


    struct PortQTBufInfo
    {
        EHwBufIdx      ePortIndex;
        QTimeStampBufInfo bufInfo;

        PortQTBufInfo(EHwBufIdx _ePortIndex)
        : ePortIndex(_ePortIndex)
        , bufInfo()
        {}
    };

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    virtual             ~IhwScenario(){};
    virtual MVOID       destroyInstance() = 0;
    static IhwScenario* createInstance(EhwMode const& mode, 
                                       halSensorType_e const & type, 
                                       halSensorDev_e const &dev,
                                       ERawPxlID const &bitorder);
 
    virtual MBOOL       init() = 0;
    virtual MBOOL       uninit() = 0;
    virtual MBOOL       start() = 0;
    virtual MBOOL       stop() = 0;
    virtual MVOID       wait(EWaitType rType) = 0;
    
    virtual MBOOL       deque(EHwBufIdx port, vector<PortQTBufInfo> *pBufIn ) = 0;
    virtual MBOOL       enque(vector<PortBufInfo> *pBufIn = NULL, vector<PortBufInfo> *pBufOut = NULL) = 0;
    virtual MBOOL       enque(vector<IhwScenario::PortQTBufInfo> const &in) = 0;
    virtual MBOOL       replaceQue(vector<PortBufInfo> *pBufOld, vector<PortBufInfo> *pBufNew) = 0;
    
    virtual MBOOL       setConfig(vector<PortImgInfo> *pImgIn) = 0;

    virtual MVOID       getHwValidSize(MUINT32 &width, MUINT32 &height) = 0;
};


/*******************************************************************************
*
********************************************************************************/

#endif
