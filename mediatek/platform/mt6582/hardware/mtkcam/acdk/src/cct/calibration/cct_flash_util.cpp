
#define LOG_TAG "cct_flash_cali.cpp"

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include <sys/time.h>
#include <unistd.h>

extern "C" {
#include <linux/fb.h>
#include <linux/mtkfb.h>
}

#include "AcdkLog.h"
#include "cct_ctrl.h"
#include "cct_calibration.h"

#include <mtkcam/acdk/cct_feature.h>
#include <mtkcam/drv/isp_drv.h>
#include "cct_main.h"
#include "awb_param.h"
#include "af_param.h"
#include "ae_param.h"
#include "dbg_isp_param.h"
#include "dbg_aaa_param.h"
#include "flash_mgr.h"
#include "isp_tuning_mgr.h"
#include "isp_mgr.h"
#include "flash_tuning_custom.h"
#include <sys/stat.h>

#include <ctype.h>


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
#include "cct_flash_util.h"
#include "cct_flash_util.h"
static int arrayToBmp(const char* f, void* pData, int pixByte, int w, int h, int pxMax);
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

int flcUtil::getMs()
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


static int isInt(const char* s)
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
int flcUtil::setFileCount(const char* fname, int cnt)
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


int flcUtil::getFileCount(const char* fname, int* fcnt, int defaultValue)
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
