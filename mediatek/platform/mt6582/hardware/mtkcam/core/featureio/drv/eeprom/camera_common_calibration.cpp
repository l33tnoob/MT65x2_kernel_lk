
#include <stdlib.h>
#include <math.h>

#include <cutils/xlog.h>
#include "camera_custom_eeprom.h"
#include "camera_common_calibration.h"
//#ifdef LOG_TAG
//#undef LOG_TAG
//#endif
#define EEPROM_LOG_TAG "CAM_COMM_CAL"


#define MHAL_LOG(fmt, arg...)    XLOGD(EEPROM_LOG_TAG fmt, ##arg)

#define EEPROM_LSC_ID_SLIM 0x010200FF
#define EEPROM_LSC_ID_DYNAMIC 0x31520000
#define EEPROM_LSC_ID_FIX 0x39333236
#define EEPROM_LSC_ID_SENSOR 0x39333236
#define EEPROM_LSC_ID_SVD 0x010000FF
#define EEPROM_LSC_ID_NO 5

MUINT32 CamCommDoISPSlimShadingLoad(PEEPROM_SHADING_STRUCT pLscTbl);
MUINT32 CamCommDoISPDynamicShadingLoad(PEEPROM_SHADING_STRUCT pLscTbl);
MUINT32 CamCommDoISPFixShadingLoad(PEEPROM_SHADING_STRUCT pLscTbl);
MUINT32 CamCommDoISPSensorShadingLoad(PEEPROM_SHADING_STRUCT pLscTbl);
MUINT32 CamCommDoISPSvdShadingLoad(PEEPROM_SHADING_STRUCT pLscTbl);

CAM_COMM_LSC_CONVERT lCamCommConvertTable[EEPROM_LSC_ID_NO]={
    { EEPROM_LSC_ID_SLIM		, CamCommDoISPSlimShadingLoad},
    { EEPROM_LSC_ID_DYNAMIC		, CamCommDoISPDynamicShadingLoad},
    { EEPROM_LSC_ID_FIX			, CamCommDoISPFixShadingLoad},
    { EEPROM_LSC_ID_SENSOR		, CamCommDoISPSensorShadingLoad},
    { EEPROM_LSC_ID_SVD			, CamCommDoISPSvdShadingLoad}
};

////sl 110317Add for WCP1 slim mode, need to remove to lib
#define GRID_MAX 16//10
#define GN_BIT 13 /* pixel gain decimal bit */
#define COEF_BASE_BIT 15 /* it dangerous when (COEF_BASE_BIT+3)+(COEF_DIV_BIT) close to 32 bit */
#define COEF_DIV_BIT 10 /* 1/3 & 1/6 decimal bit, 1/3= 341/1024 */
#define COEF_DIV6_VAL ((1<<COEF_DIV_BIT)/6.0+0.5)
#define COEF_DIV3_VAL ((1<<COEF_DIV_BIT)/3.0+0.5)
#define COEF_FXX_BIT (COEF_BASE_BIT-GN_BIT)
#define COEF_BIT0 9 /* decimal bit of coef a31,a13,a30,a21,a12,a03,a20,a11,a02 */
#define COEF_BIT1 8 /* decimal bit of coef a10 a01 */
#define COEF_BIT2 7 /* decimal bit of coef a00 */

typedef struct {
	MINT32 grid_num; // grid number
	MINT32 di;	// d[i]=x[i]-x[i-1]
	MINT32 dn;	// d[n]=x[n]-x[n-1], n=GRID_N or GRID_M
	MINT32 di2;	// d[i]*d[i]
	MINT32 dn2;	// d[n]*d[n]
	float dir;	// 1/d[i]
	float dnr;	// 1/d[n]
	float di2r; // 1/(d[i]*d[i])
	float ddir;	// 1/(x[i+1]-x[i-1])
	float ddnr;	// 1/(x[n+1]-x[n-1]), n=GRID_N-1 or GRID_M-1
} DIM_INFO_T;
typedef struct {
	MINT32 dx2;
	MINT32 dy2;
	MINT32 f00;
	MINT32 f01;
	MINT32 f10;
	MINT32 f11;
	float f00x2;
	float f01x2;
	float f10x2;
	float f11x2;
	float f00y2;
	float f01y2;
	float f10y2;
	float f11y2;
} BLK_INFO_T;


////for SLIM TABLE>>>
/////////////Protected
MINT32 coef[4][(GRID_MAX-1)*(GRID_MAX-1)][12]; // 4x9x9x12=3888words
MINT32 zz[4][GRID_MAX][GRID_MAX]; // 4x10x10=400words
float zh2[GRID_MAX][GRID_MAX]; // 100 words
float zv2[GRID_MAX][GRID_MAX]; // 100 words
float y2[GRID_MAX]; // 10 words
MINT32 zzb[GRID_MAX]; // 10 words
float yx2[GRID_MAX]; // 10 words
float yy2[GRID_MAX]; // 10 words
/*

void lscCalTbl(TBL_INFO_T tbl_info);
void resize_slim_shading_table(MUINT16 data_src_width, MUINT16 data_src_height,
				               MUINT16 data_tar_width, MUINT16 data_tar_height,
				               MUINT32 *src_data, MUINT32 *tar_data);
void rotate_slim_shading_table(MUINT16 block_number_x, MUINT16 block_number_y, MUINT32 *data);
*/
/////////////Protected
///>>>
//#if 1 //Here is ported from WCP1/isp_lsc_cali_core_v2.c, We should move lib part

#define CLAMP16(value) ((MINT32)(value)>65535 ? 65535 : ((MINT32)(value) <0 ? 0 : (MUINT16)(value)))

    MUINT8 bicubic_coeff3[64]=
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01,
     0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x02,
     0x03, 0x03, 0x03, 0x04, 0x04, 0x04, 0x05, 0x05,
     0x06, 0x06, 0x07, 0x08, 0x08, 0x09, 0x0A, 0x0A,
     0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12,
     0x13, 0x14, 0x16, 0x17, 0x18, 0x1A, 0x1B, 0x1D,
     0x1E, 0x20, 0x21, 0x23, 0x25, 0x27, 0x29, 0x2B
    };

    MUINT8 bicubic_coeff2[64]=
    {0x2D, 0x2F, 0x31, 0x33, 0x35, 0x38, 0x3A, 0x3C,
     0x3F, 0x41, 0x44, 0x46, 0x49, 0x4B, 0x4E, 0x51,
     0x53, 0x56, 0x59, 0x5B, 0x5E, 0x61, 0x63, 0x66,
     0x69, 0x6B, 0x6E, 0x70, 0x73, 0x76, 0x78, 0x7B,
     0x7D, 0x80, 0x82, 0x84, 0x87, 0x89, 0x8A, 0x8E,
     0x90, 0x91, 0x93, 0x96, 0x97, 0x99, 0x9B, 0x9C,
     0x9E, 0xA1, 0xA1, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7,
     0xA8, 0xA8, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA
    };

    MUINT8 bicubic_coeff1[64]=
    {0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xA8, 0xA8, 0xA7,
     0xA6, 0xA5, 0xA4, 0xA3, 0xA1, 0xA1, 0x9E, 0x9C,
     0x9B, 0x99, 0x97, 0x96, 0x93, 0x91, 0x90, 0x8E,
     0x8A, 0x89, 0x87, 0x84, 0x82, 0x80, 0x7D, 0x7B,
     0x78, 0x76, 0x73, 0x70, 0x6E, 0x6B, 0x69, 0x66,
     0x63, 0x61, 0x5E, 0x5B, 0x59, 0x56, 0x53, 0x51,
     0x4E, 0x4B, 0x49, 0x46, 0x44, 0x41, 0x3F, 0x3C,
     0x3A, 0x38, 0x35, 0x33, 0x31, 0x2F, 0x2D, 0x2B
    };

    MUINT8 bicubic_coeff0[64]=
    {0x29, 0x27, 0x25, 0x23, 0x21, 0x20, 0x1E, 0x1D,
     0x1B, 0x1A, 0x18, 0x17, 0x16, 0x14, 0x13, 0x12,
     0x11, 0x10, 0x0F, 0x0E, 0x0D, 0x0C, 0x0B, 0x0A,
     0x0A, 0x09, 0x08, 0x08, 0x07, 0x06, 0x06, 0x05,
     0x05, 0x04, 0x04, 0x04, 0x03, 0x03, 0x03, 0x02,
     0x02, 0x02, 0x02, 0x01, 0x01, 0x01, 0x01, 0x01,
     0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };

void lscGet2ndDerivative(DIM_INFO_T d, int y[], float y2[])
{
    MINT32 i,n = d.grid_num;
    float p,s,v[GRID_MAX];

    v[0]=0.0;
    for (i=1;i<n-2;i++)	/* i=1~n-3, the same dx */
    {
    	v[i]=(float) (v[i-1]*y2[i]-6.0*(float)(y[i+1]-y[i]-y[i]+y[i-1])*y2[i]*d.di2r/8192.0);
    }
    {/* i=n-2 */
    i = n-2;
    s=(d.di)*(d.ddnr);
    p=s*y2[i-1]+2.0;
    v[i]=(float) (((float)(y[i+1]-y[i])*(d.dnr) - (float)(y[i]-y[i-1])*(d.dir))/8192.0);
    v[i]=(float) ((6.0*v[i]*(d.ddnr)-s*v[i-1])/p);
    }
    /* get final 2nd-derivates */
    for (i=n-2;i>=0;i--)
    {
    	y2[i]=y2[i]*y2[i+1]+v[i];
    }
}


void lscGetRectCoef(BLK_INFO_T binfo, int coef[])
{
    MINT32 f00, f10, f01, f11;
    MINT32 sx2, sy2;
    MINT32 f00x2, f10x2, f01x2, f11x2, f00y2, f10y2, f01y2, f11y2;
    MINT32 a31, a13, a30, a21, a12, a03, a20, a11, a02, a10, a01, a00;
    MINT32 b3, b2, b1, c3, c2;
    MINT32 fbit=COEF_FXX_BIT;
    MINT32 val_div6=(int)COEF_DIV6_VAL;
    MINT32 val_div3=(int)COEF_DIV3_VAL;

    /* shift f** to *.COEF_BASE_BIT bit, max value of fxx is (1<<COEF_BASE_BIT)*8 about (1<<(COEF_BASE_BIT+3)) bit*/
    if(fbit>=0)
    {
    	f00 = binfo.f00<<fbit;
    	f10 = binfo.f10<<fbit;
    	f01 = binfo.f01<<fbit;
    	f11 = binfo.f11<<fbit;
    }
    else
    {
    	f00 = binfo.f00>>(-fbit);
    	f10 = binfo.f10>>(-fbit);
    	f01 = binfo.f01>>(-fbit);
    	f11 = binfo.f11>>(-fbit);
    }
    sx2=binfo.dx2;
    sy2=binfo.dy2;
    /* shift f**x2,f**y2 to 0.COEF_BASE_BIT bit*/
    sx2<<=COEF_BASE_BIT;
    sy2<<=COEF_BASE_BIT;
    f00x2 = (int)((float)sx2 * binfo.f00x2);
    f10x2 = (int)((float)sx2 * binfo.f10x2);
    f01x2 = (int)((float)sx2 * binfo.f01x2);
    f11x2 = (int)((float)sx2 * binfo.f11x2);

    f00y2 = (int)((float)sy2 * binfo.f00y2);
    f10y2 = (int)((float)sy2 * binfo.f10y2);
    f01y2 = (int)((float)sy2 * binfo.f01y2);
    f11y2 = (int)((float)sy2 * binfo.f11y2);

    /* all coef to *.(COEF_BASE_BIT+COEF_DIV_BIT) bit*/
    a30 = (f10x2 - f00x2)*val_div6;
    a20 = (f00x2<<(COEF_DIV_BIT-1));
    a10 = ((f10-f00)<<COEF_DIV_BIT) - (f00x2*val_div3) - (f10x2*val_div6);

    a03 = (f01y2 - f00y2)*val_div6;
    a02 = (f00y2<<(COEF_DIV_BIT-1));
    a01 = ((f01-f00)<<COEF_DIV_BIT) - (f00y2*val_div3) - (f01y2*val_div6);

    b3  = (f11x2 - f01x2)*val_div6;
    b2  = (f01x2<<(COEF_DIV_BIT-1));
    b1  = ((f11-f01)<<COEF_DIV_BIT) - (f01x2*val_div3) - (f11x2*val_div6);

    c3  = (f11y2 - f10y2)*val_div6;
    c2  = (f10y2<<(COEF_DIV_BIT-1));

    a31 = b3 - a30;
    a13 = c3 - a03;
    a21 = b2 - a20;
    a12 = c2 - a02;
    a11 = b1 - a10 - a13 - a12;

    /* COEF_FACTOR0 */
    a31>>=(COEF_BASE_BIT+COEF_DIV_BIT-COEF_BIT0-1); a31+=1; a31>>=1;
    a13>>=(COEF_BASE_BIT+COEF_DIV_BIT-COEF_BIT0-1); a13+=1; a13>>=1;
    a30>>=(COEF_BASE_BIT+COEF_DIV_BIT-COEF_BIT0-1); a30+=1; a30>>=1;
    a21>>=(COEF_BASE_BIT+COEF_DIV_BIT-COEF_BIT0-1); a21+=1; a21>>=1;
    a12>>=(COEF_BASE_BIT+COEF_DIV_BIT-COEF_BIT0-1); a12+=1; a12>>=1;
    a03>>=(COEF_BASE_BIT+COEF_DIV_BIT-COEF_BIT0-1); a03+=1; a03>>=1;
    a20>>=(COEF_BASE_BIT+COEF_DIV_BIT-COEF_BIT0-1); a20+=1; a20>>=1;
    a11>>=(COEF_BASE_BIT+COEF_DIV_BIT-COEF_BIT0-1); a11+=1; a11>>=1;
    a02>>=(COEF_BASE_BIT+COEF_DIV_BIT-COEF_BIT0-1); a02+=1; a02>>=1;
    /* COEF_FACTOR1 */
    a10>>=(COEF_BASE_BIT+COEF_DIV_BIT-COEF_BIT1-1); a10+=1; a10>>=1;
    a01>>=(COEF_BASE_BIT+COEF_DIV_BIT-COEF_BIT1-1); a01+=1; a01>>=1;
    /* COEF_FACTOR2 */
    f00>>=(COEF_BASE_BIT-COEF_BIT2-1); f00+=1; f00>>=1; a00=f00;

    coef[ 0]=a31;coef[ 1]=a13;coef[ 2]=a30;
    coef[ 3]=a21;coef[ 4]=a12;coef[ 5]=a03;
    coef[ 6]=a20;coef[ 7]=a11;coef[ 8]=a02;
    coef[ 9]=a10;coef[10]=a01;coef[11]=a00;
}



void lscCalTbl(TBL_INFO_T tbl_info)
{
	MINT32 kk, jj, ii,i;
	MINT32 m,n,nn;
	DIM_INFO_T dim_x,dim_y;
	BLK_INFO_T blk_info;
	MINT32 coef1, coef2, coef3;
    MUINT32 blk_mn,blk_info_0,blk_info_1;
    MUINT32 *p_pgn;
    MUINT32 *p_lsc_tbl;
    MINT32 nbayer;

    /* parse block info */
    blk_mn = tbl_info.reg_mn;
    blk_info_0=tbl_info.reg_info0;
    blk_info_1=tbl_info.reg_info1;
    p_pgn     =tbl_info.src_tbl_addr;
    p_lsc_tbl =tbl_info.dst_tbl_addr;

    dim_x.grid_num=((blk_mn&0xffff0000)>>16)+2;
    dim_x.di=(blk_info_0&0xfFFF0000)>>16;
    dim_x.dn=(blk_info_1&0xfFFF0000)>>16;
    dim_x.di2=dim_x.di*dim_x.di;
    dim_x.dn2=dim_x.dn*dim_x.dn;
    dim_x.dir=1.0/(float)dim_x.di;
    dim_x.dnr=1.0/(float)dim_x.dn;
    dim_x.di2r=1.0/(float)dim_x.di2;
    dim_x.ddir=1.0/(float)(dim_x.di+dim_x.di);
    dim_x.ddnr=1.0/(float)(dim_x.di+dim_x.dn);
    dim_y.grid_num=((blk_mn&0x0000ffff)>>0)+2;
    dim_y.di=(blk_info_0&0x0000fFFF);
    dim_y.dn=(blk_info_1&0x0000fFFF);
	dim_y.di2=dim_y.di*dim_y.di;
	dim_y.dn2=dim_y.dn*dim_y.dn;
	dim_y.dir=1.0/(float)dim_y.di;
	dim_y.dnr=1.0/(float)dim_y.dn;
	dim_y.di2r=1.0/(float)dim_y.di2;
	dim_y.ddir=1.0/(float)(dim_y.di+dim_y.di);
	dim_y.ddnr=1.0/(float)(dim_y.di+dim_y.dn);

	m=dim_x.grid_num;
	n=dim_y.grid_num;
//	MHAL_LOG("lscCalTbl(%d,%d)\n",m,n);
	if(m>GRID_MAX || n>GRID_MAX)
	{
//		MHAL_LOG("mXn is out of dimension, %d>GRID_MAX ||%d >GRID_MAX",m,n);
//		ASSERT(0);
		return;
	}
//	MHAL_LOG("lscCalTbl(#1)\n");
	/* get original 2nd derivatives */
	nn = m;
	if(m<n) nn=n;
	y2[0]=0.0;
	for (i=1;i<nn-1;i++) {
		y2[i]= (float) (-1.0/(y2[i-1]+4.0));
	}
	y2[nn-1]=0.0;

	/* parse real pixel gain in GN_BIT */
	for (jj=0;jj<n;jj++)
	{
		for (ii=0;ii<m;ii++)
		{
			zz[0][jj][ii]=(p_pgn[(jj*m+ii)*2+0]&0x0000FFFF);
			zz[1][jj][ii]=(p_pgn[(jj*m+ii)*2+0]&0xFFFF0000)>>16;
			zz[2][jj][ii]=(p_pgn[(jj*m+ii)*2+1]&0x0000FFFF);
			zz[3][jj][ii]=(p_pgn[(jj*m+ii)*2+1]&0xFFFF0000)>>16;
		}
	}

	for(nbayer=0;nbayer<4;nbayer++) // for each channel
	{
		/* get final 2nd derivatives in x-dir */
		for (jj=0;jj<n;jj++)
		{
			for (ii=0;ii<m-1;ii++) {
				yx2[ii]=y2[ii];
			}
			yx2[ii]=0.0;
			lscGet2ndDerivative(dim_x,zz[nbayer][jj],yx2);
			for (ii=0;ii<m;ii++)
			{
				zh2[jj][ii]=yx2[ii];
			}
	 	}
		/* get final 2nd derivatives in y-dir */
		for (ii=0;ii<m;ii++)
		{
			for (jj=0;jj<n;jj++)
			{
				zzb[jj]=zz[nbayer][jj][ii];
			}
			for (jj=0;jj<n-1;jj++) {
				yy2[jj]=y2[jj];
			}
			yy2[jj]=0.0;
			lscGet2ndDerivative(dim_y,zzb,yy2);
			for (jj=0;jj<n;jj++)
			{
				zv2[jj][ii]=yy2[jj];
			}
		}
		/* get 12 coef */
		for(jj = 0; jj < n-1; jj++)
		{
			for(ii = 0; ii < m-1; ii++)
			{
				blk_info.dx2=dim_x.di2; if(ii==m-2) blk_info.dx2=dim_x.dn2;
				blk_info.dy2=dim_y.di2;	if(jj==n-2) blk_info.dy2=dim_y.dn2;
				blk_info.f00 = zz[nbayer][jj][ii];
				blk_info.f10 = zz[nbayer][jj][ii+1];
				blk_info.f01 = zz[nbayer][jj+1][ii];
				blk_info.f11 = zz[nbayer][jj+1][ii+1];
				blk_info.f00x2 = zh2[jj][ii];
				blk_info.f10x2 = zh2[jj][ii+1];
				blk_info.f01x2 = zh2[jj+1][ii];
				blk_info.f11x2 = zh2[jj+1][ii+1];
				blk_info.f00y2 = zv2[jj][ii];
				blk_info.f10y2 = zv2[jj][ii+1];
				blk_info.f01y2 = zv2[jj+1][ii];
				blk_info.f11y2 = zv2[jj+1][ii+1];
				lscGetRectCoef(blk_info,coef[nbayer][jj*(m-1)+ii]);
			}
		}
	}

	/* padding to HW table */
	nn=0;
	for(jj = 0; jj < n-1; jj++)
	{
		for(ii = 0; ii < m-1; ii++)
		{
			for(nbayer=0;nbayer<4;nbayer++)
			{
				for( kk=0; kk<12; kk+=3)
				{
					/* No Existing of "512", 0->0, -1->513, 1->1 */
					coef1 = (MINT16)coef[nbayer][jj*(m-1)+ii][kk  ];
					if(coef1 < 0) coef1 = (512-coef1);
					coef2 = (MINT16)coef[nbayer][jj*(m-1)+ii][kk+1];
					if(coef2 < 0) coef2 = (512-coef2);
					coef3 = (MINT16)coef[nbayer][jj*(m-1)+ii][kk+2];
					if(coef3 < 0) coef3 = (512-coef3);
					p_lsc_tbl[nn++] = ((coef3)<<20)|((coef2)<<10)|(coef1);
				}
			}
		}
	}

//	MHAL_LOG("lscCalTbl(%d,%d)-DONE \n",m,n);
}

void shading_table_rotate(float *pShadingParam, float *pShadingCoef)
{
    const MUINT16 iFactor[12] = {512, 512, 512, 512, 512, 512, 512, 512, 512, 256, 256, 128};
    MUINT16 iCount;
    for (iCount = 0; iCount < 12; iCount++)
    {
        //kal_prompt_trace(MOD_ENG, "pShadingParam[%d]=%f",iCount, pShadingParam[iCount]);
        if (pShadingParam[iCount] > 512)
        {
            pShadingParam[iCount] = 512 - pShadingParam[iCount];
        }
        pShadingParam[iCount] = pShadingParam[iCount]/iFactor[iCount];
    }
    pShadingCoef[0] = pShadingParam[0];
    pShadingCoef[1] = pShadingParam[1];
    pShadingCoef[2] = 0-pShadingParam[0]-pShadingParam[2];
    pShadingCoef[3] = 0-pShadingParam[3]-3*pShadingParam[0];
    pShadingCoef[4] = 0-pShadingParam[4]-3*pShadingParam[1];
    pShadingCoef[5] = 0-pShadingParam[5]-pShadingParam[1];
    pShadingCoef[6] = 3*pShadingParam[2]+3*pShadingParam[0]+pShadingParam[6]+pShadingParam[3];
    pShadingCoef[7] = 3*pShadingParam[0]+3*pShadingParam[1]+2*pShadingParam[3]+2*pShadingParam[4]+pShadingParam[7];
    pShadingCoef[8] = 3*pShadingParam[1]+3*pShadingParam[5]+pShadingParam[4]+pShadingParam[8];
    pShadingCoef[9] = 0-3*pShadingParam[0]-3*pShadingParam[2]-2*pShadingParam[6]-2*pShadingParam[3]-pShadingParam[1]-pShadingParam[4]-pShadingParam[7]-pShadingParam[9];
    pShadingCoef[10] = 0-3*pShadingParam[1]-3*pShadingParam[5]-2*pShadingParam[4]-2*pShadingParam[8]-pShadingParam[10]-pShadingParam[7]-pShadingParam[0]-pShadingParam[3];
    pShadingCoef[11] = pShadingParam[0]+pShadingParam[1]+pShadingParam[2]+pShadingParam[3]+pShadingParam[4]+pShadingParam[5]+pShadingParam[6]+pShadingParam[7]
        +pShadingParam[8]+pShadingParam[9]+pShadingParam[10]+pShadingParam[11];
    for (iCount = 0; iCount < 12; iCount++)
    {
        pShadingCoef[iCount] = pShadingCoef[iCount]*iFactor[iCount];
        if (pShadingCoef[iCount] < 0)
        {
            pShadingCoef[iCount] = (512 - pShadingCoef[iCount]);
        }
        //kal_prompt_trace(MOD_ENG, "pShadingCoef[%d]=%f",iCount, pShadingCoef[iCount]);
    }
}

void rotate_fixed_shading_table(MUINT32 *pshading_table_addr, MUINT16 shading_table_size)
{
    MUINT32 i, j, iChannel, iParam;
    MUINT32 *pshading_channel=(MUINT32 *) &zz[0][0][0];
    float *pshading_param=(float *) &zh2[0][0];
    float *pshading_param_coef = pshading_param+(GRID_MAX*GRID_MAX>>1);

    if (shading_table_size >= 16)
    {
        i = 0;
        j = shading_table_size - 16;

        for (; i < j; i+=16, j-=16)
        {
            for (iChannel = 0; iChannel < 4; iChannel++)
            {
                for (iParam = 0; iParam < 4; iParam++)
                {
                    pshading_channel[iChannel*4+iParam] = pshading_table_addr[i+iChannel*4+iParam];
                }
            }
            for (iChannel = 0; iChannel < 4; iChannel++)
            {
                for (iParam = 0; iParam < 4; iParam++)
                {
                    pshading_param[iParam*3 + 0] = pshading_table_addr[j+iChannel*4+iParam] & 0x3FF;
                    pshading_param[iParam*3 + 1] = (pshading_table_addr[j+iChannel*4+iParam] >> 10) & 0x3FF;
                    pshading_param[iParam*3 + 2] = (pshading_table_addr[j+iChannel*4+iParam] >> 20) & 0x3FF;
                }
                shading_table_rotate(pshading_param, pshading_param_coef);
                for (iParam = 0; iParam < 4; iParam++)
                {
                    pshading_table_addr[i+iChannel*4+iParam] = (((MUINT16)pshading_param_coef[iParam*3 + 0]) & 0x3FF)
                        + ((((MUINT16)pshading_param_coef[iParam*3 + 1]) & 0x3FF) << 10) + ((((MUINT16)pshading_param_coef[iParam*3 + 2]) & 0x3FF) << 20);
                }
            }
            for (iChannel = 0; iChannel < 4; iChannel++)
            {
                for (iParam = 0; iParam < 4; iParam++)
                {
                    pshading_param[iParam*3 + 0] = pshading_channel[iChannel*4+iParam] & 0x3FF;
                    pshading_param[iParam*3 + 1] = (pshading_channel[iChannel*4+iParam] >> 10) & 0x3FF;
                    pshading_param[iParam*3 + 2] = (pshading_channel[iChannel*4+iParam] >> 20) & 0x3FF;
                }
                shading_table_rotate(pshading_param, pshading_param_coef);
                for (iParam = 0; iParam < 4; iParam++)
                {
                    pshading_table_addr[j+iChannel*4+iParam] = (((MUINT16)pshading_param_coef[iParam*3 + 0]) & 0x3FF)
                        + ((((MUINT16)pshading_param_coef[iParam*3 + 1]) & 0x3FF) << 10) + ((((MUINT16)pshading_param_coef[iParam*3 + 2]) & 0x3FF) << 20);
                }
            }

        }
        if (i == j)
        {
            for (iChannel = 0; iChannel < 4; iChannel++)
            {
                for (iParam = 0; iParam < 4; iParam++)
                {
                    pshading_param[iParam*3 + 0] = pshading_table_addr[i+iChannel*4+iParam] & 0x3FF;
                    pshading_param[iParam*3 + 1] = (pshading_table_addr[i+iChannel*4+iParam] >> 10) & 0x3FF;
                    pshading_param[iParam*3 + 2] = (pshading_table_addr[i+iChannel*4+iParam] >> 20) & 0x3FF;
                }
                shading_table_rotate(pshading_param, pshading_param_coef);
                for (iParam = 0; iParam < 4; iParam++)
                {
                    pshading_table_addr[i+iChannel*4+iParam] = (((MUINT16)pshading_param_coef[iParam*3 + 0]) & 0x3FF)
                        + ((((MUINT16)pshading_param_coef[iParam*3 + 1]) & 0x3FF) << 10) + ((((MUINT16)pshading_param_coef[iParam*3 + 2]) & 0x3FF) << 20);
                }
            }
        }
    }
}

void rotate_slim_shading_table(MUINT16 block_number_x, MUINT16 block_number_y, MUINT32 *data)
{
	MUINT16 i,j;
    MUINT16 block_node=(block_number_x+1)*(block_number_y+1);
    MUINT32 temp0, temp1;

    i=0;
    j=block_node*2-2;

    for (; i<j; i+=2, j-=2)
    {
        temp0=*(data+i);
        temp1=*(data+i+1);

        *(data+i) = *(data+j);
        *(data+i+1) = *(data+j+1);

        *(data+j) = temp0;
        *(data+j+1) = temp1;
    }

    if (i==j)
    {
        temp0=*(data+i);
        temp1=*(data+i+1);

        *(data+i) = temp0;
        *(data+i+1) = temp1;
    }
}




void resize_slim_shading_table(MUINT16 data_src_width, MUINT16 data_src_height,
				               MUINT16 data_tar_width, MUINT16 data_tar_height,
				               MUINT32 *src_data, MUINT32 *tar_data)
{
	MINT32 coeff_index;
	MINT32 i, j;
	MINT32 k = 0;
	MINT32 i_max, j_max, k_inc;
	MINT32 residual_max;

	MINT32 src_width, src_height;
	MINT32 tar_width, tar_height;
	MUINT32 src_width_for_ratio, src_height_for_ratio;
	MUINT32 tar_width_for_ratio, tar_height_for_ratio;

	MINT32 in0, in1, in2, in3;
	MINT32 in0_index, in1_index, in2_index, in3_index;
	MINT32 mac;

	MUINT32 ratio, residual;
	MINT32 temp_ratio, temp_residual;

	MINT32 c_c0, c_c1, c_c2, c_c3;

    MINT32 ref;
    MINT32 mantessa;

	MINT32 channel=0;
	MUINT16 shift_bits;

    MUINT16 *ptemp_data=(MUINT16 *) &zz[0][0][0];

	for (i=0; i< (data_tar_width*data_tar_height*2);i++)
		*(tar_data+i)=0;

	for (channel=0; channel<4; channel++)
	{  // for 4 channel, G, Gb, R, Gr
        if (channel & 0x01)
        	shift_bits=0;
        else
        	shift_bits=16;
        k=0;

        if (data_src_height != data_tar_height)
        {   /* vertical resize */
        	src_width = data_src_width;
        	tar_width = data_src_width;
        	src_height = data_src_height;
        	tar_height = data_tar_height;
        	src_width_for_ratio = src_width - 1;
        	src_height_for_ratio = src_height - 1;
        	tar_width_for_ratio = tar_width - 1;
        	tar_height_for_ratio = tar_height - 1;

        	ratio = (MUINT32) ((src_height_for_ratio <<20) + (tar_height_for_ratio>>1)) / tar_height_for_ratio;
        	residual = src_height_for_ratio % tar_height_for_ratio;

        	i_max = src_width;
        	j_max = tar_height;
        	k_inc = src_width;
        	residual_max = j_max -1;

            for(i=0; i<i_max; i++)
            {
        		k = i;
        	    temp_ratio = 0;
        	    temp_residual = 0;

        	    for(j=0; j<j_max; j++)
        	    {
        		    // Position and coeff_index calculation
        		    ref = (temp_ratio & 0xfff00000) >> 20;  // MSB 12 bits
        		    coeff_index = (temp_ratio & 0x000fc000) >> 14;  // 6 bits

        		    // determine effect of residual
        		    mantessa = (temp_ratio & 0xfffff);
        		    if (temp_residual== residual_max && mantessa !=0) // need rounding
        		    {
        			    ref = ref + ((temp_ratio & 0x00080000) >> 19);
        			    coeff_index = 0;

        			    // update parameters
        			    temp_ratio = (ref << 20) + ratio;
        			    temp_residual = residual;
        		    }
        		    else if (temp_residual==residual_max)  // no need rounding
        		    {
        		    	temp_ratio += ratio;
        		    	temp_residual = residual;
        		    }
        		    else
        		    {
        			    // update parameters
        			    temp_ratio += ratio;

        			    if (((temp_residual + residual) % residual_max)==0)
        			    	// next pixel may need rounding
        			    	temp_residual += residual;
        			    else
        			    	temp_residual  = (temp_residual+residual) % residual_max;
        		    }
        		    //end: Position and coeff_index calculation

        		    in0_index = ref-1;
        		    in1_index = ref;
        		    in2_index = ref+1;
        		    in3_index = ref+2;

        			in0_index = (in0_index < 0) ? 0 : (in0_index >= src_height) ? (src_height - 1) : in0_index;
        			in1_index = (in1_index < 0) ? 0 : (in1_index >= src_height) ? (src_height - 1) : in1_index;
        			in2_index = (in2_index < 0) ? 0 : (in2_index >= src_height) ? (src_height - 1) : in2_index;
        			in3_index = (in3_index < 0) ? 0 : (in3_index >= src_height) ? (src_height - 1) : in3_index;

        			in0 =(MUINT16) ((*(src_data+(in0_index*src_width+i)*2+(channel>>1))&(0xFFFF<<shift_bits))>>shift_bits);
        			in1 =(MUINT16) ((*(src_data+(in1_index*src_width+i)*2+(channel>>1))&(0xFFFF<<shift_bits))>>shift_bits);
        			in2 =(MUINT16) ((*(src_data+(in2_index*src_width+i)*2+(channel>>1))&(0xFFFF<<shift_bits))>>shift_bits);
        			in3 =(MUINT16) ((*(src_data+(in3_index*src_width+i)*2+(channel>>1))&(0xFFFF<<shift_bits))>>shift_bits);

        		    if(coeff_index ==0)
        		    {
              			c_c0 = (MINT32) bicubic_coeff3[64-1];
              			c_c1 = (MINT32) bicubic_coeff2[64-1];
              			c_c2 = (MINT32) bicubic_coeff1[64-1];
              			c_c3 = (MINT32) bicubic_coeff0[64-1];
        		    }
        		    else
        		    {
        			    c_c0 = (MINT32) bicubic_coeff0[coeff_index-1];
        			    c_c1 = (MINT32) bicubic_coeff1[coeff_index-1];
        			    c_c2 = (MINT32) bicubic_coeff2[coeff_index-1];
        			    c_c3 = (MINT32) bicubic_coeff3[coeff_index-1];
        		    }

            		mac = c_c0 * in0 +
        			      c_c1 * in1 +
        			      c_c2 * in2 +
        			      c_c3 * in3 + (1<<(8-1));

        		    *(ptemp_data+k) = CLAMP16((mac>>8));

        		    k = k + k_inc;
        	    } // j loop
            } // i loop
        }
        else
        {
        	for(j=0; j<data_tar_height; j++)
            {
        	    for(i=0; i<data_src_width; i++)
        		{
        			*(ptemp_data+k++)=(MUINT16) ((*(src_data+(j*data_src_width+i)*2+(channel>>1))&(0xFFFF<<shift_bits))>>shift_bits);
        		}
        	}
           }

        if (data_src_width != data_tar_width)
        {   /* horizontal resize */
        	src_width = data_src_width;
        	tar_width = data_tar_width;
        	src_height = data_tar_height;
        	tar_height = data_tar_height;
        	src_width_for_ratio = src_width - 1;
        	src_height_for_ratio = src_height - 1;
        	tar_width_for_ratio = tar_width - 1;
        	tar_height_for_ratio = tar_height - 1;

        	ratio = (MUINT32) ((src_width_for_ratio <<20) + (tar_width_for_ratio>>1)) / tar_width_for_ratio;
        	residual = src_width_for_ratio % tar_width_for_ratio;

        	i_max = src_height;
        	j_max = tar_width;
        	residual_max = j_max -1;

            k = 0;

            for(i=0; i<i_max; i++)
            {
        	    temp_ratio = 0;
        	    temp_residual = 0;

        	    for(j=0; j<j_max; j++)
        	    {
        		    // Position and coeff_index calculation
        		    ref = (temp_ratio & 0xfff00000) >> 20;  // MSB 12 bits
        		    coeff_index = (temp_ratio & 0x000fc000) >> 14;  // 6 bits

        		    // determine effect of residual
        		    mantessa = (temp_ratio & 0xfffff);
        		    if (temp_residual== residual_max && mantessa !=0) // need rounding
        		    {
        		    	//ref = ref + 1;
        		    	ref = ref + ((temp_ratio & 0x00080000) >> 19);
        		    	coeff_index = 0;

        		    	// update parameters
        		    	temp_ratio = (ref << 20) + ratio;
        		    	temp_residual = residual;
        		    }
        		    else if (temp_residual==residual_max)  // no need rounding
        		    {
        		    	temp_ratio += ratio;
        		    	temp_residual = residual;
        		    }
        		    else
        		    {
        		    	// update parameters
        		    	temp_ratio += ratio;

        		    	if (((temp_residual + residual) % residual_max)==0)
        		    		// next pixel may need rounding
        		    		temp_residual += residual;
        		    	else
        		    		temp_residual  = (temp_residual+residual) % residual_max;
        		    }
        		    //end: Position and coeff_index calculation

        		    in0_index = ref-1;
        		    in1_index = ref;
        		    in2_index = ref+1;
        		    in3_index = ref+2;

        		    in0_index = (in0_index < 0) ? 0 : (in0_index >= src_width) ? (src_width - 1) : in0_index;
        		    in1_index = (in1_index < 0) ? 0 : (in1_index >= src_width) ? (src_width - 1) : in1_index;
        		    in2_index = (in2_index < 0) ? 0 : (in2_index >= src_width) ? (src_width - 1) : in2_index;
        		    in3_index = (in3_index < 0) ? 0 : (in3_index >= src_width) ? (src_width - 1) : in3_index;

        		    in0 = *(ptemp_data+i*src_width+in0_index);
        		    in1 = *(ptemp_data+i*src_width+in1_index);
        		    in2 = *(ptemp_data+i*src_width+in2_index);
        		    in3 = *(ptemp_data+i*src_width+in3_index);

        		    if(coeff_index ==0)
        		    {
        		    	c_c0 = (MINT32) bicubic_coeff3[64-1];
        		    	c_c1 = (MINT32) bicubic_coeff2[64-1];
        		    	c_c2 = (MINT32) bicubic_coeff1[64-1];
        		    	c_c3 = (MINT32) bicubic_coeff0[64-1];
        		    }
        		    else
        		    {
        		    	c_c0 = (MINT32) bicubic_coeff0[coeff_index-1];
        		    	c_c1 = (MINT32) bicubic_coeff1[coeff_index-1];
        		    	c_c2 = (MINT32) bicubic_coeff2[coeff_index-1];
        		    	c_c3 = (MINT32) bicubic_coeff3[coeff_index-1];
        		    }

        		    mac = c_c0 * in0 +
        		    	  c_c1 * in1 +
        		    	  c_c2 * in2 +
        		    	  c_c3 * in3 + (1<<(8-1));

        		    *(tar_data+(k++)*2+(channel>>1)) |= CLAMP16((mac>>8))<<shift_bits;
        	    } // j loop
            } // i loop
        }
        else
        {
        	k = 0;
        	for(j=0; j<data_tar_height; j++)
            {
        	    for(i=0; i<data_tar_width; i++)
        		{
        			*(tar_data+(k++)*2+(channel>>1)) |= *(ptemp_data+j*data_tar_width+i)<<shift_bits;
        		}
        	}
        }
	}
}
///<<<
/***********************************************************************************************/
//SeanLin@20110630 To Cover Sensor color order changing >>
/***********************************************************************************************/
MUINT32 CamCommShadingTableColorOrder(MUINT32 *pu32Table, MUINT32 u32ColorOrder, MUINT32 u32MaxSize )
{
    MUINT32 ColorShift[4][4] = {
                                                {0, 1, 2, 3}, {1, 0, 3, 2},
                                                {2, 3, 1, 0}, {3, 2, 1, 0}};
    MUINT32 idx,i,k,line,Tabletmp[16];
    MHAL_LOG("ShadTbl ColorOrder()\n");
    MHAL_LOG("u32ColorOrder = %d, u32MaxSize = %d\n",u32ColorOrder,u32MaxSize);

    idx = u32ColorOrder;
    for(i=0;i<u32MaxSize/16;i++)
    {
        for(k=0;k<4;k++)
        {
            for(line=0;line<4;line++)
            {
                Tabletmp[k*4+line] = pu32Table[i*16+ColorShift[idx][k]*4+line];
            }
        }
        for(k=0;k<4;k++)
        {
            for(line=0;line<4;line++)
            {
                pu32Table[i*16+ColorShift[0][k]*4+line] = Tabletmp[k*4+line];
            }
        }
    }
#if 0
    MHAL_LOG("//aft  {\n");
    for(i=0;i<32/16;i++)
    {
        for(k=0;k<4;k++)
        {
            MHAL_LOG("            0x%08x,0x%08x,0x%08x,0x%08x,\n",
            pu32Table[i*16+k*4+0],pu32Table[i*16+k*4+1],pu32Table[i*16+k*4+2],pu32Table[i*16+k*4+3]);
        }
        MHAL_LOG("\n");
    }
    MHAL_LOG("        }\n");
#endif

  return 0;
}
//SeanLin@20110630 To Cover Sensor color order changing <<


MUINT32 CamCommShadingTableConvert(PEEPROM_SHADING_STRUCT pLscTbl)
{
    MUINT32 i;
    PEEPROM_SHADING_STRUCT lpLscTbl = pLscTbl;
    if(pLscTbl!=NULL)
    {
        for(i=0;i<EEPROM_LSC_ID_NO;i++)
        {
            if(pLscTbl->ShadingID == lCamCommConvertTable[i].ShadingID)
           	{
             	lCamCommConvertTable[i].LscConvert((PEEPROM_SHADING_STRUCT)lpLscTbl);
                MHAL_LOG("Find table convert IDX(%d),- ID(0x%x)\n",i,pLscTbl->ShadingID);
           	    break;
           	}
        }
    }
    return 0;
}


MUINT32 CamCommDoISPSlimShadingLoad(PEEPROM_SHADING_STRUCT pLscTbl)
{
    PEEPROM_SHADING_STRUCT pEerom_data = pLscTbl;
    TBL_INFO_T tbl_info;
    MUINT32 uiSlimShadingBuffer_PV[EEPROM_LSC_DATA_SIZE_SLIM_LSC1]={0};
    MINT32 i4XPreGrid, i4YPreGrid, i4XCapGrid, i4YCapGrid;
    MUINT16 i;
    i4XPreGrid = ((pEerom_data->PreRegister.shading_ctrl2&0x0001f000) >> 12) + 2;
    i4YPreGrid = ((pEerom_data->PreRegister.shading_ctrl3&0x0001f000) >> 12) + 2;
    i4XCapGrid = ((pEerom_data->CapRegister.shading_ctrl2&0x0001f000) >> 12) + 2;
    i4YCapGrid = ((pEerom_data->CapRegister.shading_ctrl3&0x0001f000) >> 12) + 2;


    if(pEerom_data->TableRotation)
    {
        MHAL_LOG("GAIN ROTATION\n");
        rotate_slim_shading_table( (i4XCapGrid-1),(i4YCapGrid-1),(MUINT32 *)&pEerom_data->CaptureTable[0][0]);
    }
    resize_slim_shading_table((MUINT16)i4XCapGrid, (MUINT16)i4YCapGrid,
            (MUINT16)i4XPreGrid, (MUINT16)i4YPreGrid,
            (MUINT32 *)&pEerom_data->CaptureTable[0][0], (MUINT32 *)uiSlimShadingBuffer_PV);


    tbl_info.reg_mn = (((i4XPreGrid-2)<<16)&0xffff0000) | (((i4YPreGrid-2))&0x0000ffff);
    tbl_info.reg_info0=((pEerom_data->PreRegister.shading_ctrl2&0x00000fff)&0xffff0000) |
            ((pEerom_data->PreRegister.shading_ctrl3&0x00000fff)&0x0000ffff);//0x60ea709a;//
    tbl_info.reg_info1=((pEerom_data->PreRegister.shading_ctrl3)&0x0fff0000)|
            (pEerom_data->PreRegister.shading_ctrl3&0x00000fff);
    tbl_info.src_tbl_addr=(MUINT32 *)uiSlimShadingBuffer_PV;
    tbl_info.dst_tbl_addr=(MUINT32 *)&pEerom_data->PreviewTable[2][0];
    lscCalTbl(tbl_info);

    MHAL_LOG("pEerom_data->ColorOrder = %d\n",pEerom_data->ColorOrder);
    if(pEerom_data->ColorOrder)
    {
        CamCommShadingTableColorOrder((MUINT32 *)&pEerom_data->PreviewTable[2][0], pEerom_data->ColorOrder, pEerom_data->PreviewSize) ;
    }
    memcpy((char*)&pEerom_data->PreviewTable[1][0], (char*)&pEerom_data->PreviewTable[2][0],pEerom_data->PreviewSize*4);
    memcpy((char*)&pEerom_data->PreviewTable[0][0], (char*)&pEerom_data->PreviewTable[2][0],pEerom_data->PreviewSize*4);


    tbl_info.reg_mn = (((i4XPreGrid-2)<<16)&0xffff0000) | (((i4YPreGrid-2))&0x0000ffff);
    tbl_info.reg_info0=((pEerom_data->CapRegister.shading_ctrl2&0x00000fff)&0xffff0000) |
            ((pEerom_data->CapRegister.shading_ctrl3&0x00000fff)&0x0000ffff);//0x60ea709a;//
    tbl_info.reg_info1=((pEerom_data->CapRegister.shading_ctrl3)&0x0fff0000)|
            (pEerom_data->CapRegister.shading_ctrl3&0x00000fff);
    tbl_info.src_tbl_addr=(MUINT32 *)&pEerom_data->CaptureTable[0][0];
    tbl_info.dst_tbl_addr=(MUINT32 *)&pEerom_data->CaptureTable[2][0];
    lscCalTbl(tbl_info);

    if(pEerom_data->ColorOrder)
    {
        CamCommShadingTableColorOrder((MUINT32 *)&pEerom_data->CaptureTable[2][0], pEerom_data->ColorOrder, pEerom_data->CaptureSize)   ;
    }

    memcpy((char*)&pEerom_data->CaptureTable[1][0], (char*)&pEerom_data->CaptureTable[2][0],pEerom_data->CaptureSize*4);
    memcpy((char*)&pEerom_data->CaptureTable[0][0], (char*)&pEerom_data->CaptureTable[2][0],pEerom_data->CaptureSize*4);
    return 0;
}

MUINT32 CamCommDoISPDynamicShadingLoad(PEEPROM_SHADING_STRUCT pLscTbl)
{
  return 0;
}
MUINT32 CamCommDoISPFixShadingLoad(PEEPROM_SHADING_STRUCT pLscTbl)
{
	return 0;
}
MUINT32 CamCommDoISPSensorShadingLoad(PEEPROM_SHADING_STRUCT pLscTbl)
{
	return 0;
}
MUINT32 CamCommDoISPSvdShadingLoad(PEEPROM_SHADING_STRUCT pLscTbl)
{

	return 0;
}




///for SLIM TABLE<<<
