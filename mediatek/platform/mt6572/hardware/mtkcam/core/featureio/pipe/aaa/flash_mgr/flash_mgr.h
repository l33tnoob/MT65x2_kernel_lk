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
#ifndef __FLASH_MGR_H__
#define __FLASH_MGR_H__

#ifdef WIN32
#else
#include <flash_feature.h>
#include <flash_param.h>
#include "FlashAlg.h"
#endif


#ifdef WIN32

typedef struct
{
    void* staBuf;

}FlashExePara;

typedef struct
{
    int nextIsFlash;
    int nextExpTime;
    int nextAfeGain;
    int nextIspGain;
    int isEnd;
	int nextDuty;
	int nextStep;
	int isCurFlashOn;

}FlashExeRep;

#else

typedef struct
{
    void* staBuf;

}FlashExePara;

typedef struct
{
    int nextIsFlash;
    int nextExpTime;
    int nextAfeGain;
    int nextIspGain;
    int nextDuty;
	int nextStep;
    int isEnd;
    int isCurFlashOn;

}FlashExeRep;

#endif


#ifdef WIN32
///////////////////////
void InitFlashProfile(FlashAlgStrobeProfile* pf);
void ReadAndAllocatFlashProfile(const char* fname, FlashAlgStrobeProfile* pf);
void FreeFlashProfile(FlashAlgStrobeProfile* pf);
void getAEExpParaWin(FlashAlgExpPara* p);
void setAEExpParaWin(FlashAlgExpPara* p);
void getHWExpParaWin(FlashAlgExpPara* p);
void setHWExpParaWin(FlashAlgExpPara* p);

void get3AStaWin(FlashAlgStaData* a3sta);
void set3AStaWin(FlashAlgStaData* a3sta);
///////////////////////


void get3ASta(FlashAlgStaData* a3sta);
void getAEExpPara(FlashAlgExpPara* p);
#endif


typedef struct
{
	//int version;
	int sceneMode;
	//int capIsFlash;
	int capIso;
	int capAfeGain;
	int capIspGain;
	int capExp;
	int capDuty;
	int capStep;
	int err1;
	int err2;
	int err3;
	int errTime1;
	int errTime2;
	int errTime3;

	int vBat;
	int isoIncMode;
	int isoIncValue;
	int pfI;
	int mfIMin;
	int mfIMax;
	int pmfIpeak;
	int torchIPeak;
	int torchI;

	int startCoolingTime;
	int startTime;
	int endTime;
	int preFireStartTime;
	int preFireEndTime;
	int coolingTime;  //previous over-heat fire to next over-heat fire
	int estPf2MFTime; //preflash start to mainflash fire time
	int delayTime;

	int thisFireStartTime;
	int thisFireEndTime;
	float coolingTM;
	int thisTimeOutTime;


}FlashMgrDebug;



typedef enum
{
	FL_ERR_FlashModeNotSupport=-100,
	FL_ERR_AFLampModeNotSupport=-100,

	FL_ERR_SetLevelFail=-101,


	FL_ERR_CCT_INPUT_SIZE_WRONG = -10001,
	FL_ERR_CCT_OUTPUT_SIZE_WRONG = -10002,
	FL_ERR_CCT_FILE_NOT_EXIST = -10003,



}FlashMgrEnum;

class FlashMgr
{
public:
	//---------------------------
	//cct hw test tool
	int cctFlashLightTest(void* duty_duration);
	int cctGetFlashInfo(void* in, int inSize, void* out, int outSize, MUINT32* realOutSize);





//	int cctSetEngTab(int exp, int afe, int isp, short* engTab, float engTabBase);
	int cctSetEngTabWithBackup(int exp, int afe, int isp, short* engTab, short* rgTab, short* bgTab);


	//control
	int cctFlashEnable(int en);

	void cctInit();
	void cctUninit();





	//tuning
	//ACDK_CCT_OP_STROBE_READ_NVRAM,	//5,
	int cctReadNvram(void* in, int inSize, void* out, int outSize, MUINT32* realOutSize);
	//ACDK_CCT_OP_STROBE_WRITE_NVRAM,	//6
	int cctWriteNvram(void* in, int inSize, void* out, int outSize, MUINT32* realOutSize);
	//ACDK_CCT_OP_STROBE_READ_DEFAULT_NVRAM,	//7
	int cctReadDefaultNvram(void* in, int inSize, void* out, int outSize, MUINT32* realOutSize);
	//ACDK_CCT_OP_STROBE_SET_PARAM,	//8
	int cctSetParam(void* in, int inSize, void* out, int outSize, MUINT32* realOutSize);
	//ACDK_CCT_OP_STROBE_GET_PARAM,	//9
	int cctGetParam(void* in, int inSize, void* out, int outSize, MUINT32* realOutSize);
	//ACDK_CCT_OP_STROBE_GET_NVDATA, 10
	int cctGetNvdata(void* in, int inSize, void* out, int outSize, MUINT32* realOutSize);
	//ACDK_CCT_OP_STROBE_SET_NVDATA, 11
	int cctSetNvdata(void* in, int inSize, void* out, int outSize, MUINT32* realOutSize);
	//ACDK_CCT_OP_STROBE_GET_ENG_Y,	//12,
	int cctGetEngY(void* in, int inSize, void* out, int outSize, MUINT32* realOutSize);
	//ACDK_CCT_OP_STROBE_SET_ENG_Y,	//13
	int cctSetEngY(void* in, int inSize, void* out, int outSize, MUINT32* realOutSize);
	//ACDK_CCT_OP_STROBE_GET_ENG_RG,	//14
	int cctGetEngRg(void* in, int inSize, void* out, int outSize, MUINT32* realOutSize);
	//ACDK_CCT_OP_STROBE_SET_ENG_RG,	//15
	int cctSetEngRg(void* in, int inSize, void* out, int outSize, MUINT32* realOutSize);
	//ACDK_CCT_OP_STROBE_GET_ENG_BG,	//16
	int cctGetEngBg(void* in, int inSize, void* out, int outSize, MUINT32* realOutSize);
	//ACDK_CCT_OP_STROBE_SET_ENG_BG,	//17
	int cctSetEngBg(void* in, int inSize, void* out, int outSize, MUINT32* realOutSize);
	//ACDK_CCT_OP_STROBE_NVDATA_TO_FILE,	//18
	int cctNvdataToFile(void* in, int inSize, void* out, int outSize, MUINT32* realOutSize);
	//ACDK_CCT_OP_STROBE_FILE_TO_NVDATA,	//19
	int cctFileToNvdata(void* in, int inSize, void* out, int outSize, MUINT32* realOutSize);

















	//current not support
	int cctCheckPara(); //return status
	void cctGetCheckParaString(int* packetNO, int* isEnd, int* bufLength, unsigned char* buf); //buf: max 1024
	//---------------------------


	void cctPreflashTest(FlashExePara* para, FlashExeRep* rep);
	void cctPreflashEnd();




	void cctSetCapDutyStep(int isEn, int duty, int step);
	void cctGetLastFireTime(int* startTime, int* endTime);

public:
	//---------------------------
	//engineer mode
	int egnSetParam(int id, int v, int ext1);
	int egnSetPfIndex(int duty, int step);
	int egnSetMfIndex(int duty, int step);






	//---------------------------
public:


    FlashMgr();
    ~FlashMgr();
    static FlashMgr* getInstance();
    int init(int senorID);
    int uninit();
    int isNeedWaitCooling(int* ms);
    int start();
    int run(FlashExePara* para, FlashExeRep* rep);
    int end();


	int setEvComp(int ind, float ev_step);

	int getAfLampMode();
	int setAfLampMode(int mode);
	int getFlashMode();
    int setShotMode(int mode);
    int isBurstShotMode();
    int setFlashMode(int mode);
    int setCamMode(int mode);
    int getCamMode();
    int isFlashOnCapture();
    int setDigZoom(int digx100);

    int capCheckAndFireFlash_Start();
    int capCheckAndFireFlash_End();
//    int capCloseFlash();
    int turnOffFlashDevice();

	int turnOffPrecapFlash();

    int TestFlash(int frmCnt, void* a3Buf);

    int getDebugInfo(FLASH_DEBUG_INFO_T* p);

    void setFlashOnOff(int en);
    void setAFLampOnOff(int en);
    int isAFLampOn();

    int isNeedFiringFlash();

public:
	//void hw_setFlashProfile(FlashAlg* pStrobeAlg);
	void hw_convert3ASta(FlashAlgStaData* staData, void* staBuf);

	void hw_setFlashProfile(FlashAlg* pStrobeAlg, FLASH_PROJECT_PARA* pPrjPara, NVRAM_CAMERA_STROBE_STRUCT* pNvram);
	void hw_setPreference(FlashAlg* pStrobeAlg, FLASH_PROJECT_PARA* pPrjPara);
	void hw_setCapPline(FLASH_PROJECT_PARA* pPrjPara, FlashAlg* pStrobeAlg);



	int writeNvramMain(void* newNvramData);


	int writeNvram(int sensorType, void* newNvramData); //newNvramData=0: current Nvram
	int loadNvram();
	int forceLoadNvram();
	int setNvram(void* newNvramData);
	int loadDefaultNvram();
	int getNvram(void* NvramData);


	int doPfOneFrame(FlashExePara* para, FlashExeRep* rep);
	int doMfOneFrame(void* aa_adr);
	int endPrecapture();
	int changeBurstEngLevel();

	//flash mgr keep memory of nvram data to operate







private:

	inline void setDebugTag(FLASH_DEBUG_INFO_T &a_rFlashInfo, MINT32 a_i4ID, MINT32 a_i4Value)
    {
        a_rFlashInfo.Tag[a_i4ID].u4FieldID = AAATAG(AAA_DEBUG_FLASH_MODULE_ID, a_i4ID, 0);
        a_rFlashInfo.Tag[a_i4ID].u4FieldValue = a_i4Value;
    }


	void addErr(int err);
	float m_evComp;
	int m_shotMode;
	int m_camMode;
	int m_flashMode;
	int m_afLampMode;
	int m_flashOnPrecapture;
	float m_digRatio;

	int m_sensorType;
	FlashMgrDebug m_db;
	NVRAM_CAMERA_STROBE_STRUCT* m_pNvram;

	int m_pfFrameCount;

	int m_thisFlashDuty;
	int m_thisFlashStep;
	int m_thisIsFlashEn;

	int m_cct_isUserDutyStep;
	int m_cct_capStep;
	int m_cct_capDuty;


	int m_iteration;

	int m_isCapFlashEndTimeValid;
	int m_isAFLampOn;



};


#endif  //#define __FLASH_MGR_H__