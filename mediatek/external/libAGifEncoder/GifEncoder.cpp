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

#include <cutils/xlog.h>
#include "allheaders.h" // Leptonica library
#include "GifEncoder.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "GifEncoder"


GifEncoder* GifEncoder::createEncoder(
        WriteFunc writeFunc,
        void *userData,
        int width,
        int height,
        unsigned int flag,
        int colorBpp) {
    GifEncoder *gifEncoder = new GifEncoder;

    XLOGV("createEncoder(func=0x%x, , %d, %d, 0x%x, %d)",
            writeFunc, width, height, flag, colorBpp);

    if (writeFunc == NULL)
        return NULL;
    if (width <= 0 || 0xffff < width ||
            height <= 0 || 0xffff < height)
        return NULL;
    if (colorBpp <= 0 || GIF_ENCODER_MAX_COLOR_BPP < colorBpp)
        return NULL;

    // data may start to be written once we open the gif
    // so we must initialize the write callback first
    gifEncoder->m_writeFunc = writeFunc;
    gifEncoder->m_userData = userData;
    
    gifEncoder->m_gifFile = EGifOpen(gifEncoder, &eGifOutputFunc, NULL);
    if (gifEncoder->m_gifFile == NULL) {
        XLOGD("EGifOpen() failed");
        delete gifEncoder;
        return NULL;
    }

    if (flag & ANIMATION)
        EGifSetGifVersion(gifEncoder->m_gifFile, true);

    gifEncoder->m_width = width;
    gifEncoder->m_height = height;
    gifEncoder->m_colorBpp = colorBpp;
    
    int gifErrorCode = EGifPutScreenDesc(
            gifEncoder->m_gifFile, width, height, colorBpp, 0, NULL);
    if (gifErrorCode != GIF_OK) {
        XLOGD("EGifPutScreenDesc() returns %d, (w,h,bpp) = (%d, %d, %d)",
                gifErrorCode, width, height, colorBpp);
        delete gifEncoder;
        return NULL;
    }

    XLOGV("createEncoder() returns 0x%x", gifEncoder);

    return gifEncoder;
}


void GifEncoder::destroyEncoder(GifEncoder *encoder) {
    XLOGV("destroyEncoder(0x%x)", encoder);
    delete encoder;
}


int GifEncoder::eGifOutputFunc(GifFileType *gifFile, const GifByteType *bytes, int size) {
    GifEncoder *encoder = static_cast<GifEncoder*>(gifFile->UserData);
    int bytesWritten = encoder->m_writeFunc(encoder->m_userData, (char*)bytes, size);
    return bytesWritten;
}


bool GifEncoder::addLoopInfo(int nLoops) {
    int gifErrorCode = GIF_OK;

    static const char *NetscapeIdAuth = "NETSCAPE2.0";
    gifErrorCode = EGifPutExtensionFirst(m_gifFile, 0xff, 11, NetscapeIdAuth);

    if (gifErrorCode == GIF_OK) {
        unsigned char appData[3];
        appData[0] = 0x1;
        appData[1] = nLoops & 0xff;
        appData[2] = (nLoops >> 8) & 0xff;
        gifErrorCode = EGifPutExtensionLast(m_gifFile, 0xff, 3, appData);
    }

    return (gifErrorCode == GIF_OK);
}


bool GifEncoder::appendImage(const char *imageInRGB888, int postDelayInMs) {
    int gifErrorCode = GIF_OK;

    // Write Graphic Control Extension
    if (postDelayInMs > 0) {
        int delayInCS = postDelayInMs / 10;
        unsigned char controlExtension[4];

        controlExtension[0] = 0x0;
        controlExtension[1] = delayInCS & 0xff;
        controlExtension[2] = (delayInCS >> 8) & 0xff;
        controlExtension[3] = 0x0;

        gifErrorCode = EGifPutExtension(m_gifFile, 0xf9, 4, controlExtension);
        if (gifErrorCode != GIF_OK)
            return false;
    }

    // Write Table-Based Image
    const int imageSize = m_width * m_height;
    GifByteType *colorIndices = new GifByteType[imageSize];

    ColorMapObject *colorMap = NULL;

    do {
        const int colorMapSize = (1 << m_colorBpp);
        colorMap = GifMakeMapObject(colorMapSize, NULL);
        if (colorMap == NULL)
            break;
            
        gifErrorCode = quantizeColorTable(
                imageInRGB888, colorMap, colorIndices);
        if (gifErrorCode != GIF_OK)
            break;

        gifErrorCode = EGifPutImageDesc(m_gifFile, 0, 0, m_width, m_height, 0, colorMap);
        if (gifErrorCode != GIF_OK)
            break;

        GifPixelType *line = colorIndices;
        for (int y = 0; y < m_height; y++) {
            gifErrorCode = EGifPutLine(m_gifFile, line, m_width);
            if (gifErrorCode != GIF_OK)
                break;
            line += m_width;
        }
    } while (0);

    delete [] colorIndices;

    if (colorMap!= NULL)
        GifFreeMapObject(colorMap);

    if (gifErrorCode != GIF_OK) {
        XLOGD("appendImage() errorCode = %d", gifErrorCode);
    }

    return (gifErrorCode == GIF_OK);
}


bool GifEncoder::close() {
    if (m_gifFile != NULL) {
        int gifErrorCode = EGifCloseFile(m_gifFile);
        if (gifErrorCode != GIF_OK) {
            XLOGD("EGifCloseFile() returns %d", gifErrorCode);
            return false;
        }
        m_gifFile = NULL;
    }

    return true;
}


#if defined(LIBLEPT_MAJOR_VERSION)

int GifEncoder::quantizeColorTableAdv(
        const char *imageInRGB888,
        ColorMapObject *colorMap,
        GifByteType *indices) {
    int gifErrorCode = GIF_ERROR;

    PIX *samplePix = pixCreate(m_width, m_height, 32);
    if (samplePix == NULL)
        return GIF_ERROR;

    { // Convert image to samplePix
        const unsigned char *pixelSrc =
                reinterpret_cast<const unsigned char*>(imageInRGB888);
        l_uint32 *samplePixLine = pixGetData(samplePix);
        const int wordsPerLine = pixGetWpl(samplePix);
        for (int y = m_height - 1; y >= 0; y--) {
            l_uint32 *samplePixel = samplePixLine;
            for (int x = m_width - 1; x >= 0; x--) {
                // *samplePixel = (pixelSrc[0] << 24) | (pixelSrc[1] << 16) | (pixelSrc[2] << 8) | 0xff;
                // ARGB -> RGBA
                *samplePixel =   (pixelSrc[2] << 24) | (pixelSrc[1] << 16) | (pixelSrc[0] << 8) | 0xff ;
                pixelSrc += 4;
                samplePixel++;
            }

            samplePixLine += wordsPerLine;
        }
    }

    const int colorMapSize = (1 << m_colorBpp);
    l_int32 ditherFlag = (m_ditherEnabled ? 1 : 0);
    PIX* quantizedPix = pixMedianCutQuantGeneral(samplePix, ditherFlag, 0, colorMapSize, 6, 0, 1);
    pixDestroy(&samplePix);

    if (quantizedPix == NULL) {
        XLOGD("pixMedianCutQuantGeneral() failed");
        return GIF_ERROR;
    }

    do {
        // Refer to pixColorizeGray() to retrieve color map
        PIXCMAP *pmap = pixGetColormap(quantizedPix);
        if (pmap == NULL)
            break;
        
        l_uint32 *rgbTable = NULL;
        l_int32 nColors;
        l_int32 result = pixcmapToRGBTable(pmap, &rgbTable, &nColors);
        if (result != 0 || rgbTable == NULL)
            break;

        int i;
        for (i = 0; i < nColors && i < colorMapSize; i++) {
            l_int32 r, g, b;
            extractRGBValues(rgbTable[i], &r, &g, &b);
            colorMap->Colors[i].Red = r;
            colorMap->Colors[i].Green = g;
            colorMap->Colors[i].Blue = b;
        }
        // If produced color map < required color map, fill residuals
        for ( ; i < colorMapSize; i++) {
            colorMap->Colors[i].Red = 0;
            colorMap->Colors[i].Green = 0;
            colorMap->Colors[i].Blue = 0;
        }
        
        FREE(rgbTable);

        // Copy back result indices
        l_uint32 *pixIndicesLine = pixGetData(quantizedPix);
        const int wordsPerLine = pixGetWpl(quantizedPix);
        for (int y = m_height - 1; y >= 0; y--) {
            l_uint32 *pixIndices4_p = pixIndicesLine;
            for (int xw = wordsPerLine - 2; xw >= 0; xw--) {
                l_uint32 pixIndices4 = *pixIndices4_p;
                indices[0] = (pixIndices4 >> 24);
                indices[1] = (pixIndices4 << 8 >> 24);
                indices[2] = (pixIndices4 << 16 >> 24);
                indices[3] = (pixIndices4 << 24 >> 24);
                
                indices += 4;
                pixIndices4_p++;
            }
            
            int remain = m_width - (wordsPerLine - 1) * 4;
            char *byte = reinterpret_cast<char*>(pixIndices4_p); // last word
            if (remain >= 1)
                indices[0] = byte[3];
            if (remain >= 2)
                indices[1] = byte[2];
            if (remain >= 3)
                indices[2] = byte[1];
            if (remain >= 4)
                indices[3] = byte[0];
            indices += remain;

            pixIndicesLine += wordsPerLine;
        }

        gifErrorCode = GIF_OK;
    } while (0);

    pixDestroy(&quantizedPix);
    
    return gifErrorCode;
}

#else

int GifEncoder::quantizeColorTableAdv(
        const char *imageInRGB888,
        ColorMapObject *colorMap,
        GifByteType *indices) {
    return GIF_ERROR;
}

#endif

int GifEncoder::quantizeColorTable(
        const char *imageInRGB888,
        ColorMapObject *colorMap,
        GifByteType *indices) {
        
#if defined(LIBLEPT_MAJOR_VERSION)

    return quantizeColorTableAdv(imageInRGB888, colorMap, indices);

#else
        
    int gifErrorCode = GIF_OK;

    const int nPixels = m_width * m_height;
    GifByteType *rChannel, *gChannel, *bChannel;
        
    rChannel = new GifByteType[nPixels];
    gChannel = new GifByteType[nPixels];
    bChannel = new GifByteType[nPixels];

    // prepare RGB channel data to quantization
    const unsigned char *pixelOfImage = reinterpret_cast<const unsigned char*>(imageInRGB888);
    for (int i = 0; i < nPixels; i++) {
        rChannel[i] = pixelOfImage[0];
        gChannel[i] = pixelOfImage[1];
        bChannel[i] = pixelOfImage[2];
        pixelOfImage += 3;
    }

    int colorMapSize = (1 << m_colorBpp);
    gifErrorCode = QuantizeBuffer(
            m_width, m_height,
            &colorMapSize,
            rChannel, gChannel, bChannel,
            indices,
            colorMap->Colors);

    delete [] rChannel;
    delete [] gChannel;
    delete [] bChannel;

    return gifErrorCode;
    
#endif
}

