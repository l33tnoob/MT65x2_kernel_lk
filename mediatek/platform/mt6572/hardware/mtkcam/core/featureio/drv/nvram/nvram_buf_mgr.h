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
#ifndef _NVRAM_BUF_MGR_H_
#define _NVRAM_BUF_MGR_H_


/*******************************************************************************
*
*******************************************************************************/
namespace NSNvram
{


/*******************************************************************************
*   NVRAM Buffer Manager
*******************************************************************************/
class NvramBufMgr
{
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
public:     ////    Autolock

    friend  class Autolock;
    class Autolock
    {
    public:
        Autolock()
            : m_pMutex(&NvramBufMgr::getInstance().m_mutex)
        {
            ::pthread_mutex_lock(m_pMutex);
        }

        ~Autolock()
        {
            ::pthread_mutex_unlock(m_pMutex);
        }

    private:
        pthread_mutex_t*const   m_pMutex;
    };

private:    ////    Data Members.

    NvramDrvBase&   m_rNvramDrv;
    pthread_mutex_t m_mutex;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
private:    ////    Instance.

    NvramBufMgr()
        : m_rNvramDrv(*NvramDrvBase::createInstance())
        , m_mutex()
    {
        ::pthread_mutex_init(&m_mutex, NULL);
    }

    ~NvramBufMgr()
    {
        m_rNvramDrv.destroyInstance();
    }

public:     ////    Singleton.
    static NvramBufMgr&     getInstance();

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
private:    ////    Nvram Data.

    template <class Buf_T, ESensorEnum_T SensorEnum>
    Buf_T* getBuf() const;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////

    template <class Buf_T, ENvramDrvCmd_T Cmd, ESensorEnum_T SensorEnum>
    MINT32
    flush(MUINT32 const u4SensorID)
    {
        return  m_rNvramDrv.writeNvram   (
            SensorEnum, u4SensorID, Cmd,
            getBuf<Buf_T, SensorEnum>(), sizeof(Buf_T)
        );
    }

    template <class Buf_T, ENvramDrvCmd_T Cmd, ESensorEnum_T SensorEnum>
    MINT32
    refresh(MUINT32 const u4SensorID)
    {
        return  m_rNvramDrv.readNvram   (
            SensorEnum, u4SensorID, Cmd,
            getBuf<Buf_T, SensorEnum>(), sizeof(Buf_T)
        );
    }

    template <class Buf_T, ENvramDrvCmd_T Cmd, ESensorEnum_T SensorEnum>
    Buf_T*
    getBuf(MUINT32 const u4SensorID)
    {
        static bool fgInitialized = false;
        if  (
                fgInitialized
            || (fgInitialized = (0 == refresh<Buf_T, Cmd, SensorEnum>(u4SensorID)))
            )
            return  getBuf<Buf_T, SensorEnum>();
        return  NULL;
    }

};


/*******************************************************************************
*
*   Map Buffer Type to Command.
*
*       template <class Buf_T> struct BufT2Cmd;
*       template<> struct BufT2Cmd<Buf_T>
*       {
*           static ENvramDrvCmd_T const val = ???;
*       };
*
*******************************************************************************/
template <class Buf_T> struct BufT2Cmd;


/*******************************************************************************
*   Implementation of Buffer I/F
*******************************************************************************/
template <class Buf_T>
struct ImpBufIF : public BufIF<Buf_T>
{
    static ENvramDrvCmd_T const Cmd = BufT2Cmd<Buf_T>::val;

    virtual
    Buf_T*
    getRefBuf(
        ESensorEnum_T const eSensorEnum, MUINT32 const u4SensorID
    )
    {
        NvramBufMgr::Autolock lock;
        switch  (eSensorEnum)
        {
        case DUAL_CAMERA_MAIN_SENSOR:
            return  NvramBufMgr::getInstance().getBuf<
                        Buf_T, Cmd, DUAL_CAMERA_MAIN_SENSOR
                    >(u4SensorID);
        case DUAL_CAMERA_MAIN_SECOND_SENSOR:
            return  NvramBufMgr::getInstance().getBuf<
                        Buf_T, Cmd, DUAL_CAMERA_MAIN_SECOND_SENSOR
                    >(u4SensorID);
        case DUAL_CAMERA_SUB_SENSOR:
            return  NvramBufMgr::getInstance().getBuf<
                        Buf_T, Cmd, DUAL_CAMERA_SUB_SENSOR
                    >(u4SensorID);
        default:
            break;
        }
        return NULL;
    }

    virtual
    MINT32
    refresh(
        ESensorEnum_T const eSensorEnum, MUINT32 const u4SensorID
    )
    {
        NvramBufMgr::Autolock lock;
        switch  (eSensorEnum)
        {
        case DUAL_CAMERA_MAIN_SENSOR:
            return  NvramBufMgr::getInstance().refresh<
                        Buf_T, Cmd, DUAL_CAMERA_MAIN_SENSOR
                    >(u4SensorID);
        case DUAL_CAMERA_MAIN_SECOND_SENSOR:
            return  NvramBufMgr::getInstance().refresh<
                        Buf_T, Cmd, DUAL_CAMERA_MAIN_SECOND_SENSOR
                    >(u4SensorID);
        case DUAL_CAMERA_SUB_SENSOR:
            return  NvramBufMgr::getInstance().refresh<
                        Buf_T, Cmd, DUAL_CAMERA_SUB_SENSOR
                    >(u4SensorID);
        default:
            break;
        }
        return  NVRAM_BAD_SENSOR_ENUM;
    }

    virtual
    MINT32
    flush(
        ESensorEnum_T const eSensorEnum, MUINT32 const u4SensorID
    )
    {
        NvramBufMgr::Autolock lock;
        switch  (eSensorEnum)
        {
        case DUAL_CAMERA_MAIN_SENSOR:
            return  NvramBufMgr::getInstance().flush<
                        Buf_T, Cmd, DUAL_CAMERA_MAIN_SENSOR
                    >(u4SensorID);
        case DUAL_CAMERA_MAIN_SECOND_SENSOR:
            return  NvramBufMgr::getInstance().flush<
                        Buf_T, Cmd, DUAL_CAMERA_MAIN_SECOND_SENSOR
                    >(u4SensorID);
        case DUAL_CAMERA_SUB_SENSOR:
            return  NvramBufMgr::getInstance().flush<
                        Buf_T, Cmd, DUAL_CAMERA_SUB_SENSOR
                    >(u4SensorID);
        default:
            break;
        }
        return  NVRAM_BAD_SENSOR_ENUM;
    }

};


/*******************************************************************************
*
*******************************************************************************/
};  //  NSNvram
#endif // _NVRAM_BUF_MGR_H_

