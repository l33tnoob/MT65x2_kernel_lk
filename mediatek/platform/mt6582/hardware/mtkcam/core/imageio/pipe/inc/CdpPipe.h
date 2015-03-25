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
* @file CdpPipe.h
*
* CdpPipe Header File
*/

#ifndef _ISPIO_CDP_PIPE_H_
#define _ISPIO_CDP_PIPE_H_

#include <vector>

using namespace std;

#include <mtkcam/imageio/ispio_pipe_ports.h>
#include <mtkcam/imageio/ispio_pipe_buffer.h>

#include "cam_path.h"
#include <mtkcam/drv/res_mgr_drv.h>


namespace NSImageio 
{
    namespace NSIspio
    {

        //TODO:remove later
        int scenario_pmem_alloc_sync(unsigned int size,int *memId,unsigned char **vAddr,unsigned int *pAddr);
        int scenario_pmem_free(unsigned char *vAddr,unsigned int size,int memId);

        int scenario_pmem_alloc_sync_2(unsigned int size,int *memId,unsigned char **vAddr,unsigned int *pAddr);
        int scenario_pmem_free_2(unsigned char *vAddr,unsigned int size,int memId);

        //Tpipe Driver
#define CDP_MAX_TDRI_HEX_SIZE  (ISP_MAX_TDRI_HEX_SIZE)

        class PipeImp;

        /**
             * @brief  CdpPipe class
             */
        class CdpPipe : public PipeImp
        {
        public:

            /**
                   * @brief  Constructor
                   *
                   *@param[in] szPipeName : pipe name
                   *@param[in] ePipeID : pipe ID
                   *@param[in] eScenarioID : HW scenario
                   *@param[in] eScenarioFmt : sensor format
                   */
            CdpPipe( char const*const szPipeName,
                        EPipeID const ePipeID,
                        EScenarioID const eScenarioID,
                        EScenarioFmt const eScenarioFmt);

            /**
                   * @brief  Destructor
                   */
            virtual ~CdpPipe();

        public:

            /**
                   * @brief  Initialize
                   */
            virtual MBOOL init();

            /**
                   * @brief  Uninitialize
                   */
            virtual MBOOL uninit();

        public:

            /**
                   * @brief  Trigger CdpPipe
                   */
            virtual MBOOL start();

            /**
                   * @brief  Sync jpeg ring buffer pass2C
                   */
            virtual MBOOL syncJpegPass2C();

            /**
                   * @brief  Trigger pass2 FMT
                   */
            virtual MBOOL startFmt();

            /**
                   * @brief  Stop CdpPipe
                   */
            virtual MBOOL stop();

        public:

            /**
                   *@brief Add a buffer into the specified non-sensor input port
                   *
                   *@param[in] portID : The input port ID to add. The port ID must be one of which specified when configPipe()
                   *@param[in] rQBufInfo : Reference to a QBufInfo structure containing a buffer information to add
                   *
                   *@return
                   *-MTRUE indicates success, otherwise indicates failure
                   */
            virtual MBOOL enqueInBuf(PortID const portID, QBufInfo const& rQBufInfo);

            /**
                   *@brief De-queue a buffer information for a specified input port ID
                   *
                   *@param[in] PortID : The output port ID to dequeue
                   *@param[in] rQBufInfo : Reference to a QTimeStampBufInfo structure to store the de-queued buffer information
                   *@param[in] u4TimeoutMs : Timeout in milliseconds. If u4TimeoutMS > 0, a timeout is specified, and this call will be blocked until any buffer is ready. 
                   *                                     If u4TimeoutMS = 0, this call must return immediately no matter whether any buffer is ready or not.
                   *
                   *@return
                   *-MTRUE indicates success, otherwise indicates failure
                   */
            virtual MBOOL dequeInBuf(PortID const portID, QTimeStampBufInfo& rQBufInfo, MUINT32 const u4TimeoutMs = 0xFFFFFFFF);

            /**
                   *@brief En-queue a buffer into the specified output port
                   *
                   *@param[in] PortID : The output port ID to enqueue. The port ID must be one of which specified when configPipe()
                   *@param[in] rQBufInfo : Reference to a QBufInfo structure containing a buffer information to enqueue
                   *
                   *@return
                   *-MTRUE indicates success, otherwise indicates failure
                   */
            virtual MBOOL enqueOutBuf(PortID const portID, QBufInfo const& rQBufInfo);

            /**
                   *@brief De-queue a buffer information for a specified output port ID
                   *
                   *@param[in] PortID : The output port ID to dequeue
                   *@param[in] rQBufInfo : Reference to a QTimeStampBufInfo structure to store the de-queued buffer information
                   *@param[in] u4TimeoutMs : Timeout in milliseconds If u4TimeoutMS > 0, a timeout is specified, and this call will be blocked until any buffer is ready.
                   *                                     If u4TimeoutMS = 0, this call must return immediately no matter whether any buffer is ready or not.
                   *
                   *@return
                   *-MTRUE indicates success, otherwise indicates failure
                   */
            virtual MBOOL dequeOutBuf(PortID const portID, QTimeStampBufInfo& rQBufInfo, MUINT32 const u4TimeoutMs = 0xFFFFFFFF);

        public:

            /**
                   *@brief Configure in/out ports of this pipe
                   *
                   *@param[in] vInPorts : Reference to the vector of input ports
                   *@param[in] vOutPorts : Reference to the vector of output ports
                   *
                   *@return
                   *-MTRUE indicates success, otherwise indicates failure
                   */
            virtual MBOOL configPipe(vector<PortInfo const*>const& vInPorts, vector<PortInfo const*>const& vOutPorts);

            /**
                   *@brief Partial update configure in/out ports of this pipe
                   *
                   *@param[in] vInPorts : Reference to the vector of input ports
                   *@param[in] vOutPorts : Reference to the vector of output ports
                   *
                   *@return
                   *-MTRUE indicates success, otherwise indicates failure
                   */
            virtual MBOOL configPipeUpdate(vector<PortInfo const*>const& vInPorts, vector<PortInfo const*>const& vOutPorts);
        public:

            /**
                   *@brief Set 2 parameters
                   *                   
                   *@param[in] u4Param1 : command 1
                   *@param[in] u4Param2 : command 2
                   */
            virtual MBOOL onSet2Params(MUINT32 const u4Param1, MUINT32 const u4Param2);

            /**
                   *@brief Get 1 parameter based on 1 input parameter
                   *                   
                   *@param[in] u4InParam : input parameter
                   *@param[in] pu4OutParam : output parameter
                   */
            virtual MBOOL onGet1ParamBasedOn1Input(MUINT32 const u4InParam, MUINT32*const pu4OutParam);

        public:

            /**
                   *@brief Interrupt handling
                   *
                   *@param[in] pass : Specific pass
                   *@param[in] irq_int : IRQ type
                   *
                   *@return
                   *-MTRUE indicates success, otherwise indicates failure
                   */
        	virtual MBOOL irq(EPipePass pass, EPipeIRQ irq_int);

        public:

            /**
                   *@brief Original style sendCommand method
                   *
                   *@param[in] cmd : command
                   *@param[in] arg1 : argument 1
                   *@param[in] arg2 : argument 2
                   *@param[in] arg3 : argument 3
                   *
                   *@return
                   *-MTRUE indicates success, otherwise indicates failure
                   */
            virtual MBOOL sendCommand(MINT32 cmd, MINT32 arg1, MINT32 arg2, MINT32 arg3);

        //
        private:
            IspDrvShell*             m_pIspDrvShell;
            ResMgrDrv*               m_resMgr;
            CamPathPass2             m_CamPathPass2;
            CamPathPass2Parameter    m_camPass2Param;
            vector<BufInfo>          m_vBufImgi;
            vector<BufInfo>          m_vBufVipi;
            vector<BufInfo>          m_vBufVip2i;
            vector<BufInfo>          m_vBufDispo;
            vector<BufInfo>          m_vBufVido;
            //
            MUINT32 tdriSize, tdriPhy, tdriRingSize, tdriRingPhy;
            MUINT8* pTdriVir;
            MUINT8* pTdriRingVir;
            MINT32  tdriMemId, tdriRingMemId;
            MUINT32 tpipe_config_size;
            MUINT32 *pTpipeConfigVa;
            MUINT32 segmSimpleConfIdxNum;
            MINT32  tpipe_config_memId;
            MUINT32 cq1_size,cq1_phy;
            MUINT8* cq1_vir;
            MINT32  cq1_memId;
            MUINT32 cq2_size,cq2_phy;
            MUINT8* cq2_vir;
            MINT32  cq2_memId;
            MUINT32 cq3_size,cq3_phy;
            MUINT8* cq3_vir;
            MINT32  cq3_memId;
            //
            EPipePass   m_pipePass;
            //
            mutable Mutex       mLock;
            //
            MINT32  m_pass2_CQ;
            EConfigSettingStage m_settingStage;
            MBOOL   m_isPartialUpdate;
            //
            MBOOL   m_isImgPlaneByImgi;
            //
            RES_MGR_DRV_MODE_STRUCT resMgrMode;
        };
    };  //namespace NSIspio
};  //namespace NSImageio
#endif  //  _ISPIO_CDP_PIPE_H_

