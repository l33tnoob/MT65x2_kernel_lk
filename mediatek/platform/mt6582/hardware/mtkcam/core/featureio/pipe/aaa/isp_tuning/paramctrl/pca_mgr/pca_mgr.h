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
#ifndef _PCA_MGR_H_
#define _PCA_MGR_H_

namespace NSIspTuning
{

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// PCA Manager
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class PcaMgr
{
public:
    static PcaMgr* createInstance(ESensorDev_T const eSensorDev, ISP_NVRAM_PCA_STRUCT& rIspPca);
    virtual MVOID destroyInstance() = 0;

private:
    enum
    {
        LUT_LOW = 0,
        LUT_MIDDLE,
        LUT_HIGH,
        NUM_OF_LUTS
    };

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Change Count.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////

    inline
    MBOOL
    isChanged() const
    {
        return  ( 0 < m_u4ChangeCount );
    }

private:    ////
    inline
    MVOID
    markChange()
    {
        m_u4ChangeCount++;
    }

    template <typename T>
    inline
    MVOID
    setIfChange(T& dst, T const src)
    {
        if  ( src != dst )
        {
            dst = src;
            markChange();
        }
    }

private:    ////    Data Members.
    //  Nonzero indicates any member has changed; otherwise zero.
    MUINT32                 m_u4ChangeCount;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Index
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////

    inline
    MUINT32
    getIdx() const
    {
        return m_u4Idx;
    }

    inline
    MBOOL
    setIdx(MUINT32 const u4Idx)
    {
        if  ( NUM_OF_LUTS <= u4Idx )
            return  MFALSE;
        setIfChange(m_u4Idx, u4Idx);
        return  MTRUE;
    }

    inline
    MUINT32    
    getSliderValue()
    {
        return m_rIspPca.Slider.value[m_u4Idx];
    }

private:
    //  PCA index
    MUINT32         m_u4Idx;
    //  Reference to m_rIspParam.ISPPca
    ISP_NVRAM_PCA_STRUCT&   m_rIspPca;
    //  Reference to m_rIspParam.ISPPca.Config
    ISP_NVRAM_PCA_T&        m_rIspPcaCfg;
    //  Reference to m_rIspParam.ISPPca.PCA_LUTs
    ISP_NVRAM_PCA_LUTS_T&   m_rIspPcaLUTs;
    ESensorDev_T m_eSensorDev;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Ctor
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////
    PcaMgr(ESensorDev_T const eSensorDev, ISP_NVRAM_PCA_STRUCT& rIspPca)
        : m_u4ChangeCount(1)
        , m_u4Idx(0)
        , m_rIspPca     (rIspPca)
        , m_rIspPcaCfg  (rIspPca.Config)
        , m_rIspPcaLUTs (rIspPca.PCA_LUTS)
        , m_eSensorDev (eSensorDev)
    {
    }

    virtual ~PcaMgr() {}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    load

    inline
    MVOID
    loadLut(MBOOL const fgForceToLoad = MFALSE)
    {
        if  (isChanged() || fgForceToLoad)
        {
            m_u4ChangeCount = 0;

            switch (m_u4Idx)
            {
            case LUT_LOW:       //color temperature: low
                ISP_MGR_PCA_T::getInstance(m_eSensorDev).loadLut(reinterpret_cast<MUINT32*>(&m_rIspPcaLUTs.lut_lo[0]));
                break;
            case LUT_MIDDLE:    //color temperature: middle
                ISP_MGR_PCA_T::getInstance(m_eSensorDev).loadLut(reinterpret_cast<MUINT32*>(&m_rIspPcaLUTs.lut_md[0]));
                break;
            case LUT_HIGH:      //color temperature: high
                ISP_MGR_PCA_T::getInstance(m_eSensorDev).loadLut(reinterpret_cast<MUINT32*>(&m_rIspPcaLUTs.lut_hi[0]));
                break;
            default:
                break;
            }
        }
    }

    inline
    MVOID
    loadConfig()
    {
        prepareIspHWBuf(m_eSensorDev, m_rIspPcaCfg );
    }

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
public:     ////

    inline
    MBOOL
    isEnable() const
    {
        return  ISP_MGR_PCA_T::getInstance(m_eSensorDev).isEnable();
    }

    inline
    ESensorDev_T
    getSensorDev() const
    {
        return m_eSensorDev;
    }

};

};  //  NSIspTuning
#endif // _PCA_MGR_H_

