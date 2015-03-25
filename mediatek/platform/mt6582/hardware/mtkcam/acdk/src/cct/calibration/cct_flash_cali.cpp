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
// AcdkCalibration.cpp  $Revision$
////////////////////////////////////////////////////////////////////////////////

//! \file  AcdkCalibration.cpp
//! \brief

#define LOG_TAG "cct_flash_cali.cpp"

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include <sys/time.h>
#include <unistd.h>

extern "C" {
#include <linux/fb.h>
#include <linux/mtkfb.h>
}

#include "AcdkLog.h"
#include "cct_ctrl.h"
#include "cct_calibration.h"

#include <mtkcam/acdk/AcdkIF.h>
#include <mtkcam/acdk/cct_feature.h>
#include <mtkcam/drv/isp_drv.h>
#include "cct_main.h"
#include "awb_param.h"
#include "af_param.h"
#include "ae_param.h"
#include "dbg_isp_param.h"
#include "dbg_aaa_param.h"
#include "flash_mgr.h"
#include "isp_tuning_mgr.h"
#include "isp_mgr.h"
#include "flash_tuning_custom.h"
#include "./ParamLSCInternal.h"
//#include "./ShadingATNTable.h"
#include <sys/stat.h>
#include <semaphore.h>  /* Semaphore */
#include "strobe_drv.h"
#include <mtkcam/hal/sensor_hal.h>
#include <nvram_drv_mgr.h>
#include <ae_tuning_custom.h>
#include <isp_mgr.h>
#include <isp_tuning.h>
#include <isp_tuning_mgr.h>

#include <aaa_sensor_mgr.h>
#include "flash_mgr.h"
#include "cct_flash_util.h"

//#include "isp_hal.h"

//using namespace NSACDK;



#define DEBUG_PATH "/sdcard/flashdata/"


#define LogInfo(fmt, arg...) XLOGD(fmt, ##arg)
#define LogErr(fmt, arg...) XLOGD("FlashError: line=%d: "fmt, __LINE__, ##arg)
#define LogWarning(fmt, arg...) XLOGD("FlashWarning: %5d: "fmt, __LINE__, ##arg)
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
typedef struct
{
	float r;
	float g;
	float b;
}fcRgb;

struct fcCaliPara
{
  int tabNum;
	int maxStep;
	int maxDuty;

	int* dutyTab; //-1 means no flash
	int* stepTab;
};

struct fcExpPara
{
	int exp;
	int afe;
	int isp;
	int step;
	int duty;
	int isFlash;
};

//======================================
//  Function
//======================================
/*
int flcMdkStart()
{
  LogInfo("flcMdkStart+");
  if (MDK_Open() == MFALSE)
  {
      LogErr("flcMdkStart-MDK_Open");
      return -1;
  }
  if (MDK_Init() == MFALSE)
  {
      LogErr("flcMdkStart-MDK_Init");
      return -1;
  }
  LogInfo("flcMdkStart-");
  return 0;
}

int flcMdkEnd()
{
  LogInfo("flcMdkEnd+");
  MDK_DeInit();
  MDK_Close();
  LogInfo("flcMdkEnd-");
  return 0;
}*/
static bool flcSendDataToACDK(MINT32   FeatureID,
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

    int ret;
    ret = (MDK_IOControl(FeatureID, &rAcdkFeatureInfo));

    if(ret==MFALSE)
      return -1;
    else
      return 0;
}
static VOID flcPrvCb(VOID *a_pParam)
{
}
int flcPreviewStart()
{
    ACDK_CCT_CAMERA_PREVIEW_STRUCT rCCTPreviewConfig;
    rCCTPreviewConfig.fpPrvCB = flcPrvCb;
    rCCTPreviewConfig.u2PreviewWidth = 320;
    rCCTPreviewConfig.u2PreviewHeight = 240;

    MUINT32 u4RetLen = 0;
    int ret;
    ret = flcSendDataToACDK (ACDK_CMD_PREVIEW_START/*ACDK_CCT_OP_PREVIEW_LCD_START*/, (UINT8 *)&rCCTPreviewConfig,
                              sizeof(ACDK_CCT_CAMERA_PREVIEW_STRUCT),
                              NULL,
                              0,
                              &u4RetLen);
    return ret;

}//copy from alps\mediatek\platform\mt6582\external\meta\cameratool\test\ccapTest\AcdkCCAPTest.cpp
int flcPreviewStop()
{
    MUINT32 u4RetLen = 0;
    int ret = flcSendDataToACDK(ACDK_CMD_PREVIEW_STOP/*ACDK_CCT_OP_PREVIEW_LCD_STOP*/, NULL, 0, NULL, 0, &u4RetLen);
    return ret;
}//copy from alps\mediatek\platform\mt6582\external\meta\cameratool\test\ccapTest\AcdkCCAPTest.cpp

AcdkBase* g_baseObj;
MUINT32 g_aeInfo;
MUINT32 g_awbInfo;
MUINT32 g_afInfo;
int g_bIni=0;
void fix3a()
{
	LogInfo("fix3a");
	MINT32 MFPos = 0;
	MUINT32 u4RetLen = 0;
	// Disable 3A
  g_baseObj->sendcommand(ACDK_CCT_OP_AE_DISABLE, NULL, 0, NULL, 0, &u4RetLen);
  g_baseObj->sendcommand(ACDK_CCT_V2_OP_MF_OPERATION, (MUINT8 *)&MFPos, sizeof(MFPos), NULL, 0, &u4RetLen); // MF
  g_baseObj->sendcommand(ACDK_CCT_OP_AF_DISABLE, NULL, 0, NULL, 0, &u4RetLen);
  g_baseObj->sendcommand(ACDK_CCT_V2_OP_AWB_DISABLE_AUTO_RUN, NULL, 0, NULL, 0, &u4RetLen);
  // Lock exposure setting
  g_baseObj->sendcommand(ACDK_CCT_OP_AE_LOCK_EXPOSURE_SETTING, NULL, 0, NULL, 0, &u4RetLen);
}
void FC_init(AcdkBase* p)
{
	//mkdir(DEBUG_PATH, S_IRWXU | S_IRWXG | S_IRWXO);
	g_baseObj=p;
	if(g_bIni==0)
	{
		g_bIni=1;
		flcPreviewStart();
	}
	MUINT32 u4RetLen = 0;
	// Backup 3A enable info
  MUINT32 u4AEInfo = 0;
  MUINT32 u4AFInfo = 0;
  MUINT32 u4AWBInfo = 0;
  g_baseObj->sendcommand(ACDK_CCT_OP_AE_GET_ENABLE_INFO, NULL, 0, (MUINT8 *)&u4AEInfo, sizeof(MUINT32), &u4RetLen);
  g_baseObj->sendcommand(ACDK_CCT_OP_AF_GET_ENABLE_INFO, NULL, 0, (MUINT8 *)&u4AFInfo, sizeof(MUINT32), &u4RetLen);
  g_baseObj->sendcommand(ACDK_CCT_V2_OP_AWB_GET_AUTO_RUN_INFO, NULL, 0, (MUINT8 *)&u4AWBInfo, sizeof(MUINT32), &u4RetLen);
  //---------------------
  g_aeInfo=u4AEInfo;
  g_awbInfo=u4AWBInfo;
  g_afInfo=u4AFInfo;
}
void FC_uninit()
{
	MUINT32 u4RetLen = 0;
	// Unlock exposure setting
	g_baseObj->sendcommand(ACDK_CCT_OP_AE_UNLOCK_EXPOSURE_SETTING, NULL, 0, NULL, 0, &u4RetLen);
	// Restore 3A
	if (g_aeInfo)
	{
	   g_baseObj->sendcommand(ACDK_CCT_OP_AE_ENABLE, NULL, 0, NULL, 0, &u4RetLen);
	}
	if (g_afInfo)
	{
	   g_baseObj->sendcommand(ACDK_CCT_OP_AF_ENABLE, NULL, 0, NULL, 0, &u4RetLen);
	}
	if (g_awbInfo)
	{
	   g_baseObj->sendcommand(ACDK_CCT_V2_OP_AWB_ENABLE_AUTO_RUN, NULL, 0, NULL, 0, &u4RetLen);
	}
	if(g_bIni==1)
	{
    usleep(3*1000000);
	  flcPreviewStop();
  }
}

static float g_rggb[4];
static volatile int g_bCapDone2=0;
static int g_prjDirN=0;
static int g_fileN=0;
static VOID capCallback(VOID *a_pParam)
{
    LogInfo("capCallback+");
    mtkRaw rawImg;
    ImageBufInfo *pImgBufInfo = (ImageBufInfo *)a_pParam;
    {
        LogInfo("Buffer Type:%d\n",  pImgBufInfo->eImgType);
        LogInfo("Size:%d\n", pImgBufInfo->RAWImgBufInfo.imgSize);
        LogInfo("Width:%d\n", pImgBufInfo->RAWImgBufInfo.imgWidth);
        LogInfo("Height:%d\n", pImgBufInfo->RAWImgBufInfo.imgHeight);
        LogInfo("BitDepth:%d\n", pImgBufInfo->RAWImgBufInfo.bitDepth);
        LogInfo("Bayer Start:%d\n", pImgBufInfo->RAWImgBufInfo.eColorOrder);
        int imgW;
        int imgH;
        int clrOrder;
		    imgW = pImgBufInfo->RAWImgBufInfo.imgWidth;
        imgH = pImgBufInfo->RAWImgBufInfo.imgHeight;
        clrOrder = pImgBufInfo->RAWImgBufInfo.eColorOrder;
        rawImg.createBuf(imgW, imgH, 8);
        rawImg.colorOrder = clrOrder;
        memcpy(rawImg.raw, pImgBufInfo->RAWImgBufInfo.bufAddr, imgW*imgH);
        rawImg.mean4Center(g_rggb);
    }
#if 1
    {
        int imgW;
        int imgH;
        FILE* fp;
        static int vv=0;
        vv++;
        char s[100];
        imgW = pImgBufInfo->RAWImgBufInfo.imgWidth;
        imgH = pImgBufInfo->RAWImgBufInfo.imgHeight;
        //write
        mkdir("/sdcard/flashdata/",S_IRWXU | S_IRWXG | S_IRWXO);
        sprintf(s,"/sdcard/flashdata/%03d/",g_prjDirN);
        mkdir(s,S_IRWXU | S_IRWXG | S_IRWXO);
   	    sprintf(s,"/sdcard/flashdata/%03d/cap_%03d.bmp", g_prjDirN, g_fileN);
   	    rawImg.toBmp(s);
        //sprintf(s,"/sdcard/flashdata/fa_%03d_%dx%d_%d_c%d.raw",vv,pImgBufInfo->RAWImgBufInfo.imgWidth,pImgBufInfo->RAWImgBufInfo.imgHeight,pImgBufInfo->RAWImgBufInfo.bitDepth
        //                                                         ,pImgBufInfo->RAWImgBufInfo.eColorOrder );
        //fp = fopen(s,"wb");
        //LogInfo("fp=%d",fp);
        //fwrite((char *)pImgBufInfo->RAWImgBufInfo.bufAddr, 1, imgW*imgH, fp);
        //fclose(fp);
    }
#endif
    g_bCapDone2 = MTRUE;
    LogInfo("capCallback-");
}
int takeRaw()
{
  LogInfo("flcTakeImage+");
  ACDK_CAPTURE_STRUCT_S/*ACDK_CCT_MULTI_SHOT_CAPTURE_STRUCT*/ rCCTMutiShotConfig;
  rCCTMutiShotConfig.eCameraMode = CAPTURE_MODE;
  rCCTMutiShotConfig.eOutputFormat = PROCESSED_RAW8_TYPE/*OUTPUT_PROCESSED_RAW8*/;
  //rCCTMutiShotConfig.u2JPEGEncWidth = 2560;
  //rCCTMutiShotConfig.u2JPEGEncHeight = 1920;
  rCCTMutiShotConfig.fpCapCB = capCallback;
  rCCTMutiShotConfig.u4CapCount = 1;
  g_bCapDone2 = 0;
  MUINT32 u4RetLen = 0;
  int err=0;
  int e;
  err = flcSendDataToACDK(ACDK_CMD_CAPTURE/*ACDK_CCT_OP_MULTI_SHOT_CAPTURE_EX*/, (UINT8 *)&rCCTMutiShotConfig,
                             sizeof(ACDK_CCT_MULTI_SHOT_CAPTURE_STRUCT),
                             NULL,
                             0,
                             &u4RetLen);

  LogInfo("flcTakeImage wait+");
  //wait JPEG Done;
  while (!g_bCapDone2)
  {
    usleep(3000);
  }
  LogInfo("flcTakeImage wait-");
  e = flcPreviewStart();
  if(err==0)
    err=e;
  LogInfo("flcTakeImage-");
  return err;
}
int flcTakeRawImage(fcExpPara* para, float* rggb)
{
  LogInfo("flcTakeRawImage line=%d",__LINE__);
	LogInfo("flcTakeRawImage exp,afe,isp,isFlash,duty,step: %d\t%d\t%d\t%d\t%d\t%d", para->exp, para->afe, para->isp, para->isFlash, para->duty, para->step);
	//set exp
	int i4Exp;
	fix3a();
	i4Exp = para->exp;
  MUINT32 u4RetLen = 0;
	int a_i4Gain=1024;
	int isp=para->isp;
	g_baseObj->sendcommand(ACDK_CCT_OP_AE_SET_SENSOR_EXP_TIME, (MUINT8 *)&i4Exp, sizeof(MINT32), NULL, 0, &u4RetLen);
	if(u4RetLen!=0)
	  LogErr("set exp");
	g_baseObj->sendcommand(ACDK_CCT_OP_AE_SET_SENSOR_GAIN, (MUINT8 *)&a_i4Gain, sizeof(MINT32), NULL, 0, &u4RetLen);
	if(u4RetLen!=0)
	  LogErr("set sensor");
	g_baseObj->sendcommand(ACDK_CCT_OP_AE_SET_ISP_RAW_GAIN, (MUINT8 *)&isp, sizeof(MINT32), NULL, 0, &u4RetLen);
	if(u4RetLen!=0)
	  LogErr("set isp");
	 //take raw
	int err;
  err = takeRaw();
  rggb[0]=g_rggb[0];
  rggb[1]=g_rggb[1];
  rggb[2]=g_rggb[2];
  rggb[3]=g_rggb[3];
  LogInfo("flcTakeRawImage meanx100=%d, %d, %d, %d",
          (int)(rggb[0]*100),(int)(rggb[1]*100),(int)(rggb[2]*100),(int)(rggb[3]*100));
  return err;
}

int __flcCalibration_test(void* in, int inSize, void* out, int outSize, MUINT32* realOutSize)
{
  int e;
  int err=0;
  //e = flcMdkStart();
  //LogInfo("flcMdkStart %d",e);
  //err = e;
  e = flcPreviewStart();
  LogInfo("flcPreviewStart %d",e);
  if(err==0)
    err = e;

  usleep(5*1000000);

  int i;
  for(i=0;i<5;i++)
  {
    takeRaw();
    usleep(2*1000000);
  }

  usleep(2*1000000);

  e = flcPreviewStop();
  LogInfo("flcPreviewStop %d",e);
  if(err==0)
    err = e;

  //e = flcMdkEnd();
  //LogInfo("flcMdkEnd %d",e);
  //if(err==0)
  //  err=e;
  return 0;
}

void CaliCoreExt(fcCaliPara* caliPara, fcRgb* rgbRet, fcExpPara* expRet)
{
  int i;
	int iniExp=30000;
	int iniIsp=1024;
	int iniAfe=1024;
	float tar=185;

  float rggb[4];
	fcExpPara para;
	para.exp=iniExp;
	para.afe=iniAfe;
	para.isp=iniIsp;
	para.isFlash=1;
	para.step=caliPara->maxStep;
	para.duty=caliPara->maxDuty;
  //AE
	for(i=0;i<8;i++)
	{
		flcTakeRawImage(&para, rggb);
		float y;
		y = rggb[1];
		usleep(2*1000000); //2s
		if(i==7)
			break;
		if(y>240)
		  para.exp/=4;
		else if(y>220)
		  para.exp/=2;
		else if(y<150)
		{
		  float a;
		  a = tar/y;
			para.exp*=a;
			if(para.exp>60000)
			{
			  a=para.exp/60000;
			  para.exp=60000;
			  para.isp*=a;
			}
		}
		else
		  break;
	}
	expRet->exp = para.exp;
	expRet->afe = para.afe;
	expRet->isp = para.isp;

	for(i=0;i<caliPara->tabNum;i++)
	{

		LogInfo("calibrate line=%d duty=%d %d",__LINE__, i,caliPara->maxDuty);
		int coolT;

		LogInfo("CaliCoreExt line=%d",__LINE__);
		if(caliPara->dutyTab[i]>=0)
		{
			LogInfo("CaliCoreExt line=%d",__LINE__);
			para.isFlash=1;
			LogInfo("CaliCoreExt line=%d",__LINE__);
			para.duty=caliPara->dutyTab[i];
			LogInfo("CaliCoreExt line=%d",__LINE__);
			para.step=caliPara->stepTab[i];
			LogInfo("CaliCoreExt line=%d",__LINE__);
		}
		else
		{
			para.duty=0;
			para.step=0;
			para.isFlash=0;
		}
		LogInfo("CaliCoreExt line=%d",__LINE__);
		float rggb[4];
		flcTakeRawImage(&para, rggb);
		rgbRet[i].r = rggb[0];
		rgbRet[i].g = rggb[1];
		rgbRet[i].b = rggb[3];
	}

//----------------------
#if 1 //write to sd
	FILE* fp;
	fp = fopen("/sdcard/flash_cali_debug1.txt", "wt");
	fprintf(fp,"duty\tg\tb\t\n");
	for(i=0;i<=caliPara->tabNum;i++)
	{
		fprintf(fp,"%f\t%f\t%f\n", rgbRet[i].r, rgbRet[i].g, rgbRet[i].b);
	}
	fclose(fp);
#endif
//----------------------
}
int AcdkCalibration::flashCalibration(void* in, int inSize, void* out, int outSize, MUINT32* realOutSize)
{
  //return __flcCalibration_test(in, inSize, out, outSize, realOutSize);
LogInfo("flashCalibration line=%d",__LINE__);
	int i;
	int j;
	FC_init(m_pAcdkBaseObj);
	FLASH_PROJECT_PARA prjPara;
	prjPara =  cust_getFlashProjectPara(0,0);

LogInfo("flashCalibration line=%d",__LINE__);

	fcCaliPara caliPr;
	caliPr.maxStep = prjPara.stepNum-1;
	caliPr.maxDuty = prjPara.dutyNum-1;

LogInfo("flashCalibration line=%d",__LINE__);

	int rexp;
	int rafe;
	int risp;
	int* dutyTab;
	int* stepTab;
	float tabBase;
	fcRgb* rgbTab;
	fcExpPara expRet;
	dutyTab = new int[200];
	stepTab = new int[200];
	rgbTab = new fcRgb[200];

LogInfo("flashCalibration line=%d",__LINE__);

	short* engTab;
	short* rgTab;
	short* bgTab;
	engTab = new short[256];
	rgTab = new short[256];
	bgTab = new short[256];

LogInfo("flashCalibration line=%d",__LINE__);
	//if(prjPara.coolTimeOutPara.tabMode==ENUM_FLASH_ENG_INDEX_MODE)
	int div[6];
	div[0] = 0;
	div[1] = 1;
	div[2] = (caliPr.maxDuty+1)/4;;
	div[3] = 2*div[2];
	div[4] = 3*div[2];
	div[5] = caliPr.maxDuty;

	if(prjPara.stepNum==1) //external
	{
		for(i=0;i<prjPara.dutyNum+2;i++)
		{
			dutyTab[i]=i-1;
			stepTab[i]=0;
		}
		dutyTab[prjPara.dutyNum+1]=-1;


		caliPr.dutyTab = dutyTab;
		caliPr.stepTab = stepTab;
	}
	else
	{
LogInfo("flashCalibration line=%d",__LINE__);

		caliPr.tabNum = prjPara.stepNum*6+2;
		caliPr.dutyTab = dutyTab;
		caliPr.stepTab = stepTab;
		dutyTab[0]=-1;
		stepTab[0]=0;
		for(i=0;i<prjPara.stepNum;i++)
		{
			int sh=1;
			for(j=0;j<6;j++)
			{
				dutyTab[6*i+j+sh]=div[j];
				stepTab[6*i+j+sh]=i;
			}
		}
		dutyTab[prjPara.stepNum*6+1]=-1;
		stepTab[prjPara.stepNum*6+1]=0;
	}

LogInfo("flashCalibration line=%d",__LINE__);
	CaliCoreExt(&caliPr, rgbTab, &expRet);

LogInfo("flashCalibration line=%d",__LINE__);
	if(prjPara.stepNum==1) //external
	{
		for(i=0;i<prjPara.dutyNum;i++)
		{
			engTab[i]=(int)((rgbTab[i+1].g-rgbTab[0].g)*128);
			rgTab[i]=1024*(rgbTab[i+1].r-rgbTab[0].r)/(rgbTab[i+1].g-rgbTab[0].g);
			bgTab[i]=1024*(rgbTab[i+1].b-rgbTab[0].b)/(rgbTab[i+1].g-rgbTab[0].g);
		}
		//FlashMgr::getInstance()->cctSetEngTabWithBackup(expRet.exp, expRet.afe, expRet.isp, engTab, rgTab, bgTab); //YosenFlash
	}
	else
	{
LogInfo("flashCalibration line=%d",__LINE__);
		float r0;
		float g0;
		float b0;
		r0= rgbTab[0].r;
		g0= rgbTab[0].g;
		b0= rgbTab[0].b;
		for(j=0;j<prjPara.stepNum;j++)
		{
			float r;
			float g;
			float b;
#if 0
			float x1;
			float x2;
			for(i=0;i<prjPara.dutyNum;i++)
			{
				for(k=1;k<6;k++)
				{
					if(i<=div[k])
					{
						x1 = div[k-1];
						x2 = div[k];
						r=flcUtil::interp((float)x1, (float)rgbTab[j*6+k].r, (float)x2, (float)rgbTab[j*6+k+1].r-r0, (float)i);
						g=flcUtil::interp((float)x1, (float)rgbTab[j*6+k].g, (float)x2, (float)rgbTab[j*6+k+1].g-g0, (float)i);
						b=flcUtil::interp((float)x1, (float)rgbTab[j*6+k].b, (float)x2, (float)rgbTab[j*6+k+1].b-b0, (float)i);
						break;
					}
				}
				engTab[j*prjPara.dutyNum+i]=g*128;
				rgTab[j*prjPara.dutyNum+i]=r/g*1024;
				bgTab[j*prjPara.dutyNum+i]=b/g*1024;
			}
#else
			for(i=0;i<prjPara.dutyNum;i++)
			{
LogInfo("flashCalibration test1= %d, %5.3f %5.3f %5.3f",prjPara.dutyNum-1, rgbTab[j*6+5+1].r, rgbTab[j*6+5+1].g, rgbTab[j*6+5+1].b);
				r=flcUtil::interp((float)-1, (float)0, (float)prjPara.dutyNum-1, (float)rgbTab[j*6+5+1].r-r0, (float)i);
				g=flcUtil::interp((float)-1, (float)0, (float)prjPara.dutyNum-1, (float)rgbTab[j*6+5+1].g-g0, (float)i);
				b=flcUtil::interp((float)-1, (float)0, (float)prjPara.dutyNum-1, (float)rgbTab[j*6+5+1].b-b0, (float)i);
LogInfo("flashCalibration test2 rgb = %5.3f %5.3f %5.3f-- %5.3f %5.3f %5.3f ", r, g, b, r0, g0, b0);
				engTab[j*prjPara.dutyNum+i]=g*128;
				rgTab[j*prjPara.dutyNum+i]=r/g*1024;
				bgTab[j*prjPara.dutyNum+i]=b/g*1024;
			}
#endif
		}
		//FlashMgr::getInstance()->cctSetEngTabWithBackup(expRet.exp, expRet.afe, expRet.isp, engTab, rgTab, bgTab); //YosenFlash
	}
LogInfo("flashCalibration line=%d",__LINE__);
	delete []engTab;
	delete []rgTab;
	delete []bgTab;
	delete []rgbTab;
	delete []stepTab;
	delete []dutyTab;
	FC_uninit();

	*realOutSize=1024;
	int* pN;
	pN = (int*)out;
	for(i=0;i<256;i++)
  {
    pN[i]=i;
  }
	return 0;

}