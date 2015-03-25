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

#ifndef __AGIF_ENCODER_H__
#define __AGIF_ENCODER_H__

#include <SkBitmap.h>
#include <SkStream.h>
#include <SkTemplates.h>
#include <SkUtils.h>

#include <cutils/properties.h>
#include <cutils/xlog.h>


#include "GifEncoder.h"



/**
 * A simple encoder interface so that we can encode a video to GIF easily.
 */
class AGifEncoder {

    


public:
    AGifEncoder() ;
    virtual ~AGifEncoder() ;    
    bool setWidth(unsigned int width);// {fWidth = width; return true ;};
    bool setHeight(unsigned int height) ;//{fHeight = height; return true ;};
    bool setFrameDuration(unsigned int time);// {fPostTime = time; return true ;};
    
    
    int width() { return fWidth; }
    int height() { return fHeight; }
    int duration() { return fPostTime; }
    int getGifTotalFrameCount() { return fEncodeFrame ; }


    bool setEncodeStream(SkWStream* stream);//{fStream = stream ; return true ;}
    
    bool setEncodeFile(const char file[]);  //{fStream = stream ; return true ;}
    /**
     * Append image to the GIF.
     *
     * @param imageInRGB888 Image in RGB888 format: R8 G8 B8 R8 G8 B8 ...
     * @param postDelayInMs Delay time to next image. That is, the duration from
     *                      this frame is RENDERED to next frame to be rendered.
     *                      The granularity is 10 ms.
     * @return True if successfully appended.
     */
     
    //bool encodeBitmap(const SkBitmap&) ;
    bool encodeBitmap(unsigned char* src_addr, SkWStream* stream) ;
    
    bool encodeBuffer(unsigned char *src_addr);
    

    /**
     * Write trailer of GIF and close the GIF encoder.
     *
     * @return True if closed.
     */
    bool closeGif();
    
    int write_fun(void *data, const char* buffer, int size) ;

    //~GifEncoder() {
    //    close();
    //}

private:
    SkWStream* fGifStream ;
    unsigned int fWidth;
    unsigned int fHeight;
    unsigned int fPostTime ;
    unsigned int fEncodeFrame ;
    unsigned int fCloseGif ;
    unsigned int fWriteSize ;
    
    bool m_ditherEnabled;
    
    unsigned int fEncodeTarget ;
    

    void *m_userData;
    
    GifEncoder *encoder ;

    /**
     * Constructor of GifEncoder.
     * For user please call createEncoder instead.
     */


    
};

#endif /* __GIF_ENCODER_H__ */

