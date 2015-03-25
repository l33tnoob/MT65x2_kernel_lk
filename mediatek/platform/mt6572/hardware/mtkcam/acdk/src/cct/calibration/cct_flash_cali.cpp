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
#include "mtkfb.h"
}

#include "AcdkLog.h"
#include "AcdkErrCode.h"
#include "AcdkCommon.h"

#include "cct_ctrl.h"
#include "cct_calibration.h"

#include "cct_feature.h"
#include "isp_drv.h"
#include "cct_if.h"
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

/*
#include "Aaa_sensor_mgr.h"
*/
#include <mtkcam/hal/sensor_hal.h>
#include <nvram_drv_mgr.h>
#include <ae_tuning_custom.h>
#include <isp_mgr.h>
#include <isp_tuning.h>
#include <isp_tuning_mgr.h>

#include <aaa_sensor_mgr.h>
#include "flash_mgr.h"


//#include "isp_hal.h"

using namespace NSACDK;



#define DEBUG_PATH "/sdcard/flashdata/"


#define LogInfo(fmt, arg...) XLOGD(fmt, ##arg)
#define LogErr(fmt, arg...) XLOGD("FlashErr: %5d: "fmt, __LINE__, ##arg)
#define LogWarning(fmt, arg...) XLOGD("FlashWarning: %5d: "fmt, __LINE__, ##arg)
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

struct fcCaliPara
{
	int maxStep;
	int maxDuty;

	int tabNum;
	int* dutyTab; //-1 means no flash
	int* stepTab;

	int coolMode;
	int coolNum;
	int coolInd[20];
	float coolTM[20];


};

typedef struct
{
	float r;
	float g;
	float b;
}fcRgb;


struct fcExpPara
{
	int exp;
	int afe;
	int isp;
	int step;
	int duty;
	int isFlash;
};

namespace FlashCali
{


template <class T>  T interp(T x1, T y1, T x2, T y2, T x)
{
	return y1+ (y2-y1)*(x-x1)/(x2-x1);
}


template <class T> T calYFromXYTab(int n, T* xNode, T* yNode, T x)
{
	T y=yNode[0];
	int i;
	T xst;
	T yst;
	T xed;
	T yed;
	xst=xNode[0];
	yst=yNode[0];
	if(x<xNode[0])
		x=xNode[0];
	else if(x>xNode[n-1])
		x=xNode[n-1];

	for(i=1;i<n;i++)
	{
		xed=xNode[i];
		yed=yNode[i];

		if(x<=xNode[i])
		{
			y=interp(xst, yst, xed, yed, x);
			break;
		}
		xst=xed;
		yst=yed;
	}
	if(x<=xNode[0])
	    y=yNode[0];
	else if(x>=xNode[n-1])
	    y=yNode[n-1];
	return y;
}

};


//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
static int getMs()
{
	//	max:
	//	2147483648 digit
	//	2147483.648 second
	//	35791.39413 min
	//	596.5232356 hour
	//	24.85513481 day
	int t;
	struct timeval tv;
	gettimeofday(&tv, NULL);
	t = (tv.tv_sec*1000 + (tv.tv_usec+500)/1000);
	return t;
}
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx


//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx




//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

static int arrayToBmp(const char* f, void* pData, int pixByte, int w, int h, int pxMax);
class mtkRaw
{
public:
	int w;
	int h;
	int depth;
	void* raw;
	int colorOrder;
	mtkRaw();
	virtual ~mtkRaw();
	void createBuf(int w, int h, int depth);
	float mean(int x, int y, int w, int h);
	int toBmp(const char* f);
	void mean4(int div_x, int div_y, int div_w, int div_h, float* rggb);
	void mean4Center(float* rggb);
};

void mtkRaw::mean4(int x, int y, int w, int h, float* rggb)
{
	float m00;
	float m01;
	float m10;
	float m11;
	m00=mean(x  ,y  ,w,h);
	m10=mean(x+1,y  ,w,h);
	m01=mean(x  ,y+1,w,h);
	m11=mean(x+1,y+1,w,h);
	if(colorOrder==0)
	{
		rggb[0]=m11;
		rggb[1]=m01;
		rggb[2]=m10;
		rggb[3]=m00;
	}
	else if(colorOrder==1)
	{
		rggb[0]=m01;
		rggb[1]=m11;
		rggb[2]=m00;
		rggb[3]=m10;
	}
	else if(colorOrder==2)
	{
		rggb[0]=m10;
		rggb[1]=m00;
		rggb[2]=m11;
		rggb[3]=m01;
	}
	else //if(colorOrder==3)
	{
		rggb[0]=m00;
		rggb[1]=m10;
		rggb[2]=m01;
		rggb[3]=m11;
	}
}
/*
void mtkRaw::mean4(int div_x, int div_y, int div_w, int div_h, float* rggb)
{
	rggb[0]=mean(div_x, div_y, div_w, div_h);
	rggb[1]=mean(div_x, div_y, div_w, div_h);
	rggb[2]=mean(div_x, div_y, div_w, div_h);
	rggb[3]=mean(div_x, div_y, div_w, div_h);
}*/
void mtkRaw::mean4Center(float* rggb)
{
	mean4(0.4*w, 0.4*h, 0.2*w, 0.2*h, rggb);
}
float mtkRaw::mean(int div_x, int div_y, int div_w, int div_h)
{
	int i;
	int j;
	int sum=0;
	int ind;
	int cnt=0;
	unsigned char* pRaw1;
	unsigned short* pRaw2;
	if(depth>8)
		pRaw2=(unsigned short*)raw;
	else
		pRaw1=(unsigned char*)raw;

	for(j=div_y;j<div_y+div_h;j+=2)
	for(i=div_x;i<div_x+div_w;i+=2)
	{
		ind=j*w+i;
		if(depth>8)
			sum+=pRaw2[ind];
		else
			sum+=pRaw1[ind];
		cnt++;
	}
	return (float)sum/cnt;
}
mtkRaw::mtkRaw()
{
	raw=0;
}
mtkRaw::~mtkRaw()
{
	if(raw!=0)
	{
		if(depth==8)
			delete [](unsigned char*)raw;
		else
			delete [](unsigned short*)raw;
	}
}
void mtkRaw::createBuf(int a_w, int a_h, int a_depth)
{
	w=a_w;
	h=a_h;
	depth=a_depth;
	if(raw!=0)
	{
		if(depth==8)
			delete [](unsigned char*)raw;
		else
			delete [](unsigned short*)raw;
	}
	if(depth!=8)
		raw = new unsigned short[w*h];
	else
		raw = new unsigned char[w*h];
}
int mtkRaw::toBmp(const char* f)
{
	int pxBytes;
	if(depth==8)
		pxBytes=1;
	else
		pxBytes=2;
	int i;
	int pxMax;
	pxMax = 1;
	for(i=0;i<depth;i++)
	{
		pxMax*=2;
	}
	pxMax-=1;
	return arrayToBmp(f, raw, pxBytes, w, h, pxMax);
}

static int arrayToBmp(const char* f, void* pData, int pixByte, int w, int h, int pxMax)
{
	int i;
	int j;
	FILE* fp;
	fp = fopen(f, "wb");
	int tmp;
	int Bytesofline;
	Bytesofline = (w+3)/4*4;
	//file header
	char ch[2]={'B','M'};
	fwrite(&ch,1,2,fp);
	tmp = 14+40+256*4+Bytesofline*h;
	fwrite(&tmp,4,1,fp);
	tmp=0;
	fwrite(&tmp,4,1,fp);
	tmp=14+40+256*4;
	fwrite(&tmp,4,1,fp);
	//info header
	tmp=40;
	fwrite(&tmp,4,1,fp);
	tmp=w;
	fwrite(&tmp,4,1,fp);
	tmp=h;
	fwrite(&tmp,4,1,fp);
	tmp=1;
	fwrite(&tmp,2,1,fp);
	tmp=8;
	fwrite(&tmp,2,1,fp);
	tmp=0;
	fwrite(&tmp,4,1,fp);
	tmp=Bytesofline*h; //image bytes (no header)
	fwrite(&tmp,4,1,fp);
	tmp=2834; //horizontal pixel per meter
	fwrite(&tmp,4,1,fp);
	tmp=2834;
	fwrite(&tmp,4,1,fp);
	tmp=0;
	fwrite(&tmp,4,1,fp);
	tmp=0;
	fwrite(&tmp,4,1,fp);
	for(i=0;i<256;i++)
	{
		tmp=i;
		fwrite(&tmp,1,1,fp);
		fwrite(&tmp,1,1,fp);
		fwrite(&tmp,1,1,fp);
		tmp=0;
		fwrite(&tmp,1,1,fp);
	}

	unsigned char* pImg;
	pImg = new unsigned char[Bytesofline*h];
	int indBmp;
	int indSrc;
	unsigned char* pSrc1=(unsigned char*)pData;
	unsigned short* pSrc2=(unsigned short*)pData;
	unsigned int* pSrc4=(unsigned int*)pData;
	for(j=0;j<h;j++)
	{
		for(i=0;i<w;i++)
		{
			indSrc=(h-j-1)*w+i;
			indBmp=j*Bytesofline+i;
			if(pixByte==1)
			{
				tmp = pSrc1[indSrc]*255/pxMax;
			}
			else if(pixByte==2)
			{
				tmp = pSrc2[indSrc]*255/pxMax;
			}
			else if(pixByte==4)
			{
				tmp = pSrc4[indSrc]*255/pxMax;
			}
			pImg[indBmp]=tmp;
		}
	}
	fwrite(pImg, 1, Bytesofline*h, fp);
	delete []pImg;

	fclose(fp);

	return 0;
}

//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//sem_t mutex22;
mtkRaw* pRawForCapture;
volatile int isCapEnd=0;
VOID captureCallBack8(VOID *a_pParam)
{
	LogInfo("captureCallBack8");

    ImageBufInfo *prImgBufInfo = (ImageBufInfo*)a_pParam;
    if (prImgBufInfo->eImgType == PURE_RAW8_TYPE)
    {
    	LogInfo("captureCallBack8 line=%d", __LINE__);
        int w;
        int h;
        w = prImgBufInfo->RAWImgBufInfo.imgWidth;
        h = prImgBufInfo->RAWImgBufInfo.imgWidth;
        pRawForCapture->w = w;
        pRawForCapture->h = h;
        pRawForCapture->colorOrder = prImgBufInfo->RAWImgBufInfo.eColorOrder;
        pRawForCapture->createBuf(w, h, 8);

        memcpy(pRawForCapture->raw, prImgBufInfo->RAWImgBufInfo.bufAddr, w*h);
    }
    LogInfo("captureCallBack8 line=%d", __LINE__);
   // sem_post(&mutex22);
    LogInfo("captureCallBack8 line=%d", __LINE__);
    isCapEnd=1;
}


int takeRawImage(mtkRaw* pRaw)
{
	LogInfo("takeRawImage line=%d", __LINE__);
	//sem_init(&mutex22, 0, 1);
	AcdkBase* g_pAcdkBaseObj = NSACDK::AcdkBase::createInstance();
   	g_pAcdkBaseObj->init();
   	pRawForCapture = pRaw;
	//g_pAcdkBaseObj->takePicture(PREVIEW_MODE, OUTPUT_PROCESSED_RAW8, captureCallBack8);
	//g_pAcdkBaseObj->takePicture(PREVIEW_MODE, PURE_RAW8_TYPE, captureCallBack8);

	LogInfo("takeRawImage line=%d", __LINE__);
	//sem_wait(&mutex22);

	LogInfo("takeRawImage line=%d", __LINE__);
	isCapEnd=0;
	g_pAcdkBaseObj->takePicture(PREVIEW_MODE, PROCESSED_RAW8_TYPE, captureCallBack8);
	usleep(30000);
	while(1)
	{
		if(isCapEnd==1)
			break;
	}


	//sem_wait(&mutex22);
	//sem_post(&mutex22);

	//PROCESSED_RAW8_TYPE
	g_pAcdkBaseObj->uninit();
	//sem_destroy(&mutex22);
	return 0;
}




/*
void setExpPara(AcdkCalibration* pAcdkCali, AcdkBase* pAcdkBase, ExpPara* p)
{
	MUINT32 u4RetLen = 0;
	pAcdkBase->sendcommand(ACDK_CCT_OP_AE_SET_SENSOR_EXP_TIME, (MUINT8 *)&p->exp, sizeof(MINT32), NULL, 0, &u4RetLen);
	pAcdkBase->sendcommand(ACDK_CCT_OP_AE_SET_SENSOR_GAIN, (MUINT8 *)&p->afe, sizeof(MINT32), NULL, 0, &u4RetLen);
	pAcdkBase->sendcommand(ACDK_CCT_OP_AE_SET_ISP_RAW_GAIN, (MUINT8 *)&p->isp, sizeof(MINT32), NULL, 0, &u4RetLen);
}*/








static VOID vPrvCb2(VOID *a_pParam)
{
}



AcdkBase* g_baseObj;
MUINT32 g_aeInfo;
MUINT32 g_awbInfo;
MUINT32 g_afInfo;
unsigned char* g_pRaw;
int g_rawWidth=-1;
int g_rawHeight=-1;
int g_clrOrder=-1;
int g_bCapDone=0;
StrobeDrv* g_pStrobe;
mtkRaw* g_rawImg;
int g_fireStartTimeMs;
int g_fireEndTimeMs;
float g_gMean;

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

	static int ini=1;
	if(ini==1)
	{
		ini=0;
		//------------------------
		//start preview
		MUINT32 u4RetLen22 = 0;
		ACDK_CCT_CAMERA_PREVIEW_STRUCT rCCTPreviewConfig;
	    rCCTPreviewConfig.fpPrvCB = vPrvCb2;
	    rCCTPreviewConfig.u2PreviewWidth = 320;
	    rCCTPreviewConfig.u2PreviewHeight = 240;
		g_baseObj->sendcommand(ACDK_CCT_OP_PREVIEW_LCD_START, (UINT8 *)&rCCTPreviewConfig, sizeof(ACDK_CCT_CAMERA_PREVIEW_STRUCT), (MUINT8 *)0, 0, &u4RetLen22);
		usleep(5000000);
		//------------------------
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

    g_pRaw=0;
    g_pStrobe = StrobeDrv::createInstance();
   	g_pStrobe->init(1);

   	g_rawImg = new mtkRaw;

}

void FC_uninit()
{
	g_pStrobe->uninit();

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

	if(g_pRaw!=0)
		delete []g_pRaw;
	delete g_rawImg;
}

/*
VOID vCapCbCap(VOID *a_pParam)
{
	 RGB565_TYPE          = 0x00000001,
    RGB888_TYPE          = 0x00000002,
    PURE_RAW8_TYPE       = 0x00000004,
    PURE_RAW10_TYPE      = 0x00000008,
    PROCESSED_RAW8_TYPE  = 0x00000010,
    PROCESSED_RAW10_TYPE = 0x00000020,
    JPEG_TYPE            = 0x00000040
}*/
VOID vCapCb22(VOID *a_pParam)
{
	LogInfo("vCapCb22");
    ImageBufInfo *prImgBufInfo = (ImageBufInfo*)a_pParam;

//    LogInfo("%d %d %d",
  //  prImgBufInfo->eImgType, prImgBufInfo->RAWImgBufInfo.imgWidth, prImgBufInfo->RAWImgBufInfo.imgSize );

  //  if ((prImgBufInfo->eImgType >= PURE_RAW8_TYPE) && (prImgBufInfo->eImgType <= PROCESSED_RAW10_TYPE))    //webber check later



    {
        ACDK_LOGD("[vCaptureCallBack] Raw Type = %d\n", prImgBufInfo->eImgType);

		g_rawWidth = prImgBufInfo->RAWImgBufInfo.imgWidth;
        g_rawHeight = prImgBufInfo->RAWImgBufInfo.imgHeight;
        MUINT32 u2ImgSize = prImgBufInfo->RAWImgBufInfo.imgSize;
        g_clrOrder = prImgBufInfo->RAWImgBufInfo.eColorOrder;

        if (g_pRaw != NULL)
        {
            delete []g_pRaw;
        }
        g_pRaw = new unsigned char [g_rawWidth*g_rawWidth*2];
        memcpy(g_pRaw, prImgBufInfo->RAWImgBufInfo.bufAddr, g_rawWidth*g_rawHeight);


        g_rawImg->createBuf(g_rawWidth, g_rawHeight, 8);
        g_rawImg->colorOrder = g_clrOrder;
        memcpy(g_rawImg->raw, prImgBufInfo->RAWImgBufInfo.bufAddr, g_rawWidth*g_rawHeight);


    }

    g_bCapDone = MTRUE;



}
MRESULT takePicture2(fcExpPara* para, fcRgb* rgbRet)//MUINT32 a_i4PreCap, eIMAGE_TYPE type)
{
	LogInfo("takePicture2 line=%d",__LINE__);
	LogInfo("exp,afe,isp,isFlash,duty,step: %d\t%d\t%d\t%d\t%d\t%d", para->exp, para->afe, para->exp, para->isFlash, para->duty, para->step);
	 int i4Exp;
	fix3a();

	i4Exp = para->exp;

	/*
	para.exp=(i+1)*1000;
	para.isp=1024;
	para.afe=1024;
	setExpPara(this, m_pAcdkBaseObj, &para);
	usleep(200*1000);
	*/
MUINT32 u4RetLen = 0;
	int a_i4Gain=1024;
	g_baseObj->sendcommand(ACDK_CCT_OP_AE_SET_SENSOR_EXP_TIME, (MUINT8 *)&i4Exp, sizeof(MINT32), NULL, 0, &u4RetLen);
	g_baseObj->sendcommand(ACDK_CCT_OP_AE_SET_SENSOR_GAIN, (MUINT8 *)&a_i4Gain, sizeof(MINT32), NULL, 0, &u4RetLen);


	int ta;
	int tb;

	ta = getMs();
	g_fireStartTimeMs = ta;



	FlashMgr::getInstance()->cctSetCapDutyStep(para->isFlash, para->duty, para->step);
	//FlashMgr::getInstance()->setFlashMode(1);

	/*

	g_pStrobe->setDuty(para->duty);
	g_pStrobe->setStep(para->step);

	if(para->isFlash==1)
	{
		g_pStrobe->setOnOff(0);
   		g_pStrobe->setOnOff(1);
   	}*/


	usleep(100000);
    MRESULT mrRet;

    g_bCapDone = 0;
    LogInfo("takePicture2 line=%d",__LINE__);
    //LogInfo("takePicture2 line=%d %d %d %d",__LINE__, (int)g_baseObj, a_i4PreCap, type);
    mrRet = g_baseObj->takePicture(0, (MUINT32)PROCESSED_RAW8_TYPE, vCapCb22);
    LogInfo("takePicture2 line=%d",__LINE__);

    while(!g_bCapDone)
    {
    	usleep(10000);
    	LogInfo("takePicture2 WAIT line=%d",__LINE__);
       // ACDK_LOGD(" Waiting Capture Done...\n");
    }
    //g_pStrobe->setOnOff(0);
    tb = getMs();

    int ts, te;
    FlashMgr::getInstance()->cctGetLastFireTime(&ts, &te);
    g_fireStartTimeMs = ts;
    g_fireEndTimeMs = te;

	float rggb[4];
    LogInfo("takePicture2 line=%d take time=%d",__LINE__, tb-ta);
    g_rawImg->mean4Center(rggb);
    LogInfo("takePicture2 mean %5.3f %5.3f %5.3f %5.3f",
    			rggb[0],
    			rggb[1],
    			rggb[2],
    			rggb[3]
    			);
   rgbRet->r=rggb[0];
   rgbRet->g=(rggb[1]+rggb[2])/2;
   rgbRet->b=rggb[3];



   	static int xx=0;
   	char* ss;

   	ss = new char[300];

   	mkdir("/sdcard/flashdata/",S_IRWXU | S_IRWXG | S_IRWXO);
   	sprintf(ss,"/sdcard/flashdata/cap_%03d.bmp", xx++);
   	g_rawImg->toBmp(ss);

   	delete []ss;


    g_baseObj->startPreview(NULL);

    usleep(1000000);    //delay for 1s



    return 0;

}

int fcExpImgMean(fcExpPara* p, fcRgb* rgbMean, int* flashTime, int* freeTime)
{
	takePicture2(p, rgbMean);
	int cur;
	cur = getMs();
	*freeTime = cur-g_fireEndTimeMs;
	*flashTime = g_fireEndTimeMs-g_fireStartTimeMs;
	return 0;
}

void CaliCoreExt(fcCaliPara* caliPara, fcRgb* rgbRet, fcExpPara* expRet)
{

	int i;
	int iniExp=30000;
	int iniIsp=1024;
	int iniAfe=1024;
	int fireTime;
	float m;
	fcRgb rgbMaxFlash;
	fcRgb rgbNoFlash;


	float tar=215;
	float* ftab;
	int* TM100;
	fcExpPara para;
	para.exp=30000;
	para.afe=1024;
	para.isp=1024;
	para.isFlash=1;
	para.step=caliPara->maxStep;
	para.duty=caliPara->maxDuty;
	ftab = new float[caliPara->maxDuty+1];
	TM100 = new int[caliPara->coolNum];

	for(i=0;i<caliPara->coolNum;i++)
		TM100[i]=caliPara->coolTM[i]*100;

	fcRgb rgb;
	int flashTime;
	int freeTime;
	int coolTm;
	int needCoolTime;

	int testCnt=0;
	for(i=0;i<8;i++)
	{
		coolTm = FlashCali::calYFromXYTab(caliPara->coolNum, caliPara->coolInd, TM100, caliPara->maxDuty );
		fcExpImgMean(&para, &rgb, &flashTime, &freeTime);

		if(ENUM_FLASH_ENG_INDEX_MODE==caliPara->coolMode)
			coolTm = FlashCali::calYFromXYTab(caliPara->coolNum, caliPara->coolInd, TM100, i );
		else
			coolTm = FlashCali::calYFromXYTab(caliPara->coolNum, caliPara->coolInd, TM100, 100);
		needCoolTime = coolTm*flashTime/100 - freeTime;
		if(needCoolTime>0)
			usleep(needCoolTime*1000);

		usleep(1000000);

		if(i==7)
			break;
		float a;
		if(rgb.g>=180 && rgb.g<=230)
		{
			testCnt++;
			if(testCnt>=2)
				break;
			a = tar/rgb.g;
			para.exp*=a;
		}
		else if(rgb.g>230)
		{
			para.exp/=3;
		}
		else
		{
			a = tar/rgb.g;
			para.exp*=a;
		}
	}
	rgbMaxFlash.r = rgb.r;
	rgbMaxFlash.g = rgb.g;
	rgbMaxFlash.b = rgb.b;

	if(para.exp>30000)
	{
		float a_isp;
		a_isp = (float)para.exp/30000;
		if(a_isp>15)
			a_isp=15;
		para.exp=para.exp/a_isp;
		para.isp*=a_isp;
	}

	expRet->exp = para.exp;
	expRet->afe = para.afe;
	expRet->isp = para.isp;

	para.isFlash=0;
	fcExpImgMean(&para, &rgbNoFlash, &flashTime, &freeTime);
	usleep(1000000);
	para.isFlash=1;


	//
	//	int tabNum;
	//int* dutyTab;
	//int* stepTab;
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
		fcExpImgMean(&para, &rgb, &flashTime, &freeTime);

		int percent;
		percent = (int)(100*(rgb.g-rgbNoFlash.g)/(rgbMaxFlash.g-rgbNoFlash.g));
		if(percent<0)
			percent=0;
		if(percent>100)
			percent=100;

		if(ENUM_FLASH_ENG_INDEX_MODE==caliPara->coolMode)
			coolT = FlashCali::calYFromXYTab(caliPara->coolNum, caliPara->coolInd, TM100, i );
		else
		{
			LogInfo("CaliCoreExt line=%d",__LINE__);


			coolT = FlashCali::calYFromXYTab(caliPara->coolNum, caliPara->coolInd, TM100, percent );



			float a;
			float b;
			float c;
			float d;
			c = rgbMaxFlash.g;
			b = rgb.g;
			a = rgbNoFlash.g;
			d = (100*(rgb.g-rgbNoFlash.g)/(rgbMaxFlash.g-rgbNoFlash.g));

			LogInfo("CaliCoreExt gm=%5.3f g=%5.3f g0=%5.3f r=%5.3f coolT=%d per=%d",c,b,a,d,coolT,percent);


		}

		int min_tm=0;
		if(percent>80)
			min_tm=400;
		if(percent>60)
			min_tm=300;
		if(percent>40)
			min_tm=200;
		if(percent>20)
			min_tm=100;

		if(coolT<min_tm)
			coolT=min_tm;





		needCoolTime = coolT*flashTime/100 - freeTime;
		if(needCoolTime>0)
			usleep(needCoolTime*1000);

		LogInfo("CaliCoreExt line=%d needCoolTime %d %d %d %d",__LINE__,
		needCoolTime, coolT, flashTime, freeTime);

		//char ss[200];
		//sprintf(ss,"/sdcard/flashdata/s_%03d.bmp", i);
		//g_rawImg->toBmp(ss);

		rgbRet[i].r = rgb.r;
		rgbRet[i].g = rgb.g;
		rgbRet[i].b = rgb.b;
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

	delete []TM100;
}

int AcdkCalibration::flashCalibration()
{
LogInfo("flashCalibration line=%d",__LINE__);
	int i;
	int j;
	FC_init(m_pAcdkBaseObj);
	FLASH_PROJECT_PARA prjPara;
	prjPara =  cust_getFlashProjectPara(0,0);

LogInfo("flashCalibration line=%d",__LINE__);

	fcCaliPara caliPr;
	caliPr.coolNum = prjPara.coolTimeOutPara.tabNum;
	int ia;
	for(ia=0;ia<caliPr.coolNum;ia++)
	{
		caliPr.coolInd[ia] = prjPara.coolTimeOutPara.tabId[ia];
		caliPr.coolTM[ia] = prjPara.coolTimeOutPara.coolingTM[ia];
	}
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
		caliPr.coolMode=ENUM_FLASH_ENG_INDEX_MODE;
		caliPr.tabNum = prjPara.dutyNum+2;
		caliPr.dutyTab = dutyTab;
		caliPr.stepTab = stepTab;
	}
	else
	{
LogInfo("flashCalibration line=%d",__LINE__);
		caliPr.coolMode=ENUM_FLASH_ENG_PERCENTAGE_MODE;
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
		FlashMgr::getInstance()->cctSetEngTabWithBackup(expRet.exp, expRet.afe, expRet.isp, engTab, rgTab, bgTab);
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
						r=FlashCali::interp((float)x1, (float)rgbTab[j*6+k].r, (float)x2, (float)rgbTab[j*6+k+1].r-r0, (float)i);
						g=FlashCali::interp((float)x1, (float)rgbTab[j*6+k].g, (float)x2, (float)rgbTab[j*6+k+1].g-g0, (float)i);
						b=FlashCali::interp((float)x1, (float)rgbTab[j*6+k].b, (float)x2, (float)rgbTab[j*6+k+1].b-b0, (float)i);
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
				r=FlashCali::interp((float)-1, (float)0, (float)prjPara.dutyNum-1, (float)rgbTab[j*6+5+1].r-r0, (float)i);
				g=FlashCali::interp((float)-1, (float)0, (float)prjPara.dutyNum-1, (float)rgbTab[j*6+5+1].g-g0, (float)i);
				b=FlashCali::interp((float)-1, (float)0, (float)prjPara.dutyNum-1, (float)rgbTab[j*6+5+1].b-b0, (float)i);
LogInfo("flashCalibration test2 rgb = %5.3f %5.3f %5.3f-- %5.3f %5.3f %5.3f ", r, g, b, r0, g0, b0);
				engTab[j*prjPara.dutyNum+i]=g*128;
				rgTab[j*prjPara.dutyNum+i]=r/g*1024;
				bgTab[j*prjPara.dutyNum+i]=b/g*1024;
			}
#endif
		}
		FlashMgr::getInstance()->cctSetEngTabWithBackup(expRet.exp, expRet.afe, expRet.isp, engTab, rgTab, bgTab);
	}
LogInfo("flashCalibration line=%d",__LINE__);
	delete []engTab;
	delete []rgTab;
	delete []bgTab;
	delete []rgbTab;
	delete []stepTab;
	delete []dutyTab;
	FC_uninit();
	return 0;
}