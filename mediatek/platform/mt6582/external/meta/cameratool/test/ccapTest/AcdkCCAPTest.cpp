///////////////////////////////////////////////////////////////////////////////
// No Warranty
// Except as may be otherwise agreed to in writing, no warranties of any
// kind, whether express or implied, are given by MTK with respect to any MTK
// Deliverables or any use thereof, and MTK Deliverables are provided on an
// "AS IS" basis.  MTK hereby expressly disclaims all such warranties,
// including any implied warranties of merchantability, non-infringement and
// fitness for a particular purpose and any warranties arising out of course
// of performance, course of dealing or usage of trade.  Parties further
// acknowledge that Company may, either presently and/or in the future,
// instruct MTK to assist it in the development and the implementation, in
// accordance with Company's designs, of certain softwares relating to
// Company's product(s) (the "Services").  Except as may be otherwise agreed
// to in writing, no warranties of any kind, whether express or implied, are
// given by MTK with respect to the Services provided, and the Services are
// provided on an "AS IS" basis.  Company further acknowledges that the
// Services may contain errors, that testing is important and Company is
// solely responsible for fully testing the Services and/or derivatives
// thereof before they are used, sublicensed or distributed.  Should there be
// any third party action brought against MTK, arising out of or relating to
// the Services, Company agree to fully indemnify and hold MTK harmless.
// If the parties mutually agree to enter into or continue a business
// relationship or other arrangement, the terms and conditions set forth
// hereunder shall remain effective and, unless explicitly stated otherwise,
// shall prevail in the event of a conflict in the terms in any agreements
// entered into between the parties.
////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2008, MediaTek Inc.
// All rights reserved.
//
// Unauthorized use, practice, perform, copy, distribution, reproduction,
// or disclosure of this information in whole or in part is prohibited.
////////////////////////////////////////////////////////////////////////////////
// AcdkCCAPTest.cpp  $Revision$
////////////////////////////////////////////////////////////////////////////////

//! \file  AcdkCCAPTest.cpp
//! \brief

#define LOG_TAG "ACDKCCAPTest"
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <linux/input.h>
#include <unistd.h>

extern "C" {
#include <pthread.h>
}

#include <mtkcam/acdk/cct_feature.h>
#include <mtkcam/acdk/CctIF.h>
#include <mtkcam/acdk/MdkIF.h>

#include "AcdkLog.h"
//#include "AcdkErrCode.h"
#include "cct_ErrCode.h"
#include "camera_custom_eeprom.h"

#define MEDIA_PATH "//data"
#define PROJECT_NAME "Yusu"

/////////////////////////////////////////////////////////////////////////
//
//! @brief insert the PMEM driver module shell command
//
/////////////////////////////////////////////////////////////////////////
static  const char*  const g_mkPMEMNod_arg_list[] = {
    "mknod",
    "/dev/pmem_multimedia",
    "c",
    "10",
    "0",
    NULL
};

/////////////////////////////////////////////////////////////////////////
//
//! @brief mount SD shell command
//
/////////////////////////////////////////////////////////////////////////
static  const char* const g_mountSD_arg_list[]  = {
       "mount",
       "-t",
       "vfat",
       "/dev/mmcblk0p1",
       "/sdcard",
       NULL
};

/////////////////////////////////////////////////////////////////////////
//
//! @brief unmount SD shell command
//
/////////////////////////////////////////////////////////////////////////
static  const char*  const g_unMountSD_arg_list[] = {
    "umount",
    "/sdcard",
    NULL
};

/////////////////////////////////////////////////////////////////////////
//
//! @brief sync shell command
//
/////////////////////////////////////////////////////////////////////////
static  const char* const g_sync_arg_list[]  = {
    "sync",
    NULL
};


/////////////////////////////////////////////////////////////////////////
//  Global Variable for the the thread
/////////////////////////////////////////////////////////////////////////
static BOOL g_bIsCLITest = MTRUE;
static pthread_t g_CliKeyThreadHandle;



/////////////////////////////////////////////////////////////////////////
//
//  thread_exit_handler () -
//! @brief the CLI key input thread, wait for CLI command
//! @param sig: The input arguments
/////////////////////////////////////////////////////////////////////////
void thread_exit_handler(MINT32 a_u4Sig)
{
    ACDK_LOGD("This signal is %d \n", a_u4Sig);
    pthread_exit(0);
}


/////////////////////////////////////////////////////////////////////////
//
//  vExecProgram () -
//! @brief execute the external program
//! @param pProgram: program name
//! @param ppArgList: Arguments
/////////////////////////////////////////////////////////////////////////
VOID vExecProgram(const char *pProgram, const char * const ppArgList[])
{
    pid_t childPid;

    //Duplicate this process
    childPid = fork ();

    if (childPid != 0)
    {
        return;
    }
    else
    {
        //execute the program, searching for it in the path
        execvp(pProgram, (char **)ppArgList);
        abort();
    }
}

/////////////////////////////////////////////////////////////////////////
//
//  vSkipSpace () -
//! @brief skip the space of the input string
//! @param ppInStr: The point of the input string
/////////////////////////////////////////////////////////////////////////
void vSkipSpace(char **ppInStr)
{
    char *s = *ppInStr;

    while (( *s == ' ' ) || ( *s == '\t' ) || ( *s == '\r' ) || ( *s == '\n' ))
    {
        s++;
    }

    *ppInStr = s;
}


/////////////////////////////////////////////////////////////////////////
//
//  getHexToken () -
//! @brief skip the space of the input string
//! @param ppInStr: The point of the input string
/////////////////////////////////////////////////////////////////////////
char* getHexToken(char *inStr, MUINT32 *outVal)
{
    MUINT32 thisVal, tVal;
    char x;
    char *thisStr = inStr;

    thisVal = 0;

    // If first character is ';', we have a comment, so
    // get out of here.

    if (*thisStr == ';')
    {
        return (thisStr);
    }
        // Process hex characters.

    while (*thisStr)
    {
        // Do uppercase conversion if necessary.

        x = *thisStr;
        if ((x >= 'a') && (x <= 'f'))
        {
            x &= ~0x20;
        }
        // Check for valid digits.

        if ( !(((x >= '0') && (x <= '9')) || ((x >= 'A') && (x <= 'F'))))
        {
            break;
        }
        // Hex ASCII to binary conversion.

        tVal = (MUINT32)(x - '0');
        if (tVal > 9)
        {
            tVal -= 7;
        }

        thisVal = (thisVal * 16) + tVal;

        thisStr++;
    }

        // Return updated pointer and decoded value.

    *outVal = thisVal;
    return (thisStr);
}

static MUINT32 g_u4ImgCnt = 0;

/////////////////////////////////////////////////////////////////////////
//
//   mrSaveRAWImg () -
//!
//!  brief for geneirc function to save image file
//
/////////////////////////////////////////////////////////////////////////
BOOL bRAW10To8(char *a_pInBuf,  MUINT32 a_u4Width, MUINT32 a_u4Height, MUINT32 a_u4Size, UINT8 a_uBitDepth,  char *a_pOutBuf)
{
    if (a_uBitDepth != 10)
    {
        ACDK_LOGE("Not support bitdepth");
        return MFALSE;
    }

    MUINT32 *pu4SrcBuf = (MUINT32 *) a_pInBuf;
    //char *pucBuf = (UCHAR *) malloc (a_u4Width * a_u4Height  * 2 * sizeof(UCHAR));

    char *puDestBuf = (char *)a_pOutBuf;

    while (puDestBuf < (char *)a_pOutBuf + a_u4Width * a_u4Height)
    {
        MUINT32 u4Pixel = *(pu4SrcBuf++);
        *(puDestBuf++) = (char)((u4Pixel & 0x03FF) >> 2);
        *(puDestBuf++) = (char)(((u4Pixel >> 10) & 0x03FF) >> 2);
        *(puDestBuf++) = (char)(((u4Pixel >> 20) & 0x03FF) >> 2);
    }


    return MTRUE;
}


/////////////////////////////////////////////////////////////////////////
//
//   mrSaveRAWImg () -
//!
//!  brief for geneirc function to save image file
//
/////////////////////////////////////////////////////////////////////////
BOOL bSaveRAWImg(char *a_pBuf,  MUINT32 a_u4Width, MUINT32 a_u4Height, MUINT32 a_u4Size, UINT8 a_uBitDepth, UINT8 a_uBayerStart)
{
    char szFileName[256];

    UINT8 uBayerStart = 0;

    //convert the bayerstart, //TODO
    switch (a_uBayerStart)
    {
        case 0xB4:
            uBayerStart = 3;
    	    break;
    }

    sprintf(szFileName, "%s//%04d_%s_%dx%d_%d_%d.raw" , MEDIA_PATH,
                                                          g_u4ImgCnt,
                                                          PROJECT_NAME,
                                                          a_u4Width,
                                                          a_u4Height,
                                                          a_uBitDepth,
                                                          uBayerStart);

    MUINT32 *pu4SrcBuf = (MUINT32 *) a_pBuf;
    char *pucBuf = (char *) malloc (a_u4Width * a_u4Height  * 2 * sizeof(char));

    UINT16 *pu2DestBuf = (UINT16 *)pucBuf;

    while (pu2DestBuf < (UINT16 *)pucBuf + a_u4Width * a_u4Height)
    {
        MUINT32 u4Pixel = *(pu4SrcBuf++);
        *(pu2DestBuf++) = (UINT16)(u4Pixel & 0x03FF);
        *(pu2DestBuf++) = (UINT16)((u4Pixel >> 10) & 0x03FF);
        *(pu2DestBuf++) = (UINT16)((u4Pixel >> 20) & 0x03FF);
    }


    FILE *pFp = fopen(szFileName, "wb");

    if (NULL == pFp )
    {
        ACDK_LOGE("Can't open file to save Image\n");
        return MFALSE;
    }

    MINT32 i4WriteCnt = fwrite(pucBuf, 1, a_u4Width * a_u4Height  * 2  , pFp);

    ACDK_LOGD("Save image file name:%s\n", szFileName);

    fclose(pFp);
    sync();
    free(pucBuf);
    return MTRUE;
}

/////////////////////////////////////////////////////////////////////////
//
//   mrSaveJPEGImg () -
//!
//!  brief for geneirc function to save image file
//
/////////////////////////////////////////////////////////////////////////
BOOL bSaveJPEGImg(char *a_pBuf,  MUINT32 a_u4Size)
{
    char szFileName[256];

    sprintf(szFileName, "%s//%04d_%s.jpg" , MEDIA_PATH, g_u4ImgCnt, PROJECT_NAME);

    FILE *pFp = fopen(szFileName, "wb");

    if (NULL == pFp )
    {
        ACDK_LOGE("Can't open file to save Image\n");
        return MFALSE;
    }

	MINT32 i4WriteCnt = fwrite(a_pBuf, 1, a_u4Size , pFp);

    ACDK_LOGD("Save image file name:%s\n", szFileName);

    fclose(pFp);
    sync();
    return MTRUE;
}


/////////////////////////////////////////////////////////////////////////
//! CCAP CLI Test Command
//! For test CCAP -> ACDK interface
/////////////////////////////////////////////////////////////////////////
static BOOL g_bAcdkOpend = MFALSE;
static bool bSendDataToACDK(MINT32   FeatureID,
						    MVOID*					pInAddr,
						    MUINT32					nInBufferSize,
                            MVOID*                  pOutAddr,
						    MUINT32					nOutBufferSize,
						    MUINT32*				pRealOutByeCnt)
{
    ACDK_FEATURE_INFO_STRUCT rAcdkFeatureInfo;

    rAcdkFeatureInfo.puParaIn = (MUINT8*)pInAddr;
    rAcdkFeatureInfo.u4ParaInLen = nInBufferSize;
    rAcdkFeatureInfo.puParaOut = (MUINT8*)pOutAddr;
    rAcdkFeatureInfo.u4ParaOutLen = nOutBufferSize;
    rAcdkFeatureInfo.pu4RealParaOutLen = pRealOutByeCnt;


    return (Mdk_IOControl(FeatureID, &rAcdkFeatureInfo));
}

static bool bSendDataToCCT(MINT32   FeatureID,
						    MVOID*					pInAddr,
						    MUINT32					nInBufferSize,
                            MVOID*                  pOutAddr,
						    MUINT32					nOutBufferSize,
						    MUINT32*				pRealOutByeCnt)
{
    ACDK_FEATURE_INFO_STRUCT rAcdkFeatureInfo;

    rAcdkFeatureInfo.puParaIn = (MUINT8*)pInAddr;
    rAcdkFeatureInfo.u4ParaInLen = nInBufferSize;
    rAcdkFeatureInfo.puParaOut = (MUINT8*)pOutAddr;
    rAcdkFeatureInfo.u4ParaOutLen = nOutBufferSize;
    rAcdkFeatureInfo.pu4RealParaOutLen = pRealOutByeCnt;


    return (CctIF_IOControl(FeatureID, &rAcdkFeatureInfo));
}


static BOOL bCapDone = MFALSE;


/////////////////////////////////////////////////////////////////////////
// vPrvCb
//! @brief capture callback function for ACDK to callback the capture buffer
//! @param a_pParam: the callback image buffer info
//!                              the buffer info will be
//!                              if the buffer type is JPEG, it use CapBufInfo
//!                              if the buffer type is RAW, it use RAWBufInfo
//!
/////////////////////////////////////////////////////////////////////////
static VOID vCapCb(VOID *a_pParam)
{
#if 0
    ACDK_LOGD("Capture Callback \n");

    ImageBufInfo *pImgBufInfo = (ImageBufInfo *)a_pParam;

    ACDK_LOGD("Buffer Type:%d\n",  pImgBufInfo->eImgType);

    BOOL bRet = MTRUE;

    if (pImgBufInfo->eImgType == RAW_TYPE)
    {
        //! currently the RAW buffer type is packed buffer
        //! The packed format is the same as MT6516 ISP format <00 Pixel1, Pixel2, Pixel3 > in 4bytes
        ACDK_LOGD("Size:%d\n", pImgBufInfo->RAWImgBufInfo.imgSize);
        ACDK_LOGD("Width:%d\n", pImgBufInfo->RAWImgBufInfo.imgWidth);
        ACDK_LOGD("Height:%d\n", pImgBufInfo->RAWImgBufInfo.imgHeight);
        ACDK_LOGD("BitDepth:%d\n", pImgBufInfo->RAWImgBufInfo.bitDepth);
        ACDK_LOGD("Bayer Start:%d\n", pImgBufInfo->RAWImgBufInfo.eColorOrder);

#if 1
        bRet = bSaveRAWImg((char *)pImgBufInfo->RAWImgBufInfo.bufAddr,
                           pImgBufInfo->RAWImgBufInfo.imgWidth,
                           pImgBufInfo->RAWImgBufInfo.imgHeight,
                           pImgBufInfo->RAWImgBufInfo.imgSize,
                           pImgBufInfo->RAWImgBufInfo.bitDepth,
                           pImgBufInfo->RAWImgBufInfo.eColorOrder);
#else  //RAW8 Save
        UCHAR *pBuf = (UCHAR *) malloc (pImgBufInfo->rRawBufInfo.rRawImgInfo.u2Width * pImgBufInfo->rRawBufInfo.rRawImgInfo.u2Height * 1);

        bRet = bRAW10To8(pImgBufInfo->rRawBufInfo.pucRawBuf,
                                       pImgBufInfo->rRawBufInfo.rRawImgInfo.u2Width,
                                       pImgBufInfo->rRawBufInfo.rRawImgInfo.u2Height,
                                       pImgBufInfo->rRawBufInfo.u4ImgSize,
                                       pImgBufInfo->rRawBufInfo.rRawImgInfo.uBitDepth,
                                       pBuf);
        FILE *pFp = fopen("/data/test8.raw", "wb");

        if (NULL == pFp )
        {
            ACDK_LOGE("Can't open file to save Image\n");
        }

        MINT32 i4WriteCnt = fwrite(pBuf, 1, pImgBufInfo->rRawBufInfo.rRawImgInfo.u2Width * pImgBufInfo->rRawBufInfo.rRawImgInfo.u2Height  * 1  , pFp);

        fclose(pFp);
        sync();
        free(pBuf);
#endif

    }
    else if (pImgBufInfo->eImgType == JPEG_TYPE)
   {
        ACDK_LOGD("Size:%d\n", pImgBufInfo->rCapBufInfo.u4ImgSize);
        ACDK_LOGD("Width:%d\n", pImgBufInfo->rCapBufInfo.u2ImgXRes);
        ACDK_LOGD("Height:%d\n", pImgBufInfo->rCapBufInfo.u2ImgYRes)

        bRet = bSaveJPEGImg((char *)pImgBufInfo->rCapBufInfo.pucImgBuf,
                                             pImgBufInfo->rCapBufInfo.u4ImgSize);
   }
   else
   {
        ACDK_LOGD("UnKnow Format \n");
   }

    bCapDone = MTRUE;
    g_u4ImgCnt ++;
#endif

    ACDK_LOGD("[AcdkCCAPTest]::Capture Callback\n");

    ImageBufInfo *pImgBufInfo = (ImageBufInfo *)a_pParam;
    bCapDone = MTRUE;

}

/////////////////////////////////////////////////////////////////////////
// vPrvCb
//! @brief preview callback function for ACDK to callback the preview buffer
//! @param a_pParam: the callback image buffer info
//!                              the buffer info will be PrvVDOBufInfo structure
//!
/////////////////////////////////////////////////////////////////////////
static VOID vPrvCb(VOID *a_pParam)
{
    //ACDK_LOGD("Preview Callback \n");

    ImageBufInfo *pImgBufInfo = (ImageBufInfo *)a_pParam;

    //ACDK_LOGD("Buffer Type:%d\n",  pImgBufInfo->eImgType);
    //ACDK_LOGD("Size:%d\n", pImgBufInfo->imgBufInfo.imgSize);
    //ACDK_LOGD("Width:%d\n", pImgBufInfo->imgBufInfo.imgWidth);
    //ACDK_LOGD("Height:%d\n", pImgBufInfo->imgBufInfo.imgHeight);
}




/////////////////////////////////////////////////////////////////////////
// FT_CCT_OP_PREVIEW_LCD_START
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPPreviewStart_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{

    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("FT_CCT_OP_PREVIEW_LCD_START\n");

    ACDK_PREVIEW_STRUCT rCCTPreviewConfig;

    rCCTPreviewConfig.fpPrvCB = vPrvCb;
    rCCTPreviewConfig.u4PrvW= 320;
    rCCTPreviewConfig.u4PrvH= 240;
	rCCTPreviewConfig.u16PreviewTestPatEn = 0;

    MUINT32 u4RetLen = 0;


    BOOL bRet = bSendDataToACDK (ACDK_CMD_PREVIEW_START/*ACDK_CCT_OP_PREVIEW_LCD_START*/, (UINT8 *)&rCCTPreviewConfig,
                                                                                                                sizeof(ACDK_PREVIEW_STRUCT),
                                                                                                                NULL,
                                                                                                                0,
                                                                                                                &u4RetLen);
    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }

    return S_CCT_CCAP_OK;

}

/////////////////////////////////////////////////////////////////////////
// FT_CCT_OP_PREVIEW_LCD_STOP
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPPreviewStop_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("FT_CCT_OP_PREVIEW_LCD_STOP\n");

    MUINT32 u4RetLen = 0;
    BOOL bRet = bSendDataToACDK(ACDK_CMD_PREVIEW_STOP/*ACDK_CCT_OP_PREVIEW_LCD_STOP*/, NULL, 0, NULL, 0, &u4RetLen);

    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }
    return S_CCT_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// FT_CCT_OP_SINGLE_SHOT_CAPTURE
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPSingleShot_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    int format;

    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("FT_CCT_OP_SINGLE_SHOT_CAPTURE\n");
    if (a_u4Argc != 2 && a_u4Argc != 4)
    {
        ACDK_LOGD("Usage: cap <mode, prv:0, cap:1> <format, 1:raw, 0:jpg> <width (Option)> <height (Option)>\n");
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_CAPTURE_STRUCT_S/*ACDK_CCT_STILL_CAPTURE_STRUCT*/ rCCTStillCapConfig;

    rCCTStillCapConfig.eCameraMode = (eACDK_CAMERA_MODE/*eCAMERA_MODE*/)atoi((char *)a_pprArgv[0]);
    format = atoi((char *)a_pprArgv[1]);

    switch(format) {
        case 0:
            rCCTStillCapConfig.eOutputFormat = JPEG_TYPE/*OUTPUT_JPEG*/;
            ACDK_LOGD("[mrCCAPSingleShot_Cmd] OUTPUT_JPEG\n");
            break;

        case 1:
            rCCTStillCapConfig.eOutputFormat = PURE_RAW8_TYPE/*OUTPUT_PURE_RAW8*/;
            ACDK_LOGD("[mrCCAPSingleShot_Cmd] OUTPUT_PURE_RAW8\n");
            break;

        case 2:
            rCCTStillCapConfig.eOutputFormat = PURE_RAW10_TYPE/*OUTPUT_PURE_RAW10*/;
            ACDK_LOGD("[mrCCAPSingleShot_Cmd] OUTPUT_PURE_RAW10\n");
            break;

        case 3:
            rCCTStillCapConfig.eOutputFormat = PROCESSED_RAW8_TYPE/*OUTPUT_PROCESSED_RAW8*/;
            ACDK_LOGD("[mrCCAPSingleShot_Cmd] OUTPUT_PROCESSED_RAW8\n");
            break;
        case 4:
            rCCTStillCapConfig.eOutputFormat = PROCESSED_RAW10_TYPE/*OUTPUT_PROCESSED_RAW10*/;
            ACDK_LOGD("[mrCCAPSingleShot_Cmd] OUTPUT_PROCESSED_RAW10\n");
            break;

        default:
            rCCTStillCapConfig.eOutputFormat = PURE_RAW8_TYPE/*OUTPUT_PURE_RAW8*/;
            ACDK_LOGD("[mrCCAPSingleShot_Cmd] OUTPUT_PURE_RAW8\n");
            break;
    }


    if  (a_u4Argc == 4)
    {
        rCCTStillCapConfig.u2JPEGEncWidth = atoi((char *)a_pprArgv[2]);
        rCCTStillCapConfig.u2JPEGEncHeight = atoi((char *)a_pprArgv[3]);
    }
    else
    {
        rCCTStillCapConfig.u2JPEGEncWidth = 0;
        rCCTStillCapConfig.u2JPEGEncHeight = 0;
    }
    rCCTStillCapConfig.fpCapCB = vCapCb;
    MUINT32 u4RetLen = 0;

    bCapDone = MFALSE;
    BOOL bRet = bSendDataToACDK(ACDK_CMD_CAPTURE/*ACDK_CCT_OP_SINGLE_SHOT_CAPTURE_EX*/, (UINT8 *)&rCCTStillCapConfig,
                                                                                                                     sizeof(ACDK_CCT_STILL_CAPTURE_STRUCT),
                                                                                                                     NULL,
                                                                                                                     0,
                                                                                                                     &u4RetLen);

    //wait JPEG Done;
    while (!bCapDone)
    {
        ACDK_LOGD("Capture Waiting...\n");
        usleep(1000);
    }

    ACDK_LOGD("CCT_OP_PREVIEW_LCD_START\n");

    ACDK_PREVIEW_STRUCT rCCTPreviewConfig;

    rCCTPreviewConfig.fpPrvCB = vPrvCb;
    rCCTPreviewConfig.u4PrvW= 320;
    rCCTPreviewConfig.u4PrvH= 240;
	rCCTPreviewConfig.u16PreviewTestPatEn = 0;

    bRet = bSendDataToACDK (ACDK_CMD_PREVIEW_START/*ACDK_CCT_OP_PREVIEW_LCD_START*/, (UINT8 *)&rCCTPreviewConfig,
                                                                                                                sizeof(ACDK_PREVIEW_STRUCT),
                                                                                                                NULL,
                                                                                                                0,
                                                                                                                &u4RetLen);

    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }

    return S_CCT_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// FT_CCT_OP_MULTI_SHOT_CAPTURE
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPMultiShot_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    int format;

    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("FT_CCT_OP_MULTI_SHOT_CAPTURE\n");

    if (a_u4Argc != 3)
    {
        ACDK_LOGD("Usage: cap <mode, prv:0, cap:1> <format, 1:raw, 0:jpg> <count> \n");
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_CAPTURE_STRUCT_S/*ACDK_CCT_MULTI_SHOT_CAPTURE_STRUCT*/ rCCTMutiShotConfig;

    rCCTMutiShotConfig.eCameraMode = (eACDK_CAMERA_MODE/*eCAMERA_MODE*/)atoi((char *)a_pprArgv[0]);
    format = atoi((char *)a_pprArgv[1]);

    switch(format) {
        case 0:
            rCCTMutiShotConfig.eOutputFormat = JPEG_TYPE/*OUTPUT_JPEG*/;
            ACDK_LOGD("[mrCCAPMultiShot_Cmd] OUTPUT_JPEG\n");
            break;

        case 1:
            rCCTMutiShotConfig.eOutputFormat = PURE_RAW8_TYPE/*OUTPUT_PURE_RAW8*/;
            ACDK_LOGD("[mrCCAPMultiShot_Cmd] OUTPUT_PURE_RAW8\n");
            break;

        case 2:
            rCCTMutiShotConfig.eOutputFormat = PURE_RAW10_TYPE/*OUTPUT_PURE_RAW10*/;
            ACDK_LOGD("[mrCCAPMultiShot_Cmd] OUTPUT_PURE_RAW10\n");
            break;

        case 3:
            rCCTMutiShotConfig.eOutputFormat = PROCESSED_RAW8_TYPE/*OUTPUT_PROCESSED_RAW8*/;
            ACDK_LOGD("[mrCCAPMultiShot_Cmd] OUTPUT_PROCESSED_RAW8\n");
            break;
        case 4:
            rCCTMutiShotConfig.eOutputFormat = PROCESSED_RAW10_TYPE/*OUTPUT_PROCESSED_RAW10*/;
            ACDK_LOGD("[mrCCAPMultiShot_Cmd] OUTPUT_PROCESSED_RAW10\n");
            break;

        default:
            rCCTMutiShotConfig.eOutputFormat = PURE_RAW8_TYPE/*OUTPUT_PURE_RAW8*/;
            ACDK_LOGD("[mrCCAPMultiShot_Cmd] OUTPUT_PURE_RAW8\n");
            break;
    }

    rCCTMutiShotConfig.u2JPEGEncWidth = 2560;
    rCCTMutiShotConfig.u2JPEGEncHeight = 1920;
    rCCTMutiShotConfig.fpCapCB = vCapCb;
    rCCTMutiShotConfig.u4CapCount = atoi((char *)a_pprArgv[2]);
    MUINT32 u4RetLen = 0;

    bCapDone = MFALSE;
    BOOL bRet = bSendDataToACDK(ACDK_CMD_CAPTURE/*ACDK_CCT_OP_MULTI_SHOT_CAPTURE_EX*/, (UINT8 *)&rCCTMutiShotConfig,
                                                                                                                     sizeof(ACDK_CCT_MULTI_SHOT_CAPTURE_STRUCT),
                                                                                                                     NULL,
                                                                                                                     0,
                                                                                                                     &u4RetLen);

    //wait JPEG Done;
    while (!bCapDone)
    {
        usleep(1000);
    }

    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }
    return S_CCT_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_OP_LOAD_FROM_NVRAM
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPLoadFromNvram_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_OP_LOAD_FROM_NVRAM\n");

    BOOL bRet = bSendDataToCCT(ACDK_CCT_OP_LOAD_FROM_NVRAM, NULL, 0, NULL, 0, NULL);

    if (!bRet)
    {
        ACDK_LOGE("ACDK_CCT_OP_LOAD_FROM_NVRAM Fail\n");
        return E_CCT_CCAP_API_FAIL;
    }

    return S_CCT_CCAP_OK;
}


/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_OP_SAVE_TO_NVRAM
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPSaveToNvram_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_OP_SAVE_TO_NVRAM\n");

    BOOL bRet = bSendDataToCCT(ACDK_CCT_OP_SAVE_TO_NVRAM, NULL, 0, NULL, 0, NULL);

    if (!bRet)
    {
        ACDK_LOGE("ACDK_CCT_OP_SAVE_TO_NVRAM Fail\n");
        return E_CCT_CCAP_API_FAIL;
    }

    return S_CCT_CCAP_OK;
}


/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_OP_ISP_LOAD_FROM_NVRAM
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPISPLoadFromNvram_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_OP_ISP_LOAD_FROM_NVRAM\n");

    BOOL bRet = ::bSendDataToCCT(ACDK_CCT_OP_ISP_LOAD_FROM_NVRAM, NULL, 0, NULL, 0, NULL);

    if (!bRet)
    {
        ACDK_LOGE("ACDK_CCT_OP_ISP_LOAD_FROM_NVRAM Fail\n");
        return E_CCT_CCAP_API_FAIL;
    }

    return S_CCT_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_OP_3A_LOAD_FROM_NVRAM
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAP3ALoadFromNvram_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_OP_3A_LOAD_FROM_NVRAM\n");

    BOOL bRet = ::bSendDataToCCT(ACDK_CCT_OP_3A_LOAD_FROM_NVRAM, NULL, 0, NULL, 0, NULL);

    if (!bRet)
    {
        ACDK_LOGE("ACDK_CCT_OP_3A_LOAD_FROM_NVRAM Fail\n");
        return E_CCT_CCAP_API_FAIL;
    }

    return S_CCT_CCAP_OK;
}


/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_OP_ISP_SAVE_TO_NVRAM
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPISPSaveToNvram_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_OP_ISP_SAVE_TO_NVRAM\n");

    BOOL bRet = ::bSendDataToCCT(ACDK_CCT_OP_ISP_SAVE_TO_NVRAM, NULL, 0, NULL, 0, NULL);

    if (!bRet)
    {
        ACDK_LOGE("ACDK_CCT_OP_ISP_SAVE_TO_NVRAM Fail\n");
        return E_CCT_CCAP_API_FAIL;
    }

    return S_CCT_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_ISP_GET_NVRAM_DATA
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPISPGetNvramData_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    printf("ACDK_CCT_V2_OP_ISP_GET_NVRAM_DATA\n");

    ACDK_CCT_NVRAM_SET_STRUCT nvram_buff;
    BOOL ret;
    MUINT32 u4RetLen = 0;
    MUINT32 pbuff[sizeof(ACDK_ISP_NVRAM_REG)] = {0}, len;

    nvram_buff.Mode= (CAMERA_NVRAM_STRUCTURE_ENUM)atoi((char *)a_pprArgv[0]);
    nvram_buff.pBuffer = (MUINT32 *)&pbuff[0];

    ret = ::bSendDataToCCT(ACDK_CCT_V2_OP_ISP_GET_NVRAM_DATA, NULL, 0, (UINT8 *)&nvram_buff, sizeof(ACDK_CCT_NVRAM_SET_STRUCT), &len);

    if (!ret)
    {
        printf("ACDK_CCT_OP_ISP_SAVE_TO_NVRAM Fail\n");
        return E_CCT_CCAP_API_FAIL;
    }

    return S_CCT_CCAP_OK;
}


/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_OP_ISP_READ_REG
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPReadISPReg_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (a_u4Argc != 1)
    {
        ACDK_LOGD("Usage: rIspReg <addr>\n");
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_CCT_REG_RW_STRUCT ACDK_reg_read;
    memset(&ACDK_reg_read,0, sizeof(ACDK_CCT_REG_RW_STRUCT));

    MUINT32 u4InRegAddr = 0;

    ACDK_LOGD("ACDK_CCT_OP_ISP_READ_REG\n");

    getHexToken((char *)a_pprArgv[0], &u4InRegAddr);

    ACDK_reg_read.RegAddr	= u4InRegAddr;

    MUINT32 u4RetLen = 0;

    BOOL bRet = bSendDataToCCT(ACDK_CCT_OP_ISP_READ_REG,
                                (UINT8 *)&ACDK_reg_read,
                                sizeof(ACDK_CCT_REG_RW_STRUCT),
                                (UINT8 *)&ACDK_reg_read,
                                sizeof(ACDK_CCT_REG_RW_STRUCT),
                                &u4RetLen);

    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }

    printf("[mrCCAPReadISPReg_Cmd] Addr:0x%X\n", ACDK_reg_read.RegAddr);
    printf("[mrCCAPReadISPReg_Cmd] RegData:0x%X\n", ACDK_reg_read.RegData);

    ACDK_LOGD("Read Addr:0x%X\n", ACDK_reg_read.RegAddr);
    ACDK_LOGD("Read Data:0x%X\n", ACDK_reg_read.RegData);

    return S_CCT_CCAP_OK;

}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_OP_ISP_WRITE_REG
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPWriteISPReg_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (a_u4Argc != 2)
    {
        ACDK_LOGD("Usage: wIspReg <addr> <data>\n");
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_CCT_REG_RW_STRUCT ACDK_reg_write;
    memset(&ACDK_reg_write,0, sizeof(ACDK_CCT_REG_RW_STRUCT));

    MUINT32 u4InRegAddr = 0;
    MUINT32 u4InRegData = 0;

    ACDK_LOGD("ACDK_CCT_OP_ISP_WRITE_REG\n");

    getHexToken((char *)a_pprArgv[0], &u4InRegAddr);
    getHexToken((char *)a_pprArgv[1], &u4InRegData);


    ACDK_reg_write.RegAddr	= u4InRegAddr;
    ACDK_reg_write.RegData = u4InRegData;

    MUINT32 u4RetLen = 0;

    BOOL bRet = bSendDataToCCT(ACDK_CCT_OP_ISP_WRITE_REG,
                                (UINT8 *)&ACDK_reg_write,
                                sizeof(ACDK_CCT_REG_RW_STRUCT),
                                (UINT8 *)&ACDK_reg_write,
                                sizeof(ACDK_CCT_REG_RW_STRUCT),
                                &u4RetLen);

    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("Write Addr:0x%X\n", ACDK_reg_write.RegAddr);
    ACDK_LOGD("Write Data:0x%X\n", ACDK_reg_write.RegData);

    return S_CCT_CCAP_OK;

}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_OP_QUERY_ISP_ID
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPQueryISPID_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_OP_QUERY_ISP_ID\n");

    MUINT32 u4ID = 0;
    MUINT32 u4RetLen = 0;

    BOOL bRet = bSendDataToCCT(ACDK_CCT_OP_QUERY_ISP_ID,
                                NULL,
                                0,
                                (UINT8*)&u4ID,
                                4,
                                &u4RetLen);

    ACDK_LOGD("ISP ID:0x%X\n", u4ID);
    printf("ISP ID:0x%X\n", u4ID);

    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }

    return S_CCT_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_ISP_SET_TUNING_INDEX
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPSetTuningIndex_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
#if 0
    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_ISP_SET_TUNING_INDEX\n");

    ACDK_CCT_ISP_REG_CATEGORY const aryCategory[EIsp_Num_Of_Category] =
    {
        EIsp_Category_OB,
        EIsp_Category_DM,
        EIsp_Category_DP,
        EIsp_Category_NR1,
        EIsp_Category_NR2,
        EIsp_Category_EE,
        EIsp_Category_Saturation,
        EIsp_Category_Contrast,
        EIsp_Category_Hue
    };

    MUINT32 const aryIndex[EIsp_Num_Of_Category] =
    {
        0, 0, 1, 2, 3, 3, 2, 1, 0
    };


    ACDK_CCT_ISP_ACCESS_NVRAM_REG_INDEX acdk_cct_reg_idx;
    ::memset(&acdk_cct_reg_idx, 0, sizeof(acdk_cct_reg_idx));

    for (MUINT32 i = 0; i < EIsp_Num_Of_Category; i++)
    {
        acdk_cct_reg_idx.u4Index    = aryIndex[i];
        acdk_cct_reg_idx.eCategory  = aryCategory[i];
        MBOOL bRet = ::bSendDataToCCT  (
                ACDK_CCT_V2_OP_ISP_SET_TUNING_INDEX,
                (MUINT8*)&acdk_cct_reg_idx, sizeof(ACDK_CCT_ISP_ACCESS_NVRAM_REG_INDEX),
                NULL, 0, NULL
            );
        if  ( ! bRet )
        {
            return E_CCT_CCAP_API_FAIL;
        }
    }
#endif

    return S_CCT_CCAP_OK;

}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_ISP_GET_TUNING_INDEX
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPGetTuningIndex_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
#if 0
    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_ISP_GET_TUNING_INDEX\n");

    ACDK_CCT_ISP_REG_CATEGORY const aryCategory[EIsp_Num_Of_Category] =
    {
        EIsp_Category_OB,
        EIsp_Category_DM,
        EIsp_Category_DP,
        EIsp_Category_NR1,
        EIsp_Category_NR2,
        EIsp_Category_EE,
        EIsp_Category_Saturation,
        EIsp_Category_Contrast,
        EIsp_Category_Hue
    };

    MUINT32 u4RetLen = 0;
    MUINT32 u4Index = 0xFFFFFFFF;

    for (MUINT32 i = 0; i < EIsp_Num_Of_Category; i++)
    {
        ACDK_CCT_ISP_REG_CATEGORY eCategory = aryCategory[i];
        u4Index = 0xFFFFFFFF;
        MBOOL bRet = ::bSendDataToCCT  (
                ACDK_CCT_V2_OP_ISP_GET_TUNING_INDEX,
                &eCategory, sizeof(eCategory),
                &u4Index, sizeof(u4Index),
                &u4RetLen
            );

        printf("Get: (bRet, u4RetLen)=(%d, %d) (Category, Index)=(%d, %d)\n", bRet, u4RetLen, eCategory, u4Index);
        if  ( ! bRet )
        {
            return E_CCT_CCAP_API_FAIL;
        }
    }
#endif
    return S_CCT_CCAP_OK;
}

#if 0
static WINMO_CCT_ISP_TUNING_CMD g_WinMo_CCT_Test_Para =
{
    //NR1
    {
        {1, 8, 4, 4, 120, 6, 9, 14, 1, 2, 3, 4, 5, 6, 7, 8, 2, 3, 4, 5, 6, 7, 8, 9, 3, 4, 5, 6, 7, 8, 9, 10, 4, 5, 6, 7, 8, 9, 10, 11, 1, 2, 7, 9},
        {0, 4, 3, 1, 121, 3, 4, 5, 2, 3, 4, 5, 6, 7, 8, 9, 3, 4, 5, 6, 7, 8, 9, 10, 4, 5, 6, 7, 8, 9, 10, 11, 5, 6, 7, 8, 9, 10, 11, 12, 0, 1, 10, 22},
        {1, 5, 1, 2, 123, 4, 5, 6, 1, 2, 3, 4, 5, 6, 7, 8, 2, 3, 4, 5, 6, 7, 8, 9, 3, 4, 5, 6, 7, 8, 9, 10, 4, 5, 6, 7, 8, 9, 10, 11, 0, 1, 10, 28},
        {0, 3, 2, 3, 124, 5, 6, 7, 2, 3, 4, 5, 6, 7, 8, 9, 3, 4, 5, 6, 7, 8, 9, 10, 4, 5, 6, 7, 8, 9, 10, 11, 5, 6, 7, 8, 9, 10, 11, 12, 0, 1, 11, 27},
        {0, 2, 3, 2, 125, 6, 7, 8, 1, 2, 3, 4, 5, 6, 7, 8, 2, 3, 4, 5, 6, 7, 8, 9, 3, 4, 5, 6, 7, 8, 9, 10, 4, 5, 6, 7, 8, 9, 10, 11, 0, 0, 12, 26},
        {0, 1, 2, 1, 126, 7, 8, 9, 2, 3, 4, 5, 6, 7, 8, 9, 3, 4, 5, 6, 7, 8, 9, 10, 4, 5, 6, 7, 8, 9, 10, 11, 5, 6, 7, 8, 9, 10, 11, 12, 1, 0, 13, 25},
        {1, 0, 1, 4, 127, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 2, 3, 4, 5, 6, 7, 8, 9, 3, 4, 5, 6, 7, 8, 9, 10, 4, 5, 6, 7, 8, 9, 10, 11, 1, 1, 14, 24},
    }
};
#endif


/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_ISP_GET_TUNING_PARAS
/////////////////////////////////////////////////////////////////////////
template <class T, MUINT32 n>
void Dump_ACDK_CCT_ISP_NVRAM_REG(T (& rParams)[n], char const*const pcszTitle)
{
    printf("< %s > \n", pcszTitle);
    for (MUINT32 i = 0; i < n; i++)
    {
        printf(" [%d] \n", i);

        T& rReg = rParams[i];
        for (MUINT32 iSet = 0; iSet < sizeof(rReg.set)/sizeof(MUINT32); iSet++)
        {
            printf("  set[%d]:\t 0x%08X\n", iSet, rReg.set[iSet]);
        }
    }
}

MRESULT mrCCAPGetTuningParas_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{

    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_ISP_GET_TUNING_PARAS\n");

    MUINT32 u4RetLen = 0;
    ACDK_CCT_ISP_GET_TUNING_PARAS   param;
    ::memset(&param, 0, sizeof(param));

    MBOOL bRet = ::bSendDataToCCT(
        ACDK_CCT_V2_OP_ISP_GET_TUNING_PARAS,
        NULL, 0,
        (MUINT8*)&param, sizeof(param),
        &u4RetLen
    );


    ACDK_CCT_ISP_NVRAM_REG& rRegs = param.stIspNvramRegs;

	ACDK_LOGD("param.stIspNvramRegs.OBC[0].offst0.val = %d\n", param.stIspNvramRegs.OBC[0].offst0.val);
	ACDK_LOGD("param.stIspNvramRegs.OBC[1].offst1.val = %d\n", param.stIspNvramRegs.OBC[1].offst1.val);
	ACDK_LOGD("param.stIspNvramRegs.OBC[2].offst2.val = %d\n", param.stIspNvramRegs.OBC[2].offst2.val);
	ACDK_LOGD("param.stIspNvramRegs.OBC[3].offst3.val = %d\n", param.stIspNvramRegs.OBC[3].offst3.val);
	
	ACDK_LOGD("param.stIspNvramRegs.OBC[0].gain0.val = %d\n", param.stIspNvramRegs.OBC[0].gain0.val);
	ACDK_LOGD("param.stIspNvramRegs.OBC[1].gain1.val = %d\n", param.stIspNvramRegs.OBC[1].gain1.val);
	ACDK_LOGD("param.stIspNvramRegs.OBC[2].gain2.val = %d\n", param.stIspNvramRegs.OBC[2].gain2.val);
	ACDK_LOGD("param.stIspNvramRegs.OBC[3].gain3.val = %d\n", param.stIspNvramRegs.OBC[3].gain3.val);	

	ACDK_LOGD("param.stIspNvramRegs.ANR[0].con1.val = %d\n", param.stIspNvramRegs.ANR[0].con1.val);
	ACDK_LOGD("param.stIspNvramRegs.ANR[10].yad1.val = %d\n", param.stIspNvramRegs.ANR[10].yad1.val);
	ACDK_LOGD("param.stIspNvramRegs.ANR[20].lut1.val = %d\n", param.stIspNvramRegs.ANR[20].lut1.val);
	ACDK_LOGD("param.stIspNvramRegs.ANR[30].lce1.val = %d\n", param.stIspNvramRegs.ANR[30].lce1.val);
	ACDK_LOGD("param.stIspNvramRegs.ANR[40].hp1.val = %d\n", param.stIspNvramRegs.ANR[40].hp1.val);
	
	ACDK_LOGD("param.stIspNvramRegs.CCR[0].uvlut.val = %d\n", param.stIspNvramRegs.CCR[0].uvlut.val);
	ACDK_LOGD("param.stIspNvramRegs.CCR[3].uvlut.val = %d\n", param.stIspNvramRegs.CCR[3].uvlut.val);
	ACDK_LOGD("param.stIspNvramRegs.CCR[5].uvlut.val = %d\n", param.stIspNvramRegs.CCR[5].uvlut.val);



    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }

    return S_CCT_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_ISP_SET_TUNING_PARAS
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPSetTuningParas_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_ISP_SET_TUNING_PARAS\n");

    ACDK_CCT_ISP_SET_TUNING_PARAS   param;
    ::memset(&param, 0xFF, sizeof(param));  //  Set all to 0xFFFFFFFF

    MUINT32 u4Category = 0;
    MUINT32 u4Index = 0;

    if  ( a_u4Argc < 2 )
    {
        printf("a_u4Argc(%d) < 2: (please give \"category\" & \"index\")  \n", a_u4Argc);
        return E_CCT_CCAP_API_FAIL;
    }

    ::sscanf((char*)a_pprArgv[0], "%d", &u4Category);
    ::sscanf((char*)a_pprArgv[1], "%d", &u4Index);

    printf("(u4Category, u4Index) = (%d, %d) \n", u4Category, u4Index);
    if  ( EIsp_Num_Of_Category <= u4Category )
        return E_CCT_CCAP_API_FAIL;

    param.u4Index   = u4Index;
    param.eCategory = (ACDK_CCT_ISP_REG_CATEGORY)u4Category;

    MBOOL bRet = ::bSendDataToCCT(
        ACDK_CCT_V2_OP_ISP_SET_TUNING_PARAS,
        (MUINT8*)&param, sizeof(param),
        NULL, 0, NULL
    );

    if (!bRet)
    {
        printf("Fail\n");
        return E_CCT_CCAP_API_FAIL;
    }

    return S_CCT_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// FT_MSDK_CCT_V2_OP_ISP_SET_SHADING_ON_OFF
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPSetShadingOnOff_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("FT_MSDK_CCT_V2_OP_ISP_SET_SHADING_ON_OFF\n");
    if (a_u4Argc != 2)
    {
        ACDK_LOGD("Usage: setShading <Mode: prv:0, cap:1, bin:2 > <On/Off, ON:1, OFF:0>\n");
        return E_CCT_CCAP_API_FAIL;
    }


    ACDK_CCT_MODULE_CTRL_STRUCT ACDK_Data;
    memset(&ACDK_Data,0,sizeof(ACDK_CCT_MODULE_CTRL_STRUCT));

    ACDK_Data.Mode   = (CAMERA_TUNING_SET_ENUM)atoi((char *)a_pprArgv[0]);
    ACDK_Data.Enable = atoi((char *)a_pprArgv[1]);

    MUINT32 u4RetLen = 0;

    BOOL bRet = bSendDataToCCT(ACDK_CCT_V2_OP_ISP_SET_SHADING_ON_OFF,
                                                       (UINT8 *)&ACDK_Data,
                                                       sizeof(ACDK_CCT_MODULE_CTRL_STRUCT),
                                                       NULL,
                                                       0,
                                                       &u4RetLen);

    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }
    ACDK_LOGD("CCAP Set Shading On/Off Success, Mode:%d    On/Off:%d\n", ACDK_Data.Mode,  ACDK_Data.Enable);
    return S_CCT_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// FT_MSDK_CCT_V2_OP_ISP_GET_SHADING_ON_OFF
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPGetShadingOnOff_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("FT_MSDK_CCT_V2_OP_ISP_GET_SHADING_ON_OFF\n");
    if (a_u4Argc != 1)
    {
        ACDK_LOGD("Usage: getShading <Mode: prv:0, cap:1, bin:2 >\n");
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_CCT_MODULE_CTRL_STRUCT ACDK_Data;
    memset(&ACDK_Data,0,sizeof(ACDK_CCT_MODULE_CTRL_STRUCT));

    ACDK_Data.Mode = (CAMERA_TUNING_SET_ENUM)atoi((char *)a_pprArgv[0]);

    MUINT32 u4RetLen = 0;

    BOOL bRet = bSendDataToCCT(ACDK_CCT_V2_OP_ISP_GET_SHADING_ON_OFF,
                                                       (UINT8 *)&ACDK_Data,
                                                       sizeof(ACDK_CCT_MODULE_CTRL_STRUCT),
                                                       (UINT8 *)&ACDK_Data,
                                                       sizeof(ACDK_CCT_MODULE_CTRL_STRUCT),
                                                       &u4RetLen);

    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }
    ACDK_LOGD("CCAP Get Shading On/Off Success, Mode:%d    On/Off:%d\n", ACDK_Data.Mode,  ACDK_Data.Enable);
    return S_CCT_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// FT_MSDK_CCT_V2_OP_ISP_SET_SHADING_PARA
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPSetShadingPara_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("FT_MSDK_CCT_V2_OP_ISP_SET_SHADING_PARA\n");
    ACDK_CCT_SHADING_COMP_STRUCT ACDK_data;
    memset(&ACDK_data,0,sizeof(ACDK_CCT_SHADING_COMP_STRUCT));
    ACDK_data.SHADING_MODE = 0;

    winmo_cct_shading_comp_struct Cct_shading;
    memset(&Cct_shading,0,sizeof(winmo_cct_shading_comp_struct));
    Cct_shading.SDBLK_TRIG = 0;
    Cct_shading.SHADING_EN = 1;
    Cct_shading.SHADINGBLK_XNUM = 7;
    Cct_shading.SHADINGBLK_YNUM = 6;
    Cct_shading.SD_TABLE_SIZE = 1024;
    Cct_shading.SDBLK_RATIO00 = 32;
    Cct_shading.SDBLK_RATIO01 = 32;
    Cct_shading.SDBLK_RATIO10 = 32;
    Cct_shading.SDBLK_RATIO11 = 32;
    Cct_shading.SHADINGBLK_WIDTH = 92;
    Cct_shading.SHADINGBLK_HEIGHT = 81;
    Cct_shading.SD_LWIDTH = 87;
    Cct_shading.SD_LHEIGHT = 72;

    //memcpy(&Cct_shading,(const cct_shading_comp_struct *)&pREQ->cmd.set_shading_para,sizeof(cct_shading_comp_struct));

    ACDK_data.pShadingComp = &Cct_shading;
    MUINT32 u4RetLen = 0;

    BOOL bRet = bSendDataToCCT(ACDK_CCT_V2_OP_ISP_SET_SHADING_PARA,
                                                       (UINT8 *)&ACDK_data,
                                                       sizeof(ACDK_CCT_SHADING_COMP_STRUCT),
                                                       NULL,
                                                       0,
                                                       &u4RetLen);
    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }
    return S_CCT_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// FT_MSDK_CCT_V2_OP_ISP_GET_SHADING_PARA
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPGetShadingPara_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("FT_MSDK_CCT_V2_OP_ISP_GET_SHADING_PARA\n");
    winmo_cct_shading_comp_struct CCT_shading;
    memset(&CCT_shading,0,sizeof(winmo_cct_shading_comp_struct));

    //CAMERA_TUNING_SET_ENUM
    ACDK_CCT_SHADING_COMP_STRUCT ACDK_Output;
    memset(&ACDK_Output,0,sizeof(ACDK_CCT_SHADING_COMP_STRUCT));

    UINT8 uCompMode = CAMERA_TUNING_PREVIEW_SET;
    ACDK_Output.pShadingComp = &CCT_shading;
    MUINT32 u4RetLen = 0;
    BOOL bRet = bSendDataToCCT(ACDK_CCT_V2_OP_ISP_GET_SHADING_PARA,
                                                       (UINT8*) &uCompMode,
                                                       sizeof(UINT8),
                                                       (UINT8*)&ACDK_Output,
                                                       sizeof(ACDK_CCT_SHADING_COMP_STRUCT),
                                                       &u4RetLen);

    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("CAMERA_TUNING_PREVIEW_SET\n");
    ACDK_LOGD("SDBLK_TRIG:%d\n", CCT_shading.SDBLK_TRIG);
    ACDK_LOGD("SHADING_EN:%d\n", CCT_shading.SHADING_EN);
    ACDK_LOGD("SHADINGBLK_XOFFSET:%d\n", CCT_shading.SHADINGBLK_XOFFSET);
    ACDK_LOGD("SHADINGBLK_YOFFSET:%d\n", CCT_shading.SHADINGBLK_YOFFSET);
    ACDK_LOGD("SHADINGBLK_XNUM:%d\n", CCT_shading.SHADINGBLK_XNUM);
    ACDK_LOGD("SHADINGBLK_YNUM:%d\n", CCT_shading.SHADINGBLK_YNUM);
    ACDK_LOGD("SHADINGBLK_WIDTH:%d\n", CCT_shading.SHADINGBLK_WIDTH);
    ACDK_LOGD("SHADINGBLK_HEIGHT:%d\n", CCT_shading.SHADINGBLK_HEIGHT);
    ACDK_LOGD("SHADING_RADDR:%d\n", CCT_shading.SHADING_RADDR);
    ACDK_LOGD("SD_LWIDTH:%d\n", CCT_shading.SD_LWIDTH);
    ACDK_LOGD("SD_LHEIGHT:%d\n", CCT_shading.SD_LHEIGHT);
    ACDK_LOGD("SDBLK_RATIO00:%d\n", CCT_shading.SDBLK_RATIO00);
    ACDK_LOGD("SDBLK_RATIO01:%d\n", CCT_shading.SDBLK_RATIO01);
    ACDK_LOGD("SDBLK_RATIO10:%d\n", CCT_shading.SDBLK_RATIO10);
    ACDK_LOGD("SDBLK_RATIO11:%d\n", CCT_shading.SDBLK_RATIO11);
    ACDK_LOGD("SD_TABLE_SIZE:%d\n",  CCT_shading.SD_TABLE_SIZE);

    uCompMode = CAMERA_TUNING_CAPTURE_SET;
    bRet = bSendDataToCCT(ACDK_CCT_V2_OP_ISP_GET_SHADING_PARA,
                                                       (UINT8*) &uCompMode,
                                                       sizeof(UINT8),
                                                       (UINT8*)&ACDK_Output,
                                                       sizeof(ACDK_CCT_SHADING_COMP_STRUCT),
                                                       &u4RetLen);

    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("CAMERA_TUNING_CAPTURE_SET\n");
    ACDK_LOGD("SDBLK_TRIG:%d\n", CCT_shading.SDBLK_TRIG);
    ACDK_LOGD("SHADING_EN:%d\n", CCT_shading.SHADING_EN);
    ACDK_LOGD("SHADINGBLK_XOFFSET:%d\n", CCT_shading.SHADINGBLK_XOFFSET);
    ACDK_LOGD("SHADINGBLK_YOFFSET:%d\n", CCT_shading.SHADINGBLK_YOFFSET);
    ACDK_LOGD("SHADINGBLK_XNUM:%d\n", CCT_shading.SHADINGBLK_XNUM);
    ACDK_LOGD("SHADINGBLK_YNUM:%d\n", CCT_shading.SHADINGBLK_YNUM);
    ACDK_LOGD("SHADINGBLK_WIDTH:%d\n", CCT_shading.SHADINGBLK_WIDTH);
    ACDK_LOGD("SHADINGBLK_HEIGHT:%d\n", CCT_shading.SHADINGBLK_HEIGHT);
    ACDK_LOGD("SHADING_RADDR:%d\n", CCT_shading.SHADING_RADDR);
    ACDK_LOGD("SD_LWIDTH:%d\n", CCT_shading.SD_LWIDTH);
    ACDK_LOGD("SD_LHEIGHT:%d\n", CCT_shading.SD_LHEIGHT);
    ACDK_LOGD("SDBLK_RATIO00:%d\n", CCT_shading.SDBLK_RATIO00);
    ACDK_LOGD("SDBLK_RATIO01:%d\n", CCT_shading.SDBLK_RATIO01);
    ACDK_LOGD("SDBLK_RATIO10:%d\n", CCT_shading.SDBLK_RATIO10);
    ACDK_LOGD("SDBLK_RATIO11:%d\n", CCT_shading.SDBLK_RATIO11);
    ACDK_LOGD("SD_TABLE_SIZE:%d\n",  CCT_shading.SD_TABLE_SIZE);


    uCompMode = CAMERA_TUNING_VIDEO_SET;
    bRet = bSendDataToCCT(ACDK_CCT_V2_OP_ISP_GET_SHADING_PARA,
            (UINT8*) &uCompMode,
            sizeof(UINT8),
            (UINT8*)&ACDK_Output,
            sizeof(ACDK_CCT_SHADING_COMP_STRUCT),
            &u4RetLen);

    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("CAMERA_TUNING_VIDEO_SET\n");
    ACDK_LOGD("SDBLK_TRIG:%d\n", CCT_shading.SDBLK_TRIG);
    ACDK_LOGD("SHADING_EN:%d\n", CCT_shading.SHADING_EN);
    ACDK_LOGD("SHADINGBLK_XOFFSET:%d\n", CCT_shading.SHADINGBLK_XOFFSET);
    ACDK_LOGD("SHADINGBLK_YOFFSET:%d\n", CCT_shading.SHADINGBLK_YOFFSET);
    ACDK_LOGD("SHADINGBLK_XNUM:%d\n", CCT_shading.SHADINGBLK_XNUM);
    ACDK_LOGD("SHADINGBLK_YNUM:%d\n", CCT_shading.SHADINGBLK_YNUM);
    ACDK_LOGD("SHADINGBLK_WIDTH:%d\n", CCT_shading.SHADINGBLK_WIDTH);
    ACDK_LOGD("SHADINGBLK_HEIGHT:%d\n", CCT_shading.SHADINGBLK_HEIGHT);
    ACDK_LOGD("SHADING_RADDR:%d\n", CCT_shading.SHADING_RADDR);
    ACDK_LOGD("SD_LWIDTH:%d\n", CCT_shading.SD_LWIDTH);
    ACDK_LOGD("SD_LHEIGHT:%d\n", CCT_shading.SD_LHEIGHT);
    ACDK_LOGD("SDBLK_RATIO00:%d\n", CCT_shading.SDBLK_RATIO00);
    ACDK_LOGD("SDBLK_RATIO01:%d\n", CCT_shading.SDBLK_RATIO01);
    ACDK_LOGD("SDBLK_RATIO10:%d\n", CCT_shading.SDBLK_RATIO10);
    ACDK_LOGD("SDBLK_RATIO11:%d\n", CCT_shading.SDBLK_RATIO11);
    ACDK_LOGD("SD_TABLE_SIZE:%d\n",  CCT_shading.SD_TABLE_SIZE);
    return S_CCT_CCAP_OK;
}

MRESULT mrCCAPSetShadingTsfAwbForce_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_ISP_SET_SHADING_TSFAWB_FORCE\n");
    if (a_u4Argc != 1)
    {
        ACDK_LOGD("Usage: setTsfAwbForce <On/Off, ON:1, OFF:0>\n");
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_CCT_FUNCTION_ENABLE_STRUCT cctOnOff;

    cctOnOff.Enable = atoi((char *)a_pprArgv[0]);

    MUINT32 u4RetLen = 0;

    BOOL bRet = bSendDataToCCT(ACDK_CCT_V2_OP_ISP_SET_SHADING_TSFAWB_FORCE,
                                                       (UINT8 *)&cctOnOff,
                                                       sizeof(ACDK_CCT_FUNCTION_ENABLE_STRUCT),
                                                       NULL,
                                                       0,
                                                       &u4RetLen);

    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }
    ACDK_LOGD("CCAP Set TSF AWB Force On/Off Success, Enable:%d\n", cctOnOff.Enable);
    
    return S_CCT_CCAP_OK;
}


/////////////////////////////////////////////////////////////////////////
// FT_MSDK_CCT_V2_ISP_DEFECT_TABLE_ON
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPDefectTblOn_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("FT_MSDK_CCT_V2_ISP_DEFECT_TABLE_ON\n");

    ACDK_CCT_MODULE_CTRL_STRUCT ACDK_MODULE_ctrl_struct;
    memset(&ACDK_MODULE_ctrl_struct,0,sizeof(ACDK_CCT_MODULE_CTRL_STRUCT));

    ACDK_MODULE_ctrl_struct.Enable = MTRUE;
    MUINT32 u4RetLen = 0;

    BOOL bRet = bSendDataToCCT(ACDK_CCT_V2_ISP_DEFECT_TABLE_ON,
                                                       (UINT8 *)&ACDK_MODULE_ctrl_struct,
                                                       sizeof(ACDK_CCT_MODULE_CTRL_STRUCT),
                                                       NULL,
                                                       0,
                                                       &u4RetLen);
    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }
    return S_CCT_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// FT_MSDK_CCT_V2_ISP_DEFECT_TABLE_OFF
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPDefectTblOff_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("FT_MSDK_CCT_V2_ISP_DEFECT_TABLE_OFF\n");

    ACDK_CCT_MODULE_CTRL_STRUCT ACDK_MODULE_ctrl_struct;
    memset(&ACDK_MODULE_ctrl_struct,0,sizeof(ACDK_CCT_MODULE_CTRL_STRUCT));

    ACDK_MODULE_ctrl_struct.Enable = MFALSE;
    MUINT32 u4RetLen = 0;

    BOOL bRet = bSendDataToCCT(ACDK_CCT_V2_ISP_DEFECT_TABLE_OFF,
                                                       (UINT8 *)&ACDK_MODULE_ctrl_struct,
                                                       sizeof(ACDK_CCT_MODULE_CTRL_STRUCT),
                                                       NULL,
                                                       0,
                                                       &u4RetLen);
    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }
    return S_CCT_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// FT_MSDK_CCT_OP_DEFECT_TABLE_CAL
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPDefectTblCal_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }
    ACDK_LOGD("FT_MSDK_CCT_OP_DEFECT_TABLE_CAL\n");
    //ToDo

    return S_CCT_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_ISP_ENABLE_DYNAMIC_BYPASS_MODE
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPEnableDynamicBypass(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_ISP_ENABLE_DYNAMIC_BYPASS_MODE\n");

    MUINT32 u4RetLen = 0;
    BOOL bRet = bSendDataToCCT(ACDK_CCT_V2_OP_ISP_ENABLE_DYNAMIC_BYPASS_MODE,
                                                       NULL,
                                                       0,
                                                       NULL,
                                                       0,
                                                       &u4RetLen);

    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }
    return S_CCT_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// FT_MSDK_CCT_V2_OP_ISP_DISABLE_DYNAMIC_BYPASS_MODE
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPDisableDynamicBypass(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("FT_MSDK_CCT_V2_OP_ISP_DISABLE_DYNAMIC_BYPASS_MODE\n");
    MUINT32 u4RetLen = 0;
    BOOL bRet = bSendDataToCCT(ACDK_CCT_V2_OP_ISP_DISABLE_DYNAMIC_BYPASS_MODE,
                                                       NULL,
                                                       0,
                                                       NULL,
                                                       0,
                                                       &u4RetLen);

    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }
    return S_CCT_CCAP_OK;
}




/////////////////////////////////////////////////////////////////////////
// FT_MSDK_CCT_V2_OP_ISP_GET_DYNAMIC_BYPASS_MODE_ON_OFF
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPGetDynamicMode_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("FT_MSDK_CCT_V2_OP_ISP_GET_DYNAMIC_BYPASS_MODE_ON_OFF\n");
    ACDK_CCT_FUNCTION_ENABLE_STRUCT ACDK_get_bypass_onoff;
    memset(&ACDK_get_bypass_onoff,0,sizeof(ACDK_CCT_FUNCTION_ENABLE_STRUCT));

    MUINT32 u4RetLen = 0;

    BOOL bRet = bSendDataToCCT(ACDK_CCT_V2_OP_ISP_GET_DYNAMIC_BYPASS_MODE_ON_OFF,
                                                        NULL,
                                                        0,
                                                        (UINT8 *)&ACDK_get_bypass_onoff,
                                                        sizeof(ACDK_CCT_FUNCTION_ENABLE_STRUCT),
                                                        &u4RetLen);

    ACDK_LOGD("Dynamic Mode Bypass:%d\n", ACDK_get_bypass_onoff.Enable);

    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }
    return S_CCT_CCAP_OK;
}


/////////////////////////////////////////////////////////////////////////
// FT_MSDK_CCT_V2_OP_ISP_GET_SHADING_TABLE_V3
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPGetShadingTableV3_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }
    ACDK_LOGD("FT_MSDK_CCT_V2_OP_ISP_GET_SHADING_TABLE_V3\n");

    UINT8 uShadingTable[MAX_SVD_SHADING_SIZE];
    memset(uShadingTable, 0, MAX_SVD_SHADING_SIZE);

    ACDK_CCT_TABLE_SET_STRUCT  shadingTable;
    memset (&shadingTable, 0, sizeof(ACDK_CCT_TABLE_SET_STRUCT));
    shadingTable.Length = MAX_SVD_SHADING_SIZE;
    shadingTable.Offset = 0;
    shadingTable.pBuffer = (MUINT32 *)&uShadingTable[0];

    MUINT32 u4RetLen = 0;


    ACDK_LOGD("Get Shading Preview Table \n");
    for (MINT32 i = 0; i < 3; i++)
    {
        shadingTable.Mode = CAMERA_TUNING_PREVIEW_SET;
        shadingTable.ColorTemp = i;
         BOOL bRet = bSendDataToCCT(ACDK_CCT_V2_OP_ISP_GET_SHADING_TABLE_V3,
                                                            NULL,
                                                            0,
                                                            (UINT8 *)&shadingTable,
                                                            sizeof(ACDK_CCT_TABLE_SET_STRUCT),
                                                            &u4RetLen);

        if (!bRet)
        {
            ACDK_LOGD("Get Shading Preview Table Fail, Color Temp:%d\n", i);
            return E_CCT_CCAP_API_FAIL;
        }
        ACDK_LOGD("Get Shading Preview Table_ColorTemp:%d\n", i);
        for (MINT32 j = 0; j  < 675; j++)
        {
            printf("%02x", uShadingTable[j]);
        }
        printf("\n\n");
    }

    ACDK_LOGD("Get Shading Capture Table \n");
    for (MINT32 i = 0; i < 3; i++)
    {
        shadingTable.Mode = CAMERA_TUNING_CAPTURE_SET;
        shadingTable.ColorTemp = i;
         BOOL bRet = bSendDataToCCT(ACDK_CCT_V2_OP_ISP_GET_SHADING_TABLE_V3,
                                                            NULL,
                                                            0,
                                                            (UINT8 *)&shadingTable,
                                                            sizeof(ACDK_CCT_TABLE_SET_STRUCT),
                                                            &u4RetLen);

        if (!bRet)
        {
            ACDK_LOGD("Get Shading Capture Table Fail, Color Temp:%d\n", i);
            return E_CCT_CCAP_API_FAIL;
        }
        ACDK_LOGD("Get Shading Capture Table_ColorTemp:%d\n", i);
        for (MINT32 j = 0; j  < 675; j++)
        {
            printf("%02x", uShadingTable[j]);
        }
        printf("\n\n");
    }
    return S_CCT_CCAP_OK;
}


/////////////////////////////////////////////////////////////////////////
// FT_MSDK_CCT_V2_OP_ISP_SET_SHADING_TABLE_V3
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPSetShadingTableV3_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("FT_MSDK_CCT_V2_OP_ISP_SET_SHADING_TABLE_V3\n");

    // this table is for ov5642 capture , 15x15 blocks setting
    // preview should show some artifact after apply this table.
    UINT8 uShadingTable[MAX_SVD_SHADING_SIZE] ={
                0x3B, 0xF0, 0x4F, 0xF0, 0x43, 0x00, 0x59, 0x00,
                0x07, 0x08,
                0x89, 0x00, 0x00, 0x00, 0x02, 0x3C, 0x00, 0x00, 0x00, 0x02,
                0xEB, 0xFF, 0xFF, 0xFF, 0x02, 0xF2, 0xFF, 0xFF, 0xFF, 0x03,
                0xED, 0xFF, 0xFF, 0xFF, 0x02, 0xF2, 0xFF, 0xFF, 0xFF, 0x03,
                0xE5, 0xFF, 0xFF, 0xFF, 0x02, 0xF8, 0xFF, 0xFF, 0xFF, 0x03,
                0xF2, 0xFF, 0xFF, 0xFF, 0x03, 0xF5, 0xFF, 0xFF, 0xFF, 0x03,
                0xEA, 0xFF, 0xFF, 0xFF, 0x03, 0xF7, 0xFF, 0xFF, 0xFF, 0x03,
                0xEF, 0xFF, 0xFF, 0xFF, 0x03, 0xF8, 0xFF, 0xFF, 0xFF, 0x03,
                0xDE, 0xB4, 0x3D, 0xD6, 0x88, 0xDC, 0xF4,
                0xB5, 0xAC, 0x70, 0x99, 0x08, 0xDC, 0x05,
                0x8A, 0x96, 0x83, 0x5B, 0x5E, 0x06, 0xA0,
                0x63, 0x6A, 0x83, 0x5F, 0xBC, 0xA0, 0x63,
                0x42, 0x4C, 0x79, 0x54, 0xC3, 0xB7, 0x95,
                0x27, 0x34, 0x7A, 0x6D, 0x99, 0xD6, 0x9B,
                0x13, 0x20, 0x75, 0x57, 0x92, 0xF8, 0x79,
                0x09, 0x12, 0x6D, 0x54, 0x62, 0xCF, 0xA0,
                0x05, 0x01, 0x54, 0x6D, 0x4D, 0xB6, 0xA8,
                0x02, 0x03, 0x48, 0x71, 0x42, 0xBC, 0x91,
                0x01, 0x03, 0x33, 0x7B, 0x45, 0x99, 0x7F,
                0x07, 0x10, 0x2C, 0x7F, 0x3A, 0x80, 0x9D,
                0x15, 0x29, 0x2A, 0x7E, 0x4D, 0x93, 0x80,
                0x2B, 0x3F, 0x17, 0x71, 0x74, 0x94, 0x53,
                0x46, 0x5E, 0x0A, 0x65, 0xCE, 0xB1, 0x51,
                0x61, 0x7C, 0x02, 0x4B, 0xCA, 0xAB, 0x7F,
                0x70, 0xB6, 0x2C, 0x03, 0x04, 0xE6, 0xC0,
                0x96, 0x9F, 0x9F, 0xAD, 0x73, 0x80, 0x80, 0x8D, 0x51, 0x5E, 0x5E, 0x69, 0x38, 0x43, 0x43, 0x4B, 0x23, 0x2E, 0x2E, 0x33, 0x15, 0x1D, 0x1D, 0x21, 0x0B, 0x12, 0x12, 0x15, 0x05, 0x0A, 0x0A, 0x0D, 0x03, 0x07, 0x07, 0x0A, 0x05, 0x08, 0x08, 0x0D, 0x0B, 0x0F, 0x0F, 0x14, 0x16, 0x1A, 0x1A, 0x20, 0x25, 0x29, 0x29, 0x32, 0x3A, 0x3F, 0x3E, 0x4B, 0x54, 0x58, 0x58, 0x69, 0x76, 0x7B, 0x7A, 0x90, 0x9A, 0x9C, 0x9C, 0xB7,
                0x0E, 0x15, 0x0C, 0x6E, 0x1B, 0x24, 0x23, 0x71, 0x0F, 0x28, 0x2C, 0x7A, 0x07, 0x3A, 0x3E, 0x90, 0x1D, 0x58, 0x5D, 0xAB, 0x38, 0x7D, 0x7D, 0xC6, 0x66, 0xA5, 0xA2, 0xE9, 0x77, 0xBC, 0xBC, 0xEF, 0x84, 0xC4, 0xC5, 0xF1, 0x81, 0xC1, 0xC2, 0xF3, 0x77, 0xAF, 0xB3, 0xE1, 0x60, 0x9C, 0x9D, 0xD0, 0x5B, 0x6F, 0x75, 0xBF, 0x52, 0x61, 0x69, 0xA5, 0x55, 0x3F, 0x46, 0x7E, 0x59, 0x47, 0x44, 0x6C, 0x6F, 0x53, 0x53, 0x79,
                0xA3, 0xB5, 0xBB, 0xB2, 0x81, 0x96, 0xA9, 0x72, 0x68, 0x80, 0x8D, 0x5A, 0x51, 0x92, 0x92, 0x5B, 0x46, 0x9C, 0x9F, 0x6D, 0x41, 0x99, 0x99, 0x77, 0x41, 0x99, 0x9A, 0x73, 0x39, 0x95, 0x98, 0x71, 0x44, 0x9B, 0xA0, 0x6A, 0x37, 0x92, 0x96, 0x64, 0x39, 0x8A, 0x88, 0x66, 0x2F, 0x80, 0x7D, 0x56, 0x16, 0x6B, 0x6D, 0x49, 0x01, 0x58, 0x69, 0x3D, 0x00, 0x61, 0x5D, 0x2E, 0x1F, 0x70, 0x77, 0x59, 0x53, 0x89, 0x87, 0x67,
                0x17, 0x38, 0x35, 0x47, 0x09, 0x16, 0x1B, 0x30, 0x24, 0x2F, 0x29, 0x0B, 0x23, 0x20, 0x21, 0x16, 0x2D, 0x26, 0x26, 0x08, 0x31, 0x34, 0x34, 0x15, 0x3D, 0x47, 0x49, 0x19, 0x40, 0x3E, 0x3C, 0x34, 0x46, 0x5A, 0x59, 0x2D, 0x42, 0x4E, 0x4D, 0x2E, 0x48, 0x5A, 0x57, 0x31, 0x41, 0x46, 0x47, 0x13, 0x3B, 0x40, 0x3F, 0x1F, 0x48, 0x3D, 0x47, 0x02, 0x3F, 0x45, 0x45, 0x0E, 0x74, 0x4E, 0x5D, 0x3E, 0xDB, 0xBD, 0xBF, 0xB6,
                0x9A, 0x63, 0x61, 0x3C, 0x44, 0x52, 0x44, 0x0D, 0x80, 0x52, 0x59, 0x38, 0x81, 0x59, 0x5D, 0x39, 0x79, 0x54, 0x55, 0x41, 0x7C, 0x6B, 0x6B, 0x40, 0x8E, 0x73, 0x6F, 0x5F, 0x84, 0x73, 0x79, 0x4C, 0x74, 0x79, 0x7E, 0x5D, 0x7C, 0x6E, 0x73, 0x5C, 0x86, 0x63, 0x71, 0x65, 0x6E, 0x60, 0x5C, 0x44, 0x7D, 0x4A, 0x51, 0x4F, 0x51, 0x57, 0x40, 0x2E, 0x5D, 0x4C, 0x4C, 0x35, 0x5B, 0x29, 0x4C, 0x03, 0x82, 0x48, 0x39, 0x02,
                0x2B, 0x24, 0x35, 0x06, 0x4A, 0x4C, 0x51, 0x83, 0x4D, 0x56, 0x37, 0x4E, 0x86, 0x58, 0x58, 0x63, 0x72, 0x6A, 0x6A, 0x6A, 0x5A, 0x5A, 0x5A, 0x66, 0x51, 0x53, 0x52, 0x43, 0x66, 0x33, 0x33, 0x59, 0x42, 0x53, 0x52, 0x3B, 0x3C, 0x46, 0x45, 0x39, 0x46, 0x4B, 0x49, 0x4D, 0x1D, 0x1A, 0x1D, 0x2F, 0x33, 0x4C, 0x50, 0x5D, 0x57, 0x32, 0x39, 0x67, 0x20, 0x1F, 0x27, 0x32, 0x3D, 0x20, 0x4B, 0x10, 0x5F, 0x53, 0x5C, 0x6F,
                0x55, 0x40, 0x3F, 0x8A, 0x2E, 0x05, 0x11, 0x25, 0x39, 0x52, 0x5A, 0x4F, 0x42, 0x4F, 0x4D, 0x4A, 0x61, 0x57, 0x5D, 0x29, 0x59, 0x3A, 0x3A, 0x4B, 0x5B, 0x29, 0x30, 0x53, 0x36, 0x3C, 0x3D, 0x2C, 0x11, 0x38, 0x34, 0x34, 0x3E, 0x35, 0x31, 0x40, 0x36, 0x35, 0x2D, 0x3F, 0x50, 0x44, 0x49, 0x62, 0x1E, 0x37, 0x33, 0x64, 0x35, 0x49, 0x3A, 0x4A, 0x49, 0x45, 0x47, 0x60, 0x06, 0x1E, 0x09, 0x19, 0x66, 0x34, 0x4A, 0x70
        };

    ACDK_CCT_TABLE_SET_STRUCT  shadingTable;
    memset (&shadingTable, 0, sizeof(ACDK_CCT_TABLE_SET_STRUCT));
    shadingTable.Length = MAX_SVD_SHADING_SIZE;
    shadingTable.Offset = 0;
    shadingTable.pBuffer = (MUINT32 *)&uShadingTable[0];

    MUINT32 u4RetLen = 0;

    ACDK_LOGD("Set Shading Preview Table \n");
    for (MINT32 i = 0; i < 3; i++)
    {
        shadingTable.Mode = CAMERA_TUNING_PREVIEW_SET;
        shadingTable.ColorTemp = i;
         BOOL bRet = bSendDataToCCT(ACDK_CCT_V2_OP_ISP_SET_SHADING_TABLE_V3,
                                                             (UINT8 *)&shadingTable,
                                                            sizeof(ACDK_CCT_TABLE_SET_STRUCT),
                                                            NULL,
                                                            0,
                                                            &u4RetLen);

        if (!bRet)
        {
            ACDK_LOGD("Set Shading Preview Table Fail, Color Temp:%d\n", i);
            return E_CCT_CCAP_API_FAIL;
        }
    }

    ACDK_LOGD("Set Shading Capture Table \n");
    for (MINT32 i = 0; i < 3; i++)
    {
        shadingTable.Mode = CAMERA_TUNING_CAPTURE_SET;
        shadingTable.ColorTemp = i;
         BOOL bRet = bSendDataToCCT(ACDK_CCT_V2_OP_ISP_SET_SHADING_TABLE_V3,
                                                             (UINT8 *)&shadingTable,
                                                            sizeof(ACDK_CCT_TABLE_SET_STRUCT),
                                                            NULL,
                                                            0,
                                                            &u4RetLen);

        if (!bRet)
        {
            ACDK_LOGD("Set Shading Capture Table Fail, Color Temp:%d\n", i);
            return E_CCT_CCAP_API_FAIL;
        }
    }
    return S_CCT_CCAP_OK;

}

/////////////////////////////////////////////////////////////////////////
// FT_MSDK_CCT_V2_OP_ISP_GET_SHADING_TABLE_POLYCOEF
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPGetShadingTablePOLYCOEF_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }
    ACDK_LOGD("ACDK_CCT_V2_OP_ISP_GET_SHADING_TABLE_POLYCOEF\n");

    UINT32 uShadingTable[MAX_SHADING_Capture_SIZE] = {0};

    ACDK_CCT_TABLE_SET_STRUCT  shadingTable;
    memset (&shadingTable, 0, sizeof(ACDK_CCT_TABLE_SET_STRUCT));
    shadingTable.Length = MAX_SHADING_Preview_SIZE;
    shadingTable.Offset = 0;
    shadingTable.pBuffer = (MUINT32 *)&uShadingTable[0];

    MUINT32 u4RetLen = 0;


    ACDK_LOGD("Get Shading Preview Table Poly Coef\n");
    for (MINT32 i = 0; i < 3; i++)
    {
        shadingTable.Mode = CAMERA_TUNING_PREVIEW_SET;
        shadingTable.ColorTemp = i;
         BOOL bRet = bSendDataToCCT(ACDK_CCT_V2_OP_ISP_GET_SHADING_TABLE_POLYCOEF,
                                                            NULL,
                                                            0,
                                                            (UINT8 *)&shadingTable,
                                                            sizeof(ACDK_CCT_TABLE_SET_STRUCT),
                                                            &u4RetLen);

        if (!bRet)
        {
            ACDK_LOGD("Get Shading Preview Table Poly Coef Fail, Color Temp:%d\n", i);
            return E_CCT_CCAP_API_FAIL;
        }
        ACDK_LOGD("Get Shading Preview Table_ColorTemp:%d\n", i);
        for (MINT32 j = 0; j  < MAX_SHADING_Preview_SIZE/8; j++)
        {
            printf("%08x", uShadingTable[j]);
        }
        printf("\n\n");
    }

    ACDK_LOGD("Get Shading Capture Table Poly Coef \n");
    shadingTable.Length = MAX_SHADING_Capture_SIZE;

    for (MINT32 i = 0; i < 3; i++)
    {
        shadingTable.Mode = CAMERA_TUNING_CAPTURE_SET;
        shadingTable.ColorTemp = i;
         BOOL bRet = bSendDataToCCT(ACDK_CCT_V2_OP_ISP_GET_SHADING_TABLE_POLYCOEF,
                                                            NULL,
                                                            0,
                                                            (UINT8 *)&shadingTable,
                                                            sizeof(ACDK_CCT_TABLE_SET_STRUCT),
                                                            &u4RetLen);

        if (!bRet)
        {
            ACDK_LOGD("Get Shading Capture Table Poly Coef Fail, Color Temp:%d\n", i);
            return E_CCT_CCAP_API_FAIL;
        }
        ACDK_LOGD("Get Shading Capture Table_ColorTemp:%d\n", i);
        for (MINT32 j = 0; j  < MAX_SHADING_Capture_SIZE/8; j++)
        {
            printf("%08x", uShadingTable[j]);
        }
        printf("\n\n");
    }
    return S_CCT_CCAP_OK;
}


/////////////////////////////////////////////////////////////////////////
// FT_MSDK_CCT_V2_OP_ISP_SET_SHADING_TABLE_POLYCOEF
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPSetShadingTablePOLYCOEF_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_ISP_SET_SHADING_TABLE_POLYCOEF\n");

    // this table is for ov5642 capture , 15x15 blocks setting
    // preview should show some artifact after apply this table.
    UINT32 uShadingTable[MAX_SHADING_Capture_SIZE] ={0};

    ACDK_CCT_TABLE_SET_STRUCT  shadingTable;
    memset (&shadingTable, 0, sizeof(ACDK_CCT_TABLE_SET_STRUCT));
    shadingTable.Length = MAX_SHADING_Preview_SIZE;
    shadingTable.Offset = 0;
    shadingTable.pBuffer = (MUINT32 *)&uShadingTable[0];

    MUINT32 u4RetLen = 0;

    ACDK_LOGD("Set Shading Preview Table Poly Coef\n");
    for (MINT32 i = 0; i < 3; i++)
    {
        shadingTable.Mode = CAMERA_TUNING_PREVIEW_SET;
        shadingTable.ColorTemp = i;
         BOOL bRet = bSendDataToCCT(ACDK_CCT_V2_OP_ISP_SET_SHADING_TABLE_POLYCOEF,
                                                             (UINT8 *)&shadingTable,
                                                            sizeof(ACDK_CCT_TABLE_SET_STRUCT),
                                                            NULL,
                                                            0,
                                                            &u4RetLen);

        if (!bRet)
        {
            ACDK_LOGD("Set Shading Preview Table Poly Coef Fail, Color Temp:%d\n", i);
            return E_CCT_CCAP_API_FAIL;
        }
    }

    ACDK_LOGD("Set Shading Capture Table Poly Coef\n");
    shadingTable.Length = MAX_SHADING_Capture_SIZE;
    for (MINT32 i = 0; i < 3; i++)
    {
        shadingTable.Mode = CAMERA_TUNING_CAPTURE_SET;
        shadingTable.ColorTemp = i;
         BOOL bRet = bSendDataToCCT(ACDK_CCT_V2_OP_ISP_SET_SHADING_TABLE_POLYCOEF,
                                                             (UINT8 *)&shadingTable,
                                                            sizeof(ACDK_CCT_TABLE_SET_STRUCT),
                                                            NULL,
                                                            0,
                                                            &u4RetLen);

        if (!bRet)
        {
            ACDK_LOGD("Set Shading Capture Table Fail, Color Temp:%d\n", i);
            return E_CCT_CCAP_API_FAIL;
        }
    }
    return S_CCT_CCAP_OK;

}


/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_ISP_GET_SHADING_NVRAM
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPGetShadingNVRAM_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }
    ACDK_LOGD("ACDK_CCT_V2_OP_ISP_GET_SHADING_NVRAM\n");

    UINT32 uShadingSTR[sizeof(ISP_SHADING_STRUCT)] = {0};
    ISP_SHADING_STRUCT * pshadistr;

    ACDK_CCT_NVRAM_SET_STRUCT  ShadingData;
    memset (&ShadingData, 0, sizeof(ACDK_CCT_NVRAM_SET_STRUCT));
    ShadingData.Mode = CAMERA_NVRAM_SHADING_STRUCT;
    ShadingData.pBuffer = (MUINT32 *)&uShadingSTR[0];

    MUINT32 u4RetLen = 0;

    BOOL bRet = bSendDataToCCT(ACDK_CCT_V2_OP_ISP_GET_NVRAM_DATA,
                                                        NULL,
                                                        0,
                                                        (UINT8 *)&ShadingData,
                                                        sizeof(ACDK_CCT_NVRAM_SET_STRUCT),
                                                        &u4RetLen);

    if (!bRet)
    {
          ACDK_LOGD("Get Shading NVRAM data Fail\n");
        return E_CCT_CCAP_API_FAIL;
    }

    pshadistr =  reinterpret_cast<ISP_SHADING_STRUCT*> (ShadingData.pBuffer) ;

    ACDK_LOGD("Buffer size :%d\n", sizeof(ISP_SHADING_STRUCT));
    ACDK_LOGD("PreviewSize :%d\n", pshadistr->LSCSize[0]);
    ACDK_LOGD("CaptureSize :%d\n", pshadistr->LSCSize[2]);
    ACDK_LOGD("Pre SVD Size :%d\n", pshadistr->PreviewSVDSize);
    ACDK_LOGD("Cap SVD Size :%d\n", pshadistr->CaptureSVDSize);
    ACDK_LOGD("Data Size  :%d\n", u4RetLen);

    return S_CCT_CCAP_OK;

}


/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_SHADING_CAL
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPCalShading_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_SHADING_CAL\n");
    if (a_u4Argc != 2)
    {
        ACDK_LOGD("Usage: calShading <mode, prv:0, cap:1> <index: 0::2900K, 1:4000K, 2:6300K>\n");
        return E_CCT_CCAP_API_FAIL;
    }


    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================

    MUINT32 u4RetLen = 0;
    ACDK_CCT_LSC_CAL_SET_STRUCT rLSCCalSet;

    rLSCCalSet.mode = (CAMERA_TUNING_SET_ENUM)(UINT8)atoi((char *)a_pprArgv[0]);
    rLSCCalSet.colorTemp = (UINT8) atoi((char *)a_pprArgv[1]);
    rLSCCalSet.boundaryEndX = 40;
    rLSCCalSet.boundaryEndY = 40;
    rLSCCalSet.boundaryStartX = 40;
    rLSCCalSet.boundaryStartY = 40;
    rLSCCalSet.attnRatio = 1;
    rLSCCalSet.u1FixShadingIndex = 0;

    ACDK_LOGD("rLSCCalSet.mode :%d\n", rLSCCalSet.mode);
    ACDK_LOGD("rLSCCalSet.colorTemp  :%d\n", rLSCCalSet.colorTemp);

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToCCT(ACDK_CCT_V2_OP_SHADING_CAL,
                                                        (UINT8 *)&rLSCCalSet,
                                                        sizeof(ACDK_CCT_LSC_CAL_SET_STRUCT),
                                                        NULL,
                                                        0,
                                                        &u4RetLen);

    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================
    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }

    return S_CCT_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_SHADING_VERIFY
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPVerifyShading_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    return S_CCT_CCAP_OK;
}


/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_DEFECT_VERIFY
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPVerifyDefect_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    return S_CCT_CCAP_OK;
}


/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_OP_READ_SENSOR_REG
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPReadSensorReg_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (a_u4Argc != 1)
    {
        ACDK_LOGD("Usage: rsensorReg <addr>\n");
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_CCT_REG_RW_STRUCT ACDK_reg_read;
    memset(&ACDK_reg_read,0, sizeof(ACDK_CCT_REG_RW_STRUCT));

    MUINT32 u4InRegAddr = 0;

    ACDK_LOGD("ACDK_CCT_OP_READ_SENSOR_REG\n");

    getHexToken((char *)a_pprArgv[0], &u4InRegAddr);

    ACDK_reg_read.RegAddr	= u4InRegAddr;

    MUINT32 u4RetLen = 0;

    BOOL bRet = bSendDataToCCT(ACDK_CCT_OP_READ_SENSOR_REG,
                                (UINT8 *)&ACDK_reg_read,
                                sizeof(ACDK_CCT_REG_RW_STRUCT),
                                (UINT8 *)&ACDK_reg_read,
                                sizeof(ACDK_CCT_REG_RW_STRUCT),
                                &u4RetLen);

    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("Read Addr:0x%X\n", ACDK_reg_read.RegAddr);
    ACDK_LOGD("Read Data:0x%X\n", ACDK_reg_read.RegData);

    return S_CCT_CCAP_OK;

}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_OP_WRITE_SENSOR_REG
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPWriteSensorReg_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (a_u4Argc != 2)
    {
        ACDK_LOGD("Usage: wsensorReg <addr> <data>\n");
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_CCT_REG_RW_STRUCT ACDK_reg_write;
    memset(&ACDK_reg_write,0, sizeof(ACDK_CCT_REG_RW_STRUCT));

    MUINT32 u4InRegAddr = 0;
    MUINT32 u4InRegData = 0;

    ACDK_LOGD("ACDK_CCT_OP_WRITE_SENSOR_REG\n");

    getHexToken((char *)a_pprArgv[0], &u4InRegAddr);
    getHexToken((char *)a_pprArgv[1], &u4InRegData);


    ACDK_reg_write.RegAddr	= u4InRegAddr;
    ACDK_reg_write.RegData = u4InRegData;

    MUINT32 u4RetLen = 0;

    BOOL bRet = bSendDataToCCT(ACDK_CCT_OP_WRITE_SENSOR_REG,
                                (UINT8 *)&ACDK_reg_write,
                                sizeof(ACDK_CCT_REG_RW_STRUCT),
                                (UINT8 *)&ACDK_reg_write,
                                sizeof(ACDK_CCT_REG_RW_STRUCT),
                                &u4RetLen);

    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("Write Addr:0x%X\n", ACDK_reg_write.RegAddr);
    ACDK_LOGD("Write Data:0x%X\n", ACDK_reg_write.RegData);

    return S_CCT_CCAP_OK;

}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_GET_SENSOR_RESOLUTION
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPGetSensorRes_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_CCT_SENSOR_RESOLUTION_STRUCT  SensorResolution;

    memset(&SensorResolution,0,sizeof(ACDK_CCT_SENSOR_RESOLUTION_STRUCT));

    MUINT32 u4RetLen = 0;

    ACDK_LOGD("ACDK_CCT_V2_OP_GET_SENSOR_RESOLUTION\n");

    BOOL bRet = bSendDataToCCT(ACDK_CCT_V2_OP_GET_SENSOR_RESOLUTION,
                                                        NULL,
                                                        0,
                                                        (UINT8 *)&SensorResolution,
                                                        sizeof(ACDK_CCT_SENSOR_RESOLUTION_STRUCT),
                                                        &u4RetLen);

    if (!bRet)
    {
        ACDK_LOGE("Get Sensor Resolution Fail \n");
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("Sensor Resolution:\n");
    ACDK_LOGD("Preview Width:%d\n", SensorResolution.SensorPreviewWidth);
    ACDK_LOGD("Preview Height:%d\n", SensorResolution.SensorPreviewHeight);
    ACDK_LOGD("Full Width:%d\n", SensorResolution.SensorFullWidth);
    ACDK_LOGD("Full Height:%d\n", SensorResolution.SensorFullHeight);
    return S_CCT_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_OP_GET_LSC_SENSOR_RESOLUTION
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPGetLSCSensorRes_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_CCT_SENSOR_RESOLUTION_STRUCT  SensorResolution;

    memset(&SensorResolution,0,sizeof(ACDK_CCT_SENSOR_RESOLUTION_STRUCT));

    MUINT32 u4RetLen = 0;

    ACDK_LOGD("ACDK_CCT_OP_GET_LSC_SENSOR_RESOLUTION\n");

    BOOL bRet = bSendDataToCCT(ACDK_CCT_OP_GET_LSC_SENSOR_RESOLUTION,
                                                        NULL,
                                                        0,
                                                        (UINT8 *)&SensorResolution,
                                                        sizeof(ACDK_CCT_SENSOR_RESOLUTION_STRUCT),
                                                        &u4RetLen);

    if (!bRet)
    {
        ACDK_LOGE("Get Sensor Resolution Fail \n");
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("LSC Sensor Resolution:\n");
    ACDK_LOGD("Preview Width:%d\n", SensorResolution.SensorPreviewWidth);
    ACDK_LOGD("Preview Height:%d\n", SensorResolution.SensorPreviewHeight);
    ACDK_LOGD("Full Width:%d\n", SensorResolution.SensorFullWidth);
    ACDK_LOGD("Full Height:%d\n", SensorResolution.SensorFullHeight);
    return S_CCT_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_OP_QUERY_SENSOR
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPQuerySensor_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_CCT_SENSOR_INFO_STRUCT ACDK_Sensor;

    memset(&ACDK_Sensor,0,sizeof(ACDK_CCT_SENSOR_INFO_STRUCT));

    MUINT32 u4RetLen = 0;

    ACDK_LOGD("ACDK_CCT_OP_QUERY_SENSOR\n");

    BOOL bRet = bSendDataToCCT(ACDK_CCT_OP_QUERY_SENSOR,
                                                        NULL,
                                                        0,
                                                        (UINT8 *)&ACDK_Sensor,
                                                        sizeof(ACDK_CCT_SENSOR_INFO_STRUCT),
                                                        &u4RetLen);


    if (!bRet)
    {
        ACDK_LOGE("CCAP Query Sensor Fail \n");
        return E_CCT_CCAP_API_FAIL;
    }


    ACDK_LOGD("Query Sensor Info:\n");
    ACDK_LOGD("Type:%d\n", ACDK_Sensor.Type);
    ACDK_LOGD("DeviceId:%d\n", ACDK_Sensor.DeviceId);
    ACDK_LOGD("StartPixelBayerPtn:%d\n", ACDK_Sensor.StartPixelBayerPtn);
    ACDK_LOGD("GrabXOffset:%d\n", ACDK_Sensor.GrabXOffset);
    ACDK_LOGD("GrabYOffset:%d\n", ACDK_Sensor.GrabYOffset);

    return S_CCT_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// MSDK_CCT_OP_GET_ENG_SENSOR_PARA
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPGetEngSensorPara_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    //!
    //! Please ref mrCCAPGetSensorPreGain_Cmd();
    //!

    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("Please ref mrCCAPGetSensorPreGain_Cmd(); \n");

    return S_CCT_CCAP_OK;
}


/////////////////////////////////////////////////////////////////////////
// MSDK_CCT_OP_SET_ENG_SENSOR_PARA
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPSetEngSensorPara_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    //!
    //! Please ref mrCCAPSetSensorPreGain_Cmd();
    //!

    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("Please ref mrCCAPSetSensorPreGain_Cmd(); \n");
    return S_CCT_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// MSDK_CCT_OP_GET_ENG_SENSOR_GROUP_PARA
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPGetEngSensorGroupPara_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("MSDK_CCT_OP_GET_ENG_SENSOR_GROUP_PARA\n");

    UINT8 group_name[64] = {0};
    ACDK_SENSOR_GROUP_INFO_STRUCT ACDK_GroupName;
    memset(&ACDK_GroupName,0,sizeof(ACDK_SENSOR_GROUP_INFO_STRUCT));
    ACDK_GroupName.GroupNamePtr = group_name;

    MUINT32 nIndate = 0;
    MUINT32 u4RetLen = 0;

    BOOL bRet = bSendDataToCCT(ACDK_CCT_OP_GET_ENG_SENSOR_GROUP_PARA,
                                                       (UINT8 *)&nIndate,
                                                       sizeof(MUINT32),
                                                       (UINT8 *)&ACDK_GroupName,
                                                       sizeof(ACDK_SENSOR_GROUP_INFO_STRUCT),
                                                       &u4RetLen);

    if (!bRet)
    {
        ACDK_LOGE("CCAP Get Eng Sensor Group Para Fail \n");
        return E_CCT_CCAP_API_FAIL;
    }

	ACDK_LOGD("ACDK_GroupName.GroupIdx = %d\n", ACDK_GroupName.GroupIdx);
	ACDK_LOGD("ACDK_GroupName.ItemCount = %d\n", ACDK_GroupName.ItemCount);

    return S_CCT_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_OP_GET_ENG_SENSOR_GROUP_COUNT
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPGetEngSensorGroupCnt_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_OP_GET_ENG_SENSOR_GROUP_COUNT\n");
    MUINT32 uGroupCnt = 0;

    MUINT32 u4RetLen = 0;

    BOOL bRet = bSendDataToCCT(ACDK_CCT_OP_GET_ENG_SENSOR_GROUP_COUNT,
                                                        NULL,
                                                        0,
                                                        (UINT8 *)&uGroupCnt,
                                                        sizeof(MUINT32),
                                                        &u4RetLen);


    if (!bRet)
    {
        ACDK_LOGE("Get Sensor Group Count Fail \n");
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("Get Sensor Group Count:%d\n", uGroupCnt);

    return S_CCT_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// FT_MSDK_CCT_OP_GET_SENSOR_PREGAIN
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPGetSensorPreGain_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("FT_MSDK_CCT_OP_GET_SENSOR_PREGAIN\n");


    ACDK_SENSOR_ITEM_INFO_STRUCT ACDK_Data;
    memset(&ACDK_Data,0,sizeof(ACDK_SENSOR_ITEM_INFO_STRUCT));

    MUINT32 u4RetLen = 0;
    BOOL bRet = MTRUE;

    //Pregain R
    ACDK_Data.GroupIdx = 0;           //g_FT_CCT_StateMachine.sensor_eng_group_idx; PRE_GAIN
    ACDK_Data.ItemIdx = 1;             //g_FT_CCT_StateMachine.sensor_eng_item_idx_pregain_R

    bRet &= bSendDataToCCT(ACDK_CCT_OP_GET_ENG_SENSOR_PARA,
                                                (UINT8*)&ACDK_Data,
                                                sizeof(ACDK_SENSOR_ITEM_INFO_STRUCT),
                                                (UINT8*)&ACDK_Data,
                                                 sizeof(ACDK_SENSOR_ITEM_INFO_STRUCT),
                                                &u4RetLen);

    ACDK_LOGD("CCAP GET_SENSOR_PREGAIN_R:%d, %d, %d\n", ACDK_Data.ItemValue,
                                                                                                  ACDK_Data.Min,
                                                                                                  ACDK_Data.Max);


    //Pregain Gr
    ACDK_Data.ItemIdx = 2;            //g_FT_CCT_StateMachine.sensor_eng_item_idx_pregain_Gr

    bRet &= bSendDataToCCT(ACDK_CCT_OP_GET_ENG_SENSOR_PARA,
                                                (UINT8*)&ACDK_Data,
                                                sizeof(ACDK_SENSOR_ITEM_INFO_STRUCT),
                                                (UINT8*)&ACDK_Data,
                                                sizeof(ACDK_SENSOR_ITEM_INFO_STRUCT),
                                                &u4RetLen);    

    ACDK_LOGD("CCAP GET_SENSOR_PREGAIN_Gr:%d, %d, %d\n", ACDK_Data.ItemValue,
                                                                                                   ACDK_Data.Min,
                                                                                                  ACDK_Data.Max);



    //Pregain Gb
    ACDK_Data.ItemIdx = 3;            //g_FT_CCT_StateMachine.sensor_eng_item_idx_pregain_Gr
    ACDK_Data.ItemValue = 0x20;   //pREQ->cmd.set_sensor_pregain.pregain_r.value;

    bRet &= bSendDataToCCT(ACDK_CCT_OP_GET_ENG_SENSOR_PARA,
                                                (UINT8*)&ACDK_Data,
                                                sizeof(ACDK_SENSOR_ITEM_INFO_STRUCT),
                                                 (UINT8*)&ACDK_Data,
                                                sizeof(ACDK_SENSOR_ITEM_INFO_STRUCT),
                                                &u4RetLen);	

    ACDK_LOGD("CCAP GET_SENSOR_PREGAIN_Gb:%d, %d, %d\n", ACDK_Data.ItemValue,
                                                                                                   ACDK_Data.Min,
                                                                                                   ACDK_Data.Max);





    //Pregain B
    ACDK_Data.ItemIdx = 4;            //g_FT_CCT_StateMachine.sensor_eng_item_idx_pregain_Gr
    ACDK_Data.ItemValue = 0x20;   //pREQ->cmd.set_sensor_pregain.pregain_r.value;

    bRet &= bSendDataToCCT(ACDK_CCT_OP_GET_ENG_SENSOR_PARA,
                                                (UINT8*)&ACDK_Data,
                                                sizeof(ACDK_SENSOR_ITEM_INFO_STRUCT),
                                                 (UINT8*)&ACDK_Data,
                                                sizeof(ACDK_SENSOR_ITEM_INFO_STRUCT),
                                                &u4RetLen);	

    ACDK_LOGD("CCAP GET_SENSOR_PREGAIN_B:%d, %d, %d\n", ACDK_Data.ItemValue,
                                                                                                   ACDK_Data.Min,
                                                                                                   ACDK_Data.Max);




    if (!bRet)
    {
        ACDK_LOGE("Get Sensor PreGain Fail \n");
        return E_CCT_CCAP_API_FAIL;
    }
    return S_CCT_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// FT_MSDK_CCT_OP_SET_SENSOR_PREGAIN
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPSetSensorPreGain_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{

    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("FT_MSDK_CCT_OP_SET_SENSOR_PREGAIN\n");
    ACDK_SENSOR_ITEM_INFO_STRUCT ACDK_Item;
    memset(&ACDK_Item,0,sizeof(ACDK_SENSOR_ITEM_INFO_STRUCT));

    MUINT32 u4RetLen = 0;
    BOOL bRet = MTRUE;

    //Pregain R
    ACDK_Item.GroupIdx = 0;           //g_FT_CCT_StateMachine.sensor_eng_group_idx; PRE_GAIN
    ACDK_Item.ItemIdx = 1;             //g_FT_CCT_StateMachine.sensor_eng_item_idx_pregain_R
    ACDK_Item.ItemValue = 0x20;    //pREQ->cmd.set_sensor_pregain.pregain_r.value;
    ACDK_LOGD("CCAP SET_SENSOR_PREGAIN_R:%d, %d, %d\n", ACDK_Item.GroupIdx,
                                                                                                  ACDK_Item.ItemIdx,
                                                                                                  ACDK_Item.ItemValue);




    bRet &= bSendDataToCCT(ACDK_CCT_OP_SET_ENG_SENSOR_PARA,
                                                (UINT8*)&ACDK_Item,
                                                sizeof(ACDK_SENSOR_ITEM_INFO_STRUCT),
                                                NULL,
                                                0,
                                                &u4RetLen);

    //Pregain Gr
    ACDK_Item.ItemIdx = 2;            //g_FT_CCT_StateMachine.sensor_eng_item_idx_pregain_Gr
    ACDK_Item.ItemValue = 0x20;   //pREQ->cmd.set_sensor_pregain.pregain_r.value;

    ACDK_LOGD("CCAP SET_SENSOR_PREGAIN_Gr:%d, %d, %d\n", ACDK_Item.GroupIdx,
                                                                                                   ACDK_Item.ItemIdx,
                                                                                                   ACDK_Item.ItemValue);

    bRet &= bSendDataToCCT(ACDK_CCT_OP_SET_ENG_SENSOR_PARA,
                                                (UINT8*)&ACDK_Item,
                                                sizeof(ACDK_SENSOR_ITEM_INFO_STRUCT),
                                                NULL,
                                                0,
                                                &u4RetLen);

    //Pregain Gb
    ACDK_Item.ItemIdx = 3;            //g_FT_CCT_StateMachine.sensor_eng_item_idx_pregain_Gr
    ACDK_Item.ItemValue = 0x20;   //pREQ->cmd.set_sensor_pregain.pregain_r.value;

    ACDK_LOGD("CCAP SET_SENSOR_PREGAIN_Gb:%d, %d, %d\n", ACDK_Item.GroupIdx,
                                                                                                   ACDK_Item.ItemIdx,
                                                                                                   ACDK_Item.ItemValue);

    bRet &= bSendDataToCCT(ACDK_CCT_OP_SET_ENG_SENSOR_PARA,
                                                (UINT8*)&ACDK_Item,
                                                sizeof(ACDK_SENSOR_ITEM_INFO_STRUCT),
                                                NULL,
                                                0,
                                                &u4RetLen);



    //Pregain B
    ACDK_Item.ItemIdx = 4;            //g_FT_CCT_StateMachine.sensor_eng_item_idx_pregain_Gr
    ACDK_Item.ItemValue = 0x20;   //pREQ->cmd.set_sensor_pregain.pregain_r.value;

    ACDK_LOGD("CCAP SET_SENSOR_PREGAIN_B:%d, %d, %d\n", ACDK_Item.GroupIdx,
                                                                                                   ACDK_Item.ItemIdx,
                                                                                                   ACDK_Item.ItemValue);

    bRet &= bSendDataToCCT(ACDK_CCT_OP_SET_ENG_SENSOR_PARA,
                                                (UINT8*)&ACDK_Item,
                                                sizeof(ACDK_SENSOR_ITEM_INFO_STRUCT),
                                                NULL,
                                                0,
                                                &u4RetLen);


    if (!bRet)
    {
        ACDK_LOGE("Set Sensor PreGain Fail \n");
        return E_CCT_CCAP_API_FAIL;
    }
    return S_CCT_CCAP_OK;
}


/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_OP_AE_ENABLE
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPAEEnable_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend)
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_OP_AE_ENABLE\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================


    MUINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToCCT(ACDK_CCT_OP_AE_ENABLE,
                                NULL,
                                0,
                                NULL,
                                0,
                                &u4RetLen);


    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================

    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }

    return S_CCT_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_OP_AE_DISABLE
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPAEDisable_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_OP_AE_DISABLE\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================


    MUINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToCCT(ACDK_CCT_OP_AE_DISABLE,
                                NULL,
                                0,
                                NULL,
                                0,
                                &u4RetLen);


    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================


    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }

    return S_CCT_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_OP_AE_GET_ENABLE_INFO
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPGetAEInfo_cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_OP_AE_GET_ENABLE_INFO\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================

    MUINT32 u4RetLen = 0;
    MUINT32 u4AEEnableInfo = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToCCT(ACDK_CCT_OP_AE_GET_ENABLE_INFO,
                                NULL,
                                0,
                                (UINT8 *)&u4AEEnableInfo,
                                sizeof(u4AEEnableInfo),
                                &u4RetLen);

    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================

    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("AE enable Mode = %d\n", u4AEEnableInfo);

    return S_CCT_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_OP_DEV_AE_SET_SCENE_MODE
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPAESetSceneMode_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_OP_DEV_AE_SET_SCENE_MODE\n");

    if (a_u4Argc != 1)
    {
        ACDK_LOGD("Usage: AE scene mode value\n");
        return E_CCT_CCAP_API_FAIL;
    }
    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================
    MUINT32 u4RetLen = 0;
    MINT32 i4AEMode;

    i4AEMode = (MINT32) atoi((char *)a_pprArgv[0]);    // Off:0 Auto:1 Night:7 Action:8 Beach:9 Candlelight:10 Fireworks:11 Landscape:12 Portrait:13
                                                      // Night portrait:14 Party:15 Snow:16 Sports:17 Steadyphoto:18 Sunset:19 Theatre:20 ISO Anti shake:21

    ACDK_LOGD("AE scene mode = %d\n", i4AEMode);

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToCCT(ACDK_CCT_OP_DEV_AE_SET_SCENE_MODE,
                                (UINT8 *)&i4AEMode,
                                sizeof(MINT32),
                                NULL,
                                0,
                                &u4RetLen);

    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================

    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }
    return S_CCT_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_OP_DEV_AE_GET_INFO
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPAEGetInfo_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    AE_NVRAM_T rAENVRAM;

    memset(&rAENVRAM,0, sizeof(AE_NVRAM_T));

    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_OP_DEV_AE_GET_INFO\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================
    MUINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToCCT(ACDK_CCT_OP_DEV_AE_GET_INFO,
                                NULL,
                                0,
                                (UINT8 *)&rAENVRAM,
                                sizeof(AE_NVRAM_T),
                                &u4RetLen);


    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================

    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }

    // TEST ONLY (check AE parameter)
    ACDK_LOGD("u4MinGain = %d\n", rAENVRAM.rDevicesInfo.u4MinGain);
    ACDK_LOGD("u4MaxGain = %d\n", rAENVRAM.rDevicesInfo.u4MaxGain);
    ACDK_LOGD("u4MiniISOGain = %d\n", rAENVRAM.rDevicesInfo.u4MiniISOGain);
    ACDK_LOGD("u4GainStepUnit = %d\n", rAENVRAM.rDevicesInfo.u4GainStepUnit);
    ACDK_LOGD("u4PreExpUnit = %d\n", rAENVRAM.rDevicesInfo.u4PreExpUnit);
    ACDK_LOGD("u4PreMaxFrameRate = %d\n", rAENVRAM.rDevicesInfo.u4PreMaxFrameRate);
    ACDK_LOGD("u4VideoExpUnit = %d\n", rAENVRAM.rDevicesInfo.u4VideoExpUnit);
    ACDK_LOGD("u4VideoMaxFrameRate = %d\n", rAENVRAM.rDevicesInfo.u4VideoMaxFrameRate);
    ACDK_LOGD("u4Video2PreRatio = %d\n", rAENVRAM.rDevicesInfo.u4Video2PreRatio);
    ACDK_LOGD("u4CapExpUnit = %d\n", rAENVRAM.rDevicesInfo.u4CapExpUnit);
    ACDK_LOGD("u4CapMaxFrameRate = %d\n", rAENVRAM.rDevicesInfo.u4CapMaxFrameRate);
    ACDK_LOGD("u4Cap2PreRatio = %d\n", rAENVRAM.rDevicesInfo.u4Cap2PreRatio);
    ACDK_LOGD("u4LensFno = %d\n", rAENVRAM.rDevicesInfo.u4LensFno);

    ACDK_LOGD("u4HistHighThres = %d\n", rAENVRAM.rHistConfig.u4HistHighThres);
    ACDK_LOGD("u4HistLowThres = %d\n", rAENVRAM.rHistConfig.u4HistLowThres);
    ACDK_LOGD("u4MostBrightRatio = %d\n", rAENVRAM.rHistConfig.u4MostBrightRatio);
    ACDK_LOGD("u4MostDarkRatio = %d\n", rAENVRAM.rHistConfig.u4MostDarkRatio);
    ACDK_LOGD("u4CentralHighBound = %d\n", rAENVRAM.rHistConfig.u4CentralHighBound);
    ACDK_LOGD("u4CentralLowBound = %d\n", rAENVRAM.rHistConfig.u4CentralLowBound);
    ACDK_LOGD("u4OverExpThres[5] = {%d, %d, %d, %d, %d}\n", rAENVRAM.rHistConfig.u4OverExpThres[0], rAENVRAM.rHistConfig.u4OverExpThres[1],
                                     rAENVRAM.rHistConfig.u4OverExpThres[2], rAENVRAM.rHistConfig.u4OverExpThres[3], rAENVRAM.rHistConfig.u4OverExpThres[4]);
    ACDK_LOGD("u4HistStretchThres[5] = {%d, %d, %d, %d, %d}\n", rAENVRAM.rHistConfig.u4HistStretchThres[0], rAENVRAM.rHistConfig.u4HistStretchThres[1],
                                     rAENVRAM.rHistConfig.u4HistStretchThres[2], rAENVRAM.rHistConfig.u4HistStretchThres[3], rAENVRAM.rHistConfig.u4HistStretchThres[4]);
    ACDK_LOGD("u4BlackLightThres[5] = {%d, %d, %d, %d, %d}\n", rAENVRAM.rHistConfig.u4BlackLightThres[0], rAENVRAM.rHistConfig.u4BlackLightThres[1],
                                     rAENVRAM.rHistConfig.u4BlackLightThres[2], rAENVRAM.rHistConfig.u4BlackLightThres[3], rAENVRAM.rHistConfig.u4BlackLightThres[4]);
    ACDK_LOGD("bEnableBlackLight = %d\n", rAENVRAM.rCCTConfig.bEnableBlackLight);
    ACDK_LOGD("bEnableHistStretch = %d\n", rAENVRAM.rCCTConfig.bEnableHistStretch);
    ACDK_LOGD("bEnableAntiOverExposure = %d\n", rAENVRAM.rCCTConfig.bEnableAntiOverExposure);
    ACDK_LOGD("bEnableTimeLPF = %d\n", rAENVRAM.rCCTConfig.bEnableTimeLPF);
    ACDK_LOGD("bEnableCaptureThres = %d\n", rAENVRAM.rCCTConfig.bEnableCaptureThres);
    ACDK_LOGD("u4AETarget = %d\n", rAENVRAM.rCCTConfig.u4AETarget);
    ACDK_LOGD("u4InitIndex = %d\n", rAENVRAM.rCCTConfig.u4InitIndex);
    ACDK_LOGD("u4BackLightWeight = %d\n", rAENVRAM.rCCTConfig.u4BackLightWeight);
    ACDK_LOGD("u4HistStretchWeight = %d\n", rAENVRAM.rCCTConfig.u4HistStretchWeight);
    ACDK_LOGD("u4AntiOverExpWeight = %d\n", rAENVRAM.rCCTConfig.u4AntiOverExpWeight);
    ACDK_LOGD("u4BlackLightStrengthIndex = %d\n", rAENVRAM.rCCTConfig.u4BlackLightStrengthIndex);
    ACDK_LOGD("u4HistStretchStrengthIndex = %d\n", rAENVRAM.rCCTConfig.u4HistStretchStrengthIndex);
    ACDK_LOGD("u4AntiOverExpStrengthIndex = %d\n", rAENVRAM.rCCTConfig.u4AntiOverExpStrengthIndex);
    ACDK_LOGD("u4TimeLPFStrengthIndex = %d\n", rAENVRAM.rCCTConfig.u4TimeLPFStrengthIndex);
    ACDK_LOGD("u4LPFConvergeLevel[5] = {%d, %d, %d, %d, %d}\n", rAENVRAM.rCCTConfig.u4LPFConvergeLevel[0], rAENVRAM.rCCTConfig.u4LPFConvergeLevel[1],
                                     rAENVRAM.rCCTConfig.u4LPFConvergeLevel[2], rAENVRAM.rCCTConfig.u4LPFConvergeLevel[3], rAENVRAM.rCCTConfig.u4LPFConvergeLevel[4]);
    ACDK_LOGD("u4InDoorEV = %d\n", rAENVRAM.rCCTConfig.u4InDoorEV);
    ACDK_LOGD("i4BVOffset = %d\n", rAENVRAM.rCCTConfig.i4BVOffset);
    ACDK_LOGD("u4PreviewFlareOffset = %d\n", rAENVRAM.rCCTConfig.u4PreviewFlareOffset);
    ACDK_LOGD("u4CaptureFlareOffset = %d\n", rAENVRAM.rCCTConfig.u4CaptureFlareOffset);
    ACDK_LOGD("u4CaptureFlareThres = %d\n", rAENVRAM.rCCTConfig.u4CaptureFlareThres);

    return S_CCT_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_AE_GET_SCENE_MODE
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPAEGetSceneMode_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_AE_GET_SCENE_MODE\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================

    MINT32 i4AEMode = 0;
    MUINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToCCT(ACDK_CCT_V2_OP_AE_GET_SCENE_MODE,
                                NULL,
                                0,
                                (UINT8 *)&i4AEMode,
                                sizeof(i4AEMode),
                                &u4RetLen);

    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================


    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("AE Mode = %d\n", i4AEMode);

    return S_CCT_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_AE_SET_METERING_MODE
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPAESetMeteringMode_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_AE_SET_METERING_MODE\n");

    if (a_u4Argc != 1)
    {
        ACDK_LOGD("Usage: AE metering mode value\n");
        return E_CCT_CCAP_API_FAIL;
    }
    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================
    MUINT32 u4RetLen = 0;
    MINT32 i4AEMeteringMode;

    i4AEMeteringMode = (MINT32) atoi((char *)a_pprArgv[0]);    // Central weighting : 0    Spot : 1    Average : 2

    ACDK_LOGD("AE metering mode = %d\n", i4AEMeteringMode);

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToCCT(ACDK_CCT_V2_OP_AE_SET_METERING_MODE,
                                (UINT8 *)&i4AEMeteringMode,
                                sizeof(MINT32),
                                NULL,
                                0,
                                &u4RetLen);

    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================

    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }
    return S_CCT_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_AE_APPLY_EXPO_INFO
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPAEApplyExpoInfo_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_AE_APPLY_EXPO_INFO\n");

    if (a_u4Argc != 6)
    {
        ACDK_LOGD("Usage: Exposure AfeGain PreFlare CapFlare bFlareAuto\n");
        return E_CCT_CCAP_API_FAIL;
    }

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================

    MUINT32 u4RetLen = 0, u4FrameRate;
    ACDK_AE_MODE_CFG_T rAEExpPara;

    rAEExpPara.u4Eposuretime = (MUINT32)atoi((char *)a_pprArgv[0]);
    rAEExpPara.u4AfeGain = (MUINT32)atoi((char *)a_pprArgv[1]);
    rAEExpPara.u4IspGain = (MUINT32)atoi((char *)a_pprArgv[2]);
    rAEExpPara.u2FlareValue= (UINT16)atoi((char *)a_pprArgv[3]);
    rAEExpPara.u2FlareGain = 128*512 / (512 - rAEExpPara.u2FlareValue);
    u4FrameRate = 1000000 / rAEExpPara.u4Eposuretime;
    rAEExpPara.u2CaptureFlareValue= (UINT16)atoi((char *)a_pprArgv[4]);
    rAEExpPara.u2CaptureFlareGain = 128*512 / (512 - rAEExpPara.u2FlareValue);
    rAEExpPara.bFlareAuto = (UINT8)atoi((char *)a_pprArgv[5]);
    if(u4FrameRate >= 30)
    {
        u4FrameRate = 30;
    }
    rAEExpPara.u2FrameRate = u4FrameRate;

    ACDK_LOGD("Expoure time:%d\n", rAEExpPara.u4Eposuretime);
    ACDK_LOGD("AFE Gain:%d Isp Gain:%d\n", rAEExpPara.u4AfeGain, rAEExpPara.u4IspGain);
    ACDK_LOGD("Flare:%d Flare Gain:%d\n",rAEExpPara.u2FlareValue, rAEExpPara.u2FlareGain);
    ACDK_LOGD("Capture Flare:%d Capture Flare Gain:%d\n",rAEExpPara.u2CaptureFlareValue, rAEExpPara.u2CaptureFlareGain);
    ACDK_LOGD("bFlareAuto:%d\n", rAEExpPara.bFlareAuto);
    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToCCT(ACDK_CCT_V2_OP_AE_APPLY_EXPO_INFO,
                                (UINT8 *)&rAEExpPara,
                                sizeof(ACDK_AE_MODE_CFG_T),
                                NULL,
                                0,
                                &u4RetLen);

    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================

    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }

    return S_CCT_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_AE_SELECT_BAND
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPAESelectBand_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_AE_SELECT_BAND\n");

    if (a_u4Argc != 1)
    {
        ACDK_LOGD("Usage: Flicker mode value\n");
        return E_CCT_CCAP_API_FAIL;
    }
    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================
    MUINT32 u4RetLen = 0;
    MINT32 i4AEFlickerMode;

    i4AEFlickerMode = (MINT32) atoi((char *)a_pprArgv[0]);    // 60Hz : 0    50Hz : 1

    ACDK_LOGD("AE Flicker mode = %d\n", i4AEFlickerMode);

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToCCT(ACDK_CCT_V2_OP_AE_SELECT_BAND,
                               (UINT8 *)&i4AEFlickerMode,
                               sizeof(MINT32),
                               NULL,
                               0,
                               &u4RetLen);

    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================

    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }
    return S_CCT_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_AE_GET_AUTO_EXPO_PARA
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPAEGetAutoExpoPara_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_AE_GET_AUTO_EXPO_PARA\n");

    if (a_u4Argc != 1)
    {
        ACDK_LOGD("[Usage] GainMode = 0: AFE Gain, GainMode = 1: ISO\n");
        return E_CCT_CCAP_API_FAIL;
    }
    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================
    MUINT32 u4RetLen = 0;
    ACDK_AE_MODE_CFG_T rAEExpPara;
    MUINT32 u4GainMode;

    u4GainMode = (MUINT32) atoi((char *)a_pprArgv[0]);

    ACDK_LOGD("Gain mode = %d\n", u4GainMode);

    memset(&rAEExpPara,0, sizeof(ACDK_AE_MODE_CFG_T));

    rAEExpPara.u4GainMode = u4GainMode;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToCCT(ACDK_CCT_V2_OP_AE_GET_AUTO_EXPO_PARA,
                                (UINT8 *)&rAEExpPara,
                                sizeof(ACDK_AE_MODE_CFG_T),
                                (UINT8 *)&rAEExpPara,
                                sizeof(ACDK_AE_MODE_CFG_T),
                                &u4RetLen);

    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================
    ACDK_LOGD("Expoure time:%d\n", rAEExpPara.u4Eposuretime);
    ACDK_LOGD("Gain Mode:%d\n", rAEExpPara.u4GainMode);
    ACDK_LOGD("AFE Gain:%d Isp Gain:%d\n", rAEExpPara.u4AfeGain, rAEExpPara.u4IspGain);
    ACDK_LOGD("ISO:%d\n", rAEExpPara.u4ISO);
    ACDK_LOGD("Preview Flare:%d Flare Gain:%d\n",rAEExpPara.u2FlareValue, rAEExpPara.u2FlareGain);
    ACDK_LOGD("Capture Flare:%d Flare Gain:%d\n",rAEExpPara.u2CaptureFlareValue, rAEExpPara.u2CaptureFlareGain);
    ACDK_LOGD("bFlareAuto:%d\n",rAEExpPara.bFlareAuto);

    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }

    return S_CCT_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_AE_GET_BAND
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPAEGetBand_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
     if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_AE_GET_BAND\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================

    MUINT32 u4RetLen = 0;
    MINT32 i4AEFlickerMode = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToCCT(ACDK_CCT_V2_OP_AE_GET_BAND,
                                NULL,
                                0,
                                (UINT8 *)&i4AEFlickerMode,
                                sizeof(MINT32),
                                &u4RetLen);

    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================

    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("AE Flicker Mode = %d\n", i4AEFlickerMode);

    return S_CCT_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_AE_GET_METERING_RESULT
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPAEGetMeteringResoult_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
     if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_AE_GET_METERING_RESULT\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================

    MUINT32 u4RetLen = 0;
    MINT32 i4AEMeteringMode = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToCCT(ACDK_CCT_V2_OP_AE_GET_METERING_RESULT,
                                NULL,
                                0,
                                (UINT8 *)&i4AEMeteringMode,
                                sizeof(MINT32),
                                &u4RetLen);

    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================

    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("AE Metering Mode = %d\n", i4AEMeteringMode);
    return S_CCT_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_OP_DEV_AE_APPLY_INFO
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPAEApplyInfo_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    // AE NVRAM test data
    AE_NVRAM_T rAENVRAMTestData;

    // TEST ONLY (check AE parameter)
    rAENVRAMTestData.rDevicesInfo.u4MinGain = 1200;
    rAENVRAMTestData.rDevicesInfo.u4MaxGain = 20480;
    rAENVRAMTestData.rDevicesInfo.u4MiniISOGain = 60;
    rAENVRAMTestData.rDevicesInfo.u4GainStepUnit = 256;
    rAENVRAMTestData.rDevicesInfo.u4PreExpUnit = 62;
    rAENVRAMTestData.rDevicesInfo.u4PreMaxFrameRate = 15;
    rAENVRAMTestData.rDevicesInfo.u4VideoExpUnit = 62;
    rAENVRAMTestData.rDevicesInfo.u4VideoMaxFrameRate = 10;
    rAENVRAMTestData.rDevicesInfo.u4Video2PreRatio = 1024;
    rAENVRAMTestData.rDevicesInfo.u4CapExpUnit = 116;
    rAENVRAMTestData.rDevicesInfo.u4CapMaxFrameRate = 4;
    rAENVRAMTestData.rDevicesInfo.u4Cap2PreRatio = 1024;
    rAENVRAMTestData.rDevicesInfo.u4LensFno = 56;

    rAENVRAMTestData.rHistConfig.u4HistHighThres = 4;
    rAENVRAMTestData.rHistConfig.u4HistLowThres = 20;
    rAENVRAMTestData.rHistConfig.u4MostBrightRatio = 4;
    rAENVRAMTestData.rHistConfig.u4MostDarkRatio = 2;
    rAENVRAMTestData.rHistConfig.u4CentralHighBound = 180;
    rAENVRAMTestData.rHistConfig.u4CentralLowBound = 40;
    rAENVRAMTestData.rHistConfig.u4OverExpThres[0] = 245;
    rAENVRAMTestData.rHistConfig.u4OverExpThres[1] = 235;
    rAENVRAMTestData.rHistConfig.u4OverExpThres[2] = 225;
    rAENVRAMTestData.rHistConfig.u4OverExpThres[3] = 215;
    rAENVRAMTestData.rHistConfig.u4OverExpThres[4] = 205;
    rAENVRAMTestData.rHistConfig.u4HistStretchThres[0] = 80;
    rAENVRAMTestData.rHistConfig.u4HistStretchThres[1] = 90;
    rAENVRAMTestData.rHistConfig.u4HistStretchThres[2] = 100;
    rAENVRAMTestData.rHistConfig.u4HistStretchThres[3] = 110;
    rAENVRAMTestData.rHistConfig.u4HistStretchThres[4] = 120;
    rAENVRAMTestData.rHistConfig.u4BlackLightThres[0] = 14;
    rAENVRAMTestData.rHistConfig.u4BlackLightThres[1] = 16;
    rAENVRAMTestData.rHistConfig.u4BlackLightThres[2] = 18;
    rAENVRAMTestData.rHistConfig.u4BlackLightThres[3] = 20;
    rAENVRAMTestData.rHistConfig.u4BlackLightThres[4] = 22;
    rAENVRAMTestData.rCCTConfig.bEnableBlackLight = MFALSE;
    rAENVRAMTestData.rCCTConfig.bEnableHistStretch = MFALSE;
    rAENVRAMTestData.rCCTConfig.bEnableAntiOverExposure = MTRUE;
    rAENVRAMTestData.rCCTConfig.bEnableTimeLPF = MFALSE;
    rAENVRAMTestData.rCCTConfig.bEnableCaptureThres = MFALSE;
    rAENVRAMTestData.rCCTConfig.u4AETarget = 50;
    rAENVRAMTestData.rCCTConfig.u4InitIndex = 40;
    rAENVRAMTestData.rCCTConfig.u4BackLightWeight = 2;
    rAENVRAMTestData.rCCTConfig.u4HistStretchWeight = 16;
    rAENVRAMTestData.rCCTConfig.u4AntiOverExpWeight = 2;
    rAENVRAMTestData.rCCTConfig.u4BlackLightStrengthIndex = 0;
    rAENVRAMTestData.rCCTConfig.u4HistStretchStrengthIndex = 0;
    rAENVRAMTestData.rCCTConfig.u4AntiOverExpStrengthIndex = 0;
    rAENVRAMTestData.rCCTConfig.u4TimeLPFStrengthIndex = 0;
    rAENVRAMTestData.rCCTConfig.u4LPFConvergeLevel[0] = 1;
    rAENVRAMTestData.rCCTConfig.u4LPFConvergeLevel[1] = 2;
    rAENVRAMTestData.rCCTConfig.u4LPFConvergeLevel[2] = 3;
    rAENVRAMTestData.rCCTConfig.u4LPFConvergeLevel[3] = 4;
    rAENVRAMTestData.rCCTConfig.u4LPFConvergeLevel[4] = 5;
    rAENVRAMTestData.rCCTConfig.u4InDoorEV = 100;
    rAENVRAMTestData.rCCTConfig.i4BVOffset = 47;
    rAENVRAMTestData.rCCTConfig.u4PreviewFlareOffset = 2;
    rAENVRAMTestData.rCCTConfig.u4CaptureFlareOffset = 0;
    rAENVRAMTestData.rCCTConfig.u4CaptureFlareThres = 2;

     if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_OP_DEV_AE_APPLY_INFO\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================


    MUINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToCCT(ACDK_CCT_OP_DEV_AE_APPLY_INFO,
                                (UINT8 *)&rAENVRAMTestData, // TEST ONLY
                                sizeof(AE_NVRAM_T),
                                NULL,
                                0,
                                &u4RetLen);

    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================


    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }

    return S_CCT_CCAP_OK;
}


/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_OP_DEV_AE_SAVE_INFO_NVRAM
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPAESaveInfoToNVRAM_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_OP_DEV_AE_SAVE_INFO_NVRAM\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================


    MUINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToCCT(ACDK_CCT_OP_DEV_AE_SAVE_INFO_NVRAM,
                                NULL,
                                0,
                                NULL,
                                0,
                                &u4RetLen);


    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================


    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }
    return S_CCT_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_OP_DEV_AE_GET_EV_CALIBRATION
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPAEGetEVCalibration_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
     if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_OP_DEV_AE_GET_EV_CALIBRATION\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================
    MUINT32 u4RetLen = 0;
    MINT32 i4AEcurrentEVValue = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToCCT(ACDK_CCT_OP_DEV_AE_GET_EV_CALIBRATION,
                                NULL,
                                0,
                                (UINT8 *)&i4AEcurrentEVValue,
                                sizeof(MINT32),
                                &u4RetLen);


    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================


    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("AE current EV value = %d\n", i4AEcurrentEVValue);

    return S_CCT_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_OP_DEV_AE_GET_FLARE_CALIBRATION
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPAEGetFlareOffsetCalibration_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
     if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_OP_DEV_AE_GET_FLARE_CALIBRATION\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================
    MUINT32 u4RetLen = 0;
    MUINT32 u4FixFlareOffset = 0;
    MUINT32 u4FlareThres;
    
    u4FlareThres = (MUINT32) atoi((char *)a_pprArgv[0]);    
    ACDK_LOGD("Flare thres ratio = %d\n", u4FlareThres);

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToCCT(ACDK_CCT_OP_DEV_AE_GET_FLARE_CALIBRATION,
                               (UINT8 *)&u4FlareThres,
                               sizeof(MUINT32),
                                (UINT8 *)&u4FixFlareOffset,
                                sizeof(MUINT32),
                                &u4RetLen);

    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================

    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("Fix Flare Offset value = %d\n", u4FixFlareOffset);

    return S_CCT_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_OP_AE_SET_SENSOR_EXP_LINE
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPAESetExpLine_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
     if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_OP_AE_SET_SENSOR_EXP_LINE\n");

    if (a_u4Argc != 1)
    {
        ACDK_LOGD("Usage: AE exposure line\n");
        return E_CCT_CCAP_API_FAIL;
    }
    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================
    MUINT32 u4RetLen = 0;
    MINT32 i4AEExpLine;

    i4AEExpLine = (MINT32) atoi((char *)a_pprArgv[0]);

    ACDK_LOGD("AE exposure line = %d\n", i4AEExpLine);

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToCCT(ACDK_CCT_OP_AE_SET_SENSOR_EXP_LINE,
                                (UINT8 *)&i4AEExpLine,
                                sizeof(MINT32),
                                NULL,
                                0,
                                &u4RetLen);

    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================

    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }

    return S_CCT_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_AWB_ENABLE_AUTO_RUN
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPAWBEnableAutoRun_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_AWB_ENABLE_AUTO_RUN\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================


    MUINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToCCT(ACDK_CCT_V2_OP_AWB_ENABLE_AUTO_RUN,
                                NULL,
                                0,
                                NULL,
                                0,
                                &u4RetLen);


    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================


    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }

    return S_CCT_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_AWB_DISABLE_AUTO_RUN
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPAWBDisableAutoRun_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_AWB_DISABLE_AUTO_RUN\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================


    MUINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToCCT(ACDK_CCT_V2_OP_AWB_DISABLE_AUTO_RUN,
                                NULL,
                                0,
                                NULL,
                                0,
                                &u4RetLen);


    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================


    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }

    return S_CCT_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_AWB_GET_AUTO_RUN_INFO
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPAWBGetEnableInfo_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_AWB_GET_AUTO_RUN_INFO\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================

    MUINT32 u4RetLen = 0;
    MINT32 i4AWBEnable;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToCCT(ACDK_CCT_V2_OP_AWB_GET_AUTO_RUN_INFO,
                                NULL,
                                0,
                                (UINT8 *)&i4AWBEnable,
                                sizeof(MINT32),
                                &u4RetLen);


    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================


    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("i4AWBEnable = %d\n", i4AWBEnable);

    return S_CCT_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_AWB_GET_GAIN
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPAWBGetGain_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_AWB_GET_GAIN\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================

    MUINT32 u4RetLen = 0;
    AWB_GAIN_T rAWBGain;

    memset(&rAWBGain,0, sizeof(rAWBGain));

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToCCT(ACDK_CCT_V2_OP_AWB_GET_GAIN,
                                NULL,
                                0,
                                (UINT8 *)&rAWBGain,
                                sizeof(AWB_GAIN_T),
                                &u4RetLen);


    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================


    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("AWB Rgain = %d\n", rAWBGain.i4R);
    ACDK_LOGD("AWB Ggain = %d\n", rAWBGain.i4G);
    ACDK_LOGD("AWB Bgain = %d\n", rAWBGain.i4B);

    return S_CCT_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_AWB_SET_GAIN
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPAWBSetGain_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_AWB_SET_GAIN\n");

    if (a_u4Argc != 3)
    {
        ACDK_LOGD("Usage: sAWBGain <Rgain> <Ggain> <Bgain>\n");
        return E_CCT_CCAP_API_FAIL;
    }

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================


    MUINT32 u4RetLen = 0;
    AWB_GAIN_T rAWBGain;

    rAWBGain.i4R = (MUINT32)atoi((char *)a_pprArgv[0]);
    rAWBGain.i4G = (MUINT32)atoi((char *)a_pprArgv[1]);
    rAWBGain.i4B = (MUINT32)atoi((char *)a_pprArgv[2]);

    ACDK_LOGD("AWB Rgain = %d\n", rAWBGain.i4R);
    ACDK_LOGD("AWB Ggain = %d\n", rAWBGain.i4G);
    ACDK_LOGD("AWB Bgain = %d\n", rAWBGain.i4B);

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToCCT(ACDK_CCT_V2_OP_AWB_SET_GAIN,
                                (UINT8 *)&rAWBGain,
                                sizeof(AWB_GAIN_T),
                                NULL,
                                0,
                                &u4RetLen);


    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================


    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }

    return S_CCT_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_AWB_APPLY_CAMERA_PARA2
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPAWBApplyCameraPara_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
// AWB NVRAM test data
AWB_NVRAM_T rAWBNVRAMTestData =
{
	// AWB calibration data
	{
		// rCalGain (calibration gain: 1.0 = 512)
		{
			0,	// u4R
			0,	// u4G
			0	// u4B
		},
		// rDefGain (Default calibration gain: 1.0 = 512)
		{
			0,	// u4R
			0,	// u4G
			0   // u4B
		},
		// rDefGain (Default calibration gain: 1.0 = 512)
		{
			0,	// u4R
			0,	// u4G
			0	// u4B
		},
		// rD65Gain (D65 WB gain: 1.0 = 512)
		{
			1270,	// u4R
			512,	// u4G
			817	// u4B
		}
	},
	// Original XY coordinate of AWB light source
	{
		// Strobe
		{
			0,	// i4X
			0	// i4Y
		},
		// Horizon
		{
			-559,	// i4X
			-575	// i4Y
		},
		// A
		{
			-370,	// i4X
			-550	// i4Y
		},
		// TL84
		{
			-95,	// i4X
			-537	// i4Y
		},
		// CWF
		{
			-102,	// i4X
			-646	// i4Y
		},
		// DNP
		{
			48,	// i4X
			-504	// i4Y
		},
		// D65
		{
			163,	// i4X
			-508	// i4Y
		},
		// DF
		{
			0, 	// i4X
			0	// i4Y
		}
	},
	// Rotated XY coordinate of AWB light source
	{
		// Strobe
		{
			0,	// i4X
			0	// i4Y
		},
		// Horizon
		{
			-606,	// i4X
			-525	// i4Y
		},
		// A
		{
			-416,	// i4X
			-516	// i4Y
		},
		// TL84
		{
			-141,	// i4X
			-527	// i4Y
		},
		// CWF
		{
			-157,	// i4X
			-635	// i4Y
		},
		// DNP
		{
			5,	// i4X
			-506	// i4Y
		},
		// D65
		{
			119,	// i4X
			-520	// i4Y
		},
		// DF
		{
			0,	// i4X
			0	// i4Y
		}
	},
	// AWB gain of AWB light source
	{
		// Strobe
		{
			512,	// u4R
			512,	// u4G
			512	// u4B
		},
		// Horizon
		{
			523,	// u4R
			512,	// u4G
			2380	// u4B
		},
		// A
		{
			653,	// u4R
			512,	// u4G
			1778	// u4B
		},
		// TL84
		{
			931,	// u4R
			512,	// u4G
			1203	// u4B
		},
		// CWF
		{
			1070,	// u4R
			512,	// u4G
			1409	// u4B
		},
		// DNP
		{
			1082,	// u4R
			512,	// u4G
			949	// u4B
		},
		// D65
		{
			1270,	// u4R
			512,	// u4G
			817	// u4B
		},
		// DF
		{
			512,	// u4R
			512,	// u4G
			512	// u4B
		}
	},
	// Rotation matrix parameter
	{
		5,	// i4RotationAngle
		255,	// i4Cos
		22,	// i4Sin
	},
	// Daylight locus parameter
	{
		-153,	// i4SlopeNumerator
		128	// i4SlopeDenominator
	},
	// AWB light area
	{
		// Strobe
		{
			0,	// i4RightBound
			0,	// i4LeftBound
			0,	// i4UpperBound
			0	// i4LowerBound
		},
		// Tungsten
		{
			-191,	// i4RightBound
			-841,	// i4LeftBound
			-471,	// i4UpperBound
			-571	// i4LowerBound
		},
		// Warm fluorescent
		{
			-191,	// i4RightBound
			-841,	// i4LeftBound
			-571,	// i4UpperBound
			-691	// i4LowerBound
		},
		// Fluorescent
		{
			-45,	// i4RightBound
			-191,	// i4LeftBound
			-456,	// i4UpperBound
			-581	// i4LowerBound
		},
		// CWF
		{
			-45,	// i4RightBound
			-191,	// i4LeftBound
			-581,	// i4UpperBound
			-685	// i4LowerBound
		},
		// Daylight
		{
			144,	// i4RightBound
			-45,	// i4LeftBound
			-440,	// i4UpperBound
			-600	// i4LowerBound
		},
		// Shade
		{
			504,	// i4RightBound
			144,	// i4LeftBound
			-440,	// i4UpperBound
			-600	// i4LowerBound
		},
		// Daylight Fluorescent
		{
			0,	// i4RightBound
			0,	// i4LeftBound
			0,	// i4UpperBound
			0	// i4LowerBound
		}
	},
	// PWB light area
	{
		// Reference area
		{
			504,	// i4RightBound
			-841,	// i4LeftBound
			-415,	// i4UpperBound
			-691	// i4LowerBound
		},
		// Daylight
		{
			169,	// i4RightBound
			-45,	// i4LeftBound
			-440,	// i4UpperBound
			-600	// i4LowerBound
		},
		// Cloudy daylight
		{
			294,	// i4RightBound
			94,	// i4LeftBound
			-440,	// i4UpperBound
			-600	// i4LowerBound
		},
		// Shade
		{
			444,	// i4RightBound
			244,	// i4LeftBound
			-440,	// i4UpperBound
			-600	// i4LowerBound
		},
		// Twilight
		{
			-45,	// i4RightBound
			-205,	// i4LeftBound
			-440,	// i4UpperBound
			-600	// i4LowerBound
		},
		// Fluorescent
		{
			169,	// i4RightBound
			-257,	// i4LeftBound
			-470,	// i4UpperBound
			-685	// i4LowerBound
		},
		// Warm fluorescent
		{
			-257,	// i4RightBound
			-516,	// i4LeftBound
			-470,	// i4UpperBound
			-685	// i4LowerBound
		},
		// Incandescent
		{
			-257,	// i4RightBound
			-516,	// i4LeftBound
			-440,	// i4UpperBound
			-600	// i4LowerBound
		},
		// Gray World
		{
			1023,	// i4RightBound
			-1024,	// i4LeftBound
			1023,	// i4UpperBound
			-1024	// i4LowerBound
		}
	},
	// PWB default gain
	{
		// Daylight
		{
			1184,	// u4R
			512,	// u4G
			888	// u4B
		},
		// Cloudy daylight
		{
			1393,	// u4R
			513,	// u4G
			731	// u4B
		},
		// Shade
		{
			1676,	// u4R
			514,	// u4G
			587	// u4B
		},
		// Twilight
		{
			940,	// u4R
			515,	// u4G
			1167	// u4B
		},
		// Fluorescent
		{
			1130,	// u4R
			516,	// u4G
			1113	// u4B
		},
		// Warm fluorescent
		{
			741,	// u4R
			517,	// u4G
			1838	// u4B
		},
		// Incandescent
		{
			681,	// u4R
			518,	// u4G
			1713	// u4B
		},
		// Gray World
		{
			512,	// u4R
			519,	// u4G
			512	// u4B
		}
	},
	// AWB preference color
	{
		// Tungsten
		{
			50,	// i4SliderValue
			4240	// i4OffsetThr
		},
		// Warm fluorescent
		{
			50,	// i4SliderValue
			4240	// i4OffsetThr
		},
		// Shade
		{
			50,	// i4SliderValue
			842	// i4OffsetThr
		},
		// Daylight WB gain
		{
			1104,	// u4R
			512,	// u4G
			965	// u4B
		},
		// Preference gain: strobe
		{
			512,	// u4R
			512,	// u4G
			512	// u4B
		},
		// Preference gain: tungsten
		{
			512,	// u4R
			512,	// u4G
			512	// u4B
		},
		// Preference gain: warm fluorescent
		{
			512,	// u4R
			512,	// u4G
			512	// u4B
		},
		// Preference gain: fluorescent
		{
			512,	// u4R
			512,	// u4G
			512	// u4B
		},
		// Preference gain: CWF
		{
			512,	// u4R
			512,	// u4G
			512	// u4B
		},
		// Preference gain: daylight
		{
			512,	// u4R
			512,	// u4G
			512	// u4B
		},
		// Preference gain: shade
		{
			512,	// u4R
			512,	// u4G
			512	// u4B
		},
		// Preference gain: daylight fluorescent
		{
			512,	// u4R
			512,	// u4G
			512	// u4B
		}
	},
	// CCT estimation
	{
		// CCT
		{
			2300,	// i4CCT[0]
			2850,	// i4CCT[1]
			4100,	// i4CCT[2]
			5100,	// i4CCT[3]
			6500 	// i4CCT[4]
		},
		// Rotated X coordinate
		{
			-725,	// i4RotatedXCoordinate[0]
			-535,	// i4RotatedXCoordinate[1]
			-260,	// i4RotatedXCoordinate[2]
			-114,	// i4RotatedXCoordinate[3]
			0 	// i4RotatedXCoordinate[4]
		}
	}

};

     if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_AWB_APPLY_CAMERA_PARA2\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================


    MUINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToCCT(ACDK_CCT_V2_OP_AWB_APPLY_CAMERA_PARA2,
                                (UINT8 *)&rAWBNVRAMTestData, // TEST ONLY
                                sizeof(AWB_NVRAM_T),
                                NULL,
                                0,
                                &u4RetLen);

    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================


    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }

    return S_CCT_CCAP_OK;

}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_AWB_GET_AWB_PARA
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPAWBGetAWBPara_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    AWB_NVRAM_T rAWBNVRAM; // TEST ONLY

    memset(&rAWBNVRAM,0, sizeof(rAWBNVRAM)); // TEST ONLY

    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_AWB_GET_AWB_PARA\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================


    MUINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToCCT(ACDK_CCT_V2_OP_AWB_GET_AWB_PARA,
                                NULL,
                                0,
                                (UINT8 *)&rAWBNVRAM,
                                sizeof(AWB_NVRAM_T),
                                &u4RetLen);

    // TEST ONLY (check AWB parameter)
    ACDK_LOGD("rUnitGain.i4R = %d\n", rAWBNVRAM.rCalData.rUnitGain.i4R);
    ACDK_LOGD("rUnitGain.i4G = %d\n", rAWBNVRAM.rCalData.rUnitGain.i4G);
    ACDK_LOGD("rUnitGain.i4B = %d\n", rAWBNVRAM.rCalData.rUnitGain.i4B);
    ACDK_LOGD("rGoldenGain.i4R = %d\n", rAWBNVRAM.rCalData.rGoldenGain.i4R);
    ACDK_LOGD("rGoldenGain.i4G = %d\n", rAWBNVRAM.rCalData.rGoldenGain.i4G);
    ACDK_LOGD("rGoldenGain.i4B = %d\n", rAWBNVRAM.rCalData.rGoldenGain.i4B);
    ACDK_LOGD("rTuningUnitGain.i4R = %d\n", rAWBNVRAM.rCalData.rTuningUnitGain.i4R);
    ACDK_LOGD("rTuningUnitGain.i4G = %d\n", rAWBNVRAM.rCalData.rTuningUnitGain.i4G);
    ACDK_LOGD("rTuningUnitGain.i4B = %d\n", rAWBNVRAM.rCalData.rTuningUnitGain.i4B);
    ACDK_LOGD("rD65Gain.i4R = %d\n", rAWBNVRAM.rCalData.rD65Gain.i4R);
    ACDK_LOGD("rD65Gain.i4G = %d\n", rAWBNVRAM.rCalData.rD65Gain.i4G);
    ACDK_LOGD("rD65Gain.i4B = %d\n", rAWBNVRAM.rCalData.rD65Gain.i4B);


    ACDK_LOGD("rOriginalXY.rStrobe.i4X = %d\n", rAWBNVRAM.rOriginalXY.rStrobe.i4X);
    ACDK_LOGD("rOriginalXY.rStrobe.i4Y = %d\n", rAWBNVRAM.rOriginalXY.rStrobe.i4Y);
    ACDK_LOGD("rOriginalXY.rHorizon.i4X = %d\n", rAWBNVRAM.rOriginalXY.rHorizon.i4X);
    ACDK_LOGD("rOriginalXY.rHorizon.i4Y = %d\n", rAWBNVRAM.rOriginalXY.rHorizon.i4Y);
    ACDK_LOGD("rOriginalXY.rA.i4X = %d\n", rAWBNVRAM.rOriginalXY.rA.i4X);
    ACDK_LOGD("rOriginalXY.rA.i4Y = %d\n", rAWBNVRAM.rOriginalXY.rA.i4Y);
    ACDK_LOGD("rOriginalXY.rTL84.i4X = %d\n", rAWBNVRAM.rOriginalXY.rTL84.i4X);
    ACDK_LOGD("rOriginalXY.rTL84.i4Y = %d\n", rAWBNVRAM.rOriginalXY.rTL84.i4Y);
    ACDK_LOGD("rOriginalXY.rCWF.i4X = %d\n", rAWBNVRAM.rOriginalXY.rCWF.i4X);
    ACDK_LOGD("rOriginalXY.rCWF.i4Y = %d\n", rAWBNVRAM.rOriginalXY.rCWF.i4Y);
    ACDK_LOGD("rOriginalXY.rDNP.i4X = %d\n", rAWBNVRAM.rOriginalXY.rDNP.i4X);
    ACDK_LOGD("rOriginalXY.rDNP.i4Y = %d\n", rAWBNVRAM.rOriginalXY.rDNP.i4Y);
    ACDK_LOGD("rOriginalXY.rD65.i4X = %d\n", rAWBNVRAM.rOriginalXY.rD65.i4X);
    ACDK_LOGD("rOriginalXY.rD65.i4Y = %d\n", rAWBNVRAM.rOriginalXY.rD65.i4Y);
    ACDK_LOGD("rOriginalXY.rDF.i4X = %d\n", rAWBNVRAM.rOriginalXY.rDF.i4X);
    ACDK_LOGD("rOriginalXY.rDF.i4Y = %d\n", rAWBNVRAM.rOriginalXY.rDF.i4Y);

    ACDK_LOGD("rRotatedXY.rStrobe.i4X = %d\n", rAWBNVRAM.rRotatedXY.rStrobe.i4X);
    ACDK_LOGD("rRotatedXY.rStrobe.i4Y = %d\n", rAWBNVRAM.rRotatedXY.rStrobe.i4Y);
    ACDK_LOGD("rRotatedXY.rHorizon.i4X = %d\n", rAWBNVRAM.rRotatedXY.rHorizon.i4X);
    ACDK_LOGD("rRotatedXY.rHorizon.i4Y = %d\n", rAWBNVRAM.rRotatedXY.rHorizon.i4Y);
    ACDK_LOGD("rRotatedXY.rA.i4X = %d\n", rAWBNVRAM.rRotatedXY.rA.i4X);
    ACDK_LOGD("rRotatedXY.rA.i4Y = %d\n", rAWBNVRAM.rRotatedXY.rA.i4Y);
    ACDK_LOGD("rRotatedXY.rTL84.i4X = %d\n", rAWBNVRAM.rRotatedXY.rTL84.i4X);
    ACDK_LOGD("rRotatedXY.rTL84.i4Y = %d\n", rAWBNVRAM.rRotatedXY.rTL84.i4Y);
    ACDK_LOGD("rRotatedXY.rCWF.i4X = %d\n", rAWBNVRAM.rRotatedXY.rCWF.i4X);
    ACDK_LOGD("rRotatedXY.rCWF.i4Y = %d\n", rAWBNVRAM.rRotatedXY.rCWF.i4Y);
    ACDK_LOGD("rRotatedXY.rDNP.i4X = %d\n", rAWBNVRAM.rRotatedXY.rDNP.i4X);
    ACDK_LOGD("rRotatedXY.rDNP.i4Y = %d\n", rAWBNVRAM.rRotatedXY.rDNP.i4Y);
    ACDK_LOGD("rRotatedXY.rD65.i4X = %d\n", rAWBNVRAM.rRotatedXY.rD65.i4X);
    ACDK_LOGD("rRotatedXY.rD65.i4Y = %d\n", rAWBNVRAM.rRotatedXY.rD65.i4Y);
    ACDK_LOGD("rRotatedXY.rDF.i4X = %d\n", rAWBNVRAM.rRotatedXY.rDF.i4X);
    ACDK_LOGD("rRotatedXY.rDF.i4Y = %d\n", rAWBNVRAM.rRotatedXY.rDF.i4Y);

    ACDK_LOGD("rStrobe.i4R = %d\n", rAWBNVRAM.rLightAWBGain.rStrobe.i4R);
    ACDK_LOGD("rStrobe.i4G = %d\n", rAWBNVRAM.rLightAWBGain.rStrobe.i4G);
    ACDK_LOGD("rStrobe.i4B = %d\n", rAWBNVRAM.rLightAWBGain.rStrobe.i4B);
    ACDK_LOGD("rHorizon.i4R = %d\n", rAWBNVRAM.rLightAWBGain.rHorizon.i4R);
    ACDK_LOGD("rHorizon.i4G = %d\n", rAWBNVRAM.rLightAWBGain.rHorizon.i4G);
    ACDK_LOGD("rHorizon.i4B = %d\n", rAWBNVRAM.rLightAWBGain.rHorizon.i4B);
    ACDK_LOGD("rA.i4R = %d\n", rAWBNVRAM.rLightAWBGain.rA.i4R);
    ACDK_LOGD("rA.i4G = %d\n", rAWBNVRAM.rLightAWBGain.rA.i4G);
    ACDK_LOGD("rA.i4B = %d\n", rAWBNVRAM.rLightAWBGain.rA.i4B);
    ACDK_LOGD("rTL84.i4R = %d\n", rAWBNVRAM.rLightAWBGain.rTL84.i4R);
    ACDK_LOGD("rTL84.i4G = %d\n", rAWBNVRAM.rLightAWBGain.rTL84.i4G);
    ACDK_LOGD("rTL84.i4B = %d\n", rAWBNVRAM.rLightAWBGain.rTL84.i4B);
    ACDK_LOGD("rCWF.i4R = %d\n", rAWBNVRAM.rLightAWBGain.rCWF.i4R);
    ACDK_LOGD("rCWF.i4G = %d\n", rAWBNVRAM.rLightAWBGain.rCWF.i4G);
    ACDK_LOGD("rCWF.i4B = %d\n", rAWBNVRAM.rLightAWBGain.rCWF.i4B);
    ACDK_LOGD("rDNP.i4R = %d\n", rAWBNVRAM.rLightAWBGain.rDNP.i4R);
    ACDK_LOGD("rDNP.i4G = %d\n", rAWBNVRAM.rLightAWBGain.rDNP.i4G);
    ACDK_LOGD("rDNP.i4B = %d\n", rAWBNVRAM.rLightAWBGain.rDNP.i4B);
    ACDK_LOGD("rD65.i4R = %d\n", rAWBNVRAM.rLightAWBGain.rD65.i4R);
    ACDK_LOGD("rD65.i4G = %d\n", rAWBNVRAM.rLightAWBGain.rD65.i4G);
    ACDK_LOGD("rD65.i4B = %d\n", rAWBNVRAM.rLightAWBGain.rD65.i4B);
    ACDK_LOGD("rDF.i4R = %d\n", rAWBNVRAM.rLightAWBGain.rDF.i4R);
    ACDK_LOGD("rDF.i4G = %d\n", rAWBNVRAM.rLightAWBGain.rDF.i4G);
    ACDK_LOGD("rDF.i4B = %d\n", rAWBNVRAM.rLightAWBGain.rDF.i4B);

    ACDK_LOGD("rRotationMatrix.i4RotationAngle = %d\n", rAWBNVRAM.rRotationMatrix.i4RotationAngle);
    ACDK_LOGD("rRotationMatrix.i4Cos = %d\n", rAWBNVRAM.rRotationMatrix.i4Cos);
    ACDK_LOGD("rRotationMatrix.i4Sin = %d\n", rAWBNVRAM.rRotationMatrix.i4Sin);

    ACDK_LOGD("rDaylightLocus.i4SlopeNumerator = %d\n", rAWBNVRAM.rDaylightLocus.i4SlopeNumerator);
    ACDK_LOGD("rDaylightLocus.i4SlopeDenominator = %d\n", rAWBNVRAM.rDaylightLocus.i4SlopeDenominator);

    ACDK_LOGD("rAWBLightArea.rStrobe.i4RightBound = %d\n", rAWBNVRAM.rAWBLightArea.rStrobe.i4RightBound);
    ACDK_LOGD("rAWBLightArea.rStrobe.i4LeftBound = %d\n", rAWBNVRAM.rAWBLightArea.rStrobe.i4LeftBound);
    ACDK_LOGD("rAWBLightArea.rStrobe.i4UpperBound = %d\n", rAWBNVRAM.rAWBLightArea.rStrobe.i4UpperBound);
    ACDK_LOGD("rAWBLightArea.rStrobe.i4LowerBound = %d\n", rAWBNVRAM.rAWBLightArea.rStrobe.i4LowerBound);
    ACDK_LOGD("rAWBLightArea.rTungsten.i4RightBound = %d\n", rAWBNVRAM.rAWBLightArea.rTungsten.i4RightBound);
    ACDK_LOGD("rAWBLightArea.rTungsten.i4LeftBound = %d\n", rAWBNVRAM.rAWBLightArea.rTungsten.i4LeftBound);
    ACDK_LOGD("rAWBLightArea.rTungsten.i4UpperBound = %d\n", rAWBNVRAM.rAWBLightArea.rTungsten.i4UpperBound);
    ACDK_LOGD("rAWBLightArea.rTungsten.i4LowerBound = %d\n", rAWBNVRAM.rAWBLightArea.rTungsten.i4LowerBound);
    ACDK_LOGD("rAWBLightArea.rWarmFluorescent.i4RightBound = %d\n", rAWBNVRAM.rAWBLightArea.rWarmFluorescent.i4RightBound);
    ACDK_LOGD("rAWBLightArea.rWarmFluorescent.i4LeftBound = %d\n", rAWBNVRAM.rAWBLightArea.rWarmFluorescent.i4LeftBound);
    ACDK_LOGD("rAWBLightArea.rWarmFluorescent.i4UpperBound = %d\n", rAWBNVRAM.rAWBLightArea.rWarmFluorescent.i4UpperBound);
    ACDK_LOGD("rAWBLightArea.rWarmFluorescent.i4LowerBound = %d\n", rAWBNVRAM.rAWBLightArea.rWarmFluorescent.i4LowerBound);
    ACDK_LOGD("rAWBLightArea.rFluorescent.i4RightBound = %d\n", rAWBNVRAM.rAWBLightArea.rFluorescent.i4RightBound);
    ACDK_LOGD("rAWBLightArea.rFluorescent.i4LeftBound = %d\n", rAWBNVRAM.rAWBLightArea.rFluorescent.i4LeftBound);
    ACDK_LOGD("rAWBLightArea.rFluorescent.i4UpperBound = %d\n", rAWBNVRAM.rAWBLightArea.rFluorescent.i4UpperBound);
    ACDK_LOGD("rAWBLightArea.rFluorescent.i4LowerBound = %d\n", rAWBNVRAM.rAWBLightArea.rFluorescent.i4LowerBound);
    ACDK_LOGD("rAWBLightArea.rCWF.i4RightBound = %d\n", rAWBNVRAM.rAWBLightArea.rCWF.i4RightBound);
    ACDK_LOGD("rAWBLightArea.rCWF.i4LeftBound = %d\n", rAWBNVRAM.rAWBLightArea.rCWF.i4LeftBound);
    ACDK_LOGD("rAWBLightArea.rCWF.i4UpperBound = %d\n", rAWBNVRAM.rAWBLightArea.rCWF.i4UpperBound);
    ACDK_LOGD("rAWBLightArea.rCWF.i4LowerBound = %d\n", rAWBNVRAM.rAWBLightArea.rCWF.i4LowerBound);
    ACDK_LOGD("rAWBLightArea.rDaylight.i4RightBound = %d\n", rAWBNVRAM.rAWBLightArea.rDaylight.i4RightBound);
    ACDK_LOGD("rAWBLightArea.rDaylight.i4LeftBound = %d\n", rAWBNVRAM.rAWBLightArea.rDaylight.i4LeftBound);
    ACDK_LOGD("rAWBLightArea.rDaylight.i4UpperBound = %d\n", rAWBNVRAM.rAWBLightArea.rDaylight.i4UpperBound);
    ACDK_LOGD("rAWBLightArea.rDaylight.i4LowerBound = %d\n", rAWBNVRAM.rAWBLightArea.rDaylight.i4LowerBound);
    ACDK_LOGD("rAWBLightArea.rShade.i4RightBound = %d\n", rAWBNVRAM.rAWBLightArea.rShade.i4RightBound);
    ACDK_LOGD("rAWBLightArea.rShade.i4LeftBound = %d\n", rAWBNVRAM.rAWBLightArea.rShade.i4LeftBound);
    ACDK_LOGD("rAWBLightArea.rShade.i4UpperBound = %d\n", rAWBNVRAM.rAWBLightArea.rShade.i4UpperBound);
    ACDK_LOGD("rAWBLightArea.rShade.i4LowerBound = %d\n", rAWBNVRAM.rAWBLightArea.rShade.i4LowerBound);
    ACDK_LOGD("rAWBLightArea.rDaylightFluorescent.i4RightBound = %d\n", rAWBNVRAM.rAWBLightArea.rDaylightFluorescent.i4RightBound);
    ACDK_LOGD("rAWBLightArea.rDaylightFluorescent.i4LeftBound = %d\n", rAWBNVRAM.rAWBLightArea.rDaylightFluorescent.i4LeftBound);
    ACDK_LOGD("rAWBLightArea.rDaylightFluorescent.i4UpperBound = %d\n", rAWBNVRAM.rAWBLightArea.rDaylightFluorescent.i4UpperBound);
    ACDK_LOGD("rAWBLightArea.rDaylightFluorescent.i4LowerBound = %d\n", rAWBNVRAM.rAWBLightArea.rDaylightFluorescent.i4LowerBound);

    ACDK_LOGD("rPWBLightArea.rReferenceArea.i4RightBound = %d\n", rAWBNVRAM.rPWBLightArea.rReferenceArea.i4RightBound);
    ACDK_LOGD("rPWBLightArea.rReferenceArea.i4LeftBound = %d\n", rAWBNVRAM.rPWBLightArea.rReferenceArea.i4LeftBound);
    ACDK_LOGD("rPWBLightArea.rReferenceArea.i4UpperBound = %d\n", rAWBNVRAM.rPWBLightArea.rReferenceArea.i4UpperBound);
    ACDK_LOGD("rPWBLightArea.rReferenceArea.i4LowerBound = %d\n", rAWBNVRAM.rPWBLightArea.rReferenceArea.i4LowerBound);
    ACDK_LOGD("rPWBLightArea.rDaylight.i4RightBound = %d\n", rAWBNVRAM.rPWBLightArea.rDaylight.i4RightBound);
    ACDK_LOGD("rPWBLightArea.rDaylight.i4LeftBound = %d\n", rAWBNVRAM.rPWBLightArea.rDaylight.i4LeftBound);
    ACDK_LOGD("rPWBLightArea.rDaylight.i4UpperBound = %d\n", rAWBNVRAM.rPWBLightArea.rDaylight.i4UpperBound);
    ACDK_LOGD("rPWBLightArea.rDaylight.i4LowerBound = %d\n", rAWBNVRAM.rPWBLightArea.rDaylight.i4LowerBound);
    ACDK_LOGD("rPWBLightArea.rCloudyDaylight.i4RightBound = %d\n", rAWBNVRAM.rPWBLightArea.rCloudyDaylight.i4RightBound);
    ACDK_LOGD("rPWBLightArea.rCloudyDaylight.i4LeftBound = %d\n", rAWBNVRAM.rPWBLightArea.rCloudyDaylight.i4LeftBound);
    ACDK_LOGD("rPWBLightArea.rCloudyDaylight.i4UpperBound = %d\n", rAWBNVRAM.rPWBLightArea.rCloudyDaylight.i4UpperBound);
    ACDK_LOGD("rPWBLightArea.rCloudyDaylight.i4LowerBound = %d\n", rAWBNVRAM.rPWBLightArea.rCloudyDaylight.i4LowerBound);
    ACDK_LOGD("rPWBLightArea.rShade.i4RightBound = %d\n", rAWBNVRAM.rPWBLightArea.rShade.i4RightBound);
    ACDK_LOGD("rPWBLightArea.rShade.i4LeftBound = %d\n", rAWBNVRAM.rPWBLightArea.rShade.i4LeftBound);
    ACDK_LOGD("rPWBLightArea.rShade.i4UpperBound = %d\n", rAWBNVRAM.rPWBLightArea.rShade.i4UpperBound);
    ACDK_LOGD("rPWBLightArea.rShade.i4LowerBound = %d\n", rAWBNVRAM.rPWBLightArea.rShade.i4LowerBound);
    ACDK_LOGD("rPWBLightArea.rTwilight.i4RightBound = %d\n", rAWBNVRAM.rPWBLightArea.rTwilight.i4RightBound);
    ACDK_LOGD("rPWBLightArea.rTwilight.i4LeftBound = %d\n", rAWBNVRAM.rPWBLightArea.rTwilight.i4LeftBound);
    ACDK_LOGD("rPWBLightArea.rTwilight.i4UpperBound = %d\n", rAWBNVRAM.rPWBLightArea.rTwilight.i4UpperBound);
    ACDK_LOGD("rPWBLightArea.rTwilight.i4LowerBound = %d\n", rAWBNVRAM.rPWBLightArea.rTwilight.i4LowerBound);
    ACDK_LOGD("rPWBLightArea.rFluorescent.i4RightBound = %d\n", rAWBNVRAM.rPWBLightArea.rFluorescent.i4RightBound);
    ACDK_LOGD("rPWBLightArea.rFluorescent.i4LeftBound = %d\n", rAWBNVRAM.rPWBLightArea.rFluorescent.i4LeftBound);
    ACDK_LOGD("rPWBLightArea.rFluorescent.i4UpperBound = %d\n", rAWBNVRAM.rPWBLightArea.rFluorescent.i4UpperBound);
    ACDK_LOGD("rPWBLightArea.rFluorescent.i4LowerBound = %d\n", rAWBNVRAM.rPWBLightArea.rFluorescent.i4LowerBound);
    ACDK_LOGD("rPWBLightArea.rWarmFluorescent.i4RightBound = %d\n", rAWBNVRAM.rPWBLightArea.rWarmFluorescent.i4RightBound);
    ACDK_LOGD("rPWBLightArea.rWarmFluorescent.i4LeftBound = %d\n", rAWBNVRAM.rPWBLightArea.rWarmFluorescent.i4LeftBound);
    ACDK_LOGD("rPWBLightArea.rWarmFluorescent.i4UpperBound = %d\n", rAWBNVRAM.rPWBLightArea.rWarmFluorescent.i4UpperBound);
    ACDK_LOGD("rPWBLightArea.rWarmFluorescent.i4LowerBound = %d\n", rAWBNVRAM.rPWBLightArea.rWarmFluorescent.i4LowerBound);
    ACDK_LOGD("rPWBLightArea.rIncandescent.i4RightBound = %d\n", rAWBNVRAM.rPWBLightArea.rIncandescent.i4RightBound);
    ACDK_LOGD("rPWBLightArea.rIncandescent.i4LeftBound = %d\n", rAWBNVRAM.rPWBLightArea.rIncandescent.i4LeftBound);
    ACDK_LOGD("rPWBLightArea.rIncandescent.i4UpperBound = %d\n", rAWBNVRAM.rPWBLightArea.rIncandescent.i4UpperBound);
    ACDK_LOGD("rPWBLightArea.rIncandescent.i4LowerBound = %d\n", rAWBNVRAM.rPWBLightArea.rIncandescent.i4LowerBound);
    ACDK_LOGD("rPWBLightArea.rGrayWorld.i4RightBound = %d\n", rAWBNVRAM.rPWBLightArea.rGrayWorld.i4RightBound);
    ACDK_LOGD("rPWBLightArea.rGrayWorld.i4LeftBound = %d\n", rAWBNVRAM.rPWBLightArea.rGrayWorld.i4LeftBound);
    ACDK_LOGD("rPWBLightArea.rGrayWorld.i4UpperBound = %d\n", rAWBNVRAM.rPWBLightArea.rGrayWorld.i4UpperBound);
    ACDK_LOGD("rPWBLightArea.rGrayWorld.i4LowerBound = %d\n", rAWBNVRAM.rPWBLightArea.rGrayWorld.i4LowerBound);

    ACDK_LOGD("rPWBDefaultGain.rDaylight.i4R = %d\n", rAWBNVRAM.rPWBDefaultGain.rDaylight.i4R);
    ACDK_LOGD("rPWBDefaultGain.rDaylight.i4G = %d\n", rAWBNVRAM.rPWBDefaultGain.rDaylight.i4G);
    ACDK_LOGD("rPWBDefaultGain.rDaylight.i4B = %d\n", rAWBNVRAM.rPWBDefaultGain.rDaylight.i4B);
    ACDK_LOGD("rPWBDefaultGain.rCloudyDaylight.i4R = %d\n", rAWBNVRAM.rPWBDefaultGain.rCloudyDaylight.i4R);
    ACDK_LOGD("rPWBDefaultGain.rCloudyDaylight.i4G = %d\n", rAWBNVRAM.rPWBDefaultGain.rCloudyDaylight.i4G);
    ACDK_LOGD("rPWBDefaultGain.rCloudyDaylight.i4B = %d\n", rAWBNVRAM.rPWBDefaultGain.rCloudyDaylight.i4B);
    ACDK_LOGD("rPWBDefaultGain.rShade.i4R = %d\n", rAWBNVRAM.rPWBDefaultGain.rShade.i4R);
    ACDK_LOGD("rPWBDefaultGain.rShade.i4G = %d\n", rAWBNVRAM.rPWBDefaultGain.rShade.i4G);
    ACDK_LOGD("rPWBDefaultGain.rShade.i4B = %d\n", rAWBNVRAM.rPWBDefaultGain.rShade.i4B);
    ACDK_LOGD("rPWBDefaultGain.rTwilight.i4R = %d\n", rAWBNVRAM.rPWBDefaultGain.rTwilight.i4R);
    ACDK_LOGD("rPWBDefaultGain.rTwilight.i4G = %d\n", rAWBNVRAM.rPWBDefaultGain.rTwilight.i4G);
    ACDK_LOGD("rPWBDefaultGain.rTwilight.i4B = %d\n", rAWBNVRAM.rPWBDefaultGain.rTwilight.i4B);
    ACDK_LOGD("rPWBDefaultGain.rFluorescent.i4R = %d\n", rAWBNVRAM.rPWBDefaultGain.rFluorescent.i4R);
    ACDK_LOGD("rPWBDefaultGain.rFluorescent.i4G = %d\n", rAWBNVRAM.rPWBDefaultGain.rFluorescent.i4G);
    ACDK_LOGD("rPWBDefaultGain.rFluorescent.i4B = %d\n", rAWBNVRAM.rPWBDefaultGain.rFluorescent.i4B);
    ACDK_LOGD("rPWBDefaultGain.rWarmFluorescent.i4R = %d\n", rAWBNVRAM.rPWBDefaultGain.rWarmFluorescent.i4R);
    ACDK_LOGD("rPWBDefaultGain.rWarmFluorescent.i4G = %d\n", rAWBNVRAM.rPWBDefaultGain.rWarmFluorescent.i4G);
    ACDK_LOGD("rPWBDefaultGain.rWarmFluorescent.i4B = %d\n", rAWBNVRAM.rPWBDefaultGain.rWarmFluorescent.i4B);
    ACDK_LOGD("rPWBDefaultGain.rIncandescent.i4R = %d\n", rAWBNVRAM.rPWBDefaultGain.rIncandescent.i4R);
    ACDK_LOGD("rPWBDefaultGain.rIncandescent.i4G = %d\n", rAWBNVRAM.rPWBDefaultGain.rIncandescent.i4G);
    ACDK_LOGD("rPWBDefaultGain.rIncandescent.i4B = %d\n", rAWBNVRAM.rPWBDefaultGain.rIncandescent.i4B);
    ACDK_LOGD("rPWBDefaultGain.rGrayWorld.i4R = %d\n", rAWBNVRAM.rPWBDefaultGain.rGrayWorld.i4R);
    ACDK_LOGD("rPWBDefaultGain.rGrayWorld.i4G = %d\n", rAWBNVRAM.rPWBDefaultGain.rGrayWorld.i4G);
    ACDK_LOGD("rPWBDefaultGain.rGrayWorld.i4B = %d\n", rAWBNVRAM.rPWBDefaultGain.rGrayWorld.i4B);

    ACDK_LOGD("rPreferenceColor.rTungsten.i4SliderValue = %d\n", rAWBNVRAM.rPreferenceColor.rTungsten.i4SliderValue);
    ACDK_LOGD("rPreferenceColor.rTungsten.i4OffsetThr = %d\n", rAWBNVRAM.rPreferenceColor.rTungsten.i4OffsetThr);
    ACDK_LOGD("rPreferenceColor.rWarmFluorescent.i4SliderValue = %d\n", rAWBNVRAM.rPreferenceColor.rWarmFluorescent.i4SliderValue);
    ACDK_LOGD("rPreferenceColor.rWarmFluorescent.i4OffsetThr = %d\n", rAWBNVRAM.rPreferenceColor.rWarmFluorescent.i4OffsetThr);
    ACDK_LOGD("rPreferenceColor.rShade.i4SliderValue = %d\n", rAWBNVRAM.rPreferenceColor.rShade.i4SliderValue);
    ACDK_LOGD("rPreferenceColor.rShade.i4OffsetThr = %d\n", rAWBNVRAM.rPreferenceColor.rShade.i4OffsetThr);
    ACDK_LOGD("rPreferenceColor.rDaylightWBGain.i4R = %d\n", rAWBNVRAM.rPreferenceColor.rDaylightWBGain.i4R);
    ACDK_LOGD("rPreferenceColor.rDaylightWBGain.i4G = %d\n", rAWBNVRAM.rPreferenceColor.rDaylightWBGain.i4G);
    ACDK_LOGD("rPreferenceColor.rDaylightWBGain.i4B = %d\n", rAWBNVRAM.rPreferenceColor.rDaylightWBGain.i4B);
    ACDK_LOGD("rPreferenceColor.rPreferenceGain_Strobe.i4R = %d\n", rAWBNVRAM.rPreferenceColor.rPreferenceGain_Strobe.i4R);
    ACDK_LOGD("rPreferenceColor.rPreferenceGain_Strobe.i4G = %d\n", rAWBNVRAM.rPreferenceColor.rPreferenceGain_Strobe.i4G);
    ACDK_LOGD("rPreferenceColor.rPreferenceGain_Strobe.i4B = %d\n", rAWBNVRAM.rPreferenceColor.rPreferenceGain_Strobe.i4B);
    ACDK_LOGD("rPreferenceColor.rPreferenceGain_Tungsten.i4R = %d\n", rAWBNVRAM.rPreferenceColor.rPreferenceGain_Tungsten.i4R);
    ACDK_LOGD("rPreferenceColor.rPreferenceGain_Tungsten.i4G = %d\n", rAWBNVRAM.rPreferenceColor.rPreferenceGain_Tungsten.i4G);
    ACDK_LOGD("rPreferenceColor.rPreferenceGain_Tungsten.i4B = %d\n", rAWBNVRAM.rPreferenceColor.rPreferenceGain_Tungsten.i4B);
    ACDK_LOGD("rPreferenceColor.rPreferenceGain_WarmFluorescent.i4R = %d\n", rAWBNVRAM.rPreferenceColor.rPreferenceGain_WarmFluorescent.i4R);
    ACDK_LOGD("rPreferenceColor.rPreferenceGain_WarmFluorescent.i4G = %d\n", rAWBNVRAM.rPreferenceColor.rPreferenceGain_WarmFluorescent.i4G);
    ACDK_LOGD("rPreferenceColor.rPreferenceGain_WarmFluorescent.i4B = %d\n", rAWBNVRAM.rPreferenceColor.rPreferenceGain_WarmFluorescent.i4B);
    ACDK_LOGD("rPreferenceColor.rPreferenceGain_Fluorescent.i4R = %d\n", rAWBNVRAM.rPreferenceColor.rPreferenceGain_Fluorescent.i4R);
    ACDK_LOGD("rPreferenceColor.rPreferenceGain_Fluorescent.i4G = %d\n", rAWBNVRAM.rPreferenceColor.rPreferenceGain_Fluorescent.i4G);
    ACDK_LOGD("rPreferenceColor.rPreferenceGain_Fluorescent.i4B = %d\n", rAWBNVRAM.rPreferenceColor.rPreferenceGain_Fluorescent.i4B);
    ACDK_LOGD("rPreferenceColor.rPreferenceGain_CWF.i4R = %d\n", rAWBNVRAM.rPreferenceColor.rPreferenceGain_CWF.i4R);
    ACDK_LOGD("rPreferenceColor.rPreferenceGain_CWF.i4G = %d\n", rAWBNVRAM.rPreferenceColor.rPreferenceGain_CWF.i4G);
    ACDK_LOGD("rPreferenceColor.rPreferenceGain_CWF.i4B = %d\n", rAWBNVRAM.rPreferenceColor.rPreferenceGain_CWF.i4B);
    ACDK_LOGD("rPreferenceColor.rPreferenceGain_Daylight.i4R = %d\n", rAWBNVRAM.rPreferenceColor.rPreferenceGain_Daylight.i4R);
    ACDK_LOGD("rPreferenceColor.rPreferenceGain_Daylight.i4G = %d\n", rAWBNVRAM.rPreferenceColor.rPreferenceGain_Daylight.i4G);
    ACDK_LOGD("rPreferenceColor.rPreferenceGain_Daylight.i4B = %d\n", rAWBNVRAM.rPreferenceColor.rPreferenceGain_Daylight.i4B);
    ACDK_LOGD("rPreferenceColor.rPreferenceGain_Shade.i4R = %d\n", rAWBNVRAM.rPreferenceColor.rPreferenceGain_Shade.i4R);
    ACDK_LOGD("rPreferenceColor.rPreferenceGain_Shade.i4G = %d\n", rAWBNVRAM.rPreferenceColor.rPreferenceGain_Shade.i4G);
    ACDK_LOGD("rPreferenceColor.rPreferenceGain_Shade.i4B = %d\n", rAWBNVRAM.rPreferenceColor.rPreferenceGain_Shade.i4B);
    ACDK_LOGD("rPreferenceColor.rPreferenceGain_DaylightFluorescent.i4R = %d\n", rAWBNVRAM.rPreferenceColor.rPreferenceGain_DaylightFluorescent.i4R);
    ACDK_LOGD("rPreferenceColor.rPreferenceGain_DaylightFluorescent.i4G = %d\n", rAWBNVRAM.rPreferenceColor.rPreferenceGain_DaylightFluorescent.i4G);
    ACDK_LOGD("rPreferenceColor.rPreferenceGain_DaylightFluorescent.i4B = %d\n", rAWBNVRAM.rPreferenceColor.rPreferenceGain_DaylightFluorescent.i4B);

    ACDK_LOGD("rCCTEstimation.i4CCT[0] = %d\n", rAWBNVRAM.rCCTEstimation.i4CCT[0]);
    ACDK_LOGD("rCCTEstimation.i4CCT[1] = %d\n", rAWBNVRAM.rCCTEstimation.i4CCT[1]);
    ACDK_LOGD("rCCTEstimation.i4CCT[2] = %d\n", rAWBNVRAM.rCCTEstimation.i4CCT[2]);
    ACDK_LOGD("rCCTEstimation.i4CCT[3] = %d\n", rAWBNVRAM.rCCTEstimation.i4CCT[3]);
    ACDK_LOGD("rCCTEstimation.i4CCT[4] = %d\n", rAWBNVRAM.rCCTEstimation.i4CCT[4]);

    ACDK_LOGD("rCCTEstimation.i4RotatedXCoordinate[0] = %d\n", rAWBNVRAM.rCCTEstimation.i4RotatedXCoordinate[0]);
    ACDK_LOGD("rCCTEstimation.i4RotatedXCoordinate[1] = %d\n", rAWBNVRAM.rCCTEstimation.i4RotatedXCoordinate[1]);
    ACDK_LOGD("rCCTEstimation.i4RotatedXCoordinate[2] = %d\n", rAWBNVRAM.rCCTEstimation.i4RotatedXCoordinate[2]);
    ACDK_LOGD("rCCTEstimation.i4RotatedXCoordinate[3] = %d\n", rAWBNVRAM.rCCTEstimation.i4RotatedXCoordinate[3]);
    ACDK_LOGD("rCCTEstimation.i4RotatedXCoordinate[4] = %d\n", rAWBNVRAM.rCCTEstimation.i4RotatedXCoordinate[4]);




    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================


    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }

    return S_CCT_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_AWB_SAVE_AWB_PARA
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPAWBSaveAWBPara_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_AWB_SAVE_AWB_PARA\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================


    MUINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToCCT(ACDK_CCT_V2_OP_AWB_SAVE_AWB_PARA,
                                NULL,
                                0,
                                NULL,
                                0,
                                &u4RetLen);


    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================


    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }
    return S_CCT_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_OP_AWB_SET_AWB_MODE
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPAWBSetAWBMode_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_OP_AWB_SET_AWB_MODE\n");

    if (a_u4Argc != 1)
    {
        ACDK_LOGD("Usage: sAWBMode <AWBMode>\n");
        return E_CCT_CCAP_API_FAIL;
    }

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================


    MUINT32 u4RetLen = 0;
    MUINT32 u4AWBMode = (MUINT32) atoi((char *)a_pprArgv[0]);

    ACDK_LOGD("AWBMode = %d\n", u4AWBMode);


    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToCCT(ACDK_CCT_OP_AWB_SET_AWB_MODE,
                                (UINT8 *)&u4AWBMode,
                                sizeof(MUINT32),
                                NULL,
                                0,
                                &u4RetLen);


    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================


    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }

    return S_CCT_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_OP_AWB_GET_AWB_MODE
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPAWBGetAWBMode_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_OP_AWB_GET_AWB_MODE\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================

    MUINT32 u4RetLen = 0;
    MUINT32 u4AWBMode;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToCCT(ACDK_CCT_OP_AWB_GET_AWB_MODE,
                                NULL,
                                0,
                                (UINT8 *)&u4AWBMode,
                                sizeof(MUINT32),
                                &u4RetLen);


    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================


    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("u4AWBMode = %d\n", u4AWBMode);

    return S_CCT_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_AWB_GET_LIGHT_PROB
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPAWBGetLightProb_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    ACDK_AWB_LIGHT_PROBABILITY_T rAWBLightProb; // TEST ONLY

    memset(&rAWBLightProb,0, sizeof(rAWBLightProb)); // TEST ONLY

    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_AWB_GET_LIGHT_PROB\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================


    MUINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToCCT(ACDK_CCT_OP_AWB_GET_LIGHT_PROB,
                                NULL,
                                0,
                                (UINT8 *)&rAWBLightProb,
                                sizeof(ACDK_AWB_LIGHT_PROBABILITY_T),
                                &u4RetLen);

    // TEST ONLY (check AWB light probability)
    MINT32 i;
    for (i = 0; i < ACDK_AWB_LIGHT_NUM; i++) {
        ACDK_LOGD("P0[%d] = %d\n", i, rAWBLightProb.u4P0[i]);
    }
    for (i = 0; i < ACDK_AWB_LIGHT_NUM; i++) {
        ACDK_LOGD("P1[%d] = %d\n", i, rAWBLightProb.u4P1[i]);
    }
    for (i = 0; i < ACDK_AWB_LIGHT_NUM; i++) {
        ACDK_LOGD("P2[%d] = %d\n", i, rAWBLightProb.u4P2[i]);
    }
    for (i = 0; i < ACDK_AWB_LIGHT_NUM; i++) {
        ACDK_LOGD("P[%d] = %d\n", i, rAWBLightProb.u4P[i]);
    }




    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================


    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }

    return S_CCT_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_AF_OPERATION
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPAFOperation_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_AF_OPERATION\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================



    MUINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToCCT(ACDK_CCT_V2_OP_AF_OPERATION,
                                NULL,
                                0,
                                NULL,
                                0,
                                &u4RetLen);


    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================


    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }

    return S_CCT_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_MF_OPERATION
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPMFOperation_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
     if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_MF_OPERATION\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================
    MINT32 i4MFPos = (MINT32)atoi((char *)a_pprArgv[0]);

    ACDK_LOGD("MF Pos = %d\n", i4MFPos);


    MUINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToCCT(ACDK_CCT_V2_OP_MF_OPERATION,
						        (UINT8 *)&i4MFPos,
								sizeof(i4MFPos),
                                NULL,
                                0,
                                &u4RetLen);


    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================


    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }

    return S_CCT_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_GET_AF_INFO
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPGetAFInfo_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
     if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_GET_AF_INFO\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================

	ACDK_AF_INFO_T sAFInfo;


    MUINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToCCT(ACDK_CCT_V2_OP_GET_AF_INFO,
			                    NULL,
								0,
								(UINT8*)&sAFInfo,
								sizeof(sAFInfo),
                                &u4RetLen);


    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================

    ACDK_LOGD("AF Info : [AFMode] %d, [AFMeter] %d, [Curr Pos] %d\n", sAFInfo.i4AFMode, sAFInfo.i4AFMeter, sAFInfo.i4CurrPos);

    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }

    return S_CCT_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_AF_GET_BEST_POS
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPAFGetBestPos_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_AF_GET_BEST_POS\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================

	MINT32 i4AFBestPos;

    MUINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToCCT(ACDK_CCT_V2_OP_AF_GET_BEST_POS,
                                NULL,
                                0,
                                (UINT8*)&i4AFBestPos,
                                sizeof(i4AFBestPos),
                                &u4RetLen);


    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================

    ACDK_LOGD("AF Best Pos = %d\n", i4AFBestPos);

    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }

    return S_CCT_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_AF_CALI_OPERATION
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPAFCaliOperation_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_AF_CALI_OPERATION\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================

    ACDK_AF_CALI_DATA_T sCaliData;

    MUINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToCCT(ACDK_CCT_V2_OP_AF_CALI_OPERATION,
                                NULL,
                                0,
							    (UINT8*)&sCaliData,
							    sizeof(sCaliData),
                                &u4RetLen);


    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================

    ACDK_LOGD("AF Best Pos = %d\n", sCaliData.i4BestPos);


    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }

    return S_CCT_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_AF_SET_RANGE
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPAFSetRange_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_AF_SET_RANGE\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================

	FOCUS_RANGE_T sFocusRange;

    sFocusRange.i4InfPos = (MINT32)atoi((char *)a_pprArgv[0]);
    sFocusRange.i4MacroPos = (MINT32)atoi((char *)a_pprArgv[1]);

    ACDK_LOGD("Focus Range = %d to %d\n", sFocusRange.i4InfPos, sFocusRange.i4MacroPos);


    MUINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToCCT(ACDK_CCT_V2_OP_AF_SET_RANGE,
                                (UINT8*)&sFocusRange,
                                sizeof(sFocusRange),
                                NULL,
                                0,
                                &u4RetLen);


    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================


    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }

    return S_CCT_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_AF_GET_RANGE
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPAFGetRange_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
     if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_AF_GET_RANGE\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================

	FOCUS_RANGE_T sFocusRange;


    MUINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToCCT(ACDK_CCT_V2_OP_AF_GET_RANGE,
                                NULL,
                                0,
                                (UINT8*)&sFocusRange,
                                sizeof(sFocusRange),
                                &u4RetLen);


    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================

    ACDK_LOGD("Focus Range = %d to %d\n", sFocusRange.i4InfPos, sFocusRange.i4MacroPos);

    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }

    return S_CCT_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_AF_SAVE_TO_NVRAM
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPAFSaveToNVRAM_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_AF_SAVE_TO_NVRAM\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================



    MUINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToCCT(ACDK_CCT_V2_OP_AF_SAVE_TO_NVRAM,
                                NULL,
                                0,
                                NULL,
                                0,
                                &u4RetLen);


    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================


    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }

    return S_CCT_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_AF_READ
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPAFRead_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_AF_READ\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================

	NVRAM_LENS_PARA_STRUCT sLensPara;


    MUINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToCCT(ACDK_CCT_V2_OP_AF_READ,
                                NULL,
                                0,
								(UINT8*)&sLensPara,
								sizeof(sLensPara),
                                &u4RetLen);


    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================

    ACDK_LOGD("[Read AF Para]\n");
    ACDK_LOGD("[Thres Main]%d\n", sLensPara.rAFNVRAM.sAF_Coef.i4THRES_MAIN);
    ACDK_LOGD("[Thres Sub]%d\n", sLensPara.rAFNVRAM.sAF_Coef.i4THRES_MAIN);
    ACDK_LOGD("[HW_TH]%d\n", sLensPara.rAFNVRAM.sAF_TH.i4HW_TH[0][0]);
    ACDK_LOGD("[Statgain]%d\n", sLensPara.rAFNVRAM.i4StatGain);
    ACDK_LOGD("[Inf Pos]%d\n", sLensPara.rFocusRange.i4InfPos);
    ACDK_LOGD("[Macro Pos]%d\n", sLensPara.rFocusRange.i4MacroPos);

    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }

    return S_CCT_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_AF_APPLY
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPAFApply_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_AF_APPLY\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================

	NVRAM_LENS_PARA_STRUCT sLensPara;

    memset(&sLensPara,   0, sizeof(sLensPara));

    sLensPara.rAFNVRAM.sAF_Coef.i4THRES_MAIN = 1;
    sLensPara.rAFNVRAM.sAF_Coef.i4THRES_SUB = 1;
    sLensPara.rAFNVRAM.sAF_TH.i4HW_TH[0][0] = 1;

    MUINT32 u4RetLen = 0;

    /*BOOL bRet = bSendDataToCCT(ACDK_CCT_V2_OP_AF_READ,
                                NULL,
                                0,
								(UINT8*)&sLensPara,
								sizeof(sLensPara),
                                &u4RetLen);
        */

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToCCT(ACDK_CCT_V2_OP_AF_APPLY,
	   				       (UINT8*)&sLensPara,
							sizeof(sLensPara),
                            NULL,
                            0,
                            &u4RetLen);


    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================


    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }

    return S_CCT_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_AF_GET_FV
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPAFGetFV_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_AF_GET_FV\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================

	ACDK_AF_POS_T sAFPos;
	ACDK_AF_VLU_T sAFVlu;

	sAFPos.i4Num = 3;
	sAFPos.i4Pos[0] = (MINT32)atoi((char *)a_pprArgv[0]);
	sAFPos.i4Pos[1] = (MINT32)atoi((char *)a_pprArgv[1]);
	sAFPos.i4Pos[2] = (MINT32)atoi((char *)a_pprArgv[2]);
	//sAFPos.i4Pos[3] = (MINT32)atoi(a_pprArgv[3]);
	//sAFPos.i4Pos[4] = (MINT32)atoi(a_pprArgv[4]);
	//sAFPos.i4Pos[5] = (MINT32)atoi(a_pprArgv[5]);

    MUINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToCCT(ACDK_CCT_V2_OP_AF_GET_FV,
							    (UINT8*)&sAFPos,
								sizeof(sAFPos),
								(UINT8*)&sAFVlu,
								sizeof(sAFVlu),
                                &u4RetLen);


    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================

	for (MINT32 i=0; i<sAFVlu.i4Num; i++)
	{
    	ACDK_LOGD("[Pos] %4d, [Vlu] %lld\n", sAFPos.i4Pos[i], sAFVlu.i8Vlu[i]);
	}

    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }

    return S_CCT_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_OP_FLASH_ENABLE
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPFLASHEnable_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_OP_FLASH_ENABLE\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================


    MUINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToCCT(ACDK_CCT_OP_FLASH_ENABLE,
                                NULL,
                                0,
                                NULL,
                                0,
                                &u4RetLen);


    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================


    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }

    return S_CCT_CCAP_OK;

}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_OP_FLASH_DISABLE
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPFLASHDisable_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
     if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_OP_FLASH_DISABLE\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================



    MUINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToCCT(ACDK_CCT_OP_FLASH_DISABLE,
                                NULL,
                                0,
                                NULL,
                                0,
                                &u4RetLen);


    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================


    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }

    return S_CCT_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_OP_FLASH_GET_INFO
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPFLASHGetInfo_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_OP_FLASH_GET_INFO\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================

    MUINT32 u4RetLen = 0;
    MINT32 i4FlashEnable;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToCCT(ACDK_CCT_OP_FLASH_GET_INFO,
                                NULL,
                                0,
                                (UINT8 *)&i4FlashEnable,
                                sizeof(MINT32),
                                &u4RetLen);


    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================


    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("i4FlashEnable = %d\n", i4FlashEnable);

    return S_CCT_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_AWB_ENABLE_DYNAMIC_CCM
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPEnableDyanmicCCM_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_AWB_ENABLE_DYNAMIC_CCM\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = ::bSendDataToCCT(
        ACDK_CCT_V2_OP_AWB_ENABLE_DYNAMIC_CCM,
        NULL, 0,
        NULL, 0, NULL
    );

    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================

    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }

    return S_CCT_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_AWB_DISABLE_DYNAMIC_CCM
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPDisableDynamicCCM_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_AWB_DISABLE_DYNAMIC_CCM\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = ::bSendDataToCCT(
        ACDK_CCT_V2_OP_AWB_DISABLE_DYNAMIC_CCM,
        NULL, 0,
        NULL, 0, NULL
    );

    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================

    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }
    return S_CCT_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_AWB_GET_CCM_PARA
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPGetCCMPara_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_AWB_GET_CCM_PARA\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================
    ACDK_CCT_NVRAM_CCM_PARA ccms;
    ::memset(&ccms, 0, sizeof(ccms));

    MUINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = ::bSendDataToCCT(
        ACDK_CCT_V2_OP_AWB_GET_CCM_PARA,
        NULL, 0,
        &ccms, sizeof(ccms),
        &u4RetLen
    );

    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================

    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("Get CCM Paras\n");
    for (MUINT32 i = 0; i < 3; i++)
    {
        printf("CCM:%d\n", i);
        ACDK_CCT_CCM_STRUCT& ccm = ccms.ccm[i];
        printf("M11 M12 M13 : 0x%03X 0x%03X 0x%03X\n", ccm.M11, ccm.M12, ccm.M13);
        printf("M21 M22 M23 : 0x%03X 0x%03X 0x%03X\n", ccm.M21, ccm.M22, ccm.M23);
        printf("M31 M32 M33 : 0x%03X 0x%03X 0x%03X\n", ccm.M31, ccm.M32, ccm.M33);
        printf("\n");
    }
    return S_CCT_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_AWB_GET_CCM_STATUS
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPGetCCMStatus_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_AWB_GET_CCM_STATUS\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================
    ACDK_CCT_FUNCTION_ENABLE_STRUCT rCCMStatus;
    ::memset(&rCCMStatus, 0, sizeof(rCCMStatus));

    MUINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = ::bSendDataToCCT(
        ACDK_CCT_V2_OP_AWB_GET_CCM_STATUS,
        NULL, 0,
        &rCCMStatus, sizeof(rCCMStatus),
        &u4RetLen
    );

    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================
    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }
    printf("Dynamic CCM Status:%d\n", rCCMStatus.Enable);

    return S_CCT_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_AWB_GET_CURRENT_CCM
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPGetCurrentCCM_Cmd(const MUINT32 a_u4Argc, MUINT8*a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_AWB_GET_CURRENT_CCM\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================
    ACDK_CCT_CCM_STRUCT ccm;
    ::memset(&ccm, 0, sizeof(ccm));
    MUINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = ::bSendDataToCCT(
        ACDK_CCT_V2_OP_AWB_GET_CURRENT_CCM,
        NULL, 0,
        &ccm, sizeof(ccm), &u4RetLen
    );

    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================
    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("Get Current CCM \n");
    printf("M11 M12 M13 : 0x%03X 0x%03X 0x%03X\n", ccm.M11, ccm.M12, ccm.M13);
    printf("M21 M22 M23 : 0x%03X 0x%03X 0x%03X\n", ccm.M21, ccm.M22, ccm.M23);
    printf("M31 M32 M33 : 0x%03X 0x%03X 0x%03X\n", ccm.M31, ccm.M32, ccm.M33);
    printf("\n");
    return S_CCT_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_AWB_GET_NVRAM_CCM
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPGetNVRAMCCM_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_AWB_GET_NVRAM_CCM\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================
    //  in
    MUINT32 u4Index = 1;
    //  out
    ACDK_CCT_CCM_STRUCT ccm;
    ::memset(&ccm, 0, sizeof(ccm));

    MUINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = ::bSendDataToCCT(
        ACDK_CCT_V2_OP_AWB_GET_NVRAM_CCM,
        &u4Index, sizeof(u4Index),
        &ccm, sizeof(ccm),
        &u4RetLen
    );


    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================
    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("Get NVRAM CCM\n");
    ACDK_LOGD("Light Mode:%d\n", u4Index);
    ACDK_LOGD("CCM Matrix\n");
    printf("M11 M12 M13 : 0x%03X 0x%03X 0x%03X\n", ccm.M11, ccm.M12, ccm.M13);
    printf("M21 M22 M23 : 0x%03X 0x%03X 0x%03X\n", ccm.M21, ccm.M22, ccm.M23);
    printf("M31 M32 M33 : 0x%03X 0x%03X 0x%03X\n", ccm.M31, ccm.M32, ccm.M33);
    printf("\n");
    return S_CCT_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_AWB_SET_CURRENT_CCM
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPSetCurrentCCM_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_AWB_SET_CURRENT_CCM\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================
    ACDK_CCT_CCM_STRUCT ccm;
    ::memset(&ccm, 0, sizeof(ccm));

    ccm.M11 = 0x123;
    ccm.M12 = 0;
    ccm.M13 = 0;

    ccm.M21 = 0;
/*Yosen mark out
>>>> ORIGINAL //ALPS_SW_PERSONAL/cylen.yao/ALPS.JB_6572.EP/alps/mediatek/platform/mt6572/external/meta/cameratool/test/ccapTest/AcdkCCAPTest.cpp#1
    ccm.M22 = 0x100;
==== THEIRS //ALPS_SW_PERSONAL/cylen.yao/ALPS.JB_6572.EP/alps/mediatek/platform/mt6572/external/meta/cameratool/test/ccapTest/AcdkCCAPTest.cpp#2
    ccm.M22 = 0x156;
==== YOURS //ws_yang.yang_3010_linuxhome/ALPS_SW/TRUNK/ALPS.JB2/alps/mediatek/platform/mt6572/external/meta/cameratool/test/ccapTest/AcdkCCAPTest.cpp
Yosen mark out*/
    ccm.M22 = 0x0;
//<<<<
    ccm.M23 = 0;

    ccm.M31 = 0;
    ccm.M32 = 0;
/*Yosen mark out
>>>> ORIGINAL //ALPS_SW_PERSONAL/cylen.yao/ALPS.JB_6572.EP/alps/mediatek/platform/mt6572/external/meta/cameratool/test/ccapTest/AcdkCCAPTest.cpp#1
    ccm.M33 = 0x100;
==== THEIRS //ALPS_SW_PERSONAL/cylen.yao/ALPS.JB_6572.EP/alps/mediatek/platform/mt6572/external/meta/cameratool/test/ccapTest/AcdkCCAPTest.cpp#2
    ccm.M33 = 0x178;
==== YOURS //ws_yang.yang_3010_linuxhome/ALPS_SW/TRUNK/ALPS.JB2/alps/mediatek/platform/mt6572/external/meta/cameratool/test/ccapTest/AcdkCCAPTest.cpp
Yosen mark out*/
    ccm.M33 = 0x0;
//<<<<

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = ::bSendDataToCCT(
        ACDK_CCT_V2_OP_AWB_SET_CURRENT_CCM,
        &ccm,
        sizeof(ccm),
        NULL, 0, NULL
    );

    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================

    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }
    return S_CCT_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// MSDK_CCT_V2_OP_AWB_SET_NVRAM_CCM
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPSetNVRAMCCM_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("MSDK_CCT_V2_OP_AWB_SET_NVRAM_CCM\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================
    ACDK_CCT_SET_NVRAM_CCM  inCCM;
    //! Fixme
    //Just for test, fill the right data from CCT
    inCCM.u4Index = 1;

    ::memset(&inCCM.ccm, 0, sizeof(inCCM.ccm));
    inCCM.ccm.M11 = 0x100;
    inCCM.ccm.M12 = 0;
    inCCM.ccm.M13 = 0;

    inCCM.ccm.M21 = 0;
    inCCM.ccm.M22 = 0x100;
    inCCM.ccm.M23 = 0;

    inCCM.ccm.M31 = 0;
    inCCM.ccm.M32 = 0;
    inCCM.ccm.M33 = 0x100;

    MUINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = ::bSendDataToCCT(
        ACDK_CCT_V2_OP_AWB_SET_NVRAM_CCM,
        &inCCM,
        sizeof(inCCM),
        NULL, 0, NULL
    );

    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================

    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }
    return S_CCT_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_AWB_UPDATE_CCM_PARA
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPUpdateCCMPara_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_AWB_UPDATE_CCM_PARA\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================
    ACDK_CCT_NVRAM_CCM_PARA ccms;
    ::memset(&ccms, 0, sizeof(ccms));

    //lightsource 1
    ACDK_CCT_CCM_STRUCT& rCcm0 = ccms.ccm[0];
    rCcm0.M11 = 0x1D8;
    rCcm0.M12 = 0x528;
    rCcm0.M13 = 0x50;
    rCcm0.M21 = 0x440;
    rCcm0.M22 = 0x168;
    rCcm0.M23 = 0x428;
    rCcm0.M31 = 0x420;
    rCcm0.M32 = 0x4B8;
    rCcm0.M33 = 0x1D8;
    //lightsource 2
    ACDK_CCT_CCM_STRUCT& rCcm1 = ccms.ccm[1];
    rCcm1.M11 = 0x188;
    rCcm1.M12 = 0x468;
    rCcm1.M13 = 0x420;
    rCcm1.M21 = 0x400;
    rCcm1.M22 = 0x148;
    rCcm1.M23 = 0x448;
    rCcm1.M31 = 0x000;
    rCcm1.M32 = 0x438;
    rCcm1.M33 = 0x130;
    //lightsource 3
    ACDK_CCT_CCM_STRUCT& rCcm2 = ccms.ccm[2];
    rCcm2.M11 = 0x1C8;
    rCcm2.M12 = 0x488;
    rCcm2.M13 = 0x440;
    rCcm2.M21 = 0x458;
    rCcm2.M22 = 0x1E0;
    rCcm2.M23 = 0x488;
    rCcm2.M31 = 0x430;
    rCcm2.M32 = 0x498;
    rCcm2.M33 = 0x1C8;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = ::bSendDataToCCT(
        ACDK_CCT_V2_OP_AWB_UPDATE_CCM_PARA,
        &ccms, sizeof(ccms),
        NULL, 0, NULL
    );

    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================

    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }
    return S_CCT_CCAP_OK;

}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_AWB_UPDATE_CCM_STATUS
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPUpdateCCMStatus_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_AWB_UPDATE_CCM_STATUS\n");
    if (a_u4Argc != 1)
    {
        ACDK_LOGD("Usage: 510 <On/Off, ON:1, OFF:0>\n");
        return E_CCT_CCAP_API_FAIL;
    }
    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================
    ACDK_CCT_FUNCTION_ENABLE_STRUCT ccmStatus;

    if (atoi((char *)a_pprArgv[0]) == 0)
    {
        ccmStatus.Enable = MFALSE;
    }
    else
    {
        ccmStatus.Enable = MTRUE;
    }

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = ::bSendDataToCCT(
        ACDK_CCT_V2_OP_AWB_UPDATE_CCM_STATUS,
        &ccmStatus, sizeof(ccmStatus),
        NULL, 0, NULL
    );

    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================

    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }

    return S_CCT_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_OP_SET_CCM_MODE
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPSetCCMMode_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_OP_SET_CCM_MODE\n");
    if (1 != a_u4Argc)
    {
        ACDK_LOGD("Usage: setCCMMode <index 0~2>\n");
        return E_CCT_CCAP_API_FAIL;
    }
    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================
    MUINT32 u4Index = atoi((char *)a_pprArgv[0]);

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = ::bSendDataToCCT(
        ACDK_CCT_OP_SET_CCM_MODE,
        &u4Index, sizeof(u4Index),
        NULL, 0, NULL
    );

    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================

    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }

    return S_CCT_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_OP_GET_CCM_MODE
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPGetCCMMode_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_OP_GET_CCM_MODE\n");
    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================
    MUINT32 u4Index = 0;

    MUINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = ::bSendDataToCCT(
        ACDK_CCT_OP_GET_CCM_MODE,
        NULL, 0,
        &u4Index, sizeof(u4Index), &u4RetLen
    );
    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================

    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("CCM Index = %d\n", u4Index);

    return S_CCT_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// FT_MSDK_CCT_V2_OP_AE_SET_GAMMA_BYPASS
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPSetGammaBypass_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("FT_MSDK_CCT_V2_OP_AE_SET_GAMMA_BYPASS\n");
    if (a_u4Argc != 1)
    {
        ACDK_LOGD("Usage: 601 <On/Off, ON:1, OFF:0>\n");
        return E_CCT_CCAP_API_FAIL;
    }
    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================
    ACDK_CCT_FUNCTION_ENABLE_STRUCT gammaCtrl;
    memset(&gammaCtrl, 0, sizeof(ACDK_CCT_FUNCTION_ENABLE_STRUCT));

    if (atoi((char *)a_pprArgv[0]) == 0)
    {
        gammaCtrl.Enable = MFALSE;
    }
    else
    {
        gammaCtrl.Enable = MTRUE;
    }

    MUINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToCCT(ACDK_CCT_V2_OP_AE_SET_GAMMA_BYPASS,
                                                        (UINT8*)&gammaCtrl,
                                                        sizeof(ACDK_CCT_FUNCTION_ENABLE_STRUCT),
                                                        NULL,
                                                        0,
                                                        &u4RetLen);


    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================


    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }
    return S_CCT_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// FT_MSDK_CCT_V2_OP_AE_GET_GAMMA_BYPASS_FLAG
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPGetGammaBypassFlag_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("FT_MSDK_CCT_V2_OP_AE_GET_GAMMA_BYPASS_FLAG\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================
    ACDK_CCT_FUNCTION_ENABLE_STRUCT gammCtrlStatus;
    memset(&gammCtrlStatus, 0, sizeof(ACDK_CCT_FUNCTION_ENABLE_STRUCT));

    MUINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToCCT(ACDK_CCT_V2_OP_AE_GET_GAMMA_BYPASS_FLAG,
                                                        NULL,
                                                        0,
                                                        (UINT8*)&gammCtrlStatus,
                                                        sizeof(ACDK_CCT_FUNCTION_ENABLE_STRUCT),
                                                        &u4RetLen);


    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================


    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }

    printf("Gamma Bypass:%d\n", gammCtrlStatus.Enable);

    return S_CCT_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// FT_MSDK_CCT_V2_OP_AE_GET_GAMMA_TABLE
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPGetGammaTable_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{

    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("FT_MSDK_CCT_V2_OP_AE_GET_GAMMA_TABLE\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================
    ACDK_CCT_GAMMA_TABLE_STRUCT gammaTable;
    memset(&gammaTable, 0, sizeof(ACDK_CCT_GAMMA_TABLE_STRUCT));

    MUINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToCCT(ACDK_CCT_V2_OP_AE_GET_GAMMA_TABLE,
                                                        NULL,
                                                        0,
                                                        (UINT8 *)&gammaTable,
                                                        sizeof(ACDK_CCT_GAMMA_TABLE_STRUCT),
                                                        &u4RetLen);


    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================
   printf("R - Gamma Table:\n");
   for (int i = 0; i < GAMMA_STEP_NO; i++)
   {
        printf("0x%x, ", gammaTable.r_tbl[i]);
   }
   printf("\n");
   printf("G - Gamma Table:\n");
   for (int i = 0; i < GAMMA_STEP_NO; i++)
   {
        printf("0x%x, ", gammaTable.g_tbl[i]);
   }
   printf("\n");
   printf("B - Gamma Table:\n");
   for (int i = 0; i < GAMMA_STEP_NO; i++)
   {
        printf("0x%x, ", gammaTable.b_tbl[i]);
   }
   printf("\n");

   return S_CCT_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_AE_SET_GAMMA_TABLE
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPSetGammaTable_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{

    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_AE_GET_GAMMA_TABLE\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================
    ACDK_CCT_GAMMA_TABLE_STRUCT gammaTable;
    memset(&gammaTable, 0, sizeof(ACDK_CCT_GAMMA_TABLE_STRUCT));

    for(int i=0;i<GAMMA_STEP_NO; i++) {
        gammaTable.r_tbl[i] = i;
        gammaTable.g_tbl[i] = i;
        gammaTable.b_tbl[i] = i;
    }

    MUINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToCCT(ACDK_CCT_V2_OP_AE_SET_GAMMA_TABLE,
                                                        (UINT8 *)&gammaTable,
                                                        sizeof(ACDK_CCT_GAMMA_TABLE_STRUCT),
                                                        NULL,
                                                        0,
                                                        &u4RetLen);


    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================

    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }

    return S_CCT_CCAP_OK;
}


/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_OP_ISP_SET_PCA_TABLE
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPSetPcaTable_Cmd(const MUINT32 a_u4Argc, MUINT8*a_pprArgv[])
{
#if 0
    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_OP_ISP_SET_PCA_TABLE\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================
    ACDK_CCT_ACCESS_NVRAM_PCA_TABLE access_set;
    ::memset(&access_set, 0, sizeof(access_set));

    if (a_u4Argc != 1)
    {
        ACDK_LOGD("Usage: setPcaTable <color temperature: 0~2 >\n");
        return E_CCT_CCAP_API_FAIL;
    }
    access_set.u4Offset = 0;
    access_set.u4Count  = PCA_BIN_NUM;
    access_set.u8ColorTemperature = ::atoi((char *)a_pprArgv[0]);
    for (MUINT32 i = 0; i < access_set.u4Count; i++)
    {
        ISP_NVRAM_PCA_BIN_T* p = &access_set.buffer[i];
        p->hue_shift= rand();
        p->sat_gain = rand();
        p->y_gain   = rand();
    }

    ACDK_CCT_ACCESS_NVRAM_PCA_TABLE access_get;
    ::memset(&access_get, 0, sizeof(access_get));
    access_get.u4Offset = access_set.u4Offset;
    access_get.u4Count  = access_set.u4Count;
    access_get.u8ColorTemperature = access_set.u8ColorTemperature;

    MUINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet_set = ::bSendDataToCCT(
        ACDK_CCT_OP_ISP_SET_PCA_TABLE,
        &access_set, sizeof(access_set),
        NULL, 0, NULL
    );

    BOOL bRet_get = ::bSendDataToCCT(
        ACDK_CCT_OP_ISP_GET_PCA_TABLE,
        NULL, 0,
        &access_get, sizeof(access_get), &u4RetLen
    );

    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================
    if  (
            ! bRet_set
        ||  ! bRet_get
        ||  sizeof(access_get) != u4RetLen
        )
    {
        ACDK_LOGE("[ERR][ACDK_CCT_OP_ISP_SET_PCA_TABLE] "
        "(bRet_set, bRet_get, u4RetLen, sizeof(access_get))=(%d, %d, %d, %d)\n",
        bRet_set, bRet_get, u4RetLen, sizeof(access_get));
        return E_CCT_CCAP_API_FAIL;
    }

    for (MUINT32 i = 0; i < access_set.u4Count; i++)
    {
        ISP_NVRAM_PCA_BIN_T* p_set = &access_set.buffer[i];
        ISP_NVRAM_PCA_BIN_T* p_get = &access_get.buffer[i];

        if  ( ::memcmp(p_set, p_get, sizeof(MUINT32)) )
        {
            printf("[%d] Mismatch \n", i);
            printf("set[%d %d %d %d] ", p_set->reserved, p_set->hue_shift, p_set->sat_gain, p_set->y_gain);
            printf("get[%d %d %d %d] ", p_get->reserved, p_get->hue_shift, p_get->sat_gain, p_get->y_gain);
            return E_CCT_CCAP_API_FAIL;
        }
    }
    printf("Compare: OK\n");
#endif

    return S_CCT_CCAP_OK;
}


/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_OP_ISP_GET_PCA_TABLE
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPGetPcaTable_Cmd(const MUINT32 a_u4Argc, MUINT8*a_pprArgv[])
{
#if 0
    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_OP_ISP_GET_PCA_TABLE\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================
    MUINT32 u4RetLen = 0;
    ACDK_CCT_ACCESS_NVRAM_PCA_TABLE access;
    ::memset(&access, 0, sizeof(access));

    if (a_u4Argc != 1)
    {
        ACDK_LOGD("Usage: getPcaTable <color temperature: 0~2 >\n");
        return E_CCT_CCAP_API_FAIL;
    }
    access.u4Offset = 0;
    access.u4Count  = PCA_BIN_NUM;
    access.u8ColorTemperature = ::atoi((char *)a_pprArgv[0]);

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = ::bSendDataToCCT(
        ACDK_CCT_OP_ISP_GET_PCA_TABLE,
        NULL, 0,
        &access, sizeof(access), &u4RetLen
    );

    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================
    if  (
            ! bRet
        ||  sizeof(access) != u4RetLen
        )
    {
        ACDK_LOGE("[ERR][ACDK_CCT_OP_ISP_GET_PCA_TABLE] (bRet, u4RetLen, sizeof(access))=(%d, %d, %d)\n", bRet, u4RetLen, sizeof(access));
        return E_CCT_CCAP_API_FAIL;
    }

    for (MUINT32 i = 0; i < access.u4Count; i++)
    {
        if  (0==i%10)
            printf("\n");

        ISP_NVRAM_PCA_BIN_T* p = &access.buffer[i];
        printf("[%08X] ", *((MUINT32*)p));
    }
    printf("\n");
#endif

    return S_CCT_CCAP_OK;
}


/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_OP_ISP_SET_PCA_PARA
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPSetPcaPara_Cmd(const MUINT32 a_u4Argc, MUINT8*a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_OP_ISP_SET_PCA_PARA\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================
    ACDK_CCT_ACCESS_PCA_CONFIG access;
    ::memset(&access, 0, sizeof(access));

    if (a_u4Argc != 1)
    {
        ACDK_LOGD("Usage: setPcaPara <On/Off, ON:1, OFF:0>\n");
        return E_CCT_CCAP_API_FAIL;
    }

    access.EN = ::atoi((char *)a_pprArgv[0]);

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = ::bSendDataToCCT(
        ACDK_CCT_OP_ISP_SET_PCA_PARA,
        &access, sizeof(access),
        NULL, 0, NULL
    );

    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================
    if  (! bRet)
    {
        ACDK_LOGE("[ERR][ACDK_CCT_OP_ISP_SET_PCA_PARA] (bRet)=(%d)\n", bRet);
        return E_CCT_CCAP_API_FAIL;
    }

    printf("PCA EN : %d\n", access.EN);
    printf("\n");

    return S_CCT_CCAP_OK;
}


/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_OP_ISP_GET_PCA_PARA
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPGetPcaPara_Cmd(const MUINT32 a_u4Argc, MUINT8*a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_OP_ISP_GET_PCA_PARA\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================
    MUINT32 u4RetLen = 0;
    ACDK_CCT_ACCESS_PCA_CONFIG access;
    ::memset(&access, 0, sizeof(access));

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = ::bSendDataToCCT(
        ACDK_CCT_OP_ISP_GET_PCA_PARA,
        NULL, 0,
        &access, sizeof(access), &u4RetLen
    );

    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================
    if  (
            ! bRet
        ||  sizeof(access) != u4RetLen
        )
    {
        ACDK_LOGE("[ERR][ACDK_CCT_OP_ISP_GET_PCA_PARA] (bRet, u4RetLen, sizeof(access))=(%d, %d, %d)\n", bRet, u4RetLen, sizeof(access));
        return E_CCT_CCAP_API_FAIL;
    }

    printf("PCA EN : %d\n", access.EN);
    printf("\n");

    return S_CCT_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_OP_ISP_SET_PCA_Slider
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPSetPcaSlider_Cmd(const MUINT32 a_u4Argc, MUINT8*a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_OP_ISP_SET_PCA_Slider\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================
    ACDK_CCT_ACCESS_PCA_SLIDER pca;

    if (a_u4Argc != 3)
    {
        ACDK_LOGD("PCA Slider must contain 3 elements\n");
        return E_CCT_CCAP_API_FAIL;
    }

    pca.slider.value[0] = ::atoi((char *)a_pprArgv[0]);
    pca.slider.value[1] = ::atoi((char *)a_pprArgv[1]);
    pca.slider.value[2] = ::atoi((char *)a_pprArgv[2]);

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = ::bSendDataToCCT(
        ACDK_CCT_OP_ISP_SET_PCA_SLIDER,
        &pca, sizeof(pca),
        NULL, 0, NULL
    );

    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================
    if  (! bRet)
    {
        ACDK_LOGE("[ERR][ACDK_CCT_OP_ISP_SET_PCA_Slider] (bRet)=(%d)\n", bRet);
        return E_CCT_CCAP_API_FAIL;
    }

    printf("\n");

    return S_CCT_CCAP_OK;

}


/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_OP_ISP_GET_PCA_Slider
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPGetPcaSlider_Cmd(const MUINT32 a_u4Argc, MUINT8*a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_OP_ISP_GET_PCA_Slider\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================
    MUINT32 u4RetLen = 0;
    ACDK_CCT_ACCESS_PCA_SLIDER pca;
    ::memset(&pca, 0, sizeof(pca));

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = ::bSendDataToCCT(
        ACDK_CCT_OP_ISP_GET_PCA_SLIDER,
        NULL, 0,
        &pca, sizeof(pca), &u4RetLen
    );

    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================
    if  (
            ! bRet
        ||  sizeof(pca) != u4RetLen
        )
    {
        ACDK_LOGE("[ERR][ACDK_CCT_OP_ISP_GET_PCA_PARA] (bRet, u4RetLen, sizeof(access))=(%d, %d, %d)\n", bRet, u4RetLen, sizeof(pca));
        return E_CCT_CCAP_API_FAIL;
    }

    printf("PCA.Slider[0] : %d\n", pca.slider.value[0]);
    printf("PCA.Slider[1] : %d\n", pca.slider.value[1]);
    printf("PCA.Slider[2] : %d\n", pca.slider.value[2]);

    printf("\n");

    return S_CCT_CCAP_OK;
}


/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_OP_SET_ISP_ON
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPSetIspOn_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_OP_SET_ISP_ON\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================
    if (1 != a_u4Argc)
    {
        ACDK_LOGD("Usage: setIspOnOn <category> \n");
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_CCT_ISP_REG_CATEGORY eCategory = static_cast<ACDK_CCT_ISP_REG_CATEGORY>(::atoi((char *)a_pprArgv[0]));

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = ::bSendDataToCCT(
        ACDK_CCT_OP_SET_ISP_ON,
        &eCategory, sizeof(eCategory),
        NULL, 0, NULL
    );

    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================
    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }
    ACDK_LOGD("(eCategory)=(%d)\n", eCategory);
    return S_CCT_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_OP_SET_ISP_OFF
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPSetIspOff_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_OP_SET_ISP_OFF\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================
    if (1 != a_u4Argc)
    {
        ACDK_LOGD("Usage: setIspOnOff <category> \n");
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_CCT_ISP_REG_CATEGORY eCategory = static_cast<ACDK_CCT_ISP_REG_CATEGORY>(::atoi((char *)a_pprArgv[0]));

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = ::bSendDataToCCT(
        ACDK_CCT_OP_SET_ISP_OFF,
        &eCategory, sizeof(eCategory),
        NULL, 0, NULL
    );

    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================
    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }
    ACDK_LOGD("(eCategory)=(%d)\n", eCategory);
    return S_CCT_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_OP_GET_ISP_ON_OFF
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPGetIspOnOff_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_OP_GET_ISP_ON_OFF\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================
    if (1 != a_u4Argc)
    {
        ACDK_LOGD("Usage: getIspOnOff <category> \n");
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_CCT_ISP_REG_CATEGORY eCategory = static_cast<ACDK_CCT_ISP_REG_CATEGORY>(::atoi((char *)a_pprArgv[0]));
    ACDK_CCT_FUNCTION_ENABLE_STRUCT Ctrl;
    ::memset(&Ctrl, 0, sizeof(Ctrl));

    MUINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = ::bSendDataToCCT(
        ACDK_CCT_OP_GET_ISP_ON_OFF,
        &eCategory, sizeof(eCategory),
        &Ctrl, sizeof(Ctrl),
        &u4RetLen
    );

    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================
    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }
    ACDK_LOGD("(u4RetLen, eCategory, Ctrl.Enable)=(%d, %d, %d)\n", u4RetLen, eCategory, Ctrl.Enable);
    return S_CCT_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_ISP_SET_SHADING_INDEX
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPSetShadingIndex_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
     if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_ISP_SET_SHADING_INDEX\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================

    if (a_u4Argc != 1)
    {
        ACDK_LOGD("Usage: setLSCIndex <Index> \n");
        return E_CCT_CCAP_API_FAIL;
    }

    UINT8 index = (UINT8)atoi((char *)a_pprArgv[0]);

    MUINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToCCT(ACDK_CCT_V2_OP_ISP_SET_SHADING_INDEX,
                                                        (UINT8*)&index,
                                                        sizeof(UINT8),
                                                        NULL,
                                                        0,
                                                        &u4RetLen);


    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================
    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }
    return S_CCT_CCAP_OK;
}


/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_V2_OP_ISP_GET_SHADING_INDEX
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPGetShadingIndex_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
     if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_V2_OP_ISP_GET_SHADING_INDEX\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================

    UINT8 index = 0;

    MUINT32 u4RetLen = 0;

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToCCT(ACDK_CCT_V2_OP_ISP_GET_SHADING_INDEX,
                                                        NULL,
                                                        0,
                                                        (UINT8*)&index,
                                                        sizeof(UINT8),
                                                        &u4RetLen);


    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================
    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("Shading Index:%d\n", index);
    return S_CCT_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_OP_CDVT_SENSOR_TEST
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPCDVTSensorTest_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
     if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_OP_CDVT_SENSOR_TEST\n");

    if (a_u4Argc != 1)
    {
        ACDK_LOGD("Usage: SensorTest <Test Item: 0 ~ 4>\n");
		ACDK_LOGD("[0] Exposure Linearity \n");
		ACDK_LOGD("[1] Gain Linearity (Gain Config)\n");
		ACDK_LOGD("[2] Gain Linearity (Gain Table)\n");
		ACDK_LOGD("[3] OB Stability (Gain Config)\n");
		ACDK_LOGD("[4] OB Stability (Gain Table)\n");
        return E_CCT_CCAP_API_FAIL;
    }

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================
    MUINT32 u4RetLen = 0;
    MUINT32 u4TestItem = (MUINT32) atoi((char *)a_pprArgv[0]);

    if ((u4TestItem > 4))
    {
        ACDK_LOGD("Un-support test item\n");
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_CDVT_SENSOR_TEST_INPUT_T rSensorTestInput;

    switch(u4TestItem)
    {
    case 0: // [0] Exposure Linearity
    	ACDK_LOGD("[0] Exposure Linearity \n");
        rSensorTestInput.eTestItem = ACDK_CDVT_TEST_EXPOSURE_LINEARITY;
	    rSensorTestInput.eSensorMode = ACDK_CDVT_SENSOR_MODE_PREVIEW;
	    rSensorTestInput.rExpLinearity.eExpMode = ACDK_CDVT_EXP_MODE_TIME;
	    rSensorTestInput.rExpLinearity.i4Gain = 1024; // 1x
	    rSensorTestInput.rExpLinearity.i4ExpStart = 33333; // 1/30 sec
	    rSensorTestInput.rExpLinearity.i4ExpEnd = 66666; // 1/15 sec
	    rSensorTestInput.rExpLinearity.i4ExpInterval = 22222; // 1/30 sec
        break;
	case 1:	// [1] Gain Linearity (Gain Config)
		ACDK_LOGD("[1] Gain Linearity (Gain Config)\n");
	    rSensorTestInput.eTestItem = ACDK_CDVT_TEST_GAIN_LINEARITY;
	    rSensorTestInput.eSensorMode = ACDK_CDVT_SENSOR_MODE_PREVIEW;
		rSensorTestInput.rGainLinearityOBStability.eGainControlMode = ACDK_CDVT_GAIN_CONFIG;
		rSensorTestInput.rGainLinearityOBStability.i4ExpTime = 33333; // 1/30 sec
		rSensorTestInput.rGainLinearityOBStability.i4GainStart = 1024; // 1x
		rSensorTestInput.rGainLinearityOBStability.i4GainEnd = 2048; // 2x
		rSensorTestInput.rGainLinearityOBStability.i4GainInterval = 1024; // 1x
		break;
	case 2:	// [2] Gain Linearity (Gain Table)
		ACDK_LOGD("[2] Gain Linearity (Gain Table)\n");
	    rSensorTestInput.eTestItem = ACDK_CDVT_TEST_GAIN_LINEARITY;
	    rSensorTestInput.eSensorMode = ACDK_CDVT_SENSOR_MODE_PREVIEW;
		rSensorTestInput.rGainLinearityOBStability.eGainControlMode = ACDK_CDVT_GAIN_TABLE;
		rSensorTestInput.rGainLinearityOBStability.i4ExpTime = 33333; // 1/30 sec
		rSensorTestInput.rGainLinearityOBStability.i4GainTableSize = 2;
		rSensorTestInput.rGainLinearityOBStability.i4GainTable[0] = 1024;
		rSensorTestInput.rGainLinearityOBStability.i4GainTable[1] = 2048;
		break;
	case 3:	// [3] OB Stability (Gain Config)
		ACDK_LOGD("[3] OB Stability (Gain Config)\n");
	    rSensorTestInput.eTestItem = ACDK_CDVT_TEST_OB_STABILITY;
	    rSensorTestInput.eSensorMode = ACDK_CDVT_SENSOR_MODE_PREVIEW;
		rSensorTestInput.rGainLinearityOBStability.eGainControlMode = ACDK_CDVT_GAIN_CONFIG;
		rSensorTestInput.rGainLinearityOBStability.i4ExpTime = 33333; // 1/30 sec
		rSensorTestInput.rGainLinearityOBStability.i4GainStart = 1024; // 1x
		rSensorTestInput.rGainLinearityOBStability.i4GainEnd = 2048; // 2x
		rSensorTestInput.rGainLinearityOBStability.i4GainInterval = 1024; // 1x
		break;
	case 4:	// [4] OB Stability (Gain Table)
	    ACDK_LOGD("[4] OB Stability (Gain Table)\n");
	    rSensorTestInput.eTestItem = ACDK_CDVT_TEST_OB_STABILITY;
	    rSensorTestInput.eSensorMode = ACDK_CDVT_SENSOR_MODE_PREVIEW;
		rSensorTestInput.rGainLinearityOBStability.eGainControlMode = ACDK_CDVT_GAIN_TABLE;
		rSensorTestInput.rGainLinearityOBStability.i4ExpTime = 33333; // 1/30 sec
		rSensorTestInput.rGainLinearityOBStability.i4GainTableSize = 2;
		rSensorTestInput.rGainLinearityOBStability.i4GainTable[0] = 1024;
		rSensorTestInput.rGainLinearityOBStability.i4GainTable[1] = 2048;
		break;
    }

	ACDK_CDVT_SENSOR_TEST_OUTPUT_T rSensorTestOutput;
    memset (&rSensorTestOutput, 0, sizeof(ACDK_CDVT_SENSOR_TEST_OUTPUT_T));

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToCCT(ACDK_CCT_OP_CDVT_SENSOR_TEST,
                                (UINT8*)&rSensorTestInput,
                                sizeof(ACDK_CDVT_SENSOR_TEST_INPUT_T),
                                (UINT8*)&rSensorTestOutput,
                                sizeof(ACDK_CDVT_SENSOR_TEST_OUTPUT_T),
                                &u4RetLen);


    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================

    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("rSensorTestOutput.i4ErrorCode = %d\n", rSensorTestOutput.i4ErrorCode);
    ACDK_LOGD("rSensorTestOutput.i4TestCount = %d\n", rSensorTestOutput.i4TestCount);

	for (MINT32 i = 0; i < rSensorTestOutput.i4TestCount; i++)
	{
		ACDK_LOGD("[%d] R = %4.2f; Gr = %4.2f; Gb = %4.2f; B = %4.2f; Median = %d\n",
			      i,
                  rSensorTestOutput.rRAWAnalysisResult[i].fRAvg,
                  rSensorTestOutput.rRAWAnalysisResult[i].fGrAvg,
                  rSensorTestOutput.rRAWAnalysisResult[i].fGbAvg,
                  rSensorTestOutput.rRAWAnalysisResult[i].fBAvg,
                  rSensorTestOutput.rRAWAnalysisResult[i].u4Median);
	}

	return S_CCT_CCAP_OK;
}



//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
// flash cmd
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
MRESULT mrCCAP_ReadNvram(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
	printf("mrCCAPCDVTFlashRead_Cmd\n");
	ACDK_LOGD("mrCCAPCDVTFlashRead_Cmd\n");
	if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    MUINT32 u4RetLen = 0;
    BOOL bRet = bSendDataToCCT(ACDK_CCT_OP_STROBE_READ_NVRAM,
                                (UINT8*)0,
                                0,
                                (UINT8*)0,
                                0,
                                &u4RetLen);
    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }
    return S_CCT_CCAP_OK;
}
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
MRESULT mrCCAP_WriteNvram(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
	ACDK_LOGD("mrCCAP_WriteNvram\n");
	if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    MUINT32 u4RetLen = 0;
    BOOL bRet = bSendDataToCCT(ACDK_CCT_OP_STROBE_WRITE_NVRAM,
                                (UINT8*)0,
                                0,
                                (UINT8*)0,
                                0,
                                &u4RetLen);
    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }
    return S_CCT_CCAP_OK;
}
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
MRESULT mrCCAP_ReadDefaultNvram(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
	ACDK_LOGD("mrCCAP_ReadDefaultNvram\n");
	if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    MUINT32 u4RetLen = 0;
    BOOL bRet = bSendDataToCCT(ACDK_CCT_OP_STROBE_READ_DEFAULT_NVRAM,
                                (UINT8*)0,
                                0,
                                (UINT8*)0,
                                0,
                                &u4RetLen);
    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }
    return S_CCT_CCAP_OK;
}
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
MRESULT mrCCAP_SetParam(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
	ACDK_LOGD("mrCCAP_SetParam argc=%d\n",a_u4Argc);
	if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }
    MUINT32 u4RetLen = 0;
    int p[3];
    if(a_u4Argc==3)
    {
	    p[0] = (MUINT32) atoi((char *)a_pprArgv[0]);
	    p[1] = (MUINT32) atoi((char *)a_pprArgv[1]);
	    p[2] = (MUINT32) atoi((char *)a_pprArgv[2]);
	}
	else
	{
	    p[0] = 100;
	    p[1] = 199;
	    p[2] = 0;
	}
    BOOL bRet = bSendDataToCCT(ACDK_CCT_OP_STROBE_SET_PARAM,
                                (UINT8*)p,
                                12,
                                (UINT8*)0,
                                0,
                                &u4RetLen);

    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }
    return S_CCT_CCAP_OK;
}
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
MRESULT mrCCAP_GetParam(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
	ACDK_LOGD("mrCCAP_GetParam\n");
	if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    MUINT32 u4RetLen = 0;
    int p[3];
    int out[1];
    if(a_u4Argc==2)
    {
	    p[0] = (MUINT32) atoi((char *)a_pprArgv[0]);
	    p[1] = (MUINT32) atoi((char *)a_pprArgv[1]);
	}
	else
	{
		p[0]=100;
		p[1]=0;
	}
    BOOL bRet = bSendDataToCCT(ACDK_CCT_OP_STROBE_GET_PARAM,
                                (UINT8*)p,
                                8,
                                (UINT8*)out,
                                4,
                                &u4RetLen);
    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }
    return S_CCT_CCAP_OK;
}
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx10
MRESULT mrCCAP_GetNvdata(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
	ACDK_LOGD("mrCCAP_GetNvdata argc=%d\n", a_u4Argc);
	if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }
    MUINT32 u4RetLen = 0;
	char* nvdata;
	nvdata = new char[CCT_FL_NVDATA_SIZE];
    BOOL bRet = bSendDataToCCT(ACDK_CCT_OP_STROBE_GET_NVDATA,
                                (UINT8*)0,
                                0,
                                (UINT8*)nvdata,
                                CCT_FL_NVDATA_SIZE,
                                &u4RetLen);
	delete []nvdata;
    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }
    return S_CCT_CCAP_OK;
}
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx11
MRESULT mrCCAP_SetNvdata(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
	ACDK_LOGD("mrCCAP_SetNvdata argc=%d\n", a_u4Argc);
	if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }
    MUINT32 u4RetLen = 0;
	char* nvdata;
	nvdata = new char[CCT_FL_NVDATA_SIZE];
    BOOL bRet = bSendDataToCCT(ACDK_CCT_OP_STROBE_SET_NVDATA,
                                (UINT8*)nvdata,
                                CCT_FL_NVDATA_SIZE,
                                (UINT8*)0,
                                0,
                                &u4RetLen);
	delete []nvdata;
    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }
    return S_CCT_CCAP_OK;
}
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx12
MRESULT mrCCAP_GetEngY(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
	ACDK_LOGD("mrCCAP_GetEngY argc=%d\n", a_u4Argc);
	if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }
    MUINT32 u4RetLen = 0;
	short* engY;
	engY = new short[CCT_FL_ENG_SIZE];
    BOOL bRet = bSendDataToCCT(ACDK_CCT_OP_STROBE_GET_ENG_Y,
    							(UINT8*)0,
                                0,
                                (UINT8*)engY,
                                CCT_FL_ENG_SIZE*CCT_FL_ENG_UNIT_SIZE,
                                &u4RetLen);
	delete []engY;
    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }
    return S_CCT_CCAP_OK;
}
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx13
MRESULT mrCCAP_SetEngY(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
	ACDK_LOGD("mrCCAP_SetEngY argc=%d\n", a_u4Argc);
	if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }
    MUINT32 u4RetLen = 0;
	short* engY;
	engY = new short[CCT_FL_ENG_SIZE];
    BOOL bRet = bSendDataToCCT(ACDK_CCT_OP_STROBE_SET_ENG_Y,
    							(UINT8*)engY,
                                CCT_FL_ENG_SIZE*CCT_FL_ENG_UNIT_SIZE,
    							(UINT8*)0,
                                0,
                                &u4RetLen);
	delete []engY;
    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }
    return S_CCT_CCAP_OK;
}
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx14
MRESULT mrCCAP_GetEngRg(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
	ACDK_LOGD("mrCCAP_GetEngRg argc=%d\n", a_u4Argc);
	if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }
    MUINT32 u4RetLen = 0;
	short* engRg;
	engRg = new short[CCT_FL_ENG_SIZE];
    BOOL bRet = bSendDataToCCT(ACDK_CCT_OP_STROBE_GET_ENG_RG,
    							(UINT8*)0,
                                0,
                                (UINT8*)engRg,
                                CCT_FL_ENG_SIZE*CCT_FL_ENG_UNIT_SIZE,
                                &u4RetLen);
	delete []engRg;
    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }
    return S_CCT_CCAP_OK;
}
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx15
MRESULT mrCCAP_SetEngRg(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
	ACDK_LOGD("mrCCAP_SetEngRg argc=%d\n", a_u4Argc);
	if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }
    MUINT32 u4RetLen = 0;
	short* engRg;
	engRg = new short[CCT_FL_ENG_SIZE];
    BOOL bRet = bSendDataToCCT(ACDK_CCT_OP_STROBE_SET_ENG_RG,
    							(UINT8*)engRg,
                                CCT_FL_ENG_SIZE*CCT_FL_ENG_UNIT_SIZE,
    							(UINT8*)0,
                                0,
                                &u4RetLen);
	delete []engRg;
    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }
    return S_CCT_CCAP_OK;
}
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx16
MRESULT mrCCAP_GetEngBg(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
	ACDK_LOGD("mrCCAP_GetEngBg argc=%d\n", a_u4Argc);
	if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }
    MUINT32 u4RetLen = 0;
	short* engBg;
	engBg = new short[CCT_FL_ENG_SIZE];
    BOOL bRet = bSendDataToCCT(ACDK_CCT_OP_STROBE_GET_ENG_BG,
    							(UINT8*)0,
                                0,
                                (UINT8*)engBg,
                                CCT_FL_ENG_SIZE*CCT_FL_ENG_UNIT_SIZE,
                                &u4RetLen);
	delete []engBg;
    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }
    return S_CCT_CCAP_OK;
}
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx17
MRESULT mrCCAP_SetEngBg(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
	ACDK_LOGD("mrCCAP_SetEngBg argc=%d\n", a_u4Argc);
	if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }
    MUINT32 u4RetLen = 0;
	short* engBg;
	engBg = new short[CCT_FL_ENG_SIZE];
    BOOL bRet = bSendDataToCCT(ACDK_CCT_OP_STROBE_SET_ENG_BG,
    							(UINT8*)engBg,
                                CCT_FL_ENG_SIZE*CCT_FL_ENG_UNIT_SIZE,
    							(UINT8*)0,
                                0,
                                &u4RetLen);
	delete []engBg;
    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }
    return S_CCT_CCAP_OK;
}
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx18
MRESULT mrCCAP_NvdataToFile(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
	ACDK_LOGD("mrCCAP_NvdataToFile argc=%d\n", a_u4Argc);
	if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    MUINT32 u4RetLen = 0;

    char fname[256];
    sprintf(fname, "//sdcard//flash_nvdata.bin");
    if(a_u4Argc==1)
    {
    	strcpy(fname, (const char*)a_pprArgv[0]);
	}
    BOOL bRet = bSendDataToCCT(ACDK_CCT_OP_STROBE_NVDATA_TO_FILE,
                                (UINT8*)fname,
                                256,
                                (UINT8*)0,
                                0,
                                &u4RetLen);
    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }
    return S_CCT_CCAP_OK;
}
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx19
MRESULT mrCCAP_FileToNvdata(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
	ACDK_LOGD("mrCCAP_FileToNvdata argc=%d\n", a_u4Argc);
	if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }
    MUINT32 u4RetLen = 0;
    char fname[256];
    sprintf(fname, "//sdcard//flash_nvdata.bin");
    if(a_u4Argc==1)
    {
    	strcpy(fname, (const char*)a_pprArgv[0]);
	}
    BOOL bRet = bSendDataToCCT(ACDK_CCT_OP_STROBE_FILE_TO_NVDATA,
                                (UINT8*)fname,
                                256,
                                (UINT8*)0,
                                0,
                                &u4RetLen);


    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }
    return S_CCT_CCAP_OK;
}


////////////////

MRESULT mrCCAPCDVTFlashCalibration_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
	ACDK_LOGD("mrCCAPCDVTFlashCalibration_Cmd\n");
	if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    MUINT32 u4RetLen = 0;
    BOOL bRet = bSendDataToCCT(ACDK_CCT_OP_FLASH_CALIBRATION,
                                (UINT8*)0,
                                0,
                                (UINT8*)0,
                                0,
                                &u4RetLen);


    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }
    return S_CCT_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_OP_AE_PLINE_TABLE_TEST
/////////////////////////////////////////////////////////////////////////
MRESULT mrAEPlineTableLinearity_cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    ACDK_LOGD("mrAEPlineTableLinearity_cmd\n");
    if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_OP_AE_PLINE_TABLE_TEST\n");

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================
    MUINT32 u4RetLen = 0;
    ACDK_CDVT_AE_PLINE_TEST_INPUT_T rAEPlineTableTestInput;
    ACDK_CDVT_AE_PLINE_TEST_OUTPUT_T rAEPlineTableTestOutput;
    memset (&rAEPlineTableTestOutput, 0, sizeof(ACDK_CDVT_SENSOR_TEST_OUTPUT_T));

    rAEPlineTableTestInput.rAEPlinetableInfo.i4ShutterDelayFrame = 0;
    rAEPlineTableTestInput.rAEPlinetableInfo.i4SensorGainDelayFrame = 0;
    rAEPlineTableTestInput.rAEPlinetableInfo.i4ISPGainDelayFrame = 2;
    rAEPlineTableTestInput.rAEPlinetableInfo.i4TestSteps = 1;
    rAEPlineTableTestInput.rAEPlinetableInfo.i4RepeatTime = 2;
    rAEPlineTableTestInput.rAEPlinetableInfo.i4IndexScanStart = 0;
    rAEPlineTableTestInput.rAEPlinetableInfo.i4IndexScanEnd = 2;
    rAEPlineTableTestInput.rAEPlinetableInfo.i4PlineTable[0].i4Index = 0;
    rAEPlineTableTestInput.rAEPlinetableInfo.i4PlineTable[0].i4ShutterTime = 1000;
    rAEPlineTableTestInput.rAEPlinetableInfo.i4PlineTable[0].i4SensorGain = 2048;
    rAEPlineTableTestInput.rAEPlinetableInfo.i4PlineTable[0].i4ISPGain = 1024;
    rAEPlineTableTestInput.rAEPlinetableInfo.i4PlineTable[1].i4Index = 1;
    rAEPlineTableTestInput.rAEPlinetableInfo.i4PlineTable[1].i4ShutterTime = 2000;
    rAEPlineTableTestInput.rAEPlinetableInfo.i4PlineTable[1].i4SensorGain = 2048;
    rAEPlineTableTestInput.rAEPlinetableInfo.i4PlineTable[1].i4ISPGain = 1024;
    rAEPlineTableTestInput.rAEPlinetableInfo.i4PlineTable[2].i4Index = 2;
    rAEPlineTableTestInput.rAEPlinetableInfo.i4PlineTable[2].i4ShutterTime = 1000;
    rAEPlineTableTestInput.rAEPlinetableInfo.i4PlineTable[2].i4SensorGain = 2048;
    rAEPlineTableTestInput.rAEPlinetableInfo.i4PlineTable[2].i4ISPGain = 2048;    

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToCCT(ACDK_CCT_OP_AE_PLINE_TABLE_TEST,
                                (UINT8*)&rAEPlineTableTestInput,
                                sizeof(ACDK_CDVT_AE_PLINE_TEST_INPUT_T),
                                (UINT8*)&rAEPlineTableTestOutput,
                                sizeof(ACDK_CDVT_AE_PLINE_TEST_OUTPUT_T),
                                &u4RetLen);

    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }

    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================

    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("rAEPlineTableTestOutput.i4ErrorCode = %d\n", rAEPlineTableTestOutput.i4ErrorCode);
    ACDK_LOGD("rAEPlineTableTestOutput.i4TestCount = %d\n", rAEPlineTableTestOutput.i4TestCount);

    for (MINT32 i = 0; i < rAEPlineTableTestOutput.i4TestCount; i++)
    {
        ACDK_LOGD("[%d] Exp:%d Sensor Gain:%d ISP Gain:%d Y:%d\n",
                  rAEPlineTableTestOutput.rYAnalysisResult[i].i4Index,
                  rAEPlineTableTestOutput.rYAnalysisResult[i].i4ShutterTime,
                  rAEPlineTableTestOutput.rYAnalysisResult[i].i4SensorGain,
                  rAEPlineTableTestOutput.rYAnalysisResult[i].i4ISPGain,
                  rAEPlineTableTestOutput.rYAnalysisResult[i].i4Yvalue);
    }
    
    return S_CCT_CCAP_OK;
}

/////////////////////////////////////////////////////////////////////////
// ACDK_CCT_OP_CDVT_SENSOR_CALIBRATION
/////////////////////////////////////////////////////////////////////////
MRESULT mrCCAPCDVTSensorCalibration_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
     if (!g_bAcdkOpend )
    {
        return E_CCT_CCAP_API_FAIL;
    }

    ACDK_LOGD("ACDK_CCT_OP_CDVT_SENSOR_CALIBRATION\n");

	if (a_u4Argc != 1)
    {
        ACDK_LOGD("Usage: SensorCal <Calibration Item: 0 ~ 3>\n");
		ACDK_LOGD("[0] OB Calibration \n");
		ACDK_LOGD("[1] Minimum ISO Calibration\n");
		ACDK_LOGD("[2] Minimum Saturation Gain Calibration\n");
        return E_CCT_CCAP_API_FAIL;
    }

    //! ====================================================
    //! 1. Fill the input / output data  here
    //!     or anything you need to initialize
    //! ====================================================
    MUINT32 u4RetLen = 0;
    MUINT32 u4CalItem = (MUINT32) atoi((char *)a_pprArgv[0]);

    if ((u4CalItem > 2))
    {
        ACDK_LOGD("Un-support calibration item\n");
        return E_CCT_CCAP_API_FAIL;
    }


    ACDK_CDVT_SENSOR_CALIBRATION_INPUT_T rSensorCalibrationInput;

	switch(u4CalItem)
    {
    case 0: // [0] OB Calibration
		ACDK_LOGD("[0] OB Calibration \n");
        rSensorCalibrationInput.eCalibrationItem = ACDK_CDVT_CALIBRATION_OB;
	    rSensorCalibrationInput.eSensorMode = ACDK_CDVT_SENSOR_MODE_PREVIEW;
	    rSensorCalibrationInput.rOB.i4ExpTime = 33333; // 1/30 sec
	    rSensorCalibrationInput.rOB.i4Gain = 1024; // 1x
	    rSensorCalibrationInput.rOB.i4RepeatTimes = 3;
        break;
	case 1:	// [1] Minimum ISO Calibration
		ACDK_LOGD("[1] Minimum ISO Calibration\n");
        rSensorCalibrationInput.eCalibrationItem = ACDK_CDVT_CALIBRATION_MIN_ISO;
	    rSensorCalibrationInput.eSensorMode = ACDK_CDVT_SENSOR_MODE_PREVIEW;
	    rSensorCalibrationInput.rMinISO.i4LV = 90; // LV 9
        rSensorCalibrationInput.rMinISO.i4FNumber = 28; // F2.8
        rSensorCalibrationInput.rMinISO.eFlicker = ACDK_CDVT_FLICKER_60_HZ;
		rSensorCalibrationInput.rMinISO.i4OB = 42;
		break;
	case 2:	// [2] Minimum Saturation Gain Calibration
		ACDK_LOGD("[2] Minimum Saturation Gain Calibration\n");
        rSensorCalibrationInput.eCalibrationItem = ACDK_CDVT_CALIBRATION_MIN_SAT_GAIN;
	    rSensorCalibrationInput.eSensorMode = ACDK_CDVT_SENSOR_MODE_PREVIEW;
	    rSensorCalibrationInput.rMinSatGain.i4TargetDeclineRate = 10; // 10%
	    rSensorCalibrationInput.rMinSatGain.i4GainBuffer = 0; // 0%
	    rSensorCalibrationInput.rMinSatGain.eFlicker = ACDK_CDVT_FLICKER_60_HZ;
		rSensorCalibrationInput.rMinSatGain.i4OB = 42;
		break;
    }

	ACDK_CDVT_SENSOR_CALIBRATION_OUTPUT_T rSensorCalibrationOutput;
    memset (&rSensorCalibrationOutput, 0, sizeof(ACDK_CDVT_SENSOR_CALIBRATION_OUTPUT_T));

    //! ====================================================
    //! 2. Modify the input data, size, and output data, size
    //! ====================================================
    BOOL bRet = bSendDataToCCT(ACDK_CCT_OP_CDVT_SENSOR_CALIBRATION,
                                (UINT8*)&rSensorCalibrationInput,
                                sizeof(ACDK_CDVT_SENSOR_CALIBRATION_INPUT_T),
                                (UINT8*)&rSensorCalibrationOutput,
                                sizeof(ACDK_CDVT_SENSOR_CALIBRATION_OUTPUT_T),
                                &u4RetLen);


    //! ====================================================
    //! 3. Check the output
    //!     Do what you want to do
    //! ====================================================

    if (!bRet)
    {
        return E_CCT_CCAP_API_FAIL;
    }

	ACDK_LOGD("rSensorCalibrationOutput.i4ErrorCode = %d\n", rSensorCalibrationOutput.i4ErrorCode);
    ACDK_LOGD("rSensorCalibrationOutput.i4OB = %d\n", rSensorCalibrationOutput.i4OB);
	ACDK_LOGD("rSensorCalibrationOutput.i4MinISO = %d\n", rSensorCalibrationOutput.i4MinISO);
	ACDK_LOGD("rSensorCalibrationOutput.i4MinSatGain = %d\n", rSensorCalibrationOutput.i4MinSatGain);

    return S_CCT_CCAP_OK;
}




/////////////////////////////////////////////////////////////////////////
//
//  ACDK CCAP CLI Command List  () -
//! @brief
//! @param
/////////////////////////////////////////////////////////////////////////
static const Acdk_CLICmd g_prAcdkCCAPCLICmds [] =
{
    //Camera Control
    {"prvstart",                      "FT_MSDK_CCT_OP_PREVIEW_LCD_START",       mrCCAPPreviewStart_Cmd},
    {"prvstop",                      "FT_MSDK__CCT_OP_PREVIEW_LCD_STOP",        mrCCAPPreviewStop_Cmd},
    {"cap",                            "FT_MSDK_CCT_OP_SINGLE_SHOT_CAPTURE, (cap <mode> <type>  <width (Option)> <height (Option)>)",   mrCCAPSingleShot_Cmd},
    {"multiCap",                    "FT_MSDK_CCT_OP_MULTI_SHOT_CAPTURE, (cap <mode <type> <cnt>)",     mrCCAPMultiShot_Cmd},
    {"lnvram",              "ACDK_CCT_OP_LOAD_FROM_NVRAM",          mrCCAPLoadFromNvram_Cmd},
    {"Snvram",              "ACDK_CCT_OP_SAVE_TO_NVRAM",            mrCCAPSaveToNvram_Cmd},
    //ISP Control
    {"qIspID",              "ACDK_CCT_OP_QUERY_ISP_ID",                 mrCCAPQueryISPID_Cmd},
    {"rIspReg",             "ACDK_CCT_OP_ISP_READ_REG",                 mrCCAPReadISPReg_Cmd},
    {"wIspReg",             "ACDK_CCT_OP_ISP_WRITE_REG",                mrCCAPWriteISPReg_Cmd},
    {"setIspIndex",         "ACDK_CCT_V2_OP_ISP_SET_TUNING_INDEX",      mrCCAPSetTuningIndex_Cmd},
    {"getIspIndex",         "ACDK_CCT_V2_OP_ISP_GET_TUNING_INDEX",      mrCCAPGetTuningIndex_Cmd},
    {"setIspParas",         "ACDK_CCT_V2_OP_ISP_SET_TUNING_PARAS",      mrCCAPSetTuningParas_Cmd},
    {"getIspParas",         "ACDK_CCT_V2_OP_ISP_GET_TUNING_PARAS",      mrCCAPGetTuningParas_Cmd},
    {"setShading",                "FT_MSDK_CCT_V2_OP_ISP_SET_SHADING_ON_OFF", mrCCAPSetShadingOnOff_Cmd},
    {"getShading",                "FT_MSDK_CCT_V2_OP_ISP_GET_SHADING_ON_OFF", mrCCAPGetShadingOnOff_Cmd},
    {"setShadingPara",          "FT_MSDK_CCT_V2_OP_ISP_SET_SHADING_PARA", mrCCAPSetShadingPara_Cmd},
    {"getShadingPara",          "FT_MSDK_CCT_V2_OP_ISP_GET_SHADING_PARA", mrCCAPGetShadingPara_Cmd},
    {"setTsfAwbForce",      "ACDK_CCT_V2_OP_ISP_SET_SHADING_TSFAWB_FORCE", mrCCAPSetShadingTsfAwbForce_Cmd},
    {"setDefectOn",               "FT_MSDK_CCT_V2_ISP_DEFECT_TABLE_ON", mrCCAPDefectTblOn_Cmd},
    {"setDefectOff",               "FT_MSDK_CCT_V2_ISP_DEFECT_TABLE_OFF", mrCCAPDefectTblOff_Cmd },
    {"enBypass",            "ACDK_CCT_V2_OP_ISP_ENABLE_DYNAMIC_BYPASS_MODE", mrCCAPEnableDynamicBypass},
    {"disBypass",           "ACDK_CCT_V2_OP_ISP_DISABLE_DYNAMIC_BYPASS_MODE", mrCCAPDisableDynamicBypass },
    {"getBypass",           "FT_MSDK_CCT_V2_OP_ISP_GET_DYNAMIC_BYPASS_MODE_ON_OFF", mrCCAPGetDynamicMode_Cmd },
    {"lnvramisp",           "ACDK_CCT_OP_ISP_LOAD_FROM_NVRAM",          mrCCAPISPLoadFromNvram_Cmd},
    {"lnvram3a",           "ACDK_CCT_OP_3A_LOAD_FROM_NVRAM",          mrCCAP3ALoadFromNvram_Cmd}, //Yosen
    {"snvramisp",           "ACDK_CCT_OP_ISP_SAVE_TO_NVRAM",            mrCCAPISPSaveToNvram_Cmd},
    {"getnvramisp",           "ACDK_CCT_V2_OP_ISP_GET_NVRAM_DATA",            mrCCAPISPGetNvramData_Cmd},

    //Calibration
    {"calShading",                  "MSDK_CCT_V2_OP_SHADING_CAL", mrCCAPCalShading_Cmd},
    {"verifyShading",              "MSDK_CCT_V2_OP_SHADING_VERIFY", mrCCAPVerifyShading_Cmd},
    {"verifyDefect",                "MSDK_CCT_V2_OP_DEFECT_VERIFY", mrCCAPVerifyDefect_Cmd},
    {"calDefect",                   "FT_MSDK_CCT_OP_DEFECT_TABLE_CAL", mrCCAPDefectTblCal_Cmd},

    //Sensor Control
    {"rsensorReg",       "ACDK_CCT_OP_READ_SENSOR_REG",        mrCCAPReadSensorReg_Cmd},
    {"wsensorReg",       "ACDK_CCT_OP_WRITE_SENSOR_REG",       mrCCAPWriteSensorReg_Cmd},
    {"getres",           "ACDK_CCT_V2_OP_GET_SENSOR_RESOLUTION", mrCCAPGetSensorRes_Cmd},
    {"getlscres",        "ACDK_CCT_OP_GET_LSC_SENSOR_RESOLUTION", mrCCAPGetLSCSensorRes_Cmd},
    {"qsensor",                      "MSDK_CCT_OP_QUERY_SENSOR", mrCCAPQuerySensor_Cmd},
    {"gsensorPara",               "MSDK_CCT_OP_GET_ENG_SENSOR_PARA", mrCCAPGetEngSensorPara_Cmd},
    {"ssensorPara",               "MSDK_CCT_OP_SET_ENG_SENSOR_PARA", mrCCAPSetEngSensorPara_Cmd},
    {"gsensorGPara",             "MSDK_CCT_OP_GET_ENG_SENSOR_GROUP_PARA", mrCCAPGetEngSensorGroupPara_Cmd},
    {"gsensorGCnt",              "MSDK_CCT_OP_GET_ENG_SENSOR_GROUP_COUNT", mrCCAPGetEngSensorGroupCnt_Cmd},
    {"gsenPreGain",               "FT_MSDK_CCT_OP_GET_SENSOR_PREGAIN",             mrCCAPGetSensorPreGain_Cmd},
    {"ssenPreGain",               "FT_MSDK_CCT_OP_SET_SENSOR_PREGAIN",              mrCCAPSetSensorPreGain_Cmd},

    //AE
    {"enAE",             "ACDK_CCT_OP_AE_ENABLE",                 mrCCAPAEEnable_Cmd},
    {"disAE",            "ACDK_CCT_OP_AE_DISABLE",                mrCCAPAEDisable_Cmd},
    {"getAE",            "ACDK_CCT_OP_AE_GET_ENABLE_INFO",        mrCCAPGetAEInfo_cmd},
    {"sAEMode",          "ACDK_CCT_OP_DEV_AE_SET_SCENE_MODE",     mrCCAPAESetSceneMode_Cmd},
    {"gAEParam",         "ACDK_CCT_OP_DEV_AE_GET_INFO",           mrCCAPAEGetInfo_Cmd},
    {"gAEMode",          "ACDK_CCT_V2_OP_AE_GET_SCENE_MODE",      mrCCAPAEGetSceneMode_Cmd},
    {"sAEMeter",         "ACDK_CCT_V2_OP_AE_SET_METERING_MODE",   mrCCAPAESetMeteringMode_Cmd},
    {"sExpPara",         "ACDK_CCT_V2_OP_AE_APPLY_EXPO_INFO",     mrCCAPAEApplyExpoInfo_Cmd},
    {"sAEFlicker",       "ACDK_CCT_V2_OP_AE_SELECT_BAND",         mrCCAPAESelectBand_Cmd},
    {"gExpPara",         "ACDK_CCT_V2_OP_AE_GET_AUTO_EXPO_PARA",  mrCCAPAEGetAutoExpoPara_Cmd},
    {"gAEFlicker",       "ACDK_CCT_V2_OP_AE_GET_BAND",            mrCCAPAEGetBand_Cmd},
    {"gAEMeter",         "ACDK_CCT_V2_OP_AE_GET_METERING_RESULT", mrCCAPAEGetMeteringResoult_Cmd},
    {"aAEInfo",          "ACDK_CCT_OP_DEV_AE_APPLY_INFO",         mrCCAPAEApplyInfo_Cmd},
    {"sAEParam",         "ACDK_CCT_OP_DEV_AE_SAVE_INFO_NVRAM",    mrCCAPAESaveInfoToNVRAM_Cmd},
    {"gAECaliEV",        "ACDK_CCT_OP_DEV_AE_GET_EV_CALIBRATION", mrCCAPAEGetEVCalibration_Cmd},
    {"sAEExpLine",       "ACDK_CCT_OP_AE_SET_SENSOR_EXP_LINE",    mrCCAPAESetExpLine_Cmd},
    {"gFlareCali",        "ACDK_CCT_OP_DEV_AE_GET_FLARE_CALIBRATION", mrCCAPAEGetFlareOffsetCalibration_Cmd},

    //AWB
    {"enAWB",             "ACDK_CCT_V2_OP_AWB_ENABLE_AUTO_RUN",   mrCCAPAWBEnableAutoRun_Cmd},
    {"disAWB",            "ACDK_CCT_V2_OP_AWB_DISABLE_AUTO_RUN",  mrCCAPAWBDisableAutoRun_Cmd},
    {"gAWBEnableInfo",    "ACDK_CCT_V2_OP_AWB_GET_AUTO_RUN_INFO", mrCCAPAWBGetEnableInfo_Cmd},
    {"gAWBGain",          "ACDK_CCT_V2_OP_AWB_GET_GAIN",          mrCCAPAWBGetGain_Cmd},
    {"sAWBGain",          "ACDK_CCT_V2_OP_AWB_SET_GAIN (sAWBGain <Rgain> <Ggain> <Bgain>)", mrCCAPAWBSetGain_Cmd},
    {"aAWBParam",         "ACDK_CCT_V2_OP_AWB_APPLY_CAMERA_PARA2", mrCCAPAWBApplyCameraPara_Cmd},
    {"gAWBParam",         "ACDK_CCT_V2_OP_AWB_GET_AWB_PARA",       mrCCAPAWBGetAWBPara_Cmd},
    {"sAWBParam",         "ACDK_CCT_V2_OP_AWB_SAVE_AWB_PARA",      mrCCAPAWBSaveAWBPara_Cmd},
    {"sAWBMode",          "ACDK_CCT_OP_AWB_SET_AWB_MODE (sAWBMode <AWBMode>)", mrCCAPAWBSetAWBMode_Cmd},
    {"gAWBMode",          "ACDK_CCT_OP_AWB_GET_AWB_MODE",          mrCCAPAWBGetAWBMode_Cmd},
    {"gAWBLightProb",     "ACDK_CCT_OP_AWB_GET_LIGHT_PROB",        mrCCAPAWBGetLightProb_Cmd},

    //AF
    {"301",               "ACDK_CCT_V2_OP_AF_OPERATION",           mrCCAPAFOperation_Cmd},
    {"302",               "ACDK_CCT_V2_OP_MF_OPERATION",           mrCCAPMFOperation_Cmd},
    {"303",               "ACDK_CCT_V2_OP_GET_AF_INFO",            mrCCAPGetAFInfo_Cmd},
    {"304",               "ACDK_CCT_V2_OP_AF_GET_BEST_POS",        mrCCAPAFGetBestPos_Cmd},
    {"305",               "ACDK_CCT_V2_OP_AF_CALI_OPERATION",      mrCCAPAFCaliOperation_Cmd},
    {"306",               "ACDK_CCT_V2_OP_AF_SET_RANGE",           mrCCAPAFSetRange_Cmd},
    {"307",               "ACDK_CCT_V2_OP_AF_GET_RANGE",           mrCCAPAFGetRange_Cmd},
    {"308",               "ACDK_CCT_V2_OP_AF_SAVE_TO_NVRAM",       mrCCAPAFSaveToNVRAM_Cmd},
    {"309",               "ACDK_CCT_V2_OP_AF_READ",                mrCCAPAFRead_Cmd},
    {"310",               "ACDK_CCT_V2_OP_AF_APPLY",               mrCCAPAFApply_Cmd},
    {"311",               "ACDK_CCT_V2_OP_AF_GET_FV",              mrCCAPAFGetFV_Cmd},

    //FLASH
    {"enFlash",           "ACDK_CCT_OP_FLASH_ENABLE",              mrCCAPFLASHEnable_Cmd},
    {"disFlash",          "ACDK_CCT_OP_FLASH_DISABLE",             mrCCAPFLASHDisable_Cmd},
    {"gFlashEnableInfo",  "ACDK_CCT_OP_FLASH_GET_INFO",            mrCCAPFLASHGetInfo_Cmd},

    //CCM
    {"enDynamicCCM",        "ACDK_CCT_V2_OP_AWB_ENABLE_DYNAMIC_CCM",    mrCCAPEnableDyanmicCCM_Cmd},
    {"disDynamicCCM",       "ACDK_CCT_V2_OP_AWB_DISABLE_DYNAMIC_CCM",   mrCCAPDisableDynamicCCM_Cmd},
    {"getCCM",              "ACDK_CCT_V2_OP_AWB_GET_CCM_PARA",          mrCCAPGetCCMPara_Cmd},
    {"getCCMStatus",        "ACDK_CCT_V2_OP_AWB_GET_CCM_STATUS",        mrCCAPGetCCMStatus_Cmd},
    {"getCurrentCCM",       "ACDK_CCT_V2_OP_AWB_GET_CURRENT_CCM",       mrCCAPGetCurrentCCM_Cmd},
    {"506",                 "ACDK_CCT_V2_OP_AWB_GET_NVRAM_CCM",         mrCCAPGetNVRAMCCM_Cmd},
    {"setCurrentCCM",       "ACDK_CCT_V2_OP_AWB_SET_CURRENT_CCM",       mrCCAPSetCurrentCCM_Cmd},
    {"508",                 "ACDK_CCT_V2_OP_AWB_SET_NVRAM_CCM",         mrCCAPSetNVRAMCCM_Cmd},
    {"updateCCM",           "ACDK_CCT_V2_OP_AWB_UPDATE_CCM_PARA",       mrCCAPUpdateCCMPara_Cmd},
    {"510",                 "ACDK_CCT_V2_OP_AWB_UPDATE_CCM_STATUS",     mrCCAPUpdateCCMStatus_Cmd},
    {"setCCMMode",          "ACDK_CCT_OP_SET_CCM_MODE",                 mrCCAPSetCCMMode_Cmd},
    {"getCCMMode",          "ACDK_CCT_OP_GET_CCM_MODE",                 mrCCAPGetCCMMode_Cmd},

    //Gamma
    {"601",                 "FT_MSDK_CCT_V2_OP_AE_SET_GAMMA_BYPASS",             mrCCAPSetGammaBypass_Cmd},
    {"602",                 "FT_MSDK_CCT_V2_OP_AE_GET_GAMMA_BYPASS_FLAG",    mrCCAPGetGammaBypassFlag_Cmd},
    {"getGamma",            "ACDK_CCT_V2_OP_AE_GET_GAMMA_TABLE",          mrCCAPGetGammaTable_Cmd},
    {"setGamma",            "ACDK_CCT_V2_OP_AE_SET_GAMMA_TABLE",          mrCCAPSetGammaTable_Cmd},

    //PCA
    {"setPcaTable",         "ACDK_CCT_OP_ISP_SET_PCA_TABLE",            mrCCAPSetPcaTable_Cmd},
    {"getPcaTable",         "ACDK_CCT_OP_ISP_GET_PCA_TABLE",            mrCCAPGetPcaTable_Cmd},
    {"setPcaPara",          "ACDK_CCT_OP_ISP_SET_PCA_PARA",             mrCCAPSetPcaPara_Cmd},
    {"getPcaPara",          "ACDK_CCT_OP_ISP_GET_PCA_PARA",             mrCCAPGetPcaPara_Cmd},
    {"setPcaSlider",        "ACDK_CCT_OP_ISP_SET_PCA_Slider",           mrCCAPSetPcaSlider_Cmd},
    {"getPcaSlider",        "ACDK_CCT_OP_ISP_GET_PCA_Slider",           mrCCAPGetPcaSlider_Cmd},

    //Module Control
    {"setIspOn",            "ACDK_CCT_OP_SET_ISP_ON",                   mrCCAPSetIspOn_Cmd},
    {"setIspOff",           "ACDK_CCT_OP_SET_ISP_OFF",                  mrCCAPSetIspOff_Cmd},
    {"getIspOnOff",         "ACDK_CCT_OP_GET_ISP_ON_OFF",               mrCCAPGetIspOnOff_Cmd},

    //Shading Table
    {"getLSCTbl",         "ACDK_CCT_V2_OP_ISP_GET_SHADING_TABLE_V3",       mrCCAPGetShadingTableV3_Cmd},
    {"setLSCTbl",         "ACDK_CCT_V2_OP_ISP_SET_SHADING_TABLE_V3",        mrCCAPSetShadingTableV3_Cmd},
    {"getLSCTblp",         "ACDK_CCT_V2_OP_ISP_GET_SHADING_TABLE_POLYCOEF",       mrCCAPGetShadingTablePOLYCOEF_Cmd},
    {"setLSCTblp",         "ACDK_CCT_V2_OP_ISP_SET_SHADING_TABLE_POLYCOEF",        mrCCAPSetShadingTablePOLYCOEF_Cmd},
    {"getLSCNvram",         "ACDK_CCT_V2_OP_ISP_GET_SHADING_NVRAM",       mrCCAPGetShadingNVRAM_Cmd},
    {"setLSCIndex",     "ACDK_CCT_V2_OP_ISP_SET_SHADING_INDEX",             mrCCAPSetShadingIndex_Cmd},
    {"getLSCIndex",     "ACDK_CCT_V2_OP_ISP_GET_SHADING_INDEX",             mrCCAPGetShadingIndex_Cmd},
    // CDVT
    {"SensorTest", "ACDK_CCT_OP_CDVT_SENSOR_TEST", mrCCAPCDVTSensorTest_Cmd},
    {"SensorCal", "ACDK_CCT_OP_CDVT_SENSOR_CALIBRATION", mrCCAPCDVTSensorCalibration_Cmd},
    {"flash_cal", "ACDK_CCT_OP_FLASH_CALIBRATION", mrCCAPCDVTFlashCalibration_Cmd},
    {"AETableLinearity", "ACDK_CCT_OP_AE_PLINE_TABLE_TEST", mrAEPlineTableLinearity_cmd},
    //flash cmd
	{"flash_read_nvram", "ACDK_CCT_OP_STROBE_READ_NVRAM", mrCCAP_ReadNvram}, //5
	{"flash_write_nvram", "ACDK_CCT_OP_STROBE_WRITE_NVRAM", mrCCAP_WriteNvram}, //6
	{"flash_read_default_nvram", "ACDK_CCT_OP_STROBE_READ_DEFAULT_NVRAM", mrCCAP_ReadDefaultNvram}, //7
	{"flash_set_param", "ACDK_CCT_OP_STROBE_SET_PARAM", mrCCAP_SetParam}, //8
	{"flash_get_param", "ACDK_CCT_OP_STROBE_GET_PARAM", mrCCAP_GetParam}, //9
	{"flash_get_nvdata", "ACDK_CCT_OP_STROBE_GET_NVDATA", mrCCAP_GetNvdata}, //10
	{"flash_set_nvdata", "ACDK_CCT_OP_STROBE_SET_NVDATA", mrCCAP_SetNvdata}, //11
	{"flash_get_eng_y", "ACDK_CCT_OP_STROBE_GET_ENG_Y", mrCCAP_GetEngY}, //12
	{"flash_set_eng_y", "ACDK_CCT_OP_STROBE_SET_ENG_Y", mrCCAP_SetEngY}, //13
	{"flash_get_eng_rg", "ACDK_CCT_OP_STROBE_GET_ENG_RG", mrCCAP_GetEngRg}, //14
	{"flash_set_eng_rg", "ACDK_CCT_OP_STROBE_SET_ENG_RG", mrCCAP_SetEngRg}, //15
	{"flash_get_eng_bg", "ACDK_CCT_OP_STROBE_GET_ENG_BG", mrCCAP_GetEngBg}, //16
	{"flash_set_eng_bg", "ACDK_CCT_OP_STROBE_SET_ENG_BG", mrCCAP_SetEngBg}, //17
	{"flash_nvdata_to_file", "ACDK_CCT_OP_STROBE_NVDATA_TO_FILE", mrCCAP_NvdataToFile}, //18
	{"flash_file_to_nvdata", "ACDK_CCT_OP_STROBE_FILE_TO_NVDATA", mrCCAP_FileToNvdata}, //19



    NULL_CLI_CMD,
};

/////////////////////////////////////////////////////////////////////////
//
//  vHelp () -
//! @brief skip the space of the input string
//! @param ppInStr: The point of the input string
/////////////////////////////////////////////////////////////////////////
VOID vHelp()
{
    if (g_prAcdkCCAPCLICmds == NULL)
    {
        ACDK_LOGE("Null Acdk Support Cmds \n");
        return;
    }

    printf("\n***********************************************************\n");
    printf("* ACDK CLI Test                                                  *\n");
    printf("* Current Support Commands                                *\n");
    printf("===========================================================\n");
    MUINT32 i = 0;
    while(g_prAcdkCCAPCLICmds[ i].pucCmdStr != NULL)
    {
        printf("%s    [%s]\n", g_prAcdkCCAPCLICmds[ i].pucCmdStr,
                                g_prAcdkCCAPCLICmds[ i].pucHelpStr);
        i++;
    }
    printf("help/h    [Help]\n");
    printf("exit/q    [Exit]\n");
}

/////////////////////////////////////////////////////////////////////////
//
//  cliKeyThread () -
//! @brief the CLI key input thread, wait for CLI command
//! @param a_pArg: The input arguments
/////////////////////////////////////////////////////////////////////////
VOID* cliKeyThread (VOID *a_pArg)
{
    char urCmds[256] = {0};

    //! ************************************************
    //! Set the signal for kill self thread
    //! this is because android don't support thread_kill()
    //! So we need to creat a self signal to receive signal
    //! to kill self
    //! ************************************************
    struct sigaction actions;
    memset(&actions, 0, sizeof(actions));
    sigemptyset(&actions.sa_mask);
    actions.sa_flags = 0;
    actions.sa_handler = thread_exit_handler;
    int rc = sigaction(SIGUSR1,&actions,NULL);

    while (1)
    {

        printf("Input Cmd#");
        fgets(urCmds, 256, stdin);

        //remove the '\n'
        urCmds[strlen(urCmds)-1] = '\0';
        char *pCmds = &urCmds[0];
        //remove the space in the front of the string
        vSkipSpace(&pCmds);

        //Ignore blank command
        if (*pCmds == '\0')
        {
            continue;
        }

        //Extract the Command  and arguments where the argV[0] is the command
        MUINT32 u4ArgCount = 0;
        MUINT8  *pucStrToken, *pucCmdToken;
        MUINT8  *pucArgValues[MAX_CLI_CMD_ARGS];

        pucStrToken = (MUINT8*)strtok(pCmds, " ");

        while (pucStrToken != NULL)
        {
            pucArgValues[u4ArgCount++] = pucStrToken;
            pucStrToken = (MUINT8*)strtok (NULL, " ");
        }

        if (u4ArgCount == 0)
        {
            continue;
        }

        pucCmdToken = pucArgValues[0];

        //parse the command
        if ((strcmp((char*)pucCmdToken, "help") == 0) ||
            (strcmp((char *)pucCmdToken, "h") == 0))
        {
            vHelp();
        }
        else if ((strcmp((char*)pucCmdToken, "exit") == 0) ||
                  (strcmp((char *)pucCmdToken, "q") == 0))
        {
            ACDK_LOGD("Exit From CLI\n");
            g_bIsCLITest = MFALSE;
        }
        else
        {
            MINT32 i4CmdIndex = 0;
            BOOL bIsFoundCmd = MFALSE;
            while (g_prAcdkCCAPCLICmds[i4CmdIndex].pucCmdStr != NULL)
            {
                if (strcmp((char*)pucCmdToken, g_prAcdkCCAPCLICmds[i4CmdIndex].pucCmdStr) == 0)
                {
                    bIsFoundCmd = MTRUE;
                    g_prAcdkCCAPCLICmds[i4CmdIndex].handleCmd(u4ArgCount - 1, &pucArgValues[1]);
                    break;
                }
                i4CmdIndex++;
            }
            if (bIsFoundCmd == MFALSE)
            {
                printf("Invalid Command\n");
            }
        }

    }

    return 0;
}


/////////////////////////////////////////////////////////////////////////
//
//  main () -
//! @brief The main function for sensor test, it will create two thread
//!        one is for CLI command
//!        the other is for keypad input command
//! @param argc: The number of the input argument
//! @param argv: The input arguments
/////////////////////////////////////////////////////////////////////////
int main (int argc, char **argv)
{
    MUINT32 u4RealOutLen = 0;

    ACDK_LOGD(" Acdk CCAP CLI Test\n");

#if 0
    ACDK_LOGD(" Insert the PMEM device \n");

    //! *************************************************
    //! insert the PMEM device,
    //! *************************************************
    if (open("/dev/pmem_multimedia", O_RDWR) < 0)
    {
        vExecProgram("mknod", g_mkPMEMNod_arg_list);
    }


    //!***************************************************
    //! mount the SD Card to SDCard
    //!***************************************************
    ACDK_LOGD(" No Mount SD Card to /sdcard folder \n");
    vExecProgram("mount", g_mountSD_arg_list);

#endif
    sleep(1);

    //! *************************************************
    //! Create the related object and init/enable it
    //! *************************************************
    if (Mdk_Open() == MFALSE)
    {
        ACDK_LOGE("Mdk_Open() Fail \n");
        goto Exit;
    }


    if (Mdk_Init() == MFALSE)
    {
        ACDK_LOGE("Mdk_Init() Fail \n");
        goto Exit;
    }

    if (CctIF_Open() == MFALSE)
    {
        ACDK_LOGE("CctIF_Open() Fail \n");
        goto Exit;
    }


    if (CctIF_Init() == MFALSE)
    {
        ACDK_LOGE("CctIF_Init() Fail \n");
        goto Exit;
    }


    g_bAcdkOpend = MTRUE;

    //! *************************************************
    //! Create the CLI command thread to listen input CLI command
    //! *************************************************
    ACDK_LOGD(" Create the CLI thread \n");

    vHelp();
    pthread_create(& g_CliKeyThreadHandle, NULL, cliKeyThread, NULL);

    //!***************************************************
    //! Main thread wait for exit
    //!***************************************************
    while (g_bIsCLITest== MTRUE)
    {
        sleep(1);
    }

    //!***************************************************
    //! Receive the exit command, cancel the two thread
    //!***************************************************
    int status;
    ACDK_LOGD("Cancel cli key thread\n");
    if ((status = pthread_kill(g_CliKeyThreadHandle, SIGUSR1)) != 0)
    {
         ACDK_LOGE("Error cancelling thread %d, error = %d (%s)\n", (int)g_CliKeyThreadHandle,
               status, (char*)strerror);
    }


Exit:

    //!***************************************************
    //! Exit
    //! 1. DeInit ACDK device and close it
    //! 2. Umount the SD card and sync the file to ensure
    //!    all files are written to SD card
    //! 3. Sync all file to SD card to ensure the files are saving in SD
    //!***************************************************

	CctIF_DeInit();
    CctIF_Close();

    Mdk_DeInit();
    Mdk_Close();
    ACDK_LOGD("umount SDCard file system\n");

#if 0
    vExecProgram("sync", g_sync_arg_list);     //sync all file tor sdcard first
    vExecProgram("umount", g_unMountSD_arg_list);
#endif

    return 0;
}


