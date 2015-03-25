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

#define LOG_TAG "lsc_mgr"
#ifndef ENABLE_MY_LOG
#define ENABLE_MY_LOG           (1)
#define GLOBAL_ENABLE_MY_LOG    (1)
#endif


//#include <linux/cache.h>
#include <sys/prctl.h>
#include <cutils/properties.h>
#include <aaa_types.h>
#include <aaa_log.h>
#include <mtkcam/common.h>
#include <eeprom_drv.h>
#include <awb_param.h>
#include "lsc_mgr.h"
#include "cam_cal_drv.h"
#include "camera_custom_cam_cal.h"
#include "camera_custom_eeprom.h"
#include "kd_camera_feature.h"
#include "kd_imgsensor_define.h"
#include "camera_common_calibration.h"
#include <mtkcam/v1/config/PriorityDefs.h>
#include "buf_mgr.h"
#include "nvram_drv.h"
#include "dbg_isp_param.h"
#include "dbg_cam_shading_param.h"
#include <mtkcam/hwutils/CameraProfile.h>
using namespace CPTool;
#include "shading_tuning_custom.h"

#if ENABLE_TSF
//#define D65_IDX     (1)
#include <mtkcam/algorithm/libtsf/MTKTsf.h>
#include "tsf_tuning_custom.h"
#endif

namespace NSIspTuning {
#include "lsc_data.h"

using namespace std;

#define GET_PROP(prop, init, val)\
{\
    char value[PROPERTY_VALUE_MAX] = {'\0'};\
    property_get(prop, value, (init));\
    (val) = atoi(value);\
}

#define TSF_SCN_DFT LSC_SCENARIO_01
#define TSF_RUN_BATCH_CAP 0

static MBOOL _bEnableMyLog = 0;

void  *LscMgr::main       = NULL;
void  *LscMgr::mainsecond = NULL;
void  *LscMgr::sub        = NULL;
void  *LscMgr::n3d        = NULL;
ESensorDev_T LscMgr::curSensorDev = ESensorDev_Main;
// calculate tables with difference between per unit calibration and golden sample

ACDK_SCENARIO_ID_ENUM
LscMgr::
getSensorScenarioByLscScenario(ELscScenario_T lsc_scenario)
{
    MY_LOG("[%s]  ", __FUNCTION__);
    switch(lsc_scenario) {
        case LSC_SCENARIO_01:
            return ACDK_SCENARIO_ID_CAMERA_PREVIEW;
        case LSC_SCENARIO_03:
            return ACDK_SCENARIO_ID_CAMERA_ZSD;
        case LSC_SCENARIO_04:
            return ACDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG;
        case LSC_SCENARIO_09_17:
            return ACDK_SCENARIO_ID_VIDEO_PREVIEW;
        case LSC_SCENARIO_30:
            return ACDK_SCENARIO_ID_CAMERA_3D_PREVIEW;
        case LSC_SCENARIO_37:
            return ACDK_SCENARIO_ID_CAMERA_3D_CAPTURE;
        default:
            return ACDK_SCENARIO_ID_CAMERA_PREVIEW;
    }
}

//NSIspTuning::LscMgr::ELscScenario_T
LscMgr::ELscScenario_T
LscMgr::
getLscScenarioBySensorScenario(ACDK_SCENARIO_ID_ENUM sensor_scenario) {
    switch(sensor_scenario) {
        case ACDK_SCENARIO_ID_CAMERA_PREVIEW:
            return LSC_SCENARIO_01;
        case ACDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
            return LSC_SCENARIO_04;
        case ACDK_SCENARIO_ID_VIDEO_PREVIEW:
            return LSC_SCENARIO_09_17;
        case ACDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
            return LSC_SCENARIO_30;
        case ACDK_SCENARIO_ID_CAMERA_ZSD:
            return LSC_SCENARIO_04;
        case ACDK_SCENARIO_ID_CAMERA_3D_PREVIEW:
        case ACDK_SCENARIO_ID_CAMERA_3D_VIDEO:
            return LSC_SCENARIO_30;
        case ACDK_SCENARIO_ID_CAMERA_3D_CAPTURE:
            return LSC_SCENARIO_37;
        default:
            return LSC_SCENARIO_04;
    }
}

MBOOL
LscMgr::
fillTblInfoByLscScenarionCT(SHADING_TBL_SPEC &tbl_sepc,
        ELscScenario_T ref_lsc,
        ELscScenario_T cur_lsc,
        MUINT8 ct,
        LSCMGR_TRANS_TYPE type)
{
    SENSOR_CROP_INFO crop_info;
    UINT32* tbl_addr;
    UINT8   LSCParamIdx = 0;
    MUINT32 sensor_scenario, ref_scenario;
    MY_LOG("[%s]  ", __FUNCTION__);

    ref_scenario = getSensorScenarioByLscScenario(ref_lsc);
    sensor_scenario = getSensorScenarioByLscScenario(cur_lsc);

    if (type == TRANS_INPUT) {
        tbl_sepc.img_width   = m_SensorCrop[sensor_scenario].u4CropW;
        tbl_sepc.img_height  = m_SensorCrop[sensor_scenario].u4CropH;
        tbl_sepc.offset_x    = 0;
        tbl_sepc.offset_y    = 0;
        tbl_sepc.crop_width  = m_SensorCrop[sensor_scenario].u4CropW;
        tbl_sepc.crop_height = m_SensorCrop[sensor_scenario].u4CropH;
        tbl_sepc.bayer       = BAYER_B;
        tbl_sepc.grid_x      = (int)m_rIspLscCfg[cur_lsc].ctl2.bits.SDBLK_XNUM+2;
        tbl_sepc.grid_y      = (int)m_rIspLscCfg[cur_lsc].ctl3.bits.SDBLK_YNUM+2;
        tbl_sepc.lwidth      = m_rIspLscCfg[cur_lsc].lblock.bits.SDBLK_lWIDTH;
        tbl_sepc.lheight     = m_rIspLscCfg[cur_lsc].lblock.bits.SDBLK_lHEIGHT;
        tbl_sepc.ratio_idx   = 0;//m_rIspLscCfg[lsc_scenario].ratio.bits.RATIO00/32;
        tbl_sepc.grgb_same   = SHADING_GRGB_SAME_NO;
#if DEBUG_ALIGN_FUNC
        tbl_sepc.table       = (MUINT32*)golden_cct;
#else
        tbl_sepc.table       = (MUINT32*)(stRawLscInfo[cur_lsc].virtAddr + (MUINT32)ct * getPerLutSize(cur_lsc));
#endif
        tbl_sepc.data_type   = SHADING_TYPE_COEFF;
    } else {

        // suppose no Sensor-side crop+scaling

        // scale down and same aspect ratio
        if (m_SensorCrop[sensor_scenario].u4CropW <= m_SensorCrop[ref_scenario].u4CropW/2) {     // scale down
            tbl_sepc.img_width   = m_SensorCrop[ref_scenario].u4CropW;
            tbl_sepc.img_height  = m_SensorCrop[ref_scenario].u4CropH;
            tbl_sepc.offset_x    = 0;
            tbl_sepc.offset_y    = 0;
            tbl_sepc.crop_width  = m_SensorCrop[ref_scenario].u4CropW;
            tbl_sepc.crop_height = m_SensorCrop[ref_scenario].u4CropH;
            MY_LOG("[%s] Scaled down", __FUNCTION__);

            if (m_SensorCrop[ref_scenario].u4CropW * m_SensorCrop[sensor_scenario].u4CropH !=
                    m_SensorCrop[sensor_scenario].u4CropW * m_SensorCrop[ref_scenario].u4CropH ) {
                MY_ERR("[%s] Not Support case, input/output scale down and different aspect ratio!!", __FUNCTION__);
            }
        }
        else 
        {   
            if (m_SensorCrop[sensor_scenario].u4SubSpW)
            {
                // crop down, same aspect ratio, and scale down
                MY_LOG("[%s] Croped down + Scaled down", __FUNCTION__);
                MUINT32 u4OutW = m_SensorCrop[sensor_scenario].u4CropW;
                MUINT32 u4OutH = m_SensorCrop[sensor_scenario].u4CropH;
                MUINT32 u4OrgW = m_SensorCrop[ref_scenario].u4CropW;
                MUINT32 u4OrgH = m_SensorCrop[ref_scenario].u4CropH;
                tbl_sepc.img_width   = u4OrgW;
                tbl_sepc.img_height  = u4OrgH;
                tbl_sepc.offset_x    = 0;
                tbl_sepc.offset_y    = (u4OrgH - u4OrgW*u4OutH/u4OutW) / 2;
                tbl_sepc.crop_width  = u4OrgW;
                tbl_sepc.crop_height = u4OrgW*u4OutH/u4OutW;    // 3200/1920*1080=1800
            }
            else
            {
                // crop down, different aspect ratio
                tbl_sepc.img_width   = m_SensorCrop[ref_scenario].u4CropW;
                tbl_sepc.img_height  = m_SensorCrop[ref_scenario].u4CropH;
                tbl_sepc.offset_x    = (m_SensorCrop[ref_scenario].u4CropW - m_SensorCrop[sensor_scenario].u4CropW)/2;
                tbl_sepc.offset_y    = (m_SensorCrop[ref_scenario].u4CropH - m_SensorCrop[sensor_scenario].u4CropH)/2;
                tbl_sepc.crop_width  = m_SensorCrop[sensor_scenario].u4CropW;
                tbl_sepc.crop_height = m_SensorCrop[sensor_scenario].u4CropH;
                MY_LOG("[%s] Croped down", __FUNCTION__);
            }
        }

        tbl_sepc.bayer       = BAYER_B;
        tbl_sepc.grid_x      = (int)m_rIspLscCfg[cur_lsc].ctl2.bits.SDBLK_XNUM+2;
        tbl_sepc.grid_y      = (int)m_rIspLscCfg[cur_lsc].ctl3.bits.SDBLK_YNUM+2;
        tbl_sepc.lwidth      = m_rIspLscCfg[cur_lsc].lblock.bits.SDBLK_lWIDTH;
        tbl_sepc.lheight     = m_rIspLscCfg[cur_lsc].lblock.bits.SDBLK_lHEIGHT;
        tbl_sepc.ratio_idx   = 0;//m_rIspLscCfg[lsc_scenario].ratio.bits.RATIO00/32;
        tbl_sepc.grgb_same   = SHADING_GRGB_SAME_YES;
#if DEBUG_ALIGN_FUNC
        tbl_sepc.table       = (MUINT32*)golden_cct;
#else
        tbl_sepc.table       = (MUINT32*)(stRawLscInfo[cur_lsc].virtAddr + (MUINT32)ct * getPerLutSize(cur_lsc));
#endif
        tbl_sepc.data_type   = SHADING_TYPE_COEFF;
    }
    MY_LOG("[%s]  \n"
            "img_width  = %d\n"
            "img_height = %d\n"
            "offset_x   = %d\n"
            "offset_y   = %d\n"
            "crop_width = %d\n"
            "crop_height= %d\n"
            "bayer      = %d\n"
            "grid_x     = %d\n"
            "grid_y     = %d\n"
            "lwidth     = %d\n"
            "lheight    = %d\n"
            "ratio_idx  = %d\n"
            "grgb_same  = %d\n", __FUNCTION__,
            tbl_sepc.img_width   ,
            tbl_sepc.img_height  ,
            tbl_sepc.offset_x    ,
            tbl_sepc.offset_y    ,
            tbl_sepc.crop_width  ,
            tbl_sepc.crop_height ,
            tbl_sepc.bayer       ,
            tbl_sepc.grid_x      ,
            tbl_sepc.grid_y      ,
            tbl_sepc.lwidth      ,
            tbl_sepc.lheight     ,
            tbl_sepc.ratio_idx   ,
            tbl_sepc.grgb_same
    );
    return MTRUE;
}

// when import eeprom data, calculate golen-aligned tables
MBOOL
LscMgr::
importEEPromData(void)
{
    MBOOL bForce1to3FromDft = isByp123ToNvram();
    UINT32 ret = 0;
    MINT32  SensorDevId = m_SensorDev;
    CAMERA_CAM_CAL_TYPE_ENUM a_eCamCalDataType = CAMERA_CAM_CAL_DATA_SHADING_TABLE;
    CAM_CAL_DATA_STRUCT cal_data;
    MUINT8 table_type = 0;
    //UINT8 tbl_buf[2048], tbl_buf_golden[2048];

    char value[PROPERTY_VALUE_MAX] = {'\0'};
    MINT32 dbg_1to3 = 0;
    property_get("debug.lsc_mgr.1to3", value, "-1");
    dbg_1to3 = atoi(value);

    if (dbg_1to3 != -1) {
        MY_LOG("[LscMgr:%s] skip 1to3 %d", __FUNCTION__,
                dbg_1to3);
        return MTRUE;
    }

    MY_LOG("[LscMgr] %s, NVRAM buf 0x%08x 0x%08x\n", __FUNCTION__, &m_rIspNvram, &m_rIspShadingLut);

    // read NVRAM flag to check if we need to read eeprom
    if (m_rIspNvram.ISPComm.CommReg[EEPROM_INFO_IN_COMM_LOAD] == CAL_DATA_LOAD && bForce1to3FromDft == 0)
    {
        MY_LOG("[%s] Tables already exist in NVRAM, Verion(%d). \n", __FUNCTION__, m_rIspNvram.Version);
        m_bIsEEPROMImported = MTRUE;
        return MTRUE;
    }

    if (m_bIsEEPROMImported == MTRUE)
        return MTRUE;

    MY_LOG("[%s] Tables don't exist in NVRAM. Need to read eeprom\n", __FUNCTION__);

    switch(m_eActive) {
        case ESensorDev_Main      :
            SensorDevId = SENSOR_DEV_MAIN;
            break;
        case ESensorDev_Sub       :
            SensorDevId = SENSOR_DEV_SUB;
            break;
        case ESensorDev_MainSecond:
            SensorDevId = SENSOR_DEV_MAIN_2;
            break;
        case ESensorDev_Main3D    :
            SensorDevId = SENSOR_DEV_MAIN_3D;
            break;
        default:
            SensorDevId = SENSOR_DEV_MAIN;
            break;
    }
    MY_LOG("[%s]  GetCamCalCalData", __FUNCTION__);
    CamCalDrvBase *m_pCamCalDrvBaseObj = CamCalDrvBase::createInstance();
    if (!m_pCamCalDrvBaseObj) {
        MY_LOG("[%s] CamCalDrvBase is NULL", __FUNCTION__);
        return MFALSE;
    }

#if DEBUG_ALIGN_FUNC
#else
    ret = m_pCamCalDrvBaseObj->GetCamCalCalData(SensorDevId, a_eCamCalDataType, &cal_data);
    MY_LOG("[%s] (0x%8x)=m_pCamCalDrvObj->GetCamCalCalData", __FUNCTION__, ret);
    if(ret&CamCalReturnErr[a_eCamCalDataType])
    {
        MY_LOG("[%s] err (%s)", __FUNCTION__,
                CamCalErrString[a_eCamCalDataType]);
        m_bIsEEPROMImported = MTRUE;
        return MFALSE;
    }
    else
    {
        MY_LOG("[%s] NO err (%s)", __FUNCTION__,
                CamCalErrString[a_eCamCalDataType]);
    }
#endif

    // (1) table type
    MUINT32 Rotation[2] = {0, 0};
    CAM_CAL_DATA_VER_ENUM module_type = cal_data.DataVer;
    CAM_CAL_LSC_DATA            *lsc_data[2] = {0, 0};
    CAM_CAL_LSC_SENSOR_TYPE     *sensor_lsc = NULL;
    CAM_CAL_LSC_MTK_TYPE        *mtk_lsc = NULL;

    MY_LOG("[%s]  module_type %d", __FUNCTION__, module_type);
    switch(module_type) {
        case CAM_CAL_SINGLE_EEPROM_DATA:
            MY_LOG("[%s]  CAM_CAL_SINGLE_EEPROM_DATA", __FUNCTION__);
        case CAM_CAL_SINGLE_OTP_DATA:
            MY_LOG("[%s]  CAM_CAL_SINGLE_OTP_DATA", __FUNCTION__);
            lsc_data[0] = &cal_data.SingleLsc.LscTable;
            Rotation[0] = cal_data.SingleLsc.TableRotation;
            break;
        default:
        case CAM_CAL_N3D_DATA:
            MY_LOG("[%s]  CAM_CAL_N3D_DATA or nuknown", __FUNCTION__);
            lsc_data[0] = &cal_data.N3DLsc.Data[0].LscTable;
            lsc_data[1] = &cal_data.N3DLsc.Data[1].LscTable;
            Rotation[0] = cal_data.N3DLsc.Data[0].TableRotation;
            Rotation[1] = cal_data.N3DLsc.Data[1].TableRotation;
            break;
    }
    table_type = lsc_data[0]->MtkLcsData.MtkLscType;

#if DEBUG_ALIGN_FUNC
    {
        ISP_NVRAM_LSC_T eeprom, golden;
        SHADIND_ALIGN_CONF  align_conf;
        ACDK_SCENARIO_ID_ENUM cali_scenario = ACDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG;
        MY_LOG("[%s]  PerUnit mtk lsc, type %d, Rotation (%d, %d)", __FUNCTION__,
                module_type,
                Rotation[0], Rotation[1]);

        char *gWorkinBuffer   = new char[SHADIND_FUNC_WORKING_BUFFER_SIZE]; // 139,116 (int) = 556,464 (bytes)
        if (!gWorkinBuffer)
        {
            MY_LOG("[LscMgr:%s]  gWorkinBuffer is NULL", __FUNCTION__);
            m_bIsEEPROMImported = MTRUE;
            return MFALSE;
        }

        align_conf.working_buff_addr = (void*)gWorkinBuffer;
        align_conf.working_buff_size = SHADIND_FUNC_WORKING_BUFFER_SIZE;


        UINT32* align_out_coef      = new UINT32 [MAX_TIL_COEFF_SIZE/sizeof(UINT32)];
        UINT32* trfm_out_coef       = new UINT32 [MAX_TIL_COEFF_SIZE/sizeof(UINT32)];

        //=============== cali ==================//
        align_conf.cali.img_width    = 2564;
        align_conf.cali.img_height   = 1926;
        align_conf.cali.offset_x     = 0;
        align_conf.cali.offset_y     = 0;
        align_conf.cali.crop_width   = 2564;
        align_conf.cali.crop_height  = 1926;
        align_conf.cali.bayer        = BAYER_B;
        align_conf.cali.grid_x       = 16;
        align_conf.cali.grid_y       = 16;
        align_conf.cali.lwidth       = 0; //doesn't use
        align_conf.cali.lheight      = 0; //doesn't use
        align_conf.cali.ratio_idx    = 0;
        align_conf.cali.grgb_same    = SHADING_GRGB_SAME_NO;
        align_conf.cali.data_type    = SHADING_TYPE_GAIN;
        align_conf.cali.table       = (UINT32*)per_unit_dnp;


        //=============== golden ==================//
        align_conf.golden.img_width      = 2564;
        align_conf.golden.img_height     = 1926;
        align_conf.golden.offset_x       = 0;
        align_conf.golden.offset_y       = 0;
        align_conf.golden.crop_width     = 2564;
        align_conf.golden.crop_height    = 1926;
        align_conf.golden.bayer          = BAYER_R;
        align_conf.golden.grid_x         = 16;
        align_conf.golden.grid_y         = 16;
        align_conf.golden.lwidth         = 0; //doesn't use
        align_conf.golden.lheight        = 0; //doesn't use
        align_conf.golden.ratio_idx      = 0;
        align_conf.golden.grgb_same      = SHADING_GRGB_SAME_NO;
        align_conf.golden.data_type      = SHADING_TYPE_GAIN;
        align_conf.golden.table       = (UINT32*)golden_dnp;

        //=============== input ==================//
        align_conf.input.img_width       = 2564;
        align_conf.input.img_height      = 1926;
        align_conf.input.offset_x        = 0;
        align_conf.input.offset_y        = 0;
        align_conf.input.crop_width      = 2564;
        align_conf.input.crop_height     = 1926;
        align_conf.input.bayer           = BAYER_R;
        align_conf.input.grid_x          = 16;
        align_conf.input.grid_y          = 16;
        align_conf.input.lwidth          = 0; //doesn't use
        align_conf.input.lheight         = 0; //doesn't use
        align_conf.input.ratio_idx       = 0;
        align_conf.input.grgb_same       = SHADING_GRGB_SAME_NO;
        align_conf.input.data_type       = SHADING_TYPE_COEFF;
        align_conf.input.table           = (UINT32*)golden_cct;

        //=============== output ==================//
        align_conf.output.img_width      = 2564;
        align_conf.output.img_height     = 1926;
        align_conf.output.offset_x       = 0;
        align_conf.output.offset_y       = 0;
        align_conf.output.crop_width     = 2564;
        align_conf.output.crop_height    = 1926;
        align_conf.output.bayer          = BAYER_R;
        align_conf.output.grid_x         = 16;
        align_conf.output.grid_y         = 16;
        align_conf.output.lwidth         = 0; //doesn't use
        align_conf.output.lheight        = 0; //doesn't use
        align_conf.output.ratio_idx      = 4;
        align_conf.output.grgb_same      = SHADING_GRGB_SAME_YES;
        align_conf.output.data_type      = SHADING_TYPE_COEFF;
        align_conf.output.table          = (UINT32*)align_out_coef;

        MY_LOG("[LscMgr:%s]  shading_align_golden", __FUNCTION__);
        if (ret != shading_align_golden(align_conf))
            MY_ERR("[%s] Fail shading_align_golden %d", __FUNCTION__,
                    ret);
        // chose the right LSC scenario to apply
        for (int i = 0; i < sizeof(align_result)/sizeof(align_result[0]); i++)
        {
            if (align_result[i] != align_out_coef[i])
            {
                MY_ERR("[%s] idx %d, (result, out) = (0x%08x, 0x%08x)", __FUNCTION__,
                        i, align_result[i], align_out_coef[i]);
            } else
                MY_LOG("[%s] OK idx %d, (result, out) = (0x%08x, 0x%08x)", __FUNCTION__,
                                        i, align_result[i], align_out_coef[i]);
        }
        MY_LOG("[%s] ALIGN_CHECK done!!", __FUNCTION__);

    }
#else
    if (table_type & (1<<0))          // sensor lsc
    {
        SET_SENSOR_CALIBRATION_DATA_STRUCT scali_struct;
        //        typedef struct {
        //            MUINT8      MtkLscType; //LSC Table type    "[0]sensor[1]MTK"   1
        //            MUINT8      PixId; //0,1,2,3: B,Gb,Gr,R
        //            MUINT16     TableSize; //TABLE SIZE      2
        //            MUINT8      SensorTable[MAX_SENSOR_SHADING_TALE_SIZE]; //LSC Data (Max 2048)        2048
        //            MUINT8      Reserve[CAM_CAL_MAX_LSC_SIZE-sizeof(MUINT8)-sizeof(MUINT8)-sizeof(MUINT16)-(sizeof(MUINT8)*MAX_SENSOR_SHADING_TALE_SIZE)]; //
        //        }CAM_CAL_LSC_SENSOR_TYPE;
        MY_LOG("[%s]  sensor lsc", __FUNCTION__);
        scali_struct.DataFormat = 0x00010001;
        scali_struct.DataSize = lsc_data[0]->SensorLcsData.TableSize;

        if (MAX_SHADING_DATA_TBL >= scali_struct.DataSize)
        {
            memcpy(&scali_struct.ShadingData,
                    &lsc_data[0]->SensorLcsData.SensorTable,
                    scali_struct.DataSize);
        }
        else
        {
            MY_ERR("Max:%d, scali_struct.DataSize is %d",
                    MAX_SHADING_DATA_TBL,
                    scali_struct.DataSize);
            m_bIsEEPROMImported = MTRUE;
            memcpy(&scali_struct.ShadingData,
                    &lsc_data[0]->SensorLcsData.SensorTable,
                    MAX_SHADING_DATA_TBL);
            //return MFALSE;
        }

        if (!m_pSensorHal)
        {
            MY_ERR("m_pSensorHal is NULL");
            m_pSensorHal = SensorHal::createInstance();
            if (m_pSensorHal->init())
            {
                MY_ERR("m_pSensorHal re-instanate fail!!");
                m_pSensorHal->destroyInstance();
                m_pSensorHal = NULL;
                return MFALSE;
            }
        }

        m_pSensorHal->sendCommand(m_SensorDev, (int)SENSOR_CMD_SET_SENSOR_CALIBRATION_DATA,
                (int)(&scali_struct),
                0, 0);
        m_bIsEEPROMImported = MTRUE;
        return MTRUE;
    }
    else if (table_type & (1<<1))   // mtk lsc
    {

        ISP_NVRAM_LSC_T eeprom, golden;
        SHADIND_ALIGN_CONF  align_conf;
        ACDK_SCENARIO_ID_ENUM cali_scenario = ACDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG;
        MY_LOG("[%s]  PerUnit mtk lsc, type %d, Rotation (%d, %d)", __FUNCTION__,
                module_type,
                Rotation[0], Rotation[1]);

        char *gWorkinBuffer   = new char[SHADIND_FUNC_WORKING_BUFFER_SIZE]; // 139,116 (int) = 556,464 (bytes)
        if (!gWorkinBuffer)
        {
            MY_ERR("[LscMgr:%s]  gWorkinBuffer is NULL");
            m_bIsEEPROMImported = MTRUE;
            return MFALSE;
        }

        align_conf.working_buff_addr = (void*)gWorkinBuffer;
        align_conf.working_buff_size = SHADIND_FUNC_WORKING_BUFFER_SIZE;

        BAYER_ID_T bayer = BAYER_B;
        BAYER_ID_T bayer_golden = BAYER_B;
        mtk_lsc = &lsc_data[0]->MtkLcsData;
        // get pixel id
        switch(mtk_lsc->PixId)   //0,1,2,3: B,Gb,Gr,R
        {
            case 1:
                bayer = BAYER_B;
                break;
            case 2:
                bayer = BAYER_GB;
                break;
            case 4:
                bayer = BAYER_GR;
                break;
            case 8:
                bayer = BAYER_R;
                break;
        }

        switch (m_rIspShadingLut.SensorGoldenCalTable.PixId)
        {
            case 1:
                bayer_golden = BAYER_B;
                break;
            case 2:
                bayer_golden = BAYER_GB;
                break;
            case 4:
                bayer_golden = BAYER_GR;
                break;
            case 8:
                bayer_golden = BAYER_R;
                break;
        }

        eeprom.ctl2.bits.SDBLK_XNUM     = ((mtk_lsc->CapIspReg[1] >> 28) & 0x0000000F);
        eeprom.ctl3.bits.SDBLK_YNUM     = ((mtk_lsc->CapIspReg[1] >> 12) & 0x0000000F);
        eeprom.ctl2.bits.SDBLK_WIDTH    = ((mtk_lsc->CapIspReg[1] >> 16) & 0x00000fFF);
        eeprom.ctl3.bits.SDBLK_HEIGHT   =  (mtk_lsc->CapIspReg[1]    & 0x00000fFF);
        eeprom.lblock.bits.SDBLK_lWIDTH = ((mtk_lsc->CapIspReg[3] >> 16) & 0x00000fFF);
        eeprom.lblock.bits.SDBLK_lHEIGHT=  (mtk_lsc->CapIspReg[3]        & 0x00000fFF);
        eeprom.ratio.val                = (mtk_lsc->CapIspReg[4]);

        MY_LOG("[%s:eeprom] XNUM, YNUM, WIDTH, HEIGHT, LWIDTH, LHEIGHT\n (0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x)", __FUNCTION__,
                eeprom.ctl2.bits.SDBLK_XNUM,
                eeprom.ctl3.bits.SDBLK_YNUM ,
                eeprom.ctl2.bits.SDBLK_WIDTH ,
                eeprom.ctl3.bits.SDBLK_HEIGHT,
                eeprom.lblock.bits.SDBLK_lWIDTH,
                eeprom.lblock.bits.SDBLK_lHEIGHT);

        if (eeprom.ctl2.bits.SDBLK_WIDTH == 0 || eeprom.ctl3.bits.SDBLK_HEIGHT == 0)
        {
            MY_ERR("[%s:eeprom] Calibration data is not correct!");
            m_bIsEEPROMImported = MTRUE;
            return MTRUE;
        }

        golden.ctl2.bits.SDBLK_XNUM         = ((m_rIspShadingLut.SensorGoldenCalTable.IspLSCReg[1] >> 28) & 0x0000000F);
        golden.ctl3.bits.SDBLK_YNUM         = ((m_rIspShadingLut.SensorGoldenCalTable.IspLSCReg[1] >> 12) & 0x0000000F);
        golden.ctl2.bits.SDBLK_WIDTH        = ((m_rIspShadingLut.SensorGoldenCalTable.IspLSCReg[1] >> 16) & 0x00000fFF);
        golden.ctl3.bits.SDBLK_HEIGHT       =  (m_rIspShadingLut.SensorGoldenCalTable.IspLSCReg[1]    & 0x00000fFF);
        golden.lblock.bits.SDBLK_lWIDTH     = ((m_rIspShadingLut.SensorGoldenCalTable.IspLSCReg[3] >> 16) & 0x00000fFF);
        golden.lblock.bits.SDBLK_lHEIGHT    =  (m_rIspShadingLut.SensorGoldenCalTable.IspLSCReg[3]        & 0x00000fFF);
        golden.ratio.val                    = m_rIspShadingLut.SensorGoldenCalTable.IspLSCReg[4];

        ///////////// DEBUG
        mtk_lsc->CaptureTblSize = ((MUINT32)eeprom.ctl2.bits.SDBLK_XNUM+2)*((MUINT32)eeprom.ctl3.bits.SDBLK_YNUM+2)*2;

        if (golden.ctl2.bits.SDBLK_XNUM != eeprom.ctl2.bits.SDBLK_XNUM ||
            golden.ctl3.bits.SDBLK_YNUM != eeprom.ctl3.bits.SDBLK_YNUM ||
            golden.ctl2.bits.SDBLK_WIDTH != eeprom.ctl2.bits.SDBLK_WIDTH ||
            golden.ctl3.bits.SDBLK_HEIGHT != eeprom.ctl3.bits.SDBLK_HEIGHT)
        {
            MY_ERR("[%s] Golden config is different from Unit.\n", __FUNCTION__);
        }

        if (golden.ctl2.bits.SDBLK_WIDTH == 0 || golden.ctl3.bits.SDBLK_HEIGHT == 0)
        {
            MY_ERR("[%s:golden] No golden setting, using eeprom setting instead", __FUNCTION__);

            golden.ctl2.bits.SDBLK_XNUM         = eeprom.ctl2.bits.SDBLK_XNUM     ;
            golden.ctl3.bits.SDBLK_YNUM         = eeprom.ctl3.bits.SDBLK_YNUM     ;
            golden.ctl2.bits.SDBLK_WIDTH        = eeprom.ctl2.bits.SDBLK_WIDTH    ;
            golden.ctl3.bits.SDBLK_HEIGHT       = eeprom.ctl3.bits.SDBLK_HEIGHT   ;
            golden.lblock.bits.SDBLK_lWIDTH     = eeprom.lblock.bits.SDBLK_lWIDTH ;
            golden.lblock.bits.SDBLK_lHEIGHT    = eeprom.lblock.bits.SDBLK_lHEIGHT;
            golden.ratio.val                    = eeprom.ratio.val;
            MUINT32 size = mtk_lsc->CaptureTblSize*4;//mtk_lsc->TableSize;
#if 1
            UINT32 *pGoldenTbl = (UINT32*)m_rIspShadingLut.SensorGoldenCalTable.GainTable;
            for (MUINT32 idx = 0; idx < 32; idx += 4) {
                MY_ERR("[golden] GoldenTbl 0x%08x/0x%08x/0x%08x/0x%08x",
                        *(pGoldenTbl+idx), *(pGoldenTbl+idx+1), *(pGoldenTbl+idx+2), *(pGoldenTbl+idx+3));
            }
            memcpy((void *)m_rIspShadingLut.SensorGoldenCalTable.GainTable, (void*)mtk_lsc->CapTable, size);//mtk_lsc->CaptureTblSize);
#else
            MUINT32 debug_golden[] =
            {
                    0x5a2952c8,    0x5bca589e,    0x4a5144c9,    0x4a6149be,
                    0x3eab39f5,    0x3e643e26,    0x37483466,    0x37773745,
                    0x37a03454,    0x371a377f,    0x3e553a59,    0x3d923e30,
                    0x49d54506,    0x48e34938,    0x59425343,    0x585b5836,
                    0x4f4e4986,    0x4f714e97,    0x3de039e5,    0x3d403d62,
                    0x308e2df8,    0x2fb0304f,    0x29d2287f,    0x29642998,
                    0x29c0282d,    0x28a92977,    0x30162de7,    0x2eec2fd2,
                    0x3d5d3a1a,    0x3b8f3d28,    0x4f194b3b,    0x4e3b4e6c,
                    0x47b5434b,    0x48014720,    0x36d43357,    0x362d361e,
                    0x298027c6,    0x28d92948,    0x221321b6,    0x2228220b,
                    0x219f2164,    0x2185219b,    0x28a82776,    0x27e728be,
                    0x35cc33a7,    0x35113580,    0x47774418,    0x46f14700,
                    0x483843a4,    0x48d647b1,    0x36ea33ba,    0x366b3676,
                    0x2992283d,    0x2943297f,    0x226021fe,    0x226f225a,
                    0x220021cd,    0x21bf221e,    0x28f627f2,    0x285c28ff,
                    0x35ff3423,    0x357f35eb,    0x47f94492,    0x48084802,
                    0x50394aed,    0x50f25030,    0x3f4a3b61,    0x3e8a3eda,
                    0x31402e6e,    0x30403130,    0x2a672924,    0x2a062a75,
                    0x2a1d2927,    0x29112a05,    0x30a12ed4,    0x2fb030cf,
                    0x3e4a3b65,    0x3d593e33,    0x51234ccc,    0x5085505e,
                    0x5b925568,    0x5d5b5bd5,    0x4d1947f3,    0x4c974c7a,
                    0x3feb3c77,    0x3fc73fe3,    0x38ac3635,    0x394238db,
                    0x37fc35d0,    0x38c0388e,    0x3eaa3bac,    0x3ef03ee5,
                    0x4b5446ec,    0x4b054add,    0x597e550c,    0x5988593c};
            memcpy((void *)m_rIspShadingLut.SensorGoldenCalTable.GainTable, (void*)debug_golden, sizeof(debug_golden));//mtk_lsc->CaptureTblSize);
#endif
        }

        MY_LOG("[%s:golden] XNUM, YNUM, WIDTH, HEIGHT, LWIDTH, LHEIGHT\n (0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x)", __FUNCTION__,
                golden.ctl2.bits.SDBLK_XNUM     ,
                golden.ctl3.bits.SDBLK_YNUM     ,
                golden.ctl2.bits.SDBLK_WIDTH    ,
                golden.ctl3.bits.SDBLK_HEIGHT   ,
                golden.lblock.bits.SDBLK_lWIDTH ,
                golden.lblock.bits.SDBLK_lHEIGHT);

        UINT32 u4GainTblSize = mtk_lsc->CaptureTblSize;
        UINT32* pUnitTbl = new UINT32[u4GainTblSize];
        UINT32* pGoldenTbl = new UINT32[u4GainTblSize];

        memcpy(pUnitTbl, (UINT32*)mtk_lsc->CapTable, u4GainTblSize*sizeof(UINT32));
        memcpy(pGoldenTbl, (UINT32*)m_rIspShadingLut.SensorGoldenCalTable.GainTable, u4GainTblSize*sizeof(UINT32));

        MY_LOG("[%s] Check Unit GainTbl\n", __FUNCTION__);
        for (MUINT32 idx = 0; idx < u4GainTblSize; idx += 4)
        {
            MY_LOG("0x%08x    0x%08x    0x%08x    0x%08x\n", 
                *(pUnitTbl+idx), *(pUnitTbl+idx+1), *(pUnitTbl+idx+2), *(pUnitTbl+idx+3));
        }
        UINT8* pu8Tbl = reinterpret_cast<UINT8*>(&pUnitTbl[0]);
        for (MUINT32 idx = 0; idx < u4GainTblSize*4; idx += 16)
        {
            MY_LOG("0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x,\n",
                *(pu8Tbl+idx), *(pu8Tbl+idx+1), *(pu8Tbl+idx+2), *(pu8Tbl+idx+3),
                *(pu8Tbl+idx+4), *(pu8Tbl+idx+5), *(pu8Tbl+idx+6), *(pu8Tbl+idx+7),
                *(pu8Tbl+idx+8), *(pu8Tbl+idx+9), *(pu8Tbl+idx+10), *(pu8Tbl+idx+11),
                *(pu8Tbl+idx+12), *(pu8Tbl+idx+13), *(pu8Tbl+idx+14), *(pu8Tbl+idx+15));
        }
        MY_LOG("[%s] Check Golden GainTbl\n", __FUNCTION__);
        for (MUINT32 idx = 0; idx < u4GainTblSize; idx += 4)
        {
            MY_LOG("0x%08x    0x%08x    0x%08x    0x%08x\n",
                *(pGoldenTbl+idx), *(pGoldenTbl+idx+1), *(pGoldenTbl+idx+2), *(pGoldenTbl+idx+3));
        }
        pu8Tbl = reinterpret_cast<UINT8*>(&pGoldenTbl[0]);
        for (MUINT32 idx = 0; idx < u4GainTblSize*4; idx += 16)
        {
            MY_LOG("0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x,\n",
                *(pu8Tbl+idx), *(pu8Tbl+idx+1), *(pu8Tbl+idx+2), *(pu8Tbl+idx+3),
                *(pu8Tbl+idx+4), *(pu8Tbl+idx+5), *(pu8Tbl+idx+6), *(pu8Tbl+idx+7),
                *(pu8Tbl+idx+8), *(pu8Tbl+idx+9), *(pu8Tbl+idx+10), *(pu8Tbl+idx+11),
                *(pu8Tbl+idx+12), *(pu8Tbl+idx+13), *(pu8Tbl+idx+14), *(pu8Tbl+idx+15));
        }

        align_conf.cali.img_width   = m_SensorCrop[cali_scenario].u4CropW;//m_SensorCrop[cali_scenario].u4SrcW;
        align_conf.cali.img_height  = m_SensorCrop[cali_scenario].u4CropH;//m_SensorCrop[cali_scenario].u4SrcH;
        align_conf.cali.offset_x    = 0;//m_SensorCrop[cali_scenario].u4GrabX;
        align_conf.cali.offset_y    = 0;//m_SensorCrop[cali_scenario].u4GrabY;
        align_conf.cali.crop_width  = m_SensorCrop[cali_scenario].u4CropW;
        align_conf.cali.crop_height = m_SensorCrop[cali_scenario].u4CropH;
        align_conf.cali.bayer       = bayer;
        align_conf.cali.grid_x      = (int)eeprom.ctl2.bits.SDBLK_XNUM+2;
        align_conf.cali.grid_y      = (int)eeprom.ctl3.bits.SDBLK_YNUM+2;
        align_conf.cali.lwidth      = eeprom.lblock.bits.SDBLK_lWIDTH;
        align_conf.cali.lheight     = eeprom.lblock.bits.SDBLK_lHEIGHT;
        align_conf.cali.ratio_idx   = 0;
        align_conf.cali.grgb_same   = SHADING_GRGB_SAME_NO;
        align_conf.cali.table       = (UINT32*)pUnitTbl;
        align_conf.cali.data_type   = SHADING_TYPE_GAIN;

        MY_LOG("[%s]  align_conf.cali \n"
                "img_width  = %d\n"
                "img_height = %d\n"
                "offset_x   = %d\n"
                "offset_y   = %d\n"
                "crop_width = %d\n"
                "crop_height= %d\n"
                "bayer      = %d\n"
                "grid_x     = %d\n"
                "grid_y     = %d\n"
                "lwidth     = %d\n"
                "lheight    = %d\n"
                "ratio_idx  = %d\n"
                "grgb_same  = %d\n"
                "table      = 0x%08x\n", __FUNCTION__,
                align_conf.cali.img_width   ,
                align_conf.cali.img_height  ,
                align_conf.cali.offset_x    ,
                align_conf.cali.offset_y    ,
                align_conf.cali.crop_width  ,
                align_conf.cali.crop_height ,
                align_conf.cali.bayer       ,
                align_conf.cali.grid_x      ,
                align_conf.cali.grid_y      ,
                align_conf.cali.lwidth      ,
                align_conf.cali.lheight     ,
                align_conf.cali.ratio_idx   ,
                align_conf.cali.grgb_same,
                align_conf.cali.table
        );

        align_conf.golden.img_width   = m_SensorCrop[cali_scenario].u4CropW;//m_SensorCrop[cali_scenario].u4SrcW;
        align_conf.golden.img_height  = m_SensorCrop[cali_scenario].u4CropH;//m_SensorCrop[cali_scenario].u4SrcH;
        align_conf.golden.offset_x    = 0;//m_SensorCrop[cali_scenario].u4GrabX;
        align_conf.golden.offset_y    = 0;//m_SensorCrop[cali_scenario].u4GrabY;
        align_conf.golden.crop_width  = m_SensorCrop[cali_scenario].u4CropW;
        align_conf.golden.crop_height = m_SensorCrop[cali_scenario].u4CropH;
        align_conf.golden.bayer       = bayer_golden;
        align_conf.golden.grid_x      = (int)golden.ctl2.bits.SDBLK_XNUM+2;
        align_conf.golden.grid_y      = (int)golden.ctl3.bits.SDBLK_YNUM+2;
        align_conf.golden.lwidth      = golden.lblock.bits.SDBLK_lWIDTH;
        align_conf.golden.lheight     = golden.lblock.bits.SDBLK_lHEIGHT;
        align_conf.golden.ratio_idx   = 0;
        align_conf.golden.grgb_same   = SHADING_GRGB_SAME_NO;
        align_conf.golden.table       = (UINT32*)pGoldenTbl;
        align_conf.golden.data_type   = SHADING_TYPE_GAIN;


        MY_LOG("[%s]  align_conf.golden\n"
                "img_width  = %d\n"
                "img_height = %d\n"
                "offset_x   = %d\n"
                "offset_y   = %d\n"
                "crop_width = %d\n"
                "crop_height= %d\n"
                "bayer      = %d\n"
                "grid_x     = %d\n"
                "grid_y     = %d\n"
                "lwidth     = %d\n"
                "lheight    = %d\n"
                "ratio_idx  = %d\n"
                "grgb_same  = %d\n"
                "table      = 0x%08x\n", __FUNCTION__,
                align_conf.golden.img_width   ,
                align_conf.golden.img_height  ,
                align_conf.golden.offset_x    ,
                align_conf.golden.offset_y    ,
                align_conf.golden.crop_width  ,
                align_conf.golden.crop_height ,
                align_conf.golden.bayer       ,
                align_conf.golden.grid_x      ,
                align_conf.golden.grid_y      ,
                align_conf.golden.lwidth      ,
                align_conf.golden.lheight     ,
                align_conf.golden.ratio_idx   ,
                align_conf.golden.grgb_same,
                align_conf.golden.table
        );

#if 1   // One color temperature only
        //Step (1) process LSC_SCENARIO_04 (cap tile) first
        // chose the right LSC scenario to apply
        LSC_RESULT ret = S_LSC_CONVERT_OK;
        int lsc_scenario = LSC_SCENARIO_04;
        MUINT8 ct_idx = 0;

        MY_LOG("[%s] start shading_align_golden", __FUNCTION__);
        //for (ct_idx = 0; ct_idx < SHADING_SUPPORT_CT_NUM && ret == S_LSC_CONVERT_OK; ct_idx++)
        for (ct_idx = 0; ct_idx < SHADING_SUPPORT_CT_NUM; ct_idx++)
        {
            //memcpy((void*)mtk_lsc->CapTable, (void*)tbl_buf, mtk_lsc->CaptureTblSize*4); // restore
            memcpy(pUnitTbl, (UINT32*)mtk_lsc->CapTable, u4GainTblSize*sizeof(UINT32));
            memcpy(pGoldenTbl, (UINT32*)m_rIspShadingLut.SensorGoldenCalTable.GainTable, u4GainTblSize*sizeof(UINT32));

            fillTblInfoByLscScenarionCT(align_conf.input,
                    LSC_SCENARIO_04,
                    (ELscScenario_T)lsc_scenario, ct_idx, TRANS_INPUT);
            fillTblInfoByLscScenarionCT(align_conf.output,
                    LSC_SCENARIO_04,
                    (ELscScenario_T)lsc_scenario, ct_idx, TRANS_OUTPUT);

            ret = shading_align_golden(align_conf);

            if (S_LSC_CONVERT_OK != ret)
            {
                MY_ERR("[%s] shading_align_golden error, lsc_scenario: %d, ct_idx: %d\n",
                        __FUNCTION__,
                        lsc_scenario,
                        ct_idx);
            } else {
                MY_LOG("[%s] align ct_idx %d done!", __FUNCTION__, ct_idx);
//                {
//                    UINT32 *pTbl = (MUINT32*)align_conf.output.table;
//                    for (MUINT32 idx = 0; idx < 16; idx += 4) {
//                        MY_LOG("[%s:Output tbl] Check ct%d tbl 0x%08x/0x%08x/0x%08x/0x%08x", __FUNCTION__,
//                                ct_idx,
//                                *(pTbl+idx), *(pTbl+idx+1), *(pTbl+idx+2), *(pTbl+idx+3));
//                    }
//                }
//                {
//                    char filename[128];
//                    sprintf(filename, "/sdcard/lsc1to3data/Cap1to3_ct%d.bin", ct_idx);
//                    MY_LOG("[LscMgr:%s] DBG: Output Capture Table to %s", __FUNCTION__, filename);
//                    FILE* fpdebug = fopen(filename,"wb");
//                    if ( fpdebug == NULL )
//                    {
//                        MY_ERR("Can't open :%s\n",filename);
//                    } else {
//                        fwrite(align_conf.output.table,
//                                getPerLutSize((ELscScenario_T)lsc_scenario),
//                                1,fpdebug);
//                        fclose(fpdebug);
//                    }
//                }
            }
        }

        // Step (2), transform LSC_SCENARIO_04 to other scenario
        SHADIND_TRFM_CONF trfm;
        //******************* Transform Test **********************/
        //======= working buffer allocation ===============//
        trfm.working_buff_addr  = gWorkinBuffer;
        trfm.working_buff_size  = SHADIND_FUNC_WORKING_BUFFER_SIZE;
        trfm.afn = SHADING_AFN_R0D;
        // Process shading_transform
        MY_LOG("[%s] start shading_transform", __FUNCTION__);


        for (ct_idx = 0; ct_idx < SHADING_SUPPORT_CT_NUM; ct_idx++)
        {
            UINT32 *pTbl = (MUINT32*)(stRawLscInfo[LSC_SCENARIO_04].virtAddr + (MUINT32)ct_idx * getPerLutSize(LSC_SCENARIO_04));
            UINT32 TblSize = getPerLutSize(LSC_SCENARIO_04);
            UINT8 BackupTbl[TblSize];
            // backup input table
            memcpy((void*)BackupTbl, (void*)pTbl, TblSize);

            fillTblInfoByLscScenarionCT(trfm.input, LSC_SCENARIO_04, LSC_SCENARIO_04, ct_idx, TRANS_INPUT);

            for (lsc_scenario = 0; lsc_scenario < LSC_SCENARIO_30; lsc_scenario++)
            {
                if (lsc_scenario != LSC_SCENARIO_04) {
                    MY_LOG("[%s] transform from LSC_SCENARIO_04 to %d, ct_idx %d start", __FUNCTION__,
                            lsc_scenario, ct_idx);
                    fillTblInfoByLscScenarionCT(trfm.output, LSC_SCENARIO_04,
                            (ELscScenario_T)lsc_scenario, ct_idx, TRANS_OUTPUT);

                    ret = shading_transform(trfm);
                    // restore input table
                    memcpy((void*)trfm.input.table, (void*)BackupTbl, TblSize);
                    if (ret != S_LSC_CONVERT_OK)
                    {
                        MY_ERR("[%s] shading_align_golden error, lsc_scenario: %d, ct_idx: %d",
                                __FUNCTION__,
                                lsc_scenario,
                                ct_idx);
                    } else {
                        MY_LOG("[%s] transform from LSC_SCENARIO_04 to %d, ct_idx %d done", __FUNCTION__,
                                lsc_scenario, ct_idx);
//                        UINT32 *pOutTbl = (UINT32*)trfm.output.table;
//                        UINT32 *pInTbl = (UINT32*)trfm.input.table;
//                        for (MUINT32 idx = 0; idx < 64; idx += 4) {
//                            MY_LOG("[%s] Check In Tbl 0x%08x/0x%08x/0x%08x/0x%08x", __FUNCTION__,
//                                    *(pInTbl+idx), *(pInTbl+idx+1), *(pInTbl+idx+2), *(pInTbl+idx+3));
//                        }
//                        for (MUINT32 idx = 0; idx < 64; idx += 4) {
//                            MY_LOG("[%s] Check Out Tbl 0x%08x/0x%08x/0x%08x/0x%08x", __FUNCTION__,
//                                    *(pOutTbl+idx), *(pOutTbl+idx+1), *(pOutTbl+idx+2), *(pOutTbl+idx+3));
//                        }
//                        {
//                            char filename[128];
//                            sprintf(filename, "/sdcard/lsc1to3data/Trans_lsc%dct%d.bin", lsc_scenario, ct_idx);
//                            MY_LOG("[LscMgr:%s] DBG: Output  Table to %s", __FUNCTION__, filename);
//                            FILE* fpdebug = fopen(filename,"wb");
//                            if ( fpdebug == NULL )
//                            {
//                                MY_ERR("Can't open :%s\n",filename);
//                            } else {
//                                fwrite(trfm.output.table,
//                                        getPerLutSize((ELscScenario_T)lsc_scenario),
//                                        1,fpdebug);
//                                fclose(fpdebug);
//                            }
//                        }
                    }
                }
            }
        }
        MY_LOG("[%s] shading_transform DONE!!", __FUNCTION__);
#else
        // chose the right LSC scenario to apply
        LSC_RESULT ret = S_LSC_CONVERT_OK;
        int lsc_scenario = 0;
        MUINT8 ct_idx = 0;
        for (lsc_scenario = 0; lsc_scenario < LSC_SCENARIO_NUM && ret == S_LSC_CONVERT_OK; lsc_scenario++)
        {
            for (ct_idx = 0; ct_idx < SHADING_SUPPORT_CT_NUM && ret == S_LSC_CONVERT_OK; ct_idx++)
            {
                fillTblInfoByLscScenarionCT(align_conf.input, (ELscScenario_T)lsc_scenario, ct_idx);
                memcpy(&align_conf.output, &align_conf.input, sizeof(align_conf.input));
                ret = shading_align_golden(align_conf);

                if (S_LSC_CONVERT_OK != ret)
                {
                    MY_ERR("[%s] shading_align_golden error, lsc_scenario: %d, ct_idx: %d",
                            lsc_scenario,
                            ct_idx);
                }
            }
        }
#endif
        delete [] pUnitTbl;
        delete [] pGoldenTbl;

        delete [] gWorkinBuffer;
        gWorkinBuffer = NULL;

        if (bForce1to3FromDft == 0)
        {
            // copy Unit table to NVRAM buf
            m_rIspShadingLut.SensorGoldenCalTable.IspLSCReg[1] = mtk_lsc->CapIspReg[1];
            m_rIspShadingLut.SensorGoldenCalTable.IspLSCReg[3] = mtk_lsc->CapIspReg[3];
            m_rIspShadingLut.SensorGoldenCalTable.IspLSCReg[4] = mtk_lsc->CapIspReg[4];
            memcpy((void *)m_rIspShadingLut.SensorGoldenCalTable.GainTable,
                    (void*)mtk_lsc->CapTable, u4GainTblSize*sizeof(UINT32));
            saveToNVRAM();
        }
    }
#endif

    RawLscTblFlushCurrTbl();
    m_bIsEEPROMImported = MTRUE;
    return MTRUE;
}

MBOOL 
LscMgr::
saveToNVRAM(void)
{
    MUINT32 u4SensorID;
    CAMERA_DUAL_CAMERA_SENSOR_ENUM eSensorEnum;
    MBOOL ret = MTRUE;

    MY_LOG("[%s]\n", __FUNCTION__);

    NVRAM_CAMERA_ISP_PARAM_STRUCT* pNvramIsp;
    NVRAM_CAMERA_SHADING_STRUCT* pNvramShading;

    NvramDrvBase* pNvramDrvObj = NvramDrvBase::createInstance();
    NSNvram::BufIF<NVRAM_CAMERA_ISP_PARAM_STRUCT>*const pBufIF_ISP = pNvramDrvObj->getBufIF< NVRAM_CAMERA_ISP_PARAM_STRUCT>();
    NSNvram::BufIF<NVRAM_CAMERA_SHADING_STRUCT>*const pBufIF_Shading = pNvramDrvObj->getBufIF< NVRAM_CAMERA_SHADING_STRUCT>();

    //  Sensor driver.
    SensorHal*const pSensorHal = SensorHal::createInstance();

    //  Query sensor ID & sensor enum.
    switch ( m_SensorDev )
    {
    case ESensorDev_Main:
        eSensorEnum = DUAL_CAMERA_MAIN_SENSOR;
        pSensorHal->sendCommand(SENSOR_DEV_MAIN, SENSOR_CMD_GET_SENSOR_ID, reinterpret_cast<MINT32>(&u4SensorID), 0, 0);
        break;
    case ESensorDev_Sub:
        eSensorEnum = DUAL_CAMERA_SUB_SENSOR;
        pSensorHal->sendCommand(SENSOR_DEV_SUB, SENSOR_CMD_GET_SENSOR_ID, reinterpret_cast<MINT32>(&u4SensorID), 0, 0);
        break;
    case ESensorDev_MainSecond:
        eSensorEnum = DUAL_CAMERA_MAIN_SECOND_SENSOR;
        pSensorHal->sendCommand(SENSOR_DEV_MAIN_2, SENSOR_CMD_GET_SENSOR_ID, reinterpret_cast<MINT32>(&u4SensorID), 0, 0);
        break;
    default:    //  Shouldn't happen.
        MY_ERR("Invalid sensor device: %d", (MUINT32)m_SensorDev);
        ret = MFALSE;
        break;
    }

    if (ret)
    {
        pNvramIsp = pBufIF_ISP->getRefBuf(eSensorEnum, u4SensorID);
        pNvramShading = pBufIF_Shading->getRefBuf(eSensorEnum, u4SensorID);

        MY_LOG("[%s] writing to NVRAM buf 0x%08x, 0x%08x, m_SensorDev=%d, eSensorEnum=%d, u4SensorID=%d \n",
            __FUNCTION__, pNvramIsp, pNvramShading, m_SensorDev, eSensorEnum, u4SensorID);

        // write pattern to NVRAM to indicate data exists in NVRAM
        pNvramIsp->ISPComm.CommReg[EEPROM_INFO_IN_COMM_LOAD] = CAL_DATA_LOAD;
        // write transformed table to NVRAM
        ::memcpy(&pNvramShading->Shading.PreviewFrmTable[0][0], reinterpret_cast<MVOID*>(stRawLscInfo[LSC_SCENARIO_01].virtAddr), getTotalLutSize(LSC_SCENARIO_01));
        ::memcpy(&pNvramShading->Shading.CaptureFrmTable[0][0], reinterpret_cast<MVOID*>(stRawLscInfo[LSC_SCENARIO_03].virtAddr), getTotalLutSize(LSC_SCENARIO_03));
        ::memcpy(&pNvramShading->Shading.CaptureTilTable[0][0], reinterpret_cast<MVOID*>(stRawLscInfo[LSC_SCENARIO_04].virtAddr), getTotalLutSize(LSC_SCENARIO_04));
        ::memcpy(&pNvramShading->Shading.VideoFrmTable[0][0], reinterpret_cast<MVOID*>(stRawLscInfo[LSC_SCENARIO_09_17].virtAddr), getTotalLutSize(LSC_SCENARIO_09_17));

        // flush to NVRAM            
        MINT32 i4IspRet = pBufIF_ISP->flush(eSensorEnum, u4SensorID);
        MINT32 i4TblRet = pBufIF_Shading->flush(eSensorEnum, u4SensorID);

        MY_LOG("[%s] writing to NVRAM, i4IspRet=%d, i4TblRet=%d\n",
            __FUNCTION__, i4IspRet, i4TblRet);
    }
    
    if ( pSensorHal )
        pSensorHal->destroyInstance();

    if ( pNvramDrvObj )
        pNvramDrvObj->destroyInstance();
    
    MY_LOG("[%s] Done\n", __FUNCTION__);

    return ret;
}


MBOOL
LscMgr::
ConfigUpdate() {
    MUINT8 idx = 0;
    ACDK_SCENARIO_ID_ENUM scenario;

    MY_LOG("[%s] !", __FUNCTION__);

    if (m_bIsLutLoaded == MFALSE)
    {
        MY_ERR("[%s] Lut not loaded yet!", __FUNCTION__);
        return MFALSE;
    }

#if USING_BUILTIN_LSC
    MY_LOG("[LscMgr] %s USING_BUILTIN_LSC", __FUNCTION__);


    // LSC scenario and Sensor scenario mapping
    for (idx = 0; idx < LSC_SCENARIO_NUM; idx++) {
        scenario = getSensorScenarioByLscScenario((ELscScenario_T)idx);
        getScenarioResolution(scenario);

        // debug
        m_rIspLscCfg[idx].lsci_en.bits.LSCI_EN = 1;
        m_rIspLscCfg[idx].lsc_en.bits.LSC_EN = 1;
        m_rIspLscCfg[idx].baseaddr.bits.BASE_ADDR = stRawLscInfo[idx].phyAddr;
        m_rIspLscCfg[idx].xsize.bits.XSIZE  = 0;

        m_rIspLscCfg[idx].ctl1.bits.SD_ULTRA_MODE = 1;
        m_rIspLscCfg[idx].ctl1.bits.SDBLK_XOFST = 0;
        m_rIspLscCfg[idx].ctl1.bits.SDBLK_YOFST = 0;
        m_rIspLscCfg[idx].ctl2.bits.SDBLK_XNUM = 15;
        m_rIspLscCfg[idx].ctl3.bits.SDBLK_YNUM = 15;

        m_rIspLscCfg[idx].ctl2.bits.SDBLK_WIDTH =
                m_SensorCrop[scenario].u4CropW/(2*((MUINT32)m_rIspLscCfg[idx].ctl2.bits.SDBLK_XNUM+1));
        m_rIspLscCfg[idx].ctl3.bits.SDBLK_HEIGHT =
                m_SensorCrop[scenario].u4CropH/(2*((MUINT32)m_rIspLscCfg[idx].ctl3.bits.SDBLK_YNUM+1));

        m_rIspLscCfg[idx].lblock.bits.SDBLK_lWIDTH =
                (m_SensorCrop[scenario].u4CropW/2 -
                        ((MUINT32)(m_rIspLscCfg[idx].ctl2.bits.SDBLK_XNUM)*
                                (MUINT32)m_rIspLscCfg[idx].ctl2.bits.SDBLK_WIDTH));

        m_rIspLscCfg[idx].lblock.bits.SDBLK_lHEIGHT =
                (m_SensorCrop[scenario].u4CropH/2 -
                        ((MUINT32)(m_rIspLscCfg[idx].ctl3.bits.SDBLK_YNUM)*
                                (MUINT32)m_rIspLscCfg[idx].ctl3.bits.SDBLK_HEIGHT));

        m_rIspLscCfg[idx].ratio.val = 0x20202020;
        m_rIspLscCfg[idx].gain_th.val = 0;

        MY_LOG("[LscMgr:%s] LSCScenario %d, sensorOp %d DMA/EN/W/H/XNum/YNum/OffX/OffY/LW/LH"
                "(%d, %d, %d, %d, %d, %d, %d, %d, %d, %d)", __FUNCTION__,
                idx, scenario,
                m_rIspLscCfg[idx].lsci_en.bits.LSCI_EN,
                m_rIspLscCfg[idx].lsc_en.bits.LSC_EN,
                m_rIspLscCfg[idx].ctl2.bits.SDBLK_WIDTH,
                m_rIspLscCfg[idx].ctl3.bits.SDBLK_HEIGHT,
                m_rIspLscCfg[idx].ctl2.bits.SDBLK_XNUM,
                m_rIspLscCfg[idx].ctl3.bits.SDBLK_YNUM,
                m_rIspLscCfg[idx].ctl1.bits.SDBLK_XOFST,
                m_rIspLscCfg[idx].ctl1.bits.SDBLK_YOFST,
                m_rIspLscCfg[idx].lblock.bits.SDBLK_lWIDTH,
                m_rIspLscCfg[idx].lblock.bits.SDBLK_lHEIGHT
        );
    }
#else
    for (idx = 0; idx < SHADING_SUPPORT_OP_NUM; idx++) {
        scenario = getSensorScenarioByLscScenario((ELscScenario_T)idx);
        getScenarioResolution(scenario);

        m_rIspLscCfg[idx].ctl2.bits.SDBLK_WIDTH =
                (m_SensorCrop[scenario].u4CropW)/(2*((MUINT32)m_rIspLscCfg[idx].ctl2.bits.SDBLK_XNUM+1));

        m_rIspLscCfg[idx].ctl3.bits.SDBLK_HEIGHT =
                (m_SensorCrop[scenario].u4CropH)/(2*((MUINT32)m_rIspLscCfg[idx].ctl3.bits.SDBLK_YNUM+1));

        m_rIspLscCfg[idx].lblock.bits.SDBLK_lWIDTH =
                (m_SensorCrop[scenario].u4CropW/2 -
                        ((MUINT32)(m_rIspLscCfg[idx].ctl2.bits.SDBLK_XNUM)*
                                (MUINT32)m_rIspLscCfg[idx].ctl2.bits.SDBLK_WIDTH));

        m_rIspLscCfg[idx].lblock.bits.SDBLK_lHEIGHT =
                (m_SensorCrop[scenario].u4CropH/2 -
                        ((MUINT32)(m_rIspLscCfg[idx].ctl3.bits.SDBLK_YNUM)*
                                (MUINT32)m_rIspLscCfg[idx].ctl3.bits.SDBLK_HEIGHT));
        m_rIspLscCfg[idx].baseaddr.bits.BASE_ADDR = stRawLscInfo[idx].phyAddr;
        m_rIspLscCfg[idx].ratio.val = 0x20202020;
        m_rIspLscCfg[idx].gain_th.val = 0;

        m_rIspLscCfg[idx].ctl1.bits.SD_ULTRA_MODE = 1;

        MY_LOG("[LscMgr:%s] LSCScenario %d, sensorOp %d DMA/EN/W/H/XNum/YNum/OffX/OffY/LW/LH/Addr"
                "(%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, 0x%08x)", __FUNCTION__,
                idx, scenario,
                m_rIspLscCfg[idx].lsci_en.bits.LSCI_EN,
                m_rIspLscCfg[idx].lsc_en.bits.LSC_EN,
                m_rIspLscCfg[idx].ctl2.bits.SDBLK_WIDTH,
                m_rIspLscCfg[idx].ctl3.bits.SDBLK_HEIGHT,
                m_rIspLscCfg[idx].ctl2.bits.SDBLK_XNUM,
                m_rIspLscCfg[idx].ctl3.bits.SDBLK_YNUM,
                m_rIspLscCfg[idx].ctl1.bits.SDBLK_XOFST,
                m_rIspLscCfg[idx].ctl1.bits.SDBLK_YOFST,
                m_rIspLscCfg[idx].lblock.bits.SDBLK_lWIDTH,
                m_rIspLscCfg[idx].lblock.bits.SDBLK_lHEIGHT,
                m_rIspLscCfg[idx].baseaddr.bits.BASE_ADDR
        );
    }
#endif
    return MTRUE;
}

NSIspTuning::LscMgr::LSCParameter &
LscMgr::
getLscNvram(void)
{
    //	        UINT8 u4Mode = 2;
    //	        for (int idx = 0; idx < 5; idx++) {
    //	            u4Mode = idx;
    //	        MY_LOG("[%s], mode = %d \n", __FUNCTION__, u4Mode);
    //	    MY_LOG("SHADING_EN:%d\n",           m_rIspLscCfg[u4Mode].lsc_en.bits.LSC_EN      );
    //	    MY_LOG("SHADINGBLK_XNUM:%d\n",      m_rIspLscCfg[u4Mode].ctl2.bits.SDBLK_XNUM    );
    //	    MY_LOG("SHADINGBLK_YNUM:%d\n",      m_rIspLscCfg[u4Mode].ctl3.bits.SDBLK_YNUM    );
    //	    MY_LOG("SHADINGBLK_WIDTH:%d\n",     m_rIspLscCfg[u4Mode].ctl2.bits.SDBLK_WIDTH   );
    //	    MY_LOG("SHADINGBLK_HEIGHT:%d\n",    m_rIspLscCfg[u4Mode].ctl3.bits.SDBLK_HEIGHT  );
    //	    MY_LOG("SHADINGBLK_ADDRESS(can not modify by user):0x%08x\n", m_rIspLscCfg[u4Mode].baseaddr.bits.BASE_ADDR);
    //	    MY_LOG("SD_LWIDTH:%d\n",            m_rIspLscCfg[u4Mode].lblock.bits.SDBLK_lWIDTH  );
    //	    MY_LOG("SD_LHEIGHT:%d\n",           m_rIspLscCfg[u4Mode].lblock.bits.SDBLK_lHEIGHT );
    //	    MY_LOG("SDBLK_RATIO00:%d\n",        m_rIspLscCfg[u4Mode].ratio.bits.RATIO00        );
    //	    MY_LOG("SDBLK_RATIO01:%d\n",        m_rIspLscCfg[u4Mode].ratio.bits.RATIO01        );
    //	    MY_LOG("SDBLK_RATIO10:%d\n",        m_rIspLscCfg[u4Mode].ratio.bits.RATIO10        );
    //	    MY_LOG("SDBLK_RATIO11:%d\n",        m_rIspLscCfg[u4Mode].ratio.bits.RATIO11        );
    //	        }
    return m_rIspLscCfg;
}


MUINT32*
LscMgr::
getLut(ELscScenario_T lsc_scenario) const {
    //    MY_LOG("[LscMgr] getLut m_eLscScenario %d m_u4CTIdx %d \n", m_eLscScenario, m_u4CTIdx);

#if USING_BUILTIN_LSC
    if (lsc_scenario == LSC_SCENARIO_04)
        return def_coef_cap;
    else
        return def_coef;
#else
    switch (lsc_scenario) {
        case LSC_SCENARIO_01:
        case LSC_SCENARIO_30:
            return &m_rIspShadingLut.PreviewFrmTable[0][0];
        case LSC_SCENARIO_03:
            return &m_rIspShadingLut.CaptureFrmTable[0][0];
        case LSC_SCENARIO_04:
        case LSC_SCENARIO_37:
            return &m_rIspShadingLut.CaptureTilTable[0][0];
        case LSC_SCENARIO_09_17:
            return &m_rIspShadingLut.VideoFrmTable[0][0];
#if 0
        case LSC_SCENARIO_30:
            return &m_rIspShadingLut.N3DPvwTable[0][0];
        case LSC_SCENARIO_37:
            return &m_rIspShadingLut.N3DCapTable[0][0];
#endif
        default:
            MY_ERR("[LscMgr] "
                    "Wrong m_eLscScenario %d\n", lsc_scenario);
            break;
    }
    return NULL;
#endif
}


MUINT32 u4BufSizeU8[SHADING_SUPPORT_OP_NUM] = {
        MAX_SHADING_PvwFrm_SIZE*sizeof(MUINT32)*SHADING_SUPPORT_CT_NUM,
        MAX_SHADING_CapFrm_SIZE*sizeof(MUINT32)*SHADING_SUPPORT_CT_NUM,
        MAX_SHADING_CapTil_SIZE*sizeof(MUINT32)*SHADING_SUPPORT_CT_NUM,
        MAX_SHADING_VdoFrm_SIZE*sizeof(MUINT32)*SHADING_SUPPORT_CT_NUM,
        MAX_SHADING_PvwFrm_SIZE*sizeof(MUINT32)*SHADING_SUPPORT_CT_NUM,
        MAX_SHADING_CapTil_SIZE*sizeof(MUINT32)*SHADING_SUPPORT_CT_NUM
};

#if ENABLE_TSF
MUINT32 u4TSFBufSizeU8[] =
{
        MAX_SHADING_CapTil_SIZE*sizeof(MUINT32), // TSF_BUFIDX_INPUT
        MAX_SHADING_CapTil_SIZE*sizeof(MUINT32), // TSF_BUFIDX_OUTPUT
        MAX_SHADING_CapTil_SIZE*sizeof(MUINT32), // TSF_BUFIDX_BAK
        AWB_STAT_SIZE,                           // TSF_BUFIDX_AWB
};
#endif

MUINT32
LscMgr::
getPerLutSize(ELscScenario_T eLscScenario) const {
    MUINT32 tableSize = 0;

#if USING_BUILTIN_LSC
    if (eLscScenario == LSC_SCENARIO_04) {
        MY_LOG("[LscMgr] %s USING_BUILTIN_LSC size %d", __FUNCTION__, sizeof(def_coef_cap));
        return sizeof(def_coef_cap);
    } else {
        MY_LOG("[LscMgr] %s USING_BUILTIN_LSC size %d", __FUNCTION__, sizeof(def_coef));
        return sizeof(def_coef);
    }
#else

    if (eLscScenario < SHADING_SUPPORT_OP_NUM) {
        //        MY_LOG("[LscMgr] %s, PertableSize %d", __FUNCTION__,
        //                u4BufSizeU8[eLscScenario]/SHADING_SUPPORT_CT_NUM);
        return u4BufSizeU8[eLscScenario]/SHADING_SUPPORT_CT_NUM;
    } else {
        MY_ERR("[LscMgr] Wrong eLscScenario %d\n", eLscScenario);
        return 0;
    }

#endif
}

MUINT32
LscMgr::
getTotalLutSize(ELscScenario_T eLscScenario) const {
    //    MY_LOG("[LscMgr] "
    //            "getLut m_eLscScenario %d\n", eLscScenario);
    MUINT32 tableSize = 0;

#if USING_BUILTIN_LSC
    if (eLscScenario == LSC_SCENARIO_04) {
        MY_LOG("[LscMgr] %s USING_BUILTIN_LSC size %d", __FUNCTION__, sizeof(def_coef_cap));
        return sizeof(def_coef_cap);
    } else {
        MY_LOG("[LscMgr] %s USING_BUILTIN_LSC size %d", __FUNCTION__, sizeof(def_coef));
        return sizeof(def_coef);
    }
#else

    if (eLscScenario < SHADING_SUPPORT_OP_NUM) {
        return u4BufSizeU8[eLscScenario];
    } else {
        MY_ERR("[LscMgr] Wrong eLscScenario %d\n", eLscScenario);
        return 0;
    }

#endif
}

MVOID
LscMgr::
loadLut()
{
    if (m_bIsLutLoaded == MTRUE)
    {
        MY_LOG("[LscMgr] m_bIsLutLoaded == MTRUE");
        return;
    }

    m_bIsLutLoaded = MTRUE;
    loadLutToSysram();
    MY_LOG("[LscMgr] loadLutToSysram Done!");
    RawLscTblDump("lscOrg");
    ConfigUpdate();
    MY_LOG("[LscMgr] ConfigUpdate Done!");
    importEEPromData();
    MY_LOG("[LscMgr] importEEPromData Done!");
    RawLscTblDump("lsc123");
    ConfigUpdate();
    MY_LOG("[LscMgr] ConfigUpdate Again!");
#if ENABLE_TSF
    loadTSFLut();
    MY_LOG("[LscMgr] loadTSFLut Done!");
#endif
}

MVOID
LscMgr::
loadLutToSysram()     //  VA <- LUT
{
    MUINT32 i;
    ELscScenario_T iScene;


#if 1
    for (i = 0; i < LSC_SCENARIO_NUM; i++)
    {
        iScene = static_cast<ELscScenario_T>(i);
#else   // debug
        iScene = m_eLscScenario;
#endif

        MY_LOG("[LscMgr] "
                "loadLutToSysram <<m_eLscScenario %d>>, m_pvVirTBA 0x%x \n", iScene,
                stRawLscInfo[iScene].virtAddr);
        if (stRawLscInfo[iScene].virtAddr == MNULL)
        {
            MY_ERR(
                    "[LscMgr] "
                    "Err :: load shading table to NULL address (m_pvVirTBA, 0x%x) \n",
                    stRawLscInfo[iScene].virtAddr);
            return;
        }

        MY_LOG("[LscMgr:%s] virAddr 0x%x, size %d, Lut 0x%x, LutSize %d", __FUNCTION__,
                stRawLscInfo[iScene].virtAddr,
                stRawLscInfo[iScene].size,
                getLut(iScene),
                getTotalLutSize(iScene));

        //        for (int j = 0; j < SHADING_SUPPORT_CT_NUM; j++) {
        //            for (int i = 0; i < 8; i++) {
        //                MY_LOG("[LscMgr:%s] NVRAM ct %d, idx %d, 0x%08x\n",
        //                        __FUNCTION__,
        //                        j,
        //                        i,
        //                        *(MUINT32*)(getLut(iScene)
        //                                + (stRawLscInfo[iScene].size/SHADING_SUPPORT_CT_NUM/4)*j
        //                                + i));
        //            }
        //        }

        if (stRawLscInfo[iScene].size < getTotalLutSize(iScene))
        {
            MY_ERR("[%s] stRawLscInfo[iScene].size %d, LutSize %d, Overflow!!", __FUNCTION__,
                    stRawLscInfo[iScene].size,
                    getTotalLutSize(iScene));
        }
        else
        {
            MY_LOG("[LscMgr:%s] virtAddr 0x%08x, src 0x%08x, size 0x%08x\n",
                    __FUNCTION__,
                    stRawLscInfo[iScene].virtAddr,
                    getLut(iScene),
                    getTotalLutSize(iScene));

            ::memcpy(reinterpret_cast<MVOID*>(stRawLscInfo[iScene].virtAddr), getLut(iScene), getTotalLutSize(iScene));
        }

        //	        for (int j = 0; j < SHADING_SUPPORT_CT_NUM; j++) {
        //	            for (int i = 0; i < 8; i++) {
        //	                MY_LOG("[LscMgr:%s] IMEM ct %d, idx %d, 0x%08x\n",
        //	                        __FUNCTION__,
        //	                        j,
        //	                        i,
        //	                        *(MUINT32*)((MUINT32*)stRawLscInfo[iScene].virtAddr
        //	                                + (stRawLscInfo[iScene].size/SHADING_SUPPORT_CT_NUM/4)*j
        //	                                + i));
        //	            }
        //	        }
        MY_LOG("[LscMgr:%s] Copy table done (start, end) = (0x%08x, 0x%08x)\n", __FUNCTION__,
                stRawLscInfo[iScene].virtAddr,
                stRawLscInfo[iScene].virtAddr + getTotalLutSize(iScene));
    }
    if (*((MUINT32*)stRawLscInfo[iScene].virtAddr+0) == 0 &&
            *((MUINT32*)stRawLscInfo[iScene].virtAddr+1) == 0 &&
            *((MUINT32*)stRawLscInfo[iScene].virtAddr+2) == 0 &&
            *((MUINT32*)stRawLscInfo[iScene].virtAddr+3) == 0) {
        MY_ERR("[LscMgr:%s] Default table is ZERO!!", __FUNCTION__);
    }

    RawLscTblFlushCurrTbl();
    return;
}


MUINT32
LscMgr::
getCTIdx() {
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    MINT32 dbg_ct = 0;
    property_get("debug.lsc_mgr.ct", value, "-1");
    dbg_ct = atoi(value);

    if (dbg_ct != -1) 
    {
        MY_LOG("[%s] DEBUG set m_u4CTIdx(%d)", __FUNCTION__, dbg_ct);
        m_u4CTIdx = dbg_ct;
    }

    MY_LOG_IF(_bEnableMyLog, "[%s] m_u4CTIdx(%d)\n", __FUNCTION__, m_u4CTIdx);
    return m_u4CTIdx;
}

MBOOL
LscMgr::
setCTIdx(MUINT32 const u4CTIdx) {
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    MINT32 dbg_ct = 0;
    property_get("debug.lsc_mgr.ct", value, "-1");
    dbg_ct = atoi(value);

    if (dbg_ct != -1) 
    {
        MY_LOG("[%s] DEBUG set m_u4CTIdx(%d)", __FUNCTION__, dbg_ct);
        m_u4CTIdx = dbg_ct;
    } 
    else 
    {
        if (SHADING_SUPPORT_CT_NUM <= u4CTIdx) 
        {
            m_fgCtIdxExcd = MTRUE;
            MY_LOG("!!! WRONG Shading idx= %d\n", u4CTIdx);
            return MFALSE;
        }

        m_fgCtIdxExcd = MFALSE;        
        setIfChange(m_u4CTIdx, u4CTIdx);
        MY_LOG_IF(_bEnableMyLog, "[%s] m_u4CTIdx(%d), m_fgTsfSetTbl(%d)\n", __FUNCTION__, u4CTIdx, m_fgTsfSetTbl);
    }
    return MTRUE;
}


MINT32
LscMgr::
setGainTable(MUINT32 u4GridNumX, MUINT32 u4GridNumY, MUINT32 u4Width, MUINT32 u4Height, float* pGainTbl)
{
    MUINT32 u4Addr;
    MUINT32 u4Idx;
    float* afWorkingBuf = new float[BUFFERSIZE];
    if (!afWorkingBuf)
    {
        MY_LOG("[%s] Allocate afWorkingBuf Fail\n", __FUNCTION__);
        return -1;
    }

    m_fgUserSetTbl = 1;

    u4Addr = IspDebug::getInstance().readLsciAddr();
    u4Idx = (m_u4DoubleBufIdx == 0 ? 1 : 0);
    
    if (u4Addr != m_rBufInfo[m_u4DoubleBufIdx].phyAddr)
    {
        u4Idx = m_u4DoubleBufIdx;
        MY_LOG_IF(_bEnableMyLog, "[%s +] Error u4Idx(%d) Addr(0x%08x)\n", __FUNCTION__, u4Idx, u4Addr);
    }
    else
    {
        MY_LOG_IF(_bEnableMyLog, "[%s +] OK u4Idx(%d) Addr(0x%08x)\n", __FUNCTION__, u4Idx, u4Addr);
    }
    
    MUINT32 u4RetLSCHwTbl = 
        LscGaintoHWTbl(pGainTbl,
                       (MUINT32*)m_rBufInfo[u4Idx].virtAddr,
                       u4GridNumX,
                       u4GridNumY,
                       u4Width,
                       u4Height,
                       (void*)afWorkingBuf,
                       BUFFERSIZE);                        

    delete [] afWorkingBuf;

    RawLscTblFlushCurrTbl();  

    m_u4DoubleBufIdx = u4Idx;
    
    MY_LOG_IF(_bEnableMyLog, "[%s -]\n", __FUNCTION__);

    return u4RetLSCHwTbl;
}


MBOOL
LscMgr::
SetTBAToISP()     //  ISP <- PA
{
    MUINT32 virAddr = 0;
    MUINT32 phyAddr = 0;

    if (m_eLscScenario >= LSC_SCENARIO_NUM)
    {
        MY_ERR("[LscMgr] %s m_eLscScenario not initialized", __FUNCTION__);
        return MFALSE;
    }

#if ENABLE_TSF
    if (m_fgCtIdxExcd)
    {
        m_fgTsfSetTbl = (isTSFEnable() && mTSFState != LSCMGR_TSF_STATE_IDLE);
    }
    else
    {
        switch (isEnableTSF(m_SensorDev))
        {
        default:
        case 0:
        case 1:
            m_fgTsfSetTbl = (isTSFEnable() && mTSFState != LSCMGR_TSF_STATE_IDLE);
            break;
        case 2:
            m_fgTsfSetTbl = 
                (isTSFEnable() && mTSFState != LSCMGR_TSF_STATE_IDLE && !m_fgPreflash);
            break;
        }
    }

    MY_LOG_IF(_bEnableMyLog, "[%s] isTSFEnable(%d), mTSFState(%d)", __FUNCTION__, isTSFEnable(), mTSFState);

    if (m_fgUserSetTbl || m_fgTsfSetTbl)
#else
    if (m_fgUserSetTbl)
#endif
    {
        MUINT32 u4Idx = m_u4DoubleBufIdx;
        MUINT32 u4Size = getPerLutSize(m_eLscScenario) - 1;
        virAddr = m_rBufInfo[u4Idx].virtAddr;
        phyAddr = m_rBufInfo[u4Idx].phyAddr;
        m_rIspLscCfg[m_eLscScenario].baseaddr.bits.BASE_ADDR = phyAddr;
        m_rIspLscCfg[m_eLscScenario].xsize.bits.XSIZE = u4Size;

        MY_LOG_IF(_bEnableMyLog, "[%s] User: u4Idx(%d) phyAddr(0x%08x), virAddr(0x%08x), size(%d)", __FUNCTION__,
                u4Idx, phyAddr, virAddr, u4Size);
    }
    else
    {
        RawLscTblFlushCurrTbl();
#if USING_BUILTIN_LSC
        MY_LOG("[LscMgr] %s USING_BUILTIN_LSC", __FUNCTION__);
        virAddr = stRawLscInfo[m_eLscScenario].virtAddr;
        m_rIspLscCfg[m_eLscScenario].baseaddr.bits.BASE_ADDR = stRawLscInfo[m_eLscScenario].phyAddr;
        m_rIspLscCfg[m_eLscScenario].xsize.bits.XSIZE =
                getTotalLutSize(static_cast<NSIspTuning::LscMgr::ELscScenario_T>(0)) - 1;

#else
        virAddr = stRawLscInfo[m_eLscScenario].virtAddr +
                getPerLutSize(m_eLscScenario) * getCTIdx();
        MUINT32 Addr =
                stRawLscInfo[m_eLscScenario].phyAddr
                + getPerLutSize(m_eLscScenario) * getCTIdx();
        m_rIspLscCfg[m_eLscScenario].baseaddr.bits.BASE_ADDR = Addr;
        m_rIspLscCfg[m_eLscScenario].xsize.bits.XSIZE = getPerLutSize(m_eLscScenario) - 1;

        MY_LOG_IF(_bEnableMyLog, "[%s] LSC: m_eLscScenario(%d), TableSize(%d), CT(%d), phyAddr(0x%08x), XSize(%d), m_eSensorOp(%d)\n",
                __FUNCTION__,
                m_eLscScenario,
                getPerLutSize(m_eLscScenario),
                m_u4CTIdx,
                m_rIspLscCfg[m_eLscScenario].baseaddr.bits.BASE_ADDR,
                m_rIspLscCfg[m_eLscScenario].xsize.bits.XSIZE,
                m_eSensorOp);
#endif  // USING_BUILTIN_LSC
    }


    {
        char value[PROPERTY_VALUE_MAX] = {'\0'};
        MINT32 dbg_tbl = 0;
        property_get("debug.lsc_mgr.dumptbl", value, "-1");
        dbg_tbl = atoi(value);

        if (virAddr != 0 && dbg_tbl != -1) {
            for (int i = 0; i < dbg_tbl; i+=4) {
                MY_LOG("[LscMgr] idx %d, 0x%08x 0x%08x 0x%08x 0x%08x\n",
                        i,
                        *(MUINT32*)((MUINT32*)virAddr + i + 0),
                        *(MUINT32*)((MUINT32*)virAddr + i + 1),
                        *(MUINT32*)((MUINT32*)virAddr + i + 2),
                        *(MUINT32*)((MUINT32*)virAddr + i + 3)
                );
            }
        }
    }

    MY_LOG_IF(_bEnableMyLog, "[%s] HW addr(0x%08x)", __FUNCTION__, IspDebug::getInstance().readLsciAddr());

    m_pu4CurAddr = (MUINT32*)virAddr;
    
    return MTRUE;
}

MVOID
LscMgr::UpdateSL2Param(void)
{
    MY_LOG_IF(_bEnableMyLog, "[%s] +\n", __FUNCTION__);

    MINT32 i4SL2En = 0;    
    GET_PROP("debug.lsc_mgr.sl2", "-1", i4SL2En);

    if (i4SL2En != -1)
    {
        MY_LOG("[%s] Trigger to set SL2 %s\n", __FUNCTION__, (i4SL2En ? "ON" : "OFF"));
        ISP_MGR_SL2_T::getInstance(m_eActive).setEnable(i4SL2En ? MTRUE : MFALSE);
    }

#if ENABLE_TSF
    if (isTSFEnable() == MTRUE && mTSFState != LSCMGR_TSF_STATE_IDLE)
    {   
        ISP_NVRAM_SL2_T rSL2Cfg;
        if (m_pTsfResultInfo)
        {
            rSL2Cfg.cen.bits.SL2_CENTER_X = m_pTsfResultInfo->SL2Para.SL2_CENTR_X;
            rSL2Cfg.cen.bits.SL2_CENTER_Y = m_pTsfResultInfo->SL2Para.SL2_CENTR_Y;
            rSL2Cfg.max0_rr.val = m_pTsfResultInfo->SL2Para.SL2_RR_0;
            rSL2Cfg.max1_rr.val = m_pTsfResultInfo->SL2Para.SL2_RR_1;
            rSL2Cfg.max2_rr.val = m_pTsfResultInfo->SL2Para.SL2_RR_2;

            ISP_MGR_SL2_T::getInstance(m_eActive).put(rSL2Cfg);
            ISP_MGR_SL2_T::getInstance(m_eActive).apply(getIspProfile()); //Apply to ISP.
            
            MY_LOG_IF(_bEnableMyLog, "[%s] TSF SL2 = %d, cen=0x%08x, max0_rr=0x%08x, max1_rr=0x%08x, max2_rr=0x%08x\n",
                __FUNCTION__, ISP_MGR_SL2_T::getInstance(m_eActive).isEnable(),
                rSL2Cfg.cen.val, rSL2Cfg.max0_rr.val, rSL2Cfg.max1_rr.val, rSL2Cfg.max2_rr.val);
        }
        else
        {
            MY_LOG_IF(_bEnableMyLog, "[%s] m_pTsfResultInfo is NULL!\n", __FUNCTION__);
        }
    }
    else
#endif
    {
        MINT32 i4Idx = (MINT32) m_eLscScenario*SHADING_SUPPORT_CT_NUM + getCTIdx();
        if (i4Idx >= SHADING_SUPPORT_OP_NUM*SHADING_SUPPORT_CT_NUM)
            MY_ERR("[%s] ERROR: i4Idx = %d\n", i4Idx);
        
        const ISP_NVRAM_SL2_T& rSL2Cfg = m_rIspSl2Cfg[i4Idx];
        ISP_MGR_SL2_T::getInstance(m_eActive).put(rSL2Cfg);
        ISP_MGR_SL2_T::getInstance(m_eActive).apply(getIspProfile()); //Apply to ISP.
        
        MY_LOG_IF(_bEnableMyLog, "[%s] SL2 = %d, [%d] cen=0x%08x, max0_rr=0x%08x, max1_rr=0x%08x, max2_rr=0x%08x\n",
            __FUNCTION__, ISP_MGR_SL2_T::getInstance(m_eActive).isEnable(),
            i4Idx, rSL2Cfg.cen.val, rSL2Cfg.max0_rr.val, rSL2Cfg.max1_rr.val, rSL2Cfg.max2_rr.val);
    }
    
    MY_LOG_IF(_bEnableMyLog, "[%s] -\n", __FUNCTION__);
}


MBOOL
LscMgr::
getScenarioResolution(ACDK_SCENARIO_ID_ENUM scenario)
{
    MUINT32 cmd;
    SENSOR_GRAB_INFO_STRUCT rGrapInfo;

    if (!m_pSensorHal)
    {
        MY_ERR("[LscMgr] %s, m_pSensorHal is NULL", __FUNCTION__);
        m_pSensorHal = SensorHal::createInstance();
        if (m_pSensorHal->init())
        {
            MY_ERR("m_pSensorHal re-instanate fail!!");
            m_pSensorHal->destroyInstance();
            m_pSensorHal = NULL;
            return MFALSE;
        }
    }

//    m_pSensorHal->sendCommand(SENSOR_DEV_NONE, SENSOR_CMD_GET_SENSOR_DEV, (int)&m_SensorDev, 0, 0);
//    m_SensorDev = (halSensorDev_s)m_eActive;
#if 1
    m_pSensorHal->sendCommand(m_SensorDev, SENSOR_CMD_GET_SENSOR_GRAB_INFO, (int)&rGrapInfo, scenario);
    m_SensorCrop[scenario].u4GrabX  = rGrapInfo.u4SensorGrabStartX;
    m_SensorCrop[scenario].u4GrabY  = rGrapInfo.u4SensorGrabStartY;
    m_SensorCrop[scenario].u4SubSpW = rGrapInfo.u2SensorSubSpW;
    m_SensorCrop[scenario].u4SubSpH = rGrapInfo.u2SensorSubSpH;
#else
    m_pSensorHal->sendCommand(m_SensorDev, SENSOR_CMD_GET_SENSOR_GRAB_INFO,
            (int)&m_SensorCrop[scenario].u4GrabX,
            (int)&m_SensorCrop[scenario].u4GrabY,
            scenario);
    //m_pSensorHal->sendCommand(m_SensorDev, SENSOR_CMD_GET_SENSOR_SUBSAMPLING_INFO,
    //        (int)&m_SensorCrop[scenario].u4SubSpW,
    //        (int)&m_SensorCrop[scenario].u4SubSpH,
    //        scenario);
    m_SensorCrop[scenario].u4SubSpW = m_SensorCrop[scenario].u4SubSpH = 0;
#endif
    switch(scenario)
    {
        case ACDK_SCENARIO_ID_CAMERA_PREVIEW:
            cmd = SENSOR_CMD_GET_SENSOR_PRV_RANGE;
            MY_LOG("acdk ACDK_SCENARIO_ID_CAMERA_PREVIEW, sensor SENSOR_CMD_GET_SENSOR_PRV_RANGE");
            break;
        case ACDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
            cmd = SENSOR_CMD_GET_SENSOR_FULL_RANGE;
            MY_LOG("acdk ACDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG, sensor SENSOR_CMD_GET_SENSOR_FULL_RANGE");
            break;
        case ACDK_SCENARIO_ID_VIDEO_PREVIEW:
            cmd = SENSOR_CMD_GET_SENSOR_VIDEO_RANGE;
            MY_LOG("acdk ACDK_SCENARIO_ID_VIDEO_PREVIEW, sensor SENSOR_CMD_GET_SENSOR_VIDEO_RANGE");
            break;
        case ACDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
            cmd = SENSOR_CMD_GET_SENSOR_HIGH_SPEED_VIDEO_RANGE;
            MY_LOG("acdk ACDK_SCENARIO_ID_HIGH_SPEED_VIDEO, sensor SENSOR_CMD_GET_SENSOR_HIGH_SPEED_VIDEO_RANGE");
            break;
        case ACDK_SCENARIO_ID_CAMERA_ZSD:
            cmd = SENSOR_CMD_GET_SENSOR_FULL_RANGE;
            MY_LOG("acdk ACDK_SCENARIO_ID_CAMERA_ZSD, sensor SENSOR_CMD_GET_SENSOR_FULL_RANGE");
            break;
        case ACDK_SCENARIO_ID_CAMERA_3D_PREVIEW:
            cmd = SENSOR_CMD_GET_SENSOR_3D_PRV_RANGE;
            MY_LOG("acdk ACDK_SCENARIO_ID_CAMERA_3D_PREVIEW, sensor SENSOR_CMD_GET_SENSOR_3D_PRV_RANGE");
            break;
        case ACDK_SCENARIO_ID_CAMERA_3D_CAPTURE:
            cmd = SENSOR_CMD_GET_SENSOR_3D_FULL_RANGE;
            MY_LOG("acdk ACDK_SCENARIO_ID_CAMERA_3D_CAPTURE, sensor SENSOR_CMD_GET_SENSOR_3D_FULL_RANGE");
            break;
        case ACDK_SCENARIO_ID_CAMERA_3D_VIDEO:
            cmd = SENSOR_CMD_GET_SENSOR_3D_VIDEO_RANGE;
            MY_LOG("acdk ACDK_SCENARIO_ID_CAMERA_3D_VIDEO, sensor SENSOR_CMD_GET_SENSOR_3D_VIDEO_RANGE");
            break;
        default:
            cmd = SENSOR_CMD_GET_SENSOR_PRV_RANGE;
            break;
    }

    m_pSensorHal->sendCommand(m_SensorDev, cmd,
            (int)&m_SensorCrop[scenario].u4CropW,
            (int)&m_SensorCrop[scenario].u4CropH,
            0);
    m_SensorCrop[scenario].u4SrcW =
            m_SensorCrop[scenario].u4CropW+m_SensorCrop[scenario].u4GrabX;
    m_SensorCrop[scenario].u4SrcH =
            m_SensorCrop[scenario].u4CropH+m_SensorCrop[scenario].u4GrabY;

    MY_LOG("[%s] SensorOP %d GrabX GrabY SrcW SrcH CropW CropH SubW SubH DataFmt (%d, %d, %d, %d, %d, %d, %d, %d, %d)", __FUNCTION__,
            scenario,
            m_SensorCrop[scenario].u4GrabX,          // For input sensor width
            m_SensorCrop[scenario].u4GrabY,          // For input sensor height
            m_SensorCrop[scenario].u4SrcW,          // For input sensor width
            m_SensorCrop[scenario].u4SrcH,          // For input sensor height
            m_SensorCrop[scenario].u4CropW,        //TG crop width
            m_SensorCrop[scenario].u4CropH,        //TG crop height
            m_SensorCrop[scenario].u4SubSpW,
            m_SensorCrop[scenario].u4SubSpH,
            m_SensorCrop[scenario].DataFmt);

    return MTRUE;
}

MBOOL
LscMgr::
updateLscScenarioBySensorMode()
{
#if 0   // get sensor Op from sensor hal

    if (!m_pSensorHal)
    {
        MY_LOG("[%s] NULL m_pSensorHal!\n", __FUNCTION__);
        return MFALSE;
    }

    m_pSensorHal->sendCommand(SENSOR_DEV_NONE, SENSOR_CMD_GET_SENSOR_DEV, (int)&m_SensorDev, 0, 0);

    if (m_SensorDev == SENSOR_DEV_NONE)
    {
        MY_LOG("[%s] m_SensorDev is incorrect %d\n", __FUNCTION__,
                SENSOR_DEV_NONE);
        return MFALSE;
    }
    m_pSensorHal->sendCommand(m_SensorDev, SENSOR_CMD_GET_SENSOR_SCENARIO, (int)&m_eSensorOp, 0, 0);

    if (m_eSensorOp >= ACDK_SCENARIO_ID_MAX || m_SensorDev == SENSOR_DEV_NONE)
    {
        MY_LOG("[%s] m_eSensorOp output range %d, max %d\n", __FUNCTION__,
                m_eSensorOp,
                ACDK_SCENARIO_ID_MAX);
        return MFALSE;
    }
#else
    m_eSensorOp = getSensorScenarioByIspProfile(m_eIspProfile);
#endif
    m_eLscScenario = (ELscScenario_T)getLscScenarioBySensorScenario(m_eSensorOp);

    MY_LOG("[LscMgr] %s, Dev %d, SensorOp %d Lsc Scenario %d", __FUNCTION__,
            m_SensorDev,
            m_eSensorOp,
            m_eLscScenario);

    m_ePrevSensorOp = m_eSensorOp;
    return MTRUE;
}


MVOID
LscMgr::
updateLscScenarioByIspProfile(EIspProfile_T profile)
{

    switch(profile)
    {
        case EIspProfile_NormalPreview:
            m_eLscScenario = LSC_SCENARIO_01;
            m_eSensorOp = ACDK_SCENARIO_ID_CAMERA_PREVIEW;
            break;
        case EIspProfile_ZsdPreview_CC:
        case EIspProfile_ZsdPreview_NCC:
        case EIspProfile_NormalCapture:
            m_eLscScenario = LSC_SCENARIO_04;
            m_eSensorOp = ACDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG;
            break;
        case EIspProfile_VideoPreview:
            m_eLscScenario = LSC_SCENARIO_09_17;
            m_eSensorOp = ACDK_SCENARIO_ID_VIDEO_PREVIEW;
            break;
        case EIspProfile_VideoCapture:
            m_eLscScenario = LSC_SCENARIO_09_17;
            m_eSensorOp = ACDK_SCENARIO_ID_VIDEO_PREVIEW;
            break;
        default:
            m_eLscScenario = LSC_SCENARIO_01;
            m_eSensorOp = ACDK_SCENARIO_ID_CAMERA_PREVIEW;
            break;
    }
    MY_LOG("[%s]  IspProfile %d, LscScenario %d", __FUNCTION__,
            m_eIspProfile,
            m_eLscScenario);
    return;
}


static ACDK_SCENARIO_ID_ENUM SensorScenarioIspProfileMapping[] =
{
        ACDK_SCENARIO_ID_CAMERA_PREVIEW,        //EIspProfile_NormalPreview
        ACDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG,   //EIspProfile_ZsdPreview_CC
        ACDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG,   //EIspProfile_ZsdPreview_NCC
        ACDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG,   //EIspProfile_NormalCapture
        ACDK_SCENARIO_ID_VIDEO_PREVIEW,         //EIspProfile_VideoPreview
        ACDK_SCENARIO_ID_VIDEO_PREVIEW,         //EIspProfile_VideoCapture
        ACDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG,   // EIspProfile_MFCapPass1
        ACDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG,   // EIspProfile_MFCapPass2
        ACDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG,   // EIspProfile_ProcessedRAW
};

ACDK_SCENARIO_ID_ENUM
LscMgr::
getSensorScenarioByIspProfile(EIspProfile_T const eIspProfile)
{
    //MY_LOG("[LscMgr:%s], EIspProfile_T %d", __FUNCTION__, eIspProfile);

    if (eIspProfile >= EIspProfile_NUM)
    {
        MY_ERR("[LscMgr] %s, EIspProfile_T %d out of range!!", __FUNCTION__, eIspProfile);
        return ACDK_SCENARIO_ID_CAMERA_PREVIEW;
    }
    return SensorScenarioIspProfileMapping[eIspProfile];
}

MBOOL
LscMgr::
setIspProfile(EIspProfile_T const eIspProfile)
{
    EIspProfile_T profile_bak;
    MBOOL bDirty = MFALSE, bSceneChange = MFALSE;
    MY_LOG("[LscMgr] %s, EIspProfile_T %d", __FUNCTION__, eIspProfile);

    if (eIspProfile >= EIspProfile_NUM)
        return MFALSE;
    
    m_bMetaMode = MFALSE;
    profile_bak = m_eIspProfile;
    bSceneChange = setIfChange(m_eIspProfile, eIspProfile);
    m_ePrevLscScenario = m_eLscScenario;
    bDirty = updateLscScenarioBySensorMode();

    if (m_eLscScenario >= LSC_SCENARIO_NUM)
    {
        MY_ERR("[%s]  m_eLscScenario out of range %d, max %d",
                m_eLscScenario,
                LSC_SCENARIO_NUM);
        return MFALSE;
    }

    if (bSceneChange) {
        m_ePrevIspProfile = profile_bak;
    }

    if (m_pIMemDrv)
    {
        CPTLog(Event_Pipe_3A_ISP, CPTFlagStart);
        loadLut();
        CPTLog(Event_Pipe_3A_ISP, CPTFlagEnd);

#if ENABLE_TSF
        if (bSceneChange == MTRUE)   // changed
        {
            if (mTSFState == LSCMGR_TSF_STATE_IDLE) {
                MY_LOG("[LscMgr] Init TSF table!");
                changeTSFState(LSCMGR_TSF_STATE_INIT);
            } else
                if (!(EIspProfile_VideoCapture == m_eIspProfile ||
                        EIspProfile_VideoCapture == m_ePrevIspProfile)) // to avoid table change
                    changeTSFState(LSCMGR_TSF_STATE_SCENECHANGE);

            if (m_bMetaMode == MTRUE) {
                MY_LOG("[LscMgr] m_bMetaMode!");
                //m_bTSF = isTSFEnable();
            } else {
                MY_LOG("[LscMgr] Normal mode!");
                //m_bTSF = isEnableTSF(m_eActive) ? MTRUE : MFALSE;
            }
        }
#endif
    }
    else
    {
        MY_LOG("[LscMgr] m_pIMemDrv 0x%x, bDirty %d", m_pIMemDrv, bDirty);
    }
    MY_LOG("[%s] m_bMetaMode(%d), PrevLscScenario(%d), LscScenario(%d)", __FUNCTION__,
            m_bMetaMode,
            m_ePrevLscScenario,
            m_eLscScenario);

    return MTRUE;
}

EIspProfile_T
LscMgr::
getIspProfile(void)
{
    return m_eIspProfile;
}

EIspProfile_T
LscMgr::
getPrevIspProfile(void)
{
    return m_ePrevIspProfile;
}

MBOOL
LscMgr::
setMetaIspProfile(EIspProfile_T const eIspProfile, MUINT32 sensor_mode)
{
    EIspProfile_T profile_bak;
    MBOOL bDirty = MFALSE, bSceneChange = MFALSE;
    MY_LOG("[LscMgr:%s], eIspProfile %d, sensor_mode %d", __FUNCTION__,
            eIspProfile,
            sensor_mode);

    m_bMetaMode = MTRUE;
    m_SensorMode = sensor_mode;

    profile_bak = m_eIspProfile;
    bSceneChange = setIfChange(m_eIspProfile, eIspProfile);
    m_ePrevLscScenario = m_eLscScenario;
    bDirty = updateLscScenarioBySensorMode();

    if (m_eLscScenario >= LSC_SCENARIO_NUM)
    {
        MY_ERR("[%s]  m_eLscScenario out of range %d, max %d",
                m_eLscScenario,
                LSC_SCENARIO_NUM);
        return MFALSE;
    }


    if (eIspProfile == EIspProfile_NormalCapture)
    {
        switch (m_SensorMode) {
            case ESensorMode_Preview:
                m_eSensorOp = ACDK_SCENARIO_ID_CAMERA_PREVIEW;
                m_eLscScenario = LSC_SCENARIO_01;
                break;
            case ESensorMode_Video:
                m_eSensorOp = ACDK_SCENARIO_ID_VIDEO_PREVIEW;
                m_eLscScenario = LSC_SCENARIO_09_17;
                break;
            case ESensorMode_Capture:
                m_eSensorOp = ACDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG;
                m_eLscScenario = LSC_SCENARIO_04;
                break;
        }
        MY_LOG("[%s] Eng Capture: m_eLscScenario(%d), m_eSensorOp(%d)", __FUNCTION__,
                m_eLscScenario,
                m_eSensorOp);
    }
    else if (eIspProfile == EIspProfile_NormalPreview && m_SensorMode == ESensorMode_Video)
    {
        m_eSensorOp = ACDK_SCENARIO_ID_VIDEO_PREVIEW;
        m_eLscScenario = LSC_SCENARIO_09_17;
        MY_LOG("[%s] Eng Preivew in Video: m_eLscScenario(%d), m_eSensorOp(%d)", __FUNCTION__,
                m_eLscScenario,
                m_eSensorOp);
    }

    if (bSceneChange) {
        m_ePrevIspProfile = profile_bak;
    }

    if (m_pIMemDrv)
    {
        CPTLog(Event_Pipe_3A_ISP, CPTFlagStart);
        loadLut();
        CPTLog(Event_Pipe_3A_ISP, CPTFlagEnd);

#if ENABLE_TSF
        if (bSceneChange == MTRUE)   // changed
        {
            if (mTSFState == LSCMGR_TSF_STATE_IDLE) {
                MY_LOG("[LscMgr] Init TSF table!");
                changeTSFState(LSCMGR_TSF_STATE_INIT);
            } else
                if (!(EIspProfile_VideoCapture == m_eIspProfile ||
                        EIspProfile_VideoCapture == m_ePrevIspProfile)) // to avoid table change
                    changeTSFState(LSCMGR_TSF_STATE_SCENECHANGE);

            if (m_bMetaMode == MTRUE) {
                //m_bTSF = MFALSE;
                MY_LOG("[LscMgr] m_bMetaMode!");
                //m_bTSF = isTSFEnable();
            } else {
                MY_LOG("[LscMgr] Normal mode!");
                //m_bTSF = isEnableTSF(m_eActive) ? MTRUE : MFALSE;
            }
        }
#endif
    }
    else
    {
        MY_LOG("[LscMgr] m_pIMemDrv 0x%x, bDirty %d", m_pIMemDrv, bDirty);
    }
    MY_LOG("[%s]  PrevLscScenario %d, LscScenario %d", __FUNCTION__,
            m_ePrevLscScenario,
            m_eLscScenario);
    return MTRUE;
}

MVOID
LscMgr::
setMetaLscScenario(ELscScenario_T lsc_scenario)
{
    m_eMetaLscScenario = lsc_scenario;
    m_bMetaMode = MTRUE;
}

MBOOL
LscMgr::
isEnable()
{
    return m_fgOnOff;
}

MBOOL
LscMgr::
enableLscWoVariable(MBOOL const fgEnable)
{
    ISP_NVRAM_LSC_T tmp;
    MBOOL fgRet = MFALSE;
    MBOOL OrgShadingEn = MFALSE;
    if (m_eLscScenario >= LSC_SCENARIO_NUM)
    {
        MY_ERR("[LscMgr] %s m_eLscScenario not initialized");
        return MFALSE;
    }

    MY_LOG("[LscMgr] -enableLsc(enableLscWoVariable)"
            "  --> %d\n", fgEnable);
    // (1) get hw setting
    ISP_MGR_LSC_T::getInstance(m_eActive).get(tmp);

    //  (2) Change the state of hw data
    tmp.lsc_en.bits.LSC_EN = tmp.lsci_en.bits.LSCI_EN = (MUINT32)fgEnable;

    //  (3) Apply to ISP.
    ISP_MGR_LSC_T::getInstance(m_eActive).put(tmp); //put to ispmgr
    fgRet = ISP_MGR_LSC_T::getInstance(m_eActive).apply(getIspProfile()); //Apply to ISP.

    MY_LOG("[LscMgr] enable %d", ISP_MGR_LSC_T::getInstance(m_eActive).isEnable());

    return fgRet;
}

MBOOL
LscMgr::
isBypass() {
    return m_bBypass;
}

MVOID
LscMgr::
enBypass(MBOOL enable) {
    m_bBypass = enable;
    enableLscWoVariable(enable);
    if (m_bBypass == MFALSE)
        enableLsc(isEnable());
}

MBOOL
LscMgr::
enableLsc(MBOOL const fgEnable)
{
    MBOOL fgRet = MFALSE;
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    MINT32 dbg_enable = 0;
    property_get("debug.lsc_mgr.enable", value, "-1");
    dbg_enable = atoi(value);

    GET_PROP("debug.lsc_mgr.log", "0", _bEnableMyLog);

    if (m_eLscScenario >= LSC_SCENARIO_NUM)
    {
        MY_ERR("[LscMgr] %s m_eLscScenario not initialized");
        return MFALSE;
    }

    //  (2) Change the state of NVRAM data
    if (dbg_enable != -1) 
    {
        MY_LOG("[LscMgr:%s] DEBUG set enable to %d", __FUNCTION__,
                dbg_enable);
        m_fgOnOff = dbg_enable;
    }
    else 
    {
        m_fgOnOff = fgEnable;
    }
    //  (3) Apply to ISP.
    fgRet = 1; //ISP_MGR_LSC_T::getInstance(m_eActive).reset();
    m_rIspLscCfg[m_eLscScenario].lsc_en.bits.LSC_EN =
    m_rIspLscCfg[m_eLscScenario].lsci_en.bits.LSCI_EN = m_fgOnOff;
    
    if (fgRet) {
        MY_LOG_IF(_bEnableMyLog, "[LscMgr:%s]  "
                "Meta %d, Shading param OnOff(%d) 0x%x, 0x%x,0x%x ,0x%x ,0x%x ,0x%x, 0x%x \n",
                __FUNCTION__,
                m_bMetaMode,
                m_fgOnOff,
                m_rIspLscCfg[m_eLscScenario].baseaddr.val,
                m_rIspLscCfg[m_eLscScenario].ctl1.val, m_rIspLscCfg[m_eLscScenario].ctl2.val,
                m_rIspLscCfg[m_eLscScenario].ctl3.val, m_rIspLscCfg[m_eLscScenario].lblock.val,
                m_rIspLscCfg[m_eLscScenario].ratio.val,
                m_rIspLscCfg[m_eLscScenario].gain_th.val);
                
        ISP_MGR_LSC_T::getInstance(m_eActive).put(m_rIspLscCfg[m_eLscScenario]); //put to ispmgr_mt6573
        fgRet = ISP_MGR_LSC_T::getInstance(m_eActive).apply(getIspProfile()); //Apply to ISP.

        UpdateSL2Param();
    }
    else
    {
        MY_LOG("[LscMgr] %s fail to read registers", __FUNCTION__);
    }

    lbExit:
    return fgRet;
}

MVOID 
LscMgr::RawLscTblDump(const char* filename)
{
    char strfile[128];
    MINT32 Scenario, i4Dbg;
    FILE* fpdebug;

    MY_LOG("[%s]\n", __FUNCTION__);

    GET_PROP("debug.lsc_mgr.dump123", "0", i4Dbg);

    if (!i4Dbg)
    {
        MY_LOG("[%s] Not to dump (%s)", __FUNCTION__, filename);
        return;
    }

    sprintf(strfile, "/sdcard/lsc1to3data/%s.log", filename);

    fpdebug = fopen(strfile, "w");
    
    if ( fpdebug == NULL )
    {
        MY_ERR("Can't open :%s\n", filename);
        return;
    } 

    for (Scenario = 0; Scenario < LSC_SCENARIO_30; Scenario++)
    {
        MUINT32 ct = 0;
        MUINT32* Addr = (MUINT32*) stRawLscInfo[Scenario].virtAddr;

        fprintf(fpdebug, "Scenario%d: {\n", Scenario);
        for (ct = 0; ct < 4; ct++)
        {
            MUINT32* AddrEnd = (MUINT32*) Addr + getPerLutSize((ELscScenario_T)Scenario)/4;

            fprintf(fpdebug, "    {\n");
            while (Addr < AddrEnd)
            {
                MUINT32 a, b, c, d;
                a = *Addr++;
                b = *Addr++;
                c = *Addr++;
                d = *Addr++;
                fprintf(fpdebug, "        0x%08x,0x%08x,0x%08x,0x%08x,\n", a, b, c, d);
            }
            fprintf(fpdebug, "    }, // ct%d\n", ct);
        }
        fprintf(fpdebug, "},\n");
    }
    fclose(fpdebug);
}

MINT32
LscMgr::
RawLscfreeMemory(IMEM_BUF_INFO& RawLscInfo)
{
    if (!m_pIMemDrv || RawLscInfo.virtAddr == 0)
    {
        MY_ERR("RawLsc Null m_pIMemDrv driver \n");
        return MFALSE;
    }
    MINT32 ret = MTRUE;

    if (!m_pIMemDrv->unmapPhyAddr(&RawLscInfo))
    {
        if (!m_pIMemDrv->freeVirtBuf(&RawLscInfo))
        {
            MY_LOG("RawLsc free VirtBuf/PhyBuf 0x%08x/0x%08x success\n",
                    RawLscInfo.virtAddr, RawLscInfo.phyAddr);
            RawLscInfo.virtAddr = 0;
            ret = MTRUE;
        }
        else
        {
            MY_ERR("RawLsc fVirtBuf/PhyBuf 0x%08x/0x%08x error\n",
                    RawLscInfo.virtAddr, RawLscInfo.phyAddr);
            ret = MFALSE;
        }
    }
    else
    {
        MY_ERR("RawLsc unmapPhyAddr error\n");
        ret = MFALSE;
    }

    return ret;
}

MBOOL
LscMgr::
RawLscTblMemInfoShow(IMEM_BUF_INFO& RawLscInfo)
{
    //MY_LOG("[LscMgr] RawLscTblMemInfoShow \n");
    MY_LOG("[LscMgr]RawLscInfo.virtAddr 0x%08x\n",
            RawLscInfo.virtAddr);
    MY_LOG("[LscMgr]RawLscInfo.phyAddr 0x%08x\n",
            RawLscInfo.phyAddr);
    MY_LOG("[LscMgr]RawLscInfo.size 0x%08x\n",
            RawLscInfo.size);
    return MTRUE;
}

MBOOL
LscMgr::
RawLscTblFlushCurrTbl(void) {
    m_pIMemDrv->cacheFlushAll();
    return MTRUE;
}

MBOOL
LscMgr::
RawLscTblSetPhyVirAddr(MUINT32 const u8LscIdx, MVOID* pPhyAddr,
        MVOID* pVirAddr) {
    MY_LOG("[LscMgr] %s not allowed!", __FUNCTION__);
    MBOOL ret = MFALSE;
    return ret;
}

MUINT32
LscMgr::
RawLscTblGetPhyAddr(MUINT32 const u8LscIdx) {
    return stRawLscInfo[u8LscIdx].phyAddr;
}

MUINT32
LscMgr::
RawLscTblGetVirAddr(MUINT32 const u8LscIdx) {
    return stRawLscInfo[u8LscIdx].virtAddr;
}


//to work around double buffer
MVOID 
LscMgr::
RawLscTblClear(ELscScenario_T Scenario, UINT8 ColorTemp)
{
	MUINT32 *virAddr = (UINT32*)reinterpret_cast<MVOID*>(stRawLscInfo[Scenario].virtAddr+ColorTemp*getPerLutSize(Scenario));
	MUINT32 memSize = getPerLutSize(Scenario);
	memset(virAddr, 0, memSize); 
	MY_LOG("[%s] Scenario %d, ColorTemp %d, Addr 0x%08x\n", __FUNCTION__,
	    Scenario,
	    ColorTemp,
	    stRawLscInfo[m_eLscScenario].virtAddr+m_u4CTIdx*getPerLutSize(m_eLscScenario));
}

MBOOL
LscMgr::
RawLscTblAlloc(IMEM_BUF_INFO& RawLscInfo,
        MUINT32 const u8LscLutSize)
{
    MBOOL mbret = MFALSE;
    MUINT32 ret;

    if (!RawLscInfo.virtAddr)
    {
        MY_LOG("[%s] RawLscInfo.u4VirAddr 0x%08x, size %d\n", __FUNCTION__,
                RawLscInfo.virtAddr,
                u8LscLutSize);
        RawLscInfo.size = u8LscLutSize;

        if (!m_pIMemDrv->allocVirtBuf(&RawLscInfo))
        {
            if (m_pIMemDrv->mapPhyAddr(&RawLscInfo))
            {
                MY_LOG("mapPhyAddr error, size 0x%04x , virtAddr 0x%04x\n",
                        RawLscInfo.size,
                        RawLscInfo.virtAddr);
                mbret = MFALSE;
            }
            else
                mbret = MTRUE;
        }
        else
        {
            MY_LOG("allocVirtBuf error, size 0x%04x \n",
                    RawLscInfo.size);
            mbret = MFALSE;
        }
    }
    else
    {
        mbret = MTRUE;
        MY_LOG("already ! RawLscInfo.virtAddr 0x%8x, size%d\n", RawLscInfo.virtAddr, u8LscLutSize);
    }
    return mbret;
}

MBOOL
LscMgr::
RawLscTblInit() {
    MBOOL ret = MFALSE;

    UINT32 u8LscIdx = 0;


    ret = MTRUE;
    if (!m_pIMemDrv) {
        MY_LOG("new pIMemDrv()\n");
        MY_LOG("sizeof(stRawLscInfo) = %d\n", sizeof(stRawLscInfo));
        //::memset(&stRawLscInfo, 0x00, sizeof(stRawLscInfo));
        m_pIMemDrv = IMemDrv::createInstance();

        if (!m_pIMemDrv)
        {
            MY_LOG("m_pIMemDrv new fail.\n");
            ret = MFALSE;
        }
        else
        {
            MY_LOG("m_pIMemDrv createInstance success!\n");
            ret = m_pIMemDrv->init();
            if (ret == MTRUE)
            {
                MY_LOG("m_pIMemDrv init success!\n");

                for (u8LscIdx = 0; u8LscIdx < SHADING_SUPPORT_OP_NUM; u8LscIdx++)
                {
                    MY_LOG("[%s] -------------stRawLscInfo %d ---------------", __FUNCTION__, u8LscIdx);
                    if (!RawLscTblAlloc(stRawLscInfo[u8LscIdx], u4BufSizeU8[u8LscIdx]))
                    {
                        MY_LOG("RawLscTblAlloc(%d) FAILED\n", u8LscIdx);
                    }
                    else
                    {
                        RawLscTblMemInfoShow(stRawLscInfo[u8LscIdx]);
                    }
                }

                for (u8LscIdx = 0; u8LscIdx < 2; u8LscIdx++)
                {
                    MY_LOG("[%s] -------------m_rBufInfo[%d] ---------------", __FUNCTION__, u8LscIdx);
                    if (!RawLscTblAlloc(m_rBufInfo[u8LscIdx], getPerLutSize(LSC_SCENARIO_04)))
                    {
                        MY_LOG("RawLscTblAlloc(%d) FAILED\n", u8LscIdx);
                    }
                    else
                    {
                        RawLscTblMemInfoShow(m_rBufInfo[u8LscIdx]);
                    }
                }

#if ENABLE_TSF
                for (u8LscIdx = 0; u8LscIdx < sizeof(m_TSFBuff)/sizeof(IMEM_BUF_INFO); u8LscIdx++)
                {
                    MY_LOG("[%s] -------------m_TSFBuff %d ---------------", __FUNCTION__, u8LscIdx);
                    if (!RawLscTblAlloc(m_TSFBuff[u8LscIdx], u4TSFBufSizeU8[u8LscIdx]))
                    {
                        MY_LOG("m_TSFBuff(%d) FAILED\n", u8LscIdx);
                    }
                    else
                    {
                        RawLscTblMemInfoShow(m_TSFBuff[u8LscIdx]);
                    }
                }
#endif
                ret = MTRUE;
            }
            else
            {
                ret = MFALSE;
                MY_LOG("m_pIMemDrv init fail!\n");
            }
        }
    }
    else
    {
        MY_LOG("m_pIMemDrv = 0x%8x\n", (MUINT32) m_pIMemDrv);
    }
    return ret;
}

MBOOL
LscMgr::
RawLscTblUnInit()
{
    UINT32 lu32ErrCode = 0, ret = 0;
    UINT32 i = 0;

    for (i = 0; i < SHADING_SUPPORT_OP_NUM; i++)
    {
        MY_LOG("~ RawLscTblUnInit(%d)!!!\n", i);
        RawLscfreeMemory(stRawLscInfo[i]);
    }

    for (i = 0; i < 2; i++)
    {
        MY_LOG("~ RawLscTblUnInit(%d)!!!\n", i);
        RawLscfreeMemory(m_rBufInfo[i]);
    }

#if ENABLE_TSF
    for (i = 0; i < sizeof(m_TSFBuff)/sizeof(IMEM_BUF_INFO); i++)
    {
        RawLscfreeMemory(m_TSFBuff[i]);
    }
#endif

    if (m_pIMemDrv)
    {
        m_pIMemDrv->uninit();
        m_pIMemDrv->destroyInstance();
        m_pIMemDrv = NULL;
    }
    return MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 
LscMgr::
dumpSdblk(const char* table_name, const ISP_NVRAM_LSC_T& LscConfig, const MUINT32 *ShadingTbl)
{
    string strTblName(table_name);
    string strFilename;
    FILE *fhwtbl,*fsdblk;

    if (ShadingTbl == NULL)
    {
        MY_LOG("[%s] NULL table\n", __FUNCTION__);
    }

    MY_LOG("[%s]+ ShadingTbl(0x%08x)\n", __FUNCTION__, ShadingTbl);
    
    //sprintf(strFilename, "%s.sdblk", table_name);
    strFilename = strTblName + ".sdblk";
    fsdblk = fopen(strFilename.c_str(), "w");
    if ( fsdblk == NULL )
    {
        MY_LOG("[%s] Can't open :%s\n", __FUNCTION__, (const char*) strFilename.c_str());
        return -1;
    }

    //sprintf(strFilename, "%s.hwtbl", table_name);               
    strFilename = strTblName + ".hwtbl";
    fhwtbl = fopen(strFilename.c_str(), "w"); 
    if ( fhwtbl == NULL )
    {
        MY_LOG("[%s] Can't open :%s\n", __FUNCTION__, (const char*) strFilename.c_str());
        return -1;
    }

    fprintf(fsdblk," %8d  %8d  %8d  %8d  %8d  %8d  %8d  %8d\n",
            LscConfig.ctl1.bits.SDBLK_XOFST,
            LscConfig.ctl1.bits.SDBLK_YOFST,
            LscConfig.ctl2.bits.SDBLK_WIDTH,
            LscConfig.ctl3.bits.SDBLK_HEIGHT,
            LscConfig.ctl2.bits.SDBLK_XNUM,
            LscConfig.ctl3.bits.SDBLK_YNUM,
            LscConfig.lblock.bits.SDBLK_lWIDTH,
            LscConfig.lblock.bits.SDBLK_lHEIGHT);

    MINT32 x_num = LscConfig.ctl2.bits.SDBLK_XNUM + 1;
    MINT32 y_num = LscConfig.ctl3.bits.SDBLK_YNUM + 1;

    MINT32 numCoef = x_num * y_num * 4 * 4;
    MINT32 i, c = 0;

    for (i = numCoef-1; i >= 0; i--)
    {
        int coef1, coef2, coef3;
        MUINT32 val = *ShadingTbl++;
        coef3 = (val& 0x3FF00000) >> 20;
        coef2 = (val& 0x000FFC00) >> 10;
        coef1 = val& 0x000003FF;
        fprintf(fsdblk, " %8d %8d %8d", coef1, coef2, coef3);
        fprintf(fhwtbl,"0x%08x, ", val);
        c ++;

        if (c == 4)
        {
            c = 0;
            fprintf(fhwtbl,"\n");
            fprintf(fsdblk,"\n");
        }
    }

    fclose(fhwtbl);
    fclose(fsdblk);

    return 0;
}

inline void setDebugTag(DEBUG_SHAD_INFO_T &a_rCamDebugInfo, MINT32 a_i4ID, MINT32 a_i4Value)
{
    a_rCamDebugInfo.Tag[a_i4ID].u4FieldID = CAMTAG(DEBUG_CAM_SHAD_MID, a_i4ID, 0);
    a_rCamDebugInfo.Tag[a_i4ID].u4FieldValue = a_i4Value;
}

MRESULT LscMgr::
getDebugInfo(DEBUG_SHAD_INFO_T &rShadingDbgInfo)
{
    ISP_NVRAM_LSC_T debug;
    ISP_MGR_LSC_T::getInstance(m_eActive).get(debug);

    MY_LOG("[%s] + size=%d\n", __FUNCTION__, sizeof(DEBUG_SHAD_INFO_T));

#if ENABLE_TSF
    MINT32* pTsfExif = NULL;
    MINT32* pTsfExifData = NULL;
    if (m_pTsfResultInfo)
    {
        pTsfExifData = m_pTsfResultInfo->ExifData;
        pTsfExif = reinterpret_cast<MINT32*>(pTsfExifData);
        MY_LOG("[%s] pTsfExif=0x%08x, ExifData=0x%08x\n", __FUNCTION__, (MUINT32)pTsfExif, (MUINT32)&m_pTsfResultInfo->ExifData[0]);
    }
#endif

    ::memset(&rShadingDbgInfo, 0, sizeof(rShadingDbgInfo));
    setDebugTag(rShadingDbgInfo, SHAD_TAG_VERSION, (MUINT32)SHAD_DEBUG_TAG_VERSION);
    setDebugTag(rShadingDbgInfo, SHAD_TAG_1TO3_EN, (m_rIspNvram.ISPComm.CommReg[EEPROM_INFO_IN_COMM_LOAD] == CAL_DATA_LOAD));
    setDebugTag(rShadingDbgInfo, SHAD_TAG_SCENE_IDX, (MUINT32)m_eLscScenario);
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CT_IDX, (MUINT32)m_u4CTIdx);
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CAM_CTL_DMA_EN, (MUINT32)debug.lsci_en.val);
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CAM_LSCI_BASE_ADDR, (MUINT32)debug.baseaddr.val);
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CAM_LSCI_XSIZE, (MUINT32)debug.xsize.val);
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CAM_CTL_EN1, (MUINT32)debug.lsc_en.val);
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CAM_LSC_CTL1, (MUINT32)debug.ctl1.val);
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CAM_LSC_CTL2, (MUINT32)debug.ctl2.val);
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CAM_LSC_CTL3, (MUINT32)debug.ctl3.val);
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CAM_LSC_LBLOCK, (MUINT32)debug.lblock.val);
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CAM_LSC_RATIO, (MUINT32)debug.ratio.val);
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CAM_LSC_GAIN_TH, (MUINT32)debug.gain_th.val);

#if ENABLE_TSF
    setDebugTag(rShadingDbgInfo, SHAD_TAG_TSF_EN, m_bTSF && m_fgTsfSetTbl);

    if (pTsfExif && m_bTSF)
    {
        MINT32 i;
        ::pthread_mutex_lock(&mTSFMutex);
        for (i = SHAD_TAG_CNT1; i < SHAD_TAG_END; i++)
        {
            setDebugTag(rShadingDbgInfo, i, *pTsfExif++);
        }
        ::pthread_mutex_unlock(&mTSFMutex);
    }
#endif

    if (m_bDumpSdblk)
    {
        if (dumpSdblk(m_strSdblkFile.c_str(), debug, m_pu4CurAddr) != 0)
            MY_ERR("[%s] dumpSdblk error\n", __FUNCTION__);
    }

    MY_LOG("[%s] -\n", __FUNCTION__);
    return 0;
}

MRESULT
LscMgr::
getDebugTbl(DEBUG_SHAD_ARRAY_INFO_T &rShadingDbgTbl)
{
    ::memset(&rShadingDbgTbl, 0, sizeof(rShadingDbgTbl));
    rShadingDbgTbl.hdr.u4KeyID = DEBUG_SHAD_ARRAY_KEYID;
    rShadingDbgTbl.hdr.u4ModuleCount = ModuleNum<1, 0>::val;
    rShadingDbgTbl.hdr.u4DbgSHADArrayOffset = sizeof(DEBUG_SHAD_ARRAY_INFO_S::Header);

    rShadingDbgTbl.rDbgSHADArray.ArrayHeight = 1200;
    rShadingDbgTbl.rDbgSHADArray.ArrayWidth = 1600;
    rShadingDbgTbl.rDbgSHADArray.Array[0] = 0xdeadbeef;
    return 0;
}

LscMgr::
LscMgr(ESensorDev_T eSensorDev,
        NVRAM_CAMERA_ISP_PARAM_STRUCT& rIspNvram,
        ISP_SHADING_STRUCT& rShadingLut):
        m_u4CTIdx(0),
        m_u4DoubleBufIdx(0),
        m_fgUserSetTbl(0),
        m_eActive(eSensorDev),
        m_SensorDev((halSensorDev_e)eSensorDev),
        m_eSensorOp(ACDK_SCENARIO_ID_CAMERA_PREVIEW),
        m_ePrevSensorOp(ACDK_SCENARIO_ID_MAX),
        m_eIspProfile(EIspProfile_NUM),
        m_eLscScenario(LSC_SCENARIO_01),
        m_ePrevLscScenario(LSC_SCENARIO_NUM),
        m_u4SensorID(0),
        m_bIsEEPROMImported(MFALSE),
        m_bIsLutLoaded(MFALSE),
        m_bBypass(MFALSE),
        m_bMetaMode(MFALSE),
        m_rIspNvram(rIspNvram),
        m_rIspSl2Cfg(rIspNvram.ISPRegs.SL2),
        m_rIspLscCfg(rIspNvram.ISPRegs.LSC),
        m_rIspShadingLut(rShadingLut),
        m_pIMemDrv(NULL),
        m_bDumpSdblk(MFALSE),
        //m_pIspMgr(NULL),
        //m_pIspDrv(NULL),
        m_pSensorHal(NULL),
        m_fgOnOff(MTRUE)
#if ENABLE_TSF
,
m_bTSF(MFALSE), m_bTsfForceAWB(MFALSE), m_fgTsfSetTbl(MFALSE), m_fgPreflash(MFALSE), m_fgCtIdxExcd(MFALSE)
#endif
{

    MY_LOG("[LscMgr] "
            "m_pIMemDrv == 0x%08x\n"
            "m_rIspLscCfg 0x%08x, m_rIspShadingLut 0x%08x, m_rIspSl2Cfg 0x%08x, m_rIspNvram 0x%08x\n",
            (MUINT32)m_pIMemDrv,
            (MUINT32)&m_rIspLscCfg,
            (MUINT32)&m_rIspShadingLut,
            (MUINT32)&m_rIspSl2Cfg,
            (MUINT32)&m_rIspNvram);

    MY_LOG("[LscMgr] Shading\n Version = 0x%x\n SensorId =  0x%x\n PreviewSVDSize =  0x%x\n VideoSVDSize =  0x%x\n CaptureSVDSize =  0x%x\n",
            m_rIspShadingLut.Version,
            m_rIspShadingLut.SensorId,
            m_rIspShadingLut.PreviewSVDSize,
            m_rIspShadingLut.VideoSVDSize,
            m_rIspShadingLut.CaptureSVDSize
    );
    {
        ISP_NVRAM_LSC_T debug;
        ISP_MGR_LSC_T::getInstance(m_eActive).reset();
        ISP_MGR_LSC_T::getInstance(m_eActive).get(debug);
        MY_LOG("[LscMgr]  %s"
                "LscMgr() Shading param 0x%x, 0x%x, 0x%x, 0x%x,0x%x ,0x%x ,0x%x ,0x%x, 0x%x \n",
                __FUNCTION__,
                debug.lsci_en.val,
                debug.baseaddr.val,
                debug.lsc_en.val,
                debug.ctl1.val, debug.ctl2.val,
                debug.ctl3.val, debug.lblock.val,
                debug.ratio.val,
                debug.gain_th.val);
    }

    MY_LOG("[LscMgr] ENTER LscMgr\n");
}

MBOOL
LscMgr::init() {
    MY_LOG("[LscMgr:%s] \n", __FUNCTION__);

    // reset LSC register for CQ0
    ISP_NVRAM_LSC_T lsc_off;
    ISP_NVRAM_SL2_T sl2_off;
    ::memset(&lsc_off, 0, sizeof(ISP_NVRAM_LSC_T));
    ::memset(&sl2_off, 0, sizeof(ISP_NVRAM_SL2_T));
    ISP_MGR_LSC_T::getInstance(m_eActive).put(lsc_off);
    ISP_MGR_LSC_T::getInstance(m_eActive).apply(EIspProfile_NormalPreview);
    ISP_MGR_LSC_T::getInstance(m_eActive).apply(EIspProfile_NormalCapture);

    ISP_MGR_SL2_T::getInstance(m_eActive).put(sl2_off);
    //ISP_MGR_SL2_T::getInstance(m_eActive).apply(EIspProfile_NormalPreview);
    //ISP_MGR_SL2_T::getInstance(m_eActive).apply(EIspProfile_NormalCapture);

    GET_PROP("debug.lsc_mgr.log", "0", _bEnableMyLog);

    CPTLog(Event_Pipe_3A_ISP, CPTFlagStart);
    if (!RawLscTblInit())
    {
        MY_LOG("FATAL WRONG m_pIMemDrv new fail.\n");
    }
    CPTLog(Event_Pipe_3A_ISP, CPTFlagEnd);

    m_pSensorHal = SensorHal::createInstance();
    if (m_pSensorHal->init())
    {
        m_pSensorHal->destroyInstance();
        m_pSensorHal = NULL;
    }

    IspDebug::getInstance().init();

#if ENABLE_TSF
    ::sem_init(&mTSFSem, 0, 0);
    m_bTSF = isEnableTSF(m_eActive) ? MTRUE : MFALSE;
    m_pTsfResultInfo = NULL;
    mTSFState = LSCMGR_TSF_STATE_IDLE;
    ::pthread_create(&mTSFThread, NULL, (VPT)mThreadLoop, this);
    MY_LOG("[%s] Create TSF thread(0x%08x), m_bTSF(%d)\n", __FUNCTION__, (MUINT32) mTSFThread, m_bTSF);
#endif

    MY_LOG("[LscMgr:%s] (m_pIMemDrv:0x%8x) (m_pSensorHal:0x%0x) \n", __FUNCTION__,
            (MUINT32) m_pIMemDrv, (MUINT32)m_pSensorHal);

    return MTRUE;
}

LscMgr::
~LscMgr()
{
    MY_LOG("[LscMgr] "
            "EXIT ~LscMgr(m_pIMemDrv - 0x%8x) >>\n", (MUINT32) m_pIMemDrv);
    enableLscWoVariable (MFALSE);
    {
        ISP_NVRAM_LSC_T debug;
        ISP_MGR_LSC_T::getInstance(m_eActive).get(debug);
        MY_LOG("[LscMgr]  %s"
                "LscMgr() Shading param 0x%x, 0x%x, 0x%x, 0x%x,0x%x ,0x%x ,0x%x ,0x%x, 0x%x \n",
                __FUNCTION__,
                debug.lsci_en.val,
                debug.baseaddr.val,
                debug.lsc_en.val,
                debug.ctl1.val, debug.ctl2.val,
                debug.ctl3.val, debug.lblock.val,
                debug.ratio.val,
                debug.gain_th.val);
    }

    MY_LOG("[LscMgr] "
            "EXIT ~LscMgr(m_pIMemDrv - 0x%8x) <<\n", (MUINT32) m_pIMemDrv);
}

MBOOL
LscMgr::uninit()
{
    MY_LOG("[LscMgr:%s] \n", __FUNCTION__);

    IspDebug::getInstance().uninit();

    m_u4CTIdx = 0;
    m_eSensorOp=ACDK_SCENARIO_ID_CAMERA_PREVIEW;
    m_ePrevSensorOp=ACDK_SCENARIO_ID_MAX;
    m_eIspProfile=EIspProfile_NUM;
    m_eLscScenario=LSC_SCENARIO_01;
    m_ePrevLscScenario=LSC_SCENARIO_NUM;
    m_bIsEEPROMImported=MFALSE;
    m_bIsLutLoaded=MFALSE;
    m_bMetaMode = MFALSE;
    m_bDumpSdblk = MFALSE;

#if ENABLE_TSF
    m_bTSF = MFALSE;
    if (mTSFState != LSCMGR_TSF_STATE_EXIT)
    {
        ::pthread_mutex_lock(&mTSFMutex);
        mTSFState = LSCMGR_TSF_STATE_EXIT;
        ::pthread_mutex_unlock(&mTSFMutex);
        ::sem_post(&mTSFSem);
        ::pthread_join(mTSFThread, NULL);
    }
    m_bTSF = isEnableTSF(m_eActive) ? MTRUE : MFALSE;
#endif

    RawLscTblUnInit();

    if (m_pSensorHal)
    {
        m_pSensorHal->uninit();
        m_pSensorHal->destroyInstance();
        m_pSensorHal = NULL;
    }

    MY_LOG("[LscMgr:%s] (m_pIMemDrv:0x%8x) (m_pSensorHal:0x%0x) \n", __FUNCTION__,
            (MUINT32) m_pIMemDrv, (MUINT32)m_pSensorHal);

    return MTRUE;
}

#if ENABLE_TSF
MVOID
LscMgr::
fillTSFLscConfig(MTK_TSF_LSC_PARAM_STRUCT &config, EIspProfile_T profile)
{
    ACDK_SCENARIO_ID_ENUM sensor_scenario = getSensorScenarioByIspProfile(profile);
    ELscScenario_T lsc_scenario =  getLscScenarioBySensorScenario(sensor_scenario);

    if (m_bMetaMode == MTRUE && profile == EIspProfile_NormalCapture) {
        MY_LOG("[LscMgr:%s] MetaMode reassign sensor/lsc scenario %d, %d", __FUNCTION__,
                m_eSensorOp,
                m_eLscScenario);
        sensor_scenario = m_eSensorOp;
        lsc_scenario = m_eLscScenario;
    }

    config.raw_ht           = m_SensorCrop[sensor_scenario].u4CropH;//1902;
    config.raw_wd           = m_SensorCrop[sensor_scenario].u4CropW;//2532;
    config.x_offset         = m_rIspLscCfg[lsc_scenario].ctl1.bits.SDBLK_XOFST;//0;
    config.y_offset         = m_rIspLscCfg[lsc_scenario].ctl1.bits.SDBLK_YOFST;//0;
    config.block_wd         = m_rIspLscCfg[lsc_scenario].ctl2.bits.SDBLK_WIDTH;//79;   // half size
    config.block_ht         = m_rIspLscCfg[lsc_scenario].ctl3.bits.SDBLK_HEIGHT;//59;
    config.x_grid_num       = (MUINT16)m_rIspLscCfg[lsc_scenario].ctl2.bits.SDBLK_XNUM+2;//17;
    config.y_grid_num       = (MUINT16)m_rIspLscCfg[lsc_scenario].ctl3.bits.SDBLK_YNUM+2;//17;
    config.block_wd_last    = m_rIspLscCfg[lsc_scenario].lblock.bits.SDBLK_lWIDTH;//81;
    config.block_ht_last    = m_rIspLscCfg[lsc_scenario].lblock.bits.SDBLK_lHEIGHT;//66;

    MY_LOG("[%s] lsc, sensor %d, %d, \n"
            "(raw_wd, raw_ht, block_wd, block_ht, xgrid, ygrid, wd_last, ht_last) = \n"
            "(%d, %d, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x) \n", __FUNCTION__,
            lsc_scenario,
            sensor_scenario,
            config.raw_wd,
            config.raw_ht,
            config.block_wd,
            config.block_ht,
            config.x_grid_num,
            config.y_grid_num,
            config.block_wd_last,
            config.block_ht_last);
}

MVOID
LscMgr::
fillTSFInitParams(MTK_TSF_ENV_INFO_STRUCT &params)
{
    MY_LOG("[LscMgr:%s] \n", __FUNCTION__);

    // LSC table spec
    fillTSFLscConfig(*params.pLscConfig, m_eIspProfile);

    CAMERA_TSF_TBL_STRUCT* pDftTsf = NULL;
    MUINT32* pu4TsfData = (MUINT32*)getTSFTrainingData();
    MINT32* pi4TsfPara = (MINT32*)getTSFTuningData();
    if (MERR_OK != NvramDrvMgr::getInstance().init(m_eActive))
    {
        MY_ERR("[%s] Fail to init NvramDrvMgr, use default TSF table\n", __FUNCTION__);
    }
    else
    {
        NvramDrvMgr::getInstance().getRefBuf(pDftTsf);
        if (pDftTsf != NULL)
        {
            pu4TsfData = (MUINT32*) pDftTsf->TSF_DATA;
            pi4TsfPara = (MINT32*) pDftTsf->TSF_PARA;
            MY_LOG("[%s] Load TSF table OK, data(0x%08x), para(0x%08x)\n", __FUNCTION__, pu4TsfData, pi4TsfPara);
        }
        else
        {
            MY_ERR("[%s] Fail to get buf, use default TSF table, please check camera_tuning_para_sensor.cpp\n", __FUNCTION__);
        }
    }

    // General data
    params.ImgWidth         = AWB_WINDOW_NUM_X;
    params.ImgHeight        = AWB_WINDOW_NUM_Y;
    params.BayerOrder       = MTK_BAYER_B;
    // awb image statistics
    params.ImgAddr          = (MUINT8 *)m_TSFBuff[TSF_BUFIDX_AWB].virtAddr;
    // shading table 16x16 or 32x32 coeff
    params.ShadingTbl       = (MINT32*)((MINT8*)(getLut(m_eLscScenario))+getTSFD65Idx()*getPerLutSize(m_eLscScenario));
    params.Para             = pu4TsfData; //(MUINT32*)getTSFTrainingData();
    params.pTuningPara      = pi4TsfPara; //(MINT32*)getTSFTuningData();
    params.Raw16_9Mode      = 0;

    params.TS_TS            = 1;
    params.MA_NUM           = 5;

    MINT32 i4TsfSL2En = 0;
    GET_PROP("debug.lsc_mgr.tsfsl2", "-1", i4TsfSL2En);

    if (i4TsfSL2En == -1)
    {
        i4TsfSL2En = isEnableSL2();
        MY_LOG("[%s] TSF set SL2 default mode %d\n", __FUNCTION__, i4TsfSL2En);
    }
    else
    {
        MY_LOG("[%s] TSF set SL2 mode %d\n", __FUNCTION__, i4TsfSL2En);
    }

    params.EnableSL2        = i4TsfSL2En;

    MY_LOG("[%s]"
            "(ImgWidth, ImgHeight, BayerOrder, ImgAddr, ShadingTbl, Raw16_9Mode) = \n"
            "(0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, ) \n", __FUNCTION__,
            params.ImgWidth,
            params.ImgHeight,
            params.BayerOrder,
            // awb image
            params.ImgAddr,
            // shading table
            params.ShadingTbl,
            params.Raw16_9Mode);

    MY_LOG("[%s] \n"
            "(raw_wd, raw_ht, block_wd, block_ht, xgrid, ygrid, wd_last, ht_last) = \n"
            "(0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, ) \n", __FUNCTION__,
            params.pLscConfig->raw_wd,
            params.pLscConfig->raw_ht,
            params.pLscConfig->block_wd,
            params.pLscConfig->block_ht,
            params.pLscConfig->x_grid_num,
            params.pLscConfig->y_grid_num,
            params.pLscConfig->block_wd_last,
            params.pLscConfig->block_ht_last);

    NvramDrvMgr::getInstance().uninit();
}

MVOID
LscMgr::
updateTSFParamByIspProfile(MTK_TSF_ENV_INFO_STRUCT &params, EIspProfile_T profile)
{
    ACDK_SCENARIO_ID_ENUM sensor_scenario = getSensorScenarioByIspProfile(profile);
    ELscScenario_T lsc_scenario =  getLscScenarioBySensorScenario(sensor_scenario);

    if (m_bMetaMode == MTRUE && profile == EIspProfile_NormalCapture) {
        MY_LOG("[LscMgr:%s] MetaMode reassign sensor/lsc scenario", __FUNCTION__);
        sensor_scenario = m_eSensorOp;
        lsc_scenario = m_eLscScenario;
    }

    MY_LOG("[LscMgr:%s] \n", __FUNCTION__);
    // LSC table spec
    fillTSFLscConfig(*params.pLscConfig, profile);

    // General data
    params.ImgWidth         = AWB_WINDOW_NUM_X;
    params.ImgHeight        = AWB_WINDOW_NUM_Y;

    switch(m_SensorCrop[sensor_scenario].DataFmt)
    {
        case 0:
            params.BayerOrder = MTK_BAYER_B;
            break;
        case 1:
            params.BayerOrder = MTK_BAYER_Gb;
            break;
        case 2:
            params.BayerOrder = MTK_BAYER_Gr;
            break;
        case 3:
            params.BayerOrder = MTK_BAYER_R;
            break;
    }

    // shading table 16x16 or 32x32 coeff
    params.ShadingTbl       = (MINT32*)((MINT8*)(getLut(m_eLscScenario))+getTSFD65Idx()*getPerLutSize(m_eLscScenario));;//(MINT32 *)stRawLscInfo[lsc_scenario].virtAddr;

    if (lsc_scenario == LSC_SCENARIO_09_17)
        params.Raw16_9Mode      = 1;
    else
        params.Raw16_9Mode      = 0;

    MY_LOG("[%s] lsc, sensor %d, %d, \n"
            "(ImgWidth, ImgHeight, BayerOrder, ImgAddr, ShadingTbl, Raw16_9Mode) = \n"
            "(0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, ) \n", __FUNCTION__,
            lsc_scenario,
            sensor_scenario,
            params.ImgWidth,
            params.ImgHeight,
            params.BayerOrder,
            // awb image
            params.ImgAddr,
            // shading table
            params.ShadingTbl,
            params.Raw16_9Mode);
}

MVOID
LscMgr::
updateTSFInputParam(MTK_TSF_SET_PROC_INFO_STRUCT &params)
{
    MY_LOG_IF(_bEnableMyLog, "[%s] m_bTsfForceAWB(%d)\n", __FUNCTION__, m_bTsfForceAWB);

    const MINT32* pAwbForceParam = getTSFAWBForceInput();

    if (m_bTsfForceAWB && pAwbForceParam)
    {
        params.ParaL       	= pAwbForceParam[0];
        params.ParaC       	= pAwbForceParam[1];
        params.FLUO_IDX		= pAwbForceParam[2];
        params.DAY_FLUO_IDX	= pAwbForceParam[3];
    }
    else
    {
        params.ParaL       	= m_TsfAwbInfo.m_i4LV;
        params.ParaC       	= m_TsfAwbInfo.m_u4CCT;
        params.FLUO_IDX		= m_TsfAwbInfo.m_FLUO_IDX;
        params.DAY_FLUO_IDX	= m_TsfAwbInfo.m_DAY_FLUO_IDX;
    }
    params.RGAIN		= m_TsfAwbInfo.m_RGAIN;
    params.GGAIN		= m_TsfAwbInfo.m_GGAIN;
    params.BGAIN		= m_TsfAwbInfo.m_BGAIN;
    params.ShadingTbl  = (MINT32*)getTSFInputAddr(m_eIspProfile);

    MY_LOG_IF(_bEnableMyLog, "[%s] L(%d), C(%d), F(%d), DF(%d), R(%d), G(%d), B(%d)\n",
        __FUNCTION__, params.ParaL, params.ParaC, params.FLUO_IDX, params.DAY_FLUO_IDX, params.RGAIN, params.GGAIN, params.BGAIN);
}

MVOID LscMgr::backupTSFTbl(void)
{
    MUINT32 i4AddrInput = m_TSFBuff[TSF_BUFIDX_INPUT].virtAddr;
    MUINT32 i4AddrBak   = m_TSFBuff[TSF_BUFIDX_BAK].virtAddr;
    MUINT32 i4Size      = getPerLutSize(TSF_SCN_DFT);
    MY_LOG("[LscMgr:%s] input(0x%08x) => bak(0x%08x), size(%d)\n",
        __FUNCTION__, i4AddrInput, i4AddrBak, i4Size);
    memcpy((void*)i4AddrBak, (void*)i4AddrInput, i4Size);
}

MVOID LscMgr::restoreTSFTbl(void)
{
    MUINT32 i4AddrInput = m_TSFBuff[TSF_BUFIDX_INPUT].virtAddr;
    MUINT32 i4AddrBak   = m_TSFBuff[TSF_BUFIDX_BAK].virtAddr;
    MUINT32 i4Size      = getPerLutSize(TSF_SCN_DFT);
    MY_LOG("[LscMgr:%s] bak(0x%08x) => input(0x%08x), size(%d)\n",
        __FUNCTION__, i4AddrBak, i4AddrInput, i4Size);
    memcpy((void*)i4AddrInput, (void*)i4AddrBak, i4Size);
}

MVOID LscMgr::assignTSFTbl(ELscScenario_T eLscScn)
{
    MUINT32 i4AddrInput = m_TSFBuff[TSF_BUFIDX_INPUT].virtAddr;
#if USING_BUILTIN_LSC
    MUINT32 i4AddrTbl   = getLut(eLscScn);
#else
    MUINT32 i4AddrTbl   = stRawLscInfo[eLscScn].virtAddr+getTSFD65Idx()*getPerLutSize(eLscScn);
#endif
    MUINT32 i4NewSize   = getPerLutSize(eLscScn);

    MY_LOG("[LscMgr:%s] eLscScn(%d): Tbl(0x%08x) => input(0x%08x), size(%d)\n",
        __FUNCTION__, eLscScn, i4AddrTbl, i4AddrInput, i4NewSize);
    memcpy((void*)i4AddrInput, (void*)i4AddrTbl, i4NewSize);
}

MVOID
LscMgr::
prepareTSFInputBuffer(EIspProfile_T profile, LSCMGR_TSF_STATE state, MBOOL fgAspectChg)
{

    ACDK_SCENARIO_ID_ENUM sensor_scenario = getSensorScenarioByIspProfile(profile);
    ELscScenario_T lsc_scenario =  getLscScenarioBySensorScenario(sensor_scenario);
    MUINT32 i4AddrInput = m_TSFBuff[TSF_BUFIDX_INPUT].virtAddr;
    MUINT32 i4AddrBak   = m_TSFBuff[TSF_BUFIDX_BAK].virtAddr;

    MY_LOG("[LscMgr:%s] input(0x%08x), bak(0x%08x)\n", __FUNCTION__, i4AddrInput, i4AddrBak);

    if (m_bMetaMode == MTRUE && profile == EIspProfile_NormalCapture)
    {
        MY_LOG("[LscMgr:%s] MetaMode reassign sensor/lsc scenario", __FUNCTION__);
        sensor_scenario = m_eSensorOp;
        lsc_scenario = m_eLscScenario;
    }

    MY_LOG("[LscMgr:%s] EIspProfile_T (%d => %d), sensor %d, lsc %d, state %d\n",
        __FUNCTION__, m_ePrevIspProfile, profile, sensor_scenario, lsc_scenario, state);
                
    ELscScenario_T ePrev, eCur;

    if (fgAspectChg)
    {
        ePrev = (m_ePrevLscScenario == LSC_SCENARIO_04) ? LSC_SCENARIO_01 : m_ePrevLscScenario;
        eCur  = (lsc_scenario == LSC_SCENARIO_04) ? LSC_SCENARIO_01 : lsc_scenario;

        if (ePrev == LSC_SCENARIO_01)
        {
            // 4:3 to 16:9
            if (eCur != LSC_SCENARIO_01)
            {
                if (state == LSCMGR_TSF_STATE_SCENECHANGE)
                {
                    MY_LOG("[%s] SCENECHANGE: profile(%d => %d), scenario(%d => %d), backup/assign",
                        __FUNCTION__, m_ePrevIspProfile, profile, m_ePrevLscScenario, lsc_scenario);
                    backupTSFTbl();
                    assignTSFTbl(lsc_scenario);
                }
            }
        }
        else
        {
            // 16:9 vdo to 4:3
            if (eCur == LSC_SCENARIO_01)
            {
                MY_LOG("[%s] state(%d): profile(%d => %d), scenario(%d => %d), restore",
                    __FUNCTION__, state, m_ePrevIspProfile, profile, m_ePrevLscScenario, lsc_scenario);
                restoreTSFTbl();
            }
        }
    }

    UINT32 last = getPerLutSize(lsc_scenario)/4 - 4;
    MY_LOG("[LscMgr:%s] input_start 0x%08x/0x%08x/0x%08x/0x%08x", __FUNCTION__,
            *((MUINT32*)i4AddrInput+0),
            *((MUINT32*)i4AddrInput+1),
            *((MUINT32*)i4AddrInput+2),
            *((MUINT32*)i4AddrInput+3));
    MY_LOG("[LscMgr:%s] input_end   0x%08x/0x%08x/0x%08x/0x%08x", __FUNCTION__,
            *((MUINT32*)i4AddrInput+last+0),
            *((MUINT32*)i4AddrInput+last+1),
            *((MUINT32*)i4AddrInput+last+2),
            *((MUINT32*)i4AddrInput+last+3));
    
    MY_LOG("[LscMgr:%s] bak 0x%08x/0x%08x/0x%08x/0x%08x", __FUNCTION__,
            *((MUINT32*)i4AddrBak+0),
            *((MUINT32*)i4AddrBak+1),
            *((MUINT32*)i4AddrBak+2),
            *((MUINT32*)i4AddrBak+3));
}

MUINT32
LscMgr::
getTSFInputAddr(EIspProfile_T profile)
{
    return m_TSFBuff[TSF_BUFIDX_INPUT].virtAddr; // D65?
}

MUINT32
LscMgr::
getTSFOutputAddr(EIspProfile_T profile)
{
    return m_TSFBuff[TSF_BUFIDX_OUTPUT].virtAddr; // D65?
}

MVOID
LscMgr::
copyToTSFOutput(void)
{
    MUINT32 u4Addr;
    MUINT32 u4Idx;

    if (0 == m_fgUserSetTbl)
    {
        u4Addr = IspDebug::getInstance().readLsciAddr();
        u4Idx = (m_u4DoubleBufIdx == 0 ? 1 : 0);

        if (u4Addr != m_rBufInfo[m_u4DoubleBufIdx].phyAddr)
        {
            u4Idx = m_u4DoubleBufIdx;
            MY_LOG_IF(_bEnableMyLog, "[%s +] Error u4Idx(%d) Addr(0x%08x)\n", __FUNCTION__, u4Idx, u4Addr);
        }
        else
        {
            MY_LOG_IF(_bEnableMyLog, "[%s +] OK u4Idx(%d) Addr(0x%08x)\n", __FUNCTION__, u4Idx, u4Addr);
        }

        ::memcpy((MVOID*)m_rBufInfo[u4Idx].virtAddr, (MVOID*)m_TSFBuff[TSF_BUFIDX_INPUT].virtAddr, MAX_SHADING_PvwFrm_SIZE*sizeof(MUINT32));       

        RawLscTblFlushCurrTbl();  

        m_u4DoubleBufIdx = u4Idx;

        MY_LOG_IF(_bEnableMyLog, "[%s -]\n", __FUNCTION__);
    }
    else
    {
        MY_ERR("TSF and setGainTable are exclusive!");
    }
}

MBOOL
LscMgr::
checkAspectRatioChange(void)
{
    EIspProfile_T prev = m_ePrevIspProfile, cur = m_eIspProfile;
    ACDK_SCENARIO_ID_ENUM prev_idx = getSensorScenarioByIspProfile(prev);
    ACDK_SCENARIO_ID_ENUM cur_idx = getSensorScenarioByIspProfile(cur);
    MUINT32 prevH, prevW, curH, curW;
    prevH = m_SensorCrop[prev_idx].u4CropH;
    prevW = m_SensorCrop[prev_idx].u4CropW;
    curH = m_SensorCrop[cur_idx].u4CropH;
    curW = m_SensorCrop[cur_idx].u4CropW;

    MY_LOG("[LscMgr:%s]  prev H/W %d/%d, cur H/W %d/%d", __FUNCTION__,
            prevH, prevW,
            curH, curW);
    if (prevW * curH == prevH * curW) {
        return MFALSE;
    } else {
        return MTRUE;
    }
}

MBOOL
LscMgr::
isTSFEnable(void)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    MINT32 manual_tsf = 0;
    property_get("debug.lsc_mgr.manual_tsf", value, "-1");
    manual_tsf = atoi(value);

    if (manual_tsf != -1)
        if (manual_tsf == 0)
            m_bTSF = MFALSE;
        else
            m_bTSF = MTRUE;

    return m_bTSF;
}

MVOID
LscMgr::
dumpTSFInput(void)
{
    ACDK_SCENARIO_ID_ENUM sensor_scenario = getSensorScenarioByIspProfile(m_eIspProfile);
    ELscScenario_T lsc_scenario =  getLscScenarioBySensorScenario(sensor_scenario);

    if (m_bMetaMode == MTRUE && m_eIspProfile == EIspProfile_NormalCapture) {
        MY_LOG("[LscMgr:%s] MetaMode reassign sensor/lsc scenario", __FUNCTION__);
        sensor_scenario = m_eSensorOp;
        lsc_scenario = m_eLscScenario;
    }

    UINT32 size = getPerLutSize(lsc_scenario)/4;
    for (UINT32 i = 0; i < 16; i+=4) {
        MY_LOG("[LscMgr:%s: %d-%d] 0x%08x/0x%08x/0x%08x/0x%08x", __FUNCTION__,
                i,i+3,
                *((MUINT32*)m_TSFBuff[TSF_BUFIDX_INPUT].virtAddr+i+0),
                *((MUINT32*)m_TSFBuff[TSF_BUFIDX_INPUT].virtAddr+i+1),
                *((MUINT32*)m_TSFBuff[TSF_BUFIDX_INPUT].virtAddr+i+2),
                *((MUINT32*)m_TSFBuff[TSF_BUFIDX_INPUT].virtAddr+i+3));
    }

    for (UINT32 i = size-32; i < size; i+=4) {
        MY_LOG("[LscMgr:%s: %d-%d] 0x%08x/0x%08x/0x%08x/0x%08x", __FUNCTION__,
                i,i+3,
                *((MUINT32*)m_TSFBuff[TSF_BUFIDX_INPUT].virtAddr+i+0),
                *((MUINT32*)m_TSFBuff[TSF_BUFIDX_INPUT].virtAddr+i+1),
                *((MUINT32*)m_TSFBuff[TSF_BUFIDX_INPUT].virtAddr+i+2),
                *((MUINT32*)m_TSFBuff[TSF_BUFIDX_INPUT].virtAddr+i+3));
    }
}

void CheckTable(MUINT32* input, MUINT32* output, MUINT32 U32length) {
    for (UINT32 i = 0; i < 4; i+=4) {
        MY_LOG("[input:%s: %d-%d] 0x%08x/0x%08x/0x%08x/0x%08x", __FUNCTION__,
                i,i+3,
                *((MUINT32*)input+i+0),
                *((MUINT32*)input+i+1),
                *((MUINT32*)input+i+2),
                *((MUINT32*)input+i+3));
    }

    for (UINT32 i = U32length-4; i < U32length; i+=4) {
        MY_LOG("[input:%s: %d-%d] 0x%08x/0x%08x/0x%08x/0x%08x", __FUNCTION__,
                i,i+3,
                *((MUINT32*)input+i+0),
                *((MUINT32*)input+i+1),
                *((MUINT32*)input+i+2),
                *((MUINT32*)input+i+3));
    }

    ///////////////////////////////////////
    for (UINT32 i = 0; i < 4; i+=4) {
        MY_LOG("[output:%s: %d-%d] 0x%08x/0x%08x/0x%08x/0x%08x", __FUNCTION__,
                i,i+3,
                *((MUINT32*)output+i+0),
                *((MUINT32*)output+i+1),
                *((MUINT32*)output+i+2),
                *((MUINT32*)output+i+3));
    }

    for (UINT32 i = U32length-4; i < U32length; i+=4) {
        MY_LOG("[output:%s: %d-%d] 0x%08x/0x%08x/0x%08x/0x%08x", __FUNCTION__,
                i,i+3,
                *((MUINT32*)output+i+0),
                *((MUINT32*)output+i+1),
                *((MUINT32*)output+i+2),
                *((MUINT32*)output+i+3));
    }
}
/////////////////////////////////
// TSF state machine
/////////////////////////////////
void *
LscMgr::
mThreadLoop(void *arg)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    MINT32 dbg_tsf = 0;
    property_get("debug.lsc_mgr.dbg_tsf", value, "0");
    dbg_tsf = atoi(value);

    LscMgr *lsc = reinterpret_cast<LscMgr*>(arg);
    MY_LOG("[LscMgr:%s]  start state %d", __FUNCTION__, lsc->mTSFState);
    ::prctl(PR_SET_NAME,"Cam@3A-Lsc", 0, 0, 0);
    // set policy/priority
#if MTKCAM_HAVE_RR_PRIORITY
    int const policy    = SCHED_RR;
    int const priority  = PRIO_RT_F858_THREAD;
    //
    struct sched_param sched_p;
    ::sched_getparam(0, &sched_p);
    //
    //  set
    sched_p.sched_priority = priority;  //  Note: "priority" is real-time priority.
    ::sched_setscheduler(0, policy, &sched_p);
    //
    //  get
    ::sched_getparam(0, &sched_p);
#endif 
    // init thread variables
    ::pthread_mutex_init(&lsc->mTSFMutex, NULL);
    ::pthread_mutex_init(&lsc->mTSFMutexSC, NULL);
    ::sem_init(&lsc->mTSFSem, 0, 0);
    ::sem_init(&lsc->mTSFSemSC, 0, 0);

    MRESULT ret = S_TSF_OK;
    ////////////////////////////////////////////////
    static MTK_TSF_ENV_INFO_STRUCT         TSFInit;
    static MTK_TSF_GET_ENV_INFO_STRUCT     TSFGetEnv;
    static MTK_TSF_SET_PROC_INFO_STRUCT    TSFInput;
    static MTK_TSF_RESULT_INFO_STRUCT      TSFOutput;
    static MTK_TSF_GET_PROC_INFO_STRUCT    TSFGetProc;
    static MTK_TSF_GET_LOG_INFO_STRUCT     TSFGetLog;
    static MTK_TSF_LSC_PARAM_STRUCT        LscConfig;
    static MTKTSF_STATE_ENUM               TSFProcState;
    static MTK_TSF_TBL_STRUCT              TSFUpdateInfo;
    static MTK_TSF_LSC_PARAM_STRUCT        UpdateLscConfig;

    MTKTsf* tsf = MTKTsf::createInstance();
    unsigned char* gWorkinBuffer = NULL;
    unsigned char* gDBGWorkinBuffer = NULL;

    // (1) create tsf instance
    if (!tsf)
    {
        MY_ERR("[%s] NULL TSF instance", __FUNCTION__);
        ::pthread_exit(0);
        return NULL;
    }


    // (2) get/allocate tsf working buffer size
    MY_LOG("[%s] MTKTSF_FEATURE_GET_ENV_INFO ", __FUNCTION__);
    tsf->TsfFeatureCtrl(MTKTSF_FEATURE_GET_ENV_INFO, 0, &TSFGetEnv);
    MY_LOG("[%s] Queried working buffer size : %d bytes\n", __FUNCTION__, TSFGetEnv.WorkingBuffSize);
    // new working buffer
    gWorkinBuffer   = new unsigned char[TSFGetEnv.WorkingBuffSize];
    if (!gWorkinBuffer)
    {
        MY_ERR("[%s] NULL gWorkinBuffer", __FUNCTION__);
        ::pthread_exit(0);
        return NULL;
    }
    
    memset(gWorkinBuffer,0, TSFGetEnv.WorkingBuffSize);

    // (3) construct data relationship
    TSFInit.WorkingBufAddr = (MUINT32 *)gWorkinBuffer;
    TSFInit.pLscConfig = &LscConfig;

    if (dbg_tsf == 1) {
        gDBGWorkinBuffer = new unsigned char[TSFGetEnv.DebugBuffSize];
        TSFInit.DebugAddr = (MUINT32*)gDBGWorkinBuffer;
        TSFInit.DebugFlag = 1;
    }

    lsc->m_pTsfResultInfo = &TSFOutput;

    while (lsc->mTSFState != LSCMGR_TSF_STATE_EXIT)
    {
        MBOOL isLscActive = MFALSE;

        ::sem_wait(&lsc->mTSFSem);
        ::pthread_mutex_lock(&lsc->mTSFMutex);
        isLscActive = (lsc->isBypass()==MTRUE)?MFALSE:MTRUE;

        if (isLscActive == MTRUE) {
            isLscActive = lsc->isEnable();
        }

        if (lsc->isTSFEnable() == MFALSE) {
            ::pthread_mutex_unlock(&lsc->mTSFMutex);
            ::sem_post(&lsc->mTSFSemSC);
            continue;
        }

        lsc->updateTSFInputParam(TSFInput);
        TSFOutput.ShadingTbl = (MUINT32*)lsc->getTSFOutputAddr(lsc->getIspProfile());

        switch(lsc->mTSFState)
        {
            case LSCMGR_TSF_STATE_IDLE: //////////////////////////////////////////////////
                MY_LOG("[%s] LSCMGR_TSF_STATE_IDLE ", __FUNCTION__);
                break; // do nothing
            case LSCMGR_TSF_STATE_INIT: //////////////////////////////////////////////////
                MY_LOG("[%s] LSCMGR_TSF_STATE_INIT ", __FUNCTION__);
                // (4) init algorithm params
                if (lsc->loadTSFLut() == MTRUE) {
                    tsf->TsfExit();
                    lsc->fillTSFInitParams(TSFInit);
                    ret = tsf->TsfInit(&TSFInit, 0);
                    if (ret != S_TSF_OK) {
                        MY_ERR("TSF init error %x", ret);
                    } else {
                        ret = tsf->TsfFeatureCtrl(MTKTSF_FEATURE_SET_PROC_INFO, &TSFInput, 0);
                        if (ret != S_TSF_OK) {
                            MY_ERR("MTKTSF_FEATURE_SET_PROC_INFO error %x", ret);
                        } else
                            lsc->mTSFState = LSCMGR_TSF_STATE_SCENECHANGE;
                    }
                } else {
                    ::sem_post(&lsc->mTSFSemSC);
                    MY_ERR("[%s] loadTSFLut fail", __FUNCTION__);
                }
                break;
            case LSCMGR_TSF_STATE_SCENECHANGE:  //////////////////////////////////////////////////
                {
                    // cur lsc scenario
                    MBOOL fgAscpectChg = lsc->checkAspectRatioChange();
                    MY_LOG("[%s] LSCMGR_TSF_STATE_SCENECHANGE ", __FUNCTION__);
                    lsc->prepareTSFInputBuffer(lsc->getIspProfile(), LSCMGR_TSF_STATE_SCENECHANGE, fgAscpectChg);
                    lsc->fillTSFLscConfig(UpdateLscConfig, lsc->getIspProfile());
                    TSFUpdateInfo.pLscConfig = &UpdateLscConfig;
                    TSFUpdateInfo.ShadingTbl = (MINT32*)lsc->getTSFInputAddr(lsc->getIspProfile());//D65 shading table Cap Size

                    if (EIspProfile_NormalPreview != lsc->getIspProfile() /*&& EIspProfile_ZsdPreview_NCC != lsc->getIspProfile()*/)
                    {
                        MY_LOG("[%s] EIspProfile_NormalPreview != lsc->getIspProfile() ", __FUNCTION__);
                        if (fgAscpectChg) 
                        {
                            MY_LOG("[LscMgr:%s] VDO mode, diff AspecRatio TsfExit", __FUNCTION__);
                            tsf->TsfExit();
                            lsc->fillTSFInitParams(TSFInit);
                            ret = tsf->TsfInit(&TSFInit, 0);
                            if (ret != S_TSF_OK) {
                                MY_ERR("TSFInit error %x", ret);
                            } else {
                                ret = tsf->TsfFeatureCtrl(MTKTSF_FEATURE_SET_PROC_INFO, &TSFInput, 0);
                                if (ret != S_TSF_OK) {
                                    MY_ERR("MTKTSF_FEATURE_SET_PROC_INFO error %x", ret);
                                } else
                                    lsc->mTSFState = LSCMGR_TSF_STATE_GETNEWINPUT;
                            }
                        }
                        else if (EIspProfile_NormalCapture == lsc->getIspProfile() ||
                             EIspProfile_ZsdPreview_NCC == lsc->getIspProfile())
                        {
                            if (EIspProfile_VideoPreview == lsc->getPrevIspProfile() ||
                                EIspProfile_VideoCapture == lsc->getPrevIspProfile())
                            {
                                MY_LOG("[LscMgr:%s] Normal Capture, MTKTSF_FEATURE_SET_TBL_CHANGE\n", __FUNCTION__);
                                ret = tsf->TsfFeatureCtrl(MTKTSF_FEATURE_SET_TBL_CHANGE, &TSFUpdateInfo, 0);
                                if (ret != S_TSF_OK) {
                                    MY_ERR("MTKTSF_FEATURE_SET_TBL_CHANGE error %x", ret);
                                }
                                MY_LOG("[LscMgr:%s] Normal Capture, MTKTSF_FEATURE_GEN_CAP_TBL\n", __FUNCTION__);
                                ret = tsf->TsfFeatureCtrl(MTKTSF_FEATURE_GEN_CAP_TBL, &TSFUpdateInfo, &TSFOutput);
                                if (ret != S_TSF_OK) {
                                    MY_ERR("MTKTSF_FEATURE_GEN_CAP_TBL error %x", ret);
                                } else {
                                    MY_LOG("[%s] memcpy src 0x%0x, dst 0x%0x, size(U8) %d", __FUNCTION__,
                                            TSFOutput.ShadingTbl,
                                            TSFInput.ShadingTbl,
                                            lsc->getPerLutSize((NSIspTuning::LscMgr::ELscScenario_T)lsc->getRegIdx()));

                                    ::memcpy((void*)TSFInput.ShadingTbl,
                                            (void*)TSFOutput.ShadingTbl,
                                            lsc->getPerLutSize((NSIspTuning::LscMgr::ELscScenario_T)lsc->getRegIdx()));
                                    lsc->copyToTSFOutput();
                                    lsc->SetTBAToISP();
                                    lsc->enableLsc(isLscActive);
                                    MY_LOG("[LscMgr:%s] MTKTSF_FEATURE_GEN_CAP_TBL complete!!\n", __FUNCTION__);
                                }

                                MY_LOG("[LscMgr:%s] prevIsp EIspProfile_VideoPreview||EIspProfile_VideoCapture", __FUNCTION__);
                                lsc->m_fgSetProcInfo = 1;
                                lsc->m_u4FrmCnt = 0;
                                lsc->mTSFState = LSCMGR_TSF_STATE_DO;
                            }
                            else
                            {
                                MY_LOG("[LscMgr:%s] Normal Capture, MTKTSF_FEATURE_SET_TBL_CHANGE\n", __FUNCTION__);
                                ret = tsf->TsfFeatureCtrl(MTKTSF_FEATURE_SET_TBL_CHANGE, &TSFUpdateInfo, 0);
                                if (ret != S_TSF_OK)
                                {
                                    MY_ERR("MTKTSF_FEATURE_SET_TBL_CHANGE error %x", ret);
                                }
                                MY_LOG("[LscMgr:%s] Normal Capture, MTKTSF_FEATURE_GEN_CAP_TBL\n", __FUNCTION__);
                                ret = tsf->TsfFeatureCtrl(MTKTSF_FEATURE_GEN_CAP_TBL, &TSFUpdateInfo, &TSFOutput);
                                if (ret != S_TSF_OK) 
                                {
                                    MY_ERR("MTKTSF_FEATURE_GEN_CAP_TBL error %x", ret);
                                } 
                                else 
                                {                              
                                    MY_LOG("[LscMgr:%s] MTKTSF_FEATURE_GEN_CAP_TBL complete!! Only update SL2\n", __FUNCTION__);
                                    lsc->UpdateSL2Param();
                                }
                                // remain the same table for preview to capture, non-zsd to zsd
                                MY_LOG("[LscMgr:%s] Normal Capture, remain the same table\n", __FUNCTION__);
                                lsc->mTSFState = LSCMGR_TSF_STATE_GETNEWINPUT;
                            }
                        } 
                        else
                            lsc->mTSFState = LSCMGR_TSF_STATE_GETNEWINPUT;
                    } 
                    else 
                    {
                        MY_LOG("[LscMgr:%s] HERE Apply previous table immediately", __FUNCTION__);
                        MY_LOG("[LscMgr:%s] HERE Preview, MTKTSF_FEATURE_SET_TBL_CHANGE\n", __FUNCTION__);

                        if (fgAscpectChg == MTRUE) {
                            MY_LOG("[LscMgr:%s] Vdo to Prv diff AspecRatio TsfExit", __FUNCTION__);
                            tsf->TsfExit();
                            lsc->fillTSFInitParams(TSFInit);
                            ret = tsf->TsfInit(&TSFInit, 0);
                            if (ret != S_TSF_OK) {
                                MY_ERR("TSFInit error %x", ret);
                            } else {
                                ret = tsf->TsfFeatureCtrl(MTKTSF_FEATURE_SET_PROC_INFO, &TSFInput, 0);
                                if (ret != S_TSF_OK) {
                                    MY_ERR("MTKTSF_FEATURE_SET_PROC_INFO error %x", ret);
                                } else
                                    lsc->mTSFState = LSCMGR_TSF_STATE_GETNEWINPUT;
                            }
                        }
                        else
                        {    
                            ret = tsf->TsfFeatureCtrl(MTKTSF_FEATURE_SET_TBL_CHANGE, &TSFUpdateInfo, 0);
                            if (ret != S_TSF_OK) {
                                MY_ERR("MTKTSF_FEATURE_SET_TBL_CHANGE error %x", ret);
                            }
                            lsc->copyToTSFOutput();
                            lsc->SetTBAToISP();
                            lsc->enableLsc(isLscActive);
                            lsc->mTSFState = LSCMGR_TSF_STATE_GETNEWINPUT;
                        }
                    }
                }
                break;
            case LSCMGR_TSF_STATE_GETNEWINPUT:  //////////////////////////////////////////////////
                // recalculate shading table according new input
                MY_LOG("[%s] LSCMGR_TSF_STATE_GETNEWINPUT ", __FUNCTION__);
                //lsc->prepareTSFInputBuffer(lsc->getIspProfile(), LSCMGR_TSF_STATE_GETNEWINPUT);
                lsc->fillTSFLscConfig(UpdateLscConfig, lsc->getIspProfile());
                lsc->updateTSFInputParam(TSFInput);
                TSFUpdateInfo.pLscConfig = &UpdateLscConfig;
                TSFUpdateInfo.ShadingTbl = (MINT32*)lsc->getTSFInputAddr(lsc->getIspProfile());//D65 shading table (capture size)
                TSFOutput.ShadingTbl = (MUINT32*)lsc->getTSFOutputAddr(lsc->getIspProfile());

                if (EIspProfile_NormalCapture == lsc->getIspProfile())
                {

                    MY_LOG("[%s] MTKTSF_FEATURE_BATCH Intput/Output tbl 0x%08x/0x%08x", __FUNCTION__,
                            TSFInput.ShadingTbl,
                            TSFOutput.ShadingTbl);
                    //CheckTable((MUINT32*)TSFInput.ShadingTbl, (MUINT32*)TSFOutput.ShadingTbl, lsc->getPerLutSize((NSIspTuning::LscMgr::ELscScenario_T)lsc->getRegIdx())/4);
#if TSF_RUN_BATCH_CAP
                    ret = tsf->TsfFeatureCtrl(MTKTSF_FEATURE_SET_TBL_CHANGE, &TSFUpdateInfo, 0);
                    if (ret != S_TSF_OK) {
                        MY_ERR("MTKTSF_FEATURE_SET_TBL_CHANGE error %x", ret);
                    }
                    ret = tsf->TsfFeatureCtrl(MTKTSF_FEATURE_BATCH, &TSFInput, &TSFOutput);
                    MY_LOG("[LscMgr:%s] MTKTSF_FEATURE_BATCH complete!!\n", __FUNCTION__);
                    if (ret != S_TSF_OK) {
                        MY_ERR("MTKTSF_FEATURE_BATCH error %x", ret);
                        lsc->dumpTSFInput();
                        tsf->TsfReset();
                        tsf->TsfFeatureCtrl(MTKTSF_FEATURE_SET_TBL_CHANGE, &TSFUpdateInfo, 0);
                    } else {
                        MY_LOG("[%s] memcpy src 0x%0x, dst 0x%0x, size %d", __FUNCTION__,
                                TSFOutput.ShadingTbl,
                                TSFInput.ShadingTbl,
                                lsc->getPerLutSize((NSIspTuning::LscMgr::ELscScenario_T)lsc->getRegIdx()));
                        ::memcpy((void*)TSFInput.ShadingTbl,
                                (void*)TSFOutput.ShadingTbl,
                                lsc->getPerLutSize((NSIspTuning::LscMgr::ELscScenario_T)lsc->getRegIdx()));

                        if (dbg_tsf == 1) {
                            char *filename = "/sdcard/tsfdata/TSFCap.bin";
                            MY_LOG("[LscMgr:%s] DBG: Output Capture Table to %s", __FUNCTION__, filename);
                            FILE* fpdebug = fopen(filename,"wb");
                            if ( fpdebug == NULL )
                            {
                                MY_ERR("Can't open :%s\n",filename);
                            } else {
                                fwrite((void*)TSFOutput.ShadingTbl,
                                        lsc->getPerLutSize((NSIspTuning::LscMgr::ELscScenario_T)lsc->getRegIdx()),
                                        1,fpdebug);
                                fclose(fpdebug);
                            }
                        }
                        lsc->copyToTSFOutput();
                        lsc->SetTBAToISP();
                        lsc->enableLsc(isLscActive);
                        MY_LOG("[LscMgr:%s] Exit LSCMGR_TSF_STATE_GETNEWINPUT!!\n", __FUNCTION__);
                    }

//                    {   // experiment on pass2 data flow
//                        ISP_NVRAM_OBC_T obc;
//                        //memset(&obc, 0, sizeof(ISP_NVRAM_OBC_T));
//                        ISP_MGR_OBC_T::getInstance(NSIspTuning::ESensorDev_Main).get(obc);
//                        obc.offst0.val = obc.offst1.val = obc.offst2.val = obc.offst3.val = 0;
//                        obc.gain0.val = obc.gain1.val = obc.gain2.val = obc.gain3.val = 512;
//                        ISP_MGR_OBC_T::getInstance(NSIspTuning::ESensorDev_Main).put(obc);
//                        ISP_MGR_OBC_T::getInstance(NSIspTuning::ESensorDev_Main).apply(NSIspTuning::EIspProfile_NormalCapture);
//                    }
#endif
                    ::sem_post(&lsc->mTSFSemSC);
                    lsc->m_fgSetProcInfo = 1;
                    lsc->m_u4FrmCnt = 0;
                    lsc->mTSFState = LSCMGR_TSF_STATE_DO;
                }
                else if (EIspProfile_ZsdPreview_NCC == lsc->getIspProfile())
                {
                    MY_LOG("[LscMgr:%s] ZSD\n", __FUNCTION__);
                    MY_LOG("[%s] MTKTSF_FEATURE_BATCH Intput/Output tbl 0x%08x/0x%08x", __FUNCTION__,
                            TSFInput.ShadingTbl,
                            TSFOutput.ShadingTbl);
#if TSF_RUN_BATCH_CAP
                    ret = tsf->TsfFeatureCtrl(MTKTSF_FEATURE_SET_TBL_CHANGE, &TSFUpdateInfo, 0);
                    if (ret != S_TSF_OK) {
                        MY_ERR("MTKTSF_FEATURE_SET_TBL_CHANGE error %x", ret);
                    }
                    ret = tsf->TsfFeatureCtrl(MTKTSF_FEATURE_BATCH, &TSFInput, &TSFOutput);
                    MY_LOG("[LscMgr:%s] MTKTSF_FEATURE_BATCH complete!!\n", __FUNCTION__);
                    if (ret != S_TSF_OK) {
                        MY_ERR("MTKTSF_FEATURE_BATCH error %x", ret);
                        lsc->dumpTSFInput();
                        tsf->TsfReset();
                        tsf->TsfFeatureCtrl(MTKTSF_FEATURE_SET_TBL_CHANGE, &TSFUpdateInfo, 0);
                    } else {
                        MY_LOG("[%s] memcpy src 0x%0x, dst 0x%0x, size %d", __FUNCTION__,
                                TSFOutput.ShadingTbl,
                                TSFInput.ShadingTbl,
                                lsc->getPerLutSize((NSIspTuning::LscMgr::ELscScenario_T)lsc->getRegIdx()));
                        ::memcpy((void*)TSFInput.ShadingTbl,
                                (void*)TSFOutput.ShadingTbl,
                                lsc->getPerLutSize((NSIspTuning::LscMgr::ELscScenario_T)lsc->getRegIdx()));

                        if (dbg_tsf == 1) {
                            char *filename = "/sdcard/tsfdata/TSFZsd.bin";
                            MY_LOG("[LscMgr:%s] DBG: Output Zsd Table to %s", __FUNCTION__, filename);
                            FILE* fpdebug = fopen(filename,"wb");
                            if ( fpdebug == NULL )
                            {
                                MY_ERR("Can't open :%s\n",filename);
                            } else {
                                fwrite((void*)TSFOutput.ShadingTbl,
                                        lsc->getPerLutSize((NSIspTuning::LscMgr::ELscScenario_T)lsc->getRegIdx()),
                                        1,fpdebug);
                                fclose(fpdebug);
                            }
                        }
                        lsc->copyToTSFOutput();
                        lsc->SetTBAToISP();
                        lsc->enableLsc(isLscActive);
                        MY_LOG("[LscMgr:%s] Exit LSCMGR_TSF_STATE_GETNEWINPUT!!\n", __FUNCTION__);
                    }
#endif
                    lsc->m_fgSetProcInfo = 1;
                    lsc->m_u4FrmCnt = 0;
                    lsc->mTSFState = LSCMGR_TSF_STATE_DO;
                }
                else
                {  // normal preview case
                    MY_LOG("[LscMgr:%s] Non Capture\n", __FUNCTION__);
                    MY_LOG("[%s] MTKTSF_FEATURE_SET_TBL_CHANGE ", __FUNCTION__);
                    ret = tsf->TsfFeatureCtrl(MTKTSF_FEATURE_SET_TBL_CHANGE, &TSFUpdateInfo, 0);
                    if (ret != S_TSF_OK) {
                        MY_ERR("MTKTSF_FEATURE_SET_TBL_CHANGE error %x", ret);
                    } else {
                        MY_LOG("[%s] MTKTSF_FEATURE_SET_PROC_INFO ", __FUNCTION__);
                        ret = tsf->TsfFeatureCtrl(MTKTSF_FEATURE_SET_PROC_INFO, &TSFInput, 0);
                        if (ret != S_TSF_OK) {
                            MY_ERR("MTKTSF_FEATURE_SET_PROC_INFO error %x", ret);
                        } else {
                            lsc->m_fgSetProcInfo = 1;
                            lsc->m_u4FrmCnt = 0;
                            lsc->mTSFState = LSCMGR_TSF_STATE_DO;
                        }
                    }
                }
                break;
            case LSCMGR_TSF_STATE_DO:   //////////////////////////////////////////////////
                if (EIspProfile_NormalCapture == lsc->getIspProfile())
                {
#if 1 // recalculate
                    // burst shot
                    ret = tsf->TsfFeatureCtrl(MTKTSF_FEATURE_SET_TBL_CHANGE, &TSFUpdateInfo, 0);
                    if (ret != S_TSF_OK) {
                        MY_ERR("MTKTSF_FEATURE_SET_TBL_CHANGE error %x", ret);
                    }
                    ret = tsf->TsfFeatureCtrl(MTKTSF_FEATURE_BATCH, &TSFInput, &TSFOutput);
                    if (ret != S_TSF_OK) {
                        MY_ERR("MTKTSF_FEATURE_BATCH error %x", ret);
                        lsc->dumpTSFInput();
                        tsf->TsfReset();
                        tsf->TsfFeatureCtrl(MTKTSF_FEATURE_SET_TBL_CHANGE, &TSFUpdateInfo, 0);
                    } else {
                        ::memcpy((void*)TSFInput.ShadingTbl,
                                (void*)TSFOutput.ShadingTbl,
                                lsc->getPerLutSize((NSIspTuning::LscMgr::ELscScenario_T)lsc->getRegIdx()));
                        lsc->copyToTSFOutput();
                        lsc->SetTBAToISP();
                        lsc->enableLsc(isLscActive);
                        MY_LOG("[LscMgr:%s] MTKTSF_FEATURE_BATCH complete!!\n", __FUNCTION__);
                    }
#else
                    // no smooth needed
                    MY_LOG("[LscMgr:%s] Continue Shot, MTKTSF_FEATURE_GEN_CAP_TBL\n", __FUNCTION__);
                    ret = tsf->TsfFeatureCtrl(MTKTSF_FEATURE_GEN_CAP_TBL, &TSFUpdateInfo, &TSFOutput);
                    if (ret != S_TSF_OK) {
                        MY_ERR("MTKTSF_FEATURE_GEN_CAP_TBL error %x", ret);
                    } else {
                        ::memcpy((void*)TSFInput.ShadingTbl,
                                (void*)TSFOutput.ShadingTbl,
                                lsc->getPerLutSize((NSIspTuning::LscMgr::ELscScenario_T)lsc->getRegIdx()));
                        MY_LOG("[LscMgr:%s] MTKTSF_FEATURE_GEN_CAP_TBL complete!!\n", __FUNCTION__);
                        lsc->copyToTSFOutput();
                        lsc->SetTBAToISP();
                        lsc->enableLsc(MTRUE);
                    }
#endif // recalculate
                    ::sem_post(&lsc->mTSFSemSC); // for continue shot
                }
                else
                {
                    if (lsc->m_fgSetProcInfo)
                    {
                        ret = tsf->TsfFeatureCtrl(MTKTSF_FEATURE_SET_PROC_INFO, &TSFInput, 0);
                        lsc->m_fgSetProcInfo = 0;
                    }
                    if (ret != S_TSF_OK)
                    {
                        MY_ERR("MTKTSF_FEATURE_SET_PROC_INFO error %x", ret);
                    }
                    else
                    {
                        ret = tsf->TsfMain();
                        if (ret != S_TSF_OK)
                        {
                            MY_ERR("TsfMain error %x", ret);
                            lsc->dumpTSFInput();
                            tsf->TsfReset();
                            lsc->mTSFState = LSCMGR_TSF_STATE_GETNEWINPUT;
                        }
                        else
                        {
                            tsf->TsfFeatureCtrl(MTKTSF_FEATURE_GET_PROC_INFO, 0, &TSFGetProc);
                            if (TSFGetProc.TsfState == MTKTSF_STATE_READY)
                            {
                                lsc->m_fgSetProcInfo = 1;
                                ret = tsf->TsfFeatureCtrl(MTKTSF_FEATURE_GET_RESULT, 0, &TSFOutput);
                                if (ret != S_TSF_OK)
                                {
                                    MY_ERR("MTKTSF_FEATURE_GET_RESULT error %x", ret);
                                    tsf->TsfReset();
                                }
                                else
                                {
                                    MY_LOG_IF(_bEnableMyLog, "[LscMgr:%s] MTKTSF_FEATURE_GET_RESULT \n", __FUNCTION__);
                                    MY_LOG_IF(_bEnableMyLog, "[%s] Copy output 0x%08x back to input 0x%08x\n", __FUNCTION__,
                                            (MUINT32)TSFOutput.ShadingTbl,
                                            (MUINT32)TSFInput.ShadingTbl);

                                    memcpy((void*)TSFInput.ShadingTbl,
                                            (void*)TSFOutput.ShadingTbl,
                                            lsc->getPerLutSize((NSIspTuning::LscMgr::ELscScenario_T)lsc->getRegIdx()));
                                    lsc->copyToTSFOutput();
                                    lsc->SetTBAToISP();
                                    lsc->enableLsc(isLscActive);
                                    tsf->TsfReset();
                                }
                            }
                        }
                    }
                }
                break;
            default:    //////////////////////////////////////////////////
                MY_LOG("[LscMgr:%s] YOU SHOULD NOT SEE ME (%d)!!!", __FUNCTION__, lsc->mTSFState);
                break;
        }
        ::pthread_mutex_unlock(&lsc->mTSFMutex);
    };

    if (dbg_tsf == 1) {
        char *filename = "/sdcard/tsfdata/TSFDebug.bin";
        tsf->TsfFeatureCtrl(MTKTSF_FEATURE_GET_LOG, 0, &TSFGetLog);
        FILE* fpdebug = fopen(filename,"wb");
        if ( fpdebug == NULL )
        {
            printf("Can't open :%s\n",filename);
        } else {
            fwrite((void*)TSFGetLog.DebugBuffAddr, TSFGetLog.DebugBuffSize, 1,fpdebug);
            fclose(fpdebug);
        }
    }
    //============================== destory instance ================================//
    tsf->TsfExit();
    tsf->destroyInstance();
    //================================================================================//
    delete [] gWorkinBuffer;
    if (gDBGWorkinBuffer)
        delete [] gDBGWorkinBuffer;


    MY_LOG("[LscMgr:%s]  end", __FUNCTION__);
    ::pthread_exit(0);
    return NULL;
}

/////////////////////////////////////
// TSF state machine control
/////////////////////////////////////
static MBOOL TSFStateRule[NSIspTuning::LscMgr::LSCMGR_TSF_STATE_NUM][NSIspTuning::LscMgr::LSCMGR_TSF_STATE_NUM] =
{
        ////////////////IDLE    INIT,  SCENECHANGE, GETNEWINPUT, DO,       EXIT
        /*IDLE*/        {MTRUE, MTRUE, MFALSE,      MFALSE,      MFALSE,   MTRUE},
        /*INIT*/        {MTRUE, MTRUE, MFALSE,      MFALSE,      MFALSE,   MTRUE},
        /*SCENECHANGE*/ {MTRUE, MFALSE,MTRUE,       MFALSE,      MFALSE,   MTRUE},
        /*GETNEWINPUT*/ {MTRUE, MFALSE,MFALSE,      MTRUE,       MFALSE,   MTRUE},
        /*DO*/          {MTRUE, MFALSE,MTRUE,       MFALSE,      MTRUE,    MTRUE},
        /*EXIT*/        {MTRUE, MFALSE,MFALSE,      MFALSE,      MFALSE,   MTRUE},
};

MBOOL
LscMgr::
changeTSFState(LSCMGR_TSF_STATE state)
{
    MBOOL ret = MFALSE;
    MY_LOG("[LscMgr:%s]  old/new state %d/%d", __FUNCTION__, mTSFState, state);

    ::pthread_mutex_lock(&mTSFMutex);
    if (TSFStateRule[mTSFState][state] == MTRUE)
    {
        mTSFState = state;
        ret = MTRUE;
    }
    else {
        MY_ERR("[LscMgr:%s]  Invalid transition!", __FUNCTION__);
        ret = MFALSE;
    }
    ::sem_post(&mTSFSem);
    ::pthread_mutex_unlock(&mTSFMutex);

    //////////////////////////////// make sure scene change processed done
    if (mTSFState == LSCMGR_TSF_STATE_SCENECHANGE && m_bMetaMode == MTRUE) {
        MUINT32 check_cnt = 0;
        do {
            usleep(100);
            check_cnt++;
        } while (mTSFState == LSCMGR_TSF_STATE_SCENECHANGE && check_cnt < 100);
        MY_LOG("[LscMgr:%s] check_cnt %d", __FUNCTION__, check_cnt);
    }
    ////////////////////////////////
    return ret;
}

//////////////////////////////////////
// when TSF finished a cycle, call this
// function to change buffer
//////////////////////////////////////
MVOID
LscMgr::
updateTSFBuffIdx(void)
{

}

MVOID
LscMgr::
enableTSF(MBOOL enable) {
    MY_LOG("[%s] m_bTSF(%d)", __FUNCTION__, enable);
    m_bTSF = enable;
}

MVOID
LscMgr::
notifyPreflash(MBOOL fgPreflash)
{
    MY_LOG("[%s] m_fgPreflash(%d)", __FUNCTION__, fgPreflash);
    m_fgPreflash = fgPreflash;
}

MBOOL
LscMgr::
loadTSFLut(void) {

#if USING_BUILTIN_LSC
    ::memcpy(reinterpret_cast<MVOID*>(m_TSFBuff[TSF_BUFIDX_INPUT].virtAddr),
            reinterpret_cast<MVOID*>(getLut(m_eLscScenario)),
            getPerLutSize(m_eLscScenario));

    ::memcpy(reinterpret_cast<MVOID*>(m_TSFBuff[TSF_BUFIDX_BAK].virtAddr),
            reinterpret_cast<MVOID*>(getLut(m_eLscScenario)),
            getPerLutSize(m_eLscScenario));
#else
    // The table should be 16x16
    ::memcpy(reinterpret_cast<MVOID*>(m_TSFBuff[TSF_BUFIDX_INPUT].virtAddr),
            reinterpret_cast<MVOID*>(stRawLscInfo[TSF_SCN_DFT].virtAddr + getTSFD65Idx()*getPerLutSize(TSF_SCN_DFT)),
            getPerLutSize(TSF_SCN_DFT));

    ::memcpy(reinterpret_cast<MVOID*>(m_TSFBuff[TSF_BUFIDX_BAK].virtAddr),
            reinterpret_cast<MVOID*>(stRawLscInfo[TSF_SCN_DFT].virtAddr + getTSFD65Idx()*getPerLutSize(TSF_SCN_DFT)),
            getPerLutSize(TSF_SCN_DFT));
#endif

    // load default table to double buffer
    MUINT32 u4Addr = stRawLscInfo[TSF_SCN_DFT].virtAddr + getTSFD65Idx()*getPerLutSize(TSF_SCN_DFT);
    ::memcpy((MVOID*)m_rBufInfo[0].virtAddr, (MVOID*)u4Addr, getPerLutSize(TSF_SCN_DFT));
    ::memcpy((MVOID*)m_rBufInfo[1].virtAddr, (MVOID*)u4Addr, getPerLutSize(TSF_SCN_DFT));

    if (*((MUINT32*)m_TSFBuff[TSF_BUFIDX_INPUT].virtAddr+0) == 0 &&
            *((MUINT32*)m_TSFBuff[TSF_BUFIDX_INPUT].virtAddr+1) == 0 &&
            *((MUINT32*)m_TSFBuff[TSF_BUFIDX_INPUT].virtAddr+2) == 0 &&
            *((MUINT32*)m_TSFBuff[TSF_BUFIDX_INPUT].virtAddr+3) == 0) {
        MY_ERR("[LscMgr:%s] Default table is ZERO!!", __FUNCTION__);
        return MFALSE;
    } else {
        MY_LOG("[LscMgr:%s] 0x%08x/0x%08x/0x%08x/0x%08x", __FUNCTION__,
                *((MUINT32*)m_TSFBuff[TSF_BUFIDX_INPUT].virtAddr+0),
                *((MUINT32*)m_TSFBuff[TSF_BUFIDX_INPUT].virtAddr+1),
                *((MUINT32*)m_TSFBuff[TSF_BUFIDX_INPUT].virtAddr+2),
                *((MUINT32*)m_TSFBuff[TSF_BUFIDX_INPUT].virtAddr+3));
        return MTRUE;
    }
}

MRESULT LscMgr::CCTOPSetTsfForceAwb(MBOOL fgForce)
{
    MY_LOG("[%s] fgForce(%d)\n", __FUNCTION__, fgForce);
    m_bTsfForceAWB = fgForce;
    return MTRUE;
}

#endif // ENABLE_TSF
//////////////////////////////////////
// update AWB statistics
//////////////////////////////////////
MBOOL
LscMgr::
updateTSFinput(LSCMGR_TSF_INPUT_SRC src, TSF_REF_INFO_T *ref, MVOID *stat)
{
#if ENABLE_TSF
    //    MY_LOG("[LscMgr:%s] lv %d, addr 0x%08x", __FUNCTION__,lv, stat);
    if (isTSFEnable() == MFALSE)
        return MTRUE;

    MINT32 u4Dbg0 = 0, u4Dbg1 = 0;
    GET_PROP("debug.lsc_mgr.tsfdbg0", "0", u4Dbg0);
    GET_PROP("debug.lsc_mgr.tsfdbg1", "0", u4Dbg1);

    ::pthread_mutex_lock(&mTSFMutex);
    m_u4FrmCnt++;
    // update parameters
    m_TsfAwbInfo.m_i4LV 		= ref->awb_info.m_i4LV;
    m_TsfAwbInfo.m_u4CCT 		= ref->awb_info.m_u4CCT;
    m_TsfAwbInfo.m_RGAIN 		= ref->awb_info.m_RGAIN;
    m_TsfAwbInfo.m_GGAIN		= ref->awb_info.m_GGAIN;
    m_TsfAwbInfo.m_BGAIN		= ref->awb_info.m_BGAIN;
    m_TsfAwbInfo.m_FLUO_IDX 	= ref->awb_info.m_FLUO_IDX;
    m_TsfAwbInfo.m_DAY_FLUO_IDX = ref->awb_info.m_DAY_FLUO_IDX;
    AWB_STAT_T *pAWBStat = reinterpret_cast<AWB_STAT_T *>(stat);

    if (u4Dbg0 == 0)
    {
        for (int y = 0; y < AWB_WINDOW_NUM_Y; y++) 
        {
            memcpy((void*)(m_TSFBuff[TSF_BUFIDX_AWB].virtAddr+(y*sizeof(AWB_WINDOW_T)*AWB_WINDOW_NUM_X)), (void*)&(pAWBStat->LINE[y]), sizeof(AWB_WINDOW_T)*AWB_WINDOW_NUM_X);
        }
    }
    else
    {
        // for AWB stat zero
        memset((void*)m_TSFBuff[TSF_BUFIDX_AWB].virtAddr, 0, AWB_STAT_SIZE);
    }

//    for (int i = sizeof(AWB_WINDOW_T)*AWB_WINDOW_NUM_X; i < sizeof(AWB_WINDOW_T)*AWB_WINDOW_NUM_X+64; i+=4)
//        MY_LOG("[LscMgr:%s] 0x%02x 0x%02x 0x%02x 0x%02x\n", __FUNCTION__,
//                *((MUINT8*)m_TSFBuff[TSF_BUFIDX_AWB].virtAddr+i+0),
//                *((MUINT8*)m_TSFBuff[TSF_BUFIDX_AWB].virtAddr+i+1),
//                *((MUINT8*)m_TSFBuff[TSF_BUFIDX_AWB].virtAddr+i+2),
//                *((MUINT8*)m_TSFBuff[TSF_BUFIDX_AWB].virtAddr+i+3));
    ::sem_post(&mTSFSem);
    ::pthread_mutex_unlock(&mTSFMutex);

    switch (src)
    {
        case TSF_INPUT_PV:
            MY_LOG_IF(_bEnableMyLog, "[%s] TSF_INPUT_PV: FrameCnt(%d), state(%d), lv(%d), cct(%d)", __FUNCTION__,
                m_u4FrmCnt, mTSFState, m_TsfAwbInfo.m_i4LV, m_TsfAwbInfo.m_u4CCT);
            if (u4Dbg1)
            {
                char fileAwbStat[256] = "\0";
                sprintf(fileAwbStat, "/sdcard/tsfdata/PVAWB_%04d.bin", m_u4FrmCnt);
                FILE* fpdebug = fopen(fileAwbStat, "wb");
                if ( fpdebug == NULL )
                {
                    MY_LOG("Can't open: %s\n", fileAwbStat);
                } 
                else
                {
                    fwrite((void*)m_TSFBuff[TSF_BUFIDX_AWB].virtAddr, m_TSFBuff[TSF_BUFIDX_AWB].size, 1, fpdebug);
                    fclose(fpdebug);
                }
            }
            break;
        case TSF_INPUT_CAP:
            MY_LOG("[LscMgr:%s:%d]  state %d, TSF_INPUT_CAP, lv %d, cct %d, rgain %d, bgain %d, ggain %d, fluo idx %d, day flou idx %d, addr 0x%08x", __FUNCTION__,
                    m_u4FrmCnt,
                    mTSFState,
                    m_TsfAwbInfo.m_i4LV,
                    m_TsfAwbInfo.m_u4CCT,
                    m_TsfAwbInfo.m_RGAIN,
                    m_TsfAwbInfo.m_GGAIN,
                    m_TsfAwbInfo.m_BGAIN,
                    m_TsfAwbInfo.m_FLUO_IDX,
                    m_TsfAwbInfo.m_DAY_FLUO_IDX,
                    stat);
            MY_LOG("[LscMgr:%s] Wait TSF thread", __FUNCTION__);
            //            for (int i = 0; i < 64; i+=4)
            //                MY_LOG("[LscMgr:%s] 0x%02x 0x%02x 0x%02x 0x%02x\n", __FUNCTION__,
            //                        *((MUINT8*)m_TSFBuff[TSF_BUFIDX_AWB].virtAddr+i+0),
            //                        *((MUINT8*)m_TSFBuff[TSF_BUFIDX_AWB].virtAddr+i+1),
            //                        *((MUINT8*)m_TSFBuff[TSF_BUFIDX_AWB].virtAddr+i+2),
            //                        *((MUINT8*)m_TSFBuff[TSF_BUFIDX_AWB].virtAddr+i+3));
            ::sem_wait(&mTSFSemSC);
            //////////////////////////////// Double check
            if (mTSFState == LSCMGR_TSF_STATE_GETNEWINPUT && m_bMetaMode == MTRUE) {
                MY_LOG("[LscMgr:%s] Weird!!!", __FUNCTION__);
                MUINT32 check_cnt = 0;
                do {
                    usleep(100);
                    check_cnt++;
                } while (mTSFState == LSCMGR_TSF_STATE_SCENECHANGE && check_cnt < 100);
                MY_LOG("[LscMgr:%s] check_cnt %d", __FUNCTION__, check_cnt);
            }
            ////////////////////////////////

//            { // DEBUG
//                char *filename = "/sdcard/tsfdata/CapAWB.bin";
//
//                FILE* fpdebug = fopen(filename,"wb");
//                if ( fpdebug == NULL )
//                {
//                    printf("Can't open :%s\n",filename);
//                } else {
//                    fwrite((void*)m_TSFBuff[TSF_BUFIDX_AWB].virtAddr, m_TSFBuff[TSF_BUFIDX_AWB].size, 1,fpdebug);
//                    fclose(fpdebug);
//                }
//            }
            MY_LOG("[LscMgr:%s] Wait TSF complete", __FUNCTION__);
            break;
        case TSF_INPUT_VDO:
            //	        MY_LOG("[LscMgr:%s:%d]  state %d, TSF_INPUT_VDO, lv %d, cct %d, addr 0x%08x", __FUNCTION__,
            //	               FrameCnt,
            //	               mTSFState,
            //	               lv,
            //	               cct,
            //	               stat);
            break;
    }
#endif
    return MTRUE;
}

MRESULT
LscMgr::
CCTOPSetSdblkFileCfg(MBOOL fgSave, const char* filename)
{
    m_bDumpSdblk = fgSave;
    m_strSdblkFile = filename;
    return 0;
}


} /* namespace NSIspTuning */
