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
#ifndef _LSC_MGR_H_
#define _LSC_MGR_H_

#include <utils/threads.h>
#include <utils/List.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <cutils/atomic.h>

#include <ispif.h>
#include <camera_custom_nvram.h>
#include <isp_mgr.h>
#include <aaa_log.h>
#include <mtkcam/hal/aaa_hal_base.h>
#include <mtkcam/drv/imem_drv.h>
#include <mtkcam/drv/isp_drv.h>
#include <mtkcam/hal/sensor_hal.h>
#include <nvram_drv_mgr.h>
#include <dbg_cam_param.h>
#include <string>

#define USING_BUILTIN_LSC   0
#define DEBUG_ALIGN_FUNC    0
#define ENABLE_TSF          1

#include <mtkcam/algorithm/liblsctrans/ShadingTblTransform.h>

#if ENABLE_TSF
#include <mtkcam/algorithm/libtsf/MTKTsf.h>
#endif

typedef void*(*VPT)(void *);

namespace NSIspTuning {
using namespace NS3A;

/*******************************************************************************
 * LSC Manager
 *******************************************************************************/
//class IMemDrv;
//class IspMgr;
//class IspDrv;
typedef struct {
        MUINT32 TableSizeU32;
        MUINT32 TableGrid;
} SHADING_SPEC;

typedef struct {    
    MUINT32 m_u4CCT;
    MINT32  m_i4LV;
    MINT32  m_RGAIN;
    MINT32  m_GGAIN;
    MINT32  m_BGAIN;
    MINT32  m_FLUO_IDX;
    MINT32  m_DAY_FLUO_IDX;
}TSF_AWB_INFO;

typedef struct {
    TSF_AWB_INFO awb_info;
}TSF_REF_INFO_T;


class LscMgr {
        enum {
            LUT_PREVIEW = 0,
            LUT_CAPTURE,
            LUT_VIDEO,
            LUT_BINNING,
            NUM_OF_LUTS_Mode
        };

        enum {
            LUT_LOW = 0,
            LUT_MIDDLE,
            LUT_HIGH,
            NUM_OF_LUTS
        };

    public:
        typedef enum {
            LSC_SCENARIO_01 = 0,    // pv  frame
            LSC_SCENARIO_03 = 1,    // cap frame
            LSC_SCENARIO_04 = 2,    // cap tile
            LSC_SCENARIO_09_17 = 3, // vdo frame

            LSC_SCENARIO_30,        // n3d pv cap
            LSC_SCENARIO_37,        // n3d cap tile
            LSC_SCENARIO_NUM

        } ELscScenario_T;

        typedef enum {
            TRANS_INPUT,
            TRANS_OUTPUT
        } LSCMGR_TRANS_TYPE;

        //    typedef struct {
        //        MUINT32 u4GrabX;          // For input sensor width
        //        MUINT32 u4GrabY;          // For input sensor height
        //        MUINT32 u4SrcW;          // For input sensor width
        //        MUINT32 u4SrcH;          // For input sensor height
        //        MUINT32 u4CropW;        //TG crop width
        //        MUINT32 u4CropH;        //TG crop height
        //        MUINT32 DataFmt;
        //    } SENSOR_CROP_INFO;
    private:
        static void  *main;
        static void  *mainsecond;
        static void  *sub;
        static void  *n3d;
        static ESensorDev_T curSensorDev;

    public:
        static LscMgr*
        createInstance(ESensorDev_T const eSensorDev, NVRAM_CAMERA_ISP_PARAM_STRUCT& rIspNvram)
        {
            MY_LOG("[%s] ", __FUNCTION__);
            NVRAM_CAMERA_SHADING_STRUCT *pNvram_Shading = NULL;
            NvramDrvMgr::getInstance().getRefBuf(pNvram_Shading);
            MY_LOG("[%s] NvramDrvMgr pNvram_Shading 0x%x", __FUNCTION__, pNvram_Shading);

            if (!pNvram_Shading) {
                MY_LOG("[%s] pNvram_Shading is NULL!!!", __FUNCTION__);
                static NVRAM_CAMERA_SHADING_STRUCT shading;
                pNvram_Shading = &shading;
            }
            curSensorDev = eSensorDev;

            switch  (eSensorDev)
            {
                case ESensorDev_Main:       //  Main Sensor
                    static LscMgr singleton_main(ESensorDev_Main, rIspNvram, pNvram_Shading->Shading);
                    MY_LOG("[%s] LscMgr singleton_main", __FUNCTION__);
                    main = (void*)&singleton_main;
                    return &singleton_main;
                case ESensorDev_MainSecond: //  Main Second Sensor
                    static LscMgr singleton_mainsecond(ESensorDev_MainSecond, rIspNvram, pNvram_Shading->Shading);
                    MY_LOG("[%s] LscMgr singleton_mainsecond", __FUNCTION__);
                    mainsecond = (void*)&singleton_mainsecond;
                    return &singleton_mainsecond;
                case ESensorDev_Sub:        //  Sub Sensor
                    static LscMgr singleton_sub(ESensorDev_Sub, rIspNvram, pNvram_Shading->Shading);
                    MY_LOG("[%s] LscMgr singleton_sub", __FUNCTION__);
                    sub = (void*)&singleton_sub;
                    return &singleton_sub;
                case ESensorDev_Main3D:       //  Main Sensor
                    static LscMgr singleton_3d(ESensorDev_Main3D, rIspNvram, pNvram_Shading->Shading);
                    MY_LOG("[%s] LscMgr singleton_3d", __FUNCTION__);
                    n3d = (void*)&singleton_3d;
                    return &singleton_3d;
            }

            return  MNULL;
        }

        static MBOOL
        destroyInstance(ESensorDev_T const eSensorDev) {
            switch(eSensorDev) {
                default:
                case ESensorDev_Main:       //  Main Sensor
                    if (!main)
                        return MFALSE;
                    delete static_cast<LscMgr*>(main);
                    main = NULL;
                    return MTRUE;
                case ESensorDev_MainSecond: //  Main Second Sensor
                    if (!mainsecond)
                        return MFALSE;
                    delete static_cast<LscMgr*>(mainsecond);
                    mainsecond = NULL;
                    return MTRUE;
                case ESensorDev_Sub:        //  Sub Sensor
                    if (!sub)
                        return MFALSE;
                    delete static_cast<LscMgr*>(sub);
                    sub = NULL;
                    return MTRUE;
                case ESensorDev_Main3D:       //  Main Sensor
                    if (!n3d)
                        return MFALSE;
                    delete static_cast<LscMgr*>(n3d);
                    n3d = NULL;
                    return MTRUE;
            }
        }

        static LscMgr*
        getInstance() {
            //            switch(curSensorDev) {
            //                default:
            //                case ESensorDev_Main:       //  Main Sensor
            //                    return static_cast<LscMgr*>(main);
            //                case ESensorDev_MainSecond: //  Main Second Sensor
            //                    return static_cast<LscMgr*>(mainsecond);
            //                case ESensorDev_Sub:        //  Sub Sensor
            //                    return static_cast<LscMgr*>(sub);
            //                case ESensorDev_Main3D:       //  Main Sensor
            //                    return static_cast<LscMgr*>(n3d);
            //            }
            return getInstance(curSensorDev);
        }

        static LscMgr*
        getInstance(ESensorDev_T sensor) {
            switch(sensor) {
                default:
                case ESensorDev_Main:       //  Main Sensor
                    return static_cast<LscMgr*>(main);
                case ESensorDev_MainSecond: //  Main Second Sensor
                    return static_cast<LscMgr*>(mainsecond);
                case ESensorDev_Sub:        //  Sub Sensor
                    return static_cast<LscMgr*>(sub);
                case ESensorDev_Main3D:       //  Main Sensor
                    return static_cast<LscMgr*>(n3d);
            }
        }

        MBOOL init();
        MBOOL uninit();
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  Change Count.
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    private:
        ////
        template<typename T>
        MBOOL setIfChange(T& dst, T const src) {
            if (src != dst) {
                dst = src;
                return MTRUE;
            } else
                return MFALSE;
        }
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  Index
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    public:
        ////

        MUINT32 getRegIdx() const {
            return m_eLscScenario;
        }


        MUINT32 getCTIdx();
        MBOOL setCTIdx(MUINT32 const u4CTIdx);
        
    private:
        ////    Common.

        //  LSC CT index
        MUINT32 m_u4CTIdx;
        TSF_AWB_INFO m_TsfAwbInfo;

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  Mode
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    public:
        ////
        MUINT32 getMode() const {
            MY_LOG("***NEED to be FIXED***[LscMgr] %s: This is the wrong control path", __FUNCTION__);
            return 0;
        }

        MBOOL                   setIspProfile(EIspProfile_T const eIspProfile);
        EIspProfile_T           getIspProfile(void);
        EIspProfile_T           getPrevIspProfile(void);

        MBOOL                   setMetaIspProfile(EIspProfile_T const eIspProfile, MUINT32 sensor_mode);
        MVOID                   setMetaLscScenario(ELscScenario_T);
    private:

        ESensorDev_T            m_eActive;
        ACDK_SCENARIO_ID_ENUM   m_eSensorOp;
        ACDK_SCENARIO_ID_ENUM   m_ePrevSensorOp;
        EIspProfile_T           m_eIspProfile;
        EIspProfile_T           m_ePrevIspProfile;
        ELscScenario_T          m_eMetaLscScenario;
        ELscScenario_T          m_eLscScenario;
        ELscScenario_T          m_ePrevLscScenario;
        unsigned long           m_eSensorType;
        unsigned long           m_u4SensorID;
        MUINT32                 m_SensorMode;
        MBOOL                   m_bIsEEPROMImported;
        MBOOL                   m_bIsLutLoaded;
        MBOOL                   m_bBypass;
        MBOOL                   m_bMetaMode;
        MBOOL                   m_bDumpSdblk;
        std::string             m_strSdblkFile;
        

        NVRAM_CAMERA_ISP_PARAM_STRUCT& m_rIspNvram;

        MBOOL fillTblInfoByLscScenarionCT(SHADING_TBL_SPEC &tbl_sepc,
                ELscScenario_T ref_lsc,
                ELscScenario_T cur_lsc,
                MUINT8 ct,
                LSCMGR_TRANS_TYPE type);
        MBOOL importEEPromData(void);

        MBOOL saveToNVRAM(void);

        ACDK_SCENARIO_ID_ENUM   getSensorScenarioByIspProfile(EIspProfile_T const eIspProfile);
        ELscScenario_T          getLscScenarioBySensorScenario(ACDK_SCENARIO_ID_ENUM sensor_scenario);
        ACDK_SCENARIO_ID_ENUM   getSensorScenarioByLscScenario(ELscScenario_T lsc_scenario);
        MBOOL                   getScenarioResolution(ACDK_SCENARIO_ID_ENUM scenario);
        MVOID                   updateLscScenarioByIspProfile(EIspProfile_T);
        MBOOL                   updateLscScenarioBySensorMode(void);

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  Address
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    public:
        ////    Operations.

        MVOID*
        getPhyTBA() const {
            printf("[%s] LscScenario %d, CT %d, Addr 0x%08x\n", __FUNCTION__,
                    m_eLscScenario,
                    m_u4CTIdx,
                    stRawLscInfo[m_eLscScenario].phyAddr+m_u4CTIdx*getPerLutSize(m_eLscScenario));
            return reinterpret_cast<MVOID*>(stRawLscInfo[m_eLscScenario].phyAddr+m_u4CTIdx*getPerLutSize(m_eLscScenario));
        }

        MVOID*
        getVirTBA() const {
            printf("[%s] LscScenario %d, CT %d, Addr 0x%08x\n", __FUNCTION__,
                    m_eLscScenario,
                    m_u4CTIdx,
                    stRawLscInfo[m_eLscScenario].virtAddr+m_u4CTIdx*getPerLutSize(m_eLscScenario));
            return reinterpret_cast<MVOID*>(stRawLscInfo[m_eLscScenario].virtAddr+m_u4CTIdx*getPerLutSize(m_eLscScenario));
        }

        MVOID*
        getPhyTBA(EIspProfile_T eIspProfile, MUINT32 ct = 0xff) {
            ACDK_SCENARIO_ID_ENUM sensor_scenario = getSensorScenarioByIspProfile(eIspProfile);
            ELscScenario_T lsc_scenario =  getLscScenarioBySensorScenario(sensor_scenario);
            if (ct == 0xff)
                ct = m_u4CTIdx;
            return reinterpret_cast<MVOID*>(stRawLscInfo[lsc_scenario].phyAddr+ct*getPerLutSize(lsc_scenario));
        }

        MVOID*
        getVirTBA(EIspProfile_T eIspProfile, MUINT32 ct = 0xff) {
            ACDK_SCENARIO_ID_ENUM sensor_scenario = getSensorScenarioByIspProfile(eIspProfile);
            ELscScenario_T lsc_scenario =  getLscScenarioBySensorScenario(sensor_scenario);
            if (ct == 0xff)
                ct = m_u4CTIdx;
            return reinterpret_cast<MVOID*>(stRawLscInfo[lsc_scenario].virtAddr+ct*getPerLutSize(lsc_scenario));
        }

        ELscScenario_T getLscScenarioByIspProfile(EIspProfile_T const eIspProfile) {
            ACDK_SCENARIO_ID_ENUM sensor_scenario = getSensorScenarioByIspProfile(eIspProfile);
            ELscScenario_T lsc_scenario =  getLscScenarioBySensorScenario(sensor_scenario);
            return lsc_scenario;
        }


        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  LUT
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    public:
        ////    get
        MUINT32* getLut(ELscScenario_T lsc_scenario) const;
        MUINT32 getTotalLutSize(ELscScenario_T eLscScenario) const;
        MUINT32 getPerLutSize(ELscScenario_T eLscScenario) const;

        MVOID UpdateSL2Param(void);

    private:
        ISP_SHADING_STRUCT& m_rIspShadingLut;
        //+++++++++++++++++++++
        //  Memory management
        //+++++++++++++++++++++++
        IMemDrv         *m_pIMemDrv;
        //+++++++++++++++++++++
        //  ISP management
        //+++++++++++++++++++++++
        ////    Common.
        halSensorDev_e  m_SensorDev;

        SENSOR_CROP_INFO    m_SensorCrop[ACDK_SCENARIO_ID_MAX];
        SensorHal           *m_pSensorHal;
    private:
        MUINT32*            m_pu4CurAddr;
        IMEM_BUF_INFO stRawLscInfo[SHADING_SUPPORT_OP_NUM];
        IMEM_BUF_INFO m_rBufInfo[2];
        MUINT32 m_u4DoubleBufIdx;
        MBOOL m_fgUserSetTbl;
        MBOOL m_fgOnOff;
        
        MVOID RawLscTblDump(const char* filename);
        MINT32 dumpSdblk(const char* table_name, const ISP_NVRAM_LSC_T& LscConfig, const MUINT32 *ShadingTbl);

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  Operations.
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    public:

        ////	Reference.
        //	Reference to rIspNvram.Shading
        typedef ISP_NVRAM_LSC_T LSCParameter[SHADING_SUPPORT_OP_NUM];
        LSCParameter& m_rIspLscCfg;

        typedef ISP_NVRAM_SL2_T SL2Parameter[NVRAM_SL2_TBL_NUM];
        SL2Parameter& m_rIspSl2Cfg;

        ////    load
        MVOID loadLut();
        MVOID loadLutToSysram();
        MBOOL SetTBAToISP();
        MBOOL ConfigUpdate();
        LSCParameter& getLscNvram();

        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    public:
        MBOOL isBypass();
        MVOID enBypass(MBOOL);
        MBOOL isEnable() ;
        MBOOL enableLscWoVariable(MBOOL const fgEnable);
        MBOOL enableLsc(MBOOL const fgEnable);

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //Memory Management functions
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

        MINT32 RawLscfreeMemory(IMEM_BUF_INFO& RawLscInfo) ;
        MBOOL RawLscTblMemInfoShow(IMEM_BUF_INFO& RawLscInfo);
        MBOOL RawLscTblFlushCurrTbl(void);
        MBOOL RawLscTblSetPhyVirAddr(MUINT32 const u8LscIdx, MVOID* pPhyAddr,
                MVOID* pVirAddr) ;
        MUINT32 RawLscTblGetPhyAddr(MUINT32 const u8LscIdx);
        MUINT32 RawLscTblGetVirAddr(MUINT32 const u8LscIdx);
        MBOOL RawLscTblAlloc(IMEM_BUF_INFO& RawLscInfo,
                MUINT32 const u8LscLutSize);
        MBOOL RawLscTblInit();
        MBOOL RawLscTblUnInit();
        //to work around double buffer
        ESensorDev_T getActiveSensorDev() const { return m_eActive; }
        MVOID RawLscTblClear(ELscScenario_T Scenario, UINT8 ColorTemp);

        MINT32 setGainTable(MUINT32 u4GridNumX, MUINT32 u4GridNumY, MUINT32 u4Width, MUINT32 u4Height, float* pGainTbl);

    public:
        MRESULT
        getDebugInfo(DEBUG_SHAD_INFO_T &rShadingDbgInfo);
        MRESULT
        getDebugTbl(DEBUG_SHAD_ARRAY_INFO_T &rShadingDbgTbl);
        ////
        LscMgr(ESensorDev_T const, NVRAM_CAMERA_ISP_PARAM_STRUCT& rIspNvram,
                ISP_SHADING_STRUCT& rIspShadingLut);
        ~LscMgr();
        //////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////

    public:
        typedef enum {
            TSF_INPUT_PV = 0,
            TSF_INPUT_CAP = 1,
            TSF_INPUT_VDO = 2,
            TSF_INPUT_NUM
        } LSCMGR_TSF_INPUT_SRC;

        typedef enum {
            TSF_BUFIDX_INPUT = 0,
            TSF_BUFIDX_OUTPUT,
            TSF_BUFIDX_BAK,
            TSF_BUFIDX_AWB,
            TSF_BUFIDX_NUM
        } LSCMGR_TSF_BUFIDX;

        typedef enum {
            LSCMGR_TSF_STATE_IDLE = 0,
            LSCMGR_TSF_STATE_INIT,
            LSCMGR_TSF_STATE_SCENECHANGE,
            LSCMGR_TSF_STATE_GETNEWINPUT,
            LSCMGR_TSF_STATE_DO,
            LSCMGR_TSF_STATE_EXIT,
            LSCMGR_TSF_STATE_NUM,
        } LSCMGR_TSF_STATE;


#if ENABLE_TSF
    private:
        pthread_t           mTSFThread;
        pthread_mutex_t     mTSFMutex;
        sem_t               mTSFSem;

        pthread_mutex_t     mTSFMutexSC;
        sem_t               mTSFSemSC;


        LSCMGR_TSF_STATE    mTSFState;
        MBOOL               m_bTSF;
        MBOOL               m_bTsfForceAWB;
        MBOOL               m_fgTsfSetTbl;
        MBOOL               m_fgPreflash;
        MBOOL               m_fgCtIdxExcd;
        MBOOL               m_fgSetProcInfo;
        MUINT32             m_u4FrmCnt;
        IMEM_BUF_INFO       m_TSFBuff[TSF_BUFIDX_NUM];

        MTK_TSF_RESULT_INFO_STRUCT* m_pTsfResultInfo;

        MBOOL
        checkAspectRatioChange();

        MVOID
        fillTSFLscConfig(MTK_TSF_LSC_PARAM_STRUCT &config, EIspProfile_T profile);

        MVOID
        fillTSFInitParams(MTK_TSF_ENV_INFO_STRUCT &params);

        MVOID
        updateTSFParamByIspProfile(MTK_TSF_ENV_INFO_STRUCT &params, EIspProfile_T profile);

        MVOID
        updateTSFInputParam(MTK_TSF_SET_PROC_INFO_STRUCT &params);

        MVOID copyToTSFOutput(void);
        MVOID swapTSFBufIdx(void);
        MVOID backupTSFTbl(void);
        MVOID restoreTSFTbl(void);
        MVOID assignTSFTbl(ELscScenario_T eLscScn);
    public:
        static void *   mThreadLoop(void *arg);
        MBOOL           loadTSFLut(void);
        MVOID           enableTSF(MBOOL);
        MVOID           notifyPreflash(MBOOL fgPreflash);
        MBOOL           isTSFEnable(void);
        MBOOL           changeTSFState(LSCMGR_TSF_STATE);

        MVOID           updateTSFBuffIdx();
        MUINT32         getTSFInputAddr(EIspProfile_T profile);
        MUINT32         getTSFOutputAddr(EIspProfile_T profile);
        MVOID           prepareTSFInputBuffer(EIspProfile_T profile, LSCMGR_TSF_STATE state, MBOOL fgAspectChg);
        MVOID           dumpTSFInput(void);

        MRESULT         CCTOPSetTsfForceAwb(MBOOL fgForce);
#endif

        MBOOL           updateTSFinput(LSCMGR_TSF_INPUT_SRC , TSF_REF_INFO_T *, MVOID *);
        MRESULT         CCTOPSetSdblkFileCfg(MBOOL fgSave, const char* filename);

};

/*******************************************************************************
 *
 *******************************************************************************/
}

#endif // _LSC_MGR_H_

