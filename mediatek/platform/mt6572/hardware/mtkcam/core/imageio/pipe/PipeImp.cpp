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
#define LOG_TAG "iio/pimp"
//
//#define _LOG_TAG_LOCAL_DEFINED_
//#include <my_log.h>
//#undef  _LOG_TAG_LOCAL_DEFINED_
//
#include <cutils/properties.h>  // For property_get().

#include "PipeImp.h"
#include "imageio_log.h"                    // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
//

/*******************************************************************************
*
********************************************************************************/
namespace NSImageio {
namespace NSIspio   {
////////////////////////////////////////////////////////////////////////////////

#if 0
#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#define  DBG_LOG_TAG        "{Pipe}"
#include "imageio_log.h"                    // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
DECLARE_DBG_LOG_VARIABLE(pipe);
//EXTERN_DBG_LOG_VARIABLE(pipe);

// Clear previous define, use our own define.
#undef PIPE_VRB
#undef PIPE_DBG
#undef PIPE_INF
#undef PIPE_WRN
#undef PIPE_ERR
#undef PIPE_AST
#define PIPE_VRB(fmt, arg...)        do { if (pipe_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define PIPE_DBG(fmt, arg...)        do { if (pipe_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
#define PIPE_INF(fmt, arg...)        do { if (pipe_DbgLogEnable_INFO   ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define PIPE_WRN(fmt, arg...)        do { if (pipe_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define PIPE_ERR(fmt, arg...)        do { if (pipe_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define PIPE_AST(cond, fmt, arg...)  do { if (pipe_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)
#else

#include "imageio_log.h"                    // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.


#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#define  DBG_LOG_TAG        ""

//DECLARE_DBG_LOG_VARIABLE(pipe);
EXTERN_DBG_LOG_VARIABLE(pipe);

#endif

/*******************************************************************************
*
********************************************************************************/
PipeImp::
PipeImp(
    char const*const szPipeName,
    EPipeID const ePipeID,
    EScenarioID const eScenarioID,
    EScenarioFmt const eScenarioFmt
)
    : mszPipeName(szPipeName)
    , mePipeID(ePipeID)
    //
    , mi4ErrorCode(0)
    //
    , mpCbUser(NULL)
    , mi4NotifyMsgSet(0)
    , mNotifyCb(NULL)
    , mi4DataMsgSet(0)
    , mDataCb(NULL)
    //
    , meScenarioID(eScenarioID)
    , meScenarioFmt(eScenarioFmt)
    //
{
    DBG_LOG_CONFIG(imageio, pipe);
}


/*******************************************************************************
*
********************************************************************************/
MVOID
PipeImp::
setCallbacks(PipeNotifyCallback_t notify_cb, PipeDataCallback_t data_cb, MVOID* user)
{
    PIPE_DBG("(notify_cb, data_cb, user)=(%p, %p, %p)", notify_cb, data_cb, user);
    mpCbUser = user;
    mNotifyCb = notify_cb;
    mDataCb = data_cb;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
PipeImp::
onNotifyCallback(PipeNotifyInfo const& msg) const
{
    MBOOL   ret = MTRUE;
    //
    if  ( mNotifyCb )
    {
        mNotifyCb(mpCbUser, msg);
        ret = MTRUE;
    }
    else
    {
        PIPE_WRN("Notify Callback is NULL");
        ret = MFALSE;
    }
    return  ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
PipeImp::
onDataCallback(PipeDataInfo const& msg) const
{
    MBOOL   ret = MTRUE;
    //
    if  ( mDataCb )
    {
        mDataCb(mpCbUser, msg);
        ret = MTRUE;
    }
    else
    {
        PIPE_WRN("Data Callback is NULL");
        ret = MFALSE;
    }
    return  ret;
}


/*******************************************************************************
*
********************************************************************************/
MINT32
PipeImp::
mapScenarioFormatToSubmode(EScenarioID const eScenarioID, EScenarioFmt const eScenarioFmt)
{
#define FMT_TO_SUBMODE(_submode, _format)  case _format: return _submode

    switch  (eScenarioID)
    {
    case eScenarioID_IC:
        switch  (eScenarioFmt)
        {
        FMT_TO_SUBMODE(0, eScenarioFmt_RAW);
        FMT_TO_SUBMODE(1, eScenarioFmt_YUV);
        FMT_TO_SUBMODE(2, eScenarioFmt_RGB);
        FMT_TO_SUBMODE(3, eScenarioFmt_JPG);
        default: break;
        }
        break;
    //
    case eScenarioID_VR:
        switch  (eScenarioFmt)
        {
        FMT_TO_SUBMODE(0, eScenarioFmt_RAW);
        FMT_TO_SUBMODE(1, eScenarioFmt_YUV);
        FMT_TO_SUBMODE(2, eScenarioFmt_RGB);
        default: break;
        }
        break;
    //
    case eScenarioID_ZSD:
        switch  (eScenarioFmt)
        {
        FMT_TO_SUBMODE(0, eScenarioFmt_RAW);
        FMT_TO_SUBMODE(1, eScenarioFmt_YUV);
        default: break;
        }
        break;
    //
    case eScenarioID_IP:
        switch  (eScenarioFmt)
        {
        FMT_TO_SUBMODE(0, eScenarioFmt_RAW);
        FMT_TO_SUBMODE(1, eScenarioFmt_YUV);
        FMT_TO_SUBMODE(2, eScenarioFmt_RGB);
        FMT_TO_SUBMODE(3, eScenarioFmt_RGB_LOAD);
        FMT_TO_SUBMODE(4, eScenarioFmt_MFB);
        default: break;
        }
        break;
    //
    case eScenarioID_VEC:
        return  0;
    //
    case eScenarioID_N3D_IC:
        switch  (eScenarioFmt)
        {
        FMT_TO_SUBMODE(0, eScenarioFmt_RAW);
        FMT_TO_SUBMODE(1, eScenarioFmt_YUV);
        FMT_TO_SUBMODE(2, eScenarioFmt_RGB);
        default: break;
        }
        break;
    //
    case eScenarioID_N3D_VR:
        switch  (eScenarioFmt)
        {
        FMT_TO_SUBMODE(0, eScenarioFmt_RAW);
        FMT_TO_SUBMODE(1, eScenarioFmt_YUV);
        FMT_TO_SUBMODE(2, eScenarioFmt_RGB);
        default: break;
        }
        break;

    default:
        break;
    }

#undef  FMT_TO_SUBMODE

    PIPE_DBG("- bad (eScenarioID, eScenarioFmt)=(%d, %d)", eScenarioID, eScenarioFmt);
    return  -1;
}

////////////////////////////////////////////////////////////////////////////////
};  //namespace NSIspio
};  //namespace NSImageio

