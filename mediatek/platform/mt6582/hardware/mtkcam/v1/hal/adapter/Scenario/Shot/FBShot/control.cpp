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

#include "Facebeauty.h"

#define MY_LOGV(fmt, arg...)    CAM_LOGV("[%s] \n"fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)    CAM_LOGD("[%s] \n"fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)    CAM_LOGI("[%s] \n"fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)    CAM_LOGW("[%s] \n"fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)    CAM_LOGE("[%s] \n"fmt, __FUNCTION__, ##arg)

//#define Debug_Mode

#ifdef Debug_Mode
#include <stdio.h>

int count=0;

static bool
saveBufToFile(char const*const fname, MUINT8 *const buf, MUINT32 const size)
{
    int nw, cnt = 0;
    uint32_t written = 0;

    MY_LOGD("(name, buf, size) = (%s, %x, %d)", fname, buf, size);
    MY_LOGD("opening file [%s]\n", fname);
    int fd = ::open(fname, O_RDWR | O_CREAT | O_TRUNC, 0);
    if (fd < 0) {
        MY_LOGE("failed to create file [%s]: %s", fname, ::strerror(errno));
        return false;
    }

    MY_LOGD("writing %d bytes to file [%s]\n", size, fname);
    while (written < size) {
        nw = ::write(fd,
                     buf + written,
                     size - written);
        if (nw < 0) {
            MY_LOGE("failed to write to file [%s]: %s", fname, ::strerror(errno));
            break;
        }
        written += nw;
        cnt++;
    }
    MY_LOGD("done writing %d bytes to file [%s] in %d passes\n", size, fname, cnt);
    ::close(fd);
    return true;
}
#endif

/*******************************************************************************
*
*******************************************************************************/
MBOOL
Mhal_facebeauty::
InitialAlgorithm(MUINT32 srcWidth, MUINT32 srcHeight, MINT32 gBlurLevel, MINT32 FBTargetColor)
{
    MINT32 err = 0;
    MY_LOGD("[InitialAlgorithm] In srcWidth %d srcHeight %d",srcWidth,srcHeight);
    MTKPipeFaceBeautyEnvInfo FaceBeautyEnvInfo;
    MTKPipeFaceBeautyTuningPara FaceBeautyTuningInfo;
    CPTLog(Event_FBShot_InitialAlgorithm, CPTFlagStart);
    //::memset(&FaceBeautyEnvInfo,0,sizeof(MTKPipeFaceBeautyEnvInfo));
    mStep1Width = (srcWidth >> 1) & 0xFFFFFFE0; /* UV stride, which is half of Y stride, has to be 16 aligned also */
    mStep1Height = (srcHeight >> 1) & 0xFFFFFFF0;
    FaceBeautyEnvInfo.Step2SrcImgWidth = mDSWidth;
    FaceBeautyEnvInfo.Step2SrcImgHeight = mDSHeight;
    FaceBeautyEnvInfo.SrcImgWidth = srcWidth;
    FaceBeautyEnvInfo.SrcImgHeight = srcHeight;
    FaceBeautyEnvInfo.Step1SrcImgWidth = mStep1Width;
    FaceBeautyEnvInfo.Step1SrcImgHeight = mStep1Height;
    FaceBeautyEnvInfo.FDWidth = FBFDWidth;
    FaceBeautyEnvInfo.FDHeight = FBFDHeight;
    FaceBeautyEnvInfo.WorkingBufAddr = mpWorkingBuferr.virtAddr;
    FaceBeautyEnvInfo.WorkingBufSize = FBWorkingBufferSize;
    FaceBeautyEnvInfo.SrcImgFormat = MTKPIPEFACEBEAUTY_IMAGE_YUV422;
    FaceBeautyEnvInfo.STEP1_ENABLE = true;

    FaceBeautyEnvInfo.pTuningPara = &FaceBeautyTuningInfo;
    FaceBeautyEnvInfo.pTuningPara->SmoothLevel = mSmoothLevel;
    FaceBeautyEnvInfo.pTuningPara->ContrastLevel = mContrastLevel;
    FaceBeautyEnvInfo.pTuningPara->BrightLevel = mBrightLevel;
    FaceBeautyEnvInfo.pTuningPara->RuddyLevel = mRuddyLevel;
    FaceBeautyEnvInfo.pTuningPara->WarpLevel = mWarpLevel;
    FaceBeautyEnvInfo.pTuningPara->WarpFaceNum = gWarpFaceNum;
	  FaceBeautyEnvInfo.pTuningPara->MinFaceRatio = gMinFaceRatio;
    FaceBeautyEnvInfo.pTuningPara->AlignTH1 = -10000;
    FaceBeautyEnvInfo.pTuningPara->AlignTH2 = -5;
    
    err = mpFb->mHalFacebeautifyInit(&FaceBeautyEnvInfo);
    MY_LOGD("[InitialAlgorithm] algorithm initail done");
    CPTLog(Event_FBShot_InitialAlgorithm, CPTFlagEnd);
    MY_LOGD("[InitialAlgorithm] Out");
    if(err)
        return  MFALSE;
    else 
    return  MTRUE;
}

/*******************************************************************************
*
*******************************************************************************/

MBOOL
Mhal_facebeauty::
ImgProcess(IMEM_BUF_INFO Srcbufinfo, MUINT32 srcWidth, MUINT32 srcHeight, EImageFormat srctype, IMEM_BUF_INFO Desbufinfo, MUINT32 desWidth, MUINT32 desHeight, EImageFormat destype) const
{
    MY_LOGD("[Resize] srcAdr 0x%x srcWidth %d srcHeight %d desAdr 0x%x desWidth %d desHeight %d ",(MUINT32)Srcbufinfo.virtAddr,srcWidth,srcHeight,(MUINT32)Desbufinfo.virtAddr,desWidth,desHeight);

    ImgBufInfo rSrcImgInfo;
    rSrcImgInfo.u4ImgWidth = srcWidth;
    rSrcImgInfo.u4ImgHeight = srcHeight;
    rSrcImgInfo.eImgFmt = srctype;
    rSrcImgInfo.u4Stride[0] = srcWidth;
    rSrcImgInfo.u4Stride[1] = srcWidth >> 1;
    rSrcImgInfo.u4Stride[2] = srcWidth >> 1;
    rSrcImgInfo.u4BufSize = Srcbufinfo.size;
    rSrcImgInfo.u4BufVA = Srcbufinfo.virtAddr;
    rSrcImgInfo.u4BufPA = Srcbufinfo.phyAddr;
    rSrcImgInfo.i4MemID = Srcbufinfo.memID;
    
    mpIMemDrv->cacheFlushAll();
    
    NSCamShot::ISImager *mpISImager = NSCamShot::ISImager::createInstance(rSrcImgInfo);
    if (mpISImager == NULL)
    {
        MY_LOGE("Null ISImager Obj \n");
        return MFALSE;
    }

    //MUINT32 u4Stride[3] = {desWidth, desWidth >> 1, desWidth >> 1};


    BufInfo rBufInfo(Desbufinfo.size, Desbufinfo.virtAddr, Desbufinfo.phyAddr, Desbufinfo.memID);
    //
    mpISImager->setTargetBufInfo(rBufInfo);
    //
    //mpISImager->setStrideAlign(u4Stride);
    //mpISImager->setFormat(destype, u4Stride);
    mpISImager->setFormat(destype);
    //
    mpISImager->setRotation(0);
    //
    mpISImager->setFlip(0);
    //
    mpISImager->setResize(desWidth, desHeight);
    //
    mpISImager->setEncodeParam(1, 90);
    //
    mpISImager->setROI(Rect(0, 0, srcWidth, srcHeight));
    //
    mpISImager->execute();
    //Sava Test
    #ifdef Debug_Mode
    //if(count==0)
    {
       MY_LOGD("Save resize file");
       char szFileName[100];
       ::sprintf(szFileName, "/sdcard/imgprc_%d_%d_%d_%d_%d.yuv", (int)srctype, (int)destype,srcWidth,desWidth,count);
       saveBufToFile(szFileName, (MUINT8*)Desbufinfo.virtAddr, (desWidth*desHeight*2));
       MY_LOGD("Save resize file done");
    }
    count++;
    #endif
    MY_LOGD("[Resize] Out");
    return  MTRUE;
}

/*******************************************************************************
*
*******************************************************************************/
MBOOL
Mhal_facebeauty::
STEP2(IMEM_BUF_INFO Srcbufinfo, MUINT32 srcWidth, MUINT32 srcHeight, IMEM_BUF_INFO DSbufinfo, MtkCameraFaceMetadata* FaceInfo, void* FaceBeautyResultInfo)
{
    MY_LOGD("[STEP2] srcAdr 0x%x DSbufinfo 0x%x size ds %d",(MUINT32)Srcbufinfo.virtAddr,(MUINT32)DSbufinfo.virtAddr,DSbufinfo.size);
    CPTLog(Event_FBShot_STEP2, CPTFlagStart);
    MINT32 err = 0;
    ImgProcess(Srcbufinfo, srcWidth, srcHeight, eImgFmt_I422, DSbufinfo, mDSWidth, mDSHeight, eImgFmt_I422);
    #ifdef Debug_Mode
    char szFileName[100];
    ::sprintf(szFileName, "/sdcard/%s_%dx%d.yuv", "VGAImg", mDSWidth, mDSHeight);
    saveBufToFile(szFileName, (MUINT8*)DSbufinfo.virtAddr, mDSWidth * mDSHeight * 2);
    MY_LOGD("[STEP2] Save File done vga.adr 0x%x",DSbufinfo.virtAddr);
    #endif

    err = mpFb->mHalSTEP2((void*)DSbufinfo.virtAddr,(void*)FaceInfo,FaceBeautyResultInfo);
    if(err)
    {
    	 MY_LOGD("[STEP2] Algo fail");
    	 return  MFALSE;
    }
    CPTLog(Event_FBShot_STEP2, CPTFlagEnd);
    MY_LOGD("[STEP2] Out err %d",err);
lbExit:
    if(err)
        return  MFALSE;
    else
        return  MTRUE;
}

/*******************************************************************************
*
*******************************************************************************/
MBOOL
Mhal_facebeauty::
STEP3(IMEM_BUF_INFO Srcbufinfo, void* FaceBeautyResultInfo) const
{
	  MINT32 err = 0;
	  #ifdef Debug_Mode
    char szFileName[100];
    ::sprintf(szFileName, "/sdcard/%s_%dx%d.yuv", "MapSrcImg", mDSWidth, mDSHeight);
    saveBufToFile(szFileName, (MUINT8*)Srcbufinfo.virtAddr, mDSWidth * mDSHeight * 2); 
    MY_LOGD("[STEP3] Save File done vga.adr 0x%x",Srcbufinfo.virtAddr);
    #endif
    MY_LOGD("[STEP3] srcAdr 0x%x size  %d",(MUINT32)Srcbufinfo.virtAddr,Srcbufinfo.size);
    CPTLog(Event_FBShot_STEP3, CPTFlagStart);
    err=mpFb->mHalSTEP3((void*)Srcbufinfo.virtAddr,FaceBeautyResultInfo);
    CPTLog(Event_FBShot_STEP3, CPTFlagEnd);
    //Sava test
    #ifdef Debug_Mode
    MTKPipeFaceBeautyResultInfo* tmp=(MTKPipeFaceBeautyResultInfo*)FaceBeautyResultInfo;
    ::sprintf(szFileName, "/sdcard/%s_%dx%d.raw", "TextureMap", mDSWidth, mDSHeight);
    saveBufToFile(szFileName, (MUINT8*)tmp->Step3ResultAddr_1, (mDSWidth * mDSHeight)); 
    MY_LOGD("[STEP3] Save File done desAdr 0x%x",tmp->Step3ResultAddr_1);
    ::sprintf(szFileName, "/sdcard/%s_%dx%d.raw", "ColorMap", mDSWidth, mDSHeight);
    saveBufToFile(szFileName, (MUINT8*)tmp->Step3ResultAddr_2, (mDSWidth * mDSHeight)); 
    MY_LOGD("[STEP3] Save File done desAdr 0x%x",tmp->Step3ResultAddr_2);
    #endif
    
    if(err)
    {
    	 MY_LOGE("[STEP3] mHalSTEP3 fail");
    	 return  MFALSE;
    }
    else
    {
        MY_LOGE("[STEP3] mHalSTEP3 out");
        return  MTRUE;
    }
}
/*******************************************************************************
*
*******************************************************************************/
MBOOL
Mhal_facebeauty::
STEP1(IMEM_BUF_INFO Srcbufinfo, MUINT32 srcWidth, MUINT32 srcHeight, IMEM_BUF_INFO Desbufinfo, IMEM_BUF_INFO Alphabufinfo, void* FaceBeautyResultInfo)
{
    MBOOL   ret = MFALSE;
    IMEM_BUF_INFO tmpSTEP1;
    MY_LOGD("[STEP1] srcAdr 0x%x srcWidth %d srcHeight %d desAdr 0x%x ",(MUINT32)Srcbufinfo.virtAddr,srcWidth,srcHeight,(MUINT32)Desbufinfo.virtAddr);
    CPTLog(Event_FBShot_STEP1, CPTFlagStart);
    CPTLog(Event_FBShot_ResizeImg, CPTFlagStart);
    ImgProcess(Srcbufinfo, srcWidth, srcHeight, eImgFmt_I422, Alphabufinfo, mStep1Width, mStep1Height, eImgFmt_I420);
    CPTLog(Event_FBShot_ResizeImg, CPTFlagEnd);
    #ifdef Debug_Mode 
    char szFileName[100];
    ::sprintf(szFileName, "/sdcard/%s_%dx%d.yuv", "STEP1Img0", mStep1Width, mStep1Height);
    saveBufToFile(szFileName, (MUINT8*)Alphabufinfo.virtAddr, ((mStep1Width) * (mStep1Height) * 3 / 2)); 
    MY_LOGD("[STEP1] Save File done Alphabufinfo.virtAddr 0x%x",Alphabufinfo.virtAddr);
    #endif 
    
    MY_LOGI("[STEP1] resize 1 done ");
    CPTLog(Event_FBShot_STEP1Algo, CPTFlagStart);
    ret=mpFb->mHalSTEP1((void*)Alphabufinfo.virtAddr,FaceBeautyResultInfo);
    CPTLog(Event_FBShot_STEP1Algo, CPTFlagEnd);
    MY_LOGI("[STEP1] algorithm done ret %d",ret);
    if(ret)
    {
    	 MY_LOGD("[STEP1] STEP1 fail");
    	 return  MFALSE;
    	 goto lbExit;
    }
    
    #ifdef Debug_Mode
    ::sprintf(szFileName, "/sdcard/%s_%dx%d.yuv", "STEP1Img1", mStep1Width, mStep1Height);
    saveBufToFile(szFileName, (MUINT8*)msFaceBeautyResultInfo.Step1ResultAddr, ((mStep1Width) * (mStep1Height) * 3 / 2));
    MY_LOGD("[STEP1] Save File done msFaceBeautyResultInfo.Step1ResultAddr 0x%x",msFaceBeautyResultInfo.Step1ResultAddr);
    #endif
    CPTLog(Event_FBShot_ResizeImg, CPTFlagStart);
    tmpSTEP1.size = ((mStep1Width) * (mStep1Height) * 3 / 2);
    if(!(allocMem(tmpSTEP1)))
    {
        tmpSTEP1.size = 0;
        MY_LOGE("[STEP1] tmpSTEP1 alloc fail");
        ret = MFALSE;
        goto lbExit;
    }
    //copy DS Alphamap to tmp buffer for HW excute
    memcpy((void*)tmpSTEP1.virtAddr,msFaceBeautyResultInfo.Step1ResultAddr,tmpSTEP1.size);
    //Do resize Alphamap
    #ifdef Debug_Mode
    ::sprintf(szFileName, "/sdcard/%s_%dx%d.yuv", "STEP1Img2", mStep1Width, mStep1Height);
    saveBufToFile(szFileName, (MUINT8*)tmpSTEP1.virtAddr, ((mStep1Width) * (mStep1Height) * 3 / 2));
    MY_LOGD("[STEP1] Save File done tmpSTEP1.virtAddr 0x%x",tmpSTEP1.virtAddr);
    #endif
    ImgProcess(tmpSTEP1, (mStep1Width), (mStep1Height), eImgFmt_I420, Desbufinfo, srcWidth, srcHeight, eImgFmt_I422);
    CPTLog(Event_FBShot_ResizeImg, CPTFlagEnd);
    MY_LOGI("[STEP1] resize 2 done ");

    if(!(deallocMem(tmpSTEP1)))
    {
        tmpSTEP1.size = 0;
        MY_LOGE("[STEP1] tmpSTEP1 dealloc fail");
        ret = MFALSE;
        goto lbExit;
    }
    //Sava Test
    #ifdef Debug_Mode
    ::sprintf(szFileName, "/sdcard/%s_%dx%d.yuv", "STEP1Img", srcWidth, srcHeight);
    saveBufToFile(szFileName, (MUINT8*)Desbufinfo.virtAddr, (srcWidth * srcHeight * 2));
    MY_LOGD("[STEP1] Save File done Srcbufinfo 0x%x",Srcbufinfo.virtAddr);
    #endif
    CPTLog(Event_FBShot_STEP1, CPTFlagEnd);

 lbExit:
    if(ret)
        return  MFALSE;
    else
        return  MTRUE;
}

MBOOL
Mhal_facebeauty::
STEP4(IMEM_BUF_INFO Srcbufinfo, MUINT32 srcWidth, MUINT32 srcHeight, IMEM_BUF_INFO Blurbufinfo, IMEM_BUF_INFO Alphabufinfo, void* FaceBeautyResultInfo)
{
	  MINT32 ret = 0;
	  MY_LOGD("[STEP4] in");
	  MTKPipeFaceBeautyResultInfo* tmp=(MTKPipeFaceBeautyResultInfo*)FaceBeautyResultInfo;
    CPTLog(Event_FBShot_STEP4, CPTFlagStart);
    CPTLog(Event_FBShot_ResizeImg, CPTFlagStart);
    int const DSAMPX = mDSWidth;
    int const DSAMPY = mDSHeight;
    IMEM_BUF_INFO tmpStep4;
    tmpStep4.size = DSAMPX * DSAMPY * 2;
    if(!(allocMem(tmpStep4)))
    {
        tmpStep4.size = 0;
        MY_LOGE("[STEP4] tmpStep4 alloc fail");
        ret = MTRUE;
        goto lbExit;
    }
    //copy DS Alphamap to tmp buffer for HW excute
    memcpy((void*)tmpStep4.virtAddr,msFaceBeautyResultInfo.Step3ResultAddr_1,tmpStep4.size);
    //Do resize Alphamap
    ImgProcess(tmpStep4, DSAMPX, DSAMPY, eImgFmt_I422, Alphabufinfo, srcWidth, srcHeight, eImgFmt_I422);
    CPTLog(Event_FBShot_ResizeImg, CPTFlagEnd);
    MY_LOGD("[STEP4] resize done");
    CPTLog(Event_FBShot_STEP4Algo, CPTFlagStart);
    ret=mpFb->mHalSTEP4((void*)Srcbufinfo.virtAddr,(void*)Blurbufinfo.virtAddr,(void*)Alphabufinfo.virtAddr,FaceBeautyResultInfo);
    CPTLog(Event_FBShot_STEP4Algo, CPTFlagEnd);
    MY_LOGD("[STEP4] algorithm done ret %d",ret);
    if(ret)
    {
    	 MY_LOGD("[STEP4] STEP4 fail");
    	 return  MFALSE;
    	 goto lbExit;
    }
    if(!(deallocMem(tmpStep4)))
    {
        tmpStep4.size = 0;
        MY_LOGE("[STEP4] tmpStep4 dealloc fail");
        ret = MTRUE;
        goto lbExit;
    }
    CPTLog(Event_FBShot_STEP4, CPTFlagEnd);
    //------------------ Sava test ----------------//
    #ifdef Debug_Mode
    char szFileName[100];
    ::sprintf(szFileName, "/sdcard/%s_%dx%d.yuv", "Step4Img", srcWidth, srcHeight);
    saveBufToFile(szFileName, (MUINT8*)msFaceBeautyResultInfo.Step4ResultAddr, (srcWidth * srcHeight * 2));
    MY_LOGD("[STEP4] Save File done Step4ResultAddr 0x%x Result 0x%x",msFaceBeautyResultInfo.Step4ResultAddr,ret);
    #endif
    //------------------ Sava test ----------------//
lbExit:
   	MY_LOGD("[STEP4] out ret %d",ret);
    if(ret)
        return  MFALSE;
    else
        return  MTRUE;
}

MBOOL
Mhal_facebeauty::
STEP5(IMEM_BUF_INFO Srcbufinfo, MUINT32 srcWidth, MUINT32 srcHeight, IMEM_BUF_INFO Alphabufinfo, void* FaceBeautyResultInfo)
{
	  MINT32 ret = 0;
	  MY_LOGD("[STEP5] in");
	  MTKPipeFaceBeautyResultInfo* tmp=(MTKPipeFaceBeautyResultInfo*)FaceBeautyResultInfo;
    CPTLog(Event_FBShot_STEP5, CPTFlagStart);
    CPTLog(Event_FBShot_ResizeImg, CPTFlagStart);
    int const DSAMPX = mDSWidth;
    int const DSAMPY = mDSHeight;
    IMEM_BUF_INFO tmpStep5;
    tmpStep5.size = DSAMPX*DSAMPY * 2;
    if(!(allocMem(tmpStep5)))
    {
        tmpStep5.size = 0;
        MY_LOGE("[STEP5] tmpStep5 alloc fail");
        ret = MTRUE;
        goto lbExit;
    }
    memcpy((void*)tmpStep5.virtAddr,msFaceBeautyResultInfo.Step3ResultAddr_2,tmpStep5.size);
    ImgProcess(tmpStep5, DSAMPX, DSAMPY, eImgFmt_I422, Alphabufinfo, srcWidth, srcHeight, eImgFmt_I422);
    CPTLog(Event_FBShot_ResizeImg, CPTFlagEnd);
    
    MY_LOGD("[STEP5] resize done");
    CPTLog(Event_FBShot_STEP5Algo, CPTFlagStart);
    ret=mpFb->mHalSTEP5((void*)Srcbufinfo.virtAddr,(void*)Alphabufinfo.virtAddr,FaceBeautyResultInfo);
    CPTLog(Event_FBShot_STEP5Algo, CPTFlagEnd);
    MY_LOGD("[STEP5] algorithm done ret %d",ret);
    if(ret)
    {
    	 MY_LOGD("[STEP5] STEP5 fail");
    	 return  MFALSE;
    	 goto lbExit;
    }
    if(!(deallocMem(tmpStep5)))
    {
        ret = MTRUE;
        goto lbExit;
    }
    CPTLog(Event_FBShot_STEP5, CPTFlagEnd);
    //------------------ Sava test ----------------//
    #ifdef Debug_Mode
    char szFileName[100];
    ::sprintf(szFileName, "/sdcard/%s_%dx%d.yuv", "STEP5Img", srcWidth, srcHeight);
    saveBufToFile(szFileName, (MUINT8*)msFaceBeautyResultInfo.Step5ResultAddr, (srcWidth * srcHeight * 2));
    MY_LOGD("[STEP5] Save File done Step5ResultAddr 0x%x Result 0x%x ",msFaceBeautyResultInfo.Step5ResultAddr,ret);
    #endif
    //------------------ Sava test ----------------//
lbExit:
   	MY_LOGD("[STEP5] out ret %d",ret);
   	if(ret)
        return  MFALSE;
    else
        return  MTRUE;
}

MBOOL
Mhal_facebeauty::
STEP6(IMEM_BUF_INFO Srcbufinfo, MUINT32 srcWidth, MUINT32 srcHeight, IMEM_BUF_INFO tmpbufinfo, void* FaceBeautyResultInfo) const
{
	  MINT32 ret = 0;
	  MY_LOGD("[STEP6] in");
	  MTKPipeFaceBeautyResultInfo* tmp=(MTKPipeFaceBeautyResultInfo*)FaceBeautyResultInfo;
    CPTLog(Event_FBShot_STEP6, CPTFlagStart);
    ret=mpFb->mHalSTEP6((void*)Srcbufinfo.virtAddr,(void*)tmpbufinfo.virtAddr,FaceBeautyResultInfo);
    MY_LOGD("[STEP6] algorithm done ret %d",ret);
    CPTLog(Event_FBShot_STEP6, CPTFlagEnd);
    //------------------ Sava test ----------------//
    #ifdef Debug_Mode
    char szFileName[100];
    ::sprintf(szFileName, "/sdcard/%s_%dx%d.yuv", "Step6Img", srcWidth, srcHeight);
    saveBufToFile(szFileName, (MUINT8*)msFaceBeautyResultInfo.Step6ResultAddr, (srcWidth * srcHeight * 2));
    MY_LOGD("[STEP6] Save File done Step6ResultAddr 0x%x Result 0x%x ",msFaceBeautyResultInfo.Step6ResultAddr,ret);
    #endif
    //------------------ Sava test ----------------//
lbExit:
   	MY_LOGD("[STEP6] out ret %d",ret);
   	if(ret)
        return  MFALSE;
    else
        return  MTRUE;
}

