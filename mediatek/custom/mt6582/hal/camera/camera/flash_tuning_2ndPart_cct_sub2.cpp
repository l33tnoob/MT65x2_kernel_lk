#define LOG_TAG "flash_custom_cct_s2.cpp"
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
int getDefaultStrobeNVRam_sub2(void* data, int* ret_size)
{
	XLOGD("getDefaultStrobeNVRam_sub2");
	//static NVRAM_CAMERA_STROBE_STRUCT strobeNVRam;
	//XLOGD("copyTuningPara ytar =%d %d", p->yTar ,nv_p->yTar);
	NVRAM_CAMERA_STROBE_STRUCT* p;
	p = (NVRAM_CAMERA_STROBE_STRUCT*)data;

	static short engTab[]=
    {
        100,200,600,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,1,
		1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,2,
		1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,3,
		1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,4,
		1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,5,
		1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,6,
		1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,7,
		1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,8,
    };

	//version
	p->u4Version = NVRAM_CAMERA_STROBE_FILE_VERSION;
	//eng tab
	memcpy(p->engTab.yTab, engTab, 256*sizeof(short));
	p->engTab.exp =20000;
	p->engTab.afe_gain = 1024;
	p->engTab.isp_gain = 1024;
	p->engTab.distance = 300; //mm
	//tuning para
	p->tuningPara[0].yTar = 188;
	p->tuningPara[0].antiIsoLevel = -5;
	p->tuningPara[0].antiExpLevel = -8;
	p->tuningPara[0].antiStrobeLevel = -10;
	p->tuningPara[0].antiUnderLevel = 0;
	p->tuningPara[0].antiOverLevel = 3;
	p->tuningPara[0].foregroundLevel = 3;
	p->tuningPara[0].isRefAfDistance = 0;
	p->tuningPara[0].accuracyLevel = 10;

	p->tuningPara[1].yTar = 188;
	p->tuningPara[1].antiIsoLevel = -5;
	p->tuningPara[1].antiExpLevel = -8;
	p->tuningPara[1].antiStrobeLevel = -10;
	p->tuningPara[1].antiUnderLevel = 0;
	p->tuningPara[1].antiOverLevel = 3;
	p->tuningPara[1].foregroundLevel = 3;
	p->tuningPara[1].isRefAfDistance = 0;
	p->tuningPara[1].accuracyLevel = 10;

	p->tuningPara[2].yTar = 188;
	p->tuningPara[2].antiIsoLevel = -5;
	p->tuningPara[2].antiExpLevel = -8;
	p->tuningPara[2].antiStrobeLevel = -10;
	p->tuningPara[2].antiUnderLevel = 0;
	p->tuningPara[2].antiOverLevel = 3;
	p->tuningPara[2].foregroundLevel = 3;
	p->tuningPara[2].isRefAfDistance = 0;
	p->tuningPara[2].accuracyLevel = 10;

	p->tuningPara[3].yTar = 188;
	p->tuningPara[3].antiIsoLevel = -5;
	p->tuningPara[3].antiExpLevel = -8;
	p->tuningPara[3].antiStrobeLevel = -10;
	p->tuningPara[3].antiUnderLevel = 0;
	p->tuningPara[3].antiOverLevel = 3;
	p->tuningPara[3].foregroundLevel = 3;
	p->tuningPara[3].isRefAfDistance = 0;
	p->tuningPara[3].accuracyLevel = 10;
	//is eng level used (or by firmware)
	p->isTorchEngUpdate =0;
	p->isNormaEnglUpdate =0;
	p->isLowBatEngUpdate =0;
	p->isBurstEngUpdate =0;
	//eng level
	memset(&p->engLevel, 0, sizeof(FLASH_ENG_LEVEL));

	*ret_size = sizeof(NVRAM_CAMERA_STROBE_STRUCT);
	return 0;
}

