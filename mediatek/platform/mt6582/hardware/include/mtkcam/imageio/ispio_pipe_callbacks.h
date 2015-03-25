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
* @file ispio_pipe_callbacks.h
*
* ispio_pipe_callbacks Header File
*/

#ifndef _ISPIO_PIPE_CALLBACKS_H_
#define _ISPIO_PIPE_CALLBACKS_H_

namespace NSImageio 
{
    namespace NSIspio
    {        
        /**
             * @brief  Pipe Notify Callback
             */
        struct PipeNotifyInfo
        {
            MUINT32 msgType;    ///<  Pipe-dependent Notify Message Type.
            MUINT32 ext1;       ///<  Extended parameter 1.
            MUINT32 ext2;       ///<  Extended parameter 2.

            /**
                   * @brief  Constructors
                   */
            PipeNotifyInfo(
                MUINT32 const _msgType = 0, 
                MUINT32 const _ext1 = 0, 
                MUINT32 const _ext2 = 0
            )
                : msgType(_msgType)
                , ext1(_ext1)
                , ext2(_ext2)
            {}
        };

        typedef MBOOL (*PipeNotifyCallback_t)(MVOID* user, PipeNotifyInfo const msg);
        
        /**
             * @brief  Pipe Data Callback
             */
        struct PipeDataInfo
        {
            MUINT32 msgType;    ///<  Pipe-dependent Data Message Type.
            MUINT32 ext1;       ///<  Extended parameter 1.
            MUINT32 ext2;       ///<  Extended parameter 2.
            MUINT8* puData;     ///<  Pointer to the callback data.
            MUINT32 u4Size;     ///<  Size of the callback data.

            /**
                   * @brief  Constructors
                   */
            PipeDataInfo(
                MUINT32 const _msgType = 0, 
                MUINT32 const _ext1 = 0, 
                MUINT32 const _ext2 = 0, 
                MUINT8* const _puData = NULL, 
                MUINT32 const _u4Size = 0
            )
                : msgType(_msgType)
                , ext1(_ext1)
                , ext2(_ext2)
                , puData(_puData)
                , u4Size(_u4Size)
            {
            }
        };

        typedef MBOOL   (*PipeDataCallback_t)(MVOID* user, PipeDataInfo const msg);
    };  //namespace NSIspio
};  //namespace NSImageio
#endif  //  _ISPIO_PIPE_CALLBACKS_H_

