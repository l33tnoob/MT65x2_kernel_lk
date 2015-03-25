/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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

#include <stdio.h>
#include <cutils/log.h>
#include <cutils/pmem.h>
#include <cutils/ashmem.h>
#include <cutils/memutil.h>
#include <unistd.h>
#include <sys/mman.h>
#include "MediaHal.h"
#include <cutils/pmem.h>
#include <cutils/memutil.h>
#include "MediaHal.h"
//#include "MT6575MDPDrv.h"
//#include "scenario_imagetransform.h"
#include "m4u_lib.h"
/********************************************************************************/
#define xlog(...) \
        do { \
            LOGD(__VA_ARGS__); \
        } while (0)

#undef LOG_TAG
#define LOG_TAG "[m4u_it]"

//Try bitblt hal
extern unsigned char src_file[];

//#define _USE_PMEM_
//#define _USE_MALLOC_
#define _USE_NEW_
// #define _USE_ASHMEM_

#define SRC_W 80
#define SRC_H 80
#define SRC_BPP 3

#define DST_W 256
#define DST_H 256 
#define DST_BPP 2

int main()
{
    mHalBltParam_t stParam;
    unsigned int i,j;
    unsigned int dst_size;
    unsigned char *src_va;
    unsigned char *dst_va;
    unsigned char *temp_va;
    int src_ID;
    int dst_ID;
    unsigned long src_pa;
    unsigned long dst_pa;
    char* pFile;
    pFile = new char[30];
    memset(pFile, 0, 30);

    struct timeval time_s;
    struct timeval time_e;
    unsigned int time_all=0, time_cur=0;
    unsigned int cnt=0;
    
    unsigned char pSrc[SRC_W*SRC_H*SRC_BPP];

#ifdef _USE_PMEM_
    xlog("======= M4U test _USE_PMEM_ ======== \n");
    src_va = (unsigned char *) pmem_alloc(SRC_W*SRC_H*SRC_BPP , &src_ID);
    src_pa = (unsigned long)pmem_get_phys(src_ID);
    dst_va = (unsigned char *) pmem_alloc(DST_W*DST_H*DST_BPP , &dst_ID);
    dst_pa = (unsigned long)pmem_get_phys(dst_ID);
#elif defined _USE_MALLOC_
    xlog("======= M4U test _USE_MALLOC_ ======== \n");
    src_va = (unsigned char *)malloc(SRC_W*SRC_H*SRC_BPP);
    dst_va = (unsigned char *)malloc(DST_W*DST_H*DST_BPP);
#elif defined  _USE_NEW_
    xlog("======= M4U test _USE_NEW_ ======== \n");
    src_va = new unsigned char[SRC_W*SRC_H*SRC_BPP];
    dst_va = new unsigned char[DST_W*DST_H*DST_BPP];
#elif defined _USE_ASHMEM_
    xlog("======= M4U test _USE_ASHMEM_ ======== \n");
    int fd = ashmem_create_region("MDPBuf", SRC_W*SRC_H*SRC_BPP);
    if(-1==fd)
    {
    	xlog("ashmem_create_region failed! \n");
    	return 0;
    }
    else
    {
       xlog("ashmem_create_region success! \n");
    }    
    int err = ashmem_set_prot_region(fd, PROT_READ | PROT_WRITE);
    if (err) {
        xlog("------ ashmem_set_prot_region failed! \n");
        return false;
    }    
    src_va = (unsigned char*)mmap(NULL, SRC_W*SRC_H*SRC_BPP, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    if (-1 == (long)src_va) {
        xlog("---------- mmap failed! \n");
        return false;
    }    

    int fd_dst = ashmem_create_region("MDPBuf2", DST_W*DST_H*DST_BPP*6);
    if(-1==fd_dst)
    {
    	xlog("ashmem_create_region failed! \n");
    	return 0;
    }    
    err = ashmem_set_prot_region(fd_dst, PROT_READ | PROT_WRITE);
    if (err) {
        xlog("------ ashmem_set_prot_region failed! \n");
        return false;
    }    
    dst_va = (unsigned char*)mmap(NULL, DST_W*DST_H*DST_BPP*6, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd_dst, 0);
    if (-1 == (long)dst_va) {
        xlog("---------- mmap failed! \n");
        return false;
    }  
#else
    xlog("======= M4U test STACK ======== \n");
    src_va = pSrc;
    dst_va = new unsigned char[DST_W*DST_H*DST_BPP];
    xlog("m4u use stack \n");
#endif

    if(src_va==NULL || dst_va==NULL )
    {
        xlog("Can not allocate memory\n");
        return -1;
    }

    memset(dst_va , 255 , DST_W*DST_H*DST_BPP);

    FILE *fp;
    unsigned long index;
    fp = fopen("/data/data_rgb888_80x80.bin", "r");
    fread(src_va , 1 , SRC_W*SRC_H*SRC_BPP , fp);
    fclose(fp);

for(j=0;j<4;j++)
for(i=0;i<4;i++)
{
    stParam.srcX = 0;
    stParam.srcY = 0;
    stParam.srcW = SRC_W;
    stParam.srcWStride = SRC_W;
    stParam.srcH = SRC_H;
    stParam.srcHStride = SRC_H;
    stParam.srcAddr = (MUINT32)src_va;
    stParam.srcFormat = MHAL_FORMAT_RGB_888;
    stParam.dstW = DST_W;
    stParam.dstH = DST_H;
    stParam.dstAddr = (MUINT32)dst_va;
    stParam.dstFormat = MHAL_FORMAT_RGB_565;
    stParam.pitch = DST_W;
    stParam.orientation = i;

    gettimeofday(&time_s, NULL);


    if (MHAL_NO_ERROR != mHalIoCtrl(MHAL_IOCTL_BITBLT, &stParam, sizeof(stParam), NULL, 0, NULL)) 
    {
        xlog("Test failed!! rgb %d", 4*j+i);
    }
    else
    {
        xlog("Test success rgb!! %d", 4*j+i);
    }
    gettimeofday(&time_e, NULL);
    time_cur = (time_e.tv_sec - time_s.tv_sec)*1000 + (time_e.tv_usec - time_s.tv_usec)/1000;
    time_all += time_cur;
    cnt++;
    xlog("rgb_m4u: time_cur= %ums, time_all= %ums, time_avg= %ums", 
        time_cur, time_all, (time_all/cnt));

//Save
    sprintf(pFile, "/data/result_rgb_%d.bin", 4*j+i);
    fp = fopen(pFile, "w");
    for(index = 0 ; index < DST_W*DST_H*DST_BPP ; index++)
    {
        fprintf(fp, "%c", dst_va[index]);
    }
    fclose(fp);
   
}

#ifdef _USE_PMEM_
    pmem_free(src_va , SRC_W*SRC_H*SRC_BPP , src_ID);
    pmem_free(dst_va , DST_W*DST_H*DST_BPP , dst_ID);
#elif defined _USE_MALLOC_
    free(src_va);
    free(dst_va);
#elif defined _USE_NEW_
      delete []src_va;
      delete []dst_va;
#elif defined _USE_ASHMEM_

#else //use stack
    delete []dst_va;
#endif


#ifdef _USE_PMEM_
    xlog("++++++- M4U test _USE_PMEM_ ++++++-- \n");
#elif defined _USE_MALLOC_
    xlog("++++++- M4U test _USE_MALLOC_ ++++++-- \n");
#elif defined  _USE_NEW_
    xlog("++++++- M4U test _USE_NEW_ ++++++-- \n");
#elif defined _USE_ASHMEM_
    xlog("++++++- M4U test _USE_ASHMEM_ ++++++-- \n");
#else
    xlog("++++++- M4U test STACK ++++++-- \n");
#endif
    
    return 0;
}

