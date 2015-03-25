#define LOG_TAG "iio/ifunc"

#include "isp_function.h"
#include <mtkcam/v1/config/PriorityDefs.h>
//open syscall
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
//close syscall
#include <unistd.h>
//mmap syscall
#include <sys/mman.h>
//mutex
#include <pthread.h>
//thread
#include <utils/threads.h>
#include <utils/StrongPointer.h>
//

#include "mdp_mgr.h"

#include <cutils/properties.h>  // For property_get().

#include "m4u_lib.h"

//
//digital zoom
#define CAM_ISP_ZOOMRARIO_GAIN (100)


#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#define  DBG_LOG_TAG        "{ispf}"
#include "imageio_log.h"                    // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
DECLARE_DBG_LOG_VARIABLE(function);
//EXTERN_DBG_LOG_VARIABLE(function);

// Clear previous define, use our own define.
#undef ISP_FUNC_VRB
#undef ISP_FUNC_DBG
#undef ISP_FUNC_INF
#undef ISP_FUNC_WRN
#undef ISP_FUNC_ERR
#undef ISP_FUNC_AST
#define ISP_FUNC_VRB(fmt, arg...)        do { if (function_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define ISP_FUNC_DBG(fmt, arg...)        do { if (function_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
#define ISP_FUNC_INF(fmt, arg...)        do { if (function_DbgLogEnable_INFO   ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define ISP_FUNC_WRN(fmt, arg...)        do { if (function_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define ISP_FUNC_ERR(fmt, arg...)        do { if (function_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define ISP_FUNC_AST(cond, fmt, arg...)  do { if (function_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)


#define ISP_FUNC_ORG_ISPDTREAM_FUNC 0 // avoid build err with new I/F of DpIspStream::dequeueSrcBuffer and dequeueDstBuffer
#define ISP_FUNC_MVA_ISPDTREAM_FUNC 1 //

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
//        if  (0==mIdx)
//        {
//            ISP_FUNC_INF("[%s] %s:(%d-th) ===> %.06f ms", mpszName, pszInfo, mIdx++, (float)(i4EndUs-mi4StartUs)/1000);
//        }
//        else
//        {
            ISP_FUNC_INF("[%s] %s:(%d-th) ===> %.06f ms (Total time till now: %.06f ms)", mpszName, pszInfo, mIdx++, (float)(i4EndUs-mi4LastUs)/1000, (float)(i4EndUs-mi4StartUs)/1000);
//        }
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



#define ISP_HW_SCENARIO_NUM     8
#define ISP_HW_SUB_MODE_NUM     5

   stIspTopEnTbl gIspTurningTag[ISP_HW_SCENARIO_NUM][ISP_HW_SUB_MODE_NUM]
=   {   {{0x07C008a8, 0x0000001D, 0x00001102  },  // IC_RAW(0)
         {0x07C008a8, 0x0000001D, 0x00001102  },  // IC_YUV
         {0x07C008a8, 0x0000001D, 0x00001102  },  // IC_JPG
         {0x00000000, 0x00000000, 0x00000000  },  // DUMMY
         {0x00000000, 0x00000000, 0x00000000  }}, // DUMMY
        {{0x07C008a8, 0x0000001D, 0x00001102  },  // VR_RAW(1)
         {0x07C008a8, 0x0000001D, 0x00001102  },  // VR_YUV
         {0x00000000, 0x00000000, 0x00000000  },  // DUMMY
         {0x00000000, 0x00000000, 0x00000000  },  // DUMMY
         {0x00000000, 0x00000000, 0x00000000  }}, // DUMMY
        {{0x07C00900, 0x0000001D, 0x00001100  },  // ZSD_RAW(2)
         {0x07C009a8, 0x0000001D, 0x00001102  },  // ZSD_YUV
         {0x00000000, 0x00000000, 0x00000000  },  // DUMMY
         {0x00000000, 0x00000000, 0x00000000  },  // DUMMY
         {0x00000000, 0x00000000, 0x00000000  }}, // DUMMY
        {{0x07C009a8, 0x0000001D, 0x00001102  },  // IP_RAW(3)
         {0x07C009a8, 0x0000001D, 0x00001102  },  // IP_YUV
         {0x07C008a8, 0x0000001D, 0x00001102  },  // IP_RGBW
         {0x07C008a8, 0x0000001D, 0x00001102  },  // IP_RGB_LOAD
         {0x07C008a8, 0x0000001D, 0x00001102  }}, // IP_MFB
        {{0x07C008a8, 0x0000001D, 0x00001102  },  // VEC_DUMMY(4)
         {0x00000000, 0x00000000, 0x00000000  },  // DUMMY
         {0x00000000, 0x00000000, 0x00000000  },  // DUMMY
         {0x00000000, 0x00000000, 0x00000000  },  // DUMMY
         {0x00000000, 0x00000000, 0x00000000  }}, // DUMMY
        {{0x07C008a8, 0x0000001D, 0x00001102  },  // Simultaneous_DUMMY(5)
         {0x00000000, 0x00000000, 0x00000000  },  // DUMMY
         {0x00000000, 0x00000000, 0x00000000  },  // DUMMY
         {0x00000000, 0x00000000, 0x00000000  },  // DUMMY
         {0x00000000, 0x00000000, 0x00000000  }}, // DUMMY
        {{0x07C00900, 0x0000001D, 0x00001100  },  // N3D_IC_RAW(6)
         {0x07C009a8, 0x0000001D, 0x00001102  },  // N3D_IC_YUV
         {0x00000000, 0x00000000, 0x00000000  },  // DUMMY
         {0x00000000, 0x00000000, 0x00000000  },  // DUMMY
         {0x00000000, 0x00000000, 0x00000000  }}, // DUMMY
        {{0x07C008a8, 0x0000001D, 0x00001102  },  // N3D_VR_RAW(7)
         {0x07C008a8, 0x0000001D, 0x00001102  },  // N3D_VR_YUV
         {0x07C008a8, 0x0000001D, 0x00001102  },  // N3D_VR_RGB
         {0x00000000, 0x00000000, 0x00000000  },  // DUMMY
         {0x00000000, 0x00000000, 0x00000000  }}  // DUMMY
    };


//    #define CAM_CTL_EN1_FOR_TURN    0x07C008a8      //0x4004   // GGM(Top) control by Imageio
//    #define CAM_CTL_EN2_FOR_TURN    0x0000003D      //0x4008
//    #define CAM_CTL_DMA_FOR_TURN    0x00001102      //0x400C

/****************************************************************************
* Global Variable
****************************************************************************/
static MdpMgr *g_pMdpMgr = NULL;
static MDPMGR_CFG_STRUCT g_MdpMgrCfgData[ISP_DRV_CQ_NUM];
static MUINT32 g_MdpMgrCqIdx = 0;

/****************************************************************************
* 
****************************************************************************/
int cam_get_DZ_cfg(
    MINT32 scenario,
    MINT32 tpipe_En,
    MINT32 zoomRatio,
    MUINT32 const cdrz_iW, //src_img_size
    MUINT32 const cdrz_iH,
    MUINT32 const curz_oW, //max output size
    MUINT32 const curz_oH,
    MUINT32 &cdrz_oW,      //imgo
    MUINT32 &cdrz_oH,
    MUINT32 &curz_iW,      //imgi
    MUINT32 &curz_iH,
    MUINT32 &curz_iOfst)
{
int zoomRatio_W = zoomRatio;
int zoomRatio_H = zoomRatio;

    if( ISP_SCENARIO_IC == scenario ) 
    {
        if(tpipe_En) 
        {
            //CURZ NOT support tpipeMode

            //CDRZ n-tap NOT support tpipeMode
        }
        else 
        {
        }
    }
    else if( ISP_SCENARIO_VR == scenario ) 
    {
        //frame mode always
        if(tpipe_En) 
        {
            //ISP_INFO("Error conflict:VR+tpipe");
            return 1;
        }
        // 0:curz->vido , prz->dispo
        // 1:curz->dispo, prz->vido
        //ISP_BITS(g_pIspReg, CAM_CTL_SEL, DISP_VID_SEL) = 0;

        //width
        if( ((cdrz_iW * CAM_ISP_ZOOMRARIO_GAIN )/zoomRatio_W) >= curz_oW) 
        {
            //after zoom input >= required output
            //use cdrz only
            curz_iW = curz_oW;
            cdrz_oW = ( curz_iW*zoomRatio_W + (CAM_ISP_ZOOMRARIO_GAIN >> 1) ) / CAM_ISP_ZOOMRARIO_GAIN;
        }
        else 
        {
            //use curz
            cdrz_oW = cdrz_iW;
            curz_iW = ( (cdrz_oW * CAM_ISP_ZOOMRARIO_GAIN) + (zoomRatio_W>>1) )/zoomRatio_W;
        }

        //height
        if( ((cdrz_iH * CAM_ISP_ZOOMRARIO_GAIN )/zoomRatio_H) >= curz_oH)
        {
            curz_iH = curz_oH;
            cdrz_oH = ( curz_iH*zoomRatio_H + (CAM_ISP_ZOOMRARIO_GAIN >> 1) ) / CAM_ISP_ZOOMRARIO_GAIN;
        }
        else 
        {
            cdrz_oH = cdrz_iH;
            curz_iH = ( (cdrz_oH * CAM_ISP_ZOOMRARIO_GAIN) + (zoomRatio_H>>1) )/zoomRatio_H;
        }

        //make even number
        cdrz_oW &= (~0x01);
        cdrz_oH &= (~0x01);
        curz_iW &= (~0x01);
        curz_iH &= (~0x01);
        curz_iOfst = cdrz_oW * ( ( cdrz_oH - curz_iH ) >> 1 ) + ( ( cdrz_oW - curz_iW ) >> 1 );
    }
    return 0;
}

/*/////////////////////////////////////////////////////////////////////////////
    Isp driver base
/////////////////////////////////////////////////////////////////////////////*/
//
IspDrv*         IspDrv_B::m_pIspDrv = NULL;
isp_reg_t*      IspDrv_B::m_pIspReg;

#if defined(USING_MTK_LDVT)
    //debug path, imagio will config all engine by CQ
    default_func    IspDrv_B::default_setting_function[] = {NULL,NULL,NULL,NULL,
                                                            NULL,NULL,NULL,NULL,
                                                            NULL,NULL,NULL,NULL,
                                                            NULL,NULL,NULL,NULL,
                                                            NULL,NULL,NULL,NULL,
                                                            NULL,IspDrv_B::cam_isp_cfa_cfg,NULL,IspDrv_B::cam_isp_g2g_cfg,
                                                            IspDrv_B::cam_isp_dgm_cfg,NULL,NULL,NULL,
                                                            IspDrv_B::cam_isp_mfb_cfg,IspDrv_B::cam_isp_c24_cfg,NULL,NULL};
#else
    //MP path, do not config engine by imageio
    default_func    IspDrv_B::default_setting_function[] = {NULL,NULL,NULL,NULL,
                                                            NULL,NULL,NULL,NULL,
                                                            NULL,NULL,NULL,NULL,
                                                            NULL,NULL,NULL,NULL,
                                                            NULL,NULL,NULL,NULL,
                                                            NULL,NULL,NULL,IspDrv_B::cam_isp_g2g_cfg,
                                                            NULL,NULL,NULL,NULL,
                                                            NULL,IspDrv_B::cam_isp_c24_cfg,NULL,NULL};
#endif

    
int ISP_TOP_CTRL::pix_id;
/*** isp module default setting ***/
MINT32
IspDrv_B::
cam_isp_hrz_cfg(MINT32 iHeight,MINT32 resize,MINT32 oSize)
{
    ISP_FUNC_DBG("E");

/*
HRZ_INPUT_HEIGHT:
    "HRZ input size
    if (tg_sel = 0) height=image0
    else height = image1"

HRZ_RESIZE:
    "HRZ resize value
    Get from 32768/decimation ratio
    decimation x1.0: hrz_resize should be 32768
    decimation x0.5: hrz_resize should be 65536
    decimation x0.25: hrz_resize should be 131072 "

HRZ_OUTSIZE:
    "HRZ output size
    And this value get from
    X1 = input size * 32768 / hrz_resize
    hbn_outsize = 2* (X1>>1)"
*/
    //m_pIspDrv->writeReg(0x00004580, 0x000116E2);   /* 0x15004580: CAM_HRZ_RES */
    //m_pIspDrv->writeReg(0x00004584, 0x00080040);   /* 0x15004584: CAM_HRZ_OUT */
    if(m_pIspDrv->IsReadOnlyMode())
    {
        ISP_IOCTL_WRITE_BITS(m_pIspDrv, m_pIspDrv->getRegAddrMap(), CAM_HRZ_RES,HRZ_INPUT_HEIGHT ,iHeight);
        ISP_IOCTL_WRITE_BITS(m_pIspDrv, m_pIspDrv->getRegAddrMap(), CAM_HRZ_RES,HRZ_RESIZE, resize);
        ISP_IOCTL_WRITE_BITS(m_pIspDrv, m_pIspDrv->getRegAddrMap(), CAM_HRZ_OUT,HRZ_OUTSIZE, oSize);
        ISP_IOCTL_WRITE_BITS(m_pIspDrv, m_pIspDrv->getRegAddrMap(), CAM_HRZ_OUT,HRZ_BPCTH, 0);
    }
    else
    {
        ISP_WRITE_BITS(m_pIspReg, CAM_HRZ_RES,HRZ_INPUT_HEIGHT ,iHeight);
        ISP_WRITE_BITS(m_pIspReg, CAM_HRZ_RES,HRZ_RESIZE, resize);
        ISP_WRITE_BITS(m_pIspReg, CAM_HRZ_OUT,HRZ_OUTSIZE, oSize);
        ISP_WRITE_BITS(m_pIspReg, CAM_HRZ_OUT,HRZ_BPCTH, 0);
    }
    return 0;
}

int
IspDrv_B::
cam_isp_cfa_cfg(void) {
    ISP_FUNC_DBG("cam_isp_cfa_cfg m_pIspDrv(0x%x)\n",m_pIspDrv);
    m_pIspDrv->writeReg(0x000048A0, 0x00000000);	/* 0x150048A0: CAM_CFA_BYPASS */
    m_pIspDrv->writeReg(0x000048A4, 0x00000C03);	/* 0x150048A4: CAM_CFA_ED_F */
    m_pIspDrv->writeReg(0x000048A8, 0x01082006);	/* 0x150048A8: CAM_CFA_ED_NYQ */
    m_pIspDrv->writeReg(0x000048AC, 0x80081008);	/* 0x150048AC: CAM_CFA_ED_STEP */
    m_pIspDrv->writeReg(0x000048B0, 0x37084208);	/* 0x150048B0: CAM_CFA_RGB_HF */
    m_pIspDrv->writeReg(0x000048B4, 0x1806200A);	/* 0x150048B4: CAM_CFA_BW_BB */
    m_pIspDrv->writeReg(0x000048B8, 0x0010A020);	/* 0x150048B8: CAM_CFA_F1_ACT */
    m_pIspDrv->writeReg(0x000048BC, 0x0015C020);	/* 0x150048BC: CAM_CFA_F2_ACT */
    m_pIspDrv->writeReg(0x000048C0, 0x0015C040);	/* 0x150048C0: CAM_CFA_F3_ACT */
    m_pIspDrv->writeReg(0x000048C4, 0x00350050);	/* 0x150048C4: CAM_CFA_F4_ACT */
    m_pIspDrv->writeReg(0x000048C8, 0x00A41440);	/* 0x150048C8: CAM_CFA_F1_L */
    m_pIspDrv->writeReg(0x000048CC, 0x00421084);	/* 0x150048CC: CAM_CFA_F2_L */
    m_pIspDrv->writeReg(0x000048D0, 0x01484185);	/* 0x150048D0: CAM_CFA_F3_L */
    m_pIspDrv->writeReg(0x000048D4, 0x00410417);	/* 0x150048D4: CAM_CFA_F4_L */
    m_pIspDrv->writeReg(0x000048D8, 0x000203FF);	/* 0x150048D8: CAM_CFA_HF_RB */
    m_pIspDrv->writeReg(0x000048DC, 0x00000008);	/* 0x150048DC: CAM_CFA_HF_GAIN */
    m_pIspDrv->writeReg(0x000048E0, 0xE0088888);	/* 0x150048E0: CAM_CFA_HF_COMP */
    m_pIspDrv->writeReg(0x000048E4, 0x00000010);	/* 0x150048E4: CAM_CFA_HF_CORING_TH */
    m_pIspDrv->writeReg(0x000048E8, 0x0052A5FF);	/* 0x150048E8: CAM_CFA_ACT_LUT */
    m_pIspDrv->writeReg(0x000048F0, 0x781F55D4);	/* 0x150048F0: CAM_CFA_SPARE */
    m_pIspDrv->writeReg(0x000048F4, 0x00018006);	/* 0x150048F4: CAM_CFA_BB*/
    return 0;
}
//
int
IspDrv_B::
cam_isp_g2g_cfg(void) {
    ISP_FUNC_DBG("cam_isp_g2g_cfg ");

    m_pIspDrv->writeReg(0x00004920, 0x00000200);	/* 0x15004920: CAM_G2G_CONV0A */
    m_pIspDrv->writeReg(0x00004924, 0x000002CE);	/* 0x15004924: CAM_G2G_CONV0B */
    m_pIspDrv->writeReg(0x00004928, 0x07500200);	/* 0x15004928: CAM_G2G_CONV1A */
    m_pIspDrv->writeReg(0x0000492C, 0x00000692);	/* 0x1500492C: CAM_G2G_CONV1B */
    m_pIspDrv->writeReg(0x00004930, 0x038B0200);	/* 0x15004930: CAM_G2G_CONV2A */
    m_pIspDrv->writeReg(0x00004934, 0x00000000);	/* 0x15004934: CAM_G2G_CONV2B */
    m_pIspDrv->writeReg(0x00004938, 0x00000009);	/* 0x15004938: CAM_G2G_ACC */
    return 0;
}
//
int
IspDrv_B::
cam_isp_dgm_cfg(void) {
    ISP_FUNC_DBG("cam_isp_dgm_cfg ");
    m_pIspDrv->writeReg(0x00004E30, 0x00200010);	/* 0x15004E30: CAM_DGM_B2 */
    m_pIspDrv->writeReg(0x00004E34, 0x00400030);	/* 0x15004E34: CAM_DGM_B4 */
    m_pIspDrv->writeReg(0x00004E38, 0x00800060);	/* 0x15004E38: CAM_DGM_B6 */
    m_pIspDrv->writeReg(0x00004E3C, 0x00C000A0);	/* 0x15004E3C: CAM_DGM_B8 */
    m_pIspDrv->writeReg(0x00004E40, 0x00E000D0);	/* 0x15004E40: CAM_DGM_B10 */
    m_pIspDrv->writeReg(0x00004E44, 0x000000F0);	/* 0x15004E44: CAM_DGM_B11 */
    return 0;
}
//
int
IspDrv_B::
cam_isp_ggm_cfg(void) {
    ISP_FUNC_DBG("cam_isp_ggm_cfg ");
//new
    m_pIspDrv->writeReg(0x00005000, 0x3000F400);   /* 0x15005000: CAM_GGM_RB_GMT[0] */
    m_pIspDrv->writeReg(0x00005004, 0x240C5C3D);   /* 0x15005004: CAM_GGM_RB_GMT[1] */
    m_pIspDrv->writeReg(0x00005008, 0x1C154454);   /* 0x15005008: CAM_GGM_RB_GMT[2] */
    m_pIspDrv->writeReg(0x0000500C, 0x181C3865);   /* 0x1500500C: CAM_GGM_RB_GMT[3] */
    m_pIspDrv->writeReg(0x00005010, 0x18223073);   /* 0x15005010: CAM_GGM_RB_GMT[4] */
    m_pIspDrv->writeReg(0x00005014, 0x14282C7F);   /* 0x15005014: CAM_GGM_RB_GMT[5] */
    m_pIspDrv->writeReg(0x00005018, 0x142D288A);   /* 0x15005018: CAM_GGM_RB_GMT[6] */
    m_pIspDrv->writeReg(0x0000501C, 0x14322494);   /* 0x1500501C: CAM_GGM_RB_GMT[7] */
    m_pIspDrv->writeReg(0x00005020, 0x1437249D);   /* 0x15005020: CAM_GGM_RB_GMT[8] */
    m_pIspDrv->writeReg(0x00005024, 0x143C20A6);   /* 0x15005024: CAM_GGM_RB_GMT[9] */
    m_pIspDrv->writeReg(0x00005028, 0x10411CAE);   /* 0x15005028: CAM_GGM_RB_GMT[10] */
    m_pIspDrv->writeReg(0x0000502C, 0x144520B5);   /* 0x1500502C: CAM_GGM_RB_GMT[11] */
    m_pIspDrv->writeReg(0x00005030, 0x104A1CBD);   /* 0x15005030: CAM_GGM_RB_GMT[12] */
    m_pIspDrv->writeReg(0x00005034, 0x104E18C4);   /* 0x15005034: CAM_GGM_RB_GMT[13] */
    m_pIspDrv->writeReg(0x00005038, 0x10521CCA);   /* 0x15005038: CAM_GGM_RB_GMT[14] */
    m_pIspDrv->writeReg(0x0000503C, 0x105618D1);   /* 0x1500503C: CAM_GGM_RB_GMT[15] */
    m_pIspDrv->writeReg(0x00005040, 0x105A18D7);   /* 0x15005040: CAM_GGM_RB_GMT[16] */
    m_pIspDrv->writeReg(0x00005044, 0x105E14DD);   /* 0x15005044: CAM_GGM_RB_GMT[17] */
    m_pIspDrv->writeReg(0x00005048, 0x106218E2);   /* 0x15005048: CAM_GGM_RB_GMT[18] */
    m_pIspDrv->writeReg(0x0000504C, 0x0C6618E8);   /* 0x1500504C: CAM_GGM_RB_GMT[19] */
    m_pIspDrv->writeReg(0x00005050, 0x106914EE);   /* 0x15005050: CAM_GGM_RB_GMT[20] */
    m_pIspDrv->writeReg(0x00005054, 0x106D14F3);   /* 0x15005054: CAM_GGM_RB_GMT[21] */
    m_pIspDrv->writeReg(0x00005058, 0x0C7114F8);   /* 0x15005058: CAM_GGM_RB_GMT[22] */
    m_pIspDrv->writeReg(0x0000505C, 0x107414FD);   /* 0x1500505C: CAM_GGM_RB_GMT[23] */
    m_pIspDrv->writeReg(0x00005060, 0x0C781502);   /* 0x15005060: CAM_GGM_RB_GMT[24] */
    m_pIspDrv->writeReg(0x00005064, 0x107B1107);   /* 0x15005064: CAM_GGM_RB_GMT[25] */
    m_pIspDrv->writeReg(0x00005068, 0x0C7F150B);   /* 0x15005068: CAM_GGM_RB_GMT[26] */
    m_pIspDrv->writeReg(0x0000506C, 0x0C821110);   /* 0x1500506C: CAM_GGM_RB_GMT[27] */
    m_pIspDrv->writeReg(0x00005070, 0x10851514);   /* 0x15005070: CAM_GGM_RB_GMT[28] */
    m_pIspDrv->writeReg(0x00005074, 0x0C891119);   /* 0x15005074: CAM_GGM_RB_GMT[29] */
    m_pIspDrv->writeReg(0x00005078, 0x0C8C111D);   /* 0x15005078: CAM_GGM_RB_GMT[30] */
    m_pIspDrv->writeReg(0x0000507C, 0x108F1521);   /* 0x1500507C: CAM_GGM_RB_GMT[31] */
    m_pIspDrv->writeReg(0x00005080, 0x0C931126);   /* 0x15005080: CAM_GGM_RB_GMT[32] */
    m_pIspDrv->writeReg(0x00005084, 0x0C96112A);   /* 0x15005084: CAM_GGM_RB_GMT[33] */
    m_pIspDrv->writeReg(0x00005088, 0x0C99112E);   /* 0x15005088: CAM_GGM_RB_GMT[34] */
    m_pIspDrv->writeReg(0x0000508C, 0x0C9C1132);   /* 0x1500508C: CAM_GGM_RB_GMT[35] */
    m_pIspDrv->writeReg(0x00005090, 0x0C9F0D36);   /* 0x15005090: CAM_GGM_RB_GMT[36] */
    m_pIspDrv->writeReg(0x00005094, 0x0CA21139);   /* 0x15005094: CAM_GGM_RB_GMT[37] */
    m_pIspDrv->writeReg(0x00005098, 0x0CA5113D);   /* 0x15005098: CAM_GGM_RB_GMT[38] */
    m_pIspDrv->writeReg(0x0000509C, 0x0CA81141);   /* 0x1500509C: CAM_GGM_RB_GMT[39] */
    m_pIspDrv->writeReg(0x000050A0, 0x0CAB0D45);   /* 0x150050A0: CAM_GGM_RB_GMT[40] */
    m_pIspDrv->writeReg(0x000050A4, 0x0CAE1148);   /* 0x150050A4: CAM_GGM_RB_GMT[41] */
    m_pIspDrv->writeReg(0x000050A8, 0x0CB10D4C);   /* 0x150050A8: CAM_GGM_RB_GMT[42] */
    m_pIspDrv->writeReg(0x000050AC, 0x0CB4114F);   /* 0x150050AC: CAM_GGM_RB_GMT[43] */
    m_pIspDrv->writeReg(0x000050B0, 0x0CB70D53);   /* 0x150050B0: CAM_GGM_RB_GMT[44] */
    m_pIspDrv->writeReg(0x000050B4, 0x0CBA1156);   /* 0x150050B4: CAM_GGM_RB_GMT[45] */
    m_pIspDrv->writeReg(0x000050B8, 0x0CBD0D5A);   /* 0x150050B8: CAM_GGM_RB_GMT[46] */
    m_pIspDrv->writeReg(0x000050BC, 0x0CC00D5D);   /* 0x150050BC: CAM_GGM_RB_GMT[47] */
    m_pIspDrv->writeReg(0x000050C0, 0x0CC31160);   /* 0x150050C0: CAM_GGM_RB_GMT[48] */
    m_pIspDrv->writeReg(0x000050C4, 0x08C60D64);   /* 0x150050C4: CAM_GGM_RB_GMT[49] */
    m_pIspDrv->writeReg(0x000050C8, 0x0CC80D67);   /* 0x150050C8: CAM_GGM_RB_GMT[50] */
    m_pIspDrv->writeReg(0x000050CC, 0x0CCB0D6A);   /* 0x150050CC: CAM_GGM_RB_GMT[51] */
    m_pIspDrv->writeReg(0x000050D0, 0x0CCE116D);   /* 0x150050D0: CAM_GGM_RB_GMT[52] */
    m_pIspDrv->writeReg(0x000050D4, 0x0CD10D71);   /* 0x150050D4: CAM_GGM_RB_GMT[53] */
    m_pIspDrv->writeReg(0x000050D8, 0x08D40D74);   /* 0x150050D8: CAM_GGM_RB_GMT[54] */
    m_pIspDrv->writeReg(0x000050DC, 0x0CD60D77);   /* 0x150050DC: CAM_GGM_RB_GMT[55] */
    m_pIspDrv->writeReg(0x000050E0, 0x0CD90D7A);   /* 0x150050E0: CAM_GGM_RB_GMT[56] */
    m_pIspDrv->writeReg(0x000050E4, 0x08DC0D7D);   /* 0x150050E4: CAM_GGM_RB_GMT[57] */
    m_pIspDrv->writeReg(0x000050E8, 0x0CDE0D80);   /* 0x150050E8: CAM_GGM_RB_GMT[58] */
    m_pIspDrv->writeReg(0x000050EC, 0x0CE10D83);   /* 0x150050EC: CAM_GGM_RB_GMT[59] */
    m_pIspDrv->writeReg(0x000050F0, 0x08E40D86);   /* 0x150050F0: CAM_GGM_RB_GMT[60] */
    m_pIspDrv->writeReg(0x000050F4, 0x0CE60D89);   /* 0x150050F4: CAM_GGM_RB_GMT[61] */
    m_pIspDrv->writeReg(0x000050F8, 0x0CE9098C);   /* 0x150050F8: CAM_GGM_RB_GMT[62] */
    m_pIspDrv->writeReg(0x000050FC, 0x08EC0D8E);   /* 0x150050FC: CAM_GGM_RB_GMT[63] */
    m_pIspDrv->writeReg(0x00005100, 0x18EE1991);   /* 0x15005100: CAM_GGM_RB_GMT[64] */
    m_pIspDrv->writeReg(0x00005104, 0x14F41597);   /* 0x15005104: CAM_GGM_RB_GMT[65] */
    m_pIspDrv->writeReg(0x00005108, 0x14F9199C);   /* 0x15005108: CAM_GGM_RB_GMT[66] */
    m_pIspDrv->writeReg(0x0000510C, 0x14FE15A2);   /* 0x1500510C: CAM_GGM_RB_GMT[67] */
    m_pIspDrv->writeReg(0x00005110, 0x150315A7);   /* 0x15005110: CAM_GGM_RB_GMT[68] */
    m_pIspDrv->writeReg(0x00005114, 0x150819AC);   /* 0x15005114: CAM_GGM_RB_GMT[69] */
    m_pIspDrv->writeReg(0x00005118, 0x150D15B2);   /* 0x15005118: CAM_GGM_RB_GMT[70] */
    m_pIspDrv->writeReg(0x0000511C, 0x151215B7);   /* 0x1500511C: CAM_GGM_RB_GMT[71] */
    m_pIspDrv->writeReg(0x00005120, 0x151715BC);   /* 0x15005120: CAM_GGM_RB_GMT[72] */
    m_pIspDrv->writeReg(0x00005124, 0x111C11C1);   /* 0x15005124: CAM_GGM_RB_GMT[73] */
    m_pIspDrv->writeReg(0x00005128, 0x152015C5);   /* 0x15005128: CAM_GGM_RB_GMT[74] */
    m_pIspDrv->writeReg(0x0000512C, 0x152515CA);   /* 0x1500512C: CAM_GGM_RB_GMT[75] */
    m_pIspDrv->writeReg(0x00005130, 0x152A15CF);   /* 0x15005130: CAM_GGM_RB_GMT[76] */
    m_pIspDrv->writeReg(0x00005134, 0x112F11D4);   /* 0x15005134: CAM_GGM_RB_GMT[77] */
    m_pIspDrv->writeReg(0x00005138, 0x153315D8);   /* 0x15005138: CAM_GGM_RB_GMT[78] */
    m_pIspDrv->writeReg(0x0000513C, 0x153815DD);   /* 0x1500513C: CAM_GGM_RB_GMT[79] */
    m_pIspDrv->writeReg(0x00005140, 0x113D11E2);   /* 0x15005140: CAM_GGM_RB_GMT[80] */
    m_pIspDrv->writeReg(0x00005144, 0x154115E6);   /* 0x15005144: CAM_GGM_RB_GMT[81] */
    m_pIspDrv->writeReg(0x00005148, 0x154611EB);   /* 0x15005148: CAM_GGM_RB_GMT[82] */
    m_pIspDrv->writeReg(0x0000514C, 0x114B11EF);   /* 0x1500514C: CAM_GGM_RB_GMT[83] */
    m_pIspDrv->writeReg(0x00005150, 0x154F15F3);   /* 0x15005150: CAM_GGM_RB_GMT[84] */
    m_pIspDrv->writeReg(0x00005154, 0x115411F8);   /* 0x15005154: CAM_GGM_RB_GMT[85] */
    m_pIspDrv->writeReg(0x00005158, 0x115811FC);   /* 0x15005158: CAM_GGM_RB_GMT[86] */
    m_pIspDrv->writeReg(0x0000515C, 0x155C1200);   /* 0x1500515C: CAM_GGM_RB_GMT[87] */
    m_pIspDrv->writeReg(0x00005160, 0x11611204);   /* 0x15005160: CAM_GGM_RB_GMT[88] */
    m_pIspDrv->writeReg(0x00005164, 0x15651208);   /* 0x15005164: CAM_GGM_RB_GMT[89] */
    m_pIspDrv->writeReg(0x00005168, 0x116A160C);   /* 0x15005168: CAM_GGM_RB_GMT[90] */
    m_pIspDrv->writeReg(0x0000516C, 0x116E1211);   /* 0x1500516C: CAM_GGM_RB_GMT[91] */
    m_pIspDrv->writeReg(0x00005170, 0x15721215);   /* 0x15005170: CAM_GGM_RB_GMT[92] */
    m_pIspDrv->writeReg(0x00005174, 0x11770E19);   /* 0x15005174: CAM_GGM_RB_GMT[93] */
    m_pIspDrv->writeReg(0x00005178, 0x117B121C);   /* 0x15005178: CAM_GGM_RB_GMT[94] */
    m_pIspDrv->writeReg(0x0000517C, 0x157F1220);   /* 0x1500517C: CAM_GGM_RB_GMT[95] */
    m_pIspDrv->writeReg(0x00005180, 0x21842224);   /* 0x15005180: CAM_GGM_RB_GMT[96] */
    m_pIspDrv->writeReg(0x00005184, 0x218C1E2C);   /* 0x15005184: CAM_GGM_RB_GMT[97] */
    m_pIspDrv->writeReg(0x00005188, 0x25942233);   /* 0x15005188: CAM_GGM_RB_GMT[98] */
    m_pIspDrv->writeReg(0x0000518C, 0x219D1E3B);   /* 0x1500518C: CAM_GGM_RB_GMT[99] */
    m_pIspDrv->writeReg(0x00005190, 0x21A51E42);   /* 0x15005190: CAM_GGM_RB_GMT[100] */
    m_pIspDrv->writeReg(0x00005194, 0x21AD1E49);   /* 0x15005194: CAM_GGM_RB_GMT[101] */
    m_pIspDrv->writeReg(0x00005198, 0x21B51E50);   /* 0x15005198: CAM_GGM_RB_GMT[102] */
    m_pIspDrv->writeReg(0x0000519C, 0x21BD1E57);   /* 0x1500519C: CAM_GGM_RB_GMT[103] */
    m_pIspDrv->writeReg(0x000051A0, 0x21C51E5E);   /* 0x150051A0: CAM_GGM_RB_GMT[104] */
    m_pIspDrv->writeReg(0x000051A4, 0x21CD1E65);   /* 0x150051A4: CAM_GGM_RB_GMT[105] */
    m_pIspDrv->writeReg(0x000051A8, 0x21D51A6C);   /* 0x150051A8: CAM_GGM_RB_GMT[106] */
    m_pIspDrv->writeReg(0x000051AC, 0x1DDD1E72);   /* 0x150051AC: CAM_GGM_RB_GMT[107] */
    m_pIspDrv->writeReg(0x000051B0, 0x21E41A79);   /* 0x150051B0: CAM_GGM_RB_GMT[108] */
    m_pIspDrv->writeReg(0x000051B4, 0x21EC1E7F);   /* 0x150051B4: CAM_GGM_RB_GMT[109] */
    m_pIspDrv->writeReg(0x000051B8, 0x1DF41A86);   /* 0x150051B8: CAM_GGM_RB_GMT[110] */
    m_pIspDrv->writeReg(0x000051BC, 0x21FB1A8C);   /* 0x150051BC: CAM_GGM_RB_GMT[111] */
    m_pIspDrv->writeReg(0x000051C0, 0x1E031A92);   /* 0x150051C0: CAM_GGM_RB_GMT[112] */
    m_pIspDrv->writeReg(0x000051C4, 0x220A1A98);   /* 0x150051C4: CAM_GGM_RB_GMT[113] */
    m_pIspDrv->writeReg(0x000051C8, 0x1E121A9E);   /* 0x150051C8: CAM_GGM_RB_GMT[114] */
    m_pIspDrv->writeReg(0x000051CC, 0x22191AA4);   /* 0x150051CC: CAM_GGM_RB_GMT[115] */
    m_pIspDrv->writeReg(0x000051D0, 0x1E211AAA);   /* 0x150051D0: CAM_GGM_RB_GMT[116] */
    m_pIspDrv->writeReg(0x000051D4, 0x1E281AB0);   /* 0x150051D4: CAM_GGM_RB_GMT[117] */
    m_pIspDrv->writeReg(0x000051D8, 0x1E2F1AB6);   /* 0x150051D8: CAM_GGM_RB_GMT[118] */
    m_pIspDrv->writeReg(0x000051DC, 0x223616BC);   /* 0x150051DC: CAM_GGM_RB_GMT[119] */
    m_pIspDrv->writeReg(0x000051E0, 0x1E3E1AC1);   /* 0x150051E0: CAM_GGM_RB_GMT[120] */
    m_pIspDrv->writeReg(0x000051E4, 0x1E451AC7);   /* 0x150051E4: CAM_GGM_RB_GMT[121] */
    m_pIspDrv->writeReg(0x000051E8, 0x1E4C16CD);   /* 0x150051E8: CAM_GGM_RB_GMT[122] */
    m_pIspDrv->writeReg(0x000051EC, 0x1E531AD2);   /* 0x150051EC: CAM_GGM_RB_GMT[123] */
    m_pIspDrv->writeReg(0x000051F0, 0x1E5A16D8);   /* 0x150051F0: CAM_GGM_RB_GMT[124] */
    m_pIspDrv->writeReg(0x000051F4, 0x1E6116DD);   /* 0x150051F4: CAM_GGM_RB_GMT[125] */
    m_pIspDrv->writeReg(0x000051F8, 0x1E681AE2);   /* 0x150051F8: CAM_GGM_RB_GMT[126] */
    m_pIspDrv->writeReg(0x000051FC, 0x1E6F16E8);   /* 0x150051FC: CAM_GGM_RB_GMT[127] */
    m_pIspDrv->writeReg(0x00005200, 0x6E7656ED);   /* 0x15005200: CAM_GGM_RB_GMT[128] */
    m_pIspDrv->writeReg(0x00005204, 0x6E915302);   /* 0x15005204: CAM_GGM_RB_GMT[129] */
    m_pIspDrv->writeReg(0x00005208, 0x6AAC4F16);   /* 0x15005208: CAM_GGM_RB_GMT[130] */
    m_pIspDrv->writeReg(0x0000520C, 0x6AC64F29);   /* 0x1500520C: CAM_GGM_RB_GMT[131] */
    m_pIspDrv->writeReg(0x00005210, 0x6AE04F3C);   /* 0x15005210: CAM_GGM_RB_GMT[132] */
    m_pIspDrv->writeReg(0x00005214, 0x66FA4B4F);   /* 0x15005214: CAM_GGM_RB_GMT[133] */
    m_pIspDrv->writeReg(0x00005218, 0x67134761);   /* 0x15005218: CAM_GGM_RB_GMT[134] */
    m_pIspDrv->writeReg(0x0000521C, 0x672C4772);   /* 0x1500521C: CAM_GGM_RB_GMT[135] */
    m_pIspDrv->writeReg(0x00005220, 0x63454783);   /* 0x15005220: CAM_GGM_RB_GMT[136] */
    m_pIspDrv->writeReg(0x00005224, 0x635D4394);   /* 0x15005224: CAM_GGM_RB_GMT[137] */
    m_pIspDrv->writeReg(0x00005228, 0x637543A4);   /* 0x15005228: CAM_GGM_RB_GMT[138] */
    m_pIspDrv->writeReg(0x0000522C, 0x5F8D43B4);   /* 0x1500522C: CAM_GGM_RB_GMT[139] */
    m_pIspDrv->writeReg(0x00005230, 0x5FA43FC4);   /* 0x15005230: CAM_GGM_RB_GMT[140] */
    m_pIspDrv->writeReg(0x00005234, 0x5FBB3FD3);   /* 0x15005234: CAM_GGM_RB_GMT[141] */
    m_pIspDrv->writeReg(0x00005238, 0x5FD23FE2);   /* 0x15005238: CAM_GGM_RB_GMT[142] */
    m_pIspDrv->writeReg(0x0000523C, 0x5BE93BF1);   /* 0x1500523C: CAM_GGM_RB_GMT[143] */
    m_pIspDrv->writeReg(0x00005300, 0x00008400);   /* 0x15005300: CAM_GGM_G_GMT[0] */
    m_pIspDrv->writeReg(0x00005304, 0x00003C21);   /* 0x15005304: CAM_GGM_G_GMT[1] */
    m_pIspDrv->writeReg(0x00005308, 0x00003030);   /* 0x15005308: CAM_GGM_G_GMT[2] */
    m_pIspDrv->writeReg(0x0000530C, 0x00002C3C);   /* 0x1500530C: CAM_GGM_G_GMT[3] */
    m_pIspDrv->writeReg(0x00005310, 0x00002447);   /* 0x15005310: CAM_GGM_G_GMT[4] */
    m_pIspDrv->writeReg(0x00005314, 0x00002050);   /* 0x15005314: CAM_GGM_G_GMT[5] */
    m_pIspDrv->writeReg(0x00005318, 0x00002058);   /* 0x15005318: CAM_GGM_G_GMT[6] */
    m_pIspDrv->writeReg(0x0000531C, 0x00001C60);   /* 0x1500531C: CAM_GGM_G_GMT[7] */
    m_pIspDrv->writeReg(0x00005320, 0x00001C67);   /* 0x15005320: CAM_GGM_G_GMT[8] */
    m_pIspDrv->writeReg(0x00005324, 0x00001C6E);   /* 0x15005324: CAM_GGM_G_GMT[9] */
    m_pIspDrv->writeReg(0x00005328, 0x00001875);   /* 0x15005328: CAM_GGM_G_GMT[10] */
    m_pIspDrv->writeReg(0x0000532C, 0x0000187B);   /* 0x1500532C: CAM_GGM_G_GMT[11] */
    m_pIspDrv->writeReg(0x00005330, 0x00001881);   /* 0x15005330: CAM_GGM_G_GMT[12] */
    m_pIspDrv->writeReg(0x00005334, 0x00001887);   /* 0x15005334: CAM_GGM_G_GMT[13] */
    m_pIspDrv->writeReg(0x00005338, 0x0000148D);   /* 0x15005338: CAM_GGM_G_GMT[14] */
    m_pIspDrv->writeReg(0x0000533C, 0x00001892);   /* 0x1500533C: CAM_GGM_G_GMT[15] */
    m_pIspDrv->writeReg(0x00005340, 0x00001498);   /* 0x15005340: CAM_GGM_G_GMT[16] */
    m_pIspDrv->writeReg(0x00005344, 0x0000149D);   /* 0x15005344: CAM_GGM_G_GMT[17] */
    m_pIspDrv->writeReg(0x00005348, 0x000014A2);   /* 0x15005348: CAM_GGM_G_GMT[18] */
    m_pIspDrv->writeReg(0x0000534C, 0x000014A7);   /* 0x1500534C: CAM_GGM_G_GMT[19] */
    m_pIspDrv->writeReg(0x00005350, 0x000010AC);   /* 0x15005350: CAM_GGM_G_GMT[20] */
    m_pIspDrv->writeReg(0x00005354, 0x000014B0);   /* 0x15005354: CAM_GGM_G_GMT[21] */
    m_pIspDrv->writeReg(0x00005358, 0x000010B5);   /* 0x15005358: CAM_GGM_G_GMT[22] */
    m_pIspDrv->writeReg(0x0000535C, 0x000014B9);   /* 0x1500535C: CAM_GGM_G_GMT[23] */
    m_pIspDrv->writeReg(0x00005360, 0x000010BE);   /* 0x15005360: CAM_GGM_G_GMT[24] */
    m_pIspDrv->writeReg(0x00005364, 0x000010C2);   /* 0x15005364: CAM_GGM_G_GMT[25] */
    m_pIspDrv->writeReg(0x00005368, 0x000010C6);   /* 0x15005368: CAM_GGM_G_GMT[26] */
    m_pIspDrv->writeReg(0x0000536C, 0x000014CA);   /* 0x1500536C: CAM_GGM_G_GMT[27] */
    m_pIspDrv->writeReg(0x00005370, 0x000010CF);   /* 0x15005370: CAM_GGM_G_GMT[28] */
    m_pIspDrv->writeReg(0x00005374, 0x000010D3);   /* 0x15005374: CAM_GGM_G_GMT[29] */
    m_pIspDrv->writeReg(0x00005378, 0x000010D7);   /* 0x15005378: CAM_GGM_G_GMT[30] */
    m_pIspDrv->writeReg(0x0000537C, 0x00000CDB);   /* 0x1500537C: CAM_GGM_G_GMT[31] */
    m_pIspDrv->writeReg(0x00005380, 0x000010DE);   /* 0x15005380: CAM_GGM_G_GMT[32] */
    m_pIspDrv->writeReg(0x00005384, 0x000010E2);   /* 0x15005384: CAM_GGM_G_GMT[33] */
    m_pIspDrv->writeReg(0x00005388, 0x000010E6);   /* 0x15005388: CAM_GGM_G_GMT[34] */
    m_pIspDrv->writeReg(0x0000538C, 0x00000CEA);   /* 0x1500538C: CAM_GGM_G_GMT[35] */
    m_pIspDrv->writeReg(0x00005390, 0x000010ED);   /* 0x15005390: CAM_GGM_G_GMT[36] */
    m_pIspDrv->writeReg(0x00005394, 0x00000CF1);   /* 0x15005394: CAM_GGM_G_GMT[37] */
    m_pIspDrv->writeReg(0x00005398, 0x000010F4);   /* 0x15005398: CAM_GGM_G_GMT[38] */
    m_pIspDrv->writeReg(0x0000539C, 0x00000CF8);   /* 0x1500539C: CAM_GGM_G_GMT[39] */
    m_pIspDrv->writeReg(0x000053A0, 0x000010FB);   /* 0x150053A0: CAM_GGM_G_GMT[40] */
    m_pIspDrv->writeReg(0x000053A4, 0x00000CFF);   /* 0x150053A4: CAM_GGM_G_GMT[41] */
    m_pIspDrv->writeReg(0x000053A8, 0x00001102);   /* 0x150053A8: CAM_GGM_G_GMT[42] */
    m_pIspDrv->writeReg(0x000053AC, 0x00000D06);   /* 0x150053AC: CAM_GGM_G_GMT[43] */
    m_pIspDrv->writeReg(0x000053B0, 0x00000D09);   /* 0x150053B0: CAM_GGM_G_GMT[44] */
    m_pIspDrv->writeReg(0x000053B4, 0x0000110C);   /* 0x150053B4: CAM_GGM_G_GMT[45] */
    m_pIspDrv->writeReg(0x000053B8, 0x00000D10);   /* 0x150053B8: CAM_GGM_G_GMT[46] */
    m_pIspDrv->writeReg(0x000053BC, 0x00000D13);   /* 0x150053BC: CAM_GGM_G_GMT[47] */
    m_pIspDrv->writeReg(0x000053C0, 0x00000D16);   /* 0x150053C0: CAM_GGM_G_GMT[48] */
    m_pIspDrv->writeReg(0x000053C4, 0x00000D19);   /* 0x150053C4: CAM_GGM_G_GMT[49] */
    m_pIspDrv->writeReg(0x000053C8, 0x00000D1C);   /* 0x150053C8: CAM_GGM_G_GMT[50] */
    m_pIspDrv->writeReg(0x000053CC, 0x0000111F);   /* 0x150053CC: CAM_GGM_G_GMT[51] */
    m_pIspDrv->writeReg(0x000053D0, 0x00000D23);   /* 0x150053D0: CAM_GGM_G_GMT[52] */
    m_pIspDrv->writeReg(0x000053D4, 0x00000D26);   /* 0x150053D4: CAM_GGM_G_GMT[53] */
    m_pIspDrv->writeReg(0x000053D8, 0x00000D29);   /* 0x150053D8: CAM_GGM_G_GMT[54] */
    m_pIspDrv->writeReg(0x000053DC, 0x00000D2C);   /* 0x150053DC: CAM_GGM_G_GMT[55] */
    m_pIspDrv->writeReg(0x000053E0, 0x00000D2F);   /* 0x150053E0: CAM_GGM_G_GMT[56] */
    m_pIspDrv->writeReg(0x000053E4, 0x00000D32);   /* 0x150053E4: CAM_GGM_G_GMT[57] */
    m_pIspDrv->writeReg(0x000053E8, 0x00000D35);   /* 0x150053E8: CAM_GGM_G_GMT[58] */
    m_pIspDrv->writeReg(0x000053EC, 0x00000938);   /* 0x150053EC: CAM_GGM_G_GMT[59] */
    m_pIspDrv->writeReg(0x000053F0, 0x00000D3A);   /* 0x150053F0: CAM_GGM_G_GMT[60] */
    m_pIspDrv->writeReg(0x000053F4, 0x00000D3D);   /* 0x150053F4: CAM_GGM_G_GMT[61] */
    m_pIspDrv->writeReg(0x000053F8, 0x00000D40);   /* 0x150053F8: CAM_GGM_G_GMT[62] */
    m_pIspDrv->writeReg(0x000053FC, 0x00000D43);   /* 0x150053FC: CAM_GGM_G_GMT[63] */
    m_pIspDrv->writeReg(0x00005400, 0x00001546);   /* 0x15005400: CAM_GGM_G_GMT[64] */
    m_pIspDrv->writeReg(0x00005404, 0x0000194B);   /* 0x15005404: CAM_GGM_G_GMT[65] */
    m_pIspDrv->writeReg(0x00005408, 0x00001551);   /* 0x15005408: CAM_GGM_G_GMT[66] */
    m_pIspDrv->writeReg(0x0000540C, 0x00001956);   /* 0x1500540C: CAM_GGM_G_GMT[67] */
    m_pIspDrv->writeReg(0x00005410, 0x0000155C);   /* 0x15005410: CAM_GGM_G_GMT[68] */
    m_pIspDrv->writeReg(0x00005414, 0x00001561);   /* 0x15005414: CAM_GGM_G_GMT[69] */
    m_pIspDrv->writeReg(0x00005418, 0x00001566);   /* 0x15005418: CAM_GGM_G_GMT[70] */
    m_pIspDrv->writeReg(0x0000541C, 0x0000156B);   /* 0x1500541C: CAM_GGM_G_GMT[71] */
    m_pIspDrv->writeReg(0x00005420, 0x00001570);   /* 0x15005420: CAM_GGM_G_GMT[72] */
    m_pIspDrv->writeReg(0x00005424, 0x00001575);   /* 0x15005424: CAM_GGM_G_GMT[73] */
    m_pIspDrv->writeReg(0x00005428, 0x0000157A);   /* 0x15005428: CAM_GGM_G_GMT[74] */
    m_pIspDrv->writeReg(0x0000542C, 0x0000157F);   /* 0x1500542C: CAM_GGM_G_GMT[75] */
    m_pIspDrv->writeReg(0x00005430, 0x00001584);   /* 0x15005430: CAM_GGM_G_GMT[76] */
    m_pIspDrv->writeReg(0x00005434, 0x00001589);   /* 0x15005434: CAM_GGM_G_GMT[77] */
    m_pIspDrv->writeReg(0x00005438, 0x0000158E);   /* 0x15005438: CAM_GGM_G_GMT[78] */
    m_pIspDrv->writeReg(0x0000543C, 0x00001193);   /* 0x1500543C: CAM_GGM_G_GMT[79] */
    m_pIspDrv->writeReg(0x00005440, 0x00001597);   /* 0x15005440: CAM_GGM_G_GMT[80] */
    m_pIspDrv->writeReg(0x00005444, 0x0000159C);   /* 0x15005444: CAM_GGM_G_GMT[81] */
    m_pIspDrv->writeReg(0x00005448, 0x000011A1);   /* 0x15005448: CAM_GGM_G_GMT[82] */
    m_pIspDrv->writeReg(0x0000544C, 0x000015A5);   /* 0x1500544C: CAM_GGM_G_GMT[83] */
    m_pIspDrv->writeReg(0x00005450, 0x000011AA);   /* 0x15005450: CAM_GGM_G_GMT[84] */
    m_pIspDrv->writeReg(0x00005454, 0x000015AE);   /* 0x15005454: CAM_GGM_G_GMT[85] */
    m_pIspDrv->writeReg(0x00005458, 0x000011B3);   /* 0x15005458: CAM_GGM_G_GMT[86] */
    m_pIspDrv->writeReg(0x0000545C, 0x000011B7);   /* 0x1500545C: CAM_GGM_G_GMT[87] */
    m_pIspDrv->writeReg(0x00005460, 0x000015BB);   /* 0x15005460: CAM_GGM_G_GMT[88] */
    m_pIspDrv->writeReg(0x00005464, 0x000011C0);   /* 0x15005464: CAM_GGM_G_GMT[89] */
    m_pIspDrv->writeReg(0x00005468, 0x000011C4);   /* 0x15005468: CAM_GGM_G_GMT[90] */
    m_pIspDrv->writeReg(0x0000546C, 0x000015C8);   /* 0x1500546C: CAM_GGM_G_GMT[91] */
    m_pIspDrv->writeReg(0x00005470, 0x000011CD);   /* 0x15005470: CAM_GGM_G_GMT[92] */
    m_pIspDrv->writeReg(0x00005474, 0x000011D1);   /* 0x15005474: CAM_GGM_G_GMT[93] */
    m_pIspDrv->writeReg(0x00005478, 0x000011D5);   /* 0x15005478: CAM_GGM_G_GMT[94] */
    m_pIspDrv->writeReg(0x0000547C, 0x000011D9);   /* 0x1500547C: CAM_GGM_G_GMT[95] */
    m_pIspDrv->writeReg(0x00005480, 0x000021DD);   /* 0x15005480: CAM_GGM_G_GMT[96] */
    m_pIspDrv->writeReg(0x00005484, 0x000021E5);   /* 0x15005484: CAM_GGM_G_GMT[97] */
    m_pIspDrv->writeReg(0x00005488, 0x000021ED);   /* 0x15005488: CAM_GGM_G_GMT[98] */
    m_pIspDrv->writeReg(0x0000548C, 0x000021F5);   /* 0x1500548C: CAM_GGM_G_GMT[99] */
    m_pIspDrv->writeReg(0x00005490, 0x000021FD);   /* 0x15005490: CAM_GGM_G_GMT[100] */
    m_pIspDrv->writeReg(0x00005494, 0x00002205);   /* 0x15005494: CAM_GGM_G_GMT[101] */
    m_pIspDrv->writeReg(0x00005498, 0x00001E0D);   /* 0x15005498: CAM_GGM_G_GMT[102] */
    m_pIspDrv->writeReg(0x0000549C, 0x00002214);   /* 0x1500549C: CAM_GGM_G_GMT[103] */
    m_pIspDrv->writeReg(0x000054A0, 0x00001E1C);   /* 0x150054A0: CAM_GGM_G_GMT[104] */
    m_pIspDrv->writeReg(0x000054A4, 0x00001E23);   /* 0x150054A4: CAM_GGM_G_GMT[105] */
    m_pIspDrv->writeReg(0x000054A8, 0x0000222A);   /* 0x150054A8: CAM_GGM_G_GMT[106] */
    m_pIspDrv->writeReg(0x000054AC, 0x00001E32);   /* 0x150054AC: CAM_GGM_G_GMT[107] */
    m_pIspDrv->writeReg(0x000054B0, 0x00001E39);   /* 0x150054B0: CAM_GGM_G_GMT[108] */
    m_pIspDrv->writeReg(0x000054B4, 0x00001E40);   /* 0x150054B4: CAM_GGM_G_GMT[109] */
    m_pIspDrv->writeReg(0x000054B8, 0x00001E47);   /* 0x150054B8: CAM_GGM_G_GMT[110] */
    m_pIspDrv->writeReg(0x000054BC, 0x00001E4E);   /* 0x150054BC: CAM_GGM_G_GMT[111] */
    m_pIspDrv->writeReg(0x000054C0, 0x00001A55);   /* 0x150054C0: CAM_GGM_G_GMT[112] */
    m_pIspDrv->writeReg(0x000054C4, 0x00001E5B);   /* 0x150054C4: CAM_GGM_G_GMT[113] */
    m_pIspDrv->writeReg(0x000054C8, 0x00001E62);   /* 0x150054C8: CAM_GGM_G_GMT[114] */
    m_pIspDrv->writeReg(0x000054CC, 0x00001A69);   /* 0x150054CC: CAM_GGM_G_GMT[115] */
    m_pIspDrv->writeReg(0x000054D0, 0x00001E6F);   /* 0x150054D0: CAM_GGM_G_GMT[116] */
    m_pIspDrv->writeReg(0x000054D4, 0x00001E76);   /* 0x150054D4: CAM_GGM_G_GMT[117] */
    m_pIspDrv->writeReg(0x000054D8, 0x00001A7D);   /* 0x150054D8: CAM_GGM_G_GMT[118] */
    m_pIspDrv->writeReg(0x000054DC, 0x00001A83);   /* 0x150054DC: CAM_GGM_G_GMT[119] */
    m_pIspDrv->writeReg(0x000054E0, 0x00001E89);   /* 0x150054E0: CAM_GGM_G_GMT[120] */
    m_pIspDrv->writeReg(0x000054E4, 0x00001A90);   /* 0x150054E4: CAM_GGM_G_GMT[121] */
    m_pIspDrv->writeReg(0x000054E8, 0x00001A96);   /* 0x150054E8: CAM_GGM_G_GMT[122] */
    m_pIspDrv->writeReg(0x000054EC, 0x00001E9C);   /* 0x150054EC: CAM_GGM_G_GMT[123] */
    m_pIspDrv->writeReg(0x000054F0, 0x00001AA3);   /* 0x150054F0: CAM_GGM_G_GMT[124] */
    m_pIspDrv->writeReg(0x000054F4, 0x00001AA9);   /* 0x150054F4: CAM_GGM_G_GMT[125] */
    m_pIspDrv->writeReg(0x000054F8, 0x00001AAF);   /* 0x150054F8: CAM_GGM_G_GMT[126] */
    m_pIspDrv->writeReg(0x000054FC, 0x00001AB5);   /* 0x150054FC: CAM_GGM_G_GMT[127] */
    m_pIspDrv->writeReg(0x00005500, 0x000062BB);   /* 0x15005500: CAM_GGM_G_GMT[128] */
    m_pIspDrv->writeReg(0x00005504, 0x00005ED3);   /* 0x15005504: CAM_GGM_G_GMT[129] */
    m_pIspDrv->writeReg(0x00005508, 0x00005AEA);   /* 0x15005508: CAM_GGM_G_GMT[130] */
    m_pIspDrv->writeReg(0x0000550C, 0x00005B00);   /* 0x1500550C: CAM_GGM_G_GMT[131] */
    m_pIspDrv->writeReg(0x00005510, 0x00005B16);   /* 0x15005510: CAM_GGM_G_GMT[132] */
    m_pIspDrv->writeReg(0x00005514, 0x0000572C);   /* 0x15005514: CAM_GGM_G_GMT[133] */
    m_pIspDrv->writeReg(0x00005518, 0x00005341);   /* 0x15005518: CAM_GGM_G_GMT[134] */
    m_pIspDrv->writeReg(0x0000551C, 0x00005755);   /* 0x1500551C: CAM_GGM_G_GMT[135] */
    m_pIspDrv->writeReg(0x00005520, 0x00004F6A);   /* 0x15005520: CAM_GGM_G_GMT[136] */
    m_pIspDrv->writeReg(0x00005524, 0x0000537D);   /* 0x15005524: CAM_GGM_G_GMT[137] */
    m_pIspDrv->writeReg(0x00005528, 0x00004F91);   /* 0x15005528: CAM_GGM_G_GMT[138] */
    m_pIspDrv->writeReg(0x0000552C, 0x00004FA4);   /* 0x1500552C: CAM_GGM_G_GMT[139] */
    m_pIspDrv->writeReg(0x00005530, 0x00004FB7);   /* 0x15005530: CAM_GGM_G_GMT[140] */
    m_pIspDrv->writeReg(0x00005534, 0x00004BCA);   /* 0x15005534: CAM_GGM_G_GMT[141] */
    m_pIspDrv->writeReg(0x00005538, 0x00004BDC);   /* 0x15005538: CAM_GGM_G_GMT[142] */
    m_pIspDrv->writeReg(0x0000553C, 0x000047EE);   /* 0x1500553C: CAM_GGM_G_GMT[143] */
    m_pIspDrv->writeReg(0x00005600, 0x00000001);   /* 0x15005600: CAM_GGM_CTRL */

    return 0;
}
//
int
IspDrv_B::
cam_isp_c02_cfg(int ysize,int yofst,int mode) {
    ISP_FUNC_DBG("ysize(%d),yofst(%d),mode(%d)",ysize,yofst,mode);
    //YUV422
        //should be BYPASS mode
    //YUV420
        //should file "ysize" parameter

    /* 0x15004950: CAM_C02_CON */
    if(m_pIspDrv->IsReadOnlyMode())
    {
        //ISP_IOCTL_WRITE_BITS(m_pIspDrv, m_pIspDrv->getRegAddrMap(), CAM_C02_CON, C02_OUTPUT_YSIZE, ysize);
        //ISP_IOCTL_WRITE_BITS(m_pIspDrv, m_pIspDrv->getRegAddrMap(), CAM_C02_CON, C02_OFFSET_Y, yofst);
        //ISP_IOCTL_WRITE_BITS(m_pIspDrv, m_pIspDrv->getRegAddrMap(), CAM_C02_CON, C02_MODE, mode); // 0:420->422, 1:bypass
    }
    else
    {
        //ISP_WRITE_BITS(m_pIspReg, CAM_C02_CON, C02_OUTPUT_YSIZE, ysize);
        //ISP_WRITE_BITS(m_pIspReg, CAM_C02_CON, C02_OFFSET_Y, yofst);
        //ISP_WRITE_BITS(m_pIspReg, CAM_C02_CON, C02_MODE, mode); // 0:420->422, 1:bypass
    }

    return 0;
}
//
int
IspDrv_B::
cam_isp_c24_cfg(void) {
    ISP_FUNC_DBG("cam_isp_c24_cfg ");
    return 0;
}
//
int
IspDrv_B::
cam_isp_g2c_cfg(void) {
    ISP_FUNC_DBG("cam_isp_g2c_cfg ");
    m_pIspDrv->writeReg(0x00004A00, 0x012D0099);	/* 0x15004A00: CAM_G2C_CONV_0A */
    m_pIspDrv->writeReg(0x00004A04, 0x0000003A);	/* 0x15004A04: CAM_G2C_CONV_0B */
    m_pIspDrv->writeReg(0x00004A08, 0x075607AA);	/* 0x15004A08: CAM_G2C_CONV_1A */
    m_pIspDrv->writeReg(0x00004A0C, 0x00000100);	/* 0x15004A0C: CAM_G2C_CONV_1B */
    m_pIspDrv->writeReg(0x00004A10, 0x072A0100);	/* 0x15004A10: CAM_G2C_CONV_2A */
    m_pIspDrv->writeReg(0x00004A14, 0x000007D6);	/* 0x15004A14: CAM_G2C_CONV_2B */
    return 0;
}
//
int
IspDrv_B::
cam_isp_c42_cfg(void) {
    ISP_FUNC_DBG("cam_isp_c42_cfg m_pIspDrv(0x%x)\n",m_pIspDrv);
    m_pIspDrv->writeReg(0x00004A1C, 0x00000000);	/* 0x15004A1C: CAM_C42_CON */
    return 0;
}
//
int
IspDrv_B::
cam_isp_mfb_cfg(void) {
    ISP_FUNC_DBG("cam_isp_mfb_cfg ");
    m_pIspDrv->writeReg(0x00004960, 0x00000002);	/* 0x15004960: CAM_MFB_CON */
    return 0;
}


/*/////////////////////////////////////////////////////////////////////////////
    IspDrvShellImp
/////////////////////////////////////////////////////////////////////////////*/
class IspDrvShellImp:public IspDrvShell
{
    public:
        static IspDrvShell* getInstance(NSImageio::NSIspio::EScenarioID   eScenarioID = NSImageio::NSIspio::eScenarioID_VR);
        virtual void        destroyInstance(void);
        virtual MBOOL       init(void);
        virtual MBOOL       uninit(void);
    public://driver object operation
        inline virtual IspDrv*      getPhyIspDrv(){return m_pPhyIspDrv_bak; }
        inline virtual isp_reg_t*   getPhyIspReg(){return m_pPhyIspReg_bak; }

    public://phy<->virt ISP switch
        inline MBOOL       ispDrvSwitch2Virtual(MINT32 cq);
        //inline MBOOL       ispDrvSwitch2Phy();
    public://commandQ operation
        virtual MBOOL       cam_cq_cfg(MINT32 cq);
        virtual MBOOL       cqAddModule(ISP_DRV_CQ_ENUM cq, CAM_MODULE_ENUM moduleId);
        virtual MBOOL       cqDelModule(ISP_DRV_CQ_ENUM cq, CAM_MODULE_ENUM moduleId);
        virtual MBOOL       setCQTriggerMode(MINT32 cq, MINT32 mode, MINT32 trig_src);
    protected:
        volatile MINT32 mInitCount;
        volatile bool m_fgIsGdmaMode;
        MTKM4UDrv*  mpM4UDrv;
};
//
IspDrvShell* IspDrvShell::createInstance(NSImageio::NSIspio::EScenarioID   eScenarioID)
{
    DBG_LOG_CONFIG(imageio, function);
//    ISP_FUNC_DBG("");
    ISP_FUNC_INF("ScenarioID: %d.", eScenarioID);
    return IspDrvShellImp::getInstance(eScenarioID);
}
//
IspDrvShell*
IspDrvShellImp::
getInstance(NSImageio::NSIspio::EScenarioID eScenarioID)
{
//    ISP_FUNC_DBG("");
    ISP_FUNC_INF("ScenarioID: %d.", eScenarioID);
    static IspDrvShellImp singleton;

    switch (eScenarioID)
    {
         case NSImageio::NSIspio::eScenarioID_GDMA:
             singleton.m_fgIsGdmaMode = MTRUE;
             break;

         case NSImageio::NSIspio::eScenarioID_CONFIG_FMT:
             // Don't change singleton.m_fgIsGdmaMode, i.e. use old value.
             break;

         default:
             singleton.m_fgIsGdmaMode = MFALSE;
    }

    return &singleton;
}
//
void
IspDrvShellImp::
destroyInstance(void)
{
}

//
stISP_BUF_LIST ISP_BUF_CTRL::m_hwbufL[_rt_dma_max_];
stISP_BUF_LIST ISP_BUF_CTRL::m_swbufL[_rt_dma_max_];
//
MBOOL IspDrvShellImp::init(void)
{
int ret = 0;

    Mutex::Autolock lock(mLock);

    ISP_FUNC_INF("mInitCount(%d) GdmaMode: %d.", mInitCount, m_fgIsGdmaMode);
    //
    if(mInitCount > 0)
    {
        android_atomic_inc(&mInitCount);
        return MTRUE;
    }

    if (!m_fgIsGdmaMode)    // Normal Mode (i.e. not GDMA).
    {
        /*============================================
            imem driver
            =============================================*/
        m_pIMemDrv = IMemDrv::createInstance();
        ISP_FUNC_DBG("[m_pIMemDrv]:0x%08x ",m_pIMemDrv);
        if ( NULL == m_pIMemDrv ) {
            ISP_FUNC_ERR("IMemDrv::createInstance fail.");
            return -1;
        }

        // Need to init IMem even in GDMA mode, because M4U port (IMGI, DISPO, ....) config and enable is in it.
        m_pIMemDrv->init();

            //m_pIMemDrv->allocPhyMem(bufInfo.base_vAddr,bufInfo.memID,bufInfo.size,&bufInfo.base_pAddr);
    }
    else    // GDMA mode.
    {
        LOCAL_PROFILING_LOG_AUTO_START(Event_IspDrvShellImp_Init);
        
#if 0 //TODO : MT6582 need modified
        mpM4UDrv = new MTKM4UDrv();
        
        //LOCAL_PROFILING_LOG_PRINT("new MTKM4UDrv()");

        mpM4UDrv->m4u_enable_m4u_func(M4U_CLNTMOD_VIP);   

        //LOCAL_PROFILING_LOG_PRINT("m4u_enable_m4u_func(M4U_CLNTMOD_VIP)");

        mpM4UDrv->m4u_enable_m4u_func(M4U_CLNTMOD_DISP);  

        //LOCAL_PROFILING_LOG_PRINT("m4u_enable_m4u_func(M4U_CLNTMOD_DISP)");

        mpM4UDrv->m4u_enable_m4u_func(M4U_CLNTMOD_IMG);

        //LOCAL_PROFILING_LOG_PRINT("m4u_enable_m4u_func(M4U_CLNTMOD_IMG)");

        M4U_PORT_STRUCT port;
        port.Virtuality = 1;
        port.Security = 0;
        port.domain = 3;
        port.Distance = 1;
        port.Direction = 0; //M4U_DMA_READ_WRITE

        port.ePortID = M4U_PORT_VIPI;
        ret = mpM4UDrv->m4u_config_port(&port);
//                                                LOCAL_PROFILING_LOG_PRINT("m4u_config_port(VIPI)");

        port.ePortID = M4U_PORT_VIP2I;
        ret = mpM4UDrv->m4u_config_port(&port);
//                                                LOCAL_PROFILING_LOG_PRINT("m4u_config_port(VIP2I)");

        port.ePortID = M4U_PORT_DISPO;
        ret = mpM4UDrv->m4u_config_port(&port);
//                                                LOCAL_PROFILING_LOG_PRINT("m4u_config_port(DISPO)");

        port.ePortID = M4U_PORT_DISPCO;
        ret = mpM4UDrv->m4u_config_port(&port);
//                                                LOCAL_PROFILING_LOG_PRINT("m4u_config_port(DISPCO)");

        port.ePortID = M4U_PORT_DISPVO;
        ret = mpM4UDrv->m4u_config_port(&port);
//                                                LOCAL_PROFILING_LOG_PRINT("m4u_config_port(DISPVO)");

        port.ePortID = M4U_PORT_IMGI;
        ret = mpM4UDrv->m4u_config_port(&port);
//                                                LOCAL_PROFILING_LOG_PRINT("m4u_config_port(IMGI)");

        port.ePortID = M4U_PORT_IMGCI;
        ret = mpM4UDrv->m4u_config_port(&port);
                                                LOCAL_PROFILING_LOG_PRINT("m4u_config_port(IMGCI)");

#endif
    }

    /*============================================
        isp driver
        =============================================*/
    m_pIspDrv = IspDrv::createInstance(m_fgIsGdmaMode);
    ISP_FUNC_INF("[m_pIspDrv]:0x%08x ",m_pIspDrv);
    if (!m_pIspDrv) {
        ISP_FUNC_ERR("IspDrv::createInstance fail ");
        return ret;
    }
    //
    ret = m_pIspDrv->init();
    if (ret < 0) {
        ISP_FUNC_ERR("pIspDrv->init() fail ");
        return ret;
    }
    //
    if(m_pIspDrv->IsReadOnlyMode()) {
        m_pIspReg = (isp_reg_t*)m_pIspDrv->getRegAddrMap();
    } else {
        m_pIspReg = (isp_reg_t*)m_pIspDrv->getRegAddr();
    }
    ISP_FUNC_DBG("[m_pIspReg]:0x%08x ",m_pIspReg);
    if ( NULL == m_pIspReg ) {
        ISP_FUNC_ERR("getRegAddr fail ");
        return -1;
    }
    //backup
    m_pPhyIspDrv_bak = m_pIspDrv;
    m_pPhyIspReg_bak = m_pIspReg;


    /*============================================
    virtual isp driver for CQ
    =============================================*/
#if 0 //js_test remove below later
    m_pVirtIspDrv = IspDrv::createInstance(ISP_DRV_INSTANCE_CQ);
    ISP_FUNC_DBG("[m_pVirtIspDrv]:0x%08x ",m_pVirtIspDrv);
    if (!m_pVirtIspDrv) {
        ISP_FUNC_ERR("g_pVirtIspDrv::createInstance fail ");
        return ret;
    }
    //
    ret = m_pVirtIspDrv->init();
    if (ret < 0) {
        ISP_FUNC_ERR("g_pVirtIspDrv->init() fail ");
        return ret;
    }
#endif
    //

    if (!m_fgIsGdmaMode)    // Normal Mode (i.e. not GDMA).
    {
        for (MINT32 cq = 0; cq<ISP_DRV_CQ_NUM; cq++ ) {
            m_pVirtIspDrv[cq] = NULL;
            m_pVirtIspReg[cq] = NULL;
            //
            m_pVirtIspDrv[cq] = m_pIspDrv->getCQInstance((ISP_DRV_CQ_ENUM)cq);
            ISP_FUNC_DBG("[m_pVirtIspDrv[cq%d]]:0x%08x ",cq,m_pVirtIspDrv[cq]);
            if (!m_pVirtIspDrv[cq]) {
                ISP_FUNC_ERR("g_pVirtIspDrv::createInstance fail ");
                return ret;
            }
            //
            ret = m_pVirtIspDrv[cq]->init();
            if (ret < 0) {
                ISP_FUNC_ERR("g_pVirtIspDrv[%d]->init() fail ",cq);
                return ret;
            }
            //
            m_pVirtIspReg[cq] = (isp_reg_t*)m_pVirtIspDrv[cq]->getRegAddr();
            ISP_FUNC_DBG("[m_pVirtIspReg[cq%d]]:0x%08x ",cq,m_pVirtIspReg[cq]);
            if ( NULL == m_pVirtIspReg[cq] ) {
                ISP_FUNC_ERR("getRegAddr fail ");
                return -1;
            }
            //descriptor buffer
            ISP_FUNC_DBG("[mpIspCQDescriptorPhy[cq%d]]:0x%08x ",cq,m_pIspDrv->getCQDescBufPhyAddr((ISP_DRV_CQ_ENUM)cq));

        }
    }

    /*============================================
        cdp driver
    =============================================*/
    m_pCdpDrv = CdpDrv::CreateInstance(m_fgIsGdmaMode);
    ISP_FUNC_DBG("[m_pCdpDrv]:0x%08x ",m_pCdpDrv);
    if ( NULL == m_pCdpDrv ) {
        ISP_FUNC_ERR("CdpDrv::CreateInstance cfail ");
        return -1;
    }
    m_pCdpDrv->Init();

    if (!m_fgIsGdmaMode)    // Normal Mode (i.e. not GDMA).
    {
        m_pCdpDrv->SetIspReg(m_pVirtIspReg[ISP_DRV_CQ01]);  // set a point for cdp CheckReady() of CalAlgoAndCStep()
    }
    else    // GDMA mode.
    {
        m_pCdpDrv->SetPhyIspDrv(m_pPhyIspDrv_bak);
    }


    if (!m_fgIsGdmaMode)    // Normal Mode (i.e. not GDMA).
    {
        /*============================================
            tdri driver
        =============================================*/
        m_pTpipeDrv = TpipeDrv::createInstance();
        ISP_FUNC_DBG("[m_pTdriDrv]:0x%08x ",m_pTpipeDrv);
        if ( NULL == m_pTpipeDrv ) {
            ISP_FUNC_ERR("TpipeDrv::createInstance cfail ");
            return -1;
        }
        m_pTpipeDrv->init();
        /*============================================
            buffer control
            =============================================*/
        //clear all once
        ISP_FUNC_DBG("clear buffer control ");
        for ( int i =0; i < _rt_dma_max_ ;i++ ) {
            ISP_BUF_CTRL::m_hwbufL[i].bufInfoList.clear();
            ISP_BUF_CTRL::m_swbufL[i].bufInfoList.clear();
        }
        //
        if(!m_pIspDrv->IsReadOnlyMode()) {
            if ( 0 != this->m_pIspDrv->m_pRTBufTbl ) {
                memset(this->m_pIspDrv->m_pRTBufTbl,0x00,sizeof(ISP_RT_BUF_STRUCT));
            }
            else {
                ISP_FUNC_ERR("NULL m_pRTBufTbl");
            }
        }
    }

    /*============================================
        ispEventThread driver
        =============================================*/
    m_pIspEventThread = IspEventThread::createInstance(m_pPhyIspDrv_bak);
    ISP_FUNC_INF("[m_pIspEventThread]:0x%08x ",m_pIspEventThread);
    if ( NULL == m_pIspEventThread ) {
        ISP_FUNC_ERR("IspEventThread::createInstance fail ");
        return -1;
    }
    m_pIspEventThread->init();
    //
    ISP_FUNC_INF("X");

    android_atomic_inc(&mInitCount);

    return MTRUE;

}
//
MBOOL
IspDrvShellImp::
uninit(void)
{
int ret = 0;

    Mutex::Autolock lock(mLock);

    ISP_FUNC_DBG("mInitCount(%d)",mInitCount);
    //
    if(mInitCount <= 0)
    {
        // No more users
        return MTRUE;
    }
    // More than one user
    android_atomic_dec(&mInitCount);
    //
    if(mInitCount > 0)
    {
        return MTRUE;
    }

    /*============================================
    cdp driver release
    =============================================*/
    //CDRZ
    m_pCdpDrv->CDRZ_Unconfig( );
    //CURZ
    m_pCdpDrv->CURZ_Unconfig( ) ;
    //PRZ
    m_pCdpDrv->PRZ_Unconfig( ) ;

//js_test move to _disable.
    //VIDO
    //m_pCdpDrv->VIDO_Unconfig( ) ;
    //DISPO
    //m_pCdpDrv->DISPO_Unconfig();
    //
    m_pCdpDrv->Uninit();
    m_pCdpDrv->DestroyInstance();
    m_pCdpDrv = NULL;


    if (!m_fgIsGdmaMode)    // Normal Mode (i.e. not GDMA).
    {
        //TDRI
        m_pTpipeDrv->uninit();
        m_pTpipeDrv->destroyInstance();
        m_pTpipeDrv = NULL;
    }

    if (!m_fgIsGdmaMode)    // Normal Mode (i.e. not GDMA).
    {
        //IMEM
        m_pIMemDrv->uninit();
        m_pIMemDrv->destroyInstance();
        m_pIMemDrv = NULL;
    }
    else    // GDMA mode.
    {

#if 0   // TODO : MT6582 need modification

        LOCAL_PROFILING_LOG_AUTO_START(Event_IspDrvShellImp_Uninit);
        
        M4U_PORT_STRUCT port;
        port.Virtuality = 0;
        port.Security = 0;
        port.domain = 3;
        port.Distance = 1;
        port.Direction = 0; //M4U_DMA_READ_WRITE

//        port.ePortID = M4U_PORT_VIPI;
//        ret = mpM4UDrv->m4u_config_port(&port);
//                                               LOCAL_PROFILING_LOG_PRINT("m4u_config_port(VIPI)");

//        port.ePortID = M4U_PORT_VIP2I;
//        ret = mpM4UDrv->m4u_config_port(&port);
//                                               LOCAL_PROFILING_LOG_PRINT("m4u_config_port(VIP2I)");

//        port.ePortID = M4U_PORT_DISPO;
//        ret = mpM4UDrv->m4u_config_port(&port);
//                                               LOCAL_PROFILING_LOG_PRINT("m4u_config_port(DISPO)");

//        port.ePortID = M4U_PORT_DISPCO;
//        ret = mpM4UDrv->m4u_config_port(&port);
//                                               LOCAL_PROFILING_LOG_PRINT("m4u_config_port(DISPCO)");

//        port.ePortID = M4U_PORT_DISPVO;
//        ret = mpM4UDrv->m4u_config_port(&port);
//                                               LOCAL_PROFILING_LOG_PRINT("m4u_config_port(DISPVO)");

//        port.ePortID = M4U_PORT_IMGI;
//        ret = mpM4UDrv->m4u_config_port(&port);
//                                               LOCAL_PROFILING_LOG_PRINT("m4u_config_port(IMGI)");

//        port.ePortID = M4U_PORT_IMGCI;
//        ret = mpM4UDrv->m4u_config_port(&port);
//                                               LOCAL_PROFILING_LOG_PRINT("m4u_config_port(IMGCI)");

        delete mpM4UDrv;
        mpM4UDrv = NULL;
        LOCAL_PROFILING_LOG_PRINT("delete mpM4UDrv");

#endif
    }

    //ispEventThread
#if 1
    m_pIspEventThread->uninit();
    m_pIspEventThread = NULL;
#endif
    //
    /*============================================
    isp driver
    =============================================*/
    ISP_FUNC_DBG("m_pIspDrv(0x%x) m_pPhyIspDrv_bak(0x%x)\n",m_pIspDrv,m_pPhyIspDrv_bak);
    if (m_pPhyIspDrv_bak) {
        ret = m_pPhyIspDrv_bak->uninit();
        if (ret < 0) {
            ISP_FUNC_ERR("pIspDrv->uninit() fail ");
            return ret;
        }
        m_pPhyIspDrv_bak->destroyInstance();
    }
    else {
        ISP_FUNC_DBG("ispDrvInit,No isp driver object ");
    }

    /*============================================
    virtual isp driver for CQ
    =============================================*/
    if (!m_fgIsGdmaMode)    // Normal Mode (i.e. not GDMA).
    {
        for (int i = 0; i<ISP_DRV_CQ_NUM; i++ ) {
            if (m_pVirtIspDrv[i]) {
                ret = m_pVirtIspDrv[i]->uninit();
                if (ret < 0) {
                    ISP_FUNC_ERR("g_pVirtIspDrv[%d]->uninit() fail ",i);
                    return ret;
                }
            }
            else {
                ISP_FUNC_DBG("g_pVirtIspDrv->uninit(?,No isp driver object ");
            }
            m_pVirtIspDrv[i] = NULL;
            m_pVirtIspReg[i] = NULL;
        }
    }

    //
    m_pIspDrv  = NULL;
    m_pIspReg  = NULL; //mmap
    m_pPhyIspDrv_bak  = NULL;
    m_pPhyIspReg_bak  = NULL;

    ISP_FUNC_INF("mInitCount(%d)",mInitCount);
    return MTRUE;
}


//
MBOOL
IspDrvShellImp::
ispDrvSwitch2Virtual(MINT32 cq)
{
int ret = MTRUE;

    ISP_FUNC_DBG("[0x%x]",cq);
    //set
    m_pIspDrv = m_pVirtIspDrv[cq];
    //
    m_pIspReg = m_pVirtIspReg[cq];
    //
    m_pCdpDrv->SetIspReg(m_pVirtIspReg[cq]);
    //
    ISP_FUNC_DBG("m_pIspReg:0x%08X ",m_pVirtIspReg[cq]);

    return MTRUE;
}//
//
/*
MBOOL
IspDrvShellImp::
ispDrvSwitch2Phy()
{
    ISP_FUNC_DBG(":E");
    m_pIspDrv = m_pPhyIspDrv_bak;
    m_pIspReg = m_pPhyIspReg_bak;
    m_pCdpDrv->SetIspReg(m_pPhyIspReg_bak);
    ISP_FUNC_DBG(":D");
    return MTRUE;
}//
*/
//
MBOOL
IspDrvShellImp::
cam_cq_cfg(
MINT32 cq
) {
    ISP_FUNC_DBG(":E cq(%d) m_pIspReg(0x%x) m_pPhyIspReg_bak(0x%x) m_pIspDrv(0x%x) m_pPhyIspDrv_bak(0x%x)\n",cq,m_pIspReg,m_pPhyIspReg_bak,m_pIspDrv,m_pPhyIspDrv_bak);

    switch(cq) {
        case CAM_ISP_CQ0:
            if(m_pPhyIspDrv_bak->IsReadOnlyMode())
            {
                ISP_IOCTL_WRITE_REG(m_pPhyIspDrv_bak, m_pPhyIspDrv_bak->getRegAddrMap(),CAM_CTL_CQ0_BASEADDR, (unsigned int)m_pPhyIspDrv_bak->getCQDescBufPhyAddr((ISP_DRV_CQ_ENUM)cq));
            }
            else
            {
                ISP_WRITE_REG(m_pPhyIspReg_bak,CAM_CTL_CQ0_BASEADDR, (unsigned int)m_pPhyIspDrv_bak->getCQDescBufPhyAddr((ISP_DRV_CQ_ENUM)cq));
            }
            //
            //m_pIspDrv->setCQTriggerMode((ISP_DRV_CQ_ENUM)cq,CQ_SINGLE_IMMEDIATE_TRIGGER,CQ_TRIG_BY_START);
            break;
        case CAM_ISP_CQ0B:
            if(m_pPhyIspDrv_bak->IsReadOnlyMode())
            {
                ISP_IOCTL_WRITE_REG(m_pPhyIspDrv_bak,m_pPhyIspDrv_bak->getRegAddrMap(),CAM_CTL_CQ0B_BASEADDR, (unsigned int)m_pPhyIspDrv_bak->getCQDescBufPhyAddr((ISP_DRV_CQ_ENUM)cq));
            }
            else
            {
                ISP_WRITE_REG(m_pPhyIspReg_bak,CAM_CTL_CQ0B_BASEADDR, (unsigned int)m_pPhyIspDrv_bak->getCQDescBufPhyAddr((ISP_DRV_CQ_ENUM)cq));
            }
            //
            //m_pIspDrv->setCQTriggerMode((ISP_DRV_CQ_ENUM)cq,CQ_SINGLE_EVENT_TRIGGER,CQ_TRIG_BY_PASS1_DONE);
            break;
        case CAM_ISP_CQ0C:
            if(m_pPhyIspDrv_bak->IsReadOnlyMode())
            {
                ISP_IOCTL_WRITE_REG(m_pPhyIspDrv_bak,m_pPhyIspDrv_bak->getRegAddrMap(),CAM_CTL_CQ0C_BASEADDR, (unsigned int)m_pPhyIspDrv_bak->getCQDescBufPhyAddr((ISP_DRV_CQ_ENUM)cq));
            }
            else
            {
                ISP_WRITE_REG(m_pPhyIspReg_bak,CAM_CTL_CQ0C_BASEADDR, (unsigned int)m_pPhyIspDrv_bak->getCQDescBufPhyAddr((ISP_DRV_CQ_ENUM)cq));
            }
            //
            //m_pIspDrv->setCQTriggerMode((ISP_DRV_CQ_ENUM)cq,CQ_SINGLE_EVENT_TRIGGER,CQ_TRIG_BY_PASS1_DONE);
            break;
        case CAM_ISP_CQ1:
            if(m_pPhyIspDrv_bak->IsReadOnlyMode())
            {
                ISP_IOCTL_WRITE_REG(m_pPhyIspDrv_bak,m_pPhyIspDrv_bak->getRegAddrMap(),CAM_CTL_CQ1_BASEADDR, (unsigned int)m_pPhyIspDrv_bak->getCQDescBufPhyAddr((ISP_DRV_CQ_ENUM)cq));
            }
            else
            {
                ISP_WRITE_REG(m_pPhyIspReg_bak,CAM_CTL_CQ1_BASEADDR, (unsigned int)m_pPhyIspDrv_bak->getCQDescBufPhyAddr((ISP_DRV_CQ_ENUM)cq));
            }
            break;
        case CAM_ISP_CQ2:
            if(m_pPhyIspDrv_bak->IsReadOnlyMode())
            {
                ISP_IOCTL_WRITE_REG(m_pPhyIspDrv_bak,m_pPhyIspDrv_bak->getRegAddrMap(),CAM_CTL_CQ2_BASEADDR, (unsigned int)m_pPhyIspDrv_bak->getCQDescBufPhyAddr((ISP_DRV_CQ_ENUM)cq));
            }
            else
            {
                ISP_WRITE_REG(m_pPhyIspReg_bak,CAM_CTL_CQ2_BASEADDR, (unsigned int)m_pPhyIspDrv_bak->getCQDescBufPhyAddr((ISP_DRV_CQ_ENUM)cq));
            }
            break;
        case CAM_ISP_CQ3:
            if(m_pPhyIspDrv_bak->IsReadOnlyMode())
            {
                ISP_IOCTL_WRITE_REG(m_pPhyIspDrv_bak,m_pPhyIspDrv_bak->getRegAddrMap(),CAM_CTL_CQ3_BASEADDR, (unsigned int)m_pPhyIspDrv_bak->getCQDescBufPhyAddr((ISP_DRV_CQ_ENUM)cq));
            }
            else
            {
                ISP_WRITE_REG(m_pPhyIspReg_bak,CAM_CTL_CQ3_BASEADDR, (unsigned int)m_pPhyIspDrv_bak->getCQDescBufPhyAddr((ISP_DRV_CQ_ENUM)cq));
            }
            break;
        default:
            ISP_FUNC_WRN("Warning:cq index ");
            break;
    }
    return MTRUE;
}
//
MBOOL
IspDrvShellImp::
cqAddModule(ISP_DRV_CQ_ENUM cq, CAM_MODULE_ENUM moduleId)
{
    ISP_FUNC_DBG(":E m_pIspDrv(0x%x) m_pPhyIspDrv(0x%x)\n",m_pIspDrv,m_pPhyIspDrv_bak);
    //
    m_pIspDrv->cqAddModule(cq, moduleId);
    ISP_FUNC_DBG(":X ");
    return MTRUE;
}


//
MBOOL
IspDrvShellImp::
cqDelModule(ISP_DRV_CQ_ENUM cq, CAM_MODULE_ENUM moduleId)
{
    ISP_FUNC_DBG(":E m_pIspDrv(0x%x) m_pPhyIspDrv(0x%x)\n",m_pIspDrv,m_pPhyIspDrv_bak);
    //
    m_pIspDrv->cqDelModule( cq, moduleId);
    ISP_FUNC_DBG(":X ");
    return MTRUE;
}
//
MBOOL
IspDrvShellImp::
setCQTriggerMode(MINT32 cq, MINT32 mode, MINT32 trig_src)
{
    ISP_FUNC_DBG(":E m_pIspDrv(0x%x) m_pPhyIspDrv(0x%x)\n",m_pIspDrv,m_pPhyIspDrv_bak);
    //
    m_pPhyIspDrv_bak->setCQTriggerMode((ISP_DRV_CQ_ENUM) cq,
                                (ISP_DRV_CQ_TRIGGER_MODE_ENUM) mode,
                                (ISP_DRV_CQ_TRIGGER_SOURCE_ENUM) trig_src);
    ISP_FUNC_DBG(":X ");

    return MTRUE;
}

/*/////////////////////////////////////////////////////////////////////////////
    IspFunction_B
  /////////////////////////////////////////////////////////////////////////////*/
unsigned long   IspFunction_B::m_Isp_Top_Reg_Base_Addr = MT6589_ISP_TOP_BASE; /*Use physical base address as default address*/
IspDrvShell*    IspFunction_B::m_pIspDrvShell = NULL;
IspDrv*         IspFunction_B::m_pIspDrv = NULL;
isp_reg_t*      IspFunction_B::m_pIspReg = NULL;
isp_reg_t*      IspFunction_B::m_pPhyIspReg = NULL;
IspDrv*         IspFunction_B::m_pPhyIspDrv = NULL;

unsigned long IspFunction_B::dec_Id( void )//Decimal ID number
{
    unsigned long dec_Id = 0;
    unsigned long id = this->id();  //virtual

    while( id )
    {
        dec_Id++;
        id = id >> 1;
    }

    return (dec_Id-1);
}

int IspFunction_B::config( void )
{
    int retval;
    //ISP_FUNC_DBG("<%2d> config Pa=0x%x Va=0x%x",(int)this->dec_Id(), (unsigned int)this->reg_Base_Addr_Phy(), (unsigned int)reg_Base_Addr() );
    retval = this->_config();       //virtual
    if( retval != 0 )   return retval;
    return 0;
}

int IspFunction_B::enable( void* pParam )
{
    int retval;

    //ISP_FUNC_DBG("<%2d> enable",(int)this->dec_Id());
    retval = this->_enable(pParam);       //virtual
    if( retval != 0 )   return retval;
    return 0;
}





int IspFunction_B::disable( void )
{
    int retval;
    //ISP_FUNC_DBG("<%2d> disable",(int)this->dec_Id());
    retval = this->_disable();       //virtual
    if( retval != 0 )   return retval;
   
    return 0;
}

int IspFunction_B::write2CQ(int cq)
{
int retval;
    //ISP_FUNC_DBG("<%s> write2CQ",this->name_Str());
    retval = this->_write2CQ(cq);       //virtual
    if( retval != 0 ){
        ISP_FUNC_ERR("this->_write2CQ fail ");
    }
    return retval;

}

int IspFunction_B::setZoom( void )
{
    int retval;
    //ISP_FUNC_DBG("<%2d> config Pa=0x%x Va=0x%x",(int)this->dec_Id(), (unsigned int)this->reg_Base_Addr_Phy(), (unsigned int)reg_Base_Addr() );
    retval = this->_setZoom();       //virtual
    if( retval != 0 )   return retval;
    return 0;
}

MBOOL IspFunction_B::ispDrvSwitch2Virtual(MINT32 cq)
{
Mutex::Autolock lock(mLock);

    ISP_FUNC_DBG("E cq(%d)\n",cq);

    this->m_pIspDrvShell->ispDrvSwitch2Virtual(cq);
    m_pIspDrv = this->m_pIspDrvShell->m_pIspDrv;
    m_pIspReg = this->m_pIspDrvShell->m_pIspReg;

    ISP_FUNC_DBG("X\n");
    return MTRUE;
}

/*
MBOOL
IspFunction_B::
ispDrvSwitch2Phy()
{
Mutex::Autolock lock(mLock);

    ISP_FUNC_DBG("E\n");

    this->m_pIspDrvShell->ispDrvSwitch2Phy();
    m_pIspDrv = this->m_pIspDrvShell->m_pIspDrv;
    m_pIspReg = this->m_pIspDrvShell->m_pIspReg;

    ISP_FUNC_DBG("X\n");
    return MTRUE;
}
*/

//
/*/////////////////////////////////////////////////////////////////////////////
    DMAI_B
  /////////////////////////////////////////////////////////////////////////////*/
int DMAI_B::_config( void )
{    
    unsigned long pBase = 0;
    MUINT32 mdpmgrIdx = 0;
    
    ISP_FUNC_DBG("DMAI_B::_config:E");
    ISP_FUNC_DBG("id(0x%08x),CQ(%d),sceID(%u)",this->id(),this->CQ,DMAI_B::sceID);
    ISP_FUNC_DBG("%s:[base_pAddr:0x%08x]/[ofst_addr:0x%08x]",this->name_Str(),this->dma_cfg.memBuf.base_pAddr,this->dma_cfg.memBuf.ofst_addr);

    // switch MdpMgr cmdQ index and sceID (only for pass2)
    if(this->CQ == CAM_ISP_CQ1)
    {
        mdpmgrIdx = CAM_ISP_CQ1;
        g_MdpMgrCqIdx = CAM_ISP_CQ1;
        g_MdpMgrCfgData[CAM_ISP_CQ1].cqIdx = CAM_ISP_CQ1;
    }
    else if(this->CQ == CAM_ISP_CQ2)
    {
        mdpmgrIdx = CAM_ISP_CQ2;
        g_MdpMgrCqIdx = CAM_ISP_CQ2;
        g_MdpMgrCfgData[CAM_ISP_CQ2].cqIdx = CAM_ISP_CQ2;
    }
    
    if((this->CQ == CAM_ISP_CQ1 || this->CQ == CAM_ISP_CQ2) && DMAI_B::sceID <= 12)
    {
        ISP_FUNC_DBG("mdpmgrIdx(%d)",mdpmgrIdx);
        g_MdpMgrCfgData[mdpmgrIdx].sceID = DMAI_B::sceID;
    }
            
     switch(this->id())
     {
        case ISP_DMA_TDRI:
            if(m_pIspDrv->IsReadOnlyMode())
            {
                ISP_IOCTL_WRITE_REG(m_pIspDrv,m_pIspDrv->getRegAddrMap(), CAM_TDRI_BASE_ADDR, this->dma_cfg.memBuf.base_pAddr);//baseAddr;
            }
            else
            {
                ISP_WRITE_REG(m_pIspReg, CAM_TDRI_BASE_ADDR, this->dma_cfg.memBuf.base_pAddr);//baseAddr;
            }
            ISP_FUNC_DBG("m_pIspReg(0x%x),CAM_TDRI_BASE_ADDR = 0x%x",m_pIspReg,this->dma_cfg.memBuf.base_pAddr);
            break;
        case ISP_DMA_CQI:
            this->m_pIspDrvShell->cam_cq_cfg(this->CQ);
            this->ispDrvSwitch2Virtual(this->CQ);
           break;
        case ISP_DMA_IMGI:
            if(m_pIspDrv->IsReadOnlyMode())
            {
                ISP_IOCTL_WRITE_BITS(m_pIspDrv, m_pIspDrv->getRegAddrMap(), CAM_IMGI_STRIDE,SWAP, this->dma_cfg.swap);
                ISP_IOCTL_WRITE_BITS(m_pIspDrv, m_pIspDrv->getRegAddrMap(), CAM_IMGI_STRIDE,FORMAT_EN, this->dma_cfg.format_en);
                ISP_IOCTL_WRITE_BITS(m_pIspDrv, m_pIspDrv->getRegAddrMap(), CAM_IMGI_STRIDE,FORMAT, this->dma_cfg.format);
                ISP_IOCTL_WRITE_BITS(m_pIspDrv, m_pIspDrv->getRegAddrMap(), CAM_IMGI_STRIDE,BUS_SIZE_EN, this->dma_cfg.bus_size_en);
                ISP_IOCTL_WRITE_BITS(m_pIspDrv, m_pIspDrv->getRegAddrMap(), CAM_IMGI_STRIDE,BUS_SIZE, this->dma_cfg.bus_size);
                ISP_IOCTL_WRITE_BITS(m_pIspDrv, m_pIspDrv->getRegAddrMap(), CAM_IMGI_STRIDE,STRIDE, ((this->dma_cfg.size.stride* this->dma_cfg.pixel_byte)>> CAM_ISP_PIXEL_BYTE_FP ));
                ISP_IOCTL_WRITE_REG(m_pIspDrv, m_pIspDrv->getRegAddrMap(), CAM_IMGI_CON, 0x08414140);  // ultra-high cause CQ1/2/3
                ISP_IOCTL_WRITE_REG(m_pIspDrv, m_pIspDrv->getRegAddrMap(), CAM_IMGI_CON2, 0x00414100);
                //ISP_IOCTL_WRITE_BITS(m_pIspDrv, m_pIspDrv->getRegAddrMap(), CAM_IMGI_RING_BUF,RING_SIZE, this->dma_cfg.ring_size);
                //ISP_IOCTL_WRITE_BITS(m_pIspDrv, m_pIspDrv->getRegAddrMap(), CAM_IMGI_RING_BUF,EN, this->dma_cfg.ring_en);
                ISP_FUNC_DBG("[ISP_BUF]:isRO(%d) IMGI_BASE_ADDR=[0x%08X] ",m_pIspDrv->IsReadOnlyMode(),ISP_IOCTL_READ_REG(m_pIspDrv, m_pIspDrv->getRegAddrMap(), CAM_IMGI_BASE_ADDR));
            }
            else
            {
                ISP_WRITE_BITS(m_pIspReg, CAM_IMGI_STRIDE,SWAP, this->dma_cfg.swap);
                ISP_WRITE_BITS(m_pIspReg, CAM_IMGI_OFST_ADDR,OFFSET_ADDR, 0); // TEST_MDP
                ISP_WRITE_BITS(m_pIspReg, CAM_IMGI_STRIDE,FORMAT_EN, this->dma_cfg.format_en);
                ISP_WRITE_BITS(m_pIspReg, CAM_IMGI_STRIDE,FORMAT, this->dma_cfg.format);
                ISP_WRITE_BITS(m_pIspReg, CAM_IMGI_STRIDE,BUS_SIZE_EN, this->dma_cfg.bus_size_en);
                ISP_WRITE_BITS(m_pIspReg, CAM_IMGI_STRIDE,BUS_SIZE, this->dma_cfg.bus_size);
                ISP_WRITE_BITS(m_pIspReg, CAM_IMGI_STRIDE,STRIDE, ((this->dma_cfg.size.stride* this->dma_cfg.pixel_byte)>> CAM_ISP_PIXEL_BYTE_FP ));
                ISP_WRITE_REG(m_pIspReg, CAM_IMGI_CON, 0x08414140); // ultra-high cause CQ1/2/3
                ISP_WRITE_REG(m_pIspReg, CAM_IMGI_CON2, 0x00414100);
//TEST 0418               ISP_WRITE_REG(m_pPhyIspReg, CAM_IMGI_CON, 0x08211540); // TEST_MDP by m_pPhyIspReg
//TEST 0418               ISP_WRITE_REG(m_pPhyIspReg, CAM_IMGI_CON2, 0x00000000);// TEST_MDP by m_pPhyIspReg
                //ISP_WRITE_BITS(m_pIspReg, CAM_IMGI_RING_BUF,RING_SIZE, this->dma_cfg.ring_size);
               // ISP_WRITE_BITS(m_pIspReg, CAM_IMGI_RING_BUF,EN, this->dma_cfg.ring_en);
                ISP_FUNC_DBG("[ISP_BUF]:isRO(%d) m_pIspReg=[0x%08x]/IMGI_BASE_ADDR=[0x%08X] ",m_pIspDrv->IsReadOnlyMode(),m_pIspReg,ISP_READ_REG(m_pIspReg, CAM_IMGI_BASE_ADDR));
            }              
    
            // collect src image info for MdpMgr
            g_MdpMgrCfgData[mdpmgrIdx].srcFmt    = this->dma_cfg.lIspColorfmt;
            g_MdpMgrCfgData[mdpmgrIdx].srcW      = this->dma_cfg.size.w;
            g_MdpMgrCfgData[mdpmgrIdx].srcH      = this->dma_cfg.size.h;
            g_MdpMgrCfgData[mdpmgrIdx].srcStride = (this->dma_cfg.size.stride* this->dma_cfg.pixel_byte) >> CAM_ISP_PIXEL_BYTE_FP;

            // collect crop info for MdpMgr
            g_MdpMgrCfgData[mdpmgrIdx].dstCropX      = this->dma_cfg.crop.x;
            g_MdpMgrCfgData[mdpmgrIdx].dstCropFloatX = this->dma_cfg.crop.floatX;
            g_MdpMgrCfgData[mdpmgrIdx].dstCropY      = this->dma_cfg.crop.y;
            g_MdpMgrCfgData[mdpmgrIdx].dstCropFloatY = this->dma_cfg.crop.floatY;
            g_MdpMgrCfgData[mdpmgrIdx].dstCropW      = this->dma_cfg.crop.w;
            g_MdpMgrCfgData[mdpmgrIdx].dstCropH      = this->dma_cfg.crop.h;

            ISP_FUNC_DBG("mdpmgrIdx(%u)",mdpmgrIdx);
                
            ISP_FUNC_DBG("g_MdpMgrCfgData_src : Fmt(%d),W(%u),H(%u),stride(%u)", g_MdpMgrCfgData[mdpmgrIdx].srcFmt,
                                                                                   g_MdpMgrCfgData[mdpmgrIdx].srcW,
                                                                                   g_MdpMgrCfgData[mdpmgrIdx].srcH,
                                                                                   g_MdpMgrCfgData[mdpmgrIdx].srcStride);

            ISP_FUNC_DBG("g_MdpMgrCfgData_dst_crop : X(%u),FloatX(%u),Y(%u),FloatY(%u),W(%u),H(%u)", g_MdpMgrCfgData[mdpmgrIdx].dstCropX,
                                                                                                       g_MdpMgrCfgData[mdpmgrIdx].dstCropFloatX,
                                                                                                       g_MdpMgrCfgData[mdpmgrIdx].dstCropY,
                                                                                                       g_MdpMgrCfgData[mdpmgrIdx].dstCropFloatY,
                                                                                                       g_MdpMgrCfgData[mdpmgrIdx].dstCropW,
                                                                                                       g_MdpMgrCfgData[mdpmgrIdx].dstCropH);            

            break;
        case ISP_DMA_IMGCI:
            if(m_pIspDrv->IsReadOnlyMode())
            {
                //ISP_IOCTL_WRITE_REG(m_pIspDrv, m_pIspDrv->getRegAddrMap(), CAM_IMGCI_BASE_ADDR, this->dma_cfg.memBuf.base_pAddr + this->dma_cfg.memBuf.ofst_addr);
                //ISP_IOCTL_WRITE_REG(m_pIspDrv, m_pIspDrv->getRegAddrMap(), CAM_IMGCI_XSIZE, ( (this->dma_cfg.size.w * this->dma_cfg.pixel_byte)>> CAM_ISP_PIXEL_BYTE_FP ) - 1);
                //ISP_IOCTL_WRITE_REG(m_pIspDrv, m_pIspDrv->getRegAddrMap(), CAM_IMGCI_YSIZE, this->dma_cfg.size.h - 1);//ySize;
                //ISP_IOCTL_WRITE_BITS(m_pIspDrv, m_pIspDrv->getRegAddrMap(), CAM_IMGCI_STRIDE,SWAP, this->dma_cfg.swap);
                //ISP_IOCTL_WRITE_BITS(m_pIspDrv, m_pIspDrv->getRegAddrMap(), CAM_IMGCI_STRIDE,FORMAT_EN, this->dma_cfg.format_en);
                //ISP_IOCTL_WRITE_BITS(m_pIspDrv, m_pIspDrv->getRegAddrMap(), CAM_IMGCI_STRIDE,FORMAT, this->dma_cfg.format);
                //ISP_IOCTL_WRITE_BITS(m_pIspDrv, m_pIspDrv->getRegAddrMap(), CAM_IMGCI_STRIDE,BUS_SIZE_EN, this->dma_cfg.bus_size_en);
                //ISP_IOCTL_WRITE_BITS(m_pIspDrv, m_pIspDrv->getRegAddrMap(), CAM_IMGCI_STRIDE,BUS_SIZE, this->dma_cfg.bus_size);
                //ISP_IOCTL_WRITE_BITS(m_pIspDrv, m_pIspDrv->getRegAddrMap(), CAM_IMGCI_STRIDE,STRIDE, ((this->dma_cfg.size.stride* this->dma_cfg.pixel_byte)>> CAM_ISP_PIXEL_BYTE_FP ));
                //ISP_IOCTL_WRITE_REG(m_pIspDrv, m_pIspDrv->getRegAddrMap(), CAM_IMGCI_CON, 0x080B0220);
            }
            else
            {
            /*ISP_REG(m_pIspReg, CAM_IMGCI_BASE_ADDR) = this->dma_cfg.memBuf.base_pAddr + this->dma_cfg.memBuf.ofst_addr;
            //ISP_REG(m_pIspReg, CAM_IMGCI_OFST_ADDR) = this->dma_cfg.memBuf.ofst_addr;
            ISP_REG(m_pIspReg, CAM_IMGCI_XSIZE) = ( (this->dma_cfg.size.w * this->dma_cfg.pixel_byte)>> CAM_ISP_PIXEL_BYTE_FP ) - 1;
            ISP_REG(m_pIspReg, CAM_IMGCI_YSIZE) = this->dma_cfg.size.h - 1;//ySize;
            ISP_BITS(m_pIspReg, CAM_IMGCI_STRIDE,SWAP) = this->dma_cfg.swap;
            ISP_BITS(m_pIspReg, CAM_IMGCI_STRIDE,FORMAT_EN) = this->dma_cfg.format_en;
            ISP_BITS(m_pIspReg, CAM_IMGCI_STRIDE,FORMAT) = this->dma_cfg.format;
            ISP_BITS(m_pIspReg, CAM_IMGCI_STRIDE,BUS_SIZE_EN) = this->dma_cfg.bus_size_en;
            ISP_BITS(m_pIspReg, CAM_IMGCI_STRIDE,BUS_SIZE) = this->dma_cfg.bus_size;
            ISP_BITS(m_pIspReg, CAM_IMGCI_STRIDE,STRIDE) = ((this->dma_cfg.size.stride* this->dma_cfg.pixel_byte)>> CAM_ISP_PIXEL_BYTE_FP );
            ISP_REG(m_pIspReg, CAM_IMGCI_CON) = 0x080B0220;*/
            }
            break;
        case ISP_DMA_VIPI:
            if(m_pIspDrv->IsReadOnlyMode())
            {
                //ISP_IOCTL_WRITE_REG(m_pIspDrv, m_pIspDrv->getRegAddrMap(), CAM_VIPI_BASE_ADDR, this->dma_cfg.memBuf.base_pAddr + this->dma_cfg.memBuf.ofst_addr);
                //ISP_IOCTL_WRITE_REG(m_pIspDrv, m_pIspDrv->getRegAddrMap(), CAM_VIPI_XSIZE, ( (this->dma_cfg.size.w * this->dma_cfg.pixel_byte)>> CAM_ISP_PIXEL_BYTE_FP ) - 1);
                //ISP_IOCTL_WRITE_REG(m_pIspDrv, m_pIspDrv->getRegAddrMap(), CAM_VIPI_YSIZE, this->dma_cfg.size.h - 1);//ySize;
                //ISP_IOCTL_WRITE_BITS(m_pIspDrv, m_pIspDrv->getRegAddrMap(), CAM_VIPI_STRIDE,SWAP, this->dma_cfg.swap);
                //ISP_IOCTL_WRITE_BITS(m_pIspDrv, m_pIspDrv->getRegAddrMap(), CAM_VIPI_STRIDE,FORMAT_EN, this->dma_cfg.format_en);
                //ISP_IOCTL_WRITE_BITS(m_pIspDrv, m_pIspDrv->getRegAddrMap(), CAM_VIPI_STRIDE,FORMAT, this->dma_cfg.format);
                //ISP_IOCTL_WRITE_BITS(m_pIspDrv, m_pIspDrv->getRegAddrMap(), CAM_VIPI_STRIDE,BUS_SIZE_EN, this->dma_cfg.bus_size_en);
                //ISP_IOCTL_WRITE_BITS(m_pIspDrv, m_pIspDrv->getRegAddrMap(), CAM_VIPI_STRIDE,BUS_SIZE, this->dma_cfg.bus_size);
                //ISP_IOCTL_WRITE_BITS(m_pIspDrv, m_pIspDrv->getRegAddrMap(), CAM_VIPI_STRIDE,STRIDE, ((this->dma_cfg.size.stride* this->dma_cfg.pixel_byte)>> CAM_ISP_PIXEL_BYTE_FP ));
                //ISP_IOCTL_WRITE_REG(m_pIspDrv, m_pIspDrv->getRegAddrMap(), CAM_VIPI_CON, 0xC11F1F20);
                //ISP_IOCTL_WRITE_BITS(m_pIspDrv, m_pIspDrv->getRegAddrMap(), CAM_VIPI_RING_BUF,RING_SIZE, this->dma_cfg.ring_size);
                //ISP_IOCTL_WRITE_BITS(m_pIspDrv, m_pIspDrv->getRegAddrMap(), CAM_VIPI_RING_BUF,EN, this->dma_cfg.ring_en);
            }
            else
            {
                 /*ISP_REG(m_pIspReg, CAM_VIPI_BASE_ADDR) = this->dma_cfg.memBuf.base_pAddr + this->dma_cfg.memBuf.ofst_addr;
                    //ISP_REG(m_pIspReg, CAM_VIPI_OFST_ADDR) = this->dma_cfg.memBuf.ofst_addr;
                    ISP_REG(m_pIspReg, CAM_VIPI_XSIZE) = ( (this->dma_cfg.size.w * this->dma_cfg.pixel_byte)>> CAM_ISP_PIXEL_BYTE_FP ) - 1;
                    ISP_REG(m_pIspReg, CAM_VIPI_YSIZE) = this->dma_cfg.size.h - 1;//ySize;
                    ISP_BITS(m_pIspReg, CAM_VIPI_STRIDE,SWAP) = this->dma_cfg.swap;
                    ISP_BITS(m_pIspReg, CAM_VIPI_STRIDE,FORMAT_EN) = this->dma_cfg.format_en;
                    ISP_BITS(m_pIspReg, CAM_VIPI_STRIDE,FORMAT) = this->dma_cfg.format;
                    ISP_BITS(m_pIspReg, CAM_VIPI_STRIDE,BUS_SIZE_EN) = this->dma_cfg.bus_size_en;
                    ISP_BITS(m_pIspReg, CAM_VIPI_STRIDE,BUS_SIZE) = this->dma_cfg.bus_size;
                    ISP_BITS(m_pIspReg, CAM_VIPI_STRIDE,STRIDE) = ((this->dma_cfg.size.stride* this->dma_cfg.pixel_byte)>> CAM_ISP_PIXEL_BYTE_FP );
                    ISP_REG(m_pIspReg, CAM_VIPI_CON) = 0xC11F1F20;

                    ISP_BITS(m_pIspReg, CAM_VIPI_RING_BUF,RING_SIZE) = this->dma_cfg.ring_size;
                    ISP_BITS(m_pIspReg, CAM_VIPI_RING_BUF,EN) = this->dma_cfg.ring_en;*/
            }
//            ISP_FUNC_DBG("ISP_DMA_VIPI = 0x%x",this->dma_cfg.memBuf.base_pAddr);
            break;
        case ISP_DMA_VIP2I:
            if(m_pIspDrv->IsReadOnlyMode())
            {
                /*ISP_IOCTL_WRITE_REG(m_pIspDrv, m_pIspDrv->getRegAddrMap(), CAM_VIP2I_BASE_ADDR, this->dma_cfg.memBuf.base_pAddr + this->dma_cfg.memBuf.ofst_addr);
                ISP_IOCTL_WRITE_REG(m_pIspDrv, m_pIspDrv->getRegAddrMap(), CAM_VIP2I_XSIZE, ( (this->dma_cfg.size.w * this->dma_cfg.pixel_byte)>> CAM_ISP_PIXEL_BYTE_FP ) - 1);
                ISP_IOCTL_WRITE_REG(m_pIspDrv, m_pIspDrv->getRegAddrMap(), CAM_VIP2I_YSIZE, this->dma_cfg.size.h - 1);//ySize;
                ISP_IOCTL_WRITE_BITS(m_pIspDrv, m_pIspDrv->getRegAddrMap(), CAM_VIP2I_STRIDE,FORMAT_EN, this->dma_cfg.format_en);
                ISP_IOCTL_WRITE_BITS(m_pIspDrv, m_pIspDrv->getRegAddrMap(), CAM_VIP2I_STRIDE,FORMAT, this->dma_cfg.format);
                ISP_IOCTL_WRITE_BITS(m_pIspDrv, m_pIspDrv->getRegAddrMap(), CAM_VIP2I_STRIDE,STRIDE, ((this->dma_cfg.size.stride* this->dma_cfg.pixel_byte)>> CAM_ISP_PIXEL_BYTE_FP ));
                ISP_IOCTL_WRITE_REG(m_pIspDrv, m_pIspDrv->getRegAddrMap(), CAM_VIP2I_CON, 0x080A0714);
                ISP_IOCTL_WRITE_BITS(m_pIspDrv, m_pIspDrv->getRegAddrMap(), CAM_VIP2I_RING_BUF,RING_SIZE, this->dma_cfg.ring_size);
                ISP_IOCTL_WRITE_BITS(m_pIspDrv, m_pIspDrv->getRegAddrMap(), CAM_VIP2I_RING_BUF,EN, this->dma_cfg.ring_en);*/
            }
            else
            {
             /*ISP_REG(m_pIspReg, CAM_VIP2I_BASE_ADDR) = this->dma_cfg.memBuf.base_pAddr + this->dma_cfg.memBuf.ofst_addr;
                //ISP_REG(m_pIspReg, CAM_VIP2I_OFST_ADDR) = this->dma_cfg.memBuf.ofst_addr;
                ISP_REG(m_pIspReg, CAM_VIP2I_XSIZE) = ( (this->dma_cfg.size.w * this->dma_cfg.pixel_byte)>> CAM_ISP_PIXEL_BYTE_FP ) - 1;
                ISP_REG(m_pIspReg, CAM_VIP2I_YSIZE) = this->dma_cfg.size.h - 1;//ySize;
                ISP_BITS(m_pIspReg, CAM_VIP2I_STRIDE,FORMAT_EN) = this->dma_cfg.format_en;
                ISP_BITS(m_pIspReg, CAM_VIP2I_STRIDE,FORMAT) = this->dma_cfg.format;
                ISP_BITS(m_pIspReg, CAM_VIP2I_STRIDE,STRIDE) = ((this->dma_cfg.size.stride* this->dma_cfg.pixel_byte)>> CAM_ISP_PIXEL_BYTE_FP );
                ISP_REG(m_pIspReg, CAM_VIP2I_CON) = 0x080A0714;
                ISP_BITS(m_pIspReg, CAM_VIP2I_RING_BUF,RING_SIZE) = this->dma_cfg.ring_size;
                ISP_BITS(m_pIspReg, CAM_VIP2I_RING_BUF,EN) = this->dma_cfg.ring_en;*/
            }
//            ISP_FUNC_DBG("CAM_VIP2I_BASE_ADDR = 0x%x",this->dma_cfg.memBuf.base_pAddr);
            break;

        case ISP_DMA_LSCI:
            if(m_pIspDrv->IsReadOnlyMode())
            {
                ISP_IOCTL_WRITE_REG(m_pIspDrv, m_pIspDrv->getRegAddrMap(), CAM_LSCI_BASE_ADDR, this->dma_cfg.memBuf.base_pAddr + this->dma_cfg.memBuf.ofst_addr);
                ISP_IOCTL_WRITE_REG(m_pIspDrv, m_pIspDrv->getRegAddrMap(), CAM_LSCI_XSIZE, ( (this->dma_cfg.size.w * this->dma_cfg.pixel_byte)>> CAM_ISP_PIXEL_BYTE_FP ) - 1);
                ISP_IOCTL_WRITE_REG(m_pIspDrv, m_pIspDrv->getRegAddrMap(), CAM_LSCI_YSIZE, this->dma_cfg.size.h - 1);//ySize;
                ISP_IOCTL_WRITE_BITS(m_pIspDrv, m_pIspDrv->getRegAddrMap(), CAM_LSCI_STRIDE,SWAP, this->dma_cfg.swap);
                ISP_IOCTL_WRITE_BITS(m_pIspDrv, m_pIspDrv->getRegAddrMap(), CAM_LSCI_STRIDE,FORMAT_EN, this->dma_cfg.format_en);
                ISP_IOCTL_WRITE_BITS(m_pIspDrv, m_pIspDrv->getRegAddrMap(), CAM_LSCI_STRIDE,FORMAT, this->dma_cfg.format);
                ISP_IOCTL_WRITE_BITS(m_pIspDrv, m_pIspDrv->getRegAddrMap(), CAM_LSCI_STRIDE,BUS_SIZE_EN, this->dma_cfg.bus_size_en);
                ISP_IOCTL_WRITE_BITS(m_pIspDrv, m_pIspDrv->getRegAddrMap(), CAM_LSCI_STRIDE,BUS_SIZE, this->dma_cfg.bus_size);
                ISP_IOCTL_WRITE_BITS(m_pIspDrv, m_pIspDrv->getRegAddrMap(), CAM_LSCI_STRIDE,STRIDE, ((this->dma_cfg.size.stride* this->dma_cfg.pixel_byte)>> CAM_ISP_PIXEL_BYTE_FP ));
#if 0
                ISP_IOCTL_WRITE_REG(m_pIspDrv, m_pIspDrv->getRegAddrMap(), CAM_LSCI_CON, 0x08161620);
#else //ultra-highest
                ISP_IOCTL_WRITE_REG(m_pIspDrv, m_pIspDrv->getRegAddrMap(), CAM_LSCI_CON, 0x08000010);
#endif
            }
            else
            {
                ISP_WRITE_REG(m_pIspReg, CAM_LSCI_BASE_ADDR, this->dma_cfg.memBuf.base_pAddr + this->dma_cfg.memBuf.ofst_addr);
                ISP_WRITE_REG(m_pIspReg, CAM_LSCI_XSIZE, ( (this->dma_cfg.size.w * this->dma_cfg.pixel_byte)>> CAM_ISP_PIXEL_BYTE_FP ) - 1);
                ISP_WRITE_REG(m_pIspReg, CAM_LSCI_YSIZE, this->dma_cfg.size.h - 1);//ySize;
                ISP_WRITE_BITS(m_pIspReg, CAM_LSCI_STRIDE,SWAP, this->dma_cfg.swap);
                ISP_WRITE_BITS(m_pIspReg, CAM_LSCI_STRIDE,FORMAT_EN, this->dma_cfg.format_en);
                ISP_WRITE_BITS(m_pIspReg, CAM_LSCI_STRIDE,FORMAT, this->dma_cfg.format);
                ISP_WRITE_BITS(m_pIspReg, CAM_LSCI_STRIDE,BUS_SIZE_EN, this->dma_cfg.bus_size_en);
                ISP_WRITE_BITS(m_pIspReg, CAM_LSCI_STRIDE,BUS_SIZE, this->dma_cfg.bus_size);
                ISP_WRITE_BITS(m_pIspReg, CAM_LSCI_STRIDE,STRIDE, ((this->dma_cfg.size.stride* this->dma_cfg.pixel_byte)>> CAM_ISP_PIXEL_BYTE_FP ));
#if 0
                ISP_WRITE_REG(m_pIspReg, CAM_LSCI_CON, 0x08161620);
#else //ultra-highest
                ISP_WRITE_REG(m_pIspReg, CAM_LSCI_CON, 0x08000010);
#endif
                //ISP_REG(m_pIspReg, CAM_LSCI_CON) = 0x481A0020;
                //ISP_REG(m_pIspReg, CAM_LSCI_CON2) = 0x000C0A16;
            }
            break;
        case ISP_DMA_LCEI:
            if(m_pIspDrv->IsReadOnlyMode())
            {
                /*ISP_IOCTL_WRITE_REG(m_pIspDrv, m_pIspDrv->getRegAddrMap(), CAM_LCEI_BASE_ADDR, this->dma_cfg.memBuf.base_pAddr + this->dma_cfg.memBuf.ofst_addr);
                ISP_IOCTL_WRITE_REG(m_pIspDrv, m_pIspDrv->getRegAddrMap(), CAM_LCEI_XSIZE, ( (this->dma_cfg.size.w * this->dma_cfg.pixel_byte)>> CAM_ISP_PIXEL_BYTE_FP ) - 1);
                ISP_IOCTL_WRITE_REG(m_pIspDrv, m_pIspDrv->getRegAddrMap(), CAM_LCEI_YSIZE, this->dma_cfg.size.h - 1);//ySize;
                ISP_IOCTL_WRITE_BITS(m_pIspDrv, m_pIspDrv->getRegAddrMap(), CAM_LCEI_STRIDE,SWAP, this->dma_cfg.swap);
                ISP_IOCTL_WRITE_BITS(m_pIspDrv, m_pIspDrv->getRegAddrMap(), CAM_LCEI_STRIDE,FORMAT_EN, this->dma_cfg.format_en);
                ISP_IOCTL_WRITE_BITS(m_pIspDrv, m_pIspDrv->getRegAddrMap(), CAM_LCEI_STRIDE,FORMAT, this->dma_cfg.format);
                ISP_IOCTL_WRITE_BITS(m_pIspDrv, m_pIspDrv->getRegAddrMap(), CAM_LCEI_STRIDE,BUS_SIZE_EN, this->dma_cfg.bus_size_en);
                ISP_IOCTL_WRITE_BITS(m_pIspDrv, m_pIspDrv->getRegAddrMap(), CAM_LCEI_STRIDE,BUS_SIZE, this->dma_cfg.bus_size);
                ISP_IOCTL_WRITE_BITS(m_pIspDrv, m_pIspDrv->getRegAddrMap(), CAM_LCEI_STRIDE,STRIDE, ((this->dma_cfg.size.stride* this->dma_cfg.pixel_byte)>> CAM_ISP_PIXEL_BYTE_FP ));
                ISP_IOCTL_WRITE_REG(m_pIspDrv, m_pIspDrv->getRegAddrMap(), CAM_LCEI_CON, 0x080B0220);*/
            }
            else
            {
                /*ISP_REG(m_pIspReg, CAM_LCEI_BASE_ADDR) = this->dma_cfg.memBuf.base_pAddr + this->dma_cfg.memBuf.ofst_addr;
                ISP_REG(m_pIspReg, CAM_LCEI_XSIZE) = ( (this->dma_cfg.size.w * this->dma_cfg.pixel_byte)>> CAM_ISP_PIXEL_BYTE_FP ) - 1;
                ISP_REG(m_pIspReg, CAM_LCEI_YSIZE) = this->dma_cfg.size.h - 1;//ySize;
                ISP_BITS(m_pIspReg, CAM_LCEI_STRIDE,SWAP) = this->dma_cfg.swap;
                ISP_BITS(m_pIspReg, CAM_LCEI_STRIDE,FORMAT_EN) = this->dma_cfg.format_en;
                ISP_BITS(m_pIspReg, CAM_LCEI_STRIDE,FORMAT) = this->dma_cfg.format;
                ISP_BITS(m_pIspReg, CAM_LCEI_STRIDE,BUS_SIZE_EN) = this->dma_cfg.bus_size_en;
                ISP_BITS(m_pIspReg, CAM_LCEI_STRIDE,BUS_SIZE) = this->dma_cfg.bus_size;
                ISP_BITS(m_pIspReg, CAM_LCEI_STRIDE,STRIDE) = ((this->dma_cfg.size.stride* this->dma_cfg.pixel_byte)>> CAM_ISP_PIXEL_BYTE_FP );
                ISP_REG(m_pIspReg, CAM_LCEI_CON) = 0x080B0220;*/
            //ISP_REG(m_pIspReg, CAM_LCEI_CON) = 0x481C0720;
            //ISP_REG(m_pIspReg, CAM_LCEI_CON2) = 0x00100A10;
            }
            break;
        default:
            break;
    }

    ISP_FUNC_DBG("DMAI_B::_config:X ");
    return 0;
}

/*
int DMAI_B::_enable( void* pParam )
{
    ISP_FUNC_DBG("DMAI_B::_enable:E ");
    ISP_FUNC_DBG("[id:0x%08d] ",this->id());
    ISP_FUNC_DBG("DMAI_B::_enable:X ");
    return 0;
}
*/

int DMAI_B::_disable( void )
{
    ISP_FUNC_DBG("DMAI_B::_disable:E ");
    ISP_FUNC_DBG("[id:0x%08d] ",this->id());

    switch(this->id()) {
        case ISP_DMA_CQI:
#if 0 //js_test remove below later
            this->m_pIspDrvShell->cam_cq_uninit();
#endif
            break;
        default:
            break;
    }

    ISP_FUNC_DBG("DMAI_B::_disable:X ");
    return 0;
}

/*
int DMAI_B::checkBusy( unsigned long* param  )
{
unsigned long u4Result;
    ISP_FUNC_DBG("DMAI_B::checkBusy:E ");
    ISP_FUNC_DBG("DMAI_B::checkBusy:X ");
    return (u4Result & 0x1 ? 0 : 1);
}
*/

int DMAI_B::_write2CQ( int cq )
{
unsigned int cq_module_id = 0xFF;

    ISP_FUNC_DBG("DMAI_B::_write2CQ:E ");
    ISP_FUNC_DBG("[id:0X%08X] ",this->id());

     switch(this->id())
     {
        case ISP_DMA_IMGI:
            cq_module_id = CAM_DMA_IMGI;  // leave it, first
            break;
        case ISP_DMA_TDRI:
            cq_module_id = CAM_DMA_TDRI;
            break;
        case ISP_DMA_VIPI:
            //cq_module_id = CAM_DMA_VIPI;
            break;
        case ISP_DMA_VIP2I:
            //cq_module_id = CAM_DMA_VIP2I;
            break;
        case ISP_DMA_IMGCI:
            //cq_module_id = CAM_DMA_IMGCI;
            break;
        case ISP_DMA_LSCI:
            cq_module_id = CAM_DMA_LSCI;
            break;
        case ISP_DMA_LCEI:
            //cq_module_id = CAM_DMA_LCEI;
            break;
        case ISP_DMA_CQI:
        default:
            ISP_FUNC_DBG(" NOT push to CQ ");
            break;
    }

    if (0xFF != cq_module_id) {
        this->m_pIspDrvShell->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,(CAM_MODULE_ENUM)cq_module_id);
#if 0 //js_test remove below later
        this->m_pIspDrvShell->cam_cq_write_to_queue_buf(
            this->CQ,
            cq_module_id,
            cq_module_info[cq_module_id].addr_ofst,
            cq_module_info[cq_module_id].reg_num,
            (unsigned int*)&this->m_pIspDrvShell->m_pVirtRegAddr[cq_module_info[cq_module_id].addr_ofst]);
#endif
    }

    ISP_FUNC_DBG("DMAI_B::_write2CQ:X ");
    return 0;
}

/*/////////////////////////////////////////////////////////////////////////////
    DMAO_B
  /////////////////////////////////////////////////////////////////////////////*/
int DMAO_B::_config( void )
{
    ISP_FUNC_DBG("DMAO_B::_config:E ");
    //ISP_FUNC_DBG("[id:0x%08d] ",this->id());
    ISP_FUNC_DBG("%s:[base_pAddr:0x%08X][ofst_addr:0x%08X][crop:%d %d] ",this->name_Str(),this->dma_cfg.memBuf.base_pAddr,
            this->dma_cfg.memBuf.ofst_addr,this->dma_cfg.crop.x,this->dma_cfg.crop.y);
     switch(this->id())
     {
        case ISP_DMA_IMGO:
            if(m_pIspDrv->IsReadOnlyMode())
            {
                //ISP_IOCTL_WRITE_REG(m_pIspDrv, m_pIspDrv->getRegAddrMap(), CAM_IMGO_BASE_ADDR, this->dma_cfg.memBuf.base_pAddr);
                ISP_IOCTL_WRITE_REG(m_pIspDrv, m_pIspDrv->getRegAddrMap(), CAM_IMGO_XSIZE,
                                (((( (this->dma_cfg.size.w * this->dma_cfg.pixel_byte + 0x03)>> CAM_ISP_PIXEL_BYTE_FP )+1)>>1)<<1) - 1);
                ISP_IOCTL_WRITE_REG(m_pIspDrv, m_pIspDrv->getRegAddrMap(), CAM_IMGO_YSIZE, this->dma_cfg.size.h - 1);//ySize;
                ISP_IOCTL_WRITE_REG(m_pIspDrv, m_pIspDrv->getRegAddrMap(), CAM_IMGO_STRIDE, (this->dma_cfg.size.stride * this->dma_cfg.pixel_byte)>> CAM_ISP_PIXEL_BYTE_FP);//stride;
#if 0
                ISP_IOCTL_WRITE_REG(m_pIspDrv, m_pIspDrv->getRegAddrMap(), CAM_IMGO_CON, 0x08000050);  // ultra-high
                ISP_IOCTL_WRITE_REG(m_pIspDrv, m_pIspDrv->getRegAddrMap(), CAM_IMGO_CON2, 0x00000000);
#elif 0
                ISP_IOCTL_WRITE_REG(m_pIspDrv, m_pIspDrv->getRegAddrMap(), CAM_IMGO_CON, 0x08010150);  // ultra-highest
                ISP_IOCTL_WRITE_REG(m_pIspDrv, m_pIspDrv->getRegAddrMap(), CAM_IMGO_CON2, 0x00010100);
#elif 1
                ISP_IOCTL_WRITE_REG(m_pIspDrv, m_pIspDrv->getRegAddrMap(), CAM_IMGO_CON, 0x08141450);  // ultra-
                ISP_IOCTL_WRITE_REG(m_pIspDrv, m_pIspDrv->getRegAddrMap(), CAM_IMGO_CON2, 0x00141400);
#endif
                ISP_IOCTL_WRITE_REG(m_pIspDrv, m_pIspDrv->getRegAddrMap(), CAM_IMGO_CROP, (this->dma_cfg.crop.y << 16) | this->dma_cfg.crop.x);
            }
            else
            {
                //ISP_WRITE_REG(m_pIspReg, CAM_IMGO_BASE_ADDR, this->dma_cfg.memBuf.base_pAddr);
                //ISP_REG(m_pIspReg, CAM_IMGO_OFST_ADDR) = this->dma_cfg.memBuf.ofst_addr;

                //rounding
                //ISP_REG(m_pIspReg, CAM_IMGO_XSIZE) = ( (this->dma_cfg.size.w * this->dma_cfg.pixel_byte + 0x03)>> CAM_ISP_PIXEL_BYTE_FP ) - 1;
                ISP_WRITE_REG(m_pIspReg, CAM_IMGO_XSIZE,
                                (((( (this->dma_cfg.size.w * this->dma_cfg.pixel_byte + 0x03)>> CAM_ISP_PIXEL_BYTE_FP )+1)>>1)<<1) - 1);
                ISP_WRITE_REG(m_pIspReg, CAM_IMGO_YSIZE, this->dma_cfg.size.h - 1);//ySize;
                ISP_WRITE_REG(m_pIspReg, CAM_IMGO_STRIDE, (this->dma_cfg.size.stride * this->dma_cfg.pixel_byte)>> CAM_ISP_PIXEL_BYTE_FP);//stride;
#if 0
                ISP_WRITE_REG(m_pIspReg, CAM_IMGO_CON, 0x08000050); // ultra-high
                ISP_WRITE_REG(m_pIspReg, CAM_IMGO_CON2, 0x00000000);
#elif 0
                ISP_WRITE_REG(m_pIspReg, CAM_IMGO_CON, 0x08010150); // ultra-highest
                ISP_WRITE_REG(m_pIspReg, CAM_IMGO_CON2, 0x00010100);
#elif 1
                ISP_WRITE_REG(m_pIspReg, CAM_IMGO_CON, 0x08141450); // ultra-
                ISP_WRITE_REG(m_pIspReg, CAM_IMGO_CON2, 0x00141400);
#endif
                ISP_WRITE_REG(m_pIspReg, CAM_IMGO_CROP, (this->dma_cfg.crop.y << 16) | this->dma_cfg.crop.x);
            }

            ISP_FUNC_DBG("[ISP_BUF]:isRO(%d),m_pIspReg=[0x%08x]",m_pIspDrv->IsReadOnlyMode(),m_pIspReg);

            break;
        case ISP_DMA_IMG2O:
            if(m_pIspDrv->IsReadOnlyMode())
            {
                ISP_IOCTL_WRITE_REG(m_pIspDrv, m_pIspDrv->getRegAddrMap(), CAM_IMG2O_BASE_ADDR, this->dma_cfg.memBuf.base_pAddr);
                ISP_IOCTL_WRITE_REG(m_pIspDrv, m_pIspDrv->getRegAddrMap(), CAM_IMG2O_XSIZE,
                                (((( (this->dma_cfg.size.w * this->dma_cfg.pixel_byte + 0x03)>> CAM_ISP_PIXEL_BYTE_FP )+1)>>1)<<1) - 1);

                ISP_IOCTL_WRITE_REG(m_pIspDrv, m_pIspDrv->getRegAddrMap(), CAM_IMG2O_YSIZE, this->dma_cfg.size.h - 1);//ySize;
                ISP_IOCTL_WRITE_REG(m_pIspDrv, m_pIspDrv->getRegAddrMap(), CAM_IMG2O_STRIDE, (this->dma_cfg.size.stride* this->dma_cfg.pixel_byte) >> CAM_ISP_PIXEL_BYTE_FP);//stride;
                ISP_IOCTL_WRITE_REG(m_pIspDrv, m_pIspDrv->getRegAddrMap(), CAM_IMG2O_CON, 0x08101040);  // ultra-high for pass1, not for 3DNR
                ISP_IOCTL_WRITE_REG(m_pIspDrv, m_pIspDrv->getRegAddrMap(), CAM_IMG2O_CON2, 0x00101000);
                ISP_IOCTL_WRITE_REG(m_pIspDrv, m_pIspDrv->getRegAddrMap(), CAM_IMG2O_CROP, (this->dma_cfg.crop.y << 16) | this->dma_cfg.crop.x);
            }
            else
            {
                ISP_WRITE_REG(m_pIspReg, CAM_IMG2O_BASE_ADDR, this->dma_cfg.memBuf.base_pAddr);
                //ISP_REG(m_pIspReg, CAM_IMG2O_OFST_ADDR) = this->dma_cfg.memBuf.ofst_addr;
                //rounding
                //ISP_REG(m_pIspReg, CAM_IMG2O_XSIZE) = ( (this->dma_cfg.size.w  * this->dma_cfg.pixel_byte + 0x03 ) >> CAM_ISP_PIXEL_BYTE_FP ) - 1;
                ISP_WRITE_REG(m_pIspReg, CAM_IMG2O_XSIZE,
                                (((( (this->dma_cfg.size.w * this->dma_cfg.pixel_byte + 0x03)>> CAM_ISP_PIXEL_BYTE_FP )+1)>>1)<<1) - 1);

                ISP_WRITE_REG(m_pIspReg, CAM_IMG2O_YSIZE, this->dma_cfg.size.h - 1);//ySize;
                ISP_WRITE_REG(m_pIspReg, CAM_IMG2O_STRIDE, (this->dma_cfg.size.stride* this->dma_cfg.pixel_byte) >> CAM_ISP_PIXEL_BYTE_FP);//stride;
                ISP_WRITE_REG(m_pIspReg, CAM_IMG2O_CON, 0x08101040); //ultra-high for pass1, not for 3DNR
                ISP_WRITE_REG(m_pIspReg, CAM_IMG2O_CON2, 0x00101000);
                ISP_WRITE_REG(m_pIspReg, CAM_IMG2O_CROP, (this->dma_cfg.crop.y << 16) | this->dma_cfg.crop.x);
            }
        default:
            break;
    }

    ISP_FUNC_DBG("DMAO_B::_config:X ");
    return 0;
}

int DMAO_B::_write2CQ( int cq )
{

unsigned int cq_module_id = 0xFF;

    ISP_FUNC_DBG("DMA)_B::_write2CQ:E ");
    ISP_FUNC_DBG("[id:0X%08X] ",this->id());

     switch(this->id())
     {
        case ISP_DMA_IMGO:
            cq_module_id = CAM_DMA_IMGO;
            break;
        case ISP_DMA_IMG2O:
            cq_module_id = CAM_DMA_IMG2O;
            break;
        default:
            ISP_FUNC_DBG(" NOT push to CQ ");
            break;
    }

    if (0xFF != cq_module_id) {
        this->m_pIspDrvShell->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,(CAM_MODULE_ENUM)cq_module_id);
    }

    ISP_FUNC_DBG("DMAO_B::_write2CQ:X ");

    return 0;
}

/*
int DMAO_B::_enable( void* pParam )
{
    ISP_FUNC_DBG("DMAO_B::_enable:E ");
    ISP_FUNC_DBG("DMAO_B::_enable:X ");
    return 0;
}

int DMAO_B::_disable( void )
{
    ISP_FUNC_DBG("DMAO_B::_disable:E ");
    ISP_FUNC_DBG("DMAO_B::_disable:X ");
    return 0;
}

int DMAO_B::checkBusy( unsigned long* param  )
{
unsigned long u4Result;
    ISP_FUNC_DBG("DMAO_B::checkBusy:E ");
    ISP_FUNC_DBG("DMAO_B::checkBusy:X ");
    return (u4Result & 0x1 ? 0 : 1);
}
*/
/*/////////////////////////////////////////////////////////////////////////////
    ISP_TOP_CTRL
  /////////////////////////////////////////////////////////////////////////////*/
int ISP_TOP_CTRL::_config( void )
{
unsigned int reg_tmp, dmaMerg = 0x00;
unsigned int turningEn1=0x00,turningEn2=0x00,turningDma=0x00;
unsigned int applyTurn = 0x00;
unsigned int turnEn1Tag=0x00, turnEn2Tag=0x00, turnDmaTag=0x00; //unable to touch sn


    ISP_FUNC_DBG("ISP_TOP_CTRL:E");
    ISP_FUNC_INF("path(%d),CQ(%d),en1(0x%08x),en2(0x%08x),dma(0x%08x),fmt(0x%08x),ctl(0x%08x),tcm_en(0x%08x),isIspOn(0x%x)", \
                                                          this->path, \
                                                          this->CQ, \
                                                          this->en_Top.enable1, \
                                                          this->en_Top.enable2, \
                                                          this->en_Top.dma, \
                                                          this->fmt_sel.reg_val, \
                                                          this->ctl_sel.reg_val, \
                                                          this->tcm_en, \
                                                          this->isIspOn);
    ISP_FUNC_DBG("int_en(0x%08x),intb_en(0x%08x),intc_en(0x%08x),dma_int(0x%x),dmab_int(0x%x),dmac_int(0x%x),muxS2(0x%x)", \
                                                          this->ctl_int.int_en, \
                                                          this->ctl_int.intb_en, \
                                                          this->ctl_int.intc_en, \
                                                          this->ctl_int.dma_int, \
                                                          this->ctl_int.dmab_int, \
                                                          this->ctl_int.dmac_int, \
                                                          ctl_mux_sel2.reg_val);
    ISP_FUNC_DBG("m_pIspReg(0x%08x) m_pPhyIspReg(0x%08x) m_pIspDrv(0x%x) m_pPhyIspDrv(0x%x)",m_pIspReg,m_pPhyIspReg,m_pIspDrv,m_pPhyIspDrv);
    /********************************************************************************************************************/
    /********************************************************************************************************************/
    /********************************************************************************************************************/
    /*** set to physical ***/
    //NOTE!!! should check CQ FIRST, otherwise virtual ISP may be NULL.
    if(CAM_ISP_CQ_NONE != this->CQ) 
    {        
        //this->ispDrvSwitch2Phy();
        
        if(ISP_PASS1 == this->path) 
        {
            //CQ enable
            if(CAM_CTL_EN2_CQ0_EN & en_Top.enable2) 
            {
                if(m_pPhyIspDrv->IsReadOnlyMode())
                {
                    ISP_IOCTL_WRITE_ENABLE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CTL_EN2, CQ0_EN, 1);
                }
                else
                {
                    ISP_WRITE_ENABLE_BITS(m_pPhyIspReg, CAM_CTL_EN2, CQ0_EN, 1);
                }

#if defined(_rtbc_use_cq0c_)
            //enable for rtbc
            if(m_pPhyIspDrv->IsReadOnlyMode())
            {
                ISP_IOCTL_WRITE_ENABLE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CTL_EN2, CQ0C_EN, 1);
            }
            else
            {
                ISP_WRITE_ENABLE_BITS(m_pPhyIspReg, CAM_CTL_EN2, CQ0C_EN, 1);
            }
#endif

            }
            //
            if(CAM_CTL_EN2_CQ0B_EN & en_Top.enable2) 
            {
                if(m_pPhyIspDrv->IsReadOnlyMode())
                {
                    ISP_IOCTL_WRITE_ENABLE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CTL_EN2, CQ0B_EN, 1);
                }
                else
                {
                    ISP_WRITE_ENABLE_BITS(m_pPhyIspReg, CAM_CTL_EN2, CQ0B_EN, 1);
                }
            }
            //
            if(CAM_CTL_EN2_CQ0C_EN & en_Top.enable2) 
            {
                if(m_pPhyIspDrv->IsReadOnlyMode())
                {
                    ISP_IOCTL_WRITE_ENABLE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CTL_EN2, CQ0C_EN, 1);
                }
                else
                {
                    ISP_WRITE_ENABLE_BITS(m_pPhyIspReg, CAM_CTL_EN2, CQ0C_EN, 1);
                }
            }
        }
        else /*if (ISP_PASS2 == this->path)*/ 
        {
            // init MdpMgr
            if(ISP_PASS2 == this->path)
            {
                ISP_FUNC_DBG("ISP_PASS2 init mdpmgr");
                
                if(MDPMGR_NO_ERROR != g_pMdpMgr->init(g_MdpMgrCfgData[CAM_ISP_CQ1]))
                {
                    ISP_FUNC_ERR("g_pMdpMgr->init fail");
                    return -1;
                }
            }
            else if(ISP_PASS2B == this->path)
            {
                ISP_FUNC_DBG("ISP_PASS2B init mdpmgr");
                
                if(MDPMGR_NO_ERROR != g_pMdpMgr->init(g_MdpMgrCfgData[CAM_ISP_CQ2]))
                {
                    ISP_FUNC_ERR("g_pMdpMgr->init fail");
                    return -1;
                }
            }
            
            /*** CQ enable should be set to real ***/
            if(m_pPhyIspDrv->IsReadOnlyMode())
            {
                ISP_IOCTL_WRITE_ENABLE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CTL_EN2, ISP_IOCTL_READ_REG(m_pPhyIspDrv,m_pPhyIspDrv->getRegAddrMap(), CAM_CTL_EN2)|( en_Top.enable2 & ( CAM_CTL_EN2_CQ1_EN|CAM_CTL_EN2_CQ2_EN|CAM_CTL_EN2_CQ3_EN ) ));
                ISP_IOCTL_WRITE_ENABLE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CTL_DMA_EN, ISP_IOCTL_READ_REG(m_pPhyIspDrv,m_pPhyIspDrv->getRegAddrMap(), CAM_CTL_DMA_EN)|( en_Top.dma & CAM_CTL_DMA_EN_IMGI_EN));
            }
            else
            {
                ISP_WRITE_ENABLE_REG(m_pPhyIspReg, CAM_CTL_EN2, ISP_READ_REG(m_pPhyIspReg, CAM_CTL_EN2)|( en_Top.enable2 & ( CAM_CTL_EN2_CQ1_EN|CAM_CTL_EN2_CQ2_EN|CAM_CTL_EN2_CQ3_EN ) ));
                ISP_WRITE_ENABLE_REG(m_pPhyIspReg, CAM_CTL_DMA_EN, ISP_READ_REG(m_pPhyIspReg, CAM_CTL_DMA_EN)|( en_Top.dma&CAM_CTL_DMA_EN_IMGI_EN ));
            }
        }
    }
    
    if(m_pPhyIspDrv->IsReadOnlyMode())
    {
        //INT enable
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CTL_INT_EN, ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CTL_INT_EN)|ctl_int.int_en);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CTL_INTB_EN, ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CTL_INTB_EN)|ctl_int.intb_en);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CTL_INTC_EN, ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CTL_INTC_EN)|ctl_int.intc_en);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CTL_DMA_INT, ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CTL_DMA_INT)|ctl_int.dma_int);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CTL_DMAB_INT, ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CTL_DMAB_INT)|ctl_int.dmab_int);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CTL_DMAC_INT, ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CTL_DMAC_INT)|ctl_int.dmac_int);
        //
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CTL_SPARE3, CAM_CTL_SPARE3_SET_1);
        //
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(),CAM_CTL_CLK_EN, 0x0001FFFF);
        //deafult set bin/bin2 disable
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(),CAM_BIN_MODE, 0x0);
        //ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(),CAM_BIN2_MODE, 0x0);
        #if 0
        if(this->en_Top.enable1 && CAM_CTL_EN1_G2G_EN)
        {
            //set G2G default
            ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(),CAM_G2G_CONV0A, 0x00000200);	/* 0x15004920: CAM_G2G_CONV0A */
            ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(),CAM_G2G_CONV0B, 0x000002CE);	/* 0x15004924: CAM_G2G_CONV0B */
            ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(),CAM_G2G_CONV1A, 0x07500200);	/* 0x15004928: CAM_G2G_CONV1A */
            ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(),CAM_G2G_CONV1B, 0x00000692);	/* 0x1500492C: CAM_G2G_CONV1B */
            ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(),CAM_G2G_CONV2A, 0x038B0200);	/* 0x15004930: CAM_G2G_CONV2A */
            ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(),CAM_G2G_CONV2B, 0x00000000);	/* 0x15004934: CAM_G2G_CONV2B */
            ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(),CAM_G2G_ACC, 0x00000009);	    /* 0x15004938: CAM_G2G_ACC */
        }
        #endif
        
        #if 0
        if(this->en_Top.enable1 && CAM_CTL_EN2_G2C_EN)
        {
            //set G2C default
            ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(),CAM_G2C_CONV_0A, 0x012D0099);	/* 0x15004A00: CAM_G2C_CONV_0A */
            ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(),CAM_G2C_CONV_0B, 0x0000003A);	/* 0x15004A04: CAM_G2C_CONV_0B */
            ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(),CAM_G2C_CONV_1A, 0x075607AA);	/* 0x15004A08: CAM_G2C_CONV_1A */
            ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(),CAM_G2C_CONV_1B, 0x00000100);	/* 0x15004A0C: CAM_G2C_CONV_1B */
            ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(),CAM_G2C_CONV_2A, 0x072A0100);	/* 0x15004A10: CAM_G2C_CONV_2A */
            ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(),CAM_G2C_CONV_2B, 0x000007D6);	/* 0x15004A14: CAM_G2C_CONV_2B */
        }
        #endif
#if 0
        // ultra-high
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(),CAM_ESFKO_CON, 0x040A0A08);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(),CAM_ESFKO_CON2,0x000A0A00);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(),CAM_AAO_CON, 0x08161620);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(),CAM_AAO_CON2, 0x00161600);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(),CAM_LSCI_CON, 0x08161620);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(),CAM_LSCI_CON2, 0x00161600);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(),CAM_FLKI_CON, 0x080A0A10);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(),CAM_FLKI_CON2, 0x000A0A00);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(),CAM_LCSO_CON, 0x08101010);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(),CAM_LCSO_CON2, 0x00101000);

#else
        // ultra-highest
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(),CAM_ESFKO_CON, 0x04000008); //0x080A0A10);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(),CAM_ESFKO_CON2,0x00000000); //0x000A0A00);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(),CAM_AAO_CON, 0x08000020);   //0x08161620
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(),CAM_AAO_CON2, 0x00000000);  //0x00161600
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(),CAM_LSCI_CON, 0x08000010);  //0x08161620
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(),CAM_LSCI_CON2, 0x00000000); //0x00161600
        //ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(),CAM_FLKI_CON, 0x08000010);//0x080A0A10
        //ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(),CAM_FLKI_CON2, 0x00000000);//0x000A0A00
        //ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(),CAM_LCSO_CON, 0x08000010);//0x08101010
        //ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(),CAM_LCSO_CON2, 0x00000000);//0x00101000
#endif

    }
    else
    {
        //INT enable
        ISP_WRITE_REG(m_pPhyIspReg, CAM_CTL_INT_EN, ISP_READ_REG(m_pPhyIspReg, CAM_CTL_INT_EN)|ctl_int.int_en);
        ISP_WRITE_REG(m_pPhyIspReg, CAM_CTL_INTB_EN, ISP_READ_REG(m_pPhyIspReg, CAM_CTL_INTB_EN)|ctl_int.intb_en);
        ISP_WRITE_REG(m_pPhyIspReg, CAM_CTL_INTC_EN, ISP_READ_REG(m_pPhyIspReg, CAM_CTL_INTC_EN)|ctl_int.intc_en);
        ISP_WRITE_REG(m_pPhyIspReg, CAM_CTL_DMA_INT, ISP_READ_REG(m_pPhyIspReg, CAM_CTL_DMA_INT)|ctl_int.dma_int);
        ISP_WRITE_REG(m_pPhyIspReg, CAM_CTL_DMAB_INT, ISP_READ_REG(m_pPhyIspReg, CAM_CTL_DMAB_INT)|ctl_int.dmab_int);
        ISP_WRITE_REG(m_pPhyIspReg, CAM_CTL_DMAC_INT, ISP_READ_REG(m_pPhyIspReg, CAM_CTL_DMAC_INT)|ctl_int.dmac_int);
        //
        ISP_WRITE_REG(m_pPhyIspReg, CAM_CTL_SPARE3, CAM_CTL_SPARE3_SET_1);
        //
        ISP_WRITE_REG(m_pPhyIspReg,CAM_CTL_CLK_EN, 0x0001FFFF);
        //deafult set bin/bin2 disable
        ISP_WRITE_REG(m_pPhyIspReg,CAM_BIN_MODE, 0x0);
        //ISP_WRITE_REG(m_pPhyIspReg,CAM_BIN2_MODE, 0x0);
        #if 0
        if(this->en_Top.enable1 && CAM_CTL_EN1_G2G_EN)
        {
            //set G2G default
            ISP_WRITE_REG(m_pPhyIspReg,CAM_G2G_CONV0A, 0x00000200);	/* 0x15004920: CAM_G2G_CONV0A */
            ISP_WRITE_REG(m_pPhyIspReg,CAM_G2G_CONV0B, 0x000002CE);	/* 0x15004924: CAM_G2G_CONV0B */
            ISP_WRITE_REG(m_pPhyIspReg,CAM_G2G_CONV1A, 0x07500200);	/* 0x15004928: CAM_G2G_CONV1A */
            ISP_WRITE_REG(m_pPhyIspReg,CAM_G2G_CONV1B, 0x00000692);	/* 0x1500492C: CAM_G2G_CONV1B */
            ISP_WRITE_REG(m_pPhyIspReg,CAM_G2G_CONV2A, 0x038B0200);	/* 0x15004930: CAM_G2G_CONV2A */
            ISP_WRITE_REG(m_pPhyIspReg,CAM_G2G_CONV2B, 0x00000000);	/* 0x15004934: CAM_G2G_CONV2B */
            ISP_WRITE_REG(m_pPhyIspReg,CAM_G2G_ACC, 0x00000009);	/* 0x15004938: CAM_G2G_ACC */
        }
        #endif
        #if 0
        if(this->en_Top.enable1 && CAM_CTL_EN2_G2C_EN)
        {
            //set G2C default
            ISP_WRITE_REG(m_pPhyIspReg,CAM_G2C_CONV_0A, 0x012D0099);	/* 0x15004A00: CAM_G2C_CONV_0A */
            ISP_WRITE_REG(m_pPhyIspReg,CAM_G2C_CONV_0B, 0x0000003A);	/* 0x15004A04: CAM_G2C_CONV_0B */
            ISP_WRITE_REG(m_pPhyIspReg,CAM_G2C_CONV_1A, 0x075607AA);	/* 0x15004A08: CAM_G2C_CONV_1A */
            ISP_WRITE_REG(m_pPhyIspReg,CAM_G2C_CONV_1B, 0x00000100);	/* 0x15004A0C: CAM_G2C_CONV_1B */
            ISP_WRITE_REG(m_pPhyIspReg,CAM_G2C_CONV_2A, 0x072A0100);	/* 0x15004A10: CAM_G2C_CONV_2A */
            ISP_WRITE_REG(m_pPhyIspReg,CAM_G2C_CONV_2B, 0x000007D6);	/* 0x15004A14: CAM_G2C_CONV_2B */
        }
        #endif

#if 0
        //ultra-high
        ISP_WRITE_REG(m_pPhyIspReg,CAM_ESFKO_CON, 0x040A0A08);
        ISP_WRITE_REG(m_pPhyIspReg,CAM_ESFKO_CON2, 0x000A0A00);
        ISP_WRITE_REG(m_pPhyIspReg,CAM_AAO_CON, 0x08161620);
        ISP_WRITE_REG(m_pPhyIspReg,CAM_AAO_CON2, 0x00161600);
        ISP_WRITE_REG(m_pPhyIspReg,CAM_LSCI_CON, 0x08161620);
        ISP_WRITE_REG(m_pPhyIspReg,CAM_LSCI_CON2, 0x00161600);
        ISP_WRITE_REG(m_pPhyIspReg,CAM_FLKI_CON, 0x08020210);
        ISP_WRITE_REG(m_pPhyIspReg,CAM_FLKI_CON2, 0x00020200);
        ISP_WRITE_REG(m_pPhyIspReg,CAM_LCSO_CON, 0x08101010);
        ISP_WRITE_REG(m_pPhyIspReg,CAM_LCSO_CON2, 0x00101000);
#else
        //ultra-highest
        ISP_WRITE_REG(m_pPhyIspReg,CAM_ESFKO_CON, 0x04000008);  //0x080A0A10
        ISP_WRITE_REG(m_pPhyIspReg,CAM_ESFKO_CON2, 0x00000000); //0x000A0A00
        ISP_WRITE_REG(m_pPhyIspReg,CAM_AAO_CON, 0x08000020);    //0x08161620
        ISP_WRITE_REG(m_pPhyIspReg,CAM_AAO_CON2, 0x00000000);   //0x00161600
        ISP_WRITE_REG(m_pPhyIspReg,CAM_LSCI_CON, 0x08000010);   //0x08161620
        ISP_WRITE_REG(m_pPhyIspReg,CAM_LSCI_CON2, 0x00000000);  //0x00161600
        //ISP_WRITE_REG(m_pPhyIspReg,CAM_FLKI_CON, 0x08000010);   //0x08020210
        //ISP_WRITE_REG(m_pPhyIspReg,CAM_FLKI_CON2, 0x00000000);  //0x00020200
        //ISP_WRITE_REG(m_pPhyIspReg,CAM_LCSO_CON, 0x08000010);   //0x08101010
        //ISP_WRITE_REG(m_pPhyIspReg,CAM_LCSO_CON2, 0x00000000);  //0x00101000
#endif
    }
    //
    if(CAM_ISP_CQ_NONE != this->CQ) 
    {
        if((CAM_ISP_CQ1 == this->CQ || CAM_ISP_CQ2 == this->CQ) && this->isApplyTurn) 
        {
            // set turning Tag
            if(ISP_HW_SCENARIO_NUM > this->fmt_sel.bit_field.scenario && ISP_HW_SUB_MODE_NUM > this->fmt_sel.bit_field.sub_mode) 
            {
                turnEn1Tag = gIspTurningTag[this->fmt_sel.bit_field.scenario][this->fmt_sel.bit_field.sub_mode].enable1;
                turnEn2Tag = gIspTurningTag[this->fmt_sel.bit_field.scenario][this->fmt_sel.bit_field.sub_mode].enable2;
                turnDmaTag = gIspTurningTag[this->fmt_sel.bit_field.scenario][this->fmt_sel.bit_field.sub_mode].dma;
            } 
            else 
            {
                ISP_FUNC_ERR("[Error]unknow scenario(%d) sub(%d)",this->fmt_sel.bit_field.scenario,this->fmt_sel.bit_field.sub_mode);
                return -1;
            }
            
            applyTurn = 1;
            turningEn1 = m_pPhyIspDrv->getTurnTopEn1(this->CQ);
            turningEn2 = m_pPhyIspDrv->getTurnTopEn2(this->CQ);
            turningDma = m_pPhyIspDrv->getTurnTopDma(this->CQ);
//            ISP_FUNC_INF(" TEST_MDP Bef_Or turningEn1 (0x%08x),turningEn2 (0x%08x),turningDma(0x%08x)\n",turningEn1,turningEn2,turningDma);
            
//            turningEn1 |= this->en_Top.enable1;  // MDP_TEST Must on , now
//            turningEn2 |= this->en_Top.enable2;// MDP_TEST Must on , now
//            turningDma |= this->en_Top.dma;// MDP_TEST Must on , now            
//            ISP_FUNC_INF(" TEST_MDP Aft_Or turningEn1 (0x%08x),turningEn2 (0x%08x),turningDma(0x%08x)\n",turningEn1,turningEn2,turningDma);
//            ISP_FUNC_INF(" TEST_MDP Aft_Or this->en_Top.enable1(0x%08x),enable2(0x%08x),dma(0x%08x)\n",this->en_Top.enable1,this->en_Top.enable2,this->en_Top.dma);
//            turningEn1 = m_pPhyIspDrv->getTurnTopEn1(this->CQ);
//            turningEn2 = m_pPhyIspDrv->getTurnTopEn2(this->CQ);
//            turningDma = m_pPhyIspDrv->getTurnTopDma(this->CQ);
//            ISP_FUNC_INF(" TEST_MDP RESET turningEn1 (%d),turningEn2 (0x%08x),turningDma(0x%08x)\n",turningEn1,turningEn2,turningDma);
#if 0 //MSG from log
[_config]  TEST_MDP Bef_Or turningEn1 (0),turningEn2 (0x00000000),turningDma(0x00000000)
[_config]  TEST_MDP Aft_Or turningEn1 (1143996416),turningEn2 (0x38a00003),turningDma(0x00080080)
[_config]  TEST_MDP Aft_Or this->en_Top.enable1(1143996416=0x44300000),enable2(0x38a00003),dma(0x00080080)
[_config]  TEST_MDP RESET turningEn1 (0),turningEn2 (0x00000000),turningDma(0x00000000)
#endif  //MSG from log
            if(this->isShareDmaCtlByTurn)
            {
                dmaMerg = (en_Top.dma & (~turnDmaTag)) | (turningDma & turnDmaTag);
            }
            else
            {
                dmaMerg = en_Top.dma;
            }

            this->tcm_en |= ((turningDma&CAM_CTL_DMA_EN_LCEI_EN)?0x00000040:0); // 4054[6]:LCEI
            this->tcm_en |= ((turningEn1&CAM_CTL_EN1_LCE_EN)?0x00080000:0); // 4054[19]:LCE
            this->tcm_en |= ((turningEn1&CAM_CTL_EN1_SL2_EN)?0x40000000:0); // 4054[30]:SL2
//            this->tcm_en |= ((turningEn1&CAM_CTL_EN1_LSC_EN)?0x00800000:0); // TEST MDP

            ISP_FUNC_INF("cq(%d),turEn1(0x%08x),turEn2(0x%08x),turDma(0x%08x),dmaMerg(0x%08x),tcm(0x%08x)\n",this->CQ,turningEn1,turningEn2,turningDma,dmaMerg,this->tcm_en);
        } 
        else
        {        
            applyTurn = 0;
            turnEn1Tag = 0x00;
            turnEn2Tag = 0x00;
            turnDmaTag = 0x00;
            dmaMerg = en_Top.dma;
        }
        ISP_FUNC_DBG("[TurnTag]cq(%d),SAppTur(%d),applyTurn(%d),En1(0x%x),En2(0x%x),Dma(0x%x)",this->CQ,this->isApplyTurn,applyTurn,turnEn1Tag,turnEn2Tag,turnDmaTag);
        //
        this->ispDrvSwitch2Virtual(this->CQ);
    }
    else    // When CAM_ISP_CQ_NONE.
    {
        applyTurn = 0;
        turnEn1Tag = 0x00;
        turnEn2Tag = 0x00;
        turnDmaTag = 0x00;
        dmaMerg = en_Top.dma;
    }
    
    /********************************************************************************************************************/

    if(ISP_PASS1 == this->path) 
    {
        if(m_pIspDrv->IsReadOnlyMode())
        {
            //clk enable
            ISP_IOCTL_WRITE_REG(m_pIspDrv, m_pIspDrv->getRegAddrMap(),CAM_CTL_CLK_EN, 0x0001FFFF);
            //continuous mode
            ISP_IOCTL_WRITE_BITS(m_pIspDrv, m_pIspDrv->getRegAddrMap(), CAM_TG_VF_CON, SINGLE_MODE, this->b_continuous?0:1);
            //DONOT clear unp_en/
            ISP_IOCTL_WRITE_ENABLE_REG(m_pIspDrv, m_pIspDrv->getRegAddrMap(), CAM_CTL_EN1_CLR,
                                ( CQ_CAM_CTL_CAN_BE_CLR_BITS(EN1,this->isIspOn)&(~0x00100000) )&(~en_Top.enable1)& \
                                CQ_CAM_CTL_BIT_FIXED(EN1,C24,this->isEn1C24StatusFixed) & \
                                CQ_CAM_CTL_BIT_FIXED(EN1,C02,this->isEn1C02StatusFixed) & \
                                CQ_CAM_CTL_BIT_FIXED(EN1,CFA,this->isEn1CfaStatusFixed) & \
                                CQ_CAM_CTL_BIT_FIXED(EN1,HRZ,this->isEn1HrzStatusFixed) & \
                                CQ_CAM_CTL_BIT_FIXED(EN1,MFB,this->isEn1MfbStatusFixed) & \
                                CQ_CAM_CTL_BIT_FIXED(EN1,AAA_GROP,this->isEn1AaaGropStatusFixed));
        }
        else
        {
            //clk enable
            ISP_WRITE_REG(m_pIspReg,CAM_CTL_CLK_EN, 0x0001FFFF);
            //continuous mode
            ISP_WRITE_BITS(m_pIspReg, CAM_TG_VF_CON, SINGLE_MODE, this->b_continuous?0:1);
            //DONOT clear unp_en/
            ISP_WRITE_ENABLE_REG(m_pIspReg, CAM_CTL_EN1_CLR,
                            ( CQ_CAM_CTL_CAN_BE_CLR_BITS(EN1,this->isIspOn)&(~0x00100000) )&(~en_Top.enable1)& \
                            CQ_CAM_CTL_BIT_FIXED(EN1,C24,this->isEn1C24StatusFixed) & \
                            CQ_CAM_CTL_BIT_FIXED(EN1,C02,this->isEn1C02StatusFixed) & \
                            CQ_CAM_CTL_BIT_FIXED(EN1,CFA,this->isEn1CfaStatusFixed) & \
                            CQ_CAM_CTL_BIT_FIXED(EN1,HRZ,this->isEn1HrzStatusFixed) & \
                            CQ_CAM_CTL_BIT_FIXED(EN1,MFB,this->isEn1MfbStatusFixed) & \
                            CQ_CAM_CTL_BIT_FIXED(EN1,AAA_GROP,this->isEn1AaaGropStatusFixed));
        }
        //control IMGO/IMG2O all the time

        //DONOT clear IMGI/IMGCI/VIPI/VIP2I
        reg_tmp = (CQ_CAM_CTL_CAN_BE_CLR_BITS(DMA_EN,this->isIspOn)&(~0x00006180)) & (~dmaMerg) & \
                CQ_CAM_CTL_BIT_FIXED(DMA,IMG2O,this->isImg2oStatusFixed) & \
                CQ_CAM_CTL_BIT_FIXED(DMA,AAO,this->isAaoStatusFixed) & \
                CQ_CAM_CTL_BIT_FIXED(DMA,ESFKO,this->isEsfkoStatusFixed) & \
                CQ_CAM_CTL_BIT_FIXED(DMA,FLKI,this->isFlkiStatusFixed) & \
                CQ_CAM_CTL_BIT_FIXED(DMA,LCSO,this->isLcsoStatusFixed);
        
        if(m_pIspDrv->IsReadOnlyMode())
        {
            ISP_IOCTL_WRITE_ENABLE_REG(m_pIspDrv, m_pIspDrv->getRegAddrMap(),  CAM_CTL_DMA_EN_CLR, reg_tmp);
            //
            ISP_IOCTL_WRITE_ENABLE_REG(m_pIspDrv, m_pIspDrv->getRegAddrMap(),  CAM_CTL_EN2_CLR, CQ_CAM_CTL_CAN_BE_CLR_BITS(EN2,this->isIspOn)& \
                CQ_CAM_CTL_BIT_FIXED(EN2,CDRZ,this->isEn2CdrzStatusFixed) & \
                CQ_CAM_CTL_BIT_FIXED(EN2,G2C,this->isEn2G2cStatusFixed) & \
                CQ_CAM_CTL_BIT_FIXED(EN2,C42,this->isEn2C42StatusFixed) & \
                (~en_Top.enable2));

            //DONOT change PASS2 field.
            ISP_IOCTL_WRITE_REG(m_pIspDrv, m_pIspDrv->getRegAddrMap(),  CAM_CTL_FMT_SEL_CLR,
                            CQ_CAM_CTL_CAN_BE_CLR_BITS(FMT_SEL,this->isIspOn)&0xFFFFF0FF & (~fmt_sel.reg_val) & CQ_CAM_CTL_BIT_FIXED(FMT,SCENARIO,this->isConcurrency) & CQ_CAM_CTL_BIT_FIXED(FMT,SUB_MODE,this->isConcurrency));
            //
            ISP_IOCTL_WRITE_ENABLE_REG(m_pIspDrv, m_pIspDrv->getRegAddrMap(),  CAM_CTL_SEL_CLR, (~ctl_sel.reg_val)&0x00000100); //CLR DISP_VID_SEL/TG_SEL only
            //
            ISP_IOCTL_WRITE_REG(m_pIspDrv, m_pIspDrv->getRegAddrMap(),  CAM_CTL_MUX_SEL_CLR, ( CQ_CAM_CTL_CAN_BE_CLR_BITS(MUX_SEL,this->isIspOn)&(~0x20018000) )&(~ctl_mux_sel.reg_val));
            //
            ISP_IOCTL_WRITE_REG(m_pIspDrv, m_pIspDrv->getRegAddrMap(),  CAM_CTL_MUX_SEL2_CLR, (this->isIspOn?(~0xBF47E083):0) & (~ctl_mux_sel2.reg_val));
            //
            ISP_IOCTL_WRITE_REG(m_pIspDrv, m_pIspDrv->getRegAddrMap(),  CAM_CTL_SRAM_MUX_CFG_CLR, ( CQ_CAM_CTL_CAN_BE_CLR_BITS(SRAM_MUX_CFG,this->isIspOn)&(~0x00FFFFFF) )&(~ctl_sram_mux_cfg.reg_val));

            ISP_IOCTL_WRITE_ENABLE_REG(m_pIspDrv, m_pIspDrv->getRegAddrMap(),  CAM_CTL_EN1_SET, en_Top.enable1);
            ISP_IOCTL_WRITE_ENABLE_REG(m_pIspDrv, m_pIspDrv->getRegAddrMap(),  CAM_CTL_EN2_SET, en_Top.enable2);
            ISP_IOCTL_WRITE_ENABLE_REG(m_pIspDrv, m_pIspDrv->getRegAddrMap(),  CAM_CTL_DMA_EN_SET, dmaMerg);

            ISP_FUNC_INF("[Pass1]0 EN1-Set(0x%x),Clr(0x%x)-En2:Set(0x%x),Clr(0x%x)-DMA:Set(0x%x),Clr(0x%x)\n", \
                    ISP_IOCTL_READ_REG(m_pIspDrv, m_pIspDrv->getRegAddrMap(),  CAM_CTL_EN1_SET),ISP_IOCTL_READ_REG(m_pIspDrv, m_pIspDrv->getRegAddrMap(),  CAM_CTL_EN1_CLR), \
                    ISP_IOCTL_READ_REG(m_pIspDrv, m_pIspDrv->getRegAddrMap(),  CAM_CTL_EN2_SET),ISP_IOCTL_READ_REG(m_pIspDrv, m_pIspDrv->getRegAddrMap(),  CAM_CTL_EN2_CLR), \
                    ISP_IOCTL_READ_REG(m_pIspDrv, m_pIspDrv->getRegAddrMap(),  CAM_CTL_DMA_EN_SET),ISP_IOCTL_READ_REG(m_pIspDrv, m_pIspDrv->getRegAddrMap(),  CAM_CTL_DMA_EN_CLR));
        }
        else
        {
            ISP_WRITE_ENABLE_REG(m_pIspReg,  CAM_CTL_DMA_EN_CLR, reg_tmp);
            //
            ISP_WRITE_ENABLE_REG(m_pIspReg,  CAM_CTL_EN2_CLR, CQ_CAM_CTL_CAN_BE_CLR_BITS(EN2,this->isIspOn)& \
                CQ_CAM_CTL_BIT_FIXED(EN2,CDRZ,this->isEn2CdrzStatusFixed) & \
                CQ_CAM_CTL_BIT_FIXED(EN2,G2C,this->isEn2G2cStatusFixed) & \
                CQ_CAM_CTL_BIT_FIXED(EN2,C42,this->isEn2C42StatusFixed) & \
                (~en_Top.enable2));

            //DONOT change PASS2 field.
            ISP_WRITE_REG(m_pIspReg,  CAM_CTL_FMT_SEL_CLR,
                            CQ_CAM_CTL_CAN_BE_CLR_BITS(FMT_SEL,this->isIspOn)&0xFFFFF0FF & (~fmt_sel.reg_val) & CQ_CAM_CTL_BIT_FIXED(FMT,SCENARIO,this->isConcurrency) & CQ_CAM_CTL_BIT_FIXED(FMT,SUB_MODE,this->isConcurrency));
            //
            ISP_WRITE_ENABLE_REG(m_pIspReg,  CAM_CTL_SEL_CLR, (~ctl_sel.reg_val)&0x00000100); //CLR DISP_VID_SEL/TG_SEL only
            //
            ISP_WRITE_REG(m_pIspReg,  CAM_CTL_MUX_SEL_CLR, ( CQ_CAM_CTL_CAN_BE_CLR_BITS(MUX_SEL,this->isIspOn)&(~0x20018000) )&(~ctl_mux_sel.reg_val));
            //
            ISP_WRITE_REG(m_pIspReg,  CAM_CTL_MUX_SEL2_CLR, (this->isIspOn?(~0xBF47E083):0) & (~ctl_mux_sel2.reg_val));
            //
            ISP_WRITE_REG(m_pIspReg,  CAM_CTL_SRAM_MUX_CFG_CLR, ( CQ_CAM_CTL_CAN_BE_CLR_BITS(SRAM_MUX_CFG,this->isIspOn)&(~0x00FFFFFF) )&(~ctl_sram_mux_cfg.reg_val));

            ISP_WRITE_ENABLE_REG(m_pIspReg,  CAM_CTL_EN1_SET, en_Top.enable1);
            ISP_WRITE_ENABLE_REG(m_pIspReg,  CAM_CTL_EN2_SET, en_Top.enable2);
            ISP_WRITE_ENABLE_REG(m_pIspReg,  CAM_CTL_DMA_EN_SET, dmaMerg);

            ISP_FUNC_INF("[Pass1] EN1:Set(0x%08x),Clr(0x%08x),En2:Set(0x%08x),Clr(0x%08x),DMA:Set(0x%08x),Clr(0x%08x)\n", \
                    ISP_READ_REG(m_pIspReg,  CAM_CTL_EN1_SET),ISP_READ_REG(m_pIspReg,  CAM_CTL_EN1_CLR), \
                    ISP_READ_REG(m_pIspReg,  CAM_CTL_EN2_SET),ISP_READ_REG(m_pIspReg,  CAM_CTL_EN2_CLR), \
                    ISP_READ_REG(m_pIspReg,  CAM_CTL_DMA_EN_SET),ISP_READ_REG(m_pIspReg,  CAM_CTL_DMA_EN_CLR));

        }
    }
    else /*if (ISP_PASS2 == this->path)*/ 
    {
        if(m_pIspDrv->IsReadOnlyMode())
        {
            /*** tpipe processing ***/
            /*** SHOULD BE AFTER CQ setting ***/
            ISP_IOCTL_WRITE_REG(m_pIspDrv, m_pIspDrv->getRegAddrMap(),CAM_CTL_TCM_EN, this->tcm_en);
            
            //IMGO_CROP_EN always ON in pass2.by TC comment
            //ISP_IOCTL_WRITE_REG(m_pIspDrv, m_pIspDrv->getRegAddrMap(),CAM_CTL_TPIPE, 0x0000004F);  // hardware do not read both tpipe width and height
            ISP_IOCTL_WRITE_REG(m_pIspDrv, m_pIspDrv->getRegAddrMap(),CAM_CTL_TILE, 0x7A03001F); // TEST_MDP for  // hardware do not read both tpipe width and height  ////from IspStreamCase0() utility.cpp ISP_BITS(pUtilityIspReg, CAM_CTL_TILE, IMGO_CROP_EN) = 0x0;	/* 0x15004050 [6:6] 0x1 */
            //ISP_IOCTL_WRITE_REG(m_pPhyIspReg, m_pIspDrv->getRegAddrMap(),CAM_CTL_TILE, 0x7A03001E); // TEST_MDP for  // hardware do not read both tpipe width and height  ////from IspStreamCase0() utility.cpp ISP_BITS(pUtilityIspReg, CAM_CTL_TILE, IMGO_CROP_EN) = 0x0;	/* 0x15004050 [6:6] 0x1 */

            //DONOT clear pak_en/pak2_en
            ISP_IOCTL_WRITE_ENABLE_REG(m_pIspDrv, m_pIspDrv->getRegAddrMap(),  CAM_CTL_EN1_CLR, \
                            (((CQ_CAM_CTL_CAN_BE_CLR_BITS(EN1,this->isIspOn)&(~0x00003000) )&(~en_Top.enable1)) & \
                            CQ_CAM_CTL_BIT_FIXED(EN1,C24,this->isEn1C24StatusFixed) & \
                            CQ_CAM_CTL_BIT_FIXED(EN1,C02,this->isEn1C02StatusFixed) & \
                            CQ_CAM_CTL_BIT_FIXED(EN1,CFA,this->isEn1CfaStatusFixed) & \
                            CQ_CAM_CTL_BIT_FIXED(EN1,HRZ,this->isEn1HrzStatusFixed) & \
                            CQ_CAM_CTL_BIT_FIXED(EN1,MFB,this->isEn1MfbStatusFixed) & \
                            CQ_CAM_CTL_BIT_FIXED(EN1,AAA_GROP,this->isEn1AaaGropStatusFixed))|((~turningEn1) & turnEn1Tag));
        }
        else
        {
            /*** tpipe processing ***/
            /*** SHOULD BE AFTER CQ setting ***/
            ISP_WRITE_REG(m_pIspReg,CAM_CTL_TCM_EN, this->tcm_en);
//TEST 0418            ISP_WRITE_REG(m_pPhyIspReg,CAM_CTL_TCM_EN, this->tcm_en);
            //
            //IMGO_CROP_EN always ON in pass2.by TC comment
            //ISP_WRITE_REG(m_pIspReg,CAM_CTL_TPIPE, 0x0000004F);  // hardware do not read both tpipe width and height
            ISP_WRITE_REG(m_pIspReg,CAM_CTL_TILE, 0x7A03001F);  // TEST_MDP // hardware do not read both tpipe width and height
//TEST 0418            ISP_WRITE_REG(m_pPhyIspReg ,CAM_CTL_TILE, 0x7A03001E);  // TEST_MDP // hardware do not read both tpipe width and height

            //DONOT clear pak_en/pak2_en
            ISP_WRITE_ENABLE_REG(m_pIspReg,  CAM_CTL_EN1_CLR, \
                            (((CQ_CAM_CTL_CAN_BE_CLR_BITS(EN1,this->isIspOn)&(~0x00003000) )&(~en_Top.enable1)) & \
                            CQ_CAM_CTL_BIT_FIXED(EN1,C24,this->isEn1C24StatusFixed) & \
                            CQ_CAM_CTL_BIT_FIXED(EN1,C02,this->isEn1C02StatusFixed) & \
                            CQ_CAM_CTL_BIT_FIXED(EN1,CFA,this->isEn1CfaStatusFixed) & \
                            CQ_CAM_CTL_BIT_FIXED(EN1,HRZ,this->isEn1HrzStatusFixed) & \
                            CQ_CAM_CTL_BIT_FIXED(EN1,MFB,this->isEn1MfbStatusFixed) & \
                            CQ_CAM_CTL_BIT_FIXED(EN1,AAA_GROP,this->isEn1AaaGropStatusFixed))|((~turningEn1) & turnEn1Tag));
        }
        //control CDP DMA all the time in pass2 path
        //DONOT clear IMG2O
        //clear IMGO by flag(ip pass2 may need to control IMGO)
        reg_tmp = (CQ_CAM_CTL_CAN_BE_CLR_BITS(DMA_EN,this->isIspOn)|CQ_CAM_CTL_CAN_BE_CLR_BITS(DMA_EN,0)|turnDmaTag) & \
                                                  (~dmaMerg) & \
                                                   CQ_CAM_CTL_BIT_FIXED(DMA,IMG2O,this->isImg2oStatusFixed) & \
                                                   CQ_CAM_CTL_BIT_FIXED(DMA,IMGO,this->isImgoStatusFixed) & \
                                                   CQ_CAM_CTL_BIT_FIXED(DMA,AAO,this->isAaoStatusFixed) & \
                                                   CQ_CAM_CTL_BIT_FIXED(DMA,ESFKO,this->isEsfkoStatusFixed) & \
                                                   CQ_CAM_CTL_BIT_FIXED(DMA,FLKI,this->isFlkiStatusFixed) & \
                                                   CQ_CAM_CTL_BIT_FIXED(DMA,LCSO,this->isLcsoStatusFixed);
        if(m_pIspDrv->IsReadOnlyMode())
        {
            ISP_IOCTL_WRITE_ENABLE_REG(m_pIspDrv, m_pIspDrv->getRegAddrMap(),  CAM_CTL_DMA_EN_CLR, reg_tmp);

            //in pass2 , CURZ should be able to disable.
            ISP_IOCTL_WRITE_ENABLE_REG(m_pIspDrv, m_pIspDrv->getRegAddrMap(),  CAM_CTL_EN2_CLR, \
                            ((CQ_CAM_CTL_CAN_BE_CLR_BITS(EN2,this->isIspOn)|(this->isIspOn?0x00040000:0)) & (~en_Top.enable2) & \
                            CQ_CAM_CTL_BIT_FIXED(EN2,G2C,this->isEn2G2cStatusFixed) & \
                            CQ_CAM_CTL_BIT_FIXED(EN2,C42,this->isEn2C42StatusFixed) & \
                            CQ_CAM_CTL_BIT_FIXED(EN2,CDRZ,this->isEn2CdrzStatusFixed)) | \
                            ((~turningEn2) & turnEn2Tag));

            //DONOT change PASS1 field.
            ISP_IOCTL_WRITE_REG(m_pIspDrv, m_pIspDrv->getRegAddrMap(),  CAM_CTL_FMT_SEL_CLR,
                            CQ_CAM_CTL_CAN_BE_CLR_BITS(FMT_SEL,this->isIspOn) & 0xFFFF0FFF & (~fmt_sel.reg_val) & CQ_CAM_CTL_BIT_FIXED(FMT,SCENARIO,this->isConcurrency) & CQ_CAM_CTL_BIT_FIXED(FMT,SUB_MODE,this->isConcurrency));
            //
            ISP_IOCTL_WRITE_ENABLE_REG(m_pIspDrv, m_pIspDrv->getRegAddrMap(),  CAM_CTL_SEL_CLR, (~ctl_sel.reg_val)&0x00800000); //CLR DISP_VID_SEL only
            //DONOT change AA_SEL/SGG_SEL/LCS_SEL
            ISP_IOCTL_WRITE_REG(m_pIspDrv, m_pIspDrv->getRegAddrMap(),  CAM_CTL_MUX_SEL_CLR, ( CQ_CAM_CTL_CAN_BE_CLR_BITS(MUX_SEL,this->isIspOn)&(~0x007000F8) )&(~ctl_mux_sel.reg_val));
            //
            ISP_IOCTL_WRITE_REG(m_pIspDrv, m_pIspDrv->getRegAddrMap(),  CAM_CTL_MUX_SEL2_CLR, (this->isIspOn?0xBF47E083:0) & (~ctl_mux_sel2.reg_val));
            //DONOT change SGG_HRZ_SEL(no EN bit)
            ISP_IOCTL_WRITE_REG(m_pIspDrv, m_pIspDrv->getRegAddrMap(),  CAM_CTL_SRAM_MUX_CFG_CLR, ( CQ_CAM_CTL_CAN_BE_CLR_BITS(SRAM_MUX_CFG,this->isIspOn)&(~0x11000000) )&(~ctl_sram_mux_cfg.reg_val));

            ISP_IOCTL_WRITE_ENABLE_REG(m_pIspDrv, m_pIspDrv->getRegAddrMap(),  CAM_CTL_EN1_SET, en_Top.enable1 | (turningEn1 & turnEn1Tag));
            ISP_IOCTL_WRITE_ENABLE_REG(m_pIspDrv, m_pIspDrv->getRegAddrMap(),  CAM_CTL_EN2_SET, en_Top.enable2 | (turningEn2 & turnEn2Tag));
            ISP_IOCTL_WRITE_ENABLE_REG(m_pIspDrv, m_pIspDrv->getRegAddrMap(),  CAM_CTL_DMA_EN_SET, dmaMerg);

            ISP_FUNC_INF("[Pass2]0 AppTurn(%d),EN1:Set(0x%08x),Clr(0x%08x),En2:Set(0x%08x),Clr(0x%08x),DMA:Set(0x%08x),Clr(0x%08x)\n", \
                    applyTurn,ISP_IOCTL_READ_REG(m_pIspDrv, m_pIspDrv->getRegAddrMap(),  CAM_CTL_EN1_SET),ISP_IOCTL_READ_REG(m_pIspDrv, m_pIspDrv->getRegAddrMap(),  CAM_CTL_EN1_CLR), \
                    ISP_IOCTL_READ_REG(m_pIspDrv, m_pIspDrv->getRegAddrMap(),  CAM_CTL_EN2_SET),ISP_IOCTL_READ_REG(m_pIspDrv, m_pIspDrv->getRegAddrMap(),  CAM_CTL_EN2_CLR), \
                    ISP_IOCTL_READ_REG(m_pIspDrv, m_pIspDrv->getRegAddrMap(),  CAM_CTL_DMA_EN_SET),ISP_IOCTL_READ_REG(m_pIspDrv, m_pIspDrv->getRegAddrMap(),  CAM_CTL_DMA_EN_CLR));
        }
        else
        {
            ISP_WRITE_ENABLE_REG(m_pIspReg,  CAM_CTL_DMA_EN_CLR, reg_tmp);

            //in pass2 , CURZ should be able to disable.
            ISP_WRITE_ENABLE_REG(m_pIspReg,  CAM_CTL_EN2_CLR,
                            ((CQ_CAM_CTL_CAN_BE_CLR_BITS(EN2,this->isIspOn)|(this->isIspOn?0x00040000:0)) & (~en_Top.enable2) & \
                            CQ_CAM_CTL_BIT_FIXED(EN2,G2C,this->isEn2G2cStatusFixed) & \
                            CQ_CAM_CTL_BIT_FIXED(EN2,C42,this->isEn2C42StatusFixed) & \
                            CQ_CAM_CTL_BIT_FIXED(EN2,CDRZ,this->isEn2CdrzStatusFixed)) | \
                            ((~turningEn2) & turnEn2Tag));
            //DONOT change PASS1 field.
            ISP_WRITE_REG(m_pIspReg,  CAM_CTL_FMT_SEL_CLR,
                            CQ_CAM_CTL_CAN_BE_CLR_BITS(FMT_SEL,this->isIspOn) & 0xFFFF0FFF & (~fmt_sel.reg_val) & CQ_CAM_CTL_BIT_FIXED(FMT,SCENARIO,this->isConcurrency) & CQ_CAM_CTL_BIT_FIXED(FMT,SUB_MODE,this->isConcurrency));
            //
            ISP_WRITE_ENABLE_REG(m_pIspReg,  CAM_CTL_SEL_CLR, (~ctl_sel.reg_val)&0x00800000); //CLR DISP_VID_SEL only
            //DONOT change AA_SEL/SGG_SEL/LCS_SEL
            ISP_WRITE_REG(m_pIspReg,  CAM_CTL_MUX_SEL_CLR, ( CQ_CAM_CTL_CAN_BE_CLR_BITS(MUX_SEL,this->isIspOn)&(~0x007000F8) )&(~ctl_mux_sel.reg_val));
            //
            ISP_WRITE_REG(m_pIspReg,  CAM_CTL_MUX_SEL2_CLR, (this->isIspOn?0xBF47E083:0) & (~ctl_mux_sel2.reg_val));
            //DONOT change SGG_HRZ_SEL(no EN bit)
            ISP_WRITE_REG(m_pIspReg,  CAM_CTL_SRAM_MUX_CFG_CLR, ( CQ_CAM_CTL_CAN_BE_CLR_BITS(SRAM_MUX_CFG,this->isIspOn)&(~0x11000000) )&(~ctl_sram_mux_cfg.reg_val));

            ISP_WRITE_ENABLE_REG(m_pIspReg,  CAM_CTL_EN1_SET, en_Top.enable1 | (turningEn1 & turnEn1Tag));
            ISP_WRITE_ENABLE_REG(m_pIspReg,  CAM_CTL_EN2_SET, en_Top.enable2 | (turningEn2 & turnEn2Tag));
            ISP_WRITE_ENABLE_REG(m_pIspReg,  CAM_CTL_DMA_EN_SET, dmaMerg);

            ISP_FUNC_INF("[Pass2]AppTurn(%d) EN1-Set(0x%x),Clr(0x%x)-En2:Set(0x%x),Clr(0x%x)-DMA:Set(0x%x),Clr(0x%x)\n", \
                    applyTurn,ISP_READ_REG(m_pIspReg,  CAM_CTL_EN1_SET),ISP_READ_REG(m_pIspReg,  CAM_CTL_EN1_CLR), \
                    ISP_READ_REG(m_pIspReg,  CAM_CTL_EN2_SET),ISP_READ_REG(m_pIspReg,  CAM_CTL_EN2_CLR), \
                    ISP_READ_REG(m_pIspReg,  CAM_CTL_DMA_EN_SET),ISP_READ_REG(m_pIspReg,  CAM_CTL_DMA_EN_CLR));

        }
    }
    ISP_FUNC_DBG("[Fixed][En1]c24(%d),mfb(%d),c02(%d),cfa(%d),hrz(%d)",
            this->isEn1C24StatusFixed,this->isEn1MfbStatusFixed,this->isEn1C02StatusFixed,this->isEn1CfaStatusFixed,this->isEn1HrzStatusFixed);

    ISP_FUNC_DBG("[Fixed][En2]Cdrz(%d),G2C(%d),C42(%d)",this->isEn2CdrzStatusFixed,this->isEn2G2cStatusFixed,this->isEn2C42StatusFixed);

    ISP_FUNC_DBG("[Fixed]isEn1AGrp(%d) isIspOn(%d) Img2o(%d) Imgo(%d) Aao(%d) Esfko(%d) Flki(%d) Lcso(%d) CleanReg(0x%x) isRo(%d)\n",this->isIspOn, \
            this->isEn1AaaGropStatusFixed,this->isImg2oStatusFixed,this->isImgoStatusFixed,this->isAaoStatusFixed, \
            this->isEsfkoStatusFixed,this->isFlkiStatusFixed,this->isLcsoStatusFixed, reg_tmp, m_pIspDrv->IsReadOnlyMode());

    /********** control by set/clr register ************/
    //ISP_REG(m_pIspReg,  CAM_CTL_DMA_EN_SET)     = en_Top.dma;
    //FMT sel  dont' care tg relative

    if(m_pIspDrv->IsReadOnlyMode())
    {
        ISP_IOCTL_WRITE_REG(m_pIspDrv, m_pIspDrv->getRegAddrMap(),  CAM_CTL_FMT_SEL_SET, ( fmt_sel.reg_val&(~0xFFFF0000) ));
        ISP_IOCTL_WRITE_ENABLE_REG(m_pIspDrv, m_pIspDrv->getRegAddrMap(),  CAM_CTL_SEL_SET, ctl_sel.reg_val);
        ISP_IOCTL_WRITE_REG(m_pIspDrv, m_pIspDrv->getRegAddrMap(),  CAM_CTL_MUX_SEL_SET, ctl_mux_sel.reg_val);
        ISP_IOCTL_WRITE_REG(m_pIspDrv, m_pIspDrv->getRegAddrMap(),  CAM_CTL_MUX_SEL2_SET, ctl_mux_sel2.reg_val);
        ISP_IOCTL_WRITE_REG(m_pIspDrv, m_pIspDrv->getRegAddrMap(),  CAM_CTL_SRAM_MUX_CFG_SET, ctl_sram_mux_cfg.reg_val);

        //bit wise
        ISP_IOCTL_WRITE_BITS(m_pIspDrv, m_pIspDrv->getRegAddrMap(), CAM_CTL_PIX_ID_SET, PIX_ID_SET, pix_id);        
        
        ISP_IOCTL_WRITE_BITS(m_pIspDrv, m_pIspDrv->getRegAddrMap(), CAM_CTL_PIX_ID_SET, MDP_SEL_SET, 0x1);
        ISP_IOCTL_WRITE_REG(m_pIspDrv, m_pIspDrv->getRegAddrMap(),  CAM_CTL_PIX_ID_CLR, CQ_CAM_CTL_CAN_BE_CLR_BITS(PIX_ID,this->isIspOn)&(~pix_id));

        ISP_FUNC_DBG("CAM_CTL_PIX_ID_SET(0x%8x),CAM_CTL_PIX_ID(0x%8x)",ISP_IOCTL_READ_REG(m_pIspDrv, m_pIspDrv->getRegAddrMap(),  CAM_CTL_PIX_ID_SET));
    }
    else
    {
        ISP_WRITE_REG(m_pIspReg,  CAM_CTL_FMT_SEL_SET, ( fmt_sel.reg_val&(~0xFFFF0000) ));
        ISP_WRITE_ENABLE_REG(m_pIspReg,  CAM_CTL_SEL_SET, ctl_sel.reg_val);
        ISP_WRITE_REG(m_pIspReg,  CAM_CTL_MUX_SEL_SET, ctl_mux_sel.reg_val);
        ISP_WRITE_REG(m_pIspReg,  CAM_CTL_MUX_SEL2_SET, ctl_mux_sel2.reg_val);
        ISP_WRITE_REG(m_pIspReg,  CAM_CTL_SRAM_MUX_CFG_SET, ctl_sram_mux_cfg.reg_val);
        
        //bit wise
        ISP_WRITE_BITS(m_pIspReg, CAM_CTL_PIX_ID_SET, PIX_ID_SET, pix_id);
//TEST 0418        ISP_WRITE_BITS(m_pIspReg, CAM_CTL_PIX_ID_SET, CDP_EDGE_SET, 0xE); // TEST_MDP
        
        ISP_WRITE_BITS(m_pIspReg, CAM_CTL_PIX_ID_SET, MDP_SEL_SET, 0x1);
        ISP_WRITE_REG(m_pIspReg,  CAM_CTL_PIX_ID_CLR, CQ_CAM_CTL_CAN_BE_CLR_BITS(PIX_ID,this->isIspOn)&(~pix_id));

        ISP_FUNC_DBG("CAM_CTL_PIX_ID_SET(0x%8x),CAM_CTL_PIX_ID(0x%8x)",ISP_READ_REG(m_pIspReg,  CAM_CTL_PIX_ID_SET));
    }
    //
    ISP_FUNC_DBG("ISP_TOP_CTRL::_config:X ");
    return 0;
}


int ISP_TOP_CTRL::_enable( void* pParam  )
{
    ISP_FUNC_DBG("path(%d),start(%d),m_pIspReg(0x%x),m_pPhyIspReg(0x%x),scenario(%d)\n",
                this->path,*((MUINT32*)pParam),this->m_pIspReg,this->m_pPhyIspReg,this->fmt_sel.bit_field.scenario);

    MUINT32 *start = (MUINT32*)pParam;
    ISP_BUFFER_CTRL_STRUCT buf_ctrl;
    MUINT32 size;

    // added for checking CQ status
    MUINT32 CQ_status, CQ_debugset;
    MBOOL readCQstatus = MTRUE;  // check both pass1 and pass2
    if( CAM_ISP_PASS2_START == *start || CAM_ISP_PASS2B_START == *start || CAM_ISP_PASS2C_START == *start )
    {
        if(m_pPhyIspDrv->IsReadOnlyMode())
        {
            ISP_IOCTL_WRITE_REG(m_pPhyIspDrv,m_pPhyIspDrv->getRegAddrMap(),CAM_CTL_DBG_SET,0x6000);
        }
        else
        {
            ISP_WRITE_REG(this->m_pPhyIspReg,CAM_CTL_DBG_SET,0x6000);
        }
        readCQstatus = MTRUE;
    }

    if (this->m_pIspDrvShell->m_pIMemDrv)    // If this->m_pIspDrvShell->m_pIMemDrv not null (i.e. not GDMA mode). // No IMem in GDMA mode so don't have to flush.
    {
        //m4u flush before HW working
        this->m_pIspDrvShell->m_pIMemDrv->cacheFlushAll();
    }
    
    if( readCQstatus )
    {
        if(m_pPhyIspDrv->IsReadOnlyMode())
        {
            CQ_status = ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CTL_DBG_PORT );
            CQ_debugset = ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CTL_DBG_SET );
        }
        else
        {
            CQ_status = ISP_READ_REG(m_pPhyIspReg, CAM_CTL_DBG_PORT );
            CQ_debugset = ISP_READ_REG(m_pPhyIspReg, CAM_CTL_DBG_SET );
        }
    }

    if( CAM_ISP_PASS1_START == (*start) ) 
    {
#if defined(_rtbc_use_cq0c_)
        //fbc enable before VF_EN
        //IMGO
        buf_ctrl.ctrl = ISP_RT_BUF_CTRL_GET_SIZE;
        buf_ctrl.buf_id = _imgo_;
        buf_ctrl.data_ptr = (MUINT32)&size;
        ISP_FUNC_INF("ctrl(%d),id(%d),ptr(0x%x)",buf_ctrl.ctrl,buf_ctrl.buf_id,buf_ctrl.data_ptr);
        if ( MTRUE != this->m_pIspDrvShell->getPhyIspDrv()->rtBufCtrl((void*)&buf_ctrl) ) 
        {
            ISP_FUNC_ERR("ERROR:rtBufCtrl");
            return -1;
        }
        ISP_FUNC_INF("dma(%d),size(%d)",_imgo_,size);
        if(m_pPhyIspDrv->IsReadOnlyMode())
        {
            ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CTL_IMGO_FBC, DROP_INT_EN, 1);

            ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CTL_IMGO_FBC, FB_NUM, size);
            //
            ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CTL_IMGO_FBC, FBC_EN, 0);
            usleep(10);
            ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CTL_IMGO_FBC, FBC_EN, 1);
        }
        else
        {
            ISP_WRITE_BITS(this->m_pPhyIspReg, CAM_CTL_IMGO_FBC, DROP_INT_EN, 1);

            ISP_WRITE_BITS(this->m_pPhyIspReg, CAM_CTL_IMGO_FBC, FB_NUM, size);
            //
            ISP_WRITE_BITS(this->m_pPhyIspReg, CAM_CTL_IMGO_FBC, FBC_EN, 0);
            usleep(10);
            ISP_WRITE_BITS(this->m_pPhyIspReg, CAM_CTL_IMGO_FBC, FBC_EN, 1);
        }

        //IMG2O
        if ( CAM_CTL_DMA_EN_IMG2O_EN & this->en_Top.dma ) 
        {
            buf_ctrl.buf_id = _img2o_;
            buf_ctrl.data_ptr = (MUINT32)&size;
            ISP_FUNC_INF("ctrl(%d),id(%d),ptr(0x%x)",buf_ctrl.ctrl,buf_ctrl.buf_id,buf_ctrl.data_ptr);
            if ( MTRUE != this->m_pIspDrvShell->getPhyIspDrv()->rtBufCtrl((void*)&buf_ctrl) ) {
                ISP_FUNC_ERR("ERROR:rtBufCtrl");
                return -1;
            }
            ISP_FUNC_INF("dma(%d),size(%d)",_img2o_,size);
            if(m_pPhyIspDrv->IsReadOnlyMode())
            {
                ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CTL_IMG2O_FBC, DROP_INT_EN, 1);

                ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CTL_IMG2O_FBC, FB_NUM, size);
                //
                ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CTL_IMG2O_FBC, FBC_EN, 0);
                usleep(10);
                ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CTL_IMG2O_FBC, FBC_EN, 1);
            }
            else
            {
                ISP_WRITE_BITS(this->m_pPhyIspReg, CAM_CTL_IMG2O_FBC, DROP_INT_EN, 1);

                ISP_WRITE_BITS(this->m_pPhyIspReg, CAM_CTL_IMG2O_FBC, FB_NUM, size);
                //
                ISP_WRITE_BITS(this->m_pPhyIspReg, CAM_CTL_IMG2O_FBC, FBC_EN, 0);
                usleep(10);
                ISP_WRITE_BITS(this->m_pPhyIspReg, CAM_CTL_IMG2O_FBC, FBC_EN, 1);
            }
        }

        //SENINF1_INT_EN
        this->m_pIspDrvShell->getPhyIspDrv()->writeReg(0x00008014, 0x0000007F);    /* 0x150048A0: CAM_CFA_BYPASS */

#if 0
        //DMA error en
        this->m_pIspDrvShell->getPhyIspDrv()->writeReg(0x000043A8, 0xFFFF0000 ); //IMGI
    	this->m_pIspDrvShell->getPhyIspDrv()->writeReg(0x000043AC, 0xFFFF0000 ); //IMGCI
    	this->m_pIspDrvShell->getPhyIspDrv()->writeReg(0x000043B0, 0xFFFF0000 ); //LSCI
    	this->m_pIspDrvShell->getPhyIspDrv()->writeReg(0x000043B4, 0xFFFF0000 ); //FLKI
    	this->m_pIspDrvShell->getPhyIspDrv()->writeReg(0x000043B8, 0xFFFF0000 ); //LCEI
    	this->m_pIspDrvShell->getPhyIspDrv()->writeReg(0x000043BC, 0xFFFF0000 ); //VIPI
    	this->m_pIspDrvShell->getPhyIspDrv()->writeReg(0x000043C0, 0xFFFF0000 ); //VIP2I
    	this->m_pIspDrvShell->getPhyIspDrv()->writeReg(0x000043C4, 0xFFFF0000 ); //IMGO
    	this->m_pIspDrvShell->getPhyIspDrv()->writeReg(0x000043C8, 0xFFFF0000 ); //IMG2O
    	this->m_pIspDrvShell->getPhyIspDrv()->writeReg(0x000043CC, 0xFFFF0000 ); //LCSO
    	this->m_pIspDrvShell->getPhyIspDrv()->writeReg(0x000043D0, 0xFFFF0000 ); //ESFKO
    	this->m_pIspDrvShell->getPhyIspDrv()->writeReg(0x000043D4, 0xFFFF0000 ); //AAO

        //CSI2 //allan
        this->m_pIspDrvShell->getPhyIspDrv()->writeReg(0x00008108,
            ( this->m_pIspDrvShell->getPhyIspDrv()->readReg(0x00008108) & 0xFFFFFFF0 ) | 0x7 );
        this->m_pIspDrvShell->getPhyIspDrv()->writeReg(0x0000810C,
              this->m_pIspDrvShell->getPhyIspDrv()->readReg(0x0000810C) & ~(0x1 << 4) );
#endif

        //SENINF1_CTRL //allan
        //FIFO overrun reset enable
        //this->m_pIspDrvShell->getPhyIspDrv()->writeReg(0x00008010,
              //this->m_pIspDrvShell->getPhyIspDrv()->readReg(0x00008010) | (0x1 << 11) );

#if defined(_rtbc_use_cq0c_)

        ISP_FUNC_DBG("FBC uses CQ0C");

        //enable CQ0C for rtbc
        if( 1 < size ) 
        {
            ISP_FUNC_DBG("ENABLE CAM_ISP_CQ0C(0x%x)",this->m_pIspDrvShell->getPhyIspDrv()->getCQDescBufPhyAddr(ISP_DRV_CQ0C)+ sizeof(CQ_RING_CMD_ST));
            if(m_pPhyIspDrv->IsReadOnlyMode())
            {
                ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(),CAM_CTL_CQ0C_BASEADDR,
                                (unsigned int)( this->m_pIspDrvShell->getPhyIspDrv()->getCQDescBufPhyAddr(ISP_DRV_CQ0C) + sizeof(CQ_RING_CMD_ST) ));
            }
            else
            {
                ISP_WRITE_REG(this->m_pPhyIspReg,CAM_CTL_CQ0C_BASEADDR,
                                (unsigned int)( this->m_pIspDrvShell->getPhyIspDrv()->getCQDescBufPhyAddr(ISP_DRV_CQ0C) + sizeof(CQ_RING_CMD_ST) ));
            }
            //this->m_pIspDrvShell->cam_cq_cfg(CAM_ISP_CQ0C);
        }
        else 
        {
            ISP_FUNC_DBG("ENABLE CAM_ISP_CQ0C(0x%x)",this->m_pIspDrvShell->getPhyIspDrv()->getCQDescBufPhyAddr(ISP_DRV_CQ0C));
            if(m_pPhyIspDrv->IsReadOnlyMode())
            {
                ISP_IOCTL_WRITE_REG(m_pPhyIspDrv,m_pPhyIspDrv->getRegAddrMap(),CAM_CTL_CQ0C_BASEADDR,
                                (unsigned int)( this->m_pIspDrvShell->getPhyIspDrv()->getCQDescBufPhyAddr(ISP_DRV_CQ0C) ));
            }
            else
            {
                ISP_WRITE_REG(this->m_pPhyIspReg,CAM_CTL_CQ0C_BASEADDR,
                                (unsigned int)( this->m_pIspDrvShell->getPhyIspDrv()->getCQDescBufPhyAddr(ISP_DRV_CQ0C) ));
            }
        }
#endif

        //
        if(m_pPhyIspDrv->IsReadOnlyMode())
        {
            MUINT32 imgoFbNum = 0,img2oFbNum = 0;
            ISP_IOCTL_READ_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CTL_IMGO_FBC, FB_NUM, imgoFbNum);
            ISP_IOCTL_READ_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CTL_IMG2O_FBC, FB_NUM, img2oFbNum);
            ISP_FUNC_DBG("FB_NUM:(%d/%d)",imgoFbNum,img2oFbNum);
        }
        else
        {
            ISP_FUNC_DBG("FB_NUM:(%d/%d)",ISP_READ_BITS(this->m_pPhyIspReg, CAM_CTL_IMGO_FBC, FB_NUM),ISP_READ_BITS(this->m_pPhyIspReg, CAM_CTL_IMG2O_FBC, FB_NUM));
        }

#else

        ISP_FUNC_DBG("FBC uses ISR");

        if(m_pPhyIspDrv->IsReadOnlyMode())
        {
            ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CTL_IMGO_FBC, FBC_EN, 0);
        }
        else
        {
            ISP_WRITE_BITS(this->m_pPhyIspReg, CAM_CTL_IMGO_FBC, FBC_EN, 0);
        }
        usleep(10);
#endif

        //TG1
        if(CAM_CTL_EN1_TG1_EN & this->en_Top.enable1)
        {
            if(m_pPhyIspDrv->IsReadOnlyMode())
            {
                ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_TG_VF_CON, VFDATA_EN, 1);
            }
            else
            {
                ISP_WRITE_BITS(m_pPhyIspReg, CAM_TG_VF_CON, VFDATA_EN, 1);
            }
        }
        
        //TG2
        if(CAM_CTL_EN1_TG2_EN & this->en_Top.enable1) 
        {
            if(m_pPhyIspDrv->IsReadOnlyMode())
            {
                //ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_TG2_VF_CON, VFDATA_EN, 1);
            }
            else
            {
                //ISP_WRITE_BITS(m_pPhyIspReg, CAM_TG2_VF_CON, VFDATA_EN, 1);
            }
        }

    }
    else if(CAM_ISP_PASS2_START == *start) 
    {
        if((CQ_status & 0x0000000F) != 0x001)
        {
            ISP_FUNC_ERR("CQ1 not idle: dbg(0x%08x 0x%08x)", CQ_debugset, CQ_status);
        }
        
        if(m_pPhyIspDrv->IsReadOnlyMode())
        {
            // remove for MDP trigger            ISP_IOCTL_WRITE_ENABLE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CTL_START, PASS2_START, 1);

            if(MDPMGR_NO_ERROR != g_pMdpMgr->startMdp(g_MdpMgrCfgData[CAM_ISP_CQ1]))
            {
                ISP_FUNC_ERR("pass2 CQ1 g_pMdpMgr->startMdp fail(1)");
                return -1;
            }
        }
        else
        {
            // remove for MDP trigger            ISP_WRITE_ENABLE_BITS(m_pPhyIspReg, CAM_CTL_START, PASS2_START, 1);

            if(MDPMGR_NO_ERROR != g_pMdpMgr->startMdp(g_MdpMgrCfgData[CAM_ISP_CQ1]))
            {
                ISP_FUNC_ERR("pass2 CQ1 g_pMdpMgr->startMdp fail(2)");
                return -1;
            }
        }    
    }
    else if(CAM_ISP_PASS2B_START == *start) 
    {
        if((CQ_status & 0x000000F0) != 0x010)
        {
            ISP_FUNC_ERR("CQ2 not idle: dbg(0x%08x 0x%08x)", CQ_debugset, CQ_status);
        }
        
        if(m_pPhyIspDrv->IsReadOnlyMode())
        {
            // remove for MDP trigger             ISP_IOCTL_WRITE_ENABLE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CTL_START, PASS2B_START, 1);
            
            if(MDPMGR_NO_ERROR != g_pMdpMgr->startMdp(g_MdpMgrCfgData[CAM_ISP_CQ2]))
            {
                ISP_FUNC_ERR("pass2 CQ2 g_pMdpMgr->startMdp fail(1)");
                return -1;
            }
        }
        else
        {
            // remove for MDP trigger             ISP_WRITE_ENABLE_BITS(m_pPhyIspReg, CAM_CTL_START, PASS2B_START, 1);
            
            if(MDPMGR_NO_ERROR != g_pMdpMgr->startMdp(g_MdpMgrCfgData[CAM_ISP_CQ2]))
            {
                ISP_FUNC_ERR("pass2 CQ2 g_pMdpMgr->startMdp fail(2)");
                return -1;
            }
        }
    }
    else if(CAM_ISP_PASS2C_START == *start) 
    {
        if((CQ_status & 0x00000F00) != 0x100)
        {
            ISP_FUNC_ERR("CQ3 not idle: dbg(0x%08x 0x%08x)", CQ_debugset, CQ_status);
        }
        if(m_pPhyIspDrv->IsReadOnlyMode())
        {
            // remove for MDP trigger             ISP_IOCTL_WRITE_ENABLE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CTL_START, PASS2C_START, 1);
        }
        else
        {
            // remove for MDP trigger             ISP_WRITE_ENABLE_BITS(m_pPhyIspReg, CAM_CTL_START, PASS2C_START, 1);
        }
    }
    else if(CAM_ISP_FMT_START == *start) 
    {
        if(m_pPhyIspDrv->IsReadOnlyMode())
        {
            ISP_IOCTL_WRITE_ENABLE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(),  CAM_CTL_START, 0x00000008);  // FMT_START
        }
        else
        {
            ISP_WRITE_ENABLE_REG(m_pPhyIspReg,  CAM_CTL_START, 0x00000008);
        }
    }
    else if(CAM_ISP_PASS1_CQ0_START == *start) 
    {
        if(m_pPhyIspDrv->IsReadOnlyMode())
        {
            ISP_IOCTL_WRITE_ENABLE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(),  CAM_CTL_START, 0x00000020);  // CQ0_START
        }
        else
        {
            ISP_WRITE_ENABLE_REG(m_pPhyIspReg,  CAM_CTL_START, 0x00000020);
        }
    }
    else if(CAM_ISP_PASS1_CQ0B_START == *start) 
    {
        if(m_pPhyIspDrv->IsReadOnlyMode())
        {
            ISP_IOCTL_WRITE_ENABLE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(),  CAM_CTL_START, 0x00000040);  // CQ0B_START
        }
        else
        {
            ISP_WRITE_ENABLE_REG(m_pPhyIspReg,  CAM_CTL_START, 0x00000040);
        }
    }


#if 1
    //register dump after start()
    if ( MTRUE == function_DbgLogEnable_INFO) 
    {
        static int pass2_cnt = 0;

        ISP_FUNC_VRB("dumpReg");
        if (CAM_ISP_PASS1_START == *start) 
        {
            usleep(1000);
            //pass1 DMA base_addr
            if(m_pPhyIspDrv->IsReadOnlyMode())
            {
                ISP_FUNC_INF("P1 dma_en(0x%x),addr(0x%x)(0x%x)(0x%x)(0x%x)", \
                    ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CTL_DMA_EN ), \
                    ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_IMGO_BASE_ADDR ), \
                    ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_IMGO_OFST_ADDR ), \
                    ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_IMG2O_BASE_ADDR ), \
                    ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_IMG2O_OFST_ADDR ));
            }
            else
            {
                ISP_FUNC_INF("P1 dma_en(0x%x),addr(0x%x)(0x%x)(0x%x)(0x%x)", \
                    ISP_READ_REG(m_pPhyIspReg, CAM_CTL_DMA_EN ), \
                    ISP_READ_REG(m_pPhyIspReg, CAM_IMGO_BASE_ADDR ), \
                    ISP_READ_REG(m_pPhyIspReg, CAM_IMGO_OFST_ADDR ), \
                    ISP_READ_REG(m_pPhyIspReg, CAM_IMG2O_BASE_ADDR ), \
                    ISP_READ_REG(m_pPhyIspReg, CAM_IMG2O_OFST_ADDR ));
            }
            
            if ( MTRUE == function_DbgLogEnable_VERBOSE) 
            {
                usleep(1000);
            this->m_pIspDrvShell->m_pPhyIspDrv_bak->dumpReg();
            pass2_cnt = 0;
            }
        }
        else if ( CAM_ISP_PASS2_START == *start || CAM_ISP_PASS2B_START == *start || CAM_ISP_PASS2C_START == *start) 
        {
            //usleep(1000);
            //pass2 DMA base_addr
#if 0
            if(m_pPhyIspDrv->IsReadOnlyMode())
            {
                ISP_FUNC_DBG("P2 dma_en(0x%x),addr(0x%x)(0x%x)(0x%x)(0x%x)(0x%x)(0x%x)", \
                    ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CTL_DMA_EN ), \
                    ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_IMGI_BASE_ADDR ), \
                    ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_IMGI_OFST_ADDR ), \
                    ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_BASE_ADDR ), \
                    ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_OFST_ADDR ), \
                    ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_BASE_ADDR ), \
                    ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_OFST_ADDR ));
            }
            else
            {
                ISP_FUNC_DBG("P2 dma_en(0x%x),addr(0x%x)(0x%x)(0x%x)(0x%x)(0x%x)(0x%x)", \
                    ISP_READ_REG(m_pPhyIspReg, CAM_CTL_DMA_EN ), \
                    ISP_READ_REG(m_pPhyIspReg, CAM_IMGI_BASE_ADDR ), \
                    ISP_READ_REG(m_pPhyIspReg, CAM_IMGI_OFST_ADDR ), \
                    ISP_READ_REG(m_pPhyIspReg, CAM_VIDO_BASE_ADDR ), \
                    ISP_READ_REG(m_pPhyIspReg, CAM_VIDO_OFST_ADDR ), \
                    ISP_READ_REG(m_pPhyIspReg, CAM_DISPO_BASE_ADDR ), \
                    ISP_READ_REG(m_pPhyIspReg, CAM_DISPO_OFST_ADDR ));
            }
#endif 
            if ( 10==pass2_cnt && MTRUE == function_DbgLogEnable_VERBOSE) 
            {
                this->m_pIspDrvShell->m_pPhyIspDrv_bak->dumpReg();
            }
        }
        pass2_cnt++;
    }

#endif

    ISP_FUNC_INF("path(%d),start(%d),SCIO(%d),En1(0x%x),En2(0x%x),Dma(0x%x)",
    this->path, \
    *((MUINT32*)pParam), \
    this->fmt_sel.bit_field.scenario,
    this->m_pIspDrvShell->getPhyIspDrv()->readReg(0x00004004),
    this->m_pIspDrvShell->getPhyIspDrv()->readReg(0x00004008),
    this->m_pIspDrvShell->getPhyIspDrv()->readReg(0x0000400C));
    //

    return 0;

}

int ISP_TOP_CTRL::_disable( void )
{
    MBOOL ret = MTRUE;
    ISP_DRV_WAIT_IRQ_STRUCT irq_TG1_DONE = {ISP_DRV_IRQ_CLEAR_WAIT,
                                            ISP_DRV_IRQ_TYPE_INT,
                                            ISP_DRV_IRQ_INT_STATUS_VS1_ST,
                                            CAM_INT_WAIT_TIMEOUT_MS};
    ISP_BUFFER_CTRL_STRUCT buf_ctrl;
    MUINT32 dummy;
    MUINT32 tgStatus;

    ISP_FUNC_DBG("ISP_TOP_CTRL::_disable:E 0x%x 0x%x\n",this->m_pIspDrvShell->m_pIspDrv,this->m_pIspDrvShell->m_pPhyIspDrv_bak);

    if (ISP_PASS1 == this->path)
    {
        //TG1
        if ( CAM_CTL_EN1_TG1_EN & this->en_Top.enable1 ) 
        {
            if(m_pPhyIspDrv->IsReadOnlyMode())
            {
                ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_TG_VF_CON, VFDATA_EN, 0);
            }
            else
            {
                ISP_WRITE_BITS(m_pPhyIspReg, CAM_TG_VF_CON, VFDATA_EN, 0);
            }
            irq_TG1_DONE.Status = ISP_DRV_IRQ_INT_STATUS_VS1_ST;
        }
        
        //TG2
        if ( CAM_CTL_EN1_TG2_EN & this->en_Top.enable1 ) 
        {
            if(m_pPhyIspDrv->IsReadOnlyMode())
            {
                //ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_TG2_VF_CON, VFDATA_EN, 0);
            }
            else
            {
                //ISP_WRITE_BITS(m_pPhyIspReg, CAM_TG2_VF_CON, VFDATA_EN, 0);
            }
            irq_TG1_DONE.Status = ISP_DRV_IRQ_INT_STATUS_VS2_ST;
        }

        if(m_pPhyIspDrv->IsReadOnlyMode())
        {
            //fbc disable before VF_EN
            //IMGO
            ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CTL_IMGO_FBC, FBC_EN, 0);
            //IMG2O
            ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CTL_IMG2O_FBC, FBC_EN, 0);
            //
            ISP_IOCTL_WRITE_ENABLE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CTL_DMA_EN_CLR, AAO_EN_CLR, 1);
            //
#if defined(_rtbc_use_cq0c_)
            //enable for rtbc
            ISP_IOCTL_WRITE_ENABLE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CTL_EN2, CQ0C_EN, 0);
#endif
        }
        else
        {
            //fbc disable before VF_EN
            //IMGO
            ISP_WRITE_BITS(this->m_pPhyIspReg, CAM_CTL_IMGO_FBC, FBC_EN, 0);
            //IMG2O
            ISP_WRITE_BITS(this->m_pPhyIspReg, CAM_CTL_IMG2O_FBC, FBC_EN, 0);
            //
            ISP_WRITE_ENABLE_BITS(this->m_pPhyIspReg, CAM_CTL_DMA_EN_CLR, AAO_EN_CLR, 1);
            //
#if defined(_rtbc_use_cq0c_)
            //enable for rtbc
            ISP_WRITE_ENABLE_BITS(this->m_pPhyIspReg, CAM_CTL_EN2, CQ0C_EN, 0);
#endif
        }

#if 1   


        ret = this->m_pPhyIspDrv->waitIrq( irq_TG1_DONE );
        if ( MFALSE == ret ) 
        {
            ISP_FUNC_ERR("waitIrq( irq_TG1_DONE ) fail(1)");
        }
        
        //double check for pass1 (HW is sync with vsync signal)
        if(m_pPhyIspDrv->IsReadOnlyMode())
        {
            ISP_IOCTL_READ_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_TG_INTER_ST, TG_CAM_CS, tgStatus);
        }
        else
        {
            tgStatus = ISP_READ_BITS(m_pPhyIspReg, CAM_TG_INTER_ST, TG_CAM_CS);
        }
        
        ISP_FUNC_INF("wait 1 VD till DMA stop tgStatus(0x%x)",tgStatus);
        if(tgStatus!=0x01)
        {
            ret = this->m_pPhyIspDrv->waitIrq( irq_TG1_DONE );
            if ( MFALSE == ret ) 
            {
                ISP_FUNC_ERR("waitIrq( irq_TG1_DONE ) fail(2)");
            }
        }

        //pass1 DMA SHOULD BE inactive.
        //dump debug info
        if(m_pPhyIspDrv->IsReadOnlyMode())
        {
            //1. TG INTER ST
            //  Field  TG_CAM_CS[8:13] should be 1
            //  Field  SYN_VF_DATA_EN[0] should be 0

            ISP_FUNC_INF("- P1 stop VF(0x%x),TG_IN(0x%x),STX(0x%x)",
                ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_TG_VF_CON), \
                ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_TG_INTER_ST), \
                ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(),CAM_CTL_INT_STATUSX ) );
        }
        else
        {
            ISP_FUNC_INF("- P1 stop VF(0x%x),TG_IN(0x%x),STX(0x%x)", \
                ISP_READ_REG(this->m_pPhyIspReg, CAM_TG_VF_CON), \
                ISP_READ_REG(this->m_pPhyIspReg, CAM_TG_INTER_ST), \
                ISP_READ_REG(this->m_pPhyIspReg, CAM_CTL_INT_STATUSX) );
        }
#else
        //
        ISP_FUNC_INF("wait 2 VD till DMA stop");
        //
        ret = this->m_pPhyIspDrv->waitIrq( irq_TG1_DONE );
        if ( MFALSE == ret ) {
            ISP_FUNC_ERR("waitIrq( irq_TG1_DONE ) fail");
        }
        //
        ret = this->m_pPhyIspDrv->waitIrq( irq_TG1_DONE );
        if ( MFALSE == ret ) {
            ISP_FUNC_ERR("waitIrq( irq_TG1_DONE ) fail");
        }
#endif

    }
    else if (ISP_PASS2 == this->path) 
    {
        if(m_pPhyIspDrv->IsReadOnlyMode())
        {
            // remove for MDP trigger             ISP_IOCTL_WRITE_ENABLE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CTL_START, PASS2_START, 0);
        }
        else
        {
            // remove for MDP trigger             ISP_WRITE_ENABLE_BITS(m_pPhyIspReg, CAM_CTL_START, PASS2_START, 0);
        }

        ISP_FUNC_INF("pass2 CQ1 stopMdp");
        if(MDPMGR_NO_ERROR != g_pMdpMgr->stopMdp(g_MdpMgrCfgData[CAM_ISP_CQ1]))
        {
            ISP_FUNC_ERR("pass2 CQ1 g_pMdpMgr->stopMdp fail");
            return -1;
        }
    }
    else if (ISP_PASS2B == this->path) 
    {
        if(m_pPhyIspDrv->IsReadOnlyMode())
        {
            // remove for MDP trigger               ISP_IOCTL_WRITE_ENABLE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CTL_START, PASS2B_START, 0);            
        }
        else
        {
            // remove for MDP trigger               ISP_WRITE_ENABLE_BITS(m_pPhyIspReg, CAM_CTL_START, PASS2B_START, 0);      
        }
        ISP_FUNC_INF("pass2 CQ2 stopMdp");

        if(MDPMGR_NO_ERROR != g_pMdpMgr->stopMdp(g_MdpMgrCfgData[CAM_ISP_CQ2]))
        {
            ISP_FUNC_ERR("pass2 CQ2 g_pMdpMgr->stopMdp fail");
            return -1;
        }
    }
    else if (ISP_PASS2C == this->path) 
    {
        if(m_pPhyIspDrv->IsReadOnlyMode())
        {
            // remove for MDP trigger               ISP_IOCTL_WRITE_ENABLE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CTL_START, PASS2C_START, 0);
            ISP_FUNC_DBG(" : No implementation for Pass2C start = 0\n");        
        }
        else
        {
            // remove for MDP trigger               ISP_WRITE_ENABLE_BITS(m_pPhyIspReg, CAM_CTL_START, PASS2C_START, 0);
            ISP_FUNC_DBG(" : No implementation for Pass2C start = 0\n");        
        }
    }

    //clear buffer queue
    //clear all buffer list
    ISP_FUNC_DBG("clr buf list,path(%d) ",this->path);
    
    for ( int rt_dma =_imgi_; rt_dma < _rt_dma_max_ ;rt_dma++ ) 
    {
        //every  instance share same buffer list
        if(ISP_PASS1 == this->path)
        {
            if( (_imgo_!=rt_dma) && (_img2o_!=rt_dma) ) 
            {
                continue;
            }
        }
        else
        {
            if( (_imgo_==rt_dma) || (_img2o_==rt_dma) ) 
            {
                continue;
            }
        }

        ISP_FUNC_DBG("[before]rt_dma(%d),size:hw(%d),sw(%d)",rt_dma, \
                                                        ISP_BUF_CTRL::m_hwbufL[rt_dma].bufInfoList.size(),
                                                        ISP_BUF_CTRL::m_swbufL[rt_dma].bufInfoList.size());

        ISP_BUF_CTRL::m_hwbufL[rt_dma].bufInfoList.clear();
        ISP_BUF_CTRL::m_swbufL[rt_dma].bufInfoList.clear();
        //
        if ((_imgo_==rt_dma) || (_img2o_==rt_dma)) 
        {
            buf_ctrl.ctrl = ISP_RT_BUF_CTRL_CLEAR;
            buf_ctrl.buf_id = (_isp_dma_enum_)rt_dma;
            buf_ctrl.data_ptr = (MUINT32)&dummy;
            ISP_FUNC_INF("rtBufCtrl.ctrl(%d)/id(%d)/ptr(0x%x)",buf_ctrl.ctrl,buf_ctrl.buf_id,buf_ctrl.data_ptr);
            if ( MTRUE != this->m_pIspDrvShell->m_pPhyIspDrv_bak->rtBufCtrl((void*)&buf_ctrl) ) 
            {
                ISP_FUNC_ERR("ERROR:rtBufCtrl");
                return -1;
            }
        }
            //
        ISP_FUNC_DBG("[after]rt_dma(%d),size:hw(%d),sw(%d) ",rt_dma, \
                ISP_BUF_CTRL::m_hwbufL[rt_dma].bufInfoList.size(), \
                ISP_BUF_CTRL::m_swbufL[rt_dma].bufInfoList.size());
    }


#if 1
    if(MTRUE == function_DbgLogEnable_INFO) 
    {        
        if(ISP_PASS1 == this->path) 
        {
            usleep(1000);
            //pass1 DMA base_addr
            if(m_pPhyIspDrv->IsReadOnlyMode())
            {
                ISP_FUNC_INF("P1 start(0x%x)", \
                    ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CTL_START ));
            }
            else
            {
                ISP_FUNC_INF("P1 start(0x%x)", \
                    ISP_READ_REG(m_pPhyIspReg, CAM_CTL_START ));
            }

        }
        else if ( ISP_PASS2 == this->path ||
                  ISP_PASS2B == this->path ||
                  ISP_PASS2C == this->path ) 
        {
            //usleep(1000);
            //pass2 DMA base_addr
            if(m_pPhyIspDrv->IsReadOnlyMode())
            {
                ISP_FUNC_INF("P2 start(0x%x)", \
                    ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CTL_START ));
            }
            else
            {
                ISP_FUNC_INF("P2 start(0x%x)", \
                    ISP_READ_REG(m_pPhyIspReg, CAM_CTL_START ));
            }
        }

    }

#endif

    ISP_FUNC_DBG("ISP_TOP_CTRL::_disable:X ");

    return 0;
}

int ISP_TOP_CTRL::checkBusy(  unsigned long* param  )
{
    int int_done_status = 0;
    ISP_FUNC_DBG("ISP_TOP_CTRL::checkBusy:E ");
    ISP_FUNC_DBG("ISP_TOP_CTRL::checkBusy:X ");
    return int_done_status?0:1;
}

int ISP_TOP_CTRL::_write2CQ( int cq )
{
    ISP_FUNC_DBG("ISP_TOP_CTRL::_write2CQ: path(%d) E ",this->path);
    //TOP
    if (ISP_PASS2 == this->path || ISP_PASS2B == this->path || ISP_PASS2C == this->path) 
    {
        //tpipe config only in pass2
        this->m_pIspDrvShell->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,(CAM_MODULE_ENUM)CAM_TOP_CTL);
    }
    this->m_pIspDrvShell->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,(CAM_MODULE_ENUM)CAM_TOP_CTL_01);
    this->m_pIspDrvShell->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,(CAM_MODULE_ENUM)CAM_TOP_CTL_02);
    ISP_FUNC_DBG("ISP_TOP_CTRL::_write2CQ:X ");

    return 0;
}

MBOOL ISP_TOP_CTRL::setCQTriggerMode(MINT32 cq, MINT32 mode, MINT32 trig_src)
{
    ISP_FUNC_DBG("ISP_TOP_CTRL::setCQTriggerMode:E ");
    //TOP
    this->m_pIspDrvShell->setCQTriggerMode(cq, mode,trig_src);
    ISP_FUNC_DBG("ISP_TOP_CTRL::setCQTriggerMode:X ");

    return 0;
}


/*/////////////////////////////////////////////////////////////////////////////
    ISP_RAW_PIPE
  /////////////////////////////////////////////////////////////////////////////*/
int ISP_RAW_PIPE::_config( void )
{
    if(this->enable1 & CAM_CTL_EN1_HRZ_EN) 
    {

        MINT32 iHeight,resize, oSize;

        ISP_FUNC_DBG("[HRZ]:src_img_h(0x%x)/cdrz_in_w(0x%x)",this->src_img_h,this->cdrz_in_w);

        /*
        HRZ_INPUT_HEIGHT:
            "HRZ input size
            if (tg_sel = 0) height=image0
            else height = image1"

        HRZ_RESIZE:
            "HRZ resize value
            Get from 32768/decimation ratio
            decimation x1.0: hrz_resize should be 32768
            decimation x0.5: hrz_resize should be 65536
            decimation x0.25: hrz_resize should be 131072 "

        HRZ_OUTSIZE:
            "HRZ output size
            And this value get from
            X1 = input size * 32768 / hrz_resize
            hbn_outsize = 2* (X1>>1)"
        */
        //
        iHeight = this->src_img_h;
        //resize = 32768*10/10;
        resize = ( this->src_img_w << 15 )/this->cdrz_in_w;
        //oSize = (this->src_img_w * 32768 / resize) & (~0x01);
        oSize = ( ( this->src_img_w << 15 ) / resize) & (~0x01);
        //
        ISP_FUNC_DBG("resize(0x%x)/oSize(0x%x)",resize,oSize);
        //
        this->m_pIspDrvShell->cam_isp_hrz_cfg(iHeight, resize, oSize);
	    if(m_pPhyIspDrv->IsReadOnlyMode())
        {
           ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_SL2_HRZ_COMP, SL2_HRZ_COMP, resize>>4);
        }
        else
        {
           ISP_WRITE_BITS(m_pPhyIspReg, CAM_SL2_HRZ_COMP, SL2_HRZ_COMP, resize>>4);
		  
        }
        		
	  
    }
    else
   	{
   	    if(m_pPhyIspDrv->IsReadOnlyMode())
        {
           ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_SL2_HRZ_COMP, SL2_HRZ_COMP, 0x800);
        }
        else
        {
           ISP_WRITE_BITS(m_pPhyIspReg, CAM_SL2_HRZ_COMP, SL2_HRZ_COMP, 0x800);
        }       
       
   	}
	memcpy(&g_MdpMgrCfgData[g_MdpMgrCqIdx].ispTpipeCfgInfo, (MUINT8 *)this->m_pIspDrvShell->m_pTpipeDrv->GetpConfigTpipeStruct(), sizeof(ISP_TPIPE_CONFIG_STRUCT));
	
	if(g_MdpMgrCfgData[g_MdpMgrCqIdx].ispTpipeCfgInfo.top.sl2_en)
	{
                     
       if(m_pPhyIspDrv->IsReadOnlyMode())
       {
         ISP_IOCTL_READ_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_SL2_HRZ_COMP, SL2_HRZ_COMP, g_MdpMgrCfgData[g_MdpMgrCqIdx].ispTpipeCfgInfo.sl2.sl2_hrz_comp);
       }
       else
       {
         g_MdpMgrCfgData[g_MdpMgrCqIdx].ispTpipeCfgInfo.sl2.sl2_hrz_comp = ISP_READ_BITS(m_pPhyIspReg, CAM_SL2_HRZ_COMP, SL2_HRZ_COMP);
       }              			   			 
			  			   
    }
	else
	{
	   g_MdpMgrCfgData[g_MdpMgrCqIdx].ispTpipeCfgInfo.sl2.sl2_hrz_comp=0x800;			
			   
	}
/*	if(m_pPhyIspDrv->IsReadOnlyMode())
    {
       
       ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CTL_EN1, SL2_EN, g_MdpMgrCfgData[g_MdpMgrCqIdx].ispTpipeCfgInfo.top.sl2_en);		
       ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CTL_TCM_EN, TCM_SL2_EN, g_MdpMgrCfgData[g_MdpMgrCqIdx].ispTpipeCfgInfo.top.sl2_en);
    }
    else
    {	
       ISP_WRITE_BITS(m_pPhyIspReg, CAM_CTL_EN1, SL2_EN, g_MdpMgrCfgData[g_MdpMgrCqIdx].ispTpipeCfgInfo.top.sl2_en);
	   ISP_WRITE_BITS(m_pPhyIspReg, CAM_CTL_TCM_EN, TCM_SL2_EN, g_MdpMgrCfgData[g_MdpMgrCqIdx].ispTpipeCfgInfo.top.sl2_en);
               
    }*/
	if(m_pPhyIspDrv->IsReadOnlyMode())
    {
       ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_SL2_HRZ_COMP, SL2_HRZ_COMP, g_MdpMgrCfgData[g_MdpMgrCqIdx].ispTpipeCfgInfo.sl2.sl2_hrz_comp);
    }
    else
    {
       ISP_WRITE_BITS(m_pPhyIspReg, CAM_SL2_HRZ_COMP, SL2_HRZ_COMP, g_MdpMgrCfgData[g_MdpMgrCqIdx].ispTpipeCfgInfo.sl2.sl2_hrz_comp);
    }
			 
	ISP_FUNC_DBG("g_MdpMgrCfgData[g_MdpMgrCqIdx].ispTpipeCfgInfo.top.sl2_en(%d)",g_MdpMgrCfgData[g_MdpMgrCqIdx].ispTpipeCfgInfo.top.sl2_en);  
	ISP_FUNC_DBG("g_MdpMgrCfgData[g_MdpMgrCqIdx].ispTpipeCfgInfo.sl2.sl2_hrz_comp(%d)",g_MdpMgrCfgData[g_MdpMgrCqIdx].ispTpipeCfgInfo.sl2.sl2_hrz_comp); 
    return 0;
}

/*
int ISP_RAW_PIPE::_enable( void* pParam )
{
    ISP_FUNC_DBG("ISP_RAW_PIPE::_enable:E ");
    ISP_FUNC_DBG("ISP_RAW_PIPE::_enable:X ");
    return 0;
}

int ISP_RAW_PIPE::_disable( void )
{
    ISP_FUNC_DBG("ISP_RAW_PIPE::_disable:E ");
    ISP_FUNC_DBG("ISP_RAW_PIPE::_disable:X ");
    return 0;
}

int ISP_RAW_PIPE::checkBusy(  unsigned long* param  )
{
int int_done_status = 0;
    ISP_FUNC_DBG("ISP_RAW_PIPE::checkBusy:E ");
    ISP_FUNC_DBG("ISP_RAW_PIPE::checkBusy:X ");
    return int_done_status?0:1;
}
*/
int ISP_RAW_PIPE::_write2CQ( int cq )
{
    ISP_FUNC_DBG("ISP_RAW_PIPE::_write2CQ:E ");

    #define CAM_ISP_RAW_MODULE_NUM    5
    struct stCam_Id_Enable cq_info[CAM_ISP_RAW_MODULE_NUM] = { \
                                        {CAM_ISP_UNP,CAM_CTL_EN1_UNP_EN},
                                       /* {CAM_ISP_OBC,CAM_CTL_EN1_OB_EN}, */
                                      {CAM_ISP_CFA,  CAM_CTL_EN1_CFA_EN},
                                        {CAM_ISP_LSC,CAM_CTL_EN1_LSC_EN},
                                        {CAM_ISP_HRZ,CAM_CTL_EN1_HRZ_EN},
                                       /* {CAM_ISP_BNR,CAM_CTL_EN1_BNR_EN}, */
                                        {CAM_ISP_PGN,CAM_CTL_EN1_PGN_EN}
                                        };

    for (int i=0;i<CAM_ISP_RAW_MODULE_NUM;i++) 
    {
        if ( this->enable1 & cq_info[i].en_bit) 
        {
            ISP_FUNC_DBG("(0x%x)write 0x%08X to CQ: ",enable1,cq_info[i].en_bit);
            //
            this->m_pIspDrvShell->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,(CAM_MODULE_ENUM)cq_info[i].id);
        }
    }

    ISP_FUNC_DBG("ISP_RAW_PIPE::_write2CQ:X ");
    return 0;
}

/*/////////////////////////////////////////////////////////////////////////////
    ISP_RGB_PIPE
  /////////////////////////////////////////////////////////////////////////////*/
int ISP_RGB_PIPE::_config( void )
{
    unsigned int enable1 = this->enable1;
    ISP_FUNC_DBG("ISP_RGB_PIPE::_config:E ");
    ISP_FUNC_DBG("[enable1:0x%08X] m_pIspDrv(0x%x)\n",enable1,m_pIspDrv);

    for(int i=0;i<32;i++) 
    {
        if( this->m_pIspDrvShell->default_setting_function[i] && ((enable1>>i)&0x1) ) 
        {
            this->m_pIspDrvShell->default_setting_function[i]();
        }
    }
    
    if( CAM_CTL_EN1_C02_EN&enable1 ) 
    {
        this->m_pIspDrvShell->cam_isp_c02_cfg(this->src_img_h,0,1);
    }

    ISP_FUNC_DBG("ISP_RGB_PIPE::_config:X ");
    return 0;
}

/*
int ISP_RGB_PIPE::_enable( void* pParam )
{
    ISP_FUNC_DBG("ISP_RGB_PIPE::_enable:E ");
    ISP_FUNC_DBG("ISP_RGB_PIPE::_enable:X ");
    return 0;
}

int ISP_RGB_PIPE::_disable( void )
{
    ISP_FUNC_DBG("ISP_RGB_PIPE::_disable:E ");
    ISP_FUNC_DBG("ISP_RGB_PIPE::_disable:X ");
    return 0;
}

int ISP_RGB_PIPE::checkBusy(  unsigned long* param  )
{
int int_done_status = 0;
    ISP_FUNC_DBG("ISP_RGB_PIPE::_disable:E ");
    ISP_FUNC_DBG("ISP_RGB_PIPE::_disable:X ");
    return int_done_status?0:1;
}
*/
int ISP_RGB_PIPE::_write2CQ( int cq )
{
    ISP_FUNC_DBG("ISP_RGB_PIPE::_write2CQ:E ");

#if defined(USING_MTK_LDVT)
    //debug path, imagio will config all engine by CQ
    #define CAM_ISP_RGB_MODULE_COUNT    9 //  6
    struct stCam_Id_Enable cq_info[CAM_ISP_RGB_MODULE_COUNT] = { \
                                         {CAM_DUMMY_,  CAM_CTL_EN1_MFB_EN}, \
                                         {CAM_DUMMY_,  CAM_CTL_EN1_C02_EN}, \
                                         {CAM_DUMMY_,  CAM_CTL_EN1_CFA_EN}, \
                                         {CAM_DUMMY_,  CAM_CTL_EN1_CCL_EN}, \
                                         {CAM_ISP_G2G,  CAM_CTL_EN1_G2G_EN}, \
                                         {CAM_DUMMY_,  CAM_CTL_EN1_LCE_EN}, \
                                         {CAM_DUMMY_,    CAM_CTL_EN1_GGM_EN}, \
                                         {CAM_DUMMY_,     CAM_CTL_EN1_GGM_EN}, \
                                         {CAM_DUMMY_,  CAM_CTL_EN1_GGM_EN}
                                         };
#else
    //MP path, do not config engine by imageio
#define CAM_ISP_RGB_MODULE_COUNT    4
    struct stCam_Id_Enable cq_info[CAM_ISP_RGB_MODULE_COUNT] = { \
                                         {CAM_DUMMY_,  CAM_CTL_EN1_MFB_EN}, \
                                         {CAM_DUMMY_,  CAM_CTL_EN1_C02_EN}, \
                                         {CAM_ISP_CFA,  CAM_CTL_EN1_CFA_EN}, \
                                         {CAM_ISP_CCL,  CAM_CTL_EN1_CCL_EN}, \
                                         };

#endif

    for (int i=0;i<CAM_ISP_RGB_MODULE_COUNT;i++) 
    {
        if ( this->enable1 & cq_info[i].en_bit) 
        {
            ISP_FUNC_DBG("(0x%x)write 0x%08X to CQ: ",enable1,cq_info[i].en_bit);

            this->m_pIspDrvShell->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,(CAM_MODULE_ENUM)cq_info[i].id);

#if 0 //js_test remove below later
            this->m_pIspDrvShell->cam_cq_write_to_queue_buf(
                this->CQ,
                cq_info[i].id,
                cq_module_info[cq_info[i].id].addr_ofst,
                cq_module_info[cq_info[i].id].reg_num,
                (unsigned int*)&this->m_pIspDrvShell->m_pVirtRegAddr[cq_module_info[cq_info[i].id].addr_ofst]);
#endif

        }
    }



    ISP_FUNC_DBG("ISP_RGB_PIPE::_write2CQ:X ");
    return 0;
}

/*/////////////////////////////////////////////////////////////////////////////
    ISP_YUV_PIPE
  /////////////////////////////////////////////////////////////////////////////*/

int ISP_YUV_PIPE::_config( void )
{
    unsigned int enable2 = this->enable2;
    ISP_FUNC_DBG("ISP_YUV_PIPE::_config:E ");
    ISP_FUNC_DBG("[enable2:0x%08X] ",enable2);

    if ( CAM_CTL_EN2_G2C_EN&enable2 ) 
    {
        //it impact raw color effect
        //tuing will update this engine
        //this->m_pIspDrvShell->cam_isp_g2c_cfg();
#if defined(USING_MTK_LDVT)
        this->m_pIspDrvShell->cam_isp_g2c_cfg();
#endif
    }
    else
    {
#if 0//1//defined(USING_MTK_LDVT)
        this->m_pIspDrvShell->cam_isp_g2c_cfg();
#endif
    }
    
    if ( CAM_CTL_EN2_C42_EN&enable2 ) 
    {
        this->m_pIspDrvShell->cam_isp_c42_cfg();
    }
    ISP_FUNC_DBG("ISP_YUV_PIPE::_config:X ");
    return 0;
}

/*
int ISP_YUV_PIPE::_enable( void* pParam )
{
    ISP_FUNC_DBG("ISP_YUV_PIPE::_enable:E ");
    ISP_FUNC_DBG("ISP_YUV_PIPE::_enable:X ");
    return 0;
}

int ISP_YUV_PIPE::_disable( void )
{
    ISP_FUNC_DBG("ISP_YUV_PIPE::_disable:E ");
    ISP_FUNC_DBG("ISP_YUV_PIPE::_disable:X ");
     return 0;
}

int ISP_YUV_PIPE::checkBusy(  unsigned long* param  )
{
int int_done_status = 0;
    ISP_FUNC_DBG("ISP_YUV_PIPE::checkBusy:E ");
    ISP_FUNC_DBG("ISP_YUV_PIPE::checkBusy:X ");
    return int_done_status?0:1;
}
*/
int ISP_YUV_PIPE::_write2CQ( int cq )
{
    ISP_FUNC_DBG("ISP_YUV_PIPE::_write2CQ:E ");



#if defined(USING_MTK_LDVT)
    //debug path, imagio will config all engine by CQ
    #define CAM_ISP_YUV_MODULE_NUM    5
    struct stCam_Id_Enable cq_info[CAM_ISP_YUV_MODULE_NUM] = { \
                                            {CAM_ISP_C42,CAM_CTL_EN2_C42_EN},
                                            {CAM_ISP_PCA,CAM_CTL_EN2_PCA_EN},
                                            {CAM_ISP_PCA_CON,CAM_CTL_EN2_PCA_EN},
                                            {CAM_ISP_SEEE,CAM_CTL_EN2_SEEE_EN},
                                            {CAM_ISP_G2C,CAM_CTL_EN2_G2C_EN}
                                            };
#else
    //MP path, do not config engine by imageio
    #define CAM_ISP_YUV_MODULE_NUM    4
    struct stCam_Id_Enable cq_info[CAM_ISP_YUV_MODULE_NUM] = { \
                                            {CAM_ISP_C42,CAM_CTL_EN2_C42_EN},
                                            {CAM_ISP_PCA,CAM_CTL_EN2_PCA_EN},
                                            {CAM_ISP_PCA_CON,CAM_CTL_EN2_PCA_EN},
                                            {CAM_ISP_SEEE,CAM_CTL_EN2_SEEE_EN}
                                            };

#endif

    for (int i=0;i<CAM_ISP_YUV_MODULE_NUM;i++) 
    {
        if ( this->enable2 & cq_info[i].en_bit) 
        {
            ISP_FUNC_DBG("(0x%x)write 0x%08X to CQ: ",enable2,cq_info[i].en_bit);

            this->m_pIspDrvShell->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,(CAM_MODULE_ENUM)cq_info[i].id);

#if 0 //js_test remove below later
            this->m_pIspDrvShell->cam_cq_write_to_queue_buf(
                this->CQ,
                cq_info[i].id,
                cq_module_info[cq_info[i].id].addr_ofst,
                cq_module_info[cq_info[i].id].reg_num,
                (unsigned int*)&this->m_pIspDrvShell->m_pVirtRegAddr[cq_module_info[cq_info[i].id].addr_ofst]);
#endif

        }
    }

    ISP_FUNC_DBG("ISP_YUV_PIPE::_write2CQ:X ");
    return 0;
}


/*/////////////////////////////////////////////////////////////////////////////
    ISP_TURNING_CTRL
  /////////////////////////////////////////////////////////////////////////////*/
int ISP_TURNING_CTRL::_config( void )
{
    ISP_FUNC_DBG("[CQ:%d] isApplyTurn(0x%x)E\n",this->CQ,this->isApplyTurn);

    if(this->CQ == CAM_ISP_CQ1 && this->isApplyTurn)
    {
        m_pPhyIspDrv->updateTurningCq1();
    }
    else if(this->CQ == CAM_ISP_CQ2 && this->isApplyTurn)
    {
        m_pPhyIspDrv->updateTurningCq2();
    }

    ISP_FUNC_DBG("_config:X\n");

    return 0;
}

/*/////////////////////////////////////////////////////////////////////////////
    CAM_CDP_PIPE
  /////////////////////////////////////////////////////////////////////////////*/

/*
int CAM_CDP_PIPE::_enable( void* pParam )
{
    ISP_FUNC_DBG("CAM_CDP_PIPE::_enable:E ");
    ISP_FUNC_DBG("CAM_CDP_PIPE::_enable:X ");

    return 0;
}
int CAM_CDP_PIPE::checkBusy(  unsigned long* param  )
{
int int_done_status = 0;
    ISP_FUNC_DBG("CAM_CDP_PIPE::checkBusy:E ");
    ISP_FUNC_DBG("CAM_CDP_PIPE::checkBusy:X ");
    return int_done_status?0:1;
}
*/


/**********************************************************************
*
**********************************************************************/
MBOOL CAM_CDP_PIPE::createMdpMgr()
{ 
    ISP_FUNC_INF("+");
    MBOOL err = MTRUE;

    if(g_pMdpMgr == NULL)
    {
        g_pMdpMgr = MdpMgr::createInstance();
    }

    if(g_pMdpMgr == NULL)
    {
        err = MFALSE;
    }
    
    return err;
}

/**********************************************************************
*
**********************************************************************/
int CAM_CDP_PIPE::_disable( void )
{
    ISP_FUNC_DBG("CAM_CDP_PIPE::_disable:E,path(0x%x) ",this->path);
//    if (ISP_PASS1 != this->path) {
//    // Release all ROTDMA SYSRAM buffer.
//    for (MUINT32 RotDma = 0; RotDma < CDP_DRV_ROTDMA_AMOUNT; RotDma++)
//        {
//          this->m_pIspDrvShell->m_pCdpDrv->FreeRotationBuf((CDP_DRV_ROTDMA_ENUM)RotDma);
//        }
//    }
    return 0;
}

int CAM_CDP_PIPE::_config( void )
{

//#define _USE_OLD_CDP_DRVIER_

#if defined(_USE_OLD_CDP_DRVIER_)
#else
    CDP_DRV_IMG_SIZE_STRUCT sizeIn;
    CDP_DRV_IMG_SIZE_STRUCT sizeOut;
    CDP_DRV_IMG_CROP_STRUCT crop;
    CDP_DRV_IMG_SIZE_STRUCT rotDMA_ImgSize;
    CDP_DRV_IMG_CROP_STRUCT rotDMA_Crop;
    MBOOL cdp_result = MTRUE;

#endif

    ISP_FUNC_DBG("CAM_CDP_PIPE::_config,tcm(%d):E",this->tpipeMode);

    if(this->conf_cdrz) 
    {
#if defined(_USE_OLD_CDP_DRVIER_)
        cam_cdp_cdrz_cfg(cdrz_filter, this->cdrz_in.w,this->cdrz_in.h,this->cdrz_out.w,this->cdrz_out.h);
#else
        ISP_FUNC_DBG("m_pCdpDrv->CDRZ_Config ");

        //CDRZ
        sizeIn.Width   = this->cdrz_in.w;
        sizeIn.Height  = this->cdrz_in.h;
        sizeOut.Width  = this->cdrz_out.w;
        sizeOut.Height = this->cdrz_out.h;
        
        // crop
        crop.Width.Start  = this->cdrz_crop.x;
        crop.Width.Size   = this->cdrz_crop.w;
        crop.Height.Start = this->cdrz_crop.y;
        crop.Height.Size  = this->cdrz_crop.h;
        
        cdp_result = this->m_pIspDrvShell->m_pCdpDrv->CDRZ_Config(this->tpipeMode,sizeIn,sizeOut,crop);
        if(MFALSE == cdp_result) 
        {
            ISP_FUNC_ERR(" CDRZ_Config");
            return -1;
        }
#endif
    }

    if(this->conf_rotDMA)
    {
        ISP_FUNC_DBG("%s:dispo:[base_pAddr:0x%08X] ",this->name_Str(),this->dispo_out.memBuf.base_pAddr);
        ISP_FUNC_DBG("cdp : cq(%d)",this->CQ);
        
        // initialize
        for(MINT32 i = 0; i < ISP_MDP_DL_NUM; i++)
        {
            if(this->CQ == CAM_ISP_CQ1)
            {
                g_MdpMgrCfgData[CAM_ISP_CQ1].dstPortCfg[i] = 0;
            }
            else if(this->CQ == CAM_ISP_CQ2)
            {
                g_MdpMgrCfgData[CAM_ISP_CQ2].dstPortCfg[i] = 0;
            }
        }

    #if defined(_USE_OLD_CDP_DRVIER_)
        cam_cdp_curz_cfg(this->curz_in.w,this->curz_in.h,this->curz_in.w,this->curz_in.h);
        cam_cdp_dispo_cfg(_FMT_YUV422_1P_,_FMT_SEQ_422_UYVY_,this->dispo_out.size.w,this->dispo_out.size.h,this->dispo_out.memBuf.base_pAddr);
    #else

        #if 0   // 82 CDP only has CDRZ

        //CURZ
        if( CAM_CTL_EN2_CURZ_EN & this->enable2 )
        {
            ISP_FUNC_DBG("m_pCdpDrv->CURZ_Config");
            
            sizeIn.Width = this->curz_in.w;
            sizeIn.Height = this->curz_in.h;
            sizeOut.Width = this->curz_out.w;
            sizeOut.Height = this->curz_out.h;

            crop.Width.Start = this->curz_crop.x;
            crop.Width.Size = this->curz_crop.w;
            crop.Height.Start = this->curz_crop.y;
            crop.Height.Size = this->curz_crop.h;
            
            cdp_result = this->m_pIspDrvShell->m_pCdpDrv->CURZ_Config(this->tpipeMode,sizeIn,sizeOut,crop);
            if(MFALSE == cdp_result)
            {
                ISP_FUNC_ERR(" CURZ_Config");
                return -1;
            }
        }
        
        //PRZ
        if ( CAM_CTL_EN2_PRZ_EN & this->enable2 ) 
        {
            ISP_FUNC_DBG("m_pCdpDrv->PRZ_Config");
            
            sizeIn.Width = this->prz_in.w;
            sizeIn.Height = this->prz_in.h;
            sizeOut.Width = this->prz_out.w;
            sizeOut.Height = this->prz_out.h;
            
            crop.Width.Start = this->prz_crop.x;
            crop.Width.Size = this->prz_crop.w;
            crop.Height.Start = this->prz_crop.y;
            crop.Height.Size = this->prz_crop.h;
            cdp_result = this->m_pIspDrvShell->m_pCdpDrv->PRZ_Config(this->tpipeMode,sizeIn,sizeOut,crop);

            if (MFALSE == cdp_result) 
            {
                ISP_FUNC_ERR(" PRZ_Config");
                return -1;
            }
        }
        
        #endif

        // VIDO
        if(CAM_CTL_DMA_EN_VIDO_EN & this->dma_enable) 
        {
            rotDMA_ImgSize.Width = this->vido_out.size.w;
            rotDMA_ImgSize.Height = this->vido_out.size.h;
            rotDMA_Crop.Width.Start = this->vido_out.crop.x;
            rotDMA_Crop.Width.Size= this->vido_out.crop.w;
            rotDMA_Crop.Height.Start= this->vido_out.crop.y;
            rotDMA_Crop.Height.Size = this->vido_out.crop.h;

            //====== MdpMgr ======

            if(this->CQ == CAM_ISP_CQ1)
            {
                g_MdpMgrCfgData[CAM_ISP_CQ1].dstPortCfg[ISP_MDP_DL_VIDO] = 1;
                memcpy(&g_MdpMgrCfgData[CAM_ISP_CQ1].dstDma[ISP_MDP_DL_VIDO], &this->vido_out, sizeof(CdpRotDMACfg));
            }
            else if(this->CQ == CAM_ISP_CQ2)
            {
                g_MdpMgrCfgData[CAM_ISP_CQ2].dstPortCfg[ISP_MDP_DL_VIDO] = 1;
                memcpy(&g_MdpMgrCfgData[CAM_ISP_CQ2].dstDma[ISP_MDP_DL_VIDO], &this->vido_out, sizeof(CdpRotDMACfg));
            }
            
        #if 0   // no need for mt6582
            //====== CDP Config ======
            
            cdp_result = this->m_pIspDrvShell->m_pCdpDrv->VIDO_Config( rotDMA_ImgSize,
                                                                       rotDMA_Crop,
                                                                       this->vido_out.Format,
                                                                       this->vido_out.Plane,
                                                                       this->vido_out.Sequence,
                                                                       this->vido_out.Rotation,
                                                                       this->vido_out.Flip,
                                                                       this->tpipe_w,
                                                                       MTRUE);    //Vent@20120926: Add for fgDitherEnable.
            if(MFALSE == cdp_result) 
            {
                ISP_FUNC_ERR(" VIDO_Config");
                return -1;
            }
            
            cdp_result = this->m_pIspDrvShell->m_pCdpDrv->VIDO_SetOutputAddr( this->vido_out.memBuf.base_pAddr,
                                                                              this->vido_out.memBuf.ofst_addr,
                                                                              this->vido_out.size.stride * this->vido_out.pixel_byte,
                                                                              this->vido_out.memBuf_c.base_pAddr,
                                                                              this->vido_out.memBuf_c.ofst_addr,
                                                                              this->vido_out.size_c.stride * this->vido_out.pixel_byte,
                                                                              this->vido_out.memBuf_v.base_pAddr,
                                                                              this->vido_out.memBuf_v.ofst_addr,
                                                                              this->vido_out.size_v.stride * this->vido_out.pixel_byte);

            ISP_FUNC_INF("[VIDO]m_pIspReg[0x%08x],0x%x-0x%x-0x%x",m_pIspReg,this->vido_out.memBuf.base_pAddr,
                    this->vido_out.memBuf_c.base_pAddr,this->vido_out.memBuf_v.base_pAddr);

            if (MFALSE == cdp_result)
            {
                ISP_FUNC_ERR(" VIDO_SetOutputAddr");
                return -1;
            }
        #endif
        }
        
        #if 0
    
        //CRSP-->vido depenent
        if ( CAM_CTL_EN2_UV_CRSA_EN& this->enable2 ) {
            ISP_FUNC_DBG("m_pCdpDrv->RSP_Config");
            sizeOut.Width = this->vido_out.size.w;
            sizeOut.Height = this->vido_out.size.h;
            cdp_result = this->m_pIspDrvShell->m_pCdpDrv->RSP_Config(sizeOut,(CDP_DRV_UV_FORMAT_ENUM)CDP_DRV_UV_FORMAT_YUV422,(CDP_DRV_ROTATION_ENUM)this->vido_out.Rotation);
            if (MFALSE == cdp_result) {
                ISP_FUNC_ERR(" RSP_Config");
                return -1;
            }
        }
        
        #endif
        
        // DISPO
        if(CAM_CTL_DMA_EN_DISPO_EN & this->dma_enable)
        {
            ISP_FUNC_DBG("m_pCdpDrv->DISPO_Config");
            
            rotDMA_ImgSize.Width = this->dispo_out.size.w;
            rotDMA_ImgSize.Height = this->dispo_out.size.h;
            rotDMA_Crop.Width.Start = this->dispo_out.crop.x;
            rotDMA_Crop.Width.Size= this->dispo_out.crop.w;
            rotDMA_Crop.Height.Start= this->dispo_out.crop.y;
            rotDMA_Crop.Height.Size = this->dispo_out.crop.h;

            //====== MdpMgr ======

            if(this->CQ == CAM_ISP_CQ1)
            {
                g_MdpMgrCfgData[CAM_ISP_CQ1].dstPortCfg[ISP_MDP_DL_DISPO] = 1;
                memcpy(&g_MdpMgrCfgData[CAM_ISP_CQ1].dstDma[ISP_MDP_DL_DISPO], &this->dispo_out, sizeof(CdpRotDMACfg));
            }
            else if(this->CQ == CAM_ISP_CQ2)
            {
                g_MdpMgrCfgData[CAM_ISP_CQ2].dstPortCfg[ISP_MDP_DL_DISPO] = 1;
                memcpy(&g_MdpMgrCfgData[CAM_ISP_CQ2].dstDma[ISP_MDP_DL_DISPO], &this->dispo_out, sizeof(CdpRotDMACfg));
            }

        #if 0   // no need for mt6582
            //====== CDP Config ======
            
            this->m_pIspDrvShell->m_pCdpDrv->DISPO_SetSource((0 == disp_vid_sel)?CDP_DRV_DISPO_SRC_PRZ:CDP_DRV_DISPO_SRC_RSP);
            
            cdp_result = this->m_pIspDrvShell->m_pCdpDrv->DISPO_Config( rotDMA_ImgSize,
                                                                        rotDMA_Crop,
                                                                        this->dispo_out.Format,
                                                                        this->dispo_out.Plane,
                                                                        this->dispo_out.Sequence,
                                                                        this->dispo_out.Rotation,
                                                                        this->dispo_out.Flip,
                                                                        MTRUE);      //Vent@20120926: Add for fgDitherEnable.
            if(MFALSE == cdp_result)
            {
                ISP_FUNC_ERR(" DISPO_Config");
                return -1;
            }
            
            cdp_result = this->m_pIspDrvShell->m_pCdpDrv->DISPO_SetOutputAddr( this->dispo_out.memBuf.base_pAddr,
                                                                               this->dispo_out.memBuf.ofst_addr,
                                                                               this->dispo_out.size.stride * this->dispo_out.pixel_byte,
                                                                               this->dispo_out.memBuf_c.base_pAddr,
                                                                               this->dispo_out.memBuf_c.ofst_addr,
                                                                               this->dispo_out.size_c.stride * this->dispo_out.pixel_byte,
                                                                               this->dispo_out.memBuf_v.base_pAddr,
                                                                               this->dispo_out.memBuf_v.ofst_addr,
                                                                               this->dispo_out.size_v.stride * this->dispo_out.pixel_byte);

            ISP_FUNC_DBG("[ISP_BUF]:m_pIspReg=[0x%08x]/DISPO_BASE_ADDR=[0x%08X] ",m_pIspReg,this->dispo_out.memBuf.base_pAddr);

            if(MFALSE == cdp_result)
            {
                ISP_FUNC_ERR(" DISPO_SetOutputAddr");
                return -1;
            }
        #endif
        }
#endif
    }

    ISP_FUNC_DBG("CAM_CDP_PIPE::_config:X ");
    return 0;
}

int CAM_CDP_PIPE::_write2CQ( int cq )
{
    ISP_FUNC_DBG("CAM_CDP_PIPE::_write2CQ:E ");
    /*
    //CDP
    CAM_CDP_CDRZ,        //15   15004B00~15004B38
    CAM_CDP_CURZ_EIS,    //23   15004B40~15004B98
    CAM_CDP_PRZ,         //15   15004BA0~15004BD8
    CAM_CDP_VRZ,         //13   15004BE0~15004C10
    CAM_CDP_FE,          //4    15004C20~15004C2C
    CAM_CDP_CRSP,        //4    15004C30~15004C3C //->use default is ok.
    CAM_CDP_VRZO,        //26   15004C50~15004CB4
    CAM_CDP_VIDO,        //29   15004CC0~15004D30
    CAM_CDP_DISPO,       //26   15004D40~15004DA4
    */
    if(this->conf_cdrz) 
    {
        //cdrz
        ISP_FUNC_DBG("CQ:push CAM_CDP_CDRZ ");

        this->m_pIspDrvShell->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,(CAM_MODULE_ENUM)CAM_CDP_CDRZ);

#if 0 //js_test remove below later
        this->m_pIspDrvShell->cam_cq_write_to_queue_buf(
            this->CQ,
            CAM_CDP_CDRZ,
            cq_module_info[CAM_CDP_CDRZ].addr_ofst,
            cq_module_info[CAM_CDP_CDRZ].reg_num,
            (unsigned int*)&this->m_pIspDrvShell->m_pVirtRegAddr[cq_module_info[CAM_CDP_CDRZ].addr_ofst]);
#endif

    }
    
    if(this->conf_rotDMA) 
    {
        //curz
        if(CAM_CTL_EN2_CURZ_EN& this->enable2) 
        {
            ISP_FUNC_DBG("CQ:push CAM_CDP_CURZ_EIS ");

            //this->m_pIspDrvShell->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,(CAM_MODULE_ENUM)CAM_CDP_CURZ);
#if 0 //js_test remove below later
            this->m_pIspDrvShell->cam_cq_write_to_queue_buf(
                this->CQ,
                CAM_CDP_CURZ_EIS,
                cq_module_info[CAM_CDP_CURZ_EIS].addr_ofst,
                cq_module_info[CAM_CDP_CURZ_EIS].reg_num,
                (unsigned int*)&this->m_pIspDrvShell->m_pVirtRegAddr[cq_module_info[CAM_CDP_CURZ_EIS].addr_ofst]);
#endif

        }
        
        //prz
        if(CAM_CTL_EN2_PRZ_EN& this->enable2) 
        {
            ISP_FUNC_DBG("CQ:push CAM_CDP_PRZ ");

            //this->m_pIspDrvShell->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,(CAM_MODULE_ENUM)CAM_CDP_PRZ);

#if 0 //js_test remove below later
            this->m_pIspDrvShell->cam_cq_write_to_queue_buf(
                this->CQ,
                CAM_CDP_PRZ,
                cq_module_info[CAM_CDP_PRZ].addr_ofst,
                cq_module_info[CAM_CDP_PRZ].reg_num,
                (unsigned int*)&this->m_pIspDrvShell->m_pVirtRegAddr[cq_module_info[CAM_CDP_PRZ].addr_ofst]);
#endif

        }

#if 0 //js_test remove below later
        //vrz
        if ( CAM_CTL_EN2_VRZ_EN& this->enable2 ) {
            ISP_FUNC_DBG("CQ:push CAM_CDP_VRZ ");


            this->m_pIspDrvShell->cam_cq_write_to_queue_buf(
                this->CQ,
                CAM_CDP_VRZ,
                cq_module_info[CAM_CDP_VRZ].addr_ofst,
                cq_module_info[CAM_CDP_VRZ].reg_num,
                (unsigned int*)&this->m_pIspDrvShell->m_pVirtRegAddr[cq_module_info[CAM_CDP_VRZ].addr_ofst]);
        }

        //crsp
        if ( CAM_CTL_EN2_UV_CRSA_EN&this->enable2 ) {
            ISP_FUNC_DBG("CQ:push CAM_CDP_CRSP ");


            this->m_pIspDrvShell->cam_cq_write_to_queue_buf(
                this->CQ,
                CAM_CDP_CRSP,
                cq_module_info[CAM_CDP_CRSP].addr_ofst,
                cq_module_info[CAM_CDP_CRSP].reg_num,
                (unsigned int*)&this->m_pIspDrvShell->m_pVirtRegAddr[cq_module_info[CAM_CDP_CRSP].addr_ofst]);

        }
#endif

        //FE
        if(CAM_CTL_EN2_FE_EN& this->enable2) 
        {         
            ISP_FUNC_DBG("CQ:push CAM_CDP_FE ");

            //this->m_pIspDrvShell->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,(CAM_MODULE_ENUM)CAM_CDP_FE);

#if 0 //js_test remove below later
            this->m_pIspDrvShell->cam_cq_write_to_queue_buf(
                this->CQ,
                CAM_CDP_FE,
                cq_module_info[CAM_CDP_FE].addr_ofst,
                cq_module_info[CAM_CDP_FE].reg_num,
                (unsigned int*)&this->m_pIspDrvShell->m_pVirtRegAddr[cq_module_info[CAM_CDP_FE].addr_ofst]);
#endif

        }

        //cdp_vido
        if(CAM_CTL_DMA_EN_VIDO_EN&this->dma_enable) 
        {
            ISP_FUNC_DBG("CQ:push CAM_CDP_VIDO ");

            //this->m_pIspDrvShell->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,(CAM_MODULE_ENUM)CAM_CDP_VIDO);

#if 0 //js_test remove below later
            this->m_pIspDrvShell->cam_cq_write_to_queue_buf(
                this->CQ,
                CAM_CDP_VIDO,
                cq_module_info[CAM_CDP_VIDO].addr_ofst,
                cq_module_info[CAM_CDP_VIDO].reg_num,
                (unsigned int*)&this->m_pIspDrvShell->m_pVirtRegAddr[cq_module_info[CAM_CDP_VIDO].addr_ofst]);
#endif

        }
        
        //cdp_dispo
        if(CAM_CTL_DMA_EN_DISPO_EN&this->dma_enable) 
        {
            ISP_FUNC_DBG("CQ:push CAM_CDP_DISPO ");

            //this->m_pIspDrvShell->cqAddModule((ISP_DRV_CQ_ENUM)this->CQ,(CAM_MODULE_ENUM)CAM_CDP_DISPO);

#if 0 //js_test remove below later
            this->m_pIspDrvShell->cam_cq_write_to_queue_buf(
                this->CQ,
                CAM_CDP_DISPO,
                cq_module_info[CAM_CDP_DISPO].addr_ofst,
                cq_module_info[CAM_CDP_DISPO].reg_num,
                (unsigned int*)&this->m_pIspDrvShell->m_pVirtRegAddr[cq_module_info[CAM_CDP_DISPO].addr_ofst]);
#endif

        }
    }

    ISP_FUNC_DBG("CAM_CDP_PIPE::_write2CQ:X ");
    return 0;
}
//
int CAM_CDP_PIPE::_setZoom( void )
{
unsigned int *data_ptr = NULL;

    ISP_FUNC_DBG(":E ");
    ISP_FUNC_DBG("0x%08x/0x%08x ",this->conf_cdrz,this->conf_rotDMA);

    if ( this->conf_cdrz ) 
    {
        ISP_FUNC_DBG("data_ptr:0x%08x",data_ptr);
    }
    
    if ( this->conf_rotDMA )
    {
        ISP_FUNC_DBG("data_ptr:0x%08x",data_ptr);
    }

    ISP_FUNC_DBG(":X ");
    return 0;
}


/*/////////////////////////////////////////////////////////////////////////////
    CAM_TDRI_PIPE
  /////////////////////////////////////////////////////////////////////////////*/
int CAM_TDRI_PIPE::_config( void )
{
    MBOOL tdri_result = MTRUE;

    ISP_FUNC_DBG("CAM_TDRI_PIPE::_config:Start this->enTdri(%d)",this->enTdri);
    if ( this->enTdri ) 
    {
        tdri_result = this->m_pIspDrvShell->m_pTpipeDrv->configTdriPara(&tdri);
        
        if (MFALSE == tdri_result) 
        {
            ISP_FUNC_ERR(" TDRI_Config");
            return -1;
        }
        else // Add
        {
            memcpy(&g_MdpMgrCfgData[g_MdpMgrCqIdx].ispTpipeCfgInfo, (MUINT8 *)this->m_pIspDrvShell->m_pTpipeDrv->GetpConfigTpipeStruct(), sizeof(ISP_TPIPE_CONFIG_STRUCT));
			if(m_pPhyIspDrv->IsReadOnlyMode())
            {
              ISP_IOCTL_READ_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_SL2_HRZ_COMP, SL2_HRZ_COMP, g_MdpMgrCfgData[g_MdpMgrCqIdx].ispTpipeCfgInfo.sl2.sl2_hrz_comp);
            }
            else
            {
              g_MdpMgrCfgData[g_MdpMgrCqIdx].ispTpipeCfgInfo.sl2.sl2_hrz_comp = ISP_READ_BITS(m_pPhyIspReg, CAM_SL2_HRZ_COMP, SL2_HRZ_COMP);
            }              	

            ISP_FUNC_DBG("g_MdpMgrCqIdx(%d)",g_MdpMgrCqIdx);
            ISP_FUNC_DBG("top.cam_in_fmt(%d)",g_MdpMgrCfgData[g_MdpMgrCqIdx].ispTpipeCfgInfo.top.cam_in_fmt);
            ISP_FUNC_DBG("top.mode(%d)",g_MdpMgrCfgData[g_MdpMgrCqIdx].ispTpipeCfgInfo.top.mode);
            ISP_FUNC_DBG("top.scenario(%d)",g_MdpMgrCfgData[g_MdpMgrCqIdx].ispTpipeCfgInfo.top.scenario);
            ISP_FUNC_DBG("top.pixel_id(%d)",g_MdpMgrCfgData[g_MdpMgrCqIdx].ispTpipeCfgInfo.top.pixel_id);
            ISP_FUNC_DBG("top.cam_in_fmt(%d)",g_MdpMgrCfgData[g_MdpMgrCqIdx].ispTpipeCfgInfo.top.cam_in_fmt);
            ISP_FUNC_DBG("img2o.img2o_mux_en(%d)",g_MdpMgrCfgData[g_MdpMgrCqIdx].ispTpipeCfgInfo.img2o.img2o_mux_en);
            ISP_FUNC_DBG("top.nbc_en(%d)",g_MdpMgrCfgData[g_MdpMgrCqIdx].ispTpipeCfgInfo.top.nbc_en);
            ISP_FUNC_DBG("top.seee_en(%d)",g_MdpMgrCfgData[g_MdpMgrCqIdx].ispTpipeCfgInfo.top.seee_en);
            ISP_FUNC_DBG("img2o.img2o_mux(%d)",g_MdpMgrCfgData[g_MdpMgrCqIdx].ispTpipeCfgInfo.img2o.img2o_mux);
        }
    }


    ISP_FUNC_DBG("CAM_TDRI_PIPE::_config:End ");

    return 0;
}

int CAM_TDRI_PIPE::_write2CQ( int cq )
{
    ISP_FUNC_DBG("CAM_TDRI_PIPE::_write2CQ");

    return 0;
}

MBOOL CAM_TDRI_PIPE::getNr3dTop(MINT32 ispCq, MUINT32 *pEn)
{
    ISP_FUNC_DBG("ispCq(%d)\n",ispCq);

    switch(ispCq){
        case ISP_DRV_CQ01:
            //this->m_pIspDrvShell->m_pTpipeDrv->getNr3dTop(TPIPE_DRV_CQ01, pEn);
            break;
        case ISP_DRV_CQ02:
            //this->m_pIspDrvShell->m_pTpipeDrv->getNr3dTop(TPIPE_DRV_CQ02, pEn);
            break;
        case ISP_DRV_CQ03:
            //this->m_pIspDrvShell->m_pTpipeDrv->getNr3dTop(TPIPE_DRV_CQ03, pEn);
            break;
        default:
            *pEn = 0;
            ISP_FUNC_ERR("[error]not support this ispCq(%d)\n",ispCq);
            return MFALSE;
    }

    return MTRUE;
}



MBOOL ISP_TOP_CTRL::tmpSimulatePath( void )  //kk test (will be removed in the future)
{
#if 0   //kk test default:0

    //static TdriMgr pTdriMgr;
    //static TdriMgr pTdriMgr2;
    static IspDrv* pIspDrv;
    static IspDrv* m_pvirIspDrv;
    static isp_reg_t* m_pVirtIspReg;
    static int init=0;
    static int num=0;

    printf("kk:tmpSimulatePath\n");


    if(init==0) {
        init = 1;

        pIspDrv = IspDrv::createInstance();
        pIspDrv->init();
        m_pvirIspDrv = pIspDrv->getCQInstance((ISP_DRV_CQ_ENUM)ISP_DRV_CQ01_SYNC);
        m_pVirtIspReg = (isp_reg_t*)m_pvirIspDrv->getRegAddr();
        TdriMgr::getInstance().init();
    }

    num++;
    if(num <= 5){
        //tmp setting
        TdriMgr::getInstance().setCfa(ISP_DRV_CQ01_SYNC, 0);
        ISP_BITS(m_pVirtIspReg, CAM_CTL_EN1_SET, CFA_EN_CLR ) = 0;
        ISP_BITS(m_pVirtIspReg, CAM_CTL_EN1_SET, CFA_EN_SET ) = 1;
        ISP_BITS(m_pVirtIspReg, CAM_CFA_BYPASS, BAYER_BYPASS ) = 0;
        ISP_BITS(m_pVirtIspReg, CAM_CFA_ED_F, BAYER_FLAT_DET_MODE ) = 1;
        ISP_BITS(m_pVirtIspReg, CAM_CFA_ED_NYQ, BAYER_NYQ_TH ) = 0x38;
        TdriMgr::getInstance().applySetting(ISP_DRV_CQ01_SYNC, TDRI_MGR_FUNC_CFA);
        //
        TdriMgr::getInstance().setSeee(ISP_DRV_CQ01_SYNC, 1, 3, 1);
        ISP_BITS(m_pVirtIspReg, CAM_CTL_EN2_SET, SEEE_EN_CLR ) = 0;
        ISP_BITS(m_pVirtIspReg, CAM_CTL_EN2_SET, SEEE_EN_SET ) = 1;
        ISP_BITS(m_pVirtIspReg, CAM_SEEE_EDGE_CTRL, SE_EDGE ) = 3;
        ISP_BITS(m_pVirtIspReg, CAM_SEEE_EDGE_CTRL1, SE_HEDGE_SEL ) = 1;  /* 0x15004ad4 */
        ISP_BITS(m_pVirtIspReg, CAM_SEEE_EDGE_CTRL1, SE_EGAIN_HA ) = 1;
        ISP_BITS(m_pVirtIspReg, CAM_SEEE_EDGE_CTRL1, SE_EGAIN_HB ) = 1;
        ISP_BITS(m_pVirtIspReg, CAM_SEEE_SRK_CTRL, USM_OVER_SHRINK_EN ) = 0;
        TdriMgr::getInstance().applySetting(ISP_DRV_CQ01_SYNC, TDRI_MGR_FUNC_SEEE);
        //
        TdriMgr::getInstance().setBnr(ISP_DRV_CQ01_SYNC, 0, 0, 0, 33, 0, 0);
        ISP_BITS(m_pVirtIspReg, CAM_CTL_EN1_CLR, BNR_EN_SET ) = 0;
        ISP_BITS(m_pVirtIspReg, CAM_CTL_EN1_CLR, BNR_EN_CLR ) = 1;
        ISP_BITS(m_pVirtIspReg, CAM_BPC_CON, BPC_ENABLE ) = 0;
        ISP_BITS(m_pVirtIspReg, CAM_BPC_CON, BPC_TABLE_ENABLE ) = 0;
        ISP_BITS(m_pVirtIspReg, CAM_BPC_TBLI2, BPC_XSIZE ) = 33;
        TdriMgr::getInstance().applySetting(ISP_DRV_CQ01_SYNC, TDRI_MGR_FUNC_BNR);
        //
        TdriMgr::getInstance().setOtherEngin(ISP_DRV_CQ01_SYNC, TDRI_MGR_FUNC_NR3D);
        ISP_BITS(m_pVirtIspReg, CAM_NR3D_LIMIT_Y_CON1, NR3D_LIMIT_Y0_TH) = 2;
        TdriMgr::getInstance().applySetting(ISP_DRV_CQ01_SYNC, TDRI_MGR_FUNC_NR3D);
        //
        TdriMgr::getInstance().setNr3dTop(ISP_DRV_CQ01_SYNC, TDRI_MGR_FUNC_NR3D_TOP);
        ISP_BITS(m_pVirtIspReg, CAM_CTL_EN2_CLR, NR3D_EN_SET) = 0;
        ISP_BITS(m_pVirtIspReg, CAM_CTL_EN2_CLR, NR3D_EN_CLR) = 1;
        TdriMgr::getInstance().applySetting(ISP_DRV_CQ01_SYNC, TDRI_MGR_FUNC_NR3D_TOP);
        //
        TdriMgr::getInstance().setOtherEngin(ISP_DRV_CQ01_SYNC, TDRI_MGR_FUNC_PCA);
        ISP_BITS(m_pVirtIspReg, CAM_PCA_TBL[84], PCA_SAT_GAIN) = 0x34;
        TdriMgr::getInstance().applySetting(ISP_DRV_CQ01_SYNC, TDRI_MGR_FUNC_PCA);
        //
    } else if(num==6){
        printf("kk:release\n");
        TdriMgr::getInstance().uninit();
        //
        pIspDrv->uninit();
        pIspDrv->destroyInstance();

    }
#endif

    return MTRUE;
}


/*/////////////////////////////////////////////////////////////////////////////
    ISP_BUF_CTRL
  /////////////////////////////////////////////////////////////////////////////*/
int ISP_BUF_CTRL::init( MUINT32 dmaChannel )
{
    MINT32 rt_dma = getDmaBufIdx(dmaChannel);

    ISP_FUNC_DBG("E");
    ISP_FUNC_DBG("0x%08x/%dn",dmaChannel,rt_dma);

    if(-1 == rt_dma)
    {
        ISP_FUNC_ERR("dma channel error ");
        return -1;
    }
    
    //clear all
    m_hwbufL[rt_dma].bufInfoList.clear();
    m_swbufL[rt_dma].bufInfoList.clear();

    debugPrint(dmaChannel);
    
    ISP_FUNC_DBG("X");
    return 0;
}

MBOOL ISP_BUF_CTRL::waitBufReady( MUINT32 dmaChannel )
{
    MBOOL ret = MTRUE;
    MINT32 rt_dma = getDmaBufIdx(dmaChannel);
    ISP_BUFFER_CTRL_STRUCT buf_ctrl;
    MUINT32 bWaitBufRdy;

    if(-1 == rt_dma) 
    {
        ISP_FUNC_ERR("dma channel error ");
        return MFALSE;
    }
    
    if(_imgo_ == rt_dma || _img2o_ == rt_dma) 
    {        
        buf_ctrl.ctrl = ISP_RT_BUF_CTRL_IS_RDY;
        buf_ctrl.buf_id = (_isp_dma_enum_)rt_dma;
        buf_ctrl.data_ptr = (MUINT32)&bWaitBufRdy;
        ISP_FUNC_DBG("rtBufCtrl.ctrl(%d)/id(%d)/ptr(0x%x)",buf_ctrl.ctrl,buf_ctrl.buf_id,buf_ctrl.data_ptr);
        
        ret = this->m_pIspDrvShell->m_pPhyIspDrv_bak->rtBufCtrl((void*)&buf_ctrl);        
        if(MFALSE == ret) 
        {
            ISP_FUNC_ERR("rtBufCtrl fail:ISP_RT_BUF_CTRL_IS_RDY");
        }
        
        if(bWaitBufRdy) 
        {
            ISP_FUNC_DBG("wait p1_done");
            //NO BUFFER FILLED
            //wait pass1 done
            ISP_DRV_WAIT_IRQ_STRUCT irq_TG1_DONE = {ISP_DRV_IRQ_CLEAR_WAIT,
                                                    ISP_DRV_IRQ_TYPE_INT,
                                                    ISP_DRV_IRQ_INT_STATUS_PASS1_TG1_DON_ST,
                                                    CAM_INT_WAIT_TIMEOUT_MS};
            //
            ret = this->m_pPhyIspDrv->waitIrq( irq_TG1_DONE );
            if(MFALSE == ret) 
            {
                ISP_FUNC_ERR("waitIrq( irq_TG1_DONE ) fail");
            }
         }
    }
    
    return ret;
}


/*
description: move filled current buffer to empty buffer list
*/
MINT32 ISP_BUF_CTRL::enqueueHwBuf( MUINT32 dmaChannel, stISP_BUF_INFO bufInfo )
{
    MINT32 rt_dma = getDmaBufIdx(dmaChannel);
    MUINT32 end = 0;
    IMEM_BUF_INFO buf_info;
    ISP_BUFFER_CTRL_STRUCT buf_ctrl;
    ISP_RT_BUF_INFO_STRUCT  rt_buf_info;
    ISP_RT_BUF_INFO_STRUCT  ex_rt_buf_info;
    MUINT32 size;
    ISP_FUNC_DBG(" : enter ISP_BUF_CTRL::enqueueHwBuf();");
    ISP_FUNC_DBG(" : this->path =  %d;", this->path);
    ISP_FUNC_DBG(" : ISP_PASS1 =  %d;",ISP_PASS1);
    ISP_FUNC_DBG(" : ISP_PASS2 =  %d;",ISP_PASS2);
    ISP_FUNC_DBG(" : ISP_PASS2B =  %d;",ISP_PASS2B);
    ISP_FUNC_DBG(" : rt_dma,_dispo_,_vido_ ,_imgi_ = %d,%d,%d,%d;\n",rt_dma,_dispo_,_vido_ ,_imgi_); //rt_dma _dispo_
    
    if ( -1 == rt_dma ) 
    {
        ISP_FUNC_ERR("dma channel error(0x%x) ",dmaChannel);
        return -1;
    }
    
    buf_info.size = bufInfo.size;
    buf_info.memID = bufInfo.memID;
    buf_info.virtAddr = bufInfo.base_vAddr;
    buf_info.phyAddr = bufInfo.base_pAddr;
    buf_info.bufSecu = bufInfo.bufSecu;
    buf_info.bufCohe = bufInfo.bufCohe;
    
    ISP_FUNC_DBG("dma(%d),id(%d),size(0x%x),VA(0x%x),PA(0x%x),S/C(%d/%d)",
                rt_dma,
                buf_info.memID,
                buf_info.size,
                buf_info.virtAddr,
                buf_info.phyAddr,
                buf_info.bufSecu,
                buf_info.bufCohe);
    
    if ( ISP_PASS1 == this->path ) 
    {
        //check is full
        //ISP_FUNC_INF("[js_test]:sizeof(ISP_RT_BUF_STRUCT)(%d) ",sizeof(ISP_RT_BUF_STRUCT));
        
        if ( _imgo_ == rt_dma || _img2o_ == rt_dma ) 
        {
            buf_ctrl.ctrl = ISP_RT_BUF_CTRL_GET_SIZE;
            buf_ctrl.buf_id = (_isp_dma_enum_)rt_dma;
            buf_ctrl.data_ptr = (MUINT32)&size;
            ISP_FUNC_DBG("rtBufCtrl.ctrl(%d)/id(%d)/ptr(0x%x)",buf_ctrl.ctrl,buf_ctrl.buf_id,buf_ctrl.data_ptr);
            if ( MTRUE != this->m_pIspDrvShell->m_pPhyIspDrv_bak->rtBufCtrl((void*)&buf_ctrl) )
            {
                ISP_FUNC_ERR("ERROR:rtBufCtrl");
                return -1;
            }
            
            if ( size == ISP_RT_CQ0C_BUF_SIZE ) 
            {
                ISP_FUNC_ERR("real time buffer number FULL:rt_dma(%d)/size(%d)",rt_dma,size);
                return -1;
            }
        }
    }
    else 
    {
        //check for pass2 only
        ISP_FUNC_DBG("[_test]:sm_hwbufL[rt_dma].bufInfoList.size() = %d",m_hwbufL[rt_dma].bufInfoList.size());        
        if( ISP_RT_BUF_SIZE <= m_hwbufL[rt_dma].bufInfoList.size() ) 
        {
            ISP_FUNC_ERR("Max buffer size(0x%x)",m_hwbufL[rt_dma].bufInfoList.size());
            return -1;
        }
        
        if( this->m_pIspDrvShell->m_pIMemDrv->mapPhyAddr(&buf_info) ) 
        {
            ISP_FUNC_ERR("ERROR:m_pIMemDrv->mapPhyAddr");
            ISP_FUNC_ERR("[ION_test]:ERROR:m_pIMemDrv->mapPhyAddr");
            return -1; //back to 89 design
//            ISP_FUNC_DBG("Keep going first wo mapphy error");
        }
        ISP_FUNC_DBG("[_test]:check for pass2 only\n");

        MUINT32 mdpmgrIdx = 0;

        if(ISP_PASS2 == this->path)
        {
            mdpmgrIdx = CAM_ISP_CQ1;
        }
        else if(ISP_PASS2B == this->path)
        {
            mdpmgrIdx = CAM_ISP_CQ2;
        }

        ISP_FUNC_DBG("mdpmgrIdx(%d)",mdpmgrIdx);
        
        if(_imgi_ == rt_dma )
        {                
            g_MdpMgrCfgData[mdpmgrIdx].srcVirAddr  = buf_info.virtAddr;
            g_MdpMgrCfgData[mdpmgrIdx].srcPhyAddr  = buf_info.phyAddr;
            g_MdpMgrCfgData[mdpmgrIdx].srcBufSize  = buf_info.size;
            g_MdpMgrCfgData[mdpmgrIdx].srcBufMemID = buf_info.memID;            
            
            ISP_FUNC_DBG("imgi,cq(%d),sceID(%d)",g_MdpMgrCfgData[mdpmgrIdx].cqIdx,g_MdpMgrCfgData[mdpmgrIdx].sceID); 
            
            ISP_FUNC_DBG("imgi,VA(0x%8x),PA(0x%8x),Size(0x%8x),ID(%d)",g_MdpMgrCfgData[mdpmgrIdx].srcVirAddr,
                                                                         g_MdpMgrCfgData[mdpmgrIdx].srcPhyAddr,
                                                                         g_MdpMgrCfgData[mdpmgrIdx].srcBufSize,
                                                                         g_MdpMgrCfgData[mdpmgrIdx].srcBufMemID); 
        }
        
        if(_dispo_ == rt_dma ) //   _dispo_
        {

            g_MdpMgrCfgData[mdpmgrIdx].dstVirAddr[ISP_MDP_DL_DISPO]  = buf_info.virtAddr;
            g_MdpMgrCfgData[mdpmgrIdx].dstPhyAddr[ISP_MDP_DL_DISPO]  = buf_info.phyAddr;
            g_MdpMgrCfgData[mdpmgrIdx].dstBufSize[ISP_MDP_DL_DISPO]  = buf_info.size;
            g_MdpMgrCfgData[mdpmgrIdx].dstBufMemID[ISP_MDP_DL_DISPO] = buf_info.memID;

            ISP_FUNC_DBG("dispo,cq(%d),sceID(%d)",g_MdpMgrCfgData[mdpmgrIdx].cqIdx,g_MdpMgrCfgData[mdpmgrIdx].sceID);

            ISP_FUNC_DBG("dispo,VA(0x%8x),PA(0x%8x),SZ(0x%8x),ID(%d)", g_MdpMgrCfgData[mdpmgrIdx].dstVirAddr[ISP_MDP_DL_DISPO],
                                                                        g_MdpMgrCfgData[mdpmgrIdx].dstPhyAddr[ISP_MDP_DL_DISPO],
                                                                        g_MdpMgrCfgData[mdpmgrIdx].dstBufSize[ISP_MDP_DL_DISPO],
                                                                        g_MdpMgrCfgData[mdpmgrIdx].dstBufMemID[ISP_MDP_DL_DISPO]);          
        }
        
        if(_vido_ == rt_dma )
        {
            g_MdpMgrCfgData[mdpmgrIdx].dstVirAddr[ISP_MDP_DL_VIDO]  = buf_info.virtAddr;
            g_MdpMgrCfgData[mdpmgrIdx].dstPhyAddr[ISP_MDP_DL_VIDO]  = buf_info.phyAddr;
            g_MdpMgrCfgData[mdpmgrIdx].dstBufSize[ISP_MDP_DL_VIDO]  = buf_info.size;
            g_MdpMgrCfgData[mdpmgrIdx].dstBufMemID[ISP_MDP_DL_VIDO] = buf_info.memID;

            ISP_FUNC_DBG("vido,cq(%d),sceID(%d)",g_MdpMgrCfgData[mdpmgrIdx].cqIdx,g_MdpMgrCfgData[mdpmgrIdx].sceID);

            ISP_FUNC_DBG("vido,VA(0x%8x),PA(0x%8x),SZ(0x%8x),ID(%d)", g_MdpMgrCfgData[mdpmgrIdx].dstVirAddr[ISP_MDP_DL_VIDO],
                                                                       g_MdpMgrCfgData[mdpmgrIdx].dstPhyAddr[ISP_MDP_DL_VIDO],
                                                                       g_MdpMgrCfgData[mdpmgrIdx].dstBufSize[ISP_MDP_DL_VIDO],
                                                                       g_MdpMgrCfgData[mdpmgrIdx].dstBufMemID[ISP_MDP_DL_VIDO]);            
        }     
    }
    
    bufInfo.base_pAddr =  (MUINT32)buf_info.phyAddr;
    //
    bufInfo.status = ISP_BUF_EMPTY;
    m_hwbufL[rt_dma].bufInfoList.push_back(bufInfo);

    if(ISP_PASS1 == this->path) 
    {
        //put to ring buffer in kernel
        rt_buf_info.memID = buf_info.memID;
        rt_buf_info.size = buf_info.size;
        rt_buf_info.base_vAddr = buf_info.virtAddr;
        rt_buf_info.base_pAddr = buf_info.phyAddr;
        ISP_FUNC_DBG("rt_buf_info.ID(%d)/size(0x%x)/vAddr(0x%x)/pAddr(0x%x)",rt_buf_info.memID,rt_buf_info.size,rt_buf_info.base_vAddr,rt_buf_info.base_pAddr);
        if ( 0 == rt_buf_info.base_pAddr ) 
        {
            ISP_FUNC_DBG("NULL PA");
        }        
        
        buf_ctrl.ctrl = ISP_RT_BUF_CTRL_ENQUE;
        buf_ctrl.buf_id = (_isp_dma_enum_)rt_dma;
        buf_ctrl.data_ptr = (MUINT32)&rt_buf_info;
        buf_ctrl.ex_data_ptr = (MUINT32)0;
        ISP_FUNC_DBG("[rtbc][ENQUQ]+(%d)/id(%d)/ptr(0x%x)",buf_ctrl.ctrl,buf_ctrl.buf_id,buf_ctrl.data_ptr);

        //enque exchanged buffer
        if( NULL != bufInfo.next )
        {
            ex_rt_buf_info.memID = bufInfo.next->memID;
            ex_rt_buf_info.size = bufInfo.next->size;
            ex_rt_buf_info.base_vAddr = bufInfo.next->base_vAddr;
            ex_rt_buf_info.base_pAddr = bufInfo.next->base_pAddr;
            
            ISP_FUNC_INF("exchange 1st buf. by 2nd buf. and enque it.ID(%d)/size(0x%x)/vAddr(0x%x)/pAddr(0x%x)",ex_rt_buf_info.memID,ex_rt_buf_info.size,ex_rt_buf_info.base_vAddr,ex_rt_buf_info.base_pAddr);
            
            if ( 0 == ex_rt_buf_info.base_pAddr ) 
            {
                ISP_FUNC_ERR("NULL PA");
            }
            
            //buf_ctrl.ctrl = ISP_RT_BUF_CTRL_EXCHANGE_ENQUE;
            buf_ctrl.ex_data_ptr = (MUINT32)&ex_rt_buf_info;
            
            ISP_FUNC_INF("[rtbc][ENQUQ]+(%d)/id(%d)/ptr(0x%x)/ex_ptr(0x%x)",buf_ctrl.ctrl,buf_ctrl.buf_id,buf_ctrl.data_ptr,buf_ctrl.ex_data_ptr);
        }
        
        if( MTRUE != this->m_pIspDrvShell->m_pPhyIspDrv_bak->rtBufCtrl((void*)&buf_ctrl) ) 
        {
            ISP_FUNC_ERR("ERROR:rtBufCtrl");
            return -1;
        }
        ISP_FUNC_DBG("[rtbc][ENQUQ]-");
    }
    else 
    {        
        ISP_FUNC_DBG("P2 dma(%d),memID(0x%x),size(%d),vAddr(0x%x),pAddr(0x%x)", \
                                    rt_dma, \
                                    m_hwbufL[rt_dma].bufInfoList.back().memID, \
                                    m_hwbufL[rt_dma].bufInfoList.size(), \
                                    m_hwbufL[rt_dma].bufInfoList.back().base_vAddr,
                                    m_hwbufL[rt_dma].bufInfoList.back().base_pAddr);
    }
    
    return 0;
}

//
/*
description: move FILLED buffer from hw to sw list
called at passx_done
*/
MINT32 ISP_BUF_CTRL::dequeueHwBuf( MUINT32 dmaChannel )
{
    MINT32 rt_dma = getDmaBufIdx(dmaChannel);
    MUINT32 count = 1;
    MUINT32 start;
    //ISP_RT_BUF_STRUCT* pstRTBuf = (ISP_RT_BUF_STRUCT*)this->m_pIspDrvShell->m_pPhyIspDrv->m_pRTBufTbl;
    stISP_BUF_INFO bufInfo;
    ISP_BUFFER_CTRL_STRUCT buf_ctrl;
    ISP_DEQUE_BUF_INFO_STRUCT deque_buf;

    ISP_FUNC_DBG("path(%d)\n",this->path);
    ISP_FUNC_DBG("dmaChannel(0x%x))\n",dmaChannel);
    ISP_FUNC_DBG("rt_dma(0x%x))\n",rt_dma);

    Mutex::Autolock lock(queHwLock);
    
    if( -1 == rt_dma ) 
    {
        ISP_FUNC_ERR("dma channel error ");
        return -1;
    }
    
    if( ISP_PASS1 == this->path ) 
    {
        if( _imgo_ == rt_dma || _img2o_ == rt_dma ) 
        {
            //deque filled buffer
            buf_ctrl.ctrl = ISP_RT_BUF_CTRL_DEQUE;
            buf_ctrl.buf_id = (_isp_dma_enum_)rt_dma;
            buf_ctrl.data_ptr = (MUINT32)&deque_buf;
            
            ISP_FUNC_DBG("dma(%d),ctrl(%d),id(%d),ptr(0x%x)",rt_dma,buf_ctrl.ctrl,buf_ctrl.buf_id,buf_ctrl.data_ptr);
            if( MTRUE != this->m_pIspDrvShell->m_pPhyIspDrv_bak->rtBufCtrl((void*)&buf_ctrl) ) 
            {
                ISP_FUNC_ERR("ERROR:rtBufCtrl");
                return -1;
            }
            
            count = deque_buf.count;
            ISP_FUNC_DBG("deque_buf.count(%d)",deque_buf.count);
            if ( ISP_RT_BUF_SIZE < count ) 
            {
                ISP_FUNC_ERR("ERROR:deque_buf.count(%d)",deque_buf.count);
                return -1;
            }
            //
            m_swbufL[rt_dma].bufInfoList.clear();
            //
            for(MUINT32 i=0;i<count;i++) 
            {
                ISP_FUNC_DBG("i(%d),dma(%d),id(0x%x),size(0x%x),VA(0x%x),PA(0x%x),size(%d)",i,rt_dma,deque_buf.data[i].memID,deque_buf.data[i].size,deque_buf.data[i].base_vAddr,deque_buf.data[i].base_pAddr,m_swbufL[rt_dma].bufInfoList.size());
                bufInfo.status = ISP_BUF_FILLED;
                bufInfo.memID = deque_buf.data[i].memID;
                bufInfo.size = deque_buf.data[i].size;
                bufInfo.base_vAddr = deque_buf.data[i].base_vAddr;
                bufInfo.base_pAddr = deque_buf.data[i].base_pAddr;
                bufInfo.timeStampS = deque_buf.data[i].timeStampS;
                bufInfo.timeStampUs = deque_buf.data[i].timeStampUs;
                m_swbufL[rt_dma].bufInfoList.push_back(bufInfo);
            }
        }
    }
    else 
    {
        //pass2
        while(count--) 
        {
            ISP_FUNC_DBG(" : pass2 (count)=(0x%8x)\n",count);
            ISP_FUNC_DBG(" : m_hwbufL[%d].bufInfoList.size()=(0x%8x)\n",rt_dma,m_hwbufL[rt_dma].bufInfoList.size());
  
            if(m_hwbufL[rt_dma].bufInfoList.size() && ISP_PASS2 == this->path) 
            {
                //change type
                m_hwbufL[rt_dma].bufInfoList.front().status = ISP_BUF_FILLED;       
            
                if(rt_dma==_dispo_)
                {                    
                    if(MDPMGR_NO_ERROR != g_pMdpMgr->dequeueBuf(MDPMGR_DEQ_DISPO, g_MdpMgrCfgData[CAM_ISP_CQ1]))
                    {
                        ISP_FUNC_ERR("g_pMdpMgr->dequeueBuf for dispo fail");
                        return -1;
                    }
                }
                
                if(rt_dma==_vido_)
                {
                    if(MDPMGR_NO_ERROR != g_pMdpMgr->dequeueBuf(MDPMGR_DEQ_VIDO, g_MdpMgrCfgData[CAM_ISP_CQ1]))
                    {
                        ISP_FUNC_ERR("g_pMdpMgr->dequeueBuf for vido fail");
                        return -1;
                    }
                }
                
                if(rt_dma==_imgi_)
                {
                    if(MDPMGR_NO_ERROR != g_pMdpMgr->dequeueBuf(MDPMGR_DEQ_SRC, g_MdpMgrCfgData[CAM_ISP_CQ1]))
                    {
                        ISP_FUNC_ERR("g_pMdpMgr->dequeueBuf for src fail");
                        return -1;
                    }     
                }                  
                
                //m_hwbufL[rt_dma].bufInfoList.front().timeStampS = pstRTBuf->ring_buf[rt_dma].data[(start-count-1)%ISP_RT_BUF_SIZE].timeStampS;
                //m_hwbufL[rt_dma].bufInfoList.front().timeStampUs = pstRTBuf->ring_buf[rt_dma].data[(start-count-1)%ISP_RT_BUF_SIZE].timeStampUs;
                
                //add to sw buffer list
                m_swbufL[rt_dma].bufInfoList.push_back(m_hwbufL[rt_dma].bufInfoList.front());
                //delete fomr hw list
                m_hwbufL[rt_dma].bufInfoList.pop_front();
                //
                ISP_FUNC_DBG("dma(%d),swsize(%d) ",rt_dma,m_swbufL[rt_dma].bufInfoList.size());

#if defined(__ISP_USE_STD_M4U__) || defined (__ISP_USE_ION__)
                ISP_FUNC_DBG("unmapPhyAddr");
                //m4u only
                IMEM_BUF_INFO buf_info;
                buf_info.memID = m_swbufL[rt_dma].bufInfoList.back().memID;
                buf_info.size = m_swbufL[rt_dma].bufInfoList.back().size;
                buf_info.virtAddr = m_swbufL[rt_dma].bufInfoList.back().base_vAddr;
                buf_info.phyAddr = m_swbufL[rt_dma].bufInfoList.back().base_pAddr;
                //free mva
                if ( this->m_pIspDrvShell->m_pIMemDrv->unmapPhyAddr(&buf_info) ) 
                {
                    ISP_FUNC_ERR("ERROR:m_pIMemDrv->unmapPhyAddr");
                    return -1;
                }
#endif

            }
            else if(ISP_PASS2B == this->path)
            {
                MUINT32 ret;
                stISP_BUF_INFO bufInfo2B;

                bufInfo2B.status = ISP_BUF_FILLED;
                
                ISP_FUNC_DBG("ISP_PASS2B dequeue");
                if(rt_dma==_dispo_)
                {
                    ret = g_pMdpMgr->dequeueBuf(MDPMGR_DEQ_DISPO, g_MdpMgrCfgData[CAM_ISP_CQ2]);
                    if(ret == MDPMGR_VSS_DEQ_FALSE_FAIL)
                    {                        
                        return -2;
                    }
                    else if(ret != MDPMGR_NO_ERROR)
                    {
                        ISP_FUNC_ERR("g_pMdpMgr->dequeueBuf for dispo fail");
                        return -1;                        
                    }
                        

                    bufInfo2B.base_vAddr = g_MdpMgrCfgData[CAM_ISP_CQ2].dstVirAddr[ISP_MDP_DL_DISPO];
                    bufInfo2B.base_pAddr = g_MdpMgrCfgData[CAM_ISP_CQ2].dstPhyAddr[ISP_MDP_DL_DISPO];
                    bufInfo2B.size       = g_MdpMgrCfgData[CAM_ISP_CQ2].dstBufSize[ISP_MDP_DL_DISPO];
                    bufInfo2B.memID      = g_MdpMgrCfgData[CAM_ISP_CQ2].dstBufMemID[ISP_MDP_DL_DISPO];
                }

                if(rt_dma==_vido_)
                {
                    ret = g_pMdpMgr->dequeueBuf(MDPMGR_DEQ_VIDO, g_MdpMgrCfgData[CAM_ISP_CQ2]);
                    if(ret == MDPMGR_VSS_DEQ_FALSE_FAIL)
                    {                        
                        return -2;
                    }
                    else if(ret != MDPMGR_NO_ERROR)
                    {
                        ISP_FUNC_ERR("g_pMdpMgr->dequeueBuf for vido fail");
                        return -1;
                    }

                    bufInfo2B.base_vAddr = g_MdpMgrCfgData[CAM_ISP_CQ2].dstVirAddr[MDPMGR_DEQ_VIDO];
                    bufInfo2B.base_pAddr = g_MdpMgrCfgData[CAM_ISP_CQ2].dstPhyAddr[MDPMGR_DEQ_VIDO];
                    bufInfo2B.size       = g_MdpMgrCfgData[CAM_ISP_CQ2].dstBufSize[MDPMGR_DEQ_VIDO];
                    bufInfo2B.memID      = g_MdpMgrCfgData[CAM_ISP_CQ2].dstBufMemID[MDPMGR_DEQ_VIDO];
                }

                if(rt_dma==_imgi_)
                {
                    if(MDPMGR_NO_ERROR != g_pMdpMgr->dequeueBuf(MDPMGR_DEQ_SRC, g_MdpMgrCfgData[CAM_ISP_CQ2]))
                    {
                        ISP_FUNC_ERR("g_pMdpMgr->dequeueBuf for src fail");
                        return -1;
                    }

                    bufInfo2B.base_vAddr = g_MdpMgrCfgData[CAM_ISP_CQ2].srcVirAddr;
                    bufInfo2B.base_pAddr = g_MdpMgrCfgData[CAM_ISP_CQ2].srcPhyAddr;
                    bufInfo2B.size       = g_MdpMgrCfgData[CAM_ISP_CQ2].srcBufSize;
                    bufInfo2B.memID      = g_MdpMgrCfgData[CAM_ISP_CQ2].srcBufMemID;
                }                

                //add to sw buffer list
                m_swbufL[rt_dma].bufInfoList.push_back(bufInfo2B);

#if defined(__ISP_USE_STD_M4U__) || defined (__ISP_USE_ION__)
                ISP_FUNC_DBG("ISP_PASS2B unmapPhyAddr");

                //m4u only
                IMEM_BUF_INFO buf_info;
                
                buf_info.memID    = m_swbufL[rt_dma].bufInfoList.back().memID;
                buf_info.size     = m_swbufL[rt_dma].bufInfoList.back().size;
                buf_info.virtAddr = m_swbufL[rt_dma].bufInfoList.back().base_vAddr;
                buf_info.phyAddr  = m_swbufL[rt_dma].bufInfoList.back().base_pAddr;
                
                //free mva
                if(this->m_pIspDrvShell->m_pIMemDrv->unmapPhyAddr(&buf_info)) 
                {
                    ISP_FUNC_ERR("ERROR:ISP_PASS2B m_pIMemDrv->unmapPhyAddr");
                    return -1;
                }
#endif
            }
            else 
            {
                ISP_FUNC_ERR("empty HW buffer");
                return -1;
            }
        }        
    }    

    ISP_FUNC_INF("path(%d),start(NA),SCIO(NA),En1(0x%x),En2(0x%x),En2(0x%x),fmt_sel(0x%x),ctl_sel(0x%x),tcm_en(0x%x),mux_sel(0x%x),mux_sel2(0x%x),mux_conf(0x%x)",this->path,
    this->m_pIspDrvShell->getPhyIspDrv()->readReg(0x00004004),
    this->m_pIspDrvShell->getPhyIspDrv()->readReg(0x00004008),
    this->m_pIspDrvShell->getPhyIspDrv()->readReg(0x0000400C),
    this->m_pIspDrvShell->getPhyIspDrv()->readReg(0x00004010),
    this->m_pIspDrvShell->getPhyIspDrv()->readReg(0x00004014),
    this->m_pIspDrvShell->getPhyIspDrv()->readReg(0x00004054),
    this->m_pIspDrvShell->getPhyIspDrv()->readReg(0x00004074),
    this->m_pIspDrvShell->getPhyIspDrv()->readReg(0x00004078),
    this->m_pIspDrvShell->getPhyIspDrv()->readReg(0x0000407C));
    return 0;
}

//
/*
description: move current buffer to filled buffer list
*/
MINT32 ISP_BUF_CTRL::dequeueSwBuf( MUINT32 dmaChannel ,stISP_FILLED_BUF_LIST& bufList )
{
    MINT32 rt_dma = getDmaBufIdx(dmaChannel);
    MUINT32 cnt = 0;
    struct _isp_buf_list_ *old_hw_head = NULL;
    //
    Mutex::Autolock lock(queSwLock);
    //
    if ( -1 == rt_dma ) 
    {
        ISP_FUNC_ERR("dma channel error ");
        return -1;
    }
    //
    if ( 0 == m_swbufL[rt_dma].bufInfoList.size() ) 
    {
        //wait semephore till
        ISP_FUNC_ERR("empty SW buffer");
        return -1;
    }
    //
    while ( m_swbufL[rt_dma].bufInfoList.size() ) 
    {
        //all element at the end
        bufList.pBufList->push_back(m_swbufL[rt_dma].bufInfoList.front());
        //delete first element
        m_swbufL[rt_dma].bufInfoList.pop_front();
        //
        ISP_FUNC_DBG("05_dma(%d)/memID(0x%x)/size(%d)/vaddr(0x%x)/paddr(0x%x)",rt_dma,
                                    bufList.pBufList->back().memID, \
                                    bufList.pBufList->size(), \
                                    bufList.pBufList->back().base_vAddr,
                                    bufList.pBufList->back().base_pAddr);
    }
    //
    return 0;
}


MUINT32 ISP_BUF_CTRL::getCurrHwBuf( MUINT32 dmaChannel )
{
    MINT32 rt_dma = getDmaBufIdx(dmaChannel);
    //ISP_RT_BUF_STRUCT* pstRTBuf = (ISP_RT_BUF_STRUCT*)this->m_pIspDrvShell->m_pIspDrv->m_pRTBufTbl;
    
    if ( -1 == rt_dma ) 
    {
        ISP_FUNC_ERR("dma channel error ");
        return 0;
    }
    
    if (0 == m_hwbufL[rt_dma].bufInfoList.size())
    {
        ISP_FUNC_ERR("ERROR:No buffer in queue");
        return 0;
    }
    
    #if 0
    if ( ISP_PASS1 == this->path ) 
    {
        if ( _imgo_ == rt_dma || _img2o_ == rt_dma ) 
        {
            //at initialization
            //fetch pass1 ring buffer
            //start + 1
            if ( 0 < pstRTBuf->ring_buf[rt_dma].count ) 
            {
                ISP_FUNC_DBG("rt_dma(%d),start(%d),count(%d)",rt_dma,pstRTBuf->ring_buf[rt_dma].start,pstRTBuf->ring_buf[rt_dma].count);
            }
        }
    }
    #endif
    
    ISP_FUNC_DBG("dma:(%d)/memID(0x%x)/vAddr:(0x%x)/pAddr:(0x%x) ",rt_dma, \
                                                        m_hwbufL[rt_dma].bufInfoList.front().memID, \
                                                        m_hwbufL[rt_dma].bufInfoList.front().base_vAddr, \
                                                        m_hwbufL[rt_dma].bufInfoList.front().base_pAddr);

    return m_hwbufL[rt_dma].bufInfoList.front().base_pAddr;
}

MUINT32 ISP_BUF_CTRL::getNextHwBuf( MUINT32 dmaChannel )
{
    MINT32 rt_dma = getDmaBufIdx(dmaChannel);
	ISP_BUF_INFO_L::iterator it;
    MUINT32 base_pAddr = 0;
    
    if( -1 == rt_dma ) 
    {
        ISP_FUNC_ERR("ERROR:dma channel error ");
        return -1;
    }
    
    if(0 == m_hwbufL[rt_dma].bufInfoList.size()) 
    {
        ISP_FUNC_ERR("ERROR:No buffer in queue");
        return 0;
    }
    
    if( 1 < m_hwbufL[rt_dma].bufInfoList.size() ) 
    {
    	it = m_hwbufL[rt_dma].bufInfoList.begin();
        it++;
        base_pAddr = it->base_pAddr;
    }

    ISP_FUNC_DBG("dma:[%d]/base_pAddr:[0x%x] ",rt_dma,base_pAddr);

    return base_pAddr;
}


MUINT32 ISP_BUF_CTRL::freeSinglePhyBuf(stISP_BUF_INFO bufInfo)
{
    MUINT32 ret = 0;
#if 0

    IMEM_BUF_INFO buf_info;

    ISP_FUNC_DBG(":E");
    //
    //this->m_pIspDrvShell->m_pIMemDrv->freePhyMem(virtAddr,memID,size,phyAddr);

    //buf_info.type = (EBUF_TYPE)bufInfo.type;
    buf_info.size = bufInfo.size;
    buf_info.memID = bufInfo.memID;
//    buf_info.ionFd = bufInfo.ionFD;
    buf_info.virtAddr = bufInfo.base_vAddr;
    buf_info.phyAddr = bufInfo.base_pAddr;
    if ( this->m_pIspDrvShell->m_pIMemDrv->unmapPhyAddr(&buf_info) ) {
        ISP_FUNC_ERR("ERROR:m_pIMemDrv->unmapPhyAddr ");
    }

    ISP_FUNC_DBG(":X");
#else
    ISP_FUNC_ERR("ERROR:NOT SUPPORT YET!");
#endif
    return ret;
}


MUINT32 ISP_BUF_CTRL::freeAllPhyBuf( void )
{
    MUINT32 ret = 0;
#if 0
    ISP_FUNC_DBG(":E");
    //
    this->m_pIspDrvShell->m_pIMemDrv->reset();

    ISP_FUNC_DBG(":X");
#else
    ISP_FUNC_ERR("ERROR:NOT SUPPORT YET!");
#endif
    return ret;
}

int ISP_BUF_CTRL::getDmaBufIdx( MUINT32 dmaChannel )
{
    _isp_dma_enum_ dma;

    switch(dmaChannel) 
    {
        case ISP_DMA_IMGI:
            dma = _imgi_;
            break;
        case ISP_DMA_IMGCI:
            dma = _imgci_;
            break;
        case ISP_DMA_VIPI:
            dma = _vipi_;
            break;
        case ISP_DMA_VIP2I:
            dma = _vip2i_;
            break;
        case ISP_DMA_IMGO:
            dma = _imgo_;
            break;
        case ISP_DMA_IMG2O:
            dma = _img2o_;
            break;
        case ISP_DMA_DISPO:
            dma = _dispo_;
            break;
        case ISP_DMA_VIDO:
            dma = _vido_;
            break;
        case ISP_DMA_FDO:
            dma = _fdo_;
            break;
        case ISP_DMA_LSCI:
            dma = _lsci_;
            break;
        case ISP_DMA_LCEI:
            dma = _lcei_;
            break;
        default:
            ISP_FUNC_ERR("Invalid dma channel(%d)",dmaChannel);
            return -1;
    }

    return (int)dma;
}
//
int
ISP_BUF_CTRL::
debugPrint( MUINT32 dmaChannel )
{
    MINT32 rt_dma = getDmaBufIdx(dmaChannel);
    ISP_BUF_INFO_L::iterator it;

    ISP_FUNC_DBG("E");
    if ( -1 == rt_dma ) {
        ISP_FUNC_ERR("dma channel error ");
        return -1;
    }
    //
    for ( it = m_hwbufL[rt_dma].bufInfoList.begin(); it != m_hwbufL[rt_dma].bufInfoList.end(); it++ ) {
        ISP_FUNC_DBG("m_hwbufL[%d].base_vAddr:[0x%x]/base_pAddr:[0x%x] ",rt_dma,it->base_vAddr,it->base_pAddr);
    }
    //
    for ( it = m_swbufL[rt_dma].bufInfoList.begin(); it != m_swbufL[rt_dma].bufInfoList.end(); it++ ) {
        ISP_FUNC_DBG("m_hwbufL[%d].base_vAddr:[0x%x]/base_pAddr:[0x%x] ",rt_dma,it->base_vAddr,it->base_pAddr);
    }
    //
    ISP_FUNC_DBG("X");

    return 0;
}

/*/////////////////////////////////////////////////////////////////////////////
    isp event thread
  /////////////////////////////////////////////////////////////////////////////*/
class IspEventThreadImp : public IspEventThread
{
    IspEventThreadImp(){}
    //
    public:     ////        Instantiation.
        static IspEventThread*  getInstance(IspDrv* pIspDrv);
        virtual MBOOL   init(void);
        virtual MBOOL   uninit(void);
    private:
        static MVOID* ispEventThread(void *arg);
    private:
        static IspDrv  *m_pPhyIspDrv;
        static sem_t    m_semIspEventthread;
        static MBOOL    m_bPass1Start;
        //
        pthread_t       m_ispEventthread;
        mutable Mutex   mLock;
        volatile MINT32 mInitCount;
};
//
IspDrv *IspEventThreadImp::m_pPhyIspDrv = NULL;
sem_t IspEventThreadImp::m_semIspEventthread;

//
IspEventThread*  IspEventThread::createInstance(IspDrv* pIspDrv)
{
    return IspEventThreadImp::getInstance(pIspDrv);
}
//
IspEventThread*  IspEventThreadImp::getInstance(IspDrv* pIspDrv)
{
    ISP_FUNC_DBG("E: pIspDrv(0x%08x)",pIspDrv);
    //
    //static IspEventThreadImp singleton;
    static IspEventThreadImp singleton;
    //
    m_pPhyIspDrv = pIspDrv;
    //
    return &singleton;
}
//
MBOOL IspEventThreadImp::init(void)
{
    ISP_FUNC_DBG("E");

#if 0
    Mutex::Autolock lock(mLock);
    ISP_FUNC_DBG("mInitCount(%d) ", mInitCount);
    //
    if(mInitCount > 0)
    {
        android_atomic_inc(&mInitCount);
        return MTRUE;
    }

    // Init semphore
    ::sem_init(&m_semIspEventthread, 0, 0);

    // Create main thread for preview and capture
    pthread_attr_t const attr = {0, NULL, 1024 * 1024, 4096, SCHED_RR, PRIO_RT_ISP_EVENT_THREAD};
    pthread_create(&m_ispEventthread, &attr, ispEventThread, NULL);
    //
    android_atomic_inc(&mInitCount);
#endif

    ISP_FUNC_DBG("X");
    return MTRUE;
}
//
MBOOL IspEventThreadImp::uninit(void)
{
    ISP_FUNC_DBG("E");
#if 0
    Mutex::Autolock lock(mLock);
    ISP_FUNC_DBG("mInitCount(%d)",mInitCount);
    //
    if(mInitCount <= 0)
    {
        // No more users
        return MTRUE;
    }
    // More than one user
    android_atomic_dec(&mInitCount);
    //
    if(mInitCount > 0)
    {
        return MTRUE;
    }

    //
    pthread_join(m_ispEventthread, NULL);

    ISP_FUNC_DBG("X");
#endif

    return MTRUE;
}
//
MVOID* IspEventThreadImp::ispEventThread(void *arg)
{
    ISP_DRV_WAIT_IRQ_STRUCT irq_SOF;
    ISP_DRV_WAIT_IRQ_STRUCT irq_VS;
    ISP_DRV_WAIT_IRQ_STRUCT irq_TG1_DONE;


    ISP_FUNC_DBG("E: tid=%d m_pPhyIspDrv(0x%x)", gettid(),m_pPhyIspDrv);

    ::prctl(PR_SET_NAME,"ispEventthread",0,0,0);

    //  detach thread => cannot be join
    ::pthread_detach(::pthread_self());
    //
    irq_SOF.Clear = ISP_DRV_IRQ_CLEAR_NONE;
    irq_SOF.Type = ISP_DRV_IRQ_TYPE_INT;
    irq_SOF.Status = ISP_DRV_IRQ_INT_STATUS_SOF1_INT_ST;
    irq_SOF.Timeout = CAM_INT_WAIT_TIMEOUT_MS;
    //
    irq_VS.Clear = ISP_DRV_IRQ_CLEAR_NONE;
    irq_VS.Type = ISP_DRV_IRQ_TYPE_INT;
    irq_VS.Status = ISP_DRV_IRQ_INT_STATUS_VS1_ST;
    irq_VS.Timeout = CAM_INT_WAIT_TIMEOUT_MS;
    //
    irq_TG1_DONE.Clear = ISP_DRV_IRQ_CLEAR_NONE;
    irq_TG1_DONE.Type = ISP_DRV_IRQ_TYPE_INT;
    irq_TG1_DONE.Status = ISP_DRV_IRQ_INT_STATUS_PASS1_TG1_DON_ST;
    irq_TG1_DONE.Timeout = CAM_INT_WAIT_TIMEOUT_MS;
    //
    while (1)
    {
        //if (pass1_start) {
            //ISP_FUNC_DBG("[ispEventthread] Wait m_semIspEventthread");
            ::sem_wait(&m_semIspEventthread); // wait here until someone use sem_post() to wake this semaphore up
            //ISP_FUNC_DBG("[ispEventthread] Got m_semIspEventthread");

            //0:VD
            m_pPhyIspDrv->waitIrq( irq_VS );
            //call back function
            //set semephore
            ISP_FUNC_DBG("irq_VS ");
            ISP_FUNC_DBG("irq_VS ");

            //1:SOF
            m_pPhyIspDrv->waitIrq( irq_SOF );
            //call back function
            //set semephore
            ISP_FUNC_DBG("irq_SOF ");
            ISP_FUNC_DBG("irq_SOF ");

            //2:EXP_DONE


            //3:PASS1_DONE
            //if (capture_enable) {
                m_pPhyIspDrv->waitIrq( irq_TG1_DONE );
                //call back function
                //set semephore
                ISP_FUNC_DBG("irq_TG1_DONE ");
                ISP_FUNC_DBG("irq_TG1_DONE ");
            //}
        //}

    }

    //
    //::sem_post(&m_semIspEventthread);

    ISP_FUNC_DBG("X");

    return NULL;
}


/*/////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////*/
//
#define ISP_KERNEL_MOTIFY_SINGAL_TEST
#ifdef ISP_KERNEL_MOTIFY_SINGAL_TEST
int isp_trigger_signal(int *pid)
{
int ret = 0;
int mFd=0;
    mFd = open("/dev/camera-isp", O_RDWR);
    ret = ioctl(mFd,ISP_SET_USER_PID,pid);
    return 0;
}
#endif

