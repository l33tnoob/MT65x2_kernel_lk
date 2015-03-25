/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * MediaTek Inc. (C) 2010. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#define LOG_TAG "MtkCam/MtkCamUtils"
//
#include "Local.h"
#include <camera/MtkCameraParameters.h>
//

/******************************************************************************
*
*******************************************************************************/
namespace android {
namespace MtkCamUtils {
namespace FmtUtils {
    
//#include <string.h>
//#include <stdlib.h>
//#include <math.h>
//#include <stdio.h>

#define RGB2YCC_YGAIN            0xff
#define RGB2YCC_YOFST            0x1
#define RGB2YCC_UGAIN            0x90
#define RGB2YCC_VGAIN            0xb7

#define UINT8 unsigned char


/******************************************************************************
*
*******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] "fmt, __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)
/******************************************************************************
*
*******************************************************************************/
static void YV12GPU_TO_YV12(UINT8  *a_pucSrcBuf, UINT8  *a_pucDstBuf, int a_BufSize, int a_ImgW, int a_ImgH);
static void YV12_TO_NV21(UINT8  *a_pucSrcBuf, UINT8  *a_pucDstBuf, int a_BufSize, int a_ImgW, int a_ImgH);
static void YUV420_3P_TO_RGB888( UINT8  *a_pucSrcBuf, UINT8  *a_pucDstBuf, int a_ImgW, int a_ImgH);
static void RGB888_TO_YUV420_3P( UINT8  *a_pucSrcBuf, UINT8  *a_pucDstBuf, int a_ImgW, int a_ImgH);
/******************************************************************************
*
*******************************************************************************/
void 
CamFormatTransform::
setSrc(String8 fmt, unsigned char *addr, int size, int width, int height, int stride1, int stride2, int stride3)
{
    src.fmt = fmt;
    src.addr = addr;
    src.size = size;
    src.width = width;
    src.height = height;
    src.stride1 = stride1;
    src.stride2 = stride2;
    src.stride3 = stride3;
}


/******************************************************************************
*
*******************************************************************************/
void 
CamFormatTransform::
setDst(String8 fmt, unsigned char *addr, int size, int width, int height, int stride1, int stride2, int stride3)
{
    dst.fmt = fmt;
    dst.addr = addr;
    dst.size = size;
    dst.width = width;
    dst.height = height;
    dst.stride1 = stride1;
    dst.stride2 = stride2;
    dst.stride3 = stride3;
}


/******************************************************************************
*
*******************************************************************************/
bool 
CamFormatTransform::
check()
{
    if ( src.fmt == ""      ||
         src.addr == NULL   ||
         src.size == 0      ||
         src.width == 0     ||
         src.height == 0    ||
         src.stride1 == 0   ||
         dst.fmt == ""      ||
         dst.addr == NULL   ||
         dst.size == 0      ||
         dst.width == 0     ||
         dst.height == 0    ||
         dst.stride1 == 0           
       )
    {
         return false;

    }

    return true;
}


/******************************************************************************
*
*******************************************************************************/
bool CamFormatTransform::convert()
{
   
if ( ! check() )
    {
        MY_LOGE("convert fail");
        return false;
    }
   
    if (src.fmt == MtkCameraParameters::PIXEL_FORMAT_YUV420P && 
        dst.fmt == MtkCameraParameters::PIXEL_FORMAT_YUV420SP)
    {
        YV12_TO_NV21(src.addr, dst.addr, src.size, src.width, src.height);
    }

    else if (src.fmt == MtkCameraParameters::PIXEL_FORMAT_YV12_GPU && 
             dst.fmt == MtkCameraParameters::PIXEL_FORMAT_YUV420P )
    {
        YV12GPU_TO_YV12(src.addr, dst.addr, src.size, src.width, src.height);
    }

    else if (src.fmt == MtkCameraParameters::PIXEL_FORMAT_YV12_GPU && 
             dst.fmt == MtkCameraParameters::PIXEL_FORMAT_YUV420SP)
    {
        // need to create a temp buffer.
        unsigned char* tmpBuffer = new unsigned char[src.size];
        YV12GPU_TO_YV12(src.addr, tmpBuffer, src.size, src.width, src.height);
        //
        YV12_TO_NV21(tmpBuffer, dst.addr, src.size, src.width, src.height); 

        if(tmpBuffer != NULL)
            delete [] tmpBuffer;
    }
    
    return true;
}


/******************************************************************************
*
*******************************************************************************/
void YV12GPU_TO_YV12(UINT8  *a_pucSrcBuf, UINT8  *a_pucDstBuf, int a_BufSize, int a_ImgW, int a_ImgH)
{
	int StrideW, half_ImgW, half_ImgH, half_StrideW;
	int i,j;
	UINT8 *pucSrcBuf_Y, *pucSrcBuf_V, *pucSrcBuf_U;
	UINT8 *pucDstBuf_Y, *pucDstBuf_V, *pucDstBuf_U;

	if( (a_BufSize % 3) !=0)
	{
        //MY_LOGE("Incorrect YV12GPU Buffer Size: %d", a_BufSize);
		return;
	}

	StrideW = (a_BufSize * 2) / (3*a_ImgH) ;

	if(StrideW < a_ImgW)
	{
        //MY_LOGE("Incorrect StrideW:%d, and a_ImgW:%d", StrideW, a_ImgW);
		return;
	}
	else if(StrideW == a_ImgW)
	{
		memcpy(a_pucDstBuf, a_pucSrcBuf, a_BufSize);
	}
	else
	{
		half_ImgW = a_ImgW>>1;
		half_ImgH = a_ImgH>>1;
		half_StrideW = StrideW >>1;
	
		pucSrcBuf_Y = a_pucSrcBuf;
		pucSrcBuf_V = pucSrcBuf_Y + StrideW*a_ImgH;
		pucSrcBuf_U = pucSrcBuf_V + half_StrideW*half_ImgH;
		pucDstBuf_Y = a_pucDstBuf;
		pucDstBuf_V = pucDstBuf_Y + a_ImgW*a_ImgH;
		pucDstBuf_U = pucDstBuf_V + half_ImgW*half_ImgH;

		//Y
		for(i=0;i<a_ImgH;i++)
		{
			for(j=0;j<a_ImgW;j++)
			{
				*(pucDstBuf_Y + i*a_ImgW + j) = *(pucSrcBuf_Y + i*StrideW + j);
			}
		}
		//V
		for(i=0;i<half_ImgH;i++)
		{
			for(j=0;j<half_ImgW;j++)
			{
				*(pucDstBuf_V + i*half_ImgW + j) = *(pucSrcBuf_V + i*half_StrideW + j);
			}
		}
		//U
		for(i=0;i<half_ImgH;i++)
		{
			for(j=0;j<half_ImgW;j++)
			{
				*(pucDstBuf_U + i*half_ImgW + j) = *(pucSrcBuf_U + i*half_StrideW + j);
			}
		}
	}
}


/******************************************************************************
*
*******************************************************************************/
void YV12_TO_NV21(UINT8  *a_pucSrcBuf, UINT8  *a_pucDstBuf, int a_BufSize, int a_ImgW, int a_ImgH)
{
	int i,j;
	int half_ImgW, half_ImgH;
	UINT8 *pucSrcBuf_Y, *pucSrcBuf_V, *pucSrcBuf_U;
	UINT8 *pucDstBuf_Y, *pucDstBuf_VU;

	half_ImgW = a_ImgW>>1;
	half_ImgH = a_ImgH>>1;

	pucSrcBuf_Y = a_pucSrcBuf;
	pucSrcBuf_V = pucSrcBuf_Y + a_ImgW*a_ImgH;
	pucSrcBuf_U = pucSrcBuf_V + half_ImgW*half_ImgH;
	pucDstBuf_Y = a_pucDstBuf;
	pucDstBuf_VU = pucDstBuf_Y + a_ImgW*a_ImgH;

	//Y
	memcpy(pucDstBuf_Y, pucSrcBuf_Y, a_ImgW*a_ImgH);

	//UV
	for(i=0;i<half_ImgH;i++)
	{
		for(j=0;j<half_ImgW;j++)
		{
			*(pucDstBuf_VU + i*a_ImgW + 2*j) = *(pucSrcBuf_V + i*half_ImgW + j);
			*(pucDstBuf_VU + i*a_ImgW + 2*j+1) = *(pucSrcBuf_U + i*half_ImgW + j);
		}
	}
}


/******************************************************************************
*
*******************************************************************************/
void YUV420_3P_TO_RGB888( UINT8  *a_pucSrcBuf, UINT8  *a_pucDstBuf, int a_ImgW, int a_ImgH)
{
	int i, j, k;
    UINT8* a_pucYBuf;
	UINT8* a_pucUBuf;
	UINT8* a_pucVBuf;
	int Y[4], U, V, R[4], G[4], B[4], r, g, b;
	
	a_pucYBuf = a_pucSrcBuf;
	a_pucUBuf = a_pucSrcBuf + (a_ImgW * a_ImgH);
	a_pucVBuf = a_pucUBuf + (a_ImgW * a_ImgH >> 2);
	
	//********************** //
	//*****   Y1  Y2    *****//
	//*****    U    V     *****//
	//*****   Y3   Y4   *****//
	//********************** //
	
	for(i=0;i<a_ImgH;i=i+2)
	{
		for(j=0;j<a_ImgW;j=j+2)
		{
			Y[0] = *(a_pucYBuf + ((i+0) * a_ImgW) + j);
			Y[1] = *(a_pucYBuf + ((i+0) * a_ImgW) + j+1) ;
			Y[2] = *(a_pucYBuf + ((i+1) * a_ImgW) + j) ;
			Y[3] = *(a_pucYBuf + ((i+1) * a_ImgW) + j+1) ;
			
			U  =  *(a_pucUBuf + (i >> 1) * (a_ImgW >> 1) + (j >>1));
			V  =  *(a_pucVBuf + (i >> 1) * (a_ImgW >> 1) + (j >>1));
			
			for(k=0;k<4;k++)
			{
				r = (32 * Y[k] + 45 * (V-128) + 16) / 32;
				g = (32 * Y[k] - 11 * (U-128) - 23 * (V-128) + 16) / 32;
				b = (32 * Y[k] + 57 * (U-128) + 16) / 32;
				
				R[k] = (r<0) ? 0: (r>255) ? 255 : r;
				G[k]= (g<0) ? 0: (g>255) ? 255 : g;
				B[k] = (b<0) ? 0: (b>255) ? 255 : b;
			}
			
		     *(a_pucDstBuf + ((i+0)  * a_ImgW + (j +0))  * 3 + 0) = R[0];
			*(a_pucDstBuf + ((i+0)  * a_ImgW + (j +0))  * 3 + 1) = G[0];
			*(a_pucDstBuf + ((i+0)  * a_ImgW + (j +0))  * 3 + 2) = B[0];
			
			*(a_pucDstBuf + ((i+0)  * a_ImgW + (j +1))  * 3 + 0) = R[1];
			*(a_pucDstBuf + ((i+0)  * a_ImgW + (j +1))  * 3 + 1) = G[1];
			*(a_pucDstBuf + ((i+0)  * a_ImgW + (j +1))  * 3 + 2) = B[1];
			
			*(a_pucDstBuf + ((i+1)  * a_ImgW + (j +0))  * 3 + 0) = R[2];
			*(a_pucDstBuf + ((i+1)  * a_ImgW + (j +0))  * 3 + 1) = G[2];
			*(a_pucDstBuf + ((i+1)  * a_ImgW + (j +0))  * 3 + 2) = B[2];
			
			*(a_pucDstBuf + ((i+1)  * a_ImgW + (j +1))  * 3 + 0) = R[3];
			*(a_pucDstBuf + ((i+1)  * a_ImgW + (j +1))  * 3 + 1) = G[3];
			*(a_pucDstBuf + ((i+1)  * a_ImgW + (j +1))  * 3 + 2) = B[3];
		}
	}	
}


/******************************************************************************
*
*******************************************************************************/
void RGB888_TO_YUV420_3P( UINT8  *a_pucSrcBuf, UINT8  *a_pucDstBuf, int a_ImgW, int a_ImgH)
{
	int i, j, k;
	int R[4], G[4], B[4], Y[4], U[4], V[4];
	int y_tmp1, y_tmp2, y_tmp3,  u_tmp1, u_tmp2, u_tmp3, u_tmp4, v_tmp1, v_tmp2, v_tmp3, v_tmp4;
	int index1, index2;
	
	index1 = a_ImgW * a_ImgH;
	index2 = index1 + a_ImgW * a_ImgH / 4;
	
	int t = 0;
     
	for(i=0;i<a_ImgH;i=i+2)
	{
		for(j=0;j<a_ImgW;j=j+2)
		{			
			R[0] = *(a_pucSrcBuf + ((i+0) * a_ImgW + (j +0)) * 3 );
			R[1] = *(a_pucSrcBuf + ((i+0) * a_ImgW + (j +1)) * 3 );
			R[2] = *(a_pucSrcBuf + ((i+1) * a_ImgW + (j +0)) * 3 );
			R[3] = *(a_pucSrcBuf + ((i+1) * a_ImgW + (j +1)) * 3 );
			
			G[0] = *(a_pucSrcBuf + ((i+0) * a_ImgW + (j +0)) * 3 + 1);
			G[1] = *(a_pucSrcBuf + ((i+0) * a_ImgW + (j +1)) * 3 + 1);
			G[2] = *(a_pucSrcBuf + ((i+1) * a_ImgW + (j +0)) * 3 + 1);
			G[3] = *(a_pucSrcBuf + ((i+1) * a_ImgW + (j +1)) * 3 + 1);
			
			B[0] = *(a_pucSrcBuf + ((i+0) * a_ImgW + (j +0)) * 3 + 2);
			B[1] = *(a_pucSrcBuf + ((i+0) * a_ImgW + (j +1)) * 3 + 2);
			B[2] = *(a_pucSrcBuf + ((i+1) * a_ImgW + (j +0)) * 3 + 2);
			B[3] = *(a_pucSrcBuf + ((i+1) * a_ImgW + (j +1)) * 3 + 2);
			
			//Y1~Y4
			for(k=0;k<4;k++)
			{
				y_tmp1 = R[k] * 77 + G[k] * 150 + B[k] * 20;
				y_tmp2 = (y_tmp1 >> 8);
				y_tmp3 = ((y_tmp2*RGB2YCC_YGAIN) >> 8) + RGB2YCC_YOFST;
				Y[k] = (y_tmp3 > 255) ? 255: (unsigned int) y_tmp3;
					
				u_tmp1 = B[k] - y_tmp2;
				u_tmp2 = (abs(u_tmp1) * RGB2YCC_UGAIN);
				u_tmp3 = 0;
				u_tmp4 = (u_tmp1 < 0) ? 128 - ((u_tmp2 >> 8) + u_tmp3) : 128 + (u_tmp2 >>8 ) + u_tmp3;
				U[k] =  (u_tmp4 > 255) ? 255 : (u_tmp4 < 0)? 0 : (unsigned int) (u_tmp4);
					
				v_tmp1 = R[k] - y_tmp2;
				v_tmp2 =  (abs(v_tmp1) * RGB2YCC_VGAIN);
				v_tmp3 = 0;
				v_tmp4 = (v_tmp1 < 0)? 128 - ((v_tmp2 >> 8) + v_tmp3) : 128 + (v_tmp2 >>8) + v_tmp3;
                    V[k] = (v_tmp4 > 255)? 255 : (v_tmp4 < 0)? 0 : (unsigned int) (v_tmp4);
			}
			
			//write YCC dara to output buffer
			*(a_pucDstBuf + (i+0) * a_ImgW + j +0) = Y[0];
			*(a_pucDstBuf + (i+0) * a_ImgW + j +1) = Y[1];
			*(a_pucDstBuf + (i+1) * a_ImgW + j +0) = Y[2];
			*(a_pucDstBuf + (i+1) * a_ImgW + j +1) = Y[3];
			*(a_pucDstBuf + index1 + t) =                  (U[0] + U[1] + U[2] + U[3]) / 4;
			*(a_pucDstBuf + index1 + index2 + t) = (V[0] + V[1] + V[2] + V[3]) / 4;
			t++;
		}
	}
	
}

};  // namespace FmtUtils
};  // namespace MtkCamUtils
};  // namespace android

