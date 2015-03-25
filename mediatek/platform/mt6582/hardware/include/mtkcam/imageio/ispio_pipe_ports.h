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
* @file ispio_pipe_ports.h
*
* ispio_pipe_ports Header File
*/

#ifndef _ISPIO_PIPE_PORTS_H_
#define _ISPIO_PIPE_PORTS_H_

#include <mtkcam/imageio/ispio_stddef.h>

namespace NSImageio
{
    namespace NSIspio
    {        
        /**
             * @brief Pipe Port Type
             */
        enum EPortType
        {
            EPortType_Memory,
            EPortType_Sensor,
            EPortType_GDMA,
            EPortType_VRZ_RDMA,
            EPortType_VID_RDMA,
            EPortType_DISP_RDMA,
        };

        /**
             * @brief Pipe Port index
             */
        enum EPortIndex
        {
            EPortIndex_TG1I,    // 0
            EPortIndex_TG2I,
            EPortIndex_IMGI,
            EPortIndex_IMGCI,
            EPortIndex_NR3I,
            EPortIndex_FLKI,    // 5
            EPortIndex_LSCI,
            EPortIndex_LCEI,
            EPortIndex_VIPI,
            EPortIndex_VIP2I,
            EPortIndex_CQI,     // 10
            EPortIndex_TDRI,
            EPortIndex_IMGO,
            EPortIndex_IMG2O,
            EPortIndex_LCSO,
            EPortIndex_AAO,     // 15
            EPortIndex_NR3O,
            EPortIndex_ESFKO,
            EPortIndex_AFO,
            EPortIndex_EISO,
            EPortIndex_DISPO,   // 20
            EPortIndex_VIDO,
            EPortIndex_FDO
        };

        /**
             * @brief Pipe Sensor index
             */
        enum ESensorIndex
        {
            ESensorIndex_NONE   = 0x00,
            ESensorIndex_MAIN   = 0x01,
            ESensorIndex_SUB    = 0x02,
            ESensorIndex_ATV    = 0x04,
            ESensorIndex_MAIN_2 = 0x08,
            ESensorIndex_MAIN_3D= 0x09,
        };

        /**
             * @brief Pipe pass index
             */
        enum EPipePass
        {
            EPipePass_PASS2 = 0,
            EPipePass_PASS2B,
            EPipePass_PASS2C,
            EPipePass_PASS1_TG1,
            EPipePass_PASS1_TG2,
            EPipePass_PASS2_Phy
        };        
        
        /**
             * @brief Pipe Port direction
             */
        enum EPortDirection
        {
            EPortDirection_In,
            EPortDirection_Out,
        };

        /**
             * @brief Pipe Port ID (Descriptor)
             */
        struct PortID
        {
        public:
            MUINT32     type    :   8;      ///<  EPortType
            MUINT32     index   :   8;      ///<  port index
            MUINT32     inout   :   1;      ///<  0:in/1:out
            MUINT32     pipePass;
            
        public:

            /**
                   * @brief Constructor
                   */
            PortID(
                EPortType const _eType     = EPortType_Memory,
                MUINT32 const _index       = 0,
                MUINT32 const _inout       = 0,
                MUINT32 const _pipePass= EPipePass_PASS2)
            {
                type        = _eType;
                index       = _index;
                inout       = _inout;
                pipePass    = _pipePass;
            }
            
        public:

            /**
                   * @brief Operations
                   */
            MUINT32 operator()() const
            {
                return  *reinterpret_cast<MUINT32 const*>(this);
            }
        };

        /**
             * @brief Pipe Port Info
             */
        struct PortInfo : public ImgInfo, public PortID, public BufInfo, public RingInfo, public SegmentInfo
        {
        public:

            /**
                   * @brief Constructor
                   */
            PortInfo()
                : ImgInfo()
                , PortID()
            {
            }

            /**
                   * @brief Constructor
                   */
            PortInfo(ImgInfo const& _ImgInfo)
                : ImgInfo(_ImgInfo)
                , PortID()
            {
            }

            /**
                   * @brief Constructor
                   */
            PortInfo(PortID const& _PortID)
                : ImgInfo()
                , PortID(_PortID)
            {
            }

            /**
                   * @brief Constructor
                   */
            PortInfo(ImgInfo const& _ImgInfo, PortID const& _PortID)
                : ImgInfo(_ImgInfo)
                , PortID(_PortID)
            {
            }

            /**
                   * @brief Constructor
                   */
            PortInfo(ImgInfo const& _ImgInfo, PortID const& _PortID, BufInfo const& _BufInfo)
                : ImgInfo(_ImgInfo)
                , PortID(_PortID)
                ,BufInfo(_BufInfo)
            {
            }

            /**
                   * @brief Constructor
                   */
            PortInfo(ImgInfo const& _ImgInfo, PortID const& _PortID, BufInfo const& _BufInfo, RingInfo const& _RingInfo )
                : ImgInfo(_ImgInfo)
                , PortID(_PortID)
                , BufInfo(_BufInfo)
                , RingInfo(_RingInfo)
            {
            }

            /**
                   * @brief Constructor
                   */
            PortInfo(ImgInfo const& _ImgInfo, PortID const& _PortID, BufInfo const& _BufInfo, RingInfo const& _RingInfo, SegmentInfo const& _SegmentInfo )
                : ImgInfo(_ImgInfo)
                , PortID(_PortID)
                , BufInfo(_BufInfo)
                , RingInfo(_RingInfo)
                , SegmentInfo(_SegmentInfo)
            {
            }
        };
    };  //namespace NSIspio
};  //namespace NSImageio
#endif  //  _ISPIO_PIPE_PORTS_H_

