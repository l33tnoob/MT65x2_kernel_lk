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
#ifndef PIPE_MGR_DRV_IMP_H
#define PIPE_MGR_DRV_IMP_H
//-----------------------------------------------------------------------------
using namespace android;
//-----------------------------------------------------------------------------
#define LOG_MSG(fmt, arg...)    XLOGD("[%s]"          fmt, __FUNCTION__,           ##arg)
#define LOG_WRN(fmt, arg...)    XLOGW("[%s]WRN(%5d):" fmt, __FUNCTION__, __LINE__, ##arg)
#define LOG_ERR(fmt, arg...)    XLOGE("[%s]ERR(%5d):" fmt, __FUNCTION__, __LINE__, ##arg)
#define LOG_DMP(fmt, arg...)    XLOGE(""              fmt,                         ##arg)
//-----------------------------------------------------------------------------
#define PIPE_MGR_DRV_DEVNAME    "/dev/camera-pipemgr"
//-----------------------------------------------------------------------------
class PipeMgrDrvImp : public PipeMgrDrv
{
    protected:
        PipeMgrDrvImp();
        virtual ~PipeMgrDrvImp();
    //
    public:
        static PipeMgrDrv* GetInstance(void);
        virtual MVOID   DestroyInstance(void);
        virtual MBOOL   Init(void);
        virtual MBOOL   Uninit(void);
        virtual MBOOL   Lock(PIPE_MGR_DRV_LOCK_STRUCT* pLock);
        virtual MBOOL   Unlock(PIPE_MGR_DRV_UNLOCK_STRUCT* pUnlock);
        virtual MBOOL   Dump(void);
        //
    private:
        mutable Mutex mLock;
        volatile MINT32 mUser;
        MINT32 mFd;
        MUINT32 mLogMask;
};
//-----------------------------------------------------------------------------
#endif

