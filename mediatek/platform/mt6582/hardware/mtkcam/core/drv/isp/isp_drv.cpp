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
#define LOG_TAG "IspDrv"

#include <utils/Errors.h>
#include <utils/Mutex.h>    // For android::Mutex.
#include <cutils/log.h>
#include <fcntl.h>
#include <sys/mman.h>
//#include <utils/threads.h>
#include <cutils/atomic.h>
//#include <cutils/pmem.h>

#include "camera_isp.h"
#include <mtkcam/drv/isp_reg.h>
#include "isp_drv_imp.h"

#include <cutils/properties.h>  // For property_get().

#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#define  DBG_LOG_TAG        "{IspDrv} "
#include "drv_log.h"                        // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
DECLARE_DBG_LOG_VARIABLE(isp_drv);
//EXTERN_DBG_LOG_VARIABLE(isp_drv);

// Clear previous define, use our own define.
#undef LOG_VRB
#undef LOG_DBG
#undef LOG_INF
#undef LOG_WRN
#undef LOG_ERR
#undef LOG_AST
#define LOG_VRB(fmt, arg...)        do { if (isp_drv_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define LOG_DBG(fmt, arg...)        do { if (isp_drv_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
#define LOG_INF(fmt, arg...)        do { if (isp_drv_DbgLogEnable_INFO   ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define LOG_WRN(fmt, arg...)        do { if (isp_drv_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define LOG_ERR(fmt, arg...)        do { if (isp_drv_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define LOG_AST(cond, fmt, arg...)  do { if (isp_drv_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)


/**************************************************************************
* CLASS DECLARATION
**************************************************************************/
class IspDbgTimer
{
protected:
    char const*const    mpszName;
    mutable MINT32      mIdx;
    MINT32 const        mi4StartUs;
    mutable MINT32      mi4LastUs;
    
public:
    IspDbgTimer(char const*const pszTitle)
        : mpszName(pszTitle)
        , mIdx(0)
        , mi4StartUs(getUs())
        , mi4LastUs(getUs())
    {
    }

    inline MINT32 getUs() const
    {
        struct timeval tv;
        ::gettimeofday(&tv, NULL);
        return tv.tv_sec * 1000000 + tv.tv_usec;
    }

    inline MBOOL ProfilingPrint(char const*const pszInfo = "") const
    {
        MINT32 const i4EndUs = getUs();
        if(0 == mIdx)
        {
            LOG_INF("[%s] %s:(%d-th) ===> [start-->now: %.06f ms]", mpszName, pszInfo, mIdx++, (float)(i4EndUs-mi4StartUs)/1000);
        }
        else
        {
            LOG_INF("[%s] %s:(%d-th) ===> [start-->now: %.06f ms] [last-->now: %.06f ms]", mpszName, pszInfo, mIdx++, (float)(i4EndUs-mi4StartUs)/1000, (float)(i4EndUs-mi4LastUs)/1000);
        }
        mi4LastUs = i4EndUs;

	    //sleep(4); //wait 1 sec for AE stable

        return  MTRUE;
    }
};

#ifndef USING_MTK_LDVT   // Not using LDVT.
    #if 0   // Use CameraProfile API
        static unsigned int G_emGlobalEventId = 0; // Used between different functions.
        static unsigned int G_emLocalEventId = 0;  // Used within each function.
        #define GLOBAL_PROFILING_LOG_START(EVENT_ID);       CPTLog(EVENT_ID, CPTFlagStart); G_emGlobalEventId = EVENT_ID;
        #define GLOBAL_PROFILING_LOG_END();                 CPTLog(G_emGlobalEventId, CPTFlagEnd);
        #define GLOBAL_PROFILING_LOG_PRINT(LOG_STRING);     CPTLogStr(G_emGlobalEventId, CPTFlagSeparator, LOG_STRING);
        #define LOCAL_PROFILING_LOG_AUTO_START(EVENT_ID);   AutoCPTLog CPTlogLocalVariable(EVENT_ID); G_emLocalEventId = EVENT_ID;
        #define LOCAL_PROFILING_LOG_PRINT(LOG_STRING);      CPTLogStr(G_emLocalEventId, CPTFlagSeparator, LOG_STRING);
    #elif 1   // Use debug print
        #define GLOBAL_PROFILING_LOG_START(EVENT_ID);
        #define GLOBAL_PROFILING_LOG_END();
        #define GLOBAL_PROFILING_LOG_PRINT(LOG_STRING);
        #define LOCAL_PROFILING_LOG_AUTO_START(EVENT_ID);   IspDbgTimer DbgTmr(#EVENT_ID);
        #define LOCAL_PROFILING_LOG_PRINT(LOG_STRING);      DbgTmr.ProfilingPrint(LOG_STRING);
    #else   // No profiling.
        #define GLOBAL_PROFILING_LOG_START(EVENT_ID);
        #define GLOBAL_PROFILING_LOG_END();
        #define GLOBAL_PROFILING_LOG_PRINT(LOG_STRING);
        #define LOCAL_PROFILING_LOG_AUTO_START(EVENT_ID);
        #define LOCAL_PROFILING_LOG_PRINT(LOG_STRING);
    #endif  // Diff Profile tool.
#else   // Using LDVT.
    #if 0   // Use debug print
        #define GLOBAL_PROFILING_LOG_START(EVENT_ID);
        #define GLOBAL_PROFILING_LOG_END();
        #define GLOBAL_PROFILING_LOG_PRINT(LOG_STRING);
        #define LOCAL_PROFILING_LOG_AUTO_START(EVENT_ID);   IspDbgTimer DbgTmr(#EVENT_ID);
        #define LOCAL_PROFILING_LOG_PRINT(LOG_STRING);      DbgTmr.ProfilingPrint(LOG_STRING);
    #else   // No profiling.
        #define GLOBAL_PROFILING_LOG_START(EVENT_ID);
        #define GLOBAL_PROFILING_LOG_END();
        #define GLOBAL_PROFILING_LOG_PRINT(LOG_STRING);
        #define LOCAL_PROFILING_LOG_AUTO_START(EVENT_ID);
        #define LOCAL_PROFILING_LOG_PRINT(LOG_STRING);
    #endif  // Diff Profile tool.
#endif  // USING_MTK_LDVT


/**************************************************************************
* DEFINTION
**************************************************************************/

#define ISP_DRV_MMAP_TO_IOCLT_ENABLE        (1)

#define ISP_CQ_WRITE_INST           0x0
#define ISP_DRV_CQ_END_TOKEN       0xFC000000
#define ISP_DRV_CQ_DUMMY_WR_TOKEN  0x00004060
#define ISP_CQ_DUMMY_PA             0x88100000

#define ISP_DRV_TURNING_TOP_SET_EN1     0x3F6F84A8      //0x4080
#define ISP_DRV_TURNING_TOP_CLR_EN1     0x3F6F84A8      //0x4084
#define ISP_DRV_TURNING_TOP_SET_EN2     0x0080001C      //0x4088
#define ISP_DRV_TURNING_TOP_CLR_EN2     0x0080001C      //0x408C
#define ISP_DRV_TURNING_TOP_NUM         0x04
#define ISP_DRV_TURNING_MAX_DBG_NUM     0x05

#define ISP_INT_BIT_NUM 32

/**************************************************************************
* ENUM / STRUCT / TYPEDEF  DECLARATION
**************************************************************************/

/**************************************************************************
* EXTERNAL  REFERENCES
**************************************************************************/

/**************************************************************************
* GLOBAL DATA
**************************************************************************/

pthread_mutex_t IspRegMutex = PTHREAD_MUTEX_INITIALIZER;

// for turning update
//static pthread_mutex_t turningMutexCq1 = PTHREAD_MUTEX_INITIALIZER;
//static pthread_mutex_t turningMutexCq2 = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t tdriMgrMutexCq1 = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t tdriMgrMutexCq2 = PTHREAD_MUTEX_INITIALIZER;

ISP_DRV_TURNING_TOP gIspDrvTurningTop[ISP_DRV_TURNING_TOP_NUM]
= { {ISP_DRV_TURNING_TOP_SET_EN1, (0x4080 >> 2)},   // >>2 for MUINT32* pointer
    {ISP_DRV_TURNING_TOP_CLR_EN1, (0x4084 >> 2)},
    {ISP_DRV_TURNING_TOP_SET_EN2, (0x4088 >> 2)},
    {ISP_DRV_TURNING_TOP_CLR_EN2, (0x408C >> 2)}
  };

ISP_DRV_CQ_MAPPING gIspDrvCqMappinig[ISP_DRV_CQ_NUM]
= {{ISP_DRV_CQ03,        ISP_DRV_DESCRIPTOR_CQ03},
   {ISP_DRV_CQ0,         ISP_DRV_DESCRIPTOR_CQ0},
   {ISP_DRV_CQ0B,        ISP_DRV_DESCRIPTOR_CQ0B},
   {ISP_DRV_CQ0C,        ISP_DRV_DESCRIPTOR_CQ0C},
   {ISP_DRV_CQ01,        ISP_DRV_DESCRIPTOR_CQ01},
   {ISP_DRV_CQ01_SYNC,   ISP_DRV_DESCRIPTOR_CQ01},
   {ISP_DRV_CQ02,        ISP_DRV_DESCRIPTOR_CQ02},
   {ISP_DRV_CQ02_SYNC,   ISP_DRV_DESCRIPTOR_CQ02},
//  {ISP_DRV_CQ03,        ISP_DRV_DESCRIPTOR_CQ03},
  };

ISP_DRV_CQ_CMD_DESC_INIT_STRUCT  gIspCQDescInit[CAM_MODULE_MAX]
#if 1
= {{ISP_DRV_CQ_DUMMY_WR_TOKEN,(MUINT32)ISP_CQ_DUMMY_PA},  \
   {ISP_DRV_CQ_DUMMY_WR_TOKEN,(MUINT32)ISP_CQ_DUMMY_PA},  \
   {ISP_DRV_CQ_DUMMY_WR_TOKEN,(MUINT32)ISP_CQ_DUMMY_PA},  \
   {ISP_DRV_CQ_DUMMY_WR_TOKEN,(MUINT32)ISP_CQ_DUMMY_PA},  \
   {ISP_DRV_CQ_DUMMY_WR_TOKEN,(MUINT32)ISP_CQ_DUMMY_PA},  \
   {ISP_DRV_CQ_DUMMY_WR_TOKEN,(MUINT32)ISP_CQ_DUMMY_PA},  \
   {ISP_DRV_CQ_DUMMY_WR_TOKEN,(MUINT32)ISP_CQ_DUMMY_PA},  \
   {ISP_DRV_CQ_DUMMY_WR_TOKEN,(MUINT32)ISP_CQ_DUMMY_PA},  \
   {ISP_DRV_CQ_DUMMY_WR_TOKEN,(MUINT32)ISP_CQ_DUMMY_PA},  \
   {ISP_DRV_CQ_DUMMY_WR_TOKEN,(MUINT32)ISP_CQ_DUMMY_PA},  \
   {ISP_DRV_CQ_DUMMY_WR_TOKEN,(MUINT32)ISP_CQ_DUMMY_PA},  \
   {ISP_DRV_CQ_DUMMY_WR_TOKEN,(MUINT32)ISP_CQ_DUMMY_PA},  \
   {ISP_DRV_CQ_DUMMY_WR_TOKEN,(MUINT32)ISP_CQ_DUMMY_PA},  \
   {ISP_DRV_CQ_DUMMY_WR_TOKEN,(MUINT32)ISP_CQ_DUMMY_PA},  \
   {ISP_DRV_CQ_DUMMY_WR_TOKEN,(MUINT32)ISP_CQ_DUMMY_PA},  \
   {ISP_DRV_CQ_DUMMY_WR_TOKEN,(MUINT32)ISP_CQ_DUMMY_PA},  \
   {ISP_DRV_CQ_DUMMY_WR_TOKEN,(MUINT32)ISP_CQ_DUMMY_PA},  \
   {ISP_DRV_CQ_DUMMY_WR_TOKEN,(MUINT32)ISP_CQ_DUMMY_PA},  \
   {ISP_DRV_CQ_DUMMY_WR_TOKEN,(MUINT32)ISP_CQ_DUMMY_PA},  \
   {ISP_DRV_CQ_DUMMY_WR_TOKEN,(MUINT32)ISP_CQ_DUMMY_PA},  \
   {ISP_DRV_CQ_DUMMY_WR_TOKEN,(MUINT32)ISP_CQ_DUMMY_PA},  \
   {ISP_DRV_CQ_DUMMY_WR_TOKEN,(MUINT32)ISP_CQ_DUMMY_PA},  \
   {ISP_DRV_CQ_DUMMY_WR_TOKEN,(MUINT32)ISP_CQ_DUMMY_PA},  \
   {ISP_DRV_CQ_DUMMY_WR_TOKEN,(MUINT32)ISP_CQ_DUMMY_PA},  \
   {ISP_DRV_CQ_DUMMY_WR_TOKEN,(MUINT32)ISP_CQ_DUMMY_PA},  \
   {ISP_DRV_CQ_DUMMY_WR_TOKEN,(MUINT32)ISP_CQ_DUMMY_PA},  \
   {ISP_DRV_CQ_DUMMY_WR_TOKEN,(MUINT32)ISP_CQ_DUMMY_PA},  \
   {ISP_DRV_CQ_DUMMY_WR_TOKEN,(MUINT32)ISP_CQ_DUMMY_PA},  \
   {ISP_DRV_CQ_DUMMY_WR_TOKEN,(MUINT32)ISP_CQ_DUMMY_PA},  \
   {ISP_DRV_CQ_DUMMY_WR_TOKEN,(MUINT32)ISP_CQ_DUMMY_PA},  \
   {ISP_DRV_CQ_DUMMY_WR_TOKEN,(MUINT32)ISP_CQ_DUMMY_PA},  \
   {ISP_DRV_CQ_DUMMY_WR_TOKEN,(MUINT32)ISP_CQ_DUMMY_PA},  \
   {ISP_DRV_CQ_DUMMY_WR_TOKEN,(MUINT32)ISP_CQ_DUMMY_PA},  \
   {ISP_DRV_CQ_DUMMY_WR_TOKEN,(MUINT32)ISP_CQ_DUMMY_PA},  \
   {ISP_DRV_CQ_DUMMY_WR_TOKEN,(MUINT32)ISP_CQ_DUMMY_PA},  \
   {ISP_DRV_CQ_DUMMY_WR_TOKEN,(MUINT32)ISP_CQ_DUMMY_PA},  \
   {ISP_DRV_CQ_DUMMY_WR_TOKEN,(MUINT32)ISP_CQ_DUMMY_PA},  \
   {ISP_DRV_CQ_DUMMY_WR_TOKEN,(MUINT32)ISP_CQ_DUMMY_PA},  \
   {ISP_DRV_CQ_DUMMY_WR_TOKEN,(MUINT32)ISP_CQ_DUMMY_PA},  \
   {ISP_DRV_CQ_DUMMY_WR_TOKEN,(MUINT32)ISP_CQ_DUMMY_PA},  \
   {ISP_DRV_CQ_DUMMY_WR_TOKEN,(MUINT32)ISP_CQ_DUMMY_PA},  \
   {ISP_DRV_CQ_DUMMY_WR_TOKEN,(MUINT32)ISP_CQ_DUMMY_PA},  \
   {ISP_DRV_CQ_END_TOKEN,(MUINT32)0}};
#endif

ISP_DRV_CQ_MODULE_INFO_STRUCT gIspCQModuleInfo[CAM_MODULE_MAX]
=   { {CAM_TOP_CTL,               0x4050, 2  },
      {CAM_TOP_CTL_01,            0x4080, 10 },
      {CAM_TOP_CTL_02,            0x40C0, 8  },
              {CAM_DMA_TDRI,              0x4208 /*0x4204*/, 2  /*3*/}, //remove CAM_REG_TDRI_BASE_ADDR          CAM_TDRI_BASE_ADDR;       // 4204 //              {CAM_DMA_TDRI,              0x4204, 3  }, //SL TEST_MDP
              {CAM_DMA_IMGI,              0x4240, 3  }, //0x4240/*0x4230*/, 3 /*8*/  },//0x4234/*0x4230*/, 6 /*8*/  }, //remove BASE/OFST/XS/YX/RSV from 4230 to 4244 just IMGI_CON.CON2 {CAM_DMA_IMGI,              0x4230, 8  }, //SL TEST_MDP
      {CAM_DMA_LSCI,              0x426C, 7  },
      {CAM_DMA_IMGO,              0x4304, 7  },
      {CAM_DMA_IMG2O,             0x4324, 7  },
      {CAM_DMA_EISO,              0x435C, 2  },
      {CAM_DMA_AFO,               0x4364, 2  },
      {CAM_DMA_ESFKO,             0x436C, 7  },
      {CAM_DMA_AAO,               0x4388, 7  },
      {CAM_RAW_TG1_TG2,           0x4410, 30 },// MT6582 only TG1
      {CAM_ISP_BIN,               0x44F0, 2  },
      {CAM_ISP_OBC,               0x4500, 8  },
      {CAM_ISP_LSC,               0x4530, 8  },
      {CAM_ISP_HRZ,               0x4580, 2  },
      {CAM_ISP_AWB,               0x45B0, 36 },
      {CAM_ISP_AE,                0x4650, 18 },
      {CAM_ISP_SGG,               0x46A0, 2  },
      {CAM_ISP_AF,                0x46B0, 23 },
      {CAM_ISP_FLK,               0x4770, 4  },
      {CAM_ISP_BNR,               0x4800, 18 },
      {CAM_ISP_PGN,               0x4880, 6  },
      {CAM_ISP_CFA,               0x48A0, 22},//22 },
      {CAM_ISP_CCL,               0x4910, 3  },
      {CAM_ISP_G2G,               0x4920, 7  },
      {CAM_ISP_UNP,               0x4948, 1  },
      {CAM_ISP_G2C,               0x4A00, 6  },
      {CAM_ISP_C42,               0x4A1C, 1  },
      {CAM_ISP_NBC,               0x4A20, 32 },
      {CAM_ISP_SEEE,              0x4AA0, 24 },
      {CAM_CDP_CDRZ,              0x4B00, 15 },
      {CAM_ISP_EIS,               0x4DC0, 9  },
      {CAM_CDP_SL2_FEATUREIO_1,   0x4F40, 4  },
      {CAM_CDP_SL2_IMAGEIO,       0x4F50, 1  },//ic.hsu
      {CAM_CDP_SL2_FEATUREIO_2,   0x4F54, 2  },
      {CAM_ISP_GGMRB,             0x5000, 144},
      {CAM_ISP_GGMG,              0x5300, 144},
      {CAM_ISP_GGM_CTL,           0x5600, 1  },
      {CAM_ISP_PCA,               0x5800, 360},
      {CAM_ISP_PCA_CON,           0x5E00, 2  },
      {CAM_DUMMY_,                0x4780, 1  }};  // dummy address 4780//0x4600, 1  }};  // dummy address 

ISP_DRV_CQ_BIT_3GP_MAPPING gIspTurnEn1Mapp[ISP_INT_BIT_NUM]
= { {CAM_DUMMY_,          CAM_DUMMY_,         CAM_DUMMY_},          // bit 0
    {CAM_DUMMY_,          CAM_DUMMY_,         CAM_DUMMY_},          // bit 1
    {CAM_DUMMY_,          CAM_DUMMY_,         CAM_DUMMY_},          // bit 2
    {CAM_ISP_OBC,         CAM_DUMMY_,         CAM_DUMMY_},          // bit 3
    {CAM_DUMMY_,          CAM_DUMMY_,         CAM_DUMMY_},          // bit 4
    {CAM_ISP_LSC,         CAM_DUMMY_,         CAM_DUMMY_},          // bit 5
    {CAM_DUMMY_,          CAM_DUMMY_,         CAM_DUMMY_},          // bit 6
    {CAM_ISP_BNR,         CAM_DUMMY_,         CAM_DUMMY_},          // bit 7
    {CAM_CDP_SL2_FEATUREIO_1,          CAM_DUMMY_,         CAM_DUMMY_},          // bit 8
    {CAM_DUMMY_,          CAM_DUMMY_,         CAM_DUMMY_},          // bit 9
    {CAM_DUMMY_,          CAM_DUMMY_,         CAM_DUMMY_},          // bit 10
    {CAM_ISP_PGN,         CAM_DUMMY_,         CAM_DUMMY_},          // bit 11
    {CAM_DUMMY_,          CAM_DUMMY_,         CAM_DUMMY_},          // bit 12
    {CAM_DUMMY_,          CAM_DUMMY_,         CAM_DUMMY_},          // bit 13
    {CAM_DUMMY_,          CAM_DUMMY_,         CAM_DUMMY_},          // bit 14
    {CAM_DUMMY_,          CAM_DUMMY_,         CAM_DUMMY_},          // bit 15
    {CAM_DUMMY_,          CAM_DUMMY_,         CAM_DUMMY_},          // bit 16
    {CAM_DUMMY_,          CAM_DUMMY_,         CAM_DUMMY_},          // bit 17
    {CAM_DUMMY_,          CAM_DUMMY_,         CAM_DUMMY_},          // bit 18
    {CAM_DUMMY_,          CAM_DUMMY_,         CAM_DUMMY_},          // bit 19
    {CAM_DUMMY_,          CAM_DUMMY_,         CAM_DUMMY_},          // bit 20
    {CAM_DUMMY_,          CAM_DUMMY_,         CAM_DUMMY_},          // bit 21
    {CAM_ISP_CCL,         CAM_DUMMY_,         CAM_DUMMY_},          // bit 22
    {CAM_ISP_G2G,         CAM_DUMMY_,         CAM_DUMMY_},          // bit 23
    {CAM_DUMMY_,          CAM_DUMMY_,         CAM_DUMMY_},          // bit 24
    {CAM_DUMMY_,          CAM_DUMMY_,         CAM_DUMMY_}, // bit 25
    {CAM_ISP_GGMRB,       CAM_ISP_GGMG,       CAM_ISP_GGM_CTL},     // bit 26 (GGM controled by 0x5600)
    {CAM_DUMMY_,          CAM_DUMMY_,         CAM_DUMMY_},          // bit 27
    {CAM_DUMMY_,          CAM_DUMMY_,         CAM_DUMMY_},          // bit 28
    {CAM_DUMMY_,          CAM_DUMMY_,         CAM_DUMMY_},          // bit 29
    {CAM_DUMMY_,          CAM_DUMMY_,         CAM_DUMMY_},          // bit 30
    {CAM_DUMMY_,          CAM_DUMMY_,         CAM_DUMMY_}};         // bit 31

ISP_DRV_CQ_BIT_2GP_MAPPING gIspTurnEn2Mapp[ISP_INT_BIT_NUM]
= { {CAM_ISP_G2C,  CAM_DUMMY_},       // bit 0
    {CAM_DUMMY_,   CAM_DUMMY_},       // bit 1
    {CAM_ISP_NBC,  CAM_DUMMY_},       // bit 2
    {CAM_ISP_PCA,  CAM_ISP_PCA_CON},  // bit 3
    {CAM_ISP_SEEE, CAM_DUMMY_},       // bit 4
    {CAM_DUMMY_,   CAM_DUMMY_},       // bit 5
    {CAM_DUMMY_,   CAM_DUMMY_},       // bit 6
    {CAM_DUMMY_,   CAM_DUMMY_},       // bit 7
    {CAM_DUMMY_,   CAM_DUMMY_},       // bit 8
    {CAM_DUMMY_,   CAM_DUMMY_},       // bit 9
    {CAM_DUMMY_,   CAM_DUMMY_},       // bit 10
    {CAM_DUMMY_,   CAM_DUMMY_},       // bit 11
    {CAM_DUMMY_,   CAM_DUMMY_},       // bit 12
    {CAM_DUMMY_,   CAM_DUMMY_},       // bit 13
    {CAM_DUMMY_,   CAM_DUMMY_},       // bit 14
    {CAM_DUMMY_,   CAM_DUMMY_},       // bit 15
    {CAM_DUMMY_,   CAM_DUMMY_},       // bit 16
    {CAM_DUMMY_,   CAM_DUMMY_},       // bit 17
    {CAM_DUMMY_,   CAM_DUMMY_},       // bit 18
    {CAM_DUMMY_,   CAM_DUMMY_},       // bit 19
    {CAM_DUMMY_,   CAM_DUMMY_},       // bit 20
    {CAM_DUMMY_,   CAM_DUMMY_},       // bit 21
    {CAM_DUMMY_,   CAM_DUMMY_},       // bit 22
    {CAM_DUMMY_,   CAM_DUMMY_},       // bit 23
    {CAM_DUMMY_,   CAM_DUMMY_},       // bit 24
    {CAM_DUMMY_,   CAM_DUMMY_},       // bit 25
    {CAM_DUMMY_,   CAM_DUMMY_},       // bit 26
    {CAM_DUMMY_,   CAM_DUMMY_},       // bit 27
    {CAM_DUMMY_,   CAM_DUMMY_},       // bit 28
    {CAM_DUMMY_,   CAM_DUMMY_},       // bit 29
    {CAM_DUMMY_,   CAM_DUMMY_},       // bit 30
    {CAM_DUMMY_,   CAM_DUMMY_}};      // bit 31

ISP_DRV_CQ_BIT_1GP_MAPPING gIspTurnDmaMapp[ISP_INT_BIT_NUM]
= { {CAM_DUMMY_   },  // bit 0
    {CAM_DMA_LSCI },  // bit 1
    {CAM_DUMMY_   },  // bit 2
    {CAM_DUMMY_   },  // bit 3
    {CAM_DUMMY_   },  // bit 4
    {CAM_DUMMY_   },  // bit 5
    {CAM_DUMMY_   },  // bit 6
    {CAM_DUMMY_   },  // bit 7
    {CAM_DUMMY_   },  // bit 8
    {CAM_DUMMY_   },  // bit 9
    {CAM_DUMMY_   },  // bit 10
    {CAM_DUMMY_   },  // bit 11
    {CAM_DUMMY_   },  // bit 12
    {CAM_DUMMY_   },  // bit 13
    {CAM_DUMMY_   },  // bit 14
    {CAM_DUMMY_   },  // bit 15
    {CAM_DUMMY_   },  // bit 16
    {CAM_DUMMY_   },  // bit 17
    {CAM_DUMMY_   },  // bit 18
    {CAM_DUMMY_   },  // bit 19
    {CAM_DUMMY_   },  // bit 20
    {CAM_DUMMY_   },  // bit 21
    {CAM_DUMMY_   },  // bit 22
    {CAM_DUMMY_   },  // bit 23
    {CAM_DUMMY_   },  // bit 24
    {CAM_DUMMY_   },  // bit 25
    {CAM_DUMMY_   },  // bit 26
    {CAM_DUMMY_   },  // bit 27
    {CAM_DUMMY_   },  // bit 28
    {CAM_DUMMY_   },  // bit 29
    {CAM_DUMMY_   },  // bit 30
    {CAM_DUMMY_   }}; // bit 31


/**************************************************************************
* Member Variable Initilization
**************************************************************************/

MUINT32 *IspDrv::mpIspHwRegAddr = NULL;
MINT32  IspDrv::mIspVirRegFd;
MUINT32 *IspDrv::mpIspVirRegBuffer;
MUINT32 IspDrv::mIspVirRegSize;
MUINT32 *IspDrv::mpIspVirRegAddrVA[ISP_DRV_CQ_NUM];
MUINT32 *IspDrv::mpIspVirRegAddrPA[ISP_DRV_CQ_NUM];
MINT32  IspDrv::mIspCQDescFd;
MUINT32 *IspDrv::mpIspCQDescBufferVirt = NULL;
MUINT32 IspDrv::mIspCQDescSize;
MUINT32 IspDrv::mpIspCQDescBufferPhy = 0;	// Fix build warning
ISP_DRV_CQ_CMD_DESC_STRUCT *IspDrv::mpIspCQDescriptorVirt[ISP_DRV_DESCRIPTOR_CQ_NUM];
MUINT32                     IspDrv::mpIspCQDescriptorPhy[ISP_DRV_DESCRIPTOR_CQ_NUM];

ISP_TURNING_UPDATE_INFO IspDrv::tdriMgrInfoCq1;
ISP_TURNING_UPDATE_INFO IspDrv::tdriMgrInfoCq2;

/*******************************************************************************
*
********************************************************************************/
IspDrv *IspDrv::createInstance(MBOOL fgIsGdmaMode)
{
    DBG_LOG_CONFIG(drv, isp_drv);
    
    //LOG_INF("fgIsGdmaMode: %d.", fgIsGdmaMode);
    return IspDrvImp::getInstance(fgIsGdmaMode);
}

/*******************************************************************************
*
********************************************************************************/
IspDrv *IspDrv::getCQInstance(ISP_DRV_CQ_ENUM cq)
{
    //LOG_DBG("");
    return IspDrvVirImp::getInstance(cq,(MUINT32)mpIspVirRegAddrVA[cq] );
}

/*******************************************************************************
*
********************************************************************************/
MBOOL IspDrv::cqAddModule(ISP_DRV_CQ_ENUM cq, CAM_MODULE_ENUM moduleId)
{
    MINT32 cmd;
    LOG_DBG("+,isp_cq[0x%x],descri_cq[0x%x],[%d]",cq,gIspDrvCqMappinig[cq].descriptorCq,moduleId);
    
    Mutex::Autolock lock(cqVirDesLock);

    cmd = (gIspCQModuleInfo[moduleId].addr_ofst&0xffff) | (((gIspCQModuleInfo[moduleId].reg_num - 1) & 0x3ff) << 16) | ((ISP_CQ_WRITE_INST) << 26);
    
    mpIspCQDescriptorVirt[gIspDrvCqMappinig[cq].descriptorCq][moduleId].v_reg_addr = (MUINT32)&mpIspVirRegAddrPA[cq][gIspCQModuleInfo[moduleId].addr_ofst >> 2]; // >>2 for MUINT32* pointer
    mpIspCQDescriptorVirt[gIspDrvCqMappinig[cq].descriptorCq][moduleId].u.cmd = cmd;
	
	LOG_DBG("-");
    return MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL IspDrv::cqDelModule(ISP_DRV_CQ_ENUM cq, CAM_MODULE_ENUM moduleId)
{
    LOG_DBG("+,isp_cq[0x%x] descri_cq[0x%x],[%d]",cq,gIspDrvCqMappinig[cq].descriptorCq,moduleId);
    
    Mutex::Autolock lock(cqVirDesLock);
	mpIspCQDescriptorVirt[gIspDrvCqMappinig[cq].descriptorCq][moduleId].u.cmd = ISP_DRV_CQ_DUMMY_WR_TOKEN;
    
	LOG_DBG("-");
    return MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
MUINT32 IspDrv::getCQDescBufPhyAddr(ISP_DRV_CQ_ENUM cq)
{
    Mutex::Autolock lock(cqPhyDesLock);

    return (MUINT32)mpIspCQDescriptorPhy[gIspDrvCqMappinig[cq].descriptorCq];
}

/*******************************************************************************
*
********************************************************************************/
MBOOL IspDrv::setCQTriggerMode(ISP_DRV_CQ_ENUM cq,
                                      ISP_DRV_CQ_TRIGGER_MODE_ENUM mode,
                                      ISP_DRV_CQ_TRIGGER_SOURCE_ENUM trig_src)
{
    isp_reg_t *pIspReg = (isp_reg_t *)getRegAddr();

    LOG_DBG("+,[%s],cq(%d),mode(%d),trig_src(%d)",__FUNCTION__, cq, mode, trig_src);
    
    switch(cq)
    {
        case ISP_DRV_CQ0:
            //trigger source is pass1_done
            if(CQ_SINGLE_IMMEDIATE_TRIGGER == mode) 
            {
                //-Immediately trigger
                //-CQ0_MODE=1(reg_4018[17]), CQ0_CONT=0(reg_4018[2])
                if(IsReadOnlyMode())
                {
                    ISP_IOCTL_WRITE_ENABLE_BITS(this, this->getRegAddrMap(), CAM_CTL_SEL, CQ0_MODE, 1);
                    ISP_IOCTL_WRITE_ENABLE_BITS(this, this->getRegAddrMap(), CAM_CTL_SEL, CQ0_CONT, 0);
                }
                else
                {
                    ISP_WRITE_ENABLE_BITS(pIspReg, CAM_CTL_SEL, CQ0_MODE, 1);
                    ISP_WRITE_ENABLE_BITS(pIspReg, CAM_CTL_SEL, CQ0_CONT, 0);
                }
            }
            else if(CQ_SINGLE_EVENT_TRIGGER == mode) 
            {
                //-Trigger and wait trigger source
                //-CQ0_MODE=0, CQ0_CONT=0
                if(IsReadOnlyMode())
                {
                    ISP_IOCTL_WRITE_ENABLE_BITS(this, this->getRegAddrMap(), CAM_CTL_SEL, CQ0_MODE, 0);
                    ISP_IOCTL_WRITE_ENABLE_BITS(this, this->getRegAddrMap(), CAM_CTL_SEL, CQ0_CONT, 0);
                }
                else
                {
                    ISP_WRITE_ENABLE_BITS(pIspReg, CAM_CTL_SEL, CQ0_MODE, 0);
                    ISP_WRITE_ENABLE_BITS(pIspReg, CAM_CTL_SEL, CQ0_CONT, 0);
                }
            }
            else if(CQ_CONTINUOUS_EVENT_TRIGGER == mode)
            {
                //-Continuous mode support(without trigger)
                //-CQ0_MODE=x, CQ0_CONT=1
                if(IsReadOnlyMode())
                {
                    ISP_IOCTL_WRITE_ENABLE_BITS(this, this->getRegAddrMap(), CAM_CTL_SEL, CQ0_MODE, 0);
                    ISP_IOCTL_WRITE_ENABLE_BITS(this, this->getRegAddrMap(), CAM_CTL_SEL, CQ0_CONT, 1);
                }
                else
                {
                    ISP_WRITE_ENABLE_BITS(pIspReg, CAM_CTL_SEL, CQ0_MODE, 0);
                    ISP_WRITE_ENABLE_BITS(pIspReg, CAM_CTL_SEL, CQ0_CONT, 1);
                }
            }
            break;
        case ISP_DRV_CQ0B:
            //-choose trigger source by CQ0B_SEL(0:img2o, 1:pass1_done)(reg_4018[11])
            if(CQ_TRIG_BY_PASS1_DONE != trig_src && CQ_TRIG_BY_IMGO_DONE != trig_src)
            {
                LOG_ERR("[%s][ISP_DRV_CQ0B]:NOT Support trigger source",__FUNCTION__);
            }

            if(CQ_TRIG_BY_PASS1_DONE == trig_src)
            {
                if(IsReadOnlyMode())
                {
                    ISP_IOCTL_WRITE_ENABLE_BITS(this, this->getRegAddrMap(), CAM_CTL_SEL, CQ0B_SEL, 1);
                }
                else
                {
                    ISP_WRITE_ENABLE_BITS(pIspReg, CAM_CTL_SEL, CQ0B_SEL, 1);
                }
            }
            else
            {
                if(IsReadOnlyMode())
                {
                    ISP_IOCTL_WRITE_ENABLE_BITS(this, this->getRegAddrMap(), CAM_CTL_SEL, CQ0B_SEL, 0);
                }
                else
                {
                    ISP_WRITE_ENABLE_BITS(pIspReg, CAM_CTL_SEL, CQ0B_SEL, 0);
                }
            }
            
            if(CQ_SINGLE_IMMEDIATE_TRIGGER == mode)
            {
                //-Immediately trigger
                //-CQ0B_MODE=1 reg_4018[25], CQ0B_CONT=0 reg_4018[3]
                if(IsReadOnlyMode())
                {
                    ISP_IOCTL_WRITE_ENABLE_BITS(this, this->getRegAddrMap(), CAM_CTL_SEL, CQ0B_MODE, 1);
                    ISP_IOCTL_WRITE_ENABLE_BITS(this, this->getRegAddrMap(), CAM_CTL_SEL, CQ0B_CONT, 0);
                }
                else
                {
                    ISP_WRITE_ENABLE_BITS(pIspReg, CAM_CTL_SEL, CQ0B_MODE, 1);
                    ISP_WRITE_ENABLE_BITS(pIspReg, CAM_CTL_SEL, CQ0B_CONT, 0);
                }
            }
            else if(CQ_SINGLE_EVENT_TRIGGER == mode)
            {
                //-Trigger and wait trigger source
                //-CQ0B_MODE=0, CQ0B_CONT=0
                if(IsReadOnlyMode())
                {
                    ISP_IOCTL_WRITE_ENABLE_BITS(this, this->getRegAddrMap(), CAM_CTL_SEL, CQ0B_MODE, 0);
                    ISP_IOCTL_WRITE_ENABLE_BITS(this, this->getRegAddrMap(), CAM_CTL_SEL, CQ0B_CONT, 0);
                }
                else
                {
                    ISP_WRITE_ENABLE_BITS(pIspReg, CAM_CTL_SEL, CQ0B_MODE, 0);
                    ISP_WRITE_ENABLE_BITS(pIspReg, CAM_CTL_SEL, CQ0B_CONT, 0);
                }
            }
            else if(CQ_CONTINUOUS_EVENT_TRIGGER == mode)
            {
                //-Continuous mode support(without trigger)
                //-CQ0B_MODE=x, CQ0B_CONT=1
                if(IsReadOnlyMode())
                {
                    ISP_IOCTL_WRITE_ENABLE_BITS(this, this->getRegAddrMap(), CAM_CTL_SEL, CQ0B_MODE, 0);
                    ISP_IOCTL_WRITE_ENABLE_BITS(this, this->getRegAddrMap(), CAM_CTL_SEL, CQ0B_CONT, 1);
                }
                else
                {
                    ISP_WRITE_ENABLE_BITS(pIspReg, CAM_CTL_SEL, CQ0B_MODE, 0);
                    ISP_WRITE_ENABLE_BITS(pIspReg, CAM_CTL_SEL, CQ0B_CONT, 1);
                }
            }
            break;
        case ISP_DRV_CQ0C:
            //-cq0c
            //    -trigger source is imgo_done (CQ0C_IMGO_SEL_SET=1,reg_40A0[7])
            //                    OR img20_done(CQ0C_IMGO_SEL_SET=1,reg_40A0[22]).
            //    -always continuous mode,NO cq0c_start.
            //     If cq0c_en=1, it always load CQ when imgo_done||img2o_done occur
            if ( CQ_TRIG_BY_IMGO_DONE != trig_src && CQ_TRIG_BY_IMG2O_DONE != trig_src)
            {
                LOG_ERR("[%s][ISP_DRV_CQ0C]:NOT Support trigger source\n",__FUNCTION__);
            }

            if(CQ_TRIG_BY_IMGO_DONE == trig_src)
            {
                if(IsReadOnlyMode())
                {
                    ISP_IOCTL_WRITE_ENABLE_BITS(this, this->getRegAddrMap(), CAM_CTL_SEL_SET, CQ0C_IMGO_SEL_SET, 1);
                    ISP_IOCTL_WRITE_ENABLE_BITS(this, this->getRegAddrMap(), CAM_CTL_SEL_CLR, CQ0C_IMGO_SEL_CLR, 0);
                }
                else
                {
                    ISP_WRITE_ENABLE_BITS(pIspReg, CAM_CTL_SEL_SET, CQ0C_IMGO_SEL_SET, 1);
                    ISP_WRITE_ENABLE_BITS(pIspReg, CAM_CTL_SEL_CLR, CQ0C_IMGO_SEL_CLR, 0);
                }
            }
            else
            {
                if(IsReadOnlyMode())
                {
                    ISP_IOCTL_WRITE_ENABLE_BITS(this, this->getRegAddrMap(), CAM_CTL_SEL_SET, CQ0C_IMGO_SEL_SET, 0);
                    ISP_IOCTL_WRITE_ENABLE_BITS(this, this->getRegAddrMap(), CAM_CTL_SEL_CLR, CQ0C_IMGO_SEL_CLR, 1);
                }
                else
                {
                    ISP_WRITE_ENABLE_BITS(pIspReg, CAM_CTL_SEL_SET, CQ0C_IMGO_SEL_SET, 0);
                    ISP_WRITE_ENABLE_BITS(pIspReg, CAM_CTL_SEL_CLR, CQ0C_IMGO_SEL_CLR, 1);
                }
            }

            if(CQ_TRIG_BY_IMG2O_DONE == trig_src)
            {
                if(IsReadOnlyMode())
                {
                    ISP_IOCTL_WRITE_ENABLE_BITS(this, this->getRegAddrMap(), CAM_CTL_SEL_SET, CQ0C_IMG2O_SEL_SET, 1);
                    ISP_IOCTL_WRITE_ENABLE_BITS(this, this->getRegAddrMap(), CAM_CTL_SEL_CLR, CQ0C_IMG2O_SEL_CLR, 0);
                }
                else
                {
                    ISP_WRITE_ENABLE_BITS(pIspReg, CAM_CTL_SEL_SET, CQ0C_IMG2O_SEL_SET, 1);
                    ISP_WRITE_ENABLE_BITS(pIspReg, CAM_CTL_SEL_CLR, CQ0C_IMG2O_SEL_CLR, 0);
                }
            }
            else
            {
                if(IsReadOnlyMode())
                {
                    ISP_IOCTL_WRITE_ENABLE_BITS(this, this->getRegAddrMap(), CAM_CTL_SEL_SET, CQ0C_IMG2O_SEL_SET, 0);
                    ISP_IOCTL_WRITE_ENABLE_BITS(this, this->getRegAddrMap(), CAM_CTL_SEL_CLR, CQ0C_IMG2O_SEL_CLR, 1);
                }
                else
                {
                    ISP_WRITE_ENABLE_BITS(pIspReg, CAM_CTL_SEL_SET, CQ0C_IMG2O_SEL_SET, 0);
                    ISP_WRITE_ENABLE_BITS(pIspReg, CAM_CTL_SEL_CLR, CQ0C_IMG2O_SEL_CLR, 1);
                }
            }

            break;
        case ISP_DRV_CQ01:
        case ISP_DRV_CQ01_SYNC:
        case ISP_DRV_CQ02:
        case ISP_DRV_CQ02_SYNC:
        case ISP_DRV_CQ03:
        default:
            //-cq1/2/3
            //    -load one time. working time is at right after set pass2x_start=1
            //    -Immediately trigger ONLY
            break;
    }

    return MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
MVOID IspDrv::setCallbacks(CALLBACKFUNC_ISP_DRV pCameraIOCB, MVOID *user)
{
    LOG_DBG("cb(0x%x),cookie(0x%x)", (MUINT32) pCameraIOCB, (MUINT32)user);
    
    if(NULL == pCameraIOCB)
    {
        mpIspDrvCB = NULL;
        mCallbackCookie = NULL;
    }
    else
    {
        mpIspDrvCB = pCameraIOCB;
        mCallbackCookie = user;
    }
}

/*******************************************************************************
* only be called by tdriMgr (prtect by tdriMgr)
********************************************************************************/
MBOOL IspDrv::flushTurnCqTable(MINT32 cq)
{
    if(cq == ISP_DRV_CQ01) 
    {
        mpIspVirRegAddrVA[ISP_DRV_CQ01_SYNC][(0x4080 >> 2)] = 0x00;  // set en1
        mpIspVirRegAddrVA[ISP_DRV_CQ01_SYNC][(0x4084 >> 2)] = 0x00;  // clr en1
        mpIspVirRegAddrVA[ISP_DRV_CQ01_SYNC][(0x4088 >> 2)] = 0x00;  // set en2
        mpIspVirRegAddrVA[ISP_DRV_CQ01_SYNC][(0x408C >> 2)] = 0x00;  // clr en2
        mpIspVirRegAddrVA[ISP_DRV_CQ01_SYNC][(0x4090 >> 2)] = 0x00;  // set dma
        mpIspVirRegAddrVA[ISP_DRV_CQ01_SYNC][(0x4094 >> 2)] = 0x00;  // clr dma        
    } 
    else if(cq == ISP_DRV_CQ02) 
    {
        mpIspVirRegAddrVA[ISP_DRV_CQ02_SYNC][(0x4080 >> 2)] = 0x00;  // set en1
        mpIspVirRegAddrVA[ISP_DRV_CQ02_SYNC][(0x4084 >> 2)] = 0x00;  // clr en1
        mpIspVirRegAddrVA[ISP_DRV_CQ02_SYNC][(0x4088 >> 2)] = 0x00;  // set en2
        mpIspVirRegAddrVA[ISP_DRV_CQ02_SYNC][(0x408C >> 2)] = 0x00;  // clr en2
        mpIspVirRegAddrVA[ISP_DRV_CQ02_SYNC][(0x4090 >> 2)] = 0x00;  // set dma
        mpIspVirRegAddrVA[ISP_DRV_CQ02_SYNC][(0x4094 >> 2)] = 0x00;  // clr dma
    }
    
    return MTRUE;
}

/*******************************************************************************
* 
********************************************************************************/
MBOOL IspDrv::setTurnTopEn1(MINT32 cq, MINT32 en1)
{
    MINT32 i;
    LOG_DBG("cq(%d),en1(0x%x)E.",cq,en1);
    
    if(cq == ISP_DRV_CQ01) 
    {
        LOG_DBG("KeepEn1(0x%x),preEn1(0x%x)",tdriMgrInfoCq1.topCtlEn1Keep,tdriMgrInfoCq1.topCtlEn1PrevKeep);
        
        turnKeepCq1Lock.lock();
        
        tdriMgrInfoCq1.topCtlEn1Keep = en1;
        
        for(i = 0;i < ISP_INT_BIT_NUM; i++)
        {
            if(en1 == ISP_DRV_TURNING_TOP_RESET)
            {
                if(gIspTurnEn1Mapp[i].grop1 != CAM_DUMMY_)
                {
                    LOG_DBG("del dma G2(%d)",gIspTurnEn1Mapp[i].grop1);
                    cqDelModule(ISP_DRV_CQ01, gIspTurnEn1Mapp[i].grop1);
                }
                
                if(gIspTurnEn1Mapp[i].grop2 != CAM_DUMMY_)
                {
                    LOG_DBG("del dma G2(%d)",gIspTurnEn1Mapp[i].grop2);
                    cqDelModule(ISP_DRV_CQ01, gIspTurnEn1Mapp[i].grop2);
                }
                
                if(gIspTurnEn1Mapp[i].grop3 != CAM_DUMMY_)
                {
                    LOG_DBG("del dma G2(%d)",gIspTurnEn1Mapp[i].grop3);
                    cqDelModule(ISP_DRV_CQ01, gIspTurnEn1Mapp[i].grop3);
                }

            } 
            else if((tdriMgrInfoCq1.topCtlEn1PrevKeep==ISP_DRV_TURNING_TOP_RESET) ||
                     (((tdriMgrInfoCq1.topCtlEn1Keep >> i) & 0x01) != ((tdriMgrInfoCq1.topCtlEn1PrevKeep >> i) & 0x01)))
            {
                if(gIspTurnEn1Mapp[i].grop1 != CAM_DUMMY_)
                {
                    if((en1 >> i) & 0x01)
                    {
                        LOG_DBG("cq(%d),add EN1 G1(%d)",cq,gIspTurnEn1Mapp[i].grop1);
                        cqAddModule(ISP_DRV_CQ01, gIspTurnEn1Mapp[i].grop1);
                    }
                    else
                    {
                        LOG_DBG("cq(%d),del EN1 G1(%d)",cq,gIspTurnEn1Mapp[i].grop1);
                        cqDelModule(ISP_DRV_CQ01, gIspTurnEn1Mapp[i].grop1);
                    }
                }
                
                if(gIspTurnEn1Mapp[i].grop2 != CAM_DUMMY_)
                {
                    if((en1 >> i) & 0x01)
                    {
                        LOG_DBG("cq(%d),add EN1 G2(%d)",cq,gIspTurnEn1Mapp[i].grop2);
                        cqAddModule(ISP_DRV_CQ01, gIspTurnEn1Mapp[i].grop2);
                    }
                    else
                    {
                        LOG_DBG("cq(%d),del EN1 G2(%d)",cq,gIspTurnEn1Mapp[i].grop2);
                        cqDelModule(ISP_DRV_CQ01, gIspTurnEn1Mapp[i].grop2);
                    }
                }
                
                if(gIspTurnEn1Mapp[i].grop3 != CAM_DUMMY_)
                {
                    if((en1 >> i) & 0x01)
                    {
                        LOG_DBG("cq(%d),add EN1 G3(%d)",cq,gIspTurnEn1Mapp[i].grop3);
                        cqAddModule(ISP_DRV_CQ01, gIspTurnEn1Mapp[i].grop3);
                    }
                    else
                    {
                        LOG_DBG("cq(%d),del EN1 G3(%d)",cq,gIspTurnEn1Mapp[i].grop3);
                        cqDelModule(ISP_DRV_CQ01, gIspTurnEn1Mapp[i].grop3);
                    }
                }
            }
        }
        
        if (en1 != ISP_DRV_TURNING_TOP_RESET)
        {
            tdriMgrInfoCq1.topCtlEn1PrevKeep = tdriMgrInfoCq1.topCtlEn1Keep;
        }
        else
        {
            tdriMgrInfoCq1.topCtlEn1PrevKeep = tdriMgrInfoCq1.topCtlEn1Keep;
            tdriMgrInfoCq1.topCtlEn1Keep = 0x00;
        }
        
        turnKeepCq1Lock.unlock();
    } 
    else if(cq == ISP_DRV_CQ02)
    {
        turnKeepCq2Lock.lock();
        
        tdriMgrInfoCq2.topCtlEn1Keep = en1;
        
        for(i = 0;i < ISP_INT_BIT_NUM; i++) 
        {
            if(en1 == ISP_DRV_TURNING_TOP_RESET)
            {
                if(gIspTurnEn1Mapp[i].grop1 != CAM_DUMMY_)
                {
                    LOG_DBG("del dma G2(%d)",gIspTurnEn1Mapp[i].grop1);
                    cqDelModule(ISP_DRV_CQ01, gIspTurnEn1Mapp[i].grop1);
                }
                
                if(gIspTurnEn1Mapp[i].grop2 != CAM_DUMMY_)
                {
                    LOG_DBG("del dma G2(%d)",gIspTurnEn1Mapp[i].grop2);
                    cqDelModule(ISP_DRV_CQ01, gIspTurnEn1Mapp[i].grop2);
                }
                
                if(gIspTurnEn1Mapp[i].grop3 != CAM_DUMMY_)
                {
                    LOG_DBG("del dma G2(%d)",gIspTurnEn1Mapp[i].grop3);
                    cqDelModule(ISP_DRV_CQ01, gIspTurnEn1Mapp[i].grop3);
                }
            } 
            else if((tdriMgrInfoCq2.topCtlEn1PrevKeep==ISP_DRV_TURNING_TOP_RESET) ||
                     (((tdriMgrInfoCq2.topCtlEn1Keep>>i)&0x01)!=((tdriMgrInfoCq2.topCtlEn1PrevKeep>>i)&0x01)))
            {
                if(gIspTurnEn1Mapp[i].grop1 != CAM_DUMMY_)
                {
                    if((en1 >> i) & 0x01)
                    {
                        LOG_DBG("cq(%d),add EN1 G1(%d)",cq,gIspTurnEn1Mapp[i].grop1);
                        cqAddModule(ISP_DRV_CQ02, gIspTurnEn1Mapp[i].grop1);
                    }
                    else
                    {
                        LOG_DBG("cq(%d),del EN1 G1(%d)",cq,gIspTurnEn1Mapp[i].grop1);
                        cqDelModule(ISP_DRV_CQ02, gIspTurnEn1Mapp[i].grop1);
                    }
                }
                
                if(gIspTurnEn1Mapp[i].grop2 != CAM_DUMMY_)
                {
                    if((en1 >> i) & 0x01)
                    {
                        LOG_DBG("cq(%d),add EN1 G2(%d)",cq,gIspTurnEn1Mapp[i].grop2);
                        cqAddModule(ISP_DRV_CQ02, gIspTurnEn1Mapp[i].grop2);
                    }
                    else
                    {
                        LOG_DBG("cq(%d),del EN1 G2(%d)",cq,gIspTurnEn1Mapp[i].grop2);
                        cqDelModule(ISP_DRV_CQ02, gIspTurnEn1Mapp[i].grop2);
                    }
                }
                
                if(gIspTurnEn1Mapp[i].grop3 != CAM_DUMMY_)
                {
                    if((en1 >> i) & 0x01)
                    {
                        LOG_DBG("cq(%d),add EN1 G3(%d)",cq,gIspTurnEn1Mapp[i].grop3);
                        cqAddModule(ISP_DRV_CQ02, gIspTurnEn1Mapp[i].grop3);
                    }
                    else
                    {
                        LOG_DBG("cq(%d),del EN1 G3(%d)",cq,gIspTurnEn1Mapp[i].grop3);
                        cqDelModule(ISP_DRV_CQ02, gIspTurnEn1Mapp[i].grop3);
                    }
                }
            }
        }
        
        if(en1 != ISP_DRV_TURNING_TOP_RESET)
        {
            tdriMgrInfoCq2.topCtlEn1PrevKeep = tdriMgrInfoCq2.topCtlEn1Keep;
        } 
        else
        {
            tdriMgrInfoCq2.topCtlEn1PrevKeep = tdriMgrInfoCq2.topCtlEn1Keep;
            tdriMgrInfoCq2.topCtlEn1Keep = 0x00;
        }
        
        turnKeepCq2Lock.unlock();
    } 
    else 
    {
        LOG_ERR("[ERROR]cq(%d) not support this fun!",cq);
    }
    
    LOG_DBG("-");
    return 0;
}

/*******************************************************************************
* 
********************************************************************************/
MBOOL IspDrv::setTurnTopEn2(MINT32 cq, MINT32 en2)
{
    MINT32 i;
    
    LOG_DBG("+,cq(%d),en2(0x%x)",cq,en2);

    if(cq == ISP_DRV_CQ01)
    {
        LOG_DBG("KeepEn2(0x%x),preEn2(0x%x)",tdriMgrInfoCq1.topCtlEn2Keep,tdriMgrInfoCq1.topCtlEn2PrevKeep);
        
        turnKeepCq1Lock.lock();
        
        tdriMgrInfoCq1.topCtlEn2Keep = en2;
        
        for(i = 0; i < ISP_INT_BIT_NUM; i++)
        {
            if(en2 == ISP_DRV_TURNING_TOP_RESET)
            {
                if(gIspTurnEn2Mapp[i].grop1 != CAM_DUMMY_)
                {
                    LOG_DBG("del dma G2(%d)",gIspTurnEn2Mapp[i].grop1);
                    cqDelModule(ISP_DRV_CQ01, gIspTurnEn2Mapp[i].grop1);
                }
                
                if(gIspTurnEn2Mapp[i].grop2 != CAM_DUMMY_)
                {
                    LOG_DBG("del dma G2(%d)",gIspTurnEn2Mapp[i].grop2);
                    cqDelModule(ISP_DRV_CQ01, gIspTurnEn2Mapp[i].grop2);
                }

            } 
            else if((tdriMgrInfoCq1.topCtlEn2PrevKeep==ISP_DRV_TURNING_TOP_RESET) ||
                     (((tdriMgrInfoCq1.topCtlEn2Keep>>i)&0x01)!=((tdriMgrInfoCq1.topCtlEn2PrevKeep>>i)&0x01)))
            {
                if(gIspTurnEn2Mapp[i].grop1 != CAM_DUMMY_)
                {
                    if((en2 >> i) & 0x01)
                    {
                        LOG_DBG("cq(%d),add EN2 G1(%d)",cq,gIspTurnEn2Mapp[i].grop1);
                        cqAddModule(ISP_DRV_CQ01, gIspTurnEn2Mapp[i].grop1);
                    }
                    else
                    {
                        LOG_DBG("cq(%d),del EN2 G1(%d)",cq,gIspTurnEn2Mapp[i].grop1);
                        cqDelModule(ISP_DRV_CQ01, gIspTurnEn2Mapp[i].grop1);
                    }
                }
                
                if(gIspTurnEn2Mapp[i].grop2 != CAM_DUMMY_)
                {
                    if((en2>>i)&0x01)
                    {
                        LOG_DBG("cq(%d),add EN2 G2(%d)",cq,gIspTurnEn2Mapp[i].grop2);
                        cqAddModule(ISP_DRV_CQ01, gIspTurnEn2Mapp[i].grop2);
                    }
                    else
                    {
                        LOG_DBG("cq(%d),del EN2 G2(%d)",cq,gIspTurnEn2Mapp[i].grop2);
                        cqDelModule(ISP_DRV_CQ01, gIspTurnEn2Mapp[i].grop2);
                    }
                }
            }
        }
        
        if(en2 != ISP_DRV_TURNING_TOP_RESET)
        {
            tdriMgrInfoCq1.topCtlEn2PrevKeep = tdriMgrInfoCq1.topCtlEn2Keep;
        }
        else
        {
            tdriMgrInfoCq1.topCtlEn2PrevKeep = tdriMgrInfoCq1.topCtlEn2Keep;
            tdriMgrInfoCq1.topCtlEn2Keep = 0x00;
        }
        
        turnKeepCq1Lock.unlock();
    } 
    else if(cq == ISP_DRV_CQ02)
    {
        turnKeepCq2Lock.lock();
        
        tdriMgrInfoCq2.topCtlEn2Keep = en2;
        
        for(i = 0; i < ISP_INT_BIT_NUM; i++)
        {
            if(en2 == ISP_DRV_TURNING_TOP_RESET)
            {
                if(gIspTurnEn2Mapp[i].grop1 != CAM_DUMMY_)
                {
                    LOG_DBG("del dma G2(%d)",gIspTurnEn2Mapp[i].grop1);
                    cqDelModule(ISP_DRV_CQ02, gIspTurnEn2Mapp[i].grop1);
                }
                
                if(gIspTurnEn2Mapp[i].grop2 != CAM_DUMMY_)
                {
                    LOG_DBG("del dma G2(%d)",gIspTurnEn2Mapp[i].grop2);
                    cqDelModule(ISP_DRV_CQ02, gIspTurnEn2Mapp[i].grop2);
                }
            } 
            else if((tdriMgrInfoCq2.topCtlEn2PrevKeep==ISP_DRV_TURNING_TOP_RESET) ||
                     (((tdriMgrInfoCq2.topCtlEn2Keep>>i)&0x01)!=((tdriMgrInfoCq2.topCtlEn2PrevKeep>>i)&0x01)))
            {
                if(gIspTurnEn2Mapp[i].grop1 != CAM_DUMMY_)
                {
                    if((en2 >> i) & 0x01)
                    {
                        LOG_DBG("cq(%d),add EN2 G1(%d)",cq,gIspTurnEn2Mapp[i].grop1);
                        cqAddModule(ISP_DRV_CQ02, gIspTurnEn2Mapp[i].grop1);
                    }
                    else
                    {
                        LOG_DBG("cq(%d),del EN2 G1(%d)",cq,gIspTurnEn2Mapp[i].grop1);
                        cqDelModule(ISP_DRV_CQ02, gIspTurnEn2Mapp[i].grop1);
                    }
                }
                
                if(gIspTurnEn2Mapp[i].grop2 != CAM_DUMMY_)
                {
                    if((en2 >> i) & 0x01)
                    {
                        LOG_DBG("cq(%d),add EN2 G2(%d)",cq,gIspTurnEn2Mapp[i].grop2);
                        cqAddModule(ISP_DRV_CQ02, gIspTurnEn2Mapp[i].grop2);
                    }
                    else
                    {
                        LOG_DBG("cq(%d),del EN2 G2(%d)",cq,gIspTurnEn2Mapp[i].grop2);
                        cqDelModule(ISP_DRV_CQ02, gIspTurnEn2Mapp[i].grop2);
                    }
                }
            }
        }
        
        if(en2 != ISP_DRV_TURNING_TOP_RESET)
        {
            tdriMgrInfoCq2.topCtlEn2PrevKeep = tdriMgrInfoCq2.topCtlEn2Keep;
        }
        else
        {
            tdriMgrInfoCq2.topCtlEn2PrevKeep = tdriMgrInfoCq2.topCtlEn2Keep;
            tdriMgrInfoCq2.topCtlEn2Keep = 0x00;
        }
        
        turnKeepCq2Lock.unlock();
    } 
    else
    {
        LOG_ERR("[ERROR]cq(%d) not support this fun!",cq);
    }
    
    LOG_DBG("-");
    return 0;
}

/*******************************************************************************
* 
********************************************************************************/
MBOOL IspDrv::setTurnTopDma(MINT32 cq, MINT32 dma)
{
    MINT32 i;
    
    LOG_DBG("+,cq(%d),dma(0x%x)\n",cq,dma);

    if(cq == ISP_DRV_CQ01)
    {
        turnKeepCq1Lock.lock();
       
        tdriMgrInfoCq1.topCtlDmaKeep = dma;

        LOG_DBG("KeepDma(0x%x) preDma(0x%x)",tdriMgrInfoCq1.topCtlDmaKeep,tdriMgrInfoCq1.topCtlDmaPrevKeep);

        for(i = 0; i < ISP_INT_BIT_NUM; i++)
        {
            if(dma == ISP_DRV_TURNING_TOP_RESET)
            {
                if(gIspTurnDmaMapp[i].grop1 != CAM_DUMMY_)
                {
                    LOG_DBG("del dma G2(%d)",gIspTurnDmaMapp[i].grop1);
                    cqDelModule(ISP_DRV_CQ01, gIspTurnDmaMapp[i].grop1);
                }
            }
            else if((tdriMgrInfoCq1.topCtlDmaPrevKeep==ISP_DRV_TURNING_TOP_RESET) ||
                     (((tdriMgrInfoCq1.topCtlDmaKeep>>i)&0x01)!=((tdriMgrInfoCq1.topCtlDmaPrevKeep>>i)&0x01)))
            {
                if(gIspTurnDmaMapp[i].grop1 != CAM_DUMMY_)
                {
                    if((dma >> i) & 0x01)
                    {
                        LOG_DBG("add dma G2(%d)",gIspTurnDmaMapp[i].grop1);
                        cqAddModule(ISP_DRV_CQ01, gIspTurnDmaMapp[i].grop1);
                        //if(CAM_DMA_IMGCI==gIspTurnDmaMapp[i].grop1) //for error check
                        //    LOG_ERR("[Warning]IMGCI be enabled!");
                    }
                    else
                    {
                        LOG_DBG("del dma G2(%d)",gIspTurnDmaMapp[i].grop1);
                        cqDelModule(ISP_DRV_CQ01, gIspTurnDmaMapp[i].grop1);
                    }
                }
            }
        }
        
        if(dma != ISP_DRV_TURNING_TOP_RESET)
        {
            tdriMgrInfoCq1.topCtlDmaPrevKeep = tdriMgrInfoCq1.topCtlDmaKeep;
        }
        else
        {
            tdriMgrInfoCq1.topCtlDmaPrevKeep = tdriMgrInfoCq1.topCtlDmaKeep;
            tdriMgrInfoCq1.topCtlDmaKeep = 0x00;
        }
        
        turnKeepCq1Lock.unlock();
    } 
    else if(cq == ISP_DRV_CQ02)
    {
        turnKeepCq2Lock.lock();
        
        tdriMgrInfoCq2.topCtlDmaKeep = dma;
        
        for(i = 0; i < ISP_INT_BIT_NUM; i++)
        {
            if(dma == ISP_DRV_TURNING_TOP_RESET)
            {
                if(gIspTurnDmaMapp[i].grop1 != CAM_DUMMY_)
                {
                    LOG_DBG("del dma G2(%d)",gIspTurnDmaMapp[i].grop1);
                    cqDelModule(ISP_DRV_CQ01, gIspTurnDmaMapp[i].grop1);
                }
            }
            else if((tdriMgrInfoCq2.topCtlDmaPrevKeep==ISP_DRV_TURNING_TOP_RESET) ||
                     (((tdriMgrInfoCq2.topCtlDmaKeep>>i)&0x01)!=((tdriMgrInfoCq2.topCtlDmaPrevKeep>>i)&0x01)))
            {
                if(gIspTurnDmaMapp[i].grop1 != CAM_DUMMY_)
                {
                    if((dma >> i) & 0x01)
                    {
                        LOG_DBG("add dma G2(%d)",gIspTurnDmaMapp[i].grop1);
                        cqAddModule(ISP_DRV_CQ02, gIspTurnDmaMapp[i].grop1);
                        //if(CAM_DMA_IMGCI==gIspTurnDmaMapp[i].grop1) //for error check
                        //    LOG_WRN("[Warning]IMGCI be enabled!");
                    }
                    else
                    {
                        LOG_DBG("del dma G2(%d)",gIspTurnDmaMapp[i].grop1);
                        cqDelModule(ISP_DRV_CQ02, gIspTurnDmaMapp[i].grop1);
                    }
                }
            }
        }
        
        if(dma != ISP_DRV_TURNING_TOP_RESET)
        {
            tdriMgrInfoCq2.topCtlDmaPrevKeep = tdriMgrInfoCq2.topCtlDmaKeep;
        } 
        else
        {
            tdriMgrInfoCq2.topCtlDmaPrevKeep = tdriMgrInfoCq2.topCtlDmaKeep;
            tdriMgrInfoCq2.topCtlDmaKeep = 0x00;
        }
        
        turnKeepCq2Lock.unlock();
    } 
    else
    {
        LOG_ERR("[ERROR]cq(%d) not support this fun!\n",cq);
    }
    
    LOG_DBG("-");
    return 0;
}

/*******************************************************************************
* 
********************************************************************************/
MUINT32 IspDrv::getTurnTopEn1(MINT32 cq)
{
    LOG_DBG("+,cq(%d)", cq);
#if 1 //ic.hsu
    if(cq == ISP_DRV_CQ01)
    {
        Mutex::Autolock lock(turnKeepCq1Lock);
        
        LOG_DBG("cq(%d),en1(0x%x)",cq,tdriMgrInfoCq1.topCtlEn1Keep);
        
        return tdriMgrInfoCq1.topCtlEn1Keep;
    } 
    else if(cq == ISP_DRV_CQ02)
    {
        Mutex::Autolock lock(turnKeepCq2Lock);
        
        LOG_DBG("cq(%d),en1(0x%x)",cq,tdriMgrInfoCq2.topCtlEn1Keep);
        
        return tdriMgrInfoCq2.topCtlEn1Keep;
    }
#endif 
    return 0;
}

/*******************************************************************************
* 
********************************************************************************/
MUINT32 IspDrv::getTurnTopEn2(MINT32 cq)
{
    LOG_DBG("+,cq(%d)", cq);
#if 1 //ic.hsu
    if(cq == ISP_DRV_CQ01)
    {
        Mutex::Autolock lock(turnKeepCq1Lock);
        
        LOG_DBG("cq(%d),en2(0x%x)",cq,tdriMgrInfoCq1.topCtlEn2Keep);
        
        return tdriMgrInfoCq1.topCtlEn2Keep;
    } 
    else if(cq == ISP_DRV_CQ02) 
    {
        Mutex::Autolock lock(turnKeepCq2Lock);
        
        return tdriMgrInfoCq2.topCtlEn2Keep;
    }
#endif
    return 0;
}

/*******************************************************************************
* 
********************************************************************************/
MUINT32 IspDrv::getTurnTopDma(MINT32 cq)
{
    LOG_DBG("+,cq(%d).", cq);
#if 1 //ic.hsu
    if(cq == ISP_DRV_CQ01) 
    {
        Mutex::Autolock lock(turnKeepCq1Lock);
        
        LOG_DBG("cq(%d),dma(0x%x)",cq,tdriMgrInfoCq1.topCtlDmaKeep);
        
        return tdriMgrInfoCq1.topCtlDmaKeep;
    } 
    else if(cq == ISP_DRV_CQ02) 
    {
        Mutex::Autolock lock(turnKeepCq2Lock);
        
        return tdriMgrInfoCq2.topCtlDmaKeep;
    }
#endif
    return 0;
}

/*******************************************************************************
* 
********************************************************************************/
MBOOL IspDrv::updateTurningCq1()
{
    LOG_INF("updateCqdes(%d) CtlEn1(0x%x) CtlEn2(0x%x) CtlMma(0x%x)\n",tdriMgrInfoCq1.updateCqDescriptorNum,
            tdriMgrInfoCq1.topCtlEn1, tdriMgrInfoCq1.topCtlEn2, tdriMgrInfoCq1.topCtlDma);
#if 1//ic.hsu   
    pthread_mutex_lock(&tdriMgrMutexCq1);
    
    if(tdriMgrInfoCq1.updateCqDescriptorNum) 
    {
        MUINT32 i, j, copySize, cqDescOffset, cqDescRegNum, turnReg;
        MUINT32 *pSrc, *pDst;
        MUINT32 en1=0, en2=0, dma=0;
        MUINT32 cnt;
        
        //update CQ descriptor
        for(i = 0; i < CAM_MODULE_MAX; i++) 
        {
            if(tdriMgrInfoCq1.mTurUpdTable[i] == MTRUE) 
            {
                tdriMgrInfoCq1.mTurUpdTable[i] = MFALSE;

                cqDescOffset = gIspCQModuleInfo[i].addr_ofst;
                cqDescRegNum = gIspCQModuleInfo[i].reg_num;

                pSrc = (MUINT32 *)&mpIspVirRegAddrVA[ISP_DRV_CQ01_SYNC][cqDescOffset >> 2];  // >>2 for MUINT32* pointer
                pDst = (MUINT32 *)&mpIspVirRegAddrVA[ISP_DRV_CQ01][cqDescOffset >> 2];       // >>2 for MUINT32* pointer
                memcpy(pDst, pSrc, (cqDescRegNum<<2));

                LOG_DBG("i(%d) cqDescOffset(0x%x) cqDescRegNum(%d) pSrc(0x%x) pDst(0x%x)"
                                ,i,cqDescOffset,cqDescRegNum,(MUINT32)pSrc,(MUINT32)pDst);    //Vent@20121107: Fix build warning: format '%x' expects argument of type 'unsigned int', but argument 8/9 has type 'MUINT32* {aka unsigned int*}' [-Wformat].

                if(cqDescRegNum > ISP_DRV_TURNING_MAX_DBG_NUM)
                {
                    cnt = ISP_DRV_TURNING_MAX_DBG_NUM;
                }
                else
                {
                    cnt = cqDescRegNum;
                }

                for(j = 0; j < cnt; j++)
                {
                    LOG_VRB("(%d)-src(0x%08X)-dst(0x%08X)\n",j,*(pSrc+j),*(pDst+j));
                }
            }
        }

        tdriMgrInfoCq1.updateCqDescriptorNum = 0;
        
        // update turning Top Ctl En1
        LOG_DBG("topCtlEn1(0x%x)",tdriMgrInfoCq1.topCtlEn1);
        
        if(tdriMgrInfoCq1.topCtlEn1)
        {
            pSrc = (MUINT32 *)&mpIspVirRegAddrVA[ISP_DRV_CQ01_SYNC][(0x4080 >> 2)];  // set en1            
            LOG_DBG("[Set_EN1]pSrc(0x%x)",*pSrc);
            
            en1 = getTurnTopEn1(ISP_DRV_CQ01);
            en1 |= (*pSrc);  // set en1
            
            pSrc = (MUINT32 *)&mpIspVirRegAddrVA[ISP_DRV_CQ01_SYNC][(0x4084 >> 2)];  // clr en1            
            LOG_DBG("[Clr_EN1]pSrc(0x%x)\n",*pSrc);
            
            en1 &= (~(*pSrc));  // clr en1
            LOG_DBG("en1(0x%x)",en1);
            
            tdriMgrInfoCq1.topCtlEn1 = 0;
            
            // update turning top data
            setTurnTopEn1(ISP_DRV_CQ01, en1);
        }
        
        // update turning Top Ctl En2
        LOG_DBG("topCtlEn2(0x%x)",tdriMgrInfoCq1.topCtlEn2);
        
        if(tdriMgrInfoCq1.topCtlEn2) 
        {
            pSrc = (MUINT32 *)&mpIspVirRegAddrVA[ISP_DRV_CQ01_SYNC][(0x4088 >> 2)];  // set en2
            LOG_DBG("[Set_EN2]pSrc(0x%x)\n",*pSrc);
            
            en2 = getTurnTopEn2(ISP_DRV_CQ01);
            en2 |= (*pSrc);  // set en2
            
            pSrc = (MUINT32 *)&mpIspVirRegAddrVA[ISP_DRV_CQ01_SYNC][(0x408C >> 2)];  // clr en2
            en2 &= (~(*pSrc));  // clr en2
            LOG_DBG("en2(0x%x)\n",en2);
            
            tdriMgrInfoCq1.topCtlEn2 = 0;
            
            // update turning top data
            setTurnTopEn2(ISP_DRV_CQ01,en2);
        }
        
        // update turning Top Ctl Dma
        LOG_DBG("topCtlDma(0x%x)\n",tdriMgrInfoCq1.topCtlDma);
        
        if(tdriMgrInfoCq1.topCtlDma) 
        {
            pSrc = (MUINT32 *)&mpIspVirRegAddrVA[ISP_DRV_CQ01_SYNC][(0x4090 >> 2)];  // set dma
            LOG_DBG("[Set_DMA]pSrc(0x%x)\n",*pSrc);

            dma = getTurnTopDma(ISP_DRV_CQ01);
            dma |= (*pSrc);  // set dma
            
            pSrc = (MUINT32 *)&mpIspVirRegAddrVA[ISP_DRV_CQ01_SYNC][(0x4094 >> 2)];  // clr dma
            dma &= (~(*pSrc));  // clr dma
            LOG_DBG("dma(0x%x),clr(0x%x)\n",dma,*pSrc);
            
            tdriMgrInfoCq1.topCtlDma = 0;
            
            // update turning top data
            setTurnTopDma(ISP_DRV_CQ01,dma);
        }
        
        // update data for tpipe table
        if(mpIspDrvCB)
        {
            mpIspDrvCB(ISP_DRV_CQ01, mCallbackCookie);
        } 
        else 
        {
            LOG_ERR("[ERROR]CQ1 callback NULL point\n");
        }
    }
    
    pthread_mutex_unlock(&tdriMgrMutexCq1);
#endif
    LOG_DBG("-");
    return MTRUE;
}

/*******************************************************************************
* 
********************************************************************************/
MBOOL IspDrv::updateTurningCq2()
{
    LOG_INF("updateCqdes(%d) CtlEn1(0x%x) CtlEn2(0x%x) CtlMma(0x%x)",tdriMgrInfoCq2.updateCqDescriptorNum,
            tdriMgrInfoCq2.topCtlEn1, tdriMgrInfoCq2.topCtlEn2, tdriMgrInfoCq2.topCtlDma);
#if 1 //ic.hsu     
    pthread_mutex_lock(&tdriMgrMutexCq2);
    
    if(tdriMgrInfoCq2.updateCqDescriptorNum)
    {
        MUINT32 i, j, copySize, cqDescOffset, cqDescRegNum, turnReg;
        MUINT32 *pSrc, *pDst;
        MUINT32 en1=0, en2=0, dma=0;
        MUINT32 cnt;
        
        //update CQ descriptor
        for(i = 0; i < CAM_MODULE_MAX; i++) 
        {        
            if(tdriMgrInfoCq2.mTurUpdTable[i] == MTRUE)
            {
                tdriMgrInfoCq2.mTurUpdTable[i] = MFALSE;

                cqDescOffset = gIspCQModuleInfo[i].addr_ofst;
                cqDescRegNum = gIspCQModuleInfo[i].reg_num;

                pSrc = (MUINT32 *)&mpIspVirRegAddrVA[ISP_DRV_CQ02_SYNC][cqDescOffset >> 2];  // >>2 for MUINT32* pointer
                pDst = (MUINT32 *)&mpIspVirRegAddrVA[ISP_DRV_CQ02][cqDescOffset >> 2];       // >>2 for MUINT32* pointer
                memcpy(pDst, pSrc, (cqDescRegNum<<2));

                LOG_DBG("i(%d) cqDescOffset(0x%x) cqDescRegNum(%d) pSrc(0x%x) pDst(0x%x)"
                                ,i,cqDescOffset,cqDescRegNum,(MUINT32)pSrc,(MUINT32)pDst);    //Vent@20121107: Fix build warning: format '%x' expects argument of type 'unsigned int', but argument 8/9 has type 'MUINT32* {aka unsigned int*}' [-Wformat].

                if(cqDescRegNum > ISP_DRV_TURNING_MAX_DBG_NUM)
                {
                    cnt = ISP_DRV_TURNING_MAX_DBG_NUM;
                }
                else
                {
                    cnt = cqDescRegNum;
                }

                for(j = 0; j < cnt; j++)
                {
                    LOG_VRB("(%d)-src(0x%08X)-dst(0x%08X)\n",j,*(pSrc+j),*(pDst+j));
                }
            }
        }

        tdriMgrInfoCq2.updateCqDescriptorNum = 0;
        
        // update turning Top Ctl En1
        LOG_DBG("topCtlEn1(0x%x)",tdriMgrInfoCq2.topCtlEn1);
        
        if(tdriMgrInfoCq2.topCtlEn1) 
        {
            pSrc = (MUINT32 *)&mpIspVirRegAddrVA[ISP_DRV_CQ02_SYNC][(0x4080 >> 2)];  // set en1
            LOG_DBG("[Set_EN1]pSrc(0x%x)\n",*pSrc);

            en1 = getTurnTopEn1(ISP_DRV_CQ02);
            en1 |= (*pSrc);  // set en1
            
            pSrc = (MUINT32 *)&mpIspVirRegAddrVA[ISP_DRV_CQ02_SYNC][(0x4084 >> 2)];  // clr en1
            LOG_DBG("[Clr_EN1]pSrc(0x%x)",*pSrc);

            en1 &= (~(*pSrc));  // clr en1
            LOG_DBG("en1(0x%x)",en1);
            
            tdriMgrInfoCq2.topCtlEn1 = 0;
            
            // update turning top data
            setTurnTopEn1(ISP_DRV_CQ02, en1);
        }
        
        // update turning Top Ctl En2
        LOG_DBG("topCtlEn2(0x%x)",tdriMgrInfoCq2.topCtlEn2);
        
        if(tdriMgrInfoCq2.topCtlEn2) 
        {
            pSrc = (MUINT32 *)&mpIspVirRegAddrVA[ISP_DRV_CQ02_SYNC][(0x4088 >> 2)];  // set en2
            LOG_DBG("[Set_EN2]pSrc(0x%x)\n",*pSrc);

            en2 = getTurnTopEn2(ISP_DRV_CQ02);
            en2 |= (*pSrc);  // set en2
            
            pSrc = (MUINT32 *)&mpIspVirRegAddrVA[ISP_DRV_CQ02_SYNC][(0x408C >> 2)];  // clr en2
            en2 &= (~(*pSrc));  // clr en2
            LOG_DBG("en2(0x%x)",en2);
            
            tdriMgrInfoCq2.topCtlEn2 = 0;
            
            // update turning top data
            setTurnTopEn2(ISP_DRV_CQ02,en2);
        }
        
        // update turning Top Ctl Dma
        LOG_DBG("topCtlDma(0x%x)\n",tdriMgrInfoCq2.topCtlDma);

        if(tdriMgrInfoCq2.topCtlDma) 
        {
            pSrc = (MUINT32 *)&mpIspVirRegAddrVA[ISP_DRV_CQ02_SYNC][(0x4090 >> 2)];  // set dma
            LOG_DBG("[Set_DMA]pSrc(0x%x)\n",*pSrc);

            dma = getTurnTopDma(ISP_DRV_CQ02);
            dma |= (*pSrc);  // set dma
            
            pSrc = (MUINT32 *)&mpIspVirRegAddrVA[ISP_DRV_CQ02_SYNC][(0x4094 >> 2)];  // clr dma
            dma &= (~(*pSrc));  // clr dma
            LOG_DBG("dma(0x%x)\n",dma);
            
            tdriMgrInfoCq2.topCtlDma = 0;
            
            // update turning top data
            setTurnTopDma(ISP_DRV_CQ02,dma);
        }
        
        // update data for tpipe table
        if(mpIspDrvCB) 
        {
            mpIspDrvCB(ISP_DRV_CQ02, mCallbackCookie);
        } 
        else 
        {
            LOG_ERR("[ERROR]CQ2 callback NULL point\n");
        }
    }
    pthread_mutex_unlock(&tdriMgrMutexCq2);
#endif
    LOG_DBG("-");
    return MTRUE;
}

/*******************************************************************************
* 
********************************************************************************/
MUINT32 IspDrv::getTpipeMgrVaCq1(MUINT32 *pDescArray, MUINT32 *pDescNum, MUINT32 *pCtlEn1, MUINT32 *pCtlEn2, MUINT32 *pCtlDma)
{
    LOG_DBG("+");

    *pDescArray = (MUINT32)tdriMgrInfoCq1.mTurUpdTable;
    *pDescNum = (MUINT32)&tdriMgrInfoCq1.updateCqDescriptorNum;
    *pCtlEn1 = (MUINT32)&tdriMgrInfoCq1.topCtlEn1;
    *pCtlEn2 = (MUINT32)&tdriMgrInfoCq1.topCtlEn2;
    *pCtlDma = (MUINT32)&tdriMgrInfoCq1.topCtlDma;

    return MTRUE;
}

/*******************************************************************************
* 
********************************************************************************/
MUINT32 IspDrv::getTpipeMgrVaCq2(MUINT32 *pDescArray, MUINT32 *pDescNum, MUINT32 *pCtlEn1, MUINT32 *pCtlEn2,  MUINT32 *pCtlDma)
{
    LOG_DBG("+");

    *pDescArray = (MUINT32)tdriMgrInfoCq2.mTurUpdTable;
    *pDescNum = (MUINT32)&tdriMgrInfoCq2.updateCqDescriptorNum;
    *pCtlEn1 = (MUINT32)&tdriMgrInfoCq2.topCtlEn1;
    *pCtlEn2 = (MUINT32)&tdriMgrInfoCq2.topCtlEn2;
    *pCtlDma = (MUINT32)&tdriMgrInfoCq2.topCtlDma;

    return MTRUE;
}


/*******************************************************************************
* 
********************************************************************************/
MVOID IspDrv::lockSemaphoreCq1()
{
    LOG_DBG("lock");

    pthread_mutex_lock(&tdriMgrMutexCq1);
}

/*******************************************************************************
* 
********************************************************************************/
MVOID IspDrv::lockSemaphoreCq2()
{

    pthread_mutex_lock(&tdriMgrMutexCq2);
}

/*******************************************************************************
* 
********************************************************************************/
MVOID IspDrv::unlockSemaphoreCq1()
{
    LOG_DBG("unlock");

    pthread_mutex_unlock(&tdriMgrMutexCq1);
}

/*******************************************************************************
* 
********************************************************************************/
MVOID IspDrv::unlockSemaphoreCq2()
{
    pthread_mutex_unlock(&tdriMgrMutexCq2);
}

/*******************************************************************************
* 
********************************************************************************/
IspDrvImp::IspDrvImp()
{
    MINT32 i;
    GLOBAL_PROFILING_LOG_START(Event_IspDrv);   // Profiling Start.

    LOG_INF("getpid[0x%08x],gettid[0x%08x]", getpid() ,gettid());
    mInitCount = 0;

    mFd = -1;
    m_pIMemDrv = NULL;
    m_pRTBufTbl = NULL;
    mpIspDrvRegMap = NULL;
    m_fgIsGdmaMode = MFALSE;
}

/*******************************************************************************
* 
********************************************************************************/
IspDrvImp::~IspDrvImp()
{
    LOG_INF("+");
    GLOBAL_PROFILING_LOG_END();; 	// Profiling End.
}

/*******************************************************************************
* 
********************************************************************************/
static IspDrvImp singleton;

IspDrv* IspDrvImp::getInstance(MBOOL fgIsGdmaMode)
{
    //LOG_DBG("singleton[0x%08x]", (MUINT32)&singleton);
    //LOG_INF("singleton[0x%08x]. fgIsGdmaMode: %d.", (MUINT32)&singleton, fgIsGdmaMode);
    
    LOG_INF("Gdma: %d.",fgIsGdmaMode);
    
    singleton.m_fgIsGdmaMode = fgIsGdmaMode;

    return &singleton;
}

/*******************************************************************************
* 
********************************************************************************/
MVOID IspDrvImp::destroyInstance()
{
}

/*******************************************************************************
* 
********************************************************************************/

//#define __PMEM_ONLY__
MBOOL IspDrvImp::init()
{
    MBOOL Result = MTRUE;

    GLOBAL_PROFILING_LOG_PRINT(__func__);
    LOCAL_PROFILING_LOG_AUTO_START(Event_IspDrv_Init);
    
    Mutex::Autolock lock(mLock);
    
    LOG_INF("+,mInitCount(%d),m_fgIsGdmaMode(%d)", mInitCount, (MINT32)m_fgIsGdmaMode);
    
    if(mInitCount > 0)
    {
        android_atomic_inc(&mInitCount);
        LOCAL_PROFILING_LOG_PRINT("atomic_inc");
        goto EXIT;
    }
    
#if ISP_DRV_MMAP_TO_IOCLT_ENABLE
    mpIspDrvRegMap = (MUINT32*)malloc(sizeof(isp_reg_t));
#endif

    LOCAL_PROFILING_LOG_PRINT("mpIspDrvRegMap malloc");

    // Open isp driver
    mFd = open(ISP_DRV_DEV_NAME, O_RDWR);
    LOCAL_PROFILING_LOG_PRINT("1st open(ISP_DRV_DEV_NAME, O_RDWR)");
    
    if(mFd < 0)	// 1st time open failed.
    {
        LOG_WRN("ISP kernel open 1st attempt fail, errno(%d):%s", errno, strerror(errno));

        // Try again, using "Read Only".
        mFd = open(ISP_DRV_DEV_NAME, O_RDONLY);
        LOCAL_PROFILING_LOG_PRINT("2nd open(ISP_DRV_DEV_NAME, O_RDONLY)");
        
        if(mFd < 0) // 2nd time open failed.
        {
            LOG_ERR("ISP kernel open 2nd attempt fail, errno(%d):%s", errno, strerror(errno));
            Result = MFALSE;
            goto EXIT;
        }

        #if (!ISP_DRV_MMAP_TO_IOCLT_ENABLE)
        mpIspDrvRegMap = (MUINT32*)malloc(sizeof(isp_reg_t));
        #endif

        LOCAL_PROFILING_LOG_PRINT("mpIspDrvRegMap malloc()");
    }
    else	// 1st time open success.   // Sometimes GDMA will go this path, too. e.g. File Manager -> Phone Storage -> Photo.
    {
        if(!m_fgIsGdmaMode)    // Normal Mode
        {
            // mmap isp reg
            mpIspHwRegAddr = (MUINT32 *) mmap(0, ISP_BASE_RANGE, (PROT_READ | PROT_WRITE | PROT_NOCACHE), MAP_SHARED, mFd, ISP_BASE_HW);
            if(mpIspHwRegAddr == MAP_FAILED)
            {
                LOG_ERR("ISP mmap fail, errno(%d):%s", errno, strerror(errno));
                Result = MFALSE;
                goto EXIT;
            }
            
            LOCAL_PROFILING_LOG_PRINT("mpIspHwRegAddr mmap()");
            
            //pass1 buffer control shared mem.
            m_RTBufTblSize = RT_BUF_TBL_NPAGES * getpagesize();
            m_pRTBufTbl = (MUINT32 *)mmap(0, m_RTBufTblSize, PROT_READ | PROT_WRITE | PROT_NOCACHE, MAP_SHARED| MAP_LOCKED, mFd, m_RTBufTblSize);
            
            LOG_DBG("m_RTBufTblSize(0x%x),m_pRTBufTbl(0x%x)",m_RTBufTblSize,(MUINT32)m_pRTBufTbl);
            
            if(m_pRTBufTbl == MAP_FAILED)
            {
                LOG_ERR("m_pRTBufTbl mmap FAIL");
                Result = MFALSE;
                goto EXIT;
            }

            LOCAL_PROFILING_LOG_PRINT("m_pRTBufTbl mmap()");
        }
    }

    // Increase ISP global reference count, and reset if 1st user.
#if defined(_use_kernel_ref_cnt_)

    LOG_DBG("use kernel ref. cnt.mFd(%d)",mFd);

    ISP_REF_CNT_CTRL_STRUCT ref_cnt;
    MINT32 count;   // For ISP global reference count.

    ref_cnt.ctrl = ISP_REF_CNT_GET;
    ref_cnt.id = ISP_REF_CNT_ID_ISP_FUNC;
    ref_cnt.data_ptr = (MUINT32)&count;
    
    if(MTRUE == kRefCntCtrl(&ref_cnt))
    {
        if (0==count)
        {
            LOG_DBG("DO ISP HW RESET");
            reset(); // Do IMGSYS SW RST.
        }
        
        ref_cnt.ctrl = ISP_REF_CNT_INC;
        if ( MFALSE == kRefCntCtrl(&ref_cnt))
        {
            LOG_ERR("ISP_REF_CNT_INC fail, errno(%d):%s.", errno, strerror(errno));
        }
        LOG_INF("ISP Global Count: %d.", count);
    }
    else
    {
        LOG_ERR("ISP_REF_CNT_GET fail, errno(%d):%s.", errno, strerror(errno));
    }

    LOCAL_PROFILING_LOG_PRINT("kRefCntCtrl and ISP reset()");
    
#else

    LOG_DBG("DO ISP HW RESET");
    reset(); // Do IMGSYS SW RST, which will also enable CAM/SEN/JPGENC/JPGDEC clock.
    
#endif

    if(!m_fgIsGdmaMode)    // Normal Mode
    {
        
        //imem driver
        m_pIMemDrv = IMemDrv::createInstance();
        LOG_DBG("[m_pIMemDrv]:0x%08x", (MUINT32)m_pIMemDrv);    //Vent@20121107: Fix build warning: format '%x' expects argument of type 'unsigned int', but argument 5 has type 'IMemDrv*' [-Wformat]

        if(NULL == m_pIMemDrv)
        {
            LOG_DBG("IMemDrv::createInstance fail");
            return -1;
        }

        LOCAL_PROFILING_LOG_PRINT("IMemDrv::createInstance()");

        m_pIMemDrv->init();

        LOCAL_PROFILING_LOG_PRINT("m_pIMemDrv->init()");
       
        //virtual CQ
        mIspVirRegSize = sizeof(MUINT32) * ISP_BASE_RANGE * ISP_DRV_CQ_NUM;
        
    #if defined(__PMEM_ONLY__)
    
        mpIspVirRegBuffer = (MUINT32 *)pmem_alloc_sync((mIspVirRegSize + (ISP_DRV_VIR_ADDR_ALIGN + 1)), &mIspVirRegFd);
        mpIspVirRegAddrPA[0] = (MUINT32 *)pmem_get_phys(mIspVirRegFd);
        
    #else   // Not PMEM.
        
        m_ispVirRegBufInfo.size = mIspVirRegSize + (ISP_DRV_VIR_ADDR_ALIGN + 1);
        m_ispVirRegBufInfo.useNoncache = 1; //alloc non-cacheable mem.
        
        LOG_INF("m_ispVirRegBufInfo.size(%d)", m_ispVirRegBufInfo.size);
        
        if( m_pIMemDrv->allocVirtBuf(&m_ispVirRegBufInfo))
        {
            LOG_ERR("ERROR:m_pIMemDrv->allocVirtBuf");
        }

        LOCAL_PROFILING_LOG_PRINT("m_pIMemDrv->allocVirtBuf(m_ispVirRegBufInfo)");
        
        mIspVirRegFd = m_ispVirRegBufInfo.memID;
        
        //virtual isp base adrress should be 4 bytes alignment
        mpIspVirRegBuffer = (MUINT32 *)( (m_ispVirRegBufInfo.virtAddr + ISP_DRV_VIR_ADDR_ALIGN) & (~ISP_DRV_VIR_ADDR_ALIGN));
        
        if(m_pIMemDrv->mapPhyAddr(&m_ispVirRegBufInfo))
        {
            LOG_ERR("ERROR:m_pIMemDrv->mapPhyAddr");
        }
        
        //virtual isp base adrress should be 4 bytes alignment
        mpIspVirRegAddrPA[0] = (MUINT32 *)((m_ispVirRegBufInfo.phyAddr + ISP_DRV_VIR_ADDR_ALIGN)  & (~ISP_DRV_VIR_ADDR_ALIGN));

        LOG_VRB("v(0x%x)(0x%x)(0x%x)(0x%x)(0x%x)(0x%x)",mIspVirRegSize,
                                                         m_ispVirRegBufInfo.size,
                                                         m_ispVirRegBufInfo.virtAddr,
                                                         m_ispVirRegBufInfo.phyAddr,
                                                         mpIspVirRegBuffer,
                                                         mpIspVirRegAddrPA[0]);

        LOCAL_PROFILING_LOG_PRINT("m_pIMemDrv->mapPhyAddr()");
        
    #endif  // __PMEM_ONLY__
        
        if(mpIspVirRegBuffer == NULL)
        {
            LOG_ERR("mem alloc fail, size(%d)",mIspVirRegSize);
            Result = MFALSE;
            goto EXIT;
        }
        
        memset((MUINT8*)mpIspVirRegBuffer,ISP_DRV_VIR_DEFAULT_DATA,mIspVirRegSize);

        LOCAL_PROFILING_LOG_PRINT("memset(mpIspVirRegBuffer)");

        for(MINT32 i = 0; i < ISP_DRV_CQ_NUM; i++) 
        {
            mpIspVirRegAddrVA[i] = mpIspVirRegBuffer + i * ISP_BASE_RANGE;
            mpIspVirRegAddrPA[i] = mpIspVirRegAddrPA[0] + i * ISP_BASE_RANGE;
            LOG_DBG("cq:[%d],virtIspAddr:virt[0x%08x]/phy[0x%08x]",i,(MUINT32)mpIspVirRegAddrVA[i], (MUINT32)mpIspVirRegAddrPA[i] );
        }

        //CQ descriptor
        mIspCQDescSize = sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT) * ISP_DRV_DESCRIPTOR_CQ_NUM * CAM_MODULE_MAX;

    #if defined(__PMEM_ONLY__)
        
        mpIspCQDescBufferVirt = (MUINT32 *)pmem_alloc_sync( (mIspCQDescSize+ (ISP_DRV_CQ_DESCRIPTOR_ADDR_ALIGN+1)), &mIspCQDescFd);
        mpIspCQDescBufferPhy = (MUINT32)pmem_get_phys(mIspCQDescFd);

    #else   // Not PMEM.
        
        m_ispCQDescBufInfo.size = mIspCQDescSize+ (ISP_DRV_CQ_DESCRIPTOR_ADDR_ALIGN + 1);
        m_ispCQDescBufInfo.useNoncache = 1; //alloc non-cacheable mem.    
        
        if( m_pIMemDrv->allocVirtBuf(&m_ispCQDescBufInfo))
        {
            LOG_ERR("ERROR:m_pIMemDrv->allocVirtBuf");
        }

        LOCAL_PROFILING_LOG_PRINT("m_pIMemDrv->allocVirtBuf(m_ispCQDescBufInfo)");
        
        mIspCQDescFd = m_ispCQDescBufInfo.memID;
        
        //CQ decriptor base address should be 8 bytes alignment
        mpIspCQDescBufferVirt = (MUINT32 *)((m_ispCQDescBufInfo.virtAddr+ISP_DRV_CQ_DESCRIPTOR_ADDR_ALIGN) & (~ISP_DRV_CQ_DESCRIPTOR_ADDR_ALIGN));
        
        if(m_pIMemDrv->mapPhyAddr(&m_ispCQDescBufInfo))
        {
            LOG_ERR("ERROR:m_pIMemDrv->mapPhyAddr");
        }
        
        //CQ decriptor base address should be 8 bytes alignment
        mpIspCQDescBufferPhy = ((m_ispCQDescBufInfo.phyAddr+ISP_DRV_CQ_DESCRIPTOR_ADDR_ALIGN) & (~ISP_DRV_CQ_DESCRIPTOR_ADDR_ALIGN));

        LOG_VRB("cq(0x%x)(0x%x)(0x%x)(0x%x)(0x%x)(0x%x)",mIspCQDescSize,
                                                          m_ispCQDescBufInfo.size,
                                                          m_ispCQDescBufInfo.virtAddr,
                                                          m_ispCQDescBufInfo.phyAddr,
                                                          mpIspCQDescBufferVirt,
                                                          mpIspCQDescBufferPhy);

        LOCAL_PROFILING_LOG_PRINT("m_pIMemDrv->mapPhyAddr(m_ispCQDescBufInfo)");

    #endif  // __PMEM_ONLY__

        for (MINT32 i = 0; i < CAM_MODULE_MAX; i++) 
        {
            gIspCQDescInit[i].v_reg_addr = (MUINT32)( mpIspVirRegAddrPA[0] + (0x4060/4) );
        }
        
        for(MINT32 i = 0; i < ISP_DRV_DESCRIPTOR_CQ_NUM; i++)
        {
            if(0 == i)
            {
                mpIspCQDescriptorVirt[i] = (ISP_DRV_CQ_CMD_DESC_STRUCT*)mpIspCQDescBufferVirt;
                mpIspCQDescriptorPhy[i]  = mpIspCQDescBufferPhy;
            }
            else 
            {
                mpIspCQDescriptorVirt[i] = (ISP_DRV_CQ_CMD_DESC_STRUCT *)((MUINT32)mpIspCQDescriptorVirt[i-1] + sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT)*CAM_MODULE_MAX);
                mpIspCQDescriptorPhy[i]  = mpIspCQDescriptorPhy[i-1] + sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT)*CAM_MODULE_MAX;
            }
            
            memcpy((MUINT8*)mpIspCQDescriptorVirt[i], gIspCQDescInit, sizeof(gIspCQDescInit));
            
            LOG_DBG("cq:[%d],mpIspCQDescriptor:Virt[0x%08x]/Phy[0x%08x]",i,(MUINT32)mpIspCQDescriptorVirt[i],(MUINT32)mpIspCQDescriptorPhy[i]);
        }
        
        LOCAL_PROFILING_LOG_PRINT("memcpy(mpIspCQDescriptorVirt[i])");
        
    #if defined(_rtbc_use_cq0c_)
    
        //reset cq0c all 0 for rtbc
        if(sizeof(CQ_RTBC_RING_ST) <= sizeof(gIspCQDescInit)) 
        {
            memset((MUINT8*)mpIspCQDescriptorVirt[ISP_DRV_DESCRIPTOR_CQ0C], 0, sizeof(CQ_RTBC_RING_ST));
        }
        else 
        {
            LOG_ERR("rtbc data too large(%d)>(%d)",sizeof(CQ_RTBC_RING_ST),sizeof(gIspCQDescInit));
        }

        LOCAL_PROFILING_LOG_PRINT("memset(mpIspCQDescriptorVirt[CQ0C])");
        
    #endif  // _rtbc_use_cq0c_

        // init for turning path
        memset((MUINT8*)&tdriMgrInfoCq1,0x00,sizeof(tdriMgrInfoCq1));

        LOCAL_PROFILING_LOG_PRINT("memset(tdriMgrInfoCq1)");
        
        memset((MUINT8*)&tdriMgrInfoCq2,0x00,sizeof(tdriMgrInfoCq2));

        LOCAL_PROFILING_LOG_PRINT("memset(tdriMgrInfoCq2)");
    }   // End of if(!m_fgIsGdmaMode).

    android_atomic_inc(&mInitCount);

    LOCAL_PROFILING_LOG_PRINT("atomic_inc");
    
EXIT:

    if(!Result)    // If some init step goes wrong.
    {
        if(mFd >= 0)
        {
            close(mFd);
            mFd = -1;
            LOCAL_PROFILING_LOG_PRINT("close isp mFd");
        }
    }

    LOG_INF("-,ret(%d),mInitCount(%d)", Result, mInitCount);
    LOCAL_PROFILING_LOG_PRINT("Exit");

    return Result;
}

/*******************************************************************************
* 
********************************************************************************/
MBOOL IspDrvImp::uninit()
{
    MBOOL Result = MTRUE;

    GLOBAL_PROFILING_LOG_PRINT(__func__);
    LOCAL_PROFILING_LOG_AUTO_START(Event_IspDrv_Uninit);
    
    Mutex::Autolock lock(mLock);
    
    LOG_INF("+,mInitCount(%d)", mInitCount);
    
    if(mInitCount <= 0)
    {
        // No more users
        goto EXIT;
    }
    
    // More than one user
    android_atomic_dec(&mInitCount);

    LOCAL_PROFILING_LOG_PRINT("atomic_dec");

    if(mInitCount > 0)    // If there are still users, exit.
    {
        goto EXIT;
    }

    if(!m_fgIsGdmaMode)    // Normal Mode (i.e. not GDMA).
    {

        munmap(mpIspHwRegAddr, ISP_BASE_RANGE);
        mpIspHwRegAddr = NULL;
        
        LOCAL_PROFILING_LOG_PRINT("munmap(mpIspHwRegAddr)");
        
        munmap(m_pRTBufTbl, m_RTBufTblSize);
        m_pRTBufTbl = NULL;

        LOCAL_PROFILING_LOG_PRINT("munmap(m_pRTBufTbl)");
        
        //virtual CQ
        if( 0 != mpIspVirRegBuffer)
        {
        #if defined(__PMEM_ONLY__)
        
            if(mIspVirRegFd >= 0) 
            {
                pmem_free((MUINT8 *)mpIspVirRegBuffer,mIspVirRegSize,mIspVirRegFd);
                mIspVirRegFd = -1;
            }
        #else
        
            if(m_pIMemDrv->unmapPhyAddr(&m_ispVirRegBufInfo))
            {
                LOG_ERR("ERROR:m_pIMemDrv->unmapPhyAddr");
            }

            LOCAL_PROFILING_LOG_PRINT("m_pIMemDrv->unmapPhyAddr(m_ispVirRegBufInfo)");
            
            if(m_pIMemDrv->freeVirtBuf(&m_ispVirRegBufInfo))
            {
                LOG_ERR("ERROR:m_pIMemDrv->freeVirtBuf");
            }

            LOCAL_PROFILING_LOG_PRINT("m_pIMemDrv->freeVirtBuf(m_ispVirRegBufInfo)");
            
            LOG_DBG("free/unmap mpIspVirRegBuffer");
            
        #endif  //__PMEM_ONLY__
        
            mpIspVirRegBuffer = NULL;
        }
        
        if(0 != mpIspCQDescBufferVirt)
        {
        #if defined(__PMEM_ONLY__)
        
            if(mIspCQDescFd >= 0)
            {
                pmem_free((MUINT8 *)mpIspCQDescBufferVirt,mIspCQDescSize,mIspCQDescFd);
                mIspCQDescFd = -1;
            }
            
        #else
        
            if(m_pIMemDrv->unmapPhyAddr(&m_ispCQDescBufInfo))
            {
                LOG_ERR("ERROR:m_pIMemDrv->unmapPhyAddr");
            }

            LOCAL_PROFILING_LOG_PRINT("m_pIMemDrv->unmapPhyAddr(m_ispCQDescBufInfo)");
            
            if(m_pIMemDrv->freeVirtBuf(&m_ispCQDescBufInfo))
            {
                LOG_ERR("ERROR:m_pIMemDrv->freeVirtBuf");
            }

            LOCAL_PROFILING_LOG_PRINT("m_pIMemDrv->freeVirtBuf(m_ispCQDescBufInfo)");
            LOG_DBG("free/unmap mpIspCQDescBufferVirt");
            
        #endif
        
            mpIspCQDescBufferVirt = NULL;
        }
     
        //IMEM
        m_pIMemDrv->uninit();

        LOCAL_PROFILING_LOG_PRINT("m_pIMemDrv->uninit()");
        
        m_pIMemDrv->destroyInstance();

        LOCAL_PROFILING_LOG_PRINT("m_pIMemDrv->destroyInstance()");
        
        m_pIMemDrv = NULL;
        
        // turning path
        memset((MUINT8*)&tdriMgrInfoCq1,0x00,sizeof(tdriMgrInfoCq1));

        LOCAL_PROFILING_LOG_PRINT("memset(tdriMgrInfoCq1)");

        memset((MUINT8*)&tdriMgrInfoCq2,0x00,sizeof(tdriMgrInfoCq2));

        LOCAL_PROFILING_LOG_PRINT("memset(tdriMgrInfoCq2)");
    }// End of if (!m_fgIsGdmaMode).

    mpIspHwRegAddr = NULL;
    mIspVirRegFd = 0;
    mpIspVirRegBuffer = NULL;
    mIspVirRegSize = 0;
    mIspCQDescFd = -1;
    mpIspCQDescBufferVirt = NULL;
    mIspCQDescSize = 0;
    mpIspCQDescBufferPhy = 0;
    
    for(MINT32 i = 0; i<ISP_DRV_CQ_NUM; i++)
    {
        mpIspVirRegAddrVA[i] = NULL;
        mpIspVirRegAddrPA[i] = NULL;
    }

    if(mFd >= 0)
    {
    #if defined(_use_kernel_ref_cnt_)
    
        ISP_REF_CNT_CTRL_STRUCT ref_cnt;
        MINT32 count;   // For ISP global reference count.

        ref_cnt.ctrl = ISP_REF_CNT_DEC;
        ref_cnt.id = ISP_REF_CNT_ID_ISP_FUNC;
        ref_cnt.data_ptr = (MUINT32)&count;
        
        if( MFALSE == kRefCntCtrl(&ref_cnt))
        {            
            LOG_ERR("ISP_REF_CNT_GET fail, errno(%d):%s.", errno, strerror(errno));
        }
        
        LOG_INF("ISP Global Count: %d.", count);

    #endif

        close(mFd);
        mFd = -1;
    }
    
    LOCAL_PROFILING_LOG_PRINT("close isp mFd");
    
    if(mpIspDrvRegMap != NULL)
    {
        free((MUINT32*)mpIspDrvRegMap);
        mpIspDrvRegMap = NULL;
    }

    LOCAL_PROFILING_LOG_PRINT("free(mpIspDrvRegMap)");
    
EXIT:

    // Reset m_fgIsGdmaMode to default (MFALSE).
    if(m_fgIsGdmaMode)
    {
        m_fgIsGdmaMode = MFALSE;
    }

    LOG_INF("-,ret(%d),mInitCount(%d)", Result, mInitCount);
    LOCAL_PROFILING_LOG_PRINT("Exit");
    return Result;
}

/*******************************************************************************
* 
********************************************************************************/
MBOOL IspDrvImp::waitIrq(ISP_DRV_WAIT_IRQ_STRUCT waitIrq)
{
    MINT32 Ret;
    
    LOG_DBG("+,Clear(%d),Type(%d),Status(0x%08x),Timeout(%d)", waitIrq.Clear, waitIrq.Type, waitIrq.Status, waitIrq.Timeout);
    
    Ret = ioctl(mFd,ISP_WAIT_IRQ,&waitIrq);
    if(Ret < 0)
    {
        LOG_ERR("ISP_WAIT_IRQ fail(%d). Clear(%d), Type(%d), Status(0x%08x), Timeout(%d).", Ret, waitIrq.Clear, waitIrq.Type, waitIrq.Status, waitIrq.Timeout);
        return MFALSE;
    }
    
#if 0
    {
        MINT32 i;
        LOG_INF("Clear(%d),Type(%d),Status(0x%08x),Timeout(%d).", waitIrq.Clear, waitIrq.Type, waitIrq.Status, waitIrq.Timeout);


        if(waitIrq.Type==0 && waitIrq.Status==0x4000){
            for(i=0;i<=CAM_ISP_PCA_CON;i++){
                if(mpIspCQDescriptorVirt[ISP_DRV_DESCRIPTOR_CQ01][i].u.cmd != 0x4060)
                    LOG_INF("kk:cq(%d)-i(%d)-val(0x%x)",ISP_DRV_DESCRIPTOR_CQ01,i,mpIspCQDescriptorVirt[ISP_DRV_DESCRIPTOR_CQ01][i].u.cmd);
            }

            for(i=0;i<=CAM_ISP_PCA_CON;i++){
                if(mpIspCQDescriptorVirt[ISP_DRV_DESCRIPTOR_CQ0][i].u.cmd != 0x4060)
                    LOG_INF("kk:cq(%d)-i(%d)-val(0x%x)",ISP_DRV_DESCRIPTOR_CQ0,i,mpIspCQDescriptorVirt[ISP_DRV_DESCRIPTOR_CQ0][i].u.cmd);
            }

        }

        if(waitIrq.Type==2 && waitIrq.Status==0x4000){
            for(i=0;i<=CAM_ISP_PCA_CON;i++){
                if(mpIspCQDescriptorVirt[ISP_DRV_DESCRIPTOR_CQ01][i].u.cmd != 0x4060)
                    LOG_INF("kk:cq(%d)-i(%d)-val(0x%x)",ISP_DRV_DESCRIPTOR_CQ02,i,mpIspCQDescriptorVirt[ISP_DRV_DESCRIPTOR_CQ02][i].u.cmd);
            }
        }
    }
#endif

    return MTRUE;
}

/*******************************************************************************
* 
********************************************************************************/
MBOOL IspDrvImp::readIrq(ISP_DRV_READ_IRQ_STRUCT *pReadIrq)
{
    MINT32 Ret;
    ISP_DRV_READ_IRQ_STRUCT ReadIrq;
    
    LOG_INF("+,Type(%d),Status(%d)",pReadIrq->Type, pReadIrq->Status);
    
    ReadIrq.Type = pReadIrq->Type;
    ReadIrq.Status = 0;
    
    Ret = ioctl(mFd,ISP_READ_IRQ,&ReadIrq);
    if(Ret < 0)
    {
        LOG_ERR("ISP_READ_IRQ fail(%d)",Ret);
        return MFALSE;
    }
    
    pReadIrq->Status = ReadIrq.Status;
    return MTRUE;
}

/*******************************************************************************
* 
********************************************************************************/
MBOOL IspDrvImp::checkIrq(ISP_DRV_CHECK_IRQ_STRUCT CheckIrq)
{
    MINT32 Ret;
    ISP_DRV_READ_IRQ_STRUCT ReadIrq;
    
    LOG_DBG("+,Type(%d),Status(%d)",CheckIrq.Type, CheckIrq.Status);
    
    ReadIrq.Type = CheckIrq.Type;
    ReadIrq.Status = 0;
    if(!readIrq(&ReadIrq))
    {
        return MFALSE;
    }
    
    if((CheckIrq.Status & ReadIrq.Status) != CheckIrq.Status)
    {
        LOG_ERR("Status:Check(0x%08X),Read(0x%08X)",CheckIrq.Status,ReadIrq.Status);
        return MFALSE;
    }
    
    return MTRUE;
}

/*******************************************************************************
* 
********************************************************************************/
MBOOL IspDrvImp::clearIrq(ISP_DRV_CLEAR_IRQ_STRUCT ClearIrq)
{
    MINT32 Ret;
    
    LOG_INF("+,Type(%d), Status(%d)",ClearIrq.Type, ClearIrq.Status);
    
    Ret = ioctl(mFd,ISP_CLEAR_IRQ,&ClearIrq);
    if(Ret < 0)
    {
        LOG_ERR("ISP_CLEAR_IRQ fail(%d)",Ret);
        return MFALSE;
    }
    
    return MTRUE;
}

/*******************************************************************************
* 
********************************************************************************/
MBOOL IspDrvImp::reset()
{
    MINT32 Ret;
    
    LOG_INF("ISP SW RESET[0x%08x]",mFd);
    
    Ret = ioctl(mFd,ISP_RESET,NULL);
    if(Ret < 0)
    {
        LOG_ERR("ISP_RESET fail(%d)",Ret);
        return MFALSE;
    }
    
    return MTRUE;
}

/*******************************************************************************
* 
********************************************************************************/
MBOOL IspDrvImp::resetBuf()
{
    MINT32 Ret;
    
    LOG_DBG("");
    
    Ret = ioctl(mFd,ISP_RESET_BUF,NULL);
    if(Ret < 0)
    {
        LOG_ERR("ISP_RESET_BUF fail(%d)",Ret);
        return MFALSE;
    }
    
    return MTRUE;
}

/*******************************************************************************
* 
********************************************************************************/
MUINT32 IspDrvImp::getRegAddr()
{
    LOG_DBG("mpIspHwRegAddr(0x%08X)",(MUINT32)mpIspHwRegAddr);
    
    if(mpIspHwRegAddr != NULL)
    {
        return (MUINT32)mpIspHwRegAddr;
    }
    else
    {
        return 0;
    }
}

/*******************************************************************************
* 
********************************************************************************/
isp_reg_t *IspDrvImp::getRegAddrMap()
{
    //LOG_VRB("mpIspDrvRegMap(0x%08X)",(MUINT32)mpIspDrvRegMap);
    
    if(mpIspDrvRegMap != NULL)
    {
        return (isp_reg_t*)mpIspDrvRegMap;
    }
    else
    {
        return 0;
    }
}

/*******************************************************************************
* 
********************************************************************************/
MBOOL IspDrvImp::readRegs(ISP_DRV_REG_IO_STRUCT *pRegIo, MUINT32 Count)
{
    MINT32 Ret;
    ISP_REG_IO_STRUCT IspRegIo;
    
    //LOG_DBG("Count(%d)",Count);
    
    if(pRegIo == NULL)
    {
        LOG_ERR("pRegIo is NULL");
        return MFALSE;
    }
    
    IspRegIo.Data = (MINT32)pRegIo;
    IspRegIo.Count = Count;
    
    Ret = ioctl(mFd, ISP_READ_REGISTER, &IspRegIo);
    if(Ret < 0)
    {
        LOG_ERR("ISP_READ_REG fail(%d)",Ret);
        return MFALSE;
    }
    
    return MTRUE;
}

/*******************************************************************************
* 
********************************************************************************/
MUINT32 IspDrvImp::readReg(MUINT32 Addr)
{
    ISP_DRV_REG_IO_STRUCT RegIo;
    
    //LOG_DBG("Addr(0x%08X)",Addr);
    
    RegIo.Addr = Addr;
    
    if(!readRegs(&RegIo, 1))
    {
        return 0;
    }
    
    return (RegIo.Data);
}

/*******************************************************************************
* 
********************************************************************************/
MBOOL IspDrvImp::writeRegs(ISP_DRV_REG_IO_STRUCT *pRegIo,MUINT32 Count)
{
    MINT32 Ret;
    ISP_REG_IO_STRUCT IspRegIo;
    
    //LOG_MSG("Count(%d)\n",Count);
    
    if(pRegIo == NULL)
    {
        LOG_ERR("pRegIo is NULL");
        return MFALSE;
    }
    
    IspRegIo.Data = (MINT32)pRegIo;
    IspRegIo.Count = Count;
    
    Ret = ioctl(mFd, ISP_WRITE_REGISTER, &IspRegIo);
    if(Ret < 0)
    {
        LOG_ERR("ISP_WRITE_REG fail(%d)",Ret);
        return MFALSE;
    }
    
    return MTRUE;
}

/*******************************************************************************
* 
********************************************************************************/
MBOOL IspDrvImp::writeReg(MUINT32 Addr, MUINT32 Data)
{
    ISP_DRV_REG_IO_STRUCT RegIo;
    
    //LOG_DBG("Addr(0x%08X),Data(0x%08X)",Addr,Data);
    
    RegIo.Addr = Addr;
    RegIo.Data = Data;
    
    if(!writeRegs(&RegIo, 1))
    {
        return MFALSE;
    }
    
    return MTRUE;
}

/*******************************************************************************
* 
********************************************************************************/
MBOOL IspDrvImp::holdReg(MBOOL En)
{
    MINT32 Ret;
    
    LOG_DBG("En(%d)",En);
    
    Ret = ioctl(mFd, ISP_HOLD_REG, &En);
    if(Ret < 0)
    {
        LOG_ERR("ISP_HOLD_REG fail(%d)",Ret);
        return MFALSE;
    }
    
    return MTRUE;
}

/*******************************************************************************
* 
********************************************************************************/
MBOOL IspDrvImp::dumpReg()
{
    MINT32 Ret;
    
    LOG_DBG("+");
    
    Ret = ioctl(mFd, ISP_DUMP_REG, NULL);
    if(Ret < 0)
    {
        LOG_ERR("ISP_DUMP_REG fail(%d)",Ret);
        return MFALSE;
    }
    
    return MTRUE;
}

/*******************************************************************************
* 
********************************************************************************/
MBOOL IspDrvImp::IsReadOnlyMode()
{
#if ISP_DRV_MMAP_TO_IOCLT_ENABLE
    
    return MTRUE;

#else

    if(mpIspDrvRegMap == NULL)
    {
        return MFALSE;
    }
    else
    {
        return MTRUE;
    }
    
#endif
}

/*******************************************************************************
* 
********************************************************************************/
#if defined(_use_kernel_ref_cnt_)

MBOOL IspDrvImp::kRefCntCtrl(ISP_REF_CNT_CTRL_STRUCT *pCtrl)
{
    MINT32 Ret;
    
    LOG_DBG("(%d)(%d)(0x%x)",pCtrl->ctrl,pCtrl->id,pCtrl->data_ptr);
    
    Ret = ioctl(mFd,ISP_REF_CNT_CTRL,pCtrl);
    if(Ret < 0)
    {
        LOG_ERR("ISP_REF_CNT_CTRL fail(%d)[errno(%d):%s] \n",Ret, errno, strerror(errno));
        return MFALSE;
    }
    
    return MTRUE;
}

#endif

/*******************************************************************************
* 
********************************************************************************/
MBOOL IspDrvImp::rtBufCtrl(MVOID *pBuf_ctrl)
{
   MINT32 Ret;
    
#if defined(_rtbc_use_cq0c_)

    if(MFALSE == cqRingBuf(pBuf_ctrl))
    {
        LOG_ERR("cqRingBuf(%d)",Ret);
        return MFALSE;
    }
    
#endif
    
    Ret = ioctl(mFd,ISP_BUFFER_CTRL,pBuf_ctrl);
    if(Ret < 0)
    {
        LOG_ERR("ISP_BUFFER_CTRL(%d) \n",Ret);
        return MFALSE;
    }
    
    return MTRUE;
}

/*******************************************************************************
* 
********************************************************************************/
MUINT32 IspDrvImp::GlobalPipeCountInc()
{
    LOG_INF("+");

    MBOOL Result = MTRUE;
    MINT32 ret = 0;
    ISP_REF_CNT_CTRL_STRUCT ref_cnt;
    MINT32 count;

    // Increase global pipe count.
	ref_cnt.ctrl = ISP_REF_CNT_INC;
    ref_cnt.id = ISP_REF_CNT_ID_GLOBAL_PIPE;
    ref_cnt.data_ptr = (MUINT32)&count;
    
    ret = ioctl(mFd, ISP_REF_CNT_CTRL, &ref_cnt);
    if(ret < 0)
    {
        LOG_ERR("ISP_REF_CNT_INC fail(%d)[errno(%d):%s]",ret, errno, strerror(errno));
        Result = MFALSE;
    }

    LOG_INF("-,Result(%d),count(%d)", Result, count);

    return Result;
}

/*******************************************************************************
* 
********************************************************************************/
MUINT32 IspDrvImp::GlobalPipeCountDec()
{
    LOG_INF("+");

    MBOOL Result = MTRUE;
    MINT32 ret = 0;
    ISP_REF_CNT_CTRL_STRUCT ref_cnt;
    MINT32 count;

    ref_cnt.ctrl = ISP_REF_CNT_DEC_AND_RESET_IF_LAST_ONE;
    ref_cnt.id = ISP_REF_CNT_ID_GLOBAL_PIPE;
    ref_cnt.data_ptr = (MUINT32)&count;
    
    ret = ioctl(mFd, ISP_REF_CNT_CTRL, &ref_cnt);
    if(ret < 0)
    {
        LOG_ERR("ISP_REF_CNT_DEC fail(%d)[errno(%d):%s]",ret, errno, strerror(errno));
        Result = MFALSE;
    }

    LOG_INF("-,Result(%d),count(%d)", Result, count);

    return Result;
}

/*******************************************************************************
* 
********************************************************************************/

#if defined(_rtbc_use_cq0c_)

MBOOL IspDrvImp::cqRingBuf(MVOID *pBuf_ctrl)
{
   MINT32 Ret;
    
    #define RTBC_GET_PA_FROM_VA(va,bva,bpa) ( ( (MUINT32)(va) - (MUINT32)(bva) ) + (MUINT32)(bpa) )

    //LOG_DBG("size of CQ_RTBC_RING_ST is (%d) \n",sizeof(CQ_RTBC_RING_ST));
    CQ_RTBC_RING_ST *pcqrtbcring_va  = (CQ_RTBC_RING_ST *)IspDrv::mpIspCQDescriptorVirt[ISP_DRV_DESCRIPTOR_CQ0C];
    CQ_RTBC_RING_ST *pcqrtbcring_pa  = (CQ_RTBC_RING_ST *)IspDrv::mpIspCQDescriptorPhy[ISP_DRV_DESCRIPTOR_CQ0C];

    MUINT32 i = 0;
    MUINT32 reg_val;
    MUINT32 reg_val2;
    MUINT32 size;
    isp_reg_t *pIspPhyReg = (isp_reg_t *)getRegAddr();

    ISP_BUFFER_CTRL_STRUCT *pbuf_ctrl = (ISP_BUFFER_CTRL_STRUCT *)pBuf_ctrl;
    ISP_RT_BUF_INFO_STRUCT *pbuf_info = (ISP_RT_BUF_INFO_STRUCT *)pbuf_ctrl->data_ptr;
    ISP_RT_BUF_INFO_STRUCT *pex_buf_info = (ISP_RT_BUF_INFO_STRUCT *)pbuf_ctrl->ex_data_ptr;
    
    LOG_DBG("[rtbc]pcqrtbcring_va(0x%08x)",pcqrtbcring_va);
    LOG_DBG("[rtbc]pcqrtbcring_pa(0x%08x)",pcqrtbcring_pa);
    LOG_DBG("[rtbc]ctrl(%d),dma(%d),PA(0x%x),or_size(%d),2or_size(%d)",pbuf_ctrl->ctrl,
                                                                        pbuf_ctrl->buf_id,
                                                                        pbuf_info->base_pAddr,
                                                                        pcqrtbcring_va->imgo_ring_size,
                                                                        pcqrtbcring_va->img2o_ring_size);

    LOG_DBG("[rtbc](%d)",sizeof(CQ_RING_CMD_ST));

    switch(pbuf_ctrl->ctrl)
    {        
        case ISP_RT_BUF_CTRL_ENQUE:
            if(IsReadOnlyMode())
            {
                reg_val = ISP_IOCTL_READ_REG(this, this->getRegAddrMap(), CAM_TG_VF_CON);
                //reg_val2 = ISP_IOCTL_READ_REG(this, this->getRegAddrMap(), CAM_TG2_VF_CON);
            }
            else
            {
                reg_val  = ISP_READ_REG(pIspPhyReg, CAM_TG_VF_CON);
                //reg_val2 = ISP_READ_REG(pIspPhyReg, CAM_TG2_VF_CON);
            }

            LOG_DBG("[rtbc]VF(0x%x),VF2(0x%x)",reg_val,reg_val2);

            //VF is IDLE
            if(0 == ((reg_val & 0x01) | (reg_val2 & 0x01)))
            {                
                LOG_INF("[rtbc]va(0x%x),pa(0x%x),ctrl(%d),dma(%d),PA(0x%x),or_size(%d),2or_size(%d)", \
                        pcqrtbcring_va, \
                        pcqrtbcring_pa, \
                        pbuf_ctrl->ctrl, \
                        pbuf_ctrl->buf_id, \
                        pbuf_info->base_pAddr, \
                        pcqrtbcring_va->imgo_ring_size,\
                        pcqrtbcring_va->img2o_ring_size);
                
                if( _imgo_ == pbuf_ctrl->buf_id ) 
                {
                    i = pcqrtbcring_va->imgo_ring_size;
                }
                else if(_img2o_ == pbuf_ctrl->buf_id) 
                {
                    i = pcqrtbcring_va->img2o_ring_size;
                }
                else
                {
                    LOG_ERR("ERROR:DMA id (%d)",pbuf_ctrl->buf_id);
                    return MFALSE;
                }
                
                pcqrtbcring_va->rtbc_ring[i].pNext = &pcqrtbcring_va->rtbc_ring[(i>0)?0:i];
                pcqrtbcring_va->rtbc_ring[i].next_pa = (MUINT32)RTBC_GET_PA_FROM_VA(pcqrtbcring_va->rtbc_ring[i].pNext,pcqrtbcring_va,pcqrtbcring_pa);
                
                pcqrtbcring_va->rtbc_ring[i].cq_rtbc.imgo.inst = 0x00004300; //ISP_DRV_CQ_DUMMY_WR_TOKEN; //reg_4300
                pcqrtbcring_va->rtbc_ring[i].cq_rtbc.imgo.data_ptr_pa = \
                    (MUINT32)RTBC_GET_PA_FROM_VA(&pcqrtbcring_va->rtbc_ring[i].cq_rtbc.imgo_base_pAddr,pcqrtbcring_va,pcqrtbcring_pa);
                
                if(0 == pcqrtbcring_va->img2o_ring_size)
                {
                    pcqrtbcring_va->rtbc_ring[i].cq_rtbc.img2o.inst = ISP_DRV_CQ_DUMMY_WR_TOKEN; //0x00004320; //reg_4320
                }
                
                pcqrtbcring_va->rtbc_ring[i].cq_rtbc.img2o.data_ptr_pa = \
                    (MUINT32)RTBC_GET_PA_FROM_VA(&pcqrtbcring_va->rtbc_ring[i].cq_rtbc.img2o_base_pAddr,pcqrtbcring_va,pcqrtbcring_pa);

                pcqrtbcring_va->rtbc_ring[i].cq_rtbc.next_cq0ci.inst = 0x000040BC; //reg_40BC
                pcqrtbcring_va->rtbc_ring[i].cq_rtbc.next_cq0ci.data_ptr_pa = \
                    (MUINT32)RTBC_GET_PA_FROM_VA(&pcqrtbcring_va->rtbc_ring[i].next_pa,pcqrtbcring_va,pcqrtbcring_pa);

                pcqrtbcring_va->rtbc_ring[i].cq_rtbc.end.inst = 0xFC000000;
                pcqrtbcring_va->rtbc_ring[i].cq_rtbc.end.data_ptr_pa = 0;
                
                if(_imgo_ == pbuf_ctrl->buf_id)
                {
                    pcqrtbcring_va->rtbc_ring[i].cq_rtbc.imgo_base_pAddr = pbuf_info->base_pAddr; // ISP_CQ_DUMMY_PA; //
                }
                else if(_img2o_ == pbuf_ctrl->buf_id)
                {
                    pcqrtbcring_va->rtbc_ring[i].cq_rtbc.img2o.inst = 0x00004320; //reg_4320
                    pcqrtbcring_va->rtbc_ring[i].cq_rtbc.img2o_base_pAddr = pbuf_info->base_pAddr;
                }
                else 
                 {
                    LOG_ERR("ERROR:DMA id (%d)",pbuf_ctrl->buf_id);
                    return MFALSE;
                }
                
                if(i > 0)
                {
                    pcqrtbcring_va->rtbc_ring[i-1].pNext = &pcqrtbcring_va->rtbc_ring[i];
                    pcqrtbcring_va->rtbc_ring[i-1].next_pa = \
                        (MUINT32)RTBC_GET_PA_FROM_VA(&pcqrtbcring_va->rtbc_ring[i],pcqrtbcring_va,pcqrtbcring_pa);
                    
                    pcqrtbcring_va->rtbc_ring[i-1].cq_rtbc.next_cq0ci.data_ptr_pa = \
                        (MUINT32)RTBC_GET_PA_FROM_VA(&pcqrtbcring_va->rtbc_ring[i-1].next_pa,pcqrtbcring_va,pcqrtbcring_pa);
                }
                
                if(_imgo_ == pbuf_ctrl->buf_id)
                {
                    pcqrtbcring_va->imgo_ring_size++;
                    size = pcqrtbcring_va->imgo_ring_size;
                }
                else if(_img2o_ == pbuf_ctrl->buf_id)
                {
                    pcqrtbcring_va->img2o_ring_size++;
                    size = pcqrtbcring_va->img2o_ring_size;
                }
                
                LOG_INF("[rtbc]or_size(%d),2or_size(%d)",pcqrtbcring_va->imgo_ring_size,pcqrtbcring_va->img2o_ring_size);
                
                {
                    MINT32 *pdata = (MINT32 *)&pcqrtbcring_va->rtbc_ring[0];
                    for (i = 0; i < (sizeof(CQ_RING_CMD_ST)>>2) * size; i++ ) 
                    {
                        LOG_DBG("[rtbc](0x%08x)(0x%08x)",pdata+i,pdata[i]);
                    }
                }
            }            
            else    //VF is ON
            {
                if(pex_buf_info)
                {
                    LOG_INF("[rtbc]exchange 1st buf. by 2nd buf. and enque it(0x%x)",pex_buf_info);
                    
                    if( _imgo_ == pbuf_ctrl->buf_id)
                    {
                        for(i = 0; i < pcqrtbcring_va->imgo_ring_size; i++)
                        {
                            if(pcqrtbcring_va->rtbc_ring[i].cq_rtbc.imgo_base_pAddr == pbuf_info->base_pAddr)
                            {
                                LOG_INF("[rtbc]old(%d) imgo buffer(0x%x)",i,pcqrtbcring_va->rtbc_ring[i].cq_rtbc.imgo_base_pAddr);
                                pcqrtbcring_va->rtbc_ring[i].cq_rtbc.imgo_base_pAddr = pex_buf_info->base_pAddr;
                                LOG_INF("new(%d) imgo buffer(0x%x)",i,pcqrtbcring_va->rtbc_ring[i].cq_rtbc.imgo_base_pAddr);
                                break;
                            }
                        }
                    }
                    else if(_img2o_ == pbuf_ctrl->buf_id)
                    {
                        for(i = 0; i < pcqrtbcring_va->img2o_ring_size; i++)
                        {
                            if(pcqrtbcring_va->rtbc_ring[i].cq_rtbc.img2o_base_pAddr == pbuf_info->base_pAddr)
                            {
                                LOG_INF("[rtbc]old(%d) img2o buffer(0x%x)",i,pcqrtbcring_va->rtbc_ring[i].cq_rtbc.img2o_base_pAddr);
                                pcqrtbcring_va->rtbc_ring[i].cq_rtbc.img2o_base_pAddr = pex_buf_info->base_pAddr;
                                LOG_INF("new(%d) img2o buffer(0x%x)",i,pcqrtbcring_va->rtbc_ring[i].cq_rtbc.img2o_base_pAddr);
                                break;
                            }
                        }
                    }
                    else 
                    {
                        LOG_ERR("ERROR:DMA id (%d)",pbuf_ctrl->buf_id);
                        return MFALSE;
                    }
                }
                else 
                {                    
                }
            }            
            break;
        case ISP_RT_BUF_CTRL_EXCHANGE_ENQUE:
            
            if(IsReadOnlyMode())
            {
                reg_val = ISP_IOCTL_READ_REG(this, this->getRegAddrMap(), CAM_TG_VF_CON);
                //reg_val2 = ISP_IOCTL_READ_REG(this, this->getRegAddrMap(), CAM_TG2_VF_CON);
            }
            else
            {
                reg_val  = ISP_READ_REG(pIspPhyReg, CAM_TG_VF_CON);
                //reg_val2 = ISP_READ_REG(pIspPhyReg, CAM_TG2_VF_CON);
            }
            
            //VF on line
            if((reg_val & 0x01) || (reg_val2 & 0x01))
            {
                LOG_INF("[rtbc]exchange 1st buf. by 2nd buf. and enque it(0x%x)",pex_buf_info);
#if 0
                if ( pex_buf_info ) {
                    //
                    if ( _imgo_ == pbuf_ctrl->buf_id ) {
                        //
                        for ( i=0 ; i<pcqrtbcring_va->imgo_ring_size ; i++ ) {
                            if ( pcqrtbcring_va->rtbc_ring[i].cq_rtbc.imgo_base_pAddr == pbuf_info->base_pAddr ) {
                                    LOG_INF("[rtbc]old(%d) imgo buffer(0x%x)",i,pcqrtbcring_va->rtbc_ring[i].cq_rtbc.imgo_base_pAddr);
                                    pcqrtbcring_va->rtbc_ring[i].cq_rtbc.imgo_base_pAddr = pex_buf_info->base_pAddr;
                                    LOG_INF("new(%d) imgo buffer(0x%x)",i,pcqrtbcring_va->rtbc_ring[i].cq_rtbc.imgo_base_pAddr);
                                break;
                            }
                        }
                    }
                    else if (_img2o_ == pbuf_ctrl->buf_id) {
                        //
                        for ( i=0 ; i<pcqrtbcring_va->img2o_ring_size ; i++ ) {
                            if ( pcqrtbcring_va->rtbc_ring[i].cq_rtbc.img2o_base_pAddr == pbuf_info->base_pAddr ) {
                                    LOG_INF("[rtbc]old(%d) img2o buffer(0x%x)",i,pcqrtbcring_va->rtbc_ring[i].cq_rtbc.img2o_base_pAddr);
                                    pcqrtbcring_va->rtbc_ring[i].cq_rtbc.img2o_base_pAddr = pex_buf_info->base_pAddr;
                                    LOG_INF("new(%d) img2o buffer(0x%x)",i,pcqrtbcring_va->rtbc_ring[i].cq_rtbc.img2o_base_pAddr);
                                break;
                            }
                        }
                    }
                    else {
                        LOG_ERR("ERROR:DMA id (%d)",pbuf_ctrl->buf_id);
                        return MFALSE;
                    }
                }
                else {
                    //
                }
#endif
            }
            break;
        case ISP_RT_BUF_CTRL_CLEAR:
            
            //reset cq0c all 0 for rtbc
            if(sizeof(CQ_RTBC_RING_ST) <= sizeof(gIspCQDescInit)) 
            {
                memset((MUINT8*)mpIspCQDescriptorVirt[ISP_DRV_DESCRIPTOR_CQ0C],0,sizeof(CQ_RTBC_RING_ST));
            }
            else 
            {
                LOG_ERR("rtbc data exceed buffer size(%d)>(%d)",sizeof(CQ_RTBC_RING_ST),sizeof(gIspCQDescInit));
            }

            break;
        default:
            //LOG_ERR("ERROR:ctrl id(%d)",pbuf_ctrl->ctrl);
            break;
    }

    //reset cq0c all 0 for rtbc
    //memset((MUINT8*)mpIspCQDescriptorVirt[ISP_DRV_DESCRIPTOR_CQ0C],0,sizeof(CQ_RTBC_RING_ST));

    return MTRUE;
}

#endif

/*******************************************************************************
* 
********************************************************************************/
IspDrvVirImp::IspDrvVirImp()
{
    LOG_INF("+");
    mpIspVirRegBuffer = NULL;
}

/*******************************************************************************
* 
********************************************************************************/
IspDrvVirImp::~IspDrvVirImp()
{
    LOG_INF("+");
}

/*******************************************************************************
* 
********************************************************************************/
IspDrv *IspDrvVirImp::getInstance(ISP_DRV_CQ_ENUM cq, MUINT32 ispVirRegAddr)
{
    LOG_INF("cq(%d),ispVirRegAddr(0x%08x)", cq, ispVirRegAddr);
    
    if(ispVirRegAddr & ISP_DRV_VIR_ADDR_ALIGN)
    {
        LOG_ERR("NOT 8 bytes alignment\n");
        return NULL;
    }
    
    static IspDrvVirImp singleton[ISP_DRV_CQ_NUM];
    singleton[cq].mpIspVirRegBuffer = (MUINT32*)ispVirRegAddr;
    
    return &singleton[cq];
}

/*******************************************************************************
* 
********************************************************************************/
MVOID IspDrvVirImp::destroyInstance()
{
}

/*******************************************************************************
* 
********************************************************************************/
MUINT32 IspDrvVirImp::getRegAddr()
{
    LOG_VRB("mpIspVirRegBuffer(0x%08X)",(MUINT32)mpIspVirRegBuffer);
    
    if(mpIspVirRegBuffer != NULL)
    {
        return (MUINT32)mpIspVirRegBuffer;
    }
    else
    {
        return 0;
    }
}

/*******************************************************************************
* 
********************************************************************************/
MBOOL IspDrvVirImp::reset()
{
    memset(mpIspVirRegBuffer,0x00,sizeof(MUINT32)*ISP_BASE_RANGE);
    return MTRUE;
}

/*******************************************************************************
* 
********************************************************************************/
MBOOL IspDrvVirImp::readRegs(ISP_DRV_REG_IO_STRUCT *pRegIo, MUINT32 Count)
{
    MUINT32 i;
    
    //Mutex::Autolock lock(mLock);
   
    //LOG_DBG("Count(%d)",Count);
    
    if(mpIspVirRegBuffer == NULL)
    {
        LOG_ERR("mpIspVirRegBuffer is NULL");
        return MFALSE;
    }
    
    for(i = 0; i < Count; i++)
    {
        pRegIo[i].Data = mpIspVirRegBuffer[(pRegIo[i].Addr) >> 2];
    }
    
    return MTRUE;
}

/*******************************************************************************
* 
********************************************************************************/
MUINT32 IspDrvVirImp::readReg(MUINT32 Addr)
{
    ISP_DRV_REG_IO_STRUCT RegIo;
    
    //Mutex::Autolock lock(mLock);
    
    //LOG_DBG("Addr(0x%08X)",Addr);
    
    RegIo.Addr = Addr;
    
    if(!readRegs(&RegIo, 1))
    {
        return 0;
    }
    
    return (RegIo.Data);
}

/*******************************************************************************
* 
********************************************************************************/
MBOOL IspDrvVirImp::writeRegs(ISP_DRV_REG_IO_STRUCT *pRegIo, MUINT32 Count)
{
    MUINT32 i;
    
    //Mutex::Autolock lock(mLock);
    
    //LOG_DBG("Count(%d)",Count);
    
    if(mpIspVirRegBuffer == NULL)
    {
        LOG_ERR("mpIspVirRegBuffer is NULL");
        return MFALSE;
    }
    
    for(i = 0; i < Count; i++)
    {
        mpIspVirRegBuffer[(pRegIo[i].Addr)>>2] = pRegIo[i].Data;
    }
    
    return MTRUE;
}

/*******************************************************************************
* 
********************************************************************************/
MBOOL IspDrvVirImp::writeReg(MUINT32 Addr, MUINT32 Data)
{
    ISP_DRV_REG_IO_STRUCT RegIo;
    
    //Mutex::Autolock lock(mLock);
    
    //LOG_DBG("Addr(0x%08X),Data(0x%08X)",Addr,Data);
    
    RegIo.Addr = Addr;
    RegIo.Data = Data;
    
    if(!writeRegs(&RegIo, 1))
    {
        return MFALSE;
    }
    
    return MTRUE;
}


