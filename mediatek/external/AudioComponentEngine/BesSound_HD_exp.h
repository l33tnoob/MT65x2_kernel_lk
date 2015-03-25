/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2005
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/

/*****************************************************************************
 *
 * Filename:
 * ---------
 * BesSound_HD_exp.h
 *
 * Project:
 * --------
 * SWIP
 *
 * Description:
 * ------------
 * BesSound_HD Common header file.
 *
 * Author:
 * -------
 * Scholar Chang
 *
 *============================================================================
 *             HISTORY
 * Below this line, this part is controlled by PVCS VM. DO NOT MODIFY!!
 *------------------------------------------------------------------------------
 * $Revision$
 * $Modtime$
 * $Log$
 *
 *------------------------------------------------------------------------------
 * Upper this line, this part is controlled by PVCS VM. DO NOT MODIFY!!
 *============================================================================
 ****************************************************************************/

#ifndef __BESSOUND_HD_EXP_H__
#define __BESSOUND_HD_EXP_H__

// Notice: All the buffer pointers are required to be 
//         four-byte-aligned to avoid the potential on-target process error !!!

typedef struct BS_HD_EngineInfoStruct BS_HD_EngineInfo;
struct BS_HD_EngineInfoStruct{
    int Version;
    // The number of samples that applications have to feed to 
    // engines for flushing out the audio samples buffered in engines,
    // or to flush out the tail.
    // L/R pair counts as 2 samples.
    int FlushOutSampleCount;
};

typedef struct BS_HD_HandleStruct BS_HD_Handle;

struct BS_HD_HandleStruct{
    /*
        Get the required buffer sizes of this BesSound engine
        Return value                    < 0  : Error 
                                        >= 0 : Normal
        p_internal_buf_size_in_byte [O] Required internal buffer size in bytes
        p_temp_buf_size_in_byte     [O] Required temp buffer size in bytes
        PCM_Format                  [I] Input / output buffer 16 / 32-bit PCM format
    */
    int (*GetBufferSize)(unsigned int *p_internal_buf_size_in_byte,
                         unsigned int *p_temp_buf_size_in_byte,
                         unsigned int PCM_Format);
    
    /*
        Initialize this BesSound engine
        Return value                < 0  : Error 
                                    >= 0 : Normal
        p_handle                [I] Handle of the current BesSound engine
        p_internal_buf          [I] Pointer to the internal buffer
        p_init_param            [I] Pointer to the initial parameters
    */
    int (*Open)( BS_HD_Handle *p_handle,
                 char *p_internal_buf, 
                 const void *p_init_param);
    
    /*
        Set the runtime parameters of this BesSound engine
        Return value                   < 0  : Error 
                                       >= 0 : Normal
        p_handle                   [I] Handle of the current BesSound engine
        p_runtime_param            [I] Pointer to the runtime parameters
    */
    int (*SetParameters)(   BS_HD_Handle *p_handle,
                            const void *p_runtime_param);
    
    /*
        Get the runtime status of this BesSound engine
        Return value                   < 0  : Error 
                                       >= 0 : Normal
        p_handle                   [I] Handle of the current BesSound engine
        p_runtime_param            [O] Pointer to the runtime status
    */
    int (*GetStatus)(   BS_HD_Handle *p_handle,
                        void *p_runtime_status);
    
    /*
        Get the build-time information about this BesSound engine
        Return value      < 0  : Error 
                          >= 0 : Normal
        p_engine_info [O] Pointer to the structure of the engine information
    */
    int (*GetEngineInfo)(  BS_HD_EngineInfo *p_engine_info);

    /*
        Process 16-bit / 32-bit data from input buffer to output buffer
        Return value      < 0  : Error 
                          >= 0 : Normal
        p_handle      [I] Handle of current BesSound engine
        p_temp_buf    [I] Pointer to the temp buffer used in processing the data
        p_in_buf      [I] Pointer to the input PCM buffer
                          For stereo input, the layout of LR is L/R/L/R...
        p_in_byte_cnt [I] Valid input buffer size in bytes
                      [O] Consumed input buffer size in bytes
        p_ou_buf      [I] Pointer to the output PCM buffer
                          For stereo output, the layout of LR is L/R/L/R...
        p_ou_byte_cnt [I] Available output buffer size in bytes
                      [O] Produced output buffer size in bytes
    */
    int (*Process)( BS_HD_Handle *p_handle,
                    char *p_temp_buf,
                    void *p_in_buf,
                    int  *p_in_byte_cnt,
                    void *p_ou_buf,
                    int  *p_ou_byte_cnt);
    
    void *p_internal_handle;
};

#endif // __BESSOUND_HD_EXP_H__
