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

#ifndef __GIF_ENCODER_H__
#define __GIF_ENCODER_H__

#include "gif_lib.h"


/**
 * Maximum color depth(bits-per-pixel) supported by GIF.
 */
#define GIF_ENCODER_MAX_COLOR_BPP 8


/**
 * A simple encoder interface so that we can encode a video to GIF easily.
 */
class GifEncoder {
public:
    /**
     * User-provided write function for encoded data.
     *
     * @return the number of bytes written
     */
    typedef int (*WriteFunc)(void *userData, const char *bytes, int size);

    /**
     * Flag used for createdEncoder()
     */
    enum CreateFlag {
        /**
         * Create a gif image supports animation.
         */
        ANIMATION = 0x1,
    };

    /**
     * Loop constant for addLoopInfo()
     */
    enum LoopConst {
        LOOP_INFINITE = 0
    };
    
private:
    int m_colorBpp;
    int m_width;
    int m_height;
    bool m_ditherEnabled;
    GifFileType *m_gifFile;

    WriteFunc m_writeFunc;
    void *m_userData;

    /**
     * Constructor of GifEncoder.
     * For user please call createEncoder instead.
     */
    GifEncoder() :
            m_colorBpp(GIF_ENCODER_MAX_COLOR_BPP),
            m_width(0),
            m_height(0),
            m_ditherEnabled(true),
            m_gifFile(NULL),
            m_writeFunc(NULL),
            m_userData(NULL) {
    }

    /**
     * Output hook of egif library.
     * This function will direct the writting to user's callback.
     */
    static int eGifOutputFunc(GifFileType *gifFile, const GifByteType *bytes, int size);

    int quantizeColorTableAdv(
        const char *imageInRGB888,
        ColorMapObject *colorMap,
        GifByteType *indices);

    int quantizeColorTable(
        const char *imageInRGB888,
        ColorMapObject *colorMap,
        GifByteType *indices);

public:
    
    /**
     * To create an GIF encoder.
     *
     * @param writeFunc All encoded data will be directed into this write function
     *                  User can direct the write function to file write or memory
     *                  write. Note that this function will be invoked right away
     *                  to write GIF header in createEncoder().
     * @param userData  Will be passed into the writeFunc
     * @param width     Image width
     * @param height    Image height
     * @param colorBpp  Color depth of GIF in bpp, controls the color map size.
     *                  At most 8, which implies 256 entries of color map.
     * @return An encoder instance. NULL if failed to create.
     */
    static GifEncoder* createEncoder(
            WriteFunc writeFunc,
            void *userData,
            int width,
            int height,
            unsigned int flag = 0,
            int colorBpp = GIF_ENCODER_MAX_COLOR_BPP);

    /**
     * Enable dithering or not. Should be set before appendImage().
     *
     * @param enabled True to enable dithering
     */
    void setDitherEnabled(bool enabled) {
        m_ditherEnabled = enabled;
    }

    /**
     * Destroy the GIF encoder. All encoders created from createEncoder() should be
     * destroyed by this function to release resource.
     *
     * @param encoder   The encoder object to be deleted.
     */
    static void destroyEncoder(GifEncoder *encoder);

    /**
     * Add loop information into the GIF file. This will add Netscape application
     * extension to the GIF. Suggest to add this info at the begin of the GIF file.
     *
     * @param LOOP_INFINITE or the number of loops.
     * @return True if successfully added.
     */
    bool addLoopInfo(int nLoops);

    /**
     * Append image to the GIF.
     *
     * @param imageInRGB888 Image in RGB888 format: R8 G8 B8 R8 G8 B8 ...
     * @param postDelayInMs Delay time to next image. That is, the duration from
     *                      this frame is RENDERED to next frame to be rendered.
     *                      The granularity is 10 ms.
     * @return True if successfully appended.
     */
    bool appendImage(const char *imageInRGB888, int postDelayInMs = 0);

    /**
     * Write trailer of GIF and close the GIF encoder.
     *
     * @return True if closed.
     */
    bool close();

    ~GifEncoder() {
        close();
    }
};

#endif /* __GIF_ENCODER_H__ */

