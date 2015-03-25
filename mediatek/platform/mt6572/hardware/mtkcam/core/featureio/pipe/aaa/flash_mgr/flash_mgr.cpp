#ifdef WIN32
#include "stdafx.h"
#include "FlashSim.h"
#include "sim_MTKAECommon.h"
#include "sim_MTKAE.h"
#include "FlashAlg.h"
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
#include <ae_algo_if.h>
#include <mtkcam/hal/sensor_hal.h>
#include <nvram_drv_mgr.h>
#include <ae_tuning_custom.h>
#include <isp_mgr.h>
#include <isp_tuning.h>
#include <aaa_sensor_mgr.h>
#include "FlashAlg.h"
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




#define ALG_TAG_SIZE 500
#define A3_DIV_X 120
#define A3_DIV_Y 90
#define Z0_FL_DIV_X 24
#define Z0_FL_DIV_Y 18

#define LogInfo(fmt, arg...) XLOGD(fmt, ##arg)
#define LogVerbose(fmt, arg...) if(g_isVerboseLogEn) LogInfo(fmt, ##arg)
#define LogError(fmt, arg...)   XLOGE("FlashError: func=%s line=%d: "fmt, __FUNCTION__, __LINE__, ##arg)
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
// variable
#ifdef WIN32
#else
StrobeDrv* g_pStrobe;
#endif
FlashAlgExpPara g_expPara;
static int g_isVerboseLogEn=0;
int g_sceneCnt=0;

static strEvSetting* g_plineEvSetting;
//====================================================
// function
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
	g_pStrobe = StrobeDrv::createInstance();
	g_pStrobe->init(1);
	m_flashMode = LIB3A_FLASH_MODE_FORCE_OFF;
	m_flashOnPrecapture = 0;
	m_digRatio = 1;
	m_pNvram =0;
	m_pfFrameCount=0;
	m_thisFlashDuty=-1;
	m_thisFlashStep=-1;
	m_thisIsFlashEn=0;
	m_evComp=0;

	m_isCapFlashEndTimeValid=1;

}
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
FlashMgr::~FlashMgr()
{
	g_pStrobe = StrobeDrv::createInstance();
	g_pStrobe->uninit();

	if(m_pNvram!=0)
		delete []m_pNvram;

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

	LogInfo("m_pNvram engTab[0,1,2,3,4,5,6,7,15,23,31] %d %d %d %d %d %d %d %d %d %d %d\n",
	m_pNvram->engTab.yTab[0],	m_pNvram->engTab.yTab[1],	m_pNvram->engTab.yTab[2],	m_pNvram->engTab.yTab[3],	m_pNvram->engTab.yTab[4],	m_pNvram->engTab.yTab[5],	m_pNvram->engTab.yTab[6],	m_pNvram->engTab.yTab[7],	m_pNvram->engTab.yTab[15],	m_pNvram->engTab.yTab[23],	m_pNvram->engTab.yTab[31]);

	LogInfo("m_pNvram-engTab[32,39,47,54,63] %d %d %d %d %d\n",
	m_pNvram->engTab.yTab[32], m_pNvram->engTab.yTab[39], m_pNvram->engTab.yTab[47], m_pNvram->engTab.yTab[54], m_pNvram->engTab.yTab[63]);


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
	if(pPrjPara->engLevel.pmfEngMode == ENUM_FLASH_ENG_INDEX_MODE)
	{
		 m_db.pfI = pPrjPara->engLevel.pfDuty;
		 m_db.mfIMin = pPrjPara->engLevel.mfDutyMin;
		 m_db.mfIMax = pPrjPara->engLevel.mfDutyMax;
		 m_db.pmfIpeak = pPrjPara->engLevel.pmfStep;
		 m_db.torchIPeak = pPrjPara->engLevel.torchStep;
		 m_db.torchI = pPrjPara->engLevel.torchDuty;
	}
	else   //ENUM_FLASH_ENG_CURRENT_MODE
	{
		m_db.pfI = pPrjPara->engLevel.pfAveI;
		m_db.mfIMin = pPrjPara->engLevel.mfAveIMin;
		m_db.mfIMax = pPrjPara->engLevel.mfAveIMax;
		m_db.pmfIpeak = pPrjPara->engLevel.pmfPeakI;
		m_db.torchIPeak = pPrjPara->engLevel.torchPeakI;
		m_db.torchI = pPrjPara->engLevel.torchAveI;
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
	pStrobeAlg->setOverSuppressionLevel(tune_p.antiOverLevel);
	pStrobeAlg->setForegroundWIncreaseLevel(tune_p.foregroundLevel);

	//evComp = AeMgr::getInstance().getEVCompensateIndex();
	//LogInfo("hw_setPreference() EvComp100(from AE)=%d",evComp);
	//evComp /= 100.0f;
	pStrobeAlg->setEVComp(m_evComp);
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
	g_pStrobe->setOnOff(0);
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
		if(pPrjPara->pfExpFollowPline == 0)
			hw_speedUpExpPara(expPara, pPrjPara->maxPfAfe);
	}
	else if(propFollowPline==0)
	{
		if(propMaxAfe==1)
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
		g_pStrobe->setDuty(duty);
		g_pStrobe->setStep(step);
   		g_pStrobe->setOnOff(1);
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

		calFlashDigWinNum(w, r, &wst, &wdiv, &wbin, 20, 30);
		calFlashDigWinNum(h, r, &hst, &hdiv, &hbin, 15, 25);
		double werr;
		double herr;
		werr = (double)(w/r - wdiv*wbin)/ (w/r);
		herr = (double)(h/r - hdiv*hbin)/ (h/r);
		if(werr>0.1 || werr<-0.1)
		{
			calFlashDigWinNum(w, r, &wst, &wdiv, &wbin, 10, 30);
		}
		if(herr>0.1 || herr<-0.1)
		{
			calFlashDigWinNum(h, r, &hst, &hdiv, &hbin, 10, 25);
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
int FlashMgr::setDigZoom(int digx100)
{
	m_digRatio = digx100/100.0;
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

	int maxIso;
	AE_DEVICES_INFO_T devInfo;
	AeMgr::getInstance().getSensorDeviceInfo(devInfo);

	if(maxAfe!=0)
		hw_gainToIso(devInfo.u4MaxGain, 15*1024, &maxIso);
	else
		hw_gainToIso(maxAfe, 15*1024, &maxIso);

	LogInfo("hw_setExpPara bv=%d",now_bv);
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
	LogInfo("line=%d hw_speedUpExpPara exp=%d iso=%d g=%f mxG=%f",__LINE__, expPara->exp, expPara->iso, g, maxG);
	if(g>maxG)
		g=maxG;
	if(g>1)
	{
		expPara->exp = reducedExp;
		expPara->iso = g* expPara->iso;
	}
	LogInfo("line=%d hw_speedUpExpPara exp=%d iso=%d",__LINE__, expPara->exp, expPara->iso);
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
	m_sensorType = sensorType;
	//forceLoadNvram();
	return 0;
}
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
int FlashMgr::uninit()
{
	return 0;
}
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

void FlashMgr::setFlashOnOff(int en)
{
	g_pStrobe = StrobeDrv::createInstance();
	g_pStrobe->setOnOff(en);
}

int FlashMgr::isAFLampOn()
{
	return m_isAFLampOn;

}
void FlashMgr::setAFLampOnOff(int en)
{
	//@@ af level by current mode
	g_pStrobe = StrobeDrv::createInstance();
	m_isAFLampOn=en;
	if(en==1)
	{
		LogInfo("setAFLampOnOff 1");
		loadNvram();
		FLASH_PROJECT_PARA prjPara;
		prjPara = cust_getFlashProjectPara(LIB3A_AE_MODE_AUTO, m_pNvram);

		int step=0;
		int duty=0;
		if(prjPara.engLevel.afEngMode==ENUM_FLASH_ENG_INDEX_MODE)
		{
			LogInfo("setAFLampOnOff ENUM_FLASH_ENG_INDEX_MODE");
			step = prjPara.engLevel.afStep;
			duty = prjPara.engLevel.afDuty;
		}
		LogInfo("setAFLampOnOff mode=%d duty=%d step=%d",prjPara.engLevel.afEngMode, duty, step);
		g_pStrobe->setTimeOutTime(0);
		g_pStrobe->setStep(step);
		g_pStrobe->setDuty(duty);
	}
	else
	{
		LogInfo("setAFLampOnOff 0");
		g_pStrobe->setTimeOutTime(1000);
	}
	g_pStrobe->setOnOff(en);
}
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

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
		pStrobeAlg->fillDebugData2(algDebug);
		int i;
		for(i=0;i<ALG_TAG_SIZE/4;i++)
		{
			setDebugTag(*p, FL_T_NUM+i, (MUINT32)algDebug[i]);
		}
	}
	return 0;
}
int FlashMgr::writeNvramMain(void* newNvramData)
{
	return writeNvram(DUAL_CAMERA_MAIN_SENSOR, newNvramData);
}
int FlashMgr::writeNvram(int sensorType, void* newNvramData)
{
	LogInfo("writeNvram() line=%d sensorType=%d", __LINE__,sensorType);
	if(m_pNvram==0)
	{
		LogInfo("writeNvram() line=%d", __LINE__);
		m_pNvram=new NVRAM_CAMERA_STROBE_STRUCT[1];
	}
	NvramDrvBase* nvDrv = NvramDrvBase::createInstance();
	unsigned long sz;

	if(newNvramData==0)
	{
		LogInfo("writeNvram() line=%d", __LINE__);
		if(m_pNvram==0)
		{
			LogInfo("writeNvram() line=%d", __LINE__);
			loadNvram();
		}
		LogInfo("loadDefaultNvram engTab[0,1,2,3,4,5,6,7,15,23,31] %d %d %d %d %d %d %d %d %d %d %d\n",
		m_pNvram->engTab.yTab[0],	m_pNvram->engTab.yTab[1],	m_pNvram->engTab.yTab[2],	m_pNvram->engTab.yTab[3],	m_pNvram->engTab.yTab[4],	m_pNvram->engTab.yTab[5],	m_pNvram->engTab.yTab[6],	m_pNvram->engTab.yTab[7],	m_pNvram->engTab.yTab[15],	m_pNvram->engTab.yTab[23],	m_pNvram->engTab.yTab[31]);

		LogInfo("loadDefaultNvram-engTab[32,39,47,54,63] %d %d %d %d %d\n",
		m_pNvram->engTab.yTab[32], m_pNvram->engTab.yTab[39], m_pNvram->engTab.yTab[47], m_pNvram->engTab.yTab[54], m_pNvram->engTab.yTab[63]);

		nvDrv->writeNvram( DUAL_CAMERA_MAIN_SENSOR, 0, CAMERA_NVRAM_DATA_STROBE, m_pNvram, sizeof(NVRAM_CAMERA_STROBE_STRUCT));
	}
	else
	{
		LogInfo("writeNvram() line=%d", __LINE__);
		nvDrv->writeNvram( DUAL_CAMERA_MAIN_SENSOR, 0, CAMERA_NVRAM_DATA_STROBE, newNvramData, sizeof(NVRAM_CAMERA_STROBE_STRUCT));

	}
	return 0;
}

int FlashMgr::forceLoadNvram()
{
	if(m_pNvram==0)
		m_pNvram=new NVRAM_CAMERA_STROBE_STRUCT[1];
	NvramDrvBase* nvDrv = NvramDrvBase::createInstance();
	unsigned long sz;
	nvDrv->readNvram( DUAL_CAMERA_MAIN_SENSOR, 0, CAMERA_NVRAM_DATA_STROBE, m_pNvram, sizeof(NVRAM_CAMERA_STROBE_STRUCT));
	return 0;

}

int FlashMgr::setNvram(void* newNvramData)
{
	if(m_pNvram==0)
		loadNvram();
	memcpy(m_pNvram, newNvramData, sizeof(NVRAM_CAMERA_STROBE_STRUCT));
	return 0;
}

int FlashMgr::getNvram(void* NvramData)
{
	if(m_pNvram==0)
		loadNvram();
	memcpy(NvramData, m_pNvram, sizeof(NVRAM_CAMERA_STROBE_STRUCT));
	return 0;
}

int FlashMgr::loadDefaultNvram()
{
	LogInfo("loadDefaultNvram()");
	if(m_pNvram==0)
	{
		LogInfo("loadDefaultNvram()-NVRAM_CAMERA_STROBE_STRUCT line=%d", __LINE__);
		m_pNvram=new NVRAM_CAMERA_STROBE_STRUCT[1];
	}

	int sz;
	getDefaultStrobeNVRam(0, m_pNvram, &sz);

	LogInfo("loadDefaultNvram engTab[0,1,2,3,4,5,6,7,15,23,31] %d %d %d %d %d %d %d %d %d %d %d\n",
	m_pNvram->engTab.yTab[0],	m_pNvram->engTab.yTab[1],	m_pNvram->engTab.yTab[2],	m_pNvram->engTab.yTab[3],	m_pNvram->engTab.yTab[4],	m_pNvram->engTab.yTab[5],	m_pNvram->engTab.yTab[6],	m_pNvram->engTab.yTab[7],	m_pNvram->engTab.yTab[15],	m_pNvram->engTab.yTab[23],	m_pNvram->engTab.yTab[31]);

	LogInfo("loadDefaultNvram-engTab[32,39,47,54,63] %d %d %d %d %d\n",
	m_pNvram->engTab.yTab[32], m_pNvram->engTab.yTab[39], m_pNvram->engTab.yTab[47], m_pNvram->engTab.yTab[54], m_pNvram->engTab.yTab[63]);


	return 0;
}

int FlashMgr::loadNvram()
{
	if(m_pNvram==0)
	{
		LogInfo("loadNvram line=%d",__LINE__);
		forceLoadNvram();
	}
	return 0;
}

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

	loadNvram();
	FLASH_PROJECT_PARA prjPara;
	prjPara = cust_getFlashProjectPara(LIB3A_AE_MODE_AUTO, m_pNvram);

	int err=0;
	int e;
	StrobeDrv* pStrobe = StrobeDrv::createInstance();
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




	if(m_cct_isUserDutyStep==1)
	{
		FLASH_PROJECT_PARA prjPara;
		loadNvram();
		int aeMode;
		aeMode = AeMgr::getInstance().getAEMode();
		prjPara = cust_getFlashProjectPara(aeMode, m_pNvram);
		FlashAlg* pStrobeAlg;
		pStrobeAlg = FlashAlg::getInstance();
		//flash profile
		hw_setFlashProfile(pStrobeAlg, &prjPara, m_pNvram);

		rep->isCurFlashOn=0;
		rep->isEnd=1;
		return 0;
	}



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
	}
	LogInfo("doPfOneFrame frame=%d flashState=%d preExeFrame=%d",m_pfFrameCount,flashState,preExeFrameCnt);
	if(flashState==FLASH_STATE_START)
	{
		flashStatus = FLASH_STATUS_OFF;
		LogInfo("doPfOneFrame state=start");
		start();
		if(isFlashOnCapture()==0)
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
		if(m_pfFrameCount-preExeFrameCnt>=3)
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
	turnOffPrecapFlash();

	ClearAePlineEvSetting();
	int ratioEn;
	ratioEn = FlashUtil::getPropInt("z.flash_ratio_en",0);
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
#if defined(DUMMY_FLASHLIGHT)
#else
	if(m_cct_isUserDutyStep==1)
		return 1;
#endif

	LogInfo("isNeedFiringFlash(void) line=%d",__LINE__);
	m_db.startTime = FlashUtil::getMs();
    int bFlashOn;
    FLASH_INFO_T finfo;

    if(m_flashMode==LIB3A_FLASH_MODE_FORCE_OFF)
    {
		LogInfo("isNeedFiringFlash() line=%d flash mode = off",__LINE__);
    	bFlashOn=0;
    	finfo.flashMode = FLASHLIGHT_FORCE_OFF;
    	finfo.isFlash = 0;
    }
    else if(m_flashMode==LIB3A_FLASH_MODE_FORCE_ON)
    {
    	LogInfo("isNeedFiringFlash() line=%d flash mode = LIB3A_FLASH_MODE_FORCE_ON",__LINE__);
    	bFlashOn=1;
    	finfo.flashMode = FLASHLIGHT_FORCE_ON;
    	finfo.isFlash = 1;
    }
    else //auto
    {
    	finfo.flashMode = FLASHLIGHT_AUTO;
    	if(AeMgr::getInstance().IsStrobeBVTrigger()==1)
    	{
    		LogInfo("isNeedFiringFlash() line=%d auto, fire",__LINE__);
	    	bFlashOn=1;
    		finfo.isFlash = 1;
    	}
    	else
    	{
    		LogInfo("isNeedFiringFlash() line=%d auto, off",__LINE__);
    		bFlashOn=0;
    		finfo.isFlash = 0;
    	}
    }
#if defined(DUMMY_FLASHLIGHT)

	LogInfo("isNeedFiringFlash() line=%d off due to dummy",__LINE__);
	bFlashOn=0;
	finfo.flashMode = FLASHLIGHT_FORCE_OFF;
    finfo.isFlash = 0;
#endif

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
	if(isFlash==0)
		return 0;

	FLASH_PROJECT_PARA prjPara;
	loadNvram();
	/*
	static int vv=0;
	if(vv=0)
	{
		if(m_pNvram!=0)
			delete []m_pNvram;
		m_pNvram = new NVRAM_CAMERA_STROBE_STRUCT[1];
		int sz;
		getDefaultStrobeNVRam(m_sensorID, m_pNvram, &sz);
	}*/
	int aeMode;
	aeMode = AeMgr::getInstance().getAEMode();
	prjPara = cust_getFlashProjectPara(aeMode, m_pNvram);


	LogInfo("start() nvram_tar = %d %d %d %d aemode=%d",
		m_pNvram->tuningPara[0].yTar,
		m_pNvram->tuningPara[1].yTar,
		m_pNvram->tuningPara[2].yTar,
		m_pNvram->tuningPara[3].yTar,
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
	hw_setFlashProfile(pStrobeAlg, &prjPara, m_pNvram);
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
int FlashMgr::turnOffPrecapFlash()
{
	hw_turnOffFlash();
	return 0;
}
int FlashMgr::run(FlashExePara* para, FlashExeRep* rep)
{
LogInfo("run() line=%d",__LINE__);
	FLASH_PROJECT_PARA prjPara;
	loadNvram();
	int aeMode;
	aeMode = AeMgr::getInstance().getAEMode();
	prjPara = cust_getFlashProjectPara(aeMode, m_pNvram);


	FlashAlg* pStrobeAlg;
	pStrobeAlg = FlashAlg::getInstance();
	FlashAlgStaData staData;
	short g_data2[40*30*2];
	staData.data = g_data2;


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
		pStrobeAlg->Estimate(&g_expPara);

		int afe;
		int isp;
		hw_capIsoToGain(g_expPara.iso, &afe, &isp);

		rep->nextAfeGain = afe;
		rep->nextIspGain = isp;
		rep->nextExpTime = g_expPara.exp;
		rep->nextIsFlash = g_expPara.isFlash;
		rep->nextDuty = g_expPara.duty;
		rep->nextStep = g_expPara.step;
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
	m_evComp = ind*ev_step*0.75;
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
	LogInfo("setFlashMode mode=%d",mode);
	if(mode<LIB3A_FLASH_MODE_MIN || mode>LIB3A_FLASH_MODE_MAX)
	{
		return FL_ERR_FlashModeNotSupport;
	}
	else
	{
		int preMode;
		if(m_flashMode==FLASHLIGHT_TORCH && mode!=m_flashMode) //prviouw mode is torch. and change to another mode.
		{
			g_pStrobe->setOnOff(0);
		}


		if(mode==FLASHLIGHT_TORCH && mode!=m_flashMode)
		{
			FLASH_PROJECT_PARA prjPara;
			loadNvram();
			int aeMode;
			aeMode = AeMgr::getInstance().getAEMode();
			prjPara = cust_getFlashProjectPara(aeMode, m_pNvram);
			g_pStrobe = StrobeDrv::createInstance();
			//@@if(current mode)
			LogInfo("setFlashMode mode duty,step=%d %d",prjPara.engLevel.torchDuty, prjPara.engLevel.torchStep);
			g_pStrobe->setDuty(prjPara.engLevel.torchDuty);
			g_pStrobe->setStep(prjPara.engLevel.torchStep);
			g_pStrobe->setTimeOutTime(0);
			g_pStrobe->setOnOff(0);
			g_pStrobe->setOnOff(1);
		}
		else if(mode==FLASHLIGHT_FORCE_OFF)
		{
			g_pStrobe = StrobeDrv::createInstance();
			g_pStrobe->setTimeOutTime(1000);
			g_pStrobe->setOnOff(0);
		}
		m_flashMode = mode;
		return 0;
	}
}
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
int FlashMgr::isFlashOnCapture()
{
	LogInfo("run() line=%d isFlashOnCapture=%d",__LINE__,m_flashOnPrecapture);

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

int FlashMgr::capCheckAndFireFlash_End()
{
	LogInfo("capCheckAndFireFlash_End line=%d  getMs=%d",__LINE__, FlashUtil::getMs());
	if(m_cct_isUserDutyStep==1)
		turnOffFlashDevice();
	if(isBurstShotMode()!=1)
	{
	    //FLASH OFF
	    turnOffFlashDevice();
    }
    LogInfo("capCheckAndFireFlash_End line=%d  thisFireStartTime & End: %d %d",__LINE__, m_db.thisFireStartTime,m_db.thisFireEndTime);
	return 0;
}
int FlashMgr::capCheckAndFireFlash_Start()
{
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
		aeMode = AeMgr::getInstance().getAEMode();
		prjPara = cust_getFlashProjectPara(aeMode, m_pNvram);

		if(prjPara.coolTimeOutPara.tabMode==ENUM_FLASH_ENG_INDEX_MODE)
		{
			timeOutTime = interpTimeOut_index(m_db.capDuty, prjPara.coolTimeOutPara.tabNum, prjPara.coolTimeOutPara.tabId, prjPara.coolTimeOutPara.timOutMs);
			m_db.thisTimeOutTime = timeOutTime;
			m_db.coolingTM = interpCoolTM_index(m_db.capDuty, prjPara.coolTimeOutPara.tabNum, prjPara.coolTimeOutPara.tabId, prjPara.coolTimeOutPara.coolingTM);
		}
		else if(prjPara.coolTimeOutPara.tabMode==ENUM_FLASH_ENG_PERCENTAGE_MODE)
		{
			int eng;
			FlashAlg* pStrobeAlg;
			pStrobeAlg = FlashAlg::getInstance();
			eng = pStrobeAlg->calFlashEng(m_db.capDuty, m_db.capStep);
			timeOutTime = interpTimeOut_index(eng/10, prjPara.coolTimeOutPara.tabNum, prjPara.coolTimeOutPara.tabId, prjPara.coolTimeOutPara.timOutMs);
			m_db.thisTimeOutTime = timeOutTime;
			m_db.coolingTM = interpCoolTM_index(eng, prjPara.coolTimeOutPara.tabNum, prjPara.coolTimeOutPara.tabId, prjPara.coolTimeOutPara.coolingTM);
		}

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
		g_pStrobe->setOnOff(0);
		g_pStrobe->setOnOff(1);

	}
	return 0;

}
int FlashMgr::turnOffFlashDevice()
{
	LogInfo("%s line=%d",__FUNCTION__,__LINE__);
	if(m_flashOnPrecapture==1 && m_isCapFlashEndTimeValid==0)
	{
		m_isCapFlashEndTimeValid=1;
		g_pStrobe = StrobeDrv::createInstance();
		g_pStrobe->setOnOff(0);
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
	g_pStrobe->setOnOff(0);
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
	g_plineEvSetting = new strEvSetting[pAE->u4TotalIndex];
	for(i=0;i<(int)pAE->u4TotalIndex;i++)
	{
		g_plineEvSetting[i] = pAE->pCurrentTable[i];
	}
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
