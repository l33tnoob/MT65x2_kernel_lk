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
#include <mtkcam/algorithm/lib3a/FlashAlg.h>
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

/** \brief When executing flash flow (currently, only doPfOneFrame), the input struct is FlashExePara, output struct is FlashExeRep.
*
*/
typedef struct
{
    void* staBuf;
    int flickerMode;

}FlashExePara;

/** \brief When executing flash flow (currently, only doPfOneFrame), the input struct is FlashExePara, output struct is FlashExeRep.
*
*/
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
    int flashAwbWeight;
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

/** \brief flash local variables (not for API's input/output).
*
*/
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


/** \brief the enum of error for flash public API
*
*/
typedef enum
{
	FL_ERR_FlashModeNotSupport=-100,
	FL_ERR_AFLampModeNotSupport=-100,

	FL_ERR_SetLevelFail=-101,


	FL_ERR_CCT_INPUT_SIZE_WRONG = -10001,
	FL_ERR_CCT_OUTPUT_SIZE_WRONG = -10002,
	FL_ERR_CCT_FILE_NOT_EXIST = -10003,



}FlashMgrEnum;


/** \brief FlashMgr: the class is mainly for handling flash/strobe related tasks.
*
*/
class FlashMgr
{
public:
	enum
	{
		e_NonePreview,
		e_VideoPreview,
		e_VideoRecording,
		e_CapturePreview,
		e_Capture,
	};

	enum
	{
		e_Flicker50,
		e_Flicker60,
		e_FlickerUnknown,
	};



	//======================
	//cct related function
	//======================

	/** \brief call for testing hardware compentnent. when calling the function, the flash will be turn on and turn off for a while.
	*
	*/
	int cctFlashLightTest(void* duty_duration);

	/** \brief get the flash current status (on/off).
	*
	*/
	int cctGetFlashInfo(void* in, int inSize, void* out, int outSize, MUINT32* realOutSize);


	/** \brief set engergy table to flash buf (then, can be written to NVram)
	*
	*/
	int cctSetEngTabWithBackup(int exp, int afe, int isp, short* engTab, short* rgTab, short* bgTab);


	/** \brief call for set flash enable/disable
	*
	*/
	int cctFlashEnable(int en);

	/** \brief when calling flash ratio calibration, the function should be called.
	*
	*/
	void cctInit();

	/** \brief when calling flash ratio calibration and end, the function should be called.
	*
	*/
	void cctUninit();





	//tuning
	//ACDK_CCT_OP_STROBE_READ_NVRAM,	//5,
	/** \brief read nvram to buf
	*
	*/
	int cctReadNvram(void* in, int inSize, void* out, int outSize, MUINT32* realOutSize);
	//ACDK_CCT_OP_STROBE_WRITE_NVRAM,	//6
	/** \brief write buf to nvram
	*
	*/
	int cctWriteNvram(void* in, int inSize, void* out, int outSize, MUINT32* realOutSize);
	//ACDK_CCT_OP_STROBE_READ_DEFAULT_NVRAM,	//7
	/** \brief read default nvram data to buf
	*
	*/
	int cctReadDefaultNvram(void* in, int inSize, void* out, int outSize, MUINT32* realOutSize);
	//ACDK_CCT_OP_STROBE_SET_PARAM,	//8
	int cctSetParam(void* in, int inSize, void* out, int outSize, MUINT32* realOutSize);
	//ACDK_CCT_OP_STROBE_GET_PARAM,	//9
	int cctGetParam(void* in, int inSize, void* out, int outSize, MUINT32* realOutSize);
	//ACDK_CCT_OP_STROBE_GET_NVDATA, 10
	/** \brief get buf data
	*
	*/
	int cctGetNvdata(void* in, int inSize, void* out, int outSize, MUINT32* realOutSize);
	//ACDK_CCT_OP_STROBE_SET_NVDATA, 11
	/** \brief set buf data
	*
	*/
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
	/** \brief write buf data to file
	*
	*/
	int cctNvdataToFile(void* in, int inSize, void* out, int outSize, MUINT32* realOutSize);
	//ACDK_CCT_OP_STROBE_FILE_TO_NVDATA,	//19
	/** \brief read file data and keep in buf.
	*
	*/
	int cctFileToNvdata(void* in, int inSize, void* out, int outSize, MUINT32* realOutSize);




  //20
  /** \brief for cct tool used only. Read nvram and change to ACDK_STROBE_STRUCT format.
	*
	*/
  int cctReadNvramToPcMeta(void* in, int inSize, void* out, int outSize, MUINT32* realOutSize);
  //21
  /** \brief for cct tool used only. change from ACDK_STROBE_STRUCT format and save to buf.
	*
	*/
  int cctSetNvdataMeta(void* in, int inSize, void* out, int outSize, MUINT32* realOutSize);












	//current not support
	/** \brief not used.
	*
	*/
	int cctCheckPara(); //return status
	/** \brief not used.
	*
	*/
	void cctGetCheckParaString(int* packetNO, int* isEnd, int* bufLength, unsigned char* buf); //buf: max 1024
	//---------------------------

	/** \brief do ratio calibration during precapture
	*
	*/
	void cctPreflashTest(FlashExePara* para, FlashExeRep* rep);
	/** \brief end ratio calibration during precapture
	*
	*/
	void cctPreflashEnd();



	/** \brief Set capture duty and step range
	*
	*/
	void cctSetCapDutyStep(int isEn, int duty, int step);



public:
    //======================
	//engineer mode related function
	//======================


	//int egnSetPfIndex(int duty, int step);
	//int egnSetMfIndex(int duty, int step);

	int egSetMfDutyStep(int duty, int step);
	int egGetDutyRange(int* st, int* ed);
	int egGetStepRange(int* st, int* ed);








public:
    //======================
	//normal used function
	//======================

    /** \brief constructor
	*
	*/
    FlashMgr();

    /** \brief deconstructor
	*
	*/
    ~FlashMgr();

    /** \brief get the handle of FlashMgr (singleton)
	*
	*/
    static FlashMgr* getInstance();

    /** \brief Before use the object, the object should be init.
	*   @return FlashMgr handle
	*/
    int init(int senorID);

    /** \brief uninit, when exit camera. or in the case, before the FlashMgr init again (with different init parameters).
	*   @return error code : (0: success)
	*/
    int uninit();

    /** \brief set senser device before init for other function reference.
	*   @return error code : (0: success, always return 0)
	*/
    int setSensorDev(int dev);

    /** \brief test if this capture should fire flash or not. call this function before preflash.
    *   @return 1: should fire flash. 0: should NOT fire flash.
	*/
    int isNeedFiringFlash();

    /** \brief test if this capture is with or without flash.
    *   @return 1: flash firing during capture. 0: flash not firing during capture.
	*/
    int isFlashOnCapture();

    /** \brief get AF lamp mode
    *   @return AF lamp mode AF_LAMP_T in alps\mediatek\custom\common\kernel\imgsensor\inc\kd_camera_feature_enum.h
	*/
	int getAfLampMode();

	/** \brief get AF lamp mode
	*   @return error code : (0: success)
	*/
	int setAfLampMode(int mode);

	/** \brief get flash mode
    *   @return flash mode AE_STROBE_T in alps\mediatek\custom\mt6582\hal\inc\aaa\flash_feature.h
    *   LIB3A_FLASH_MODE_T alps\mediatek\custom\mt6582\hal\inc\aaa\flash_feature.h
	*/
	int getFlashMode();

	/** \brief set flash mode
	*   @return error code : (0: success)
	*/
    int setFlashMode(int mode);

	/** \brief set cam mode
	*   @return error code : (0: success)
	*/
    int setCamMode(int mode);


	/** \brief get cam mode
	*   @return cam code : EAppMode in alps\mediatek\platform\mt6582\hardware\camera\inc\common\CamDefs.h
	*/
    int getCamMode();


    /** \brief set cam mode
	*   @param in mode : shot mode, CAPTURE_MODE_T in alps\mediatek\custom\common\kernel\imgsensor\inc\kd_camera_feature_enum.h
	*   @return error code : (0: success)
	*/
    int setShotMode(int mode);


    /** \brief check if it is shot mode or not
	*   @return 1: burst shot mode. 0: not burst shot mode
	*/
    int isBurstShotMode();

    /** \brief set digital zoom
	*   @param in digx100: digital zoom ratio*100
	*   @return error code : (0: success)
	*/
    int setDigZoom(int digx100);

    /** \brief set ev compensation value
	*   @param in ind: index of ev comp
	*   @param in ev_step: one step of space of index of ev comp. ev comp value = ind * ev_step
	*   @return error code : (0: success)
	*/
    int setEvComp(int ind, float ev_step);

    /** \brief befere capture, the fuction should call to start firing flash if needed.
	*   @return error code : (0: success)
	*/
    int capCheckAndFireFlash_Start();

    /** \brief after capture, the fuction should call to start firing flash if needed.
	*   @return error code : (0: success)
	*/
    int capCheckAndFireFlash_End();

    /** \brief at any time, call this function to turn of flash led.
	*   @return error code : (0: success)
	*/
    int turnOffFlashDevice();

    /** \brief get debug information.
	*   @return error code : (0: success)
	*/
    int getDebugInfo(FLASH_DEBUG_INFO_T* p);

    //void setFlashOnOff(int en);

    /** \brief turn on/off af lamp.
	*   @param in en : 1 on. 0 off
	*/
    void setAFLampOnOff(int en);

    void setTorchOnOff(int en);

    /** \brief check if AF lamp is on/off.
	*   @return error code : (0: success)
	*/
    int isAFLampOn();

	/** \brief process one frame of preflash stage
	*   @return error code : (0: success)
	*/
	int doPfOneFrame(FlashExePara* para, FlashExeRep* rep);

	/** \brief process one frame of capture stage
	*   @param in aa_adr : pointer of data buf (aa statistical data)
	*   @return error code : (0: success)
	*/
	int doMfOneFrame(void* aa_adr);

	/** \brief the function should be called after preflash is end.
	*   @return error code : (0: success)
	*/
	int endPrecapture();



	/** \brief the function should be called for capture preview end.
	*   @return error code : (0: success)
	*/
	int capturePreviewEnd();

	/** \brief the function should be called for capture preview start.
	*   @return error code : (0: success)
	*/
	int capturePreviewStart();

	/** \brief the function should be called for video preview start.
	*   @return error code : (0: success)
	*/
	int videoPreviewStart();

	/** \brief the function should be called for video preview end.
	*   @return error code : (0: success)
	*/
	int videoPreviewEnd();

	/** \brief the function should be called for video recording start.
	*   @return error code : (0: success)
	*/
	int videoRecordingStart();

	/** \brief the function should be called for video recording end.
	*   @return error code : (0: success)
	*/
	int videoRecordingEnd();


	int notifyAfEnter();

	int notifyAfExit();




	/** \brief get project parameters of flash (at auto mode)
	*   @return flash project parameters
	*/
	FLASH_PROJECT_PARA& getAutoProjectPara();

	/** \brief get project parameters of flash (by ae mode)
	*   @param aeMode: ae mode
	*   @return flash project parameters
	*/
	FLASH_PROJECT_PARA& getFlashProjectPara(int aeMode);


	/** \brief set function for handling jobs after flash turn on/off
	*   @param pFunc: pointer to post function
	*   @return
	*/
	void setPostFlashFunc(void (* pFunc)(int en));


	/** \brief set function for handling jobs before flash turn on/off
	*   @param pFunc: pointer to pre-flash function
	*   @return
	*/
	void setPreFlashFunc(void (* pFunc)(int en));


	/** \brief get state of flash flow
	*   @param
	*   @return e_NonePreview, e_VideoPreview, e_VideoRecording, e_CapturePreview, e_Capture
	*/
	int getFlashFlowState();


	int setFlickerMode(int mode);


	void setCapPara();

    void setPfParaToAe();



private:


    int isNeedWaitCooling(int* ms);
    int start();
    int run(FlashExePara* para, FlashExeRep* rep);
    int end();
	void hw_convert3ASta(FlashAlgStaData* staData, void* staBuf);

	void hw_setFlashProfile(FlashAlg* pStrobeAlg, FLASH_PROJECT_PARA* pPrjPara, NVRAM_CAMERA_STROBE_STRUCT* pNvram);
	void hw_setPreference(FlashAlg* pStrobeAlg, FLASH_PROJECT_PARA* pPrjPara);
	void hw_setCapPline(FLASH_PROJECT_PARA* pPrjPara, FlashAlg* pStrobeAlg);


	int nvForceRead();
    int nvGetBuf(NVRAM_CAMERA_STROBE_STRUCT*& buf);
    int nvWrite();
    int nvReadDefault();


	int getFlashModeStyle(int sensorType, int flashMode);
	int getVideoFlashModeStyle(int sensorType, int flashMode);

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
	int m_bRunPreFlash;
	float m_digRatio;

	int m_sensorType;
	int m_sensorDev2; //from setSensorDev (setParameter using. another is from init)
	FlashMgrDebug m_db;


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

	int m_isAfState;




};


#endif  //#define __FLASH_MGR_H__