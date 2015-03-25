/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/

/**
* @file ispio_pipe_identity.h
*
* ispio_pipe_identity Header File
*/

#ifndef _ISPIO_PIPE_IDENTITY_H_
#define _ISPIO_PIPE_IDENTITY_H_

namespace NSImageio 
{
    namespace NSIspio
    {
        /**
             * @brief  Pipe ID
             */
        enum EPipeID
        {
            //
            ePipeID_1x1_Sensor_Tg_Mem,          //  (In:1/Out:1) SENSOR -> TG -> MEM
            //
            ePipeID_1x2_Sensor_Tg_Mem,          //  (In:1/Out:2) SENSOR -> TG -> MEM
            //
            ePipeID_2x2_Sensor_Tg_Mem,          //  (In:2/Out:2) SENSOR -> TG -> MEM
                                                   //                      -> ISP-> MEM
            //
            ePipeID_1x1_Sensor_Tg_Isp_Mem,      //  (In:1/Out:1) SENSOR -> TG -> ISP -> MEM
            //
            ePipeID_1x3_Mem_Cdp_Mem,            //  (In:1/Out:3) MEM -> TG -> ISP -> MEM
            //
            ePipeID_1x3_Gdma_Cdp_Mem,           //  (In:1/Out:3) GDMA -> CDP -> MEM
            //
            ePipeID_1x3_Mem_Isp_Cdp_Mem,        //  (In:1/Out:3) MEM -> ISP -> CDP -> MEM
            //
            ePipeID_1x3_Gdma_Isp_Cdp_Mem,       //  (In:1/Out:3) GDMA -> ISP -> CDP -> MEM
            //
            ePipeID_1x1_Mem_Vec_Mem,            //  (In:1/Out:1) MEM -> Vec -> MEM
            //
            ePipeID_1x1_Mem_Lsc_PreGain_Mem,    //  (In:1/Out:1) MEM -> Lsc -> PreGain -> MEM (img2o)
            //
            ePipeID_Total_Count                 //  total count of pipes.
        };
    };  //namespace NSIspio
};  //namespace NSImageio
#endif  //  _ISPIO_PIPE_IDENTITY_H_

