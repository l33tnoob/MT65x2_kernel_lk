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
#include <ctype.h>
#include <sys/stat.h>

#include <ctype.h>
enum
{
	FILE_COUNT_FILE_NOT_FOUND = -1,
	FILE_COUNT_FILE_NOT_CONTAIN_INT = -2,
};
FlashUtil* FlashUtil::getInstance()
{
	static FlashUtil obj;
	return &obj;
}
FlashUtil::FlashUtil()
{

}
FlashUtil::~FlashUtil()
{

}


int isInt(const char* s)
{
  bool isInt = true;
  int slen;
  slen = strlen(s);
    for(int i = 0; i < slen; i++)
	{
      if(!isdigit(s[i]))
	  {
        isInt = false;
		break;
	  }
    }
  return isInt;
}


#include <cutils/properties.h>
int FlashUtil::getPropInt(const char* sId, int DefVal)
{
	int ret;
	char ss[PROPERTY_VALUE_MAX];
	char sDef[20];
	sprintf(sDef,"%d",DefVal);
	property_get(sId, ss, sDef);
	ret = atoi(ss);
	return ret;
}





int FlashUtil::getMs()
{
	//	max:
	//	2147483648 digit
	//	2147483.648 second
	//	35791.39413 min
	//	596.5232356 hour
	//	24.85513481 day
	//int t;
	//struct timeval tv;
	//gettimeofday(&tv, NULL);
	//t = (tv.tv_sec*1000 + (tv.tv_usec+500)/1000);

	int t;
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	t = (ts.tv_sec*1000+ts.tv_nsec/1000000);

	return t;
}



void FlashUtil::createDir(const char* dir)
{
#ifdef WIN32
	CreateDirectory(dir,0);
#else
	mkdir(dir,S_IRWXU | S_IRWXG | S_IRWXO);

#endif

}
void FlashUtil::aaSub(void* arr, int* sub)
{
	int AA_W=120;
	int AA_H=90;
	AWBAE_STAT_T* p;
	p = (AWBAE_STAT_T*)arr;
	int xi;
	int yi;
	int yy;

	int i;
	int j;
	for(i=0;i<25;i++)
	{
		sub[i]=0;
	}


	for(j=0;j<AA_H;j++)
	for(i=0;i<AA_W;i++)
	{
		yy=p->LINE[j].AE_WIN[i];
		xi = i/24;
		yi = j/18;
		sub[yi*5+xi]+=yy;
	}

	for(i=0;i<25;i++)
	{
		sub[i]/=12*9;
	}


}
int FlashUtil::aaToBmp(void* arr, const char* aeF, const char* awbF)
{
	AWBAE_STAT_T* p;
	p = (AWBAE_STAT_T*)arr;
	int ret=0;
	int i;
	int j;
	short* pR;
	short* pG;
	short* pB;
	short* pY;
	int AA_W=120;
	int AA_H=90;
	pY = new short[AA_H*AA_W];
	pR = new short[AA_H*AA_W];
	pG = new short[AA_H*AA_W];
	pB = new short[AA_H*AA_W];
	int ind=0;
	for(j=0;j<AA_H;j++)
	for(i=0;i<AA_W;i++)
	{
		pY[ind]=p->LINE[j].AE_WIN[i];
		pR[ind]=p->LINE[j].AWB_WIN[i].rMainStat.ucR;
		pG[ind]=p->LINE[j].AWB_WIN[i].rMainStat.ucG;
		pB[ind]=p->LINE[j].AWB_WIN[i].rMainStat.ucB;
		if(pY[ind]>255)
			pY[ind]=255;
		if(pR[ind]>255)
			pR[ind]=255;
		if(pG[ind]>255)
			pB[ind]=255;
		if(pB[ind]>255)
			pB[ind]=255;
		ind++;
	}
	int e=0;
/*
	if(aeF!=0)
		e = arrayToGrayBmp(aeF, pY, 2, AA_W, AA_H, 255);
	if(ret==0)
		ret=e;
	if(awbF!=0)
		e = arrayToColorBmp(awbF, pR, pG, pB, 2, AA_W, AA_H, 255);
*/
	if(ret==0)
		ret=e;
	delete []pY;
	delete []pR;
	delete []pG;
	delete []pB;
	return ret;
}
int FlashUtil::setFileCount(const char* fname, int cnt)
{
	int err=0;
	FILE* fp;
	fp = fopen(fname, "wt");
	if(fp==NULL)
	{
		err = FILE_COUNT_FILE_NOT_FOUND;
	}
	else
	{
		fprintf(fp,"%d",cnt);
		fclose(fp);
	}
	return err;
}


int FlashUtil::getFileCount(const char* fname, int* fcnt, int defaultValue)
{
	int err=0;
	int v;
	FILE* fp;
	fp = fopen(fname, "rb");
	if(fp==NULL)
	{
		err = FILE_COUNT_FILE_NOT_FOUND;
	}
	else
	{
		char s[101];
		fscanf(fp,"%100s",s);
		fclose(fp);
		int bNum;
		bNum = isInt(s);
		if(bNum==(int)true)
		{
			v=atoi(s);
		}
		else
		{
			err = FILE_COUNT_FILE_NOT_CONTAIN_INT;
		}
	}
	if(err==0)
		*fcnt = v;
	else
		*fcnt = defaultValue;

	return err;
}
