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

#ifndef _MTK_HAL_CAMADAPTER_INC_IMGBUFPROVIDERSMANAGER_H_
#define _MTK_HAL_CAMADAPTER_INC_IMGBUFPROVIDERSMANAGER_H_
//


namespace android {
/*******************************************************************************
*   Image Buffer Providers Manager
*******************************************************************************/
class ImgBufProvidersManager : public RefBase
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Instantiation.
                                    ImgBufProvidersManager()
                                        : mRWLockPvdrs()
                                        , mvPvdrs()
                                    {
                                        mvPvdrs.setCapacity(IImgBufProvider::eID_TOTAL_NUM);
                                        for (uint_t i = 0; i < mvPvdrs.capacity(); i++)
                                        {
                                            mvPvdrs.push_back(NULL);
                                        }
                                    }

public:     ////                    Attributes.
    //
    inline  size_t                  getProvidersSize() const
                                    {
                                        return mvPvdrs.size();
                                    }
    //
    inline  void                    setProvider(int32_t const i4PvdrId, sp<IImgBufProvider>const& rpPvdr)
                                    {
                                        RWLock::AutoWLock _l(mRWLockPvdrs);
                                        mvPvdrs.editItemAt(i4PvdrId) = rpPvdr;
                                    }
    //
    inline sp<IImgBufProvider>      getProvider(int32_t const i4PvdrId) const
                                    {
                                        RWLock::AutoRLock _l(mRWLockPvdrs);
                                        return  mvPvdrs.itemAt(i4PvdrId);
                                    }

    inline String8                  queryFormat(int32_t const i4PvdrId) const
                                    {
                                        String8 s8Format = String8::empty();
                                        ImgBufQueNode node;
                                        sp<IImgBufProvider> pImgBufPvdr = getProvider(i4PvdrId);

                                        if  ( pImgBufPvdr != 0 && pImgBufPvdr->queryProvider(node) ) {
                                            s8Format = node.getImgBuf()->getImgFormat();
                                        }
                                        return  s8Format;
                                    }

public:     ////                    Operations.

    //  get [Display] Image Buffer Provider
    sp<IImgBufProvider>             getDisplayPvdr() const  { return getProvider(IImgBufProvider::eID_DISPLAY); }
    //  get [Record Callback] Image Buffer Provider
    sp<IImgBufProvider> const       getRecCBPvdr() const    { return getProvider(IImgBufProvider::eID_REC_CB); }
    //  get [Preview Callback] Image Buffer Provider
    sp<IImgBufProvider> const       getPrvCBPvdr() const    { return getProvider(IImgBufProvider::eID_PRV_CB); }
    //  get [Face Detection] Image Buffer Provider
    sp<IImgBufProvider> const       getFDBufPvdr() const    { return getProvider(IImgBufProvider::eID_FD); }
    //  get [genaric preview feature] Image Buffer Provider    
    sp<IImgBufProvider> const       getGenericBufPvdr()const{ return getProvider(IImgBufProvider::eID_GENERIC); }
    

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data Members.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:    ////                    Data Members.
    mutable RWLock                  mRWLockPvdrs;
    Vector< sp<IImgBufProvider> >   mvPvdrs;

};


}; // namespace android
#endif  //_MTK_HAL_CAMADAPTER_INC_IMGBUFPROVIDERSMANAGER_H_

