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
 
//! \file ftm_camera.cpp

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/mount.h>
#include <sys/statfs.h>

extern "C" {
#include "common.h"
#include "miniui.h"
#include "ftm.h"
}
#include <mtkcam/acdk/MdkIF.h>
#include <cutils/properties.h>

//#define LOG_TAG "[FTM_CAMERA] "

#define DEBUG_FTM_CAMERA

//#define SMT_USED  // SMT only support Preview 
#ifdef DEBUG_FTM_CAMERA
#define FTM_CAMERA_DBG(fmt, arg...)  LOGD(fmt, ##arg)
#define FTM_CAMERA_ERR(fmt, arg...)  LOGE("Err: %5d:, "fmt, __LINE__, ##arg)
#else
#define FTM_CAMERA_DBG(a,...)
#define FTM_CAMERA_ERR(a,...)
#endif

/*******************************************************************************
*
********************************************************************************/
enum
{
    ITEM_PASS,
    ITEM_FAIL,
    ITEM_RETURN,
    ITEM_CAMERA_TEST,
//    ITEM_CAPTURE,
//    ITEM_STROBE,
};
/*******************************************************************************
*
********************************************************************************/
static item_t camera_items[] = {
    {ITEM_CAMERA_TEST, uistr_camera_preview, 0, 0},
    {ITEM_PASS,        uistr_pass,           0, 0},
    {ITEM_FAIL,        uistr_fail,           0, 0},
    {-1,               NULL,                 0, 0},
};

/*******************************************************************************
*
********************************************************************************/
struct camera
{
    char  info[1024];
    bool  exit_thd;

    text_t    title;
    text_t    text;
    text_t    left_btn;
    text_t    center_btn;
    text_t    right_btn;

    //text_t    cap_left_btn;
    //text_t    cap_center_btn;
    //text_t    cap_right_btn;

    //pthread_t update_button_thd;
    //pthread_t update_cap_thd;
    struct ftm_module *mod;
    struct textview tv;
    struct itemview *iv;
//    struct imageview imv;
//    struct paintview pv;
};

typedef struct
{
	MUINT16 *C1;
	MUINT16 *C2;
	MUINT16 *C3;
	MUINT16 *C4;
	MINT32 channel_num;
} AnaImage;

/*******************************************************************************
*
********************************************************************************/
#define mod_to_camera(p)     (struct camera*)((char*)(p) + sizeof(struct ftm_module))

#define FREEIF(p)   do { if(p) free(p); (p) = NULL; } while(0)

pthread_t camera_flash_thread_handle = 0;
//FLASHLIGHT_TYPE_ENUM eFlashSupport = FLASHLIGHT_NONE;
MUINT32 srcDev = 0;
static int g_previewStop = MFALSE;

static MBOOL gCapCBDone = MFALSE;
static MBOOL gQvDone = MFALSE;
//auto test      
static int gcamera_autoTest = 1; //  0: Manual mode, 1: auto test pattern method 

#define max(x1,x2) ((x1) > (x2) ? (x1):(x2))

#define min(x1,x2) ((x1) > (x2) ? (x2):(x1))

static MBOOL gPinError_pass = MFALSE;
static MBOOL autoTest_2nd = MFALSE;
static MUINT32 gImgFmt = 0;
static MUINT32 gImgW = 0;
static MUINT32 gImgH = 0;
static MUINT32 gBitDepth = 0;
static MUINT32 gColorOdr = 0;
static MUINT32 gRecShutter = 0;
static MINT32 gPinErrType = 0;
static MUINT32 gCRCValue = 0;

/*******************************************************************************
*  Calculates the CRC-8 of the first len bits in data
********************************************************************************/
static const MUINT32 ACDK_CRC_Table[256]=
{
    0x0,        0x4C11DB7,  0x9823B6E,  0xD4326D9,  0x130476DC, 0x17C56B6B, 0x1A864DB2, 0x1E475005,
    0x2608EDB8, 0x22C9F00F, 0x2F8AD6D6, 0x2B4BCB61, 0x350C9B64, 0x31CD86D3, 0x3C8EA00A, 0x384FBDBD,
    0x4C11DB70, 0x48D0C6C7, 0x4593E01E, 0x4152FDA9, 0x5F15ADAC, 0x5BD4B01B, 0x569796C2, 0x52568B75,
    0x6A1936C8, 0x6ED82B7F, 0x639B0DA6, 0x675A1011, 0x791D4014, 0x7DDC5DA3, 0x709F7B7A, 0x745E66CD,
    0x9823B6E0, 0x9CE2AB57, 0x91A18D8E, 0x95609039, 0x8B27C03C, 0x8FE6DD8B, 0x82A5FB52, 0x8664E6E5,
    0xBE2B5B58, 0xBAEA46EF, 0xB7A96036, 0xB3687D81, 0xAD2F2D84, 0xA9EE3033, 0xA4AD16EA, 0xA06C0B5D,
    0xD4326D90, 0xD0F37027, 0xDDB056FE, 0xD9714B49, 0xC7361B4C, 0xC3F706FB, 0xCEB42022, 0xCA753D95,
    0xF23A8028, 0xF6FB9D9F, 0xFBB8BB46, 0xFF79A6F1, 0xE13EF6F4, 0xE5FFEB43, 0xE8BCCD9A, 0xEC7DD02D,
    0x34867077, 0x30476DC0, 0x3D044B19, 0x39C556AE, 0x278206AB, 0x23431B1C, 0x2E003DC5, 0x2AC12072,
    0x128E9DCF, 0x164F8078, 0x1B0CA6A1, 0x1FCDBB16, 0x18AEB13,  0x54BF6A4,  0x808D07D,  0xCC9CDCA,
    0x7897AB07, 0x7C56B6B0, 0x71159069, 0x75D48DDE, 0x6B93DDDB, 0x6F52C06C, 0x6211E6B5, 0x66D0FB02,
    0x5E9F46BF, 0x5A5E5B08, 0x571D7DD1, 0x53DC6066, 0x4D9B3063, 0x495A2DD4, 0x44190B0D, 0x40D816BA,
    0xACA5C697, 0xA864DB20, 0xA527FDF9, 0xA1E6E04E, 0xBFA1B04B, 0xBB60ADFC, 0xB6238B25, 0xB2E29692,
    0x8AAD2B2F, 0x8E6C3698, 0x832F1041, 0x87EE0DF6, 0x99A95DF3, 0x9D684044, 0x902B669D, 0x94EA7B2A,
    0xE0B41DE7, 0xE4750050, 0xE9362689, 0xEDF73B3E, 0xF3B06B3B, 0xF771768C, 0xFA325055, 0xFEF34DE2,
    0xC6BCF05F, 0xC27DEDE8, 0xCF3ECB31, 0xCBFFD686, 0xD5B88683, 0xD1799B34, 0xDC3ABDED, 0xD8FBA05A,
    0x690CE0EE, 0x6DCDFD59, 0x608EDB80, 0x644FC637, 0x7A089632, 0x7EC98B85, 0x738AAD5C, 0x774BB0EB,
    0x4F040D56, 0x4BC510E1, 0x46863638, 0x42472B8F, 0x5C007B8A, 0x58C1663D, 0x558240E4, 0x51435D53,
    0x251D3B9E, 0x21DC2629, 0x2C9F00F0, 0x285E1D47, 0x36194D42, 0x32D850F5, 0x3F9B762C, 0x3B5A6B9B,
    0x315D626,  0x7D4CB91,  0xA97ED48,  0xE56F0FF,  0x1011A0FA, 0x14D0BD4D, 0x19939B94, 0x1D528623,
    0xF12F560E, 0xF5EE4BB9, 0xF8AD6D60, 0xFC6C70D7, 0xE22B20D2, 0xE6EA3D65, 0xEBA91BBC, 0xEF68060B,
    0xD727BBB6, 0xD3E6A601, 0xDEA580D8, 0xDA649D6F, 0xC423CD6A, 0xC0E2D0DD, 0xCDA1F604, 0xC960EBB3,
    0xBD3E8D7E, 0xB9FF90C9, 0xB4BCB610, 0xB07DABA7, 0xAE3AFBA2, 0xAAFBE615, 0xA7B8C0CC, 0xA379DD7B,
    0x9B3660C6, 0x9FF77D71, 0x92B45BA8, 0x9675461F, 0x8832161A, 0x8CF30BAD, 0x81B02D74, 0x857130C3,
    0x5D8A9099, 0x594B8D2E, 0x5408ABF7, 0x50C9B640, 0x4E8EE645, 0x4A4FFBF2, 0x470CDD2B, 0x43CDC09C,
    0x7B827D21, 0x7F436096, 0x7200464F, 0x76C15BF8, 0x68860BFD, 0x6C47164A, 0x61043093, 0x65C52D24,
    0x119B4BE9, 0x155A565E, 0x18197087, 0x1CD86D30, 0x29F3D35,  0x65E2082,  0xB1D065B,  0xFDC1BEC,
    0x3793A651, 0x3352BBE6, 0x3E119D3F, 0x3AD08088, 0x2497D08D, 0x2056CD3A, 0x2D15EBE3, 0x29D4F654,
    0xC5A92679, 0xC1683BCE, 0xCC2B1D17, 0xC8EA00A0, 0xD6AD50A5, 0xD26C4D12, 0xDF2F6BCB, 0xDBEE767C,
    0xE3A1CBC1, 0xE760D676, 0xEA23F0AF, 0xEEE2ED18, 0xF0A5BD1D, 0xF464A0AA, 0xF9278673, 0xFDE69BC4,
    0x89B8FD09, 0x8D79E0BE, 0x803AC667, 0x84FBDBD0, 0x9ABC8BD5, 0x9E7D9662, 0x933EB0BB, 0x97FFAD0C,
    0xAFB010B1, 0xAB710D06, 0xA6322BDF, 0xA2F33668, 0xBCB4666D, 0xB8757BDA, 0xB5365D03, 0xB1F740B4
};  // Table of 8-bit remainders

/*******************************************************************************
*
********************************************************************************/
static bool bSendDataToACDK(eACDK_COMMAND	FeatureID,
                                     MUINT8*  pInAddr,
                                     MUINT32  nInBufferSize,
                                     MUINT8*  pOutAddr,
                                     MUINT32  nOutBufferSize,
                                     MUINT32* pRealOutByeCnt)
{
    ACDK_FEATURE_INFO_STRUCT rAcdkFeatureInfo;

    rAcdkFeatureInfo.puParaIn     = pInAddr;
    rAcdkFeatureInfo.u4ParaInLen  = nInBufferSize;
    rAcdkFeatureInfo.puParaOut    = pOutAddr;
    rAcdkFeatureInfo.u4ParaOutLen = nOutBufferSize;
    rAcdkFeatureInfo.pu4RealParaOutLen = pRealOutByeCnt;

    return (Mdk_IOControl(FeatureID, &rAcdkFeatureInfo));
}
/*******************************************************************************
*!brief for af information 
********************************************************************************/
static int camera_AF_Info_test()
{
    FTM_CAMERA_DBG("%s ,Enter\n", __FUNCTION__);

    MUINT32 u4RetLen = 0, AFStatus = 0;
    MBOOL bRet;

    bRet = bSendDataToACDK(ACDK_CMD_GET_AF_INFO,
                             NULL,
                             0,
                             (MUINT8 *)&AFStatus,
                             sizeof(MUINT32),
                             &u4RetLen);
    if(AFStatus == 1)
    {
        FTM_CAMERA_DBG("Auto focus Succeed!\n");
    }
    else
    {
        FTM_CAMERA_DBG("Auto focus Fail!\n");
    }

    if(!bRet)
    {
        FTM_CAMERA_ERR("camera_AF_Info_test Fail\n");
        return 1;
    }

    FTM_CAMERA_DBG("%s ,Exit\n", __FUNCTION__);

    return 0;
}

/*******************************************************************************
*!brief for camera preview test
********************************************************************************/
enum
{
    CAMERA_STATE_NONE,
    CAMERA_STATE_IDLE,
    CAMERA_STATE_PREVIEW,
};

#define MEDIA_PATH "/data/"
static unsigned int g_u4ImgCnt = 0;
static bool bCapDone = MFALSE;
static int camera_state = CAMERA_STATE_NONE;
static char szFileName[256];

/////////////////////////////////////////////////////////////////////////
//
//   camera_preview_test () -
//!
//!  brief for camera preview test
//
/////////////////////////////////////////////////////////////////////////
static MINT32 camera_preview_test()
{
    FTM_CAMERA_DBG("%s ,Enter\n", __FUNCTION__);

    ACDK_PREVIEW_STRUCT rACDKPrvConfig;
    rACDKPrvConfig.fpPrvCB = NULL;
    rACDKPrvConfig.u4PrvW = 320;
    rACDKPrvConfig.u4PrvH = 240;

    if(gcamera_autoTest == 1)// Auto test
        rACDKPrvConfig.u16PreviewTestPatEn = 1;
    else// manual test
        rACDKPrvConfig.u16PreviewTestPatEn = 0;
    
    MUINT32 u4RetLen = 0;
    rACDKPrvConfig.eOperaMode    = ACDK_OPT_FACTORY_MODE;

    bool bRet = bSendDataToACDK(ACDK_CMD_PREVIEW_START,
                                (MUINT8 *)&rACDKPrvConfig,
                                sizeof(ACDK_PREVIEW_STRUCT),
                                NULL,
                                0,
                                &u4RetLen);

    if (!bRet)
    {
        FTM_CAMERA_ERR("ACDK_PREVIEW_STRUCT Fail\n");
        return 1;
    }

    camera_state = CAMERA_STATE_PREVIEW;

    FTM_CAMERA_DBG("%s ,Exit\n", __FUNCTION__);

    return 0;
}

/////////////////////////////////////////////////////////////////////////
//
//   camera_flash_test_thread () -
//!
//!  brief for camera flash test thread
//
/////////////////////////////////////////////////////////////////////////

static void *camera_flash_test_thread(void *priv)
{
	/*
    unsigned int u4RetLen = 0;
    ACDK_FLASH_CONTROL flashCtrl;

    memset (&flashCtrl, 0, sizeof(ACDK_FLASH_CONTROL));
    flashCtrl.level = 32;       //max is 32, min is 0
    flashCtrl.duration = 50000;



    FTM_CAMERA_DBG("ACDK_CCT_OP_FLASH_CONTROL ON \n");
    
    struct camera *cam = (struct camera *)priv;

		StrobeDrv*  pStrobe;
		pStrobe = StrobeDrv::createInstance();
		pStrobe->init(1);

		//pStrobe->setDuty(0);

    while (!cam->exit_thd) {
        //bSendDataToACDK(ACDK_CCT_OP_FLASH_CONTROL, (UINT8*)&flashCtrl, (UINT32)sizeof(int),NULL, 0, &u4RetLen);
        FTM_CAMERA_DBG("[camera_flash_test_thread] \n");

        pStrobe->setOnOff(1);
        usleep(100000);//300ms
        pStrobe->setOnOff(0);
        usleep(800000);
    }
    pStrobe->uninit();
    */

    return NULL;
}

/////////////////////////////////////////////////////////////////////////
//
//   mrSaveJPEGImg () -
//!
//!  brief for geneirc function to save image file
//
/////////////////////////////////////////////////////////////////////////
bool bSaveJPEGImg(char *a_pBuf,  unsigned int a_u4Size)
{
    sprintf(szFileName, "%s//%04d.jpg" , MEDIA_PATH, g_u4ImgCnt);

    FILE *pFp = fopen(szFileName, "wb");

    if (NULL == pFp )
    {
        FTM_CAMERA_ERR("Can't open file to save Image\n");
        return MFALSE;
    }


    int i4WriteCnt = fwrite(a_pBuf, 1, a_u4Size , pFp);

    FTM_CAMERA_DBG("Save image file name:%s\n", szFileName);

    fclose(pFp);
    sync();
    return MTRUE;
}

/*******************************************************************************
*!  brief parse RAW image for auto-testing algorithm
********************************************************************************/
MINT32 loadRawImage(const MUINT32 height, 
                         const MUINT32 width, 
                         const MUINT32 format, 
                         AnaImage* I) 
{
    FTM_CAMERA_DBG("[loadRawImage] +\n");
    
    MUINT16 *R, *GR, *B, *GB, *rawTmp;
    MUINT32 rows = height >> 1;
    MUINT32 cols = width >> 1;
    MUINT32 size = rows * cols;

    char szFileName[256];
    sprintf(szFileName, "%s/ftm_pureRAW_10.raw" , MEDIA_PATH);
    
    FILE *pFp = fopen(szFileName, "rb");

    if(!pFp) 
    {
        FTM_CAMERA_ERR("Could not load image file: %s\n",szFileName);
        return -99;
    }

    rawTmp = (MUINT16 *)malloc(sizeof(MUINT16) * size << 2);   
    R  = (MUINT16 *)malloc(sizeof(MUINT16) * size);
    GR = (MUINT16 *)malloc(sizeof(MUINT16) * size);
    GB = (MUINT16 *)malloc(sizeof(MUINT16) * size);
    B  = (MUINT16 *)malloc(sizeof(MUINT16) * size);

    fread(rawTmp, sizeof(MUINT16), size << 2, pFp);
    
    for(MUINT32 i = 0 ; i < height ; i += 2) 
    {
        for(MUINT32 j = 0 ; j < width ; j += 2)
        {
            MUINT32 des = (i / 2) * cols + (j / 2);

            switch(format)
            {
            case 0 :    // BGGR
                     R[des]  = rawTmp[(i + 1) * width + j + 1];
                     GR[des] = rawTmp[(i + 1) * width + j];
                     B[des]  = rawTmp[i * width + j];
                     GB[des] = rawTmp[i * width + j + 1];
                break;
            case 1 :    // GBRG
                     R[des]  = rawTmp[(i + 1) * width + j];
                     GR[des] = rawTmp[(i + 1) * width + j + 1];
                     B[des]  = rawTmp[i * width + j + 1];
                     GB[des] = rawTmp[i * width + j];
                break;
            case 2 :    // GRBG
                     R[des]  = rawTmp[i * width + j + 1];
                     GR[des] = rawTmp[i * width + j];
                     B[des]  = rawTmp[(i + 1) * width + j];
                     GB[des] = rawTmp[(i + 1) * width + j + 1];
                break;
            case 3 :    // RGGB
                     R[des]  = rawTmp[i * width + j];
                     GR[des] = rawTmp[i * width + j + 1];
                     B[des]  = rawTmp[(i + 1) * width + j + 1];
                     GB[des] = rawTmp[(i + 1) * width + j];
                break;
            default :   // error
                FTM_CAMERA_ERR("wrong RAW format\n");
                fclose(pFp);
                return -1;
            }
        }
    }

    I->C1 = R;
    I->C2 = GR;
    I->C3 = B;
    I->C4 = GB;
    I->channel_num = 3;

    fclose(pFp);

    FTM_CAMERA_DBG("[loadRawImage] -\n");
    return 0;
}

/*******************************************************************************
*!  brief parse YUV image for auto-testing algorithm
********************************************************************************/
MINT32 loadYuvImage(const MUINT32 height,
                         const MUINT32 width,
                         const MUINT32 format,
                         AnaImage* I)
{
    FTM_CAMERA_DBG("[loadYuvImage] +\n");
    
    MUINT8  *yuvTmp;
    MUINT16 *YU, *YV, *U, *V;
    MUINT32 rows = height;
    MUINT32 cols = width >> 1;
    MUINT32 size = rows * cols << 2;
    char szFileName[256];

    switch(format)
    {
        case YUVFmt_UYVY : sprintf(szFileName, "%s/YUVImg_UYVY.yuv" , MEDIA_PATH);
            break;
        case YUVFmt_VYUY : sprintf(szFileName, "%s/YUVImg_VYUY.yuv" , MEDIA_PATH);
            break;
        case YUVFmt_YUY2 : sprintf(szFileName, "%s/YUVImg_YUY2.yuv" , MEDIA_PATH);
            break;
        case YUVFmt_YVYU : sprintf(szFileName, "%s/YUVImg_YVYU.yuv" , MEDIA_PATH);
            break;
    }
    
    FILE *pFp = fopen(szFileName, "rb");
    if(!pFp)
    {
        FTM_CAMERA_ERR("Could not load image file: %s\n",szFileName);
        return -99;
    }

    yuvTmp = (MUINT8 *)malloc(sizeof(MUINT8) * size << 2);
    YU = (MUINT16 *)malloc(sizeof(MUINT16) * size);
    YV = (MUINT16 *)malloc(sizeof(MUINT16) * size);
    U  = (MUINT16 *)malloc(sizeof(MUINT16) * size);
    V  = (MUINT16 *)malloc(sizeof(MUINT16) * size);

    fread(yuvTmp , sizeof(MUINT8), size << 2 , pFp);

    for(MUINT32 i = 0; i < size; i += 4)
    {
        MUINT32 des = i >> 2;

        switch(format)
        {
        case YUVFmt_UYVY :
                YU[des] = yuvTmp[i + 1];
                YV[des] = yuvTmp[i + 3];
                U[des]  = yuvTmp[i];
                V[des]  = yuvTmp[i + 2];
            break;
        case YUVFmt_VYUY :
                YU[des] = yuvTmp[i + 3];
                YV[des] = yuvTmp[i + 1];
                U[des]  = yuvTmp[i + 2];
                V[des]  = yuvTmp[i];
            break;
        case YUVFmt_YUY2 :
                YU[des] = yuvTmp[i];
                YV[des] = yuvTmp[i + 2];
                U[des]  = yuvTmp[i + 1];
                V[des]  = yuvTmp[i + 3];
            break;
        case YUVFmt_YVYU :
                YU[des] = yuvTmp[i + 1];
                YV[des] = yuvTmp[i + 3];
                U[des]  = yuvTmp[i];
                V[des]  = yuvTmp[i + 2];
            break;
        default : 
            FTM_CAMERA_ERR("wrong RAW format\n");
            fclose(pFp);
            return -1;
        }
    }
    
    I->C1 = YU;
    I->C2 = U;
    I->C3 = V;
    I->C4 = YV;
    I->channel_num = 3;

    fclose(pFp);

    FTM_CAMERA_DBG("[loadYuvImage] -\n");
    return 0;
}


/*******************************************************************************
*!  brief pin-error test
********************************************************************************/
MINT32 BitErrorAnalysis(AnaImage *I, 
                           MUINT32 bit, 
                           MUINT32 height, 
                           MUINT32 width, 
                           MUINT32 inShutter)
{  
    FTM_CAMERA_DBG("[BitErrorAnalysis] +\n");
    
    MINT32 *histC1, *histC2, *histC3, *histC4;
    MINT32 start = -1, end = -1, startValue = -1;
    MUINT32 dataRange, rangeAll, countCtn, countBrk, countBrk_total;
    MUINT32 maxStart, minEnd;
    double breakTH = 0;
    double midp = 0, sgain = 1, gmin, gmax;
    MBOOL *breakPoint;
    MBOOL flag = MFALSE;

    rangeAll = 1 << bit;
    midp = rangeAll >> 1;

    histC1 = (MINT32 *)malloc(sizeof(MINT32) << bit);
    histC2 = (MINT32 *)malloc(sizeof(MINT32) << bit);
    histC3 = (MINT32 *)malloc(sizeof(MINT32) << bit);
    histC4 = (MINT32 *)malloc(sizeof(MINT32) << bit);
    breakPoint = (MBOOL *)malloc(sizeof(MBOOL) << bit);

    for(MUINT32 i = 0; i < rangeAll; i++)
    {
        histC1[i] = 0;
        histC2[i] = 0;
        histC3[i] = 0;
        histC4[i] = 0;
    }

    if(I->channel_num == 3) // RAW, YUV 
    {
        for(MUINT32 i = 0; i < height; i++)
        {
            for(MUINT32 j = 0; j < width; j++)
            {
                MUINT32 des = i * width + j;
                histC1[I->C1[des]]++;
                histC2[I->C2[des]]++;
                histC3[I->C3[des]]++;
                histC1[I->C4[des]]++;
            }
        }
    }
    else if(I->channel_num == 4)    // RGBC
    {
        for(MUINT32 i = 0; i < height; i++)
        {
            for(MUINT32 j = 0; j < width; j++)
            {
                MUINT32 des = i * width + j;
                histC1[I->C1[des]]++;
                histC2[I->C2[des]]++;
                histC3[I->C3[des]]++;
                histC4[I->C4[des]]++;
            }
        }
    }

    for(MUINT32 i = 0; i < rangeAll; i++)
    {
        if(histC1[i]> 5 || histC2[i] > 5 || histC3[i] > 5 || histC4[i] > 5)
        {
            breakPoint[i] = 0;

            if(start == -1)
            {
                start = i;
                startValue = max(max(histC1[i],histC2[i]),histC3[i]);
                startValue = max(max(max(histC1[i+1],histC2[i+1]),histC3[i+1]),startValue);
            }
        }
        else
        {
            breakPoint[i] = 1;
        }
    }

    for(MINT32 i = (rangeAll - 1); i >= 0; i--)
    {
        if(breakPoint[i] == 0)
        {
            end = i;
            break;
        }
    }

    dataRange = end - start + 1;
    breakTH = width * height / dataRange * 0.4;

    if(startValue > breakTH && (abs((rangeAll>>1) - dataRange) < 3 || abs( (rangeAll >> 2) - dataRange) < 3))      // bit-10 error detection
    {
        flag = MTRUE;
        gPinErrType = 31;
    }
    else if(dataRange <= (rangeAll >> 3))   // bit-10 error detection
    {
        flag = MTRUE;
        gPinErrType = 32;
    }
    else    // bit-2 ~ bit-9 error detection
    {
        countBrk_total = 0;
        countBrk = 0;

        for (MINT32 i = start; i <= end; i++)
        {
            if(breakPoint[i] == 1)
            {
                countBrk = countBrk + 1;
            }
            else
            {
                if(countBrk > 1)
                {
                    countBrk_total = countBrk_total + countBrk;
                }
                countBrk = 0;
            }
        }

        if(countBrk_total >= dataRange / 3)
        {
            flag = MTRUE;
            gPinErrType = 2;
        }
    }
    
    if(flag == MFALSE)   // bit-1 error detection
    {
        countCtn = 0;
        flag = MTRUE;
        gPinErrType = 1;

        for(MINT32 i = start; i <= end; i++)
        {
            if(breakPoint[i])
            {
                countCtn = 0;
            }
            else
            {
                countCtn = countCtn + 1;
            }

            if(countCtn == 2)
            {
                flag = MFALSE;
                gPinErrType = 0;
                break;
            }
        }
    }

    gPinError_pass = !flag;

    // check the lighting condition of the test enviroment
    if(gPinError_pass)
    {
        maxStart = (midp - (1 << (bit >> 1)));
        minEnd	 = (midp + (1 << (bit >> 1)));
        
        FTM_CAMERA_DBG("active range check: (%d, %d) / (%d, %d) for %d-bit sensor",start, end, maxStart, minEnd, bit);

        if(end < minEnd )
        {
            gPinErrType = 91;
            gPinError_pass = MFALSE;
            gmin = minEnd / end;
            gmax = maxStart / start;

            if(gmax < gmin)
            {
                gPinErrType = 90;
                FTM_CAMERA_ERR("Fail *** WARNING: pleace check the shading correction is OFF ***\n");
            }
            else
            {
                sgain = min(max((1 << (bit >> 1)) * 4.0 / dataRange, midp / (start + 0.75 * dataRange)), gmax);
                gRecShutter = sgain * inShutter;
                // recShutter = (double)inShutter * (minEnd + (1<<(bit>>1))) / end;
                FTM_CAMERA_DBG("Fail *** WARNING: lighting enviroment is too dark ***\n");
                FTM_CAMERA_DBG("gRecShutter(%u)\n",gRecShutter);
            }
        }
        else if(start > maxStart)
        {
            gPinErrType = 92;
            gPinError_pass = MFALSE;
            gmin = minEnd / end;
            gmax = maxStart / start;

            if(gmax < gmin)
            {
                gPinErrType = 90;
                FTM_CAMERA_ERR("Fail *** WARNING: pleace check the shading correction is OFF ***\n");
            }
            else
            {
                sgain = max(midp / (start + 0.75 * dataRange), gmin);
                gRecShutter = sgain * inShutter;
                // recShutter = (double)inShutter * start / (maxStart - (1<<(bit>>1)));
                FTM_CAMERA_DBG("Fail *** WARNING: over-lighting! (pleace lower the exposure or move away from lighting) ***\n");
                FTM_CAMERA_DBG("gRecShutter(%u)\n",gRecShutter);
            }
        }
        else
        {
            FTM_CAMERA_DBG("pin-error detection OK\n");
        }
    }
    else
    {
        FTM_CAMERA_DBG("pin-error detection Fail\n");        
    }

    FTM_CAMERA_DBG("[BitErrorAnalysis]-\n");
    return 0;
}


/*******************************************************************************
*!  brief for Check CRC
********************************************************************************/
static MINT32 Camera_check_crc(MUINT8 *out_buffer, MUINT32 crc, MUINT32 size)
{
  register MUINT32 crc_accum = 0;

  FTM_CAMERA_DBG("[Camera_check_crc]\n");
  while (size-- > 0)
  {
    crc_accum = (crc_accum << 8) ^ ACDK_CRC_Table[(MUINT8)(crc_accum >> 24) ^ (*out_buffer++)];
  }
  FTM_CAMERA_DBG("CRC_accum 0x%x\n", ~crc_accum);

  if (~crc_accum == crc) return 1;

  return 0;
}



/////////////////////////////////////////////////////////////////////////
//
//   vCapCb () -
//!
//!  brief for capture callback
//
/////////////////////////////////////////////////////////////////////////
static void vCapCb(void *a_pParam)
{
    FTM_CAMERA_DBG("[vCapCb]+\n");    
    
    gCapCBDone = MTRUE;
    g_u4ImgCnt++;
}

/*******************************************************************************
*!  brief for auto-testing capture callback
********************************************************************************/
static void vAutoCapCb(void *a_pParam)
{
    FTM_CAMERA_DBG("[vAutoCapCb]+\n");

    ImageBufInfo *pImgBufInfo = (ImageBufInfo *)a_pParam;

    if(pImgBufInfo->eImgType == PURE_RAW10_TYPE)
    {
        FTM_CAMERA_DBG("[vAutoCapCb] PURE_RAW10_TYPE\n");
        FTM_CAMERA_DBG("[vAutoCapCb] addr(0x%x),size(%u)\n",(MUINT32)pImgBufInfo->RAWImgBufInfo.bufAddr, pImgBufInfo->RAWImgBufInfo.imgSize);
        FTM_CAMERA_DBG("[vAutoCapCb] W(%u),H(%u)\n", pImgBufInfo->RAWImgBufInfo.imgWidth,pImgBufInfo->RAWImgBufInfo.imgHeight);
        FTM_CAMERA_DBG("[vAutoCapCb] depth(%u),packed(%d),colorOrder(%d)\n",pImgBufInfo->RAWImgBufInfo.bitDepth,
                                                                              pImgBufInfo->RAWImgBufInfo.isPacked,
                                                                              pImgBufInfo->RAWImgBufInfo.eColorOrder);

        gImgFmt = 1; //RAW
        gImgW = pImgBufInfo->RAWImgBufInfo.imgWidth;
        gImgH = pImgBufInfo->RAWImgBufInfo.imgHeight;
        gBitDepth = pImgBufInfo->RAWImgBufInfo.bitDepth;
        gColorOdr = pImgBufInfo->RAWImgBufInfo.eColorOrder;

        char szFileName[256];
        MINT32 i4WriteCnt = 0;

        sprintf(szFileName, "%s/ftm_pureRAW_10.raw" , MEDIA_PATH);

        FILE *pFp = fopen(szFileName, "wb");

        if(NULL == pFp)
        {
            FTM_CAMERA_ERR("[vAutoCapCb] Can't open file to save RAW image");
            return;
        }

        i4WriteCnt = fwrite(pImgBufInfo->RAWImgBufInfo.bufAddr, 1, pImgBufInfo->RAWImgBufInfo.imgSize, pFp);

        fflush(pFp);

        if(0 != fsync(fileno(pFp)))
        {
            FTM_CAMERA_ERR("[vAutoCapCb] RAW fync fail");
            fclose(pFp);
            return;
        }

        FTM_CAMERA_DBG("[vAutoCapCb] Save image file name:%s,w(%u),h(%u)",szFileName,pImgBufInfo->RAWImgBufInfo.imgWidth,pImgBufInfo->RAWImgBufInfo.imgHeight);

        fclose(pFp);
    }
    else if(pImgBufInfo->eImgType == YUV_TYPE)
    {
        FTM_CAMERA_DBG("[vAutoCapCb] YUV_TYPE\n");
        FTM_CAMERA_DBG("[vAutoCapCb] addr(0x%x),size(%u)\n",(MUINT32)pImgBufInfo->imgBufInfo.bufAddr, pImgBufInfo->imgBufInfo.imgSize);
        FTM_CAMERA_DBG("[vAutoCapCb] W(%u),H(%u)\n", pImgBufInfo->imgBufInfo.imgWidth,pImgBufInfo->imgBufInfo.imgHeight);
        FTM_CAMERA_DBG("[vAutoCapCb] YUVImgFmt(%d)\n",pImgBufInfo->imgBufInfo.imgFmt);

        gImgFmt = 2; //YUV
        gImgW = pImgBufInfo->imgBufInfo.imgWidth;
        gImgH = pImgBufInfo->imgBufInfo.imgHeight;
        gBitDepth = 0;
        gColorOdr = pImgBufInfo->imgBufInfo.imgFmt;

        char szFileName[256];
        MINT32 i4WriteCnt = 0;

        switch(pImgBufInfo->imgBufInfo.imgFmt)
        {
            case YUVFmt_UYVY : sprintf(szFileName, "%s/YUVImg_UYVY.yuv" , MEDIA_PATH);
                break;
            case YUVFmt_VYUY : sprintf(szFileName, "%s/YUVImg_VYUY.yuv" , MEDIA_PATH);
                break;
            case YUVFmt_YUY2 : sprintf(szFileName, "%s/YUVImg_YUY2.yuv" , MEDIA_PATH);
                break;
            case YUVFmt_YVYU : sprintf(szFileName, "%s/YUVImg_YVYU.yuv" , MEDIA_PATH);
                break;
        }
        
        FILE *pFp = fopen(szFileName, "wb");

        if(NULL == pFp)
        {
            FTM_CAMERA_ERR("[vAutoCapCb] Can't open file to save YUV image");
            return;
        }

        i4WriteCnt = fwrite(pImgBufInfo->imgBufInfo.bufAddr, 1, pImgBufInfo->imgBufInfo.imgSize, pFp);

        fflush(pFp);

        if(0 != fsync(fileno(pFp)))
        {
            FTM_CAMERA_ERR("[vAutoCapCb] YUV fync fail");
            fclose(pFp);
            return;
        }

        FTM_CAMERA_DBG("[vAutoCapCb] Save image file name:%s,w(%u),h(%u)", szFileName, pImgBufInfo->imgBufInfo.imgWidth,pImgBufInfo->imgBufInfo.imgHeight);

        fclose(pFp);        
    }
    else
    {
        FTM_CAMERA_DBG("[vAutoCapCb] UnKnow Format\n");       
    }

    gCapCBDone = MTRUE;
    g_u4ImgCnt++;
}

/*******************************************************************************
*!  brief for auto-testing capture callback
********************************************************************************/
static void vAutoCapCb_patg(void *a_pParam)
{
    FTM_CAMERA_DBG("[vAutoCapCb_patag]+\n");

    ImageBufInfo *pImgBufInfo = (ImageBufInfo *)a_pParam;
    MINT32 u32ATA_Mode;
    
#ifdef ATA_MODE_GET_RAWDATA
    u32ATA_Mode = get_is_ata();
    FTM_CAMERA_DBG("[vAutoCapCb_patag] ATA Mode = %d \n",u32ATA_Mode);
#else
    u32ATA_Mode = 0;
#endif


    
    if(pImgBufInfo->eImgType == PURE_RAW10_TYPE)
    {
        FTM_CAMERA_DBG("[vAutoCapCb] PURE_RAW10_TYPE\n");
        FTM_CAMERA_DBG("[vAutoCapCb] addr(0x%x),size(%u)\n",(MUINT32)pImgBufInfo->RAWImgBufInfo.bufAddr, pImgBufInfo->RAWImgBufInfo.imgSize);
        FTM_CAMERA_DBG("[vAutoCapCb] W(%u),H(%u)\n", pImgBufInfo->RAWImgBufInfo.imgWidth,pImgBufInfo->RAWImgBufInfo.imgHeight);
        FTM_CAMERA_DBG("[vAutoCapCb] depth(%u),packed(%d),colorOrder(%d)\n",pImgBufInfo->RAWImgBufInfo.bitDepth,
                                                                              pImgBufInfo->RAWImgBufInfo.isPacked,
                                                                              pImgBufInfo->RAWImgBufInfo.eColorOrder);
        gImgFmt = 1; //RAW
        gImgW = pImgBufInfo->RAWImgBufInfo.imgWidth;
        gImgH = pImgBufInfo->RAWImgBufInfo.imgHeight;
        gBitDepth = pImgBufInfo->RAWImgBufInfo.bitDepth;
        gColorOdr = pImgBufInfo->RAWImgBufInfo.eColorOrder;

        if(1 == Camera_check_crc(pImgBufInfo->RAWImgBufInfo.bufAddr,gCRCValue,pImgBufInfo->RAWImgBufInfo.imgSize))
        {
            gPinError_pass = MTRUE;
            FTM_CAMERA_DBG("CRC Pass\n");
        
        }
        else
        {
            gPinError_pass = MFALSE;
            FTM_CAMERA_DBG("CRC Fail\n");
        }

        if((gPinError_pass == MFALSE) || u32ATA_Mode)
        {
            char szFileName[256];
            MINT32 i4WriteCnt = 0;
            if(u32ATA_Mode)
                sprintf(szFileName, "%s/ftm_CamRawData.raw" , MEDIA_PATH);
            else
                sprintf(szFileName, "%s/ftm_pureRAW_10.raw" , MEDIA_PATH);
            
            FILE *pFp = fopen(szFileName, "wb");
            
            if(NULL == pFp)
            {
                FTM_CAMERA_ERR("[vAutoCapCb] Can't open file to save RAW image");
                return;
            }
            
            i4WriteCnt = fwrite(pImgBufInfo->RAWImgBufInfo.bufAddr, 1, pImgBufInfo->RAWImgBufInfo.imgSize, pFp);
            
            fflush(pFp);
            
            if(0 != fsync(fileno(pFp)))
            {
                FTM_CAMERA_ERR("[vAutoCapCb] RAW fync fail");
                fclose(pFp);
                return;
            }
            
            FTM_CAMERA_DBG("[vAutoCapCb] Save image file name:%s,w(%u),h(%u)",szFileName,pImgBufInfo->RAWImgBufInfo.imgWidth,pImgBufInfo->RAWImgBufInfo.imgHeight);
            
            fclose(pFp);
        }

    }
    else if(pImgBufInfo->eImgType == YUV_TYPE)
    {
        FTM_CAMERA_DBG("[vAutoCapCb] YUV_TYPE\n");
        FTM_CAMERA_DBG("[vAutoCapCb] addr(0x%x),size(%u)\n",(MUINT32)pImgBufInfo->imgBufInfo.bufAddr, pImgBufInfo->imgBufInfo.imgSize);
        FTM_CAMERA_DBG("[vAutoCapCb] W(%u),H(%u)\n", pImgBufInfo->imgBufInfo.imgWidth,pImgBufInfo->imgBufInfo.imgHeight);
        FTM_CAMERA_DBG("[vAutoCapCb] YUVImgFmt(%d)\n",pImgBufInfo->imgBufInfo.imgFmt);

        gImgFmt = 2; //YUV
        gImgW = pImgBufInfo->imgBufInfo.imgWidth;
        gImgH = pImgBufInfo->imgBufInfo.imgHeight;
        gBitDepth = 0;
        gColorOdr = pImgBufInfo->imgBufInfo.imgFmt;

        //if(1 == Camera_check_crc(pImgBufInfo->imgBufInfo.bufAddr,gCRCValue,pImgBufInfo->imgBufInfo.imgSize))
        if(1 == Camera_check_crc(pImgBufInfo->imgBufInfo.bufAddr,gCRCValue,pImgBufInfo->RAWImgBufInfo.imgSize))
        {
            gPinError_pass = MTRUE;
            FTM_CAMERA_DBG("CRC Pass\n");
        
        }
        else
        {
            gPinError_pass = MFALSE;
            FTM_CAMERA_DBG("CRC Fail\n");
        }

        if((gPinError_pass == MFALSE) || u32ATA_Mode)
        {
            char szFileName[256];
            MINT32 i4WriteCnt = 0;

            if(u32ATA_Mode)
            {
                sprintf(szFileName, "%s/ftm_CamRawData.raw" , MEDIA_PATH);
            }
            else
            {
                switch(pImgBufInfo->imgBufInfo.imgFmt)
                {
                    case YUVFmt_UYVY : sprintf(szFileName, "%s/YUVImg_UYVY.yuv" , MEDIA_PATH);
                        break;
                    case YUVFmt_VYUY : sprintf(szFileName, "%s/YUVImg_VYUY.yuv" , MEDIA_PATH);
                        break;
                    case YUVFmt_YUY2 : sprintf(szFileName, "%s/YUVImg_YUY2.yuv" , MEDIA_PATH);
                        break;
                    case YUVFmt_YVYU : sprintf(szFileName, "%s/YUVImg_YVYU.yuv" , MEDIA_PATH);
                        break;
                }
            }
            
            FILE *pFp = fopen(szFileName, "wb");
            
            if(NULL == pFp)
            {
                FTM_CAMERA_ERR("[vAutoCapCb] Can't open file to save YUV image");
                return;
            }
            
            i4WriteCnt = fwrite(pImgBufInfo->imgBufInfo.bufAddr, 1, pImgBufInfo->imgBufInfo.imgSize, pFp);
            
            fflush(pFp);
            
            if(0 != fsync(fileno(pFp)))
            {
                FTM_CAMERA_ERR("[vAutoCapCb] YUV fync fail");
                fclose(pFp);
                return;
            }
            
            FTM_CAMERA_DBG("[vAutoCapCb] Save image file name:%s,w(%u),h(%u)", szFileName, pImgBufInfo->imgBufInfo.imgWidth,pImgBufInfo->imgBufInfo.imgHeight);
            
            fclose(pFp); 
        }

    }
    else
    {
        FTM_CAMERA_DBG("[vAutoCapCb_patg] UnKnow Format\n");       
    }

    gCapCBDone = MTRUE;
    g_u4ImgCnt++;
}


/*******************************************************************************
*!  brief for camera capture test
********************************************************************************/
static int camera_capture_test()
{
    FTM_CAMERA_DBG("camera_capture_test-E\n");

    unsigned int u4RetLen = 0;
    MBOOL bRet;

#if 1   //JPEG capture

#if 0
    ACDK_CCT_SENSOR_RESOLUTION_STRUCT  SensorResolution;

    memset(&SensorResolution,0,sizeof(ACDK_CCT_SENSOR_RESOLUTION_STRUCT));

    FTM_CAMERA_DBG("[camera_capture_test] Get Sensor Resolution Info\n");

    bRet = bSendDataToACDK(ACDK_CCT_V2_OP_GET_SENSOR_RESOLUTION,
                           NULL,
                           0,
                           (UINT8 *)&SensorResolution,
                           sizeof(ACDK_CCT_SENSOR_RESOLUTION_STRUCT),
                           &u4RetLen);


    if (!bRet)
    {
        FTM_CAMERA_DBG("[camera_capture_test]Get Sensor Resolution Fail \n");
        return 1;
    }

    FTM_CAMERA_DBG("SensorFullWidth(%u),SensorFullHeight(%u)\n",SensorResolution.SensorFullWidth,SensorResolution.SensorFullHeight);  
    FTM_CAMERA_DBG("JPEGEncWidth(%u),JPEGEncHeight(%u)\n",rACDKCapConfig.u2JPEGEncWidth,rACDKCapConfig.u2JPEGEncHeight);
#endif

    ACDK_CAPTURE_STRUCT rACDKCapConfig;

    rACDKCapConfig.eCameraMode   = CAPTURE_MODE;
    rACDKCapConfig.eOperaMode    = ACDK_OPT_FACTORY_MODE;
    rACDKCapConfig.eOutputFormat = JPEG_TYPE;
    
    rACDKCapConfig.u2JPEGEncWidth =  0;     //will be alignd to 16x
    rACDKCapConfig.u2JPEGEncHeight =  0;    //will be alignd to 16x
    rACDKCapConfig.fpCapCB = vCapCb;
    rACDKCapConfig.i4IsSave = 1;    // 0-no save, 1-save   

#else   //RAW capture

    ACDK_CCT_STILL_CAPTURE_STRUCT rACDKCapConfig;

    rACDKCapConfig.eCameraMode   = CAPTURE_MODE;
    rACDKCapConfig.eOperaMode    = ACDK_OPT_FACTORY_MODE;
    rACDKCapConfig.eOutputFormat = OUTPUT_PURE_RAW10; 

    rACDKCapConfig.u2JPEGEncWidth  =  0;    //will be alignd to 16x
    rACDKCapConfig.u2JPEGEncHeight =  0;    //will be alignd to 16x
    rACDKCapConfig.fpCapCB = vCapCb;
    rACDKCapConfig.i4IsSave = 1;    // 0-no save, 1-save

#endif



    gCapCBDone = MFALSE;
    bRet = bSendDataToACDK(ACDK_CMD_CAPTURE,
                           (unsigned char *)&rACDKCapConfig,
                           sizeof(ACDK_CAPTURE_STRUCT),
                           NULL,
                           0,
                           &u4RetLen);

    //wait capture done
    if(bRet == MTRUE)
    {
        FTM_CAMERA_DBG("[camera_capture_test] wait capture done\n");
        while(!gCapCBDone)
        {
            usleep(1000);
        }
    }
    else
    {
        FTM_CAMERA_DBG("[camera_capture_test] capture fail\n");
        return 1;
    }

    FTM_CAMERA_DBG("[camera_capture_test] X\n");
    return 0;
}


/*******************************************************************************
*!  brief for camera capture auto-test
********************************************************************************/
static int camera_capture_autotest()
{
    FTM_CAMERA_DBG("camera_capture_autotest-E\n");

    MUINT32 u4RetLen = 0, oriShutter = 0;
    MBOOL bRet;

    ACDK_CAPTURE_STRUCT rACDKCapConfig;

    rACDKCapConfig.eCameraMode   = CAPTURE_MODE;
    rACDKCapConfig.eOperaMode    = ACDK_OPT_FACTORY_MODE;
    rACDKCapConfig.eOutputFormat = PURE_RAW10_TYPE; 

    rACDKCapConfig.u2JPEGEncWidth  =  0;    //will be alignd to 16x
    rACDKCapConfig.u2JPEGEncHeight =  0;    //will be alignd to 16x
    rACDKCapConfig.fpCapCB = vAutoCapCb;
    rACDKCapConfig.i4IsSave = 0;    // 0-no save, 1-save
    rACDKCapConfig.bUnPack= MTRUE;

    //======  2nd Auto-Test Need To Set Shutter Time Forcedly ======
    
    if(autoTest_2nd == MTRUE)
    {
        MUINT32 newShutter = gRecShutter;

        FTM_CAMERA_DBG("newShutter(%u)\n",newShutter);
        
        bRet = bSendDataToACDK(ACDK_CMD_SET_SHUTTER_TIME,
                               (MUINT8 *)&newShutter,
                               sizeof(MUINT32),
                               NULL,
                               0,
                               &u4RetLen);
    }

    //====== Single Capture ======
    
    gCapCBDone = MFALSE;
    bRet = bSendDataToACDK(ACDK_CMD_CAPTURE,
                           (unsigned char *)&rACDKCapConfig,
                           sizeof(ACDK_CAPTURE_STRUCT),
                           NULL,
                           0,
                           &u4RetLen);
    //====== Wait Capture Done ======
    
    if(bRet == MTRUE)
    {
        FTM_CAMERA_DBG("[camera_capture_autotest] wait capture done\n");
        while(!gCapCBDone)
        {
            usleep(1000);
        }
    }
    else
    {
        FTM_CAMERA_DBG("[camera_capture_autotest] capture fail\n");
        return 1;
    }

    //====== 1st Auto-Test Need To Get Shutter Time ======
    
    if(autoTest_2nd == MFALSE)
    {        
        bRet = bSendDataToACDK(ACDK_CMD_GET_SHUTTER_TIME,
                                 NULL,
                                 0,
                                 (MUINT8 *)&oriShutter,
                                 sizeof(MUINT32),
                                 &u4RetLen);

        FTM_CAMERA_DBG("oriShutter(%u)\n",oriShutter);
    }

    //====== Pin-Error Detect ======

    AnaImage *ana_image = (AnaImage *)malloc(sizeof(AnaImage));
    MINT32 err = 0;    

    // variable setting
    gPinErrType = 0;
    gRecShutter = 0;

    // load image
    if(gImgFmt == 1)    //RAW
    {
        err = loadRawImage(gImgH,
                             gImgW,
                             gColorOdr,
                             ana_image);
    }
    else if(gImgFmt == 2)   //YUV
    {
        err = loadYuvImage(gImgH,
                             gImgW,
                             gColorOdr,
                             ana_image);
    }
    else
    {
        FTM_CAMERA_ERR("[camera_capture_autotest] not support format\n");
    }

    // pin-error algorithm
    if(err != 0)
    {
        FTM_CAMERA_ERR("[camera_capture_autotest] load image fail\n");
    }
    else
    {
        BitErrorAnalysis(ana_image,
                           gBitDepth,
                           gImgH >> 1,
                           gImgW >> 1,
                           oriShutter);
    }

    //====== Check Detection Result ======

    free(ana_image);   

    if(gPinErrType != 0)
    {
        FTM_CAMERA_DBG("[camera_capture_autotest] BitErrorAnalysis fail(%d)\n",gPinErrType);
    }

    FTM_CAMERA_DBG("[camera_capture_autotest] X\n");
    return 0;
}


/*******************************************************************************
*!  brief for camera capture auto-test
********************************************************************************/
static int camera_capture_autotest_patg()
{
    FTM_CAMERA_DBG("camera_capture_autotest_patg-E\n");

    MUINT32 u4RetLen = 0, oriShutter = 0, u32checksum = 0;
    MBOOL bRet;

    ACDK_CAPTURE_STRUCT rACDKCapConfig;

    rACDKCapConfig.eCameraMode   = CAPTURE_MODE;
    rACDKCapConfig.eOperaMode    = ACDK_OPT_FACTORY_MODE;
    rACDKCapConfig.eOutputFormat = PURE_RAW10_TYPE; 

    rACDKCapConfig.u2JPEGEncWidth  =  0;    //will be alignd to 16x
    rACDKCapConfig.u2JPEGEncHeight =  0;    //will be alignd to 16x
    rACDKCapConfig.fpCapCB = vAutoCapCb_patg;
    rACDKCapConfig.i4IsSave = 0;    // 0-no save, 1-save
    rACDKCapConfig.bUnPack= MFALSE;


    //======  To get sensor driver checksum value  ======  
    bRet = bSendDataToACDK(ACDK_CMD_GET_CHECKSUM,
                           NULL,
                           0,
                           (MUINT8 *)&u32checksum,
                           sizeof(MUINT32),
                           &u4RetLen);
    gCRCValue = u32checksum;

    //====== Single Capture ======
    
    gCapCBDone = MFALSE;
    bRet = bSendDataToACDK(ACDK_CMD_CAPTURE,
                           (unsigned char *)&rACDKCapConfig,
                           sizeof(ACDK_CAPTURE_STRUCT),
                           NULL,
                           0,
                           &u4RetLen);


    //====== Wait Capture Done ======
    
    if(bRet == MTRUE)
    {
        FTM_CAMERA_DBG("[camera_capture_autotest] wait capture done\n");
        while(!gCapCBDone)
        {
            usleep(1000);
        }
    }
    else
    {
        FTM_CAMERA_DBG("[camera_capture_autotest] capture fail\n");
        return 1;
    }

    FTM_CAMERA_DBG("[camera_capture_autotest] X\n");
    return 0;
}


/////////////////////////////////////////////////////////////////////////
//
//   camera_preview_stop () -
//!
//!  brief for camera preview stop
//
/////////////////////////////////////////////////////////////////////////
static int camera_preview_stop(void)
{
    if(camera_state != CAMERA_STATE_PREVIEW)
    {
        FTM_CAMERA_ERR("[camera_preview_stop] camera_state != CAMERA_STATE_PREVIEW\n");
        return 0;
    }

    FTM_CAMERA_DBG("[camera_preview_stop] Stop Camera Preview\n");

    unsigned int u4RetLen = 0;
    bool bRet = bSendDataToACDK(ACDK_CMD_PREVIEW_STOP, NULL, 0, NULL, 0, &u4RetLen);

    if (!bRet)
    {
        return -1;
    }

    camera_state = CAMERA_STATE_IDLE;
    g_previewStop = MTRUE;

    FTM_CAMERA_DBG("[camera_preview_stop] X\n");
    return 0;
}


/////////////////////////////////////////////////////////////////////////
//
//   camera_reset_layer_buffer () -
//!
//!  brief for camera reset camera preview layer buffer
//
/////////////////////////////////////////////////////////////////////////
static int camera_reset_layer_buffer(void)
{
    unsigned int u4RetLen = 0;
    bool bRet = 0;
    ACDK_FEATURE_INFO_STRUCT rAcdkFeatureInfo;

    rAcdkFeatureInfo.puParaIn = NULL;
    rAcdkFeatureInfo.u4ParaInLen = 0;
    rAcdkFeatureInfo.puParaOut = NULL;
    rAcdkFeatureInfo.u4ParaOutLen = 0;
    rAcdkFeatureInfo.pu4RealParaOutLen = &u4RetLen;

    bRet = Mdk_IOControl(ACDK_CMD_RESET_LAYER_BUFFER, &rAcdkFeatureInfo);
    if (!bRet)
    {
        return -1;
    }
    return 0;
}

/////////////////////////////////////////////////////////////////////////
//
//   vQuickViewCallback () -
//!
//!  brief for camera quick view callback
//
/////////////////////////////////////////////////////////////////////////

static volatile int quickViewDone = MFALSE;
MVOID vQuickViewCallback(MVOID *a_pArg)
{
    FTM_CAMERA_DBG("Quick View the image callback \n");

    quickViewDone = MTRUE;
}

/////////////////////////////////////////////////////////////////////////
//
//   camera_show_image () -
//!
//!  brief for camera show image on the screen
//
/////////////////////////////////////////////////////////////////////////
static int camera_show_image(void)
{
#if 0   //ACDK will show QV automatic
    if(camera_state != CAMERA_STATE_IDLE)
    {
        FTM_CAMERA_ERR("[camera_show_image] camera_state != CAMERA_STATE_IDLE\n");
        return 0;
    }

    FTM_CAMERA_DBG("QuickView the Image\n");

    MUINT32 u4RetLen = 0;
    ACDK_FEATURE_INFO_STRUCT rAcdkFeatureInfo;
    rAcdkFeatureInfo.puParaIn =  (MUINT8 *)szFileName;
    rAcdkFeatureInfo.u4ParaInLen = 256;
    rAcdkFeatureInfo.puParaOut = (MUINT8 *)vQuickViewCallback;
    rAcdkFeatureInfo.u4ParaOutLen = 0;
    rAcdkFeatureInfo.pu4RealParaOutLen =&u4RetLen;

    bool bRet = Mdk_IOControl(ACDK_CCT_FEATURE_QUICK_VIEW_IMAGE, &rAcdkFeatureInfo);

    if (!bRet)
    {
        FTM_CAMERA_ERR("[camera_show_image] QV fail\n");
        return 1;
    }

    FTM_CAMERA_DBG("[camera_show_image]-X\n");
#endif
    quickViewDone = MTRUE;
    return 0;
}



/*******************************************************************************
*
********************************************************************************/
static volatile int capture_done = MFALSE;
static int camera_cap_key_handler(int key, void *priv)
{
    int err = 0;
    int handled = 0, exit = 0;
    struct camera *cam = (struct camera *)priv;
    struct textview *tv = &cam->tv;
    struct ftm_module *fm = cam->mod;

    FTM_CAMERA_DBG("%s: Enter\n", __FUNCTION__);

    switch (key)
    {
    case UI_KEY_CONFIRM:
        FTM_CAMERA_DBG("Key_Confirm Click - Capture \n");

#ifdef SMT_USED
            FTM_CAMERA_DBG("Capture Function Not Ready\n");
            handled = -1;
#else
        if (gcamera_autoTest == 1)
            err = camera_capture_autotest_patg();
        else
            err = camera_capture_test();
        
        if(err == 0) 
        {
            capture_done = MTRUE; 
        }
        else
        {
            FTM_CAMERA_DBG("Capture Fail\n");
        }
        exit = 1;
#endif //end of SMT_USED
        break;
    case UI_KEY_BACK:
        FTM_CAMERA_DBG("Back Button Click - Back\n");                 
        exit = 1;
        break;
    default:
        handled = -1;
        break;
    }
    if (exit)
    {
        FTM_CAMERA_DBG( "%s: Exit thead\n", __FUNCTION__);
        cam->exit_thd = MTRUE;
        tv->exit(tv);
    }
    return handled;
}

/*******************************************************************************
*
********************************************************************************/
#if 0
static int camera_preview_key_handler(int key, void *priv)
{
    int handled = 0, exit = 0;
    struct camera *cam = (struct camera *)priv;
    struct textview *tv = &cam->tv;
    struct ftm_module *fm = cam->mod;

    switch (key) {
    case UI_KEY_CONFIRM:
        FTM_CAMERA_DBG("Key Confirm press \n");
        cam->mod->test_result = FTM_TEST_PASS;
        exit = 1;
        break;
    case UI_KEY_BACK:
        FTM_CAMERA_DBG("Key back press \n");
        cam->mod->test_result = FTM_TEST_FAIL;
        exit = 1;
        break;
    default:
        handled = -1;
        break;
    }
    if (exit) {
        FTM_CAMERA_DBG( "%s: Exit thead\n", __FUNCTION__);
        cam->exit_thd = true;
        tv->exit(tv);
    }
    return handled;
}
#endif

/*******************************************************************************
*
********************************************************************************/
static void *camera_update_capture_tv_thread(void *priv)
{
    struct camera *cam = (struct camera *)priv;
    struct textview *tv = &cam->tv;
    struct statfs stat;
    int count = 1, chkcnt = 5;
    int key;

    //tv = &cam ->tv;
    ui_init_textview(tv, camera_cap_key_handler, (void*)cam );
    tv->set_title(tv, &cam->title);
    tv->set_text(tv, &cam->text);
    tv->set_btn(tv, &cam->left_btn, &cam->center_btn, &cam->right_btn);
    tv->redraw(tv);

    FTM_CAMERA_DBG("%s: Start\n", __FUNCTION__);

    camera_preview_test();

    FTM_CAMERA_DBG("PREVIEW_Start\n");

    cam->exit_thd = MFALSE;

#if 0 // strobe will be independent item

#ifdef CONSTANT_FLASHLIGHT
	FTM_CAMERA_DBG("camera_update_capture_tv_thread line=%d\n",__LINE__);

    if(srcDev==1) //only for main sensor with strobe led (srcDev==1 => main sensor)
    {
    	FTM_CAMERA_DBG("camera_update_capture_tv_thread line=%d\n",__LINE__);
        if (pthread_create(&camera_flash_thread_handle, NULL, camera_flash_test_thread, cam))
        {
            FTM_CAMERA_DBG("Create flash test thread fail \n");
            return (0);
        }
    }
#else
	FTM_CAMERA_DBG("camera_update_capture_tv_thread line=%d\n",__LINE__);
#endif
#endif


    while (1)
    {
        key = ui_wait_key();
        usleep(200000);
        chkcnt--;
        tv->m_khandler(key, tv->m_priv);
        if (cam ->exit_thd)
            break;

        if (chkcnt > 0)
            continue;

        chkcnt = 5;
    }
    FTM_CAMERA_DBG("%s: Exit\n", __FUNCTION__);

#if 0 // strobe will be independent item

    if (eFlashSupport != FLASHLIGHT_NONE)
    {
        pthread_join(camera_flash_thread_handle, NULL);
        camera_flash_thread_handle = 0;
    }
#endif

    //pthread_exit(NULL);
    return NULL;
}

/*******************************************************************************
*
********************************************************************************/
static void *camera_update_auto_test(void *priv)
{
    struct camera *cam = (struct camera *)priv;
    struct textview *tv = &cam->tv;
    MINT32 chkcnt = 0;
    MINT32 key;

    ui_init_textview(tv, camera_cap_key_handler, (void*)cam );
    tv->set_title(tv, &cam->title);
    tv->set_text(tv, &cam->text);
    tv->set_btn(tv, &cam->left_btn, &cam->center_btn, &cam->right_btn);
    tv->redraw(tv);

    FTM_CAMERA_DBG("%s: Start\n", __FUNCTION__);

    camera_preview_test();

    FTM_CAMERA_DBG("PREVIEW_Start\n");

    cam->exit_thd = false;

    while(1)
    {
        ++chkcnt;
        usleep(1000);

        if(chkcnt > 1000)
            break;
    }
    
    key = UI_KEY_CONFIRM;   //capture KEY
    tv->m_khandler(key, tv->m_priv);
       
    FTM_CAMERA_DBG("%s: Exit\n", __FUNCTION__);

    return NULL;
}


/*******************************************************************************
*
********************************************************************************/
static int camera_cap_result_key_handler(int key, void *priv)
{
    int handled = 0, exit = 0;
    struct camera *cam = (struct camera *)priv;
    struct textview *tv = &cam->tv;
    struct ftm_module *fm = cam->mod;

    switch (key)
    {
    case UI_KEY_BACK:
        FTM_CAMERA_DBG("Back Button Click\n");
        exit = 1;
        break;
    default:
        handled = -1;
        break;
    }

    if (exit)
    {
        FTM_CAMERA_DBG("%s: Exit thead\n", __FUNCTION__);
        cam->exit_thd = MTRUE;
        tv->exit(tv);
    }
    return handled;
}


/*******************************************************************************
*
********************************************************************************/
static void *camera_update_showImg_tv_thread(void *priv)
{
    struct camera *cam = (struct camera *)priv;
    struct textview *tv =  &cam->tv;
    struct statfs stat;
    int count = 1, chkcnt = 5, key;

    FTM_CAMERA_DBG("%s: Start\n", __FUNCTION__);

    /* Initial the title info. */
    init_text(&cam ->left_btn, "", COLOR_YELLOW);
    init_text(&cam ->center_btn, "", COLOR_YELLOW);
    init_text(&cam ->right_btn, uistr_camera_back, COLOR_YELLOW);

    /* Initial the paintview function pointers */
    ui_init_textview(tv, camera_cap_result_key_handler,  (void*)cam);
    tv->set_title(tv, &cam->title);
    tv->set_text(tv, &cam->text);
    tv->set_btn(tv, &cam ->left_btn, &cam ->center_btn, &cam ->right_btn);
    camera_show_image();

    while (!quickViewDone)
    {
        usleep(200);
    }

    tv->redraw(tv);
    cam->exit_thd = MFALSE;

    while (1)
    {
        key = ui_wait_key();
        usleep(200000);
        chkcnt--;
        tv->m_khandler(key, tv->m_priv);
        if (cam ->exit_thd)
            break;

        if (chkcnt > 0)
            continue;

        chkcnt = 5;
    }

    FTM_CAMERA_DBG("%s: Exit\n", __FUNCTION__);
    return NULL;
}

/*******************************************************************************
*
********************************************************************************/
static int acdkIFInit()
{
    FTM_CAMERA_DBG("%s : Open ACDK\n",__FUNCTION__);

    //====== Local Variable ======

    ACDK_FEATURE_INFO_STRUCT rAcdkFeatureInfo;
    bool bRet;
    unsigned int u4RetLen;

    //====== Create ACDK Object ======
    if (Mdk_Open() == MFALSE)
    {
        FTM_CAMERA_ERR("Mdk_Open() Fail \n");
        return -1;
    }

    //====== Select Camera Sensor ======

    rAcdkFeatureInfo.puParaIn = (MUINT8 *)&srcDev;
    rAcdkFeatureInfo.u4ParaInLen = sizeof(int);
    rAcdkFeatureInfo.puParaOut = NULL;
    rAcdkFeatureInfo.u4ParaOutLen = 0;
    rAcdkFeatureInfo.pu4RealParaOutLen = &u4RetLen;

    FTM_CAMERA_DBG("%s : srcDev:%d\n",__FUNCTION__,srcDev);
    bRet = Mdk_IOControl(ACDK_CMD_SET_SRC_DEV, &rAcdkFeatureInfo);
    if (!bRet)
    {
        FTM_CAMERA_DBG("ACDK_FEATURE_SET_SRC_DEV Fail: %d\n",srcDev);
        return -1;
    }

    //====== Initialize ACDK ======

    FTM_CAMERA_DBG("%s : Init ACDK\n",__FUNCTION__);
    if(Mdk_Init() == MFALSE)
    {
        return -1;
    }

    return 0;
}


/*******************************************************************************
*
********************************************************************************/
int camera_entry(struct ftm_param *param, void *priv)
{
    char *ptr;
    int chosen;
    bool exit = MFALSE;
    int index = 0;
    struct camera *cam = (struct camera *)priv;
    struct textview *tv = NULL ;
    struct itemview *iv = NULL ;
    struct statfs stat;
//    PFLASH_LIGHT_FUNCTION_STRUCT pstFlash;
    int isTestDone = 0;
    int initDone = 1;
    char rawVal[PROPERTY_VALUE_MAX] = {'\0'};

 #ifdef CAMERA_ATA_2
    if(FTM_AUTO_ITEM  == param->test_type)
    {
        gcamera_autoTest = 1;
    }
    else
    {
        gcamera_autoTest = 0;
    }
 #endif

    property_get("camera.acdk.manual", rawVal, "0");  // 1: Manual mode, 2 : auto mode
    if(atoi(rawVal) == 1)
    {
        gcamera_autoTest = 0;
    }
    else if(atoi(rawVal) == 2)
    {
        gcamera_autoTest = 1;
    }
    
    FTM_CAMERA_DBG("[%s]AutoMode = %d\n",__FUNCTION__, gcamera_autoTest);

    init_text(&cam ->title, param->name, COLOR_YELLOW);
    init_text(&cam ->text, &cam->info[0], COLOR_YELLOW);

    cam->exit_thd = MFALSE;

    if (!cam->iv)
    {
        iv = ui_new_itemview();
        if (!iv)
        {
            FTM_CAMERA_DBG("No memory");
            return -1;
        }
        cam->iv = iv;
    }

    iv = cam->iv;
    iv->set_title(iv, &cam->title);

#if 0
    //init camera items
    eFlashSupport = FlashLightInit(&pstFlash);
    if (initDone)
    {
        if (eFlashSupport != FLASHLIGHT_NONE )
        {
            //init camera items with support flash
            camera_items[index].id = ITEM_CAMERA_TEST;
            camera_items[index].name = uistr_camera_preview_strobe;
        }
        else
        {
            //init camera items without support flash
            camera_items[index].id = ITEM_CAMERA_TEST;
            camera_items[index].name = uistr_camera_preview;
        }
        index++;
        //ITEM PASS
        camera_items[index].id = ITEM_PASS;
        camera_items[index].name = uistr_pass;
        index++;
    }
#endif

    if(initDone)
    {
        //init camera items without support flash
        camera_items[index].id = ITEM_CAMERA_TEST;

        if(srcDev == 0x02)  //sub camera
        {
            #ifdef SMT_USED
            camera_items[index].name = uistr_camera_preview;
            #else
            //camera_items[index].name = uistr_camera_preview;
            camera_items[index].name = uistr_camera_prv_cap;
            #endif
        }
        else
        {
            #ifdef SMT_USED
            camera_items[index].name = uistr_camera_preview;
            #else
            //camera_items[index].name = uistr_camera_preview;
            camera_items[index].name = uistr_camera_prv_cap;
            #endif
        }
        index++;

        //ITEM PASS
        camera_items[index].id = ITEM_PASS;
        camera_items[index].name = uistr_pass;
        index++;
    }

    //ITEM FAIL
    camera_items[index].id = ITEM_FAIL;
    camera_items[index].name = uistr_fail;
    index++;

    camera_items[index].id = -1;
    camera_items[index].name = NULL;

    iv->set_items(iv, camera_items, 0);
    iv->set_text(iv, &cam->text);

    camera_state = CAMERA_STATE_IDLE;

    // Auto test
    if(gcamera_autoTest)
    {
        MINT32 autoCnt = 0;

        FTM_CAMERA_DBG("[camera_entry]AUTO-TEST\n");

        iv->redraw(iv); // for auto-test    

        autoTest_2nd = MFALSE;
        
        while(1)
        {
            gPinError_pass = MFALSE;        
            isTestDone = 0;
            
            if(camera_state == CAMERA_STATE_IDLE)
            {
                capture_done = MFALSE;
                g_previewStop = MFALSE;

                if(0 != acdkIFInit())
                {
                    FTM_CAMERA_ERR("acdkIFInit() Fail\n");

                    Mdk_DeInit();
                    Mdk_Close();

                    memset(cam->info, 0, 1024);
                    sprintf(cam->info, "acdkIFInit Fail\n");
                    iv->set_text(iv, &cam->text);
                    iv->redraw(iv);

                    camera_state = CAMERA_STATE_NONE;
                }
                else
                {
                    init_text(&cam ->left_btn, "", COLOR_YELLOW);
                    init_text(&cam ->center_btn, uistr_camera_capture, COLOR_YELLOW);
                    //init_text(&cam ->center_btn, "", COLOR_YELLOW);
                    init_text(&cam ->right_btn, uistr_camera_back, COLOR_YELLOW);
                    
                    camera_update_auto_test(priv);
                    camera_preview_stop();          
                    camera_reset_layer_buffer();

                    memset(cam->info, 0, 1024);
                    iv->set_text(iv, &cam->text);
                    iv->redraw(iv);

                    isTestDone = 1;
                }
            }

            if(g_previewStop == MTRUE)
            {
                FTM_CAMERA_DBG("Uninit ACDK object\n");
                Mdk_DeInit();
                Mdk_Close();
            }

            //====== Err Handling ======
            
            if((gPinErrType == 91 || gPinErrType == 92) && gPinError_pass == MFALSE)
            {
                FTM_CAMERA_DBG("Too dark, do 2st auto-test\n");
                
                autoTest_2nd = MTRUE;

                if(autoCnt < 1)
                {
                    ++autoCnt;
                }
                else
                {
                    FTM_CAMERA_DBG("still too dark but reach auto-test limit times\n");
                    break;
                }
            }
            else
            {
                break;
            }
        }

        if(isTestDone == 1)
        {
            FTM_CAMERA_DBG("[camera_entry]gPinError_pass(%d)\n",gPinError_pass);
            if(gPinError_pass == MTRUE)
            {
                FTM_CAMERA_DBG("Pin-Error Auto-Test Pass\n");
                cam->mod->test_result = FTM_TEST_PASS;
            }
            else
            {
                FTM_CAMERA_DBG("Pin-Error Auto-Test Fail\n");
                cam->mod->test_result = FTM_TEST_FAIL;
             }

            cam->exit_thd = true;
        }     

    }
    else
    {
        FTM_CAMERA_DBG("[camera_entry]MANUAL-TEST\n");

        do
        {
            chosen = iv->run(iv, &exit);

            switch (chosen)
            {
            case ITEM_CAMERA_TEST:
                if(camera_state == CAMERA_STATE_IDLE)
                {
                    capture_done = MFALSE;
                    g_previewStop = MFALSE;

                    if(0 != acdkIFInit())
                    {
                        FTM_CAMERA_ERR("acdkIFInit() Fail\n");

                        Mdk_DeInit();
                        Mdk_Close();

                        memset(cam->info, 0, 1024);
                        sprintf(cam->info, "acdkIFInit Fail\n");
                        iv->set_text(iv, &cam->text);
                        iv->redraw(iv);

                        camera_state = CAMERA_STATE_NONE;
                    }
                    else
                    {
                        init_text(&cam ->left_btn, "", COLOR_YELLOW);
                    #ifdef SMT_USED
                        init_text(&cam ->center_btn, uistr_camera_preview, COLOR_YELLOW);
                    #else
                        init_text(&cam ->center_btn, uistr_camera_capture, COLOR_YELLOW);
                    #endif
                        //init_text(&cam ->center_btn, "", COLOR_YELLOW);
                        init_text(&cam ->right_btn, uistr_camera_back, COLOR_YELLOW);
                        camera_update_capture_tv_thread(priv);
                        camera_preview_stop();
                        if (capture_done == MTRUE)
                        {
                            camera_update_showImg_tv_thread(priv);
                        }
                        camera_reset_layer_buffer();

                        memset(cam->info, 0, 1024);
                        iv->set_text(iv, &cam->text);
                        iv->redraw(iv);

                        isTestDone = 1;
                    }
                }

                if(g_previewStop == MTRUE)
                {
                    FTM_CAMERA_DBG("Uninit ACDK object\n");
                    Mdk_DeInit();
                    Mdk_Close();
                }
                break;
            case ITEM_PASS:
                if(isTestDone)
                {
                    cam->mod->test_result = FTM_TEST_PASS;
                    exit = MTRUE;
                }
                else
                {
                    memset(cam->info, 0, 1024);
                    sprintf(cam->info, "Not test done !! \n");
                    iv->set_text(iv, &cam->text);
                    iv->redraw(iv);
                }
                break;
            case ITEM_FAIL:
                if(isTestDone)
                {
                    cam->mod->test_result = FTM_TEST_FAIL;
                    exit = MTRUE;
                }
                else
                {
                    memset(cam->info, 0, 1024);
                    sprintf(cam->info, "Not test done !! \n");
                    iv->set_text(iv, &cam->text);
                    iv->redraw(iv);
                }
                break;
            }

            if(exit)
            {
                cam->exit_thd = MTRUE;
                break;
            }
        } while (1);

    }


Exit:

    camera_state = CAMERA_STATE_NONE;

    return 0;
}


/*******************************************************************************
*
********************************************************************************/
int camera_main_preview_entry(struct ftm_param *param, void *priv)
{
    srcDev = 0x01; //main sensor
    return camera_entry(param,priv);
}

/*******************************************************************************
*
********************************************************************************/
int camera_main2_preview_entry(struct ftm_param *param, void *priv)
{
    srcDev = 0x08; //main2 sensor
    return camera_entry(param,priv);
}


/*******************************************************************************
*
********************************************************************************/
int camera_sub_preview_entry(struct ftm_param *param, void *priv)
{
    srcDev = 0x02; //sub sensor
    return camera_entry(param,priv);
}

/*******************************************************************************
*
********************************************************************************/
extern "C" int camera_main_init(void)
{
    int ret = 0;
    struct ftm_module *mod;
    struct camera *cam;

    FTM_CAMERA_DBG( "%s\n", __FUNCTION__);

    mod = ftm_alloc(ITEM_MAIN_CAMERA, sizeof(struct camera));
    cam  = mod_to_camera(mod);

    memset(cam, 0x0, sizeof(struct camera));

    /*NOTE: the assignment MUST be done, or exception happens when tester press Test Pass/Test Fail*/
    cam->mod = mod;

    if (!mod)
        return -ENOMEM;

    ret = ftm_register(mod, camera_main_preview_entry, (void*)cam);

    return ret;
}

/*******************************************************************************
*
********************************************************************************/
extern "C" int camera_main2_init(void)
{
    int ret = 0;
    struct ftm_module *mod;
    struct camera *cam;

    FTM_CAMERA_DBG( "%s\n", __FUNCTION__);

    mod = ftm_alloc(ITEM_MAIN2_CAMERA, sizeof(struct camera));
    cam  = mod_to_camera(mod);

    memset(cam, 0x0, sizeof(struct camera));

    /*NOTE: the assignment MUST be done, or exception happens when tester press Test Pass/Test Fail*/
    cam->mod = mod;

    if (!mod)
        return -ENOMEM;

    ret = ftm_register(mod, camera_main2_preview_entry, (void*)cam);

    return ret;
}


/*******************************************************************************
*
********************************************************************************/
extern "C" int camera_sub_init(void)
{
    int ret = 0;
    struct ftm_module *mod;
    struct camera *cam;

    FTM_CAMERA_DBG("%s\n", __FUNCTION__);

    mod = ftm_alloc(ITEM_SUB_CAMERA, sizeof(struct camera));
    cam  = mod_to_camera(mod);

    memset(cam, 0x0, sizeof(struct camera));

    /*NOTE: the assignment MUST be done, or exception happens when tester press Test Pass/Test Fail*/
    cam->mod = mod;

    if (!mod)
        return -ENOMEM;

    ret = ftm_register(mod, camera_sub_preview_entry, (void*)cam);
    return ret;
}



//#endif
