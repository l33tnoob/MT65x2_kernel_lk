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
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#ifndef PIPE_MGR_DRV_H
#define PIPE_MGR_DRV_H
//-----------------------------------------------------------------------------
#define PIPE_MGR_DRV_PIPE_MASK_CAM_IO       ((MUINT32)1 << 0)
#define PIPE_MGR_DRV_PIPE_MASK_POST_PROC    ((MUINT32)1 << 1)
#define PIPE_MGR_DRV_PIPE_MASK_XDP_CAM      ((MUINT32)1 << 2)
//-----------------------------------------------------------------------------
typedef struct
{
    MUINT32 PipeMask;
    MUINT32 Timeout; //ms
}PIPE_MGR_DRV_LOCK_STRUCT;

typedef struct
{
    MUINT32 PipeMask;
}PIPE_MGR_DRV_UNLOCK_STRUCT;
//-----------------------------------------------------------------------------
class PipeMgrDrv
{
    protected:
        virtual ~PipeMgrDrv() {};
    //
    public:
        static PipeMgrDrv* CreateInstance(void);
        virtual MVOID   DestroyInstance(void) = 0;
        virtual MBOOL   Init(void) = 0;
        virtual MBOOL   Uninit(void) = 0;
        virtual MBOOL   Lock(PIPE_MGR_DRV_LOCK_STRUCT* pLock) = 0;
        virtual MBOOL   Unlock(PIPE_MGR_DRV_UNLOCK_STRUCT* pUnlock) = 0;
        virtual MBOOL   Dump(void) = 0;
};
//-----------------------------------------------------------------------------
#endif

