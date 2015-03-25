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
#ifndef ISP_DRV_IMP_H
#define ISP_DRV_IMP_H


//#include <utils/Errors.h>
#include "mtkcam/common.h"
#include <mtkcam/drv/isp_drv.h>
#include <mtkcam/drv/imem_drv.h>   // For IMemDrv*

using namespace android;

/*******************************************************************************
* 
********************************************************************************/

#define ISP_DRV_DEV_NAME            "/dev/camera-isp"
#define ISP_DRV_VIR_DEFAULT_DATA    (0)
#define ISP_DRV_VIR_ADDR_ALIGN      (0x03) // 4 bytes alignment
#define ISP_DRV_CQ_DESCRIPTOR_ADDR_ALIGN      (0x07) // 8 bytes alignment

/*******************************************************************************
* 
********************************************************************************/

typedef struct 
{
    MUINT32     checkBit;
    MUINT32     checkOffset;
}ISP_DRV_TURNING_TOP;

typedef struct 
{
    ISP_DRV_CQ_ENUM             virtualAddrCq;
    ISP_DRV_DESCRIPTOR_CQ_ENUM  descriptorCq;
}ISP_DRV_CQ_MAPPING;

/*******************************************************************************
* 
********************************************************************************/
class IspDrvImp : public IspDrv
{
    public:
        IspDrvImp();
        virtual ~IspDrvImp();
        
        static IspDrv *getInstance(MBOOL fgIsGdmaMode = MFALSE);
        virtual MVOID  destroyInstance();
        virtual MBOOL  init();
        virtual MBOOL  uninit();
        virtual MBOOL  waitIrq(ISP_DRV_WAIT_IRQ_STRUCT WaitIrq);
        virtual MBOOL  readIrq(ISP_DRV_READ_IRQ_STRUCT* pReadIrq);
        virtual MBOOL  checkIrq(ISP_DRV_CHECK_IRQ_STRUCT CheckIrq);
        virtual MBOOL  clearIrq(ISP_DRV_CLEAR_IRQ_STRUCT ClearIrq);
        virtual MBOOL  reset();
        virtual MBOOL  resetBuf();
        virtual MUINT32 getRegAddr();
        virtual isp_reg_t *getRegAddrMap();
        virtual MBOOL   readRegs(ISP_DRV_REG_IO_STRUCT *pRegIo, MUINT32 Count);
        virtual MUINT32 readReg(MUINT32 Addr);
        virtual MBOOL   writeRegs(ISP_DRV_REG_IO_STRUCT *pRegIo, MUINT32 Count);
        virtual MBOOL   writeReg(MUINT32 Addr, MUINT32 Data);
        virtual MBOOL   holdReg(MBOOL En);
        virtual MBOOL   dumpReg();
        virtual MBOOL   IsReadOnlyMode();
//        virtual MBOOL   IsGdmaMode(void);

#if defined(_use_kernel_ref_cnt_)

        virtual MBOOL   kRefCntCtrl(ISP_REF_CNT_CTRL_STRUCT *pCtrl);
#endif

        virtual MUINT32 GlobalPipeCountInc();
        virtual MUINT32 GlobalPipeCountDec();
        virtual MBOOL   rtBufCtrl(MVOID *pBuf_ctrl);

#if defined(_rtbc_use_cq0c_)
        virtual MBOOL cqRingBuf(MVOID *pBuf_ctrl);
#endif
    
    private:
        volatile MINT32 mInitCount;
        mutable Mutex   mLock;
        
        MBOOL m_fgIsGdmaMode;
        
        //imem
        IMemDrv *m_pIMemDrv ;
        IMEM_BUF_INFO m_ispVirRegBufInfo;
        IMEM_BUF_INFO m_ispCQDescBufInfo;
    //
};

/*******************************************************************************
* 
********************************************************************************/
class IspDrvVirImp : public IspDrv
{
    public:
        IspDrvVirImp();
        virtual ~IspDrvVirImp();

        static IspDrv *getInstance(ISP_DRV_CQ_ENUM cq, MUINT32 ispVirRegAddr);
        virtual MVOID  destroyInstance();
        virtual MBOOL  init(){ /*LOG_ERR("Not support");*/return MTRUE;}
        virtual MBOOL  uninit(){/*LOG_ERR("Not support");*/return MTRUE;}
        virtual MBOOL  waitIrq(ISP_DRV_WAIT_IRQ_STRUCT WaitIrq){/*LOG_ERR("Not support");*/return MTRUE;}
        virtual MBOOL  readIrq(ISP_DRV_READ_IRQ_STRUCT *pReadIrq){/*LOG_ERR("Not support");*/return MTRUE;}
        virtual MBOOL  checkIrq(ISP_DRV_CHECK_IRQ_STRUCT CheckIrq){/*LOG_ERR("Not support");*/return MTRUE;}
        virtual MBOOL  clearIrq(ISP_DRV_CLEAR_IRQ_STRUCT ClearIrq){/*LOG_ERR("Not support");*/return MTRUE;}
        virtual MBOOL  reset();
        virtual MBOOL  resetBuf(){/*LOG_ERR("Not support");*/return MTRUE;}
        virtual MUINT32 getRegAddr();
        virtual isp_reg_t *getRegAddrMap(){return NULL;}
        virtual MBOOL   readRegs(ISP_DRV_REG_IO_STRUCT *pRegIo, MUINT32 Count);
        virtual MUINT32 readReg(MUINT32 Addr);
        virtual MBOOL   writeRegs(ISP_DRV_REG_IO_STRUCT *pRegIo, MUINT32 Count);
        virtual MBOOL   writeReg(MUINT32 Addr, MUINT32 Data);
        virtual MBOOL   holdReg(MBOOL En){/*LOG_ERR("Not support");*/return MTRUE;}
        virtual MBOOL   dumpReg(){/*LOG_ERR("Not support");*/return MTRUE;}
        virtual MBOOL   IsReadOnlyMode(){return MFALSE;}
//      virtual MBOOL   IsGdmaMode(){return MFALSE;}
        virtual MBOOL   rtBufCtrl(MVOID *pBuf_ctrl){return MTRUE;}
        virtual MUINT32 GlobalPipeCountInc(){return MTRUE;};
        virtual MUINT32 GlobalPipeCountDec(){return MTRUE;};

    private:
        mutable Mutex   mLock;
        MUINT32 *mpIspVirRegBuffer;
};

#endif

