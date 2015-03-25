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
#ifndef _IMEM_DRV_IMP_H_
#define _IMEM_DRV_IMP_H_
//-----------------------------------------------------------------------------
#include <utils/Errors.h>
#include <cutils/log.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <utils/threads.h>
#include <cutils/atomic.h>
#include <linux/cache.h>
//
#include "mtkcam/common.h"
#include <mtkcam/drv/imem_drv.h>
//
#if defined (__ISP_USE_PMEM__)     //seanlin fix if using PMEM
   #include <cutils/pmem.h>
#endif
#if defined (__ISP_USE_STD_M4U__) || defined (__ISP_USE_ION__)
    #include "m4u_lib.h"
#endif

#if defined (__ISP_USE_ION__)
    #include <linux/ion_drv.h>
    #include <ion/ion.h>
#endif

#include <vector>
#include <map>
#include <list>
using namespace std;
//-----------------------------------------------------------------------------
using namespace android;
//-----------------------------------------------------------------------------

#include <cutils/properties.h>              // For property_get().

#undef  DBG_LOG_TAG                        // Decide a Log TAG for current file.
#define DBG_LOG_TAG     LOG_TAG
#include "drv_log.h"                    // Note: DBG_LOG_TAG will be used in header file, so header must be included after definition.
DECLARE_DBG_LOG_VARIABLE(imem_drv);
//EXTERN_DBG_LOG_VARIABLE(imem_drv);

// Clear previous define, use our own define.
#undef LOG_VRB
#undef LOG_DBG
#undef LOG_INF
#undef LOG_WRN
#undef LOG_ERR
#undef LOG_AST
#define LOG_VRB(fmt, arg...)        do { if (imem_drv_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define LOG_DBG(fmt, arg...)        do { if (imem_drv_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
#define LOG_INF(fmt, arg...)        do { if (imem_drv_DbgLogEnable_INFO   ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define LOG_WRN(fmt, arg...)        do { if (imem_drv_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define LOG_ERR(fmt, arg...)        do { if (imem_drv_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define LOG_AST(cond, fmt, arg...)  do { if (imem_drv_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)

#define IMEM_VRB    LOG_VRB
#define IMEM_DBG    LOG_DBG
#define IMEM_INF    LOG_INF
#define IMEM_WRN    LOG_WRN
#define IMEM_ERR    LOG_ERR
//-----------------------------------------------------------------------------
typedef struct _imem_map_info_
{
    MUINT32         pAddr;
    MUINT32         size;    
}stIMEM_MAP_INFO;

//-----------------------------------------------------------------------------
class IMemDrvImp : public IMemDrv
{
    public:
        IMemDrvImp();
        ~IMemDrvImp();
    //
    public:
        static IMemDrv*  getInstance(void);
        virtual void    destroyInstance(void);
        virtual MBOOL   init(void);
        virtual MBOOL   uninit(void);
        virtual MBOOL   reset(void);
        virtual MINT32  allocVirtBuf(
            IMEM_BUF_INFO* pInfo);
        virtual MINT32  freeVirtBuf(
            IMEM_BUF_INFO* pInfo);
        virtual MINT32  mapPhyAddr(
            IMEM_BUF_INFO* pInfo);
        virtual MINT32  unmapPhyAddr(
            IMEM_BUF_INFO* pInfo);
        virtual MINT32  cacheFlushAll(void);
    //
#if defined (__ISP_USE_STD_M4U__) || defined (__ISP_USE_ION__)
        //m4u
        virtual MINT32  allocM4UMemory(
            MUINT32     virtAddr,
            MUINT32     size,
            MUINT32*    m4uVa,
            MINT32      memID); 
        virtual MINT32  freeM4UMemory(
            MUINT32     virtAddr,
            MUINT32     m4uVa,
            MUINT32     size,
            MINT32      memID);
        //
        MTKM4UDrv*  mpM4UDrv;
#endif
#if defined (__ISP_USE_ION__)

//        virtual MUINT32 ion_alloc(
//            stIspIonAllocData ion_alloc_data);
//        virtual MUINT32 ion_free(
//            stIspIonHandleData handle_data);

        #define stIspIonAllocData struct ion_allocation_data
        #define stIspIonHandleData struct ion_handle_data

        MINT32 mIonDrv;
#endif

    private:
        volatile MINT32 mInitCount;
        mutable Mutex   mLock;
        map<MUINT32,stIMEM_MAP_INFO> buf_map;
        MINT32 mIspFd;
        volatile MINT32 mLocal_InitCount;        
};


#endif


