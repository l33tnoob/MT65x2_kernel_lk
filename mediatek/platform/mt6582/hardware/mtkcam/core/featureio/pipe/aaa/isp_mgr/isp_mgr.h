/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

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
#ifndef _ISP_MGR_H_
#define _ISP_MGR_H_

#include <utils/threads.h>
#include <ispdrv_mgr.h>
#include <mtkcam/drv/isp_reg.h>
#include <isp_tuning.h>
#include <mtkcam/drv/isp_drv.h>
#include <ispif.h>
#include <aaa_types.h>
#include <ae_param.h>
#include <shading_tuning_custom.h>

using namespace android;
using namespace NS3A;

namespace NSIspTuning
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  ISP manager
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef class ISP_MGR_BASE
{
protected:
    typedef ISP_MGR_BASE  MyType;
    typedef ISPREG_INFO_T RegInfo_T;

protected:
    virtual ~ISP_MGR_BASE() {}
    ISP_MGR_BASE(MVOID*const pRegInfo, MUINT32 const u4RegInfoNum, MUINT32& u4StartAddr)
     : m_pRegInfo(pRegInfo)
     , m_u4RegInfoNum(u4RegInfoNum)
     , m_u4StartAddr(u4StartAddr)
    {
    }

protected:
    MVOID*const     m_pRegInfo;
    MUINT32 const   m_u4RegInfoNum;
    MUINT32&        m_u4StartAddr;

//==============================================================================
protected:

#define REG_ADDR(reg)\
    ((MUINT32)(&getIspReg(ISPDRV_MODE_ISP)->reg) - (MUINT32)getIspReg(ISPDRV_MODE_ISP))

#define REG_INFO(reg)\
    (m_rIspRegInfo[ERegInfo_##reg])

#define REG_INFO_ADDR(reg)\
    (REG_INFO(reg).addr)

#define REG_INFO_VALUE(reg)\
    (REG_INFO(reg).val)

#define REG_INFO_VALUE_PTR(reg)\
    (& REG_INFO_VALUE(reg))

#define INIT_REG_INFO_ADDR(reg)\
    REG_INFO_ADDR(reg) = REG_ADDR(reg)

#define INIT_REG_INFO_VALUE(reg, val)\
    REG_INFO_VALUE(reg) = val

#define PUT_REG_INFO(dest, src)\
    REG_INFO_VALUE(dest) = setbits(REG_INFO_VALUE(dest), rParam.src)

#define GET_REG_INFO(src, dest)\
    rParam.dest.val = REG_INFO_VALUE(src)

#define INIT_ISP_DRV_MODE(camMode, ispDrvMode) \
    m_rIspDrvMode[camMode] = ispDrvMode

    template <class _ISP_XXX_T>
    inline
    MUINT32
    setbits(MUINT32 const dest, _ISP_XXX_T const src)
    {
        MUINT32 const u4Mask = _ISP_XXX_T::MASK;
        //  (1) clear bits + (2) set bits
        return  ((dest & ~u4Mask) | (src.val & u4Mask));
    }

    inline
    isp_reg_t*
    getIspReg(MINT32 i4IspDrvMode) const
    {
        return reinterpret_cast<isp_reg_t*> (
            IspDrvMgr::getInstance().getIspReg(static_cast<ISPDRV_MODE_T>(i4IspDrvMode))
        );
    }

    inline
    MBOOL
    readRegs(MINT32 i4IspDrvMode, RegInfo_T*const pRegInfo, MUINT32 const u4RegInfoNum) const
    {
        return  IspDrvMgr::getInstance().readRegs(static_cast<ISPDRV_MODE_T>(i4IspDrvMode), pRegInfo, u4RegInfoNum);
    }

    inline
    MBOOL
    writeRegs(CAM_MODULE_ENUM eCamModule, MINT32 i4IspDrvMode, RegInfo_T*const pRegInfo, MUINT32 const u4RegInfoNum)
    {
        return  IspDrvMgr::getInstance().writeRegs(eCamModule, static_cast<ISPDRV_MODE_T>(i4IspDrvMode), pRegInfo, u4RegInfoNum);
    }

public: // Interfaces
    virtual
    MBOOL
    reset()
    {
        addressErrorCheck("Before reset()");
        MBOOL err = readRegs(ISPDRV_MODE_ISP, static_cast<RegInfo_T*>(m_pRegInfo), m_u4RegInfoNum);
        addressErrorCheck("After reset()");
        return err;
    }

    virtual
    MVOID
    dumpRegInfo();

    virtual
    MVOID
    dumpRegs();

    virtual
    MVOID
    addressErrorCheck(char const*const ptestCastName);

} ISP_MGR_BASE_T;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  CAM_CTL_EN
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef class ISP_MGR_CTL_EN : public ISP_MGR_BASE_T
{
    typedef ISP_MGR_CTL_EN    MyType;
private:
    MUINT32 m_u4StartAddr; // for debug purpose: 0x4004

    enum
    {
        ERegInfo_CAM_CTL_EN1,
        ERegInfo_CAM_CTL_EN2,
        ERegInfo_NUM
    };
    RegInfo_T     m_rIspRegInfo[ERegInfo_NUM];

protected:
    ISP_MGR_CTL_EN()
        : ISP_MGR_BASE_T(m_rIspRegInfo, ERegInfo_NUM, m_u4StartAddr)
        , m_u4StartAddr(REG_ADDR(CAM_CTL_EN1))
    {
        INIT_REG_INFO_ADDR(CAM_CTL_EN1); // 0x4004
        INIT_REG_INFO_ADDR(CAM_CTL_EN2); // 0x4008
    }

    virtual ~ISP_MGR_CTL_EN() {}

public:
    static MyType&  getInstance(ESensorDev_T const eSensorDev);

public: // Interfaces.

    template <class ISP_xxx_T>
    MyType& get(ISP_xxx_T & rParam);

} ISP_MGR_CTL_EN_T;

template <ESensorDev_T const eSensorDev>
class ISP_MGR_CTL_EN_DEV : public ISP_MGR_CTL_EN_T
{
public:
    static
    ISP_MGR_CTL_EN_T&
    getInstance()
    {
        static ISP_MGR_CTL_EN_DEV<eSensorDev> singleton;
        return singleton;
    }
    virtual MVOID destroyInstance() {}

    ISP_MGR_CTL_EN_DEV()
        : ISP_MGR_CTL_EN_T()
    {}

    virtual ~ISP_MGR_CTL_EN_DEV() {}

};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  OBC
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef class ISP_MGR_OBC : public ISP_MGR_BASE_T
{
    typedef ISP_MGR_OBC    MyType;
private:
    MBOOL m_bEnable;
    MINT32 m_u4IspAEGain;
    MUINT32 m_u4StartAddr; // for debug purpose: 0x4500

    enum
    {
        ERegInfo_CAM_OBC_OFFST0,
        ERegInfo_CAM_OBC_OFFST1,
        ERegInfo_CAM_OBC_OFFST2,
        ERegInfo_CAM_OBC_OFFST3,
        ERegInfo_CAM_OBC_GAIN0,
        ERegInfo_CAM_OBC_GAIN1,
        ERegInfo_CAM_OBC_GAIN2,
        ERegInfo_CAM_OBC_GAIN3,
        ERegInfo_NUM
    };
    RegInfo_T     m_rIspRegInfo[ERegInfo_NUM];
    MINT32 m_rIspDrvMode[EIspProfile_NUM];


protected:
    ISP_MGR_OBC()
        : ISP_MGR_BASE_T(m_rIspRegInfo, ERegInfo_NUM, m_u4StartAddr)
        , m_bEnable(MTRUE)
        , m_u4IspAEGain(512)
        , m_u4StartAddr(REG_ADDR(CAM_OBC_OFFST0))
    {
        INIT_REG_INFO_ADDR(CAM_OBC_OFFST0); // 0x4500
        INIT_REG_INFO_ADDR(CAM_OBC_OFFST1); // 0x4504
        INIT_REG_INFO_ADDR(CAM_OBC_OFFST2); // 0x4508
        INIT_REG_INFO_ADDR(CAM_OBC_OFFST3); // 0x450C
        INIT_REG_INFO_ADDR(CAM_OBC_GAIN0);  // 0x4510
        INIT_REG_INFO_ADDR(CAM_OBC_GAIN1);  // 0x4514
        INIT_REG_INFO_ADDR(CAM_OBC_GAIN2);  // 0x4518
        INIT_REG_INFO_ADDR(CAM_OBC_GAIN3);  // 0x451C

        INIT_ISP_DRV_MODE(EIspProfile_NormalPreview, ISPDRV_MODE_CQ0); // PASS1:CQ0, PASS2: CQ1_SYNC
        INIT_ISP_DRV_MODE(EIspProfile_ZsdPreview_CC, ISPDRV_MODE_CQ0); // PASS1:CQ0, PASS2: CQ1_SYNC
        INIT_ISP_DRV_MODE(EIspProfile_ZsdPreview_NCC, ISPDRV_MODE_CQ0); // PASS1:CQ0
        INIT_ISP_DRV_MODE(EIspProfile_NormalCapture, ISPDRV_MODE_CQ0|ISPDRV_MODE_CQ1_SYNC); // PASS1:CQ0, PASS2: CQ1_SYNC
        INIT_ISP_DRV_MODE(EIspProfile_VideoPreview, ISPDRV_MODE_CQ0); // PASS1:CQ0, PASS2: CQ1_SYNC
        INIT_ISP_DRV_MODE(EIspProfile_VideoCapture, ISPDRV_MODE_CQ0); // PASS1:CQ0, PASS2: CQ2_SYNC
        INIT_ISP_DRV_MODE(EIspProfile_MFCapPass1, ISPDRV_MODE_CQ0); // TBD
        INIT_ISP_DRV_MODE(EIspProfile_MFCapPass2, ISPDRV_MODE_CQ0); // TBD
    }

    virtual ~ISP_MGR_OBC() {}

public:
    static MyType&  getInstance(ESensorDev_T const eSensorDev);

public: // Interfaces.

    template <class ISP_xxx_T>
    MyType& put(ISP_xxx_T const& rParam);

    template <class ISP_xxx_T>
    MyType& get(ISP_xxx_T & rParam);

    MBOOL
    isEnable()
    {
        return m_bEnable;
    }

    MVOID
    setEnable(MBOOL bEnable)
    {
        m_bEnable = bEnable;
    }

    MVOID setIspAEGain(MUINT32 u4IspAEGain)
    {
        m_u4IspAEGain = u4IspAEGain;
    }

    MVOID getIspAEGain(MUINT32 *u4IspAEGain)
    {
        *u4IspAEGain = m_u4IspAEGain;
    }

    MBOOL apply(EIspProfile_T eIspProfile);

} ISP_MGR_OBC_T;

template <ESensorDev_T const eSensorDev>
class ISP_MGR_OBC_DEV : public ISP_MGR_OBC_T
{
public:
    static
    ISP_MGR_OBC_T&
    getInstance()
    {
        static ISP_MGR_OBC_DEV<eSensorDev> singleton;
        return singleton;
    }
    virtual MVOID destroyInstance() {}

    ISP_MGR_OBC_DEV()
        : ISP_MGR_OBC_T()
    {}

    virtual ~ISP_MGR_OBC_DEV() {}

};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  BNR (BPC + NR1)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef class ISP_MGR_BNR : public ISP_MGR_BASE_T
{
    typedef ISP_MGR_BNR    MyType;
private:
    MBOOL m_bBPCEnable;
    MBOOL m_bCTEnable;
    MUINT32 m_u4StartAddr; // for debug purpose: 0x4800

    enum
    {
        ERegInfo_CAM_BPC_CON,
        ERegInfo_CAM_BPC_CD1_1,
        ERegInfo_CAM_BPC_CD1_2,
        ERegInfo_CAM_BPC_CD1_3,
        ERegInfo_CAM_BPC_CD1_4,
        ERegInfo_CAM_BPC_CD1_5,
        ERegInfo_CAM_BPC_CD1_6,
        ERegInfo_CAM_BPC_CD2_1,
        ERegInfo_CAM_BPC_CD2_2,
        ERegInfo_CAM_BPC_CD2_3,
        ERegInfo_CAM_BPC_CD0,
        ERegInfo_CAM_BPC_COR,
        ERegInfo_CAM_NR1_CON,
        ERegInfo_CAM_NR1_CT_CON,
        ERegInfo_NUM
    };
    RegInfo_T     m_rIspRegInfo[ERegInfo_NUM];
    MINT32 m_rIspDrvMode[EIspProfile_NUM];

protected:
    ISP_MGR_BNR()
        : ISP_MGR_BASE_T(m_rIspRegInfo, ERegInfo_NUM, m_u4StartAddr)
        , m_bBPCEnable(MTRUE)
        , m_bCTEnable(MTRUE)
        , m_u4StartAddr(REG_ADDR(CAM_BPC_CON))
    {
        INIT_REG_INFO_ADDR(CAM_BPC_CON);    // 0x4800
        INIT_REG_INFO_ADDR(CAM_BPC_CD1_1);  // 0x4804
        INIT_REG_INFO_ADDR(CAM_BPC_CD1_2);  // 0x4808
        INIT_REG_INFO_ADDR(CAM_BPC_CD1_3);  // 0x480C
        INIT_REG_INFO_ADDR(CAM_BPC_CD1_4);  // 0x4810
        INIT_REG_INFO_ADDR(CAM_BPC_CD1_5);  // 0x4814
        INIT_REG_INFO_ADDR(CAM_BPC_CD1_6);  // 0x4818
        INIT_REG_INFO_ADDR(CAM_BPC_CD2_1);  // 0x481C
        INIT_REG_INFO_ADDR(CAM_BPC_CD2_2);  // 0x4820
        INIT_REG_INFO_ADDR(CAM_BPC_CD2_3);  // 0x4824
        INIT_REG_INFO_ADDR(CAM_BPC_CD0);    // 0x4828
        INIT_REG_INFO_ADDR(CAM_BPC_COR);    // 0x4830
        INIT_REG_INFO_ADDR(CAM_NR1_CON);    // 0x4840
        INIT_REG_INFO_ADDR(CAM_NR1_CT_CON); // 0x4844

        INIT_ISP_DRV_MODE(EIspProfile_NormalPreview, ISPDRV_MODE_CQ0 ); // PASS1:CQ0, PASS2: CQ1_SYNC
        INIT_ISP_DRV_MODE(EIspProfile_ZsdPreview_CC, ISPDRV_MODE_CQ0 ); // PASS1:CQ0, PASS2: CQ1_SYNC
        INIT_ISP_DRV_MODE(EIspProfile_ZsdPreview_NCC, ISPDRV_MODE_CQ0); // PASS1:CQ0
        INIT_ISP_DRV_MODE(EIspProfile_NormalCapture, ISPDRV_MODE_CQ0|ISPDRV_MODE_CQ1_SYNC); // PASS1:CQ0, PASS2: CQ1_SYNC
        INIT_ISP_DRV_MODE(EIspProfile_VideoPreview, ISPDRV_MODE_CQ0 ); // PASS1:CQ0, PASS2: CQ1_SYNC
        INIT_ISP_DRV_MODE(EIspProfile_VideoCapture, ISPDRV_MODE_CQ0 ); // PASS1:CQ0, PASS2: CQ2_SYNC
        INIT_ISP_DRV_MODE(EIspProfile_MFCapPass1, ISPDRV_MODE_CQ0 ); // TBD
        INIT_ISP_DRV_MODE(EIspProfile_MFCapPass2, ISPDRV_MODE_CQ0 ); // TBD
    }

    virtual ~ISP_MGR_BNR() {}

public:
    static MyType&  getInstance(ESensorDev_T const eSensorDev);

public: // Interfaces.

    template <class ISP_xxx_T>
    MyType& put(ISP_xxx_T const& rParam);

    template <class ISP_xxx_T>
    MyType& get(ISP_xxx_T & rParam);

    MBOOL
    isBPCEnable()
    {
        return m_bBPCEnable;
    }

    MBOOL
    isCTEnable()
    {
        return m_bCTEnable;
    }

    MVOID
    setBPCEnable(MBOOL bEnable)
    {
        m_bBPCEnable = bEnable;
    }

    MVOID
    setCTEnable(MBOOL bEnable)
    {
        m_bCTEnable = bEnable;
    }

    MBOOL apply(EIspProfile_T eIspProfile);

} ISP_MGR_BNR_T;

template <ESensorDev_T const eSensorDev>
class ISP_MGR_BNR_DEV : public ISP_MGR_BNR_T
{
public:
    static
    ISP_MGR_BNR_T&
    getInstance()
    {
        static ISP_MGR_BNR_DEV<eSensorDev> singleton;
        return singleton;
    }
    virtual MVOID destroyInstance() {}

    ISP_MGR_BNR_DEV()
        : ISP_MGR_BNR_T()
    {}

    virtual ~ISP_MGR_BNR_DEV() {}

};


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  LSC
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef class ISP_MGR_LSC : public ISP_MGR_BASE_T
{
    typedef ISP_MGR_LSC    MyType;
private:
    MUINT32 m_u4StartAddr; // for debug purpose: 0x400C

    enum
    {
        ERegInfo_CAM_CTL_DMA_EN = 0,
        ERegInfo_CAM_LSCI_BASE_ADDR,
        ERegInfo_CAM_LSCI_XSIZE,
        ERegInfo_CAM_CTL_EN1,
        ERegInfo_CAM_LSC_START,
        ERegInfo_CAM_LSC_CTL1 = ERegInfo_CAM_LSC_START,
        ERegInfo_CAM_LSC_CTL2,
        ERegInfo_CAM_LSC_CTL3,
        ERegInfo_CAM_LSC_LBLOCK,
        ERegInfo_CAM_LSC_RATIO,
        ERegInfo_CAM_LSC_GAIN_TH,
        ERegInfo_NUM
    };
    RegInfo_T     m_rIspRegInfo[ERegInfo_NUM];
    MINT32 m_rIspDrvMode[EIspProfile_NUM];

protected:
    ISP_MGR_LSC()
        : ISP_MGR_BASE_T(m_rIspRegInfo, ERegInfo_NUM, m_u4StartAddr)
        , m_u4StartAddr(REG_ADDR(CAM_CTL_DMA_EN))
    {
        INIT_REG_INFO_ADDR(CAM_CTL_DMA_EN);     // 0x400C
        INIT_REG_INFO_ADDR(CAM_LSCI_BASE_ADDR); // 0x4530
        INIT_REG_INFO_ADDR(CAM_LSCI_XSIZE);     // 0x4274
        INIT_REG_INFO_ADDR(CAM_CTL_EN1);        // 0x4004
        INIT_REG_INFO_ADDR(CAM_LSC_CTL1);    // 0x4530
        INIT_REG_INFO_ADDR(CAM_LSC_CTL2);    // 0x4534
        INIT_REG_INFO_ADDR(CAM_LSC_CTL3);    // 0x4538
        INIT_REG_INFO_ADDR(CAM_LSC_LBLOCK);  // 0x453C
        INIT_REG_INFO_ADDR(CAM_LSC_RATIO);   // 0x4540
        INIT_REG_INFO_ADDR(CAM_LSC_GAIN_TH); // 0x454C

        INIT_ISP_DRV_MODE(EIspProfile_NormalPreview, ISPDRV_MODE_CQ0);
        INIT_ISP_DRV_MODE(EIspProfile_ZsdPreview_CC, ISPDRV_MODE_CQ0);
        INIT_ISP_DRV_MODE(EIspProfile_ZsdPreview_NCC, ISPDRV_MODE_CQ0);
        INIT_ISP_DRV_MODE(EIspProfile_NormalCapture, ISPDRV_MODE_CQ0|ISPDRV_MODE_CQ1_SYNC);
        INIT_ISP_DRV_MODE(EIspProfile_VideoPreview, ISPDRV_MODE_CQ0);
        INIT_ISP_DRV_MODE(EIspProfile_VideoCapture, ISPDRV_MODE_CQ0);
        INIT_ISP_DRV_MODE(EIspProfile_MFCapPass1, ISPDRV_MODE_CQ0);
        INIT_ISP_DRV_MODE(EIspProfile_MFCapPass2, ISPDRV_MODE_CQ0);
    }

    virtual ~ISP_MGR_LSC() {}

public:
    static MyType&  getInstance(ESensorDev_T const eSensorDev);

public: // Interfaces.

    template <class ISP_xxx_T>
    MyType& put(ISP_xxx_T const& rParam);

    template <class ISP_xxx_T>
    MyType& get(ISP_xxx_T & rParam);

    MBOOL apply(EIspProfile_T eIspProfile);
    MVOID enableLsc(MBOOL enable);
    MBOOL isEnable(void);


} ISP_MGR_LSC_T;

template <ESensorDev_T const eSensorDev>
class ISP_MGR_LSC_DEV : public ISP_MGR_LSC_T
{
public:
    static
    ISP_MGR_LSC_T&
    getInstance()
    {
        static ISP_MGR_LSC_DEV<eSensorDev> singleton;
        return singleton;
    }
    virtual MVOID destroyInstance() {}

    ISP_MGR_LSC_DEV()
        : ISP_MGR_LSC_T()
    {}

    virtual ~ISP_MGR_LSC_DEV() {}

};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  PGN
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef class ISP_MGR_PGN : public ISP_MGR_BASE_T
{
    typedef ISP_MGR_PGN    MyType;
private:
    MUINT32 m_u4StartAddr; // for debug purpose: 0x4880

    enum
    {
        ERegInfo_CAM_PGN_SATU01,
        ERegInfo_CAM_PGN_SATU23,
        ERegInfo_CAM_PGN_GAIN01,
        ERegInfo_CAM_PGN_GAIN23,
        ERegInfo_CAM_PGN_OFFS01,
        ERegInfo_CAM_PGN_OFFS23,
        ERegInfo_NUM
    };
    RegInfo_T     m_rIspRegInfo[ERegInfo_NUM];
    MINT32 m_rIspDrvMode[EIspProfile_NUM];
    AWB_GAIN_T  m_rIspAWBGain; // 1x = 512
    MINT32      m_i4FlareGain; // 1x = 512
    MINT32      m_i4FlareOffset;
    AWB_GAIN_T  m_rIspPregain; // = m_rIspAWBGain x m_i4FlareGain
    MINT32      m_i4IspFlareOffset; // =  m_i4FlareOffset x m_i4FlareGain

protected:
    ISP_MGR_PGN()
        : ISP_MGR_BASE_T(m_rIspRegInfo, ERegInfo_NUM, m_u4StartAddr)
        , m_u4StartAddr(REG_ADDR(CAM_PGN_SATU01))
    {
        INIT_REG_INFO_ADDR(CAM_PGN_SATU01); // 0x4880
        INIT_REG_INFO_ADDR(CAM_PGN_SATU23); // 0x4884
        INIT_REG_INFO_ADDR(CAM_PGN_GAIN01); // 0x4888
        INIT_REG_INFO_ADDR(CAM_PGN_GAIN23); // 0x488C
        INIT_REG_INFO_ADDR(CAM_PGN_OFFS01); // 0x4890
        INIT_REG_INFO_ADDR(CAM_PGN_OFFS23); // 0x4894

        INIT_REG_INFO_VALUE(CAM_PGN_SATU01,ISP_NVRAM_PGN_SATU01_T::DEFAULT);
        INIT_REG_INFO_VALUE(CAM_PGN_SATU23,ISP_NVRAM_PGN_SATU23_T::DEFAULT);

        INIT_ISP_DRV_MODE(EIspProfile_NormalPreview, ISPDRV_MODE_CQ1_SYNC);
        INIT_ISP_DRV_MODE(EIspProfile_ZsdPreview_CC, ISPDRV_MODE_CQ1_SYNC);
        INIT_ISP_DRV_MODE(EIspProfile_ZsdPreview_NCC, ISPDRV_MODE_CQ0);
        INIT_ISP_DRV_MODE(EIspProfile_NormalCapture, ISPDRV_MODE_CQ1_SYNC);
        INIT_ISP_DRV_MODE(EIspProfile_VideoPreview, ISPDRV_MODE_CQ1_SYNC);
        INIT_ISP_DRV_MODE(EIspProfile_VideoCapture, ISPDRV_MODE_CQ2_SYNC);
        INIT_ISP_DRV_MODE(EIspProfile_MFCapPass1, ISPDRV_MODE_CQ1_SYNC);
        INIT_ISP_DRV_MODE(EIspProfile_MFCapPass2, ISPDRV_MODE_CQ1_SYNC);

        m_rIspAWBGain.i4R = m_rIspAWBGain.i4G = m_rIspAWBGain.i4B = 512;
        m_rIspPregain.i4R = m_rIspPregain.i4G = m_rIspPregain.i4B = 512;
        m_i4FlareGain = 512;
        m_i4FlareOffset = 0;
        m_i4IspFlareOffset = 0;
    }

    virtual ~ISP_MGR_PGN() {}

public:
    static MyType&  getInstance(ESensorDev_T const eSensorDev);

public: // Interfaces.

    inline MBOOL setIspAWBGain(AWB_GAIN_T& rIspAWBGain)
    {
        m_rIspAWBGain = rIspAWBGain;
        return setIspPregain();
    }

    inline MBOOL setIspFlare(MINT32 i4FlareGain, MINT32 i4FlareOffset)
    {
        m_i4FlareGain = i4FlareGain;
        m_i4FlareOffset = i4FlareOffset;
        return setIspPregain();
    }

    MBOOL setIspPregain();

    template <class ISP_xxx_T>
    MyType& put(ISP_xxx_T const& rParam);

    template <class ISP_xxx_T>
    MyType& get(ISP_xxx_T & rParam);


    MBOOL apply(EIspProfile_T eIspProfile);

} ISP_MGR_PGN_T;

template <ESensorDev_T const eSensorDev>
class ISP_MGR_PGN_DEV : public ISP_MGR_PGN_T
{
public:
    static
    ISP_MGR_PGN_T&
    getInstance()
    {
        static ISP_MGR_PGN_DEV<eSensorDev> singleton;
        return singleton;
    }
    virtual MVOID destroyInstance() {}

    ISP_MGR_PGN_DEV()
        : ISP_MGR_PGN_T()
    {}

    virtual ~ISP_MGR_PGN_DEV() {}

};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  SL2
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef class ISP_MGR_SL2 : public ISP_MGR_BASE_T
{
    typedef ISP_MGR_SL2    MyType;
private:
    MBOOL m_bEnable;
    MUINT32 m_u4StartAddr; // for debug purpose: 0x4F40

    enum
    {
        ERegInfo_CAM_SL2_CEN,
        ERegInfo_CAM_SL2_MAX0_RR,
        ERegInfo_CAM_SL2_MAX1_RR,
        ERegInfo_CAM_SL2_MAX2_RR,
        ERegInfo_NUM
    };
    RegInfo_T     m_rIspRegInfo[ERegInfo_NUM];
    MINT32 m_rIspDrvMode[EIspProfile_NUM];

protected:
    ISP_MGR_SL2()
        : ISP_MGR_BASE_T(m_rIspRegInfo, ERegInfo_NUM, m_u4StartAddr)
        , m_bEnable(isEnableSL2() ? MTRUE : MFALSE)
        , m_u4StartAddr(REG_ADDR(CAM_SL2_CEN))
    {
        INIT_REG_INFO_ADDR(CAM_SL2_CEN);      // 0x4F40
        INIT_REG_INFO_ADDR(CAM_SL2_MAX0_RR);  // 0x4F44
        INIT_REG_INFO_ADDR(CAM_SL2_MAX1_RR);  // 0x4F48
        INIT_REG_INFO_ADDR(CAM_SL2_MAX2_RR);  // 0x4F4C

        INIT_ISP_DRV_MODE(EIspProfile_NormalPreview, ISPDRV_MODE_CQ1_SYNC);
        INIT_ISP_DRV_MODE(EIspProfile_ZsdPreview_CC, ISPDRV_MODE_CQ1_SYNC);
        INIT_ISP_DRV_MODE(EIspProfile_ZsdPreview_NCC, ISPDRV_MODE_CQ0);
        INIT_ISP_DRV_MODE(EIspProfile_NormalCapture, ISPDRV_MODE_CQ1_SYNC);
        INIT_ISP_DRV_MODE(EIspProfile_VideoPreview, ISPDRV_MODE_CQ1_SYNC);
        INIT_ISP_DRV_MODE(EIspProfile_VideoCapture, ISPDRV_MODE_CQ2_SYNC);
        INIT_ISP_DRV_MODE(EIspProfile_MFCapPass1, ISPDRV_MODE_CQ1_SYNC);
        INIT_ISP_DRV_MODE(EIspProfile_MFCapPass2, ISPDRV_MODE_CQ1_SYNC);
    }

    virtual ~ISP_MGR_SL2() {}

public:
    static MyType&  getInstance(ESensorDev_T const eSensorDev);

public: // Interfaces.

    MBOOL
    isEnable()
    {
        return m_bEnable;
    }

    MVOID
    setEnable(MBOOL bEnable)
    {
        m_bEnable = bEnable;
    }
    
    template <class ISP_xxx_T>
    MyType& put(ISP_xxx_T const& rParam);

    template <class ISP_xxx_T>
    MyType& get(ISP_xxx_T & rParam);


    MBOOL apply(EIspProfile_T eIspProfile);

} ISP_MGR_SL2_T;

template <ESensorDev_T const eSensorDev>
class ISP_MGR_SL2_DEV : public ISP_MGR_SL2_T
{
public:
    static
    ISP_MGR_SL2_T&
    getInstance()
    {
        static ISP_MGR_SL2_DEV<eSensorDev> singleton;
        return singleton;
    }
    virtual MVOID destroyInstance() {}

    ISP_MGR_SL2_DEV()
        : ISP_MGR_SL2_T()
    {}

    virtual ~ISP_MGR_SL2_DEV() {}

};


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  CFA
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef class ISP_MGR_CFA : public ISP_MGR_BASE_T
{
    typedef ISP_MGR_CFA    MyType;
private:
    MBOOL m_bEnable;
    MUINT32 m_u4StartAddr; // for debug purpose: 0x48A0

    enum
    {
        ERegInfo_CAM_CFA_BYPASS,
        ERegInfo_CAM_CFA_ED_F,
        ERegInfo_CAM_CFA_ED_NYQ,
        ERegInfo_CAM_CFA_ED_STEP,
        ERegInfo_CAM_CFA_RGB_HF,
        ERegInfo_CAM_CFA_BW,
        ERegInfo_CAM_CFA_F1_ACT,
        ERegInfo_CAM_CFA_F2_ACT,
        ERegInfo_CAM_CFA_F3_ACT,
        ERegInfo_CAM_CFA_F4_ACT,
        ERegInfo_CAM_CFA_F1_L,
        ERegInfo_CAM_CFA_F2_L,
        ERegInfo_CAM_CFA_F3_L,
        ERegInfo_CAM_CFA_F4_L,
        ERegInfo_CAM_CFA_HF_RB,
        ERegInfo_CAM_CFA_HF_GAIN,
        ERegInfo_CAM_CFA_HF_COMP,
        ERegInfo_CAM_CFA_HF_CORING_TH,
        ERegInfo_CAM_CFA_ACT_LUT,
        ERegInfo_CAM_CFA_SPARE,
        ERegInfo_CAM_CFA_BB,
        ERegInfo_NUM
    };
    RegInfo_T   m_rIspRegInfo[ERegInfo_NUM];
    MINT32 m_rIspDrvMode[EIspProfile_NUM];

protected:
    ISP_MGR_CFA()
        : ISP_MGR_BASE_T(m_rIspRegInfo, ERegInfo_NUM, m_u4StartAddr)
        , m_bEnable(MTRUE)
        , m_u4StartAddr(REG_ADDR(CAM_CFA_BYPASS))
    {
        INIT_REG_INFO_ADDR(CAM_CFA_BYPASS);       // 0x48A0
        INIT_REG_INFO_ADDR(CAM_CFA_ED_F);         // 0x48A4
        INIT_REG_INFO_ADDR(CAM_CFA_ED_NYQ);       // 0x48A8
        INIT_REG_INFO_ADDR(CAM_CFA_ED_STEP);      // 0x48AC
        INIT_REG_INFO_ADDR(CAM_CFA_RGB_HF);       // 0x48B0
        INIT_REG_INFO_ADDR(CAM_CFA_BW);           // 0x48B4
        INIT_REG_INFO_ADDR(CAM_CFA_F1_ACT);       // 0x48B8
        INIT_REG_INFO_ADDR(CAM_CFA_F2_ACT);       // 0x48BC
        INIT_REG_INFO_ADDR(CAM_CFA_F3_ACT);       // 0x48C0
        INIT_REG_INFO_ADDR(CAM_CFA_F4_ACT);       // 0x48C4
        INIT_REG_INFO_ADDR(CAM_CFA_F1_L);         // 0x48C8
        INIT_REG_INFO_ADDR(CAM_CFA_F2_L);         // 0x48CC
        INIT_REG_INFO_ADDR(CAM_CFA_F3_L);         // 0x48D0
        INIT_REG_INFO_ADDR(CAM_CFA_F4_L);         // 0x48D4
        INIT_REG_INFO_ADDR(CAM_CFA_HF_RB);        // 0x48D8
        INIT_REG_INFO_ADDR(CAM_CFA_HF_GAIN);      // 0x48DC
        INIT_REG_INFO_ADDR(CAM_CFA_HF_COMP);      // 0x48E0
        INIT_REG_INFO_ADDR(CAM_CFA_HF_CORING_TH); // 0x48E4
        INIT_REG_INFO_ADDR(CAM_CFA_ACT_LUT);      // 0x48E8
        INIT_REG_INFO_ADDR(CAM_CFA_SPARE);        // 0x48F0
        INIT_REG_INFO_ADDR(CAM_CFA_BB);           // 0x48F4

        INIT_ISP_DRV_MODE(EIspProfile_NormalPreview, ISPDRV_MODE_CQ1_SYNC);
        INIT_ISP_DRV_MODE(EIspProfile_ZsdPreview_CC, ISPDRV_MODE_CQ1_SYNC);
        INIT_ISP_DRV_MODE(EIspProfile_ZsdPreview_NCC, ISPDRV_MODE_CQ0);
        INIT_ISP_DRV_MODE(EIspProfile_NormalCapture, ISPDRV_MODE_CQ1_SYNC);
        INIT_ISP_DRV_MODE(EIspProfile_VideoPreview, ISPDRV_MODE_CQ1_SYNC);
        INIT_ISP_DRV_MODE(EIspProfile_VideoCapture, ISPDRV_MODE_CQ2_SYNC);
        INIT_ISP_DRV_MODE(EIspProfile_MFCapPass1, ISPDRV_MODE_CQ1_SYNC);
        INIT_ISP_DRV_MODE(EIspProfile_MFCapPass2, ISPDRV_MODE_CQ1_SYNC);
    }

    virtual ~ISP_MGR_CFA() {}

public:
    static MyType&  getInstance(ESensorDev_T const eSensorDev);

public: // Interfaces.

    MBOOL
    isEnable()
    {
        return m_bEnable;
    }

    MVOID
    setEnable(MBOOL bEnable)
    {
        m_bEnable = bEnable;
    }

    template <class ISP_xxx_T>
    MyType& put(ISP_xxx_T const& rParam);

    template <class ISP_xxx_T>
    MyType& get(ISP_xxx_T & rParam);

    MBOOL apply(EIspProfile_T eIspProfile);

} ISP_MGR_CFA_T;

template <ESensorDev_T const eSensorDev>
class ISP_MGR_CFA_DEV : public ISP_MGR_CFA_T
{
public:
    static
    ISP_MGR_CFA_T&
    getInstance()
    {
        static ISP_MGR_CFA_DEV<eSensorDev> singleton;
        return singleton;
    }
    virtual MVOID destroyInstance() {}

    ISP_MGR_CFA_DEV()
        : ISP_MGR_CFA_T()
    {}

    virtual ~ISP_MGR_CFA_DEV() {}

};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  CCM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef class ISP_MGR_CCM : public ISP_MGR_BASE_T
{
    typedef ISP_MGR_CCM    MyType;
private:
    MBOOL m_bEnable;
    MUINT32 m_u4StartAddr; // for debug purpose: 0x4920

    enum
    {
        ERegInfo_CAM_G2G_CONV0A,
        ERegInfo_CAM_G2G_CONV0B,
        ERegInfo_CAM_G2G_CONV1A,
        ERegInfo_CAM_G2G_CONV1B,
        ERegInfo_CAM_G2G_CONV2A,
        ERegInfo_CAM_G2G_CONV2B,
        ERegInfo_NUM
    };
    RegInfo_T   m_rIspRegInfo[ERegInfo_NUM];
    MINT32 m_rIspDrvMode[EIspProfile_NUM];

protected:
    ISP_MGR_CCM()
        : ISP_MGR_BASE_T(m_rIspRegInfo, ERegInfo_NUM, m_u4StartAddr)
        , m_bEnable(MTRUE)
        , m_u4StartAddr(REG_ADDR(CAM_G2G_CONV0A))
    {
        INIT_REG_INFO_ADDR(CAM_G2G_CONV0A); // 0x4920
        INIT_REG_INFO_ADDR(CAM_G2G_CONV0B); // 0x4924
        INIT_REG_INFO_ADDR(CAM_G2G_CONV1A); // 0x4928
        INIT_REG_INFO_ADDR(CAM_G2G_CONV1B); // 0x492C
        INIT_REG_INFO_ADDR(CAM_G2G_CONV2A); // 0x4930
        INIT_REG_INFO_ADDR(CAM_G2G_CONV2B); // 0x4934

        INIT_ISP_DRV_MODE(EIspProfile_NormalPreview, ISPDRV_MODE_CQ1_SYNC);
        INIT_ISP_DRV_MODE(EIspProfile_ZsdPreview_CC, ISPDRV_MODE_CQ1_SYNC);
        INIT_ISP_DRV_MODE(EIspProfile_ZsdPreview_NCC, ISPDRV_MODE_CQ0);
        INIT_ISP_DRV_MODE(EIspProfile_NormalCapture, ISPDRV_MODE_CQ1_SYNC);
        INIT_ISP_DRV_MODE(EIspProfile_VideoPreview, ISPDRV_MODE_CQ1_SYNC);
        INIT_ISP_DRV_MODE(EIspProfile_VideoCapture, ISPDRV_MODE_CQ2_SYNC);
        INIT_ISP_DRV_MODE(EIspProfile_MFCapPass1, ISPDRV_MODE_CQ1_SYNC);
        INIT_ISP_DRV_MODE(EIspProfile_MFCapPass2, ISPDRV_MODE_CQ1_SYNC);
    }

    virtual ~ISP_MGR_CCM() {}

public:
    static MyType&  getInstance(ESensorDev_T const eSensorDev);

public: // Interfaces.

    MBOOL
    isEnable()
    {
        return m_bEnable;
    }

    MVOID
    setEnable(MBOOL bEnable)
    {
        m_bEnable = bEnable;
    }

    template <class ISP_xxx_T>
    MyType& put(ISP_xxx_T const& rParam);

    template <class ISP_xxx_T>
    MyType& get(ISP_xxx_T & rParam);

    MBOOL apply(EIspProfile_T eIspProfile);

} ISP_MGR_CCM_T;

template <ESensorDev_T const eSensorDev>
class ISP_MGR_CCM_DEV : public ISP_MGR_CCM_T
{
public:
    static
    ISP_MGR_CCM_T&
    getInstance()
    {
        static ISP_MGR_CCM_DEV<eSensorDev> singleton;
        return singleton;
    }
    virtual MVOID destroyInstance() {}

    ISP_MGR_CCM_DEV()
        : ISP_MGR_CCM_T()
    {}

    virtual ~ISP_MGR_CCM_DEV() {}

};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  GGM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef class ISP_MGR_GGM : public ISP_MGR_BASE_T
{
    typedef ISP_MGR_GGM    MyType;
private:
    MBOOL m_bEnable;
    MUINT32 m_u4StartAddr; // for debug purpose: 0x5000

    enum
    {
        ERegInfo_CAM_GGM_CTRL,
        ERegInfo_NUM
    };
    RegInfo_T   m_rIspRegInfo[ERegInfo_NUM];
    
    RegInfo_T   m_rIspRegInfo_GGM_RB[GGM_LUT_SIZE];
    RegInfo_T   m_rIspRegInfo_GGM_G[GGM_LUT_SIZE];
    MINT32 m_rIspDrvMode[EIspProfile_NUM];

protected:
    ISP_MGR_GGM()
        : ISP_MGR_BASE_T(m_rIspRegInfo, ERegInfo_NUM, m_u4StartAddr)
        , m_bEnable(MTRUE)
        , m_u4StartAddr(REG_ADDR(CAM_GGM_CTRL))
    {

        INIT_REG_INFO_ADDR(CAM_GGM_CTRL); // 0x5600

        // CAM_GGM_RB_GMT
        MUINT32 u4StartAddr = REG_ADDR(CAM_GGM_RB_GMT[0]);
        for (MINT32 i = 0; i < GGM_LUT_SIZE; i++) {
            m_rIspRegInfo_GGM_RB[i].addr = u4StartAddr + 4*i;
            //MY_LOG("m_rIspRegInfo_GGM_RB[%d].addr = 0x%8x", i, m_rIspRegInfo_GGM_RB[i].addr);
        }

        // CAM_GGM_G_GMT
        u4StartAddr = REG_ADDR(CAM_GGM_G_GMT[0]);
        for (MINT32 i = 0; i < GGM_LUT_SIZE; i++) {
            m_rIspRegInfo_GGM_G[i].addr = u4StartAddr + 4*i;
            //MY_LOG("m_rIspRegInfo_GGM_G[%d].addr = 0x%8x", i, m_rIspRegInfo_GGM_G[i].addr);
        }

        INIT_ISP_DRV_MODE(EIspProfile_NormalPreview, ISPDRV_MODE_CQ1_SYNC);
        INIT_ISP_DRV_MODE(EIspProfile_ZsdPreview_CC, ISPDRV_MODE_CQ1_SYNC);
        INIT_ISP_DRV_MODE(EIspProfile_ZsdPreview_NCC, ISPDRV_MODE_CQ0);
        INIT_ISP_DRV_MODE(EIspProfile_NormalCapture, ISPDRV_MODE_CQ1_SYNC);
        INIT_ISP_DRV_MODE(EIspProfile_VideoPreview, ISPDRV_MODE_CQ1_SYNC);
        INIT_ISP_DRV_MODE(EIspProfile_VideoCapture, ISPDRV_MODE_CQ2_SYNC);
        INIT_ISP_DRV_MODE(EIspProfile_MFCapPass1, ISPDRV_MODE_CQ1_SYNC);
        INIT_ISP_DRV_MODE(EIspProfile_MFCapPass2, ISPDRV_MODE_CQ1_SYNC);
    }

    virtual ~ISP_MGR_GGM() {}

public:
    static MyType&  getInstance(ESensorDev_T const eSensorDev);

public: // Interfaces.

    template <class ISP_xxx_T>
    MyType& put(ISP_xxx_T const& rParam);

    template <class ISP_xxx_T>
    MyType& get(ISP_xxx_T & rParam);

    MBOOL
    isEnable()
    {
        return m_bEnable;
    }

    MVOID
    setEnable(MBOOL bEnable)
    {
        m_bEnable = bEnable;
    }

    MBOOL apply(EIspProfile_T eIspProfile);

} ISP_MGR_GGM_T;

template <ESensorDev_T const eSensorDev>
class ISP_MGR_GGM_DEV : public ISP_MGR_GGM_T
{
public:
    static
    ISP_MGR_GGM_T&
    getInstance()
    {
        static ISP_MGR_GGM_DEV<eSensorDev> singleton;
        return singleton;
    }
    virtual MVOID destroyInstance() {}

    ISP_MGR_GGM_DEV()
        : ISP_MGR_GGM_T()
    {}

    virtual ~ISP_MGR_GGM_DEV() {}

};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  G2C
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef class ISP_MGR_G2C : public ISP_MGR_BASE_T
{
    typedef ISP_MGR_G2C    MyType;
private:
    MBOOL m_bEnable;
    MUINT32 m_u4StartAddr; // for debug purpose: 0x4A00

    enum
    {
        ERegInfo_CAM_G2C_CONV_0A,
        ERegInfo_CAM_G2C_CONV_0B,
        ERegInfo_CAM_G2C_CONV_1A,
        ERegInfo_CAM_G2C_CONV_1B,
        ERegInfo_CAM_G2C_CONV_2A,
        ERegInfo_CAM_G2C_CONV_2B,
        ERegInfo_NUM
    };
    RegInfo_T   m_rIspRegInfo[ERegInfo_NUM];
    MINT32 m_rIspDrvMode[EIspProfile_NUM];

protected:
    ISP_MGR_G2C()
        : ISP_MGR_BASE_T(m_rIspRegInfo, ERegInfo_NUM, m_u4StartAddr)
        , m_bEnable(MTRUE)
        , m_u4StartAddr(REG_ADDR(CAM_G2C_CONV_0A))
    {
        INIT_REG_INFO_ADDR(CAM_G2C_CONV_0A); // 0x4A00
        INIT_REG_INFO_ADDR(CAM_G2C_CONV_0B); // 0x4A04
        INIT_REG_INFO_ADDR(CAM_G2C_CONV_1A); // 0x4A08
        INIT_REG_INFO_ADDR(CAM_G2C_CONV_1B); // 0x4A0C
        INIT_REG_INFO_ADDR(CAM_G2C_CONV_2A); // 0x4A10
        INIT_REG_INFO_ADDR(CAM_G2C_CONV_2B); // 0x4A14

        INIT_ISP_DRV_MODE(EIspProfile_NormalPreview, ISPDRV_MODE_CQ1_SYNC);
        INIT_ISP_DRV_MODE(EIspProfile_ZsdPreview_CC, ISPDRV_MODE_CQ1_SYNC);
        INIT_ISP_DRV_MODE(EIspProfile_ZsdPreview_NCC, ISPDRV_MODE_CQ0);
        INIT_ISP_DRV_MODE(EIspProfile_NormalCapture, ISPDRV_MODE_CQ1_SYNC);
        INIT_ISP_DRV_MODE(EIspProfile_VideoPreview, ISPDRV_MODE_CQ1_SYNC);
        INIT_ISP_DRV_MODE(EIspProfile_VideoCapture, ISPDRV_MODE_CQ2_SYNC);
        INIT_ISP_DRV_MODE(EIspProfile_MFCapPass1, ISPDRV_MODE_CQ1_SYNC);
        INIT_ISP_DRV_MODE(EIspProfile_MFCapPass2, ISPDRV_MODE_CQ1_SYNC);
    }

    virtual ~ISP_MGR_G2C() {}

public:
    static MyType&  getInstance(ESensorDev_T const eSensorDev);

public: // Interfaces.

    MBOOL
    isEnable()
    {
        return m_bEnable;
    }

    MVOID
    setEnable(MBOOL bEnable)
    {
        m_bEnable = bEnable;
    }

    template <class ISP_xxx_T>
    MyType& put(ISP_xxx_T const& rParam);

    template <class ISP_xxx_T>
    MyType& get(ISP_xxx_T & rParam);

    MBOOL apply(EIspProfile_T eIspProfile);

} ISP_MGR_G2C_T;

template <ESensorDev_T const eSensorDev>
class ISP_MGR_G2C_DEV : public ISP_MGR_G2C_T
{
public:
    static
    ISP_MGR_G2C_T&
    getInstance()
    {
        static ISP_MGR_G2C_DEV<eSensorDev> singleton;
        return singleton;
    }
    virtual MVOID destroyInstance() {}

    ISP_MGR_G2C_DEV()
        : ISP_MGR_G2C_T()
    {}

    virtual ~ISP_MGR_G2C_DEV() {}

};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  NBC (ANR + CCR)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef class ISP_MGR_NBC : public ISP_MGR_BASE_T
{
    typedef ISP_MGR_NBC    MyType;
private:
    MBOOL m_bANREnable;
    MBOOL m_bCCREnable;
	MBOOL m_bCCREnBackup;
	MBOOL m_bANRENCBackup;
	MBOOL m_bANRENYBackup;
    MUINT32 m_u4StartAddr; // for debug purpose: 0x4A20

    enum
    {
        ERegInfo_CAM_ANR_CON1,
        ERegInfo_CAM_ANR_CON2,
        ERegInfo_CAM_ANR_CON3,
        ERegInfo_CAM_ANR_YAD1,
        ERegInfo_CAM_ANR_YAD2,
        ERegInfo_CAM_ANR_4LUT1,
        ERegInfo_CAM_ANR_4LUT2,
        ERegInfo_CAM_ANR_4LUT3,
        ERegInfo_CAM_ANR_PTY,
        ERegInfo_CAM_ANR_CAD,
        ERegInfo_CAM_ANR_PTC,
        ERegInfo_CAM_ANR_LCE1,
        ERegInfo_CAM_ANR_LCE2,
        ERegInfo_CAM_ANR_HP1,
        ERegInfo_CAM_ANR_HP2,
        ERegInfo_CAM_ANR_HP3,
        ERegInfo_CAM_ANR_ACTY,
        ERegInfo_CAM_ANR_ACTC,
        ERegInfo_CAM_CCR_CON,
        ERegInfo_CAM_CCR_YLUT,
        ERegInfo_CAM_CCR_UVLUT,
        ERegInfo_CAM_CCR_YLUT2,
        ERegInfo_NUM
    };
    RegInfo_T   m_rIspRegInfo[ERegInfo_NUM];
    MINT32 m_rIspDrvMode[EIspProfile_NUM];

protected:
    ISP_MGR_NBC()
        : ISP_MGR_BASE_T(m_rIspRegInfo, ERegInfo_NUM, m_u4StartAddr)
        , m_bANREnable(MTRUE)
        , m_bCCREnable(MTRUE)
        , m_bCCREnBackup(MFALSE)
        , m_bANRENCBackup(MFALSE)
        , m_bANRENYBackup(MFALSE)
        , m_u4StartAddr(REG_ADDR(CAM_ANR_CON1))
    {
        INIT_REG_INFO_ADDR(CAM_ANR_CON1);  // 0x4A20
        INIT_REG_INFO_ADDR(CAM_ANR_CON2);  // 0x4A24
        INIT_REG_INFO_ADDR(CAM_ANR_CON3);  // 0x4A28
        INIT_REG_INFO_ADDR(CAM_ANR_YAD1);  // 0x4A2C
        INIT_REG_INFO_ADDR(CAM_ANR_YAD2);  // 0x4A30
        INIT_REG_INFO_ADDR(CAM_ANR_4LUT1); // 0x4A34
        INIT_REG_INFO_ADDR(CAM_ANR_4LUT2); // 0x4A38
        INIT_REG_INFO_ADDR(CAM_ANR_4LUT3); // 0x4A3C
        INIT_REG_INFO_ADDR(CAM_ANR_PTY);   // 0x4A40
        INIT_REG_INFO_ADDR(CAM_ANR_CAD);   // 0x4A44
        INIT_REG_INFO_ADDR(CAM_ANR_PTC);   // 0x4A48
        INIT_REG_INFO_ADDR(CAM_ANR_LCE1);  // 0x4A4C
        INIT_REG_INFO_ADDR(CAM_ANR_LCE2);  // 0x4A50
        INIT_REG_INFO_ADDR(CAM_ANR_HP1);   // 0x4A54
        INIT_REG_INFO_ADDR(CAM_ANR_HP2);   // 0x4A58
        INIT_REG_INFO_ADDR(CAM_ANR_HP3);   // 0x4A5C
        INIT_REG_INFO_ADDR(CAM_ANR_ACTY);  // 0x4A60
        INIT_REG_INFO_ADDR(CAM_ANR_ACTC);  // 0x4A64
        INIT_REG_INFO_ADDR(CAM_CCR_CON);  // 0x4A90
        INIT_REG_INFO_ADDR(CAM_CCR_YLUT); // 0x4A94
        INIT_REG_INFO_ADDR(CAM_CCR_UVLUT); // 0x4A98
        INIT_REG_INFO_ADDR(CAM_CCR_YLUT2); // 0x4A9C

        INIT_ISP_DRV_MODE(EIspProfile_NormalPreview, ISPDRV_MODE_CQ1_SYNC);
        INIT_ISP_DRV_MODE(EIspProfile_ZsdPreview_CC, ISPDRV_MODE_CQ1_SYNC);
        INIT_ISP_DRV_MODE(EIspProfile_ZsdPreview_NCC, ISPDRV_MODE_CQ0);
        INIT_ISP_DRV_MODE(EIspProfile_NormalCapture, ISPDRV_MODE_CQ1_SYNC);
        INIT_ISP_DRV_MODE(EIspProfile_VideoPreview, ISPDRV_MODE_CQ1_SYNC);
        INIT_ISP_DRV_MODE(EIspProfile_VideoCapture, ISPDRV_MODE_CQ2_SYNC);
        INIT_ISP_DRV_MODE(EIspProfile_MFCapPass1, ISPDRV_MODE_CQ1_SYNC);
        INIT_ISP_DRV_MODE(EIspProfile_MFCapPass2, ISPDRV_MODE_CQ1_SYNC);
    }

    virtual ~ISP_MGR_NBC() {}

public:
    static MyType&  getInstance(ESensorDev_T const eSensorDev);

public: // Interfaces.

    template <class ISP_xxx_T>
    MyType& put(ISP_xxx_T const& rParam);

    template <class ISP_xxx_T>
    MyType& get(ISP_xxx_T & rParam);

    MBOOL
    isANREnable()
    {
        return m_bANREnable;
    }

    MBOOL
    isCCREnable()
    {
        return m_bCCREnable;
    }

    MVOID
    setANREnable(MBOOL bEnable)
    {
        m_bANREnable = bEnable;
    }

    MVOID
    setCCREnable(MBOOL bEnable)
    {
        m_bCCREnable = bEnable;
    }

    MBOOL apply(EIspProfile_T eIspProfile);

} ISP_MGR_NBC_T;

template <ESensorDev_T const eSensorDev>
class ISP_MGR_NBC_DEV : public ISP_MGR_NBC_T
{
public:
    static
    ISP_MGR_NBC_T&
    getInstance()
    {
        static ISP_MGR_NBC_DEV<eSensorDev> singleton;
        return singleton;
    }
    virtual MVOID destroyInstance() {}

    ISP_MGR_NBC_DEV()
        : ISP_MGR_NBC_T()
    {}

    virtual ~ISP_MGR_NBC_DEV() {}

};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  PCA
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef class ISP_MGR_PCA : public ISP_MGR_BASE_T
{
    typedef ISP_MGR_PCA    MyType;
private:
    MBOOL m_bEnable;
    MUINT32 m_u4StartAddr; // for debug purpose: 0x5E00

    enum
    {
        ERegInfo_CAM_PCA_CON1,
        ERegInfo_CAM_PCA_CON2,
        ERegInfo_PCA_CON_NUM
    };
    RegInfo_T   m_rIspRegInfo[ERegInfo_PCA_CON_NUM]; // PCA_CON
    RegInfo_T   m_rIspRegInfo_PCA_LUT[PCA_BIN_NUM];
    MINT32 m_rIspDrvMode[EIspProfile_NUM];

protected:
    ISP_MGR_PCA()
        : ISP_MGR_BASE_T(m_rIspRegInfo, ERegInfo_PCA_CON_NUM, m_u4StartAddr)
        , m_bEnable(MTRUE)
        , m_u4StartAddr(REG_ADDR(CAM_PCA_CON1))
    {
        // PCA_CON
        INIT_REG_INFO_ADDR(CAM_PCA_CON1); // 0x5E00
        INIT_REG_INFO_ADDR(CAM_PCA_CON2); // 0x5E04

        // PCA_LUT
        MUINT32 u4StartAddr = REG_ADDR(CAM_PCA_TBL[0]);
        for (MINT32 i = 0; i < PCA_BIN_NUM; i++) {
            m_rIspRegInfo_PCA_LUT[i].addr = u4StartAddr + 4*i;
            //MY_LOG("m_rIspRegInfo_PCA_LUT[%d].addr = 0x%8x", i, m_rIspRegInfo_PCA_LUT[i].addr);
        }

        INIT_ISP_DRV_MODE(EIspProfile_NormalPreview, ISPDRV_MODE_CQ1_SYNC);
        INIT_ISP_DRV_MODE(EIspProfile_ZsdPreview_CC, ISPDRV_MODE_CQ1_SYNC);
        INIT_ISP_DRV_MODE(EIspProfile_ZsdPreview_NCC, ISPDRV_MODE_CQ0);
        INIT_ISP_DRV_MODE(EIspProfile_NormalCapture, ISPDRV_MODE_CQ1_SYNC);
        INIT_ISP_DRV_MODE(EIspProfile_VideoPreview, ISPDRV_MODE_CQ1_SYNC);
        INIT_ISP_DRV_MODE(EIspProfile_VideoCapture, ISPDRV_MODE_CQ2_SYNC);
        INIT_ISP_DRV_MODE(EIspProfile_MFCapPass1, ISPDRV_MODE_CQ1_SYNC);
        INIT_ISP_DRV_MODE(EIspProfile_MFCapPass2, ISPDRV_MODE_CQ1_SYNC);
    }

    virtual ~ISP_MGR_PCA() {}

public:
    static MyType&  getInstance(ESensorDev_T const eSensorDev);

public: // Interfaces.

    template <class ISP_xxx_T>
    MyType& put(ISP_xxx_T const& rParam);

    template <class ISP_xxx_T>
    MyType& get(ISP_xxx_T & rParam);

    MBOOL
    isEnable()
    {
        return m_bEnable;
    }

    MVOID
    setEnable(MBOOL bEnable)
    {
        m_bEnable = bEnable;
    }

    MBOOL apply(EIspProfile_T eIspProfile);

    inline MVOID loadLut(MUINT32* pPcaLut)
    {
        for (MINT32 i = 0; i < PCA_BIN_NUM; i++) {
            m_rIspRegInfo_PCA_LUT[i].val = pPcaLut[i];
        }
    }

    inline MVOID getLut(MUINT32* pPcaLut)
    {
        for (MINT32 i = 0; i < PCA_BIN_NUM; i++) {
            pPcaLut[i] = m_rIspRegInfo_PCA_LUT[i].val;
        }
    }

} ISP_MGR_PCA_T;

template <ESensorDev_T const eSensorDev>
class ISP_MGR_PCA_DEV : public ISP_MGR_PCA_T
{
public:
    static
    ISP_MGR_PCA_T&
    getInstance()
    {
        static ISP_MGR_PCA_DEV<eSensorDev> singleton;
        return singleton;
    }
    virtual MVOID destroyInstance() {}

    ISP_MGR_PCA_DEV()
        : ISP_MGR_PCA_T()
    {}

    virtual ~ISP_MGR_PCA_DEV() {}

};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  SEEE (SE + EE)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef class ISP_MGR_SEEE : public ISP_MGR_BASE_T
{
    typedef ISP_MGR_SEEE    MyType;
private:
    MBOOL m_bEnable;
    MUINT32 m_u4StartAddr; // for debug purpose: 0x4AA0

    enum
    {
        ERegInfo_CAM_SEEE_SRK_CTRL,
        ERegInfo_CAM_SEEE_CLIP_CTRL,
        ERegInfo_CAM_SEEE_HP_CTRL1,
        ERegInfo_CAM_SEEE_HP_CTRL2,
        ERegInfo_CAM_SEEE_ED_CTRL1,
        ERegInfo_CAM_SEEE_ED_CTRL2,
        ERegInfo_CAM_SEEE_ED_CTRL3,
        ERegInfo_CAM_SEEE_ED_CTRL4,
        ERegInfo_CAM_SEEE_ED_CTRL5,
        ERegInfo_CAM_SEEE_ED_CTRL6,
        ERegInfo_CAM_SEEE_ED_CTRL7,
        ERegInfo_CAM_SEEE_EDGE_CTRL,
        ERegInfo_CAM_SEEE_Y_CTRL,
        ERegInfo_CAM_SEEE_EDGE_CTRL1,
        ERegInfo_CAM_SEEE_EDGE_CTRL2,
        ERegInfo_CAM_SEEE_EDGE_CTRL3,
        ERegInfo_CAM_SEEE_SPECIAL_CTRL,
        ERegInfo_CAM_SEEE_CORE_CTRL1,
        ERegInfo_CAM_SEEE_CORE_CTRL2,
        ERegInfo_CAM_SEEE_EE_LINK1,
        ERegInfo_CAM_SEEE_EE_LINK2,
        ERegInfo_CAM_SEEE_EE_LINK3,
        ERegInfo_CAM_SEEE_EE_LINK4,
        ERegInfo_CAM_SEEE_EE_LINK5,
        ERegInfo_NUM
    };
    RegInfo_T   m_rIspRegInfo[ERegInfo_NUM];
    MINT32 m_rIspDrvMode[EIspProfile_NUM];

protected:
    ISP_MGR_SEEE()
        : ISP_MGR_BASE_T(m_rIspRegInfo, ERegInfo_NUM, m_u4StartAddr)
        , m_bEnable(MTRUE)
        , m_u4StartAddr(REG_ADDR(CAM_SEEE_SRK_CTRL))
    {
        INIT_REG_INFO_ADDR(CAM_SEEE_SRK_CTRL);  // 0x4AA0
        INIT_REG_INFO_ADDR(CAM_SEEE_CLIP_CTRL); // 0x4AA4
        INIT_REG_INFO_ADDR(CAM_SEEE_HP_CTRL1);  // 0x4AA8
        INIT_REG_INFO_ADDR(CAM_SEEE_HP_CTRL2);  // 0x4AAC
        INIT_REG_INFO_ADDR(CAM_SEEE_ED_CTRL1);  // 0x4AB0
        INIT_REG_INFO_ADDR(CAM_SEEE_ED_CTRL2);  // 0x4AB4
        INIT_REG_INFO_ADDR(CAM_SEEE_ED_CTRL3);  // 0x4AB8
        INIT_REG_INFO_ADDR(CAM_SEEE_ED_CTRL4);  // 0x4ABC
        INIT_REG_INFO_ADDR(CAM_SEEE_ED_CTRL5);  // 0x4AC0
        INIT_REG_INFO_ADDR(CAM_SEEE_ED_CTRL6);  // 0x4AC4
        INIT_REG_INFO_ADDR(CAM_SEEE_ED_CTRL7);  // 0x4AC8

        INIT_REG_INFO_ADDR(CAM_SEEE_EDGE_CTRL);    // 0x4ACC
        INIT_REG_INFO_ADDR(CAM_SEEE_Y_CTRL);       // 0x4AD0
        INIT_REG_INFO_ADDR(CAM_SEEE_EDGE_CTRL1);   // 0x4AD4
        INIT_REG_INFO_ADDR(CAM_SEEE_EDGE_CTRL2);   // 0x4AD8
        INIT_REG_INFO_ADDR(CAM_SEEE_EDGE_CTRL3);   // 0x4ADC
        INIT_REG_INFO_ADDR(CAM_SEEE_SPECIAL_CTRL); // 0x4AE0
        INIT_REG_INFO_ADDR(CAM_SEEE_CORE_CTRL1);   // 0x4AE4
        INIT_REG_INFO_ADDR(CAM_SEEE_CORE_CTRL2);   // 0x4AE8
        INIT_REG_INFO_ADDR(CAM_SEEE_EE_LINK1);     // 0x4AEC
        INIT_REG_INFO_ADDR(CAM_SEEE_EE_LINK2);     // 0x4AF0
        INIT_REG_INFO_ADDR(CAM_SEEE_EE_LINK3);     // 0x4AF4
        INIT_REG_INFO_ADDR(CAM_SEEE_EE_LINK4);     // 0x4AF8
        INIT_REG_INFO_ADDR(CAM_SEEE_EE_LINK5);     // 0x4AFC

        INIT_ISP_DRV_MODE(EIspProfile_NormalPreview, ISPDRV_MODE_CQ1_SYNC);
        INIT_ISP_DRV_MODE(EIspProfile_ZsdPreview_CC, ISPDRV_MODE_CQ1_SYNC);
        INIT_ISP_DRV_MODE(EIspProfile_ZsdPreview_NCC, ISPDRV_MODE_CQ0);
        INIT_ISP_DRV_MODE(EIspProfile_NormalCapture, ISPDRV_MODE_CQ1_SYNC);
        INIT_ISP_DRV_MODE(EIspProfile_VideoPreview, ISPDRV_MODE_CQ1_SYNC);
        INIT_ISP_DRV_MODE(EIspProfile_VideoCapture, ISPDRV_MODE_CQ2_SYNC);
        INIT_ISP_DRV_MODE(EIspProfile_MFCapPass1, ISPDRV_MODE_CQ1_SYNC);
        INIT_ISP_DRV_MODE(EIspProfile_MFCapPass2, ISPDRV_MODE_CQ1_SYNC);
    }

    virtual ~ISP_MGR_SEEE() {}

public:
    static MyType&  getInstance(ESensorDev_T const eSensorDev);

public: // Interfaces.

    template <class ISP_xxx_T>
    MyType& put(ISP_xxx_T const& rParam);

    template <class ISP_xxx_T>
    MyType& get(ISP_xxx_T & rParam);

    MBOOL
    isEnable()
    {
        return m_bEnable;
    }

    MVOID
    setEnable(MBOOL bEnable)
    {
        m_bEnable = bEnable;
    }

    MBOOL apply(EIspProfile_T eIspProfile);

} ISP_MGR_SEEE_T;

template <ESensorDev_T const eSensorDev>
class ISP_MGR_SEEE_DEV : public ISP_MGR_SEEE_T
{
public:
    static
    ISP_MGR_SEEE_T&
    getInstance()
    {
        static ISP_MGR_SEEE_DEV<eSensorDev> singleton;
        return singleton;
    }
    virtual MVOID destroyInstance() {}

    ISP_MGR_SEEE_DEV()
        : ISP_MGR_SEEE_T()
    {}

    virtual ~ISP_MGR_SEEE_DEV() {}

};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  AWB statistics config
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef class ISP_MGR_AWB_STAT_CONFIG : public ISP_MGR_BASE_T
{
    typedef ISP_MGR_AWB_STAT_CONFIG    MyType;
private:
    MUINT32 m_u4StartAddr; // for debug purpose: 0x45B0

    enum
    {
        ERegInfo_CAM_AWB_WIN_ORG,
        ERegInfo_CAM_AWB_WIN_SIZE,
        ERegInfo_CAM_AWB_WIN_PITCH,
        ERegInfo_CAM_AWB_WIN_NUM,
        ERegInfo_CAM_AWB_RAWPREGAIN1_0,
        ERegInfo_CAM_AWB_RAWPREGAIN1_1,
        ERegInfo_CAM_AWB_RAWLIMIT1_0,
        ERegInfo_CAM_AWB_RAWLIMIT1_1,
        ERegInfo_CAM_AWB_LOW_THR,
        ERegInfo_CAM_AWB_HI_THR,
        ERegInfo_CAM_AWB_PIXEL_CNT0,
        ERegInfo_CAM_AWB_PIXEL_CNT1,
        ERegInfo_CAM_AWB_PIXEL_CNT2,
        ERegInfo_CAM_AWB_ERR_THR,
        ERegInfo_CAM_AWB_ROT,
        ERegInfo_CAM_AWB_L0_X,
        ERegInfo_CAM_AWB_L0_Y,
        ERegInfo_CAM_AWB_L1_X,
        ERegInfo_CAM_AWB_L1_Y,
        ERegInfo_CAM_AWB_L2_X,
        ERegInfo_CAM_AWB_L2_Y,
        ERegInfo_CAM_AWB_L3_X,
        ERegInfo_CAM_AWB_L3_Y,
        ERegInfo_CAM_AWB_L4_X,
        ERegInfo_CAM_AWB_L4_Y,
        ERegInfo_CAM_AWB_L5_X,
        ERegInfo_CAM_AWB_L5_Y,
        ERegInfo_CAM_AWB_L6_X,
        ERegInfo_CAM_AWB_L6_Y,
        ERegInfo_CAM_AWB_L7_X,
        ERegInfo_CAM_AWB_L7_Y,
        ERegInfo_CAM_AWB_L8_X,
        ERegInfo_CAM_AWB_L8_Y,
        ERegInfo_CAM_AWB_L9_X,
        ERegInfo_CAM_AWB_L9_Y,
        ERegInfo_NUM
    };
    RegInfo_T   m_rIspRegInfo[ERegInfo_NUM];

protected:
    ISP_MGR_AWB_STAT_CONFIG()
        : ISP_MGR_BASE_T(m_rIspRegInfo, ERegInfo_NUM, m_u4StartAddr)
        , m_u4StartAddr(REG_ADDR(CAM_AWB_WIN_ORG))
    {
        INIT_REG_INFO_ADDR(CAM_AWB_WIN_ORG);       // 0x45B0
        INIT_REG_INFO_ADDR(CAM_AWB_WIN_SIZE);      // 0x45B4
        INIT_REG_INFO_ADDR(CAM_AWB_WIN_PITCH);     // 0x45B8
        INIT_REG_INFO_ADDR(CAM_AWB_WIN_NUM);       // 0x45BC
        INIT_REG_INFO_ADDR(CAM_AWB_RAWPREGAIN1_0); // 0x45C0
        INIT_REG_INFO_ADDR(CAM_AWB_RAWPREGAIN1_1); // 0x45C4
        INIT_REG_INFO_ADDR(CAM_AWB_RAWLIMIT1_0);   // 0x45C8
        INIT_REG_INFO_ADDR(CAM_AWB_RAWLIMIT1_1);   // 0x45CC
        INIT_REG_INFO_ADDR(CAM_AWB_LOW_THR);       // 0x45D0
        INIT_REG_INFO_ADDR(CAM_AWB_HI_THR);        // 0x45D4
        INIT_REG_INFO_ADDR(CAM_AWB_PIXEL_CNT0);    // 0x45D8
        INIT_REG_INFO_ADDR(CAM_AWB_PIXEL_CNT1);    // 0x45DC
        INIT_REG_INFO_ADDR(CAM_AWB_PIXEL_CNT2);    // 0x45E0
        INIT_REG_INFO_ADDR(CAM_AWB_ERR_THR);       // 0x45E4
        INIT_REG_INFO_ADDR(CAM_AWB_ROT);           // 0x45E8
        INIT_REG_INFO_ADDR(CAM_AWB_L0_X);          // 0x45EC
        INIT_REG_INFO_ADDR(CAM_AWB_L0_Y);          // 0x45F0
        INIT_REG_INFO_ADDR(CAM_AWB_L1_X);          // 0x45F4
        INIT_REG_INFO_ADDR(CAM_AWB_L1_Y);          // 0x45F8
        INIT_REG_INFO_ADDR(CAM_AWB_L2_X);          // 0x45FC
        INIT_REG_INFO_ADDR(CAM_AWB_L2_Y);          // 0x4600
        INIT_REG_INFO_ADDR(CAM_AWB_L3_X);          // 0x4604
        INIT_REG_INFO_ADDR(CAM_AWB_L3_Y);          // 0x4608
        INIT_REG_INFO_ADDR(CAM_AWB_L4_X);          // 0x460C
        INIT_REG_INFO_ADDR(CAM_AWB_L4_Y);          // 0x4610
        INIT_REG_INFO_ADDR(CAM_AWB_L5_X);          // 0x4614
        INIT_REG_INFO_ADDR(CAM_AWB_L5_Y);          // 0x4618
        INIT_REG_INFO_ADDR(CAM_AWB_L6_X);          // 0x461C
        INIT_REG_INFO_ADDR(CAM_AWB_L6_Y);          // 0x4620
        INIT_REG_INFO_ADDR(CAM_AWB_L7_X);          // 0x4624
        INIT_REG_INFO_ADDR(CAM_AWB_L7_Y);          // 0x4628
        INIT_REG_INFO_ADDR(CAM_AWB_L8_X);          // 0x462C
        INIT_REG_INFO_ADDR(CAM_AWB_L8_Y);          // 0x4630
        INIT_REG_INFO_ADDR(CAM_AWB_L9_X);          // 0x4634
        INIT_REG_INFO_ADDR(CAM_AWB_L9_Y);          // 0x4638
    }

    virtual ~ISP_MGR_AWB_STAT_CONFIG() {}

public: ////
    static MyType&  getInstance(ESensorDev_T const eSensorDev);

public: //    Interfaces
    MBOOL config(AWB_STAT_CONFIG_T& rAWBStatConfig);

    MBOOL apply();

} ISP_MGR_AWB_STAT_CONFIG_T;

template <ESensorDev_T const eSensorDev>
class ISP_MGR_AWB_STAT_CONFIG_DEV : public ISP_MGR_AWB_STAT_CONFIG_T
{
public:
    static
    ISP_MGR_AWB_STAT_CONFIG_T&
    getInstance()
    {
        static ISP_MGR_AWB_STAT_CONFIG_DEV<eSensorDev> singleton;
        return singleton;
    }
    virtual MVOID destroyInstance() {}

    ISP_MGR_AWB_STAT_CONFIG_DEV()
        : ISP_MGR_AWB_STAT_CONFIG_T()
    {}

    virtual ~ISP_MGR_AWB_STAT_CONFIG_DEV() {}

};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  FLK statistics
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#if 1
typedef class ISP_MGR_FLK_CONFIG : public ISP_MGR_BASE_T
{
    typedef ISP_MGR_FLK_CONFIG    MyType;
private:
    MUINT32 m_u4StartAddr; // for debug purpose: 0x4770

    enum
    {
        ERegInfo_CAM_FLK_CON,
        ERegInfo_CAM_CTL_EN1,
        ERegInfo_CAM_CTL_EN1_SET,
        ERegInfo_CAM_CTL_DMA_EN,
        ERegInfo_CAM_CTL_DMA_EN_SET,
        ERegInfo_CAM_CTL_DMA_INT,
        ERegInfo_CAM_CTL_INT_EN,
        ERegInfo_CAM_FLK_WNUM,
        ERegInfo_CAM_FLK_SOFST,
        ERegInfo_CAM_FLK_WSIZE,
        ERegInfo_CAM_ESFKO_XSIZE,
        ERegInfo_CAM_ESFKO_YSIZE,
        ERegInfo_CAM_ESFKO_STRIDE,
        ERegInfo_CAM_ESFKO_OFST_ADDR,
        ERegInfo_CAM_ESFKO_BASE_ADDR,
        ERegInfo_NUM
    };
    RegInfo_T   m_rIspRegInfo[ERegInfo_NUM];

protected:
    ISP_MGR_FLK_CONFIG()
        : ISP_MGR_BASE_T(m_rIspRegInfo, ERegInfo_NUM, m_u4StartAddr)
        , m_u4StartAddr(REG_ADDR(CAM_FLK_CON))
    {
        INIT_REG_INFO_ADDR(CAM_FLK_CON);          // 0x4770
        INIT_REG_INFO_ADDR(CAM_CTL_EN1);          // 0x4004
        INIT_REG_INFO_ADDR(CAM_CTL_EN1_SET);      // 0x4080
        INIT_REG_INFO_ADDR(CAM_CTL_DMA_EN);       // 0x400C
        INIT_REG_INFO_ADDR(CAM_CTL_DMA_EN_SET);   //0x 4090
        INIT_REG_INFO_ADDR(CAM_CTL_DMA_INT);      // 0x4028
        INIT_REG_INFO_ADDR(CAM_CTL_INT_EN);      // 0x4020
        INIT_REG_INFO_ADDR(CAM_FLK_WNUM);         // 0x477C
        INIT_REG_INFO_ADDR(CAM_FLK_SOFST);        // 0x4774
        INIT_REG_INFO_ADDR(CAM_FLK_WSIZE);        // 0x4778
        INIT_REG_INFO_ADDR(CAM_ESFKO_XSIZE);      // 0x4370
        INIT_REG_INFO_ADDR(CAM_ESFKO_YSIZE);      // 0x4378
        INIT_REG_INFO_ADDR(CAM_ESFKO_STRIDE);     // 0x437C
        INIT_REG_INFO_ADDR(CAM_ESFKO_OFST_ADDR);  // 0x4374
        INIT_REG_INFO_ADDR(CAM_ESFKO_BASE_ADDR);  // 0x436C
    }

    virtual ~ISP_MGR_FLK_CONFIG() {}

public: ////
    static MyType&  getInstance(ESensorDev_T const eSensorDev);

public: //    Interfaces

    MBOOL apply();
    MVOID enableFlk(MBOOL enable);
    MVOID SetFLKWin(MINT32 offsetX,MINT32 offsetY , MINT32 sizeX ,MINT32 sizeY);
    MVOID SetFKO_DMA_Addr(MINT32 address,MINT32 size);

}ISP_MGR_FLK_CONFIG_T;
template <ESensorDev_T const eSensorDev>
class ISP_MGR_FLK_DEV : public ISP_MGR_FLK_CONFIG_T
{
public:
    static
    ISP_MGR_FLK_CONFIG_T&
    getInstance()
    {
        static ISP_MGR_FLK_DEV<eSensorDev> singleton;
        return singleton;
    }
    virtual MVOID destroyInstance() {}

    ISP_MGR_FLK_DEV()
        : ISP_MGR_FLK_CONFIG_T()
    {}

    virtual ~ISP_MGR_FLK_DEV() {}

};

#endif

/////////////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////////////



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  AE RAW Pre-gain2
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef class ISP_MGR_AE_RAWPREGAIN2 : public ISP_MGR_BASE_T
{
    typedef ISP_MGR_AE_RAWPREGAIN2    MyType;
private:
    MUINT32 m_u4StartAddr; // for debug purpose: 0x4654

    enum
    {
        ERegInfo_CAM_AE_RAWPREGAIN2_0,
        ERegInfo_CAM_AE_RAWPREGAIN2_1,
        ERegInfo_NUM
    };
    RegInfo_T   m_rIspRegInfo[ERegInfo_NUM];

protected:
    ISP_MGR_AE_RAWPREGAIN2()
        : ISP_MGR_BASE_T(m_rIspRegInfo, ERegInfo_NUM, m_u4StartAddr)
        , m_u4StartAddr(REG_ADDR(CAM_AE_RAWPREGAIN2_0))
    {
        INIT_REG_INFO_ADDR(CAM_AE_RAWPREGAIN2_0); // 0x4654
        INIT_REG_INFO_ADDR(CAM_AE_RAWPREGAIN2_1); // 0x4658
    }

    virtual ~ISP_MGR_AE_RAWPREGAIN2() {}

public: ////
    static MyType&  getInstance(ESensorDev_T const eSensorDev);

public: //    Interfaces

    MBOOL setRAWPregain2(AWB_GAIN_T& rAWBRAWPregain2);

    MBOOL apply();

} ISP_MGR_AE_RAWPREGAIN2_T;

template <ESensorDev_T const eSensorDev>
class ISP_MGR_AE_RAWPREGAIN2_DEV : public ISP_MGR_AE_RAWPREGAIN2_T
{
public:
    static
    ISP_MGR_AE_RAWPREGAIN2_T&
    getInstance()
    {
        static ISP_MGR_AE_RAWPREGAIN2_DEV<eSensorDev> singleton;
        return singleton;
    }
    virtual MVOID destroyInstance() {}

    ISP_MGR_AE_RAWPREGAIN2_DEV()
        : ISP_MGR_AE_RAWPREGAIN2_T()
    {}

    virtual ~ISP_MGR_AE_RAWPREGAIN2_DEV() {}

};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  AE statistics and histogram config
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef class ISP_MGR_AE_STAT_HIST_CONFIG : public ISP_MGR_BASE_T
{
    typedef ISP_MGR_AE_STAT_HIST_CONFIG    MyType;
private:
    MUINT32 m_u4StartAddr; // for debug purpose: 0x465C
    enum
    {
        ERegInfo_CAM_AE_RAWLIMIT2_0,
        ERegInfo_CAM_AE_RAWLIMIT2_1,
        ERegInfo_CAM_AE_MATRIX_COEF0,
        ERegInfo_CAM_AE_MATRIX_COEF1,
        ERegInfo_CAM_AE_MATRIX_COEF2,
        ERegInfo_CAM_AE_MATRIX_COEF3,
        ERegInfo_CAM_AE_MATRIX_COEF4,
        ERegInfo_CAM_AE_YGAMMA_0,
        ERegInfo_CAM_AE_YGAMMA_1,
        ERegInfo_CAM_AE_HST_SET,
        ERegInfo_CAM_AE_HST0_RNG,
        ERegInfo_CAM_AE_HST1_RNG,
        ERegInfo_CAM_AE_HST2_RNG,
        ERegInfo_CAM_AE_HST3_RNG,
        ERegInfo_NUM
    };
    RegInfo_T   m_rIspRegInfo[ERegInfo_NUM];

protected:
    ISP_MGR_AE_STAT_HIST_CONFIG()
        : ISP_MGR_BASE_T(m_rIspRegInfo, ERegInfo_NUM, m_u4StartAddr)
        , m_u4StartAddr(REG_ADDR(CAM_AE_RAWLIMIT2_0))
    {
        INIT_REG_INFO_ADDR(CAM_AE_RAWLIMIT2_0);       // 0x465C
        INIT_REG_INFO_ADDR(CAM_AE_RAWLIMIT2_1);       // 0x4660
        INIT_REG_INFO_ADDR(CAM_AE_MATRIX_COEF0);     // 0x4664
        INIT_REG_INFO_ADDR(CAM_AE_MATRIX_COEF1);     // 0x4668
        INIT_REG_INFO_ADDR(CAM_AE_MATRIX_COEF2);     // 0x466C
        INIT_REG_INFO_ADDR(CAM_AE_MATRIX_COEF3);     // 0x4670
        INIT_REG_INFO_ADDR(CAM_AE_MATRIX_COEF4);     // 0x4674
        INIT_REG_INFO_ADDR(CAM_AE_YGAMMA_0);            // 0x4678
        INIT_REG_INFO_ADDR(CAM_AE_YGAMMA_1);            // 0x467C
        INIT_REG_INFO_ADDR(CAM_AE_HST_SET);               // 0x4680
        INIT_REG_INFO_ADDR(CAM_AE_HST0_RNG);            // 0x4684
        INIT_REG_INFO_ADDR(CAM_AE_HST1_RNG);            // 0x4688
        INIT_REG_INFO_ADDR(CAM_AE_HST2_RNG);             // 0x468C
        INIT_REG_INFO_ADDR(CAM_AE_HST3_RNG);             // 0x4690
    }

    virtual ~ISP_MGR_AE_STAT_HIST_CONFIG() {}

public: ////
    static MyType&  getInstance(ESensorDev_T const eSensorDev);

public: //    Interfaces
    MBOOL config(AE_STAT_PARAM_T &rAEStatConfig);

    MBOOL apply();

} ISP_MGR_AE_STAT_HIST_CONFIG_T;

template <ESensorDev_T const eSensorDev>
class ISP_MGR_AE_STAT_HIST_CONFIG_DEV : public ISP_MGR_AE_STAT_HIST_CONFIG_T
{
public:
    static
    ISP_MGR_AE_STAT_HIST_CONFIG_T&
    getInstance()
    {
        static ISP_MGR_AE_STAT_HIST_CONFIG_DEV<eSensorDev> singleton;
        return singleton;
    }
    virtual MVOID destroyInstance() {}

    ISP_MGR_AE_STAT_HIST_CONFIG_DEV()
        : ISP_MGR_AE_STAT_HIST_CONFIG_T()
    {}

    virtual ~ISP_MGR_AE_STAT_HIST_CONFIG_DEV() {}

};

class IspDebug
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Ctor/Dtor.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:    ////    Disallowed.
    //  Copy constructor is disallowed.
    IspDebug(IspDebug const&);
    //  Copy-assignment operator is disallowed.
    IspDebug& operator=(IspDebug const&);

public:  ////
    IspDebug();
    ~IspDebug() {}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    static IspDebug& getInstance();
    MBOOL init();
    MBOOL uninit();
    MBOOL dumpIspDebugMessage();
    MUINT32 readLsciAddr();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data member: please follow the order of member initialization list in constructor
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    IspDrv*     m_pIspDrv;
    isp_reg_t*  m_pIspReg;
    volatile MINT32        m_Users;
    mutable android::Mutex m_Lock;
    MBOOL       m_bDebugEnable;
};

};  //  namespace NSIspTuning

#endif // _ISP_MGR_H_

