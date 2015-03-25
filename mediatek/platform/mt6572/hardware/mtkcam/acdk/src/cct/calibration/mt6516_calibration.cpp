/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

///////////////////////////////////////////////////////////////////////////////
// No Warranty
// Except as may be otherwise agreed to in writing, no warranties of any
// kind, whether express or implied, are given by MTK with respect to any MTK
// Deliverables or any use thereof, and MTK Deliverables are provided on an
// "AS IS" basis.  MTK hereby expressly disclaims all such warranties,
// including any implied warranties of merchantability, non-infringement and
// fitness for a particular purpose and any warranties arising out of course
// of performance, course of dealing or usage of trade.  Parties further
// acknowledge that Company may, either presently and/or in the future,
// instruct MTK to assist it in the development and the implementation, in
// accordance with Company's designs, of certain softwares relating to
// Company's product(s) (the "Services").  Except as may be otherwise agreed
// to in writing, no warranties of any kind, whether express or implied, are
// given by MTK with respect to the Services provided, and the Services are
// provided on an "AS IS" basis.  Company further acknowledges that the
// Services may contain errors, that testing is important and Company is
// solely responsible for fully testing the Services and/or derivatives
// thereof before they are used, sublicensed or distributed.  Should there be
// any third party action brought against MTK, arising out of or relating to
// the Services, Company agree to fully indemnify and hold MTK harmless.
// If the parties mutually agree to enter into or continue a business
// relationship or other arrangement, the terms and conditions set forth
// hereunder shall remain effective and, unless explicitly stated otherwise,
// shall prevail in the event of a conflict in the terms in any agreements
// entered into between the parties.
////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2008, MediaTek Inc.
// All rights reserved.
//
// Unauthorized use, practice, perform, copy, distribution, reproduction,
// or disclosure of this information in whole or in part is prohibited.
////////////////////////////////////////////////////////////////////////////////
// MT6516Calibration.cpp  $Revision$
////////////////////////////////////////////////////////////////////////////////

//! \file  MT6516Calibration.cpp
//! \brief
#define LOG_TAG "MT6516_CALIBRATION"

#include <stdlib.h>
#include <math.h>

#include "mtkcam/common.h"
#include "AcdkLog.h"
#include "AcdkErrCode.h"
#include "ParamCALInternal.h"

#define MEDIA_PATH "//data"

void *vector(float *v, long nl, long nh);
float pythag(float a, float b);
void free_vector(float *v, long nl, long nh);
void vLSC_PARAM_INIT(LSC_CAL_INI_PARAM_T a_rLSCCaliINIParam);
MUINT32 u4LSC_Integer_SQRT(unsigned long long x);

#define PREVIEW_TEST            (MUINT32)0x1
#define CAPTURE_TEST            (MUINT32)0x2
#define VIDEO_TEST              (MUINT32)0x3

/* Lens shading calibratino global variables declaration */
unsigned short *g_raw;
LSC_CALI_INFO_T g_lsc_cali_info;
unsigned int *g_src_tbl;
unsigned int *g_dst_tbl;
LSC_PARAM_T g_lsc_param;
LSC_RESULT_T g_lsc_result;
int g_col_idx[GRID_MAX];
int g_row_idx[GRID_MAX];

/* Bad Pixel calibratino global variables declaration */
BPC_CAL_INI_PARAM_T g_bpc_cali_param;

/////////////////////////////////////////////////////////////////////////
//
//   LSC SVD () -
/////////////////////////////////////////////////////////////////////////
float truncate (float w, float uv , int min , int fract_len  )
{
    float mul;
    mul = ((-sqrt(w)*uv)-min)*pow(2,fract_len);
    mul = (int) mul;
    mul = (mul/pow(2,fract_len)) + min;
    return mul;
}

int truncate_UV (float w, float uv , int min , int fract_len  )
{
    float mul;
    mul = ((-sqrt(w)*uv)-min)*pow(2,fract_len);
    return (int)mul;
}


void svdcali(float **a, int arg_i4m, int arg_i4n, float w[], float **v)
// Input matrix a[1..m][1..n], calculate its singular value
// decomposition, m = [U][w][v]-T. [U] replaces [a] when output. The diagonal
// matrix of singular values [w] is output as a vector w[1..n]. The matrix [v]
// (not [v]-T) is output as v[1..n][1..n]
{
    int i4flag =0;
    int i4i = 0;
    int i4iteration = 0;
    int i4j = 0;
    int i4jj = 0;
    int i4k = 0;
    int i4nm = 0;
    int i4l = 0;
    float fnorm = 0.0f;
    float fc = 0.0f;
    float ff = 0.0f;
    float fg = 0.0f;
    float fh = 0.0f;
    float fs = 0.0f;
    float fscale = 0.0f;
    float fx = 0.0f;
    float fy = 0.0f;
    float fz = 0.0f;
    float *pfrv1 = NULL;

    pfrv1=(float *)vector(pfrv1, 1,arg_i4n);
    fg=fscale=fnorm=0.0;
    for (i4i=1;i4i<=arg_i4n;i4i++) {
        i4l=i4i+1;
        pfrv1[i4i]=fscale*fg;
        fg=fs=fscale=0.0;
        if (i4i <= arg_i4m) {
            for (i4k=i4i;i4k<=arg_i4m;i4k++) fscale += (float) fabs(a[i4k][i4i]);
            if (fscale) {
                for (i4k=i4i;i4k<=arg_i4m;i4k++) {
                    a[i4k][i4i] /= fscale;
                    fs += a[i4k][i4i]*a[i4k][i4i];
                }
                ff=a[i4i][i4i];
                fg = (float) (-SIGN(sqrt(fs),ff));
                fh=ff*fg-fs;
                a[i4i][i4i]=ff-fg;
                for (i4j=i4l;i4j<=arg_i4n;i4j++) {
                    for (fs=0.0,i4k=i4i;i4k<=arg_i4m;i4k++) fs += a[i4k][i4i]*a[i4k][i4j];
                    ff=fs/fh;
                    for (i4k=i4i;i4k<=arg_i4m;i4k++) a[i4k][i4j] += ff*a[i4k][i4i];
                }
                for (i4k=i4i;i4k<=arg_i4m;i4k++) a[i4k][i4i] *= fscale;
            }
        }
        w[i4i]=fscale *fg;
        fg=fs=fscale=0.0;
        if (i4i <= arg_i4m && i4i != arg_i4n) {
            for (i4k=i4l;i4k<=arg_i4n;i4k++) fscale += (float) fabs(a[i4i][i4k]);
            if (fscale) {
                for (i4k=i4l;i4k<=arg_i4n;i4k++) {
                    a[i4i][i4k] /= fscale;
                    fs += a[i4i][i4k]*a[i4i][i4k];
                }
                ff=a[i4i][i4l];
                fg = (float) (-SIGN(sqrt(fs),ff));
                fh=ff*fg-fs;
                a[i4i][i4l]=ff-fg;
                for (i4k=i4l;i4k<=arg_i4n;i4k++) pfrv1[i4k]=a[i4i][i4k]/fh;
                for (i4j=i4l;i4j<=arg_i4m;i4j++) {
                    for (fs=0.0,i4k=i4l;i4k<=arg_i4n;i4k++) fs += a[i4j][i4k]*a[i4i][i4k];
                    for (i4k=i4l;i4k<=arg_i4n;i4k++) a[i4j][i4k] += fs*pfrv1[i4k];
                }
                for (i4k=i4l;i4k<=arg_i4n;i4k++) a[i4i][i4k] *= fscale;
            }
        }
        fnorm=(float) FMAX(fnorm,(float) (fabs(w[i4i])+fabs(pfrv1[i4i])));
    }
    for (i4i=arg_i4n;i4i>=1;i4i--) {
        if (i4i < arg_i4n) {
            if (fg) {
                for (i4j=i4l;i4j<=arg_i4n;i4j++)
                    v[i4j][i4i]=(a[i4i][i4j]/a[i4i][i4l])/fg;
                for (i4j=i4l;i4j<=arg_i4n;i4j++) {
                    for (fs=0.0,i4k=i4l;i4k<=arg_i4n;i4k++) fs += a[i4i][i4k]*v[i4k][i4j];
                    for (i4k=i4l;i4k<=arg_i4n;i4k++) v[i4k][i4j] += fs*v[i4k][i4i];
                }
            }
            for (i4j=i4l;i4j<=arg_i4n;i4j++) v[i4i][i4j]=v[i4j][i4i]=0.0;
        }
        v[i4i][i4i]=1.0;
        fg=pfrv1[i4i];
        i4l=i4i;
    }
    for (i4i=IMIN(arg_i4m,arg_i4n);i4i>=1;i4i--) {
        i4l=i4i+1;
        fg=w[i4i];
        for (i4j=i4l;i4j<=arg_i4n;i4j++) a[i4i][i4j]=0.0;
        if (fg) {
            fg=(float) 1.0/fg;
            for (i4j=i4l;i4j<=arg_i4n;i4j++) {
                for (fs=0.0,i4k=i4l;i4k<=arg_i4m;i4k++) fs += a[i4k][i4i]*a[i4k][i4j];
                ff=(fs/a[i4i][i4i])*fg;
                for (i4k=i4i;i4k<=arg_i4m;i4k++) a[i4k][i4j] += ff*a[i4k][i4i];
            }
            for (i4j=i4i;i4j<=arg_i4m;i4j++) a[i4j][i4i] *= fg;
        } else for (i4j=i4i;i4j<=arg_i4m;i4j++) a[i4j][i4i]=0.0;
        ++a[i4i][i4i];
    }
    for (i4k=arg_i4n;i4k>=1;i4k--) {
        for (i4iteration=1;i4iteration<=30;i4iteration++) {
            i4flag=1;
            for (i4l=i4k;i4l>=1;i4l--) {
                i4nm=i4l-1;
                if ((float)(fabs(pfrv1[i4l])+fnorm) == fnorm) {
                    i4flag=0;
                    break;
                }
                if ((float)(fabs(w[i4nm])+fnorm) == fnorm) break;
            }
            if (i4flag) {
                fc=0.0;
                fs=1.0;
                for (i4i=i4l;i4i<=i4k;i4i++) {
                    ff=fs*pfrv1[i4i];
                    pfrv1[i4i]=fc*pfrv1[i4i];
                    if ((float)(fabs(ff)+fnorm) == fnorm) break;
                    fg=w[i4i];
                    fh=pythag(ff,fg);
                    w[i4i]=fh;
                    fh=(float)1.0/fh;
                    fc=fg*fh;
                    fs = -ff*fh;
                    for (i4j=1;i4j<=arg_i4m;i4j++) {
                        fy=a[i4j][i4nm];
                        fz=a[i4j][i4i];
                        a[i4j][i4nm]=fy*fc+fz*fs;
                        a[i4j][i4i]=fz*fc-fy*fs;
                    }
                }
            }
            fz=w[i4k];
            if (i4l == i4k) {
                if (fz < 0.0) {
                    w[i4k] = -fz;
                    for (i4j=1;i4j<=arg_i4n;i4j++) v[i4j][i4k] = -v[i4j][i4k];
                }
                break;
            }
            if (i4iteration == 30)
            {
                printf("can't convergence during 30 svd calculation iterations");
            }
            fx=w[i4l];
            i4nm=i4k-1;
            fy=w[i4nm];
            fg=pfrv1[i4nm];
            fh=pfrv1[i4k];
            ff=(float) (((fy-fz)*(fy+fz)+(fg-fh)*(fg+fh))/(2.0*fh*fy));
            fg=pythag(ff,1.0);
            ff=(float) (((fx-fz)*(fx+fz)+fh*((fy/(ff+SIGN(fg,ff)))-fh))/fx);
            fc=fs=1.0;
            for (i4j=i4l;i4j<=i4nm;i4j++) {
                i4i=i4j+1;
                fg=pfrv1[i4i];
                fy=w[i4i];
                fh=fs*fg;
                fg=fc*fg;
                fz=pythag(ff,fh);
                pfrv1[i4j]=fz;
                fc=ff/fz;
                fs=fh/fz;
                ff=fx*fc+fg*fs;
                fg = fg*fc-fx*fs;
                fh=fy*fs;
                fy *= fc;
                for (i4jj=1;i4jj<=arg_i4n;i4jj++) {
                    fx=v[i4jj][i4j];
                    fz=v[i4jj][i4i];
                    v[i4jj][i4j]=fx*fc+fz*fs;
                    v[i4jj][i4i]=fz*fc-fx*fs;
                }
                fz=pythag(ff,fh);
                w[i4j]=fz;
                if (fz) {
                    fz=(float) (1.0/fz);
                    fc=ff*fz;
                    fs=fh*fz;
                }
                ff=fc*fg+fs*fy;
                fx=fc*fy-fs*fg;
                for (i4jj=1;i4jj<=arg_i4m;i4jj++) {
                    fy=a[i4jj][i4j];
                    fz=a[i4jj][i4i];
                    a[i4jj][i4j]=fy*fc+fz*fs;
                    a[i4jj][i4i]=fz*fc-fy*fs;
                }
            }
            pfrv1[i4l]=0.0;
            pfrv1[i4k]=ff;
            w[i4k]=fx;
        }
    }
    free_vector(pfrv1,1,arg_i4n);
}


float pythag(float a, float b)
{
    float absa,absb;
    absa=(float) fabs(a);
    absb=(float) fabs(b);
    if (absa > absb) return (float) (absa*sqrt(1.0+SQR(absb/absa)));
    else return (absb == 0.0 ? (float) 0.0 : (float) (absb*sqrt(1.0+SQR(absa/absb))));
}


void *vector(float *v, long nl, long nh)
/* allocate a float vector with subscript range v[nl..nh] */
{
    v=(float *)malloc((size_t) ((nh-nl+1+NR_END)*sizeof(float)));
    if (!v) ACDK_LOGE(" AcdkCalibration::allocation failure in *vector(float*v, ...)\n");
    return (void*)(v-nl+NR_END);
}


void *vector(int *v, long nl, long nh)
/* allocate a float vector with subscript range v[nl..nh] */
{
    v=(int *)malloc((size_t) ((nh-nl+1+NR_END)*sizeof(int)));
    if (!v) ACDK_LOGE(" AcdkCalibration::allocation failure in *vector(int*v, ...)\n");
    return (void *)(v-nl+NR_END);
}


void **matrix(unsigned char **m, long nrl, long nrh, long ncl, long nch)
/* allocate a float matrix with subscript range m[nrl..nrh][ncl..nch] */
{
    long i, nrow=nrh-nrl+1,ncol=nch-ncl+1;

    /* allocate pointers to rows */
    m=(unsigned char **) malloc(((nrow+NR_END)*sizeof(unsigned char*)));
    if (!m) printf("allocation failure 1 in matrix(u1 **, ...)\n");
    m += NR_END;
    m -= nrl;

    /* allocate rows and set pointers to them */
    m[nrl]=(unsigned char *) malloc(((nrow*ncol+NR_END)*sizeof(unsigned char)));
    if (!m[nrl]) printf("allocation failure 2 in matrix(u1 **, ...)\n");
    m[nrl] += NR_END;
    m[nrl] -= ncl;

    for(i=nrl+1;i<=nrh;i++) m[i]=m[i-1]+ncol;

    /* return pointer to array of pointers to rows */
    return (void**)m;
}


void **matrix(int **m, long nrl, long nrh, long ncl, long nch)
/* allocate a float matrix with subscript range m[nrl..nrh][ncl..nch] */
{
    long i, nrow=nrh-nrl+1,ncol=nch-ncl+1;

    /* allocate pointers to rows */
    m=(int **) malloc(((nrow+NR_END)*sizeof(int*)));
    if (!m) printf("allocation failure 1 in matrix(int **, ...)\n");
    m += NR_END;
    m -= nrl;

    /* allocate rows and set pointers to them */
    m[nrl]=(int *) malloc(((nrow*ncol+NR_END)*sizeof(int)));
    if (!m[nrl]) printf("allocation failure 2 in matrix(int **, ...)\n");
    m[nrl] += NR_END;
    m[nrl] -= ncl;

    for(i=nrl+1;i<=nrh;i++) m[i]=m[i-1]+ncol;

    /* return pointer to array of pointers to rows */
    return (void**)m;
}


void **matrix(long **m, long nrl, long nrh, long ncl, long nch)
/* allocate a float matrix with subscript range m[nrl..nrh][ncl..nch] */
{
    long i, nrow=nrh-nrl+1,ncol=nch-ncl+1;

    /* allocate pointers to rows */
    m=(long **) malloc(((nrow+NR_END)*sizeof(long*)));
    if (!m) printf("allocation failure 1 in matrix(int **, ...)\n");
    m += NR_END;
    m -= nrl;

    /* allocate rows and set pointers to them */
    m[nrl]=(long *) malloc(((nrow*ncol+NR_END)*sizeof(long)));
    if (!m[nrl]) printf("allocation failure 2 in matrix(int **, ...)\n");
    m[nrl] += NR_END;
    m[nrl] -= ncl;

    for(i=nrl+1;i<=nrh;i++) m[i]=m[i-1]+ncol;

    /* return pointer to array of pointers to rows */
    return (void**)m;
}


void **matrix(float **m, long nrl, long nrh, long ncl, long nch)
/* allocate a float matrix with subscript range m[nrl..nrh][ncl..nch] */
{
    long i, nrow=nrh-nrl+1,ncol=nch-ncl+1;

    /* allocate pointers to rows */
    m=(float **) malloc(((nrow+NR_END)*sizeof(float*)));
    if (!m) printf("allocation failure 1 in matrix(float **, ...)\n");
    m += NR_END;
    m -= nrl;

    /* allocate rows and set pointers to them */
    m[nrl]=(float *) malloc(((nrow*ncol+NR_END)*sizeof(float)));
    if (!m[nrl]) printf("allocation failure 2 in matrix(float **, ...)\n");
    m[nrl] += NR_END;
    m[nrl] -= ncl;

    for(i=nrl+1;i<=nrh;i++) m[i]=m[i-1]+ncol;

    /* return pointer to array of pointers to rows */
    return (void**)m;
}


void eigen_sequence(float *w, int *e_seq, int length)
{
    int i = 1;
    int j = 1;
    float test_val = 0.0;
    int sequence_cnt = 0;
    for (i = 1; i <= length ; i ++)
    {
        test_val = w[i];
        j = 1;
        sequence_cnt = 0;
        while (j <= length)
        {
            if (test_val<=w[j])
            {
                sequence_cnt++;
            }
            j++;
        }
        e_seq[i] = sequence_cnt;
    }
    //check same eigen value
    for (i = 1; i <= length ; i ++)
    {
        j = 1;
        sequence_cnt = 0;
        while(j <= length)
        {
            if (e_seq[i] == e_seq[j])
            {
                sequence_cnt++;
            }
            j++;
        }
        e_seq[i] = e_seq[i] + 1 - sequence_cnt; //deduct same eigen value
    }
}


void free_vector(float *v, long nl, long nh)
/* free a float vector allocated with vector() */
{
    free((float *) (v+nl-NR_END));
}

void free_vector(int *v, long nl, long nh)
/* free a float vector allocated with vector() */
{
    free((int *) (v+nl-NR_END));
}


void free_matrix(unsigned char **m, long nrl, long nrh, long ncl, long nch)
/* free a float matrix allocated by matrix() */
{
    free((unsigned char *) (m[nrl]+ncl-NR_END));
    free((unsigned char *) (m+nrl-NR_END));
}

void free_matrix(int **m, long nrl, long nrh, long ncl, long nch)
/* free a float matrix allocated by matrix() */
{
    free((int *) (m[nrl]+ncl-NR_END));
    free((int *) (m+nrl-NR_END));
}

void free_matrix(long **m, long nrl, long nrh, long ncl, long nch)
/* free a float matrix allocated by matrix() */
{
    free((long *) (m[nrl]+ncl-NR_END));
    free((long *) (m+nrl-NR_END));
}


void free_matrix(float **m, long nrl, long nrh, long ncl, long nch)
/* free a float matrix allocated by matrix() */
{
    free((float *) (m[nrl]+ncl-NR_END));
    free((float *) (m+nrl-NR_END));
}

//the num_len is fix to 8, other the data compress will be kind of complicated...
void svd_tool(unsigned int *src_addr, int nrow, int ncol, int num_term, int num_len, TBL_INFO_T rTableINFO, MUINT8 a_u1Mode)
{
    float	**a =NULL, *w = NULL, **u = NULL, **v = NULL, sum, runsum;

    //float *vector(long nl, long nh);
    //float **matrix(long nrl, long nrh, long ncl, long nch);
    //void	free_vector(float *v, long nl, long nh);
    //void	free_ivector(int *v, long nl, long nh);
    //void	free_matrix(float **m, long nrl, long nrh, long ncl, long nch);
    int		i, j, k;
    int     m , org;
    int     min[20][2][5];
    int     len_int[20][2][5];
    int     len_frac[20][2][5];
    int     *e_seq = NULL;
    int     mm;
    float   maxinput = 0.0;
    unsigned char u1temp;
    char pi4temp[4];

    double  avg_diff;
    float min_val = 0;
    float min_tmp;
    float max_val =0;
    float max_tmp;
    float f;
    int   d_tmp;
    int   len = 0;
    FILE    *fbout;
#if LSC_Debug_Table_Output
    FILE    *ftout;
#endif
    int idx0;


    char fileNameBin[100];
    switch(a_u1Mode) {
        case 0:
            sprintf(fileNameBin, "%s//Preview_LSC.bin", MEDIA_PATH);
            break;
        case 1:
            sprintf(fileNameBin, "%s//Capture_LSC.bin", MEDIA_PATH);
            break;
        case 2:
            sprintf(fileNameBin, "%s//Video_LSC.bin", MEDIA_PATH);
            break;
    }

    fbout  = fopen(fileNameBin, "wb");
    if(fbout == NULL) { ACDK_LOGE(" AcdkCalibration::cannot open %s\n", fileNameBin); exit(2); }
#if LSC_Debug_Table_Output
    if (a_u1Mode == 0)
    {
        ftout  = fopen(Preview_SVD_MATRIX_NAME, "wt");
        if(ftout == NULL) {ACDK_LOGE(" AcdkCalibration::cannot open %s\n", Preview_SVD_MATRIX_NAME); exit(2); }
    }
    else
    {
        ftout  = fopen(Capture_SVD_MATRIX_NAME, "wt");
        if(ftout == NULL) {ACDK_LOGE(" AcdkCalibration::cannot open %s\n", Capture_SVD_MATRIX_NAME); exit(2); }
    }
#endif

    ACDK_LOGD("AcdkCalibration:: nrow= %d, ncol= %d\n", nrow, ncol);
    ncol = ncol*4;
    a = (float **)matrix(a, 1, nrow, 1, ncol);		// input array (index starts with 1)
    u = (float **)matrix(u, 1, nrow, 1, ncol);		// output array (index starts with 1)
    v = (float **)matrix(v, 1, ncol, 1, ncol);		// output array (index starts with 1)
    w = (float *)vector(w, 1,ncol);
    e_seq = (int *)vector(e_seq, 1,ncol);

    /* read in input array */
    for(j = 1; j <= nrow; j++)
    {
        for(i = 1; i <= ncol; i+=4)
        {
            idx0 = ((j-1)*ncol/4+(i-1)/4)*2;

            a[j][i]=*(src_addr+(idx0  ))&0x0000ffff;
            a[j][i+1]=(*(src_addr+(idx0  ))>>16)&0x0000ffff;
            a[j][i+2]=*(src_addr+(idx0 + 1))&0x0000ffff;
            a[j][i+3]=(*(src_addr+(idx0 + 1))>>16)&0x0000ffff;

            for (m = 0 ; m <4 ; m++)
            {
                u[j][i+m]=a[j][i+m];
                if (u[j][i+m] > maxinput)
                {
                    maxinput = u[j][i+m];
                }
            }
        }
    }

    svdcali(u, nrow, ncol, w, v);
    eigen_sequence(w, e_seq, ncol);

    for(mm=1 ; mm<= num_term ; mm++)
    {
#if EIGEN_SEQUENCE_MODIFICATION
        m=1;
        while(mm !=  e_seq[m])
        {
            m++;
        }
#else
        m = mm;
#endif
        for(j = 1; j <= nrow; j++)
        {

            min_tmp = -sqrt(w[m])*u[j][m];
            if(j==1 || min_val > min_tmp)
                min_val = min_tmp;

            if(j==1 || min_tmp > max_val)
                max_val = min_tmp;
        }
        if(min_val<0)
        {
            min_val = (int)(abs(min_val))+1;
            min_val = - min_val;
        }
        else
            min_val = (int)min_val;

        d_tmp = (int)(max_val - min_val);

        for(i=1 ; i<14 ; i++)
        {
            if(pow(2,i)>d_tmp)
            {
                len = i;
                break;
            }
        }
        min[m][0][0] = min_val;
        len_int[m][0][0] = len;
        len_frac[m][0][0] = num_len - len;

        if(len_frac[m][0][0]<0)
        {
            ACDK_LOGE("AcdkCalibration::ERROR!! Please enter num_len >= %d\n",len);
        }
        for(i = 1; i <= ncol; i++)
        {
            min_tmp = -sqrt(w[m])*v[i][m];
            if(i==1 || min_val > min_tmp)
                min_val = min_tmp;
            if(i==1 || min_tmp > max_val)
                max_val = min_tmp;

        }
        if(min_val<0)
        {
            min_val = (int)(abs(min_val))+1;
            min_val = - min_val;
        }
        else
            min_val = (int)min_val;

        d_tmp = (int)(max_val - min_val);

        for(i=1 ; i<14 ; i++)
        {
            if(pow(2,i)>d_tmp)
            {
                len = i;
                break;
            }
        }
        min[m][1][0] = min_val;
        len_int[m][1][0] = len;
        len_frac[m][1][0] = num_len - len;
        if(len_frac[m][1][0]<0)
        {
            ACDK_LOGE("AcdkCalibration::!! Please enter num_len >= %d\n",len);
        }
    }


    /* this block outputs the singular values and their cummulative engergy */
    sum = 0.0;
    for(i = 1; i <= ncol; i++)
    {
        sum += w[i];
    }
    runsum = 0.0;
    ACDK_LOGD("AcdkCalibration ::\n");
    ACDK_LOGD("eigen value, power \n");
    for(i = 1; i <= ncol; i++)
    {
        runsum += w[i];
        ACDK_LOGD("[%s] w[%d] %7.4g runsum %7.4g, sum %7.4g, %7.4g\n",__FUNCTION__,
                i, w[i], runsum, sum, runsum/sum);
    }


    /* this block check average difference */
    avg_diff = 0;
    for(j = 1; j <= nrow; j++)
    {
        for(i = 1; i <= ncol; i++)
        {
            org = (int) (a[j][i]+0.5);
            f = 0;
            for(mm=1 ; mm<= num_term ; mm++)
            {
#if EIGEN_SEQUENCE_MODIFICATION
                m=1;
                while(mm !=  e_seq[m])
                {
                    m++;
                }
#else
                m = mm;
#endif
                f+=truncate(w[m],u[j][m],min[m][0][0],len_frac[m][0][0])*truncate(w[m],v[i][m],min[m][1][0],len_frac[m][1][0]);
            }
            k = (int)(f+0.5);
            avg_diff += abs(org-k);
        }

    }
    avg_diff = avg_diff / (nrow*ncol) / maxinput;
    ACDK_LOGD("AcdkCalibration::average difference (normalize to maxmun input element) = %f\n",avg_diff);


    //store table info for hw table calculation --> 8 bytes
    fwrite(&(rTableINFO.reg_mn), 1, sizeof(rTableINFO.reg_mn), fbout); //u min is signed int
    fwrite(&(rTableINFO.reg_info0), 1, sizeof(rTableINFO.reg_info0), fbout); //u min is signed int
    fwrite(&(rTableINFO.reg_info1), 1, sizeof(rTableINFO.reg_info1), fbout); //u min is signed int
#if LSC_Debug_Table_Output
    fprintf(ftout, "unsigned char SVD_MATRIX[%d] = {\n", (nrow+ncol+10)*num_term + 2 + 8);
    //    u1temp = (char)(rTableINFO.reg_mn);
    //    fprintf(ftout, "0x%02X, ", u1temp);

    u1temp = (char)(rTableINFO.reg_mn);
    fprintf(ftout, "0x%02X, ", u1temp);
    u1temp = (char)(rTableINFO.reg_mn>>8);
    fprintf(ftout, "0x%02X, ", u1temp);
    u1temp = (char)(rTableINFO.reg_mn>>16);
    fprintf(ftout, "0x%02X, ", u1temp);
    u1temp = (char)(rTableINFO.reg_mn>>24);
    fprintf(ftout, "0x%02X, ", u1temp);
    u1temp = (char)(rTableINFO.reg_info0);
    fprintf(ftout, "0x%02X, ", u1temp);
    u1temp = (char)(rTableINFO.reg_info0>>8);
    fprintf(ftout, "0x%02X, ", u1temp);
    u1temp = (char)(rTableINFO.reg_info0>>16);
    fprintf(ftout, "0x%02X, ", u1temp);
    u1temp = (char)(rTableINFO.reg_info0>>24);
    fprintf(ftout, "0x%02X, ", u1temp);
    u1temp = (char)(rTableINFO.reg_info1);
    fprintf(ftout, "0x%02X, ", u1temp);
    u1temp = (char)(rTableINFO.reg_info1>>8);
    fprintf(ftout, "0x%02X, ", u1temp);
    u1temp = (char)(rTableINFO.reg_info1>>16);
    fprintf(ftout, "0x%02X, ", u1temp);
    u1temp = (char)(rTableINFO.reg_info1>>24);
    fprintf(ftout, "0x%02X, \n", u1temp);
#endif

    //store term_num
    u1temp = (unsigned char)num_term;
    fwrite(&u1temp, 1, 1, fbout);
#if LSC_Debug_Table_Output
    fprintf(ftout, "0x%02X, ", u1temp);
#endif

    //store num_len
    u1temp = (unsigned char)num_len;
    fwrite(&u1temp, 1, 1, fbout);
#if LSC_Debug_Table_Output
    fprintf(ftout, "0x%02X, \n", u1temp);
#endif

    //store minimum value and length of fraction part
    for(mm=1 ; mm<= num_term ; mm++)
    {
#if EIGEN_SEQUENCE_MODIFICATION
        m=1;
        while(mm !=  e_seq[m])
        {
            m++;
        }
#else
        m = mm;
#endif
        fwrite(&min[m][0][0], 1, sizeof(min[m][0][0]), fbout); //u min is signed int
#if LSC_Debug_Table_Output
        u1temp = (char)(min[m][0][0]);
        fprintf(ftout, "0x%02X, ", u1temp);
        u1temp = (char)(min[m][0][0]>>8);
        fprintf(ftout, "0x%02X, ", u1temp);
        u1temp = (char)(min[m][0][0]>>16);
        fprintf(ftout, "0x%02X, ", u1temp);
        u1temp = (char)(min[m][0][0]>>24);
        fprintf(ftout, "0x%02X, ", u1temp);
#endif
        u1temp = (unsigned char)len_frac[m][0][0]; //u fraction length
        fwrite(&u1temp, 1, 1 , fbout);
#if LSC_Debug_Table_Output
        fprintf(ftout, "0x%02X, ", u1temp);
#endif

        fwrite(&min[m][1][0], 1, sizeof(min[m][1][0]), fbout);//v min is signed int
#if LSC_Debug_Table_Output
        u1temp = (char)(min[m][1][0]);
        fprintf(ftout, "0x%02X, ", u1temp);
        u1temp = (char)(min[m][1][0]>>8);
        fprintf(ftout, "0x%02X, ", u1temp);
        u1temp = (char)(min[m][1][0]>>16);
        fprintf(ftout, "0x%02X, ", u1temp);
        u1temp = (char)(min[m][1][0]>>24);
        fprintf(ftout, "0x%02X, ", u1temp);
#endif

        u1temp = (unsigned char)len_frac[m][1][0]; //v fraction length
        fwrite(&u1temp, 1, 1 , fbout);
#if LSC_Debug_Table_Output
        fprintf(ftout, "0x%02X, \n", u1temp);
#endif
    }

    //output U (nrow x m) matrix
    for(j = 1; j <= nrow; j++)
    {
        for(mm=1 ; mm<= num_term ; mm++)
        {
#if EIGEN_SEQUENCE_MODIFICATION
            m=1;
            while(mm !=  e_seq[m])
            {
                m++;
            }
#else
            m = mm;
#endif
            //the element in U < 2^num_len , and in our application num_len = 8.
            u1temp = (unsigned char)truncate_UV(w[m],u[j][m],min[m][0][0],len_frac[m][0][0]);
            fwrite(&u1temp, 1, 1 , fbout);
#if LSC_Debug_Table_Output
            fprintf(ftout, "0x%02X, ", u1temp);
#endif
        }
#if LSC_Debug_Table_Output
        fprintf(ftout, "\n");
#endif
    }

    //output V (m x ncol) matrix
    for(mm=1 ; mm<= num_term ; mm++)
    {
#if EIGEN_SEQUENCE_MODIFICATION
        m=1;
        while(mm !=  e_seq[m])
        {
            m++;
        }
#else
        m = mm;
#endif
        for(i = 1; i <= ncol; i++)
        {
            //the element in V < 2^num_len , and in our application num_len = 8.
            u1temp = (unsigned char)truncate_UV(w[m],v[i][m],min[m][1][0],len_frac[m][1][0]);
            fwrite(&u1temp, 1, 1 , fbout);
#if LSC_Debug_Table_Output
            fprintf(ftout, "0x%02X, ", u1temp);
#endif
        }
#if LSC_Debug_Table_Output
        fprintf(ftout, "\n");
#endif
    }
#if LSC_Debug_Table_Output
    fprintf(ftout, "};\n");
    fclose(ftout);
#endif
    fclose(fbout);

    m = (nrow + ncol + 10)*num_term*num_len;
    m = (int)((m+7)/8)+2 + 8;
    ACDK_LOGD("AcdkCalibration:: Total bytecnt = %d\n", m);

    free_vector(w,1,ncol);
    free_vector(e_seq, 1, ncol);
    free_matrix(a,1, nrow, 1, ncol);
    free_matrix(u,1, nrow, 1, ncol);
    free_matrix(v,1, ncol, 1, ncol);
}


/////////////////////////////////////////////////////////////////////////
//
//   vLSC_2nd_Derivative_Get () -
//
/////////////////////////////////////////////////////////////////////////
void vlsc2ndDerivativeGet(DIM_INFO_T d, int y[], float y2[])
{
    int i,n = d.grid_num;
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

/////////////////////////////////////////////////////////////////////////
//
//   vLSC_RECT_COEF_Get () -
//
/////////////////////////////////////////////////////////////////////////
void vlscRectCoefGet(BLK_INFO_T binfo, int coef[])
{
    int f00, f10, f01, f11;
    int sx2, sy2;
    int f00x2, f10x2, f01x2, f11x2, f00y2, f10y2, f01y2, f11y2;
    int a31, a13, a30, a21, a12, a03, a20, a11, a02, a10, a01, a00;
    int b3, b2, b1, c3, c2;
    int fbit=COEF_FXX_BIT;
    int val_div6=(int)COEF_DIV6_VAL;
    int val_div3=(int)COEF_DIV3_VAL;

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


/*******************************************************************************
 *
 ********************************************************************************/
void lscReConstruct(int nrow, int ncol, unsigned char *input_buffer, unsigned short *output_buffer)
{
    SVD_UV_INFO_T rUV_INFO;
    TBL_INFO_T rTableINTO;
    int     min[20][2];// num_term, u/v,
    int     len_frac[20][2];
    unsigned int readsize = 0;
    unsigned char **u, **v;
    char num_term = 1;
    char num_len = 1;
    int m, i, j;
    unsigned short k;
    float f = 0.0;

//    for (i = 0; i < 4*3+2; i++)
//        ACDK_LOGE("[%s] %d, 0x%1x", __FUNCTION__, i,  *(unsigned char*)(input_buffer+i));


    readsize = sizeof(rTableINTO.reg_mn)+sizeof(rTableINTO.reg_info0)+sizeof(rTableINTO.reg_info1); //lst 8 bytes is for hw table
    //num_term
    num_term = *(input_buffer + readsize);
    readsize ++;
    num_len = *(input_buffer + readsize);
    readsize ++;

    ACDK_LOGD("[%s] readsize %d, term %d, len %d\n", __FUNCTION__,
            readsize,
            num_term,
            num_len);

    // allocate memory to contain the whole file:
    u = 0;
    v = 0;
    u = (unsigned char **)matrix(u , 1,nrow,1,num_term);		// output array (index starts with 1)
    v = (unsigned char **)matrix(v , 1,ncol,1,num_term);		// output array (index starts with 1)

    //u = cmatrix(1,nrow,1,num_term);		// output array (index starts with 1)
    //v = cmatrix(1,ncol,1,num_term);		// output array (index starts with 1)


    for (m = 1 ; m <= num_term ; m++)
    {
        //U info
        memcpy(&rUV_INFO, input_buffer+readsize, 5);
        readsize += 5;
        min[m][0] = rUV_INFO.min_val;
        len_frac[m][0] = rUV_INFO.len_frac;
        //V info
        memcpy(&rUV_INFO, input_buffer+readsize, 5);
        readsize += 5;
        min[m][1] = rUV_INFO.min_val;
        len_frac[m][1] = rUV_INFO.len_frac;
    }

    for (j = 1 ; j <= nrow ; j++)
    {
        for(m = 1 ; m <= num_term ; m++)
        {
            u[j][m] = *(input_buffer + readsize);
            readsize++;
        }
    }

    for(m = 1 ; m <= num_term ; m++)
    {
        for (i = 1 ; i <= ncol ; i++)
        {
            v[i][m] = *(input_buffer + readsize);
            readsize++;
        }
    }


    for(j = 1; j <= nrow; j++)
    {
        for(i = 1; i <= ncol; i++)
        {
            f = 0.0;
            for(m=1 ; m <= num_term ; m++)
            {
                f+=( ( ((float)u[j][m])/pow(2,len_frac[m][0]) ) + min[m][0])*((((float)v[i][m])/pow(2,len_frac[m][1])) + min[m][1]);
            }
            if (f > 65535) f = 65535;
            k = (unsigned short)(f+0.5);
            *(output_buffer + ((j-1)*ncol + (i-1))) = k;
        }
    }
    free_matrix(u, 1, nrow, 1, num_term);
    free_matrix(v, 1, ncol, 1, num_term);

    ACDK_LOGD("[%s] Done!!\n", __FUNCTION__);
}

/////////////////////////////////////////////////////////////////////////
//
//   vLSC_HW_TBL_CAL () -
//
/////////////////////////////////////////////////////////////////////////
void vlscHWTBLCAL(TBL_INFO_T info)
{
    int kk, jj, ii,i;
    int m,n,nn;
    DIM_INFO_T dim_x,dim_y;
    BLK_INFO_T blk_info;
    int coef1, coef2, coef3;
    unsigned int blk_mn, blk_info_0,blk_info_1;
    unsigned int *p_pgn;
    unsigned int *p_lsc_tbl;
    int nbayer;

    //for hardware table
    int zz[4][GRID_MAX][GRID_MAX];
    int zzb[GRID_MAX];
    float zh2[GRID_MAX][GRID_MAX];
    float zv2[GRID_MAX][GRID_MAX];
    float y2[GRID_MAX];
    float yx2[GRID_MAX];
    float yy2[GRID_MAX];
    int coef[4][(GRID_MAX-1)*(GRID_MAX-1)][12];

    /* parse block info */
    blk_mn = info.reg_mn;
    blk_info_0=info.reg_info0;
    blk_info_1=info.reg_info1;
    p_pgn     =info.src_tbl_addr;
    p_lsc_tbl =info.dst_tbl_addr;

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

//    ACDK_LOGD("[%s] m,n %d,%d\n", __FUNCTION__,
//            m, n);

    printf("[%s] m,n %d,%d\n", __FUNCTION__,
                m, n);
    //if(m > 8 || n > GRID_MAX || m > GRID_MAX)
    //     MHAL_LOG(" LSC calculate HW table ERROR! X Grid must <=8 \n");

    /* get original 2nd derivatives */
    nn = m;
    if(m<n) nn=n;
    y2[0]=0.0;
    for (i=1;i<nn-1;i++) {
        y2[i]= (float) (-1.0/(y2[i-1]+4.0));
    }
    y2[nn-1]=0.0;

//    ACDK_LOGD("[%s] Get real pixel\n", __FUNCTION__);
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
//        ACDK_LOGD("[%s] Each channel %d\n", __FUNCTION__, nbayer);
        /* get final 2nd derivatives in x-dir */
        for (jj=0;jj<n;jj++)
        {
            for (ii=0;ii<m-1;ii++) {
                yx2[ii]=y2[ii];
            }
            yx2[ii]=0.0;
            vlsc2ndDerivativeGet(dim_x,zz[nbayer][jj],yx2);
            for (ii=0;ii<m;ii++)
            {
                zh2[jj][ii]=yx2[ii];
            }
        }

//        ACDK_LOGD("[%s] get final 2nd derivatives in y-dir\n", __FUNCTION__);
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

            vlsc2ndDerivativeGet(dim_y,zzb,yy2);
            for (jj=0;jj<n;jj++)
            {
                zv2[jj][ii]=yy2[jj];
            }
        }
//        ACDK_LOGD("[%s] get 12 coef\n", __FUNCTION__);
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
                vlscRectCoefGet(blk_info,coef[nbayer][jj*(m-1)+ii]);
            }
        }
    }


//    ACDK_LOGD("[%s] Output //data//hwtbl.bin\n", __FUNCTION__);
    FILE *fbout;
    fbout  = fopen("//data//hwtbl.bin", "wb");

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
                    coef1 = (int)coef[nbayer][jj*(m-1)+ii][kk  ];
                    if(coef1 < 0) coef1 = (512-coef1);
                    coef2 = (int)coef[nbayer][jj*(m-1)+ii][kk+1];
                    if(coef2 < 0) coef2 = (512-coef2);
                    coef3 = (int)coef[nbayer][jj*(m-1)+ii][kk+2];
                    if(coef3 < 0) coef3 = (512-coef3);
                    p_lsc_tbl[nn++] = ((coef3)<<20)|((coef2)<<10)|(coef1);
                    fwrite(&p_lsc_tbl[nn-1], 4, 1, fbout);
                }
            }
        }
    }
    fclose(fbout);

//    ACDK_LOGD("[%s] Done\n", __FUNCTION__);

    printf("[%s]Done\n", __FUNCTION__);
}


/////////////////////////////////////////////////////////////////////////
//
//   u4LSC_Integer_SQRT () -
//
/////////////////////////////////////////////////////////////////////////
MUINT32 u4LSC_Integer_SQRT(unsigned long long x)
{
#define INIT_GUESS    30
    unsigned long long op, res, one;
    op = x;
    res = 0;
    one = (1uL << INIT_GUESS);

    while (one > op) one >>= 2;
    while (one != 0)
    {
        if (op >= res + one)
        {
            op = op - (res + one);
            res = res +  2 * one;
        }
        res >>= 1;
        one >>= 2;
    }
    return((MUINT32)res);
}

/////////////////////////////////////////////////////////////////////////
//
//   mrLSC_POLY_Ratio_Get () -
//
/////////////////////////////////////////////////////////////////////////
MINT32 mrLSC_POLY_Ratio_Get(int d, int dmax, RATIO_POLY_T coef_t)
{
    float di,dni,ri;
    float coefpoly[6];
    int k;
    coefpoly[0]=coef_t.coef_f;
    coefpoly[1]=coef_t.coef_e;
    coefpoly[2]=coef_t.coef_d;
    coefpoly[3]=coef_t.coef_c;
    coefpoly[4]=coef_t.coef_b;
    coefpoly[5]=coef_t.coef_a;

    di=(float)d/(float)dmax;
    dni=di;
    ri=coefpoly[0];
    for(k=1 ; k<6 ; k++)
    {
        ri+=(coefpoly[k]*dni);
        dni*=di;
    }
    return ((int)(ri*(float)(1<<RATIO_POLY_BIT)));
}

/////////////////////////////////////////////////////////////////////////
//
//   vLSC_PARAM_INIT () -
//
/////////////////////////////////////////////////////////////////////////
void vLSC_PARAM_INIT(LSC_CAL_INI_PARAM_T a_rLSCCaliINIParam)
{
    int i,u;
    /* init calibration parameters */
    g_lsc_param.raw_wd=a_rLSCCaliINIParam.u4ImgWidth;
    g_lsc_param.raw_ht=a_rLSCCaliINIParam.u4ImgHeight;
    g_lsc_param.avg_pixel_size=AVG_PIXEL_SIZE;
    g_lsc_param.bayer_order=a_rLSCCaliINIParam.u2BayerStart;
    g_lsc_param.crop_ini_x=a_rLSCCaliINIParam.i4XINIBorder>>1;
    g_lsc_param.crop_ini_y=a_rLSCCaliINIParam.i4YINIBorder>>1;
    g_lsc_param.crop_end_x=(a_rLSCCaliINIParam.u4ImgWidth- a_rLSCCaliINIParam.i4XENDBorder)>>1;
    g_lsc_param.crop_end_y=(a_rLSCCaliINIParam.u4ImgHeight- a_rLSCCaliINIParam.i4YENDBorder)>>1;
    g_lsc_param.x_grid_num=a_rLSCCaliINIParam.i4GridXNUM;
    g_lsc_param.y_grid_num=a_rLSCCaliINIParam.i4GridYNUM;
    g_lsc_param.pxl_gain_max=MAX_PIXEL_GAIN;
    g_lsc_param.poly_coef.coef_a=a_rLSCCaliINIParam.poly_coef.coef_a;
    g_lsc_param.poly_coef.coef_b=a_rLSCCaliINIParam.poly_coef.coef_b;
    g_lsc_param.poly_coef.coef_c=a_rLSCCaliINIParam.poly_coef.coef_c;
    g_lsc_param.poly_coef.coef_d=a_rLSCCaliINIParam.poly_coef.coef_d;
    g_lsc_param.poly_coef.coef_e=a_rLSCCaliINIParam.poly_coef.coef_e;
    g_lsc_param.poly_coef.coef_f=a_rLSCCaliINIParam.poly_coef.coef_f;
    /* check if apply ratio compensation from poly coef */
    if( g_lsc_param.poly_coef.coef_a!=0.0 ||
            g_lsc_param.poly_coef.coef_b!=0.0 ||
            g_lsc_param.poly_coef.coef_c!=0.0 ||
            g_lsc_param.poly_coef.coef_d!=0.0 ||
            g_lsc_param.poly_coef.coef_e!=0.0) g_lsc_param.poly_coef.ratio_poly_flag=1;
    else g_lsc_param.poly_coef.ratio_poly_flag=0;
    g_lsc_param.plane_wd=(a_rLSCCaliINIParam.u4ImgWidth>>1);
    g_lsc_param.plane_ht=(a_rLSCCaliINIParam.u4ImgHeight>>1);
    g_lsc_param.block_wd=(g_lsc_param.plane_wd)/(g_lsc_param.x_grid_num-1);
    g_lsc_param.block_ht=(g_lsc_param.plane_ht)/(g_lsc_param.y_grid_num-1);
    g_lsc_param.block_wd_last=g_lsc_param.plane_wd-(g_lsc_param.block_wd)*(g_lsc_param.x_grid_num-2);
    g_lsc_param.block_ht_last=g_lsc_param.plane_ht-(g_lsc_param.block_ht)*(g_lsc_param.y_grid_num-2);
    /* get right shift bit as the divider of block avg */
    i=0;u=g_lsc_param.avg_pixel_size;
    do{u>>=1;i++;}while(u>1);
    g_lsc_param.avg_pixel_size_bit=i;
    /* condition check */
    //if(g_lsc_param.raw_wd-(g_lsc_param.raw_wd&0xfffffffc)) // raw width must multiple of 4 pixel
    //	ACDK_LOGE(" ERROR :: LSC calibraton raw width illegal, raw width must multiple of 4 pixel \n");
    if(g_lsc_param.raw_wd-(g_lsc_param.raw_wd&0xfffffffe)) // raw width must multiple of 4 pixel
        ACDK_LOGE(" ERROR :: LSC calibraton raw width illegal, raw width must multiple of 2 pixel \n");
    if(g_lsc_param.raw_ht-(g_lsc_param.raw_ht&0xfffffffe)) // raw height must multiple of 2 pixel
        ACDK_LOGE(" ERROR :: LSC calibraton raw height illegal, raw height must multiple of 2 pixel \n");

}

/////////////////////////////////////////////////////////////////////////
//
//   vLSC_Calibration_INIT () -
//
/////////////////////////////////////////////////////////////////////////
void vLSC_Calibration_INIT(MUINT8* a_u1BufferAddr)
{
    int i,j;
    int m=g_lsc_param.x_grid_num;
    int n=g_lsc_param.y_grid_num;
    int blk_wd=g_lsc_param.block_wd;
    int blk_ht=g_lsc_param.block_ht;
    int blk_wd_last=g_lsc_param.block_wd_last;
    int blk_ht_last=g_lsc_param.block_ht_last;

    /* init source image & working buffer */
    //g_raw = (unsigned int *)malloc((RAW_X_DIM*RAW_Y_DIM/4)*sizeof(unsigned int));
    g_raw = (unsigned short *)a_u1BufferAddr;
    //lsc_raw8_load(RAW_IN_NAME, g_raw, RAW_X_DIM*RAW_Y_DIM/4);
    g_src_tbl = (unsigned int *)malloc(m*n*2*sizeof(unsigned int));
    g_dst_tbl = (unsigned int *)malloc((m-1)*(n-1)*16*sizeof(unsigned int));
    /* init max position & value search */
    memset( &g_lsc_result, 0, sizeof(LSC_RESULT_T) );
    /* init grid x coordinates */
    g_col_idx[0] = 0;
    g_col_idx[g_lsc_param.x_grid_num-1] = g_lsc_param.plane_wd-1;
    for(i = 1; i < g_lsc_param.x_grid_num-1; i++) g_col_idx[i]=g_lsc_param.block_wd*i;
    /* init grid y coordinates */
    g_row_idx[0] = 0;
    g_row_idx[g_lsc_param.y_grid_num-1] = g_lsc_param.plane_ht-1;
    for(j = 1; j < g_lsc_param.y_grid_num-1; j++) g_row_idx[j]=g_lsc_param.block_ht*j;
    /* init calibration informaiton */
    //g_lsc_cali_info.raw_img_addr=g_raw;
    g_lsc_cali_info.raw_img_addr=g_raw;
    //g_lsc_cali_info.tbl_info.reg_info0=(((m-2)<<28)&0xf0000000)|((blk_wd<<16)&0x0fff0000) |(((n-2)<<12)&0x0000f000)|((blk_ht)&0x00000fff);;
    //g_lsc_cali_info.tbl_info.reg_info1=((blk_wd_last<<16)&0x0fff0000)|(blk_ht_last&0x00000fff);;

    g_lsc_cali_info.tbl_info.reg_mn=(((m-2)<<16)&0xffff0000) | (((n-2))&0x0000ffff);
    g_lsc_cali_info.tbl_info.reg_info0=((blk_wd<<16)&0xffff0000) | ((blk_ht)&0x0000ffff);
    g_lsc_cali_info.tbl_info.reg_info1=((blk_wd_last<<16)&0xffff0000)|(blk_ht_last&0x0000ffff);
    g_lsc_cali_info.tbl_info.src_tbl_addr=g_src_tbl;
    g_lsc_cali_info.tbl_info.dst_tbl_addr=g_dst_tbl;
}


MINT32 ModifyBayerOrder(MUINT32 *coeff_table, MUINT32 length, int old_order, int new_order)
{
    MUINT32 coeff_idx = 0;
    MUINT32 coeff_temp[4][4];
    char ColorOrder[4][16] = {
            {"B|Gb|Gr|R"}, // 0
            {"Gb|B|R|Gr"}, // 1
            {"Gr|R|B|Gb"}, // 2
            {"R|Gr|Gb|B"}, // 3
    };

    MINT32 iColorOrder[4][4] = {
      {0,  0,  0, 0},
      {-1, 1, -1, 1},
      {-2, -2, 2, 2},
      {-3, -1, 1, 3},
    };

    ACDK_LOGD("[%s] Orig [%s] to New [%s], length %d\n", __FUNCTION__,
        ColorOrder[old_order],
        ColorOrder[new_order],
        length);

     int idx0 = 4*((0-iColorOrder[old_order][0]-iColorOrder[new_order][0]+4)%4);
     int idx1 = 4*((1-iColorOrder[old_order][1]-iColorOrder[new_order][1]+4)%4);
     int idx2 = 4*((2-iColorOrder[old_order][2]-iColorOrder[new_order][2]+4)%4);
     int idx3 = 4*((3-iColorOrder[old_order][3]-iColorOrder[new_order][3]+4)%4);
     ACDK_LOGD("0->%d(%d, %d, %d), 1->%d(%d, %d, %d), 2->%d(%d, %d, %d), 3->%d(%d, %d, %d)\n",
         idx0, 0, iColorOrder[old_order][0], iColorOrder[new_order][0],
         idx1, 1, iColorOrder[old_order][1], iColorOrder[new_order][1],
         idx2, 2, iColorOrder[old_order][2], iColorOrder[new_order][2],
         idx3, 3, iColorOrder[old_order][3], iColorOrder[new_order][3]);

     for (coeff_idx = 0; coeff_idx < length; coeff_idx += 4*4) {
       //        ACDK_LOGD("[%s] idx %d, Backup 0x%04x, size %d, ch size %d\n", __FUNCTION__,
       //                coeff_idx, &coeff_table[coeff_idx], sizeof(coeff_temp), sizeof(coeff_temp[0]));
       memcpy(coeff_temp, &coeff_table[coeff_idx], sizeof(coeff_temp));

       memcpy(&coeff_table[coeff_idx+idx0], coeff_temp[0], sizeof(coeff_temp[0]));
       memcpy(&coeff_table[coeff_idx+idx1], coeff_temp[1], sizeof(coeff_temp[0]));
       memcpy(&coeff_table[coeff_idx+idx2], coeff_temp[2], sizeof(coeff_temp[0]));
       memcpy(&coeff_table[coeff_idx+idx3], coeff_temp[3], sizeof(coeff_temp[0]));
     }

    return MTRUE;
}
/////////////////////////////////////////////////////////////////////////
//
//   mrLSC_Calibrate () -
//
/////////////////////////////////////////////////////////////////////////
MINT32 mrLSC_Calibrate(LSC_CALI_INFO_T cali_info, MUINT8 a_u1Mode, MUINT16 a_u2SVDTermNum)
{
    int i,j,k,bi,bj;
    int kx,ky,x0,y0,xn,yn;
    int idx0,idx1;
    unsigned int vlong0,vlong1;
    int val[4];
    int bayer_order=g_lsc_param.bayer_order; // first pixel in which plane
    int m=g_lsc_param.x_grid_num; // x-dir grid number
    int n=g_lsc_param.y_grid_num; // y-dir grid number
    int avg_pxl=g_lsc_param.avg_pixel_size; // avg_pxl*avg_pxl pixels sqare region for pixel value average, avoid noise
    unsigned long long wd=g_lsc_param.plane_wd; // raw domain width
    unsigned long long ht=g_lsc_param.plane_ht; // raw domain height
    int sb=(g_lsc_param.avg_pixel_size_bit)*2; // shift bit after accumulation of pixel values in avg_pxl*avg_pxl square region
    int pxl_gain_max; // max pixel gain in fix point
    int ratio_poly; // compensation ratio to original from polynominal calculation
    int ratio_poly_enable=g_lsc_param.poly_coef.ratio_poly_flag; // en/disable of compensation ratio
    int dis_max; // for compensation ratio usage, the max distance to center point of raw
    int dis_cur; // for compensation ratio usage, distance to center from current grid point
    //int wd32=g_lsc_param.raw_wd/4; // 32 bit raw data array line width, 4pixel packed, original width/4
    //unsigned int *raw_addr=cali_info.raw_img_addr; // raw data address
    int wd16=g_lsc_param.raw_wd/2; // 16 bit raw data array line width, 2pixel packed, original width/2
    unsigned short *raw_addr=cali_info.raw_img_addr; // raw data address
    TBL_INFO_T tbl_cal_info=cali_info.tbl_info; // parameters for table calculation
    unsigned int *src_addr=tbl_cal_info.src_tbl_addr; // source pixel gain table address

    MINT32 mrRet = S_ACDK_CALIBRATION_OK;
    ACDK_LOGD("[%s] m,n (%d,%d), plane wd,ht (%d, %d) raw wd,ht (%d, %d)\n", __FUNCTION__,
            m, n,
            wd, ht,
            g_lsc_param.raw_wd,
            g_lsc_param.raw_ht);

    /* get average pixel values near grid points, (grid point-avg_pxl/2) ~ (grid point+avg_pxl/2) */
    for(j = 0; j < n; j++)
    {
        ky = g_row_idx[j];
        if(ky<(g_lsc_param.crop_ini_y)) ky=g_lsc_param.crop_ini_y;
        if(ky>(g_lsc_param.crop_end_y)) ky=g_lsc_param.crop_end_y;
        for(i = 0; i < m; i++)
        {
            kx = g_col_idx[i];
            if(kx<(g_lsc_param.crop_ini_x)) kx=g_lsc_param.crop_ini_x;
            if(kx>(g_lsc_param.crop_end_x)) kx=g_lsc_param.crop_end_x;

            val[0]=val[1]=val[2]=val[3]=0;
            /* x block extent define */
            x0=kx; // leftest
            if(kx>(wd-avg_pxl)) x0=kx-avg_pxl+1; // rightest
            else if(kx>(avg_pxl>>1)) x0=kx-(avg_pxl>>1)+1; // general case
            if((x0>>1)!=((x0+1)>>1)) x0-=1; // x0 must even
            if(x0<0) x0=0;
            /* y block extent define */
            y0=ky;
            if(ky>(ht-avg_pxl)) y0=ky-avg_pxl+1;
            else if(ky>(avg_pxl>>1)) y0=ky-(avg_pxl>>1)+1;
            if(y0<0) y0=0;
            /* do average */
            for(bj=0;bj<avg_pxl;bj++)
            {
                for(bi=0;bi<avg_pxl;bi+=1)
                {
#if 0	//8 bits raw
                    idx0=((y0+bj)*2+0)*wd16+((x0+bi));
                    idx1=((y0+bj)*2+1)*wd16+((x0+bi));

                    vlong0=*(raw_addr+idx0);
                    vlong1=*(raw_addr+idx1);

                    val[0] += ((vlong0    )&0xff); // #0 plane,B
                    val[1] += ((vlong0>> 8)&0xff); // #1 plane,Gb
                    val[2] += ((vlong1    )&0xff); // #2 plane,Gr
                    val[3] += ((vlong1>> 8)&0xff); // #3 plane,R
#else     //10 bits raw
                    idx0=((y0+bj)*2+0)*g_lsc_param.raw_wd+((x0+bi)*2);
                    idx1=((y0+bj)*2+1)*g_lsc_param.raw_wd+((x0+bi)*2);
                    val[0] += *(raw_addr+idx0); // #0 plane,B
                    val[1] += *(raw_addr+idx0+1); // #1 plane,Gb
                    val[2] += *(raw_addr+idx1); // #2 plane,Gr
                    val[3] += *(raw_addr+idx1+1); // #3 plane,R
#endif
                }
            }

//            ACDK_LOGD("[%s] avg val[0-3] %d, %d, %d, %d\n", __FUNCTION__,
//                    val[0], val[1], val[2], val[3]);

            /* search max value between grid point average values */
            for(k=0;k<4;k++)
            {
                val[k] >>= sb;
                if(val[k]>g_lsc_result.max_val[k])
                {
                    g_lsc_result.max_val[k]=val[k];
                    g_lsc_result.x_max_pos[k]=kx;
                    g_lsc_result.y_max_pos[k]=ky;
                }
            }
            /* save to pixel gain table temporalily as working buffer */
            idx0=(j*m+i)*2;
            *(src_addr+(idx0  ))=((val[1]<<16)&0xffff0000)|((val[0])&0x0000ffff);
            *(src_addr+(idx0+1))=((val[3]<<16)&0xffff0000)|((val[2])&0x0000ffff);
        }
    }

    /* prepare start & end index for max value search near raw center point */
    if(((m+1)>>1)==(m>>1)) kx=(m>>1); // x grid number even -> search from idx[(m/2-1)]~idx[(m/2)]
    else kx=(m>>1)+1; // x grid number odd -> search from idx[m/2-1)]~idx[(m/2+1)]
    if(((n+1)>>1)==(n>>1)) ky=(n>>1);
    else ky=(n>>1)+1;
    x0=g_col_idx[m/2-1];
    xn=g_col_idx[kx];
    y0=g_row_idx[n/2-1];
    yn=g_row_idx[ky];
    if((x0>>1)!=((x0+1)>>1)) x0+=1;
    /* start max value search @ centre near region */
    for(j=y0;j<=yn;j+=avg_pxl)
    {
        for(i=x0;i<=xn;i+=avg_pxl)
        {
            val[0]=val[1]=val[2]=val[3]=0;
            /* do average */
            for(bj=0;bj<avg_pxl;bj++)
            {
                for(bi=0;bi<avg_pxl;bi+=1)
                {
#if 0   // 8 bits raw
                    idx0=((j+bj)*2+0)*wd16+((i+bi));
                    idx1=((j+bj)*2+1)*wd16+((i+bi));
                    vlong0=*(raw_addr+idx0);
                    vlong1=*(raw_addr+idx1);

                    val[0] += ((vlong0    )&0xff); // #0 plane,B
                    val[1] += ((vlong0>> 8)&0xff); // #1 plane,Gb
                    val[2] += ((vlong1    )&0xff); // #2 plane,Gr
                    val[3] += ((vlong1>> 8)&0xff); // #3 plane,R
#else     //10 bits raw
                    idx0=((j+bj)*2+0)*g_lsc_param.raw_wd+((i+bi)*2);
                    idx1=((j+bj)*2+1)*g_lsc_param.raw_wd+((i+bi)*2);
                    val[0] += *(raw_addr+idx0); // #0 plane,B
                    val[1] += *(raw_addr+idx0+1); // #1 plane,Gb
                    val[2] += *(raw_addr+idx1); // #2 plane,Gr
                    val[3] += *(raw_addr+idx1+1); // #3 plane,R
#endif
                }
            }

//            ACDK_LOGD("[%s] near center val[0-3] %d, %d, %d, %d\n", __FUNCTION__,
//                                val[0], val[1], val[2], val[3]);

            /* search max value near center points */
            for(k=0;k<4;k++)
            {
                val[k] >>= sb;
                if(val[k]>g_lsc_result.max_val[k])
                {
                    g_lsc_result.max_val[k]=val[k];
                    g_lsc_result.x_max_pos[k]=i+(avg_pxl>>1);
                    g_lsc_result.y_max_pos[k]=j+(avg_pxl>>1);
                }
            }
        }
    }

    pxl_gain_max=MIN2(((1<<16)-1),(g_lsc_param.pxl_gain_max)<<GN_BIT); /* pixel gain upper bound is min of user defined or 16 bit */
    ratio_poly=(1<<RATIO_POLY_BIT); /* default ratio of compensation ratio (1.0), but in fix point to RATIO_POLY_BIT bit */
    dis_max=u4LSC_Integer_SQRT((unsigned long long)((((long long)wd*(long long)wd)>>2)+(((long long)ht*(long long)ht)>>2))<<SQRT_NORMALIZE_BIT); // max distance to raw center: from (0,0) to (wd/2,ht/2), for sqrt calculation, right shift SQRT_NORMALIZE_BIT bit for accuracy

    ACDK_LOGD("[%s] gain_max, ratio dis_max %d, %d, %d", __FUNCTION__,
            pxl_gain_max, ratio_poly, dis_max);
#if LSC_Debug_Table_Output
    char fileName[100];
    sprintf(fileName, "%s//%04d_%04d.Table.dat", MEDIA_PATH, g_lsc_param.raw_wd, g_lsc_param.raw_ht);

    FILE *fin;
    fin  = fopen(fileName, "wt");
#endif
    for(j = 0; j < n; j++)
    {
        for(i = 0; i < m; i++)
        {
            /* do compensation ratio */
            if(ratio_poly_enable)
            {
                kx=g_col_idx[i];ky=g_row_idx[j];
                x0=(wd>>1);y0=(ht>>1);
                dis_cur=u4LSC_Integer_SQRT((unsigned long long)((long long)(kx-x0)*(long long)(kx-x0)+(long long)(ky-y0)*(long long)(ky-y0))<<SQRT_NORMALIZE_BIT);
                ratio_poly=mrLSC_POLY_Ratio_Get(dis_cur, dis_max, g_lsc_param.poly_coef);
            }
            /* parse original grid average pixel values from buffer */
            idx0=(j*m+i)*2;
            val[0]=(*(src_addr+(idx0  ))    )&0x0000ffff;
            val[1]=(*(src_addr+(idx0  ))>>16)&0x0000ffff;
            val[2]=(*(src_addr+(idx0+1))    )&0x0000ffff;
            val[3]=(*(src_addr+(idx0+1))>>16)&0x0000ffff;
            /* calculate pixel gain of each grid point, right shift GN_BIT bit to fix point */
            for(k=0;k<4;k++)
            {
                if(val[k]==0) val[k]=pxl_gain_max;
                else val[k]=((g_lsc_result.max_val[k]*ratio_poly)<<(GN_BIT-RATIO_POLY_BIT))/val[k];
                if(val[k]>pxl_gain_max) val[k]=pxl_gain_max;
            }

            *(src_addr+(idx0  ))=((val[1]<<16)&0xffff0000)|((val[0])&0x0000ffff);
            *(src_addr+(idx0+1))=((val[3]<<16)&0xffff0000)|((val[2])&0x0000ffff);
#if LSC_Debug_Table_Output
            fprintf(fin, "%5d, %5d, %5d, %5d",val[0], val[1], val[2], val[3]);
            fprintf(fin, "\n");
            //fprintf(fin, "0x%x ,",*(src_addr + idx0));
            //fprintf(fin, "0x%x ,",*(src_addr + idx0 +1));
            //fprintf(fin, "\n");
#endif
        }
    }
#if LSC_Debug_Table_Output
    fflush(fin);
    fclose(fin);

    char fileNameCoef[100];
    sprintf(fileNameCoef, "%s//%04d_%04d.Coeff.c", MEDIA_PATH, g_lsc_param.raw_wd, g_lsc_param.raw_ht);

    fin  = fopen(fileNameCoef, "wt");

    // 1. 214h  enable
    // 2. 218h  block number and block width/height
    // 3. 21ch  table address
    // 4. 220h  last block number
    // 5. 224h  0x20202020 : unit gain

    fprintf(fin, "218h = 0x%08x;", ((m-2)<<28 & 0xf0000000) | (g_lsc_param.block_wd<<16 & 0x0fff0000) |
            (((n-2)<<12) & 0x0000f000) | (g_lsc_param.block_ht & 0x00000fff));
    fprintf(fin, "220h = 0x%08x;\n", ((g_lsc_param.block_wd_last<<16) & 0x0fff0000) | (g_lsc_param.block_ht_last & 0x00000fff));
    fprintf(fin, "224h = 0x20202020;\n");

    fclose(fin);
#endif
    sync();
    sync();
    sync();

    //Convert to SVD matrix
    svd_tool(src_addr, n, m, (int)a_u2SVDTermNum, 8, tbl_cal_info,a_u1Mode);

    /* calculate HW table */
    //vlscHWTBLCAL(tbl_cal_info);

    free(g_src_tbl);
    free(g_dst_tbl);
    return mrRet;
}

/////////////////////////////////////////////////////////////////////////
//
//   mrBPC_Calibrate () -
//
/////////////////////////////////////////////////////////////////////////
MINT32 mrBPC_Calibrate(BPC_CAL_INI_PARAM_T cali_info)
{
    ACDK_LOGD(" mrBPC_Calibrate.\n");
    int				width, height, i, j, x, y, count, value, idx;
    unsigned short	row, col;
    int				Bright_Pixel_Number;
    int				Dark_Pixel_Number;
    int				overlap;
    float				Bright_Pixel_Threshold = 1.3;
    float				Dark_Pixel_Threshold = 0.7;

    //float				ratio;
    int				ratio;
    bool				minus;
    int                        cur_pixel_idx = 0;
    unsigned char		*input_buffer = NULL;
    unsigned char		**buf_mid = NULL;
    unsigned char		*mask = NULL;
#if  BPC_Debug_Table_Output
    FILE				*fp;
    timeval                  sStarttime;
    timeval                  sEndtime;
    long                       time_ms;
#endif

    unsigned char		m;
    Block_Info		sBlock_Info;
    unsigned char        average_length = 23;
    int				k, x_offset, y_offset;
    long                      block_acc_value;

    int                         block_index, channel_offset;

    int				 Bright_Pixel_Threshold_int;
    int                          Dark_Pixel_Threshold_int;

    FILE				*fbout = NULL;
    char 			fileNameBin[100];

    width = cali_info.u4ImgWidth;
    height = cali_info.u4ImgHeight;
    Bright_Pixel_Threshold = cali_info.fBrightPixelLevel;
    Dark_Pixel_Threshold = cali_info.fDarkPixelLevel;
    input_buffer = cali_info.raw_img_addr;
    mask = cali_info.mask_buffer;
    sBlock_Info.ACC = NULL;
    sBlock_Info.MAX_cnt = NULL;
    sBlock_Info.valid_cnt = NULL;


    Bright_Pixel_Threshold_int = (int)(Bright_Pixel_Threshold*1024);
    Dark_Pixel_Threshold_int = (int)(Dark_Pixel_Threshold*1024);


    buf_mid = (unsigned char **)matrix(buf_mid , 0,height-1,0,width-1);		// input array (index starts with 0)
    CAL_ASSERT(buf_mid ==  NULL, "mrBPC_Calibrate : malloc fail!");
    sBlock_Info.valid_cnt = (int **)matrix(sBlock_Info.valid_cnt, 0, 3, 0, width/(average_length*2));// 4 channel
    CAL_ASSERT(sBlock_Info.valid_cnt ==  NULL, "mrBPC_Calibrate : malloc fail!");
    sBlock_Info.MAX_cnt = (int **)matrix(sBlock_Info.MAX_cnt, 0, 3, 0, width/(average_length*2));// 4 channel
    CAL_ASSERT(sBlock_Info.MAX_cnt  ==  NULL, "mrBPC_Calibrate : malloc fail!");
    sBlock_Info.ACC = (long **)matrix(sBlock_Info.ACC, 0, 3, 0, width/(average_length*2));// 4 channel
    CAL_ASSERT(sBlock_Info.ACC ==  NULL, "mrBPC_Calibrate : malloc fail!");
    for(i = width/(average_length*2) ; i >= 0 ; i--)
    {
        for (j = 3 ; j >= 0 ; j--)
        {
            sBlock_Info.valid_cnt[j][i] = 0;
            sBlock_Info.MAX_cnt[j][i] = 0;
            sBlock_Info.ACC[j][i] = 0;
        }
    }


    if (cali_info.u4CalibrateMode == PREVIEW_TEST)
    {
        sprintf(fileNameBin, "%s//Preview_BPC.bin", MEDIA_PATH);
    }
    else
    {
        sprintf(fileNameBin, "%s//Capture_BPC.bin", MEDIA_PATH);
    }

    fbout  = fopen(fileNameBin, "wb");
    CAL_ASSERT(fbout ==  NULL, "mrBPC_Calibrate : open file fail");
    //if(fbout == NULL) { ACDK_LOGE(" AcdkCalibration::cannot open %s\n", fileNameBin); exit(2); }

#if  BPC_Debug_Table_Output
    if (cali_info.u4CalibrateMode == PREVIEW_TEST)
    {
        fp = fopen(Preview_BPC_Table_NAME, "wt");
    }
    else
    {
        fp = fopen(Capture_BPC_Table_NAME, "wt");
    }
#endif

    //chage to 2D array
    for(j = 0; j < height; j++)
    {
        for(i = 0; i < width; i++)
        {
            buf_mid[j][i]=input_buffer[j*width + i];
        }
    }


    Bright_Pixel_Number = 0;
    Dark_Pixel_Number = 0;
    overlap = 0;
    idx = 0;

#if  BPC_Debug_Table_Output
    gettimeofday(&sStarttime, NULL);
#endif

    // detect bright pixel and dark pixel defects
    for(row=0; row<height; row++)
    {
        for(col=0; col<width; col++)
        {
            cur_pixel_idx = width*row+col;
            count = 0;
            value = 0;

            block_index = col/(average_length<<1);

            //0.5 second at PC
            if ( (sBlock_Info.valid_cnt[0][block_index] == 0) && (sBlock_Info.valid_cnt[3][block_index] == 0)) //chech channel 0 & 3 is enough
            {
                for(k=0; k<=3 ; k++)
                {
                    count = 0;
                    block_acc_value = 0;
                    if (k == 0) {x_offset =0 ; y_offset = 0;}
                    else if (k == 1) {x_offset =1 ; y_offset = 0;}
                    else if (k == 2) {x_offset =0 ; y_offset = 1;}
                    else {x_offset =1 ; y_offset = 1;}
                    for(j=average_length-1; j>=0; j--)
                    {
                        y = row + (j<<1) + y_offset;
                        if ( y>=0 && y<height)
                        {
                            for(i=average_length-1; i>=0; i--)
                            {
                                x = col + (i<<1) + x_offset;
                                if(x>=0 && x<width)
                                {
                                    count++;
                                    block_acc_value += buf_mid[y][x];
                                }
                            }
                        }
                    }
                    sBlock_Info.valid_cnt[k][block_index]=count;
                    sBlock_Info.MAX_cnt[k][block_index]=count;
                    sBlock_Info.ACC[k][block_index]=block_acc_value;
                }
                //must be 1st channel data
                sBlock_Info.valid_cnt[0][block_index]--;
                count = sBlock_Info.MAX_cnt[0][block_index];
                block_acc_value = sBlock_Info.ACC[0][block_index];
            }
            else
            {
                x_offset = col&0x0001;// mod(col,2);
                y_offset = row&0x0001;// mod(row,2);
                channel_offset = x_offset + (y_offset<<1);

                sBlock_Info.valid_cnt[channel_offset][block_index]--;
                count = sBlock_Info.MAX_cnt[channel_offset][block_index];
                block_acc_value = sBlock_Info.ACC[channel_offset][block_index];
            }

            count -= 1;
            block_acc_value -= buf_mid[row][col];
            //block_acc_value /= count;
            ratio = (buf_mid[row][col] << 10) * count / block_acc_value;

            if(ratio > Bright_Pixel_Threshold_int)
            {
                if(mask[cur_pixel_idx] > 0)
                    overlap++;
                mask[cur_pixel_idx] = 4;
                Bright_Pixel_Number++;
            }
            else if(ratio < Dark_Pixel_Threshold_int)
            {
                if(mask[cur_pixel_idx] > 0)
                    overlap++;
                mask[cur_pixel_idx] = 6;
                Dark_Pixel_Number++;
            }

            if(mask[cur_pixel_idx] > 0)
            {
                idx++;
                fwrite(&col, 1, sizeof(unsigned short), fbout); //u min is signed int
                fwrite(&row, 1, sizeof(unsigned short), fbout); //u min is signed int
#if  BPC_Debug_Table_Output
                fprintf(fp, "idx, type, col, row = %d %d %d %d\n",idx, mask[cur_pixel_idx], col, row);
#endif
            }
        }
    }
    //output end symbol
    col = 0xFFFF;
    row = 0xFFFF;
    fwrite(&col, 1, sizeof(unsigned short), fbout); //u min is signed int
    fwrite(&row, 1, sizeof(unsigned short), fbout); //u min is signed int

#if  BPC_Debug_Table_Output
    gettimeofday(&sEndtime, NULL);
    time_ms = (sEndtime.tv_sec - sStarttime.tv_sec) * 1000000;
    if (sEndtime.tv_usec >= sStarttime.tv_usec) {
        time_ms += (sEndtime.tv_usec - sStarttime.tv_usec);
    }
    else {
        time_ms -= (sStarttime.tv_usec - sEndtime.tv_usec);
    }
    time_ms /= 1000;
    ACDK_LOGE("BPC Calculation Time is  = %ld ms\n", time_ms);

    // detect other pixel defects
    count = 0;

    for(row=0; row<height; row++)
    {
        for(col=0; col<width; col++)
        {
            cur_pixel_idx = width*row+col;
            if(mask[cur_pixel_idx] > 0)
                count++;
        }
    }

    ACDK_LOGE("Total Defect Count = %d\n", count);
    if (count*4 > CAL_Table_Size)
    {
        ACDK_LOGE("/******************************************/\n");
        ACDK_LOGE("/*  Total Defect Count larger then reserved PMEM */\n");
        ACDK_LOGE("/******************************************/\n");
    }
    ACDK_LOGE("Bright_Pixel_Number = %d\n", Bright_Pixel_Number);
    ACDK_LOGE("Dark_Pixel_Number = %d\n", Dark_Pixel_Number);
    ACDK_LOGE("Overlap_Number = %d\n", overlap);
    fclose(fp);
#endif
    free_matrix(buf_mid, 0, height-1, 0, width-1);
    fclose(fbout);

    return 0;
}
