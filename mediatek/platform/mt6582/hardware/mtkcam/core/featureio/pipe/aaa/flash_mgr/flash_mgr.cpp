#ifdef WIN32
#include "stdafx.h"
#include "FlashSim.h"
#include "sim_MTKAECommon.h"
#include "sim_MTKAE.h"
#include <mtkcam/algorithm/lib3a/FlashAlg.h>
#include "flash_mgr.h"
#else
#define LOG_TAG "flash_mgr.cpp"

#include <aaa_types.h>
#include <aaa_error_code.h>
#include <aaa_log.h>
#include <dbg_aaa_param.h>
#include <dbg_isp_param.h>
#include <mtkcam/hal/aaa_hal_base.h>
#include <aaa_hal.h>
#include <camera_custom_nvram.h>
#include <awb_param.h>
#include <ae_param.h>
#include <af_param.h>
#include <camera_custom_AEPlinetable.h>
#include <mtkcam/common.h>
using namespace NSCam;
#include <ae_mgr.h>
#include <mtkcam/algorithm/lib3a/ae_algo_if.h>
#include <mtkcam/hal/sensor_hal.h>
#include <nvram_drv_mgr.h>
#include <ae_tuning_custom.h>
#include <isp_mgr.h>
#include <isp_tuning.h>
#include <aaa_sensor_mgr.h>
#include <mtkcam/algorithm/lib3a/FlashAlg.h>
#include "flash_mgr.h"
#include "flash_tuning_custom.h"
#include "strobe_drv.h"
#include <time.h>
#include <kd_camera_feature.h>
#include "dbg_flash_param.h"
#include <isp_mgr.h>
#include <ispdrv_mgr.h>
#include <isp_tuning_mgr.h>
#include <nvram_drv.h>
#include <nvram_drv_mgr.h>
#include "flash_util.h"
#include <vector>
#include <mtkcam/acdk/cct_feature.h>

//#define CCT_TEST


using namespace NS3A;
using namespace NSIspTuning;

#define VERBOSE_STR "z.flash_verbose"

#define PROP_BIN_EN_STR		"z.flash_bin_en"
#define PROP_PF_BMP_EN_STR	"z.flash_pf_bmp_en"
#define PROP_MF_BMP_EN_STR	"z.flash_mf_bmp_en"


#define PROP_MF_ON_STR 		"z.flash_mf_on"
#define PROP_MF_DUTY_STR 	"z.flash_mf_duty"
#define PROP_MF_STEP_STR 	"z.flash_mf_step"
#define PROP_MF_EXP_STR 	"z.flash_mf_exp"
#define PROP_MF_ISO_STR 	"z.flash_mf_iso"
#define PROP_MF_AFE_STR 	"z.flash_mf_afe"
#define PROP_MF_ISP_STR 	"z.flash_mf_isp"

#define PROP_PF_ON_STR 		"z.flash_pf_on"
#define PROP_PF_DUTY_STR 	"z.flash_pf_duty"
#define PROP_PF_STEP_STR 	"z.flash_pf_step"
#define PROP_PF_EXP_STR 	"z.flash_pf_exp"
#define PROP_PF_ISO_STR 	"z.flash_pf_iso"
#define PROP_PF_AFE_STR 	"z.flash_pf_afe"
#define PROP_PF_ISP_STR 	"z.flash_pf_isp"



#define PROP_PF_EXP_FOLLOW_PLINE 	"z.flash_pf_by_pline"
#define PROP_PF_MAX_AFE	"z.flash_pf_max_afe"

#define PROP_MF_PLINE_EXTEND_GAIN	"z.flash_mf_pline_extend_gain"




#define ALG_TAG_SIZE 4080  //1020*4    //578*4 2312
#define A3_DIV_X 120
#define A3_DIV_Y 90
#define Z0_FL_DIV_X 24
#define Z0_FL_DIV_Y 18

#define LogInfo(fmt, arg...) XLOGD(fmt, ##arg)
#define LogVerbose(fmt, arg...) if(g_isVerboseLogEn) LogInfo(fmt, ##arg)
#define LogError(fmt, arg...)   XLOGE("MError: func=%s line=%d: "fmt, __FUNCTION__, __LINE__, ##arg)
//#define LogErrClear() {FILE* fp; fp=fopen("/sdcard/err.txt","wt"); fclose(fp);}
//#define LogErr(fmt, arg...)   {FILE* fp; fp=fopen("/sdcard/err.txt","at"); fprintf(fp,"MError: func=%s line=%d: "fmt, __FUNCTION__, __LINE__, ##arg); fprintf(fp, "\n"); fclose(fp);}
#define LogErr(fmt, arg...) XLOGE("MError: func=%s line=%d: "fmt, __FUNCTION__, __LINE__, ##arg)
#define LogWarning(fmt, arg...) XLOGE("FlashWarning: func=%s line=%d: "fmt, __FUNCTION__, __LINE__, ##arg)
#endif
//====================================================
// functions prototype
static void PLineTrans(PLine* p, strAETable* pAE);
static void AETableLim(strAETable* pAE, int maxExpLim);
static void PLineClear(PLine* p);
void hw_capIsoToGain(int iso, int* afe, int* isp);
void hw_isoToGain(int iso, int* afe, int* isp);
void hw_gainToIso(int afe, int isp, int* iso);
void hw_speedUpExpPara(FlashAlgExpPara* expPara, int maxAfe);
void ClearAePlineEvSetting();


//====================================================
struct FlasExpParaEx
{
   	int exp;
    int afeGain;
    int ispGain;
};
// variable
#ifdef WIN32
#else
StrobeDrv* g_pStrobe;
#endif
FlashAlgExpPara g_expPara;
FlasExpParaEx g_pfExpPara;
static int g_isVerboseLogEn=0;
int g_sceneCnt=0;
static int g_previewMode=FlashMgr::e_NonePreview;
static strEvSetting* g_plineEvSetting;
int m_flickerMode;

int g_eg_bUserMf=0;
int g_eg_mfDuty=-1;
int g_eg_mfStep=-1;
static void (* g_pPostFunc)(int en)=0;
static void (* g_pPreFunc)(int en)=0;
//====================================================
// function
//====================================================
void hwSetFlashOn(int reTrig=1)
{
	if(g_pPreFunc!=0)
    g_pPreFunc(1);
    if(reTrig==1)
	    g_pStrobe->setOnOff(0);
	g_pStrobe->setOnOff(1);
	if(g_pPostFunc!=0)
	g_pPostFunc(1);
}
void hwSetFlashOff()
{
	if(g_pPreFunc!=0)
    g_pPreFunc(0);
	g_pStrobe->setOnOff(0);
	if(g_pPostFunc!=0)
	g_pPostFunc(0);
}

void turnOnTorch()
{
    int bOn;
    int step;
    int duty;
	g_pStrobe->isOn(&bOn);
	g_pStrobe->getDuty(&duty);
	g_pStrobe->getStep(&step);

        FLASH_PROJECT_PARA prjPara;
    	int aeMode;
    	aeMode = AeMgr::getInstance().getAEMode();
    	prjPara = FlashMgr::getInstance()->getFlashProjectPara(aeMode);
   	LogInfo("turnOnTorch line=%d isOn=%d",__LINE__,bOn);
	if(bOn==0 || prjPara.engLevel.torchDuty!=duty || prjPara.engLevel.torchStep!=step)
	{
    	LogInfo("turnOnTorch duty,step=%d %d",prjPara.engLevel.torchDuty, prjPara.engLevel.torchStep);
    	g_pStrobe->setDuty(prjPara.engLevel.torchDuty);
    	g_pStrobe->setStep(prjPara.engLevel.torchStep);
    	g_pStrobe->setTimeOutTime(0);
    	hwSetFlashOn();
    }
}
void turnOnAf()
{
    int bOn;
    int step;
    int duty;
	g_pStrobe->isOn(&bOn);
	g_pStrobe->getDuty(&duty);
	g_pStrobe->getStep(&step);

	FLASH_PROJECT_PARA prjPara;
	int aeMode;
	aeMode = AeMgr::getInstance().getAEMode();
   	prjPara = FlashMgr::getInstance()->getFlashProjectPara(aeMode);
   	LogInfo("turnOnAf line=%d isOn=%d",__LINE__,bOn);
	if(bOn==0 || prjPara.engLevel.afDuty!=duty || prjPara.engLevel.afStep!=step)
	{
    	LogInfo("turnOnAf duty,step=%d %d",prjPara.engLevel.afDuty, prjPara.engLevel.afStep);
    	g_pStrobe->setDuty(prjPara.engLevel.afDuty);
    	g_pStrobe->setStep(prjPara.engLevel.afStep);
    	g_pStrobe->setTimeOutTime(0);
    	hwSetFlashOn();
    }
}

void updateVerboseFlag()
{
	g_isVerboseLogEn = FlashUtil::getPropInt(VERBOSE_STR, 0);
}
void ClearAePlineEvSetting()
{
	if(g_plineEvSetting!=0)
		delete []g_plineEvSetting;
	g_plineEvSetting=0;
}

int FlashMgr::getFlashFlowState()
{
	LogInfo("getFlashFlowState state=%d",g_previewMode);
	return g_previewMode;
}

int FlashMgr::getFlashModeStyle(int sensorType, int flashMode)
{
  return cust_getFlashModeStyle(m_sensorType, flashMode);

}


int FlashMgr::getVideoFlashModeStyle(int sensorType, int flashMode)
{

  return cust_getVideoFlashModeStyle(m_sensorType, flashMode);
}

FLASH_PROJECT_PARA& FlashMgr::getAutoProjectPara()
{
	return getFlashProjectPara(LIB3A_AE_MODE_AUTO);
}

FLASH_PROJECT_PARA& FlashMgr::getFlashProjectPara(int aeMode)
{
    NVRAM_CAMERA_STROBE_STRUCT* pNv;
    int err;
    err = nvGetBuf(pNv);
    if(err!=0)
        LogError("nvGetBuf err");
    return cust_getFlashProjectPara_V2(m_sensorType, aeMode, pNv);
}
FlashMgr::FlashMgr()
{
	g_plineEvSetting=0;
	m_isAFLampOn=0;
	m_iteration=0;
	///------------

	m_db.preFireStartTime=FlashUtil::getMs();
	m_db.preFireEndTime=m_db.preFireStartTime;
	m_db.coolingTime=0;
	m_db.coolingTM = 0;


	m_flashMode = LIB3A_FLASH_MODE_FORCE_OFF;
	m_flashOnPrecapture = 0;
	m_digRatio = 1;
	m_pfFrameCount=0;
	m_thisFlashDuty=-1;
	m_thisFlashStep=-1;
	m_thisIsFlashEn=0;
	m_evComp=0;

	m_isCapFlashEndTimeValid=1;
	g_pStrobe = StrobeDrv::createInstance();


	m_sensorType = (int)DUAL_CAMERA_MAIN_SENSOR;
	m_bRunPreFlash = 0;
	m_flickerMode = e_FlickerUnknown;
	//g_bCapturing=0;



	StrobeDrv* pDrv;
	pDrv = StrobeDrv::createInstance();
	int mainPartId;
	int subPartId;
	mainPartId = pDrv->getPartId(DUAL_CAMERA_MAIN_SENSOR);
	subPartId = pDrv->getPartId(DUAL_CAMERA_SUB_SENSOR);

	if(mainPartId>2)
		LogInfo("error: if(mainPartId>2)");
	if(subPartId>2)
		LogInfo("error: if(subPartId>2)");

	cust_setFlashPartId_main(mainPartId);
	cust_setFlashPartId_sub(subPartId);
}
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
FlashMgr::~FlashMgr()
{
    //g_bCapturing=0;


}
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
void hw_setPfPline(FlashAlg* pStrobeAlg)
{
#ifdef WIN32
	//pf pline
	MTKAE ae;
	PLine pfPline;
	PLineTrans(&pfPline, ae.m_pPreviewTableForward);
	pStrobeAlg->setPreflashPLine(&pfPline, 70);
	PLineClear(&pfPline);
#else
	LogInfo("hw_setPfPline() line=%d\n",__LINE__);
	strAETable pfPlineTab;
	strAETable capPlineTab;
	strAFPlineInfo pfPlineInfo;
	AE_DEVICES_INFO_T devInfo;
	AeMgr::getInstance().getCurrentPlineTable(pfPlineTab, capPlineTab, pfPlineInfo);
	AeMgr::getInstance().getSensorDeviceInfo(devInfo);
	PLine pfPline;
	PLineTrans(&pfPline, &pfPlineTab);
	pStrobeAlg->setPreflashPLine(&pfPline, devInfo.u4MiniISOGain);
	PLineClear(&pfPline);
	LogInfo("hw_setPfPline() line=%d u4MiniISOGain=%d\n",__LINE__,devInfo.u4MiniISOGain);
#endif
}

//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
void FlashMgr::hw_setCapPline(FLASH_PROJECT_PARA* pPrjPara, FlashAlg* pStrobeAlg)
{
#ifdef WIN32
	//cap pline
	MTKAE ae;
	PLine capPline;
	PLineTrans(&capPline, ae.m_pCaptureTable);
	pStrobeAlg->setCapturePLine(&capPline, 70);
	PLineClear(&capPline);
#else

	LogInfo("line=%d hw_setCapPline()\n",__LINE__);
	//err

	strAETable pfPlineTab;
	strAETable capPlineTab;
	strAFPlineInfo pfPlineInfo;
	AE_DEVICES_INFO_T devInfo;

	AeMgr::getInstance().getCurrentPlineTable(pfPlineTab, capPlineTab, pfPlineInfo);
	AeMgr::getInstance().getSensorDeviceInfo(devInfo);


	int i;
	/*
	FILE* fp;
	fp = fopen("/sdcard/aep.txt","wt");
	for(i=0;i<capPlineTab.u4TotalIndex;i++)
	{
			fprintf(fp, "%d\t%d\t%d\n",
			capPlineTab.pCurrentTable[i].u4Eposuretime,
			capPlineTab.pCurrentTable[i].u4AfeGain,
			capPlineTab.pCurrentTable[i].u4IspGain);
	}
	fclose(fp);
	*/

	PLine capPline;
	LogInfo("hw_setCapPline() line=%d pPrjPara->maxCapExpTimeUs=%d\n",__LINE__, pPrjPara->maxCapExpTimeUs);
	if(pPrjPara->maxCapExpTimeUs!=0)
	{
		AETableLim(&capPlineTab, pPrjPara->maxCapExpTimeUs);
	}

	/*
	fp = fopen("/sdcard/aep2.txt","wt");
	for(i=0;i<capPlineTab.u4TotalIndex;i++)
	{
			fprintf(fp, "%d\t%d\t%d\n",
			capPlineTab.pCurrentTable[i].u4Eposuretime,
			capPlineTab.pCurrentTable[i].u4AfeGain,
			capPlineTab.pCurrentTable[i].u4IspGain);
	}
	fclose(fp);
	*/

	PLineTrans(&capPline, &capPlineTab);

	int cap2PreRatio;
	if(eAppMode_ZsdMode==m_camMode)
		cap2PreRatio=1024;
	else
		cap2PreRatio=devInfo.u4Cap2PreRatio;
	pStrobeAlg->setCapturePLine(&capPline, devInfo.u4MiniISOGain* cap2PreRatio/1024);  //u4Cap2PreRatio: 1024 base, <1
	PLineClear(&capPline);

	LogInfo("line=%d u4MiniISOGain=%d cap/preview = %d(device), %d(real)\n",__LINE__,devInfo.u4MiniISOGain, devInfo.u4Cap2PreRatio, cap2PreRatio);

#endif
}


//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
void FlashMgr::hw_setFlashProfile(FlashAlg* pStrobeAlg, FLASH_PROJECT_PARA* pPrjPara, NVRAM_CAMERA_STROBE_STRUCT* pNvram)
{
#ifdef WIN32
	FlashAlgStrobeProfile	flashProfile;
	CFlashSimApp* app2 = (CFlashSimApp* )AfxGetApp();
	CString str2;
	str2 = app2->strAppPath;
	str2 += "\\data\\default_flash_profile.txt";
	InitFlashProfile(&flashProfile);
	ReadAndAllocatFlashProfile(str2.GetBuffer(0),&flashProfile);
	pStrobeAlg->setFlashProfile(&flashProfile);
	FreeFlashProfile(&flashProfile);
	//pStrobeAlg->setStrobeMaxDutyStep(12, 7, 31, 7);
	pStrobeAlg->setStrobeMaxDutyStep(12, 7, 31, 7);
	pStrobeAlg->setStrobeMinDutyStep(0, 7);
#else
	LogInfo("line=%d hw_setFlashProfile()\n");
	AE_DEVICES_INFO_T devInfo;
	AeMgr::getInstance().getSensorDeviceInfo(devInfo);

	FlashAlgStrobeProfile pf;
	pf.iso =  devInfo.u4MiniISOGain*(pNvram->engTab.afe_gain*pNvram->engTab.isp_gain)/1024/1024 ;
	pf.exp = pNvram->engTab.exp;
	pf.distance = pNvram->engTab.distance;
	pf.dutyNum = pPrjPara->dutyNum;
	pf.stepNum = pPrjPara->stepNum;
	pf.dutyTickNum = pPrjPara->dutyNum;
	pf.stepTickNum = pPrjPara->stepNum;
	int dutyTick[32];
	int stepTick[16];
	int i;
	for(i=0;i<pf.dutyNum;i++)
		dutyTick[i]=i;
	for(i=0;i<pf.stepNum;i++)
		stepTick[i]=i;
	pf.dutyTick = dutyTick;
	pf.stepTick = stepTick;

	float *engTable;
	engTable = new float[pf.dutyNum*pf.stepNum];
	for(i=0;i<pf.dutyNum*pf.stepNum;i++)
	{
		engTable[i]=pNvram->engTab.yTab[i];
	}
	pf.engTab = engTable;
	pStrobeAlg->setFlashProfile(&pf);
	delete []engTable;

	NVRAM_CAMERA_STROBE_STRUCT* pNv;
    nvGetBuf(pNv);

	LogInfo("m_pNvram engTab[0,1,2,3,4,5,6,7,15,23,31] %d %d %d %d %d %d %d %d %d %d %d\n",
	pNv->engTab.yTab[0],	pNv->engTab.yTab[1],	pNv->engTab.yTab[2],	pNv->engTab.yTab[3],	pNv->engTab.yTab[4],	pNv->engTab.yTab[5],	pNv->engTab.yTab[6],	pNv->engTab.yTab[7],	pNv->engTab.yTab[15],	pNv->engTab.yTab[23],	pNv->engTab.yTab[31]);

	LogInfo("m_pNvram-engTab[32,39,47,54,63] %d %d %d %d %d\n",
	pNv->engTab.yTab[32], pNv->engTab.yTab[39], pNv->engTab.yTab[47], pNv->engTab.yTab[54], pNv->engTab.yTab[63]);


	//@@ current mode
	if(0)
	//if(eng_p.pmfEngMode==ENUM_FLASH_ENG_CURRENT_MODE)
	{
		//mapdutystep
		//pStrobeAlg->setStrobeMaxDutyStep(eng_p.pfDuty, eng_p.pmfStep, eng_p.mfDuty, eng_p.pmfStep);
	}
	else
	{

		pStrobeAlg->setStrobeMaxDutyStep(pPrjPara->engLevel.pfDuty, pPrjPara->engLevel.pmfStep, pPrjPara->engLevel.mfDutyMax, pPrjPara->engLevel.pmfStep);
		pStrobeAlg->setStrobeMinDutyStep(pPrjPara->engLevel.mfDutyMin, pPrjPara->engLevel.pmfStep);
	}

	int vbat;
	int err;
	err = g_pStrobe->getVBat(&vbat);
	if(pPrjPara->engLevel.IChangeByVBatEn==1 && err==0)
	{
		LogInfo("setProfile-IChangeByVBatEn line=%d",__LINE__);
		if(vbat<pPrjPara->engLevel.vBatL)
		{
			pStrobeAlg->setStrobeMaxDutyStep(pPrjPara->engLevel.pfDutyL, pPrjPara->engLevel.pmfStepL, pPrjPara->engLevel.mfDutyMaxL, pPrjPara->engLevel.pmfStepL);
			pStrobeAlg->setStrobeMinDutyStep(pPrjPara->engLevel.mfDutyMinL, pPrjPara->engLevel.pmfStepL);
		}
	}

	if(m_shotMode==CAPTURE_MODE_BURST_SHOT)
	{
		LogInfo("setProfile-CAPTURE_MODE_BURST_SHOT line=%d",__LINE__);
		if(pPrjPara->engLevel.IChangeByBurstEn==1)
		{
			LogInfo("setProfile-IChangeByBurstEn en line=%d",__LINE__);
		pStrobeAlg->setStrobeMaxDutyStep(pPrjPara->engLevel.pfDutyB, pPrjPara->engLevel.pmfStepB, pPrjPara->engLevel.mfDutyMaxB, pPrjPara->engLevel.pmfStepB);
		pStrobeAlg->setStrobeMinDutyStep(pPrjPara->engLevel.mfDutyMinB, pPrjPara->engLevel.pmfStepB);
		}
	}




	//set debug info
		 m_db.pfI = pPrjPara->engLevel.pfDuty;
		 m_db.mfIMin = pPrjPara->engLevel.mfDutyMin;
		 m_db.mfIMax = pPrjPara->engLevel.mfDutyMax;
		 m_db.pmfIpeak = pPrjPara->engLevel.pmfStep;
		 m_db.torchIPeak = pPrjPara->engLevel.torchStep;
		 m_db.torchI = pPrjPara->engLevel.torchDuty;


    if(m_cct_isUserDutyStep==1)
	{
    	pStrobeAlg->setStrobeMaxDutyStep(pPrjPara->engLevel.pfDuty, pPrjPara->engLevel.pmfStep, m_cct_capDuty, pPrjPara->engLevel.pmfStep);
        pStrobeAlg->setStrobeMinDutyStep(m_cct_capDuty, pPrjPara->engLevel.pmfStep);
	}




#endif


}
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
void FlashMgr::hw_setPreference(FlashAlg* pStrobeAlg, FLASH_PROJECT_PARA* pPrjPara)
{
#ifdef WIN32
	pStrobeAlg->setDefaultPreferences();
	//setWTable256
	pStrobeAlg->setMeasuredDistanceCM(0);
	pStrobeAlg->setYTarget(188, 10);
	pStrobeAlg->setIsRefDistance(0);
	pStrobeAlg->SetAccuracyLevel(-10);
	pStrobeAlg->setIsoSuppressionLevel(-10);
	pStrobeAlg->setExpSuppressionLevel(-10);
	pStrobeAlg->setStrobeSuppressionLevel(-10);

	pStrobeAlg->setUnderSuppressionLevel(2);
	pStrobeAlg->setOverSuppressionLevel(0);
	pStrobeAlg->setForegroundWIncreaseLevel(0);
	pStrobeAlg->setEVComp(0);
	pStrobeAlg->setDebugDataSize(500);
#else
	//err
	int aeMode;
	//float evComp;
	aeMode = AeMgr::getInstance().getAEMode();

	FLASH_TUNING_PARA tune_p;
	tune_p = pPrjPara->tuningPara;
	pStrobeAlg->setDefaultPreferences();
	pStrobeAlg->setMeasuredDistanceCM(0);
	pStrobeAlg->setYTarget(tune_p.yTar, 10);
	pStrobeAlg->setIsRefDistance(tune_p.isRefAfDistance);
	pStrobeAlg->SetAccuracyLevel(tune_p.accuracyLevel);
	pStrobeAlg->setIsoSuppressionLevel(tune_p.antiIsoLevel);
	pStrobeAlg->setExpSuppressionLevel(tune_p.antiExpLevel);
	pStrobeAlg->setStrobeSuppressionLevel(tune_p.antiStrobeLevel);
	pStrobeAlg->setUnderSuppressionLevel(tune_p.antiUnderLevel);

	pStrobeAlg->setYTargetWeight(pPrjPara->yTargetWeight);	// TBD. refer to pPrjPara
	pStrobeAlg->setLowReflectaneThreshold(pPrjPara->lowReflectanceThreshold);
	pStrobeAlg->setLowReflectaneTuningEnable(pPrjPara->lowReflectanceTuningEnable);
	pStrobeAlg->setflashReflectanceWeight(pPrjPara->flashReflectanceWeight);






	int i4SceneLV = AeMgr::getInstance().getLVvalue(MFALSE);
	int overAdd=0;
	int fbAdd=0;
	if(i4SceneLV<10)
		overAdd=3;
	else if(i4SceneLV>50)
		overAdd=0;
	else
		overAdd=(int)(FlashUtil::flash_interp(5.0, 0.0, 1.0, 3.0, i4SceneLV/10.0)+0.5);
    fbAdd = overAdd;

	int antiOverLevel;
	antiOverLevel = tune_p.antiOverLevel+overAdd;
	if(antiOverLevel>10)
		antiOverLevel=10;
	pStrobeAlg->setOverSuppressionLevel(antiOverLevel);

	int fbLevel;
	fbLevel = tune_p.foregroundLevel+fbAdd;
	if(fbLevel>10)
	    fbLevel=10;
	pStrobeAlg->setForegroundWIncreaseLevel(fbLevel);

	int overLevel;
	overLevel = FlashUtil::getPropInt("z.flash_anti_over", -40);
	if(overLevel>=-10 && overLevel<=10)
		pStrobeAlg->setOverSuppressionLevel(overLevel);

	int foreBackLevel;
	foreBackLevel = FlashUtil::getPropInt("z.flash_fore_level", -40);
	if(foreBackLevel>=-10 && foreBackLevel<=10)
		pStrobeAlg->setForegroundWIncreaseLevel(foreBackLevel);





	LogInfo("hw_setPreference() i4SceneLV=%d",i4SceneLV);

	//evComp = AeMgr::getInstance().getEVCompensateIndex();
	//LogInfo("hw_setPreference() EvComp100(from AE)=%d",evComp);
	//evComp /= 100.0f;
	int maxTar;
	int num;
	float* evIndTab;
	float* evTab;
	float* evLevel;

	cust_getEvCompPara(maxTar, num, evIndTab, evTab, evLevel);

	//LogInfo("maxTar=%d",maxTar);
	//LogInfo("maxTar=%d",num);
	//LogInfo("maxTar=%d %d %d %d %d",(int)(evIndTab[0]*100),(int)(evIndTab[1]*100),(int)(evIndTab[2]*100),(int)(evIndTab[3]*100),(int)(evIndTab[4]*100));
	//LogInfo("maxTar=%d %d %d %d %d",(int)(evTab[0]*100),(int)(evTab[1]*100),(int)(evTab[2]*100),(int)(evTab[3]*100),(int)(evTab[4]*100));
	//LogInfo("maxTar=%d %d %d %d %d",(int)(evLevel[0]*100),(int)(evLevel[1]*100),(int)(evLevel[2]*100),(int)(evLevel[3]*100),(int)(evLevel[4]*100));

    float evComp;
    float evLevelRet;
    evComp = FlashUtil::flash_calYFromXYTab(5, evIndTab, evTab, (float)m_evComp);
    evLevelRet = FlashUtil::flash_calYFromXYTab(5, evIndTab, evLevel, (float)m_evComp);

    LogInfo("evcompx=%d %d %d",(int)(m_evComp*100), (int)(evComp*100), (int)(evLevelRet*100));




	//pStrobeAlg->setEVComp(m_evComp);
	pStrobeAlg->setEVCompEx(evComp, maxTar, evLevelRet);

	pStrobeAlg->setDebugDataSize(ALG_TAG_SIZE);

	LogInfo("hw_setPreference() yTar=%d",tune_p.yTar);




#endif
}

//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
void hw_turnOffFlash()
{
#ifdef WIN32
#else
	g_pStrobe = StrobeDrv::createInstance();
	hwSetFlashOff();
#endif
}
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx


void hw_setExpPara(FlashAlgExpPara* expPara, int sensorType, FLASH_PROJECT_PARA* pPrjPara)
{
#ifdef WIN32
	setHWExpParaWin(expPara);
#else
	LogInfo("hw_setExpPara pfexp1 %d %d %d exp=%d iso=%d",
	expPara->isFlash,	expPara->duty,	expPara->step,	expPara->exp,	expPara->iso);

	int exp;
	int iso;
	int afe;
	int isp;
	int isFlash;
	int duty;
	int step;


	int propFollowPline=-1;
	int propMaxAfe;
	propFollowPline = FlashUtil::getPropInt(PROP_PF_EXP_FOLLOW_PLINE,-1);
	propMaxAfe = FlashUtil::getPropInt(PROP_PF_MAX_AFE,-1);

	if(propFollowPline==-1)
	{
	    strAETable pfPlineTab;
    	strAETable capPlineTab;
    	strAFPlineInfo pfPlineInfo;
    	AeMgr::getInstance().getCurrentPlineTable(pfPlineTab, capPlineTab, pfPlineInfo);

        LogInfo("info_en %d", (int)pfPlineInfo.bAFPlineEnable);

    	if(pfPlineInfo.bAFPlineEnable)
			hw_speedUpExpPara(expPara, pPrjPara->maxPfAfe);

		//if(pPrjPara->pfExpFollowPline == 0)

	}
	else if(propFollowPline==0)
	{
		if(propMaxAfe==-1)
			hw_speedUpExpPara(expPara, pPrjPara->maxPfAfe);
		else
			hw_speedUpExpPara(expPara, propMaxAfe);
	}




	exp = expPara->exp;
	iso = expPara->iso;
	hw_isoToGain(iso, &afe, &isp);
	step = expPara->step;
	duty = expPara->duty;
	isFlash = expPara->isFlash;

	int propOn;
	int propExp;
	int propAfe;
	int propIsp;
	int propIso;
	int propStep;
	int propDuty;

	propOn = FlashUtil::getPropInt(PROP_PF_ON_STR,-1);
	propDuty = FlashUtil::getPropInt(PROP_PF_DUTY_STR,-1);
	propStep = FlashUtil::getPropInt(PROP_PF_STEP_STR,-1);
	propExp = FlashUtil::getPropInt(PROP_PF_EXP_STR,-1);
	propIso = FlashUtil::getPropInt(PROP_PF_ISO_STR,-1);
	propAfe = FlashUtil::getPropInt(PROP_PF_AFE_STR,-1);
	propIsp = FlashUtil::getPropInt(PROP_PF_ISP_STR,-1);

	if(propOn!=-1)
		isFlash = propOn;
	if(propDuty!=-1)
		duty=propDuty;
	if(propStep!=-1)
		step=propStep;
	if(propExp!=-1)
		exp=propExp;
	if(propIso!=-1)
	{
		iso=propIso;
		hw_isoToGain(iso, &afe, &isp);
	}

	if(propAfe!=-1)
		afe=propAfe;
	if(propIsp!=-1)
		isp=propIsp;

	int err;
   	if(isFlash)
   	{
   		g_pStrobe = StrobeDrv::createInstance();
   		g_pStrobe->setTimeOutTime(20000);
		g_pStrobe->setDuty(duty);
		g_pStrobe->setStep(step);
   		hwSetFlashOn(0);
   	}
	else
   	{
   		g_pStrobe = StrobeDrv::createInstance();
   		hwSetFlashOff();
   	}

	LogInfo("hw_setExpPara pfexp2 %d %d %d exp %d %d, %d %d",
	isFlash, duty, step, exp, iso,
	afe,	isp	);

	err = AAASensorMgr::getInstance().setSensorExpTime(exp);
    if (FAILED(err))
        return;

    err = AAASensorMgr::getInstance().setSensorGain(afe);
    if (FAILED(err))
        return;

	AE_INFO_T rAEInfo2ISP;
	rAEInfo2ISP.u4Eposuretime = exp;
    rAEInfo2ISP.u4AfeGain = afe;
    rAEInfo2ISP.u4IspGain = isp;
    rAEInfo2ISP.u4RealISOValue = iso;
    IspTuningMgr::getInstance().setAEInfo(rAEInfo2ISP);

	ISP_MGR_OBC_T::getInstance((ESensorDev_T)sensorType).setIspAEGain(isp>>1);
    // valdate ISP
    IspTuningMgr::getInstance().validatePerFrame(MFALSE);



	g_pfExpPara.exp = exp;
    g_pfExpPara.afeGain = afe;
    g_pfExpPara.ispGain = isp;


#endif
}

//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

void hw_setCapExpPara(FlashAlgExpPara* expPara)
{
#ifdef WIN32
	setHWExpParaWin(expPara);
#else
	LogInfo("hw_setCapExpPara mfexp1 %d %d %d exp=%d iso=%d",
	expPara->isFlash,	expPara->duty,	expPara->step,	expPara->exp,	expPara->iso);


	int propExp;
	int propIso;
	int propAfe;
	int propIsp;
	propExp = FlashUtil::getPropInt(PROP_MF_EXP_STR,-1);
	propIso = FlashUtil::getPropInt(PROP_MF_ISO_STR,-1);
	propAfe = FlashUtil::getPropInt(PROP_MF_AFE_STR,-1);
	propIsp = FlashUtil::getPropInt(PROP_MF_ISP_STR,-1);

	int iso;
	int exp;
	int afe;
	int isp;
	exp = expPara->exp;
	iso = expPara->iso;
	hw_capIsoToGain(iso, &afe, &isp);

	//prop
	if(propExp!=-1)
		exp = propExp;
	if(propIso!=-1)
	{
		iso=propIso;
		hw_capIsoToGain(iso, &afe, &isp);
	}
	if(propAfe!=-1)
		afe = propAfe;
	if(propIsp!=-1)
		isp = propIsp;






	AE_MODE_CFG_T capInfo;
	AeMgr::getInstance().getCaptureParams(0, 0, capInfo);
		capInfo.u4Eposuretime = exp;
		capInfo.u4AfeGain = afe;
		capInfo.u4IspGain = isp;

	AeMgr::getInstance().updateCaptureParams(capInfo);

	LogInfo("hw_setExpPara mfexp2 %d %d %d exp %d %d, %d %d",
	expPara->isFlash,	expPara->duty,	expPara->step, exp, iso,
	afe,	isp	);

#endif
}
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//int whAll, float r, int* whStart, int* whDiv, int* bin, int whDivMin, int whDivMax
//for x dirction example (120x90):
// whAll (in): 120
// r (in): ratio = 3
// whStart (out)  = 40
// whDiv (out): 20
// bin (out): 2
// whDivMin (in)
// whDivMax (out)
void calFlashDigWinNum(int whAll, float r, int* whStart, int* whDiv, int* bin, int whDivMin, int whDivMax)
{
	float whTar;
	whTar = whAll/r;
	whDivMin = (whDivMin+1) & 0xffe;
	whDivMax = whDivMax & 0xffe;

	float* TestErr;
	int* TestV;
	int testNum;
	testNum = (whDivMax-whDivMin)/2+1;
	TestErr = new float [testNum];
	TestV = new int [testNum];

	int whDigRet;
	int binRet;
	float minErr;
	int ind;

	int i;
	minErr=10000;
	if((int)whTar<=whDivMax)
	{
		whDigRet = ((int)whTar)/2*2;
		binRet = 1;
	}
	else
	{
		binRet=1;
		whDigRet = ((int)whTar)/2*2;

		for(i=0;i<=whTar/2;i++)
		{
			ind=whDivMax-2*i;
			if(ind<whDivMin)
				break;
			TestV[i]= ind;
			TestErr[i] =  whTar-(int)(whTar/ind)*ind;
			if(TestErr[i]==0)
			{
				whDigRet=ind;
				binRet=(int)(whTar/ind);
				break;
			}
			else
			{
				if(minErr>TestErr[i])
				{
					minErr=TestErr[i];
					whDigRet=ind;
					binRet=(int)(whTar/ind);
				}
			}
		}
	}
	*whDiv = whDigRet;
	*bin = binRet;
	*whStart = (whAll - whDigRet*binRet)/2;

	delete []TestErr;
	delete []TestV;
}
//r: digital zoom
//w: data w
//h: data h
//z0Wdiv: no digital zoom's wdiv
//z0Wdiv: no digital zoom's hdiv
//rzData: resized data
void resizeYData(double r, short* data, int w, int h, int z0Wdiv, int z0Ydiv, short* rzData, int* rzW, int* rzH)
{
	int i;
	int j;
	int wst;
	int wdiv;
	int wbin;
	int hst;
	int hdiv;
	int hbin;
	if(r<1.05)
	{
		wdiv = z0Wdiv;
		hdiv = z0Ydiv;
		wbin=w/wdiv;
		hbin=h/hdiv;
		wst= (w-wbin*wdiv)/2;
		hst= (h-hbin*hdiv)/2;
	}
	else
	{

		calFlashDigWinNum(w, r, &wst, &wdiv, &wbin, 20, 24);
		calFlashDigWinNum(h, r, &hst, &hdiv, &hbin, 15, 18);
		double werr;
		double herr;
		werr = (double)(w/r - wdiv*wbin)/ (w/r);
		herr = (double)(h/r - hdiv*hbin)/ (h/r);
		if(werr>0.1 || werr<-0.1)
		{
			calFlashDigWinNum(w, r, &wst, &wdiv, &wbin, 10, 24);
		}
		if(herr>0.1 || herr<-0.1)
		{
			calFlashDigWinNum(h, r, &hst, &hdiv, &hbin, 10, 18);
		}
	}
	for(i=0;i<wdiv*hdiv;i++)
		rzData[i]=0;

	for(j=hst;j<hst+hbin*hdiv;j++)
	for(i=wst;i<wst+wbin*wdiv;i++)
	{
		int id;
		int jd;
		id = (i-wst)/wbin;
		jd = (j-hst)/hbin;
		rzData[id+wdiv*jd]+=data[i+j*w];
	}
	for(i=0;i<wdiv*hdiv;i++)
		rzData[i]=rzData[i]/(wbin*hbin);
	*rzW = wdiv;
	*rzH = hdiv;
}
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
float g_uiDigRatio=1;
int FlashMgr::setDigZoom(int digx100)
{
	g_uiDigRatio = digx100/100.0;
	//m_digRatio = digx100/100.0;
	return 0;
}
int FlashMgr::isBurstShotMode()
{
	LogInfo("isBurstShotMode() shot mode=%d", m_shotMode);

	if(CAPTURE_MODE_BURST_SHOT==m_shotMode)
		return 1;
	else
		return 0;

}
int FlashMgr::setShotMode(int mode)
{
	LogInfo("setShotMode() mode=%d", mode);
	m_shotMode = mode;

	return 0;
}


void FlashMgr::hw_convert3ASta(FlashAlgStaData* staData, void* staBuf)
{
#ifdef WIN32
	get3ASta(staData);
#else
	//err
	AWBAE_STAT_T* p;
    p = (AWBAE_STAT_T*)staBuf;

    int i;
	int j;
	/*
	//--------------------------
    static int vv=0;
	vv++;
	FILE* fp;
	char s[100];
	sprintf(s,"/sdcard/aa_%03d.txt",vv);
	fp = fopen(s,"wt");
	for(j=0;j<90;j++)
	{
		for(i=0;i<120;i++)
		{
			fprintf(fp,"%d\t",p->LINE[j].AE_WIN[i]);
		}
		fprintf(fp,"\n");
	}
	fclose(fp);
	//--------------------------
*/

	short* A3Data;
	short* pData;
	pData = staData->data;
	A3Data = new short[A3_DIV_X*A3_DIV_Y];
	int ind=0;
	for(j=0;j<A3_DIV_Y;j++)
	for(i=0;i<A3_DIV_X;i++)
	{
		A3Data[ind]=p->LINE[j].AE_WIN[i]*4;
		ind++;
	}
	//static int vv=0;
	//vv++;
	//char ss[100];
	//sprintf(ss, "/sdcard/flashdata/a3_%03d%03d.bmp", g_fileCnt, vv);
	//arrayToGrayBmp(ss, A3Data, 2, A3_DIV_X, A3_DIV_Y, 1023);
	int rzW;
	int rzH;
	int toAwbW=0;
	int toAwbH=0;;
	resizeYData(m_digRatio, A3Data, A3_DIV_X, A3_DIV_Y, Z0_FL_DIV_X, Z0_FL_DIV_Y, pData, &rzW, &rzH);
	LogInfo("line=%d hw_convert3ASta m_digRatio=%lf, rzW=%d, rzH=%d", __LINE__, (double)m_digRatio, rzW, rzH);
	//sprintf(ss, "/sdcard/flashdata/fl_%03d%03d.bmp", g_fileCnt, vv);
	//arrayToGrayBmp(ss, pData, 2, rzW, rzH, 1023);
	if(m_digRatio>1.1)
	{
		pData+=rzW*rzH; //awb data pointer
		resizeYData(1, A3Data, A3_DIV_X, A3_DIV_Y, Z0_FL_DIV_X, Z0_FL_DIV_Y, pData, &toAwbW, &toAwbH);
		LogInfo("line=%d hw_convert3ASta m_digRatio=%lf, toAwbW=%d, toAwbH=%d", __LINE__, (double)m_digRatio, toAwbW, toAwbH);
	}
	staData->row = rzH;
	staData->col = rzW;
	staData->bit = 10;
	staData->normalizeFactor =1;
	staData->dig_row = toAwbH;
	staData->dig_col = toAwbW;

	delete []A3Data;


	//staData->dig_row =0;
#endif

}

//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
void hw_getAEExpPara(FlashAlgExpPara* aePara)
{
#ifdef WIN32
	getAEExpPara(aePara);
#else
	AE_MODE_CFG_T previewInfo;
	AeMgr::getInstance().getPreviewParams(previewInfo);
	AE_DEVICES_INFO_T devInfo;
	AeMgr::getInstance().getSensorDeviceInfo(devInfo);

	double gain;
	gain = (double)previewInfo.u4AfeGain*previewInfo.u4IspGain/1024/1024;
	int iso;
	iso = gain* devInfo.u4MiniISOGain;
	aePara->iso=iso;
	aePara->isFlash=0;
	aePara->exp=previewInfo.u4Eposuretime;

	LogInfo("aeexp %d %d %d %d minIsoGain=%d", previewInfo.u4Eposuretime, iso, previewInfo.u4AfeGain, previewInfo.u4IspGain, devInfo.u4MiniISOGain);

#endif
}
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
void hw_speedUpExpPara(FlashAlgExpPara* expPara, int maxAfe)
{
	 //re-calculate iso
    strAETable pfPlineTab;
	strAETable capPlineTab;
	strAFPlineInfo pfPlineInfo;
	int now_bv;
	AeMgr::getInstance().getCurrentPlineTable(pfPlineTab, capPlineTab, pfPlineInfo);
	now_bv = AeMgr::getInstance().getBVvalue();

	LogInfo("hw_speedUpExpPara bv=%d maxAfe=%d",now_bv, maxAfe);

	int maxIso;
	AE_DEVICES_INFO_T devInfo;
	AeMgr::getInstance().getSensorDeviceInfo(devInfo);

	if(maxAfe!=0)
	  hw_gainToIso(maxAfe, 15*1024, &maxIso);
	else
		hw_gainToIso(devInfo.u4MaxGain, 15*1024, &maxIso);

	LogInfo("maxIso=%d",maxIso);


	LogInfo("info_en %d", (int)pfPlineInfo.bAFPlineEnable);
	LogInfo("info_frm1 %d %d", (int)pfPlineInfo.i2FrameRate[0][0], (int)pfPlineInfo.i2FrameRate[0][1]);
	LogInfo("info_frm2 %d %d", (int)pfPlineInfo.i2FrameRate[1][0], (int)pfPlineInfo.i2FrameRate[1][1]);
	LogInfo("info_frm3 %d %d", (int)pfPlineInfo.i2FrameRate[2][0], (int)pfPlineInfo.i2FrameRate[2][1]);
	LogInfo("info_frm4 %d %d", (int)pfPlineInfo.i2FrameRate[3][0], (int)pfPlineInfo.i2FrameRate[3][1]);
	LogInfo("info_frm5 %d %d", (int)pfPlineInfo.i2FrameRate[4][0], (int)pfPlineInfo.i2FrameRate[4][1]);

	int lvTab[5];
	int fpsTab[5];
	int i;
	for(i=0;i<5;i++)
	{
		lvTab[i]=pfPlineInfo.i2FrameRate[i][0];
		fpsTab[i]=pfPlineInfo.i2FrameRate[i][1];
	}
	int fpsRet;
	int reducedExp;
	FlashUtil::flash_sortxy_xinc(5, lvTab, fpsTab);
	fpsRet = FlashUtil::flash_calYFromXYTab(5, lvTab, fpsTab, now_bv+50);
	reducedExp = 1000000/fpsRet;

	float g;
	g = (float)expPara->exp/reducedExp;
	float maxG;
	maxG = (float)maxIso*0.95/expPara->iso;
	LogInfo("hw_speedUpExpPara exp=%d iso=%d g=%f mxG=%f", expPara->exp, expPara->iso, g, maxG);
	if(g>maxG)
		g=maxG;
	if(g>1)
	{
		int expNew;
		expNew = expPara->exp/g;
		double align;
		if(m_flickerMode==FlashMgr::e_Flicker60)
			align=1e6/(double)120;
		else  //if(m_flickerMode==e_Flicker50)
			align=10000;
		//else
			//align=1;

		int expNew1;
		int expNew2;
		expNew1 = ((int)(expNew/align))*align;
		expNew2 = ((int)(expNew/align)+1)*align;

		double g1=0;
		double g2;
		if(expNew1!=0)
			g1 = (double)expPara->exp/expNew1;
		g2 = (double)expPara->exp/expNew2;

		if(expNew2>expPara->exp)
		{
			if(g1>maxG || expNew1==0)
			{
				expNew=expPara->exp;
			}
			else
			{
				expNew=expNew1;
			}
		}
		else
		{
			expNew=expNew2;
		}
#if 1
		double m;
		m= expPara->exp/(double)expNew;
		expPara->exp = expNew;
		expPara->iso = m* expPara->iso;
#else
		expPara->exp = expPara->exp/g;
	expPara->iso = g* expPara->iso

#endif
	}
	LogInfo("hw_speedUpExpPara exp=%d iso=%d", expPara->exp, expPara->iso);
}

void hw_gainToIso(int afe, int isp, int* iso)
{
	AE_DEVICES_INFO_T devInfo;
	AeMgr::getInstance().getSensorDeviceInfo(devInfo);
	double isoV;
	isoV = (double)devInfo.u4MiniISOGain*afe*isp/1024/1024;
	*iso = (int)isoV;

	LogInfo("dev_1xGainIso %d", (int)devInfo.u4MiniISOGain);
	LogInfo("dev_minG %d", (int)devInfo.u4MinGain);
	LogInfo("dev_maxG %d", (int)devInfo.u4MaxGain);
	LogInfo("line=%d hw_gainToIso afe=%d isp=%d iso=%d",__LINE__, afe, isp, *iso);
}
void hw_isoToGain(int iso, int* afe, int* isp)
{

#ifdef WIN32
	float g;
	g = (float)iso/70;
	if(g<3.5)
	{
		*afe=g*1024;
		*isp = 1024;
	}
	else
	{
		*afe = 3.5*1024;
		*isp = (g/3.5)*1024;
	}
#else

	AE_DEVICES_INFO_T devInfo;
	AeMgr::getInstance().getSensorDeviceInfo(devInfo);



	float g;
	g = (float)iso/devInfo.u4MiniISOGain;
	LogInfo("line=%d hw_isoToGain=iso=%d gain=%5.3f",__LINE__, iso, g);

	if(g<devInfo.u4MaxGain/1024.0f)
	{
		*afe=g*1024;
		*isp = 1024;
	}
	else
	{
		*afe = devInfo.u4MaxGain;
		*isp = (g*1024/devInfo.u4MaxGain)*1024;
	}

	FLASH_PROJECT_PARA prjPara;
	prjPara = FlashMgr::getInstance()->getAutoProjectPara();
	if(prjPara.maxAfeGain>0 && prjPara.maxAfeGain<(int)devInfo.u4MaxGain)
	{
		//LogInfo("prjPara.maxAfeGain %d", (int)prjPara.maxAfeGain);
		if(*afe>prjPara.maxAfeGain)
		{
			double m;
			m = (*afe)*(*isp);
			*afe = prjPara.maxAfeGain;
			*isp = m/(*afe);
		}
	}
	LogInfo("dev_1xGainIso %d", (int)devInfo.u4MiniISOGain);
	LogInfo("dev_minG %d", (int)devInfo.u4MinGain);
	LogInfo("dev_maxG %d", (int)devInfo.u4MaxGain);
	LogInfo("line=%d hw_isoToGain iso=%d afe=%d isp=%d (a=%5.3f)",__LINE__, iso, *afe, *isp, g);
#endif
}
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
void hw_capIsoToGain(int iso, int* afe, int* isp)
{

#ifdef WIN32
	float g;
	g = (float)iso/70;
	if(g<3.5)
	{
		*afe=g*1024;
		*isp = 1024;
	}
	else
	{
		*afe = 3.5*1024;
		*isp = (g/3.5)*1024;
	}
#else
    //eer recalculate iso
	AE_DEVICES_INFO_T devInfo;
	AeMgr::getInstance().getSensorDeviceInfo(devInfo);
	float g;

	int cap2PreRatio;
	if(eAppMode_ZsdMode==FlashMgr::getInstance()->getCamMode())
		cap2PreRatio=1024;
	else
		cap2PreRatio=devInfo.u4Cap2PreRatio;

	g = (float)iso/((double)devInfo.u4MiniISOGain* cap2PreRatio/1024);
	LogInfo("line=%d hw_capIsoToGain=iso=%d gain=%5.3f",__LINE__, iso, g);

	if(g<devInfo.u4MaxGain/1024.0f)
	{
		*afe=g*1024;
		*isp = 1024;
	}
	else
	{
		*afe = devInfo.u4MaxGain;
		*isp = (g*1024/devInfo.u4MaxGain)*1024;
	}

	FLASH_PROJECT_PARA prjPara;
	prjPara = FlashMgr::getInstance()->getAutoProjectPara();
	if(prjPara.maxAfeGain>0 && prjPara.maxAfeGain<(int)devInfo.u4MaxGain)
	{
		//LogInfo("prjPara.maxAfeGain %d", (int)prjPara.maxAfeGain);
		if(*afe>prjPara.maxAfeGain)
		{
			double m;
			m = (*afe)*(*isp);
			*afe = prjPara.maxAfeGain;
			*isp = m/(*afe);
		}
	}
	LogInfo("hw_capIsoToGain dev_1xGainIso %d", (int)devInfo.u4MiniISOGain);
	LogInfo("hw_capIsoToGain dev_minG %d", (int)devInfo.u4MinGain);
	LogInfo("hw_capIsoToGain dev_maxG %d", (int)devInfo.u4MaxGain);
	LogInfo("hw_capIsoToGain iso=%d afe=%d isp=%d (a=%5.3f)", iso, *afe, *isp, g);
#endif
}
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
FlashMgr* FlashMgr::getInstance()
{
    static  FlashMgr singleton;
    return  &singleton;
}
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
int FlashMgr::init(int sensorType)
{
	LogInfo("init sensorTypo=%d",sensorType);
	m_sensorType = sensorType;
	g_pStrobe = StrobeDrv::createInstance();
	g_pStrobe->init(sensorType);
	NVRAM_CAMERA_STROBE_STRUCT* pNv;
    nvGetBuf(pNv);
	//forceLoadNvram();
	return 0;
}
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
int FlashMgr::uninit()
{
	LogInfo("uninitt");
	g_pStrobe = StrobeDrv::createInstance();
	hwSetFlashOff();
	g_pStrobe->uninit();
	m_flashMode = LIB3A_FLASH_MODE_UNSUPPORTED;
	g_previewMode=e_NonePreview;
	return 0;
}

//=======================================
int FlashMgr::setSensorDev(int sensorDev)
{
    LogInfo("setSensorDev");
    m_sensorType = sensorDev;
    return 0;
}


//=======================================
/*
void FlashMgr::setFlashOnOff(int en)
{
	g_pStrobe = StrobeDrv::createInstance();
	g_pStrobe->setOnOff(en);
}*/

int FlashMgr::isAFLampOn()
{
	//return m_isAFLampOn;
	int bOn;
	g_pStrobe->isOn(&bOn);
	LogInfo("isAFLampOn %d",bOn);
	return bOn;
}
void FlashMgr::setAFLampOnOff(int en)
{
	m_isAFLampOn=en;
	if(en==1)
	{
		LogInfo("setAFLampOnOff 1");
		turnOnAf();
	}
	else
	{
		LogInfo("setAFLampOnOff 0");
		g_pStrobe->setTimeOutTime(1000);
		hwSetFlashOff();
	}
}
//============================================
void FlashMgr::setTorchOnOff(int en)
{
	if(en==1)
	{
		LogInfo("setTorchOnOff 1");
		turnOnTorch();
	}
	else
	{
		LogInfo("setTorchOnOff 0");
		g_pStrobe->setTimeOutTime(1000);
		hwSetFlashOff();
	}
}
//============================================

void FlashMgr::addErr(int err)
{
	m_db.err3=m_db.err2;
	m_db.err2=m_db.err1;
	m_db.err1=err;

	m_db.errTime3=m_db.errTime2;
	m_db.errTime2=m_db.errTime1;
	m_db.errTime1=FlashUtil::getMs();

}

int FlashMgr::getDebugInfo(FLASH_DEBUG_INFO_T* p)
{
	int sz;
	sz = sizeof(FLASH_DEBUG_INFO_T);
	memset(p, 0, sz);

	setDebugTag(*p, FL_T_VERSION, (MUINT32) FLASH_DEBUG_TAG_VERSION);
  	//setDebugTag(*p, FL_T_SCENE_MODE, (MUINT32) m_db.sceneMode);
	setDebugTag(*p, FL_T_IS_FLASH_ON, (MUINT32) m_flashOnPrecapture); //
	setDebugTag(*p, FL_T_ISO, (MUINT32) m_db.capIso);
	setDebugTag(*p, FL_T_AFE_GAIN, (MUINT32) m_db.capAfeGain);
	setDebugTag(*p, FL_T_ISP_GAIN, (MUINT32) m_db.capIspGain);
	setDebugTag(*p, FL_T_EXP_TIME, (MUINT32) m_db.capExp);
	setDebugTag(*p, FL_T_DUTY, (MUINT32) m_db.capDuty);
	setDebugTag(*p, FL_T_STEP, (MUINT32) m_db.capStep);

	setDebugTag(*p, FL_T_ERR1, (MUINT32) m_db.err1); //@@
	setDebugTag(*p, FL_T_ERR2, (MUINT32) m_db.err2); //@@
	setDebugTag(*p, FL_T_ERR3, (MUINT32) m_db.err3); //@@
	setDebugTag(*p, FL_T_ERR1_TIME, (MUINT32) m_db.errTime1); //@@
	setDebugTag(*p, FL_T_ERR2_TIME, (MUINT32) m_db.errTime2); //@@
	setDebugTag(*p, FL_T_ERR3_TIME, (MUINT32) m_db.errTime3); //@@

	setDebugTag(*p, FL_T_VBAT, (MUINT32) m_db.vBat); //@@
	setDebugTag(*p, FL_T_ISO_INC_MODE, (MUINT32) m_db.isoIncMode);
	setDebugTag(*p, FL_T_ISO_INC_VALUE, (MUINT32) m_db.isoIncValue);
	setDebugTag(*p, FL_T_PF_I, (MUINT32) m_db.pfI);
	setDebugTag(*p, FL_T_MF_I_MIN, (MUINT32) m_db.mfIMin);
	setDebugTag(*p, FL_T_MF_I_MAX, (MUINT32) m_db.mfIMax);
	setDebugTag(*p, FL_T_PMF_I_PEAK, (MUINT32) m_db.pmfIpeak);
	setDebugTag(*p, FL_T_TORCH_I_PEAK, (MUINT32) m_db.torchIPeak);
	setDebugTag(*p, FL_T_TORCH_I, (MUINT32) m_db.torchI);


	setDebugTag(*p, FL_T_PF_START_COOLING_TIME, (MUINT32) m_db.startCoolingTime);
	setDebugTag(*p, FL_T_PF_START_TIME, (MUINT32) m_db.startTime);
	setDebugTag(*p, FL_T_PF_END_TIME, (MUINT32) m_db.endTime);
	setDebugTag(*p, FL_T_PRE_FIRE_ST_TIME, (MUINT32) m_db.preFireStartTime); //@@
	setDebugTag(*p, FL_T_PRE_FIRE_ED_TIME, (MUINT32) m_db.preFireEndTime); //@@
	setDebugTag(*p, FL_T_COOLING_TIME, (MUINT32) m_db.coolingTime); //@@
	setDebugTag(*p, FL_T_EST_PFMF_TIME, (MUINT32) m_db.estPf2MFTime); //@@
	setDebugTag(*p, FL_T_DELAY_TIME, (MUINT32) m_db.delayTime); //@@

	if(m_flashOnPrecapture==1)
	{
		int algDebug[ALG_TAG_SIZE/4];
		FlashAlg* pStrobeAlg;
		pStrobeAlg = FlashAlg::getInstance();
		int i;
		if(m_bRunPreFlash)
		{
		pStrobeAlg->fillDebugData2(algDebug);
	}
	else
	{
		    for(i=0;i<ALG_TAG_SIZE/4;i++)
		        algDebug[i]=0;
}

		for(i=0;i<ALG_TAG_SIZE/4;i++)
{
			setDebugTag(*p, FL_T_NUM+i, (MUINT32)algDebug[i]);
}
	}
	return 0;
}
///////////////////////////////////////////

int FlashMgr::cctGetFlashInfo(void* in, int inSize, void* out, int outSize, MUINT32* realOutSize)
{

	int* ret;
	ret = (int*)out;
	if(getFlashMode()==FLASHLIGHT_FORCE_ON)
	{
		*ret = 1;
	}
	else
	{
		*ret = 0;
	}


	*realOutSize =4;
	return 0;
}

int FlashMgr::cctFlashLightTest(void* pIn)
{
	int* p;
	p = (int*)pIn;
	int duration;
	duration = 300000;


	LogInfo("cctFlashLightTest() p[0]=%d, p[1]=%d", p[0], p[1]);


	FLASH_PROJECT_PARA prjPara;
	prjPara = getAutoProjectPara();


	int err=0;
	int e;
	StrobeDrv* pStrobe = StrobeDrv::createInstance();
	pStrobe->init(1);
	pStrobe->setDuty(prjPara.engLevel.torchDuty);
	pStrobe->setStep(prjPara.engLevel.torchStep);
	LogInfo("cctFlashLightTest() duty=%d, duty num=%d", prjPara.engLevel.torchDuty, prjPara.engLevel.torchStep);

	pStrobe->setTimeOutTime(2000);


	err = pStrobe->setOnOff(0);
	e   = pStrobe->setOnOff(1);
	if(err==0)
		err = e;
	usleep(duration);
	e   = pStrobe->setOnOff(0);
	if(err==0)
		err = e;
	usleep(duration);
	pStrobe->uninit();
	LogInfo("cctFlashLightTest() err=%d", err);
	return err;
}


int FlashMgr::isNeedWaitCooling(int* a_waitTimeMs)
{
	int isNeedWait=0;
	*a_waitTimeMs=0;
	int curTime = FlashUtil::getMs();
	static int coolingFrame=0;
	if(m_db.coolingTM!=0)
	{
		m_db.preFireStartTime = m_db.thisFireStartTime;
		m_db.preFireEndTime = m_db.thisFireEndTime;
		m_db.coolingTime = (m_db.thisFireEndTime-m_db.thisFireStartTime)*m_db.coolingTM;

LogInfo("isNeedWaitCooling() coolingTime=%d startTime=%d endTime=%d coolingTMx1000=%d",
			(int)(m_db.coolingTime),(int)m_db.thisFireStartTime, (int)m_db.thisFireEndTime, (int)(m_db.coolingTM*1000));

		int waitTime;
		m_db.estPf2MFTime=300;
		waitTime = (m_db.preFireEndTime+m_db.coolingTime) - (curTime+m_db.estPf2MFTime);
LogInfo("isNeedWaitCooling() waitTime=%d endTime=%d coolingTime=%d curTime=%d, estPf2MFTime=%d coolFrame=%d",
		(int)(waitTime), (int)m_db.preFireEndTime, (int)m_db.coolingTime, (int)curTime, (int)m_db.estPf2MFTime, (int)coolingFrame);


		if(coolingFrame==0)
		{
			m_db.startCoolingTime = curTime;
			if(waitTime>0)
				m_db.delayTime = waitTime;
			else
				m_db.delayTime=0;
		}
		coolingFrame++;


		if(waitTime<=0 || coolingFrame>150)
		{
			coolingFrame=0;
			isNeedWait = 0;
		}
		else
		{
			isNeedWait=1;
			*a_waitTimeMs=waitTime;
		}
	}
	else
	{
		isNeedWait=0;
	}

	return isNeedWait;
}

int FlashMgr::doPfOneFrame(FlashExePara* para, FlashExeRep* rep)
{
    LogInfo("doPfOneFrame +");
	/*
	FlashAlgStaData staData;
	short g_data2[40*30*2];
	staData.data = g_data2;
	hw_convert3ASta(&staData, para->staBuf);

	{
		int i;
		for(i=42;i<47;i++)
			LogInfo("aay %d %d %d %d %d",
	}*/

/*
	int aaArr[25];
  FlashUtil::aaSub((void*)para->staBuf, aaArr);
  int i;
  for(i=0;i<5;i++)
  {
  	XLOGD("pre aeyy %d\t%d\t%d\t%d\t%d\t%d",i, aaArr[i*5+0], aaArr[i*5+1], aaArr[i*5+2], aaArr[i*5+3], aaArr[i*5+4]);
  }
  */



    static int pfRunCycleFrames=3;//if sensor delay frame > 2, the value should be change. ( = delay frame +1 )




	int ratioEn;
	ratioEn = FlashUtil::getPropInt("z.flash_ratio",0);
	if(ratioEn==1)
	{
		cctPreflashTest(para, rep);
		return 0;
	}


#define FLASH_STATE_START 0
#define FLASH_STATE_COOLING 1
#define FLASH_STATE_RUN 2
#define FLASH_STATE_NULL 3


#define FLASH_STATUS_OFF 0
#define FLASH_STATUS_NEXT_ON 1
#define FLASH_STATUS_ON 2




	rep->isEnd=0;
	static int flashStatus=FLASH_STATUS_OFF;
	static int preExeFrameCnt; //only for start() and run()
	static int flashState=FLASH_STATE_START;
	if(m_pfFrameCount==0)
	{
		flashState=FLASH_STATE_START;
		int curTime;
		curTime = FlashUtil::getMs();
		LogInfo("doPfOneFrame start ms=%d", curTime);
		m_flickerMode = para->flickerMode;

		MINT32 i4SutterDelay, i4SensorGainDelay, i4IspGainDelay;
		AAASensorMgr::getInstance().getSensorSyncinfo(&i4SutterDelay, &i4SensorGainDelay, &i4IspGainDelay);
		int maxDelay;
		maxDelay = i4SutterDelay;
		if(maxDelay<i4SensorGainDelay)
			maxDelay=i4SensorGainDelay;
		if(maxDelay<i4IspGainDelay)
			maxDelay=i4IspGainDelay;
		pfRunCycleFrames = maxDelay+1;


	}
		LogInfo("doPfOneFrame frame=%d flashState=%d preExeFrame=%d pfRunFrames=%d",m_pfFrameCount,flashState,preExeFrameCnt,pfRunCycleFrames);
	if(flashState==FLASH_STATE_START)
	{
		flashStatus = FLASH_STATUS_OFF;
		LogInfo("doPfOneFrame state=start");
		m_digRatio = g_uiDigRatio;
		start();
		if(g_eg_mfDuty!=-1 && g_eg_mfStep!=-1)
		{
		    g_eg_bUserMf=1;
		    //rep->isEnd=1;
		}
		else
		{
		    g_eg_bUserMf=0;
		}

		if(m_bRunPreFlash==0)
			rep->isEnd=1;
		else
		{
			flashState=FLASH_STATE_COOLING;
		}
		preExeFrameCnt=m_pfFrameCount;
	}
	else if(flashState==FLASH_STATE_COOLING)
	{
		flashStatus = FLASH_STATUS_OFF;
		LogInfo("doPfOneFrame state=cooling");
		int waitMs;
		if(isNeedWaitCooling(&waitMs)==1)
		{
			LogInfo("cooling time=%d",waitMs);
			flashState=FLASH_STATE_COOLING;
		}
		else
		{
			flashState=FLASH_STATE_RUN;
		}
	}
	if(flashState==FLASH_STATE_RUN)
	{
		LogInfo("doPfOneFrame state=run");
		if(m_pfFrameCount-preExeFrameCnt>=pfRunCycleFrames)
		{
			run(para, rep);

			/*
			if(flashStatus == FLASH_STATUS_OFF)
			{
				if(rep->nextIsFlash==1)
					flashStatus = FLASH_STATUS_NEXT_ON;
			}
			else if(flashStatus == FLASH_STATUS_NEXT_ON)
			{
				if(rep->nextIsFlash==1)
					flashStatus = FLASH_STATUS_ON;
			}*/
			if(rep->nextIsFlash==1)
				flashStatus = FLASH_STATUS_ON;



			flashState=FLASH_STATE_RUN;
			preExeFrameCnt=m_pfFrameCount;
			if(rep->isEnd==1)
				flashState=FLASH_STATE_NULL;
		}
	}
	else if(flashState==FLASH_STATE_NULL)
	{

	}

	if(flashStatus == FLASH_STATUS_ON)
		rep->isCurFlashOn=1;
	else
		rep->isCurFlashOn=0;

	int pfBmpEn;
	pfBmpEn  = FlashUtil::getPropInt(PROP_PF_BMP_EN_STR,0);
	if(pfBmpEn==1)
	{
		char aeF[256];
		char awbF[256];
		sprintf(aeF, "/sdcard/flashdata/bmp/pf_ae_%03d_%02d.bmp",g_sceneCnt,m_pfFrameCount);
		sprintf(awbF, "/sdcard/flashdata/bmp/pf_awb_%03d_%02d.bmp",g_sceneCnt,m_pfFrameCount);
		FlashUtil::aaToBmp((void*)para->staBuf,  aeF, awbF);
	}

	m_pfFrameCount++;
	LogInfo("doPfOneFrame isEnd=%d",rep->isEnd);
	LogInfo("doPfOneFrame -");
	return 0;
}

int FlashMgr::doMfOneFrame(void* aa_adr)
{
	int mfBmpEn;
	mfBmpEn  = FlashUtil::getPropInt(PROP_MF_BMP_EN_STR,0);
	if(mfBmpEn==1)
	{
		char aeF[256];
		char awbF[256];
		sprintf(aeF, "/sdcard/flashdata/bmp/mf_ae_%03d.bmp",g_sceneCnt);
		sprintf(awbF, "/sdcard/flashdata/bmp/mf_awb_%03d.bmp",g_sceneCnt);
		FlashUtil::aaToBmp((void*)aa_adr,  aeF, awbF);
	}
	return 0;
}

int FlashMgr::endPrecapture()
{
    int flashMode;
    int flashStyle;
    flashMode = getFlashMode();
    flashStyle = getFlashModeStyle(m_sensorType, flashMode);
    LogInfo("endPrecapture %d %d",flashMode,flashStyle);
    if(flashStyle==(int)e_FLASH_STYLE_ON_ON || flashStyle==(int)e_FLASH_STYLE_ON_TORCH)
	{
	    turnOnTorch();
	}
	else
	{
		turnOffFlashDevice();
	}

	ClearAePlineEvSetting();
	int ratioEn;
	ratioEn = FlashUtil::getPropInt("z.flash_ratio",0);
	if(ratioEn==1)
	{
		cctPreflashEnd();
		return 0;
	}

	m_pfFrameCount=0;
	return 0;

}

int FlashMgr::isNeedFiringFlash()
{
    LogInfo("isNeedFiringFlash()");
    m_db.startTime = FlashUtil::getMs();

    if(g_pStrobe->hasFlashHw()!=0)
{
	if(m_cct_isUserDutyStep==1)
		return 1;
    }

    int fmode;
    int fstyle;
    fmode = getFlashMode();
    fstyle = getFlashModeStyle(m_sensorType, fmode);

    int bFlashOn;
    int bRunPreFlash;
    int ispFlashMode;
    if(g_pStrobe->hasFlashHw()==0)
    {
        LogInfo("isNeedFiringFlash() No flash hw");
        bFlashOn=0;
        bRunPreFlash=0;
        ispFlashMode=FLASHLIGHT_FORCE_OFF;
    }
    else if(g_eg_mfDuty!=-1 && g_eg_mfStep!=-1)
    {
        LogInfo("isNeedFiringFlash() eng");
        bFlashOn=1;
        bRunPreFlash=0;
        ispFlashMode=FLASHLIGHT_FORCE_ON;

        int bPropPfEn;
        bPropPfEn  = FlashUtil::getPropInt("z.flash_eg_pf_en",-1);
        if(bPropPfEn==1)
            bRunPreFlash=1;
    }
    else if(fstyle==e_FLASH_STYLE_OFF_OFF)
    {
        LogInfo("isNeedFiringFlash() XX");
    	bFlashOn=0;
        bRunPreFlash=0;
        ispFlashMode=FLASHLIGHT_FORCE_OFF;
    }
    else if(fstyle==e_FLASH_STYLE_OFF_ON || fstyle==e_FLASH_STYLE_ON_ON)
    {
        LogInfo("isNeedFiringFlash() XO OO");
    	bFlashOn=1;
        bRunPreFlash=1;
        ispFlashMode=FLASHLIGHT_FORCE_ON;
    }
    else if(fstyle==e_FLASH_STYLE_ON_TORCH)
    {
        LogInfo("isNeedFiringFlash() OT");
        bFlashOn=1;
        bRunPreFlash=0;
        ispFlashMode=FLASHLIGHT_FORCE_ON;
    }
    else //if(fstyle==e_FLASH_STYLE_OFF_AUTO)
    {
    	if(AeMgr::getInstance().IsStrobeBVTrigger()==1)
    	{
            ispFlashMode = FLASHLIGHT_AUTO;
	    	bFlashOn=1;
            bRunPreFlash=1;
            LogInfo("isNeedFiringFlash() XA triger");
    	}
    	else
    	{
            ispFlashMode = FLASHLIGHT_AUTO;
    		bFlashOn=0;
            bRunPreFlash=0;
            LogInfo("isNeedFiringFlash() XA NOT triger");
    	}
    }
    m_bRunPreFlash = bRunPreFlash;

    FLASH_INFO_T finfo;
    finfo.flashMode = ispFlashMode;
    finfo.isFlash = bFlashOn;
	IspTuningMgr::getInstance().setFlashInfo(finfo);

   	if(bFlashOn==0)
   	{
   		m_flashOnPrecapture=0;
   		m_db.endTime = FlashUtil::getMs();
   		return 0;
   	}
   	else
   	{
   		m_flashOnPrecapture=1;
   	}
   	return 1;

}


void debugCnt()
{
	LogInfo("debugCnt %d",__LINE__);
	int binEn;
	int pfBmpEn;
	int mfBmpEn;
	binEn = FlashUtil::getPropInt(PROP_BIN_EN_STR,0);
	pfBmpEn  = FlashUtil::getPropInt(PROP_PF_BMP_EN_STR,0);
	mfBmpEn  = FlashUtil::getPropInt(PROP_MF_BMP_EN_STR,0);
	LogInfo("debugCnt binEn, pfBmpEn, mfBmpEn %d %d %d", binEn, pfBmpEn, mfBmpEn);
	if(binEn==1 || pfBmpEn==1 || mfBmpEn==1)
	{
		FlashUtil::getFileCount("/sdcard/flash_file_cnt.txt", &g_sceneCnt, 0);
		FlashUtil::setFileCount("/sdcard/flash_file_cnt.txt", g_sceneCnt+1);
	}
	if(pfBmpEn==1 || mfBmpEn==1)
	{
		LogInfo("debugCnt %d",__LINE__);
		FlashUtil::createDir("/sdcard/flashdata/");
		FlashUtil::createDir("/sdcard/flashdata/bmp/");
	}
	if(binEn==1)
	{
		LogInfo("binEn = %d",binEn);
		FlashAlg* pStrobeAlg;
		pStrobeAlg = FlashAlg::getInstance();
		pStrobeAlg->setIsSaveSimBinFile(1);
		char prjName[50];
		sprintf(prjName,"%03d",g_sceneCnt);
		pStrobeAlg->setDebugDir("/sdcard/flashdata/",prjName);
	}

}

void logProjectPara(FLASH_PROJECT_PARA* pp)
{
	FLASH_TUNING_PARA *pt;
	FLASH_ENG_LEVEL *pe;
	FLASH_COOL_TIMEOUT_PARA *pct;

    LogInfo("projectPara ds %d %d",pp->dutyNum, pp->stepNum);

	pt = &pp->tuningPara;
	LogInfo("prjP tp %d %d %d %d %d %d %d %d %d",
	    pt->yTar, pt->antiIsoLevel, pt->antiExpLevel, pt->antiStrobeLevel, pt->antiUnderLevel,
	    pt->antiOverLevel, pt->foregroundLevel, pt->isRefAfDistance, pt->accuracyLevel  );

	pe = &pp->engLevel;
	LogInfo("prjP eng1 %d %d, %d %d, %d %d %d %d",
	    pe->torchDuty, pe->torchStep, pe->afDuty, pe->afStep,
	    pe->pfDuty, pe->pmfStep, pe->mfDutyMin, pe->mfDutyMax);

	LogInfo("prjP engL %d %d %d %d, %d %d",
	    pe->IChangeByVBatEn, pe->vBatL, pe->pfDutyL, pe->pmfStepL,
	    pe->mfDutyMinL, pe->mfDutyMaxL);

	LogInfo("prjP engB %d %d %d %d %d",
	    pe->IChangeByBurstEn, pe->pfDutyB, pe->pmfStepB, pe->mfDutyMinB, pe->mfDutyMaxB);

	pct = &pp->coolTimeOutPara;
	LogInfo("prjP ct %d", pct->tabNum);
	LogInfo("prjP ct_id %d %d %d %d %d %d %d %d %d %d",
    	pct->tabId[0],	pct->tabId[1],	pct->tabId[2],	pct->tabId[3],	pct->tabId[4],
    	pct->tabId[5],	pct->tabId[6],	pct->tabId[7],	pct->tabId[8],	pct->tabId[9]);

    LogInfo("prjP ct_clx100 %d %d %d %d %d %d %d %d %d %d",
    	(int)(pct->coolingTM[0]*100),(int)(pct->coolingTM[1]*100),(int)(pct->coolingTM[2]*100),(int)(pct->coolingTM[3]*100),(int)(pct->coolingTM[4]*100),
    	(int)(pct->coolingTM[5]*100),(int)(pct->coolingTM[6]*100),(int)(pct->coolingTM[7]*100),(int)(pct->coolingTM[8]*100),(int)(pct->coolingTM[9]*100));

    LogInfo("prjP ct_to %d %d %d %d %d %d %d %d %d %d",
    	pct->timOutMs[0],	pct->timOutMs[1],	pct->timOutMs[2],	pct->timOutMs[3],	pct->timOutMs[4],
    	pct->timOutMs[5],	pct->timOutMs[6],	pct->timOutMs[7],	pct->timOutMs[8],	pct->timOutMs[9]);

    LogInfo("prjP oth %d %d %d %d", pp->maxCapExpTimeUs, pp->pfExpFollowPline, pp->maxPfAfe, pp->maxAfeGain);

}
int FlashMgr::start()
{
	debugCnt();

	int isFlash;
	if(eAppMode_ZsdMode==m_camMode)
	{
		isFlash = m_flashOnPrecapture;
	}
	else
	{
	 	isFlash = isNeedFiringFlash();
	}
	AeMgr::getInstance().setStrobeMode(m_flashOnPrecapture);
	if(m_bRunPreFlash==0)
		return 0;

    NVRAM_CAMERA_STROBE_STRUCT* pNv;
    nvGetBuf(pNv);
	FLASH_PROJECT_PARA prjPara;
	int aeMode;
	aeMode = AeMgr::getInstance().getAEMode();
	prjPara = getFlashProjectPara(aeMode);
	logProjectPara(&prjPara);


	//log eng
	int i;
	int j;
	LogInfo("logEng dutyN=%d stepN=%d", prjPara.dutyNum, prjPara.stepNum);
	if(prjPara.stepNum<1 || prjPara.stepNum>8)
	{
		LogErr("!stepN is wrong! valid:[1,8]");
	}
	else
	{
		LogInfo("Test stepN is ok");
	}

	if(prjPara.dutyNum<1 || prjPara.dutyNum>32)
	{
		LogErr("!dutyN is wrong! valid:[1,32]");
	}
	else
	{
		LogInfo("Test dutyN is ok");
	}

	char* chTemp;
	char ch[50];
	chTemp = new char[1024];
	int ii=0;
	for(j=0;j<prjPara.stepNum;j++)
	{
		chTemp[0]=0;
		sprintf(chTemp, "[%d] ",j);
		for(i=0;i<prjPara.dutyNum;i++)
		{
			sprintf(ch,"%5.2f ",(float)pNv->engTab.yTab[ii]);
			ii++;
			strcat(chTemp,ch);
		}
		LogInfo("%s",chTemp);
	}

	//eng test
	ii=0;
	for(j=0;j<prjPara.stepNum;j++)
	{
		ii++;
		int bEngErr=0;
		for(i=1;i<prjPara.dutyNum;i++)
		{
			if(pNv->engTab.yTab[ii]<pNv->engTab.yTab[ii-1])
			{
				bEngErr=1;
			}
			ii++;
		}
		if(bEngErr==1)
			LogErr("EngTab is wrong! step=%d, value are not inc!",j);
		else
			LogInfo("Test EngTab is ok (step=%d)",j);
	}
	delete []chTemp;




	LogInfo("start() nvram_tar = %d %d %d %d aemode=%d",
		pNv->tuningPara[0].yTar,
		pNv->tuningPara[1].yTar,
		pNv->tuningPara[2].yTar,
		pNv->tuningPara[3].yTar,
		aeMode
		);


	LogInfo("start() line=%d ytar=%d",__LINE__,prjPara.tuningPara.yTar);
	LogInfo("start() line=%d engLevel.vBatL=%d", __LINE__, prjPara.engLevel.vBatL);



	m_iteration=0;
	FlashAlg* pStrobeAlg;
	pStrobeAlg = FlashAlg::getInstance();
	pStrobeAlg->Reset();
	//pf pline
	hw_setPfPline(pStrobeAlg);
	//cap pline
	hw_setCapPline(&prjPara, pStrobeAlg);
	//flash profile
	hw_setFlashProfile(pStrobeAlg, &prjPara, pNv);
	//preference
	hw_setPreference(pStrobeAlg, &prjPara);
	//
	int err;
	err = pStrobeAlg->checkInputParaError(0,0);
	if(err!=0)
	{
		addErr(err);
		LogError("checkInputParaError err=%d", err);
		m_flashOnPrecapture=0;
   		m_db.endTime = FlashUtil::getMs();
   		return 0;
	}
	FlashAlgExpPara aePara;

	hw_getAEExpPara(&aePara);
	pStrobeAlg->CalFirstEquAEPara(&aePara, &g_expPara);
	//set exp
	hw_setExpPara(&g_expPara, m_sensorType, &prjPara);
	return 0;
}
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
int FlashMgr::run(FlashExePara* para, FlashExeRep* rep)
{
LogInfo("run() line=%d",__LINE__);
	FLASH_PROJECT_PARA prjPara;
	int aeMode;
	aeMode = AeMgr::getInstance().getAEMode();
	prjPara= getFlashProjectPara(aeMode);


	FlashAlg* pStrobeAlg;
	pStrobeAlg = FlashAlg::getInstance();
	FlashAlgStaData staData;
	short g_data2[40*30*2];
	staData.data = g_data2;

MUINT32 u4FlashResultWeight;

	//convert flash3A
	hw_convert3ASta(&staData, para->staBuf);


	int isNext;
	FlashAlgExpPara paraNext;
	pStrobeAlg->AddStaData10(&staData, &g_expPara, &isNext, &paraNext);


	g_expPara = paraNext;

	m_iteration++;
	if(m_iteration>10 || isNext==0)
	{
		rep->isEnd=1;
		LogInfo("Estimate+");
		pStrobeAlg->Estimate(&g_expPara);
		LogInfo("Estimate-");


		int afe;
		int isp;
		hw_capIsoToGain(g_expPara.iso, &afe, &isp);

		rep->nextAfeGain = afe;
		rep->nextIspGain = isp;
		rep->nextExpTime = g_expPara.exp;
		rep->nextIsFlash = g_expPara.isFlash;
		rep->nextDuty = g_expPara.duty;
		rep->nextStep = g_expPara.step;
		rep->flashAwbWeight = u4FlashResultWeight;
		//hw_turnOffFlash();
		hw_setCapExpPara(&g_expPara);

		m_thisFlashDuty=  g_expPara.duty;
		m_thisFlashStep= g_expPara.step;
		m_thisIsFlashEn=g_expPara.isFlash;

		m_db.capIso = g_expPara.iso;
		hw_capIsoToGain(m_db.capIso, &m_db.capAfeGain, &m_db.capIspGain);
		m_db.capExp = g_expPara.exp;
		m_db.capDuty = g_expPara.duty;
		m_db.capStep = g_expPara.step;

		m_db.endTime = FlashUtil::getMs();
	}
	else
	{
		rep->isEnd=0;
		rep->nextIsFlash = g_expPara.isFlash;
		hw_setExpPara(&g_expPara, m_sensorType, &prjPara);
	}
LogInfo("run() line=%d isEnd=%d",__LINE__,rep->isEnd);

	return 0;
}
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
int FlashMgr::getAfLampMode()
{
	return m_afLampMode;
}

int FlashMgr::setEvComp(int ind, float ev_step)
{
	LogInfo("setEvComp ind=%d evs=%f",ind,ev_step);
	m_evComp = ind*ev_step;
	return 0;

}
int FlashMgr::setAfLampMode(int mode)
{
	if(mode<AF_LAMP_BEGIN || mode>(NUM_OF_AF_LAMP-1))
		return FL_ERR_FlashModeNotSupport;
	m_afLampMode=mode;
	return 0;

}
int FlashMgr::getFlashMode()
{
	return m_flashMode;
}
int FlashMgr::getCamMode()
{
	return m_camMode;
}
int FlashMgr::setCamMode(int mode)
{
	LogInfo("setCamMode mode=%d",mode);
	m_camMode = mode;
	return 0;
}
int FlashMgr::setFlashMode(int mode)
{
	LogInfo("setFlashMode+ mode=%d",mode);
	LogInfo("OA XO XX OO OT TO TT");


	int e=0;
	int err=0;

	if(mode<LIB3A_FLASH_MODE_MIN || mode>LIB3A_FLASH_MODE_MAX)
	{
	    LogInfo("setFlashMode-");
		return FL_ERR_FlashModeNotSupport;
	}

	if(m_isAfState==1)
	{
		m_flashMode = mode;
		LogInfo("setFlashMode-");
		return 0;
	}

    int fmode;
    int fstyle;
    fmode = mode;
    if(g_previewMode==e_Capture)
    {
        LogInfo("capturing");
        LogInfo("setFlashMode-");
        return 0;

    }
    else if(g_previewMode==e_NonePreview)
    {
        LogInfo("nonepreview");
        if(mode==LIB3A_FLASH_MODE_FORCE_TORCH)
        {
            LogInfo("torch");
            g_pStrobe = StrobeDrv::createInstance();
            g_pStrobe->initTemp(m_sensorType);
            turnOnTorch();
        }
        else
        {
            LogInfo("off");
            g_pStrobe = StrobeDrv::createInstance();
            g_pStrobe->initTemp(m_sensorType);
            turnOffFlashDevice();
        }
    }
    else if(g_previewMode==e_CapturePreview)
    {
        fstyle = getFlashModeStyle(m_sensorType, fmode);
        LogInfo("capPrv m,s=%d,%d",fmode,fstyle);
        if(fstyle==e_FLASH_STYLE_ON_ON || fstyle==e_FLASH_STYLE_ON_TORCH)
        {
            turnOnTorch();
        }
        else
        {
            turnOffFlashDevice();
        }
    }
    else if(g_previewMode==e_VideoPreview)
    {
        fstyle = getVideoFlashModeStyle(m_sensorType, fmode);
        LogInfo("videoPrv m,s=%d,%d",fmode,fstyle);
        if(fstyle==e_FLASH_STYLE_ON_ON || fstyle==e_FLASH_STYLE_ON_TORCH)
        {
            turnOnTorch();
        }
        else //if(fstyle==e_FLASH_STYLE_OFF_OFF || fstyle==e_FLASH_STYLE_OFF_ON || fstyle==e_FLASH_STYLE_OFF_AUTO)
	{
            turnOffFlashDevice();
        }
    }
    else if(g_previewMode==e_VideoRecording)
    {
        fstyle = getVideoFlashModeStyle(m_sensorType, fmode);
        LogInfo("vidoR m,s=%d,%d",fmode,fstyle);
        if(fstyle==e_FLASH_STYLE_ON_ON || fstyle==e_FLASH_STYLE_ON_TORCH || fstyle==e_FLASH_STYLE_OFF_ON)
		{
            turnOnTorch();
		}
        else if(fstyle==e_FLASH_STYLE_OFF_OFF)
		{
            turnOffFlashDevice();
        }
		}
    else
		{
        LogError("preview mode is wrong");
		}
		m_flashMode = mode;
	LogInfo("setFlashMode-");
		return 0;

}
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
int FlashMgr::isFlashOnCapture()
{
	LogInfo("isFlashOnCapture() line=%d isFlashOnCapture=%d",__LINE__,m_flashOnPrecapture);

	return m_flashOnPrecapture;
}

float interpCoolTM_index(int ind, int tabNum, int* xTab, float* tmTab)
{
	int* yTab;
	int y;
	yTab = new int[tabNum];
	int i;
	for(i=0;i<tabNum;i++)
	{
		yTab[i] = (int)(tmTab[i]*1024);
	}
	y = FlashUtil::flash_calYFromXYTab(tabNum, xTab, yTab, ind);

	delete []yTab;
	return (float)(y/1024.0);
}
int interpTimeOut_index(int ind, int tabNum, int* xTab, int* tTab)
{
	int y;
	y = FlashUtil::flash_calYFromXYTab(tabNum, xTab, tTab, ind);
	return y;
}
#ifdef CCT_TEST
    void testCctNv();
#endif

int FlashMgr::capCheckAndFireFlash_Start()
{
#ifdef CCT_TEST
    testCctNv();
#endif
    g_previewMode=e_Capture;

    if(g_eg_bUserMf==1)
{
        int tOut;
		FLASH_PROJECT_PARA prjPara;
		prjPara = getAutoProjectPara();
		tOut = interpTimeOut_index(g_eg_mfDuty, prjPara.coolTimeOutPara.tabNum, prjPara.coolTimeOutPara.tabId, prjPara.coolTimeOutPara.timOutMs);
        g_pStrobe = StrobeDrv::createInstance();
        hwSetFlashOff();
		if(m_db.thisTimeOutTime == ENUM_FLASH_TIME_NO_TIME_OUT)
			g_pStrobe->setTimeOutTime(0);
		else
			g_pStrobe->setTimeOutTime(tOut);
		g_pStrobe->setDuty(g_eg_mfDuty);
		g_pStrobe->setStep(g_eg_mfStep);
		hwSetFlashOn();
        return 0;
    }



    int flashMode;
    int flashStyle;
    flashMode = getFlashMode();
    flashStyle = getFlashModeStyle(m_sensorType, flashMode);
    LogInfo("capCheckAndFireFlash_Start2 %d %d",flashMode,flashStyle);
    if(flashStyle==(int)e_FLASH_STYLE_OFF_OFF)
	{
	    turnOffFlashDevice();
	    return 0;
    }
	else if(flashStyle==(int)e_FLASH_STYLE_ON_TORCH)
	{
	    turnOnTorch();
	return 0;
}

	LogInfo("capCheckAndFireFlash_Start line=%d  getMs=%d",__LINE__, FlashUtil::getMs());
	m_isCapFlashEndTimeValid=0;
	int propOn;
	int propDuty;
	int propStep;
	propOn = FlashUtil::getPropInt(PROP_MF_ON_STR, -1);
	propDuty = FlashUtil::getPropInt(PROP_MF_DUTY_STR, -1);
	propStep = FlashUtil::getPropInt(PROP_MF_STEP_STR, -1);
	if(propOn!=-1)
		m_flashOnPrecapture=propOn;
	if(propDuty!=-1)
	{
		g_expPara.duty=propDuty;
		m_db.capDuty=propDuty;
	}
	if(propStep != -1)
	{
		g_expPara.step=propStep;
		m_db.capStep=propStep;
	}


	if(m_cct_isUserDutyStep==1)
	{
		g_expPara.duty=m_cct_capDuty;
		g_expPara.step=m_cct_capStep;
		m_db.capDuty=m_cct_capDuty;
		m_db.capStep=m_cct_capStep;
		m_flashOnPrecapture=1;
	}

	LogInfo("cap mfexp %d %d %d",m_flashOnPrecapture, m_db.capDuty, m_db.capStep);


	if(m_flashOnPrecapture==1)
	{

		int timeOutTime;
		m_db.thisFireStartTime = FlashUtil::getMs();
		//@@ set timeout, percentage, current timeout, tm
		int aeMode;
		FLASH_PROJECT_PARA prjPara;
		prjPara = getAutoProjectPara();


			timeOutTime = interpTimeOut_index(m_db.capDuty, prjPara.coolTimeOutPara.tabNum, prjPara.coolTimeOutPara.tabId, prjPara.coolTimeOutPara.timOutMs);
			m_db.thisTimeOutTime = timeOutTime;
			m_db.coolingTM = interpCoolTM_index(m_db.capDuty, prjPara.coolTimeOutPara.tabNum, prjPara.coolTimeOutPara.tabId, prjPara.coolTimeOutPara.coolingTM);



		int i;
		for(i=0;i<prjPara.coolTimeOutPara.tabNum;i++)
		{
			LogInfo("cap coolTimeOut\t%d\t%d\t%d", prjPara.coolTimeOutPara.tabId[i], prjPara.coolTimeOutPara.coolingTM[i], prjPara.coolTimeOutPara.timOutMs[i]);
		}

		g_pStrobe = StrobeDrv::createInstance();

		if(m_db.thisTimeOutTime == ENUM_FLASH_TIME_NO_TIME_OUT)
			g_pStrobe->setTimeOutTime(0);
		else
			g_pStrobe->setTimeOutTime(m_db.thisTimeOutTime);
		g_pStrobe->setDuty(g_expPara.duty);
		g_pStrobe->setStep(g_expPara.step);
		hwSetFlashOn();

	}
	return 0;

}

int FlashMgr::capCheckAndFireFlash_End()
{
	LogInfo("capCheckAndFireFlash_End line=%d  getMs=%d",__LINE__, FlashUtil::getMs());
	if(g_eg_bUserMf==1)
    {
        g_pStrobe = StrobeDrv::createInstance();
        g_pStrobe->setTimeOutTime(500);
        hwSetFlashOff();
        return 0;
    }

	int flashMode;
    int flashStyle;
    flashMode = getFlashMode();
    flashStyle = getFlashModeStyle(m_sensorType, flashMode);
    LogInfo("capCheckAndFireFlash_End %d %d",flashMode,flashStyle);
    if(flashStyle==(int)e_FLASH_STYLE_OFF_OFF)
	{
	    turnOffFlashDevice();
	    return 0;
	}
	else if(flashStyle==(int)e_FLASH_STYLE_ON_TORCH)
	{
	    turnOnTorch();
	    return 0;
	}


	if(m_cct_isUserDutyStep==1)
		turnOffFlashDevice();
	if(isBurstShotMode()==1 )
	{
	}
	else if(getFlashMode()==FLASHLIGHT_FORCE_ON && getFlashModeStyle(m_sensorType, FLASHLIGHT_FORCE_ON)==(int)e_FLASH_STYLE_ON_ON)
	{
		//flash on
		FLASH_PROJECT_PARA prjPara;

		int aeMode;
		aeMode = AeMgr::getInstance().getAEMode();
			//prjPara = cust_getFlashProjectPara(aeMode, m_pNvram);
		prjPara = getFlashProjectPara(aeMode);
		g_pStrobe = StrobeDrv::createInstance();
			//@@if(current mode)
		LogInfo("setFlashMode mode duty,step=%d %d",prjPara.engLevel.torchDuty, prjPara.engLevel.torchStep);
		g_pStrobe->setDuty(prjPara.engLevel.torchDuty);
		g_pStrobe->setStep(prjPara.engLevel.torchStep);
		g_pStrobe->setTimeOutTime(0);
		hwSetFlashOn();

	}
	else
	{
	    //FLASH OFF
	    turnOffFlashDevice();
    }
    LogInfo("capCheckAndFireFlash_End line=%d  thisFireStartTime & End: %d %d",__LINE__, m_db.thisFireStartTime,m_db.thisFireEndTime);
	return 0;
}

int FlashMgr::videoPreviewStart()
{
    LogInfo("videoPreviewStart+");
    //g_bCapturing=0;


    int flashMode;
    int flashStyle;
    flashMode = getFlashMode();
    flashStyle = getVideoFlashModeStyle(m_sensorType, flashMode);
    LogInfo("videoPreviewStart %d %d",flashMode,flashStyle);

    if(flashStyle==(int)e_FLASH_STYLE_ON_ON)
	{
	    turnOnTorch();

	    m_flashOnPrecapture=1;
	}
	else
	{
	    LogInfo("turn off flash");
		turnOffFlashDevice();

		m_flashOnPrecapture=0;
	}
	g_previewMode=e_VideoPreview;
	LogInfo("videoPreviewStart-");
	return 0;
}

int FlashMgr::videoRecordingStart()
{
    LogInfo("videoRecordingStart+");
    int flashMode;
    int flashStyle;
    flashMode = getFlashMode();
    flashStyle = getVideoFlashModeStyle(m_sensorType, flashMode);
    LogInfo("mode,style: %d %d",flashMode,flashStyle);
    if(flashStyle==(int)e_FLASH_STYLE_ON_ON || flashStyle==(int)e_FLASH_STYLE_OFF_ON)
    {
        LogInfo("on style");
        turnOnTorch();
        m_flashOnPrecapture=1;
    }
    else if(flashStyle==(int)e_FLASH_STYLE_OFF_AUTO)
    {
        LogInfo("auto style");
        if(AeMgr::getInstance().IsStrobeBVTrigger()==1)
        {
            LogInfo("triger");
            turnOnTorch();
            m_flashOnPrecapture=1;
        }
        else
        {
            LogInfo("not triger");
            m_flashOnPrecapture=0;
        }
    }
    else
	{
	    m_flashOnPrecapture=0;
		LogInfo("off style");
	}
	g_previewMode=e_VideoRecording;
    LogInfo("videoRecordingStart-");
    return 0;
}

int FlashMgr::videoRecordingEnd()
{
    LogInfo("videoRecordingEnd+");
    int flashMode;
    int flashStyle;
    flashMode = getFlashMode();
    flashStyle = getVideoFlashModeStyle(m_sensorType, flashMode);
    LogInfo("mode,style: %d %d",flashMode,flashStyle);
    if(flashStyle==(int)e_FLASH_STYLE_OFF_ON || flashStyle==(int)e_FLASH_STYLE_OFF_OFF || flashStyle==(int)e_FLASH_STYLE_OFF_AUTO)
    {
        LogInfo("off style");
        turnOffFlashDevice();
    }
    else
	{
	    LogInfo("on style");

	}

    g_previewMode=e_VideoPreview;
    LogInfo("videoRecordingEnd-");
    return 0;
}

int FlashMgr::capturePreviewStart()
{
    LogInfo("capturePreviewStart+");
    //g_bCapturing=0;
    int fmode;
    int fstyle;
    fmode = getFlashMode();
    fstyle = getFlashModeStyle(m_sensorType, fmode);
    if(fstyle==(int)e_FLASH_STYLE_ON_ON || fstyle==(int)e_FLASH_STYLE_ON_TORCH)
	{
		turnOnTorch();
	}
	else
	{
		turnOffFlashDevice();
	}
	g_previewMode=e_CapturePreview;
	m_flashOnPrecapture=0;
	LogInfo("capturePreviewStart-");
	return 0;
}

int FlashMgr::capturePreviewEnd()
{
    LogInfo("capturePreviewEnd+");
    /*
    int fmode;
    int fstyle;
    fmode = getFlashMode();
    fstyle = getFlashModeStyle(m_sensorType, fmode);
    if(fstyle==(int)e_FLASH_STYLE_ON_ON || fstyle==(int)e_FLASH_STYLE_ON_TORCH)
	{
		turnOnTorch();
	}
	else
	{
		turnOffFlashDevice();
	}*/
	//g_previewMode=e_NonePreview;
	//turnOffFlashDevice();
	LogInfo("capturePreviewEnd-");
	return 0;
}
int FlashMgr::videoPreviewEnd()
{
    LogInfo("videoPreviewEnd+");
    /*
    int fmode;
    int fstyle;
    fmode = getFlashMode();
    fstyle = getFlashModeStyle(m_sensorType, fmode);
    if(fstyle==(int)e_FLASH_STYLE_ON_ON || fstyle==(int)e_FLASH_STYLE_ON_TORCH)
	{
		turnOnTorch();
	}
	else
	{
		turnOffFlashDevice();
	}
	g_previewMode=e_CapturePreview;
	*/
	g_previewMode=e_NonePreview;
	turnOffFlashDevice();
	LogInfo("videoPreviewEnd-");
	return 0;
}

int FlashMgr::notifyAfEnter()
{
	LogInfo("notifyAfEnter");
	m_isAfState=1;
	return 0;
}

int FlashMgr::notifyAfExit()
{
	LogInfo("notifyAfExit");
	m_isAfState=0;
	return 0;
}

int FlashMgr::turnOffFlashDevice()
{
	LogInfo("%s line=%d",__FUNCTION__,__LINE__);
	if(m_flashOnPrecapture==1 && m_isCapFlashEndTimeValid==0)
	{



		m_isCapFlashEndTimeValid=1;
		g_pStrobe = StrobeDrv::createInstance();
		hwSetFlashOff();
		m_thisIsFlashEn=0;
		g_pStrobe->setTimeOutTime(1000);
		int ms;
		ms = FlashUtil::getMs();
		if(m_db.thisTimeOutTime !=0)
		{
			if(ms - m_db.thisFireStartTime - m_db.thisTimeOutTime > 0)
			{
				m_db.thisFireEndTime = m_db.thisFireStartTime + m_db.thisTimeOutTime;
LogWarning("capture flash timeout line=%d",__LINE__);
			}
			else
			{
				m_db.thisFireEndTime = ms;
			}
		}
		else
		{
			m_db.thisFireEndTime = ms;
		}
	}
	g_pStrobe = StrobeDrv::createInstance();
	hwSetFlashOff();
	g_pStrobe->setTimeOutTime(1000);
	LogInfo("turnOffFlashDevice-thisFireEndTime=%d",m_db.thisFireEndTime);
	return 0;
}
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
int FlashMgr::end()
{
	return 0;
}
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
void AETableLim(strAETable* pAE, int maxExpLim)
{
	int maxAfe=0;
	int maxIsp=0;
	int i;
	for(i=0;i<(int)pAE->u4TotalIndex;i++)
	{
		if((int)pAE->pCurrentTable[i].u4AfeGain>maxAfe)
			maxAfe = pAE->pCurrentTable[i].u4AfeGain;

		if((int)pAE->pCurrentTable[i].u4IspGain>maxIsp)
			maxIsp = pAE->pCurrentTable[i].u4IspGain;
	}

//pline copy
	ClearAePlineEvSetting();
	g_plineEvSetting = new strEvSetting[pAE->u4TotalIndex+20];
	for(i=0;i<(int)pAE->u4TotalIndex;i++)
	{
		g_plineEvSetting[i] = pAE->pCurrentTable[i];
	}

	int LastIndOri= (int)pAE->u4TotalIndex-1;
	for(i=(int)pAE->u4TotalIndex;i<(int)pAE->u4TotalIndex+20;i++)
		g_plineEvSetting[i] = pAE->pCurrentTable[LastIndOri];

	pAE->u4TotalIndex = pAE->u4TotalIndex+20;
	pAE->pCurrentTable = g_plineEvSetting;



	for(i=0;i<(int)pAE->u4TotalIndex;i++)
	{
		if((int)pAE->pCurrentTable[i].u4Eposuretime>maxExpLim)
		{
			float r;
			int afe = pAE->pCurrentTable[i].u4AfeGain;
			int isp = pAE->pCurrentTable[i].u4IspGain;

			r = (float)pAE->pCurrentTable[i].u4Eposuretime/maxExpLim;
			if(r < (float)maxAfe/afe)
				pAE->pCurrentTable[i].u4AfeGain *= r;
			else if(r < (float)maxAfe*maxIsp/afe/isp)
			{
				pAE->pCurrentTable[i].u4AfeGain = maxAfe;
				pAE->pCurrentTable[i].u4IspGain *= r/ ((float)maxAfe/afe);
			}
			else
			{
				pAE->pCurrentTable[i].u4AfeGain = maxAfe;
				pAE->pCurrentTable[i].u4IspGain = maxIsp;
			}
			pAE->pCurrentTable[i].u4Eposuretime = maxExpLim;maxExpLim;
		}

	}


	i=pAE->u4TotalIndex-1;
	LogInfo("AETableLim=%d %d %d",pAE->pCurrentTable[i].u4Eposuretime, pAE->pCurrentTable[i].u4AfeGain, pAE->pCurrentTable[i].u4IspGain);

/*
	FILE* fp;
	fp = fopen("/sdcard/flpline.txt", "at");
	fprintf(fp,"===\n");
	fprintf(fp,"total index = %d\n", pAE->u4TotalIndex);
	for(i=0;i<(int)pAE->u4TotalIndex;i++)
		fprintf(fp,"%d\t%d\t%d\n",pAE->pCurrentTable[i].u4Eposuretime, pAE->pCurrentTable[i].u4AfeGain, pAE->pCurrentTable[i].u4IspGain);
	fclose(fp);
	*/

	int maxExtGain;
	maxExtGain = FlashUtil::getPropInt(PROP_MF_PLINE_EXTEND_GAIN, 0);
	if(maxExtGain!=0)
	{
		double baseExpP;
		double calExpP;
		double expP;
		int indBase=20;
		baseExpP = (double)pAE->pCurrentTable[indBase].u4Eposuretime*pAE->pCurrentTable[indBase].u4AfeGain*pAE->pCurrentTable[indBase].u4IspGain;
		for(i=0;i<(int)pAE->u4TotalIndex;i++)
		{
			if(i>indBase)
			{
				calExpP = baseExpP*pow(2.0, (i-20.0)/10.0);
				expP = pAE->pCurrentTable[i].u4Eposuretime*pAE->pCurrentTable[i].u4AfeGain*pAE->pCurrentTable[i].u4IspGain;
				if(expP/calExpP<0.90)
				{
					double gtest;
					gtest = calExpP/pAE->pCurrentTable[i].u4Eposuretime/1024.0;
					if(gtest>maxExtGain)
						gtest=maxExtGain;
					pAE->pCurrentTable[i].u4IspGain = gtest/pAE->pCurrentTable[i].u4AfeGain*1024.0;
	}
}
}
	}


	/*
	FILE* fp2;
	fp2 = fopen("/sdcard/flpline.txt", "at");
	fprintf(fp2,"===\n");
	fprintf(fp2,"total index = %d\n", pAE->u4TotalIndex);
	for(i=0;i<(int)pAE->u4TotalIndex;i++)
		fprintf(fp2,"%d\t%d\t%d\n",pAE->pCurrentTable[i].u4Eposuretime, pAE->pCurrentTable[i].u4AfeGain, pAE->pCurrentTable[i].u4IspGain);
	fclose(fp2);
	*/





}
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

static void PLineTrans(PLine* p, strAETable* pAE)
{
	p->i4MaxBV=pAE->i4MaxBV;
	p->i4MinBV=pAE->i4MinBV;
	p->u4TotalIndex=pAE->u4TotalIndex;
	p->i4StrobeTrigerBV=pAE->i4StrobeTrigerBV;
	int i;
	p->pCurrentTable = new evSetting[pAE->u4TotalIndex];
	for(i=0;i<(int)pAE->u4TotalIndex;i++)
	{
		p->pCurrentTable[i].u4Eposuretime = pAE->pCurrentTable[i].u4Eposuretime;
		p->pCurrentTable[i].u4AfeGain = pAE->pCurrentTable[i].u4AfeGain;
		p->pCurrentTable[i].u4IspGain = pAE->pCurrentTable[i].u4IspGain;
		p->pCurrentTable[i].uIris = pAE->pCurrentTable[i].uIris;
		p->pCurrentTable[i].uSensorMode = pAE->pCurrentTable[i].uSensorMode;
		p->pCurrentTable[i].uFlag = pAE->pCurrentTable[i].uFlag;
	}
}


//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
static void PLineClear(PLine* p)
{
	delete []p->pCurrentTable;

}
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
#ifdef CCT_TEST
    #include "flash_cct_test.cpp"
#endif


NVRAM_CAMERA_STROBE_STRUCT* nv_getMainPointer()
{
    static NVRAM_CAMERA_STROBE_STRUCT buf;
    return &buf;
}
NVRAM_CAMERA_STROBE_STRUCT* nv_getSubPointer()
{
    static NVRAM_CAMERA_STROBE_STRUCT buf;
    return &buf;
}

NVRAM_CAMERA_STROBE_STRUCT* nv_getPointer(int sensorType)
{
	if(sensorType==DUAL_CAMERA_MAIN_SENSOR)
	{
	    return nv_getMainPointer();
	}
	else
	{
	    return nv_getSubPointer();
	}
	return 0;
}
int nv_forceRead(int sensorId)
{
	LogInfo("nv_forceRead sensorType=%d", sensorId);
	//get mem
	NVRAM_CAMERA_STROBE_STRUCT* pMem;
	pMem = (NVRAM_CAMERA_STROBE_STRUCT*)nv_getPointer(sensorId);
	//read
	NvramDrvBase* nvDrv = NvramDrvBase::createInstance();
	nvDrv->readNvram( (CAMERA_DUAL_CAMERA_SENSOR_ENUM)sensorId, 0, CAMERA_NVRAM_DATA_STROBE, pMem, sizeof(NVRAM_CAMERA_STROBE_STRUCT));
	return 0;
}
int nv_getBuf(int sensorId, NVRAM_CAMERA_STROBE_STRUCT*& buf)
{
	int isRead;
	if(sensorId==DUAL_CAMERA_MAIN_SENSOR)
	{
		static int isReadMain=0;
		isRead = isReadMain;
		isReadMain=1;
	}
	else
	{
		static int isReadSub=0;
		isRead = isReadSub;
		isReadSub=1;
	}
	if(isRead==0)
		nv_forceRead(sensorId);
    buf = nv_getPointer(sensorId);
	return 0;
}
int nv_write(int sensorId)
{
	//get mem
	NVRAM_CAMERA_STROBE_STRUCT* pMem;
	pMem = (NVRAM_CAMERA_STROBE_STRUCT*)nv_getPointer(sensorId);
	//write
	NvramDrvBase* nvDrv = NvramDrvBase::createInstance();
	nvDrv->writeNvram( (CAMERA_DUAL_CAMERA_SENSOR_ENUM)sensorId, 0, CAMERA_NVRAM_DATA_STROBE, pMem, sizeof(NVRAM_CAMERA_STROBE_STRUCT));
	return 0;
}


int FlashMgr::nvForceRead()
{
    return nv_forceRead(m_sensorType);
}

int FlashMgr::nvGetBuf(NVRAM_CAMERA_STROBE_STRUCT*& buf)
{
  int e;
  e = nv_getBuf(m_sensorType, buf);
  LogInfo("nvGetBuf-mfdutymax %d\n", buf->engLevel.mfDutyMax);
  LogInfo("nvGetBuf-mfdutymax %d\n", buf->engLevel.mfDutyMax);
  return e;
}

int FlashMgr::nvWrite()
{
    int e;
    e = nv_write(m_sensorType);
	return e;
}

int FlashMgr::nvReadDefault()
{
    LogInfo("loadDefaultNvram()");

    NVRAM_CAMERA_STROBE_STRUCT* pNv;
    pNv =  nv_getPointer(m_sensorType);

	int sz;
	getDefaultStrobeNVRam(m_sensorType, pNv, &sz);

	LogInfo("loadDefaultNvram engTab[0,1,2,3,4,5,6,7,15,23,31] %d %d %d %d %d %d %d %d %d %d %d\n",
	pNv->engTab.yTab[0],	pNv->engTab.yTab[1],	pNv->engTab.yTab[2],	pNv->engTab.yTab[3],	pNv->engTab.yTab[4],	pNv->engTab.yTab[5],	pNv->engTab.yTab[6],	pNv->engTab.yTab[7],	pNv->engTab.yTab[15],	pNv->engTab.yTab[23],	pNv->engTab.yTab[31]);

	LogInfo("loadDefaultNvram-engTab[32,39,47,54,63] %d %d %d %d %d\n",
	pNv->engTab.yTab[32], pNv->engTab.yTab[39], pNv->engTab.yTab[47], pNv->engTab.yTab[54], pNv->engTab.yTab[63]);

	LogInfo("loadDefaultNvram-mfdutymax %d\n", pNv->engLevel.mfDutyMax);
	LogInfo("loadDefaultNvram-mfdutymax %d\n", pNv->engLevel.mfDutyMax);

    return 0;
}

int FlashMgr::egSetMfDutyStep(int duty, int step)
{
    g_eg_mfDuty=duty;
    g_eg_mfStep=step;
    return 0;
}
int FlashMgr::egGetDutyRange(int* st, int* ed)
{
    FLASH_PROJECT_PARA prjPara;
	prjPara = FlashMgr::getInstance()->getAutoProjectPara();
	*st = 0;
	*ed = prjPara.dutyNum-1;
	LogInfo("egGetDutyRange %d %d\n", *st, *ed);
	return 0;
}
int FlashMgr::egGetStepRange(int* st, int* ed)
{
    FLASH_PROJECT_PARA prjPara;
	prjPara = FlashMgr::getInstance()->getAutoProjectPara();
	*st = 0;
	*ed = prjPara.stepNum-1;
	LogInfo("egGetStepRange %d %d\n", *st, *ed);
	return 0;
}


void FlashMgr::setPostFlashFunc(void (* pFunc)(int en))
{
    g_pPostFunc=pFunc;
}

void FlashMgr::setPreFlashFunc(void (* pFunc)(int en))
{
    g_pPreFunc=pFunc;
}
void FlashMgr::setCapPara()
{
	 	int fmode;
    int fstyle;
    fmode = getFlashMode();
    fstyle = getFlashModeStyle(m_sensorType, fmode);    
    if(fstyle!=e_FLASH_STYLE_ON_TORCH)
    {        
    hw_setCapExpPara(&g_expPara);
}
}


void FlashMgr::setPfParaToAe()
{
    //g_pfExpPara

    strAETable pfPlineTab;
	strAETable capPlineTab;
	strAFPlineInfo pfPlineInfo;
	AE_DEVICES_INFO_T devInfo;
	AeMgr::getInstance().getCurrentPlineTable(pfPlineTab, capPlineTab, pfPlineInfo);
	AeMgr::getInstance().getSensorDeviceInfo(devInfo);




    int exp;
    int afe;
    int isp;
    exp = g_pfExpPara.exp;
    afe = g_pfExpPara.afeGain;
    isp = g_pfExpPara.ispGain;
    double expLvTar;
    double expLv;
    double err;
    double minErr=0;

    expLvTar = (double)exp*afe*isp/1000000.0;
    XLOGD("exposure time22, afe, isp %d %d %d %lf", exp, afe, isp,expLvTar);
    int sz;
    int bestInd;
    sz = (int)pfPlineTab.u4TotalIndex;
    int i;

    bestInd=0;
    if(expLvTar==0)
        bestInd=0;
    else
    {
        for(i=0;i<sz;i++)
        {
            exp = pfPlineTab.pCurrentTable[i].u4Eposuretime;
            afe = pfPlineTab.pCurrentTable[i].u4AfeGain;
            isp = pfPlineTab.pCurrentTable[i].u4IspGain;
            expLv = (double)exp*afe*isp/1000000.0;;

            //XLOGD("%5.3lf", expLv);
            if(i==0)
            {
                minErr=expLv/expLvTar-1;

                //XLOGD("min0 %5.3lf %5.3lf %5.3lf", expLv, expLvTar, minErr);

                if(minErr<0)
                    minErr=-minErr;

                //XLOGD("min0 %lf", minErr);
            }
            err=expLv/expLvTar-1;
            if(err<0)
                err=-err;
             //XLOGD("err %lf", err);
            if(minErr>err)
            {
                minErr=err;
                bestInd=i;

                //XLOGD("best %d", bestInd);
            }
        }//for(i=0;i<sz;i++)
    }

    exp = pfPlineTab.pCurrentTable[bestInd].u4Eposuretime;
    afe = pfPlineTab.pCurrentTable[bestInd].u4AfeGain;
    isp = pfPlineTab.pCurrentTable[bestInd].u4IspGain;

    AE_MODE_CFG_T previewInfo;
	AeMgr::getInstance().getPreviewParams(previewInfo);
	previewInfo.u4Eposuretime = exp;
	previewInfo.u4AfeGain = afe;
	previewInfo.u4IspGain = isp;
	AeMgr::getInstance().updatePreviewParams(previewInfo, bestInd);

	XLOGD("exposure time, afe, isp %d %d %d, ind=%d", exp, afe, isp, bestInd);

	    //for(i=0;i<(int)pAE->u4TotalIndex;i++)
	    //		p->pCurrentTable[i].u4Eposuretime = pAE->pCurrentTable[i].u4Eposuretime;
		//p->pCurrentTable[i].u4AfeGain = pAE->pCurrentTable[i].u4AfeGain;
		//p->pCurrentTable[i].u4IspGain = pAE->pCurrentTable[i].u4IspGain;
		//p->pCurrentTable[i].uIris = pAE->pCurrentTable[i].uIris;
		//p->pCurrentTable[i].uSensorMode = pAE->pCurrentTable[i].uSensorMode;
		//p->pCurrentTable[i].uFlag = pAE->pCurrentTable[i].uFlag;
}
