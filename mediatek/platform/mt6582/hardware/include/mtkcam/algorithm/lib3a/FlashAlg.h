// FlashAlg.h: interface for the FlashAlg class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FLASHALG_H__B5C93207_0BD8_4488_85C9_47A3A86829E5__INCLUDED_)
#define AFX_FLASHALG_H__B5C93207_0BD8_4488_85C9_47A3A86829E5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


struct FlashAlgStrobeProfile
{
public:
	float iso;
	int exp;
	float distance;
	int dutyNum;
	int stepNum;
	int dutyTickNum;
	int stepTickNum;
	int* dutyTick;
	int* stepTick;
	float* engTab;
};

struct FlashAlgStrobeLim
{
	int minStep;
	int maxStep;
	int minDuty;
	int maxDuty;
};

struct FlashAlgExpPara
{
	int exp;
	int iso;
	int isFlash;
	int step;
	int duty;
};

struct FlashAlgStaData
{
	int row;
	int col;
	int dig_row;
	int dig_col;
	short* data;
	double normalizeFactor;
	int bit;
};

struct FlashAlgPLineNode
{
	int iso;
	int exp;
};


typedef struct	evSetting
{
    MUINT32 u4Eposuretime;   //!<: Exposure time in ms
    MUINT32 u4AfeGain;       //!<: raw gain
    MUINT32 u4IspGain;       //!<: sensor gain
    MUINT8  uIris;           //!<: Iris
    MUINT8  uSensorMode;     //!<: sensor mode
    MUINT8  uFlag;           //!<: flag to indicate hysteresis ...
//    MUINT8  uLV;                        //!<: LV avlue , in ISO 100 condition  LV=TV+AV
}evSetting;


typedef struct	PLine
{
	MUINT32       u4TotalIndex;      //preview table Tatal index
	MINT32        i4StrobeTrigerBV;  // Strobe triger point in strobe auto mode
	MINT32        i4MaxBV;
	MINT32        i4MinBV;
	evSetting *pCurrentTable;   //point to current table
}PLine;


enum
{
	FlashAlg_Err_Ok=0,
	FlashAlg_Err_NoMem=-100,
	FlashAlg_Err_Div0=-101,
	FlashAlg_Err_Para=-102,
	FlashAlg_Err_Other=-103,
	FlashAlg_Err_CaliDataNotSet=-104,
	FlashAlg_Stage_Af=1,
	FlashAlg_Stage_Capture=0,
};

class FlashAlg
{
public:
	virtual ~FlashAlg();
	static FlashAlg* getInstance();

	//procedure (must call)
	virtual void Estimate(FlashAlgExpPara* exp)=0;
	virtual int Reset()=0;
	virtual int ResetIntermediate()=0;
	virtual int ResetReport()=0;
	virtual int AddStaData10(FlashAlgStaData* data, FlashAlgExpPara* exp, int* isNeedNext, FlashAlgExpPara* expNext)=0;
	virtual int CalFirstEquAEPara(FlashAlgExpPara* exp, FlashAlgExpPara* EquExp)=0;
	virtual int setStrobeMaxDutyStep(int PreEquDuty, int PreMaxStep, int CapEquDuty, int CapMaxStep)=0;
	virtual int setStrobeMinDutyStep(int CapMinDuty, int CapMinStep)=0;
	virtual int setFlashProfile(FlashAlgStrobeProfile* pr)=0;
	virtual int setCapturePLine(int num, FlashAlgPLineNode* nodes)=0;
	virtual int setPreflashPLine(int num, FlashAlgPLineNode* nodes)=0;

	virtual int setCapturePLine(PLine* p, int isoAtGain1x)=0;
	virtual int setPreflashPLine(PLine* p, int isoAtGain1x)=0;


	virtual int setEVComp(float ev_comp)=0;
	virtual int setEVCompEx(float ev_comp, float tar, float evLevel)=0;
	virtual float calFlashEng(int duty, int rStep)=0;
	//checkInputParaError
	//setDebugDataSize


	//strobe cali
	virtual int setCaliData(int caliNum, int* caliStep, int* caliVBat_mV, int* caliIFlash_mA, int refIRange, int extrapIRange)=0;
	virtual int calStepDuty(int Bat_mV, int peak_mA, int ave_mA, int* step, int* duty)=0;



	//preference (optional setting)
	virtual int setMeasuredDistanceCM(int distance)=0;
	virtual int setWTable256(int w, int h, short* tab)=0;
	virtual int setSaftyExp(int expUs)=0;
	virtual int setYTarget(int tar, int bit)=0;
	virtual int setMaxCaptureIso(int iso)=0;
	virtual int setIsRefDistance(int isRefDistance)=0;

	virtual int setDefaultPreferences()=0;
	virtual int SetAccuracyLevel(int level)=0;//level -10: speed
	virtual int setIsoSuppressionLevel(int level)=0;
	virtual int setExpSuppressionLevel(int level)=0;
	virtual int setStrobeSuppressionLevel(int level)=0;
	virtual int setUnderSuppressionLevel(int level)=0;
	virtual int setOverSuppressionLevel(int level)=0;
	virtual int setForegroundWIncreaseLevel(int level)=0;

	//debug
	virtual int setIsSaveSimBinFile(int isSaveBin)=0;
	virtual int setDebugDir(const char* DirName, const char* PrjName)=0;
	virtual void getLastErr(int* time, int* type, int* reserve)=0; //max 3
	virtual void getLastWarning(int* time, int* type, int* reserve)=0; //max 3
	virtual void fillDebugData2(void* data)=0; //500 bytes
	virtual int checkInputParaError(int* num, int* errBuf)=0; //num: input and output
	virtual void setDebugDataSize(int sz)=0; //should be set initially for check the size in the alg.
	virtual int setYTargetWeight(int weight)=0;
	virtual int setLowReflectaneThreshold(int threshold)=0;
	virtual int setLowReflectaneTuningEnable(int enable)=0;
	virtual int setflashReflectanceWeight(int weight)=0;  // SC NEW ADD
};

#endif // !defined(AFX_FLASHALG_H__B5C93207_0BD8_4488_85C9_47A3A86829E5__INCLUDED_)
