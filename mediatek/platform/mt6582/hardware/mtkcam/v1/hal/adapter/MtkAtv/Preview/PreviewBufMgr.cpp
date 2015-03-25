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
//
#define LOG_TAG "MtkCam/PreviewBufMgr"
//
#include <inc/CamUtils.h>
using namespace android;
using namespace MtkCamUtils;
#include <adapter/inc/ImgBufProvidersManager.h>
#include <mtkcam/v1/hwscenario/HwBuffHandler.h>
#include <mtkcam/v1/hwscenario/IhwScenarioType.h>
using namespace NSHwScenario;
//
#include <mtkcam/v1/IParamsManager.h>
#include <inc/PreviewCmdQueThread.h>
#include <inc/IPreviewBufMgr.h>
using namespace NSMtkAtvCamAdapter;
//
/******************************************************************************
*
*******************************************************************************/
#include "mtkcam/Log.h"
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)[%s] "fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)[%s] "fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)[%s] "fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)[%s] "fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)[%s] "fmt, ::gettid(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, arg...)    if (cond) { MY_LOGV(arg); }
#define MY_LOGD_IF(cond, arg...)    if (cond) { MY_LOGD(arg); }
#define MY_LOGI_IF(cond, arg...)    if (cond) { MY_LOGI(arg); }
#define MY_LOGW_IF(cond, arg...)    if (cond) { MY_LOGW(arg); }
#define MY_LOGE_IF(cond, arg...)    if (cond) { MY_LOGE(arg); }

#define FUNCTION_IN                 MY_LOGD("+")
#define FUNCTION_OUT                MY_LOGD("-")
/******************************************************************************
 *
 ******************************************************************************/


namespace android {
namespace NSMtkAtvCamAdapter {
/******************************************************************************
 *
 ******************************************************************************/
class PreviewBufMgr : public IPreviewBufMgr
{

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IPreviewBufMgr Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    virtual bool dequeBuffer(int ePort, ImgBufQueNode &node);
    virtual bool enqueBuffer(ImgBufQueNode const& node);
    virtual void allocBuffer(int w, int h, const char* format, int cnt); 
    virtual void freeBuffer();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++    
public:
    PreviewBufMgr(sp<ImgBufProvidersManager> &rImgBufProvidersMgr);
    virtual ~PreviewBufMgr();
    virtual void destroyInstance();


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Private.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++     
 private:
    sp<ImgBufProvidersManager>    mspImgBufProvidersMgr;
    sp<HwBuffProvider>            mspHwBufPvdr;
};

};
};



/*******************************************************************************
*
********************************************************************************/
void 
PreviewBufMgr::
allocBuffer(int w, int h, const char *format, int cnt)
{
    for (int i = 0; i < cnt; i++)
    {
        sp<HwBuffer> buf = new HwBuffer(w, h, format); 
        mspHwBufPvdr->addBuf(buf);
    }
} 


/*******************************************************************************
*
********************************************************************************/
void 
PreviewBufMgr::
freeBuffer()
{
    mspHwBufPvdr->removeBuf();
}


/*******************************************************************************
*
********************************************************************************/
bool 
PreviewBufMgr::
dequeBuffer(int ePort, ImgBufQueNode &node)
{
    bool ret = false; 
    
    switch (ePort)
    {
        case eID_Pass1Out:
        {
            if ( mspHwBufPvdr != 0 )
            {
                sp<IImgBuf> buf;
                mspHwBufPvdr->deque(buf);
                node = ImgBufQueNode(buf, ImgBufQueNode::eSTATUS_TODO);
                node.setCookieDE(eBuf_Pass1);
                ret = true;
            }
        }
        break;
        //
        case eID_Pass2DISPO:
        {
            sp<IImgBufProvider> bufProvider =  mspImgBufProvidersMgr->getDisplayPvdr();
            if (bufProvider != 0 && bufProvider->dequeProvider(node))
            {
                node.setCookieDE(eBuf_Disp);
                ret = true;
            }
        }
        break;
        //      
        case eID_Pass2VIDO:
        {
            {
                sp<IImgBufProvider> bufProvider =  mspImgBufProvidersMgr->getRecCBPvdr();
                if (bufProvider != 0 && bufProvider->dequeProvider(node))
                {
                    node.setCookieDE(eBuf_Rec);
                    ret = true;
                    break;
                }
            }
            {
                sp<IImgBufProvider> bufProvider =  mspImgBufProvidersMgr->getPrvCBPvdr();
                if (bufProvider != 0 && bufProvider->dequeProvider(node))
                {
                    node.setCookieDE(eBuf_AP);
                    ret = true;
                    break;
                }
            }
            {
                sp<IImgBufProvider> bufProvider =  mspImgBufProvidersMgr->getFDBufPvdr();
                if (bufProvider != 0 && bufProvider->dequeProvider(node))
                {
                    node.setCookieDE(eBuf_FD);
                    ret = true;
                    break;
                }
            }
        }
        break;
        //
        default:
            MY_LOGE("unknown port!!");
        break;
    }

    return ret;
}


/*******************************************************************************
*
********************************************************************************/
bool 
PreviewBufMgr::
enqueBuffer(ImgBufQueNode const& node)
{
    // (1) set DONE tag into package
    const_cast<ImgBufQueNode*>(&node)->setStatus(ImgBufQueNode::eSTATUS_DONE);

    // (2) choose the correct "client"
    switch (node.getCookieDE())
    {
        case eBuf_Pass1:
        {
            if (mspHwBufPvdr != 0)
            {
                mspHwBufPvdr->enque(node.getImgBuf());
            }
        }
        break;
        //
        case eBuf_Disp:
        {
            sp<IImgBufProvider> bufProvider = mspImgBufProvidersMgr->getDisplayPvdr();
            if (bufProvider != 0)
            {
                bufProvider->enqueProvider(node);
            }
        }
        break;
        //
        case eBuf_AP:
        {
            sp<IImgBufProvider> bufProvider;
            {
                bufProvider = mspImgBufProvidersMgr->getPrvCBPvdr();
                if ( bufProvider != 0 )
                {
                    const_cast<ImgBufQueNode*>(&node)->setCookieDE(0); // 0 for preview
                    bufProvider->enqueProvider(node);
                }

                // If fd exists, copy to it
                bufProvider = mspImgBufProvidersMgr->getFDBufPvdr();
                ImgBufQueNode FDnode; 
                if (bufProvider != 0 && bufProvider->dequeProvider(FDnode))
                {
                    if ( FDnode.getImgBuf()->getBufSize() >= node.getImgBuf()->getBufSize())
                    {
                        memcpy(FDnode.getImgBuf()->getVirAddr(), 
                           node.getImgBuf()->getVirAddr(), 
                           node.getImgBuf()->getBufSize());
                    }
                    else 
                    {
                        MY_LOGE("fd buffer size < ap buffer size");
                        const_cast<ImgBufQueNode*>(&FDnode)->setStatus(ImgBufQueNode::eSTATUS_CANCEL);
                    }
                    //
                    bufProvider->enqueProvider(FDnode);                    
                }
            }
        }
        break;        
        //
        case eBuf_FD:
        {
            sp<IImgBufProvider> bufProvider = mspImgBufProvidersMgr->getFDBufPvdr();
            if (bufProvider != 0)
            {
                bufProvider->enqueProvider(node);
            }
        }
        break; 
        //
        case eBuf_Rec:
        {
            sp<IImgBufProvider> bufProvider = mspImgBufProvidersMgr->getRecCBPvdr();
            if (bufProvider != 0)
            {
                bufProvider->enqueProvider(node);
            }
        }
        break;
        //
        default:
            MY_LOGE("unknown port(%d)!!", node.getCookieDE());
        break;
    }
    
    return true;
}


/*******************************************************************************
*
********************************************************************************/
PreviewBufMgr::
PreviewBufMgr(sp<ImgBufProvidersManager> &rImgBufProvidersMgr)
    : mspImgBufProvidersMgr(rImgBufProvidersMgr)
    , mspHwBufPvdr(new HwBuffProvider())
{
}


/*******************************************************************************
*
********************************************************************************/
PreviewBufMgr::~PreviewBufMgr()
{
    MY_LOGD("this=%p, mspImgBufProvidersMgr=%p, mspHwBufPvdr=%p, sizeof:%d", 
             this, &mspImgBufProvidersMgr, &mspHwBufPvdr, sizeof(PreviewBufMgr));
}


/*******************************************************************************
*
********************************************************************************/
void 
PreviewBufMgr::
destroyInstance()
{
    // let mspHwBufPvdr de-allocate by itself
}


/*******************************************************************************
*
********************************************************************************/
IPreviewBufMgr*
IPreviewBufMgr::
createInstance(sp<ImgBufProvidersManager> &rImgBufProvidersMgr)
{
    return new PreviewBufMgr(rImgBufProvidersMgr);
}
