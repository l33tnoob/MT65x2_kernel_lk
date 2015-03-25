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
#ifndef _MY_LOG_H_
#define _MY_LOG_H_

/*-----------------------------------------------------------------------------
    DEBUG LOG UTILITY
  -----------------------------------------------------------------------------*/
#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.

#ifdef _LOG_TAG_LOCAL_DEFINED_
#define  DBG_LOG_TAG        LOG_TAG
#else
#define  DBG_LOG_TAG        "{imageio} "
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

//#undef   DBG_LOG_LEVEL                      // Obsolete   // Decide Debug Log level for current file. Can only choose from 2~8.
//#define  DBG_LOG_LEVEL      8               // Obsolete   // 2(VERBOSE)/3(DEBUG)/4(INFO)/5(WARN)/6(ERROR)/7(ASSERT)/8(SILENT).

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <cutils/properties.h>              // For property_get().
#include "imageio_log.h"                    // Note: DBG_LOG_TAG will be used in header file, so header must be included after definition.
//DECLARE_DBG_LOG_VARIABLE(imageio);
EXTERN_DBG_LOG_VARIABLE(imageio);

// Clear previous define, use our own define.
#undef LOG_VRB
#undef LOG_DBG
#undef LOG_INF
#undef LOG_WRN
#undef LOG_ERR
#undef LOG_AST
#define LOG_VRB(fmt, arg...)        do { if (imageio_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define LOG_DBG(fmt, arg...)        do { if (imageio_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
#define LOG_INF(fmt, arg...)        do { if (imageio_DbgLogEnable_INFO   ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define LOG_WRN(fmt, arg...)        do { if (imageio_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define LOG_ERR(fmt, arg...)        do { if (imageio_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define LOG_AST(cond, fmt, arg...)  do { if (imageio_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)

//
#define DEBUG_STR_BEGIN "EEEEEEEEEEEEEEEEEEEEE"
#define DEBUG_STR_END   "XXXXXXXXXXXXXXXXXXXXX"
//
#define ISP_FUNC_VRB(fmt,arg...)    LOG_VRB(fmt,##arg)
#define ISP_FUNC_DBG(fmt,arg...)    LOG_DBG(fmt,##arg)
#define ISP_FUNC_INF(fmt,arg...)    LOG_INF(fmt,##arg)
#define ISP_FUNC_WRN(fmt,arg...)    LOG_WRN(fmt,##arg)
#define ISP_FUNC_ERR(fmt,arg...)    LOG_ERR(fmt,##arg)
//
#define ISP_PATH_VRB    LOG_VRB
#define ISP_PATH_DBG    LOG_DBG
#define ISP_PATH_INF    LOG_INF
#define ISP_PATH_WRN    LOG_WRN
#define ISP_PATH_ERR    LOG_ERR
//
#define PIPE_VRB    LOG_VRB
#define PIPE_DBG    LOG_DBG
#define PIPE_INF    LOG_INF
#define PIPE_WRN    LOG_WRN
#define PIPE_ERR    LOG_ERR
//
#define MY_LOGD LOG_DBG
#define MY_LOGW LOG_WRN
#define MY_LOGE LOG_ERR
//

#ifdef USING_MTK_LDVT
#include "uvvf.h"
#define UTIL_VRB(fmt,...)      VV_MSG(fmt"\n", ##__VA_ARGS__)
#define UTIL_DBG(fmt,...)      VV_MSG(fmt"\n", ##__VA_ARGS__)
#define UTIL_INF(fmt,...)      VV_MSG(fmt"\n", ##__VA_ARGS__)
#define UTIL_WRN(fmt,...)      VV_MSG(fmt"\n", ##__VA_ARGS__)
#define UTIL_ERR(fmt,...)      VV_MSG(fmt"\n", ##__VA_ARGS__)
#else
#include <cutils/xlog.h>
#define UTIL_VRB(fmt,...)      XLOGV("[%s]"fmt"\n", __FUNCTION__, ##__VA_ARGS__)
#define UTIL_DBG(fmt,...)      XLOGD("[%s]"fmt"\n", __FUNCTION__, ##__VA_ARGS__)
#define UTIL_INF(fmt,...)      XLOGI("[%s]"fmt"\n", __FUNCTION__, ##__VA_ARGS__)
#define UTIL_WRN(fmt,...)      XLOGW("[%s]"fmt"\n", __FUNCTION__, ##__VA_ARGS__)
#define UTIL_ERR(fmt,...)      XLOGE("[%s]"fmt"\n", __FUNCTION__, ##__VA_ARGS__)
#endif  // USING_MTK_LDVT
//



//


#endif  //  _MY_LOG_H_

