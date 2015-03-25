#ifdef WIN32
#include "stdafx.h"
#include "FlashSim.h"
#include "sim_MTKAECommon.h"
#include "sim_MTKAE.h"
#include "FlashAlg.h"
#include "flash_mgr.h"



#else

#define LOG_TAG "flash_cct.cpp"


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
#include <vector>
#include <sstream>
#include "cct_feature.h"
#include <vector>
#include <asm/arch/mt6589_sync_write.h>
#include <isp_reg.h>
#include <isp_drv.h>
#include "flash_util.h"

#define LogInfo(fmt, arg...) XLOGD(fmt, ##arg)
#define LogError(fmt, arg...) XLOGD("FlashError: func=%s line=%d: "fmt, __FUNCTION__, __LINE__, ##arg)
#define LogWarning(fmt, arg...) XLOGD("FlashWarning: %5d: "fmt, __LINE__, ##arg)

using namespace NS3A;
using namespace NSIspTuning;
using namespace std;

#define ALG_TAG_SIZE 500

#define A3_DIV_X 120
#define A3_DIV_Y 90
#define Z0_FL_DIV_X 24
#define Z0_FL_DIV_Y 18


#endif


//==============================================




#define CK_MIN_ERR	1
#define CK_MAX_ERR	2
#define CK_MIN_WARN	4
#define CK_MAX_WARN	8

#define toVAndString(x) x, #x

template <class var, class strType>
void RangeCheck(var v, strType checkString, var min4Err, var max4Err, var min4Warn, var max4Warn,  ostringstream& errStream, int checkType)
{
	if( (checkType & CK_MIN_ERR) && v<min4Err)
		errStream<<"err: "<< checkString << "=" << v << "<" << min4Err << endl;
	if( (checkType & CK_MAX_ERR) && v>max4Err)
		errStream<<"err: "<< checkString << "=" << v << ">" << max4Err <<   endl;
	if( (checkType & CK_MIN_WARN) && v<min4Warn)
		errStream<<"warning: "<< checkString << "=" << v << "<" << min4Warn << endl;
	if( (checkType & CK_MAX_WARN) && v>max4Warn)
		errStream<<"warning: "<< checkString << "=" << v << ">" << max4Warn << endl;
}


int g_checkErrPacketNO=0;
vector<char> g_checkErrVector;


static int g_cctPfFrameCount=0;

void FlashMgr::cctInit()
{
	g_cctPfFrameCount=0;
}
void FlashMgr::cctUninit()
{
}

int FlashMgr::cctCheckPara()
{
	return 0;
}

int FlashMgr::cctFlashEnable(int en)
{
	LogInfo("cctFlashEnable(en=%d) line=%d",en,__LINE__);
	if(en==1)
	{
		setFlashMode(FLASHLIGHT_FORCE_ON);
	}
	else
	{
		setFlashMode(FLASHLIGHT_FORCE_OFF);
	}
	return 0;
}
void cctGetCheckParaString(int* packetNO, int* isEnd, int* bufLength, unsigned char* buf) //buf: max 1024
{
	/*
	int dutyNum;
	int stepNum;
	FLASH_PROJECT_PARA prjPara;
	NVRAM_CAMERA_STROBE_STRUCT* pNvram;
	NvramDrvMgr::getInstance().init(ESensorDev_Main);
	NvramDrvMgr::getInstance().getRefBuf(pNvram);
	prjPara = cust_getFlashProjectPara(LIB3A_AE_MODE_AUTO, pNvram);


	RangeCheck(toVAndString(prjPara.dutyNum), 1, 0, 0, 0,  checkErrStream, CK_MIN_ERR);
	RangeCheck(toVAndString(prjPara.stepNum), 1, 0, 0, 0,  checkErrStream, CK_MIN_ERR);

	dutyNum = prjPara.dutyNum;
	stepNum = prjPara.stepNum;

	RangeCheck(toVAndString(prjPara.engLevel.torchEngMode), ENUM_FLASH_ENG_INDEX_MODE, ENUM_FLASH_ENG_CURRENT_MODE, 0, 0,  checkErrStream, CK_MIN_ERR|CK_MAX_ERR);
	if(prjPara.engLevel.torchEngMode==ENUM_FLASH_ENG_CURRENT_MODE)
	{
		RangeCheck(toVAndString(prjPara.engLevel.torchPeakI), 0, 0, 0, 0,  checkErrStream, CK_MIN_ERR);
		RangeCheck(toVAndString(prjPara.engLevel.torchAveI), 0, 0, 0, 0,  checkErrStream, CK_MIN_ERR);
	}
	else
	{
		RangeCheck(toVAndString(prjPara.engLevel.torchDuty), 0, dutyNum-1, 0, 0,  checkErrStream, CK_MIN_ERR|CK_MAX_ERR);
		RangeCheck(toVAndString(prjPara.engLevel.torchStep), 0, stepNum-1, 0, 0,  checkErrStream, CK_MIN_ERR|CK_MAX_ERR);
	}

	RangeCheck(toVAndString(prjPara.engLevel.afEngMode), ENUM_FLASH_ENG_INDEX_MODE, ENUM_FLASH_ENG_CURRENT_MODE, 0, 0,  checkErrStream, CK_MIN_ERR|CK_MAX_ERR);
	if(prjPara.engLevel.afEngMode==ENUM_FLASH_ENG_CURRENT_MODE)
	{
		RangeCheck(toVAndString(prjPara.engLevel.afPeakI), 0, 0, 0, 0,  checkErrStream, CK_MIN_ERR);
		RangeCheck(toVAndString(prjPara.engLevel.afAveI), 0, 0, 0, 0,  checkErrStream, CK_MIN_ERR);
	}
	else
	{
		RangeCheck(toVAndString(prjPara.engLevel.afDuty), 0, dutyNum-1, 0, 0,  checkErrStream, CK_MIN_ERR|CK_MAX_ERR);
		RangeCheck(toVAndString(prjPara.engLevel.afStep), 0, stepNum-1, 0, 0,  checkErrStream, CK_MIN_ERR|CK_MAX_ERR);
	}

	RangeCheck(toVAndString(prjPara.engLevel.pmfEngMode), ENUM_FLASH_ENG_INDEX_MODE, ENUM_FLASH_ENG_CURRENT_MODE, 0, 0,  checkErrStream, CK_MIN_ERR|CK_MAX_ERR);
	if(prjPara.engLevel.pmfEngMode==ENUM_FLASH_ENG_CURRENT_MODE)
	{
		RangeCheck(toVAndString(prjPara.engLevel.pfAveI), 0, 0, 0, 0,  checkErrStream, CK_MIN_ERR);
		RangeCheck(toVAndString(prjPara.engLevel.mfAveIMax), 0, 0, 0, 0,  checkErrStream, CK_MIN_ERR);
		RangeCheck(toVAndString(prjPara.engLevel.mfAveIMin), 0, 0, 0, 0,  checkErrStream, CK_MIN_ERR);
		RangeCheck(toVAndString(prjPara.engLevel.pmfPeakI), 0, 0, 0, 0,  checkErrStream, CK_MIN_ERR);
	}
	else
	{
		RangeCheck(toVAndString(prjPara.engLevel.pfDuty), 0, dutyNum-1, 0, 0,  checkErrStream, CK_MIN_ERR|CK_MAX_ERR);
		RangeCheck(toVAndString(prjPara.engLevel.mfDutyMax), 0, dutyNum-1, 0, 0,  checkErrStream, CK_MIN_ERR|CK_MAX_ERR);
		RangeCheck(toVAndString(prjPara.engLevel.mfDutyMin), 0, dutyNum-1, 0, 0,  checkErrStream, CK_MIN_ERR|CK_MAX_ERR);
		RangeCheck(toVAndString(prjPara.engLevel.pmfStep), 0, stepNum-1, 0, 0,  checkErrStream, CK_MIN_ERR|CK_MAX_ERR);
	}

	RangeCheck(toVAndString(prjPara.engLevel.IChangeByVBatEn), 0, 1, 0, 0,  checkErrStream, CK_MIN_ERR|CK_MAX_ERR);
	if(prjPara.engLevel.IChangeByVBatEn==1)
	{
		RangeCheck(toVAndString(prjPara.engLevel.vBatL), 0, 0, 0, 0,  checkErrStream, CK_MIN_ERR);
		if(prjPara.engLevel.pmfEngMode==ENUM_FLASH_ENG_CURRENT_MODE)
		{
			RangeCheck(toVAndString(prjPara.engLevel.pfAveIL), 0, 0, 0, 0,  checkErrStream, CK_MIN_ERR);
			RangeCheck(toVAndString(prjPara.engLevel.mfAveIMaxL), 0, 0, 0, 0,  checkErrStream, CK_MIN_ERR);
			RangeCheck(toVAndString(prjPara.engLevel.mfAveIMinL), 0, 0, 0, 0,  checkErrStream, CK_MIN_ERR);
			RangeCheck(toVAndString(prjPara.engLevel.pmfPeakIL), 0, 0, 0, 0,  checkErrStream, CK_MIN_ERR);
		}
		else
		{
			RangeCheck(toVAndString(prjPara.engLevel.pfDutyL), 0, dutyNum-1, 0, 0,  checkErrStream, CK_MIN_ERR|CK_MAX_ERR);
			RangeCheck(toVAndString(prjPara.engLevel.mfDutyMaxL), 0, dutyNum-1, 0, 0,  checkErrStream, CK_MIN_ERR|CK_MAX_ERR);
			RangeCheck(toVAndString(prjPara.engLevel.mfDutyMinL), 0, dutyNum-1, 0, 0,  checkErrStream, CK_MIN_ERR|CK_MAX_ERR);
			RangeCheck(toVAndString(prjPara.engLevel.pmfStepL), 0, stepNum-1, 0, 0,  checkErrStream, CK_MIN_ERR|CK_MAX_ERR);
		}
	}

	RangeCheck(toVAndString(prjPara.engLevel.IChangeByBurstEn), 0, 1, 0, 0,  checkErrStream, CK_MIN_ERR|CK_MAX_ERR);
	if(prjPara.engLevel.IChangeByBurstEn==1)
	{
		if(prjPara.engLevel.pmfEngMode==ENUM_FLASH_ENG_CURRENT_MODE)
		{
			RangeCheck(toVAndString(prjPara.engLevel.pfAveIB), 0, 0, 0, 0,  checkErrStream, CK_MIN_ERR);
			RangeCheck(toVAndString(prjPara.engLevel.mfAveIMaxB), 0, 0, 0, 0,  checkErrStream, CK_MIN_ERR);
			RangeCheck(toVAndString(prjPara.engLevel.mfAveIMinB), 0, 0, 0, 0,  checkErrStream, CK_MIN_ERR);
			RangeCheck(toVAndString(prjPara.engLevel.pmfPeakIB), 0, 0, 0, 0,  checkErrStream, CK_MIN_ERR);
		}
		else
		{
			RangeCheck(toVAndString(prjPara.engLevel.pfDutyB), 0, dutyNum-1, 0, 0,  checkErrStream, CK_MIN_ERR|CK_MAX_ERR);
			RangeCheck(toVAndString(prjPara.engLevel.mfDutyMaxB), 0, dutyNum-1, 0, 0,  checkErrStream, CK_MIN_ERR|CK_MAX_ERR);
			RangeCheck(toVAndString(prjPara.engLevel.mfDutyMinB), 0, dutyNum-1, 0, 0,  checkErrStream, CK_MIN_ERR|CK_MAX_ERR);
			RangeCheck(toVAndString(prjPara.engLevel.pmfStepB), 0, stepNum-1, 0, 0,  checkErrStream, CK_MIN_ERR|CK_MAX_ERR);
		}
	}

	if(prjPara.engLevel.pmfEngMode==ENUM_FLASH_ENG_CURRENT_MODE)
	{
		RangeCheck(toVAndString(prjPara.caliPara.extrapI), 0, 0, 0, 0,  checkErrStream, CK_MIN_ERR);
		RangeCheck(toVAndString(prjPara.caliPara.extrapRefI), 0, 0, 0, 0,  checkErrStream, CK_MIN_ERR);
		RangeCheck(toVAndString(prjPara.caliPara.minPassI), 0, 0, 0, 0,  checkErrStream, CK_MIN_ERR);
		RangeCheck(toVAndString(prjPara.caliPara.maxTestI), 0, 0, 0, 0,  checkErrStream, CK_MIN_ERR);
		RangeCheck(toVAndString(prjPara.caliPara.minTestBatV), 0, 0, 0, 0,  checkErrStream, CK_MIN_ERR);
		RangeCheck(toVAndString(prjPara.caliPara.toleranceI), 0, 0, 0, 0,  checkErrStream, CK_MIN_ERR);
		RangeCheck(toVAndString(prjPara.caliPara.toleranceV), 0, 0, 0, 0,  checkErrStream, CK_MIN_ERR);
	}


	RangeCheck(toVAndString(prjPara.coolTimeOutPara.tabNum), 2, 0, 0, 0,  checkErrStream, CK_MIN_ERR);
	RangeCheck(toVAndString(prjPara.coolTimeOutPara.tabMode), ENUM_FLASH_ENG_MODE_MIN_CD, ENUM_FLASH_ENG_MODE_MAX_CD, 0, 0,  checkErrStream, CK_MIN_ERR|CK_MAX_ERR);




*/





}


int FlashMgr::egnSetParam(int id, int v, int ext1)
{
	int p[3];
	p[0]=id;
	p[1]=v;
	p[2]=ext1;
	//return cctSetParam(p);
	return 0;
}

int FlashMgr::egnSetPfIndex(int duty, int step)
{
	FLASH_PROJECT_PARA prjPara;
	prjPara =  cust_getFlashProjectPara(0,0);
	int dutyN;
	int stepN;
	stepN = prjPara.stepNum;
	dutyN = prjPara.dutyNum;
	LogInfo("egnSetPfIndex() dutyN,stepN=%d %d,  duty,step=%d %d",dutyN,stepN, duty, step);
	if(step>= stepN || step<0)
	{
		return FL_ERR_SetLevelFail;
	}
	if(duty>= dutyN || duty<0)
	{
		return FL_ERR_SetLevelFail;
	}


	egnSetParam(ACDK_FL_CCT_ID_isNormaEnglUpdate, 1, 0);
	egnSetParam(ACDK_FL_CCT_ID_isLowBatEngUpdate, 1, 0);
	egnSetParam(ACDK_FL_CCT_ID_pmfEngMode, ACDK_FL_CCT_ENG_INDEX_MODE, 0);
	egnSetParam(ACDK_FL_CCT_ID_pfDuty, duty, 0);
	egnSetParam(ACDK_FL_CCT_ID_pmfStep, step, 0);
	egnSetParam(ACDK_FL_CCT_ID_IChangeByVBatEn, 0, 0);




	prjPara = cust_getFlashProjectPara(LIB3A_AE_MODE_AUTO, m_pNvram);
	LogInfo("egnSetPfIndex() pmfEngMode=%d", prjPara.engLevel.pmfEngMode);
	LogInfo("egnSetPfIndex() pfAveI=%d", prjPara.engLevel.pfAveI);
	LogInfo("egnSetPfIndex() mfAveIMax=%d", prjPara.engLevel.mfAveIMax);
	LogInfo("egnSetPfIndex() mfAveIMin=%d", prjPara.engLevel.mfAveIMin);
	LogInfo("egnSetPfIndex() pmfPeakI=%d", prjPara.engLevel.pmfPeakI);
	LogInfo("egnSetPfIndex() pfDuty=%d", prjPara.engLevel.pfDuty);
	LogInfo("egnSetPfIndex() mfDutyMax=%d", prjPara.engLevel.mfDutyMax);
	LogInfo("egnSetPfIndex() mfDutyMin=%d", prjPara.engLevel.mfDutyMin);
	LogInfo("egnSetPfIndex() pmfStep=%d", prjPara.engLevel.pmfStep);
	LogInfo("egnSetPfIndex() IChangeByVBatEn=%d", prjPara.engLevel.IChangeByVBatEn);




	return 0;
}
int FlashMgr::egnSetMfIndex(int duty, int step)
{
	FLASH_PROJECT_PARA prjPara;
	prjPara =  cust_getFlashProjectPara(0,0);
	int dutyN;
	int stepN;
	stepN = prjPara.stepNum;
	dutyN = prjPara.dutyNum;
	LogInfo("egnSetMfIndex() dutyN,stepN=%d %d,  duty,step=%d %d",dutyN,stepN, duty, step);
	if(step>= stepN || step<0)
	{
		return FL_ERR_SetLevelFail;
	}
	if(duty>= dutyN || duty<0)
	{
		return FL_ERR_SetLevelFail;
	}

	egnSetParam(ACDK_FL_CCT_ID_isNormaEnglUpdate, 1, 0);
	egnSetParam(ACDK_FL_CCT_ID_isLowBatEngUpdate, 1, 0);
	egnSetParam(ACDK_FL_CCT_ID_pmfEngMode, ACDK_FL_CCT_ENG_INDEX_MODE, 0);
	egnSetParam(ACDK_FL_CCT_ID_mfDutyMax, duty, 0);
	egnSetParam(ACDK_FL_CCT_ID_mfDutyMin, duty, 0);
	egnSetParam(ACDK_FL_CCT_ID_pmfStep, step, 0);
	egnSetParam(ACDK_FL_CCT_ID_IChangeByVBatEn, 0, 0);


	prjPara = cust_getFlashProjectPara(LIB3A_AE_MODE_AUTO, m_pNvram);
	LogInfo("egnSetMfIndex() pmfEngMode=%d", prjPara.engLevel.pmfEngMode);
	LogInfo("egnSetMfIndex() pfAveI=%d", prjPara.engLevel.pfAveI);
	LogInfo("egnSetMfIndex() mfAveIMax=%d", prjPara.engLevel.mfAveIMax);
	LogInfo("egnSetMfIndex() mfAveIMin=%d", prjPara.engLevel.mfAveIMin);
	LogInfo("egnSetMfIndex() pmfPeakI=%d", prjPara.engLevel.pmfPeakI);
	LogInfo("egnSetMfIndex() pfDuty=%d", prjPara.engLevel.pfDuty);
	LogInfo("egnSetMfIndex() mfDutyMax=%d", prjPara.engLevel.mfDutyMax);
	LogInfo("egnSetMfIndex() mfDutyMin=%d", prjPara.engLevel.mfDutyMin);
	LogInfo("egnSetMfIndex() pmfStep=%d", prjPara.engLevel.pmfStep);
	LogInfo("egnSetMfIndex() IChangeByVBatEn=%d", prjPara.engLevel.IChangeByVBatEn);
	return 0;
}




void calCheckSum8(void* d, int bytes, int* sum)
{
	unsigned char* pData;
	pData = (unsigned char*)d;
	int i;
	int j;
	for(i=0;i<8;i++)
		sum[i]=0;
	int div[8]={1,2,3,3,4,4,5,5};
	int rem[8]={0,0,0,1,0,1,0,1};
	for(i=0;i<bytes;i++)
	{
		for(j=0;j<8;j++)
		{
			if((i%div[j])==rem[j])
				sum[j]+=pData[i];
		}
	}
}

int FlashMgr::cctSetEngTabWithBackup(int exp, int afe, int isp, short* engTab, short* rgTab, short* bgTab)
{
	LogInfo("cctSetEngTab line=%d", __LINE__);

	loadNvram();
	m_pNvram->engTab.exp = exp;
	m_pNvram->engTab.afe_gain = afe;
	m_pNvram->engTab.isp_gain = isp;

	LogInfo("cctSetEngTab line=%d", __LINE__);


	FLASH_PROJECT_PARA prjPara;
	prjPara =  cust_getFlashProjectPara(0,0);
	prjPara.stepNum;
	prjPara.dutyNum;

	LogInfo("cctSetEngTab line=%d", __LINE__);

	int i;
	for(i=0;i<prjPara.stepNum*prjPara.dutyNum;i++)
	{
		LogInfo("cctSetEngTab\t%d\t%d\t%d",
					engTab[i], rgTab[i], bgTab[i]);
		m_pNvram->engTab.yTab[i]=engTab[i];
		m_pNvram->engTab.rgTab[i]=rgTab[i];
		m_pNvram->engTab.bgTab[i]=bgTab[i];
	}

	int rsv_cnt=400;
	for(i=0;i<rsv_cnt;i++)
	{
		m_pNvram->engTab.rsv[i]=0;
	}

	if(prjPara.stepNum*prjPara.dutyNum<128)
	{
		for(i=0;i<prjPara.stepNum*prjPara.dutyNum;i++)
		{
			m_pNvram->engTab.rsv[i]=engTab[i];
			m_pNvram->engTab.rsv[128+i]=rgTab[i];
			m_pNvram->engTab.rsv[256+i]=bgTab[i];
		}
	}
	else
	{
		int sum[8];
		for(i=0;i<prjPara.stepNum*prjPara.dutyNum/2;i++)
		{
			m_pNvram->engTab.rsv[i]=engTab[i*2];
			m_pNvram->engTab.rsv[128+i]=rgTab[i*2];
			m_pNvram->engTab.rsv[256+i]=bgTab[i*2];

		}
	}

	calCheckSum8((void*)m_pNvram->engTab.rsv,rsv_cnt*2-32, (int*)(&m_pNvram->engTab.rsv[rsv_cnt-16]));




	LogInfo("cctSetEngTab line=%d", __LINE__);
	return 0;
}



struct AeOut
{
	int exp;
	int afe;
	int isp;
	int isEnd;
};

struct AeIn
{
	int frameCount;
	void* staBuf;
	int tar;
	int tarMax;
	int tarMin;
	int senType;
	int cycle;
};



void getMean(AWBAE_STAT_T* p, float* y, float* r, float* g, float* b)
{
	int i;
	int j;
	int rs=0;
	int gs=0;
	int bs=0;
	int ys=0;
	int cnt=0;
	for(i=48;i<72;i++)
	for(j=36;j<54;j++)
	{
		ys+=p->LINE[j].AE_WIN[i];
		rs+=p->LINE[j].AWB_WIN[i].rMainStat.ucR;
		gs+=p->LINE[j].AWB_WIN[i].rMainStat.ucG;
		bs+=p->LINE[j].AWB_WIN[i].rMainStat.ucB;
		cnt++;
	}
	*r = (double)rs/(24*18);
	*g = (double)gs/(24*18);
	*b = (double)bs/(24*18);
	*y = (double)ys/(24*18);
	LogInfo("cnt=%d %d",cnt,24*18);

}
void doAe(AeIn* in, AeOut* out)
{
int IniExp=30000;
int IniAfe=1024;
int IniIsp=1024;

	static int exp;
	static int afe;
	static int isp;
	static double tar;
	static double tarMax;
	static double tarMin;
	//int rgby
	float rm=0;
	float gm=0;
	float bm=0;
	float ym=0;

	int i;
	out->isEnd=0;
	if(in->frameCount%in->cycle==0)
	{
		if(in->frameCount==0)
		{
			exp=IniExp;
			isp=IniIsp;
			afe=IniAfe;
			tar=in->tar;
			tarMax=in->tarMax;
			tarMin=in->tarMin;
		}
		else
		{
			AWBAE_STAT_T* p;
	    	p = (AWBAE_STAT_T*)in->staBuf;
	    	getMean(p, &ym, &rm, &gm, &bm);

	    	LogInfo("doAe yrgb %5.3f %5.3f %5.3f %5.3f %d %d",ym, rm, gm, bm, (int)tarMin, (int)tarMax);

			if(ym>tarMin && ym<tarMax && in->frameCount>0)
			{

				LogInfo("doAeE line=%d",__LINE__);
				out->isEnd=1;
			}
			else if(ym>255)
			{
				LogInfo("doAe line=%d",__LINE__);
				exp=exp/3;
			}
			else if(ym<10)
			{
				LogInfo("doAe line=%d",__LINE__);
				exp=tar/10*exp;
			}
			else
			{
				LogInfo("doAe line=%d",__LINE__);
				exp=tar/ym*exp;
			}
		}
		if(out->isEnd==0)
		{
			//set exp
			AAASensorMgr::getInstance().setSensorExpTime(exp);
	    	AAASensorMgr::getInstance().setSensorGain(afe);
	    	ISP_MGR_OBC_T::getInstance((ESensorDev_T)in->senType).setIspAEGain(isp>>1);
	    	LogInfo("doAe %d %d %d",exp, afe, isp);
	    	out->exp = exp;
	    	out->afe = afe;
	    	out->isp = isp;
    	}
    	else
    	{
    		out->exp = exp;
	    	out->afe = afe;
	    	out->isp = isp;
    	}
	}
}


void FlashMgr::cctGetLastFireTime(int* startTime, int* endTime)
{
	*startTime = m_db.thisFireStartTime;
	*endTime = m_db.thisFireEndTime;
}
//float* g_ym=0;
//fl/oat* g_rm=0;
//float* g_gm=0;
//float* g_bm=0;




void FlashMgr::cctPreflashTest(FlashExePara* para, FlashExeRep* rep)
{
#define STATE_INIT 0
#define STATE_AE 1
#define STATE_AE_POST 2
#define STATE_RATIO 3
#define STATE_END 4
#define STATE_END2 5

	static int state=STATE_INIT;
	rep->isEnd=0;
	int i;
	int j;
	static int* stepArr=0;
	static int* dutyArr=0;
	static float* rm=0;
	static float* gm=0;
	static float* bm=0;
	static float* ym=0;

	static int dutyNum;
	static int stepNum;
	static int testNum;
	static int preStateEndCnt=-1;

	static int exp;
	static int isp;
	static int afe;

	if(g_cctPfFrameCount==0)
	{
		IspDrv* pIspDrv;
		pIspDrv = IspDrv::createInstance();
		isp_reg_t* pIspReg;
		pIspReg = (isp_reg_t*)pIspDrv->getRegAddr();
	    ISP_WRITE_BITS(pIspReg , CAM_AWB_RAWPREGAIN1_0, AWB_GAIN1_R, 0x200);
	    ISP_WRITE_BITS(pIspReg , CAM_AWB_RAWPREGAIN1_0, AWB_GAIN1_G, 0x200);
	    ISP_WRITE_BITS(pIspReg , CAM_AWB_RAWPREGAIN1_1, AWB_GAIN1_B, 0x200);
	    ISP_WRITE_BITS(pIspReg , CAM_AWB_RAWLIMIT1_0, AWB_LIMIT1_R, 0xfff);
	    ISP_WRITE_BITS(pIspReg , CAM_AWB_RAWLIMIT1_0, AWB_LIMIT1_G, 0xfff);
	    ISP_WRITE_BITS(pIspReg , CAM_AWB_RAWLIMIT1_1, AWB_LIMIT1_B, 0xfff);
	    ISP_WRITE_BITS(pIspReg , CAM_AWB_LOW_THR	, AWB_LOW_THR0, 0);
	    ISP_WRITE_BITS(pIspReg , CAM_AWB_LOW_THR	, AWB_LOW_THR1, 0);
	    ISP_WRITE_BITS(pIspReg , CAM_AWB_LOW_THR	, AWB_LOW_THR2, 0);
	    ISP_WRITE_BITS(pIspReg , CAM_AWB_HI_THR	, AWB_HI_THR0, 255);
	    ISP_WRITE_BITS(pIspReg , CAM_AWB_HI_THR	, AWB_HI_THR1, 255);
	    ISP_WRITE_BITS(pIspReg , CAM_AWB_HI_THR	, AWB_HI_THR2, 255);
	    ISP_WRITE_BITS(pIspReg , CAM_AWB_ERR_THR	, AWB_ERR_THR, 0);
	    ISP_WRITE_BITS(pIspReg , CAM_AWB_ROT	, AWB_C, 0x100);
	    ISP_WRITE_BITS(pIspReg , CAM_AWB_ROT	, AWB_S, 0x0);
	    ISP_WRITE_BITS(pIspReg , CAM_AWB_L0_X	, AWB_L0_X_LOW, 0x3388);
	    ISP_WRITE_BITS(pIspReg , CAM_AWB_L0_X	, AWB_L0_X_UP, 0x1388);
	    ISP_WRITE_BITS(pIspReg , CAM_AWB_L0_Y	, AWB_L0_Y_LOW, 0x3388);
	    ISP_WRITE_BITS(pIspReg , CAM_AWB_L0_Y	, AWB_L0_Y_UP, 0x1388);

		state=STATE_INIT;
	}
	if(state==STATE_INIT)
	{
		LogInfo("cctPreflashTest state=STATE_INIT line=%d",__LINE__);
		loadNvram();
		FLASH_PROJECT_PARA prjPara;
		prjPara = cust_getFlashProjectPara(LIB3A_AE_MODE_AUTO, m_pNvram);
		dutyNum = prjPara.dutyNum;
		stepNum = prjPara.stepNum;
		if(prjPara.stepNum>1)
		{
			int dataCnt;
			dutyArr = new int[prjPara.stepNum*8+2];
			stepArr = new int[prjPara.stepNum*8+2];
			ym = new float[10*(prjPara.stepNum*8+2)];
			rm = new float[10*(prjPara.stepNum*8+2)];
			gm = new float[10*(prjPara.stepNum*8+2)];
			bm = new float[10*(prjPara.stepNum*8+2)];
			int ind=0;
			stepArr[ind]=0;
			dutyArr[ind]=-1;
			ind++;
			for(j=0;j<prjPara.stepNum;j++)
			{
				for(i=0;i<8;i++)
				{
					int duty;
					if(i==0 || i==1)
						duty=i;
					else if (i==6)
						duty=prjPara.dutyNum-2;
					else if (i==7)
						duty=prjPara.dutyNum-1;
					else
						duty=(i-1)*(prjPara.dutyNum-2)/5;
					stepArr[ind]=j;
					dutyArr[ind]=duty;
					ind++;
				}
			}
			stepArr[ind]=0;
			dutyArr[ind]=-1;
			ind++;
			testNum = ind;
		}
		else
		{
			dutyArr = new int[prjPara.dutyNum+2];
			stepArr = new int[prjPara.dutyNum+2];
			ym = new float[10*(prjPara.dutyNum+2)];
			rm = new float[10*(prjPara.dutyNum+2)];
			gm = new float[10*(prjPara.dutyNum+2)];
			bm = new float[10*(prjPara.dutyNum+2)];
			for(i=0;i<prjPara.dutyNum+1;i++)
			{
				stepArr[i]=0;
				dutyArr[i]=i-1;
			}
			stepArr[prjPara.dutyNum+1]=0;
			dutyArr[prjPara.dutyNum+1]=-1;
			testNum = prjPara.dutyNum+2;
		}
		state=STATE_AE;
	}
	if(state==STATE_AE)
	{
		LogInfo("cctPreflashTest state=STATE_AE line=%d",__LINE__);
		int aeCycle=10;
		AeIn in;
		AeOut out;
		in.staBuf = para->staBuf;
		in.tar = 150;
		in.tarMax = 180;
		in.tarMin = 120;
		in.frameCount = g_cctPfFrameCount-2;
		in.senType = m_sensorType;
		in.cycle=aeCycle;
		out.isEnd=0;
		if(g_cctPfFrameCount%aeCycle==0)
		{
			StrobeDrv* pStrobe = StrobeDrv::createInstance();
			usleep(15000);
			pStrobe->setDuty(dutyNum-1);
			pStrobe->setStep(stepNum-1);
			pStrobe->setTimeOutTime(300);
			pStrobe->setOnOff(0);
			pStrobe->setOnOff(1);
		}
		else if(g_cctPfFrameCount%aeCycle==2)
		{
			StrobeDrv* pStrobe = StrobeDrv::createInstance();
			pStrobe->setOnOff(0);
			doAe(&in, &out);
		}


		if(out.isEnd==1)
		{
			LogInfo("cctPreflashTest ae_end line=%d",__LINE__);
			StrobeDrv* pStrobe = StrobeDrv::createInstance();
			pStrobe->setOnOff(0);
			state=STATE_AE_POST;
			preStateEndCnt = g_cctPfFrameCount;
			exp = out.exp;
			isp = out.isp;
			afe = out.afe;
		}
	}
	else if(state==STATE_AE_POST)
	{
		LogInfo("cctPreflashTest state=STATE_AE_POST line=%d",__LINE__);
		if(g_cctPfFrameCount>preStateEndCnt+150)
		{
			state=STATE_RATIO;
			preStateEndCnt=g_cctPfFrameCount;
		}
	}
	else if(state==STATE_RATIO)
	{
		LogInfo("cctPreflashTest state=STATE_RATIO line=%d",__LINE__);
		int RatioCycle=80;
		int id;
		int cnt;
		id = (g_cctPfFrameCount-preStateEndCnt-1)/RatioCycle;
		cnt = (g_cctPfFrameCount-preStateEndCnt	-1)%RatioCycle;

		LogInfo("cctPreflashTest state=STATE_RATIO line=%d id=%d %d",__LINE__,id,cnt);

		if(id>=testNum)
		{
			state=STATE_END;
			preStateEndCnt=g_cctPfFrameCount+1;
			goto FUNC_NEXT;
		}
		if(cnt==0)
		{
			StrobeDrv* pStrobe = StrobeDrv::createInstance();
			if(dutyArr[id]>=0)
			{
				pStrobe->setTimeOutTime(300);
				pStrobe->setDuty(dutyArr[id]);
				pStrobe->setStep(stepArr[id]);
				pStrobe->setOnOff(0);
				pStrobe->setOnOff(1);
			}
			else
				pStrobe->setOnOff(0);
		}
		if(cnt<10)
		{
			float rv;
			float gv;
			float bv;
			float yv;
			AWBAE_STAT_T* p;
	    	p = (AWBAE_STAT_T*)para->staBuf;
	    	getMean(p, &yv, &rv, &gv, &bv);
	    	LogInfo("cctPreflashTest-out-rgby\t%5.3f\t%5.3f\t%5.3f\t%5.3f\n",rv,gv,bv,yv);
			rm[id*10+cnt]=rv;
			gm[id*10+cnt]=gv;
			bm[id*10+cnt]=bv;
			ym[id*10+cnt]=yv;
		}
	}
	else if(state==STATE_END)
	{
		LogInfo("cctPreflashTest state=STATE_END line=%d",__LINE__);
		FILE* fp;
		fp = fopen("/sdcard/eng_all.txt","wt");
		fprintf(fp,"duty\tstep\ty\tr\tg\tb\n");
		for(j=0;j<testNum;j++)
		{
			for(i=0;i<10;i++)
			{
				fprintf(fp,"%d\t%d\t%5.3f\t%5.3f\t%5.3f\t%5.3f\n",
					dutyArr[j], stepArr[j], ym[j*10+i], rm[j*10+i], gm[j*10+i], bm[j*10+i]);
			}
		}
		fclose(fp);

		short* ynv;
		short* rgnv;
		short* bgnv;
		ynv = new short[256];
		rgnv = new short[256];
		bgnv = new short[256];
		float y0;
		float r0;
		float g0;
		float b0;
		y0 = ym[5];
		r0 = rm[5];
		g0 = gm[5];
		b0 = bm[5];
		int frmSh=5;
		if(stepNum>1)
		{
			LogInfo("cctPreflashTest stepNum>1 line=%d",__LINE__);
			for(j=0;j<stepNum;j++)
			{
				float xtab[8];
				float ytab[8];
				float rtab[8];
				float gtab[8];
				float btab[8];
				xtab[0]=0;
				xtab[1]=1;
				xtab[2]=(dutyNum-2)/5;
				xtab[3]=2*(dutyNum-2)/5;
				xtab[4]=3*(dutyNum-2)/5;
				xtab[5]=4*(dutyNum-2)/5;
				xtab[6]=dutyNum-2;
				xtab[7]=dutyNum-1;

				ytab[0]=ym[(j*8+1)*10+frmSh];
				ytab[1]=ym[(j*8+1)*10+10+frmSh];
				ytab[2]=ym[(j*8+1)*10+20+frmSh];
				ytab[3]=ym[(j*8+1)*10+30+frmSh];
				ytab[4]=ym[(j*8+1)*10+40+frmSh];
				ytab[5]=ym[(j*8+1)*10+50+frmSh];
				ytab[6]=ym[(j*8+1)*10+60+frmSh];
				ytab[7]=ym[(j*8+1)*10+70+frmSh];

				rtab[0]=rm[(j*8+1)*10+frmSh];
				rtab[1]=rm[(j*8+1)*10+10+frmSh];
				rtab[2]=rm[(j*8+1)*10+20+frmSh];
				rtab[3]=rm[(j*8+1)*10+30+frmSh];
				rtab[4]=rm[(j*8+1)*10+40+frmSh];
				rtab[5]=rm[(j*8+1)*10+50+frmSh];
				rtab[6]=rm[(j*8+1)*10+60+frmSh];
				rtab[7]=rm[(j*8+1)*10+70+frmSh];

				gtab[0]=gm[(j*8+1)*10+frmSh];
				gtab[1]=gm[(j*8+1)*10+10+frmSh];
				gtab[2]=gm[(j*8+1)*10+20+frmSh];
				gtab[3]=gm[(j*8+1)*10+30+frmSh];
				gtab[4]=gm[(j*8+1)*10+40+frmSh];
				gtab[5]=gm[(j*8+1)*10+50+frmSh];
				gtab[6]=gm[(j*8+1)*10+60+frmSh];
				gtab[7]=gm[(j*8+1)*10+70+frmSh];

				btab[0]=bm[(j*8+1)*10+frmSh];
				btab[1]=bm[(j*8+1)*10+10+frmSh];
				btab[2]=bm[(j*8+1)*10+20+frmSh];
				btab[3]=bm[(j*8+1)*10+30+frmSh];
				btab[4]=bm[(j*8+1)*10+40+frmSh];
				btab[5]=bm[(j*8+1)*10+50+frmSh];
				btab[6]=bm[(j*8+1)*10+60+frmSh];
				btab[7]=bm[(j*8+1)*10+70+frmSh];
				for(i=0;i<dutyNum;i++)
				{
					float r2;
					float g2;
					float b2;
					ynv[j*dutyNum+i]=(FlashUtil::flash_calYFromXYTab(8, xtab, ytab, (float)i)-y0)*64;
					int ind2;
					ind2=i;
					if(ind2<3)
					{
						ind2=3;
					}
					r2=FlashUtil::flash_calYFromXYTab(8, xtab, rtab, (float)ind2)-r0;
					g2=FlashUtil::flash_calYFromXYTab(8, xtab, gtab, (float)ind2)-g0;
					b2=FlashUtil::flash_calYFromXYTab(8, xtab, btab, (float)ind2)-b0;
					rgnv[j*dutyNum+i]=r2/g2*1024;
					bgnv[j*dutyNum+i]=b2/g2*1024;
				}
			}//for(j=0;j<stepNum;j++)
		}//	if(prjPara.stepNum>1)
		else
		{
			for(i=0;i<dutyNum;i++)
			{
				float y2;
				float r2;
				float g2;
				float b2;
				y2 = ym[(i+1)*10+frmSh]-y0;
				r2 = rm[(i+1)*10+frmSh]-r0;
				g2 = gm[(i+1)*10+frmSh]-g0;
				b2 = bm[(i+1)*10+frmSh]-b0;
				ynv[i]=y2*64;
				rgnv[i]=r2/g2*1024;
				bgnv[i]=b2/g2*1024;
			}
		}
		cctSetEngTabWithBackup(exp, afe, isp, ynv, rgnv, bgnv);
		delete []ynv;
		delete []rgnv;
		delete []bgnv;

		delete []ym;
		delete []rm;
		delete []gm;
		delete []bm;
		delete []dutyArr;
		delete []stepArr;

		loadNvram();
		FILE* fp2;
		fp2 = fopen("/sdcard/flash_nvdata.bin","wb");
		fwrite(m_pNvram, 1, sizeof(NVRAM_CAMERA_STROBE_STRUCT), fp2);
		fclose(fp2);
		rep->isEnd=1;
		state = STATE_END2;
		goto FUNC_END;
	}
FUNC_NEXT:
	g_cctPfFrameCount++;
FUNC_END:
	return;
}
void FlashMgr::cctPreflashEnd()
{
	/*
	int i;
	int j;


	loadNvram();
	FLASH_PROJECT_PARA prjPara;
	prjPara = cust_getFlashProjectPara(LIB3A_AE_MODE_AUTO, m_pNvram);

	int sz;
	sz = prjPara.engLevel.mfDutyMax+2;

	FILE* fp;
	fp = fopen("/sdcard/a.txt","wt");
	for(i=0;i<sz;i++)
		fprintf(fp,"%5.3f\t%5.3f\t%5.3f\t%5.3f\n",g_rm[i][5],g_gm[i][5],g_bm[i][5],g_ym[i][5]);
	fclose(fp);

	fp = fopen("/sdcard/b.txt","wt");
	for(j=0;j<10;j++)
	{
		for(i=0;i<sz;i++)
			fprintf(fp,"%5.3f\t%5.3f\t%5.3f\t%5.3f\t\t",g_rm[i][j],g_gm[i][j],g_bm[i][j],g_ym[i][j]);
		fprintf(fp,"\n");
	}
	fclose(fp);
	*/

	g_cctPfFrameCount=0;

}

void FlashMgr::cctSetCapDutyStep(int isEn, int duty, int step)
{
	m_cct_isUserDutyStep = isEn;
	m_cct_capDuty = duty;
	m_cct_capStep = step;
}


//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//ACDK_CCT_OP_STROBE_READ_NVRAM,	//5,
int FlashMgr::cctReadNvram(void* in, int inSize, void* out, int outSize, MUINT32* realOutSize)
{
	*realOutSize=0;
	LogInfo("cctReadNvram line=%d inSize=%d outSize=%d realOutSize=%d", __LINE__, inSize, outSize, *realOutSize);
	return loadNvram();
}
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//ACDK_CCT_OP_STROBE_WRITE_NVRAM,	//6
int FlashMgr::cctWriteNvram(void* in, int inSize, void* out, int outSize, MUINT32* realOutSize)
{
	*realOutSize=0;
	LogInfo("cctWriteNvram line=%d inSize=%d outSize=%d realOutSize=%d", __LINE__, inSize, outSize, *realOutSize);
	return writeNvramMain(0);
}
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//ACDK_CCT_OP_STROBE_READ_DEFAULT_NVRAM,	//7
int FlashMgr::cctReadDefaultNvram(void* in, int inSize, void* out, int outSize, MUINT32* realOutSize)
{
	*realOutSize=0;
	LogInfo("cctReadDefaultNvram line=%d inSize=%d outSize=%d realOutSize=%d", __LINE__, inSize, outSize, *realOutSize);
	return loadDefaultNvram();
}
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//ACDK_CCT_OP_STROBE_SET_PARAM,	//8
int FlashMgr::cctSetParam(void* in, int inSize, void* out, int outSize, MUINT32* realOutSize)
{
	*realOutSize=0;
	LogInfo("cctSetParam line=%d inSize=%d outSize=%d realOutSize=%d", __LINE__, inSize, outSize, *realOutSize);
	if(inSize!=12)
	{
		LogError("inSize is wrong");
		return FL_ERR_CCT_INPUT_SIZE_WRONG;
	}
	int id;
	int v1;
	int v2;
	id=((int*)in)[0];
	v1=((int*)in)[1];
	v2=((int*)in)[2];
	loadNvram();
	switch(id)
	{
	case ACDK_FL_CCT_ID_yTar:
		m_pNvram->tuningPara[v2].yTar=v1;
	break;
	case ACDK_FL_CCT_ID_antiIsoLevel:
		m_pNvram->tuningPara[v2].antiIsoLevel=v1;
	break;
	case ACDK_FL_CCT_ID_antiExpLevel:
		m_pNvram->tuningPara[v2].antiExpLevel=v1;
	break;
	case ACDK_FL_CCT_ID_antiStrobeLevel:
		m_pNvram->tuningPara[v2].antiStrobeLevel=v1;
	break;
	case ACDK_FL_CCT_ID_antiUnderLevel:
		m_pNvram->tuningPara[v2].antiUnderLevel=v1;
	break;
	case ACDK_FL_CCT_ID_antiOverLevel:
		m_pNvram->tuningPara[v2].antiOverLevel=v1;
	break;
	case ACDK_FL_CCT_ID_foregroundLevel:
		m_pNvram->tuningPara[v2].foregroundLevel=v1;
	break;
	case ACDK_FL_CCT_ID_isRefAfDistance:
		m_pNvram->tuningPara[v2].isRefAfDistance=v1;
	break;
	case ACDK_FL_CCT_ID_accuracyLevel:
		m_pNvram->tuningPara[v2].accuracyLevel=v1;
	break;
	//------------------------
	case ACDK_FL_CCT_ID_isTorchEngUpdate:
		m_pNvram->isTorchEngUpdate=v1;
	break;
	case ACDK_FL_CCT_ID_isAfEngUpdate:
		m_pNvram->isAfEngUpdate=v1;
	break;
	case ACDK_FL_CCT_ID_isNormaEnglUpdate:
		m_pNvram->isNormaEnglUpdate=v1;
	break;
	case ACDK_FL_CCT_ID_isLowBatEngUpdate:
		m_pNvram->isLowBatEngUpdate=v1;
	break;
	case ACDK_FL_CCT_ID_isBurstEngUpdate:
		m_pNvram->isBurstEngUpdate=v1;
	break;
	//------------------------
	//@@
	case ACDK_FL_CCT_ID_torchEngMode:
		if(v1==ACDK_FL_CCT_ENG_INDEX_MODE)
			m_pNvram->engLevel.torchEngMode=ENUM_FLASH_ENG_INDEX_MODE;
		else if(v1==ACDK_FL_CCT_ENG_CURRENT_MODE)
			m_pNvram->engLevel.torchEngMode=ENUM_FLASH_ENG_CURRENT_MODE;
	break;
	case ACDK_FL_CCT_ID_torchPeakI:
		m_pNvram->engLevel.torchPeakI=v1;
	break;
	case ACDK_FL_CCT_ID_torchAveI:
		m_pNvram->engLevel.torchAveI=v1;
	break;
	case ACDK_FL_CCT_ID_torchDuty:
		m_pNvram->engLevel.torchDuty=v1;
	break;
	case ACDK_FL_CCT_ID_torchStep:
		m_pNvram->engLevel.torchStep=v1;
	break;
	//@@
	case ACDK_FL_CCT_ID_afEngMode:
		if(v1==ACDK_FL_CCT_ENG_INDEX_MODE)
			m_pNvram->engLevel.afEngMode=ENUM_FLASH_ENG_INDEX_MODE;
		else if(v1==ACDK_FL_CCT_ENG_CURRENT_MODE)
			m_pNvram->engLevel.afEngMode=ENUM_FLASH_ENG_CURRENT_MODE;
	break;
	case ACDK_FL_CCT_ID_afPeakI:
		m_pNvram->engLevel.afPeakI=v1;
	break;
	case ACDK_FL_CCT_ID_afAveI:
		m_pNvram->engLevel.afAveI=v1;
	break;
	case ACDK_FL_CCT_ID_afDuty:
		m_pNvram->engLevel.afDuty=v1;
	break;
	case ACDK_FL_CCT_ID_afStep:
		m_pNvram->engLevel.afStep=v1;
	break;
	//@@
	case ACDK_FL_CCT_ID_pmfEngMode:
		if(v1==ACDK_FL_CCT_ENG_INDEX_MODE)
			m_pNvram->engLevel.pmfEngMode=ENUM_FLASH_ENG_INDEX_MODE;
		else if(v1==ACDK_FL_CCT_ENG_CURRENT_MODE)
			m_pNvram->engLevel.pmfEngMode=ENUM_FLASH_ENG_CURRENT_MODE;
	break;
	case ACDK_FL_CCT_ID_pfAveI:
		m_pNvram->engLevel.pfAveI=v1;
	break;
	case ACDK_FL_CCT_ID_mfAveIMax:
		m_pNvram->engLevel.mfAveIMax=v1;
	break;
	case ACDK_FL_CCT_ID_mfAveIMin:
		m_pNvram->engLevel.mfAveIMin=v1;
	break;
	case ACDK_FL_CCT_ID_pmfPeakI:
		m_pNvram->engLevel.pmfPeakI=v1;
	break;
	case ACDK_FL_CCT_ID_pfDuty:
		m_pNvram->engLevel.pfDuty=v1;
	break;
	case ACDK_FL_CCT_ID_mfDutyMax:
		m_pNvram->engLevel.mfDutyMax=v1;
	break;
	case ACDK_FL_CCT_ID_mfDutyMin:
		m_pNvram->engLevel.mfDutyMin=v1;
	break;
	case ACDK_FL_CCT_ID_pmfStep:
		m_pNvram->engLevel.pmfStep=v1;
	break;

	case ACDK_FL_CCT_ID_IChangeByVBatEn:
		m_pNvram->engLevel.IChangeByVBatEn=v1;
	break;
	case ACDK_FL_CCT_ID_vBatL:
		m_pNvram->engLevel.vBatL=v1;
	break;
	case ACDK_FL_CCT_ID_pfAveIL:
		m_pNvram->engLevel.pfAveIL=v1;
	break;
	case ACDK_FL_CCT_ID_mfAveIMaxL:
		m_pNvram->engLevel.mfAveIMaxL=v1;
	break;
	case ACDK_FL_CCT_ID_mfAveIMinL:
		m_pNvram->engLevel.pmfStep=v1;
	break;

	case ACDK_FL_CCT_ID_pmfPeakIL:
		m_pNvram->engLevel.pmfPeakIL=v1;
	break;
	case ACDK_FL_CCT_ID_pfDutyL:
		m_pNvram->engLevel.pfDutyL=v1;
	break;
	case ACDK_FL_CCT_ID_mfDutyMaxL:
		m_pNvram->engLevel.mfDutyMaxL=v1;
	break;
	case ACDK_FL_CCT_ID_mfDutyMinL:
		m_pNvram->engLevel.mfDutyMinL=v1;
	break;
	case ACDK_FL_CCT_ID_pmfStepL:
		m_pNvram->engLevel.pmfStepL=v1;
	break;

	case ACDK_FL_CCT_ID_IChangeByBurstEn:
		m_pNvram->engLevel.IChangeByBurstEn=v1;
	break;
	case ACDK_FL_CCT_ID_pfAveIB:
		m_pNvram->engLevel.pfAveIB=v1;
	break;
	case ACDK_FL_CCT_ID_mfAveIMaxB:
		m_pNvram->engLevel.mfAveIMaxB=v1;
	break;
	case ACDK_FL_CCT_ID_mfAveIMinB:
		m_pNvram->engLevel.mfAveIMinB=v1;
	break;
	case ACDK_FL_CCT_ID_pmfPeakIB:
		m_pNvram->engLevel.pmfPeakIB=v1;
	break;
	case ACDK_FL_CCT_ID_pfDutyB:
		m_pNvram->engLevel.pfDutyB=v1;
	break;
	case ACDK_FL_CCT_ID_mfDutyMaxB:
		m_pNvram->engLevel.mfDutyMaxB=v1;
	break;
	case ACDK_FL_CCT_ID_mfDutyMinB:
		m_pNvram->engLevel.mfDutyMinB=v1;
	break;
	case ACDK_FL_CCT_ID_pmfStepB:
		m_pNvram->engLevel.pmfStepB=v1;
	break;
	case ACDK_FL_CCT_ID_distance:
		m_pNvram->engTab.distance=v1;
	break;

	}
	return 0;
}
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//ACDK_CCT_OP_STROBE_GET_PARAM,	//9
int FlashMgr::cctGetParam(void* in, int inSize, void* out, int outSize, MUINT32* realOutSize)
{
	if(inSize!=8)
	{
		*realOutSize=0;
		LogInfo("cctGetParam line=%d inSize=%d outSize=%d realOutSize=%d", __LINE__, inSize, outSize, *realOutSize);
		LogError("inSize is wrong");
		return FL_ERR_CCT_INPUT_SIZE_WRONG;
	}
	*realOutSize=4;
	LogInfo("cctSetParam line=%d inSize=%d outSize=%d realOutSize=%d", __LINE__, inSize, outSize, *realOutSize);
	int* pIn = (int*)in;
	int* pOut = (int*)out;
	int id = pIn[0];
	int p1 = pIn[1];
	int retV;
	int tmp;
	loadNvram();
	switch(id)
	{
	case ACDK_FL_CCT_ID_yTar:
		retV = m_pNvram->tuningPara[p1].yTar;
	break;
	case ACDK_FL_CCT_ID_antiIsoLevel:
		retV = m_pNvram->tuningPara[p1].antiIsoLevel;
	break;
	case ACDK_FL_CCT_ID_antiExpLevel:
		retV = m_pNvram->tuningPara[p1].antiExpLevel;
	break;
	case ACDK_FL_CCT_ID_antiStrobeLevel:
		retV = m_pNvram->tuningPara[p1].antiStrobeLevel;
	break;
	case ACDK_FL_CCT_ID_antiUnderLevel:
		retV = m_pNvram->tuningPara[p1].antiUnderLevel;
	break;
	case ACDK_FL_CCT_ID_antiOverLevel:
		retV = m_pNvram->tuningPara[p1].antiOverLevel;
	break;
	case ACDK_FL_CCT_ID_foregroundLevel:
		retV = m_pNvram->tuningPara[p1].foregroundLevel;
	break;
	case ACDK_FL_CCT_ID_isRefAfDistance:
		retV = m_pNvram->tuningPara[p1].isRefAfDistance;
	break;
	case ACDK_FL_CCT_ID_accuracyLevel:
		retV = m_pNvram->tuningPara[p1].accuracyLevel;
	break;
	//------------------------
	case ACDK_FL_CCT_ID_isTorchEngUpdate:
		retV = m_pNvram->isTorchEngUpdate;
	break;
	case ACDK_FL_CCT_ID_isAfEngUpdate:
		retV = m_pNvram->isAfEngUpdate;
	break;
	case ACDK_FL_CCT_ID_isNormaEnglUpdate:
		retV = m_pNvram->isNormaEnglUpdate;
	break;
	case ACDK_FL_CCT_ID_isLowBatEngUpdate:
		retV = m_pNvram->isLowBatEngUpdate;
	break;
	case ACDK_FL_CCT_ID_isBurstEngUpdate:
		retV = m_pNvram->isBurstEngUpdate;
	break;
	//------------------------
	//@@
	case ACDK_FL_CCT_ID_torchEngMode:
		tmp = m_pNvram->engLevel.torchEngMode;
		if(tmp==ENUM_FLASH_ENG_INDEX_MODE)
			retV=ACDK_FL_CCT_ENG_INDEX_MODE;
		else if(tmp==ACDK_FL_CCT_ENG_CURRENT_MODE)
			retV=ACDK_FL_CCT_ENG_CURRENT_MODE;
	break;
	case ACDK_FL_CCT_ID_torchPeakI:
		retV = m_pNvram->engLevel.torchPeakI;
	break;
	case ACDK_FL_CCT_ID_torchAveI:
		retV = m_pNvram->engLevel.torchAveI;
	break;
	case ACDK_FL_CCT_ID_torchDuty:
		retV = m_pNvram->engLevel.torchDuty;
	break;
	case ACDK_FL_CCT_ID_torchStep:
		retV = m_pNvram->engLevel.torchStep;
	break;
	//@@
	case ACDK_FL_CCT_ID_afEngMode:
		tmp = m_pNvram->engLevel.afEngMode;
		if(tmp==ENUM_FLASH_ENG_INDEX_MODE)
			retV=ACDK_FL_CCT_ENG_INDEX_MODE;
		else if(tmp==ACDK_FL_CCT_ENG_CURRENT_MODE)
			retV=ACDK_FL_CCT_ENG_CURRENT_MODE;
	break;
	case ACDK_FL_CCT_ID_afPeakI:
		retV = m_pNvram->engLevel.afPeakI;
	break;
	case ACDK_FL_CCT_ID_afAveI:
		retV = m_pNvram->engLevel.afAveI;
	break;
	case ACDK_FL_CCT_ID_afDuty:
		retV = m_pNvram->engLevel.afDuty;
	break;
	case ACDK_FL_CCT_ID_afStep:
		retV = m_pNvram->engLevel.afStep;
	break;
	//@@
	case ACDK_FL_CCT_ID_pmfEngMode:
		tmp = m_pNvram->engLevel.pmfEngMode;
		if(tmp==ENUM_FLASH_ENG_INDEX_MODE)
			retV=ACDK_FL_CCT_ENG_INDEX_MODE;
		else if(tmp==ACDK_FL_CCT_ENG_CURRENT_MODE)
			retV=ACDK_FL_CCT_ENG_CURRENT_MODE;
	break;
	case ACDK_FL_CCT_ID_pfAveI:
		retV = m_pNvram->engLevel.pfAveI;
	break;
	case ACDK_FL_CCT_ID_mfAveIMax:
		retV = m_pNvram->engLevel.mfAveIMax;
	break;
	case ACDK_FL_CCT_ID_mfAveIMin:
		retV = m_pNvram->engLevel.mfAveIMin;
	break;
	case ACDK_FL_CCT_ID_pmfPeakI:
		retV = m_pNvram->engLevel.pmfPeakI;
	break;
	case ACDK_FL_CCT_ID_pfDuty:
		retV = m_pNvram->engLevel.pfDuty;
	break;
	case ACDK_FL_CCT_ID_mfDutyMax:
		retV = m_pNvram->engLevel.mfDutyMax;
	break;
	case ACDK_FL_CCT_ID_mfDutyMin:
		retV = m_pNvram->engLevel.mfDutyMin;
	break;
	case ACDK_FL_CCT_ID_pmfStep:
		retV = m_pNvram->engLevel.pmfStep;
	break;

	case ACDK_FL_CCT_ID_IChangeByVBatEn:
		retV = m_pNvram->engLevel.IChangeByVBatEn;
	break;
	case ACDK_FL_CCT_ID_vBatL:
		retV = m_pNvram->engLevel.vBatL;
	break;
	case ACDK_FL_CCT_ID_pfAveIL:
		retV = m_pNvram->engLevel.pfAveIL;
	break;
	case ACDK_FL_CCT_ID_mfAveIMaxL:
		retV = m_pNvram->engLevel.mfAveIMaxL;
	break;
	case ACDK_FL_CCT_ID_mfAveIMinL:
		retV = m_pNvram->engLevel.pmfStep;
	break;

	case ACDK_FL_CCT_ID_pmfPeakIL:
		retV = m_pNvram->engLevel.pmfPeakIL;
	break;
	case ACDK_FL_CCT_ID_pfDutyL:
		retV = m_pNvram->engLevel.pfDutyL;
	break;
	case ACDK_FL_CCT_ID_mfDutyMaxL:
		retV = m_pNvram->engLevel.mfDutyMaxL;
	break;
	case ACDK_FL_CCT_ID_mfDutyMinL:
		retV = m_pNvram->engLevel.mfDutyMinL;
	break;
	case ACDK_FL_CCT_ID_pmfStepL:
		retV = m_pNvram->engLevel.pmfStepL;
	break;

	case ACDK_FL_CCT_ID_IChangeByBurstEn:
		retV = m_pNvram->engLevel.IChangeByBurstEn;
	break;
	case ACDK_FL_CCT_ID_pfAveIB:
		retV = m_pNvram->engLevel.pfAveIB;
	break;
	case ACDK_FL_CCT_ID_mfAveIMaxB:
		retV = m_pNvram->engLevel.mfAveIMaxB;
	break;
	case ACDK_FL_CCT_ID_mfAveIMinB:
		retV = m_pNvram->engLevel.mfAveIMinB;
	break;
	case ACDK_FL_CCT_ID_pmfPeakIB:
		retV = m_pNvram->engLevel.pmfPeakIB;
	break;
	case ACDK_FL_CCT_ID_pfDutyB:
		retV = m_pNvram->engLevel.pfDutyB;
	break;
	case ACDK_FL_CCT_ID_mfDutyMaxB:
		retV = m_pNvram->engLevel.mfDutyMaxB;
	break;
	case ACDK_FL_CCT_ID_mfDutyMinB:
		retV = m_pNvram->engLevel.mfDutyMinB;
	break;
	case ACDK_FL_CCT_ID_pmfStepB:
		retV = m_pNvram->engLevel.pmfStepB;
	break;
	case ACDK_FL_CCT_ID_distance:
		retV = m_pNvram->engTab.distance;
	break;

	}
	pOut[0] = retV;
	return 0;
}
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//ACDK_CCT_OP_STROBE_GET_NVDATA, 10
int FlashMgr::cctGetNvdata(void* in, int inSize, void* out, int outSize, MUINT32* realOutSize)
{
	int sz= sizeof(NVRAM_CAMERA_STROBE_STRUCT);
	if(outSize!=sz)
	{
		*realOutSize =0;
		LogError("outSize is wrong");
		LogInfo("cctGetNvdata line=%d inSize=%d outSize=%d realOutSize=%d", __LINE__, inSize, outSize, *realOutSize);
		return  FL_ERR_CCT_OUTPUT_SIZE_WRONG;
	}

	*realOutSize = sz;
	LogInfo("cctGetNvdata line=%d inSize=%d outSize=%d realOutSize=%d", __LINE__, inSize, outSize, *realOutSize);
	loadNvram();
	memcpy(out, m_pNvram, *realOutSize);
	return 0;
}

//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//ACDK_CCT_OP_STROBE_SET_NVDATA, 11
int FlashMgr::cctSetNvdata(void* in, int inSize, void* out, int outSize, MUINT32* realOutSize)
{
	*realOutSize = 0;
	LogInfo("cctSetNvdata line=%d inSize=%d outSize=%d realOutSize=%d", __LINE__, inSize, outSize, *realOutSize);
	loadNvram();
	int sz;
	sz = sizeof(NVRAM_CAMERA_STROBE_STRUCT);
	if(inSize!=sz)
	{
		LogError("inSize is wrong");
		return FL_ERR_CCT_INPUT_SIZE_WRONG;
	}
	memcpy(m_pNvram, in, sz);
	return 0;
}

//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//ACDK_CCT_OP_STROBE_GET_ENG_Y,	//12,
int FlashMgr::cctGetEngY(void* in, int inSize, void* out, int outSize, MUINT32* realOutSize)
{
	int sz;
	sz = CCT_FL_ENG_SIZE*CCT_FL_ENG_UNIT_SIZE;
	if(outSize!=sz)
	{
		*realOutSize =0;
		LogError("outSize is wrong");
		LogInfo("cctGetEngY line=%d inSize=%d outSize=%d realOutSize=%d", __LINE__, inSize, outSize, *realOutSize);
		return  FL_ERR_CCT_OUTPUT_SIZE_WRONG;
	}
	*realOutSize =sz;
	LogInfo("cctGetEngY line=%d inSize=%d outSize=%d realOutSize=%d", __LINE__, inSize, outSize, *realOutSize);
	short* tab;
	tab = (short*)out;
	loadNvram();
	FLASH_PROJECT_PARA prjPara;
	prjPara =  cust_getFlashProjectPara(0,0);
	int i;
	for(i=0;i<prjPara.stepNum*prjPara.dutyNum;i++)
	{
		tab[i] = m_pNvram->engTab.yTab[i];
	}
	return 0;
}

//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//ACDK_CCT_OP_STROBE_SET_ENG_Y,	//13
int FlashMgr::cctSetEngY(void* in, int inSize, void* out, int outSize, MUINT32* realOutSize)
{
	*realOutSize=0;
	LogInfo("cctGetEngY line=%d inSize=%d outSize=%d realOutSize=%d", __LINE__, inSize, outSize, *realOutSize);
	int sz;
	sz = CCT_FL_ENG_SIZE*CCT_FL_ENG_UNIT_SIZE;
	if(inSize!=sz)
	{
		LogError("inSize is wrong");
		return FL_ERR_CCT_INPUT_SIZE_WRONG;
	}
	short* tab;
	tab = (short*)in;
	loadNvram();
	FLASH_PROJECT_PARA prjPara;
	prjPara =  cust_getFlashProjectPara(0,0);
	int i;
	for(i=0;i<prjPara.stepNum*prjPara.dutyNum;i++)
	{
		m_pNvram->engTab.yTab[i]=tab[i];
	}
	return 0;
}

//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//ACDK_CCT_OP_STROBE_GET_ENG_RG,	//14
int FlashMgr::cctGetEngRg(void* in, int inSize, void* out, int outSize, MUINT32* realOutSize)
{
	int sz;
	sz = CCT_FL_ENG_SIZE*CCT_FL_ENG_UNIT_SIZE;
	if(outSize!=sz)
	{
		*realOutSize =0;
		LogError("outSize is wrong");
		LogInfo("cctGetEngRg line=%d inSize=%d outSize=%d realOutSize=%d", __LINE__, inSize, outSize, *realOutSize);
		return  FL_ERR_CCT_OUTPUT_SIZE_WRONG;
	}
	*realOutSize =sz;
	LogInfo("cctGetEngRg line=%d inSize=%d outSize=%d realOutSize=%d", __LINE__, inSize, outSize, *realOutSize);
	short* tab;
	tab = (short*)out;
	loadNvram();
	FLASH_PROJECT_PARA prjPara;
	prjPara =  cust_getFlashProjectPara(0,0);
	int i;
	for(i=0;i<prjPara.stepNum*prjPara.dutyNum;i++)
	{
		tab[i] = m_pNvram->engTab.rgTab[i];
	}
	return 0;
}
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//ACDK_CCT_OP_STROBE_SET_ENG_RG,	//15
int FlashMgr::cctSetEngRg(void* in, int inSize, void* out, int outSize, MUINT32* realOutSize)
{
	*realOutSize=0;
	LogInfo("cctSetEngRg line=%d inSize=%d outSize=%d realOutSize=%d", __LINE__, inSize, outSize, *realOutSize);
	int sz;
	sz = CCT_FL_ENG_SIZE*CCT_FL_ENG_UNIT_SIZE;
	if(inSize!=sz)
	{
		LogError("inSize is wrong");
		return FL_ERR_CCT_INPUT_SIZE_WRONG;
	}
	short* tab;
	tab = (short*)in;
	loadNvram();
	FLASH_PROJECT_PARA prjPara;
	prjPara =  cust_getFlashProjectPara(0,0);
	int i;
	for(i=0;i<prjPara.stepNum*prjPara.dutyNum;i++)
	{
		m_pNvram->engTab.rgTab[i]=tab[i];
	}
	return 0;
}
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//ACDK_CCT_OP_STROBE_GET_ENG_BG,	//16
int FlashMgr::cctGetEngBg(void* in, int inSize, void* out, int outSize, MUINT32* realOutSize)
{
	int sz;
	sz = CCT_FL_ENG_SIZE*CCT_FL_ENG_UNIT_SIZE;
	if(outSize!=sz)
	{
		*realOutSize =0;
		LogError("outSize is wrong");
		LogInfo("cctGetEngBg line=%d inSize=%d outSize=%d realOutSize=%d", __LINE__, inSize, outSize, *realOutSize);
		return  FL_ERR_CCT_OUTPUT_SIZE_WRONG;
	}
	*realOutSize =sz;
	LogInfo("cctGetEngBg line=%d inSize=%d outSize=%d realOutSize=%d", __LINE__, inSize, outSize, *realOutSize);
	short* tab;
	tab = (short*)out;
	loadNvram();
	FLASH_PROJECT_PARA prjPara;
	prjPara =  cust_getFlashProjectPara(0,0);
	int i;
	for(i=0;i<prjPara.stepNum*prjPara.dutyNum;i++)
	{
		tab[i] = m_pNvram->engTab.bgTab[i];
	}
	return 0;
}
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//ACDK_CCT_OP_STROBE_SET_ENG_BG,	//17
int FlashMgr::cctSetEngBg(void* in, int inSize, void* out, int outSize, MUINT32* realOutSize)
{
	*realOutSize=0;
	LogInfo("cctSetEngBg line=%d inSize=%d outSize=%d realOutSize=%d", __LINE__, inSize, outSize, *realOutSize);
	int sz;
	sz = CCT_FL_ENG_SIZE*CCT_FL_ENG_UNIT_SIZE;
	if(inSize!=sz)
	{
		LogError("inSize is wrong");
		return FL_ERR_CCT_INPUT_SIZE_WRONG;
	}
	short* tab;
	tab = (short*)in;
	loadNvram();
	FLASH_PROJECT_PARA prjPara;
	prjPara =  cust_getFlashProjectPara(0,0);
	int i;
	for(i=0;i<prjPara.stepNum*prjPara.dutyNum;i++)
	{
		m_pNvram->engTab.bgTab[i]=tab[i];
	}
	return 0;
}
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//ACDK_CCT_OP_STROBE_NVDATA_TO_FILE,	//18
int FlashMgr::FlashMgr::cctNvdataToFile(void* in, int inSize, void* out, int outSize, MUINT32* realOutSize)
{
	*realOutSize = 0;
	LogInfo("cctNvdataToFile line=%d inSize=%d outSize=%d realOutSize=%d", __LINE__, inSize, outSize, *realOutSize);
	char* fname;
	fname = (char*)in;
	FILE* fp;
	fp = fopen(fname, "wb");
	if(fp==0)
	{
		LogError("file not exist");
		return FL_ERR_CCT_FILE_NOT_EXIST;
	}
	loadNvram();
	int sz;
	sz = sizeof(NVRAM_CAMERA_STROBE_STRUCT);
	fwrite(m_pNvram, 1, sz, fp);
	fclose(fp);
	return 0;
}
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//ACDK_CCT_OP_STROBE_FILE_TO_NVDATA,	//19
int FlashMgr::cctFileToNvdata(void* in, int inSize, void* out, int outSize, MUINT32* realOutSize)
{
	*realOutSize = 0;
	LogInfo("cctFileToNvdata line=%d inSize=%d outSize=%d realOutSize=%d", __LINE__, inSize, outSize, *realOutSize);
	char* fname;
	fname = (char*)in;
	FILE* fp;
	fp = fopen(fname, "rb");
	if(fp==0)
	{
		LogError("file not exist");
		return FL_ERR_CCT_FILE_NOT_EXIST;
	}
	loadNvram();
	int sz;
	sz = sizeof(NVRAM_CAMERA_STROBE_STRUCT);
	fread(m_pNvram, 1, sz, fp);
	fclose(fp);
	return 0;
}