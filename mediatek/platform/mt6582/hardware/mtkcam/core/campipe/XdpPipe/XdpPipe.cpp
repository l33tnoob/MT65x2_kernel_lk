/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#define LOG_TAG "campipe/xdp"
//
#include <mtkcam/Log.h>
#define MY_LOGV(fmt, arg...)    CAM_LOGV("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)    CAM_LOGD("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)    CAM_LOGI("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)    CAM_LOGW("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)    CAM_LOGE("[%s] "fmt, __FUNCTION__, ##arg)
#define FUNCTION_LOG_START      MY_LOGD("+");
#define FUNCTION_LOG_END        MY_LOGD("-");

//
#include <mtkcam/common.h>
#include <mtkcam/common/hw/hwstddef.h>
//
#include <mtkcam/imageio/IPipe.h>
#include <mtkcam/imageio/ispio_stddef.h>
#include <mtkcam/imageio/ispio_pipe_ports.h>
#include <mtkcam/imageio/ispio_pipe_scenario.h>

//
#include <mtkcam/drv/isp_drv.h>
#include <mtkcam/drv/imem_drv.h>
//
#include "../inc/PipeImp.h"
#include "../inc/XdpPipe.h"
#include "../inc/CampipeImgioPipeMapper.h"
//
//use MDP
#include "DpIspStream.h"

#define DP_SET_UVSTRIDE      (1)

#if 1
#define DP_ASSERT_STATUS( ret, stage, ... )
#else
#define DP_ASSERT_STATUS( ret, stage, ... ) \
    do { \
        if( (ret) < 0 ){ \
            MY_LOGE("Dp failed st(%i) at "stage, ret, ##__VA_ARGS__ ); \
            return false; \
        } \
    } while(0)
#endif

//#define register_using_ion

/*******************************************************************************
*
********************************************************************************/
namespace NSCamPipe {
////////////////////////////////////////////////////////////////////////////////
/*******************************************************************************
* 
********************************************************************************/
XdpPipe::
XdpPipe(
    char const*const szPipeName, 
    EPipeID const ePipeID, 
    ESWScenarioID const eSWScenarioID, 
    EScenarioFmt const eScenarioFmt
)
    : PipeImp(szPipeName, ePipeID, eSWScenarioID, eScenarioFmt)
    , mpStream(NULL)
    , mu4OutPortEnableFlag(0)
{
    mu4SrcPlaneNumber = 0;
    for( MUINT32 i = 0 ; i < MAX_NUM_OUTPORT_XDP ; i++ )
        mu4DstPlaneNumber[i] = 0;
}


/*******************************************************************************
* 
********************************************************************************/
MBOOL
XdpPipe::
init()
{
    FUNCTION_LOG_START;
    mpStream = new DpIspStream(DpIspStream::ISP_ZSD_STREAM);

    if ( NULL == mpStream )
    {
        return MFALSE;
    }

    mIMemDrv = IMemDrv::createInstance();

    if ( ! mIMemDrv || ! mIMemDrv->init() )
    {
        MY_LOGE("IMemDrv init fail.");
    }

    //FUNCTION_LOG_END;
    //
    return MTRUE;
}


/*******************************************************************************
* 
********************************************************************************/
MBOOL
XdpPipe::
uninit()
{
     FUNCTION_LOG_START;
     MBOOL ret = MTRUE;

     if ( NULL != mpStream )
     {
         delete mpStream;
         mpStream = NULL;
     }
     
     if ( mIMemDrv )
     {
         if ( !mIMemDrv->uninit() )
         {
             MY_LOGE("IMemDrv uninit fail.");
         }
         mIMemDrv->destroyInstance();
     }


     //FUNCTION_LOG_END;
     //
     return ret;
}


/*******************************************************************************
* 
********************************************************************************/
MBOOL
XdpPipe::
start()
{
    //FUNCTION_LOG_START

    MY_LOGD("+ (tid:%d), enable port:0x%x", ::gettid(), mu4OutPortEnableFlag); 

    MINT32 bufID;
    MUINT32 base[3];
    DP_STATUS_ENUM ret;
    
    ret = mpStream->startStream();

    DP_ASSERT_STATUS( ret, "startStream" );

    FUNCTION_LOG_END
    return  MTRUE;
}


/*******************************************************************************
* 
********************************************************************************/
MBOOL
XdpPipe::
stop()
{
    FUNCTION_LOG_START;

    DP_STATUS_ENUM ret;

    ret = mpStream->stopStream();
   
    DP_ASSERT_STATUS( ret, "stopStream" );

    FUNCTION_LOG_END
    return MTRUE;
}


/*******************************************************************************
* 
********************************************************************************/

MBOOL
XdpPipe::
enqueBuf(PortID const ePortID, QBufInfo const& rQBufInfo)
{
    //FUNCTION_LOG_START;

    //MY_LOGD("QBufInfo:(user, reserved, num)=(0x%x, %d, %d)", \
    //        rQBufInfo.u4User, rQBufInfo.u4Reserved, rQBufInfo.vBufInfo.size());

    for (std::vector<BufInfo>::const_iterator it = rQBufInfo.vBufInfo.begin() ; \
            it != rQBufInfo.vBufInfo.end(); ++it )
    {
        MY_LOGD("+ tid(%d) type(%d,%d,%d) (VA,PA,Size,ID)(0x%08x, 0x%08x, %d, %d)",
                gettid(),
                ePortID.type, ePortID.index, ePortID.inout,
                it->u4BufVA, it->u4BufPA,
                it->u4BufSize, it->i4MemID);
    }
    // 
    MUINT32 planenum = 0;
    MUINT32 size[3];
    DP_STATUS_ENUM ret;

    if ( ePortID.type == EPortType_MemoryOut )
    {
        planenum = mu4DstPlaneNumber[ ePortID.index ];
    }
    else if ( ePortID.type == EPortType_MemoryIn )
    {
        planenum = mu4SrcPlaneNumber;
    }
    else if(EPortType_MemoryOut != ePortID.type && EPortType_MemoryIn != ePortID.type)
    {
        MY_LOGE("enqueBuf only support memory in/out port type"); 
        return MFALSE;
    }

    if( planenum == 0 )
    {
        MY_LOGE("planenum(%d): should config pipe first", planenum);
    }
    // Note:: can't update config, but address
    //
    if (EPortType_MemoryOut == ePortID.type) 
    {
        MUINT32 index = ePortID.index;

        if( index >= MAX_NUM_OUTPORT_XDP )
        {
            MY_LOGE("not supported port index(%d)", index );
            return MFALSE;
        }

        std::vector<BufInfo>::const_iterator it = rQBufInfo.vBufInfo.begin() ;
        //for (std::vector<BufInfo>::iterator it = rQBufInfo.vBufInfo.begin() ; 
        //    it != rQBufInfo.vBufInfo.end(); ++it )
        {
            mDstBuf[ ePortID.index ] = *it;
            mapPhyAddr( &mDstBuf[ ePortID.index ] );

            MUINT32 va[3];
            MUINT32 mva[3];
            va[0] = mDstBuf[ ePortID.index ].u4BufVA;
            va[1] = va[0] + mDstPlaneSize[ index ].size[0];
            va[2] = va[1] + mDstPlaneSize[ index ].size[1];
            mva[0] = mDstBuf[ ePortID.index ].u4BufPA;
            mva[1] = mva[0] + mDstPlaneSize[ index ].size[0];
            mva[2] = mva[1] + mDstPlaneSize[ index ].size[1];
            MY_LOGV(" Dst(%u), va(0x%08X, 0x%08x, 0x%08x), mva(0x%08x, 0x%08x, 0x%08x), size(%u,%u,%u), planenum %u", \
                    index, \
                    va[0], va[1], va[2], \
                    mva[0], mva[1], mva[2], \
                    mDstPlaneSize[ index ].size[0], \
                    mDstPlaneSize[ index ].size[1], \
                    mDstPlaneSize[ index ].size[2], \
                    planenum );

            ret = mpStream->queueDstBuffer( index,
                                               (void**)va,
                                               mva,
                                               mDstPlaneSize[ index ].size,
                                               planenum );

            DP_ASSERT_STATUS( ret, "queueDstBuffer %i", index );
       }
    }
    else
    {
        std::vector<BufInfo>::const_iterator it = rQBufInfo.vBufInfo.begin(); 
        //for (std::vector<BufInfo>::iterator it = rQBufInfo.vBufInfo.begin() ; 
        //    it != rQBufInfo.vBufInfo.end(); ++it )
        {
            mSrcBuf = *it;
            mapPhyAddr( &mSrcBuf );

            MUINT32 va[3];
            MUINT32 mva[3];
            va[0] = mSrcBuf.u4BufVA;
            va[1] = va[0] + mSrcPlaneSize.size[0];
            va[2] = va[1] + mSrcPlaneSize.size[1];
            mva[0] = mSrcBuf.u4BufPA;
            mva[1] = mva[0] + mSrcPlaneSize.size[0];
            mva[2] = mva[1] + mSrcPlaneSize.size[1];

            MY_LOGV(" Src, va(0x%08X, 0x%08x, 0x%08x), mva(0x%08X, 0x%08x, 0x%08x), size(%u,%u,%u), planenum %u", \
                    va[0], va[1], va[2], \
                    mva[0], mva[1], mva[2], \
                    mSrcPlaneSize.size[0], \
                    mSrcPlaneSize.size[1], \
                    mSrcPlaneSize.size[2], \
                    planenum );

            ret = mpStream->queueSrcBuffer( (void**)va,
                                               mva,
                                               mSrcPlaneSize.size,
                                               planenum );
            DP_ASSERT_STATUS( ret, "queueSrcBuffer" );
        }
    }

    //FUNCTION_LOG_END;
    return  MTRUE;
}


/*******************************************************************************
* 
********************************************************************************/
MBOOL
XdpPipe::
dequeBuf(PortID const ePortID, QTimeStampBufInfo& rQBufInfo, MUINT32 const u4TimeoutMs /*= 0xFFFFFFFF*/)
{
    //FUNCTION_LOG_START;
    MBOOL ret = MTRUE; 
    
    MY_LOGD("+ tid(%d) type(%d, %d, %d, %d)", gettid(), ePortID.type, ePortID.index, ePortID.inout, u4TimeoutMs);

    if (EPortType_MemoryOut == ePortID.type) 
    {
        MUINT32 base[3];
        if ( mu4OutPortEnableFlag & ( 0x1 << (ePortID.index) ) )
        {
            mpStream->dequeueDstBuffer( ePortID.index,
                                        (void**)&base,
                                        true);
            DP_ASSERT_STATUS( ret, "dequeueDstBuffer %i", ePortID.index);

            MY_LOGV(" dst(%i): base(0x%08x, 0x%08x, 0x%08x)", \
                    ePortID.index, base[0], base[1], base[2] );

            MY_LOGD("deqDst(%i) (VA, PA, Size, ID) = (0x%08x, 0x%08x, %d, %d)", \
                        ePortID.index, \
                        mDstBuf[ePortID.index].u4BufVA, mDstBuf[ePortID.index].u4BufPA, \
                        mDstBuf[ePortID.index].u4BufSize, mDstBuf[ePortID.index].i4MemID);

            unmapPhyAddr( &mDstBuf[ePortID.index] );

            rQBufInfo.vBufInfo.push_back( mDstBuf[ePortID.index] ); 
        }
    }
    else
    {
        mpStream->dequeueSrcBuffer();

        DP_ASSERT_STATUS( ret, "dequeueSrcBuffer" );

        //dump buf info
        MY_LOGD("deqSrc(VA, PA, Size, ID) = (0x%08x, 0x%08x, %d, %d)", \
                    mSrcBuf.u4BufVA, mSrcBuf.u4BufPA, \
                    mSrcBuf.u4BufSize, mSrcBuf.i4MemID);

        unmapPhyAddr( &mSrcBuf );

        rQBufInfo.vBufInfo.push_back( mSrcBuf ); 
    }

    rQBufInfo.u4User = 0; 
    rQBufInfo.u4Reserved = 0;
    rQBufInfo.i4TimeStamp_sec = 0;
    rQBufInfo.i4TimeStamp_us = 0; 

    //FUNCTION_LOG_END;
    return  ret;
}


/*******************************************************************************
* 
********************************************************************************/
MBOOL
XdpPipe::
configPipe(vector<PortInfo const*>const& vInPorts, vector<PortInfo const*>const& vOutPorts, MBOOL concurrency)
{
    //FUNCTION_LOG_START;
    MBOOL ret = MTRUE; 

    MY_LOGD("+ %d in / %d out", vInPorts.size(), vOutPorts.size());
    // 
    if (0 == vInPorts.size() 
        || 0 == vOutPorts.size() 
        || vOutPorts.size() > MAX_NUM_OUTPORT_XDP ) 
    {
        MY_LOGE("Port config error");
        return false; 
    }
    //
    if (EPortType_MemoryIn != vInPorts.at(0)->type) 
    {
        MY_LOGE("The IN port type should be EPortType_MemoryIn type"); 
        return false; 
    }
    //
    for (MUINT32 i = 0; i < vOutPorts.size(); i++) 
    {
        if (EPortType_MemoryOut != vOutPorts.at(i)->type) 
        {
            MY_LOGE("The OUT port type should be EPortType_MemoryOut");
            return false; 
        }
    }  

    DP_STATUS_ENUM dp_ret;
    //
    // (1). MemoryIn Port ( 1 in ) 
    MemoryInPortInfo const* const pMemoryInPort = reinterpret_cast<MemoryInPortInfo const*> (vInPorts.at(0)); 

    MY_LOGD("In: (fmt,w,h)(0x%x,%d,%d) stride(%d,%d,%d) crop(%d,%d,%d,%d)",
            pMemoryInPort->eImgFmt, pMemoryInPort->u4ImgWidth, pMemoryInPort->u4ImgHeight, 
            pMemoryInPort->u4Stride[0],  pMemoryInPort->u4Stride[1],  pMemoryInPort->u4Stride[2],
            pMemoryInPort->rCrop.x, pMemoryInPort->rCrop.y,
            pMemoryInPort->rCrop.w, pMemoryInPort->rCrop.h); 
    //
    DpColorFormat srcFormat;
    MUINT32 srcStride[3];

    ret = mapDpFormat( pMemoryInPort->eImgFmt,
                       pMemoryInPort->u4ImgHeight, pMemoryInPort->u4Stride,
                       srcFormat, mu4SrcPlaneNumber, srcStride, mSrcPlaneSize.size );

    MY_LOGV(" Src config fmt 0x%x, p %i, wxh %ux%u, strdie %u, %u, %u ", \
                                     srcFormat, \
                                     mu4SrcPlaneNumber, \
                                     pMemoryInPort->u4ImgWidth, \
                                     pMemoryInPort->u4ImgHeight, \
                                     srcStride[0], srcStride[1], srcStride[2] );
#if DP_SET_UVSTRIDE
    dp_ret = mpStream->setSrcConfig( pMemoryInPort->u4ImgWidth,
                                     pMemoryInPort->u4ImgHeight,
                                     srcStride[0],
                                     srcStride[1],
                                     srcFormat);
#else
    dp_ret = mpStream->setSrcConfig( srcFormat, 
                                     pMemoryInPort->u4ImgWidth,
                                     pMemoryInPort->u4ImgHeight,
                                     srcStride[0]);
#endif

    DP_ASSERT_STATUS( dp_ret, "setSrcConfig fmt 0x%x, wxh %ux%u stirde %u", \
                                srcFormat, \
                                pMemoryInPort->u4ImgWidth, \
                                pMemoryInPort->u4ImgHeight, \
                                srcStride[0]);

    MY_LOGV(" Src crop X(%i, %i) Y(%i, %i) wxh %ix%i ", \
                                   pMemoryInPort->rCrop.x, 0, \
                                   pMemoryInPort->rCrop.y, 0, \
                                   pMemoryInPort->rCrop.w, \
                                   pMemoryInPort->rCrop.h);

    dp_ret = mpStream->setSrcCrop( pMemoryInPort->rCrop.x, 0,
                                   pMemoryInPort->rCrop.y, 0,
                                   pMemoryInPort->rCrop.w,
                                   pMemoryInPort->rCrop.h);

    DP_ASSERT_STATUS( dp_ret, "setSrcCrop x/y/w/h %d/%d/%d/%d", \
                                pMemoryInPort->rCrop.x, \
                                pMemoryInPort->rCrop.y, \
                                pMemoryInPort->rCrop.w, \
                                pMemoryInPort->rCrop.h );
    // (2). MemoryOut Port (up to MAX_NUM_OUTPORT_XDP out)
    for ( MUINT32 i = 0 ; i < vOutPorts.size(); i++ )
    {
        MemoryOutPortInfo const* const memOutPort = reinterpret_cast<MemoryOutPortInfo const*> (vOutPorts.at(i));
        MY_LOGD("Out %d: (fmt,w,h)(0x%x,%d,%d) stride(%d,%d,%d) r(%d) f(%d)", \
                memOutPort->index, \
                memOutPort->eImgFmt, memOutPort->u4ImgWidth, memOutPort->u4ImgHeight, \
                memOutPort->u4Stride[0],  memOutPort->u4Stride[1],  memOutPort->u4Stride[2],\
                memOutPort->u4Rotation, memOutPort->u4Flip);

        DpColorFormat dstFormat;
        MUINT32 dstStride[3];

        ret = mapDpFormat( memOutPort->eImgFmt,
                    memOutPort->u4ImgHeight, memOutPort->u4Stride,
                    dstFormat, mu4DstPlaneNumber[memOutPort->index],
                    dstStride, mDstPlaneSize[memOutPort->index].size );

        MY_LOGV(" Dst(%i) config fmt 0x%x, p %i, wxh %ux%u, strdie %u, %u, %u ", \
                                     memOutPort->index, \
                                     dstFormat, \
                                     mu4DstPlaneNumber[memOutPort->index], \
                                     memOutPort->u4ImgWidth, memOutPort->u4ImgHeight, \
                                     dstStride[0], dstStride[1], dstStride[2]);
#if DP_SET_UVSTRIDE
        dp_ret = mpStream->setDstConfig( memOutPort->index, 
                                         memOutPort->u4ImgWidth,
                                         memOutPort->u4ImgHeight,
                                         dstStride[0],
                                         dstStride[1],
                                         dstFormat);
#else
        dp_ret = mpStream->setDstConfig( memOutPort->index, 
                                         dstFormat, 
                                         memOutPort->u4ImgWidth,
                                         memOutPort->u4ImgHeight,
                                         dstStride[0]);
#endif

        DP_ASSERT_STATUS( dp_ret, "setDstConfig" );

        if( memOutPort->index == 0 )
        {
            // port 0 do not suppot rotation and flip
            dp_ret = mpStream->setRotation( 0, 0 );
            DP_ASSERT_STATUS( dp_ret, "port 0 setRotation 0" );

            dp_ret = mpStream->setFlipStatus( 0, false );
            DP_ASSERT_STATUS( dp_ret, "port 0 setFlipStatus 0" );

            mu4OutPortEnableFlag |= 0x1;  
        }
        else if( memOutPort->index == 1 )
        {
            // port 1 support rotation and flip
            //MY_LOGD("Out 1 rot(%u), flip(%u) ", memOutPort->u4Rotation, memOutPort->u4Flip);

            dp_ret = mpStream->setRotation( 1, memOutPort->u4Rotation );
            DP_ASSERT_STATUS( dp_ret, "port 1 setRotation %u", memOutPort->u4Rotation );

            dp_ret = mpStream->setFlipStatus( 1, memOutPort->u4Flip );
            DP_ASSERT_STATUS( dp_ret, "port 1 setFlipStatus %u", memOutPort->u4Flip );

            mu4OutPortEnableFlag |= 0x2;  
        }

    }
    //FUNCTION_LOG_END;
    return  ret;
}

/*******************************************************************************
* 
********************************************************************************/
MBOOL
XdpPipe::
queryPipeProperty(vector<PortProperty > &vInPorts, vector<PortProperty > &vOutPorts)
{
    FUNCTION_LOG_START;

    PortID rMemInPortID(EPortType_MemoryIn, 0, 0); 
    PortID rMemOutDispPortID(EPortType_MemoryOut, 0, 1); 
    PortID rMemOutVdoPortID(EPortType_MemoryOut, 1, 1);   
    //
#warning [TODO]
    PortProperty rMemInPortProperty(rMemInPortID, eImgFmt_UNKNOWN, MFALSE, MFALSE); 
    PortProperty rMemOutDispProperty(rMemOutDispPortID, eImgFmt_BAYER10|eImgFmt_YUY2, MFALSE, MFALSE); 
    PortProperty rMemOutVdoProperty(rMemOutVdoPortID, eImgFmt_YUY2, MFALSE, MFALSE); 

    vInPorts.clear(); 
    vOutPorts.clear(); 

    vInPorts.push_back(rMemInPortProperty);     
    vOutPorts.push_back(rMemOutDispProperty);  
    vOutPorts.push_back(rMemOutVdoProperty); 

    dumpPipeProperty(vInPorts, vOutPorts); 

    FUNCTION_LOG_END;
    return  MTRUE;
}

/*******************************************************************************
* 
********************************************************************************/
MBOOL
XdpPipe::
sendCommand(MINT32 cmd, MINT32 arg1, MINT32 arg2, MINT32 arg3)
{
 
   FUNCTION_LOG_START;
   FUNCTION_LOG_END;
   return MTRUE; 
}

MBOOL XdpPipe::mapPhyAddr( BufInfo* buf )
{
    IMEM_BUF_INFO imembuf( buf->u4BufSize, buf->i4MemID, buf->u4BufVA, \
                           buf->u4BufPA, buf->i4BufSecu, buf->i4BufCohe );

    if( mIMemDrv->mapPhyAddr( &imembuf ) < 0 )
    {
        MY_LOGE("mapPhyAddr failed.");
        return MFALSE;
    }

    buf->u4BufPA = imembuf.phyAddr;

    return MTRUE;
}


MBOOL XdpPipe::unmapPhyAddr( BufInfo* buf )
{
    IMEM_BUF_INFO imembuf( buf->u4BufSize, buf->i4MemID, buf->u4BufVA, \
                           buf->u4BufPA, buf->i4BufSecu, buf->i4BufCohe );

    if( mIMemDrv->unmapPhyAddr( &imembuf ) < 0 )
    {
        MY_LOGE("unmapPhyAddr failed.");
        return MFALSE;
    }
    return MTRUE;
}
//
MBOOL XdpPipe::mapDpFormat(EImageFormat fmt, MUINT32 h, const MUINT32 stride_p[3], 
            DpColorFormat& dp_fmt, MUINT32& planenum, MUINT32 stride[3], MUINT32 size[3])
{
    MUINT32 bit_pixel;
    //unsigned int uv_h_shift = 0;
    MUINT32 uv_v_shift = 0;
    //stride already define in bytes
    switch( fmt )
    {
#define FMTCASE( fmt, dpfmt, bpp, n_plane, v_shift ) \
        case fmt: \
            dp_fmt = dpfmt; \
            bit_pixel = bpp; \
            planenum = n_plane; \
            uv_v_shift = v_shift; \
            break
        FMTCASE( eImgFmt_YUY2   , DP_COLOR_YUYV, 16, 1, 0 );
        FMTCASE( eImgFmt_UYVY   , DP_COLOR_UYVY, 16, 1, 0 );
        FMTCASE( eImgFmt_YVYU   , DP_COLOR_YVYU, 16, 1, 0 );
        FMTCASE( eImgFmt_VYUY   , DP_COLOR_VYUY, 16, 1, 0 );
        FMTCASE( eImgFmt_NV16   , DP_COLOR_NV16,  8, 2, 0 );
        FMTCASE( eImgFmt_NV61   , DP_COLOR_NV61,  8, 2, 0 );
        FMTCASE( eImgFmt_NV21   , DP_COLOR_NV21,  8, 2, 1 );
        FMTCASE( eImgFmt_NV12   , DP_COLOR_NV12,  8, 2, 1 );
        FMTCASE( eImgFmt_YV16   , DP_COLOR_YV16,  8, 3, 0 );
        FMTCASE( eImgFmt_I422   , DP_COLOR_I422,  8, 3, 0 );
        FMTCASE( eImgFmt_YV12   , DP_COLOR_YV12,  8, 3, 1 );
        FMTCASE( eImgFmt_I420   , DP_COLOR_I420,  8, 3, 1 );
        FMTCASE( eImgFmt_Y800   , DP_COLOR_GREY,  8, 1, 0 );
        FMTCASE( eImgFmt_RGB565 , DP_COLOR_RGB565, 16, 1, 0);
        FMTCASE( eImgFmt_RGB888 , DP_COLOR_RGB888, 24, 1, 0);
        FMTCASE( eImgFmt_ARGB888, DP_COLOR_ARGB8888, 32, 1, 0);
        
#undef FMTCASE
        case eImgFmt_NV21_BLK: // 420, 2 plane (Y), (VU)
        case eImgFmt_BAYER8:
        case eImgFmt_BAYER10:
        case eImgFmt_BAYER12:
        case eImgFmt_JPEG:
        case eImgFmt_NV12_BLK: // 420, 2 plane (Y), (VU)
            MY_LOGW("fmt(0x%x) not support in DP", fmt);
        default:
            MY_LOGW("fmt (0x%x) is not supported");
            return false;
            break;
    }

    stride[0] = (stride_p[0] * bit_pixel) >> 3;
    stride[1] = planenum > 1 ? (stride_p[1] * bit_pixel) >> 3 : 0;
    stride[2] = planenum > 2 ? (stride_p[2] * bit_pixel) >> 3 : 0;

    size[0] = stride[0] * h;
    size[1] = planenum > 1 ? stride[1] * (h >> uv_v_shift) : 0;
    size[2] = planenum > 2 ? stride[2] * (h >> uv_v_shift) : 0;

    MY_LOGV("fmt(0x%x) -> (0x%x), stride(%i, %i, %i) -> (%i, %i, %i), size(%i,%i,%i)", \
           fmt, dp_fmt, \
           stride_p[0], stride_p[1], stride_p[2], \
           stride[0], stride[1], stride[2], \
           size[0], size[1], size[2]); 
    return true;
}

////////////////////////////////////////////////////////////////////////////////
};  //namespace NSCamPipe
