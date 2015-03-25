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
* @file ICamIOPipe.h
*
* CamIOPipe Header File
*/

#ifndef _ISPIO_I_CAMIO_PIPE_H_
#define _ISPIO_I_CAMIO_PIPE_H_

namespace NSImageio 
{
    namespace NSIspio 
    {
        /**
             * @brief  CamIOPipe class
             */
        class ICamIOPipe : public IPipe
        {
        public:
            static EPipeID const ePipeID = ePipeID_1x1_Sensor_Tg_Mem;

        public:

            /**
                   *@brief Create CamIOPipe object
                   *
                   *@param[in] eScenarioID : HW scenario
                   *@param[in] eScenarioFmt : sensor type
                   *
                   *@return
                   *-Pointer to CamIOPipe object, otherwise indicates fail
                   */
            static ICamIOPipe* createInstance(EScenarioID const eScenarioID, EScenarioFmt const eScenarioFmt);        
        public:

            /**
                   * @brief  Interface of Command Class
                   */
            class ICmd : public IPipeCommand
            {
            public:

                /**
                          *@brief Constructor
                          *
                          *@param[in] pIPipe : IPipe object pointer
                          */
                ICmd(IPipe*const pIPipe);

                /**
                          *@brief verifySelf
                          */
                virtual MBOOL   verifySelf();

            protected:
                MVOID*const     mpIPipe;
            };

            /**
                   * @brief  Set 2 parameters
                   */
            class Cmd_Set2Params : public ICmd
            {
            public:
                
                /**
                          *@brief Constructor
                          *
                          *@param[in] pIPipe : IPipe object pointer
                          *@param[in] u4Param1 : command 1
                          *@param[in] u4Param2 : command 2
                          */
                Cmd_Set2Params(IPipe*const pIPipe, MUINT32 const u4Param1, MUINT32 const u4Param2);

                /**
                          *@brief execute
                          */
                virtual MBOOL   execute();
                
            protected:
                MUINT32         mu4Param1;
                MUINT32         mu4Param2;
            };

            /**
                   * @brief  Get 1 parameter based on 1 input parameter
                   */
            class Cmd_Get1ParamBasedOn1Input : public ICmd
            {
            public:

                /**
                          *@brief Constructor
                          *
                          *@param[in] pIPipe : IPipe object pointer
                          *@param[in] u4InParam : input parameter
                          *@param[in] pu4OutParam : output parameter
                          */
                Cmd_Get1ParamBasedOn1Input(IPipe*const pIPipe, MUINT32 const u4InParam, MUINT32*const pu4OutParam);

                /**
                          *@brief execute
                          */
                virtual MBOOL   execute();
                
            protected:
                MUINT32         mu4InParam;
                MUINT32*        mpu4OutParam;
            };

            /**
                   * @brief  Trigger command queue 0
                   */
            virtual MBOOL   startCQ0() = 0;

            /**
                   * @brief  Trigger command queue 0B
                   */
            virtual MBOOL   startCQ0B() = 0;    
            
            //virtual MBOOL   queryRawDMAOutInfo(MUINT32 const imgFmt, MUINT32& u4ImgWidth, MUINT32& u4Stride, MUINT32& pixel_byte);
        };
    };  //namespace NSIspio
};  //namespace NSImageio
#endif  //  _ISPIO_I_CAMIO_PIPE_H_

