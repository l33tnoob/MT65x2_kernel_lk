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
#ifndef VSS_IMG_TRANS_IMP_H
#define VSS_IMG_TRANS_IMP_H
//----------------------------------------------------------------------------
using namespace android;
//----------------------------------------------------------------------------
#define LOG_MSG(fmt, arg...)    XLOGD("(%d)[%s]"          fmt, ::gettid(), __FUNCTION__,           ##arg)
#define LOG_WRN(fmt, arg...)    XLOGW("(%d)[%s]WRN(%5d):" fmt, ::gettid(), __FUNCTION__, __LINE__, ##arg)
#define LOG_ERR(fmt, arg...)    XLOGE("(%d)[%s]ERR(%5d):" fmt, ::gettid(), __FUNCTION__, __LINE__, ##arg)
#define LOG_DMP(fmt, arg...)    XLOGE("(%d)"              fmt, ::gettid()                          ##arg)
//----------------------------------------------------------------------------
class VssImgTransImp : public VssImgTrans
{
    protected:
        VssImgTransImp();
        ~VssImgTransImp();
    //
    public:
        static VssImgTrans* GetInstance(void);
        virtual void    DestroyInstance(void);
        virtual MBOOL   Init(CONFIG_STRUCT& Config, MUINT32& TpipeNum);
        virtual MBOOL   Uninit(void);
        virtual MBOOL   Start(MUINT32 TpipeIndex);
        virtual MBOOL   WaitDone(void);
        //
        virtual MVOID   ConfigImgi(PortInfo &ImgiPort);
        virtual MVOID   ConfigDispo(PortInfo &DispoPort);
        virtual MVOID   ConfigVido(PortInfo &VidoPort);
        virtual MVOID   ConfigPass2(void);
    //
    private:
        mutable Mutex       mLock;
        volatile MINT32     mUser;
        volatile MUINT32    mTpipeNum;
        volatile MBOOL      mStart;
        CONFIG_STRUCT       mConfig;
        EScenarioFmt        mFormat;
        PortInfo            mImgiPort;
        PortInfo            mDispoPort;
        PortInfo            mVidoPort;
        IPipe*              mpPipePass2;
        IPostProcPipe*      mpPostProcPipe;
        ICdpPipe*           mpCdpPipe;
        vector<PortInfo const*> mvPortIn;
        vector<PortInfo const*> mvPortOut;
};
//----------------------------------------------------------------------------
#endif





