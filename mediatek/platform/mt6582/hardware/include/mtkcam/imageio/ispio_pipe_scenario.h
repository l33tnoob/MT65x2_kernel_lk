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
* @file ispio_pipe_scenario.h
*
* ispio_pipe_scenario Header File
*/

#ifndef _ISPIO_PIPE_SCENARIO_H_
#define _ISPIO_PIPE_SCENARIO_H_

namespace NSImageio 
{
    namespace NSIspio
    {        
        /**
             * @brief Scenario ID
             */
        enum EScenarioID
        {
            //
            eScenarioID_IC,         // 0    //  Image Capture
            //
            eScenarioID_VR,                 //  Video Recording/Preview
            //
            eScenarioID_ZSD,                //  Zero Shutter Delay
            //
            eScenarioID_IP,                 //  Image Playback
            //
            eScenarioID_VEC,                //  Vector Generation
            //
            eScenarioID_RESERVED,   // 5    //  Reserved
            //
            eScenarioID_N3D_IC,             //  Native Stereo Camera IC
            //
            eScenarioID_N3D_VR,             //  Native Stereo Camera VR
            //
            eScenarioID_VSS,                //  video snap shot, derived from N3D_IC
            //
            eScenarioID_ZSD_CDP_CC,         //  ZSD CDP concurrency
            //
            eScenarioID_VSS_CDP_CC, // 10   //  VSS CDP concurrency
            //
            eScenarioID_GDMA,               //  VSS CDP concurrency
            //
            eScenarioID_CONFIG_FMT          //  FMT
        };        
        
        /**
             * @brief Scenario Format
             */
        enum EScenarioFmt
        {            
            eScenarioFmt_RAW,            
            eScenarioFmt_YUV,            
            eScenarioFmt_RGB,            
            eScenarioFmt_JPG,            
            eScenarioFmt_MFB,            
            eScenarioFmt_RGB_LOAD,            
            eScenarioFmt_UNKNOWN = 0xFFFFFFFF
        };
    };  //namespace NSIspio
};  //namespace NSImageio
#endif  //  _ISPIO_PIPE_SCENARIO_H_

