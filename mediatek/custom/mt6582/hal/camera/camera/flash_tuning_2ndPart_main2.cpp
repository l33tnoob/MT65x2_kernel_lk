#define LOG_TAG "flash_custom_cct_m2.cpp"
#include "string.h"
#include "camera_custom_nvram.h"
#include "camera_custom_types.h"
#include "camera_custom_AEPlinetable.h"
#include "camera_custom_nvram.h"
#include <cutils/xlog.h>
#include "flash_feature.h"
#include "flash_param.h"
#include "flash_tuning_custom.h"
#include <kd_camera_feature.h>

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


static void copyTuningPara(FLASH_TUNING_PARA* p, NVRAM_FLASH_TUNING_PARA* nv_p)
{
	p->yTar = nv_p->yTar;
	p->antiIsoLevel = nv_p->antiIsoLevel;
	p->antiExpLevel = nv_p->antiExpLevel;
	p->antiStrobeLevel = nv_p->antiStrobeLevel;
	p->antiUnderLevel = nv_p->antiUnderLevel;
	p->antiOverLevel = nv_p->antiOverLevel;
	p->foregroundLevel = nv_p->foregroundLevel;
	p->isRefAfDistance = nv_p->isRefAfDistance;
    p->accuracyLevel = nv_p->accuracyLevel;
}




FLASH_PROJECT_PARA& cust_getFlashProjectPara_main2(int aeMode, NVRAM_CAMERA_STROBE_STRUCT* nvrame)
{
	XLOGD("cust_getFlashProjectPara_main2");
	static FLASH_PROJECT_PARA para;

	para.dutyNum = 2;
	para.stepNum = 1;
	//tuning



	if(nvrame!=0)
	{
		XLOGD("nvrame!=0");
		switch(aeMode)
		{
			case LIB3A_AE_MODE_OFF:
			case LIB3A_AE_MODE_AUTO:
				copyTuningPara(&para.tuningPara, &nvrame->tuningPara[0]);
			break;
			case LIB3A_AE_MODE_NIGHT:
			case LIB3A_AE_MODE_CANDLELIGHT:
			case LIB3A_AE_MODE_FIREWORKS:
			case LIB3A_AE_MODE_NIGHT_PORTRAIT:
				copyTuningPara(&para.tuningPara, &nvrame->tuningPara[1]);
			break;
			case LIB3A_AE_MODE_ACTION:
			case LIB3A_AE_MODE_SPORTS:
			case LIB3A_AE_MODE_ISO_ANTI_SHAKE:
				copyTuningPara(&para.tuningPara, &nvrame->tuningPara[2]);
			break;
			case LIB3A_AE_MODE_PORTRAIT:
				copyTuningPara(&para.tuningPara, &nvrame->tuningPara[3]);
			break;
			default:
				copyTuningPara(&para.tuningPara, &nvrame->tuningPara[0]);
			break;
		}
	}
	//--------------------
	//eng level
	//index mode
	//torch
	para.engLevel.torchEngMode = ENUM_FLASH_ENG_INDEX_MODE;
	para.engLevel.torchDuty = 0;
	para.engLevel.torchStep = 0;

	//af
	para.engLevel.afEngMode = ENUM_FLASH_ENG_INDEX_MODE;
	para.engLevel.afDuty = 0;
	para.engLevel.afStep = 0;

	//pf, mf, normal
	para.engLevel.pmfEngMode = ENUM_FLASH_ENG_INDEX_MODE;
	para.engLevel.pfDuty = 0;
	para.engLevel.mfDutyMax = 1;
	para.engLevel.mfDutyMin = 0;
	para.engLevel.pmfStep = 0;

	//low bat
	para.engLevel.IChangeByVBatEn=0;
	para.engLevel.vBatL = 3550;	//mv
	para.engLevel.pfDutyL = 0;
	para.engLevel.mfDutyMaxL = 0;
	para.engLevel.mfDutyMinL = 0;
	para.engLevel.pmfStepL = 0;

	//burst setting
	para.engLevel.IChangeByBurstEn=1;
	para.engLevel.pfDutyB = 0;
	para.engLevel.mfDutyMaxB = 0;
	para.engLevel.mfDutyMinB = 0;
	para.engLevel.pmfStepB = 0;

	//--------------------
	//cooling delay para
	para.coolTimeOutPara.tabMode = ENUM_FLASH_ENG_INDEX_MODE;
	para.coolTimeOutPara.tabNum = 2;
	para.coolTimeOutPara.tabId[0]=0;
	para.coolTimeOutPara.tabId[1]=1;

	para.coolTimeOutPara.coolingTM[0]=0;
	para.coolTimeOutPara.coolingTM[1]=5;


	para.coolTimeOutPara.timOutMs[0]=ENUM_FLASH_TIME_NO_TIME_OUT;
	para.coolTimeOutPara.timOutMs[1]=500;


	para.maxCapExpTimeUs=100000;
	para.pfExpFollowPline=0;
	//para.maxPfAfe=3000;
	para.maxAfeGain=4096;

	//92 new
	para.lowReflectanceTuningEnable=1;
	para.yTargetWeight=0;
	para.lowReflectanceThreshold=16;
	para.flashReflectanceWeight = 0;


	if(nvrame!=0)
	{
		if(nvrame->isTorchEngUpdate)
		{
			para.engLevel.torchEngMode = nvrame->engLevel.torchEngMode;
			para.engLevel.torchPeakI = nvrame->engLevel.torchPeakI;
		    para.engLevel.torchAveI = nvrame->engLevel.torchAveI;
			para.engLevel.torchDuty = nvrame->engLevel.torchDuty;
			para.engLevel.torchStep = nvrame->engLevel.torchStep;
		}
		if(nvrame->isAfEngUpdate)
		{
			para.engLevel.afEngMode = nvrame->engLevel.afEngMode;
			para.engLevel.afPeakI = nvrame->engLevel.afPeakI;
		    para.engLevel.afAveI = nvrame->engLevel.afAveI;
			para.engLevel.afDuty = nvrame->engLevel.afDuty;
			para.engLevel.afStep = nvrame->engLevel.afStep;
		}
		if(nvrame->isNormaEnglUpdate)
		{
			para.engLevel.pfAveI = nvrame->engLevel.pfAveI;
			para.engLevel.mfAveIMax = nvrame->engLevel.mfAveIMax;
			para.engLevel.mfAveIMin = nvrame->engLevel.mfAveIMin;
			para.engLevel.pmfPeakI = nvrame->engLevel.pmfPeakI;
			para.engLevel.pfDuty = nvrame->engLevel.pfDuty;
			para.engLevel.mfDutyMax = nvrame->engLevel.mfDutyMax;
			para.engLevel.mfDutyMin = nvrame->engLevel.mfDutyMin;
			para.engLevel.pmfStep = nvrame->engLevel.pmfStep;
		}
		if(nvrame->isLowBatEngUpdate)
		{
			para.engLevel.IChangeByVBatEn = nvrame->engLevel.IChangeByVBatEn;
			para.engLevel.vBatL = nvrame->engLevel.vBatL;
			para.engLevel.pfAveIL = nvrame->engLevel.pfAveIL;
			para.engLevel.mfAveIMaxL = nvrame->engLevel.mfAveIMaxL;
			para.engLevel.mfAveIMinL = nvrame->engLevel.mfAveIMinL;
			para.engLevel.pmfPeakIL = nvrame->engLevel.pmfPeakIL;
			para.engLevel.pfDutyL = nvrame->engLevel.pfDutyL;
			para.engLevel.mfDutyMaxL = nvrame->engLevel.mfDutyMaxL;
			para.engLevel.mfDutyMinL = nvrame->engLevel.mfDutyMinL;
			para.engLevel.pmfStepL = nvrame->engLevel.pmfStepL;
		}
		if(nvrame->isBurstEngUpdate)
		{
			para.engLevel.IChangeByBurstEn = nvrame->engLevel.IChangeByBurstEn;
			para.engLevel.pfAveIB = nvrame->engLevel.pfAveIB;
			para.engLevel.mfAveIMaxB = nvrame->engLevel.mfAveIMaxB;
			para.engLevel.mfAveIMaxB = nvrame->engLevel.mfAveIMaxB;
			para.engLevel.mfAveIMinB = nvrame->engLevel.mfAveIMinB;
			para.engLevel.pmfPeakIB = nvrame->engLevel.pmfPeakIB;
			para.engLevel.pfDutyB = nvrame->engLevel.pfDutyB;
			para.engLevel.mfDutyMaxB = nvrame->engLevel.mfDutyMaxB;
			para.engLevel.mfDutyMinB = nvrame->engLevel.mfDutyMinB;
			para.engLevel.pmfStepB = nvrame->engLevel.pmfStepB;
		}
	}
	return para;
}


