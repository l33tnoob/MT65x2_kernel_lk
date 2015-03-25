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

#include <DpDataType.h>
#include <DpBlitStream.h>
#include <sys/time.h> 

#define DDP_TEST_LOOP_TIMEOUT   2000000  // 5 sec

DP_STATUS_ENUM DpFactoryModeSingleTest(void          **pInput,
                                       uint32_t      *pInSize,
                                       int32_t       inPlane,
                                       int32_t       inWidth,
                                       int32_t       inHeight,
                                       int32_t       inYPitch,
                                       int32_t       inUVPitch,
                                       DP_COLOR_ENUM inFormat,
                                       void          **pOutput,
                                       uint32_t      *pOutSize,
                                       int32_t       outPlane,
                                       int32_t       outWidth,
                                       int32_t       outHeight,
                                       int32_t       outYPitch,
                                       int32_t       outUVPitch,
                                       DP_COLOR_ENUM outFormat)
{
    DP_STATUS_ENUM status;
    DpBlitStream  stream;

    status = stream.setSrcBuffer(pInput,
                                 pInSize,
                                 inPlane);
    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        return status;
    }

    status = stream.setSrcConfig(inWidth,
                                 inHeight,
                                 inYPitch,
                                 inUVPitch,
                                 inFormat);
    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        return status;
    }

    status = stream.setDstBuffer(pOutput,
                                 pOutSize,
                                 outPlane);
    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        return status;
    }

    status = stream.setDstConfig(outWidth,
                                 outHeight,
                                 outYPitch,
                                 outUVPitch,
                                 outFormat);
    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        return status;
    }

    return stream.invalidate();
}


int32_t DpFactoryModeTest(void *pArg)
{
    DP_STATUS_ENUM  status;
    struct timeval  value1;
    struct timeval  value2;
    uint32_t        time1;
    uint32_t        time2;
    int32_t         index;
    void            *pInBuf[3];
    uint32_t        inSize[3];
    void            *pOutBuf[3];
    uint32_t        outSize[3];

    status =  DP_STATUS_RETURN_SUCCESS;

    pInBuf[0] = malloc(640 * 480 * 3);
    pInBuf[1] = 0;
    pInBuf[2] = 0;

    inSize[0] = 640 * 480 * 3;
    inSize[1] = 0;
    inSize[2] = 0;

    pOutBuf[0] = malloc(640 * 480 * 3);
    pOutBuf[1] = 0;
    pOutBuf[2] = 0;

    outSize[0] = 640 * 480 * 3;
    outSize[1] = 0;
    outSize[2] = 0;

    index = 0;

    gettimeofday(&value1, NULL);
    time1 = (value1.tv_sec * 1000000 + value1.tv_usec);

    while(1)
    {
        printf("[DDP] DDP Test Loop (%d).\n", index);

        status = DpFactoryModeSingleTest(pInBuf,
                                         inSize,
                                         1,
                                         640,
                                         480,
                                         640 * 3,
                                         0,
                                         DP_COLOR_RGB888,
                                         pOutBuf,
                                         outSize,
                                         1,
                                         640,
                                         480,
                                         640 * 3,
                                         0,
                                         DP_COLOR_RGB888);
        if(DP_STATUS_RETURN_SUCCESS != status)
        {
            printf("[DDP ERROR] DDP Test Loop (%d) failed!\n", index);
            break;
        }

        gettimeofday(&value2, NULL);
        time2 = (value2.tv_sec * 1000000 + value2.tv_usec);
        if(time2 > (time1 + DDP_TEST_LOOP_TIMEOUT))
        {
            printf("[DDP] DDP Test Loop time up (%dus)!\n", DDP_TEST_LOOP_TIMEOUT);
            break;
        }

        index++;
    }

    if (0 != pInBuf[0])
    {
        free(pInBuf[0]);
        pInBuf[0] = 0;
    }

    if (0 != pInBuf[1])
    {
        free(pInBuf[1]);
        pInBuf[1] = 0;
    }

    if (0 != pInBuf[2])
    {
        free(pInBuf[2]);
        pInBuf[2] = 0;
    }

    if (0 != pOutBuf[0])
    {
        free(pOutBuf[0]);
        pOutBuf[0] = 0;
    }

    if (0 != pOutBuf[1])
    {
        free(pOutBuf[1]);
        pOutBuf[1] = 0;
    }

    if (0 != pOutBuf[2])
    {
        free(pOutBuf[2]);
        pOutBuf[2] = 0;
    }

    if(DP_STATUS_RETURN_SUCCESS != status)
    {
        printf("[DDP] Test Failed (%d)!\n", status);
        return -1;
    }

    return 0;
}
